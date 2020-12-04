// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#define DEBUG_TYPE "InstCounter"

#include "InstCounter.h"
#include "WIAnalysis.h"
#include "Predicator.h"
#include "Mangler.h"
#include "LoopUtils/LoopUtils.h"
#include "OpenclRuntime.h"
#include "OCLPassSupport.h"
#include "InitializePasses.h"
#include "CompilationUtils.h"
#include "OclTune.h"
#include "MetadataAPI.h"

#include "llvm/InitializePasses.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/DominanceFrontier.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"

#include <iomanip>
#include <sstream>

using namespace Intel::OpenCL::DeviceBackend;

namespace intel {

char WeightedInstCounter::ID = 0;

OCL_INITIALIZE_PASS_BEGIN(WeightedInstCounter, "winstcounter", "Weighted Instruction Counter", false, false)
OCL_INITIALIZE_PASS_DEPENDENCY(ScalarEvolutionWrapperPass)
OCL_INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
OCL_INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
OCL_INITIALIZE_PASS_DEPENDENCY(PostDominatorTreeWrapperPass)
OCL_INITIALIZE_PASS_DEPENDENCY(PostDominanceFrontier)
OCL_INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfo)
OCL_INITIALIZE_PASS_END(WeightedInstCounter, "winstcounter", "Weighted Instruction Counter", false, false)

char VectorizationPossibilityPass::ID = 0;

OCL_INITIALIZE_PASS_BEGIN(VectorizationPossibilityPass, "vectorpossible", "Check whether vectorization is possible", false, false)
OCL_INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
OCL_INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfo)
OCL_INITIALIZE_PASS_END(VectorizationPossibilityPass, "vectorpossible", "Check whether vectorization is possible", false, false)


const float WeightedInstCounter::RATIO_MULTIPLIER = 0.98f;
const float WeightedInstCounter::ALL_ZERO_LOOP_PENALTY = 0;
const float WeightedInstCounter::TID_EQUALITY_PENALTY = 0.1f;
const int PENALTY_FACTOR_FOR_GEP_WITH_SIX_PARAMETERS = 7;
const unsigned int NUMBER_OF_PARAMETERS_IN_GEP_PENALTY_HACK = 6;


// Costs for transpose functions for 64bit systems
WeightedInstCounter::FuncCostEntry WeightedInstCounter::CostDB64Bit[] = {
   { "__ocl_load_transpose_char_4x4", 8 },
   { "__ocl_transpose_store_char_4x4", 8 },
   { "__ocl_masked_load_transpose_char_4x4", 12 },
   { "__ocl_masked_transpose_store_char_4x4", 12 },
   { "__ocl_gather_transpose_float_4x4", 200 },
   { "__ocl_transpose_scatter_float_4x4", 200 },
   { "__ocl_load_transpose_float_4x8", 70 },
   { "__ocl_transpose_store_float_4x8", 70 },
   { "__ocl_gather_transpose_float_4x8", 200 },
   { "__ocl_transpose_scatter_float_4x8", 200 },
   { "__ocl_masked_load_transpose_float_4x8", 80},
   { "__ocl_masked_transpose_store_float_4x8", 80},
   { "__ocl_masked_gather_transpose_float_4x8", 200},
   { "__ocl_masked_transpose_scatter_float_4x8", 200},
   { "__ocl_load_transpose_char_4x16", 70 },
   { "__ocl_gather_transpose_char_4x16", 150 },
   { "__ocl_transpose_scatter_char_4x16", 150 },
   { "__ocl_masked_gather_transpose_char_4x16", 200 },
   { "__ocl_masked_transpose_scatter_char_4x16", 200 },
   { "__ocl_gather_transpose_short_4x16", 150 },
   { "__ocl_masked_gather_transpose_short_4x16", 200 },
   { "__ocl_load_transpose_int_4x16", 70 },
   { "__ocl_load_transpose_float_4x16", 70 },
   { "__ocl_masked_load_transpose_float_4x16", 80 },

   // The line below must be the last line in the DB,
   // serving as a terminator.
   { 0, 0 }
};

// Costs for transpose functions for 32bit systems
WeightedInstCounter::FuncCostEntry WeightedInstCounter::CostDB32Bit[] = {

   { "__ocl_load_transpose_char_4x4", 8 },
   { "__ocl_transpose_store_char_4x4", 8 },
   { "__ocl_masked_load_transpose_char_4x4", 12 },
   { "__ocl_masked_transpose_store_char_4x4", 12 },
   { "__ocl_load_transpose_float_4x8", 70 },
   { "__ocl_transpose_store_float_4x8", 70 },
   { "__ocl_gather_transpose_float_4x8", 75 },
   { "__ocl_transpose_scatter_float_4x8", 75 },
   { "__ocl_masked_load_transpose_float_4x8", 80},
   { "__ocl_masked_transpose_store_float_4x8", 80},
   { "__ocl_masked_gather_transpose_float_4x8", 90},
   { "__ocl_masked_transpose_scatter_float_4x8", 90},
   { "__ocl_load_transpose_char_4x16", 70 },
   { "__ocl_gather_transpose_char_4x16", 75 },
   { "__ocl_transpose_scatter_char_4x16", 80 },
   { "__ocl_masked_gather_transpose_char_4x16", 90 },
   { "__ocl_masked_transpose_scatter_char_4x16", 90 },
   { "__ocl_gather_transpose_short_4x16", 80 },
   { "__ocl_masked_gather_transpose_short_4x16", 90 },
   { "__ocl_load_transpose_int_4x16", 70 },
   { "__ocl_load_transpose_float_4x16", 70 },
   { "__ocl_masked_load_transpose_float_4x16", 80 },

   // The line below must be the last line in the DB,
   // serving as a terminator.
   { 0, 0 }
};

WeightedInstCounter::FuncCostEntry* WeightedInstCounter::getCostDB() const {
  if(is64BitArch()) return CostDB64Bit;
  return CostDB32Bit;
}

bool WeightedInstCounter::is64BitArch() const {
  return m_cpuid.Is64BitOS();
}

bool WeightedInstCounter::hasAVX512() const {
  return m_cpuid.HasAVX512Core();
}

bool WeightedInstCounter::hasAVX() const {
  return m_cpuid.HasAVX1();
}

bool WeightedInstCounter::hasAVX2() const {
  return m_cpuid.HasAVX2();
}


WeightedInstCounter::WeightedInstCounter(bool preVec, Intel::CPUId cpuId):
                              FunctionPass(ID), m_cpuid(cpuId), m_preVec(preVec),
                              m_desiredWidth(1), m_totalWeight(0) {
  initializeWeightedInstCounterPass(*PassRegistry::getPassRegistry());

  int i = 0;
  FuncCostEntry* costDB = getCostDB();
  while ((costDB[i]).name) {
    const FuncCostEntry* e = &(costDB[i]);
    m_transCosts[e->name] = e->cost;
    i++;
  }
}

