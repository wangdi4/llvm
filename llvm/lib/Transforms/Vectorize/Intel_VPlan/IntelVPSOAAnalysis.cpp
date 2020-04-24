//===- IntelVPSOAAnalysis.cpp ------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements the VPSOAAnalysis class.
//===---------------------------------------------------------------------===//

#include "IntelVPSOAAnalysis.h"
#include "IntelVPlanUtils.h"
#include "llvm/Support/CommandLine.h"

#define DEBUG_TYPE "vpsoa-analysis"

using namespace llvm;
using namespace llvm::vpo;

// Flag to enable SOA-analysis.
static cl::opt<bool, true>
    EnableSOAAnalysisOpt("vplan-enable-soa", cl::Hidden,
                         cl::location(EnableSOAAnalysis),
                         cl::desc("Enable VPlan SOAAnalysis."));

// Flag to enable printing of SOA-analysis information.
static cl::opt<bool, true> VPlanDisplaySOAAnalysisInformationOpt(
    "vplan-dump-soa-info", cl::Hidden,
    cl::location(VPlanDisplaySOAAnalysisInformation),
    cl::desc("Display information about SOA Analysis on loop-entities."));

namespace llvm {
namespace vpo {
bool EnableSOAAnalysis = false;
bool VPlanDisplaySOAAnalysisInformation = false;

static bool checkInstructionInLoop(const VPValue *V, const VPLoop &Loop) {
  // Check for nullptr and VPInstruction here to avoid these checks on the
  // caller-side.
  return V == nullptr || !isa<VPInstruction>(V) ||
         Loop.contains(cast<VPInstruction>(V)->getParent());
}

void VPSOAAnalysis::doSOAAnalysis() {
  if (!EnableSOAAnalysis)
    return;
  assert(Plan.getVPlanDA() && "Expect DA to be run and the DA object be set "
                              "before running of SOAAnalysis.");

  VPBasicBlock *Preheader = Loop.getLoopPreheader();
  // Iterate through all the instructions in the loop-preheader, and for
  // VPAllocatePrivate instruction check if that instruction itself or any of
  // its possible uses, escapes.
  for (VPInstruction &VInst : *Preheader) {
    if (VPAllocatePrivate *AllocaPriv = dyn_cast<VPAllocatePrivate>(&VInst))
      if (!memoryEscapes(AllocaPriv))
        AllocaPriv->setSOASafe();
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  dump(dbgs());
#endif
}

// Returns true if the pointee type of the alloca-inst is a scalar value.
bool VPSOAAnalysis::isScalarTy(Type *Ty) {
  assert(Ty && "Expect a non-null argument to isScalarTy function.");
  return (!(Ty->isAggregateType() || Ty->isVectorTy()));
}

// Returns true if the pointee type of the alloca-inst is a scalar value.
bool VPSOAAnalysis::isSOASupportedTy(Type *Ty) {

  Type *PointeeTy = Ty->getPointerElementType();

  // If it is an array-type, check the element-type and return true only for
  // scalar-type.
  return (isa<ArrayType>(PointeeTy) &&
          (isScalarTy(cast<ArrayType>(PointeeTy)->getElementType())));
}

// Return true if \p UseInst is a safe bitcast instruction, i.e. it's a
// pointer-to-pointer cast doesn't change the size of the pointed elements.
bool VPSOAAnalysis::isPotentiallyUnsafeSafeBitCast(
    const VPInstruction *UseInst) {
  if (!UseInst || (UseInst->getOpcode() != Instruction::BitCast))
    return false;

  // We expect to have only pointer-type operands.
  PointerType *SrcPtrTy =
      dyn_cast<PointerType>(UseInst->getOperand(0)->getType());
  if (!SrcPtrTy)
    return true;

  PointerType *DstPtrTy = cast<PointerType>(UseInst->getType());
  return Plan.getDataLayout()->getTypeSizeInBits(
             SrcPtrTy->getPointerElementType()) !=
         Plan.getDataLayout()->getTypeSizeInBits(
             DstPtrTy->getPointerElementType());
}

// Returns true if UseInst is any function call, that we know is safe to pass a
// private-pointer to and does not change the data-layout.
bool VPSOAAnalysis::isSafePointerEscapeFunction(const VPInstruction *UseInst) {

  // If this is not a call-instruction, return false.
  if (UseInst->getOpcode() != Instruction::Call)
    return false;

  Function *CalleeFunc = getCalledFunction(UseInst);
  // For indirect calls we will have a null CalledFunc.
  if (!CalleeFunc)
    return false;
  if (CalleeFunc->isIntrinsic())
    return (CalleeFunc->getIntrinsicID() == Intrinsic::lifetime_start ||
            CalleeFunc->getIntrinsicID() == Intrinsic::lifetime_end ||
            CalleeFunc->getIntrinsicID() == Intrinsic::invariant_start ||
            CalleeFunc->getIntrinsicID() == Intrinsic::invariant_end);
  return false;
}

// Returns true if the instruction has operands registered as potentially-unsafe
// during analysis.
bool VPSOAAnalysis::hasPotentiallyUnsafeOperands(const VPInstruction *UseInst) {
  return any_of(UseInst->operands(), [=](const VPValue *VPOper) {
    return PotentiallyUnsafeInsts.count(VPOper);
  });
}

// Returns true if any of the following instruction with specific constraints
// are encountered.
bool VPSOAAnalysis::isSafeLoadStore(const VPInstruction *UseInst,
                                    const VPInstruction *CurrentI) {
  unsigned OpCode = UseInst->getOpcode();
  switch (OpCode) {
  // We consider loads unsafe when the pointer operand is considered
  // potentially unsafe (e.g., via unsafe bitcast).
  case Instruction::Load:
    return PotentiallyUnsafeInsts.count(UseInst->getOperand(0)) == 0;
  // We consider store's unsafe when the pointer operand is considered
  // potentially unsafe (e.g., via unsafe bitcast) or the private-pointer
  // or its alias escapes via a write to external memory.
  case Instruction::Store:
    return PotentiallyUnsafeInsts.count(UseInst->getOperand(1)) == 0 &&
           UseInst->getOperand(0) != CurrentI;
  default:
    return false;
  }
}

// An umbrella function to determine the safety of an operation.
bool VPSOAAnalysis::isSafeUse(const VPInstruction *UseInst,
                              const VPInstruction *CurrentI) {
  return isTrivialPointerAliasingInst<VPInstruction>(UseInst) ||
         isSafePointerEscapeFunction(UseInst) ||
         isSafeLoadStore(UseInst, CurrentI);
}

// Return true if the instruction is either in the loop-preheader or the
// loop-body.
bool VPSOAAnalysis::isInstructionInRelevantScope(const VPInstruction *UseInst) {
  if (!UseInst)
    return false;
  VPBasicBlock *Preheader = Loop.getLoopPreheader();
  return ((UseInst->getParent() == Preheader) ||
          (checkInstructionInLoop(UseInst, Loop)));
}

// Function which determines if the given loop-entity escapes.
bool VPSOAAnalysis::memoryEscapes(const VPAllocatePrivate *Alloca) {

  // Clear the 'PotentiallyUnsafeInsts' dense-set.
  PotentiallyUnsafeInsts.clear();

  // Clear the WorkList and AnalyzedInsts of contents of the earlier run.
  WL.clear();
  AnalyzedInsts.clear();

  // If this is a scalar-private, just return. The real memory layout for simple
  // scalars is identical for both SOA and AOS, it's just vector of elements.
  assert(Alloca->getType()->isPointerTy() &&
         "Expect the 'alloca' to have a pointer-type.");
  if (isScalarTy(Alloca->getType()->getPointerElementType()))
    return false;

  // Non-array aggregate types are currently not supported. Conservatively, just
  // return 'true', i.e., the memory escapes.
  if (!isSOASupportedTy(Alloca->getType()))
    return true;

  // Initialize the WorkList with the memory-pointer.
  WL.insert(Alloca);

  // Get all the Uses of these instructions. Consider the use as safe under
  // the following conditions,
  //
  // 1) If it is load/store, where the pointer does not come via an unsafe
  //  bitcast.
  //
  // 2) If it is a store and the value-operand, a pointer we are checking, is
  // not written to external memory (e.g., output argument of a function).
  //
  // 3) Call to a function that is known to not read/write memory passed by
  // pointer and not storing the pointer to memory.

  // Every other instruction is considered unsafe or potentially-unsafe.

  // Instructions that create pointer aliases, checked via
  // isTrivialPointerAliasingInst(), are added to the worklist for further
  // analysis.

  while (!WL.empty()) {
    const VPInstruction *CurrentI = WL.pop_back_val();

    // If the UseInst has already been analyzed, skip.
    if (!AnalyzedInsts.insert(CurrentI).second)
      continue;

    // Analyze the users of the current-instruction.
    for (VPValue *User : CurrentI->users()) {
      const VPInstruction *UseInst = dyn_cast<VPInstruction>(User);

      // We are only interested in pointer or its alias which is either in the
      // Loop-preheader of within the loop itself.
      if (!isInstructionInRelevantScope(UseInst))
        continue;

      if (!isSafeUse(UseInst, CurrentI))
        return true;

      if (isTrivialPointerAliasingInst<VPInstruction>(UseInst))
        // If this is one of the aliasing instructions add it to the
        // worklist.
        WL.insert(UseInst);

      // Determine if the instruction is potentially unsafe. It could be
      // potentially unsafe when,
      //
      // 1) It results in a narrowed or widened pointer.
      //    e.g., %bc = bitcast [624 x i32]* priv.ptr to i8*
      // 2) One of the operands has previously been determined as unsafe.
      //    e.g., Could use the previous bitcast either directly or
      //    indirectly,
      //          %l2 = load i8, i8* %bc
      //                    or
      //          %gep = ....
      //               ...
      //               ...
      //          %bc2 = bitcast ...
      //          %l2 = load i8, i8* %bc2
      if (isPotentiallyUnsafeSafeBitCast(UseInst) ||
          hasPotentiallyUnsafeOperands(UseInst))
        PotentiallyUnsafeInsts.insert(UseInst);
    }
  }
  // All encountered uses to the pointer or its aliases are safe instructions.
  // We can say that it is SOASafe and return false.
  return false;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void VPSOAAnalysis::dump(raw_ostream &OS) const {
  if (!VPlanDisplaySOAAnalysisInformation)
    return;

  VPBasicBlock *Preheader = Loop.getLoopPreheader();
  // TODO: Dump profitability information along with safety information.
  for (VPInstruction &VPInst : *Preheader) {
    if (VPAllocatePrivate *VPAllocaPriv =
            dyn_cast<VPAllocatePrivate>(&VPInst)) {

      if (VPAllocaPriv->isSOASafe())
        OS << "SOASafe = " << VPAllocaPriv->getOrigName() << "\n";
      else
        OS << "SOAUnsafe = " << VPAllocaPriv->getOrigName() << "\n";
    }
  }
}
#endif // NDEBUG
} // namespace vpo.
} // namespace llvm.
