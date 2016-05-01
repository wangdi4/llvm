//===----------------- WRegion.h - W-Region node ----------------*- C++ -*-===//
//
//   Copyright (C) 2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
//   This file defines the classes derived from WRegionNode that
//   correspond to each construct type in WRegionNode::WRegionNodeKind
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_WREGION_H
#define LLVM_ANALYSIS_VPO_WREGION_H

#include "llvm/IR/Intel_LoopIR/HLNode.h"
#include "llvm/Transforms/Intel_VPO/Utils/VPOUtils.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionNode.h"

#include <set>
#include <iterator>

namespace llvm {

class LoopInfo;
class Loop;

namespace vpo {

//
// Classes below represent REGIONS, one for each type in
// WRegionNode::WRegionNodeKind
//
// All of them are derived classes of the base WRegionNode
//
// For completeness, there's a class for each WRegionNodeKind, but depending
// on implementation, some classes may not be needed (eg, #pragma omp section
// was not represented in the icc implementation)
//
//    WRegionNodeKind:    Class name:             Example OpenMP pragma
//
//    WRNParallel         WRNParallelNode         #pragma omp parallel
//    WRNParallelLoop     WRNParallelLoopNode     #pragma omp parallel for
//    WRNParallelSections WRNParallelSectionsNode #pragma omp parallel sections
//    WRNTask             WRNTaskNode             #pragma omp task
//    WRNVecLoop          WRNVecLoopNode          #pragma omp simd
//    WRNWksLoop          WRNWksLoopNode          #pragma omp for
//    WRNWksSections      WRNWksSectionsNode      #pragma omp sections
//    WRNSection          WRNSectionNode          #pragma omp section
//    WRNSingle           WRNSingleNode           #pragma omp single
//    WRNMaster           WRNMasterNode           #pragma omp master
//    WRNAtomic           WRNAtomicNode           #pragma omp atomic
//    WRNBarrier          WRNBarrierNode          #pragma omp barrier
//    WRNCancel           WRNCancelNode           #pragma omp cancel
//    WRNCritical         WRNCriticalNode         #pragma omp critical
//    WRNFlush            WRNFlushNode            #pragma omp flush
//    WRNOrdered          WRNOrderedNode          #pragma omp ordered
//    WRNTaskgroup        WRNTaskgroupNode        #pragma omp taskgroup

//
// WRNParallelNode
//
// #pragma omp parallel
//
class WRNParallelNode : public WRegionNode {
private:
  SharedClause *Shared;
  PrivateClause *Priv;
  FirstprivateClause *Fpriv;
  ReductionClause *Reduction;
  CopyinClause *Copyin;
  EXPR IfExpr;
  EXPR NumThreads;
  WRNDefaultKind Default;
  WRNProcBindKind ProcBind;

protected:
  void setShared(SharedClause *S) { Shared = S; }
  void setPriv(PrivateClause *P) { Priv = P; }
  void setFpriv(FirstprivateClause *F) { Fpriv = F; }
  void setRed(ReductionClause *R) { Reduction = R; }
  void setCopyin(CopyinClause *C) { Copyin = C; }
  void setIf(EXPR E) { IfExpr = E; }
  void setNumThreads(EXPR E) { NumThreads = E; }
  void setDefault(WRNDefaultKind D) { Default = D; }
  void setProcBind(WRNProcBindKind P) { ProcBind = P; }

public:
  WRNParallelNode(BasicBlock *BB);
  WRNParallelNode(WRNParallelNode *W);

  SharedClause *getShared() const { return Shared; }
  PrivateClause *getPriv() const { return Priv; }
  FirstprivateClause *getFpriv() const { return Fpriv; }
  ReductionClause *getRed() const { return Reduction; }
  CopyinClause *getCopyin() const { return Copyin; }
  EXPR getIf() const { return IfExpr; }
  EXPR getNumThreads() const { return NumThreads; }
  WRNDefaultKind getDefault() const { return Default; }
  WRNProcBindKind getProcBind() const { return ProcBind; }

