//===---------------- SOAToAOS.cpp - SOAToAOSPass -------------------------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the DTrans Structure of Arrays to Array of Structures
// data layout optimization pass.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/SOAToAOS.h"
#include "Intel_DTrans/Analysis/DTrans.h"
#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "Intel_DTrans/DTransCommon.h"
#include "Intel_DTrans/Transforms/DTransOptBase.h"

#include "SOAToAOSArrays.h"
#include "SOAToAOSEffects.h"
#include "SOAToAOSStruct.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Utils/FunctionComparator.h"

#define DEBUG_TYPE "dtrans-soatoaos"

namespace {
using namespace llvm;
using namespace dtrans;
using namespace soatoaos;

// Global guard to enable/disable transformation.
static cl::opt<bool>
    DTransSOAToAOSGlobalGuard("enable-dtrans-soatoaos", cl::init(false),
                              cl::Hidden, cl::desc("Enable DTrans SOAToAOS"));

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
static cl::opt<std::string> DTransSOAToAOSType("dtrans-soatoaos-typename",
                                               cl::ReallyHidden);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

class SOAToAOSTransformImpl : public DTransOptBase {
public:
  SOAToAOSTransformImpl(DTransAnalysisInfo &DTInfo, LLVMContext &Context,
                        const DataLayout &DL, const TargetLibraryInfo &TLI,
                        StringRef DepTypePrefix,
                        DTransTypeRemapper *TypeRemapper)
      : DTransOptBase(DTInfo, Context, DL, TLI, DepTypePrefix, TypeRemapper) {}

  ~SOAToAOSTransformImpl() {
    for (auto *Cand : Candidates) {
      delete Cand;
    }
    Candidates.clear();
  }

private:
  SOAToAOSTransformImpl(const SOAToAOSTransformImpl &) = delete;
  SOAToAOSTransformImpl &operator=(const SOAToAOSTransformImpl &) = delete;

  bool prepareTypes(Module &M) override;
  void populateTypes(Module &M) override;

  // Relatively heavy weight checks to classify methods, see MethodKind.
  // Has internal memory handling for DepMap:
  // copying is forbidden.
  //
  class CandidateSideEffectsInfo : public SOAToAOSCFGInfo, DepMap {
  public:
    // Computes dependencies using DepMap.
    bool populateSideEffects(SOAToAOSTransformImpl &Impl, Module &M);

  protected:
    CandidateSideEffectsInfo() {}

    struct CombinedCallSiteInfo : public CallSiteComparator::CallSitesInfo {
      // Calls are from append-like method, no need for special processing.
      // Comparison is done using FunctionComparator.
      CallSiteComparator::FunctionSet Reallocs;
    };
    CombinedCallSiteInfo CSInfo;


  private:
    // Compare call sites of methods to be combined.
    bool compareCallSites(Function *F1, Function* F2, MethodKind MK) const;

    CandidateSideEffectsInfo(const CandidateSideEffectsInfo &) = delete;
    CandidateSideEffectsInfo &
    operator=(const CandidateSideEffectsInfo &) = delete;

  };

  class CandidateInfo : public CandidateSideEffectsInfo {
  public:
    void setCandidateStructs(SOAToAOSTransformImpl &Impl,
                             dtrans::StructInfo *S) {
      StructsToConvert.push_back(S);

      unsigned I = 0;
      for (auto *OrigTy : fields()) {
        I++;
        StructsToConvert.push_back(
            cast<dtrans::StructInfo>(Impl.DTInfo.getTypeInfo(OrigTy)));
      }
    }

    // FIXME: Place holder for experiments.
    void prepareTypes(SOAToAOSTransformImpl &Impl, Module &M) {
      for (auto &S : StructsToConvert) {
        auto *OrigTy = cast<StructType>(S->getLLVMType());

        StructType *NewStructTy = StructType::create(
            Impl.Context, (Impl.DepTypePrefix + OrigTy->getName()).str());
        Impl.TypeRemapper->addTypeMapping(OrigTy, NewStructTy);
        Impl.OrigToNewTypeMapping[OrigTy] = NewStructTy;
      }
    }

    // FIXME: Place holder for experiments.
    void populateTypes(SOAToAOSTransformImpl &Impl, Module &M) {
      for (auto &Elem : Impl.OrigToNewTypeMapping) {
        SmallVector<Type *, 8> DataTypes;
        auto NumFields = cast<StructType>(Elem.first)->getNumElements();
        for (size_t i = 0; i < NumFields; ++i) {
          DataTypes.push_back(Impl.TypeRemapper->remapType(
              cast<StructType>(Elem.first)->getElementType(i)));
        }

        cast<StructType>(Elem.second)
            ->setBody(DataTypes, cast<StructType>(Elem.first)->isPacked());
      }
    }

