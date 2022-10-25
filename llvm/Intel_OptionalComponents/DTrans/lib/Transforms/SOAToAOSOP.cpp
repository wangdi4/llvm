//===---------------- SOAToAOSOP.cpp - SOAToAOSOPPass ---------------------===//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the DTrans Structure of Arrays to Array of Structures
// data layout optimization pass for IR with either opaque pointers or
// non-opaque pointers.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/SOAToAOSOP.h"

#include "Intel_DTrans/Analysis/DTransAnnotator.h"
#include "Intel_DTrans/Analysis/DTransSafetyAnalyzer.h"
#include "Intel_DTrans/Analysis/DTransTypeMetadataBuilder.h"
#include "Intel_DTrans/Analysis/PtrTypeAnalyzer.h"
#include "Intel_DTrans/DTransCommon.h"
#include "Intel_DTrans/Transforms/DTransOPOptBase.h"

#include "SOAToAOSOPArrays.h"
#include "SOAToAOSOPClassInfo.h"
#include "SOAToAOSOPEffects.h"
#include "SOAToAOSOPInternal.h"
#include "SOAToAOSOPStruct.h"

#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/Module.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Utils/FunctionComparator.h"

#define DEBUG_TYPE "dtrans-soatoaosop"

namespace {
using namespace llvm;
using namespace dtransOP;
using namespace soatoaosOP;
using dtrans::DTransAnnotator;

// This option makes populateCFGInformation ignore
// number of uses and number of BasicBlocks in structs' and arrays' methods.
//
// It helps to debug issues in transformed code with debug prints, for
// example.
static cl::opt<bool> DTransSOAToAOSOPSizeHeuristic(
    "dtrans-soatoaosop-size-heuristic", cl::init(true), cl::Hidden,
    cl::desc("Respect size heuristic in DTrans SOAToAOS"));

class SOAToAOSOPTransformImpl : public DTransOPOptBase {
public:
  SOAToAOSOPTransformImpl(
      LLVMContext &Context, DTransSafetyInfo &DTInfo,
      StringRef DepTypePrefix, const DataLayout &DL,
      std::function<const TargetLibraryInfo &(const Function &)> GetTLI,
      std::function<DominatorTree &(Function &)> GetDT)
      : DTransOPOptBase(Context, &DTInfo, DepTypePrefix),
        DL(DL), GetTLI(GetTLI), GetDT(GetDT) {}

  ~SOAToAOSOPTransformImpl() {
    for (auto *Cand : Candidates) {
      delete Cand;
    }
    Candidates.clear();
  }

private:
  SOAToAOSOPTransformImpl(const SOAToAOSOPTransformImpl &) = delete;
  SOAToAOSOPTransformImpl &operator=(const SOAToAOSOPTransformImpl &) = delete;

  bool prepareTypes(Module &M) override;
  void populateTypes(Module &M) override;
  void prepareModule(Module &M) override;
  void postprocessFunction(Function &OrigFunc, bool isCloned) override;

  // Relatively heavy weight checks to classify methods, see MethodKind.
  // Has internal memory handling for DepMap:
  // copying is forbidden.
  //
  class CandidateSideEffectsInfo : public SOAToAOSOPCFGInfo, DepMap {
  public:
    // Computes dependencies using DepMap.
    bool populateSideEffects(SOAToAOSOPTransformImpl &Impl, Module &M);

    // Checks that all field arrays' methods are called from structure's
    // methods; Necessary check for legality analysis.
    bool checkCFG(DTransSafetyInfo &DTInfo) const {
      for (auto *Fld : fields()) {
        auto *FI = cast_or_null<dtrans::StructInfo>(DTInfo.getTypeInfo(Fld));

        if (!FI)
          return FALSE("array type has no DTrans info.");

        // May restrict analysis to DStruct's methods.
        auto &CG = FI->getCallSubGraph();
        if (CG.isTop() || CG.isBottom() ||
            CG.getEnclosingType() != DStruct->getLLVMType())
          return FALSE("array type has unsupported CFG.");
      }
      return true;
    }

