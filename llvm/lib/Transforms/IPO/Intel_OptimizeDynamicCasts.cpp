//===--- Intel_OptimizeDynamicCasts.cpp - Optimize dynamic_cast calls. ----===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements dynamic casts optimization pass.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/IPO/Intel_OptimizeDynamicCasts.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/TargetParser/Triple.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO.h"

using namespace llvm;
using namespace PatternMatch;

#define DEBUG_TYPE "optimize-dyn-casts"

STATISTIC(OptimizedCounter, "Count of dynamic_cast calls optimized");

static bool isTypeInfoGlobalForFinalClass(GlobalVariable *TypeInfoGlobal) {

  auto TestForGlobal = [](User *U) -> bool {
    auto TI = dyn_cast<GlobalVariable>(U);
    if (!TI) {
      LLVM_DEBUG(dbgs()
                 << "Expected virtual table but found something else: "
                 << *U << "\n");
      LLVM_DEBUG(dbgs() << "So class could be NOT FINAL."
                        << "\n\n");
      return false;
    }
    if (TI->hasMetadata() &&
        TI->getMetadata(llvm::LLVMContext::MD_type)) {
      // This is a virtual table, we can skip it.
      return true;
    }
    // We don't know what is it.
    LLVM_DEBUG(dbgs() << "Found unexpected user: " << *U << "\n");
    LLVM_DEBUG(dbgs() << "So class could be NOT FINAL."
                      << "\n\n");
    return false;
  };

  assert(TypeInfoGlobal && "Expected non-null pointer to type_info!");

  LLVM_DEBUG(dbgs() << "Analysis of type_info:"
                    << "  " << TypeInfoGlobal->getName() << "\n");
  // Even when whole program is detected there could be library classes from
  // standard header files with descendants in some other parts of library.
  // In this case all information will be available for the linker, so we
  // could rely on the linkage type of a type_info global (because there are
  // no things like dlopen in the standard library which could break the
  // ABI).  The linkage type will be identified by internalization pass
  // using information from the linker when the LTO is enabled. If  the
  // linkage type is not internal then we could not say that a class is
  // final.
  if (!TypeInfoGlobal->hasInternalLinkage()) {
    LLVM_DEBUG(dbgs() << "Has not internal linkage. Could be NOT FINAL.\n\n");
    return false;
  }

  // Analyze users of TypeInfoGlobal. We are not interested in users like
  // dynamic_cast calls and other instructions. We are interested in other
  // globals that use TypeInfoGlobal (maybe through the chain of
  // GEPs). We expect that there are two kind of such globals: vtables or
  // other type_info objects. If we find something else then we cannot say
  // that class is final.
  //
  // Example of vtable initializer:
  // @_ZTV8Derived2 = internal dso_local unnamed_addr constant { [3 x i8*] }
  // { [3 x ptr] [ptr null, ptr @_ZTI8Derived2, ptr @_ZN8Derived21kEv]
  // }, comdat, align 8, !type !7, !type !8
  for (auto U1 : TypeInfoGlobal->users()) {
    for (auto U2 : U1->users()) {
      // We can skip instructions that use type_info (ex, dynamic_cast calls)
      // because we are interested in constant initializers of global variables.
      if (isa<Instruction>(U2))
        continue;

      if (!isa<Constant>(U2)) {
        LLVM_DEBUG(dbgs() << "Found unexpected user: " << *U2 << "\n");
        LLVM_DEBUG(dbgs() << "So class could be NOT FINAL."
                          << "\n\n");
        return false;
      }

      // If U3 is Constant then check that its user is a vtable. If not then we
      // found something unexpected, so be conservative and bail out  - we can't
      // prove the class is final. If U3 is GlobalVariable then it is a
      // type_info or something unexpected, return false in both cases.
      for (auto U3 : U2->users()) {
        if (!isa<Constant>(U3)) {
          // We don't know what is it.
          LLVM_DEBUG(dbgs() << "Found unexpected user: " << *U1 << "\n");
          LLVM_DEBUG(dbgs() << "So class could be NOT FINAL."
                            << "\n\n");
          return false;
        }

        if (!TestForGlobal(U3))
          return false;
      }
    }
  }
  LLVM_DEBUG(dbgs() << "There are no users of this type_info.\n");
  LLVM_DEBUG(dbgs() << "So class is FINAL."
                    << "\n\n");
  return true;
}

/// Check that all users of the call instruction are compare instructions with
/// EQ or NE predicate.
static bool allUsersICmpEQorNE(CallInst *Call) {
  ICmpInst::Predicate Pred = ICmpInst::BAD_ICMP_PREDICATE;
  Value *ICmpLHS;
  for (auto U : Call->users()) {
    if (!match(U, m_ICmp(Pred, m_Value(ICmpLHS), m_Zero())))
      return false;
    if (Pred != ICmpInst::ICMP_EQ && Pred != ICmpInst::ICMP_NE)
      return false;
  }
  return true;
}