    dtrans::StructInfo *getOuterStruct() const { return StructsToConvert[0]; }

    CandidateInfo() {}

  private:
    CandidateInfo(const CandidateInfo &) = delete;
    CandidateInfo &operator=(const CandidateInfo &) = delete;
    SmallVector<dtrans::StructInfo *, 3> StructsToConvert;
  };

  constexpr static int MaxNumStructCandidates = 1;

  SmallVector<CandidateInfo *, MaxNumStructCandidates> Candidates;

  // A mapping from the original structure type to the new structure type
  TypeToTypeMap OrigToNewTypeMapping;
};

// Hook point. Top-level returns from populate* methods.
inline bool FALSE() { return false; }

bool SOAToAOSTransformImpl::CandidateSideEffectsInfo::populateSideEffects(
    SOAToAOSTransformImpl &Impl, Module &M) {

  for (auto Pair : zip_first(methodsets(), fields()))
    for (auto *F : *std::get<0>(Pair)) {
      DepCompute DC(Impl.DTInfo, Impl.DL, Impl.TLI, F, std::get<1>(Pair),
                    // *this as DepMap to fill.
                    *this);

      bool Result = DC.computeDepApproximation();
      LLVM_DEBUG(dbgs() << "; Dep approximation for array's method "
                        << F->getName()
                        << (Result ? " is successful\n" : " incomplete\n"));
      DEBUG_WITH_TYPE(DTRANS_SOADEP, {
        dbgs() << "; Dump computed dependencies ";
        DepMap::DepAnnotatedWriter Annotate(*this);
        F->print(dbgs(), &Annotate);
      });
      if (!Result)
        return FALSE();
    }

  for (auto *F : StructMethods) {
    DepCompute DC(Impl.DTInfo, Impl.DL, Impl.TLI, F, Struct, *this);

    bool Result = DC.computeDepApproximation();
    LLVM_DEBUG(dbgs() << "; Dep approximation for struct's method "
                      << F->getName()
                      << (Result ? " is successful\n" : " incomplete\n"));
    DEBUG_WITH_TYPE(DTRANS_SOADEP, {
      dbgs() << "; Dump computed dependencies ";

      DepMap::DepAnnotatedWriter Annotate(*this);
      F->print(dbgs(), &Annotate);
    });

    if (!Result)
      return FALSE();
  }

  // TODO: store results somewhere for transformation.
  ComputeArrayMethodClassification::TransformationData Data;
  unsigned Cnt = 0;
  bool UnknownSeen = false;
  for (auto Tuple :
       zip_first(methodsets(), fields(), elements(), ArrayFieldOffsets)) {
    ++Cnt;

    const Function *MethodsCalled = nullptr;
    for (auto *F : *std::get<0>(Tuple)) {
      ArraySummaryForIdiom S(std::get<1>(Tuple), std::get<2>(Tuple),
                             MemoryInterface, F);
      LLVM_DEBUG(dbgs() << "; Checking array's method " << F->getName()
                        << "\n");

      ComputeArrayMethodClassification MC(Impl.DL,
                                          // *this as DepMap to query.
                                          *this, S,
                                          // Info for transformation
                                          Data);
      auto Res = MC.classify();
      auto Kind = Res.first;

      LLVM_DEBUG(dbgs() << "; Classification: " << Kind << "\n");

      if (Kind == MK_Unknown)
        UnknownSeen = true;

      if (UnknownSeen && !DTransSOAToAOSComputeAllDep)
        return FALSE();

      bool ToCombine = true;
      if (Kind == MK_Ctor)
        CSInfo.Ctors.push_back(F);
      else if (Kind == MK_CCtor)
        CSInfo.CCtors.push_back(F);
      else if (Kind == MK_Dtor)
        CSInfo.Dtors.push_back(F);
      else if (Kind == MK_Append)
        CSInfo.Appends.push_back(F);
      else if (Kind == MK_Realloc)
        CSInfo.Reallocs.push_back(F);
      else
        ToCombine = false;

      if (ToCombine) {
        if (!F->hasOneUse())
          return FALSE();
        if (!ImmutableCallSite(F->use_begin()->getUser()))
          return FALSE();
      }

      // Simple processing of MK_Append calling MK_Realloc.
      if (Res.second) {
        if (Kind != MK_Append)
          return FALSE();
        if (MethodsCalled)
          return FALSE();
        MethodsCalled = Res.second;
      }
    }

    if (!Cnt)
      return FALSE();

    if (CSInfo.Ctors.size() != Cnt || CSInfo.CCtors.size() != Cnt ||
        CSInfo.Dtors.size() != Cnt || CSInfo.Appends.size() != Cnt ||
        CSInfo.Reallocs.size() != Cnt)
      return FALSE();

    // Simple processing of MK_Append.
    // As direct calls to MK_Realloc is not tested, forbid it, MK_Realloc is
    // called only from MK_Append.
    if (CSInfo.Reallocs.back() != MethodsCalled)
      return FALSE();
  }

  if (UnknownSeen) {
    assert(DTransSOAToAOSComputeAllDep &&
           "MK_Unknown methods encountered too late");
    return FALSE();
  }

  // Check combined methods.
  SmallVector<const Function *, 5> Pivot;
  // realloc-like should be compared first.
  Pivot.push_back(CSInfo.Reallocs[0]);
  Pivot.push_back(CSInfo.CCtors[0]);
  Pivot.push_back(CSInfo.Ctors[0]);
  Pivot.push_back(CSInfo.Dtors[0]);
  Pivot.push_back(CSInfo.Appends[0]);

  assert(CSInfo.Reallocs.size() == ArrayFieldOffsets.size() &&
         "There should be as many realloc-like methods as array structs");

  for (unsigned I = 1, E = CSInfo.Reallocs.size(); I != E; ++I) {
    SmallVector<const Function *, 5> Running;
    Running.push_back(CSInfo.Reallocs[I]);
    Running.push_back(CSInfo.CCtors[I]);
    Running.push_back(CSInfo.Ctors[I]);
    Running.push_back(CSInfo.Dtors[I]);
    Running.push_back(CSInfo.Appends[I]);
    assert(Running.size() == Pivot.size() &&
           "Missing checks in array method classification");

    GlobalNumberState GNS;
    for (auto Pair : zip(Pivot, Running)) {
      auto *F = std::get<0>(Pair);
      auto *O = std::get<1>(Pair);
      FunctionComparator CmpFunc(F, O, &GNS);
      if (CmpFunc.compare() == 0) {
        LLVM_DEBUG(dbgs() << "; Comparison of " << F->getName() << " and "
                          << O->getName() << " showed bit-to-bit equality.\n");
        // GNS keeps pointers to modifiable values in a map.
        GNS.setEqual(const_cast<Function *>(F), const_cast<Function *>(O));
      } else {
        LLVM_DEBUG(dbgs() << "; Comparison of " << F->getName() << " and "
                          << O->getName() << " showed some differences, "
                          << "this situation cannot be handled in SOAToAOS.\n");
        return FALSE();
      }
    }
  }

  SmallVector<StructType *, MaxNumFieldCandidates> Arrays;
  for (auto *ArrTy : fields())
    Arrays.push_back(ArrTy);

  for (auto *F : StructMethods) {
    SummaryForIdiom S(Struct, MemoryInterface, F);

    // TODO: store results somewhere for transformation.
    // FIXME: Make TI per-module and not per-function.
    StructureMethodAnalysis::TransformationData TI;
    // TODO: make order of arguments consistent.
    // TODO: add diagnostic messages.
    StructureMethodAnalysis MChecker(Impl.DL, Impl.DTInfo, Impl.TLI, *this, S,
                                     Arrays, TI);
    bool CheckResult = MChecker.checkStructMethod();
    LLVM_DEBUG(dbgs() << "; Struct's method " << F->getName()
                      << (CheckResult ? " has only expected side-effects\n"
                                      : " needs analysis of instructions\n"));
    if (!CheckResult)
      return FALSE();

    CallSiteComparator CSCmp(Impl.DL, Impl.DTInfo, Impl.TLI, *this, S, Arrays,
                             ArrayFieldOffsets, TI, CSInfo,
                             BasePointerOffset);
    bool Comparison = CSCmp.canCallSitesBeMerged();
    LLVM_DEBUG(dbgs() << "; Array call sites analysis result: "
                      << (Comparison
                              ? "required call sites can be merged"
                              : "problem with call sites required to be merged")
                      << " in " << F->getName() << "\n");
    if (!Comparison)
      return FALSE();
  }

  return true;
}

// FIXME: make sure padding fields are dead.
bool SOAToAOSTransformImpl::prepareTypes(Module &M) {

  for (dtrans::TypeInfo *TI : DTInfo.type_info_entries()) {
    std::unique_ptr<CandidateInfo> Info(new CandidateInfo());

    if (!Info->populateLayoutInformation(TI->getLLVMType())) {
      LLVM_DEBUG({
        dbgs() << "  ; Rejecting ";
        TI->getLLVMType()->print(dbgs(), true, true);
        dbgs() << " because it does not look like a candidate structurally.\n";
      });
      continue;
    }

    // FIXME: FP exception handling.
    if (!Info->checkCFG(this->DTInfo) || !Info->populateCFGInformation(M)) {
      LLVM_DEBUG({
        dbgs() << "  ; Rejecting ";
        TI->getLLVMType()->print(dbgs(), true, true);
        dbgs() << " because it does not look like a candidate from CFG "
                  "analysis.\n";
      });
      continue;
    }

    if (!Info->populateSideEffects(*this, M)) {
      LLVM_DEBUG({
        dbgs() << "  ; Rejecting ";
        TI->getLLVMType()->print(dbgs(), true, true);
        dbgs() << " because some methods contains unknown side effect.\n";
      });
      continue;
    }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    if (!DTransSOAToAOSType.empty() &&
        DTransSOAToAOSType != cast<StructType>(TI->getLLVMType())->getName()) {
      LLVM_DEBUG({
        dbgs() << "  ; Rejecting ";
        TI->getLLVMType()->print(dbgs(), true, true);
        dbgs() << " based on dtrans-soatoaos-typename option.\n";
      });
      return false;
    }
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

    if (Candidates.size() > MaxNumStructCandidates) {
      LLVM_DEBUG(dbgs() << "  Too many candidates found. Give-up.\n");
      return false;
    }

    Info->setCandidateStructs(*this, cast<dtrans::StructInfo>(TI));
    Candidates.push_back(Info.release());
  }

  if (Candidates.empty()) {
    LLVM_DEBUG(dbgs() << "  No candidates found.\n");
    return false;
  }

  for_each(Candidates,
           [this, &M](CandidateInfo *C) { C->prepareTypes(*this, M); });

  return true;
}

void SOAToAOSTransformImpl::populateTypes(Module &M) {
  for_each(Candidates,
           [this, &M](CandidateInfo *C) { C->populateTypes(*this, M); });
}
} // namespace

