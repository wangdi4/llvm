//===----------------------DTransSafetyAnalyzer.cpp-----------------------===//
//
// Copyright (C) 2020-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===---------------------------------------------------------------------===//

#include "Intel_DTrans/Analysis/DTransSafetyAnalyzer.h"

#include "Intel_DTrans/Analysis/DTrans.h"
#include "Intel_DTrans/Analysis/DTransTypes.h"
#include "Intel_DTrans/Analysis/DTransUtils.h"
#include "Intel_DTrans/Analysis/PtrTypeAnalyzer.h"
#include "Intel_DTrans/Analysis/TypeMetadataReader.h"
#include "Intel_DTrans/DTransCommon.h"

#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"

#define DEBUG_TYPE "dtrans-safetyanalyzer"

using namespace llvm;
using namespace dtrans;

// This class is responsible for analyzing the LLVM IR using information
// collected by the PtrTypeAnalyzer class to mark the DTrans safety bits on the
// TypeInfo objects managed by the DTransSafetyInfo class. This will also
// collect the field usage information for structure fields.
class DTransSafetyInstVisitor : public InstVisitor<DTransSafetyInstVisitor> {
public:
  DTransSafetyInstVisitor(LLVMContext &Ctx, const DataLayout &DL,
                          DTransSafetyInfo &DTInfo, PtrTypeAnalyzer &PTA,
                          DTransTypeManager &TM)
      : DL(DL), DTInfo(DTInfo), PTA(PTA), TM(TM) {
    DTransI8Type = TM.getOrCreateAtomicType(llvm::Type::getInt8Ty(Ctx));
    DTransI8PtrType = TM.getOrCreatePointerType(DTransI8Type);
    DTransPtrSizedIntType = TM.getOrCreateAtomicType(
        llvm::Type::getIntNTy(Ctx, DL.getPointerSizeInBits()));
    DTransPtrSizedIntPtrType = TM.getOrCreatePointerType(DTransPtrSizedIntType);
  }

  DTransType *getDTransI8Type() const { return DTransI8Type; }
  DTransPointerType *getDTransI8PtrType() const { return DTransI8PtrType; }
  DTransType *getDTransPtrSizedIntType() const { return DTransPtrSizedIntType; }
  DTransPointerType *getDTransPtrSizedIntPtrType() const {
    return DTransPtrSizedIntPtrType;
  }

  void visitModule(Module &M) {
    for (StructType *Ty : M.getIdentifiedStructTypes())
      if (auto *DTStructType = TM.getStructType(Ty->getStructName()))
        DTInfo.getOrCreateTypeInfo(DTStructType);

    // TODO: Collect basic safety bits on the structures.
  }

private:
  const DataLayout &DL;
  DTransSafetyInfo &DTInfo;
  PtrTypeAnalyzer &PTA;
  DTransTypeManager &TM;

  // Types that are frequently needed for comparing type aliases against
  // known types.
  DTransType *DTransI8Type = nullptr;                    // i8
  DTransPointerType *DTransI8PtrType = nullptr;          // i8*
  DTransType *DTransPtrSizedIntType = nullptr;           // i64 or i32
  DTransPointerType *DTransPtrSizedIntPtrType = nullptr; // i64* or i32*
};

DTransSafetyInfo::DTransSafetyInfo(DTransSafetyInfo &&Other)
    : TM(std::move(Other.TM)), MDReader(std::move(Other.MDReader)),
      PtrAnalyzer(std::move(Other.PtrAnalyzer)),
      DTransSafetyAnalysisRan(Other.DTransSafetyAnalysisRan) {}

DTransSafetyInfo::~DTransSafetyInfo() { reset(); }

DTransSafetyInfo &DTransSafetyInfo::operator=(DTransSafetyInfo &&Other) {
  reset();
  TM = std::move(Other.TM);
  MDReader = std::move(Other.MDReader);
  PtrAnalyzer = std::move(Other.PtrAnalyzer);
  DTransSafetyAnalysisRan = Other.DTransSafetyAnalysisRan;
  return *this;
}

