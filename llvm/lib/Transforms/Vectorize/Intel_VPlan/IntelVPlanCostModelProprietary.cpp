//===-- IntelVPlanCostModelProprietary.cpp --------------------------------===//
//
//   Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements VPlan cost modeling with Intel's IP.
//
//===----------------------------------------------------------------------===//

#include "IntelVPlanCostModelProprietary.h"
#include "IntelVPlan.h"
#include "IntelVPlanIdioms.h"
#include "IntelVPlanUtils.h"
#include "IntelVPlanVLSAnalysis.h"
#include "VPlanHIR/IntelVPlanVLSAnalysisHIR.h"
#include "VPlanHIR/IntelVPlanVLSClientHIR.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Analysis/VectorUtils.h"
#include "IntelVPlanPatternMatch.h"

#define DEBUG_TYPE "vplan-cost-model-proprietary"

static cl::opt<bool> UseOVLSCM(
  "vplan-cm-use-ovlscm", cl::init(true),
  cl::desc("Consider cost returned by OVLSCostModel "
           "for optimized gathers and scatters."));

using namespace llvm::loopopt;

namespace llvm {

namespace vpo {

unsigned VPlanCostModelProprietary::getArithmeticInstructionCost(
  const unsigned Opcode,
  const VPValue *Op1,
  const VPValue *Op2,
  const Type *ScalarTy,
  const unsigned VF) {
  unsigned BaseCMCost = VPlanCostModel::getArithmeticInstructionCost(
    Opcode, Op1, Op2, ScalarTy, VF);

  if (BaseCMCost == UnknownCost)
    return BaseCMCost;

  // Special case for integer DIV/REM operation.
  //
  // Vector integer DIV/REM implemented through serialized scalar code
  // for VF = 2, yelding slightly worse performance comparing to vanilla
  // scalar code due to serialization overhead.
  //
  // For VF > 2 SVML functions are invoked: 8 elements function for int32 and
  // 4 elements function for int64.
  //
  // For int32 SVML yelds ~2x better performance vs scalar version for its
  // natural VF = 8 and for all VF greater than the natural VF.
  //
  // VF = 4 int32 implemented as masked VF = 8 case yelding 2x worse perfomance
  // vs VF = 8.
  //
  // int64 VF = 1 implementation holds RT check for input data that can be
  // divided using 32-bit DIV instruction giving 3.5x better performance.  For
  // 64-bit input data scalar version is ~30% slower of vector version.  We
  // make an assumption that in real applications half data fits 32-bit
  // representation making scalar version of 64-bit DIV/REM (3.5 / 2) ~ 2x
  // faster VS SVML version.
  //
  // Factor in those impirical data into multiplier of the scalar cost
  // of DIV operation.  This is not modelled well by TTI.
  //
  // Don't mess with with 8-/16-bit input data type if it ever possible to get
  // them here.
  //
  // TODO:
  // OpenCL CPU RT uses an alternative version of SVML and the heuristic
  // doesn't cover it.  OCL context can be checked with:
  // M->getNamedMetadata("opencl.ocl.version") != nullptr, where M is Module.
  // Currently CM doesn't have access to Module, neither we always have
  // UnderyingValue valid for Op1.  Thereby as of now OCL context check is
  // missed which is not an issue until VPlan becomes a part of OCL pipeline.
  //
  // TODO:
  // The code below is a temporal code to WA this problem.  Eventually
  // we want to have CG interface which would tells us what instructions
  // [U|S]Div/Rem or any other VPIntruction is implemented with.
  if (VF > 1 &&
      (Opcode == Instruction::UDiv || Opcode == Instruction::SDiv ||
       Opcode == Instruction::URem || Opcode == Instruction::SRem) &&
      TLI->isSVMLEnabled()) {
    unsigned ElemSize = ScalarTy->getPrimitiveSizeInBits();
    if (ElemSize != 32 && ElemSize != 64)
      return BaseCMCost;

    unsigned ScalarCost = VPlanCostModel::getArithmeticInstructionCost(
      Opcode, Op1, Op2, ScalarTy, 1);

    if (ScalarCost == UnknownCost)
      return ScalarCost;

    unsigned VectorCost;

    if (ElemSize == 64)
      VectorCost = ScalarCost * VF * 2;
    else if (VF == 2)
      VectorCost = ScalarCost * VF;
    else if (VF == 4)
      VectorCost = ScalarCost * 3;
    else
      VectorCost = ScalarCost * (VF / 2);

    // For operations with constant in argument basic cost model gives better
    // estimation, which we want to use instead of VectorCost.
    return std::min(VectorCost, BaseCMCost);
  }

  return BaseCMCost;
}

unsigned VPlanCostModelProprietary::getLoadStoreCost(
  const VPInstruction *VPInst, Align Alignment,
  unsigned VF,
  const bool UseVLSCost) {
  unsigned Cost =
    VPlanCostModel::getLoadStoreCost(VPInst, Alignment, VF);

  if (!UseOVLSCM || !VLSA || !UseVLSCost || VF == 1)
    return Cost;

  OVLSGroup *Group = VLSA->getGroupsFor(Plan, VPInst);
  if (!Group || Group->size() <= 1)
    return Cost;

  // FIXME: Really ugly to get LLVMContext from VLSA, which may not
  // even exist, but so far there's no other simple way to pass it here.
  // Unlike LLVM IR VPBB has no LLVMContext, because Type is not yet
  // implemented.
  //
  // FIXME: OVLSCostModel abstructions need to be revisitted as
  // VPlanVLSCostModel::getInstructionCost() implementation requires external
  // VF to form Vector Type of OVLSInstruction, whereas
  // OVLSTTICostModel::getInstructionCost() implementation fetches number of
  // elements using I->getType().
  //
  VPlanVLSCostModel VLSCM(VF, VPTTI.getTTI(), VLSA->getContext());
  /// OptVLSInterface costs are not scaled up yet.
  unsigned VLSGroupCost =
    VPlanTTIWrapper::Multiplier * OptVLSInterface::getGroupCost(*Group, VLSCM);

  if (ProcessedOVLSGroups.count(Group) != 0) {
    if (!ProcessedOVLSGroups[Group]) {
      LLVM_DEBUG(dbgs() << "TTI cost of memrefs in the Group containing ";
                 VPInst->printWithoutAnalyses(dbgs());
                 dbgs() << " is less than its OVLS Group cost.\n");
      return Cost;
    }

    assert(is_contained(Group->getMemrefVec(), Group->getInsertPoint()) &&
           "OVLS Group's insertion point is not a member of the Group.");

    if (cast<VPVLSClientMemref>(Group->getInsertPoint())->getInstruction() ==
        VPInst) {
      LLVM_DEBUG(dbgs() << "Whole OVLS Group cost is assigned on ";
               VPInst->printWithoutAnalyses(dbgs());
               dbgs() << '\n');
      return VLSGroupCost;
    }

    LLVM_DEBUG(dbgs() << "Group cost for ";
               VPInst->printWithoutAnalyses(dbgs());
               dbgs() << " has already been taken into account.\n");

    return 0;
  }

  unsigned TTIGroupCost = 0;
  for (OVLSMemref *OvlsMemref : Group->getMemrefVec())
    TTIGroupCost += VPlanCostModel::getLoadStoreCost(
      cast<VPVLSClientMemref>(OvlsMemref)->getInstruction(), Alignment, VF);

  if (VLSGroupCost >= TTIGroupCost) {
    LLVM_DEBUG(dbgs() << "Cost for "; VPInst->printWithoutAnalyses(dbgs());
               dbgs() << " was not reduced from " << Cost << " (TTI group cost "
                      << TTIGroupCost << ") to group cost " << VLSGroupCost
                      << '\n');
    ProcessedOVLSGroups[Group] = false;
    return Cost;
  }

  LLVM_DEBUG(dbgs() << "Reduced cost for ";
             VPInst->printWithoutAnalyses(dbgs());
             dbgs() << " from " << Cost << " (TTI group cost " << TTIGroupCost
                    << " to group cost " << VLSGroupCost << ")\n");
  ProcessedOVLSGroups[Group] = true;
  return VLSGroupCost;
}

unsigned VPlanCostModelProprietary::getCost(const VPInstruction *VPInst) {
  unsigned Opcode = VPInst->getOpcode();
  switch (Opcode) {
  case Instruction::Load:
  case Instruction::Store:
    return getLoadStoreCost(VPInst, VF, true);
  // TODO: So far there's no explicit representation for reduction
  // initializations and finalizations. Need to account overhead for such
  // instructions, until VPlan is ready to have explicit representation for
  // that.
  default:
    return VPlanCostModel::getCost(VPInst);
  }
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
std::string VPlanCostModelProprietary::getAttrString(
  const VPInstruction *VPInst) const {
  std::string VPInstAttributes = VPlanCostModel::getAttrString(VPInst);

  if (OVLSGroup *Group = VLSA->getGroupsFor(Plan, VPInst))
    if (ProcessedOVLSGroups.count(Group) != 0)
      VPInstAttributes += " OVLS";

  return VPInstAttributes;
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP

} // namespace vpo

} // namespace llvm
