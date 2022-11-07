//===- IntelVPlanVLSAnalysis.cpp - -----------------------------------------===/
//
//   Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
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
#include "IntelVPlanUtils.h"
#include "IntelVPlanVLSTransform.h"
#if INTEL_CUSTOMIZATION
#include "VPlanHIR/IntelVPlanVLSAnalysisHIR.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#endif // INTEL_CUSTOMIZATION

#include "llvm/Support/CommandLine.h"

#define DEBUG_TYPE "vplan-vls-analysis"

namespace llvm {

namespace vpo {

enum VPlanVLSLevelVariant {
  VPlanVLSRunNever,
  VPlanVLSRunAuto,
  VPlanVLSRunAlways
};

cl::opt<VPlanVLSLevelVariant> VPlanVLSLevel(
    "vplan-vls-level", cl::desc("Level of VLS optimization in VPlan"),
    cl::values(clEnumValN(VPlanVLSRunNever, "never", "Disable OptVLS in VPlan"),
               clEnumValN(VPlanVLSRunAuto, "auto",
                          "Run OptVLS only for select targets"),
               clEnumValN(VPlanVLSRunAlways, "always",
                          "Always run OptVLS during loop vectorization")),
    cl::init(VPlanVLSRunAuto));

OVLSMemref *VPlanVLSAnalysis::createVLSMemref(const VPLoadStoreInst *VPInst,
                                              const unsigned VF) const {
  int Opcode = VPInst->getOpcode();
  OVLSAccessKind AccKind = OVLSAccessKind::Unknown;
  int AccessSize;

  if (Opcode == Instruction::Load) {
    AccKind = OVLSAccessKind::SLoad;
    AccessSize = DL.getTypeAllocSizeInBits(VPInst->getType());
  } else {
    assert(Opcode == Instruction::Store);
    AccKind = OVLSAccessKind::SStore;
    AccessSize = DL.getTypeAllocSizeInBits(VPInst->getOperand(0)->getType());
  }

  // Skip volatile and atomic memory accesses.
  if (auto *I = dyn_cast_or_null<Instruction>(VPInst->getUnderlyingValue()))
    if (isVolatileOrAtomic(I))
      return nullptr;

  OVLSType Ty(AccessSize, VF);

  // At this point we are not sure if this memref should be created. So, we
  // create a temporary memref on the stack and move it to the heap only if it
  // is strided.
  VPVLSClientMemref Memref(OVLSMemref::VLSK_VPlanVLSClientMemref, AccKind, Ty,
                           VPInst, this);
  return Memref.getConstStride() ? new VPVLSClientMemref(std::move(Memref))
                                 : nullptr;
}

void VPlanVLSAnalysis::collectMemrefs(
    OVLSVector<std::unique_ptr<OVLSMemref>> &MemrefVector, const VPlan *Plan,
    unsigned VF) {

  // VPlanVLSLevel option allows users to override TTI::isVPlanVLSProfitable().
  if (VPlanVLSLevel == VPlanVLSRunNever ||
      (VPlanVLSLevel == VPlanVLSRunAuto && !TTI->isVPlanVLSProfitable()))
    return;

  if (!TTI->isAggressiveVLSProfitable())
    return;

  for (const VPBasicBlock *Block : depth_first(&Plan->getEntryBlock())) {
    for (const VPInstruction &VPInst : *Block) {
      auto *LoadStore = dyn_cast<VPLoadStoreInst>(&VPInst);
      if (!LoadStore)
        continue;

      // FIXME: VPOCodeGen does not support widening of VLS groups composed of
      //        types with padding (e.g. <3 x i32> or x86_fp80). See
      //        CMPLRLLVM-23003 for more details.
      Type *MrfTy = LoadStore->getValueType();
      if (hasIrregularTypeForUnitStride(MrfTy, &DL))
        continue;

      OVLSMemref *Memref = createVLSMemref(LoadStore, VF);
      if (!Memref)
        continue;

      // FIXME: Remove this if-stmt after VLS server can handle big access
      //        sizes. At the moment, it crashes trying to compute access mask
      //        for a group if element size is greater than MAX_VECTOR_LENGTH.
      if (Memref->getType().getElementSize() >= MAX_VECTOR_LENGTH * 8) {
        delete Memref;
        continue;
      }

      MemrefVector.emplace_back(Memref);
      LLVM_DEBUG(dbgs() << "VLSA: Added instruction "; VPInst.dump(););
    }
  }
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
    for (auto &Memref : VLSInfoIt->second.Memrefs)
      Memref->setNumElements(VF);
    LLVM_DEBUG(
        dbgs() << "Fixed all OVLSTypes for previously collected memrefs.\n";
        this->dump());
    // Clean grouping info so it won't merge with new analysis results.
    Plan2VLSInfo[Plan].Groups.clear();
    Plan2VLSInfo[Plan].Mem2Group.clear();
  } else {
    if (VLSInfoIt != Plan2VLSInfo.end())
      VLSInfoIt->second.erase();
    else {
      VLSInfoIt = Plan2VLSInfo.insert(std::make_pair(Plan, VLSInfo{})).first;
    }
    collectMemrefs(VLSInfoIt->second.Memrefs, Plan, VF);
  }