bool OptimizeDynamicCastsPass::isTransformationApplicable(CallInst *Call) {
  assert(Call->arg_size() == 4 &&
         "Unexpected number of operands in dynamic_cast call!");

  auto DstOp = Call->getOperand(2);
  assert(DstOp && "Expected the non-null third operand of dynamic_cast call!");
  GlobalVariable *DestTypeInfo = nullptr;
  DestTypeInfo = dyn_cast<GlobalVariable>(DstOp);
  if (!DestTypeInfo)
    return false;

  // We could optimize only dynamic_cast to the final class.
  TypeInfoMap::iterator it = TypeInfoAnalysis.find(DestTypeInfo);
  if (it != TypeInfoAnalysis.end()) {
    // This type_info was already met.
    if (!it->second)
      return false;
  } else {
    bool IsTransformable = isTypeInfoGlobalForFinalClass(DestTypeInfo);
    TypeInfoAnalysis.insert(
        std::pair<GlobalVariable *, bool>(DestTypeInfo, IsTransformable));
    if (!IsTransformable)
      return false;
  }

  // Extract hint of the __dynamic_cast.
  // Hint gives an information about the structure of the inheritance.
  // When  hint >= 0 then it is an offset to the most derived object.
  assert(Call->getOperand(3) && "No hint operand in dynamic_cast call!");
  auto *Hint = cast<ConstantInt>(Call->getOperand(3));

  // We make an optimization only in the following cases:
  // 1. Hint >= 0.
  // 2. If Hint < 0 then all uses of dynamic_cast should be cmp (eq or ne
  // only) with nullptr.
  if (!Hint->isNegative())
    return true;

  // Hint is negative, check that all uses are cmp with nullptr (eq or ne
  // only).
  return allUsersICmpEQorNE(Call);
}

PreservedAnalyses OptimizeDynamicCastsPass::runImpl(
    Module &M, WholeProgramInfo &WPI,
    std::function<const TargetLibraryInfo &(Function &F)> GetTLI) {
  // Transformation is not supported for Microsoft ABI.
  Triple T(M.getTargetTriple());
  if (T.isKnownWindowsMSVCEnvironment())
    return PreservedAnalyses::all();

  if (!WPI.isWholeProgramSafe())
    return PreservedAnalyses::all();

  bool Changed = false;
  for (Function &F : M) {
    for (BasicBlock &BB : F) {
      for (Instruction &I : BB) {

        // We are interested in __dynamic_cast calls.
        CallInst *Call = dyn_cast<CallInst>(&I);
        if (!Call)
          continue;

        Function *DynCastFunc = Call->getCalledFunction();
        if (!DynCastFunc || Call->isNoBuiltin())
          continue;

        LibFunc Func = NumLibFuncs;
        if (!GetTLI(F).getLibFunc(*DynCastFunc, Func) ||
            Func != LibFunc::LibFunc_dynamic_cast)
          continue;
        auto ObjPointer = Call->getArgOperand(0);
        if (!isTransformationApplicable(Call))
          continue;

        LLVM_DEBUG(dbgs() << "Found dynamic_cast eligible for transformation:\n"
                          << "  " << *Call << "\n");
        LLVM_DEBUG(
            dbgs() << "Users of dynamic_cast before the transformation:\n");
        LLVM_DEBUG(for (const auto &U : Call->users()) dbgs() << *U << "\n");
        LLVM_DEBUG(dbgs() << "\n");

        // Here we have an appropriate case. Generate comparison of pointers
        // to type_info objects.
        Type *ObjType = nullptr;
        ObjType = Type::getInt8Ty(Call->getContext());

        IRBuilder<> Builder(&I);

        // Cast pointer to object to i8***, because it is a pointer to pointer
        // to vtable that contains pointer to type_info object.
        LoadInst *Vptr = nullptr;
        auto Int8PtrTy = PointerType::getUnqual(Call->getContext());
        // Load pointer to vtable.
        Vptr = Builder.CreateLoad(Int8PtrTy, ObjPointer);
        // Calculate address of pointer to type_info.
        auto AddressOfTypeInfoPtr =
            Builder.CreateGEP(Int8PtrTy, Vptr, Builder.getInt32(-1));
        // Load pointer to type_info.
        auto TypeInfoPtr = Builder.CreateLoad(Int8PtrTy, AddressOfTypeInfoPtr);

        assert(
            Call->getOperand(3) &&
            "Hint operand of the dynamic_cast call expected to be not null!");
        auto *Hint = cast<ConstantInt>(Call->getOperand(3));
        if (Hint->isNegative()) {
          assert(allUsersICmpEQorNE(Call) &&
                 "Only ICmpInst users are expected!");
          for (auto U : Call->users()) {
            auto CompareWithNull = cast<ICmpInst>(U);
            // Compare pointers to type_info. When the result of a dynamic_cast
            // != null types are equal and when the result  == null then types
            // are not equal, so use inverse predicate.
            auto RTTICompare =
                Builder.CreateICmp(CompareWithNull->getInversePredicate(),
                                   TypeInfoPtr, Call->getArgOperand(2));
            CompareWithNull->replaceAllUsesWith(RTTICompare);
          }
        } else {
          auto RTTIEqualCheck =
              Builder.CreateICmpEQ(TypeInfoPtr, Call->getArgOperand(2));
          Value *CastedObjPointer = ObjPointer;
          if (!Hint->isZero()) {
            // Calculate value of the pointer after dynamic cast.
            auto NegHint =
                APInt::getZero(Hint->getType()->getScalarSizeInBits());
            NegHint -= Hint->getValue();
            CastedObjPointer = Builder.CreateGEP(
                ObjType, ObjPointer,
                ConstantInt::getIntegerValue(Hint->getType(), NegHint));
          }
          auto ResultOfDynCast = Builder.CreateSelect(
              RTTIEqualCheck, CastedObjPointer,
              ConstantInt::getNullValue(ObjPointer->getType()));
          Call->replaceAllUsesWith(ResultOfDynCast);
        }
        Changed = true;
        OptimizedCounter++;
      }
    }
  }
  if (!Changed)
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  PA.preserve<TargetLibraryAnalysis>();
  return PA;
}

PreservedAnalyses OptimizeDynamicCastsPass::run(Module &M,
                                                ModuleAnalysisManager &AM) {
  auto &WPI = AM.getResult<WholeProgramAnalysis>(M);
  FunctionAnalysisManager &FAM =
      AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetTLI = [&FAM](Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(F);
  };
  return runImpl(M, WPI, GetTLI);
}
