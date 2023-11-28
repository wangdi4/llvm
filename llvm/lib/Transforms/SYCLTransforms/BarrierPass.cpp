//==--- BarrierPass.cpp - Main Barrier pass - C++ -*------------------------==//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/SYCLTransforms/BarrierPass.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/RegionInfo.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Transforms/SYCLTransforms/DataPerBarrierPass.h"
#include "llvm/Transforms/SYCLTransforms/DataPerValuePass.h"
#include "llvm/Transforms/SYCLTransforms/Utils/BarrierRegionInfo.h"
#include "llvm/Transforms/SYCLTransforms/Utils/BarrierUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/DiagnosticInfo.h"
#include "llvm/Transforms/SYCLTransforms/Utils/LoopUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/MetadataAPI.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/Local.h"
#include <map>
#include <sstream>
#include <unordered_set>
#include <vector>

#define DEBUG_TYPE "sycl-kernel-barrier"

using namespace llvm;

namespace {

class KernelBarrierImpl {
public:
  KernelBarrierImpl(DataPerBarrier *DPB, DataPerValue *DPV)
      : DPB(DPB), DPV(DPV) {}

  bool run(Module &M);

private:
  using BBSet = CompilationUtils::BBSet;
  using FuncSet = CompilationUtils::FuncSet;
  using InstSet = CompilationUtils::InstSet;
  using InstVec = CompilationUtils::InstVec;
  using ValueVec = CompilationUtils::ValueVec;

  /// Execute pass on given function.
  /// F function to optimize.
  /// Returns True if function was modified.
  bool runOnFunction(Function &F);

  /// Use the stack for kernel function execution rather then the special
  /// work item buffer. This is needed for DWARF based debugging.
  /// Variable data will be copied out of a basic block from the special work
  /// item buffer up on entry. Variable data will be copied back out to the
  /// work item buffer upon leaving the basic block.
  /// InsertBeforeBegin instruction to insert copy out of stack before
  /// InsertBeforeEnd instruction to insert copy into stack before
  void useStackAsWorkspace(Instruction *InsertBeforeBegin,
                           Instruction *InsertBeforeEnd);

  /// Hanlde Values of Group-A and debug info.
  /// F Function to fix.
  void fixAllocaAndDbg(Function &F);

  /// Hanlde Values of Group-B.1 of processed function.
  void fixSpecialValues();

  /// Hanlde Values of Group-B.2 of processed function.
  /// \p F Function to fix.
  void fixCrossBarrierValues(Function &F);

  /// Handle synchronize value of processed function.
  void replaceSyncInstructions();

  /// In a basic block, only keep the first CurrSBIndex load instruction and
  /// replace other CurrSBIndex instructions with it.
  void deduplicateCurrSBIndexInsts(Function &F);

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

  /// Return instructions to insert new instruction before, which are
  /// termenators of prevBBs of PhiNode with respect to Inst, and instructions
  /// can be empty if prevBBs are equal to BB(Inst).
  /// Inst - value that PhiNode is using,
  /// PhiNode - instruction that is using Inst value
  SmallVector<Instruction *> getInstructionsToInsertBefore(Instruction *Inst,
                                                           PHINode *PhiNode);

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
  void fixArgumentUsage(Value *OriginalArg);

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
  bool fixSynclessTIDUsers(Module &M, const FuncSet &FuncsWithSync);

  /// Remove all instructions in ToRemoveInstructions.
  bool eraseAllToRemoveInstructions();

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
    return B.CreateLoad(I32Ty, CurrentBarrierKeyValues->CurrBarrierId,
                        "CurrBarrierId");
  }
  Instruction *createSetCurrBarrierId(Value *V, IRBuilder<> &B) {
    return B.CreateStore(V, CurrentBarrierKeyValues->CurrBarrierId);
  }
  Instruction *createGetCurrSBIndex(IRBuilder<> &B) {
    return B.CreateLoad(SizeTTy, CurrentBarrierKeyValues->CurrSBIndex,
                        "SBIndex");
  }
  Instruction *createSetCurrSBIndex(Value *V, IRBuilder<> &B) {
    return B.CreateStore(V, CurrentBarrierKeyValues->CurrSBIndex);
  }
  Instruction *createGetLocalId(unsigned Dim, IRBuilder<> &B) {
    Value *Ptr = createGetPtrToLocalId(Dim);
    return B.CreateLoad(SizeTTy, Ptr,
                        CompilationUtils::AppendWithDimension("LocalId_", Dim));
  }
  Instruction *createGetLocalId(Value *LocalIdValues, Value *Dim,
                                IRBuilder<> &B) {
    Value *Ptr = CompilationUtils::createGetPtrToLocalId(
        LocalIdValues, LocalIdArrayTy, Dim, B);
    return B.CreateLoad(SizeTTy, Ptr,
                        CompilationUtils::AppendWithDimension("LocalId_", Dim));
  }
  Instruction *createGetLocalId(Value *LocalIdValues, unsigned Dim,
                                IRBuilder<> &B) {
    Value *Ptr = CompilationUtils::createGetPtrToLocalId(
        LocalIdValues, LocalIdArrayTy, ConstantInt::get(I32Ty, APInt(32, Dim)),
        B);
    return B.CreateLoad(SizeTTy, Ptr,
                        CompilationUtils::AppendWithDimension("LocalId_", Dim));
  }
  Instruction *createSetLocalId(unsigned Dim, Value *V, IRBuilder<> &B) {
    Value *Ptr = createGetPtrToLocalId(Dim);
    return B.CreateStore(V, Ptr);
  }
  Value *createGetPtrToLocalId(unsigned Dim) {
    // For accesses to constant dimensions, cache the GEP instruction
    Value **Ptr;
    if (HasTLSGlobals) {
      Ptr = PtrLocalId + Dim;
    } else {
      Ptr = CurrentBarrierKeyValues->PtrLocalId + Dim;
    }
    if (!*Ptr) {
      Function *F;
      if (HasTLSGlobals) {
        F = CurrentFunction;
      } else {
        F = CurrentBarrierKeyValues->TheFunction;
      }
      IRBuilder<> LB(F->getEntryBlock().getTerminator());
      Value *LocalIdValues;
      if (HasTLSGlobals) {
        LocalIdValues = TLSLocalIds;
      } else {
        // If the LocalIDValues are generated externally to the function, make
        // sure we place the GEP before the value is accessed
        if (!isa<Instruction>(CurrentBarrierKeyValues->LocalIdValues))
          LB.SetInsertPoint(
              &*CurrentBarrierKeyValues->TheFunction->getEntryBlock().begin());
        LocalIdValues = CurrentBarrierKeyValues->LocalIdValues;
      }
      *Ptr = CompilationUtils::createGetPtrToLocalId(
          LocalIdValues, LocalIdArrayTy,
          ConstantInt::get(I32Ty, APInt(32, Dim)), LB);
    }
    return *Ptr;
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

private:
  void calculateDirectPrivateSize(
      Module &M, FuncSet &FnsWithSync,
      DenseMap<Function *, size_t> &DirectPrivateSizeMap);
  void calculatePrivateSize(Module &M, FuncSet &FnsWithSync,
                            DenseMap<Function *, size_t> &PrivateSizeMap);

  const DataLayout *DL = nullptr;

  /// This is barrier utility class.
  BarrierUtils Utils;

  /// This holds the processed module context.
  LLVMContext *Context = nullptr;
  /// This holds size of size_t of processed module.
  unsigned int SizeT = 0;
  /// This holds type of size_t of processed module.
  Type *SizeTTy = nullptr;
  Type *I32Ty = nullptr;

  /// The module has TLS globals if true, implicit arguments otherwise.
  bool HasTLSGlobals = false;
  /// Type of allocation used for storing local ID values for all dimensions.
  PointerType *LocalIdAllocTy = nullptr;
  /// This holds TLS global containing local ids.
  GlobalVariable *TLSLocalIds = nullptr;
  /// This holds type of the TLS global containing local ids.
  ArrayType *LocalIdArrayTy = nullptr;
  /// This holds cached GEP instructions for local ids.
  Value *PtrLocalId[MAX_WORK_DIM] = {nullptr};

  Value *ConstZero = nullptr;
  Value *ConstOne = nullptr;

  /// This holds instruction to be removed in the processed function/module.
  InstVec InstructionsToRemove;

  /// This holds the container of all Group-A values in processed function.
  ValueVec *AllocaValues = nullptr;
  /// This holds the container of all Group-B.1 values in processed function.
  ValueVec *SpecialValues = nullptr;
  /// This holds the container of all Group-B.2 values in processed function.
  ValueVec *CrossBarrierValues = nullptr;

  /// This holds the container of all sync instructions in processed function.
  InstSet *SyncInstructions = nullptr;

  /// This holds the data per barrier analysis pass.
  DataPerBarrier *DPB = nullptr;

  /// This holds the data per value analysis pass.
  DataPerValue *DPV = nullptr;

  struct BarrierKeyValues {
    BarrierKeyValues()
        : TheFunction(0), NumDims(0), LocalIdValues(0), CurrBarrierId(0),
          SpecialBufferValue(0), CurrSBIndex(0), StructureSizeValue(0),
          CurrentVectorizedWidthValue(0) {
      Value *V = 0;
      std::fill(PtrLocalId, LocalSize + MAX_WORK_DIM, V);
      std::fill(LocalSize, LocalSize + MAX_WORK_DIM, V);
    }
    /// Pointer to function is needed because it is not always known how a
    /// BarrierKeyValues was obtained.
    Function *TheFunction;
    unsigned NumDims;
    /// This value is an array of size_t with MAX_WORK_DIM elements.
    Value *LocalIdValues;
    /// This array of pointers is used to cache GEP instructions.
    Value *PtrLocalId[MAX_WORK_DIM];

    /// This holds the alloca value of processed barrier id.
    Value *CurrBarrierId;
    /// This holds the argument value of special buffer address.
    Value *SpecialBufferValue;
    /// This holds the alloca value of current stride offset in Special Buffer.
    Value *CurrSBIndex;
    Value *LocalSize[MAX_WORK_DIM];
    /// This holds the constant value of structure size of Special Buffer.
    Value *StructureSizeValue;
    Value *CurrentVectorizedWidthValue;
  };
  using MapFunctionToKeyValuesTy = std::map<Function *, BarrierKeyValues>;

  /// This holds the function currently being handled.
  Function *CurrentFunction = nullptr;
  /// This holds barrier key values for current handled function.
  BarrierKeyValues *CurrentBarrierKeyValues = nullptr;
  /// This holds a map between function and its barrier key values.
  MapFunctionToKeyValuesTy BarrierKeyValuesPerFunction;

  using MapBasicBlockToBasicBlockTy = DenseMap<BasicBlock *, BasicBlock *>;
  /// This holds a map between sync basic block and previous pre sync loop
  /// header basic block.
  MapBasicBlockToBasicBlockTy PreSyncLoopHeader;

  /// This holds a map between function to its total size of all new addr
  /// alloca created in fixAllocaAndDbg.
  DenseMap<Function *, uint64_t> AddrAllocaSize;

  /// This holds per-function map from sync basic block to newly splitted sync
  /// basic block.
  DenseMap<Function *, DenseMap<BasicBlock *, BasicBlock *>> OldToNewSyncBBMap;

  using MapAllocaToBasicBlockVector =
      MapVector<Value *, SmallVector<BasicBlock *, 8>>;
  /// Map from alloca to BasicBlock vector.
  /// If original alloca %my_i used in debugger intrinsic functions has user
  /// %my_i.ascast generated from addrspacecast and %my_i.ascast is
  /// cross-barrier value, then new %my_i.addr need to update value from special
  /// buffer in the BBs where %my_i is not used but %my_i.ascast is used.
  /// Without this, debug info of %my_i is incorrect in those BBs.
  MapAllocaToBasicBlockVector AllocaUpdateMap;

  /// This holds a map from function to its vector of CurrSBIndex instructions.
  DenseMap<Function *, SmallVector<Instruction *, 0>> FuncToCurrSBIndexInstsMap;
};

} // namespace

