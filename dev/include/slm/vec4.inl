inline vec4::vec4()
{
#ifdef _DEBUG
	const int nan = 0x7F800001;
	x = y = z = w = *(const float*)&nan;
#endif
}

inline vec4::vec4( float v )
{
	SLMATH_VEC_ASSERT( check(v) );
	m128() = SLMATH_LOAD_PS1(&v);
}

inline vec4::vec4( float x0, float y0, float z0, float w0 ) : 
	x(x0), y(y0), z(z0), w(w0) 
{
	SLMATH_VEC_ASSERT( check(*this) );
}

inline vec4::vec4( const vec2& v0, float z0, float w0 ) :
	x(v0.x), y(v0.y), z(z0), w(w0) 
{
	SLMATH_VEC_ASSERT( check(*this) );
}

inline vec4::vec4( const vec3& v0, float w0 ) :
	x(v0.x), y(v0.y), z(v0.z), w(w0) 
{
	SLMATH_VEC_ASSERT( check(*this) );
}

inline void vec4::set( float x0, float y0, float z0, float w0 )
{
	x=x0;
	y=y0;
	z=z0;
	w=w0;
	SLMATH_VEC_ASSERT( check(*this) );
}

inline vec4& vec4::operator+=( const vec4& o )
{
	m128() = SLMATH_ADD_PS( m128(), o.m128() );
	SLMATH_VEC_ASSERT( check(*this) );
	return *this;
}

inline vec4& vec4::operator-=( const vec4& o )
{
	m128() = SLMATH_SUB_PS( m128(), o.m128() );
	SLMATH_VEC_ASSERT( check(*this) );
	return *this;
}

inline vec4& vec4::operator*=( float s )
{
	m128() = SLMATH_MUL_PS( m128(), SLMATH_LOAD_PS1(&s) );
	SLMATH_VEC_ASSERT( check(*this) );
	return *this;
}

inline vec4& vec4::operator*=( const vec4& o )
{
	m128() = SLMATH_MUL_PS( m128(), o.m128() );
	SLMATH_VEC_ASSERT( check(*this) );
	return *this;
}

inline vec4& vec4::operator/=( const vec4& o )
{
	m128() = SLMATH_DIV_PS( m128(), o.m128() );
	SLMATH_VEC_ASSERT( check(*this) );
	return *this;
}

inline vec4& vec4::operator/=( float s )
{
	SLMATH_VEC_ASSERT( fabsf(s) >= FLT_MIN ); // s must be != 0
	
	const float invs = 1.f/s;
	m128() = SLMATH_MUL_PS( m128(), SLMATH_LOAD_PS1(&invs) );
	return *this;
}

inline vec4 vec4::operator/( float s ) const
{
	SLMATH_VEC_ASSERT( fabsf(s) >= FLT_MIN ); // s must be != 0
	const float invs = 1.f/s;
	return vec4( SLMATH_MUL_PS( m128(), SLMATH_LOAD_PS1(&invs) ) );
}

inline vec3& vec4::xyz()
{
	return *(vec3*)&x;
}

inline const vec3& vec4::xyz() const
{
	return *(const vec3*)&x;
}

inline vec4 vec4::operator*( const vec4& o ) const
{
	SLMATH_VEC_ASSERT( check(o) );
	SLMATH_VEC_ASSERT( check(*this) );
	return vec4( SLMATH_MUL_PS( m128(), o.m128() ) );
}

inline vec4 vec4::operator/( const vec4& o ) const
{
	SLMATH_VEC_ASSERT( check(*this) );
	SLMATH_VEC_ASSERT( fabsf(o.x) > FLT_MIN );
	SLMATH_VEC_ASSERT( fabsf(o.y) > FLT_MIN );
	SLMATH_VEC_ASSERT( fabsf(o.z) > FLT_MIN );
	SLMATH_VEC_ASSERT( fabsf(o.w) > FLT_MIN );
	return vec4( SLMATH_DIV_PS( m128(), o.m128() ) );
}

inline vec4 vec4::operator+( const vec4& o ) const
{
	SLMATH_VEC_ASSERT( check(o) );
	SLMATH_VEC_ASSERT( check(*this) );
	return vec4( SLMATH_ADD_PS( m128(), o.m128() ) );
}

