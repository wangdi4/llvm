//===- IntelVPlanVLSAnalysis.cpp - -----------------------------------------===/
//
//   Copyright (C) 2018 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements VPlanVLSAnalysis.
///
//===----------------------------------------------------------------------===//
#include "IntelVPlanVLSAnalysis.h"
#include "IntelVPlan.h"
#if INTEL_CUSTOMIZATION
#include "VPlanHIR/IntelVPlanVLSAnalysisHIR.h"
#endif // INTEL_CUSTOMIZATION

#define DEBUG_TYPE "vplan-vls-analysis"

namespace llvm {

namespace vpo {

VPlanVLSAnalysis::MemAccessTy
VPlanVLSAnalysis::getInstructionAccessType(const VPInstruction *Inst,
                                           const unsigned Level) {
#if INTEL_CUSTOMIZATION
  unsigned Opcode = Inst->getOpcode();
  if (Opcode != Instruction::Load && Opcode != Instruction::Store)
    return MemAccessTy::Unknown;

  if (auto *I = dyn_cast<HLInst>(Inst->HIR.getUnderlyingNode())) {
    // FIXME: It's not correct to getParentLoop() for outerloop
    // vectorization.
    int64_t Stride;
    return Inst->getOpcode() == Instruction::Load
               ? VPlanVLSAnalysisHIR::getAccessType(I->getOperandDDRef(1),
                                                    Level, &Stride)
               : VPlanVLSAnalysisHIR::getAccessType(I->getLvalDDRef(), Level,
                                                    &Stride);
  }
#endif // INTEL_CUSTOMIZATION
  return MemAccessTy::Unknown;
}

/// Traverse through all VPInstructions and collect all non-unit memrefs.
/// \p Force parameter must be used by caller if \p Plan was modified so that
/// either new memrefs occurred or old ones were moved or removed. In this
/// case cached information about groups is not valid and has to be recomputed.
// TODO: For stencil case vectorizer also has to pass unit loads.
void VPlanVLSAnalysis::getOVLSMemrefs(const VPlan *Plan, const unsigned VF,
                                      const bool Force) {
  // In case of VPlan was already processed and memrefs were collected,
  // we may simply change OVLSType for each collected memref.
  auto VLSInfoIt = Plan2VLSInfo.find(Plan);
  if (!Force && VLSInfoIt != Plan2VLSInfo.end()) {
    VLSInfoIt->second.eraseGroups();
    for (auto *Memref : VLSInfoIt->second.Memrefs)
      Memref->setNumElements(VF);
    LLVM_DEBUG(
        dbgs() << "Fixed all OVLSTypes for previously collected memrefs.\n";
        this->dump());
  } else {
    // Otherwise do simple DFS and collect memrefs.
    std::function<void(const VPRegionBlock *)> CollectMemrefs =
        [&](const VPRegionBlock *Region) -> void {
      for (const VPBlockBase *Block :
           make_range(df_iterator<const VPRegionBlock *>::begin(Region),
                      df_iterator<const VPRegionBlock *>::end(Region))) {
        if (auto *NestedRegion = dyn_cast<const VPRegionBlock>(Block))
          CollectMemrefs(NestedRegion);
        else {
          auto BasicBlock = cast<VPBasicBlock>(Block);
          for (const VPRecipeBase &Recipe : *BasicBlock) {
            const auto Inst = dyn_cast<const VPInstruction>(&Recipe);
            // Currently process only master instruction.
            if (!Inst)
              continue;
            unsigned Level = -1;
            const HLNode *Node =
                Inst->HIR.isMaster()
                    ? Inst->HIR.getUnderlyingNode()
                    : Inst->HIR.getMaster()->HIR.getUnderlyingNode();
            if (Node)
              Level = Node->getParentLoop()
                          ? Node->getParentLoop()->getNestingLevel()
                          : 0;
            MemAccessTy AccTy = getInstructionAccessType(Inst, Level);

#if 0
            unsigned Opcode = Inst->getOpcode();
            if ((AccTy == MemAccessTy::Strided ||
                 AccTy == MemAccessTy::Indexed) &&
                (Opcode == Instruction::Load || Opcode == Instruction::Store))
              if (OVLSMemref *Memref = createVLSMemref(Inst, AccTy, Level, VF)) {
                VLSInfoIt->second.Memrefs.push_back(Memref);
                LLVM_DEBUG(dbgs() << "VLSA: Added instruction "; Inst->dump(););
              }
#else
            // FIXME: Decomposition doesn't create or extract RegDDRefs, so
            // try to create memrefs unconditionally.
            // This code is useless for LLVM-IR-based vectorizer.
            if (OVLSMemref *Memref = createVLSMemref(Inst, AccTy, Level, VF)) {
              VLSInfoIt->second.Memrefs.push_back(Memref);
              LLVM_DEBUG(dbgs() << "VLSA: Added instruction "; Inst->dump(););
            }
#endif
          }
        }
      }
    };

    if (VLSInfoIt != Plan2VLSInfo.end())
      VLSInfoIt->second.erase();
    else
      std::tie(VLSInfoIt, std::ignore) = Plan2VLSInfo.insert({Plan, {}});

    CollectMemrefs(cast<VPRegionBlock>(Plan->getEntry()));
  }

  // Finally run grouping of collected/changed memrefs.
  // TODO: From vectorizer perspective, formed groups should be same regardless
  // of VF, so we may not run this interface second time if we just changed
  // OVLSTypes.
  OptVLSInterface::getGroups(Plan2VLSInfo[Plan].Memrefs,
                             Plan2VLSInfo[Plan].Groups, MaxVectorWidthInBytes,
                             &Plan2VLSInfo[Plan].Mem2Group);
}

void VPlanVLSAnalysis::dump(const VPlan *Plan) const {
  dbgs() << "For the VPlan " << Plan << '\n';
  if (!Plan2VLSInfo.count(Plan)) {
    dbgs() << "\t memrefs were not collected\n";
    return;
  }
  const auto VLSInfoIt = Plan2VLSInfo.find(Plan);
  assert(VLSInfoIt != Plan2VLSInfo.end() && "No VLSInfo for a given VPlan.");
  const OVLSMemrefVector &Memrefs = VLSInfoIt->second.Memrefs;
  for (const auto Memref : Memrefs)
    cast<VPVLSClientMemrefHIR>(Memref)->dump();

  // For each collected memref print information about distance and dependency
  // to each next memref from the vector of memrefs.
  for (auto I = Memrefs.begin(), E = Memrefs.end(); I != E; ++I) {
    dbgs() << "Information about ";
    auto From = cast<VPVLSClientMemrefHIR>(*I);
    From->print(dbgs());
    dbgs() << '\n';
    for (auto J = I + 1; J != E; ++J) {
      dbgs() << "\t distance to ";
      const auto To = cast<VPVLSClientMemrefHIR>(*J);
      To->print(dbgs(), "\t");
      int64_t Dist;
      if (From->isAConstDistanceFrom(*To, &Dist))
        dbgs() << "\t" << Dist;
      else
        dbgs() << "\t Unknown";

      dbgs() << " | "
             << (From->canMoveTo(*To) ? "can be moved" : "cannot be moved");
      dbgs() << "\n";
    }
  }
}

void VPlanVLSAnalysis::dump() const {
  for (auto &PI : Plan2VLSInfo)
    dump(PI.first);
}

} // namespace vpo

} // namespace llvm
