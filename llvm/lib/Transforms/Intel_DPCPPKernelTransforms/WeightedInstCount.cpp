//===- WeightedInstCount.cpp - Weighted inst count analysis ---------------===//
//
// Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/WeightedInstCount.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/InstructionCost.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/DPCPPStatistic.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/NameMangleAPI.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/PostDominanceFrontier.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/Predicator.h"

using namespace llvm;
using namespace CompilationUtils;

#define DEBUG_TYPE "dpcpp-kernel-weighted-inst-count-analysis"

extern cl::opt<VFISAKind> IsaEncodingOverride;

// MAGIC NUMBERS

// Guess for the number of iterations for loops for which
// the actual number is unknown.
static constexpr int LoopIterGuess = 32;
// Weights for different types of instructions.
// This is the baseline.
static constexpr int DefaultWeight = 1;
static constexpr int NoopWeight = 0;
// Binary operations weigh the same.
static constexpr int BinaryOpWeight = DefaultWeight;
// TODO: Calls have uniform (heavy) weight, which is nonsense.
// Replace with something that actually makes sense.
static constexpr int CallWeight = 25;
static constexpr int CallMaskWeight = 5;
// Broadcasts are effective and fast.
static constexpr int BroadcastWeight = DefaultWeight;
// Shuffles/Extracts/Inserts may be more expensive.
static constexpr int ExpensiveShuffleWeight = 5;
static constexpr int CheapShuffleWeight = 2;
static constexpr int ExpensiveExtractWeight = 2;
static constexpr int CheapExtractWeight = DefaultWeight;
static constexpr int InsertWeight = 2;

// Memops are complicated.
static constexpr int MemOpWeight = 6;
static constexpr int CheapMemOpWeight = 2;
static constexpr int ExpensiveMemOpWeight = 30;
// scatter latency on SKX ~ 10.
static constexpr int ScatterWeight = 10;
// gather latency on SKX 20-30.
static constexpr int GatherWeight = 20;
// Conditional branches are potentially expensive...
// misprediction penalty.
static constexpr int CondBranchWeight = 4;

// Special cases for cheap calls. This is clearly overfitting...
static constexpr int CallClampWeight = 2;
static constexpr int CallMinMaxWeight = 1;
static constexpr int CallFloorWeight = 2;
static constexpr int CallFakeInsertExtractWeight = 2;
static constexpr int CallRelational = 3;

static constexpr int PenaltyFactorForGEPWithSixParams = 7;
static constexpr unsigned NumParamsInGEPPenaltyHack = 6;

namespace llvm {
class InstCountResultImpl {
public:
  InstCountResultImpl(Function &F, TargetTransformInfo &TTI,
                      PostDominatorTree &DT, LoopInfo &LI, ScalarEvolution &SE,
                      VFISAKind ISA, bool PreVec);

  void analyze();

  void print(raw_ostream &OS);

  // Returns the desired vectorization factor.
  int getDesiredVF() const { return DesiredVF; }

  // Returns the computed total weight.
  float getWeight() const { return TotalWeight; }

  // Returns the probability of a basic block being executed.
  float getBBProb(const BasicBlock *BB) const { return ProbMap.lookup(BB); }

  /// Calculate the heuristic results per basic block and output as counts.
  void countPerBlockHeuristics(std::map<BasicBlock *, int> *PreCosts,
                               int PacketWidth);

  /// Allow the vectorizerCore to maintain the costs of blocks
  /// in the pre vectorization version until after vectorization.
  void copyBlockCosts(std::map<BasicBlock *, int> *Dest);

private:
  // Estimate the number of iterations each loop runs.
  void estimateIterations(DenseMap<Loop *, int> &IterMap) const;

  // Estimate the "straight-line" probability of each block being executed.
  // That is, loops are not taken into account, and backedges are considered
  // to be taken with the same probability as any other edge.
  void estimateProbability(Function &F,
                           DenseMap<BasicBlock *, float> &ProbMap) const;

  // Check which instructions depend on data (loads, image reads0
  void estimateDataDependence(Function &F,
                              DenseSet<Instruction *> &DepSet) const;

  // Get the weight of an instruction
  int getInstructionWeight(Instruction *I,
                           DenseMap<Instruction *, int> &MemOpCostMap);

  // Returns the preferred vectorization width for this kernel/architecture pair
  int getPreferredVectorizationWidth(Function &F,
                                     DenseMap<Loop *, int> &IterMap,
                                     DenseMap<BasicBlock *, float> &ProbMap);

  // Guess what the dominant type is.
  Type *estimateDominantType(Function &F, DenseMap<Loop *, int> &IterMap,
                             DenseMap<BasicBlock *, float> &ProbMap) const;

  // Estimate the relative cost of a binary operator.
  // The cost depends on the type. e.g. an operation on <16 x double> is not
  // normally supported natively. So, take that into account by estimating how
  // many native operations are required.
  int estimateBinOp(BinaryOperator *Op);

  // Helper for estimateBinOp
  unsigned getOpWidth(FixedVectorType *VecType) const;

  // Estimate the relative cost of a call.
  // Some calls are expensive, some are cheap... how do we know which are which?
  // Good question. Right now, we don't.
  int estimateCall(CallInst *CI);

  // Try to estimate the memory usage pattern. Basically, given a 2D array
  // (which is pretty common for OpenCL kernels), iterating over a row is
  // cheaper than iterating over a column. This has two implications: a) A
  // kernel that iterates over a row may be harmed by vectorization, because
  // suddenly the accesses are no longer consecutive.
  // b) A kernel that iterates over a column will probably gain from
  // vectorization because that will put work on consecutive loads together.
  void estimateMemOpCosts(Function &F,
                          DenseMap<Instruction *, int> &CostMap) const;

  // Helper function for DFS
  void addUsersToWorklist(Instruction *I, DenseSet<Instruction *> &Visited,
                          std::vector<Instruction *> &WorkList) const;

  // Return the cost of a function call
  int getFuncCost(StringRef Name);

private:
  Function &F;

  TargetTransformInfo &TTI;

  PostDominatorTree &DT;

  LoopInfo &LI;

  ScalarEvolution &SE;

  VFISAKind ISA;

  // Is this a before or after vectorization pass.
  // Affects debug printing right now, but may count some
  // things differently
  bool PreVec;

  // Outputs:
  // Desired vectorization width
  int DesiredVF = 1;

  // Total weight of all instructions
  float TotalWeight;

  // for statistical purposes, cost of
  // basic block (without probability and trip count)
  std::map<BasicBlock *, int> BlockCosts;

  // A map of costs for the load/store transpose functions
  StringMap<int> TransCosts;

  // The probability of each basic block being executed.
  DenseMap<BasicBlock *, float> ProbMap;
};
} // namespace llvm

