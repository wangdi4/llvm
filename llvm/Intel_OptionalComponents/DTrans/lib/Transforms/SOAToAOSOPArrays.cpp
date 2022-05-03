//===-------------- SOAToAOSOPArrays.cpp - Part of SOAToAOSOPPass ---------===//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements debug functionality related specifically to array
// structures for SOA-to-AOS-OP: method analysis and transformations.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/Compiler.h"

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
#include "SOAToAOSOPArrays.h"

#include "Intel_DTrans/Analysis/DTransSafetyAnalyzer.h"
#include "Intel_DTrans/Analysis/DTransTypeMetadataBuilder.h"
#include "Intel_DTrans/Transforms/DTransOPOptBase.h"
#include "Intel_DTrans/Transforms/SOAToAOSOP.h"

#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Error.h"
#include "llvm/Transforms/Utils/Cloning.h"

// Same as in SOAToAOSOP.cpp.
#define DEBUG_TYPE "dtrans-soatoaosop"

namespace llvm {
namespace dtransOP {
namespace soatoaosOP {

// Offset of base pointer in array type.
cl::opt<unsigned> DTransSOAToAOSOPBasePtrOff(
    "dtrans-soatoaosop-base-ptr-off", cl::init(-1U), cl::ReallyHidden,
    cl::desc("Base pointer offset in array structure"));

ArraySummaryForIdiom
getParametersForSOAToAOSArrayMethodsCheckDebug(Function &F,
                                               DTransSafetyInfo *DTInfo) {

  SummaryForIdiom S = getParametersForSOAToAOSMethodsCheckDebug(F, DTInfo);

  if (S.StrType->getNumFields() <= DTransSOAToAOSOPBasePtrOff)
    report_fatal_error("Incorrect base pointer specification: "
                       "dtrans-soatoaosop-base-ptr-off points beyond last "
                       "element of array structure.");

  auto *PBase = dyn_cast<DTransPointerType>(
      S.StrType->getFieldType(DTransSOAToAOSOPBasePtrOff));

  if (!PBase || !isa<DTransPointerType>(PBase->getPointerElementType()))
    report_fatal_error("Incorrect base pointer specification: "
                       "type at dtrans-soatoaosop-base-ptr-off offset is not "
                       "pointer to pointer.");

  return ArraySummaryForIdiom(
      S, cast<DTransPointerType>(PBase->getPointerElementType()));
}
} // namespace soatoaosOP

using namespace soatoaosOP;

char SOAToAOSOPArrayMethodsCheckDebug::PassID;

// Provide a definition for the static class member used to identify passes.
AnalysisKey SOAToAOSOPArrayMethodsCheckDebug::Key;

SOAToAOSOPArrayMethodsCheckDebug::Ignore::Ignore(
    SOAToAOSOPArrayMethodsCheckDebugResult *Ptr)
    : Ptr(Ptr) {}
SOAToAOSOPArrayMethodsCheckDebug::Ignore::Ignore(Ignore &&Other)
    : Ptr(std::move(Other.Ptr)) {}
const SOAToAOSOPArrayMethodsCheckDebugResult *
SOAToAOSOPArrayMethodsCheckDebug::Ignore::get() const {
  return Ptr.get();
}
SOAToAOSOPArrayMethodsCheckDebug::Ignore::~Ignore() {}

SOAToAOSOPArrayMethodsCheckDebug::Ignore
SOAToAOSOPArrayMethodsCheckDebug::run(Module &M, ModuleAnalysisManager &MAM) {

  auto &FAM = MAM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto *Res = &MAM.getResult<SOAToAOSOPApproximationDebug>(M);
  const DepMap *DM = Res->get();
  if (!DM)
    report_fatal_error("Missing SOAToAOSOPApproximationDebug results before "
                       "SOAToAOSOPArrayMethodsCheckDebug.");

  auto *DTInfo = &MAM.getResult<DTransSafetyAnalyzer>(M);

  std::unique_ptr<SOAToAOSOPArrayMethodsCheckDebugResult> Result(
      new SOAToAOSOPArrayMethodsCheckDebugResult());
  for (auto &F : M) {
    if (F.isDeclaration())
      continue;
    if (isFunctionIgnoredForSOAToAOSOP(F))
      continue;

    ArraySummaryForIdiom S =
        getParametersForSOAToAOSArrayMethodsCheckDebug(F, DTInfo);

    LLVM_DEBUG(dbgs() << "; Checking array's method " << F.getName() << "\n");

    auto &TLI =
        FAM.getResult<TargetLibraryAnalysis>(*(const_cast<Function *>(&F)));
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
  }
  return Ignore(Result.release());
}

namespace {
class SOAArrayMethodReplacement : public DTransOPOptBase {
public:
  SOAArrayMethodReplacement(
      DTransSafetyInfo &DTInfo, LLVMContext &Context, const DataLayout &DL,
      std::function<const TargetLibraryInfo &(const Function &)> GetTLI,
      const ArraySummaryForIdiom &S,
      const SOAToAOSOPArrayMethodsCheckDebugResult &InstsToTransform,
      StringRef DepTypePrefix)
      : DTransOPOptBase(Context, &DTInfo, DepTypePrefix), S(S),
        InstsToTransform(InstsToTransform), DL(DL), GetTLI(GetTLI) {}

