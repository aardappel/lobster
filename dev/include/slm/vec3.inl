inline vec3::vec3() 
{
#ifdef _DEBUG
	const int nan = 0x7F800001;
	x = y = z = *(const float*)&nan;
#endif
}

inline vec3::vec3( float v )
{
	SLMATH_VEC_ASSERT( check(v) );
	z = y = x = v;
}

inline vec3::vec3( float x0, float y0, float z0 ) : 
	x(x0), y(y0), z(z0)
{
	SLMATH_VEC_ASSERT( check(*this) );
}

inline vec3::vec3( const vec2& v0, float z0 ) : 
	x(v0.x), y(v0.y), z(z0)
{
	SLMATH_VEC_ASSERT( check(*this) );
}

inline void vec3::set( float x0, float y0, float z0 )
{
	x=x0;
	y=y0;
	z=z0;
	SLMATH_VEC_ASSERT( check(*this) );
}

inline vec3& vec3::operator+=( const vec3& o )
{
	x+=o.x;
	y+=o.y;
	z+=o.z;
	SLMATH_VEC_ASSERT( check(*this) );
	return *this;
}

inline vec3& vec3::operator-=( const vec3& o )
{
	x-=o.x;
	y-=o.y;
	z-=o.z;
	SLMATH_VEC_ASSERT( check(*this) );
	return *this;
}

inline vec3& vec3::operator*=( float s )
{
	x*=s;
	y*=s;
	z*=s;
	SLMATH_VEC_ASSERT( check(*this) );
	return *this;
}

inline vec3& vec3::operator*=( const vec3& o )
{
	x *= o.x;
	y *= o.y;
	z *= o.z;
	SLMATH_VEC_ASSERT( check(*this) );
	return *this;
}

inline vec3& vec3::operator/=( const vec3& o )
{
	SLMATH_VEC_ASSERT( fabsf(o.x) > FLT_MIN );
	SLMATH_VEC_ASSERT( fabsf(o.y) > FLT_MIN );
	SLMATH_VEC_ASSERT( fabsf(o.z) > FLT_MIN );
	x /= o.x;
	y /= o.y;
	z /= o.z;
	SLMATH_VEC_ASSERT( check(*this) );
	return *this;
}

inline vec3& vec3::operator/=( float s )
{
	SLMATH_VEC_ASSERT( fabsf(s) >= FLT_MIN ); // s must be != 0
	const float sinv = 1.f/s;
	x *= sinv;
	y *= sinv;
	z *= sinv;
	SLMATH_VEC_ASSERT( check(*this) );
	return *this;
}

inline vec3 vec3::operator/( float s ) const
{
	SLMATH_VEC_ASSERT( check(*this) );
	SLMATH_VEC_ASSERT( fabsf(s) >= FLT_MIN ); // s must be != 0
	const float sinv = 1.f/s;
	return vec3( x*sinv, y*sinv, z*sinv );
}

inline vec2& vec3::xy()
{
	return *(vec2*)&x;
}

inline vec3 vec3::operator*( const vec3& o ) const
{
	SLMATH_VEC_ASSERT( check(o) );
	SLMATH_VEC_ASSERT( check(*this) );
	return vec3(x*o.x,y*o.y,z*o.z);
}

inline vec3 vec3::operator/( const vec3& o ) const
{
	SLMATH_VEC_ASSERT( check(*this) );
	SLMATH_VEC_ASSERT( fabsf(o.x) > FLT_MIN );
	SLMATH_VEC_ASSERT( fabsf(o.y) > FLT_MIN );
	SLMATH_VEC_ASSERT( fabsf(o.z) > FLT_MIN );
	return vec3(x/o.x,y/o.y,z/o.z);
}

inline vec3 vec3::operator+( const vec3& o ) const
{
	SLMATH_VEC_ASSERT( check(o) );
	SLMATH_VEC_ASSERT( check(*this) );
	return vec3(x+o.x,y+o.y,z+o.z);
}

