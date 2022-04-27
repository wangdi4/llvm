//===-------------- SOAToAOSOPStruct.cpp - Part of SOAToAOSOPPass ---------===//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements debug functionality related specifically to structures
// containing arrays for SOA-to-AOS-OP: method analysis and transformations.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/Compiler.h"

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
#include "SOAToAOSOPStruct.h"

#include "Intel_DTrans/Analysis/DTransSafetyAnalyzer.h"
#include "Intel_DTrans/Analysis/DTransTypeMetadataBuilder.h"
#include "Intel_DTrans/Transforms/DTransOPOptBase.h"
#include "Intel_DTrans/Transforms/SOAToAOSOP.h"

#include "llvm/Support/CommandLine.h"

// Same as in SOAToAOSOP.cpp.
#define DEBUG_TYPE "dtrans-soatoaosop"

namespace llvm {
namespace dtransOP {
namespace soatoaosOP {
// Array types for structure containing arrays.
static cl::list<std::string>
    DTransSOAToAOSOPArrays("dtrans-soatoaosop-array-type", cl::ReallyHidden);

// Methods of arrays, which are append-like.
static cl::list<std::string>
    DTransSOAToAOSOPAppends("dtrans-soatoaosop-array-append", cl::ReallyHidden);
// Methods of arrays, which are copy ctors.
static cl::list<std::string>
    DTransSOAToAOSOPCCtors("dtrans-soatoaosop-array-cctor", cl::ReallyHidden);
// Methods of arrays, which are regular ctors.
static cl::list<std::string>
    DTransSOAToAOSOPCtors("dtrans-soatoaosop-array-ctor", cl::ReallyHidden);
// Methods of arrays, which are dtors.
static cl::list<std::string>
    DTransSOAToAOSOPDtors("dtrans-soatoaosop-array-dtor", cl::ReallyHidden);

// CallSite comparisons to perform, requires corresponding
// dtrans-soatoaosop-arrays-* to be set.
// Valid values are cctor, ctor, dtor, append.
static cl::list<std::string>
    DTransSOAToAOSOPComparison("dtrans-soatoaosop-method-call-site-comparison",
                               cl::ReallyHidden);

static std::pair<SmallVector<DTransStructType *, 3>, SmallVector<unsigned, 3>>
getArrayTypesForSOAToAOSOPStructMethodsCheckDebug(Function &F,
                                                  DTransSafetyInfo *DTInfo) {
  SmallVector<DTransStructType *, 3> ArrayTypes;
  for (auto &Name : DTransSOAToAOSOPArrays) {
    auto *ArrType = StructType::getTypeByName(F.getContext(), Name);
    if (!ArrType)
      report_fatal_error(Twine("Cannot find struct/class type ") + Name + ".");
    ArrayTypes.push_back(
        DTInfo->getTypeManager().getOrCreateStructType(ArrType));
  }

  if (ArrayTypes.size() <= 1)
    report_fatal_error("0 or 1 array types were provided, need 2 or more.");

  auto *Struct = getOPStructTypeOfMethod(&F, DTInfo);
  if (!Struct)
    report_fatal_error(Twine("Cannot extract struct/class type from ") +
                       F.getName() + ".");

  SmallVector<unsigned, 3> ArrayOffsets;
  for (unsigned I = 0, E = Struct->getNumFields(); I != E; ++I) {
    auto *PTy = dyn_cast<DTransPointerType>(Struct->getFieldType(I));
    if (!PTy)
      continue;

    auto *STy = dyn_cast<DTransStructType>(PTy->getPointerElementType());
    if (STy && std::find(ArrayTypes.begin(), ArrayTypes.end(), STy) !=
                   ArrayTypes.end())
      ArrayOffsets.push_back(I);
  }

  if (ArrayTypes.size() != ArrayOffsets.size())
    return std::make_pair(SmallVector<DTransStructType *, 3>(),
                          SmallVector<unsigned, 3>());

  return std::make_pair(ArrayTypes, ArrayOffsets);
}

static CallSiteComparator::FunctionSet
getSOAToAOSOPArrayMethods(const cl::list<std::string> &List,
                          const SmallVector<DTransStructType *, 3> &ArrayTypes,
                          Function &F, DTransSafetyInfo *DTInfo) {
  CallSiteComparator::FunctionSet Methods(ArrayTypes.size(), nullptr);
  unsigned Count = 0;
  for (auto &Name : List) {
    auto *AF = F.getParent()->getFunction(Name);
    if (!AF)
      continue;

    auto *ClassType = getOPStructTypeOfMethod(AF, DTInfo);
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
    report_fatal_error(
        "There should be none or 1 method to combine for each array type.");

  return Methods;
}
} // namespace soatoaosOP
using namespace soatoaosOP;
char SOAToAOSOPStructMethodsCheckDebug::PassID;

// Provide a definition for the static class member used to identify passes.
AnalysisKey SOAToAOSOPStructMethodsCheckDebug::Key;

SOAToAOSOPStructMethodsCheckDebug::Ignore::Ignore(
    SOAToAOSOPStructMethodsCheckDebugResult *Ptr)
    : Ptr(Ptr) {}
SOAToAOSOPStructMethodsCheckDebug::Ignore::Ignore(Ignore &&Other)
    : Ptr(std::move(Other.Ptr)) {}
const SOAToAOSOPStructMethodsCheckDebugResult *
SOAToAOSOPStructMethodsCheckDebug::Ignore::get() const {
  return Ptr.get();
}
SOAToAOSOPStructMethodsCheckDebug::Ignore::~Ignore() {}

SOAToAOSOPStructMethodsCheckDebug::Ignore
SOAToAOSOPStructMethodsCheckDebug::run(Module &M, ModuleAnalysisManager &MAM) {
  auto &FAM = MAM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto *DTInfo = &MAM.getResult<DTransSafetyAnalyzer>(M);
  auto *Res = &MAM.getResult<SOAToAOSOPApproximationDebug>(M);
  const DepMap *DM = Res->get();
  if (!DM)
    report_fatal_error("Missing SOAToAOSOPApproximationDebug results before "
                       "SOAToAOSOPStructMethodsCheckDebug.");

  std::unique_ptr<SOAToAOSOPStructMethodsCheckDebugResult> Result(
      new SOAToAOSOPStructMethodsCheckDebugResult());

  for (auto &F : M) {
    if (F.isDeclaration())
      continue;

    SummaryForIdiom S = getParametersForSOAToAOSMethodsCheckDebug(F, DTInfo);

    LLVM_DEBUG(dbgs() << "; Checking structure's method " << F.getName()
                      << "\n");

    auto P = getArrayTypesForSOAToAOSOPStructMethodsCheckDebug(F, DTInfo);

    if (isFunctionIgnoredForSOAToAOSOP(F))
      continue;

    for (auto &CmpKind : DTransSOAToAOSOPComparison)
      if (CmpKind == "append")
        Result->Appends = getSOAToAOSOPArrayMethods(DTransSOAToAOSOPAppends,
                                                    P.first, F, DTInfo);
      else if (CmpKind == "cctor")
        Result->CCtors = getSOAToAOSOPArrayMethods(DTransSOAToAOSOPCCtors,
                                                   P.first, F, DTInfo);
      else if (CmpKind == "ctor")
        Result->Ctors = getSOAToAOSOPArrayMethods(DTransSOAToAOSOPCtors,
                                                  P.first, F, DTInfo);
      else if (CmpKind == "dtor")
        Result->Dtors = getSOAToAOSOPArrayMethods(DTransSOAToAOSOPDtors,
                                                  P.first, F, DTInfo);
      else
        report_fatal_error(
            "Incorrect value in dtrans-soatoaosop-method-call-site-comparison."
            "append/cctor/ctor/dtor are allowed.");

    if (DTransSOAToAOSOPBasePtrOff == -1U)
      report_fatal_error("dtrans-soatoaosop-base-ptr-off was not provided.");

    auto &TLI =
        FAM.getResult<TargetLibraryAnalysis>(*(const_cast<Function *>(&F)));
    StructureMethodAnalysis Checks(F.getParent()->getDataLayout(), *DTInfo, TLI,
                                   *DM, S, P.first,
                                   *Result /*TransformationData*/);

    bool SeenArrays = false;
    bool CheckedAll = Checks.checkStructMethod(SeenArrays);
    (void)CheckedAll;
    LLVM_DEBUG(
        dbgs()
        << "; IR: "
        << (CheckedAll && SeenArrays
                ? " has only expected side-effects\n"
                : (SeenArrays
                       ? " needs analysis of instructions)\n"
                       : " no need to analyze: no accesses to arrays\n")));

    // If there are no accesses to arrays, then there should be no instructions
    // to update.
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
      CallSiteComparator Cmp(F.getParent()->getDataLayout(), *DTInfo, TLI, *DM,
                             S, P.first, P.second,
                             *Result, /* CallSiteComparator */
                             *Result, /*TransformationData*/
                             DTransSOAToAOSOPBasePtrOff);

      bool Comparison = Cmp.canCallSitesBeMerged();
      (void)Comparison;
      LLVM_DEBUG(
          dbgs() << "; Array call sites analysis result: "
                 << (Comparison
                         ? "required call sites can be merged"
                         : "problem with call sites required to be merged")
                 << "\n");
    }
  }
  return Ignore(Result.release());
}

namespace {
class SOAStructMethodReplacement : public DTransOPOptBase {
public:
  SOAStructMethodReplacement(
      DTransSafetyInfo &DTInfo, LLVMContext &Context,
      std::function<const TargetLibraryInfo &(const Function &)> GetTLI,
      const SummaryForIdiom &S,
      const SmallVectorImpl<DTransStructType *> &Arrays,
      const SmallVectorImpl<unsigned> &Offsets,
      const SOAToAOSOPStructMethodsCheckDebugResult &InstsToTransform,
      StringRef DepTypePrefix)
      : DTransOPOptBase(Context, &DTInfo,
                        DTInfo.getPtrTypeAnalyzer().sawOpaquePointer(),
                        DepTypePrefix),
        InstsToTransform(InstsToTransform), S(S), Arrays(Arrays),
        Offsets(Offsets) {}