bool WeightedInstCounter::runOnFunction(Function &F) {

  // for statistics:
  OCLSTAT_GATHER_CHECK(
    m_blockCosts.clear();
  );

  //This is for safety - don't return 0.
  m_totalWeight = 1;

  // Check if this is the "pre" stage.
  // If it is, compute things that are relevant only here.
  DenseMap<Instruction*, int> MemOpCostMap;
  if (m_preVec) {
    // Compute the "cost map" for stores and loads. This is only
    // done pre-vectorization. See function for extended explanation.
    estimateMemOpCosts(F, MemOpCostMap);
  }

  // First, estimate the total number of iterations each loop in the
  // functions runs. Once we have this, we can multiply the count
  // by each instruction's weight.
  DenseMap<Loop*, int> IterMap;
  estimateIterations(IterMap);

  // Now compute some estimation of the probability of each basic block
  // being executed in a run.
  DenseMap<BasicBlock*, float> ProbMap;
  estimateProbability(F, ProbMap);

  // Ok, start counting with 0
  m_totalWeight = 0;

  LoopInfo *LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  // For each basic block, add up its weight
  LLVM_DEBUG(dbgs() << "********************\nCalculating cost for function "
                    << F.getName() << '\n');
  for (Function::iterator BBIter = F.begin(), BBEndIter = F.end(); BBIter != BBEndIter; ++BBIter) {

    BasicBlock* const BB = &*BBIter;

    bool discardPhis = false;
    bool discardTerminator = false;
    int blockWeights = 0; // for statistical purposes.

    // Check if BB is an idom of an allOnes branch
    // and if it does discard its phis cost
    Predicator::AllOnesBlockType blockType = Predicator::getAllOnesBlockType(BB);
    if (dyn_cast<PHINode>(BB->begin())) {

      if (blockType == Predicator::EXIT) {
        discardPhis = true;
      }
    }

    if (blockType == Predicator::ALLONES ||
      blockType == Predicator::ORIGINAL) {
        discardTerminator = true;
    }

    // Check if the basic block in a loop. If it is, we want to multiply
    // all of its instruction's weights by its tripcount.
    int TripCount = 1;
    if (Loop* ContainingLoop = LI->getLoopFor(BB))
      TripCount = IterMap.lookup(ContainingLoop);

    float Probability = ProbMap.lookup(BB);

    // And now, sum up all the instructions
    LLVM_DEBUG(dbgs() << "Calculating cost for BB " << BB->getName() << '\n');
    for (BasicBlock::iterator IIter = BB->begin(), IE=BB->end(); IIter != IE; ++IIter){
      Instruction* const I = &*IIter;
      if (discardPhis && dyn_cast<PHINode>(I))
        continue;
      if (discardTerminator &&  I->isTerminator())
        continue;

      int instWeight = getInstructionWeight(I, MemOpCostMap);
      float Cost = Probability * TripCount * instWeight;
      m_totalWeight += Cost;
      LLVM_DEBUG(dbgs() << "  [" << format_decimal(instWeight, 4) << ']');
      LLVM_DEBUG(I->dump());
      blockWeights += instWeight; // for statisical purposes.
    }
    LLVM_DEBUG(dbgs() << "Cost of BB " << BB->getName() << ':'
                      << " Final Cost: "
                      << format("%6.2f", blockWeights * Probability * TripCount)
                      << " Block Weight: " << blockWeights
                      << " Prob: " << format("%4.2f", Probability)
                      << " TripCount: " << TripCount << '\n');
    // for statistics:
    OCLSTAT_GATHER_CHECK(
      m_blockCosts[BB] = blockWeights;
    );
  }
  LLVM_DEBUG(dbgs() << "Cost of function " << F.getName() << ": "
                    << format("%.2f", m_totalWeight) << '\n');

  // If we are pre-vectorization, decide what the vectorization width should be.
  // Note that v16 support was already decided earlier. The reason the code is split
  // is that for the v16 we don't need to compute the various maps,
  // while in this part of the code we want to use them.
  if (m_preVec) {
    m_desiredWidth = getPreferredVectorizationWidth(F, IterMap, ProbMap);

    // Here, ocl_recommended_vector_length is set. This metadata is
    // only used by VPO-VecClone.
    auto vkimd = Intel::MetadataAPI::KernelInternalMetadataAPI(&F);
    vkimd.OclRecommendedVectorLength.set(m_desiredWidth);
  }
  return false;
}

int WeightedInstCounter::getPreferredVectorizationWidth(Function &F, DenseMap<Loop*, int> &IterMap,
      DenseMap<BasicBlock*, float> &ProbMap)
{
  // For SSE, this is always 4.
  if (!hasAVX())
    return 4;

  // For AVX, estimate the most used type in the kernel.
  // Integers have no 8-wide operations on AVX1, so vectorize to 4,
  // otherwise, 8.
  // This logic was inherited from the old heuristic, but the types
  // are computed slightly more rationally now.
  if (!hasAVX2()) {
    auto kimd = Intel::MetadataAPI::KernelInternalMetadataAPI(&F);
    // For AVX, there is only x4 native sub-group implementation.
    if (kimd.KernelHasSubgroups.hasValue() && kimd.KernelHasSubgroups.get())
      return 4;
    Type* DominantType = estimateDominantType(F, IterMap, ProbMap);
    if (DominantType->isIntegerTy())
      return 4;
    else
      return 8;
  }

  // For AVX2, the logical choice would be always 8.
  // For AVX512 the logical choice would be always 16
  // (unless we run into the need of not using zmms on Client like ICC did).
  // Unfortunately, this fails for some corner cases, due to both
  // inherent reasons and compiler deficiencies.
  // The first corner case is <k x i16*> buffers. Since we don't have transpose
  // operations for i16, reading and writing from these buffers becomes
  // expensive.
  for (const auto &Arg : F) {
    Type* argType = Arg.getType();

    // Is this a pointer type?
    PointerType* PtrArgType = dyn_cast<PointerType>(argType);
    if (!PtrArgType)
      continue;

    // Pointer to vector of i16?
    Type* PointedType = PtrArgType->getElementType();
    if (!PointedType->isVectorTy())
      continue;

    if (!PointedType->getScalarType()->isIntegerTy(16))
      continue;

    // Last thing to check - that this buffer is not trivially dead.
    if (Arg.hasNUsesOrMore(1))
      return 4;
  }

  if (!hasAVX512())
    return 8;

  return 16;
}

// This allows a consistent comparison between scalar and vector types.
// Under normal conditions, a pointer comparison always occures, which
// is consistent for a single run, but not between runs.
struct TypeComp {
  bool operator() (Type* Left, Type* Right) const {
    FixedVectorType *VTypeLeft = dyn_cast<FixedVectorType>(Left);
    FixedVectorType *VTypeRight= dyn_cast<FixedVectorType>(Right);

    if( nullptr != VTypeRight && nullptr == VTypeLeft )
        return true;

    if( nullptr == VTypeRight && nullptr != VTypeLeft )
        return false;

    if( nullptr != VTypeLeft && nullptr != VTypeRight)
      if (VTypeLeft->getNumElements() != VTypeRight->getNumElements())
        return (VTypeLeft->getNumElements() < VTypeRight->getNumElements());

    if (Left->getScalarSizeInBits() != Right->getScalarSizeInBits())
      return (Left->getScalarSizeInBits() < Right->getScalarSizeInBits());

    Type* ScalarLeft = Left->getScalarType();
    Type* ScalarRight = Right->getScalarType();

    if (ScalarLeft->isIntegerTy() && !ScalarRight->isIntegerTy())
      return true;

    if (!ScalarLeft->isIntegerTy() && ScalarRight->isIntegerTy())
      return false;

    //Fallback to a pointer comparison for other types.
    return Left < Right;
  }
};

Type* WeightedInstCounter::estimateDominantType(Function &F, DenseMap<Loop*, int> &IterMap,
                                           DenseMap<BasicBlock*, float> &ProbMap) const
{
  DenseMap<Type*, float> countMap;

  // For each type, count how many times it is the first operand of a binop.
  LoopInfo *LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  for (Function::iterator BBIter = F.begin(), BBEndIter = F.end(); BBIter != BBEndIter; ++BBIter) {
    BasicBlock* const BB = &*BBIter;
    int TripCount = 1;
    if (Loop* ContainingLoop = LI->getLoopFor(BB))
      TripCount = IterMap.lookup(ContainingLoop);

    float Probability = ProbMap.lookup(BB);
    for (BasicBlock::iterator I = BB->begin(), IE=BB->end(); I != IE; ++I) {
      // We only care about BinOps
      if (BinaryOperator* BinOp = dyn_cast<BinaryOperator>(I)) {
        Type* OpType = BinOp->getOperand(0)->getType();
        // Get the base type for vectors
        OpType = OpType->getScalarType();
        countMap[OpType] += TripCount * Probability;
      }
    }
  }

  // Find the maximum. The map is expected to be small, just iterate over it.
  // Use i32 by default... if there are no binops at all. Should be pretty rare.
  // In case there are several values with equal maximum keys, use a comparator
  // to choose the maximum.
  // This is needed because otherwise the choice depends on the order of iteration
  // which, in turn, depends on the values of the Type pointers, which change
  // from run to run. This makes the choice inconsistent between runs.
  Type* DominantType = Type::getInt32Ty(F.getContext());
  float MaxCount = 0;
  for (DenseMap<Type*, float>::iterator I = countMap.begin(),
       E = countMap.end(); I != E; ++I) {
    if ((I->second > MaxCount) ||
        (I->second == MaxCount && TypeComp()(I->first, DominantType))) {
      MaxCount = I->second;
      DominantType = I->first;
    }
  }
  return DominantType;
}

