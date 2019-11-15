//===---- HIRSafeReductionAnalysis.cpp - Identify Safe Reduction Chain ----===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements Safe Reduction Identification
//
//===----------------------------------------------------------------------===//

#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/DDTests.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRSafeReductionAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Passes.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/BlobUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/CanonExprUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/ForEach.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeVisitor.h"

#include <algorithm>
#include <map>
#include <vector>

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "hir-safe-reduction-analysis"

AnalysisKey HIRSafeReductionAnalysisPass::Key;
HIRSafeReductionAnalysis
HIRSafeReductionAnalysisPass::run(Function &F, FunctionAnalysisManager &AM) {
  return HIRSafeReductionAnalysis(AM.getResult<HIRFrameworkAnalysis>(F),
                                  AM.getResult<HIRDDAnalysisPass>(F));
}

FunctionPass *llvm::createHIRSafeReductionAnalysisPass() {
  return new HIRSafeReductionAnalysisWrapperPass();
}

char HIRSafeReductionAnalysisWrapperPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRSafeReductionAnalysisWrapperPass,
                      "hir-safe-reduction-analysis",
                      "HIR Safe Reduction Analysis", false, true)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatisticsWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_END(HIRSafeReductionAnalysisWrapperPass,
                    "hir-safe-reduction-analysis",
                    "HIR Safe Reduction Analysis", false, true)

void HIRSafeReductionAnalysisWrapperPass::getAnalysisUsage(
    AnalysisUsage &AU) const {

  AU.setPreservesAll();
  AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
  // Loop Statistics is not used by this pass directly but it used by
  // HLNodeUtils::dominates() utility. This is a workaround to keep the pass
  // manager from freeing it.
  AU.addRequiredTransitive<HIRLoopStatisticsWrapperPass>();
  AU.addRequiredTransitive<HIRDDAnalysisWrapperPass>();
}

//  Sample code for calling Safe Reduction.
//
//  SRA = &getAnalysis<HIRSafeReductionAnalysis>();
//  -> Compute by passing outerloops or innermost loops
//  SRA->computeSafeReductionChains(outerloops);
//  a. In LoopInterchange:
//  if (SRA->isSafeReduction(Inst))
//
//  b. In Vectorizer:
//	if (SRA->isSafeReduction(Inst))   ....
//  or this to walk the chains
//  const SafeRedInfoList & SRCL = SRA->getSafeRedInfoList(Loop);
//	if (!SRCL.empty()) {
//		for (auto SRC : SRCL) {
//			for (auto Inst : SRC) {
//			 	Inst->print(OS, 2, false);
//  TODO: Compute SafeReduction chains for non-innermost loops
//
bool HIRSafeReductionAnalysisWrapperPass::runOnFunction(Function &F) {
  HSR.reset(new HIRSafeReductionAnalysis(
      getAnalysis<HIRFrameworkWrapperPass>().getHIR(),
      getAnalysis<HIRDDAnalysisWrapperPass>().getDDA()));
  return false;
}

void HIRSafeReductionAnalysisWrapperPass::releaseMemory() { HSR.reset(); }

HIRSafeReductionAnalysis::HIRSafeReductionAnalysis(HIRFramework &HIRF,
                                                   HIRDDAnalysis &DDA)
    : HIRAnalysis(HIRF), DDA(DDA) {}

namespace {
void printAChain(formatted_raw_ostream &OS, unsigned Indented,
                 const loopopt::SafeRedChain &SRC) {
  for (auto Inst : SRC) {
    Inst->print(OS, Indented, false);
  }
}
} // namespace

//  Identify Safe Reduction chain for a loop
//  "Safe" implies Reduction recurrence can be ignored for both
//  parallelization and vectorization.
//  Handles temps only. For memory reference, some preprocessing
//  for load hoisting or store sinking is needed.
//  It selects child stmts directly under the loop and skips
//  if stmts and stmts inside inner loops.
//  Invoked mostly for innermost loops, but supports any loop level.
//
//  Refer to HIRLoopInterchange.cpp for sample code to invoke and get
//  Safe Reductions

