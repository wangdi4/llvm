//=------------ SGLoopConstruct.h - Create subgroup loop - C++ -*------------=//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_SUBGROUP_EMULATION_LOOP_CONSTRUCT_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_SUBGROUP_EMULATION_LOOP_CONSTRUCT_H

#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/SYCLTransforms/SubgroupEmulation/SGHelper.h"
#include "llvm/Transforms/SYCLTransforms/SubgroupEmulation/SGSizeAnalysis.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"

#include <tuple>

namespace llvm {
// TODO: the sub-group emulation passes should be decoupled with barrier passes.
class SGLoopConstructPass : public PassInfoMixin<SGLoopConstructPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  /// Glue for old PM.
  bool runImpl(Module &M, const SGSizeInfo *SSI);

private:
  /// TODO: Refine name
  /// Split the Barrier BB to create the existing block for previous loop.
  /// Assign an unique ID for every sync instruction.
  void collectSyncInsts();

  /// For every sub_group_barrier, find all sync instructions which can reach
  /// here.
  CompilationUtils::InstSet findSyncPredecessors(Instruction *I);

  /// Find the sg loop header for every sub_group_barrier, and create subgroup
  /// loop.
  void createSGLoop();

  /// Replace get_local_id(vec_dim) with get_local_id(vec_dim) +
  /// get_sub_group_local_id; Replace get_global_id(vec_dim) with
  /// get_global_id(vec_dim) + get_sub_group_local_id
  void updateTIDCalls(Module &M);

  /// Hoist get_sub_group_local_id to an emulated function.
  void hoistSGLIdCalls(Module &M);

  /// Replace get_sub_group_local_id with sg loop control variable.
  void resolveSGLIdCalls(Module &M);

  /// Set vectorized_width as sg_emu_size for emulated kernels.
  void updateMetadata(Module &M);

  /// Map from function to all sub_group_barrier calls inside.
  MapVector<Function *, CompilationUtils::InstSet> FuncToSGBarriers;

  /// Map from function to Alloca for sub-group local id and sub-group loop
  /// source.
  MapVector<Function *, std::tuple<AllocaInst *, AllocaInst *>> FuncToLoopStuff;

  /// Map from sub_group_barrier call to  sub_group_barrier / dummy_sg_barrier
  /// calls may reach it.
  MapVector<Instruction *, CompilationUtils::InstSet> BarrierToJumpTargets;

  /// Map from sync instruction to a unique ID.
  MapVector<Instruction *, unsigned> SyncInstToUniqueID;

  MapVector<BasicBlock *, BasicBlock *> LoopHeaderToPrevExiting;

  CompilationUtils::FuncSet FunctionsNeedEmulation;

  SGHelper Helper;
  const SGSizeInfo *SizeInfo = nullptr;
};
} // namespace llvm
#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_SUBGROUP_EMULATION_LOOP_CONSTRUCT_H