  bool prepareTypes(Module &M) override {
    LLVMContext &Context = M.getContext();

    NewLLVMStruct = StructType::create(
        Context, (Twine(DepTypePrefix) + S.StrType->getName()).str());
    NewDTStruct = TM.getOrCreateStructType(NewLLVMStruct);
    TypeRemapper.addTypeMapping(S.StrType->getLLVMType(), NewLLVMStruct,
                                S.StrType, NewDTStruct);

    NewLLVMArray = StructType::create(
        Context, (Twine(DepTypePrefix) + "AR_" + Arrays[0]->getName()).str());
    NewDTArray = TM.getOrCreateStructType(NewLLVMArray);
    NewLLVMElement = StructType::create(
        Context, (Twine(DepTypePrefix) + "EL_" + S.StrType->getName()).str());
    NewDTElement = TM.getOrCreateStructType(NewLLVMElement);

    for (auto *Fld : Arrays)
      TypeRemapper.addTypeMapping(Fld->getLLVMType(), NewLLVMArray, Fld,
                                  NewDTArray);
    return true;
  }

  void populateTypes(Module &M) override {
    LLVMContext &Context = M.getContext();
    {
      SmallVector<Type *, 6> LLVMElements;
      SmallVector<DTransType *, 6> DTElements;
      for (auto *Fld : Arrays) {
        auto *FldTy = getOPSOAElementType(Fld, DTransSOAToAOSOPBasePtrOff);
        DTElements.push_back(FldTy);
        LLVMElements.push_back(FldTy->getLLVMType());
      }
      NewLLVMElement->setBody(LLVMElements);
      NewDTElement->setBody(DTElements);
    }

    {
      SmallVector<DTransType *, 6> ArrayDTElems;
      size_t NumFields = Arrays[0]->getNumFields();
      for (size_t Idx = 0; Idx < NumFields; ++Idx)
        ArrayDTElems.push_back(Arrays[0]->getFieldType(Idx));
      ArrayDTElems[DTransSOAToAOSOPBasePtrOff] =
          TM.getOrCreatePointerType(NewDTElement);
      SmallVector<Type *, 6> ArrayLLVMElems;
      for (auto *Fld : ArrayDTElems)
        ArrayLLVMElems.push_back(Fld->getLLVMType());
      NewDTArray->setBody(ArrayDTElems);
      NewLLVMArray->setBody(ArrayLLVMElems);
    }

    {
      SmallVector<DTransType *, 6> StructDTElems;
      size_t NumFields = S.StrType->getNumFields();
      for (size_t Idx = 0; Idx < NumFields; ++Idx)
        StructDTElems.push_back(S.StrType->getFieldType(Idx));

      auto *DTFPType = TM.getOrCreateAtomicType(Type::getFloatTy(Context));
      auto *PFloat = TM.getOrCreatePointerType(DTFPType);
      // Do not change layout of structure, all but one
      // pointer to transformed array is replaced with pointer to 'float'.
      for (auto O : Offsets)
        StructDTElems[O] = PFloat;

      AOSOffset = *std::min_element(Offsets.begin(), Offsets.end());

      StructDTElems[AOSOffset] = TM.getOrCreatePointerType(NewDTArray);

      SmallVector<Type *, 6> StructLLVMElems;
      for (auto *Fld : StructDTElems)
        StructLLVMElems.push_back(Fld->getLLVMType());

      NewDTStruct->setBody(StructDTElems);
      NewLLVMStruct->setBody(StructLLVMElems);
    }
  }