    // Verify that all member functions of vector field classes are
    // expected pattern.
    bool checkClassInfoAnalysis(SOAToAOSOPTransformImpl &Impl, Module &M) {

      std::unique_ptr<SOACandidateInfo> CandD(
          new SOACandidateInfo(Impl.DTInfo->getTypeMetadataReader()));
      if (!CandD->isCandidateType(DStruct) || !CandD->collectMemberFunctions(M))
        return false;

      ClassCandI = CandD.release();
      for (auto Loc : ArrayFieldOffsets) {
        std::unique_ptr<ClassInfo> ClassD(
            new ClassInfo(Impl.DL, *Impl.DTInfo, Impl.GetTLI, Impl.GetDT,
                          ClassCandI, Loc, false));
        if (!ClassD->analyzeClassFunctions())
          return false;

        ArraysClassInfo.push_back(ClassD.release());
      }
      return true;
    }

    // Compare functions F1 and F2 using class analysis info.
    bool classInfoAnalysisCompare(Function *F1, Function *F2,
                                  SOAToAOSOPTransformImpl &Impl) {

      auto GetClassInfo = [this](Function *F) -> ClassInfo * {
        for (auto *CInfo : ArraysClassInfo)
          if (CInfo->isCandidateMemberFunction(F))
            return CInfo;
        return nullptr;
      };

      // Returns true if 2nd argument of given AppendElem function
      // F is address of element variant.
      auto IsAddressOfElemVariantAppendElem = [&](ClassInfo *CInfo,
                                                  Function *F) {
        auto *DTFuncTy = dyn_cast_or_null<DTransFunctionType>(
            Impl.DTInfo->getTypeMetadataReader().getDTransTypeFromMD(F));
        assert(DTFuncTy && "Must have type if function is being transformed");

        if (CInfo->isElemDataAddrType(DTFuncTy->getArgType(1)))
          return true;
        return false;
      };

      auto *A1Ty = getOPStructTypeOfMethod(F1, Impl.DTInfo);
      auto *A2Ty = getOPStructTypeOfMethod(F2, Impl.DTInfo);
      if (!A1Ty || !A2Ty)
        return false;
      ClassInfo *C1Info = GetClassInfo(F1);
      ClassInfo *C2Info = GetClassInfo(F2);
      if (!C1Info || !C2Info)
        return false;
      FunctionKind FKind1 = C1Info->getFinalFuncKind(F1);
      FunctionKind FKind2 = C2Info->getFinalFuncKind(F2);
      if (FKind1 != FKind2)
        return false;

      switch (FKind1) {
      default:
        // No need to combine the other member functions.
        return false;

      case Constructor: {
        // Make sure flag value is same. All other fields will be
        // verified at callsites.
        StoreInst *FlagSI1 = C1Info->getFlagFieldStoreInstInCtor();
        StoreInst *FlagSI2 = C2Info->getFlagFieldStoreInstInCtor();
        if (!FlagSI1 || !FlagSI2)
          return false;
        if (FlagSI1->getValueOperand() == FlagSI2->getValueOperand())
          return true;
      } break;

      case CopyConstructor:
        // No variations are expected.
        return true;

      case AppendElem:
        // Make sure address of element type is passed in both cases.
        // Allow only if 2nd argument of both functions represent
        // "element address".
        if (IsAddressOfElemVariantAppendElem(C1Info, F1) &&
            IsAddressOfElemVariantAppendElem(C2Info, F2))
          return true;
        break;

      case Resize:
      case Destructor:
        // TODO: Even though these are semantically almost same, there are
        // some minor differences. Need to add more checks and handle
        // the  differences either in SOAToAOS or SOAToAOSPrepare passes.
        return true;
      }
      return false;
    }

  protected:
    CandidateSideEffectsInfo() {}
    ~CandidateSideEffectsInfo() {
      for (auto *CInfo : ArraysClassInfo)
        delete CInfo;
      if (ClassCandI)
        delete ClassCandI;
    }

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

    // These are used to keep class info of vector field classes.
    SOACandidateInfo *ClassCandI = nullptr;
    SmallVector<ClassInfo *, MaxNumFieldCandidates> ArraysClassInfo;

    // Mapping between member functions of struct / array classes and
    // DTransFunctionTypes.
    SmallDenseMap<Function *, DTransStructType *> FunctionOrigClassTypeMap;

  private:
    CandidateSideEffectsInfo(const CandidateSideEffectsInfo &) = delete;
    CandidateSideEffectsInfo &
    operator=(const CandidateSideEffectsInfo &) = delete;
  };

