//===----------------------- HIRNontemporalMarking.cpp --------------------===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
// A pass for identifying loads and stores that could benefit from being
// nontemporal.
//
// The primary criterion we use is to identify loops that churn through a lot
// of cache, where the values being stored do not have any reuse within the loop
// itself. In addition, we should have some indication that the loop is likely
// to be bound by memory traffic. Since using nontemporal stores causes cache
// lines to be evicted in the cache, we should be conservative about applying
// this change.
//
// The initial motivating example for this transformation is the LBM benchmark
// in SPEC, where the hot kernel is roughly as follows:
// for (int iter = 0; iter < N; iter++) {
//   std::swap(src, dest);
//   // Loop is vectorizable after doing a AoS-to-SoA transformation.
//   for (int i = 0; i < 100 * 100 * 130; i++) {
//     dest[i][0:20] = pure_math(src[i][0:20]);
//   }
// }
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_LoopTransforms/HIRNontemporalMarking.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLocalityAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeVisitor.h"
#include "llvm/IR/IntrinsicsX86.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"

#define DEBUG_TYPE "hir-nontemporal-marking"

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool> DisablePass("disable-hir-nontemporal-marking",
                                 cl::init(false), cl::Hidden,
                                 cl::desc("Disable nontemporal marking pass"));

static cl::opt<uint64_t> CacheLineThreshold("hir-nontemporal-cacheline-count",
    cl::init(100000), cl::Hidden,
    cl::desc("Number of cache lines used before triggering nontemporal marking"));

namespace {

class HIRNontemporalMarkingLegacyPass : public HIRTransformPass {
public:
  static char ID;
  HIRNontemporalMarkingLegacyPass() : HIRTransformPass{ID} {
    initializeHIRNontemporalMarkingLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<HIRFrameworkWrapperPass>();
    AU.addRequired<HIRDDAnalysisWrapperPass>();
    AU.addRequired<HIRLoopLocalityWrapperPass>();
    AU.setPreservesAll();
  }
};

class HIRNontemporalMarking {
  HIRFramework &HIRF;
  HIRDDAnalysis &HDDA;
  HIRLoopLocality &HLL;

  bool markInnermostLoop(HLLoop *Loop);
public:
  HIRNontemporalMarking(HIRFramework &HIRF, HIRDDAnalysis &HDDA,
                        HIRLoopLocality &HLL)
    : HIRF(HIRF), HDDA(HDDA), HLL(HLL) {}
  bool run();
};

} // namespace

bool HIRNontemporalMarking::run() {
  if (DisablePass) {
    return false;
  }

  HLNodeUtils &HNU = HIRF.getHLNodeUtils();
  SmallVector<HLLoop *, 8> Loops;
  HNU.gatherInnermostLoops(Loops);

  bool Changed = false;
  for (auto Loop : Loops) {
    Changed |= markInnermostLoop(Loop);
  }
  return Changed;
}

