//===-- IntelVPlanNoCostInstructionAnalysis.cpp ----------------*- C++ -*--===//
//
//   Copyright (C) 2023 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#include "IntelVPlanNoCostInstructionAnalysis.h"
#include "IntelVPlan.h"
#include "IntelVPlanPatternMatch.h"

#define DEBUG_TYPE "VPlanNoCostInstructionAnalysis"

using namespace llvm;
using namespace llvm::vpo;
using namespace llvm::PatternMatch;

/// Analyze expression trees rooted at the condition of an '@llvm.assume', and
/// mark instructions in the tree as 'Always' no-cost if their only use is
/// within an assume call expression tree, e.g:
///
///   for (unsigned I = 0; I < 256; ++I) {
///     __builtin_assume(2 * (N + I) < 256)
///     __builtin_assume(2 * (N + I) + 1 < 255)
///     A[N + I] = I;
///     A[2 * (N + I)] = I;
///   }
///
///   ===>
///
///     ; __builtin__assume(N + I < 256)
///     %add1 = add i32 %N, %I           <-- Not zero-cost (used by %gep)
///     %mul  = mul i32 2, %add1         <-- Not zero-cost (used by %gep2)
///     %cmp1 = icmp slt i32 %add1, 256  <-- Zero-cost (used by 'assume')
///     call void @llvm.assume(i1 %cmp)  <-- Zero-cost ('assume' inst)
///
///     %add2 = add i32 %mul, 1          <-- Zero-cost (used by %cmp2)
///     %cmp2 = icmp slt i32 %add2, 255  <-- Zero-cost (used by 'assume')
///     call void @llvm.assume(i1 %cmp2) <-- Zero-cost ('assume' inst)
///
///     %gep = getelementptr ... %add1
///     %gep2 = getelementptr ... %mul
///
///  This is to account for the fact that these instructions are dead: they will
///  not be executed, and will be removed in CodeGenPrepare prior to the final
///  translation from LLVM-IR to MIR.
void VPlanNoCostInstAnalysis::analyzeAssumptions(const VPAssumptionCache *AC) {
  assert(AC && "Not computed?");

  SmallVector<const VPInstruction *, 16> Worklist;
  SmallPtrSet<const VPInstruction *, 16> Visited;

  const auto WasVisited = [&Visited](const VPUser *U) {
    return Visited.contains(dyn_cast<VPInstruction>(U));
  };
  const auto IsNoCost = [this](const VPUser *U) {
    return getScenario(dyn_cast<VPInstruction>(U)) == Scenario::Always;
  };

  // First, iterate over internal assumes, pushing each to the worklist.
  for (const auto &Assume : AC->assumptions()) {
    if (const auto *AssumeCall = dyn_cast<VPCallInstruction>(Assume)) {
      LLVM_DEBUG(dbgs() << "Push (root): " << *AssumeCall << '\n');
      Worklist.push_back(AssumeCall);
    }
  }

  // Starting with our assumptions as roots, work bottom-up, marking
  // instructions as no-cost if all of their users are no-cost. Traversal is
  // done breadth-first: each node is visited once all of its users have been
  // visited. Additionally, instructions which are not candidates are ignored
  // and prevent further traversal.
  while (!Worklist.empty()) {
    const auto *I = Worklist.pop_back_val();
    LLVM_DEBUG(dbgs() << "Pop: " << *I << '\n');

    // Skip already-visited instructions.
    if (!Visited.insert(I).second)
      continue;

    // When an instruction is visited the first time, all of its users should
    // have been processed. If all users are no-cost, we can safely determine
    // it to be no-cost as well.
    if (!all_of(I->users(), IsNoCost))
      continue;

    LLVM_DEBUG(dbgs() << "Definitely dead: " << *I << '\n');
    setScenario(I, Scenario::Always);

    for (const VPValue *Op : I->operands()) {
      const auto *I = dyn_cast<VPInstruction>(Op);
      if (!I || I->mayHaveSideEffects())
        continue;

      // Defer processing instructions whose users are not all yet visited.
      if (!all_of(I->users(), WasVisited)) {
        LLVM_DEBUG(dbgs() << "Skip: " << *I << " (not all users visited)\n");
        continue;
      }

      LLVM_DEBUG(dbgs() << "Push: " << *I << '\n');
      Worklist.push_back(I);
    }
  }
}

/// Analyze a masked-mode plan to identify loop normalization instructions.
/// A loop normalization instruction is of the form:
///
///     <add/sub> ..., <orig-lower-bound>
///
/// In a peel loop, this original lower bound will be '0', hence we mark
/// these instructions with the 'IfPeeling' no-cost scenario.
void VPlanNoCostInstAnalysis::analyzeMaskedModeNormalizationInstructions(
    const VPlanMasked &Plan) {
  LLVM_DEBUG(
      dbgs() << "Analyzing masked mode VPlan for normalization insts.\n");

  const auto *OrigLB = Plan.getMainLoop(true)->getOrigLowerBound();
  for (const auto &I : vpinstructions(&Plan)) {
    if (match(&I, m_Add(m_VPValue(), m_Specific(OrigLB))) ||
        match(&I, m_Sub(m_VPValue(), m_Specific(OrigLB)))) {
      LLVM_DEBUG(dbgs() << "No-cost if peeling: " << I << '\n');
      setScenario(&I, Scenario::IfPeeling);
    }
  }
}

void VPlanNoCostInstAnalysis::analyze(const VPlanVector &Plan) {
  analyzeAssumptions(Plan.getVPAC());
  if (const auto *MaskedPlan = dyn_cast<VPlanMasked>(&Plan))
    analyzeMaskedModeNormalizationInstructions(*MaskedPlan);
}