  // Check debug passes in SOAToAOSOPArrays.cpp and SOAToAOSOPStruct.cpp.
  class CandidateInfo : public CandidateSideEffectsInfo {
  public:
    void prepareTypes(SOAToAOSOPTransformImpl &Impl, Module &M) {
      LLVMContext &Context = M.getContext();

      auto &TM = Impl.DTInfo->getTypeManager();

      NewLLVMStruct = StructType::create(
          Context, (Twine(Impl.DepTypePrefix) + DStruct->getName()).str());
      NewDTStruct = TM.getOrCreateStructType(NewLLVMStruct);
      Impl.TypeRemapper.addTypeMapping(DStruct->getLLVMType(), NewLLVMStruct,
                                       DStruct, NewDTStruct);

      NewLLVMArray =
          StructType::create(Context, (Twine(Impl.DepTypePrefix) + "AR_" +
                                       (*fields_begin())->getName())
                                          .str());
      NewDTArray = TM.getOrCreateStructType(NewLLVMArray);

      NewLLVMElement = StructType::create(
          Context,
          (Twine(Impl.DepTypePrefix) + "EL_" + DStruct->getName()).str());
      NewDTElement = TM.getOrCreateStructType(NewLLVMElement);

      for (auto *Fld : fields())
        Impl.TypeRemapper.addTypeMapping(Fld->getLLVMType(), NewLLVMArray, Fld,
                                         NewDTArray);
    }

    void populateTypes(SOAToAOSOPTransformImpl &Impl, Module &M) {
      auto &TM = Impl.DTInfo->getTypeManager();
      {
        SmallVector<Type *, 6> LLVMElements;
        SmallVector<DTransType *, 6> DTElements;
        for (auto *FldTy : elements()) {
          DTElements.push_back(FldTy);
          LLVMElements.push_back(FldTy->getLLVMType());
        }
        NewLLVMElement->setBody(LLVMElements);
        NewDTElement->setBody(DTElements);

        // Create Metadata for new element type that is not mapped to any
        // existing types.
        NamedMDNode *DTMDTypes = TypeMetadataReader::getDTransTypesMetadata(M);
	assert(DTMDTypes && "Expected non-null DTMDTypes");
        DTMDTypes->addOperand(
            NewDTElement->createMetadataStructureDescriptor());
      }

      {
        SmallVector<DTransType *, 6> ArrayDTElems;
        SmallVector<Type *, 6> ArrayLLVMElems;
        auto *S = *fields_begin();
        for (auto &Fld : S->elements()) {
          auto *FldTy = Fld.getType();
          ArrayDTElems.push_back(FldTy);
          ArrayLLVMElems.push_back(FldTy->getLLVMType());
        }
        ArrayDTElems[BasePointerOffset] =
            TM.getOrCreatePointerType(NewDTElement);
        ArrayLLVMElems[BasePointerOffset] =
            ArrayDTElems[BasePointerOffset]->getLLVMType();
        NewDTArray->setBody(ArrayDTElems);
        NewLLVMArray->setBody(ArrayLLVMElems);
      }

      {
        SmallVector<DTransType *, 6> StructDTElems;
        SmallVector<Type *, 6> StructLLVMElems;

        for (auto &Fld : DStruct->elements())
          StructDTElems.push_back(Fld.getType());

        auto PlaceHolderDTType = TM.getOrCreateAtomicType(Type::getIntNTy(
            M.getContext(), M.getDataLayout().getPointerSizeInBits()));
        // Do not change layout of structure, all but one
        // pointer to transformed array is replaced with intptr_t.
        for (auto O : ArrayFieldOffsets)
          StructDTElems[O] = PlaceHolderDTType;

        AOSOffset = *std::min_element(ArrayFieldOffsets.begin(),
                                      ArrayFieldOffsets.end());

        StructDTElems[AOSOffset] = TM.getOrCreatePointerType(NewDTArray);

        for (auto *FldTy : StructDTElems)
          StructLLVMElems.push_back(FldTy->getLLVMType());

        NewDTStruct->setBody(StructDTElems);
        NewLLVMStruct->setBody(StructLLVMElems);
      }
    }