  void print(formatted_raw_ostream &OS, unsigned Depth) const;

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNParallel;
  }
};

//
// WRNParallelSectionsNode
//
// #pragma omp parallel sections
//
class WRNParallelSectionsNode : public WRegionNode {
private:
  SharedClause *Shared;
  PrivateClause *Priv;
  FirstprivateClause *Fpriv;
  ReductionClause *Reduction;
  CopyinClause *Copyin;
  EXPR IfExpr;
  EXPR NumThreads;
  WRNDefaultKind Default;
  WRNProcBindKind ProcBind;

protected:
  void setShared(SharedClause *S) { Shared = S; }
  void setPriv(PrivateClause *P) { Priv = P; }
  void setFpriv(FirstprivateClause *F) { Fpriv = F; }
  void setRed(ReductionClause *R) { Reduction = R; }
  void setCopyin(CopyinClause *C) { Copyin = C; }
  void setIf(EXPR E) { IfExpr = E; }
  void setNumThreads(EXPR E) { NumThreads = E; }
  void setDefault(WRNDefaultKind D) { Default = D; }
  void setProcBind(WRNProcBindKind P) { ProcBind = P; }

public:
  WRNParallelSectionsNode(BasicBlock *BB);
  WRNParallelSectionsNode(WRNParallelSectionsNode *W);

  SharedClause *getShared() const { return Shared; }
  PrivateClause *getPriv() const { return Priv; }
  FirstprivateClause *getFpriv() const { return Fpriv; }
  ReductionClause *getRed() const { return Reduction; }
  CopyinClause *getCopyin() const { return Copyin; }
  EXPR getIf() const { return IfExpr; }
  EXPR getNumThreads() const { return NumThreads; }
  WRNDefaultKind getDefault() const { return Default; }
  WRNProcBindKind getProcBind() const { return ProcBind; }

  void print(formatted_raw_ostream &OS, unsigned Depth) const;

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNParallelSections;
  }
};

//
// WRNParallelLoopNode
//
// #pragma omp parallel loop
//
class WRNParallelLoopNode : public WRegionNode {
private:
  SharedClause *Shared;
  PrivateClause *Priv;
  FirstprivateClause *Fpriv;
  ReductionClause *Reduction;
  CopyinClause *Copyin;
  EXPR IfExpr;
  EXPR NumThreads;
  WRNDefaultKind Default;
  WRNProcBindKind ProcBind;
  WRNScheduleKind Schedule;
  int Collapse;
  bool Ordered;
  LoopInfo *LI;
  Loop *Lp;

protected:
  void setShared(SharedClause *S) { Shared = S; }
  void setPriv(PrivateClause *P) { Priv = P; }
  void setFpriv(FirstprivateClause *F) { Fpriv = F; }
  void setRed(ReductionClause *R) { Reduction = R; }
  void setCopyin(CopyinClause *C) { Copyin = C; }
  void setIf(EXPR E) { IfExpr = E; }
  void setNumThreads(EXPR E) { NumThreads = E; }
  void setDefault(WRNDefaultKind D) { Default = D; }
  void setProcBind(WRNProcBindKind P) { ProcBind = P; }
  void setSchedule(WRNScheduleKind S) { Schedule = S; }
  void setCollapse(int N) { Collapse = N; }
  void setOrdered(bool Flag) { Ordered = Flag; }
  void setLoopInfo(LoopInfo *L) { LI = L; }
  void setLoop(Loop *L) { Lp = L; }

public:
  WRNParallelLoopNode(BasicBlock *BB, LoopInfo *L);
  WRNParallelLoopNode(WRNParallelLoopNode *W);

