//===------- DTransDtransPaddedMalloc.cpp - DTrans Padded Malloc -*------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
// This file does perform Padded Malloc
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/DTransPaddedMalloc.h"
#include "Intel_DTrans/Analysis/DTrans.h"
#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "Intel_DTrans/DTransCommon.h"
#include "llvm/Analysis/Intel_VPO/Utils/VPOAnalysisUtils.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Type.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/IPO.h"
#include <queue>

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "dtrans-paddedmalloc"

// Set the limit for the global counter used to identify if padded malloc
// can be applied or not
static cl::opt<unsigned> DTransPaddedMallocLimit("dtrans-paddedmalloc-limit",
                                                 cl::init(250),
                                                 cl::ReallyHidden);

// Set the size used to increase the memory allocation for padded malloc
static cl::opt<unsigned> DTransPaddedMallocSize("dtrans-paddedmalloc-size",
                                                cl::init(32), cl::ReallyHidden);
namespace {

class DTransPaddedMallocWrapper : public ModulePass {
private:
  dtrans::PaddedMallocPass Impl;

public:
  static char ID;

  DTransPaddedMallocWrapper() : ModulePass(ID) {
    initializeDTransPaddedMallocWrapperPass(*PassRegistry::getPassRegistry());
  }

  // Analyses needed
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DTransAnalysisWrapper>();
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequired<TargetLibraryInfoWrapperPass>();
    AU.addPreserved<WholeProgramWrapperPass>();
  }

  // Run the implementation
  bool runOnModule(Module &M) override {

    if (skipModule(M))
      return false;

    DTransAnalysisInfo &DTInfo =
        getAnalysis<DTransAnalysisWrapper>().getDTransInfo();

    const TargetLibraryInfo &TLInfo =
        getAnalysis<TargetLibraryInfoWrapperPass>().getTLI();

    // Lambda function to find the LoopInfo related to an input function
    dtrans::PaddedMallocPass::LoopInfoFuncType GetLI =
        [this](Function &F) -> LoopInfo & {
      return this->getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();
    };

    return Impl.runImpl(M, DTInfo, GetLI, TLInfo);
  }
};

} // end of anonymous namespace

// Traverse through each Function stored in PaddedMallocFuncs and
// apply the padded malloc optimization.
bool dtrans::PaddedMallocPass::applyPaddedMalloc(
    std::vector<Function *> &PaddedMallocFuncs, GlobalVariable *GlobalCounter,
    Function *PMFunc, Module *M, const TargetLibraryInfo &TLInfo,
    DTransAnalysisInfo &DTInfo, bool UseOpenMP) {

  bool FuncMod = false;

  LLVM_DEBUG(dbgs() << "  dtrans-paddedmalloc: Applying padded malloc" <<
      (UseOpenMP ? " with atomic operation" : "") << "\n");

  for (Function *F : PaddedMallocFuncs) {
    for (BasicBlock &BB : *F) {
      if (updateBasicBlock(BB, F, GlobalCounter, TLInfo, M, UseOpenMP)) {
        LLVM_DEBUG(dbgs() << "\tFunction update: " << F->getName() << "\n");
        FuncMod = true;
        break;
      }
    }
  }

  // In case none of the functions were updated, then remove the interface
  // and the global variable from the symbol table
  if (!FuncMod) {
    LLVM_DEBUG(dbgs() << "\tNone of the functions were updated\n");
    GlobalCounter->eraseFromParent();
    PMFunc->eraseFromParent();
  }

  DTInfo.setPaddedMallocInfo(DTransPaddedMallocSize, PMFunc);

  return FuncMod;
}

