//===---------------- SOAToAOSStruct.cpp - Part of SOAToAOSPass -----------===//
//
// Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements debug functionality related specifically to structures
// containing arrays for SOA-to-AOS: method analysis and transformations.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/Compiler.h"

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
#include "SOAToAOSStruct.h"

#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "Intel_DTrans/Transforms/DTransOptBase.h"
#include "Intel_DTrans/Transforms/SOAToAOS.h"

#include "llvm/Support/CommandLine.h"

// Same as in SOAToAOS.cpp.
#define DEBUG_TYPE "dtrans-soatoaos"

namespace llvm {
namespace dtrans {
namespace soatoaos {
// Array types for structure containing arrays.
static cl::list<std::string> DTransSOAToAOSArrays("dtrans-soatoaos-array-type",
                                                  cl::ReallyHidden);

// Methods of arrays, which are append-like.
static cl::list<std::string>
    DTransSOAToAOSAppends("dtrans-soatoaos-array-append", cl::ReallyHidden);
// Methods of arrays, which are copy ctors.
static cl::list<std::string> DTransSOAToAOSCCtors("dtrans-soatoaos-array-cctor",
                                                  cl::ReallyHidden);
// Methods of arrays, which are regular ctors.
static cl::list<std::string> DTransSOAToAOSCtors("dtrans-soatoaos-array-ctor",
                                                 cl::ReallyHidden);
// Methods of arrays, which are dtors.
static cl::list<std::string> DTransSOAToAOSDtors("dtrans-soatoaos-array-dtor",
                                                 cl::ReallyHidden);

// CallSite comparisons to perform, requires corresponding
// dtrans-soatoaos-arrays-* to be set.
// Valid values are cctor, ctor, dtor, append.
static cl::list<std::string>
    DTransSOAToAOSComparison("dtrans-soatoaos-method-call-site-comparison",
                             cl::ReallyHidden);

static std::pair<SmallVector<StructType *, 3>, SmallVector<unsigned, 3>>
getArrayTypesForSOAToAOSStructMethodsCheckDebug(Function &F) {
  SmallVector<StructType *, 3> ArrayTypes;
  for (auto &Name : DTransSOAToAOSArrays) {
    auto *ArrType = F.getParent()->getTypeByName(Name);
    if (!ArrType)
      report_fatal_error(Twine("Cannot find struct/class type ") + Name + ".");
    ArrayTypes.push_back(ArrType);
  }

  if (ArrayTypes.size() <= 1)
    report_fatal_error("0 or 1 array types were provided, need 2 or more.");

  auto *Struct = getStructTypeOfMethod(F);
  if (!Struct)
    report_fatal_error(Twine("Cannot extract struct/class type from ") +
                       F.getName() + ".");

  SmallVector<unsigned, 3> ArrayOffsets;
  for (unsigned I = 0, E = Struct->getNumElements(); I != E; ++I) {
    auto *PTy = dyn_cast<PointerType>(Struct->getElementType(I));
    if (!PTy)
      continue;

    auto *STy = dyn_cast<StructType>(PTy->getPointerElementType());
    if (std::find(ArrayTypes.begin(), ArrayTypes.end(), STy) !=
        ArrayTypes.end())
      ArrayOffsets.push_back(I);
  }

  if (ArrayTypes.size() != ArrayOffsets.size())
    return std::make_pair(SmallVector<StructType *, 3>(),
                          SmallVector<unsigned, 3>());

  return std::make_pair(ArrayTypes, ArrayOffsets);
}

static CallSiteComparator::FunctionSet
getSOAToAOSArrayMethods(const cl::list<std::string> &List,
                        const SmallVector<StructType *, 3> &ArrayTypes,
                        Function &F) {
  CallSiteComparator::FunctionSet Methods(ArrayTypes.size(), nullptr);
  unsigned Count = 0;
  for (auto &Name : List) {
    auto *AF = F.getParent()->getFunction(Name);
    if (!AF)
      continue;

    auto *ClassType = getStructTypeOfMethod(*AF);
    if (!ClassType)
      report_fatal_error(Twine("Cannot extract struct/class type from ") +
                         Name + ".");

    auto It = std::find(ArrayTypes.begin(), ArrayTypes.end(), ClassType);
    if (It == ArrayTypes.end())
      report_fatal_error(
          Twine("Function ") + Name +
          " is not related to array types from dtrans-soatoaos-array-type.");

    if (Methods[It - ArrayTypes.begin()])
      report_fatal_error(Twine("Function ") + Name +
                         " has the same kind (append/ctor/cctor/dtor) as "
                         "another function for the same array type. Methods to "
                         "combine should be unique.");
    Count++;
    Methods[It - ArrayTypes.begin()] = AF;
  }


  if (Count == 0)
    return CallSiteComparator::FunctionSet();

  if (Count != ArrayTypes.size())
    report_fatal_error("There should be none or 1 method to combine for each array type.");

  return Methods;
}
} // namespace soatoaos
using namespace soatoaos;
char SOAToAOSStructMethodsCheckDebug::PassID;

// Provide a definition for the static class member used to identify passes.
AnalysisKey SOAToAOSStructMethodsCheckDebug::Key;

SOAToAOSStructMethodsCheckDebug::Ignore::Ignore(
    SOAToAOSStructMethodsCheckDebugResult *Ptr)
    : Ptr(Ptr) {}
SOAToAOSStructMethodsCheckDebug::Ignore::Ignore(Ignore &&Other)
    : Ptr(std::move(Other.Ptr)) {}
const SOAToAOSStructMethodsCheckDebugResult *
SOAToAOSStructMethodsCheckDebug::Ignore::get() const {
  return Ptr.get();
}
SOAToAOSStructMethodsCheckDebug::Ignore::~Ignore() {}

SOAToAOSStructMethodsCheckDebug::Ignore
SOAToAOSStructMethodsCheckDebug::run(Function &F, FunctionAnalysisManager &AM) {
  const ModuleAnalysisManager &MAM =
      AM.getResult<ModuleAnalysisManagerFunctionProxy>(F).getManager();
  auto *DTInfo = MAM.getCachedResult<DTransAnalysis>(*F.getParent());
  auto *TLI = AM.getCachedResult<TargetLibraryAnalysis>(F);
  if (!DTInfo || !TLI)
    report_fatal_error("DTransAnalysis was not run before "
                       "SOAToAOSStructMethodsCheckDebug.");

  auto *Res = AM.getCachedResult<SOAToAOSApproximationDebug>(F);
  if (!Res)
    report_fatal_error("SOAToAOSApproximationDebug was not run before "
                       "SOAToAOSStructMethodsCheckDebug.");
  const DepMap *DM = Res->get();
  if (!DM)
    report_fatal_error("Missing SOAToAOSApproximationDebug results before "
                       "SOAToAOSStructMethodsCheckDebug.");

  SummaryForIdiom S = getParametersForSOAToAOSMethodsCheckDebug(F);

  LLVM_DEBUG(dbgs() << "; Checking structure's method " << F.getName() << "\n");

  auto P = getArrayTypesForSOAToAOSStructMethodsCheckDebug(F);

  std::unique_ptr<SOAToAOSStructMethodsCheckDebugResult> Result(
      new SOAToAOSStructMethodsCheckDebugResult());

  for (auto &CmpKind : DTransSOAToAOSComparison)
    if (CmpKind == "append")
      Result->Appends =
          getSOAToAOSArrayMethods(DTransSOAToAOSAppends, P.first, F);
    else if (CmpKind == "cctor")
      Result->CCtors =
          getSOAToAOSArrayMethods(DTransSOAToAOSCCtors, P.first, F);
    else if (CmpKind == "ctor")
      Result->Ctors =
          getSOAToAOSArrayMethods(DTransSOAToAOSCtors, P.first, F);
    else if (CmpKind == "dtor")
      Result->Dtors =
          getSOAToAOSArrayMethods(DTransSOAToAOSDtors, P.first, F);
    else
      report_fatal_error("Incorrect value in dtrans-soatoaos-method-call-site-comparison."
                         "append/cctor/ctor/dtor are allowed.");

  if (DTransSOAToAOSBasePtrOff == -1U)
    report_fatal_error("dtrans-soatoaos-base-ptr-off was not provided.");

  StructureMethodAnalysis Checks(F.getParent()->getDataLayout(), *DTInfo, *TLI,
                                 *DM, S, P.first,
                                 *Result /*TransformationData*/);

  bool SeenArrays = false;
  bool CheckedAll = Checks.checkStructMethod(SeenArrays);
  (void)CheckedAll;
  LLVM_DEBUG(
      dbgs() << "; IR: "
             << (CheckedAll && SeenArrays
                     ? " has only expected side-effects\n"
                     : (SeenArrays
                            ? " needs analysis of instructions)\n"
                            : " no need to analyze: no accesses to arrays\n")));

  // If there are no accesses to array, then there should be no instructions to
  // update.
  assert((SeenArrays || Checks.getTotal() == 0) &&
         "Inconsistent checkStructMethod");

  // Dump results of analysis.
  DEBUG_WITH_TYPE(DTRANS_SOASTR, {
    dbgs() << "; Dump instructions needing update. Total = "
           << Checks.getTotal();
    StructureMethodAnalysis::AnnotatedWriter Annotate(Checks);
    F.print(dbgs(), &Annotate);
  });

  if (SeenArrays) {
    CallSiteComparator Cmp(F.getParent()->getDataLayout(), *DTInfo, *TLI, *DM,
                           S, P.first, P.second,
                           *Result, /* CsllSiteComparator */
                           *Result, /*TransformationData*/
                           DTransSOAToAOSBasePtrOff);

    bool Comparison = Cmp.canCallSitesBeMerged();
    (void)Comparison;
    LLVM_DEBUG(dbgs() << "; Array call sites analysis result: "
                      << (Comparison
                              ? "required call sites can be merged"
                              : "problem with call sites required to be merged")
                      << "\n");
  }
  return Ignore(Result.release());
}

namespace {
class SOAStructMethodReplacement : public DTransOptBase {
public:
  SOAStructMethodReplacement(
      DTransAnalysisInfo &DTInfo, LLVMContext &Context, const DataLayout &DL,
      std::function<const TargetLibraryInfo &(const Function &)> GetTLI,
      const SummaryForIdiom &S, const SmallVectorImpl<StructType *> &Arrays,
      const SmallVectorImpl<unsigned> &Offsets,
      const SOAToAOSStructMethodsCheckDebugResult &InstsToTransform,
      StringRef DepTypePrefix, DTransTypeRemapper *TypeRemapper)
      : DTransOptBase(&DTInfo, Context, DL, GetTLI, DepTypePrefix,
                      TypeRemapper),
        InstsToTransform(InstsToTransform), S(S), Arrays(Arrays),
        Offsets(Offsets) {}

