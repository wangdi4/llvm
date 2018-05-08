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

/// \brief Attributes that may be specified on loops.
struct LoopAttributes {
  explicit LoopAttributes(bool IsParallel = false);
  void clear();

  /// \brief Generate llvm.loop.parallel metadata for loads and stores.
  bool IsParallel;

  /// \brief State of loop vectorization or unrolling.
  enum LVEnableState { Unspecified, Enable, Disable, Full };

#if INTEL_CUSTOMIZATION
  /// \brief Value for llvm.loop.coalesce.enable metadata.
  LVEnableState LoopCoalesceEnable;

  /// \brief Value for llvm.loop.coalesce.count metadata.
  unsigned LoopCoalesceCount;

  /// \brief Value for llvm.loop.ii.count metadata.
  unsigned IICount;

  /// \brief Value for llvm.loop.max_concurrency.count metadata.
  unsigned MaxConcurrencyCount;

  /// \brief Value for llvm.loop.ivdep.enable metadata.
  LVEnableState IVDepEnable;

  /// \brief Value for llvm.loop.ivdep.safelen metadata.
  unsigned IVDepCount;

  /// \brief Value for llvm.loop.nofusion.enable metadata.
  LVEnableState NoFusionEnable;
#endif // INTEL_CUSTOMIZATION

  /// \brief Value for llvm.loop.vectorize.enable metadata.
  LVEnableState VectorizeEnable;

  /// \brief Value for llvm.loop.unroll.* metadata (enable, disable, or full).
  LVEnableState UnrollEnable;

  /// \brief Value for llvm.loop.vectorize.width metadata.
  unsigned VectorizeWidth;

  /// \brief Value for llvm.loop.interleave.count metadata.
  unsigned InterleaveCount;

  /// \brief llvm.unroll.
  unsigned UnrollCount;

  /// \brief Value for llvm.loop.distribute.enable metadata.
  LVEnableState DistributeEnable;
};

/// \brief Information used when generating a structured loop.
class LoopInfo {
public:
  /// \brief Construct a new LoopInfo for the loop with entry Header.
  LoopInfo(llvm::BasicBlock *Header, const LoopAttributes &Attrs,
           const llvm::DebugLoc &StartLoc, const llvm::DebugLoc &EndLoc);
#if INTEL_CUSTOMIZATION
  /// \brief Construct a new LoopInfo with a given loop id metadata.
  LoopInfo(llvm::MDNode *LoopID, const LoopAttributes &Attrs);
#endif  // INTEL_CUSTOMIZATION
  /// \brief Get the loop id metadata for this loop.
  llvm::MDNode *getLoopID() const { return LoopID; }

  /// \brief Get the header block of this loop.
  llvm::BasicBlock *getHeader() const { return Header; }

  /// \brief Get the set of attributes active for this loop.
  const LoopAttributes &getAttributes() const { return Attrs; }

private:
  /// \brief Loop ID metadata.
  llvm::MDNode *LoopID;
  /// \brief Header block of this loop.
  llvm::BasicBlock *Header;
  /// \brief The attributes for this loop.
  LoopAttributes Attrs;
};

/// \brief A stack of loop information corresponding to loop nesting levels.
/// This stack can be used to prepare attributes which are applied when a loop
/// is emitted.
class LoopInfoStack {
  LoopInfoStack(const LoopInfoStack &) = delete;
  void operator=(const LoopInfoStack &) = delete;

public:
  LoopInfoStack() {}

  /// \brief Begin a new structured loop. The set of staged attributes will be
  /// applied to the loop and then cleared.
  void push(llvm::BasicBlock *Header, const llvm::DebugLoc &StartLoc,
            const llvm::DebugLoc &EndLoc);

#if INTEL_CUSTOMIZATION
  /// \brief Extend the code region as part of a parallel loop which might be
  /// inside another llvm function.
  void push(llvm::MDNode *LoopID, bool IsParallel);
#endif  // INTEL_CUSTOMIZATION

