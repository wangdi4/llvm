//===-------------- MemInitTrimDownOP.cpp - DTransMemInitTrimDownOPPass ---===//
//
// Copyright (C) 2021-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the DTrans Initial Memory Allocation Trim Down
// optimization pass.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/MemInitTrimDownOP.h"
#include "Intel_DTrans/Analysis/DTrans.h"
#include "Intel_DTrans/Analysis/DTransSafetyAnalyzer.h"
#include "Intel_DTrans/DTransCommon.h"

#include "SOAToAOSOPClassInfo.h"

#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/Module.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/Utils/Local.h"

using namespace llvm;
using namespace dtransOP;

#define DTRANS_MEMINITTRIMDOWNOP "dtrans-meminittrimdownop"

// This option is used for testing.
cl::opt<bool>
    DTransMemInitOPRecognizeAll("dtrans-meminitop-recognize-all",
                                cl::init(false), cl::Hidden,
                                cl::desc("Recognize All member functions"));

namespace {

class DTransMemInitTrimDownOPWrapper : public ModulePass {
private:
  dtransOP::MemInitTrimDownOPPass Impl;

public:
  static char ID;

  DTransMemInitTrimDownOPWrapper() : ModulePass(ID) {
    initializeDTransMemInitTrimDownOPWrapperPass(
        *PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;
    auto &DTAnalysisWrapper = getAnalysis<DTransSafetyAnalyzerWrapper>();
    DTransSafetyInfo &DTInfo = DTAnalysisWrapper.getDTransSafetyInfo(M);

    SOADominatorTreeType GetDT = [this](Function &F) -> DominatorTree & {
      return this->getAnalysis<DominatorTreeWrapperPass>(F).getDomTree();
    };
    auto GetTLI = [this](const Function &F) -> const TargetLibraryInfo & {
      return this->getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
    };
    bool Changed =
        Impl.runImpl(M, DTInfo, GetTLI,
                     getAnalysis<WholeProgramWrapperPass>().getResult(), GetDT);
    return Changed;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DTransSafetyAnalyzerWrapper>();
    AU.addRequired<TargetLibraryInfoWrapperPass>();
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<WholeProgramWrapperPass>();
    AU.addPreserved<DTransSafetyAnalyzerWrapper>();
    AU.addPreserved<WholeProgramWrapperPass>();
  }
};

} // end anonymous namespace

char DTransMemInitTrimDownOPWrapper::ID = 0;
INITIALIZE_PASS_BEGIN(DTransMemInitTrimDownOPWrapper,
                      "dtrans-meminittrimdownop",
                      "DTrans Mem Init Trim Down OP", false, false)
INITIALIZE_PASS_DEPENDENCY(DTransSafetyAnalyzerWrapper)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_END(DTransMemInitTrimDownOPWrapper, "dtrans-meminittrimdownop",
                    "DTrans Mem Init Trim Down OP", false, false)

ModulePass *llvm::createDTransMemInitTrimDownOPWrapperPass() {
  return new DTransMemInitTrimDownOPWrapper();
}

namespace llvm {

namespace dtransOP {

class MemInitClassInfo : public ClassInfo {
public:
  MemInitClassInfo(const DataLayout &DL, DTransSafetyInfo &DTInfo,
                   SOAGetTLITy GetTLI, SOADominatorTreeType GetDT,
                   SOACandidateInfo *SOACInfo, int32_t FieldIdx,
                   bool RecognizeAll)
      : ClassInfo(DL, DTInfo, GetTLI, GetDT, SOACInfo, FieldIdx,
                  RecognizeAll){};
};

class MemInitTrimDownImpl {

public:
  MemInitTrimDownImpl(Module &M, const DataLayout &DL, DTransSafetyInfo &DTInfo,
                      SOAGetTLITy GetTLI, SOADominatorTreeType GetDT)
      : M(M), DL(DL), DTInfo(DTInfo), GetTLI(GetTLI), GetDT(GetDT){};

  ~MemInitTrimDownImpl() {
    for (auto *CInfo : Candidates)
      delete CInfo;
    for (auto *CInfo : ClassInfoSet)
      delete CInfo;
  }
  bool run(void);

private:
  Module &M;
  const DataLayout &DL;
  DTransSafetyInfo &DTInfo;
  SOAGetTLITy GetTLI;
  SOADominatorTreeType GetDT;

  constexpr static int MaxNumCandidates = 1;
  SmallVector<SOACandidateInfo *, MaxNumCandidates> Candidates;

  // Collection of ClassInfo. It will used for more analysis and
  // transformation.
  SmallPtrSet<MemInitClassInfo *, 4> ClassInfoSet;

