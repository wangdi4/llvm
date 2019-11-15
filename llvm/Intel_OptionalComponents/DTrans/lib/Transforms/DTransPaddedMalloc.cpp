//===------- DtransPaddedMalloc.cpp - DTrans Padded Malloc -*------===//
//
// Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
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
#include "llvm/Analysis/VPO/Utils/VPOAnalysisUtils.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Type.h"
#include "llvm/InitializePasses.h"
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

// Build the global variable and interface if true. Used for testing purposes.
static cl::opt<bool> DTransTestPaddedMalloc("dtrans-test-paddedmalloc",
                                            cl::init(false), cl::ReallyHidden);

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
    AU.addRequired<WholeProgramWrapperPass>();
    AU.addPreserved<WholeProgramWrapperPass>();
    AU.addPreserved<DTransAnalysisWrapper>();
  }

  // Run the implementation
  bool runOnModule(Module &M) override {

    if (skipModule(M))
      return false;

    DTransAnalysisInfo &DTInfo =
        getAnalysis<DTransAnalysisWrapper>().getDTransInfo(M);

    auto GetTLI = [this](Function &F) -> TargetLibraryInfo & {
      return this->getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
    };

    WholeProgramInfo &WPInfo =
        getAnalysis<WholeProgramWrapperPass>().getResult();

    // Lambda function to find the LoopInfo related to an input function
    dtrans::PaddedMallocPass::LoopInfoFuncType GetLI =
        [this](Function &F) -> LoopInfo & {
      return this->getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();
    };

    return Impl.runImpl(M, DTInfo, GetLI, GetTLI, WPInfo);
  }
};

} // end of anonymous namespace

