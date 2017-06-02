//===-- LoopVectorizationPlanner.h -------------------------------*- C++ -*-===//
//
//   Copyright (C) 2015-2017 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file defines LoopVectorizationPlannerBase and LoopVectorizationPlanner.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_VPLAN_LOOPVECTORIZATIONPLANNER_H
#define LLVM_TRANSFORMS_VECTORIZE_VPLAN_LOOPVECTORIZATIONPLANNER_H

#include "IntelVPlan.h"
#include "llvm/ADT/DenseMap.h"

namespace llvm {

class Loop;
class ScalarEvolution;
class TargetLibraryInfo;
class TargetTransformInfo;
class VPOCodeGen;
class VPOVectorizationLegality;
namespace vpo {
class WRNVecLoopNode;
}

using namespace vpo;

/// LoopVectorizationPlanner - builds and optimizes the Vectorization Plans
/// which record the decisions how to vectorize the given loop.
/// In particular, represent the control-flow of the vectorized version,
/// the replication of instructions that are to be scalarized, and interleave
/// access groups.
class LoopVectorizationPlannerBase {
public:
  /// Build initial VPlans according to the information gathered by Legal
  /// when it checked if it is legal to vectorize this loop.
  /// Returns the number of VPlans built, zero if failed.
  unsigned buildInitialVPlans(unsigned MinVF, unsigned MaxVF);

  virtual void collectDeadInstructions() {}

  /// On VPlan construction, each instruction marked for predication by Legal
  /// gets its own basic block guarded by an if-then. This initial planning
  /// is legal, but is not optimal. This function attempts to leverage the
  /// necessary conditional execution of the predicated instruction in favor
  /// of other related instructions. The function applies these optimizations
  /// to all VPlans.
  //void optimizePredicatedInstructions();

  /// Record CM's decision and dispose of all other VPlans.
  void setBestPlan(unsigned VF, unsigned UF);

  /// Generate the IR code for the body of the vectorized loop according to the
  /// best selected VPlan.
  //void executeBestPlan(InnerLoopVectorizer &LB);

  IntelVPlan *getVPlanForVF(unsigned VF) { return VPlans[VF].get(); }

  void printCurrentPlans(const std::string &Title, raw_ostream &O);

protected:
  LoopVectorizationPlannerBase(WRNVecLoopNode *WRL) : WRLoop(WRL) {}
  virtual ~LoopVectorizationPlannerBase() {}

  /// Build an initial VPlan according to the information gathered by Legal
  /// when it checked if it is legal to vectorize this loop. \return a VPlan
  /// that corresponds to vectorization factors starting from the given
  /// \p StartRangeVF and up to \p EndRangeVF, exclusive, possibly decreasing
  /// the given \p EndRangeVF.
  virtual std::shared_ptr<IntelVPlan> buildInitialVPlan(unsigned StartRangeVF,
                                                   unsigned &EndRangeVF) = 0;

  WRNVecLoopNode *WRLoop;

private:
  /// Determine whether \p I will be scalarized in a given range of VFs.
  /// The returned value reflects the result for a prefix of the range, with \p
  /// EndRangeVF modified accordingly.
  //bool willBeScalarized(Instruction *I, unsigned StartRangeVF,
  //                      unsigned &EndRangeVF);

  /// Iteratively sink the scalarized operands of a predicated instruction into
  /// the block that was created for it.
  //void sinkScalarOperands(Instruction *PredInst, VPlan *Plan);

  /// Determine whether a newly-created recipe adds a second user to one of the
  /// variants the values its ingredients use. This may cause the defining
  /// recipe to generate that variant itself to serve all such users.
  //void assignScalarVectorConversions(Instruction *PredInst, VPlan *Plan);

  /// The loop that we evaluate.
  Loop *TheLoop;

  /// Loop Info analysis.
  //LoopInfo *LI;

  /// Target Library Info.
  //const TargetLibraryInfo *TLI;

  /// Target Transform Info.
  //const TargetTransformInfo *TTI;

  /// The legality analysis.
  //LoopVectorizationLegality *Legal;

  /// The profitablity analysis.
  //LoopVectorizationCostModel *CM;

  //InnerLoopVectorizer *ILV = nullptr;

  // Holds instructions from the original loop that we predicated. Such
  // instructions reside in their own conditioned VPBasicBlock and represent
  // an optimization opportunity for sinking their scalarized operands thus
  // reducing their cost by the predicate's probability.
  //SmallPtrSet<Instruction *, 4> PredicatedInstructions;