bool KernelBarrierImpl::run(Module &M) {
  DL = &M.getDataLayout();

  // Initialize barrier utils class with current module.
  Utils.init(&M);
  // This call is needed to initialize vectorization widths.
  Utils.getAllKernelsWithBarrier();

  Context = &M.getContext();
  SizeT = M.getDataLayout().getPointerSizeInBits(0);
  SizeTTy = IntegerType::get(*Context, SizeT);
  I32Ty = IntegerType::get(*Context, 32);
  LocalIdArrayTy = ArrayType::get(SizeTTy, MAX_WORK_DIM);
  LocalIdAllocTy = PointerType::get(LocalIdArrayTy, 0);
  ConstZero = ConstantInt::get(SizeTTy, 0);
  ConstOne = ConstantInt::get(SizeTTy, 1);

  bool ModuleHasAnyInternalCalls = false;
  bool Changed = false;

  TLSLocalIds = M.getNamedGlobal(CompilationUtils::getTLSLocalIdsName());
  HasTLSGlobals = TLSLocalIds != nullptr;

  // Find all functions that call synchronize instructions.
  CompilationUtils::FuncSet FunctionsWithSync =
      Utils.getAllFunctionsWithSynchronization();
  Changed |= !FunctionsWithSync.empty();

  // TODO if we factor private memory calculation out of barrier pass and
  // FunctionsWithSync is empty, we shall early return here.

  // Note: We can't early exit here, otherwise, optimizer will claim the
  // functions to be resolved in this pass are undefined.
  CompilationUtils::FuncSet RecursiveFunctions =
      Utils.getRecursiveFunctionsWithSync();
  if (!RecursiveFunctions.empty() &&
      CompilationUtils::isGeneratedFromOCLCPP(M)) {
    std::string ErrMsg;
    raw_string_ostream OS(ErrMsg);
    OS << "Recursive call in function with barrier is unsupported:";
    for (Function *F : RecursiveFunctions)
      OS << "\n  " << F->getName();
    Context->diagnose(OptimizationErrorDiagInfo(ErrMsg));
    Changed = true;
  }

  // Collect data for each function with synchronize instruction.
  for (Function *Func : FunctionsWithSync) {
    // Check if function has no synchronize instructions!
    assert(DPB->hasSyncInstruction(Func) &&
           "Cannot reach here with function that has no barrier");

    // Create new BB at the begining of the function for declarations.
    BasicBlock *EntryBB = &Func->getEntryBlock();
    BasicBlock *FirstBB =
        Func->begin()->splitBasicBlock(EntryBB->begin(), "FirstBB");
    OldToNewSyncBBMap[Func][EntryBB] = FirstBB;

    // Initialize the argument values.
    // This is needed for optimize LocalId calculation.
    bool HasNoInternalCalls = !Utils.doesCallModuleFunction(Func);
    ModuleHasAnyInternalCalls =
        ModuleHasAnyInternalCalls || !HasNoInternalCalls;
    createBarrierKeyValues(Func, HasNoInternalCalls);
  }

  // Fix non inlined internal functions that need special handling.
  // Run over functions with synchronize instruction:
  // 1. Handle call instructions to non-inline functions.
  for (Function *FuncToFix : FunctionsWithSync) {

    // Run over old users of FuncToFix and prepare parameters as needed.
    for (User *U : FuncToFix->users()) {
      CallInst *CI = dyn_cast<CallInst>(U);
      if (!CI)
        continue;
      // Handle call instruction operands and return value, if needed.
      fixCallInstruction(CI);
    }
  }
  // 2. Handle non-inline functions.
  for (Function *FuncToFix : FunctionsWithSync) {
    // Load arguments from special buffer at specific offset as needed.
    fixNonInlineFunction(FuncToFix);
  }

  // Run over functions with synchronize instruction:
  // 1. Handle Values from Group-A, Group-B.1 and Group-B.2.
  // 2. Hanlde synchronize instructions.
  for (Function *FuncToFix : FunctionsWithSync)
    runOnFunction(*FuncToFix);

  // Update Map with structure stride size for each kernel.
  // fixAllocaAndDbg may add new alloca.
  updateStructureStride(M, FunctionsWithSync);

  // Fix TID user functions that doesn't contains sync instruction. This is
  // only needed for subgroup emulation.
  if (!HasTLSGlobals)
    Changed |= fixSynclessTIDUsers(M, FunctionsWithSync);

  // Fix get_local_id() and get_global_id() function calls.
  Changed |= fixGetWIIdFunctions(M);

  return Changed;
}

bool KernelBarrierImpl::runOnFunction(Function &F) {
  // Get key values for this functions.
  getBarrierKeyValues(&F);

  SyncInstructions = &DPB->getSyncInstructions(&F);

  SpecialValues = &DPV->getValuesToHandle(&F);
  AllocaValues = &DPV->getAllocaValuesToHandle(&F);
  CrossBarrierValues = &DPV->getUniformValuesToHandle(&F);

  // Clear container for new iteration on new function.
  InstructionsToRemove.clear();
  PreSyncLoopHeader.clear();

  // Fix special values.
  fixSpecialValues();

  // Fix alloca values and debug info.
  fixAllocaAndDbg(F);

  // Fix cross barrier uniform values.
  fixCrossBarrierValues(F);

  // Replace sync instructions with internal loop over WI ID.
  replaceSyncInstructions();

  // Remove redundant CurrSBIndex load instructions in each basic block.
  deduplicateCurrSBIndexInsts(F);

  // Remove all instructions in InstructionsToRemove.
  (void)eraseAllToRemoveInstructions();

  return true;
}

bool KernelBarrierImpl::fixSynclessTIDUsers(Module &M,
                                            const FuncSet &FuncsWithSync) {
  DenseMap<Function *, Value *> PatchedFToLocalIds;
  for (auto &Pair : BarrierKeyValuesPerFunction)
    PatchedFToLocalIds.insert({Pair.first, Pair.second.LocalIdValues});
  auto CreateLIDArg = [&](CallInst *CI) -> Value * {
    llvm_unreachable("unexpected CreateLIDArg call");
  };

  // Collect TID calls in not-inlined functions, which are neither kernel nor
  // functions with sync instruction.
  CompilationUtils::InstVec TIDCallsToFix;
  auto FindTIDsToPatch = [&](const CompilationUtils::InstVec &TIDs) {
    for (auto *I : TIDs) {
      auto *CI = cast<CallInst>(I);
      if (!FuncsWithSync.contains(CI->getFunction()))
        TIDCallsToFix.push_back(CI);
    }
  };
  FindTIDsToPatch(CompilationUtils::getCallInstUsersOfFunc(
      M, CompilationUtils::mangledGetLID()));
  FindTIDsToPatch(CompilationUtils::getCallInstUsersOfFunc(
      M, CompilationUtils::mangledGetGID()));

  if (TIDCallsToFix.empty())
    return false;

  IRBuilder<> Builder(M.getContext());
  CompilationUtils::patchNotInlinedTIDUserFunc(
      M, Builder, FuncsWithSync, TIDCallsToFix, PatchedFToLocalIds,
      LocalIdAllocTy, CreateLIDArg);

  // Update BarrierKeyValuesPerFunction with newly created local ids.
  for (auto &Pair : PatchedFToLocalIds) {
    auto It = BarrierKeyValuesPerFunction.find(Pair.first);
    if (It == BarrierKeyValuesPerFunction.end())
      It = BarrierKeyValuesPerFunction.insert({Pair.first, BarrierKeyValues{}})
               .first;
    It->second.TheFunction = Pair.first;
    It->second.LocalIdValues = Pair.second;
  }

  return true;
}

