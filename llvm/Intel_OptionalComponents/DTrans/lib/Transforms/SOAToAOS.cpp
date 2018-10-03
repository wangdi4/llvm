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

    // There should be at most one method for each MethodKind,
    // which should be combined.
    using MethodKindSetTy = SmallVector<Function *, MaxNumIntFields>;
    // Supported only MK_Last kinds of methods.
    using ClassifyMethodsTy = SmallVector<MethodKindSetTy, MK_Last + 1>;
    using MethodsBinsTy = SmallVector<ClassifyMethodsTy, MaxNumFieldCandidates>;

    // Helper class for methodbins* methods.
    template <typename IterTy, typename ClassifyMethTy>
    class MethodsKindIter
        : public iterator_adaptor_base<
              MethodsKindIter<IterTy, ClassifyMethTy>, IterTy,
              typename std::iterator_traits<IterTy>::iterator_category,
              ClassifyMethTy *,
              typename std::iterator_traits<IterTy>::difference_type,
              ClassifyMethTy **, ClassifyMethTy *> {
      using BaseTy = iterator_adaptor_base<
          MethodsKindIter<IterTy, ClassifyMethTy>, IterTy,
          typename std::iterator_traits<IterTy>::iterator_category,
          ClassifyMethTy *,
          typename std::iterator_traits<IterTy>::difference_type,
          ClassifyMethTy **, ClassifyMethTy *>;

    public:
      const CandidateSideEffectsInfo *Info;
      MethodsKindIter(const CandidateSideEffectsInfo *Info, IterTy It)
          : BaseTy(It), Info(Info) {}

      typename BaseTy::reference operator*() const { return &*this->wrapped(); }
    };

    // Updating Classifications.
    using iterator =
        MethodsKindIter<MethodsBinsTy::iterator, ClassifyMethodsTy>;

    iterator methodbins_begin() {
      return iterator(this, Classifications.begin());
    }
    iterator methodbins_end() { return iterator(this, Classifications.end()); }
    iterator_range<iterator> methodbins() {
      return make_range(methodbins_begin(), methodbins_end());
    }

    // Analysis of Classifications.
    using const_iterator =
        MethodsKindIter<MethodsBinsTy::const_iterator, const ClassifyMethodsTy>;

    const_iterator methodbins_begin() const {
      return const_iterator(this, Classifications.begin());
    }
    const_iterator methodbins_end() const {
      return const_iterator(this, Classifications.end());
    }
    iterator_range<const_iterator> methodbins() const {
      return make_range(methodbins_begin(), methodbins_end());
    }

  private:
    // Compare call sites of methods to be combined.
    bool compareCallSites(Function *F1, Function* F2, MethodKind MK) const;

    CandidateSideEffectsInfo(const CandidateSideEffectsInfo &) = delete;
    CandidateSideEffectsInfo &
    operator=(const CandidateSideEffectsInfo &) = delete;

    // It is matched by ArrayFieldsMethods,
    // can be iterated in parallel using zip_first.
    MethodsBinsTy Classifications;
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

  // Direct map from MK to set of methods.
  Classifications.assign(getNumArrays(),
                         ClassifyMethodsTy(MK_Last + 1, MethodKindSetTy()));

  // TODO: store results somewhere for transformation.
  ComputeArrayMethodClassification::TransformationData Data;
  for (auto Tuple :
       zip_first(methodsets(), fields(), elements(), methodbins())) {
    SmallVector<const Function *, 1> MethodsCalled;
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

      if (Kind == MK_Unknown && !DTransSOAToAOSComputeAllDep)
        return FALSE();

      (*std::get<3>(Tuple))[Kind].push_back(F);

      // Simple processing of MK_Append calling MK_Realloc.
      if (Res.second) {
        if (Kind != MK_Append)
          return FALSE();
        MethodsCalled.push_back(Res.second);
      }
    }
    // Simple processing of MK_Append.
    // As direct calls to MK_Realloc is not tested, forbid it, MK_Realloc is
    // called only from MK_Append.
    if (MethodsCalled.size() != 1)
      return FALSE();
    else if ((*std::get<3>(Tuple))[MK_Realloc][0] != MethodsCalled[0])
      return FALSE();
  }

  // Check combined methods.
  auto &FirstBins = **methodbins_begin();
  GlobalNumberState GNS;
  for (auto Tuple :
       zip_first(make_range(methodbins_begin() + 1, methodbins_end()),
                 make_range(methodsets_begin() + 1, methodsets_end()))) {
    auto &OtherBins = *std::get<0>(Tuple);
    for (int i = MK_Unknown; i <= MK_Last; ++i)
      switch (static_cast<MethodKind>(i)) {
      case MK_Unknown:
        if (!FirstBins[i].empty() || !OtherBins[i].empty()) {
          assert(DTransSOAToAOSComputeAllDep &&
                 "MK_Unknown methods encountered too late");
          return FALSE();
        }
        break;
      // Combined methods.
      case MK_Realloc:
      case MK_Append:
      case MK_Ctor:
      case MK_CCtor:
      case MK_Dtor: {
        if (FirstBins[i].size() != 1 || OtherBins[i].size() != 1)
          return FALSE();

        if (!FirstBins[i][0]->hasOneUse() || !OtherBins[i][0]->hasOneUse())
          return FALSE();

        Function *F = FirstBins[i][0];
        Function *O = OtherBins[i][0];

        if (!ImmutableCallSite(F->use_begin()->getUser()) ||
            !ImmutableCallSite(O->use_begin()->getUser()))
          return FALSE();

        FunctionComparator cmp(F, O, &GNS);
        if (cmp.compare() == 0) {
          LLVM_DEBUG(dbgs() << "; Comparison of " << F->getName() << " and "
                            << O->getName() << " showed bit-to-bit equality\n");
          GNS.setEqual(F, O);
        } else {
          LLVM_DEBUG(dbgs()
                     << "; Comparison of " << F->getName() << " and "
                     << O->getName() << " showed some differences, "
                     << "this situation cannot be handled in SOAToAOS\n");
          return FALSE();
        }
        break;
      }
      // Not combined methods.
      case MK_GetInteger:
      case MK_Set:
      case MK_GetElement:
        break;
      }
  }

  SmallVector<StructType*, MaxNumFieldCandidates> Arrays;
  for (auto *ArrTy : fields())
    Arrays.push_back(ArrTy);

  CallSiteComparator::CallSitesInfo CSInfo;
  for (auto *PBin : methodbins()) {
    auto &Bin = *PBin;
    if (!Bin[MK_Ctor].empty())
      CSInfo.Ctors.push_back(Bin[MK_Ctor][0]);
    if (!Bin[MK_CCtor].empty())
      CSInfo.CCtors.push_back(Bin[MK_CCtor][0]);
    if (!Bin[MK_Dtor].empty())
      CSInfo.Dtors.push_back(Bin[MK_Dtor][0]);
    if (!Bin[MK_Append].empty())
      CSInfo.Appends.push_back(Bin[MK_Append][0]);
  }

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
        dbgs() << "  Rejecting ";
        TI->getLLVMType()->print(dbgs(), true, true);
        dbgs() << " because it does not look like a candidate structurally.\n";
      });
      continue;
    }

    if (!Info->checkCFG(this->DTInfo) || !Info->populateCFGInformation(M)) {
      LLVM_DEBUG({
        dbgs() << "  Rejecting ";
        TI->getLLVMType()->print(dbgs(), true, true);
        dbgs() << " because it does not look like a candidate from CFG "
                  "analysis.\n";
      });
      continue;
    }

    if (!Info->populateSideEffects(*this, M)) {
      LLVM_DEBUG({
        dbgs() << "  Rejecting ";
        TI->getLLVMType()->print(dbgs(), true, true);
        dbgs() << " because some methods contains unknown side effect.\n";
      });
      continue;
    }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    if (!DTransSOAToAOSType.empty() &&
        DTransSOAToAOSType != cast<StructType>(TI->getLLVMType())->getName()) {
      LLVM_DEBUG({
        dbgs() << "  Rejecting ";
        TI->getLLVMType()->print(dbgs(), true, true);
        dbgs() << " based on dtrans-soatoaos-typename option.\n";
      });
      return false;
    }
#endif

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
