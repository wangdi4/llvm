//===--------------- Transpose.cpp - DTransTransposePass------------------===//
//
// Copyright (C) 2019-2022 Intel Corporation. All rights reserved.
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
#include "Intel_DTrans/Analysis/DTransUtils.h"
#include "Intel_DTrans/DTransCommon.h"

#include "llvm/ADT/Optional.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Analysis/Intel_DopeVectorAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include <cmath>
#include <numeric>

using namespace llvm;

using namespace dvanalysis;

using namespace llvm::PatternMatch;

#define DEBUG_TYPE "dtrans-transpose"

// Trace messages about the analysis of the IR for transposing
#define DEBUG_ANALYSIS "dtrans-transpose-analysis"

// Trace messages about the dope vector object analysis
#define DEBUG_DOPE_VECTORS "dtrans-transpose-dopevectors"

// Trace messages about the IR transformation
#define DEBUG_TRANSFORM "dtrans-transpose-transform"

// Trace messages about the IR profitability
#define DEBUG_PROFITABILITY "dtrans-transpose-profitability"

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// Print the list of candidates identified and their analysis result.
static cl::opt<bool> PrintCandidates("dtrans-transpose-print-candidates",
                                     cl::ReallyHidden);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// Command line option to override the profitability heuristics, and enable a
// specific transpose transformation to occur, subject to the variable passing
// the safety checks.
//
// The argument format is: <varname>,Dim(N-1)Index,...,Dim(1)Index,Dim(0)Index
//
// There is one integer index argument for each dimension of the array, which
// represents the dimension to take the stride from when doing the transpose.
//
// Additional variables can be supplied using a separator of ';'.
//
// For a 3 dimensional array, the default ordering corresponds to "2,1,0", which
// corresponds to subscript calls of the form:
//   %t1 = call @llvm.intel.subscript(2, 1, 324, @block, %idx2) ; stride = 324
//   %t2 = call @llvm.intel.subscript(1, 1, 36, %t1, %idx1) ; stride = 36
//   %t3 = call @llvm.intel.subscript(0, 1, 4, %t2, %idx0) ; stride = 4
//
// This option can be used to modify the ordering as follows:
//     -dtrans-transpose-override=block,0,1,2;tetra,2,0,1
// This would transpose the 1st and 3rd strides for 'block' (give dimension 2
// the stride value of dimension 0, and vice-versa) and transpose the 2nd and
// 3rd strides for 'tetra'
static cl::opt<std::string> TransposeOverride("dtrans-transpose-override",
                                              cl::ReallyHidden);
// We will not perform the transpose unless each dimension of the array has
// at least this number of elements.
static cl::opt<uint64_t> TransposeMinDim("dtrans-transpose-min-dim",
                                         cl::init(8), cl::ReallyHidden);

// If the percentage of references that are indirectly indexed on a dimension
// is greater than this, this dimension will get a 0.0 gain in the transpose
// profitability computation.
static cl::opt<uint64_t> TransposeMinIIRatio("dtrans-transpose-min-ii-ratio",
                                             cl::init(10), cl::ReallyHidden);

namespace {

// This is the class that manages the analysis and transformation
// of the stride information for a candidate variable.
class TransposeCandidate {
public:
  TransposeCandidate(GlobalVariable *GV, uint32_t ArrayRank,
                     SmallVector<uint64_t, 4> &ArrayLength,
                     uint64_t ElementSize, llvm::Type *ElementType,
                     dtrans::TransposeTLIType GetTLI,
                     DopeVectorInfo *DVI = nullptr,
                     Optional<uint64_t> NestedFieldNum = None)
      : GV(GV), ArrayRank(ArrayRank), ArrayLength(ArrayLength),
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
        ElementSize(ElementSize),
#endif
        ElementType(ElementType), GetTLI(GetTLI), IsGlobalDV(DVI),
        NestedFieldNumber(NestedFieldNum), IsValid(false), IsProfitable(false) {
    assert(ArrayRank > 0 && ArrayRank <= FortranMaxRank && "Invalid Rank");
    uint64_t Stride = ElementSize;
    for (uint32_t RankNum = 0; RankNum < ArrayRank; ++RankNum) {
      Strides.push_back(Stride);
      Stride *= ArrayLength[RankNum];
    }
    if (DVI) {
      DVI->identifyPtrAddrSubs(SubscriptCalls);
      DVI->identifyStrideStores(StrideStores);
    } else {
      SubscriptCalls.clear();
      for (unsigned I = 0; I < ArrayRank; ++I)
        StrideStores[I].clear();
    }
    DVSubscriptCalls.clear();
  }

  ~TransposeCandidate() { cleanup(); }

  // getters and setters for class.

  StringRef getName() const { return GV->getName(); }

  Optional<uint64_t> getNestedFieldNumber() const {
    return NestedFieldNumber;
  }

  uint32_t getArrayRank() const { return ArrayRank; }
  void setIsProfitable(bool Val) { IsProfitable = Val; }
  void setTransposition(ArrayRef<uint32_t> A) {
    assert(A.size() == ArrayRank && "Invalid rank description");
    std::copy(A.begin(), A.end(), std::back_inserter(Transposition));
  }

  // Clean up memory allocated during analysis of the candidate.
  void cleanup() {
    for (auto *DVA : DopeVectorInstances)
      delete DVA;

    DopeVectorInstances.clear();
    SubscriptCalls.clear();
    DVSubscriptCalls.clear();
  }

  // This function analyzes a candidate to check whether all uses of the
  // variable are supported for the transformation. Either an array which
  // is a global variable or an allocatable array represented by a dope
  // vector can be analyzed.
  //
  bool analyze(const DataLayout &DL) {
    if (IsGlobalDV) {
      // The actual analysis was done during dope vector analysis.
      // Nothing else to do here.
      IsValid = true;
      return true;
    }
    return analyzeGlobalVar(DL);
  }

