//===-- CSAMemopOrderingBase.h - Common memop ordering base -----*- C++ -*-===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
///===---------------------------------------------------------------------===//
/// \file
///
/// This file contains a base class for CSA memop ordering passes.
///
///===---------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CSA_MEMOPORDERING_MEMOPORDERINGBASE_H
#define LLVM_LIB_TARGET_CSA_MEMOPORDERING_MEMOPORDERINGBASE_H

#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

/// A base class for CSA memop ordering passes.
///
/// This class implements common memory ordering functionality such as
/// determining which instructions need ordering edges, how to add ordering
/// edges, and memory ordering boilerplate such as adding the mementry
/// instruction and cleaning up annotation intrinsics. Subclasses should
/// override the order method, getPassName, and possibly getAnalysisUsage if
/// more analysis passes are needed.
class CSAMemopOrderingBase : public FunctionPass {
public:
  void getAnalysisUsage(AnalysisUsage &) const override;
  bool runOnFunction(Function &) override final;

protected:
  /// Alias analysis results for the current function being ordered.
  AAResults *AA;

  /// The mementry intrinsic added to this function before order is called.
  Value *MemEntry;

  /// The value to use for <none>.
  Value *NoneVal;

  /// A protected constructor so that derived types can pass their ID through to
  /// the FunctionPass constructor.
  CSAMemopOrderingBase(char &ID) : FunctionPass{ID} {}

  /// Determines whether an instruction needs to have ordering edges added to
  /// it.
  ///
  /// The criteria for this is based on logic in instruction selection that
  /// checks for edges when they are needed and leaves stray edges (that can't
  /// be lowered and result in errors) when they aren't. This ensures that no
  /// memory operations that actually do need to be ordered are silently
  /// separated from their inord/outord intrinsic calls.
  bool needsOrderingEdges(Instruction &) const;

  /// Adds a phi node of the right type for memop ordering edges.
  ///
  /// The phi node returned will be empty; call addIncomingValue to populate it.
  ///
  /// \param BB The basic block to put the phi node in.
  /// \param Name If provided, the name to use for the phi node's value.
  PHINode *createPHI(BasicBlock *BB, const Twine &Name = "memop.p");

  /// Adds inord/outord calls to an instruction.
  ///
  /// \param I The instruction to add edges to.
  /// \param InOrd The input ordering edge to use in the inord call.
  /// \param Name If provided, the name to use for the outord value.
  /// \result The output ordering edge produced by the outord call, or nullptr
  /// if I is a return and shouldn't have an outord.
  Value *createOrderingEdges(Instruction *I, Value *InOrd,
                             const Twine &Name = "memop.o");

  /// Adds an all0 dependency token merge.
  ///
  /// \param Inputs The inputs to merge.
  /// \param Where The instruction to put the merge ahead of.
  /// \param Name If provided, the name to use for the merge result.
  /// \result The result of the merge.
  Value *createAll0(ArrayRef<Value *> Inputs, Instruction *Where,
                    const Twine &Name = "memop.m");

  /// Adds ordering edges to all memory operations in a function to satisfy
  /// ordering requirements.
  virtual void order(Function &) = 0;

private:
  /// Pre-loaded declarations and types for ordering-related instructions.
  /// @{
  Function *InordIntr;
  Function *OutordIntr;
  Function *All0Intr;
  Type *MemordType;

  Function *PrefetchIntr;
  /// @}

  /// Deletes parallel region/section intrinsics from a function.
  void deleteParallelIntrinsics(Function &);

  /// Converts a value to the memory ordering token type by adding bitcast/trunc
  /// instructions ahead of Where in order to satisfy LLVM's type system. This
  /// is used in expandDataGatedPrefetches.
  Value *toMemOrdValue(Value *, Instruction *Where);

  /// Expands data-gated prefetch intrinsics into prefetches ordered off of data
  /// values.
  void expandDataGatedPrefetches(Function &);
};

} // namespace llvm

#endif