void KernelBarrierImpl::fixAllocaAndDbg(Function &F) {
  DIBuilder DIB(*F.getParent(), /*AllowUnresolved*/ false);
  const DataLayout &DL = F.getParent()->getDataLayout();
  Instruction *AddrInsertBefore = &*F.getEntryBlock().begin();
  DISubprogram *SP = F.getSubprogram();
  DebugLoc DB;
  auto File = DIB.createFile("CPU_DEVICE_RT", "/");
  if (SP) {
    auto Scope = DIB.createLexicalBlockFile(SP, File, 0);
    DB = DILocation::get(SP->getContext(), 0, 0, Scope);
  }

  // This holds a map from basic block to its containing sync instruction.
  DenseMap<BasicBlock *, Instruction *> SyncPerBB;
  for (auto *I : *SyncInstructions)
    if (I->getFunction() == &F)
      SyncPerBB[I->getParent()] = I;

  ValueVec WorkList(*AllocaValues);
  // Fix kernel argument which has debug info.
  for (auto &Arg : F.args())
    if (!Arg.use_empty() && DPV->hasOffset(&Arg))
      WorkList.push_back(&Arg);

  // Compute barrier region info. If a cross-barrier value is used in multiple
  // basic blocks within a barrier region, it is only necessary to load the
  // value from special buffer once in region header. The uses in the region
  // will be replaced by the loaded value.
  DominatorTree DT;
  DominanceFrontier DF;
  PostDominatorTree PDT;
  RegionInfo RI;
  std::unique_ptr<BarrierRegionInfo> BRI;
  // BarrierRegionInfo isn't able to handle a rare case that a basic block is
  // unreachable from entry block. For this case, we need to load from special
  // buffer for every use of a cross-barrier value.
  if (!WorkList.empty()) {
    DT.recalculate(F);
    if (llvm::none_of(F, [&](BasicBlock &BB) { return !DT.getNode(&BB); })) {
      DF.analyze(DT);
      BRI.reset(new BarrierRegionInfo(&F, &DF, &DT));
      PDT.recalculate(F);
      RI.recalculate(F, &DT, &PDT, &DF);
    }
  }

  for (Value *V : WorkList) {
    auto *AI = dyn_cast<AllocaInst>(V);

    // Don't fix implicit GID.
    if (SP && AI && CompilationUtils::isImplicitGID(AI)) {
      // Move implicit GID out of barrier loop.
      AI->moveBefore(AddrInsertBefore);
      continue;
    }

    // Collect debug intrinsic.
    TinyPtrVector<DbgDeclareInst *> DIs;
    if (SP)
      DIs = CompilationUtils::findDbgUses(V);

    // Only use the first DbgVariableIntrinsic.
    // TODO: there might be multiple llvm.dbg.addr calls when llvm.dbg.declare
    // is deprecated. We probably need to insert new llvm.dbg.addr for each
    // call.
    DbgVariableIntrinsic *DI = DIs.empty() ? nullptr : DIs.front();
    // Need not to create new alloca if the value isn't an alloca and doesn't
    // have debug info.
    if (!DI && !AI)
      continue;

    // Insert new alloca which stores AI's address in special buffer.
    // AI's users will be replaced by result of load instruction from the new
    // alloca.
    StringRef AllocaName = V->getName();
    PointerType *AllocatedTy = AI ? AI->getType()
                                  : cast<Argument>(V)->getType()->getPointerTo(
                                        SPECIAL_BUFFER_ADDR_SPACE);
    AllocaInst *AddrAI = new AllocaInst(AllocatedTy, DL.getAllocaAddrSpace(),
                                        AllocaName + ".addr", AddrInsertBefore);
    if (AI)
      AddrAI->setDebugLoc(AI->getDebugLoc());
    uint64_t ASize = AddrAI->getAllocationSizeInBits(DL).value() / 8;
    AddrAI->setAlignment(assumeAligned(ASize));
    AddrAllocaSize[&F] += ASize;

    if (DI) {
      DIExpression *Expr =
          DIExpression::prepend(DI->getExpression(), DIExpression::DerefBefore);
      // byval argument are passed by value on the stack. It is represented as a
      // pointer. We need to dereference its pointer type.
      if (auto A = dyn_cast<Argument>(V); A && A->hasByValAttr())
        Expr = DIExpression::prepend(Expr, DIExpression::DerefBefore);
      DIB.insertDeclare(AddrAI, DI->getVariable(), Expr,
                        DI->getDebugLoc().get(), DI);
    }

    // Get offset of alloca value in special buffer.
    unsigned int Offset = DPV->getOffset(V);
    // Update AddrAI value.
    if (AI && DI && AllocaUpdateMap.contains(AI)) {
      for (auto *BB : AllocaUpdateMap[AI]) {
        Instruction *InsertBefore = BB->getFirstNonPHI();
        if (BarrierUtils::isBarrierOrDummyBarrierCall(InsertBefore))
          InsertBefore = InsertBefore->getNextNode();
        assert(InsertBefore && "InsertBefore is invalid, debug info isn't "
                               "fixed for alloca pointer cast users.");
        Value *AddrInSpecialBuffer =
            getAddressInSpecialBuffer(Offset, AllocatedTy, InsertBefore, &DB);
        IRBuilder<> Builder(InsertBefore);
        Builder.SetCurrentDebugLocation(DB);
        Builder.CreateStore(AddrInSpecialBuffer, AddrAI);
      }
    }

    // Insert instruction to load V's address in special buffer to AddrAI.
    // Barrier region header is a coarse estimate of insert point. Within the
    // barrier region, RegionInfo is used to find the insert point basic block
    // that dominators the parent basic blocks of V's users.
    // E.g.
    //   for.cond:
    //     br i1 %cmp3, label %for.body, label %for.end
    //   for.body:
    //     br %master.thread.fallthru
    //   master.thread.fallthru:
    //     call void @_Z18work_group_barrierj12memory_scope(i32 3, i32 1)
    //     br label %for.inc
    //   for.inc:
    //     br label %for.cond
    //   for.end:
    //     store i32 0, i32* %j, align 4
    // for.end and for.cond are in the same barrier region, and %j has single
    // user in for.end which is outside the loop. If we load %j's address in
    // special buffer in region header, %j's debug info will change in the loop,
    // which isn't desired.
    MapVector<BasicBlock *, SmallVector<Instruction *, 8>> BBUsers;
    for (auto *U : V->users()) {
      BasicBlock *BB = cast<Instruction>(U)->getParent();
      if (BRI)
        BB = BRI->getRegionHeaderFor(BB);
      BBUsers[BB].push_back(cast<Instruction>(U));
    }
    for (auto &Pair : BBUsers) {
      BasicBlock *BB = Pair.first;
      Instruction *InsertBefore = nullptr;
      if (BB->isEntryBlock()) {
        InsertBefore = BB->getTerminator();
      } else if (AI && AI->getParent() == BB) {
        // The inserted instructions will get debug loc of current alloca.
        InsertBefore = AI;
      } else if (BRI) {
        // Get top level region.
        Region *R = nullptr;
        for (auto *UI : Pair.second) {
          auto *R1 = RI.getRegionFor(UI->getParent());
          R = R ? RI.getCommonRegion(R, R1) : R1;
        }

        // Find dominator basic block.
        BasicBlock *Dom = nullptr;
        SmallPtrSet<Region *, 4> Visited;
        for (auto *UI : Pair.second) {
          auto *BB1 = UI->getParent();
          auto *R1 = RI.getRegionFor(BB1);
          if (R1 == R) {
            Dom = Dom ? DT.findNearestCommonDominator(Dom, BB1) : BB1;
          } else if (Visited.insert(R1).second) {
            auto *Entry = R1->getEntry();
            Dom = Dom ? DT.findNearestCommonDominator(Dom, Entry) : Entry;
          }
        }

        bool NonPHI = llvm::none_of(Pair.second, [&](auto *I) {
          return I->getParent() == Dom && isa<PHINode>(I);
        });
        if (NonPHI && BRI->getRegionHeaderFor(Dom) == BB) {
          if (auto It = SyncPerBB.find(Dom); It != SyncPerBB.end())
            InsertBefore = It->second->getNextNode();
          else
            InsertBefore = Dom->getFirstNonPHI();
        }
      }
      if (!InsertBefore) {
        if (auto It = SyncPerBB.find(BB); It != SyncPerBB.end()) {
          InsertBefore = It->second->getNextNode();
          assert(
              InsertBefore->getParent() == BB &&
              "sync instruction must not be the last instruction in the block");
        } else {
          InsertBefore = BB->getFirstNonPHI();
        }
      }
      assert(InsertBefore && "InsertBefore is invalid");
      // Calculate the pointer of the current alloca in the special buffer.
      Value *AddrInSpecialBuffer =
          getAddressInSpecialBuffer(Offset, AllocatedTy, InsertBefore, &DB);
      IRBuilder<> Builder(InsertBefore);
      Builder.SetCurrentDebugLocation(DB);
      Builder.CreateStore(AddrInSpecialBuffer, AddrAI);
      LoadInst *LI = Builder.CreateLoad(AllocatedTy, AddrAI);
      if (isa<Argument>(V))
        LI =
            Builder.CreateLoad(cast<Argument>(V)->getType(), LI, "loadedValue");

      for (auto *I : Pair.second)
        if (!isa<DbgVariableIntrinsic>(I))
          I->replaceUsesOfWith(V, LI);
    }

    if (AI)
      InstructionsToRemove.push_back(AI);

    // Remove old DbgVariableIntrinsic.
    if (SP)
      for (auto *DI : DIs)
        DI->eraseFromParent();
  }
  if (BasicBlock *FirstBB = OldToNewSyncBBMap[&F][&F.getEntryBlock()]) {
    // Find llvm.dbg.value intrinsics referencing arguments and hoist them
    // to the newly created entry block
    TinyPtrVector<DbgVariableIntrinsic*> HoistedDVIs;
    auto IsArgument = [](Value *V) -> bool { return isa<Argument>(V); };
    for (Instruction &I : *FirstBB) {
      if (auto *DVI = dyn_cast<DbgVariableIntrinsic>(&I)) {
        if (llvm::all_of(DVI->location_ops(), IsArgument))
          HoistedDVIs.push_back(DVI);
      }
    }
    Instruction *InsertionPoint = &F.getEntryBlock().front();
    for (DbgVariableIntrinsic *DVI : HoistedDVIs)
      DVI->moveBefore(InsertionPoint);
  }
  DIB.finalize();
}