void DTransSafetyInfo::analyzeModule(
    Module &M,
    std::function<const TargetLibraryInfo &(const Function &)> GetTLI,
    WholeProgramInfo &WPInfo,
    function_ref<BlockFrequencyInfo &(Function &)> GetBFI) {

  LLVM_DEBUG(dbgs() << "DTransSafetyInfo::analyzeModule running\n");
  if (!WPInfo.isWholeProgramSafe()) {
    LLVM_DEBUG(dbgs() << "  DTransSafetyInfo: Not Whole Program Safe\n");
    return;
  }

  LLVMContext &Ctx = M.getContext();
  const DataLayout &DL = M.getDataLayout();
  TM = std::make_unique<DTransTypeManager>(Ctx);
  MDReader = std::make_unique<TypeMetadataReader>(*TM);
  if (!MDReader->initialize(M)) {
    LLVM_DEBUG(dbgs() << "DTransSafetyInfo: Type metadata reader did not find "
                         "structure type metadata\n");
    return;
  }

  PtrAnalyzer =
      std::make_unique<PtrTypeAnalyzer>(Ctx, *TM, *MDReader, DL, GetTLI);
  PtrAnalyzer->run(M);
  LLVM_DEBUG(dbgs() << "DTransSafetyInfo: PtrTypeAnalyzer complete\n");

  DTransSafetyInstVisitor Visitor(Ctx, DL, *this, *PtrAnalyzer, *TM);
  Visitor.visit(M);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  if (DTransPrintAnalyzedTypes)
    printAnalyzedTypes();
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
}

void DTransSafetyInfo::reset() {
  // Clean up the TypeInfoMap.
  // DTransSafetyInfo owns the TypeInfo objects in the TypeInfoMap.
  // The DTransType pointers are owned by the DTransTypeManager, and
  // will be cleaned up when that object is reset.
  for (auto Entry : TypeInfoMap) {
    switch (Entry.second->getTypeInfoKind()) {
    case dtrans::TypeInfo::NonAggregateInfo:
      delete cast<dtrans::NonAggregateTypeInfo>(Entry.second);
      break;
    case dtrans::TypeInfo::PtrInfo:
      delete cast<dtrans::PointerInfo>(Entry.second);
      break;
    case dtrans::TypeInfo::StructInfo:
      delete cast<dtrans::StructInfo>(Entry.second);
      break;
    case dtrans::TypeInfo::ArrayInfo:
      delete cast<dtrans::ArrayInfo>(Entry.second);
      break;
    }
  }
  TypeInfoMap.clear();

  TM.reset();
  MDReader.reset();
  PtrAnalyzer.reset();
  DTransSafetyAnalysisRan = false;
}

bool DTransSafetyInfo::useDTransSafetyAnalysis() const {
  return DTransSafetyAnalysisRan;
}

TypeInfo *DTransSafetyInfo::getOrCreateTypeInfo(DTransType *Ty) {
  // If we already have this type in our map, just return it.
  auto *TI = getTypeInfo(Ty);
  if (TI)
    return TI;

  // Create the DTrans TypeInfo object for this type and any sub-types.
  TypeInfo *DTransTI;
  if (Ty->isPointerTy()) {
    // For pointer types, we want to record the pointer type info
    // and then record what it points to. We must add the pointer to the
    // map early to avoid infinite recursion.
    DTransTI = new dtrans::PointerInfo(Ty);
    TypeInfoMap[Ty] = DTransTI;
    getOrCreateTypeInfo(cast<DTransPointerType>(Ty)->getPointerElementType());
    return DTransTI;
  } else if (Ty->isArrayTy()) {
    dtrans::TypeInfo *ElementInfo =
        getOrCreateTypeInfo(Ty->getArrayElementType());
    DTransTI = new dtrans::ArrayInfo(
        Ty, ElementInfo, cast<DTransArrayType>(Ty)->getNumElements());
  } else if (Ty->isStructTy()) {
    DTransStructType *STy = cast<DTransStructType>(Ty);
    SmallVector<dtrans::AbstractType, 16> FieldTypes;
    unsigned NumFields = STy->getNumFields();
    for (unsigned Idx = 0; Idx < NumFields; ++Idx) {
      DTransType *FieldTy = STy->getFieldType(Idx);
      assert(FieldTy && "Expected non-null field type");
      getOrCreateTypeInfo(FieldTy);
      FieldTypes.push_back(FieldTy);
    }
    // TODO: StructInfo constructor should be cleaned up to not require
    // the IgnoreFor parameter, because they are always created with 0,
    // and then updated with a call to the SetIgnoreFor method.
    DTransTI = new dtrans::StructInfo(Ty, FieldTypes, /*IgnoreFor=*/0);
  } else {
    // TODO: TypeInfo does not support vector types, so they will be stored
    // within NonAggregateTypeInfo objects currently.
    assert(!Ty->isAggregateType() &&
           "DTransAnalysisInfo::getOrCreateTypeInfo unexpected aggregate type");
    DTransTI = new dtrans::NonAggregateTypeInfo(Ty);
  }

  TypeInfoMap[Ty] = DTransTI;
  return DTransTI;
}

