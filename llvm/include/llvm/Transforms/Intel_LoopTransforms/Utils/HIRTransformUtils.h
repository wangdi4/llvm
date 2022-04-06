//===------ HIRTransformUtils.h ---------------------------- --*- C++ -*---===//
//
// Copyright (C) 2015-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file defines utility functions for the following transformations:
// - HIR Loop Reversal;
// -
// -
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_HIRTRANSFORM_UTILS_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_HIRTRANSFORM_UTILS_H

#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/SmallVector.h"

#include "llvm/Analysis/Intel_LoopAnalysis/IR/CanonExpr.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLDDNode.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLNode.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeVisitor.h"

#include <stdint.h>

namespace llvm {

class OptReportBuilder;
#if INTEL_FEATURE_SW_DTRANS
class FieldModRefResult;
class DTransImmutableInfo;
#endif // INTEL_FEATURE_SW_DTRANS

namespace loopopt {

class RegDDRef;
class HLLoop;
class HLIf;
class HIRDDAnalysis;
class HIRSafeReductionAnalysis;
class HIRLoopStatistics;

enum OptimizationType { Unroll, UnrollAndJam, Vectorizer };

const StringRef EnableSpecialLoopInterchangeMetaName =
    "intel.loop.special.interchange.enable";

/// Defines HIRLoopTransformationUtils class.
/// It contains static member functions to analyze and transform a loop.
class HIRTransformUtils {
public:
  typedef struct ProfInfo {
    ProfInfo(uint64_t T, uint64_t F) : TrueWeight(T), FalseWeight(F) {}
    uint64_t TrueWeight;
    uint64_t FalseWeight;

    uint64_t Quotient; // Quotient of TrueWeight/Denom. Denom is not maintained.
    uint64_t Remainder; // Remainder of TrueWeight/Denom.
  } ProfInfo;

private:
  /// Do not allow instantiation
  HIRTransformUtils() = delete;

  /// Updates bound DDRef by setting the correct defined at level and
  /// adding a blob DDref for the newly created temp.
  static void updateBoundDDRef(RegDDRef *BoundRef, unsigned BlobIndex,
                               unsigned DefLevel);

  /// Returns true if unrolling \p OrigLoop by UnrollOrVecFactor requires a
  /// remainder loop. It also creates new bounds for unrolled loops.
  /// \p NewTripCountP is used to return new loop trip count for constant trip
  /// loops \p  NewTCRef is used for non-constant trip loops.
  static bool isRemainderLoopNeeded(HLLoop *OrigLoop,
                                    unsigned UnrollOrVecFactor,
                                    uint64_t *NewTripCountP,
                                    RegDDRef **NewTCRef, HLIf *RTIf);

  /// \brief Creates a new loop for unrolling or vectorization. \p NewTripCount
  /// contains the new loop trip count if the original loop is a constant trip
  /// count. For a original non-constant trip count loop, the new loop trip
  /// count is specified in \p NewTCRef.
  static HLLoop *
  createUnrollOrVecLoop(HLLoop *OrigLoop, unsigned UnrollOrVecFactor,
                        uint64_t NewTripCount, const RegDDRef *NewTCRef,
                        bool NeedRemainderLoop, OptReportBuilder &ORBuilder,
                        OptimizationType, HLIf *RTIf, ProfInfo *Prof);

  /// \brief Processes the remainder loop for general unrolling and
  /// vectorization. The loop passed in \p OrigLoop is set up to be
  /// the remainder loop with lowerbound set using \p NewTripCount or
  /// \p NewTCRef.
  static void processRemainderLoop(HLLoop *OrigLoop, unsigned UnrollOrVecFactor,
                                   uint64_t NewTripCount,
                                   const RegDDRef *NewTCRef,
                                   const bool HasRuntimeCheck,
                                   bool NeedZeroTripCheck,
                                   const ProfInfo *Prof);