void HIRSafeReductionAnalysis::identifySafeReduction(const HLLoop *Loop) {

  // Safe reductions are of the follow forms:
  // a. Single statement
  //    t1 = t1 + ..
  // b. Reduction chain
  //
  //    t1 = t3 + ..
  //    t2 = t1 + .
  //    t3 = t2 + ..
  //    (In some cases,  we see copy stmt as the first stmt in
  //    the cycle,  t1 = t3)
  //
  //  Checking needed:
  //  - opcode:  max, min, add, sub, and, or, xor, mul, div
  //  - non-linear temps
  //  - flow edge (<) ; anti edge (=)
  //  - single use (single flow edge) in loop

  if (!Loop->isDo() || !(FirstChild = Loop->getFirstChild())) {
    return;
  }

  DDGraph DDG = DDA.getGraph(Loop);

  identifySafeReductionChain(Loop, DDG);
}

void HIRSafeReductionAnalysis::computeSafeReductionChains(const HLLoop *Loop) {

  SmallVector<const HLLoop *, 32> CandidateLoops;
  Loop->getHLNodeUtils().gatherInnermostLoops(CandidateLoops, Loop);
  for (auto &Lp : CandidateLoops) {
    auto SRCL = SafeReductionMap.find(Lp);
    if (SRCL != SafeReductionMap.end()) {
      continue;
    }
    identifySafeReduction(Lp);
  }
}

const SafeRedInfoList &
HIRSafeReductionAnalysis::getSafeRedInfoList(const HLLoop *Loop) {

  //  assert(Loop->isInnermost() && "SafeReduction supports only innermost
  //  loop");
  SafeRedInfoList &SRCL = SafeReductionMap[Loop];
  return SRCL;
}

