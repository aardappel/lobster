#ifndef SLMATH_SIMD_H
#define SLMATH_SIMD_H

#ifndef SLMATH_PP_H
#include <slm/slmath_pp.h>
#endif

#undef SLMATH_ALIGN16
#undef SLMATH_MUL_PS
#undef SLMATH_ADD_PS
#undef SLMATH_SUB_PS
#undef SLMATH_LOAD_PS1

#ifdef SLMATH_SSE2_MSVC
	#define SLMATH_ALIGN16 __declspec(align(16))
#else
	#define SLMATH_ALIGN16
#endif

#if defined(SLMATH_SSE2_MSVC)
	#include <xmmintrin.h>

	SLMATH_BEGIN()
		typedef __m128 m128_t;
	SLMATH_END()

	#define SLMATH_MUL_PS(A,B) _mm_mul_ps(A,B)
	#define SLMATH_ADD_PS(A,B) _mm_add_ps(A,B)
	#define SLMATH_SUB_PS(A,B) _mm_sub_ps(A,B)
	#define SLMATH_DIV_PS(A,B) _mm_div_ps(A,B)
	#define SLMATH_SETZERO_PS() _mm_setzero_ps()
	#define SLMATH_LOAD_PS1(A) _mm_load_ps1(A)
	#define SLMATH_MIN_PS(A,B) _mm_min_ps(A,B)
	#define SLMATH_MAX_PS(A,B) _mm_max_ps(A,B)
#else 
	// SIMD emulation with standard C++, so you can still use SIMD-macros even without SIMD support if you want
	#undef SLMATH_SIMD

	SLMATH_BEGIN()
		SLMATH_ALIGN16 struct m128_emu
		{
			float m[4]; 
			
			m128_emu() {} 
			m128_emu(float x) {m[0]=m[1]=m[2]=m[3]=x;} 
			m128_emu(float x,float y,float z,float w) {m[0]=x;m[1]=y;m[2]=z;m[3]=w;} 
		};
		typedef m128_emu m128_t;
	SLMATH_END()

	#define SLMATH_MUL_PS(A,B) SLMATH_NS(m128_emu)( (A).m[0]*(B).m[0], (A).m[1]*(B).m[1], (A).m[2]*(B).m[2], (A).m[3]*(B).m[3] )
	#define SLMATH_ADD_PS(A,B) SLMATH_NS(m128_emu)( (A).m[0]+(B).m[0], (A).m[1]+(B).m[1], (A).m[2]+(B).m[2], (A).m[3]+(B).m[3] )
	#define SLMATH_SUB_PS(A,B) SLMATH_NS(m128_emu)( (A).m[0]-(B).m[0], (A).m[1]-(B).m[1], (A).m[2]-(B).m[2], (A).m[3]-(B).m[3] )
	#define SLMATH_DIV_PS(A,B) SLMATH_NS(m128_emu)( (A).m[0]/(B).m[0], (A).m[1]/(B).m[1], (A).m[2]/(B).m[2], (A).m[3]/(B).m[3] )
	#define SLMATH_SETZERO_PS() SLMATH_NS(m128_emu)( 0.f )
	#define SLMATH_LOAD_PS1(A) SLMATH_NS(m128_emu)( *(A) )
	#define SLMATH_MIN_PS(A,B) SLMATH_NS(m128_emu)( (A).m[0]<(B).m[0]?(A).m[0]:(B).m[0], (A).m[1]<(B).m[1]?(A).m[1]:(B).m[1], (A).m[2]<(B).m[2]?(A).m[2]:(B).m[2], (A).m[3]<(B).m[3]?(A).m[3]:(B).m[3] )
	#define SLMATH_MAX_PS(A,B) SLMATH_NS(m128_emu)( (A).m[0]<(B).m[0]?(B).m[0]:(A).m[0], (A).m[1]<(B).m[1]?(B).m[1]:(A).m[1], (A).m[2]<(B).m[2]?(B).m[2]:(A).m[2], (A).m[3]<(B).m[3]?(B).m[3]:(A).m[3] )
#endif

#endif

// This file is part of 'slm' C++ library. Copyright (C) 2009 Jani Kajala (kajala@gmail.com). See http://sourceforge.net/projects/slm/