  /// \brief Update CE for stripmined Loops
  static void updateStripminedLoopCE(HLLoop *Loop);

public:
  ///
  /// Do Reversal Tests for a given HIR inner-most loop and return true if
  /// the loop is reversible.
  ///
  /// Parameters:
  /// -HLLoop*: the HLLoop *
  /// -HIRDDAnalysis &: Existing DD Analysis
  /// -HIRSafeReductionAnalysis &: Existing SafeReduction analysis
  /// -HIRLoopStatistics &: existing LoopStatistics analysis
  /// -DoProfitTest:
  ///    Whether to conduct profitable test using reverser's profit model.
  ///    Default is false, which ignores Reverser's profit model and don't do
  ///    profit test.
  /// -SkipLoopBoundChecks: Indicates skipping checks related to loop bounds
  /// like whether loop is normalized. These are weaknesses of the reveral
  /// transformation and do not affect legality. Therefore they can be skipped
  /// for the purposes of legality analysis.
  ///
  /// Return: bool
  /// - true:  the loop is reversible;
  /// - false: otherwise;
  ///
  /// Note: the following decisions are made after group (HPO+Vectorization)
  ///       discussions.
  ///
  /// - DoProfitTest:
  ///   This is an option to the client, and can be ignored if the client
  ///   decides to proceed without using Reversal's profit model.
  ///
  /// - NO DoLegalTest flag
  ///   Can't allow a client to skip this test. The client may not have a legal
  ///   model thus has to rely on Reversal's legal model instead.
  ///   Even if the client does have one, it may be quite different from the
  ///   reversal's legal model, and may not want to spend effort maintaining.
  ///   As a result, this function will implicitly perform legal test.
  ///
  /// There is also an idea of passing context from this API to the next one in
  /// order to avoid doing repetitive work on collection and analysis (save some
  /// compile time). This idea is currently on hold due to unclear usage cases
  /// from client.
  /// Will revisit the situation once there is any request from potential
  /// client.
  ///
  static bool isLoopReversible(HLLoop *InnermostLp, HIRDDAnalysis &HDDA,
                               HIRSafeReductionAnalysis &HSRA,
                               HIRLoopStatistics &HLS,
                               bool DoProfitTest = false,
                               bool SkipLoopBoundChecks = false);

  /// Do Certain Reversal Tests for a given HIR inner-most loop.
  /// Reverse the loop if the loop is legal to reverse.
  //
  // Parameters:
  // -HLLoop*: the HLLoop *
  // -HIRDDAnalysis &: Existing DD Analysis
  // -HIRSafeReductionAnalysis &: Existing SafeReduction analysis
  // -HIRLoopStatistics &: existing LoopStatistics analysis
  //
  // Return: void
  // - assert if any required reversal tests fail.
  // - reverse the loop if reversal tests are successful.
  //
  // Note: the following decisions are made after group discussions.
  //
  // Ideally, a client will call this function after a previous call to
  // isHIRLoopReverible(-) and the loop is indeed reversible.
  // However, in case the client calls this function directly without proper
  // preparation, this function will implicitly assert on preliminary check
  // and legal check, only ignoring profit check.
  //
  static void doLoopReversal(
      HLLoop *InnermostLp,            // INPUT + OUTPUT: an inner-most loop
      HIRDDAnalysis &HDDA,            // INPUT: HIR DDAnalysis
      HIRSafeReductionAnalysis &HSRA, // INPUT: HIRSafeReductionAnalysis
      HIRLoopStatistics &HLS          // INPUT: Existing HIRLoopStatitics
  );

  /// Returns true if \p MemRef is invariant inside \p Loop. \p If IgnoreIVs is
  /// set to true, any IVs present inside \p MemRef will be ignored when making
  /// structural checks. The invariance is checked using DD/ModRef. When
  /// IgnoreIVs is true, the result indicates invariance of the entire range of
  /// locations accessed.
  ///
  /// For example, A[i2] may be considered invariant in the i1 loop if it
  /// doesn't alias with B[][]-
  //
  /// DO i1
  ///   DO i2
  ///     B[i1][i2] = A[i2];
  ///   END DO
  /// END DO
  ///
  /// In some cases, it may return true if the base is reloaded inside the loop
  /// but is also invariant. For example-
  ///
  /// DO i1
  ///   p = S[5].2;  << invariant base
  ///   DO i2
  ///     B[i1][i2] = p[i2]; << invariant load
  ///   END DO
  /// END DO
  static bool isLoopInvariant(const RegDDRef *MemRef, const HLLoop *Loop,
                              HIRDDAnalysis &HDDA, HIRLoopStatistics &HLS,
#if INTEL_FEATURE_SW_DTRANS
                              FieldModRefResult *FieldModRef = nullptr,
#endif // INTEL_FEATURE_SW_DTRANS
                              bool IgnoreIVs = false);

