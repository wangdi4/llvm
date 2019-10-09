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

  // Dominator/PostDominator tree for Plan's HCFG.
  VPDominatorTree VPDomTree;
  VPPostDominatorTree VPPostDomTree;

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

    // To be used as a key in std::map.
    bool operator<(const PredicateTerm &Other) const {
      return std::tie(OriginBlock, Condition, Negate) <
             std::tie(Other.OriginBlock, Other.Condition, Other.Negate);
    }
  };
  using PredicateTermsSet = SmallVector<PredicateTerm, 4>;

  // Mapping from the block to its complete set of PredicateTerms affecting this
  // block's predicates.
  DenseMap<VPBlockBase *, std::pair<PredicateTermsSet, bool>>
      Block2PredicateTermsAndUniformity;
  std::map<PredicateTerm, SmallVector<VPBlockBase *, 4>>
      PredicateTerm2UseBlocks;

  std::map<PredicateTerm, DenseMap<VPBlockBase *, VPValue *>>
      PredicateTerm2LiveInMap;

  // Map from condition bits to their's negation. Needed to avoid duplicate
  // "not" vpinstructions creation.
  DenseMap<VPValue *, VPValue *> Cond2NotCond;

  // Set of blocks that were already split to insert "AND" calculation for
  // PredicateTerms.
  SmallPtrSet<VPBlockBase *, 16> SplitBlocks;

  // We don't emit block predicates for uniform blocks, thus need store the
  // predicate in a separate map.
  DenseMap<VPBlockBase *, VPValue *> Block2Predicate;

  /// Returns the negation of the \p Cond inserted at the end of the block
  /// defining it or at VPlan's entry. Avoids creating duplicates by caching
  /// created NOTs.
  VPValue *getOrCreateNot(VPValue *Cond);

  // Fill in the information about PredicateTerms of the predicate of the
  // \p CurrBlock.
  void calculatePredicateTerms(VPBlockBase *CurrBlock);

  /// Helper method to prepare data for Iterated Dominance Frontier calculation
  /// that is needed to determine where SSA phis need to be inserted in the
  /// updated CFG to preserve SSA form for the values calculating predicates.
  void computeLiveInsForIDF(PredicateTerm Term,
                            SmallPtrSetImpl<VPBlockBase *> &LiveInBlocks);

  /// Helper for getOrCreateValueForPredicateTerm. Only creates the defining
  /// value for the \p PredTerm, without any SSA phi insertion.
  ///
  /// If PredTerm.Condition is empty, just return PredTerm.OriginBlock's
  /// predicate. Otherwise, create Value represeting \p PredTerm, insert it at
  /// the current Builder's insertion point and return it. That value is
  ///
  ///   Val = OriginBlock.Predicate && (possibly negated)Condition.
  ///
  /// Note, that "and" instruction is created in a new block inserted right
  /// after the OriginBlock so that its calculation isn't affected by
  /// OriginBlock's block-predicate instruction.
  VPValue *createDefiningValueForPredicateTerm(PredicateTerm PredTerm);

  /// This must be run *after* the linearization of divergent parts of the
  /// region has happened!
  ///
  /// Given
  ///
  ///   Val = OriginBlock.Predicate && (possibly negated)Condition.
  ///
  /// This method returns either the Val itself, if it's available at \p
  /// AtBlock, or a phi node having "Val" for predecessors on the path from
  /// PredTerm.OriginBlock to \p AtBlock. and false for others.
  ///
  /// At the first query for a given \p PredTerm both the Val and the
  /// required phi-nodes to preserve SSA-form are created/inserted and recorded in
  /// internal map. Subsequent queries return pre-calculated values.
  VPValue *getOrCreateValueForPredicateTerm(PredicateTerm PredTerm,
                                            VPBlockBase *AtBlock);

  /// Generate and return the result of ORing all the predicate VPValues in \p
  /// Worklist. Uses the current insertion point of Builder member.
  VPValue *genPredicateTree(std::list<VPValue *> &Worklist);

  /// Predicate and linearize the CFG within \p Region, recursively.
  void predicateAndLinearizeRegionRec(VPRegionBlock *Region,
                                      bool SearchLoopHack);

  /// Linearize \p Region (without recursion) and mark PHIs in the linearized
  /// blocks as blended. It does *NOT* update condition bits for the blocks,
  /// this information is needed for block predicate insertion after
  /// linearization.
  void
  linearizeRegion(const ReversePostOrderTraversal<VPBlockBase *> &RegionRPOT);

  bool shouldPreserveUniformBranches() const;

  bool shouldPreserveOutgoingEdges(VPBlockBase *Block);

#if INTEL_CUSTOMIZATION
  void handleInnerLoopBackedges(VPLoop *VPL);
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
