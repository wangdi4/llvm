//===---------------- SOAToAOSArrays.cpp - Part of SOAToAOSPass -----------===//
//
// Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements debug functionality related specifically to array
// structures for SOA-to-AOS: method analysis and transformations.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/Compiler.h"

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
#include "SOAToAOSArrays.h"

#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "Intel_DTrans/Transforms/DTransOptBase.h"
#include "Intel_DTrans/Transforms/SOAToAOS.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Error.h"

// Same as in SOAToAOS.cpp.
#define DEBUG_TYPE "dtrans-soatoaos"

namespace llvm {
namespace dtrans {
namespace soatoaos {

// Offset of base pointer in array type.
cl::opt<unsigned> DTransSOAToAOSBasePtrOff(
    "dtrans-soatoaos-base-ptr-off", cl::init(-1U), cl::ReallyHidden,
    cl::desc("Base pointer offset in array structure"));

ArraySummaryForIdiom
getParametersForSOAToAOSArrayMethodsCheckDebug(Function &F) {

  SummaryForIdiom S = getParametersForSOAToAOSMethodsCheckDebug(F);

  if (S.StrType->getNumElements() <= DTransSOAToAOSBasePtrOff)
    report_fatal_error("Incorrect base pointer specification: "
                       "dtrans-soatoaos-base-ptr-off points beyond last "
                       "element of array structure.");

  auto *PBase = dyn_cast<PointerType>(
      S.StrType->getTypeAtIndex(DTransSOAToAOSBasePtrOff));

  if (!PBase || !isa<PointerType>(PBase->getElementType()))
    report_fatal_error(
        "Incorrect base pointer specification: "
        "type at dtrans-soatoaos-base-ptr-off offset is not pointer to pointer.");

  return ArraySummaryForIdiom(S, cast<PointerType>(PBase->getElementType()));
}
} // namespace soatoaos

using namespace soatoaos;

char SOAToAOSArrayMethodsCheckDebug::PassID;

// Provide a definition for the static class member used to identify passes.
AnalysisKey SOAToAOSArrayMethodsCheckDebug::Key;

SOAToAOSArrayMethodsCheckDebug::Ignore::Ignore(
    SOAToAOSArrayMethodsCheckDebugResult *Ptr)
    : Ptr(Ptr) {}
SOAToAOSArrayMethodsCheckDebug::Ignore::Ignore(Ignore &&Other)
    : Ptr(std::move(Other.Ptr)) {}
const SOAToAOSArrayMethodsCheckDebugResult *
SOAToAOSArrayMethodsCheckDebug::Ignore::get() const {
  return Ptr.get();
}
SOAToAOSArrayMethodsCheckDebug::Ignore::~Ignore() {}

SOAToAOSArrayMethodsCheckDebug::Ignore
SOAToAOSArrayMethodsCheckDebug::run(Function &F, FunctionAnalysisManager &AM) {

  auto *Res = AM.getCachedResult<SOAToAOSApproximationDebug>(F);
  auto &TLI = AM.getResult<TargetLibraryAnalysis>(F);
  if (!Res)
    report_fatal_error("SOAToAOSApproximationDebug was not run before "
                       "SOAToAOSArrayMethodsCheckDebug.");

  const DepMap *DM = Res->get();
  if (!DM)
    report_fatal_error("Missing SOAToAOSApproximationDebug results before "
                       "SOAToAOSArrayMethodsCheckDebug.");

  ArraySummaryForIdiom S = getParametersForSOAToAOSArrayMethodsCheckDebug(F);

  LLVM_DEBUG(dbgs() << "; Checking array's method " << F.getName() << "\n");

  std::unique_ptr<SOAToAOSArrayMethodsCheckDebugResult> Result(
      new SOAToAOSArrayMethodsCheckDebugResult());
  ComputeArrayMethodClassification MC(F.getParent()->getDataLayout(), *DM, S,
                                      *Result, TLI);
  Result->MK = MC.classify().first;
  LLVM_DEBUG(dbgs() << "; Classification: " << Result->MK << "\n");

  // Dump results of analysis.
  DEBUG_WITH_TYPE(DTRANS_SOAARR, {
    dbgs() << "; Dump instructions needing update. Total = " << MC.getTotal();
    ComputeArrayMethodClassification::AnnotatedWriter Annotate(MC);
    F.print(dbgs(), &Annotate);
  });
  return Ignore(Result.release());
}

namespace {
class SOAArrayMethodReplacement : public DTransOptBase {
public:
  SOAArrayMethodReplacement(
      DTransAnalysisInfo &DTInfo, LLVMContext &Context, const DataLayout &DL,
      std::function<const TargetLibraryInfo &(const Function &)> GetTLI,
      const ArraySummaryForIdiom &S,
      const SOAToAOSArrayMethodsCheckDebugResult &InstsToTransform,
      StringRef DepTypePrefix, DTransTypeRemapper *TypeRemapper)
      : DTransOptBase(&DTInfo, Context, DL, GetTLI, DepTypePrefix,
                      TypeRemapper),
        S(S), InstsToTransform(InstsToTransform) {}

