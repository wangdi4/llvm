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

// Macro to define getters for list-type clauses. It creates two versions:
//   1. a const version used primarily by dumpers
//   2. a nonconst version used by parsing and other code
#define DEFINE_GETTER(CLAUSETYPE, GETTER, CLAUSEOBJ)       \
   const CLAUSETYPE &GETTER() const { return CLAUSEOBJ; }  \
         CLAUSETYPE &GETTER()       { return CLAUSEOBJ; }


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
  CopyinClause Copyin;
  EXPR IfExpr;
  EXPR NumThreads;
  WRNDefaultKind Default;
  WRNProcBindKind ProcBind;

public:
  WRNParallelNode(BasicBlock *BB);

protected:
  void setIf(EXPR E) { IfExpr = E; }
  void setNumThreads(EXPR E) { NumThreads = E; }
  void setDefault(WRNDefaultKind D) { Default = D; }
  void setProcBind(WRNProcBindKind P) { ProcBind = P; }

public:
  DEFINE_GETTER(SharedClause,       getShared, Shared)
  DEFINE_GETTER(PrivateClause,      getPriv,   Priv)
  DEFINE_GETTER(FirstprivateClause, getFpriv,  Fpriv)
  DEFINE_GETTER(ReductionClause,    getRed,    Reduction)
  DEFINE_GETTER(CopyinClause,       getCopyin, Copyin)

  EXPR getIf() const { return IfExpr; }
  EXPR getNumThreads() const { return NumThreads; }
  WRNDefaultKind getDefault() const { return Default; }
  WRNProcBindKind getProcBind() const { return ProcBind; }

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
  SharedClause Shared;
  PrivateClause Priv;
  FirstprivateClause Fpriv;
  LastprivateClause Lpriv;
  ReductionClause Reduction;
  LinearClause Linear;
  CopyinClause Copyin;
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

protected:
  void setIf(EXPR E) { IfExpr = E; }
  void setNumThreads(EXPR E) { NumThreads = E; }
  void setDefault(WRNDefaultKind D) { Default = D; }
  void setProcBind(WRNProcBindKind P) { ProcBind = P; }
  void setCollapse(int N) { Collapse = N; }
  void setOrdered(int N) { Ordered = N; }
  void setLoopInfo(LoopInfo *L) { LI = L; }
  void setLoop(Loop *L) { Lp = L; }

public:
  DEFINE_GETTER(SharedClause,       getShared, Shared)
  DEFINE_GETTER(PrivateClause,      getPriv,   Priv)
  DEFINE_GETTER(FirstprivateClause, getFpriv,  Fpriv)
  DEFINE_GETTER(LastprivateClause,  getLpriv,  Lpriv)
  DEFINE_GETTER(ReductionClause,    getRed,    Reduction)
  DEFINE_GETTER(LinearClause,       getLinear, Linear)
  DEFINE_GETTER(CopyinClause,       getCopyin, Copyin)
  DEFINE_GETTER(ScheduleClause,     getSchedule, Schedule)

  EXPR getIf() const { return IfExpr; }
  EXPR getNumThreads() const { return NumThreads; }
  WRNDefaultKind getDefault() const { return Default; }
  WRNProcBindKind getProcBind() const { return ProcBind; }
  int getCollapse() const { return Collapse; }
  int getOrdered() const { return Ordered; }
  LoopInfo *getLoopInfo() const { return LI; }
  Loop *getLoop() const { return Lp; }

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
  ReductionClause Reduction;
  CopyinClause Copyin;
  EXPR IfExpr;
  EXPR NumThreads;
  WRNDefaultKind Default;
  WRNProcBindKind ProcBind;
  LoopInfo *LI;
  Loop *Lp;

public:
  WRNParallelSectionsNode(BasicBlock *BB, LoopInfo *L);

protected:
  void setIf(EXPR E) { IfExpr = E; }
  void setNumThreads(EXPR E) { NumThreads = E; }
  void setDefault(WRNDefaultKind D) { Default = D; }
  void setProcBind(WRNProcBindKind P) { ProcBind = P; }
  void setLoopInfo(LoopInfo *L) { LI = L; }
  void setLoop(Loop *L) { Lp = L; }

