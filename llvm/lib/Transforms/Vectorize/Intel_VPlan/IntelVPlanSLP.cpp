//===-- IntelVPlanSLP.cpp -------------------------------------------------===//
//
//   Copyright (C) 2023 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements VPlanSlp class methods.
//
//===----------------------------------------------------------------------===//

#include <queue>

#include "IntelVPlan.h"
#include "IntelVPlanSLP.h"
#include "IntelVPlanCostModel.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/DDGraph.h"

#define DEBUG_TYPE "intel-vplan-slp"

static cl::opt<unsigned> SlpUDDepthLimit(
    "vplan-slp-ud-depth-limit", cl::init(10), cl::Hidden,
    cl::desc("Limits how deep SLP pattern search code goes along "
             "use-def chain."));

static cl::opt<unsigned> SlpReportDetailLevel(
    "vplan-slp-report-detail-level", cl::init(0), cl::Hidden,
    cl::desc("Enables VPlan SLP detection verbose report"));

using namespace llvm::loopopt;

namespace llvm {

namespace vpo {

bool VPlanSlp::canMoveTo(const VPLoadStoreInst *FromInst,
                         const VPLoadStoreInst *ToInst) const {
  const RegDDRef *FromDDRef = FromInst->getHIRMemoryRef();
  const RegDDRef *ToDDRef = ToInst->getHIRMemoryRef();

  if (!FromDDRef || !ToDDRef)
    return false;

  if (FromInst == ToInst)
    return true;

  const HLDDNode *FromDDNode = FromDDRef->getHLDDNode();
  const HLDDNode *ToDDNode = ToDDRef->getHLDDNode();

  auto hasDependency =
    [](const HLDDNode *FromDDNode, const HLDDNode *ToDDNode,
       const DDEdge *Edge, const bool IsOutgoingEdge) -> bool {
      // skip self-dependence edges and input deps.
      if (Edge->getSrc() == Edge->getSink())
        return false;
      if (Edge->isInput())
        return false;

      DDRef *Ref = IsOutgoingEdge ? Edge->getSink() : Edge->getSrc();
      HLDDNode *Node = Ref->getHLDDNode();

      // Source/sink node is not in between the nodes of interest.
      if (!HLNodeUtils::isInTopSortNumRange(Node, FromDDNode, ToDDNode) &&
          !HLNodeUtils::isInTopSortNumRange(Node, ToDDNode, FromDDNode)) {
        return false;
      }

      return true;
    };

  const loopopt::DDGraph *DDG = CM->DDG;

  for (auto RefIt = FromDDRef->all_dd_begin(),
         RefEndIt = FromDDRef->all_dd_end(); RefIt != RefEndIt; ++RefIt) {
    for (auto II = DDG->outgoing_edges_begin(*RefIt),
           EE = DDG->outgoing_edges_end(*RefIt);
         II != EE; ++II) {
      if (hasDependency(FromDDNode, ToDDNode, *II, true))
        return false;
    }

    for (auto II = DDG->incoming_edges_begin(*RefIt),
           EE = DDG->incoming_edges_end(*RefIt);
         II != EE; ++II) {
      if (hasDependency(FromDDNode, ToDDNode, *II, false))
        return false;
    }
  }
  return true;
}

void VPlanSlp::collectMemRefDistances(
    const VPLoadStoreInst *BaseMem,
    ArrayRef<const VPInstruction *> Insts,
    SmallVectorImpl<ssize_t> &Distances) {

  const auto *BaseDDRef = BaseMem->getHIRMemoryRef();
  assert(BaseDDRef && "No DDRef for Base memref.");
  unsigned ElSize = BaseDDRef->getDestTypeSizeInBytes();

  for (const auto *Inst : Insts) {
    const auto *InstMem = dyn_cast<VPLoadStoreInst>(Inst);
    assert(InstMem && "Only memrefs are expected on input.");
    const auto *InstDDRef = InstMem->getHIRMemoryRef();
    assert(InstDDRef && "No DDRef for Inst memref.");

    int64_t Distance = 0;
    if (InstDDRef->getDestTypeSizeInBytes() == ElSize &&
        DDRefUtils::getConstByteDistance(BaseDDRef, InstDDRef, &Distance) &&
        (Distance % ElSize) == 0)
      Distances.push_back(Distance / ElSize);
  }
}

bool VPlanSlp::isUnitStrideMemRef(SmallVectorImpl<ssize_t> &Distances) const {
  llvm::sort(Distances);
  if (SlpReportDetailLevel >= 1) {
    LLVM_DEBUG(dbgs() << "VSLP sorted distances (bundle/graph " << BundleID
                 << '/' << GraphID << " in " << BB->getName() << "):\t";
               for (int D : Distances) {
                 dbgs() << D;
                 dbgs() << ' ';
               };
               dbgs() << '\n';);
  }

  for(ssize_t i = 1; i < static_cast<ssize_t>(Distances.size()); i++)
    if (Distances[i] != i + Distances[0])
      return false;

  return true;
}

VPInstructionCost VPlanSlp::getVectorCost(const VPInstruction *Base,
                                          unsigned VF, bool IsUnitMemref,
                                          bool IsMasked) const {
  // Apply gather/scatter cost if the operation is load/store and it is not
  // unit. If it is unit, do not call CM->getTTICostForVF() as it might
  // determine input as non unit over iterations and apply gather/scatter
  // cost rather than unit load/store cost.
  if (const auto *BaseMem = dyn_cast<VPLoadStoreInst>(Base)) {
    auto Opcode = BaseMem->getOpcode();
    auto AddrSpace = BaseMem->getPointerAddressSpace();
    auto Alignment = CM->getMemInstAlignment(BaseMem);
    auto VecTy = getWidenedType(BaseMem->getValueType(), VF);

    if (IsUnitMemref) {
      if (IsMasked)
        return CM->TTI.getMaskedMemoryOpCost(Opcode, VecTy,
                                             Alignment, AddrSpace);
      else
        return CM->TTI.getMemoryOpCost(Opcode, VecTy, Alignment, AddrSpace);
    }
    else
      return CM->TTI.getGatherScatterOpCost(
        Opcode, VecTy, CM->getLoadStoreIndexSize(BaseMem),
        IsMasked, Alignment.value(), AddrSpace);
  }

  return CM->getTTICostForVF(Base, VF);
}

bool VPlanSlp::areVectorizable(
    ArrayRef<const VPValue *> Values,
    SmallVectorImpl<const VPInstruction *> &Insts) const {
  assert(!Values.empty() && "Missed operands.");
  if (Values.empty())
    return false;

  // Determine so-called 'base' instruction in the Values array, which is
  // essential for memrefs: we move all memrefs to the first inst operand in
  // Values or to the last Inst in Values for stores.
  auto *Base = dyn_cast<VPInstruction>(Values.front());
  if (Base && Base->getOpcode() == Instruction::Store)
    Base = dyn_cast<VPInstruction>(Values.back());

  if (!Base)
    return false;

  for (const auto *V : Values) {
    // TODO: We may want to consider multiple users within vectorizable graph.
    if (V->getNumUsers() > 1)
      continue;

    auto *Inst = dyn_cast<VPInstruction>(V);
    if (!Inst)
      continue;

    // Check that the types and opcodes match for all operands and they are all
    // in the same BB.
    if (Inst->getParent() != Base->getParent() ||
        Inst->getOpcode() != Base->getOpcode())
      continue;

    // Special check for load/store value type as we care only about data
    // elements size when we load/store the data. We expect vector load/store
    // HW instruction can handle elements of different types within a single
    // vector.
    if (isa<VPLoadStoreInst>(Inst)) {
      if (CM->DL->getTypeSizeInBits(
            cast<VPLoadStoreInst>(Inst)->getValueType()) !=
          CM->DL->getTypeSizeInBits(
            cast<VPLoadStoreInst>(Base)->getValueType()))
        continue;
    }
    else if (Inst->getType() != Base->getType())
      continue;

    const auto *InstMem = dyn_cast<VPLoadStoreInst>(Inst);
    const auto *BaseMem = dyn_cast<VPLoadStoreInst>(Base);
    if (BaseMem && InstMem) {
      // Special considerations for load/store instructions: we don't want
      // conflicting load/stores to be in between instructions we try to
      // vectorize, and we want to record offsets vs the base memref if it is
      // constant.
      // We use HIR utilities thus all these checks are HIR pipeline specific
      // and loads/stores are not supported for LLVM-IR path.
      if (!canMoveTo(InstMem, BaseMem))
        continue;
    }

    Insts.push_back(Inst);
  }

  return Insts.size() > 1;
}

VPInstructionCost VPlanSlp::estimateSLPCostDifference(
    ArrayRef<const VPInstruction *> Insts) const {
  // Determine so-called 'base' instruction in the Values array, which is
  // essential for memrefs: we move all memrefs to the first inst operand in
  // Values or to the last Inst in Values for stores.
  const auto *Base = Insts.front()->getOpcode() == Instruction::Store ?
    Insts.back() : Insts.front();

  // ScalCost is just a sum of TTI cost of everything in Insts.
  VPInstructionCost ScalCost =
    std::accumulate(Insts.begin(), Insts.end(), VPInstructionCost(0),
                    [this](VPInstructionCost Cost, const VPInstruction *Inst) {
                      return Cost + CM->getTTICostForVF(Inst, 1);
                    });

  SmallVector<ssize_t, 8> Distances;
  // Now collect the distances for memrefs.
  if (const auto *BaseMem = dyn_cast<VPLoadStoreInst>(Base)) {
    collectMemRefDistances(BaseMem, Insts, Distances);

    if (SlpReportDetailLevel >= 2) {
      LLVM_DEBUG(dbgs() << "VSLP unsorted distances (bundle/graph # "
                        << BundleID << '/' << GraphID << " in "
                        << BB->getName() << "):\t";
                 for (int D : Distances) {
                   dbgs() << D;
                   dbgs() << ' ';
                 };
                 dbgs() << '\n';);
    }
  }

  // Now we have ScalCost calculated, all Values checked and Distances for
  // memrefs. Calculate the cost of vector instruction representing Values.
  //
  // For VF we pick the nearest power of two value from Values.size() rounding
  // upwards assuming that SLP can vectorize it with masked loads/stores,
  // although SLP doesn't support non power of two VFs yet.
  unsigned VF = llvm::NextPowerOf2(Insts.size());

  // If it is a memory reference. Now see if it is unit by checking
  // that values in Distance make sequence w/o gaps (SLP doesn't support
  // masked loads/stores).
  bool IsUnitMemref = Distances.size() == Insts.size() ?
    isUnitStrideMemRef(Distances) : false;

  VPInstructionCost VecCost =
    getVectorCost(Base, VF, IsUnitMemref, VF != Insts.size());

  if (SlpReportDetailLevel >= 1) {
    LLVM_DEBUG(dbgs() << "VSLP estimated costs for bundle/graph # " << BundleID
                      << '/' << GraphID << " in " << BB->getName()
                      << ", vector: "  << VecCost << ", scalar:" << ScalCost
                      << '\n');
  }

  return VecCost - ScalCost;
}

VPInstructionCost VPlanSlp::buildGraph(ArrayRef<const VPInstruction *> Seed) {
  unsigned Depth = 0;
  VPInstructionCost Cost = 0;
  // The queue stores the operands yet to be processed.
  std::queue<SmallVector<const VPValue *, 8>> WorkQueue;

  // Initialize the queue with Seed from input and proceed to the main loop.
  WorkQueue.emplace(Seed);

  while (!WorkQueue.empty() && Depth++ < SlpUDDepthLimit) {
    SmallVector<const VPValue *, 8> Values = WorkQueue.front();
    SmallVector<const VPInstruction *, 8> Insts;
    WorkQueue.pop();

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    BundleID++;
#endif // !NDEBUG || LLVM_ENABLE_DUMP

    bool Vectorizable = areVectorizable(Values, Insts);
    VPInstructionCost VCost = VPInstructionCost::getInvalid();

    if (Vectorizable)
      VCost = estimateSLPCostDifference(Insts);

    if (SlpReportDetailLevel >= 1) {
      LLVM_DEBUG(dbgs()
        << "VSLP: bundle/graph " << BundleID << '/' << GraphID << " in "
        << BB->getName() << " is" << (Vectorizable ? " " : " not ")
        << "vectorizable with VL = " << Insts.size() << ":\n";
        for (const auto *V : Values) {
          dbgs() << '\t';
          if (const auto *I = dyn_cast<VPInstruction>(V))
            I->printWithoutAnalyses(dbgs());
          else
            V->printAsOperand(dbgs());
          dbgs() << '\n';
        });
    }

    if (!VCost.isValid())
      return VPInstructionCost::getInvalid();

    // TODO:
    // Do not support VL reduction yet.
    if (Insts.size() != Values.size())
      return VPInstructionCost::getInvalid();

    Cost += VCost;

    // Insert vectors of Operands for each instructions in Insts.
    // Example with two instructions with 3 operands each:
    // Vector 0 <- { Insts[0].Op[0], Insts[1], Op[0] }
    // Vector 1 <- { Insts[0].Op[1], Insts[1], Op[1] }
    // Vector 2 <- { Insts[0].Op[2], Insts[1], Op[2] }
    //
    // Don't walk through memref operands for stores & loads.
    unsigned NumOperands = Insts[0]->getNumOperands();
    if (Insts[0]->getOpcode() == Instruction::Load)
      NumOperands = 0;
    else if (Insts[0]->getOpcode() == Instruction::Store)
      NumOperands = 1;

    for (unsigned I = 0; I < NumOperands; I++)
      WorkQueue.emplace(map_range(Insts, [I](const auto *Inst) {
        return Inst->getOperand(I);
      }));
  }
  return Cost;
}

VPInstructionCost VPlanSlp::formAndCostBundles(
  ArrayRef<const VPInstruction *> InSeed,
  std::function<bool(const VPInstruction *,
                     const VPInstruction *) > Compare,
  SmallVectorImpl<const VPInstruction *> *OutSeed) {
  unsigned constexpr BundleSizeMin = 3, BundleSizeMax = 8;
  VPInstructionCost Cost = 0;
  unsigned BaseIdx = 0, Idx;
  if (OutSeed)
    OutSeed->clear();

  for(Idx = 1; Idx < InSeed.size(); Idx++) {
    // Can proceed to the next element if there is the next element.
    if (Compare(InSeed[BaseIdx], InSeed[Idx]) &&
        (Idx - BaseIdx) < BundleSizeMax &&
        (Idx + 1) < InSeed.size())
      continue;

    // If the current memref is not in the same group as Stores[BaseIdx]
    // try to vectorize Stores[BaseIdx] .. Stores[Idx - 1] when it is large
    // enough.
    //
    // Also we hit this code when (Idx + 1) == InSeed.size() meaning that
    // we are at the last iteration of the loop and we need to process all
    // elements in [BaseIdx .. InSeed.size() - 1] range including the last
    // element.
    unsigned LastIdx = ((Idx + 1) == InSeed.size()) ? Idx : Idx - 1;
    // The size of the graph seed LastIdx - BaseIdx + 1 since we include
    // both BaseIdx and LastIdx elements into the seed.
    auto Seed = InSeed.slice(BaseIdx, LastIdx - BaseIdx + 1);

    if (Seed.size() >= BundleSizeMin) {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
      GraphID++;
#endif // !NDEBUG || LLVM_ENABLE_DUMP
      VPInstructionCost GraphCost = buildGraph(Seed);
      LLVM_DEBUG(
        dbgs() << "VSLP Cost " << GraphCost << " for the graph "
               << GraphID << " in " << BB->getName() << ". Graph seed:\n";
        for (const auto *Inst : Seed) {
          dbgs() << '\t';
          Inst->printWithoutAnalyses(dbgs());
          dbgs() << '\n';
        });
      // Update the cost only when it is profitable to vectorize.
      if (GraphCost.isValid() && GraphCost < 0)
        Cost += GraphCost;
    }
    // If the pattern is small enough (less than BundleSizeMin elements) we
    // preserve it in OutSeed array for further analysis.
    else if (OutSeed)
      OutSeed->insert(OutSeed->end(), Seed.begin(), Seed.end());

    BaseIdx = LastIdx + 1;
  }

  return Cost;
}

VPInstructionCost VPlanSlp::searchSLPPatterns(
    SmallVectorImpl<const VPInstruction *> &Seed) {
  // Sort input vector so the same symbol memrefs go bundled and sorted by
  // offset from the base.
  // Thereby, if input array is:
  // a[2], b[4], a[1], b[3]
  // after sort it is expected to be:
  // a[1], a[2], b[3], b[4]
  //
  // Do not change lexical order though. Otherwise there are chances to run
  // into unvectorizable patterns due to memory dependencies.
  llvm::sort(Seed, [&](const auto *Inst1, const auto *Inst2) {
    const auto *InstMem1 = dyn_cast<VPLoadStoreInst>(Inst1);
    const auto *InstMem2 = dyn_cast<VPLoadStoreInst>(Inst2);
    if (!InstMem1 || !InstMem2)
      return false;

    const auto *DDRef1 = InstMem1->getHIRMemoryRef();
    const auto *DDRef2 = InstMem2->getHIRMemoryRef();
    if (!DDRef1 || !DDRef2)
      return false;

    const HLDDNode *DDNode1 = DDRef1->getHLDDNode();
    const HLDDNode *DDNode2 = DDRef2->getHLDDNode();
    if (!DDNode1 || !DDNode2 ||
        DDNode1->getTopSortNum() > DDNode2->getTopSortNum())
      return false;

    if (DDRef1->getSymbase() != DDRef2->getSymbase())
      return DDRef1->getSymbase() < DDRef2->getSymbase();

    // Same symbase memrefs Sort by the distance.
    // If the distance is not a constant, getConstByteDistance() does not
    // update 'distance' and 0 is returned keeping input order.
    int64_t Distance = 0;
    DDRefUtils::getConstByteDistance(DDRef1, DDRef2, &Distance);
    return static_cast<int>(Distance) < 0;
  });

  LLVM_DEBUG(dbgs() << "VSLP: Sorted Seed array in " << BB->getName() << ":\n";
             for (const auto *I : Seed) {
               dbgs() << '\t';
               I->printWithoutAnalyses(dbgs());
               dbgs() << '\n';
             });

  // Further take out some 'similar' stores out of 'Seed' array and try to
  // SLP those.
  //
  // We don't consider VL = 2 in the current implementation.
  // TODO: Let cost modelling to decide whether VL = 2 SLP is profitable.
  //
  // We do consider VL = 3 although SLP does not support it yet but in many
  // cases it is SLP'ed with VL = 4 after unroll.
  // TODO: yet to be proven that unroll would be likely to happen and that it
  // would allow vectorization with VL 4 or higher.
  //
  // Create the longest bundle of 'similar' but less than 8 in length.
  //
  // Two functions of 'similarity' are defined for stores so far.
  // The 1-st function replicates getConstByteDistance semantics, meaning that
  // the 'similar' stores may represent a sequence of adjacent-in-memory
  // stores.
  // The 2-nd function checks the types to be able to pick stores that
  // possibly can be turned into a scatter instruction.
  //
  // Rather than deleting vectorized bundles from 'Seed' we keep remaining
  // stores in another local array for effectiveness.
  auto AreMemoryAdjacent =
    [](const VPInstruction *I1, const VPInstruction *I2) {
      const auto *S1 = dyn_cast<VPLoadStoreInst>(I1);
      const auto *S2 = dyn_cast<VPLoadStoreInst>(I2);

      if (!S1 || !S2)
        return false;

      const auto *DDRef1 = S1->getHIRMemoryRef();
      const auto *DDRef2 = S2->getHIRMemoryRef();
      int64_t Distance;

      if (!DDRef1 || !DDRef2)
        return false;

      return DDRefUtils::getConstByteDistance(DDRef1, DDRef2, &Distance);
    };

  auto AreSame32bit64bitSize =
    [this](const VPInstruction *I1, const VPInstruction *I2) {
      const auto *IMem1 = dyn_cast<VPLoadStoreInst>(I1);
      const auto *IMem2 = dyn_cast<VPLoadStoreInst>(I2);

      if (IMem1 && IMem2) {
        unsigned BW1 = CM->DL->getTypeSizeInBits(IMem1->getValueType());
        unsigned BW2 = CM->DL->getTypeSizeInBits(IMem2->getValueType());
        return (BW1 == BW2 && (BW1 == 32 || BW1 == 64));
      }

      return I1->getType() == I2->getType();
    };

  VPInstructionCost Cost = 0;
  SmallVector<const VPInstruction *, 32> LocalSeed;

  Cost += formAndCostBundles(Seed, AreMemoryAdjacent, &LocalSeed);
  Cost += formAndCostBundles(LocalSeed, AreSame32bit64bitSize);

  Seed.clear();
  return Cost;
}

VPInstructionCost VPlanSlp::estimateSLPCostDifference() {
  // Bailout in LLVM-IR pipeline.
  if (CM->DDG == nullptr)
    return 0;

  // To limit compilation time we limit the size of the chunk of instructions
  // that we process at once. It can happen that SLP pattern is split between
  // two chunks and we miss it.
  // TODO: If that appears to be the case consider overlapping search windows.
  constexpr unsigned SearchWindowSize = 64;
  // The storage for stores that are seed instructions.
  SmallVector<const VPInstruction *, SearchWindowSize> Stores;

  VPInstructionCost Cost = 0;

  for (const VPInstruction &I : BB->getInstructions()) {
    // Collect all stores but filter out those that are trivially not
    // vectorizable (volatile, no HIR node, etc.)
    // TODO: We need to extend the support to cover reductions.
    if (I.getOpcode() != Instruction::Store ||
        !cast<VPLoadStoreInst>(&I)->isSimple() ||
        cast<VPLoadStoreInst>(&I)->getHIRMemoryRef() == nullptr)
      continue;
    Stores.push_back(&I);

    // Launch the pattern search once the container size hits SearchWindowSize.
    if (Stores.size() >= SearchWindowSize)
      Cost += searchSLPPatterns(Stores);
  }

  if (Stores.size() > 1)
    Cost += searchSLPPatterns(Stores);

  return Cost;
}

} // namespace vpo

} // namespace llvm
