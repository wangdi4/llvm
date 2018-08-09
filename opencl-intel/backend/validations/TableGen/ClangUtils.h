// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#ifndef __CLANG_UTILS_H__
#define __CLANG_UTILS_H__

#include <string>
#include "OclBuiltinEmitter.h"

#define XSTR(A) STR(A)
#define STR(A) #A

///////////////////////////////////////////////////////////////////////////////
//Purpose: generates a dummy body for the given ocl (primitive) type, with the
//given vector length.
///////////////////////////////////////////////////////////////////////////////
std::string generateDummyBody(const std::string& type, size_t veclen);

///////////////////////////////////////////////////////////////////////////////
//Purpose: compiles the given ocl code (using clang), and writes the result to
//the given file name.
///////////////////////////////////////////////////////////////////////////////
void build(const std::string& code, std::string fileName);

#endif//__CLANG_UTILS_H__backend/libraries/ocl_builtins/utils/TableGen/OclBuiltinsHeaderGen.cpp
