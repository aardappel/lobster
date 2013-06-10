#include <slm/mat4.h>
#include <slm/quat.h>
#include <slm/no_simd.h>

// for computing matrix minors
#define DET2(a,b,c,d) ( a*d-b*c )
#define DET3(a,b,c, d,e,f, g,h,i) ( a*DET2(e,f,h,i) - b*DET2(d,f,g,i) + c*DET2(d,e,g,h) )
#define DET4(a,b,c,d, e,f,g,h, i,j,k,l, m,n,o,p) ( a*DET3(f,g,h,j,k,l,n,o,p) - b*DET3(e,g,h,i,k,l,m,o,p) + c*DET3(e,f,h,i,j,l,m,n,p) - d*DET3(e,f,g,i,j,k,m,n,o) )

SLMATH_BEGIN()

mat4::mat4( float d )
{
	register vec4* const m = v4();
	m[0] = vec4( d,0,0,0 );
	m[1] = vec4( 0,d,0,0 );
	m[2] = vec4( 0,0,d,0 );
	m[3] = vec4( 0,0,0,d );
}

mat4::mat4( float a, const vec3& v )
{
	SLMATH_VEC_ASSERT( length(v) > FLT_MIN );

	float s, c;
	sincos( a, &s, &c );

	const float		t = 1.f - c;
	const vec3		n = normalize( v );
	const float		x = n.x;
	const float		y = n.y;
	const float		z = n.z;

	register vec4* const m = v4();
	m[0][0] = t*x*x + c;
	m[0][1] = t*x*y + z*s;
	m[0][2] = t*x*z - y*s;
	m[0][3] = 0.f;
	m[1][0] = t*x*y - z*s;
	m[1][1] = t*y*y + c;
	m[1][2] = t*y*z + x*s;
	m[1][3] = 0.f;
	m[2][0] = t*x*z + y*s;
	m[2][1] = t*y*z - x*s;
	m[2][2] = t*z*z + c;
	m[2][3] = 0.f;
	m[3][0] = 0.f;
	m[3][1] = 0.f;
	m[3][2] = 0.f;
	m[3][3] = 1.f;
}

mat4::mat4( const vec4& c0, const vec4& c1, const vec4& c2, const vec4& c3 )
{
	register vec4* const m = v4();
	m[0] = c0;
	m[1] = c1;
	m[2] = c2;
	m[3] = c3;
}

mat4& mat4::operator+=( const mat4& o )
{
	m_m128[0] = SLMATH_ADD_PS( m_m128[0], o.m_m128[0] );
	m_m128[1] = SLMATH_ADD_PS( m_m128[1], o.m_m128[1] );
	m_m128[2] = SLMATH_ADD_PS( m_m128[2], o.m_m128[2] );
	m_m128[3] = SLMATH_ADD_PS( m_m128[3], o.m_m128[3] );
	return *this;
}

mat4& mat4::operator-=( const mat4& o )
{
	m_m128[0] = SLMATH_SUB_PS( m_m128[0], o.m_m128[0] );
	m_m128[1] = SLMATH_SUB_PS( m_m128[1], o.m_m128[1] );
	m_m128[2] = SLMATH_SUB_PS( m_m128[2], o.m_m128[2] );
	m_m128[3] = SLMATH_SUB_PS( m_m128[3], o.m_m128[3] );
	return *this;
}

mat4& mat4::operator*=( float s )
{
	m128_t s128 = SLMATH_LOAD_PS1(&s);
	m_m128[0] = SLMATH_MUL_PS( m_m128[0], s128 );
	m_m128[1] = SLMATH_MUL_PS( m_m128[1], s128 );
	m_m128[2] = SLMATH_MUL_PS( m_m128[2], s128 );
	m_m128[3] = SLMATH_MUL_PS( m_m128[3], s128 );
	return *this;
}

bool mat4::operator==( const mat4& o ) const
{
	SLMATH_VEC_ASSERT( check(*this) );
	SLMATH_VEC_ASSERT( check(o) );

	register const vec4* const m = v4();
	for ( size_t i = 0 ; i < 4 ; ++i )
		if ( m[i] != o[i] )
			return false;
	return true;
}