namespace {
// This is for recognizing a safe reduction chain
// with mix of addition and subtractions.
// e.g. s = q - a[i]
//      q = s + b[i] ==> q = q + (b[i] - a[i]) is a safe reduction.
bool isValidMixOfOpcodes(unsigned OpCode1, unsigned OpCode2) {
  if (OpCode1 == OpCode2)
    return true;

  if (OpCode1 == Instruction::FAdd && OpCode2 == Instruction::FSub ||
      OpCode1 == Instruction::FSub && OpCode2 == Instruction::FAdd ||
      OpCode1 == Instruction::Add && OpCode2 == Instruction::Sub ||
      OpCode1 == Instruction::Sub && OpCode2 == Instruction::Add)
    return true;

  return false;
}
} // namespace
// Safe reduction chain could be
// a.  t1 = t2 +
//     t3 = t1 +
//     t2 = t3 +
// b.
//     t1 = t2
//     t3 = t1 +
//     t2 = t3 +
// c.
//     t1 = t1 +
//     t1 = t1 +
//     t1 = t1 +
bool HIRSafeReductionAnalysis::isValidSR(const RegDDRef *LRef,
                                         const HLLoop *Loop, HLInst **SinkInst,
                                         DDRef **SinkDDRef,
                                         unsigned ReductionOpCode,
                                         DDGraph DDG) {
  HLNode *UseNode = nullptr;
  HLInst *SingleOutputDepInst = nullptr;
  bool FlowEdgeFound = false;

  for (auto I = DDG.outgoing_edges_begin(LRef),
            E = DDG.outgoing_edges_end(LRef);
       I != E; ++I) {
    const DDEdge *Edge = *I;
    if (Edge->isOutput()) {
      // Allow only one output edge for case 'c' mentioned above in the
      // comments.
      if (SingleOutputDepInst) {
        return false;
      }

      // Capture the destination instruction of the single output edge
      SingleOutputDepInst = dyn_cast<HLInst>(Edge->getSink()->getHLDDNode());
      continue;
    }
    assert(Edge->isFlow() && "Outgoing edges from lval has to be either an "
                                "OUTPUT or a FLOW edge.");
    FlowEdgeFound = true;

    *SinkDDRef = Edge->getSink();
    HLNode *SinkNode = (*SinkDDRef)->getHLDDNode();
    HLNode *SinkParent = SinkNode->getParent();

    if (isa<HLLoop>(SinkParent)) {
      if (!HLNodeUtils::postDominates(SinkNode, FirstChild)) {
        return false;
      }
    } else {
      if (isa<HLSwitch>(SinkParent)) {
        return false;
      }
      HLNode *SrcNode = (Edge->getSrc())->getHLDDNode();
      HLNode *SrcParent = SrcNode->getParent();
      // Both Src and Sink are under same If
      if (SrcParent != SinkParent) {
        return false;
      }
      HLIf *If;
      if ((If = dyn_cast<HLIf>(SrcParent)) &&
          (If->isThenChild(SrcNode) != If->isThenChild(SinkNode))) {
        return false;
      }
    }

    *SinkInst = dyn_cast<HLInst>(SinkNode);
    if (!(*SinkInst)) {
      return false;
    }
    BlobDDRef *Bref = dyn_cast<BlobDDRef>(*SinkDDRef);
    if (Bref) {
      // Avoids
      // %t = %t1 + 1
      // %t1 = A[%t]
      RegDDRef *ParentRef = Bref->getParentDDRef();
      if (!ParentRef->isTerminalRef()) {
        return false;
      }
      // Integer sum can occur as blobs
      // sum =  10 * sum + ..
      CanonExpr *CE = ParentRef->getSingleCanonExpr();
      if (!isRedTemp(CE, Bref->getBlobIndex())) {
        return false;
      }
    }
    // Is Sink a copy stmt?
    if ((*SinkInst)->isCopyInst()) {
      continue;
    }
    unsigned ReductionOpCodeSave = ReductionOpCode;
    if (!(*SinkInst)->isReductionOp(&ReductionOpCode)) {
      return false;
    }
    if ((ReductionOpCode == Instruction::FSub ||
         ReductionOpCode == Instruction::Sub) &&
        (*SinkDDRef) == (*SinkInst)->getOperandDDRef(2)) {
      // S = .. - S, we bail out
      return false;
    }
    if (!isValidMixOfOpcodes(ReductionOpCode, ReductionOpCodeSave)) {
      return false;
    }
    if (Bref && ReductionOpCode != Instruction::Add) {
      // If BlobDDRef
      // Bail out t1 = (t1 + t2) * A[i];
      // OK if it were t1 = (t1 + t2) + A[i];
      // We intentionally skip Sub for now because
      // being t1 = (t1 + -1* t2) - A[i] or (t1 + t2) - A[i]
      // a safe reduction depends on the client's interpretation.
      return false;
    }
    bool IsMinMax = (ReductionOpCode == Instruction::Select);
    // In case of min/max reduction, make sure both uses belong to the same
    // 'select' operation
    if (IsMinMax) {
      if (!UseNode) {
        UseNode = SinkNode;
      } else {
        return (UseNode == SinkNode);
      }
    }
    if (!DDUtils::maxUsesInLoop(LRef, Loop, DDG, IsMinMax ? 2 : 1)) {
      return false;
    }
  }

  // The destination of that single output edge should be same as the flow edge
  // destination
  if (!FlowEdgeFound ||
      (SingleOutputDepInst && (*SinkInst != SingleOutputDepInst))) {
    return false;
  }

  return true;
}

//  Check for  valid temps
//  These are not safe reductions:
//  s = 2 * s  +  ..
//  s = n * s  +  ..
//  s = 2 * s * i  +  ..
bool HIRSafeReductionAnalysis::isRedTemp(CanonExpr *CE, unsigned BlobIndex) {
  return CE->containsStandAloneBlob(BlobIndex, false);
}

