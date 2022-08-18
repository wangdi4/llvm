#if INTEL_COLLAB // -*- C++ -*-
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
//===------------------ WRegion.h - WRegion node ----------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
///  This file defines the classes derived from WRegionNode that
///  correspond to each construct type in WRegionNode::WRegionNodeKind
///
/// Classes below represent REGIONS, one for each type in
/// WRegionNode::WRegionNodeKind
///
/// All of them are derived classes of the base WRegionNode
///
///   Class name:             | Example OpenMP pragma
/// --------------------------|------------------------------------
///   WRNParallelNode         | #pragma omp parallel
///   WRNParallelLoopNode     | #pragma omp parallel for
///   WRNParallelSectionsNode | #pragma omp parallel sections
///   WRNParallelWorkshareNode| !$omp parallel workshare
///   WRNTeamsNode            | #pragma omp teams
///   WRNDistributeParLoopNode| #pragma omp distribute parallel for
///   WRNTargetNode           | #pragma omp target
///   WRNTargetDataNode       | #pragma omp target data
///   WRNTargetEnterDataNode  | #pragma omp target enter data
///   WRNTargetExitDataNode   | #pragma omp target exit data
///   WRNTargetUpdateNode     | #pragma omp target update
///   WRNTargetVariantNode    | #pragma omp target variand dispatch
///   WRNDispatchNode         | #pragma omp dispatch
///   WRNTaskNode             | #pragma omp task
///   WRNTaskloopNode         | #pragma omp taskloop
///   WRNVecLoopNode          | #pragma omp simd
///   WRNWksLoopNode          | #pragma omp for
///   WRNSectionsNode         | #pragma omp sections
///   WRNGenericLoopNode      | #pragma omp loop
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
///   WRNSectionNode          | #pragma omp section
#endif // INTEL_FEATURE_CSA
#endif // INTEL_CUSTOMIZATION
///   WRNWorkshareNode        | !$omp workshare
///   WRNDistributeNode       | #pragma omp distribute
///   WRNAtomicNode           | #pragma omp atomic
///   WRNBarrierNode          | #pragma omp barrier
///   WRNCancelNode           | #pragma omp cancel
///   WRNCriticalNode         | #pragma omp critical
///   WRNFlushNode            | #pragma omp flush
///   WRNPrefetchNode         | #pragma omp prefetch
///   WRNOrderedNode          | #pragma omp ordered
///   WRNMaskedNode           | #pragma omp masked/master
///   WRNSingleNode           | #pragma omp single
///   WRNTaskgroupNode        | #pragma omp taskgroup
///   WRNTaskwaitNode         | #pragma omp taskwait
///   WRNTaskyieldNode        | #pragma omp taskyield
///   WRNInteropNode          | #pragma omp interop
///   WRNScopeNode            | #pragma omp scope
///   WRNGuardMemMotion       | WRN to guard memory motion
///   WRNTileNode             | #pragma omp tile
///   WRNScanNode             | #pragma omp scan
///
/// One exception is WRNTaskloopNode, which is derived from WRNTasknode.
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_WREGION_H
#define LLVM_ANALYSIS_VPO_WREGION_H

#if INTEL_CUSTOMIZATION
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLNode.h"
#endif //INTEL_CUSTOMIZATION
#include "llvm/Analysis/VPO/Utils/VPOAnalysisUtils.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionNode.h"
#include "llvm/IR/Instructions.h"


#include <set>
#include <iterator>

namespace llvm {

class LoopInfo;
class Loop;

namespace vpo {


/// Macro to define getters for list-type clauses. It creates two versions:
///     1. a const version used primarily by dumpers
///     2. a nonconst version used by parsing and other code
// 20171023: Also use this for WRNLoopInfo's getter fns
#define DEFINE_GETTER(CLAUSETYPE, GETTER, CLAUSEOBJ)       \
   const CLAUSETYPE &GETTER() const override { return CLAUSEOBJ; }  \
         CLAUSETYPE &GETTER()       override { return CLAUSEOBJ; }

/// Loop information associated with loop-type constructs
class WRNLoopInfo {
private:
  LoopInfo   *LI = nullptr;
  Loop       *Lp = nullptr;
  SmallVector<Value *, 2> NormIV; // normalized IV's created by FE
  SmallVector<Type *, 2> NormIVElemTy; // normalized IV's ElementTypes
  SmallVector<Value *, 2> NormUB; // normalized UB's
  SmallVector<Type *, 2> NormUBElemTy; // normalized UB's ElementTypes

  /// Basic blocks with the zero-trip test for all loops in a loop nest.
  DenseMap<unsigned, BasicBlock *> ZTTBB;

  /// If set to true, then the associated loop(s) tripcounts
  /// are present in the NDRANGE clause of the enclosing "omp target"
  /// region.
  bool KnownNDRange = false;

  /// If set to true, then the associated loop(s) is optimized away.
  bool LoopOptimizedAway = false;

  /// For each loop in the loop nest described by this WRNLoopInfo
  /// we have to map the loop to some ND-range dimension.
  /// NDRangeStartDim specifies the starting dimension for the loop
  /// nest. For example,
  ///   for (int i = ...)
  ///     for (int j = ...)
  ///
  /// By default the j-loop will be mapped to dimension 0,
  /// and the i-loop will be mapped to dimension 1.
  /// If NDRangeStartDim is equal to 1, then it will be applied
  /// during the mapping for both loops, i.e. the j-loop will map
  /// to dimension 1, and the i-loop will map to dimension 2.
  ///
  /// The final dimension value must never exceed 2, which is
  /// asserted during the mapping.
  uint8_t NDRangeStartDim = 0;

public:
  WRNLoopInfo(LoopInfo *L) : LI(L) {}
  void setLoopInfo(LoopInfo *L) { LI = L; }
  void setLoop(Loop *L) { Lp = L; }
  void addNormIV(Value *IV, Type *IVElemTy) {
    NormIV.push_back(IV);
    NormIVElemTy.push_back(IVElemTy);
  }
  void addNormUB(Value *UB, Type *UBElemTy) {
    NormUB.push_back(UB);
    NormUBElemTy.push_back(UBElemTy);
  }

  /// Set basic block \p BB as a zero-trip test block for the loop nest's
  /// loop with the given index \p Idx.
  void setZTTBB(BasicBlock *BB, unsigned Idx = 0) {
    assert(ZTTBB.find(Idx) == ZTTBB.end() &&
           "Loop already has ZTT block set up.");
    ZTTBB[Idx] = BB;
  }
  LoopInfo *getLoopInfo() const { return LI; }

  /// Return the ith level of loop in a loop nest. This utility is valid
  /// only if all the loops inside the loop nest up to depth I contains only
  /// one child loop. Return nullptr if the nested loop is optimized away.
  Loop *getLoop(unsigned I = 0) const {
    // When I==0, allow Lp to be nullptr (do not assert)
    if (I == 0)
      return Lp;
    Loop *CurLoop = Lp;
    while (I > 0) {
      // We are expecting a nested loop but if the loop is not there then
      // most likely the loop is optimized away.
      if (CurLoop->getSubLoops().empty())
        return nullptr;
      CurLoop = CurLoop->getSubLoops()[0];
      I--;
    }
    return CurLoop;
  }
  Value *getNormIV(unsigned I=0) const;
  Type *getNormIVElemTy(unsigned I = 0) const;
  Value *getNormUB(unsigned I=0) const;
  Type *getNormUBElemTy(unsigned I = 0) const;
  ArrayRef<Value *> getNormUBs() const { return NormUB; }
  ArrayRef<Type *> getNormUBElemTys() const { return NormUBElemTy; }
  unsigned getNormIVSize() const { return NormIV.size(); }
  unsigned getNormUBSize() const { return NormUB.size(); }

  /// Return basic block containing a zero-trip test for the loop nest's
  /// loop with the given index \p Idx.  Assert that the zero-trip test
  /// basic block has been set up for the specified loop.
  BasicBlock *getZTTBB(unsigned Idx = 0) const {
    auto It = ZTTBB.find(Idx);
    // We should never rely on ZTTBB, if is has not been set up,
    // which happens at a certain point of VPO Paropt transformation.
    assert(It != ZTTBB.end() && "ZTT block has not been set up for Loop.");
    return It->second;
  }

  /// Return ZTTBB, if it has been set up, otherwise, return nullptr.
  /// This version may be used for WRNLoopInfo printing, which may happen
  /// before ZTTBB setup.
  BasicBlock *getZTTBBOrNull(unsigned Idx = 0) const {
    return ZTTBB.lookup(Idx);
  }

  void setLoopOptimizedAway() { LoopOptimizedAway = true; }

  bool getLoopOptimizedAway() { return LoopOptimizedAway; }

  void setKnownNDRange() {
    assert(!KnownNDRange && "KnownNDRange must be set only once.");
    KnownNDRange = true;
  }
  void resetKnownNDRange() {
    KnownNDRange = false;
  }
  bool isKnownNDRange() const { return KnownNDRange; }
  void setNDRangeStartDim(uint8_t Dim) {
    assert(Dim < 3 && "Valid dimension must be from [0, 2].");
    assert(NDRangeStartDim == 0 &&
           "NDRangeStartDim must be set only once.");
    NDRangeStartDim = Dim;
  }
  uint8_t getNDRangeStartDim() const {
    return NDRangeStartDim;
  }
  // If the LoopInfo completely contained F before outlining, remove F's
  // blocks from the LoopInfo.
  // Remove any resulting empty Loops.
  void removeBlocksInFn(Function *F) {
    // If the entry block is contained in LI, the entire function is
    // contained.
    // But we must use the original entry before outlining, not the new
    // outlined entry. This entry is a single-entry block following the new
    // entry.
    BasicBlock *OrigEntry = &(F->getEntryBlock());
    if (auto *SingleSucc = OrigEntry->getSingleSuccessor())
      if (SingleSucc->hasNPredecessors(1))
        OrigEntry = SingleSucc;
    if (LI && F && LI->getLoopDepth(OrigEntry)) {
      for (BasicBlock &BB : *F)
        LI->removeBlock(&BB);
      // If F had a loop, it will be empty after block removal. Remove it.
      for (Loop *L : LI->getLoopsInPreorder())
        if (L->getNumBlocks() == 0)
          LI->erase(L);
    }
  }

  void print(formatted_raw_ostream &OS, unsigned Depth,
             unsigned Verbosity=1) const;

