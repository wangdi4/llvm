//===---- CGLoopInfo.h - LLVM CodeGen for loop metadata -*- C++ -*---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This is the internal state used for llvm translation for loop statement
// metadata.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_CODEGEN_CGLOOPINFO_H
#define LLVM_CLANG_LIB_CODEGEN_CGLOOPINFO_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/Compiler.h"

namespace llvm {
class BasicBlock;
class Instruction;
class MDNode;
} // end namespace llvm

namespace clang {
class Attr;
class ASTContext;
namespace CodeGen {

/// Attributes that may be specified on loops.
struct LoopAttributes {
  explicit LoopAttributes(bool IsParallel = false);
  void clear();

  /// Generate llvm.loop.parallel metadata for loads and stores.
  bool IsParallel;

  /// State of loop vectorization or unrolling.
  enum LVEnableState { Unspecified, Enable, Disable, Full };

  /// Value for llvm.loop.vectorize.enable metadata.
  LVEnableState VectorizeEnable;

#if INTEL_CUSTOMIZATION
  /// Value for llvm.loop.coalesce.enable metadata.
  bool LoopCoalesceEnable;

  /// Value for llvm.loop.coalesce.count metadata.
  unsigned LoopCoalesceCount;

  /// Value for llvm.loop.ii.count metadata.
  unsigned IICount;

  /// Value for llvm.loop.max_concurrency.count metadata.
  unsigned MaxConcurrencyCount;

  /// Value for llvm.loop.max_interleaving.count metadata.
  unsigned MaxInterleavingCount;

  /// Value for llvm.loop.vectorize.ivdep_back metadata.
  bool IVDepEnable;

  /// Value for llvm.loop.ivdep.enable metadata.
  bool IVDepHLSEnable;

  /// Value for both  llvm.loop.vectorize.ivdep_back and
  /// llvm.loop.ivdep.enable metadata.
  bool IVDepHLSIntelEnable;

  /// Value for llvm.loop.ivdep.safelen metadata.
  unsigned IVDepCount;

  /// Value for llvm.loop.intel.ii.at.most.count metadata.
  unsigned IIAtMost;

  /// Value for llvm.loop.intel.ii.at.least.count metadata.
  unsigned IIAtLeast;

  /// Value for llvm.loop.intel.speculated.iterations.count metadata.
  int SpeculatedIterations;

  /// Value for llvm.loop.intel.min.ii.at.target.fmax metadata.
  bool MinIIAtTargetFmaxEnable;

  /// Value for llvm.loop.intel.pipelining.disable metadata.
  bool DisableLoopPipeliningEnable;

  /// Value for llvm.loop.intel.[hyperopt|nohyperopt] metadata.
  LVEnableState ForceHyperoptEnable;

  /// Value for llvm.loop.fusion.* metadata.
  LVEnableState FusionEnable;

  /// Value for llvm.loop.vectorize.ivdep_loop metadata.
  bool IVDepLoop;

  /// Value for llvm.loop.vectorize.ivdep_back metadata.
  bool IVDepBack;

  /// Value for llvm.loop.vector_always.enable metadata.
  bool VectorizeAlwaysEnable;

  /// Value for llvm.loop.intel.loopcount
  llvm::SmallVector<unsigned, 2> LoopCount;

  /// Value for llvm.loop.intel.loopcount_minimum
  unsigned LoopCountMin;

  /// Value for llvm.loop.intel.loopcount_maximum
  unsigned LoopCountMax;

  /// Value for llvm.loop.intel.loopcount_averag
  unsigned LoopCountAvg;

#endif // INTEL_CUSTOMIZATION

  /// Value for llvm.loop.unroll.* metadata (enable, disable, or full).
  LVEnableState UnrollEnable;

  /// Value for llvm.loop.unroll_and_jam.* metadata (enable, disable, or full).
  LVEnableState UnrollAndJamEnable;

  /// Value for llvm.loop.vectorize.predicate metadata
  LVEnableState VectorizePredicateEnable;

  /// Value for llvm.loop.vectorize.width metadata.
  unsigned VectorizeWidth;

  /// Value for llvm.loop.interleave.count metadata.
  unsigned InterleaveCount;

  /// Value for llvm.loop.ivdep.enable metadata.
  bool SYCLIVDepEnable;

  /// Value for llvm.loop.ivdep.safelen metadata.
  unsigned SYCLIVDepSafelen;

  /// Value for llvm.loop.ii.count metadata.
  unsigned SYCLIInterval;

  /// Flag for llvm.loop.max_concurrency.count metadata.
  bool SYCLMaxConcurrencyEnable;

  /// Value for llvm.loop.max_concurrency.count metadata.
  unsigned SYCLMaxConcurrencyNThreads;

