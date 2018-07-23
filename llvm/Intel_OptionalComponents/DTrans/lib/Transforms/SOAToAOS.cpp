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
#include "llvm/Pass.h"

using namespace llvm;
using namespace dtrans;

#define DEBUG_TYPE "dtrans-soatoaos"

static cl::opt<bool>
    DTransSOAToAOSGlobalGuard("enable-dtrans-soatoaos", cl::init(false),
                              cl::Hidden, cl::desc("Enable DTrans SOAToAOS"));

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// Compare first and second fields (1-based numbering) of given type
// for congruence. See FIXME in SOAToAOSTransformImpl.
static cl::opt<std::string> DTransSOAToAOSType("dtrans-soatoaos-typename",
                                               cl::ReallyHidden);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

namespace {
class SOAToAOSTransformImpl : public DTransOptBase {
public:
  SOAToAOSTransformImpl(DTransAnalysisInfo &DTInfo, LLVMContext &Context,
                        const DataLayout &DL, const TargetLibraryInfo &TLI,
                        StringRef DepTypePrefix,
                        DTransTypeRemapper *TypeRemapper)
      : DTransOptBase(DTInfo, Context, DL, TLI, DepTypePrefix, TypeRemapper) {}

  ~SOAToAOSTransformImpl() {}

private:
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
  //    %elem**,
  //    MemoryManager*
  // }
  // %class.vector.0 = type {
  //    i8, i32, i32,
  //    %class.elem0**,
  //    MemoryManager*
  // }
  class CandidateLayoutInfo {
    constexpr static int MaxNumFieldArrayCandidates = 3;
    constexpr static int MaxNumFieldCandidates = 2;
    // There should be at least 'capacity' and 'size' fields.
    // Also permit auxiliary field.
    constexpr static int MaxNumIntFields = 3;

  public:
    StructType *Struct = nullptr;
    // Offsets in Struct's elements() to represent pointers to candidate
    // _arrays_. _Arrays_ are represented as some classes.
    SmallVector<unsigned, MaxNumFieldArrayCandidates> ArrayFieldOffsets;
    // Offset inside _arrays_' elements(), which represent base pointers to
    // allocated memory.
    unsigned BasePointerOffset = -1U;
    // Memory management pointer.
    StructType *MemoryInterface = nullptr;

    // Returns false if idiom is not recognized.
    // Single out-of-line method with lambdas.
    bool populateLayoutInformation(Type *Ty);
  };

  class CandidateInfo : public CandidateLayoutInfo {
  public:
    void setCandidateStructs(SOAToAOSTransformImpl &Impl,
                             dtrans::StructInfo *S) {
      StructsToConvert.push_back(S);

      unsigned I = 0;
      for (auto Index : ArrayFieldOffsets) {
        I++;
        auto *OrigTy = cast<StructType>(
            cast<PointerType>(cast<StructType>(getOuterStruct()->getLLVMType())
                                  ->getElementType(Index))
                ->getElementType());
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

    dtrans::StructInfo *getOuterStruct() const {
      return StructsToConvert[0];
    }

  private:
    SmallVector<dtrans::StructInfo *, 3> StructsToConvert;
  };

  constexpr static int MaxNumStructCandidates = 1;

  SmallVector<CandidateInfo, MaxNumStructCandidates> Candidates;

  // A mapping from the original structure type to the new structure type
  TypeToTypeMap OrigToNewTypeMapping;
};

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

  auto StripTrivialDerivedClasses = [](StructType *STy) -> StructType * {
    auto *I = STy;
    for (; I && I->getNumElements() == 1;
         I = dyn_cast<StructType>(I->getElementType(0))) {
    }
    return I;
  };

