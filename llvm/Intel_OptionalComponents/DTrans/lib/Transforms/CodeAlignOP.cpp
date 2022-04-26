//===--------------- CodeAlignOP.cpp - DTransCodeAlignPass ----------------===//
//
// Copyright (C) 2022-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the DTrans Code Align pass for opaque pointers.
//
// The main goal of this transformation is to increase alignment
// of a routine in one of the benchmarks. The heuristics used in
// the transformation are not generally useful in identifying routines
// that need better code alignment. This transformation will increase
// alignment of selected functions to 64.
//
// Heuristic used to find the routine:
// Collect all member functions of the %”DOMStringCache" class below.
// Ignore member functions like Ctor/Dtor/Single Callsite/Small functions
// Increase code alignment for the remaining member functions of the class.
// Trigger this transformation only when SOAToAOS is applied.
//
// %"DOMStringCache" = type { %"Vector", %"Vector", i32, [4 x i8],
//                            %"DOMStringReusableAllocator" }
// %"Vector" = type { %"MemoryManager"*, i64, i64, %"DOMString"** }
// %"DOMString" = type <{ %"Vector.0", i32, [4 x i8] }>
// %"Vector.0" = type { %"MemoryManager"*, i64, i64, i16* }
// %"DOMStringReusableAllocator" = type { %"ReusableArenaAllocator" }
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/CodeAlignOP.h"
#include "Intel_DTrans/Analysis/DTransAnnotator.h"
#include "Intel_DTrans/Analysis/DTransSafetyAnalyzer.h"
#include "Intel_DTrans/DTransCommon.h"
#include "Intel_DTrans/Transforms/ClassInfoOPUtils.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"

using namespace llvm;
using namespace dtransOP;
using dtrans::DTransAnnotator;

#define DEBUG_TYPE "dtrans-codealignop"

namespace {

class DTransCodeAlignOPWrapper : public ModulePass {
private:
  dtransOP::CodeAlignPass Impl;

public:
  static char ID;

  DTransCodeAlignOPWrapper() : ModulePass(ID) {
    initializeDTransCodeAlignOPWrapperPass(*PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;

    auto &WPInfo = getAnalysis<WholeProgramWrapperPass>().getResult();
    DTransSafetyAnalyzerWrapper &DTAnalysisWrapper =
        getAnalysis<DTransSafetyAnalyzerWrapper>();
    DTransSafetyInfo &DTInfo = DTAnalysisWrapper.getDTransSafetyInfo(M);

    return Impl.runImpl(M, WPInfo, &DTInfo);
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<WholeProgramWrapperPass>();
    AU.addRequired<DTransSafetyAnalyzerWrapper>();
    AU.addPreserved<DTransSafetyAnalyzerWrapper>();
    AU.addPreserved<WholeProgramWrapperPass>();
  }
};
} //  end anonymous namespace

PreservedAnalyses dtransOP::CodeAlignPass::run(Module &M,
                                               ModuleAnalysisManager &AM) {
  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);
  DTransSafetyInfo *DTInfo = &AM.getResult<DTransSafetyAnalyzer>(M);
  if (!runImpl(M, WPInfo, DTInfo))
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserve<DTransSafetyAnalyzer>();
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}

namespace llvm {
namespace dtransOP {

class CodeAlignImpl {
public:
  CodeAlignImpl(Module &M, DTransSafetyInfo *DTInfo) : M(M), DTInfo(DTInfo){};
  bool run();

private:
  Module &M;
  DTransSafetyInfo *DTInfo;

  // New alignment is used for selected functions.
  static constexpr int NewAlignment = 64;

  // Heuristic: Minimum size of a function to consider it as
  // a candidate.
  static constexpr int MaxFunctionSize = 15;

  // Max limit: Number of candidate functions.
  static constexpr int MaxNumberFunctions = 1;

  bool collectFunctionsToAlign();

  void increaseAlignment();

