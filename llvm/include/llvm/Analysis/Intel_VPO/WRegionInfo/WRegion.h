#if INTEL_COLLAB // -*- C++ -*-
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
///   WRNTaskNode             | #pragma omp task
///   WRNTaskloopNode         | #pragma omp taskloop
///   WRNVecLoopNode          | #pragma omp simd
///   WRNWksLoopNode          | #pragma omp for
///   WRNSectionsNode         | #pragma omp sections
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
///   WRNOrderedNode          | #pragma omp ordered
///   WRNMasterNode           | #pragma omp master
///   WRNSingleNode           | #pragma omp single
///   WRNTaskgroupNode        | #pragma omp taskgroup
///   WRNTaskwaitNode         | #pragma omp taskwait
///   WRNTaskyieldNode        | #pragma omp taskyield
///
/// One exception is WRNTaskloopNode, which is derived from WRNTasknode.
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_WREGION_H
#define LLVM_ANALYSIS_VPO_WREGION_H

#if INTEL_CUSTOMIZATION
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLNode.h"
#endif //INTEL_CUSTOMIZATION
#include "llvm/Analysis/Intel_VPO/Utils/VPOAnalysisUtils.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionNode.h"


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
   const CLAUSETYPE &GETTER() const { return CLAUSEOBJ; }  \
         CLAUSETYPE &GETTER()       { return CLAUSEOBJ; }

/// Loop information associated with loop-type constructs
class WRNLoopInfo {
private:
  LoopInfo   *LI;
  Loop       *Lp;
  Value      *NormIV; // normalized iv created by FE
  Value      *NormUB; // normalized ub (currently for loops from SECTIONS only)
  BasicBlock *ZTTBB;  // bblock with the zero-trip test
public:
  WRNLoopInfo(LoopInfo *L) : LI(L), Lp(nullptr), NormIV(nullptr),
                             NormUB(nullptr), ZTTBB(nullptr){}
  void setLoopInfo(LoopInfo *L) { LI = L; }
  void setLoop(Loop *L) { Lp = L; }
  void setNormIV(Value *IV) { NormIV = IV; }
  void setNormUB(Value *UB) { NormUB = UB; }
  void setZTTBB(BasicBlock *BB) { ZTTBB = BB; }
  LoopInfo *getLoopInfo() const { return LI; }
  Loop *getLoop() const { return Lp; }
  Value *getNormIV() const { return NormIV; }
  Value *getNormUB() const { return NormUB; }
  BasicBlock *getZTTBB() const { return ZTTBB; }
  void print(formatted_raw_ostream &OS, unsigned Depth,
             unsigned Verbosity=1) const;
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
  CopyinClause Copyin;
  EXPR IfExpr;
  EXPR NumThreads;
  WRNDefaultKind Default;
  WRNProcBindKind ProcBind;
  SmallVector<Instruction *, 2> CancellationPoints;
  SmallVector<AllocaInst *, 2> CancellationPointAllocas;

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
  const SmallVectorImpl<Instruction *> &getCancellationPoints() const {
    return CancellationPoints;
  }
  void addCancellationPoint(Instruction *I) { CancellationPoints.push_back(I); }
  const SmallVectorImpl<AllocaInst *> &getCancellationPointAllocas() const {
    return CancellationPointAllocas;
  }
  void addCancellationPointAlloca(AllocaInst *I) {
    CancellationPointAllocas.push_back(I);
  }

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                             unsigned Verbosity=1) const;

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
  WRNLoopInfo WRNLI;
  SmallVector<Instruction *, 2> CancellationPoints;
  SmallVector<AllocaInst *, 2> CancellationPointAllocas;

public:
  WRNParallelLoopNode(BasicBlock *BB, LoopInfo *L);

