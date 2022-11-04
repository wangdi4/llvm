//=--- DTransForceInlineOP.cpp - Force inlining/noninlining for DTrans ---===//
//=---------------- Opaque pointer friendly version -------------------------//
//
// Copyright (C) 2022-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/DTransForceInlineOP.h"
#include "Intel_DTrans/Analysis/DTransTypeMetadataBuilder.h"
#include "Intel_DTrans/Analysis/DTransTypeMetadataConstants.h"
#include "Intel_DTrans/Analysis/DTransTypes.h"
#include "Intel_DTrans/Analysis/DTransUtils.h"
#include "Intel_DTrans/Analysis/TypeMetadataReader.h"
#include "Intel_DTrans/DTransCommon.h"
#include "Intel_DTrans/Transforms/MemManageInfoOPImpl.h"
#include "Intel_DTrans/Transforms/StructOfArraysOPInfoImpl.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Operator.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"

#include "SOAToAOSOPInternal.h"
#include "SOAToAOSOPStruct.h"

#define DEBUG "dtrans-force-inline-op"

using namespace llvm;
using namespace dtransOP;

static constexpr unsigned ParamNumberUsesLimit = 6;

namespace {
class DTransForceInlineOP {
public:
  bool run(Module &M,
           std::function<const TargetLibraryInfo &(const Function &)> GetTLI);
};

bool DTransForceInlineOP::run(
    Module &M,
    std::function<const TargetLibraryInfo &(const Function &)> GetTLI) {

  // Returns true if “Fn” is empty.
  auto IsEmptyFunction = [](Function *Fn) {
    if (Fn->isDeclaration())
      return false;
    for (auto &I : Fn->getEntryBlock()) {
      if (isa<DbgInfoIntrinsic>(I))
        continue;
      if (isa<ReturnInst>(I))
        return true;
      break;
    }
    return false;
  };

  // Returns true if Argument "Param" is used only to pass as an argument to
  // calls and typecast to multiple types before passing.
  //  Ex:
  //      foo(void * value) {
  //        ...
  //        bar1(..., (short*) value);
  //        ...
  //        bar2(..., (char*) value);
  //        ...
  //        bar3(..., (struct A *) value);
  //        ...
  //      }
  auto IsArgPassedAsMultipleTypes =
      [](Argument *Param, SmallPtrSetImpl<CallBase *> &NoInlineCalls,
         TypeMetadataReader &MDReader) {
        SmallPtrSet<Type *, 4> UsedTypesSet;
        for (Use &U : Param->uses()) {
          // Makes sure "Param" is used only in Calls.
          auto *CB = dyn_cast<CallBase>(U.getUser());
          if (!CB || !CB->isArgOperand(&U))
            return false;
          Function *Callee = CB->getCalledFunction();
          if (!Callee)
            return false;
          unsigned ArgNo = CB->getArgOperandNo(&U);
          auto *DTy = dyn_cast_or_null<DTransFunctionType>(
              MDReader.getDTransTypeFromMD(Callee));
          if (!DTy || Callee->arg_size() <= ArgNo)
            return false;
          DTransType *DTArgTy = DTy->getArgType(ArgNo);
          auto *PTy = dyn_cast<DTransPointerType>(DTArgTy);
          if (!PTy)
            return false;
          Type *ElemTy = PTy->getPointerElementType()->getLLVMType();
          // Ignore if Param is passed as I8Ptr.
          if (ElemTy->isIntegerTy(8))
            continue;
          UsedTypesSet.insert(ElemTy);
          if (Callee->isDeclaration())
            continue;
          if (Callee->getArg(ArgNo)->hasNUses(0))
            continue;
          NoInlineCalls.insert(CB);
        }
        // Check if "Param" is typecast to multiple types before passing
        // as arguments to calls.
        if (UsedTypesSet.size() > 1 && NoInlineCalls.size() > 0)
          return true;
        return false;
      };

  // Returns true if "F" has a i8* argument that is saved into a struct
  // and a pointer to struct is passed as the argument to "F".
  //  Ex:
  //      foo(void * value) {
  //        ...
  //        struct.A* c;
  //        ...
  //        c->field1 =  value;
  //        ...
  //      }
  //      bar() {
  //        struct.A* ptr = baz();
  //        ...
  //        foo((void*) ptr);
  //      }
  //
  // Set "noinline-dtrans" attribute for "foo" to avoid badcasting for
  // "struct.A" in the example.
  //
  auto IsStructPtrReturnValuePassedAsI8 = [&](Function &F,
                                              DTransFunctionType *DFnTy,
                                              TypeMetadataReader &MDReader) {
    // Allow only single callsite.
    if (!F.hasOneUse())
      return false;
    auto *CB = dyn_cast<CallBase>(F.user_back());
    if (!CB || CB->getCalledFunction() != &F)
      return false;
    unsigned NumArgs = F.arg_size();
    for (unsigned ArgIdx = 0; ArgIdx < NumArgs; ++ArgIdx) {
      // Check argument is i8*
      DTransType *DTArgTy = DFnTy->getArgType(ArgIdx);
      auto *PTy = dyn_cast<DTransPointerType>(DTArgTy);
      if (!PTy)
        continue;
      if (!PTy->getPointerElementType()->getLLVMType()->isIntegerTy(8))
        continue;
      auto *Param = F.getArg(ArgIdx);
      if (Param->hasNUsesOrMore(ParamNumberUsesLimit))
        continue;
      // Check argument is a return value of another call and type of
      // the return value is "pointer to a struct".
      Value *Arg = CB->getArgOperand(ArgIdx);
      auto *ArgCall = dyn_cast<CallBase>(Arg);
      if (!ArgCall || ArgCall->hasNUsesOrMore(ParamNumberUsesLimit))
        continue;
      Function *ArgCallee = ArgCall->getCalledFunction();
      if (!ArgCallee)
        continue;
      DTransType *DType = MDReader.getDTransTypeFromMD(ArgCallee);
      if (!DType)
        continue;
      auto *FnType = cast<DTransFunctionType>(DType);
      DTransType *DRetTy = FnType->getReturnType();
      if (!DRetTy->isPointerTy() ||
          !isa<DTransStructType>(DRetTy->getPointerElementType()))
        continue;

      // Check if I8* argument is stored to a struct.
      bool ParamStoredInStruct = false;
      for (User *U : Param->users()) {
        auto *SI = dyn_cast<StoreInst>(U);
        if (!SI)
          continue;
        if (SI->getValueOperand() != Param)
          continue;
        auto *GEPI = dyn_cast<GetElementPtrInst>(SI->getPointerOperand());
        if (!GEPI || !isa<StructType>(GEPI->getSourceElementType()))
          continue;
        ParamStoredInStruct = true;
        break;
      }
      if (!ParamStoredInStruct)
        continue;
      return true;
    }
    return false;
  };

  // Only run this pass if we have opaque pointers
  if (M.getContext().supportsTypedPointers())
    return false;

  // Set up DTrans type manager and metdata reader
  DTransTypeManager TM(M.getContext());
  TypeMetadataReader MDReader(TM);
  if (!MDReader.initialize(M))
    return false;

  // Set of SOAToAOS candidates.
  SmallPtrSet<DTransStructType *, 4> SOAToAOSCandidates;
  // Suppress inlining for SOAToAOS candidates.
  SmallSet<Function *, 20> SOAToAOSCandidateMethods;
  for (auto *Str : M.getIdentifiedStructTypes()) {
    if (!Str->hasName())
      continue;
    dtransOP::DTransStructType *StrType = TM.getStructType(Str->getName());
    assert(StrType && "Expected DTransStructType");
    dtransOP::soatoaosOP::SOAToAOSOPCFGInfo Info;
    if (!Info.populateLayoutInformation(StrType)) {
      DEBUG_WITH_TYPE(DTRANS_LAYOUT_DEBUG_TYPE, {
        dbgs() << "  ; Not candidate ";
        Str->print(dbgs(), true, true);
        dbgs() << " because it does not look like a candidate structurally.\n";
      });
      continue;
    }
    if (!Info.populateCFGInformation(
            M, MDReader, true /* Respect size restrictions */,
            false /* Do not respect param attribute restrictions */)) {
      DEBUG_WITH_TYPE(DTRANS_LAYOUT_DEBUG_TYPE, {
        dbgs() << "  ; Not candidate ";
        Str->print(dbgs(), true, true);
        dbgs() << " because it does not look like a candidate from CFG "
                  "analysis.\n";
      });
      continue;
    }

    // Not more than 1 candidate.
    if (!SOAToAOSCandidateMethods.empty()) {
      DEBUG_WITH_TYPE(DTRANS_LAYOUT_DEBUG_TYPE,
                      dbgs() << "  ; Too many candidates found\n");
      SOAToAOSCandidateMethods.clear();
      break;
    }

    DEBUG_WITH_TYPE(DTRANS_LAYOUT_DEBUG_TYPE, {
      dbgs() << "  ; ";
      Str->print(dbgs(), true, true);
      dbgs() << " looks like SOAToAOS candidate.\n";
    });

    SOAToAOSCandidates.insert(StrType);
    Info.collectFuncs(&SOAToAOSCandidateMethods);
  }
  // Don’t need to track empty functions for DTrans. Analysis will
  // be simpler if empty functions are inlined.
  for (Function *F : SOAToAOSCandidateMethods)
    if (!IsEmptyFunction(F))
      F->addFnAttr("noinline-dtrans");

  SmallSet<Function *, 32> MemInitFuncs;
  // Only SOAToAOS candidates are considered for MemInitTrimDown.
  for (auto *TI : SOAToAOSCandidates) {
    dtransOP::SOACandidateInfo MemInfo(MDReader);
    if (!MemInfo.isCandidateType(TI))
      continue;
    DEBUG_WITH_TYPE(DTRANS_STRUCTOFARRAYSOPINFO, {
      dbgs() << "MemInitTrimDown transformation";
      dbgs() << "  Considering candidate: ";
      TI->getLLVMType()->print(dbgs(), true, true);
      dbgs() << "\n";
    });
    if (!MemInfo.collectMemberFunctions(M, false)) {
      DEBUG_WITH_TYPE(DTRANS_STRUCTOFARRAYSOPINFO, {
        dbgs() << "  Failed: member functions collections.\n";
      });
      continue;
    }

    if (!MemInitFuncs.empty()) {
      DEBUG_WITH_TYPE(DTRANS_STRUCTOFARRAYSOPINFO, {
        dbgs() << "  Failed: More than one candidate struct found.\n";
      });
      MemInitFuncs.clear();
      break;
    }
    // Collect all member functions of candidate
    // struct and candidate array field structs.
    MemInfo.collectFuncs(M, &MemInitFuncs);
  }
  //   1. Member functions of candidate struct
  //   2. Member functions of all candidate array field structs.
  for (Function *F : MemInitFuncs)
    if (!IsEmptyFunction(F))
      F->addFnAttr("noinline-dtrans");

  // MEMMANAGETRANS:
  // Force inlining for all inner functions of Allocator.
  std::set<Function *> MemManageInlineMethods;
  // Suppress inlining for interface functions, StringAllocator
  // functions and StringObject functions.
  SmallSet<Function *, 16> MemManageNoInlineMethods;

  DTransLibraryInfo DTransLibInfo(TM, GetTLI);
  DTransLibInfo.initialize(M);
  FunctionTypeResolver TypeResolver(MDReader, DTransLibInfo);
  for (auto *Str : M.getIdentifiedStructTypes()) {
    if (!Str->hasName())
      continue;
    dtransOP::DTransStructType *StrType = TM.getStructType(Str->getName());
    assert(StrType && "Expected DTransStructType");

    // Determine whether this is the "StringAllocator" struct.
    dtransOP::MemManageCandidateInfo MemManageInfo(M);
    if (!MemManageInfo.isCandidateType(StrType))
      continue;

    DEBUG_WITH_TYPE(DTRANS_MEMMANAGEINFOOP, {
      dbgs() << "MemManageTrans considering candidate: ";
      Str->print(dbgs(), true, true);
      dbgs() << "\n";
    });
    if (!MemManageInfo.collectMemberFunctions(TypeResolver, false)) {
      DEBUG_WITH_TYPE(DTRANS_MEMMANAGEINFOOP, {
        dbgs() << "  Failed: member functions collections.\n";
      });
      continue;
    }

    if (!MemManageInlineMethods.empty() || !MemManageNoInlineMethods.empty()) {
      DEBUG_WITH_TYPE(DTRANS_MEMMANAGEINFOOP, {
        dbgs() << "  Failed: More than one candidate found.\n";
      });
      MemManageInlineMethods.clear();
      MemManageNoInlineMethods.clear();
      break;
    }

    if (!MemManageInfo.collectInlineNoInlineMethods(
            &MemManageInlineMethods, &MemManageNoInlineMethods)) {
      DEBUG_WITH_TYPE(DTRANS_MEMMANAGEINFOOP,
                      { dbgs() << "  Failed: Heuristics\n"; });
      MemManageInlineMethods.clear();
      MemManageNoInlineMethods.clear();
      break;
    }
  }
  // Suppress inlining.
  for (Function *F : MemManageNoInlineMethods)
    if (!IsEmptyFunction(F))
      F->addFnAttr("noinline-dtrans");
  // Force inlining.
  for (Function *F : MemManageInlineMethods)
    F->addFnAttr("prefer-inline-dtrans");

  // Mark calls with "noinline-dtrans" if any argument of a function is
  // used only to pass as an argument to calls and typecast to multiple
  // types before passing.
  //  Ex: Mark "noinline-dtrans" attribute for bar1, bar2 and bar3 calls.
  //
  //      foo(void * value) {
  //        ...
  //        bar1(..., (short*) value);
  //        ...
  //        bar2(..., (char*) value);
  //        ...
  //        bar3(..., (struct A *) value);
  //        ...
  //      }
  for (auto &F : M) {
    if (F.isDeclaration() || F.arg_size() < 1)
      continue;
    auto *DFnTy =
        dyn_cast_or_null<DTransFunctionType>(MDReader.getDTransTypeFromMD(&F));
    if (!DFnTy)
      continue;

    unsigned NumArgs = F.arg_size();
    SmallPtrSet<CallBase *, 8> NoInlinedCallSites;
    for (unsigned ArgIdx = 0; ArgIdx < NumArgs; ++ArgIdx) {
      DTransType *DTArgTy = DFnTy->getArgType(ArgIdx);
      auto *PTy = dyn_cast<DTransPointerType>(DTArgTy);
      if (!PTy)
        continue;
      if (!PTy->getPointerElementType()->getLLVMType()->isIntegerTy(8))
        continue;
      auto *Param = F.getArg(ArgIdx);
      if (Param->hasNUsesOrMore(ParamNumberUsesLimit))
        continue;
      NoInlinedCallSites.clear();
      if (!IsArgPassedAsMultipleTypes(Param, NoInlinedCallSites, MDReader))
        continue;
      for (auto *CB : NoInlinedCallSites)
        if (!CB->getCalledFunction()->hasFnAttribute(Attribute::AlwaysInline))
          CB->addFnAttr("noinline-dtrans");
    }

    // TODO: This code can be removed once CMPLRLLVM-41532 is fixed.
    if (IsStructPtrReturnValuePassedAsI8(F, DFnTy, MDReader))
      if (!F.hasFnAttribute(Attribute::AlwaysInline))
        F.addFnAttr("noinline-dtrans");
  }
  return true;
}

class DTransForceInlineOPWrapper : public ModulePass {
public:
  static char ID;

