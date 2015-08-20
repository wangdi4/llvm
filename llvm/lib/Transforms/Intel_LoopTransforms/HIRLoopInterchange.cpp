//===----- Interchange.cpp - Permutations of HIR loops ------*- C++ -*-----===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
// 2
// Performs loop interchange to achieve best legal permutations
// so the number of cache lines accessed is smallest.
// Input:  Temporal & spatial locality reuse per loop nest
//         DDG
//
// Steps:
// 1) Walk all loops, look for outer loops that are perfectly nested
// 2) compute Loop cost by adding pre-computed temporal and spatial reuse
// 3) if already in decreasing order (from outer to inner),  all done
// 4) exclude loops that has pragma  for unroll or unroll & jam
//    exclude triangular loop until later
// 5) Gather DV from DDG:
//    instead of calling Demand driven, filter out DV that implies INDEP
//    e.g. dv (> = *)  and our loop interchange candidate is from level 2 to 3,
//    then this edge can be ignored.
//    drop other edges that will not prevent loop interchange: e.g.
//    safe reduction, loop indepedent dep (t1=;  = t1)
//    anti dep for temps (< *)
// 6) Sum all DV based on nesting
// 7) Sort loop nests based on cost and get permutation P1.
//    If it's all legal to interchange, proceed to Gencode
// 8) Construct next permutation P2 that's legal
//    based on the permutation P1, from outermost to innermost,
//    choose loop L into P2 if legal. Discard L from P1 and repeat to add
//    more on P2
// 9) extract pre-hdr & post-exit of outermost loop
// 10) Gencode:  based on P1/P2,  update loop bounds/loop body
// 11) clear safe-reduction flag for minor cases like:
//       do i; do j; s = s + a(j) ->  do j; do i; s = s + a(j)
//===----------------------------------------------------------------------===//

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

#include "llvm/IR/Intel_LoopIR/HLSwitch.h"
#include "llvm/IR/Intel_LoopIR/HLLoop.h"
#include "llvm/IR/Intel_LoopIR/HLRegion.h"
#include "llvm/IR/Intel_LoopIR/HLIf.h"
#include "llvm/IR/Intel_LoopIR/HLInst.h"
#include "llvm/IR/Intel_LoopIR/RegDDRef.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRParser.h"
#include "llvm/Analysis/Intel_LoopAnalysis/DDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/DDGraph.h"
#include "llvm/Analysis/Intel_LoopAnalysis/DDTests.h"

#include "llvm/Transforms/Intel_LoopTransforms/Passes.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeVisitor.h"

#define DEBUG_TYPE "hir-loopinterchange"

using namespace llvm;
using namespace llvm::loopopt;

namespace {

class HIRLoopInterchange : public HIRTransformPass {
public:
  static char ID;

  HIRLoopInterchange() : HIRTransformPass(ID) {
    initializeHIRLoopInterchangePass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override;
  void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesAll();
    AU.addRequiredTransitive<HIRParser>();
    AU.addRequiredTransitive<DDAnalysis>();
  }

private:
  Function *F;
  SmallVector<HLLoop *, 16> CandidateLoops;
  std::vector<std::pair<unsigned, unsigned>> LoopCost;
  void TransformLoop(HLLoop *Loop);
  void ProcessLoop(HLLoop *Loop, DDAnalysis *DDA);
  // returns true means legal for any permutation
  bool LegalForInterchange(HLNode *Loop, DDAnalysis *DDA);
  void visit(HLNode *Node);
  void visit(HLRegion *Node);
  void visit(HLLoop *Node);
  void visit(HLIf *Node);
  void visit(HLSwitch *Node);
};
}

char HIRLoopInterchange::ID = 0;
INITIALIZE_PASS_BEGIN(HIRLoopInterchange, "HIRLoopInterchange",
                      "HIR Loop Interchange", false, false)
INITIALIZE_PASS_DEPENDENCY(HIRParser)
INITIALIZE_PASS_END(HIRLoopInterchange, "HIRLoopInterchange",
                    "HIR Loop Interchange", false, false)

FunctionPass *llvm::createHIRLoopInterchangePass() {
  return new HIRLoopInterchange();
}

