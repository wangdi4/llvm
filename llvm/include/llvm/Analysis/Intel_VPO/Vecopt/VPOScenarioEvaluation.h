//===-- VPOScenarioEvaluation.h ---------------------------------*- C++ -*-===//
//
//   Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This header file declares the class for exploration of vectorization
/// candidates.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALISYS_VPO_VPOSCENARIOEVALUATION_H
#define LLVM_ANALISYS_VPO_VPOSCENARIOEVALUATION_H

#include "llvm/Analysis/Intel_LoopAnalysis/HIRVLSClient.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRVectVLSAnalysis.h"
#include "llvm/Analysis/Intel_OptVLS.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrGenerate.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOVecContext.h"

namespace llvm {
namespace vpo {

// FIXME: Switch to USEALOOP (and remove unused code) once the underlying 
// loop can be obtained from the AVRLoop.
//#define USEALOOP 1
#undef USEALOOP

typedef SmallVector<unsigned int, 6> VFsVector;
typedef VFsVector::iterator LoopVFsIt;

class VPOVecContextBase;

/// \brief CostModel Utilities for evaluation of vectorization profitability.
// TODO: Provide cost utilities for different granularities? (AVRWrn,
// AVRLoop, BasicBlock?).
class VPOCostModelBase {
private:
  // TODO: We are currently not taking advantage of information across 
  // different loops within a region. Currently the scope of our passes
  // (cost model, SLEV) is an individual loop. The intention in the future 
  // is to look at multiple loops together.
  AVRWrn *AWrn;
public:
  VPOCostModelBase(AVRWrn *AWrn) : AWrn(AWrn) {}
  VPOCostModelBase() {}

  /// \brief Calculate a cost for the given \p ALoop assuming the Vectorization
  /// factor is \p VF.
  // TODO: Consider also VLS group information
  // TODO: Calculate a cost at the scope of a region, taking advantage of 
  // information about multiple loops in a region at once. Currently our scope
  // is one ALoop at a time. For that we'll also need to finalize how we really
  // want to encode the results of the CostModel, namely, which ALoops in the
  // region to vectorize and using which VFs (directly/explicitely in the 
  // AVR?...)
  int getCost(AVRLoop *ALoop, unsigned int VF) const;
};

/// \brief Visitor Class for accumulating the cost of an AVR Loop.
// TODO: Extend to non-innermost loops. 
class VPOCostGatherer {
private:
  /// \name Used per AvrLoop in region to accumulate the overall vectorization
  /// cost for an AvrLoop in the region. Some costs are incured in each 
  /// iteration; These are counted in \p LoopBodyCost (which will be multiplied
  /// by the loop's iteration-count). Some costs are incured once per loop;
  /// these are counted in \p OutOfLoop Cost.
  /// @{
  unsigned int LoopBodyCost;
  unsigned int OutOfLoopCost;
  /// @}

public:
  VPOCostGatherer() { LoopBodyCost = 0; OutOfLoopCost = 0; }  
  
  /// \name Get the costs associated with vectorizing the AvrLoop currently 
  /// under consideration.
  /// @{
  unsigned int getLoopBodyCost() { return LoopBodyCost; }
  unsigned int getOutOfLoopCost() { return OutOfLoopCost; }
  /// @}

  /// \name Visit Functions 
  /// @{
  bool isDone() { return false; } 
  bool skipRecursion(AVR *ANode) { return false; } 

  void visit(AVR* ANode) {} 
  void postVisit(AVR* ANode) {} 

  void visit(AVRLoop* Loop);
  void postVisit(AVRLoop* Loop);

  void visit(AVRAssign* Assign);
  void postVisit(AVRAssign* Assign);

  void visit(AVRExpression* Expr);
  void postVisit(AVRExpression* Expr);