    void postprocessStructMethod(
        SOAToAOSOPTransformImpl &Impl, Function &OrigFunc,
        const StructureMethodAnalysis::TransformationData &TI, bool IsCloned) {

      SmallVector<DTransStructType *, MaxNumFieldCandidates> Fields(
          fields_begin(), fields_end());

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
      if (!IsCloned) {
        Clone = CloneFunction(&OrigFunc, NewVMap);
        fixCallInfo(OrigFunc, Impl.DTInfo, NewVMap);
      }

      StructMethodTransformation SMT(*Impl.DTInfo,
                                     IsCloned ? Impl.VMap : NewVMap, CSInfo, TI,
                                     OrigFunc.getParent()->getContext(),
                                     IsCloned, Impl.CloneFuncToOrigFuncMap);

      SMT.updateReferences(DStruct, NewDTArray, Fields, AOSOffset,
                           BasePointerOffset);
      if (!IsCloned)
        replaceOrigFuncBodyWithClonedFuncBody(OrigFunc, *Clone);
    }

    // For “Append” member function, new parameters are added during the
    // transformation. Earlier, old function types of “Append” member function
    // were mapped to new function types in “prepareTypes” so that OptBase class
    // does the conversion from old types to the new types. But, that doesn’t
    // work with opaque pointers since all “Append” member functions have same
    // function type. New “Append” functions are created here and then mapped
    // to the old “Append” member functions in VMap so that OptBase class
    // clones new “Append” member functions and replaces old “Append” member
    // functions with new “Append” member functions in entire Module.
    void prepareModule(SOAToAOSOPTransformImpl &Impl, Module &M) {
      auto &MD = Impl.DTInfo->getTypeMetadataReader();
      auto &TM = Impl.DTInfo->getTypeManager();
      DTransFunctionType *NewDTFunctionTy = nullptr;
      for (auto *Method : CSInfo.Appends) {
        if (!NewDTFunctionTy) {
          SmallVector<DTransPointerType *, MaxNumFieldCandidates> Elems;
          for (auto *Elem : elements())
            Elems.push_back(Elem);

          auto *ArrType = getOPStructTypeOfMethod(
              const_cast<Function *>(Method), Impl.DTInfo);
          assert(ArrType && "Expected class type for array method");
          NewDTFunctionTy = ArrayMethodTransformation::mapNewAppendType(
              *Method, getOPSOAElementType(ArrType, BasePointerOffset), Elems,
              ArrType, &Impl.TypeRemapper, AppendMethodElemParamOffset, MD, TM);
        }
        createAndMapNewAppendFunc(
            const_cast<llvm::Function *>(Method), M, NewDTFunctionTy, Impl.VMap,
            Impl.OrigFuncToCloneFuncMap, Impl.CloneFuncToOrigFuncMap,
            AppendsFuncToDTransTyMap);
      }
    }

    void postprocessArrayMethod(
        SOAToAOSOPTransformImpl &Impl, Function &OrigFunc,
        const ComputeArrayMethodClassification::TransformationData &TI,
        bool IsCloned) {

      auto &TM = Impl.DTInfo->getTypeManager();

      // Get original class type of OrigFunc.
      auto *ArrType = FunctionOrigClassTypeMap[&OrigFunc];
      assert(ArrType && "Expected class type for array method");
      auto *CurrElem = getOPSOAElementType(ArrType, BasePointerOffset);

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
          CurrElem != getOPSOAElementType(getOPSOAArrayType(DStruct, AOSOffset),
                                          BasePointerOffset)) {
        auto *NewFunc =
            IsCloned ? Impl.OrigFuncToCloneFuncMap[&OrigFunc] : &OrigFunc;
        for (auto &I : instructions(*NewFunc))
          Impl.DTInfo->deleteCallInfo(&I);
        NewFunc->deleteBody();
        return;
      }

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
      ValueToValueMapTy NewVMap;
      if (!IsCloned) {
        Clone = CloneFunction(&OrigFunc, NewVMap);
        fixCallInfo(OrigFunc, Impl.DTInfo, NewVMap);
      }

      const TargetLibraryInfo &TLI = Impl.GetTLI(IsCloned ? OrigFunc : *Clone);
      ArrayMethodTransformation AMT(Impl.DL, *Impl.DTInfo, TLI,
                                    IsCloned ? Impl.VMap : NewVMap, TI,
                                    OrigFunc.getParent()->getContext());

      DTransPointerType *NBT = TM.getOrCreatePointerType(NewDTElement);
      AMT.updateBasePointerInsts(CopyElemInsts, getNumArrays(), NBT,
                                 TM.getOrCreatePointerType(NBT));