InstCountResultImpl::InstCountResultImpl(Function &F, TargetTransformInfo &TTI,
                                         PostDominatorTree &DT, LoopInfo &LI,
                                         ScalarEvolution &SE,
                                         VFISAKind ISA,
                                         bool PreVec)
    : F(F), TTI(TTI), DT(DT), LI(LI), SE(SE), ISA(ISA), PreVec(PreVec) {
  if (IsaEncodingOverride.getNumOccurrences())
    this->ISA = IsaEncodingOverride.getValue();

  // Costs for transpose functions for 64bit target.
  using FuncCostEntry = std::pair<const char *, int>;
  static const SmallVector<FuncCostEntry, 24> CostDB64Bit = {
      {"__ocl_load_transpose_char_4x4", 8},
      {"__ocl_transpose_store_char_4x4", 8},
      {"__ocl_masked_load_transpose_char_4x4", 12},
      {"__ocl_masked_transpose_store_char_4x4", 12},
      {"__ocl_gather_transpose_float_4x4", 200},
      {"__ocl_transpose_scatter_float_4x4", 200},
      {"__ocl_load_transpose_float_4x8", 70},
      {"__ocl_transpose_store_float_4x8", 70},
      {"__ocl_gather_transpose_float_4x8", 200},
      {"__ocl_transpose_scatter_float_4x8", 200},
      {"__ocl_masked_load_transpose_float_4x8", 80},
      {"__ocl_masked_transpose_store_float_4x8", 80},
      {"__ocl_masked_gather_transpose_float_4x8", 200},
      {"__ocl_masked_transpose_scatter_float_4x8", 200},
      {"__ocl_load_transpose_char_4x16", 70},
      {"__ocl_gather_transpose_char_4x16", 150},
      {"__ocl_transpose_scatter_char_4x16", 150},
      {"__ocl_masked_gather_transpose_char_4x16", 200},
      {"__ocl_masked_transpose_scatter_char_4x16", 200},
      {"__ocl_gather_transpose_short_4x16", 150},
      {"__ocl_masked_gather_transpose_short_4x16", 200},
      {"__ocl_load_transpose_int_4x16", 70},
      {"__ocl_load_transpose_float_4x16", 70},
      {"__ocl_masked_load_transpose_float_4x16", 80},
  };

  // Costs for transpose functions for 32bit target.
  static const SmallVector<FuncCostEntry, 24> CostDB32Bit = {
      {"__ocl_load_transpose_char_4x4", 8},
      {"__ocl_transpose_store_char_4x4", 8},
      {"__ocl_masked_load_transpose_char_4x4", 12},
      {"__ocl_masked_transpose_store_char_4x4", 12},
      {"__ocl_load_transpose_float_4x8", 70},
      {"__ocl_transpose_store_float_4x8", 70},
      {"__ocl_gather_transpose_float_4x8", 75},
      {"__ocl_transpose_scatter_float_4x8", 75},
      {"__ocl_masked_load_transpose_float_4x8", 80},
      {"__ocl_masked_transpose_store_float_4x8", 80},
      {"__ocl_masked_gather_transpose_float_4x8", 90},
      {"__ocl_masked_transpose_scatter_float_4x8", 90},
      {"__ocl_load_transpose_char_4x16", 70},
      {"__ocl_gather_transpose_char_4x16", 75},
      {"__ocl_transpose_scatter_char_4x16", 80},
      {"__ocl_masked_gather_transpose_char_4x16", 90},
      {"__ocl_masked_transpose_scatter_char_4x16", 90},
      {"__ocl_gather_transpose_short_4x16", 80},
      {"__ocl_masked_gather_transpose_short_4x16", 90},
      {"__ocl_load_transpose_int_4x16", 70},
      {"__ocl_load_transpose_float_4x16", 70},
      {"__ocl_masked_load_transpose_float_4x16", 80},
  };

  if (F.getParent()->getDataLayout().getPointerSize() == 64)
    TransCosts.insert(CostDB64Bit.begin(), CostDB64Bit.end());
  else
    TransCosts.insert(CostDB32Bit.begin(), CostDB32Bit.end());

  // If function is already vectorized, PreVec should be forced to be false.
  // E.g. when this pass is run before VectorKernelElimination, vectorized
  // kernel probably exists.
  auto KIMD = DPCPPKernelMetadataAPI::KernelInternalMetadataAPI(&F);
  static constexpr StringRef VolcanoVectorKernelPrefix = "__Vectorized_";
  if (!F.getName().startswith(VolcanoVectorKernelPrefix) &&
      KIMD.ScalarKernel.hasValue() && KIMD.ScalarKernel.get())
    this->PreVec = false;

  analyze();
}

void InstCountResultImpl::analyze() {
  if (F.hasOptNone() || isGlobalCtorDtor(&F))
    return;

  // for statistics:
  DPCPP_STAT_GATHER_CHECK(BlockCosts.clear(););

  // This is for safety - don't return 0.
  TotalWeight = 1;

  // Check if this is the "pre" stage.
  // If it is, compute things that are relevant only here.
  DenseMap<Instruction *, int> MemOpCostMap;
  if (PreVec) {
    // Compute the "cost map" for stores and loads. This is only
    // done pre-vectorization. See function for extended explanation.
    estimateMemOpCosts(F, MemOpCostMap);
  }

  // First, estimate the total number of iterations each loop in the
  // functions runs. Once we have this, we can multiply the count
  // by each instruction's weight.
  DenseMap<Loop *, int> IterMap;
  estimateIterations(IterMap);

  // Now compute some estimation of the probability of each basic block
  // being executed in a run.
  estimateProbability(F, ProbMap);

  // Ok, start counting with 0
  TotalWeight = 0;

  // For each basic block, add up its weight
  LLVM_DEBUG(dbgs() << "Calculate cost for function " << F.getName()
                    << ", PreVec: " << PreVec << "\n");

  for (auto &BB : F) {
    bool DiscardPhis = false;
    bool DiscardTerminator = false;
#ifndef INTEL_PRODUCT_RELEASE
    int BlockWeights = 0; // for statistical purposes.
#endif                    // #ifndef INTEL_PRODUCT_RELEASE

    // Check if BB is an idom of an allOnes branch
    // and if it does discard its phis cost
    Predicator::AllOnesBlockType BlockType =
        Predicator::getAllOnesBlockType(&BB);
    if (isa<PHINode>(BB.begin()))
      if (BlockType == Predicator::EXIT)
        DiscardPhis = true;

    if (BlockType == Predicator::ALLONES || BlockType == Predicator::ORIGINAL) {
      DiscardTerminator = true;
    }

    // Check if the basic block in a loop. If it is, we want to multiply
    // all of its instruction's weights by its tripcount.
    int TripCount = 1;
    if (Loop *ContainingLoop = LI.getLoopFor(&BB))
      TripCount = IterMap.lookup(ContainingLoop);

    float Probability = ProbMap.lookup(&BB);

    // And now, sum up all the instructions
    LLVM_DEBUG(dbgs() << "Calculate cost for BB " << BB.getName() << '\n');
    for (auto &I : BB) {
      if (DiscardPhis && dyn_cast<PHINode>(&I))
        continue;
      if (DiscardTerminator && I.isTerminator())
        continue;

      int InstWeight = getInstructionWeight(&I, MemOpCostMap);
      float Cost = Probability * TripCount * InstWeight;
      TotalWeight += Cost;
      LLVM_DEBUG(dbgs() << "  [" << format_decimal(InstWeight, 4) << "] " << I
                        << "\n");

#ifndef INTEL_PRODUCT_RELEASE
      BlockWeights += InstWeight; // for statisical purposes.
#endif                            // #ifndef INTEL_PRODUCT_RELEASE
    }
    LLVM_DEBUG(dbgs() << "Cost of BB " << BB.getName() << ':' << " Final Cost: "
                      << format("%6.2f", BlockWeights * Probability * TripCount)
                      << " Block Weight: " << BlockWeights
                      << " Prob: " << format("%4.2f", Probability)
                      << " TripCount: " << TripCount << '\n');
    // for statistics:
    DPCPP_STAT_GATHER_CHECK(BlockCosts[&BB] = BlockWeights;);
  }
  LLVM_DEBUG(dbgs() << "Cost of function " << F.getName() << ": "
                    << format("%.2f", TotalWeight) << '\n');

  // If we are pre-vectorization, decide what the vectorization width should be.
  // Note that v16 support was already decided earlier. The reason the code is
  // split is that for the v16 we don't need to compute the various maps, while
  // in this part of the code we want to use them.
  if (PreVec)
    DesiredVF = getPreferredVectorizationWidth(F, IterMap, ProbMap);
}

