//===-- X86TargetTransformInfo.h - X86 specific TTI -------------*- C++ -*-===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021-2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
/// \file
/// This file a TargetTransformInfo::Concept conforming object specific to the
/// X86 target machine. It uses the target's detailed information to
/// provide more precise answers to certain TTI queries, while letting the
/// target independent and default TTI implementations handle the rest.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_X86_X86TARGETTRANSFORMINFO_H
#define LLVM_LIB_TARGET_X86_X86TARGETTRANSFORMINFO_H

#include "X86TargetMachine.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/CodeGen/BasicTTIImpl.h"

namespace llvm {

class InstCombiner;

class X86TTIImpl : public BasicTTIImplBase<X86TTIImpl> {
  typedef BasicTTIImplBase<X86TTIImpl> BaseT;
  typedef TargetTransformInfo TTI;
  friend BaseT;

  const X86Subtarget *ST;
  const X86TargetLowering *TLI;

  const X86Subtarget *getST() const { return ST; }
  const X86TargetLowering *getTLI() const { return TLI; }

  const FeatureBitset InlineFeatureIgnoreList = {
      // This indicates the CPU is 64 bit capable not that we are in 64-bit
      // mode.
      X86::FeatureX86_64,

      // These features don't have any intrinsics or ABI effect.
      X86::FeatureNOPL,
      X86::FeatureCX16,
      X86::FeatureLAHFSAHF64,

      // Some older targets can be setup to fold unaligned loads.
      X86::FeatureSSEUnalignedMem,

      // Codegen control options.
      X86::TuningFast11ByteNOP,
      X86::TuningFast15ByteNOP,
      X86::TuningFastBEXTR,
      X86::TuningFastHorizontalOps,
      X86::TuningFastLZCNT,
      X86::TuningFastScalarFSQRT,
      X86::TuningFastSHLDRotate,
      X86::TuningFastScalarShiftMasks,
      X86::TuningFastVectorShiftMasks,
      X86::TuningFastVariableCrossLaneShuffle,
      X86::TuningFastVariablePerLaneShuffle,
      X86::TuningFastVectorFSQRT,
      X86::TuningLEAForSP,
      X86::TuningLEAUsesAG,
      X86::TuningLZCNTFalseDeps,
      X86::TuningBranchFusion,
      X86::TuningMacroFusion,
#if INTEL_CUSTOMIZATION
      X86::TuningDSB,
      X86::Tuning2KDSB,
      X86::Tuning4KDSB,
      X86::TuningMULCFalseDeps,
      X86::TuningPERMFalseDeps,
      X86::TuningRANGEFalseDeps,
      X86::TuningGETMANTFalseDeps,
      X86::TuningMULLQFalseDeps,
      X86::TuningMRN,
#endif // INTEL_CUSTOMIZATION
      X86::TuningPadShortFunctions,
      X86::TuningPOPCNTFalseDeps,
      X86::TuningMULCFalseDeps,
      X86::TuningPERMFalseDeps,
      X86::TuningRANGEFalseDeps,
      X86::TuningGETMANTFalseDeps,
      X86::TuningMULLQFalseDeps,
      X86::TuningSlow3OpsLEA,
      X86::TuningSlowDivide32,
      X86::TuningSlowDivide64,
      X86::TuningSlowIncDec,
      X86::TuningSlowLEA,
      X86::TuningSlowPMADDWD,
      X86::TuningSlowPMULLD,
      X86::TuningSlowSHLD,
      X86::TuningSlowTwoMemOps,
      X86::TuningSlowUAMem16,
      X86::TuningPreferMaskRegisters,
      X86::TuningInsertVZEROUPPER,
      X86::TuningUseSLMArithCosts,
      X86::TuningUseGLMDivSqrtCosts,

      // Perf-tuning flags.
      X86::TuningFastGather,
      X86::TuningSlowUAMem32,

      // Based on whether user set the -mprefer-vector-width command line.
      X86::TuningPrefer128Bit,
      X86::TuningPrefer256Bit,

      // CPU name enums. These just follow CPU string.
      X86::ProcIntelAtom
  };

public:
  explicit X86TTIImpl(const X86TargetMachine *TM, const Function &F)
      : BaseT(TM, F.getParent()->getDataLayout()), ST(TM->getSubtargetImpl(F)),
        TLI(ST->getTargetLowering()) {}

