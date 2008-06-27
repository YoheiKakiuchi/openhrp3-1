// -*- mode: c++; indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*-
/** @file CollisionDetector/server/CollisionDetector_impl.cpp
 * Implementation of CollisionDetector_impl and CollisionDetectorFactory_impl
 *
 * @version 0.2
 * @date 2002/01/15
 *
 */

#include "CollisionDetector_impl.h"
#include "CdCache.h"

#include <time.h>
#include <iostream>
#include <string>
#include <vector>

using namespace std;
using namespace OpenHRP;

//#define COLLISIONDETECTOR_DEBUG

CollisionDetector_impl::CollisionDetector_impl
(
 CORBA_ORB_ptr        orb,
 CdCharCache*         cache
 ) : orb_(CORBA_ORB::_duplicate(orb))
{
#ifdef COLLISIONDETECTOR_DEBUG
    cerr << "CollisionDetector_impl::CollisionDetector_impl()" << endl;
#endif
    cache_ = cache;
    scene_ = new CdScene();
}

CollisionDetector_impl::~CollisionDetector_impl()
{
#ifdef COLLISIONDETECTOR_DEBUG
    cerr << "CollisionDetector_impl::~CollisionDetector_impl()" << endl;
#endif
    delete scene_;
}

void
CollisionDetector_impl::destroy()
{
#ifdef COLLISIONDETECTOR_DEBUG
    cerr << "CollisionDetector_impl::destroy()" << endl;
#endif
    PortableServer::POA_var poa = _default_POA();
    PortableServer::ObjectId_var id = poa -> servant_to_id(this);
    poa -> deactivate_object(id);
}




void CollisionDetector_impl::addModel(
	const char* charName,
	BodyInfo_ptr binfo)
{
#ifdef COLLISIONDETECTOR_DEBUG
    cerr << "CollisionDetector_impl::addModel() " << charName <<endl;
#endif

// #define READTRI_COUNT 1000

    CdModelCache* cachedModel;

    //���ɃL���b�V���ɓ����Ă�H
    CORBA::String_var url = binfo->url();

    if( cache_->exist( url ) )
	{
        cachedModel = cache_->getChar( url );
    }
	else
	{
        cerr << "creating Cd object..." << endl;

        //���[�h
        LinkInfoSequence_var jList = binfo->links();
        cachedModel = new CdModelCache();

        double p[3][3];
        // DblSequence tris;
		vector<double> triangles;
        CORBA::String_var jName;

        // �W���C���g���ƂɎO�p�`�W���ǉ�
		for( unsigned int i = 0 ; i < jList->length() ; ++i )
		{
			jName = jList[i].name;
            int ntri = 0;
	
			// ���̃����N��ShapeInfo�̒��_���W�Q���擾����
			triangles = getTrianglesOfLink( i, binfo );

			// ���_����9�̔{���łȂ���΁C
			//  ( ��O�p���b�V���Ȃ̂ŁC���_����9�̔{���ł���͂�)
			if( triangles.size() % 9 )
			{
				// ##### [TODO] ##### �G���[�𓊂���
				cerr << "There is a probrem in the number of vertices.";
				continue;
			}
			size_t trianglesNumber = triangles.size() / 9;

			CdModelSet* modelSet = 0;

			// ��W���C���g�ł͂͂Ȃ���
			if( 0 < trianglesNumber )
			{
				modelSet = new CdModelSet();
				modelSet->linkIndex = i;

				for( size_t j = 0 ; j < trianglesNumber ; j++ )
				{
					p[0][0] = triangles[j*9+0];
					p[0][1] = triangles[j*9+1];
					p[0][2] = triangles[j*9+2];
					p[1][0] = triangles[j*9+3];
					p[1][1] = triangles[j*9+4];
					p[1][2] = triangles[j*9+5];
					p[2][0] = triangles[j*9+6];
					p[2][1] = triangles[j*9+7];
					p[2][2] = triangles[j*9+8];
// ##### DEBUG
//cout << "( " << p[0][0] << ", " << p[0][1] << ", " << p[0][2] << ") ";
//cout << "( " << p[1][0] << ", " << p[1][1] << ", " << p[1][2] << ") ";
//cout << "( " << p[2][0] << ", " << p[2][1] << ", " << p[2][2] << ") ";
//if( ! ( j % 9 ) ) cout << endl;
// ##### DEBUG
					modelSet->addTriangle( p[0], p[1], p[2] );					

				}
				modelSet->endModel();
			}

			//�L�����N�^�ɃW���C���g��ǉ�
			cachedModel->addModel(jName, modelSet);
			
			cerr << jName << " has "<< trianglesNumber << " triangles." << endl;
        }

		cerr << "finished." << endl;

		//�L���b�V���ɃL�����N�^��ǉ�
		cache_->addChar(url,cachedModel);

	}

	scene_->addChar(charName, new CdChar(cachedModel,charName) );

}


