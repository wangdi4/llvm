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

#include "llvm/IR/InstIterator.h"
#include "llvm/Pass.h"
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

SummaryForIdiom getParametersForSOAToAOSMethodsCheckDebug(Function &F) {
  StructType *ClassType = getStructTypeOfArray(F);

  SummaryForIdiom Failure(nullptr, nullptr, nullptr, nullptr);

  if (!ClassType)
    return Failure;

  if (ClassType->getNumElements() <= DTransSOAToAOSBasePtrOff)
    return Failure;

  auto *PBase = dyn_cast<PointerType>(
      ClassType->getTypeAtIndex(DTransSOAToAOSBasePtrOff));

  if (!PBase)
    return Failure;

  StructType *MemoryInterface = nullptr;
  if (ClassType->getNumElements() > DTransSOAToAOSMemoryInterfaceOff) {
    auto *PMemInt = dyn_cast<PointerType>(
      ClassType->getTypeAtIndex(DTransSOAToAOSMemoryInterfaceOff));
    if (!PMemInt)
      return Failure;
    MemoryInterface = dyn_cast<StructType>(PMemInt->getPointerElementType());
  }

  return SummaryForIdiom(ClassType, PBase->getPointerElementType(),
                         MemoryInterface, &F);
}
} // namespace soatoaos

using namespace soatoaos;

char SOAToAOSMethodsCheckDebug::PassID;

// Provide a definition for the static class member used to identify passes.
AnalysisKey SOAToAOSMethodsCheckDebug::Key;

SOAToAOSMethodsCheckDebug::Ignore::Ignore(SOAToAOSMethodsCheckDebugResult *Ptr)
    : Ptr(Ptr) {}
SOAToAOSMethodsCheckDebug::Ignore::Ignore(Ignore &&Other)
    : Ptr(std::move(Other.Ptr)) {}
const SOAToAOSMethodsCheckDebugResult *
SOAToAOSMethodsCheckDebug::Ignore::get() const {
  return Ptr.get();
}
SOAToAOSMethodsCheckDebug::Ignore::~Ignore() {}

SOAToAOSMethodsCheckDebug::Ignore
SOAToAOSMethodsCheckDebug::run(Function &F, FunctionAnalysisManager &AM) {

  auto *Res = AM.getCachedResult<SOAToAOSApproximationDebug>(F);
  if (!Res)
    return Ignore(nullptr);
  const DepMap *DM = Res->get();
  // TODO: add diagnostic message.
  if (!DM)
    return Ignore(nullptr);

  SummaryForIdiom S = getParametersForSOAToAOSMethodsCheckDebug(F);
  // TODO: add diagnostic message.
  if (!S.ArrType)
    return Ignore(nullptr);

  LLVM_DEBUG(dbgs() << "; Checking array's method " << F.getName() << "\n");

  std::unique_ptr<SOAToAOSMethodsCheckDebugResult> Result(
      new SOAToAOSMethodsCheckDebugResult());
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
      const TargetLibraryInfo &TLI, const SummaryForIdiom &S,
      const SOAToAOSMethodsCheckDebugResult &InstsToTransform,
      StringRef DepTypePrefix, DTransTypeRemapper *TypeRemapper)
      : DTransOptBase(DTInfo, Context, DL, TLI, DepTypePrefix, TypeRemapper),
        S(S), InstsToTransform(InstsToTransform) {}

  bool prepareTypes(Module &M) override {
    LLVMContext &Context = M.getContext();

    NewArray = StructType::create(
        Context, (Twine(DepTypePrefix) + S.ArrType->getName()).str());
    NewElement = StructType::create(
        Context, (Twine(DepTypePrefix) + "EL_" + S.ArrType->getName()).str());

    TypeRemapper->addTypeMapping(S.ArrType, NewArray);
    return true;
  }

  void populateTypes(Module &M) override {
    PFloat = PointerType::get(Type::getFloatTy(Context), 0);

    // S.ElementType is pointer type too.
    // For testing purposes assume that the second element type is simply
    // float*.
    NewElement->setBody(PFloat, S.ElementType);

    SmallVector<Type *, 6> DataTypes;
    for (auto *T : S.ArrType->elements()) {
      if (auto *PT = dyn_cast<PointerType>(T))
        if (PT->getElementType() == S.ElementType) {
          // Replace base pointer
          DataTypes.push_back(NewElement->getPointerTo(0));
          continue;
        }
      DataTypes.push_back(T);
    }
    NewArray->setBody(DataTypes, S.ArrType->isPacked());
  }

  void postprocessFunction(Function &OrigFunc, bool isCloned) override {
    // InstsToTransform contains all needed information.
    // New instructions after cloning is obtained using VMap.
    if (!isCloned)
      return;

    ArrayMethodTransformation AMT(DL, DTInfo, TLI, VMap, InstsToTransform,
                                  Context);

    bool CopyElemInsts = InstsToTransform.MK == MK_Realloc ||
                         InstsToTransform.MK == MK_Ctor ||
                         InstsToTransform.MK == MK_CCtor;

    DenseMap<Instruction *, Instruction *> OrigToCopy;

    AMT.updateBasePointerInsts(CopyElemInsts, 2, NewElement->getPointerTo(0));
    if (CopyElemInsts) {
      AMT.rawCopyAndRelink(OrigToCopy, true, 2);
      AMT.gepRAUW(true, OrigToCopy, 0, NewElement->getPointerTo(0));
    }
    // Original instructions from cloned function should be replaced as a last
    // step to keep OrigToCopy valid.
    AMT.gepRAUW(false, OrigToCopy, 1, NewElement->getPointerTo(0));
  }

private:
  const SummaryForIdiom &S;
  const SOAToAOSMethodsCheckDebugResult &InstsToTransform;

  StructType *NewElement = nullptr;
  StructType *NewArray = nullptr;

  // Cached types for transformation.
  // NewElement*
  PointerType *PNewElement = nullptr;
  // float*
  PointerType *PFloat = nullptr;
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

  SummaryForIdiom S = getParametersForSOAToAOSMethodsCheckDebug(*MethodToTest);
  // TODO: add diagnostic.
  if (!S.ArrType)
    return PreservedAnalyses::all();

  auto &DTInfo = AM.getResult<DTransAnalysis>(M);
  auto &TLI = AM.getResult<TargetLibraryAnalysis>(M);
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  // SOAToAOSMethodsCheckDebug uses SOAToAOSApproximationDebug internally.
  FAM.getResult<SOAToAOSApproximationDebug>(*MethodToTest);
  auto &InstsToTransformPtr =
      FAM.getResult<SOAToAOSMethodsCheckDebug>(*MethodToTest);

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
