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
class raw_ostream;
namespace vpo {

class VPAllocatePrivate;
class VPlanVector;
class VPLoop;
class VPInstruction;
class VPValue;
class VPLoadStoreInst;
class VPGEPInstruction;
class VPCallInstruction;
class VPSubscriptInst;

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
  VPSOAAnalysis(const VPlanVector &Plan, const VPLoop &Loop)
      : Plan(Plan), Loop(Loop) {}

  /// Public interface for SOA-analysis for all loop-privates. \p SOAVars is the
  /// output argument that return the variables marked for SOA-layout.
  void doSOAAnalysis(SmallPtrSetImpl<VPInstruction *> &SOAVars);

private:
  using AllocaToPhiMapTy =
      DenseMap<VPAllocatePrivate *, DenseSet<const VPInstruction *>>;
  using PhiToAllocaMapTy =
      DenseMap<const VPInstruction *, DenseSet<VPAllocatePrivate *>>;

  // Cache to hold previously-determined profitable/unprofitable mem-accesses.
  DenseMap<const VPInstruction *, bool> AccessProfitabilityInfo;

  // VPlan object.
  const VPlanVector &Plan;

  // VPLoop object.
  const VPLoop &Loop;

  // Maps VPAllocaPrivate to a vector of VPInstructions that mix it with other
  // VPAllocatePrivate (VPPHINode or Select).
  AllocaToPhiMapTy AllocaToPhiMap;

  // Maps VPPHINode or Select to their operands which are [derived from]
  // VPAllocatePrivate.
  PhiToAllocaMapTy PhiToAllocaMap;

  class SOASafetyChecker {
    // The running worklist of all the instructions which we want to track.
    using WorkList = SetVector<const VPInstruction *>;
    WorkList WL;

    // Set to avoid repeat-processing in case of cyclic Use-Def chains.
    DenseSet<const VPInstruction *> AnalyzedInsts;

    VPSOAAnalysis &Analysis;
    VPAllocatePrivate *Private;

  public:
    SOASafetyChecker(const SOASafetyChecker &) = delete;
    SOASafetyChecker &operator=(const SOASafetyChecker &) = delete;

    SOASafetyChecker(VPSOAAnalysis &A, VPAllocatePrivate *Priv)
        : Analysis(A), Private(Priv) {}

    // Determine if the layout of the \Private may escape or
    // may change during memory read/writes.
    // The layout of a pointer to private can escape by several reasons:
    //  - storing it into a memory
    //  - passing it to a function
    //  - mixing it with a non-private pointer in a phi or select.
    // The layout is changed e.g. when an array of i32 is read as
    // an array of i8, i.e. when the width of access is changed.
    //
    bool isSafeForSOA();

    /// \returns true if \p UseInst is a safe operation. This would evaluate
    /// to see if \p UseInst is a trivial pointer aliasing instruction,
    /// or a safe function-call.
    bool isSafeUse(const VPInstruction *UseInst, const VPInstruction *CurrentI,
                   Type *AllocatedType);

    /// \returns true if \p UseInst is a safe instruction. Load/Stores are
    /// tested for various constraints to determine safety and every other
    /// instruction is considered unsafe.
    bool isSafeLoadStore(const VPLoadStoreInst *LSI,
                         const VPInstruction *CurrentI, Type *PrivElemSize);

    /// \returns true is \p VPGEP is a safe instruction.
    bool isSafeGEPInst(const VPGEPInstruction *VPGEP, Type *AllocatedType,
                       Type *PrivElemSize) const;

    /// \returns true is \p VPS is a safe instruction.
    bool isSafeVPSubscriptInst(const VPSubscriptInst *VPS, Type *AllocatedType,
                               Type *PrivElemSize) const;

    /// \returns true if \p UseInst is any function call that we know is a
    /// safe-function, i.e., passing a private-pointer would not result in
    /// change of data-layout on the pointee.
    bool isSafePointerEscapeFunction(const VPCallInstruction *VPCall);

    /// \return true if \p Inst is safe for soa layout.
    /// It's assumed that Inst is a PHI or Select.
    bool isMergeSafeForSOA(const VPInstruction *Inst);

    // Check whether \p V is a chain of VPInstructions started with
    // VPAllocatePrivate(s) - in case of phi/select we check for all operands be
    // derived from VPAllocatePrivate.
    bool isDerivedFromAllocatePriv(const VPValue *V);
  };

  /// \return true if the incoming instruction is in the relevant scope.
  /// Specifically, we check if the instruction is non-null pointer and
  /// either
  /// a) In the loop-preheader, or,
  /// b) In the loop.
  bool isInstructionInRelevantScope(const VPInstruction *UseInst);

  /// Determine if the memory pointed to by the Alloca escapes.
  bool memoryEscapes(VPAllocatePrivate *Alloca);

  /// \returns true if \p Val's pointee-type is a scalar.
  bool isSOASupportedTy(Type *Ty);

  /// Determine whether there is a data-access profitable with SOA layout to
  /// the private memory pointed to by the Alloca. Profitability is determined
  /// by the vector shape of access. Only uniform accesses are considered
  /// profitable.
  bool isSOAProfitable(VPAllocatePrivate *Alloca);

  /// \return true if the given instruction does not break profitability of
  /// SOA layout.
  bool isProfitableForSOA(const VPInstruction *I);

  /// Collect all the loads/stores resulting from a given private \p Alloca.
  void collectLoadStores(const VPAllocatePrivate *Alloca,
                         DenseSet<VPLoadStoreInst *> &LoadStores);

  /// Return the iterator-range to the list of currently analyzed
  /// SOA-profitable instructions.
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
