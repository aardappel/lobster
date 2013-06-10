inline mat4::mat4()
{
#ifdef _DEBUG
	const int nan = 0x7F800001;
	float* p = (float*)&m_m128;
	for ( size_t i = 0 ; i < 16 ; ++i ) 
		p[i] = *(const float*)&nan;
#endif
}

inline void mat4::set( size_t i, const vec4& v )
{
	v4()[i] = v;
}

inline const vec4& mat4::get( size_t i ) const
{
	SLMATH_VEC_ASSERT( i < 4 );
	return v4()[i];
}

inline vec4& mat4::operator[]( size_t i )// This file is part of 'slm' C++ library. Copyright (C) 2009 Jani Kajala (kajala@gmail.com). See http://sourceforge.net/projects/slm/
{
	SLMATH_VEC_ASSERT( i < 4 );
	return v4()[i];
}

inline const vec4& mat4::operator[]( size_t i ) const
{
	SLMATH_VEC_ASSERT( i < 4 );
	return v4()[i];
}

inline vec4 operator*( const vec4& v, const mat4& m )
{
	SLMATH_VEC_ASSERT( check(v) );
	SLMATH_VEC_ASSERT( check(m) );
	return vec4( dot(v,m[0]), dot(v,m[1]), dot(v,m[2]), dot(v,m[3]) );
}

inline vec4 operator*( const mat4& m, const vec4& v )
{
	SLMATH_VEC_ASSERT( check(v) );
	SLMATH_VEC_ASSERT( check(m) );
	return m[0]*v.x + m[1]*v.y + m[2]*v.z + m[3]*v.w;
}

inline vec4 mul( const mat4& m, const vec4& v )
{
	return m*v;
}

inline vec4 mul( const vec4& v, const mat4& m )
{
	return v*m;
}

inline bool check( const mat4& v )
{
	return check( &v[0][0], 4*4 );
}

// This file is part of 'slm' C++ library. Copyright (C) 2009 Jani Kajala (kajala@gmail.com). See http://sourceforge.net/projects/slm/
