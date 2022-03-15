//===- CoerceWin64Types.cpp - Coerce types to ensure win64 ABI compliance -===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/CoerceWin64Types.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"
#include <utility>

#define DEBUG_TYPE "dpcpp-kernel-coerce-win64-types"

using namespace llvm;
using namespace DPCPPKernelMetadataAPI;
using namespace DPCPPKernelCompilationUtils;

namespace {
/// Legacy CoerceWin64Types pass.
class CoerceWin64TypesLegacy : public ModulePass {
public:
  static char ID;

  CoerceWin64TypesLegacy();

  StringRef getPassName() const override { return "CoerceWin64TypesLegacy"; }

  bool runOnModule(Module &M) override;

private:
  CoerceWin64TypesPass Impl;
};
} // namespace

char CoerceWin64TypesLegacy::ID = 0;

INITIALIZE_PASS(CoerceWin64TypesLegacy, DEBUG_TYPE,
                "Performs function argument and return value type coercion to "
                "ensure windows-64 ABI compliance",
                false, false)

CoerceWin64TypesLegacy::CoerceWin64TypesLegacy() : ModulePass(ID), Impl() {
  initializeCoerceWin64TypesLegacyPass(*PassRegistry::getPassRegistry());
}

bool CoerceWin64TypesLegacy::runOnModule(Module &M) { return Impl.runImpl(M); }

ModulePass *llvm::createCoerceWin64TypesLegacyPass() {
  return new CoerceWin64TypesLegacy();
}

CoerceWin64TypesPass::CoerceWin64TypesPass() {}

PreservedAnalyses CoerceWin64TypesPass::run(Module &M,
                                            ModuleAnalysisManager &) {
  if (!runImpl(M))
    return PreservedAnalyses::all();
  return PreservedAnalyses::none();
}

// Aggregates of size 1, 2, 4, 8 bytes are passed as if they were integers of
// the same size. Aggregate of other sizes are passed as a pointer to memory
// allocated by the caller.
static bool shouldPassByval(uint64_t Bytes) {
  if (Bytes == 1 || Bytes == 2 || Bytes == 4 || Bytes == 8)
    return true;
  return false;
}

static Type *getBitCastType(uint64_t Bytes, LLVMContext &C) {
  Type *NewType;
  switch (Bytes) {
  case 1:
    NewType = Type::getInt8Ty(C);
    break;
  case 2:
    NewType = Type::getInt16Ty(C);
    break;
  case 4:
    NewType = Type::getInt32Ty(C);
    break;
  case 8:
    NewType = Type::getInt64Ty(C);
    break;
  default:
    llvm_unreachable("Unhandled argument type!");
  }
  return NewType;
}

static bool isFunctionSupported(Function &F) {
  // Leave functions with users that are not supported yet (function pointer
  // related) unchanged
  for (User *U : F.users()) {
    if (!isa<CallInst>(U))
      return false;
  }
  return true;
}

// This is actually like Function::copyAttributesFrom but will remove
// all the attributes of 'byval' argument.
static void copyFunctionAttrWithoutByval(const Function *OldF, Function *NewF,
                                         LLVMContext &C) {
  NewF->setVisibility(OldF->getVisibility());
  NewF->setUnnamedAddr(OldF->getUnnamedAddr());
  NewF->setThreadLocalMode(OldF->getThreadLocalMode());
  NewF->setDLLStorageClass(OldF->getDLLStorageClass());
  NewF->setDSOLocal(OldF->isDSOLocal());
  NewF->setPartition(OldF->getPartition());
  NewF->setAlignment(OldF->getAlign());
  NewF->setSection(OldF->getSection());
  NewF->setCallingConv(OldF->getCallingConv());

  AttributeList AttrList = OldF->getAttributes();
  // Copy all of the argument attributes except 'byval' argument.
  for (unsigned I = 0; I < OldF->arg_size(); ++I)
    if (AttrList.hasParamAttr(I, Attribute::ByVal))
      AttrList = AttrList.removeParamAttributes(C, I);

  NewF->setAttributes(AttrList);

  if (OldF->hasGC())
    NewF->setGC(OldF->getGC());
  else
    NewF->clearGC();
  if (OldF->hasPersonalityFn())
    NewF->setPersonalityFn(OldF->getPersonalityFn());
  if (OldF->hasPrefixData())
    NewF->setPrefixData(OldF->getPrefixData());
  if (OldF->hasPrologueData())
    NewF->setPrologueData(OldF->getPrologueData());
}

