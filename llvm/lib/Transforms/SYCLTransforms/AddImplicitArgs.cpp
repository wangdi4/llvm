//===- AddImplicitArgs.cpp - Add implicit arguments to DPC++ kernel  ------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/SYCLTransforms/AddImplicitArgs.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/ValueMap.h"
#include "llvm/Transforms/SYCLTransforms/DevLimits.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/ImplicitArgsUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"

using namespace llvm;

#define DEBUG_TYPE "sycl-kernel-add-implicit-args"

namespace {
class FuncTypeRemapper : public ValueMapTypeRemapper {
  DenseMap<Type *, Type *> MappedTypes;

public:
  void addMapping(Type *SrcTy, Type *DstTy) { MappedTypes[SrcTy] = DstTy; }

  Type *remapType(Type *SrcTy) override {
    auto It = MappedTypes.find(SrcTy);
    return It != MappedTypes.end() ? It->second : SrcTy;
  }

  /// Recursively add type mapping.
  Type *get(Type *Ty, SmallPtrSetImpl<Type *> &Visited) {
    if (Visited.contains(Ty))
      return remapType(Ty);
    Visited.insert(Ty);

    if (auto It = MappedTypes.find(Ty); It != MappedTypes.end())
      return It->second;

    if (isa<PointerType>(Ty))
      return Ty;

    bool Changed = false;
    SmallVector<Type *, 8> NewEltTypes;
    for (auto *SubTy : Ty->subtypes()) {
      Type *MappedTy = get(SubTy, Visited);
      Changed |= MappedTy != Ty;
      NewEltTypes.push_back(MappedTy);
    }
    if (!Changed)
      return Ty;

    Type *NewTy;
    switch (Ty->getTypeID()) {
    case Type::ArrayTyID:
      NewTy = ArrayType::get(NewEltTypes[0], Ty->getArrayNumElements());
      break;
    case Type::FixedVectorTyID:
    case Type::ScalableVectorTyID:
      NewTy = VectorType::get(NewEltTypes[0],
                              cast<VectorType>(Ty)->getElementCount());
      break;
    case Type::StructTyID:
      if (cast<StructType>(Ty)->isLiteral())
        NewTy = StructType::get(Ty->getContext(), NewEltTypes,
                                cast<StructType>(Ty)->isPacked());
      else
        NewTy = StructType::create(NewEltTypes, Ty->getStructName(),
                                   cast<StructType>(Ty)->isPacked());
      break;
    default:
      llvm_unreachable("Unhandled type");
    }

    addMapping(Ty, NewTy);
    return NewTy;
  }
};
} // namespace

PreservedAnalyses AddImplicitArgsPass::run(Module &M,
                                           ModuleAnalysisManager &AM) {
  CallGraph *CG = &AM.getResult<CallGraphAnalysis>(M);
  ImplicitArgsInfo *IAInfo = &AM.getResult<ImplicitArgsAnalysis>(M);
  if (!runImpl(M, IAInfo, CG))
    return PreservedAnalyses::all();
  PreservedAnalyses PA;
  PA.preserve<ImplicitArgsAnalysis>();
  return PA;
}

bool AddImplicitArgsPass::runImpl(Module &M, ImplicitArgsInfo *IAInfo,
                                  CallGraph *CG) {
  this->IAInfo = IAInfo;
  this->CG = CG;

  // Clear call instruction to fix container
  FixupCalls.clear();
  // Clear functions refs to fix container
  FixupFunctionsRefs.clear();

  // Collect all module functions that are not declarations into work list.
  SmallVector<Function *, 4> WorkList;
  for (auto &F : M) {
    if (F.isDeclaration()) {
      // Function is not defined inside module.
      continue;
    }

    // global ctor's and dtor's don't need implicit arguments, as it only
    // called once by runStaticConstructorsDestructors function.
    // C++ function does not need implicit arguments.
    if (CompilationUtils::isGlobalCtorDtorOrCPPFunc(&F))
      continue;

    WorkList.push_back(&F);
  }

  // Run on all collected functions.
  for (auto *F : WorkList)
    runOnFunction(F);

  // update Metadata now.
  CompilationUtils::updateFunctionMetadata(&M, FixupFunctionsRefs);

  // Indirect calls are not users of any functions. We need to collect them
  // and add stubs for implicit arguments here
  SmallPtrSet<CallInst *, 16> IndirectCalls;
  for (const auto &It : FixupCalls) {
    CallInst *CI = It.first;
    if (!CI->getCalledFunction())
      IndirectCalls.insert(CI);
  }

  for (auto *CI : IndirectCalls) {
    Value **Args = FixupCalls[CI];
    SmallVector<Type *, 16> ArgTys;
    for (unsigned I = 0; I < ImplicitArgsUtils::NUM_IMPLICIT_ARGS; ++I)
      ArgTys.push_back(Args[I]->getType());

    replaceCallInst(CI, ArgTys, nullptr);
  }

  // Go over all call instructions that need to be changed and add implicit
  // arguments to them.
  // Call instructions already contain extra agruments, but all of them are
  // Undef's - we need to replace Undef's with the correct values.
  for (auto &It : FixupCalls) {
    CallInst *CI = It.first;
    Value **CallArgs = It.second;

    unsigned ImplicitArgStart =
        CI->arg_size() - ImplicitArgsUtils::NUM_IMPLICIT_ARGS;
    for (unsigned i = ImplicitArgStart, j = 0;
         j < ImplicitArgsUtils::NUM_IMPLICIT_ARGS; ++i, ++j)
      CI->setArgOperand(i, CallArgs[j]);

    delete[] CallArgs;
  }

  return !WorkList.empty();
}

