//===------------------- SGValueWiden.h - Widen values -------------------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===---------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_SG_EMULATION_SG_VALUE_WIDEN_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_SG_EMULATION_SG_VALUE_WIDEN_H

#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/SYCLTransforms/SubgroupEmulation/SGHelper.h"
#include "llvm/Transforms/SYCLTransforms/Utils/BarrierUtils.h"

namespace llvm {
class DominatorTree;

/// 1. Widen the prototype for functions to be emulated except for kernels. This
///    makes the function looks like being vectorized. All sub-group built-in
///    declarations are also processed in this phase. This phase looks like
///    VecClone, the main difference is we don't create the SIMD Loop here and
///    just widen the parameters / return value and then update their uses /
///    def.
/// 2. Alloca an array / vector for non-uniform value crossed by
///    sub_group_barrier.
/// 3. Alloca a scalar counterpart for uniform value crossed by
///    sub_group_barrier.
/// 4. Replace original use with the value loaded from corresponding alloca
///    instruction; Store the def to corresponding alloca instruction.
/// 5. For widened functions, replace their orignal scalar calls with widened
///    calls.
/// TODO:
/// 1. Complete the logic handling attributes for vector parameters.
/// 2. Improve WIRelatedAnalysis pass to make it suitable for subgroup
///    emulation.
/// 3. Improve the logic handling <VF x iN> while N is not power of 2.
///    This was partly done by promoting <VF x iN> to <VF x iM> where M is a
///    multiple of 8. Could be improved further -- avoid generating GEP
///    instructions into vector elements -- use load/store and
///    extractelement/insertelement instead.
/// 4. Improve / fix the logic handling <C x Ty> where C is not power of 2.
/// 5. Fix debug info for parameters, llvm.dbg.value intrinsics.
class SGValueWidenPass : public PassInfoMixin<SGValueWidenPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

private:
  SGHelper Helper;

  BarrierUtils Utils;

  Value *ConstZero = nullptr;

  void runOnFunction(Function &F, const unsigned &Sizes, DominatorTree &DT);

  /// Alloca vector / array for original "scalar" alloca instruction.
  void widenAlloca(Instruction *V, Instruction *FirstI, unsigned Size,
                   DominatorTree &DT);

  /// Alloca vector for values whose type is int / float / pointer.
  /// Alloca array for values with other types.
  /// Fill VecValueMap.
  void widenValue(Instruction *V, Instruction *FirstI, unsigned Size,
                  DominatorTree &DT);

  /// If the value is uniform in sub-group, there is no need to alloca
  /// a vector /array counterpart. We can just alloca a scalar counterpart
  /// for it.
  void hoistUniformValue(Instruction *V, Instruction *FirstI,
                         DominatorTree &DT);

  /// Collect all calls to be widned.
  void collectWideCalls(Module &M);

  /// Vectorize calls which call emulated functions or sub-group built-ins.
  void widenCalls();

  bool isWideCall(Value *V) {
    if (auto *CI = dyn_cast<CallInst>(V))
      return WideCalls.count(CI);
    return false;
  }

  /// Get the widend value for V.
  Value *getVectorValue(Value *V, unsigned Size, Instruction *IP);

  /// Get the pointer of widend value for V.
  Value *getVectorValuePtr(Value *V, unsigned Size, Instruction *IP);

  /// Get the scalar value for V.
  /// This will be used when the VectorKind is uniform.
  Value *getScalarValue(Value *V, Instruction *IP);

  /// Set the data as widend value for V.
  void setVectorValue(Value *Data, Value *V, unsigned Size, Instruction *IP);

  /// Check whether Def I is cross by sub_group_barrier / dummy_sg_barrier.
  bool isCrossBarrier(Instruction *I,
                      const CompilationUtils::InstSet &SyncInsts) const;

  /// Chekc if V is uniform in sub-group.
  bool isWIRelated(Value *V);

  /// Get the insert point for I when processing operand V.
  Instruction *getInsertPoint(Instruction *I, Value *V, DominatorTree &DT);

  /// Get current address of Val for U.
  Value *getWIOffset(Instruction *U, Value *Val);
  /// Get current value of Val for U.
  Value *getWIValue(Instruction *U, Value *Val, DominatorTree &DT);
  /// Set current value of Val.
  void setWIValue(Value *Val);

  /// Calls to be widened.
  CompilationUtils::InstSet WideCalls;

  /// All functions need to be widened.
  CompilationUtils::FuncSet FunctionsToBeWidened;

  /// Map from Function to Widend Function.
  DenseMap<Function *, std::set<Function *>> FuncMap;

  /// BB will be excluded from work-group loop.
  DenseMap<Function *, BasicBlock *> WGExcludeBBMap;
  /// BB will be excluded from sub-group loop.
  DenseMap<Function *, BasicBlock *> SGExcludeBBMap;

  /// Map from value to its widened alloca instruction.
  DenseMap<Value *, Value *> VecValueMap;

  /// Map from value to its widened argument.
  DenseMap<Value *, Value *> VecArgMap;

  /// Map from value to its hoisted alloca instruction.
  DenseMap<Value *, Value *> UniValueMap;

  /// Map from alloca for debug to the widened alloca instruction.
  DenseMap<AllocaInst *, AllocaInst *> DebugAIMap;

  /// Instructions to be removed.
  SmallVector<Instruction *, 64> InstsToBeRemoved;
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_SG_EMULATION_SG_VALUE_WIDEN_H