  void printNormIVUB(formatted_raw_ostream &OS) const;
};

/// WRN for
/// \code
///   #pragma omp parallel
/// \endcode
class WRNParallelNode : public WRegionNode {
private:
  SharedClause Shared;
  PrivateClause Priv;
  FirstprivateClause Fpriv;
  ReductionClause Reduction;
  AllocateClause Alloc;
  CopyinClause Copyin;
  EXPR IfExpr;
  EXPR NumThreads;
  WRNDefaultKind Default;
  WRNProcBindKind ProcBind;
  SmallVector<Instruction *, 2> CancellationPoints;
  SmallVector<AllocaInst *, 2> CancellationPointAllocas;
  /// Values such as linear step, array section bounds, which will be
  /// used directly inside the outlined function created for the WRegion.
  SmallVector<Value *, 2> DirectlyUsedNonPointerValues;
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
  int NumWorkers;
  int PipelineDepth;
#endif // INTEL_FEATURE_CSA
#endif //INTEL_CUSTOMIZATION

public:
  WRNParallelNode(BasicBlock *BB);

protected:
  void setIf(EXPR E) override { IfExpr = E; }
  void setNumThreads(EXPR E) override { NumThreads = E; }
  void setDefault(WRNDefaultKind D) override { Default = D; }
  void setProcBind(WRNProcBindKind P) override { ProcBind = P; }
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
  void setNumWorkers(int N) { NumWorkers = N; }
  void setPipelineDepth(int P) { PipelineDepth = P; }
#endif // INTEL_FEATURE_CSA
#endif //INTEL_CUSTOMIZATION

public:
  DEFINE_GETTER(SharedClause,       getShared,   Shared)
  DEFINE_GETTER(PrivateClause,      getPriv,     Priv)
  DEFINE_GETTER(FirstprivateClause, getFpriv,    Fpriv)
  DEFINE_GETTER(ReductionClause,    getRed,      Reduction)
  DEFINE_GETTER(AllocateClause,     getAllocate, Alloc)
  DEFINE_GETTER(CopyinClause,       getCopyin,   Copyin)

  EXPR getIf() const override { return IfExpr; }
  EXPR getNumThreads() const override { return NumThreads; }
  WRNDefaultKind getDefault() const override { return Default; }
  WRNProcBindKind getProcBind() const override { return ProcBind; }
  const SmallVectorImpl<Instruction *> &getCancellationPoints() const override {
    return CancellationPoints;
  }
  void addCancellationPoint(Instruction *I) override { CancellationPoints.push_back(I); }
  const SmallVectorImpl<AllocaInst *> &getCancellationPointAllocas() const override {
    return CancellationPointAllocas;
  }
  void addCancellationPointAlloca(AllocaInst *I) override {
    CancellationPointAllocas.push_back(I);
  }
  const SmallVectorImpl<Value *> &getDirectlyUsedNonPointerValues() const override {
    return DirectlyUsedNonPointerValues;
  }
  void addDirectlyUsedNonPointerValue(Value *V) override {
    DirectlyUsedNonPointerValues.push_back(V);
  }
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
  int getNumWorkers() const { return NumWorkers; }
  int getPipelineDepth() const { return PipelineDepth; }
#endif // INTEL_FEATURE_CSA
#endif //INTEL_CUSTOMIZATION

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                             unsigned Verbosity=1) const override;

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNParallel;
  }
};

/// WRN for
/// \code
///   #pragma omp parallel for
/// \endcode
class WRNParallelLoopNode : public WRegionNode {
private:
  SharedClause Shared;
  PrivateClause Priv;
  FirstprivateClause Fpriv;
  LastprivateClause Lpriv;
  ReductionClause Reduction;
  AllocateClause Alloc;
  LinearClause Linear;
  CopyinClause Copyin;
  EXPR IfExpr;
  EXPR NumThreads;
  WRNDefaultKind Default;
  WRNProcBindKind ProcBind;
  ScheduleClause Schedule;
  int Collapse;
  int Ordered;
  WRNLoopOrderKind LoopOrder;
  WRNLoopInfo WRNLI;
  SmallVector<Value *, 2> OrderedTripCounts;
  SmallVector<Instruction *, 2> CancellationPoints;
  SmallVector<AllocaInst *, 2> CancellationPointAllocas;
  /// Values such as linear step, array section bounds, which will be
  /// used directly inside the outlined function created for the WRegion.
  SmallVector<Value *, 2> DirectlyUsedNonPointerValues;
#if INTEL_CUSTOMIZATION
  bool IsDoConcurrent = false; // Used from Fortran DO Concurrent
  loopopt::HLNode *EntryHLNode; // for HIR only
  loopopt::HLNode *ExitHLNode;  // for HIR only
  loopopt::HLLoop *HLp;         // for HIR only
#if INTEL_FEATURE_CSA
  int NumWorkers;
  int PipelineDepth;
  ScheduleClause WorkerSchedule;
#endif // INTEL_FEATURE_CSA
#endif //INTEL_CUSTOMIZATION

public:
  WRNParallelLoopNode(BasicBlock *BB, LoopInfo *L);
#if INTEL_CUSTOMIZATION
  // constructor for HIR representation
  WRNParallelLoopNode(loopopt::HLNode *EntryHLN);
#endif //INTEL_CUSTOMIZATION

protected:
  void setIf(EXPR E) override { IfExpr = E; }
  void setNumThreads(EXPR E) override { NumThreads = E; }
  void setDefault(WRNDefaultKind D) override { Default = D; }
  void setProcBind(WRNProcBindKind P) override { ProcBind = P; }
  void setCollapse(int N) override { Collapse = N; }
  void setOrdered(int N) override { Ordered = N; }
  void setLoopOrder(WRNLoopOrderKind LO) override { LoopOrder = LO; }

#if INTEL_CUSTOMIZATION
  void setIsDoConcurrent(bool B) override { IsDoConcurrent = B; }
  void setEntryHLNode(loopopt::HLNode *E) override { EntryHLNode = E; }
  void setExitHLNode(loopopt::HLNode *X) override { ExitHLNode = X; }
  void setHLLoop(loopopt::HLLoop *L) override { HLp = L; }
#if INTEL_FEATURE_CSA
  void setNumWorkers(int N) { NumWorkers = N; }
  void setPipelineDepth(int P) { PipelineDepth = P; }
#endif // INTEL_FEATURE_CSA
#endif //INTEL_CUSTOMIZATION

public:
  DEFINE_GETTER(SharedClause,       getShared,   Shared)
  DEFINE_GETTER(PrivateClause,      getPriv,     Priv)
  DEFINE_GETTER(FirstprivateClause, getFpriv,    Fpriv)
  DEFINE_GETTER(LastprivateClause,  getLpriv,    Lpriv)
  DEFINE_GETTER(ReductionClause,    getRed,      Reduction)
  DEFINE_GETTER(AllocateClause,     getAllocate, Alloc)
  DEFINE_GETTER(LinearClause,       getLinear,   Linear)
  DEFINE_GETTER(CopyinClause,       getCopyin,   Copyin)
  DEFINE_GETTER(ScheduleClause,     getSchedule, Schedule)
  DEFINE_GETTER(WRNLoopInfo,        getWRNLoopInfo, WRNLI)
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
  DEFINE_GETTER(ScheduleClause, getWorkerSchedule, WorkerSchedule)
#endif // INTEL_FEATURE_CSA
#endif //INTEL_CUSTOMIZATION

  EXPR getIf() const override { return IfExpr; }
  EXPR getNumThreads() const override { return NumThreads; }
  WRNDefaultKind getDefault() const override { return Default; }
  WRNProcBindKind getProcBind() const override { return ProcBind; }
  int getCollapse() const override { return Collapse; }
  int getOrdered() const override { return Ordered; }
  int getOmpLoopDepth() const override {
    // Depth of associated loop. Ordered >= Collapse if both exist
    assert((Ordered <= 0 || Ordered >= Collapse) &&
           "Ordered must be >= Collapse when both are specified.");
    return Ordered > 0 ? Ordered : (Collapse > 0 ? Collapse : 1);
  }
  WRNLoopOrderKind getLoopOrder() const override { return LoopOrder; }
  void addOrderedTripCount(Value *TC) override { OrderedTripCounts.push_back(TC); }
  const SmallVectorImpl<Value *> &getOrderedTripCounts() const override {
    return OrderedTripCounts;
  }
  const SmallVectorImpl<Instruction *> &getCancellationPoints() const override {
    return CancellationPoints;
  }
  void addCancellationPoint(Instruction *I) override { CancellationPoints.push_back(I); }
  const SmallVectorImpl<AllocaInst *> &getCancellationPointAllocas() const override {
    return CancellationPointAllocas;
  }
  void addCancellationPointAlloca(AllocaInst *I) override {
    CancellationPointAllocas.push_back(I);
  }
  const SmallVectorImpl<Value *> &getDirectlyUsedNonPointerValues() const override {
    return DirectlyUsedNonPointerValues;
  }
  void addDirectlyUsedNonPointerValue(Value *V) override {
    DirectlyUsedNonPointerValues.push_back(V);
  }

#if INTEL_CUSTOMIZATION
  bool getIsDoConcurrent() const override { return IsDoConcurrent; }
  loopopt::HLNode *getEntryHLNode() const override { return EntryHLNode; }
  loopopt::HLNode *getExitHLNode() const override { return ExitHLNode; }
  loopopt::HLLoop *getHLLoop() const override  { return HLp; }
  void printHIR(formatted_raw_ostream &OS, unsigned Depth,
                                           unsigned Verbosity=1) const override;
#if INTEL_FEATURE_CSA
  int getNumWorkers() const { return NumWorkers; }
  int getPipelineDepth() const { return PipelineDepth; }
#endif // INTEL_FEATURE_CSA
#endif //INTEL_CUSTOMIZATION

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                             unsigned Verbosity=1) const override;

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNParallelLoop;
  }
};

/// WRN for
/// \code
///   #pragma omp parallel sections
/// \endcode
class WRNParallelSectionsNode : public WRegionNode {
private:
  SharedClause Shared;
  PrivateClause Priv;
  FirstprivateClause Fpriv;
  LastprivateClause Lpriv;
  ReductionClause Reduction;
  AllocateClause Alloc;
  CopyinClause Copyin;
  EXPR IfExpr;
  EXPR NumThreads;
  WRNDefaultKind Default;
  WRNProcBindKind ProcBind;
  WRNLoopInfo WRNLI;
  SmallVector<Instruction *, 2> CancellationPoints;
  SmallVector<AllocaInst *, 2> CancellationPointAllocas;
  /// Values such as linear step, array section bounds, which will be
  /// used directly inside the outlined function created for the WRegion.
  SmallVector<Value *, 2> DirectlyUsedNonPointerValues;

public:
  WRNParallelSectionsNode(BasicBlock *BB, LoopInfo *L);

protected:
  void setIf(EXPR E) override { IfExpr = E; }
  void setNumThreads(EXPR E) override { NumThreads = E; }
  void setDefault(WRNDefaultKind D) override { Default = D; }
  void setProcBind(WRNProcBindKind P) override { ProcBind = P; }

public:
  DEFINE_GETTER(SharedClause,       getShared,   Shared)
  DEFINE_GETTER(PrivateClause,      getPriv,     Priv)
  DEFINE_GETTER(FirstprivateClause, getFpriv,    Fpriv)
  DEFINE_GETTER(LastprivateClause,  getLpriv,    Lpriv)
  DEFINE_GETTER(ReductionClause,    getRed,      Reduction)
  DEFINE_GETTER(AllocateClause,     getAllocate, Alloc)
  DEFINE_GETTER(CopyinClause,       getCopyin,   Copyin)
  DEFINE_GETTER(WRNLoopInfo,        getWRNLoopInfo, WRNLI)