  // For “Append” member function, new parameters are added during the
  // transformation. Earlier, old function types of “Append” member function
  // were mapped to new function type in “prepareTypes” so that OptBase class
  // does the conversion from old types to the new types. But, that doesn’t work
  // with opaque pointers since all “Append” member functions have same function
  // type. New “Append” functions are created here and then mapped to the
  // old “Append” member functions in VMap so that OptBase class clones new
  // “Append” member functions and replaces old “Append” member functions with
  // new “Append” member functions in entire Module.
  void prepareModule(Module &M) override {
    auto &MD = DTInfo->getTypeMetadataReader();
    auto &TM = DTInfo->getTypeManager();
    DTransFunctionType *NewDTFunctionTy = nullptr;
    for (auto *Method : InstsToTransform.Appends) {
      if (!NewDTFunctionTy) {
        SmallVector<DTransPointerType *, 3> Elems;
        for (auto *ArrType : Arrays)
          Elems.push_back(
              getOPSOAElementType(ArrType, DTransSOAToAOSOPBasePtrOff));

        // updateAppends in StructMethodTransformation computes necessary
        // offset during CallSite creation.
        unsigned IgnoreElemOffset = 0;
        auto *StType =
            getOPStructTypeOfMethod(const_cast<Function *>(Method), DTInfo);
        assert(StType && "Expected class type for struct method");
        NewDTFunctionTy = ArrayMethodTransformation::mapNewAppendType(
            *Method, getOPSOAElementType(StType, DTransSOAToAOSOPBasePtrOff),
            Elems, StType, &TypeRemapper, IgnoreElemOffset, MD, TM);
      }
      createAndMapNewAppendFunc(const_cast<llvm::Function *>(Method), M,
                                NewDTFunctionTy, VMap, OrigFuncToCloneFuncMap,
                                CloneFuncToOrigFuncMap,
                                AppendsFuncToDTransTyMap);
    }
  }