int InstCountResultImpl::getPreferredVectorizationWidth(
    Function &F, DenseMap<Loop *, int> &IterMap,
    DenseMap<BasicBlock *, float> &ProbMap) {
  // For SSE, this is always 4.
  if (ISA == VFISAKind::SSE)
    return 4;

  // For AVX, estimate the most used type in the kernel.
  // Integers have no 8-wide operations on AVX1, so vectorize to 4,
  // otherwise, 8.
  // This logic was inherited from the old heuristic, but the types
  // are computed slightly more rationally now.
  if (ISA == VFISAKind::AVX) {
    auto KIMD = DPCPPKernelMetadataAPI::KernelInternalMetadataAPI(&F);
    // For AVX, there is only x4 native sub-group implementation.
    if (KIMD.KernelHasSubgroups.hasValue() && KIMD.KernelHasSubgroups.get())
      return 4;
    Type *DominantType = estimateDominantType(F, IterMap, ProbMap);
    return DominantType->isIntegerTy() ? 4 : 8;
  }

  // For AVX2, the logical choice would be always 8.
  // For AVX512 the logical choice would be always 16
  // (unless we run into the need of not using zmms on Client like ICC did).
  // Unfortunately, this fails for some corner cases, due to both
  // inherent reasons and compiler deficiencies.
  // The first corner case is <k x i16*> buffers. Since we don't have transpose
  // operations for i16, reading and writing from these buffers becomes
  // expensive.
  for (const auto &Arg : F.args()) {
    Type *ArgType = Arg.getType();

    // Is this a pointer type?
    PointerType *PtrArgType = dyn_cast<PointerType>(ArgType);
    if (!PtrArgType)
      continue;

    // Pointer to vector of i16?
    Type *PointedType = PtrArgType->getElementType();
    if (!PointedType->isVectorTy())
      continue;

    if (!PointedType->getScalarType()->isIntegerTy(16))
      continue;

    // Last thing to check - that this buffer is not trivially dead.
    if (Arg.hasNUsesOrMore(1))
      return 4;
  }

  if (ISA != VFISAKind::AVX512)
    return 8;

  return 16;
}

// This allows a consistent comparison between scalar and vector types.
// Under normal conditions, a pointer comparison always occurs, which
// is consistent for a single run, but not between runs.
static bool typeCompare(Type *Left, Type *Right) {
  FixedVectorType *VTypeLeft = dyn_cast<FixedVectorType>(Left);
  FixedVectorType *VTypeRight = dyn_cast<FixedVectorType>(Right);

  if (nullptr != VTypeRight && nullptr == VTypeLeft)
    return true;

  if (nullptr == VTypeRight && nullptr != VTypeLeft)
    return false;

  if (nullptr != VTypeLeft && nullptr != VTypeRight)
    if (VTypeLeft->getNumElements() != VTypeRight->getNumElements())
      return (VTypeLeft->getNumElements() < VTypeRight->getNumElements());

  if (Left->getScalarSizeInBits() != Right->getScalarSizeInBits())
    return (Left->getScalarSizeInBits() < Right->getScalarSizeInBits());

  Type *ScalarLeft = Left->getScalarType();
  Type *ScalarRight = Right->getScalarType();

  if (ScalarLeft->isIntegerTy() && !ScalarRight->isIntegerTy())
    return true;

  if (!ScalarLeft->isIntegerTy() && ScalarRight->isIntegerTy())
    return false;

  // Fallback to a pointer comparison for other types.
  return Left < Right;
}

Type *InstCountResultImpl::estimateDominantType(
    Function &F, DenseMap<Loop *, int> &IterMap,
    DenseMap<BasicBlock *, float> &ProbMap) const {
  DenseMap<Type *, float> CountMap;

  // For each type, count how many times it is the first operand of a binop.
  for (auto &BB : F) {
    int TripCount = 1;
    if (Loop *ContainingLoop = LI.getLoopFor(&BB))
      TripCount = IterMap.lookup(ContainingLoop);

    float Probability = ProbMap.lookup(&BB);
    for (auto &I : BB) {
      // We only care about BinOps
      if (auto *BinOp = dyn_cast<BinaryOperator>(&I)) {
        Type *OpType = BinOp->getOperand(0)->getType();
        // Get the base type for vectors
        OpType = OpType->getScalarType();
        CountMap[OpType] += TripCount * Probability;
      }
    }
  }

  // Find the maximum. The map is expected to be small, just iterate over it.
  // Use i32 by default... if there are no binops at all. Should be pretty rare.
  // In case there are several values with equal maximum keys, use a comparator
  // to choose the maximum.
  // This is needed because otherwise the choice depends on the order of
  // iteration which, in turn, depends on the values of the Type pointers, which
  // change from run to run. This makes the choice inconsistent between runs.
  Type *DominantType = Type::getInt32Ty(F.getContext());
  float MaxCount = 0;
  for (auto &Pair : CountMap) {
    if ((Pair.second > MaxCount) ||
        (Pair.second == MaxCount && typeCompare(Pair.first, DominantType))) {
      MaxCount = Pair.second;
      DominantType = Pair.first;
    }
  }
  return DominantType;
}

/// isequal ... islessgreater are already replaced with instruction in
/// BuiltinCallToInst pass, so there is no need to handle them.
static bool isRelationalBuiltin(StringRef Name) {
  return StringSwitch<bool>(Name)
      .Case("isfinite", true)
      .Case("isinf", true)
      .Case("isnan", true)
      .Case("isnormal", true)
      .Case("isordered", true)
      .Case("isunordered", true)
      .Case("signbit", true)
      .Case("any", true)
      .Case("all", true)
      .Case("bitselect", true)
      .Case("select", true)
      .Default(false);
}

