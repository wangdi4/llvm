//=----------- SGLoopConstruct.cpp - Create subgroup loop - C++ -*-----------=//
//
// Copyright (C) 2020-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/SubgroupEmulation/SGLoopConstruct.h"

#include "llvm/IR/IRBuilder.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"

#define DEBUG_TYPE "dpcpp-kernel-sg-emu-loop-construct"

using namespace llvm;
using namespace DPCPPKernelMetadataAPI;
using namespace DPCPPKernelCompilationUtils;

extern bool DPCPPEnableSubGroupEmulation;

namespace {
/// Legacy SGLoopConstruct pass.
class SGLoopConstructLegacy : public ModulePass {
public:
  static char ID;

  SGLoopConstructLegacy();

  StringRef getPassName() const override { return "SGLoopConstructLegacy"; }

  bool runOnModule(Module &M) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<SGSizeAnalysisLegacy>();
    AU.addPreserved<SGSizeAnalysisLegacy>();
  }

private:
  SGLoopConstructPass Impl;
};
} // namespace

char SGLoopConstructLegacy::ID = 0;

INITIALIZE_PASS_BEGIN(SGLoopConstructLegacy, DEBUG_TYPE, "Create subgroup loop",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(SGSizeAnalysisLegacy)
INITIALIZE_PASS_END(SGLoopConstructLegacy, DEBUG_TYPE, "Create subgroup loop",
                    false, false)

SGLoopConstructLegacy::SGLoopConstructLegacy() : ModulePass(ID) {
  initializeSGLoopConstructLegacyPass(*PassRegistry::getPassRegistry());
}

bool SGLoopConstructLegacy::runOnModule(Module &M) {
  const SGSizeInfo *SSI = &getAnalysis<SGSizeAnalysisLegacy>().getResult();
  return Impl.runImpl(M, SSI);
}

ModulePass *llvm::createSGLoopConstructLegacyPass() {
  return new SGLoopConstructLegacy();
}

PreservedAnalyses SGLoopConstructPass::run(Module &M,
                                           ModuleAnalysisManager &AM) {
  const SGSizeInfo *SSI = &AM.getResult<SGSizeAnalysisPass>(M);
  if (!runImpl(M, SSI))
    return PreservedAnalyses::all();
  PreservedAnalyses PA;
  PA.preserve<SGSizeAnalysisPass>();
  return PA;
}

bool SGLoopConstructPass::runImpl(Module &M, const SGSizeInfo *SSI) {
  if (!DPCPPEnableSubGroupEmulation)
    return false;

  Helper.initialize(M);
  FunctionsNeedEmulation = Helper.getAllFunctionsNeedEmulation();
  if (FunctionsNeedEmulation.empty())
    return false;

  SizeInfo = SSI;

  collectSyncInsts();
  createSGLoop();
  updateTIDCalls(M);
  hoistSGLIdCalls(M);
  resolveSGLIdCalls(M);
  updateMetadata(M);

  return !FuncToSGBarriers.empty();
}

void SGLoopConstructPass::collectSyncInsts() {
  for (Function *F : FunctionsNeedEmulation) {
    // Assign an unique ID for every sync instruction, it's used to identify
    // where the sub-group loop comes from when there are more than one jump
    // targets for sub-group barrier.
    unsigned UniqueID = 0;
    LLVM_DEBUG(dbgs() << "Collecting sync insts for function: " << F->getName()
                      << " (" << F << ")\n");
    assert(SizeInfo->hasEmuSize(F) && "Function doesn't have emulation size");

    InstSet Barriers = Helper.getBarriersForFunction(F);
    FuncToSGBarriers[F] = Barriers;
    for (auto *BarrierCall : Barriers) {
      auto *BarrierBB = BarrierCall->getParent();
      auto *LoopHeader =
          BarrierBB->splitBasicBlock(BarrierCall, "sg.barrier.split.");
      LoopHeaderToPrevExiting[LoopHeader] = BarrierBB;
      SyncInstToUniqueID[BarrierCall] = UniqueID++;
    }

    InstSet DummyBarriers = Helper.getDummyBarriersForFunction(F);
    for (auto *DummyBarrierCall : DummyBarriers) {
      auto *DummyBarrierBB = DummyBarrierCall->getParent();
      auto *LoopHeader =
          DummyBarrierBB->splitBasicBlock(DummyBarrierCall, "sg.dummy.split.");
      LoopHeaderToPrevExiting[LoopHeader] = DummyBarrierBB;
      SyncInstToUniqueID[DummyBarrierCall] = UniqueID++;
    }
  }
}

InstSet SGLoopConstructPass::findSyncPredecessors(Instruction *I) {
  InstSet SyncPredInsts;

  BBVec WorkList;
  BBSet BBsHandled;

  WorkList.push_back(I->getParent());
  while (!WorkList.empty()) {
    auto *BB = WorkList.pop_back_val();
    for (auto It = pred_begin(BB), End = pred_end(BB); It != End; ++It) {
      auto *PredBB = *It;
      if (!BBsHandled.insert(PredBB))
        continue;
      auto *FirstI = &*(PredBB->begin());
      if (Helper.isSyncCall(FirstI))
        SyncPredInsts.insert(FirstI);
      else
        WorkList.push_back(PredBB);
    }
  }
  return SyncPredInsts;
}

void SGLoopConstructPass::createSGLoop() {
  for (auto It = FuncToSGBarriers.begin(), End = FuncToSGBarriers.end();
       It != End; ++It) {
    Function *F = It->first;
    InstSet &BarrierInsts = It->second;

    // Here we must make sure the FirstInst is the terminator of
    // sg.loop.exclude. If not, we will insert alloca in non-entry block which
    // will cause dynamic allocation under O0 and then cause unaligned stack
    // access. For the first loop header, if there is no barrier in the kernel,
    // we can skip twice to get the sg.loop.exclude BB. If there is barrier in
    // the kernel, it's safe to alloca in non-entry blocks, since they will all
    // be placed into special buffer.
    Instruction *FirstInst = Helper.getFirstDummyBarrier(F)
                                 ->getParent()
                                 ->getSinglePredecessor()
                                 ->getSinglePredecessor()
                                 ->getTerminator();

    IRBuilder<> Builder(FirstInst);

    auto *Int32Type = Builder.getInt32Ty();
    auto *SGLIdPtr = Builder.CreateAlloca(Int32Type, nullptr, "sg.lid.ptr");
    auto *SGLoopSrcPtr =
        Builder.CreateAlloca(Int32Type, nullptr, "sg.loop.src.ptr");
    // Can't insert get_sub_group_size here, because we need to make sure the
    // call is inside barrier / WG loop.
    FuncToLoopStuff[F] = {SGLIdPtr, SGLoopSrcPtr};

    for (auto *I : BarrierInsts)
      BarrierToJumpTargets[I] = findSyncPredecessors(I);
  }

  for (auto &SyncItem : SyncInstToUniqueID) {
    auto *SyncInst = SyncItem.first;
    auto UniqueID = SyncItem.second;

    auto *F = SyncInst->getFunction();
    auto &Context = F->getContext();

    AllocaInst *SGLIdPtr = nullptr, *SGLoopSrcPtr = nullptr;
    std::tie(SGLIdPtr, SGLoopSrcPtr) = FuncToLoopStuff[F];

    auto *CurrentBB = SyncInst->getParent();
    auto *PrevExitingBB = LoopHeaderToPrevExiting[CurrentBB];

    if (Helper.isDummyBarrier(SyncInst)) {
      // dummy_sg_barrier instructions are used to mark the jump targets
      // for sub_group_barriers. We won't construct loop from them, so we
      // should only update the loop stuff here.
      IRBuilder<> Builder(PrevExitingBB->getTerminator());
      Builder.CreateStore(Builder.getInt32(0), SGLIdPtr);
      Builder.CreateStore(Builder.getInt32(UniqueID), SGLoopSrcPtr);
    }

    if (Helper.isBarrier(SyncInst)) {
      // Make exiting.
      // Increment sub-group local id and compare it with sub-group size, if
      // sub_group_local_id is less then sub-group size then goto latch block
      // else goto exit block.
      PrevExitingBB->setName("sg.loop.exiting.");
      IRBuilder<> Builder(PrevExitingBB->getTerminator());
      auto *ConstOne = Builder.getInt32(1);
      auto *SGLId =
          Builder.CreateLoad(Builder.getInt32Ty(), SGLIdPtr, "sg.lid.");
      auto *SGLIdInc = Builder.CreateNUWAdd(SGLId, ConstOne, "sg.lid.inc.");
      Builder.CreateStore(SGLIdInc, SGLIdPtr);

      auto *LoopLatch =
          BasicBlock::Create(Context, "sg.loop.latch.", F, CurrentBB);
      auto *LoopExit =
          BasicBlock::Create(Context, "sg.loop.exit.", F, CurrentBB);

      auto *SGLoopSize = Helper.createGetSubGroupSize((Instruction *)SGLId);
      auto *LoopContinue = Builder.CreateICmpULT(SGLIdInc, SGLoopSize);
      Builder.CreateCondBr(LoopContinue, LoopLatch, LoopExit);

      // Dump sub-group loop control variable for debugging.
      LLVM_DEBUG(insertPrintf(PrevExitingBB->getName(), (Instruction *)SGLIdInc,
                              SGLId));

      // Make latch.
      // Add unconditional branch to the single target OR add switch to
      // multiple targets.
      Builder.SetInsertPoint(LoopLatch);
      Builder.SetCurrentDebugLocation(SyncInst->getDebugLoc());
      auto &JumpTargets = BarrierToJumpTargets[SyncInst];
      auto *FirstTarget = (*JumpTargets.begin())->getParent();
      FirstTarget->setName("sg.loop.header.");
      if (JumpTargets.size() == 1) {
        Builder.CreateBr(FirstTarget);
      } else {
        auto *SGLoopSrc = Builder.CreateLoad(Builder.getInt32Ty(), SGLoopSrcPtr,
                                             "sg.loop.src.");
        auto *Switch = Builder.CreateSwitch(SGLoopSrc, FirstTarget,
                                            JumpTargets.size() - 1);
        for (auto It = ++JumpTargets.begin(), End = JumpTargets.end();
             It != End; ++It) {
          BasicBlock *JumpTarget = (*It)->getParent();
          JumpTarget->setName("sg.loop.header.");
          Switch->addCase(Builder.getInt32(SyncInstToUniqueID[*It]),
                          JumpTarget);
        }
      }

      // Remove the terminator here so that its debug info will attach to the
      // other instructions emitted above.
      PrevExitingBB->getTerminator()->eraseFromParent();

      // Make exit block.
      // 1) Set the sub-group loop control variable to 0.
      // 2) Set the loop source to ID of current sync instruction.
      Builder.SetInsertPoint(LoopExit);
      Builder.CreateStore(Builder.getInt32(0), SGLIdPtr);
      Builder.CreateStore(Builder.getInt32(UniqueID), SGLoopSrcPtr);
      Builder.CreateBr(CurrentBB);
    }
  }

  // Remove all sync instructions.
  for (auto &SyncItem : SyncInstToUniqueID)
    SyncItem.first->eraseFromParent();
}

void SGLoopConstructPass::updateTIDCalls(Module &M) {
  // Collect TID calls
  static const std::string TIDFuncNames[] = {mangledGetGID(), mangledGetLID()};
  InstVec TIDCalls;

  for (auto &TIDFuncName : TIDFuncNames) {
    auto *Fn = M.getFunction(TIDFuncName);
    if (!Fn)
      continue;
    for (auto *U : Fn->users()) {
      auto *CI = cast<CallInst>(U);
      auto *PF = CI->getFunction();
      if (!SizeInfo->hasEmuSize(PF))
        continue;
      // Variable TID Calls have been resolved in ResolveVariableTIDCall pass.
      auto *C = cast<ConstantInt>(CI->getArgOperand(0));
      if (C->getZExtValue() != 0)
        continue;
      LLVM_DEBUG(dbgs() << "Collect TID Call:" << *CI);
      LLVM_DEBUG(dbgs() << " in Function: " << PF->getName() << "\n");
      TIDCalls.push_back(CI);
    }
  }

  for (auto *Id : TIDCalls) {
    auto *SGLId = Helper.createGetSubGroupLId(Id);
    IRBuilder<> Builder(Id->getNextNode());
    Builder.SetCurrentDebugLocation(Id->getDebugLoc());
    auto *SGLIdExt = Builder.CreateZExtOrBitCast(SGLId, Id->getType());
    auto *NewId = Builder.CreateBinOp(Instruction::Add, SGLIdExt, Id);
    Id->replaceUsesWithIf(NewId,
                          [NewId](Use &U) { return U.getUser() != NewId; });
  }
}

void SGLoopConstructPass::hoistSGLIdCalls(Module &M) {
  Function *SGLIdFunc = M.getFunction(mangledGetSubGroupLocalId());
  if (!SGLIdFunc)
    return;

  InstSet SGLIdCallsToBeUpdated;
  FuncSet FuncsToBePatched;
  FuncVec WorkList;

  // Collect all get_sub_group_local_id calls to be hoisted and initial
  // functions to be patched.
  for (User *U : SGLIdFunc->users()) {
    auto *CI = cast<CallInst>(U);
    auto *PF = CI->getFunction();
    // If the parent function is emulated, we can resolve SGLId there and no
    // need to patch it.
    if (FunctionsNeedEmulation.count(PF))
      continue;
    // This function may also be called by a non-emulated kernel.
    if (!SizeInfo->hasEmuSize(PF))
      continue;
    SGLIdCallsToBeUpdated.insert(CI);
    if (FuncsToBePatched.insert(PF))
      WorkList.push_back(PF);
  }

  // Collect all functions and calls to be patched.
  InstSet CallsToBePatched;
  while (!WorkList.empty()) {
    Function *F = WorkList.pop_back_val();
    for (User *U : F->users()) {
      auto *CI = dyn_cast<CallInst>(U);
      if (!CI)
        continue;
      CallsToBePatched.insert(CI);
      auto *PF = CI->getFunction();
      // If the parent function is emulated, we can resolve SGLId there and no
      // need to patch it.
      if (FunctionsNeedEmulation.count(PF))
        continue;
      // This function may also be called by a non-emulated kernel.
      if (!SizeInfo->hasEmuSize(PF))
        continue;
      if (FuncsToBePatched.insert(PF))
        WorkList.push_back(PF);
    }
  }

  Type *SGLIdType = IntegerType::get(M.getContext(), 32);

  // Patch functions.
  MapVector<Function *, Function *> FuncToPatchedFunc;
  FuncSet PatchedFuncs;
  for (auto *F : FuncsToBePatched) {
    LLVM_DEBUG(dbgs() << "Patching function: " << F->getName() << "\n");
    Function *PatchedFunc =
        AddMoreArgsToFunc(F, SGLIdType, "sg.lid", None, "SGLoopConstruct");
    FuncToPatchedFunc[F] = PatchedFunc;
    PatchedFuncs.insert(PatchedFunc);
  }

  // Replace SGLID call with argument.
  for (auto *I : SGLIdCallsToBeUpdated) {
    Function *Caller = I->getFunction();
    assert(PatchedFuncs.count(Caller) &&
           "Replacing get_sub_group_local_id in a function not patched");
    I->replaceAllUsesWith(&*(Caller->arg_end() - 1));
    I->eraseFromParent();
  }

  // Patch calls.
  for (auto *I : CallsToBePatched) {
    LLVM_DEBUG(dbgs() << "Patching call:" << *I << "\n");
    auto *CI = cast<CallInst>(I);
    Function *Caller = CI->getFunction();
    Function *Callee = CI->getCalledFunction();
    Value *Arg = nullptr;
    if (PatchedFuncs.count(Caller)) {
      Arg = &*(Caller->arg_end() - 1);
    } else {
      // If the parent function is emulated, we need to replace the arg with
      // get_sub_group_local_id; If the parent function is not emulated, this
      // indicates current kernel is a scalar kernel, we should replace the arg
      // with 0.
      if (FunctionsNeedEmulation.count(Caller))
        Arg = Helper.createGetSubGroupLId(CI);
      else
        Arg = Helper.getZero();
    }
    AddMoreArgsToCall(CI, Arg, FuncToPatchedFunc[Callee]);
  }
}

void SGLoopConstructPass::resolveSGLIdCalls(Module &M) {
  Function *SGLIdFunc = M.getFunction(mangledGetSubGroupLocalId());
  if (!SGLIdFunc)
    return;

  // The SGLId is only updated in exiting and exit block, and we won't read
  // from SGLIdPtr in these blocks, so for a single BasicBlock, we can read
  // once.
  MapVector<BasicBlock *, InstSet> BBToSGLIdCalls;
  for (User *U : SGLIdFunc->users()) {
    CallInst *CI = dyn_cast<CallInst>(U);
    if (!CI)
      continue;
    auto *PF = CI->getFunction();
    if (!FuncToLoopStuff.count(PF))
      continue;
    BBToSGLIdCalls[CI->getParent()].insert(CI);
  }
  for (auto &Item : BBToSGLIdCalls) {
    auto *PF = Item.first->getParent();
    AllocaInst *SGLIdPtr = std::get<0>(FuncToLoopStuff[PF]);

    Instruction *IP = Item.first->getFirstNonPHI();
    IRBuilder<> Builder(IP);
    auto *SGLId = Builder.CreateLoad(Builder.getInt32Ty(), SGLIdPtr, "sg.lid.");

    for (auto *I : Item.second) {
      I->replaceAllUsesWith(SGLId);
      I->eraseFromParent();
    }
  }
}

void SGLoopConstructPass::updateMetadata(Module &M) {
  using namespace DPCPPKernelMetadataAPI;
  auto Kernels = KernelList(M).getList();
  auto KernelRange = make_range(Kernels.begin(), Kernels.end());
  for (auto &Pair : FuncToSGBarriers) {
    Function *F = Pair.first;
    auto &EmuSizes = SizeInfo->getEmuSizes(F);
    if (find(KernelRange, F) != Kernels.end()) {
      auto KIMD = KernelInternalMetadataAPI(F);
      // Only support one size at this moment.
      KIMD.VectorizedWidth.set(*EmuSizes.begin());
      KIMD.VectorizationDimension.set(0);
      MDValueGlobalObjectStrategy::unset(F, "sg_emu_size");
    } else {
      // After function vectorization enabled, we can also use this attribute
      // to indicate vectorization width.
      F->addFnAttr("widened-size", std::to_string(*EmuSizes.begin()));
    }
  }
}
