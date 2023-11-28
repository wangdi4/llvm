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
#include "llvm/Analysis/VPO/Utils/VPOAnalysisUtils.h"
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

// This is a secondary threshold that lets us avoid marking known small loops
// such as vector peel/remainder loops even if CacheLineThreshold is set to
// zero. Its default value is set based on a dynamic threshold in
// __libirc_nontemporal_store, which avoids using nontemporal stores for any
// buffered store with a footprint less than four times the maximum architecture
// vector size in total. Picking the smaller of the common maximum vector
// sizes we work with, 256 bits, this is 4 * 32 B = 128 B.
static cl::opt<uint64_t> StoreFootprintThreshold(
    "hir-nontemporal-min-store-footprint", cl::init(128), cl::Hidden,
    cl::desc("Minimum possible store footprint (in bytes) for nontemporal "
             "marking to apply to a store"));

namespace {

class HIRNontemporalMarking {
  HIRFramework &HIRF;
  HIRDDAnalysis &HDDA;
  HIRLoopLocality &HLL;
  TargetTransformInfo &TTI;

  /// Whether this transform should only be run on vector-aligned stores, which
  /// can benefit from nontemporal marking even if not transformed later by the
  /// unaligned-nontemporal pass.
  bool OnlyVectorAligned;

  bool markInnermostLoop(HLLoop *Loop);
public:
  HIRNontemporalMarking(HIRFramework &HIRF, HIRDDAnalysis &HDDA,
                        HIRLoopLocality &HLL, TargetTransformInfo &TTI)
      : HIRF(HIRF), HDDA(HDDA), HLL(HLL), TTI(TTI), OnlyVectorAligned(false) {}
  bool run();
};

} // namespace

bool HIRNontemporalMarking::run() {
  if (DisablePass ||
      !(TTI.isAdvancedOptEnabled(
            TargetTransformInfo::AO_TargetHasIntelAVX512) ||
        TTI.isAdvancedOptEnabled(TargetTransformInfo::AO_TargetHasIntelAVX2))) {
    return false;
  }

  // If LibIRC is not enabled, the nontemporal library function is not available
  // but vector-aligned stores can still be marked.
  OnlyVectorAligned = !TTI.isLibIRCAllowed();
  LLVM_DEBUG({
    if (OnlyVectorAligned)
      dbgs() << "LibIRC not enabled; will only mark vector-aligned stores as "
                "nontemporal\n";
  });

  // CMPLRLLVM-21684: The nontemporal library function does not work correctly
  // on 32-bit code for reasons that are not yet understood. Disable this
  // transformation until it can be better understood.
  if (HIRF.getDataLayout().getPointerSizeInBits(0) != 64)
    return false;

  HLNodeUtils &HNU = HIRF.getHLNodeUtils();
  SmallVector<HLLoop *, 8> Loops;
  HNU.gatherInnermostLoops(Loops);

  bool Changed = false;
  for (auto Loop : Loops) {
    Changed |= markInnermostLoop(Loop);
  }
  return Changed;
}

/// Determines whether an edge in \p Edges represents a possible conflict within
/// loop level \p NestLevel.
template <typename DDEdgeRange>
static const DDEdge *hasConflictingAccess(DDEdgeRange &&Edges,
                                          unsigned NestLevel) {
  for (DDEdge *E : Edges) {
    // Ignore self-dependences; these are preserved by the conversion to
    // nontemporal stores anyways.
    if (E->getSrc() == E->getSink())
      continue;

    if (E->getDV().isIndepFromLevel(NestLevel))
      continue;

    return E;
  }

  // No conflicting edges found.
  return nullptr;
}

/// Determines if \p Loop has a statically known maximum trip count.
///
/// This can be a constant trip count, or it can be marked with either of our
/// max trip count metadata, llvm.loop.intel.loopcount_minimum or
/// llvm.loop.intel.max.trip_count.
static std::optional<uint64_t> getKnownMaxTripCount(const HLLoop *Loop) {
  uint64_t ConstTripCount;
  if (Loop->isConstTripLoop(&ConstTripCount))
    return ConstTripCount;
  if (const uint64_t LegalMaxTripCount = Loop->getLegalMaxTripCount())
    return LegalMaxTripCount;
  return std::nullopt;
}

