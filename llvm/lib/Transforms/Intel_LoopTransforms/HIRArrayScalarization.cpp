//===--- HIRArrayScalarization.cpp - ---------------------*- C++ -*---===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===-----------------------------------------------------------------===//
// This file implements HIR Array Scalarization class.
//===-----------------------------------------------------------------===//

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"

#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "HIRArrayScalarization.h"

#define OPT_SWITCH "hir-array-scalarization"
#define OPT_DESC "HIR Array Scalarization"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;
using namespace llvm::loopopt::arrayscalarization;

const StringRef ArrayScalarizeTempName = "array-scalarize";
const StringRef CopyName = "copy";

STATISTIC(HIRArrayScalarizationGroupsPromoted,
          "Number of HIR Array-Scalarization Group(s) Promoted");

// -----------------------------------------------------------------------
/*  Following will be ArrayScalarizationMemRefGroup's implementation     */
// -----------------------------------------------------------------------

bool ArrayScalarizationMemRefGroup::collect(ArrayRef<RegDDRef *> ArrRef) {

  // single Ref provided: collect by following DDEdge(s)
  if (ArrRef.size() == 1) {
    RegDDRef *Ref = ArrRef[0];
    RefVec.push_back(Ref);

    // Iterate over each outgoing edge:
    for (auto I = DDG.outgoing_edges_begin(Ref),
              E = DDG.outgoing_edges_end(Ref);
         I != E; ++I) {
      DDEdge *Edge = (*I);
      RegDDRef *SrcRef = cast<RegDDRef>(Edge->getSrc());
      RegDDRef *SinkRef = cast<RegDDRef>(Edge->getSink());

      // Ignore any self edge:
      if (SinkRef != SrcRef) {
        RefVec.push_back(SinkRef);
      }
    }
  } else {
    // multiple Refs provided: save those refs as input
    for (auto *TheRef : ArrRef) {
      RefVec.push_back(TheRef);
      if (CollectSymbase) {
        SBS.insert(TheRef->getSymbase());
      }
    }
  }

  // Sort the group:
  llvm::sort(RefVec.begin(), RefVec.end(),
             [](const RegDDRef *Ref0, const RegDDRef *Ref1) {
               return Ref0->getHLDDNode()->getTopSortNum() <
                      Ref1->getHLDDNode()->getTopSortNum();
             });

  LLVM_DEBUG(dump(true););

  return !RefVec.empty();
}

// Analyze the group.
//
// each ref:
// - is a RegDDRef with constant int-only subscripts
// - its symbase is among the symbases provided
// - is non-volatile, non-address taken, and within a given lp
//
// the entire group:
// - the leading ref is the only store, and all remaining ref(s)
//   is/are load(s)
// - ref(0) dominates all other refs
// - all refs in the group are exactly the same
//
bool ArrayScalarizationMemRefGroup::analyze(void) {
  // leading ref is a store
  RegDDRef *Ref0 = RefVec[0];
  if (!Ref0->isLval()) {
    return false;
  }

  // all other refs are loads and is dominated by the leading ref
  HLDDNode *Ref0DDNode = Ref0->getHLDDNode();
  for (unsigned I = 1, E = RefVec.size(); I < E; ++I) {
    RegDDRef *Ref = RefVec[I];
    if (!Ref->isRval()) {
      return false;
    }
    if (!HLNodeUtils::dominates(Ref0DDNode, Ref->getHLDDNode())) {
      return false;
    }
  }

  for (auto *Ref : RefVec) {
    if (Ref->isAddressOf() || Ref->isVolatile() ||
        (Ref->getParentLoop() != Lp)) {
      return false;
    }

    if (!DDRefUtils::isMemRefAllDimsConstOnly(Ref)) {
      return false;
    }

    if (std::find(SBS.begin(), SBS.end(), Ref->getSymbase()) == SBS.end()) {
      return false;
    }
  }

  // Check: ref(s) in the group are exactly the same
  if (std::find_if(std::next(RefVec.begin()), RefVec.end(),
                   [&](const RegDDRef *Ref) {
                     return !DDRefUtils::areEqual(Ref, Ref0);
                   }) != RefVec.end()) {

    // Check again: still ok if equal under relaxed mode.
    if (std::find_if(
            std::next(RefVec.begin()), RefVec.end(), [&](const RegDDRef *Ref) {
              return !DDRefUtils::areEqual(Ref, Ref0, true /* relaxed */);
            }) == RefVec.end()) {
      IsRelaxedGroup = true;
    } else {
      // Show the whole group: verify there are in deed different items!
      LLVM_DEBUG({
        dbgs() << "non-equal Ref found in relaxed mode, input rejected\n";
        for (unsigned I = 0, E = RefVec.size(); I < E; ++I) {
          dbgs() << I << "\t";
          RefVec[I]->dump(1);
          dbgs() << "\n";
        }
      });

      return false;
    }
  }

  return true;
}