bool HIRLoopInterchange::runOnFunction(Function &F) {
  DEBUG(dbgs() << "Loop Interchange for Function : " << F.getName() << "\n");

  this->F = &F;

  auto HIRP = &getAnalysis<HIRParser>();
  auto DDAP = &getAnalysis<DDAnalysis>();

  // 1) Walk all loops, look for outer loops that are perfectly nested

  for (auto I = HIRP->hir_begin(), E = HIRP->hir_end(); I != E; I++) {
    visit(I);
  }

  for (auto Iter = CandidateLoops.begin(), End = CandidateLoops.end();
       Iter != End; ++Iter) {
    HLLoop *Loop = *Iter;
    ProcessLoop(Loop, DDAP);
  }

  return false;
}

// This function will be deleted later
// The exact formula will be returned from some Util
static unsigned getLoopCost(HLLoop *Loop) {
  return Loop->getTemporalLocalityWt() + Loop->getSpatialLocalityWt();
}

void HIRLoopInterchange::ProcessLoop(HLLoop *Loop, DDAnalysis *DDA) {

  // 2) Compute Loop cost by adding pre-computed temporal and spatial reuse
  HLNode *Tmp = Loop;
  LoopCost.clear();
  unsigned idx = 0;
  while (Tmp) {
    HLLoop *Loop2 = dyn_cast<HLLoop>(Tmp);
    if (Loop2) {
      LoopCost.push_back(std::make_pair(getLoopCost(Loop2), ++idx));
      if (Loop2->isInnermost()) {
        break;
      }
      Tmp = Loop2->child_begin();
      continue;
    }
    llvm_unreachable("Perfect loop nests expected");
  }

  // 3) If already in decreasing order (from outer to inner) of loop cost,
  //    all done. If not, try to find best permutation that's legal

  unsigned LastSum = 0;
  bool InterchangeNeeded = false;
  for (size_t i = 0; i <= LoopCost.size(); ++i) {

    // Next line test only !!!
    LoopCost[i].first = 10.0 * i;

    unsigned Sum = LoopCost[i].first;

    if (LastSum == 0) {
      LastSum = Sum;
      continue;
    }
    if (Sum > LastSum) {
      InterchangeNeeded = true;
      break;
    }
    LastSum = Sum;
  }

  DEBUG(dbgs() << "\nInterchangeNeeded=" << InterchangeNeeded << "\n");

  if (!InterchangeNeeded) {
    return;
  }

  // When returning legal == true, means we can just interchange w/o
  // examining DV.
  // Otherwise, need to find best permutation
  if (!LegalForInterchange(cast<HLNode>(Loop), DDA)) {
  }
}

static bool IgnoreAntiEdge(DDEdge const *Edge, DVType *DV) {

  if (Edge->isANTIdep()) {
    DDRef *SrcDDRef = Edge->getSrc();
    RegDDRef *RegRef = cast<RegDDRef>(SrcDDRef);
    unsigned Direction = DV[0];
    if (RegRef->isScalarRef() && Direction == DV::LT) {
      raw_ostream &OS = dbgs();
      DEBUG(dbgs() << "\n\tANTI edge for Temp dropped");
      DEBUG(dbgs() << "\t"; Edge->print(OS));
      return true;
    }
  }
  return false;
}

static bool IgnoreEdge(DDEdge const *Edge, HLNode *CandidateLoop,
                       DVType *RefinedDV = nullptr) {

  //  1. Ignore all  (= = ..)
  //  2. for temps, ignore  anti (< ..)
  //     If there is a loop carried flow for scalars, the DV will not
  //     be all =
  //  3. Safe reduction (already done before calling here)

  DVType *DV = RefinedDV;

  if (DV == nullptr) {
    DV = const_cast<DVType *>(Edge->getDV());
  }

  if (isDValEQ(DV)) {
    DEBUG(dbgs() << "\n\tDV all =");
    return true;
  }

  HLLoop *Loop = cast<HLLoop>(CandidateLoop);

  if (isDVIndepFromLevel(DV, Loop->getNestingLevel())) {
    DEBUG(dbgs() << "\n\tIndep based on head of DV");
    return true;
  }

  if (Edge->isINPUTdep()) {
    DEBUG(dbgs() << "\n\tInput DEP edge");
    return true;
  }

  if (IgnoreAntiEdge(Edge, DV)) {
    DEBUG(dbgs() << "\n\tIgnore Anti edge");
    return true;
  }

  return false;
}

struct WalkHIR {

