//==--- BarrierPass.h - Main Barrier pass - C++ -*--------------------------==//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_BARRIER_PASS_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_BARRIER_PASS_H

#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelBarrierUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DataPerBarrierPass.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DataPerValuePass.h"

#include <map>

namespace llvm {

/// Barrier pass is a module pass that handles
/// Special values
///   Group-A   : Alloca instructions
///   Group-B.1 : Values crossed barriers and the value is
///              related to WI-Id or initialized inside a loop
///   Group-B.2 : Value crossed barrier but does not suit Group-B.2
/// Synchronize instructions
///   barrier(), fiber() and dummyBarrier() instructions.
/// Get LID/GID instructions
///   get_local_id() will be replaced with get_new_local_id()
///   get_global_id() will be replaced with get_new_global_id()
/// Non Inlined Internal Function
///   module functions with barriers that are called from inside the module
class KernelBarrier : public ModulePass {

public:
  using MapFunctionNameToBufferStrideTy = std::map<std::string, unsigned int>;
  using BasicBlockToBasicBlockTy = DenseMap<BasicBlock *, BasicBlock *>;
  using BasicBlockToBasicBlockSetTy = DenseMap<BasicBlock *, BasicBlockSet>;
  using BasicBlockToBasicBlockVectorTy =
      DenseMap<BasicBlock *, SmallVector<BasicBlock *, 8>>;
  using BasicBlockToInstructionMapVectorTy =
      MapVector<BasicBlock *, SmallVector<Instruction *, 8>>;

  static char ID;

  /// IsNativeDebug true if we are debugging natively (gdb).
  KernelBarrier(bool IsNativeDebug = false, bool useTLSGlobals = false);

  ~KernelBarrier() {}

  /// Provides name of pass.
  llvm::StringRef getPassName() const override {
    return "Intel Kernel Barrier";
  }

  /// Execute pass on given module.
  /// M module to optimize.
  /// Returns True if module was modified.
  bool runOnModule(Module &M) override;

  /// Inform about usage/mofication/dependency of this pass.
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DataPerBarrier>();
    AU.addRequired<DataPerValue>();
    AU.addRequired<DominatorTreeWrapperPass>();
  }

  /// Return special buffer stride size map.
  /// BufferStrideMap - the map to output all data into.
  void getStrideMap(std::map<std::string, unsigned int> &BufferStrideMap) {
    BufferStrideMap.clear();
    BufferStrideMap.insert(this->BufferStrideMap.begin(),
                           this->BufferStrideMap.end());
  }

private:
  /// Execute pass on given function.
  /// F function to optimize.
  /// Returns True if function was modified.
  virtual bool runOnFunction(Function &F);

  /// Use the stack for kernel function execution rather then the special
  /// work item buffer. This is needed for DWARF based debugging.
  /// Variable data will be copied out of a basic block from the special work
  /// item buffer up on entry. Variable data will be copied back out to the
  /// work item buffer upon leaving the basic block.
  /// InsertBeforeBegin instruction to insert copy out of stack before
  /// InsertBeforeEnd instruction to insert copy into stack before
  void useStackAsWorkspace(Instruction *InsertBeforeBegin,
                           Instruction *InsertBeforeEnd);

  /// Hanlde Values of Group-A of processed function.
  /// F Function to fix.
  void fixAllocaValues(Function &F);

  /// Hanlde Values of Group-B.1 of processed function.
  void fixSpecialValues();

  /// Hanlde Values of Group-B.2 of processed function.
  /// InsertBefore instruction to insert new alloca before.
  void fixCrossBarrierValues(Instruction *InsertBefore);

  /// Handle synchronize value of processed function.
  void replaceSyncInstructions();

  /// Initialize general values used to handle special values
  /// and synchronize instructions in the processed function
  /// Func function to create key values for.
  /// HasNoInternalCalls true if and only if processed function
  /// has no calls function inside the module.
  void createBarrierKeyValues(Function *Func, bool HasNoInternalCalls);

  /// Get general values used to handle special values
  /// and synchronize instructions in the processed function.
  /// Func function to get key values for.
  void getBarrierKeyValues(Function *Func);

  /// Calculate address in special buffer.
  /// Offset - offset of the address in the structure,
  /// PtrType - type of the address to calculate,
  /// InsertBefore - instruction to insert new instructions before,
  /// DB Debug location, NULL if not available.
  /// Returns value represnting the calculated address in the special buffer.
  Value *getAddressInSpecialBuffer(unsigned int Offset, PointerType *PtrTy,
                                   Instruction *InsertBefore,
                                   const DebugLoc *DB);

