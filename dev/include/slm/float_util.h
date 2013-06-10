#ifndef SLMATH_FLOAT_UTIL_H
#define SLMATH_FLOAT_UTIL_H

#include <slm/slmath_pp.h>

SLMATH_BEGIN()

/**
 * \defgroup float_util Scalar helper functions.
 * @ingroup slm
 */
/*@{*/

/**
 * Returns cotangent of z.
 * @param z tan(z) must be != 0
 * @return 1/tan(z)
 */
float	cot( float z );

/**
 * Returns both sin and cos for the angle.
 * @param a Angle in radians.
 * @param sina Sine of angle a.
 * @param cosa Cosine of angle a.
 */
void	sincos( float a, float* sina, float* cosa );

/** 
 * Returns value stepped to specified edge, so that 0 is returned for values below the edge.
 * @param edge The edge value.
 * @param v The source value.
 * @return 0 if v<edge, otherwise 1.
 */
float	step( float edge, float v );

/** 
 * Returns value smoothly interpolated between two edges.
 * Useful for smooth transitions.
 * @param edge0 The lower edge value.
 * @param edge1 The upper edge value.
 * @param v The source value.
 * @return 0 if v<edge0, 1 if v>edge1, and Hermite interpolated value (0,1) in between edge0 and edge1.
 */
float	smoothstep( float edge0, float edge1, float v );

/** 
 * Limits value to specified range.
 * @param v Source value.
 * @param vmax Upper limit, inclusive.
 * @param vmin Lower limit, inclusive.
 * @return Source value limited between [vmin,vmax].
 */
float	clamp( float v, float vmin, float vmax );

/** 
 * Limits value to range [0,1].
 * @param v Source value.
 * @return Source value limited between [0,1].
 */
float	saturate( float v );

/**
 * Returns distance to the origin.
 * @param v The point on a line.
 * @return Absolute of the value.
 */
float	length( float v );

/**
 * Returns true if the floating point value is in valid range.
 * @param v Value to check.
 * @return true if -FLT_MAX <= v <= FLT_MAX
 */
bool	check( float v );

/**
 * Returns true if all the floating point values are in valid range.
 * @param v Values to check.
 * @param n Number of values to check.
 * @return true if for each v[i] -FLT_MAX <= v[i] <= FLT_MAX
 */
bool	check( const float* v, size_t n );

/**
 * Returns minimum of two values.
 */
float	min( float a, float b );

/**
 * Returns maximum of two values.
 */
float	max( float a, float b );

/** 
 * Converts angle from degrees to radians.
 * @param deg Angle in degrees.
 * @return Angle in radians.
 */
float	radians( float deg );

/** 
 * Converts angle from radians to degrees.
 * @param rad Angle in radians.
 * @return Angle in degrees.
 */
float	degrees( float rad );

/**
 * Calculates 1D Gaussian blur filter kernel with specified size.
 * @param s Width of the kernel. Radius of the kernel is (s-1)/2. s must be at least 1.
 * @param o Standard deviation. Pass in 0.f to use default formula: ((s-1)*0.5 - 1)*0.3 + 0.8.
 * @param kernel [out] Array of size [s] receiving filter weights
 */
void	getGaussianBlurKernel1D( int s, float o, float* kernel );

/**
 * Calculates 2D Gaussian blur filter kernel with specified size.
 * @param s Width of the kernel. Radius of the kernel is (s-1)/2. s must be at least 1.
 * @param o Standard deviation. Pass in 0.f to use default formula: ((s-1)*0.5 - 1)*0.3 + 0.8.
 * @param kernel [out] Array of size [s*s] receiving filter weights
 */
void	getGaussianBlurKernel2D( int s, float o, float* kernel );

/** 
 * Converts angle from degrees to radians.
 * @param deg Angle in degrees.
 * @return Angle in radians.
 */
float	radians( float deg );

/** 
 * Converts angle from radians to degrees.
 * @param rad Angle in radians.
 * @return Angle in degrees.
 */
float	degrees( float rad );

/** 
 * Returns linear blend between two values. Formula is x*(1-a)+y*a.
 * @param x The first value.
 * @param y The second value.
 * @param a The blend factor.
 * @return x*(1-a)+y*a
 */
float	mix( float x, float y, float a );

/** 
 * Returns distance between two values. 
 * @param p0 The first point vector.
 * @param p1 The second point vector.
 * @return Distance between p0 and p1, i.e. length(p1-p0).
 */
float	distance( float p0, float p1 );

/**
 * Returns value with components clamped between [min,max].
 */
float	clamp( float v, float min, float max );

/**
 * Returns value with components clamped between [0,1].
 */
float	saturate( float v );

// int/size_t support functions
#ifndef SWIG

/**
 * Returns minimum of two values.
 */
int		min( int a, int b );

/**
 * Returns maximum of two values.
 */
int		max( int a, int b );

/** 
 * Limits value to specified range.
 * @param v Source value.
 * @param vmax Upper limit, inclusive.
 * @param vmin Lower limit, inclusive.
 * @return Source value limited between [vmin,vmax].
 */
int		clamp( int v, int vmin, int vmax );

/**
 * Returns minimum of two values.
 */
size_t	min( size_t a, size_t b );

/**
 * Returns maximum of two values.
 */
size_t	max( size_t a, size_t b );

/** 
 * Limits value to specified range.
 * @param v Source value.
 * @param vmax Upper limit, inclusive.
 * @param vmin Lower limit, inclusive.
 * @return Source value limited between [vmin,vmax].
 */
size_t	clamp( size_t v, size_t vmin, size_t vmax );
#endif

/*@}*/

#include <slm/float_util.inl>

SLMATH_END()

#endif

// This file is part of 'slm' C++ library. Copyright (C) 2009 Jani Kajala (kajala@gmail.com). See http://sourceforge.net/projects/slm/