vector<double> CollisionDetector_impl::getTrianglesOfLink(
	int linkIndex,
	BodyInfo_ptr binfo )
{
	vector<double> triangles;

	ShapeInfoSequence_var	shapes				= binfo->shapes(); 
	AllLinkShapeIndices_var	allLinkShapeIndices	= binfo->linkShapeIndices(); 

	// linkIndex�Ԗڂ̃����N��ShapeIndices
	ShortSequence shapeIndices = allLinkShapeIndices[linkIndex];
	
	// ����Link��ShapeInfo�̐������[�v����
	for( size_t i = 0 ; i < shapeIndices.length() ; i++ )
	{
		// shapeIndices���Ŏw�肳�ꂽ shapeInfo �̃C���f�b�N�X��
		short shapeIndex = shapeIndices[i];
		ShapeInfo shapeInfo = shapes[shapeIndex];

		// triangles���w���������_���Ƀ��[�v����
		for( size_t t = 0 ; t < shapeInfo.triangles.length() ; t++ )
		{
			// ���_�ԍ��́C
			long vertexIndex = shapeInfo.triangles[t];

			// ���_���W�́C
			//  [NOTE] vertices�ɂ� XYZXYZ...�̂悤�ɒ��_���W���i�[����Ă���̂ŁC
			//         ���_�ԍ�(vertexIndex)�ɑΉ�����(vertices����)���_���W�̔z��Y����
			//             X���W = vertexIndex*3
			//             Y���W = vertexIndex*3+1
			//             Z���W = vertexIndex*3+2
			//         �Ŏ擾(�A�N�Z�X)�\�ł���B
			double x = static_cast<double>(shapeInfo.vertices[vertexIndex*3+0]);
			double y = static_cast<double>(shapeInfo.vertices[vertexIndex*3+1]);
			double z = static_cast<double>(shapeInfo.vertices[vertexIndex*3+2]);

			triangles.push_back( x );
			triangles.push_back( y );
			triangles.push_back( z );
			
			// ��L�̏����͈ȉ��L�q���\�ł��邪�C����Ղ�����ړI�� 3���W�ɂ��ċL�q�����B
			//	for( int c = 0 ; c < 3 ; c++ )
			//	{
			//		triangles.push_back( static_cast<double>(shapeInfo.vertices[vertexIndex*3+c]) );
			//	}
		}
	}

	return triangles;
}


#if 0 // �����O

