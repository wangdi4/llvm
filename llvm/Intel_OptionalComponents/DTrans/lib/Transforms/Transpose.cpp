//===--------------- Transpose.cpp - DTransTransposePass------------------===//
//
// Copyright (C) 2019-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the DTrans Transpose optimization for Fortran
// multi-dimensional arrays.
//
//===----------------------------------------------------------------------===//
#include "Intel_DTrans/Transforms/Transpose.h"
#include "Intel_DTrans/DTransCommon.h"

#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Pass.h"

using namespace llvm;

#define DEBUG_TYPE "dtrans-transpose"

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// Print the list of candidates identified and their analysis result.
static cl::opt<bool> PrintCandidates("dtrans-transpose-print-candidates",
                                     cl::ReallyHidden);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

namespace {

// Maximum rank for a Fortran array.
const uint32_t FortranMaxRank = 9;

// This is the class that manages the analysis and transformation
// of the stride information for a candidate variable.
class TransposeCandidate {
public:
  TransposeCandidate(GlobalVariable *GV, uint32_t ArrayRank,
                     uint64_t ArrayLength, uint64_t ElementSize)
      : GV(GV), ArrayRank(ArrayRank), ArrayLength(ArrayLength),
        ElementSize(ElementSize), IsValid(false) {
    assert(ArrayRank > 0 && ArrayRank <= FortranMaxRank && "Invalid Rank");
    uint64_t Stride = ElementSize;
    for (uint32_t RankNum = 0; RankNum < ArrayRank; ++RankNum) {
      Strides.push_back(Stride);
      Stride *= ArrayLength;
    }
  }

  // This function analyzes a candidate to check whether all uses of the
  // variable are supported for the transformation.
  bool analyze() {
    // TODO: Add the safety and profitability analysis for the candidate
    return false;
  }

  // Transform the strides in the subscript calls and dope vector creation, if
  // the candidate is valid for being transposed.
  bool transform() {
    if (!IsValid)
      return false;

    transposeStrides();
    return true;
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump() { print(dbgs()); }

  void print(raw_ostream &OS) {
    OS << "Transpose candidate: " << GV->getName() << "\n";
    OS << "Type         : " << *GV->getType() << "\n";
    OS << "Rank         : " << ArrayRank << "\n";
    OS << "Length       : " << ArrayLength << "\n";
    OS << "Element size : " << ElementSize << "\n";
    OS << "Strides      :";
    for (uint32_t RankNum = 0; RankNum < ArrayRank; ++RankNum)
      OS << " " << Strides[RankNum];
    OS << "\n";

    OS << "Transposition:";
    if (!Transposition.empty())
      for (uint32_t RankNum = 0; RankNum < ArrayRank; ++RankNum)
        OS << " " << Transposition[RankNum];
    OS << "\n";
    OS << "IsValid      : " << (IsValid ? "true" : "false") << "\n";
    OS << "--------------\n";
  }
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

private:
  // The global variable that is a possible candidate
  GlobalVariable *GV;

  // Number of dimensions (Fortran Rank) for the array
  uint32_t ArrayRank;

  // Number of elements in each dimension of the array. (Candidates must have
  // the same length in all dimensions)
  uint64_t ArrayLength;

  // Size of one element in the array, in bytes.
  uint64_t ElementSize;

  // This vector stores the stride values used when operating on the complete
  // array. For this optimization, we do not support cases where a sub-object is
  // passed to a function as a portion of the array.
  SmallVector<uint64_t, FortranMaxRank> Strides;

  // This vector stores the transpose index that will be used to access the
  // stride for a particular rank. For example, the regular layout of an array
  // that accesses 'block[i][j][k]', uses 'i' for the Rank 2 element, 'j' for
  // the Rank 1 element, and 'k' for the Rank 0 element, which would be
  // represented as accessing elements 0, 1, and 2 from the 'Strides' array.
  // Transposing the strides for the i and k elements would correspond to this
  // index lookup array being {2, 1, 0}
  SmallVector<uint32_t, FortranMaxRank> Transposition;

  // Indicates whether the analysis determined the candidate is safe to
  // transpose.
  bool IsValid;

  // This function will swap the strides used for indexing into the array. These
  // need to be changed for subscript operators that directly index into the
  // global variable, and for the setup of the dope vectors used when passing
  // the global variable to another function.
  void transposeStrides() {
    // TODO: transformation of uses goes here.
  }
};

//
// The array stride transpose optimization for Fortran.
//
// This optimization swaps the stride values used for multi-dimensional Fortran
// arrays to improve cache utilization or enable loop unrolling by having unit
// stride memory access patterns.
//
// For example, the default memory layout for the Fortran array declared as
// "integer block(3,3)" is stored in column-major order resulting in the access
// to block(i,j) being computed as:
//     &block + j * 3 * sizeof(integer) + i * sizeof(integer)
//
// For a loop iterating along 'j', transposing the strides may enable downstream
// optimizations so that iterations along 'j' will be a unit stride.
//
// This class will heuristically estimate the benefit and swap the stride values
// when beneficial.
class TransposeImpl {
public:
  TransposeImpl(std::function<LoopInfo &(Function &)> &GetLI) : GetLI(GetLI) {}