  EXPR getIf() const override { return IfExpr; }
  EXPR getNumThreads() const override { return NumThreads; }
  WRNDefaultKind getDefault() const override  { return Default; }
  WRNProcBindKind getProcBind() const override { return ProcBind; }
  const SmallVectorImpl<Instruction *> &getCancellationPoints() const override {
    return CancellationPoints;
  }
  void addCancellationPoint(Instruction *I) override { CancellationPoints.push_back(I); }
  const SmallVectorImpl<AllocaInst *> &getCancellationPointAllocas() const override {
    return CancellationPointAllocas;
  }
  void addCancellationPointAlloca(AllocaInst *I) override {
    CancellationPointAllocas.push_back(I);
  }
  const SmallVectorImpl<Value *> &getDirectlyUsedNonPointerValues() const override {
    return DirectlyUsedNonPointerValues;
  }
  void addDirectlyUsedNonPointerValue(Value *V) override {
    DirectlyUsedNonPointerValues.push_back(V);
  }

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                             unsigned Verbosity=1) const override;

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNParallelSections;
  }
};

/// WRN for
/// \code
///   !$omp parallel workshare
/// \endcode
class WRNParallelWorkshareNode : public WRegionNode {
private:
  SharedClause Shared;
  PrivateClause Priv;
  FirstprivateClause Fpriv;
  ReductionClause Reduction;
  AllocateClause Alloc;
  CopyinClause Copyin;
  ScheduleClause Schedule;
  EXPR IfExpr;
  EXPR NumThreads;
  WRNDefaultKind Default;
  WRNProcBindKind ProcBind;
  WRNLoopInfo WRNLI;

public:
  WRNParallelWorkshareNode(BasicBlock *BB, LoopInfo *L);

protected:
  void setIf(EXPR E) override { IfExpr = E; }
  void setNumThreads(EXPR E) override { NumThreads = E; }
  void setDefault(WRNDefaultKind D) override { Default = D; }
  void setProcBind(WRNProcBindKind P) override { ProcBind = P; }

public:
  DEFINE_GETTER(SharedClause,       getShared,   Shared)
  DEFINE_GETTER(PrivateClause,      getPriv,     Priv)
  DEFINE_GETTER(FirstprivateClause, getFpriv,    Fpriv)
  DEFINE_GETTER(ReductionClause,    getRed,      Reduction)
  DEFINE_GETTER(AllocateClause,     getAllocate, Alloc)
  DEFINE_GETTER(CopyinClause,       getCopyin,   Copyin)
  DEFINE_GETTER(ScheduleClause,     getSchedule, Schedule)
  DEFINE_GETTER(WRNLoopInfo,        getWRNLoopInfo, WRNLI)

  EXPR getIf() const override { return IfExpr; }
  EXPR getNumThreads() const override { return NumThreads; }
  WRNDefaultKind getDefault() const override { return Default; }
  WRNProcBindKind getProcBind() const override { return ProcBind; }

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                             unsigned Verbosity=1) const override;

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNParallelWorkshare;
  }
};

/// WRN for
/// \code
///   #pragma omp teams
/// \endcode
class WRNTeamsNode : public WRegionNode {
private:
  SharedClause Shared;
  PrivateClause Priv;
  FirstprivateClause Fpriv;
  ReductionClause Reduction;
  AllocateClause Alloc;
  EXPR ThreadLimit;
  Type *ThreadLimitTy = nullptr;
  EXPR NumTeams;
  Type *NumTeamsTy = nullptr;
  WRNDefaultKind Default;
#if INTEL_CUSTOMIZATION
  bool IsDoConcurrent = false; // Used for Fortran Do Concurrent
#endif // INTEL_CUSTOMIZATION

public:
  WRNTeamsNode(BasicBlock *BB);

protected:
  void setThreadLimit(EXPR E) override { ThreadLimit = E; }
  void setThreadLimitType(Type *T) override { ThreadLimitTy = T; }
  void setNumTeams(EXPR E) override { NumTeams = E; }
  void setNumTeamsType(Type *T) override { NumTeamsTy = T; }
  void setDefault(WRNDefaultKind D) override { Default = D; }
#if INTEL_CUSTOMIZATION
  void setIsDoConcurrent(bool B) override { IsDoConcurrent = B; }
#endif // INTEL_CUSTOMIZATION

public:
  DEFINE_GETTER(SharedClause,       getShared,   Shared)
  DEFINE_GETTER(PrivateClause,      getPriv,     Priv)
  DEFINE_GETTER(FirstprivateClause, getFpriv,    Fpriv)
  DEFINE_GETTER(ReductionClause,    getRed,      Reduction)
  DEFINE_GETTER(AllocateClause,     getAllocate, Alloc)

  EXPR getThreadLimit() const override { return ThreadLimit; }
  Type *getThreadLimitType() const override { return ThreadLimitTy; }
  EXPR getNumTeams() const override  { return NumTeams; }
  Type *getNumTeamsType() const override { return NumTeamsTy; }
  WRNDefaultKind getDefault() const override { return Default; }
#if INTEL_CUSTOMIZATION
  bool getIsDoConcurrent() const override { return IsDoConcurrent; }
#endif // INTEL_CUSTOMIZATION

  void printExtra(formatted_raw_ostream &OS,
                  unsigned Depth,
                  unsigned Verbosity=1) const override;

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNTeams;
  }
};

/// WRN for
/// \code
///   #pragma omp distribute parallel for
/// \endcode
class WRNDistributeParLoopNode : public WRegionNode {
private:
  SharedClause Shared;
  PrivateClause Priv;
  FirstprivateClause Fpriv;
  LastprivateClause Lpriv;
  ReductionClause Reduction;
  AllocateClause Alloc;
  LinearClause Linear;
  CopyinClause Copyin;
  EXPR IfExpr;
  EXPR NumThreads;
  WRNDefaultKind Default;
  WRNProcBindKind ProcBind;
  ScheduleClause Schedule;
  ScheduleClause DistSchedule;
  int Collapse;
  int Ordered;
  WRNLoopOrderKind LoopOrder;
  WRNLoopInfo WRNLI;
  bool TreatDistributeParLoopAsDistribute; // Used during transformation.
  /// Values such as dist_schedule chunk size, which will be
  /// used directly inside the outlined function created for the WRegion.
  SmallVector<Value *, 2> DirectlyUsedNonPointerValues;

#if INTEL_CUSTOMIZATION
  bool IsDoConcurrent = false; // Used for Fortran DO Concurrent.
#endif // INTEL_CUSTOMIZATION

public:
  WRNDistributeParLoopNode(BasicBlock *BB, LoopInfo *L);

protected:
  void setIf(EXPR E) override { IfExpr = E; }
  void setNumThreads(EXPR E) override { NumThreads = E; }
  void setDefault(WRNDefaultKind D) override { Default = D; }
  void setProcBind(WRNProcBindKind P) override { ProcBind = P; }
  void setCollapse(int N) override { Collapse = N; }
  void setOrdered(int N) override { Ordered = N; }
  void setLoopOrder(WRNLoopOrderKind LO) override { LoopOrder = LO; }
#if INTEL_CUSTOMIZATION
  void setIsDoConcurrent(bool B) override { IsDoConcurrent = B; }
#endif // INTEL_CUSTOMIZATION

public:
  DEFINE_GETTER(SharedClause,       getShared,       Shared)
  DEFINE_GETTER(PrivateClause,      getPriv,         Priv)
  DEFINE_GETTER(FirstprivateClause, getFpriv,        Fpriv)
  DEFINE_GETTER(LastprivateClause,  getLpriv,        Lpriv)
  DEFINE_GETTER(ReductionClause,    getRed,          Reduction)
  DEFINE_GETTER(AllocateClause,     getAllocate,     Alloc)
  DEFINE_GETTER(LinearClause,       getLinear,       Linear)
  DEFINE_GETTER(CopyinClause,       getCopyin,       Copyin)
  DEFINE_GETTER(ScheduleClause,     getSchedule,     Schedule)
  DEFINE_GETTER(ScheduleClause,     getDistSchedule, DistSchedule)
  DEFINE_GETTER(WRNLoopInfo,        getWRNLoopInfo,  WRNLI)

  EXPR getIf() const override { return IfExpr; }
  EXPR getNumThreads() const override { return NumThreads; }
  WRNDefaultKind getDefault() const override { return Default; }
  WRNProcBindKind getProcBind() const override { return ProcBind; }
  int getCollapse() const override { return Collapse; }
  int getOrdered() const override { return Ordered; }
  int getOmpLoopDepth() const override {
    // Depth of associated loop. Ordered >= Collapse if both exist
    assert((Ordered <= 0 || Ordered >= Collapse) &&
           "Ordered must be >= Collapse when both are specified.");
    return Ordered > 0 ? Ordered : (Collapse > 0 ? Collapse : 1);
  }
  WRNLoopOrderKind getLoopOrder() const override { return LoopOrder; }

  void setTreatDistributeParLoopAsDistribute(bool Flag) override {
    TreatDistributeParLoopAsDistribute = Flag;
  }
  bool getTreatDistributeParLoopAsDistribute() const override {
    return TreatDistributeParLoopAsDistribute;
  }

#if INTEL_CUSTOMIZATION
  bool getIsDoConcurrent() const override { return IsDoConcurrent; }
#endif // INTEL_CUSTOMIZATION

  const SmallVectorImpl<Value *> &getDirectlyUsedNonPointerValues() const override {
    return DirectlyUsedNonPointerValues;
  }
  void addDirectlyUsedNonPointerValue(Value *V) override {
    DirectlyUsedNonPointerValues.push_back(V);
  }

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                             unsigned Verbosity=1) const override ;

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNDistributeParLoop;
  }
};

