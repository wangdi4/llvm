//===-- VPOParoptLoopFuseCollapse.cpp - OpenMP Loop FuseAndCollapse Pass --===//
//
//   Copyright (C) 2023 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the implementation of Loop FuseAndCollapse optimization
///
/// \note
/// This pass only concerns OMP 'loop' constructs, so whenever an OMP loop
/// is mentioned below it means OMP generic loop.
///
/// The main goal of this optimization is to increase the scope for
/// ND-Range parallelization (specific ND-Range) aka collapsing. For example:
///
///   Before optimization:
/// \code
///       #pragma omp target
///       #pragma omp loop
///       for (int i = 0; i < M; ++i) {
///         #pragma omp loop
///         for (int j = 0; j < N; ++j)
///           Body1
///         #pragma omp loop
///         for (int j = 0; j < P; ++j)
///           Body2
///       }
/// \endcode
///    After optimization:
/// \code
///       #pragma omp target
///       #pragma omp loop collapse(2)
///       for (int i = 0; i < M; ++i) {
///         for (int j = 0; j < max(N, P); ++j) {
///           if (j < N)
///             Body1
///           if (j < P)
///             Body2
///         }
///       }
/// \endcode
///
/// Original collapsing was only applicable to canonical loop nests of non-OMP
/// loops only, except for a single outermost OMP loop. That approach was
/// missing two optimization opportunities:
///
/// 1. Parallelization of the nests containing OMP loops, e.g.
/// \code
///       #pragma omp target
///       #pragma omp loop
///       for (int i = 0; i < M; ++i)
///         #pragma omp loop
///         for (int j = 0; j < N; ++j)
/// \endcode
///
///    Using ND-Range parallelization for such nest may prove especially
///    beneficial when M is much lower than N.
/// 2. Loop nest is not in a canonical form. This particular pass aims for
///    a nests where the inner loops may be fused hence allowing the
///    subsequent parallelization, e.g.
/// \code
///       #pragma omp target
///       #pragma omp loop
///       for (int i = 0; i < M; ++i) {
///         #pragma omp loop
///         for (int j = 0; j < N; ++j)
///           ...
///         #pragma omp loop
///         for (int j = 0; j < N; ++j)
///           ...
///       }
/// \endcode
///
///
/// The pass consists of 4 main stages:
/// 1. Fusion candidates collection
/// 2. Checking the fusion and collapsing legality
/// 3. Loops fusion:
///    a. Hoisting the LB/UB out of the target region
///    b. Fusion of LLVM loops
///    c. Adjusting the original OMP loop directives
/// 4. Loop collapsing:
///    a. Merging the inner loop directives into the outer one
///    b. Hoisting the inner loops LB/UB and the fused UB out of the target
///    region
///    c. Setting the collapse clause
///
// TODOS:
// 1. Allow fusion of >2 loops
// 2. Incorporate legality checks into the candidates collection to be able
//    to pick other candidates after the checks fail
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/VPO/Paropt/Intel_VPOParoptLoopFuseCollapse.h"
#include "llvm/Analysis/OptimizationRemarkEmitter.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionInfo.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptTransform.h"
#include "llvm/Transforms/VPO/VPOPasses.h"

#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/DomTreeUpdater.h"
#include "llvm/Analysis/LoopNestAnalysis.h"

#include "llvm/Transforms/Utils/CodeMoverUtils.h"

#include "llvm/Analysis/Intel_OptReport/OptReportBuilder.h"

using namespace llvm;
using namespace llvm::vpo;

#define OPT_SWITCH "vpo-paropt-loop-fuse-collapse"
#define PASS_NAME "VPO Paropt Loop FuseAndCollapse Function Pass"
#define DEBUG_TYPE OPT_SWITCH

cl::opt<bool>
    EnableFuseAndCollapse(OPT_SWITCH, cl::Hidden, cl::init(false),
                          cl::desc("Enable paropt loop fuse&collapse pass"));

static bool fuseAndCollapse(Function &F, WRegionInfo &WI,
                            OptimizationRemarkEmitter &ORE) {
  bool Changed = false;

  WI.buildWRGraph();

  if (WI.WRGraphIsEmpty()) {
    LLVM_DEBUG(dbgs() << "\nNo WRegion Candidates for Transformation \n");
    return Changed;
  }

  LLVM_DEBUG(WI.print(dbgs()));
  LLVM_DEBUG(dbgs() << PASS_NAME << " for Function: ");
  LLVM_DEBUG(dbgs().write_escaped(F.getName()) << '\n');

  VPOParoptTransform VP(nullptr, &F, &WI, WI.getDomTree(), WI.getLoopInfo(),
                        WI.getSE(), WI.getTargetTransformInfo(),
                        WI.getAssumptionCache(), WI.getTargetLibraryInfo(),
                        WI.getAliasAnalysis(), FuseCollapse,
                        OptReportVerbosity::None, ORE, 2, false);

  Changed |= VP.paroptTransforms();

  return Changed;
}

