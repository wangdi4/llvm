//===------ Intel_DopeVectorHoist.cpp - Hoisting Dope Vector Fields -*-----===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
// This pass performs special load hoisting of DopeVector base addresses.
// Currently, this pass hoists only base address loads of dope-vector arguments
// that are marked as noalias, nocapture, readonly, dereferenceable
// assumed_shape and ptrnoalias.
//
// Before:
//  define void @sub_(%"QNCA_0"* noalias nocapture readonly dereferenceable(96)
//                    "assumed_shape" "ptrnoalias" %A) {
//    BB0:
//     ...
//    BB1:    ; pred BB0
//     %"A_addr_a0" = getelementptr %"QNCA_0", %"QNCA_0"* %A, i64 0, i32 0
//     %"A_addr_a0_fetch" = load float*, float** %"A_addr_a0", align 1
//     %80 = getelementptr float, float* %"A_addr_a0_fetch", i64 %79
//     ...
//    BB2:     ; Pred BB0
//     %"A_addr_a1" = getelementptr %"QNCA_0", %"QNCA_0"* %A, i64 0, i32 0
//     %"A_addr_a1_fetch" = load float*, float** %"A_addr_a1", align 1
//     %53 = getelementptr inbounds float, float* %"A_addr_a1_fetch", i64 %52
//     ...
//   }
//
// After:
//  define void @sub_(%"QNCA_0"* noalias nocapture readonly dereferenceable(96)
//                    "assumed_shape" "ptrnoalias" %A) {
//    BB0:
//     %"A_addr_a_new" = getelementptr %"QNCA_0", %"QNCA_0"* %A, i64 0, i32 0
//     %"A_addr_a_new_fetch" = load float*, float** %"A_addr_a_new", align 1
//     ...
//    BB1:    ; pred BB0
//     %80 = getelementptr float, float* %"A_addr_a_new_fetch", i64 %79
//     ...
//    BB2:     ; Pred BB0
//     %53 = getelementptr float, float* %"A_addr_a_new_fetch", i64 %52
//     ...
//   }
//
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Scalar/Intel_DopeVectorHoist.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/Intel_Andersens.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/Loads.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/MemorySSA.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Verifier.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;

#define DEBUG_TYPE "dopevectorhoist"

// Option to enable DopeVector Hoist.
static cl::opt<bool>
    DopeVectorHoistEnable("dopevector-hoist-enable", cl::init(true), cl::Hidden,
                          cl::desc("Enable DopeVector Hoist."));

// Max limit: DopeVectorHoist is not triggered if number of new loads
// needed exceeds this limit.
static cl::opt<unsigned> DopeVectorHoistedLoadMaxLimit(
    "dopevector-hoist-max-load-limit", cl::init(25), cl::Hidden,
    cl::desc("Maximum number of new loads created to perform "
             "DopeVector Hoist."));

namespace {

// Main class to implement DopeVectorHoist.
class DopeVectorHoistImpl {

private:
  Function &F;
  DominatorTree &DT;
  TargetTransformInfo &TTI;

  // Set of candidate dope vector arguments.
  SmallPtrSet<Argument *, 16> UnmodifiedDopeVectorArgs;

  // Map of dope vector argument to set of all base address loads of the
  // dope vector argument.
  MapVector<Argument *, SmallPtrSet<LoadInst *, 32>> ArgLoadMap;

  bool collectUnmodifiedDopeVectorArgs();
  bool isGEPBaseAddrComputation(GetElementPtrInst *);
  bool collectDopeVectorBaseAddrLoads();
  void hoistDopeVectorBaseAddrLoads();

public:
  DopeVectorHoistImpl(Function &F, DominatorTree &DT, TargetTransformInfo &TTI)
      : F(F), DT(DT), TTI(TTI) {}

  bool run();
};

} // end of anonymous namespace

// Collect all candidate dope vector arguments that are not
// modified in the routine.
// Returns false if no arguments are found.
bool DopeVectorHoistImpl::collectUnmodifiedDopeVectorArgs() {
  if (F.arg_size() == 0)
    return false;
  for (auto &Arg : F.args())
    if (Arg.hasAttribute("ptrnoalias") && Arg.hasAttribute("assumed_shape") &&
        Arg.onlyReadsMemory() && Arg.hasNoAliasAttr() &&
        Arg.hasAttribute(Attribute::NoCapture))
      UnmodifiedDopeVectorArgs.insert(&Arg);

  return UnmodifiedDopeVectorArgs.size() != 0;
}

// Returns true if GEP is computing base address.
// Not allowing any other fields of dope vector intentionally
// except the base address field.
// Ex:
//  %"A_addr_a0" = getelementptr %"QNCA_0", %"QNCA_0"* %A, i64 0, i32 0
//
bool DopeVectorHoistImpl::isGEPBaseAddrComputation(GetElementPtrInst *GEP) {
  if (GEP->getNumIndices() != 2)
    return false;
  if (!GEP->hasAllZeroIndices())
    return false;
  return true;
}

