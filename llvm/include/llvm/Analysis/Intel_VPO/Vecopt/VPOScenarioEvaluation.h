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
#include "llvm/Analysis/Intel_OptVLSClientUtils.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrGenerate.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrDecomposeHIR.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOSIMDLaneEvolution.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOVecContext.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Transforms/Intel_VPO/Vecopt/VPOAvrLLVMCodeGen.h"
#include "llvm/Transforms/Intel_VPO/Vecopt/VPOAvrHIRCodeGen.h"

namespace llvm {
namespace vpo {

typedef SmallVector<unsigned int, 6> VFsVector;
typedef VFsVector::iterator LoopVFsIt;

class VPOVecContextBase;


// ----------------- VPOVLSInfo ------------------ //
/// \brief Utilities for Vector-Load-Store (VLS) Optimization Analysis:
/// gather information about the memory accesses in the Loop and the proximity
/// relations between them.
class VPOVLSInfoBase {
public:
  /// Discriminator for LLVM-style RTTI (dyn_cast<> et al.)
  enum VPOVLSInfoKind {
    VPOVLSK_LLVMIR,
    VPOVLSK_HIR
  };

protected:
  /// \brief Mapping from Memrefs to the VLS-Groups they belong to.
  /// A memref that has a mapping to a VLS group is usually a strided or an
  /// indexed memory reference, that can be either vectorized individually
  /// using a gather/scatter, or can be vectorized together with its neighbors 
  /// in the VLS-Group using wide loads/stores and shuffles. 
  OVLSMemrefToGroupMap MemrefToGroupMap;

private:
  const VPOVLSInfoKind Kind;

public:
  VPOVLSInfoBase(VPOVLSInfoKind K) : Kind(K) {}
  virtual ~VPOVLSInfoBase() { MemrefToGroupMap.clear(); }

  /// \brief Analyze the loop memory references with respect to VectorContext.
  /// \param [out] VLSMrfs holds objects that can provide services
  /// about these memory references (such as information on access-pattern,
  /// distances, safety of moving).
  virtual void analyzeVLSMemrefsInLoop(OVLSMemrefVector &VLSMrfs) = 0;

  /// \name Helpers to find the VLS Group that a memref belongs to. We have
  /// a mapping from underlying-IR accesses to their respective OVLSMemref
  /// object, and from the latter to the VLS Group it is a member in.
  /// @{
  virtual OVLSMemref *getVLSMemrefInfoForAccess(const AVRValue *Ptr) = 0;
  virtual OVLSGroup *getVLSGroupInfoForVLSMemref(OVLSMemref *Mrf) = 0;
  virtual OVLSGroup *getVLSGroupInfoForAccess(const AVRValue *Ptr) = 0;
  /// }@

  /// Analyze proximity between the memory references given in \p VLSMrfs.
  /// \param [out] VLSGrps holds the proximity groups found.
  virtual void analyzeVLSGroupsInLoop(OVLSMemrefVector &VLSMrfs,
                                      OVLSGroupVector &VLSGrps) = 0;

  VPOVLSInfoKind getKind() const { return Kind; }
};

/// LLVMIR VLSInfo. Currently an empty implmentation.
// TODO.
class VPOVLSInfo : public VPOVLSInfoBase {
public:
  VPOVLSInfo() : VPOVLSInfoBase(VPOVLSK_LLVMIR) {}
  ~VPOVLSInfo() {}

  void analyzeVLSMemrefsInLoop(OVLSMemrefVector &VLSMrfs) override {}

  OVLSMemref *getVLSMemrefInfoForAccess(const AVRValue *Ptr) override { 
    return nullptr; 
  }
  OVLSGroup *getVLSGroupInfoForVLSMemref(OVLSMemref *Mrf) override {
    return nullptr;
  }
  OVLSGroup *getVLSGroupInfoForAccess(const AVRValue *Ptr) override {
    return nullptr;
  }

