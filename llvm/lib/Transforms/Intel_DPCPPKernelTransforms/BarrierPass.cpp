//==--- BarrierPass.cpp - Main Barrier pass - C++ -*------------------------==//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/BarrierPass.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelBarrierUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelLoopUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/Local.h"

#include <set>
#include <sstream>
#include <vector>

#define DEBUG_TYPE "dpcpp-kernel-barrier"

INITIALIZE_PASS_BEGIN(KernelBarrier, DEBUG_TYPE,
                      "KernelBarrier Pass - Handle special values & replace "
                      "barrier/fiber with internal loop over WIs",
                      false, true)
INITIALIZE_PASS_DEPENDENCY(DataPerBarrier)
INITIALIZE_PASS_DEPENDENCY(DataPerValue)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_END(KernelBarrier, DEBUG_TYPE,
                    "KernelBarrier Pass - Handle special values & replace "
                    "barrier/fiber with internal loop over WIs",
                    false, true)

namespace llvm {
char KernelBarrier::ID = 0;
KernelBarrier::KernelBarrier(bool IsNativeDebug, bool UseTLSGlobals)
    : ModulePass(ID), DL(nullptr), Context(nullptr), SizeT(0), SizeTTy(nullptr),
      I32Ty(nullptr), UseTLSGlobals(UseTLSGlobals), LocalIdAllocTy(nullptr),
      LocalIds(nullptr), LocalIdArrayTy(nullptr), ConstZero(nullptr),
      ConstOne(nullptr), DPV(nullptr), AllocaValues(nullptr),
      SpecialValues(nullptr), CrossBarrierValues(nullptr), DPB(nullptr),
      SyncInstructions(nullptr), CurrentFunction(nullptr),
      CurrentBarrierKeyValues(nullptr), IsNativeDBG(IsNativeDebug) {
  std::fill(PtrLocalId, PtrLocalId + MaxNumDims, nullptr);
  initializeKernelBarrierPass(*llvm::PassRegistry::getPassRegistry());
}

bool KernelBarrier::runOnModule(Module &M) {
  // Get Analysis data.
  DPB = &getAnalysis<DataPerBarrier>();
  DPV = &getAnalysis<DataPerValue>();

  DL = &M.getDataLayout();

  // Initialize barrier utils class with current module.
  BarrierUtils.init(&M);
  // This call is needed to initialize vectorization widths.
  BarrierUtils.getAllKernelsWithBarrier();

  Context = &M.getContext();
  // Initialize the side of size_t.
  SizeT = M.getDataLayout().getPointerSizeInBits(0);
  SizeTTy = IntegerType::get(*Context, SizeT);
  I32Ty = IntegerType::get(*Context, 32);
  LocalIdArrayTy = ArrayType::get(SizeTTy, MaxNumDims);
  LocalIdAllocTy = PointerType::get(LocalIdArrayTy, 0);
  ConstZero = ConstantInt::get(SizeTTy, 0);
  ConstOne = ConstantInt::get(SizeTTy, 1);

  bool ModuleHasAnyInternalCalls = false;

  if (UseTLSGlobals) {
    // Add thread local variable for local ids.
    LocalIds = new GlobalVariable(M, LocalIdArrayTy, false,
                                  GlobalValue::LinkOnceODRLinkage,
                                  UndefValue::get(LocalIdArrayTy), "LocalIds",
                                  nullptr, GlobalValue::GeneralDynamicTLSModel);
    LocalIds->setAlignment(
        MaybeAlign(M.getDataLayout().getPreferredAlignment(LocalIds)));
  }

  // Find all functions that call synchronize instructions.
  FuncSet &FunctionsWithSync =
      BarrierUtils.getAllFunctionsWithSynchronization();

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
    // This is needed for optimize pLocalId calculation.
    bool HasNoInternalCalls = !BarrierUtils.doesCallModuleFunction(Func);
    ModuleHasAnyInternalCalls =
        ModuleHasAnyInternalCalls || !HasNoInternalCalls;
    createBarrierKeyValues(Func, HasNoInternalCalls);
  }