static void removeCallArgsAttr(const CallInst *OldCI, CallInst *NewCI,
                               LLVMContext &C) {
  if (OldCI->hasMetadata())
    NewCI->setDebugLoc(OldCI->getDebugLoc());
  NewCI->setCallingConv(OldCI->getCallingConv());
  // Remove all the attributes of 'byval' argument. Including argument alignment
  // and byval attribute.
  AttributeList AttrList = OldCI->getAttributes();
  for (unsigned I = 0; I < OldCI->arg_size(); ++I) {
    if (OldCI->paramHasAttr(I, Attribute::ByVal))
      AttrList = AttrList.removeParamAttributes(C, I);
  }
  NewCI->setAttributes(AttrList);
}

static Value *CreateAllocaInst(Type *Ty, Function *F, unsigned Alignment,
                               unsigned AS) {
  Module *M = F->getParent();
  const DataLayout &DL = M->getDataLayout();
  unsigned AllocaAS = DL.getAllocaAddrSpace();
  IRBuilder<> Builder(&F->getEntryBlock().front());
  AllocaInst *AllocaRes = Builder.CreateAlloca(Ty, AllocaAS);
  // If the alignment is defined, set it.
  if (Alignment)
    AllocaRes->setAlignment(Align(Alignment));
  if (AS != AllocaAS)
    return Builder.CreateAddrSpaceCast(AllocaRes, PointerType::get(Ty, AS));
  return AllocaRes;
}

// For the aggregate of size 1, 2, 4, 8 bytes we pass the value
// directly:
// Before:
// %struct.A = type { i32 }
//
// call void @foo(%struct.A* byval(%struct.A) align 4 %arg1)
//
// After:
// %1 = bitcast %struct.A* %arg1 to i32*
// %arg2 = load i32, i32* %1, align 4
// call void @foo(i32 align 4 %arg2)
//
// For the aggregate of another size, we memcopy the argument to a
// temporary stack, and pass the pointer which point to the stack.
// Before:
//
//%struct.D = type { i32, i64 }
//
// %2 = call i32 @foo(%struct.D* byval(%struct.D) align 8 %arg1)
//
// After:
//   %6 = alloca %struct.D, align 8
//   %7 = getelementptr inbounds %struct.D, %struct.D* %6, i32 0
//   %8 = bitcast %struct.D* %7 to i8*
//   %9 = bitcast %struct.D* %arg1 to i8*
//   call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %8, i8* align 8 %9, i64
//   12, i1 false) %10 = call i32 @foo(%struct.D* align 8 %6)
static void
updateCallInst(CallInst *CI, Function *NewFun,
               DenseMap<unsigned, std::pair<unsigned, uint64_t>> &ValueMap,
               LLVMContext &C) {
  IRBuilder<> Builder(CI);
  SmallVector<Value *, 16> Args;
  for (unsigned I = 0; I < CI->arg_size(); ++I) {
    Value *ArgI = CI->getArgOperand(I);
    if (CI->paramHasAttr(I, Attribute::ByVal)) {
      auto *PT = cast<PointerType>(ArgI->getType());
      Type *ElementTy = CI->getParamByValType(I);
      // In case that FE doesn't passing the size infomation.
      unsigned Alignment = ValueMap[I].first;
      uint64_t MemSize = ValueMap[I].second;
      if (shouldPassByval(MemSize)) {
        Value *BI = Builder.CreateBitCast(
            ArgI, PointerType::get(NewFun->getArg(I)->getType(),
                                   PT->getAddressSpace()));
        LoadInst *LI = Builder.CreateAlignedLoad(NewFun->getArg(I)->getType(),
                                                 BI, MaybeAlign(Alignment));
        Args.push_back(LI);
      } else {
        Value *Alloca = CreateAllocaInst(ElementTy, CI->getFunction(),
                                         Alignment, PT->getAddressSpace());
        Value *DstPtr =
            Builder.CreateInBoundsGEP(ElementTy, Alloca, Builder.getInt32(0));
        Builder.CreateMemCpy(DstPtr, MaybeAlign(Alignment), ArgI,
                             MaybeAlign(Alignment), MemSize);
        Args.push_back(Alloca);
      }
    } else
      Args.push_back(ArgI);
  }
  CallInst *NewCI = Builder.CreateCall(NewFun, Args);
  removeCallArgsAttr(CI, NewCI, C);
  CI->replaceAllUsesWith(NewCI);
  CI->eraseFromParent();
}

