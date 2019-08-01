// CQ#366531
// RUN: %clang_cc1 -fintel-compatibility -I. -I%S/Inputs/include_next -verify -DOK %s
// RUN: %clang_cc1 -I. -I%S/Inputs/include_next -verify -DNOT_OK %s

#include <x_include_next_intel_compat.h>

#ifndef INCLUDE_NEXT_INTEL_COMPAT_H
#error INCLUDE_NEXT_INTEL_COMPAT_H not defined
#endif // not INCLUDE_NEXT_INTEL_COMPAT_H

#ifndef INC_INCLUDE_NEXT_INTEL_COMPAT_H
#error INC_INCLUDE_NEXT_INTEL_COMPAT_H not defined
#endif // not INC_INCLUDE_NEXT_INTEL_COMPAT_H