  raw_ostream &OS = dbgs();
  DDGraph DDG;
  HLNode *CandidateLoop;
  // Indicates if we need to call Demand Driven DD to refine DV
  bool RefineDV;
  DVectorTy ResultDV;
  // start, end level of Candidate Loop nest
  unsigned int StartLevel;
  unsigned int EndLevel;

  void visit(HLNode *Node) {

    HLInst *Inst = dyn_cast<HLInst>(Node);

    if (!Inst) {
      return;
    }
    if (Inst->isSafeRedn()) {
      return;
    }
    for (auto I = Inst->ddref_begin(), E = Inst->ddref_end(); I != E; ++I) {
      if ((*I)->isConstant()) {
        continue;
      }
      for (auto II = DDG.outgoing_edges_begin(*I),
                EE = DDG.outgoing_edges_end(*I);
           II != EE; ++II) {

        DDRef *DDref = II->getSink();

        if (!(HLNodeUtils::contains(CandidateLoop, DDref->getHLDDNode()))) {
          DEBUG(dbgs() << "\n\tSink DDRef not in loop");
          continue;
        }
        DDEdge const *edge = &(*II);
        if (IgnoreEdge(edge, CandidateLoop)) {
          DEBUG(dbgs() << "\n\t<Edge dropped>");
          DEBUG(dbgs() << "\t"; II->print(OS));
          continue;
        }

        const DVType *TempDV = II->getDV();
        HLLoop *Loop = cast<HLLoop>(CandidateLoop);

        const DVType *DV = II->getDV();
        DDtest DA;

        if (StartLevel == 0) {
          StartLevel = Loop->getNestingLevel();
          // get last level from  DD Util
          EndLevel = DA.lastLevelInDV(DV);
        }

        // Calling Demand Driven DD to refine DV
        if (RefineDV) {
          RegDDRef *RegDDref = dyn_cast<RegDDRef>(DDref);
          if (RegDDref && !(RegDDref->isScalarRef())) {

            DDRef *SrcDDRef = II->getSrc();
            DDRef *DstDDRef = DDref;
            DVectorTy InputDV;
            DVectorTy RefinedDV;

            DA.setInputDV(InputDV, StartLevel, EndLevel);
            DEBUG(dbgs() << "\nCalling Demand Driven DD");
            auto Result = DA.depends(SrcDDRef, DstDDRef, InputDV);

            if (Result == nullptr) {
              DEBUG(dbgs() << "\nIs INDEP from Demand Driven DD\n");
              continue;
            }

            for (unsigned I = 1; I <= Result->getLevels(); ++I) {
              RefinedDV[I - 1] = Result->getDirection(I);
            }

            DEBUG(Result->dump(OS));
            if (IgnoreEdge(edge, CandidateLoop, &RefinedDV[0])) {
              continue;
            }
            TempDV = &RefinedDV[0];
          }
        }

        for (unsigned I = StartLevel; I <= EndLevel; ++I) {
          ResultDV[I - 1] |= TempDV[I - 1];
        }

        DEBUG(dbgs() << "\n\t<Edge selected>");
        DEBUG(dbgs() << "\t"; II->print(OS));
      }

      for (auto BRefI = (*I)->blob_cbegin(), BRefE = (*I)->blob_cend();
           BRefI != BRefE; ++BRefI) {
        // TODO *BRefI
      }
    }
  }

  void postVisit(HLNode *) {}
  bool isDone() { return false; }
  bool skipRecursion(HLNode *Node) { return false; }
  WalkHIR(DDGraph DDG, HLNode *CandidateLoop, bool RefineDV)
      : DDG(DDG), CandidateLoop(CandidateLoop), RefineDV(RefineDV) {}
};

