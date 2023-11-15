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
// This file implements VPlanSLP class methods.
//
//===----------------------------------------------------------------------===//

#include <queue>

#include "IntelVPlan.h"
#include "IntelVPlanCostModel.h"
#include "IntelVPlanPatternMatch.h"
#include "IntelVPlanSLP.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/DDGraph.h"

#define DEBUG_TYPE "intel-vplan-slp"

static cl::opt<unsigned> SLPUDDepthLimit(
    "vplan-slp-ud-depth-limit", cl::init(10), cl::Hidden,
    cl::desc("Limits how deep SLP pattern search code goes along "
             "use-def chain."));

static cl::opt<unsigned> SLPReportDetailLevel(
    "vplan-slp-report-detail-level", cl::init(0), cl::Hidden,
    cl::desc("Enables VPlan SLP detection verbose report"));

using namespace llvm::loopopt;

using namespace llvm::PatternMatch;

namespace llvm {

namespace vpo {

const VPValue *VPlanSLP::VPlanSLPNodeElement::getOperand(unsigned N) const {
  // Only first two operands replacement is supported so far.
  if (N >= 2 || AltOpcode == 0)
    return cast<VPInstruction>(getValue())->getOperand(N);

  assert(Op[N] && "Op[N] is not established yet");

  return Op[N];
}

void VPlanSLP::foldAddMulToSub(ElemMutableArrayRef Values) const {
  llvm::for_each(Values, [](VPlanSLPNodeElement &E) {
    const auto *I = dyn_cast<VPInstruction>(E.getValue());
    const VPValue *AddOp1, *AddOp2;

    if (!I || !match(I, m_Add(m_Bind(AddOp1), m_Bind(AddOp2))))
      return;

    // Checker that 'I' is a mul -1 instruction.
    // Returns non constant operand of mul instructions or null otherwise.
    auto IsMulIN1 = [](const VPInstruction *I) -> const VPValue * {
      const VPValue *MulOp;
      if (I &&
          match(I, m_c_Mul(m_Bind(MulOp), m_ConstantInt<-1, VPConstantInt>())))
        return MulOp;

      return nullptr;
    };

    // Find operand, which is Mul VPInstruction - the only supported pattern so
    // far.
    if (const auto *MulOp = IsMulIN1(dyn_cast<VPInstruction>(AddOp1)))
      E.setOpsAndAltOpcode(AddOp2, MulOp, Instruction::Sub);
    else if (const auto *MulOp = IsMulIN1(dyn_cast<VPInstruction>(AddOp2)))
      E.setOpsAndAltOpcode(AddOp1, MulOp, Instruction::Sub);
  });
}

bool VPlanSLP::canMoveTo(const VPLoadStoreInst *FromInst,
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
      if (!HLNodeUtils::isBetweenNodes(Node, FromDDNode, ToDDNode))
        return false;

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

void VPlanSLP::collectMemRefDistances(const VPLoadStoreInst *BaseMem,
                                      ElemArrayRef Values,
                                      SmallVectorImpl<ssize_t> &Distances) {

  const auto *BaseDDRef = BaseMem->getHIRMemoryRef();
  assert(BaseDDRef && "No DDRef for Base memref.");
  unsigned ElSize = BaseDDRef->getDestTypeSizeInBytes();

  for (const auto &E : Values) {
    const auto *InstMem = dyn_cast<VPLoadStoreInst>(E.getValue());
    assert(InstMem && "Only memrefs are expected on input.");
    const auto *InstDDRef = InstMem->getHIRMemoryRef();
    assert(InstDDRef && "No DDRef for memref.");

    int64_t Distance = 0;
    if (InstDDRef->getDestTypeSizeInBytes() == ElSize &&
        DDRefUtils::getConstByteDistance(BaseDDRef, InstDDRef, &Distance) &&
        (Distance % ElSize) == 0)
      Distances.push_back(Distance / ElSize);
  }
}

bool VPlanSLP::isUnitStrideMemRef(SmallVectorImpl<ssize_t> &Distances) const {
  llvm::sort(Distances);
  if (SLPReportDetailLevel >= 1) {
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

VPInstructionCost VPlanSLP::getVectorCost(const VPInstruction *Base,
                                          unsigned VF, bool IsUnitMemref,
                                          bool IsMasked) const {
  // Apply gather/scatter cost if the operation is load/store and it is not
  // unit. If it is unit, do not call CM->getTTICostForVF() as it might
  // determine input as non unit over iterations and apply gather/scatter
  // cost rather than unit load/store cost.
  if (SLPReportDetailLevel >= 3) {
    LLVM_DEBUG(dbgs() << "VSLP fetching vector cost for: ";
               Base->printAsOperand(dbgs());
               dbgs() << " with VF = " << VF << ", IsUnitMemref = "
                      << IsUnitMemref << ", IsMasked = " << IsMasked << '\n');
  }

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

bool VPlanSLP::getSplatVector(ElemArrayRef InValues,
                              ElemVectorImplTy &OutValues) const {
  // First sort InValues to group equal values in continuous chunks.
  // Can not use standard sort() algorithm comparing VPValues addresses as it
  // may form different splat vectors depending on VPValue address values and
  // eventually can cause different vectorization decision.
  // Instead do 'swap' manual sort.
  ElemVectorTy InValuesSorted;
  InValuesSorted.assign(InValues.begin(), InValues.end());
  ElemVectorTy::iterator BaseIt = InValuesSorted.begin();
  for (ElemVectorTy::iterator It1 = BaseIt + 1; It1 < InValuesSorted.end();
       It1++) {
    // Continue to the next element as long as the elements are the same as
    // *BaseIt.
    if (It1->getValue() == BaseIt->getValue())
      continue;
    // Scan for elements equal to *BaseIt in the rest of InValuesSorted.
    for (ElemVectorTy::iterator It2 = It1 + 1; It2 < InValuesSorted.end();
         It2++)
      if (It2->getValue() == BaseIt->getValue()) {
        // Move it on the current It1 position when *It2 matches *BaseIt.
        std::swap(*It1, *It2);
        // Advance It1 to continue grouping BaseIt-like elements.
        It1++;
      }
    // It1 now points at element, which does not match *BaseIt or at the end of
    // InValuesSorted. In both cases assign BaseIt to It1 and let the outer
    // loop to gather new group or to finish.
    BaseIt = It1;
  }
  OutValues.clear();

  // Scan InValues until matching pair is found. When matching pair is found
  // scan to the end of InValues and copy all values that are the same.
  ElemVectorTy::const_iterator BaseE = nullptr;
  for (const auto &E : InValuesSorted) {
    // Skip values of non-vectorizable types.
    if (!isVectorizableTy(E.getValue()->getType()) ||
        E.getValue()->getType()->isVoidTy())
      continue;

    // Continuous chunk of same Values: push them in OutValues and continue
    // scanning.
    if (BaseE && (E.getValue() == BaseE->getValue()))
      OutValues.push_back(E);
    // Get to this code when it is either:
    // *) BaseE is nullptr yet.
    // *) BaseE is not nullptr and we have not started gathering a group yet
    //    due to OutValues.empty() check.
    // In both cases set up BaseE with new value and continue scanning.
    else if (OutValues.empty())
      BaseE = &E;
    // E.getValue() != BaseE->getValue() and we have some elements in OutValues.
    // It means it is the end of BaseE group. Stop scanning.
    else
      break;
  }
  // If OutValues has at least one element we add BaseE to OutValues as we
  // haven't added it so far. All elements in OutValues are expected to hold
  // the same VPValue, which is expected to be equal to BaseE->getValue().
  // When OutValues is empty no repetitive values are detected in InValues.
  if (!OutValues.empty())
    OutValues.push_back(*BaseE);
  return OutValues.size() > 1;
}

bool VPlanSLP::getConstVector(ElemArrayRef InValues,
                              ElemVectorImplTy &OutValues) const {
  OutValues.clear();
  llvm::for_each(InValues, [&OutValues](const auto &E) {
    if (isa<VPConstant>(E.getValue()))
      OutValues.push_back(E);
  });
  return OutValues.size() > 1;
}

bool VPlanSLP::getVecInstsVector(ElemArrayRef InValues,
                                 ElemVectorImplTy &OutValues) const {
  assert(!InValues.empty() && "Missed operands.");
  if (InValues.empty())
    return false;

  ElemVectorTy LocalValues;
  ElemArrayRef::iterator BaseE = nullptr;
  OutValues.clear();

  // Loop through InValues to set BaseE in the first VPInstruction which passes
  // vectorization sanity checks. Also populate OutValues with values that are
  // possibly vectorizable with BaseE.
  //
  // Please note that the algorithm picks the first suitable BaseE, while it is
  // not always optimal and there can be other vectorization opportunities with
  // BaseE set to another instruction from InValues.
  for (const auto &E : InValues) {
    const auto *V = E.getValue();
    // Skip not vectorizable types.
    if (const auto *InstMem = dyn_cast<VPLoadStoreInst>(V)) {
      if (!isVectorizableTy(InstMem->getValueType()))
        continue;
    } else if (!isVectorizableTy(V->getType()))
      continue;

    // TODO: We may want to consider multiple users within vectorizable graph.
    if (V->getNumUsers() > 1)
      continue;

    auto *Inst = dyn_cast<VPInstruction>(V);
    if (!Inst)
      continue;

    // If BaseE is not set yet assign it to &E and continue to the next
    // element from InValues.
    if (BaseE == nullptr) {
      LocalValues.push_back(E);
      BaseE = &E;
      continue;
    }

    // Check that the types and opcodes match for all operands and they are all
    // in the same BB.
    if (Inst->getParent() !=
            cast<VPInstruction>(BaseE->getValue())->getParent() ||
        E.getOpcode() != BaseE->getOpcode() ||
        Inst->getNumOperands() !=
            cast<VPInstruction>(BaseE->getValue())->getNumOperands())
      continue;

    // Special check for load/store value type as we care only about data
    // elements size when we load/store the data. We expect vector load/store
    // HW instruction can handle elements of different types within a single
    // vector.
    if (isa<VPLoadStoreInst>(Inst)) {
      if (CM->DL->getTypeSizeInBits(
              cast<VPLoadStoreInst>(Inst)->getValueType()) !=
          CM->DL->getTypeSizeInBits(
              cast<VPLoadStoreInst>(BaseE->getValue())->getValueType()))
        continue;
    } else if (Inst->getType() != BaseE->getValue()->getType())
      continue;

    LocalValues.push_back(E);
  }

  // No vectorizable instructions are detected.
  if (!BaseE)
    return false;

  // Special considerations for load/store instructions: we don't want
  // conflicting load/stores to be in between instructions we try to vectorize.
  //
  // NOTE:
  // We use HIR utilities thus all these checks are HIR pipeline specific and
  // loads/stores are not supported for LLVM-IR path.
  //
  // We move all memrefs to the first instruction we met in InValues for loads
  // and to the last suitable instruction from Values for stores.

  const VPLoadStoreInst *BaseMem;
  if (BaseE->getOpcode() == Instruction::Store)
    BaseMem = dyn_cast<VPLoadStoreInst>(LocalValues.back().getValue());
  else
    BaseMem = dyn_cast<VPLoadStoreInst>(LocalValues.front().getValue());

  if (BaseMem) {
    for (const auto &E : LocalValues) {
      const auto *InstMem = dyn_cast<VPLoadStoreInst>(E.getValue());
      if (canMoveTo(InstMem, BaseMem))
        OutValues.push_back(E);
    }
  } else
    // Vanilla vector copy as nothing else to check to not memrefs.
    OutValues.assign(LocalValues.begin(), LocalValues.end());

  return OutValues.size() > 1;
}

VPlanSLPNodeTy
VPlanSLP::getVectorizableValues(ElemArrayRef InValues,
                                ElemVectorImplTy &OutValues) const {

  assert(!InValues.empty() && "Missed operands.");
  if (InValues.empty())
    return NotVector;

  if (getConstVector(InValues, OutValues))
    return ConstVector;
  else if (getSplatVector(InValues, OutValues))
    return SplatVector;
  else if (getVecInstsVector(InValues, OutValues))
    return InstVector;

  return NotVector;
}

VPInstructionCost
VPlanSLP::estimateSLPCostDifference(ElemArrayRef Values,
                                    VPlanSLPNodeTy NType) const {
  assert(NType != NotVector && "Invalid Node Type");

  if (Values.size() <= 1)
    return TTI::TCC_Free;

  // TODO:
  // The code supporting constants implements X86 specific cost modelling.
  // It needs to be revisited for any other target.
  //
  // Support for the vector of constants.
  // 1) The cost of an integer scalar constant value which fits 32 bits is 0 as
  // such constant is encoded as a part of instruction.
  // 2) An integer scalar constant value which does not fit 32 bits burns GPR
  // and requires a separate instruction to materialize the value.
  // 3) A float constant (vector or scalar) and vector integer constant are
  // loaded from memory.
  //
  // FP/Vector constants are normally allocated in readonly memory and naturally
  // aligned. It means that the number of cache lines spoiled due to load of
  // multiple FP scalar constant depends on their layout in memory which we have
  // no knowledge about.
  //
  // For purposes of SLP cost modelling only we assume the cost of case #1 to
  // be TCC_Free and cost of all other cases to be TCC_Basic. We also assume
  // that VL scalar loads spoils VL/2 cache lines so yields TCC_Basic * VL / 2
  // overall cost.
  if (NType == ConstVector) {
    VPInstructionCost ScalCost = TTI::TCC_Free, VecCost = TTI::TCC_Basic;

    // Special case integer constants.
    // Those which fit 32 bits do not contribute. Other contribute TCC_Basic.
    for (const auto &E : Values) {
      const auto *VPConst = cast<VPConstant>(E.getValue());
      if (const auto *IntConst =
              dyn_cast<ConstantInt>(VPConst->getConstant())) {
        if (!llvm::isIntN(32, IntConst->getSExtValue()))
          ScalCost += TTI::TCC_Basic;
      } else {
        // Any single non integer constant forces FP-case.
        ScalCost = TTI::TCC_Basic * Values.size() / 2;
        break;
      }
    }

    if (SLPReportDetailLevel >= 1) {
      LLVM_DEBUG(dbgs() << "VSLP estimated costs for bundle/graph # "
                        << BundleID << '/' << GraphID << " in " << BB->getName()
                        << ", vector: " << VecCost << ", scalar: " << ScalCost
                        << '\n');
    }
    return VecCost - ScalCost;
  }

  // Support for a splat vector: the scalar cost is a cost of a single scalar
  // instruction, the vector cost is the cost of broadcast instruction plus
  // the cost of a single scalar instruction.
  if (NType == SplatVector) {
    auto DiffCost = CM->TTI.getShuffleCost(
        TTI::SK_Broadcast,
        getWidenedType(Values[0].getValue()->getType(), Values.size()));

    if (SLPReportDetailLevel >= 1) {
      LLVM_DEBUG(dbgs() << "VSLP estimated costs for bundle/graph # "
                        << BundleID << '/' << GraphID << " in " << BB->getName()
                        << ", vector: X + " << DiffCost << ", scalar: X\n");
    }

    return DiffCost;
  }

  if (NType != InstVector)
    return VPInstructionCost::getUnknown();

  // Only VPInstructions in Values are expected below this point.
  const auto *BaseI = cast<VPInstruction>(Values.front().getValue());
  assert(BaseI && "Base VPInstruction is missed");

  // ScalCost is just a sum of TTI cost of everything in Insts.
  VPInstructionCost ScalCost = std::accumulate(
      Values.begin(), Values.end(), VPInstructionCost(0),
      [this](VPInstructionCost Cost, const auto &E) {
        return Cost + CM->getTTICostForVF(cast<VPInstruction>(E.getValue()), 1);
      });

  SmallVector<ssize_t, 8> Distances;
  // IsMasked tells when masked load/store instructions needs to be used.
  // On platforms that do not have such instructions the cost of masked
  // memory operations is prohibiting.
  bool IsMasked = false;

  // Now collect the distances for memrefs.
  if (const auto *BaseMem = dyn_cast<VPLoadStoreInst>(BaseI)) {
    collectMemRefDistances(BaseMem, Values, Distances);

    if (SLPReportDetailLevel >= 2) {
      LLVM_DEBUG(dbgs() << "VSLP unsorted distances (bundle/graph # "
                        << BundleID << '/' << GraphID << " in "
                        << BB->getName() << "):\t";
                 for (int D : Distances) {
                   dbgs() << D;
                   dbgs() << ' ';
                 };
                 dbgs() << '\n';);
    }
    // Anything that is not power of two definitely needs a mask. Small sizes
    // such 32 and 64 are covered with scalar instructions. We let TTI to handle
    // cost of even smaller sizes such as 2 .. 16 and register pumping case with
    // sizes that do not fit available HW.
    unsigned DataElBitSize = CM->DL->getTypeSizeInBits(BaseMem->getValueType());
    IsMasked = !llvm::isPowerOf2_32(DataElBitSize * Values.size());
  }

  // Now we have ScalCost calculated, all Values checked and Distances for
  // memrefs. Calculate the cost of vector instruction representing Values.
  //
  // For VF we pick the nearest power of two value from Insts.size() rounding
  // upwards for non power of two Insts.size(). We assume that SLP can vectorize
  // it with masked loads/stores, although SLP doesn't support non power of two
  // VFs yet.
  unsigned VF = llvm::PowerOf2Ceil(Values.size());

  // If it is a memory reference. Now see if it is unit by checking
  // that values in Distance make sequence w/o gaps (SLP doesn't support
  // masked loads/stores).
  bool IsUnitMemref =
      Distances.size() == Values.size() ? isUnitStrideMemRef(Distances) : false;

  VPInstructionCost VecCost = getVectorCost(BaseI, VF, IsUnitMemref, IsMasked);

  if (SLPReportDetailLevel >= 1) {
    LLVM_DEBUG(dbgs() << "VSLP estimated costs for bundle/graph # " << BundleID
                      << '/' << GraphID << " in " << BB->getName()
                      << ", vector: " << VecCost << ", scalar: " << ScalCost
                      << '\n');
  }

  return VecCost - ScalCost;
}

void VPlanSLP::tryReorderOperands(ElemMutableArrayRef Op1,
                                  ElemMutableArrayRef Op2) const {
  const VPValue *BaseV = Op1[0].getValue();
  bool IsInst = isa<VPInstruction>(BaseV);
  unsigned Opcode = IsInst ? Op1[0].getOpcode() : 0;

  // 'Similar' VPValues are likely to form vectorizable vector, not
  // guaranteed though. 'Not similar' VPValues are guaranteed to be
  // non vectorizable.
  //
  // Define the function of 'similarity' as follow:
  // 1) Equal VPValues are similar.
  // 2) VPInstructions with the same opcode are similar.
  // 3) VPValues that are not VPInstructions are similar.
  // NOTE:
  // 3) can be improved to distinguish VPConstants vs other VPValues,
  // although it is not clear whether it can manifest vectorizable
  // code which is not detected otherwise.
  // TODO:
  // We might want to establish cost-wise operands swapping.
  auto IsSimilar = [BaseV, IsInst, Opcode](const auto &E) {
    return (E.getValue() == BaseV) ||
           (!IsInst && !isa<VPInstruction>(E.getValue())) ||
           (isa<VPInstruction>(E.getValue()) && E.getOpcode() == Opcode);
  };

  for (unsigned Idx = 1; Idx < Op1.size(); Idx++) {
    if (!IsSimilar(Op1[Idx])) {
      std::swap(Op1[Idx], Op2[Idx]);
      if (SLPReportDetailLevel >= 2) {
        LLVM_DEBUG(
            dbgs() << "VSLP: swapped operands ";
            Op1[Idx].getValue()->printAsOperand(dbgs()); dbgs() << " <-> ";
            Op2[Idx].getValue()->printAsOperand(dbgs());
            dbgs() << " at index = " << Idx << " in bundle/graph " << BundleID
                   << '/' << GraphID << " in " << BB->getName() << '\n');
      }
    }
  }
}

VPInstructionCost VPlanSLP::buildGraph(ElemArrayRef Seed) {
  unsigned Depth = 0;
  VPInstructionCost Cost = 0;
  // The queue stores the operands yet to be processed.
  std::queue<ElemVectorTy> WorkQueue;

  assert(Seed.size() > 1 && "Too small Seed's size.");

  // Initialize the queue with Seed from input and proceed to the main loop.
  WorkQueue.emplace(Seed);

  while (!WorkQueue.empty() && Depth++ < SLPUDDepthLimit) {
    ElemVectorTy Values = WorkQueue.front();
    ElemVectorTy VecValues;
    WorkQueue.pop();

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    BundleID++;
#endif // !NDEBUG || LLVM_ENABLE_DUMP

    foldAddMulToSub(Values);

    VPlanSLPNodeTy NType = getVectorizableValues(Values, VecValues);
    VPInstructionCost VCost = VPInstructionCost::getInvalid();

    if (NType != NotVector)
      VCost = estimateSLPCostDifference(VecValues, NType);

    if (SLPReportDetailLevel >= 1) {
      LLVM_DEBUG(dbgs() << "VSLP: bundle/graph " << BundleID << '/' << GraphID
                        << " in " << BB->getName() << " is"
                        << (VCost.isValid() ? " " : " not ")
                        << "vectorizable with VL = "
                        << (VCost.isValid() ? VecValues.size() : Values.size())
                        << ":\n";
                 printVector(Values));
    }

    if (!VCost.isValid())
      return VCost;

    // VL can be reduced at this point if VecValues.size() < Values.size().
    // There are already processed bundles whose cost is estimated for large VL
    // and there are pending bundles in WorkQueue that are built assuming larger
    // VL. We need to bail out to rebuild the graph using new seed that excludes
    // unvectorizable elements (lanes).
    assert(VecValues.size() <= Values.size() && VecValues.size() > 1 &&
           "Vectorizable array size is out of expected bounds.");
    if (VecValues.size() > Values.size() || VecValues.size() <= 1)
      return VPInstructionCost::getInvalid();

    // TODO:
    // VL reduction is not supported yet.
    // We need to calculate new seed basing on lane indexes where VecValues are
    // and communicate this information to the caller to restart buildGraph.
    if (VecValues.size() < Values.size()) {
      LLVM_DEBUG(dbgs() << "VSLP: dropping candidate bundle/graph " << BundleID
                        << '/' << GraphID << " in " << BB->getName() << ": "
                        << VecValues.size() << " != " << Values.size() << '\n');
      return VPInstructionCost::getUnknown();
    }

    Cost += VCost;

    // We are done for non VPInstructions in VecValues. And we need to push
    // operands of VPInstructions onto the work queue yet.
    if (NType != InstVector && NType != InstShuffleVector)
      continue;

    // Insert vectors of Operands for each instructions in Insts.
    // Example with two instructions with 3 operands each:
    // Vector 0 <- { VecValues[0].Op[0], VecValues[1], Op[0] }
    // Vector 1 <- { VecValues[0].Op[1], VecValues[1], Op[1] }
    // Vector 2 <- { VecValues[0].Op[2], VecValues[1], Op[2] }
    //
    // Don't walk through memref operands for stores & loads.
    // Stop traversing at splat vectors.
    const auto *Inst0 = cast<VPInstruction>(VecValues[0].getValue());
    unsigned NumOperands = Inst0->getNumOperands();
    if (Inst0->getOpcode() == Instruction::Load)
      NumOperands = 0;
    else if (Inst0->getOpcode() == Instruction::Store)
      NumOperands = 1;

    for (unsigned I = 0; I < NumOperands; I++)
      WorkQueue.emplace(
          map_range(VecValues, [I](const auto &E) { return E.getOperand(I); }));

    // If the instruction is binary and commutative we do 1 step
    // 'look ahead' check: if the first operand's type/opcode doesn't match
    // to what we already selected we peek the second operand.
    // TODO:
    // Consider deeper look ahead check.
    // TODO:
    // Support InstShuffleVector node type.
    if (NType == InstVector &&
        Instruction::isBinaryOp(VecValues[0].getOpcode()) &&
        Instruction::isCommutative(VecValues[0].getOpcode())) {
      ElemVectorTy Op2Vec = WorkQueue.front();
      WorkQueue.pop();
      ElemVectorTy Op1Vec = WorkQueue.front();
      WorkQueue.pop();

      tryReorderOperands(Op1Vec, Op2Vec);

      WorkQueue.push(Op1Vec);
      WorkQueue.push(Op2Vec);
    }
  }
  return Cost;
}

VPInstructionCost
VPlanSLP::formAndCostBundles(ElemArrayRef InSeed,
                             std::function<bool(const VPlanSLPNodeElement &,
                                                const VPlanSLPNodeElement &)>
                                 Compare,
                             ElemVectorImplTy *OutSeed) {
  // We consider 2 <= VLs <= 16. SLP does not support non-power-of-2 VLs yet but
  // in many cases the code is SLP'ed with power-of-2 VLs after unroll.
  // TODO: yet to be proven that unroll would be likely to happen and that it
  // would allow vectorization with VL 4 or higher.
  unsigned constexpr BundleSizeMin = 2, BundleSizeMax = 16;
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
      LLVM_DEBUG(dbgs() << "VSLP Cost " << GraphCost << " for the graph "
                        << GraphID << " in " << BB->getName()
                        << ". Graph seed:\n";
                 printVector(Seed));
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

VPInstructionCost VPlanSLP::searchSLPPatterns(ElemVectorImplTy &Seed) {
  // TODO:
  // Consider sorting Seed input to form more profitable start vectors, such as
  // if input array for example is:
  // a[2], b[4], a[1], b[3]
  // after sort we would get:
  // a[1], a[2], b[3], b[4]
  //
  // We would change lexical order though and we would take chances to run into
  // unvectorizable code due to memory dependencies. So we might want to do that
  // as an additional run(s) of formAndCostBundles or prove that memory
  // dependencies allow such sort.
  //
  // The base approach is to take out some 'similar' stores out of 'Seed' array
  // and try to SLP those with help of formAndCostBundles().
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
  auto AreMemoryAdjacent = [](const auto &V1, const auto &V2) {
    const auto *S1 = dyn_cast<VPLoadStoreInst>(V1.getValue());
    const auto *S2 = dyn_cast<VPLoadStoreInst>(V2.getValue());

    if (!S1 || !S2)
      return false;

    const auto *DDRef1 = S1->getHIRMemoryRef();
    const auto *DDRef2 = S2->getHIRMemoryRef();
    int64_t Distance;

    if (!DDRef1 || !DDRef2)
      return false;

    return DDRefUtils::getConstByteDistance(DDRef1, DDRef2, &Distance);
  };

  auto AreSame32bit64bitSize = [this](const auto &V1, const auto &V2) {
    const auto *IMem1 = dyn_cast<VPLoadStoreInst>(V1.getValue());
    const auto *IMem2 = dyn_cast<VPLoadStoreInst>(V2.getValue());

    if (IMem1 && IMem2) {
      unsigned BW1 = CM->DL->getTypeSizeInBits(IMem1->getValueType());
      unsigned BW2 = CM->DL->getTypeSizeInBits(IMem2->getValueType());
      return (BW1 == BW2 && (BW1 == 32 || BW1 == 64));
    }

    return V1.getValue()->getType() == V2.getValue()->getType();
  };

  VPInstructionCost Cost = 0;
  ElemVectorTy LocalSeed;

  Cost += formAndCostBundles(Seed, AreMemoryAdjacent, &LocalSeed);
  Cost += formAndCostBundles(LocalSeed, AreSame32bit64bitSize);

  Seed.clear();
  return Cost;
}

VPInstructionCost VPlanSLP::estimateSLPCostDifference() {
  // Bailout in LLVM-IR pipeline.
  if (CM->DDG == nullptr)
    return 0;

  // To limit compilation time we limit the size of the chunk of instructions
  // that we process at once. It can happen that SLP pattern is split between
  // two chunks and we miss it.
  // TODO: If that appears to be the case consider overlapping search windows.
  constexpr unsigned SearchWindowSize = 64;
  // The storage for stores that are seed instructions.
  SmallVector<VPlanSLPNodeElement, SearchWindowSize> Stores;

  VPInstructionCost Cost = 0;

  for (const VPInstruction &I : BB->getInstructions()) {
    // Collect all stores but filter out those that are trivially not
    // vectorizable (volatile, no HIR node, etc.)
    // TODO: We need to extend the support to cover reductions.
    if (I.getOpcode() != Instruction::Store ||
        !cast<VPLoadStoreInst>(I).isSimple() ||
        cast<VPLoadStoreInst>(I).getHIRMemoryRef() == nullptr)
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

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void VPlanSLP::VPlanSLPNodeElement::dump(raw_ostream &OS) const {
  if (const auto *I = dyn_cast<VPInstruction>(getValue())) {
    I->printWithoutAnalyses(OS);
    if (getOpcode() != I->getOpcode()) {
      OS << " (altered: opcode: " << getOpcode() << ", Op0: ";
      getOperand(0)->printAsOperand(OS);
      OS << ", Op1: ";
      getOperand(1)->printAsOperand(OS);
      OS << ')';
    }
  } else
    getValue()->printAsOperand(OS);
  dbgs() << '\n';
}

// This function gets inlined even in debug compiler if implemented in the
// header file, which makes it unusable in gdb session.
void VPlanSLP::VPlanSLPNodeElement::dump() const { dump(dbgs()); }

void VPlanSLP::printVector(ElemArrayRef Elems) {
  for_each(Elems, [](const auto &E) { E.dump(dbgs()); });
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP

} // namespace vpo

} // namespace llvm
