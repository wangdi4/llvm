//===---------------- SOAToAOS.cpp - SOAToAOSPass -------------------------===//
//
// Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
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
#include "Intel_DTrans/Analysis/DTransAnnotator.h"
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

// This option makes populateCFGInformation to ignore
// number of uses and number of BasicBlocks in struct's and arrays' methods.
//
// It helps to debug issues in transformed code with debug prints, for
// example.
static cl::opt<bool> DTransSOAToAOSSizeHeuristic(
    "dtrans-soatoaos-size-heuristic", cl::init(true), cl::Hidden,
    cl::desc("Respect size heuristic in DTrans SOAToAOS"));


#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
static cl::opt<std::string> DTransSOAToAOSType("dtrans-soatoaos-typename",
                                               cl::ReallyHidden);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

class SOAToAOSTransformImpl : public DTransOptBase {
public:
  SOAToAOSTransformImpl(
      DTransAnalysisInfo &DTInfo, LLVMContext &Context, const DataLayout &DL,
      std::function<const TargetLibraryInfo &(const Function &)> GetTLI,
      StringRef DepTypePrefix, DTransTypeRemapper *TypeRemapper)
      : DTransOptBase(&DTInfo, Context, DL, GetTLI, DepTypePrefix,
                      TypeRemapper) {}

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
  void postprocessFunction(Function &OrigFunc, bool isCloned) override;

  // Relatively heavy weight checks to classify methods, see MethodKind.
  // Has internal memory handling for DepMap:
  // copying is forbidden.
  //
  class CandidateSideEffectsInfo : public SOAToAOSCFGInfo, DepMap {
  public:
    // Computes dependencies using DepMap.
    bool populateSideEffects(SOAToAOSTransformImpl &Impl, Module &M);

    // Checks that all field arrays' methods are called from structure's
    // methods; Necessary check for legality analysis.
    bool checkCFG(const DTransAnalysisInfo &DTInfo) const {
      for (auto *ArrTy : fields()) {
        auto *FI = cast_or_null<dtrans::StructInfo>(DTInfo.getTypeInfo(ArrTy));

        if (!FI)
          return FALSE("array type has no DTrans info.");

        // May restrict analysis to Struct's methods.
        auto &CG = FI->getCallSubGraph();
        if (CG.isTop() || CG.isBottom() || CG.getEnclosingType() != Struct)
          return FALSE("array type has unsupported CFG.");
      }
      return true;
    }

  protected:
    CandidateSideEffectsInfo() {}

    struct CombinedCallSiteInfo : public CallSiteComparator::CallSitesInfo {
      // Calls are from append-like method, no need for special processing.
      // Comparison is done using FunctionComparator.
      CallSiteComparator::FunctionSet Reallocs;
    };
    CombinedCallSiteInfo CSInfo;

    DenseMap<const Function *,
             std::unique_ptr<StructureMethodAnalysis::TransformationData>>
        StructTransInfo;
    DenseMap<
        const Function *,
        std::unique_ptr<ComputeArrayMethodClassification::TransformationData>>
        ArrayTransInfo;

  private:
    CandidateSideEffectsInfo(const CandidateSideEffectsInfo &) = delete;
    CandidateSideEffectsInfo &
    operator=(const CandidateSideEffectsInfo &) = delete;

  };