// returns true means legal for any permutation
bool HIRLoopInterchange::LegalForInterchange(HLNode *Loop, DDAnalysis *DDAP) {

  bool DVhasLT;
  bool DVhasGT;

  // Sorting in decending order
  // first hold the localityWt sum
  // second keeps track of the original index (starts from 1)

  std::sort(
      LoopCost.begin(), LoopCost.end(),
      [](std::pair<unsigned, unsigned> L1, std::pair<unsigned, unsigned> L2) {
        return L1.first > L2.first;
      });

  for (size_t I = 0; I < LoopCost.size(); ++I) {
    DEBUG(dbgs() << "\n wt, index " << LoopCost[I].first << " "
                 << LoopCost[I].second << "\n");
  }

  // TODO
  // 4) exclude loops that has pragma for unroll or unroll & jam
  //    exclude triangular loop until later

  // 5) Gather/sum DV from DDG. Filter out loop indep dep for temps,
  // safe reduction

  // We plan to avoid demand driven DD refining DV.
  // Will set last srgument of WalkHIR as false later after testing

  DEBUG(dbgs() << "\n\tCalling Rebuild\n");
  DDGraph DDG = DDAP->getGraph(Loop, false);
  DEBUG(dbgs() << "\n\tEnd calling Rebuild\n");

  WalkHIR WHIR(DDG, Loop, true);
  DDtest DA;
  DA.initDV(WHIR.ResultDV);
  WHIR.StartLevel = 0;
  WHIR.EndLevel = 0;

  HLNodeUtils::visit(WHIR, Loop, true, true, true);

  DEBUG(dbgs() << "\nStart, End level\n" << WHIR.StartLevel << " "
               << WHIR.EndLevel);

  DEBUG(dbgs() << "\nDV Accumulated: ";
        printDV(WHIR.ResultDV, WHIR.EndLevel, dbgs()));

  // If no edges are selected (Startlevel == 0)
  // then there is no dependence preventing interchange

  if (WHIR.StartLevel == 0) {
    return true;
  }

  DVhasLT = DVhasGT = false;

  for (unsigned II = WHIR.StartLevel; II <= WHIR.EndLevel; ++II) {
    if (!DVhasGT && (WHIR.ResultDV[II - 1] & DV::GT)) {
      DVhasGT = true;
    }
    if (!DVhasLT && (WHIR.ResultDV[II - 1] & DV::LT)) {
      DVhasLT = true;
    }
  }

  // Note: 2 stars will get both boolean flags set on

  if (!DVhasLT || !DVhasGT) {
    return true;
  }

  return false;
}

void HIRLoopInterchange::visit(HLNode *Node) {

  if (isa<HLRegion>(Node)) {
    HLRegion *Region = cast<HLRegion>(Node);
    visit(Region);
  } else if (isa<HLLoop>(Node)) {
    HLLoop *Loop = cast<HLLoop>(Node);
    visit(Loop);
  } else if (isa<HLIf>(Node)) {
    HLIf *IfNode = cast<HLIf>(Node);
    visit(IfNode);
  } else if (isa<HLSwitch>(Node)) {
    HLSwitch *SwitchNode = cast<HLSwitch>(Node);
    visit(SwitchNode);
  } else if (isa<HLInst>(Node) || isa<HLGoto>(Node) || isa<HLLabel>(Node)) {
    return;
  } else {
    llvm_unreachable("Unknown HLNode type!");
  }
}

void HIRLoopInterchange::visit(HLRegion *Region) {

  for (auto Iter = Region->child_begin(), End = Region->child_end();
       Iter != End; Iter++) {
    HLNode *Node = cast<HLNode>(Iter);
    visit(Node);
  }
}

void HIRLoopInterchange::visit(HLLoop *Loop) {

  // Gather perfect loop nests
  bool Found = false;
  HLLoop *Tmp = Loop;
  while (Tmp) {
    auto Child = Tmp->child_begin();
    HLLoop *Loop2 = dyn_cast<HLLoop>(Child);
    if (!Loop2) {
      break;
    }
    if (Loop2->isInnermost()) {
      Found = true;
      break;
    }
    Tmp = Loop2;
  }

  if (Found) {
    CandidateLoops.push_back(Loop);
    return;
  }

  for (auto ChildIter = Loop->child_begin(), ChildIterEnd = Loop->child_end();
       ChildIter != ChildIterEnd; ++ChildIter) {
    HLNode *Node = cast<HLNode>(ChildIter);
    visit(Node);
  }
}

void HIRLoopInterchange::visit(HLIf *IfNode) {

  for (auto ThenIter = IfNode->then_begin(), ThenIterEnd = IfNode->then_end();
       ThenIter != ThenIterEnd; ++ThenIter) {
    HLNode *Node = cast<HLNode>(ThenIter);
    visit(Node);
  }

  for (auto ElseIter = IfNode->else_begin(), ElseIterEnd = IfNode->else_end();
       ElseIter != ElseIterEnd; ++ElseIter) {
    HLNode *Node = cast<HLNode>(ElseIter);
    visit(Node);
  }
}

void HIRLoopInterchange::visit(HLSwitch *Switch) {
  llvm_unreachable("HLSwitch not implemented currently.");
}