  void analyzeVLSGroupsInLoop(OVLSMemrefVector &VLSMrfs,
                              OVLSGroupVector &VLSGrps) override {}

  static bool classof(const VPOVLSInfoBase *VLSInfo) {
    return VLSInfo->getKind() == VPOVLSK_LLVMIR;
  }
};

/// HIR VLSInfo.
class VPOVLSInfoHIR : public VPOVLSInfoBase {
private:
  HIRVectVLSAnalysis *VLS;

  /// Information about the Vectorization Context currently considered, namely
  /// the loop, Vectorization Factor, and data dependence graph.
  VPOVecContextHIR *VC;

  /// The memory references of the loop in the underlying IR (HIR RegDDRefs).
  LoopMemrefsVector *LoopMemrefs;

  /// A mapping from the underlying IR Memrefs (HIR RegDDRefs) to their 
  /// respective OVLSMemref objects (the result of HIR-level memory access
  /// pattern analysis; to be used by the VLS-group analysis).
  HIRToVLSMemrefsMap MemrefsMap;

public:
  VPOVLSInfoHIR(HIRVectVLSAnalysis *VLS, VPOVecContextHIR *VC,
                LoopMemrefsVector &Refs)
      : VPOVLSInfoBase(VPOVLSK_HIR), VLS(VLS), VC(VC), LoopMemrefs(&Refs) {}
  ~VPOVLSInfoHIR() { MemrefsMap.clear(); }

  void analyzeVLSMemrefsInLoop(OVLSMemrefVector &VLSMrfs) override {
    assert(VC && "VectorContext not set.");
    VLS->analyzeVLSMemrefsInLoop(*VC, *LoopMemrefs, VLSMrfs, MemrefsMap);
  }

  OVLSMemref *getVLSMemrefInfoForAccess(const AVRValue *Ptr) override {
    assert(isa<AVRValueHIR>(Ptr) && "not AVRValueHIR?");
    const AVRValueHIR *ConstHIRPtr = cast<AVRValueHIR>(Ptr);
    AVRValueHIR *HIRPtr = const_cast<AVRValueHIR *>(ConstHIRPtr);
    // FIXME: AVRValueHIR may not be a RegDDRef
    if (RegDDRef *Ref = dyn_cast<RegDDRef>(HIRPtr->getValue())) {
      assert(Ref && "no RegDDRef for AVRValueHIR");
      return getVLSMemrefInfoForAccess(Ref);
    }
    return nullptr;
  }

  OVLSGroup *getVLSGroupInfoForVLSMemref(OVLSMemref *Mrf) override {
    auto MemrefToGroupIt = MemrefToGroupMap.find(Mrf);
    OVLSGroup *Grp = nullptr;
    if (MemrefToGroupIt != MemrefToGroupMap.end()) 
      Grp = MemrefToGroupIt->second; 
    return Grp;
  }

  OVLSGroup *getVLSGroupInfoForAccess(const AVRValue *Ptr) override {
    OVLSMemref *Mrf = getVLSMemrefInfoForAccess(Ptr);
    assert(Mrf && "No OVLSMemref object found for Ptr");
    OVLSGroup *Grp = getVLSGroupInfoForVLSMemref(Mrf);
    return Grp;
  }

  void analyzeVLSGroupsInLoop(OVLSMemrefVector &VLSMrfs,
                              OVLSGroupVector &VLSGrps) override {
    assert(VC && "VectorContext not set.");
    VLS->computeVLSGroups(VLSMrfs, *VC, VLSGrps, &MemrefToGroupMap);
  }

  static bool classof(const VPOVLSInfoBase *VLSInfo) {
    return VLSInfo->getKind() == VPOVLSK_HIR;
  }

private:
  OVLSMemref *getVLSMemrefInfoForAccess(const RegDDRef *Ref) {
    auto MemRefIt = MemrefsMap.find(Ref);
    if (MemRefIt == MemrefsMap.end())
      return nullptr;
    OVLSMemref *Mrf = MemRefIt->second;
    assert(Mrf && " VLSMemref info not found for Ref.");
    return Mrf; 
  }
};


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

