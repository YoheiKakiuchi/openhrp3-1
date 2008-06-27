// -*- mode: c++; indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*-

/**
   \file
   \brief Implementations of Link class
   \author S.Nakaoka
*/


/**
   \ifnot jp
   \class OpenHRP::Link
   A class for representing a rigid body that consists of an articulated body.
   
   \var int Link::index
   Index among all the links in the body.
   
   \endif
*/


/**
   \if jp
   \class OpenHRP::Link
   他関節モデルの中の個々の剛体（リンク）を表すクラス。
   
   \var int Link::index
   全リンクを対象としたリンクのインデックス。
   
   モデルファイルにおけるJointノード定義の出現順（ルートからの探索順）に対応する。
   なお、関節となるリンクのみを対象としたインデックスとして、jointId がある。
   \endif
*/


#include "Link.h"


using namespace std;
using namespace OpenHRP;


Link::Link()
{
    jointId = -1;
    parent = 0;
	sibling = 0;
	child = 0;
	
	isHighGainMode = false;
}


Link::Link(const Link& org)
{
    jointType = org.jointType;
    jointId = org.jointId;
    a = org.a;
    d = org.d;
    b = org.b;
    c = org.c;
    m = org.m;
    I = org.I;
    Ir = org.Ir;
    torqueConst = org.torqueConst;
    encoderPulse = org.encoderPulse;
    gearRatio = org.gearRatio;
	gearEfficiency = org.gearEfficiency;
    rotorResistor = org.rotorResistor;
    Jm2 = org.Jm2;
    ulimit = org.ulimit;
    llimit = org.llimit;
    uvlimit = org.uvlimit;
    lvlimit = org.lvlimit;
	isHighGainMode = org.isHighGainMode;

    parent = child = sibling = 0;

    if(org.child){
        for(Link* orgChild = org.child; orgChild; orgChild = orgChild->sibling){
            Link* newChild = new Link(*orgChild);
            newChild->parent = this;
            newChild->sibling = child;
            child = newChild;
        }
    }
}


Link::~Link()
{
    Link* link = child;
    while(link){
        Link* linkToDelete = link;
        link = link->sibling;
        delete linkToDelete;
    }
}


namespace {
	void setBodyIter(Link* link, Body* body)
	{
		link->body = body;
		
		if(link->sibling){
			setBodyIter(link->sibling, body);
		}
		if(link->child){
			setBodyIter(link->child, body);
		}
	}
}


void Link::addChild(Link* link)
{
	if(link->parent){
		link->parent->detachChild(link);
	}

    link->sibling = child;
    link->parent = this;
    child = link;

	setBodyIter(link, body);
}


/**
   A child link is detached from the link.
   The detached child link is *not* deleted by this function.
   If a link given by the parameter is not a child of the link, false is returned.
*/
bool Link::detachChild(Link* childToRemove)
{
	bool removed = false;

	Link* link = child;
	Link* prevSibling = 0;
	while(link){
		if(link == childToRemove){
			removed = true;
			if(prevSibling){
				prevSibling->sibling = link->sibling;
			} else {
				child = link->sibling;
			}
			break;
		}
		prevSibling = link;
		link = link->sibling;
	}

	if(removed){
		childToRemove->parent = 0;
		childToRemove->sibling = 0;
		setBodyIter(childToRemove, 0);
	}

	return removed;
}


std::ostream& operator<<(std::ostream &out, Link& link)
{
    link.putInformation(out);
    return out;
}


void Link::putInformation(std::ostream& os)
{
    os << "Link " << name << " Link Index = " << index << ", Joint ID = " << jointId << "\n";

    os << "Joint Type: ";

    switch(jointType) {
    case FREE_JOINT:
        os << "Free Joint\n";
        break;
    case FIXED_JOINT:
        os << "Fixed Joint\n";
        break;
    case ROTATIONAL_JOINT:
        os << "Rotational Joint\n";
        os << "Axis = " << a << "\n";
        break;
    case SLIDE_JOINT:
        os << "Slide Joint\n";
        os << "Axis = " << d << "\n";
        break;
    }

    os << "parent = " << (parent ? parent->name : "null") << "\n";

	os << "child = ";
	if(child){
		Link* link = child;
		while(true){
			os << link->name;
			link = link->sibling;
			if(!link){
				break;
			}
        	os << ", ";
		}
	} else {
		os << "null";
	}
    os << "\n";

    os << "b = "  << b << "\n";
    os << "c = "  << c << "\n";
    os << "m = "  << m << "\n";
    os << "Ir = " << Ir << "\n";
    os << "I = "  << I << "\n";
    os << "torqueConst = " << torqueConst << "\n";
    os << "encoderPulse = " << encoderPulse << "\n";
    os << "gearRatio = " << gearRatio << "\n";
	os << "gearEfficiency = " << gearEfficiency << "\n";
    os << "Jm2 = " << Jm2 << "\n";
    os << "ulimit = " << ulimit << "\n";
    os << "llimit = " << llimit << "\n";
    os << "uvlimit = " << uvlimit << "\n";
    os << "lvlimit = " << lvlimit << "\n";

	if(false){
		os << "R = " << R << "\n";
		os << "p = " << p << ", wc = " << wc << "\n";
    	os << "v = " << v << ", vo = " << vo << ", dvo = " << dvo << "\n";
    	os << "w = " << w << ", dw = " << dw << "\n";

    	os << "u = " << u << ", q = " << q << ", dq = " << dq << ", ddq = " << ddq << "\n";

    	os << "fext = " << fext << ", tauext = " << tauext << "\n";

    	os << "sw = " << sw << ", sv = " << sv << "\n";
    	os << "Ivv = " << Ivv << "\n";
    	os << "Iwv = " << Iwv << "\n";
    	os << "Iww = " << Iww << "\n";
    	os << "cv = " << cv << ", cw = " << cw << "\n";
    	os << "pf = " << pf << ", ptau = " << ptau << "\n";
    	os << "hhv = " << hhv << ", hhw = " << hhw << "\n";
    	os << "uu = " << uu << ", dd = " << dd << "\n";

    	os << std::endl;
	}
}