  bool prepareTypes(Module &M) override {
    LLVMContext &Context = M.getContext();

    NewStruct = StructType::create(
        Context, (Twine(DepTypePrefix) + S.StrType->getName()).str());
    TypeRemapper->addTypeMapping(S.StrType, NewStruct);
    NewArray = StructType::create(
        Context, (Twine(DepTypePrefix) + "AR_" + Arrays[0]->getName()).str());
    NewElement = StructType::create(
        Context, (Twine(DepTypePrefix) + "EL_" + S.StrType->getName()).str());

    for (auto *Fld: Arrays)
      TypeRemapper->addTypeMapping(Fld, NewArray);

    FunctionType *NewFunctionTy = nullptr;
    for (auto *Method : InstsToTransform.Appends) {
      auto *FunctionTy = Method->getFunctionType();
      if (!NewFunctionTy) {
        SmallVector<PointerType *, 3> Elems;
        for (auto *ArrType : Arrays)
          Elems.push_back(getSOAElementType(ArrType, DTransSOAToAOSBasePtrOff));

        // updateAppends in StructMethodTransformation compute necessary
        // offset during CallSite creation.
        unsigned IgnoreElemOffset = 0;
        auto *StType = getStructTypeOfMethod(*Method);
        assert(StType && "Expected class type for struct method");
        NewFunctionTy = ArrayMethodTransformation::mapNewAppendType(
            *Method,
            getSOAElementType(StType, DTransSOAToAOSBasePtrOff),
            Elems, TypeRemapper, IgnoreElemOffset);
      } else
        TypeRemapper->addTypeMapping(FunctionTy, NewFunctionTy);
    }

    return true;
  }

