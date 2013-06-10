#ifndef SLMATH_NO_SIMD_H
#define SLMATH_NO_SIMD_H
// Routines optimized for non-SIMD platforms

/** Multiplies mat4 m with mat4 o and store results to res. */
#define MAT4_MUL_MAT4( res, m, o ) \
	res[0].x = m[0].x * o[0].x  +  m[1].x * o[0].y  +  m[2].x * o[0].z + m[3].x * o[0].w;\
	res[1].x = m[0].x * o[1].x  +  m[1].x * o[1].y  +  m[2].x * o[1].z + m[3].x * o[1].w;\
	res[2].x = m[0].x * o[2].x  +  m[1].x * o[2].y  +  m[2].x * o[2].z + m[3].x * o[2].w;\
	res[3].x = m[0].x * o[3].x  +  m[1].x * o[3].y  +  m[2].x * o[3].z + m[3].x * o[3].w;\
	res[0].y = m[0].y * o[0].x  +  m[1].y * o[0].y  +  m[2].y * o[0].z + m[3].y * o[0].w;\
	res[1].y = m[0].y * o[1].x  +  m[1].y * o[1].y  +  m[2].y * o[1].z + m[3].y * o[1].w;\
	res[2].y = m[0].y * o[2].x  +  m[1].y * o[2].y  +  m[2].y * o[2].z + m[3].y * o[2].w;\
	res[3].y = m[0].y * o[3].x  +  m[1].y * o[3].y  +  m[2].y * o[3].z + m[3].y * o[3].w;\
	res[0].z = m[0].z * o[0].x  +  m[1].z * o[0].y  +  m[2].z * o[0].z + m[3].z * o[0].w;\
	res[1].z = m[0].z * o[1].x  +  m[1].z * o[1].y  +  m[2].z * o[1].z + m[3].z * o[1].w;\
	res[2].z = m[0].z * o[2].x  +  m[1].z * o[2].y  +  m[2].z * o[2].z + m[3].z * o[2].w;\
	res[3].z = m[0].z * o[3].x  +  m[1].z * o[3].y  +  m[2].z * o[3].z + m[3].z * o[3].w;\
	res[0].w = m[0].w * o[0].x  +  m[1].w * o[0].y  +  m[2].w * o[0].z + m[3].w * o[0].w;\
	res[1].w = m[0].w * o[1].x  +  m[1].w * o[1].y  +  m[2].w * o[1].z + m[3].w * o[1].w;\
	res[2].w = m[0].w * o[2].x  +  m[1].w * o[2].y  +  m[2].w * o[2].z + m[3].w * o[2].w;\
	res[3].w = m[0].w * o[3].x  +  m[1].w * o[3].y  +  m[2].w * o[3].z + m[3].w * o[3].w;	

#endif // SLMATH_NO_SIMD_H

// This file is part of 'slmath' C++ library. Copyright (C) 2009 Jani Kajala (kajala@gmail.com). See http://sourceforge.net/projects/slmath/
