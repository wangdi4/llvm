//===------------ MemManageTransOP.cpp - DTransMemManageTransPass ---------===//
//
// Copyright (C) 2022-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the DTrans Initial Memory Management Transformation
// for opaque pointers.
//
// Detects memory pool allocator class by analyzing member functions of the
// class like allocateBlock, destroyObject, commitAllocationObject, etc.
// Increases the size of block (i.e number of objects allocated each time) if
// there are no legality issues.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/MemManageTransOP.h"

#include "Intel_DTrans/Analysis/DTransAllocCollector.h"
#include "Intel_DTrans/Analysis/TypeMetadataReader.h"
#include "Intel_DTrans/Transforms/MemManageInfoOPImpl.h"
#include "llvm/Analysis/Intel_WP.h"

#define DTRANS_MEMMANAGETRANSOP "dtrans-memmanagetransop"

using namespace llvm;
using namespace dtransOP;

namespace {

class MemManageTransImpl {

  constexpr static int MaxNumCandidates = 1;

public:
  MemManageTransImpl(DTransTypeManager &TM, TypeMetadataReader &MDReader,
                     llvm::dtransOP::MemManageTransOPPass::MemTLITy GetTLI)
      : TM(TM), MDReader(MDReader), GetTLI(GetTLI), DTAC(MDReader, GetTLI){};

  ~MemManageTransImpl() {
    for (auto *CInfo : Candidates)
      delete CInfo;
  }

  // Returns current processing candidate.
  MemManageCandidateInfo *getCurrentCandidate() {
    // For now, only one candidate is allowed.
    assert(Candidates.size() == 1 && "Unexpected number of candidates");
    auto Cand = *Candidates.begin();
    return Cand;
  }

  bool run(Module &M);

private:
  DTransTypeManager &TM;
  TypeMetadataReader &MDReader;
  llvm::dtransOP::MemManageTransOPPass::MemTLITy GetTLI;
  DTransAllocCollector DTAC;

  // List of candidates. For now, only one candidate is allowed.
  SmallVector<MemManageCandidateInfo *, MaxNumCandidates> Candidates;

  // Set of Interface functions and their users.
  SmallPtrSet<Function *, 32> RelatedFunctions;

  bool gatherCandidates(Module &M, FunctionTypeResolver &TypeResolver);
  bool analyzeCandidates(Module &M);

  bool isUsedOnlyInUnusedVTable(Value *);
  bool checkInterfaceFunctions();
};

// Collect candidates.
bool MemManageTransImpl::gatherCandidates(Module &M,
                                          FunctionTypeResolver &TypeResolver) {
  for (auto *Str : M.getIdentifiedStructTypes()) {
    if (!Str->hasName())
      continue;

    DTransStructType *StrType = TM.getStructType(Str->getName());
    if (!StrType)
      continue;

    std::unique_ptr<MemManageCandidateInfo> CInfo(
        new MemManageCandidateInfo(M));

    if (!CInfo->isCandidateType(StrType))
      continue;

    DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP, {
      dbgs() << "  Considering candidate: ";
      Str->print(dbgs(), true, true);
      dbgs() << "\n";
    });

    if (!CInfo->collectMemberFunctions(TypeResolver, true)) {
      DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP,
                      { dbgs() << "  Failed: member function collection.\n"; });
      continue;
    }
    if (Candidates.size() >= MaxNumCandidates) {
      DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP, {
        dbgs() << "  Failed: Exceeding maximum candidate limit.\n";
      });
      return false;
    }
    Candidates.push_back(CInfo.release());
  }

  if (Candidates.empty()) {
    DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP,
                    { dbgs() << "  Failed: No candidates found.\n"; });
    return false;
  }

  DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP, {
    dbgs() << "  Possible candidate structs: \n";
    for (auto CInfo : Candidates) {
      DTransStructType *St = CInfo->getStringAllocatorType();
      dbgs() << "      " << St->getName() << "\n";
    }
  });

  return true;
}

