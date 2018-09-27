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

#include "CSASubtarget.h"
#include "CSA.h"
#include "CSATargetMachine.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/SmallSet.h"
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
Pass *createCSAReplaceAllocaWithMallocPass(CSATargetMachine &TM);
} // namespace llvm

namespace {
struct CSAReplaceAllocaWithMalloc : public ModulePass {
  static char ID;

  explicit CSAReplaceAllocaWithMalloc(CSATargetMachine &TM) : ModulePass(ID),
                                                              ST(*TM.getSubtargetImpl()) {}
  StringRef getPassName() const override { return PASS_DESC; }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }

  bool runOnModule(Module &M) override;
private:
  std::set<Instruction *> ToDelete;
  const CSASubtarget &ST;
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

Pass *llvm::createCSAReplaceAllocaWithMallocPass(CSATargetMachine &TM) { return new CSAReplaceAllocaWithMalloc(TM); }

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
  int BitWidth = CSAMalloc->arg_begin()->getType()->getIntegerBitWidth();
  Value *SizeV = llvm::ConstantInt::get(Context, llvm::APInt(BitWidth, Size, true));
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
          Size += dyn_cast<ConstantInt>(CI->getOperand(0))->getSExtValue();
          Value *Offset = llvm::ConstantInt::get(Context, llvm::APInt(32, Size, true));
          Value *NewGEP = IRBuilder<>{CI}
                          .CreateGEP(cast<PointerType>(FirstCSAMallocInst->getType())->getElementType(), 
                                                       FirstCSAMallocInst, Offset, "tmp");
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
          if (AI->isStaticAlloca()) {
            LLVM_DEBUG(errs() << "Alloca Inst found in " << F.getName() << "\n");
            return true;
          }
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

// @llvm.used is an array of global constants created by llvm.
// It contains all globals marked with llvm.used attribute
// csa_mem_alloc, csa_mem_free, and csa_mem_initialize functions are marked with this attribute
// so that they are not internalized and deleted by dead code elimination before their references
// are introduced in this pass
// After this pass, the csa_mem* will have valid references and should not rely on the llvm.used attribute
// In these cases, these functions should be removed from the @llvm.used array
static void removeCSAMemoryFunctionsFromUsedList(Module &M, Function *CSAMalloc,
                                     Function *CSAFree, Function *CSAInitialize) {
  SmallVector<Constant *, 32> FuncsMarkedAsUsed;
  GlobalVariable *UsedV = M.getGlobalVariable("llvm.used");
  if (!UsedV || !UsedV->hasInitializer()) return;
  const ConstantArray *UsedList = cast<ConstantArray>(UsedV->getInitializer());
  for (unsigned i = 0, e = UsedList->getNumOperands(); i != e; ++i)
    if (Function *F =
        dyn_cast<Function>(UsedList->getOperand(i)->stripPointerCasts())) {
      if (F != CSAMalloc && F != CSAFree && F != CSAInitialize)
        FuncsMarkedAsUsed.push_back(UsedList->getOperand(i));
    } else
      FuncsMarkedAsUsed.push_back(UsedList->getOperand(i));
  auto UsedLinkage = UsedV->getLinkage();
  auto UsedVName = UsedV->getName();
  UsedV->eraseFromParent();
  if (FuncsMarkedAsUsed.size()) {
    ArrayType *ArrTy = ArrayType::get(UsedList->getType()->getElementType(),FuncsMarkedAsUsed.size());
    auto *NewUsedV = new GlobalVariable(M,ArrTy,true,UsedLinkage,0,UsedVName);
    NewUsedV->setInitializer(ConstantArray::get(ArrTy,FuncsMarkedAsUsed));
  }
}

// Function to delete all csa_mem* functions if needed
static void deleteCSAMemoryFunctions(Function *CSAMalloc,
                                     Function *CSAFree, Function *CSAInitialize) {
  if (CSAMalloc)     { CSAMalloc->eraseFromParent(); }
  if (CSAFree)       { CSAFree->eraseFromParent(); }
  if (CSAInitialize) { CSAInitialize->eraseFromParent(); }
}

// Given a CallInst. this function returns a handle to the callee function
// This function is duplicated in CSAProcedureCalls pass
// Changes will need to be synchronized
static const Function *getLoweredFunc(const CallInst *CI, const Module &M) {
  const Function *LowerF = nullptr;
  if (!CI->getCalledFunction()) return nullptr;
  auto F = CI->getCalledFunction();
  auto IID = F->getIntrinsicID();
  if (IID == Intrinsic::not_intrinsic) {
    if (F->isDeclaration()) return nullptr;
    LowerF = F;
  }
  LLVM_DEBUG(errs() << "CI = " << *CI << " and num ops = " << CI->getNumOperands() << "\n");
  if (CI->getNumArgOperands() == 0) return nullptr;
  bool IsFloat = (CI->getArgOperand(0)->getType()->getTypeID() == Type::FloatTyID);
  bool IsDouble = (CI->getArgOperand(0)->getType()->getTypeID() == Type::DoubleTyID);
  if (IsDouble) {
    switch (IID) {
    case Intrinsic::ceil: { LowerF = M.getFunction("ceil"); break; }
    case Intrinsic::cos: { LowerF = M.getFunction("cos"); break; }
    case Intrinsic::exp: { LowerF = M.getFunction("exp"); break; }
    case Intrinsic::exp2: { LowerF = M.getFunction("exp2"); break; }
    case Intrinsic::floor: { LowerF = M.getFunction("floor"); break; }
    case Intrinsic::log: { LowerF = M.getFunction("log"); break; }
    case Intrinsic::log2: { LowerF = M.getFunction("log2"); break; }
    case Intrinsic::log10: { LowerF = M.getFunction("log10"); break; }
    case Intrinsic::pow: { LowerF = M.getFunction("pow"); break; }
    case Intrinsic::round: { LowerF = M.getFunction("round"); break; }
    case Intrinsic::sin: { LowerF = M.getFunction("sin"); break; }
    case Intrinsic::trunc: { LowerF = M.getFunction("trunc"); break; }
    default: { break; }
    }
  } else if (IsFloat) {
    switch (IID) {
    case Intrinsic::ceil: { LowerF = M.getFunction("ceilf"); break; }
    case Intrinsic::cos: { LowerF = M.getFunction("cosf"); break; }
    case Intrinsic::exp: { LowerF = M.getFunction("expf"); break; }
    case Intrinsic::exp2: { LowerF = M.getFunction("exp2f"); break; }
    case Intrinsic::floor: { LowerF = M.getFunction("floorf"); break; }
    case Intrinsic::log: { LowerF = M.getFunction("logf"); break; }
    case Intrinsic::log2: { LowerF = M.getFunction("log2f"); break; }
    case Intrinsic::log10: { LowerF = M.getFunction("log10f"); break; }
    case Intrinsic::pow: { LowerF = M.getFunction("powf"); break; }
    case Intrinsic::round: { LowerF = M.getFunction("roundf"); break; }
    case Intrinsic::sin: { LowerF = M.getFunction("sinf"); break; }
    case Intrinsic::trunc: { LowerF = M.getFunction("truncf"); break; }
    default: { break; }
    }
  }
  return LowerF;
}

// This deletes all unused math library functions
static void deleteUnusedMathFunctions(Module &M, Function *CSAMalloc,
                                     Function *CSAFree, Function *CSAInitialize, const CSASubtarget &ST) {
  SmallSet<Constant *, 32> FuncsMarkedAsUsed;
  SmallVector<Constant *, 32> FuncsMarkedAsUsed_V;
  SmallVector<Function *, 32> FuncsToDelete;
  GlobalVariable *UsedV = M.getGlobalVariable("llvm.used");
  if (!UsedV || !UsedV->hasInitializer()) return;
  const ConstantArray *UsedList = cast<ConstantArray>(UsedV->getInitializer());
  auto UsedLinkage = UsedV->getLinkage();
  auto UsedVName = UsedV->getName();
  for (auto &F : M) {
    for (BasicBlock &BB : F) {
      for (auto II = std::begin(BB), E = std::end(BB); II != E;) {
        Instruction *I = &*II;
        ++II;
        const Function *LowerF;
        if (const CallInst *CI = dyn_cast<const CallInst>(I)) {
          LLVM_DEBUG(errs() << "CI = " << *CI << "\n");
          LowerF = getLoweredFunc(CI,M);
          // if this function is marked with llvm.used, add to FuncsMarkedAsUsed set
          if (LowerF != nullptr) {
            for (unsigned i = 0, e = UsedList->getNumOperands(); i != e; ++i)
              if (Function *F =
                    dyn_cast<Function>(UsedList->getOperand(i)->stripPointerCasts()))
              if (F == LowerF)
                FuncsMarkedAsUsed.insert(UsedList->getOperand(i));
          }
        }
      }
    }
  }

  // FuncsMarkedAsUsed_V is a subset of UsedList
  // All functions in UsedList that have atleast one call-site and are not csa_mem*
  // functions are copied to FuncsMarkedAsUsed_V
  // All the other functions in UsedList that have no call-sites will be marked for deletion
  for (unsigned i = 0, e = UsedList->getNumOperands(); i != e; ++i)
    if (Function *F = dyn_cast<Function>(UsedList->getOperand(i)->stripPointerCasts())) {
      // The function will be marked for deletion if it is not any of the csa_mem* functions
      // and (if it is not called anywhere or if the Math0 option for CSA subtarget is turned on)
      if (F != CSAMalloc && F != CSAFree && F != CSAInitialize && (FuncsMarkedAsUsed.count(UsedList->getOperand(i)) == 0 || ST.hasMath0())) {
        FuncsToDelete.push_back(F);
      } else {
        FuncsMarkedAsUsed_V.push_back(UsedList->getOperand(i));
        F->setLinkage(llvm::Function::InternalLinkage);
      }
    }
  UsedV->eraseFromParent();
  if (FuncsMarkedAsUsed_V.size()) {
    ArrayType *ArrTy = ArrayType::get(UsedList->getType()->getElementType(),FuncsMarkedAsUsed_V.size());
    auto *NewUsedV = new GlobalVariable(M,ArrTy,true,UsedLinkage,0,UsedVName);
    NewUsedV->setInitializer(ConstantArray::get(ArrTy,FuncsMarkedAsUsed_V));
  }

  // sincos and sincosf are forcibly deleted if not needed
  // this might be removed later with better DCE support
  bool SinDeleted = false;
  bool CosDeleted = false;
  bool SinfDeleted = false;
  bool CosfDeleted = false;
  for (auto F: FuncsToDelete) {
    if (F->getName() == "sin") SinDeleted = true;
    if (F->getName() == "cos") CosDeleted = true;
    if (F->getName() == "sinf") SinfDeleted = true;
    if (F->getName() == "cosf") CosfDeleted = true;
    F->eraseFromParent();
  }
  if (SinDeleted && CosDeleted) {
    Function *F = M.getFunction("sincos");
    if (F) F->eraseFromParent();
  }
  if (SinfDeleted && CosfDeleted) {
    Function *F = M.getFunction("sincosf");
    if (F) F->eraseFromParent();
  }
}

bool CSAReplaceAllocaWithMalloc::runOnModule(Module &M) {
  const DataLayout &DL = M.getDataLayout();
  LLVMContext &Context = M.getContext();
  Function *CSAMalloc = M.getFunction("csa_mem_alloc");
  if (!CSAMalloc) CSAMalloc = M.getFunction("CsaMemAlloc");
  Function *CSAFree = M.getFunction("csa_mem_free");
  if (!CSAFree) CSAFree = M.getFunction("CsaMemFree");
  Function *CSAInitialize = M.getFunction("csa_mem_initialize");
  if (!CSAInitialize) CSAInitialize = M.getFunction("CsaMemInitialize");

  // delete all math functions from llvm.used if Math0 is enabled
  // delete only unused math functions from llvm.used if Math0 is disabled
  deleteUnusedMathFunctions(M,CSAMalloc,CSAFree,CSAInitialize,ST);

  // Delete csa_mem* functions if SXU linkage selected or if there is no stack required
  if (!csa_utils::isAlwaysDataFlowLinkageSet()) {
    LLVM_DEBUG(errs() << "Data flow linkage not set\n");
    removeCSAMemoryFunctionsFromUsedList(M,CSAMalloc,CSAFree,CSAInitialize);
    deleteCSAMemoryFunctions(CSAMalloc,CSAFree,CSAInitialize);
    return false;
  }
  bool IsAllocaPresent = isAllocaPresent(M);
  if (!IsAllocaPresent) {
    LLVM_DEBUG(errs() << "No stack allocations found. No need to run this pass\n");
    removeCSAMemoryFunctionsFromUsedList(M,CSAMalloc,CSAFree,CSAInitialize);
    deleteCSAMemoryFunctions(CSAMalloc,CSAFree,CSAInitialize);
    return false;
  }
  if (!CSAMalloc || CSAMalloc->isDeclaration())
    report_fatal_error("CSAMalloc function definition not found!");
  if (!CSAFree || CSAFree->isDeclaration())
    report_fatal_error("CSAFree function definition not found!");
  if (!CSAInitialize || CSAInitialize->isDeclaration())
    report_fatal_error("CSAInitialize function definition not found!");
  for (auto &F : M) {
    LLVM_DEBUG(errs() << "Looking at " << F.getName() << "\n");
    if (F.isDeclaration()) {
      LLVM_DEBUG(errs() << "Only declaration\n");
      continue;
    }
    // Look for retInst
    ReturnInst *RI = getReturnInst(F);
    if (!RI)
      report_fatal_error("Return Inst not found!!!!\n");
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
          int BitWidth = CSAMalloc->arg_begin()->getType()->getIntegerBitWidth();
          Value *SizeV = llvm::ConstantInt::get(Context, llvm::APInt(BitWidth, Size, true));
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
  removeCSAMemoryFunctionsFromUsedList(M,CSAMalloc,CSAFree,CSAInitialize);
  // Fix properties for CSAInitialize
  CSAInitialize->setLinkage(llvm::Function::ExternalLinkage);
  CSAInitialize->removeFromParent();
  M.getFunctionList().push_back(CSAInitialize);
  return true;
}

