#ifndef __IRCF__H__
#define __IRCF__H__

#include "llvm/Transforms/Scalar.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/Dominators.h"
#include "llvm/Pass.h"
#include "llvm/ADT/SmallSet.h"
#include "RuntimeServices.h"
#include <stdlib.h>
#include "TargetArch.h"

using namespace llvm;

namespace intel {

 class WIAnalysis;
  
  class VectorizationHeuristics : public FunctionPass {
  public:
    static char ID; // Pass ID, replacement for typeid
    VectorizationHeuristics(Intel::ECPU cpuId=Intel::CPU_LAST, 
      Intel::ECPUFeatureSupport featureSupport=Intel::CFS_NONE): 
      FunctionPass(ID), m_preferedVectorSize(0), m_hasBarrier(false),
      m_mayVectorize(false), m_featureSupport(featureSupport), m_cpuid(cpuId),
      m_WIAnalysisPass(0) {
      assert((!isMic() || !hasAVX()) && "illegal configuration!");
      // Check if heuristics are disabled thru environment variable
      const char *useHeuristicsEnv = getenv("CL_CONFIG_VECTORIZER_HEURISTICS");
      m_shouldUseHeuristics = (!(useHeuristicsEnv && (StringRef(useHeuristicsEnv) == "false")));
    }

    //  Detect Irreducible Control flow
    bool runOnFunction(Function &F);

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<LoopInfo>();
      AU.addRequired<DominatorTree>();
      AU.addRequired<PostDominatorTree>();
      AU.setPreservesAll();
    }

    void reset() {
      m_preferedVectorSize = 0; 
      m_hasBarrier = false;
    }

    int getVectorSize() const {
      return m_preferedVectorSize;
    }

    bool hasBarrier() const {
      return m_hasBarrier;
    }

    bool mayVectorize() const {
      return m_mayVectorize; 
    }

  private:
    typedef std::vector< std::pair<BasicBlock*, BasicBlock*> > ControlList;
    void getMaxNestLevel(ControlList& control, std::map<BasicBlock*, unsigned>& MaxNest);
    /// @brief Check if the function is control flow heavy
    /// @param F function to check
    /// @return True if heavy
    bool isCFHeavy(Function &F);
    /// @brief Check ifthe function is heavy on scatter and gather
    /// @param F function to check
    /// @return True if has heavy
    bool isHeavyScatterGather(Function &F);
    /// @brief Check if two blocks are control dependent
    /// @param x 
    /// @param y 
    /// @return True if X -> Y
    bool isCD(const BasicBlock* x, const BasicBlock *y);
    /// @brief Check if the block should be protected with masks
    /// @param x
    /// @return True if masked
    bool isMasked(BasicBlock* x);    
    /// @brief Check if the function has variable access to get_global/loval_id(X)
    /// @param F function to check
    /// @return True if has such access
    bool hasVariableGetTIDAccess(Function &F);
    /// @brief Check if the function needs masking
    /// @param F function to check
    /// @return True if need to use masks
    bool masksNeeded(Function &F);
    /// @brief True if the function includes relatively lots of int operations
    /// @param F function to check
    /// @return True if has lots of int
    bool isIntegerHeavy(Function &F);
        /// @brief True if the function includes relatively lots of vector types
    /// @param F function to check
    /// @return True if has lots of vectors
    bool isVectorHeavy(Function &F);
    /// @brief True if the function includes lots of PHIs.
    /// Phis are translated to selects in the predication phase, which has
    /// a high overhead.
    /// @param F function to check
    /// @return True if has lots of PHIs.
    bool isPHIHeavy(Function &F);
    /// @brief True if the function uses slow barriers
    ///  barriers inside loops, or barrier when no loops
    /// @param F function to check
    /// @return True if has slow barriers
    bool hasLoopBarriers(Function &F);
    /// @brief Checks if the function has loops, return max nest
    /// @param F function to check
    /// @return Highest nest
    unsigned maxLoopNest(Function &F);
    /// @brief Checks if the program has reducible control flow
    /// @param F function to check
    /// @return True if reducible
    bool isReducibleControlFlow(Function &F);
    /// @brief Checks if the incoming program has illegal types
    /// @param F function to check
    /// @return True if illegal types
    bool hasIllegalTypes(Function &F);
    /// @brief Checks if the incoming program has types which the 
    // vectorize/codegen handle poorly
    /// @param F function to check
    /// @return True if has bad types
    bool hasDifficultTypes(Function &F);
    /// @brief Checks if the incoming program too many GEPs in the main loop
    /// @param F function to check
    /// @return True if has too many GEPS
    bool isGEPHeavy(Function &F);
    /// @brief indicates whether the cpu id is MIC.
    bool isMic()const;
    /// @brief indicates whether the architecture supports AVX 256
    bool hasAVX()const;
    /// @brief indicates whether the architecture supports haswell
    bool hasAVX2()const;
    /// The desired vector size for vectorizations
    /// Zero for no-vectorization
    int m_preferedVectorSize;
    /// Whether or not the kernel has a barrier call
    bool m_hasBarrier;
    /// Whether or not we should use heuristics at all
    bool m_shouldUseHeuristics;
    /// Indicates that the vectorization is impossible
    bool m_mayVectorize;
    /// Features supported by the architecture
    int m_featureSupport;
    /// Is for MIC architecture
    Intel::ECPU m_cpuid;
    /// Last instance of WIAnalysisPass
    WIAnalysis *m_WIAnalysisPass;
  };

}

#endif