void CollisionDetector_impl::addModel(
	const char* charName,
	BodyInfo_ptr binfo)
{
#ifdef COLLISIONDETECTOR_DEBUG
    cerr << "CollisionDetector_impl::addModel() " << charName <<endl;
#endif

#define READTRI_COUNT 1000

    CdModelCache* cachedModel;

    //���ɃL���b�V���ɓ����Ă�H
    CORBA::String_var url = binfo->url();

    if(cache_->exist(url)){
        cachedModel = cache_->getChar(url);
    }else{
        cerr << "creating Cd object..." << endl;

        //���[�h
        LinkInfoSequence_var jList = binfo->links();
        cachedModel = new CdModelCache();

        double p[3][3];
        DblSequence tris;
        CORBA::String_var jName;

        // �W���C���g���ƂɎO�p�`�W���ǉ�
		for( unsigned int i = 0 ; i < jList->length() ; ++i )
		{
			jName = jList[i].name;
            int ntri = 0;

// ###### [TODO] readTriangles �� NewModelLoader �ɂ͖����B
//               ShapeInfo ���琶�����铯���̏����������ɁB 
//            tris = jList[i]->readTriangles(ntri, READTRI_COUNT);

			// linkShapeIndices �ɑS�����N�ɑ�����SahpeInfo (shapes��Index)
			//���A�����N���ɃZ�b�g����Ă���̂ŁA�����N�����w�肵�Ď擾����B
			AllLinkShapeIndices_var alllinkShapeIndices = binfo->linkShapeIndices(); 

			//�P�����N��Shape��
			int shapeCountsByLink = alllinkShapeIndices[i].length();

			//�� i �����N��ShapeIndices �̃V�[�P���X�f�[�^
			ShortSequence shapeIndicesSeqByLink = alllinkShapeIndices[i];
			short shapeIndex;		// �������ׂ� ShapeInfo�� shapeIndicesSeqByLink �̒���Index
			int triCountsSum = 0;	// tri �f�[�^�T�C�Y
			int triInsertIndex = 0;	// tri �f�[�^ �}���ʒu

			// �P�����N��Shape���� ���[�v�� �O�p���b�V���f�[�^�� tris[] �ɃZ�b�g����B
			for(unsigned int k=0; k < shapeCountsByLink; k++){

				// (triangle)�ʏ��� �� k ���_�� shapes���� Index�擾����B
				shapeIndex = shapeIndicesSeqByLink[k];	
				// BodyInfo���� shapes�f�[�^��擾
				ShapeInfoSequence_var shapeIndicesByLink = binfo->shapes(); 
				// shapes�f�[�^�񂩂�� k �Ԗڗv�f�� ShapeInfo(k) �擾
				ShapeInfo shapeInfoByLink = shapeIndicesByLink[k];

				// ShapeInfo(k) �̎O�p���b�V�����~�R ���擾����
				int meshVertexCounts = shapeInfoByLink.triangles.length();

				// ShapeInfo(k)�̎O�p�p�b�`�f�[�^���i�O�p���b�V�����~�R�~�R�j�Z�o
				int triCounts = meshVertexCounts * 3;

				// tris �̒����ݒ�
				triInsertIndex = triCountsSum;	//tri �f�[�^ �}���ʒu
				triCountsSum += triCounts;		//tri �f�[�^ ���T�C�Y�T�C�Y
				tris.length( triCountsSum );

				// tris�ɒ��_���ƂɁiXYZ)���W�l�i�[����
				for(unsigned int m=0; m < meshVertexCounts; m++){ 
					int vNo = shapeInfoByLink.triangles[m];
					int vindex_X = vNo * 3;
					tris[triInsertIndex + m + 0] = shapeInfoByLink.vertices[vindex_X];
					tris[triInsertIndex + m + 1] = shapeInfoByLink.vertices[vindex_X + 1];
					tris[triInsertIndex + m + 2] = shapeInfoByLink.vertices[vindex_X + 2];
				}
			}

			CdModelSet* modelSet = 0;
			
            //��W���C���g�ł͂͂Ȃ���
			if(tris.length())
			{
                modelSet = new CdModelSet();
				modelSet->linkIndex = i;
				
				int trisCnts = tris.length()/9;

                while (trisCnts > 0){
                    ntri += tris.length()/9;	//�VIDL�ł́A���g�p
					trisCnts -= 1;
                    for(unsigned int j=0;j<tris.length();j+=9){
                        p[0][0]=tris[j+0];
                        p[0][1]=tris[j+1];
                        p[0][2]=tris[j+2];
                        p[1][0]=tris[j+3];
                        p[1][1]=tris[j+4];
                        p[1][2]=tris[j+5];
                        p[2][0]=tris[j+6];
                        p[2][1]=tris[j+7];
                        p[2][2]=tris[j+8];
                        modelSet->addTriangle(p[0],p[1],p[2]);					
                    }
                    //��tris�͎����I�ɉ�������

					// ###### [TODO] readTriangles �� NewModelLoader �ɂ͖����B
					//               ShapeInfo ���琶�����铯���̏����������ɁB
                    // tris=jList[i]->readTriangles(ntri, READTRI_COUNT);
                }
                modelSet->endModel();
			}

			//�L�����N�^�ɃW���C���g��ǉ�
			cachedModel->addModel(jName, modelSet);
			
            cerr << jName << " has "<< ntri << " tris." << endl;
        }

        cerr << "finished." << endl;

        //�L���b�V���ɃL�����N�^��ǉ�
        cache_->addChar(url,cachedModel);

    }

    scene_->addChar(charName, new CdChar(cachedModel,charName) );

}