protected:
  void setIf(EXPR E) { IfExpr = E; }
  void setNumThreads(EXPR E) { NumThreads = E; }
  void setDefault(WRNDefaultKind D) { Default = D; }
  void setProcBind(WRNProcBindKind P) { ProcBind = P; }
  void setCollapse(int N) { Collapse = N; }
  void setOrdered(int N) { Ordered = N; }

public:
  DEFINE_GETTER(SharedClause,       getShared, Shared)
  DEFINE_GETTER(PrivateClause,      getPriv,   Priv)
  DEFINE_GETTER(FirstprivateClause, getFpriv,  Fpriv)
  DEFINE_GETTER(LastprivateClause,  getLpriv,  Lpriv)
  DEFINE_GETTER(ReductionClause,    getRed,    Reduction)
  DEFINE_GETTER(LinearClause,       getLinear, Linear)
  DEFINE_GETTER(CopyinClause,       getCopyin, Copyin)
  DEFINE_GETTER(ScheduleClause,     getSchedule, Schedule)
  DEFINE_GETTER(WRNLoopInfo,        getWRNLoopInfo, WRNLI)

  EXPR getIf() const { return IfExpr; }
  EXPR getNumThreads() const { return NumThreads; }
  WRNDefaultKind getDefault() const { return Default; }
  WRNProcBindKind getProcBind() const { return ProcBind; }
  int getCollapse() const { return Collapse; }
  int getOrdered() const { return Ordered; }
  const SmallVectorImpl<Instruction *> &getCancellationPoints() const {
    return CancellationPoints;
  }
  void addCancellationPoint(Instruction *I) { CancellationPoints.push_back(I); }
  const SmallVectorImpl<AllocaInst *> &getCancellationPointAllocas() const {
    return CancellationPointAllocas;
  }
  void addCancellationPointAlloca(AllocaInst *I) {
    CancellationPointAllocas.push_back(I);
  }

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                             unsigned Verbosity=1) const;

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
  CopyinClause Copyin;
  EXPR IfExpr;
  EXPR NumThreads;
  WRNDefaultKind Default;
  WRNProcBindKind ProcBind;
  WRNLoopInfo WRNLI;
  SmallVector<Instruction *, 2> CancellationPoints;
  SmallVector<AllocaInst *, 2> CancellationPointAllocas;

public:
  WRNParallelSectionsNode(BasicBlock *BB, LoopInfo *L);

protected:
  void setIf(EXPR E) { IfExpr = E; }
  void setNumThreads(EXPR E) { NumThreads = E; }
  void setDefault(WRNDefaultKind D) { Default = D; }
  void setProcBind(WRNProcBindKind P) { ProcBind = P; }

public:
  DEFINE_GETTER(SharedClause,       getShared, Shared)
  DEFINE_GETTER(PrivateClause,      getPriv,   Priv)
  DEFINE_GETTER(FirstprivateClause, getFpriv,  Fpriv)
  DEFINE_GETTER(LastprivateClause,  getLpriv,  Lpriv)
  DEFINE_GETTER(ReductionClause,    getRed,    Reduction)
  DEFINE_GETTER(CopyinClause,       getCopyin, Copyin)
  DEFINE_GETTER(WRNLoopInfo,        getWRNLoopInfo, WRNLI)

  EXPR getIf() const { return IfExpr; }
  EXPR getNumThreads() const { return NumThreads; }
  WRNDefaultKind getDefault() const { return Default; }
  WRNProcBindKind getProcBind() const { return ProcBind; }
  const SmallVectorImpl<Instruction *> &getCancellationPoints() const {
    return CancellationPoints;
  }
  void addCancellationPoint(Instruction *I) { CancellationPoints.push_back(I); }
  const SmallVectorImpl<AllocaInst *> &getCancellationPointAllocas() const {
    return CancellationPointAllocas;
  }
  void addCancellationPointAlloca(AllocaInst *I) {
    CancellationPointAllocas.push_back(I);
  }

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                             unsigned Verbosity=1) const;

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
  ScheduleClause Schedule;
  EXPR IfExpr;
  EXPR NumThreads;
  WRNDefaultKind Default;
  WRNProcBindKind ProcBind;
  WRNLoopInfo WRNLI;

