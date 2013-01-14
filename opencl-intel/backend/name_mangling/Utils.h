/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "Type.h"
#include <string>

#ifndef __MANGLING_UTILS_H__
#define __MANGLING_UTILS_H__

namespace reflection {
const char* mangledString(const Type*);
const char* readableString(const Type*);
primitives::Primitive parseType(const std::string& s);

}

#endif//__MANGLING_UTILS_H__
