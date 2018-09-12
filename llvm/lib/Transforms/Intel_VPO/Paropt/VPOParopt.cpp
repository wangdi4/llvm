#if INTEL_COLLAB
//===-------- VPOParopt.cpp - Paropt Pass for Auto-Par and OpenMP ---------===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation. and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
// Authors:
// --------
// Xinmin Tian (xinmin.tian@intel.com)
//
// Major Revisions:
// ----------------
// Nov 2015: Initial Implementation of Paropt Pass (Xinmin Tian)
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements the ParOpt pass interface to perform transformation
/// for OpenMP and Auto-parallelization
///
//===----------------------------------------------------------------------===//

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/InitializePasses.h"

#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionInfo.h"

#include "llvm/Transforms/Intel_VPO/Paropt/VPOParopt.h"
#include "llvm/Transforms/Intel_VPO/Paropt/VPOParoptTpv.h"
#include "llvm/Transforms/Intel_VPO/Paropt/VPOParoptTransform.h"
#include "llvm/Transforms/Intel_VPO/Utils/VPOUtils.h"
#include "llvm/Transforms/Intel_VPO/VPOPasses.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/Utils/Local.h"
#define DEBUG_TYPE "VPOParopt"


using namespace llvm;
using namespace llvm::vpo;

static cl::opt<bool> SwitchToOffload(
    "switch-to-offload", cl::Hidden, cl::init(false),
    cl::desc("switch to offload mode (default = false)"));

INITIALIZE_PASS_BEGIN(VPOParopt, "vpo-paropt", "VPO Paropt Module Pass", false,
                      false)
INITIALIZE_PASS_DEPENDENCY(LoopSimplify)
INITIALIZE_PASS_DEPENDENCY(WRegionInfoWrapperPass)
INITIALIZE_PASS_END(VPOParopt, "vpo-paropt", "VPO Paropt Module Pass", false,
                    false)

char VPOParopt::ID = 0;

ModulePass *llvm::createVPOParoptPass(unsigned Mode, unsigned OptLevel) {
  return new VPOParopt(
      (ParTrans | OmpPar | OmpVec | OmpTpv | OmpOffload | OmpTbb) & Mode,
      OptLevel);
}

VPOParopt::VPOParopt(unsigned MyMode, unsigned OptLevel)
    : ModulePass(ID), Impl(MyMode, OptLevel) {
  initializeVPOParoptPass(*PassRegistry::getPassRegistry());
}

VPOParoptPass::VPOParoptPass(unsigned MyMode, unsigned OptLevel)
    : Mode(MyMode), OptLevel(OptLevel) {
  LLVM_DEBUG(dbgs() << "\n\n====== Start VPO Paropt Pass ======\n\n");
}

void VPOParopt::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequiredID(LoopSimplifyID);
  AU.addRequired<WRegionInfoWrapperPass>();
}

bool VPOParopt::runOnModule(Module &M) {
  if (skipModule(M))
    return false;

  auto WRegionInfoGetter = [&](Function &F) -> WRegionInfo & {
    return getAnalysis<WRegionInfoWrapperPass>(F).getWRegionInfo();
  };

  return Impl.runImpl(M, WRegionInfoGetter);
}

PreservedAnalyses VPOParoptPass::run(Module &M, ModuleAnalysisManager &AM) {
  auto WRegionInfoGetter = [&](Function &F) -> WRegionInfo & {
    auto &FAM =
        AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
    return FAM.getResult<WRegionInfoAnalysis>(F);
  };

  if (!runImpl(M, WRegionInfoGetter))
    return PreservedAnalyses::all();

  return PreservedAnalyses::none();
}