  // Analyze a global variable array for transpose.
  // The only valid uses for the global variable are:
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
  bool analyzeGlobalVar(const DataLayout &DL) {

    using VecPair = SmallVector<std::pair<Value *, User *>, 8>;

    // Collect pairs of base values and users for analysis.
    // Skip through GEP(X,0,0)s as they may not be present for
    // opaque pointers.
    auto CollectPairs = [](GlobalVariable *GV,
                           VecPair &Pairs) -> bool {
      for (auto *U : GV->users()) {
        if (auto *GepOp = dyn_cast<GEPOperator>(U)) {
          if (GepOp->hasAllZeroIndices()) {
            for (auto *UU : GepOp->users())
              Pairs.push_back({GepOp, UU});
          } else {
            return false;
          }
        } else {
          Pairs.push_back({GV, U});
        }
      }
      return true;
    };

    DEBUG_WITH_TYPE(DEBUG_ANALYSIS,
                    dbgs() << "\nAnalyzing variable: " << *GV << "\n");

    IsValid = true;
    VecPair Pairs;
    // Collect up the uses GV, skipping past GEP(X,0,0) cases. This loop will
    // also collect the functions that take a dope vector which need to be
    // checked.
    if (!CollectPairs(GV, Pairs)) {
      IsValid = false;
    } else {
      for (const auto &Pair : Pairs) {
        Value *Base = Pair.first;
        User *U = Pair.second;
        // Now check the users of the pointer address for safety
        DEBUG_WITH_TYPE(DEBUG_ANALYSIS, {
          dbgs() << "  Checking global var address use: " << *U << "\n";
          if (auto *I = dyn_cast<Instruction>(U))
            dbgs() << "  in function: "
                   << I->getParent()->getParent()->getName() << "\n";
        });

        // Check that the call is to llvm.intel.subscript.
        //
        // This could be extended in the future to allow other CallInst that
        // take the address without a dope vector, but that is not needed for
        // the case of interest, at the moment.
        if (auto *Subs = dyn_cast<SubscriptInst>(U)) {
          // The global variable should only be accessed with a subscript call
          // that uses the rank of the variable, and the array should only be
          // using default values for the lower bound and stride, rather than a
          // user defined value for the lower bound. It should not be required
          // for the transform, but it avoids cases such as:
          //     integer :: my_array(2:10, 9, 11:19)
          if (!isValidUseOfSubscriptForGlobal(*Subs, *Base)) {
            DEBUG_WITH_TYPE(
                DEBUG_ANALYSIS,
                dbgs() << "  Invalid: Subscript call values not supported\n");

            IsValid = false;
            break;
          }

          // Save the subscript call because we will need this for computing
          // profitability and transforming the arguments later.
          SubscriptCalls.insert(Subs);
        } else if (auto *SI = dyn_cast<StoreInst>(U)) {
          // The only case the address of the variable may be saved is into a
          // dope vector, check that case here.
          if (!isValidStoreForGlobal(*SI, Base, DL)) {
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
                          dbgs() << "Unsupported use of global: " << *U
                                 << "\n");
          IsValid = false;
          break;
        }
      }
    }

    if (IsValid) {
      // Analyze all the functions that the dope vector was passed to. Collate
      // them to a single set in case the function was called multiple times.
      FuncArgPosPairSet FuncsWithDopeVector;
      for (DopeVectorAnalyzer *DVA : DopeVectorInstances) {
        auto Range = DVA->funcsWithDVParam();
        FuncsWithDopeVector.insert(Range.begin(), Range.end());
        for (unsigned I = 0; I < ArrayRank; ++I)
          StrideStores[I].insert(DVA->getStrideStores(I).begin(),
              DVA->getStrideStores(I).end());
      }

      for (auto &FuncPos : FuncsWithDopeVector)
        if (!analyzeDopeVectorCallArgument(*FuncPos.first, FuncPos.second)) {
          IsValid = false;
          break;
        }
    }

    LLVM_DEBUG(dbgs() << "Candidate " << (IsValid ? "PASSED" : "FAILED")
                      << " safety tests: " << GV->getName() << "\n");

    if (!IsValid)
      cleanup();

    return IsValid;
  }

  // Check that \p Subs is a supported subscript call on the global array base
  // address \p BasePtr. For a global variable, we expect the subscript call to
  // contain the constant values for the lower bound and stride that represent
  // the full array, and a lower bound index of 1.
  bool isValidUseOfSubscriptForGlobal(const SubscriptInst &Subs,
                                      const Value &BasePtr) {

    // Helper that checks constants for one subscript call, and recurse if
    // there are more ranks to check.
    std::function<bool(const SubscriptInst &, const Value &, uint32_t)>

        IsValidUseForRank = [this, &IsValidUseForRank](
                                const SubscriptInst &SubsPtr, const Value &Ptr,
                                uint32_t Rank) -> bool {
      if (!isValidUseOfSubscriptCall(SubsPtr, Ptr, ArrayRank, Rank, true, 1,
                                     Strides[Rank]))
        return false;

      // Verify the subscript result is only fed to another subscript call. In
      // the future this could be extended to support PHI nodes/select
      // instructions, but for now that is not needed.
      if (Rank > 0) {
        for (const auto *U : SubsPtr.users()) {
          const auto *Subs2 = dyn_cast<SubscriptInst>(U);
          if (!Subs2)
            return false;

          if (!IsValidUseForRank(*Subs2, SubsPtr, Rank - 1))
            return false;
        }
      }
      return true;
    };

    // Check the use of this subscript call, and all the subscript calls the
    // result is fed to. Note, subscript call rank parameter value starts at 0,
    // not 1.
    return IsValidUseForRank(Subs, BasePtr, ArrayRank - 1);
  }

  // The only supported use of storing the address of the array's base pointer
  // into another memory location is when the address is being stored into a
  // dope vector, and the dope vector is describing the entire array (Lower
  // Bound = 1, Extent = array length, and Stride is each element for each array
  // dimension).
  bool isValidStoreForGlobal(StoreInst &SI, const Value *BasePtr,
                             const DataLayout &DL) {
    if (SI.getValueOperand() != BasePtr)
      return false;

    Value *DVObject = isPotentialDVStore(SI, DL);
    if (!DVObject)
      return false;

    // Collect the use of the dope vector pointer.
    std::unique_ptr<DopeVectorAnalyzer> DVA(
        new DopeVectorAnalyzer(DVObject, nullptr, GetTLI));
    DVA->analyze(/*ForCreation = */ true);
    DEBUG_WITH_TYPE(DEBUG_DOPE_VECTORS, {
      dbgs() << "Analysis of potential dope vector:\n";
      DVA->dump();
      dbgs() << "\n";
    });

    if (!DVA->getIsValid()) {
      DEBUG_WITH_TYPE(DEBUG_ANALYSIS,
                      dbgs() << "Invalid: Unsupported dope vector\n");
      return false;
    }

    // Check that the only write of the pointer field is to store the address
    // we expect for the array object.
    if (DVA->getPtrAddrField().getSingleValue() != BasePtr)
      return false;

    // Check that the dope vector is set up using the lower bound, stride and
    // extent that represents the complete object, and not a sub-object.
    auto MatchesConstant = [](Value *V, unsigned long Expect) {
      if (auto *C = dyn_cast<ConstantInt>(V))
        return C->getLimitedValue() == Expect;
      return false;
    };

    for (uint32_t Dim = 0; Dim < ArrayRank; ++Dim) {
      Value *LB = DVA->getLowerBound(Dim);
      Value *Extent = DVA->getExtent(Dim);
      Value *Stride = DVA->getStride(Dim);
      if (!LB || !Extent || !Stride) {
        DEBUG_WITH_TYPE(
            DEBUG_ANALYSIS,
            dbgs() << "Invalid: Unable to analyze dope vector fields\n");
        return false;
      }

      if (!MatchesConstant(LB, 1) ||
          !MatchesConstant(Extent, ArrayLength[Dim]) ||
          !MatchesConstant(Stride, Strides[Dim])) {
        DEBUG_WITH_TYPE(DEBUG_ANALYSIS,
                        dbgs() << "Invalid: DV does not capture entire "
                                  "array with unit strides\n");
        return false;
      }
    }

    // Save the dope vector info for analysis of the called functions, and
    // updates to the setup.
    DopeVectorInstances.insert(DVA.release());
    return true;
  }