void AddImplicitArgsPass::runOnFunction(Function *F) {
  SmallVector<Type *, 16> NewTypes;
  SmallVector<const char *, 16> NewNames;
  SmallVector<AttributeSet, 16> NewAttrs;
  unsigned NumExplicitArgs = F->arg_size();

  AttrBuilder B(F->getContext());
  B.addAttribute(Attribute::NoAlias);
  AttributeSet NoAlias = AttributeSet::get(F->getContext(), B);
  AttributeSet NoAttr;
  // For each implicit arg, setup its type, name and attributes.
  for (unsigned I = 0; I < ImplicitArgsUtils::NUM_IMPLICIT_ARGS; ++I) {
    Type *ArgType = IAInfo->getArgType(I);
    NewTypes.push_back(ArgType);
    NewNames.push_back(ImplicitArgsUtils::getArgName(I));
    // We know that all implicit args that are pointers are non-aliasing, but
    // we must be careful are review this assumption every time we add a new
    // implicit arg.
    if (ArgType->isPointerTy())
      NewAttrs.push_back(NoAlias);
    else
      NewAttrs.push_back(NoAttr);
  }
  // Create the new function with appended implicit attributes.
  Function *NewF = CompilationUtils::AddMoreArgsToFunc(
      F, NewTypes, NewNames, NewAttrs, "AddImplicitArgs");

  // maintain this map to preserve original/modified relation for functions.
  FixupFunctionsRefs[F] = NewF;

  // Apple LLVM-IR workaround
  // 1.  Pass WI information structure as the next parameter after given
  // function parameters
  // 2.  We don't want to use TLS for local memory.
  //    Our solution to move all internal local memory blocks to be allocated
  //    by the execution engine and passed within additional parameters to the
  //    kernel, those parameters are not exposed to the user.

  // Go through new function instructions and search calls.
  CG->addToCallGraph(NewF);
  for (auto &N : *(*CG)[NewF]) {
    auto *CI = cast<CallInst>(*N.first);
    // Ignore calls of inline assembly code.
    if (CI->isInlineAsm())
      continue;

    // Check call for not inlined module function.
    Function *Callee = CI->getCalledFunction();
    bool IsDirectCall = (Callee != nullptr);
    // C++ Callee will be skipped as C++ func does not add implicit arguments.
    // TODO: handle C++ function as indirect callee.
    if (IsDirectCall && (Callee->isDeclaration() ||
                         CompilationUtils::isGlobalCtorDtorOrCPPFunc(Callee)))
      continue;
    Value **CallArgs = new Value *[ImplicitArgsUtils::NUM_IMPLICIT_ARGS];
    Function::arg_iterator IA = NewF->arg_begin();
    // Skip over explicit args.
    for (unsigned I = 0; I < NumExplicitArgs; ++I, ++IA)
      ;
    // Copy over implicit args.
    for (unsigned I = 0; I < ImplicitArgsUtils::NUM_IMPLICIT_ARGS; ++I, ++IA)
      CallArgs[I] = static_cast<Value *>(&*IA);
    assert(IA == NewF->arg_end());

    FixupCalls[CI] = CallArgs;
  }

  // All users which need to be replaced are handled here.
  SmallVector<User *, 16> Users(F->users());
  for (User *U : Users)
    if (auto *CI = dyn_cast<CallInst>(U); CI && CI->getCalledOperand() == F)
      replaceCallInst(CI, NewTypes, NewF);

  FuncTypeRemapper TypeMapper;
  ValueToValueMapTy VMap;
  VMap[F] = NewF;
  ValueMapper VMapper(VMap, RF_NoModuleLevelChanges | RF_IgnoreMissingLocals,
                      &TypeMapper);

  for (User *U : make_early_inc_range(F->users())) {
    if (auto *I = dyn_cast<Instruction>(U)) {
      VMapper.remapInstruction(*I);
    } else if (auto *GV = dyn_cast<GlobalVariable>(U)) {
      GV->setInitializer(VMapper.mapConstant(*GV->getInitializer()));
    } else if (auto *C = dyn_cast<Constant>(U)) {
      Constant *NewC = VMapper.mapConstant(*C);
      C->replaceAllUsesWith(NewC);
    } else {
      llvm_unreachable("unhandled kernel user");
    }
  }
}

void AddImplicitArgsPass::replaceCallInst(CallInst *CI,
                                          ArrayRef<Type *> ImplicitArgsTypes,
                                          Function *NewF) {
  bool FixupCallsNeedsUpdate = FixupCalls.count(CI) > 0;
  Value **CallArgs = nullptr;
  if (FixupCallsNeedsUpdate) {
    // Remove the old call from the mapping.
    CallArgs = FixupCalls[CI];
    FixupCalls.erase(CI);
  }
  SmallVector<Value *, 16> NewArgs;
  // Push undefs for new arguments.
  assert(ImplicitArgsTypes.size() == ImplicitArgsUtils::NUM_IMPLICIT_ARGS);
  for (unsigned I = 0; I < ImplicitArgsUtils::NUM_IMPLICIT_ARGS; ++I) {
    NewArgs.push_back(UndefValue::get(ImplicitArgsTypes[I]));
  }
  CallInst *NewCI =
      NewF ? CompilationUtils::AddMoreArgsToCall(CI, NewArgs, NewF)
           : CompilationUtils::addMoreArgsToIndirectCall(CI, NewArgs);

  // Place the new call into the mapping.
  if (FixupCallsNeedsUpdate)
    FixupCalls[NewCI] = CallArgs;
}
