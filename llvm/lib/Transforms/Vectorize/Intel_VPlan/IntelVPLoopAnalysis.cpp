//===- IntelVPLoopAnalysis.cpp --------------------------------------------===//
//
//   Copyright (C) 2019 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
// \file
/// This file provides VPLoop-based analysis. Right now VPLoopAnalysisBase can
/// only be used to compute min, known, estimated or max trip counts for a
/// VPLoopRegion. In addition Min,max and average tripcounts are set based
/// on the values specified by the user in pragma loop_count
//
//===----------------------------------------------------------------------===//

#include "IntelVPLoopAnalysis.h"
#include "IntelVPlan.h"
#include "llvm/ADT/GraphTraits.h"

#define DEBUG_TYPE "vploop-analysis"

using namespace llvm;
using namespace llvm::vpo;

unsigned int VPInduction::getInductionOpcode() const {
  return getInductionBinOp() ? getInductionBinOp()->getOpcode() : BinOpcode;
}

void VPLoopEntities::addReduction(VPInstruction *Instr, VPValue *Incoming,
                                  VPInstruction *Exit, RecurrenceKind Kind, FastMathFlags FMF,
                                  MinMaxRecurrenceKind MKind, Type *RedTy,
                                  bool Signed, VPValue *AI) {

  assert(Instr && "null instruction");
  VPReduction *Red =
      new VPReduction(Incoming, Exit, Kind, FMF, MKind, RedTy, Signed);
  ReductionMap[Instr] = std::unique_ptr<VPReduction>(Red);
  createMemDescFor(Red, AI);
}
void VPLoopEntities::addIndexReduction(VPInstruction *Instr,
                                       VPReduction *Parent, VPValue *Incoming,
                                       VPInstruction *Exit, Type *RedTy,
                                       bool Signed, bool ForLast, VPValue *AI) {
  assert(Instr && "null instruction");
  assert(Parent && "null parent in index-reduction");
  VPIndexReduction *Red =
      new VPIndexReduction(Parent, Incoming, Exit, RedTy, Signed, ForLast);
  ReductionMap[Instr] = std::unique_ptr<VPIndexReduction>(Red);
  MinMaxIndexes[Parent] = Red;
  createMemDescFor(Red, AI);
}
void VPLoopEntities::addInduction(VPInstruction *Start, VPValue *Incoming,
                                  InductionKind Kind, VPValue *Step,
                                  VPInstruction *InductionBinOp,
                                  unsigned int Opc, VPValue *AI) {
  assert(Start && "null starting instruction");
  VPInduction *Ind = new VPInduction(Incoming, Kind, Step, InductionBinOp, Opc);
  InductionMap[Start] = std::unique_ptr<VPInduction>(Ind);
  createMemDescFor(Ind, AI);
}
void VPLoopEntities::addPrivate(VPInstruction *Assign, bool isConditional,
                                bool Explicit, VPValue *AI) {
  assert(Assign && "null assign");
  VPPrivate *Priv = new VPPrivate(Assign, isConditional, Explicit);
  PrivateMap[Assign] = std::unique_ptr<VPPrivate>(Priv);
  createMemDescFor(Priv, AI);
}

void VPLoopEntities::createMemDescFor(VPLoopEntity *E, VPValue *AI) {
  if (AI) {
    VPInMemoryEntity *Mem = new VPInMemoryEntity(AI);
    MemoryDescriptors[E] = std::unique_ptr<VPInMemoryEntity>(Mem);
  }
}

/// Returns identity corresponding to the RecurrenceKind.
VPValue *VPLoopEntities::getReductionIdentiy(const VPReduction *Red) const {
  switch (Red->getRecurrenceKind()) {
  case RecurrenceKind::RK_IntegerXor:
  case RecurrenceKind::RK_IntegerAdd:
  case RecurrenceKind::RK_IntegerOr:
  case RecurrenceKind::RK_IntegerMult:
  case RecurrenceKind::RK_IntegerAnd:
  case RecurrenceKind::RK_FloatMult:
  case RecurrenceKind::RK_FloatAdd: {
    Constant *C = VPReduction::getRecurrenceIdentity(Red->getRecurrenceKind(),
                                                     Red->getRecurrenceType());
    return Plan->getVPConstant(C);
  }
  case RecurrenceKind::RK_IntegerMinMax:
  case RecurrenceKind::RK_FloatMinMax:
    return Red->getRecurrenceStartValue();
  default:
    llvm_unreachable("Unknown recurrence kind");
  }
}