  /// Return instruction to insert new instruction before
  /// if UserInst is not a PHINode then return UserInst. Otherwise,
  /// return termenator of prevBB of UserInst with respect to Inst.
  /// Inst value that UserInst is using,
  /// UserInst instruction that is using Inst value,
  /// ExpectNULL true if allow returning NULL when,
  /// UserInst is a PHINode and BB(pInst) == PrevBB(pUserInst).
  /// Returns best instruction to insert new instruction before.
  Instruction *getInstructionToInsertBefore(Instruction *Inst,
                                            Instruction *UserInst,
                                            bool ExpectNULL);

  /// Fix get_local_id and get_global_id.
  /// M module to optimize.
  /// Returns True if module was modified.
  bool fixGetWIIdFunctions(Module &M);

  /// Create new fixed function with extra offset arguments.
  /// FuncToFix original function to clone.
  void fixNonInlineFunction(Function *FuncToFix);

  /// Fix usage of argument by loading it from special buffer
  /// if needed instead of reading it directly from the function arguments.
  /// OriginalArg original argument that need to be fix its usages,
  /// OffsetArg offset in special buffer to load the argument value from.
  void fixArgumentUsage(Value *OriginalArg, unsigned int OffsetArg);

  /// Fix return value by storing it to special buffer at given offset
  /// RetVal value to be saved, it is the function return value,
  /// OffsetRet offset in special buffer to save the return value at,
  /// InsertBefore new instructions will be added before this
  /// instruction.
  void fixReturnValue(Value *RetVal, unsigned int OffsetRet,
                      Instruction *InsertBefore);

  /// Handle parameters and return value of call instruction
  /// store relevent parametrs in sepcial buffer and load result
  /// from special buffer.
  /// OriginalCall original call instruction to handle.
  void fixCallInstruction(CallInst *OriginalCall);

  /// fixSynclessTIDUsers - Patch functions which are users of get_*_id() and do
  /// not produce the values within.
  void fixSynclessTIDUsers(Module &M, const FuncSet &);

  /// Remove all instructions in ToRemoveInstructions.
  void eraseAllToRemoveInstructions();

  /// Update Map with structure stride size for each kernel.
  /// M module to optimize.
  void updateStructureStride(Module &M, FuncSet &FnsWithSync);

  unsigned computeNumDim(Function *F);
  using BarrierBBIdListTy = std::vector<std::pair<ConstantInt *, BasicBlock *>>;
  BasicBlock *createLatchNesting(unsigned Dim, BasicBlock *Body,
                                 BasicBlock *Dispatch, Value *Step,
                                 const DebugLoc &DL);