      // Generate Store instructions early for each element except the CurrElem
      // only when member functions need to be combined (i.e CopyElemInsts is
      // true). The original StoreInst is used for the CurrElem.
      ArrayMethodTransformation::ClonedElemLoadStoreMapTy
          ClonedElemLoadStoreMap;
      if (CopyElemInsts) {
        unsigned Off = -1U;
        for (auto *Elem : elements()) {
          ++Off;
          if (Elem == CurrElem)
            continue;
          AMT.earlyCloneElemLoadStoreInst(Off, ClonedElemLoadStoreMap);
        }
      }

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
                  Off /* Offset in argument list of new element*/,
              ClonedElemLoadStoreMap, Off);
          AMT.gepRAUW(true /*Do copy*/, OrigToCopy,
                      Off /*Elem's offset in NewElement*/,
                      TM.getOrCreatePointerType(NewDTElement));
          UpdateUnique = false;
        }
      }

      // Original instructions from cloned function should be replaced as a
      // last step to keep OrigToCopy valid.
      AMT.gepRAUW(false /*Only update existing insts*/,
                  ArrayMethodTransformation::OrigToCopyTy(),
                  CurrOff /*CurElem's offset in NewElement*/,
                  TM.getOrCreatePointerType(NewDTElement));

      // If temporary cloned is used to fix IR, just move the IR to the
      // OrigFunc.
      if (!IsCloned)
        replaceOrigFuncBodyWithClonedFuncBody(OrigFunc, *Clone);

      // If the function was the constructor, copy constructor, destructor,
      // realloc or append method, mark it so that the Weak Align pass can
      // recognize it.
      if (CopyElemInsts) {
        auto *NewFunc =
            IsCloned ? Impl.OrigFuncToCloneFuncMap[&OrigFunc] : &OrigFunc;
        DTransAnnotator::createDTransSOAToAOSTypeAnnotation(*NewFunc,
                                                            NewLLVMElement, 1);
      }
      // Fix DTransFunctionType of new “Append” member function.
      auto *NewFunc =
          IsCloned ? Impl.OrigFuncToCloneFuncMap[&OrigFunc] : &OrigFunc;
      auto *AppendFuncDTy = AppendsFuncToDTransTyMap[NewFunc];
      if (AppendFuncDTy)
        DTransTypeMetadataBuilder::setDTransFuncMetadata(NewFunc,
                                                         AppendFuncDTy);
    }

    void postprocessFunction(SOAToAOSOPTransformImpl &Impl, Function &OrigFunc,
                             bool isCloned) {
      auto SIt = StructTransInfo.find(&OrigFunc);
      if (SIt != StructTransInfo.end()) {
        postprocessStructMethod(Impl, OrigFunc, *SIt->second.get(), isCloned);
        return;
      }

      auto AIt = ArrayTransInfo.find(&OrigFunc);
      if (AIt != ArrayTransInfo.end()) {
        postprocessArrayMethod(Impl, OrigFunc, *AIt->second.get(), isCloned);
        return;
      }
    }

    CandidateInfo() {}

  private:
    CandidateInfo &operator=(const CandidateInfo &) = delete;

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
    // Offset of the first element parameter for MK_Append method.
    // Element parameters are arranged in order of NewElement.
    unsigned AppendMethodElemParamOffset = -1U;

    // Mapping between new “Append” member functions and DTransFunctionTypes.
    SmallDenseMap<Function *, DTransFunctionType *> AppendsFuncToDTransTyMap;
  };

  constexpr static int MaxNumStructCandidates = 1;
  SmallVector<CandidateInfo *, MaxNumStructCandidates> Candidates;
  const DataLayout &DL;
  std::function<const TargetLibraryInfo &(const Function &)> GetTLI;
  std::function<DominatorTree &(Function &)> GetDT;
};

// Hook point. Top-level returns from populate* methods.
inline bool FALSE(const char *Msg) {
  LLVM_DEBUG(dbgs() << "; dtrans-soatoaosop " << Msg << "\n");
  return false;
}