  /// The minimum/maximum bit widths of types of values loaded/stored in the
  /// loop. We only look at loads/stores for now.
  unsigned int MinBitWidth;
  unsigned int MaxBitWidth;

protected:
  /// \brief A handle to Target Information
  const TargetTransformInfo &TTI;

  /// \brief A handle to Target Library Info
  const TargetLibraryInfo &TLI;

  /// \brief A handle to Data Layout Info
  const DataLayout &DL;

  /// \brief Vectorization Factor.
  unsigned int VF;

  /// \brief The AvrLoop currently being evaluated.
  // CHECKME: Maybe switch to keeping the IR/HIR-Loops in the derived classes
  // instead.
  AVRLoop *ALoop;

public:
  VPOCostGathererBase(const TargetTransformInfo &TTI, 
                      const TargetLibraryInfo &TLI,
                      const DataLayout &DL,
                      unsigned int VF,
                      AVRLoop *ALoop)
    : TTI(TTI), TLI(TLI), DL(DL), VF(VF), ALoop(ALoop) {
    LoopBodyCost = 0;
    OutOfLoopCost = 0;

    // Initialize to reasonable values - a loop may not see a
    // load/store. This avoids issues such as divide by zero.
    // Consider the loop
    //    DO i2 = 
    //        %incdec.ptr = &((%workarea)[0][i2 + 1]);
    //    END LOOP
    // LHS exprs are currently being skipped and RHS is an address
    // computation
    MinBitWidth = 64;
    MaxBitWidth = 8;
  }
  virtual ~VPOCostGathererBase() {}

  /// \name Get the costs associated with vectorizing the AvrLoop currently
  /// under consideration.
  /// @{
  unsigned int getLoopBodyCost() { return LoopBodyCost; }

  unsigned int getMinBitWidth() const { return MinBitWidth; }
  unsigned int getMaxBitWidth() const { return MaxBitWidth; }

  unsigned int getOutOfLoopCost() { return OutOfLoopCost; }
  void addOutOfLoopCost(unsigned int AddCost) { OutOfLoopCost += AddCost; }
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
  void visit(AVRPredicate *Predicate);
  /// @}

  /// \brief Wrapper to calling the TTI utility for gather/scatter cost.
  /// Checks whether the indexes to the gather/scatter can fit in 32bit.
  virtual int getGatherScatterOpCost(unsigned Opcode, Type *SrcVTy, AVR *Ptr,
                                     bool VariableMask, unsigned Alignment) = 0;

  /// \brief Check whether the address computation for a non-consecutive memory
  /// access looks like an unlikely candidate for being merged into the indexing
  /// mode.
  // In LLVMIR we check that exactly one of the GEP indices is an induction
  // variable with a small enough stride, and the rest are invariant.
  // In HIR this probably translates to checking that we have an evolution
  // only in one dimension (i.e. in one subscript).
  virtual bool isLikelyComplexAddressComputation(AVR *Ptr) = 0;

  /// \brief Return a handle to the results of VLS analysis: namely a mapping
  /// from loads/stores to the VLS Group (Group of neighbouring loads/stores)
  /// they belong to. The cost gatherer needs to take that into account when
  /// computing the cost of vectorizing a load/store: the cost of vectorizing
  /// neighboring loads/stores together as a group (using continguous vector
  /// loads/stores and optimized shuffle sequences) may be better then the
  /// cost of vectorizing each of them seperately using gathers/scatters. 
  virtual VPOVLSInfoBase *getVLSInfo() const = 0;

