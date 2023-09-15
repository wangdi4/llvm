//===-- HIRPMSymbolicTripCountCompleteUnroll.h -----------===//
// HIR Loop Pattern Match Pass for Symbolic TripCount 2-level loop nest.
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
#if INTEL_FEATURE_SW_ADVANCED
// This is a special pattern-recognition and matching pass for JIRA-43393, a
// 4.5% performance gain!
//
// [SOURCE CODE: cpu2017/541.leela/FastBoard.cpp]
// void FastBoard::add_neighbour(const int i, const int color) {
//  assert(color == WHITE || color == BLACK || color == EMPTY);
//
//  std::tr1::array<int, 4> nbr_pars;
//  int nbr_par_cnt = 0;
//
//  for (int k = 0; k < 4; k++) { //OuterLp
//    int ai = i + m_dirs[k];
//
//    m_neighbours[ai] += (1 << (NBR_SHIFT*color)) - (1 << (NBR_SHIFT*EMPTY));
//
//    bool found = false;
//    for (int i = 0; i < nbr_par_cnt; i++) {	// InnerLp
//      if (nbr_pars[i] == m_parent[ai]) {
//        found = true;
//        break;
//      }
//    }
//
//    if (!found) {
//      m_libs[m_parent[ai]]--;
//      nbr_pars[nbr_par_cnt++] = m_parent[ai];
//    }
//  }
//}
// This function produces LIT a testcase
// hir-pm-symbolictripcountcompleteunroll0.ll
//
//
// *** HIR of the LoopNest BEFORE Pattern Match ***
//
//<0>  BEGIN REGION { }
//
//<61>  + DO i1 = 0, 3, 1   <DO_LOOP>  // OuterLp
//<2>   |   %nbr_par_cnt.061.out = %nbr_par_cnt.061;
//<4>   |   %2 = (%this)[0].12.0[i1];
//<8>   |   %3 = (%this)[0].10.0[%2 + %i];
//<12>  |   (%this)[0].10.0[%2 + %i] = zext.i16.i32(%3)+%shl.neg+256;
//<15>  |   %5 = (%this)[0].7.0[%2 + %i];
//
//<17>  |   if (%nbr_par_cnt.061 > 0)                // HLIF0
//<17>  |   {
//<62>  |      + DO i2 = 0, smax(1, sext.i32.i64(%nbr_par_cnt.061)) + -1, 1
//                 <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 4>  //InnerLp
//<29>  |      |   if ((%nbr_pars)[0].0[i2] == %5)
//<29>  |      |   {
//<30>  |      |      goto if.end32.loopexit;        // HLIF1
//<29>  |      |   }
//<62>  |      + END LOOP
//<62>  |
//<39>  |      goto if.then18;
//<40>  |      if.end32.loopexit:
//<41>  |      goto if.end32;
//<17>  |   }
//
//<42>  |   if.then18:
//<45>  |   %8 = (%this)[0].8.0[%5];
//<47>  |   (%this)[0].8.0[%5] = %8 + 1;
//<48>  |   %9 = (%this)[0].7.0[%2 + %i];
//<50>  |   %nbr_par_cnt.061 = %nbr_par_cnt.061  +  1;
//<52>  |   (%nbr_pars)[0].0[%nbr_par_cnt.061.out] = %9;
//<54>  |   if.end32:
//<61>  + END LOOP
//
//<0>   END REGION
//
//
// *** HIR of the LoopNest AFTER Pattern Match ***
//
//<0>  BEGIN REGION { modified }
// unrolled iter0, with final store sinked:
//<64>   %2 = (%this)[0].12.0[0];
//<65>   %3 = (%this)[0].10.0[%2 + %i];
//<66>   (%this)[0].10.0[%2 + %i] = zext.i16.i32(%3) + %shl + 65280;
//<67>   %5 = (%this)[0].7.0[%2 + %i];
//<68>   %8 = (%this)[0].8.0[%5];
//
// unrolled iter1, with final store sinked, and all temp-definitions renamed;
//<70>   %mv = (%this)[0].12.0[1];
//<71>   %mv2 = (%this)[0].10.0[%i + %mv];
//<72>   (%this)[0].10.0[%i + %mv] = %shl + zext.i16.i32(%mv2) + 65280;
//<73>   %mv3 = (%this)[0].7.0[%i + %mv];
//<74>   %mv4 = (%this)[0].8.0[%mv3];
//
// unrolled iter2, with final store sinked, and all temp-definitions renamed;
//<76>   %mv5 = (%this)[0].12.0[2];
//<77>   %mv6 = (%this)[0].10.0[%i + %mv5];
//<78>   (%this)[0].10.0[%i + %mv5] = %shl + zext.i16.i32(%mv6) + 65280;
//<79>   %mv7 = (%this)[0].7.0[%i + %mv5];
//<80>   %mv8 = (%this)[0].8.0[%mv7];
//
// unrolled iter3, with final store sinked, and all temp-definitions renamed;
//<82>   %mv9 = (%this)[0].12.0[3];
//<83>   %mv10 = (%this)[0].10.0[%i + %mv9];
//<84>   (%this)[0].10.0[%i + %mv9] = %shl + zext.i16.i32(%mv10) + 65280;
//<85>   %mv11 = (%this)[0].7.0[%i + %mv9];
//<86>   %mv12 = (%this)[0].8.0[%mv11];
//
// store-sink area:
//<69>   (%this)[0].8.0[%5] = %8 + -1;       // from iter0
//<75>   (%this)[0].8.0[%mv3] = %mv4 + -1;   // from iter1
//<81>   (%this)[0].8.0[%mv7] = %mv8 + -1;   // from iter2
//<87>   (%this)[0].8.0[%mv11] = %mv12 + -1; // from iter3
//
//<0>  END REGION

