//===--------------------- Intel_VPOParoptApplyConfig ---------------------===//
//
//   Copyright (C) 2021 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements a pass that modifies/adds clauses requested
/// by the user via VPOParoptConfig analysis. Users pass a YAML file
/// asking for particular num_teams/thread_limit values for
/// some 'target' regions. This pass looks for the enclosed 'teams' regions
/// in corresponding 'target' regions and modifies/adds clauses to the
/// 'teams' regions. Since some other passes may change their behavior
/// based on the presence of the clauses, VPOParoptApplyConfig pass
/// must be run pretty early.
/// Changing the clauses in the IR helps readability, otherwise, we could
/// probably have applied the user-specified clauses during the regions parsing.
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/VPO/Paropt/Intel_VPOParoptApplyConfig.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionInfo.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptTransform.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptUtils.h"
#include "llvm/Transforms/VPO/VPOPasses.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vpo-paropt-apply-config"
#define PASS_NAME "VPO Paropt Apply Config"

static cl::opt<int64_t> InnermostLoopUnrollCount(
    "vpo-paropt-innermost-loop-unroll-count", cl::Hidden, cl::init(-1),
    cl::desc("Set the unroll value for all innermost loops."));

namespace {
class VPOParoptApplyConfig : public FunctionPass {
public:
  static char ID;