  /// \name Scalar TTI Implementations
  /// @{
  TTI::PopcntSupportKind getPopcntSupport(unsigned TyWidth);
  unsigned getLoopRotationDefaultThreshold(bool OptForSize) const;

  /// @}

  /// \name Cache TTI Implementation
  /// @{
  llvm::Optional<unsigned> getCacheSize(
    TargetTransformInfo::CacheLevel Level) const override;
  llvm::Optional<unsigned> getCacheAssociativity(
    TargetTransformInfo::CacheLevel Level) const override;
  /// @}

  /// \name Vector TTI Implementations
  /// @{

  unsigned getNumberOfRegisters(unsigned ClassID) const;
  TypeSize getRegisterBitWidth(TargetTransformInfo::RegisterKind K) const;
  unsigned getLoadStoreVecRegBitWidth(unsigned AS) const;
  unsigned getMaxInterleaveFactor(unsigned VF);
  InstructionCost getArithmeticInstrCost(
      unsigned Opcode, Type *Ty, TTI::TargetCostKind CostKind,
      TTI::OperandValueKind Opd1Info = TTI::OK_AnyValue,
      TTI::OperandValueKind Opd2Info = TTI::OK_AnyValue,
      TTI::OperandValueProperties Opd1PropInfo = TTI::OP_None,
      TTI::OperandValueProperties Opd2PropInfo = TTI::OP_None,
      ArrayRef<const Value *> Args = ArrayRef<const Value *>(),
      const Instruction *CxtI = nullptr);
  InstructionCost getShuffleCost(TTI::ShuffleKind Kind, VectorType *Tp,
                                 ArrayRef<int> Mask, int Index,
                                 VectorType *SubTp,
                                 ArrayRef<const Value *> Args = None);
  InstructionCost getCastInstrCost(unsigned Opcode, Type *Dst, Type *Src,
                                   TTI::CastContextHint CCH,
                                   TTI::TargetCostKind CostKind,
                                   const Instruction *I = nullptr);
  InstructionCost getCmpSelInstrCost(unsigned Opcode, Type *ValTy, Type *CondTy,
                                     CmpInst::Predicate VecPred,
                                     TTI::TargetCostKind CostKind,
                                     const Instruction *I = nullptr);
  using BaseT::getVectorInstrCost;
  InstructionCost getVectorInstrCost(unsigned Opcode, Type *Val,
                                     unsigned Index);
  InstructionCost getScalarizationOverhead(VectorType *Ty,
                                           const APInt &DemandedElts,
                                           bool Insert, bool Extract);
  InstructionCost getReplicationShuffleCost(Type *EltTy, int ReplicationFactor,
                                            int VF,
                                            const APInt &DemandedDstElts,
                                            TTI::TargetCostKind CostKind);
  InstructionCost getMemoryOpCost(unsigned Opcode, Type *Src,
                                  MaybeAlign Alignment, unsigned AddressSpace,
                                  TTI::TargetCostKind CostKind,
                                  const Instruction *I = nullptr);
  InstructionCost getMaskedMemoryOpCost(unsigned Opcode, Type *Src,
                                        Align Alignment, unsigned AddressSpace,
                                        TTI::TargetCostKind CostKind);
  InstructionCost getGatherScatterOpCost(unsigned Opcode, Type *DataTy,
                                         const Value *Ptr, bool VariableMask,
                                         Align Alignment,
                                         TTI::TargetCostKind CostKind,
                                         const Instruction *I,
                                         bool UndefPassThru); // INTEL
#if INTEL_CUSTOMIZATION
  InstructionCost getGatherScatterOpCost(unsigned Opcode, Type *DataTy,
                                         unsigned IndexSize, bool VariableMask,
                                         unsigned Alignment,
                                         unsigned AddressSpace,
                                         TTI::TargetCostKind CostKind,
                                         const Instruction *I);
#endif // INTEL_CUSTOMIZATION
  InstructionCost getAddressComputationCost(Type *PtrTy, ScalarEvolution *SE,
                                            const SCEV *Ptr);

