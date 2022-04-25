//===---------------- CodeAlign.cpp - DTransCodeAlignPass -----------------===//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the DTrans Code Align pass.
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

#include "Intel_DTrans/Transforms/CodeAlign.h"
#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "Intel_DTrans/Analysis/DTransAnnotator.h"
#include "Intel_DTrans/DTransCommon.h"
#include "Intel_DTrans/Transforms/MemManageInfoImpl.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"

using namespace llvm;

#define DEBUG_TYPE "dtrans-codealign"

namespace {

class DTransCodeAlignWrapper : public ModulePass {
private:
  dtrans::CodeAlignPass Impl;

public:
  static char ID;

  DTransCodeAlignWrapper() : ModulePass(ID) {
    initializeDTransCodeAlignWrapperPass(*PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;

    auto &WPInfo = getAnalysis<WholeProgramWrapperPass>().getResult();
    return Impl.runImpl(M, WPInfo);
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<WholeProgramWrapperPass>();
    AU.addPreserved<DTransAnalysisWrapper>();
    AU.addPreserved<WholeProgramWrapperPass>();
  }
};
} //  end anonymous namespace

PreservedAnalyses dtrans::CodeAlignPass::run(Module &M,
                                             ModuleAnalysisManager &AM) {
  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);
  if (!runImpl(M, WPInfo))
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserve<DTransAnalysis>();
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}

namespace llvm {
namespace dtrans {

class CodeAlignImpl {
public:
  CodeAlignImpl(Module &M) : M(M){};
  bool run();

private:
  Module &M;

  // New alignment is used for selected functions.
  static constexpr int NewAlignment = 64;

  // Heuristic: Minimum size of a function to consider it as
  // a candidate.
  static constexpr int MaxFunctionSize = 15;

  // Max limit: Number of candidate functions.
  static constexpr int MaxNumberFunctions = 1;

  bool collectFunctionsToAlign();

  void increaseAlignment();

  bool isCandidateClass(Type *Ty);
  bool isReusableAllocType(Type *Ty);
  bool isStructWithNoRealData(Type *Ty);
  bool isStringVecType(Type *Ty);
  bool isStringStruct(Type *Ty);
  bool isVecType(Type *Ty);

  // Represents "MemoryManager" class in the example.
  StructType *MemInterfaceType = nullptr;

  // Set of Candidate classes.
  SmallPtrSet<Type *, 2> CandidateClasses;

  // Set of candidate Functions.
  SmallVector<Function *, 4> FunctionsToAlign;
};

bool CodeAlignImpl::run() {

  // Collect candidate classes by looking at struct types.
  for (auto *Ty : M.getIdentifiedStructTypes()) {
    if (!isCandidateClass(Ty))
      continue;
    LLVM_DEBUG(dbgs() << "DTRANS Code Align: Candidate Class: " << *Ty
                      << "\n";);
    CandidateClasses.insert(Ty);
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
bool CodeAlignImpl::isReusableAllocType(Type *Ty) {
  auto *STy = getValidStructTy(Ty);
  if (!STy || STy->getNumElements() != 1)
    return false;
  if (!isa<StructType>(STy->getElementType(0)))
    return false;
  return true;
}

// Returns true if 'Ty' is a struct that doesn't have any real data
// except vftable.
// Ex:
//      %"MemoryManager" = type { i32 (...)** }
bool CodeAlignImpl::isStructWithNoRealData(Type *Ty) {
  auto *STy = getValidStructTy(Ty);
  if (!STy || STy->getNumElements() > 1)
    return false;
  if (STy->getNumElements() == 1 && !isVFTablePointer(STy->getElementType(0)))
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
bool CodeAlignImpl::isStringVecType(Type *Ty) {
  auto *STy = getValidStructTy(Ty);
  if (!STy)
    return false;

  unsigned NumStringPtr = 0;
  unsigned NumInt = 0;
  unsigned NumMemInterface = 0;
  for (auto *ETy : STy->elements()) {
    if (ETy->isIntegerTy(64)) {
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
bool CodeAlignImpl::isStringStruct(Type *Ty) {
  auto *STy = getValidStructTy(Ty);
  if (!STy)
    return false;

  unsigned NumInt = 0;
  unsigned NumVec = 0;
  unsigned NumUnused = 0;
  for (auto *ETy : STy->elements()) {
    if (ETy->isIntegerTy(32)) {
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
bool CodeAlignImpl::isVecType(Type *Ty) {
  auto *STy = getValidStructTy(Ty);
  if (!STy)
    return false;

  unsigned NumInt = 0;
  unsigned NumShortPtr = 0;
  unsigned NumMemInterface = 0;
  for (auto *ETy : STy->elements()) {
    if (ETy->isIntegerTy(64)) {
      NumInt++;
      continue;
    }
    auto *PTy = getPointeeType(ETy);
    if (!PTy)
      return false;
    if (PTy->isIntegerTy(16)) {
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
bool CodeAlignImpl::isCandidateClass(Type *Ty) {
  auto *STy = getValidStructTy(Ty);
  if (!STy)
    return false;

  unsigned NumStrVectors = 0;
  unsigned NumReusableAlloc = 0;
  unsigned NumInt = 0;
  unsigned NumUnused = 0;
  Type *StringVecType = nullptr;
  for (auto *ETy : STy->elements()) {
    if (!StringVecType && StringVecType == ETy) {
      NumStrVectors++;
      continue;
    }
    if (isStringVecType(ETy)) {
      NumStrVectors++;
      StringVecType = ETy;
      continue;
    }
    if (ETy->isIntegerTy(32)) {
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

    auto *ThisTy = getThisClassType(&F);
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

bool CodeAlignPass::runImpl(Module &M, WholeProgramInfo &WPInfo) {
  auto TTIAVX2 = TargetTransformInfo::AdvancedOptLevel::AO_TargetHasIntelAVX2;
  if (!WPInfo.isWholeProgramSafe() || !WPInfo.isAdvancedOptEnabled(TTIAVX2)) {
    LLVM_DEBUG(
        dbgs() << "DTRANS Code Align: inhibited -- not whole program safe");
    return false;
  }
  if (!M.getContext().supportsTypedPointers()) {
    LLVM_DEBUG(dbgs() << "DTRANS Code Align: inhibited -- opaque pointers");
    return false;
  }

  CodeAlignImpl Impl(M);
  return Impl.run();
}

} // end namespace dtrans
} // end namespace llvm

char DTransCodeAlignWrapper::ID = 0;
INITIALIZE_PASS_BEGIN(DTransCodeAlignWrapper, "dtrans-codealign",
                      "DTrans code align", false, false)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_END(DTransCodeAlignWrapper, "dtrans-codealign",
                    "DTrans code align", false, false)

ModulePass *llvm::createDTransCodeAlignWrapperPass() {
  return new DTransCodeAlignWrapper();
}
