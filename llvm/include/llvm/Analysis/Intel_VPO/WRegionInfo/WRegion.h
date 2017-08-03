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

#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLNode.h"
#include "llvm/Analysis/Intel_VPO/Utils/VPOAnalysisUtils.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionNode.h"


#include <set>
#include <iterator>

namespace llvm {

class LoopInfo;
class Loop;

namespace vpo {

///
/// Classes below represent REGIONS, one for each type in
/// WRegionNode::WRegionNodeKind
///
/// All of them are derived classes of the base WRegionNode
///
///    Class name:               Example OpenMP pragma
///
///    WRNParallelNode           #pragma omp parallel
///    WRNParallelLoopNode       #pragma omp parallel for
///    WRNParallelSectionsNode   #pragma omp parallel sections
///    WRNParallelWorkshareNode  !$omp parallel workshare
///    WRNTeamsNode              #pragma omp teams
///    WRNDistributeParLoopNode  #pragma omp distribute parallel for
///    WRNTargetNode             #pragma omp target
///    WRNTargetDataNode         #pragma omp target data
///    WRNTaskNode               #pragma omp task
///    WRNTaskloopNode           #pragma omp taskloop
///    WRNVecLoopNode            #pragma omp simd
///    WRNWksLoopNode            #pragma omp for
///    WRNSectionsNode           #pragma omp sections
///    WRNWorkshareNode          !$omp workshare
///    WRNDistributeNode         #pragma omp distribute
///    WRNAtomicNode             #pragma omp atomic
///    WRNBarrierNode            #pragma omp barrier
///    WRNCancelNode             #pragma omp cancel
///    WRNCriticalNode           #pragma omp critical
///    WRNFlushNode              #pragma omp flush
///    WRNOrderedNode            #pragma omp ordered
///    WRNMasterNode             #pragma omp master
///    WRNSingleNode             #pragma omp single
///    WRNTaskgroupNode          #pragma omp taskgroup
///    WRNTaskwaitNode           #pragma omp taskwait
///    WRNTaskyieldNode          #pragma omp taskyield


/// WRN for 
/// \code
///   #pragma omp parallel
/// \endcode
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

public:
  WRNParallelNode(BasicBlock *BB);
  WRNParallelNode(WRNParallelNode *W);

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

/// WRN for 
/// \code
///   #pragma omp parallel loop
/// \endcode
class WRNParallelLoopNode : public WRegionNode {
private:
  SharedClause *Shared;
  PrivateClause *Priv;
  FirstprivateClause *Fpriv;
  LastprivateClause *Lpriv;
  ReductionClause *Reduction;
  LinearClause *Linear;
  CopyinClause *Copyin;
  EXPR IfExpr;
  EXPR NumThreads;
  WRNDefaultKind Default;
  WRNProcBindKind ProcBind;
  ScheduleClause Schedule;
  int Collapse;
  int Ordered;
  LoopInfo *LI;
  Loop *Lp;

public:
  WRNParallelLoopNode(BasicBlock *BB, LoopInfo *L);
  WRNParallelLoopNode(WRNParallelLoopNode *W);

protected:
  void setShared(SharedClause *S) { Shared = S; }
  void setPriv(PrivateClause *P) { Priv = P; }
  void setFpriv(FirstprivateClause *F) { Fpriv = F; }
  void setLpriv(LastprivateClause *L) { Lpriv = L; }
  void setRed(ReductionClause *R) { Reduction = R; }
  void setCopyin(CopyinClause *C) { Copyin = C; }
  void setLinear(LinearClause *L) { Linear = L; }
  void setIf(EXPR E) { IfExpr = E; }
  void setNumThreads(EXPR E) { NumThreads = E; }
  void setDefault(WRNDefaultKind D) { Default = D; }
  void setProcBind(WRNProcBindKind P) { ProcBind = P; }
  void setSchedule(ScheduleClause S) { Schedule = S; }
  void setCollapse(int N) { Collapse = N; }
  void setOrdered(int N) { Ordered = N; }
  void setLoopInfo(LoopInfo *L) { LI = L; }
  void setLoop(Loop *L) { Lp = L; }

public:
  SharedClause *getShared() const { return Shared; }
  PrivateClause *getPriv() const { return Priv; }
  FirstprivateClause *getFpriv() const { return Fpriv; }
  LastprivateClause *getLpriv() const { return Lpriv; }
  ReductionClause *getRed() const { return Reduction; }
  LinearClause *getLinear() const { return Linear; }
  CopyinClause *getCopyin() const { return Copyin; }
  EXPR getIf() const { return IfExpr; }
  EXPR getNumThreads() const { return NumThreads; }
  WRNDefaultKind getDefault() const { return Default; }
  WRNProcBindKind getProcBind() const { return ProcBind; }
  ScheduleClause & getSchedule() { return Schedule; }
  int getCollapse() const { return Collapse; }
  int getOrdered() const { return Ordered; }
  LoopInfo *getLoopInfo() const { return LI; }
  Loop *getLoop() const { return Lp; }