int InstCountResultImpl::estimateCall(CallInst *CI) {
  // TID generators are extremely common and very cheap.
  bool IsTidGen;
  unsigned Dim;
  std::tie(IsTidGen, Dim) = isTIDGenerator(CI);
  if (IsTidGen)
    return DefaultWeight;

  Function *Callee = CI->getCalledFunction();

  // Handle the special case of unnamed functions (call through a bitcast)
  if (!Callee)
    return CallWeight;

  StringRef Name = Callee->getName();
  bool IsIntrinsic = Callee->isIntrinsic();

  // For volcano
  bool IsOCLLoad = Predicator::isMangledLoad(Name);
  bool IsOCLStore = Predicator::isMangledStore(Name);
  // For VPlan
  bool IsLoadIntrinsic =
      IsIntrinsic && Callee->getIntrinsicID() == Intrinsic::masked_load;
  bool IsStoreIntrinsic =
      IsIntrinsic && Callee->getIntrinsicID() == Intrinsic::masked_store;

  // Since we run before the resolver, masked load/stores should count
  // as load/stores, not calls. Maybe slightly better or worse.
  if (IsOCLLoad || IsOCLStore || IsLoadIntrinsic || IsStoreIntrinsic) {
    // If the mask is non-scalar, this will become a lot of memops,
    // since the CPU doesn't have gathers.
    unsigned MaskIdx;
    if (IsOCLLoad || IsOCLStore) {
      MaskIdx = 0;
    } else if (IsLoadIntrinsic) {
      MaskIdx = 2;
    } else { // IsStoreIntrinsic
      MaskIdx = 3;
    }
    Value *Mask = CI->getArgOperand(MaskIdx);
    Type *MaskType = Mask->getType();

    // apperently, masked stores to a memory location that was retrieved via
    // a get element pointer instruction with 6 parameters,
    // inside a block which is a loop of a single block, are
    // surprisingly expensive (about 7 times that of a usual MEM_OP).
    // alternatively...
    // this could be the result of an outragous over-fitting, designed
    // to prevent LuxMark::Sampler from vectorizing.
    // So yes, this is a hack for this purpose.
    // The check is very specific trying to catch LuxMark::Sampler.
    // alone without causing collateral damage.
    //
    // FIXME:
    // I checked the IR in LuxMark/Luxball_HDR and LuxMark/Microphone, and
    // there's no call to masked_load/maskd_store with this kind of GEP at
    // all. This kind of GEP's are directly used by 'load' instructions. I
    // suggest removing this piece of code or making it fit in more general
    // cases. E.g., penalize GEP's with more than 6 operands instead of exact
    // 6 operands.
    if ((IsOCLStore || IsStoreIntrinsic) && !MaskType->isVectorTy()) {
      assert(CI->arg_size() == 3 && "expected 3 params in masked store");
      if (auto *GEP = dyn_cast<GetElementPtrInst>(CI->getArgOperand(1))) {
        if (GEP->getNumOperands() == NumParamsInGEPPenaltyHack) {
          BasicBlock *BB = CI->getParent();
          for (auto *Succ : successors(BB))
            if (Succ == BB)
              return MemOpWeight * PenaltyFactorForGEPWithSixParams;
        }
      }
    }

    // For scalar masks, it'll be a pretty little vector store/load
    if (!MaskType->isVectorTy())
      return MemOpWeight;

    // This is a vector type, it'll be ugly.
    int NumElements = cast<FixedVectorType>(MaskType)->getNumElements();
    return MemOpWeight * NumElements;
    // TODO: if the vector is really large, still need to multiply...
  }

  // For volcano
  bool IsOCLGather = Predicator::isMangledGather(Name);
  bool IsOCLScatter = Predicator::isMangledScatter(Name);
  // For VPlan
  bool IsGatherIntrinsic =
      IsIntrinsic && Callee->getIntrinsicID() == Intrinsic::masked_gather;
  bool IsScatterIntrinsic =
      IsIntrinsic && Callee->getIntrinsicID() == Intrinsic::masked_scatter;

  if (IsOCLGather || IsOCLScatter || IsGatherIntrinsic || IsScatterIntrinsic) {
    // 16 x 32bit element gather/scatter with 64 bit indices will turn
    // into 2 gathers/scatters, so adjust weight accordingly.

    int Weight;
    if (IsOCLGather || IsGatherIntrinsic)
      Weight = GatherWeight;
    else
      Weight = ScatterWeight;

    // FIXME: For Volcano-style ocl_masked_gather/scatter, the address is
    // calculated by ptr + indices, so we can check if the indices are in 64
    // bits; while LLVM gather/scatter intrinsic simply accepts a vector of
    // address, so we simply return the Weight.
    if (IsGatherIntrinsic || IsScatterIntrinsic) {
      unsigned Opcode =
          IsGatherIntrinsic ? Instruction::Load : Instruction::Store;
      Type *DataTy =
          IsGatherIntrinsic ? CI->getType() : CI->getArgOperand(0)->getType();
      const Value *Ptr =
          IsGatherIntrinsic ? CI->getArgOperand(0) : CI->getArgOperand(1);
      bool VariableMask = !isa<ConstantVector>(
          IsGatherIntrinsic ? CI->getArgOperand(2) : CI->getArgOperand(3));
      auto *Alignment =
          IsGatherIntrinsic ? CI->getArgOperand(1) : CI->getArgOperand(2);
      InstructionCost IC = TTI.getGatherScatterOpCost(
          Opcode, DataTy, Ptr, VariableMask,
          Align(cast<ConstantInt>(Alignment)->getZExtValue()));
      return *IC.getValue();
    }

    // TODO remove after volcano vectorizer is removed.
    Value *Index = CI->getArgOperand(2);
    auto *Ty = cast<VectorType>(Index->getType());
    // return doubled weight for 64 bit indices
    return Ty->getScalarSizeInBits() > 32 ? (2 * Weight) : Weight;
  }

  // vloads and vstores also count as loads/stores.
  //
  // FIXME:
  // What about maskedf__Z6vload and maskedf__Z6vstore? I checked the IR, and
  // the cost of these 2 functions are 30 returned by 'getFuncCost' at the
  // last line of this function. But the maskedf__Z6vload is actually
  // expanded to the following IR in the later pass.
  //   bb:
  //     br i1 %mask, %preload, %postload
  //   preload:
  //     %loaded = call <3 x float> @_Z6vload3mPU3AS1Kf(i64 0, float
  //     addrspace(1)* %ptr) br %postload
  //   postload:
  //     %val = phi <3 x float> [ zeroinitializer, %bb ], [ %loaded, %preload ]
  // The pass just fails to identify vload/vstore/... functions with
  // maskedf_ prefix, and it treats them as unknown functions calls, whose
  // cost is 25. But the pass does know adding mask-check overhead for
  // maskdf_* functions, and the cost for mask check is 5. Then 5+25 = 30.
  // But we set the cost of vload as 6, so the correct cost should be 5+6 =
  // 11 IMO.
  if (Name.startswith("vload") || Name.startswith("_Z6vload") ||
      Name.startswith("vstore") || Name.startswith("_Z7vstore")) {
    return MemOpWeight;
  }

  // Ugly hacks start here.
  // FIXME: What about masked versions?
  using namespace NameMangleAPI;
  StringRef DemangledName = isMangledName(Name) ? stripName(Name) : Name;
  if (DemangledName == "clamp")
    return CallClampWeight;

  if (DemangledName == "floor")
    return CallFloorWeight;

  if (DemangledName == "fmin" || DemangledName == "min" ||
      DemangledName == "fmax" || DemangledName == "max")
    return CallMinMaxWeight;

  if (isRelationalBuiltin(DemangledName))
    return CallRelational;

  if (Name.startswith("fake.insert") || Name.startswith("fake.extract"))
    return CallFakeInsertExtractWeight;

  // allZero are cheap, it's basically a xor/ptest
  // allone we do not count at all, since we don't want allone-bypasses
  // to effect the result of the heuristics.
  if (Predicator::isAllZero(Name))
    return DefaultWeight;

  if (Predicator::isAllOne(Name))
    return NoopWeight;

  // See if we can find the function in the function cost table
  return getFuncCost(Name);
}

int InstCountResultImpl::getFuncCost(StringRef Name) {
  if (TransCosts.find(Name) != TransCosts.end())
    return TransCosts[Name];

  // Function is not in the table, return default call weight,
  // except that mangled (masked) calls are more expensive by default
  if (Predicator::isMangledCall(Name))
    return CallWeight + CallMaskWeight;

  return CallWeight;
}

int InstCountResultImpl::estimateBinOp(BinaryOperator *Op) {
  int Weight = BinaryOpWeight;
  Type *OpType = Op->getOperand(0)->getType();

  // If it's a scalar op, return the base weight.
  if (!OpType->isVectorTy())
    return Weight;

  FixedVectorType *VecType = cast<FixedVectorType>(OpType);
  unsigned OpWidth = getOpWidth(VecType);

  return Weight * OpWidth;
}

unsigned InstCountResultImpl::getOpWidth(FixedVectorType *VecType) const {
  Type *BaseType = VecType->getScalarType();
  unsigned BaseWidth = VecType->getScalarSizeInBits();
  unsigned ElemCount = VecType->getNumElements();

  unsigned Float, Double, LongInt, ShortInt;
  if (ISA == VFISAKind::AVX512) {
    Float = 16;
    Double = 8;
    LongInt = 8;
    ShortInt = 16;
  } else if (ISA == VFISAKind::AVX2) {
    Float = 8;
    Double = 4;
    LongInt = 4;
    ShortInt = 8;
  } else if (ISA == VFISAKind::AVX) {
    // Only AVX, 4-wide on ints, 2-wide on i64
    Float = 8;
    Double = 4;
    LongInt = 2;
    ShortInt = 4;
  } else {
    // SSE
    Float = 4;
    Double = 2;
    LongInt = 2;
    ShortInt = 4;
  }

  // All multiplications need to be rounded up, and
  // for positive integers, ceil(x/y) = (x + y - 1) / y.
  auto CeilDiv = [](unsigned X, unsigned Y) -> unsigned {
    return (X + Y - 1) / Y;
  };
  if (BaseType->isFloatingPointTy())
    return CeilDiv(ElemCount, BaseType->isFloatTy() ? Float : Double);
  return CeilDiv(ElemCount, BaseWidth > 32 ? LongInt : ShortInt);
}