bool VPOParoptPass::runImpl(
    Module &M,
    std::function<vpo::WRegionInfo &(Function &F)> WRegionInfoGetter) {

  bool Changed = false;

  /// \brief As new functions to be added, so we need to prepare the
  /// list of functions we want to work on in advance.
  std::vector<Function *> FnList;

  for (auto F = M.begin(), E = M.end(); F != E; ++F) {
    // TODO: need Front-End to set F->hasOpenMPDirective()
    if (F->isDeclaration()) // if(!F->hasOpenMPDirective()))
      continue;
    LLVM_DEBUG(dbgs() << "\n=== VPOParoptPass func: " << F->getName()
                      << " {\n");
    FnList.push_back(&*F);
  }

  // Iterate over all functions which OpenMP directives to perform Paropt
  // transformation and generate MT-code
  for (auto F : FnList) {

    LLVM_DEBUG(dbgs() << "\n=== VPOParoptPass Process func: " << F->getName()
                      << " {\n");

    // Walk the W-Region Graph top-down, and create W-Region List
    WRegionInfo &WI = WRegionInfoGetter(*F);
    WI.buildWRGraph(WRegionCollection::LLVMIR);

    if (WI.WRGraphIsEmpty()) {
      LLVM_DEBUG(dbgs() << "\nNo WRegion Candidates for Parallelization \n");
    }

    LLVM_DEBUG(WI.print(dbgs()));

    //
    // Set up a function pass manager so that we can run some cleanup
    // transforms on the LLVM IR after code gen.
    //
    // legacy::FunctionPassManager FPM(&M);

    LLVM_DEBUG(errs() << "VPOParoptPass: ");
    LLVM_DEBUG(errs().write_escaped(F->getName()) << '\n');

    LLVM_DEBUG(dbgs() << "\n=== VPOParoptPass before ParoptTransformer{\n");

    // AUTOPAR | OPENMP | SIMD | OFFLOAD
    VPOParoptTransform VP(F, &WI, WI.getDomTree(), WI.getLoopInfo(), WI.getSE(),
                          WI.getTargetTransformInfo(), WI.getAssumptionCache(),
                          WI.getTargetLibraryInfo(), WI.getAliasAnalysis(),
                          Mode, OptLevel, SwitchToOffload);
    Changed = Changed | VP.paroptTransforms();

    LLVM_DEBUG(dbgs() << "\n}=== VPOParoptPass after ParoptTransformer\n");

    // Remove calls to directive intrinsics since the LLVM back end does not
    // know how to translate them.
    // VPOUtils::stripDirectives(*F);

    // It is possible that stripDirectives eliminates all instructions in a
    // basic block except for the branch instruction. Use CFG simplify to
    // eliminate them.
    // FPM.add(createCFGSimplificationPass());
    // FPM.run(*F);

    LLVM_DEBUG(dbgs() << "\n}=== VPOParopt end func: " << F->getName() << "\n");
  }

  if ((Mode & OmpPar) && (Mode & ParTrans))
    fixTidAndBidGlobals(M);

  genCtorList(M);
  if (((Mode & OmpOffload) || SwitchToOffload) && (Mode & ParTrans)) {
    removeTargetUndeclaredGlobals(M);
    if (VPOAnalysisUtils::isTargetSPIRV(&M)) {
      // Add the metadata to indicate that the module is OpenCL C version.
      if (!M.getNamedMetadata("spirv.Source")) {
        LLVMContext &C = M.getContext();
        SmallVector<Metadata *, 8> opSource = {
            ConstantAsMetadata::get(ConstantInt::get(Type::getInt32Ty(C), 3)),
            ConstantAsMetadata::get(
                ConstantInt::get(Type::getInt32Ty(C), 200000))};
        MDNode *srcMD = MDNode::get(C, opSource);
        M.getOrInsertNamedMetadata("spirv.Source")->addOperand(srcMD);
      }
    }
  }

  // Thread private legacy mode implementation
  if (Mode & OmpTpv) {
    VPOParoptTpvLegacyPass VPTL;
    ModuleAnalysisManager DummyMAM;
    PreservedAnalyses PA = VPTL.run(M, DummyMAM);
    Changed = Changed | !PA.areAllPreserved();
  }

  LLVM_DEBUG(dbgs() << "\n====== End VPO ParoptPass ======\n\n");
  return Changed;
}

// Collect the uses of the given global variable.
void VPOParoptPass::collectUsesOfGlobals(
    Constant *PtrHolder, SmallVectorImpl<Instruction *> &RewriteIns) {
  for (auto IB = PtrHolder->user_begin(), IE = PtrHolder->user_end(); IB != IE;
       IB++) {
    if (Instruction *User = dyn_cast<Instruction>(*IB))
      RewriteIns.push_back(User);
  }
}