public:
  DEFINE_GETTER(SharedClause,       getShared, Shared)
  DEFINE_GETTER(PrivateClause,      getPriv,   Priv)
  DEFINE_GETTER(FirstprivateClause, getFpriv,  Fpriv)
  DEFINE_GETTER(ReductionClause,    getRed,    Reduction)
  DEFINE_GETTER(CopyinClause,       getCopyin, Copyin)

  EXPR getIf() const { return IfExpr; }
  EXPR getNumThreads() const { return NumThreads; }
  WRNDefaultKind getDefault() const { return Default; }
  WRNProcBindKind getProcBind() const { return ProcBind; }
  LoopInfo *getLoopInfo() const { return LI; }
  Loop *getLoop() const { return Lp; }

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
  CopyinClause Copyin;
  EXPR IfExpr;
  EXPR NumThreads;
  WRNDefaultKind Default;
  WRNProcBindKind ProcBind;
  LoopInfo *LI;
  Loop *Lp;

public:
  WRNParallelWorkshareNode(BasicBlock *BB, LoopInfo *L);

protected:
  void setIf(EXPR E) { IfExpr = E; }
  void setNumThreads(EXPR E) { NumThreads = E; }
  void setDefault(WRNDefaultKind D) { Default = D; }
  void setProcBind(WRNProcBindKind P) { ProcBind = P; }
  void setLoopInfo(LoopInfo *L) { LI = L; }
  void setLoop(Loop *L) { Lp = L; }

public:
  DEFINE_GETTER(SharedClause,       getShared, Shared)
  DEFINE_GETTER(PrivateClause,      getPriv,   Priv)
  DEFINE_GETTER(FirstprivateClause, getFpriv,  Fpriv)
  DEFINE_GETTER(ReductionClause,    getRed,    Reduction)
  DEFINE_GETTER(CopyinClause,       getCopyin, Copyin)

  EXPR getIf() const { return IfExpr; }
  EXPR getNumThreads() const { return NumThreads; }
  WRNDefaultKind getDefault() const { return Default; }
  WRNProcBindKind getProcBind() const { return ProcBind; }
  LoopInfo *getLoopInfo() const { return LI; }
  Loop *getLoop() const { return Lp; }

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
  EXPR ThreadLimit;
  EXPR NumTeams;
  WRNDefaultKind Default;

public:
  WRNTeamsNode(BasicBlock *BB);

protected:
  void setThreadLimit(EXPR E) { ThreadLimit = E; }
  void setNumTeams(EXPR E) { NumTeams = E; }
  void setDefault(WRNDefaultKind D) { Default = D; }

public:
  DEFINE_GETTER(SharedClause,       getShared, Shared)
  DEFINE_GETTER(PrivateClause,      getPriv,   Priv)
  DEFINE_GETTER(FirstprivateClause, getFpriv,  Fpriv)
  DEFINE_GETTER(ReductionClause,    getRed,    Reduction)

  EXPR getThreadLimit() const { return ThreadLimit; }
  EXPR getNumTeams() const { return NumTeams; }
  WRNDefaultKind getDefault() const { return Default; }

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
  LoopInfo *LI;
  Loop *Lp;

public:
  WRNDistributeParLoopNode(BasicBlock *BB, LoopInfo *L);

protected:
  void setIf(EXPR E) { IfExpr = E; }
  void setNumThreads(EXPR E) { NumThreads = E; }
  void setDefault(WRNDefaultKind D) { Default = D; }
  void setProcBind(WRNProcBindKind P) { ProcBind = P; }
  void setCollapse(int N) { Collapse = N; }
  void setOrdered(int N) { Ordered = N; }
  void setLoopInfo(LoopInfo *L) { LI = L; }
  void setLoop(Loop *L) { Lp = L; }

public:
  DEFINE_GETTER(SharedClause,       getShared, Shared)
  DEFINE_GETTER(PrivateClause,      getPriv,   Priv)
  DEFINE_GETTER(FirstprivateClause, getFpriv,  Fpriv)
  DEFINE_GETTER(LastprivateClause,  getLpriv,  Lpriv)
  DEFINE_GETTER(ReductionClause,    getRed,    Reduction)
  DEFINE_GETTER(LinearClause,       getLinear, Linear)
  DEFINE_GETTER(CopyinClause,       getCopyin, Copyin)
  DEFINE_GETTER(ScheduleClause,     getSchedule, Schedule)
  DEFINE_GETTER(ScheduleClause,     getDistSchedule, Schedule)

  EXPR getIf() const { return IfExpr; }
  EXPR getNumThreads() const { return NumThreads; }
  WRNDefaultKind getDefault() const { return Default; }
  WRNProcBindKind getProcBind() const { return ProcBind; }
  int getCollapse() const { return Collapse; }
  int getOrdered() const { return Ordered; }
  LoopInfo *getLoopInfo() const { return LI; }
  Loop *getLoop() const { return Lp; }

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
  DependClause Depend;
  IsDevicePtrClause IsDevicePtr;
  EXPR IfExpr;
  EXPR Device;
  bool Nowait;
  bool DefaultmapTofromScalar;        // defaultmap(tofrom:scalar)