namespace llvm {
namespace vpo {

void VPLoopAnalysisBase::setTripCountsFromPragma(const VPLoopRegion *Lp,
                                                 uint64_t MinTripCount,
                                                 uint64_t MaxTripCount,
                                                 uint64_t AvgTripCount) {
  bool IsMaxTakenFromPragma = false;
  bool IsMinTakenFromPragma = false;
  bool IsAverageTakenFromPragma = false;

  if (MaxTripCount) {
    setMaxTripCountFor(Lp, MaxTripCount);
    IsMaxTakenFromPragma = true;
  } else
    setMaxTripCountFor(Lp, DefaultTripCount);

  if (MinTripCount) {
    setMinTripCountFor(Lp, MinTripCount);
    IsMinTakenFromPragma = true;
  } else
    setMinTripCountFor(Lp, 0);

  if (AvgTripCount) {
    setEstimatedTripCountFor(Lp, AvgTripCount);
    IsAverageTakenFromPragma = true;
  } else if (IsMaxTakenFromPragma && IsMinTakenFromPragma)
    setEstimatedTripCountFor(Lp, (MaxTripCount + MinTripCount) >> 1);
  else if (IsMaxTakenFromPragma)
    setEstimatedTripCountFor(Lp, MaxTripCount);
  else if (IsMinTakenFromPragma)
    setEstimatedTripCountFor(Lp, MinTripCount);
  else
    setEstimatedTripCountFor(Lp, DefaultTripCount);

  (void)IsMaxTakenFromPragma;
  (void)IsMinTakenFromPragma;
  (void)IsAverageTakenFromPragma;

  LLVM_DEBUG(
      dbgs()
      << "Max trip count is " << getMaxTripCountFor(Lp)
      << (IsMaxTakenFromPragma
              ? " updated by loop opt upon retrieving loop count from pragma"
              : " assumed default trip count by vectorizer")
      << '\n');
  LLVM_DEBUG(dbgs() << "Average trip count is " << getTripCountFor(Lp)
                    << (IsAverageTakenFromPragma
                            ? " set by pragma loop count"
                            : " assumed default trip count by vectorizer")
                    << '\n');
  LLVM_DEBUG(dbgs() << "Min trip count is " << getMinTripCountFor(Lp)
                    << (IsMinTakenFromPragma
                            ? " set by pragma loop count"
                            : " assumed default trip count by vectorizer")
                    << '\n');
}

// Metadata is attached to the loop latch. Loop through the VPBasicBlocks to
// find the underlying original basic block and get the LoopID.
// Set the max, min and average trip counts from the metadata.
// Fix Me: This fails when there is an outerloop and there are different
// pragma values for inner and outer loop.
void VPLoopAnalysis::computeTripCountImpl(const VPLoopRegion *Lp) {

  StringRef MaxInfo = "llvm.loop.intel.loopcount_maximum";
  StringRef MinInfo = "llvm.loop.intel.loopcount_minimum";
  StringRef AvgInfo = "llvm.loop.intel.loopcount_average";

  uint64_t MinTripCount = 0, MaxTripCount = 0, AvgTripCount = 0;

  LoopTripCounts[Lp] = TripCountInfo();

  for (const VPBlockBase *VPB :
       make_range(df_iterator<const VPBlockBase *>::begin(Lp->getEntry()),
                  df_iterator<const VPBlockBase *>::end(Lp->getExit()))) {
    const VPBasicBlock *CurrentVPBB = cast<const VPBasicBlock>(VPB);

    if (BasicBlock *OriginalBB = CurrentVPBB->getOriginalBB()) {
      const Loop *Loop = LI->getLoopFor(OriginalBB);
      if (Loop) {
        MDNode *LoopID = Loop->getLoopID();
        if (LoopID) {
          for (unsigned i = 1, ie = LoopID->getNumOperands(); i < ie; ++i) {
            MDNode *MD = dyn_cast<MDNode>(LoopID->getOperand(i));
            if (MD) {
              const MDString *S = dyn_cast<MDString>(MD->getOperand(0));
              if (MaxInfo.equals(S->getString()))
                MaxTripCount = mdconst::extract<ConstantInt>(MD->getOperand(1))
                                   ->getZExtValue();
              if (MinInfo.equals(S->getString()))
                MinTripCount = mdconst::extract<ConstantInt>(MD->getOperand(1))
                                   ->getZExtValue();
              if (AvgInfo.equals(S->getString()))
                AvgTripCount = mdconst::extract<ConstantInt>(MD->getOperand(1))
                                   ->getZExtValue();
            }
          }
        }
      }
      break;
    }
  }
  setTripCountsFromPragma(Lp, MinTripCount, MaxTripCount, AvgTripCount);
}
} // namespace vpo
} // namespace llvm