bool SOAToAOSOPTransformImpl::CandidateSideEffectsInfo::populateSideEffects(
    SOAToAOSOPTransformImpl &Impl, Module &M) {

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
    DepCompute DC(*Impl.DTInfo, Impl.DL, TLI, F, DStruct, *this);

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
  for (const auto &Tuple :
       zip_first(methodsets(), fields(), elements(), ArrayFieldOffsets)) {
    ++Cnt;

    const Function *MethodsCalled = nullptr;
    for (auto *F : *std::get<0>(Tuple)) {
      ArraySummaryForIdiom S(std::get<1>(Tuple), std::get<2>(Tuple),
                             MemoryInterface, F, Impl.DTInfo);
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

      if (UnknownSeen && !DTransSOAToAOSOPComputeAllDep)
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
      // Set original class type for array member functions.
      FunctionOrigClassTypeMap[F] = getOPStructTypeOfMethod(F, Impl.DTInfo);
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
      return FALSE("method  called from append-like method is not realloc.");
  }

  if (UnknownSeen) {
    assert(DTransSOAToAOSOPComputeAllDep &&
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
    for (const auto &Pair : zip(Pivot, Running)) {
      auto *F = std::get<0>(Pair);
      auto *O = std::get<1>(Pair);
      FunctionComparator CmpFunc(F, O, &GNS);
      if (CmpFunc.compare() == 0) {
        LLVM_DEBUG(dbgs() << "; Comparison of " << F->getName() << " and "
                          << O->getName() << " showed bit-to-bit equality.\n");
        // GNS keeps pointers to modifiable values in a map.
        GNS.setEqual(const_cast<Function *>(F), const_cast<Function *>(O));
      } else if (classInfoAnalysisCompare(const_cast<Function *>(F),
                                          const_cast<Function *>(O), Impl)) {
        LLVM_DEBUG(dbgs() << "; Comparison of " << F->getName() << " and "
                          << O->getName()
                          << " by ClassInfo showed semantically equal.\n");
      } else {
        LLVM_DEBUG(dbgs() << "; Comparison of " << F->getName() << " and "
                          << O->getName() << " showed some differences, "
                          << "this situation cannot be handled in SOAToAOS.\n");
        return FALSE("bit-to-bit equality failed.");
      }
    }
  }

  SmallVector<DTransStructType *, MaxNumFieldCandidates> Arrays;
  for (auto *ArrTy : fields())
    Arrays.push_back(ArrTy);

  for (auto *F : StructMethods) {
    SummaryForIdiom S(DStruct, MemoryInterface, F, Impl.DTInfo);

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
                     : (SeenArrays
                            ? " needs analysis of instructions)\n"
                            : " no need to analyze: no accesses to arrays\n"));
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

    // Set original class type for struct member functions.
    FunctionOrigClassTypeMap[F] = getOPStructTypeOfMethod(F, Impl.DTInfo);
    // Pass ownership to StructTransInfo.
    StructTransInfo[F] = decltype(Data)(Data.release());
  }

  return true;
}

bool SOAToAOSOPTransformImpl::prepareTypes(Module &M) {

  for (dtrans::TypeInfo *TI : DTInfo->type_info_entries()) {

    auto *StInfo = dyn_cast<dtrans::StructInfo>(TI);
    if (!StInfo || cast<StructType>(StInfo->getLLVMType())->isLiteral())
      continue;

    std::unique_ptr<CandidateInfo> Info(new CandidateInfo());

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

    if (SafetyViolation) {
      LLVM_DEBUG({
        dbgs() << "  ; Rejecting ";
        StInfo->getLLVMType()->print(dbgs(), true, true);
        dbgs() << " because safety checks were violated.\n";
      });
      continue;
    }

    if (!Info->populateLayoutInformation(StInfo->getDTransType())) {
      LLVM_DEBUG({
        dbgs() << "  ; Rejecting ";
        TI->getLLVMType()->print(dbgs(), true, true);
        dbgs() << " because it does not look like a candidate structurally.\n";
      });
      continue;
    }
    TypeMetadataReader &MDReader = DTInfo->getTypeMetadataReader();
    if (!Info->populateCFGInformation(M, MDReader,
                                      DTransSOAToAOSOPSizeHeuristic, true)) {
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

    if (!Info->checkClassInfoAnalysis(*this, M)) {
      LLVM_DEBUG({
        dbgs() << "  ; Rejecting ";
        TI->getLLVMType()->print(dbgs(), true, true);
        dbgs() << " because ClassInfo analysis failed.\n";
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

    if (Candidates.size() >= MaxNumStructCandidates) {
      return FALSE("too many candidates found.");
    }

    LLVM_DEBUG({
      dbgs() << "  ; SOA-to-AOS possible for ";
      StInfo->getLLVMType()->print(dbgs(), true, true);
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

void SOAToAOSOPTransformImpl::populateTypes(Module &M) {
  for_each(Candidates,
           [this, &M](CandidateInfo *C) { C->populateTypes(*this, M); });
}

void SOAToAOSOPTransformImpl::prepareModule(Module &M) {
  for_each(Candidates,
           [this, &M](CandidateInfo *C) { C->prepareModule(*this, M); });
}

void SOAToAOSOPTransformImpl::postprocessFunction(Function &OrigFunc,
                                                  bool isCloned) {
  for_each(Candidates, [this, &OrigFunc, isCloned](CandidateInfo *C) {
    C->postprocessFunction(*this, OrigFunc, isCloned);
  });
}

} // namespace

namespace llvm {
namespace dtransOP {

bool SOAToAOSOPPass::runImpl(
    Module &M, DTransSafetyInfo &DTInfo, WholeProgramInfo &WPInfo,
    std::function<const TargetLibraryInfo &(const Function &)> GetTLI,
    std::function<DominatorTree &(Function &)> GetDT) {
  auto TTIAVX2 = TargetTransformInfo::AdvancedOptLevel::AO_TargetHasIntelAVX2;
  if (!WPInfo.isWholeProgramSafe() || !WPInfo.isAdvancedOptEnabled(TTIAVX2))
    return false;

  if (!DTInfo.useDTransSafetyAnalysis()) {
    LLVM_DEBUG(dbgs() << "  DTransSafetyAnalyzer results not available\n");
    return false;
  }

  // Perform the actual transformation.
  SOAToAOSOPTransformImpl Transformer(M.getContext(), DTInfo, "__SOADT_",
                                      M.getDataLayout(), GetTLI, GetDT);
  return Transformer.run(M);
}

PreservedAnalyses SOAToAOSOPPass::run(Module &M, ModuleAnalysisManager &AM) {
  auto &WP = AM.getResult<WholeProgramAnalysis>(M);
  if (!WP.isWholeProgramSafe())
    return PreservedAnalyses::all();

  auto &DTransInfo = AM.getResult<DTransSafetyAnalyzer>(M);
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetTLI = [&FAM](const Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(*(const_cast<Function *>(&F)));
  };
  auto GetDT = [&FAM](Function &F) -> DominatorTree & {
    return FAM.getResult<DominatorTreeAnalysis>(F);
  };

  bool Changed = runImpl(M, DTransInfo, WP, GetTLI, GetDT);
  if (!Changed)
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}

} // namespace dtransOP
} // end namespace llvm

namespace {
class DTransSOAToAOSOPWrapper : public ModulePass {
private:
  dtransOP::SOAToAOSOPPass Impl;

public:
  static char ID;

  DTransSOAToAOSOPWrapper() : ModulePass(ID) {
    initializeDTransSOAToAOSOPWrapperPass(*PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;

    auto &WP = getAnalysis<WholeProgramWrapperPass>().getResult();
    if (!WP.isWholeProgramSafe())
      return false;

    auto &DTAnalysisWrapper = getAnalysis<DTransSafetyAnalyzerWrapper>();
    DTransSafetyInfo &DTInfo = DTAnalysisWrapper.getDTransSafetyInfo(M);
    auto GetTLI = [this](const Function &F) -> TargetLibraryInfo & {
      return this->getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
    };
    auto GetDT = [this](Function &F) -> DominatorTree & {
      return this->getAnalysis<DominatorTreeWrapperPass>(F).getDomTree();
    };

    bool Changed = Impl.runImpl(M, DTInfo, WP, GetTLI, GetDT);

    // TODO: Need to set setInvalidated() when Changed is true.
    return Changed;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<DTransSafetyAnalyzerWrapper>();
    AU.addRequired<TargetLibraryInfoWrapperPass>();
    AU.addRequired<WholeProgramWrapperPass>();
    AU.addPreserved<WholeProgramWrapperPass>();
  }
};

} // namespace

char DTransSOAToAOSOPWrapper::ID = 0;
INITIALIZE_PASS_BEGIN(DTransSOAToAOSOPWrapper, "dtrans-soatoaosop",
                      "DTransOP struct of arrays to array of structs", false,
                      false)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(DTransSafetyAnalyzerWrapper)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_END(DTransSOAToAOSOPWrapper, "dtrans-soatoaosop",
                    "DTransOP struct of arrays to array of structs", false,
                    false)

ModulePass *llvm::createDTransSOAToAOSOPWrapperPass() {
  return new DTransSOAToAOSOPWrapper();
}
