//===----------------------------------------------------------------------===//
//
/// \file
//
//===----------------------------------------------------------------------===//
//===- CSAReplaceAllocaWithMalloc.cpp - ------------------------------*- C++ -*--===//
//
// Copyright (C) 2017-2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
// This pass is IR level pass that parses a function to identify stack allocations
// (alloca's) and replace them with calls to csa_malloc(..) and csa_free(..).
// example
// Input IR
// %c = alloca [100 x i32], align 16
// %0 = bitcast [100 x i32]* %c to i8*
// call void @llvm.lifetime.start.p0i8(i64 400, i8* nonnull %0) #3 <-- Site for csa_malloc call
// %7 = bitcast [100 x i32]* %c to i8*
// call void @llvm.lifetime.end.p0i8(i64 400, i8* nonnull %7) #3 <-- Site for csa_free call
// Output IR
// %call_c = tail call i8* @csa_malloc(i32 100)
// %c = bitcast i8* %call_c to [100 x i32]*
// tail call void @csa_free(i8* %call_c)
//===

#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/StackProtector.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <utility>
#include <set>

#include "CSAUtils.h"
using namespace llvm;

static cl::opt<bool>
  CoalesceCSAMallocs("csa-coalesce-mallocs", cl::Hidden,
                 cl::desc("CSA Specific: Coalesce all csa malloc calls"),
                 cl::init(true));

#define DEBUG_TYPE "csa-replace-alloca"
#define REMARK_NAME "csa-replace-alloca-remark"
#define PASS_DESC                                                              \
  "CSA: Identify and replace alloca's with calls to csa_malloc and csa_free"

namespace llvm {
void initializeCSAReplaceAllocaWithMallocPass(PassRegistry &);
Pass *createCSAReplaceAllocaWithMallocPass();
} // namespace llvm

namespace {
struct CSAReplaceAllocaWithMalloc : public ModulePass {
  static char ID;

  explicit CSAReplaceAllocaWithMalloc() : ModulePass(ID) {
    initializeCSAReplaceAllocaWithMallocPass(*PassRegistry::getPassRegistry());
  }
  StringRef getPassName() const override { return PASS_DESC; }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }

  bool runOnModule(Module &M) override;
private:
  std::set<Instruction *> ToDelete;
  void coalesceMallocs(Function &F, Function *CSAMalloc, Function *CSAFree, ReturnInst *RI);
  void addToDelete(Instruction *I) { ToDelete.insert(I); }
  void deleteInstructions(void) {
    for (auto I : ToDelete) {
      I->eraseFromParent();
    }
    ToDelete.clear();
  }
};
} // namespace

char CSAReplaceAllocaWithMalloc::ID = 0;
INITIALIZE_PASS_BEGIN(CSAReplaceAllocaWithMalloc, "csa-replace-alloca", PASS_DESC,
                      false, false)
INITIALIZE_PASS_END(CSAReplaceAllocaWithMalloc, "csa-replace-alloca", PASS_DESC,
                    false, false)

Pass *llvm::createCSAReplaceAllocaWithMallocPass() { return new CSAReplaceAllocaWithMalloc(); }