  VPOParoptApplyConfig() : FunctionPass(ID) {
    initializeVPOParoptApplyConfigPass(
        *PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override { return PASS_NAME; }

  bool runOnFunction(Function &F) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<WRegionInfoWrapperPass>();
#if 0
    // We could preserve WRegionInfo, but this requires
    // friending WRegionNode::setEntryDirective() with
    // the code in applyConfig.
    AU.addPreserved<WRegionInfoWrapperPass>();
#endif
    AU.addRequired<VPOParoptConfigWrapper>();
    AU.addPreserved<VPOParoptConfigWrapper>();
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

static int64_t
getKernelUnrollCount(WRegionNode *WLoop, const llvm::VPOParoptConfig *VPC,
                     const SmallVector<OffloadEntry *, 8> &OffloadEntries) {
  WRNTargetNode *WRegionParent = dyn_cast_if_present<WRNTargetNode>(
      WRegionUtils::getParentRegion(WLoop, WRegionNode::WRNTarget));
  if (!WRegionParent)
    return -1;
  OffloadEntry *OE = VPOParoptUtils::getTargetRegionOffloadEntry(
      WRegionParent, OffloadEntries);
  StringRef RegionName = OE->getName();
  int64_t UnrollCount = VPC->getKernelInnermostLoopUnrollCount(RegionName);
  LLVM_DEBUG(dbgs() << PASS_NAME
             ": config specifies InnermostLoopUnrollCount: '"
                    << UnrollCount << "'\n");
  return UnrollCount;
}

static bool applyInnerLoopUnrollCount(
    WRegionNode *WLoop, const llvm::VPOParoptConfig *VPC,
    const SmallVector<OffloadEntry *, 8> &OffloadEntries) {
  bool Changed = false;
  if (!WLoop->getChildren().empty())
    return Changed;
  auto &LoopInfo = WLoop->getWRNLoopInfo();
  auto *Loop = LoopInfo.getLoop();
  const char *UnrollMetadataName = "llvm.loop.unroll.count";
  SmallVector<llvm::Loop *, 4> Loops;
  SmallVector<llvm::Loop *, 4> InnermostLoops;
  int64_t KernelUnrollCount = getKernelUnrollCount(WLoop, VPC, OffloadEntries);
  assert(KernelUnrollCount >= -1 && "Invalid per-kernel unroll count");
  if (InnermostLoopUnrollCount < 0 && KernelUnrollCount < 0)
    return Changed;
  Loops.push_back(Loop);
  while (!Loops.empty()) {
    auto *Cur = Loops.pop_back_val();
    if (Cur->getSubLoops().empty()) {
      InnermostLoops.push_back(Cur);
      continue;
    }
    for (auto *Sub : Cur->getSubLoops())
      Loops.push_back(Sub);
  }
  for (auto *InnermostLoop : InnermostLoops) {
    std::optional<int> Factor =
        getOptionalIntLoopAttribute(InnermostLoop, UnrollMetadataName);
    bool UnrollDisabled =
        getBooleanLoopAttribute(InnermostLoop, "llvm.loop.unroll.disable");
    if (!Factor && !UnrollDisabled) {
      // If both the per-kernel and global unroll counts are set, prefer the
      // per-kernel count.
      int64_t UnrollCount =
          KernelUnrollCount >= 0 ? KernelUnrollCount : InnermostLoopUnrollCount;
      if (UnrollCount == 0)
        InnermostLoop->setLoopAlreadyUnrolled();
      else
        addStringMetadataToLoop(InnermostLoop, UnrollMetadataName, UnrollCount);
      LLVM_DEBUG(dbgs() << PASS_NAME ": Added unroll(" << UnrollCount
                        << ") metadata to loop: '" << InnermostLoop->getName()
                        << "'.\n");
      Changed = true;
    } else {
      LLVM_DEBUG(dbgs() << PASS_NAME
                 ": Innermost loop unroll specification was ignored for loop '"
                        << InnermostLoop->getName()
                        << "'. It has existing unroll metadata.\n");
      if (KernelUnrollCount >= 0) {
        llvm::report_fatal_error("Loop already has an unroll count but has a "
                                 "per-kernel unroll specification.",
                                 false /* no crash dialog */);
      }
    }
  }
  return Changed;
}

static bool applyConfig(Function &F, WRegionInfo &WI,
                        const llvm::VPOParoptConfig *VPC) {
  bool Changed = false;
  if (!VPC)
    return Changed;

  WI.buildWRGraph();
  if (WI.WRGraphIsEmpty())
    return Changed;

  SmallVector<OffloadEntry *, 8> OffloadEntries =
      VPOParoptUtils::loadOffloadMetadata(*F.getParent());

  if (OffloadEntries.empty())
    return Changed;

  // Collect a list of regions that can be looped-over in a depth-first manner.
  WRegionListTy WRegionList;
  VPOWRegionVisitor Visitor(WRegionList);
  WRegionUtils::forwardVisit(Visitor, WI.getWRGraph());

  for (auto *WT : WRegionList) {
    if (!isa<WRNTargetNode>(WT)) {
      if (WT->getIsOmpLoop())
        Changed = applyInnerLoopUnrollCount(WT, VPC, OffloadEntries);
      continue;
    }

    OffloadEntry *OE =
        VPOParoptUtils::getTargetRegionOffloadEntry(WT, OffloadEntries);
    StringRef RegionName = OE->getName();
    LLVM_DEBUG(dbgs() << PASS_NAME ": processing target region '" <<
               RegionName << "', index: " << WT->getOffloadEntryIdx() << "\n");

    uint64_t KernelThreadLimit = VPC->getKernelThreadLimit(RegionName);
    if (KernelThreadLimit != 0)
      LLVM_DEBUG(dbgs() << PASS_NAME ": config specifies ThreadLimit: '" <<
                 KernelThreadLimit << "'\n");
    uint64_t KernelNumTeams = VPC->getKernelNumTeams(RegionName);
    if (KernelNumTeams != 0)
      LLVM_DEBUG(dbgs() << PASS_NAME ": config specifies NumTeams: '" <<
                 KernelNumTeams << "'\n");

    // TODO: we should probably handle getKernelSPMDSIMDWidth() here as well
    //       by adding some special clause defining the kernel SIMD width.
    //       Right now it is handled in
    //       VPOParoptTransform::finalizeKernelFunction().
    if (KernelThreadLimit == 0 && KernelNumTeams == 0)
      continue;

    if (KernelThreadLimit > std::numeric_limits<int32_t>::max() ||
        KernelNumTeams > std::numeric_limits<int32_t>::max()) {
      LLVM_DEBUG(dbgs() << PASS_NAME ": config specifies thread_limit or/and "
                 "num_teams that do not fit int32_t - ignore them.\n");
      continue;
    }

    // See if we can find the child 'teams' region. It should be the only
    // child region, if it exists.
    WRegionNode *W = WT->getFirstChild();
    if (!W) {
      LLVM_DEBUG(dbgs() << PASS_NAME ": there is no child teams region.\n");
      continue;
    }
    if (!isa<WRNTeamsNode>(W)) {
      LLVM_DEBUG(dbgs() <<
                 PASS_NAME ": target's child region is not a teams region.\n");
      continue;
    }

    Changed = true;

    // Modify or add THREAD_LIMIT/NUM_TEAMS clauses for the 'teams' region.
    SmallVector<int, 2> ClausesToRemove;
    if (W->getThreadLimit() && KernelThreadLimit) {
      ClausesToRemove.push_back(QUAL_OMP_THREAD_LIMIT);
      LLVM_DEBUG(dbgs() <<
                 PASS_NAME ": config overrides user-specified thread_limit: " <<
                 *W->getThreadLimit() << "\n");
    }

    if (W->getNumTeams() && KernelNumTeams) {
      ClausesToRemove.push_back(QUAL_OMP_NUM_TEAMS);
      LLVM_DEBUG(dbgs() <<
                 PASS_NAME ": config overrides user-specified num_teams: " <<
                 *W->getNumTeams() << "\n");
    }

    CallInst *CI = cast<CallInst>(W->getEntryDirective());
    if (!ClausesToRemove.empty())
      CI = VPOUtils::removeOpenMPClausesFromCall(CI, ClausesToRemove);

    IRBuilder<> Builder(CI);
    SmallVector<std::pair<StringRef, ArrayRef<Value *>>, 2> NewClauses;

    // Add new TYPED THREAD_LIMIT or/and NUM_TEAMS clauses.
    // Note that these clauses will affect both target and host
    // code generation. We used to affect only the target part before,
    // but it does not seem to be safe. Mismatching clauses may cause issues.
    std::string NewThreadLimitString =
        VPOAnalysisUtils::getClauseString(QUAL_OMP_THREAD_LIMIT).str() +
        ":TYPED";
    SmallVector<Value *, 2> ThreadLimitArgs;
    if (KernelThreadLimit != 0) {
      ThreadLimitArgs.push_back(Builder.getInt32(KernelThreadLimit));
      ThreadLimitArgs.push_back(Builder.getInt32(0));
      NewClauses.emplace_back(NewThreadLimitString, ThreadLimitArgs);
    }

    std::string NewNumTeamsString =
        VPOAnalysisUtils::getClauseString(QUAL_OMP_NUM_TEAMS).str() +
        ":TYPED";
    SmallVector<Value *, 2> NumTeamsArgs;
    if (KernelNumTeams != 0) {
      NumTeamsArgs.push_back(Builder.getInt32(KernelNumTeams));
      NumTeamsArgs.push_back(Builder.getInt32(0));
      NewClauses.emplace_back(NewNumTeamsString, NumTeamsArgs);
    }
    CI = VPOUtils::addOperandBundlesInCall(CI, NewClauses);
#if 0
    // We could preserve WRegionInfo, but this requires
    // friending WRegionNode::setEntryDirective() with
    // the code in applyConfig.
    // In addition, the parsed THREAD_LIMIT/NUM_TEAMS values
    // need to be updated for the affected WRegionNode's.
    W->setEntryDirective(CI);
#endif
  }

  for (OffloadEntry *OE : OffloadEntries)
    delete OE;
  OffloadEntries.clear();

  return Changed;
}
} // end anonymous namespace

char VPOParoptApplyConfig::ID = 0;
// CFG is kept untouched.
INITIALIZE_PASS_BEGIN(VPOParoptApplyConfig, DEBUG_TYPE, PASS_NAME, true, false)
INITIALIZE_PASS_DEPENDENCY(WRegionInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(VPOParoptConfigWrapper)
INITIALIZE_PASS_END(VPOParoptApplyConfig, DEBUG_TYPE, PASS_NAME, true, false)

FunctionPass *llvm::createVPOParoptApplyConfigPass() {
  return new VPOParoptApplyConfig();
}

bool VPOParoptApplyConfig::runOnFunction(Function &F) {
  WRegionInfo &WI = getAnalysis<WRegionInfoWrapperPass>().getWRegionInfo();

  LLVM_DEBUG(dbgs() << "\n\n====== Enter " << PASS_NAME << " ======\n\n");
  auto &ParoptConfig = getAnalysis<VPOParoptConfigWrapper>().getResult();
  bool Changed = applyConfig(F, WI, &ParoptConfig);
  LLVM_DEBUG(dbgs() << "\n\n====== Exit  " << PASS_NAME << " ======\n\n");
  return Changed;
}

PreservedAnalyses VPOParoptApplyConfigPass::run(
    Function &F, FunctionAnalysisManager &AM) {
  WRegionInfo &WI = AM.getResult<WRegionInfoAnalysis>(F);

  PreservedAnalyses PA;

  LLVM_DEBUG(dbgs() << "\n\n====== Enter " << PASS_NAME << " ======\n\n");
  auto &MAMProxy = AM.getResult<ModuleAnalysisManagerFunctionProxy>(F);
  auto *ParoptConfig =
      MAMProxy.getCachedResult<VPOParoptConfigAnalysis>(*F.getParent());
  (void)applyConfig(F, WI, ParoptConfig);
#if 0
  // We could preserve WRegionInfo, but this requires
  // friending WRegionNode::setEntryDirective() with
  // the code in applyConfig.
  PA.preserve<WRegionInfoAnalysis>();
#endif
  PA.preserve<VPOParoptConfigAnalysis>();
  PA.preserveSet<CFGAnalyses>();
  LLVM_DEBUG(dbgs() << "\n\n====== Exit  " << PASS_NAME << " ======\n\n");

  return PA;
}