  /// llvm.unroll.
  unsigned UnrollCount;

  /// llvm.unroll.
  unsigned UnrollAndJamCount;

  /// Value for llvm.loop.distribute.enable metadata.
  LVEnableState DistributeEnable;

  /// Value for llvm.loop.pipeline.disable metadata.
  bool PipelineDisabled;

  /// Value for llvm.loop.pipeline.iicount metadata.
  unsigned PipelineInitiationInterval;
};

/// Information used when generating a structured loop.
class LoopInfo {
public:
  /// Construct a new LoopInfo for the loop with entry Header.
  LoopInfo(llvm::BasicBlock *Header, const LoopAttributes &Attrs,
           const llvm::DebugLoc &StartLoc, const llvm::DebugLoc &EndLoc,
           LoopInfo *Parent);

  /// Get the loop id metadata for this loop.
  llvm::MDNode *getLoopID() const { return TempLoopID.get(); }

  /// Get the header block of this loop.
  llvm::BasicBlock *getHeader() const { return Header; }

  /// Get the set of attributes active for this loop.
  const LoopAttributes &getAttributes() const { return Attrs; }

  /// Return this loop's access group or nullptr if it does not have one.
  llvm::MDNode *getAccessGroup() const { return AccGroup; }

  /// Create the loop's metadata. Must be called after its nested loops have
  /// been processed.
  void finish();

private:
  /// Loop ID metadata.
  llvm::TempMDTuple TempLoopID;
  /// Header block of this loop.
  llvm::BasicBlock *Header;
  /// The attributes for this loop.
  LoopAttributes Attrs;
  /// The access group for memory accesses parallel to this loop.
  llvm::MDNode *AccGroup = nullptr;
  /// Start location of this loop.
  llvm::DebugLoc StartLoc;
  /// End location of this loop.
  llvm::DebugLoc EndLoc;
  /// The next outer loop, or nullptr if this is the outermost loop.
  LoopInfo *Parent;
  /// If this loop has unroll-and-jam metadata, this can be set by the inner
  /// loop's LoopInfo to set the llvm.loop.unroll_and_jam.followup_inner
  /// metadata.
  llvm::MDNode *UnrollAndJamInnerFollowup = nullptr;

  /// Create a LoopID without any transformations.
  llvm::MDNode *
  createLoopPropertiesMetadata(llvm::ArrayRef<llvm::Metadata *> LoopProperties);

  /// Create a LoopID for transformations.
  ///
  /// The methods call each other in case multiple transformations are applied
  /// to a loop. The transformation first to be applied will use LoopID of the
  /// next transformation in its followup attribute.
  ///
  /// @param Attrs             The loop's transformations.
  /// @param LoopProperties    Non-transformation properties such as debug
  ///                          location, parallel accesses and disabled
  ///                          transformations. These are added to the returned
  ///                          LoopID.
  /// @param HasUserTransforms [out] Set to true if the returned MDNode encodes
  ///                          at least one transformation.
  ///
  /// @return A LoopID (metadata node) that can be used for the llvm.loop
  ///         annotation or followup-attribute.
  /// @{
  llvm::MDNode *
  createPipeliningMetadata(const LoopAttributes &Attrs,
                           llvm::ArrayRef<llvm::Metadata *> LoopProperties,
                           bool &HasUserTransforms);
  llvm::MDNode *
  createPartialUnrollMetadata(const LoopAttributes &Attrs,
                              llvm::ArrayRef<llvm::Metadata *> LoopProperties,
                              bool &HasUserTransforms);
  llvm::MDNode *
  createUnrollAndJamMetadata(const LoopAttributes &Attrs,
                             llvm::ArrayRef<llvm::Metadata *> LoopProperties,
                             bool &HasUserTransforms);
  llvm::MDNode *
  createLoopVectorizeMetadata(const LoopAttributes &Attrs,
                              llvm::ArrayRef<llvm::Metadata *> LoopProperties,
                              bool &HasUserTransforms);
  llvm::MDNode *
  createLoopDistributeMetadata(const LoopAttributes &Attrs,
                               llvm::ArrayRef<llvm::Metadata *> LoopProperties,
                               bool &HasUserTransforms);
  llvm::MDNode *
  createFullUnrollMetadata(const LoopAttributes &Attrs,
                           llvm::ArrayRef<llvm::Metadata *> LoopProperties,
                           bool &HasUserTransforms);
  /// @}