bool mat4::operator!=( const mat4& o ) const
{
	SLMATH_VEC_ASSERT( check(*this) );
	SLMATH_VEC_ASSERT( check(o) );

	register const vec4* const m = v4();
	for ( size_t i = 0 ; i < 4 ; ++i )
		if ( m[i] != o[i] )
			return true;
	return false;
}

mat4 mat4::operator+( const mat4& o ) const
{
	mat4 res;
	res.m_m128[0] = SLMATH_ADD_PS( m_m128[0], o.m_m128[0] );
	res.m_m128[1] = SLMATH_ADD_PS( m_m128[1], o.m_m128[1] );
	res.m_m128[2] = SLMATH_ADD_PS( m_m128[2], o.m_m128[2] );
	res.m_m128[3] = SLMATH_ADD_PS( m_m128[3], o.m_m128[3] );
	return res;
}

mat4 mat4::operator-( const mat4& o ) const
{
	mat4 res;
	res.m_m128[0] = SLMATH_SUB_PS( m_m128[0], o.m_m128[0] );
	res.m_m128[1] = SLMATH_SUB_PS( m_m128[1], o.m_m128[1] );
	res.m_m128[2] = SLMATH_SUB_PS( m_m128[2], o.m_m128[2] );
	res.m_m128[3] = SLMATH_SUB_PS( m_m128[3], o.m_m128[3] );
	return res;
}

mat4 mat4::operator-() const
{
	mat4 res;
	m128_t zero = SLMATH_SETZERO_PS();
	res.m_m128[0] = SLMATH_SUB_PS( zero, m_m128[0] );
	res.m_m128[1] = SLMATH_SUB_PS( zero, m_m128[1] );
	res.m_m128[2] = SLMATH_SUB_PS( zero, m_m128[2] );
	res.m_m128[3] = SLMATH_SUB_PS( zero, m_m128[3] );
	return res;
}

mat4 mat4::operator*( float s ) const
{
	mat4 res;
	m128_t s128 = SLMATH_LOAD_PS1(&s);
	res.m_m128[0] = SLMATH_MUL_PS( m_m128[0], s128 );
	res.m_m128[1] = SLMATH_MUL_PS( m_m128[1], s128 );
	res.m_m128[2] = SLMATH_MUL_PS( m_m128[2], s128 );
	res.m_m128[3] = SLMATH_MUL_PS( m_m128[3], s128 );
	return res;
}

mat4& mat4::operator*=( const mat4& o )
{
	return *this = *this * o;
}

mat4 mat4::operator*( const mat4& o ) const
{
	mat4 res;

#ifdef SLMATH_SIMD
	#define VTMP(i,j) SLMATH_MUL_PS( m_m128[i], SLMATH_LOAD_PS1(&o[j][i]) )
	m128_t* const o128 = res.m128();
	o128[0] = SLMATH_ADD_PS( SLMATH_ADD_PS(VTMP(0,0),VTMP(1,0)), SLMATH_ADD_PS(VTMP(2,0),VTMP(3,0)) );
	o128[1] = SLMATH_ADD_PS( SLMATH_ADD_PS(VTMP(0,1),VTMP(1,1)), SLMATH_ADD_PS(VTMP(2,1),VTMP(3,1)) );
	o128[2] = SLMATH_ADD_PS( SLMATH_ADD_PS(VTMP(0,2),VTMP(1,2)), SLMATH_ADD_PS(VTMP(2,2),VTMP(3,2)) );
	o128[3] = SLMATH_ADD_PS( SLMATH_ADD_PS(VTMP(0,3),VTMP(1,3)), SLMATH_ADD_PS(VTMP(2,3),VTMP(3,3)) );
	#undef VTMP
#else
	// note: above SIMD-macro version works also on non-SIMD platforms, this is much faster if there is no SIMD support
	const vec4* const mp = &get(0);
	const vec4* const op = &o.get(0);
	vec4* const resp = res.v4();
	MAT4_MUL_MAT4( resp, mp, op );
#endif

	SLMATH_VEC_ASSERT( check(res) );
	return res;
}

    mat4 transpose( const mat4& m )
    {
	    mat4 res;

    #ifdef SLMATH_SSE2_MSVC
    
	    const m128_t* const mp = m.m128();
	    m128_t* const resp = res.m128();
        m128_t tmp0 = _mm_shuffle_ps(mp[0], mp[1], 0x44);
        m128_t tmp2 = _mm_shuffle_ps(mp[0], mp[1], 0xEE);
        m128_t tmp1 = _mm_shuffle_ps(mp[2], mp[3], 0x44);
        m128_t tmp3 = _mm_shuffle_ps(mp[2], mp[3], 0xEE);
	    resp[0] = _mm_shuffle_ps(tmp0, tmp1, 0x88);
        resp[1] = _mm_shuffle_ps(tmp0, tmp1, 0xDD);
        resp[2] = _mm_shuffle_ps(tmp2, tmp3, 0x88);
        resp[3] = _mm_shuffle_ps(tmp2, tmp3, 0xDD);

    #else
	
	    for ( size_t j = 0 ; j < 4 ; ++j )
	    {
		    res[0][j] = m[j][0];
		    res[1][j] = m[j][1];
		    res[2][j] = m[j][2];
		    res[3][j] = m[j][3];
	    }

    #endif

	    return res;
    }