  bool prepareTypes(Module &M) override {
    LLVMContext &Context = M.getContext();

    NewLLVMArray = StructType::create(
        Context, (Twine(DepTypePrefix) + S.StrType->getName()).str());
    NewLLVMElement = StructType::create(
        Context, (Twine(DepTypePrefix) + "EL_" + S.StrType->getName()).str());
    NewDTElement = TM.getOrCreateStructType(NewLLVMElement);

    NewDTArray = TM.getOrCreateStructType(NewLLVMArray);
    TypeRemapper.addTypeMapping(S.StrType->getLLVMType(), NewLLVMArray,
                                S.StrType, NewDTArray);

    // For testing purposes assume that the second element type is simply
    // float*.
    auto *DTFPType = TM.getOrCreateAtomicType(Type::getFloatTy(Context));
    PLLVMFloat = Type::getFloatTy(Context)->getPointerTo(0);
    PFloat = TM.getOrCreatePointerType(DTFPType);

    return true;
  }

  void populateTypes(Module &M) override {
    // S.ElementType is pointer type too.
    // For testing purposes assume that the second element type is simply
    // float*.
    NewLLVMElement->setBody(PLLVMFloat, S.ElementType->getLLVMType());
    SmallVector<DTransType *, 6> DTDataTypes;
    DTDataTypes.push_back(PFloat);
    DTDataTypes.push_back(S.ElementType);
    NewDTElement->setBody(DTDataTypes);

    SmallVector<Type *, 6> DataTypes;
    SmallVector<DTransType *, 6> DTElemDataTypes;
    size_t NumFields = S.StrType->getNumFields();
    for (size_t Idx = 0; Idx < NumFields; ++Idx) {
      DTransType *T = S.StrType->getFieldType(Idx);
      assert(T && "Invalid DTrans structure type");
      if (auto *PT = dyn_cast<DTransPointerType>(T))
        if (PT->getPointerElementType() == S.ElementType) {
          // Replace base pointer
          DataTypes.push_back(NewLLVMElement->getPointerTo(0));
          DTElemDataTypes.push_back(TM.getOrCreatePointerType(NewDTElement));
          continue;
        }
      DataTypes.push_back(T->getLLVMType());
      DTElemDataTypes.push_back(T);
    }
    NewLLVMArray->setBody(DataTypes, S.StrType->isPacked());
    NewDTArray->setBody(DTElemDataTypes);
  }