// ***Non-local storage Mapping on LLVM IR level ***
//
// m_neighbour <-> (%this)[0].10.0
// m_dirs      <-> (%this)[0].12.0
// m_libs      <-> (%this)[0].08.0
// m_parent    <-> (%this)[0].07.0
//
// Notice a few things that clang had transformed the function to when reaching
// HIR level inside the HIR loop optimizer:
//
// - (1) OuterLp: this maps to the OuterLp's structure on source code (i1);
//
// - (2) OuterLp has HLIF0, and HLIF0 has the entire InnerLp + a few label(s)
//       goto(s);
//
// - (3) HLIF1: the HLIf structure inside InnerLp.
//       It has the side-exit for InnerLp.
//
// - (4) HLIF2: the 2nd HLIf structure on the source level is GONE.
//       Its condition is flip-ed, and its THEN_BODY is flattened and merged
//       into OuterLp's body;
//
// A very similar pattern also exists in cpu2017/541.leela/FastBoard.cpp for
// function: void FastBoard::remove_neighbour(.) {.}.
// This becomes a LIT named testcase hir-pm-symbolictripcountcompleteunroll1.ll.
//
#endif //INTEL_FEATURE_SW_ADVANCED

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UNROLLSYMTCIMPL_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UNROLLSYMTCIMPL_H

#include "llvm/Pass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

namespace llvm {
class Function;

namespace loopopt {

class DDGraph;
class HIRFramework;
class HIRDDAnalysis;
#if INTEL_FEATURE_SW_ADVANCED

namespace unrollsymtc {

typedef SmallVector<HLNode *, 16> NodeVecTy;

class HIRPMSymbolicTripCountCompleteUnroll {

private:
#ifndef NDEBUG
  HIRFramework &HIRF;
#endif
  const TargetTransformInfo &TTI;
  HIRDDAnalysis &HDDA;
  HLNodeUtils &HNU;

  HLLoop *OuterLp = nullptr;
  HLLoop *InnerLp = nullptr;
  HLIf *HLIF0 = nullptr;
  HLIf *HLIF1 = nullptr;

  NodeVecTy OuterLpNodeVec; // HLNode(s) in OuterLp
  SmallVector<HLInst *, 16> OuterLpInstVec;
  SmallVector<HLLabel *, 4> OuterLpLabelVec;
  SmallVector<HLGoto *, 4> OuterLpGotoVec;

  NodeVecTy InnerLpNodeVec; // HLNode(s) in InnerLp

  SmallVector<RegDDRef *, 16> NonLocalRefVec;
  SmallVector<RegDDRef *, 16> MParentRefVec;
  SmallVector<RegDDRef *, 16> MLibsRefVec;

  struct StructuralCollector;

public:
  HIRPMSymbolicTripCountCompleteUnroll(HIRFramework &HIRF,
                                       const TargetTransformInfo &TTI,
                                       HIRDDAnalysis &HDDA)
      :
#ifndef NDEBUG
    HIRF(HIRF),
#endif
    TTI(TTI), HDDA(HDDA), HNU(HIRF.getHLNodeUtils()) {}

  bool run();

private:
  // main entry function to this pass on a given InnerLoop
  bool doLoopPatternMatch(HLLoop *InnerLoop);

  // Analyze the LoopNest, look for the expected pattern, and bail out ASAP if
  // the pattern is not found.
  //
  // Partition the Pattern Recognition process into the following sections:
  // - Collect loop-nest structures and do Preliminary structural Tests;
  // - Pattern Tests;
  // - Legal Tests;
  //
  bool doAnalysis(HLLoop *InnerLp);

  // Do pattern recognition on matching the following structures in order:
  // - OuterLp
  // - InnerLp
  // - HLIF0
  // - HLIF1
  // - Deep OuterLp Pattern Match
  // - Deep InnerLp Pattern Match
  //
  bool isPattern(void);

