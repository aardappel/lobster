#ifndef SLMATH_QUAT_H
#define SLMATH_QUAT_H

#include <slm/mat4.h>

SLMATH_BEGIN()

/**
 * \defgroup quat_util Quaternion class and helper functionality.
 * @ingroup slm
 */

/**
 * Quaternion class.
 *
 * Note naming convention: This class is starting with small letter since 
 * it is NOT initialized by the default constructor, much like int, float, etc. types.
 *
 * @ingroup quat_util
 */
class quat
{
public:
	/** Constants related to the class. */
	enum Constants
	{
		/** Number of dimensions in quaternion interepreted as floats. */
		SIZE = 4,
	};

	/** The first component of the quaternion. */
	float x;

	/** The second component of the quaternion. */
	float y;

	/** The third component of the quaternion. */
	float z;

	/** The fourth component of the quaternion. */
	float w;

	/** Constructs undefined quaternion. In _DEBUG build the vector is initialized to NaN. */
	quat();

	/** 
	 * Constructs the quaternion from scalars.
	 * @param x0 The quaternion X-component (0)
	 * @param y0 The quaternion Y-component (1)
	 * @param z0 The quaternion Z-component (2)
	 * @param w0 The quaternion W-component (3)
	 */
	quat( float x0, float y0, float z0, float w0 );

	/** 
	 * Constructs the quaternion from angle-axis rotation.
	 * @param a Angle in radians.
	 * @param v Rotation axis.
	 */
	quat( float a, const vec3& v );

	/** 
	 * Constructs quaternion from rotation matrix.
	 */
	explicit quat( const mat4& m );

	/** Component-wise addition. */
	quat&		operator+=( const quat& o );
	
	/** Component-wise subtraction. */
	quat&		operator-=( const quat& o );
	
	/** Component-wise scalar multiplication. */
	quat&		operator*=( float s );
	
	/** Quaternion multiplication. */
	quat&		operator*=( const quat& o );

	/** Returns component at specified index. */
	float&		operator[]( size_t i );

	/** Returns pointer to the first float. */
	float*		begin()			{return &x;}

	/** Returns pointer to one beyond the last float. */
	float*		end()			{return &x+SIZE;}

	/** Normalizes this quaternion. */
	void		normalize();

	/** Quaternion multiplication. */
	quat		operator*( const quat& o ) const;

	/** Component wise addition. */
	quat		operator+( const quat& o ) const;

	/** Component wise subtraction. */
	quat		operator-( const quat& o ) const;

	/** Component wise negate. */
	quat		operator-() const;

	/** Component wise scalar multiplication. */
	quat		operator*( float s ) const;

	/** Returns component at specified index. */
	const float& operator[]( size_t i ) const;

	/** Returns true if the quaternions are bitwise equal. */
	bool		operator==( const quat& o ) const;

	/** Returns true if the quaternions are bitwise inequal. */
	bool		operator!=( const quat& o ) const;

	/** Returns quaternion represented as 4-vector. */
	const vec4&	xyzw() const;

	/** Returns const pointer to the first float. */
	const float*	begin() const	{return &x;}

	/** Returns const pointer to one beyond the last float. */
	const float*	end() const		{return &x+SIZE;}
};

/** 
 * Returns true if all components of the quaternion are valid numbers. 
 * @ingroup quat_util
 */
bool		check( const quat& q );

/** 
 * Returns dot product of two quaternions. 
 * @param a The first quaternion
 * @param b The second quaternion
 * @return a dot b
 * @ingroup quat_util
 */
float		dot( const quat& a, const quat& b );

/** 
 * Returns squared norm of the quaternion. 
 * @ingroup quat_util
 */
float		normSquared( const quat& q );

/** 
 * Returns norm of the quaternion. 
 * @ingroup quat_util
 */
float		norm( const quat& q );

/** 
 * Returns normalized quaternion.
 * @ingroup quat_util
 */
quat		normalize( const quat& q );

/** 
 * Returns conjugate of quaternion. 
 * @ingroup quat_util
 */
quat		conjugate( const quat& q );

/** 
 * Returns inverse of quaternion. 
 * @ingroup quat_util
 */
quat		inverse( const quat& q );

/** 
 * Returns exponent of unit quaternion. 
 * @ingroup quat_util
 */
quat		exp( const quat& q );

/** 
 * Returns logarithm of unit quaternion. 
 * @ingroup quat_util
 */
quat		log( const quat& q );

/** 
 * Returns unit quaternion raised to power n. 
 * @ingroup quat_util
 */
quat		pow( const quat& q, float n );

/** 
 * Returns spherical linear interpolation for unit quaternions. 
 * @param a The quaternion when u=0.
 * @param b The quaternion when u=1.
 * @param u Interpolation phase [0,1]
 * @return Interpolated quaternion.
 * @ingroup quat_util
 */
quat		slerp( const quat& a, const quat& b, float u );

/**
 * Converts unit quaternion to angle-axis representation.
 * @param q Input quaternion.
 * @param angle [out] Receives rotation angle in radians.
 * @param axis [out] Receives rotation axis.
 * @ingroup quat_util
 */
void		toAngleAxis( const quat& q, float* angle, vec3* axis );

#include <slm/quat.inl>

SLMATH_END()

#endif

// This file is part of 'slm' C++ library. Copyright (C) 2009 Jani Kajala (kajala@gmail.com). See http://sourceforge.net/projects/slm/
