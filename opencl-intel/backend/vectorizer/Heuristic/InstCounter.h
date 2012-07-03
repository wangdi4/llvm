#ifndef __INSTCOUNTER__H__
#define __INSTCOUNTER__H__

#include <stdlib.h>

#include "llvm/Pass.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/Dominators.h"
#include "llvm/ADT/SmallSet.h"
#include "RuntimeServices.h"
#include "TargetArch.h"


using namespace llvm;

namespace intel {
  class WeightedInstCounter : public FunctionPass {
  public:
    static char ID; // Pass ID, replacement for typeid
      WeightedInstCounter(bool preVec = true, bool sanityOnly = false, 
                          Intel::CPUId cpuId = Intel::CPUId()): 
                              FunctionPass(ID), m_cpuid(cpuId), m_preVec(preVec),  
                              m_sanityOnly(sanityOnly), 
                              m_desiredWidth(1), m_totalWeight(0) {        
      initializeLoopInfoPass(*PassRegistry::getPassRegistry());
      initializeDominatorTreePass(*PassRegistry::getPassRegistry());
      initializePostDominatorTreePass(*PassRegistry::getPassRegistry());
      initializePostDominanceFrontierPass(*PassRegistry::getPassRegistry());
    }

    // Provides name of pass
    virtual const char *getPassName() const {
      return "WeightedInstCounter";
    }

    bool runOnFunction(Function &F);
    
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<LoopInfo>();
      AU.addRequired<DominatorTree>();
      AU.addRequired<PostDominatorTree>();
      AU.addRequired<PostDominanceFrontier>();
      AU.setPreservesAll();
    }

    // Returns the desired vectorization width (4/8/16).
    // This is only calculated by the "pre" version of the pass.
    int getDesiredWidth() const {
      return m_desiredWidth;
    }

    // Returns the computed total weight. 
    // This is calculated by both the before and after passes.
    // Internally it's computed as a floating-point weight, but we
    // truncate it to int here.
    float getWeight() const {
      return m_totalWeight;
    }

  private:
    // Indicates whether the architecture is MIC
    bool isMic()const;
    // Indicates whether the architecture supports AVX 256
    bool hasAVX()const;
    // Indicates whether the architecture supports AVX 2 (Haswell)
    bool hasAVX2()const;
    
    // Estimate the number of iterations each loop runs. 
    void estimateIterations(Function &F, DenseMap<Loop*, int> &IterMap) const;

    // Estimate the "straight-line" probability of each block being executed.
    // That is, loops are not taken into account, and backedges are considered
    // to be taken with the same probability as any other edge.
    void estimateProbability(Function &F, DenseMap<BasicBlock*, float> &ProbMap) const;

    // Check which instructions depend on data (loads, image reads0
    void estimateDataDependence(Function &F, DenseSet<Instruction*> &DepSet) const;

    // Get the weight of an instruction
    int getInstructionWeight(Instruction *I, DenseMap<Instruction*, int> &MemOpCostMap);

    // Guess what the dominant type is.
    Type* estimateDominantType(Function &F, DenseMap<Loop*, int> &IterMap,
      DenseMap<BasicBlock*, float> &ProbMap) const;

    // Estimate the relative cost of a binary operator.
    // The cost depends on the type. e.g. an operation on <16 x double> is not normally
    // supported natively. So, take that into account by estimating how many native
    // operations are required.
    int estimateBinOp(BinaryOperator *I);
    
    // Helper for estimateBinOp
    int getOpWidth(VectorType* VecType, int Float, int Double, int LongInt, int ShortInt);

    // Estimate the relative cost of a call.
    // Some calls are expensive, some are cheap... how do we know which are which?
    // Good question. Right now, we don't.
    int estimateCall(CallInst *I);

    // Try to estimate the memory usage pattern. Basically, given a 2D array
    // (which is pretty common for OpenCL kernels), iterating over a row is cheaper
    // than iterating over a column. This has two implications:
    // a) A kernel that iterates over a row may be harmed by vectorization, because
    // suddenly the accesses are no longer consecutive.
    // b) A kernel that iterates over a column will probably gain from vectorization
    // because that will put work on consecutive loads together.
    void estimateMemOpCosts(Function &F, DenseMap<Instruction*, int> &CostMap) const;
    