float det( const mat4& m )
{
	const float res = DET4( m[0][0],m[0][1],m[0][2],m[0][3], m[1][0],m[1][1],m[1][2],m[1][3], m[2][0],m[2][1],m[2][2],m[2][3], m[3][0],m[3][1],m[3][2],m[3][3] );
	SLMATH_VEC_ASSERT( check(res) );
	return res;
}

mat4 inverse( const mat4& m0 )
{
	SLMATH_VEC_ASSERT( check(m0) );

	const float* const mp = &m0[0][0];
	const float a = mp[0];	const float b = mp[1];	const float c = mp[2];	const float d = mp[3];
	const float e = mp[4];	const float f = mp[5];	const float g = mp[6];	const float h = mp[7];
	const float i = mp[8];	const float j = mp[9];	const float k = mp[10];	const float l = mp[11];
	const float m = mp[12];	const float n = mp[13];	const float o = mp[14];	const float p = mp[15];
	
	const float min_a = DET3(f,g,h,j,k,l,n,o,p);
	const float min_b = DET3(e,g,h,i,k,l,m,o,p);
	const float min_c = DET3(e,f,h,i,j,l,m,n,p);
	const float min_d = DET3(e,f,g,i,j,k,m,n,o);
	const float det_m = a*min_a - b*min_b + c*min_c - d*min_d;
	SLMATH_VEC_ASSERT( det_m > FLT_MIN || det_m < -FLT_MIN ); // invertible?

	mat4 res;
	res[0][0] = min_a;
	res[1][0] = -min_b;
	res[2][0] = min_c;
	res[3][0] = -min_d;
	res[0][1] = -DET3(b,c,d,j,k,l,n,o,p);
	res[1][1] = DET3(a,c,d,i,k,l,m,o,p);
	res[2][1] = -DET3(a,b,d,i,j,l,m,n,p);
	res[3][1] = DET3(a,b,c,i,j,k,m,n,o);
	res[0][2] = DET3(b,c,d,f,g,h,n,o,p);
	res[1][2] = -DET3(a,c,d,e,g,h,m,o,p);
	res[2][2] = DET3(a,b,d,e,f,h,m,n,p);
	res[3][2] = -DET3(a,b,c,e,f,g,m,n,o);
	res[0][3] = -DET3(b,c,d,f,g,h,j,k,l);
	res[1][3] = DET3(a,c,d,e,g,h,i,k,l);
	res[2][3] = -DET3(a,b,d,e,f,h,i,j,l);
	res[3][3] = DET3(a,b,c,e,f,g,i,j,k);
	return res * (1.f/det_m);
/*
	res *= (1.f/det_m);
	mat4 res2;
	res2[0][0] = m0[1][2]*m0[2][3]*m0[3][1] - m0[1][3]*m0[2][2]*m0[3][1] + m0[1][3]*m0[2][1]*m0[3][2] - m0[1][1]*m0[2][3]*m0[3][2] - m0[1][2]*m0[2][1]*m0[3][3] + m0[1][1]*m0[2][2]*m0[3][3];
	res2[0][1] = m0[0][3]*m0[2][2]*m0[3][1] - m0[0][2]*m0[2][3]*m0[3][1] - m0[0][3]*m0[2][1]*m0[3][2] + m0[0][1]*m0[2][3]*m0[3][2] + m0[0][2]*m0[2][1]*m0[3][3] - m0[0][1]*m0[2][2]*m0[3][3];
	res2[0][2] = m0[0][2]*m0[1][3]*m0[3][1] - m0[0][3]*m0[1][2]*m0[3][1] + m0[0][3]*m0[1][1]*m0[3][2] - m0[0][1]*m0[1][3]*m0[3][2] - m0[0][2]*m0[1][1]*m0[3][3] + m0[0][1]*m0[1][2]*m0[3][3];
	res2[0][3] = m0[0][3]*m0[1][2]*m0[2][1] - m0[0][2]*m0[1][3]*m0[2][1] - m0[0][3]*m0[1][1]*m0[2][2] + m0[0][1]*m0[1][3]*m0[2][2] + m0[0][2]*m0[1][1]*m0[2][3] - m0[0][1]*m0[1][2]*m0[2][3];
	res2[1][0] = m0[1][3]*m0[2][2]*m0[3][0] - m0[1][2]*m0[2][3]*m0[3][0] - m0[1][3]*m0[2][0]*m0[3][2] + m0[1][0]*m0[2][3]*m0[3][2] + m0[1][2]*m0[2][0]*m0[3][3] - m0[1][0]*m0[2][2]*m0[3][3];
	res2[1][1] = m0[0][2]*m0[2][3]*m0[3][0] - m0[0][3]*m0[2][2]*m0[3][0] + m0[0][3]*m0[2][0]*m0[3][2] - m0[0][0]*m0[2][3]*m0[3][2] - m0[0][2]*m0[2][0]*m0[3][3] + m0[0][0]*m0[2][2]*m0[3][3];
	res2[1][2] = m0[0][3]*m0[1][2]*m0[3][0] - m0[0][2]*m0[1][3]*m0[3][0] - m0[0][3]*m0[1][0]*m0[3][2] + m0[0][0]*m0[1][3]*m0[3][2] + m0[0][2]*m0[1][0]*m0[3][3] - m0[0][0]*m0[1][2]*m0[3][3];
	res2[1][3] = m0[0][2]*m0[1][3]*m0[2][0] - m0[0][3]*m0[1][2]*m0[2][0] + m0[0][3]*m0[1][0]*m0[2][2] - m0[0][0]*m0[1][3]*m0[2][2] - m0[0][2]*m0[1][0]*m0[2][3] + m0[0][0]*m0[1][2]*m0[2][3];
	res2[2][0] = m0[1][1]*m0[2][3]*m0[3][0] - m0[1][3]*m0[2][1]*m0[3][0] + m0[1][3]*m0[2][0]*m0[3][1] - m0[1][0]*m0[2][3]*m0[3][1] - m0[1][1]*m0[2][0]*m0[3][3] + m0[1][0]*m0[2][1]*m0[3][3];
	res2[2][1] = m0[0][3]*m0[2][1]*m0[3][0] - m0[0][1]*m0[2][3]*m0[3][0] - m0[0][3]*m0[2][0]*m0[3][1] + m0[0][0]*m0[2][3]*m0[3][1] + m0[0][1]*m0[2][0]*m0[3][3] - m0[0][0]*m0[2][1]*m0[3][3];
	res2[2][2] = m0[0][1]*m0[1][3]*m0[3][0] - m0[0][3]*m0[1][1]*m0[3][0] + m0[0][3]*m0[1][0]*m0[3][1] - m0[0][0]*m0[1][3]*m0[3][1] - m0[0][1]*m0[1][0]*m0[3][3] + m0[0][0]*m0[1][1]*m0[3][3];
	res2[2][3] = m0[0][3]*m0[1][1]*m0[2][0] - m0[0][1]*m0[1][3]*m0[2][0] - m0[0][3]*m0[1][0]*m0[2][1] + m0[0][0]*m0[1][3]*m0[2][1] + m0[0][1]*m0[1][0]*m0[2][3] - m0[0][0]*m0[1][1]*m0[2][3];
	res2[3][0] = m0[1][2]*m0[2][1]*m0[3][0] - m0[1][1]*m0[2][2]*m0[3][0] - m0[1][2]*m0[2][0]*m0[3][1] + m0[1][0]*m0[2][2]*m0[3][1] + m0[1][1]*m0[2][0]*m0[3][2] - m0[1][0]*m0[2][1]*m0[3][2];
	res2[3][1] = m0[0][1]*m0[2][2]*m0[3][0] - m0[0][2]*m0[2][1]*m0[3][0] + m0[0][2]*m0[2][0]*m0[3][1] - m0[0][0]*m0[2][2]*m0[3][1] - m0[0][1]*m0[2][0]*m0[3][2] + m0[0][0]*m0[2][1]*m0[3][2];
	res2[3][2] = m0[0][2]*m0[1][1]*m0[3][0] - m0[0][1]*m0[1][2]*m0[3][0] - m0[0][2]*m0[1][0]*m0[3][1] + m0[0][0]*m0[1][2]*m0[3][1] + m0[0][1]*m0[1][0]*m0[3][2] - m0[0][0]*m0[1][1]*m0[3][2];
	res2[3][3] = m0[0][1]*m0[1][2]*m0[2][0] - m0[0][2]*m0[1][1]*m0[2][0] + m0[0][2]*m0[1][0]*m0[2][1] - m0[0][0]*m0[1][2]*m0[2][1] - m0[0][1]*m0[1][0]*m0[2][2] + m0[0][0]*m0[1][1]*m0[2][2];
	res2 *= 1.f/det_m;
	mat4 dres = res2-res;
	float diff = length(dres[0])+length(dres[1])+length(dres[2])+length(dres[3]);
	SLMATH_VEC_ASSERT( diff < 1e-6f );
	return res;
*/
}