  /// Adjust trip count estimates for given loop by taking into account
  /// the loop unroll or vectorization factor.
  static void adjustTCEstimatesForUnrollOrVecFactor(HLLoop *NewLoop,
                                                    unsigned UnrollOrVecFactor);

  /// This function creates and returns a new loop that will be used as the
  /// main loop for unrolling or vectorization(current clients). The bounds
  /// for this newly created loop are set appropriately using the bounds of
  /// \p OrigLoop and \p UnrollOrVecFactor. If \p PeelFirstIteration is set, a
  /// peel loop executing a single iteration is created and inserted before
  /// the the main loop. If \p PeelArrayRef is set, then generic peeling is done
  /// to align memory accesses to this array. The peel loop is returned in \p
  /// PeelLoop. If a remainder loop is needed, \p NeedRemainderLoop is set to
  /// true and the bounds of \p OrigLoop are updated appropriately. Client is
  /// responsible for deleting OrigLoop if a remainder loop is not needed. \p
  /// VecMode specifies whether the client is vectorizer - which is used to set
  /// loop bounds and stride appropriately as vectorizer uses \p
  /// UnrollOrVecFactor as stride whereas unroller users a stride of 1. The
  /// default client is assumed to be the unroller.
  static HLLoop *setupPeelMainAndRemainderLoops(
      HLLoop *OrigLoop, unsigned UnrollOrVecFactor, bool &NeedRemainderLoop,
      OptReportBuilder &ORBuilder, OptimizationType,
      HLLoop **PeelLoop = nullptr, const RegDDRef *PeelArrayRef = nullptr,
      SmallVectorImpl<std::tuple<HLPredicate, RegDDRef *, RegDDRef *>>
          *RTChecks = nullptr);

  /// Updates Loop properties (Bounds, etc) based on input Permutations
  /// Used by Interchange now.
  /// Loops are added to \p LoopPermutation in the desired permuted order.
  /// The given loopnest starting from OutermostLoop does not have to be
  /// a perfect loop nest.
  /// Then, update loop body's IVs accordingly to LoopPermuation.
  /// E.g. If Loop is permutated from (1 2) --> (2 1), IV i1 is changed into i2,
  /// and i2 is changed into i1.
  /// Notice for full interchange effect, both permuteLoopNests() and
  /// updatePermutedLoopBody need to be called.
  static void
  permuteLoopNests(HLLoop *OutermostLoop,
                   const SmallVectorImpl<const HLLoop *> &LoopPermutation,
                   unsigned InnermostLevel);

  /// Updates target HLLabel in every HLGoto node according to the mapping.
  static void remapLabelsRange(const HLNodeMapper &Mapper, HLNode *Begin,
                               HLNode *End);

  ///  Perform Stripmine.
  ///  Utility that can be shared by distribution and blocking
  ///  Distribution has a sequence of loops
  ///  Blocking normally has one -  FirstLoop == LastLoop
  ///
  ///  Returns true when Stripmine is performed
  ///
  ///    DO i1=0,N-1
  ///      A[i1] =
  ///    ENDDO
  ///    DO i1=0,N-1
  ///      A[i1] += 1
  ///    ENDDO
  ///  ==>
  ///    First form a loop to enclose the input loops
  ///    DO i1=0,N-1
  ///      DO i2=0,N-1
  ///         A[i1] =
  ///      ENDDO
  ///      DO i2=0,N-1
  ///        ...
  ///  ==>
  ///    Before normalization, assuming  StripmineSize = 64
  ///    It is changed as
  ///    DO i1=0, (N-1) / 64
  ///       N2 = min(-64 *i1 + N-1, 64-1)
  ///       do i2=64*i1, 64*i1 + N2
  ///          A[i2] = 1
  ///
  /// When \p AllowExplicitBoundInst is set, the lower bound ref
  /// gets its own instruction setting a selfblob, allowing normalization
  /// to succeed when it would normally fail. Before the loop, we will see
  /// %lb blob being set like so:
  ///     %lb = 14 * i2;
  ///   DO i3 = 0, 14 * i2 + %min + -1 * %lb, 1
  ///
  static void stripmine(HLLoop *FirstLoop, HLLoop *LastLoop,
                        unsigned StripmineSize,
                        bool AllowExplicitBoundInst = false);