// For each dope vector argument candidate, collect base address loads.
// Returns false if no base address loads are found.
bool DopeVectorHoistImpl::collectDopeVectorBaseAddrLoads() {
  for (auto &I : instructions(F)) {
    auto *LI = dyn_cast<LoadInst>(&I);
    if (!LI)
      continue;
    Value *V = LI->getPointerOperand();
    auto GEP = dyn_cast<GetElementPtrInst>(V);
    if (!GEP)
      continue;
    if (!isGEPBaseAddrComputation(GEP))
      continue;
    auto *A = dyn_cast<Argument>(GEP->getPointerOperand());
    // Makes sure "A" is unmodified DopeVector argument.
    if (!A || !UnmodifiedDopeVectorArgs.count(A))
      continue;
    const auto &DL = LI->getModule()->getDataLayout();
    if (!isDereferenceablePointer(V, LI->getType(), DL))
      continue;
    ArgLoadMap[A].insert(LI);
  }

  if (!ArgLoadMap.size()) {
    LLVM_DEBUG(dbgs() << " Skipped (No candidates)\n");
    return false;
  }

  LLVM_DEBUG({
    dbgs() << "All candidate loads:\n";
    for (auto &Cand : ArgLoadMap) {
      dbgs() << " Argument: " << *Cand.first << "\n";
      auto &LoadSet = Cand.second;
      if (LoadSet.size() <= 1) {
        dbgs() << "    Skip ... has single load\n";
        continue;
      }
      for (auto LI : LoadSet)
        dbgs() << "    " << *LI << "\n";
      dbgs() << "\n";
    }
  });

  return true;
}

// For each final dope vector argument candidate, find common dominator
// block for all base address loads, create new GEP and Load instructions
// in the block and replace all uses of loads instructions with the new
// load instruction.
void DopeVectorHoistImpl::hoistDopeVectorBaseAddrLoads() {
  for (auto &Cand : ArgLoadMap) {
    auto &LoadSet = Cand.second;
    if (LoadSet.size() <= 1)
      continue;
    // Find common dominator for all loads.
    BasicBlock *CommonDom = (*LoadSet.begin())->getParent();
    for (auto I = std::next(LoadSet.begin()), E = LoadSet.end(); I != E; I++) {
      if (!CommonDom)
        break;
      CommonDom = DT.findNearestCommonDominator(CommonDom, (*I)->getParent());
    }
    // Use first block as common dominator if we can't find common dominator
    // for some reason.
    if (!CommonDom)
      CommonDom = &F.getEntryBlock();
    auto IP = CommonDom->getFirstInsertionPt();

    LLVM_DEBUG(dbgs() << " Common Dominator for Arg: " << *Cand.first);
    LLVM_DEBUG(dbgs() << *CommonDom << "\n");

    GetElementPtrInst *NewGEP = nullptr;
    LoadInst *NewLI = nullptr;
    for (auto LI : LoadSet) {
      if (!NewGEP) {
        // Create new GEP and Load instructions.
        auto *V = LI->getPointerOperand();
        auto OldGEP = dyn_cast<GetElementPtrInst>(V);
        assert(OldGEP && "Expected GetElementPtrInst");
        NewGEP = cast<GetElementPtrInst>(OldGEP->clone());
        NewGEP->insertBefore(&*IP);
        NewLI = cast<LoadInst>(LI->clone());
        NewLI->replaceUsesOfWith(OldGEP, NewGEP);
        NewLI->insertAfter(NewGEP);
        LLVM_DEBUG(dbgs() << " NewGEP: " << *NewGEP << "\n");
        LLVM_DEBUG(dbgs() << " NewLI: " << *NewLI << "\n");
      }
      LLVM_DEBUG(dbgs() << " Replacing all uses of LI: " << *LI << "\n");
      LI->replaceAllUsesWith(NewLI);
    }
    LLVM_DEBUG(dbgs() << "\n");
  }
}

bool DopeVectorHoistImpl::run() {

  LLVM_DEBUG(dbgs() << "DopeVectorHoist for " << F.getName() << " ");

  if (!DopeVectorHoistEnable) {
    LLVM_DEBUG(dbgs() << " disabled.\n");
    return false;
  }
  // If AVX2 or higher is not present, then don't run optimization
  if (!TTI.isAdvancedOptEnabled(
          TargetTransformInfo::AdvancedOptLevel::AO_TargetHasIntelAVX2)) {
    LLVM_DEBUG(dbgs() << " suitable for target");
    return false;
  }

  if (!F.isFortran()) {
    LLVM_DEBUG(dbgs() << " Skipped (Not Fortran)\n");
    return false;
  }

  // Collect Dope Vector arguments that are candidates for
  // the transformation.
  if (!collectUnmodifiedDopeVectorArgs()) {
    LLVM_DEBUG(dbgs() << " Skipped (No Valid Args)\n");
    return false;
  }

  // Collect base address loads of candidate dope vectors arguments.
  if (!collectDopeVectorBaseAddrLoads())
    return false;

  // Check heuristics.
  unsigned NumCandidates = 0;
  for (auto &Cand : ArgLoadMap) {
    auto &LoadSet = Cand.second;
    // Ignore if Arg has only one load.
    if (LoadSet.size() <= 1)
      continue;
    NumCandidates++;
  }
  if (!NumCandidates) {
    LLVM_DEBUG(dbgs() << " Skipped (No Final candidates)\n");
    return false;
  }
  if (NumCandidates > DopeVectorHoistedLoadMaxLimit) {
    LLVM_DEBUG(dbgs() << " Skipped (Heuristics)\n");
    return false;
  }

  // Apply transformations.
  hoistDopeVectorBaseAddrLoads();
  return true;
}

PreservedAnalyses DopeVectorHoistPass::run(Function &F,
                                           FunctionAnalysisManager &AM) {
  auto &DT = AM.getResult<DominatorTreeAnalysis>(F);
  auto &TTI = AM.getResult<TargetIRAnalysis>(F);

  if (!DopeVectorHoistImpl(F, DT, TTI).run())
    return PreservedAnalyses::all();

  auto PA = PreservedAnalyses();
  PA.preserve<WholeProgramAnalysis>();
  PA.preserve<GlobalsAA>();
  PA.preserve<AndersensAA>();
  PA.preserve<DominatorTreeAnalysis>();
  PA.preserve<MemorySSAAnalysis>();
  PA.preserve<LoopAnalysis>();
  return PA;
}