  // Fix non inlined internal functions that need special handling.
  // Run over functions with synchronize instruction:
  // 1. Handle call instructions to non-inline functions.
  for (Function *FuncToFix : FunctionsWithSync) {
    // Run over old users of pFuncToFix and prepare parameters as needed.
    for (auto *U : FuncToFix->users()) {
      CallInst *CI = dyn_cast<CallInst>(U);
      if (!CI)
        continue;
      // Skip non-kernel calls.
      if (!(CI->getFunction()->hasFnAttribute("sycl_kernel")))
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
  // 1. Handle Values from Group-A, Group-B.1 and Group-B.2
  // 2. Hanlde synchronize instructions.
  for (Function *FuncToFix : FunctionsWithSync) {
    runOnFunction(*FuncToFix);
  }

  // Update Map with structure stride size for each kernel.
  // fixAllocaValues may add new alloca.
  updateStructureStride(M, FunctionsWithSync);

  fixSynclessTIDUsers(M, FunctionsWithSync);
  // Fix get_local_id() and get_global_id() function calls.
  fixGetWIIdFunctions(M);

  return true;
}

bool KernelBarrier::runOnFunction(Function &F) {

  assert(!DPB->hasFiberInstruction(&F) &&
         "Handle case when having fiber instructions!");

  // Get key values for this functions.
  getBarrierKeyValues(&F);

  SyncInstructions = &DPB->getSyncInstructions(&F);

  SpecialValues = &DPV->getValuesToHandle(&F);
  AllocaValues = &DPV->getAllocaValuesToHandle(&F);
  CrossBarrierValues = &DPV->getUniformValuesToHandle(&F);

  Instruction *InsertBefore = &*F.getEntryBlock().begin();
  if (IsNativeDBG) {
    // Move alloca instructions for locals/parameters for debugging purposes.
    for (Value *V : *AllocaValues) {
      AllocaInst *AI = cast<AllocaInst>(V);
      AI->moveBefore(InsertBefore);
    }
  }

  // Clear container for new iteration on new function.
  InstructionsToRemove.clear();
  PreSyncLoopHeader.clear();

  // Fix special values.
  fixSpecialValues();

  // Fix alloca values.
  fixAllocaValues(F);

  // Fix cross barrier uniform values.
  fixCrossBarrierValues(&*F.begin()->begin());

  // Replace sync instructions with initernal loop over WI ID.
  replaceSyncInstructions();

  // Remove all instructions in InstructionsToRemove.
  eraseAllToRemoveInstructions();

  return true;
}

void KernelBarrier::fixSynclessTIDUsers(Module &M,
                                        const FuncSet &FuncsWithSync) {
  std::vector<Function *> Worklist;
  std::set<Function *> FuncsToPatch;
  std::set<CallInst *> CIsToPatch;
  std::map<ConstantExpr *, Function *> ConstBitcastsToPatch;
  std::vector<std::string> TIDFuncNames;
  TIDFuncNames.push_back("__builtin_get_local_id");
  // Initialize the set of functions that need patching by selecting the
  // functions which contain direct calls to get_*_id() and are w/o syncs.
  for (unsigned I = 0; I < TIDFuncNames.size(); ++I) {
    Function *F = M.getFunction(TIDFuncNames[I]);
    if (!F)
      continue;
    for (auto *U : F->users()) {
      CallInst *CI = dyn_cast<CallInst>(U);
      if (!CI)
        continue;
      Function *CallingF = CI->getParent()->getParent();
      assert(CallingF);
      if (FuncsWithSync.count(CallingF))
        continue;
      FuncsToPatch.insert(CallingF);
      Worklist.push_back(CallingF);
    }
  }
  // Traverse back the call graph and find the set of all functions which need
  // to be patched. Also find the coresponding call intructions Function which
  // need to be patched are either:
  // 1. Functions w/o sync instructions which are direct calls of get_*_id()
  // (handled in loop above).
  // 2. Functions which are direct caller of functions described in 1. or
  // (recursively) functions defined in this line which do not contain sync
  // instructions.
  for (unsigned WorkListIdx = 0; WorkListIdx < Worklist.size(); ++WorkListIdx) {
    Function *CalledF = Worklist[WorkListIdx];
    for (auto *U :  CalledF->users()) {
      // OCL2.0. handle constant expression with bitcast of function pointer.
      if (ConstantExpr *CE = dyn_cast<ConstantExpr>(U)) {
        if ((CE->getOpcode() == Instruction::BitCast ||
             CE->getOpcode() == Instruction::AddrSpaceCast) &&
            CE->getType()->isPointerTy()) {
          ConstBitcastsToPatch[CE] = CalledF;
          continue;
        }
      }

      CallInst *CI = dyn_cast<CallInst>(U);
      if (!CI)
        continue;
      CIsToPatch.insert(CI);
      Function *CallingF = CI->getParent()->getParent();
      if (FuncsWithSync.count(CallingF))
        continue;
      FuncsToPatch.insert(CallingF);
      Worklist.push_back(CallingF);
    }
  }

  typedef std::map<Function *, Function *> F2FMap;
  F2FMap OldF2PatchedF;
  if (!UseTLSGlobals) {
    // Setup stuff needed for adding another argument to patched functions.
    SmallVector<Attribute, 1> NoAlias(
        1, Attribute::get(M.getContext(), Attribute::NoAlias));
    SmallVector<AttributeSet, 1> NewAttrs(1,
                                          AttributeSet::get(*Context, NoAlias));
    // Patch the functions.
    for (Function *OldF : FuncsToPatch) {
      Function *PatchedF = DPCPPKernelCompilationUtils::AddMoreArgsToFunc(
          OldF, LocalIdAllocTy, "pLocalIdValues", NewAttrs, "BarrierPass");
      OldF2PatchedF[OldF] = PatchedF;
      assert(!BarrierKeyValuesPerFunction.count(OldF));
      // So now the last arg of NewF is the base of the memory holding
      // LocalId's.
      // Find the last arg.
      Function::arg_iterator AI = PatchedF->arg_begin();
      for (unsigned I = 0; I < PatchedF->arg_size() - 1; ++I, ++AI) {
        // Skip over the original args.
      }
      BarrierKeyValuesPerFunction[PatchedF].TheFunction = PatchedF;
      BarrierKeyValuesPerFunction[PatchedF].LocalIdValues = &*AI;
    }
    // Patch the calls.
    for (CallInst *CI : CIsToPatch) {
      Function *CallingF = CI->getParent()->getParent();
      Function *CalledF = CI->getCalledFunction();
      assert(OldF2PatchedF.find(CalledF) != OldF2PatchedF.end());
      Function *PatchedF = OldF2PatchedF[CalledF];
      // Use calling functions's LocalIdValues as additional argument to
      // called function.
      assert(BarrierKeyValuesPerFunction.find(CallingF) !=
             BarrierKeyValuesPerFunction.end());
      Value *NewArg =
          BarrierKeyValuesPerFunction.find(CallingF)->second.LocalIdValues;
      SmallVector<Value *, 1> NewArgs(1, NewArg);
      DPCPPKernelCompilationUtils::AddMoreArgsToCall(CI, NewArgs, PatchedF);
    }
  }

  // Patch the constant function ptr addr bitcasts. Used in OCL20. Extended
  // execution.
  for (auto &KV : ConstBitcastsToPatch) {
    ConstantExpr *CE = KV.first;
    Function *F = KV.second;

    if (!UseTLSGlobals) {
      assert(OldF2PatchedF.find(F) != OldF2PatchedF.end() &&
             "Expected to find patched function in map");
      F = OldF2PatchedF[F];
    }

    // this case happens when global block variable is used
    Constant *NewCE = ConstantExpr::getPointerCast(F, CE->getType());
    CE->replaceAllUsesWith(NewCE);
  }
}

void KernelBarrier::findSyncBBSuccessors() {
  BasicBlockToBasicBlockSetTy SyncBBSuccs;
  for (Instruction *SyncI : *SyncInstructions) {
    BasicBlock *BB = SyncI->getParent();
    SyncPerBB[BB] = SyncI;

    InstSet &SyncPreds = DPB->getBarrierPredecessors(SyncI).RelatedBarriers;
    for (Instruction *Pred : SyncPreds) {
      // Note Pred->getParent() may be different from Pred->getParent()
      // in DataPerBarrierPass, because barrier basic block may change when
      // splitting basic block, see FirstBB and CallBB in this pass.
      BasicBlock *PredBB = Pred->getParent();
      SyncBBSuccs[PredBB].insert(BB);
    }
  }

  // Run DFS to find successors recursively.
  for (auto &S : SyncBBSuccs) {
    BasicBlock *BB = S.first;
    SmallVector<BasicBlock *, 8> BBsToHandle;
    for (BasicBlock *Succ : S.second) {
      BBsToHandle.push_back(Succ);
      SyncBBSuccessors[BB].insert(Succ);
    }
    while (!BBsToHandle.empty()) {
      BasicBlock *V = BBsToHandle.pop_back_val();
      if (!SyncBBSuccs.count(V))
        continue;
      for (BasicBlock *Succ : SyncBBSuccs[V]) {
        if (SyncBBSuccessors[BB].count(Succ))
          continue;
        SyncBBSuccessors[BB].insert(Succ);
        BBsToHandle.push_back(Succ);
      }
    }
  }
}

BasicBlock *KernelBarrier::findNearestDominatorSyncBB(DominatorTree &DT,
                                                      BasicBlock *BB) {
  BasicBlock *Dominator = nullptr;
  for (BasicBlock *SyncBB : BBToPredSyncBB[BB]) {
    if (BB == SyncBB || !DT.dominates(SyncBB, BB))
      continue;
    if (Dominator) {
      // Skip if Dominator is SyncBB's successor.
      if (SyncBBSuccessors[SyncBB].count(Dominator))
        continue;
      Dominator = SyncBB;
      continue;
    }
    // Check that there isn't another barrier in any path from SyncBB to BB.
    auto noBarrierFromTo = [this](BasicBlock *SyncBB, BasicBlock *BB) -> bool {
      for (BasicBlock *Succ : SyncBBSuccessors[SyncBB]) {
        if (isPotentiallyReachable(Succ, BB))
          return false;
      }
      return true;
    };
    if (noBarrierFromTo(SyncBB, BB))
      Dominator = SyncBB;
  }
  return Dominator;
}

// This function binds alloca's user instruction to a basic block.
// It also does optimization based on dominance information so that we load
// address from new alloca only if necessary. For example, if both basick
// block A and B contain alloca's users, A dominates B and there is no barrier
// between A and B, we can bind users in B to A. Then we only need to load
// address in A.
// For the following IR, there is a barrier in for loop. There is no barrier
// between while.cond and while.body, we only need to load from %i.addr in
// while.cond and reuse it in while.body.
//
// for.body:
// "Barrier BB":
// while.cond:
//   %1 = load i32, i32* %i, align 4, !dbg !46
//   br i1 %cmp1, label %while.body, label %while.end, !dbg !45
// while.body:
//   store i32 1, i32* %i, align 4, !dbg !48
// while.end:
// for.inc:
//
// Output is:
//
// for.body:
// "Barrier BB":
// while.cond:
//   %SBIndex22 = load i64, i64* %pCurrSBIndex, align 8, !dbg !46
//   %SB_LocalId_Offset23 = add nuw i64 %SBIndex22, 40, !dbg !46
//   %11 = getelementptr inbounds i8, i8* %pSB, i64 %SB_LocalId_Offset23,
//       !dbg !46
//   %pSB_LocalId24 = bitcast i8* %11 to i32*, !dbg !46
//   store i32* %pSB_LocalId24, i32** %i.addr, align 8, !dbg !46
//   %12 = load i32*, i32** %i.addr, align 8, !dbg !46
//   call void @llvm.dbg.value(metadata i32* %12, metadata !43,
//       metadata !DIExpression(DW_OP_deref)), !dbg !41
//   %13 = load i32, i32* %12, align 4, !dbg !46
//   br i1 %cmp1, label %while.body, label %while.end, !dbg !45
// while.body:
//   store i32 1, i32* %12, align 4, !dbg !48
// while.end:
// for.inc:
void KernelBarrier::bindUsersToBasicBlock(
    AllocaInst *AI, DbgDeclareInst *DI,
    BasicBlockToInstructionMapVectorTy &BBUsers) {
  Function &F = *AI->getFunction();
  DominatorTree &DT = getAnalysis<DominatorTreeWrapperPass>(F).getDomTree();

  SmallVector<Instruction *, 8> UIs;
  for (User *U : AI->users()) {
    Instruction *UI = dyn_cast<Instruction>(U);
    assert(UI && "uses of alloca instruction is not an instruction!");
    UIs.push_back(UI);
  }

  BasicBlock *DIBB = nullptr;
  if (DI) {
    UIs.push_back(DI);
    DIBB = DI->getParent();
  }

  DenseMap<Instruction *, BasicBlock *> UIToInsertPointBB;
  BasicBlockSet BBs;
  for (Instruction *UI : UIs) {
    Instruction *InsertBefore = getInstructionToInsertBefore(AI, UI, false);
    // If UI is PHINode, BB is PHINode's previous basic block, otherwise
    // BB is UI's parent basic block.
    BasicBlock *BB = InsertBefore->getParent();
    UIToInsertPointBB[UI] = BB;
    BBs.insert(BB);

    if (SyncPerBB.count(BB))
      continue;

    // Collect BB's predecessors that are SyncBB
    if (!BBToPredSyncBB.count(BB) && DPB->hasPredecessors(BB)) {
      BasicBlockSet &Preds = DPB->getPredecessors(BB);
      for (BasicBlock *Pred : Preds) {
        if (SyncPerBB.count(Pred))
          BBToPredSyncBB[BB].push_back(Pred);
      }
      for (auto &OldToNewSync : OldToNewSyncBBMap[&F]) {
        BasicBlock *Pred = OldToNewSync.second;
        if (Preds.count(OldToNewSync.first) && SyncPerBB.count(Pred))
          BBToPredSyncBB[BB].push_back(Pred);
      }
    }

    // Find the nearest SyncBB that dominates BB
    if (!BBToNearestDominatorSyncBB.count(BB))
      BBToNearestDominatorSyncBB[BB] = findNearestDominatorSyncBB(DT, BB);
  }

  // FIXME IsSingleUserBB and it related code should be removed.
  // Currently a variable's new llvm.dbg.value intrinsic is correct after
  // removing IsSingleUserBB, however, a few debugger_test_type tests fail
  // because register is killed if the variable has only one use, making it
  // optimized-out at a later breakpoint.
  // DBG_VALUE renamable $rdx, $noreg, !"v", !DIExpression(DW_OP_deref)
  // MOV32mi killed renamable $rdx
  bool IsSingleUserBB = (BBs.size() == 1);

  // Map from user BB to its bound dominator that value loaded from AddrAI in
  // the dominator will be reused in the user BB.
  BasicBlockToBasicBlockTy BBBindToDominator;
  // Basic blocks that are bound to their nearest dominator SyncBB.
  SmallSet<BasicBlock *, 8> BBBindToNearestSyncDominator;

  for (BasicBlock *BB : BBs)
    BBBindToDominator[BB] = BB;

  // Bind user basic blocks to SyncBB
  for (auto &BBToDominator : BBToNearestDominatorSyncBB) {
    BasicBlock *BB = BBToDominator.first;
    BasicBlock *Dominator = BBToDominator.second;
    if (!Dominator)
      continue;
    // If Dominator is DIBB's predecessor, we shouldn't bind BB to Dominator
    // so that AI's debug scope is within DIBB.
    if (DIBB && isPotentiallyReachable(Dominator, DIBB) && !IsSingleUserBB)
      continue;

    BBBindToDominator[BB] = Dominator;
    BBBindToNearestSyncDominator.insert(BB);
  }

  if (IsSingleUserBB) {
    BasicBlock *BB = UIs[0]->getParent();
    if (BBBindToNearestSyncDominator.count(BB)) {
      if (BasicBlock *Pred = BB->getUniquePredecessor())
        BBBindToDominator[BB] = Pred;
    }
  }

  for (BasicBlock *BB : BBs) {
    DenseMap<BasicBlock *, bool> &HasBarrierTo = HasBarrierFromTo[BB];

    if (!BBToDominatedBBs.count(BB))
      DT.getDescendants(BB, BBToDominatedBBs[BB]);

    for (auto *DominatedBB : BBToDominatedBBs[BB]) {
      // Skip if DominatedBB is a SyncBB because binding to SyncBB is already
      // handled above.
      if (BB == DominatedBB || !BBs.count(DominatedBB) ||
          SyncPerBB.count(DominatedBB))
        continue;
      // Skip if binding is already done.
      if (BBBindToNearestSyncDominator.count(DominatedBB))
        continue;
      // Skip if DominatedBB is already bound to either the basic block
      // containing DbgDeclareInst or a basic block that BB doesn't dominate.
      if ((BB != DIBB) && (!DT.dominates(BB, BBBindToDominator[DominatedBB]) ||
                           BBBindToDominator[DominatedBB] == DIBB))
        continue;

      if (!HasBarrierTo.count(DominatedBB))
        HasBarrierTo[DominatedBB] =
            BarrierUtils.isCrossedByBarrier(*SyncInstructions, DominatedBB, BB);
      if (!HasBarrierTo[DominatedBB])
        BBBindToDominator[DominatedBB] = BB;
    }
  }
  for (Instruction *UI : UIs) {
    BasicBlock *BB = UIToInsertPointBB[UI];
    BBUsers[BBBindToDominator[BB]].push_back(UI);
  }
}

void KernelBarrier::fixAllocaValues(Function &F) {
  DIBuilder DIB(*F.getParent(), /*AllowUnresolved*/ false);
  uint64_t Addr[] = {llvm::dwarf::DW_OP_deref};
  DIExpression *Expr = DIB.createExpression(Addr);
  const DataLayout &DL = F.getParent()->getDataLayout();
  Instruction *AddrInsertBefore = &*F.getEntryBlock().begin();

  // Reset containers for the current function.
  SyncPerBB.clear();
  SyncBBSuccessors.clear();
  BBToDominatedBBs.clear();
  BBToPredSyncBB.clear();
  BBToNearestDominatorSyncBB.clear();
  HasBarrierFromTo.clear();

  // For the current function, find all successors of each basic block which
  // contains a synchronization instruction.
  findSyncBBSuccessors();

  for (Value *V : *AllocaValues) {
    AllocaInst *AI = dyn_cast<AllocaInst>(V);
    assert(AI && "container of alloca values has non AllocaInst value!");

    // Don't fix implicit GID.
    if (IsNativeDBG && BarrierUtils.isImplicitGID(AI))
      continue;

    // Insert new alloca which stores AI's address in special buffer.
    // AI's users will be replaced by result of load instruction from the new
    // alloca.
    StringRef allocaName = AI->getName();
    AllocaInst *AddrAI = new AllocaInst(AI->getType(), DL.getAllocaAddrSpace(),
                                        allocaName + ".addr", AddrInsertBefore);
    uint64_t ASize = AddrAI->getAllocationSizeInBits(DL).getValue() / 8;
    AddrAI->setAlignment(assumeAligned(ASize));
    AddrAllocaSize[&F] += ASize;

    // Collect debug intrinsic.
    TinyPtrVector<DbgDeclareInst *> DIs;
    if (IsNativeDBG) {
      for (DbgVariableIntrinsic *DVI : FindDbgAddrUses(AI)) {
        if (auto *DDI = dyn_cast<DbgDeclareInst>(DVI))
          DIs.push_back(DDI);
      }
    }
    // Only use the first DbgDeclareInst.
    DbgDeclareInst *DI = DIs.empty() ? nullptr : DIs.front();

    // Get offset of alloca value in special buffer.
    unsigned int Offset = DPV->getOffset(AI);

    // Bind AI's users to basic blocks that update AddrAI.
    // MapVector is used so that access order is deterministic.
    BasicBlockToInstructionMapVectorTy BBUsers;
    bindUsersToBasicBlock(AI, DI, BBUsers);

    // Now each user is bound to a basic block, in which we insert instruction
    // to load AI's address in special buffer to AddrAI, and replace the user
    // with result of the load instruction.
    for (auto &BBUser : BBUsers) {
      BasicBlock *BB = BBUser.first;
      Instruction *InsertBefore;
      if (SyncPerBB.count(BB)) {
        InsertBefore = SyncPerBB[BB]->getNextNode();
        assert(
            InsertBefore->getParent() == BB &&
            "sync instruction must not be the last instruction in the block");
      } else
        InsertBefore = BB->getFirstNonPHI();
      assert(InsertBefore && "InsertBefore is invalid");
      // Calculate the pointer of the current alloca in the special buffer
      Value *AddrInSpecialBuffer = getAddressInSpecialBuffer(
          Offset, AI->getType(), InsertBefore, &InsertBefore->getDebugLoc());
      IRBuilder<> Builder(InsertBefore);
      Builder.CreateStore(AddrInSpecialBuffer, AddrAI);
      LoadInst *LI = Builder.CreateLoad(AddrAI);
      if (IsNativeDBG && DI) {
        const DebugLoc *DB = &DI->getDebugLoc();
        DIB.insertDbgValueIntrinsic(LI, DI->getVariable(), Expr, DB->get(),
                                    InsertBefore);
      }
      for (Instruction *UI : BBUser.second) {
        if (!isa<DbgDeclareInst>(UI))
          UI->replaceUsesOfWith(AI, LI);
      }
    }

    InstructionsToRemove.push_back(AI);

    // Remove old DbgDeclareInst
    if (IsNativeDBG) {
      for (auto *DI : DIs)
        DI->eraseFromParent();
    }
  }
  DIB.finalize();
}

void KernelBarrier::fixSpecialValues() {
  for (Value *V : *SpecialValues) {
    Instruction *Inst = cast<Instruction>(V);

    const DebugLoc &DB = Inst->getDebugLoc();
    // This will hold the real type of this value in the special buffer.
    Type *TypeInSP = Inst->getType();
    bool OneBitBaseType = DPV->isOneBitElementType(Inst);
    if (OneBitBaseType) {
      // Base type is i1 need to ZEXT/TRUNC to/from i32.
      VectorType *VecType = dyn_cast<VectorType>(Inst->getType());
      if (VecType) {
        TypeInSP = FixedVectorType::get(IntegerType::get(*Context, 32),
                                    VecType->getNumElements());
      } else {
        TypeInSP = IntegerType::get(*Context, 32);
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
    PointerType *PtrType = TypeInSP->getPointerTo(SPECIAL_BUFFER_ADDR_SPACE);
    // Handle Special buffer only if it is not a call instruction.
    // Special buffer value of call instruction will be handled in the callee.
    CallInst *CI = dyn_cast<CallInst>(Inst);
    if (!(CI && DPV->hasOffset(CI->getCalledFunction()))) {
      // Calculate the pointer of the current special in the special buffer.
      Value *AddrInSpecialBuffer =
          getAddressInSpecialBuffer(Offset, PtrType, NextInst, &DB);
      Instruction *InstToStore =
          !OneBitBaseType ? Inst
                          : CastInst::CreateZExtOrBitCast(
                                Inst, TypeInSP, "ZEXT-i1Toi32", NextInst);
      // Need to set DebugLoc for the case is oneBitBaseType. It won't hart to
      // set Same DebugLoc for the other case, as DB = pInst->getDebugLoc();
      InstToStore->setDebugLoc(DB);
      // Add Store instruction after the value instruction.
      StoreInst *PtrStoreInst =
          new StoreInst(InstToStore, AddrInSpecialBuffer, NextInst);
      PtrStoreInst->setDebugLoc(DB);
    }

    InstSet UserInsts;
    // Save all uses of pInst and add them to a container before start handling
    // them!
    for (auto *U : Inst->users()) {
      Instruction *UserInst = cast<Instruction>(U);
      if (Inst->getParent() == UserInst->getParent()) {
        // This use of pInst is at the same basic block (no barrier cross so
        // far) assert( !isa<PHINode>(pUserInst) && "user instruction is a
        // PHINode and appears befre pInst in BB" );
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
    // Run over all saved user instructions and handle by adding
    // load instruction before each value use.
    for (Instruction *UserInst : UserInsts) {
      Instruction *InsertBefore =
          getInstructionToInsertBefore(Inst, UserInst, true);
      if (!InsertBefore) {
        // as no barrier in the middle, no need to load & replace the origin
        // value.
        continue;
      }
      const DebugLoc &DB = UserInst->getDebugLoc();
      // Calculate the pointer of the current special in the special buffer.
      Value *AddrInSpecialBuffer =
          getAddressInSpecialBuffer(Offset, PtrType, InsertBefore, &DB);
      Instruction *LoadedValue = new LoadInst(TypeInSP, AddrInSpecialBuffer,
                                              "loadedValue", InsertBefore);
      Instruction *RealValue =
          !OneBitBaseType
              ? LoadedValue
              : CastInst::CreateTruncOrBitCast(LoadedValue, Inst->getType(),
                                               "Trunc-i1Toi32", InsertBefore);
      LoadedValue->setDebugLoc(DB);
      RealValue->setDebugLoc(DB);
      // Replace the use of old value with the new loaded value from special
      // buffer.
      UserInst->replaceUsesOfWith(Inst, RealValue);
    }
  }
}

void KernelBarrier::fixCrossBarrierValues(Instruction *InsertBefore) {
  for (Value *V : *CrossBarrierValues) {
    Instruction *Inst = dyn_cast<Instruction>(V);
    assert(Inst && "Container of special values has non Instruction value!");
    // Find next instruction so we can create new instruction before it.
    Instruction *NextInst = &*(++BasicBlock::iterator(Inst));
    if (isa<PHINode>(NextInst)) {
      // NextInst is a PHINode, find first non PHINode to add instructions
      // before it.
      NextInst = NextInst->getParent()->getFirstNonPHI();
    }
    // Create alloca of value type at begining of function.
    AllocaInst *AI =
        new AllocaInst(Inst->getType(), DL->getAllocaAddrSpace(),
                       Inst->getName(), InsertBefore);
    // Add Store instruction after the value instruction.
    StoreInst *SI = new StoreInst(Inst, AI, NextInst);
    SI->setDebugLoc(Inst->getDebugLoc());

    InstSet UserInsts;
    // Save all uses of pInst and add them to a container before start handling
    // them!
    for (auto *U : Inst->users()) {
      Instruction *UserInst = dyn_cast<Instruction>(U);
      assert(UserInst && "uses of special instruction is not an instruction!");
      if (Inst->getParent() == UserInst->getParent() &&
          !isa<PHINode>(UserInst)) {
        // This use of pInst is at the same basic block (no barrier cross so
        // far).
        continue;
      }
      UserInsts.insert(UserInst);
    }
    // Run over all saved user instructions and handle by adding
    // load instruction before each value use.
    for (Instruction *UserInst : UserInsts) {
      Instruction *InsertBefore =
          getInstructionToInsertBefore(Inst, UserInst, true);
      if (!InsertBefore) {
        // As no barrier in the middle, no need to load & replace the origin
        // value.
        continue;
      }
      // Calculate the pointer of the current special in the special buffer.
      Instruction *LoadedValue =
          new LoadInst(AI->getAllocatedType(), AI,
                       "loadedValue", InsertBefore);
      LoadedValue->setDebugLoc(UserInst->getDebugLoc());
      // Replace the use of old value with the new loaded value from special
      // buffer.
      UserInst->replaceUsesOfWith(Inst, LoadedValue);
    }
  }
}

BasicBlock *KernelBarrier::createLatchNesting(unsigned Dim, BasicBlock *Body,
                                              BasicBlock *Dispatch, Value *Step,
                                              const DebugLoc &DL) {

  LLVMContext &C = Body->getContext();
  Function *F = Body->getParent();
  // BB that is jumped to if loop in current nesting finishes
  BasicBlock *LoopEnd =
      BasicBlock::Create(C, AppendWithDimension("LoopEnd_", Dim), F, Dispatch);

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

BasicBlock *KernelBarrier::createBarrierLatch(BasicBlock *PreSyncBB,
                                              BasicBlock *SyncBB,
                                              BarrierBBIdListTy &BBId,
                                              Value *UniqueID,
                                              const DebugLoc &DL) {
  Function *F = PreSyncBB->getParent();
  unsigned NumDims = getNumDims();
  // A. change the preSync basic block as follow
  // A(1). remove the unconditional jump instruction.
  PreSyncBB->getTerminator()->eraseFromParent();
  // Create then and else basic-blocks.
  BasicBlock *Dispatch = BasicBlock::Create(*Context, "Dispatch", F, SyncBB);
  BasicBlock *InnerMost = PreSyncBB;
  assert(CurrentBarrierKeyValues->CurrentVectorizedWidthValue);
  Value *LoopSteps[MaxNumDims] = {
      CurrentBarrierKeyValues->CurrentVectorizedWidthValue, ConstOne, ConstOne};
  for (unsigned I = 0; I < NumDims; ++I)
    InnerMost = createLatchNesting(I, InnerMost, Dispatch, LoopSteps[I], DL);

  // A(2). add the entry tail code
  // if(LocalId < WGSize[dim]) {Dispatch} else {ElseBB}.

  // B. Create LocalId++ and switch instruction in Dispatch.

  // Create "LocalId+=VectorizationWidth" code.

  // Create "CurrSBBase+=Stride" code.
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

  // C. Create initialization to LocalId, currSB and currBarrier in ElseBB.
  // LocalId = 0
  // currSB = 0
  // currBarrier = id
  // And connect the ElseBB to the SyncBB with unconditional jump.
  {
    IRBuilder<> B(InnerMost);
    B.SetCurrentDebugLocation(DL);
    createSetCurrSBIndex(ConstZero, B);
    if (UniqueID) {
      createSetCurrBarrierId(UniqueID, B);
    }
    B.CreateBr(SyncBB);
  }
  // Only if we are debugging, copy data into the stack from local buffer
  // for execution and copy data out when finished. This allows for proper
  // DWARF based debugging.
  if (IsNativeDBG) {
    createDebugInstrumentation(Dispatch, InnerMost);
  }
  return InnerMost;
}

void KernelBarrier::createDebugInstrumentation(BasicBlock *Then,
                                               BasicBlock *Else) {
  // Use the then and else blocks to copy local buffer data.
  Instruction &ThenFront = Then->front();
  Instruction &ElseFront = Else->front();

  // I add the function DebugCopy as a marker so it can be handled later
  // in LocalBuffers pass.
  // LocalBuffers pass is responsible for implementing __local variables
  // correctly in OpenCL
  // (ie. as work-group globals and not thread globals). I insert them
  // in these marked blocks
  // so that I know when I need to copy from the local buffer into the
  // thread local (global).
  // This is also how I know where the beginning of each work item
  // iteration is (in the presence
  // of barriers) which is where the copying occurs.

  // Maybe there is a better way, I'm not sure. The problem I found is
  // LocalBuffers finds all
  // uses of a __local variable and updates the references to a local
  // buffer memory location
  // rather then the thread specific global for which the __local
  // variable symbol is defined.
  // So any changes to __local variables would have to be delayed until
  // this pass or LocalBuffers
  // would have to behave very differently.

  // There is also a copy that occurs from the local buffer into the
  // global variable after each
  // use of the __local variable so that the thread specific global
  // stays updated. This is
  // independent of the function markers. This is done in LocalBuffers
  // pass.

  // This only allows for reading of __local variables and not setting.

  Type *Result = Type::getVoidTy(*Context);
  Module *M = Then->getParent()->getParent();
  FunctionCallee Func = M->getOrInsertFunction("DebugCopy.", Result);
  CallInst::Create(Func, "", &ThenFront);
  CallInst::Create(Func, "", &ElseFront);
}

void KernelBarrier::replaceSyncInstructions() {
  // Run over all sync instructions and split its basic-block
  // in order to create an empty basic-block previous to the sync basic block.
  unsigned ID = 0;
  std::stringstream Name;
  for (Instruction *Inst : *SyncInstructions) {
    assert(Inst && "sync instruction container contains non instruction!");
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
    SyncType SyncTy = DPB->getSyncType(Inst);
    BasicBlock *SyncBB = Inst->getParent();
    BasicBlock *PreSyncBB = PreSyncLoopHeader[SyncBB];
    assert(PreSyncBB &&
           "SyncBB assumed to have sync loop header basic block!");
    if (SyncTypeDummyBarrier == SyncTy) {
      // This is a dummy barrier replace with the following
      // LocalId = 0
      // currSB = 0
      // currBarrier = id.
      IRBuilder<> B(&*PreSyncBB->begin());
      unsigned NumDimsToZero = getNumDims();
      assert((!IsNativeDBG || NumDimsToZero == MaxNumDims) &&
             "Debugger requires local/global_id in all dimensions to be valid");
      for (unsigned Dim = 0; Dim < NumDimsToZero; ++Dim) {
        createSetLocalId(Dim, ConstZero, B);
      }
      createSetCurrSBIndex(ConstZero, B);
      createSetCurrBarrierId(UniqueID, B);
      continue;
    }
    // This is a barrier/fiber instruction.
    // For the innermost loop, replace with the following code:
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
    assert(!Related->HasFiberRelated &&
           "We reach here only if function has no fiber!");
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

void KernelBarrier::createBarrierKeyValues(Function *Func,
                                           bool HasNoInternalCalls) {
  BarrierKeyValues *BarrierKeyValuesPtr = &BarrierKeyValuesPerFunction[Func];

  const auto AllocaAddrSpace = DL->getAllocaAddrSpace();

  BarrierKeyValuesPtr->TheFunction = Func;
  unsigned NumDims = computeNumDim(Func);
  BarrierKeyValuesPtr->NumDims = NumDims;
  Instruction *InsertBefore = &*Func->getEntryBlock().begin();
  // Add currBarrier alloca.
  BarrierKeyValuesPtr->CurrBarrierId =
      new AllocaInst(Type::getInt32Ty(*Context), AllocaAddrSpace,
                     "pCurrBarrier", InsertBefore);

  // Will hold the index in special buffer and will be increased by stride size.
  BarrierKeyValuesPtr->CurrSBIndex =
      new AllocaInst(SizeTTy, AllocaAddrSpace, "pCurrSBIndex", InsertBefore);

  if (!UseTLSGlobals) {
    // get_local_id():
    BarrierKeyValuesPtr->LocalIdValues =
        new AllocaInst(LocalIdAllocTy->getElementType(), AllocaAddrSpace,
                       "pLocalIds", InsertBefore);
  }

  // get_special_buffer():
  BarrierKeyValuesPtr->SpecialBufferValue =
      BarrierUtils.createGetSpecialBuffer(InsertBefore);

  // get_local_size():
  for (unsigned i = 0; i < NumDims; ++i)
    BarrierKeyValuesPtr->LocalSize[i] =
        BarrierUtils.createGetLocalSize(i, InsertBefore);

  unsigned int StructureSize = DPV->getStrideSize(Func);
  BarrierKeyValuesPtr->StructureSizeValue =
      ConstantInt::get(SizeTTy, APInt(SizeT, StructureSize));
  BarrierKeyValuesPtr->CurrentVectorizedWidthValue = ConstantInt::get(
      SizeTTy, BarrierUtils.getKernelVectorizationWidth(Func));
}

void KernelBarrier::getBarrierKeyValues(Function *Func) {
  CurrentFunction = Func;
  assert(BarrierKeyValuesPerFunction.count(Func) &&
         "Initiation of argument values is broken");
  CurrentBarrierKeyValues = &BarrierKeyValuesPerFunction[Func];
}

Instruction *KernelBarrier::getInstructionToInsertBefore(Instruction *Inst,
                                                         Instruction *UserInst,
                                                         bool ExpectNULL) {
  if (!isa<PHINode>(UserInst)) {
    // UserInst is not a PHINode, we can insert instruction before it.
    return UserInst;
  }
  // UserInst is a PHINode, find previous basic block.
  BasicBlock *PrevBB =
      DPCPPKernelBarrierUtils::findBasicBlockOfUsageInst(Inst, UserInst);

  if (ExpectNULL && PrevBB == Inst->getParent()) {
    // In such case no need to load & replace the origin value
    // as no barrier in the middle, return NULL to indecate that.
    return nullptr;
  }
  return PrevBB->getTerminator();
}

Value *KernelBarrier::getAddressInSpecialBuffer(unsigned int Offset,
                                                PointerType *PtrTy,
                                                Instruction *InsertBefore,
                                                const DebugLoc *DB) {
  Value *OffsetVal = ConstantInt::get(SizeTTy, APInt(SizeT, Offset));
  // If hit this assert then need to handle PHINode!
  assert(!isa<PHINode>(InsertBefore) &&
         "Cannot add instructions before a PHI node!");
  IRBuilder<> B(InsertBefore);
  if (DB)
    B.SetCurrentDebugLocation(*DB);
  // Calculate the pointer of the given offset for LocalId in the special buffer.
  Value *CurrSB = createGetCurrSBIndex(B);
  CurrSB = B.CreateNUWAdd(CurrSB, OffsetVal, "SB_LocalId_Offset");
  Value *Idxs[1] = {CurrSB};
  Value *AddrInSBinBytes = B.CreateInBoundsGEP(
      CurrentBarrierKeyValues->SpecialBufferValue, ArrayRef<Value *>(Idxs));
  // Bitcast pointer according to alloca type!
  Value *AddrInSpecialBuffer =
      B.CreatePointerCast(AddrInSBinBytes, PtrTy, "pSB_LocalId");
  return AddrInSpecialBuffer;
}

Instruction *KernelBarrier::createOOBCheckGetLocalId(CallInst *Call) {
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
    ConstantInt *MaxWorkDimI32 =
        ConstantInt::get(*Context, APInt(32U, uint64_t(MaxNumDims), false));
    Value *CheckIndex = B.CreateICmpULT(
        Call->getArgOperand(0), MaxWorkDimI32, "check.index.inbound");
    B.CreateCondBr(CheckIndex, GetWIProperties, SplitContinue);
  }

  // B.Build the get.wi.properties block
  // Now retrieve address of the DIM count.

  BranchInst::Create(SplitContinue, GetWIProperties);
  IRBuilder<> B(GetWIProperties->getTerminator());
  B.SetCurrentDebugLocation(Call->getDebugLoc());
  Value *LocalIds = nullptr;
  if (UseTLSGlobals) {
    LocalIds = LocalIds;
  } else {
    LocalIds = CurrentBarrierKeyValues->LocalIdValues;
  }
  Instruction *Result = createGetLocalId(LocalIds, Call->getArgOperand(0), B);

  // C.Create Phi node at the first of the splitted BB.
  PHINode *AttrResult = PHINode::Create(IntegerType::get(*Context, SizeT), 2,
                                        "", SplitContinue->getFirstNonPHI());
  AttrResult->addIncoming(Result, GetWIProperties);
  // The overflow value.
  AttrResult->addIncoming(ConstZero, Block);

  return AttrResult;
}

Value *KernelBarrier::resolveGetLocalIDCall(CallInst *Call) {
  Value *Dimension = Call->getOperand(0);
  if (ConstantInt *C = dyn_cast<ConstantInt>(Dimension)) {
    uint64_t Dim = C->getZExtValue();
    if (Dim >= MaxNumDims) {
      // OpenCL Spec says to return zero for OOB dim value.
      return ConstZero;
    }
    // assert(BarrierKeyValuesPerFunction[pFunc].NumDims > Dim);
    IRBuilder<> B(Call);
    return createGetLocalId(Dim, B);
  }
  // assert(BarrierKeyValuesPerFunction[pFunc].NumDims == MaxNumDims);
  return createOOBCheckGetLocalId(Call);
}

bool KernelBarrier::fixGetWIIdFunctions(Module &M) {
  // Clear container for new iteration on new function.
  InstructionsToRemove.clear();

  std::string Name;
  // Find all get_local_id instructions.
  InstVector &GetLIDInstructions = BarrierUtils.getAllGetLocalId();
  for (auto *I : GetLIDInstructions) {
    CallInst *OldCall = dyn_cast<CallInst>(I);
    assert(OldCall &&
           "Something other than CallInst is using get_local_id function!");
    Function *Func = OldCall->getFunction();
    if (!UseTLSGlobals)
      getBarrierKeyValues(Func);
    else
      CurrentFunction = Func;
    Value *LID = resolveGetLocalIDCall(OldCall);
    OldCall->replaceAllUsesWith(LID);
    InstructionsToRemove.push_back(OldCall);
  }

  // Remove all instructions in InstructionsToRemove.
  eraseAllToRemoveInstructions();

  return true;
}

void KernelBarrier::fixNonInlineFunction(Function *FuncToFix) {
  // TODO: do we need to set DebugLoc for these instructions?
  // Get key values for this functions.
  getBarrierKeyValues(FuncToFix);

  unsigned int NumOfArgs = FuncToFix->getFunctionType()->getNumParams();
  // Use offsets instead of original parameters.
  Function::arg_iterator ArgIter = FuncToFix->arg_begin();
  for (unsigned int i = 0; i < NumOfArgs; ++i, ++ArgIter) {
    Value *ArgVal = &*ArgIter;
    if (DPV->hasOffset(ArgVal)) {
      unsigned int Offset = DPV->getOffset(ArgVal);
      fixArgumentUsage(ArgVal, Offset);
    }
  }
  if (DPV->hasOffset(FuncToFix)) {
    unsigned int Offset = DPV->getOffset(FuncToFix);

    std::vector<BasicBlock *> VecBB;
    for (BasicBlock &BB : *FuncToFix) {
      VecBB.push_back(&BB);
    }
    // Run over all basic blocks of the new function and handle return
    // terminators.
    for (BasicBlock *BB : VecBB) {
      ReturnInst *RetInst = dyn_cast<ReturnInst>(BB->getTerminator());
      if (!RetInst) {
        // It is not return instruction terminator, check next basic block.
        continue;
      }
      Value *RetVal = RetInst->getOperand(0);
      Instruction *NextInst;
      if (Instruction *Inst = dyn_cast<Instruction>(RetVal)) {
        // Find next instruction so we can create new instruction before it.
        NextInst = &*(++BasicBlock::iterator(Inst));
        if (isa<PHINode>(NextInst)) {
          // NextInst is a PHINode, find first non PHINode to add instructions.
          // before it.
          NextInst = NextInst->getParent()->getFirstNonPHI();
        }
      } else {
        // In this case the return value is not an instruction and
        // it cannot be assumed that it is inside the barrier loop.
        // Thus, need to create a new barrier loop that store this value
        // in the special buffer, that is why we needed to find the values:
        // CurrSBIndex, m_pLocalIdValue, m_pWIIterationCountValue
        // Before:
        //  BB:
        //      ret pRetVal
        // After:
        //  BB:
        //      br loopBB
        //  loopBB:
        //      pSB[pCurrSBValue+offset] = pRetVal
        //      cond LocalId < IterCount
        //      LocalId++
        //      pCurrSBValue += Stride
        //      br cond, loopBB, RetBB
        //  RetBB:
        //      ret pRetVal
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

void KernelBarrier::fixArgumentUsage(Value *OriginalArg,
                                     unsigned int OffsetArg) {
  // TODO: do we need to set DebugLoc for these instructions?
  assert((!DPV->isOneBitElementType(OriginalArg) ||
          !isa<VectorType>(OriginalArg->getType())) &&
         "OriginalArg with base type i1!");
  InstSet UserInsts;
  for (auto *U : OriginalArg->users()) {
    Instruction *UserInst = dyn_cast<Instruction>(U);
    UserInsts.insert(UserInst);
  }
  for (Instruction *UserInst : UserInsts) {
    assert(UserInst &&
           "Something other than Instruction is using a function argument!");
    Instruction *InsertBefore = UserInst;
    if (isa<PHINode>(UserInst)) {
      BasicBlock *PrevBB = DPCPPKernelBarrierUtils::findBasicBlockOfUsageInst(
          OriginalArg, UserInst);
      InsertBefore = PrevBB->getTerminator();
    }
    // In this case we will always get a valid offset and need to load the
    // argument from the special buffer using the offset corresponding argument.
    PointerType *PtrTy =
        OriginalArg->getType()->getPointerTo(SPECIAL_BUFFER_ADDR_SPACE);
    Value *AddrInSpecialBuffer =
        getAddressInSpecialBuffer(OffsetArg, PtrTy, InsertBefore, nullptr);
    Value *LoadedValue =
        new LoadInst(OriginalArg->getType(), AddrInSpecialBuffer,
                     "loadedValue", InsertBefore);
    UserInst->replaceUsesOfWith(OriginalArg, LoadedValue);
  }
}

void KernelBarrier::fixReturnValue(Value *RetVal, unsigned int OffsetRet,
                                   Instruction *InsertBefore) {
  // TODO: do we need to set DebugLoc for these instructions?
  assert((!DPV->isOneBitElementType(RetVal) ||
          !isa<VectorType>(RetVal->getType())) &&
         "RetVal with base type i1!");
  // RetVal might be a result of calling other function itself
  // in such case no need to handle it here as it will be saved
  // to the special buffer by the called function itself.
  // Calculate the pointer of the current special in the special buffer.
  PointerType *PtrTy =
      RetVal->getType()->getPointerTo(SPECIAL_BUFFER_ADDR_SPACE);
  Value *AddrInSpecialBuffer =
      getAddressInSpecialBuffer(OffsetRet, PtrTy, InsertBefore, nullptr);
  // Add Store instruction after the value instruction.
  new StoreInst(RetVal, AddrInSpecialBuffer, InsertBefore);
}

void KernelBarrier::fixCallInstruction(CallInst *CallToFix) {
  Function *CalledFunc = CallToFix->getCalledFunction();
  assert(CalledFunc && "Call instruction has no called function");
  Function *Func = CallToFix->getParent()->getParent();

  // Get key values for this functions.
  getBarrierKeyValues(Func);

  const DebugLoc &DB = CallToFix->getDebugLoc();
  Instruction *InsertBefore = nullptr;
  Function::arg_iterator ArgIter = CalledFunc->arg_begin();
  for (CallInst::const_op_iterator opi = CallToFix->arg_begin(),
                                   ope = CallToFix->arg_end();
       opi != ope; ++opi, ++ArgIter) {
    if (!DPV->hasOffset(&*ArgIter))
      continue;

    if (!InsertBefore) {
      // Split sync instruction basic-block that contains the call instruction.
      BasicBlock *PreBB = CallToFix->getParent();
      BasicBlock::iterator FirstInst = PreBB->begin();
      assert(DPB->getSyncInstructions(Func).count(&*FirstInst) &&
             "Assume first instruction to be sync instruction");
      BasicBlock *CallBB = PreBB->splitBasicBlock(FirstInst, "CallBB");
      InsertBefore = PreBB->getTerminator();
      OldToNewSyncBBMap[Func][PreBB] = CallBB;
    }
    // Need to handle operand.
    Value *OpVal = *opi;
    unsigned int Offset = DPV->getOffset(&*ArgIter);

    // Calculate the pointer of the current special in the special buffer.
    PointerType *PtrTy =
        OpVal->getType()->getPointerTo(SPECIAL_BUFFER_ADDR_SPACE);
    Value *AddrInSpecialBuffer =
        getAddressInSpecialBuffer(Offset, PtrTy, InsertBefore, &DB);
    // Add Store instruction before the synchronize instruction (in the pre
    // basic block).
    StoreInst *SI =
        new StoreInst(OpVal, AddrInSpecialBuffer, InsertBefore);
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
  assert(DPB->getSyncInstructions(Func).count(&*FirstInst) &&
         "Assume first instruction to be sync instruction");
  // Find next instruction so we can create new instruction before it.
  Instruction *NextInst = &*(++FirstInst);

  unsigned int Offset = DPV->getOffset(CalledFunc);

  // Calculate the pointer of the current special in the special buffer.
  PointerType *PtrTy =
      CallToFix->getType()->getPointerTo(SPECIAL_BUFFER_ADDR_SPACE);
  Value *AddrInSpecialBuffer =
      getAddressInSpecialBuffer(Offset, PtrTy, NextInst, &DB);
  // Add Load instruction from special buffer at function offset.
  LoadInst *LoadedValue = new LoadInst(
      CallToFix->getType(), AddrInSpecialBuffer, "loadedValue", NextInst);
  LoadedValue->setDebugLoc(DB);

  if (DPV->hasOffset(CallToFix)) {
    // CallInst return value has an offset in the special buffer
    // Store the value to this offset.
    unsigned int OffsetRet = DPV->getOffset(CallToFix);

    // Calculate the pointer of the current special in the special buffer.
    Value *AddrInSpecialBuffer =
        getAddressInSpecialBuffer(OffsetRet, PtrTy, NextInst, &DB);
    // Add Store instruction to special buffer at return value offset.
    StoreInst *SI =
        new StoreInst(LoadedValue, AddrInSpecialBuffer, NextInst);
    SI->setDebugLoc(DB);
  } else {
    CallToFix->replaceAllUsesWith(LoadedValue);
  }
}

void KernelBarrier::eraseAllToRemoveInstructions() {
  // Remove all instructions in InstructionsToRemove.
  for (Instruction *Inst : InstructionsToRemove) {
    assert(Inst && "Remove instruction container contains non instruction!");
    Inst->eraseFromParent();
  }
}

unsigned KernelBarrier::computeNumDim(Function *F) {
  // TODO: port decusing of max Dim. Or make hint in the header.
  return 3;
}

// Use DFS to calculate a function's non-barrier memory usage.
static unsigned getPrivateSize(Function *Func, const CallGraph &CG,
                               DataPerValue *DataPerVal,
                               DenseMap<Function *, uint64_t> &AddrAllocaSize,
                               llvm::DenseMap<Function *, unsigned> &FnPrivSize,
                               FuncSet &FnsWithSync) {
  // External function or function pointer.
  if (!Func || Func->isDeclaration())
    return 0;
  if (FnPrivSize.count(Func))
    return FnPrivSize[Func];
  unsigned MaxSubPrivSize = 0;
  const CallGraphNode *NodeCG = CG[Func];
  for (auto &CI : *NodeCG) {
    Function *CalledFunc = CI.second->getFunction();
    MaxSubPrivSize =
        std::max(MaxSubPrivSize,
                 getPrivateSize(CalledFunc, CG, DataPerVal, AddrAllocaSize,
                                FnPrivSize, FnsWithSync));
  }
  FnPrivSize[Func] =
      MaxSubPrivSize +
      (AddrAllocaSize.count(Func) ? AddrAllocaSize[Func] : 0) +
      (FnsWithSync.count(Func) ? 0 : DataPerVal->getStrideSize(Func));

  return FnPrivSize[Func];
}

void KernelBarrier::updateStructureStride(Module &M,
                                          FuncSet &FunctionsWithSync) {
  // Collect functions to process.
  CallGraph CG{M};
  FuncVector KernelList;

  for (auto &F : M) {
    if (F.hasFnAttribute("sycl_kernel"))
      KernelList.push_back(&F);
  }
  llvm::DenseMap<Function *, unsigned> FuncToPrivSize;
  auto TodoList =
      DPCPPKernelBarrierUtils::getAllKernelsAndVectorizedCounterparts(
          KernelList, &M);

  // Get the kernels using the barrier for work group loops.
  for (auto Func : TodoList) {
    // Need to check if Vectorized Width Value exists, it is not guaranteed
    // that Vectorized is running in all scenarios.
    int VecWidth = 1;
    if (Func->hasFnAttribute("vectorized_width")) {
      bool Res = to_integer(
          Func->getFnAttribute("vectorized_width").getValueAsString(),
          VecWidth);
      // Silence warning to avoid an extra call.
      (void)Res;
      assert(Res && "vectorized_width has to have a numeric value");
    }
    unsigned int StrideSize = DPV->getStrideSize(Func);
    assert(VecWidth && "VecWidth should not be 0!");
    StrideSize = (StrideSize + VecWidth - 1) / VecWidth;

    auto PrivateSize = getPrivateSize(Func, CG, DPV, AddrAllocaSize,
                                      FuncToPrivSize, FunctionsWithSync);
    // Need to check if NoBarrierPath Value exists, it is not guaranteed that
    // KernelAnalysisPass is running in all scenarios.
    // CSSD100016517, CSSD100018743: workaround
    // Private memory is always considered to be non-uniform. I.e. it is not
    // shared by each WI per vector lane. If it is uniform (i.e. its content
    // doesn't depend on non-uniform values) the private memory query returns a
    // smaller value than actual private memory usage. This subtle is taken into
    // account in the query for the maximum work-group.
    bool NoBarrierPath = false;
    if (Func->hasFnAttribute(NO_BARRIER_PATH_ATTRNAME)) {
      StringRef Value =
          Func->getFnAttribute(NO_BARRIER_PATH_ATTRNAME).getValueAsString();
      assert((Value == "true" || Value == "false") &&
             "Barrier: unexpected " NO_BARRIER_PATH_ATTRNAME " value!");
      NoBarrierPath = Value == "true" ? true : false;
    }
    if (NoBarrierPath) {
      Func->addFnAttr("dpcpp-kernel-barrier-buffer-size", utostr(0));
      // if there are no barrier in the kernel, strideSize is the kernel
      // body's private memory usage. So need to add sub-function's memory size.
      Func->addFnAttr(
          "dpcpp-kernel-private-memory-size",
          utostr(StrideSize + PrivateSize - DPV->getStrideSize(Func)));
    } else {
      Func->addFnAttr("dpcpp-kernel-barrier-buffer-size", utostr(StrideSize));
      // if there are some barriers in the kernel, stiderSize is barrier
      // buffer size. So need to add non barrier private memory.
      Func->addFnAttr("dpcpp-kernel-private-memory-size",
                       utostr(StrideSize + PrivateSize));
    }
  }
}

ModulePass *createKernelBarrierPass(bool IsNativeDebug, bool UseTLSGlobals) {
  return new llvm::KernelBarrier(IsNativeDebug, UseTLSGlobals);
}

void getBarrierPassStrideSize(
    Pass *PassPtr, std::map<std::string, unsigned int> &BufferStrideMap) {
  ((llvm::KernelBarrier *)PassPtr)->getStrideMap(BufferStrideMap);
}

} // namespace llvm
