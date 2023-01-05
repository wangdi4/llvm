//===-------- DDUtils.h - Utilities for DD  -------------------------------===//
//
// Copyright (C) 2015-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file defines the utilities for DDUtils class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_DDUTILS_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_DDUTILS_H
#include "llvm/ADT/SmallSet.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"

namespace llvm {
namespace loopopt {

typedef SmallSet<unsigned, 16> SpecialSymbasesTy;
typedef SpecialSymbasesTy InterchangeIgnorableSymbasesTy;

struct FPInductionInfo {
  const HLInst *DefInst;
  const RegDDRef *StrideRef;
};

/// \brief Defines utilities for DDUtils Class
///
/// It contains a functions which analyze and manipulate DD
/// It does not store any state.
///
class HIRFramework;
class DDGraph;

class DDUtils {
private:
  /// \brief Do not allow instantiation
  DDUtils() = delete;
  friend class HIRParser;
  friend class HLNodeUtils;

public:
  /// \brief Any incoming/outgoing edge into Loop?
  static bool anyEdgeToLoop(DDGraph DDG, const DDRef *Ref, HLLoop *Loop);

  /// \brief Count each incoming/outgoing edge into Loop where 1 end of the
  /// edge is given in Ref.
  /// Return true if there is any edge to loop. Both Lval
  /// count and Rval count will be returned implicitly.
  static bool countEdgeToLoop(DDGraph DDG, const DDRef *Ref, HLLoop *Loop,
                              unsigned &LvalCount, unsigned &RvalCount);

  ///  \brief Update the linearity of DDRef when it becomes part of the
  ///  innermost loop
  ///  (as a result of ld/st movement or complete unrolling)
  ///  Current code only work on stmts inside the innermost loop
  static void updateDDRefsLinearity(SmallVectorImpl<HLInst *> &HLInsts,
                                    DDGraph DDG);

  /// \brief  Enables Perfect Loop Nests
  /// Takes care of simple cases that are needed for Interchange
  /// InnermostLoop is the loop into which instructions enclosinging it
  /// (i.e. instructions of the loopbody of the parent loop of InnermostLoop)
  /// DDG is the data dependency graph before instructions are sinked.
  /// SinkedTempDDRefSymbases is a set of symbases of the temps
  /// in the sinked instruction. These are ignored from DD edge checks
  /// for loop interchange later. The same DDG before sinking will be used
  /// later.
  static bool
  enablePerfectLoopNest(HLLoop *InnermostLoop, DDGraph DDG,
                        InterchangeIgnorableSymbasesTy &SinkedTempDDRefSymbases,
                        bool AllowNonPerfectSinking = false);

  /// \brief  Checks if a LvalRef has 'Threshold' uses in a loop
  static bool maxUsesInLoop(const RegDDRef *LvalRef, const HLLoop *Loop,
                            DDGraph DDG, const unsigned Threshold);

  /// \brief  Checks if a LvalRef has 1 single use in a loop
  static bool singleUseInLoop(const RegDDRef *LvalRef, const HLLoop *Loop,
                              DDGraph DDG);

  /// \brief  Checks if a DDRef is a valid reduction. It needs to match
  /// the symbase as well
  static bool isValidReductionDDRef(RegDDRef *RRef, HLLoop *Loop,
                                    unsigned FirstSymbase,
                                    bool *LastReductionInst, DDGraph DDG);

  ///  Check all DV to see if it's legal to move SrcLevel's element
  ///  past DstLevel's element.
  ///  e.g. Assuming  dv = (< = >),  SrcLevel = 3 and DstLevel  = 1
  ///       It will return false because > cannot cross <.
  ///
  ///  Notice that using this utility to verify the validity of a new
  ///  permutation is not straightforward unless SrcLevel and DstLevel are
  ///  adjacent both in the original and the new permutations.
  static bool isLegalForPermutation(unsigned DstLevel, unsigned SrcLevel,
                                    unsigned OutmostNestingLevel,
                                    ArrayRef<DirectionVector> DVs);

  /// Collect DVs for checking the legality of loop permutation (a.k.a
  /// interchange). Usable in any transformation for preparing checking
  /// validity of permutation. OutermostLoop is the outermost loop of
  /// the loopnest being analyzed for permutation. Collect Edges that
  /// prevent permutation for OptReport.
  static void computeDVsForPermuteWithSBs(
      SmallVectorImpl<std::pair<DirectionVector, unsigned>> &DVs,
      const HLLoop *OutermostLoop, unsigned InnermostNestingLevel,
      HIRDDAnalysis &DDA, HIRSafeReductionAnalysis &SRA, bool RefineDV,
      const SpecialSymbasesTy *SpecialSBs,
      SmallVectorImpl<const DDEdge *> *PermutePreventEdges = nullptr);

  static void computeDVsForPermuteWithSBs(
      SmallVectorImpl<DirectionVector> &DVs, const HLLoop *OutermostLoop,
      unsigned InnermostNestingLevel, HIRDDAnalysis &DDA,
      HIRSafeReductionAnalysis &SRA, bool RefineDV,
      const SpecialSymbasesTy *SpecialSBs,
      SmallVectorImpl<const DDEdge *> *PermutePreventEdges = nullptr);

  /// Looks for a single dominating (load inst) definition of the base pointer
  /// of \p MemRef. Returns the rval load ref if found, nullptr otherwise. Ex-
  ///
  /// \code
  /// p = A[0].1; << BasePtrLoadRef
  ///   = p[5]; << MemRef
  /// \endcode
  static const RegDDRef *getSingleBasePtrLoadRef(const DDGraph &DDG,
                                                 const RegDDRef *MemRef);

  // \p IsPreLoop indicates whether the sinked instruction appeared before or
  // after the loop before sinking.
  static void updateLiveinsLiveoutsForSinkedInst(HLLoop *InnermostLoop,
                                                 HLInst *SinkedInst,
                                                 bool IsPreLoop);

  static void gatherTempRegDDRefSymbases(
      const SmallVectorImpl<HLInst *> &Insts,
      InterchangeIgnorableSymbasesTy &SinkedTempDDRefSymbases);

  static void
  populateFPInductions(const HLLoop *Lp, const DDGraph &DDG,
                       SmallVectorImpl<FPInductionInfo> &FPInductions);
};
} // End namespace loopopt
} // End namespace llvm

#endif

// TODO:
// 1.Passing a DDGraph object is very expensive, try to use DDGraph& instead;
// 2.
// 3.
// 4.
// 5.