/**********************************************
INPUT
%tmp121 = call i8* @csa_malloc(i64 400)
%tmp122 = bitcast i8* %tmp121 to [100 x i32]*
%tmp123 = call i8* @csa_malloc(i64 400)
%tmp124 = bitcast i8* %tmp123 to [100 x i32]*
...
%4 = bitcast [100 x i32]* %tmp121 to i8*
call void @csa_free(i8* %4)
%5 = bitcast [100 x i32]* %tmp123 to i8*
call void @csa_free(i8* %5)
...
OUTPUT
%tmp121 = call i8* @csa_malloc(i64 800) <--- 400+400
%tmp122 = bitcast i8* %tmp121 to [100 x i32]*
...
%tmp123 = add i8* %tmp119, 400
%tmp124 = bitcast i8* %tmp123 to [100 x i32]*
...
%4 = bitcast [100 x i32]* %tmp121 to i8*
call void @csa_free(i8* %4)
<-- delete all other csa_free calls
**********************************************/
void CSAReplaceAllocaWithMalloc::coalesceMallocs(Function &F, Function *CSAMalloc, Function *CSAFree, ReturnInst *RI) {
  CallInst *FirstCSAMallocInst = 0;
  // Find total size allocated using csa_malloc
  uint64_t Size = 0;
  for (BasicBlock &BB : F)
    for (auto II = std::begin(BB), E = std::end(BB); II != E; ++II) {
      Instruction *I = &*II;
      if (CallInst *CI = dyn_cast<CallInst>(I)) {
	if (CI->isInlineAsm()) continue;
        if (CI->getCalledFunction() == CSAMalloc) {
          if (!FirstCSAMallocInst) FirstCSAMallocInst = CI;
          Size += dyn_cast<ConstantInt>(CI->getOperand(0))->getZExtValue();
        }
      }
    }
  LLVM_DEBUG(dbgs() << "Total size = " << Size << "\n");
  if (Size == 0 || !FirstCSAMallocInst) return;

  // Modify the first csa_malloc to allocate totalSize bytes
  LLVMContext &Context = F.getContext();
  Value *SizeV = llvm::ConstantInt::get(Context, llvm::APInt(64, Size, false));
  if (SizeV->getType() != CSAMalloc->arg_begin()->getType())
    SizeV = new BitCastInst(SizeV, CSAMalloc->arg_begin()->getType(), "tmp", &*FirstCSAMallocInst);
  FirstCSAMallocInst->setOperand(0, SizeV);

  // Replace other csa_alloc's with GEP with proper offsets
  Size = 0;
  for (BasicBlock &BB : F)
    for (auto II = std::begin(BB), E = std::end(BB); II != E;) {
      Instruction *I = &*II;
      ++II;
      if (CallInst *CI = dyn_cast<CallInst>(I)) {
        if (CI == FirstCSAMallocInst) continue;
	if (CI->isInlineAsm()) continue;
        if (CI->getCalledFunction() == CSAMalloc) {
          Size += dyn_cast<ConstantInt>(CI->getOperand(0))->getZExtValue();
          Value *Offset = llvm::ConstantInt::get(Context, llvm::APInt(64, Size, false));
          Value *NewGEP = IRBuilder<>{CI}.CreateGEP(cast<PointerType>(FirstCSAMallocInst->getType())->getElementType(), FirstCSAMallocInst, Offset, "tmp");
          CI->replaceAllUsesWith(NewGEP);
          addToDelete(CI);
        }
      }
    }
  // Delete all csa_free's except first one
  CallInst *FirstCSAFreeInst = 0;
  for (BasicBlock &BB : F)
    for (auto II = std::begin(BB), E = std::end(BB); II != E;) {
      Instruction *I = &*II;
      ++II;
      if (CallInst *CI = dyn_cast<CallInst>(I)) {
        if (CI->isInlineAsm()) continue;
        if (CI->getCalledFunction() == CSAFree) {
          if (!FirstCSAFreeInst)
            FirstCSAFreeInst = CI;
          else {
            BitCastInst *BCI = dyn_cast<BitCastInst>(CI->getOperand(0));
            addToDelete(CI);
            if (BCI && BCI->hasOneUse()) addToDelete(BCI);
          }
        }
      }
    }
  return;
}

// This assumes one return instruction per function
static ReturnInst *getReturnInst(Function &F) {
  ReturnInst *RI = 0;
  for (BasicBlock &BB : F)
    for (auto II = std::begin(BB), E = std::end(BB); II != E; ++II) {
      Instruction *I = &*II;
      if (dyn_cast<ReturnInst>(I)) {
        RI = dyn_cast<ReturnInst>(I);
        LLVM_DEBUG(errs() << "Return Inst found; RI = " << *RI << "\n");
        return RI;
      }
    }
  return 0;
}

// Check if the Module has alloca instructions
static bool isAllocaPresent(Module &M) {
  for (auto &F : M) {
    for (BasicBlock &BB : F) {
      for (auto II = std::begin(BB), E = std::end(BB); II != E; ++II) {
        Instruction *I = &*II;
        if (AllocaInst *AI = dyn_cast<AllocaInst>(I))
          if (AI->isStaticAlloca())
            return true;
      }
    }
  }
  return false;
}

