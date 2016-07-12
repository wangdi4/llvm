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
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOSIMDLaneEvolution.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOVecContext.h"
#include "llvm/Analysis/TargetTransformInfo.h"

namespace llvm {
namespace vpo {

typedef SmallVector<unsigned int, 6> VFsVector;
typedef VFsVector::iterator LoopVFsIt;

class VPOVecContextBase;

// ----------------- VPOCostModelGatherer ------------------ //
/// \brief Visitor Class for accumulating the cost of an AVR Loop.
// TODO: Extend to support if-then-else
// TODO: Extend to non-innermost loops.
class VPOCostGathererBase {
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

protected:
  /// \brief A handle to Target Information
  const TargetTransformInfo &TTI;

  /// \brief Vectorization Factor.
  unsigned int VF;

  /// \brief The AvrLoop currently being evaluated.
  // CHECKME: Maybe switch to keeping the IR/HIR-Loops in the derived classes
  // instead.
  AVRLoop *ALoop;

public:
  VPOCostGathererBase(const TargetTransformInfo &TTI, unsigned int VF,
                      AVRLoop *ALoop)
      : TTI(TTI), VF(VF), ALoop(ALoop) {
    LoopBodyCost = 0;
    OutOfLoopCost = 0;
  }

  /// \name Get the costs associated with vectorizing the AvrLoop currently
  /// under consideration.
  /// @{
  unsigned int getLoopBodyCost() { return LoopBodyCost; }
  unsigned int getOutOfLoopCost() { return OutOfLoopCost; }
  /// @}

  /// \name Visit Functions
  /// @{
  bool isDone() { return false; }
  bool skipRecursion(AVR *ANode);

  void visit(AVR *ANode);
  void postVisit(AVR *ANode) {}

  void visit(AVRLoop *Loop);
  void postVisit(AVRLoop *Loop);

  void visit(AVRAssign *Assign);
  void postVisit(AVRAssign *Assign);

  void visit(AVRExpression *Expr);
  void postVisit(AVRExpression *Expr);

  void visit(AVRValue *AValue);
  void postVisit(AVRValue *AValue);

  // Following are soon to become expressions in an Assign
  void visit(AVRLabel *Label);
  void visit(AVRPhi *Phi);
  void visit(AVRBranch *Branch);
  void visit(AVRCompare *Compare);
  void visit(AVRIf *If);
  void visit(AVRSelect *Select);
  void visit(AVRCall *Call);
  /// @}

  /// \brief Wrapper to calling the TTI utility for gather/scatter cost.
  /// Checks whether the indexes to the gather/scatter can fit in 32bit.
  virtual int getGatherScatterOpCost(unsigned Opcode, Type *SrcVTy,
                                     AVRValue *Ptr, bool VariableMask,
                                     unsigned Alignment) = 0;

  /// \brief Check whether the address computation for a non-consecutive memory
  /// access looks like an unlikely candidate for being merged into the indexing
  /// mode.
  // In LLVMIR we check that exactly one of the GEP indices is an induction
  // variable with a small enough stride, and the rest are invariant.
  // In HIR this probably translates to checking that we have an evolution
  // only in one dimension (i.e. in one subscript).
  virtual bool isLikelyComplexAddressComputation(AVRValue *Ptr) = 0;
};

/// LLVMIR CostGatherer
class VPOCostGatherer : public VPOCostGathererBase {
public:
  VPOCostGatherer(const TargetTransformInfo &TTI, unsigned int VF,
                  AVRLoop *ALoop)
      : VPOCostGathererBase(TTI, VF, ALoop) {
    assert(isa<AVRLoopIR>(*ALoop) && "Loop not set.");
  }

  int getGatherScatterOpCost(unsigned Opcode, Type *SrcVTy, AVRValue *Ptr,
                             bool VariableMask, unsigned Alignment) override {
    assert(isa<AVRValueIR>(Ptr) && "not AVRValueIR?");
    const Value *ConstPtrVal = (cast<AVRValueIR>(Ptr))->getLLVMValue();
    Value *PtrVal = const_cast<Value *>(ConstPtrVal);
    return TTI.getGatherScatterOpCost(Opcode, SrcVTy, PtrVal, VariableMask,
                                      Alignment);
  }