int WeightedInstCounter::estimateCall(CallInst *Call)
{
    // TID generators are extremely common and very cheap.
    RuntimeServices* services = getAnalysis<BuiltinLibInfo>().getRuntimeServices();
    bool err = false;
    unsigned dim = 0;
    if(services->isTIDGenerator(Call, &err, &dim))
      return DEFAULT_WEIGHT;

    Function *Callee = Call->getCalledFunction();

    // Handle the special case of unnamed functions (call through a bitcast)
    if (!Callee)
      return CALL_WEIGHT;

    StringRef Name = Callee->getName();
    bool isIntrinsic = Callee->isIntrinsic();

    // For volcano
    bool isOCLLoad = Mangler::isMangledLoad(Name.str());
    bool isOCLStore = Mangler::isMangledStore(Name.str());
    // For VPlan
    bool isLoadIntrinsic =
        isIntrinsic && Callee->getIntrinsicID() == Intrinsic::masked_load;
    bool isStoreIntrinsic =
       isIntrinsic && Callee->getIntrinsicID() == Intrinsic::masked_store;

    // Since we run before the resolver, masked load/stores should count
    // as load/stores, not calls. Maybe slightly better or worse.
    if (isOCLLoad || isOCLStore || isLoadIntrinsic || isStoreIntrinsic) {
      // If the mask is non-scalar, this will become a lot of memops,
      // since the CPU doesn't have gathers.
      unsigned MaskIdx;
      if (isOCLLoad || isOCLStore) {
        MaskIdx = 0;
      } else if (isLoadIntrinsic) {
        MaskIdx = 2;
      } else { // isStoreIntrinsic
        MaskIdx = 3;
      }
      Value *Mask = Call->getArgOperand(MaskIdx);
      Type* MaskType = Mask->getType();

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
      // there's no call to masked_load/maskd_store with this kind of gep at
      // all. This kind of gep's are directly used by 'load' instructions. I
      // suggest removing this piece of code or making it fit in more general
      // cases. E.g., penalize gep's with more than 6 operands instead of exact
      // 6 operands.
      if ((isOCLStore || isStoreIntrinsic) && !MaskType->isVectorTy()) {
        V_ASSERT(Call->getNumArgOperands() == 3 && "expected 3 params in masked store");
        if (auto * gep = dyn_cast<GetElementPtrInst>(Call->getArgOperand(1))) {
          if (gep->getNumOperands() == NUMBER_OF_PARAMETERS_IN_GEP_PENALTY_HACK) {
            BasicBlock* BB = Call->getParent();
            for (succ_iterator it = succ_begin(BB), e = succ_end(BB);
              it != e; ++it) {
              if (*it == BB) {
                return MEM_OP_WEIGHT * PENALTY_FACTOR_FOR_GEP_WITH_SIX_PARAMETERS;
              }
            }
          }
        }
      }

      // For scalar masks, it'll be a pretty little vector store/load
      if (!MaskType->isVectorTy())
        return MEM_OP_WEIGHT;

       // This is a vector type, it'll be ugly.
      int NumElements = cast<FixedVectorType>(MaskType)->getNumElements();
      return MEM_OP_WEIGHT * NumElements;
      // TODO: if the vector is really large, still need to multiply...
    }

    // For volcano
    bool isOCLGather = Mangler::isMangledGather(Name.str());
    bool isOCLScatter = Mangler::isMangledScatter(Name.str());
    // For VPlan
    bool isGatherIntrinsic =
        isIntrinsic && Callee->getIntrinsicID() == Intrinsic::masked_gather;
    bool isScatterIntrinsic =
       isIntrinsic && Callee->getIntrinsicID() == Intrinsic::masked_scatter;

    if (isOCLGather || isOCLScatter ||
        isGatherIntrinsic || isScatterIntrinsic) {
      // 16 x 32bit element gather/scatter with 64 bit indices will turn
      // into 2 gathers/scatters, so adjust weight accordingly.

      int Weight;
      if (isOCLGather || isGatherIntrinsic)
        Weight = GATHER_WEIGHT;
      else
        Weight = SCATTER_WEIGHT;

      // FIXME: For Volcano-style ocl_masked_gather/scatter, the address is
      // calculated by ptr + indices, so we can check if the indices are in 64
      // bits; while LLVM gather/scatter intrinsic simply accepts a vector of
      // address, so we simply return the Weight.
      if (isGatherIntrinsic || isScatterIntrinsic)
        return Weight;

      Value *Index = Call->getArgOperand(2);
      auto Ty = cast<VectorType>(Index->getType());
      if (Ty->getScalarSizeInBits() > 32)
        // return doubled weight for 64 bit indices
        return 2 * Weight;
      else
        return Weight;
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
    //     %loaded = call <3 x float> @_Z6vload3mPU3AS1Kf(i64 0, float addrspace(1)* %ptr)
    //     br %postload
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
      return MEM_OP_WEIGHT;
    }

    // Ugly hacks start here.
    // FIXME: What about masked versions?
    if (Name.startswith("_Z5clamp") || Name.startswith("clamp"))
      return CALL_CLAMP_WEIGHT;

    if (Name.startswith("_Z5floor") || Name.startswith("floor"))
      return CALL_FLOOR_WEIGHT;

    if (Name.startswith("_Z3min") || Name.startswith("min") ||
       Name.startswith("_Z3max") || Name.startswith("max"))
      return CALL_MINMAX_WEIGHT;

    if (Name.startswith("fake.insert") || Name.startswith("fake.extract"))
      return CALL_FAKE_INSERT_EXTRACT_WEIGHT;


    // allZero are cheap, it's basically a xor/ptest
    // allone we do not count at all, since we don't want allone-bypasses
    // to effect the result of the heuristics.
    if (Mangler::isAllZero(std::string(Name)))
      return DEFAULT_WEIGHT;

    if (Mangler::isAllOne(std::string(Name)))
      return NOOP_WEIGHT;

    // See if we can find the function in the function cost table
    return getFuncCost(std::string(Name));
}

int WeightedInstCounter::getFuncCost(const std::string& name)
{
  if (m_transCosts.find(name) != m_transCosts.end()) {
    return m_transCosts[name];
  } else {
    // Function is not in the table, return default call weight,
    // except that mangled (masked) calls are more expensive by default
    if (Mangler::isMangledCall(name))
      return CALL_WEIGHT + CALL_MASK_WEIGHT;

    return CALL_WEIGHT;
  }
}

int WeightedInstCounter::estimateBinOp(BinaryOperator *I)
{
  int Weight = BINARY_OP_WEIGHT;
  Type* OpType = I->getOperand(0)->getType();

  // If it's a scalar op, return the base weight.
  if (!OpType->isVectorTy())
    return Weight;

  FixedVectorType* VecType = cast<FixedVectorType>(OpType);
  unsigned OpWidth = getOpWidth(VecType);

  return Weight * OpWidth;
}