  // Check debug passes in SOAToAOSArrays.cpp and SOAToAOSStruct.cpp.
  class CandidateInfo : public CandidateSideEffectsInfo {
  public:
    void prepareTypes(SOAToAOSTransformImpl &Impl, Module &M) {
      LLVMContext &Context = M.getContext();

      NewStruct = StructType::create(
          Context, (Twine(Impl.DepTypePrefix) + Struct->getName()).str());
      Impl.TypeRemapper->addTypeMapping(Struct, NewStruct);
      NewArray =
          StructType::create(Context, (Twine(Impl.DepTypePrefix) + "AR_" +
                                       (*fields_begin())->getName())
                                          .str());
      NewElement = StructType::create(
          Context,
          (Twine(Impl.DepTypePrefix) + "EL_" + Struct->getName()).str());

      for (auto *Fld : fields())
        Impl.TypeRemapper->addTypeMapping(Fld, NewArray);

      FunctionType *NewFunctionTy = nullptr;
      for (auto *Method : CSInfo.Appends) {
        auto *FunctionTy = Method->getFunctionType();
        if (!NewFunctionTy) {
          SmallVector<PointerType *, MaxNumFieldCandidates> Elems;
          for (auto *Elem : elements())
            Elems.push_back(Elem);

          auto *ArrType = getStructTypeOfMethod(*Method);
          assert(ArrType && "Expected class type for array method");
          NewFunctionTy = ArrayMethodTransformation::mapNewAppendType(
              *Method,
              getSOAElementType(ArrType, BasePointerOffset),
              Elems, Impl.TypeRemapper, AppendMethodElemParamOffset);
        } else
          Impl.TypeRemapper->addTypeMapping(FunctionTy, NewFunctionTy);
      }
    }

    void populateTypes(SOAToAOSTransformImpl &Impl, Module &M) {
      {
        SmallVector<Type *, 6> Elements(elements_begin(), elements_end());
        NewElement->setBody(Elements);
      }

      {
        SmallVector<Type *, 6> ArrayElems((*fields_begin())->element_begin(),
                                          (*fields_begin())->element_end());
        ArrayElems[BasePointerOffset] = NewElement->getPointerTo(0);
        NewArray->setBody(ArrayElems);
      }

      {
        SmallVector<Type *, 6> StructElems(Struct->element_begin(),
                                           Struct->element_end());

        auto PlaceHolderType = Impl.DL.getIntPtrType(M.getContext(), 0);
        // Do not change layout of structure, all but one
        // pointer to transformed array is replaced with intptr_t.
        for (auto O : ArrayFieldOffsets)
          StructElems[O] = PlaceHolderType;

        AOSOffset = *std::min_element(ArrayFieldOffsets.begin(),
                                      ArrayFieldOffsets.end());

        StructElems[AOSOffset] = NewArray->getPointerTo(0);
        NewStruct->setBody(StructElems);
      }
    }

    void postprocessStructMethod(
        SOAToAOSTransformImpl &Impl, Function &OrigFunc,
        const StructureMethodAnalysis::TransformationData &TI) {

      SmallVector<StructType *, MaxNumFieldCandidates> Fields(fields_begin(),
                                                              fields_end());
      StructMethodTransformation SMT(Impl.DL, *Impl.DTInfo, Impl.VMap,
                                     CSInfo, TI,
                                     OrigFunc.getParent()->getContext());

      SMT.updateReferences(Struct, NewArray, Fields, AOSOffset,
                           BasePointerOffset);
    }

