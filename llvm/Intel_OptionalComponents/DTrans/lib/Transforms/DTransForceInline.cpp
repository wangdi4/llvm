//===----- DTransForceInline.cpp - Force inlining/noninling for DTrans ----===//
//
// Copyright (C) 2022-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/DTransForceInline.h"
#include "Intel_DTrans/Analysis/DTransUtils.h"
#include "Intel_DTrans/DTransCommon.h"
#include "Intel_DTrans/Transforms/MemManageInfoImpl.h"
#include "Intel_DTrans/Transforms/StructOfArraysInfoImpl.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Operator.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"

#include "SOAToAOSInternal.h"

#define DEBUG "dtrans-force-inline"

using namespace llvm;
using namespace dtrans;

namespace {
class DTransForceInline {
public:
  bool run(Module &M);
};

bool DTransForceInline::run(Module &M) {
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

  // Only run this pass if we have typed pointers
  if (!M.getContext().supportsTypedPointers())
    return false; 

  // Set of SOAToAOS candidates.
  SmallPtrSet<StructType*, 4> SOAToAOSCandidates;
  // Suppress inlining for SOAToAOS candidates.
  SmallSet<Function *, 20> SOAToAOSCandidateMethods;
  for (auto *Str : M.getIdentifiedStructTypes()) {
    dtrans::soatoaos::SOAToAOSCFGInfo Info;
    if (!Info.populateLayoutInformation(Str)) {
      DEBUG_WITH_TYPE(DTRANS_LAYOUT_DEBUG_TYPE, {
        dbgs() << "  ; Not candidate ";
        Str->print(dbgs(), true, true);
        dbgs() << " because it does not look like a candidate structurally.\n";
      });
      continue;
    }
    if (!Info.populateCFGInformation(
            M, true /* Respect size restrictions */,
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

    SOAToAOSCandidates.insert(Str);
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
    dtrans::SOACandidateInfo MemInfo;
    if (!MemInfo.isCandidateType(TI))
      continue;
    DEBUG_WITH_TYPE(DTRANS_STRUCTOFARRAYSINFO, {
      dbgs() << "MemInitTrimDown transformation";
      dbgs() << "  Considering candidate: ";
      TI->print(dbgs(), true, true);
      dbgs() << "\n";
    });
    if (!MemInfo.collectMemberFunctions(M, false)) {
      DEBUG_WITH_TYPE(DTRANS_STRUCTOFARRAYSINFO, {
        dbgs() << "  Failed: member functions collections.\n";
      });
      continue;
    }

    if (!MemInitFuncs.empty()) {
      DEBUG_WITH_TYPE(DTRANS_STRUCTOFARRAYSINFO, {
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
  for (auto *Str : M.getIdentifiedStructTypes()) {
    dtrans::MemManageCandidateInfo MemManageInfo(M);
    if (!MemManageInfo.isCandidateType(Str))
      continue;
    DEBUG_WITH_TYPE(DTRANS_MEMMANAGEINFO, {
      dbgs() << "MemManageTrans considering candidate: ";
      Str->print(dbgs(), true, true);
      dbgs() << "\n";
    });
    if (!MemManageInfo.collectMemberFunctions(false)) {
      DEBUG_WITH_TYPE(DTRANS_MEMMANAGEINFO, {
        dbgs() << "  Failed: member functions collections.\n";
      });
      continue;
    }
    if (!MemManageInlineMethods.empty() || !MemManageNoInlineMethods.empty()) {
      DEBUG_WITH_TYPE(DTRANS_MEMMANAGEINFO, {
        dbgs() << "  Failed: More than one candidate found.\n";
      });
      MemManageInlineMethods.clear();
      MemManageNoInlineMethods.clear();
      break;
    }
    if (!MemManageInfo.collectInlineNoInlineMethods(&MemManageInlineMethods,
                                               &MemManageNoInlineMethods)) {
      DEBUG_WITH_TYPE(DTRANS_MEMMANAGEINFO, {
        dbgs() << "  Failed: Heuristics\n";
      });
      MemManageInlineMethods.clear();
      MemManageNoInlineMethods.clear();
      break;
    }
  }
  // Suppress inlining.
  for (Function *F: MemManageNoInlineMethods)
    if (!IsEmptyFunction(F))
      F->addFnAttr("noinline-dtrans");
  // Force inlining.
  for (Function *F: MemManageInlineMethods)
    F->addFnAttr("prefer-inline-dtrans");
  return true;
}

class DTransForceInlineWrapper : public ModulePass {
public:
  static char ID;

  DTransForceInlineWrapper() : ModulePass(ID) {
    initializeDTransForceInlineWrapperPass(
        *PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override { return Impl.runImpl(M); }

private:
  DTransForceInlinePass Impl;
};

} // end anonymous namespace

namespace llvm {
namespace dtrans {

PreservedAnalyses
DTransForceInlinePass::run(Module &M, ModuleAnalysisManager &MAM) {
  (void) runImpl(M);
  return PreservedAnalyses::all();
}

bool DTransForceInlinePass::runImpl(Module &M) {
  DTransForceInline Transform;
  return Transform.run(M);
}

} // end namespace dtrans
} // end namespace llvm

char DTransForceInlineWrapper::ID = 0;
INITIALIZE_PASS(DTransForceInlineWrapper,
                "dtrans-force-inline",
                "DTrans force inline and noinline", false, false)

ModulePass *llvm::createDTransForceInlineWrapperPass() {
  return new DTransForceInlineWrapper();
}