unsigned WeightedInstCounter::getOpWidth(FixedVectorType* VecType) const
{
  Type* BaseType = VecType->getScalarType();
  unsigned BaseWidth = VecType->getScalarSizeInBits();
  unsigned ElemCount = VecType->getNumElements();

  unsigned Float, Double, LongInt, ShortInt;
  if (hasAVX512()) {
    Float = 16;
    Double = 8;
    LongInt = 8;
    ShortInt = 16;
  } else if (hasAVX2()) {
    Float = 8;
    Double = 4;
    LongInt = 4;
    ShortInt = 8;
  } else if (hasAVX()) {
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

  //ceil_div(x,y) is ceil(x/y) for positive integers.
  if (BaseType->isFloatingPointTy())
  {
    if (BaseType->isFloatTy())
      return ceil_div(ElemCount, Float);
    else
      return ceil_div(ElemCount, Double);
  }
  else if (BaseWidth > 32)
    return ceil_div(ElemCount, LongInt);
  else
    return ceil_div(ElemCount, ShortInt);
}

int WeightedInstCounter::getInstructionWeight(Instruction *I, DenseMap<Instruction*, int> &MemOpCostMap)
{
  // We could replace this by a switch on the opcode, but that introduces
  // a bit too many cases. So using this method (also used in WIAnalysis).
  // TODO: A lot of cases have been introduced as is, so, perhaps replace.
  if (BinaryOperator* BinOp = dyn_cast<BinaryOperator>(I))
    return estimateBinOp(BinOp);

  if (CallInst* called = dyn_cast<CallInst>(I))
    return estimateCall(called);

  // GEP and PHI nodes are free
  // NOTE: In the GEP case this is it not entirely true because it may result in LEA
  if (isa<GetElementPtrInst>(I) || isa<PHINode>(I) || isa<AllocaInst>(I) || isa<BitCastInst>(I) || isa<AddrSpaceCastInst>(I))
    return NOOP_WEIGHT;

  // Shuffles/extracts/inserts are mostly representative
  // of transposes.
  if (isa<ShuffleVectorInst>(I)) {
    // Shuffling from 4xi32, 8xi32, 4xfloat and 8xfloat is cheap,
    // shuffling 16xi32, 16xfloat also should be cheap on AVX-512.
    // everything else is expensive.
    // (This is purely empirical, probably overfitting)
    Value* Vec = I->getOperand(0);
    FixedVectorType* OpType = dyn_cast<FixedVectorType>(Vec->getType());
    VectorType* ResType = dyn_cast<VectorType>(I->getType());
    assert(OpType && "Shuffle with a non-vector type!");

    ArrayRef<int> Mask = cast<ShuffleVectorInst>(I)->getShuffleMask();
    // Check whether this shuffle is a part of a broadcast sequence.
    // If it is, the price is 0, since we already paid for the insert.
    if (all_of(Mask, [](int Elt) { return Elt == 0; }))
      return BROADCAST_WEIGHT;

    unsigned NumSpilt = getOpWidth(OpType);

    // A shuffle between different types won't be cheap, even if
    // the types are sensible. This should, amongst other things,
    // make revectorization from 4 to 8 less appealing.
    if (ResType != OpType)
      return EXPENSIVE_SHUFFLE_WEIGHT * NumSpilt;

    if ((OpType->getElementType()->isFloatTy()) ||
         OpType->getElementType()->isIntegerTy(32))
      return CHEAP_SHUFFLE_WEIGHT * NumSpilt;

    return EXPENSIVE_SHUFFLE_WEIGHT * NumSpilt;
  }

  if (isa<ExtractElementInst>(I)) {
    //Same logic as for shuffles.
    Value* Vec = I->getOperand(0);
    FixedVectorType* OpType = dyn_cast<FixedVectorType>(Vec->getType());
    assert(OpType && "Extract from a non-vector type!");

    if (((OpType->getNumElements() == 4) ||
         (OpType->getNumElements() == 8) ||
         ((hasAVX512()) && (OpType->getNumElements() == 16)))
       &&
        ((OpType->getElementType()->isFloatTy()) ||
          OpType->getElementType()->isIntegerTy(32)))
      return CHEAP_EXTRACT_WEIGHT;

    return EXPENSIVE_EXTRACT_WEIGHT;
  }

  if (InsertElementInst * insertInst = dyn_cast<InsertElementInst>(I)) {
    // Broadcast is a compound of two instructions and is way cheaper
    // than an insert by itself.
    // Example:
    //   %ins = insertelement <8 x i32> undef, float %val, i32 0
    //   %bro = shufflevector <8 x i32> %ins, <8 x i32> undef, <8 x i32> zeroinitializer
    // So if this insert is a part of a broadcast then simply don't count it.
    // The broadcast weight will be counted at ShuffleVectorInst.
    if (insertInst->hasOneUse()) {
      User * userInst = insertInst->user_back();
      if(ShuffleVectorInst * shuffleInst = dyn_cast<ShuffleVectorInst>(userInst)) {
        if (all_of(shuffleInst->getShuffleMask(),
                   [](int Elt) { return Elt == 0; }))
          return NOOP_WEIGHT;
      }
    }

    return INSERT_WEIGHT;
  }


  // We can't take spilling into account at this point, so counting loads
  // and stores may be voodoo magic.
  // Still, it works.
  if (isa<LoadInst>(I) || isa<StoreInst>(I)) {
    // Look up precomputed "cheap"/"expensive" ops
    DenseMap<Instruction*, int>::iterator OpCost = MemOpCostMap.find(I);
    if (OpCost != MemOpCostMap.end())
      return OpCost->second;

    // Not known to be special, return normal weight.
    return MEM_OP_WEIGHT;
  }

  // Conditional branches are expensive.
  // This has two reasons - a direct one (misprediction)
  // and an indirect one (to punish complex control flow).
  if (BranchInst* BR = dyn_cast<BranchInst>(I))
    if (BR->isConditional()) {
      // we do not count allones branches, because we do not
      // want the allones optimization to change heuristic results
      // of kernels.
      Value* Cond = BR->getCondition();
      CallInst* CondCall = dyn_cast<CallInst>(Cond);
      if (CondCall && CondCall->getCalledFunction()) {
        StringRef Name = CondCall->getCalledFunction()->getName();
        if (Mangler::isAllOne(std::string(Name)))
          return NOOP_WEIGHT;
      }

      return COND_BRANCH_WEIGHT;
    }

  // For everything else - use a default weight
  return DEFAULT_WEIGHT;
}

void WeightedInstCounter::estimateIterations(
    DenseMap<Loop*, int> &IterMap) const
{
  // Walk the loop tree, "estimating" the total number of loop
  // iterations for each loop. Since we assume control flow is sane
  // the number of iterations is simply the number of iterations of
  // the current loop multiplied by the computed total number for its
  // parent loop.
  // If the number for the current loop is unknown (because it's not
  // constant, or LoopInfo could not figure it out), we guess it to be
  // LOOP_ITER_GUESS. It may be possible to refine this, but I don't see
  // a good way right now.

  std::vector<Loop*> WorkList;
  LoopInfo *LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  ScalarEvolution *SI = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();

  // Add all the top-level loops to the worklist
  for (LoopInfo::iterator L = LI->begin(), LE = LI->end(); L != LE; ++L)
    WorkList.push_back(*L);

  // Now, walk the loop tree.
  while(!WorkList.empty()) {
    Loop* L = WorkList.back();
    WorkList.pop_back();

    assert(IterMap.find(L) == IterMap.end() &&
      "Looking at same loop twice, loop tree is not a tree!");

    int Multiplier = 1;
    // Is this a top-level loop?
    if (Loop* Parent = L->getParentLoop()) {
      // No, it has a parent, so we want to mulitply by the parents' count
      assert(IterMap.find(Parent) != IterMap.end() &&
        "Parent of loop is not in iteration map!");

      Multiplier = IterMap.lookup(Parent);
    }

    int Count = LOOP_ITER_GUESS;
    BasicBlock* Latch = L->getLoopLatch();
    // cheating heuristics to get the same results when applying allones.
    // this is important when the condition of the latch is uniform.
    // In such a case this loop will never be reached
    // after the allones loop, and getSmallConstantTripCount() returns 0.
    if (Predicator::getAllOnesBlockType(L->getHeader())
      == Predicator::SINGLE_BLOCK_LOOP_ORIGINAL) {
        Latch = Predicator::getAllOnesSingleLoopBlock(L->getHeader());
        Count = SI->getSmallConstantTripCount(LI->getLoopFor(Latch), Latch);
    }
    else if (Latch)
      Count = SI->getSmallConstantTripCount(L);

    // getSmallConstantTripCount() returns 0 for non-constant trip counts
    // and on error conditions. In this case guess and hope for the best.
    if (Count == 0)
      Count = LOOP_ITER_GUESS;

    Count *= Multiplier;

    IterMap[L] = Count;

    // Add all subloops
    for (Loop::iterator Sub = L->begin(), SubE = L->end(); Sub != SubE; ++Sub)
      WorkList.push_back(*Sub);
  }
}

void WeightedInstCounter::
     estimateProbability(Function &F, DenseMap<BasicBlock*, float>
                         &ProbMap) const {

  // What we really ant is a control-dependance graph.
  // Luckily, a node is control-dependent exactly on its postdom
  // frontier.

  PostDominanceFrontier* PDF = &getAnalysis<PostDominanceFrontier>();

  //Debug output
  LLVM_DEBUG(dbgs() << F.getName());
  if (m_preVec)
    LLVM_DEBUG(dbgs() << " Before");
  else
    LLVM_DEBUG(dbgs() << " After");
  LLVM_DEBUG(dbgs() << "\n");
  LLVM_DEBUG(PDF->dump());

  // Check which instructions depend on "data" (results of loads and stores).
  DenseSet<Instruction*> DepSet;
  estimateDataDependence(F, DepSet);

  // We want to make sure the heuristics gets the same result with or without
  // the allones optimization. For this reason, we need to
  // 'cheat' at the probability of some blocks.
  // some blocks need to get the probability of the entry
  // to allones block (which is why we need this map),
  // and some blocks simply need to get a zero probability.
  std::map<BasicBlock*, BasicBlock*> allOnesToEntryBlock;

  // Now do a VERY coarse measurement. The number of nodes on which a block
  // is control-dependent is the number of decision points. For a block to be
  // reached, all decisions need to go "its way".
  // This code makes all sorts of silly assumptions.

  for (Function::iterator BBIter = F.begin(), BBEndIter = F.end(); BBIter != BBEndIter; BBIter++) {
    BasicBlock* const BB = &*BBIter;
    PostDominanceFrontier::iterator iter = PDF->find(BB);
    // It's actually possible that a BB has no PDF (as opposed to an empty one)
    // This happens for infinite loops. If this is the case,
    // we don't really care to evaluate this block.
    if (iter == PDF->end()) {
      ProbMap[BB] = 0;
      continue;
    }

    PostDominanceFrontier::DomSetType &Frontier = iter->second;
    // Before vectorization, the probability is simple 1/(2^k) where k
    // is the number of branches the block is control-dependent on.
    //
    // FIXME:
    // The code below doesn't do what the comments above goes. The k of a basic
    // block is calculated as the number of its post-dominance frontiers, which
    // is absolutely wrong. Given a CFG as follows,
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
    int count = (int)Frontier.size();

    // since we do not want the allones optimization to influence heuristics
    // results (vs. without using allones), we have several specail cases.
    Predicator::AllOnesBlockType blockType = Predicator::getAllOnesBlockType(BB);
    switch (blockType)
    {
    case Predicator::ALLONES : // not counting, this is a duplication of ORIGINAL.
    case Predicator::SINGLE_BLOCK_LOOP_ALLONES : // dup of SINGLE_BLOCK_LOOP_ORIGINAL
    case Predicator::SINGLE_BLOCK_LOOP_ENTRY_TO_ORIGINAL : // overhead of allones
    case Predicator::SINGLE_BLOCK_LOOP_EXIT : // overhead of allones
    case Predicator::SINGLE_BLOCK_LOOP_TEST_ALLZEROES : // overhead of allones
      ProbMap[BB] = 0;
      continue;
    // original is counted without the allones optimization,
    // so we want to count it here as well. However,
    // we need to make sure we give it the same probability
    // as would have been given without the allones optimization.
    // this probability is the on that 'entry' would get
    // (since it has the same incoming edges original previously had).
    case Predicator::ORIGINAL :
      // find entry, and fill Prob at the end.
      allOnesToEntryBlock[BB] = Predicator::getEntryBlockFromOriginal(BB);
      continue;
    // analogoues to original, but probability here
    // should be half of that of the entry (and not identical),
    // because there is another branch (without the allones bypasses version)
    // for this reason, we divide it by 2 at the end.
    case Predicator::SINGLE_BLOCK_LOOP_ORIGINAL :
      // find entry, and fill Prob at the end.
      allOnesToEntryBlock[BB] = Predicator::getEntryBlockFromLoopOriginal(BB);
      continue;
    case Predicator::NONE : // regular block treated normally.
    // ENTRY and EXIT holds part of what is found in ORIGINAL block
    // before the allones bypass (only part of the block is duplicated)
    // so we need to count them normally as well.
    case Predicator::ENTRY :
    case Predicator::EXIT :
    // SINGLE_BLOCK_LOOP_ENTRY should have probability zero (not be counted)
    // but we do that later. First we calculate the probability
    // to be used for the SINGLE_BLOCK_LOOP_ORIGINAL.
    case Predicator::SINGLE_BLOCK_LOOP_ENTRY :
      break;
    default :
      V_ASSERT(false && "unknown type");
      break;
    }

    if (!m_preVec)
    {
      // For each branch we depend on, check what the branch depends on
      for(PostDominanceFrontier::DomSetType::iterator Anc = Frontier.begin(),
          AE = Frontier.end(); Anc != AE; Anc++) {
        // find allZero/allOne conditions, and filter them out.
        BasicBlock *Ancestor = *Anc;
        BranchInst* BR = dyn_cast<BranchInst>(Ancestor->getTerminator());
        if (!BR)
          // The ancestor's terminator isn't a branch, definitely not an allZero one
          continue;

        Value* Cond = BR->getCondition();

        // For VPlan, detect the following pattern of all-zero bypass,
        //   %bc = bitcast <8 x i1> %mask to i8
        //   %az = icmp eq i8 %bc, 0
        //   br i1 %az, label %VPlannedBB.end, label %VPlannedBB
        //
        // We also detect the serialized masked operations, and assume the mask
        // is always 1,
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

          Value *Op0 = Cmp->getOperand(0),
                *Op1 = Cmp->getOperand(1);
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
          if (!MaskTy ||
              MaskTy->getScalarType() !=
                  IntegerType::getInt1Ty(MaskTy->getContext()))
            continue;

          if (DepSet.find(BitCastOrExtract) != DepSet.end())
            count--;
          continue;
        }

        CallInst* CondCall = dyn_cast<CallInst>(Cond);
        if (!CondCall)
          // A branch, but not directly based on a call.
          continue;

        // Ok, so it's a call.
        // After vectorization, we consider dependence on an allZero/allOne
        // condition to be a bad thing. The normal structure for an allZero branch
        // is (A => B, A => C, B => C), where the A => B branch is taken only
        // if all workitems satisfy a condition. Note that block C does not depend
        // on the branch in A(since it's always executed), only B does.
        // Because of the "all workitems" conditions,  it is a good idea to assume
        // that block B is executed more often than a block that depends on a "normal"
        // branch. For loops, the stucture is similar.
        // If it has no operands, it's definitely not allOne/allZero.
        // This is basically just a sanity check, there's no good reason for
        // anyone to branch on the result of a call with no arguments.
        if (CondCall->getNumOperands() < 1)
          continue;

        // Handled unnamed functions (bitcasts)
        if (!CondCall->getCalledFunction())
          continue;

        StringRef Name = CondCall->getCalledFunction()->getName();
        Value* AllZOp = CondCall->getOperand(0);
        Type* AllZOpType = AllZOp->getType();

        // So, we do not count the ancestor if it's all1/all0, has a vector type, and is data
        // dependent. Not counting penalizes this block, because the less blocks you're
        // control-dependent on, the higher your probability of being executed is.
        // The reason data dependence is taken into account is that an allZero call
        // that does not depend on data is a pretty weird beast. There are two cases:
        // a) The branch is a guard ("if (x > width)")
        // b) Different workitems perform different computations, according to their GID.
        // We believe case (a) is more common. There is no reason to punish guards, since
        // the logic of "all workitems must agree for the block to be skipped, which is rarer
        // then a single workitem satisfying the condition" no longer applies.
        if (Mangler::isAllZero(std::string(Name))
          && AllZOpType->isVectorTy()
          && (DepSet.find(CondCall) != DepSet.end()))
            count--;

        // A degenerate case - an allZero(true) branch is never taken.
        // If you depend on one of those, ignore this dependency.
        ConstantInt* ConstOp = dyn_cast<ConstantInt>(AllZOp);
        if (Mangler::isAllZero(std::string(Name)) &&
            ConstOp && ConstOp->isOne())
              count--;
      }
    }
    ProbMap[BB] = 1.0/(pow(2.0, count));
  }

  // We want to ensure that the allones optimization
  // won't cause noises in the heuristics decision.
  // (that is, will return the same result as without running the allones
  // optimization.)
  for (std::map<BasicBlock*,BasicBlock*>::iterator it = allOnesToEntryBlock.begin(),
    e = allOnesToEntryBlock.end(); it != e; ++ it) {
      if (Predicator::getAllOnesBlockType(it->second) ==
        Predicator::SINGLE_BLOCK_LOOP_ENTRY) {
          ProbMap[it->first] = ProbMap[it->second] / 2;
          ProbMap[it->second] = 0;
      }
      else {
         ProbMap[it->first] = ProbMap[it->second];
      }
  }

}