// Traverse through each Function stored in PaddedMallocFuncs and
// apply the padded malloc optimization.
bool dtrans::PaddedMallocPass::applyPaddedMalloc(
    std::vector<PaddedMallocFunc> &PaddedMallocVect,
    GlobalVariable *GlobalCounter, Function *PMFunc, Module *M,
    const TargetLibraryInfo &TLInfo, DTransAnalysisInfo &DTInfo) {

  bool FuncMod = false;

  LLVM_DEBUG(dbgs() << "  dtrans-paddedmalloc: Applying padded malloc\n");

  for (auto &Entry : PaddedMallocVect) {
    for (BasicBlock &BB : *(Entry.first)) {
      if (updateBasicBlock(BB, Entry.first, GlobalCounter, TLInfo, M,
                           Entry.second)) {
        LLVM_DEBUG(dbgs() << "\tFunction updated: " << Entry.first->getName()
                          << (Entry.second ? " with atomic operation" : "")
                          << "\n");
        FuncMod = true;
        break;
      }
    }
  }

  // In case none of the functions were updated, then remove the interface
  // and the global variable from the symbol table
  if (!FuncMod) {
    LLVM_DEBUG(dbgs() << "\tNone of the functions were updated\n");
    PaddedMallocData.destroyGlobalsInfo(*M);
  }

  return FuncMod;
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

// Traverse through the PaddedMallocVect and identify which functions are used
// inside a parallel region.
void dtrans::PaddedMallocPass::checkForParallelRegion(
    Module &M, std::vector<PaddedMallocFunc> &PaddedMallocVect) {

  // Note: There are two types of OpenMP outlining:
  //  a) -fopenmp:  OpenMP outlining is done by CFE (frontend)
  //  b) -fiopenmp: OpenMP outlining is done by VPO (backend)
  //
  // This implementation is for identifying the OpenMP outline
  // function if -fiopenmp was used (backend outlining).
  //
  // TODO: In case icc -xllvm supports -fopenmp (frontend
  // outlining), then one of the following options must be
  // implemented:
  //
  //   a) Traverse the symbol table and identify if there is any
  //      function with "__kmpc_" in its name. If so, assume that all
  //      malloc functions will need atomic operations.
  //   b) The driver passes to the backend that -fopenmp is being
  //      used and assume that all malloc functions will need atomic
  //      operations.
  //   c) The CFE must set the attributes mt-func or task-mt-func to
  //      the outline functions when they are created. (Nothing to
  //      update here).
  //   d) The CFE sets an special attribute (e.g. is-omp-outline)
  //      to the outline functions when they are created. The
  //      function isOutlineFunction must be updated so it can recognize
  //      the new attribute.

  // Check if there is a parallel region
  bool HasOpenMP = false;
  for (Function &F : M) {
    if (VPOAnalysisUtils::mayHaveOpenmpDirective(F) || isOutlineFunction(&F)) {
      HasOpenMP = true;
      break;
    }
  }

  if (!HasOpenMP)
    return;

  // Check if the malloc Functions are inside a parallel region
  SmallPtrSet<Function *, 10> VisitedFuncs;
  unsigned i = 0;
  for (i = 0; i < PaddedMallocVect.size(); i++) {
    if (insideParallelRegion(PaddedMallocVect[i].first, VisitedFuncs))
      PaddedMallocVect[i].second = true;
    VisitedFuncs.clear();
  }
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
    DTransAnalysisInfo &DTInfo,
    std::vector<PaddedMallocFunc> &PaddedMallocVect) {

  LLVM_DEBUG(dbgs() << "  dtrans-paddedmalloc: Identifying alloc functions\n");

  // Go through the structures
  for (dtrans::TypeInfo *TyInfo : DTInfo.type_info_entries()) {
    auto *StInfo = dyn_cast<dtrans::StructInfo>(TyInfo);
    if (!StInfo)
      continue;

    // Go through each field of the structure
    for (dtrans::FieldInfo &FldInfo : StInfo->getFields()) {

      if (FldInfo.isSingleAllocFunction() &&
          !FldInfo.getSingleAllocFunction()->isDeclaration()) {
        Function *Fn = FldInfo.getSingleAllocFunction();
        PaddedMallocVect.push_back({Fn, false});
        LLVM_DEBUG(dbgs() << "\tAlloc Function: " << Fn->getName() << "\n");
      }
    }
  }

  if (PaddedMallocVect.empty()) {
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

// Return true if Fn is being called from an OpenMP region,
// else return false.
bool dtrans::PaddedMallocPass::insideParallelRegion(
    Function *Fn, SmallPtrSet<Function *, 10> &VisitedFuncs) {

  if (!Fn)
    return false;

  if (Fn->hasAddressTaken())
    return true;

  VisitedFuncs.insert(Fn);

  for (User *User : Fn->users()) {

    if (Instruction *Inst = dyn_cast<Instruction>(User)) {
      if (!isa<CallInst>(Inst) && !isa<InvokeInst>(Inst))
        continue;

      Function *Caller = cast<CallBase>(Inst)->getCaller();

      if (isOutlineFunction(Caller)) {
        return true;
      }

      if (VisitedFuncs.find(Caller) != VisitedFuncs.end())
        continue;

      if (insideParallelRegion(Caller, VisitedFuncs))
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
    if (LoopData->contains(SuccBB) && LoopData->isLoopExiting(SuccBB))
      return true;
  }

  return false;
}

// Return true if the input Function is an OpenMP outline function,
// else return false
bool dtrans::PaddedMallocPass::isOutlineFunction(Function *F) {

  AttributeList FnAttr = F->getAttributes();

  // Note: These attributes are set if the OpenMP outlining is done by the
  // backend (-fiopenmp).
  if (FnAttr.hasAttribute(AttributeList::FunctionIndex, "mt-func") ||
      FnAttr.hasAttribute(AttributeList::FunctionIndex, "task-mt-func"))
    return true;

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

// If the BasicBlock contains a malloc call, then apply the padded
// malloc optimization.
bool dtrans::PaddedMallocPass::updateBasicBlock(BasicBlock &BB, Function *F,
                                                GlobalVariable *GlobalCounter,
                                                const TargetLibraryInfo &TLInfo,
                                                Module *M, bool UseOpenMP) {

  // Traverse the instructions in the BasicBlock
  for (Instruction &Inst : BB) {

    CallBase *Call = dyn_cast<CallBase>(&Inst);
    if (!Call)
      continue;

    // Check that the instruction is a call to malloc
    if (dtrans::getAllocFnKind(Call, TLInfo) != dtrans::AK_Malloc) {
      continue;
    }

    // The input value to the malloc must always
    // be an integer
    Value *InputVal = Call->getArgOperand(0);
    Type *IntType = InputVal->getType();
    if (!IntType->isIntegerTy())
      continue;

    // Clone the instruction
    Instruction *mallocCallMod = &Inst;
    Instruction *mallocCall = mallocCallMod->clone();

    IRBuilder<> Builder(M->getContext());
    Builder.SetInsertPoint(mallocCallMod);

    // Insert the conditional for the multiversioning
    Value *PMLimitVal = Builder.getInt32(getPaddedMallocLimit());
    LoadInst *LoadGlobal = Builder.CreateLoad(GlobalCounter);
    if (UseOpenMP) {
      LoadGlobal->setAtomic(AtomicOrdering::SequentiallyConsistent);
      LoadGlobal->setAlignment(MaybeAlign(4));
    }
    Value *Cmp = Builder.CreateICmpULT(LoadGlobal, PMLimitVal);

    // Build the BasicBlock for checking with the limit
    Instruction *LoadInst = cast<Instruction>(LoadGlobal);
    BasicBlock *NewBB = BB.splitBasicBlock(LoadInst);

    // Build the BasicBlocks structure
    BasicBlock *BBMerge = NewBB->splitBasicBlock(mallocCallMod);
    BasicBlock *BBIf = BasicBlock::Create(M->getContext(), "BBif", F);
    BasicBlock *BBElse = BasicBlock::Create(M->getContext(), "BBelse", F);

    // Reorganize the BasicBlocks
    BBIf->moveBefore(BBMerge);
    BBElse->moveBefore(BBMerge);

    // Remove the old terminator for NewBB
    NewBB->getTerminator()->eraseFromParent();

    // The new terminator for BB will be the branch between BBIf and BBElse
    Builder.SetInsertPoint(NewBB);
    Builder.CreateCondBr(Cmp, BBIf, BBElse);

    // Create the check for INT_MAX
    BasicBlock *TurnOffBB = BasicBlock::Create(M->getContext(), "MaxBB",
                                               F, BBIf);

    Builder.SetInsertPoint(&BB);
    BB.getTerminator()->eraseFromParent();

    // Generate the INT_MAX limit
    APInt MaxIntAP = APInt::getMaxValue(32);
    Type *Int32Ty = Type::getInt32Ty(M->getContext());
    Value *MaxInt = cast<Value>(ConstantInt::get(Int32Ty, MaxIntAP));

    // We need to cast the value of MaxInt to the integer type
    // that is being used by the input value of malloc
    Value *IntCast = Builder.CreateIntCast(MaxInt, IntType,
                                            false /* isSigned */);

    Value *LimitCmp = Builder.CreateICmpULT(InputVal, IntCast);
    // If the size of the malloc function is lower than INT_MAX,
    // then proceed with checking with the padded malloc limit.
    // Else, turn off the padded malloc by setting the value of
    // __Intel_PaddedMallocCounter to the limit.
    Builder.CreateCondBr(LimitCmp, NewBB, TurnOffBB);
    Builder.SetInsertPoint(TurnOffBB);
    // Increase to the limit
    if (UseOpenMP)
      // Use the atomic exchange in case we are using OpenMP
      Builder.CreateAtomicRMW(AtomicRMWInst::BinOp::Xchg, GlobalCounter,
                              PMLimitVal,
                              AtomicOrdering::SequentiallyConsistent, 1);
    else
      // Else use the regular store
      Builder.CreateStore(PMLimitVal, GlobalCounter);

    // Go to the branch that has the regular malloc
    Builder.CreateBr(BBElse);

    Builder.SetInsertPoint(NewBB);

    // Set the if side
    mallocCallMod->removeFromParent();
    Builder.SetInsertPoint(BBIf);
    // Collect the BitWidth of the input Value for the malloc
    // function and use that size to create the new add operation
    unsigned BitWidthSize = InputVal->getType()->getIntegerBitWidth();
    Value *PMSizeVal = Builder.getIntN(BitWidthSize, DTransPaddedMallocSize);
    Value *NewSize = Builder.CreateAdd(InputVal, PMSizeVal);
    Call->setArgOperand(0, NewSize);
    Value *MallocMod = Builder.Insert(mallocCallMod);
    // Increase the global variable
    if (UseOpenMP)
      Builder.CreateAtomicRMW(AtomicRMWInst::BinOp::Add, GlobalCounter,
                              Builder.getInt32(1),
                              AtomicOrdering::SequentiallyConsistent, 1);
    else {
      Value *addOp = Builder.CreateAdd(Builder.getInt32(1), LoadGlobal);
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
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_END(DTransPaddedMallocWrapper, "dtrans-paddedmalloc",
                    "DTrans padded malloc", false, false)

ModulePass *llvm::createDTransPaddedMallocWrapperPass() {
  return new DTransPaddedMallocWrapper();
}

unsigned llvm::getPaddedMallocLimit() {
  return DTransPaddedMallocLimit;
}

// Actual implementation of padded malloc
bool dtrans::PaddedMallocPass::runImpl(
    Module &M, DTransAnalysisInfo &DTInfo, LoopInfoFuncType &GetLI,
    std::function<const TargetLibraryInfo &(Function &)> GetTLI,
    WholeProgramInfo &WPInfo) {

  if (!WPInfo.isWholeProgramSafe())
    return false;

  if (!DTInfo.useDTransAnalysis())
    return false;

  // TODO: Guard the optimization with -memory-layout-trans=3 when
  // support is available.

  LLVM_DEBUG(dbgs() << "dtrans-paddedmalloc: Trace for DTrans Padded Malloc"
                    << "\n");

  if (DTransTestPaddedMalloc) {
    PaddedMallocData.buildGlobalsInfo(M);
  } else if (!PaddedMallocData.isPaddedMallocDataAvailable(M)) {
    // Make sure everything is destroyed in case the data related to
    // padded malloc isn't set properly
    PaddedMallocData.destroyGlobalsInfo(M);
    LLVM_DEBUG(dbgs() << "  dtrans-paddedmalloc: Padded malloc disabled\n");
    return false;
  }

  // Check if the module requires runtime safety checks
  SmallPtrSet<Function *, 16> Funcs;
  unsigned ArgumentIndex;
  unsigned StructIndex;
  if (DTInfo.requiresBadCastValidation(Funcs, ArgumentIndex, StructIndex)) {
    for (auto *Func : Funcs) {
      bool Ret = PaddedMallocData.buildFuncBadCastValidation(
          Func, ArgumentIndex, StructIndex);
      if (!Ret) {
        LLVM_DEBUG(dbgs() << "  dtrans-paddedmalloc: unable to construct "
                             "required runtime safety checks\n");
        PaddedMallocData.destroyGlobalsInfo(M);
        return false;
      }
    }
  }

  // Collect the functions that padded malloc can be applied
  std::vector<PaddedMallocFunc> PaddedMallocVect;
  if (!findFieldSingleValueFuncs(DTInfo, PaddedMallocVect)) {
    PaddedMallocData.destroyGlobalsInfo(M);
    return false;
  }

  // Look for search loops
  if (!findSearchLoops(M, GetLI)) {
    PaddedMallocData.destroyGlobalsInfo(M);
    return false;
  }

  GlobalVariable *GlobalCounter = PaddedMallocData.getPaddedMallocVariable(M);
  Function *PMFunc = PaddedMallocData.getPaddedMallocInterface(M);

  assert(GlobalCounter && "dtrans-paddedmalloc: Variable not generated");
  assert(PMFunc && "dtrans-paddedmalloc: Interface not generated");

  LLVM_DEBUG(dbgs() << "  dtrans-paddedmalloc: Global variable: "
                    << GlobalCounter->getName() << "\n");

  LLVM_DEBUG(dbgs() << "  dtrans-paddedmalloc: Interface function: "
                    << PMFunc->getName() << "\n");

  checkForParallelRegion(M, PaddedMallocVect);

  const TargetLibraryInfo &TLIInfo = GetTLI(*PMFunc);

  return applyPaddedMalloc(PaddedMallocVect, GlobalCounter, PMFunc, &M, TLIInfo,
                           DTInfo);
}

PreservedAnalyses dtrans::PaddedMallocPass::run(Module &M,
                                                ModuleAnalysisManager &AM) {

  auto &DTransInfo = AM.getResult<DTransAnalysis>(M);
  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);

  FunctionAnalysisManager &FAM =
      AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();

  LoopInfoFuncType GetLI = [&FAM](Function &F) -> LoopInfo & {
    return FAM.getResult<LoopAnalysis>(F);
  };

  auto GetTLI = [&FAM](Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(F);
  };

  if (!runImpl(M, DTransInfo, GetLI, GetTLI, WPInfo))
    return PreservedAnalyses::all();

  // TODO: Mark the actual preserved analyses.
  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  PA.preserve<DTransAnalysis>();
  return PA;
}

// Implementation of the PaddedMallocGlobals class

// Build the global variable and interface that will be used for
// padded malloc
void dtrans::PaddedMallocGlobals::buildGlobalsInfo(Module &M) {

  buildGlobalVariableCounter(M);
  buildInterfaceFunction(M);

  Function *PaddedMallocFunc = getPaddedMallocInterface(M);

  assert(PaddedMallocFunc && "dtrans-paddedmalloc: Interface not found");

  LLVMContext &Context = PaddedMallocFunc->getContext();

  // Build the integer
  IntegerType *Int32Ty = Type::getInt32Ty(M.getContext());
  ConstantInt *ConstInt =
      ConstantInt::get(Int32Ty, DTransPaddedMallocSize, false /*isSigned*/);

  // Create the node
  MDNode *Node = MDNode::get(Context, ConstantAsMetadata::get(ConstInt));

  // Set metadata
  PaddedMallocFunc->setMetadata("dtrans.paddedmallocsize", Node);
}

// Build the global variable related to padded malloc in the input Module and
// return it. This variable is a counter that identifies if the padded malloc
// will be used or not.
void dtrans::PaddedMallocGlobals::buildGlobalVariableCounter(Module &M) {

  GlobalVariable *PaddedMallocVar = M.getGlobalVariable(
      "__Intel_PaddedMallocCounter", true /*AllowInternal*/);

  // Check if the global variable was created already
  if (PaddedMallocVar)
    return;

  IRBuilder<> Builder(M.getContext());
  ConstantInt *initVal = Builder.getInt32(0);

  PaddedMallocVar = new GlobalVariable(M, Builder.getInt32Ty(), false,
                                       GlobalValue::InternalLinkage, initVal,
                                       "__Intel_PaddedMallocCounter", nullptr,
                                       GlobalValue::NotThreadLocal, 0, false);

  LLVM_DEBUG(
      assert(PaddedMallocVar && "dtrans-paddedmalloc: Variable not generated"));
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
void dtrans::PaddedMallocGlobals::buildInterfaceFunction(Module &M) {

  Function *PMFunction = M.getFunction("__Intel_PaddedMallocInterface");

  // Check if the global interface was created
  if (PMFunction)
    return;

  IRBuilder<> Builder(M.getContext());

  GlobalVariable *GlobalCounter = M.getGlobalVariable(
      "__Intel_PaddedMallocCounter", true /*AllowInternal*/);

  assert(GlobalCounter && "dtrans-paddedmalloc: Global counter not generated");

  // Create the function first
  FunctionType *funcType = FunctionType::get(Builder.getInt1Ty(), false);
  PMFunction = Function::Create(funcType, Function::ExternalLinkage,
                                "__Intel_PaddedMallocInterface", &M);

  // Create the entry BasicBlock
  BasicBlock *entry = BasicBlock::Create(M.getContext(), "entry", PMFunction);

  Builder.SetInsertPoint(entry);

  // Insert the conditional for testing
  Value *PMLimitVal = Builder.getInt32(getPaddedMallocLimit());
  LoadInst *load = Builder.CreateLoad(GlobalCounter);
  Value *Cmp = Builder.CreateICmpULT(load, PMLimitVal);

  Builder.CreateRet(Cmp);

  LLVM_DEBUG(
      assert(PMFunction && "dtrans-paddedmalloc: Interface not generated"));
}

// Build runtime validation for \p Func where an argument with index \p
// ArgumentIndex should be a structure and \p StructIndex specifies the field to
// validate against NULL.
//
// The method inserts two basic blocks to the beginning of the function:
//   bb1:
//     %2 = gep %struct, %struct* <Argument>, i64 0, i32 <StructIndex>
//     %3 = load i8*, i8** %2
//     %4 = icmp ne i8* %3, null
//     br i1 %4, label %bb2, label %bb3
//
//  bb2:
//    store i32 251, i32* @__Intel_PaddedMallocCounter
//    br label %6
//
//  bb3:
//    <original entry block>
bool dtrans::PaddedMallocGlobals::buildFuncBadCastValidation(
    Function *Func, unsigned ArgumentIndex, unsigned StructIndex) {
  LLVM_DEBUG(
      dbgs() << "dtrans-paddedmalloc: Building runtime safety check for Func: "
             << Func->getName() << "\n");

  // Validate function signature against ArgumentIndex and StructIndex.
  Argument *Arg = Func->arg_begin() + ArgumentIndex;
  Type *ArgType = Arg->getType();

  // Argument with ArgumentIndex should be a pointer...
  if (!ArgType->isPointerTy()) {
    LLVM_DEBUG(dbgs() << "dtrans-paddedmalloc: Arg not a pointer\n");
    return false;
  }

  // ... to a structure ...
  StructType *ArgSType = dyn_cast<StructType>(ArgType->getPointerElementType());
  if (!ArgSType) {
    LLVM_DEBUG(dbgs() << "dtrans-paddedmalloc: Arg not a struct\n");
    return false;
  }

  // ... where StructIndex field should be an i8*.
  Type *ElemType = ArgSType->getElementType(StructIndex);
  if (!ElemType->isPointerTy() ||
      !ElemType->getPointerElementType()->isIntegerTy(8)) {
    LLVM_DEBUG(dbgs() << "dtrans-paddedmalloc: Arg:StructIndex not an i8*\n");
    return false;
  }

  // Now build the runtime check.
  Type *OffsetType = Func->getParent()->getDataLayout().getIntPtrType(
      Func->getContext(), ArgType->getPointerAddressSpace());

  IRBuilder<> Builder(Func->getContext());

  BasicBlock *EntryBB = &Func->getEntryBlock();
  BasicBlock *CheckBB = BasicBlock::Create(Func->getContext());
  BasicBlock *SetBB = BasicBlock::Create(Func->getContext());

  Func->getBasicBlockList().push_front(SetBB);
  Func->getBasicBlockList().push_front(CheckBB);

  // Construct BB with a check
  Builder.SetInsertPoint(CheckBB);
  Value *GEP = Builder.CreateGEP(
      Arg, {ConstantInt::get(OffsetType, 0), Builder.getInt32(StructIndex)});
  Value *Load = Builder.CreateLoad(GEP);
  Value *IsNotNull = Builder.CreateIsNotNull(Load);
  Builder.CreateCondBr(IsNotNull, SetBB, EntryBB);

  // Construct BB where PaddedMallocVariable is set to the threshold+1 value.
  Builder.SetInsertPoint(SetBB);
  auto *CounterPtr = getPaddedMallocVariable(*Func->getParent());
  assert(CounterPtr && "No global counter is present in module");
  Value *PMLimitVal = Builder.getInt32(getPaddedMallocLimit() + 1);
  Builder.CreateStore(PMLimitVal, CounterPtr);
  Builder.CreateBr(EntryBB);

  assert(std::find(BadCastValidatedFuncs.begin(), BadCastValidatedFuncs.end(),
                   Func) == BadCastValidatedFuncs.end() &&
         "Found function duplicate");

  BadCastValidatedFuncs.push_back(Func);
  return true;
}

// Remove from the IR the global counter and interface that was generated for
// padded malloc.
void dtrans::PaddedMallocGlobals::destroyGlobalsInfo(Module &M) {

  Function *PMFunc = getPaddedMallocInterface(M);
  GlobalVariable *GlobalCounter = getPaddedMallocVariable(M);

  if (PMFunc)
    PMFunc->eraseFromParent();

  if (GlobalCounter)
    GlobalCounter->eraseFromParent();

  for (auto *F : BadCastValidatedFuncs) {
    F->getBasicBlockList().pop_front();
    F->getBasicBlockList().pop_front();
  }
  BadCastValidatedFuncs.clear();
}

// Common function to be used both by the PaddedMallocGlobals class and
// by external function PaddedMallocIsActive().
static Function *getPaddedMallocInterface(Module &M) {
  return M.getFunction("__Intel_PaddedMallocInterface");
}

extern bool PaddedMallocIsActive(Module &M) {
  return getPaddedMallocInterface(M);
}

// Return the interface generated by padded malloc optimization
Function *dtrans::PaddedMallocGlobals::getPaddedMallocInterface(Module &M) {

  Function *PaddedMallocInterface = ::getPaddedMallocInterface(M);

  return PaddedMallocInterface;
}

// Return the global variable generated by padded malloc optimization
GlobalVariable *
dtrans::PaddedMallocGlobals::getPaddedMallocVariable(Module &M) {

  GlobalVariable *PaddedMallocVariable = M.getGlobalVariable(
      "__Intel_PaddedMallocCounter", true /*AllowInternal*/);

  return PaddedMallocVariable;
}

// Return the size used in the padded malloc optimizaton
unsigned dtrans::PaddedMallocGlobals::getPaddedMallocSize(Module &M) {

  Function *PaddedMallocFunc = M.getFunction("__Intel_PaddedMallocInterface");

  if (!PaddedMallocFunc)
    return 0;

  if (!PaddedMallocFunc->hasMetadata("dtrans.paddedmallocsize"))
    return 0;

  // Collect the node related to the metadata
  MDNode *Node = PaddedMallocFunc->getMetadata("dtrans.paddedmallocsize");

  assert(Node && "Padded malloc metadata found but not accessible?");

  // Get the metadata
  ConstantAsMetadata *ConstMetaData =
      cast<ConstantAsMetadata>(Node->getOperand(0));

  // Collect the value stored
  unsigned PaddedMallocSize =
      cast<ConstantInt>(ConstMetaData->getValue())->getZExtValue();

  return PaddedMallocSize;
}

// Return true if all the data for padded malloc is set correctly.
bool dtrans::PaddedMallocGlobals::isPaddedMallocDataAvailable(Module &M) {
  return (getPaddedMallocSize(M) > 0 && getPaddedMallocVariable(M) &&
          getPaddedMallocInterface(M));
}