void ArrayScalarizationMemRefGroup::createATemp(HLLoop *Lp, RegDDRef *Ref,
                                                RegDDRef *&TmpRef) {
  assert(!TmpRef && "Expect TmpRef be a nullptr");
  HLNodeUtils &HNU = Lp->getHLNodeUtils();
  TmpRef = HNU.createTemp(Ref->getSrcType(), ArrayScalarizeTempName);
}

void ArrayScalarizationMemRefGroup::replaceRefWithTmp(HLLoop *Lp, RegDDRef *Ref,
                                                      RegDDRef *TmpRef) {
  assert(TmpRef && "Expect tmp be a valid ptr");
  assert(Ref && "Expect ref be a valid ptr");
  HLNodeUtils &HNU = Lp->getHLNodeUtils();
  HLDDNode *DDNode = Ref->getHLDDNode();
  RegDDRef *TmpRefClone = TmpRef->clone();

  HLInst *HInst = dyn_cast<HLInst>(DDNode);
  // Handle HLInst* special cases: LoadInst and StoreInst
  if (HInst) {
    const Instruction *LLVMInst = HInst->getLLVMInstruction();
    HLInst *CopyInst = nullptr;
    RegDDRef *OtherRef = nullptr;

    // StoreInst: replace with a LoadInst or CopyInst depending on the rval.
    if (isa<StoreInst>(LLVMInst) && Ref->isLval()) {
      OtherRef = DDNode->removeOperandDDRef(1);
      if (OtherRef->isMemRef()) {
        auto LInst = HNU.createLoad(OtherRef, CopyName, TmpRefClone);
        HLNodeUtils::replace(DDNode, LInst);
      } else {
        CopyInst = HNU.createCopyInst(OtherRef, CopyName, TmpRefClone);
        HLNodeUtils::replace(DDNode, CopyInst);
      }
    }
    // LoadInst: replace with a CopyInst
    else if (isa<LoadInst>(LLVMInst)) {
      OtherRef = DDNode->removeOperandDDRef(0);
      CopyInst = HNU.createCopyInst(TmpRefClone, CopyName, OtherRef);
      HLNodeUtils::replace(DDNode, CopyInst);
    }
    // Neither a Load nor a Store in HLInst*: do regular replacement
    else {
      DDNode->replaceOperandDDRef(Ref, TmpRefClone);
    }

  }
  // All other cases: do regular replacement
  else {
    DDNode->replaceOperandDDRef(Ref, TmpRefClone);
  }
}

// handle only groups of {W, R+}
bool ArrayScalarizationMemRefGroup::transform(void) {
  // create a temp outside of innermost lp
  RegDDRef *TmpRef = nullptr;
  RegDDRef *Ref0 = RefVec[0];
  createATemp(Lp, Ref0, TmpRef);
  assert(TmpRef && "Expect a valid TmpRef");

  // Create an additional cast from (i64)0 to (f64)0.0 only for relaxed group
  // where the RHS of the store is a constant integer 0.
  //
  // E.g. f64 A[N][N]
  // [FROM]
  // (i64*)A[0][0]  = 0    ;   0 is i64 type
  //  ...
  //                = A[0][0]
  //
  // [TO]
  // (i64*)A[0][0]  = 0.0  ;   0.0 is f64 type
  //  ...
  //                = A[0][0]
  //
  // After this compile-time cast, the relaxed group can be transformed as a
  // regular group.
  if (IsRelaxedGroup) {
    HLInst *PInst = dyn_cast<HLInst>(Ref0->getHLDDNode());
    RegDDRef *Rval = PInst->getRvalDDRef();

    // Only support integer const 0:
    int64_t Val = -1;
    if (!Rval->isIntConstant(&Val) || (Val != 0)) {
      LLVM_DEBUG(
          dbgs()
              << "Can't handle a relaxed group with a non-0 RHS on store\n";);
      return false;
    }

    // on Rval: replace int 0 with float 0.0
    auto &DRU = Rval->getDDRefUtils();
    RegDDRef *NullRef = DRU.createNullDDRef(Ref0->getSrcType());
    PInst->setRvalDDRef(NullRef);
    LLVM_DEBUG({
      dbgs() << "DestType: ";
      Rval->getDestType()->dump();
      dbgs() << "Node: ";
      PInst->dump(1);
    });
  }

  // replace each ref in the group with the tmp
  for (auto *Ref : RefVec) {
    replaceRefWithTmp(Lp, Ref, TmpRef);
  }

  return true;
}