    void postprocessArrayMethod(
        SOAToAOSTransformImpl &Impl, Function &OrigFunc,
        const ComputeArrayMethodClassification::TransformationData &TI) {

      auto *ArrType = getStructTypeOfMethod(OrigFunc);
      assert(ArrType && "Expected class type for array method");
      auto *CurrElem = getSOAElementType(ArrType, BasePointerOffset);

      auto It = std::find(fields_begin(), fields_end(), ArrType);
      assert(It != fields_end() && "Incorrect array method");
      auto Ind = It - fields_begin();

      bool CopyElemInsts = CSInfo.Reallocs[Ind] == &OrigFunc;
      if (!CSInfo.CCtors.empty())
        CopyElemInsts |= CSInfo.CCtors[Ind] == &OrigFunc;
      if (!CSInfo.Ctors.empty())
        CopyElemInsts |= CSInfo.Ctors[Ind] == &OrigFunc;
      if (!CSInfo.Dtors.empty())
        CopyElemInsts |= CSInfo.Dtors[Ind] == &OrigFunc;
      if (!CSInfo.Appends.empty())
        CopyElemInsts |= CSInfo.Appends[Ind] == &OrigFunc;

      // Only one copy of combined methods is left.
      if (CopyElemInsts &&
          CurrElem != getSOAElementType(getSOAArrayType(Struct, AOSOffset),
                                        BasePointerOffset)) {
        auto *NewFunc = Impl.OrigFuncToCloneFuncMap[&OrigFunc];
        for (auto &I : instructions(*NewFunc))
          Impl.DTInfo->deleteCallInfo(&I);
        NewFunc->deleteBody();
        return;
      }

      const TargetLibraryInfo &TLI = Impl.GetTLI(OrigFunc);
      ArrayMethodTransformation AMT(Impl.DL, *Impl.DTInfo, TLI, Impl.VMap,
                                    TI, OrigFunc.getParent()->getContext());

      AMT.updateBasePointerInsts(CopyElemInsts, getNumArrays(),
                                 NewElement->getPointerTo(0));

      bool UpdateUnique = true;
      unsigned Off = -1U;
      unsigned CurrOff = 0U;
      for (auto *Elem : elements()) {
        ++Off;

        if (Elem == CurrElem) {
          CurrOff = Off;
          continue;
        }

        if (CopyElemInsts) {
          ArrayMethodTransformation::OrigToCopyTy OrigToCopy;
          AMT.rawCopyAndRelink(
              OrigToCopy, UpdateUnique, getNumArrays(),
              Elem /*Type related to copies*/,
              AppendMethodElemParamOffset +
                  Off /* Offset in argument list of new element*/);
          AMT.gepRAUW(true /*Do copy*/, OrigToCopy,
                      Off /*Elem's offset in NewElement*/,
                      NewElement->getPointerTo(0));
          UpdateUnique = false;
        }
      }

      // Original instructions from cloned function should be replaced as a
      // last step to keep OrigToCopy valid.
      AMT.gepRAUW(false /*Only update existing insts*/,
                  ArrayMethodTransformation::OrigToCopyTy(),
                  CurrOff /*CurElem's offset in NewElement*/,
                  NewElement->getPointerTo(0));

      // If the function was the constructor, copy constructor, destructor,
      // realloc or append method, mark it so that the Weak Align pass can
      // recognize it.
      if (CopyElemInsts) {
        auto *NewFunc = Impl.OrigFuncToCloneFuncMap[&OrigFunc];
        DTransAnnotator::createDTransSOAToAOSTypeAnnotation(
            *NewFunc, NewElement->getPointerTo());
      }
    }

    void postprocessFunction(SOAToAOSTransformImpl &Impl, Function &OrigFunc,
                             bool isCloned) {
      if (!isCloned)
        return;

      auto SIt = StructTransInfo.find(&OrigFunc);
      if (SIt != StructTransInfo.end()) {
        postprocessStructMethod(Impl, OrigFunc, *SIt->second.get());
        return;
      }

      auto AIt = ArrayTransInfo.find(&OrigFunc);
      if (AIt != ArrayTransInfo.end()) {
        postprocessArrayMethod(Impl, OrigFunc, *AIt->second.get());
        return;
      }
    }

    CandidateInfo() {}

  private:
    CandidateInfo(const CandidateInfo &) = delete;
    CandidateInfo &operator=(const CandidateInfo &) = delete;

    // Structure containing one element per each transformed array.
    StructType *NewElement = nullptr;
    // New array of structures, structure is NewElement.
    StructType *NewArray = nullptr;
    // Structure containing pointer to NewArray.
    StructType *NewStruct = nullptr;

    // Offset of array-of-structure in NewStruct.
    unsigned AOSOffset = -1U;
    // Offset of the first element parameter for MK_Append method.
    // Element parameters are arranged in order of NewElement.
    unsigned AppendMethodElemParamOffset = -1U;
  };

  constexpr static int MaxNumStructCandidates = 1;

  SmallVector<CandidateInfo *, MaxNumStructCandidates> Candidates;
};

// Hook point. Top-level returns from populate* methods.
inline bool FALSE(const char *Msg) {
  LLVM_DEBUG(dbgs() << "; dtrans-soatoaos: " << Msg << "\n");
  return false;
}