  void print(formatted_raw_ostream &OS, unsigned Depth) const;

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
  SharedClause *Shared;
  PrivateClause *Priv;
  FirstprivateClause *Fpriv;
  ReductionClause *Reduction;
  CopyinClause *Copyin;
  EXPR IfExpr;
  EXPR NumThreads;
  WRNDefaultKind Default;
  WRNProcBindKind ProcBind;
  LoopInfo *LI;
  Loop *Lp;

public:
  WRNParallelSectionsNode(BasicBlock *BB, LoopInfo *L);

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
  void setLoopInfo(LoopInfo *L) { LI = L; }
  void setLoop(Loop *L) { Lp = L; }

public:
  SharedClause *getShared() const { return Shared; }
  PrivateClause *getPriv() const { return Priv; }
  FirstprivateClause *getFpriv() const { return Fpriv; }
  ReductionClause *getRed() const { return Reduction; }
  CopyinClause *getCopyin() const { return Copyin; }
  EXPR getIf() const { return IfExpr; }
  EXPR getNumThreads() const { return NumThreads; }
  WRNDefaultKind getDefault() const { return Default; }
  WRNProcBindKind getProcBind() const { return ProcBind; }
  LoopInfo *getLoopInfo() const { return LI; }
  Loop *getLoop() const { return Lp; }

  void print(formatted_raw_ostream &OS, unsigned Depth) const;

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
  SharedClause *Shared;
  PrivateClause *Priv;
  FirstprivateClause *Fpriv;
  ReductionClause *Reduction;
  CopyinClause *Copyin;
  EXPR IfExpr;
  EXPR NumThreads;
  WRNDefaultKind Default;
  WRNProcBindKind ProcBind;
  LoopInfo *LI;
  Loop *Lp;

public:
  WRNParallelWorkshareNode(BasicBlock *BB, LoopInfo *L);

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
  void setLoopInfo(LoopInfo *L) { LI = L; }
  void setLoop(Loop *L) { Lp = L; }

public:
  SharedClause *getShared() const { return Shared; }
  PrivateClause *getPriv() const { return Priv; }
  FirstprivateClause *getFpriv() const { return Fpriv; }
  ReductionClause *getRed() const { return Reduction; }
  CopyinClause *getCopyin() const { return Copyin; }
  EXPR getIf() const { return IfExpr; }
  EXPR getNumThreads() const { return NumThreads; }
  WRNDefaultKind getDefault() const { return Default; }
  WRNProcBindKind getProcBind() const { return ProcBind; }
  LoopInfo *getLoopInfo() const { return LI; }
  Loop *getLoop() const { return Lp; }