public:
  WRNTargetNode(BasicBlock *BB);

protected:
  void setIf(EXPR E) { IfExpr = E; }
  void setDevice(EXPR E) { Device = E; }
  void setNowait(bool Flag) { Nowait = Flag; }
  void setDefaultmapTofromScalar(bool Flag) { DefaultmapTofromScalar = Flag; }

public:
  DEFINE_GETTER(PrivateClause,      getPriv,        Priv)
  DEFINE_GETTER(FirstprivateClause, getFpriv,       Fpriv)
  DEFINE_GETTER(MapClause,          getMap,         Map)
  DEFINE_GETTER(DependClause,       getDepend,      Depend)
  DEFINE_GETTER(IsDevicePtrClause,  getIsDevicePtr, IsDevicePtr)

  EXPR getIf() const { return IfExpr; }
  EXPR getDevice() const { return Device; }
  bool getNowait() const { return Nowait; }
  bool getDefaultmapTofromScalar() const { return DefaultmapTofromScalar; }

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
  MapClause Map;  // used for the map clause and the to/from clauses
  DependClause Depend;
  UseDevicePtrClause UseDevicePtr;
  EXPR IfExpr;
  EXPR Device;
  bool Nowait;

public:
  WRNTargetDataNode(BasicBlock *BB);

protected:
  void setIf(EXPR E) { IfExpr = E; }
  void setDevice(EXPR E) { Device = E; }
  void setNowait(bool Flag) { Nowait = Flag; }

public:
  DEFINE_GETTER(MapClause,          getMap,          Map)
  DEFINE_GETTER(DependClause,       getDepend,       Depend)
  DEFINE_GETTER(UseDevicePtrClause, getUseDevicePtr, UseDevicePtr)

  EXPR getIf() const { return IfExpr; }
  EXPR getDevice() const { return Device; }
  bool getNowait() const { return Nowait; }

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNTargetData;
  }
};

/// \brief Task flags used to invoke tasking RTL for both Task and Taskloop
enum WRNTaskFlag : uint32_t {
  Tied         = 0x00000001,  // bit 1
  Final        = 0x00000002,  // bit 2
  MergedIf0    = 0x00000004,  // bit 3
  DtorThunk    = 0x00000008,  // bit 4
  Proxy        = 0x00000010   // bit 5

  // bits  6-16: reserved for compiler
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
  ReductionClause Reduction;
  DependClause Depend;
  EXPR Final;
  EXPR IfExpr;
  EXPR Priority;
  WRNDefaultKind Default;
  bool Untied;
  bool Mergeable;
  unsigned TaskFlag; // flag bit vector used to invoke tasking RTL

public:
  WRNTaskNode(BasicBlock *BB);

protected:
  void setFinal(EXPR E) { Final = E; }
  void setIf(EXPR E) { IfExpr = E; }
  void setPriority(EXPR E) { Priority = E; }
  void setDefault(WRNDefaultKind D) { Default = D; }
  void setUntied(bool B) { Untied = B; }
  void setMergeable(bool B) { Mergeable = B; }
  void setTaskFlag(unsigned F) { TaskFlag = F; }

public:
  DEFINE_GETTER(SharedClause,       getShared, Shared)
  DEFINE_GETTER(PrivateClause,      getPriv,   Priv)
  DEFINE_GETTER(FirstprivateClause, getFpriv,  Fpriv)
  DEFINE_GETTER(ReductionClause,    getRed,    Reduction)
  DEFINE_GETTER(DependClause,       getDepend, Depend)

  EXPR getFinal() const { return Final; }
  EXPR getIf() const { return IfExpr; }
  EXPR getPriority() const { return Priority; }
  WRNDefaultKind getDefault() const { return Default; }
  bool getUntied() const { return Untied; }
  bool getMergeable() const { return Mergeable; }
  unsigned getTaskFlag() const { return TaskFlag; }

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNTask;
  }
};

/// WRN for 
/// \code
///   #pragma omp taskloop
/// \endcode
class WRNTaskloopNode : public WRNTaskNode {
private:
  LastprivateClause Lpriv;
  EXPR Grainsize;
  EXPR NumTasks;
  int SchedCode; // 1 for Grainsize, 2 for num_tasks, 0 for none.
  int Collapse;
  bool Nogroup;
  LoopInfo *LI;
  Loop *Lp;
  // These taskloop clauses are also clauses in the parent class WRNTaskNode
  //   SharedClause Shared;
  //   PrivateClause Priv;
  //   FirstprivateClause Fpriv;
  //   DependClause Depend;
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
  void setGrainsize(EXPR E) { Grainsize = E; }
  void setNumTasks(EXPR E) { NumTasks = E; }
  void setSchedCode(int N) { SchedCode = N; }
  void setCollapse(int N) { Collapse = N; }
  void setNogroup(bool B) { Nogroup = B; }
  void setLoopInfo(LoopInfo *L) { LI = L; }
  void setLoop(Loop *L) { Lp = L; }

