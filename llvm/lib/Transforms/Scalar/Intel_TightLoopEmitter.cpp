#if INTEL_FEATURE_SW_ADVANCED
//===------- TightLoopEmitter.cpp - Prints Optimization reports -----------===//
//
// Copyright (C) 2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements Tight Loop emitter.
//
// This pass is for emitting diagnostic information upon finding tight loops.
// A tight loop can be defined as a cycle satisfying conditions below.
// - A cycle of instruction starting from a phi node and feeding the phi node.
// - The number of instructions in the cycle is small enough.
// - The cycle contains a fma on vector type data.
//
// Notice this pass is for helping the architectural explorations of a future
// core. It is a stand-alone no-op pass other than printing out useful
// information for a future core. The conditions above may change as needed in
// the future.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Scalar/Intel_TightLoopEmitter.h"

#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"

#define OPT_SWITCH "tight-loop-emitter"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;

namespace {
enum EmitterOption { SSCMark, Remark };

// Used for sanity check. In practice, tight loop is much shorter.
const int RelaxedCycleLenLimit = 128;
} // namespace

static cl::opt<EmitterOption> TightLoopEmitterControl(
    OPT_SWITCH "-run", cl::ReallyHidden,
    cl::desc("Run tight loop emitter pass"),
    cl::values(clEnumValN(SSCMark, "sscmark", "Do nothing."),
               clEnumValN(Remark, "remark", "Emit tight loop information.")));

static cl::opt<int> TightLoopSizeLimit(
    OPT_SWITCH "-cycle-len-limit", cl::ReallyHidden, cl::init(8),
    cl::desc("The longest length of cycle that is considered as a tight loop"));

static cl::opt<bool> RelaxTightness(
    OPT_SWITCH "-relax", cl::ReallyHidden, cl::init(false),
    cl::desc("Make more cycles reported by relaxing tightness conditions"));

namespace {
class TightLoopEmitter {
  void analysisLoopRecursive(const Loop *L) const;
  void analyzeInnermostLoop(const Loop *L) const;

  bool isValidChildToExplore(const Value *User, const Loop *L,
                             SmallPtrSetImpl<const Value *> &Visited,
                             int Depth) const {
    if (Visited.count(User))
      return false;
    if (!isa<Instruction>(User) || !L->contains(cast<Instruction>(User)))
      return false;
    if (Depth > CycleLenLimit)
      return false;

    return true;
  }

  static bool isFavorablePhi(const Value *Phi) {
    if (RelaxTightness)
      return true;

    if (auto *ValTy = dyn_cast<VectorType>(Phi->getType()))
      return ValTy->getElementType()->isFloatingPointTy();

    return false;
  }

  bool findACycle(const Value *TargetHeaderPhi, const Loop *L) const;

  void printCycle(const Loop *L,
                  const SmallVectorImpl<
                      std::pair<const Value *, Value::const_user_iterator>>
                      &Workitems) const;

public:
  TightLoopEmitter(OptReportBuilder &ORBuilder, LoopInfo &LI, int CycleLenLimit)
      : ORBuilder(ORBuilder), LI(LI), CycleLenLimit(CycleLenLimit) {}

  bool run(Function &F);

private:
  OptReportBuilder &ORBuilder;
  LoopInfo &LI;