void KernelBarrierImpl::fixSpecialValues() {
  for (Value *V : *SpecialValues) {
    Instruction *Inst = cast<Instruction>(V);

    const DebugLoc &DB = Inst->getDebugLoc();

    // Update AllocaUpdateMap.
    if (auto *AI = dyn_cast<AllocaInst>(V->stripPointerCasts()); AI != V) {
      if (std::find(AllocaValues->begin(), AllocaValues->end(), AI) !=
          AllocaValues->end()) {
        std::unordered_set<BasicBlock *> AIUserBBs;
        std::unordered_set<BasicBlock *> AIPointerCastUserBBs;
        for (auto *U : AI->users())
          if (auto *Inst = dyn_cast<Instruction>(U))
            AIUserBBs.insert(Inst->getParent());

        for (auto *U : V->users())
          if (auto *Inst = dyn_cast<Instruction>(U))
            AIPointerCastUserBBs.insert(Inst->getParent());

        SmallVector<BasicBlock *, 8> AllocaUpdateVec;
        for (auto *BB : AIPointerCastUserBBs)
          if (!AIUserBBs.count(BB))
            AllocaUpdateVec.push_back(BB);

        if (!AllocaUpdateVec.empty())
          AllocaUpdateMap.insert({AI, AllocaUpdateVec});
      }
    }

    // This will hold the real type of this value in the special buffer.
    Type *TyInSP = Inst->getType();
    bool OneBitBaseType = DPV->isOneBitElementType(Inst);
    if (OneBitBaseType) {
      // Base type is i1 need to ZEXT/TRUNC to/from i32.
      VectorType *VecType = dyn_cast<VectorType>(Inst->getType());
      if (VecType) {
        TyInSP = FixedVectorType::get(
            IntegerType::get(*Context, 32),
            cast<FixedVectorType>(VecType)->getNumElements());
      } else {
        TyInSP = IntegerType::get(*Context, 32);
      }
    }

    // Get offset of special value in special buffer.
    unsigned int Offset = DPV->getOffset(Inst);
    // Find next instruction so we can create new instruction before it.
    Instruction *NextInst = &*(++BasicBlock::iterator(Inst));
    if (isa<PHINode>(NextInst)) {
      // NextInst is a PHINode, find first non PHINode to add instructions
      // before it.
      NextInst = NextInst->getParent()->getFirstNonPHI();
    }
    // Get PointerType of value type.
    PointerType *Ty = TyInSP->getPointerTo(SPECIAL_BUFFER_ADDR_SPACE);
    // Handle Special buffer only if it is not a call instruction.
    // Special buffer value of call instruction will be handled in the callee.
    CallInst *CI = dyn_cast<CallInst>(Inst);
    if (!(CI && DPV->hasOffset(CI->getCalledFunction()))) {
      // Calculate the pointer of the current special in the special buffer.
      Value *AddrInSpecialBuffer =
          getAddressInSpecialBuffer(Offset, Ty, NextInst, &DB);
      Instruction *InstToStore =
          !OneBitBaseType ? Inst
                          : CastInst::CreateZExtOrBitCast(
                                Inst, TyInSP, "ZEXT-i1Toi32", NextInst);
      // Need to set DebugLoc for the case is OneBitBaseType. It won't hart to
      // set Same DebugLoc for the other case, as DB = Inst->getDebugLoc();
      InstToStore->setDebugLoc(DB);
      // Add Store instruction after the value instruction.
      StoreInst *SI = new StoreInst(InstToStore, AddrInSpecialBuffer, NextInst);
      SI->setDebugLoc(DB);
    }

    InstSet UserInsts;
    // Save all uses of Inst and add them to a container before start handling
    // them!
    for (User *U : Inst->users()) {
      Instruction *UserInst = cast<Instruction>(U);
      if (Inst->getParent() == UserInst->getParent()) {
        // This use of Inst is at the same basic block (no barrier cross so far)
        // assert( !isa<PHINode>(UserInst) && "user instruction is a PHINode and
        // appears befre Inst in BB" );
        if (!isa<PHINode>(UserInst)) {
          continue;
        }
      }
      if (isa<ReturnInst>(UserInst)) {
        // We don't want to return the value from the Special buffer we will
        // load it later by the caller.
        continue;
      }
      UserInsts.insert(UserInst);
    }

    auto InsertLoadedValue = [&](Instruction *InsertBefore,
                                 const DebugLoc &DB) {
      // Calculate the pointer of the current special in the special buffer.
      Value *AddrInSpecialBuffer =
          getAddressInSpecialBuffer(Offset, Ty, InsertBefore, &DB);
      Instruction *LoadedValue = new LoadInst(TyInSP, AddrInSpecialBuffer,
                                              "loadedValue", InsertBefore);
      Instruction *RealValue =
          !OneBitBaseType
              ? LoadedValue
              : CastInst::CreateTruncOrBitCast(LoadedValue, Inst->getType(),
                                               "Trunc-i1Toi32", InsertBefore);
      LoadedValue->setDebugLoc(DB);
      RealValue->setDebugLoc(DB);
      return RealValue;
    };

    // Run over all saved user instructions and handle by adding
    // load instruction before each value use.
    for (Instruction *UserInst : UserInsts) {
      const DebugLoc &DB = UserInst->getDebugLoc();

      if (auto *PhiNode = dyn_cast<PHINode>(UserInst)) {
        auto InsertBefores = getInstructionsToInsertBefore(Inst, PhiNode);
        for (auto *InsertBefore : InsertBefores) {
          auto *RealValue = InsertLoadedValue(InsertBefore, DB);
          PhiNode->setIncomingValueForBlock(InsertBefore->getParent(),
                                            RealValue);
        }
      } else {
        auto *RealValue = InsertLoadedValue(UserInst, DB);
        UserInst->replaceUsesOfWith(Inst, RealValue);

        // If UserInst is a sync instruction, then it will not be the first
        // instruction of basic block, which will break the assumption of
        // BarrierUtils::isCrossedByBarrier, thus we split it to satisfy the
        // assumption
        if (SyncInstructions->contains(UserInst)) {
          const auto Name = UserInst->getParent()->getName() + ".BarrierFix";
          UserInst->getParent()->splitBasicBlock(BasicBlock::iterator(UserInst),
                                                 Name);
        }
      }
    }
  }
}

/// Hoist an uniform cross-barrier instruction and its depedent instructions to
/// entry basic block, which donimates the instruction's users. Return true if
/// the instruction either is already in the entry block or is hoisted. Return
/// false if the instruction can't be hoisted.
static bool
hoistUniformCrossBarrierInstToEntryBlock(Instruction *I,
                                         Instruction *InsertBefore) {
  BasicBlock *InsertBeforeBB = InsertBefore->getParent();
  if (I->getParent() == InsertBeforeBB)
    return true;
  if (isa<CallInst>(I) || I->use_empty())
    return false;

  // Collect dependent instructions to move.
  SmallSetVector<Instruction *, 16> ToMove;
  Use *TheUse = &*I->use_begin();
  for (Use *U : make_range(po_begin(TheUse), po_end(TheUse))) {
    Value *V = U->get();
    // Skip constant value that may consist of global variable.
    if (isa<GlobalVariable>(V) || isa<ConstantExpr>(V) ||
        isa<ConstantAggregate>(V))
      return false;
    if (auto *Inst = dyn_cast<Instruction>(V)) {
      if (auto *CI = dyn_cast<CallInst>(Inst)) {
        Function *Callee = CI->getCalledFunction();
        if (!Callee)
          return false;
        StringRef Name = Callee->getName();
        using namespace CompilationUtils;
        if (!isGetGlobalSize(Name) && !isGetGroupId(Name) &&
            !isGetLocalSize(Name) && !isGetNumGroups(Name) &&
            !isGetWorkDim(Name))
          return false;
      }
      if (Inst->getParent() != InsertBeforeBB)
        ToMove.insert(Inst);
    }
  }

  for (Instruction *Inst : ToMove) {
    Inst->moveBefore(InsertBefore);
    Inst->dropLocation();
  }

  return true;
}