/// WRN for
/// \code
///   #pragma omp target
/// \endcode
class WRNTargetNode : public WRegionNode {
private:
  PrivateClause Priv;
  FirstprivateClause Fpriv;
  MapClause Map;
  AllocateClause Alloc;
  DependClause Depend;            // from "QUAL.OMP.DEPEND"
  EXPR DepArray = nullptr;        // Arr in "QUAL.OMP.DEPARRAY"(i32 N, i8* Arr)
  EXPR DepArrayNumDeps = nullptr; // N above; ie, number of depend-items in Arr
  IsDevicePtrClause IsDevicePtr;
  EXPR IfExpr;
  EXPR Device;
  SubdeviceClause Subdevice;
  LiveinClause Livein;
  AllocaInst *ParLoopNdInfoAlloca;    // supports kernel loop parallelization
  bool Nowait = false;
  WRNDefaultmapBehavior Defaultmap[WRNDefaultmapCategorySize] =
      {WRNDefaultmapAbsent};
  int OffloadEntryIdx;
  SmallVector<Value *, 2> DirectlyUsedNonPointerValues;
  SmallVector<Value *, 3> UncollapsedNDRangeDimensions;
  SmallVector<Type *, 3> UncollapsedNDRangeTypes;
  uint8_t NDRangeDistributeDim = 0;
  unsigned SPIRVSIMDWidth = 0;
  bool HasTeamsReduction = false;
  uint8_t HasAtomicFreeReduction = 0;
#if INTEL_CUSTOMIZATION
  bool IsDoConcurrent = false;  // Used fro Fortran Do Concurrent
#endif // INTEL_CUSTOMIZATION

public:
  WRNTargetNode(BasicBlock *BB);

protected:
  void setDepArray(EXPR E) override { DepArray = E; }
  void setDepArrayNumDeps(EXPR E) override { DepArrayNumDeps = E; }
  void setIf(EXPR E) override { IfExpr = E; }
  void setDevice(EXPR E) override { Device = E; }
  void setNowait(bool Flag) override { Nowait = Flag; }
#if INTEL_CUSTOMIZATION
  void setIsDoConcurrent(bool B) override { IsDoConcurrent = B; }
#endif // INTEL_CUSTOMIZATION
  void setDefaultmap(WRNDefaultmapCategory C, WRNDefaultmapBehavior B) override {
    Defaultmap[C] = B;
  }
  void setOffloadEntryIdx(int Idx) override { OffloadEntryIdx = Idx; }
  void setUncollapsedNDRangeDimensions(ArrayRef<Value *> Dims) override {
    assert(UncollapsedNDRangeDimensions.empty() &&
           "Uncollapsed NDRange must be set only once.");
    UncollapsedNDRangeDimensions.insert(
        UncollapsedNDRangeDimensions.begin(), Dims.begin(), Dims.end());
  }
  void setUncollapsedNDRangeTypes(ArrayRef<Type *> Types) override {
    assert(UncollapsedNDRangeTypes.empty() &&
           "Uncollapsed NDRange must be set only once.");
    UncollapsedNDRangeTypes.insert(
        UncollapsedNDRangeTypes.begin(), Types.begin(), Types.end());
  }
  void setHasTeamsReduction() override {
    HasTeamsReduction = true;
  }
 public:
  DEFINE_GETTER(PrivateClause,      getPriv,        Priv)
  DEFINE_GETTER(FirstprivateClause, getFpriv,       Fpriv)
  DEFINE_GETTER(MapClause,          getMap,         Map)
  DEFINE_GETTER(AllocateClause,     getAllocate,    Alloc)
  DEFINE_GETTER(DependClause,       getDepend,      Depend)
  DEFINE_GETTER(IsDevicePtrClause,  getIsDevicePtr, IsDevicePtr)
  DEFINE_GETTER(SubdeviceClause,    getSubdevice,   Subdevice)
  DEFINE_GETTER(LiveinClause,       getLivein,      Livein)

  // ParLoopNdInfoAlloca is set by transformation rather than parsing, so
  // setter is public instead of protected
  void setParLoopNdInfoAlloca(AllocaInst *AI) override { ParLoopNdInfoAlloca = AI; }
  AllocaInst *getParLoopNdInfoAlloca() const override { return ParLoopNdInfoAlloca; }
  EXPR getDepArray() const override { return DepArray; }
  EXPR getDepArrayNumDeps() const override { return DepArrayNumDeps; }
  EXPR getIf() const override { return IfExpr; }
  EXPR getDevice() const override { return Device; }
  bool getNowait() const override { return Nowait; }
#if INTEL_CUSTOMIZATION
  bool getIsDoConcurrent() const override { return IsDoConcurrent; }
#endif // INTEL_CUSTOMIZATION

  WRNDefaultmapBehavior getDefaultmap(WRNDefaultmapCategory C) const override {
    return Defaultmap[C];
  }
  int getOffloadEntryIdx() const override { return OffloadEntryIdx; }
  const SmallVectorImpl<Value *> &getDirectlyUsedNonPointerValues() const override {
    return DirectlyUsedNonPointerValues;
  }
  void addDirectlyUsedNonPointerValue(Value *V) override {
    DirectlyUsedNonPointerValues.push_back(V);
  }

  const SmallVectorImpl<Value *> &
      getUncollapsedNDRangeDimensions() const override {
    return UncollapsedNDRangeDimensions;
  }
  const SmallVectorImpl<Type *> &getUncollapsedNDRangeTypes() const override {
    return UncollapsedNDRangeTypes;
  }

  void setSPIRVSIMDWidth(unsigned Width) {
    SPIRVSIMDWidth = Width;
  }

  unsigned getSPIRVSIMDWidth() const {
    return SPIRVSIMDWidth;
  }

  void resetUncollapsedNDRange() override {
    UncollapsedNDRangeDimensions.clear();
    UncollapsedNDRangeTypes.clear();
  }

  void setNDRangeDistributeDim(uint8_t Dim) override {
    assert(Dim != 0 && "Dim is 0 by default.");
    assert(NDRangeDistributeDim == 0 &&
           "NDRangeDistributeDim must be set only once.");
    NDRangeDistributeDim = Dim;
  }

  uint8_t getNDRangeDistributeDim() const override {
    return NDRangeDistributeDim;
  }

  bool getHasTeamsReduction() const override {
    return HasTeamsReduction;
  }

  void setHasLocalAtomicFreeReduction() {
    HasAtomicFreeReduction |= VPOParoptAtomicFreeReduction::Kind_Local;
  }

  void setHasGlobalAtomicFreeReduction() {
    HasAtomicFreeReduction |= VPOParoptAtomicFreeReduction::Kind_Global;
  }

  bool getHasLocalAtomicFreeReduction() const {
    return HasAtomicFreeReduction & VPOParoptAtomicFreeReduction::Kind_Local;
  }

  bool getHasGlobalAtomicFreeReduction() const {
    return HasAtomicFreeReduction & VPOParoptAtomicFreeReduction::Kind_Global;
  }

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                             unsigned Verbosity=1) const override ;

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNTarget;
  }
};

/// This WRN is similar to WRNTargetNode but it does not offload a code block
/// to the device. It holds map clauses that describe data movement between
/// host and device as specified by the OMP directive:
/// \code
///   #pragma omp target data
/// \endcode
class WRNTargetDataNode : public WRegionNode {
private:
  MapClause Map;
  UseDevicePtrClause UseDevicePtr;
  EXPR IfExpr;
  EXPR Device;
  SubdeviceClause Subdevice;

public:
  WRNTargetDataNode(BasicBlock *BB);

protected:
  void setIf(EXPR E) override { IfExpr = E; }
  void setDevice(EXPR E) override { Device = E; }

public:
  DEFINE_GETTER(MapClause,          getMap,          Map)
  DEFINE_GETTER(UseDevicePtrClause, getUseDevicePtr, UseDevicePtr)
  DEFINE_GETTER(SubdeviceClause,    getSubdevice,   Subdevice)

  EXPR getIf() const override { return IfExpr; }
  EXPR getDevice() const override { return Device; }

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                             unsigned Verbosity=1) const override;

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNTargetData;
  }
};

/// WRN for
/// \code
///   #pragma omp target enter data
/// \endcode
class WRNTargetEnterDataNode : public WRegionNode {
private:
  MapClause Map;
  DependClause Depend;            // from "QUAL.OMP.DEPEND"
  EXPR DepArray = nullptr;        // Arr in "QUAL.OMP.DEPARRAY"(i32 N, i8* Arr)
  EXPR DepArrayNumDeps = nullptr; // N above; ie, number of depend-items in Arr
  EXPR IfExpr;
  EXPR Device;
  SubdeviceClause Subdevice;
  bool Nowait = false;

public:
  WRNTargetEnterDataNode(BasicBlock *BB);

protected:
  void setDepArray(EXPR E) override { DepArray = E; }
  void setDepArrayNumDeps(EXPR E) override { DepArrayNumDeps = E; }
  void setIf(EXPR E) override { IfExpr = E; }
  void setDevice(EXPR E) override { Device = E; }
  void setNowait(bool Flag) override { Nowait = Flag; }

public:
  DEFINE_GETTER(MapClause,          getMap,          Map)
  DEFINE_GETTER(DependClause,       getDepend,       Depend)
  DEFINE_GETTER(SubdeviceClause,    getSubdevice,   Subdevice)

  EXPR getDepArray() const override { return DepArray; }
  EXPR getDepArrayNumDeps() const override { return DepArrayNumDeps; }
  EXPR getIf() const override { return IfExpr; }
  EXPR getDevice() const override { return Device; }
  bool getNowait() const override { return Nowait; }

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                             unsigned Verbosity=1) const override ;

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNTargetEnterData;
  }
};

/// WRN for
/// \code
///   #pragma omp target exit data
/// \endcode
class WRNTargetExitDataNode : public WRegionNode {
private:
  MapClause Map;
  DependClause Depend;            // from "QUAL.OMP.DEPEND"
  EXPR DepArray = nullptr;        // Arr in "QUAL.OMP.DEPARRAY"(i32 N, i8* Arr)
  EXPR DepArrayNumDeps = nullptr; // N above; ie, number of depend-items in Arr
  EXPR IfExpr;
  EXPR Device;
  SubdeviceClause Subdevice;
  bool Nowait = false;

public:
  WRNTargetExitDataNode(BasicBlock *BB);

protected:
  void setDepArray(EXPR E) override { DepArray = E; }
  void setDepArrayNumDeps(EXPR E) override { DepArrayNumDeps = E; }
  void setIf(EXPR E) override { IfExpr = E; }
  void setDevice(EXPR E) override { Device = E; }
  void setNowait(bool Flag) override { Nowait = Flag; }

public:
  DEFINE_GETTER(MapClause,          getMap,          Map)
  DEFINE_GETTER(DependClause,       getDepend,       Depend)
  DEFINE_GETTER(SubdeviceClause,    getSubdevice,   Subdevice)

  EXPR getDepArray() const override { return DepArray; }
  EXPR getDepArrayNumDeps() const override { return DepArrayNumDeps; }
  EXPR getIf() const override { return IfExpr; }
  EXPR getDevice() const override { return Device; }
  bool getNowait() const override { return Nowait; }

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                             unsigned Verbosity=1) const override;

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNTargetExitData;
  }
};