  Optional<Instruction *> instCombineIntrinsic(InstCombiner &IC,
                                               IntrinsicInst &II) const;
  Optional<Value *>
  simplifyDemandedUseBitsIntrinsic(InstCombiner &IC, IntrinsicInst &II,
                                   APInt DemandedMask, KnownBits &Known,
                                   bool &KnownBitsComputed) const;
  Optional<Value *> simplifyDemandedVectorEltsIntrinsic(
      InstCombiner &IC, IntrinsicInst &II, APInt DemandedElts, APInt &UndefElts,
      APInt &UndefElts2, APInt &UndefElts3,
      std::function<void(Instruction *, unsigned, APInt, APInt &)>
          SimplifyAndSetOp) const;

  unsigned getAtomicMemIntrinsicMaxElementSize() const;

  InstructionCost
  getTypeBasedIntrinsicInstrCost(const IntrinsicCostAttributes &ICA,
                                 TTI::TargetCostKind CostKind);
  InstructionCost getIntrinsicInstrCost(const IntrinsicCostAttributes &ICA,
                                        TTI::TargetCostKind CostKind);

  InstructionCost getArithmeticReductionCost(unsigned Opcode, VectorType *Ty,
                                             Optional<FastMathFlags> FMF,
                                             TTI::TargetCostKind CostKind);

  InstructionCost getMinMaxCost(Type *Ty, Type *CondTy, bool IsUnsigned);

  InstructionCost getMinMaxReductionCost(VectorType *Ty, VectorType *CondTy,
                                         bool IsUnsigned,
                                         TTI::TargetCostKind CostKind);

  InstructionCost getInterleavedMemoryOpCost(
      unsigned Opcode, Type *VecTy, unsigned Factor, ArrayRef<unsigned> Indices,
      Align Alignment, unsigned AddressSpace, TTI::TargetCostKind CostKind,
      bool UseMaskForCond = false, bool UseMaskForGaps = false);
  InstructionCost getInterleavedMemoryOpCostAVX512(
      unsigned Opcode, FixedVectorType *VecTy, unsigned Factor,
      ArrayRef<unsigned> Indices, Align Alignment, unsigned AddressSpace,
      TTI::TargetCostKind CostKind, bool UseMaskForCond = false,
      bool UseMaskForGaps = false);

  InstructionCost getIntImmCost(int64_t);

  InstructionCost getIntImmCost(const APInt &Imm, Type *Ty,
                                TTI::TargetCostKind CostKind);

  InstructionCost getCFInstrCost(unsigned Opcode, TTI::TargetCostKind CostKind,
                                 const Instruction *I = nullptr);

  InstructionCost getIntImmCostInst(unsigned Opcode, unsigned Idx,
                                    const APInt &Imm, Type *Ty,
                                    TTI::TargetCostKind CostKind,
                                    Instruction *Inst = nullptr);
  InstructionCost getIntImmCostIntrin(Intrinsic::ID IID, unsigned Idx,
                                      const APInt &Imm, Type *Ty,
                                      TTI::TargetCostKind CostKind);
  /// Return the cost of the scaling factor used in the addressing
  /// mode represented by AM for this target, for a load/store
  /// of the specified type.
  /// If the AM is supported, the return value must be >= 0.
  /// If the AM is not supported, it returns a negative value.
  InstructionCost getScalingFactorCost(Type *Ty, GlobalValue *BaseGV,
                                       int64_t BaseOffset, bool HasBaseReg,
                                       int64_t Scale, unsigned AddrSpace) const;

#if INTEL_CUSTOMIZATION
  InstructionCost getSerializationCost(Type *EltTy, unsigned NumElts,
                                       InstructionCost BuildVecCost) const;
#endif // INTEL_CUSTOMIZATION

