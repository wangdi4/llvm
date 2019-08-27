//===-- VPlanPredicator.h ---------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file defines the VPlanPredicator class which contains the public
/// interfaces to predicate and linearize the VPlan region.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_VPLAN_PREDICATOR_H
#define LLVM_TRANSFORMS_VECTORIZE_VPLAN_PREDICATOR_H

#if INTEL_CUSTOMIZATION
#include "IntelLoopVectorizationPlanner.h"
#include "IntelVPlan.h"
#include "IntelVPlanBuilder.h"
#include "IntelVPlanDominatorTree.h"
#include "llvm/IR/Dominators.h"

using namespace llvm::vpo;
#define VPlanPredicator NewVPlanPredicator
#else
#include "LoopVectorizationPlanner.h"
#include "VPlan.h"
#include "VPlanDominatorTree.h"
#endif // INTEL_CUSTOMIZATION

namespace llvm {

class VPlanPredicator {
private:
  enum class EdgeType {
    TRUE_EDGE,
    FALSE_EDGE,
  };

  // VPlan being predicated.
  VPlan &Plan;

  // VPLoopInfo for Plan's HCFG.
  VPLoopInfo *VPLI;

  // Dominator tree for Plan's HCFG.
  VPDominatorTree VPDomTree;

  // VPlan builder used to generate VPInstructions for block predicates.
  VPBuilder Builder;

#if INTEL_CUSTOMIZATION
  /// Uniform inner loop regions that need to be fixed up using the loop
  /// preheader's block predicate
  SmallVector<VPLoopRegion *, 2> FixupLoopRegions;
  void fixupUniformInnerLoops(void);
#endif

  // Describe an edge/condition/predicate affecting given block predicate. Final
  // predicate for that block is an OR of all the PredicateTerms affecting the
  // block.
  struct PredicateTerm {
    // Its predicate will affect this term.
    VPBlockBase *OriginBlock;
    // Needed if currect block that has this PredicateTerm is affected by the
    // conditional branch terminator in the OriginBlock.
    VPValue *Condition;
    // If condition isn't nullptr and Negate is "true", the influence is coming
    // through the "False" edge coming from the OriginBlock. Must be false if
    // Condition is nullptr.
    bool Negate;

    PredicateTerm(VPBlockBase *OriginBlock, VPValue *Condition, bool Negate)
        : OriginBlock(OriginBlock), Condition(Condition), Negate(Negate) {
      assert((Condition || !Negate) && "Can't negate missing condition!");
    }

    PredicateTerm(VPBlockBase *OriginBlock)
        : PredicateTerm(OriginBlock, nullptr, false) {}

    PredicateTerm(const PredicateTerm &) = default;
  };
  using PredicateTermsSet = SmallVector<PredicateTerm, 4>;

  // Mapping from the block to its complete set of PredicateTerms affecting this
  // block's predicates.
  DenseMap<VPBlockBase *, PredicateTermsSet> Block2PredicateTerms;

  /// Create (not Cond) at the current Builder's insertion point.
  VPValue *createNot(VPValue *Cond);

  // Fill in the information about PredicateTerms of the predicate of the
  // \p CurrBlock.
  void calculatePredicateTerms(VPBlockBase *CurrBlock);

  /// If PredTerm.Condition is empty, just return PredTerm.OriginBlock's
  /// predicate. Otherwise, create Value represeting \p PredTerm, insert it at
  /// the current Builder's insertion point and return it. That value is
  ///
  ///   Val = OriginBlock.Predicate && (possibly negated)Condition.
  ///
  VPValue *createValueForPredicateTerm(PredicateTerm PredTerm);

  /// Generate and return the result of ORing all the predicate VPValues in \p
  /// Worklist. Uses the current insertion point of Builder member.
  VPValue *genPredicateTree(std::list<VPValue *> &Worklist);

  /// Predicate and linearize the CFG within \p Region, recursively.
  void predicateAndLinearizeRegionRec(VPRegionBlock *Region,
                                      bool SearchLoopHack);

  /// Linearize \p Region (without recursion) and mark PHIs in the linearized
  /// blocks as blended.
  void
  linearizeRegion(const ReversePostOrderTraversal<VPBlockBase *> &RegionRPOT);

#if INTEL_CUSTOMIZATION
  void handleInnerLoopBackedges(VPLoopRegion *LoopRegion);
#endif // INTEL_CUSTOMIZATION
public:
  VPlanPredicator(VPlan &Plan);

  /// Predicate Plan's HCFG.
  void predicate(void);
};
#if INTEL_CUSTOMIZATION
#undef VPlanPredicator
#endif // INTEL_CUSTOMIZATION
} // end namespace llvm
#endif // LLVM_TRANSFORMS_VECTORIZE_VPLAN_PREDICATOR_H