  // Check whether the store of the variable is potentially to a dope vector
  // structure. Currently, the front-end does not add metadata tags to indicate
  // dope vectors, so we will pattern match this. (The later analysis on the
  // usage and limitations of usage will filter out any false positive matches.)
  //
  // For a store of the form:
  //   store i32* getelementptr inbounds(
  //        [9 x[9 x[9 x i32]]], [9 x[9 x[9 x i32]]] * @block,
  //           i64 0, i64 0, i64 0, i64 0),
  //        i32** %ptr, align 8
  //
  // Look for the pointer operand of the form:
  //     %ptr = getelementptr inbounds
  //          { i32*, i64, i64, i64, i64, i64, [N x { i64, i64, i64 }] },
  //          { i32*, i64, i64, i64, i64, i64, [N x { i64, i64, i64 }] }*
  //            %object, i64 0, i32 0
  //
  //  where the type for %ptr matches a dope vector type, %object is a locally
  //  allocated object.
  //
  Value *isPotentialDVStore(StoreInst &SI, const DataLayout &DL) {
    // Check that the store is a field that matches a dope vector type
    Value *Ptr = SI.getPointerOperand();
    auto *FieldGEP = dyn_cast<GetElementPtrInst>(Ptr);
    if (!FieldGEP)
      return nullptr;

    llvm::Type *GEPType = FieldGEP->getSourceElementType();
    if (!isDopeVectorType(GEPType, DL))
      return nullptr;

    // Check that the field address is where we expect the array address to be
    // stored within the dope vector.
    if (DopeVectorAnalyzer::identifyDopeVectorField(*(cast<GEPOperator>(FieldGEP))) !=
        DopeVectorFieldType::DV_ArrayPtr)
      return nullptr;

    Value *DVObject = FieldGEP->getPointerOperand();
    if (!isa<AllocaInst>(DVObject))
      return nullptr;

    return DVObject;
  }

  // Check if the type Ty looks like a dope vector type which matches the
  // candidate variable.
  bool isDopeVectorType(const llvm::Type *Ty, const DataLayout &DL) {
    uint32_t ArRank;
    Type *ElemType;
    return llvm::dvanalysis::isDopeVectorType(Ty, DL, &ArRank, &ElemType)
        && ArRank == ArrayRank && (!ElemType || ElemType == ElementType);
  }

  // A dope vector passed to a function is allowed to have the following uses:
  // - Load the fields of the dope vector object. (No field writes allowed).
  // - The loaded fields are also checked to be sure the array does not escape
  //   and the stride value used for the accesses comes from the dope vector.
  // - Store the address of the dope vector into an uplevel variable, and
  //   pass the uplevel variable to another function.
  bool analyzeDopeVectorCallArgument(Function &F, unsigned int ArgPos) {
    DEBUG_WITH_TYPE(DEBUG_ANALYSIS,
                    dbgs() << "  Checking use of dope vector in function: "
                           << F.getName() << " Arg: " << ArgPos << "\n");
    if (F.isDeclaration()) {
      DEBUG_WITH_TYPE(DEBUG_ANALYSIS,
                      dbgs() << "IR not available for function: " << F.getName()
                             << "\n");
      return false;
    }

    assert(ArgPos < F.arg_size() && "Invalid argument position");
    auto Args = F.arg_begin();
    std::advance(Args, ArgPos);
    Argument *FormalArg = &(*Args);

    DopeVectorAnalyzer DVA(FormalArg, nullptr, GetTLI);
    DVA.analyze(/*ForCreation = */ false);
    if (!DVA.analyzeDopeVectorUseInFunction(F, &DVSubscriptCalls))
      return false;
    return true;
  }