  /// \brief Return a handle to the TTI cost-model utility that provides
  /// the costs for the building blocks of the optimized code sequences
  /// for vectorizing a VLS Group (Group of neighbouring loads/stores).
  virtual OVLSTTICostModel *getVLSCostModel() const = 0;
};

/// LLVMIR CostGatherer
class VPOCostGatherer : public VPOCostGathererBase {
public:
  VPOCostGatherer(const TargetTransformInfo &TTI,
                  const TargetLibraryInfo &TLI,
                  const DataLayout &DL, unsigned int VF,
                  AVRLoop *ALoop, OVLSTTICostModelLLVMIR *TTICM, 
                  VPOVLSInfo *VLSInfo)
    : VPOCostGathererBase(TTI, TLI, DL, VF, ALoop), TTICM(TTICM),
                            VLSInfo(VLSInfo) {
    assert(isa<AVRLoopIR>(*ALoop) && "Loop not set.");
  }
  ~VPOCostGatherer() {}

  // CHECKME: Can we rely on having the underlying LLVM value available?
  int getGatherScatterOpCost(unsigned Opcode, Type *SrcVTy, AVR *Ptr,
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
  bool isLikelyComplexAddressComputation(AVR *Ptr) {
    return false; // FIXME.
  }

  VPOVLSInfoBase *getVLSInfo() const { return VLSInfo; }

  OVLSTTICostModel *getVLSCostModel() const override {
    assert(TTICM && "No CostModel set for VLS");
    return TTICM;
  }

private:
  OVLSTTICostModelLLVMIR *TTICM;
  VPOVLSInfo *VLSInfo;
};

/// HIR CostGatherer
class VPOCostGathererHIR : public VPOCostGathererBase {
public:
  VPOCostGathererHIR(const TargetTransformInfo &TTI,
                     const TargetLibraryInfo &TLI,
                     const DataLayout &DL, unsigned int VF,
                     AVRLoop *ALoop, OVLSTTICostModelHIR *TTICM, 
                     VPOVLSInfoHIR *VLSInfo)
    : VPOCostGathererBase(TTI, TLI, DL, VF, ALoop), TTICM(TTICM),
                            VLSInfo(VLSInfo) {
    assert(isa<AVRLoopHIR>(*ALoop) && "Loop not set.");
  }
  ~VPOCostGathererHIR() {}

  // TODO: Need to figure out if the gather/scatter indices can fit in 32bit;
  // Also, need to introduce a new TTI API that takes in as input the index
  // width and the pointer type (instead of taking as input the pointer itself).
  //
  // To figure out the width of the gather/scatter indices:
  // For a random access, such as A[B[i]], check that the type of B[i] time VF
  // fits in 32bit. For a strided access, check that the stride times VF fits
  // in 32bit.
  //
  // Alternatively, find the respective underlying LLVM Value.
  // CHECKME: Can we rely on having the underlying LLVM value available?
  int getGatherScatterOpCost(unsigned Opcode, Type *SrcVTy, AVR *Ptr,
                             bool VariableMask, unsigned Alignment) override {

    if (AVRValueHIR *AValHIR = dyn_cast<AVRValueHIR>(Ptr)) {
      // Obtain the LLVMIR Pointer Value
      const HLNode *Node = AValHIR->getNode();
      const HLInst *INode = dyn_cast<HLInst>(Node);
      const Instruction *CurInst = INode->getLLVMInstruction();
      Instruction *I = const_cast<Instruction *>(CurInst);
      StoreInst *SI = dyn_cast<StoreInst>(I);
      LoadInst *LI = dyn_cast<LoadInst>(I);
      Value *PtrVal = SI ? SI->getPointerOperand() : LI->getPointerOperand();

      // Call the TTI routine
      return TTI.getGatherScatterOpCost(Opcode, SrcVTy, PtrVal, VariableMask,
                                        Alignment);
    } else { // TODO: Implicit loads
      return 0;
    }
  }

  // TODO.
  bool isLikelyComplexAddressComputation(AVR *Ptr) {
    return false; // FIXME.
  }

  VPOVLSInfoBase *getVLSInfo() const { return VLSInfo; }