// Build the global variable related to padded malloc in the input Module and
// return it. This variable is a counter that identifies if the padded malloc
// will be used or not.
GlobalVariable *
dtrans::PaddedMallocPass::buildGlobalVariableCounter(Module &M) {

  IRBuilder<> Builder(M.getContext());
  ConstantInt *initVal = Builder.getInt32(0);

  GlobalVariable *PaddedMallocVar = new GlobalVariable(
      M, Builder.getInt32Ty(), false, GlobalValue::InternalLinkage, initVal,
      DTransPaddedMallocVar, nullptr, GlobalValue::NotThreadLocal, 0, false);

  // The constructor for a new GlobalVariable will automatically
  // generate an unique name if the variable's name has been used
  // already. If this name changes then the value of
  // DTransPaddedMallocVar must be updated.
  DTransPaddedMallocVar = PaddedMallocVar->getName();

  LLVM_DEBUG(dbgs() << "  dtrans-paddedmalloc: Global variable: "
                    << PaddedMallocVar->getName() << "\n");

  return PaddedMallocVar;
}

// Build an interface function in the input Module that checks
// if the input GlobalVariable hasn't reached the limit, and
// return the created function. The interface will look like this:
//
// define i1 @pmCounterCheck() {
//   %0 = load i32, i32* @PaddedMallocCounter
//   %1 = icmp ult i32 %1, 250
//   ret i1 %1
// }
//
// The function pmCounterCheck will return true if _pm_counter is
// lower that the limit (250 in this case), else return false.
Function *
dtrans::PaddedMallocPass::buildInterfaceFunction(Module *M,
                                                 GlobalVariable *GlobalCounter) {

  IRBuilder<> Builder(M->getContext());

  // Create the function first
  FunctionType *funcType = FunctionType::get(Builder.getInt1Ty(), false);
  Function *PMFunction = Function::Create(funcType, Function::ExternalLinkage,
                                          DTransPaddedMallocFunc, M);

  // Create the entry BasicBlock
  BasicBlock *entry = BasicBlock::Create(M->getContext(), "entry", PMFunction);

  Builder.SetInsertPoint(entry);

  // Insert the conditional for testing
  Value *PMLimitVal = Builder.getInt32(DTransPaddedMallocLimit);
  LoadInst *load = Builder.CreateLoad(GlobalCounter);
  Value *Cmp = Builder.CreateICmpULT(load, PMLimitVal);

  Builder.CreateRet(Cmp);

  // Update the name stored for the padded malloc interface
  // in case the name wasn't unique and the constructor
  // modified it.
  DTransPaddedMallocFunc = PMFunction->getName();

  LLVM_DEBUG(dbgs() << "  dtrans-paddedmalloc: Interface function: "
                    << DTransPaddedMallocFunc << "\n");
  return PMFunction;
}

// Trace through the Users of the input Instruction and return true if
// the input BranchInst is found. Else, return false. For example:
//
//  %12 = getelementptr inbounds [10 x i32], [10 x i32]* %1, i64 0, i64 %11
//  %13 = load i32, i32* %12, align 4
//  %14 = icmp eq i32 %13, %19
//  br i1 %14, label %25, label %15
//
// Here, the Instruction %12 will be the starting point to check if it ends
// in the branch. The Users of %12 will be %13. Then, the User of %13 is %14.
// The User of %14 is the branch, therefore it return true.
bool dtrans::PaddedMallocPass::checkDependence(Instruction *CheckInst,
                                               BranchInst *Branch) {

  if (!CheckInst)
    return false;

  if (CheckInst->user_empty())
    return false;

  // The number of instructions we are looking for is small (~3-4).
  llvm::SmallVector<Instruction *, 5> UsersVector;
  UsersVector.push_back(CheckInst);
  unsigned i = 0;

  for (i = 0; i < UsersVector.size(); i++) {

    Instruction *Inst = UsersVector[i];

    for (User *User : Inst->users()) {
      // The branch was found
      if (BranchInst *InstBranch = dyn_cast<BranchInst>(User)) {
        if (InstBranch == Branch)
          return true;
      }

      if (isa<GetElementPtrInst>(User) || isa<LoadInst>(User) ||
          isa<CmpInst>(User)) {
        Inst = dyn_cast<Instruction>(User);
        UsersVector.push_back(Inst);
      }
    }
  }

  return false;
}

