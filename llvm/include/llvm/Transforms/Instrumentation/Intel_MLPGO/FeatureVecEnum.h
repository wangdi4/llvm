//===- FeatureVecEnum.h - Feature Vector Enum -------------------*- C++ -*-===//
//
// Copyright (C) 2023 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
//===----------------------------------------------------------------------===//

#ifndef TRANSFORMS_INSTRUMENTATIONS_MLPGO_INTEL_FEATUREVECNUM_H
#define TRANSFORMS_INSTRUMENTATIONS_MLPGO_INTEL_FEATUREVECNUM_H

#include <stdint.h>

namespace llvm {
namespace mlpgo {

enum BranchDirection : uint8_t { Forward, Backward };

enum TypeDescIdx : unsigned {
  HalfTy,
  BFloatTy,
  FloatTy,
  DoubleTy,
  X86_FP80Ty,
  FP128Ty,
  PPC_FP128Ty,
  X86_MMXTy,
  X86_AMXTy,
  Int1Ty,
  Int8Ty,
  Int16Ty,
  Int32Ty,
  Int64Ty,
  Int128Ty,
  BigIntTy, // > 128 Bits
  VariableSizeTy,
  PrimTySize,
  PointerTy,
  EmptyTy = PrimTySize * 5,
  LabelTy
};

enum ProcedureType : uint8_t {
  Leaf = 0,
  NonLeaf,
  CallSelf,
  OnlyIntrinsic,
  None
};

enum SuccessorEndKind : uint8_t {
  Nothing,
  FT,     // Fall Through
  CBR,    // Conditional Branch
  SWITCH, // Switch
  IVK,    // invoke
  UBR,    // Unconditional Branch
  IJUMP,  // indirect jump
  IJSR,   // Indirect Jump Subroutines
  Ret,    // Return
  Resume, // Resume
};

enum ExtendOpFunc {
#define LAST_OTHER_INST(N) OtherOpsEndOpFunc = N + 1,
#include "llvm/IR/Instruction.def"
  BadOpFunc,
  ConstantZeroOpFunc,
  ConstantOpFunc,
  ConstantOneFunc,
  ConstantMinusOneFunc,

  /// previously constant Zero Op Func only denotes that it will satisfy the
  /// Zero Heuristic in LLVM. The constant Only Zero Func means that the
  /// constant is indeed zero, but other conditions of Zero Heuristic are not
  /// satisfied. The same with only one and only minus one.
  ConstantOnlyZeroFunc,
  ConstantOnlyOneFunc,
  ConstantOnlyMinusOneFunc,

  LoadGlobalOpFunc
};

enum SuccessorExitEdgeType : unsigned {
  NLE,
  LE,
  SiblingLE,
};

enum SuccessorUnlikelyType : unsigned {
  Normallikely,
  Unlikely,
  SiblingUnlikely,
};

} // end namespace mlpgo
} // end namespace llvm

#endif // TRANSFORMS_INSTRUMENTATIONS_MLPGO_INTEL_FEATUREVECNUM_H