  SharedClause *getShared() const { return Shared; }
  PrivateClause *getPriv() const { return Priv; }
  FirstprivateClause *getFpriv() const { return Fpriv; }
  ReductionClause *getRed() const { return Reduction; }
  CopyinClause *getCopyin() const { return Copyin; }
  EXPR getIf() const { return IfExpr; }
  EXPR getNumThreads() const { return NumThreads; }
  WRNDefaultKind getDefault() const { return Default; }
  WRNProcBindKind getProcBind() const { return ProcBind; }
  WRNScheduleKind getSchedule() const { return Schedule; }
  int getCollapse() const { return Collapse; }
  bool getOrdered() const { return Ordered; }
  LoopInfo *getLoopInfo() const { return LI; }
  Loop *getLoop() const { return Lp; }

  void print(formatted_raw_ostream &OS, unsigned Depth) const;

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNParallelLoop;
  }
};

//
// WRNVecLoopNode
//
// #pragma omp simd
//
class WRNVecLoopNode : public WRegionNode {
private:
  PrivateClause *Priv;        // qualOpndList
  LastprivateClause *Lpriv;   // qualOpndList
  ReductionClause *Reduction; // qualOpndList
  LinearClause *Linear;       // qualOpndList
  UniformClause *Uniform;     // qualOpndList
  AlignedClause *Aligned;     // qualOpndList
  int Simdlen;                // qualOpnd
  int Safelen;                // qualOpnd
  int Collapse;               // qualOpnd
  bool IsAutoVec;
  LoopInfo *LI;                 // for LLVM IR only
  Loop *Lp;                     // for LLVM IR only
  loopopt::HLNode *EntryHLNode; // for HIR only
  loopopt::HLNode *ExitHLNode;  // for HIR only
  loopopt::HLLoop *HLp;         // for HIR only

public:
  void setPriv(PrivateClause *P) { Priv = P; }
  void setLpriv(LastprivateClause *L) { Lpriv = L; }
  void setRed(ReductionClause *R) { Reduction = R; }
  void setLinear(LinearClause *L) { Linear = L; }
  void setUniform(UniformClause *L) { Uniform = L; }
  void setAligned(AlignedClause *A) { Aligned = A; }
  void setSimdlen(int N) { Simdlen = N; }
  void setSafelen(int N) { Safelen = N; }
  void setCollapse(int N) { Collapse = N; }
  void setIsAutoVec(bool Flag) { IsAutoVec = Flag; }
  void setLoopInfo(LoopInfo *L) { LI = L; }
  void setLoop(Loop *L) { Lp = L; }
  void setEntryHLNode(loopopt::HLNode *E) { EntryHLNode = E; }
  void setExitHLNode(loopopt::HLNode *X) { ExitHLNode = X; }
  void setHLLoop(loopopt::HLLoop *L) { HLp = L; }

  WRNVecLoopNode(BasicBlock *BB, LoopInfo *L); // LLVM IR representation
  WRNVecLoopNode(loopopt::HLNode *EntryHLN);   // HIR representation
  WRNVecLoopNode(WRNVecLoopNode *W);
  // WRNVecLoopNode(const WRNVecLoopNode &W);  // copy constructor

  PrivateClause *getPriv() const { return Priv; }
  LastprivateClause *getLpriv() const { return Lpriv; }
  ReductionClause *getRed() const { return Reduction; }
  LinearClause *getLinear() const { return Linear; }
  UniformClause *getUniform() const { return Uniform; }
  AlignedClause *getAligned() const { return Aligned; }

  int getSimdlen() const { return Simdlen; }
  int getSafelen() const { return Safelen; }
  int getCollapse() const { return Collapse; }
  bool getIsAutoVec() const { return IsAutoVec; }
  LoopInfo *getLoopInfo() const { return LI; }
  Loop *getLoop() const { return Lp; }

  loopopt::HLNode *getEntryHLNode() const { return EntryHLNode; }
  loopopt::HLNode *getExitHLNode() const { return ExitHLNode; }
  loopopt::HLLoop *getHLLoop() const { return HLp; }

  void print(formatted_raw_ostream &OS, unsigned Depth) const;
  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNVecLoop;
  }
};

} // End namespace vpo

} // End namespace llvm

#endif
