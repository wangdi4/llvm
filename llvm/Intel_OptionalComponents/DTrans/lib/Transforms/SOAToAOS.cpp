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

#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Utils/FunctionComparator.h"

#define DEBUG_TYPE "dtrans-soatoaos"
// DepCompute
#define DTRANS_SOADEP "dtrans-soatoaos-deps"

#include "SOAToAOSEffects.h"
#include "SOAToAOSArrays.h"

namespace {
using namespace llvm;
using namespace dtrans;
using namespace soatoaos;

// Global guard to enable/disable transformation.
static cl::opt<bool>
    DTransSOAToAOSGlobalGuard("enable-dtrans-soatoaos", cl::init(false),
                              cl::Hidden, cl::desc("Enable DTrans SOAToAOS"));

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// Compare first and second fields (1-based numbering) of given type
// for congruence. See FIXME in SOAToAOSTransformImpl.
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

  // Prepare layout information for candidates.
  //
  // %class.FieldValueMap = type {
  //    %class.vector*,
  //    %class.vector.0*,
  //    %class.refvector*,
  //    MemoryManager*
  // }
  // %class.vector = type {
  //    i8, i32, i32,
  //    %class.elem**,
  //    MemoryManager*
  // }
  // %class.vector.0 = type {
  //    i8, i32, i32,
  //    %class.elem0**,
  //    MemoryManager*
  // }
  class CandidateLayoutInfo {
  public:
    // Returns false if idiom is not recognized.
    // Single out-of-line method with lambdas.
    bool populateLayoutInformation(Type *Ty);

  protected:
    constexpr static int MaxNumFieldTotalCandidates = 3;
    constexpr static int MaxNumFieldCandidates = 2;
    // There should be at least 'capacity' and 'size' fields.
    // Also permit auxiliary field.
    constexpr static int MaxNumIntFields = 3;

    using OffsetsTy = SmallVector<unsigned, MaxNumFieldCandidates>;

    // Helper class for fields* methods.
    template <typename IterTy>
    class ArrayIter
        : public iterator_adaptor_base<
              ArrayIter<IterTy>, IterTy,
              typename std::iterator_traits<IterTy>::iterator_category,
              StructType *,
              typename std::iterator_traits<IterTy>::difference_type,
              StructType **, StructType *> {
      using BaseTy = iterator_adaptor_base<
          ArrayIter, IterTy,
          typename std::iterator_traits<IterTy>::iterator_category,
          StructType *, typename std::iterator_traits<IterTy>::difference_type,
          StructType **, StructType *>;

    public:
      const CandidateLayoutInfo *Info;
      ArrayIter(const CandidateLayoutInfo *Info, IterTy It)
          : BaseTy(It), Info(Info) {}

      typename BaseTy::reference operator*() const {
        return Info->getSOAArrayType(*this->wrapped());
      }
    };

    // Helper class for elements* methods.
    template <typename IterTy>
    class ElementIter
        : public iterator_adaptor_base<
              ElementIter<IterTy>, IterTy,
              typename std::iterator_traits<IterTy>::iterator_category, Type *,
              typename std::iterator_traits<IterTy>::difference_type, Type **,
              Type *> {
      using BaseTy = iterator_adaptor_base<
          ElementIter, OffsetsTy::const_iterator,
          typename std::iterator_traits<IterTy>::iterator_category, Type *,
          typename std::iterator_traits<IterTy>::difference_type, Type **,
          Type *>;

    public:
      const CandidateLayoutInfo *Info;
      ElementIter(const CandidateLayoutInfo *Info, IterTy It)
          : BaseTy(It), Info(Info) {}

      typename BaseTy::reference operator*() const {
        return Info->getSOAElementType(*this->wrapped());
      }
    };

    // Accessing structures representing arrays.
    using const_iterator = ArrayIter<OffsetsTy::const_iterator>;

