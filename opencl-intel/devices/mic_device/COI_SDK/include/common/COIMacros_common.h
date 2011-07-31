/* ************************************************************************* *\
                INTEL CORPORATION PROPRIETARY INFORMATION
     This software is supplied under the terms of a license agreement or
     nondisclosure agreement with Intel Corporation and may not be copied
     or disclosed except in accordance with the terms of that agreement.
        Copyright (C) 2010-2011 Intel Corporation. All Rights Reserved.
\* ************************************************************************* */
#ifndef COIMACROS_COMMON_H
#define COIMACROS_COMMON_H
/// @file common/COIMacros_common.h
/// Commonly used macros

#include <stdint.h>
// Note that UNUSUED_ATTR means that it is "possibly" unused, not "definitely".
// This should compile out in release mode if indeed it is unused.
#define UNUSED_ATTR __attribute__((unused))

#ifndef UNREFERENCED_CONST_PARAM
#define UNREFERENCED_CONST_PARAM(P)     { void* x UNUSED_ATTR = \
                                                 (void*)(uint64_t)P; \
                                        }
#endif

// This seems to work on everything. 
#ifndef UNREFERENCED_PARAM
#define UNREFERENCED_PARAM(P)          (P = P)
#endif

#endif /* COIMACROS_COMMON_H */