public:
  WRNParallelWorkshareNode(BasicBlock *BB, LoopInfo *L);

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
  DEFINE_GETTER(ScheduleClause,     getSchedule, Schedule)
  DEFINE_GETTER(WRNLoopInfo,        getWRNLoopInfo, WRNLI)

  EXPR getIf() const { return IfExpr; }
  EXPR getNumThreads() const { return NumThreads; }
  WRNDefaultKind getDefault() const { return Default; }
  WRNProcBindKind getProcBind() const { return ProcBind; }

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                             unsigned Verbosity=1) const;

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

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                             unsigned Verbosity=1) const;

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
  WRNLoopInfo WRNLI;

public:
  WRNDistributeParLoopNode(BasicBlock *BB, LoopInfo *L);

protected:
  void setIf(EXPR E) { IfExpr = E; }
  void setNumThreads(EXPR E) { NumThreads = E; }
  void setDefault(WRNDefaultKind D) { Default = D; }
  void setProcBind(WRNProcBindKind P) { ProcBind = P; }
  void setCollapse(int N) { Collapse = N; }
  void setOrdered(int N) { Ordered = N; }

public:
  DEFINE_GETTER(SharedClause,       getShared, Shared)
  DEFINE_GETTER(PrivateClause,      getPriv,   Priv)
  DEFINE_GETTER(FirstprivateClause, getFpriv,  Fpriv)
  DEFINE_GETTER(LastprivateClause,  getLpriv,  Lpriv)
  DEFINE_GETTER(ReductionClause,    getRed,    Reduction)
  DEFINE_GETTER(LinearClause,       getLinear, Linear)
  DEFINE_GETTER(CopyinClause,       getCopyin, Copyin)
  DEFINE_GETTER(ScheduleClause,     getSchedule, Schedule)
  DEFINE_GETTER(ScheduleClause,     getDistSchedule, DistSchedule)
  DEFINE_GETTER(WRNLoopInfo,        getWRNLoopInfo, WRNLI)

  EXPR getIf() const { return IfExpr; }
  EXPR getNumThreads() const { return NumThreads; }
  WRNDefaultKind getDefault() const { return Default; }
  WRNProcBindKind getProcBind() const { return ProcBind; }
  int getCollapse() const { return Collapse; }
  int getOrdered() const { return Ordered; }

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                             unsigned Verbosity=1) const;

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

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                             unsigned Verbosity=1) const;

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

public:
  WRNTargetDataNode(BasicBlock *BB);

protected:
  void setIf(EXPR E) { IfExpr = E; }
  void setDevice(EXPR E) { Device = E; }

public:
  DEFINE_GETTER(MapClause,          getMap,          Map)
  DEFINE_GETTER(UseDevicePtrClause, getUseDevicePtr, UseDevicePtr)

  EXPR getIf() const { return IfExpr; }
  EXPR getDevice() const { return Device; }

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                             unsigned Verbosity=1) const;

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
  DependClause Depend;
  EXPR IfExpr;
  EXPR Device;
  bool Nowait;

public:
  WRNTargetEnterDataNode(BasicBlock *BB);

protected:
  void setIf(EXPR E) { IfExpr = E; }
  void setDevice(EXPR E) { Device = E; }
  void setNowait(bool Flag) { Nowait = Flag; }

public:
  DEFINE_GETTER(MapClause,          getMap,          Map)
  DEFINE_GETTER(DependClause,       getDepend,       Depend)

  EXPR getIf() const { return IfExpr; }
  EXPR getDevice() const { return Device; }
  bool getNowait() const { return Nowait; }

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                             unsigned Verbosity=1) const;

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
  DependClause Depend;
  EXPR IfExpr;
  EXPR Device;
  bool Nowait;