void KernelBarrierImpl::fixCrossBarrierValues(Function &F) {
  ValueVec WorkList;
  if (!F.hasOptNone()) {
    Instruction *InsertBefore = &*F.begin()->getFirstNonPHIOrDbgOrAlloca();
    for (Value *V : llvm::reverse(*CrossBarrierValues))
      if (!hoistUniformCrossBarrierInstToEntryBlock(cast<Instruction>(V),
                                                    InsertBefore))
        WorkList.push_back(V);
  } else {
    WorkList = *CrossBarrierValues;
  }

  Instruction *InsertBefore = &*F.begin()->begin();

  for (Value *V : WorkList) {
    Instruction *Inst = dyn_cast<Instruction>(V);
    assert(Inst && "container of special values has non Instruction value!");
    // Find next instruction so we can create new instruction before it.
    Instruction *NextInst = &*(++BasicBlock::iterator(Inst));
    if (isa<PHINode>(NextInst)) {
      // NextInst is a PHINode, find first non PHINode to add instructions
      // before it.
      NextInst = NextInst->getParent()->getFirstNonPHI();
    }
    // Create alloca of value type at begining of function.
    AllocaInst *AI = new AllocaInst(Inst->getType(), DL->getAllocaAddrSpace(),
                                    Inst->getName(), InsertBefore);
    // Add Store instruction after the value instruction.
    StoreInst *SI = new StoreInst(Inst, AI, NextInst);
    SI->setDebugLoc(Inst->getDebugLoc());

    InstSet UserInsts;
    // Save all uses of Inst and add them to a container before start handling
    // them!
    for (User *U : Inst->users()) {
      Instruction *UserInst = dyn_cast<Instruction>(U);
      assert(UserInst && "uses of special instruction is not an instruction!");
      if (Inst->getParent() == UserInst->getParent() &&
          !isa<PHINode>(UserInst)) {
        // This use of Inst is at the same basic block (no barrier cross so
        // far).
        continue;
      }
      UserInsts.insert(UserInst);
    }
    // Run over all saved user instructions and handle by adding
    // load instruction before each value use.
    for (Instruction *UserInst : UserInsts) {
      const DebugLoc &DB = UserInst->getDebugLoc();
      if (auto *PhiNode = dyn_cast<PHINode>(UserInst)) {
        auto InsertBefores = getInstructionsToInsertBefore(Inst, PhiNode);
        for (auto *InsertBefore : InsertBefores) {
          // Calculate the pointer of the current special in the special buffer.
          Instruction *LoadedValue = new LoadInst(AI->getAllocatedType(), AI,
                                                  "loadedValue", InsertBefore);
          LoadedValue->setDebugLoc(DB);
          PhiNode->setIncomingValueForBlock(InsertBefore->getParent(),
                                            LoadedValue);
        }
      } else {
        // Calculate the pointer of the current special in the special buffer.
        Instruction *LoadedValue =
            new LoadInst(AI->getAllocatedType(), AI, "loadedValue", UserInst);
        LoadedValue->setDebugLoc(DB);
        UserInst->replaceUsesOfWith(Inst, LoadedValue);
        // FIXME: For the same reason as the end of fixSpecialValues, this also
        // need to be done here. But:
        //   1. Currently, BarrierUtils::isCrossedByBarrier is not called after
        //      this function
        //   2. It's complex to update the containers in fixAllocaAndDbg
        // So, it's unnecessary and a little risky to do this fix now.
        //
        // if (SyncInstructions->contains(UserInst)) {
        //   assert(UserInst == InsertBefore &&
        //          "UserInst should be the insertion point");
        //   const auto Name = UserInst->getParent()->getName() + ".BarrierFix";
        //   UserInst->getParent()->splitBasicBlock(BasicBlock::iterator(UserInst),
        //                                          Name);
        // }
      }
    }
  }
}

BasicBlock *KernelBarrierImpl::createLatchNesting(unsigned Dim,
                                                  BasicBlock *Body,
                                                  BasicBlock *Dispatch,
                                                  Value *Step,
                                                  const DebugLoc &DL) {
  LLVMContext &C = Body->getContext();
  Function *F = Body->getParent();
  // BB that is jumped to if loop in current nesting finishes.
  BasicBlock *LoopEnd = BasicBlock::Create(
      C, CompilationUtils::AppendWithDimension("LoopEnd_", Dim), F, Dispatch);

  {
    IRBuilder<> B(Body);
    B.SetCurrentDebugLocation(DL);
    Value *LocalId = createGetLocalId(Dim, B);
    LocalId = B.CreateNUWAdd(LocalId, Step);
    createSetLocalId(Dim, LocalId, B);

    // if(LocalId[Dim] < WGSize[dim]) {BB Dispatch} else {BB LoopEnd}
    Value *IsContinue = B.CreateICmpULT(LocalId, getLocalSize(Dim));
    B.CreateCondBr(IsContinue, Dispatch, LoopEnd);
  }

  {
    IRBuilder<> B(LoopEnd);
    B.SetCurrentDebugLocation(DL);
    createSetLocalId(Dim, ConstZero, B);
  }
  return LoopEnd;
}

BasicBlock *KernelBarrierImpl::createBarrierLatch(BasicBlock *PreSyncBB,
                                                  BasicBlock *SyncBB,
                                                  BarrierBBIdListTy &BBId,
                                                  Value *UniqueID,
                                                  const DebugLoc &DL) {
  Function *F = PreSyncBB->getParent();
  unsigned NumDims = getNumDims();
  // A. change the preSync basic block as follow
  // A(1). remove the unconditional jump instruction.
  PreSyncBB->getTerminator()->eraseFromParent();
  // Create then and else basicblocks.
  BasicBlock *Dispatch = BasicBlock::Create(*Context, "Dispatch", F, SyncBB);
  BasicBlock *InnerMost = PreSyncBB;
  assert(CurrentBarrierKeyValues->CurrentVectorizedWidthValue);
  Value *LoopSteps[MAX_WORK_DIM] = {
      CurrentBarrierKeyValues->CurrentVectorizedWidthValue, ConstOne, ConstOne};
  for (unsigned I = 0; I < NumDims; ++I)
    InnerMost = createLatchNesting(I, InnerMost, Dispatch, LoopSteps[I], DL);

  // A(2). add the entry tail code
  // if(LocalId < WGSize[dim]) {Dispatch} else {pElseBB}
  // B. Create LocalId++ and switch instruction in Dispatch
  // Create "LocalId+=VectorizationWidth" code
  // Create "CurrSBBase+=Stride" code
  {
    IRBuilder<> B(Dispatch);
    B.SetCurrentDebugLocation(DL);
    Value *CurrSBIndex = createGetCurrSBIndex(B);
    Value *UpdatedCurrSB = B.CreateNUWAdd(
        CurrSBIndex, CurrentBarrierKeyValues->StructureSizeValue);
    createSetCurrSBIndex(UpdatedCurrSB, B);

    if (BBId.size() == 1) {
      // Only one case, no need for switch, create unconditional jump.
      B.CreateBr(BBId[0].second);
    } else {
      // More than one case, create a switch.
      Value *CurrBarrierId = createGetCurrBarrierId(B);
      // The first sync instruction is chosen to be the switch Default case.
      SwitchInst *S =
          B.CreateSwitch(CurrBarrierId, BBId[0].second, BBId.size() - 1);
      for (unsigned I = 1; I < BBId.size(); ++I)
        S->addCase(BBId[I].first, BBId[I].second);
    }
  }

  // C. Create initialization to LocalId, currSB and currBarrier in pElseBB
  // LocalId = 0
  // currSB = 0
  // currBarrier = id
  // And connect the pElseBB to the SyncBB with unconditional jump.
  {
    IRBuilder<> B(InnerMost);
    B.SetCurrentDebugLocation(DL);
    createSetCurrSBIndex(ConstZero, B);
    if (UniqueID) {
      createSetCurrBarrierId(UniqueID, B);
    }
    B.CreateBr(SyncBB);
  }
  return InnerMost;
}

void KernelBarrierImpl::replaceSyncInstructions() {
  // Run over all sync instructions and split its basic-block
  // in order to create an empty basic-block previous to the sync basic block.
  unsigned ID = 0;
  std::stringstream Name;
  for (Instruction *Inst : *SyncInstructions) {
    BasicBlock *LoopHeaderBB = Inst->getParent();
    Name.str("");
    Name << "SyncBB" << ID++;
    BasicBlock *LoopEntryBB = Inst->getParent()->splitBasicBlock(
        BasicBlock::iterator(Inst), Name.str());
    PreSyncLoopHeader[LoopEntryBB] = LoopHeaderBB;
    InstructionsToRemove.push_back(Inst);
  }

  for (Instruction *Inst : *SyncInstructions) {
    DebugLoc DL = Inst->getDebugLoc();
    unsigned int Id = DPB->getUniqueID(Inst);
    Value *UniqueID = ConstantInt::get(I32Ty, APInt(32, Id));
    BasicBlock *SyncBB = Inst->getParent();
    BasicBlock *PreSyncBB = PreSyncLoopHeader[SyncBB];
    assert(PreSyncBB && "SyncBB assumed to have sync loop header basic block!");
    if (SyncType::DummyBarrier == DPB->getSyncType(Inst)) {
      // This is a dummy barrier replace with the following
      // LocalId = 0
      // currSB = 0
      // currBarrier = Id
      IRBuilder<> B(&*PreSyncBB->begin());
      unsigned NumDimsToConstZero = getNumDims();
      for (unsigned Dim = 0; Dim < NumDimsToConstZero; ++Dim) {
        createSetLocalId(Dim, ConstZero, B);
      }
      createSetCurrSBIndex(ConstZero, B);
      createSetCurrBarrierId(UniqueID, B);
      continue;
    }
    // This is a barrier instruction.
    // For the innermost loop, replace with the following code
    // if (LocalId.0 < GroupSize.0) {
    //   LocalId.0+=VecWidth
    //   switch (currBarrier) {
    //     case i: goto barrier_i;
    //   }
    // } else {
    //   LocalIdi.0 = 0;
    //   currBarrier = id
    //   if (LocalId.1 < GroupSize.1) {
    //    LocalId.1+=1
    //   } else {
    //    LocalId.1 = 0;
    //    if (LocalId.2 < GroupSize.2) {
    //     LocalId.2+=1
    //   }
    // }

    BarrierBBIdListTy BBId;
    // Create List of barrier label that may be jumped to.
    DataPerBarrier::BarrierRelated *Related =
        &DPB->getBarrierPredecessors(Inst);
    InstSet *SyncPreds = &Related->RelatedBarriers;
    for (Instruction *SyncInst : *SyncPreds) {
      unsigned int PredId = DPB->getUniqueID(SyncInst);
      BBId.push_back(
          std::make_pair(ConstantInt::get(*Context, APInt(32, PredId)),
                         SyncInst->getParent()));
    }
    createBarrierLatch(PreSyncBB, SyncBB, BBId, UniqueID, DL);
  }
}

