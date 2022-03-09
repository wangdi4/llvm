//===--------------------- Intel_VPOParoptApplyConfig ---------------------===//
//
//   Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
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
#include "llvm/Transforms/VPO/Paropt/VPOParoptTransform.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptUtils.h"
#include "llvm/Transforms/VPO/VPOPasses.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vpo-paropt-apply-config"
#define PASS_NAME "VPO Paropt Apply Config"

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

  for (auto I = WI.begin(), IE = WI.end(); I != IE; ++I) {
    WRegionNode *WT = *I;
    if (!isa<WRNTargetNode>(WT))
      continue;

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
