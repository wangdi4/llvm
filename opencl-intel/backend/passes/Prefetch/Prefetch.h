#ifndef __PREFETCH_H_
#define __PREFETCH_H_

#include "llvm/Pass.h"
#include "llvm/Function.h"
#include "llvm/Module.h"
#include "llvm/GlobalVariable.h"
#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/Analysis/Dominators.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/ScalarEvolutionExpander.h"


using namespace llvm;

namespace intel{

  class Prefetch : public FunctionPass {

    public:
      static char ID; // Pass identification, replacement for typeid
      Prefetch();

      ~Prefetch();

      virtual const char *getPassName() const {
        return "Prefetch";
      }

      virtual bool runOnFunction(Function &F);

      virtual void getAnalysisUsage(AnalysisUsage &AU) const {
        AU.addRequired<LoopInfo>();
        AU.addRequired<ScalarEvolution>();
        AU.addRequired<BranchProbabilityInfo>();
        AU.addPreserved<BranchProbabilityInfo>();
        AU.addRequired<DominatorTree>();
        AU.setPreservesCFG();
      }

      static const int defaultL1PFType;
      static const int defaultL2PFType;

    private:
      // Pass pointers
      LoopInfo        * m_LI ;
      ScalarEvolution * m_SE;
      SCEVExpander    * m_ADRExpander;

      // Controls
      bool m_exclusiveMPF;  // if MPF is detected don't generate prefetches at all
      bool m_coopAPFMPF;    // if MPF is detected continue to generated APF
      bool m_disableAPF;    // don't APF

      // Type pointers
      Type *m_i8;
      Type *m_i32;
      Type *m_i64;
      Type *m_pi8;
      Type *m_void;

      static const std::string m_intrinsicName;
      static const std::string m_prefetchIntrinsicName;
      static const std::string m_gatherPrefetchIntrinsicName;
      static const std::string m_scatterPrefetchIntrinsicName;
      static const std::string m_gatherIntrinsicName;
      static const std::string m_maskGatherIntrinsicName;
      static const std::string m_scatterIntrinsicName;
      static const std::string m_maskScatterIntrinsicName;

      static const int L1MissLatency;
      static const int L2MissLatency;
      static const int L1PrefetchSlots;
      static const int L2PrefetchSlots;
      static const int MaxThreads;
      static const int CacheLineSize;

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

        loopPFInfo() {}
        loopPFInfo (int count) : numRefs(count), L1Distance(0), L2Distance(0),
          numThreads(MaxThreads) {}
      };
      typedef std::map<Loop *, loopPFInfo> LoopInfoMap;
      LoopInfoMap m_LoopInfo;

      struct memAccess {
        Instruction *I;
        const SCEV *S;
        int step;
        int offset;
        bool recurring;

        memAccess () {}
        memAccess (Instruction *i, const SCEV *s, int _step, int _offset) :
            I(i), S(s), step(_step), offset(_offset), recurring(false) {}
      };

      typedef std::vector<memAccess> memAccessV;
      typedef std::map<BasicBlock *, memAccessV> BBAccesses;

      // holds all memory accesses that deserve prefetching
      BBAccesses m_addresses;

    private:
      void init();

      // memAccessExists - checks if acccess overlaps with another access
      // recorded in MAV.
      // accessIsReady tells weather all access details are already calculated
      bool memAccessExists (memAccess &access, memAccessV &MAV,
                            bool accessIsReady);

      bool detectReferencesForPrefetch(Function &F);

      void countPFPerLoop ();

      unsigned int IterLength(Loop *L);

      void getPFDistance(Loop *L, loopPFInfo &info);

      void getPFDistance();

      void InsertPF (Instruction *I, Loop *L, int PFType,
                     const SCEV *SAddr, unsigned count);

      void emitPrefetches();

      bool autoPrefetch(Function &F);

  }; // Prefetch class

} // namespace intel


#endif // __PREFETCH_H_