  void print(formatted_raw_ostream &OS, unsigned Depth) const;

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
  SharedClause *Shared;
  PrivateClause *Priv;
  FirstprivateClause *Fpriv;
  ReductionClause *Reduction;
  EXPR ThreadLimit;
  EXPR NumTeams;
  WRNDefaultKind Default;

public:
  WRNTeamsNode(BasicBlock *BB);

protected:
  void setShared(SharedClause *S) { Shared = S; }
  void setPriv(PrivateClause *P) { Priv = P; }
  void setFpriv(FirstprivateClause *F) { Fpriv = F; }
  void setRed(ReductionClause *R) { Reduction = R; }
  void setThreadLimit(EXPR E) { ThreadLimit = E; }
  void setNumTeams(EXPR E) { NumTeams = E; }
  void setDefault(WRNDefaultKind D) { Default = D; }

public:
  SharedClause *getShared() const { return Shared; }
  PrivateClause *getPriv() const { return Priv; }
  FirstprivateClause *getFpriv() const { return Fpriv; }
  ReductionClause *getRed() const { return Reduction; }
  EXPR getThreadLimit() const { return ThreadLimit; }
  EXPR getNumTeams() const { return NumTeams; }
  WRNDefaultKind getDefault() const { return Default; }

  void print(formatted_raw_ostream &OS, unsigned Depth) const;

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
  SharedClause *Shared;
  PrivateClause *Priv;
  FirstprivateClause *Fpriv;
  LastprivateClause *Lpriv;
  ReductionClause *Reduction;
  LinearClause *Linear;
  CopyinClause *Copyin;
  EXPR IfExpr;
  EXPR NumThreads;
  WRNDefaultKind Default;
  WRNProcBindKind ProcBind;
  ScheduleClause Schedule;
  ScheduleClause DistSchedule;
  int Collapse;
  int Ordered;
  LoopInfo *LI;
  Loop *Lp;

public:
  WRNDistributeParLoopNode(BasicBlock *BB, LoopInfo *L);

protected:
  void setShared(SharedClause *S) { Shared = S; }
  void setPriv(PrivateClause *P) { Priv = P; }
  void setFpriv(FirstprivateClause *F) { Fpriv = F; }
  void setLpriv(LastprivateClause *L) { Lpriv = L; }
  void setRed(ReductionClause *R) { Reduction = R; }
  void setLinear(LinearClause *L) { Linear = L; }
  void setCopyin(CopyinClause *C) { Copyin = C; }
  void setIf(EXPR E) { IfExpr = E; }
  void setNumThreads(EXPR E) { NumThreads = E; }
  void setDefault(WRNDefaultKind D) { Default = D; }
  void setProcBind(WRNProcBindKind P) { ProcBind = P; }
  void setSchedule(ScheduleClause S) { Schedule = S; }
  void setDistSchedule(ScheduleClause S) { DistSchedule = S; }
  void setCollapse(int N) { Collapse = N; }
  void setOrdered(int N) { Ordered = N; }
  void setLoopInfo(LoopInfo *L) { LI = L; }
  void setLoop(Loop *L) { Lp = L; }

public:
  SharedClause *getShared() const { return Shared; }
  PrivateClause *getPriv() const { return Priv; }
  FirstprivateClause *getFpriv() const { return Fpriv; }
  LastprivateClause *getLpriv() const { return Lpriv; }
  ReductionClause *getRed() const { return Reduction; }
  LinearClause *getLinear() const { return Linear; }
  CopyinClause *getCopyin() const { return Copyin; }
  EXPR getIf() const { return IfExpr; }
  EXPR getNumThreads() const { return NumThreads; }
  WRNDefaultKind getDefault() const { return Default; }
  WRNProcBindKind getProcBind() const { return ProcBind; }
  ScheduleClause & getSchedule() { return Schedule; }
  ScheduleClause & getDistSchedule() { return DistSchedule; }
  int getCollapse() const { return Collapse; }
  int getOrdered() const { return Ordered; }
  LoopInfo *getLoopInfo() const { return LI; }
  Loop *getLoop() const { return Lp; }