void WeightedInstCounter::estimateMemOpCosts(Function &F, DenseMap<Instruction*, int> &CostMap) const
{
  LLVM_DEBUG(dbgs() << "Estimate MemOp Cost for : ");
  LLVM_DEBUG(dbgs() << F.getName() << "\n");

  std::vector<Instruction*> TIDUsers;
  std::vector<Instruction*> Muls;

  std::vector<Instruction*> CheapGEP;
  std::vector<Instruction*> ExpensiveGEP;

  DenseSet<Instruction*> Visited;

  RuntimeServices* services = getAnalysis<BuiltinLibInfo>().getRuntimeServices();
  assert(services && "Unable to get runtime services");

  // The idea is that some access patterns are good for vectorization and some are bad.
  // What we try to do here is look at each memory access, and decide
  // if vectorization will help or hurt it. However, looking at the actual pattern is hard
  // so each operation is looked at in isolation, and some very ugly heuristics is used.
  // We classify each access as "cheap" or "expensive".
  // An access is "cheap" if we expect the same workitem to access several adjacent locations.
  // It's "expensive" if we expect the pointers to be consecutive in the workitem dimension.
  // The basic heuristic is that if the access is ptr[j] where j depends on the TID, then:
  // 1) if j is some multiple of the TID, then the kernel is probably doing either single-cell
  // or row accesses, so the op is "cheap"
  // 2) if j is not a multiple of the TID, then the kernel is probably doing column accesses,
  // so the op is expensive. That is, if the accessed location if K+id where K is uniform,
  // then we expect accesses to be consecutive.
  // (Using WIAnalisys would be better, but we can't use it here, since this is pre-scalarization)
  // The above is only true for GEPs where the TID is the last index.
  // However, if the ID is not the last index, then your accesses are in fact of the form (K+id*L)+M
  // so after vectorization they are strided (not good), but before vectorization, if you're accessing
  // several fields of a struct (with different M) this is in fact "cheap", even when not in a loop.

  LoopInfo *LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();

  // First, find all the TID generators.
  for (Function::iterator bbit = F.begin(), bbe=F.end(); bbit != bbe; ++bbit) {
    for (BasicBlock::iterator it = bbit->begin(), e=bbit->end(); it!=e;++it) {
      if (CallInst* Call = dyn_cast<CallInst>(it)) {
        bool err = false;
        unsigned dim = 0;
        if (!Call->getCalledFunction())
          // skip indirect calls
          continue;
        StringRef Name = Call->getCalledFunction()->getName();
        // get_group_id() is not a TID generator, but plays the same role here.
        if (services->isTIDGenerator(Call, &err, &dim) || Name.equals("get_group_id"))
        {
          assert(!err && "Should not have variable TID access at this stage");
          addUsersToWorklist(Call, Visited, TIDUsers);
        }
      }
    }
  }

  // Now run a DFS from each TID generator. We want to find two kinds of thing:
  // a) GEP instructions that use the generator.
  // b) MUL/SHL instructions that use it.
  while (!TIDUsers.empty())
  {
    Instruction* I = TIDUsers.back();
    TIDUsers.pop_back();
    Visited.insert(I);
    if (GetElementPtrInst* GEP = dyn_cast<GetElementPtrInst>(I)) {
      // Check which index is TID-dependent. If it's the last one this is expensive,
      // otherwise, it's cheap. However, accesses to the local (addrspace 3) addresses
      // are never expensive.
      // Another possible exception is that wide accesses are never expensive.
      // e.g. use something like:
      // int RetSize = GEP->getType()->getElementType()->getPrimitiveSizeInBits();
      // and then check whether (RetSize < 128).
      // Not in right now since it doesn't appear to be helpful.

      Value* LastOp = GEP->getOperand(GEP->getNumOperands() - 1);
      Instruction* LastOpInst = dyn_cast<Instruction>(LastOp);
      // If this is not in a loop, we don't care.
      if (!(LI->getLoopFor(GEP->getParent())))
        continue;

      if (LastOpInst && (Visited.find(LastOpInst) != Visited.end()) &&
           (GEP->getPointerAddressSpace() != 3))
        ExpensiveGEP.push_back(GEP);
      else
        CheapGEP.push_back(GEP);
    }
    else if (BinaryOperator* BinOp = dyn_cast<BinaryOperator>(I)) {
      if ((BinOp->getOpcode() == Instruction::Mul) ||
          (BinOp->getOpcode() == Instruction::FMul) ||
          (BinOp->getOpcode() == Instruction::Shl)) {
        Muls.push_back(BinOp);
        // We shouldn't look for uses here, will be done later.
        continue;
      }
    }
    else if (isa<LoadInst>(I) || isa<StoreInst>(I) || isa<CallInst>(I))
      // Don't propagate through loads/stores/calls.
      continue;

    addUsersToWorklist(I, Visited, TIDUsers);
  }

  // Ok, now we have the MULs and SHLs
  // Find the GEPs that depend on them, and mark as cheap.
  Visited.clear();
  while(!Muls.empty()) {
    Instruction* I = Muls.back();
    Muls.pop_back();
    Visited.insert(I);
    if (GetElementPtrInst* GEP = dyn_cast<GetElementPtrInst>(I))
        CheapGEP.push_back(GEP);

    if (!isa<LoadInst>(I) && !isa<StoreInst>(I) && !isa<CallInst>(I))
      addUsersToWorklist(I, Visited, Muls);
  }

  // Now, actually fill in the cost maps.
  // The user of a GEP should normally be either a load, a store, or a call (such as vload/vstore)
  // You wouldn't expect them to propagate through anything, but in fact, phi nodes happen.
  // Could run a DFS, instead support one level of phi.
  // Mark the expensive ones first, since if something is reachable through both types of paths,
  // it should be cheap.
  //
  // FIXME:
  // Ahh!!! A gep can propagate through bitcast, extractelement, etc.
  // Besides, the costs of masked_load, masked_store, masked_gather,
  // masked_scatter, vload and vstore are estimated here by 'gep', but they're
  // not used at all. Why do we estimate them using anothor method in
  // estimateCall()?
  for (std::vector<Instruction*>::iterator I = ExpensiveGEP.begin(), E = ExpensiveGEP.end();
    I != E; ++I) {
    for (Instruction::user_iterator U = (*I)->user_begin(), UE = (*I)->user_end();
        U != UE; ++U)
    {
      if (Instruction* User = dyn_cast<Instruction>(*U))
        CostMap[User] = EXPENSIVE_MEMOP_WEIGHT;

      LLVM_DEBUG(dbgs() << "Expensive: ");
      LLVM_DEBUG(U->dump());
    }
  }

  for (std::vector<Instruction*>::iterator I = CheapGEP.begin(), E = CheapGEP.end();
    I != E; I++) {
    for (Instruction::user_iterator U = (*I)->user_begin(), UE = (*I)->user_end();
        U != UE; ++U)
    {
      if (Instruction* User = dyn_cast<Instruction>(*U))
        CostMap[User] = CHEAP_MEMOP_WEIGHT;
      LLVM_DEBUG(dbgs() << "Cheap: ");
      LLVM_DEBUG(U->dump());
    }
  }
}