  bool run(Module &M) {
    IdentifyCandidates(M);

    bool ValidCandidate = false;
    for (auto &Cand : Candidates) {
      ValidCandidate |= Cand.analyze();

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
      if (PrintCandidates)
        Cand.dump();
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    }

    bool Changed = false;
    if (ValidCandidate)
      for (auto &Cand : Candidates)
        Changed |= Cand.transform();

    return Changed;
  }

private:
  std::function<LoopInfo &(Function &)> &GetLI;

  // Global variable candidates for the transformation.
  SmallVector<TransposeCandidate, 8> Candidates;

  // Identify potential candidates for the transpose optimization.
  //
  // The initial set of candidates meet the following criteria:
  // - Global Variable with internal linkage
  // - Multi-dimensional array of integer type
  // - The array lengths in all dimensions are equal
  // - Variable uses zero initializer
  void IdentifyCandidates(Module &M) {
    const DataLayout &DL = M.getDataLayout();

    for (auto &GV : M.globals()) {
      if (!GV.hasInitializer() || !GV.getInitializer()->isZeroValue())
        continue;

      // All uses of the variable need to be analyzed, therefore we need
      // internal linkage.
      if (!GV.hasInternalLinkage())
        continue;

      // All global variables are pointers
      llvm::Type *Ty = GV.getType()->getPointerElementType();
      auto *ArrType = dyn_cast<llvm::ArrayType>(Ty);
      if (!ArrType)
        continue;

      uint32_t Dimensions = 1;
      bool AllSame = true;
      uint64_t Length = ArrType->getArrayNumElements();
      llvm::Type *ElemType = ArrType->getArrayElementType();
      while (ElemType->isArrayTy()) {
        auto *InnerArrType = cast<llvm::ArrayType>(ElemType);
        if (InnerArrType->getArrayNumElements() != Length) {
          AllSame = false;
          break;
        }
        Dimensions++;
        ElemType = InnerArrType->getArrayElementType();
      }

      if (AllSame && Dimensions > 1 && Dimensions <= FortranMaxRank &&
          ElemType->isIntegerTy()) {
        LLVM_DEBUG(dbgs() << "Adding candidate: " << GV << "\n");
        uint64_t ElemSize = DL.getTypeStoreSize(ElemType);
        TransposeCandidate Candidate(&GV, Dimensions, Length, ElemSize);
        Candidates.push_back(Candidate);
      }
    }
  }
};

// Legacy pass manager wrapper for invoking the Transpose pass.
class DTransTransposeWrapper : public ModulePass {
private:
  dtrans::TransposePass Impl;

public:
  static char ID;
  DTransTransposeWrapper() : ModulePass(ID) {
    initializeDTransTransposeWrapperPass(*PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;

    dtrans::LoopInfoFuncType GetLI = [this](Function &F) -> LoopInfo & {
      return this->getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();
    };

    return Impl.runImpl(M, GetLI);
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    // Note, this transformation is not dependent on Whole Program Analysis.
    // The only candidates that may be selected for the transformation will
    // have internal linkage, and the analysis will be verifying all uses of
    // the candidate, which will ensure that the candidate is not escaped to an
    // external routine.

    AU.addRequired<LoopInfoWrapperPass>();

    // The swapping of the stride values in the dope vectors and
    // llvm.intel.subscript intrinsic call should not invalidate any analysis.
    AU.setPreservesAll();
  }
};

} // end anonymous namespace

char DTransTransposeWrapper::ID = 0;
INITIALIZE_PASS_BEGIN(DTransTransposeWrapper, "dtrans-transpose",
                      "DTrans multi-dimensional array transpose for Fortran",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_END(DTransTransposeWrapper, "dtrans-transpose",
                    "DTrans multi-dimensional array transpose for Fortran",
                    false, false)

ModulePass *llvm::createDTransTransposeWrapperPass() {
  return new DTransTransposeWrapper();
}

namespace llvm {

namespace dtrans {

PreservedAnalyses TransposePass::run(Module &M, ModuleAnalysisManager &AM) {
  FunctionAnalysisManager &FAM =
      AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();

  LoopInfoFuncType GetLI = [&FAM](Function &F) -> LoopInfo & {
    return FAM.getResult<LoopAnalysis>(F);
  };

  runImpl(M, GetLI);

  // The swapping of the stride values in the dope vectors and
  // llvm.intel.subscript intrinsic call should not invalidate any analysis.
  return PreservedAnalyses::all();
}

bool TransposePass::runImpl(Module &M,
                            std::function<LoopInfo &(Function &)> &GetLI) {
  TransposeImpl Transpose(GetLI);
  return Transpose.run(M);
}

} // end namespace dtrans
} // end namespace llvm
