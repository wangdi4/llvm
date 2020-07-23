//===--------   HIRSafeReductionAnalysis.h    -----------------------------===//
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
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_INTEL_SAFEREDUCTION_H
#define LLVM_ANALYSIS_INTEL_SAFEREDUCTION_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRAnalysisPass.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/BlobUtils.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"

#include <map>

namespace llvm {

class formatted_raw_ostream;

namespace loopopt {

class CanonExpr;
class HLInst;
class HLLoop;

typedef SmallVector<const HLInst *, 4> SafeRedChain;

/// An HIR reduction identified by HIRSafeReductionAnalysis.
struct SafeRedInfo {

  /// The HLInsts that make up the reduction.
  SafeRedChain Chain;

  /// The reduction value's symbase.
  unsigned Symbase;

  /// The reduction operation's opcode.
  ///
  /// Reduction chains can include multiple instructions with different opcodes,
  /// as in this example:
  ///
  /// \code
  /// %red = %red  +  (%A)[i1];
  /// %red = %red  -  (%A)[i1 + %offs]; \endcode
  ///
  /// In this case, OpCode will reflect the first instruction in the chain (an
  /// add in the above example).
  unsigned OpCode;

  /// Whether this reduction involves operations requiring strict floating point
  /// semantics which are unsafe to optimize.
  ///
  /// * For unconditional reductions, this flag is set if any instructions in
  /// the chain disallow reassociation (by not being marked `reassoc`).
  /// * For conditional reductions, this flag is set if any instructions in the
  /// chain disallow any kind of potentially-unsafe optimization (by not being
  /// marked `fast`).
  bool HasUnsafeAlgebra;

  /// Constructs a SafeRedInfo with the given field values.
  SafeRedInfo(SafeRedChain &RedInsts, unsigned Symbase, unsigned RedOpCode,
              bool HasUnsafeAlgebra)
      : Chain(RedInsts), Symbase(Symbase), OpCode(RedOpCode),
        HasUnsafeAlgebra(HasUnsafeAlgebra) {}

  /// An accessor for \ref HasUnsafeAlgebra.
  bool hasUnsafeAlgebra(void) const { return HasUnsafeAlgebra; }
};

typedef SmallVector<SafeRedInfo, 4> SafeRedInfoList;

/// An analysis to identify reductions in HIR.
///
/// This analysis is able to identify many types of non-memory reductions within
/// innermost HLLoops; these can be simple single-instruction reductions, such
/// as:
///
/// \code
/// %red = %red + (%A)[i1]; \endcode
///
/// or even more complicated multi-instruction reductions, such as:
///
/// \code
/// %temp1 = %temp3;
/// %temp2 = %temp1 + (%A)[i1];
/// %temp2 = %temp2 - (%A)[i1 + %offs];
/// %temp3 = %temp2 + (%B)[i1]; \endcode
///
/// Supported reduction operations include `max`, `min`, `add`, `sub`, `and`,
/// `or`, `xor`, `mul`, and `div` for integer, floating-point, and vector types.
/// Multi-instruction reduction chains can include copies, and can also include
/// compatible mixed operations (though this is currently only supported for
/// `add`/`sub` reductions).
///
/// In order to use this pass, you must first call
/// \ref computeSafeReductionChains on either the relevant loop or one of its
/// parents:
///
/// \code
/// SRA.computeSafeReductionChains(RedLoop); \endcode
///
/// Once this is done, the reduction information (stored in the form of a
/// SafeRedInfo for each recognized reduction) can be queried for the entire
/// loop using the \ref getSafeRedInfoList method:
///
/// \code
/// for (const SafeRedInfo& Reduction : SRA.getSafeRedInfoList(RedLoop)) {
///   ...
/// } \endcode
///
/// or for individual instructions using the \ref getSafeRedInfo,
/// \ref isSafeReduction, or \ref isReductionRef methods:
///
/// \code
/// for (HLNode& Node :
///      make_range(RedLoop->child_begin(), RedLoop->child_end())) {
///   ...
///   auto*const Inst = dyn_cast<HLInst>(&Node);
///   if (!Inst) continue;
///   ...
///   if (const SafeRedInfo*const Reduction = SRA.getSafeRedInfo(Inst)) {
///     ...
///   }
/// } \endcode
///
/// Aside from the reduction instruction/instruction chain in
/// SafeRedInfo::Chain, SafeRedInfo records also have several fields that are
/// helpful in determining whether a reduction is safe to optimize in the
/// context of a particular transformation. These include SafeRedInfo::OpCode,
/// which indicates the type of reduction operation performed, and
/// SafeRedInfo::HasUnsafeAlgebra, which indicates the absence of certain fast
/// math flags required for certain types of optimizations.
class HIRSafeReductionAnalysis : public HIRAnalysis {
  HIRDDAnalysis &DDA;