namespace llvm {
namespace dtrans {

bool SOAToAOSPass::runImpl(Module &M, DTransAnalysisInfo &DTInfo,
                           const TargetLibraryInfo &TLI) {
  // Perform the actual transformation.
  DTransTypeRemapper TypeRemapper;
  SOAToAOSTransformImpl Transformer(DTInfo, M.getContext(), M.getDataLayout(),
                                    TLI, "__SOADT_", &TypeRemapper);
  return Transformer.run(M);
}

PreservedAnalyses SOAToAOSPass::run(Module &M, ModuleAnalysisManager &AM) {
  if (!DTransSOAToAOSGlobalGuard)
    return PreservedAnalyses::all();

  auto &WP = AM.getResult<WholeProgramAnalysis>(M);
  if (!WP.isWholeProgramSafe())
    return PreservedAnalyses::all();

  auto &DTransInfo = AM.getResult<DTransAnalysis>(M);
  auto &TLI = AM.getResult<TargetLibraryAnalysis>(M);
  bool Changed = runImpl(M, DTransInfo, TLI);

  if (!Changed)
    return PreservedAnalyses::all();

  // TODO: Mark the actual preserved analyses.
  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}

} // end namespace dtrans
} // end namespace llvm

namespace {
class DTransSOAToAOSWrapper : public ModulePass {
private:
  dtrans::SOAToAOSPass Impl;

public:
  static char ID;