mat4::mat4( const quat& q )
{
    const float sqw = q.w*q.w;
    const float sqx = q.x*q.x;
    const float sqy = q.y*q.y;
    const float sqz = q.z*q.z;
	const float qlen = sqx + sqy + sqz + sqw;

	SLMATH_VEC_ASSERT( qlen > FLT_MIN );
    const float invs = 1.f / qlen; // only needed if not normalized

	register vec4* const m = v4();
    m[0][0] = ( sqx - sqy - sqz + sqw)*invs;
    m[1][1] = (-sqx + sqy - sqz + sqw)*invs;
    m[2][2] = (-sqx - sqy + sqz + sqw)*invs;

    register float tmp1 = q.x*q.y;
    register float tmp2 = q.z*q.w;
    m[0][1] = 2.0f * (tmp1 + tmp2)*invs;
    m[1][0] = 2.0f * (tmp1 - tmp2)*invs;
    
    tmp1 = q.x*q.z;
    tmp2 = q.y*q.w;
    m[0][2] = 2.0f * (tmp1 - tmp2)*invs;
    m[2][0] = 2.0f * (tmp1 + tmp2)*invs;
    
	tmp1 = q.y*q.z;
    tmp2 = q.x*q.w;
    m[1][2] = 2.0f * (tmp1 + tmp2)*invs;
    m[2][1] = 2.0f * (tmp1 - tmp2)*invs;

	m[0][3] = 0.f;
	m[1][3] = 0.f;
	m[2][3] = 0.f;
	m[3] = vec4(0,0,0,1.f);
}

