#ifndef SLMATH_VEC2_H
#define SLMATH_VEC2_H

#include <slm/vec_impl.h>

SLMATH_BEGIN()

/**
 * 2-vector.
 *
 * Note naming convention: This class is starting with small letter since 
 * it is NOT initialized by the default constructor, much like int, float, etc. types.
 *
 * @ingroup vec_util
 */
class vec2
{
public:
	/** Constants related to the class. */
	enum Constants
	{
		/** Number of dimensions in this vector. */
		SIZE = 2,
	};

	/** The first component of the vector. */
	float x;

	/** The second component of the vector. */
	float y;

	/** 
	 * Constructs undefined 2-vector. In _DEBUG build the vector is initialized to NaN.
	 */
	vec2();

	/** 
	 * Constructs vector with all components set to the same value.
	 * @param v Each of the vector components will be set to this value.
	 */
	explicit vec2( float v );

	/** 
	 * Constructs the vector from scalars. 
	 * @param x0 The vector X-component (0)
	 * @param y0 The vector Y-component (1)
	 */
	vec2( float x0, float y0 );

	/** Sets vector value. */
	void		set( float x0, float y0 );

	/** Component wise addition. */
	vec2&		operator+=( const vec2& o );

	/** Component wise subtraction. */
	vec2&		operator-=( const vec2& o );

	/** Component wise multiplication. */
	vec2&		operator*=( const vec2& o );

	/** Component wise division. */
	vec2&		operator/=( const vec2& o );

	/** Component wise scalar subtract */
    vec2&        operator-=( float s );

	/** Component wise scalar addition */
	vec2&        operator+=( float s );

	/** Component wise scalar multiplication. */
	vec2&		operator*=( float s );

	/** Component wise scalar division. */
	vec2&		operator/=( float s );

	/** Returns component at specified index. */
	float&		operator[]( size_t i )				{SLMATH_VEC_ASSERT( i < 3 ); return (&x)[i];}

	/** Returns pointer to the first float. */
	float*		begin()								{return &x;}

	/** Returns pointer to one beyond the last float. */
	float*		end()								{return &x+SIZE;}

	/** Normalizes this vector. */
	void		normalize();

	/** Component wise multiplication. */
	vec2		operator*( const vec2& o ) const;

	/** Component wise division. */
	vec2		operator/( const vec2& o ) const;

	/** Component wise addition. */
	vec2		operator+( const vec2& o ) const;

	/** Component wise subtraction. */
	vec2		operator-( const vec2& o ) const;

	/** Component wise subtraction. */
	vec2		operator-() const;

	/** Component wise scalar multiplication. */
	vec2		operator*( float s ) const;

	/** Component wise scalar division. */
	vec2		operator/( float s ) const;

	/** Returns component at specified index. */
	const float& operator[]( size_t i ) const			{SLMATH_VEC_ASSERT( i < 3 ); return (&x)[i];}

	/** Returns true if vectors are bitwise equal. */
	bool		operator==( const vec2& o ) const;

	/** Returns true if vectors are bitwise inequal. */
	bool		operator!=( const vec2& o ) const;

	/** Returns const pointer to the first float. */
	const float*	begin() const	{return &x;}

	/** Returns const pointer to one beyond the last float. */
	const float*	end() const		{return &x+SIZE;}
};


/** 
 * Component wise scalar multiplication. 
 * @ingroup vec_util
 */
vec2	operator*( float s, const vec2& v );

/** 
 * Returns length of the vector. 
 * @ingroup vec_util
 */
float	length( const vec2& v );

/** 
 * Returns dot product (inner product) of two vectors. 
 * @param a The first vector
 * @param b The second vector
 * @return a dot b
 * @ingroup vec_util
 */
float	dot( const vec2& a, const vec2& b );

/** 
 * Returns the vector normalized to unit (1) length.
 * @ingroup vec_util
 */
vec2	normalize( const vec2& v );

/** 
 * Returns true if all components of the vector are valid numbers. 
 * @ingroup vec_util
 */
bool	check( const vec2& v );

/**
 * Returns component wise maximum of two vectors.
 * @ingroup vec_util
 */
vec2	max( const vec2& a, const vec2& b );

/**
 * Returns component wise minimum of two vectors.
 * @ingroup vec_util
 */
vec2	min( const vec2& a, const vec2& b );

/**
 * Returns component wise absolute of the vector.
 * @ingroup vec_util
 */
vec2	abs( const vec2& v );

/** 
 * Returns linear blend between two values. Formula is x*(1-a)+y*a.
 * @param x The first value.
 * @param y The second value.
 * @param a The blend factor.
 * @return x*(1-a)+y*a
 * @ingroup vec_util
 */
vec2	mix( const vec2& x, const vec2& y, float a );

/** 
 * Returns distance between two values. 
 * @param p0 The first point vector.
 * @param p1 The second point vector.
 * @return Distance between p0 and p1, i.e. length(p1-p0).
 * @ingroup vec_util
 */
float	distance( const vec2& p0, const vec2& p1 );

/**
 * Returns value with components clamped between [min,max].
 * @ingroup vec_util
 */
vec2	clamp( const vec2& v, const vec2& min, const vec2& max );

/**
 * Returns value with components clamped between [0,1].
 * @ingroup vec_util
 */
vec2	saturate( const vec2& v );

/**
 * Returns true if all components are valid numbers.
 * @ingroup vec_util
 */
bool	check( const vec2& v );

/**
 * Returns negated vector. This function is useful for scripting where overloaded operators are not available.
 * @ingroup vec_util
 */
vec2	neg( const vec2& a );

/**
 * Adds two vectors together. This function is useful for scripting where overloaded operators are not available.
 * @ingroup vec_util
 */
vec2	add( const vec2& a, const vec2& b );

/**
 * Subtracts two vectors together. This function is useful for scripting where overloaded operators are not available.
 * @ingroup vec_util
 */
vec2	sub( const vec2& a, const vec2& b );

/**
 * Multiplies vector by scalar. This function is useful for scripting where overloaded operators are not available.
 * @ingroup vec_util
 */
vec2	mul( const vec2& a, float b );

/**
 * Multiplies vector by scalar. This function is useful for scripting where overloaded operators are not available.
 * @ingroup vec_util
 */
vec2	mul( float b, const vec2& a );

/**
 * Rotates vector v by specified angle a (in radians).
 * @ingroup vec_util
 */
vec2	rotate( const vec2& v, float a );

#include <slm/vec2.inl>

SLMATH_END()

#endif

// This file is part of 'slm' C++ library. Copyright (C) 2009 Jani Kajala (kajala@gmail.com). See http://sourceforge.net/projects/slm/