TypeInfo *DTransSafetyInfo::getTypeInfo(DTransType *Ty) const {
  // If we have this type in our map, return it.
  auto It = TypeInfoMap.find(Ty);
  if (It != TypeInfoMap.end())
    return It->second;

  return nullptr;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void DTransSafetyInfo::printAnalyzedTypes() {
  std::vector<dtrans::TypeInfo *> TypeInfoEntries;
  for (auto *TI : type_info_entries())
    if (isa<dtrans::ArrayInfo>(TI) || isa<dtrans::StructInfo>(TI))
      TypeInfoEntries.push_back(TI);

  std::sort(TypeInfoEntries.begin(), TypeInfoEntries.end(),
            [](dtrans::TypeInfo *A, dtrans::TypeInfo *B) {
              std::string TypeStrA;
              llvm::raw_string_ostream RSO_A(TypeStrA);
              A->getDTransType()->print(RSO_A);
              std::string TypeStrB;
              llvm::raw_string_ostream RSO_B(TypeStrB);
              B->getDTransType()->print(RSO_B);
              return RSO_A.str().compare(RSO_B.str()) < 0;
            });

  dbgs() << "================================\n";
  dbgs() << " DTRANS Analysis Types Created\n";
  dbgs() << "================================\n\n";

  for (auto TI : TypeInfoEntries)
    if (auto *AI = dyn_cast<dtrans::ArrayInfo>(TI))
      AI->print(dbgs());
    else if (auto *SI = dyn_cast<dtrans::StructInfo>(TI))
      SI->print(dbgs());

  dbgs().flush();
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// Provide a definition for the static class member used to identify passes.
AnalysisKey DTransSafetyAnalyzer::Key;

DTransSafetyInfo DTransSafetyAnalyzer::run(Module &M,
                                           ModuleAnalysisManager &AM) {
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetBFI = [&FAM](Function &F) -> BlockFrequencyInfo & {
    return FAM.getResult<BlockFrequencyAnalysis>(F);
  };
  auto GetTLI = [&FAM](const Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(*(const_cast<Function *>(&F)));
  };
  WholeProgramInfo &WPInfo = AM.getResult<WholeProgramAnalysis>(M);

  DTransSafetyInfo DTResult;
  DTResult.analyzeModule(M, GetTLI, WPInfo, GetBFI);
  return DTResult;
}

namespace {
class DTransSafetyAnalyzerWrapper : public ModulePass {

public:
  static char ID;

  DTransSafetyAnalyzerWrapper() : ModulePass(ID) {
    initializeDTransSafetyAnalyzerWrapperPass(*PassRegistry::getPassRegistry());
  }

  DTransSafetyInfo &getDTransSafetyInfo(Module &M) { return Result; }

  bool runOnModule(Module &M) override {
    auto GetBFI = [this](Function &F) -> BlockFrequencyInfo & {
      return this->getAnalysis<BlockFrequencyInfoWrapperPass>(F).getBFI();
    };
    auto GetTLI = [this](const Function &F) -> TargetLibraryInfo & {
      return this->getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
    };

    WholeProgramInfo &WPInfo =
        getAnalysis<WholeProgramWrapperPass>().getResult();
    Result.analyzeModule(M, GetTLI, WPInfo, GetBFI);
    return false;
  }

  bool doFinalization(Module &M) override {
    Result.reset();
    return false;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
    AU.addRequired<TargetLibraryInfoWrapperPass>();
    AU.addRequired<BlockFrequencyInfoWrapperPass>();
    AU.addRequired<WholeProgramWrapperPass>();
  }

private:
  DTransSafetyInfo Result;
};
} // end anonymous namespace

INITIALIZE_PASS_BEGIN(DTransSafetyAnalyzerWrapper, "dtrans-safetyanalyzer",
                      "Data transformation safety analyzer", false, true)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(BlockFrequencyInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_END(DTransSafetyAnalyzerWrapper, "dtrans-safetyanalyzer",
                    "Data transformation safety analyzer", false, true)

char DTransSafetyAnalyzerWrapper::ID = 0;

ModulePass *llvm::createDTransSafetyAnalyzerTestWrapperPass() {
  return new DTransAnalysisWrapper();
}