  void postprocessFunction(Function &OrigFunc, bool isCloned) override {
    if (isFunctionIgnoredForSOAToAOSOP(OrigFunc))
      return;

    // For typed pointers, all member functions will be cloned by
    // DTransOPOptBase. For opaque pointers, none of the member functions
    // will be cloned. The original implementation is designed to work
    // on cloned routines. First, it collects instructions to transform
    // in the pre-cloned function and then modifies the instructions in
    // cloned routine by getting corresponding instructions by using VMap.
    // Since the cloning is not happening with opaque pointers, clone
    // OrigFunc temporarily, modify the instructions in the cloned routine
    // and then move the modified instructions to the OrigFunc. Cloned
    // routine will be deleted at end of this routine.
    Function *Clone;
    ValueToValueMapTy NewVMap;
    // InstsToTransform contains all needed information.
    // New instructions after cloning are obtained using VMap.
    if (!isCloned) {
      Clone = CloneFunction(&OrigFunc, NewVMap);
      fixCallInfo(OrigFunc, DTInfo, NewVMap);
    }
    StructMethodTransformation SMT(*DTInfo, isCloned ? VMap : NewVMap,
                                   InstsToTransform /* CallSiteComparator */,
                                   InstsToTransform /*TransformationData*/,
                                   OrigFunc.getContext(), isCloned,
                                   CloneFuncToOrigFuncMap);

    SMT.updateReferences(S.StrType, NewDTArray, Arrays, AOSOffset,
                         DTransSOAToAOSOPBasePtrOff);

    if (!isCloned)
      replaceOrigFuncBodyWithClonedFuncBody(OrigFunc, *Clone);

    // Fix DTransFunctionType of new “Append” member function.
    auto *NewFunc = isCloned ? OrigFuncToCloneFuncMap[&OrigFunc] : &OrigFunc;
    auto *AppendFuncDTy = AppendsFuncToDTransTyMap[NewFunc];
    if (AppendFuncDTy)
      DTransTypeMetadataBuilder::setDTransFuncMetadata(NewFunc, AppendFuncDTy);
  }

private:
  const SOAToAOSOPStructMethodsCheckDebugResult &InstsToTransform;
  const SummaryForIdiom &S;
  const SmallVectorImpl<DTransStructType *> &Arrays;
  const SmallVectorImpl<unsigned> &Offsets;