int InstCountResultImpl::getInstructionWeight(
    Instruction *I, DenseMap<Instruction *, int> &MemOpCostMap) {
  // We could replace this by a switch on the opcode, but that introduces
  // a bit too many cases. So using this method (also used in
  // WorkItemAnalysis).
  // TODO: A lot of cases have been introduced as is, so, perhaps replace.
  if (BinaryOperator *BinOp = dyn_cast<BinaryOperator>(I))
    return estimateBinOp(BinOp);

  if (auto *CI = dyn_cast<CallInst>(I))
    return estimateCall(CI);

  // GEP and PHI nodes are free
  // NOTE: In the GEP case this is it not entirely true because it may result
  // in LEA
  if (isa<GetElementPtrInst>(I) || isa<PHINode>(I) || isa<AllocaInst>(I) ||
      isa<BitCastInst>(I) || isa<AddrSpaceCastInst>(I))
    return NoopWeight;

  // Shuffles/extracts/inserts are mostly representative
  // of transposes.
  if (isa<ShuffleVectorInst>(I)) {
    // Shuffling from 4xi32, 8xi32, 4xfloat and 8xfloat is cheap,
    // shuffling 16xi32, 16xfloat also should be cheap on AVX-512.
    // everything else is expensive.
    // (This is purely empirical, probably overfitting)
    Value *Vec = I->getOperand(0);
    FixedVectorType *OpType = dyn_cast<FixedVectorType>(Vec->getType());
    VectorType *ResType = dyn_cast<VectorType>(I->getType());
    assert(OpType && "Shuffle with a non-vector type!");

    ArrayRef<int> Mask = cast<ShuffleVectorInst>(I)->getShuffleMask();
    // Check whether this shuffle is a part of a broadcast sequence.
    // If it is, the price is 0, since we already paid for the insert.
    if (all_of(Mask, [](int Elt) { return Elt == 0; }))
      return BroadcastWeight;

    unsigned NumSpilt = getOpWidth(OpType);

    // A shuffle between different types won't be cheap, even if
    // the types are sensible. This should, amongst other things,
    // make revectorization from 4 to 8 less appealing.
    if (ResType != OpType)
      return ExpensiveShuffleWeight * NumSpilt;

    if ((OpType->getElementType()->isFloatTy()) ||
        OpType->getElementType()->isIntegerTy(32))
      return CheapShuffleWeight * NumSpilt;

    return ExpensiveShuffleWeight * NumSpilt;
  }

  if (isa<ExtractElementInst>(I)) {
    // Same logic as for shuffles.
    Value *Vec = I->getOperand(0);
    FixedVectorType *OpType = dyn_cast<FixedVectorType>(Vec->getType());
    assert(OpType && "Extract from a non-vector type!");

    if (((OpType->getNumElements() == 4) || (OpType->getNumElements() == 8) ||
         (ISA == VFISAKind::AVX512 && (OpType->getNumElements() == 16))) &&
        ((OpType->getElementType()->isFloatTy()) ||
         OpType->getElementType()->isIntegerTy(32)))
      return CheapExtractWeight;

    return ExpensiveExtractWeight;
  }

  if (InsertElementInst *IEI = dyn_cast<InsertElementInst>(I)) {
    // Broadcast is a compound of two instructions and is way cheaper
    // than an insert by itself.
    // Example:
    //   %ins = insertelement <8 x i32> undef, float %val, i32 0
    //   %bro = shufflevector <8 x i32> %ins, <8 x i32> undef, <8 x i32>
    //   zeroinitializer
    // So if this insert is a part of a broadcast then simply don't count it.
    // The broadcast weight will be counted at ShuffleVectorInst.
    if (IEI->hasOneUse()) {
      if (auto *SVI = dyn_cast<ShuffleVectorInst>(IEI->user_back())) {
        if (all_of(SVI->getShuffleMask(), [](int Elt) { return Elt == 0; }))
          return NoopWeight;
      }
    }

    return InsertWeight;
  }

  // We can't take spilling into account at this point, so counting loads
  // and stores may be voodoo magic.
  // Still, it works.
  if (isa<LoadInst>(I) || isa<StoreInst>(I)) {
    // Look up precomputed "cheap"/"expensive" ops
    DenseMap<Instruction *, int>::iterator OpCost = MemOpCostMap.find(I);
    if (OpCost != MemOpCostMap.end())
      return OpCost->second;

    // Not known to be special, return normal weight.
    InstructionCost IC = TTI.getMemoryOpCost(
        I->getOpcode(), getLoadStoreType(I), getLoadStoreAlignment(I),
        getLoadStoreAddressSpace(I));
    return *IC.getValue();
  }

  // Conditional branches are expensive.
  // This has two reasons - a direct one (misprediction)
  // and an indirect one (to punish complex control flow).
  if (BranchInst *BR = dyn_cast<BranchInst>(I))
    if (BR->isConditional()) {
      // we do not count allones branches, because we do not
      // want the allones optimization to change heuristic results
      // of kernels.
      Value *Cond = BR->getCondition();
      CallInst *CondCall = dyn_cast<CallInst>(Cond);
      if (CondCall && CondCall->getCalledFunction()) {
        StringRef Name = CondCall->getCalledFunction()->getName();
        if (Predicator::isAllOne(Name))
          return NoopWeight;
      }

      return CondBranchWeight;
    }

  // For everything else - use a default weight
  return DefaultWeight;
}

void InstCountResultImpl::estimateIterations(
    DenseMap<Loop *, int> &IterMap) const {
  // Walk the loop tree, "estimating" the total number of loop
  // iterations for each loop. Since we assume control flow is sane
  // the number of iterations is simply the number of iterations of
  // the current loop multiplied by the computed total number for its
  // parent loop.
  // If the number for the current loop is unknown (because it's not
  // constant, or LoopInfo could not figure it out), we guess it to be
  // LoopIterGuess. It may be possible to refine this, but I don't see
  // a good way right now.
  std::vector<Loop *> WorkList;

  // Add all the top-level loops to the worklist
  for (auto *L : LI)
    WorkList.push_back(L);

  // Now, walk the loop tree.
  while (!WorkList.empty()) {
    Loop *L = WorkList.back();
    WorkList.pop_back();

    assert(IterMap.find(L) == IterMap.end() &&
           "Looking at same loop twice, loop tree is not a tree!");

    int Multiplier = 1;
    // Is this a top-level loop?
    if (Loop *Parent = L->getParentLoop()) {
      // No, it has a parent, so we want to mulitply by the parents' count
      assert(IterMap.find(Parent) != IterMap.end() &&
             "Parent of loop is not in iteration map!");

      Multiplier = IterMap.lookup(Parent);
    }

    int Count = LoopIterGuess;
    BasicBlock *Latch = L->getLoopLatch();

    // cheating heuristics to get the same results when applying allones.
    // this is important when the condition of the latch is uniform.
    // In such a case this loop will never be reached
    // after the allones loop, and getSmallConstantTripCount() returns 0.
    if (Predicator::getAllOnesBlockType(L->getHeader()) ==
        Predicator::SINGLE_BLOCK_LOOP_ORIGINAL) {
      Latch = Predicator::getAllOnesSingleLoopBlock(L->getHeader());
      Count = SE.getSmallConstantTripCount(LI.getLoopFor(Latch), Latch);
    } else if (Latch) {
      Count = SE.getSmallConstantTripCount(L);
    }

    // getSmallConstantTripCount() returns 0 for non-constant trip counts
    // and on error conditions. In this case guess and hope for the best.
    if (Count == 0)
      Count = LoopIterGuess;

    Count *= Multiplier;

    IterMap[L] = Count;

    // Add all subloops
    for (auto *Sub : *L)
      WorkList.push_back(Sub);
  }
}