  /// Return the innermost nested BB (where all the action happens).
  BasicBlock *createBarrierLatch(BasicBlock *pPreSyncBB, BasicBlock *pSyncBB,
                                 BarrierBBIdListTy &BBId, Value *UniqueID,
                                 const DebugLoc &DL);
  /// Below are generators which are used for accessing and setting the
  /// function's various values. Most rely on that CurrentBarrierKeyValues is
  /// set for the function being processed.
  Instruction *createGetCurrBarrierId(IRBuilder<> &B) {
    return B.CreateLoad(CurrentBarrierKeyValues->CurrBarrierId,
                        "CurrBarrierId");
  }
  Instruction *createSetCurrBarrierId(Value *V, IRBuilder<> &B) {
    return B.CreateStore(V, CurrentBarrierKeyValues->CurrBarrierId);
  }
  Instruction *createGetCurrSBIndex(IRBuilder<> &B) {
    return B.CreateLoad(CurrentBarrierKeyValues->CurrSBIndex, "SBIndex");
  }
  Instruction *createSetCurrSBIndex(Value *V, IRBuilder<> &B) {
    return B.CreateStore(V, CurrentBarrierKeyValues->CurrSBIndex);
  }
  Instruction *createGetLocalId(unsigned Dim, IRBuilder<> &B) {
    Value *Ptr = createGetPtrToLocalId(Dim);
    return B.CreateLoad(Ptr, AppendWithDimension("LocalId_", Dim));
  }
  Instruction *createGetLocalId(Value *LocalIdValues, Value *Dim,
                                IRBuilder<> &B) {
    Value *Ptr = createGetPtrToLocalId(LocalIdValues, Dim, B);
    return B.CreateLoad(Ptr, AppendWithDimension("LocalId_", Dim));
  }
  Instruction *createGetLocalId(Value *LocalIdValues, unsigned Dim,
                                IRBuilder<> &B) {
    Value *Ptr = createGetPtrToLocalId(
        LocalIdValues, ConstantInt::get(I32Ty, APInt(32, Dim)), B);
    return B.CreateLoad(Ptr, AppendWithDimension("LocalId_", Dim));
  }
  Instruction *createSetLocalId(unsigned Dim, Value *V, IRBuilder<> &B) {
    Value *Ptr = createGetPtrToLocalId(Dim);
    return B.CreateStore(V, Ptr);
  }
  Value *createGetPtrToLocalId(unsigned Dim) {
    // For accesses to constant dimensions, cache the GEP instruction
    Value **Ptr;
    if (UseTLSGlobals) {
      Ptr = PtrLocalId + Dim;
    } else {
      Ptr = CurrentBarrierKeyValues->PtrLocalId + Dim;
    }
    if (!*Ptr) {
      Function *F;
      if (UseTLSGlobals) {
        F = CurrentFunction;
      } else {
        F = CurrentBarrierKeyValues->TheFunction;
      }
      IRBuilder<> LB(F->getEntryBlock().getTerminator());
      Value *LocalIdValues;
      if (UseTLSGlobals) {
        LocalIdValues = LocalIds;
      } else {
        // If the LocalIDValues are generated externally to the function, make
        // sure we place the GEP before the value is accessed
        if (!isa<Instruction>(CurrentBarrierKeyValues->LocalIdValues))
          LB.SetInsertPoint(
              &*CurrentBarrierKeyValues->TheFunction->getEntryBlock().begin());
        LocalIdValues = CurrentBarrierKeyValues->LocalIdValues;
      }
      *Ptr = createGetPtrToLocalId(LocalIdValues,
                                   ConstantInt::get(I32Ty, APInt(32, Dim)), LB);
    }
    return *Ptr;
  }
  Value *createGetPtrToLocalId(Value *LocalIdValues, Value *Dim,
                               IRBuilder<> &B) {
    SmallVector<Value *, 4> Indices;
    Indices.push_back(ConstZero);
    Indices.push_back(Dim);
    return B.CreateInBoundsGEP(LocalIdValues, Indices,
                               AppendWithDimension("pLocalId_", Dim));
  }
  Value *getLocalSize(unsigned Dim) {
    return CurrentBarrierKeyValues->LocalSize[Dim];
  }
  void createDebugInstrumentation(BasicBlock *Then, BasicBlock *Else);
  Instruction *createOOBCheckGetLocalId(CallInst *Call);
  /// Emits code equivalent to get_local_id().
  /// Call - a call where first arg is dimension. New instructions are emitted
  /// before this instruction.
  Value *resolveGetLocalIDCall(CallInst *Call);
  unsigned getNumDims() const { return CurrentBarrierKeyValues->NumDims; }

  /// For each sync intruction in current function, find its parent
  /// basic and successors that also contains sync instruction.
  void findSyncBBSuccessors();

  /// Find the nearest SyncBB that dominates basic block BB.
  /// DT Dominator tree of current function.
  /// BB The basic block to process.
  /// Return The nearest SyncBB.
  BasicBlock *findNearestDominatorSyncBB(DominatorTree &DT, BasicBlock *BB);

  /// Bind AI's users to basic blocks so that a user will be replaced
  /// by value loaded from AI's new address alloca in its bound basic block.
  /// AI AllocaInst to process,
  /// DI DbgDeclareInst of AI,
  /// BBUsers Output binding map.
  void bindUsersToBasicBlock(AllocaInst *AI, DbgDeclareInst *DI,
                             BasicBlockToInstructionMapVectorTy &BBUsers);

private:
  static const unsigned MaxNumDims = 3;

  const DataLayout *DL;

  /// This is barrier utility class.
  DPCPPKernelBarrierUtils BarrierUtils;

  /// This holds the processed module context.
  LLVMContext *Context;
  /// This holds size of size_t of processed module.
  unsigned int SizeT;
  /// This holds type of size_t of processed module.
  Type *SizeTTy;
  Type *I32Ty;

  /// Use TLS globals if true, implicit arguments otherwise.
  bool UseTLSGlobals;
  /// Type of allocation used for storing local ID values for all dimensions.
  PointerType *LocalIdAllocTy;
  /// This holds TLS global containing local ids.
  GlobalVariable *LocalIds;
  /// This holds type of the TLS global containing local ids.
  ArrayType *LocalIdArrayTy;
  /// This holds cached GEP instructions for local ids.
  Value *PtrLocalId[MaxNumDims];

