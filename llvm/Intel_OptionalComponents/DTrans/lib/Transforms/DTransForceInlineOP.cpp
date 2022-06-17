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

namespace {
class DTransForceInlineOP {
public:
  bool run(Module &M, std::function<const TargetLibraryInfo& (const Function&)> GetTLI);
};

bool DTransForceInlineOP::run(
    Module &M,
    std::function<const TargetLibraryInfo &(const Function &)> GetTLI) {

  // Returns true if “Fn” is empty.
  auto IsEmptyFunction = [] (Function *Fn) {
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
  for (Function *F: SOAToAOSCandidateMethods)
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
  for (Function *F: MemInitFuncs)
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
  return true;
}

class DTransForceInlineOPWrapper : public ModulePass {
public:
  static char ID;

  DTransForceInlineOPWrapper() : ModulePass(ID) {
    initializeDTransForceInlineOPWrapperPass(
        *PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override {
    auto GetTLI = [this](const Function& F) -> TargetLibraryInfo& {
      return this->getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
    };
    return Impl.runImpl(M, GetTLI); }

private:
  DTransForceInlineOPPass Impl;
};

} // end anonymous namespace

namespace llvm {
namespace dtransOP {

PreservedAnalyses
DTransForceInlineOPPass::run(Module &M, ModuleAnalysisManager &MAM) {
  auto& FAM = MAM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetTLI = [&FAM](const Function& F) -> TargetLibraryInfo& {
    return FAM.getResult<TargetLibraryAnalysis>(*(const_cast<Function*>(&F)));
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
INITIALIZE_PASS_BEGIN(DTransForceInlineOPWrapper,
                "dtrans-force-inline-op",
                "DTrans force inline and noinline", false, false)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_END(DTransForceInlineOPWrapper,
                "dtrans-force-inline-op",
                "DTrans force inline and noinline", false, false)
ModulePass *llvm::createDTransForceInlineOPWrapperPass() {
  return new DTransForceInlineOPWrapper();
}
