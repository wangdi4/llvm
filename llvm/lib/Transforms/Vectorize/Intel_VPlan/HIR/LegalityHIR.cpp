//===-- LegalityHIR.cpp ---------------------------------------------------===//
//
//   INTEL CONFIDENTIAL
//
//   Copyright (C) 2017 Intel Corporation
//
//   This software and the related documents are Intel copyrighted materials,
//   and your use of them is governed by the express license under which they
//   were provided to you ("License").  Unless the License provides otherwise,
//   you may not use, modify, copy, publish, distribute, disclose or treansmit
//   this software or the related documents without Intel's prior written
//   permission.
//
//   This software and the related documents are provided as is, with no
//   express or implied warranties, other than those that are expressly
//   stated in the License.
//
//===----------------------------------------------------------------------===//
///
///  \file LegalityHIR.cpp
///  VPlan vectorizer's HIR legality analysis.
///
///  Split from IntelVPlanHCFGBuilderHIR.cpp on 2023-10-03.
///
//===----------------------------------------------------------------------===//

#include "LegalityHIR.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRParVecAnalysis.h"

#define DEBUG_TYPE "VPlanLegality"

using namespace llvm;
using namespace vpo;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void LegalityHIR::dump(raw_ostream &OS) const {
  OS << "HIRLegality Descriptor Lists\n";
  OS << "\n\nHIRLegality PrivatesList:\n";
  for (auto &Pvt : PrivatesList) {
    Pvt.dump();
    OS << "\n";
  }
  OS << "\n\nHIRLegality PrivatesNonPODList:\n";
  for (auto &NPPvt : PrivatesNonPODList) {
    NPPvt.dump();
    OS << "\n";
  }
  OS << "\n\nHIRLegality PrivatesF90DVList:\n";
  for (auto &F90DVPvt : PrivatesF90DVList) {
    F90DVPvt.dump();
    OS << "\n";
  }
  OS << "\n\nHIRLegality LinearList:\n";
  for (auto &Lin : LinearList) {
    Lin.dump();
    OS << "\n";
  }
  OS << "\n\nHIRLegality ReductionList:\n";
  for (auto &Red : ReductionList) {
    Red.dump();
    OS << "\n";
  }
  OS << "\n\nHIRLegality UDRList:\n";
  for (auto &UDR : UDRList) {
    UDR.dump();
    OS << "\n";
  }
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP

/// Check if the incoming \p Ref matches the original SIMD descriptor DDRef \p
/// DescrRef.
bool LegalityHIR::isSIMDDescriptorDDRef(const RegDDRef *DescrRef,
                                        const DDRef *Ref, bool isF90DV) const {
  assert(DescrRef->isAddressOf() &&
         "Original SIMD descriptor ref is not address of type.");

  auto *RegRef = dyn_cast<RegDDRef>(Ref);
  if (RegRef) {
    if (!RegRef->isMemRef())
      return false;

    // Since we know descriptor ref is always address of type, call dedicated
    // compare.
    if (DDRefUtils::areEqualWithoutAddressOf(DescrRef, RegRef) ||
        // Compares BaseCE and number of dimension for DescrRef and RegRef.
        // We can have situation like below which should be recognized:
        // DescrRef: &((%sum)[0])
        // RegRef: (%sum)[0].0
        DDRefUtils::haveEqualBaseAndShape(DescrRef, RegRef,
                                          false /* RelaxedMode */))
      return true;

    if (isF90DV && DescrRef->getBasePtrSymbase() == RegRef->getBasePtrSymbase())
      return true;
  } else {
    // Special casing for incoming Ref of the form %s which was actually the
    // Base CE of the memref %s[0]
    auto *DescrRefCE = DescrRef->getBaseCE();
    if (auto *BDDR = dyn_cast<BlobDDRef>(Ref)) {
      // Additional checks for when the SIMD descriptor DDRef has struct offsets
      // associated with it. A descriptor ref like &(%s[0].field) shouldn't
      // match with %s, so if the descriptor ref has struct offsets then we need
      // to check whether the incoming ref matches those.
      // Example HIR:
      // %tok = ... region.entry(); [ ...,
      //     QUAL.OMP.LINEAR:IV.TYPED(&((%struct.var)[0].1.3)011) ] %smax =
      //     @llvm.smax.i32(%.lb,  %.ub);
      // + DO i1 = 0, ... , 1   <DO_LOOP>
      // |   (%struct.var)[0].1.0[i1 + sext.i32.i64(trunc.i64.i32(%.lb))] = i1;
      // + END LOOP
      // (%struct.var)[0].1.3 = %smax + 1;
      // @llvm.directive.region.exit(%tok); [ DIR.OMP.END.SIMD() ]
      bool RefsMatchWithStructOffsets = false;
      if (DescrRef->hasTrailingStructOffsets()) {
        // Try to get struct fields of incoming ref, if they exist
        auto *ParentRef = BDDR->getParentDDRef();
        if (!ParentRef->hasGEPInfo())
          return false;
        if (ParentRef->getNumDimensions() == DescrRef->getNumDimensions() + 1) {
          // Consider cases where descriptor ref looks like &(%s[0].field) while
          // the ref being matched against looks like %s[0].field[i1]. Instead
          // of simply comparing the base pointer operand, we drop the last
          // dimension of the incoming ref and compare the two refs to ensure
          // that they match along with the struct offsets.
          auto *ParentRefClone = ParentRef->clone();
          ParentRefClone->removeDimension(1);
          ParentRef = ParentRefClone;
        }
        RefsMatchWithStructOffsets =
            DDRefUtils::areEqualWithoutAddressOf(DescrRef, ParentRef);
      } else {
        RefsMatchWithStructOffsets = true;
      }
      auto *RefCE = BDDR->getSingleCanonExpr();
      if (RefsMatchWithStructOffsets &&
          CanonExprUtils::areEqual(DescrRefCE, RefCE))
        return true;
    }
  }
  return false;
}

void LegalityHIR::recordPotentialSIMDDescrUse(DDRef *Ref) {

  // Check, whether Ref is POD or non-POD Private
  DescrWithAliasesTy *Descr = getPrivateDescr(Ref);
  if (!Descr)
    Descr = getPrivateDescrNonPOD(Ref);
  if (!Descr)
    Descr = getPrivateDescrF90DV(Ref);
  // If Ref is not private check if it is linear reduction
  if (!Descr)
    Descr = getLinearRednDescriptors(Ref);

  // If Ref does not correspond to SIMD descriptor then nothing to do
  if (!Descr)
    return;

  assert(isa<RegDDRef>(Descr->getRef()) &&
         "The original SIMD descriptor Ref is not a RegDDRef.");
  if (isSIMDDescriptorDDRef(cast<RegDDRef>(Descr->getRef()), Ref)) {
    // Ref refers to the original descriptor
    // TODO: should we assert that InitVPValue is not set already?
    // Tracked in CMPLRLLVM-52295.
    if (auto *DescrWithInit = dyn_cast<DescrWithInitValueTy>(Descr))
      DescrWithInit->setInitValue(Ref);
  } else {
    // Ref is an alias to the original descriptor
    auto AliasIt = Descr->findAlias(Ref);
    assert(AliasIt && "Alias not found.");
    if (auto *Alias = dyn_cast<DescrWithInitValueTy>(AliasIt))
      Alias->setInitValue(Ref);
  }
}

bool LegalityHIR::mapsToSIMDDescriptor(const DDRef *Ref) {
  // Check whether Ref is POD private
  DescrWithAliasesTy *Descr =
      findDescrThatUsesSymbase<PrivDescrTy>(PrivatesList, Ref);
  // Check whether Ref is non-POD private
  if (!Descr)
    Descr =
        findDescrThatUsesSymbase<PrivDescrNonPODTy>(PrivatesNonPODList, Ref);
  // Check whether Ref is F90DV private
  if (!Descr)
    Descr = findDescrThatUsesSymbase<PrivDescrF90DVTy>(PrivatesF90DVList, Ref);

  // If Ref does not correspond to SIMD descriptor then nothing to do
  if (!Descr)
    return false;
  return true;
}

void LegalityHIR::recordPotentialSIMDDescrUpdate(HLInst *UpdateInst) {
  RegDDRef *Ref = UpdateInst->getLvalDDRef();

  // Instruction does not write into any LVal, bail out of analysis
  if (!Ref)
    return;

  // Check whether Ref is POD or non-POD Private
  DescrWithAliasesTy *Descr = getPrivateDescr(Ref);
  // Check whether Ref is non-POD private
  if (!Descr)
    Descr = getPrivateDescrNonPOD(Ref);
  // Check whether Ref is F90DV private
  if (!Descr)
    Descr = getPrivateDescrF90DV(Ref);
  // If Ref is not private check if it is linear reduction
  if (!Descr)
    Descr = getLinearRednDescriptors(Ref);

  // If Ref does not correspond to SIMD descriptor then nothing to do
  if (!Descr)
    return;

  assert(isa<RegDDRef>(Descr->getRef()) &&
         "The original SIMD descriptor Ref is not a RegDDRef.");
  if (isSIMDDescriptorDDRef(cast<RegDDRef>(Descr->getRef()), Ref)) {
    // Ref refers to the original descriptor
    Descr->addUpdateInstruction(UpdateInst);
  } else {
    // Ref is an alias to the original descriptor
    auto Alias = Descr->findAlias(Ref);
    assert(Alias && "Alias not found.");
    Alias->addUpdateInstruction(UpdateInst);
  }
}

bool LegalityHIR::canVectorize(const WRNVecLoopNode *WRLp) {
  clearBailoutRemark();
  // Send explicit data from WRLoop to the Legality.
  bool RetVal = EnterExplicitData(WRLp);
  assert((RetVal || BR.BailoutRemark) &&
         "EnterExplicitData didn't set bailout data!");
  return RetVal;
}

void LegalityHIR::findAliasDDRefs(HLNode *BeginNode, HLNode *EndNode,
                                  HLLoop *HLoop) {

  // Containers to collect all nodes that are present before/after HLoop to
  // process for potential aliases.
  SetVector<HLNode *> PreLoopNodes;
  SetVector<HLNode *> PostLoopNodes;

  // Collect nodes between the begin-SIMD clause directive and the HLLoop node.
  HLNode *CurNode = BeginNode;
  while (auto *NextNode = CurNode->getNextNode()) {
    if (NextNode == HLoop)
      break;
    PreLoopNodes.insert(NextNode);
    CurNode = NextNode;
  }
  // Collect nodes present in HLLoop's preheader.
  auto PreRange =
      map_range(HLoop->preheaderNodes(), [](HLNode &N) { return &N; });
  PreLoopNodes.insert(PreRange.begin(), PreRange.end());

  // Collect nodes present in HLLoop's postexit.
  // In some cases the EndNode can reside in the loop post exit. Thus we might
  // miss it going by the nodes after the loop. See the
  // hir_simd_directives_preheader_postexit.ll for an example of such EndNode
  // placement.
  bool EndFound = false;
  for (HLNode &Node : HLoop->postExitNodes()) {
    if (&Node == EndNode) {
      EndFound = true;
      break;
    }
    PostLoopNodes.insert(&Node);
  }
  // Collect nodes between the HLLoop node and the end-SIMD clause directive.
  if (!EndFound) {
    CurNode = HLoop->getNextNode();
    while (CurNode && CurNode != EndNode) {
      PostLoopNodes.insert(CurNode);
      CurNode = CurNode->getNextNode();
    }
    assert(CurNode && "can't find region end");
  }
  auto getDescr = [this](RegDDRef *Ref) {
    // Check if Ref is any of explicit SIMD descriptors.
    DescrWithAliasesTy *Descr = getPrivateDescr(Ref);
    if (!Descr)
      Descr = getPrivateDescrNonPOD(Ref);
    if (!Descr)
      Descr = getPrivateDescrF90DV(Ref);
    if (!Descr)
      Descr = getLinearRednDescriptors(Ref);
    return Descr;
  };
  auto addAlias = [](DescrWithAliasesTy *Descr, DDRef *Val) {
    if (isa<DescrWithInitValueTy>(Descr))
      Descr->addAlias(Val, std::make_unique<DescrWithInitValueTy>(Val));
    else
      Descr->addAlias(Val, std::make_unique<DescrWithAliasesTy>(Val));
  };
  // Process all pre-loop nodes.
  for (HLNode *PLN : PreLoopNodes) {
    LLVM_DEBUG(dbgs() << "PreHLLoop node: "; PLN->dump(););
    // Evaluate Rvals of only HLInsts in the pre-loop nodes.
    auto *HInst = dyn_cast<HLInst>(PLN);
    // TODO: Check whether we really can have HLoop, HLIf, or other-non-HInst
    // things here (between the simd-region begin statement and the loop). If
    // so we need a special processing for them. Same for the postloop nodes.
    // Tracked in CMPLRLLVM-52295.
    if (!HInst)
      continue;
    RegDDRef *RVal = HInst->getRvalDDRef();
    if (!RVal)
      continue;

    DescrWithAliasesTy *Descr = getDescr(RVal);
    // RVal is not a SIMD descriptor, move to next HLInst.
    if (!Descr)
      continue;

    LLVM_DEBUG(dbgs() << "Found an alias for a SIMD descriptor ref ";
               RVal->dump(); dbgs() << "...."; HInst->dump(); dbgs() << "\n");
    RegDDRef *LVal = HInst->getLvalDDRef();
    assert(LVal && "HLInst in the preheader does not have an Lval.");
    addAlias(Descr, LVal);
  }
  // Process all post-loop nodes.
  for (HLNode *PLN : PostLoopNodes) {
    LLVM_DEBUG(dbgs() << "PostHLLoop node: "; PLN->dump(););
    // Evaluate LVals of only HLInsts in the pre-loop nodes.
    auto *HInst = dyn_cast<HLInst>(PLN);
    if (!HInst)
      continue;
    RegDDRef *LVal = HInst->getLvalDDRef();
    if (!LVal)
      continue;

    DescrWithAliasesTy *Descr = getDescr(LVal);
    // LVal is not a SIMD descriptor, move to next HLInst.
    if (!Descr)
      continue;

    LLVM_DEBUG(dbgs() << "Found an alias for a SIMD descriptor ref ";
               LVal->dump(); dbgs() << "...."; HInst->dump(); dbgs() << "\n");
    RegDDRef *RVal = HInst->getRvalDDRef();
    assert(RVal && "HLInst in the postexit does not have an Rval.");
    // TODO: extend to non-terminals. That might be useful for inductions.
    // Currently, we can bailout on non-recognized phi for such entities.
    // Tracked in CMPLRLLVM-52295.
    if (RVal->isTerminalRef())
      addAlias(Descr, RVal);
  }
  LLVM_DEBUG(dbgs() << "HIR legality after collecting aliases\n"; dump(););
}

const HIRVectorIdioms *
LegalityHIR::getVectorIdioms(HLLoop *Loop) const {
  IdiomListTy &IdiomList = VecIdioms[Loop];
  if (!IdiomList) {
    IdiomList.reset(new HIRVectorIdioms());
    HIRVectorIdiomAnalysis Analysis;
    Analysis.gatherIdioms(TTI, *IdiomList, DDAnalysis->getGraph(Loop), *SRA,
                          Loop);
  }
  return IdiomList.get();
}

bool LegalityHIR::isMinMaxIdiomTemp(const DDRef *Ref, HLLoop *Loop) const {
  auto *Idioms = getVectorIdioms(Loop);
  for (auto &IdiomDescr : make_range(Idioms->begin(), Idioms->end()))
    if ((IdiomDescr.second == HIRVectorIdioms::IdiomId::MinOrMax ||
         IdiomDescr.second == HIRVectorIdioms::IdiomId::MMFirstLastIdx ||
         IdiomDescr.second == HIRVectorIdioms::IdiomId::MMFirstLastVal) &&
        DDRefUtils::areEqual(
            static_cast<const HLInst *>(IdiomDescr.first)->getLvalDDRef(), Ref))
      return true;

  return false;
}