  OVLSTTICostModel *getVLSCostModel() const override {
    assert(TTICM && "No CostModel set for VLS");
    return TTICM;
  }

private:
  OVLSTTICostModelHIR *TTICM;
  VPOVLSInfoHIR *VLSInfo;
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
  const TargetTransformInfo &TTI;
  const TargetLibraryInfo &TLI;
  
  /// \brief Cost for one iteration of ScalarLoop
  unsigned int ScalarIterCost;
public:
  VPOCostModelBase(AVRWrn *AWrn, const TargetTransformInfo &TTI,
                   const TargetLibraryInfo &TLI)
    : AWrn(AWrn), TTI(TTI), TLI(TLI), ScalarIterCost(0) {}

  /// \brief Calculate a cost for the given \p ALoop assuming the Vectorization
  /// Factor is \p VF.
  // TODO: Consider also VLS group information
  // TODO: Calculate a cost at the scope of a region, taking advantage of
  // information about multiple loops in a region at once. Currently our scope
  // is one ALoop at a time. For that we'll also need to finalize how we really
  // want to encode the results of the CostModel, namely, which ALoops in the
  // region to vectorize and using which VFs (directly/explicitely in the
  // AVR?...)
  // The minimum/maximum bit width of values loaded/stored are returned in
  // MinBitWidthP/MaxBitWidthP when non-null. 
  uint64_t getCost(AVRLoop *ALoop, unsigned int VF, VPOVLSInfoBase *VLSInfo,
                   unsigned int *MinBitWidthP = nullptr,
                   unsigned int *MaxBitWidthP = nullptr);

  virtual VPOCostGathererBase *getCostGatherer(unsigned int VF, AVRLoop *ALoop, 
                                               VPOVLSInfoBase *VLSInfo) = 0;

  // \brief Obtain the cost of aligning the loop trip count to the \p VF
  virtual int getRemainderLoopCost(unsigned int VF, uint64_t &ConstTripCount) = 0;
};

/// LLVMIR CostModel
class VPOCostModel : public VPOCostModelBase {
public:
  VPOCostModel(AVRWrn *AWrn, const TargetTransformInfo &TTI, 
               const TargetLibraryInfo &TLI, LLVMContext &LLVMCntxt)
      : VPOCostModelBase(AWrn, TTI, TLI), CG(nullptr),
                         VLSCostModel(TTI, LLVMCntxt) {
    CostGatherer = nullptr;
  }

  void setCG(AVRCodeGen *LLVMIRCG) { CG = LLVMIRCG; } 

  VPOCostGathererBase *getCostGatherer(unsigned int VF, AVRLoop *ALoop, 
                                       VPOVLSInfoBase *VLSInfo) override {
    // FIXME: Find a better way to get DL! We also need to look into avoiding
    // such duplicated code.
    const DataLayout &DL =
      (*cast<AVRLoopIR>(ALoop)->getLoop()->block_begin())
      ->getParent()
      ->getParent()
      ->getDataLayout();

    if (VLSInfo == nullptr) {
      CostGatherer = new VPOCostGatherer(TTI, TLI, DL, VF, ALoop,
                                         nullptr, nullptr);
      return CostGatherer;
    }
    assert(isa<VPOVLSInfo>(*VLSInfo) && "VLSInfo not an LLVMIR VLSInfo");
    VPOVLSInfo *VLSInfoLLVMIR = cast<VPOVLSInfo>(VLSInfo);
    // Pass the underlying LLVMIR Loop instead
    CostGatherer = new VPOCostGatherer(TTI, TLI, DL, VF, ALoop, &VLSCostModel, 
                                       VLSInfoLLVMIR);
    return CostGatherer;
  }