bool HIRNontemporalMarking::markInnermostLoop(HLLoop *Loop) {
  // Loops must have some minimal amount of structure or else the next checks
  // will fire assertion failures. Additionally, we don't expect significant
  // benefit from optimizing multi-exit loops, and so those are also skipped
  // here.
  if (!Loop->isDo())
    return false;

  // Check for sufficient memory traffic in this loop. If the total memory
  // traffic is going to blow out our caches, then it may be profitable to mark
  // memory accesses as nontemporal.
  if (HLL.getNumCacheLines(Loop) < CacheLineThreshold)
    return false;

  // If this loop has a known max trip count, compute the minimum store size
  // needed to reach the store footprint threshold. Checking this threshold
  // through division avoids overflow issues we would have had if multiplying
  // instead, but we do need to check that the threshold is non-zero. The trip
  // count is non-zero by definition, because HIR does not consider the ZTT to
  // be part of the loop and without the ZTT the loop must execute at least one
  // iteration.
  //
  // The division-based check takes advantage of this equivalence:
  //   A*B < C => A < ceil(C/B)
  //
  // For example, if the threshold (C) is 40 B, when the trip count is 10 the
  // minimum store size to clear the threshold (A) is 4 B (ceil(40/10)), for a
  // total minimum footprint of 40 B. If the trip count is 9, the minimum store
  // size would be 5 B (ceil(40/9)), for a total minimum footprint of 45 B.
  //
  // This also uses a common pattern to implement the ceiling-division:
  //   ceil(A/B) => floor((A-1)/B) + 1
  //
  // ceil(A/B) = floor(A/B) + 1 for all cases when A is not divisible by B, but
  // ceil(A/B) = floor(A/B) when A is divisible by B.
  // floor((A-1)/B) = floor(A/B) when A is not divisible by B, but
  // floor((A-1)/B) = floor(A/B) - 1 if A is divisible by B. Taken together,
  // ceil(A/B) = floor((A-1)/B) + 1 whether or not A is divisible by B, which is
  // where this pattern comes from.
  const std::optional<uint64_t> KnownMaxTripCount = getKnownMaxTripCount(Loop);
  std::optional<uint64_t> MinStoreSize;
  if (KnownMaxTripCount && StoreFootprintThreshold != 0) {
    assert(*KnownMaxTripCount != 0);
    MinStoreSize = (StoreFootprintThreshold - 1) / (*KnownMaxTripCount) + 1;
  }

  LLVM_DEBUG({
    formatted_raw_ostream os(dbgs());
    os << "Checking constraints on loop\n";
    Loop->print(os, 0, false);
    os << "Cache lines used: " << HLL.getNumCacheLines(Loop) << "\n";
    if (MinStoreSize)
      os << "Minimum store size threshold: " << *MinStoreSize << " B = ceil("
         << StoreFootprintThreshold << " B / " << *KnownMaxTripCount << ")\n";
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
    // Only consider stores that postdominate the header, as only they have the
    // potential to be contiguous (needed for the library function to kick in
    // for unaligned stores).
    if (!HLNodeUtils::postDominates(&N, &LoopHead))
      continue;

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

    LLVM_DEBUG({
      formatted_raw_ostream os(dbgs());
      I->print(os, 0);
    });

    // Skip non-contiguous stores; they won't benefit from our later
    // optimizations.
    bool IsNegStride;
    RegDDRef *StoreRef = I->getLvalDDRef();
    if (!StoreRef->isUnitStride(Loop->getNestingLevel(), IsNegStride)) {
      LLVM_DEBUG(dbgs() << "Not contiguous\n");
      continue;
    }

    // Skip masked stores; they won't benefit from the later optimization
    // either.
    if (StoreRef->isMasked()) {
      LLVM_DEBUG(dbgs() << "Store is masked\n");
      continue;
    }

    // Also skip stores that aren't vector-aligned if the unaligned nontemporal
    // optimization is not available. Stores that are 8 bytes or smaller are
    // assumed to not benefit from nontemporal marking alone in this case.
    const uint64_t StoreSize = StoreRef->getDestTypeSizeInBytes();
    if (OnlyVectorAligned) {
      const unsigned Alignment = StoreRef->getAlignment();
      if (Alignment <= 8 || Alignment < StoreSize) {
        LLVM_DEBUG(dbgs() << "Not vector-aligned\n");
        continue;
      }
    }

    // Also skip stores that don't meet the minimum store size threshold for
    // nontemporal marking.
    if (MinStoreSize && StoreSize < *MinStoreSize) {
      LLVM_DEBUG(
          dbgs() << "Store size " << StoreSize
                 << " B is smaller than the minimum store size threshold ("
                 << MinStoreSize << " B)\n");
      continue;
    }

    // Regular store in the main body of the loop, check dependencies.
    if (const DDEdge *const Conflict = hasConflictingAccess(
          DepGraph.outgoing(StoreRef), Loop->getNestingLevel())) {
      LLVM_DEBUG(dbgs() << "Interference from edge ");
      LLVM_DEBUG(Conflict->dump());
      continue;
    }

    // Also check for incoming edges from possibly-aliased operations earlier in
    // the loop. While it is still legal to optimize in the presence of such
    // operations, we don't expect to see any benefits because these other
    // operations will still bring data into the cache and negate the benefits
    // of our nontemporal store.
    if (const DDEdge *const Conflict = hasConflictingAccess(
          DepGraph.incoming(StoreRef), Loop->getNestingLevel())) {
      LLVM_DEBUG(dbgs() << "Located temporal incoming edge ");
      LLVM_DEBUG(Conflict->dump());
      continue;
    }

    // We're good to mark this as nontemporal.
    StoreRef->setMetadata(LLVMContext::MD_nontemporal, NT_metadata);
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

PreservedAnalyses HIRNontemporalMarkingPass::runImpl(
    Function &F, llvm::FunctionAnalysisManager &AM, HIRFramework &HIRF) {
  HIRNontemporalMarking NTM(HIRF, AM.getResult<HIRDDAnalysisPass>(F),
                            AM.getResult<HIRLoopLocalityAnalysis>(F),
                            AM.getResult<TargetIRAnalysis>(F));
  ModifiedHIR = NTM.run();
  return PreservedAnalyses::all();
}
