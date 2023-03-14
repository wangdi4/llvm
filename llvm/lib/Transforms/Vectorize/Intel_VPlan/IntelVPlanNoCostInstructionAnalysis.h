//===-- IntelVPlanNoCostInstructionAnalysis.h ------------------*- C++ -*--===//
//
//   Copyright (C) 2023 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_NO_COST_INSTRUCTION_ANALYSIS_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_NO_COST_INSTRUCTION_ANALYSIS_H

#include "llvm/ADT/DenseMap.h"

#include <memory>

namespace llvm {
namespace vpo {
class VPAssumptionCache;
class VPInstruction;
class VPlanMasked;
class VPlanVector;

class VPlanNoCostInstAnalysis {
public:
  /// The scenarios in which an instruction can be considered to have no-cost.
  enum class Scenario {
    None,      /// Instruction has no special behavior (default value).
    Always,    /// Instruction should always be considered no-cost.
    IfPeeling, /// Instruction should be considered no-cost in peel loops.
  };

  VPlanNoCostInstAnalysis() = default;
  VPlanNoCostInstAnalysis(const VPlanNoCostInstAnalysis &) = default;
  ~VPlanNoCostInstAnalysis() = default;

  /// \Returns the scenario where the given instruction \p I is zero cost, or
  /// `Scenario::None` if none was set.
  Scenario getScenario(const VPInstruction *I) const {
    const auto It = Inst2Scenario.find(I);
    if (It == Inst2Scenario.end())
      return Scenario::None;
    return It->second;
  }

  /// Analyze the given \p Plan and assign zero-cost scenarios to instructions
  /// which should be handled specially in the cost model.
  void analyze(const VPlanVector &Plan);

private:
  /// Analyze expression trees rooted at the condition of an '@llvm.assume', and
  /// mark instructions in the tree as 'Always' no-cost if their only use is
  /// within an assume call expression tree.
  void analyzeAssumptions(const VPAssumptionCache *AC);

  /// Analyze a masked-mode plan to identify loop normalization instructions and
  /// mark them as no-cost 'IfPeeling'.
  void analyzeMaskedModeNormalizationInstructions(const VPlanMasked &Plan);

  /// Sets the zero-cost scenario \p S for the given instruction \p I.
  /// Asserts that \p S is not `SpecialCMBehavior::None`.
  void setScenario(const VPInstruction *I, Scenario S) {
    assert(S != Scenario::None && "`Scenario::None` shouldn't be explicitly "
                                  "set -- this is only a default value");
    assert(getScenario(I) == S || getScenario(I) == Scenario::None);
    Inst2Scenario.insert({I, S});
  }

  using ScenarioMap = llvm::DenseMap<const VPInstruction *, Scenario>;
  ScenarioMap Inst2Scenario;
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_NO_COST_INSTRUCTION_ANALYSIS_H
