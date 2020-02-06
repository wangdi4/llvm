//===--- opencl-c.h - OpenCL C language builtin function header -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _OPENCL_H_
#define _OPENCL_H_

#include "opencl-c-common.h"
#include "opencl-c-platform.h"

#if defined(__OPENCL_CPP_VERSION__) || (__OPENCL_C_VERSION__ >= CL_VERSION_2_0)
#include "opencl-c-20.h"
#include "opencl-c-platform-20.h"
#else
#include "opencl-c-platform-12.h"
#endif

// Disable any extensions we may have enabled previously.
#pragma OPENCL EXTENSION all : disable

#undef __purefn
#undef __cnfn
#undef __ovld

#endif //_OPENCL_H_