  void print(formatted_raw_ostream &OS, unsigned Depth) const;

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
  PrivateClause *Priv;
  FirstprivateClause *Fpriv;
  MapClause *Map;
  DependClause *Depend;
  IsDevicePtrClause *IsDevicePtr;
  EXPR IfExpr;
  EXPR Device;
  bool Nowait;
  bool DefaultmapTofromScalar;        // defaultmap(tofrom:scalar)

public:
  WRNTargetNode(BasicBlock *BB);

protected:
  void setPriv(PrivateClause *P) { Priv = P; }
  void setFpriv(FirstprivateClause *F) { Fpriv = F; }
  void setMap(MapClause *M) { Map = M; }
  void setDepend(DependClause *D) { Depend = D; }
  void setIsDevicePtr(IsDevicePtrClause *IDP) { IsDevicePtr = IDP; }
  void setIf(EXPR E) { IfExpr = E; }
  void setDevice(EXPR E) { Device = E; }
  void setNowait(bool Flag) { Nowait = Flag; }
  void setDefaultmapTofromScalar(bool Flag) { DefaultmapTofromScalar = Flag; }

public:
  PrivateClause *getPriv() const { return Priv; }
  FirstprivateClause *getFpriv() const { return Fpriv; }
  MapClause *getMap() const { return Map; }
  DependClause *getDepend() const { return Depend; }
  IsDevicePtrClause *getIsDevicePtr() const { return IsDevicePtr; }
  EXPR getIf() const { return IfExpr; }
  EXPR getDevice() const { return Device; }
  bool getNowait() const { return Nowait; }
  bool getDefaultmapTofromScalar() const { return DefaultmapTofromScalar; }

  void print(formatted_raw_ostream &OS, unsigned Depth) const;

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNTarget;
  }
};

/// This WRN is similar to WRNTargetNode but it does not offload a code block
/// to the device. It holds map clauses that describe data movement between
/// host and device as specified by any of the following OMP directives:
/// \code
///   #pragma omp target data
///   #pragma omp target enter data
///   #pragma omp target exit data
///   #pragma omp target update
/// \endcode
class WRNTargetDataNode : public WRegionNode {
private:
  MapClause *Map;  // used for the map clause and the to/from clauses
  DependClause *Depend;
  UseDevicePtrClause *UseDevicePtr;
  EXPR IfExpr;
  EXPR Device;
  bool Nowait;

public:
  WRNTargetDataNode(BasicBlock *BB);

protected:
  void setMap(MapClause *M) { Map = M; }
  void setDepend(DependClause *D) { Depend = D; }
  void setUseDevicePtr(UseDevicePtrClause *UDP) { UseDevicePtr = UDP; }
  void setIf(EXPR E) { IfExpr = E; }
  void setDevice(EXPR E) { Device = E; }
  void setNowait(bool Flag) { Nowait = Flag; }

public:
  MapClause *getMap() const { return Map; }
  DependClause *getDepend() const { return Depend; }
  UseDevicePtrClause *getUseDevicePtr() const { return UseDevicePtr; }
  EXPR getIf() const { return IfExpr; }
  EXPR getDevice() const { return Device; }
  bool getNowait() const { return Nowait; }

  void print(formatted_raw_ostream &OS, unsigned Depth) const;

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNTargetData;
  }
};