  void visit(AVRValue* AValue);
  void postVisit(AVRValue* AValue);
  // TODO: Support other AVR nodes as well.
  /// @}
};

/// \brief Information on Data Dependences in a loop.
// CHECKME: Maybe we don't need this abstraction? (currently used only by VLS).
class VPODataDepInfoBase {
public:
  VPODataDepInfoBase() {}
};

class VPODataDepInfo : public VPODataDepInfoBase {
public:
  VPODataDepInfo() : VPODataDepInfoBase() {}
};

class VPODataDepInfoHIR : public VPODataDepInfoBase {
private:
  DDGraph DDG;

public:
  VPODataDepInfoHIR() : DDG(nullptr, nullptr) {}
  VPODataDepInfoHIR(HIRDDAnalysis *DDA, HLLoop *Loop)
      : DDG(DDA->getGraph(Loop, false)) {}
  DDGraph getDDG() const { return DDG; }
};

/// \brief Utilities for Vector-Load-Store (VLS) Optimization Analysis:
/// gather information about the memory accesses in the Loop and the proximity
/// relations between them.
class VPOVLSInfoBase {
public:
  VPOVLSInfoBase() {}
  virtual ~VPOVLSInfoBase() {}

  /// Analyze the loop memory references with respect to VectorContext.
  /// \param [out] VLSMrfs holds objects that can provide services
  /// about these memory references (such as information on access-pattern, 
  /// distances, safety of moving).
  virtual void analyzeVLSMemrefsInLoop(OVLSMemrefVector &VLSMrfs) = 0;

  /// Analyze proximity between the memory references given in \p VLSMrfs.
  /// \param [out] VLSGrps holds the proximity groups found.
  virtual void analyzeVLSGroupsInLoop(OVLSMemrefVector &VLSMrfs,
                                      OVLSGroupVector &VLSGrps) = 0;
};

/// LLVMIR VLSInfo. Currently an empty implmentation.
// TODO.
class VPOVLSInfo : public VPOVLSInfoBase {
public:
  VPOVLSInfo() : VPOVLSInfoBase() {}
  ~VPOVLSInfo() {}

  void analyzeVLSMemrefsInLoop(OVLSMemrefVector &VLSMrfs) override {}
  void analyzeVLSGroupsInLoop(OVLSMemrefVector &VLSMrfs,
                              OVLSGroupVector &VLSGrps) override {}
};

/// HIR VLSInfo.
class VPOVLSInfoHIR : public VPOVLSInfoBase {
private:
  HIRVectVLSAnalysis *VLS;

  /// Information about the Vectorization Context currently considered, namely
  /// the loop, Vectorization Factor, and data dependence graph.
  VPOVecContextHIR *VC;

  /// The memory references of the loop (in HIR).
  // CHECKME: Introduce a base-level abstraction for the loop memrefs?
  LoopMemrefsVector *LoopMemrefs;

public:
  VPOVLSInfoHIR(HIRVectVLSAnalysis *VLS, VPOVecContextHIR *VC,
                LoopMemrefsVector &Refs)
      : VLS(VLS), VC(VC), LoopMemrefs(&Refs) {}
  ~VPOVLSInfoHIR() {}

  void analyzeVLSMemrefsInLoop(OVLSMemrefVector &VLSMrfs) override {
    assert(VC && "VectorContext not set.");
    VLS->analyzeVLSMemrefsInLoop(*VC, *LoopMemrefs, VLSMrfs);
  }

  void analyzeVLSGroupsInLoop(OVLSMemrefVector &VLSMrfs,
                              OVLSGroupVector &VLSGrps) override {
    assert(VC && "VectorContext not set.");
    VLS->computeVLSGroups(VLSMrfs, *VC, VLSGrps);
  }
};

/// \brief Manage exploration of vectorization candidates within a region.
class VPOScenarioEvaluationBase {
private:
  /// AVR Region at hand.
  AVRWrn *AWrn;

  /// AVRLoop in AVR region.
  AVRLoop *ALoop;