  // TODO. The LLVMIR implementation in LoopVectorizer requires checking if
  // Legal->isInductionVariable(Opd) on the GEP indices. Also the LLVMIR
  // implementation in LoopVectorize requires ScalarEvolution Analysis. Can we
  // use SLEV instead of Leval and ScalarEvolution?
  bool isLikelyComplexAddressComputation(AVRValue *Ptr) {
    return false; // FIXME.
  }
};

/// HIR CostGatherer
class VPOCostGathererHIR : public VPOCostGathererBase {
public:
  VPOCostGathererHIR(const TargetTransformInfo &TTI, unsigned int VF,
                     AVRLoop *ALoop)
      : VPOCostGathererBase(TTI, VF, ALoop) {
    assert(isa<AVRLoopHIR>(*ALoop) && "Loop not set.");
  }

  // TODO: Need to figure out if the gather/scatter indices can fit in 32bit;
  // Also, need to introduce a new TTI API that takes in as input the index
  // width and the pointer type (instead of taking as input the pointer itself).
  //
  // To figure out the width of the gather/scatter indices:
  // For a random access, such as A[B[i]], check that the type of B[i] time VF
  // fits in 32bit. For a strided access, check that the stride times VF fits
  // in 32bit.
  int getGatherScatterOpCost(unsigned Opcode, Type *SrcVTy, AVRValue *Ptr,
                             bool VariableMask, unsigned Alignment) override {
    return VF; // FIXME
  }

  // TODO.
  bool isLikelyComplexAddressComputation(AVRValue *Ptr) {
    return false; // FIXME.
  }
};

// ----------------- VPOCostModel ------------------ //
/// \brief CostModel Utilities for evaluation of vectorization profitability.
// TODO: Provide cost utilities for different granularities? (AVRWrn,
// AVRLoop, BasicBlock?).
class VPOCostModelBase {
protected:
  /// \brief Region being evaluated.
  // TODO: We are currently not taking advantage of information across
  // different loops within a region. Currently the scope of our passes
  // (cost model, SLEV) is an individual loop. The intention in the future
  // is to look at multiple loops together.
  AVRWrn *AWrn;

  /// \brief A handle to Target Information
  const TargetTransformInfo *TTI;

public:
  VPOCostModelBase(AVRWrn *AWrn, const TargetTransformInfo *TTI)
      : AWrn(AWrn), TTI(TTI) {}

  /// \brief Calculate a cost for the given \p ALoop assuming the Vectorization
  /// Factor is \p VF.
  // TODO: Consider also VLS group information
  // TODO: Calculate a cost at the scope of a region, taking advantage of
  // information about multiple loops in a region at once. Currently our scope
  // is one ALoop at a time. For that we'll also need to finalize how we really
  // want to encode the results of the CostModel, namely, which ALoops in the
  // region to vectorize and using which VFs (directly/explicitely in the
  // AVR?...)
  int getCost(AVRLoop *ALoop, unsigned int VF);

  virtual VPOCostGathererBase &getCostGatherer(unsigned int VF,
                                               AVRLoop *ALoop) = 0;
};

/// LLVMIR CostMddel
class VPOCostModel : public VPOCostModelBase {
public:
  VPOCostModel(AVRWrn *AWrn, const TargetTransformInfo *TTI)
      : VPOCostModelBase(AWrn, TTI) {
    CostGatherer = nullptr;
  }

  VPOCostGathererBase &getCostGatherer(unsigned int VF,
                                       AVRLoop *ALoop) override {
    assert(TTI && "no TTI Info");
    // Pass the underlying LLVMIR Loop instead
    CostGatherer = new VPOCostGatherer(*TTI, VF, ALoop);
    return *CostGatherer;
  }

private:
  VPOCostGatherer *CostGatherer;
};

/// HIR CostModel
class VPOCostModelHIR : public VPOCostModelBase {
public:
  VPOCostModelHIR(AVRWrn *AWrn, const TargetTransformInfo *TTI)
      : VPOCostModelBase(AWrn, TTI) {
    CostGatherer = nullptr;
  }

  VPOCostGathererBase &getCostGatherer(unsigned int VF,
                                       AVRLoop *ALoop) override {
    assert(TTI && "no TTI Info");
    // Pass the underlying HLLoop instead
    CostGatherer = new VPOCostGathererHIR(*TTI, VF, ALoop);
    return *CostGatherer;
  }

private:
  VPOCostGathererHIR *CostGatherer;
};

// ----------------- VPODataDepInfo ------------------ //
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

// ----------------- VPOVLSInfo ------------------ //
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

// ----------------- VPOScenarioEvaluation ------------------ //
/// \brief Manage exploration of vectorization candidates within a region.
class VPOScenarioEvaluationBase {
protected:
  /// Handle to Target Information
  const TargetTransformInfo *TTI;

