//===- LoopRotation.cpp - Loop Rotation Pass ------------------------------===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements Loop Rotation Pass.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Scalar/LoopRotation.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/InstructionSimplify.h"
#include "llvm/Analysis/Intel_Andersens.h"                  // INTEL
#include "llvm/Analysis/LazyBlockFrequencyInfo.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/MemorySSA.h"
#include "llvm/Analysis/MemorySSAUpdater.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/LoopRotationUtils.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
using namespace llvm;

#define DEBUG_TYPE "loop-rotate"

static cl::opt<unsigned> DefaultRotationThreshold(
    "rotation-max-header-size", cl::init(16), cl::Hidden,
    cl::desc("The default maximum header size for automatic loop rotation"));

static cl::opt<bool> PrepareForLTOOption(
    "rotation-prepare-for-lto", cl::init(false), cl::Hidden,
    cl::desc("Run loop-rotation in the prepare-for-lto stage. This option "
             "should be used for testing only."));

#if INTEL_CUSTOMIZATION
// Loops * total blocks, limit only for extreme cases.
static cl::opt<uint64_t>
    ComplexityLimit("loop-rotate-max-complexity", cl::init(1000000000),
                    cl::Hidden,
                    cl::desc("Stop rotating if loops*blocks exceeds this"));
#endif // INTEL_CUSTOMIZATION

LoopRotatePass::LoopRotatePass(bool EnableHeaderDuplication, bool PrepareForLTO)
    : EnableHeaderDuplication(EnableHeaderDuplication),
      PrepareForLTO(PrepareForLTO) {}

PreservedAnalyses LoopRotatePass::run(Loop &L, LoopAnalysisManager &AM,
                                      LoopStandardAnalysisResults &AR,
                                      LPMUpdater &) {
  // Vectorization requires loop-rotation. Use default threshold for loops the
  // user explicitly marked for vectorization, even when header duplication is
  // disabled.
  int Threshold = EnableHeaderDuplication ||
                          hasVectorizeTransformation(&L) == TM_ForcedByUser
                      ? DefaultRotationThreshold
                      : 0;
#if INTEL_CUSTOMIZATION
  // 23961: Disable rotation when loops * blocks is extremely large.
  if (L.getHeader())
    if ((uint64_t)AR.LI.getTopLevelLoops().size() *
            L.getHeader()->getParent()->size() >
        ComplexityLimit)
      return PreservedAnalyses::all();

  // Max header size is set by target, if the option is not specified, and
  // rotation has not been disabled.
  if (DefaultRotationThreshold.getNumOccurrences() == 0 && Threshold != 0)
    Threshold = AR.TTI.getLoopRotationDefaultThreshold(true);
#endif // INTEL_CUSTOMIZATION

  const DataLayout &DL = L.getHeader()->getModule()->getDataLayout();
  const SimplifyQuery SQ = getBestSimplifyQuery(AR, DL);

  Optional<MemorySSAUpdater> MSSAU;
  if (AR.MSSA)
    MSSAU = MemorySSAUpdater(AR.MSSA);
  bool Changed =
      LoopRotation(&L, &AR.LI, &AR.TTI, &AR.AC, &AR.DT, &AR.SE,
                   MSSAU.hasValue() ? MSSAU.getPointer() : nullptr, SQ, false,
                   Threshold, false, PrepareForLTO || PrepareForLTOOption);

  if (!Changed)
    return PreservedAnalyses::all();

  if (AR.MSSA && VerifyMemorySSA)
    AR.MSSA->verifyMemorySSA();

  auto PA = getLoopPassPreservedAnalyses();
  if (AR.MSSA)
    PA.preserve<MemorySSAAnalysis>();
  return PA;
}

namespace {

class LoopRotateLegacyPass : public LoopPass {
  unsigned MaxHeaderSize;
  bool PrepareForLTO;

public:
  static char ID; // Pass ID, replacement for typeid
  LoopRotateLegacyPass(int SpecifiedMaxHeaderSize = -1,
                       bool PrepareForLTO = false)
      : LoopPass(ID), PrepareForLTO(PrepareForLTO) {
    initializeLoopRotateLegacyPassPass(*PassRegistry::getPassRegistry());
    MaxHeaderSize = unsigned(SpecifiedMaxHeaderSize);  // INTEL
  }

  // LCSSA form makes instruction renaming easier.
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<AssumptionCacheTracker>();
    AU.addRequired<TargetTransformInfoWrapperPass>();
    AU.addPreserved<MemorySSAWrapperPass>();
    getLoopAnalysisUsage(AU);
    AU.addPreserved<AndersensAAWrapperPass>();  // INTEL

    // Lazy BFI and BPI are marked as preserved here so LoopRotate
    // can remain part of the same loop pass manager as LICM.
    AU.addPreserved<LazyBlockFrequencyInfoPass>();
    AU.addPreserved<LazyBranchProbabilityInfoPass>();
  }

  bool runOnLoop(Loop *L, LPPassManager &LPM) override {
    Function &F = *L->getHeader()->getParent();
    if (skipLoop(L))
      return false;

    auto *LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
    const auto *TTI = &getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F);
    auto *AC = &getAnalysis<AssumptionCacheTracker>().getAssumptionCache(F);
    auto &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();
    auto &SE = getAnalysis<ScalarEvolutionWrapperPass>().getSE();
    const SimplifyQuery SQ = getBestSimplifyQuery(*this, F);
    Optional<MemorySSAUpdater> MSSAU;
    // Not requiring MemorySSA and getting it only if available will split
    // the loop pass pipeline when LoopRotate is being run first.
    auto *MSSAA = getAnalysisIfAvailable<MemorySSAWrapperPass>();
    if (MSSAA)
      MSSAU = MemorySSAUpdater(&MSSAA->getMSSA());
#if INTEL_CUSTOMIZATION
    // Disable rotation when loops * blocks is extremely large.
    if (L->getHeader())
      if ((uint64_t)LI->getTopLevelLoops().size() *
              L->getHeader()->getParent()->size() >
          ComplexityLimit)
        return false;

    // Max header size is set by target, if the option is not specified.
    if (MaxHeaderSize == (unsigned)-1)
      MaxHeaderSize = DefaultRotationThreshold.getNumOccurrences() > 0 ?
          DefaultRotationThreshold :
          TTI->getLoopRotationDefaultThreshold(true);
#endif //INTEL_CUSTOMIZATION
    // Vectorization requires loop-rotation. Use default threshold for loops the
    // user explicitly marked for vectorization, even when header duplication is
    // disabled.
    int Threshold = hasVectorizeTransformation(L) == TM_ForcedByUser
                        ? DefaultRotationThreshold
                        : MaxHeaderSize;

    return LoopRotation(L, LI, TTI, AC, &DT, &SE,
                        MSSAU.hasValue() ? MSSAU.getPointer() : nullptr, SQ,
                        false, Threshold, false,
                        PrepareForLTO || PrepareForLTOOption);
  }
};
} // end namespace

char LoopRotateLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(LoopRotateLegacyPass, "loop-rotate", "Rotate Loops",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(AssumptionCacheTracker)
INITIALIZE_PASS_DEPENDENCY(LoopPass)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(MemorySSAWrapperPass)
INITIALIZE_PASS_END(LoopRotateLegacyPass, "loop-rotate", "Rotate Loops", false,
                    false)

Pass *llvm::createLoopRotatePass(int MaxHeaderSize, bool PrepareForLTO) {
  return new LoopRotateLegacyPass(MaxHeaderSize, PrepareForLTO);
}
