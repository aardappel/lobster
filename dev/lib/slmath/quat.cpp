#include <slm/quat.h>

#define UNIT_QUATERNION_EPS 1e-3f
#define ASSERT_IS_UNIT(Q) SLMATH_VEC_ASSERT( fabsf(norm(Q)-1.f) < UNIT_QUATERNION_EPS )

SLMATH_BEGIN()

void quat::normalize()
{
	float n = norm(*this);
	SLMATH_VEC_ASSERT( n > FLT_MIN );
	*this *= 1.f/n;
}

bool check( const quat& v )
{
	return 
		v.x >= -FLT_MAX && v.x <= FLT_MAX &&
		v.y >= -FLT_MAX && v.y <= FLT_MAX &&
		v.z >= -FLT_MAX && v.z <= FLT_MAX &&
		v.w >= -FLT_MAX && v.w <= FLT_MAX;
}

quat::quat( float a, const vec3& v )
{
	SLMATH_VEC_ASSERT( fabsf(length(v)-1.f) < 1e-3f ); // axis must be normalized before calling this

	float s,c;
	sincos( a*.5f, &s, &c );
	x = v.x * s;
	y = v.y * s;
	z = v.z * s;
	w = c;

	SLMATH_VEC_ASSERT( check(*this) );
}

quat::quat( const mat4& m )
{
	const float trace = m[0][0] + m[1][1] + m[2][2];

	if ( trace > 0.f )
	{
		float root = sqrtf( trace + 1.f );
		w = root * .5f;
		SLMATH_VEC_ASSERT( fabsf(root) >= FLT_MIN );
		root = .5f / root;
        x = root * (m[1][2] - m[2][1]);
        y = root * (m[2][0] - m[0][2]);
        z = root * (m[0][1] - m[1][0]);
	}
	else
	{
		register size_t i = 0;
		if ( m[1][1] > m[0][0] )
			i = 1;
		if ( m[2][2] > m[i][i] )	
			i = 2;
		const size_t j = (i == 2 ? 0 : i+1);
		const size_t k = (j == 2 ? 0 : j+1);

		float root = sqrtf( m[i][i] - m[j][j] - m[k][k] + 1.f );
		float* v = &x;
		v[i] = root * .5f;
		SLMATH_VEC_ASSERT( fabsf(root) >= FLT_MIN );
		root = .5f / root;
		v[j] = root * (m[i][j] + m[j][i]);
		v[k] = root * (m[i][k] + m[k][i]);
		w = root * (m[j][k] - m[k][j]);
	}

	SLMATH_VEC_ASSERT( check(*this) );
}

quat quat::operator*( const quat& o ) const
{
	quat q;
	q.w = (w*o.w - x*o.x - y*o.y - z*o.z);
	q.x = (w*o.x + x*o.w + y*o.z - z*o.y);
	q.y = (w*o.y - x*o.z + y*o.w + z*o.x);
	q.z = (w*o.z + x*o.y - y*o.x + z*o.w);

	SLMATH_VEC_ASSERT( check(q) );
	return q;
}

float normSquared( const quat& q )
{
	return dot(q.xyzw(),q.xyzw());
}

float norm( const quat& q )
{
	return sqrtf( dot(q.xyzw(),q.xyzw()) );
}

quat normalize( const quat& q )
{
	float n = norm(q);
	SLMATH_VEC_ASSERT( n > FLT_MIN );
	return q * (1.f/n);
}

quat inverse( const quat& q )
{
	float n = norm(q);
	SLMATH_VEC_ASSERT( n > FLT_MIN );
	n = 1.f/n; 
	return quat( -q.x*n, -q.y*n, -q.z*n, q.w*n );
}

quat exp( const quat& q )
{
	ASSERT_IS_UNIT( q ); // normalize(q) before use this function

	const float a = length( q.xyzw().xyz() );
	quat r( q.x, q.y, q.z, cosf(a) );

	if ( a > FLT_MIN )
	{
		const float c = sinf(a) / a;
		r.x *= c;
		r.y *= c;
		r.z *= c;
	}
	return r;
}

quat log( const quat& q )
{
	ASSERT_IS_UNIT( q ); // normalize(q) before use this function

	quat r( q.x, q.y, q.z, 0.f );
	
	if ( q.w < 1.f && -q.w < 1.f )
	{
		const float a = acosf( q.w );
		const float s = sinf(a);

		if ( fabsf(s) > FLT_MIN )
		{
			const float c = a/s;
			r.x *= c;
			r.y *= c;
			r.z *= c;
		}
	}
	return r;
}

quat pow( const quat& q, float n )
{
	ASSERT_IS_UNIT( q ); // normalize(q) before use this function

	const float a = length( q.xyzw().xyz() );
	quat r( q.x, q.y, q.z, cosf(a*n) );
	
	if ( a > FLT_MIN )
	{
		const float c = sinf(a*n)/a;
		r.x *= c;
		r.y *= c;
		r.z *= c;
	}
	return r;
}

quat slerp( const quat& a, const quat& b, float u )
{
	ASSERT_IS_UNIT( a ); // normalize(q) before use this function
	ASSERT_IS_UNIT( b ); // normalize(q) before use this function

	const float c = clamp( dot(a,b), -1.f, 1.f );
	const float angle = acosf(c);
	const float s = sinf(angle);

	if ( s < FLT_MIN )
	{
		return a;
	}
	else
	{
		const float invs = 1.f / s;
		const float c0 = invs * sinf( (1.f-u)*angle );
		const float c1 = invs * sinf( u*angle );
		return a*c0 + b*c1;
	}
}

void toAngleAxis( const quat& q, float* angle, vec3* axis )
{
	ASSERT_IS_UNIT( q ); // normalize(q) before use this function

	*angle = 2.f * acosf(q.w);
	const float s = sqrtf(1.f-q.w*q.w);
	
	if ( s <= FLT_MIN ) 
	{ 
		axis->x = q.x;
		axis->y = q.y;
		axis->z = q.z;
	} 
	else 
	{
		const float is = 1.f/s;
		axis->x = q.x * is;
		axis->y = q.y * is;
		axis->z = q.z * is;
	}
}

SLMATH_END()

// This file is part of 'slm' C++ library. Copyright (C) 2009 Jani Kajala (kajala@gmail.com). See http://sourceforge.net/projects/slm/
