/*****************************************************************************\

Copyright (c) Intel Corporation (2013).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  Prefetch.h

\*****************************************************************************/

#ifndef __PREFETCH_H_
#define __PREFETCH_H_

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/ScalarEvolutionExpander.h"
#include "OclTune.h"


using namespace llvm;

namespace intel{

  // Micro-architecture specific information
  struct UarchInfo {
    static const int L1MissLatency;
    static const int L2MissLatency;
    static const int L1PrefetchSlots;
    static const int L2PrefetchSlots;
    static const int MaxThreads;
    static const int CacheLineSize;
    static const int defaultL1PFType;
    static const int defaultL2PFType;
  };

  // PrefetchStats class contains all the stats that are used by the
  // class PrefetchCandidateUtils static methods, since stats can't be static
  class PrefetchStats {
  public:
    PrefetchStats(Statistic::ActiveStatsT &statList);

    // Statistics
    Statistic Candidate_global_gather;
    Statistic Candidate_local_gather;
    Statistic Candidate_constant_gather;
    Statistic Candidate_private_gather;

    Statistic Candidate_global_masked_gather;
    Statistic Candidate_local_masked_gather;
    Statistic Candidate_constant_masked_gather;
    Statistic Candidate_private_masked_gather;

    Statistic Candidate_global_scatter;
    Statistic Candidate_local_scatter;
    Statistic Candidate_constant_scatter;
    Statistic Candidate_private_scatter;

    Statistic Candidate_global_masked_scatter;
    Statistic Candidate_local_masked_scatter;
    Statistic Candidate_constant_masked_scatter;
    Statistic Candidate_private_masked_scatter;
    Statistic PF_triggered_by_masked_unaligned_loads;
    Statistic PF_triggered_by_masked_unaligned_stores;
    Statistic PF_triggered_by_masked_random_gather;
    Statistic PF_triggered_by_masked_random_scatter;
    Statistic PF_triggered_by_random_gather;
    Statistic PF_triggered_by_random_scatter;

    Statistic Abort_APF_since_detected_manual_PF;
  };

  class Prefetch : public FunctionPass {

    public:
      static char ID; // Pass identification, replacement for typeid
      Prefetch(int level=2);

      ~Prefetch();

      virtual llvm::StringRef getPassName() const {
        return "Prefetch";
      }

      virtual bool runOnFunction(Function &F);

      virtual void getAnalysisUsage(AnalysisUsage &AU) const {
        AU.addRequired<LoopInfoWrapperPass>();
        AU.addRequired<ScalarEvolutionWrapperPass>();
        AU.addRequired<BranchProbabilityInfoWrapperPass>();
        AU.addPreserved<BranchProbabilityInfoWrapperPass>();
        AU.addRequired<DominatorTreeWrapperPass>();
        AU.setPreservesCFG();
      }

    private:
      // Pass pointers
      LoopInfo        * m_LI ;
      ScalarEvolution * m_SE;
      SCEVExpander    * m_ADRExpander;

      // Controls
                                  // prefetch level, which accesses to consider for prefetching:
      int  m_level;               // 0 - none, 1 - loads/stores, 2 - all sequential accesses, 3 - all accesses
                                  // if MPF is detected don't generate
      bool m_exclusiveMPF;        // prefetches at all
      bool m_coopAPFMPF;          // if MPF is detected continue to generated APF
      bool m_disableAPF;          // don't APF
      bool m_disableAPFGS;        // don't APF gathers and scatters
                                  // don't APF gathers and scatters and don't
      bool m_disableAPFGSTune;    // collect tuning info
                                  // Consider further ahead prefetch for less
      bool m_calcFactor;          // than cache line accesses
      bool m_prefetchScalarCode;  // prefetch for accesses in scalar BBs

      // Type pointers
      Type *m_i8;
      Type *m_i32;
      Type *m_i64;
      Type *m_pi8;
      Type *m_void;

      static const std::string m_prefetchIntrinsicName;

      static const int PrefecthedAddressSpaces;

      static const int defaultTripCount;

      // optimal number of threads for this loop
      int m_numThreads;

