/*
 * Copyright (c) 2008, AIST, the University of Tokyo and General Robotix Inc.
 * All rights reserved. This program is made available under the terms of the
 * Eclipse Public License v1.0 which accompanies this distribution, and is
 * available at http://www.eclipse.org/legal/epl-v10.html
 * Contributors:
 * National Institute of Advanced Industrial Science and Technology (AIST)
 */
/**
   @author Shin'ichiro Nakaoka
*/

#include "ColdetModelPair.h"
#include "ColdetModelSharedDataSet.h"
#include "Opcode/Opcode.h"

using namespace hrp;


ColdetModelPair::ColdetModelPair()
{

}


ColdetModelPair::ColdetModelPair(ColdetModelPtr model0, ColdetModelPtr model1)
{
    model0_ = model0;
    model1_ = model1;
}


ColdetModelPair::ColdetModelPair(const ColdetModelPair& org)
{
    model0_ = org.model0_;
    model1_ = org.model1_;
}


ColdetModelPair::~ColdetModelPair()
{

}


void ColdetModelPair::set(ColdetModelPtr model0, ColdetModelPtr model1)
{
    model0_ = model0;
    model1_ = model1;
}


collision_data* ColdetModelPair::detectCollisionsSub(bool detectAllContacts)
{
    if(cdContactsCount){
        if (cdContact){
            delete[] cdContact;
            cdContact = 0;
        }
        cdContactsCount = 0;
    }

    if (model0_->getPrimitiveType() == ColdetModel::SP_PLANE &&
        model1_->getPrimitiveType() == ColdetModel::SP_CYLINDER){
        return detectPlaneCylinderCollisions(detectAllContacts);
    }else{
        return detectMeshMeshCollisions(detectAllContacts);
    }
}

collision_data* ColdetModelPair::detectMeshMeshCollisions(bool detectAllContacts)
{
    collision_data* result = 0;
    
    if(model0_->isValid() && model1_->isValid()){

        Opcode::BVTCache colCache;

        // inverse order because of historical background
        // this should be fixed.(note that the direction of normal is inversed when the order inversed 
        colCache.Model0 = &model1_->dataSet->model;
        colCache.Model1 = &model0_->dataSet->model;
        
        Opcode::AABBTreeCollider collider;
        
        if(!detectAllContacts){
            collider.SetFirstContact(true);
        }
        
        bool isOK = collider.Collide(colCache, model1_->transform, model0_->transform);
        
        cdBoxTestsCount = collider.GetNbBVBVTests();
        cdTriTestsCount = collider.GetNbPrimPrimTests();
        
        if(isOK){
            result = cdContact;
        }
    }

    return result;
}

collision_data* ColdetModelPair::detectPlaneCylinderCollisions(bool detectAllContacts)
{
    IceMaths::Matrix4x4 pTrans = (*(model0_->pTransform)) * (*(model0_->transform));
    IceMaths::Matrix4x4 cTrans = (*(model1_->pTransform)) * (*(model1_->transform));

    float radius, height; // height and radius of cylinder
    model1_->getPrimitiveParam(0, radius);
    model1_->getPrimitiveParam(1, height);

    IceMaths::Point pTopLocal(0, height/2, 0), pBottomLocal(0, -height/2, 0);
    IceMaths::Point pTop, pBottom; // center points of top and bottom discs
    IceMaths::TransformPoint4x3(pTop,    pTopLocal,    cTrans);
    IceMaths::TransformPoint4x3(pBottom, pBottomLocal, cTrans);
    
    IceMaths::Point pOnPlane, nLocal(0,0,1), n;
    IceMaths::TransformPoint3x3(n, nLocal, pTrans);
    pTrans.GetTrans(pOnPlane);
    float d = pOnPlane|n; // distance between origin and plane

    float dTop    = (pTop|n) - d;
    float dBottom = (pBottom|n) - d;

    if (dTop > radius && dBottom > radius) return NULL;

    double theta = asin((dTop - dBottom)/height);
    double rcosth = radius*cos(theta);

    if (rcosth >= dTop) cdContactsCount++;
    if (rcosth >= dBottom) cdContactsCount++;

    if (cdContactsCount){
        collision_data *cdata = new collision_data[cdContactsCount];
        for (unsigned int i=0; i<cdContactsCount; i++){
            cdata[i].num_of_i_points = 1;
            cdata[i].i_point_new[0]=1; 
            cdata[i].i_point_new[1]=0; 
            cdata[i].i_point_new[2]=0; 
            cdata[i].i_point_new[3]=0; 
            cdata[i].n_vector[0] = n.x;
            cdata[i].n_vector[1] = n.y;
            cdata[i].n_vector[2] = n.z;
        }
        IceMaths::Point vBottomTop = pTop - pBottom;
        IceMaths::Point v = vBottomTop^n;
        IceMaths::Point w = v^n;
        w.Normalize();

        unsigned int index=0;
        if (rcosth >= dBottom){ // bottom disc collides
            double depth = rcosth - dBottom;
            IceMaths::Point iPoint = pBottom - dBottom*n - dBottom*tan(theta)*w;
            cdata[index].i_points[0][0] = iPoint.x;
            cdata[index].i_points[0][1] = iPoint.y;
            cdata[index].i_points[0][2] = iPoint.z;
            cdata[index].depth = depth;
            index++;
        }
        if (rcosth >= dTop){ // top disc collides
            double depth = rcosth - dTop;
            IceMaths::Point iPoint = pTop - dTop*n - dTop*tan(theta)*w;
            cdata[index].i_points[0][0] = iPoint.x;
            cdata[index].i_points[0][1] = iPoint.y;
            cdata[index].i_points[0][2] = iPoint.z;
            cdata[index].depth = depth;
            index++;
        }

        return cdata;
    }
    return NULL;
}