  int CycleLenLimit;
};

// An iterative method finding a cycle starting from TargetHeaderPhi
// by tracing def-use chain. A list, Workitems, keeps track of ancestor nodes
// starting from TargetHeaderPhi to current Value* CurNode. When a cycle
// is found by noticing TargetHeaderPhi is reached again, Workitems contains
// all the nodes in the cycle in order. Visited is a set
// keeps track of all nodes that has been visited (i.e. reached).
// This function returns finding one cycle. It does not attempt to find
// SCCs (Strongly Connected Components) or multiple cylces in a SCC.
bool TightLoopEmitter::findACycle(const Value *TargetHeaderPhi,
                                  const Loop *L) const {

  SmallVector<std::pair<const Value *, Value::const_user_iterator>, 8>
      Workitems;
  Workitems.push_back(
      std::make_pair(TargetHeaderPhi, TargetHeaderPhi->user_begin()));

  SmallPtrSet<const Value *, 8> Visited;
  Visited.insert(TargetHeaderPhi);

  bool CycleFound = false;
  while (!CycleFound && !Workitems.empty()) {

    std::pair<const Value *, Value::const_user_iterator> &TopItem =
        Workitems.back();
    const Value *CurNode = TopItem.first;
    Value::const_user_iterator &NextUserI = TopItem.second;

    if (NextUserI == CurNode->user_end()) {
      Workitems.pop_back();
      continue;
    }

    const Value *ChildNode = *NextUserI;

    // Update current stack top before pushing a new item
    NextUserI = std::next(NextUserI);

    if (ChildNode == TargetHeaderPhi) {
      printCycle(L, Workitems);
      CycleFound = true;
      break;
    }

    if (!isValidChildToExplore(ChildNode, L, Visited, Workitems.size()))
      continue;

    Workitems.push_back(std::make_pair(ChildNode, ChildNode->user_begin()));
    Visited.insert(ChildNode);
  }

  return CycleFound;
}

static std::string getOptReportStr(const Value *V) {

  std::string Str;
  raw_string_ostream OS(Str);
  V->print(OS);

  return OS.str();
}

void TightLoopEmitter::printCycle(
    const Loop *L,
    const SmallVectorImpl<std::pair<const Value *, Value::const_user_iterator>>
        &Workitems) const {

  if (!RelaxTightness) {
    bool seenFMA = llvm::any_of(Workitems, [](auto const &Item) {
      if (auto *II = dyn_cast<IntrinsicInst>(Item.first)) {
        return II->getIntrinsicID() == Intrinsic::fma;
      }
      return false;
    });
    if (!seenFMA)
      return;
  }

  LLVM_DEBUG(dbgs() << "Tight cycle found for Loop: " << L->getName() << "\n");
  if (ORBuilder.getVerbosity() >= OptReportVerbosity::High)
    ORBuilder(*(const_cast<Loop *>(L)), LI)
        .addRemark(OptReportVerbosity::High, "Tight cycle found for Loop %s",
                   L->getName());

  for (auto const &I : Workitems) {
    LLVM_DEBUG(I.first->dump());
    if (ORBuilder.getVerbosity() >= OptReportVerbosity::High)
      ORBuilder(*(const_cast<Loop *>(L)), LI)
          .addRemark(OptReportVerbosity::High, "%s", getOptReportStr(I.first));
  }
}

// Top-level function of the innermost loop
void TightLoopEmitter::analyzeInnermostLoop(const Loop *L) const {

  const BasicBlock *Header = L->getHeader();

  for (auto const &I : *Header) {
    // PhiNodes are flocked at the beginning of the basic block
    if (!isa<PHINode>(&I))
      break;

    if (!isFavorablePhi(&I))
      continue;

    const Value *Phi = &I;

    findACycle(Phi, L);
  }
}

void TightLoopEmitter::analysisLoopRecursive(const Loop *L) const {

  if (L->isInnermost())
    analyzeInnermostLoop(L);

  for (const Loop *CL : L->getSubLoops())
    analysisLoopRecursive(CL);
}

bool TightLoopEmitter::run(Function &F) {

  if (TightLoopEmitterControl == EmitterOption::SSCMark)
    return false;

  LLVM_DEBUG(dbgs() << "Intel Tight Loop Emitter : " << F.getName() << "\n");

  // Traversal through all loops of the program in lexicographical order.
  // Due to the specifics of loop build algorithm, it is achieved via reverse
  // iteration.
  for (LoopInfo::reverse_iterator I = LI.rbegin(), E = LI.rend(); I != E; ++I)
    analysisLoopRecursive(*I);

  LLVM_DEBUG(
      dbgs()
      << "================================================================="
         "\n\n");

  return false;
}
} // namespace

PreservedAnalyses TightLoopEmitterPass::run(Function &F,
                                            FunctionAnalysisManager &AM) {
  // OptReport options
  auto &ORO = AM.getResult<OptReportOptionsAnalysis>(F);
  OptReportBuilder ORBuilder;
  ORBuilder.setup(F.getContext(), ORO.getVerbosity());

  TightLoopEmitter Emitter(ORBuilder, AM.getResult<LoopAnalysis>(F),
                           RelaxTightness ? RelaxedCycleLenLimit
                                          : TightLoopSizeLimit);
  Emitter.run(F);
  return PreservedAnalyses::all();
}

#endif // INTEL_FEATURE_SW_ADVANCED