/// WRN for
/// \code
///   #pragma omp target update
/// \endcode
class WRNTargetUpdateNode : public WRegionNode {
private:
  MapClause Map;        // used for the to/from clauses
  DependClause Depend;            // from "QUAL.OMP.DEPEND"
  EXPR DepArray = nullptr;        // Arr in "QUAL.OMP.DEPARRAY"(i32 N, i8* Arr)
  EXPR DepArrayNumDeps = nullptr; // N above; ie, number of depend-items in Arr
  EXPR IfExpr;
  EXPR Device;
  SubdeviceClause Subdevice;
  bool Nowait = false;

public:
  WRNTargetUpdateNode(BasicBlock *BB);

protected:
  void setDepArray(EXPR E) override { DepArray = E; }
  void setDepArrayNumDeps(EXPR E) override { DepArrayNumDeps = E; }
  void setIf(EXPR E) override { IfExpr = E; }
  void setDevice(EXPR E) override { Device = E; }
  void setNowait(bool Flag) override { Nowait = Flag; }

public:
  DEFINE_GETTER(MapClause,          getMap,          Map)
  DEFINE_GETTER(DependClause,       getDepend,       Depend)
  DEFINE_GETTER(SubdeviceClause,    getSubdevice,   Subdevice)

  EXPR getDepArray() const override { return DepArray; }
  EXPR getDepArrayNumDeps() const override { return DepArrayNumDeps; }
  EXPR getIf() const override { return IfExpr; }
  EXPR getDevice() const override { return Device; }
  bool getNowait() const override { return Nowait; }

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                             unsigned Verbosity=1) const override;

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNTargetUpdate;
  }
};

/// WRN for
/// \code
///   #pragma omp target variant dispatch
/// \endcode
class WRNTargetVariantNode : public WRegionNode {
private:
  MapClause Map;
  UseDevicePtrClause UseDevicePtr;
  EXPR Device;
  SubdeviceClause Subdevice;
  bool Nowait = false;

public:
  WRNTargetVariantNode(BasicBlock *BB);

protected:
  void setDevice(EXPR E) override { Device = E; }
  void setNowait(bool Flag) override { Nowait = Flag; }

public:
  DEFINE_GETTER(MapClause,          getMap,          Map)
  DEFINE_GETTER(UseDevicePtrClause, getUseDevicePtr, UseDevicePtr)
  DEFINE_GETTER(SubdeviceClause,    getSubdevice,   Subdevice)
  EXPR getDevice() const override { return Device; }
  bool getNowait() const override { return Nowait; }

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                             unsigned Verbosity=1) const override ;

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNTargetVariant;
  }
};

/// WRN for
/// \code
///   #pragma omp dispatch
/// \endcode
class WRNDispatchNode : public WRegionNode {
private:
  // No need to represent depend clause here; it's moved to the implicit task
  IsDevicePtrClause IsDevicePtr;
  SubdeviceClause Subdevice;
  EXPR Device;
  EXPR Nocontext;
  EXPR Novariants;
  bool Nowait = false;

  // To be populated during Paropt codegen
  CallInst *Call;                  // The dispatch call.
  MapClause Map;                   // Map and UseDevicePtr clauses let us reuse
  UseDevicePtrClause UseDevicePtr; // the target data logic to use device ptrs.

public:
  WRNDispatchNode(BasicBlock *BB);

protected:
  void setDevice(EXPR E) override { Device = E; }
  void setNocontext(EXPR E) override { Nocontext = E; }
  void setNovariants(EXPR E) override { Novariants = E; }
  void setNowait(bool Flag) override { Nowait = Flag; }
  void setCall(CallInst *CI) override { Call = CI; }

public:
  DEFINE_GETTER(IsDevicePtrClause,  getIsDevicePtr,  IsDevicePtr)
  DEFINE_GETTER(SubdeviceClause,    getSubdevice,    Subdevice)
  DEFINE_GETTER(MapClause,          getMap,          Map)
  DEFINE_GETTER(UseDevicePtrClause, getUseDevicePtr, UseDevicePtr)
  EXPR getDevice() const override { return Device; }
  EXPR getNocontext() const override { return Nocontext; }
  EXPR getNovariants() const override { return Novariants; }
  bool getNowait() const override { return Nowait; }
  CallInst *getCall() const override { return Call; }

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                  unsigned Verbosity = 1) const override;

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNDispatch;
  }
};

/// WRN for
/// \code
///   #pragma omp interop
/// \endcode
class WRNInteropNode : public WRegionNode {
private:
  EXPR Device;
  DependClause Depend;            // from "QUAL.OMP.DEPEND"
  EXPR DepArray = nullptr;        // Arr in "QUAL.OMP.DEPARRAY"(i32 N, i8* Arr)
  EXPR DepArrayNumDeps = nullptr; // N above; ie, number of depend-items in Arr
  InteropActionClause InteropAction;
  bool Nowait = false;

public:
  WRNInteropNode(BasicBlock *BB);

protected:
  void setDevice(EXPR E) override { Device = E; }
  void setDepArray(EXPR E) override { DepArray = E; }
  void setDepArrayNumDeps(EXPR E) override { DepArrayNumDeps = E; }
  void setNowait(bool Flag) override { Nowait = Flag; }

public:
  DEFINE_GETTER(DependClause, getDepend, Depend)
  DEFINE_GETTER(InteropActionClause, getInteropAction, InteropAction)

  EXPR getDevice() const override { return Device; }
  EXPR getDepArray() const override { return DepArray; }
  EXPR getDepArrayNumDeps() const override { return DepArrayNumDeps; }
  bool getNowait() const override { return Nowait; }

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                  unsigned Verbosity = 1) const override;

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNInterop;
  }
};

/// \brief Task flags used to invoke tasking RTL for both Task and Taskloop
enum WRNTaskFlag : uint32_t {
  Tied         = 0x00000001,
  Final        = 0x00000002,
  MergedIf0    = 0x00000004,
  DtorThunk    = 0x00000008,
  Proxy        = 0x00000010,
  PriorityUsed = 0x00000020,
  Detachable   = 0x00000040,
  HiddenHelper = 0x00000080
  // bits  9-16: reserved for compiler
  // bits 17-20: library flags
  // bits 21-25: task state flags
  // bits 26-32: reserved for library
};

/// WRN for
/// \code
///   #pragma omp task
/// \endcode
class WRNTaskNode : public WRegionNode {
private:
  SharedClause Shared;
  PrivateClause Priv;
  FirstprivateClause Fpriv;
  ReductionClause InReduction;
  AllocateClause Alloc;
  DependClause Depend;            // from "QUAL.OMP.DEPEND"
  EXPR DepArray = nullptr;        // Arr in "QUAL.OMP.DEPARRAY"(i32 N, i8* Arr)
  EXPR DepArrayNumDeps = nullptr; // N above; ie, number of depend-items in Arr
  EXPR Final;
  EXPR IfExpr;
  EXPR Priority;
  WRNDefaultKind Default;
  bool Untied;
  bool Mergeable;
  bool IsTargetTask; // Task is the implicit task surrounding a target region.
  unsigned TaskFlag; // flag bit vector used to invoke tasking RTL
  SmallVector<Instruction *, 2> CancellationPoints;
  SmallVector<AllocaInst *, 2> CancellationPointAllocas;
  bool IsTargetNowaitTask = false; // set to true when parsing nowait on
                                   // taskwait as task.

public:
  WRNTaskNode(BasicBlock *BB);

protected:
  void setDepArray(EXPR E) override { DepArray = E; }
  void setDepArrayNumDeps(EXPR E) override { DepArrayNumDeps = E; }
  void setFinal(EXPR E) override { Final = E; }
  void setIf(EXPR E) override { IfExpr = E; }
  void setPriority(EXPR E) override { Priority = E; }
  void setDefault(WRNDefaultKind D) override { Default = D; }
  void setUntied(bool B) override { Untied = B; }
  void setMergeable(bool B) override { Mergeable = B; }
  void setIsTargetTask(bool B) override { IsTargetTask = B; }
  void setTaskFlag(unsigned F) override { TaskFlag = F; }
  void setIsTaskwaitNowaitTask(bool B) override { IsTargetNowaitTask = B; }

public:
  DEFINE_GETTER(SharedClause,       getShared,   Shared)
  DEFINE_GETTER(PrivateClause,      getPriv,     Priv)
  DEFINE_GETTER(FirstprivateClause, getFpriv,    Fpriv)
  DEFINE_GETTER(ReductionClause,    getInRed,    InReduction)
  DEFINE_GETTER(AllocateClause,     getAllocate, Alloc)
  DEFINE_GETTER(DependClause,       getDepend,   Depend)

  EXPR getDepArray() const override { return DepArray; }
  EXPR getDepArrayNumDeps() const override { return DepArrayNumDeps; }
  EXPR getFinal() const override { return Final; }
  EXPR getIf() const override { return IfExpr; }
  EXPR getPriority() const override { return Priority; }
  WRNDefaultKind getDefault() const override { return Default; }
  bool getUntied() const override { return Untied; }
  bool getMergeable() const override { return Mergeable; }
  bool getIsTargetTask() const override { return IsTargetTask; }
  unsigned getTaskFlag() const override { return TaskFlag; }
  bool getIsTaskwaitNowaitTask() const override { return IsTargetNowaitTask; }
  const SmallVectorImpl<Instruction *> &getCancellationPoints() const override {
    return CancellationPoints;
  }
  void addCancellationPoint(Instruction *I) override { CancellationPoints.push_back(I); }
  const SmallVectorImpl<AllocaInst *> &getCancellationPointAllocas() const  override {
    return CancellationPointAllocas;
  }
  void addCancellationPointAlloca(AllocaInst *I) override {
    CancellationPointAllocas.push_back(I);
  }

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                             unsigned Verbosity=1) const override ;

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNTask;
  }
};

/// WRN for
/// \code
///   #pragma omp taskloop
/// \endcode
/// A taskloop can have both reduction and in_reduction clauses. Therefore,
/// WRNTaskloopNode has two members of the class ReductionClause:
/// 'InReduction' is inherited from the parent WRNTaskNode, and
/// 'Reduction' is declared for taskloop but not task.
class WRNTaskloopNode : public WRNTaskNode {
private:
  LastprivateClause Lpriv;
  ReductionClause Reduction;
  EXPR Grainsize;
  EXPR NumTasks;
  int SchedCode; // 1 for Grainsize, 2 for num_tasks, 0 for none.
  int Collapse;
  bool Nogroup;
  WRNLoopInfo WRNLI;