  /// AVR Region at hand.
  AVRWrn *AWrn;

private:
  /// AVRLoop in AVR region.
  // FIXME: Get rid of this member (given that we also pass the ALoop explicitly
  // to processLoop)?
  AVRLoop *ALoop;

public:
  VPOScenarioEvaluationBase(AVRWrn *AWrn, const TargetTransformInfo *TTI)
      : TTI(TTI), AWrn(AWrn) {}

  virtual ~VPOScenarioEvaluationBase() {}

  /// \brief Set the AVR Loop currently being evaluated.
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
  VPOVecContextBase *processLoop(AVRLoop *ALoop, int *Cost);

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

  virtual void setLoop(AVRLoop *ALoop) = 0;
  virtual void gatherMemrefsInLoop() = 0;
  virtual VPODataDepInfoBase getDataDepInfoForLoop() = 0;
  virtual VPOVLSInfoBase *getVLSInfoForCandidate() = 0;
  virtual VPOVecContextBase &setVecContext(unsigned VF) = 0;
  virtual SIMDLaneEvolutionAnalysisUtilBase &getSLEVUtil() = 0;
  virtual void resetLoopInfo() = 0;
  virtual VPOCostModelBase *getCM() = 0;
};

/// LLVMIR ScenarioEvaluation. Currently an empty implementation.
// TODO.
class VPOScenarioEvaluation : public VPOScenarioEvaluationBase {
private:
  VPODataDepInfo VPODDG;
  VPOVecContext VC;
  SIMDLaneEvolutionAnalysisUtil SLEVUtil;

  /// Provide cost evaluation utilities for the region.
  VPOCostModel CM;

public:
  VPOScenarioEvaluation(AVRWrn *AvrWrn, const TargetTransformInfo *TTI,
                        AvrDefUse &DU)
      : VPOScenarioEvaluationBase(AvrWrn, TTI), SLEVUtil(DU), CM(AWrn, TTI) {}
  ~VPOScenarioEvaluation() {}

  /// Obtain the underlying loop.
  void setLoop(AVRLoop *ALoop) override { return; }

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

  SIMDLaneEvolutionAnalysisUtilBase &getSLEVUtil() override { return SLEVUtil; }

  void resetLoopInfo() override { return; }

  VPOCostModelBase *getCM() { return &CM; }
};

/// HIR ScenarioEvaluation
class VPOScenarioEvaluationHIR : public VPOScenarioEvaluationBase {
private:
  HIRDDAnalysis *DDA;
  HIRVectVLSAnalysis *VLS;
  SIMDLaneEvolutionAnalysisUtilHIR SLEVUtil;

  /// Provide cost evaluation utilities for the region.
  VPOCostModelHIR CM;

  /// \name Information about the loop currently under consideration.
  /// These data-structures are initially empty. We set them per loop.
  /// @{
  // TODO: After we finalize which of these members we really want to keep,
  // consider creating an object to encapsulate all LoopContext stuff.
  // TODO: In the future we want to be able to consider multiple loops in a
  // region, together, so this would change.
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
  VPOScenarioEvaluationHIR(AVRWrn *AvrWrn, HIRDDAnalysis *DDA,
                           HIRVectVLSAnalysis *VLS, AvrDefUseHIR &DU,
                           const TargetTransformInfo *TTI)
      : VPOScenarioEvaluationBase(AvrWrn, TTI), DDA(DDA), VLS(VLS),
        SLEVUtil(DU), CM(AWrn, TTI), Loop(nullptr) {}
  ~VPOScenarioEvaluationHIR() {}

  void setLoop(AVRLoop *ALoop) override {
    assert(isa<AVRLoopHIR>(*ALoop) && "Loop not set.");
    AVRLoopHIR *AHLoop = cast<AVRLoopHIR>(ALoop);
    Loop = nullptr;
    Loop = const_cast<HLLoop *>(AHLoop->getLoop());
    assert(Loop && "Null HLLoop.");
  }

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

  SIMDLaneEvolutionAnalysisUtilBase &getSLEVUtil() override { return SLEVUtil; }

  void resetLoopInfo() override { LoopMemrefs.clear(); }

  VPOCostModelBase *getCM() { return &CM; }
};

} // End namespace vpo
} // End namespace llvm

#endif // LLVM_ANALYSIS_VPO_VPOSCENARIOEVALUATION_H
