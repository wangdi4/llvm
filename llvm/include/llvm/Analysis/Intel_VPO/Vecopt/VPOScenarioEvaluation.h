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
// Just a placeholder for now.
// TODO: Provide cost utilities for different granularities? (AVRWrn,
// AVRLoop, BasicBlock?).
class VPOCostModelBase {
private:
  AVRWrn *AWrn;

public:
  VPOCostModelBase(AVRWrn *AWrn) : AWrn(AWrn) {}
  VPOCostModelBase() {}

  /// \brief Calculate a cost for the given AVR region, according to the
  // information in the Vectorization Context, namely which Aloops in the
  // region are vectorized and how (using which Vectorization Factor).
  // TODO: Consider also VLS group information
  int getCost(const VPOVecContextBase *VC) const;
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

  /// Analyze a specific vectorization candidate (Loop and Vectorization
  /// Factor) as indicated by the \p VC.
  // TODO: Ideally we have very few VF sensitive adjusments to make.
  // processCandidate is as much as possible just a getCost call.
  int processCandidate(VPOVecContextBase &VC);

  /// Analyze which Vectorization Factors make sense for the loop (in terms of
  /// target support and data-types operated on in the loop).
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
};

/// LLVMIR ScenarioEvaluation. Currently an empty implementation.
// TODO.
class VPOScenarioEvaluation : public VPOScenarioEvaluationBase {
private:
  VPODataDepInfo VPODDG;
  VPOVecContext VC;

public:
  VPOScenarioEvaluation() {}

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
    LoopMemrefs.clear();
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
};

} // End namespace vpo
} // End namespace llvm

#endif // LLVM_ANALYSIS_VPO_VPOSCENARIOEVALUATION_H