  unsigned FirstRvalSB;
  const HLNode *FirstChild;
  // From Loop, look up all sets of Insts in a Safe Reduction chain
  SmallDenseMap<const HLLoop *, SafeRedInfoList, 16> SafeReductionMap;
  // From Inst, Look up  Index to Reduction Info (Chain, Symbase and Opcode).
  // There is no need to go through Loop,
  // because there are not many safe reductions in a function.
  SmallDenseMap<const HLInst *, unsigned, 16> SafeReductionInstMap;

  bool findFirstRedStmt(const HLLoop *Loop, const HLInst *Inst,
                        bool *SingleStmtReduction, unsigned *FirstSB,
                        unsigned *ReductionOpCode, DDGraph DDG);

  void setSafeRedChainList(SafeRedChain &RedInsts, const HLLoop *Loop,
                           unsigned RedSymbase, unsigned RedOpCode);

  void identifySafeReductionChain(const HLLoop *Loop, DDGraph DDG);
  bool isValidSR(const RegDDRef *LRef, const HLLoop *Loop, HLInst **SinkInst,
                 DDRef **SinkDDRef, unsigned ReductionOpCode, DDGraph DDG);

  // Perform  SafeReducton Analysis
  void identifySafeReduction(const HLLoop *Loop);
  // Checks if a temp is legal to be used for Reduction
  //  e.g s =  10 * s + ..  is not legal
  bool isRedTemp(CanonExpr *CE, unsigned BlobIndex);

public:
  HIRSafeReductionAnalysis(HIRFramework &HIRF, HIRDDAnalysis &DDA);
  HIRSafeReductionAnalysis(const HIRSafeReductionAnalysis &) = delete;
  HIRSafeReductionAnalysis(HIRSafeReductionAnalysis &&Arg)
      : HIRAnalysis(Arg.HIRF), DDA(Arg.DDA),
        SafeReductionMap(std::move(Arg.SafeReductionMap)),
        SafeReductionInstMap(std::move(Arg.SafeReductionInstMap)) {}
  virtual ~HIRSafeReductionAnalysis() {}

  /// Identifies reductions for all innermost loops within \p Loop.
  void computeSafeReductionChains(const HLLoop *Loop);

  /// Gets identified reductions for \p Loop.
  ///
  /// \ref computeSafeReductionChains must be called on \p Loop or on one of its
  /// parent loops before calling this function in order for its results to be
  /// correct.
  const SafeRedInfoList &getSafeRedInfoList(const HLLoop *Loop);

  /// Is \p Inst part of a reduction?
  ///
  /// \param IsSingleStmt If not null, this will be set to true if the reduction
  /// is single-instruction or false if the reduction is a multi-instruction
  /// chain.
  /// \param HasUnsafeAlgebra If not null, this will be set according to
  /// the reduction's SafeRedInfo::HasUnsafeAlgebra value.
  ///
  /// \ref computeSafeReductionChains must be called on a parent loop of \p Inst
  /// before calling this function in order for its results to be correct.
  bool isSafeReduction(const HLInst *Inst, bool *IsSingleStmt = nullptr,
                       bool *HasUnsafeAlgebra = nullptr) const;

  /// Gets the SafeRedInfo record for \p Inst if it is part of a reduction;
  /// Otherwise, returns null.
  ///
  /// \ref computeSafeReductionChains must be called on a parent loop of \p Inst
  /// before calling this function in order for its results to be correct.
  const SafeRedInfo *getSafeRedInfo(const HLInst *Inst) const;

  /// Checks if \p Ref is an operand of an instruction involved in a reduction.
  /// If so, \p RedOpCode is set to the reduction's SafeRedInfo::OpCode value.
  ///
  /// \ref computeSafeReductionChains must be called on a parent loop of the
  /// instruction containing \p Ref before calling this function in order for
  /// its results to be correct.
  bool isReductionRef(const RegDDRef *Ref, unsigned &RedOpCode);

  void printAnalysis(raw_ostream &OS) const override;
  void print(formatted_raw_ostream &OS, const HLLoop *Loop) override;
  void print(formatted_raw_ostream &OS, const HLLoop *Loop,
             const SafeRedInfoList *SR);

  void markLoopBodyModified(const HLLoop *L) override;
};

class HIRSafeReductionAnalysisWrapperPass : public FunctionPass {
  std::unique_ptr<HIRSafeReductionAnalysis> HSR;

public:
  static char ID;
  HIRSafeReductionAnalysisWrapperPass() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  void releaseMemory() override;

  void print(raw_ostream &OS, const Module * = nullptr) const override {
    getHSR().printAnalysis(OS);
  }

  HIRSafeReductionAnalysis &getHSR() { return *HSR; }
  const HIRSafeReductionAnalysis &getHSR() const { return *HSR; }
};

class HIRSafeReductionAnalysisPass
    : public AnalysisInfoMixin<HIRSafeReductionAnalysisPass> {
  friend struct AnalysisInfoMixin<HIRSafeReductionAnalysisPass>;

  static AnalysisKey Key;

public:
  using Result = HIRSafeReductionAnalysis;

  HIRSafeReductionAnalysis run(Function &F, FunctionAnalysisManager &AM);
};

} // End namespace loopopt

} // End namespace llvm

#endif