// Transform the use of the tid global into __kmpc_global_thread_num or the
// the use of the first argument of the OMP outlined function. The use of
// bid global is transformed accordingly.
void VPOParoptPass::fixTidAndBidGlobals(Module &M) {
  LLVMContext &C = M.getContext();
  Constant *TidPtrHolder =
      M.getOrInsertGlobal("@tid.addr", Type::getInt32Ty(C));
  SmallVector<Instruction *, 8> RewriteIns;

  collectUsesOfGlobals(TidPtrHolder, RewriteIns);
  processUsesOfGlobals(TidPtrHolder, RewriteIns, true);

  RewriteIns.clear();
  Constant *BidPtrHolder =
      M.getOrInsertGlobal("@bid.addr", Type::getInt32Ty(C));
  collectUsesOfGlobals(BidPtrHolder, RewriteIns);
  processUsesOfGlobals(BidPtrHolder, RewriteIns, false);
}

// The utility to transform the tid/bid global variable.
void VPOParoptPass::processUsesOfGlobals(
    Constant *PtrHolder, SmallVectorImpl<Instruction *> &RewriteIns,
    bool IsTid) {

  while (!RewriteIns.empty()) {
    Instruction *User = RewriteIns.pop_back_val();

    Function *F = User->getParent()->getParent();
    if (F->getAttributes().hasAttribute(AttributeList::FunctionIndex,
                                        "mt-func")) {
      auto IT = F->arg_begin();
      if (!IsTid)
        IT++;
      User->replaceUsesOfWith(PtrHolder, &*IT);
    } else if (IsTid && F->getAttributes().hasAttribute(
                            AttributeList::FunctionIndex, "task-mt-func")) {
      BasicBlock *EntryBB = &F->getEntryBlock();
      IRBuilder<> Builder(EntryBB->getFirstNonPHI());
      AllocaInst *TidPtr =
          Builder.CreateAlloca(Type::getInt32Ty(F->getContext()));
      Builder.CreateStore(&*(F->arg_begin()), TidPtr);
      User->replaceUsesOfWith(PtrHolder, TidPtr);
    } else {
      BasicBlock *EntryBB = &F->getEntryBlock();
      Instruction *Tid = nullptr;
      AllocaInst *TidPtr = nullptr;
      if (IsTid)
        Tid = VPOParoptUtils::findKmpcGlobalThreadNumCall(EntryBB);
      if (!Tid) {
        IRBuilder<> Builder(EntryBB->getFirstNonPHI());
        TidPtr = Builder.CreateAlloca(Type::getInt32Ty(F->getContext()));
        if (IsTid) {
          Tid = VPOParoptUtils::genKmpcGlobalThreadNumCall(F, TidPtr, nullptr);
          Tid->insertBefore(EntryBB->getFirstNonPHI());
        }
        StoreInst *SI = nullptr;
        if (IsTid)
          SI = new StoreInst(Tid, TidPtr);
        else
          SI = new StoreInst(
              ConstantInt::get(Type::getInt32Ty(F->getContext()), 0), TidPtr);
        SI->insertAfter(TidPtr);
      } else {
        for (auto IB = Tid->user_begin(), IE = Tid->user_end(); IB != IE;
             IB++) {
          auto User = dyn_cast<Instruction>(*IB);
          if (User && User->getParent() == Tid->getParent()) {
            StoreInst *SI = dyn_cast<StoreInst>(User);
            if (SI) {
              Value *V = SI->getPointerOperand();
              TidPtr = dyn_cast<AllocaInst>(V);
              break;
            }
          }
        }
      }

      if (TidPtr == nullptr) {
        IRBuilder<> Builder(EntryBB->getFirstNonPHI());
        TidPtr = Builder.CreateAlloca(Type::getInt32Ty(F->getContext()));
        StoreInst *SI = new StoreInst(Tid, TidPtr);
        SI->insertAfter(Tid);
      }
      User->replaceUsesOfWith(PtrHolder, TidPtr);
    }
  }
}