mat4 translation( const vec3& t )
{
	SLMATH_VEC_ASSERT( check(t) );
	mat4 tm;
	tm[0] = vec4(   1,   0,   0,  0 );
	tm[1] = vec4(   0,   1,   0,  0 );
	tm[2] = vec4(   0,   0,   1,  0 );
	tm[3] = vec4( t.x, t.y, t.z,  1 );
	return tm;
}

mat4 scaling( float s )
{
	SLMATH_VEC_ASSERT( check(s) );
	mat4 tm;
	tm[0] = vec4(   s,   0,   0,  0 );
	tm[1] = vec4(   0,   s,   0,  0 );
	tm[2] = vec4(   0,   0,   s,  0 );
	tm[3] = vec4(   0,   0,   0,  1 );
	return tm;
}

/** 
 * Perspective projection matrix.
 * @param handedness 1.f for RH, -1.f for LH 
 */
static mat4 perspectiveFov( float fovy, float aspect, float znear, float zfar, float handedness )
{
	SLMATH_VEC_ASSERT( aspect > 0.001f );
	SLMATH_VEC_ASSERT( fabsf(zfar-znear) > 1e-6f );
	SLMATH_VEC_ASSERT( fovy >= radians(0.1f) );
	SLMATH_VEC_ASSERT( fovy <= radians(179.f) );

	const float y = cot( fovy * .5f );
	const float x = y / aspect;
	const float zdist = (znear-zfar)*handedness;
	const float zfar_per_zdist = zfar / zdist;

	mat4 m;
	m[0] = vec4(x,		  0,        0,					   		0 );
	m[1] = vec4(0,        y,        0,					   		0 );
	m[2] = vec4(0,        0,        zfar_per_zdist,    	   		-1.f*handedness );
	m[3] = vec4(0,        0,        znear*zfar_per_zdist*handedness,  0 );
	return m;
}

