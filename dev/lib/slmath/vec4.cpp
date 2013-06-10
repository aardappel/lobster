#include <slm/vec4.h>

SLMATH_BEGIN()

void vec4::normalize()
{
	float len = length( *this );
	SLMATH_VEC_ASSERT( len >= FLT_MIN );
	*this *= 1.f / len;
}

vec4 normalize( const vec4& v )
{
	SLMATH_VEC_ASSERT( check(v) );

	float len = length(v);
	assert( len >= FLT_MIN );
	
	float invlen = 1.f / len;
	vec4 res( v.x*invlen, v.y*invlen, v.z*invlen, v.w*invlen );
	SLMATH_VEC_ASSERT( check(res) );
	return res;
}

SLMATH_END()

// This file is part of 'slm' C++ library. Copyright (C) 2009 Jani Kajala (kajala@gmail.com). See http://sourceforge.net/projects/slm/