  // This function decides whether the candidate should have any of the
  // strides transposed based on the loop depth that the individual dimensions
  // are accessed at.
  //
  // The heuristics rely on loop invariant code motion to have already
  // moved values that are invariant out of loops, so that we can tell which
  // loop level is varying the dimension's 'index' component of the subscript
  // call.
  void computeProfitability(dtrans::TransposeLoopInfoFuncType GetLI) {
    // Gain factor that a dimension with a high stride value must exceed the
    // gain value of a low stride value by to be considered worth performing the
    // transpose.
    const unsigned TransposeMinGainFactor = 100;

    // Lambda to estimate the loop trip count for simple counted loops.
    // These are loops with a single back edge, where a counter variable
    // starts with a constant value, is incremented by a constant
    // value inside the loop, and compared against a constant for the loop
    // backedge. If a trip count can be determined, return it, otherwise
    // return 0.
    auto EstimateLoopTripCount = [](Loop *L) -> uint64_t {
      if (!L)
        return 0;

      BasicBlock *Latch = L->getLoopLatch();
      if (!Latch)
        return 0;

      if (auto BrInst = dyn_cast<BranchInst>(Latch->getTerminator())) {
        if (BrInst->isConditional()) {
          Value *Cond = BrInst->getCondition();

          ICmpInst::Predicate Pred;
          Instruction *PstIncr;
          const APInt *Limit;
          if (!match(Cond,
                     m_ICmp(Pred, m_Instruction(PstIncr), m_APInt(Limit))))
            return 0;

          // For now, only consider counting up to some value. This could be
          // enhanced for more cases in the future. For testing for equality, we
          // expect the loop to continue while the condition is false. For less
          // than, it should be the 'true' path.
          unsigned BackedgeSuccNum;
          if (Pred == ICmpInst::ICMP_EQ)
            BackedgeSuccNum = 1;
          else if (Pred == ICmpInst::ICMP_ULT || Pred == ICmpInst::ICMP_SLT)
            BackedgeSuccNum = 0;
          else
            return 0;

          if (BrInst->getSuccessor(BackedgeSuccNum) != L->getHeader())
            return 0;

          Instruction *PreIncr;
          const APInt *Slope;
          if (!match(PstIncr, m_Add(m_Instruction(PreIncr), m_APInt(Slope))))
            return 0;

          if (auto *Phi = dyn_cast<PHINode>(PreIncr)) {
            if (Phi->getNumIncomingValues() == 2) {
              // expect the phi to be:
              //   PreIncr = phi [PstIncr, Label2], [const, Label1]
              if (Phi->getIncomingValue(0) != PstIncr &&
                  Phi->getIncomingValue(1) != PstIncr)
                return 0;

              ConstantInt *CI = dyn_cast<ConstantInt>(Phi->getIncomingValue(0));
              if (!CI)
                CI = dyn_cast<ConstantInt>(Phi->getIncomingValue(1));
              if (CI) {
                // Ignore negative numbers to make things simple.
                if (CI->isNegative() || Slope->isNegative())
                  return 0;

                return Limit->getLimitedValue() -
                  CI->getLimitedValue() / Slope->getLimitedValue();
              }
            }
          }
        }
      }

      return 0;
    };

    // Return 'true' if 'Inst' represents a PHINode which is the index of
    // SubscriptInst potentially offset by a constant and then optionally sign or
    // zero extended. Such a pattern is indicative of indexing into an array.
    auto IsIndirectIndex = [](Instruction *Inst) -> bool {
      if (!Inst)
        return false;
      auto CI = dyn_cast<CastInst>(Inst);
      if (CI && (isa<SExtInst>(CI) || isa<ZExtInst>(CI))) {
        Inst = dyn_cast<Instruction>(CI->getOperand(0));
        if (!Inst)
          return false;
      }
      Value *V = nullptr;
      ConstantInt *C = nullptr;
      if (match(Inst, m_Add(m_Value(V), m_ConstantInt(C))) ||
          match(Inst, m_Add(m_ConstantInt(C), m_Value(V))) ||
          match(Inst, m_Sub(m_Value(V), m_ConstantInt(C))) ||
          match(Inst, m_Sub(m_ConstantInt(C), m_Value(V)))) {
        Inst = dyn_cast<Instruction>(V);
        if (!Inst)
          return false;
      }
      auto LI = dyn_cast<LoadInst>(Inst);
      if (!LI)
        return false;
      auto SI = dyn_cast<SubscriptInst>(LI->getPointerOperand());
      if (!SI)
        return false;
      while (auto W = dyn_cast<SubscriptInst>(SI->getPointerOperand()))
        SI = W;
      return isa<PHINode>(SI->getIndex());
    };

    // This lambda function will recurse down the subscript intrinsic call
    // chain, accumulating a gain value for each dimension which will be used to
    // determine which dimension should be given the smallest stride.
    std::function<void(Instruction *, LoopInfo &,
                       std::array<Instruction *, FortranMaxRank> &,
                       std::array<unsigned, FortranMaxRank> &,
                       std::array<unsigned, FortranMaxRank> &,
                       std::array<unsigned, FortranMaxRank> &,
                       std::array<double, FortranMaxRank> &,
                       SmallPtrSetImpl<Instruction *> &)>
        ComputeGain;
    ComputeGain = [this, &ComputeGain, &EstimateLoopTripCount,
                      &IsIndirectIndex](
                      Instruction *I, LoopInfo &LI,
                      std::array<Instruction *, FortranMaxRank> &IndexChain,
                      std::array<unsigned, FortranMaxRank> &IndexVarianceDepth,
                      std::array<unsigned, FortranMaxRank> &IICount,
                      std::array<unsigned, FortranMaxRank> &NoIICount,
                      std::array<double, FortranMaxRank> &Gains,
                      SmallPtrSetImpl<Instruction *> &Visited) {
      if (!Visited.insert(I).second)
        return;

      // We want the transpose to enable processing of elements in the loop to
      // enable unit-stride accesses between successive elements to allow for
      // vectorization. This value sets a minimum estimated trip count that we
      // want in order to treat a loop as being worthwhile to have unit strides.
      const unsigned VectorMinForGain = 8;

      if (isa<SelectInst>(I) || isa<PHINode>(I)) {
        for (auto *UU : I->users())
          if (auto *I2 = dyn_cast<Instruction>(UU))
            ComputeGain(I2, LI, IndexChain, IndexVarianceDepth,
                        IICount, NoIICount, Gains, Visited);
        return;
      }

      auto *Subs = dyn_cast<SubscriptInst>(I);
      if (!Subs) {
        return;
      }

      unsigned Dim = Subs->getRank();
      DEBUG_WITH_TYPE(DEBUG_PROFITABILITY, {
        dbgs().indent((ArrayRank - Dim) * 2);
        dbgs() << *Subs << "\n";
      });

      Value *Index = Subs->getIndex();
      Instruction *IndexInst = dyn_cast<Instruction>(Index);
      if (IsIndirectIndex(IndexInst))
        IICount[Dim]++;
      else
        NoIICount[Dim]++;

      unsigned IndexLoopDepth =
          IndexInst ? LI.getLoopDepth(IndexInst->getParent()) : 0;

      // Save the info about this dimension to be used when we reach the end of
      // the chain.
      IndexChain[Dim] = IndexInst;
      IndexVarianceDepth[Dim] = IndexLoopDepth;

      if (Dim != 0) {
        for (auto *UU : Subs->users())
          if (auto *I3 = dyn_cast<Instruction>(UU))
            ComputeGain(I3, LI, IndexChain, IndexVarianceDepth,
                        IICount, NoIICount, Gains, Visited);
      } else {
        // We are relying on the subscript calls being chained together
        // from highest dimension to lowest dimension, when we reach the
        // lowest dimension, this will be the loop that is used to access
        // the elements.
        unsigned VariantDim = 0;
        unsigned VariantDepth = 0;
        for (unsigned Idx = 0; Idx < ArrayRank; ++Idx)
          if (IndexVarianceDepth[Idx] > VariantDepth) {
            VariantDepth = IndexVarianceDepth[Idx];
            VariantDim = Idx;
          }

        // If an index value was varying in a loop, check to see if
        // there should be given a profitability gain.
        if (VariantDepth) {
          DEBUG_WITH_TYPE(DEBUG_PROFITABILITY,
                          dbgs() << "Deepest iteration index:" << VariantDim
                                 << "\n");

          auto BB = IndexChain[VariantDim]->getParent();
          Loop *L = LI.getLoopFor(BB);
          unsigned TC = EstimateLoopTripCount(L);

          // If a potential trip count could not be identified, estimate
          // the loop will process half of the array elements.
          if (TC == 0)
            TC = ArrayLength[VariantDim] / 2;

          // Only consider loops that appear to be good candidates for
          // unit-stride vectorization.
          if (TC >= VectorMinForGain) {
            // The unit-stride loop may be embedded within a loop that is
            // walking one of the other dimensions, so we need to apply an
            // appropriate gain level to the loops that led to this loop.
            for (unsigned Idx = 0; Idx < ArrayRank; ++Idx) {
              // Estimate a factor of 10 for each loop level based on the
              // variance depth of the dimension's index.
              double Gain = pow(10.0, IndexVarianceDepth[Idx]) *
                  Subs->getNumUses();
              // Saturate to avoid numeric overflow.
              double NewGain = Gains[Idx] + Gain;
              Gains[Idx] = NewGain > Gains[Idx]
                           ? NewGain
                           : std::numeric_limits<double>::max();
              DEBUG_WITH_TYPE(DEBUG_PROFITABILITY,
                              dbgs() << "  Gain for dimension "
                                     << Idx << ": " << Gain << "\n");
            }
          }
        }
      }
    };

    if (!IsValid)
      return;

    DEBUG_WITH_TYPE(DEBUG_PROFITABILITY,
                    dbgs() << "\nAnalyzing variable for profitability : " << *GV
                           << "\n");

    // Collect the subscript calls per function, so that loop info will only
    // need to be computed once per function.
    DenseMap<Function *, SmallVector<SubscriptInst *, 32>> FuncToSubsVec;
    for (auto *Subs : SubscriptCalls)
      FuncToSubsVec[Subs->getParent()->getParent()].push_back(Subs);
    for (auto *Subs : DVSubscriptCalls)
      FuncToSubsVec[Subs->getParent()->getParent()].push_back(Subs);

    // This will hold the profitability value for each dimension of the array.
    // Higher values will mean a dimension is more important to be unit-stride
    // when determining whether the array should be transposed.
    std::array<double, FortranMaxRank> Gains = {};

    // The subscript calls chain together from one dimension to another, and
    // each call indexes some element of that dimension. This holds the index
    // value for each dimension of the subscript chain.
    std::array<Instruction *, FortranMaxRank> IndexChain = {};

    // This holds the loop level that the index value used for the subscript
    // call is varying at. A value of 0, means the index does not vary within
    // any loop, whereas a value of 2 would mean the value is varying inside a
    // nested loop.
    std::array<unsigned, FortranMaxRank> IndexVarianceDepth = {};

    // Number of times a reference to a particular dimension was indirectly
    // indexed.
    std::array<unsigned, FortranMaxRank> IICount = {};

    // Number of times a reference to a particular dimension was directly
    // indexed.
    std::array<unsigned, FortranMaxRank> NoIICount = {};

    for (unsigned I = 0; I < FortranMaxRank; ++I) {
      IICount[I] = 0;
      NoIICount[I] = 0;
    }
    SmallPtrSet<Instruction *, 32> Visited;
    for (auto &KV : FuncToSubsVec) {
      auto &LI = (GetLI)(*KV.first);
      if (LI.empty())
        continue;

      for (auto *Subs : KV.second)
        ComputeGain(Subs, LI, IndexChain, IndexVarianceDepth,
                    IICount, NoIICount, Gains, Visited);
    }
    // Ensure that we do not make an indirectly indexed dimension a fast
    // moving one if the indirectly indexed references are a significant
    // percentage of the total.
    for (unsigned I = 0; I < FortranMaxRank; ++I) {
      unsigned Total = IICount[I] + NoIICount[I];
      unsigned Ratio = Total > 0 ? 100 * IICount[I] / Total : 0;
      if (IICount[I] && (Ratio > TransposeMinIIRatio)) {
        DEBUG_WITH_TYPE(DEBUG_PROFITABILITY,
          dbgs() << "Indirectly Indexed: Forcing Gain[" << I << "] to 0.0\n");
        Gains[I] = 0.0;
      }
    }
    double CurrentUnitStridedGain = Gains[0];

    // This vector will hold {Gain, Dimension} to sort the gains from lowest to
    // highest. Because we include the dimension number, the sort will be
    // deterministic, and keep the existing order in case of equal gain values.
    SmallVector<std::pair<double, unsigned>, FortranMaxRank> GainDimArray;
    for (unsigned Dim = 0; Dim < ArrayRank; ++Dim) {
      DEBUG_WITH_TYPE(DEBUG_PROFITABILITY, dbgs() << "  Gain[" << Dim << "] = "
                                                  << Gains[Dim] << "\n");
      GainDimArray.push_back({Gains[Dim], Dim});
    }

    std::sort(GainDimArray.begin(), GainDimArray.end());

    // If the element with the highest gain, is not currently the unit stride,
    // then check ratio of it against the element that currently has the
    // smallest stride, and mark it as profitable if appropriate.
    unsigned MaxVaryingDim = GainDimArray[ArrayRank - 1].second;
    if (MaxVaryingDim != 0) {
      double MaxGain = GainDimArray[ArrayRank - 1].first;
      if ((MaxGain / CurrentUnitStridedGain) >= TransposeMinGainFactor) {
        DEBUG_WITH_TYPE(
            DEBUG_PROFITABILITY,
            dbgs() << "  Transpose is profitable. Max gain [Dimension="
                   << MaxVaryingDim << "] = " << MaxGain
                   << " Cur gain = " << CurrentUnitStridedGain << "\n");

        SmallVector<uint32_t, FortranMaxRank> TransposeOrder;
        for (unsigned Dim = ArrayRank; Dim > 0; --Dim)
          TransposeOrder.push_back(GainDimArray[Dim - 1].second);

        DEBUG_WITH_TYPE(DEBUG_PROFITABILITY, {
          dbgs() << "Transpose order: [";
          for (auto I : TransposeOrder)
            dbgs() << " " << I;
          dbgs() << " ]\n";
        });
        setIsProfitable(true);
        setTransposition(TransposeOrder);
      }
    }
  }