// Return true if the input BasicBlock is a comparison between
// two pointer/array/vector entries in order to exit a loop.
// Else, return false. For example:
//
// ; <label>:1:
//  %2 = icmp ult i64 %10, 10
//  br i1 %2, label %3, label %11
//
// ; <label>:3:
//   %4 = phi i64 [ 0, %0 ], [ %10, %1 ]
//   %5 = getelementptr inbounds [10 x i32], [10 x i32]* @arr1, i64 0, i64 %4
//   %6 = load i32, i32* %5, align 4
//   %7 = getelementptr inbounds [10 x i32], [10 x i32]* @arr2, i64 0, i64 %4
//   %8 = load i32, i32* %7, align 4
//   %9 = icmp eq i32 %6, %8
//   %10 = add nuw nsw i64 %4, 1
//   br i1 %9, label %10, label %1
//
//; <label>:11:
//  *
//  *
//
// In this case, there are two array entries accessed (%5 and
// %7) and then compared (%9). The branching will be used to
// continue the search (BasicBlock 3) or to step out and do
// something else (BasicBlock 11).
bool dtrans::PaddedMallocPass::exitDueToSearch(BasicBlock &BB) {

  BranchInst *EndBranch = dyn_cast<BranchInst>(BB.getTerminator());
  if (!EndBranch)
    return false;

  unsigned ElemPtrsFound = 0;

  // Traverse the BasicBlock
  for (Instruction &Inst : BB) {
    GetElementPtrInst *ElemInst = dyn_cast<GetElementPtrInst>(&Inst);
    if (!ElemInst || !isValidType(ElemInst))
      continue;

    // Check if traversing the Users of the element pointer ends in
    // the branch
    if (checkDependence(ElemInst, EndBranch))
      ElemPtrsFound++;

    // If ElemPtrsFound is 2 then it means that we have two element pointers
    // that are used for the branching.
    if (ElemPtrsFound == 2)
      return true;
  }

  return false;
}

// Traverse through each field of the structures stored in DTInfo and check
// if the memory allocation for that field only happens in one Function. If
// so, then collect that Function and store it in PaddedMallocFuncs. Return
// true if at least one Function was collected, else return false.
bool dtrans::PaddedMallocPass::findFieldSingleValueFuncs(
    DTransAnalysisInfo &DTInfo, std::vector<Function *> &PaddedMallocFuncs) {

  LLVM_DEBUG(dbgs() << "  dtrans-paddedmalloc: Identifying alloc functions\n");

  // Go through the structures
  for (dtrans::TypeInfo *TyInfo : DTInfo.type_info_entries()) {
    auto *StInfo = dyn_cast<dtrans::StructInfo>(TyInfo);
    if (!StInfo)
      continue;

    // Go through each field of the structure
    for (dtrans::FieldInfo &FldInfo : StInfo->getFields()) {

      if (FldInfo.isSingleAllocFunction()) {
        Function *Fn = FldInfo.getSingleAllocFunction();
        PaddedMallocFuncs.push_back(Fn);
        LLVM_DEBUG(dbgs() << "\tAlloc Function: " << Fn->getName() << "\n");
      }
    }
  }

  if (PaddedMallocFuncs.empty()) {
    LLVM_DEBUG(dbgs() << "\tNo alloc functions found\n");
    return false;
  }

  return true;
}

// Return true if at least one Function in the input Module contains
// a search loop. The input function GetLI is used to collect the loops
// within each function.
bool dtrans::PaddedMallocPass::findSearchLoops(Module &M,
                                               LoopInfoFuncType &GetLI) {

  LLVM_DEBUG(dbgs() << "  dtrans-paddedmalloc: Identifying search loops\n");

  for (Function &F : M) {
    if (funcHasSearchLoop(F, GetLI)) {
      return true;
    }
  }

  LLVM_DEBUG(dbgs() << "\tNo search loops found\n");
  return false;
}