mat4 perspectiveFovLH( float fovy, float aspect, float znear, float zfar )
{
	return perspectiveFov(fovy,aspect,znear,zfar,-1.f);
}

mat4 perspectiveFovRH( float fovy, float aspect, float znear, float zfar )
{
	return perspectiveFov(fovy,aspect,znear,zfar,1.f);
}

mat4 rotationX( float a )
{
	float s, c;
	sincos( a, &s, &c );

	mat4 m;
	m[0] = vec4( 1, 0, 0, 0 );
	m[1] = vec4( 0, c, s, 0 );
	m[2] = vec4( 0,-s, c, 0 );
	m[3] = vec4( 0, 0, 0, 1 );

	SLMATH_VEC_ASSERT( det(m) != 0.f );
	return m;
}

mat4 rotationY( float a )
{
	float s, c;
	sincos( a, &s, &c );

	mat4 m;
	m[0] = vec4( c, 0,-s, 0 );
	m[1] = vec4( 0, 1, 0, 0 );
	m[2] = vec4( s, 0, c, 0 );
	m[3] = vec4( 0, 0, 0, 1 );

	SLMATH_VEC_ASSERT( det(m) != 0.f );
	return m;
}

mat4 rotationZ( float a )
{
	float s, c;
	sincos( a, &s, &c );

	mat4 m;
	m[0] = vec4( c, s, 0, 0 );
	m[1] = vec4(-s, c, 0, 0 );
	m[2] = vec4( 0, 0, 1, 0 );
	m[3] = vec4( 0, 0, 0, 1 );

	SLMATH_VEC_ASSERT( det(m) != 0.f );
	return m;
}

mat4 targetAtRH( const vec3& eye, const vec3& at, const vec3& up )
{
	SLMATH_VEC_ASSERT( distance(eye,at) > FLT_MIN );
	SLMATH_VEC_ASSERT( length(up) > FLT_MIN );

	vec3 zaxis = normalize(eye - at);
	vec3 xaxis = normalize(cross(up, zaxis));
	vec3 yaxis = cross(zaxis, xaxis);

	mat4 m;
	m[0] = vec4(xaxis,0);
	m[1] = vec4(yaxis,0);
	m[2] = vec4(zaxis,0);
	m[3] = vec4(eye,1);
	return m;
}

