//===- CoerceTypes.h - Coerce Types pass C++ -*---------------------------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_COERCE_TYPES_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_COERCE_TYPES_H

#include "llvm/IR/PassManager.h"

namespace llvm {

class CoerceTypesPass : public PassInfoMixin<CoerceTypesPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  bool runImpl(Module &M);

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

private:
  Module *PModule = nullptr;
  const DataLayout *PDataLayout = nullptr;
  DenseMap<Function *, Function *> FunctionMap;
};
} // namespace llvm
#endif // LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_COERCE_TYPES_H