  // Finally run grouping of collected/changed memrefs.
  OptVLSInterface::getGroups(Plan2VLSInfo[Plan].Memrefs,
                             Plan2VLSInfo[Plan].Groups, MaxVectorWidthInBytes,
                             &Plan2VLSInfo[Plan].Mem2Group);
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void VPlanVLSAnalysis::dump(const VPlan *Plan) const {
  dbgs() << "For the VPlan " << Plan << '\n';
  if (!Plan2VLSInfo.count(Plan)) {
    dbgs() << "\t memrefs were not collected\n";
    return;
  }
  const auto VLSInfoIt = Plan2VLSInfo.find(Plan);
  assert(VLSInfoIt != Plan2VLSInfo.end() && "No VLSInfo for a given VPlan.");
  for(auto &Memref : VLSInfoIt->second.Memrefs)
    Memref->dump();

  // For each collected memref print information about distance and dependency
  // to each next memref from the vector of memrefs.
  for (auto I = VLSInfoIt->second.Memrefs.begin(),
            E = VLSInfoIt->second.Memrefs.end();
       I != E; ++I) {
    dbgs() << "Information about ";
    auto *From = I->get();
    From->print(dbgs());
    dbgs() << '\n';
    for (auto J = I + 1; J != E; ++J) {
      dbgs() << "\t distance to ";
      const auto *To = J->get();
      To->print(dbgs(), 2);
      dbgs() << "  " << From->getConstDistanceFrom(*To);

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

void VPVLSClientMemref::print(raw_ostream &OS, unsigned Indent) const {
  OVLSMemref::print(OS, Indent);
  OS << ": ";
  Inst->printWithoutAnalyses(OS);
}

#endif // !NDEBUG || LLVM_ENABLE_DUMP

/// InterleaveIndex is a distance (in elements) of a \p Memref from the first
/// memory reference in the \p Group.
int computeInterleaveIndex(OVLSMemref *Memref, OVLSGroup *Group) {
  OVLSMemref *FirstMemref = Group->getFirstMemref();
  Optional<int64_t> Offset = Memref->getConstDistanceFrom(*FirstMemref);
  assert(Offset && "Memref is from another group?");

  auto ElementSizeInBits = Memref->getType().getElementSize();
  int InterleaveIndex = *Offset / (ElementSizeInBits / 8);
  assert(InterleaveIndex * ElementSizeInBits == (*Offset) * 8 &&
         "Offset is not a multiple of element size");

  return InterleaveIndex;
}

/// InterleaveFactor is a stride of a \p Memref (in elements).
int computeInterleaveFactor(OVLSMemref *Memref) {
  Optional<int64_t> Stride = Memref->getConstStride();
  assert(Stride && "Interleave factor requested for non-strided accesses");

  auto ElementSizeInBits = Memref->getType().getElementSize();
  int InterleaveFactor = *Stride / (ElementSizeInBits / 8);
  assert(InterleaveFactor * (int)ElementSizeInBits == 8 * (*Stride) &&
         "Stride is not a multiple of element size");

  return InterleaveFactor;
}
} // namespace vpo

} // namespace llvm