  void populateTypes(Module &M) override {
    {
      SmallVector<Type *, 6> Elements;
      for (auto *Fld : Arrays)
        Elements.push_back(getSOAElementType(Fld, DTransSOAToAOSBasePtrOff));
      NewElement->setBody(Elements);
    }

    {
      SmallVector<Type *, 6> ArrayElems(Arrays[0]->element_begin(),
                                        Arrays[0]->element_end());
      ArrayElems[DTransSOAToAOSBasePtrOff] = NewElement->getPointerTo(0);
      NewArray->setBody(ArrayElems);
    }

    {
      SmallVector<Type *, 6> StructElems(S.StrType->element_begin(),
                                         S.StrType->element_end());

      auto PFloat = Type::getFloatTy(Context)->getPointerTo(0);
      // Do not change layout of structure, all but one
      // pointer to transformed array is replaced with pointer to 'float'.
      for (auto O : Offsets)
        StructElems[O] = PFloat;

      AOSOffset = *std::min_element(Offsets.begin(), Offsets.end());

      StructElems[AOSOffset] = NewArray->getPointerTo(0);
      NewStruct->setBody(StructElems);
    }
  }

  void postprocessFunction(Function &OrigFunc, bool isCloned) override {
    // InstsToTransform contains all needed information.
    // New instructions after cloning is obtained using VMap.
    if (!isCloned)
      return;

    StructMethodTransformation SMT(
        DL, *DTInfo, VMap, InstsToTransform /* CallSiteComparator */,
        InstsToTransform /*TransformationData*/, Context);

    SMT.updateReferences(S.StrType, NewArray, Arrays, AOSOffset,
                         DTransSOAToAOSBasePtrOff);
  }

private:
  const SOAToAOSStructMethodsCheckDebugResult &InstsToTransform;
  const SummaryForIdiom &S;
  const SmallVectorImpl<StructType *> &Arrays;
  const SmallVectorImpl<unsigned> &Offsets;