  int getRemainderLoopCost(unsigned int VF, uint64_t &ConstTripCount) override {
    assert(CG && "CG not set.");
    ConstTripCount =  CG->getTripCount();

    // Check for positive trip count and that trip count is a multiple of vector
    // length. Otherwise a remainder loop is needed.
    if (ConstTripCount == 0) {
      // Assume the remainder loop executes VF/2 times in scalar fashion.
      return ScalarIterCost * VF / 2;
    } else {
      return (ConstTripCount % VF) * ScalarIterCost;
    }
  } 

private:
  VPOCostGatherer *CostGatherer;
  AVRCodeGen *CG;
  OVLSTTICostModelLLVMIR VLSCostModel;
};

/// HIR CostModel
class VPOCostModelHIR : public VPOCostModelBase {
public:
  VPOCostModelHIR(AVRWrn *AWrn, const TargetTransformInfo &TTI, 
                  const TargetLibraryInfo &TLI, LLVMContext &LLVMCntxt) 
      : VPOCostModelBase(AWrn, TTI, TLI), CG(nullptr),
        VLSCostModel(TTI, LLVMCntxt) {
    CostGatherer = nullptr;
  }

  void setCG(AVRCodeGenHIR *HIRCG) { CG = HIRCG; } 

  VPOCostGathererBase *getCostGatherer(unsigned int VF, AVRLoop *ALoop,
                                       VPOVLSInfoBase *VLSInfo) override {
    // FIXME: Find a better way to get DL! We also need to look into avoiding
    // such duplicated code.
    const DataLayout &DL =
      (*cast<AVRLoopHIR>(ALoop)->getLoop()->getLLVMLoop()->block_begin())
      ->getParent()
      ->getParent()
      ->getDataLayout();

    if (VLSInfo == nullptr) {
      CostGatherer = new VPOCostGathererHIR(TTI, TLI, DL, VF, ALoop, nullptr,
                                            nullptr);
      return CostGatherer;
    }
    assert(isa<VPOVLSInfoHIR>(*VLSInfo) && "VLSInfo not a VLSInfoHIR");
    VPOVLSInfoHIR *VLSInfoHIR = cast<VPOVLSInfoHIR>(VLSInfo);
    // Pass the underlying HLLoop instead
    CostGatherer = new VPOCostGathererHIR(TTI, TLI, DL, VF, ALoop,
                                          &VLSCostModel, 
                                          VLSInfoHIR);
    return CostGatherer;
  }

  int getRemainderLoopCost(unsigned int VF, uint64_t &ConstTripCount) override {
    assert(CG && "CG not set.");
    ConstTripCount =  CG->getTripCount();

    // Check for positive trip count and that trip count is a multiple of vector
    // length. Otherwise a remainder loop is needed.
    if (ConstTripCount == 0) {
      // Assume the remainder loop executes VF/2 times in scalar fashion.
      return ScalarIterCost * VF / 2;
    } else {
      return (ConstTripCount % VF) * ScalarIterCost;
    }
  } 

private:
  VPOCostGathererHIR *CostGatherer;
  AVRCodeGenHIR *CG;
  OVLSTTICostModelHIR VLSCostModel;
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


// ----------------- VPOScenarioEvaluation ------------------ //
/// \brief Manage exploration of vectorization candidates within a region.
class VPOScenarioEvaluationBase {
public:
  /// Discriminator for LLVM-style RTTI (dyn_cast<> et al.)
  enum ScenarioEvaluationKind {
    SCEK_LLVMIR,
    SCEK_HIR
  };

private:
  const ScenarioEvaluationKind Kind;

protected:
  /// Handle to Target Information
  const TargetTransformInfo &TTI;
  const TargetLibraryInfo &TLI;

  /// AVR Region at hand.
  AVRWrn *AWrn;

  /// \brief A handle to the LLVM Context
  LLVMContext &LLVMCntxt; 

  /// Vectorization Factor (VF) forced by a directive or compiler switch.
  /// Set to 0 if no VF is enforced.
  // CHECKME: per ALoop or per region?
  unsigned int ForceVF;

