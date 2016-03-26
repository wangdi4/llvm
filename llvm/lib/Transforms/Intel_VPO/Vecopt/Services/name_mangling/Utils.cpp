/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "Utils.h"
#include <cassert>
#include <sstream>
#include <string>

namespace reflection {

  //string represenration for the primitive types
  static const char* PrimitiveNames[PRIMITIVE_NUM] ={
    "bool",
    "uchar",
    "char",
    "ushort",
    "short",
    "uint",
    "int",
    "ulong",
    "long",
    "half",
    "float",
    "double",
    "void",
    "...",
  };

  const char* mangledTypes[PRIMITIVE_NUM] = {
    "b",  //BOOL
    "h",  //UCHAR
    "c",  //CHAR
    "t",  //USHORT
    "s",  //SHORT
    "j",  //UINT
    "i",  //INT
    "m",  //ULONG
    "l",  //LONG
    "Dh", //HALF
    "f",  //FLOAT
    "d",  //DOUBLE
    "v",  //VOID
    "z",  //VarArg
  };

  const char* readableAttribute[ATTR_NUM] = {
    "restrict",
    "volatile",
    "const"
  };

  const char* mangledAttribute[ATTR_NUM] = {
    "r",
    "V",
    "K"
  };

  const char* mangledPrimitiveString(TypePrimitiveEnum t) {
    return mangledTypes[t];
  }

  const char* readablePrimitiveString(TypePrimitiveEnum t) {
    return PrimitiveNames[t];
  }

  std::string llvmPrimitiveString(TypePrimitiveEnum t) {
    assert(t >= PRIMITIVE_STRUCT_FIRST && t <= PRIMITIVE_STRUCT_LAST &&
      "assuming struct primitive type only!");
    return std::string("opencl.") + std::string(PrimitiveNames[t]);
  }

  std::string getMangledAttribute(TypeAttributeEnum attribute) {
    return mangledAttribute[attribute];
  }

  std::string getReadableAttribute(TypeAttributeEnum attribute) {
    return readableAttribute[attribute];
  }

  std::string getDuplicateString(int index) {
    assert (index >= 0 && "illegal index");
    if (0 == index)
      return "S_";
    std::stringstream ss;
    ss << "S" << index-1 << "_";
    return ss.str();
  }

} // namespace reflection {