  DTransForceInlineOPWrapper() : ModulePass(ID) {
    initializeDTransForceInlineOPWrapperPass(*PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override {
    auto GetTLI = [this](const Function &F) -> TargetLibraryInfo & {
      return this->getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
    };
    return Impl.runImpl(M, GetTLI);
  }

private:
  DTransForceInlineOPPass Impl;
};

} // end anonymous namespace

namespace llvm {
namespace dtransOP {

PreservedAnalyses DTransForceInlineOPPass::run(Module &M,
                                               ModuleAnalysisManager &MAM) {
  auto &FAM = MAM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetTLI = [&FAM](const Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(*(const_cast<Function *>(&F)));
  };
  runImpl(M, GetTLI);
  return PreservedAnalyses::all();
}

bool DTransForceInlineOPPass::runImpl(
    Module &M,
    std::function<const TargetLibraryInfo &(const Function &)> GetTLI) {
  DTransForceInlineOP Transform;
  return Transform.run(M, GetTLI);
}

} // end namespace dtransOP
} // end namespace llvm

char DTransForceInlineOPWrapper::ID = 0;
INITIALIZE_PASS_BEGIN(DTransForceInlineOPWrapper, "dtrans-force-inline-op",
                      "DTrans force inline and noinline", false, false)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_END(DTransForceInlineOPWrapper, "dtrans-force-inline-op",
                    "DTrans force inline and noinline", false, false)
ModulePass *llvm::createDTransForceInlineOPWrapperPass() {
  return new DTransForceInlineOPWrapper();
}