bool SOAToAOSTransformImpl::CandidateSideEffectsInfo::populateSideEffects(
    SOAToAOSTransformImpl &Impl, Module &M) {

  for (auto Pair : zip_first(methodsets(), fields()))
    for (auto *F : *std::get<0>(Pair)) {
      const TargetLibraryInfo &TLI = Impl.GetTLI(*F);
      DepCompute DC(*Impl.DTInfo, Impl.DL, TLI, F, std::get<1>(Pair),
                    // *this as DepMap to fill.
                    *this);

      bool Result = DC.computeDepApproximation();
      LLVM_DEBUG({
        dbgs() << "; Dep approximation for array's method " << F->getName()
               << (Result ? " is successful\n" : " incomplete.\n");
        if (!Result)
          dbgs() << "; See -debug-only=" << DTRANS_SOADEP
                 << " RUN-lines in soatoaos02*.ll on debugging.";
      });
      DEBUG_WITH_TYPE(DTRANS_SOADEP, {
        dbgs() << "; Dump computed dependencies ";
        DepMap::DepAnnotatedWriter Annotate(*this);
        F->print(dbgs(), &Annotate);
      });
      if (!Result)
        return FALSE("unhandled instruction seen in array method.");
    }

  for (auto *F : StructMethods) {
    const TargetLibraryInfo &TLI = Impl.GetTLI(*F);
    DepCompute DC(*Impl.DTInfo, Impl.DL, TLI, F, Struct, *this);

    bool Result = DC.computeDepApproximation();
    LLVM_DEBUG({
      dbgs() << "; Dep approximation for struct's method " << F->getName()
             << (Result ? " is successful\n" : " incomplete.\n");
      if (!Result)
        dbgs() << "; See -debug-only=" << DTRANS_SOADEP
               << " RUN-lines in soatoaos05*.ll on debugging.\n";
    });
    DEBUG_WITH_TYPE(DTRANS_SOADEP, {
      dbgs() << "; Dump computed dependencies ";

      DepMap::DepAnnotatedWriter Annotate(*this);
      F->print(dbgs(), &Annotate);
    });

    if (!Result)
      return FALSE("unhandled instruction seen in struct method.");
  }

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

      std::unique_ptr<ComputeArrayMethodClassification::TransformationData>
          Data(new ComputeArrayMethodClassification::TransformationData());

      const TargetLibraryInfo &TLI = Impl.GetTLI(*F);
      ComputeArrayMethodClassification MC(Impl.DL,
                                          // *this as DepMap to query.
                                          *this, S,
                                          // Info for transformation
                                          *Data, TLI);
      auto Res = MC.classify();
      auto Kind = Res.first;

      LLVM_DEBUG({
        dbgs() << "; Classification: " << Kind << "\n";
        if (Kind == MK_Unknown)
          dbgs() << "; See -debug-only=" << DTRANS_SOAARR
                 << " RUN-lines in soatoaos03*.ll and soatoaos04*.ll on "
                    "debugging.\n";
      });

      if (Kind == MK_Unknown)
        UnknownSeen = true;

      if (UnknownSeen && !DTransSOAToAOSComputeAllDep)
        return FALSE("cannot classify array method");

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
          return FALSE("combined array method does not have 1 use.");
        if (!isa<CallBase>(F->use_begin()->getUser()))
          return FALSE(
              "combined array method is referenced not in call/invoke.");
      }

      // Simple processing of MK_Append calling MK_Realloc.
      if (Res.second) {
        if (Kind != MK_Append)
          return FALSE(
              "another array method called from not append-like method.");
        if (MethodsCalled)
          return FALSE("2 calls to realloc array method called from "
                       "2 append-like methods.");
        MethodsCalled = Res.second;
      }
      // Ownership is passed to ArrayTransInfo.
      ArrayTransInfo[F] = decltype(Data)(Data.release());
    }

    if (CSInfo.Ctors.size() != Cnt || CSInfo.CCtors.size() != Cnt ||
        CSInfo.Dtors.size() != Cnt || CSInfo.Appends.size() != Cnt ||
        CSInfo.Reallocs.size() != Cnt)
      return FALSE("cannot combine array methods: number of "
                   "ctor/cctor/dtor/realloc/append inconsistent.");

    // Simple processing of MK_Append.
    // As direct calls to MK_Realloc is not tested, forbid it, MK_Realloc is
    // called only from MK_Append.
    if (CSInfo.Reallocs.back() != MethodsCalled)
      return FALSE(
          "method  called from append-like method is not realloc.");
  }

  if (UnknownSeen) {
    assert(DTransSOAToAOSComputeAllDep &&
           "MK_Unknown methods encountered too late");
    return FALSE(" could not classify array method.");
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
        return FALSE("bit-to-bit equality failed.");
      }
    }
  }

  SmallVector<StructType *, MaxNumFieldCandidates> Arrays;
  for (auto *ArrTy : fields())
    Arrays.push_back(ArrTy);

  for (auto *F : StructMethods) {
    SummaryForIdiom S(Struct, MemoryInterface, F);

    std::unique_ptr<StructureMethodAnalysis::TransformationData> Data(
        new StructureMethodAnalysis::TransformationData());

    const TargetLibraryInfo &TLI = Impl.GetTLI(*F);
    StructureMethodAnalysis MChecker(Impl.DL, *Impl.DTInfo, TLI, *this, S,
                                     Arrays, *Data);
    bool SeenArrays = false;
    bool CheckResult = MChecker.checkStructMethod(SeenArrays);
    LLVM_DEBUG({
      dbgs() << "; Struct's method " << F->getName()
             << (CheckResult && SeenArrays
                     ? " has only expected side-effects\n"
                     : (SeenArrays ?  " needs analysis of instructions)\n":
                        " no need to analyze: no accesses to arrays\n"));
      if (!CheckResult && SeenArrays)
        dbgs() << "; See -debug-only=" << DTRANS_SOASTR
               << " RUN-lines in soatoaos05*.ll on debugging.\n";
    });

    if (!CheckResult && SeenArrays)
      return FALSE("cannot process all side-effects in struct method.");

    if (!SeenArrays && MChecker.getTotal() != 0)
      llvm_unreachable("inconsistent logic in checkStructMethod.");

    if (SeenArrays) {
      CallSiteComparator CSCmp(Impl.DL, *Impl.DTInfo, TLI, *this, S, Arrays,
                               ArrayFieldOffsets, *Data, CSInfo,
                               BasePointerOffset);
      bool Comparison = CSCmp.canCallSitesBeMerged();
      LLVM_DEBUG({
        dbgs() << "; Array call sites analysis result: "
               << (Comparison ? "required call sites can be merged"
                              : "problem with call sites required to be merged")
               << " in " << F->getName() << "\n";
        if (!Comparison)
          dbgs() << "; See -debug-only=" << DTRANS_SOASTR
                 << " RUN-lines in soatoaos05*.ll on debugging.\n";
      });
      if (!Comparison)
        return FALSE("cannot compare call sites of array methods to combine.");
    }

    // Pass ownership to StructTransInfo.
    StructTransInfo[F] = decltype(Data)(Data.release());
  }

  return true;
}