  // These taskloop clauses are also clauses in the parent class WRNTaskNode
  //   SharedClause Shared;
  //   PrivateClause Priv;
  //   FirstprivateClause Fpriv;
  //   AllocateClause Alloc;
  //   DependClause Depend;
  //   EXPR DepArray;
  //   EXPR DepArrayNumDeps;
  //   EXPR Final;
  //   EXPR IfExpr;
  //   EXPR Priority;
  //   WRNDefaultKind Default;
  //   bool Untied;
  //   bool Mergeable;
  //   unsigned TaskFlag;

public:
  WRNTaskloopNode(BasicBlock *BB, LoopInfo *L);

protected:
  void setGrainsize(EXPR E) override { Grainsize = E; }
  void setNumTasks(EXPR E) override { NumTasks = E; }
  void setSchedCode(int N) override { SchedCode = N; }
  void setCollapse(int N) override { Collapse = N; }
  void setNogroup(bool B) override { Nogroup = B; }

  // Defined in parent class WRNTaskNode
  //   void setDepArray(EXPR E) override { DepArray = E; }
  //   void setDepArrayNumDeps(EXPR E) override { DepArrayNumDeps = E; }
  //   void setFinal(EXPR E) { Final = E; }
  //   void setif(expr e) { ifexpr = e; }
  //   void setPriority(EXPR E) { Priority = E; }
  //   void setDefault(WRNDefaultKind D) { Default = D; }
  //   void setUntied(bool B) { Untied = B; }
  //   void setMergeable(bool B) { Mergeable = B; }
  //   void setTaskFlag(unsigned F) { TaskFlag = F; }

public:
  DEFINE_GETTER(LastprivateClause,  getLpriv,  Lpriv)
  DEFINE_GETTER(ReductionClause,    getRed,    Reduction)
  DEFINE_GETTER(WRNLoopInfo,        getWRNLoopInfo, WRNLI)
  EXPR getGrainsize() const override { return Grainsize; }
  EXPR getNumTasks() const override { return NumTasks; }
  int getSchedCode() const override { return SchedCode; }
  int getCollapse() const override { return Collapse; }
  int getOmpLoopDepth() const override { return Collapse > 0 ? Collapse : 1; }
  bool getNogroup() const override { return Nogroup; }

  // Defined in parent class WRNTaskNode
  //   DEFINE_GETTER(SharedClause,       getShared,   Shared)
  //   DEFINE_GETTER(PrivateClause,      getPriv,     Priv)
  //   DEFINE_GETTER(FirstprivateClause, getFpriv,    Fpriv)
  //   DEFINE_GETTER(AllocateClause,     getAllocate, Alloc)
  //   DEFINE_GETTER(DependClause,       getDepend,   Depend)
  //   EXPR getDepArray() const override { return DepArray; }
  //   EXPR getDepArrayNumDeps() const override { return DepArrayNumDeps; }
  //   EXPR getFinal() const { return Final; }
  //   EXPR getIf() const { return IfExpr; }
  //   EXPR getPriority() const { return Priority; }
  //   WRNDefaultKind getDefault() const { return Default; }
  //   bool getUntied() const { return Untied; }
  //   bool getMergeable() const { return Mergeable; }
  //   unsigned getTaskFlag() const { return TaskFlag; }

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNTaskloop;
  }
};

/// WRN for
/// \code
///   #pragma omp simd
/// \endcode
class WRNVecLoopNode : public WRegionNode {
private:
  PrivateClause Priv;
  LastprivateClause Lpriv;
  LiveinClause Livein;
  ReductionClause Reduction;
  LinearClause Linear;
  AlignedClause Aligned;
  NontemporalClause Nontemporal;
  UniformClause Uniform; // The simd construct does not take a uniform clause,
                         // so we won't get this from the front-end, but this
                         // list can/will be populated by the vector backend
  EXPR IfExpr;
  int Simdlen;
  int Safelen;
  int Collapse;

  WRNLoopOrderKind LoopOrder;
  WRNLoopInfo WRNLI;
#if INTEL_CUSTOMIZATION
  bool IsDoConcurrent = false; // Used for Fortran Do Concurrent
  bool IsAutoVec;
  bool HasVectorAlways;
  loopopt::HLNode *EntryHLNode; // for HIR only
  loopopt::HLNode *ExitHLNode;  // for HIR only
  loopopt::HLLoop *HLp;         // for HIR only
#endif //INTEL_CUSTOMIZATION

public:
#if INTEL_CUSTOMIZATION
  WRNVecLoopNode(BasicBlock *BB, LoopInfo *L,
                 const bool isAutoVec); // LLVM IR representation
  WRNVecLoopNode(loopopt::HLNode *EntryHLN,
                 const bool isAutoVec); // HIR representation
#else
  WRNVecLoopNode(BasicBlock *BB, LoopInfo *L);
#endif //INTEL_CUSTOMIZATION

  void setIf(EXPR E) override { IfExpr = E; }
  void setSimdlen(int N) override { Simdlen = N; }
  void setSafelen(int N) override { Safelen = N; }
  void setCollapse(int N)  override { Collapse = N; }
  void setLoopOrder(WRNLoopOrderKind LO)  override { LoopOrder = LO; }

#if INTEL_CUSTOMIZATION
  void setIsDoConcurrent(bool B) override { IsDoConcurrent = B; }
  void setIsAutoVec(bool Flag)  override{ IsAutoVec = Flag; }
  void setHasVectorAlways(bool Flag)  override{ HasVectorAlways = Flag; }
  void setEntryHLNode(loopopt::HLNode *E)  override{ EntryHLNode = E; }
  void setExitHLNode(loopopt::HLNode *X)  override{ ExitHLNode = X; }
  void setHLLoop(loopopt::HLLoop *L)  override { HLp = L; }
#endif //INTEL_CUSTOMIZATION

  DEFINE_GETTER(PrivateClause,     getPriv,        Priv)
  DEFINE_GETTER(LastprivateClause, getLpriv,       Lpriv)
  DEFINE_GETTER(LiveinClause,      getLivein,      Livein)
  DEFINE_GETTER(ReductionClause,   getRed,         Reduction)
  DEFINE_GETTER(LinearClause,      getLinear,      Linear)
  DEFINE_GETTER(AlignedClause,     getAligned,     Aligned)
  DEFINE_GETTER(NontemporalClause, getNontemporal, Nontemporal)
  DEFINE_GETTER(UniformClause,     getUniform,     Uniform)
  DEFINE_GETTER(WRNLoopInfo,       getWRNLoopInfo, WRNLI)

  EXPR getIf() const override { return IfExpr; }
  int getSimdlen() const override{ return Simdlen; }
  int getSafelen() const override{ return Safelen; }
  int getCollapse() const override{ return Collapse; }
  int getOmpLoopDepth() const override { return Collapse > 0 ? Collapse : 1; }
  WRNLoopOrderKind getLoopOrder() const override{ return LoopOrder; }

#if INTEL_CUSTOMIZATION
  bool getIsDoConcurrent() const override { return IsDoConcurrent; }
  bool getIsAutoVec() const override{ return IsAutoVec; }
  bool getHasVectorAlways() const override{ return HasVectorAlways; }
  loopopt::HLNode *getEntryHLNode() const override{ return EntryHLNode; }
  loopopt::HLNode *getExitHLNode() const override{ return ExitHLNode; }
  loopopt::HLLoop *getHLLoop() const override{ return HLp; }
  bool isOmpSIMDLoop() const { return !getIsAutoVec(); }
  bool isValidHIRSIMDRegion() const;
#endif //INTEL_CUSTOMIZATION

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                             unsigned Verbosity=1) const override;

#if INTEL_CUSTOMIZATION
  void printHIR(formatted_raw_ostream &OS, unsigned Depth,
                                           unsigned Verbosity=1) const override;

  template <class LoopType> LoopType *getTheLoop() const {
    llvm_unreachable("Unsupported LoopType");
  }
#endif //INTEL_CUSTOMIZATION

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNVecLoop;
  }
};

#if INTEL_CUSTOMIZATION
template <> Loop *WRNVecLoopNode::getTheLoop<Loop>() const;
template <>
loopopt::HLLoop *WRNVecLoopNode::getTheLoop<loopopt::HLLoop>() const;
#endif //INTEL_CUSTOMIZATION

/// WRN for
/// \code
///   #pragma omp for
/// \endcode
class WRNWksLoopNode : public WRegionNode {
private:
  PrivateClause Priv;
  FirstprivateClause Fpriv;
  LastprivateClause Lpriv;
  LiveinClause Livein;
  ReductionClause Reduction;
  AllocateClause Alloc;
  LinearClause Linear;
  ScheduleClause Schedule;
  int Collapse;
  int Ordered;
  WRNLoopOrderKind LoopOrder;
  bool Nowait = false;

  WRNLoopInfo WRNLI;
  SmallVector<Value *, 2> OrderedTripCounts;
  SmallVector<Instruction *, 2> CancellationPoints;
  SmallVector<AllocaInst *, 2> CancellationPointAllocas;
#if INTEL_CUSTOMIZATION
  bool IsDoConcurrent = false; // Used for Fortran Do Concurrent
#if INTEL_FEATURE_CSA
  ScheduleClause WorkerSchedule;
#endif // INTEL_FEATURE_CSA
#endif //INTEL_CUSTOMIZATION

public:
  WRNWksLoopNode(BasicBlock *BB, LoopInfo *L);

protected:
  void setCollapse(int N) override { Collapse = N; }
  void setOrdered(int N) override { Ordered = N; }
  void setLoopOrder(WRNLoopOrderKind LO) override{ LoopOrder = LO; }
  void setNowait(bool Flag) override { Nowait = Flag; }
#if INTEL_CUSTOMIZATION
  void setIsDoConcurrent(bool B) override { IsDoConcurrent = B; }
#endif // INTEL_CUSTOMIZATION

public:
  DEFINE_GETTER(PrivateClause,      getPriv,        Priv)
  DEFINE_GETTER(FirstprivateClause, getFpriv,       Fpriv)
  DEFINE_GETTER(LastprivateClause,  getLpriv,       Lpriv)
  DEFINE_GETTER(LiveinClause,       getLivein,      Livein)
  DEFINE_GETTER(ReductionClause,    getRed,         Reduction)
  DEFINE_GETTER(AllocateClause,     getAllocate,    Alloc)
  DEFINE_GETTER(LinearClause,       getLinear,      Linear)
  DEFINE_GETTER(ScheduleClause,     getSchedule,    Schedule)
  DEFINE_GETTER(WRNLoopInfo,        getWRNLoopInfo, WRNLI)
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
  DEFINE_GETTER(ScheduleClause, getWorkerSchedule, WorkerSchedule)
#endif // INTEL_FEATURE_CSA
#endif //INTEL_CUSTOMIZATION

  int getCollapse() const override{ return Collapse; }
  int getOrdered() const override { return Ordered; }
  int getOmpLoopDepth() const override {
    assert((Ordered <= 0 || Ordered >= Collapse) &&
           "Ordered must be >= Collapse when both are specified.");
    // Depth of associated loop. Ordered >= Collapse if both exist
    return Ordered > 0 ? Ordered : (Collapse > 0 ? Collapse : 1);
  }
  WRNLoopOrderKind getLoopOrder() const override{ return LoopOrder; }
  bool getNowait() const override { return Nowait; }
#if INTEL_CUSTOMIZATION
  bool getIsDoConcurrent() const override { return IsDoConcurrent; }
#endif // INTEL_CUSTOMIZATION

