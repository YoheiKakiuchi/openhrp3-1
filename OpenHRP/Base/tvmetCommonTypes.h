// -*- mode: c++; indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*-

#ifndef TVMET_COMMON_TYPES_H_INCLUDED
#define TVMET_COMMON_TYPES_H_INCLUDED

//---- needed for preventing a compile error in VC++ ----
#include <iostream>

#ifdef _WIN32
#pragma warning( disable : 4251 4275 4661 )
#undef min
#undef max
#endif

//------------------------------------------------------

#ifdef __QNX__
#include <cmath>
using std::size_t;
using std::sin;
using std::cos;
using std::sqrt;
using std::fabs;
using std::acos;
using std::asin;
using std::atan2;
#endif

#include <tvmet/Matrix.h>
#include <tvmet/Vector.h>

namespace OpenHRP
{
	typedef tvmet::Matrix<double, 3, 3> matrix33;
	typedef tvmet::Vector<double, 3> vector3;
}


#endif