void WeightedInstCounter::addUsersToWorklist(Instruction *I,
                                DenseSet<Instruction*> &Visited,
                                std::vector<Instruction*> &WorkList) const
{
  // Find all users, add them to the worklist if they haven't been visited yet
  for (Instruction::user_iterator U = I->user_begin(), UE = I->user_end();
       U != UE; U++)
    if (Instruction* User = dyn_cast<Instruction>(*U))
      if (Visited.find(User) == Visited.end())
        WorkList.push_back(User);
}


void WeightedInstCounter::estimateDataDependence(Function &F,
                     DenseSet<Instruction*> &DepSet) const
{
  // Finds every instruction that depends on loaded data.

  // TODO: Add image reads?
  std::vector<Instruction*> DataUsers;

  // First, find all GEP instructions.
  // This used to be LoadInst but was changed to GEP.
  // LoadInst seems to make intuitive sense, but in fact also counts loads from allocas.
  // Why would there even be allocas at this stage? Because of soa builtins that return
  // results through local pointers. Oops.
  for (Function::iterator BBIter = F.begin(), BBEndIter = F.end(); BBIter != BBEndIter; ++BBIter) {
    BasicBlock* const BB = &*BBIter;
    for (BasicBlock::iterator IIter = BB->begin(), IEndIter = BB->end(); IIter != IEndIter; ++IIter) {
      Instruction* const I = &*IIter;
      if (dyn_cast<GetElementPtrInst>(I))
        DataUsers.push_back(I);
    }
  }

  // Now run a DFS from each GEP, and mark all its (indirect) users.
  while (!DataUsers.empty()) {
    Instruction* I = DataUsers.back();
    DataUsers.pop_back();
    DepSet.insert(I);
    addUsersToWorklist(I, DepSet, DataUsers);
  }
}