#endif // �����O



void CollisionDetector_impl::addCollisionPair(const LinkPair& colPair,
                                              CORBA::Boolean convexsize1,
                                              CORBA::Boolean convexsize2)
{
    const char* charName1 = colPair.charName1;
    const char* charName2 = colPair.charName2;
    const char* jointName1 = colPair.linkName1;
    const char* jointName2 = colPair.linkName2;
    //#ifdef COLLISIONDETECTOR_DEBUG
    cerr << "CollisionDetector_impl::addCollisionPair("
         << charName1 <<  "::" << jointName1 << ", "
         << charName2 <<  "::" << jointName2 << ")" << endl;
    //#endif

	CdJoint* joint1 = 0;
	CdJoint* joint2 = 0;

    CdChar* char1 = scene_->getChar(charName1);
    if (!char1){
        cerr << "CollisionDetector_impl::addCollisionPair : Character not found("
             << charName1 << ")" << endl;
    } else {
	    joint1 = char1->getJoint(jointName1);
    	if (!joint1){
        	cerr << "CollisionDetector_impl::addCollisionPair : Joint not found("
            	 << charName1 << ","
             	<< jointName1 << ")" << endl;
	    }
    }

    CdChar* char2 = scene_->getChar(charName2);
    if (!char2){
        cerr << "CollisionDetector_impl::addCollisionPair : Character not found("
             << charName2 << ")" << endl;
    } else {
    	joint2 = char2->getJoint(jointName2);
    	if (!joint2){
        	cerr << "CollisionDetector_impl::addCollisionPair : Joint not found("
            	 << charName2 << ","
             	<< jointName2 << ")" << endl;
    	}
    }

    static const bool NOT_SKIP_EMPTY_JOINT = true;

    if(NOT_SKIP_EMPTY_JOINT || (joint1 && joint2)){
    	scene_->addCheckPair(new CdCheckPair(joint2, joint1));
    }

}

void CollisionDetector_impl::removeCollisionPair(const LinkPair& colPair)
{
#ifdef COLLISIONDETECTOR_DEBUG
    cerr << "CollisionDetector_impl::removeCollisionPair()" << endl;
#endif
}


//�W���C���g�ʒu�Z�b�g
void CollisionDetector_impl::_setCharacterData
(
 const CharacterPositionSequence& characterPositions
 )
{
    if (joints.size() == 0){ //first time
        for (unsigned int i=0; i < characterPositions.length(); i++){
            const CharacterPosition& characterPosition = characterPositions[i];
            CdChar *rChar = scene_->getChar(characterPosition.characterName);
            if (rChar){
				const LinkPositionSequence& linkPositions = characterPosition.linkPositions;
                for (unsigned int j=0; j < linkPositions.length(); j++){
                    CdJoint *rJoint = rChar->getJoint(j);
                    if (rJoint){
                        joints.push_back(rJoint);
                    }else{
                        joints.push_back(NULL);
                    }
                }
            } else {
                cerr << "CollisionDetector_impl::_setCharacterData : Character not found(" << characterPosition.characterName << ")" << endl;
            }
        }
    }

    //�W���C���g�̈ʒu�p�����Z�b�g
    vector<CdJoint *>::iterator it = joints.begin();
    for(unsigned int i = 0; i < characterPositions.length(); i++){
		const LinkPositionSequence& linkPositions = characterPositions[i].linkPositions;
        for (unsigned int j=0; j < linkPositions.length(); j++, it++){
			CdJoint* cdJoint = *it;
            if (cdJoint){
                const LinkPosition& linkPosition = linkPositions[j];
				const DblArray3& po = linkPosition.p;
				const DblArray9& Ro = linkPosition.R;
				double* p    = cdJoint->translation_;
				double (*R)[3] = cdJoint->rotation_;
				p[0] = po[0]; p[1] = po[1]; p[2] = po[2];
				R[0][0] = Ro[0]; R[0][1] = Ro[1]; R[0][2] = Ro[2];
				R[1][0] = Ro[3]; R[1][1] = Ro[4]; R[1][2] = Ro[5];
				R[2][0] = Ro[6]; R[2][1] = Ro[7]; R[2][2] = Ro[8];
            }
        }
    }
}

