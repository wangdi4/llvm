//===---------------- SOAToAOSStruct.cpp - Part of SOAToAOSPass -----------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
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
    auto *ArrayType = F.getParent()->getTypeByName(Name);
    if (!ArrayType)
      report_fatal_error(Twine("Cannot find struct/class type ") + Name + ".");
    ArrayTypes.push_back(ArrayType);
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
    // TODO: add remark.
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

  if (Count != 0 && Count != ArrayTypes.size())
    report_fatal_error("There should be none or 1 method to combine for each array type.");

  if (Count == 0)
    return CallSiteComparator::FunctionSet();

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
  auto *TLI = MAM.getCachedResult<TargetLibraryAnalysis>(*F.getParent());
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
                                 *DM, S, P.first, P.second,
                                 *Result /*TransformationData*/);

  bool CheckedAll = Checks.checkStructMethod();
  (void)CheckedAll;
  LLVM_DEBUG(dbgs() << "; IR: "
                    << (CheckedAll ? "analysed completely"
                                   : "has some side-effect to analyse")
                    << "\n");

  // Dump results of analysis.
  DEBUG_WITH_TYPE(DTRANS_SOASTR, {
    dbgs() << "; Dump instructions needing update. Total = "
           << Checks.getTotal();
    StructureMethodAnalysis::AnnotatedWriter Annotate(Checks);
    F.print(dbgs(), &Annotate);
  });

  CallSiteComparator Cmp(F.getParent()->getDataLayout(), *DTInfo, *TLI, *DM, S,
                         P.first, P.second, *Result, /* CsllSiteComparator */
                         *Result,                    /*TransformationData*/
                         DTransSOAToAOSBasePtrOff);

  bool Comparison = Cmp.canCallSitesBeMerged();
  (void)Comparison;
  LLVM_DEBUG(dbgs() << "; Array call sites analysis result: "
                    << (Comparison ? "required call sites can be merged"
                                   : "problem with call sites required to be merged")
                    << "\n");
  return Ignore(Result.release());
}

namespace {
class SOAStructMethodReplacement : public DTransOptBase {
public:
  SOAStructMethodReplacement(
      DTransAnalysisInfo &DTInfo, LLVMContext &Context, const DataLayout &DL,
      const TargetLibraryInfo &TLI, const SummaryForIdiom &S,
      const SmallVectorImpl<StructType *> &Fields,
      const SmallVectorImpl<unsigned> &Offsets,
      const SOAToAOSStructMethodsCheckDebugResult &InstsToTransform,
      StringRef DepTypePrefix, DTransTypeRemapper *TypeRemapper)
      : DTransOptBase(DTInfo, Context, DL, TLI, DepTypePrefix, TypeRemapper),
        InstsToTransform(InstsToTransform), S(S), Fields(Fields),
        Offsets(Offsets) {}

  bool prepareTypes(Module &M) override {
    LLVMContext &Context = M.getContext();

    NewStruct = StructType::create(
        Context, (Twine(DepTypePrefix) + S.StrType->getName()).str());
    TypeRemapper->addTypeMapping(S.StrType, NewStruct);
    NewArray = StructType::create(
        Context, (Twine(DepTypePrefix) + "AR_" + Fields[0]->getName()).str());
    NewElement = StructType::create(
        Context, (Twine(DepTypePrefix) + "EL_" + S.StrType->getName()).str());

    for (auto *Fld: Fields)
      TypeRemapper->addTypeMapping(Fld, NewArray);

    FunctionType *NewFunctionTy = nullptr;
    for (auto P : zip(InstsToTransform.Appends, Fields)) {
      auto *Method = std::get<0>(P);
      auto *FunctionTy = Method->getFunctionType();
      if (NewFunctionTy) {
        TypeRemapper->addTypeMapping(FunctionTy, NewFunctionTy);
        continue;
      }

      auto *ElementType = std::get<1>(P)
                              ->getTypeAtIndex(DTransSOAToAOSBasePtrOff)
                              ->getPointerElementType();

      SmallVector<Type *, 5> Params;
      for (auto *Param : FunctionTy->params()) {
        if (Param == ElementType) {
          for (auto *Fld : Fields)
            Params.push_back(Fld->getTypeAtIndex(DTransSOAToAOSBasePtrOff)
                                 ->getPointerElementType());
          continue;
        }

        if (auto *Ptr = dyn_cast<PointerType>(Param)) {
          if (Ptr->getElementType() == std::get<1>(P)) {
            Params.push_back(NewArray->getPointerTo());
            continue;
          }

          if (Ptr->getElementType() == ElementType) {
            for (auto *Fld : Fields)
              Params.push_back(Fld->getTypeAtIndex(DTransSOAToAOSBasePtrOff));
            continue;
          }
        }
        Params.push_back(Param);
      }

      NewFunctionTy =
          FunctionType::get(FunctionTy->getReturnType(), Params, false);

      TypeRemapper->addTypeMapping(FunctionTy, NewFunctionTy);
    }

    return true;
  }