inline vec3 vec3::operator-( const vec3& o ) const
{
	SLMATH_VEC_ASSERT( check(o) );
	SLMATH_VEC_ASSERT( check(*this) );
	return vec3(x-o.x,y-o.y,z-o.z);
}

inline vec3 vec3::operator-() const
{
	SLMATH_VEC_ASSERT( check(*this) );
	return vec3(-x,-y,-z);
}

inline vec3 vec3::operator*( float s ) const
{
	SLMATH_VEC_ASSERT( check(s) );
	SLMATH_VEC_ASSERT( check(*this) );
	return vec3(x*s,y*s,z*s);
}

inline bool vec3::operator==( const vec3& o ) const
{
	SLMATH_VEC_ASSERT( check(o) );
	SLMATH_VEC_ASSERT( check(*this) );
	return x==o.x && y==o.y && z==o.z;
}

inline bool vec3::operator!=( const vec3& o ) const
{
	return !(*this == o);
}

inline const vec2& vec3::xy() const
{
	return *(const vec2*)&x;
}

inline vec3 operator*( float s, const vec3& v )
{
	SLMATH_VEC_ASSERT( check(s) );
	SLMATH_VEC_ASSERT( check(v) );
	return vec3(v.x*s,v.y*s,v.z*s);
}

inline float length( const vec3& v )					
{
	SLMATH_VEC_ASSERT( check(v) );

	float res = sqrtf( v.x*v.x + v.y*v.y + v.z*v.z );

	SLMATH_VEC_ASSERT( res >= 0.f && res <= FLT_MAX );
	return res;
}

inline float dot( const vec3& a, const vec3& b )		
{
	SLMATH_VEC_ASSERT( check(a) );
	SLMATH_VEC_ASSERT( check(b) );

	float res = a.x*b.x + a.y*b.y + a.z*b.z;

	SLMATH_VEC_ASSERT( res >= -FLT_MAX && res <= FLT_MAX );
	return res;
}

inline vec3 max( const vec3& a, const vec3& b )
{
	return vec3( max(a[0],b[0]), max(a[1],b[1]), max(a[2],b[2]) );
}

inline vec3 min( const vec3& a, const vec3& b )
{
	return vec3( min(a[0],b[0]), min(a[1],b[1]), min(a[2],b[2]) );
}

inline vec3 abs( const vec3& v )
{
	return vec3( fabsf(v[0]), fabsf(v[1]), fabsf(v[2]) );
}

inline vec3 mix( const vec3& x, const vec3& y, float a )
{
	return vecMix( x, y, a );
}

inline float distance( const vec3& p0, const vec3& p1 )
{
	return length( p0 - p1 );
}

inline vec3 clamp( const vec3& v, const vec3& min, const vec3& max )
{
	return vecClamp( v, min, max );
}

inline vec3 saturate( const vec3& v )
{
	return vecSaturate( v );
}

inline bool check( const vec3& v )
{
	return v.x<=FLT_MAX && v.x>=-FLT_MAX && v.y<=FLT_MAX && v.y>=-FLT_MAX && v.z<=FLT_MAX && v.z>=-FLT_MAX;
}

inline vec3 neg( const vec3& a )
{
	return -a;
}

inline vec3 add( const vec3& a, const vec3& b )
{
	return a+b;
}

inline vec3 sub( const vec3& a, const vec3& b )
{
	return a-b;
}

inline vec3 mul( const vec3& a, float b )
{
	return a*b;
}

inline vec3 mul( float b, const vec3& a )
{
	return a*b;
}

inline vec3 cross( const vec3& a, const vec3& b )		
{
	vec3 res( a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x );
	SLMATH_VEC_ASSERT( check(res) );
	return res;
}

inline vec3& vec3::operator-=( float s )
{
	x -= s; 
	y -= s; 
	z -= s; 
	return *this;
}

inline vec3& vec3::operator+=( float s )
{
	x += s;
	y += s;
	z += s;
	return *this;
}

// This file is part of 'slm' C++ library. Copyright (C) 2009 Jani Kajala (kajala@gmail.com). See http://sourceforge.net/projects/slm/
