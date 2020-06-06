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
#include "IntelVPlanVLSAnalysis.h"
#include "VPlanHIR/IntelVPlanVLSAnalysisHIR.h"
#include "VPlanHIR/IntelVPlanVLSClientHIR.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Support/CommandLine.h"

#define DEBUG_TYPE "vplan-cost-model-proprietary"

static cl::opt<bool> UseOVLSCM("vplan-cm-use-ovlscm", cl::init(true),
                               cl::desc("Consider cost returned by OVLSCostModel "
                                        "for optimized gathers and scatters."));
static cl::opt<unsigned> BoolInstsBailOut(
    "vplan-cost-model-i1-bail-out-limit", cl::init(45), cl::Hidden,
    cl::desc("Don't vectorize if number of boolean computations in the VPlan "
             "is higher than the threshold."));

using namespace llvm::loopopt;

namespace llvm {

namespace vpo {

bool VPlanCostModelProprietary::isUnitStrideLoadStore(
  const VPInstruction *VPInst) const {
  unsigned Opcode = VPInst->getOpcode();
  assert((Opcode == Instruction::Load || Opcode == Instruction::Store) &&
         "Is not load or store instruction.");

  if (VPlanCostModel::isUnitStrideLoadStore(VPInst))
    return true;

  if (!VPInst->HIR.isMaster())
    return false;

  if (auto Inst = dyn_cast<HLInst>(VPInst->HIR.getUnderlyingNode())) {
    // FIXME: It's not correct to getParentLoop() for outerloop
    // vectorization.
    if (!Inst->getParentLoop()) {
      return false;
    }
    assert(Inst->getParentLoop()->isInnermost() &&
           "Outerloop vectorization is not supported.");
    unsigned NestingLevel = Inst->getParentLoop()->getNestingLevel();

    return Opcode == Instruction::Load
      ? VPlanVLSAnalysisHIR::isUnitStride(Inst->getOperandDDRef(1),
                                          NestingLevel)
      : VPlanVLSAnalysisHIR::isUnitStride(Inst->getLvalDDRef(),
                                          NestingLevel);
  }
  return false;
}

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
  const VPInstruction *VPInst, const bool UseVLSCost) {
  unsigned Cost = VPlanCostModel::getLoadStoreCost(VPInst);

  if (!UseOVLSCM || !VLSCM || !UseVLSCost || VF == 1)
    return Cost;

  OVLSGroup *Group = VLSA->getGroupsFor(Plan, VPInst);
  if (!Group || Group->size() <= 1)
    return Cost;

  if (ProcessedOVLSGroups.count(Group) != 0) {
    // Group cost has already been assigned.
    LLVM_DEBUG(dbgs() << "Group cost for "; VPInst->print(dbgs());
               dbgs() << " has already been taken into account.\n");
    return ProcessedOVLSGroups[Group] ? 0 : Cost;
  }

  unsigned VLSGroupCost = OptVLSInterface::getGroupCost(*Group, *VLSCM);
  unsigned TTIGroupCost = 0;
  for (OVLSMemref *OvlsMemref : Group->getMemrefVec())
    TTIGroupCost += VPlanCostModel::getLoadStoreCost(
      cast<VPVLSClientMemref>(OvlsMemref)->getInstruction());

  if (VLSGroupCost >= TTIGroupCost) {
    LLVM_DEBUG(dbgs() << "Cost for "; VPInst->print(dbgs());
               dbgs() << " was not reduced from " << Cost <<
               " (TTI group cost " << TTIGroupCost <<
               ") to group cost " << VLSGroupCost << '\n');
    ProcessedOVLSGroups[Group] = false;
    return Cost;
  }

  LLVM_DEBUG(dbgs() << "Reduced cost for "; VPInst->print(dbgs());
             dbgs() << " from " << Cost << " (TTI group cost " <<
             TTIGroupCost << " to group cost " << VLSGroupCost << '\n');
  ProcessedOVLSGroups[Group] = true;
  return VLSGroupCost;
}

unsigned VPlanCostModelProprietary::getCost(const VPInstruction *VPInst) {
  if (VPInst->getType()->isIntegerTy(1))
    ++NumberOfBoolComputations;

  unsigned Opcode = VPInst->getOpcode();
  switch (Opcode) {
  case Instruction::Load:
  case Instruction::Store:
    return getLoadStoreCost(VPInst, true);
  // TODO: So far there's no explicit representation for reduction
  // initializations and finalizations. Need to account overhead for such
  // instructions, until VPlan is ready to have explicit representation for
  // that.
  default:
    return VPlanCostModel::getCost(VPInst);
  }
}

// Right now it calls for VPlanCostModel::getCost(VPBB), but later we may want
// to have more precise cost estimation for VPBB.
unsigned VPlanCostModelProprietary::getCost(const VPBasicBlock *VPBB) {
  return VPlanCostModel::getCost(VPBB);
}

unsigned VPlanCostModelProprietary::getCost() {
  NumberOfBoolComputations = 0;
  ProcessedOVLSGroups.clear();
  unsigned Cost = VPlanCostModel::getCost();

  // Array ref which needs to be aligned via loop peeling, if any.
  RegDDRef *PeelArrayRef = nullptr;
  switch (VPlanIdioms::isSearchLoop(Plan, VF, true, PeelArrayRef)) {
  case VPlanIdioms::Unsafe:
    return UnknownCost;
  case VPlanIdioms::SearchLoopStrEq:
    // Without proper type information, cost model cannot properly compute the
    // cost, thus hard code VF.
    if (VF == 1)
      return 1000;
    if (VF != 32)
      // Return some huge value, so that VectorCost still could be computed.
      return UnknownCost;
    break;
  case VPlanIdioms::SearchLoopStructPtrEq:
    // Without proper type information, cost model cannot properly compute the
    // cost, thus hard code VF.
    if (VF == 1)
      return 1000;
    if (VF != 4)
      // Return some huge value, so that VectorCost still could be computed.
      return UnknownCost;
    break;
  default:
    // FIXME: Keep VF = 32 as unsupported right now due to huge perf
    // regressions.
    if (VF == 32)
      return UnknownCost;
  }

  LLVM_DEBUG(dbgs() << "Number of i1 calculations: " << NumberOfBoolComputations
                    << "\n");
  if (VF != 1 && NumberOfBoolComputations >= BoolInstsBailOut) {
    LLVM_DEBUG(
        dbgs() << "Returning UnknownCost due to too many i1 calculations.\n");
    return UnknownCost;
  }

  return Cost;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void VPlanCostModelProprietary::printForVPInstruction(
  raw_ostream &OS, const VPInstruction *VPInst) {
  OS << "  Cost " << getCostNumberString(getCost(VPInst)) << " for ";
  VPInst->print(OS);
  // TODO: Attributes yet to be populated.
  std::string Attributes = "";

  if (OVLSGroup *Group = VLSA->getGroupsFor(Plan, VPInst))
    if (ProcessedOVLSGroups.count(Group) != 0)
      Attributes += "OVLS ";

  if (!Attributes.empty())
    OS << " ( " << Attributes << ")";
  OS << '\n';
}

void VPlanCostModelProprietary::printForVPBasicBlock(
  raw_ostream &OS, const VPBasicBlock *VPBB) {
  OS << "Analyzing VPBasicBlock " << VPBB->getName() << ", total cost: " <<
    getCostNumberString(getCost(VPBB)) << '\n';

  // Clearing ProcessedOVLSGroups is valid while VLS works within a basic block.
  // TODO: The code should be revisited once the assumption is changed.
  // Clearing before Instruction traversal is required to allow Instruction
  // dumping function to print out correct Costs and not required for CM to
  // work properly.
  ProcessedOVLSGroups.clear();

  for (const VPInstruction &VPInst : *VPBB)
    printForVPInstruction(OS, &VPInst);
}

void VPlanCostModelProprietary::print(
  raw_ostream &OS, const std::string &Header) {
  OS << "HIR Cost Model for VPlan " << Header << " with VF = " << VF << ":\n";
  OS << "Total Cost: " << getCost() << '\n';
  LLVM_DEBUG(dbgs() << *Plan;);

  // TODO: match print order with "vector execution order".
  for (const VPBasicBlock *Block : depth_first(Plan->getEntryBlock()))
    printForVPBasicBlock(OS, Block);
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP

} // namespace vpo

} // namespace llvm