      // map from loop to loop iteration length estimation
      typedef std::map<Loop *, unsigned int> LoopToLengthMap;
      LoopToLengthMap m_iterLength;

      // indication for each loop whether it's vectorized. if it's not
      // vectorized prefetches will not be inserted
      std::set<Loop *> m_isVectorized;

      // map from loop to PF related info
      struct loopPFInfo {
        int numRefs;       // number of accesses to prefetch in this loop
        int L1Distance;    // L1 prefetch distance
        int L2Distance;    // L2 prefetch distance
        int numThreads;    // optimal number of threads
        int iterLen;       // IR based iteration length
                           // number of iterations in which the same cache line
        int factor;        // is the prefech target
                           // Effective number of references if loop iteration
        int factNumRefs;   // length is factored
        int numRandom;     // number of random accesses to prefetch in loop

        loopPFInfo() {}
        loopPFInfo (int count, int _factor, int _numRandom) : numRefs(count),
            L1Distance(0), L2Distance(0), numThreads(UarchInfo::MaxThreads),
            iterLen(0), factor (_factor), factNumRefs(0), numRandom(_numRandom)
        {}
      };
      typedef std::map<Loop *, loopPFInfo> LoopInfoMap;
      LoopInfoMap m_LoopInfo;

      struct memAccess {
        Instruction *I;
        const SCEV *S;
        int step;
        int offset;
        int factor;
        int flags;

        memAccess () {}
        memAccess (Instruction *i, const SCEV *s, int _step, int _offset,
            bool random, bool exclusive) :
            I(i), S(s), step(_step), offset(_offset), flags(0) {
          if (random) setRandom();
          if (exclusive) setExclusive();
        }

        // set and query flags
        void setRecurring () {flags |= 0x1;}
        bool isRecurring () {return (flags & 0x1);}

        void setRandom() {flags |= 0x2;}
        bool isRandom() {return (flags & 0x2);}

        void setExclusive () {flags |= 0x4;}
        bool isExclusive () {return (flags & 0x4);}
      };

      typedef std::vector<memAccess> memAccessV;
      typedef std::map<BasicBlock *, memAccessV> BBAccesses;

      // holds all memory accesses that deserve prefetching
      BBAccesses m_addresses;

      // Statistics
      Statistic::ActiveStatsT m_kernelStats;
      Statistic Ignore_BB_lacking_vector_instructions;
      Statistic Ignore_BB_not_in_loop;
      Statistic Accesses_merged_to_1_PF_have_large_diff_256;
      Statistic Accesses_not_merged_to_1_PF_have_large_diff_1024;
      Statistic Ignore_non_simple_load;
      Statistic Ignore_local_load;
      Statistic Ignore_private_load;
      Statistic Ignore_non_simple_store;
      Statistic Ignore_local_store;
      Statistic Ignore_private_store;

      Statistic Access_has_no_scalar_evolution_in_loop;
      Statistic Access_step_is_loop_variant;
      Statistic Access_step_is_loop_invariant;
      Statistic Access_match_in_same_BB;
      Statistic Access_match_in_dominating_BB;
      Statistic Access_exact_match;
      Statistic Access_match_same_cache_line;
      Statistic PF_triggered_by_partial_cache_line_access;
      Statistic PF_triggered_by_full_cache_line_access;
      Statistic PF_triggered_by_multiple_cache_line_access;
      Statistic Access_step_is_variable;

      PrefetchStats m_stats;

    private:
      void init();

      // memAccessExists - checks if access overlaps with another access
      // recorded in MAV.
      // accessIsReady tells weather all access details are already calculated
      bool memAccessExists (memAccess &access, memAccessV &MAV,
                            bool accessIsReady);

      bool detectReferencesForPrefetch(Function &F);

      void countPFPerLoop ();

      unsigned int IterLength(Loop *L);

      void getPFDistance(Loop *L, loopPFInfo &info);

      void getPFDistance();

      void insertPF (Instruction *I, Loop *L, int PFType,
                     const SCEV *SAddr, unsigned count, bool pfExclusive);

      void emitPrefetches();

      bool autoPrefetch(Function &F);

  }; // Prefetch class

} // namespace intel


#endif // __PREFETCH_H_