  // Structure containing one element per each transformed array.
  StructType *NewElement = nullptr;
    // New array of structures, structure is NewElement.
  StructType *NewArray = nullptr;
  // Structure containing pointer to NewArray.
  StructType *NewStruct = nullptr;
  // Offset of array-of-structure in NewStruct.
  unsigned AOSOffset = -1U;
};
} // namespace

PreservedAnalyses
SOAToAOSStructMethodsTransformDebug::run(Module &M, ModuleAnalysisManager &AM) {
  // It should be lit-test with single defined function at this point.
  Function *MethodToTest = nullptr;
  for (auto &F : M)
    if (!F.isDeclaration()) {
      if (MethodToTest)
        report_fatal_error("Single function definition per compilation unit is "
                           "allowed in SOAToAOSStructMethodsTransformDebug.");
      MethodToTest = &F;
    }

  if (!MethodToTest)
    report_fatal_error(
        "Exactly one function definition per compilation unit is "
        "required in SOAToAOSStructMethodsTransformDebug.");

  SummaryForIdiom S = getParametersForSOAToAOSMethodsCheckDebug(*MethodToTest);

  auto P = getArrayTypesForSOAToAOSStructMethodsCheckDebug(*MethodToTest);

  if (DTransSOAToAOSBasePtrOff == -1U)
    report_fatal_error("dtrans-soatoaos-base-ptr-off was not provided.");

  auto &DTInfo = AM.getResult<DTransAnalysis>(M);
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetTLI = [&FAM](const Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(*(const_cast<Function*>(&F)));
  };
  // SOAToAOSStructMethodsCheckDebug uses SOAToAOSApproximationDebug internally.
  FAM.getResult<SOAToAOSApproximationDebug>(*MethodToTest);
  auto &InstsToTransformPtr =
      FAM.getResult<SOAToAOSStructMethodsCheckDebug>(*MethodToTest);

  DTransTypeRemapper TypeRemapper;
  SOAStructMethodReplacement Transformer(
      DTInfo, M.getContext(), M.getDataLayout(), GetTLI, S, P.first, P.second,
      *InstsToTransformPtr.get(), "__SOA_", &TypeRemapper);

  bool Changed = Transformer.run(M);
  if (!Changed)
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}
} // namespace dtrans
} // namespace llvm
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