/// WRN for 
/// \code
///   #pragma omp task
/// \endcode
class WRNTaskNode : public WRegionNode {
private:
  SharedClause *Shared;
  PrivateClause *Priv;
  FirstprivateClause *Fpriv;
  DependClause *Depend;
  EXPR Final;
  EXPR IfExpr;
  EXPR Priority;
  WRNDefaultKind Default;
  bool Untied;
  bool Mergeable;

public:
  WRNTaskNode(BasicBlock *BB);

protected:
  void setShared(SharedClause *S) { Shared = S; }
  void setPriv(PrivateClause *P) { Priv = P; }
  void setFpriv(FirstprivateClause *F) { Fpriv = F; }
  void setDepend(DependClause *D) { Depend = D; }
  void setFinal(EXPR E) { Final = E; }
  void setIf(EXPR E) { IfExpr = E; }
  void setPriority(EXPR E) { Priority = E; }
  void setDefault(WRNDefaultKind D) { Default = D; }
  void setUntied(bool Flag) { Untied = Flag; }
  void setMergeable(bool Flag) { Mergeable = Flag; }

public:
  SharedClause *getShared() const { return Shared; }
  PrivateClause *getPriv() const { return Priv; }
  FirstprivateClause *getFpriv() const { return Fpriv; }
  DependClause *getDepend() const { return Depend; }
  EXPR getFinal() const { return Final; }
  EXPR getIf() const { return IfExpr; }
  EXPR getPriority() const { return Priority; }
  WRNDefaultKind getDefault() const { return Default; }
  bool getUntied() const { return Untied; }
  bool getMergeable() const { return Mergeable; }

  void print(formatted_raw_ostream &OS, unsigned Depth) const;

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNTask;
  }
};

/// WRN for 
/// \code
///   #pragma omp taskloop
/// \endcode
class WRNTaskloopNode : public WRegionNode {
private:
  SharedClause *Shared;
  PrivateClause *Priv;
  FirstprivateClause *Fpriv;
  LastprivateClause *Lpriv;
  DependClause *Depend;
  EXPR Final;
  EXPR Grainsize;
  EXPR IfExpr;
  EXPR NumTasks;
  EXPR Priority;
  WRNDefaultKind Default;
  int Collapse;
  bool Untied;
  bool Mergeable;
  bool Nogroup;
  LoopInfo *LI;
  Loop *Lp;

public:
  WRNTaskloopNode(BasicBlock *BB, LoopInfo *L);

protected:
  void setShared(SharedClause *S) { Shared = S; }
  void setPriv(PrivateClause *P) { Priv = P; }
  void setFpriv(FirstprivateClause *F) { Fpriv = F; }
  void setLpriv(LastprivateClause *L) { Lpriv = L; }
  void setDepend(DependClause *D) { Depend = D; }
  void setFinal(EXPR E) { Final = E; }
  void setGrainsize(EXPR E) { Grainsize = E; }
  void setIf(EXPR E) { IfExpr = E; }
  void setNumTasks(EXPR E) { NumTasks = E; }
  void setPriority(EXPR E) { Priority = E; }
  void setDefault(WRNDefaultKind D) { Default = D; }
  void setCollapse(int N) { Collapse = N; }
  void setUntied(bool Flag) { Untied = Flag; }
  void setMergeable(bool Flag) { Mergeable = Flag; }
  void setNogroup(bool Flag) { Nogroup = Flag; }
  void setLoopInfo(LoopInfo *L) { LI = L; }
  void setLoop(Loop *L) { Lp = L; }

public:
  SharedClause *getShared() const { return Shared; }
  PrivateClause *getPriv() const { return Priv; }
  FirstprivateClause *getFpriv() const { return Fpriv; }
  LastprivateClause *getLpriv() const { return Lpriv; }
  DependClause *getDepend() const { return Depend; }
  EXPR getFinal() const { return Final; }
  EXPR getGrainsize() const { return Grainsize; }
  EXPR getIf() const { return IfExpr; }
  EXPR getNumTasks() const { return NumTasks; }
  EXPR getPriority() const { return Priority; }
  WRNDefaultKind getDefault() const { return Default; }
  int getCollapse() const { return Collapse; }
  bool getUntied() const { return Untied; }
  bool getMergeable() const { return Mergeable; }
  bool getNogroup() const { return Nogroup; }
  LoopInfo *getLoopInfo() const { return LI; }
  Loop *getLoop() const { return Lp; }