bool HIRNontemporalMarking::markInnermostLoop(HLLoop *Loop) {
  // Loops must have some minimal amount of structure or else the next checks
  // will fire assertion failures.
  if (Loop->isUnknown())
    return false;

  // Check for sufficient memory traffic in this loop. If the total memory
  // traffic is going to blow out our caches, then it may be profitable to mark
  // memory accesses as nontemporal.
  if (HLL.getNumCacheLines(Loop) < CacheLineThreshold)
    return false;

  LLVM_DEBUG({
    formatted_raw_ostream os(dbgs());
    os << "Checking constraints on loop\n";
    Loop->print(os, 0, false);
    os << "Cache lines used: " << HLL.getNumCacheLines(Loop) << "\n";
  });

  // Collect all of the stores that exist at the top level of the loop. In
  // principle, we could convert the stores located within if statements as
  // well, but that would invalidate the option to handle unaligned nontemporal
  // stores--and without that transformation, we stand a good chance of making
  // the code *slower* instead of faster.
  DDGraph DepGraph = HDDA.getGraph(Loop);

  bool Converted = false;
  MDNode *NT_metadata = MDNode::get(HIRF.getContext(),
      ConstantAsMetadata::get(
        ConstantInt::get(Type::getInt32Ty(HIRF.getContext()), 1)));
  HLNode &LoopHead = *Loop->child_begin();
  for (HLNode &N : make_range(Loop->child_begin(), Loop->child_end())) {
    // Stop processing if we'll not be executed every iteration.
    if (!HLNodeUtils::postDominates(&N, &LoopHead))
      break;

    // Check if it's a store that's not marked nontemporal.
    HLInst *I = dyn_cast<HLInst>(&N);
    if (!I)
      continue;

    if (isa<StoreInst>(I->getLLVMInstruction())) {
      // Ignore anyone who has nontemporal markings already.
      if (I->getLvalDDRef()->getMetadata(LLVMContext::MD_nontemporal))
        continue;
    } else {
      continue;
    }

    // Regular store in the main body of the loop, check dependencies.
    LLVM_DEBUG({
      formatted_raw_ostream os(dbgs());
      I->print(os, 0);
    });
    RegDDRef *StoreAddr = I->getLvalDDRef();
    bool HasConflictingAccess = false;
    for (DDEdge *E : DepGraph.outgoing(StoreAddr)) {
      // Ignore self-dependences; these are preserved by the conversion to
      // nontemporal stores anyways.
      if (E->getSrc() == E->getSink())
        continue;

      if (E->getDV().isIndepFromLevel(Loop->getNestingLevel()))
        continue;

      LLVM_DEBUG(dbgs() << "Interference from edge ");
      LLVM_DEBUG(E->dump());
      HasConflictingAccess = true;
      break;
    }

    if (HasConflictingAccess)
      continue;

    // We're good to mark this as nontemporal.
    StoreAddr->setMetadata(LLVMContext::MD_nontemporal, NT_metadata);
    LLVM_DEBUG(dbgs() << "No interference, marked as nontemporal\n");
    Converted = true;
  }

  // If we converted anything, add an SFENCE after the loop.
  if (Converted) {
    HLNodeUtils &HNU = HIRF.getHLNodeUtils();
    FunctionCallee FenceIntrinsic(
      Intrinsic::getDeclaration(&HIRF.getModule(), Intrinsic::x86_sse_sfence));
    HLNode *Fence = HNU.createCall(FenceIntrinsic, {});
    HLNodeUtils::insertAsFirstPostexitNode(Loop, Fence);

    // Make sure that our changes get reflected when we go back to LLVM IR.
    Loop->getParentRegion()->setGenCode(true);
  }
  return Converted;
}

char HIRNontemporalMarkingLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRNontemporalMarkingLegacyPass, DEBUG_TYPE,
                      "HIR unaligned nontemporal marking pass", false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopLocalityWrapperPass)
INITIALIZE_PASS_END(HIRNontemporalMarkingLegacyPass, DEBUG_TYPE,
                    "HIR unaligned nontemporal marking pass", false, false)

FunctionPass *llvm::createHIRNontemporalMarkingPass() {
  return new HIRNontemporalMarkingLegacyPass{};
}

bool HIRNontemporalMarkingLegacyPass::runOnFunction(Function &F) {
  if (skipFunction(F)) {
    return false;
  }

  HIRNontemporalMarking NTM(getAnalysis<HIRFrameworkWrapperPass>().getHIR(),
                            getAnalysis<HIRDDAnalysisWrapperPass>().getDDA(),
                            getAnalysis<HIRLoopLocalityWrapperPass>().getHLL());
  return NTM.run();
}

PreservedAnalyses HIRNontemporalMarkingPass::run(Function &F,
    llvm::FunctionAnalysisManager &AM) {
  HIRNontemporalMarking NTM(AM.getResult<HIRFrameworkAnalysis>(F),
                            AM.getResult<HIRDDAnalysisPass>(F),
                            AM.getResult<HIRLoopLocalityAnalysis>(F));
  NTM.run();
  return PreservedAnalyses::all();
}