void InstCountResultImpl::estimateProbability(
    Function &F, DenseMap<BasicBlock *, float> &ProbMap) const {

  // What we really ant is a control-dependance graph.
  // Luckily, a node is control-dependent exactly on its postdom
  // frontier.

  PostDominanceFrontier PDF;
  PDF.analyze(DT);

  // Debug output
  LLVM_DEBUG(dbgs() << F.getName());
  if (PreVec)
    LLVM_DEBUG(dbgs() << " Before");
  else
    LLVM_DEBUG(dbgs() << " After");
  LLVM_DEBUG(dbgs() << "\n");
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  LLVM_DEBUG(PDF.dump());
#endif

  // Check which instructions depend on "data" (results of loads and stores).
  DenseSet<Instruction *> DepSet;
  estimateDataDependence(F, DepSet);

  // We want to make sure the heuristics gets the same result with or without
  // the allones optimization. For this reason, we need to
  // 'cheat' at the probability of some blocks.
  // some blocks need to get the probability of the entry
  // to allones block (which is why we need this map),
  // and some blocks simply need to get a zero probability.
  std::map<BasicBlock *, BasicBlock *> AllOnesToEntryBlock;

  // Now do a VERY coarse measurement. The number of nodes on which a block
  // is control-dependent is the number of decision points. For a block to be
  // reached, all decisions need to go "its way".
  // This code makes all sorts of silly assumptions.

  for (auto &BB : F) {
    PostDominanceFrontier::iterator It = PDF.find(&BB);
    // It's actually possible that a BB has no PDF (as opposed to an empty
    // one) This happens for infinite loops. If this is the case, we don't
    // really care to evaluate this block.
    if (It == PDF.end()) {
      ProbMap[&BB] = 0;
      continue;
    }

    PostDominanceFrontier::DomSetType &Frontier = It->second;
    // Before vectorization, the probability is simple 1/(2^k) where k
    // is the number of branches the block is control-dependent on.
    //
    // FIXME:
    // The code below doesn't do what the comments above goes. The k of a
    // basic block is calculated as the number of its post-dominance
    // frontiers, which is absolutely wrong. Given a CFG as follows,
    //         A
    //        / \
    //       B   C
    //          / \
    //         D   E
    // and assuming that the probability of each branch is 1/2. So,
    //   Prob(C) := 1/2
    //   Prob(D) := 1/2 * Prob(C)
    //           := 1/4
    // However, the number of D's post-dominance frontiers is 1 (C), i.e.,
    //   k(D) = 1
    // and 1/2^k(D) = 1/2, which doesn't equal to 1/4 we conclude above.
    // AFAIC, The right way to estimate the probability of a BB is,
    //   Prob(X) := \sum_{Y in S(X)}{1/n(X) * Prob(Y)}
    // where S(X) and n(x) are the set of post-dominance frontiers and the
    // successor number of X, respectively.
    int Count = (int)Frontier.size();

    // since we do not want the allones optimization to influence heuristics
    // results (vs. without using allones), we have several specail cases.
    Predicator::AllOnesBlockType BlockType =
        Predicator::getAllOnesBlockType(&BB);
    assert((BlockType >= Predicator::NONE &&
            BlockType <= Predicator::SINGLE_BLOCK_LOOP_EXIT) &&
           "Unknown block type");
    switch (BlockType) {
    case Predicator::ALLONES: // not counting, this is a duplication of
                              // ORIGINAL.
    case Predicator::SINGLE_BLOCK_LOOP_ALLONES: // dup of
                                                // SINGLE_BLOCK_LOOP_ORIGINAL
    case Predicator::SINGLE_BLOCK_LOOP_ENTRY_TO_ORIGINAL: // overhead of allones
    case Predicator::SINGLE_BLOCK_LOOP_EXIT:              // overhead of allones
    case Predicator::SINGLE_BLOCK_LOOP_TEST_ALLZEROES:    // overhead of allones
      ProbMap[&BB] = 0;
      continue;
    // original is counted without the allones optimization,
    // so we want to count it here as well. However,
    // we need to make sure we give it the same probability
    // as would have been given without the allones optimization.
    // this probability is the on that 'entry' would get
    // (since it has the same incoming edges original previously had).
    case Predicator::ORIGINAL:
      // find entry, and fill Prob at the end.
      AllOnesToEntryBlock[&BB] = Predicator::getEntryBlockFromOriginal(&BB);
      continue;
    // analogoues to original, but probability here
    // should be half of that of the entry (and not identical),
    // because there is another branch (without the allones bypasses version)
    // for this reason, we divide it by 2 at the end.
    case Predicator::SINGLE_BLOCK_LOOP_ORIGINAL:
      // find entry, and fill Prob at the end.
      AllOnesToEntryBlock[&BB] = Predicator::getEntryBlockFromLoopOriginal(&BB);
      continue;
    case Predicator::NONE: // regular block treated normally.
    // ENTRY and EXIT holds part of what is found in ORIGINAL block
    // before the allones bypass (only part of the block is duplicated)
    // so we need to count them normally as well.
    case Predicator::ENTRY:
    case Predicator::EXIT:
    // SINGLE_BLOCK_LOOP_ENTRY should have probability zero (not be counted)
    // but we do that later. First we calculate the probability
    // to be used for the SINGLE_BLOCK_LOOP_ORIGINAL.
    case Predicator::SINGLE_BLOCK_LOOP_ENTRY:
      break;
    }

    if (!PreVec) {
      // For each branch we depend on, check what the branch depends on
      for (BasicBlock *Ancestor : Frontier) {
        // find allZero/allOne conditions, and filter them out.
        BranchInst *BR = dyn_cast<BranchInst>(Ancestor->getTerminator());
        // The ancestor's terminator isn't a branch, definitely not an
        // allZero one.
        if (!BR)
          continue;

        Value *Cond = BR->getCondition();

        // For VPlan, detect the following pattern of all-zero bypass,
        //   %bc = bitcast <8 x i1> %mask to i8
        //   %az = icmp eq i8 %bc, 0
        //   br i1 %az, label %VPlannedBB.end, label %VPlannedBB
        //
        // We also detect the serialized masked operations, and assume the
        // mask is always 1,
        //   %Pred = extractelement <8 x i1> %mask, i64 0
        //   br i1 %Pred, label %VPlannedBB, label %VPlannedBB.end
        // The reason I do this is to follow a bug of the cost model in
        // Volcano. In Volcano, function calls which cannot be vectorized are
        // mangled to maskdf_*, and are expanded to serialized ones in later
        // passes. But the cost model here fails to idenitfy those maskedf_*
        // functions, and assumes they're always executed, and even returns
        // very high cost values for them. See FIXME in estimateCall(). If the
        // bug is fixed, we need to adjust cost values accordingly. But for
        // now, I just follow what Volcano does.
        Instruction *BitCastOrExtract = nullptr;
        if (auto *Cmp = dyn_cast<ICmpInst>(Cond)) {
          BitCastInst *BitCastOrExtract;
          ConstantInt *Zero;

          Value *Op0 = Cmp->getOperand(0), *Op1 = Cmp->getOperand(1);
          if ((Zero = dyn_cast<ConstantInt>(Op0)))
            BitCastOrExtract = dyn_cast<BitCastInst>(Op1);
          else if ((Zero = dyn_cast<ConstantInt>(Op1)))
            BitCastOrExtract = dyn_cast<BitCastInst>(Op0);
          else
            continue;

          if (!BitCastOrExtract || !Zero->isZero())
            continue;
        }

        if (!BitCastOrExtract)
          BitCastOrExtract = dyn_cast<ExtractElementInst>(Cond);

        if (BitCastOrExtract) {
          Value *Mask = BitCastOrExtract->getOperand(0);
          FixedVectorType *MaskTy = dyn_cast<FixedVectorType>(Mask->getType());
          if (!MaskTy || MaskTy->getScalarType() !=
                             IntegerType::getInt1Ty(MaskTy->getContext()))
            continue;

          if (DepSet.find(BitCastOrExtract) != DepSet.end())
            Count--;
          continue;
        }

        CallInst *CondCall = dyn_cast<CallInst>(Cond);
        if (!CondCall)
          // A branch, but not directly based on a call.
          continue;

        // Ok, so it's a call.
        // After vectorization, we consider dependence on an allZero/allOne
        // condition to be a bad thing. The normal structure for an allZero
        // branch is (A => B, A => C, B => C), where the A => B branch is
        // taken only if all workitems satisfy a condition. Note that block C
        // does not depend on the branch in A(since it's always executed),
        // only B does. Because of the "all workitems" conditions,  it is a
        // good idea to assume that block B is executed more often than a
        // block that depends on a "normal" branch. For loops, the stucture is
        // similar. If it has no operands, it's definitely not allOne/allZero.
        // This is basically just a sanity check, there's no good reason for
        // anyone to branch on the result of a call with no arguments.
        if (CondCall->getNumOperands() < 1)
          continue;

        // Handled unnamed functions (bitcasts)
        if (!CondCall->getCalledFunction())
          continue;

        StringRef Name = CondCall->getCalledFunction()->getName();
        Value *AllZOp = CondCall->getOperand(0);
        Type *AllZOpType = AllZOp->getType();

        // So, we do not count the ancestor if it's all1/all0, has a vector
        // type, and is data dependent. Not counting penalizes this block,
        // because the less blocks you're control-dependent on, the higher
        // your probability of being executed is. The reason data dependence
        // is taken into account is that an allZero call that does not depend
        // on data is a pretty weird beast. There are two cases: a) The branch
        // is a guard
        // ("if (x > width)") b) Different workitems perform different
        // computations, according to their GID. We believe case (a) is more
        // common. There is no reason to punish guards, since the logic of
        // "all workitems must agree for the block to be skipped, which is
        // rarer then a single workitem satisfying the condition" no longer
        // applies.
        if (Predicator::isAllZero(Name) && AllZOpType->isVectorTy() &&
            (DepSet.find(CondCall) != DepSet.end()))
          Count--;

        // A degenerate case - an allZero(true) branch is never taken.
        // If you depend on one of those, ignore this dependency.
        ConstantInt *ConstOp = dyn_cast<ConstantInt>(AllZOp);
        if (Predicator::isAllZero(Name) && ConstOp && ConstOp->isOne())
          Count--;
      }
    }
    ProbMap[&BB] = 1.0 / (pow(2.0, Count));
  }

  // We want to ensure that the allones optimization
  // won't cause noises in the heuristics decision.
  // (that is, will return the same result as without running the allones
  // optimization.)
  for (auto &Pair : AllOnesToEntryBlock) {
    if (Predicator::getAllOnesBlockType(Pair.second) ==
        Predicator::SINGLE_BLOCK_LOOP_ENTRY) {
      ProbMap[Pair.first] = ProbMap[Pair.second] / 2;
      ProbMap[Pair.second] = 0;
    } else {
      ProbMap[Pair.first] = ProbMap[Pair.second];
    }
  }
}