void HIRSafeReductionAnalysis::identifySafeReductionChain(const HLLoop *Loop,
                                                          DDGraph DDG) {

  LLVM_DEBUG(dbgs() << "\nIn Sum Reduction Chain\n");

  ForEach<const HLInst>::visitRange(
      Loop->child_begin(), Loop->child_end(),
      [this, Loop, DDG](const HLInst *Inst) {
        FirstRvalSB = 0;
        unsigned ReductionOpCode = 0;
        SafeRedChain RedInsts;
        bool SingleStmtReduction;
        const HLInst *FirstInstOfChain = nullptr;

        if (isa<HLSwitch>(Inst->getParent())) {
          return;
        }

        if (isa<HLLoop>(Inst->getParent()) &&
            !HLNodeUtils::postDominates(Inst, FirstChild)) {
          // By checking for PostDomination, it allows goto and label
          return;
        }

        // If already marked as part of some reduction chain, skip to the next
        if (isSafeReduction(Inst)) {
          return;
        }

        if (!findFirstRedStmt(Loop, Inst, &SingleStmtReduction, &FirstRvalSB,
                              &ReductionOpCode, DDG)) {
          return;
        }

        RedInsts.push_back(Inst);
        FirstInstOfChain = Inst;

        HLInst *SinkInst = nullptr;
        DDRef *SinkDDRef = nullptr;

        // Loop thru all flow edges to sink stmt
        //      t1 = t2 +
        //      t3 = t1 +
        //      t2 = t3 +
        //       - sink stmt postdom FirstChild
        //       - reduction Op matches
        //       - single use
        while (true) {

          const RegDDRef *LRef = Inst->getLvalDDRef();
          if (!isValidSR(LRef, Loop, &SinkInst, &SinkDDRef, ReductionOpCode,
                         DDG)) {
            break;
          }
          if (FirstRvalSB == SinkDDRef->getSymbase() &&
              SinkInst == FirstInstOfChain) {
            if (SingleStmtReduction) {
              LLVM_DEBUG(dbgs() << "\nSelf-reduction found\n");
            } else {
              LLVM_DEBUG(dbgs() << "\nSafe Reduction chain found\n");
            }
            setSafeRedChainList(RedInsts, Loop, FirstRvalSB, ReductionOpCode);
            LLVM_DEBUG(formatted_raw_ostream FOS(dbgs());
                       printAChain(FOS, 1, RedInsts));
            break;
          }

          // We only expect to go lexically forward starting from
          // FirstInstOfChain
          if (SinkInst->getTopSortNum() <= Inst->getTopSortNum()) {
            break;
          }

          RedInsts.push_back(SinkInst);
          Inst = SinkInst;
        }
      });
}