  /// Create a LoopID for this loop, including transformation-unspecific
  /// metadata such as debug location.
  ///
  /// @param Attrs             This loop's attributes and transformations.
  /// @param LoopProperties    Additional non-transformation properties to add
  ///                          to the LoopID, such as transformation-specific
  ///                          metadata that are not covered by @p Attrs.
  /// @param HasUserTransforms [out] Set to true if the returned MDNode encodes
  ///                          at least one transformation.
  ///
  /// @return A LoopID (metadata node) that can be used for the llvm.loop
  ///         annotation.
  llvm::MDNode *createMetadata(const LoopAttributes &Attrs,
                               llvm::ArrayRef<llvm::Metadata *> LoopProperties,
                               bool &HasUserTransforms);
};

/// A stack of loop information corresponding to loop nesting levels.
/// This stack can be used to prepare attributes which are applied when a loop
/// is emitted.
class LoopInfoStack {
  LoopInfoStack(const LoopInfoStack &) = delete;
  void operator=(const LoopInfoStack &) = delete;

public:
  LoopInfoStack() {}

  /// Begin a new structured loop. The set of staged attributes will be
  /// applied to the loop and then cleared.
  void push(llvm::BasicBlock *Header, const llvm::DebugLoc &StartLoc,
            const llvm::DebugLoc &EndLoc);

  /// Begin a new structured loop. Stage attributes from the Attrs list.
  /// The staged attributes are applied to the loop and then cleared.
  void push(llvm::BasicBlock *Header, clang::ASTContext &Ctx,
            llvm::ArrayRef<const Attr *> Attrs, const llvm::DebugLoc &StartLoc,
            const llvm::DebugLoc &EndLoc);

  /// End the current loop.
  void pop();

  /// Return the top loop id metadata.
  llvm::MDNode *getCurLoopID() const {                     // INTEL
    return hasInfo() ? getInfo().getLoopID() : nullptr;    // INTEL
  }                                                        // INTEL

  /// Return true if the top loop is parallel.
  bool getCurLoopParallel() const {
    return hasInfo() ? getInfo().getAttributes().IsParallel : false;
  }

  /// Function called by the CodeGenFunction when an instruction is
  /// created.
  void InsertHelper(llvm::Instruction *I) const;

  /// Set the next pushed loop as parallel.
  void setParallel(bool Enable = true) { StagedAttrs.IsParallel = Enable; }

#if INTEL_CUSTOMIZATION
  /// Set the next pushed loop 'coalesce.enable'
  void setLoopCoalesceEnable() {
    StagedAttrs.LoopCoalesceEnable = true;
  }

  /// Set the coalesce count for the next loop pushed.
  void setLoopCoalesceCount(unsigned C) { StagedAttrs.LoopCoalesceCount = C; }

  /// Set the ii count for the next loop pushed.
  void setIICount(unsigned C) { StagedAttrs.IICount = C; }

  /// Set the max_concurrency count for the next loop pushed.
  void setMaxConcurrencyCount(unsigned C) {
    StagedAttrs.MaxConcurrencyCount = C;
  }

  /// Set the max_interleaving count for the next loop pushed.
  void setMaxInterleavingCount(unsigned C) {
    StagedAttrs.MaxInterleavingCount = C;
  }

  /// Set flag for three types of plain #pragma ivdep
  void setIVDepEnable() { StagedAttrs.IVDepEnable = true; }
  void setIVDepHLSEnable() { StagedAttrs.IVDepHLSEnable = true; }
  void setIVDepHLSIntelEnable() { StagedAttrs.IVDepHLSIntelEnable = true; }

  /// Set the safelen count for the next loop pushed.
  void setIVDepCount(unsigned C) { StagedAttrs.IVDepCount = C; }


  /// Set II_AT_MOST for the next loop pushed.
  void setIIAtMost(unsigned C) { StagedAttrs.IIAtMost = C; }

  /// Set II_AT_LEAST for the next loop pushed.
  void setIIAtLeast(unsigned C) { StagedAttrs.IIAtLeast = C; }

  /// Set SpeculatedIterations for the next loop pushed.
  void setSpeculatedIterations(unsigned C) {
    StagedAttrs.SpeculatedIterations = C;
  }
  /// Set the next pushed loop MinIIAtTargetFmaxEnable
  void setMinIIAtTargetFmaxEnable() {
    StagedAttrs.MinIIAtTargetFmaxEnable = true;
  }

  /// Set the next pushed loop DisableLoopPipeliningEnable
  void setDisableLoopPipeliningEnable() {
    StagedAttrs.DisableLoopPipeliningEnable = true;
  }

  /// Set the next pushed loop 'force_hyperopt/force_no_hyperopt'
  void setForceHyperoptEnable(bool Enable = true) {
    StagedAttrs.ForceHyperoptEnable =
        Enable ? LoopAttributes::Enable : LoopAttributes::Disable;
  }

