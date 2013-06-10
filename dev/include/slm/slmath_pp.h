#ifndef SLMATH_PP_H
#define SLMATH_PP_H

#include <math.h>
#include <float.h>
#include <assert.h>
#include <stddef.h>
#include <slm/slmath_configure.h>

// make sure <windows.h> does not define min and max as macros
#undef min
#undef max
#ifndef NOMINMAX
#define NOMINMAX
#endif

// Use this instead of namespaces directly to enable easier porting for platforms without namespace support
#ifdef SLMATH_NAMESPACE
	#define SLMATH_BEGIN() namespace slm {
	#define SLMATH_END() }
	#define SLMATH_USING() using namespace slm;
	#define SLMATH_NS(A) slm::A
	namespace slm {}
#else
	#define SLMATH_BEGIN()
	#define SLMATH_END()
	#define SLMATH_USING()
	#define SLMATH_NS(A) A
#endif

// Verify requested configuration for this build:
// SSE2 extensions supported for Visual Studio 2005 and newer
#if defined(SLMATH_SIMD)
	#if (_MSC_VER >= 1500)
		#define SLMATH_SSE2_MSVC
		#define SLMATH_MSVC_HAS_INTRIN_H
	#elif (_MSC_VER >= 1300)
		// Enable SSE2 in Visual Studio 2003
		// <intrin.h> is not available.
		#define SLMATH_SSE2_MSVC
	#endif
#endif

// Auto-link library on MSVC if SLMATH_AUTOLINK defined
// Note: Disabled by default to avoid forcing user to link to this if e.g. user just adds sources to his project
#ifdef SLMATH_AUTOLINK
	#ifdef _MSC_VER
		#ifdef _DEBUG
			#pragma comment( lib, "slmath-Debug.lib" )
		#else
			#pragma comment( lib, "slmath-Release.lib" )
		#endif
	#endif
#endif

// Assert used in vec-ops in _DEBUG build
#ifdef SLMATH_VEC_ASSERTS
	#define SLMATH_VEC_ASSERT(V) assert( V )
#else
	#define SLMATH_VEC_ASSERT(V)
#endif

// SIMD macros
#include <slm/simd.h>

#endif

// This file is part of 'slm' C++ library. Copyright (C) 2009 Jani Kajala (kajala@gmail.com). See http://sourceforge.net/projects/slm/