  void print(formatted_raw_ostream &OS, unsigned Depth) const;

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
  PrivateClause *Priv;
  LastprivateClause *Lpriv;
  ReductionClause *Reduction;
  LinearClause *Linear;
  AlignedClause *Aligned;
  UniformClause *Uniform; // The simd construct does not take a uniform clause,
                          // so we won't get this from the front-end, but this
                          // list can/will be populated by the vector backend
  int Simdlen;
  int Safelen;
  int Collapse;
  bool IsAutoVec;
  LoopInfo *LI;                 // for LLVM IR only
  Loop *Lp;                     // for LLVM IR only
  loopopt::HLNode *EntryHLNode; // for HIR only
  loopopt::HLNode *ExitHLNode;  // for HIR only
  loopopt::HLLoop *HLp;         // for HIR only

public:
  WRNVecLoopNode(BasicBlock *BB, LoopInfo *L); // LLVM IR representation
  WRNVecLoopNode(loopopt::HLNode *EntryHLN);   // HIR representation
  WRNVecLoopNode(WRNVecLoopNode *W);
  // WRNVecLoopNode(const WRNVecLoopNode &W);  // copy constructor

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

/// WRN for 
/// \code
///   #pragma omp for
/// \endcode
class WRNWksLoopNode : public WRegionNode {
private:
  PrivateClause *Priv;
  FirstprivateClause *Fpriv;
  LastprivateClause *Lpriv;
  ReductionClause *Reduction;
  LinearClause *Linear;
  ScheduleClause Schedule;
  int Collapse;
  int Ordered;
  bool Nowait;
  LoopInfo *LI;
  Loop *Lp;

public:
  WRNWksLoopNode(BasicBlock *BB, LoopInfo *L);

protected:
  void setPriv(PrivateClause *P) { Priv = P; }
  void setFpriv(FirstprivateClause *F) { Fpriv = F; }
  void setLpriv(LastprivateClause *L) { Lpriv = L; }
  void setRed(ReductionClause *R) { Reduction = R; }
  void setLinear(LinearClause *L) { Linear = L; }
  void setSchedule(ScheduleClause S) { Schedule = S; }
  void setCollapse(int N) { Collapse = N; }
  void setOrdered(int N) { Ordered = N; }
  void setNowait(bool Flag) { Nowait = Flag; }
  void setLoopInfo(LoopInfo *L) { LI = L; }
  void setLoop(Loop *L) { Lp = L; }

public:
  PrivateClause *getPriv() const { return Priv; }
  FirstprivateClause *getFpriv() const { return Fpriv; }
  LastprivateClause *getLpriv() const { return Lpriv; }
  ReductionClause *getRed() const { return Reduction; }
  LinearClause *getLinear() const { return Linear; }
  ScheduleClause & getSchedule() { return Schedule; }
  int getCollapse() const { return Collapse; }
  int getOrdered() const { return Ordered; }
  bool getNowait() const { return Nowait; }
  LoopInfo *getLoopInfo() const { return LI; }
  Loop *getLoop() const { return Lp; }

  void print(formatted_raw_ostream &OS, unsigned Depth) const;

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
  PrivateClause *Priv;
  FirstprivateClause *Fpriv;
  LastprivateClause *Lpriv;
  ReductionClause *Reduction;
  bool Nowait;
  LoopInfo *LI;
  Loop *Lp;

public:
  WRNSectionsNode(BasicBlock *BB, LoopInfo *L);

protected:
  void setPriv(PrivateClause *P) { Priv = P; }
  void setFpriv(FirstprivateClause *F) { Fpriv = F; }
  void setLpriv(LastprivateClause *L) { Lpriv = L; }
  void setRed(ReductionClause *R) { Reduction = R; }
  void setNowait(bool Flag) { Nowait = Flag; }
  void setLoopInfo(LoopInfo *L) { LI = L; }
  void setLoop(Loop *L) { Lp = L; }

public:
  PrivateClause *getPriv() const { return Priv; }
  FirstprivateClause *getFpriv() const { return Fpriv; }
  LastprivateClause *getLpriv() const { return Lpriv; }
  ReductionClause *getRed() const { return Reduction; }
  bool getNowait() const { return Nowait; }
  LoopInfo *getLoopInfo() const { return LI; }
  Loop *getLoop() const { return Lp; }