  void populateTypes(Module &M) override {
    {
      SmallVector<Type *, 6> Elements;
      for (auto *Fld : Fields)
        Elements.push_back(Fld->getTypeAtIndex(DTransSOAToAOSBasePtrOff)
                               ->getPointerElementType());
      NewElement->setBody(Elements);
    }

    {
      SmallVector<Type *, 6> ArrayElems;
      unsigned Off = 0;
      for (auto *Fld : Fields[0]->elements()) {
        if (Off == DTransSOAToAOSBasePtrOff)
          ArrayElems.push_back(NewElement->getPointerTo(0));
        else
          ArrayElems.push_back(Fld);
        ++Off;
      }
      NewArray->setBody(ArrayElems);
    }

    {
      SmallVector<Type *, 6> StructElems;
      for (auto *Fld : S.StrType->elements())
        StructElems.push_back(Fld);

      auto PFloat = Type::getFloatTy(Context)->getPointerTo(0);
      // Do not change layout of structure, all but one
      // pointer to transformed array is replaced with pointer to 'float'.
      for (auto O : Offsets)
        StructElems[O] = PFloat;

      AOSOff = *std::min_element(Offsets.begin(), Offsets.end());

      StructElems[AOSOff] = NewArray->getPointerTo(0);
      NewStruct->setBody(StructElems);
    }
  }

  void postprocessFunction(Function &OrigFunc, bool isCloned) override {
    // InstsToTransform contains all needed information.
    // New instructions after cloning is obtained using VMap.
    if (!isCloned)
      return;

    StructMethodTransformation SMT(
        DL, DTInfo, TLI, VMap, InstsToTransform /* CsllSiteComparator */,
        InstsToTransform /*TransformationData*/, Context);

    SMT.updateReferences(S.StrType, NewArray, Fields, AOSOff,
                         DTransSOAToAOSBasePtrOff);
  }

private:
  const SOAToAOSStructMethodsCheckDebugResult &InstsToTransform;
  const SummaryForIdiom &S;
  const SmallVectorImpl<StructType *> &Fields;
  const SmallVectorImpl<unsigned> &Offsets;

  StructType *NewElement = nullptr;
  StructType *NewArray = nullptr;
  StructType *NewStruct = nullptr;
  // Offset of array-of-structure in NewStruct.
  unsigned AOSOff = -1U;
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
  auto &TLI = AM.getResult<TargetLibraryAnalysis>(M);
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  // SOAToAOSStructMethodsCheckDebug uses SOAToAOSApproximationDebug internally.
  FAM.getResult<SOAToAOSApproximationDebug>(*MethodToTest);
  auto &InstsToTransformPtr =
      FAM.getResult<SOAToAOSStructMethodsCheckDebug>(*MethodToTest);

  DTransTypeRemapper TypeRemapper;
  SOAStructMethodReplacement Transformer(
      DTInfo, M.getContext(), M.getDataLayout(), TLI, S, P.first, P.second,
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
