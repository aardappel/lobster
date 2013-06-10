inline float clamp( float v, float min, float max )
{
	return v < min ? min : (v > max ? max : v);
}

inline float saturate( float v )
{
	return clamp( v, 0.f, 1.f );
}

inline float mix( float x, float y, float a )
{
	return x + (y-x)*a;
}

inline float distance( float p0, float p1 )
{
	return length( p1 - p0 );
}

inline float radians( float deg )
{
	return deg * 0.017453292519943295769236907684886f; // pi/180
}

inline float degrees( float rad )
{
	return rad * 57.295779513082320876798154814105f; // 180/pi;
}

inline float min( float a, float b )		
{
	return a < b ? a : b;
}

inline float max( float a, float b )		
{
	return a < b ? b : a;
}

inline void sincos( float a, float* sina, float* cosa )
{
	*sina = sinf(a);
	*cosa = cosf(a);
}

inline float step( float edge, float v )
{
	return v < edge ? 0.f : 1.f;
}

inline bool check( float v )
{
	return v <= FLT_MAX && v >= -FLT_MAX;
}

inline bool check( const float* v, size_t n )
{
	for ( size_t i = 0 ; i < n ; ++i )
		if ( !(v[i] <= FLT_MAX && v[i] >= -FLT_MAX) )
			return false;
	return true;
}

inline float length( float v )
{
	return v < 0.f ? -v : v;
}

inline int min( int a, int b )
{
	return a < b ? a : b;
}

inline int max( int a, int b )
{
	return a < b ? b : a;
}

inline int clamp( int v, int vmin, int vmax )
{
	return v < vmin ? v : (v > vmax ? vmax : v);
}

inline size_t min( size_t a, size_t b )
{
	return a < b ? a : b;
}

inline size_t max( size_t a, size_t b )
{
	return a < b ? b : a;
}

inline size_t clamp( size_t v, size_t vmin, size_t vmax )
{
	return v < vmin ? v : (v > vmax ? vmax : v);
}

// This file is part of 'slm' C++ library. Copyright (C) 2009 Jani Kajala (kajala@gmail.com). See http://sourceforge.net/projects/slm/