//������
int CollisionDetector_impl::_contactIntersection
(
 CdCheckPair* rPair
 )
{
    collision_data* pair;
    int ret;
    int i;

    rPair->collide(&ret,&pair,CD_FIRST_CONTACT);

    // ���̒i�K�ł͐ڐG�_��������Ȃ��̂ŁA�ڐG�_�����J�E���g����B
    int npoints = 0;
    for (i = 0; i < ret; ++i) {
        npoints += pair[i].num_of_i_points;
    }
    delete pair;
    return npoints;
}
//������
void CollisionDetector_impl::_contactDetermination(CdCheckPair* rPair, Collision&  out_collision)
{
    int ret;

	collision_data *cdata;
    rPair->collide(&ret, &cdata);

    // ���̒i�K�ł͐ڐG�_��������Ȃ��̂ŁA�ڐG�_�����J�E���g����B
    int npoints = 0;
    for (int i = 0; i < ret; i++) {
        for (int j = 0; j<cdata[i].num_of_i_points; j++){
            if (cdata[i].i_point_new[j]) npoints ++;
        }
    }
    if (npoints > 0){
        out_collision.points.length(npoints);

        int idx = 0;
        for (int i = 0; i < ret; i++) {
			collision_data& cd = cdata[i];
            for (int j=0; j < cd.num_of_i_points; j++){
                if (cd.i_point_new[j]){
					CollisionPoint& point = out_collision.points[idx];
                    for(int k=0; k < 3; k++){
                        point.position[k] = cd.i_points[j][k];
					}
                    for(int k=0; k < 3; k++){
                        point.normal[k]   = cd.n_vector[k];
                    }
                    point.idepth = cd.depth;
                    idx++;
                }
            }
        }
    }
    if (rPair->joint_[0]){
        out_collision.pair.charName1 = CORBA::string_dup(rPair->joint_[0]->parent_->name_.c_str());    // �p�X��1
        out_collision.pair.linkName1 = CORBA::string_dup(rPair->joint_[0]->name_.c_str());    // �p�X��1
    }
    if (rPair->joint_[1]){
        out_collision.pair.charName2 = CORBA::string_dup(rPair->joint_[1]->parent_->name_.c_str());    // �p�X��2
        out_collision.pair.linkName2 = CORBA::string_dup(rPair->joint_[1]->name_.c_str());    // �p�X��2
    }
}


CORBA::Boolean
CollisionDetector_impl::queryContactDeterminationForDefinedPairs
(
 const CharacterPositionSequence& characterPositions,
 CollisionSequence_out collisions
 )
{
#ifdef COLLISIONDETECTOR_DEBUG
    cerr << "CollisionDetector_impl::queryContactDeterminationForDefinedPairs()" << endl;
#endif
    _setCharacterData(characterPositions);

    long unsigned int numPair = scene_->getNumCheckPairs();

    collisions = new CollisionSequence;
    collisions->length(numPair);
    bool flag = false;
    for(long unsigned int pCount = 0 ; pCount < numPair ; ++pCount){
        CdCheckPair* rPair = scene_->getCheckPair(pCount);
        _contactDetermination(rPair, collisions[pCount]);
        if (collisions[pCount].points.length() > 0) flag = true;
    }

    //cerr << "CollisionDetector_impl::checkCollision(2)" << endl;

//###### DEBUG
	if( flag )
	{
		cout << "##### Collision!!!!" << endl;
	}

    return flag;
}