  // Looks like array wrapper.
  auto IsPtrToArrayCandidate = [&ExtractStructTy, &ExtractPointeeTy, &IsVTable,
                                &HasNoData](StructType *STy,
                                            bool &HasVT) -> bool {
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
    return false;

  if (Struct->getNumElements() < 2)
    return false;

  // Set ArrayFieldOffsets and MemoryInterface.
  {
    unsigned NumberPtrsToNoData = 0;
    unsigned NumberPtrsToArr = 0;
    unsigned Offset = -1U;
    for (auto *FTy : Struct->elements()) {
      ++Offset;

      auto *Pointee = ExtractStructTy(ExtractPointeeTy(FTy));
      if (!Pointee)
        return false;

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
          if (MaxNumFieldArrayCandidates < NumberPtrsToArr)
            return false;

          // Ignore classes with non-trivial base classes and/or vtable.
          if (!HasVT && S == Pointee)
            ArrayFieldOffsets.push_back(Offset);
          continue;
        }
      }

      return false;
    }

    if (ArrayFieldOffsets.size() < 2 || NumberPtrsToNoData > 1 ||
        ArrayFieldOffsets.size() > MaxNumFieldCandidates)
      return false;
  }

  // Set BasePointerOffset.
  // Check first array candidate.
  {
    unsigned NumIntFields = 0;
    unsigned Offset = -1U;
    for (auto E : cast<StructType>(Struct->getElementType(ArrayFieldOffsets[0])
                                       ->getPointerElementType())
                      ->elements()) {
      ++Offset;

      if (E->isIntegerTy()) {
        ++NumIntFields;
        continue;
      }

      if (auto *F = ExtractStructTy(ExtractPointeeTy(E))) {
        if (MemoryInterface != F)
          return false;
        continue;
      }

      if (!ExtractPointeeTy(ExtractPointeeTy(E)))
        return false;

      BasePointerOffset = Offset;
    }

    // There should be at least 'capacity' and 'size' fields.
    if (NumIntFields > MaxNumIntFields || NumIntFields < 2)
      return false;
  }

  // Compare classes representing arrays.
  {
    auto *S = cast<StructType>(
        Struct->getElementType(ArrayFieldOffsets[0])->getPointerElementType());

    // Compare first array candidate with remaining.
    for (auto I = ArrayFieldOffsets.begin() + 1, E = ArrayFieldOffsets.end();
         I != E; ++I) {

      auto *S1 =
          cast<StructType>(Struct->getElementType(*I)->getPointerElementType());

      if (S1->getNumElements() != S->getNumElements())
        return false;

      for (auto Pair : zip_first(S->elements(), S1->elements())) {
        auto *E = std::get<0>(Pair);
        auto *E1 = std::get<1>(Pair);

        if (auto *F = ExtractStructTy(ExtractPointeeTy(E1))) {
          if (MemoryInterface != F)
            return false;
          continue;
        }

        if (E == E1 && E1->isIntegerTy())
          continue;

        if (!ExtractPointeeTy(ExtractPointeeTy(E1)))
          return false;

        assert(ExtractPointeeTy(ExtractPointeeTy(E)) &&
               "Non-exhaustive checks of 0th candidate");
      }
    }
  }

  return true;
}

bool SOAToAOSTransformImpl::prepareTypes(Module &M) {

  for (dtrans::TypeInfo *TI : DTInfo.type_info_entries()) {
    CandidateInfo Info;
    if (!Info.populateLayoutInformation(TI->getLLVMType())) {
      LLVM_DEBUG({
        dbgs() << "  Rejecting ";
        TI->getLLVMType()->print(dbgs(), true, true);
        dbgs() << " because it does not look like a candidate.\n";
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

    Info.setCandidateStructs(*this, cast<dtrans::StructInfo>(TI));
    Candidates.emplace_back(Info);
  }

  if (Candidates.empty()) {
    LLVM_DEBUG(dbgs() << "  No candidates found.\n");
    return false;
  }

  for_each(Candidates, [this, &M](CandidateInfo &C) {
    C.prepareTypes(*this, M);
  });

  return true;
}

void SOAToAOSTransformImpl::populateTypes(Module &M) {
  for_each(Candidates, [this, &M](CandidateInfo &C) {
    C.populateTypes(*this, M);
  });
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