  /// The minimum/maximum bit widths of types of values loaded/stored in the
  /// loop. We only look at loads/stores for now.
  unsigned int MinBitWidth;
  unsigned int MaxBitWidth;

private:
  /// AVRLoop in AVR region.
  // FIXME: Get rid of this member (given that we also pass the ALoop explicitly
  // to processLoop)?
  AVRLoop *ALoop;

public:
  VPOScenarioEvaluationBase(ScenarioEvaluationKind K, AVRWrn *AWrn, 
                            const TargetTransformInfo &TTI, 
                            const TargetLibraryInfo &TLI, LLVMContext &C)
    : Kind(K), TTI(TTI), TLI(TLI), AWrn(AWrn), LLVMCntxt(C), ForceVF(0),
      MinBitWidth(64), MaxBitWidth(8) {}

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
  /// the selected candidate. The VecContext contains the Aloops and 
  /// Vectorization Factors selected for vectorization.
  /// If DefaultVF is set via compiler switch, or VF is given by a directive, 
  /// getBestCandidate just prepares the requested scenario, but doesn't 
  /// evaluate its cost and doesn't consider other VFs.
  VPOVecContextBase getBestCandidate(AVRWrn *AvrWrn);

  /// Gather loop-level information (memory references, data-dependencs) and
  /// drive the exploration of alternative Vectorization Factors for a given
  /// AVRLoop in a region.
  VPOVecContextBase processLoop(AVRLoop *ALoop, uint64_t *Cost);

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
  uint64_t processCandidate(AVRLoop *ALoop, unsigned int VF,
                            VPOVecContextBase &VC);

  /// \brief Analyze which Vectorization Factors make sense for the loop (in
  /// terms of target support and data-types operated on in the loop).
  void findVFCandidates(VFsVector &VFCandidates) const;

  // Functions to be implemented at the underlying IR level

  virtual void setLoop(AVRLoop *ALoop) = 0;
  virtual bool loopIsHandled(unsigned int ForceVF) = 0;
  virtual void gatherMemrefsInLoop() = 0;
  virtual VPODataDepInfoBase getDataDepInfoForLoop() = 0;
  virtual VPOVLSInfoBase *getVLSInfoForCandidate() = 0;
  virtual VPOVecContextBase &setVecContext(unsigned VF) = 0;
  virtual SIMDLaneEvolutionAnalysisUtilBase &getSLEVUtil() = 0;
  virtual void resetLoopInfo() = 0;
  virtual VPOCostModelBase *getCM() = 0;
  /// Perform IR-specific transformations on \p ALoop to prepare it for
  /// later processing
  virtual void prepareLoop(AVRLoop * ALoop) = 0;

  ScenarioEvaluationKind getKind() const { return Kind; }
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

  /// Provide AVRCodeGen query utilities
  AVRCodeGen *CG;

public:
  VPOScenarioEvaluation(AVRWrn *AvrWrn, const TargetTransformInfo &TTI, 
                        const TargetLibraryInfo &TLI,
                        LLVMContext &C, AvrDefUse &DU)
      : VPOScenarioEvaluationBase(SCEK_LLVMIR, AvrWrn, TTI, TLI, C),
                                  SLEVUtil(DU), CM(AWrn, TTI, TLI, C),
                                  CG(nullptr) {}

  /// Obtain a handle to AVRCodeGen utilities
  void setCG(AVRCodeGen *LLVMIRCG) { CG = LLVMIRCG; } 

  /// Obtain the underlying loop.
  void setLoop(AVRLoop *ALoop) override { return; }

  /// Return true of we can widen this loop (i.e. if AVRCodegen can support
  /// this loop). If \p VF is provided, it will be considered as well as the 
  /// Vectorization Factor of the loop; if \p VF is 0 it will be ignored. 
  /// During vectorization cost evaluation we want to be able to query if the 
  /// loop is supportable before having selected a VF. So normally a \p VF is
  /// provided only when the user requested a specific Vectorization Factor via 
  /// directives or compiler switch. 
  bool loopIsHandled(unsigned int VF) override {
    return CG->loopIsHandled(VF);
  }

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