  // For “Append” member function, new parameters are added during the
  // transformation. Earlier, old function types of “Append” member function
  // were mapped to new function types in “prepareTypes” so that OptBase class
  // does the conversion from old types to the new types. But, that doesn’t work
  // with opaque pointers since all “Append” member functions have same function
  // type. New “Append” functions are created here and then mapped to the
  // old “Append” member functions in VMap so that OptBase class clones new
  // “Append” member functions and replaces old “Append” member functions with
  // new “Append” member functions in entire Module.
  void prepareModule(Module &M) override {
    auto *ArrType =
        getOPStructTypeOfMethod(const_cast<Function *>(S.Method), S.DTInfo);
    if (InstsToTransform.MK == MK_Append) {
      SmallVector<DTransPointerType *, 2> Elems;
      Elems.push_back(S.ElementType);
      Elems.push_back(PFloat);

      auto *NewDTFunctionTy = ArrayMethodTransformation::mapNewAppendType(
          *S.Method, S.ElementType, Elems, ArrType, &TypeRemapper,
          AppendMethodElemParamOffset, DTInfo->getTypeMetadataReader(),
          DTInfo->getTypeManager());
      createAndMapNewAppendFunc(const_cast<llvm::Function *>(S.Method), M,
                                NewDTFunctionTy, VMap, OrigFuncToCloneFuncMap,
                                CloneFuncToOrigFuncMap,
                                AppendsFuncToDTransTyMap);
    }
  }

  void postprocessFunction(Function &OrigFunc, bool isCloned) override {
    if (isFunctionIgnoredForSOAToAOSOP(OrigFunc))
      return;

    ValueToValueMapTy NewVMap;

    // For typed pointers, all member functions will be cloned by
    // DTransOPOptBase. For opaque pointers, none of the member functions
    // will be cloned except for MK_Append. For MK_Append, prototype is
    // completely different because new params are added.
    // The original implementation is designed to work on cloned routines.
    // First, it collects instructions to transform (i.e InstsToTransform)
    // in the pre-cloned function and then modifies the instructions in
    // cloned routine by getting corresponding instructions by using VMap.
    // Since the cloning is not happening with opaque pointers, clone
    // OrigFunc temporarily, modify the instructions in the cloned routine
    // and then move the modified instructions to the OrigFunc. Cloned
    // routine will be deleted at end of this routine.
    Function *Clone;
    if (!isCloned) {
      Clone = CloneFunction(&OrigFunc, NewVMap);
      fixCallInfo(OrigFunc, DTInfo, NewVMap);
    }
    const TargetLibraryInfo &TLI = GetTLI(isCloned ? OrigFunc : *Clone);
    // InstsToTransform contains all needed information.
    // New instructions after cloning are obtained using VMap or NewVMap
    // depending whether it is using temporary Clone or not.
    ArrayMethodTransformation AMT(DL, *DTInfo, TLI, isCloned ? VMap : NewVMap,
                                  InstsToTransform, OrigFunc.getContext());

    bool CopyElemInsts =
        InstsToTransform.MK == MK_Realloc || InstsToTransform.MK == MK_Append ||
        InstsToTransform.MK == MK_Ctor || InstsToTransform.MK == MK_CCtor;

    // Create StoreInst early for each field of newly created SOAToAOSOP array
    // element.
    ArrayMethodTransformation::ClonedElemLoadStoreMapTy ClonedElemLoadStoreMap;
    if (CopyElemInsts)
      AMT.earlyCloneElemLoadStoreInst(1 /* Element Offset */,
                                      ClonedElemLoadStoreMap);

    DTransPointerType *NBT = TM.getOrCreatePointerType(NewDTElement);
    AMT.updateBasePointerInsts(CopyElemInsts, 2 /*Number of arrays*/,
                               TM.getOrCreatePointerType(NewDTElement),
                               TM.getOrCreatePointerType(NBT));
    if (CopyElemInsts) {
      ArrayMethodTransformation::OrigToCopyTy OrigToCopy;
      AMT.rawCopyAndRelink(OrigToCopy,
                           true /* Update unique instructions like memset*/,
                           2 /*Number of arrays*/, PFloat /*New element type*/,
                           AppendMethodElemParamOffset +
                               1 /* Offset in argument list of new element*/,
                           ClonedElemLoadStoreMap, 1 /* Element Offset */);
      AMT.gepRAUW(true /*Do copy*/, OrigToCopy,
                  0 /*PFloat's offset in NewDTElement*/,
                  TM.getOrCreatePointerType(NewDTElement));
    }

    // Original instructions from cloned function should be replaced as a last
    // step to keep OrigToCopy valid.
    AMT.gepRAUW(false /*Only update existing insts*/,
                ArrayMethodTransformation::OrigToCopyTy(),
                1 /*S.ElementType's offset in NewDTElement*/,
                TM.getOrCreatePointerType(NewDTElement));

    // If temporary cloned is used to fix IR, just move the IR to the
    // OrigFunc.
    if (!isCloned)
      replaceOrigFuncBodyWithClonedFuncBody(OrigFunc, *Clone);

    // Fix DTransFunctionType of new “Append” member function.
    auto *NewFunc = isCloned ? OrigFuncToCloneFuncMap[&OrigFunc] : &OrigFunc;
    auto *AppendFuncDTy = AppendsFuncToDTransTyMap[NewFunc];
    if (AppendFuncDTy)
      DTransTypeMetadataBuilder::setDTransFuncMetadata(NewFunc, AppendFuncDTy);

    // TODO: Remove unnecessary BitCast instructions that convert from
    // pointer to pointer.
  }

private:
  const ArraySummaryForIdiom &S;
  const SOAToAOSOPArrayMethodsCheckDebugResult &InstsToTransform;