  /// Set the next pushed loop 'fusion.enable'
  void setFusionEnable(bool Enable = true) {
    StagedAttrs.FusionEnable =
        Enable ? LoopAttributes::Enable : LoopAttributes::Disable;
  }

  /// Set the loop flag for ivdep.
  void setIVDepLoop() { StagedAttrs.IVDepLoop = true; }

  /// Set the back flag for ivdep.
  void setIVDepBack() { StagedAttrs.IVDepBack = true; }
  /// Set next pushed loop  'vector_always.enable'
  void setVectorizeAlwaysEnable() {
    StagedAttrs.VectorizeAlwaysEnable = true;
  }

  /// Set the LoopCount for the next loop pushed.
  void setLoopCount(unsigned C) {
    StagedAttrs.LoopCount.push_back(C);
  }

  /// Set the LoopCountMin for the next loop pushed.
  void setLoopCountMin(unsigned C) {
    StagedAttrs.LoopCountMin = C;
  }

  /// Set the LoopCountMax for the next loop pushed.
  void setLoopCountMax(unsigned C) {
    StagedAttrs.LoopCountMax = C;
  }

  /// Set the LoopCountAvg for the next loop pushed.
  void setLoopCountAvg(unsigned C) {
    StagedAttrs.LoopCountAvg = C;
  }
#endif // INTEL_CUSTOMIZATION

  /// Set the next pushed loop 'vectorize.enable'
  void setVectorizeEnable(bool Enable = true) {
    StagedAttrs.VectorizeEnable =
        Enable ? LoopAttributes::Enable : LoopAttributes::Disable;
  }

  /// Set the next pushed loop as a distribution candidate.
  void setDistributeState(bool Enable = true) {
    StagedAttrs.DistributeEnable =
        Enable ? LoopAttributes::Enable : LoopAttributes::Disable;
  }

  /// Set the next pushed loop unroll state.
  void setUnrollState(const LoopAttributes::LVEnableState &State) {
    StagedAttrs.UnrollEnable = State;
  }

  /// Set the next pushed vectorize predicate state.
  void setVectorizePredicateState(const LoopAttributes::LVEnableState &State) {
    StagedAttrs.VectorizePredicateEnable = State;
  }

  /// Set the next pushed loop unroll_and_jam state.
  void setUnrollAndJamState(const LoopAttributes::LVEnableState &State) {
    StagedAttrs.UnrollAndJamEnable = State;
  }

  /// Set the vectorize width for the next loop pushed.
  void setVectorizeWidth(unsigned W) { StagedAttrs.VectorizeWidth = W; }

  /// Set the interleave count for the next loop pushed.
  void setInterleaveCount(unsigned C) { StagedAttrs.InterleaveCount = C; }

  /// Set flag of ivdep for the next loop pushed.
  void setSYCLIVDepEnable() { StagedAttrs.SYCLIVDepEnable = true; }

  /// Set value of safelen count for the next loop pushed.
  void setSYCLIVDepSafelen(unsigned C) { StagedAttrs.SYCLIVDepSafelen = C; }

  /// Set value of an initiation interval for the next loop pushed.
  void setSYCLIInterval(unsigned C) { StagedAttrs.SYCLIInterval = C; }

  /// Set flag of max_concurrency for the next loop pushed.
  void setSYCLMaxConcurrencyEnable() {
    StagedAttrs.SYCLMaxConcurrencyEnable = true;
  }

  /// Set value of threads for the next loop pushed.
  void setSYCLMaxConcurrencyNThreads(unsigned C) {
    StagedAttrs.SYCLMaxConcurrencyNThreads = C;
  }

  /// Set the unroll count for the next loop pushed.
  void setUnrollCount(unsigned C) { StagedAttrs.UnrollCount = C; }

  /// \brief Set the unroll count for the next loop pushed.
  void setUnrollAndJamCount(unsigned C) { StagedAttrs.UnrollAndJamCount = C; }

  /// Set the pipeline disabled state.
  void setPipelineDisabled(bool S) { StagedAttrs.PipelineDisabled = S; }

  /// Set the pipeline initiation interval.
  void setPipelineInitiationInterval(unsigned C) {
    StagedAttrs.PipelineInitiationInterval = C;
  }

private:
  /// Returns true if there is LoopInfo on the stack.
  bool hasInfo() const { return !Active.empty(); }
  /// Return the LoopInfo for the current loop. HasInfo should be called
  /// first to ensure LoopInfo is present.
  const LoopInfo &getInfo() const { return Active.back(); }
  /// The set of attributes that will be applied to the next pushed loop.
  LoopAttributes StagedAttrs;
  /// Stack of active loops.
  llvm::SmallVector<LoopInfo, 4> Active;
};

} // end namespace CodeGen
} // end namespace clang

#endif
