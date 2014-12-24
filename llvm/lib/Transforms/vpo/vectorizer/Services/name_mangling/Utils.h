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

namespace reflection {

  const char* mangledPrimitiveString(TypePrimitiveEnum primitive);
  const char* readablePrimitiveString(TypePrimitiveEnum primitive);
  std::string llvmPrimitiveString(TypePrimitiveEnum primitive);

  std::string getMangledAttribute(TypeAttributeEnum attribute);
  std::string getReadableAttribute(TypeAttributeEnum attribute);

  std::string getDuplicateString(int index);

}

#endif //__MANGLING_UTILS_H__