inline vec4 vec4::operator-( const vec4& o ) const
{
	SLMATH_VEC_ASSERT( check(o) );
	SLMATH_VEC_ASSERT( check(*this) );
	return vec4( SLMATH_SUB_PS( m128(), o.m128() ) );
}

inline vec4 vec4::operator-() const
{
	SLMATH_VEC_ASSERT( check(*this) );
	return vec4( SLMATH_SUB_PS( SLMATH_SETZERO_PS(), m128() ) );
}

inline vec4 vec4::operator*( float s ) const
{
	SLMATH_VEC_ASSERT( check(s) );
	SLMATH_VEC_ASSERT( check(*this) );
	return vec4( SLMATH_MUL_PS( m128(), SLMATH_LOAD_PS1(&s) ) );
}

inline bool vec4::operator==( const vec4& o ) const
{
	return x==o.x && y==o.y && z==o.z && w==o.w;
}

inline bool vec4::operator!=( const vec4& o ) const
{
	return !(*this == o);
}

inline vec4 operator*( float s, const vec4& v )
{
	SLMATH_VEC_ASSERT( check(s) );
	SLMATH_VEC_ASSERT( check(v) );
	return vec4( SLMATH_MUL_PS( SLMATH_LOAD_PS1(&s), v.m128() ) );
}

inline float length( const vec4& v )
{
	const float res = sqrtf( v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w );
	SLMATH_VEC_ASSERT( res >= 0.f && res <= FLT_MAX );
	return res;
}

inline float dot( const vec4& a, const vec4& b )
{
	const float res = a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
	SLMATH_VEC_ASSERT( res >= -FLT_MAX && res <= FLT_MAX );
	return res;
}

inline vec4 max( const vec4& a, const vec4& b )
{
	return vec4( SLMATH_MAX_PS(a.m128(),b.m128()) );
}

inline vec4 min( const vec4& a, const vec4& b )
{
	return vec4( SLMATH_MIN_PS(a.m128(),b.m128()) );
}

inline vec4 abs( const vec4& v )
{
	return vec4( SLMATH_MAX_PS(SLMATH_SUB_PS(SLMATH_SETZERO_PS(),v.m128()),v.m128()) );
}

inline vec4 mix( const vec4& x, const vec4& y, float a )
{
	return vecMix( x, y, a );
}

inline float distance( const vec4& p0, const vec4& p1 )
{
	return length( p0 - p1 );
}

inline vec4 clamp( const vec4& v, const vec4& min, const vec4& max )
{
	return vecClamp( v, min, max );
}

inline vec4 saturate( const vec4& v )
{
	return vecSaturate( v );
}

inline bool check( const vec4& v )
{
#ifdef SLMATH_SIMD
	SLMATH_VEC_ASSERT( 0 == (reinterpret_cast<size_t>(&v.x)&0xF) ); // 16B aligned?
#endif
	return v.x<=FLT_MAX && v.x>=-FLT_MAX && v.y<=FLT_MAX && v.y>=-FLT_MAX && v.z<=FLT_MAX && v.z>=-FLT_MAX && v.w<=FLT_MAX && v.w>=-FLT_MAX;
}

inline vec4 neg( const vec4& a )
{
	return -a;
}

inline vec4 add( const vec4& a, const vec4& b )
{
	return a+b;
}

inline vec4 sub( const vec4& a, const vec4& b )
{
	return a-b;
}

inline vec4 mul( const vec4& a, float b )
{
	return a*b;
}

inline vec4 mul( float b, const vec4& a )
{
	return a*b;
}

inline vec4& vec4::operator-=( float s )
{
	x -= s; 
	y -= s; 
	z -= s; 
	z -= s;
	return *this;
}

inline vec4& vec4::operator+=( float s )
{
	x += s;
	y += s;
	z += s;
	w += s;
	return *this;
}

// This file is part of 'slm' C++ library. Copyright (C) 2009 Jani Kajala (kajala@gmail.com). See http://sourceforge.net/projects/slm/
