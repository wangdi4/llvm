//===---------------- SOAToAOSArrays.cpp - Part of SOAToAOSPass -----------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
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

// Same as in SOAToAOS.cpp.
#define DEBUG_TYPE "dtrans-soatoaos"

namespace llvm {
namespace dtrans {
namespace soatoaos {

// Offset of base pointer in DTransSOAToAOSApproxTypename.
cl::opt<unsigned> DTransSOAToAOSBasePtrOff(
    "dtrans-soatoaos-base-ptr-off", cl::init(-1U), cl::ReallyHidden,
    cl::desc("Base pointer offset in dtrans-soatoaos-approx-typename"));

// Offset of memory interface in DTransSOAToAOSApproxTypename.
cl::opt<unsigned> DTransSOAToAOSMemoryInterfaceOff(
    "dtrans-soatoaos-mem-off", cl::init(-1U), cl::ReallyHidden,
    cl::desc("Memory interface offset in dtrans-soatoaos-approx-typename"));

ArraySummaryForIdiom
getParametersForSOAToAOSArrayMethodsCheckDebug(Function &F) {

  SummaryForIdiom S = getParametersForSOAToAOSMethodsCheckDebug(F);
  ArraySummaryForIdiom Failure(nullptr, nullptr, nullptr, nullptr);

  if (!S.StrType)
    return Failure;

  if (S.StrType->getNumElements() <= DTransSOAToAOSBasePtrOff)
    return Failure;

  auto *PBase = dyn_cast<PointerType>(
      S.StrType->getTypeAtIndex(DTransSOAToAOSBasePtrOff));

  if (!PBase)
    return Failure;


  return ArraySummaryForIdiom(S, PBase->getPointerElementType());
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
  if (!Res)
    return Ignore(nullptr);
  const DepMap *DM = Res->get();
  // TODO: add diagnostic message.
  if (!DM)
    return Ignore(nullptr);

  ArraySummaryForIdiom S = getParametersForSOAToAOSArrayMethodsCheckDebug(F);
  // TODO: add diagnostic message.
  if (!S.StrType)
    return Ignore(nullptr);

  LLVM_DEBUG(dbgs() << "; Checking array's method " << F.getName() << "\n");

  std::unique_ptr<SOAToAOSArrayMethodsCheckDebugResult> Result(
      new SOAToAOSArrayMethodsCheckDebugResult());
  ComputeArrayMethodClassification MC(F.getParent()->getDataLayout(), *DM, S,
                                      *Result);
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
      const TargetLibraryInfo &TLI, const ArraySummaryForIdiom &S,
      const SOAToAOSArrayMethodsCheckDebugResult &InstsToTransform,
      StringRef DepTypePrefix, DTransTypeRemapper *TypeRemapper)
      : DTransOptBase(DTInfo, Context, DL, TLI, DepTypePrefix, TypeRemapper),
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
    PFloat = PointerType::get(Type::getFloatTy(Context), 0);

    if (InstsToTransform.MK == MK_Append) {
      // Processing single function in this debug pass.
      auto *FunctionTy = S.Method->getFunctionType();
      SmallVector<Type *, 5> Params;

      for (auto *P : FunctionTy->params()) {
        if (P == S.ElementType) {
          Params.push_back(P);
          Params.push_back(PFloat);
          NewParamOffset = Params.size() - 1;
        }
        if (auto *Ptr = dyn_cast<PointerType>(P)) {
          if (Ptr->getElementType() == S.StrType)
            Params.push_back(NewArray->getPointerTo());
          if (Ptr->getElementType() == S.ElementType) {
            Params.push_back(P);
            Params.push_back(PFloat->getPointerTo());
            NewParamOffset = Params.size() - 1;
          }
        }
      }
      auto *NewFunctionTy =
          FunctionType::get(FunctionTy->getReturnType(), Params, false);

      TypeRemapper->addTypeMapping(FunctionTy, NewFunctionTy);
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

    ArrayMethodTransformation AMT(DL, DTInfo, TLI, VMap, InstsToTransform,
                                  Context);

    bool CopyElemInsts = InstsToTransform.MK == MK_Realloc ||
                         InstsToTransform.MK == MK_Append ||
                         InstsToTransform.MK == MK_Ctor ||
                         InstsToTransform.MK == MK_CCtor;

    ArrayMethodTransformation::OrigToCopyTy OrigToCopy;

    AMT.updateBasePointerInsts(CopyElemInsts, 2, NewElement->getPointerTo(0));
    if (CopyElemInsts) {
      AMT.rawCopyAndRelink(OrigToCopy, true, 2, PFloat, NewParamOffset);
      AMT.gepRAUW(true, OrigToCopy, 0, NewElement->getPointerTo(0));
    }
    // Original instructions from cloned function should be replaced as a last
    // step to keep OrigToCopy valid.
    AMT.gepRAUW(false, OrigToCopy, 1, NewElement->getPointerTo(0));
  }

private:
  const ArraySummaryForIdiom &S;
  const SOAToAOSArrayMethodsCheckDebugResult &InstsToTransform;

  StructType *NewElement = nullptr;
  StructType *NewArray = nullptr;

  // Cached types for transformation.
  // NewElement*
  PointerType *PNewElement = nullptr;
  // float*
  PointerType *PFloat = nullptr;
  // Offset of new parameter for MK_Append method.
  unsigned NewParamOffset = -1U;
};
} // namespace

PreservedAnalyses
SOAToAOSArrayMethodsTransformDebug::run(Module &M, ModuleAnalysisManager &AM) {

  // It should be lit-test with single defined function at this point.
  Function *MethodToTest = nullptr;
  for (auto &F : M)
    if (!F.isDeclaration()) {
      if (!MethodToTest)
        MethodToTest = &F;
      else
        // TODO: add diagnostic.
        return PreservedAnalyses::all();
    }

  // TODO: add diagnostic.
  if (!MethodToTest)
    return PreservedAnalyses::all();

  ArraySummaryForIdiom S =
      getParametersForSOAToAOSArrayMethodsCheckDebug(*MethodToTest);
  // TODO: add diagnostic.
  if (!S.StrType)
    return PreservedAnalyses::all();

  auto &DTInfo = AM.getResult<DTransAnalysis>(M);
  auto &TLI = AM.getResult<TargetLibraryAnalysis>(M);
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  // SOAToAOSArrayMethodsCheckDebug uses SOAToAOSApproximationDebug internally.
  FAM.getResult<SOAToAOSApproximationDebug>(*MethodToTest);
  auto &InstsToTransformPtr =
      FAM.getResult<SOAToAOSArrayMethodsCheckDebug>(*MethodToTest);

  // TODO: add diagnostic.
  if (InstsToTransformPtr.get()->MK == MK_Unknown)
    return PreservedAnalyses::all();

  DTransTypeRemapper TypeRemapper;

  SOAArrayMethodReplacement Transformer(
      DTInfo, M.getContext(), M.getDataLayout(), TLI, S,
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
