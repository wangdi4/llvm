//===------ SOAToAOSPrepare.cpp - SOAToAOSPreparePass ---------------------===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the DTrans SOAToAOS prepare pass to perform
// transformations that enable SOATOSOA for more candidates.
//
// Prepare layout information for candidates.
//
// In the below example, SOAToAOS transformation considers
// first two fields as candidates since they look like
// vector classes and rejects 3rd field as candidate. This
// pass converts the class of 3rd field to simple vector
// class (i.e similar to first two fields) by eliminating
// VTable pointer and removing wrapper class after doing
// legality checks.
//
// %class.FieldValueMap = type {
//    %class.vector*,
//    %class.vector.0*,
//    %class.refvector*,
//    MemoryManager*
// }
// %class.vector = type {
//    i8, i32, i32,
//    %class.elem**,
//    MemoryManager*
// }
// %class.vector.0 = type {
//    i8, i32, i32,
//    %class.elem0**,
//    MemoryManager*
// }
// %class.refvector = type { %class.basevector }
// %class.basevector = type {
//    Vtable *,
//    i8, i32, i32,
//    %class.elem2**,
//    MemoryManager*
// }
//
// This pass also fix member function calls, combine multiple
// calls into single call etc so that 3rd field can be considered
// as candidate by SOAToAOS transformation. This pass helps to
// avoid adding a lot of workarounds to SOAToAOS implementation.
// TODO: Add more examples later.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/SOAToAOSPrepare.h"
#include "Intel_DTrans/Analysis/DTrans.h"
#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "Intel_DTrans/Analysis/DTransAnnotator.h"
#include "Intel_DTrans/DTransCommon.h"
#include "Intel_DTrans/Transforms/SOAToAOSExternal.h"

#include "llvm/Analysis/Intel_WP.h"

#define DTRANS_SOATOAOSPREPARE "dtrans-soatoaos-prepare"

using namespace llvm;
using namespace dtrans;
using namespace soatoaos;

namespace llvm {
namespace dtrans {
namespace soatoaos {

class CandidateInfo {
public:
  bool isCandidate(Type *Ty, unsigned Offset);
  void printCandidateInfo(void);

private:
  StructType *Struct;
  unsigned FieldOffset;
};

class SOAToAOSPrepareImpl {
public:
  SOAToAOSPrepareImpl(Module &M, DTransAnalysisInfo &DTInfo)
      : M(M), DTInfo(DTInfo) {}

  ~SOAToAOSPrepareImpl() {
    for (auto *Cand : Candidates) {
      delete Cand;
    }
    Candidates.clear();
  }
  bool run(void);

private:
  constexpr static int MaxNumCandidates = 1;
  constexpr static int MaxNumPotentialArrs = 1;

  Module &M;
  DTransAnalysisInfo &DTInfo;
  SmallSet<CandidateInfo *, MaxNumCandidates> Candidates;

  bool gatherCandidateInfo(void);
};

bool CandidateInfo::isCandidate(Type *Ty, unsigned Offset) {
  Struct = dyn_cast<StructType>(Ty);
  FieldOffset = Offset;
  // TODO: Add More checks
  return true;
}

void CandidateInfo::printCandidateInfo(void) {
  dbgs() << "    Candidate struct: " << getStructName(Struct);
  dbgs() << "    FieldOff: " << FieldOffset << "\n";
}

bool SOAToAOSPrepareImpl::gatherCandidateInfo() {
  for (dtrans::TypeInfo *TI : DTInfo.type_info_entries()) {
    dtrans::soatoaos::SOAToAOSCFGInfo Info;
    Type *Ty = TI->getLLVMType();

    if (!Info.populateLayoutInformation(Ty))
      continue;
    if (DTInfo.testSafetyData(TI, dtrans::DT_SOAToAOS))
      continue;
    bool FieldSafetyCheck = true;
    for (auto *FI : Info.fields()) {
      auto *FInfo = DTInfo.getTypeInfo(FI);
      if (!FInfo || DTInfo.testSafetyData(FInfo, dtrans::DT_SOAToAOS)) {
        FieldSafetyCheck = false;
        break;
      }
    }
    if (!FieldSafetyCheck)
      continue;
    if (!Info.populateCFGInformation(M, true, true))
      continue;
    if (Info.getNumPotentialArrays() != MaxNumPotentialArrs)
      continue;
    auto *ArrOffsetIt = Info.potential_arr_fields().begin();

    std::unique_ptr<CandidateInfo> CandI(new CandidateInfo());
    if (!CandI->isCandidate(Ty, *ArrOffsetIt))
      continue;

    // TODO: Add more checks here.
    Candidates.insert(CandI.release());
  }
  if (Candidates.empty()) {
    DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE, {
      dbgs() << "SOAToAOSPrepare Failed: No candidates found.\n";
    });
    return false;
  }
  return true;
}

bool SOAToAOSPrepareImpl::run(void) {
  if (!gatherCandidateInfo())
    return false;
  if (Candidates.size() != MaxNumCandidates) {
    dbgs() << "SOAToAOSPrepare: Candidate found\n";
    DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE, {
      dbgs() << "SOAToAOSPrepare Failed: More candidates found.\n";
    });
    return false;
  }
  DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE, {
    dbgs() << "SOAToAOSPrepare: Candidate found.\n";
    auto *CandI = *Candidates.begin();
    CandI->printCandidateInfo();
  });