  /// Performs complete unroll for \p Loop.
  /// NOTE: Does not handle non-constant lower bounds. For example-
  /// DO i1 = t, t+1, 1
  static void completeUnroll(HLLoop *Loop);

  /// Multiplies trip count of \p Loop using passed in \p Multiplier.
  /// It also updates trip count pragma and max trip count estimate.
  /// Returns false if multiplied trip count will overflow uint64,
  /// and do not change Loop's trip count.
  ///
  /// \p Loop is expected to be normalized.
  ///
  /// The utility may widen the IV if there is chance of overflow in the
  /// multiplied trip count. In some cases, it might create a new instruction in
  /// the preheader and invalidate the parent loop.
  ///
  /// This utility was created to be shared between do loop rerolling and
  /// do multi-exit loop rerolling pass.
  static bool multiplyTripCount(HLLoop *Loop, unsigned Multiplier);

  /// Update profile data attached to [\p Being, \p End), if any, by dividing
  /// the branch weights by \p Denominator.
  /// This function is used by unroll-related transformations.
  static void divideProfileDataBy(HLContainerTy::iterator Begin,
                                  HLContainerTy::iterator End,
                                  uint64_t Denominator);

  /// Collect Ztt conditions of \p Loop into passed \p ZTTs.
  /// If \p Clone is true, existing Ztts are just cloned and not removed from \p
  /// Loop. Otherwise, they are removed from \p Loop.
  static void cloneOrRemoveZttPredicates(HLLoop *Loop,
                                         SmallVectorImpl<PredicateTuple> &ZTTs,
                                         bool Clone);

  /// Add conditions in ZTTs (LHS PRED_OP RHS) to Loop's ztt. Conditions are
  /// merged with logical and operation.
  static void mergeZtt(HLLoop *Loop, SmallVectorImpl<PredicateTuple> &ZTTs);

  /// Run HIRDeadStoreElimination Pass on a HLLoop
  static bool doDeadStoreElimination(HLRegion &Region, HLLoop *Lp,
                                     HIRDDAnalysis &HDDA,
                                     HIRLoopStatistics &HLS);

  /// Run HIRDeadStoreElimination Pass on a HLRegion
  static bool doDeadStoreElimination(HLRegion &Region, HIRDDAnalysis &HDDA,
                                     HIRLoopStatistics &HLS);

  /// Utility to substitute identity matrix uses. Example:
  ///
  /// do i=1,5
  ///   do j=1,5
  ///     B(j,i) = t1 - 0.5* dt * B(j,i) * A(j,i)
  ///   enddo
  /// enddo
  ///
  /// Assuming A(j,i) is the identity ref, then we can substitute 1 or 0
  /// depending on i and j values. Loop should be unrolled, so indices will be
  /// constant.
  static bool doIdentityMatrixSubstitution(HLLoop *Loop,
                                           const RegDDRef *IdentityRef);

  /// Propagates constants to refs and does constant folding for instructions.
  /// Also substitutes constant global refs with equivalent constants.
#if INTEL_FEATURE_SW_DTRANS
  static bool doConstantPropagation(HLNode *Node, DTransImmutableInfo *DTII);
#else  // INTEL_FEATURE_SW_DTRANS
  static bool doConstantPropagation(HLNode *Node);
#endif // INTEL_FEATURE_SW_DTRANS

  /// Returns true if instruction was folded, along with the new instruction.
  /// If the instruction is null, it folded into a self-assignment (no-op).
  /// \p Invalidate indicates that we need to invalidate the parent loop/region
  static std::pair<bool, HLInst *> constantFoldInst(HLInst *Inst,
                                                    bool Invalidate);