void InstCountResultImpl::estimateMemOpCosts(
    Function &F, DenseMap<Instruction *, int> &CostMap) const {
  LLVM_DEBUG(dbgs() << "Estimate MemOp Cost for : ");
  LLVM_DEBUG(dbgs() << F.getName() << "\n");

  std::vector<Instruction *> TIDUsers;
  std::vector<Instruction *> Muls;

  std::vector<Instruction *> CheapGEP;
  std::vector<Instruction *> ExpensiveGEP;

  DenseSet<Instruction *> Visited;

  // The idea is that some access patterns are good for vectorization and some
  // are bad. What we try to do here is look at each memory access, and decide
  // if vectorization will help or hurt it. However, looking at the actual
  // pattern is hard so each operation is looked at in isolation, and some
  // very ugly heuristics is used. We classify each access as "cheap" or
  // "expensive". An access is "cheap" if we expect the same workitem to
  // access several adjacent locations. It's "expensive" if we expect the
  // pointers to be consecutive in the workitem dimension. The basic heuristic
  // is that if the access is ptr[j] where j depends on the TID, then: 1) if j
  // is some multiple of the TID, then the kernel is probably doing either
  // single-cell or row accesses, so the op is "cheap" 2) if j is not a
  // multiple of the TID, then the kernel is probably doing column accesses,
  // so the op is expensive. That is, if the accessed location if K+id where K
  // is uniform, then we expect accesses to be consecutive. (Using WIAnalisys
  // would be better, but we can't use it here, since this is
  // pre-scalarization) The above is only true for GEPs where the TID is the
  // last index. However, if the ID is not the last index, then your accesses
  // are in fact of the form (K+id*L)+M so after vectorization they are
  // strided (not good), but before vectorization, if you're accessing several
  // fields of a struct (with different M) this is in fact "cheap", even when
  // not in a loop.

  // First, find all the TID generators.
  for (auto &I : instructions(&F)) {
    if (CallInst *CI = dyn_cast<CallInst>(&I)) {
      if (!CI->getCalledFunction())
        // skip indirect calls
        continue;
      StringRef Name = CI->getCalledFunction()->getName();
      // get_group_id() is not a TID generator, but plays the same role here.
      bool IsTidGen;
      unsigned Dim;
      std::tie(IsTidGen, Dim) = isTIDGenerator(CI);
      if (IsTidGen || isGetGroupId(Name))
        addUsersToWorklist(CI, Visited, TIDUsers);
    }
  }

  // Now run a DFS from each TID generator. We want to find two kinds of
  // thing: a) GEP instructions that use the generator. b) MUL/SHL
  // instructions that use it.
  while (!TIDUsers.empty()) {
    Instruction *I = TIDUsers.back();
    TIDUsers.pop_back();
    Visited.insert(I);
    if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(I)) {
      // Check which index is TID-dependent. If it's the last one this is
      // expensive, otherwise, it's cheap. However, accesses to the local
      // (addrspace 3) addresses are never expensive. Another possible
      // exception is that wide accesses are never expensive. e.g. use
      // something like: int RetSize =
      // GEP->getType()->getElementType()->getPrimitiveSizeInBits(); and then
      // check whether (RetSize < 128). Not in right now since it doesn't
      // appear to be helpful.

      Value *LastOp = GEP->getOperand(GEP->getNumOperands() - 1);
      Instruction *LastOpInst = dyn_cast<Instruction>(LastOp);
      // If this is not in a loop, we don't care.
      if (!(LI.getLoopFor(GEP->getParent())))
        continue;

      if (LastOpInst && (Visited.find(LastOpInst) != Visited.end()) &&
          (GEP->getPointerAddressSpace() != 3))
        ExpensiveGEP.push_back(GEP);
      else
        CheapGEP.push_back(GEP);
    } else if (BinaryOperator *BinOp = dyn_cast<BinaryOperator>(I)) {
      if ((BinOp->getOpcode() == Instruction::Mul) ||
          (BinOp->getOpcode() == Instruction::FMul) ||
          (BinOp->getOpcode() == Instruction::Shl)) {
        Muls.push_back(BinOp);
        // We shouldn't look for uses here, will be done later.
        continue;
      }
    } else if (isa<LoadInst>(I) || isa<StoreInst>(I) || isa<CallInst>(I))
      // Don't propagate through loads/stores/calls.
      continue;

    addUsersToWorklist(I, Visited, TIDUsers);
  }

  // Ok, now we have the MULs and SHLs
  // Find the GEPs that depend on them, and mark as cheap.
  Visited.clear();
  while (!Muls.empty()) {
    Instruction *I = Muls.back();
    Muls.pop_back();
    Visited.insert(I);
    if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(I))
      CheapGEP.push_back(GEP);

    if (!isa<LoadInst>(I) && !isa<StoreInst>(I) && !isa<CallInst>(I))
      addUsersToWorklist(I, Visited, Muls);
  }

  // Now, actually fill in the cost maps.
  // The user of a GEP should normally be either a load, a store, or a call
  // (such as vload/vstore) You wouldn't expect them to propagate through
  // anything, but in fact, phi nodes happen. Could run a DFS, instead support
  // one level of phi. Mark the expensive ones first, since if something is
  // reachable through both types of paths, it should be cheap.
  //
  // FIXME:
  // Ahh!!! A GEP can propagate through bitcast, extractelement, etc.
  // Besides, the costs of masked_load, masked_store, masked_gather,
  // masked_scatter, vload and vstore are estimated here by 'GEP', but they're
  // not used at all. Why do we estimate them using anothor method in
  // estimateCall()?
  for (auto *I : ExpensiveGEP) {
    for (User *U : I->users()) {
      if (Instruction *User = dyn_cast<Instruction>(U))
        CostMap[User] = ExpensiveMemOpWeight;

      LLVM_DEBUG(dbgs() << "Expensive: ");
      LLVM_DEBUG(U->dump());
    }
  }

  for (auto *I : CheapGEP) {
    for (User *U : I->users()) {
      if (auto *User = dyn_cast<Instruction>(U))
        CostMap[User] = CheapMemOpWeight;
      LLVM_DEBUG(dbgs() << "Cheap: ");
      LLVM_DEBUG(U->dump());
    }
  }
}

