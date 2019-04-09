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

// Trace messages regarding the analysis of the candidate variables.
#define DEBUG_ANALYSIS "dtrans-transpose-analysis"

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// Print the list of candidates identified and their analysis result.
static cl::opt<bool> PrintCandidates("dtrans-transpose-print-candidates",
                                     cl::ReallyHidden);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

namespace {

// Helper routine to check if a CallInst is to llvm.intel.subscript.
bool isSubscriptIntrinsicCall(const CallInst &CI) {
  const Function *F = CI.getCalledFunction();
  return F && F->getIntrinsicID() == Intrinsic::intel_subscript;
}
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
  //
  // The only valid uses for the global variable itself are:
  // - Base pointer argument in outermost call of a llvm.intel.subscript
  //   intrinsic call chain.
  // - Storing the array's address into a dope vector that represents the
  //   entire array object using the default values for the lower bound/
  //   extent/stride.
  // - The dope vector object may be passed to a function that takes an
  //   assumed shape array. The called function will be checked that there
  //   are only reads of the dope vector structure elements, or the transfer
  //   of the dope vector pointer to an uplevel variable.
  // - The uplevel variable can be passed to a function, and again all uses
  //   of the dope vector fields will be checked to verify that only reads
  //   are done on the dope vector elements.
  //
  bool analyze() {
    DEBUG_WITH_TYPE(DEBUG_ANALYSIS,
                    dbgs() << "Analyzing variable: " << *GV << "\n");

    // Check all the direct uses of the global. This loop will also collect
    // the functions that take a dope vector which need to be checked.
    IsValid = true;
    for (auto *U : GV->users()) {
      DEBUG_WITH_TYPE(DEBUG_ANALYSIS,
                      dbgs() << "Checking global var use: " << *U << "\n");

      // Uses of the global should be in the form of a GEP operator which should
      // only be getting the base address of the array. For example:
      //   i32* getelementptr ([9 x [9 x i32]],
      //                       [9 x [9 x i32]]* @var1, i64 0, i64 0, i64 0)

      auto *GepOp = dyn_cast<GEPOperator>(U);
      if (!GepOp) {
        DEBUG_WITH_TYPE(DEBUG_ANALYSIS,
                        dbgs() << "  Invalid: Unsupported instruction: " << *U
                               << "\n");
        IsValid = false;
        break;
      }

      if (!GepOp->hasAllZeroIndices()) {
        DEBUG_WITH_TYPE(
            DEBUG_ANALYSIS,
            dbgs() << "  Invalid: Global variable GEP not getting base "
                      "pointer address\n");
        IsValid = false;
        break;
      }

      // Now check the users of the pointer address for safety
      for (auto *GepOpUser : GepOp->users()) {
        DEBUG_WITH_TYPE(DEBUG_ANALYSIS, {
          dbgs() << "  Checking global var address use: " << *GepOpUser << "\n";
          if (auto *I = dyn_cast<Instruction>(GepOpUser))
            dbgs() << "  in function: "
                   << I->getParent()->getParent()->getName() << "\n";
        });

        if (auto *CI = dyn_cast<CallInst>(GepOpUser)) {
          // Check that the call is to llvm.intel.subscript.
          //
          // This could be extended in the future to allow the address to be
          // passed without a dope vector, but that is not needed for the case
          // of interest, at the moment.
          if (!isSubscriptIntrinsicCall(*CI)) {
            DEBUG_WITH_TYPE(
                DEBUG_ANALYSIS,
                dbgs() << "  Invalid: Call with pointer address may only be "
                          "subscript intrinsic call\n");

            IsValid = false;
            break;
          }

          // The global variable should only be accessed with a subscript call
          // that uses the rank of the variable, and the array should only be
          // using default values for the lower bound and stride, rather than a
          // user defined value for the lower bound. It should not be required
          // for the transform, but it avoids cases such as:
          //     integer :: my_array(2:10, 9, 11:19)
          if (!isValidUseOfSubscriptForGlobal(*CI, *GepOp)) {
            DEBUG_WITH_TYPE(
                DEBUG_ANALYSIS,
                dbgs() << "  Invalid: Subscript call values not supported\n");

            IsValid = false;
            break;
          }

          // Save the subscript call because we will need this for computing
          // profitability and transforming the arguments later.
          SubscriptCalls.insert(CI);
        } else if (auto *SI = dyn_cast<StoreInst>(GepOpUser)) {
          // The only case the address of the variable may be saved is into a
          // dope vector, check that case here.
          if (!isValidStoreForGlobal(*SI, GepOp)) {
            DEBUG_WITH_TYPE(
                DEBUG_ANALYSIS,
                dbgs()
                    << "  Invalid: Store of pointer address not supported\n");

            IsValid = false;
            break;
          }
        } else {
          // Other uses are not allowed.
          DEBUG_WITH_TYPE(DEBUG_ANALYSIS,
                          dbgs() << "Unsupported use of global: " << *GepOpUser
                                 << "\n");
          IsValid = false;
          break;
        }
      }
    }

    // TODO: Analysis of the dope vectors passed to functions will go here.

    if (!IsValid)
      SubscriptCalls.clear();

    return IsValid;
  }