    const_iterator fields_begin() const {
      return const_iterator(this, ArrayFieldOffsets.begin());
    }
    const_iterator fields_end() const {
      return const_iterator(this, ArrayFieldOffsets.end());
    }
    iterator_range<const_iterator> fields() const {
      return make_range(fields_begin(), fields_end());
    }

    // Accessing types of elements stored in arrays.
    using const_elem_iterator = ElementIter<OffsetsTy::const_iterator>;

    const_elem_iterator elements_begin() const {
      return const_elem_iterator(this, ArrayFieldOffsets.begin());
    }
    const_elem_iterator elements_end() const {
      return const_elem_iterator(this, ArrayFieldOffsets.end());
    }
    iterator_range<const_elem_iterator> elements() const {
      return make_range(elements_begin(), elements_end());
    }

    unsigned getNumArrays() const { return ArrayFieldOffsets.size(); }

    StructType *Struct = nullptr;
    // Memory management pointer.
    StructType *MemoryInterface = nullptr;
    // Offsets in Struct's elements() to represent pointers to candidate
    // _arrays_. _Arrays_ are represented as some classes.
    OffsetsTy ArrayFieldOffsets;
    // Offset inside _arrays_' elements(), which represent base pointers to
    // allocated memory.
    unsigned BasePointerOffset = -1U;

  private:
    // Off is some element of ArrayFieldOffsets.
    StructType *getSOAArrayType(unsigned Off) const {
      return cast<StructType>(
          Struct->getElementType(Off)->getPointerElementType());
    }
    // Off is some element of ArrayFieldOffsets.
    Type *getSOAElementType(unsigned Off) const {
      return getSOAArrayType(Off)
          ->getElementType(BasePointerOffset)
          ->getPointerElementType();
    }
  };

  // Prepare CFG information and methods to analyze.
  // Cheap checks are performed.
  class CandidateCFGInfo : public CandidateLayoutInfo {
  public:
    // Returns false if idiom is not recognized.
    // Single out-of-line method with lambdas.
    //
    // Checks that
    // 1. all field arrays' methods are called from structure's methods;
    // 2. arrays' methods are small and have at most MaxNumFieldMethodUses call
    //    sites;
    // 3. arguments of arrays' methods are 'this' pointers (not captured),
    //    elements of array (not captured if passed by reference),
    //    integers and MemoryInterface.
    bool populateCFGInformation(SOAToAOSTransformImpl &Impl, Module &M);

  protected:
    constexpr static int NumRequiredMethods = 4;
    // Specific methods (3) and integer fields accessors (3).
    constexpr static int NumSupportedMethods = 3 + 3;
    constexpr static int MaxNumMethods =
        NumRequiredMethods + NumSupportedMethods;
    constexpr static int MaxMethodBBCount = 10;

    // Field methods are used only Struct's methods,
    // it should not be too big.
    const static int MaxNumFieldMethodUses = 2;

    using MethodSetTy = SmallVector<Function *, MaxNumMethods>;
    using ArrayMethodSetTy = SmallVector<MethodSetTy, MaxNumFieldCandidates>;

    // Helper class for methodsets* methods.
    template <typename IterTy, typename MethSetTy>
    class MethodsIter
        : public iterator_adaptor_base<
              MethodsIter<IterTy, MethSetTy>, IterTy,
              typename std::iterator_traits<IterTy>::iterator_category,
              MethSetTy *,
              typename std::iterator_traits<IterTy>::difference_type,
              MethSetTy **, MethSetTy *> {
      using BaseTy = iterator_adaptor_base<
          MethodsIter, IterTy,
          typename std::iterator_traits<IterTy>::iterator_category,
          MethSetTy *, typename std::iterator_traits<IterTy>::difference_type,
          MethSetTy **, MethSetTy *>;

    public:
      MethodsIter(IterTy It) : BaseTy(It) {}

      typename BaseTy::reference operator*() const { return &*this->wrapped(); }
    };

    // Updating method sets of arrays.
    using iterator = MethodsIter<ArrayMethodSetTy::iterator, MethodSetTy>;