void InstCountResultImpl::addUsersToWorklist(
    Instruction *I, DenseSet<Instruction *> &Visited,
    std::vector<Instruction *> &WorkList) const {
  // Find all users, add them to the worklist if they haven't been visited yet
  for (User *U : I->users()) {
    if (Instruction *User = dyn_cast<Instruction>(U))
      if (Visited.find(User) == Visited.end())
        WorkList.push_back(User);
  }
}

void InstCountResultImpl::estimateDataDependence(
    Function &F, DenseSet<Instruction *> &DepSet) const {
  // Finds every instruction that depends on loaded data.

  // TODO: Add image reads?
  std::vector<Instruction *> DataUsers;

  // First, find all GEP instructions.
  // This used to be LoadInst but was changed to GEP.
  // LoadInst seems to make intuitive sense, but in fact also counts loads
  // from allocas. Why would there even be allocas at this stage? Because of
  // soa builtins that return results through local pointers. Oops.
  for (auto &I : instructions(&F))
    if (isa<GetElementPtrInst>(&I))
      DataUsers.push_back(&I);

  // Now run a DFS from each GEP, and mark all its (indirect) users.
  while (!DataUsers.empty()) {
    Instruction *I = DataUsers.back();
    DataUsers.pop_back();
    DepSet.insert(I);
    addUsersToWorklist(I, DepSet, DataUsers);
  }
}

void InstCountResultImpl::copyBlockCosts(std::map<BasicBlock *, int> *Dest) {
  DPCPP_STAT_GATHER_CHECK(Dest->insert(BlockCosts.begin(), BlockCosts.end()););
}

void InstCountResultImpl::countPerBlockHeuristics(
    std::map<BasicBlock *, int> *PreCosts, int PacketWidth) {
  // this method is just for statistical purposes.
  DPCPP_STAT_GATHER_CHECK(
      DPCPPStatistic::ActiveStatsT kernelStats;
      int vectorizedVersionIsBetter = 0; int scalarVersionIsBetter = 0;
      for (auto &Pair : *PreCosts) {
        // Do not try to access to the object 'BB' points,
        // the pointer may be no longer valid
        BasicBlock *BB = Pair.first;
        int scalarVersionWeight = Pair.second;
        if (!BlockCosts.count(BB)) // no weight for vectorized version.
          continue;
        int vectorizedVersionWeight = BlockCosts[BB];
        if (vectorizedVersionWeight > scalarVersionWeight * PacketWidth)
          scalarVersionIsBetter++;
        else
          vectorizedVersionIsBetter++;
      } DPCPP_STAT_DEFINE(Blocks_That_Are_Better_Vectorized,
                          "blocks for which the heuristics says it is better "
                          "to vectorize",
                          kernelStats);
      Blocks_That_Are_Better_Vectorized = vectorizedVersionIsBetter;
      DPCPP_STAT_DEFINE(Blocks_That_Are_Better_Scalarized,
                        "blocks for which the heuristics says it is better to "
                        "leave scalar version",
                        kernelStats);
      Blocks_That_Are_Better_Scalarized = scalarVersionIsBetter;
      DPCPPStatistic::pushFunctionStats(kernelStats, F, DEBUG_TYPE););
}

void InstCountResultImpl::print(raw_ostream &OS) {
  OS << "InstCountResult for function " << F.getName() << ":\n";
  OS.indent(2) << "Desired VF: " << DesiredVF << "\n";
  OS.indent(2) << "Total weight : " << TotalWeight << "\n";
}

InstCountResult::InstCountResult(Function &F, TargetTransformInfo &TTI,
                                 PostDominatorTree &DT, LoopInfo &LI,
                                 ScalarEvolution &SE,
                                 VFISAKind ISA, bool PreVec) {
  Impl.reset(new InstCountResultImpl(F, TTI, DT, LI, SE, ISA, PreVec));
}

InstCountResult::InstCountResult(InstCountResult &&Other)
    : Impl(std::move(Other.Impl)) {}

InstCountResult::~InstCountResult() = default;

void InstCountResult::print(raw_ostream &OS) { return Impl->print(OS); }

unsigned InstCountResult::getDesiredVF() const { return Impl->getDesiredVF(); }

float InstCountResult::getWeight() const { return Impl->getWeight(); }

float InstCountResult::getBBProb(const BasicBlock *BB) const {
  return Impl->getBBProb(BB);
}

void InstCountResult::countPerBlockHeuristics(
    std::map<BasicBlock *, int> *PreCosts, int PacketWidth) {
  Impl->countPerBlockHeuristics(PreCosts, PacketWidth);
}

void InstCountResult::copyBlockCosts(std::map<BasicBlock *, int> *Dest) {
  Impl->copyBlockCosts(Dest);
}

INITIALIZE_PASS_BEGIN(WeightedInstCountAnalysisLegacy, DEBUG_TYPE,
                      "Weighted instruction count analysis", false, true)
INITIALIZE_PASS_DEPENDENCY(PostDominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolutionWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_END(WeightedInstCountAnalysisLegacy, DEBUG_TYPE,
                    "Weighted instruction count analysis", false, true)

char WeightedInstCountAnalysisLegacy::ID = 0;

WeightedInstCountAnalysisLegacy::WeightedInstCountAnalysisLegacy(
    VFISAKind ISA, bool PreVec)
    : FunctionPass(ID), ISA(ISA), PreVec(PreVec) {
  initializeWeightedInstCountAnalysisLegacyPass(
      *PassRegistry::getPassRegistry());
}

void WeightedInstCountAnalysisLegacy::getAnalysisUsage(
    AnalysisUsage &AU) const {
  AU.addRequiredTransitive<TargetTransformInfoWrapperPass>();
  AU.addRequired<PostDominatorTreeWrapperPass>();
  AU.addRequired<LoopInfoWrapperPass>();
  AU.addRequired<ScalarEvolutionWrapperPass>();
  AU.setPreservesAll();
}

bool WeightedInstCountAnalysisLegacy::runOnFunction(Function &F) {
  TargetTransformInfo &TTI =
      getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F);
  PostDominatorTree &DT =
      getAnalysis<PostDominatorTreeWrapperPass>().getPostDomTree();
  LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  ScalarEvolution &SE = getAnalysis<ScalarEvolutionWrapperPass>().getSE();
  Result.reset(new InstCountResult(F, TTI, DT, LI, SE, ISA, PreVec));
  return false;
}

FunctionPass *
llvm::createWeightedInstCountAnalysisLegacyPass(VFISAKind ISA,
                                                bool PreVec) {
  return new WeightedInstCountAnalysisLegacy(ISA, PreVec);
}

AnalysisKey WeightedInstCountAnalysis::Key;

InstCountResult WeightedInstCountAnalysis::run(Function &F,
                                               FunctionAnalysisManager &AM) {
  TargetTransformInfo &TTI = AM.getResult<TargetIRAnalysis>(F);
  PostDominatorTree &DT = AM.getResult<PostDominatorTreeAnalysis>(F);
  LoopInfo &LI = AM.getResult<LoopAnalysis>(F);
  ScalarEvolution &SE = AM.getResult<ScalarEvolutionAnalysis>(F);
  return InstCountResult{F, TTI, DT, LI, SE, ISA, PreVec};
}