  // Structure containing one element per each transformed array.
  StructType *NewLLVMElement = nullptr;
  DTransStructType *NewDTElement = nullptr;
  // New array of structures, structure is NewElement.
  StructType *NewLLVMArray = nullptr;
  DTransStructType *NewDTArray = nullptr;
  // Structure containing pointer to NewArray.
  StructType *NewLLVMStruct = nullptr;
  DTransStructType *NewDTStruct = nullptr;
  // Offset of array-of-structure in NewStruct.
  unsigned AOSOffset = -1U;

  // Mapping between new “Append” member functions and DTransFunctionTypes.
  SmallDenseMap<Function *, DTransFunctionType *> AppendsFuncToDTransTyMap;
};
} // namespace

PreservedAnalyses
SOAToAOSStructMethodsTransformDebug::run(Module &M, ModuleAnalysisManager &AM) {
  // It should be lit-test with single defined function at this point.
  Function *MethodToTest = nullptr;
  for (auto &F : M) {
    if (isFunctionIgnoredForSOAToAOSOP(F))
      continue;
    if (!F.isDeclaration()) {
      if (MethodToTest)
        report_fatal_error("Single function definition per compilation unit is "
                           "allowed in SOAToAOSStructMethodsTransformDebug.");
      MethodToTest = &F;
    }
  }

  if (!MethodToTest)
    report_fatal_error(
        "Exactly one function definition per compilation unit is "
        "required in SOAToAOSStructMethodsTransformDebug.");

  if (DTransSOAToAOSOPBasePtrOff == -1U)
    report_fatal_error("dtrans-soatoaosop-base-ptr-off was not provided.");

  auto &DTInfo = AM.getResult<DTransSafetyAnalyzer>(M);
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetTLI = [&FAM](const Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(*(const_cast<Function *>(&F)));
  };
  SummaryForIdiom S =
      getParametersForSOAToAOSMethodsCheckDebug(*MethodToTest, &DTInfo);

  auto P =
      getArrayTypesForSOAToAOSOPStructMethodsCheckDebug(*MethodToTest, &DTInfo);
  // SOAToAOSOPStructMethodsCheckDebug uses SOAToAOSOPApproximationDebug
  // internally.
  AM.getResult<SOAToAOSOPApproximationDebug>(M);
  auto &InstsToTransformPtr =
      AM.getResult<SOAToAOSOPStructMethodsCheckDebug>(M);

  SOAStructMethodReplacement Transformer(DTInfo, M.getContext(), GetTLI, S,
                                         P.first, P.second,
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
