#ifndef INC_INCLUDE_NEXT_INTEL_COMPAT_H
#define INC_INCLUDE_NEXT_INTEL_COMPAT_H
#endif // INC_INCLUDE_NEXT_INTEL_COMPAT_H

#ifndef INCLUDE_NEXT_INTEL_COMPAT_H
#define INCLUDE_NEXT_INTEL_COMPAT_H
#ifdef OK
#include_next <include_next_intel_compat.h> // expected-warning {{#include_next with absolute path}}
#else
#include_next <include_next_intel_compat.h> // expected-error{{'include_next_intel_compat.h' file not found}}
#endif // OK
#endif // not INCLUDE_NEXT_INTEL_COMPAT_H