  void print(formatted_raw_ostream &OS, unsigned Depth) const;

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNSections;
  }
};

/// Fortran-only WRN for 
/// \code
///   !$omp workshare
/// \endcode
class WRNWorkshareNode : public WRegionNode {
private:
  bool Nowait;
  LoopInfo *LI;
  Loop *Lp;

public:
  WRNWorkshareNode(BasicBlock *BB, LoopInfo *L);

protected:
  void setNowait(bool Flag) { Nowait = Flag; }
  void setLoopInfo(LoopInfo *L) { LI = L; }
  void setLoop(Loop *L) { Lp = L; }

public:
  bool getNowait() const { return Nowait; }
  LoopInfo *getLoopInfo() const { return LI; }
  Loop *getLoop() const { return Lp; }

  void print(formatted_raw_ostream &OS, unsigned Depth) const;

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
  PrivateClause *Priv;
  FirstprivateClause *Fpriv;
  LastprivateClause *Lpriv;
  ScheduleClause DistSchedule;
  int Collapse;
  LoopInfo *LI;
  Loop *Lp;

public:
  WRNDistributeNode(BasicBlock *BB, LoopInfo *L);

protected:
  void setPriv(PrivateClause *P) { Priv = P; }
  void setFpriv(FirstprivateClause *F) { Fpriv = F; }
  void setLpriv(LastprivateClause *L) { Lpriv = L; }
  void setDistSchedule(ScheduleClause S) { DistSchedule = S; }
  void setCollapse(int N) { Collapse = N; }
  void setLoopInfo(LoopInfo *L) { LI = L; }
  void setLoop(Loop *L) { Lp = L; }

public:
  PrivateClause *getPriv() const { return Priv; }
  FirstprivateClause *getFpriv() const { return Fpriv; }
  LastprivateClause *getLpriv() const { return Lpriv; }
  ScheduleClause & getDistSchedule() { return DistSchedule; }
  int getCollapse() const { return Collapse; }
  LoopInfo *getLoopInfo() const { return LI; }
  Loop *getLoop() const { return Lp; }

  void print(formatted_raw_ostream &OS, unsigned Depth) const;

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
  void setAtomicKind(WRNAtomicKind AK) { AtomicKind = AK; }
  void setHasSeqCstClause(bool SC) { HasSeqCstClause = SC; }

public:
  WRNAtomicNode(BasicBlock *BB);
  WRNAtomicNode(WRNAtomicNode *W);

  WRNAtomicKind getAtomicKind() const { return AtomicKind; }
  bool getHasSeqCstClause() const { return HasSeqCstClause; }

  void print(formatted_raw_ostream &OS, unsigned Depth) const;

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
  FlushSet *FValueSet;  // qualOpndList

protected:
  void setFlush(FlushSet *FS) { FValueSet = FS; }

public:

  WRNFlushNode(BasicBlock *BB);
  WRNFlushNode(WRNFlushNode *W);

  FlushSet *getFlush() const { return FValueSet; }

  void print(formatted_raw_ostream &OS, unsigned Depth) const;

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNFlush;
  }
};

/// WRN for 
/// \code
///   #pragma omp barrier
/// \endcode
class WRNBarrierNode : public WRegionNode {
public:
  WRNBarrierNode(BasicBlock *BB);

