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
      : TM(TM), MDReader(MDReader), GetTLI(GetTLI){};

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

  // List of candidates. For now, only one candidate is allowed.
  SmallVector<MemManageCandidateInfo *, MaxNumCandidates> Candidates;

  bool gatherCandidates(Module &M, FunctionTypeResolver &TypeResolver);
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