  /// \brief Begin a new structured loop. Stage attributes from the Attrs list.
  /// The staged attributes are applied to the loop and then cleared.
  void push(llvm::BasicBlock *Header, clang::ASTContext &Ctx,
            llvm::ArrayRef<const Attr *> Attrs, const llvm::DebugLoc &StartLoc,
            const llvm::DebugLoc &EndLoc);

  /// \brief End the current loop.
  void pop();

  /// \brief Return the top loop id metadata.
  llvm::MDNode *getCurLoopID() const {                     // INTEL
    return hasInfo() ? getInfo().getLoopID() : nullptr;    // INTEL
  }                                                        // INTEL

  /// \brief Return true if the top loop is parallel.
  bool getCurLoopParallel() const {
    return hasInfo() ? getInfo().getAttributes().IsParallel : false;
  }

  /// \brief Function called by the CodeGenFunction when an instruction is
  /// created.
  void InsertHelper(llvm::Instruction *I) const;

  /// \brief Set the next pushed loop as parallel.
  void setParallel(bool Enable = true) { StagedAttrs.IsParallel = Enable; }

#if INTEL_CUSTOMIZATION
  /// \brief Set the next pushed loop 'coalesce.enable'
  void setLoopCoalesceEnable() {
    StagedAttrs.LoopCoalesceEnable = LoopAttributes::Enable;
  }

  /// \brief Set the coalesce count for the next loop pushed.
  void setLoopCoalesceCount(unsigned C) { StagedAttrs.LoopCoalesceCount = C; }

  /// \brief Set the ii count for the next loop pushed.
  void setIICount(unsigned C) { StagedAttrs.IICount = C; }

  /// \brief Set the max_concurrency count for the next loop pushed.
  void setMaxConcurrencyCount(unsigned C) {
    StagedAttrs.MaxConcurrencyCount = C;
  }

  /// \brief Set the next pushed loop 'ivdep.enable'
  void setIVDepEnable() { StagedAttrs.IVDepEnable = LoopAttributes::Enable; }

  /// \brief Set the safelen count for the next loop pushed.
  void setIVDepCount(unsigned C) { StagedAttrs.IVDepCount = C; }

  /// \brief Set the next pushed loop 'nofusion.enable'
  void setNoFusionEnable() {
    StagedAttrs.NoFusionEnable = LoopAttributes::Enable;
  }
#endif // INTEL_CUSTOMIZATION

  /// \brief Set the next pushed loop 'vectorize.enable'
  void setVectorizeEnable(bool Enable = true) {
    StagedAttrs.VectorizeEnable =
        Enable ? LoopAttributes::Enable : LoopAttributes::Disable;
  }

  /// \brief Set the next pushed loop as a distribution candidate.
  void setDistributeState(bool Enable = true) {
    StagedAttrs.DistributeEnable =
        Enable ? LoopAttributes::Enable : LoopAttributes::Disable;
  }

  /// \brief Set the next pushed loop unroll state.
  void setUnrollState(const LoopAttributes::LVEnableState &State) {
    StagedAttrs.UnrollEnable = State;
  }

  /// \brief Set the vectorize width for the next loop pushed.
  void setVectorizeWidth(unsigned W) { StagedAttrs.VectorizeWidth = W; }

  /// \brief Set the interleave count for the next loop pushed.
  void setInterleaveCount(unsigned C) { StagedAttrs.InterleaveCount = C; }

  /// \brief Set the unroll count for the next loop pushed.
  void setUnrollCount(unsigned C) { StagedAttrs.UnrollCount = C; }

private:
  /// \brief Returns true if there is LoopInfo on the stack.
  bool hasInfo() const { return !Active.empty(); }
  /// \brief Return the LoopInfo for the current loop. HasInfo should be called
  /// first to ensure LoopInfo is present.
  const LoopInfo &getInfo() const { return Active.back(); }
  /// \brief The set of attributes that will be applied to the next pushed loop.
  LoopAttributes StagedAttrs;
  /// \brief Stack of active loops.
  llvm::SmallVector<LoopInfo, 4> Active;
};

} // end namespace CodeGen
} // end namespace clang

#endif