public:
  WRNTargetExitDataNode(BasicBlock *BB);

protected:
  void setIf(EXPR E) { IfExpr = E; }
  void setDevice(EXPR E) { Device = E; }
  void setNowait(bool Flag) { Nowait = Flag; }

public:
  DEFINE_GETTER(MapClause,          getMap,          Map)
  DEFINE_GETTER(DependClause,       getDepend,       Depend)

  EXPR getIf() const { return IfExpr; }
  EXPR getDevice() const { return Device; }
  bool getNowait() const { return Nowait; }

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                             unsigned Verbosity=1) const;

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
  DependClause Depend;
  EXPR IfExpr;
  EXPR Device;
  bool Nowait;

public:
  WRNTargetUpdateNode(BasicBlock *BB);

protected:
  void setIf(EXPR E) { IfExpr = E; }
  void setDevice(EXPR E) { Device = E; }
  void setNowait(bool Flag) { Nowait = Flag; }

public:
  DEFINE_GETTER(MapClause,          getMap,          Map)
  DEFINE_GETTER(DependClause,       getDepend,       Depend)

  EXPR getIf() const { return IfExpr; }
  EXPR getDevice() const { return Device; }
  bool getNowait() const { return Nowait; }

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                             unsigned Verbosity=1) const;

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNTargetUpdate;
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
  ReductionClause InReduction;
  DependClause Depend;
  EXPR Final;
  EXPR IfExpr;
  EXPR Priority;
  WRNDefaultKind Default;
  bool Untied;
  bool Mergeable;
  unsigned TaskFlag; // flag bit vector used to invoke tasking RTL
  SmallVector<Instruction *, 2> CancellationPoints;
  SmallVector<AllocaInst *, 2> CancellationPointAllocas;

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
  DEFINE_GETTER(ReductionClause,    getInRed,  InReduction)
  DEFINE_GETTER(DependClause,       getDepend, Depend)

  EXPR getFinal() const { return Final; }
  EXPR getIf() const { return IfExpr; }
  EXPR getPriority() const { return Priority; }
  WRNDefaultKind getDefault() const { return Default; }
  bool getUntied() const { return Untied; }
  bool getMergeable() const { return Mergeable; }
  unsigned getTaskFlag() const { return TaskFlag; }
  const SmallVectorImpl<Instruction *> &getCancellationPoints() const {
    return CancellationPoints;
  }
  void addCancellationPoint(Instruction *I) { CancellationPoints.push_back(I); }
  const SmallVectorImpl<AllocaInst *> &getCancellationPointAllocas() const {
    return CancellationPointAllocas;
  }
  void addCancellationPointAlloca(AllocaInst *I) {
    CancellationPointAllocas.push_back(I);
  }

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                             unsigned Verbosity=1) const;

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
  DEFINE_GETTER(ReductionClause,    getRed,    Reduction)
  DEFINE_GETTER(WRNLoopInfo,        getWRNLoopInfo, WRNLI)
  EXPR getGrainsize() const { return Grainsize; }
  EXPR getNumTasks() const { return NumTasks; }
  int getSchedCode() const { return SchedCode; }
  int getCollapse() const { return Collapse; }
  bool getNogroup() const { return Nogroup; }

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
  WRNLoopInfo WRNLI;
#if INTEL_CUSTOMIZATION
  bool IsAutoVec;
  bool IgnoreProfitability;
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

  void setSimdlen(int N) { Simdlen = N; }
  void setSafelen(int N) { Safelen = N; }
  void setCollapse(int N) { Collapse = N; }
#if INTEL_CUSTOMIZATION
  void setIsAutoVec(bool Flag) { IsAutoVec = Flag; }
  void setIgnoreProfitability(bool Flag) { IgnoreProfitability = Flag; }
  void setEntryHLNode(loopopt::HLNode *E) { EntryHLNode = E; }
  void setExitHLNode(loopopt::HLNode *X) { ExitHLNode = X; }
  void setHLLoop(loopopt::HLLoop *L) { HLp = L; }