  bool isCandidateClass(DTransType *Ty);
  bool isReusableAllocType(DTransType *Ty);
  bool isStructWithNoRealData(DTransType *Ty);
  bool isStringVecType(DTransType *Ty);
  bool isStringStruct(DTransType *Ty);
  bool isVecType(DTransType *Ty);

  // Represents "MemoryManager" class in the example.
  DTransStructType *MemInterfaceType = nullptr;

  // Set of Candidate classes.
  SmallPtrSet<DTransType *, 2> CandidateClasses;

  // Set of candidate Functions.
  SmallVector<Function *, 4> FunctionsToAlign;
};

bool CodeAlignImpl::run() {

  // Collect candidate classes by looking at struct types.
  for (dtrans::TypeInfo *TI : DTInfo->type_info_entries()) {
    auto *StInfo = dyn_cast<dtrans::StructInfo>(TI);
    if (!StInfo || cast<StructType>(StInfo->getLLVMType())->isLiteral())
      continue;

    if (!isCandidateClass(StInfo->getDTransType()))
      continue;
    LLVM_DEBUG(dbgs() << "DTRANS Code Align: Candidate Class: "
                      << *StInfo->getLLVMType() << "\n";);
    CandidateClasses.insert(StInfo->getDTransType());
  }

  // Check if there are no candidates.
  if (CandidateClasses.empty()) {
    LLVM_DEBUG(
        dbgs() << "DTRANS Code Align: Not triggered ...No candidate classes\n");
    return false;
  }

  // Collect candidate functions to increase alignment.
  if (!collectFunctionsToAlign())
    return false;

  // Bump up alignments of selected functions.
  increaseAlignment();

  return true;
}

// Returns true if 'Ty' is a struct that looks like format below.
//
//  %"DOMStringReusableAllocator" = type { %"ReusableArenaAllocator" }
//
// This doesn't check for the layout of %"ReusableArenaAllocator".
//
bool CodeAlignImpl::isReusableAllocType(DTransType *Ty) {
  auto *STy = getValidStructTy(Ty);
  if (!STy || STy->getNumFields() != 1)
    return false;
  if (!isa<DTransStructType>(STy->getFieldType(0)))
    return false;
  return true;
}

// Returns true if 'Ty' is a struct that doesn't have any real data
// except vftable.
// Ex:
//      %"MemoryManager" = type { i32 (...)** }
bool CodeAlignImpl::isStructWithNoRealData(DTransType *Ty) {
  auto *STy = getValidStructTy(Ty);
  if (!STy || STy->getNumFields() > 1)
    return false;
  if (STy->getNumFields() == 1 && !isPtrToVFTable(STy->getFieldType(0)))
    return false;
  if (!MemInterfaceType)
    MemInterfaceType = STy;
  else if (MemInterfaceType != STy)
    return false;
  return true;
}

// Returns true if 'Ty' is a struct that looks like format below.
// Not checking the order of the fields in 'Ty'.
//
// %"Vector" = type { %"MemoryManager"*, i64, i64, %"DOMString"** }
bool CodeAlignImpl::isStringVecType(DTransType *Ty) {
  auto *STy = getValidStructTy(Ty);
  if (!STy)
    return false;

  unsigned NumStringPtr = 0;
  unsigned NumInt = 0;
  unsigned NumMemInterface = 0;
  for (unsigned I = 0, E = STy->getNumFields(); I != E; ++I) {
    auto *ETy = STy->getFieldType(I);
    assert(ETy && "DTransType was not initialized properly");
    if (ETy->getLLVMType()->isIntegerTy(64)) {
      NumInt++;
      continue;
    }
    auto *PTy = getPointeeType(ETy);
    if (!PTy)
      return false;
    if (isStructWithNoRealData(PTy)) {
      NumMemInterface++;
      continue;
    }
    auto *PPTy = getPointeeType(PTy);
    if (!PPTy)
      return false;
    if (isStringStruct(PPTy)) {
      NumStringPtr++;
      continue;
    }
    return false;
  }
  if (NumStringPtr != 1 || NumInt != 2 || NumMemInterface != 1)
    return false;
  return true;
}

// Returns true if 'Ty' is a struct that looks like format below.
// Not checking the order of the fields in 'Ty'.
//
// %"DOMString" = type <{ %"Vector.0", i32, [4 x i8] }>
bool CodeAlignImpl::isStringStruct(DTransType *Ty) {
  auto *STy = getValidStructTy(Ty);
  if (!STy)
    return false;

  unsigned NumInt = 0;
  unsigned NumVec = 0;
  unsigned NumUnused = 0;
  for (unsigned I = 0, E = STy->getNumFields(); I != E; ++I) {
    auto *ETy = STy->getFieldType(I);
    assert(ETy && "DTransType was not initialized properly");
    if (ETy->getLLVMType()->isIntegerTy(32)) {
      NumInt++;
      continue;
    }
    if (isVecType(ETy)) {
      NumVec++;
      continue;
    }
    if (isPotentialPaddingField(ETy)) {
      NumUnused++;
      continue;
    }
    return false;
  }
  if (NumInt != 1 || NumVec != 1 || NumUnused > 1)
    return false;
  return true;
}

// Returns true if 'Ty' is a struct that looks like format below.
// Not checking the order of the fields in 'Ty'.
//
// %"Vector.0" = type { %"MemoryManager"*, i64, i64, i16* }
bool CodeAlignImpl::isVecType(DTransType *Ty) {
  auto *STy = getValidStructTy(Ty);
  if (!STy)
    return false;

  unsigned NumInt = 0;
  unsigned NumShortPtr = 0;
  unsigned NumMemInterface = 0;
  for (unsigned I = 0, E = STy->getNumFields(); I != E; ++I) {
    auto *ETy = STy->getFieldType(I);
    assert(ETy && "DTransType was not initialized properly");
    if (ETy->getLLVMType()->isIntegerTy(64)) {
      NumInt++;
      continue;
    }
    auto *PTy = getPointeeType(ETy);
    if (!PTy)
      return false;
    if (PTy->getLLVMType()->isIntegerTy(16)) {
      NumShortPtr++;
      continue;
    }
    if (isStructWithNoRealData(PTy)) {
      NumMemInterface++;
      continue;
    }
    return false;
  }
  if (NumInt != 2 || NumShortPtr != 1 || NumMemInterface != 1)
    return false;
  return true;
}

// Returns true if 'Ty' is a struct that looks like format below.
// Not checking the order of the fields in 'Ty'.
//
// %"DOMStringCache" = type { %"Vector", %"Vector", i32, [4 x i8],
//                            %"DOMStringReusableAllocator" }
bool CodeAlignImpl::isCandidateClass(DTransType *Ty) {
  auto *STy = getValidStructTy(Ty);
  if (!STy)
    return false;

  unsigned NumStrVectors = 0;
  unsigned NumReusableAlloc = 0;
  unsigned NumInt = 0;
  unsigned NumUnused = 0;
  DTransType *StringVecType = nullptr;
  for (unsigned I = 0, E = STy->getNumFields(); I != E; ++I) {
    auto *ETy = STy->getFieldType(I);
    assert(ETy && "DTransType was not initialized properly");
    if (!StringVecType && StringVecType == ETy) {
      NumStrVectors++;
      continue;
    }
    if (isStringVecType(ETy)) {
      NumStrVectors++;
      StringVecType = ETy;
      continue;
    }
    if (ETy->getLLVMType()->isIntegerTy(32)) {
      NumInt++;
      continue;
    }
    if (isReusableAllocType(ETy)) {
      NumReusableAlloc++;
      continue;
    }
    if (isPotentialPaddingField(ETy)) {
      NumUnused++;
      continue;
    }
    return false;
  }
  if (NumReusableAlloc != 1 || NumInt != 1 || NumStrVectors != 2 ||
      NumUnused > 1)
    return false;
  return true;
}

// Collect member functions of candidate classes.
bool CodeAlignImpl::collectFunctionsToAlign() {

  bool SOAToAOSApplied = false;
  for (auto &F : M) {

    // Check if SOAToAOS is applied.
    if (DTransAnnotator::hasDTransSOAToAOSTypeAnnotation(F))
      SOAToAOSApplied = true;

    auto *ThisTy = getClassType(&F, DTInfo->getTypeMetadataReader());
    if (!ThisTy || !CandidateClasses.count(ThisTy))
      continue;

    // Ignore Ctor/Dtor functions since they aren't usually
    // hot functions.
    if (F.hasFnAttribute("intel-mempool-destructor") ||
        F.hasFnAttribute("intel-mempool-constructor")) {
      LLVM_DEBUG(dbgs() << "DTRANS Code Align: Function ignored"
                           " (Ctor/Dtor) "
                        << F.getName() << "\n");
      continue;
    }

    // Ignore if current alignment is better than NewAlignment.
    if (F.getAlignment() >= NewAlignment) {
      LLVM_DEBUG(dbgs() << "DTRANS Code Align: Function ignored"
                           " (Has Better Alignment) "
                        << F.getName() << "\n");
      continue;
    }

    // Ignore small routines as they will be inlined.
    if (F.size() < MaxFunctionSize) {
      LLVM_DEBUG(dbgs() << "DTRANS Code Align: Function ignored (Size) "
                        << F.getName() << "\n");
      continue;
    }

    // Ignore single callsite functions as they will be inlined.
    if (F.getNumUses() <= 1) {
      LLVM_DEBUG(dbgs() << "DTRANS Code Align: Function ignored (Single CS) "
                        << F.getName() << "\n");
      continue;
    }

    LLVM_DEBUG(dbgs() << "DTRANS Code Align: Function selected " << F.getName()
                      << "\n");

    FunctionsToAlign.push_back(&F);
  }

  if (FunctionsToAlign.empty()) {
    LLVM_DEBUG(dbgs() << "DTRANS Code Align: inhibited -- Did not find "
                         "function to align:\n");
    return false;
  }

  if (FunctionsToAlign.size() > MaxNumberFunctions) {
    LLVM_DEBUG(dbgs() << "DTRANS Code Align: inhibited -- "
                         "Exceeds number of functions limit\n");
    return false;
  }

  if (!SOAToAOSApplied) {
    LLVM_DEBUG(dbgs() << "DTRANS Code Align: inhibited -- Did not find "
                         "SOA-to-AOS transformed routine:\n");
    return false;
  }

  return true;
}

// Increase alignment of all functions in FunctionsToAlign to NewAlignment.
void CodeAlignImpl::increaseAlignment() {
  for (auto *F : FunctionsToAlign) {
    MaybeAlign Al(NewAlignment);
    F->setAlignment(Al);
    LLVM_DEBUG(dbgs() << "DTRANS Code Align increased: " << F->getName() << ": "
                      << F->getAlignment() << "\n");
  }
}

bool CodeAlignPass::runImpl(Module &M, WholeProgramInfo &WPInfo,
                            DTransSafetyInfo *DTInfo) {
  auto TTIAVX2 = TargetTransformInfo::AdvancedOptLevel::AO_TargetHasIntelAVX2;
  if (!WPInfo.isWholeProgramSafe() || !WPInfo.isAdvancedOptEnabled(TTIAVX2)) {
    LLVM_DEBUG(
        dbgs() << "DTRANS Code Align: inhibited -- not whole program safe");
    return false;
  }

  CodeAlignImpl Impl(M, DTInfo);
  return Impl.run();
}

} // end namespace dtransOP
} // end namespace llvm

char DTransCodeAlignOPWrapper::ID = 0;
INITIALIZE_PASS_BEGIN(DTransCodeAlignOPWrapper, "dtrans-codealignop",
                      "DTrans code align", false, false)
INITIALIZE_PASS_DEPENDENCY(DTransSafetyAnalyzerWrapper)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_END(DTransCodeAlignOPWrapper, "dtrans-codealignop",
                    "DTrans code align", false, false)

ModulePass *llvm::createDTransCodeAlignOPWrapperPass() {
  return new DTransCodeAlignOPWrapper();
}