  void print(formatted_raw_ostream &OS, unsigned Depth) const;

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
  void setCancelKind(WRNCancelKind CK) { CancelKind = CK; }
  void setIf(EXPR E) { IfExpr = E; }

public:
  bool getIsCancellationPoint() const { return IsCancellationPoint; }
  WRNCancelKind getCancelKind() const { return CancelKind; }
  EXPR getIf() const { return IfExpr; }

  void print(formatted_raw_ostream &OS, unsigned Depth) const;

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNCancel;
  }
};

/// WRN for 
/// \code
///   #pragma omp master
/// \endcode
class WRNMasterNode : public WRegionNode {
public:
  WRNMasterNode(BasicBlock *BB);
  WRNMasterNode(WRNMasterNode *W);

  void print(formatted_raw_ostream &OS, unsigned Depth) const;

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNMaster;
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

  // The following field is meaningful only if IsDoacross==false
  bool IsThreads;          // true for "threads" (default); false for "simd"

  // The following two fields are meaningful only if IsDoacross==true
  bool IsDepSource;        // true if the directive has depend(source)
  DepSinkClause *DepSink;  // meaningful only when DepSource==false
  void assertDoacrossTrue() const  { assert (IsDoacross &&
                              "This WRNOrdered represents Doacross"); }
  void assertDoacrossFalse() const { assert (!IsDoacross &&
                              "This WRNOrdered does not represent Doacross"); }

public:
  WRNOrderedNode(BasicBlock *BB);
  WRNOrderedNode(WRNOrderedNode *W);

protected:
  void setIsDoacross(bool Flag) { IsDoacross = Flag; }
  void setThreads(bool Flag) { assertDoacrossFalse(); IsThreads = Flag; }
  void setDepSource(bool Flag) { assertDoacrossTrue(); IsDepSource = Flag; }
  void setDepSink(DepSinkClause *D) { assertDoacrossTrue(); DepSink = D; }

public:
  bool getIsDoacross() const {  return IsDoacross; }
  bool getIsThreads() const { assertDoacrossFalse(); return IsThreads; }
  bool getIsDepSource() const { assertDoacrossTrue(); return IsDepSource; }
  DepSinkClause *getDepSink() const { assertDoacrossTrue(); return DepSink; }

  void print(formatted_raw_ostream &OS, unsigned Depth) const;

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
  PrivateClause *Priv;
  FirstprivateClause *Fpriv;
  CopyprivateClause *Cpriv;
  bool Nowait;

public:
  WRNSingleNode(BasicBlock *BB);
  WRNSingleNode(WRNSingleNode *W);

protected:
  void setPriv(PrivateClause *P) { Priv = P; }
  void setFpriv(FirstprivateClause *F) { Fpriv = F; }
  void setCpriv(CopyprivateClause *C) { Cpriv = C; }
  void setNowait(bool Flag) { Nowait = Flag; }

public:
  PrivateClause *getPriv() const { return Priv; }
  FirstprivateClause *getFpriv() const { return Fpriv; }
  CopyprivateClause *getCpriv() const { return Cpriv; }
  bool getNowait() const { return Nowait; }

  void print(formatted_raw_ostream &OS, unsigned Depth) const;

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

protected:
  void setUserLockName(StringRef LN) { UserLockName = LN; }

public:
  WRNCriticalNode(BasicBlock *BB);
  WRNCriticalNode(WRNCriticalNode *W);

  StringRef getUserLockName() const { return UserLockName.str(); }
  void print(formatted_raw_ostream &OS, unsigned Depth) const;

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

public:
  WRNTaskgroupNode(BasicBlock *BB);

  void print(formatted_raw_ostream &OS, unsigned Depth) const;

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

public:
  WRNTaskwaitNode(BasicBlock *BB);

  void print(formatted_raw_ostream &OS, unsigned Depth) const;

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

  void print(formatted_raw_ostream &OS, unsigned Depth) const;

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNTaskyield;
  }
};

} // End namespace vpo

} // End namespace llvm

#endif
