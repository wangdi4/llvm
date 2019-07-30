// INTEL CONFIDENTIAL
//
// Copyright 2019 Intel Corporation.
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

#ifndef __COERCE_TYPES_H__
#define __COERCE_TYPES_H__

#include "llvm/Pass.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"

namespace intel {
using namespace llvm;

class CoerceTypes : public ModulePass {
public:
  static char ID;

  CoerceTypes();

  llvm::StringRef getPassName() const override { return "CoerceTypes"; }

  bool runOnModule(Module &M) override;

protected:
  // Applicable X86_64 ABI classes, in the increasing order of their preference
  // during merging
  enum class TypeClass { NO_CLASS, SSE, INTEGER, MEMORY };

  using ClassPair = std::pair<TypeClass, TypeClass>;
  using TypePair = std::pair<Type *, Type *>;

  bool runOnFunction(Function *F);

  // Checks if F is supported by the pass
  bool isFunctionSupported(Function *F);

  // Get coerced type(s) that are guaranteed to be passed in the correct
  // registers
  TypePair getCoercedType(Argument *T, unsigned &FreeIntRegs,
                          unsigned &FreeSSERegs) const;

  // Get coerced type for the eightbyte of T at Offset (0 or 8)
  Type *getCoercedType(StructType *T, unsigned Offset, TypeClass Class) const;

  // Get type that will be passed in an INTEGER register, for the eightbyte of T
  // at Offset
  Type *getIntegerType(StructType *T, unsigned Offset) const;

  // Get type that will be passed in an SSE register, for the eightbyte of T at
  // Offset
  Type *getSSEType(StructType *T, unsigned Offset) const;

  // Recurse into T to find its non-composite field type that starts exactly at
  // Offset, returns nullptr if not applicable
  Type *getNonCompositeTypeAtExactOffset(Type *T, unsigned Offset) const;

  // Classify T according to the X86_64 ABI algorithm. Both Offset and the
  // returned class pair are relative to the top-level struct. Assumes that T is
  // no more than 16 bytes.
  ClassPair classify(Type *T, unsigned Offset = 0) const;

  // Classify a struct type
  ClassPair classifyStruct(StructType *T, unsigned Offset = 0) const;

  // Classify a scalar type
  TypeClass classifyScalar(Type *T) const;

  // Merge classes in accordance with the X86_64 ABI algorithm
  ClassPair mergeClasses(ClassPair A, ClassPair B) const;

  // Return a single type containing both coerced eightbytes
  Type *getCombinedCoercedType(TypePair CoercedTypes,
                               StringRef OriginalTypeName) const;

  // Copy attributes and argument names from old function to the new one
  void copyAttributesAndArgNames(Function *OldF, Function *NewF,
                                 ArrayRef<TypePair> NewArgTypePairs);

  // Move old function body to the new one, replace uses of old arguments with
  // the new ones
  void moveFunctionBody(Function *OldF, Function *NewF,
                        ArrayRef<TypePair> NewArgTypePairs);

  Module *m_pModule;
  const DataLayout *m_pDataLayout;
  DenseMap<Function *, Function *> m_FunctionMap;
};
}
#endif // __COERCE_TYPES_H__