// \brief Remove routines and global variables which has no target declare
// attribute.
void VPOParoptPass::removeTargetUndeclaredGlobals(Module &M) {
  std::vector<GlobalVariable *> DeadGlobalVars; // Keep track of dead globals
  for (GlobalVariable &GV : M.globals())
    if (!GV.isTargetDeclare()) {
      DeadGlobalVars.push_back(&GV); // Keep track of dead globals
      // TODO  The check of use_empty will be removed after the frontend
      // generates target_declare attribute for the variable GV.
      if (GV.use_empty() && GV.hasInitializer()) {
        Constant *Init = GV.getInitializer();
        GV.setInitializer(nullptr);
        if (!isa<GlobalValue>(Init) && !isa<ConstantData>(Init))
          Init->destroyConstant();
      }
    }

  std::vector<Function *> DeadFunctions;

  for (Function &F : M) {
    if (!F.getAttributes().hasAttribute(AttributeList::FunctionIndex,
                                        "target.declare")) {
      DeadFunctions.push_back(&F);
      if (!F.isDeclaration())
        F.deleteBody();
    } else {
#if INTEL_CUSTOMIZATION
      // This is a workaround for resolving the incompatibility issue
      // between the xmain compiler and IGC compiler. The IGC compiler
      // cannot accept the following instruction, which is generated
      // at compile time for helping infering the address space.
      //   %4 = bitcast [10000 x i32] addrspace(1)* %B to i8*
      // The instruction becomes dead after the inferring is done.
#endif // INTEL_CUSTOMIZATION
      for (BasicBlock &BB : F)
        for (BasicBlock::iterator I = BB.begin(), E = BB.end(); I != E;) {
          Instruction *Inst = &*I++;
          if (isInstructionTriviallyDead(Inst))
            BB.getInstList().erase(Inst);
        }
    }
  }
  auto EraseUnusedGlobalValue = [&](GlobalValue *GV) {
    // TODO  The check of use_empty will be removed after the frontend
    // generates target_declare attribute for the variable GV.
    if (!GV->use_empty())
      return;
    GV->removeDeadConstantUsers();
    GV->eraseFromParent();
  };

  for (GlobalVariable *GV : DeadGlobalVars)
    EraseUnusedGlobalValue(GV);

  for (Function *F : DeadFunctions)
    EraseUnusedGlobalValue(F);
}

// Creates the global llvm.global_ctors initialized with
// with the function .omp_offloading.descriptor_reg
void VPOParoptPass::genCtorList(Module &M) {
  LLVMContext &C = M.getContext();
  Type *VoidPtrTy = Type::getInt8PtrTy(C);
  Type *Int32Ty = Type::getInt32Ty(C);
  Type *VoidTy = Type::getVoidTy(C);

  FunctionType *CtorFTy = FunctionType::get(VoidTy, false);
  Type *CtorPFTy = PointerType::getUnqual(CtorFTy);

  StructType *CtorStructTy = StructType::get(
      C, { Int32Ty, llvm::PointerType::getUnqual(CtorFTy), VoidPtrTy });

  SmallVector<Constant *, 16> CtorArrayInitBuffer;

  int CtorCnt = 0;
  for (auto F = M.begin(), E = M.end(); F != E; ++F) {
    if (F->getAttributes().hasAttribute(AttributeList::FunctionIndex,
                                        "offload.ctor"))
      CtorCnt++;
  }
  if (CtorCnt == 0)
    return;

  for (auto F = M.begin(), E = M.end(); F != E; ++F) {
    if (!F->getAttributes().hasAttribute(AttributeList::FunctionIndex,
                                         "offload.ctor"))
      continue;
    F->removeFnAttr("offload.ctor");
    SmallVector<Constant *, 16> CtorInitBuffer;
    CtorInitBuffer.push_back(ConstantInt::getSigned(Int32Ty, 0));
    Constant *Initializer = &*F;
    CtorInitBuffer.push_back(ConstantExpr::getBitCast(Initializer, CtorPFTy));
    CtorInitBuffer.push_back(ConstantPointerNull::get(Type::getInt8PtrTy(C)));

    Constant *CtorInit = ConstantStruct::get(CtorStructTy, CtorInitBuffer);

    CtorArrayInitBuffer.push_back(CtorInit);
  }
  Constant *CtorArrayInit = ConstantArray::get(
      ArrayType::get(CtorStructTy, CtorCnt), CtorArrayInitBuffer);

  new GlobalVariable(M, CtorArrayInit->getType(), false,
                     GlobalValue::AppendingLinkage, CtorArrayInit,
                     "llvm.global_ctors");
}
#endif // INTEL_COLLAB