bool HIRSafeReductionAnalysis::findFirstRedStmt(
    const HLLoop *Loop, const HLInst *Inst, bool *SingleStmtReduction,
    unsigned *FirstRvalSB, unsigned *ReductionOpCode, DDGraph DDG) {

  // Start by guessing the start of reduction chain
  // S1  is Inst
  // Look for these 3 patterns:
  // (1)
  //  S1: t1 = t1 + ..
  // (2)
  //  S1: t1 = t3 +
  //      ..
  //  S3: t3 = t2 + a[i];
  // (3)
  //  S1: t1 = t3
  //   ..
  //  S3: t3 = t2 + a[i];
  //  Look for incoming flow edge (<) into S1.
  //  S3 needs to be a reduction stmt
  //
  //  The code below loops thru the RHS ddref, and checks if  there is only
  //  1 incoming edge from stmts below. So stmt like this (integer)
  //  s1 =  (n * 4 +  s0) + tx
  //  The ddref encountered is only tx.
  //  Later,  isValidSR will check if the reduction temp on LHS has
  //  only single use through DD edge

  unsigned ReductionOpCodeSave = 0;
  *SingleStmtReduction = false;

  if (!Inst->isCopyInst()) {
    if (!Inst->isReductionOp(ReductionOpCode)) {
      return false;
    }
    ReductionOpCodeSave = *ReductionOpCode;
  }

  unsigned OperandNum = 0;
  for (auto I = Inst->rval_op_ddref_begin(), E2 = Inst->rval_op_ddref_end();
       I != E2; ++I, ++OperandNum) {
    const RegDDRef *RRef = *I;

    if (!RRef->isTerminalRef()) {
      continue;
    }

    // sum = a[i] - sum   is not a reduction
    if ((OperandNum == 1) && (ReductionOpCodeSave == Instruction::FSub ||
                              ReductionOpCodeSave == Instruction::Sub)) {
      return false;
    }

    enum Answer { NO_REDUCTION, SKIPTONEXT, POTENTIAL_REDUCTION };
    auto Finder = [&](const DDRef *Ref) {
      for (auto I = DDG.incoming_edges_begin(Ref),
                E = DDG.incoming_edges_end(Ref);
           I != E; ++I) {
        if (!(*I)->isFlow()) {
          continue;
        }

        DDRef *DDRefSrc = (*I)->getSrc();
        HLInst *SrcInst = dyn_cast<HLInst>(DDRefSrc->getHLDDNode());
        assert(SrcInst && "Source of flow edge is not an instruction!");

        if (!SrcInst->isReductionOp(ReductionOpCode)) {
          return SKIPTONEXT;
        }

        // First stmt could be   a.  t1 = t2
        //          or           b.  t1 = t2 + ..

        if (!Inst->isCopyInst() &&
            !isValidMixOfOpcodes(ReductionOpCodeSave, *ReductionOpCode)) {
          return SKIPTONEXT;
        }

        if (Inst == SrcInst) {
          const RegDDRef *LRef = Inst->getLvalDDRef();
          if (DDUtils::maxUsesInLoop(LRef, Loop, DDG,
                                     Inst->isMinOrMax() ? 2 : 1)) {
            *SingleStmtReduction = true;
            *FirstRvalSB = DDRefSrc->getSymbase();
            return POTENTIAL_REDUCTION;
          } else {
            return NO_REDUCTION;
          }
        }

        // The caller has already checked that Inst post-dominates the first
        // child of the loop. So, SrcInst postDominating Inst implies that- a)
        // SrcInst also postdominates first child of the loop. b) This is a
        // cross-iteration dependency.
        if (!HLNodeUtils::postDominates(SrcInst, Inst)) {
          return SKIPTONEXT;
        }

        *FirstRvalSB = DDRefSrc->getSymbase();
        return POTENTIAL_REDUCTION;
      }
      return SKIPTONEXT;
    };

    auto Found = Finder(RRef);
    if (Found == POTENTIAL_REDUCTION) {
      return true;
    } else if (Found == NO_REDUCTION) {
      return false;
    }

    // Blob dd refs of rval dd refs scanned as well because
    // rval sinks of incoming edges can be a blob ddref.
    for (auto BI = RRef->blob_begin(), BE = RRef->blob_end(); BI != BE; ++BI) {
      auto Found = Finder(*BI);
      if (Found == POTENTIAL_REDUCTION) {
        return true;
      } else if (Found == NO_REDUCTION) {
        return false;
      }
    }
  }

  return false;
}

static bool anyUnsafeAlgebraInChain(SafeRedChain &RedInsts) {
  bool IsFP = RedInsts[0]->getLvalDDRef()->getDestType()->isFPOrFPVectorTy();
  if (!IsFP) {
    return false;
  }

  for (auto *Inst : RedInsts) {
    auto *FPInst = dyn_cast<FPMathOperator>(Inst->getLLVMInstruction());

    // FPInst can be NULL for a copy instruction.
    if (!FPInst) {
      continue;
    }

    // Return unsafe to vectorize if we are dealing with a Floating
    // point reduction and fast flag is off.
    if (!FPInst->isFast()) {
      LLVM_DEBUG(dbgs() << "\tis unsafe to vectorize/parallelize "
                           "(FP reduction with fast flag off)\n");
      return true;
    }
  }
  return false;
}

void HIRSafeReductionAnalysis::setSafeRedChainList(SafeRedChain &RedInsts,
                                                   const HLLoop *Loop,
                                                   unsigned RedSymbase,
                                                   unsigned RedOpCode) {

  SafeRedInfoList &SRCL = SafeReductionMap[Loop];
  SRCL.emplace_back(RedInsts, RedSymbase, RedOpCode,
                    anyUnsafeAlgebraInChain(RedInsts));
  unsigned SRIIndex = SRCL.size() - 1;

  // We should use []operator instead of insert() to overwrite the previous
  // entry for the instruction. SafeReductionMap and SafeReductionInstMap can
  // go out of sync due to deleted loops. Refer to comment in
  // getSafeRedInfo().
  for (auto &Inst : RedInsts) {
    SafeReductionInstMap[Inst] = SRIIndex;
  }
}