// Returns true if we can prove that "U" is used only in GlobalVariables that
// are only accessed from interface functions.
bool MemManageTransImpl::isUsedOnlyInUnusedVTable(Value *U) {
  std::function<bool(Value *, SmallPtrSetImpl<GlobalVariable *> &)>
      FindAllGlobalVariableUsers;
  std::function<bool(Value *)> CheckAllUsesInInterfaceFuncs;

  // Recursively finds all GlobalVariable users of "V". Returns false if it
  // finds any non-GlobalVariable user.
  FindAllGlobalVariableUsers =
      [&FindAllGlobalVariableUsers](Value *V,
                                    SmallPtrSetImpl<GlobalVariable *> &GVSet) {
        if (auto *GV = dyn_cast<GlobalVariable>(V)) {
          GVSet.insert(GV);
          return true;
        }
        auto *CE = dyn_cast<Constant>(V);
        if (!CE)
          return false;
        for (User *CEUser : CE->users())
          if (!FindAllGlobalVariableUsers(CEUser, GVSet))
            return false;
        return true;
      };

  // Recursively finds all users of "V" and makes sure "V" is used only
  // in interface functions. Returns false if it finds any non-function
  // user.
  CheckAllUsesInInterfaceFuncs = [this,
                                  &CheckAllUsesInInterfaceFuncs](Value *V) {
    MemManageCandidateInfo *Cand = getCurrentCandidate();
    if (auto *I = dyn_cast<Instruction>(V)) {
      // Check if "I" is in interface functions.
      if (!Cand->isInterfaceFunction(I->getFunction()))
        return false;
    } else if (auto *CE = dyn_cast<Constant>(V)) {
      for (User *CEUser : CE->users())
        if (!CheckAllUsesInInterfaceFuncs(CEUser))
          return false;
    } else {
      // Don't allow any non-constant users.
      return false;
    }
    return true;
  };

  // Returns true if all uses of "GV" are in interface functions.
  auto IsGVUsedOnlyInInterfaceFunctions =
      [&CheckAllUsesInInterfaceFuncs](GlobalVariable *GV) {
        for (auto *UU : GV->users())
          if (!CheckAllUsesInInterfaceFuncs(UU))
            return false;
        return true;
      };

  SmallPtrSet<GlobalVariable *, 2> GVSet;
  if (!FindAllGlobalVariableUsers(U, GVSet))
    return false;
  if (GVSet.empty())
    return false;
  for (auto *GV : GVSet)
    if (!IsGVUsedOnlyInInterfaceFunctions(GV))
      return false;
  return true;
}

// Returns false if any interface function is not called from
// member functions of StringAllocator. It does check for the following
// two things.
// 1. All interface functions need to be called from either StringAllocator
//    or Interface functions.
// 2. If interface function is in VTable (i.e GlobalVariable), this makes
//    sure the all uses of VTable are in interface functions, which will be
//    analyzed later. In the analysis, it will be indirectly proved that
//    there is no real use of VTable.
bool MemManageTransImpl::checkInterfaceFunctions() {
  MemManageCandidateInfo *Cand = getCurrentCandidate();
  for (auto InterF : Cand->interface_functions()) {
    RelatedFunctions.insert(InterF);
    for (User *U : InterF->users()) {
      if (auto *CB = dyn_cast<CallBase>(U)) {
        auto *CalledF = CB->getFunction();
        if (!CalledF)
          return false;
        RelatedFunctions.insert(CalledF);
        if (!Cand->isStrAllocatorOrInterfaceFunction(CalledF) &&
            !isUsedOnlyInUnusedVTable(CalledF))
          return false;
      } else {
        // If "U" is not a call, check "U" is in unused VTables.
        if (!isUsedOnlyInUnusedVTable(U))
          return false;
      }
    }
  }
  return true;
}

