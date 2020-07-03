//===- IntelVPSOAAnalysis.h -------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===---------------------------------------------------------------------===//
///
/// \file
/// This file defines the VPSOAAnalysis class that is used for doing SOA-safety
/// and SOA-profitability analysis for loop-privates.
//===---------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTEL_VP_SOA_ANALYSIS_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTEL_VP_SOA_ANALYSIS_H

#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallPtrSet.h"

namespace llvm {
class Type;
namespace vpo {

class VPAllocatePrivate;
class VPlan;
class VPLoop;
class VPInstruction;
class VPValue;

extern bool EnableSOAAnalysis;
extern bool VPlanDisplaySOAAnalysisInformation;

/// Class for SOA-datalayout transformation analysis for loop-privates.
class VPSOAAnalysis {
  // The 'UseInst' input argument-name to the functions in the class represent
  // the 'use'-instruction and 'CurrentI' represents the instruction of which
  // 'UseInst' is an use.

public:
  VPSOAAnalysis(const VPSOAAnalysis &) = delete;
  VPSOAAnalysis &operator=(const VPSOAAnalysis &) = delete;

  // Constructor.
  // We want to make sure that SOAAnalysis is run only after results of DA are
  // available. Similarly, it is good to have the VPlan and the VPLoop object
  // set before SOAAnalysis is run.
  VPSOAAnalysis(const VPlan &Plan, const VPLoop &Loop)
      : Plan(Plan), Loop(Loop) {}

  /// Public interface for SOA-analysis for all loop-privates. \p SOAVars is the
  /// output argument that return the variables marked for SOA-layout.
  void doSOAAnalysis(SmallPtrSetImpl<VPInstruction *> &SOAVars);

private:
  using WorkList = SetVector<const VPInstruction *>;

  // The running worklist of all the instructions which we want to track.
  WorkList WL;

  // Set to avoid repeat-processing in case of cyclic Use-Def chains.
  DenseSet<const VPInstruction *> AnalyzedInsts;

  // The list of potentially unsafe instructions. The users of these
  // instructions have to be analyzed for any unsafe behavior.
  DenseSet<const VPValue *> PotentiallyUnsafeInsts;

  // Cache to hold previously-determined profitable/unprofitable mem-accesses.
  DenseMap<const VPInstruction *, bool> AccessProfitabilityInfo;

  // VPlan object.
  const VPlan &Plan;

  // VPLoop object.
  const VPLoop &Loop;

  /// \returns true if \p UseInst is a safe operation. This would evaluate
  /// to see if \p UseInst is a trivial pointer aliasing instruction, a safe
  /// function-call or a safe load/store as determined by
  /// \link VPSOAAnalysis::isSafeInstruction \endlink.
  bool isSafeUse(const VPInstruction *UseInst, const VPInstruction *CurrentI);

  /// \returns true if \p UseInst is a safe instruction. Load/Stores are
  /// tested for various constraints to determine safety and every other
  /// instruction is considered unsafe.
  bool isSafeLoadStore(const VPInstruction *UseInst,
                       const VPInstruction *CurrentI);

  /// Determine if the memory pointed to by the Alloca escapes.
  bool memoryEscapes(const VPAllocatePrivate *Alloca);

  /// \return true if \p UseInst is a known unsafe cast instruction.
  bool isPotentiallyUnsafeCast(const VPInstruction *UseInst);

  /// \ return true if \p UseInst has any potentially-unsafe operands.
  bool hasPotentiallyUnsafeOperands(const VPInstruction *UseInst);

  /// \returns true if \p UseInst is any function call that we know is a
  /// safe-function, i.e., passing a private-pointer would not result in
  /// change of data-layout on the pointee.
  bool isSafePointerEscapeFunction(const VPInstruction *UseInst);

  /// \returns true if \p Val's pointee-type is a scalar.
  bool isSOASupportedTy(Type *Ty);

  /// \returns true if \p Val's pointee-type is a scalar.
  bool isScalarTy(Type *Ty);

  /// \return true if the incoming instruction is in the relevant scope.
  /// Specifically, we check if the instruction is non-null pointer and
  /// either
  /// a) In the loop-preheader, or,
  /// b) In the loop.
  bool isInstructionInRelevantScope(const VPInstruction *UseInst);

  /// Determine whether there is a data-access profitable with SOA layout to
  /// the private memory pointed to by the Alloca. Profitability is determined
  /// by the vector shape of access. Only uniform accesses are considered
  /// profitable.
  bool isSOAProfitable(VPAllocatePrivate *Alloca);

  /// \return true if the given instruction does not break profitability of SOA
  /// layout.
  bool isProfitableForSOA(const VPInstruction *I);

  /// Collect all the loads/stores resulting from a given private \p Alloca.
  void collectLoadStores(const VPAllocatePrivate *Alloca,
                         DenseSet<VPInstruction *> &LoadStores);

  /// Return the iterator-range to the list of currently analyzed SOA-profitable
  // instructions.
  decltype(auto) analyzedMemAccessInsts() const {
    return make_range(AccessProfitabilityInfo.begin(),
                      AccessProfitabilityInfo.end());
  }

  /// Return an iterator range to profitable or unprofitable memory access
  /// instructions based on the boolean template parameter.
  template <bool Profitable = true>
  inline decltype(auto) profitabilityAnalyzedAccessInsts() const {
    return map_range(make_filter_range(analyzedMemAccessInsts(),
                                       [](const auto &InstProfitabilityPair) {
                                         return InstProfitabilityPair.second ==
                                                Profitable;
                                       }),
                     [](const auto &InstProfitabilityPair) {
                       return InstProfitabilityPair.first;
                     });
  }
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump(raw_ostream &OS) const;

  // For use in debug sessions, dump the list of SOA-profitable instructions
  // encountered till the given point in Analysis.
  void dumpSOAProfitable(raw_ostream &OS) const;

  // For use in debug sessions, dump the list of SOA-unprofitable instructions
  // encountered till the given point in Analysis.
  void dumpSOAUnprofitable(raw_ostream &OS) const;
#endif // NDEBUG
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTEL_VP_SOA_ANALYSIS_H