#endif //INTEL_CUSTOMIZATION

  DEFINE_GETTER(PrivateClause,     getPriv,    Priv)
  DEFINE_GETTER(LastprivateClause, getLpriv,   Lpriv)
  DEFINE_GETTER(ReductionClause,   getRed,     Reduction)
  DEFINE_GETTER(LinearClause,      getLinear,  Linear)
  DEFINE_GETTER(AlignedClause,     getAligned, Aligned)
  DEFINE_GETTER(UniformClause,     getUniform, Uniform)
  DEFINE_GETTER(WRNLoopInfo,       getWRNLoopInfo, WRNLI)

  int getSimdlen() const { return Simdlen; }
  int getSafelen() const { return Safelen; }
  int getCollapse() const { return Collapse; }
#if INTEL_CUSTOMIZATION
  bool getIsAutoVec() const { return IsAutoVec; }
  bool getIgnoreProfitability() const { return IgnoreProfitability; }
  loopopt::HLNode *getEntryHLNode() const { return EntryHLNode; }
  loopopt::HLNode *getExitHLNode() const { return ExitHLNode; }
  loopopt::HLLoop *getHLLoop() const { return HLp; }
#endif //INTEL_CUSTOMIZATION

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                             unsigned Verbosity=1) const;

#if INTEL_CUSTOMIZATION
  void printHIR(formatted_raw_ostream &OS, unsigned Depth,
                                           unsigned Verbosity=1) const;
#endif //INTEL_CUSTOMIZATION

  template <class LoopType> LoopType *getTheLoop() const {
    llvm_unreachable("Unsupported LoopType");
  }

  /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
  static bool classof(const WRegionNode *W) {
    return W->getWRegionKindID() == WRegionNode::WRNVecLoop;
  }
};

template <> Loop *WRNVecLoopNode::getTheLoop<Loop>() const;
#if INTEL_CUSTOMIZATION
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
  ReductionClause Reduction;
  LinearClause Linear;
  ScheduleClause Schedule;
  int Collapse;
  int Ordered;
  bool Nowait;
  WRNLoopInfo WRNLI;
  SmallVector<Instruction *, 2> CancellationPoints;
  SmallVector<AllocaInst *, 2> CancellationPointAllocas;

public:
  WRNWksLoopNode(BasicBlock *BB, LoopInfo *L);

protected:
  void setCollapse(int N) { Collapse = N; }
  void setOrdered(int N) { Ordered = N; }
  void setNowait(bool Flag) { Nowait = Flag; }

public:
  DEFINE_GETTER(PrivateClause,      getPriv,     Priv)
  DEFINE_GETTER(FirstprivateClause, getFpriv,    Fpriv)
  DEFINE_GETTER(LastprivateClause,  getLpriv,    Lpriv)
  DEFINE_GETTER(ReductionClause,    getRed,      Reduction)
  DEFINE_GETTER(LinearClause,       getLinear,   Linear)
  DEFINE_GETTER(ScheduleClause,     getSchedule, Schedule)
  DEFINE_GETTER(WRNLoopInfo,        getWRNLoopInfo, WRNLI)

  int getCollapse() const { return Collapse; }
  int getOrdered() const { return Ordered; }
  bool getNowait() const { return Nowait; }
  const SmallVectorImpl<Instruction *> &getCancellationPoints() const {
    return CancellationPoints;
  }
  void addCancellationPoint(Instruction *I) { CancellationPoints.push_back(I); }
  const SmallVectorImpl<AllocaInst *> &getCancellationPointAllocas() const {
    return CancellationPointAllocas;
  }
  void addCancellationPointAlloca(AllocaInst *I) {
    CancellationPointAllocas.push_back(I);
  }

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                             unsigned Verbosity=1) const;

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
  ScheduleClause Schedule;
  bool Nowait;
  WRNLoopInfo WRNLI;
  SmallVector<Instruction *, 2> CancellationPoints;
  SmallVector<AllocaInst *, 2> CancellationPointAllocas;