mat4 lookAtRH( const vec3& eye, const vec3& at, const vec3& up )
{
	vec3 zaxis = normalize(eye - at);
	vec3 xaxis = normalize(cross(up, zaxis));
	vec3 yaxis = cross(zaxis, xaxis);
	
	mat4 m;
	m[0] = vec4(  xaxis.x,           yaxis.x,           zaxis.x,          0 );
	m[1] = vec4(  xaxis.y,           yaxis.y,           zaxis.y,          0 );
	m[2] = vec4(  xaxis.z,           yaxis.z,           zaxis.z,          0 );
	m[3] = vec4( -dot(xaxis, eye),  -dot(yaxis, eye),  -dot(zaxis, eye),  1 );
	return m;
}

mat4 cubeMapViewRH( size_t face, const vec3& worldpos )
{
	SLMATH_VEC_ASSERT( face < 6 );
	SLMATH_VEC_ASSERT( check(worldpos) );

	vec3 x,y;
	switch ( face )
	{
	case 0: // X+
		x = vec3(0,0,-1);
		y = vec3(0,-1,0);
		break;

	case 1: // X-
		x = vec3(0,0,1);
		y = vec3(0,-1,0);
		break;

	case 2: // Y+
		x = vec3(1,0,0);
		y = vec3(0,0,1);
		break;

	case 3: // Y-
		x = vec3(1,0,0);
		y = vec3(0,0,-1);
		break;

	case 4: // Z+
		x = vec3(1,0,0);
		y = vec3(0,-1,0);
		break;

	case 5: // Z-
		x = vec3(-1,0,0);
		y = vec3(0,-1,0);
		break;
	}

	mat4 tm;
	tm[0] = vec4(x,0);
	tm[1] = vec4(y,0);
	tm[2] = vec4( normalize( cross(x,y) ), 0.f );
	tm[3] = vec4( worldpos, 1.f );
	
	return inverse(tm);
}

mat4 cubeMapProjectionRH( float znear, float zfar )
{
	SLMATH_VEC_ASSERT( znear > 1e-5f && znear < 1e6f );
	SLMATH_VEC_ASSERT( zfar > 1e-5f && zfar < 1e6f );
	SLMATH_VEC_ASSERT( zfar > znear );

	mat4 m = perspectiveFovRH( radians(90.f), 1.f, znear, zfar );
	m[1][1] = -m[1][1];
	return m;
}

mat4 fromToRotation( const vec3& from, const vec3& to )
{
	const float EPSILON = 0.000001f;

	mat4 mtx;
	mtx[3] = vec4(0,0,0,1);
	mtx[0][3] = 0.f;
	mtx[1][3] = 0.f;
	mtx[2][3] = 0.f;

	vec3 v;
	float e, h, f;

	v = cross(from, to);
	e = dot(from, to);
	f = (e < 0.f) ? -e : e;
	if (f > 1.0 - EPSILON)     /* "from" and "to"-vector almost parallel */
	{
		vec3 u, v; /* temporary storage vectors */
		vec3 x;       /* vector most nearly orthogonal to "from" */
		float c1, c2, c3; /* coefficients for later use */
		size_t i, j;

		x[0] = (from[0] > 0.0f)? from[0] : -from[0];
		x[1] = (from[1] > 0.0f)? from[1] : -from[1];
		x[2] = (from[2] > 0.0f)? from[2] : -from[2];

		if (x[0] < x[1])
		{
			if (x[0] < x[2])
			{
				x[0] = 1.0; x[1] = x[2] = 0.0f;
			}
			else
			{
				x[2] = 1.0; x[0] = x[1] = 0.0f;
			}
		}
		else
		{
			if (x[1] < x[2])
			{
				x[1] = 1.0; x[0] = x[2] = 0.0f;
			}
			else
			{
				x[2] = 1.0; x[0] = x[1] = 0.0f;
			}
		}

		u[0] = x[0] - from[0]; u[1] = x[1] - from[1]; u[2] = x[2] - from[2];
		v[0] = x[0] - to[0];   v[1] = x[1] - to[1];   v[2] = x[2] - to[2];

		c1 = 2.0f / dot(u, u);
		c2 = 2.0f / dot(v, v);
		c3 = c1 * c2  * dot(u, v);

		for (i = 0; i < 3; i++) 
		{
			for (j = 0; j < 3; j++) 
			{
				mtx[j][i] =  - c1 * u[i] * u[j] - c2 * v[i] * v[j] + c3 * v[i] * u[j];
			}
			mtx[i][i] += 1.0f;
		}
	}
	else  /* the most common case, unless "from"="to", or "from"=-"to" */
	{
		h = 1.0f/(1.0f + e);      /* optimization by Gottfried Chen */
		mtx[0][0] = e + h * v[0] * v[0];
		mtx[1][0] = h * v[0] * v[1] - v[2];
		mtx[2][0] = h * v[0] * v[2] + v[1];

		mtx[0][1] = h * v[0] * v[1] + v[2];
		mtx[1][1] = e + h * v[1] * v[1];
		mtx[2][1] = h * v[1] * v[2] - v[0];

		mtx[0][2] = h * v[0] * v[2] - v[1];
		mtx[1][2] = h * v[1] * v[2] + v[0];
		mtx[2][2] = e + h * v[2] * v[2];
	}

	return mtx;
}