PreservedAnalyses
VPOParoptLoopFuseCollapsePass::run(Function &F, FunctionAnalysisManager &AM) {

  if (!EnableFuseAndCollapse)
    return PreservedAnalyses::all();

  WRegionInfo &WI = AM.getResult<WRegionInfoAnalysis>(F);
  auto &ORE = AM.getResult<OptimizationRemarkEmitterAnalysis>(F);

  PreservedAnalyses PA;

  LLVM_DEBUG(dbgs() << "\n\n====== Enter " << PASS_NAME << " ======\n\n");
  if (!fuseAndCollapse(F, WI, ORE))
    PA = PreservedAnalyses::all();
  else
    PA = PreservedAnalyses::none();
  LLVM_DEBUG(dbgs() << "\n\n====== Exit  " << PASS_NAME << " ======\n\n");

  return PA;
}

namespace llvm {
namespace vpo {

class FusionCandidate {
  WRegionNode *W;
  Loop *L;

public:
  FusionCandidate(WRegionNode *W) : W(W), L(W->getWRNLoopInfo().getLoop()) {}

  Loop *getLoop() const { return L; }
  WRegionNode *getRegion() const { return W; }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump() const {
    dbgs() << "OMP fusion candidate:\n\t" << *W->getEntryDirective() << "\n";
  }
#endif
};

} // namespace vpo
} // namespace llvm

bool VPOParoptTransform::fuseAndCollapseOmpLoops(
    WRegionNode *W, const SmallPtrSet<WRegionNode *, 1> &HaveCollapse) {
  auto *WTarget = WRegionUtils::getParentRegion(W, WRegionNode::WRNTarget);

  if (!WTarget)
    return false;

  if (!isa<WRNGenericLoopNode>(W))
    return false;

  // if the enclosing loop's normalized UB wasn't firstprivatized by the
  // frontend, collapsing is not going to happen anyway
  if (std::any_of(W->getWRNLoopInfo().getNormUBs().begin(),
                  W->getWRNLoopInfo().getNormUBs().end(), [WTarget](Value *UB) {
                    return !WRegionUtils::wrnSeenAsFirstprivate(WTarget, UB);
                  })) {
    LLVM_DEBUG(dbgs() << __FUNCTION__
                      << ": Not all loop's UB are firstprivate, exiting");
    return false;
  }

  // Step 1. Fusion candidates collection

  bool ConsiderCurrent = true;
  if (HaveCollapse.count(W)) {
    // already being collapsed, check if that can be increased
    ConsiderCurrent = true;
  } else {
    // should not be included in other collapsed nest,
    // nor contain other collapsed loop
    ConsiderCurrent =
        !WRegionUtils::containsWRNsWith(W,
                                        [](WRegionNode *WChild) {
                                          return WChild->canHaveCollapse() &&
                                                 WChild->getCollapse() > 1;
                                        }) &&
        !WRegionUtils::hasAncestorWRNWith(W, [](WRegionNode *WAnc) {
          return WAnc->canHaveCollapse() && WAnc->getCollapse() > 1;
        });
    if (!ConsiderCurrent) {
      LLVM_DEBUG(dbgs() << "Intervening collapse clause found, exiting\n");
    }
  }
  if (!ConsiderCurrent)
    return false;

  bool HasCollapse = W->getCollapse() > 1;
  unsigned LookupLevel = (HasCollapse ? W->getCollapse() : 1);

  using CandidatePtrTy = std::unique_ptr<FusionCandidate>;
  using CandidatesVecTy = SmallVector<CandidatePtrTy, 2>;
  CandidatesVecTy Candidates;

  // OMP collapse clause implies existence of some non-OMP child loops,
  // and we don't want to interfere with that until we decide to
  // to support fusion of non-OMP loops too
  if (!HasCollapse) {
    std::queue<std::pair<WRegionNode *, /* loop level*/ unsigned>> Q;

    for (auto *WC : W->Children)
      Q.push({WC, 1});

    while (!Q.empty()) {
      auto [WC, L] = Q.front();
      Q.pop();
      // Currently only non-simd OMP loops are allowed, with no other constructs
      // in between
      bool IsValidLoop = WC->getIsOmpLoop() &&
                         WC->getWRegionKindID() != WRegionNode::WRNVecLoop;
      if (!IsValidLoop)
        break;
      if (L == LookupLevel) {
        Candidates.push_back(std::make_unique<FusionCandidate>(WC));
      } else if (L < LookupLevel) {
        for (auto *WCC : WC->Children)
          Q.push({WCC, L + 1});
      }
    }
  }

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Candidates:\n"; for (auto &FC
                                                               : Candidates) {
    dbgs() << "\t";
    FC->dump();
    dbgs() << "\n";
  } dbgs() << "\n";);

  if (Candidates.empty()) {
    LLVM_DEBUG(dbgs() << __FUNCTION__
                      << ": No suitable candidates found, exiting\n");
    return false;
  }

  if (Candidates.size() == 1) {
    LLVM_DEBUG(dbgs() << __FUNCTION__
                      << ": Only one candidate found but fuseless collapsing "
                         "disabled, exiting\n");
    return false;
  }

  if (Candidates.size() > 2) {
    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": >2  candidates found, exiting\n");
    return false;
  }

  return false;
}