  // Do 3 Legal tests:
  // - m_parent[] is ReadOnly within the LoopNest;
  // - OuterLp has no LiveOut (in particular: nbr_cnt is not LiveOut of OuterLp)
  // - m_parent[] is NOT aliased to m_libs[];
  //   [note: disabled by default, need improvement on Alias Analysis 1st.]
  //
  bool isLegal(void);

  // Check: m_parent[.] DDRef is READONLY
  bool isMParentReadOnly(void);

  // Check: m_parent[] and m_libs[] are not aliased to each other.
  //
  // Mapping on LLVM-IR level:
  // m_libs      <-> (%this)[0].08.0
  // m_parent    <-> (%this)[0].07.0
  bool checkMParentAndMLibs(void);

  // Check:
  // for any edge with one end in Ref, is the other end (OtherRef) in the RefV?
  bool checkExclusiveEdge(RegDDRef *Ref, SmallVectorImpl<RegDDRef *> &RefV,
                          DDGraph &DDG);

  // Do some preliminary checks;
  // Overall:
  // - OuterLp and InnerLp are both normalized;
  //
  // OuterLp:
  // -
  // -
  //
  // InnerLp:
  // -
  // -
  //
  bool doPreliminaryChecks(HLLoop *InnerLoop);

  // Pre-Collection Tests:
  bool doOuterLpTest(void);
  bool doInnerLpTest(void);

  // The Collector populates the following 4 unique items:
  // - OuterLpNodeVec;
  // - InnerLpNodeVec;
  // - HLIfVec;
  // - NonLocalRefVec;
  //
  // The collector also populates the OuterlpNodeVec into 3 containers and eases
  // follow-up processing:
  // - OuterLpInstVec
  // - OuterLpLabelVec
  // - OuterLpGotoVec
  //
  bool doCollection(void);

  // Post-Collection Tests:
  bool doHLIF0Test(void);
  bool doHLIF1Test(void);
  bool doDeepPatternTestOuterLp(void);
  bool doDeepPatternTestInnerLp(void);

  bool isCanonExprConstVal(CanonExpr *CE, int64_t ConstValue) {
    int64_t IntConst = 0;
    if (!CE || !CE->isIntConstant(&IntConst) || (IntConst != ConstValue)) {
      return false;
    }
    return true;
  }

  // Transform code (CodeGen) for the matched pattern:
  bool doTransform(HLLoop *OuterLp);

  // Inside OuterLp's body:
  // 1. Remove any HLLabel* or HLGoto* node(s);
  // 2. Remove any instruction that load/store local data;
  // 3. Cleanup an extra Load on a non-local MemRef array that has NO use
  //    (becomes dead load in step2);
  void cleanOuterLpBody(void);

  // *** DoUnrollActions ***
  // 1. Completely unroll the OuterLp;
  // 2. Rename each Temp's Definition:
  // 3. Store Sinking:
  //      Move the Last Store from each unrolled iteration to right BEFORE the
  //      LastStoreMarker;
  void doUnrollActions(void);

  // For each HLInst* in V: fix any IV inside the HLInst* to a given constant
  // (part of manual complete unroll)
  void fixLoopIvToConst(HLContainerTy &V, unsigned LoopLevel, unsigned IVConst);

  // For each HLInst* in V:
  // If the HInst defines a temp, collect the temp definition into DefVec
  // (part of temp-definition renaming)
  void collectTempDefition(HLContainerTy &V,
                           SmallVectorImpl<RegDDRef *> &DefVec);

  // Build mapping between OLD Definition and NEW Definition:
  // and save each mapping into DefMap.
  // (part of temp-definition renaming)
  void buildTempDefMap(SmallVectorImpl<RegDDRef *> &DefVec,
                       DenseMap<RegDDRef *, RegDDRef *> &DefMap);

  // For each HLInst* in V: update use of OLD temp to NEW temp
  // (part of temp-definition renaming)
  void updateTempUse(HLContainerTy &V, SmallVectorImpl<RegDDRef *> &DefVec,
                     DenseMap<RegDDRef *, RegDDRef *> &DefMap);

  void clearWorkingSetMemory(void);

  // *** Utility functions ***

  static bool isLocalMemRef(const RegDDRef *Ref) {
    return HIRPMSymbolicTripCountCompleteUnroll::isLocalOrNonLocalMemRef(Ref,
                                                                         true);
  }

  static bool isNonLocalMemRef(const RegDDRef *Ref) {
    return HIRPMSymbolicTripCountCompleteUnroll::isLocalOrNonLocalMemRef(Ref,
                                                                         false);
  }

  static bool isLocalOrNonLocalMemRef(const RegDDRef *Ref, bool IsLocal) {
    if (!Ref->isMemRef()) {
      return false;
    }
    bool AccessAlloca = Ref->accessesAlloca();
    return IsLocal ? AccessAlloca : !AccessAlloca;
  }

