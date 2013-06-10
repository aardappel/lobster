#include <slm/vec2.h>

SLMATH_BEGIN()

void vec2::normalize()
{
	float len = length( *this );
	SLMATH_VEC_ASSERT( len >= FLT_MIN );
	*this *= 1.f / len;
}

vec2 normalize( const vec2& v )
{
	assert( check(v) );

	float len = length(v);
	assert( len >= FLT_MIN );
	
	float invlen = 1.f / len;
	vec2 res( v.x*invlen, v.y*invlen );
	assert( check(res) );
	return res;
}

vec2 rotate( const vec2& v, float a )
{
	float s,c;
	sincos( -a, &s, &c );
	return vec2(v.x * c + v.y * s, v.y * c - v.x * s);
}

SLMATH_END()

// This file is part of 'slm' C++ library. Copyright (C) 2009 Jani Kajala (kajala@gmail.com). See http://sourceforge.net/projects/slm/
