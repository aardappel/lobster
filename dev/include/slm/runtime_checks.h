#ifndef SLMATH_RUNTIME_CHECKS_H
#define SLMATH_RUNTIME_CHECKS_H

#include <slm/slmath_pp.h>

SLMATH_BEGIN()

/**
 * \defgroup runtime_checks Runtime CPU/environment information.
 * @ingroup slm
 */
/*@{*/

/** Returns true if CPU supports SSE2 instruction set. */
bool isSSE2CPU();

/** Returns true if current compilation options match platform capabilities. */
bool isValidCPU();

/*@}*/

#include <slm/runtime_checks.inl>

SLMATH_END()

#endif

// This file is part of 'slm' C++ library. Copyright (C) 2009 Jani Kajala (kajala@gmail.com). See http://sourceforge.net/projects/slm/