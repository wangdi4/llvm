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

#include "SOAToAOSClassInfo.h"

#include "llvm/Analysis/Intel_WP.h"

#define DTRANS_SOATOAOSPREPARE "dtrans-soatoaos-prepare"

using namespace llvm;
using namespace dtrans;
using namespace soatoaos;

namespace llvm {
namespace dtrans {
namespace soatoaos {

class SOAToAOSPrepCandidateInfo {
public:
  SOAToAOSPrepCandidateInfo(Module &M, const DataLayout &DL,
                            DTransAnalysisInfo &DTInfo, MemGetTLITy GetTLI,
                            MemInitDominatorTreeType GetDT)
      : M(M), DL(DL), DTInfo(DTInfo), GetTLI(GetTLI), GetDT(GetDT){};

  ~SOAToAOSPrepCandidateInfo() {
    if (CandI)
      delete CandI;
    if (ClassI)
      delete ClassI;
  }
  bool isCandidateField(Type *, unsigned);
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void printCandidateInfo();
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

private:
  Module &M;
  const DataLayout &DL;
  DTransAnalysisInfo &DTInfo;
  MemGetTLITy GetTLI;
  MemInitDominatorTreeType GetDT;

  // ClassInfo for the candidate field class.
  ClassInfo *ClassI = nullptr;

  // Info of candidate Struct.
  MemInitCandidateInfo *CandI = nullptr;

  // Candidate field class which is derived from BaseTy.
  StructType *DerivedTy = nullptr;

  // Base type of candidate field class.
  StructType *BaseTy = nullptr;
};

// This routine analyzes that field of Ty at Offset is potential candidate
// vector class.
//  Ex: %class.refvector will be considered as candidate.
//    %class.refvector = type { %class.basevector }
//    %class.basevector = type {
//       Vtable *, i8, i32, i32, %class.elem2**, MemoryManager* }
//
bool SOAToAOSPrepCandidateInfo::isCandidateField(Type *Ty, unsigned Offset) {

  std::unique_ptr<MemInitCandidateInfo> CandD(new MemInitCandidateInfo());

  // Check if it is a candidate field.
  Type *DTy = CandD->isSimpleDerivedVectorType(Ty, Offset);
  if (!DTy)
    return false;
  DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE, {
    dbgs() << "  SOAToAOSPrepare: Candidate selected for more analysis\n";
    dbgs() << "    Candidate struct: " << getStructName(CandD->getStructTy());
    dbgs() << "    FieldOff: " << Offset << "\n";
  });

  // Check if member functions are okay.
  if (!CandD->collectMemberFunctions(M))
    return false;

  // Collect Derived and Base types.
  CandI = CandD.release();
  Type *BTy = getMemInitSimpleBaseType(DTy);
  assert(BTy && "Unexpected Base Type");
  DerivedTy = dyn_cast<StructType>(DTy);
  BaseTy = dyn_cast<StructType>(BTy);
  assert(DerivedTy && BaseTy && "Unexpected Derived and Base Types");

  // Analyze member functions to make sure it is vector class.
  std::unique_ptr<ClassInfo> ClassD(
      new ClassInfo(DL, DTInfo, GetTLI, GetDT, CandI, Offset, false));
  if (!ClassD->analyzeClassFunctions()) {
    DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE, {
      dbgs() << "  Candidate failed after functionality analysis.\n";
    });
    return false;
  }

  ClassI = ClassD.release();
  return true;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void SOAToAOSPrepCandidateInfo::printCandidateInfo() {
  CandI->printCandidateInfo();
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

class SOAToAOSPrepareImpl {
public:
  SOAToAOSPrepareImpl(Module &M, const DataLayout &DL,
                      DTransAnalysisInfo &DTInfo, MemGetTLITy GetTLI,
                      MemInitDominatorTreeType GetDT)
      : M(M), DL(DL), DTInfo(DTInfo), GetTLI(GetTLI), GetDT(GetDT) {}

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
  const DataLayout &DL;
  DTransAnalysisInfo &DTInfo;
  MemGetTLITy GetTLI;
  MemInitDominatorTreeType GetDT;
  SmallPtrSet<SOAToAOSPrepCandidateInfo *, MaxNumCandidates> Candidates;

  bool gatherCandidateInfo(void);
};

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

    std::unique_ptr<SOAToAOSPrepCandidateInfo> Candidate(
        new SOAToAOSPrepCandidateInfo(M, DL, DTInfo, GetTLI, GetDT));

    if (!Candidate->isCandidateField(Ty, *ArrOffsetIt))
      continue;

    Candidates.insert(Candidate.release());
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
    dbgs() << "    Candidate Passed Analysis.\n";
    auto *CandI = *Candidates.begin();
    CandI->printCandidateInfo();
  });

  // TODO: Add more code here.
  return true;
}

} // end namespace soatoaos

bool SOAToAOSPreparePass::runImpl(Module &M, DTransAnalysisInfo &DTInfo,
                                  MemGetTLITy GetTLI, WholeProgramInfo &WPInfo,
                                  dtrans::MemInitDominatorTreeType &GetDT) {
  auto TTIAVX2 = TargetTransformInfo::AdvancedOptLevel::AO_TargetHasAVX2;
  if (!WPInfo.isWholeProgramSafe() || !WPInfo.isAdvancedOptEnabled(TTIAVX2))
    return false;

  if (!DTInfo.useDTransAnalysis())
    return false;

  auto &DL = M.getDataLayout();

  SOAToAOSPrepareImpl PrepareImpl(M, DL, DTInfo, GetTLI, GetDT);

  return PrepareImpl.run();
}

PreservedAnalyses SOAToAOSPreparePass::run(Module &M,
                                           ModuleAnalysisManager &AM) {
  auto &WP = AM.getResult<WholeProgramAnalysis>(M);
  auto &DTransInfo = AM.getResult<DTransAnalysis>(M);
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  MemInitDominatorTreeType GetDT = [&FAM](Function &F) -> DominatorTree & {
    return FAM.getResult<DominatorTreeAnalysis>(F);
  };
  auto GetTLI = [&FAM](const Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(*(const_cast<Function *>(&F)));
  };

  bool Changed = runImpl(M, DTransInfo, GetTLI, WP, GetDT);

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

    auto &DTAnalysisWrapper = getAnalysis<DTransAnalysisWrapper>();
    DTransAnalysisInfo &DTInfo = DTAnalysisWrapper.getDTransInfo(M);
    dtrans::MemInitDominatorTreeType GetDT =
        [this](Function &F) -> DominatorTree & {
      return this->getAnalysis<DominatorTreeWrapperPass>(F).getDomTree();
    };
    auto GetTLI = [this](const Function &F) -> const TargetLibraryInfo & {
      return this->getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
    };

    bool Changed =
        Impl.runImpl(M, DTInfo, GetTLI,
                     getAnalysis<WholeProgramWrapperPass>().getResult(), GetDT);
    if (Changed)
      DTAnalysisWrapper.setInvalidated();
    return Changed;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DTransAnalysisWrapper>();
    AU.addRequired<TargetLibraryInfoWrapperPass>();
    AU.addRequired<DominatorTreeWrapperPass>();
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
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_END(DTransSOAToAOSPrepareWrapper, "dtrans-soatoaos-prepare",
                    "DTrans soatoaos prepare", false, false)

ModulePass *llvm::createDTransSOAToAOSPrepareWrapperPass() {
  return new DTransSOAToAOSPrepareWrapper();
}
