//===-- CSAISelLowering.h - CSA DAG Lowering Interface ----------*- C++ -*-===//
//
// Copyright (C) 2017-2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that CSA uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CSA_CSAISELLOWERING_H
#define LLVM_LIB_TARGET_CSA_CSAISELLOWERING_H

#include "CSA.h"
#include "CSAMachineFunctionInfo.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/TargetLowering.h"

namespace llvm {
namespace CSAISD {
enum {
  FIRST_NUMBER = ISD::BUILTIN_OP_END,
  Call,
  TailCall,
  Ret,

  // These custom ISel nodes represent min/max operations with a second i1 "sel"
  // output indicating which parameter was the minimum/maximum value following
  // the conventions for the CSA instructions. Min/Max are used for signed
  // integer and floating point types while UMin/UMax are for unsigned integer.
  Min,
  UMin,
  Max,
  UMax,

  // Wrapper - A wrapper node for TargetConstantPool, TargetExternalSymbol,
  // and TargetGlobalAddress
  Wrapper
};
}

class CSASubtarget;

class CSATargetLowering : public TargetLowering {
public:
  explicit CSATargetLowering(const TargetMachine &TM, const CSASubtarget &ST);

  EVT getSetCCResultType(const DataLayout &DL, LLVMContext &Context,
                         EVT VT) const override;

  // Returns the type that the value to shift-by should have
  // given the EVT of that operand. Since the CSA supports
  // all types save i1, that is what we return.
  // (Copied from FFWD)
  MVT getScalarShiftAmountTy(const DataLayout &, EVT LHSTy) const override {
    assert(LHSTy.isInteger() && "Can only shift integer types");
    unsigned bits = LHSTy.getScalarSizeInBits();
    if (bits < 8) bits = 8;
    return MVT::getIntegerVT(1 << Log2_32_Ceil(bits));
  }

  /// LowerOperation - Provide custom lowering hooks for some operations.
  SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const override;

  /// getTargetNodeName - This method returns the name of a target specific
  /// DAG node.
  const char *getTargetNodeName(unsigned Opcode) const override;