void KernelBarrierImpl::deduplicateCurrSBIndexInsts(Function &F) {
  auto It = FuncToCurrSBIndexInstsMap.find(&F);
  if (It == FuncToCurrSBIndexInstsMap.end())
    return;

  DenseMap<BasicBlock *, SmallVector<Instruction *, 16>> BBToSBIndexInstsMap;
  for (auto *I : It->second)
    BBToSBIndexInstsMap[I->getParent()].push_back(I);
  for (auto &[BB, SBIndexInsts] : BBToSBIndexInstsMap) {
    if (SBIndexInsts.size() == 1)
      continue;
    // Find the first CurrSBIndex according to order in the basic block.
    Instruction *First = SBIndexInsts[0];
    for (unsigned Idx = 1; Idx < SBIndexInsts.size(); ++Idx)
      if (SBIndexInsts[Idx]->comesBefore(First))
        First = SBIndexInsts[Idx];
    for (auto *I : SBIndexInsts) {
      if (I == First)
        continue;
      I->replaceAllUsesWith(First);
      InstructionsToRemove.push_back(I);
    }
  }
}

void KernelBarrierImpl::createBarrierKeyValues(Function *Func,
                                               bool /*HasNoInternalCalls*/) {
  BarrierKeyValues *KeyValues = &BarrierKeyValuesPerFunction[Func];

  const auto AllocaAddrSpace = DL->getAllocaAddrSpace();

  KeyValues->TheFunction = Func;
  unsigned NumDims = computeNumDim(Func);
  KeyValues->NumDims = NumDims;
  Instruction *InsertBefore = &*Func->getEntryBlock().begin();
  // Add currBarrier alloca.
  KeyValues->CurrBarrierId =
      new AllocaInst(Type::getInt32Ty(*Context), AllocaAddrSpace,
                     "pCurrBarrier", InsertBefore);

  // Will hold the index in special buffer and will be increased by stride size.
  KeyValues->CurrSBIndex =
      new AllocaInst(SizeTTy, AllocaAddrSpace, "pCurrSBIndex", InsertBefore);

  if (!HasTLSGlobals) {
    // get_local_id()
    KeyValues->LocalIdValues =
        new AllocaInst(LocalIdArrayTy, AllocaAddrSpace,
                       "pLocalIds", InsertBefore);
  }

  // get_special_buffer()
  KeyValues->SpecialBufferValue = Utils.createGetSpecialBuffer(InsertBefore);

  // get_local_size()
  for (unsigned i = 0; i < NumDims; ++i)
    KeyValues->LocalSize[i] = Utils.createGetLocalSize(i, InsertBefore);

  unsigned int StructureSize = DPV->getStrideSize(Func);
  KeyValues->StructureSizeValue =
      ConstantInt::get(SizeTTy, APInt(SizeT, StructureSize));
  KeyValues->CurrentVectorizedWidthValue =
      ConstantInt::get(SizeTTy, Utils.getFunctionVectorizationWidth(Func));
}

void KernelBarrierImpl::getBarrierKeyValues(Function *Func) {
  CurrentFunction = Func;
  assert(BarrierKeyValuesPerFunction.count(Func) &&
         "initiation of argument values is broken");
  CurrentBarrierKeyValues = &BarrierKeyValuesPerFunction[Func];
}

SmallVector<Instruction *>
KernelBarrierImpl::getInstructionsToInsertBefore(Instruction *Inst,
                                                 PHINode *PhiNode) {
  auto PrevBBs = BarrierUtils::findBasicBlocksOfPhiNode(Inst, PhiNode);
  SmallVector<Instruction *, 1> InsertBefores;
  for (auto *BB : PrevBBs) {
    if (BB != Inst->getParent()) {
      InsertBefores.push_back(BB->getTerminator());
    }
  }
  return InsertBefores;
}

Value *KernelBarrierImpl::getAddressInSpecialBuffer(unsigned int Offset,
                                                    PointerType *Ty,
                                                    Instruction *InsertBefore,
                                                    const DebugLoc *DB) {
  Value *OffsetVal = ConstantInt::get(SizeTTy, APInt(SizeT, Offset));
  // If hit this assert then need to handle PHINode!
  assert(!isa<PHINode>(InsertBefore) &&
         "cannot add instructions before a PHI node!");
  IRBuilder<> B(InsertBefore);
  if (DB)
    B.SetCurrentDebugLocation(*DB);
  // Calculate the pointer of the given Offset for LocalId in the special
  // buffer.
  Instruction *CurrSB = createGetCurrSBIndex(B);
  FuncToCurrSBIndexInstsMap[CurrSB->getFunction()].push_back(CurrSB);
  Value *SBIndex = B.CreateNUWAdd(CurrSB, OffsetVal, "SB_LocalId_Offset");
  Value *Idxs[1] = {SBIndex};
  Value *AddrInSBinBytes =
      B.CreateInBoundsGEP(Utils.getSpecialBufferValueTy(),
                          CurrentBarrierKeyValues->SpecialBufferValue,
                          ArrayRef<Value *>(Idxs), "pSB_LocalId");
  return AddrInSBinBytes;
}

// TODO: Since ResolveVariableTIDCall ran before this pass, we won't encounter
// variable TID call. This logic can be removed.
Instruction *KernelBarrierImpl::createOOBCheckGetLocalId(CallInst *Call) {
  // if we are going in this path, then no chance that we can run less than 3D
  //
  // Create three basic blocks to contain the dim check as follows
  // entry: (old basic block tail)
  //   %0 = icmp ult i32 %dimndx, MAX_WORK_DIM
  //   br i1 %0, label %get.wi.properties, label %split.continue
  //
  // get.wi.properties:  (new basic block in case of in bound)
  //   ... ; load the property
  //   br label %split.continue
  //
  // split.continue:  (the second half of the splitted basic block head)
  //   %4 = phi i32 [ %res, %get.wi.properties ], [ out-of-bound-value, %entry ]

  BasicBlock *Block = Call->getParent();
  Function *F = Block->getParent();
  // First need to split the current basic block to two BB's and create new BB.
  BasicBlock *GetWIProperties =
      BasicBlock::Create(*Context, "get.wi.properties", F);
  BasicBlock *SplitContinue =
      Block->splitBasicBlock(BasicBlock::iterator(Call), "split.continue");

  // A.change the old basic block to the detailed entry
  // Entry:1. remove the unconditional jump instruction.
  Block->getTerminator()->eraseFromParent();

  // Entry:2. add the entry tail code (as described up).
  {
    IRBuilder<> B(Block);
    B.SetCurrentDebugLocation(Call->getDebugLoc());
    ConstantInt *MaxWorkDimI32 =
        ConstantInt::get(*Context, APInt(32U, uint64_t(MAX_WORK_DIM), false));
    Value *CheckIndex = B.CreateICmpULT(Call->getArgOperand(0), MaxWorkDimI32,
                                        "check.index.inbound");
    B.CreateCondBr(CheckIndex, GetWIProperties, SplitContinue);
  }

  // B.Build the get.wi.properties block
  // Now retrieve address of the DIM count.

  IRBuilder<> B(GetWIProperties);
  B.SetCurrentDebugLocation(Call->getDebugLoc());
  Value *LocalIds =
      TLSLocalIds ? TLSLocalIds : CurrentBarrierKeyValues->LocalIdValues;
  Instruction *Result = createGetLocalId(LocalIds, Call->getArgOperand(0), B);
  B.CreateBr(SplitContinue);

  // C.Create Phi node at the first of the splitted BB.
  PHINode *AttrResult = PHINode::Create(IntegerType::get(*Context, SizeT), 2,
                                        "", SplitContinue->getFirstNonPHI());
  AttrResult->addIncoming(Result, GetWIProperties);
  // The overflow value.
  AttrResult->addIncoming(ConstZero, Block);
  AttrResult->setDebugLoc(Call->getDebugLoc());
  return AttrResult;
}

Value *KernelBarrierImpl::resolveGetLocalIDCall(CallInst *Call) {
  Value *Dimension = Call->getOperand(0);
  if (ConstantInt *C = dyn_cast<ConstantInt>(Dimension)) {
    uint64_t Dim = C->getZExtValue();
    if (Dim >= MAX_WORK_DIM) {
      // OpenCL Spec says to return zero for OOB dim value.
      return ConstZero;
    }
    // assert(BarrierKeyValuesPerFunction[Func].NumDims > Dim);
    IRBuilder<> B(Call);
    return createGetLocalId(Dim, B);
  }
  // assert(BarrierKeyValuesPerFunction[Func].NumDims == MAX_WORK_DIM);
  return createOOBCheckGetLocalId(Call);
}

