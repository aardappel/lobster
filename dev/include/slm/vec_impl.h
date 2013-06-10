#ifndef SLMATH_VEC_TEMPL_FUNC_H
#define SLMATH_VEC_TEMPL_FUNC_H

#include <slm/float_util.h>

SLMATH_BEGIN()

/**
 * \defgroup mat_util Matrix class and helper functionality.
 * @ingroup slm
 */

/**
 * \defgroup vec_util Vector classes and helper functionality.
 * @ingroup slm
 */

/*
 * Returns true if all the values are in valid range.
 * Calls check(v) for each value.
 * @param v Values to check.
 * @param n Number of values to check.
 * @return true if none of v is NaN value.
 */
template <class T> inline bool vecCheck( const T& v )
{
	for ( size_t i = 0 ; i < T::SIZE ; ++i )
		if ( !check(v[i]) )
			return false;
	return true;
}

/*
 * Returns vector with components clamped between [min,max].
 * Assumes that class T has SIZE constant defined as number of components.
 */
template <class T> inline T vecClamp( const T& v, const T& min, const T& max )
{
	T v1;
	for ( size_t i = 0 ; i < T::SIZE ; ++i )
		v1[i] = v[i] > max[i] ? max[i] : (v[i] < min[i] ? min[i] : v[i]);
	return v1;
}

/*
 * Returns vector with components clamped between [0,1].
 * Assumes that class T has SIZE constant defined as number of components.
 */
template <class T> inline T vecSaturate( const T& v )
{
	assert( check(v) );
	T v1;
	for ( size_t i = 0 ; i < T::SIZE ; ++i )
		v1[i] = v[i] > 1.f ? 1.f : (v[i] < 0.f ? 0.f : v[i]);
	return v1;
}

/*
 * Returns linear blend between two values. Formula is x*(1-a)+y*a.
 * @param x The first value.
 * @param y The second value.
 * @param a The blend factor.
 * @return x*(1-a)+y*a
 */
template <class T> inline T vecMix( const T& x, const T& y, float a )
{
	return x + (y-x)*a;
}

SLMATH_END()

#endif // SLMATH_VEC_TEMPL_FUNC_H

// This file is part of 'slm' C++ library. Copyright (C) 2009 Jani Kajala (kajala@gmail.com). See http://sourceforge.net/projects/slm/
