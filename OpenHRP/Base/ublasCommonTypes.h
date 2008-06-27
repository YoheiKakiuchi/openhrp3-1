// -*- mode: c++; indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*-
#ifndef UBLAS_COMMON_TYPES_H_INCLUDED
#define UBLAS_COMMON_TYPES_H_INCLUDED

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/vector_proxy.hpp>
#include <boost/numeric/ublas/io.hpp>


namespace OpenHRP {
    
    namespace ublas = boost::numeric::ublas;
    
    typedef boost::numeric::ublas::matrix<double, ublas::column_major> dmatrix;
	typedef ublas::bounded_matrix<double, 3, 3, ublas::column_major> dmatrix33;
	typedef ublas::bounded_matrix<double, 6, 6, ublas::column_major> dmatrix66;
    typedef ublas::zero_matrix<double> dzeromatrix;
    typedef ublas::identity_matrix<double> didentity;

    typedef boost::numeric::ublas::vector<double> dvector;
	typedef ublas::bounded_vector<double, 3> dvector3;
	typedef ublas::bounded_vector<double, 6> dvector6;
    typedef ublas::zero_vector<double> dzerovector;
	typedef ublas::unit_vector<double> dunit;
};


#endif