mat4 frameFromNormal( const vec3& z )
{
	SLMATH_VEC_ASSERT( fabsf(length(z)-1.f) < 1e-6f );

	vec3 y = vec3( -z.z, z.x, -z.y );
	y = normalize( y - z*dot(z,y) );
	vec3 x = cross( z, y );
	
	mat4 m;
	m[0].xyz() = x; m[0].w = 0.f;
	m[1].xyz() = y; m[1].w = 0.f;
	m[2].xyz() = z; m[2].w = 0.f;
	m[3] = vec4(0,0,0,1.f);

	SLMATH_VEC_ASSERT( dot(x,y) < 1e-6f );
	SLMATH_VEC_ASSERT( dot(y,z) < 1e-6f );
	SLMATH_VEC_ASSERT( dot(x,z) < 1e-6f );
	return m;
}

/**
 * Orthonormal projection setup implementation for LH/RH.
 * @param handedness 1.f for RH, -1.f for LH 
 */
static inline mat4 ortho( float w, float h, float znear, float zfar, float sign )
{
	SLMATH_VEC_ASSERT( w > FLT_MIN );
	SLMATH_VEC_ASSERT( h > FLT_MIN );
	SLMATH_VEC_ASSERT( znear > FLT_MIN );
	SLMATH_VEC_ASSERT( fabsf(znear-zfar) > FLT_MIN );

	const float dz = (znear - zfar) * sign;
	const float dzi = 1.f/dz;

	mat4 m;
	m[0] = vec4( 2.f/w, 0, 0, 0 );
	m[1] = vec4( 0, 2.f/h, 0, 0 );
	m[2] = vec4( 0, 0, dzi, 0 );
	m[3] = vec4( 0, 0, znear*dzi * sign, 1.f );
	return m;
}

mat4 orthoRH( float w, float h, float znear, float zfar )
{
	return ortho( w, h, znear, zfar, 1.f );
}

mat4 orthoLH( float w, float h, float znear, float zfar )
{
	return ortho( w, h, znear, zfar, -1.f );
}

mat4 outerProduct( const vec4& a, const vec4& b )
{
	mat4 o;
	m128_t* const o128 = o.m128();
	o128[0] = SLMATH_MUL_PS( SLMATH_LOAD_PS1(&a[0]), b.m128() );
	o128[1] = SLMATH_MUL_PS( SLMATH_LOAD_PS1(&a[1]), b.m128() );
	o128[2] = SLMATH_MUL_PS( SLMATH_LOAD_PS1(&a[2]), b.m128() );
	o128[3] = SLMATH_MUL_PS( SLMATH_LOAD_PS1(&a[3]), b.m128() );
	return o;
}

SLMATH_END()

// This file is part of 'slm' C++ library. Copyright (C) 2009 Jani Kajala (kajala@gmail.com). See http://sourceforge.net/projects/slm/
