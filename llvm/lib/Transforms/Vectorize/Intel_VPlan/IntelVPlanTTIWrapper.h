//===------------------------------------------------------------*- C++ -*-===//
////
////   Copyright (C) 2020-2020 Intel Corporation. All rights reserved.
////
////   The information and source code contained herein is the exclusive
////   property of Intel Corporation and may not be disclosed, examined
////   or reproduced in whole or in part without explicit written authorization
////   from the company.
////
////===--------------------------------------------------------------------===//
/////
///// \file
///// This file defines the wrapper for TTI interfaces to provide scaling up of
///// cost values.
////===--------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANTTIWRAPPER_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANTTIWRAPPER_H

#include "llvm/Analysis/TargetTransformInfo.h"

namespace llvm {

namespace vpo {

/// Do not inherit from TargetTransformInfo to avoid accidental
/// usage of values that haven't been scaled.
class VPlanTTIWrapper {
public:
  /// Scale up factor for native TargetTransformInfo.
  static constexpr unsigned Multiplier = 1000;

  static constexpr unsigned estimateNumberOfInstructions(unsigned ScaledCost) {
    return ScaledCost / Multiplier;
  }

  explicit VPlanTTIWrapper(const TargetTransformInfo &TTI, const DataLayout &DL)
      : TTI(TTI), DL(DL) {}

  /// Required to pass actual TTI along. Ideally it is not exposed.
  const TargetTransformInfo &getTTI() const { return TTI; }

  /// TTI methods.
  /// Only the methods required by VPlan Cost Model are listed here.

  unsigned getNumberOfRegisters(unsigned ClassID) const {
    return TTI.getNumberOfRegisters(ClassID);
  }
  unsigned getRegisterClassForType(bool Vector, Type *Ty = nullptr) const {
    return TTI.getRegisterClassForType(Vector, Ty);
  }
  unsigned getRegisterBitWidth(TargetTransformInfo::RegisterKind K) const {
    return TTI.getRegisterBitWidth(K).getFixedSize();
  }
  unsigned getCacheLineSize() const { return TTI.getCacheLineSize(); }
  Optional<unsigned> getCacheSize(TargetTransformInfo::CacheLevel Level) const {
    return TTI.getCacheSize(Level);
  }
  bool isLegalMaskedStore(Type *DataType, Align Alignment) const {
    return TTI.isLegalMaskedStore(DataType, Alignment);
  }
  bool isLegalMaskedLoad(Type *DataType, Align Alignment) const {
    return TTI.isLegalMaskedLoad(DataType, Alignment);
  }
  bool isLegalMaskedScatter(Type *DataType, Align Alignment) const {
    return TTI.isLegalMaskedScatter(DataType, Alignment);
  }
  bool isLegalMaskedGather(Type *DataType, Align Alignment) const {
    return TTI.isLegalMaskedGather(DataType, Alignment);
  }
  unsigned getArithmeticInstrCost(
      unsigned Opcode, Type *Ty,
      TTI::TargetCostKind CostKind = TTI::TCK_RecipThroughput,
      TTI::OperandValueKind Opd1Info = TTI::OK_AnyValue,
      TTI::OperandValueKind Opd2Info = TTI::OK_AnyValue,
      TTI::OperandValueProperties Opd1PropInfo = TTI::OP_None,
      TTI::OperandValueProperties Opd2PropInfo = TTI::OP_None,
      ArrayRef<const Value *> Args = ArrayRef<const Value *>(),
      const Instruction *CxtI = nullptr) const {
    return Multiplier *
           TTI.getArithmeticInstrCost(Opcode, Ty, CostKind, Opd1Info, Opd2Info,
                                      Opd1PropInfo, Opd2PropInfo, Args, CxtI);
  }
  int getShuffleCost(TargetTransformInfo::ShuffleKind Kind, VectorType *Tp,
                     int Index = 0, VectorType *SubTp = nullptr) const {
    return Multiplier * TTI.getShuffleCost(Kind, Tp, llvm::None, Index, SubTp);
  }
  int getCastInstrCost(unsigned Opcode, Type *Dst, Type *Src,
                       TTI::CastContextHint CCH,
                       TTI::TargetCostKind CostKind = TTI::TCK_SizeAndLatency,
                       const Instruction *I = nullptr) const {
    return Multiplier *
           *TTI.getCastInstrCost(Opcode, Dst, Src, CCH, CostKind, I).getValue();
  }
  int getCmpSelInstrCost(
      unsigned Opcode, Type *ValTy, Type *CondTy = nullptr,
      CmpInst::Predicate VecPred = CmpInst::BAD_ICMP_PREDICATE,
      TTI::TargetCostKind CostKind = TTI::TCK_RecipThroughput,
      const Instruction *I = nullptr) const {
    return Multiplier *
           TTI.getCmpSelInstrCost(Opcode, ValTy, CondTy, VecPred, CostKind, I);
  }