// Return true if the input Function contains a search loop. The function
// GetLI is used to collect the LoopInfo analysis related to the Function.
bool dtrans::PaddedMallocPass::funcHasSearchLoop(Function &Fn,
                                                 LoopInfoFuncType &GetLI) {

  if (Fn.isDeclaration())
    return false;

  LoopInfo &LI = (GetLI)(Fn);

  if (LI.empty())
    return false;

  // Go through each BasicBlock in the Function
  for (BasicBlock &BB : Fn) {

    // Check if the BasicBlock will exit a loop
    Loop *LoopBB = LI.getLoopFor(&BB);
    if (!isExitLoop(LoopBB, &BB))
      continue;

    // Check if the element pointers in the BasicBlock will
    // exit the loop due to a pointer/array/vector entries
    // comparison
    if (exitDueToSearch(BB)) {
      LLVM_DEBUG(dbgs() << "\tSearch loop found in: " << Fn.getName() << "\n");
      return true;
    }
  }
  return false;
}

// Return true if at least one of the BasicBlock's successors goes
// out of the loop.
bool dtrans::PaddedMallocPass::isExitLoop(Loop *LoopData, BasicBlock *BB) {

  if (!LoopData)
    return false;

  for (BasicBlock *SuccBB : successors(BB)) {
    if (LoopData->isLoopExiting(SuccBB))
      return true;
  }

  return false;
}

// A GetElementPtrInst can represent a pointer to a space allocated
// (e.g. p[i]), an array, vector, struct or even a class. This function will
// return true if the input GetElementPtr references to a memory space, array
// or vector. Else, return false.
bool dtrans::PaddedMallocPass::isValidType(GetElementPtrInst *ElemInst) {

  if (!ElemInst->hasIndices())
    return false;

  Type *Type = ElemInst->getPointerOperandType();

  if (Type->isPointerTy()) {
    Type = Type->getPointerElementType();
    // pointer to an integer or floating point
    if (Type->isIntegerTy() || Type->isFloatingPointTy())
      return true;
  }

  // An array of integers or floating point
  if (Type->isArrayTy() && (Type->getArrayElementType()->isIntegerTy() ||
                            Type->getArrayElementType()->isFloatingPointTy()))
    return true;

  // A vector of integers of floating point
  if (Type->isVectorTy() && (Type->getVectorElementType()->isIntegerTy() ||
                             Type->getVectorElementType()->isFloatingPointTy()))
    return true;

  // Type represents something else that is not needed in a search loop
  return false;
}

// If the BasicBlock contains a malloc CallSite, then apply the padded
// malloc optimization.
bool dtrans::PaddedMallocPass::updateBasicBlock(BasicBlock &BB, Function *F,
                                                GlobalVariable *GlobalCounter,
                                                const TargetLibraryInfo &TLInfo,
                                                Module *M, bool UseOpenMP) {

  // Traverse the instructions in the BasicBlock
  for (Instruction &Inst: BB) {

    CallSite CS = CallSite(&Inst);
    if (!CS.isCall() && !CS.isInvoke())
      continue;

    // Check that the instruction is a call site to malloc
    if (dtrans::getAllocFnKind(CS.getCalledFunction(), TLInfo) !=
        dtrans::AK_Malloc) {
      continue;
    }

    // Clone the instruction
    Instruction *mallocCallMod = &Inst;
    Instruction *mallocCall = mallocCallMod->clone();

    IRBuilder<> Builder(M->getContext());
    Builder.SetInsertPoint(mallocCallMod);

    // Insert the conditional for the multiversioning
    Value *PMLimitVal = Builder.getInt32(DTransPaddedMallocLimit);
    LoadInst *load = Builder.CreateLoad(GlobalCounter);
    Value *Cmp = Builder.CreateICmpULT(load, PMLimitVal);

    // Build the BasicBlocks structure
    BasicBlock *BBMerge = BB.splitBasicBlock(mallocCallMod);
    BasicBlock *BBIf = BasicBlock::Create(M->getContext(), "BBif", F);
    BasicBlock *BBElse = BasicBlock::Create(M->getContext(), "BBelse", F);

    // Reorganize the BasicBlocks
    BBIf->moveBefore(BBMerge);
    BBElse->moveBefore(BBMerge);

    // Remove the old terminator for BB
    BB.getTerminator()->eraseFromParent();

    // The new terminator for BB will be the branch between BBIf and BBElse
    Builder.SetInsertPoint(&BB);
    Builder.CreateCondBr(Cmp, BBIf, BBElse);

    // Set the if side
    mallocCallMod->removeFromParent();
    Builder.SetInsertPoint(BBIf);
    Value *InputVal = CS.getArgument(0);
    // Collect the BitWidth of the input Value for the malloc
    // function and use that size to create the new add operation
    unsigned BitWidthSize = InputVal->getType()->getIntegerBitWidth();
    Value *PMSizeVal = Builder.getIntN(BitWidthSize, DTransPaddedMallocSize);
    Value *NewSize = Builder.CreateAdd(InputVal, PMSizeVal);
    CS.setArgument(0, NewSize);
    Value *MallocMod = Builder.Insert(mallocCallMod);
    // Increase the global variable
    if (UseOpenMP)
      Builder.CreateAtomicRMW(AtomicRMWInst::BinOp::Add, GlobalCounter,
                              Builder.getInt32(1),
                              AtomicOrdering::AcquireRelease, 1);
    else {
      Value *addOp = Builder.CreateAdd(Builder.getInt32(1), load);
      Builder.CreateStore(addOp, GlobalCounter);
    }
    Builder.CreateBr(BBMerge);

    // Set the else side
    Builder.SetInsertPoint(BBElse);
    Value *MallocOrig = Builder.Insert(mallocCall);
    Builder.CreateBr(BBMerge);

    // Set the merge side
    Builder.SetInsertPoint(&(BBMerge->front()));
    PHINode *Phi = Builder.CreatePHI(MallocMod->getType(), 2);
    MallocMod->replaceAllUsesWith(Phi);
    Phi->addIncoming(MallocMod, BBIf);
    Phi->addIncoming(MallocOrig, BBElse);

    return true;
  }

  return false;
}