    iterator methodsets_begin() { return iterator(ArrayFieldsMethods.begin()); }
    iterator methodsets_end() { return iterator(ArrayFieldsMethods.end()); }
    iterator_range<iterator> methodsets() {
      return make_range(methodsets_begin(), methodsets_end());
    }

    using const_iterator =
        MethodsIter<ArrayMethodSetTy::const_iterator, const MethodSetTy>;
    const_iterator methodsets_begin() const {
      return const_iterator(ArrayFieldsMethods.begin());
    }
    const_iterator methodsets_end() const {
      return const_iterator(ArrayFieldsMethods.end());
    }
    iterator_range<const_iterator> methodsets() const {
      return make_range(methodsets_begin(), methodsets_end());
    }

    MethodSetTy StructMethods;
    // It is matched by ArrayFieldsMethods,
    // can be iterated in parallel using zip_first.
    ArrayMethodSetTy ArrayFieldsMethods;
  };

  // Relatively heavy weight checks to classify methods, see MethodKind.
  // Has internal memory handling for DepMap:
  // copying is forbidden.
  //
  class CandidateSideEffectsInfo : public CandidateCFGInfo, DepMap {
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

bool SOAToAOSTransformImpl::CandidateLayoutInfo::populateLayoutInformation(
    Type *Ty) {

  // All struct types should pass this check.
  auto ExtractStructTy = [](Type *Ty) -> StructType * {
    if (auto *St = dyn_cast_or_null<StructType>(Ty))
      if (!St->isLiteral() && St->isSized())
        return St;
    return nullptr;
  };

  // All pointers should pass this check.
  auto ExtractPointeeTy = [](Type *Ty) -> Type * {
    if (auto *PTy = dyn_cast_or_null<PointerType>(Ty)) {
      if (PTy->getAddressSpace())
        return nullptr;
      return PTy->getElementType();
    }
    return nullptr;
  };

  // Actually array of pointers to vararg functions.
  auto IsVTable = [&ExtractPointeeTy](Type *FTy) -> bool {
    if (auto *ElementTy = ExtractPointeeTy(ExtractPointeeTy(FTy)))
      if (ElementTy->isFunctionTy())
        return true;

    return false;
  };

  // Empty class or interface (has only vtable).
  auto HasNoData = [&IsVTable](StructType *STy) -> bool {
    if (STy->getNumElements() >= 2)
      return false;

    if (STy->getNumElements() == 1 && !IsVTable(STy->getElementType(0)))
      return false;

    return true;
  };

  auto IsPaddingFieldCandidate = [](Type *Ty) -> bool {
    if (!Ty->isArrayTy())
      return false;
    auto *El = Ty->getArrayElementType();
    if (!El->isIntegerTy() || El->getIntegerBitWidth() != 8)
      return false;
    return true;
  };

  auto StripTrivialDerivedClasses =
      [&IsPaddingFieldCandidate](StructType *STy) -> StructType * {
    auto *I = STy;
    for (; I && I->getNumElements() >= 1 && I->getNumElements() <= 2;
         I = dyn_cast<StructType>(I->getElementType(0))) {
      if (I->getNumElements() == 2)
        if (!IsPaddingFieldCandidate(I->getElementType(1)))
          return I;
    }
    return I;
  };

  // Looks like array wrapper.
  auto IsPtrToArrayCandidate =
      [&ExtractStructTy, &ExtractPointeeTy, &IsVTable, &HasNoData,
       &IsPaddingFieldCandidate](StructType *STy, bool &HasVT) -> bool {
    HasVT = false;
    unsigned NoDataPointerFields = 0;
    unsigned ElemDataPointerFields = 0;
    for (auto *ETy : STy->elements()) {
      if (ETy->isIntegerTy())
        continue;

      if (IsVTable(ETy)) {
        HasVT = true;
        continue;
      }

      if (IsPaddingFieldCandidate(ETy))
        continue;

      auto *Pointee = ExtractPointeeTy(ETy);
      if (!Pointee)
        return false;

      if (auto St = ExtractStructTy(Pointee))
        if (HasNoData(St)) {
          ++NoDataPointerFields;
          continue;
        }

      if (ExtractPointeeTy(Pointee)) {
        ++ElemDataPointerFields;
        continue;
      }

      return false;
    }

    if (NoDataPointerFields > 1 || ElemDataPointerFields != 1)
      return false;

    return true;
  };

  // Outer struct set.
  Struct = ExtractStructTy(Ty);

  if (!Struct)
    return FALSE();

  if (Struct->getNumElements() < 2)
    return FALSE();

  // Set ArrayFieldOffsets and MemoryInterface.
  {
    unsigned NumberPtrsToNoData = 0;
    unsigned NumberPtrsToArr = 0;
    unsigned Offset = -1U;
    for (auto *FTy : Struct->elements()) {
      ++Offset;

      if (IsPaddingFieldCandidate(FTy))
        continue;

      auto *Pointee = ExtractStructTy(ExtractPointeeTy(FTy));
      if (!Pointee)
        return FALSE();

      if (HasNoData(Pointee)) {
        ++NumberPtrsToNoData;
        MemoryInterface = Pointee;
        continue;
      }

      if (auto *S = StripTrivialDerivedClasses(Pointee)) {
        bool HasVT = false;
        if (IsPtrToArrayCandidate(S, HasVT)) {
          ++NumberPtrsToArr;

          // Too many fields to analyze.
          if (MaxNumFieldTotalCandidates < NumberPtrsToArr)
            return FALSE();

          // Ignore classes with non-trivial base classes and/or vtable.
          if (!HasVT && S == Pointee)
            ArrayFieldOffsets.push_back(Offset);
          continue;
        }
      }

      return FALSE();
    }

    if (getNumArrays() < 2 || getNumArrays() > MaxNumFieldCandidates ||
        NumberPtrsToNoData > 1)
      return FALSE();
  }

  // Set BasePointerOffset.
  // Check first array candidate.
  {
    unsigned NumIntFields = 0;
    unsigned Offset = -1U;
    auto *S = *fields_begin();
    for (auto E : S->elements()) {
      ++Offset;

      if (IsPaddingFieldCandidate(E))
        continue;

      if (E->isIntegerTy()) {
        ++NumIntFields;
        continue;
      }

      if (auto *F = ExtractStructTy(ExtractPointeeTy(E))) {
        if (MemoryInterface != F)
          return FALSE();
        continue;
      }

      // Only pointers as elements are permitted.
      // This assumption simplifies allocation size computations.
      if (!ExtractPointeeTy(ExtractPointeeTy(E)))
        return FALSE();

      BasePointerOffset = Offset;
    }

    // There should be at least 2 integer fields: 'capacity' and 'size' fields.
    // There should be base pointer field.
    if (NumIntFields > MaxNumIntFields || NumIntFields < 2 ||
        BasePointerOffset == -1U)
      return FALSE();
  }

  // Compare classes representing arrays.
  {
    auto *S = *fields_begin();
    // Compare first array candidate with remaining.
    for (auto *S1 : fields()) {
      if (S1->getNumElements() != S->getNumElements())
        return FALSE();

      for (auto Pair : zip_first(S->elements(), S1->elements())) {
        auto *E = std::get<1>(Pair);

        if (auto *F = ExtractStructTy(ExtractPointeeTy(E))) {
          if (MemoryInterface != F)
            return FALSE();
          continue;
        }

        if (std::get<0>(Pair) == E)
          if (IsPaddingFieldCandidate(E) || E->isIntegerTy())
            continue;

        // Only pointers as elements are permitted.
        // This assumption simplifies allocation size computations.
        if (!ExtractPointeeTy(ExtractPointeeTy(E)))
          return FALSE();

        assert(ExtractPointeeTy(ExtractPointeeTy(std::get<0>(Pair))) &&
               "Non-exhaustive checks of 0th candidate");
      }
    }
  }

  return true;
}

bool SOAToAOSTransformImpl::CandidateCFGInfo::populateCFGInformation(
    SOAToAOSTransformImpl &Impl, Module &M) {

  auto GetThisParameter = [](Function &F) -> StructType * {
    FunctionType *FTy = F.getFunctionType();

    if (FTy->getNumParams() < 1)
      return nullptr;

    if (auto *PTy = dyn_cast<PointerType>(*FTy->param_begin()))
      if (auto *STy = dyn_cast<StructType>(PTy->getPointerElementType()))
        return STy;

    return nullptr;
  };

  // Cannot process declarations and varargs.
  auto IsValidMethod = [](Function &F) -> bool {
    if (F.isDeclaration() || F.isVarArg())
      return false;
    return true;
  };

  for (auto *ArrTy : fields()) {
    auto *FI = cast_or_null<dtrans::StructInfo>(Impl.DTInfo.getTypeInfo(ArrTy));

    if (!FI)
      return FALSE();

    // May restrict analysis to Struct's methods.
    auto &CG = FI->getCallSubGraph();
    if (CG.isTop() || CG.isBottom() || CG.getEnclosingType() != Struct)
      return FALSE();
  }

  // Initialize to fields' methods to empty sets.
  ArrayFieldsMethods.assign(getNumArrays(), MethodSetTy());

  for (auto &F : M)
    if (auto *ThisTy = GetThisParameter(F)) {
      if (ThisTy == Struct) {

        if (!IsValidMethod(F))
          return FALSE();

        StructMethods.push_back(&F);
        continue;
      }

      // Tolerating linear search, because ArrayFieldOffsets.size() is very
      // small.
      for (auto Pair : zip_first(fields(), methodsets()))
        if (ThisTy == std::get<0>(Pair)) {
          if (!IsValidMethod(F))
            return FALSE();

          // Structure of array should have simple call graph wrt to arrays'
          // methods.
          if (F.hasNUsesOrMore(MaxNumFieldMethodUses + 1))
            return FALSE();

          std::get<1>(Pair)->push_back(&F);
        }
    }

  for (auto Triple : zip_first(methodsets(), elements(), fields())) {
    if (std::get<0>(Triple)->size() > MaxNumMethods)
      return FALSE();

    for (auto *F : *std::get<0>(Triple)) {
      // Given that only primitive methods are recognized,
      // restrict the size.
      if (std::distance(F->begin(), F->end()) > MaxMethodBBCount)
        return FALSE();

      for (auto &Arg : F->args()) {
        auto *Ty = Arg.getType();
        if (Ty->isIntegerTy())
          continue;
        // By-value argument of element type.
        if (Ty == std::get<1>(Triple))
          continue;

        if (!Ty->isPointerTy())
          return FALSE();

        if (Ty->getPointerAddressSpace())
          return FALSE();

        auto *Pointee = Ty->getPointerElementType();
        if (Pointee == MemoryInterface)
          continue;

        // Only MemoryInterface and by-value argument of element type can be
        // captured.
        if (!Arg.hasNoCaptureAttr())
          return FALSE();

        // See list of methods supported.
        if (Pointee != std::get<1>(Triple) && Pointee != std::get<2>(Triple))
          return FALSE();
      }
    }
  }

  assert(StructMethods.size() && "There should be methods for outer struct");

  LLVM_DEBUG({
    dbgs() << "  Struct's " << Struct->getName() << " methods:\n";
    for (auto *F : StructMethods) {
      dbgs() << "   " << F->getName() << "\n";
    }

    for (auto Pair : zip_first(fields(), methodsets())) {
      dbgs() << "  Fields's " << std::get<0>(Pair)->getName() << " methods:\n";
      for (auto *F : *std::get<1>(Pair)) {
        dbgs() << "   " << F->getName() << ", #uses = " << F->getNumUses()
               << "\n";
      }
    }
  });

  return true;
}

bool SOAToAOSTransformImpl::CandidateSideEffectsInfo::populateSideEffects(
    SOAToAOSTransformImpl &Impl, Module &M) {
  for (auto Pair : zip_first(methodsets(), fields()))
    for (auto *F : *std::get<0>(Pair)) {
      DepCompute DC(Impl.DTInfo, Impl.DL, Impl.TLI, F, std::get<1>(Pair),
                    // *this as DepMap to fill.
                    *this);

      auto &AllMethods = *std::get<0>(Pair);

      // Mark calls to methods as known.
      std::function<bool(const Function *)> IsMethod =
          [&AllMethods](const Function *F) -> bool {
        return std::find(AllMethods.begin(), AllMethods.end(), F) !=
               AllMethods.end();
      };
      DC.computeDepApproximation(IsMethod);

      DEBUG_WITH_TYPE(DTRANS_SOADEP, {
        dbgs() << "; Dump computed dependencies ";

        DepMap::DepAnnotatedWriter Annotate(*this);
        F->print(dbgs(), &Annotate);
      });
    }

  for (auto *F: StructMethods) {
    DepCompute DC(Impl.DTInfo, Impl.DL, Impl.TLI, F, Struct, *this);

    std::function<bool(const Function *)> IsMethod =
        [F](const Function *F1) -> bool {
      return F == F1;
    };

    DC.computeDepApproximation(IsMethod);

    DEBUG_WITH_TYPE(DTRANS_SOADEP, {
      dbgs() << "; Dump computed dependencies ";

      DepMap::DepAnnotatedWriter Annotate(*this);
      F->print(dbgs(), &Annotate);
    });
  }

  // Direct map from MK to set of methods.
  Classifications.assign(getNumArrays(),
                         ClassifyMethodsTy(MK_Last + 1, MethodKindSetTy()));

  for (auto Tuple :
       zip_first(methodsets(), fields(), elements(), methodbins())) {
    SmallVector<const Function *, 1> MethodsCalled;
    for (auto *F : *std::get<0>(Tuple)) {
      SummaryForIdiom S(std::get<1>(Tuple), std::get<2>(Tuple), MemoryInterface,
                        F);
      LLVM_DEBUG(dbgs() << "; Checking array's method " << F->getName()
                        << "\n");

      ComputeArrayMethodClassification MC(Impl.DL,
                                          // *this as DepMap to query.
                                          *this, S);
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
    if (MethodsCalled.size() > 1)
      return FALSE();
    else if (MethodsCalled.size() == 1) {
      if ((*std::get<3>(Tuple))[MK_Realloc][0] != MethodsCalled[0])
        return FALSE();
    }
  }

  // Check combined methods.
  auto &FirstBins = **methodbins_begin();
  auto &FirstSet = **methodsets_begin();

  GlobalNumberState GNS;
  for (auto Tuple :
       zip_first(make_range(methodbins_begin() + 1, methodbins_end()),
                 make_range(methodsets_begin() + 1, methodsets_end()))) {
    auto &OtherBins = *std::get<0>(Tuple);
    auto &OtherSet = *std::get<1>(Tuple);
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
          GNS.setEqual(F, O);
          if (std::find(FirstSet.begin(), FirstSet.end(),
                        cast<Instruction>(F->use_begin()->getUser())
                            ->getParent()
                            ->getParent()) == FirstSet.end() ||
              std::find(OtherSet.begin(), OtherSet.end(),
                        cast<Instruction>(O->use_begin()->getUser())
                            ->getParent()
                            ->getParent()) == OtherSet.end())
            if (!compareCallSites(F, O, static_cast<MethodKind>(i)))
              return FALSE();
        } else
          return FALSE();
        break;
      }
      // Not combined methods.
      case MK_GetInteger:
      case MK_Set:
      case MK_GetElement:
        break;
      }
  }

  return true;
}

// FIXME: Fill-in placeholder.
// Need to compare arguments and check adjacency of calls.
bool SOAToAOSTransformImpl::CandidateSideEffectsInfo::compareCallSites(
    Function *F1, Function* F2, MethodKind MK) const {
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

    if (!Info->populateCFGInformation(*this, M)) {
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