  void addOrderedTripCount(Value *TC) override { OrderedTripCounts.push_back(TC); }
  const SmallVectorImpl<Value *> &getOrderedTripCounts() const override {
    return OrderedTripCounts;
  }
  const SmallVectorImpl<Instruction *> &getCancellationPoints() const override{
    return CancellationPoints;
  }
  void addCancellationPoint(Instruction *I) override{ CancellationPoints.push_back(I); }
  const SmallVectorImpl<AllocaInst *> &getCancellationPointAllocas() const override {
    return CancellationPointAllocas;
  }
  void addCancellationPointAlloca(AllocaInst *I) override {
    CancellationPointAllocas.push_back(I);
  }

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                             unsigned Verbosity=1) const override;

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNWksLoop;
  }
};

/// WRN for
/// \code
///   #pragma omp sections
/// \endcode
class WRNSectionsNode : public WRegionNode {
private:
  PrivateClause Priv;
  FirstprivateClause Fpriv;
  LastprivateClause Lpriv;
  ReductionClause Reduction;
  AllocateClause Alloc;
  ScheduleClause Schedule;
  bool Nowait = false;
  WRNLoopInfo WRNLI;
  SmallVector<Instruction *, 2> CancellationPoints;
  SmallVector<AllocaInst *, 2> CancellationPointAllocas;

public:
  WRNSectionsNode(BasicBlock *BB, LoopInfo *L);

protected:
  void setNowait(bool Flag) override { Nowait = Flag; }

public:
  DEFINE_GETTER(PrivateClause,      getPriv,   Priv)
  DEFINE_GETTER(FirstprivateClause, getFpriv,  Fpriv)
  DEFINE_GETTER(LastprivateClause,  getLpriv,  Lpriv)
  DEFINE_GETTER(ReductionClause,    getRed,    Reduction)
  DEFINE_GETTER(AllocateClause,     getAllocate, Alloc)
  DEFINE_GETTER(ScheduleClause,     getSchedule, Schedule)
  DEFINE_GETTER(WRNLoopInfo,        getWRNLoopInfo, WRNLI)

  bool getNowait() const override { return Nowait; }
  const SmallVectorImpl<Instruction *> &getCancellationPoints() const override {
    return CancellationPoints;
  }
  void addCancellationPoint(Instruction *I) override { CancellationPoints.push_back(I); }
  const SmallVectorImpl<AllocaInst *> &getCancellationPointAllocas() const override {
    return CancellationPointAllocas;
  }
  void addCancellationPointAlloca(AllocaInst *I) override {
    CancellationPointAllocas.push_back(I);
  }

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                             unsigned Verbosity=1) const override;

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNSections;
  }
};

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
/// WRN for
/// \code
///   #pragma omp section
/// \endcode
class WRNSectionNode : public WRegionNode {
public:
  WRNSectionNode(BasicBlock *BB);

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNSection;
  }
};
#endif // INTEL_FEATURE_CSA
#endif // INTEL_CUSTOMIZATION

/// Fortran-only WRN for
/// \code
///   !$omp workshare
/// \endcode
class WRNWorkshareNode : public WRegionNode {
private:
  bool Nowait = false;
  WRNLoopInfo WRNLI;

public:
  WRNWorkshareNode(BasicBlock *BB, LoopInfo *L);

protected:
  void setNowait(bool Flag) override  { Nowait = Flag; }

public:
  DEFINE_GETTER(WRNLoopInfo, getWRNLoopInfo, WRNLI)
  bool getNowait() const override  { return Nowait; }

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                             unsigned Verbosity=1) const override ;

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNWorkshare;
  }
};

/// WRN for
/// \code
///   #pragma omp distribute
/// \endcode
class WRNDistributeNode : public WRegionNode {
private:
  PrivateClause Priv;
  FirstprivateClause Fpriv;
  LastprivateClause Lpriv;
  AllocateClause Alloc;
  ScheduleClause DistSchedule;
  int Collapse;
  WRNLoopInfo WRNLI;

public:
  WRNDistributeNode(BasicBlock *BB, LoopInfo *L);

protected:
  void setCollapse(int N) override  { Collapse = N; }

public:
  DEFINE_GETTER(PrivateClause,      getPriv,         Priv)
  DEFINE_GETTER(FirstprivateClause, getFpriv,        Fpriv)
  DEFINE_GETTER(LastprivateClause,  getLpriv,        Lpriv)
  DEFINE_GETTER(AllocateClause,     getAllocate,     Alloc)
  DEFINE_GETTER(ScheduleClause,     getDistSchedule, DistSchedule)
  DEFINE_GETTER(WRNLoopInfo,        getWRNLoopInfo,  WRNLI)

  int getCollapse() const override  { return Collapse; }
  int getOmpLoopDepth() const override { return Collapse > 0 ? Collapse : 1; }

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                             unsigned Verbosity=1) const override ;

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNDistribute;
  }
};

/// WRegion Node for OMP Atomic Directive.
/// \code
///   #pragma omp atomic [seq_cst[,]] atomic-clause [[,]seq_cst]
/// \endcode
/// Where:
///   'atomic-clause' can be read, write, update or capture.
///   'seq_cst' clause is optional.
class WRNAtomicNode : public WRegionNode {
private:
  WRNAtomicKind AtomicKind;
  bool HasSeqCstClause;

protected:
  void setAtomicKind(WRNAtomicKind AK) override  { AtomicKind = AK; }
  void setHasSeqCstClause(bool SC) override { HasSeqCstClause = SC; }

public:
  WRNAtomicNode(BasicBlock *BB);
  WRNAtomicNode(WRNAtomicNode *W);

  WRNAtomicKind getAtomicKind() const override { return AtomicKind; }
  bool getHasSeqCstClause() const override { return HasSeqCstClause; }

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                             unsigned Verbosity=1) const override ;

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNAtomic;
  }
};

/// WRegion Node for OMP flush Directive.
/// \code
///   #pragma omp flush [(item-list)]
/// \endcode
class WRNFlushNode : public WRegionNode {
private:
  FlushSet FValueSet;  // qualOpndList

public:
  WRNFlushNode(BasicBlock *BB);

  DEFINE_GETTER(FlushSet, getFlush, FValueSet)

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNFlush;
  }
};

/// WRN for
/// \code
///   #pragma omp prefetch
/// \endcode
class WRNPrefetchNode : public WRegionNode {
private:
  EXPR IfExpr;
  DataClause Data;

public:
  WRNPrefetchNode(BasicBlock *BB);

protected:
  void setIf(EXPR E) override { IfExpr = E; }

public:
  DEFINE_GETTER(DataClause, getData, Data)

  EXPR getIf() const override { return IfExpr; }

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                  unsigned Verbosity = 1) const override;

  // Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNPrefetch;
  }
};

/// WRN for
/// \code
///   #pragma omp scope
/// \endcode
class WRNScopeNode : public WRegionNode {
private:
  PrivateClause Priv;
  ReductionClause Reduction;
  bool Nowait = false;

public:
  WRNScopeNode(BasicBlock *BB);

protected:
  void setNowait(bool Flag) override { Nowait = Flag; }

public:
  DEFINE_GETTER(PrivateClause,      getPriv,     Priv)
  DEFINE_GETTER(ReductionClause,    getRed,      Reduction)
  bool getNowait() const override { return Nowait; }
  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                  unsigned Verbosity=1) const override;

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNScope;
  }
};

/// WRN for regions that guard memory motion of OMP clause variables.
class WRNGuardMemMotionNode : public WRegionNode {
private:
  LiveinClause Livein;

public:
  WRNGuardMemMotionNode(BasicBlock *BB);
  DEFINE_GETTER(LiveinClause, getLivein, Livein)

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNGuardMemMotion;
  }
};

/// WRN for
/// \code
///   #pragma omp tile
/// \endcode
class WRNTileNode : public WRegionNode {
private:
  // TODO: To avoid confusion, when possible use the auxiliary Livein clause
  // instead of Firstprivate, which is not allowed for this construct per
  // OpenMP spec.
  FirstprivateClause Fpriv;  // Not allowed in spec
  LiveinClause Livein;
  SizesClause Sizes;
  WRNLoopInfo WRNLI;

public:
  WRNTileNode(BasicBlock *BB, LoopInfo *L);
  DEFINE_GETTER(FirstprivateClause, getFpriv,       Fpriv)
  DEFINE_GETTER(LiveinClause,       getLivein,      Livein)
  DEFINE_GETTER(SizesClause,        getSizes,       Sizes)
  DEFINE_GETTER(WRNLoopInfo,        getWRNLoopInfo, WRNLI)

  int getOmpLoopDepth() const override { return Sizes.size(); }

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNTile;
  }
};

/// WRN for
/// \code
///   #pragma omp scan
/// \endcode
class WRNScanNode : public WRegionNode {
private:
  InclusiveClause Incl;
  ExclusiveClause Excl;

public:
  WRNScanNode(BasicBlock *BB);
  DEFINE_GETTER(InclusiveClause,    getInclusive,   Incl)
  DEFINE_GETTER(ExclusiveClause,    getExclusive,   Excl)

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                  unsigned Verbosity = 1) const override;

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNScan;
  }
};

/// WRN for
/// \code
///   #pragma omp barrier
/// \endcode
class WRNBarrierNode : public WRegionNode {
public:
  WRNBarrierNode(BasicBlock *BB);

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNBarrier;
  }
};

/// WRN for either of these directives:
/// \code
///   #pragma omp cancel             <construct-type> [if(..)]
///   #pragma omp cancellation point <construct-type>
/// \endcode
class WRNCancelNode : public WRegionNode {
private:
  bool IsCancellationPoint;
  WRNCancelKind CancelKind;
  EXPR IfExpr;   // valid only when IsCancellationPoint==false

public:
  WRNCancelNode(BasicBlock *BB, bool IsCP);

protected:
  void setIsCancellationPoint(bool IsCP) { IsCancellationPoint = IsCP; }
  void setCancelKind(WRNCancelKind CK) override  { CancelKind = CK; }
  void setIf(EXPR E) override { IfExpr = E; }

public:
  bool getIsCancellationPoint() const { return IsCancellationPoint; }
  WRNCancelKind getCancelKind() const override  { return CancelKind; }
  EXPR getIf() const override   { return IfExpr; }

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                             unsigned Verbosity=1) const override ;

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNCancel;
  }
};