bool KernelBarrierImpl::fixGetWIIdFunctions(Module & /*M*/) {
  // clear container for new iteration on new function.
  InstructionsToRemove.clear();

  std::string Name;
  // Find all get_local_id instructions.
  CompilationUtils::InstVec &GetLIDInstructions = Utils.getAllGetLocalId();
  for (Instruction *I : GetLIDInstructions) {
    CallInst *OldCall = cast<CallInst>(I);
    Function *Func = OldCall->getFunction();
    if (!HasTLSGlobals)
      getBarrierKeyValues(Func);
    else
      CurrentFunction = Func;
    Value *LID = resolveGetLocalIDCall(OldCall);
    OldCall->replaceAllUsesWith(LID);
    InstructionsToRemove.push_back(OldCall);
  }

  // Maps [Function, ConstDimension] --> BaseGlobalId.
  typedef std::pair<Function *, ConstantInt *> FuncDimPair;
  std::map<FuncDimPair, Value *> FuncToBaseGID;
  // Find all get_global_id instructions.
  CompilationUtils::InstVec &GetGIDInstructions = Utils.getAllGetGlobalId();
  for (auto *I : GetGIDInstructions) {
    CallInst *OldCall = cast<CallInst>(I);
    Function *Func = OldCall->getFunction();
    if (!HasTLSGlobals)
      getBarrierKeyValues(Func);
    else
      CurrentFunction = Func;
    Value *BaseGID = nullptr;
    Value *Dim = OldCall->getOperand(0);
    // Computation of BaseGID: If the dimension is a constant, cache it and
    // reuse in function.
    if (ConstantInt *ConstDim = dyn_cast<ConstantInt>(Dim)) {
      FuncDimPair Key = std::make_pair(Func, ConstDim);
      Value *&Val = FuncToBaseGID[Key];
      if (!Val) {
        Val = Utils.createGetBaseGlobalId(Dim, &*Func->getEntryBlock().begin());
      }
      BaseGID = Val;
    } else
      BaseGID = Utils.createGetBaseGlobalId(Dim, OldCall);

    auto KIMD = SYCLKernelMetadataAPI::KernelInternalMetadataAPI(Func);
    // Non-kernel function doesn't have NoBarrierPath metadata.
    if (KIMD.NoBarrierPath.hasValue() && KIMD.NoBarrierPath.get()) {
      OldCall->replaceAllUsesWith(BaseGID);
    } else {
      Value *LID = resolveGetLocalIDCall(OldCall);
      // Replace get_global_id(arg) with global_base_id + local_id.
      Name = CompilationUtils::AppendWithDimension("GlobalID_", Dim);
      Instruction *GlobalID =
          BinaryOperator::CreateAdd(LID, BaseGID, Name, OldCall);
      GlobalID->setDebugLoc(OldCall->getDebugLoc());
      OldCall->replaceAllUsesWith(GlobalID);
    }
    InstructionsToRemove.push_back(OldCall);
  }

  // Remove all instructions in InstructionsToRemove.
  return eraseAllToRemoveInstructions();
}

void KernelBarrierImpl::fixNonInlineFunction(Function *FuncToFix) {
  // TODO: do we need to set DebugLoc for these instructions?
  // Get key values for this functions.
  getBarrierKeyValues(FuncToFix);

  unsigned int NumOfArgs = FuncToFix->getFunctionType()->getNumParams();
  // Use Offsets instead of original parameters
  Function::arg_iterator ArgIter = FuncToFix->arg_begin();
  for (unsigned int i = 0; i < NumOfArgs; ++i, ++ArgIter) {
    Value *ArgVal = &*ArgIter;
    if (DPV->hasOffset(ArgVal))
      fixArgumentUsage(ArgVal);
  }
  if (DPV->hasOffset(FuncToFix)) {
    unsigned int Offset = DPV->getOffset(FuncToFix);

    std::vector<BasicBlock *> VecBB;
    for (BasicBlock &BB : *FuncToFix)
      VecBB.push_back(&BB);
    // Run over all basic blocks of the new function and handle return
    // terminators
    for (BasicBlock *BB : VecBB) {
      ReturnInst *RetInst = dyn_cast<ReturnInst>(BB->getTerminator());
      if (!RetInst) {
        // It is not return instruction terminator, check next basic block
        continue;
      }
      Value *RetVal = RetInst->getOperand(0);
      Instruction *NextInst;
      Instruction *Inst = dyn_cast<Instruction>(RetVal);
      CallInst *CI = dyn_cast<CallInst>(RetVal);
      Function *Func = nullptr;
      if (CI != nullptr)
        Func = CI->getCalledFunction();
      // If there is uniform work group builtin, we need to store return value
      // in the special buffer.
      std::string FuncName;
      if (Func) {
        FuncName = Func->getName().str();
        if (CompilationUtils::hasWorkGroupFinalizePrefix(FuncName))
          FuncName = CompilationUtils::removeWorkGroupFinalizePrefix(FuncName);
      }
      if ((Inst != nullptr) &&
          !(Func != nullptr &&
            CompilationUtils::isWorkGroupUniform(FuncName))) {
        // Find next instruction so we can create new instruction before it.
        NextInst = &*(++BasicBlock::iterator(Inst));
        if (isa<PHINode>(NextInst)) {
          // NextInst is a PHINode, find first non PHINode to add instructions
          // before it.
          NextInst = NextInst->getParent()->getFirstNonPHI();
        }
      } else {
        // In this case the return value is not an instruction and
        // it cannot be assumed that it is inside the barrier loop.
        // Thus, need to create a new barrier loop that store this value
        // in the special buffer, that is why we needed to find the values:
        //  CurrSBIndex, LocalIdValue, WIIterationCountValue
        // Before:
        //   BB:
        //       ret RetVal
        // After:
        //   BB:
        //       br loopBB
        //   loopBB:
        //       pSB[pCurrSBValue+Offset] = RetVal
        //       cond LocalId < IterCount
        //       LocalId++
        //       pCurrSBValue += Stride
        //       br cond, loopBB, RetBB
        //   RetBB:
        //       ret RetVal
        BasicBlock *LoopBB =
            BB->splitBasicBlock(BasicBlock::iterator(RetInst), "LoopBB");
        BasicBlock *RetBB = LoopBB->splitBasicBlock(LoopBB->begin(), "RetBB");
        BarrierBBIdListTy BBId(
            1,
            std::make_pair(ConstantInt::get(*Context, APInt(32, 0)), LoopBB));
        DebugLoc DL = RetInst->getDebugLoc();
        Value *UniqueID = 0;
        createBarrierLatch(LoopBB, RetBB, BBId, UniqueID, DL);

        NextInst = LoopBB->getFirstNonPHI();
      }
      fixReturnValue(RetVal, Offset, NextInst);
    }
  }
}

void KernelBarrierImpl::fixArgumentUsage(Value *OriginalArg) {
  assert((!DPV->isOneBitElementType(OriginalArg) ||
          !isa<VectorType>(OriginalArg->getType())) &&
         "OriginalArg with base type i1!");

  // function argument with debug info will be handled in fixAllocaAndDbg.
  if (HasTLSGlobals && !CompilationUtils::findDbgUses(OriginalArg).empty())
    return;

  // offset in special buffer to load the argument value from.
  unsigned OffsetArg = DPV->getOffset(OriginalArg);

  InstSet UserInsts;
  for (User *U : OriginalArg->users()) {
    Instruction *UserInst = dyn_cast<Instruction>(U);
    UserInsts.insert(UserInst);
  }

  auto InsertLoadedValue = [&](Instruction *InsertBefore, const DebugLoc &DB) {
    // In this case we will always get a valid offset and need to load the
    // argument from the special buffer using the offset corresponding argument.
    PointerType *Ty =
        OriginalArg->getType()->getPointerTo(SPECIAL_BUFFER_ADDR_SPACE);
    Value *AddrInSpecialBuffer =
        getAddressInSpecialBuffer(OffsetArg, Ty, InsertBefore, &DB);
    auto *LoadedValue =
        new LoadInst(OriginalArg->getType(), AddrInSpecialBuffer, "loadedValue",
                     InsertBefore);
    LoadedValue->setDebugLoc(DB);
    return LoadedValue;
  };

  for (Instruction *UserInst : UserInsts) {
    assert(UserInst &&
           "Something other than Instruction is using function argument!");
    const DebugLoc &DB = UserInst->getDebugLoc();
    if (auto *PhiNode = dyn_cast<PHINode>(UserInst)) {
      auto PrevBBs =
          BarrierUtils::findBasicBlocksOfPhiNode(OriginalArg, PhiNode);
      for (auto *PrevBB : PrevBBs) {
        auto *LoadedValue = InsertLoadedValue(PrevBB->getTerminator(), DB);
        PhiNode->setIncomingValueForBlock(PrevBB, LoadedValue);
      }
    } else {
      auto *LoadedValue = InsertLoadedValue(UserInst, DB);
      UserInst->replaceUsesOfWith(OriginalArg, LoadedValue);
    }
  }
}

void KernelBarrierImpl::fixReturnValue(Value *RetVal, unsigned int OffsetRet,
                                       Instruction *InsertBefore) {
  assert((!DPV->isOneBitElementType(RetVal) ||
          !isa<VectorType>(RetVal->getType())) &&
         "RetVal with base type i1!");
  // RetVal might be a result of calling other function itself
  // in such case no need to handle it here as it will be saved
  // to the special buffer by the called function itself.
  // Calculate the pointer of the current special in the special buffer.
  PointerType *Ty = RetVal->getType()->getPointerTo(SPECIAL_BUFFER_ADDR_SPACE);
  Value *AddrInSpecialBuffer =
      getAddressInSpecialBuffer(OffsetRet, Ty, InsertBefore, nullptr);
  // Add Store instruction after the value instruction.
  auto *SI = new StoreInst(RetVal, AddrInSpecialBuffer, InsertBefore);
  SI->setDebugLoc(InsertBefore->getDebugLoc());
}