bool HIRSafeReductionAnalysis::isSafeReduction(const HLInst *Inst,
                                               bool *IsSingleStmt) const {

  const SafeRedInfo *SRI = getSafeRedInfo(Inst);
  if (!SRI) {
    return false;
  }

  if (IsSingleStmt) {
    *IsSingleStmt = (SRI->Chain.size() == 1 ? true : false);
  }

  return true;
}

void HIRSafeReductionAnalysis::printAnalysis(raw_ostream &OS) const {
  // Need a non-const pointer to force SRA for opt -analyze mode.
  auto NonConstSRA = const_cast<HIRSafeReductionAnalysis *>(this);
  formatted_raw_ostream FOS(OS);

  // Gather the innermost loops as candidates.
  SmallVector<const HLLoop *, 32> CandidateLoops;
  HIRF.getHLNodeUtils().gatherInnermostLoops(CandidateLoops);

  for (auto &Loop : CandidateLoops) {
    NonConstSRA->identifySafeReduction(Loop);
    unsigned Depth = Loop->getNestingLevel() + 1;

    Loop->printHeader(FOS, Depth - 1, false);

    auto &SRCL = NonConstSRA->SafeReductionMap[Loop];
    if (SRCL.empty()) {
      FOS << "No Safe Reduction\n";
    } else {
      for (auto &SRI : SRCL) {
        printAChain(FOS, Depth, SRI.Chain);
      }
    }

    Loop->printFooter(FOS, Depth - 1);
  }
}

void HIRSafeReductionAnalysis::print(formatted_raw_ostream &OS,
                                     const HLLoop *Loop,
                                     const SafeRedInfoList *SRCL) {
  unsigned Depth = Loop->getNestingLevel() + 1;

  if (SRCL->empty()) {
    Loop->indent(OS, Depth);
    OS << "No Safe Reduction\n";
    return;
  }

  for (auto &SRI : *SRCL) {
    Loop->indent(OS, Depth);
    printAChain(OS, Depth, SRI.Chain);
  }
}

void HIRSafeReductionAnalysis::print(formatted_raw_ostream &OS,
                                     const HLLoop *Loop) {

  auto &SRCL = SafeReductionMap[Loop];
  print(OS, Loop, &SRCL);
}

void HIRSafeReductionAnalysis::markLoopBodyModified(const HLLoop *Loop) {

  assert(Loop && " Loop parameter is null.");
  // No need to clean up info in parent loop
  auto Iter = SafeReductionMap.find(Loop);
  if (Iter != SafeReductionMap.end()) {
    for (auto &SRI : Iter->second) {
      for (auto Inst : SRI.Chain) {
        SafeReductionInstMap.erase(Inst);
      }
    }
    SafeReductionMap.erase(Loop);
  }
}

const SafeRedInfo *
HIRSafeReductionAnalysis::getSafeRedInfo(const HLInst *Inst) const {

  auto Iter = SafeReductionInstMap.find(Inst);
  if (Iter == SafeReductionInstMap.end()) {
    return nullptr;
  }

  // Get index of SafeRedInfo via Inst
  auto &SRIIndex = Iter->second;
  const HLLoop *Loop = Inst->getLexicalParentLoop();
  // Get SafeRedInfoList via Loop
  auto Iter2 = SafeReductionMap.find(Loop);

  assert(Iter2 != SafeReductionMap.end() &&
         "safe reduction analysis is in an inconsistent state!");

  auto &SRIL = Iter2->second;

  // Return SafeRedInfo via obtained Index and SRCL
  return &SRIL[SRIIndex];
}

bool HIRSafeReductionAnalysis::isReductionRef(const RegDDRef *Ref,
                                              unsigned &RedOpCode) {
  auto Node = Ref->getHLDDNode();

  assert(Node && "RegDDRef with null HLDDNode?");
  auto Inst = dyn_cast<HLInst>(Node);

  if (!Inst) {
    return false;
  }

  const SafeRedInfo *SRI = getSafeRedInfo(Inst);

  if (!SRI) {
    return false;
  }

  if (SRI->Symbase == Ref->getSymbase()) {
    RedOpCode = SRI->OpCode;
    return true;
  } else {
    return false;
  }
}