  VPOCostModelBase *getCM() override { 
    CM.setCG(CG); 
    return &CM; 
  }

  /// Perform LLVM-IR specific transformations on \p ALoop to prepare it for
  /// later processing. No prepare transformations are necessary for LLVM-IR
  /// so far.
  void prepareLoop(AVRLoop * ALoop) override {
  }

  static bool classof(const VPOScenarioEvaluationBase *EvaluationEngine) {
    return EvaluationEngine->getKind() == SCEK_LLVMIR;
  }
};

/// HIR ScenarioEvaluation
class VPOScenarioEvaluationHIR : public VPOScenarioEvaluationBase {
private:
  HIRDDAnalysis *DDA;
  HIRVectVLSAnalysis *VLS;
  SIMDLaneEvolutionAnalysisUtilHIR SLEVUtil;
  AVRDecomposeHIR ValueDecomposerUtil;

  /// Provide cost evaluation utilities for the region.
  VPOCostModelHIR CM;

  /// Obtain a handle to AVRCodeGen utilities
  AVRCodeGenHIR *CG;

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
                           const TargetTransformInfo &TTI, 
                           const TargetLibraryInfo &TLI, LLVMContext &C)
      : VPOScenarioEvaluationBase(SCEK_HIR, AvrWrn, TTI, TLI, C), DDA(DDA),
                                  VLS(VLS), SLEVUtil(DU), CM(AWrn, TTI, TLI, C),
                                  CG(nullptr), Loop(nullptr) {}
  ~VPOScenarioEvaluationHIR() {}

  void setCG(AVRCodeGenHIR *HIRCG) { CG = HIRCG; } 

  /// Return true of we can widen this loop (i.e. if AVRCodegen can support
  /// this loop). If \p VF is provided, it will be considered as well as the 
  /// Vectorization Factor of the loop; if \p VF is 0 it will be ignored. 
  /// During vectorization cost evaluation we want to be able to query if the 
  /// loop is supportable before having selected a VF. So normally a \p VF is
  /// provided only when the user requested a specific Vectorization Factor via 
  /// directives or compiler switch. 
  bool loopIsHandled(unsigned int VF) override {
    return CG->loopIsHandled(VF);
  }

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

  // FIXME: This assumes that VPODDG has been set, which is not the case when 
  // we evaluate the cost of the scalar loop (VF=1) or when a specific VF was
  // requested by the user. Indeed this function is not called in these 
  // situations, but we should assert thet VPODDG is available.
  VPOVecContextHIR &setVecContext(unsigned VF) override {
    VC = VPOVecContextHIR(VF, VPODDG.getDDG(), Loop);
    // TODO: mark that VC is now valid
    return VC;
  }

  SIMDLaneEvolutionAnalysisUtilBase &getSLEVUtil() override { return SLEVUtil; }
  AVRDecomposeHIR &getValueDecomposerUtil() { return ValueDecomposerUtil; }

  void resetLoopInfo() override { LoopMemrefs.clear(); }

  VPOCostModelBase *getCM() override { 
    CM.setCG(CG); 
    return &CM; 
  }

  /// Perform HIR-specific transformations on \p ALoop to prepare it for
  /// later processing.
  void prepareLoop(AVRLoop * ALoop) override {
    AVRDecomposeHIR Decomposer;
    // FIXME: Find a better way to get DL!
    const DataLayout &DL =
        (*cast<AVRLoopHIR>(ALoop)->getLoop()->getLLVMLoop()->block_begin())
            ->getParent()
            ->getParent()
            ->getDataLayout();
    Decomposer.runOnAvr(ALoop, DL);
  }

  static bool classof(const VPOScenarioEvaluationBase *EvaluationEngine) {
    return EvaluationEngine->getKind() == SCEK_HIR;
  }
};

} // End namespace vpo
} // End namespace llvm

#endif // LLVM_ANALYSIS_VPO_VPOSCENARIOEVALUATION_H