  // Structure containing one element per each transformed array.
  StructType *NewLLVMElement = nullptr;
  // DTStructure that corresponds to NewLLVMElement.
  DTransStructType *NewDTElement = nullptr;
  // New array of structures, structure is NewDTElement.
  StructType *NewLLVMArray = nullptr;
  // DTStructure that corresponds to NewLLVMArray.
  DTransStructType *NewDTArray = nullptr;

  // float*, arbitrary 2nd type in NewDTElement
  DTransPointerType *PFloat = nullptr;
  PointerType *PLLVMFloat = nullptr;
  // Offset of the first element parameter for MK_Append method.
  // Element parameters are arranged in order of NewDTElement.
  unsigned AppendMethodElemParamOffset = -1U;
  const DataLayout &DL;
  std::function<const TargetLibraryInfo &(const Function &)> GetTLI;

  // Mapping between new “Append” member functions and DTransFunctionTypes.
  SmallDenseMap<Function *, DTransFunctionType *> AppendsFuncToDTransTyMap;
};
} // namespace

PreservedAnalyses
SOAToAOSArrayMethodsTransformDebug::run(Module &M, ModuleAnalysisManager &AM) {

  // It should be lit-test with single defined function at this point.
  Function *MethodToTest = nullptr;
  for (auto &F : M) {
    if (isFunctionIgnoredForSOAToAOSOP(F))
      continue;

    if (!F.isDeclaration()) {
      if (MethodToTest)
        report_fatal_error("Single function definition per compilation unit is "
                           "allowed in SOAToAOSArrayMethodsTransformDebug.");
      MethodToTest = &F;
    }
  }

  if (!MethodToTest)
    report_fatal_error(
        "Exactly one function definition per compilation unit is "
        "required in SOAToAOSArrayMethodsTransformDebug.");

  auto &DTInfo = AM.getResult<DTransSafetyAnalyzer>(M);
  ArraySummaryForIdiom S =
      getParametersForSOAToAOSArrayMethodsCheckDebug(*MethodToTest, &DTInfo);

  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetTLI = [&FAM](const Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(*(const_cast<Function *>(&F)));
  };
  // SOAToAOSOPArrayMethodsCheckDebug uses SOAToAOSOPApproximationDebug
  // internally.
  AM.getResult<SOAToAOSOPApproximationDebug>(M);
  auto &InstsToTransformPtr = AM.getResult<SOAToAOSOPArrayMethodsCheckDebug>(M);
  if (InstsToTransformPtr.get()->MK == MK_Unknown)
    report_fatal_error("Please debug array method's classification before "
                       "attempting to transform it.");

  SOAArrayMethodReplacement Transformer(DTInfo, M.getContext(),
                                        M.getDataLayout(), GetTLI, S,
                                        *InstsToTransformPtr.get(), "__SOA_");

  bool Changed = Transformer.run(M);
  if (!Changed)
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}

} // namespace dtransOP
} // namespace llvm
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