  int getVectorInstrCost(unsigned Opcode, Type *Val,
                         unsigned Index = -1) const {
    return Multiplier * TTI.getVectorInstrCost(Opcode, Val, Index);
  }
  int getMemoryOpCost(unsigned Opcode, Type *Src, Align Alignment,
                      unsigned AddressSpace,
                      TTI::TargetCostKind CostKind = TTI::TCK_RecipThroughput,
                      const Instruction *I = nullptr) const;
  int getMaskedMemoryOpCost(
      unsigned Opcode, Type *Src, Align Alignment, unsigned AddressSpace,
      TTI::TargetCostKind CostKind = TTI::TCK_RecipThroughput) const {
    return Multiplier * TTI.getMaskedMemoryOpCost(Opcode, Src, Alignment,
                                                  AddressSpace, CostKind);
  }
  int getGatherScatterOpCost(
      unsigned Opcode, Type *DataTy, const Value *Ptr, bool VariableMask,
      Align Alignment, TTI::TargetCostKind CostKind = TTI::TCK_RecipThroughput,
      const Instruction *I = nullptr) const {
    return Multiplier * *TTI.getGatherScatterOpCost(Opcode, DataTy, Ptr,
                                                    VariableMask, Alignment,
                                                    CostKind, I)
                             .getValue();
  }
#if INTEL_CUSTOMIZATION
  int getGatherScatterOpCost(
      unsigned Opcode, Type *DataTy, unsigned IndexSize, bool VariableMask,
      unsigned Alignment, unsigned AddressSpace,
      TTI::TargetCostKind CostKind = TTI::TCK_RecipThroughput,
      const Instruction *I = nullptr) const {
    return Multiplier * TTI.getGatherScatterOpCost(Opcode, DataTy, IndexSize,
                                                   VariableMask, Alignment,
                                                   AddressSpace, CostKind, I);
  }
  int getInterleavedMemoryOpCost(unsigned Opcode, Type *VecTy, unsigned Factor,
                                 ArrayRef<unsigned> Indices, Align Alignment,
                                 unsigned AddressSpace,
                                 TTI::TargetCostKind CostKind,
                                 bool UseMaskForCond,
                                 bool UseMaskForGaps) const {
    return Multiplier *
           TTI.getInterleavedMemoryOpCost(Opcode, VecTy, Factor, Indices,
                                          Alignment, AddressSpace, CostKind,
                                          UseMaskForCond, UseMaskForGaps);
  }
#endif // INTEL_CUSTOMIZATION
  int getIntrinsicInstrCost(const IntrinsicCostAttributes &ICA,
                            TTI::TargetCostKind CostKind) const {
    return Multiplier * *TTI.getIntrinsicInstrCost(ICA, CostKind).getValue();
  }
  unsigned getNumberOfParts(Type *Tp) const { return TTI.getNumberOfParts(Tp); }
#if INTEL_CUSTOMIZATION
  bool isVPlanVLSProfitable() const { return TTI.isVPlanVLSProfitable(); }

  bool isAggressiveVLSProfitable() const {
    return TTI.isAggressiveVLSProfitable();
  }

#endif // INTEL_CUSTOMIZATION
  unsigned getLoadStoreVecRegBitWidth(unsigned AddrSpace) const {
    return TTI.getLoadStoreVecRegBitWidth(AddrSpace);
  }

protected:
  const TargetTransformInfo &TTI;
  const DataLayout &DL;

protected:
  // Determine cost adjustment for a memref with specific Alignment.
  int getNonMaskedMemOpCostAdj(unsigned Opcode, Type *Src,
                                Align Alignment) const;
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANTTIWRAPPER_H