static void
moveFunctionBody(Function *OldF, Function *NewF,
                 DenseMap<unsigned, std::pair<unsigned, uint64_t>> &ValueMap) {
  // Splice the body of the old function into the new one
  NewF->getBasicBlockList().splice(NewF->begin(), OldF->getBasicBlockList());

  // Delete original function body - this is needed to remove linkage (if
  // exists)
  OldF->deleteBody();

  // Loop over the original arguments, replace the use of each with the values
  // obtained from coerced ones.
  Function::arg_iterator NewArgI = NewF->arg_begin();
  IRBuilder<> Builder(&*NewF->getEntryBlock().begin());
  for (Argument &OldArg : OldF->args()) {
    // Change the argumrnt name.
    NewArgI->setName(OldArg.getName());
    // If the argument doesn't 'byval' attribute, keep it.
    if (!OldArg.hasByValAttr()) {
      OldArg.replaceAllUsesWith(&*NewArgI);
      ++NewArgI;
      continue;
    }

    uint64_t MemSize = ValueMap[NewArgI->getArgNo()].second;
    if (shouldPassByval(MemSize)) {
      // For the aggregates of size 1, 2, 4, 8 bytes, allocating the original
      // type, store values from the coerced arguments, replace uses of old
      // argument with the allocated value:
      // Before:
      //  %struct.A = type { float }
      //  define dso_local void @foo(%struct.A* byval(%struct.A) align 4 %0)
      // After:
      //  define dso_local void @foo(i32 align 4 %0)
      //   %2 = alloca %struct.A, align 4
      //   %3 = bitcast %struct.A* %2 to i32*
      //   store i32 %0, i32* %4, align 4
      unsigned Alignment = ValueMap[NewArgI->getArgNo()].first;
      Type *NewArgT = NewArgI->getType();
      auto *OldArgPT = cast<PointerType>(OldArg.getType());
      Type *OldArgByvalType = OldArg.getParamByValType();
      assert(OldArgByvalType != NULL && "The parameter must have 'byval' type");
      Value *Alloca = CreateAllocaInst(OldArgByvalType, NewF, Alignment,
                                       OldArgPT->getAddressSpace());

      Value *BC = Builder.CreateBitCast(
          Alloca, PointerType::get(NewArgT, OldArgPT->getAddressSpace()));
      Builder.CreateAlignedStore(&*NewArgI, BC, MaybeAlign(Alignment));
      OldArg.replaceAllUsesWith(Alloca);
    } else
      // For the aggregates of other size, nothing to do, simply replace the
      // use of old argument with the new argument:
      // Before:
      //  %struct.A = type { float, i64 }
      //  define dso_local spir_func i32 @foo(%struct.A* byval(%struct.A) align
      //  8 %0){
      //    %2 = getelementptr inbounds %struct.A, %struct.A* %0, i32 0, i32 0
      // After:
      //  define dso_local spir_func i32 @foo(%struct.A* align 8 %0) {
      //    %2 = getelementptr inbounds %struct.D, %struct.D* %0, i32 0, i32 0
      OldArg.replaceAllUsesWith(NewArgI);

    ++NewArgI;
  }
}
bool CoerceWin64TypesPass::runImpl(Module &M) {
  bool Changed = false;

  // Leave kernel signatures intact
  KernelList KL(&M);
  SmallPtrSet<Function *, 16> Kernels(KL.begin(), KL.end());

  // Store the original functions since some of them will be replaced with new
  // ones
  std::vector<Function *> FuncsToHandle;
  for (auto &Func : M) {
    if (!Func.isIntrinsic() && isFunctionSupported(Func) &&
        !Kernels.count(&Func))
      FuncsToHandle.push_back(&Func);
  }

  for (auto *Func : FuncsToHandle)
    Changed |= runOnFunction(Func);

  updateFunctionMetadata(&M, FunctionMap);

  return Changed;
}

