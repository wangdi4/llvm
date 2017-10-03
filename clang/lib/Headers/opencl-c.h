//===--- opencl-c.h - OpenCL C language builtin function header -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef _OPENCL_H_
#define _OPENCL_H_

#include "opencl-c-common.h"
#include "opencl-c-platform.h"

#if __OPENCL_C_VERSION__ >= CL_VERSION_2_0
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