  // Defined in parent class WRNTaskNode
  //   void setFinal(EXPR E) { Final = E; }
  //   void setif(expr e) { ifexpr = e; }
  //   void setPriority(EXPR E) { Priority = E; }
  //   void setDefault(WRNDefaultKind D) { Default = D; }
  //   void setUntied(bool B) { Untied = B; }
  //   void setMergeable(bool B) { Mergeable = B; }
  //   void setTaskFlag(unsigned F) { TaskFlag = F; }

public:
  DEFINE_GETTER(LastprivateClause,  getLpriv,  Lpriv)
  EXPR getGrainsize() const { return Grainsize; }
  EXPR getNumTasks() const { return NumTasks; }
  int getSchedCode() const { return SchedCode; }
  int getCollapse() const { return Collapse; }
  bool getNogroup() const { return Nogroup; }
  LoopInfo *getLoopInfo() const { return LI; }
  Loop *getLoop() const { return Lp; }

  // Defined in parent class WRNTaskNode
  //   DEFINE_GETTER(SharedClause,       getShared, Shared)
  //   DEFINE_GETTER(PrivateClause,      getPriv,   Priv)
  //   DEFINE_GETTER(FirstprivateClause, getFpriv,  Fpriv)
  //   DEFINE_GETTER(DependClause,       getDepend, Depend)
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
  ReductionClause Reduction;
  LinearClause Linear;
  AlignedClause Aligned;
  UniformClause Uniform; // The simd construct does not take a uniform clause,
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

  void setSimdlen(int N) { Simdlen = N; }
  void setSafelen(int N) { Safelen = N; }
  void setCollapse(int N) { Collapse = N; }
  void setIsAutoVec(bool Flag) { IsAutoVec = Flag; }
  void setLoopInfo(LoopInfo *L) { LI = L; }
  void setLoop(Loop *L) { Lp = L; }
  void setEntryHLNode(loopopt::HLNode *E) { EntryHLNode = E; }
  void setExitHLNode(loopopt::HLNode *X) { ExitHLNode = X; }
  void setHLLoop(loopopt::HLLoop *L) { HLp = L; }

  DEFINE_GETTER(PrivateClause,     getPriv,    Priv)
  DEFINE_GETTER(LastprivateClause, getLpriv,   Lpriv)
  DEFINE_GETTER(ReductionClause,   getRed,     Reduction)
  DEFINE_GETTER(LinearClause,      getLinear,  Linear)
  DEFINE_GETTER(AlignedClause,     getAligned, Aligned)
  DEFINE_GETTER(UniformClause,     getUniform, Uniform)

  int getSimdlen() const { return Simdlen; }
  int getSafelen() const { return Safelen; }
  int getCollapse() const { return Collapse; }
  bool getIsAutoVec() const { return IsAutoVec; }
  LoopInfo *getLoopInfo() const { return LI; }
  Loop *getLoop() const { return Lp; }
  loopopt::HLNode *getEntryHLNode() const { return EntryHLNode; }
  loopopt::HLNode *getExitHLNode() const { return ExitHLNode; }
  loopopt::HLLoop *getHLLoop() const { return HLp; }

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                             bool Verbose=false) const;

  void printHIR(formatted_raw_ostream &OS, unsigned Depth,
                                             bool Verbose=false) const;

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
  PrivateClause Priv;
  FirstprivateClause Fpriv;
  LastprivateClause Lpriv;
  ReductionClause Reduction;
  LinearClause Linear;
  ScheduleClause Schedule;
  int Collapse;
  int Ordered;
  bool Nowait;
  LoopInfo *LI;
  Loop *Lp;

public:
  WRNWksLoopNode(BasicBlock *BB, LoopInfo *L);

protected:
  void setCollapse(int N) { Collapse = N; }
  void setOrdered(int N) { Ordered = N; }
  void setNowait(bool Flag) { Nowait = Flag; }
  void setLoopInfo(LoopInfo *L) { LI = L; }
  void setLoop(Loop *L) { Lp = L; }