void WeightedInstCounter::copyBlockCosts(std::map<BasicBlock*,int>* dest) {
  OCLSTAT_GATHER_CHECK(
  dest->insert(m_blockCosts.begin(), m_blockCosts.end());
  );
}

void WeightedInstCounter::countPerBlockHeuristics(
  Function &F, std::map<BasicBlock*, int>* preCosts, int packetWidth) {
  // this method is just for statistical purposes.
  OCLSTAT_GATHER_CHECK(
  Statistic::ActiveStatsT kernelStats;
  int vectorizedVersionIsBetter = 0;
  int scalarVersionIsBetter = 0;
  for (std::map<BasicBlock*, int>::iterator it = preCosts->begin(),
    e = preCosts->end(); it!= e; ++it) {
      // Do not try to access to the object 'BB' points,
      // the pointer may be no longer valid
      BasicBlock* BB = it->first;
      int scalarVersionWeight = it->second;
      if (!m_blockCosts.count(BB)) // no weight for vectorized version.
        continue;
      int vectorizedVersionWeight = m_blockCosts[BB];
      if (vectorizedVersionWeight > scalarVersionWeight * packetWidth)
        scalarVersionIsBetter++;
      else
        vectorizedVersionIsBetter++;
  }
  OCLSTAT_DEFINE(Blocks_That_Are_Better_Vectorized,"blocks for which the heuristics says it is better to vectorize",kernelStats);
  Blocks_That_Are_Better_Vectorized = vectorizedVersionIsBetter;
  OCLSTAT_DEFINE(Blocks_That_Are_Better_Scalarized,"blocks for which the heuristics says it is better to leave scalar version",kernelStats);
  Blocks_That_Are_Better_Scalarized = scalarVersionIsBetter;
  intel::Statistic::pushFunctionStats (kernelStats, F, DEBUG_TYPE);
  );
}


bool VectorizationPossibilityPass::runOnFunction(Function & F)
{
  DominatorTree &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  RuntimeServices* services = getAnalysis<BuiltinLibInfo>().getRuntimeServices();
  m_canVectorize = CanVectorizeImpl::canVectorize(F, DT, services);
  return false;
}

bool CanVectorizeImpl::canVectorizeForVPO(Function &F, RuntimeServices *services)
{
  Statistic::ActiveStatsT kernelStats;
  if (hasVariableGetTIDAccess(F, services)) {
    LLVM_DEBUG(dbgs() << "Variable TID access, can not vectorize\n");
    OCLSTAT_DEFINE(CantVectGIDMess,"Unable to vectorize because get_global_id is messed up",kernelStats);
    CantVectGIDMess++;
    intel::Statistic::pushFunctionStats (kernelStats, F, DEBUG_TYPE);
    return false;
  }

  if (hasNonInlineUnsupportedFunctions(F)) {
    LLVM_DEBUG(dbgs() << "Call to unsupported functions, can not vectorize\n");
    OCLSTAT_DEFINE(CantVectNonInlineUnsupportedFunctions,"Unable to vectorize because of calls to functions that can't be inlined",kernelStats);
    CantVectNonInlineUnsupportedFunctions++;
    intel::Statistic::pushFunctionStats (kernelStats, F, DEBUG_TYPE);
    return false;
  }

  // TODO: Do we need other checks?

  OCLSTAT_DEFINE(CanVect,"Code is vectorizable",kernelStats);
  CanVect++;
  intel::Statistic::pushFunctionStats (kernelStats, F, DEBUG_TYPE);
  return true;
}

bool CanVectorizeImpl::canVectorize(Function &F, DominatorTree &DT, RuntimeServices* services)
{
  Statistic::ActiveStatsT kernelStats;

  if (hasVariableGetTIDAccess(F, services)) {
    LLVM_DEBUG(dbgs() << "Variable TID access, can not vectorize\n");
    OCLSTAT_DEFINE(CantVectGIDMess,"Unable to vectorize because get_global_id is messed up",kernelStats);
    CantVectGIDMess++;
    intel::Statistic::pushFunctionStats (kernelStats, F, DEBUG_TYPE);
    return false;
  }

  if (!isReducibleControlFlow(F, DT)) {
    LLVM_DEBUG(dbgs() << "Irreducible control flow, can not vectorize\n");
    OCLSTAT_DEFINE(CantVectNonReducable,"Unable to vectorize because the control flow is irreducible",kernelStats);
    CantVectNonReducable++;
    intel::Statistic::pushFunctionStats (kernelStats, F, DEBUG_TYPE);
    return false;
  }

  if (hasIllegalTypes(F)) {
    LLVM_DEBUG(dbgs() << "Types unsupported by codegen, can not vectorize\n");
    OCLSTAT_DEFINE(CantVectIllegalTypes,"Unable to vectorize because of unsupported opcodes",kernelStats);
    CantVectIllegalTypes++;
    intel::Statistic::pushFunctionStats (kernelStats, F, DEBUG_TYPE);
    return false;
  }

  if (hasNonInlineUnsupportedFunctions(F)) {
    LLVM_DEBUG(dbgs() << "Call to unsupported functions, can not vectorize\n");
    OCLSTAT_DEFINE(CantVectNonInlineUnsupportedFunctions,"Unable to vectorize because of calls to functions that can't be inlined",kernelStats);
    CantVectNonInlineUnsupportedFunctions++;
    intel::Statistic::pushFunctionStats (kernelStats, F, DEBUG_TYPE);
    return false;
  }

  if (hasDirectStreamCalls(F, services)) {
    LLVM_DEBUG(dbgs() << "Has direct calls to stream functions, can not vectorize\n");
    OCLSTAT_DEFINE(CantVectStreamCalls,"Unable to vectorize because the code contains direct stream calls",kernelStats);
    CantVectStreamCalls++;
    intel::Statistic::pushFunctionStats (kernelStats, F, DEBUG_TYPE);
    return false;
  }

  if (hasUnreachableInstructions(F)) {
    LLVM_DEBUG(dbgs() << "Has unreachable instructions, can not vectorize\n");
    OCLSTAT_DEFINE(CantVectUnreachableCode,"Unable to vectorize because the code contains unreachable code",kernelStats);
    CantVectUnreachableCode++;
    intel::Statistic::pushFunctionStats (kernelStats, F, DEBUG_TYPE);
    return false;
  }

  OCLSTAT_DEFINE(CanVect,"Code is vectorizable",kernelStats);
  CanVect++;
  intel::Statistic::pushFunctionStats (kernelStats, F, DEBUG_TYPE);
  return true;
}