  /// VPlans are shared between VFs, use smart pointers.
  DenseMap<unsigned, std::shared_ptr<IntelVPlan>> VPlans;

protected:
  unsigned BestVF = 0;
  unsigned BestUF = 0;
};

class LoopVectorizationPlanner : public LoopVectorizationPlannerBase {
public:
  LoopVectorizationPlanner(WRNVecLoopNode *WRL, Loop *Lp, LoopInfo *LI,
                           ScalarEvolution *SE,
                           const TargetLibraryInfo *TLI,
                           const TargetTransformInfo *TTI,
                           class DominatorTree* DT,
                           VPOVectorizationLegality *Legal)
      : LoopVectorizationPlannerBase(WRL),
        TheLoop(Lp), LI(LI), SE(SE), TLI(TLI), TTI(TTI), DT(DT), Legal(Legal) {}

  ~LoopVectorizationPlanner() {}

  /// On VPlan construction, each instruction marked for predication by Legal
  /// gets its own basic block guarded by an if-then. This initial planning
  /// is legal, but is not optimal. This function attempts to leverage the
  /// necessary conditional execution of the predicated instruction in favor
  /// of other related instructions. The function applies these optimizations
  /// to all VPlans.
  //void optimizePredicatedInstructions();

  /// Record CM's decision and dispose of all other VPlans.
  //void setBestPlan(unsigned VF, unsigned UF);

  /// Generate the IR code for the body of the vectorized loop according to the
  /// best selected VPlan.
  void executeBestPlan(VPOCodeGen &LB);

  Loop *getLoop() { return TheLoop; }

  void collectDeadInstructions() override;

private:
  /// Build an initial VPlan according to the information gathered by Legal
  /// when it checked if it is legal to vectorize this loop. \return a VPlan
  /// that corresponds to vectorization factors starting from the given
  /// \p StartRangeVF and up to \p EndRangeVF, exclusive, possibly decreasing
  /// the given \p EndRangeVF.
  std::shared_ptr<IntelVPlan> buildInitialVPlan(unsigned StartRangeVF,
                                                unsigned &EndRangeVF) override;

  /// Determine whether \p I will be scalarized in a given range of VFs.
  /// The returned value reflects the result for a prefix of the range, with
  /// \p
  /// EndRangeVF modified accordingly.
  // bool willBeScalarized(Instruction *I, unsigned StartRangeVF,
  //                      unsigned &EndRangeVF);

  /// Iteratively sink the scalarized operands of a predicated instruction
  /// into
  /// the block that was created for it.
  // void sinkScalarOperands(Instruction *PredInst, VPlan *Plan);

  /// Determine whether a newly-created recipe adds a second user to one of
  /// the
  /// variants the values its ingredients use. This may cause the defining
  /// recipe to generate that variant itself to serve all such users.
  // void assignScalarVectorConversions(Instruction *PredInst, VPlan *Plan);

  /// The loop that we evaluate.
  Loop *TheLoop;

  /// Loop Info analysis.
  LoopInfo *LI;

  /// Scalar Evolution analysis.
  ScalarEvolution *SE;

  /// Target Library Info.
  const TargetLibraryInfo *TLI;

  /// Target Transform Info.
  const TargetTransformInfo *TTI;

  /// The dominators tree.
  class DominatorTree* DT;

  /// The legality analysis.
  VPOVectorizationLegality *Legal;

  /// The profitablity analysis.
  // LoopVectorizationCostModel *CM;

  VPOCodeGen *ILV = nullptr;

  // Holds instructions from the original loop that we predicated. Such
  // instructions reside in their own conditioned VPBasicBlock and represent
  // an optimization opportunity for sinking their scalarized operands thus
  // reducing their cost by the predicate's probability.
  // SmallPtrSet<Instruction *, 4> PredicatedInstructions;

  // Holds instructions from the original loop whose counterparts in the
  // vectorized loop would be trivially dead if generated. For example,
  // original induction update instructions can become dead because we
  // separately emit induction "steps" when generating code for the new loop.
  // Similarly, we create a new latch condition when setting up the structure
  // of the new loop, so the old one can become dead.
  SmallPtrSet<Instruction *, 4> DeadInstructions;
};


// Code from POC for HIR

//class LoopVectorizationPlannerHIR : public LoopVectorizationPlannerBase { 
//
//private: 
//  /// HIRP - HIR Parser 
//  HIRFramework *HIRF; 
//
//  VPlan *buildInitialVPlan(unsigned StartRangeVF, 
//                           unsigned &EndRangeVF) override; 
//
//public: 
//  static char ID; 
//
//  LoopVectorizationPlannerHIR() : LoopVectorizationPlannerBase(ID) { 
//    llvm::initializeLoopVectorizationPlannerHIRPass( 
//      *PassRegistry::getPassRegistry()); 
//  } 
//  bool runOnFunction(Function &F) override; 
//  void getAnalysisUsage(AnalysisUsage &AU) const override; 
//}; 




} // End LLVM Namespace

#endif //LLVM_TRANSFORMS_VECTORIZE_VPLAN_LOOPVECTORIZATIONPLANNER_H