public:
  WRNSectionsNode(BasicBlock *BB, LoopInfo *L);

protected:
  void setNowait(bool Flag) { Nowait = Flag; }

public:
  DEFINE_GETTER(PrivateClause,      getPriv,   Priv)
  DEFINE_GETTER(FirstprivateClause, getFpriv,  Fpriv)
  DEFINE_GETTER(LastprivateClause,  getLpriv,  Lpriv)
  DEFINE_GETTER(ReductionClause,    getRed,    Reduction)
  DEFINE_GETTER(ScheduleClause,     getSchedule, Schedule)
  DEFINE_GETTER(WRNLoopInfo,        getWRNLoopInfo, WRNLI)

  bool getNowait() const { return Nowait; }
  const SmallVectorImpl<Instruction *> &getCancellationPoints() const {
    return CancellationPoints;
  }
  void addCancellationPoint(Instruction *I) { CancellationPoints.push_back(I); }
  const SmallVectorImpl<AllocaInst *> &getCancellationPointAllocas() const {
    return CancellationPointAllocas;
  }
  void addCancellationPointAlloca(AllocaInst *I) {
    CancellationPointAllocas.push_back(I);
  }

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                             unsigned Verbosity=1) const;

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
  bool Nowait;
  WRNLoopInfo WRNLI;

public:
  WRNWorkshareNode(BasicBlock *BB, LoopInfo *L);

protected:
  void setNowait(bool Flag) { Nowait = Flag; }

public:
  DEFINE_GETTER(WRNLoopInfo, getWRNLoopInfo, WRNLI)
  bool getNowait() const { return Nowait; }

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                             unsigned Verbosity=1) const;

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
  WRNLoopInfo WRNLI;

public:
  WRNDistributeNode(BasicBlock *BB, LoopInfo *L);

protected:
  void setCollapse(int N) { Collapse = N; }

public:
  DEFINE_GETTER(PrivateClause,      getPriv,         Priv)
  DEFINE_GETTER(FirstprivateClause, getFpriv,        Fpriv)
  DEFINE_GETTER(LastprivateClause,  getLpriv,        Lpriv)
  DEFINE_GETTER(ScheduleClause,     getDistSchedule, DistSchedule)
  DEFINE_GETTER(WRNLoopInfo,        getWRNLoopInfo,  WRNLI)

  int getCollapse() const { return Collapse; }

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                             unsigned Verbosity=1) const;

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
                                             unsigned Verbosity=1) const;

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

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                             unsigned Verbosity=1) const;

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

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                             unsigned Verbosity=1) const;

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

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                             unsigned Verbosity=1) const;

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
  // TODO: Add HINT

protected:
  void setUserLockName(StringRef LN) { UserLockName = LN; }

public:
  WRNCriticalNode(BasicBlock *BB);

  StringRef getUserLockName() const { return UserLockName.str(); }

  void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                             unsigned Verbosity=1) const;

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

public:
  WRNTaskgroupNode(BasicBlock *BB);
  DEFINE_GETTER(ReductionClause,    getRed,    Reduction)

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


/// \brief Print the fields common to WRNs for which getIsPar()==true.
/// Possible constructs are: WRNParallel, WRNParallelLoop,
///                          WRNParallelSections, WRNParallelWorkshare,
/// The fields to print are: IfExpr, NumThreads, Default, ProcBind
extern void printExtraForParallel(WRegionNode const *W,
                                  formatted_raw_ostream &OS, int Depth,
                                  unsigned Verbosity=1);

/// \brief Print the fields common to some WRNs for which getIsOmpLoop()==true.
/// Possible constructs are: WRNParallelLoop, WRNDistributeParLoop, WRNWksLoop
/// The fields to print are: Collapse, Ordered, Nowait
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