  DTransSOAToAOSWrapper() : ModulePass(ID) {
    initializeDTransSOAToAOSWrapperPass(*PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M) || !DTransSOAToAOSGlobalGuard)
      return false;

    auto &WP = getAnalysis<WholeProgramWrapperPass>().getResult();
    if (!WP.isWholeProgramSafe())
      return false;

    auto &DTInfo = getAnalysis<DTransAnalysisWrapper>().getDTransInfo();
    if (!DTInfo.useDTransAnalysis())
      return false;

    auto &TLI = getAnalysis<TargetLibraryInfoWrapperPass>().getTLI();

    return Impl.runImpl(M, DTInfo, TLI);
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DTransAnalysisWrapper>();
    AU.addRequired<TargetLibraryInfoWrapperPass>();
    AU.addRequired<WholeProgramWrapperPass>();
    // TODO: Mark the actual preserved analyses.
    AU.addPreserved<WholeProgramWrapperPass>();
  }
};

} // namespace

char DTransSOAToAOSWrapper::ID = 0;
INITIALIZE_PASS_BEGIN(DTransSOAToAOSWrapper, "dtrans-soatoaos",
                      "DTrans struct of arrays to array of structs", false,
                      false)
INITIALIZE_PASS_DEPENDENCY(DTransAnalysisWrapper)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_END(DTransSOAToAOSWrapper, "dtrans-soatoaos",
                    "DTrans struct of arrays to array of structs", false, false)

ModulePass *llvm::createDTransSOAToAOSWrapperPass() {
  return new DTransSOAToAOSWrapper();
}