// Check legality issues for candidates.
bool MemManageTransImpl::analyzeCandidates(Module &M) {

  // Returns true if "Call" is library function call.
  auto IsLibFunction = [](const CallBase *Call, const TargetLibraryInfo &TLI) {
    LibFunc LibF;
    auto *F = dyn_cast_or_null<Function>(Call->getCalledFunction());
    if (!F || !TLI.getLibFunc(*F, LibF) || !TLI.has(LibF))
      return false;
    return true;
  };

  MemManageCandidateInfo *Cand = getCurrentCandidate();
  DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP,
                  { dbgs() << "  Analyzing Candidate ...\n"; });

  // Check all inner function calls, which are calls in InterfaceFunctions,
  // are only:
  // 1. Library function calls
  // 2. DTrans alloc/free calls
  // 3. Dummy alloc/free calls.
  for (auto CB : Cand->inner_function_calls()) {
    auto &TLI = GetTLI(*CB->getFunction());
    if (IsLibFunction(CB, TLI))
      continue;

    dtrans::AllocKind AK = DTAC.getAllocFnKind(CB, TLI);
    if (AK != dtrans::AK_NotAlloc)
      continue;

    dtrans::FreeKind FK = DTAC.getFreeFnKind(CB, TLI);
    if (FK != dtrans::FK_NotFree)
      continue;

    if (DTransAllocCollector::isDummyFuncWithThisAndIntArgs(CB, TLI,
                                                            MDReader) ||
        DTransAllocCollector::isDummyFuncWithThisAndInt8PtrArgs(CB, TLI,
                                                                MDReader))
      continue;

    DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP, {
      dbgs() << "    Failed: Unexpected Inner call " << *CB << "\n";
    });
    return false;
  }

  if (!checkInterfaceFunctions()) {
    DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP, {
      dbgs() << "   Failed: Unknown Interface function uses\n";
    });
    return false;
  }

  // TODO: Makes sure ReusableArenaAllocatorType and all other related
  // types are not escaped.

  DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP,
                  { dbgs() << "  No issues found with candidate\n"; });

  return true;
}

bool MemManageTransImpl::run(Module &M) {
  DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP,
                  { dbgs() << "MemManageTransOP transformation: \n"; });

  // Collect candidates.
  DTransLibraryInfo DTransLibInfo(TM, GetTLI);
  DTransLibInfo.initialize(M);
  FunctionTypeResolver TypeResolver(MDReader, DTransLibInfo);
  if (!gatherCandidates(M, TypeResolver))
    return false;

  DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP, {
    if (MemManageCandidateInfo *Cand = getCurrentCandidate())
      Cand->dump();
  });

  DTAC.populateAllocDeallocTable(M);
  if (!analyzeCandidates(M))
    return false;

  // TODO: Implement the pass.
  return false;
}

} // end anonymous namespace

namespace llvm {

namespace dtransOP {

bool MemManageTransOPPass::runImpl(Module &M, WholeProgramInfo &WPInfo,
                                   MemManageTransOPPass::MemTLITy GetTLI) {
  if (!dtrans::shouldRunOpaquePointerPasses(M)) {
    DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP,
                    dbgs() << "MemManageTransOP inhibited: opaque pointer "
                              "passes NOT in use\n");
    return false;
  }

  auto TTIAVX2 = TargetTransformInfo::AdvancedOptLevel::AO_TargetHasIntelAVX2;
  if (!WPInfo.isWholeProgramSafe() || !WPInfo.isAdvancedOptEnabled(TTIAVX2)) {
    DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP,
                    dbgs() << "MemManageTransOP inhibited: "
                           << "Not whole-program-safe or not AVX2\ns");
    return false;
  }

  DTransTypeManager TM(M.getContext());
  TypeMetadataReader MDReader(TM);
  if (!MDReader.initialize(M))
    return false;

  MemManageTransImpl MemManageTransI(TM, MDReader, GetTLI);
  return MemManageTransI.run(M);
}

PreservedAnalyses MemManageTransOPPass::run(Module &M,
                                            ModuleAnalysisManager &AM) {
  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);

  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetTLI = [&FAM](const Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(*(const_cast<Function *>(&F)));
  };

  if (!runImpl(M, WPInfo, GetTLI))
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}

} // namespace dtransOP

} // namespace llvm
