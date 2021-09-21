//===---------------- SOAToAOSOP.cpp - SOAToAOSOPPass ---------------------===//
//
// Copyright (C) 2021-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the DTrans Structure of Arrays to Array of Structures
// data layout optimization pass for IR with either opaque pointers or
// non-opaque pointers.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/SOAToAOSOP.h"

#include "Intel_DTrans/Analysis/DTransSafetyAnalyzer.h"
#include "Intel_DTrans/Analysis/PtrTypeAnalyzer.h"
#include "Intel_DTrans/DTransCommon.h"
#include "Intel_DTrans/Transforms/DTransOPOptBase.h"
#include "Intel_DTrans/Transforms/SOAToAOSOPExternal.h"
#include "SOAToAOSOPEffects.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/Module.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"

#define DEBUG_TYPE "dtrans-soatoaosop"

namespace {
using namespace llvm;
using namespace dtransOP;
using namespace soatoaosOP;

// This option makes populateCFGInformation ignore
// number of uses and number of BasicBlocks in structs' and arrays' methods.
//
// It helps to debug issues in transformed code with debug prints, for
// example.
static cl::opt<bool> DTransSOAToAOSOPSizeHeuristic(
    "dtrans-soatoaosop-size-heuristic", cl::init(true), cl::Hidden,
    cl::desc("Respect size heuristic in DTrans SOAToAOS"));

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
static cl::opt<std::string> DTransSOAToAOSOPType("dtrans-soatoaosop-typename",
                                                 cl::ReallyHidden);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

class SOAToAOSOPTransformImpl : public DTransOPOptBase {
public:
  SOAToAOSOPTransformImpl(LLVMContext &Context, DTransSafetyInfo &DTInfo,
                          bool UsingOpaquePointers, StringRef DepTypePrefix)
      : DTransOPOptBase(Context, &DTInfo, UsingOpaquePointers, DepTypePrefix) {}

  ~SOAToAOSOPTransformImpl() {}

private:
  SOAToAOSOPTransformImpl(const SOAToAOSOPTransformImpl &) = delete;
  SOAToAOSOPTransformImpl &operator=(const SOAToAOSOPTransformImpl &) = delete;

  bool prepareTypes(Module &M) override;
  void populateTypes(Module &M) override;

  class CandidateSideEffectsInfo : public SOAToAOSOPCFGInfo, DepMap {
  protected:
    CandidateSideEffectsInfo() {}

  private:
    CandidateSideEffectsInfo(const CandidateSideEffectsInfo &) = delete;
    CandidateSideEffectsInfo &
    operator=(const CandidateSideEffectsInfo &) = delete;
  };

  class CandidateInfo : public CandidateSideEffectsInfo {
  public:
    CandidateInfo() {}

  private:
    CandidateInfo &operator=(const CandidateInfo &) = delete;
  };

  constexpr static int MaxNumStructCandidates = 1;
  SmallVector<CandidateInfo *, MaxNumStructCandidates> Candidates;
};

// Hook point. Top-level returns from populate* methods.
inline bool FALSE(const char *Msg) {
  LLVM_DEBUG(dbgs() << "; dtrans-soatoaosop " << Msg << "\n");
  return false;
}

bool SOAToAOSOPTransformImpl::prepareTypes(Module &M) {

  for (dtrans::TypeInfo *TI : DTInfo->type_info_entries()) {

    auto *StInfo = dyn_cast<dtrans::StructInfo>(TI);
    if (!StInfo || cast<StructType>(StInfo->getLLVMType())->isLiteral())
      continue;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    if (!DTransSOAToAOSOPType.empty() &&
        DTransSOAToAOSOPType !=
            cast<StructType>(StInfo->getLLVMType())->getName()) {
      LLVM_DEBUG({
        dbgs() << "  ; Rejecting ";
        StInfo->getLLVMType()->print(dbgs(), true, true);
        dbgs() << " based on dtrans-soatoaosop-typename option.\n";
      });
      return FALSE("conflicting -dtrans-soatoaosop-typename.");
    }
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

    std::unique_ptr<CandidateInfo> Info(new CandidateInfo());

    if (!Info->populateLayoutInformation(StInfo->getDTransType(), DTInfo)) {
      LLVM_DEBUG({
        dbgs() << "  ; Rejecting ";
        TI->getLLVMType()->print(dbgs(), true, true);
        dbgs() << " because it does not look like a candidate structurally.\n";
      });
      continue;
    }

    if (!Info->populateCFGInformation(M, DTInfo, DTransSOAToAOSOPSizeHeuristic,
                                      true)) {
      LLVM_DEBUG({
        dbgs() << "  ; Rejecting ";
        TI->getLLVMType()->print(dbgs(), true, true);
        dbgs() << " because it does not look like a candidate from CFG "
                  "analysis.\n";
      });
      continue;
    }

    // TODO: Add code here to identify candidates.

    if (Candidates.size() >= MaxNumStructCandidates) {
      return FALSE("too many candidates found.");
    }

    LLVM_DEBUG({
      dbgs() << "  ; SOA-to-AOS possible for ";
      StInfo->getLLVMType()->print(dbgs(), true, true);
      dbgs() << ".\n";
    });

    Candidates.push_back(Info.release());
  }

  if (Candidates.empty()) {
    return FALSE("no candidates found.");
  }
  return true;
}

void SOAToAOSOPTransformImpl::populateTypes(Module &M) {}

} // namespace