  bool isLSRCostLess(const TargetTransformInfo::LSRCost &C1,
                     const TargetTransformInfo::LSRCost &C2);
  bool canMacroFuseCmp();
  bool isLegalMaskedLoad(Type *DataType, Align Alignment);
  bool isLegalMaskedStore(Type *DataType, Align Alignment);
  bool isLegalNTLoad(Type *DataType, Align Alignment);
  bool isLegalNTStore(Type *DataType, Align Alignment);
  bool isLegalBroadcastLoad(Type *ElementTy, ElementCount NumElements) const;
  bool forceScalarizeMaskedGather(VectorType *VTy, Align Alignment);
  bool forceScalarizeMaskedScatter(VectorType *VTy, Align Alignment) {
    return forceScalarizeMaskedGather(VTy, Alignment);
  }
  bool isLegalMaskedGather(Type *DataType, Align Alignment);
#if INTEL_CUSTOMIZATION
  bool shouldScalarizeMaskedGather(CallInst *CI);
  bool shouldOptGatherToLoadPermute(Type *ArrayElemTy, uint64_t ArrayNum,
                                    uint32_t GatherNum,
                                    uint32_t *WidenNum) const;
  bool isLegalToTransformGather2PermuteLoad(const IntrinsicInst *II,
                                            Type *&ArrayElemTy,
                                            uint64_t &ArrayNum,
                                            unsigned &GatherNum,
                                            unsigned &WidenNum) const;
  bool isLegalToTransformGather2PermuteLoad(
      Intrinsic::ID ID, Type *DataTy, const Value *Ptr, bool VariableMask,
      bool UndefPassThru, Type *&ArrayElemTy, uint64_t &ArrayNum,
      unsigned &GatherNum, unsigned &WidenNum) const;
#endif // INTEL_CUSTOMIZATION
  bool isLegalMaskedScatter(Type *DataType, Align Alignment);
  bool isLegalMaskedExpandLoad(Type *DataType);
  bool isLegalMaskedCompressStore(Type *DataType);
#if INTEL_CUSTOMIZATION
  bool isAdvancedOptEnabled(TTI::AdvancedOptLevel AO) const;
  bool isLibIRCAllowed() const;
  unsigned getMaxScale() const;
  bool adjustCallArgs(CallInst* CI);
  bool isTargetSpecificShuffleMask(ArrayRef<uint32_t> Mask) const;
  bool isVPlanVLSProfitable() const;
  bool isAggressiveVLSProfitable() const;
  int getMatchingVectorVariant(
      const VFInfo &ForCall,
      const SmallVectorImpl<VFInfo> &Variants,
      const Module *M) const;
  const char *getISASetForIMLFunctions() const;
  bool hasCDI() const;
  bool hasVLX() const;
  bool has2KDSB() const;
  bool has4KDSB() const;
  bool displacementFoldable() const;
#endif // INTEL_CUSTOMIZATION
  bool isLegalAltInstr(VectorType *VecTy, unsigned Opcode0, unsigned Opcode1,
                       const SmallBitVector &OpcodeMask) const;
  bool hasDivRemOp(Type *DataType, bool IsSigned);
  bool isFCmpOrdCheaperThanFCmpZero(Type *Ty);
  bool areInlineCompatible(const Function *Caller,
                           const Function *Callee) const;
  bool areTypesABICompatible(const Function *Caller, const Function *Callee,
                             const ArrayRef<Type *> &Type) const;
  TTI::MemCmpExpansionOptions enableMemCmpExpansion(bool OptSize,
                                                    bool IsZeroCmp) const;
  bool prefersVectorizedAddressing() const;
  bool supportsEfficientVectorElementLoadStore() const;
  bool enableInterleavedAccessVectorization();

private:
  bool supportsGather() const;
#if INTEL_CUSTOMIZATION
  InstructionCost getGSScalarCost(unsigned Opcode, Type *PtrTy, Type *DataTy,
                                  bool VariableMask, Align Alignment,
                                  unsigned AddressSpace);
  InstructionCost getGSVectorCost(unsigned Opcode, Type *DataTy,
                                  unsigned IndexSize, Align Alignment,
                                  unsigned AddressSpace);
  bool targetMatchesVariantISA(VFISAKind VariantISA) const;
#endif // INTEL_CUSTOMIZATION
  InstructionCost getGSVectorCost(unsigned Opcode, Type *DataTy,
                                  const Value *Ptr, Align Alignment,
                                  unsigned AddressSpace);

  int getGatherOverhead() const;
  int getScatterOverhead() const;

  /// @}
};

} // end namespace llvm

#endif
