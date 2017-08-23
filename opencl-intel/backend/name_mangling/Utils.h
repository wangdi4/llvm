/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef __MANGLING_UTILS_H__
#define __MANGLING_UTILS_H__

#include "ParameterType.h"
#include <string>

// Current implementation in open source has OpenCL types substitution enabled.
// Originally I tried to fix the test w/o modifying clang behavior, but it will
// require additional processing SPIR 1.2 code that doesn't substitue OpenCL
// types. Once we drop support for old SPIR format in favor of SPIR-V format we
// can enable this macro.
//#define SUBSTITUTE_OPENCL_TYPES 1

namespace reflection {

const char *mangledPrimitiveString(TypePrimitiveEnum primitive);
const char *readablePrimitiveString(TypePrimitiveEnum primitive);
std::string llvmPrimitiveString(TypePrimitiveEnum primitive);

std::string getMangledAttribute(TypeAttributeEnum attribute);
std::string getReadableAttribute(TypeAttributeEnum attribute);

std::string getDuplicateString(int index);
}

#endif //__MANGLING_UTILS_H__
