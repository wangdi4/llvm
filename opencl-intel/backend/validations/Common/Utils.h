// INTEL CONFIDENTIAL
//
// Copyright 2010 Intel Corporation.
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

#ifndef __UTILS_H__
#define __UTILS_H__

#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Type.h"
#include "llvm/Transforms/SYCLTransforms/Utils/ParameterType.h"

namespace Validation {
// this function is used to calculate ilogb(2m-1) - the number of
// least significant bits of each mask element for built-ins shuffle and
// shuffle2
inline int shuffleGetNumMaskBits(unsigned length) {
  int numBits = 0;
  switch (length) {
  case 1:
    numBits = 1;
    break;
  case 2:
    numBits = 1;
    break;
  case 3:
    numBits = 2;
    break;
  case 4:
    numBits = 2;
    break;
  case 8:
    numBits = 3;
    break;
  case 16:
    numBits = 4;
    break;
  default:
    throw Exception::InvalidArgument(
        "[shuffleGetNumMaskBits] Wrong vector size");
    break;
  }
  return numBits;
}

inline llvm::Type *parseLLVMTypeFromName(llvm::LLVMContext &C,
                                         llvm::StringRef TypeName) {
  llvm::Type *Ty = nullptr;

  if (TypeName == "half")
    Ty = llvm::Type::getHalfTy(C);
  else if (TypeName == "float")
    Ty = llvm::Type::getFloatTy(C);
  else if (TypeName == "double")
    Ty = llvm::Type::getDoubleTy(C);
  else if (TypeName.contains("bool"))
    Ty = llvm::Type::getInt1Ty(C);
  else if (TypeName.contains("char"))
    Ty = llvm::Type::getInt8Ty(C);
  else if (TypeName.contains("short"))
    Ty = llvm::Type::getInt16Ty(C);
  else if (TypeName.contains("int"))
    Ty = llvm::Type::getInt32Ty(C);
  else if (TypeName.contains("long"))
    Ty = llvm::Type::getInt64Ty(C);
  else
    assert(false && "Unhandled type name");

  return Ty;
}

} // namespace Validation
#endif // __UTILS_H__