    // Helper function for DFS
    void addUsersToWorklist(Instruction *I, DenseSet<Instruction*> &Visited,
                           std::vector<Instruction*> &WorkList) const;

    // All multiplications need to be rounded up, and
    // for positive integers, ceil(x/y) = (x + y - 1) / y.
    static inline float ceil_div(int x, int y)
    {
      return (x + y - 1) / y;
    }

    // Used to identify MIC/AVS support
    Intel::CPUId m_cpuid;

    // Is this a before or after vectorization pass.
    // Affects debug printing right now, but may count some
    // things differently
    bool m_preVec;
    
    // Whether only sanity should run, instead of
    // actually counting.
    bool m_sanityOnly;

    // Outputs:
    // Desired vectorization width
    int m_desiredWidth;

    // Total weight of all instructions
    float m_totalWeight;

    // MAGIC NUMBERS
    // Guess for the number of iterations for loops for which 
    // the actual number is unknown.
    static const int LOOP_ITER_GUESS = 32;
    // Weights for different types of instructions
    // This is the baseline
    static const int DEFAULT_WEIGHT = 1;
    // Binary operations weigh the same
    static const int BINARY_OP_WEIGHT = DEFAULT_WEIGHT;
    // TODO: Calls have uniform (heavy) weight, which is nonsense. 
    // Replace with something that actually makes sense.
    static const int CALL_WEIGHT = 20;
    // Shuffles/Extracts/Inserts may be more expensive.
    static const int EXPENSIVE_SHUFFLE_WEIGHT = 5;
    static const int CHEAP_SHUFFLE_WEIGHT = 2;
    static const int EXPENSIVE_EXTRACT_WEIGHT = 2;
    static const int CHEAP_EXTRACT_WEIGHT = DEFAULT_WEIGHT;
    static const int INSERT_WEIGHT = DEFAULT_WEIGHT;

    // Memops are complicated.
    static const int MEM_OP_WEIGHT = 4;
    static const int CHEAP_MEMOP_WEIGHT = DEFAULT_WEIGHT;
    static const int EXPENSIVE_MEMOP_WEIGHT = 16;
    // Conditional branches are potentially expensive... 
    // misprediction penalty.
    static const int COND_BRANCH_WEIGHT = 4;
    // Penalty for allZero/allOne loops
    static const float ALL_ZERO_LOOP_PENALTY;
    // Penalty for checking TID equality
    static const float TID_EQUALITY_PENALTY;

    // Special cases for cheap calls. This is clearly overfitting...
    static const int CALL_CLAMP_WEIGHT = 4;
    static const int CALL_FLOOR_WEIGHT = 2;
public:
    // Ratio multiplier
    static const float RATIO_MULTIPLIER;
  };

  class VectorizationPossibilityPass : public FunctionPass {
  public:
    static char ID; // Pass ID, replacement for typeid
    VectorizationPossibilityPass(): FunctionPass(ID), m_canVectorize(false) {}

    bool runOnFunction(Function &F);
     
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<DominatorTree>();
    }

    // Returns false is the code is essentially non-vectorizeable.
    // This is not advice, this is a final decision.
    bool isVectorizable() const {
      return m_canVectorize;
    }

  private:
    bool m_canVectorize;
  };

  class CanVectorizeImpl
  {
  public:
    // Checks whether we can (as opposed to should) vectorize 
    // this function.
    static bool canVectorize(Function &F, DominatorTree &DT);

  private:
    // Functions imported as is from old heuristic.

    // Checks if the program has reducible control flow
    static bool isReducibleControlFlow(Function &F, DominatorTree &DT);
    
    // Check if the function has variable access to get_global/loval_id(X)
    static bool hasVariableGetTIDAccess(Function &F);
    
    // Checks if the incoming program has illegal types
    // An illegal type in this context is iX, where X > 64.
    // TODO: Check if this is still relevant to the current codegen.
    static bool hasIllegalTypes(Function &F);
  };
}

#endif // __INSTCOUNTER__H__
