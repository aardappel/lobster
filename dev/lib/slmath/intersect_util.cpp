#include <slm/intersect_util.h>

SLMATH_BEGIN()

intersectLineBox_Line::intersectLineBox_Line( const vec3& origin, const vec3& direction ) :
	o( origin ),
	d( direction ),
	inv_d( fabsf(direction.x) > FLT_MIN ? 1.f/direction.x : FLT_MAX, 
		   fabsf(direction.y) > FLT_MIN ? 1.f/direction.y : FLT_MAX, 
		   fabsf(direction.z) > FLT_MIN ? 1.f/direction.z : FLT_MAX ),
	signx( direction.x < 0.f ),
	signy( direction.y < 0.f ),
	signz( direction.z < 0.f )
{
}

bool intersectLineTri( const vec3& o, const vec3& d, const vec3& v0, const vec3& v1, const vec3& v2, float* t )
{
	const vec3	e1		= v1 - v0;
	const vec3	e2		= v2 - v0;
	const vec3	pvec	= cross( d, e2 );
	const float det		= dot( e1, pvec );
	if ( fabsf(det) <= FLT_MIN )
		return false;

	const float invdet	= 1.f / det;
	const vec3	tvec	= o - v0;
	const float u		= dot(tvec,pvec) * invdet;
	if ( 0.f > u || 1.f < u )
		return false;

	const vec3	qvec	= cross( tvec, e1 );
	const float v		= dot(d,qvec) * invdet;
	if ( 0.f > v || 1.f < u + v )
		return false;
	const float s		= dot(e2,qvec) * invdet;
	if ( s < 0.f || s >= 1.f )
		return false;

	if (t) 
		*t = s;

	assert( s >= 0.f && s <= 1.f );
	return true;
}

bool intersectLineBox( const intersectLineBox_Line& r, const vec3* boxminmax )
{
	const float t0 = 0.0f;
	const float t1 = 1.0f;

	const int* const sign = &r.signx;
	register float tmin, tmax, tymin, tymax, tzmin, tzmax;
	tmin = (boxminmax[sign[0]].x - r.o.x) * r.inv_d.x;
	tmax = (boxminmax[1-sign[0]].x - r.o.x) * r.inv_d.x;
	tymin = (boxminmax[sign[1]].y - r.o.y) * r.inv_d.y;
	tymax = (boxminmax[1-sign[1]].y - r.o.y) * r.inv_d.y;
	if ( (tmin > tymax) || (tymin > tmax) ) 
		return false;
	if (tymin > tmin)
		tmin = tymin;
	if (tymax < tmax)
		tmax = tymax;
	tzmin = (boxminmax[sign[2]].z - r.o.z) * r.inv_d.z;
	tzmax = (boxminmax[1-sign[2]].z - r.o.z) * r.inv_d.z;
	if ( (tmin > tzmax) || (tzmin > tmax) ) 
		return false;
	if (tzmin > tmin)
	tmin = tzmin;
	if (tzmax < tmax)
		tmax = tzmax;
	return ( (tmin < t1) && (tmax > t0) );
}

bool intersectLineBox( const vec3& o, const vec3& d, const vec3& boxmin, const vec3& boxmax )
{
	// these could be pre-init per line
	intersectLineBox_Line line( o, d );
	// these could be pre-init per box
	const vec3 boxminmax[2] = {boxmin, boxmax};
	return intersectLineBox( line, boxminmax );
}

SLMATH_END()

// This file is part of 'slm' C++ library. Copyright (C) 2009 Jani Kajala (kajala@gmail.com). See http://sourceforge.net/projects/slm/