  Value *ConstZero;
  Value *ConstOne;

  /// This holds instruction to be removed in the processed function/module.
  InstVector InstructionsToRemove;

  /// This holds the data per value analysis pass.
  DataPerValue *DPV;
  /// This holds the container of all Group-A values in processed function.
  ValueVector *AllocaValues;
  /// This holds the container of all Group-B.1 values in processed function.
  ValueVector *SpecialValues;
  /// This holds the container of all Group-B.2 values in processed function.
  ValueVector *CrossBarrierValues;

  /// This holds the data per barrier analysis pass.
  DataPerBarrier *DPB;
  /// This holds the container of all sync instructions in processed function.
  InstSet *SyncInstructions;

  struct BarrierKeyValues {
    BarrierKeyValues()
        : TheFunction(0), NumDims(0), LocalIdValues(0), CurrBarrierId(0),
          SpecialBufferValue(0), CurrSBIndex(0), StructureSizeValue(0),
          CurrentVectorizedWidthValue(0) {
      Value *V = 0;
      std::fill(PtrLocalId, LocalSize + MaxNumDims, V);
      std::fill(LocalSize, LocalSize + MaxNumDims, V);
    }
    /// Pointer to function is needed because it is not always known how a
    /// BarrierKeyValues was obtained.
    Function *TheFunction;
    unsigned NumDims;
    /// This value is an array of size_t with MaxNumDims elements.
    Value *LocalIdValues;
    /// This array of pointers is used to cache GEP instructions.
    Value *PtrLocalId[MaxNumDims];

    /// This holds the alloca value of processed barrier id.
    Value *CurrBarrierId;
    /// This holds the argument value of special buffer address.
    Value *SpecialBufferValue;
    /// This holds the alloca value of current stride offset in Special Buffer.
    Value *CurrSBIndex;
    Value *LocalSize[MaxNumDims];
    /// This holds the constant value of structure size of Special Buffer.
    Value *StructureSizeValue;
    Value *CurrentVectorizedWidthValue;
  };
  using MapFunctionToKeyValuesTy = std::map<Function *, BarrierKeyValues>;

  /// This holds the function currently being handled.
  Function *CurrentFunction;
  /// This holds barrier key values for current handled function.
  BarrierKeyValues *CurrentBarrierKeyValues;
  /// This holds a map between function and its barrier key values.
  MapFunctionToKeyValuesTy BarrierKeyValuesPerFunction;

  using MapBasicBlockToBasicBlockTy = DenseMap<BasicBlock *, BasicBlock *>;
  /// This holds a map between sync basic block and previous pre sync loop
  /// header basic block.
  MapBasicBlockToBasicBlockTy PreSyncLoopHeader;

  /// This holds a map between kernel function name and buffer stride size.
  MapFunctionNameToBufferStrideTy BufferStrideMap;

  /// true if and only if we are running in native (gdb) dbg mode.
  bool IsNativeDBG;

  /// This holds a map between function to its total size of all new addr
  /// alloca created in fixAllocaValues.
  DenseMap<Function *, uint64_t> AddrAllocaSize;

  /// This holds per-function map from sync basic block to newly splitted sync
  /// basic block.
  DenseMap<Function *, DenseMap<BasicBlock *, BasicBlock *>> OldToNewSyncBBMap;

  /// This holds a map from basic block to its containing sync instruction.
  DenseMap<BasicBlock *, Instruction *> SyncPerBB;

  /// This holds a map from a sync basic block to its successors that are also
  /// sync basic blocks.
  BasicBlockToBasicBlockSetTy SyncBBSuccessors;

  /// This holds a map from a basic block to all nodes dominated by the basic
  /// block.
  BasicBlockToBasicBlockVectorTy BBToDominatedBBs;

  /// This holds a map from a basic block to its predecessor basic blocks that
  /// contain a sync instruction.
  BasicBlockToBasicBlockVectorTy BBToPredSyncBB;

  /// This holds a map from a basic block to its nearest dominator that
  /// contains a sync instruction.
  BasicBlockToBasicBlockTy BBToNearestDominatorSyncBB;

  /// This holds a map from a basic block to another basic blocks and to
  /// whether there is a barrier in any path from the basic block to another
  /// basic block.
  DenseMap<BasicBlock *, DenseMap<BasicBlock *, bool>> HasBarrierFromTo;
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_BARRIER_PASS_H