CORBA::Boolean
CollisionDetector_impl::queryContactDeterminationForGivenPairs
(
 const LinkPairSequence& checkPairs,
 const CharacterPositionSequence& characterPositions,
 CollisionSequence_out collisions
 )
{
#ifdef COLLISIONDETECTOR_DEBUG
    cerr << "CollisionDetector_impl::queryContactDeterminationForGivenPairs()" << endl;
#endif

    _setCharacterData(characterPositions);

    vector<CdCheckPair*> rPairSeq;

    //�Վ��y�A����
    for(unsigned int i=0;i<checkPairs.length();++i){
        const char* charName1 = checkPairs[i].charName1;
        const char* charName2 = checkPairs[i].charName2;
        const char* jointName1 = checkPairs[i].linkName1;
        const char* jointName2 = checkPairs[i].linkName2;

        CdChar* char1 = scene_->getChar(charName1);
        CdChar* char2 = scene_->getChar(charName2);

        if(char1 && char2){
            CdJoint* joint1 = char1->getJoint(jointName1);
            CdJoint* joint2 = char2->getJoint(jointName2);
            if(joint1 && joint2){
                rPairSeq.push_back(
                                   new CdCheckPair(
												   joint2,
												   joint1
												   )
                                   );
            }
        }

    }

    collisions = new CollisionSequence;
    collisions->length(rPairSeq.size());
    bool flag = false;
    // check
    unsigned long int pCount;
    for(pCount = 0 ; pCount < rPairSeq.size() ; ++pCount){
        CdCheckPair* rPair = rPairSeq[pCount];
        _contactDetermination(rPair,collisions[pCount]);
        if (collisions[pCount].points.length() > 0) flag = true;
    }
    for(pCount = 0 ; pCount < rPairSeq.size() ; ++pCount){
        // delete
        CdCheckPair* rPair = rPairSeq[pCount];
        delete rPair;
    }

    //cerr << "CollisionDetector_impl::checkCollision(2)" << endl;

    return flag;
}

CORBA::Boolean CollisionDetector_impl::queryIntersectionForDefinedPairs
(
 CORBA::Boolean checkAll,
 const CharacterPositionSequence& characterPositions,
 LinkPairSequence_out collidedPairs
 )
{
    _setCharacterData(characterPositions);

    int numPair = scene_->getNumCheckPairs();


    vector<CdCheckPair*> contactPairs;

    bool flag = false;
    for(int pCount = 0 ; pCount < numPair ; ++pCount){
        CdCheckPair* rPair = scene_->getCheckPair(pCount);
        int ret = _contactIntersection(rPair);
        if (ret){
            contactPairs.push_back(rPair);
        }
        flag = (flag || ret);
        if(!checkAll && flag){
            break;
        }
    }

    //���쐬

    //cerr << "CollisionDetector_impl::checkCollision(2)" << endl;


    collidedPairs = new LinkPairSequence();
    collidedPairs->length(contactPairs.size());

    for(unsigned int i=0;i<contactPairs.size();++i){
        CdCheckPair* rPair = contactPairs[i];
        (*collidedPairs)[i].charName1 = CORBA::string_dup(rPair->joint_[0]->parent_->name_.c_str());
        (*collidedPairs)[i].linkName1 = CORBA::string_dup(rPair->joint_[0]->name_.c_str());
        (*collidedPairs)[i].charName2 = CORBA::string_dup(rPair->joint_[1]->parent_->name_.c_str());
        (*collidedPairs)[i].linkName2 = CORBA::string_dup(rPair->joint_[1]->name_.c_str());
    }

    return flag;
}