bool CanVectorizeImpl::hasVariableGetTIDAccess(Function &F, RuntimeServices* services) {
  assert(services && "Unable to get runtime services");
  for (Function::iterator bbit = F.begin(), bbe=F.end(); bbit != bbe; ++bbit) {
    for (BasicBlock::iterator it = bbit->begin(), e=bbit->end(); it!=e;++it) {
      if (CallInst* call = dyn_cast<CallInst>(it)) {
        bool err = false;
        unsigned dim = 0;
        services->isTIDGenerator(call, &err, &dim);
        // We are unable to vectorize this code because get_global_id is messed up
        if (err) return true;
      }
    }
  }
  // TID access is okay
  return false;
}

// Reference:
//  Book: High Performance Compilers for Parallel Computing / Michael Wolfe
//  Page 60, Section 3.2.4 - Finding Cycles in Directed Graphs
//
// Check if the graph is irreducible using the standard algorithm:
// 1. If you ignore backedges, graph is acyclic.
// 2. backedges are edges where the target dominates the src.
bool CanVectorizeImpl::isReducibleControlFlow(Function& F, DominatorTree& DT) {
  llvm::SmallSet<BasicBlock*, 16> removed;
  llvm::SmallSet<BasicBlock*, 16> toProcess;
  toProcess.insert(&F.getEntryBlock());

  while (!toProcess.empty()) {
    BasicBlock* next = nullptr;
    // Find a free node
    llvm::SmallSet<BasicBlock*, 16>::iterator bb(toProcess.begin());
    llvm::SmallSet<BasicBlock*, 16>::iterator bbe(toProcess.end());
    // for each of the toProcess blocks, find a block who's
    // preds are all removed
    for (; bb != bbe ; ++bb) {
      // Are all of its preds removed ?
      bool Removed = true;
      llvm::pred_iterator p = pred_begin(*bb), pe = pred_end(*bb);

      // for each pred
      for (; p != pe; ++p) {
        // This is a back-edge, ignore it
        // Note: nodes dominate themselves.
        // This is good because this is how we handle self-edges.
        if (DT.dominates(*bb, *p)) continue;
        // pred in removed list ?
        Removed &= removed.count(*p);
      }

      // Found a candidate to remove
      if (Removed) {
        next = *bb;
        break;
      }
    }

    // Did not find a free node to remove,
    // The graph has a cycle. This code is irreducible.
    if (!next) {
      return false;
    }

    // Remove this node
    toProcess.erase(next);
    removed.insert(next);
    // Insert all successors to the 'todo' queue
    llvm::succ_iterator s = succ_begin(next), se = succ_end(next);
    for (; s != se; ++s) {
      // did not visit this before
      if (!removed.count(*s)) toProcess.insert(*s);
    }
  }

  // Code is reducible
  return true;
}

bool CanVectorizeImpl::hasIllegalTypes(Function &F) {
  // For each BB
  for (Function::iterator bbit = F.begin(), bbe=F.end(); bbit != bbe; ++bbit) {
    // For each instruction
    for (BasicBlock::iterator it = bbit->begin(), e=bbit->end(); it!=e;++it) {
      Type* tp = it->getType();
      // strip vector types
      if (VectorType* VT = dyn_cast<VectorType>(tp)) {
        tp = VT->getElementType();
      }
      // check that integer types are legal
      if (IntegerType* IT = dyn_cast<IntegerType>(tp)) {
        unsigned BW = IT->getBitWidth();
        if (BW > 64)
          return true;
      }
    }
  }
  return false;
}

bool CanVectorizeImpl::hasNonInlineUnsupportedFunctions(Function &F) {
  using namespace Intel::MetadataAPI;

  Module *pM = F.getParent();
  std::set<Function *> unsupportedFunctions;
  std::set<Function *> roots;

  // Add all kernels to root functions
  // Kernels assumes to have implicit barrier
  auto kernels = KernelList(pM);
  roots.insert(kernels.begin(), kernels.end());

  // Add all functions that contains synchronize/get_local_id/get_global_id to root functions
  CompilationUtils::FunctionSet oclFunction;

  //Get all synchronize built-ins declared in module
  CompilationUtils::getAllSyncBuiltinsDcls(oclFunction, pM);

  //Get get_local_id built-in if declared in module
  if ( Function *pF = pM->getFunction(CompilationUtils::mangledGetLID()) ) {
    oclFunction.insert(pF);
  }
  //Get get_global_id built-in if declared in module
  if ( Function *pF = pM->getFunction(CompilationUtils::mangledGetGID()) ) {
    oclFunction.insert(pF);
  }

  for ( CompilationUtils::FunctionSet::iterator fi = oclFunction.begin(), fe = oclFunction.end(); fi != fe; ++fi ) {
    Function *F = *fi;
    for (Function::user_iterator ui = F->user_begin(), ue = F->user_end(); ui != ue; ++ui ) {
      CallInst *CI = dyn_cast<CallInst> (*ui);
      if (!CI) continue;
      Function *pCallingFunc = CI->getParent()->getParent();
      roots.insert(pCallingFunc);
    }
  }

  // Fill unsupportedFunctions set with all functions that calls directly or undirectly
  // functions from the root functions set
  LoopUtils::fillFuncUsersSet(roots, unsupportedFunctions);
  return unsupportedFunctions.count(&F);
}

bool CanVectorizeImpl::hasDirectStreamCalls(Function &F, RuntimeServices* services) {
  Module *pM = F.getParent();
  bool isPointer64 = pM->getDataLayout().getPointerSizeInBits(0) == 64;
  std::set<Function *> streamFunctions;
  std::set<Function *> unsupportedFunctions;

  Function* readStreamFunc = ((OpenclRuntime*)services)->getReadStream(isPointer64);
  if (readStreamFunc) {
    // This returns the read stream function *from the runtime module*.
    // We need a function in *this* module with the same name.
    readStreamFunc = pM->getFunction(readStreamFunc->getName());
    if (readStreamFunc)
      streamFunctions.insert(readStreamFunc);
  }

  Function* writeStreamFunc = ((OpenclRuntime*)services)->getWriteStream(isPointer64);
  if (writeStreamFunc) {
    // This returns the write stream function *from the runtime module*.
    // We need a function in *this* module with the same name.
    writeStreamFunc = pM->getFunction(writeStreamFunc->getName());
    if (writeStreamFunc)
      streamFunctions.insert(writeStreamFunc);
  }

  // If we have stream functions in the module, don't vectorize their users.
  if (streamFunctions.size())
    LoopUtils::fillFuncUsersSet(streamFunctions, unsupportedFunctions);

  return unsupportedFunctions.count(&F);

}

bool CanVectorizeImpl::hasUnreachableInstructions(Function &F) {
  for (Function::iterator bbit = F.begin(), bbe = F.end(); bbit != bbe; ++bbit) {
    if (isa<UnreachableInst>(bbit->getTerminator()))
      return true;
  }
  return false;
}

extern "C" {
  FunctionPass* createWeightedInstCounter(bool preVec = true,
                                          Intel::CPUId cpuId = Intel::CPUId()) {
    return new intel::WeightedInstCounter(preVec, cpuId);
  }
}


} // namespace intel