  // Check that \p CI is a supported subscript call on the global array base
  // address \p BasePtr. For a global variable, we expect the subscript call to
  // contain the constant values for the lower bound and stride that represent
  // the full array, and a lower bound index of 1.
  bool isValidUseOfSubscriptForGlobal(const CallInst &CI,
                                      const Value &BasePtr) {

    // Helper that checks constants for one subscript call, and recurse if
    // there are more ranks to check.
    std::function<bool(const CallInst &, const Value &, uint32_t)>

        IsValidUseForRank = [this, &IsValidUseForRank](const CallInst &Call,
                                                       const Value &Ptr,
                                                       uint32_t Rank) -> bool {
      if (!isValidUseOfSubscriptCall(Call, Ptr, Rank, 1, Strides[Rank]))
        return false;

      // Verify the subscript result is only fed to another subscript call. In
      // the future this could be extended to support PHI nodes/select
      // instructions, but for now that is not needed.
      if (Rank > 0) {
        for (const auto *U : Call.users()) {
          const auto *CI = dyn_cast<CallInst>(U);
          if (!CI)
            return false;

          if (!IsValidUseForRank(*CI, Call, Rank - 1))
            return false;
        }
      }
      return true;
    };

    // Check the use of this subscript call, and all the subscript calls the
    // result is fed to. Note, subscript call rank parameter value starts at 0,
    // not 1.
    return IsValidUseForRank(CI, BasePtr, ArrayRank - 1);
  }

  // Check for arguments of a subscript intrinsic call for the expected values.
  // The intrinsic call is declared as:
  //    declare <ty>* @llvm.intel.subscript...(i8 <rank>, <ty> <lb>,
  //                                           <ty> <stride>, <ty>* <base>,
  //                                           <ty> <index>)
  //
  // Return 'true' if call has the expected values for the Base, Rank,
  // LowerBound and Stride parameters.
  //
  bool isValidUseOfSubscriptCall(const CallInst &CI, const Value &Base,
                                 uint32_t Rank, uint64_t LowerBound,
                                 uint64_t Stride) {
    const unsigned RankOpNum = 0;
    const unsigned LBOpNum = 1;
    const unsigned StrideOpNum = 2;
    const unsigned PtrOpNum = 3;

    DEBUG_WITH_TYPE(DEBUG_ANALYSIS, {
      dbgs().indent((ArrayRank - Rank) * 2 + 4);
      dbgs() << "Checking call: " << CI << "\n";
    });

    if (!isSubscriptIntrinsicCall(CI))
      return false;

    if (CI.getArgOperand(PtrOpNum) != &Base)
      return false;

    auto RankVal = dyn_cast<ConstantInt>(CI.getArgOperand(RankOpNum));
    if (!RankVal || RankVal->getLimitedValue() != Rank)
      return false;

    auto LBVal = dyn_cast<ConstantInt>(CI.getArgOperand(LBOpNum));
    if (!LBVal || LBVal->getLimitedValue() != LowerBound)
      return false;

    auto StrideVal = dyn_cast<ConstantInt>(CI.getArgOperand(StrideOpNum));
    if (!StrideVal || StrideVal->getLimitedValue() != Stride)
      return false;

    return true;
  }

  // The only supported use of storing the address of the array's base pointer
  // into another memory location is when the address is being stored into a
  // dope vector, and the dope vector is describing the entire array (Lower
  // Bound = 1, Extent = array length, and Stride is each element for each array
  // dimension).
  bool isValidStoreForGlobal(const StoreInst &SI, const Value *BasePtr) {
    // TODO: Implement checks for storing into a dope vector
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

  // Set of calls to the subscript intrinsic that directly access the array
  // address. These have the highest 'rank' value for the subscript calls. The
  // result of this instruction is fed to the subscript call of the next lower
  // rank, so we only need to store the initial call to get to all the others
  // for computing profitability and transposing the stride values.
  SmallPtrSet<CallInst *, 16> SubscriptCalls;

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

      // TODO: Analyze the candidate for profitability

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