  /// Provide cost evaluation utilities for the region.
  VPOCostModelBase CM;

public:
  VPOScenarioEvaluationBase() {}
  virtual ~VPOScenarioEvaluationBase() {}
  void setAWrn(AVRWrn *AvrWrn) { AWrn = AvrWrn; }
  void setALoop(AVRLoop *L) { ALoop = L; } 

  // Functions with a base-level implementation common to both underlying IRs

  /// \brief Decide if/how to vectorize a region in terms of profitability.
  /// Evaluates several vectorization candidates and selects the best one.
  /// Navigates through the search-space of vectorization candidates,
  /// preparing the Avr of each candidate along the way, and obtaining
  /// a cost for each. Upon completion the best candidate is returned;
  /// The outcome of evaluation of all candidates is the VecContext of
  /// the selected candidate, null if no candidate found (vectorization
  /// is not profitable compared to scalar version). The VecContext
  /// contains the Aloops and Vectorization Factors selected for vectorization.
  VPOVecContextBase *getBestCandidate(AVRWrn *AvrWrn);

  /// Gather loop-level information (memory references, data-dependencs) and
  /// drive the exploration of alternative Vectorization Factors for a given
  /// AVRLoop in a region.
#ifdef USEALOOP
  VPOVecContextBase *processLoop(AVRLoop *ALoop, int *Cost);
#else
  VPOVecContextBase *processLoop(AVRWrn *AvrWrn, int *Cost);
#endif

  /// \brief Analyze a specific vectorization candidate, namely a specific
  /// \p ALoop and \p VF (Vectorization factor). Additional information
  /// (mostly at level of underlying IR) is passed in the \p VC (VecContext). 
  // TODO: Ideally we have very few VF sensitive adjusments to make.
  // processCandidate is as much as possible just a getCost call.
  // TODO: Support a scenario which consists of more than one loop.
  // TODO: Decide how we indicate for this function which Loops and VFs to 
  // consider (possibly directly/explicitely in the AVR?). Looks like we will 
  // not use VecContext for this purpose. Need to finalize what vecContext 
  // will be used for. Currently it is used to pass underlying-ir level 
  // information.
  int processCandidate(AVRLoop *ALoop, unsigned int VF, VPOVecContextBase &VC);

  /// \brief Analyze which Vectorization Factors make sense for the loop (in 
  /// terms of target support and data-types operated on in the loop).
  void findVFCandidates(VFsVector &VFCandidates) const;

// Functions to be implemented at the underlying IR level

#ifdef USEALOOP
  virtual void setLoop(AVRLoop *ALoop) = 0;
#else
  virtual void setLoop(AVRWrn *AvrWrn) = 0;
#endif
  virtual void gatherMemrefsInLoop() = 0;
  virtual VPODataDepInfoBase getDataDepInfoForLoop() = 0;
  virtual VPOVLSInfoBase *getVLSInfoForCandidate() = 0;
  virtual VPOVecContextBase &setVecContext(unsigned VF) = 0;
  virtual void resetLoopInfo() = 0;
};

/// LLVMIR ScenarioEvaluation. Currently an empty implementation.
// TODO.
class VPOScenarioEvaluation : public VPOScenarioEvaluationBase {
private:
  VPODataDepInfo VPODDG;
  VPOVecContext VC;

public:
  VPOScenarioEvaluation() {}
  ~VPOScenarioEvaluation() {}

  /// Obtain the underlying loop.
#ifdef USEALOOP
  void setLoop(AVRLoop *ALoop) override { return; }
#else
  void setLoop(AVRWrn *AvrWrn) override { return; }
#endif

  /// Gather the memory references in the loop.
  void gatherMemrefsInLoop() override { return; }

  /// Obtain data-dependence information for the loop.
  VPODataDepInfoBase getDataDepInfoForLoop() override { return VPODDG; }

  /// Obtain a handle to VLS-analysis services.
  VPOVLSInfoBase *getVLSInfoForCandidate() override { return new VPOVLSInfo(); }