  bool gatherCandidateInfo(void);
  bool analyzeCandidate(SOACandidateInfo *);
};

// Analyze functionality of each member function of candidate
// field classes to prove that the classes are vector classes.
bool MemInitTrimDownImpl::analyzeCandidate(SOACandidateInfo *Cand) {
  for (auto Loc : Cand->candidate_fields()) {
    std::unique_ptr<MemInitClassInfo> ClassI(new MemInitClassInfo(
        DL, DTInfo, GetTLI, GetDT, Cand, Loc, DTransMemInitOPRecognizeAll));
    if (!ClassI->analyzeClassFunctions())
      return false;
    // Continue checking remaining candidate fields if
    // DTransMemInitOPRecognizeAll is true.
    if (DTransMemInitOPRecognizeAll)
      continue;
    ClassInfoSet.insert(ClassI.release());
  }
  // Skip further analysis if DTransMemInitOPRecognizeAll is true.
  if (DTransMemInitOPRecognizeAll)
    return false;
  return true;
}

bool MemInitTrimDownImpl::gatherCandidateInfo(void) {

  DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWNOP, {
    dbgs() << "MemInitTrimDown transformation:";
    dbgs() << "\n";
  });
  // TODO: Since MemInitTrimDown now runs before SOAToAOS, we can't limit
  // MemInitTrimDown transformation to only the structs that are transformed
  // by SOAToAOS.
  // Add more checks that are done by SOAToAOS like populateCFGInformation,
  // populateSideEffects etc to limit the number of possible candidates (
  // also to be on the safe side).
  //
  for (dtrans::TypeInfo *TI : DTInfo.type_info_entries()) {
    std::unique_ptr<SOACandidateInfo> CInfo(
        new SOACandidateInfo(DTInfo.getTypeMetadataReader()));

    auto *StInfo = dyn_cast<dtrans::StructInfo>(TI);
    if (!StInfo)
      continue;

    if (!CInfo->isCandidateType(StInfo->getDTransType()))
      continue;

    DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWNOP, {
      dbgs() << "  Considering candidate: ";
      TI->getLLVMType()->print(dbgs(), true, true);
      dbgs() << "\n";
    });

    if (DTInfo.testSafetyData(StInfo, dtrans::DT_MemInitTrimDown)) {
      DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWNOP, {
        dbgs() << "  Failed: safety test for candidate struct.\n";
      });
      continue;
    }
    // Check SafetyData for candidate field array structs.
    bool FieldSafetyCheck = true;
    for (auto Loc : CInfo->candidate_fields()) {
      auto *FInfo = DTInfo.getTypeInfo(CInfo->getFieldElemTy(Loc));
      if (!FInfo || DTInfo.testSafetyData(FInfo, dtrans::DT_MemInitTrimDown)) {
        FieldSafetyCheck = false;
        break;
      }
    }
    if (!FieldSafetyCheck) {
      DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWNOP, {
        dbgs() << "  Failed: safety test for field array struct.\n";
      });
      continue;
    }

    if (!CInfo->collectMemberFunctions(M)) {
      DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWNOP,
                      { dbgs() << "  Failed: member function collection.\n"; });
      continue;
    }

    if (Candidates.size() >= MaxNumCandidates) {
      DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWNOP, {
        dbgs() << "  Failed: Exceeding maximum candidate limit.\n";
      });
      return false;
    }
    Candidates.push_back(CInfo.release());
  }
  if (Candidates.empty()) {
    DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWNOP,
                    { dbgs() << "  Failed: No candidates found.\n"; });
    return false;
  }
  DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWNOP, {
    dbgs() << "  Possible candidate structs: \n";
    for (auto *CInfo : Candidates)
      CInfo->printCandidateInfo();
  });
  return true;
}

bool MemInitTrimDownImpl::run(void) {

  if (!gatherCandidateInfo())
    return false;

  // Analyze member functions of candidate classes to prove
  // that functionality matches with usual vector class.
  SmallVector<SOACandidateInfo *, MaxNumCandidates> ValidCandidates;
  for (auto *Cand : Candidates)
    if (analyzeCandidate(Cand))
      ValidCandidates.push_back(Cand);
  std::swap(Candidates, ValidCandidates);

  if (Candidates.empty()) {
    DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWNOP, {
      dbgs() << "  Failed: No candidates after functionality analysis.\n";
    });
    return false;
  }
  return false;
}

bool MemInitTrimDownOPPass::runImpl(Module &M, DTransSafetyInfo &DTInfo,
                                    SOAGetTLITy GetTLI,
                                    WholeProgramInfo &WPInfo,
                                    SOADominatorTreeType &GetDT) {

  auto TTIAVX2 = TargetTransformInfo::AdvancedOptLevel::AO_TargetHasIntelAVX2;
  if (!WPInfo.isWholeProgramSafe() || !WPInfo.isAdvancedOptEnabled(TTIAVX2))
    return false;

  if (!DTInfo.useDTransSafetyAnalysis())
    return false;

  auto &DL = M.getDataLayout();

  MemInitTrimDownImpl MemInitTrimDownI(M, DL, DTInfo, GetTLI, GetDT);
  return MemInitTrimDownI.run();
}

PreservedAnalyses MemInitTrimDownOPPass::run(Module &M,
                                             ModuleAnalysisManager &AM) {
  auto &DTransInfo = AM.getResult<DTransSafetyAnalyzer>(M);
  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  SOADominatorTreeType GetDT = [&FAM](Function &F) -> DominatorTree & {
    return FAM.getResult<DominatorTreeAnalysis>(F);
  };
  auto GetTLI = [&FAM](const Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(*(const_cast<Function *>(&F)));
  };

  if (!runImpl(M, DTransInfo, GetTLI, WPInfo, GetDT))
    return PreservedAnalyses::all();

  // TODO: Mark the actual preserved analyses.
  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  PA.preserve<DTransSafetyAnalyzer>();
  return PA;
}

} // namespace dtransOP

} // namespace llvm