namespace llvm {
namespace dtransOP {

bool SOAToAOSOPPass::runImpl(Module &M, DTransSafetyInfo &DTInfo,
                             WholeProgramInfo &WPInfo) {
  auto TTIAVX2 = TargetTransformInfo::AdvancedOptLevel::AO_TargetHasIntelAVX2;
  if (!WPInfo.isWholeProgramSafe() || !WPInfo.isAdvancedOptEnabled(TTIAVX2))
    return false;

  if (!DTInfo.useDTransSafetyAnalysis()) {
    LLVM_DEBUG(dbgs() << "  DTransSafetyAnalyzer results not available\n");
    return false;
  }

  // Perform the actual transformation.
  SOAToAOSOPTransformImpl Transformer(
      M.getContext(), DTInfo, DTInfo.getPtrTypeAnalyzer().sawOpaquePointer(),
      "__SOADT_");
  return Transformer.run(M);
}

PreservedAnalyses SOAToAOSOPPass::run(Module &M, ModuleAnalysisManager &AM) {
  auto &WP = AM.getResult<WholeProgramAnalysis>(M);
  if (!WP.isWholeProgramSafe())
    return PreservedAnalyses::all();

  auto &DTransInfo = AM.getResult<DTransSafetyAnalyzer>(M);

  bool Changed = runImpl(M, DTransInfo, WP);

  if (!Changed)
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}

} // namespace dtransOP
} // end namespace llvm

namespace {
class DTransSOAToAOSOPWrapper : public ModulePass {
private:
  dtransOP::SOAToAOSOPPass Impl;

public:
  static char ID;

  DTransSOAToAOSOPWrapper() : ModulePass(ID) {
    initializeDTransSOAToAOSOPWrapperPass(*PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;

    auto &WP = getAnalysis<WholeProgramWrapperPass>().getResult();
    if (!WP.isWholeProgramSafe())
      return false;

    auto &DTAnalysisWrapper = getAnalysis<DTransSafetyAnalyzerWrapper>();
    DTransSafetyInfo &DTInfo = DTAnalysisWrapper.getDTransSafetyInfo(M);

    bool Changed = Impl.runImpl(M, DTInfo, WP);

    // TODO: Need to set setInvalidated() when Changed is true.
    return Changed;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DTransSafetyAnalyzerWrapper>();
    AU.addRequired<WholeProgramWrapperPass>();
    AU.addPreserved<WholeProgramWrapperPass>();
  }
};

} // namespace

char DTransSOAToAOSOPWrapper::ID = 0;
INITIALIZE_PASS_BEGIN(DTransSOAToAOSOPWrapper, "dtrans-soatoaosop",
                      "DTransOP struct of arrays to array of structs", false,
                      false)
INITIALIZE_PASS_DEPENDENCY(DTransSafetyAnalyzerWrapper)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_END(DTransSOAToAOSOPWrapper, "dtrans-soatoaosop",
                    "DTransOP struct of arrays to array of structs", false,
                    false)

ModulePass *llvm::createDTransSOAToAOSOPWrapperPass() {
  return new DTransSOAToAOSOPWrapper();
}