static bool isLifeTimeInst(Instruction *I) {
  if (const auto Intrinsic = dyn_cast<IntrinsicInst>(I)) {
    const auto Id = Intrinsic->getIntrinsicID();
    return Id == Intrinsic::lifetime_start || Id == Intrinsic::lifetime_end;
  }
  return false;
}

bool CSAReplaceAllocaWithMalloc::runOnModule(Module &M) {
  const DataLayout &DL = M.getDataLayout();
  LLVMContext &Context = M.getContext();
  if (!csa_utils::isAlwaysDataFlowLinkageSet()) {
    LLVM_DEBUG(errs() << "Data flow linkage not set\n");
    return false;
  }
  bool IsAllocaPresent = isAllocaPresent(M);
  if (!IsAllocaPresent) {
    LLVM_DEBUG(errs() << "No stack allocations found. No need to run this pass\n");
    return false;
  }
  Function *CSAMalloc = M.getFunction("csa_malloc");
  if (!CSAMalloc || CSAMalloc->isDeclaration())
    report_fatal_error("CSAMalloc function definition not found!");
  Function *CSAFree = M.getFunction("csa_free");
  if (!CSAFree || CSAFree->isDeclaration())
    report_fatal_error("CSAFree function definition not found!");
  for (auto &F : M) {
    LLVM_DEBUG(errs() << "Looking at " << F.getName() << "\n");
    if (F.isDeclaration()) {
      LLVM_DEBUG(errs() << "Only declaration\n");
      continue;
    }
    // Look for retInst
    ReturnInst *RI = getReturnInst(F);
    if (!RI)
      assert(0 && "Return Inst not found!!!!\n");
    // look at all of the instructions in each function
    for (BasicBlock &BB : F)
      for (auto II = std::begin(BB), E = std::end(BB); II != E;) {
        Instruction *I = &*II;
        ++II;
        if (AllocaInst *AI = dyn_cast<AllocaInst>(I)) {
          if (!AI->isStaticAlloca()) continue;
          LLVM_DEBUG(errs() << "Alloca Inst found; AI = " << *AI << "\n");
          Type *ty = AI->getAllocatedType();
          uint64_t Size = DL.getTypeAllocSize(ty);
          Value *SizeV = llvm::ConstantInt::get(Context, llvm::APInt(64, Size, false));
          if (SizeV->getType() != CSAMalloc->arg_begin()->getType())
            SizeV = new BitCastInst(SizeV, CSAMalloc->arg_begin()->getType(), "tmp", &*AI);
          CallInst *NewMallocCI = IRBuilder<>{AI}.CreateCall(CSAMalloc,SizeV,"tmp");
          BitCastInst *NewBCI = new BitCastInst(NewMallocCI,AI->getType(), "tmp", &*AI);
          IRBuilder<>{RI}.CreateCall(CSAFree,NewMallocCI);
          for (Value::user_iterator UI1 = AI->user_begin(), E1 = AI->user_end(); UI1 != E1; ++UI1) {
            Instruction *User1 = cast<Instruction>(*UI1);
            if (BitCastInst *BCI = dyn_cast<BitCastInst>(User1)) {
              if (BCI->use_empty()) addToDelete(BCI);
              for (auto UI2 = BCI->user_begin(), E2 = BCI->user_end(); UI2 != E2; ++UI2) {
                Instruction *User2 = cast<Instruction>(*UI2);
                if (CallInst *CI = dyn_cast<CallInst>(User2)) {
                  if (!CI->isInlineAsm() && isLifeTimeInst(CI)) {
                    addToDelete(CI);
                  }
                }
              } // End of UI2 loop
            }
            if (CallInst *CI = dyn_cast<CallInst>(User1))
              if (!CI->isInlineAsm() && isLifeTimeInst(CI)) {
                addToDelete(CI);
              }
          } // End of UI1 loop
          AI->replaceAllUsesWith(NewBCI);
          if (AI->use_empty()) addToDelete(AI);
        }
      } // End of II loop
    if (CoalesceCSAMallocs)
      coalesceMallocs(F, CSAMalloc, CSAFree, RI);
  } // End of F loop
  deleteInstructions();
  return true;
}