void ArrayScalarizationMemRefGroup::print(raw_ostream &OS,
                                          bool PrintDetails) const {
  OS << "ASMemRefGroup: " << RefVec.size() << " -\t";
  for (auto Ref : RefVec) {
    OS << "  " << (Ref->isLval() ? " W " : " R ");
  }
  OS << "  Symbase: " << RefVec[0]->getSymbase() << "\n";

  formatted_raw_ostream FOS(OS);
  unsigned Count = 0;
  for (auto Ref : RefVec) {
    OS << "  " << Count++ << "\t";
    Ref->print(FOS, PrintDetails);
    OS << "  " << (Ref->isLval() ? "W" : "R") << "\n";
  }

  if (PrintDetails) {
    // SBS:
    FOS << "  SBS: " << SBS.size() << " {";
    for (auto I : SBS) {
      FOS << I << " ";
    }
    FOS << "\n IsRelaxedGroup: " << IsRelaxedGroup << "\n"
        << "}\n";
  }
}

// -------------------------------------------------------------------
/*  Following will be HIRArrayScalarization's implementation       */
// -------------------------------------------------------------------

// Do array scalarization by:
//
// Scan the entire innermost-loop's body, for each HLInst:
// - find its Lval: if it is a suitable RegDDRef with all int-const
//   subscritp(s):
//   . follow the DDEdge, and all uses, collect them all into a group
//   . filter non-supported group:
//     the only supported group has 1 leading W, following with 0 or more
//     read(s).
//
// - transform the supported group:
//  . create a temp in the lp's preheader:
//   -no need to create a load, since the 1st ref is a store.
//
//  . replace each ref with the tmp:
//   -no need to create a store in any loop's postexit, since the array is
//    dead after the loop.
//
bool HIRArrayScalarization::doScalarization(HLLoop *InnermostLp,
                                            SmallSet<unsigned, 8> &SBS) {
  DDG = HDDA.getGraph(InnermostLp);
  SmallVector<ArrayScalarizationMemRefGroup, 8> ASMemRefGrpVec;

  for (const HLNode &Node :
       make_range(InnermostLp->child_begin(), InnermostLp->child_end())) {
    const HLInst *HInst = dyn_cast<HLInst>(&Node);
    if (!HInst || !HInst->hasLval()) {
      continue;
    }

    // Obtain the lval ref, build its group, validate it, and proceed to
    // transform if suitable.
    const RegDDRef *LvalRef = HInst->getLvalDDRef();
    if (!LvalRef->isMemRef() ||
        !DDRefUtils::isMemRefAllDimsConstOnly(LvalRef)) {
      continue;
    }

    LLVM_DEBUG(dbgs() << "a potential ref: "; LvalRef->dump(0);
               dbgs() << "\n";);

    ArrayScalarizationMemRefGroup MRG(const_cast<RegDDRef *>(LvalRef), DDG, SBS,
                                      InnermostLp);
    if (!MRG.analyze()) {
      LLVM_DEBUG(dbgs() << "an invalid group after analysis:\n"; MRG.dump(0););
      continue;
    }

    LLVM_DEBUG({
      dbgs() << "a suitable group transformed for HIR Array Scalarization:\n";
      MRG.dump(1);
    });

    ASMemRefGrpVec.push_back(MRG);
  }

  // Bailout: if none suitable group is collected
  if (ASMemRefGrpVec.empty()) {
    LLVM_DEBUG(dbgs() << "No suitable group after collection\n";);
    return false;
  }

  // Do Transformation:
  const unsigned NumGroupsTransformed = ASMemRefGrpVec.size();
  bool Result = true;
  for (auto &MRG : ASMemRefGrpVec) {
    Result = MRG.transform() && Result;
  }

  HIRArrayScalarizationGroupsPromoted += NumGroupsTransformed;
  return Result;
}

bool HIRArrayScalarization::doScalarization(
    HLLoop *InnermostLp, SmallVectorImpl<RegDDRef *> &TheRefVec) {

  DDG = HDDA.getGraph(InnermostLp);
  SmallSet<unsigned, 8> SBS;
  ArrayScalarizationMemRefGroup MRG(TheRefVec, DDG, SBS, InnermostLp,
                                    true /* CollectSymbase */);

  if (!MRG.analyze()) {
    LLVM_DEBUG(dbgs() << "an invalid group for HIR Array Scalarization:\n";
               MRG.dump(0););
    return false;
  }

  LLVM_DEBUG({
    dbgs() << "a suitable group transformed for HIR Array Scalarization:\n";
    MRG.dump(0);
  });

  bool Result = MRG.transform();
  if (!Result) {
    LLVM_DEBUG({
      dbgs() << "Fail to transform Group - \n";
      MRG.dump(0);
    });
    return false;
  }

  ++HIRArrayScalarizationGroupsPromoted;
  return true;
}