bool SOAToAOSTransformImpl::prepareTypes(Module &M) {

  for (dtrans::TypeInfo *TI : DTInfo->type_info_entries()) {
    std::unique_ptr<CandidateInfo> Info(new CandidateInfo());

    if (!Info->populateLayoutInformation(TI->getLLVMType())) {
      LLVM_DEBUG({
        dbgs() << "  ; Rejecting ";
        TI->getLLVMType()->print(dbgs(), true, true);
        dbgs() << " because it does not look like a candidate structurally.\n";
      });
      continue;
    }

    // Test safety violations on both structure and array types.
    bool SafetyViolation = DTInfo->testSafetyData(TI, dtrans::DT_SOAToAOS);
    if (!SafetyViolation)
      for (auto *Fld : Info->fields()) {
        auto *FTI = DTInfo->getTypeInfo(Fld);
        if (!FTI || DTInfo->testSafetyData(FTI, dtrans::DT_SOAToAOS)) {
          SafetyViolation = true;
          break;
        }
      }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    if (!DTransSOAToAOSType.empty() &&
        DTransSOAToAOSType == cast<StructType>(TI->getLLVMType())->getName())
      SafetyViolation = false;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

    if (SafetyViolation) {
      LLVM_DEBUG({
        dbgs() << "  ; Rejecting ";
        TI->getLLVMType()->print(dbgs(), true, true);
        dbgs() << " because safety checks were violated\n";
      });
      continue;
    }

    if (!Info->populateCFGInformation(M, DTransSOAToAOSSizeHeuristic, true)) {
      LLVM_DEBUG({
        dbgs() << "  ; Rejecting ";
        TI->getLLVMType()->print(dbgs(), true, true);
        dbgs() << " because it does not look like a candidate from CFG "
                  "analysis.\n";
      });
      continue;
    }

    if (!Info->checkCFG(*this->DTInfo)) {
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
      return FALSE("conflicting -dtrans-soatoaos-typename.");
    }
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

    if (Candidates.size() > MaxNumStructCandidates) {
      return FALSE("too many candidates found.");
    }

    LLVM_DEBUG({
      dbgs() << "  ; SOA-to-AOS possible for ";
      TI->getLLVMType()->print(dbgs(), true, true);
      dbgs() << ".\n";
    });
    Candidates.push_back(Info.release());
  }

  if (Candidates.empty()) {
    return FALSE("no candidates found.");
  }

  for_each(Candidates,
           [this, &M](CandidateInfo *C) { C->prepareTypes(*this, M); });

  return true;
}