  SDValue LowerGlobalAddress(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerExternalSymbol(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerBlockAddress(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerJumpTable(SDValue Op, SelectionDAG &DAG) const;
  /*
  SDValue LowerRETURNADDR(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerFRAMEADDR(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerVASTART(SDValue Op, SelectionDAG &DAG) const;
  SDValue getReturnAddressFrameIndex(SelectionDAG &DAG) const;
  */

  ConstraintType getConstraintType(StringRef Constraint) const override;
  std::pair<unsigned, const TargetRegisterClass *>
  getRegForInlineAsmConstraint(const TargetRegisterInfo *TRI,
                               StringRef Constraint, MVT VT) const override;
  SDValue LowerAtomicLoad(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerAtomicStore(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerMUL_LOHI(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerADDSUB_Carry(SDValue Op, SelectionDAG &DAG, bool isAdd) const;
  SDValue LowerExtractElement(SDValue Op, SelectionDAG &DAG) const;

  bool isTruncateFree(EVT VT1, EVT VT2) const override;
  bool isTruncateFree(Type *Ty1, Type *Ty2) const override;
  bool isLegalAddressingMode(const DataLayout &DL, const AddrMode &AM, Type *Ty,
                              unsigned AddrSpace = 0, Instruction *i = nullptr) const override;
  /// isZExtFree - Return true if any actual instruction that defines a value
  /// of type Ty1 implicit zero-extends the value to Ty2 in the result
  /// register. This does not necessarily include registers defined in unknown
  /// ways, such as incoming arguments, or copies from unknown virtual
  /// registers. Also, if isTruncateFree(Ty2, Ty1) is true, this does not
  /// necessarily apply to truncate instructions. e.g. on msp430, all
  /// instructions that define 8-bit values implicit zero-extend the result
  /// out to 16 bits.
  bool isZExtFree(Type *Ty1, Type *Ty2) const override;
  bool isZExtFree(EVT VT1, EVT VT2) const override;
  bool isZExtFree(SDValue Val, EVT VT2) const override;

  /// Return true if it's profitable to narrow
  /// operations of type VT1 to VT2.
  bool isNarrowingProfitable(EVT VT1, EVT VT2) const override;

  /// Return true if an FMA operation is faster than a pair of fmul and fadd
  /// instructions.
  bool isFMAFasterThanFMulAndFAdd(EVT VT) const override;

  /// post-selection hooks, in this case a workaround for LLVM's mishandling of
  /// optional register defs
  void AdjustInstrPostInstrSelection(MachineInstr &, SDNode *) const override;

  TargetLowering::AtomicExpansionKind
  shouldExpandAtomicRMWInIR(AtomicRMWInst *AI) const override;

  SDValue PerformDAGCombine(SDNode *, DAGCombinerInfo &) const override;

private:
  /// Keep a reference to the CSASubtarget around so that we can
  /// make the right decision when generating code for different targets.
  const CSASubtarget &Subtarget;

  /*
  SDValue LowerCCCCallTo(SDValue Chain, SDValue Callee,
                         CallingConv::ID CallConv, bool isVarArg,
                         bool isTailCall,
                         const SmallVectorImpl<ISD::OutputArg> &Outs,
                         const SmallVectorImpl<SDValue> &OutVals,
                         const SmallVectorImpl<ISD::InputArg> &Ins,
                         const SDLoc &dl, SelectionDAG &DAG,
                         SmallVectorImpl<SDValue> &InVals) const;

  SDValue LowerCCCArguments(SDValue Chain,
                            CallingConv::ID CallConv,
                            bool isVarArg,
                            const SmallVectorImpl<ISD::InputArg> &Ins,
                            const SDLoc &dl,
                            SelectionDAG &DAG,
                            SmallVectorImpl<SDValue> &InVals) const;
  */
  /// IsEligibleForTailCallOptimization - Check whether the call is eligible
  /// for tail call optimization.
  bool
  IsEligibleForTailCallOptimization(unsigned NextStackOffset,
                                    const CSAMachineFunctionInfo &FI) const;

  SDValue LowerCall(TargetLowering::CallLoweringInfo &CLI,
                    SmallVectorImpl<SDValue> &InVals) const override;

  SDValue LowerCallResult(SDValue Chain, SDValue InFlag,
                          CallingConv::ID CallConv, bool isVarArg,
                          const SmallVectorImpl<ISD::InputArg> &Ins,
                          const SDLoc &dl, SelectionDAG &DAG,
                          SmallVectorImpl<SDValue> &InVals) const;

  SDValue LowerFormalArguments(SDValue Chain, CallingConv::ID CallConv,
                               bool isVarArg,
                               const SmallVectorImpl<ISD::InputArg> &Ins,
                               const SDLoc &dl, SelectionDAG &DAG,
                               SmallVectorImpl<SDValue> &InVals) const override;
  SDValue LowerReturn(SDValue Chain, CallingConv::ID CallConv, bool isVarArg,
                      const SmallVectorImpl<ISD::OutputArg> &Outs,
                      const SmallVectorImpl<SDValue> &OutVals, const SDLoc &dl,
                      SelectionDAG &DAG) const override;

  SDValue CombineSelect(SDNode *, SelectionDAG &) const;
  SDValue CombineMinMax(SDNode *, SelectionDAG &) const;

  /// Searches the SelectionDAG for compares with the same inputs as a
  /// recently-created min/max op and attempts to replace them with a value
  /// derived from that op's sel output.
  ///
  /// \param Sel The sel output from the new min/max op.
  /// \param IgnoreCmp If this is not SDValue{}, ignore it when searching for
  /// comparisons.
  void FoldComparesIntoMinMax(SDValue Sel, SelectionDAG &,
                              SDValue IgnoreCmp = {}) const;
};
} // namespace llvm

#endif