  // Does the given HLInst* access (load/store/copy/BinOp) local data?
  //(Local MemRef or temp)?
  static bool hasLocalLoadOrStore(HLInst *HInst);

  // Detect if the Ref has at least 1 Edge that the other end (ref) is not in
  // the given Loop.
  static bool hasEdgeInLoop(HLLoop *Lp, RegDDRef *Ref, DDGraph &DDG);

#ifndef NDEBUG
  void print(SmallVectorImpl<HLNode *> &V,
             bool PrintIndividualNewLine = false) {
    formatted_raw_ostream FOS(dbgs());
    FOS << "Vector of HLNode*: " << V.size() << "\n";

    for (auto Item : V) {
      Item->dump();
      if (PrintIndividualNewLine) {
        FOS << "\n";
      }
    }
  }

  void print(SmallVectorImpl<HLDDNode *> &V,
             bool PrintIndividualNewLine = false) {
    formatted_raw_ostream FOS(dbgs());
    FOS << "Vector of HLDDNode*: " << V.size() << "\n";

    for (auto Item : V) {
      Item->dump();
      if (PrintIndividualNewLine) {
        FOS << "\n";
      }
    }
  }

  void print(SmallVectorImpl<HLInst *> &V,
             bool PrintIndividualNewLine = false) {
    formatted_raw_ostream FOS(dbgs());
    FOS << "Vector of HLInst*: " << V.size() << "\n";

    for (auto Item : V) {
      Item->dump();
      if (PrintIndividualNewLine) {
        FOS << "\n";
      }
    }
  }

  void print(SmallVectorImpl<HLLabel *> &V,
             bool PrintIndividualNewLine = false) {
    formatted_raw_ostream FOS(dbgs());
    FOS << "Vector of HLLabel*: " << V.size() << "\n";

    for (auto Item : V) {
      Item->dump();
      if (PrintIndividualNewLine) {
        FOS << "\n";
      }
    }
  }

  void print(SmallVectorImpl<HLGoto *> &V,
             bool PrintIndividualNewLine = false) {
    formatted_raw_ostream FOS(dbgs());
    FOS << "Vector of HLGoto*: " << V.size() << "\n";

    for (auto Item : V) {
      Item->dump();
      if (PrintIndividualNewLine) {
        FOS << "\n";
      }
    }
  }

  void print(SmallVectorImpl<RegDDRef *> &V, bool PrintIndividualNewLine = true,
             bool PrintIndex = true) {
    formatted_raw_ostream FOS(dbgs());
    FOS << "Vector of RegDDRefs: " << V.size() << "\n";
    unsigned Idx = 0;

    for (auto Item : V) {
      // Print index
      if (PrintIndex) {
        FOS << Idx << ": ";
      }
      ++Idx;

      // Print Item
      Item->dump();

      if (PrintIndividualNewLine) {
        FOS << "\n";
      }
    }
  }

  // print every HLDDNode types from a given HLContainerTy
  void print(HLContainerTy &V, bool PrintIndex = true,
             bool PrintIndividualNewLine = false) {
    formatted_raw_ostream FOS(dbgs());
    FOS << "HLContainerTy: " << V.size() << "\n";
    unsigned Idx = 0;

    for (auto I = V.begin(), E = V.end(); I != E; ++I) {
      HLDDNode *Node = dyn_cast<HLDDNode>(I);
      // Skip anything that is not HLInst, HLIf, HLSwitch, and HLLoop.
      if (!Node) {
        continue;
      }

      // Print index
      if (PrintIndex) {
        FOS << Idx << ": ";
      }
      ++Idx;

      // Print Item:
      Node->dump();

      if (PrintIndividualNewLine) {
        FOS << "\n";
      }
    }
  }

  // DenseMap<RegDDRef *, RegDDRef *> DefMap
  void print(DenseMap<RegDDRef *, RegDDRef *> &M, bool PrintIndex = true,
             bool PrintIndividualNewLine = true) {
    formatted_raw_ostream FOS(dbgs());
    FOS << "MapSize: " << M.size() << "\n";
    unsigned Idx = 0;

    for (auto const &Item : M) {
      RegDDRef *First = Item.first;
      RegDDRef *Second = Item.second;

      // Print index
      if (PrintIndex) {
        FOS << Idx << ": ";
      }
      ++Idx;

      // Print Item:
      First->dump(), FOS << " -> ", Second->dump();

      if (PrintIndividualNewLine) {
        FOS << "\n";
      }
    }
  }

#endif
};
} // namespace unrollsymtc
#endif //INTEL_FEATURE_SW_ADVANCED

} // namespace loopopt
} // namespace llvm

#endif //LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UNROLLSYMTCIMPL_H