CORBA::Boolean CollisionDetector_impl::queryIntersectionForGivenPairs
(
 CORBA::Boolean checkAll,
 const LinkPairSequence& checkPairs,
 const CharacterPositionSequence& characterPositions,
 LinkPairSequence_out collidedPairs
 )
{
#ifdef COLLISIONDETECTOR_DEBUG
    //cerr << "CollisionDetector_impl::queryIntersectionForGivenPairs()" << endl;
#endif
    _setCharacterData(characterPositions);

    vector<CdCheckPair*> rPairSeq;

    //�Վ��y�A����
    unsigned int i;
    for(i=0;i<checkPairs.length();++i){
        const char* charName1  = checkPairs[i].charName1;
        const char* charName2  = checkPairs[i].charName2;
        const char* jointName1 = checkPairs[i].linkName1;
        const char* jointName2 = checkPairs[i].linkName2;

        //cerr << checkPairs[i].charName1 << "." <<checkPairs[i].jointName1<< "-";
        //cerr << checkPairs[i].charName2 << "." <<checkPairs[i].jointName2<<endl;

        CdChar* char1 = scene_->getChar(charName1);
        CdChar* char2 = scene_->getChar(charName2);

        if(char1 && char2){
            CdJoint* joint1 = char1->getJoint(jointName1);
            CdJoint* joint2 = char2->getJoint(jointName2);
            if(joint1 && joint2){
                rPairSeq.push_back(
                                   new CdCheckPair(
												   joint2,
												   joint1
												   )
                                   );
            }
            //cerr << "create" ;
        }
        //cerr << endl;

    }


    vector<CdCheckPair*> contactPairs;

    bool flag = false;
    unsigned int pCount;
    for(pCount = 0 ; pCount < rPairSeq.size() ; ++pCount){
        CdCheckPair* rPair = rPairSeq[pCount];
        int ret = _contactIntersection(rPair);
        if (ret){
            contactPairs.push_back(rPair);
            //cerr << "insects! "<< endl;
        }
        flag = (flag || ret);
        if(!checkAll && flag){
            break;
        }
    }

    //���쐬

    //cerr << "CollisionDetector_impl::checkCollision(2)" << endl;


    collidedPairs = new LinkPairSequence();
    collidedPairs->length(contactPairs.size());

    for(i=0;i<contactPairs.size();++i){
        CdCheckPair* rPair = contactPairs[i];
        (*collidedPairs)[i].charName1 = CORBA::string_dup(rPair->joint_[0]->parent_->name_.c_str());
        (*collidedPairs)[i].linkName1 = CORBA::string_dup(rPair->joint_[0]->name_.c_str());
        (*collidedPairs)[i].charName2 = CORBA::string_dup(rPair->joint_[1]->parent_->name_.c_str());
        (*collidedPairs)[i].linkName2 = CORBA::string_dup(rPair->joint_[1]->name_.c_str());
    }

    for(pCount = 0 ; pCount < rPairSeq.size() ; ++pCount){
        // delete
        CdCheckPair* rPair = rPairSeq[pCount];
        delete rPair;
    }

    return flag;

}


void CollisionDetector_impl::clearCache(const char* url)
{
#ifdef COLLISIONDETECTOR_DEBUG
    cerr << "CollisionDetector_impl::clearCache()" << endl;
#endif
    cache_->removeChar(url);
}

void CollisionDetector_impl::clearAllCache()
{
#ifdef COLLISIONDETECTOR_DEBUG
    cerr << "CollisionDetector_impl::clearAllCache()" << endl;
    cache_->removeAllChar();
#endif
}

CollisionDetectorFactory_impl::CollisionDetectorFactory_impl
(
 CORBA_ORB_ptr   orb
 ) : orb_(CORBA_ORB::_duplicate(orb))
{
#ifdef COLLISIONDETECTOR_DEBUG
    cerr << "CollisionDetectorFactory_impl::CollisionDetectorFactory_impl()" << endl;
#endif
    cache_ = new CdCharCache();
}

CollisionDetectorFactory_impl::~CollisionDetectorFactory_impl()
{
#ifdef COLLISIONDETECTOR_DEBUG
    cerr << "CollisionDetectorFactory_impl::~CollisionDetectorFactory_impl()" << endl;
#endif
    PortableServer::POA_var poa = _default_POA();
    PortableServer::ObjectId_var id = poa -> servant_to_id(this);
    poa -> deactivate_object(id);
    delete cache_;
}

CollisionDetector_ptr
CollisionDetectorFactory_impl::create()
{
#ifdef COLLISIONDETECTOR_DEBUG
    cerr << "CollisionDetectorFactory_impl::create()" << endl;
#endif

    CollisionDetector_impl* collisionDetectorImpl = new CollisionDetector_impl(orb_,cache_);

    PortableServer::ServantBase_var collisionDetectorrServant = collisionDetectorImpl;
    PortableServer::POA_var poa_ = _default_POA();
    PortableServer::ObjectId_var id =
        poa_ -> activate_object(collisionDetectorImpl);
    return collisionDetectorImpl -> _this();
}

void CollisionDetectorFactory_impl::shutdown()
{
    orb_->shutdown(false);
}