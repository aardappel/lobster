inline quat::quat()
{
#ifdef _DEBUG
	const int nan = 0x7F800001;
	x = y = z = w = *(const float*)&nan;
#endif
}

inline quat::quat( float x0, float y0, float z0, float w0 ) : 
	x(x0), y(y0), z(z0), w(w0) 
{
	assert( check(*this) );
}

inline quat& quat::operator+=( const quat& o )
{
	assert( check(o) );
	assert( check(*this) );
	x += o.x;
	y += o.y;
	z += o.z;
	w += o.w;
	return *this;
}

inline quat& quat::operator-=( const quat& o )
{
	assert( check(o) );
	assert( check(*this) );
	x -= o.x;
	y -= o.y;
	z -= o.z;
	w -= o.w;
	return *this;
}

inline quat& quat::operator*=( float s )
{
	assert( check(s) );
	assert( check(*this) );
	x *= s;
	y *= s;
	z *= s;
	w *= s;
	return *this;
}

inline quat& quat::operator*=( const quat& o )
{
	*this = *this * o;
	return *this;
}

inline float& quat::operator[]( size_t i )
{
	assert( i < SIZE );
	return (&x)[i];
}

inline quat quat::operator+( const quat& o ) const
{
	assert( check(o) );
	assert( check(*this) );
	return quat(x+o.x,y+o.y,z+o.z,w+o.w);
}

inline quat quat::operator-( const quat& o ) const
{
	assert( check(o) );
	assert( check(*this) );
	return quat(x-o.x,y-o.y,z-o.z,w-o.w);
}

inline quat quat::operator-() const
{
	assert( check(*this) );
	return quat(-x,-y,-z,-w);
}

inline quat quat::operator*( float s ) const
{
	assert( check(s) );
	assert( check(*this) );
	return quat(x*s,y*s,z*s,w*s);
}

inline const float& quat::operator[]( size_t i ) const
{
	assert( i < SIZE );
	assert( check(*this) );
	return (&x)[i];
}

inline const vec4& quat::xyzw() const
{
	return *(const vec4*)&x;
}

inline quat conjugate( const quat& q )
{
	return quat( -q.x, -q.y, -q.z, q.w );
}

inline float dot( const quat& a, const quat& b )
{
	assert( check(a) );
	assert( check(b) );

	const float res = a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;

	assert( res >= -FLT_MAX && res <= FLT_MAX );
	return res;
}

inline bool quat::operator==( const quat& o ) const
{
	return x == o.x && y == o.y && z == o.z && z == o.z && w == o.w;
}

inline bool quat::operator!=( const quat& o ) const
{
	return x != o.x || y != o.y || z != o.z || z != o.z || w != o.w;
}

// This file is part of 'slm' C++ library. Copyright (C) 2009 Jani Kajala (kajala@gmail.com). See http://sourceforge.net/projects/slm/