char DTransPaddedMallocWrapper::ID = 0;
INITIALIZE_PASS_BEGIN(DTransPaddedMallocWrapper, "dtrans-paddedmalloc",
                      "DTrans padded malloc", false, false)
INITIALIZE_PASS_DEPENDENCY(DTransAnalysisWrapper)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_END(DTransPaddedMallocWrapper, "dtrans-paddedmalloc",
                    "DTrans padded malloc", false, false)

ModulePass *llvm::createDTransPaddedMallocWrapperPass() {
  return new DTransPaddedMallocWrapper();
}

// Actual implementation of padded malloc
bool dtrans::PaddedMallocPass::runImpl(Module &M, DTransAnalysisInfo &DTInfo,
                                       LoopInfoFuncType &GetLI,
                                       const TargetLibraryInfo &TLInfo) {

  LLVM_DEBUG(dbgs() << "dtrans-paddedmalloc: Trace for DTrans Padded Malloc"
                    << "\n");

  // Collect the functions that padded malloc can be applied
  std::vector<Function *> PaddedMallocFuncs;
  if (!findFieldSingleValueFuncs(DTInfo, PaddedMallocFuncs)) {
    return false;
  }

  // Look for search loops
  if (!findSearchLoops(M, GetLI)) {
    return false;
  }

  // Build the global counter and the interface to access it
  GlobalVariable *GlobalCounter = buildGlobalVariableCounter(M);
  Function *PMFunc = buildInterfaceFunction(&M, GlobalCounter);

  // TODO: UseOpenMP needs to be fixed using a mechanism from
  // the driver that collects the OpenMP flags passed by the user.
  return applyPaddedMalloc(PaddedMallocFuncs, GlobalCounter, PMFunc, &M, TLInfo,
                           DTInfo, /*UseOpenMP*/ false);
}

PreservedAnalyses dtrans::PaddedMallocPass::run(Module &M,
                                                ModuleAnalysisManager &AM) {

  auto &DTransInfo = AM.getResult<DTransAnalysis>(M);
  auto &TLInfo = AM.getResult<TargetLibraryAnalysis>(M);

  FunctionAnalysisManager &FAM =
      AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();

  LoopInfoFuncType GetLI = [&FAM](Function &F) -> LoopInfo & {
    return FAM.getResult<LoopAnalysis>(F);
  };

  if (!runImpl(M, DTransInfo, GetLI, TLInfo))
    return PreservedAnalyses::all();

  // TODO: Mark the actual preserved analyses.
  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}
