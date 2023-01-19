//===- ImplicitArgsAnalysis.h - DPC++ kernel implicit argument analysis ---===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_IMPLICIT_ARGS_ANALYSIS_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_IMPLICIT_ARGS_ANALYSIS_H

#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

/// Provide implicit arguments info.
class ImplicitArgsInfo {
public:
  ImplicitArgsInfo(Module &M);
  ~ImplicitArgsInfo();

  /// Returns the type of the ID implicit argument
  Type *getArgType(unsigned ID) { return ArgTypes[ID]; }

  const Type *getArgType(unsigned ID) const { return ArgTypes[ID]; }

  /// Returns the type of the ID member of WORK_GROUP_INFO implicit argument
  /// (which is a struct).
  Type *getWorkGroupInfoMemberType(unsigned ID) {
    return WGInfoMembersTypes[ID];
  }

  const Type *getWorkGroupInfoMemberType(unsigned ID) const {
    return WGInfoMembersTypes[ID];
  }

  Value *GenerateGetFromWorkInfo(unsigned RecordID, Value *WorkInfo,
                                 unsigned Dimension, IRBuilder<> &Builder);
  Value *GenerateGetFromWorkInfo(unsigned RecordID, Value *WorkInfo,
                                 Value *Dimension, IRBuilder<> &Builder);
  Value *GenerateGetFromWorkInfo(unsigned RecordID, Value *WorkInfo,
                                 IRBuilder<> &Builder);
  Value *GenerateGetGlobalOffset(Value *WorkInfo, unsigned Dimension,
                                 IRBuilder<> &Builder);
  Value *GenerateGetGlobalOffset(Value *WorkInfo, Value *Dimension,
                                 IRBuilder<> &Builder);

  Value *GenerateGetLocalSize(bool uniformWGSize, Value *WorkInfo, Value *pWGId,
                              bool IsUserRequired, Value *Dimension,
                              IRBuilder<> &Builder);

  Value *GenerateGetEnqueuedLocalSize(Value *WorkInfo, bool IsUserRequired,
                                      unsigned Dimension, IRBuilder<> &Builder);

  Value *GenerateGetEnqueuedLocalSize(Value *WorkInfo, bool IsUserRequired,
                                      Value *Dimension, IRBuilder<> &Builder);

  Value *GenerateGetGroupID(Value *GroupID, unsigned Dimension,
                            IRBuilder<> &Builder);
  Value *GenerateGetGroupID(Value *GroupID, Value *Dimension,
                            IRBuilder<> &Builder);

  Value *GenerateGetBaseGlobalID(Value *BaseGlobalID, Value *Dimension,
                                 IRBuilder<> &Builder);

private:
  /// Initialize this class.
  void init(LLVMContext *C, unsigned PointerSizeInBits);

  /// The following implementation is generic for get_local_size and
  /// get_enqueued_local_size.
  Value *GenerateGetLocalSizeGeneric(Value *WorkInfo, bool IsUserRequired,
                                     Value *LocalSizeIdx, Value *Dimension,
                                     IRBuilder<> &Builder);

private:
  // Each entry matches an IMPLICIT_ARGS enum
  SmallVector<Type *, 16> ArgTypes;
  // Breakdown of WORK_GROUP_INFO which is a structure
  SmallVector<Type *, 16> WGInfoMembersTypes;
  LLVMContext *C;
  Module *M;
  unsigned InitializedTo;
};

// Provide implicit arguments info.
class ImplicitArgsAnalysis : public AnalysisInfoMixin<ImplicitArgsAnalysis> {
  static AnalysisKey Key;
  friend AnalysisInfoMixin<ImplicitArgsAnalysis>;
  static char PassID;

public:
  typedef ImplicitArgsInfo Result;

  ImplicitArgsInfo run(Module &M, AnalysisManager<Module> &AM);
};

} // namespace llvm
#endif