bool CoerceWin64TypesPass::runOnFunction(Function *F) {
  bool Change = false;
  SmallVector<Type *, 16> NewArgTypes;
  LLVMContext &C = F->getContext();
  const DataLayout &DL = F->getParent()->getDataLayout();

  // Using a map to record the alignment and size of arguments.
  DenseMap<unsigned, std::pair<unsigned, uint64_t>> ValueMap;
  // Handle only 'ByVal' argument.
  for (Argument &Arg : F->args()) {
    if (Arg.hasByValAttr()) {
      Change = true;
      Type *ArgMemTy = Arg.getParamByValType();
      uint64_t MemSize = 0;
      if (auto StructTy = dyn_cast<StructType>(ArgMemTy)) {
        auto OldStructT = cast<StructType>(ArgMemTy);
        MemSize = DL.getStructLayout(OldStructT)->getSizeInBytes();
      } else
        MemSize = DL.getTypeAllocSize(ArgMemTy);
      assert(MemSize != 0 && "Invalid memory size for byval type!");
      ValueMap[Arg.getArgNo()] = {Arg.getParamAlignment(), MemSize};
      if (shouldPassByval(MemSize))
        NewArgTypes.push_back(getBitCastType(MemSize, C));
      else
        NewArgTypes.push_back(Arg.getType());
    } else
      NewArgTypes.push_back(Arg.getType());
  }

  if (!Change)
    return false;

  // Replace function with a new one
  // TODO: coerce return value type as well
  FunctionType *FuncType =
    FunctionType::get(F->getReturnType(), NewArgTypes, F->isVarArg());
  std::string Name = F->getName().str();
  F->setName("__" + F->getName() + "_before.CoerceWin64Types");
  Function *NewF =
    Function::Create(FuncType, F->getLinkage(), Name, F->getParent());

  FunctionMap[F] = NewF;
  NewF->copyMetadata(F, 0);
  copyFunctionAttrWithoutByval(F, NewF, C);
  NewF->setSubprogram(F->getSubprogram());
  NewF->setComdat(F->getComdat());
  if (!F->isDeclaration()) {
    moveFunctionBody(F, NewF, ValueMap);
    // F becomes a declaration and it should not contain Comdat.
    F->setComdat(nullptr);
  }

  // Patch users of the old function
  std::vector<CallInst *> FuncUsers;
  for (auto UI = F->user_begin(), UE = F->user_end(); UI != UE; UI++) {
    if (CallInst *CI = dyn_cast<CallInst>(*UI))
      FuncUsers.push_back(CI);
    else
      llvm_unreachable("Unhandled function reference");
  }
  // replace call instruction
  for (auto User : FuncUsers)
    updateCallInst(User, NewF, ValueMap, C);
  F->eraseFromParent();

  return Change;
}
