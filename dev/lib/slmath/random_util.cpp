#include <slm/random.h>
#include <slm/float_util.h>
#include <assert.h>

SLMATH_BEGIN()

// ----------------------------------------------------------------------------
// Local functions
// ----------------------------------------------------------------------------

/**
 * Makes transform frame of reference from normal vector.
 * Uses Gram-Schmidt orthonormalization.
 * @param z Z-axis of the frame. Must be unit vector.
 * @param xout [out] Receives X-axis.
 * @param yout [out] Receives Y-axis.
 */
static void makeFrame( const vec3& z, vec3& x, vec3& y )
{
	assert( fabs(length(z)-1.f) < 1e-6f );

	y = vec3(-z.z,z.x,-z.y);
	y = normalize( y - z*dot(z,y) );
	x = cross( z, y );
	
	assert( dot(x,y) < 1e-6f );
	assert( dot(y,z) < 1e-6f );
	assert( dot(x,z) < 1e-6f );
}

/**
 * Produces n stratified x,y pairs. Component z of output array is not touched.
 * @param n Number of pairs to output.
 * @param out [out] Receives output pairs.
 * @param seed [inout] Seed to random number generator.
 */
static void makeStratifiedPairs( size_t n, vec3* out, random& seed )
{
	const float nf = float(n);
	const float invn = 1.f/float(n);
	float f = 0.f;
    for ( size_t i = 0 ; i < n ; ++i )
    {
        out[i].x = f;
        out[i].y = f;
		f += invn;
    }

    for ( size_t i = 0 ; i < n ; ++i )
    {
        size_t j = size_t( seed.randomFloat()*nf );
		assert( j < n );
		float tmp = out[i].x;
		out[i].x = out[j].x;
		out[j].x = tmp;

        j = size_t( seed.randomFloat()*nf );
		assert( j < n );
		tmp = out[i].y;
		out[i].y = out[j].y;
		out[j].y = tmp;
    }
}

// ----------------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------------

vec3 randomPointOnSphere( random& seed )
{
	const float u = seed.randomFloat();
	const float u2 = u*u;
	const float b = randomFloatSigned(seed) * 3.1415926535897932384626433832795f;
	float sina, cosa;
	sincos( b, &sina, &cosa );
	const float d = sqrtf( 1.f - u2 );
	return vec3( d * cosa, d * sina, u );
}

vec3 randomPointInSphere( random& seed )
{
	const size_t n = 100;
	for ( size_t i = 0 ; i < n ; ++i )
	{
		const vec3 p( randomFloatSigned(seed), randomFloatSigned(seed), randomFloatSigned(seed) );
		if ( dot(p,p) < 1.f )
			return p;
	}

	// This puts too much weight on center but chance that this happen is 7.88861e-031 :)
	return randomPointOnSphere(seed) * randomFloatSigned(seed);
}

void randomSamplePointsOnHemisphere( const vec3& normal, float angle, size_t n, vec3* out, random& seed )
{
	assert( angle > 0.f && angle <= radians(90.f) );
	assert( n > 0 );
	assert( check(normal) );

    const float c = cosf( angle );

	vec3 v1, v2;
	makeFrame( normal, v1, v2 );
    makeStratifiedPairs( n, out, seed );

    for ( size_t i = 0 ; i < n ; ++i )
    {
		const float x1 = out[i].x;
		const float x2 = out[i].y;
		const float costheta2 = c * c + x1 * (1.f - c * c);
		const float costheta = sqrtf( costheta2 );
		const float sintheta = sqrtf( 1.0f - costheta2 );
		const float angle = 6.283185307179586476925286766559f * x2;
		const vec3 p0( sintheta*cosf(angle), sintheta*sinf(angle), costheta );

		out[i] = vec3( p0.x * v1 + p0.y * v2 + p0.z * normal );
    }
}

void randomSamplePointsInHemisphere( const vec3& normal, float angle, size_t n, vec3* out, random& seed )
{
	assert( angle > 0.f && angle <= radians(90.f) );
	assert( n > 0 );
	assert( check(normal) );

	randomSamplePointsOnHemisphere( normal, angle, n, out, seed );

	// randomize lengths according to volume distribution (plain *=randomFloat() would put more weight on center)
	for ( size_t i = 0 ; i < n ; ++i )
		out[i] *= length( randomPointInSphere(seed) );
}

SLMATH_END()

// This file is part of 'slm' C++ library. Copyright (C) 2009 Jani Kajala (kajala@gmail.com). See http://sourceforge.net/projects/slm/