/// WRN for
/// \code
///   #pragma omp masked
/// \endcode
class WRNMaskedNode : public WRegionNode {
private:
  EXPR Filter;

public:
  WRNMaskedNode(BasicBlock *BB);

protected:
  void setFilter(EXPR E) override { Filter = E; }

public:
  EXPR getFilter() const override { return Filter; }

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                  unsigned Verbosity = 1) const override;

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNMasked;
  }
};

/// WRN for these constructs:
/// \code
///   #pragma omp ordered [threads | simd]
///   #pragma omp ordered depend(source | sink(..))
/// \endcode
/// The first form is the traditional OpenMP ordered contruct, while the
/// second form is new with OpenMP4.x, to describe DOACROSS
class WRNOrderedNode : public WRegionNode {
private:
  bool IsDoacross;         // true iff a depend clause is seen (2nd form above)
  // IsSIMD and IsThreads are meaningful only if IsDoacross==false
  bool IsSIMD;
  bool IsThreads;

  // The following two fields are meaningful only if IsDoacross==true
  DepSinkClause DepSink;
  DepSourceClause DepSource;
  void assertDoacrossTrue() const  { assert (IsDoacross &&
                              "This WRNOrdered represents Doacross"); }
  void assertDoacrossFalse() const { assert (!IsDoacross &&
                              "This WRNOrdered does not represent Doacross"); }

public:
  WRNOrderedNode(BasicBlock *BB);

protected:
  void setIsDoacross(bool Flag) override { IsDoacross = Flag; }
  void setIsSIMD(bool Flag) override { assertDoacrossFalse(); IsSIMD = Flag; }
  void setIsThreads(bool Flag) override  { assertDoacrossFalse(); IsThreads = Flag; }

public:
  bool getIsDoacross() const override {  return IsDoacross; }
  bool getIsSIMD() const override { return !IsDoacross && IsSIMD; }
  bool getIsThreads() const override { return !IsDoacross && (IsThreads || !IsSIMD); }

  const DepSinkClause &getDepSink() const override {assertDoacrossTrue();
                                           return DepSink; }
  DepSinkClause &getDepSink() override { assertDoacrossTrue(); return DepSink; }

  const DepSourceClause &getDepSource() const override  {assertDoacrossTrue();
                                           return DepSource; }
  DepSourceClause &getDepSource() override { assertDoacrossTrue(); return DepSource; }

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                             unsigned Verbosity=1) const override ;

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNOrdered;
  }
};

/// WRN for
/// \code
///   #pragma omp single
/// \endcode
class WRNSingleNode : public WRegionNode {

private:
  PrivateClause Priv;
  FirstprivateClause Fpriv;
  AllocateClause Alloc;
  CopyprivateClause Cpriv;
  bool Nowait = false;

public:
  WRNSingleNode(BasicBlock *BB);

protected:
  void setNowait(bool Flag) override { Nowait = Flag; }

public:
  DEFINE_GETTER(PrivateClause,      getPriv,     Priv)
  DEFINE_GETTER(FirstprivateClause, getFpriv,    Fpriv)
  DEFINE_GETTER(AllocateClause,     getAllocate, Alloc)
  DEFINE_GETTER(CopyprivateClause,  getCpriv,    Cpriv)

  bool getNowait() const override { return Nowait; }

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                             unsigned Verbosity=1) const override ;

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNSingle;
  }
};

/// WRegion Node for OMP Critical Directive.
/// \code
///    #pragma omp critical [(name)]
/// \endcode
/// Where `name` is an optional parameter provided by the user as an identifier
/// for the critical section. Internally, it is used as suffix in the name of
/// the lock variable used for the critical section.
class WRNCriticalNode : public WRegionNode {
private:
  SmallString<64> UserLockName; ///< Lock name provided by the user.
  uint32_t  Hint;

protected:
  void setUserLockName(StringRef LN) override { UserLockName = LN; }
  void setHint(uint32_t N) override { Hint = N; }

public:
  WRNCriticalNode(BasicBlock *BB);

  StringRef getUserLockName() const override { return UserLockName.str(); }
  uint32_t getHint() const override { return Hint; }

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                             unsigned Verbosity=1) const override ;

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNCritical;
  }
};

/// WRN for
/// \code
///   #pragma omp taskgroup
/// \endcode
class WRNTaskgroupNode : public WRegionNode {
private:
  ReductionClause Reduction;  // for the task_reduction clause
  AllocateClause Alloc;

public:
  WRNTaskgroupNode(BasicBlock *BB);
  DEFINE_GETTER(ReductionClause,    getRed,      Reduction)
  DEFINE_GETTER(AllocateClause,     getAllocate, Alloc)

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNTaskgroup;
  }
};

/// WRN for
/// \code
///   #pragma omp taskwait
/// \endcode
class WRNTaskwaitNode : public WRegionNode {

private:
  DependClause Depend;            // from "QUAL.OMP.DEPEND"
  EXPR DepArray = nullptr;        // Arr in "QUAL.OMP.DEPARRAY"(i32 N, i8* Arr)
  EXPR DepArrayNumDeps = nullptr; // N above; ie, number of depend-items in Arr

protected:
  void setDepArray(EXPR E) override { DepArray = E; }
  void setDepArrayNumDeps(EXPR E) override { DepArrayNumDeps = E; }

public:
  WRNTaskwaitNode(BasicBlock *BB);
  DEFINE_GETTER(DependClause, getDepend, Depend)
  EXPR getDepArray() const override { return DepArray; }
  EXPR getDepArrayNumDeps() const override { return DepArrayNumDeps; }

/// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNTaskwait;
  }
};

/// WRN for
/// \code
///   #pragma omp taskyield
/// \endcode
class WRNTaskyieldNode : public WRegionNode {

public:
  WRNTaskyieldNode(BasicBlock *BB);

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNTaskyield;
  }
};

/// WRN for
/// \code
///   #pragma omp loop
/// \endcode
class WRNGenericLoopNode : public WRegionNode {
private:
  // Shared and Firstprivate clauses are not allowed for loop construct as in
  // OpenMP spec 5.0, but they're needed for outlining logic in backend. Backend
  // maps the loop construct to the underlying loop-related directive, checks
  // the clauses are needed by the underlying directive, and decides to keep or
  // drop the clauses.
  //
  // TODO: To avoid confusion, when possible use the auxiliary Livein clause
  // instead of Shared, which is not allowed for this construct per OMP spec.
  SharedClause Shared;      // Not allowed in spec
  PrivateClause Priv;
  FirstprivateClause Fpriv; // Not allowed in spec
  LastprivateClause Lpriv;
  LiveinClause Livein;
  ReductionClause Reduction;
  int Collapse;

  WRNLoopBindKind LoopBind;
  WRNLoopOrderKind LoopOrder;
  WRNLoopInfo WRNLI;
  int MappedDir;
#if INTEL_CUSTOMIZATION
  bool IsDoConcurrent = false; // used for Fortran Do Concurrent
#endif // INTEL_CUSTOMIZATION

public:
  WRNGenericLoopNode(BasicBlock *BB, LoopInfo *L);

protected:
  void setLoopBind(WRNLoopBindKind LB) override { LoopBind = LB; }
  void setLoopOrder(WRNLoopOrderKind LO) override { LoopOrder = LO; }
  void setCollapse(int N) override { Collapse = N; }
#if INTEL_CUSTOMIZATION
  void setIsDoConcurrent(bool B) override { IsDoConcurrent = B; }
#endif // INTEL_CUSTOMIZATION

public:
  DEFINE_GETTER(SharedClause,       getShared,      Shared)
  DEFINE_GETTER(PrivateClause,      getPriv,        Priv)
  DEFINE_GETTER(FirstprivateClause, getFpriv,       Fpriv)
  DEFINE_GETTER(LastprivateClause,  getLpriv,       Lpriv)
  DEFINE_GETTER(LiveinClause,       getLivein,      Livein)
  DEFINE_GETTER(ReductionClause,    getRed,         Reduction)
  DEFINE_GETTER(WRNLoopInfo,        getWRNLoopInfo, WRNLI)

  int getCollapse() const override { return Collapse; }
  int getOmpLoopDepth() const override { return Collapse > 0 ? Collapse : 1; }
#if INTEL_CUSTOMIZATION
  bool getIsDoConcurrent() const override { return IsDoConcurrent; }
#endif // INTEL_CUSTOMIZATION
  WRNLoopBindKind getLoopBind() const override { return LoopBind; }
  WRNLoopOrderKind getLoopOrder() const override { return LoopOrder; }

  bool mapLoopScheme();

  int getMappedDir() const { return MappedDir; }

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                  unsigned Verbosity = 1) const override ;

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNGenericLoop;
  }
};

/// \brief Print the fields common to WRNs for which getIsPar()==true.
/// Possible constructs are: WRNParallel, WRNParallelLoop,
///                          WRNParallelSections, WRNParallelWorkshare,
/// The fields to print are: IfExpr, NumThreads, Default, ProcBind
extern void printExtraForParallel(WRegionNode const *W,
                                  formatted_raw_ostream &OS, int Depth,
                                  unsigned Verbosity=1);

/// \brief Print the fields common to some WRNs for which getIsOmpLoop()==true.
/// Possible constructs are: WRNParallelLoop, WRNDistributeParLoop, WRNWksLoop
/// The fields to print are: Collapse, Ordered, Order, Nowait
extern void printExtraForOmpLoop(WRegionNode const *W,
                                  formatted_raw_ostream &OS, int Depth,
                                  unsigned Verbosity=1);

/// \brief Print the fields common to WRNs for which getIsTarget()==true.
/// Possible constructs are: WRNTarget, WRNTargetData, WRNTargetUpdate
/// The fields to print are: IfExpr, Device, Nowait
/// Additionally, for WRNTarget also print the Defaultmap clause
extern void printExtraForTarget(WRegionNode const *W,
                                formatted_raw_ostream &OS, int Depth,
                                unsigned Verbosity=1);

/// \brief Print the fields common to WRNs for which getIsTask()==true.
/// Possible constructs are: WRNTask, WRNTaskloop
/// The fields to print are:
///          IfExpr, Default, Final, Priority, Untied, Mergeable
/// Additionally, for WRNTaskloop also print these:
///          Grainsize, NumTasks, Collapse, Nogroup
extern void printExtraForTask(WRegionNode const *W, formatted_raw_ostream &OS,
                              int Depth, unsigned Verbosity=1);

/// \brief Print the fields common to WRNs for which
/// canHaveCancellationPoints()==true. Possible constructs are: WRNParallel,
/// WRNWksLoop, WRNParallelLoop, WRNSections, WRNParallelSections, WRNTask
extern void printExtraForCancellationPoints(WRegionNode const *W,
                                            formatted_raw_ostream &OS,
                                            int Depth, unsigned Verbosity = 1);
} // End namespace vpo


} // End namespace llvm

#endif // LLVM_ANALYSIS_VPO_WREGION_H
#endif // INTEL_COLLAB
