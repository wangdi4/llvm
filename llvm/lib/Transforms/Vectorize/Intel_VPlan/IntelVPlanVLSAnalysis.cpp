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
#if INTEL_CUSTOMIZATION
#include "VPlanHIR/IntelVPlanVLSAnalysisHIR.h"
#endif // INTEL_CUSTOMIZATION

#define DEBUG_TYPE "vplan-vls-analysis"

namespace llvm {

namespace vpo {

OVLSMemref *VPlanVLSAnalysis::createVLSMemref(const VPInstruction *VPInst,
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

  OVLSType Ty(AccessSize, VF);

  // At this point we are not sure if this memref should be created. So, we
  // create a temporary memref on the stack and move it to the heap only if it
  // is strided.
  VPVLSClientMemref Memref(OVLSMemref::VLSK_VPlanVLSClientMemref, AccKind, Ty,
                           VPInst, this);
  return Memref.getConstStride() ? new VPVLSClientMemref(std::move(Memref))
                                 : nullptr;
}

void VPlanVLSAnalysis::collectMemrefs(const VPRegionBlock *Region,
                                      OVLSMemrefVector &MemrefVector,
                                      unsigned VF) {
  auto Range = make_range(df_iterator<const VPRegionBlock *>::begin(Region),
                          df_iterator<const VPRegionBlock *>::end(Region));

  for (const VPBlockBase *Block : Range) {
    if (auto *NestedRegion = dyn_cast<const VPRegionBlock>(Block)) {
      collectMemrefs(NestedRegion, MemrefVector, VF);
      continue;
    }

    auto BasicBlock = cast<VPBasicBlock>(Block);
    for (const VPInstruction &VPInst : BasicBlock->vpinstructions()) {
      auto Opcode = VPInst.getOpcode();
      if (Opcode != Instruction::Load && Opcode != Instruction::Store)
        continue;

      OVLSMemref *Memref = createVLSMemref(&VPInst, VF);
      if (!Memref)
        continue;

      MemrefVector.push_back(Memref);
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
    VLSInfoIt->second.eraseGroups();
    for (auto *Memref : VLSInfoIt->second.Memrefs)
      Memref->setNumElements(VF);
    LLVM_DEBUG(
        dbgs() << "Fixed all OVLSTypes for previously collected memrefs.\n";
        this->dump());
  } else {
    if (VLSInfoIt != Plan2VLSInfo.end())
      VLSInfoIt->second.erase();
    else
      std::tie(VLSInfoIt, std::ignore) = Plan2VLSInfo.insert({Plan, {}});

    collectMemrefs(cast<VPRegionBlock>(Plan->getEntry()),
                   VLSInfoIt->second.Memrefs, VF);
  }

  // Finally run grouping of collected/changed memrefs.
  // TODO: From vectorizer perspective, formed groups should be same regardless
  // of VF, so we may not run this interface second time if we just changed
  // OVLSTypes.
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
      dbgs() << "\t" << From->getConstDistanceFrom(*To);

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

void VPVLSClientMemref::print(raw_ostream &Os, const Twine Indent) const {
  Os << Indent;
  Os << "OVLSMemref for VPInst ";
  Inst->print(Os);
  Os << "[ ";
  Os << "id = " << getId();
  Os << " | AccessType: ";
  getAccessKind().print(Os);
  Os << " | VLSType = ";
  getType().print(Os);
  Os << " | Stride = " << getConstStride();
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
  assert(InterleaveFactor * ElementSizeInBits == 8 * (*Stride) &&
         "Stride is not a multiple of element size");

  return InterleaveFactor;
}

} // namespace vpo

} // namespace llvm
