inline vec2::vec2() 
{
#ifdef _DEBUG
	const int nan = 0x7F800001;
	x = y = *(const float*)&nan;
#endif
}

inline vec2::vec2( float v )
{
	SLMATH_VEC_ASSERT( check(v) );
	y = x = v;
}

inline vec2::vec2( float x0, float y0 ) : 
	x(x0), y(y0)
{
	SLMATH_VEC_ASSERT( check(*this) );
}

inline void vec2::set( float x0, float y0 )
{
	x=x0;
	y=y0;
	SLMATH_VEC_ASSERT( check(*this) );
}

inline vec2& vec2::operator+=( const vec2& o )
{
	x += o.x;
	y += o.y;
	SLMATH_VEC_ASSERT( check(*this) );
	return *this;
}

inline vec2& vec2::operator-=( const vec2& o )
{
	x -= o.x;
	y -= o.y;
	SLMATH_VEC_ASSERT( check(*this) );
	return *this;
}

inline vec2& vec2::operator*=( float s )
{
	x *= s;
	y *= s;
	SLMATH_VEC_ASSERT( check(*this) );
	return *this;
}

inline vec2& vec2::operator/=( float s )
{
	SLMATH_VEC_ASSERT( check(*this) );
	SLMATH_VEC_ASSERT( fabsf(s) >= FLT_MIN ); // s must be != 0
	const float sinv = 1.f/s;
	x *= sinv;
	y *= sinv;
	return *this;
}

inline vec2 vec2::operator/( float s ) const
{
	SLMATH_VEC_ASSERT( check(*this) );
	SLMATH_VEC_ASSERT( fabsf(s) >= FLT_MIN ); // s must be != 0
	const float sinv = 1.f/s;
	return vec2( x*sinv, y*sinv );
}

inline vec2& vec2::operator*=( const vec2& o )
{
	SLMATH_VEC_ASSERT( check(o) );
	SLMATH_VEC_ASSERT( check(*this) );
	x *= o.x;
	y *= o.y;
	return *this;
}

inline vec2& vec2::operator/=( const vec2& o )
{
	SLMATH_VEC_ASSERT( check(o) );
	SLMATH_VEC_ASSERT( check(*this) );
	SLMATH_VEC_ASSERT( fabsf(o.x) > FLT_MIN );
	SLMATH_VEC_ASSERT( fabsf(o.y) > FLT_MIN );
	x /= o.x;
	y /= o.y;
	return *this;
}

inline vec2 vec2::operator*( const vec2& o ) const
{
	SLMATH_VEC_ASSERT( check(o) );
	SLMATH_VEC_ASSERT( check(*this) );
	return vec2(x*o.x,y*o.y);
}

inline vec2 vec2::operator/( const vec2& o ) const
{
	SLMATH_VEC_ASSERT( check(o) );
	SLMATH_VEC_ASSERT( check(*this) );
	SLMATH_VEC_ASSERT( fabsf(o.x) > FLT_MIN );
	SLMATH_VEC_ASSERT( fabsf(o.y) > FLT_MIN );
	return vec2(x/o.x,y/o.y);
}

inline vec2 vec2::operator+( const vec2& o ) const
{
	SLMATH_VEC_ASSERT( check(o) );
	SLMATH_VEC_ASSERT( check(*this) );
	return vec2(x+o.x,y+o.y);
}

inline vec2 vec2::operator-( const vec2& o ) const
{
	SLMATH_VEC_ASSERT( check(o) );
	SLMATH_VEC_ASSERT( check(*this) );
	return vec2(x-o.x,y-o.y);
}

inline vec2 vec2::operator-() const
{
	SLMATH_VEC_ASSERT( check(*this) );
	return vec2(-x,-y);
}

inline vec2 vec2::operator*( float s ) const
{
	SLMATH_VEC_ASSERT( check(s) );
	SLMATH_VEC_ASSERT( check(*this) );
	return vec2(x*s,y*s);
}

inline bool vec2::operator==( const vec2& o ) const
{
	return x==o.x && y==o.y;
}

inline bool vec2::operator!=( const vec2& o ) const
{
	return !(*this == o);
}

inline float length( const vec2& v )
{
	SLMATH_VEC_ASSERT( check(v) );

	float res = sqrtf( v.x*v.x + v.y*v.y );

	SLMATH_VEC_ASSERT( res >= 0.f && res <= FLT_MAX );
	return res;
}

inline float dot( const vec2& a, const vec2& b )
{
	SLMATH_VEC_ASSERT( check(a) );
	SLMATH_VEC_ASSERT( check(b) );

	float res = a.x*b.x + a.y*b.y;

	SLMATH_VEC_ASSERT( res >= -FLT_MAX && res <= FLT_MAX );
	return res;
}

inline vec2	operator*( float s, const vec2& v )		
{
	SLMATH_VEC_ASSERT( check(s) );
	SLMATH_VEC_ASSERT( check(v) ); 
	return vec2(v.x*s,v.y*s);
}

inline vec2 max( const vec2& a, const vec2& b )
{
	return vec2( max(a[0],b[0]), max(a[1],b[1]) );
}

inline vec2 min( const vec2& a, const vec2& b )
{
	return vec2( min(a[0],b[0]), min(a[1],b[1]) );
}

inline vec2 abs( const vec2& v )
{
	return vec2( fabsf(v[0]), fabsf(v[1]) );
}

inline vec2 mix( const vec2& x, const vec2& y, float a )
{
	return vecMix( x, y, a );
}

inline float distance( const vec2& p0, const vec2& p1 )
{
	return length( p0 - p1 );
}

inline vec2 clamp( const vec2& v, const vec2& min, const vec2& max )
{
	return vecClamp( v, min, max );
}

inline vec2 saturate( const vec2& v )
{
	return vecSaturate( v );
}

inline bool check( const vec2& v )
{
	return v.x<=FLT_MAX && v.x>=-FLT_MAX && v.y<=FLT_MAX && v.y>=-FLT_MAX;
}

inline vec2 neg( const vec2& a )
{
	return -a;
}

inline vec2 add( const vec2& a, const vec2& b )
{
	return a+b;
}

inline vec2 sub( const vec2& a, const vec2& b )
{
	return a-b;
}

inline vec2 mul( const vec2& a, float b )
{
	return a*b;
}

inline vec2 mul( float b, const vec2& a )
{
	return a*b;
}

inline vec2& vec2::operator-=( float s )
{
	x -= s; 
	y -= s; 
	return *this;
}

inline vec2& vec2::operator+=( float s )
{
	x += s;
	y += s;
	return *this;
}

// This file is part of 'slm' C++ library. Copyright (C) 2009 Jani Kajala (kajala@gmail.com). See http://sourceforge.net/projects/slm/