public:
  DEFINE_GETTER(PrivateClause,      getPriv,     Priv)
  DEFINE_GETTER(FirstprivateClause, getFpriv,    Fpriv)
  DEFINE_GETTER(LastprivateClause,  getLpriv,    Lpriv)
  DEFINE_GETTER(ReductionClause,    getRed,      Reduction)
  DEFINE_GETTER(LinearClause,       getLinear,   Linear)
  DEFINE_GETTER(ScheduleClause,     getSchedule, Schedule)

  int getCollapse() const { return Collapse; }
  int getOrdered() const { return Ordered; }
  bool getNowait() const { return Nowait; }
  LoopInfo *getLoopInfo() const { return LI; }
  Loop *getLoop() const { return Lp; }

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
  bool Nowait;
  LoopInfo *LI;
  Loop *Lp;

public:
  WRNSectionsNode(BasicBlock *BB, LoopInfo *L);

protected:
  void setNowait(bool Flag) { Nowait = Flag; }
  void setLoopInfo(LoopInfo *L) { LI = L; }
  void setLoop(Loop *L) { Lp = L; }

public:
  DEFINE_GETTER(PrivateClause,      getPriv,   Priv)
  DEFINE_GETTER(FirstprivateClause, getFpriv,  Fpriv)
  DEFINE_GETTER(LastprivateClause,  getLpriv,  Lpriv)
  DEFINE_GETTER(ReductionClause,    getRed,    Reduction)

  bool getNowait() const { return Nowait; }
  LoopInfo *getLoopInfo() const { return LI; }
  Loop *getLoop() const { return Lp; }

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
  ScheduleClause DistSchedule;
  int Collapse;
  LoopInfo *LI;
  Loop *Lp;

public:
  WRNDistributeNode(BasicBlock *BB, LoopInfo *L);

protected:
  void setCollapse(int N) { Collapse = N; }
  void setLoopInfo(LoopInfo *L) { LI = L; }
  void setLoop(Loop *L) { Lp = L; }

public:
  DEFINE_GETTER(PrivateClause,      getPriv,         Priv)
  DEFINE_GETTER(FirstprivateClause, getFpriv,        Fpriv)
  DEFINE_GETTER(LastprivateClause,  getLpriv,        Lpriv)
  DEFINE_GETTER(ScheduleClause,     getDistSchedule, DistSchedule)

  int getCollapse() const { return Collapse; }
  LoopInfo *getLoopInfo() const { return LI; }
  Loop *getLoop() const { return Lp; }

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

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                             bool Verbose=false) const;

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
  void setCancelKind(WRNCancelKind CK) { CancelKind = CK; }
  void setIf(EXPR E) { IfExpr = E; }

public:
  bool getIsCancellationPoint() const { return IsCancellationPoint; }
  WRNCancelKind getCancelKind() const { return CancelKind; }
  EXPR getIf() const { return IfExpr; }

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
  DepSinkClause DepSink;  // meaningful only when DepSource==false
  void assertDoacrossTrue() const  { assert (IsDoacross &&
                              "This WRNOrdered represents Doacross"); }
  void assertDoacrossFalse() const { assert (!IsDoacross &&
                              "This WRNOrdered does not represent Doacross"); }

public:
  WRNOrderedNode(BasicBlock *BB);

protected:
  void setIsDoacross(bool Flag) { IsDoacross = Flag; }
  void setIsThreads(bool Flag) { assertDoacrossFalse(); IsThreads = Flag; }
  void setIsDepSource(bool Flag) { assertDoacrossTrue(); IsDepSource = Flag; }

public:
  bool getIsDoacross() const {  return IsDoacross; }
  bool getIsThreads() const { assertDoacrossFalse(); return IsThreads; }
  bool getIsDepSource() const { assertDoacrossTrue(); return IsDepSource; }

  const DepSinkClause &getDepSink() const {assertDoacrossTrue(); 
                                           return DepSink; }
  DepSinkClause &getDepSink() { assertDoacrossTrue(); return DepSink; }

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
  CopyprivateClause Cpriv;
  bool Nowait;

public:
  WRNSingleNode(BasicBlock *BB);

protected:
  void setNowait(bool Flag) { Nowait = Flag; }

public:
  DEFINE_GETTER(PrivateClause,      getPriv,  Priv)
  DEFINE_GETTER(FirstprivateClause, getFpriv, Fpriv)
  DEFINE_GETTER(CopyprivateClause,  getCpriv, Cpriv)

  bool getNowait() const { return Nowait; }

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

  StringRef getUserLockName() const { return UserLockName.str(); }

  void printExtra(formatted_raw_ostream &OS, unsigned Depth, 
                                             bool Verbose=false) const;

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

} // End namespace vpo

} // End namespace llvm

#endif