  // Transform the strides in the subscript calls and dope vector creation, if
  // the candidate is valid for being transposed.
  bool transform() {
    if (!IsValid || !IsProfitable)
      return false;

    LLVM_DEBUG(dbgs() << "Transforming candidate:" << GV->getName() << "\n");
    transposeStrides();
    return true;
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump() { print(dbgs()); }

  void print(raw_ostream &OS) {
    OS << "Transpose candidate: " << GV->getName() << "\n";
    OS << "Type         : " << *GV->getType() << "\n";
    OS << "Rank         : " << ArrayRank << "\n";
    OS << "Element size : " << ElementSize << "\n";
    OS << "Element type : " << *ElementType << "\n";
    OS << "Dope vector  : " << (IsGlobalDV ? true : false) << "\n";
    if (getNestedFieldNumber())
      OS << "Nested Field Number : " << *getNestedFieldNumber() << "\n";
    OS << "Strides      :";
    for (uint32_t RankNum = 0; RankNum < ArrayRank; ++RankNum)
      OS << " " << Strides[RankNum];
    OS << "\n";
    OS << "Array Lengths :";
    for (uint32_t RankNum = 0; RankNum < ArrayRank; ++RankNum)
      OS << " " << ArrayLength[RankNum];
    OS << "\n";
    OS << "Transposition:";
    if (!Transposition.empty())
      for (uint32_t RankNum = 0; RankNum < ArrayRank; ++RankNum)
        OS << " " << Transposition[RankNum];
    OS << "\n";
    OS << "IsValid      : " << (IsValid ? "true" : "false") << "\n";
    OS << "IsProfitable : " << (IsProfitable ? "true" : "false") << "\n";
    OS << "--------------\n";
  }
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

private:
  // The global variable that is a possible candidate
  GlobalVariable *GV;

  // Number of dimensions (Fortran Rank) for the array
  uint32_t ArrayRank;

  // Number of elements in each dimension of the array.
  // ArrayLength[I] is the length of the I-th dimension of the array.
  //
  SmallVector<uint64_t, 4> ArrayLength;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  // Size of one element in the array, in bytes.
  uint64_t ElementSize;
#endif

  // Element type in the array
  llvm::Type *ElementType;

  dtrans::TransposeTLIType GetTLI;

  // 'true' if the candidate is represented by a global dope vector
  bool IsGlobalDV;

  // If present, the field in the structure containing the candidate
  Optional<uint64_t> NestedFieldNumber;

  // This vector stores the stride values used when operating on the complete
  // array. For this optimization, we do not support cases where a sub-object is
  // passed to a function as a portion of the array. Strides are stored in the
  // vector so that the dimension can be used as the index value into the
  // vector. i.e. a 9x9 array of integers would store {4,36}, since Rank 0 uses
  // a stride of 4 and Rank 1 uses a stride of 36.
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
  SubscriptInstSet SubscriptCalls;

  // Set of calls to the subscript intrinsic that access the candidate via a
  // dope vector. These calls should be analyzed for profitability but do not
  // need to be transformed because they take their parameters from the dope
  // vector.
  SubscriptInstSet DVSubscriptCalls;

  // Stores to strides of the dope vector (which must be updated if the
  // transpose is performed.)
  DopeVectorFieldUse::StoreInstSet StrideStores[FortranMaxRank];

  // Set of dope vector objects that were directly created from the global
  // variable.
  SmallPtrSet<DopeVectorAnalyzer *, 4> DopeVectorInstances;

  // Indicates whether the analysis determined the candidate is safe to
  // transpose.
  bool IsValid;

  // Indicates whether the analysis determined the candidate should be
  // transposed.
  bool IsProfitable;

  // This function will swap the strides used for indexing into the array.
  // These need to be changed for subscript operators that directly index
  // into the global variable, and for the setup of the dope vectors used
  // when passing the global variable to another function. Extents in the
  // dope vectors will also be swapped.
  void transposeStrides() {
    assert(!Transposition.empty() && "New indices should have been set.");

    for (auto *Call : SubscriptCalls)
      transposeSubscriptCall(*Call, /*TransposeStrides=*/true);

    for (auto *Call : DVSubscriptCalls)
      transposeSubscriptCall(*Call, /*TransposeStrides=*/false);

    transposeDopeVector();
  }

  // This takes a subscript call for the highest Rank, whose result is fed to a
  // subscript call of the next lower rank, and updates the subscript call to
  // change the Rank parameter to reflect the transposed ordering. When \p
  // TranspoeStrides is 'true', the call is using constant values for the stride
  // parameter which also need to be updated.
  void transposeSubscriptCall(SubscriptInst &Subs, bool TransposeStrides) {

    std::function<void(SubscriptInst &, unsigned, bool,
                       SmallPtrSetImpl<SubscriptInst *> &)>
        ProcessSubscriptCall;

    std::function<void(Instruction *, unsigned, bool,
                       SmallPtrSetImpl<SubscriptInst *> &)>
        ProcessUsers = [&ProcessSubscriptCall, &ProcessUsers](
                           Instruction *I, unsigned Rank, bool TransposeStrides,
                           SmallPtrSetImpl<SubscriptInst *> &Visited) -> void {
      for (auto *UU : I->users()) {
        if (auto *Subs2 = dyn_cast<SubscriptInst>(UU))
          ProcessSubscriptCall(*Subs2, Rank - 1, TransposeStrides, Visited);
        else if (isa<PHINode>(UU) || isa<SelectInst>(UU))
          ProcessUsers(cast<Instruction>(UU), Rank, TransposeStrides, Visited);
      }
    };

    // Lambda to process one subscript call, and recurse to subscript calls that
    // use the result.
    ProcessSubscriptCall =
        [this, &ProcessUsers](
            SubscriptInst &Subs, unsigned Rank, bool TransposeStrides,
            SmallPtrSetImpl<SubscriptInst *> &Visited) -> void {
      if (!Visited.insert(&Subs).second)
        return;

      // Check if this index is being transposed from its original rank.
      unsigned TransposeIdx = Transposition[Rank];
      if (TransposeIdx != Rank) {
        DEBUG_WITH_TYPE(DEBUG_TRANSFORM,
                        dbgs() << "Before: " << Subs.getFunction()->getName()
                               << ":" << Subs << "\n");
        if (TransposeStrides) {
          // At this point we know the constant stride values whether or not
          // they have been replaced in the IR. So, we can generate constant
          // stride arguments for the updated SubscriptInsts regardless.
          uint64_t NewStride = Strides[0];
          for (unsigned I = 0; I < TransposeIdx; ++I)
            NewStride *= ArrayLength[Transposition[I]];
          auto *NewStrideConst = ConstantInt::get(
              Subs.getArgOperand(StrideOpNum)->getType(), NewStride);
          Subs.setArgOperand(StrideOpNum, NewStrideConst);
          // We may as well set the lower bound, since we have screened
          // for a constant value of 1.
          auto *NewLBConst = ConstantInt::get(
              Subs.getArgOperand(LBOpNum)->getType(), 1);
          Subs.setArgOperand(LBOpNum, NewLBConst);
        }

        // Modify the 'Rank' field because loop opt expects the stride
        // values decrease as the Rank decreases.
        Subs.setArgOperand(
            RankOpNum,
            ConstantInt::get(Subs.getArgOperand(RankOpNum)->getType(),
                             TransposeIdx));
        DEBUG_WITH_TYPE(DEBUG_TRANSFORM,
                        dbgs() << "After : " << Subs.getFunction()->getName()
                               << ":" << Subs << "\n");
      }

      if (Rank != 0)
        ProcessUsers(&Subs, Rank, TransposeStrides, Visited);
    };

    SmallPtrSet<SubscriptInst *, 32> Visited;
    ProcessSubscriptCall(Subs, ArrayRank - 1, TransposeStrides, Visited);
  }

  //
  // Modify the value stored into the stride fields of the dope vector.
  //
  // Here's an important thing to note:
  // Before the transpose operation, the subscripts indexing a multi-dimension
  // array will be chained so that the highest dimension appears first
  // (outermost) in the chain. For example, in this chain for a 2-dimensional
  // array with base pointer %33, the first subscript call is for the rank-1
  // dimension:
  //   %37 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(
  //       i8 1, i64 %36, i64 %35, float* %33, i64 %29)
  //   %38 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(
  //       i8 0, i64 %34, i64 4, float* %37, i64 %27)
  // after the transposing the array, the subscript chain would look like this:
  //   %37 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(
  //       i8 0, i64 %36, i64 4, float* %33, i64 %29)
  //   %38 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(
  //       i8 1, i64 %34, i64 400, float* %37, i64 %27)
  // This is counter to the expectation of Loop Opt which genenerally expects
  // the subscript calls in the previous order.  To compensate, after the
  // transpose, the i-th dimension of the dope vector will actually be in the
  // TranspositionOrder[i]-th position. So, for example, if the 2-dimensional
  // array is transposed, dimension 0 of the dope vector will refer to
  // dimension 1 of the array, and vice versa. This implies that updating the
  // dope vector involves only updating the strides, as the lower bounds and
  // extents will stay the same.
  //
  // Alternately, we could have chosen to rethread the chain so that reference
  // to the highest dimension appeared first in the chain after the transpose
  // was done. This would be difficult, though, because hoisting of the
  // subscripts is done before the transpose is performed and it could require
  // sinking all of the subscript calls in a chain so that they are all in the
  // same basic block before we do the rethreading. That is why the rethreading
  // approach was not chosen.
  //
  void transposeDopeVector() {

    uint64_t NewStride = Strides[0];
    for (unsigned Rank = 0; Rank < ArrayRank; ++Rank) {
      unsigned TransposeIdx = Transposition[Rank];
      // Check if this index is being transposed from its original rank.
      uint64_t Multiplier = ArrayLength[TransposeIdx];
      if (TransposeIdx == Rank) {
        NewStride *= Multiplier;
        continue;
      }
      for (auto *SI : StrideStores[TransposeIdx]) {
        DEBUG_WITH_TYPE(DEBUG_TRANSFORM, dbgs() << "Before: " << *SI << "\n");

        auto *NewStrideConst =
            ConstantInt::get(SI->getOperand(0)->getType(), NewStride);
        SI->setOperand(0, NewStrideConst);
        DEBUG_WITH_TYPE(DEBUG_TRANSFORM, dbgs() << "After : " << *SI << "\n");
      }
      NewStride *= Multiplier;
    }
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
  TransposeImpl(dtrans::TransposeLoopInfoFuncType GetLI,
                dtrans::TransposeTLIType GetTLI) :
                GetLI(GetLI), GetTLI(GetTLI) {}

  bool run(Module &M) {
    const DataLayout &DL = M.getDataLayout();

    IdentifyCandidates(M);

    // Look for any candidates that should be marked as profitable based on
    // command line flags.
    if (!TransposeOverride.empty())
      parseOverrideFlag();

    bool ValidCandidate = false;
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    if (PrintCandidates && Candidates.empty())
      dbgs() << "No transpose candidates\n";
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    for (auto &Cand : Candidates) {
      ValidCandidate |= Cand.analyze(DL);

      if (ValidCandidate)
        Cand.computeProfitability(GetLI);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
      if (PrintCandidates)
        Cand.dump();
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    }

    bool Changed = false;
    if (ValidCandidate)
      for (auto &Cand : Candidates) {
        DEBUG_WITH_TYPE(DEBUG_TRANSFORM, {
          dbgs() << "Transform candidate: " << Cand.getName();
          Optional<uint64_t> NFN = Cand.getNestedFieldNumber();
          if (NFN)
            dbgs() << "[" << *NFN << "]";
          dbgs() << "\n";
        });
        Changed |= Cand.transform();
      }

    return Changed;
  }

private:
  dtrans::TransposeLoopInfoFuncType GetLI;
  dtrans::TransposeTLIType GetTLI;

  // Global variable candidates for the transformation.
  SmallVector<TransposeCandidate, 8> Candidates;

  // Identify potential candidates for the transpose optimization.
  //
  // The initial set of candidates meet the following criteria:
  // - Global Variable with internal linkage
  // - Multi-dimensional array of integer or floating point type
  // - Variable uses zero initializer or has no initializer
  void IdentifyCandidates(Module &M) {

    auto TestString = [](const StringRef &Name, unsigned StartIndex,
                        unsigned Size, const char MatchString[]) -> bool {
      return Name.substr(StartIndex, Size).equals(MatchString);
    };

    auto GetElemTypeSize = [&TestString](const DataLayout &DL,
                                         LLVMContext &C,
                                         DopeVectorInfo *DVI,
                                         llvm::Type *&ElemType,
                                         uint64_t &ElemSize) -> bool {
      // For the typed pointer case, 'ElemType' should not be nullptr.
      if (ElemType) {
        if (!ElemType->isIntegerTy() && !ElemType->isFloatingPointTy())
          return false;
        ElemSize = DL.getTypeStoreSize(ElemType);
        return true;
      }
      // For the opaque pointer case, we will not have the pointer type in
      // the IR, use the struct name to get it. Handle a few important cases
      // for now.
      unsigned StartIndex = 0;
      unsigned Size = 0;
      bool IsPointer = false;
      const StringRef &Name = DVI->getLLVMStructType()->getStructName();
      if (FindDVTypeName(Name, StartIndex, Size, IsPointer) && IsPointer) {
        if (TestString(Name, StartIndex, Size, "i32")) {
          ElemType = llvm::Type::getInt32Ty(C);
          ElemSize = DL.getTypeStoreSize(ElemType);
          return true;
        }
        if (TestString(Name, StartIndex, Size, "float")) {
          ElemType = llvm::Type::getFloatTy(C);
          ElemSize = DL.getTypeStoreSize(ElemType);
          return true;
        }
        if (TestString(Name, StartIndex, Size, "double")) {
          ElemType = llvm::Type::getDoubleTy(C);
          ElemSize = DL.getTypeStoreSize(ElemType);
          return true;
        }
      }
      return false;
    };

    // Return 'true' if 'DVI' with the given 'ArrayRank' and 'ElemType'
    // is a good candidate for transpose because:
    //   (1) Its lower bounds are all 1.
    //   (2) The stride of the zeroth element is the element size.
    //   (3) The extents are all constant.
    // When we return 'true', set the dimensions of the array in
    // 'ArrayLength', and set the 'ElemSize'.
    //
    auto IsGoodCandidate =
        [&GetElemTypeSize](
            DopeVectorInfo *DVI, const DataLayout &DL, LLVMContext &C,
            uint32_t ArrayRank, llvm::Type *&ElemType,
            SmallVector<uint64_t, 4> &ArrayLength, uint64_t &ElemSize) -> bool {
      if (ArrayRank <= 1 || ArrayRank >= FortranMaxRank)
        return false;
      if (!GetElemTypeSize(DL, C, DVI, ElemType, ElemSize))
        return false;
      for (uint32_t I = 0; I < ArrayRank; ++I) {
        auto Extent = DVI->getDopeVectorField(DV_ExtentBase, I);
        if (!Extent)
          return false;
        auto CV = Extent->getConstantValue();
        if (!CV)
          return false;
        uint64_t ThisArrayDim = CV->getZExtValue();
        if (ThisArrayDim < TransposeMinDim)
          return false;
        ArrayLength.push_back(ThisArrayDim);
        auto Stride = DVI->getDopeVectorField(DV_StrideBase, I);
        if (!Stride || !Stride->getConstantValue() ||
            (I == 0 && Stride->getConstantValue()->getZExtValue() != ElemSize))
          return false;
        auto LowerBound = DVI->getDopeVectorField(DV_LowerBoundBase, I);
        if (!LowerBound || !LowerBound->getConstantValue() ||
            !LowerBound->getConstantValue()->isOne())
          return false;
      }
      return true;
    };

    // Return 'true' if 'GV' is a pointer to a global dope vector
    // whose array could be a candidate for transpose.
    //
    // This function can generate a single non-nested TransposeCandidate and
    // push it on the list of Candidates, or generate one or more nested
    // TransposeCandidates, and push them on the list of Candidates.
    //
    // Note that this function will return 'true' if GV is a global dope
    // vector, even if no TransposeCandidates are created. This keeps us
    // from checking for the case where 'GV' is a global which is not a
    // dope vector.
    //
    // Note also that this function calls the GlobalDopeVector class member
    // function collectAndValidate(), which also performs safety checks,
    // so they do not need to be performed when analyze() is called.
    //
    auto GenGlobalDVCandidates = [this, &IsGoodCandidate](GlobalVariable *GV,
                                  const DataLayout &DL) -> bool {
      uint32_t ArrayRank = 0;
      llvm::Type *ElemType = nullptr;
      LLVMContext &C = GV->getParent()->getContext();
      if (!isDopeVectorType(GV->getValueType(), DL, &ArrayRank, &ElemType))
        return false;
      // In the future, it may be possible to transform some of the nested
      // dope vectors, but not all. For now, the GlobalDopeVector analysis
      // is all or nothing, so return immediately if it does not pass.
      GlobalDopeVector GlobDV(GV, GV->getValueType(), GetTLI);
      GlobDV.collectAndValidate(DL, /*ForDVCP=*/false);
      auto GAR = GlobDV.getAnalysisResult();
      if (GAR != GlobalDopeVector::AnalysisResult::AR_Pass)
        return true;
      if (GlobDV.hasNestedDopeVectors()) {
        for (NestedDopeVectorInfo *NestDVI : GlobDV.getAllNestedDopeVectors()) {
          if (NestDVI->getAnalysisResult() !=
             DopeVectorInfo::AnalysisResult::AR_Pass)
           continue;
          // Use isDopeVectorType() to get 'NVArrayRank' and 'NVElemType'.
          uint32_t NVArrayRank = 0;
          llvm::Type *NVElemType = nullptr;
          if (!isDopeVectorType(NestDVI->getLLVMStructType(), DL,
              &NVArrayRank, &NVElemType))
            continue;
          SmallVector<uint64_t, 4> NVArrayLength;
          uint64_t NVElemSize = 0;
          if (!IsGoodCandidate(NestDVI, DL, C, NVArrayRank, NVElemType,
                               NVArrayLength, NVElemSize))
            continue;
          TransposeCandidate Candidate(GV, NVArrayRank, NVArrayLength,
                                       NVElemSize, NVElemType, GetTLI, NestDVI,
                                       NestDVI->getFieldNum());
          Candidates.push_back(Candidate);
        }
        return true;
      }
      DopeVectorInfo *DVI = GlobDV.getGlobalDopeVectorInfo();
      SmallVector<uint64_t, 4> ArrayLength;
      uint64_t ElemSize = 0;
      if (!IsGoodCandidate(DVI, DL, C, ArrayRank,
                           ElemType, ArrayLength, ElemSize))
        return true;
      TransposeCandidate Candidate(GV, ArrayRank, ArrayLength, ElemSize,
                                   ElemType, GetTLI, DVI);
      Candidates.push_back(Candidate);
      return true;
    };

    const DataLayout &DL = M.getDataLayout();

    for (auto &GV : M.globals()) {

      // All uses of the variable need to be analyzed, therefore we need
      // internal linkage.
      if (!GV.hasInternalLinkage())
        continue;

     if (GenGlobalDVCandidates(&GV, DL))
       continue;

     if (!GV.hasInitializer() || GV.getInitializer()->isZeroValue()) {

        llvm::Type *Ty = GV.getValueType();
        auto *ArrType = dyn_cast<llvm::ArrayType>(Ty);
        if (!ArrType)
          continue;

        uint32_t Dimensions = 0;
        SmallVector<uint64_t, 4> ArrayLength;
        uint64_t ThisArrayDim = ArrType->getArrayNumElements();
        if (ThisArrayDim < TransposeMinDim)
          continue;
        ArrayLength.push_back(ThisArrayDim);
        llvm::Type *ElemType = ArrType->getArrayElementType();
        bool SkipCandidate = false;
        for (Dimensions = 1; ElemType->isArrayTy(); Dimensions++) {
          auto *InnerArrType = cast<llvm::ArrayType>(ElemType);
          uint64_t ThisArrayDim = InnerArrType->getArrayNumElements();
          if (ThisArrayDim < TransposeMinDim) {
            SkipCandidate = true;
            break;
          }
          ArrayLength.insert(ArrayLength.begin(), ThisArrayDim);
          ElemType = InnerArrType->getArrayElementType();
        }
        if (SkipCandidate)
          continue;

        if (Dimensions > 1 && Dimensions <= FortranMaxRank &&
            (ElemType->isIntegerTy() || ElemType->isFloatingPointTy())) {
          LLVM_DEBUG(dbgs() << "Adding candidate: " << GV << "\n");
          uint64_t ElemSize = DL.getTypeStoreSize(ElemType);
          TransposeCandidate Candidate(&GV, Dimensions, ArrayLength,
                                       ElemSize, ElemType, GetTLI);
          Candidates.push_back(Candidate);
        }
      }
    }
  }

  // Parse the command line override flag that specifies a transpose ordering
  // for a variable.
  void parseOverrideFlag() {
    SmallVector<StringRef, 4> CandStrings;
    SmallVector<StringRef, 4> FieldStrings;
    SplitString(TransposeOverride, CandStrings, ";");
    for (auto &Arg : CandStrings) {
      SplitString(Arg, FieldStrings, ",");
      StringRef Name = FieldStrings[0];
      for (auto &Cand : Candidates) {
        if (Cand.getName() == Name) {
          uint32_t Ranks = FieldStrings.size() - 1;
          SmallVector<uint32_t, FortranMaxRank> TransposeVector;
          for (unsigned Idx = 1; Idx <= Ranks; ++Idx)
            TransposeVector.push_back(
                std::stoi(std::string(FieldStrings[Idx])));

          // Validate the index values as having one value per rank.
          SmallVector<uint32_t, FortranMaxRank> Tmp;
          Tmp.resize(Cand.getArrayRank());
          std::iota(Tmp.begin(), Tmp.end(), 0);
          if (!std::is_permutation(Tmp.begin(), Tmp.end(),
                                   TransposeVector.begin())) {
            LLVM_DEBUG(dbgs() << "Invalid rank description: " << Arg << "\n");
            continue;
          }

          // Transposition wants the vector to be indexed by the Rank ID.
          std::reverse(TransposeVector.begin(), TransposeVector.end());
          Cand.setTransposition(TransposeVector);
          Cand.setIsProfitable(true);
        }
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

    auto GetLI = [this](Function &F) -> LoopInfo & {
      return this->getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();
    };
    auto GetTLI = [this](const Function &F) -> TargetLibraryInfo & {
      return this->getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
    };

    return Impl.runImpl(M, GetLI, GetTLI);
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    // Note, this transformation is not dependent on Whole Program Analysis.
    // The only candidates that may be selected for the transformation will
    // have internal linkage, and the analysis will be verifying all uses of
    // the candidate, which will ensure that the candidate is not escaped to
    // an external routine.

    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequired<TargetLibraryInfoWrapperPass>();

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
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
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

  auto GetLI = [&FAM](Function &F) -> LoopInfo & {
    return FAM.getResult<LoopAnalysis>(F);
  };
  auto GetTLI = [&FAM](const Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(*(const_cast<Function*>(&F)));
  };

  runImpl(M, GetLI, GetTLI);

  // The swapping of the stride values in the dope vectors and
  // llvm.intel.subscript intrinsic call should not invalidate any analysis.
  return PreservedAnalyses::all();
}

bool TransposePass::runImpl(Module &M, TransposeLoopInfoFuncType GetLI,
                            dtrans::TransposeTLIType GetTLI) {
  TransposeImpl Transpose(GetLI, GetTLI);
  return Transpose.run(M);
}

} // end namespace dtrans
} // end namespace llvm