  // TODO: Add more code here.
  return true;
}

} // end namespace soatoaos

bool SOAToAOSPreparePass::runImpl(Module &M, DTransAnalysisInfo &DTInfo,
                                  WholeProgramInfo &WPInfo) {
  auto TTIAVX2 = TargetTransformInfo::AdvancedOptLevel::AO_TargetHasAVX2;
  if (!WPInfo.isWholeProgramSafe() || !WPInfo.isAdvancedOptEnabled(TTIAVX2))
    return false;

  SOAToAOSPrepareImpl PrepareImpl(M, DTInfo);

  return PrepareImpl.run();
}

PreservedAnalyses SOAToAOSPreparePass::run(Module &M,
                                           ModuleAnalysisManager &AM) {
  auto &WP = AM.getResult<WholeProgramAnalysis>(M);
  if (!WP.isWholeProgramSafe())
    return PreservedAnalyses::all();

  auto &DTransInfo = AM.getResult<DTransAnalysis>(M);
  bool Changed = runImpl(M, DTransInfo, WP);

  if (!Changed)
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}

} // end namespace dtrans
} // end namespace llvm

namespace {
class DTransSOAToAOSPrepareWrapper : public ModulePass {
private:
  dtrans::SOAToAOSPreparePass Impl;

public:
  static char ID;

  DTransSOAToAOSPrepareWrapper() : ModulePass(ID) {
    initializeDTransSOAToAOSPrepareWrapperPass(
        *PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;

    auto &WP = getAnalysis<WholeProgramWrapperPass>().getResult();
    if (!WP.isWholeProgramSafe())
      return false;

    auto &DTAnalysisWrapper = getAnalysis<DTransAnalysisWrapper>();
    DTransAnalysisInfo &DTInfo = DTAnalysisWrapper.getDTransInfo(M);
    if (!DTInfo.useDTransAnalysis())
      return false;

    bool Changed = Impl.runImpl(M, DTInfo, WP);
    if (Changed)
      DTAnalysisWrapper.setInvalidated();
    return Changed;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DTransAnalysisWrapper>();
    AU.addRequired<TargetLibraryInfoWrapperPass>();
    AU.addRequired<WholeProgramWrapperPass>();
    AU.addPreserved<WholeProgramWrapperPass>();
  }
};

} // namespace

char DTransSOAToAOSPrepareWrapper::ID = 0;
INITIALIZE_PASS_BEGIN(DTransSOAToAOSPrepareWrapper, "dtrans-soatoaos-prepare",
                      "DTrans soatoaos prepare", false, false)
INITIALIZE_PASS_DEPENDENCY(DTransAnalysisWrapper)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_END(DTransSOAToAOSPrepareWrapper, "dtrans-soatoaos-prepare",
                    "DTrans soatoaos prepare", false, false)

ModulePass *llvm::createDTransSOAToAOSPrepareWrapperPass() {
  return new DTransSOAToAOSPrepareWrapper();
}