void KernelBarrierImpl::fixCallInstruction(CallInst *CallToFix) {
  Function *CalledFunc = CallToFix->getCalledFunction();
  assert(CalledFunc && "Call instruction has no called function");
  Function *Func = CallToFix->getFunction();

  // Get key values for this functions.
  getBarrierKeyValues(Func);

  const DebugLoc &DB = CallToFix->getDebugLoc();
  Instruction *InsertBefore = nullptr;
  Function::arg_iterator ArgIter = CalledFunc->arg_begin();
  for (CallInst::const_op_iterator Opi = CallToFix->arg_begin(),
                                   Ope = CallToFix->arg_end();
       Opi != Ope; ++Opi, ++ArgIter) {
    if (!DPV->hasOffset(&*ArgIter))
      continue;

    if (!InsertBefore) {
      // Split sync instruction basic-block that contains the call instruction.
      BasicBlock *PreBB = CallToFix->getParent();
      BasicBlock::iterator FirstInst = PreBB->begin();
      assert(DPB->getSyncInstructions(Func).contains(&*FirstInst) &&
             "assume first instruction to be sync instruction");
      BasicBlock *CallBB = PreBB->splitBasicBlock(FirstInst, "CallBB");
      InsertBefore = PreBB->getTerminator();
      OldToNewSyncBBMap[Func][PreBB] = CallBB;
    }
    // Need to handle Operand.
    Value *OpVal = *Opi;
    unsigned int Offset = DPV->getOffset(&*ArgIter);

    // Calculate the pointer of the current special in the special buffer.
    PointerType *Ty = OpVal->getType()->getPointerTo(SPECIAL_BUFFER_ADDR_SPACE);
    Value *AddrInSpecialBuffer =
        getAddressInSpecialBuffer(Offset, Ty, InsertBefore, &DB);
    // Add Store instruction before the synchronize instruction (in the pre
    // basic block)
    StoreInst *SI = new StoreInst(OpVal, AddrInSpecialBuffer, InsertBefore);
    SI->setDebugLoc(DB);
  }
  // Check if return value has usages.
  if (!CallToFix->getNumUses())
    return;

  if (!DPV->hasOffset(CalledFunc))
    return;
  // Need to handle return value.

  // Validate that next basic block is a synchronize basic block.
  BasicBlock *CallBB = CallToFix->getParent();
  BranchInst *BrInst = dyn_cast<BranchInst>(CallBB->getTerminator());
  assert(BrInst && BrInst->getNumSuccessors() == 1 &&
         "callInst BB has more than one successor");
  BasicBlock::iterator FirstInst = BrInst->getSuccessor(0)->begin();
  assert(DPB->getSyncInstructions(Func).contains(&*FirstInst) &&
         "assume first instruction to be sync instruction");
  // Find next instruction so we can create new instruction before it.
  Instruction *NextInst = &*(++FirstInst);

  unsigned int Offset = DPV->getOffset(CalledFunc);

  // Calculate the pointer of the current special in the special buffer.
  PointerType *Ty =
      CallToFix->getType()->getPointerTo(SPECIAL_BUFFER_ADDR_SPACE);
  Value *AddrInSpecialBuffer =
      getAddressInSpecialBuffer(Offset, Ty, NextInst, &DB);
  // Add Load instruction from special buffer at function offset.
  LoadInst *LoadedValue = new LoadInst(
      CallToFix->getType(), AddrInSpecialBuffer, "loadedValue", NextInst);
  LoadedValue->setDebugLoc(DB);

  if (DPV->hasOffset(CallToFix)) {
    // CallInst return value has an offset in the special buffer
    // Store the value to this offset.
    unsigned int OffsetRet = DPV->getOffset(CallToFix);

    // Calculate the pointer of the current special in the special buffer
    Value *AddrInSpecialBuffer =
        getAddressInSpecialBuffer(OffsetRet, Ty, NextInst, &DB);
    // Add Store instruction to special buffer at return value offset
    StoreInst *SI = new StoreInst(LoadedValue, AddrInSpecialBuffer, NextInst);
    SI->setDebugLoc(DB);
  } else {
    CallToFix->replaceAllUsesWith(LoadedValue);
  }
}

bool KernelBarrierImpl::eraseAllToRemoveInstructions() {
  // Remove all instructions in InstructionsToRemove.
  for (Instruction *Inst : InstructionsToRemove)
    Inst->eraseFromParent();
  return !InstructionsToRemove.empty();
}

unsigned KernelBarrierImpl::computeNumDim(Function *F) {
  auto MaxWGDimMDApi =
      SYCLKernelMetadataAPI::KernelInternalMetadataAPI(F).MaxWGDimensions;
  if (MaxWGDimMDApi.hasValue()) {
    return MaxWGDimMDApi.get();
  }
  return MAX_WORK_DIM;
}

static size_t
getCalculatedPrivateSize(Function *Func,
                         DenseMap<Function *, size_t> &FnPrivSize) {
  // External function or function pointer.
  if (!Func || Func->isDeclaration())
    return 0;
  if (!FnPrivSize.contains(Func)) {
    LLVM_DEBUG(dbgs() << "No private size calculated for function "
                      << Func->getName() << "\n");
    return 0;
  }

  LLVM_DEBUG(dbgs() << "Get private size for function " << Func->getName()
                    << ": " << FnPrivSize[Func] << "\n");

  return FnPrivSize[Func];
}

void KernelBarrierImpl::calculateDirectPrivateSize(
    Module &M, FuncSet &FnsWithSync,
    DenseMap<Function *, size_t> &DirectPrivateSizeMap) {
  for (auto &F : *const_cast<Module *>(&M)) {
    if (F.isDeclaration())
      continue;

    DirectPrivateSizeMap[&F] =
        (AddrAllocaSize.contains(&F) ? (size_t)AddrAllocaSize[&F] : 0) +
        (FnsWithSync.contains(&F) ? 0 : (size_t)DPV->getStrideSize(&F));
  }
}

void KernelBarrierImpl::calculatePrivateSize(
    Module &M, FuncSet &FnsWithSync,
    DenseMap<Function *, size_t> &PrivateSizeMap) {
  DenseMap<Function *, size_t> DirectPrivateSizeMap;
  calculateDirectPrivateSize(M, FnsWithSync, DirectPrivateSizeMap);
  // Use post order traversal to calculate function' non-barrier memory usage.
  CallGraph CG{M};
  CompilationUtils::calculateMemorySizeWithPostOrderTraversal(
      CG, DirectPrivateSizeMap, PrivateSizeMap);
}

void KernelBarrierImpl::updateStructureStride(Module &M,
                                              FuncSet &FunctionsWithSync) {
  // Collect Functions to process.
  DenseMap<Function *, size_t> FuncToPrivSize;

  auto TodoList = BarrierUtils::getAllKernelsAndVectorizedCounterparts(
      SYCLKernelMetadataAPI::KernelList(&M).getList());

  calculatePrivateSize(M, FunctionsWithSync, FuncToPrivSize);

  // Get the kernels using the barrier for work group loops.
  for (auto *Func : TodoList) {
    auto KIMD = SYCLKernelMetadataAPI::KernelInternalMetadataAPI(Func);
    // Need to check if Vectorized Width Value exists, it is not guaranteed
    // that  Vectorized is running in all scenarios.
    int VecWidth =
        KIMD.VectorizedWidth.hasValue() ? KIMD.VectorizedWidth.get() : 1;
    unsigned int StrideSize = DPV->getStrideSize(Func);
    assert(VecWidth && "VecWidth should not be 0!");
    StrideSize = (StrideSize + VecWidth - 1) / VecWidth;

    auto PrivateSize = getCalculatedPrivateSize(Func, FuncToPrivSize);

    // CSSD100016517, CSSD100018743: workaround
    // Private memory is always considered to be non-uniform. I.e. it is not
    // shared by each WI per vector lane. If it is uniform (i.e. its content
    // doesn't depend on non-uniform values) the private memory query returns a
    // smaller value than actual private memory usage. This subtle is taken
    // into account in the query for the maximum work-group.
    if (KIMD.NoBarrierPath.get()) {
      KIMD.BarrierBufferSize.set(0);
      // If there are no barrier in the kernel, StrideSize is the kernel
      // body's private memory usage. So need to add sub-function's memory size.
      KIMD.PrivateMemorySize.set(StrideSize + PrivateSize -
                                 DPV->getStrideSize(Func));
    } else {
      KIMD.BarrierBufferSize.set(StrideSize);
      // If there are some barriers in the kernel, StrideSize is barrier
      // buffer size. So need to add non barrier private memory.
      KIMD.PrivateMemorySize.set(StrideSize + PrivateSize);
    }
    LLVM_DEBUG(dbgs() << "Set metadata for kernel " << Func->getName()
                      << ": BarrierBufferSize=" << KIMD.BarrierBufferSize.get()
                      << ", PrivateMemorySize=" << KIMD.PrivateMemorySize.get()
                      << '\n');
  }
}

PreservedAnalyses KernelBarrier::run(Module &M, ModuleAnalysisManager &MAM) {
  // Get Analysis data.
  auto *DPB = &MAM.getResult<DataPerBarrierAnalysis>(M);
  auto *DPV = &MAM.getResult<DataPerValueAnalysis>(M);
  KernelBarrierImpl Impl(DPB, DPV);
  if (!Impl.run(M))
    return PreservedAnalyses::all();
  PreservedAnalyses PA;
  PA.preserve<DataPerBarrierAnalysis>();
  PA.preserve<DataPerValueAnalysis>();
  PA.preserve<DominatorTreeAnalysis>();
  return PA;
}