  /// Conduct Array Scalarization for all memrefs provided in a set of symbases.
  ///
  /// A typical transformation looks like-
  ///  example1 [BEFORE]
  ///  do i1
  ///    A[0][0] = .
  ///    ...
  ///       .    = A[0][0]
  ///    ...
  ///
  ///  end do
  ///
  ///  These memrefs are on local arrays, contain only const-only subscripts,
  ///  and are dead after the loop. Thus they can be replaced by a scalar temp.
  ///  The above loop will become-
  ///
  ///  example1 [AFTER]
  ///    type t0
  ///  do i1
  ///    t0       = .
  ///    ...
  ///          .  = t0
  ///    ...
  ///
  ///  end do
  ///
  static bool doArrayScalarization(HLLoop *InnermostLp,
                                   SmallSet<unsigned, 8> &SBS);

  static bool doOptVarPredicate(HLLoop *Loop,
                                SmallVectorImpl<HLLoop *> &OutLoops,
                                SmallPtrSetImpl<HLNode *> &NodesToInvalidate);

  /// Set \p Ref to be \p Blob with \p BlobIndex
  /// The ref should be a single canon expr.
  /// Notice that makeConsistent over Ref is NOT called.
  /// It is user's responisbility.
  static void setSelfBlobDDRef(RegDDRef *Ref, BlobTy Blob, unsigned BlobIndex);

  /// Replaces \p OldRef in its HLDDNode by \p NewRef. It returns the node where
  /// the replacement was performed. This may be different than the original
  /// node if creating a new one was necessary for replacement. Old node will be
  /// replaced by the new one in this case. Note: This utility does not
  /// invalidate analyses.
  static HLDDNode *replaceOperand(RegDDRef *OldRef, RegDDRef *NewRef);

  /// Contract a single high-dimension memref into its equivalent low-dimension
  /// memref counterpart.
  ///  example [BEFORE]
  ///  ..
  ///    A[1][2][i][j][k] = .
  ///  ..
  ///
  /// when information is provided to specify that dimensions 1, 2, and 3 are
  /// to be contracted, the memref will become:
  ///
  ///  example [AFTER]
  ///  ..
  ///    AA[1][2] = .
  ///  ..
  ///
  static bool
  contractMemRef(RegDDRef *ToContractRef,               /* INPUT */
                 SmallSet<unsigned, 4> &PreservedDims,  /* Dims preserved */
                 SmallSet<unsigned, 4> &ToContractDims, /* Dims to contract */
                 HLRegion &Reg,
                 RegDDRef *&AfterContractRef /* Ref after contraction */);

  /// Special Sink utility to create an otherwise perfect loopnest.
  /// It can Handle cases similar to the loopnest below:
  /// .. ..
  ///   %1442 = %720  *  5.000000e-01; /* 5 prehdr instructions */
  ///   %1443 =  - %1442;
  ///   %1444 = %720  *  %7;
  ///   %1445 = %1444  *  2.000000e+00;*** ******** ***
  ///   %1446 = %1445  *  %501;
  /// + DO i2 = 0, sext.i32.i64(%6) + -1, 1   <DO_LOOP>
  /// |   + DO i3 = 0, sext.i32.i64(%3) + -1, 1   <DO_LOOP>
  /// |   |      %1472 = i3 + 1  %  %3;         *** to sink:    ***
  /// |   |      %1476 = i3 + %3 + -1  %  %3;   *** ops: mod(%) ***
  /// |   |   + DO i4 = 0, sext.i32.i64(%2) + -1, 1   <DO_LOOP>
  /// |   |   |   ...
  /// |   |   |   use(s) of %1472, %1476, %1442, etc.
  /// |   |   |   ...
  /// |   |   + END LOOP
  /// |   + END LOOP
  /// + END LOOP
  ///
  /// [Notes]
  /// - The loopnest is perfect except the preheader of the innermost loop.
  /// - The statements include mod(%) and select operators.
  ///   They are not covered by copy, load, and store, thus existing
  ///   DDUtils::enablePerfectLoopNest(.) can't handle it.
  ///
  static bool doSpecialSinkForPerfectLoopnest(HLLoop *OuterLp, HLLoop *InnerLp,
                                              HIRDDAnalysis &HDDA);
};

} // End namespace loopopt

} // End namespace llvm

#endif
