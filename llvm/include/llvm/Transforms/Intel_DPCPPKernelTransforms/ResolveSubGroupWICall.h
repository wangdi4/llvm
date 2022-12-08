//=- ResolveSubGroupWICall.h - Resolve DPC++ kernel subgroup work-item call -=//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef INTEL_DPCPP_KERNEL_TRANSFORMS_RESOLVE_SUBGROUP_WI_CALL_H
#define INTEL_DPCPP_KERNEL_TRANSFORMS_RESOLVE_SUBGROUP_WI_CALL_H

#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

namespace llvm {

class BuiltinLibInfo;
class RuntimeService;

class ResolveSubGroupWICallPass
    : public PassInfoMixin<ResolveSubGroupWICallPass> {
public:
  ResolveSubGroupWICallPass(bool ResolveSGBarrier = true);

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  // Glue for old PM.
  bool runImpl(Module &M, BuiltinLibInfo *BLI);

  static bool isRequired() { return true; }

private:
  Value *replaceGetSubGroupSize(Instruction *insertBefore, Value *VF,
                                int32_t VD);
  Value *replaceGetMaxSubGroupSize(Instruction *insertBefore, Value *VF,
                                   int32_t VD);

  Value *replaceGetEnqueuedNumSubGroups(Instruction *insertBefore, Value *VF,
                                        int32_t VD);
  Value *replaceGetNumSubGroups(Instruction *insertBefore, Value *VF,
                                int32_t VD);
  Value *replaceSubGroupBarrier(Instruction *insertBefore, Value *VF,
                                int32_t VD);

  Value *replaceGetSubGroupId(Instruction *insertBefore, Value *VF, int32_t VD);

  // Resolves
  // call i64 @get_sub_group_slice_length.(i32 %total.element.count)
  // -->
  // constant: ceil(%total.element.count / VF)
  Value *replaceGetSubGroupSliceLength(Instruction *InsertBefore, Value *VF,
                                       int32_t VD);

  // Resolves the sequence
  // %id = call i64 @get_sub_group_rowslice_id(<N x T> %mat, i32 immarg R, i32
  //   immarg C, <IntType> %index)
  // %slice = call <VF x T> @_ZGVbN<VF>u_sub_group_rowslice_extractelement(i64
  //   %id)
  // -->
  // %baseid = mul i32 %index, VF
  // %row.index = udiv i32 %baseid, C
  // %col.index = urem i32 %baseid, C
  // %slice = call <VF x T> @llvm.experimental.matrix.extract.row.slice(<N x T>
  //   %mat, i32 %row.index, i32 %col.index, i32 immarg VF, i32 immarg R, i32
  //   immarg C)
  //
  // Or the scalar situation (VF == 1):
  // %id = call i64 @get_sub_group_rowslice_id(<N x T> %mat, i32 immarg R, i32
  //   immarg C, <IntType> %index)
  // %slice = call T @sub_group_rowslice_extractelement(i64 %id)
  // -->
  // %baseid = mul i32 %index, 1
  // %row.index = udiv i32 %baseid, C
  // %col.index = urem i32 %baseid, C
  // %rowslice = call <1 x T> @llvm.experimental.matrix.extract.row.slice(<N x
  //   T> %mat, i32 %row.index, i32 %col.index, i32 immarg 1, i32 immarg R, i32
  //   immarg C)
  // %slice = extractelement <1 x T> %rowslice, i32 0
  Value *replaceSubGroupRowSliceExtractElement(Instruction *InsertBefore,
                                               Value *VF, int32_t VD);

  // Resolves the sequence
  // %id = call i64 @get_sub_group_rowslice_id(<N x T> %mat, i32 immarg R, i32
  //   immarg C, <IntType> %index)
  // call void @_ZGVbN<VF>uv_sub_group_rowslice_insertelement(i64 %id,
  //   <VF x T> %data)
  // %mat.update = call <N x T> @sub_group_insert_rowslice_to_matrix(i64 %id)
  // -->
  // %baseid = mul i32 %index, VF
  // %row.index = udiv i32 %baseid, C
  // %col.index = urem i32 %baseid, C
  // %mat.update = call <N x T> @llvm.experimental.matrix.insert.row.slice(
  //   <VF x T> %data, <N x T> %mat, i32 %row.index, i32 %col.index, i32 immarg
  //   VF, i32 immarg R, i32 immarg C)
  //
  // Or the scalar situation (VF == 1):
  // %id = call i64 @get_sub_group_rowslice_id(<N x T> %mat, i32 immarg R, i32
  //   immarg C, <IntType> %index)
  // call void @sub_group_rowslice_insertelement(i64 %id, T %data)
  // %mat.update = call <N x T> @sub_group_insert_rowslice_to_matrix(i64 %id)
  // -->
  // %baseid = mul i32 %index, VF
  // %row.index = udiv i32 %baseid, C
  // %col.index = urem i32 %baseid, C
  // %rowslice = insertelement <1 x T> undef, T %data, i32 0
  // %mat.update = call <N x T> @llvm.experimental.matrix.insert.row.slice(
  //   <1 x T> %rowslice, <N x T> %mat, i32 %row.index, i32 %col.index, i32
  //   immarg VF, i32 immarg R, i32 immarg C)
  Value *replaceSubGroupInsertRowSliceToMatrix(Instruction *InsertBefore,
                                               Value *VF, int32_t VD);

  // Helpers:
  CallInst *createWIFunctionCall(Module *M, char const *twine,
                                 std::string const &name,
                                 Instruction *insertBefore, Value *actPar);

  ConstantInt *createVFConstant(LLVMContext &, const DataLayout &, size_t VF);

  // Parses info from the `RowSliceId`, which might be an
  // get_sub_group_rowslice_id call itself, or a PHI node selecting from the
  // call.
  // By appending `%mat`, `%row.index`, `%col.index`, `i32 immarg VF`,
  // `i32 immarg R`, `i32 immarg C` to ParsedArgs. The values are created on
  // demand via Builder.
  // If `RowSliceId` is a PHI node, the PHI node and the corresponding call inst
  // are added to `ExtraInstToRemove`;
  // Otherwise (`RowSliceId` is already an call inst), the call inst will be added
  // to `ExtraInstToRemove`.
  void resolveGetSubGroupRowSliceId(Value *RowSliceId, unsigned RowSliceLength,
                                    IRBuilder<> &Builder,
                                    SmallVectorImpl<Value *> &ParsedArgs);

  // This pass will run twice, the first time is after vectorizer and before
  // WGLoopCreater / Barrier, all sub-group WI built-ins except sub_group
  // barrier will be resolved. The second time is after sub-group emulation
  // passes, all sub-group barriers and get_sub_group_size introduced by
  // emulation passes will be resolved. The flag is used to control whether
  // sub-group barrier should be resolved. We cannot resolve sub-group barriers
  // at the first time, since sub-group barriers in kernels to be emulated
  // must be visible to emulation passes.
  bool ResolveSGBarrier;

  RuntimeService *RTS = nullptr;

  SmallVector<Instruction *, 8> ExtraInstToRemove;
};
} // namespace llvm

#endif