void SOAToAOSTransformImpl::populateTypes(Module &M) {
  for_each(Candidates,
           [this, &M](CandidateInfo *C) { C->populateTypes(*this, M); });
}

void SOAToAOSTransformImpl::postprocessFunction(Function &OrigFunc,
                                                bool isCloned) {
  for_each(Candidates, [this, &OrigFunc, isCloned](CandidateInfo *C) {
    C->postprocessFunction(*this, OrigFunc, isCloned);
  });
}
} // namespace

namespace llvm {
namespace dtrans {

bool SOAToAOSPass::runImpl(
    Module &M, DTransAnalysisInfo &DTInfo,
    std::function<const TargetLibraryInfo &(const Function &)> GetTLI,
    WholeProgramInfo &WPInfo) {
  auto TTIAVX2 = TargetTransformInfo::AdvancedOptLevel::AO_TargetHasAVX2;
  if (!WPInfo.isWholeProgramSafe() || !WPInfo.isAdvancedOptEnabled(TTIAVX2))
    return false;

  // Perform the actual transformation.
  DTransTypeRemapper TypeRemapper;
  SOAToAOSTransformImpl Transformer(DTInfo, M.getContext(), M.getDataLayout(),
                                    GetTLI, "__SOADT_", &TypeRemapper);
  return Transformer.run(M);
}

PreservedAnalyses SOAToAOSPass::run(Module &M, ModuleAnalysisManager &AM) {
  auto &WP = AM.getResult<WholeProgramAnalysis>(M);
  if (!WP.isWholeProgramSafe())
    return PreservedAnalyses::all();

  auto &DTransInfo = AM.getResult<DTransAnalysis>(M);
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetTLI = [&FAM](const Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(*(const_cast<Function*>(&F)));
  };
  bool Changed = runImpl(M, DTransInfo, GetTLI, WP);

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
    if (skipModule(M))
      return false;

    auto &WP = getAnalysis<WholeProgramWrapperPass>().getResult();
    if (!WP.isWholeProgramSafe())
      return false;

    auto &DTAnalysisWrapper = getAnalysis<DTransAnalysisWrapper>();
    DTransAnalysisInfo &DTInfo = DTAnalysisWrapper.getDTransInfo(M);
    if (!DTInfo.useDTransAnalysis())
      return false;

    auto GetTLI = [this](const Function &F) -> TargetLibraryInfo & {
      return this->getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
    };
    bool Changed = Impl.runImpl(M, DTInfo, GetTLI, WP);
    if (Changed)
      DTAnalysisWrapper.setInvalidated();
    return Changed;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DTransAnalysisWrapper>();
    AU.addRequired<TargetLibraryInfoWrapperPass>();
    AU.addRequired<WholeProgramWrapperPass>();
    // TODO: Mark the actual preserved analyses.
    AU.addPreserved<DTransAnalysisWrapper>();
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
