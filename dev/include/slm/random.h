#ifndef SLMATH_RANDOM_H
#define SLMATH_RANDOM_H

#include <slm/float_util.h>
#include <slm/vec3.h>
#include <slm/mtrnd.h>

SLMATH_BEGIN()

/**
 * \defgroup random_util Random number and distribution generation.
 * @ingroup slm
 */
/*@{*/

/**
 * Random number value. Generator based on Mersenne Twister.
 *
 * Note naming convention: This class is starting with small r since 
 * it is not initialized to any specific value by the default constructor.
 */
class random
{
public:
	/**
	 * Initializes random number generator with current time.
	 */
	random();

	/**
	 * Initializes random number generator with specified seed value.
	 */
	random( int seed );

	/*
	 * Returns random floating point number in range [0,1].
	 */
	float	randomFloat();

	/*
	 * Returns random floating point number in half-open range [0,1).
	 */
	float	randomFloatOpen();

	/*
	 * Returns random integer.
	 */
	int		randomInt();

	/**
	 * Returns the seed value which was used to initialize this random number generator.
	 */
	int		seed() const;

private:
	MersenneTwist	m_mt;
	int				m_seed;
};

/** 
 * Returns random floating point number in range [0,1].
 * @param seed random number generator
 */
float	randomFloat( random& seed );

/** 
 * Returns random floating point number in half-open range [0,1).
 * @param seed random number generator
 */
float	randomFloatOpen( random& seed );

/**
 * Returns random integer.
 * @param seed random number generator
 */
int		randomInt( random& seed );

/** 
 * Returns signed random floating point number in range [-1,1].
 * @param seed random number generator
 */
float	randomFloatSigned( random& seed );

/** 
 * Returns uniformly distributed random point on unit sphere. 
 * @param seed random number generator
 */
vec3	randomPointOnSphere( random& seed );

/** 
 * Returns uniformly distributed random point inside unit sphere. 
 * @param seed random number generator
 */
vec3	randomPointInSphere( random& seed );

/**
 * Produces n stratified samples on hemisphere surface.
 * @param normal Hemisphere direction. Must be unit vector.
 * @param angle Cone angle to limit hemisphere (pass pi/2 to sample on full hemisphere surface).
 * @param n Number of samples to generate.
 * @param out [out] Receives generated samples.
 * @param seed [inout] random number generator.
 */
void	randomSamplePointsOnHemisphere( const vec3& normal, float angle, size_t n, vec3* out, random& seed );

/**
 * Produces n stratified samples within hemisphere volume.
 * @param normal Hemisphere direction. Must be unit vector.
 * @param angle Cone angle to limit hemisphere (pass pi/2 to sample within full hemisphere volume).
 * @param n Number of samples to generate.
 * @param out [out] Receives generated samples.
 * @param seed [inout] random number generator.
 */
void	randomSamplePointsInHemisphere( const vec3& normal, float angle, size_t n, vec3* out, random& seed );

/*@}*/

#include <slm/random.inl>

SLMATH_END()

#endif

// This file is part of 'slm' C++ library. Copyright (C) 2009 Jani Kajala (kajala@gmail.com). See http://sourceforge.net/projects/slm/
