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
  for (User *U : Users) {
    if (auto *CI = dyn_cast<CallInst>(U)) {
      if (CI->getCalledOperand() == F) {
        replaceCallInst(CI, NewTypes, NewF);
      } else if (F->getContext().supportsTypedPointers()) {
        // The function is used as an argument.
        auto *Cast = CastInst::CreatePointerCast(NewF, F->getType(), "", CI);
        Cast->setDebugLoc(CI->getDebugLoc());
        CI->replaceUsesOfWith(F, Cast);
#ifndef NDEBUG
        // FIXME:
        // The only case that a function ptr is passed as an argument is in
        // task_sequence, and in such case, we assume the passed function won't
        // be called directly inside the task_sequence, so we don't do extra
        // work except bitcast'ing the argument's pointer type. We add an
        // assert here to assure it won't be called. If the passed function ptr
        // is called inside the function in the future, we need to fix call
        // instructions in the function.
        auto *CalledF = dyn_cast<Function>(CI->getCalledOperand());
        if (!CalledF)
          continue;
        for (int i = 0, e = CI->arg_size(); i < e; ++i) {
          if (CI->getArgOperand(i) != Cast)
            continue;
          assert(llvm::all_of(CalledF->getArg(i)->users(),
                              [](User *U) { return !isa<CallBase>(U); }) &&
                 "Calling a function pointer from argument isn't implemented.");
        }
#endif
      }
      continue;
    }

    if (!F->getContext().supportsTypedPointers())
      continue;

    if (auto *SI = dyn_cast<StoreInst>(U)) {
      // This function was stored as a function pointer, but the type of
      // function was changed (implicit args were added) - to avoid changing
      // types let's just cast new function type into the old one before
      // storing.
      Type *OldFPtrTy = SI->getValueOperand()->getType();

      auto *Cast = CastInst::CreatePointerCast(NewF, OldFPtrTy, "", SI);
      auto *NewSI = new StoreInst(Cast, SI->getPointerOperand(), SI);
      NewSI->setDebugLoc(SI->getDebugLoc());
      SI->replaceAllUsesWith(NewSI);
      SI->eraseFromParent();
    }
  }

  FuncTypeRemapper TypeMapper;
  ValueToValueMapTy VMap;
  VMap[F] = NewF;
  ValueMapper VMapper(VMap, RF_NoModuleLevelChanges | RF_IgnoreMissingLocals,
                      &TypeMapper);

  if (F->getContext().supportsTypedPointers()) {
    SmallPtrSet<Type *, 4> Visited{F->getType()};
    TypeMapper.addMapping(F->getType(), NewF->getType());

    // It seems that removing function use (by changing its operand to another
    // function) somehow breaks data structure used to hold uses and for example
    // for two uses, the loop stops after the first one.
    // Let's store info which need to be updated and perform updates outside
    // of the loop over function uses.
    DenseMap<User *, std::pair<unsigned, Value *>> UsersToReplace;

    // All users which are not to be replaced are handled here.
    for (Use &U : F->uses()) {
      User *Usr = U.getUser();
      if (auto *SI = dyn_cast<SelectInst>(Usr)) {
        unsigned OpNo = U.getOperandNo();
        // This function goes though a select instruction, but the type of the
        // function was changed (implicit args were added) - to avoid changing
        // types let's just cast new function type into the old one before
        // select.
        if (SI->getOperand(OpNo)->getType() != NewF->getType()) {
          auto *Cast = CastInst::CreatePointerCast(
              NewF, SI->getOperand(OpNo)->getType(), "", SI);
          UsersToReplace[SI] = {OpNo, Cast};
        } else {
          UsersToReplace[SI] = {OpNo, NewF};
        }
      } else if (auto *C = dyn_cast<Constant>(Usr)) {
        std::ignore = TypeMapper.get(C->getType(), Visited);
      }
    }

    for (const auto &I : UsersToReplace) {
      const std::pair<unsigned, Value *> &R = I.second;
      I.first->setOperand(R.first, R.second);
    }
  }

  for (User *U : make_early_inc_range(F->users())) {
    if (auto *I = dyn_cast<Instruction>(U)) {
      VMapper.remapInstruction(*I);
    } else if (auto *C = dyn_cast<Constant>(U)) {
      Constant *NewC = nullptr;
      if (F->getContext().supportsTypedPointers() &&
          isa<ConstantAggregate>(C)) {
        SmallVector<Constant *, 8> Data;
        for (auto *Op : C->operand_values()) {
          Constant *OpC = cast<Constant>(Op);
          Data.push_back(ConstantExpr::getBitCast(VMapper.mapConstant(*OpC),
                                                  OpC->getType()));
        }
        if (auto *CA = dyn_cast<ConstantArray>(C))
          NewC = ConstantArray::get(CA->getType(), Data);
        else if (auto *CS = dyn_cast<ConstantStruct>(C))
          NewC = ConstantStruct::get(CS->getType(), Data);
        else
          NewC = ConstantVector::get(Data);
      } else {
        NewC = VMapper.mapConstant(*C);
      }
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