  /// Set the \p VC to a specific vectorization candidate.
  VPOVecContextBase &setVecContext(unsigned VF) override {
    VC = VPOVecContext(VF);
    return VC;
  }

  void resetLoopInfo() override { return; }
};

/// HIR ScenarioEvaluation
class VPOScenarioEvaluationHIR : public VPOScenarioEvaluationBase {
private:
  HIRDDAnalysis *DDA;
  HIRVectVLSAnalysis *VLS;

  /// \name Information about the loop currently under consideration.
  /// These data-structures are initially empty. We set them per loop.
  /// @{
  // TODO: After we finalize which of these members we really want to keep,
  // consider creating an object to encapsulate all LoopContext stuff.
  HLLoop *Loop;
  LoopMemrefsVector LoopMemrefs;
  VPODataDepInfoHIR VPODDG;
  /// @}

  /// \name Information about the specific vectorization candidate currently
  /// under consideration (namely: loop + VF). It is initially empty. We set
  /// it per candidate.
  /// @{
  // CHECKME: keep this member internally, or pass around at base level?
  VPOVecContextHIR VC;
  /// @}

public:
  VPOScenarioEvaluationHIR(HIRDDAnalysis *DDA, HIRVectVLSAnalysis *VLS)
      : DDA(DDA), VLS(VLS), Loop(nullptr) {}
  ~VPOScenarioEvaluationHIR() {}

#ifdef USEALOOP
  // CHECKME: This code is not working (no HLLoop is obtained).
  void setLoop(AVRLoop *ALoop) override {
    assert(isa<AVRLoopHIR>(*ALoop) && "Loop not set.");
    AVRLoopHIR *AHLoop = cast<AVRLoopHIR>(ALoop);
    Loop = nullptr;
    Loop = const_cast<HLLoop *>(AHLoop->getLoop());
    assert(Loop && "Null HLLoop.");
  }
#else
  void setLoop(AVRWrn *AvrWrn) override {
    WRNVecLoopNode *WVecNode = AvrWrn->getWrnNode();
    Loop = nullptr;
    Loop = WVecNode->getHLLoop();
    assert(Loop && "Null HLLoop.");
  }
#endif

  // CHECKME: pass AVRLoop as parameter instead of keeping the underlying loop
  // internally?
  void gatherMemrefsInLoop() override {
    assert(Loop && "Loop not set.");
    HIRVectVLSAnalysis::gatherMemrefsInLoop(Loop, LoopMemrefs);
    // TODO: mark that Memrefs are now valid.
  }

  // FIXME: We are returning VPODDG and also keeping it inside... Decide how we
  // want to handle the DataDepsInfo.
  // CHECKME: pass AVRLoop as parameter instead of keeping the underlying loop
  // internally?
  VPODataDepInfoBase getDataDepInfoForLoop() override {
    assert(Loop && "Loop not set.");
    VPODDG = VPODataDepInfoHIR(DDA, Loop);
    // TODO: mark that VPODDG is now valid.
    return VPODDG;
  }

  // CHECKME: pass AVRLoop as parameter instead of keeping the underlying loop
  // internally? Same for VC...
  VPOVLSInfoBase *getVLSInfoForCandidate() override {
    // TODO: check that VC is valid
    // TODO: check that LoopMemrefs have been gathered
    return new VPOVLSInfoHIR(VLS, &VC, LoopMemrefs);
  }

  VPOVecContextHIR &setVecContext(unsigned VF) override {
    VC = VPOVecContextHIR(VF, VPODDG.getDDG(), Loop);
    // TODO: mark that VC is now valid
    return VC;
  }

  void resetLoopInfo() override { LoopMemrefs.clear(); }
};

} // End namespace vpo
} // End namespace llvm

#endif // LLVM_ANALYSIS_VPO_VPOSCENARIOEVALUATION_H
