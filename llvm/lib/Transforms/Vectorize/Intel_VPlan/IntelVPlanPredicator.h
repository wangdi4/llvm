//===-- IntelVPlanPredicator.h ----------------------------------*- C++ -*-===//
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

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_PREDICATOR_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_PREDICATOR_H

#include "IntelLoopVectorizationPlanner.h"
#include "IntelVPlan.h"
#include "IntelVPlanBuilder.h"
#include "IntelVPlanDominatorTree.h"
#include "llvm/IR/Dominators.h"

namespace llvm {
namespace vpo {

class VPlanPredicator {
private:
  // VPlan being predicated.
  VPlan &Plan;

  // VPLoopInfo for Plan's HCFG.
  VPLoopInfo *VPLI;

  ReversePostOrderTraversal<VPBasicBlock *> RPOT;

  // Describe an edge/condition/predicate affecting given block predicate. Final
  // predicate for that block is an OR of all the PredicateTerms affecting the
  // block.
  struct PredicateTerm {
    // Its predicate will affect this term.
    VPBasicBlock *OriginBlock;
    // Needed if currect block that has this PredicateTerm is affected by the
    // conditional branch terminator in the OriginBlock.
    VPValue *Condition;
    // If condition isn't nullptr and Negate is "true", the influence is coming
    // through the "False" edge coming from the OriginBlock. Must be false if
    // Condition is nullptr.
    bool Negate;

    PredicateTerm(VPBasicBlock *OriginBlock, VPValue *Condition, bool Negate)
        : OriginBlock(OriginBlock), Condition(Condition), Negate(Negate) {
      assert((Condition || !Negate) && "Can't negate missing condition!");
    }

    PredicateTerm(VPBasicBlock *OriginBlock)
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
  DenseMap<VPBasicBlock *, std::pair<PredicateTermsSet, bool>>
      Block2PredicateTermsAndUniformity;
  std::map<PredicateTerm, SmallVector<VPBasicBlock *, 4>>
      PredicateTerm2UseBlocks;

  std::map<PredicateTerm, DenseMap<VPBasicBlock *, VPValue *>>
      PredicateTerm2LiveInMap;

  // Map from condition bits to their's negation. Needed to avoid duplicate
  // "not" vpinstructions creation.
  DenseMap<VPValue *, VPValue *> Cond2NotCond;

  // Predicate-related instructions (and/not/or) should be executed without the
  // mask. As such, in general, we need to have a separate block for them.
  //
  // The problem here is that we want to delay phis-to-blend processing until
  // after predicates are inserted and we don't want the phis to be modified
  // prior to that in order to carry original information in their incoming
  // blocks. Unfortunately, CFG modifications caused by the block splitting
  // would update such phis too.
  //
  // The solution to this problem is to make predicate-related instruction
  // insertion two-phase:
  //  - 1) Insert instructions *into* the blocks that are possibly predicated
  //  - do phis-to-blends processing
  //  - 2) Split the blocks from 1)
  //
  // Record the split points in this map during
  // createDefiningValueForPredicateTerm and use it to perform the actual
  // splitting in the end of the predication process.
  DenseMap<VPBasicBlock *, VPInstruction *> BlocksToSplit;

  // We don't emit block predicates for uniform blocks, thus need store the
  // predicate in a separate map.
  DenseMap<VPBasicBlock *, VPValue *> Block2Predicate;

  // Blocks where phis should be turned into blends as a result of
  // linearization. The transformation is delayed to after-linearization as it
  // might be needed to create extra blends and/or phis on the way through
  // multiple edges in the final CFG (using IDF).
  //
  // Also, we need blocks' predicates for the explicit VPBlend instruction, so
  // for the transform to be performed on-the-fly we would need keeping
  // VPLInfo/DomTree/PostDomTree up-to-date through the linearization process
  // which would be prohibitively hard.
  SetVector<VPBasicBlock *> BlocksToBlendProcess;

  /// Returns the negation of the \p Cond inserted at the end of the block
  /// defining it or at VPlan's entry. Avoids creating duplicates by caching
  /// created NOTs.
  VPValue *getOrCreateNot(VPValue *Cond);

  // Fill in the information about PredicateTerms of the predicate of the
  // \p CurrBlock.
  void calculatePredicateTerms(VPBasicBlock *CurrBlock);

  /// Helper method to prepare data for Iterated Dominance Frontier calculation
  /// that is needed to determine where SSA phis need to be inserted in the
  /// updated CFG to preserve SSA form for the values calculating predicates.
  void computeLiveInsForIDF(PredicateTerm Term,
                            SmallPtrSetImpl<VPBasicBlock *> &LiveInBlocks);
  void computeLiveInsForBlendsIDF(const SmallPtrSetImpl<VPBasicBlock *> &DefBlocks,
                                  const VPBasicBlock *OrigPhiBlock,
                                  SmallPtrSetImpl<VPBasicBlock *> &LiveInBlocks);

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
  /// required phi-nodes to preserve SSA-form are created/inserted and recorded
  /// in internal map. Subsequent queries return pre-calculated values.
  VPValue *getOrCreateValueForPredicateTerm(PredicateTerm PredTerm,
                                            VPBasicBlock *AtBlock);

  /// Generate and return the result of ORing all the predicate VPValues in \p
  /// Worklist. Uses the current insertion point of Builder member.
  VPValue *genPredicateTree(std::list<VPValue *> &Worklist, VPBuilder &Builder);

  /// Lower predicate terms into VPInstructions. Note that some of the
  /// instructions that need not be predicated are under the block-predicate
  /// influence after this method finishes (ANDs used to calculate predicates).
  /// That is handled later by splitting basic blocks right before the first AND
  /// created in the end of those blocks.
  void emitPredicates();

  /// Linearize Plan and mark blocks that should be post-processed for
  /// phi-to-blend substitutuion. It does *NOT* update condition bits for the
  /// blocks, this information is needed for block predicate insertion after
  /// linearization.
  void linearizeRegion();

  /// Process phis that are merge points with incoming edges changed during
  /// linearization process. Includes creation of explicit VPBlendInsts and
  /// insertion of real VPPHINodes to maintain proper SSA form.
  void transformPhisToBlends();
  void transformPhisToBlends(VPBasicBlock *Block);

  // Add an additional all-zero-check for inner loops with uniform backedge
  // condition on a divergent path. Temporary workaround untill proper region
  // bypass infrastructure is implemented.
  void fixupUniformInnerLoops();

  bool shouldPreserveUniformBranches() const;

  bool shouldPreserveOutgoingEdges(VPBasicBlock *Block);

public:
  VPlanPredicator(VPlan &Plan);

  /// Predicate Plan's HCFG.
  void predicate(void);
};
} // namespace vpo
} // end namespace llvm
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_PREDICATOR_H
