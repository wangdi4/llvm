//==-------------  Tests for OpenMP support -*- OpenCL -*-------------------==//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef _TYPE_DEFS_H_
#define _TYPE_DEFS_H_

#if defined(DTYPE_SHORT) && (DTYPE_SHORT > 0)
typedef short TYPE;
#define FORMAT "%d"
#define EPSILON 0
#elif defined(DTYPE_UINT) && (DTYPE_UINT > 0)
typedef unsigned TYPE;
#define FORMAT "%u"
#define EPSILON 0
#elif defined(DTYPE_FLOAT) && (DTYPE_FLOAT > 0)
typedef float TYPE;
#define FORMAT "%f"
#define EPSILON 0.00001f
#elif defined(DTYPE_DOUBLE) && (DTYPE_DOUBLE > 0)
typedef float TYPE;
#define FORMAT "%lf"
#define EPSILON 0.0000001f
#elif defined(DTYPE_LONG) && (DTYPE_LONG > 0)
typedef long TYPE;
#define FORMAT "%l"
#define EPSILON 0
#else
// Default is int
typedef int TYPE;
#define FORMAT "%d"
#define EPSILON 0
#endif

#endif // _TYPE_DEFS_H_