  bool prepareTypes(Module &M) override {
    LLVMContext &Context = M.getContext();

    NewArray = StructType::create(
        Context, (Twine(DepTypePrefix) + S.StrType->getName()).str());
    NewElement = StructType::create(
        Context, (Twine(DepTypePrefix) + "EL_" + S.StrType->getName()).str());

    TypeRemapper->addTypeMapping(S.StrType, NewArray);

    // For testing purposes assume that the second element type is simply
    // float*.
    PFloat = Type::getFloatTy(Context)->getPointerTo(0);

    if (InstsToTransform.MK == MK_Append) {
      SmallVector<PointerType *, 2> Elems;
      Elems.push_back(S.ElementType);
      Elems.push_back(PFloat);

      ArrayMethodTransformation::mapNewAppendType(
          *S.Method, S.ElementType, Elems, TypeRemapper,
          AppendMethodElemParamOffset);
    }
    return true;
  }

  void populateTypes(Module &M) override {
    // S.ElementType is pointer type too.
    // For testing purposes assume that the second element type is simply
    // float*.
    NewElement->setBody(PFloat, S.ElementType);

    SmallVector<Type *, 6> DataTypes;
    for (auto *T : S.StrType->elements()) {
      if (auto *PT = dyn_cast<PointerType>(T))
        if (PT->getElementType() == S.ElementType) {
          // Replace base pointer
          DataTypes.push_back(NewElement->getPointerTo(0));
          continue;
        }
      DataTypes.push_back(T);
    }
    NewArray->setBody(DataTypes, S.StrType->isPacked());
  }

  void postprocessFunction(Function &OrigFunc, bool isCloned) override {
    // InstsToTransform contains all needed information.
    // New instructions after cloning is obtained using VMap.
    if (!isCloned)
      return;
    const TargetLibraryInfo &TLI = GetTLI(OrigFunc);
    ArrayMethodTransformation AMT(DL, *DTInfo, TLI, VMap, InstsToTransform,
                                  Context);

    bool CopyElemInsts = InstsToTransform.MK == MK_Realloc ||
                         InstsToTransform.MK == MK_Append ||
                         InstsToTransform.MK == MK_Ctor ||
                         InstsToTransform.MK == MK_CCtor;

    AMT.updateBasePointerInsts(CopyElemInsts, 2/*Number of arrays*/,
                               NewElement->getPointerTo(0));
    if (CopyElemInsts) {
      ArrayMethodTransformation::OrigToCopyTy OrigToCopy;
      AMT.rawCopyAndRelink(OrigToCopy,
                           true /* Update unique instructions like memset*/,
                           2 /*Number of arrays*/, PFloat /*New element type*/,
                           AppendMethodElemParamOffset +
                               1 /* Offset in argument list of new element*/);
      AMT.gepRAUW(true /*Do copy*/, OrigToCopy,
                  0 /*PFloat's offset in NewElement*/,
                  NewElement->getPointerTo(0));
    }
    // Original instructions from cloned function should be replaced as a last
    // step to keep OrigToCopy valid.
    AMT.gepRAUW(false /*Only update existing insts*/,
                ArrayMethodTransformation::OrigToCopyTy(),
                1 /*S.ElementType's offset in NewElement*/,
                NewElement->getPointerTo(0));
  }

private:
  const ArraySummaryForIdiom &S;
  const SOAToAOSArrayMethodsCheckDebugResult &InstsToTransform;

  // Structure containing one element per each transformed array.
  StructType *NewElement = nullptr;
  // New array of structures, structure is NewElement.
  StructType *NewArray = nullptr;

  // float*, arbitrary 2nd type in NewElement
  PointerType *PFloat = nullptr;
  // Offset of the first element parameter for MK_Append method.
  // Element parameters are arranged in order of NewElement.
  unsigned AppendMethodElemParamOffset = -1U;
};
} // namespace

PreservedAnalyses
SOAToAOSArrayMethodsTransformDebug::run(Module &M, ModuleAnalysisManager &AM) {

  // It should be lit-test with single defined function at this point.
  Function *MethodToTest = nullptr;
  for (auto &F : M)
    if (!F.isDeclaration()) {
      if (MethodToTest)
        report_fatal_error("Single function definition per compilation unit is "
                           "allowed in SOAToAOSArrayMethodsTransformDebug.");
      MethodToTest = &F;
    }

  if (!MethodToTest)
    report_fatal_error(
        "Exactly one function definition per compilation unit is "
        "required in SOAToAOSArrayMethodsTransformDebug.");

  ArraySummaryForIdiom S =
      getParametersForSOAToAOSArrayMethodsCheckDebug(*MethodToTest);

  auto &DTInfo = AM.getResult<DTransAnalysis>(M);
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetTLI = [&FAM](const Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(*(const_cast<Function*>(&F)));
  };
  // SOAToAOSArrayMethodsCheckDebug uses SOAToAOSApproximationDebug internally.
  FAM.getResult<SOAToAOSApproximationDebug>(*MethodToTest);
  auto &InstsToTransformPtr =
      FAM.getResult<SOAToAOSArrayMethodsCheckDebug>(*MethodToTest);

  if (InstsToTransformPtr.get()->MK == MK_Unknown)
    report_fatal_error("Please debug array method's classification before "
                       "attempting to transform it.");

  DTransTypeRemapper TypeRemapper;

  SOAArrayMethodReplacement Transformer(
      DTInfo, M.getContext(), M.getDataLayout(), GetTLI, S,
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
