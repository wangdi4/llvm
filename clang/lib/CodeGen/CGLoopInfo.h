//===---- CGLoopInfo.h - LLVM CodeGen for loop metadata -*- C++ -*---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
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

  /// Value for llvm.loop.vectorize.ivdep_back metadata.
  bool IVDepEnable;

  /// Value for llvm.loop.ivdep.enable metadata.
  bool IVDepHLSEnable;

  /// Value for both  llvm.loop.vectorize.ivdep_back and
  /// llvm.loop.ivdep.enable metadata.
  bool IVDepHLSIntelEnable;

  /// Value for llvm.loop.ivdep.safelen metadata.
  unsigned IVDepCount;

  /// \brief Value for llvm.loop.fusion.* metadata.
  LVEnableState FusionEnable;

  /// \brief Value for llvm.loop.vectorize.ivdep_loop metadata.
  bool IVDepLoop;

  /// \brief Value for llvm.loop.vectorize.ivdep_back metadata.
  bool IVDepBack;

  /// \brief Value for llvm.loop.vector_always.enable metadata.
  bool VectorizeAlwaysEnable;
#endif // INTEL_CUSTOMIZATION

  /// Value for llvm.loop.unroll.* metadata (enable, disable, or full).
  LVEnableState UnrollEnable;

  /// Value for llvm.loop.unroll_and_jam.* metadata (enable, disable, or full).
  LVEnableState UnrollAndJamEnable;

  /// Value for llvm.loop.vectorize.width metadata.
  unsigned VectorizeWidth;

  /// Value for llvm.loop.interleave.count metadata.
  unsigned InterleaveCount;

  /// llvm.unroll.
  unsigned UnrollCount;

  /// llvm.unroll.
  unsigned UnrollAndJamCount;

  /// Value for llvm.loop.distribute.enable metadata.
  LVEnableState DistributeEnable;
};

/// Information used when generating a structured loop.
class LoopInfo {
public:
  /// Construct a new LoopInfo for the loop with entry Header.
  LoopInfo(llvm::BasicBlock *Header, const LoopAttributes &Attrs,
           const llvm::DebugLoc &StartLoc, const llvm::DebugLoc &EndLoc);

#if INTEL_CUSTOMIZATION
  /// Construct a new LoopInfo with a given loop id metadata.
  LoopInfo(llvm::MDNode *LoopID, const LoopAttributes &Attrs);
#endif  // INTEL_CUSTOMIZATION

  /// Get the loop id metadata for this loop.
  llvm::MDNode *getLoopID() const { return LoopID; }

  /// Get the header block of this loop.
  llvm::BasicBlock *getHeader() const { return Header; }

  /// Get the set of attributes active for this loop.
  const LoopAttributes &getAttributes() const { return Attrs; }

private:
  /// Loop ID metadata.
  llvm::MDNode *LoopID;
  /// Header block of this loop.
  llvm::BasicBlock *Header;
  /// The attributes for this loop.
  LoopAttributes Attrs;
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

#if INTEL_CUSTOMIZATION
  /// Extend the code region as part of a parallel loop which might be
  /// inside another llvm function.
  void push(llvm::MDNode *LoopID, bool IsParallel);
#endif  // INTEL_CUSTOMIZATION

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

  /// Set flag for three types of plain #pragma ivdep
  void setIVDepEnable() { StagedAttrs.IVDepEnable = true; }
  void setIVDepHLSEnable() { StagedAttrs.IVDepHLSEnable = true; }
  void setIVDepHLSIntelEnable() { StagedAttrs.IVDepHLSIntelEnable = true; }

  /// Set the safelen count for the next loop pushed.
  void setIVDepCount(unsigned C) { StagedAttrs.IVDepCount = C; }

  /// \brief Set the next pushed loop 'fusion.enable'
  void setFusionEnable(bool Enable = true) {
    StagedAttrs.FusionEnable =
        Enable ? LoopAttributes::Enable : LoopAttributes::Disable;
  }

  /// \brief Set the loop flag for ivdep.
  void setIVDepLoop() { StagedAttrs.IVDepLoop = true; }

  /// \brief Set the back flag for ivdep.
  void setIVDepBack() { StagedAttrs.IVDepBack = true; }
  /// \brief Set next pushed loop  'vector_always.enable'
  void setVectorizeAlwaysEnable() {
    StagedAttrs.VectorizeAlwaysEnable = true;
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

  /// Set the next pushed loop unroll_and_jam state.
  void setUnrollAndJamState(const LoopAttributes::LVEnableState &State) {
    StagedAttrs.UnrollAndJamEnable = State;
  }

  /// Set the vectorize width for the next loop pushed.
  void setVectorizeWidth(unsigned W) { StagedAttrs.VectorizeWidth = W; }

  /// Set the interleave count for the next loop pushed.
  void setInterleaveCount(unsigned C) { StagedAttrs.InterleaveCount = C; }

  /// Set the unroll count for the next loop pushed.
  void setUnrollCount(unsigned C) { StagedAttrs.UnrollCount = C; }

  /// \brief Set the unroll count for the next loop pushed.
  void setUnrollAndJamCount(unsigned C) { StagedAttrs.UnrollAndJamCount = C; }

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
