//===--- CSASetIntrinsicFunctionAttributes.h --*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This pass sets the attributes of intrinsic functions to indicate that they
// do not change any global state. Specifically, they do not set errno. This
// allows later passes to convert the calls to builtin instructions.
//
//===----------------------------------------------------------------------===//

#ifndef CSA_SET_INTRINSIC_FUNCTION_ATTRIBUTES_H
#define CSA_SET_INTRINSIC_FUNCTION_ATTRIBUTES_H

namespace llvm {
  class FunctionPass;

  FunctionPass *createCSASetIntrinsicFunctionAttributesPass();

} // end namespace llvm;


#endif // CSA_SET_INTRINSIC_FUNCTION_ATTRIBUTES_H
