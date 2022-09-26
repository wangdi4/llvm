#if INTEL_COLLAB
//===----------------------- VPOParoptGuardMemoryMotion -------------------===//
//
//   Copyright (C) 2022 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements a pass that identifies memory in loop work regions that
/// need to be guarded from code motion across the loop.
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/VPO/Paropt/VPOParoptGuardMemoryMotion.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptTransform.h"
#include "llvm/Transforms/VPO/VPOPasses.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vpo-paropt-guard-memory-motion"
#define PASS_NAME "VPO Paropt Guard Memory Motion"

static cl::opt<bool> DisablePass("disable-" DEBUG_TYPE, cl::init(false),
                                 cl::Hidden,
                                 cl::desc("Disable " PASS_NAME " pass"));

namespace {
class VPOParoptGuardMemoryMotion : public FunctionPass {
public:
  static char ID;

  VPOParoptGuardMemoryMotion() : FunctionPass(ID) {
    initializeVPOParoptGuardMemoryMotionPass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override { return PASS_NAME; }

  bool runOnFunction(Function &F) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<WRegionInfoWrapperPass>();
  }
};

// Use with the WRNVisitor class (in WRegionUtils.h) to walk the WRGraph
// (DFS) to gather all WRegion Nodes;
class VPOWRegionVisitor {
public:
  WRegionListTy &WRNList;

  VPOWRegionVisitor(WRegionListTy &WL) : WRNList(WL) {}
  void preVisit(WRegionNode *W) {}
  // Use DFS visiting of WRegionNodes.
  void postVisit(WRegionNode *W) { WRNList.push_back(W); }
  bool quitVisit(WRegionNode *W) { return false; }
};

// Main function to identify WRNs of interest and add guard directives to
// prohibit motion of memory outside the region. Variables that should not be
// moved are added as live-in operands to the directive. This guarding is
// currently done only for user-defined reduction variables specified in SIMD
// loops.
static bool guardMemoryMotion(Function &F, WRegionInfo &WI) {
  bool Changed = false;

  // Pass is disabled, nothing to do.
  if (DisablePass)
    return Changed;

  WI.buildWRGraph();
  // No WRNs to process.
  if (WI.WRGraphIsEmpty())
    return Changed;

  WRegionListTy WRegionList;
  VPOWRegionVisitor Visitor(WRegionList);
  WRegionUtils::forwardVisit(Visitor, WI.getWRGraph());

  for (auto *W : WRegionList) {
    auto *VecNode = dyn_cast<WRNVecLoopNode>(W);
    if (!VecNode || !VecNode->isOmpSIMDLoop())
      continue;

    Loop *Lp = VecNode->getTheLoop<Loop>();
    assert(Lp && "Loop associated with SIMD region not found.");
    CallInst *GuardDirective = nullptr;
    SmallVector<std::pair<StringRef, SmallVector<Value *, 1>>, 2> LiveinBundles;
    StringRef LiveInClauseString =
        VPOAnalysisUtils::getClauseString(QUAL_OMP_LIVEIN);

    for (ReductionItem *Item : VecNode->getRed().items()) {
      // Guarding is needed only for UDR variables.
      if (Item->getType() == ReductionItem::WRNReductionUdr) {
        if (!GuardDirective)
          GuardDirective = VPOUtils::getOrCreateLoopGuardForMemMotion(Lp);

        // Add the UDR variable as QUAL.OMP.LIVEIN operand to the directive.
        LiveinBundles.push_back({LiveInClauseString, {Item->getOrig()}});
        Changed = true;
      }
    }

    if (Changed)
      GuardDirective = VPOUtils::addOperandBundlesInCall(
          GuardDirective, makeArrayRef(LiveinBundles));
  }

  return Changed;
}
} // end anonymous namespace

char VPOParoptGuardMemoryMotion::ID = 0;
INITIALIZE_PASS_BEGIN(VPOParoptGuardMemoryMotion, DEBUG_TYPE, PASS_NAME, true,
                      false)
INITIALIZE_PASS_DEPENDENCY(WRegionInfoWrapperPass)
INITIALIZE_PASS_END(VPOParoptGuardMemoryMotion, DEBUG_TYPE, PASS_NAME, true,
                    false)

FunctionPass *llvm::createVPOParoptGuardMemoryMotionPass() {
  return new VPOParoptGuardMemoryMotion();
}

bool VPOParoptGuardMemoryMotion::runOnFunction(Function &F) {
  WRegionInfo &WI = getAnalysis<WRegionInfoWrapperPass>().getWRegionInfo();
  LLVM_DEBUG(dbgs() << "\n\n====== Enter " << PASS_NAME << " ======\n\n");
  bool Changed = guardMemoryMotion(F, WI);
  LLVM_DEBUG(dbgs() << "\n\n====== Exit  " << PASS_NAME << " ======\n\n");
  return Changed;
}

PreservedAnalyses
VPOParoptGuardMemoryMotionPass::run(Function &F, FunctionAnalysisManager &AM) {
  WRegionInfo &WI = AM.getResult<WRegionInfoAnalysis>(F);

  LLVM_DEBUG(dbgs() << "\n\n====== Enter " << PASS_NAME << " ======\n\n");
  if (!guardMemoryMotion(F, WI))
    return PreservedAnalyses::all();

  LLVM_DEBUG(dbgs() << "\n\n====== Exit  " << PASS_NAME << " ======\n\n");

  return PreservedAnalyses::none();
}

#endif // INTEL_COLLAB
