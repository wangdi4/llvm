#if INTEL_COLLAB // -*- C++ -*-
//===---------- WRegionNode.h - WRegion Graph Node --------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file defines the WRegion Graph node.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_WREGIONNODE_H
#define LLVM_ANALYSIS_VPO_WREGIONNODE_H

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"

#include "llvm/IR/Dominators.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"

#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/BasicBlock.h"

#include "llvm/Transforms/Utils/GeneralUtils.h"
#include "llvm/Analysis/VPO/Utils/VPOAnalysisUtils.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionClause.h"

#include <set>
#include <unordered_map>

namespace llvm {
#if INTEL_CUSTOMIZATION

namespace loopopt {
class HLNode;
class HLInst;
class HLLoop;
}
#endif //INTEL_CUSTOMIZATION

namespace vpo {

extern std::unordered_map<int, StringRef> WRNName;

typedef VPOSmallVectorBB WRegionBBSetTy;

class WRegionNode;
typedef SmallVector<WRegionNode *, 4>  WRContainerTy;
typedef SmallVectorImpl<WRegionNode *> WRContainerImpl;
typedef WRContainerTy::iterator               wrn_iterator;
typedef WRContainerTy::const_iterator         wrn_const_iterator;
typedef WRContainerTy::reverse_iterator       wrn_reverse_iterator;
typedef WRContainerTy::const_reverse_iterator wrn_const_reverse_iterator;

class WRNLoopInfo;  // WRegion.h

/// WRegion Node base class
class WRegionNode {

public:

  /// Iterators to iterator over basic block set
  typedef WRegionBBSetTy::iterator bbset_iterator;
  typedef WRegionBBSetTy::const_iterator bbset_const_iterator;
  typedef WRegionBBSetTy::reverse_iterator bbset_reverse_iterator;
  typedef WRegionBBSetTy::const_reverse_iterator bbset_const_reverse_iterator;

private:
  /// Make class uncopyable.
  void operator=(const WRegionNode &) = delete;

  /// Unique number associated with this WRegionNode.
  unsigned Number;

  /// Nesting level of the WRN node in the WRN graph. Outermost level is 0.
  unsigned Level;

  /// ID to differentitate between concrete subclasses.
  unsigned SubClassID;

  /// The OMP_DIRECTIVES enum representing the OMP construct. This is useful
  /// for opt reporting, which can't use SubClassID because multiple
  /// OMP_DIRECTIVES may map to the same SubClassID. For example,
  ///   DIR_OMP_TARGET_DATA, DIR_OMP_TARGET_ENTER_DATA, and
  ///   DIR_OMP_TARGET_EXIT_DATA
  /// all map to WRNTargetDataNode
  int DirID;

  /// Bit vector for attributes such as WRNIsParLoop or WRNIsTask.
  /// The enum WRNAttributes lists the attributes.
  uint32_t Attributes;

  /// Entry and Exit BBs of this WRN
  BasicBlock    *EntryBBlock;
  BasicBlock    *ExitBBlock;
  Instruction   *EntryDirective;
  Instruction   *ExitDirective;

  /// Set containing all the BBs in this WRN.
  /// If BBlockSet is not empty, it must be valid. Therefore, any
  /// transformation that invalidates the BBlockSet must clear it
  /// so subsequent transformations can recompute the BBlockSet if needed.
  WRegionBBSetTy BBlockSet;

  /// Enclosing parent of WRegionNode in CFG.
  WRegionNode *Parent;

  /// Children container
  WRContainerTy Children;

  /// Counter used for assigning unique numbers to WRegionNodes.
  static unsigned UniqueNum;

  /// Sets the unique number associated with this WRegionNode.
  void setNextNumber() { Number = ++UniqueNum; }

  /// True for regions that may potentially "invoke" "omp critical"
  /// (either explicitly or down the call stack).
  bool MayHaveOMPCritical = false;

#if INTEL_CUSTOMIZATION
  /// True if the WRN came from HIR; false otherwise
  bool IsFromHIR;

  /// Sets the flag to indicate if WRN came from HIR
  void setIsFromHIR(bool flag) { IsFromHIR = flag; }

  /// Used only while parsing clauses. Contains the list of RegDDRefs for the
  /// operand bundle being currently parsed.
  SmallVector<loopopt::RegDDRef*, 4> CurrentBundleDDRefs;
#endif // INTEL_CUSTOMIZATION

  /// Destroys all objects of this class. Should only be
  /// called after code gen.
  static void destroyAll();

protected:

  /// constructors
  WRegionNode(unsigned SCID, BasicBlock *BB); // for LLVM IR
#if INTEL_CUSTOMIZATION
  WRegionNode(unsigned SCID);                 // for HIR only
#endif // INTEL_CUSTOMIZATION
  WRegionNode(WRegionNode *W);                // for both

  // copy constructor not needed (at least for now)
  // WRegionNode(const WRegionNode &WRegionNodeObj);

  /// Destroys the object.
  void destroy();

  /// Sets the nesting level of this WRegionNode.
  void setLevel(int K) { Level = K; }

  /// Sets the entry(first) bblock of this region.
  void setEntryBBlock(BasicBlock *EntryBB) { EntryBBlock = EntryBB; }

  /// Sets the exit(last) bblock of this region.
  void setExitBBlock(BasicBlock *ExitBB) { ExitBBlock = ExitBB; }

  /// Sets the entry(first) Directive of this region.
  void setEntryDirective(Instruction *EntryDir) { EntryDirective = EntryDir; }

  /// Sets the exit(last) Directive of this region.
  void setExitDirective(Instruction *ExitDir) { ExitDirective = ExitDir; }

  /// Sets the graph parent of this WRegionNode.
  void setParent(WRegionNode *P) { Parent = P; }

  /// Finish creating the WRN once its ExitDir is found. This routine calls
  /// setExitDirective(ExitDir) and setExitBBlock(ExitDir->getParent()). In
  /// addition, if the WRN is a loop construct, this routine also calls
  /// GeneralUtils::getLoopFromLoopInfo to find the Loop from LoopInfo.
  void finalize(Instruction *ExitDir, DominatorTree *DT);

  //
  // Routines for parsing clauses
  //

  /// Top-level code to parse a clause
  void parseClause(const ClauseSpecifier &ClauseInfo, const Use *Args,
                   unsigned NumArgs);

  /// Update WRN for clauses with no operands.
  void handleQual(int ClauseID);

  /// Update WRN for clauses with one operand.
  void handleQualOpnd(int ClauseID, Value *V);

  /// Update WRN for clauses with operand list.
  void handleQualOpndList(const Use *Args, unsigned NumArgs,
                          const ClauseSpecifier &ClauseInfo);

  /// Update WRN for clauses from the OperandBundles under the
  /// directive.region.entry/exit representation
  void getClausesFromOperandBundles();
#if INTEL_CUSTOMIZATION
  void getClausesFromOperandBundles(IntrinsicInst *Call,
                                    loopopt::HLInst *H = nullptr);
#else
  void getClausesFromOperandBundles(IntrinsicInst *Call);
#endif // INTEL_CUSTOMIZATION

public:
  /// \name Functions to check if the WRN allows a given clause type.
  /// @{
  bool canHaveSchedule() const;
  bool canHaveDistSchedule() const;
  bool canHaveShared() const;
  bool canHavePrivate() const;
  bool canHaveFirstprivate() const;
  bool canHaveLastprivate() const;
  bool canHaveInReduction() const;
  bool canHaveReduction() const;
  bool canHaveCopyin() const;
  bool canHaveCopyprivate() const;
  bool canHaveLinear() const;
  bool canHaveUniform() const;
  bool canHaveMap() const;
  bool canHaveIsDevicePtr() const;
  bool canHaveUseDevicePtr() const;
  bool canHaveDepend() const;
  bool canHaveDepSrcSink() const;
  bool canHaveAligned() const;
  bool canHaveFlush() const;
  bool canHaveCancellationPoints() const; ///< Constructs that can be cancelled
  /// @}

  /// Returns `true` if the construct needs to be outlined into a separate
  /// function which is accepted by the OpenMP runtime.
  bool needsOutlining() const;

  // Below are virtual functions to get/set clause and other information of
  // the WRN. They should never be called; calling them indicates intention
  // to access clause info for a WRN that does not allow such clause (eg, a
  // parallel construct does not take a collapse clause). These virtual
  // functions defined in the base class will all emit an error message.

  void errorClause(StringRef ClauseName) const;
  void errorClause(int ClauseID) const;

  // Define a macro to call errorClause() followed by llvm_unreachable().
  // This way, the virtual getter functions don't need a return.
  #define WRNERROR(x) errorClause(x); llvm_unreachable("Bad Clause");

  // list-type clauses (getters only; no setters)

  virtual AlignedClause &getAligned()        {WRNERROR(QUAL_OMP_ALIGNED);     }
  virtual CopyinClause &getCopyin()          {WRNERROR(QUAL_OMP_COPYIN);      }
  virtual CopyprivateClause &getCpriv()      {WRNERROR(QUAL_OMP_COPYPRIVATE); }
  virtual DependClause &getDepend()          {WRNERROR("DEPEND");             }
  virtual DepSinkClause &getDepSink()        {WRNERROR("DEPEND(SINK:..)");    }
  virtual DepSourceClause &getDepSource()    {WRNERROR("DEPEND(SOURCE)");     }
  virtual FirstprivateClause &getFpriv()     {WRNERROR(QUAL_OMP_FIRSTPRIVATE);}
  virtual FlushSet &getFlush()               {WRNERROR(QUAL_OMP_FLUSH);       }
  virtual IsDevicePtrClause &getIsDevicePtr()
                                            {WRNERROR(QUAL_OMP_IS_DEVICE_PTR);}
  virtual LastprivateClause &getLpriv()      {WRNERROR(QUAL_OMP_LASTPRIVATE); }
  virtual LinearClause &getLinear()          {WRNERROR(QUAL_OMP_LINEAR);      }
  virtual MapClause &getMap()                {WRNERROR("MAP");                }
  virtual PrivateClause &getPriv()           {WRNERROR(QUAL_OMP_PRIVATE);     }
  virtual ReductionClause &getInRed()        {WRNERROR("IN_REDUCTION");       }
  virtual ReductionClause &getRed()          {WRNERROR("REDUCTION");          }
  virtual ScheduleClause &getSchedule()      {WRNERROR("SCHEDULE");           }
       // ScheduleClause is not list-type, but has similar API so put here too
  virtual ScheduleClause &getDistSchedule()   {WRNERROR("DIST_SCHEDULE");     }

  virtual SharedClause &getShared()          {WRNERROR(QUAL_OMP_SHARED);      }
  virtual UniformClause &getUniform()        {WRNERROR(QUAL_OMP_UNIFORM);     }
  virtual UseDevicePtrClause &getUseDevicePtr()
                                           {WRNERROR(QUAL_OMP_USE_DEVICE_PTR);}

  // list-type clauses (const getters)

  virtual const AlignedClause &getAligned() const
                                           {WRNERROR(QUAL_OMP_ALIGNED);     }
  virtual const CopyinClause &getCopyin() const
                                           {WRNERROR(QUAL_OMP_COPYIN);      }
  virtual const CopyprivateClause &getCpriv() const
                                           {WRNERROR(QUAL_OMP_COPYPRIVATE); }
  virtual const DependClause &getDepend() const
                                           {WRNERROR("DEPEND");             }
  virtual const DepSinkClause &getDepSink() const
                                           {WRNERROR("DEPEND(SINK:..)");    }
  virtual const DepSourceClause &getDepSource() const
                                           {WRNERROR("DEPEND(SOURCE)");     }
  virtual const FirstprivateClause &getFpriv() const
                                           {WRNERROR(QUAL_OMP_FIRSTPRIVATE);}
  virtual const FlushSet &getFlush() const {WRNERROR(QUAL_OMP_FLUSH);       }
  virtual const IsDevicePtrClause &getIsDevicePtr() const
                                           {WRNERROR(QUAL_OMP_IS_DEVICE_PTR);}
  virtual const LastprivateClause &getLpriv() const
                                           {WRNERROR(QUAL_OMP_LASTPRIVATE); }
  virtual const LinearClause &getLinear() const
                                           {WRNERROR(QUAL_OMP_LINEAR);      }
  virtual const MapClause &getMap() const  {WRNERROR("MAP");                }
  virtual const PrivateClause &getPriv() const
                                           {WRNERROR(QUAL_OMP_PRIVATE);     }
  virtual const ReductionClause &getInRed() const
                                           {WRNERROR("IN_REDUCTION");       }
  virtual const ReductionClause &getRed() const
                                           {WRNERROR("REDUCTION");          }
  virtual const ScheduleClause &getSchedule() const
                                           {WRNERROR("SCHEDULE");           }
  virtual const ScheduleClause &getDistSchedule() const
                                           {WRNERROR("DIST_SCHEDULE");      }
  virtual const SharedClause &getShared() const
                                           {WRNERROR(QUAL_OMP_SHARED);      }
  virtual const UniformClause &getUniform() const
                                           {WRNERROR(QUAL_OMP_UNIFORM);     }
  virtual const UseDevicePtrClause &getUseDevicePtr() const
                                           {WRNERROR(QUAL_OMP_USE_DEVICE_PTR);}

  // other clauses (both getters and setters)

  virtual void setAtomicKind(WRNAtomicKind A)   {WRNERROR("ATOMIC_KIND");     }
  virtual WRNAtomicKind getAtomicKind()   const {WRNERROR("ATOMIC_KIND");     }
  virtual void setCancelKind(WRNCancelKind CK)  {WRNERROR("CANCEL TYPE");     }
  virtual WRNCancelKind getCancelKind()   const {WRNERROR("CANCEL TYPE");     }
  virtual void setCollapse(int N)               {WRNERROR(QUAL_OMP_COLLAPSE); }
  virtual int getCollapse()               const {WRNERROR(QUAL_OMP_COLLAPSE); }
  virtual void setDefault(WRNDefaultKind T)     {WRNERROR("DEFAULT");         }
  virtual WRNDefaultKind getDefault()     const {WRNERROR("DEFAULT");         }
  virtual void setDefaultmapTofromScalar(bool F) {WRNERROR("DEFAULTMAP");     }
  virtual bool getDefaultmapTofromScalar() const {WRNERROR("DEFAULTMAP");     }
  virtual void setDevice(EXPR E)                {WRNERROR(QUAL_OMP_DEVICE);   }
  virtual EXPR getDevice()                const {WRNERROR(QUAL_OMP_DEVICE);   }
  virtual void setFinal(EXPR E)                 {WRNERROR(QUAL_OMP_FINAL);    }
  virtual EXPR getFinal()                 const {WRNERROR(QUAL_OMP_FINAL);    }
  virtual void setGrainsize(EXPR E)             {WRNERROR(QUAL_OMP_GRAINSIZE);}
  virtual EXPR getGrainsize()             const {WRNERROR(QUAL_OMP_GRAINSIZE);}
  virtual void setHasSeqCstClause(bool SC)      {WRNERROR("SEQ_CST");         }
  virtual bool getHasSeqCstClause()       const {WRNERROR("SEQ_CST");         }
  virtual void setIf(EXPR E)                    {WRNERROR(QUAL_OMP_IF);       }
  virtual EXPR getIf()                    const {WRNERROR(QUAL_OMP_IF);       }
  virtual void setIsDoacross(bool F)         {WRNERROR("DEPEND(SOURCE|SINK)");}
  virtual bool getIsDoacross()         const {WRNERROR("DEPEND(SOURCE|SINK)");}
  virtual void setIsThreads(bool Flag)          {WRNERROR("THREADS/SIMD");    }
  virtual bool getIsThreads()             const {WRNERROR("THREADS/SIMD");    }
  virtual void setMergeable(bool Flag)          {WRNERROR(QUAL_OMP_MERGEABLE);}
  virtual bool getMergeable()             const {WRNERROR(QUAL_OMP_MERGEABLE);}
  virtual void setIsTargetTask(bool Flag)     {WRNERROR(QUAL_OMP_TARGET_TASK);}
  virtual bool getIsTargetTask()        const {WRNERROR(QUAL_OMP_TARGET_TASK);}
  virtual void setNogroup(bool Flag)            {WRNERROR(QUAL_OMP_NOGROUP);  }
  virtual bool getNogroup()               const {WRNERROR(QUAL_OMP_NOGROUP);  }
  virtual void setNowait(bool Flag)             {WRNERROR(QUAL_OMP_NOWAIT);   }
  virtual bool getNowait()                const {WRNERROR(QUAL_OMP_NOWAIT);   }
  virtual void setNumTasks(EXPR E)              {WRNERROR(QUAL_OMP_NUM_TASKS);}
  virtual EXPR getNumTasks()              const {WRNERROR(QUAL_OMP_NUM_TASKS);}
  virtual void setNumTeams(EXPR E)              {WRNERROR(QUAL_OMP_NUM_TEAMS);}
  virtual EXPR getNumTeams()              const {WRNERROR(QUAL_OMP_NUM_TEAMS);}
  virtual void setNumThreads(EXPR E)          {WRNERROR(QUAL_OMP_NUM_THREADS);}
  virtual EXPR getNumThreads()          const {WRNERROR(QUAL_OMP_NUM_THREADS);}
  virtual void setOffloadEntryIdx(int N)       {WRNERROR("OFFLOAD_ENTRY_IDX");}
  virtual int  getOffloadEntryIdx()      const {WRNERROR("OFFLOAD_ENTRY_IDX");}
  virtual void setOrdered(int N)                {WRNERROR(QUAL_OMP_ORDERED);  }
  virtual int getOrdered()                const {WRNERROR(QUAL_OMP_ORDERED);  }
  virtual void addOrderedTripCount(Value *TC)
                                            {WRNERROR("ORDERED_TRIP_COUNTS"); }
  virtual const SmallVectorImpl<Value *> &getOrderedTripCounts() const
                                            {WRNERROR("ORDERED_TRIP_COUNTS"); }
  virtual void setParLoopNdInfoAlloca(AllocaInst *AI) { WRNERROR("LOOP_DESC"); }
  virtual AllocaInst *getParLoopNdInfoAlloca() const { WRNERROR("LOOP_DESC"); }
  virtual void setPriority(EXPR E)              {WRNERROR(QUAL_OMP_PRIORITY); }
  virtual EXPR getPriority()              const {WRNERROR(QUAL_OMP_PRIORITY); }
  virtual void setProcBind(WRNProcBindKind P)   {WRNERROR("PROC_BIND");       }
  virtual void setLoopBind(WRNLoopBindKind L)   {WRNERROR("LOOP_BIND");       }
  virtual void setLoopOrder(WRNLoopOrderKind L) {WRNERROR("LOOP_ORDER");      }

  virtual const SmallVectorImpl<Instruction *> &getCancellationPoints() const {
    WRNERROR("CANCELLATION_POINTS");
  }
  virtual void addCancellationPoint(Instruction *V) {
    WRNERROR("CANCELLATION_POINTS");
  }
  virtual const SmallVectorImpl<AllocaInst *> &
  getCancellationPointAllocas() const {
    WRNERROR("CANCELLATION_POINT_ALLOCAS");
  }
  virtual void addCancellationPointAlloca(AllocaInst *V) {
    WRNERROR("CANCELLATION_POINT_ALLOCAS");
  }
  virtual const SmallVectorImpl<Value *> &
  getDirectlyUsedNonPointerValues() const {
    WRNERROR("DIRECTLY_USED_NON_POINTER_VALUES");
  }
  virtual void addDirectlyUsedNonPointerValue(Value *V) {
    WRNERROR("DIRECTLY_USED_NON_POINTER_VALUES");
  }
  virtual WRNProcBindKind getProcBind()   const {WRNERROR("PROC_BIND");       }
  virtual WRNLoopBindKind getLoopBind()   const {WRNERROR("LOOP_BIND");       }
  virtual WRNLoopOrderKind getLoopOrder() const {WRNERROR("LOOP_ORDER");      }
  virtual void setSafelen(int N)                {WRNERROR(QUAL_OMP_SAFELEN);  }
  virtual int getSafelen()                const {WRNERROR(QUAL_OMP_SAFELEN);  }
  virtual void setSchedCode(int N)              {WRNERROR("TAKSLOOP SCHED");  }
  virtual int getSchedCode()              const {WRNERROR("TAKSLOOP SCHED");  }
  virtual void setSimdlen(int N)                {WRNERROR(QUAL_OMP_SIMDLEN);  }
  virtual int getSimdlen()                const {WRNERROR(QUAL_OMP_SIMDLEN);  }
  virtual void setThreadLimit(EXPR E)        {WRNERROR(QUAL_OMP_THREAD_LIMIT);}
  virtual EXPR getThreadLimit()        const {WRNERROR(QUAL_OMP_THREAD_LIMIT);}
  virtual void setUntied(bool Flag)             {WRNERROR(QUAL_OMP_UNTIED);   }
  virtual bool getUntied()                const {WRNERROR(QUAL_OMP_UNTIED);   }
  virtual void setUserLockName(StringRef LN)    {WRNERROR(QUAL_OMP_NAME);     }
  virtual StringRef getUserLockName()     const {WRNERROR(QUAL_OMP_NAME);     }

  // WRNLoopInfo

  virtual WRNLoopInfo &getWRNLoopInfo()         {WRNERROR("WRNLoopInfo");     }
  virtual const WRNLoopInfo &getWRNLoopInfo() const
                                                {WRNERROR("WRNLoopInfo");     }

  // Task

  virtual void setTaskFlag(unsigned F)          {WRNERROR("TASK FLAG");       }
  virtual unsigned getTaskFlag()          const {WRNERROR("TASK FLAG");       }

#if INTEL_CUSTOMIZATION
  // Vectorization attributes not to be exposed externally
  virtual void setIsAutoVec(bool)               {WRNERROR("IsAutoVec");       }
  virtual bool getIsAutoVec() const             {WRNERROR("IsAutoVec");       }
  virtual void setHasVectorAlways(bool)         {WRNERROR("HasVectorAlways"); }
  virtual bool getHasVectorAlways() const       {WRNERROR("HasVectorAlways"); }

  // These methods are only available in WRNs constructed from HIR
  virtual void setEntryHLNode(loopopt::HLNode *)  {WRNERROR("EntryHLNode");   }
  virtual loopopt::HLNode *getEntryHLNode() const {WRNERROR("EntryHLNode");   }
  virtual void setExitHLNode(loopopt::HLNode *)   {WRNERROR("ExitHLNode");    }
  virtual loopopt::HLNode *getExitHLNode() const  {WRNERROR("ExitHLNode");    }
  virtual void setHLLoop(loopopt::HLLoop *)       {WRNERROR("HLLoop");        }
  virtual loopopt::HLLoop *getHLLoop() const      {WRNERROR("HLLoop");        }
#endif //INTEL_CUSTOMIZATION

  /// Only these classes are allowed to create/modify/delete WRegionNode.
  friend class WRegionUtils;
  friend class WRegionCollection;  //temporary
  friend class VPOParoptTransform;

  // WRegionNodes are destroyed in bulk using WRegionUtils::destroyAll()
  virtual ~WRegionNode() {
    for (auto *Child : Children)
      delete Child;
    Children.clear();
  }

  // Virtual Clone Method
  // virtual WRegionNode *clone() const = 0;

  /// Returns the unique number associated with this WRegionNode.
  unsigned getNumber() const { return Number; }

  /// Returns the nesting level of this WRegionNode.
  unsigned getLevel() const { return Level; }

#if INTEL_CUSTOMIZATION
  /// Returns the flag that indicates if WRN came from HIR
  bool getIsFromHIR() const { return IsFromHIR; }
#endif // INTEL_CUSTOMIZATION

  /// Dumps WRegionNode.
  void dump(unsigned Verbosity=0) const;

  /// Default printer for WRegionNode. The derived WRegion can define
  /// its own print() routine to override this one.
  virtual void print(formatted_raw_ostream &OS, unsigned Depth,
                     unsigned Verbosity=1) const;

  /// Prints "BEGIN  <DIRECTIVE_NAME> {"
  void printBegin(formatted_raw_ostream &OS, unsigned Depth) const;

  /// Prints "} END  <DIRECTIVE_NAME>"
  void printEnd(formatted_raw_ostream &OS, unsigned Depth) const;

  /// This virtual function is intended for derived WRNs to print
  /// additional information specific to the derived WRN not covered by
  /// printBody() below.
  virtual void printExtra(formatted_raw_ostream &OS, unsigned Depth,
                          unsigned Verbosity=1) const {}

  /// Prints content of the WRegionNode.
  void printBody(formatted_raw_ostream &OS, bool PrintChildren, unsigned Depth,
                 unsigned Verbosity=1) const;

  /// Prints content of list-type clauses in the WRN
  void printClauses(formatted_raw_ostream &OS, unsigned Depth,
                    unsigned Verbosity=1) const;

  /// Prints EntryBB, ExitBB, and BBlockSet
  void printEntryExitBB(formatted_raw_ostream &OS, unsigned Depth,
                        unsigned Verbosity=1) const;

#if INTEL_CUSTOMIZATION
  /// When IsFromHIR==true, prints EntryHLNode, ExitHLNode, and HLLoop.
  /// This is virtual here; the derived WRNs supporting HIR have to provide
  /// the actual routine.
  virtual void printHIR(formatted_raw_ostream &OS, unsigned Depth,
                        unsigned Verbosity=1) const {}
#endif // INTEL_CUSTOMIZATION

  /// If IsOmpLoop==true, prints loop preheader, header, and latch BBs
  void printLoopBB(formatted_raw_ostream &OS, unsigned Depth,
                   unsigned Verbosity=1) const;

  /// Prints WRegionNode children.
  void printChildren(formatted_raw_ostream &OS, unsigned Depth,
                     unsigned Verbosity=1) const;

  /// Returns the predecessor bblock of this region.
  BasicBlock *getPredBBlock() const;

  /// Returns the successor bblock of this region.
  BasicBlock *getSuccBBlock() const;

  /// Returns the immediate enclosing parent of the WRegionNode.
  WRegionNode *getParent() const { return Parent; }

  /// Children iterator methods
  wrn_iterator         wrn_child_begin() { return Children.begin(); }
  wrn_iterator         wrn_child_end()   { return Children.end(); }

  wrn_const_iterator   wrn_child_begin() const { return Children.begin(); }
  wrn_const_iterator   wrn_child_end()   const { return Children.end(); }

  wrn_reverse_iterator wrn_child_rbegin() { return Children.rbegin(); }
  wrn_reverse_iterator wrn_child_rend()   { return Children.rend(); }

  wrn_const_reverse_iterator wrn_child_rbegin() const {
    return Children.rbegin();
  }
  wrn_const_reverse_iterator wrn_child_rend() const {
    return Children.rend();
  }

  /// Children acess methods

  /// Returns true if it has children.
  bool hasChildren() const { return !Children.empty(); }

  /// Returns the number of children.
  unsigned getNumChildren() const { return Children.size(); }

  /// Returns the Children container (by ref)
  WRContainerImpl &getChildren() { return Children ; }

  /// Returns the first child if it exists, otherwise returns null.
  WRegionNode *getFirstChild();

  /// Returns the last child if it exists, otherwise returns null.
  WRegionNode *getLastChild();

  /// Returns an ID for the concrete type of this object.
  ///
  /// This is used to implement the classof checks in LLVM and should't
  /// be used for any other purpose.
  unsigned getWRegionKindID() const { return SubClassID; }
  void setWRegionKindID(unsigned ID) { SubClassID = ID; }

  /// Returns the name for this WRN based on its SubClassID
  StringRef getName() const;

  // Methods for BBlockSet

  /// Returns the entry(first) bblock of this region.
  BasicBlock *getEntryBBlock() const { return EntryBBlock; }

  /// Returns the exit(last) bblock of this region.
  BasicBlock *getExitBBlock() const { return ExitBBlock; }

  /// Returns the entry(first) Directive of this region.
  Instruction *getEntryDirective() const { return EntryDirective; }

  /// Returns the exit(last) Directive of this region.
  Instruction *getExitDirective() const { return ExitDirective; }

  /// Basic Block set iterator methods.
  bbset_iterator bbset_begin() { return BBlockSet.begin(); }
  bbset_const_iterator bbset_begin() const { return BBlockSet.begin(); }
  bbset_iterator bbset_end() { return BBlockSet.end(); }
  bbset_const_iterator bbset_end() const { return BBlockSet.end(); }

  bbset_reverse_iterator bbset_rbegin() { return BBlockSet.rbegin(); }
  bbset_const_reverse_iterator bbset_rbegin() const {
                                                  return BBlockSet.rbegin(); }
  bbset_reverse_iterator bbset_rend() { return BBlockSet.rend(); }
  bbset_const_reverse_iterator bbset_rend() const { return BBlockSet.rend(); }

  iterator_range<bbset_iterator> blocks() {
    return make_range(bbset_begin(), bbset_end());
  }
  iterator_range<bbset_const_iterator> blocks() const {
    return make_range(bbset_begin(), bbset_end());
  }

  /// Returns \b true if \p BB is in the BBlockSet.
  /// Assumes BBlockSet is computed.
  bool contains(BasicBlock *BB) const {
    return is_contained(blocks(), BB);
  }

  /// Returns \b true if BasicBlockSet is empty.
  bool isBBSetEmpty() const { return BBlockSet.empty(); }

  /// Returns the number of BasicBlocks in BBlockSet.
  unsigned getBBSetSize() const { return BBlockSet.size(); }

  /// Populates BBlockSet with BBs in the WRN from EntryBB to ExitBB.
  /// If \p Always is false (default), then compute the BBlockSet only if
  /// it is empty. Otherwise, compute it unconditionally.
  /// Returns \b true if a new BBlockSet is computed; \b false if no change.
  bool populateBBSet(bool Always = false);

  /// Clears the BBlockSet. Any transformation that invalidates the BBlockSet
  /// must clear it so the next transformation doesn't use an outdated set.
  void resetBBSet() { BBlockSet.clear(); }
  void resetBBSetIfChanged(bool Changed) {
    if (Changed)
      BBlockSet.clear();
  }

  /// Routines to set WRN primary attributes
  void setAttributes(unsigned A) { Attributes = A; }
  void setIsDistribute()         { Attributes |= WRNIsDistribute; }
  void setIsPar()                { Attributes |= WRNIsPar; }
  void setIsOmpLoop()            { Attributes |= WRNIsOmpLoop; }
  void setIsSections()           { Attributes |= WRNIsSections; }
  void setIsTarget()             { Attributes |= WRNIsTarget; }
  void setIsTask()               { Attributes |= WRNIsTask; }
  void setIsTeams()              { Attributes |= WRNIsTeams; }

  /// Routines to get WRN primary attributes
  unsigned getAttributes() const { return Attributes; }
  bool getIsDistribute()   const { return Attributes & WRNIsDistribute; }
  bool getIsPar()          const { return Attributes & WRNIsPar; }
  bool getIsOmpLoop()      const { return Attributes & WRNIsOmpLoop; }
  bool getIsSections()     const { return Attributes & WRNIsSections; }
  bool getIsTarget()       const { return Attributes & WRNIsTarget; }
  bool getIsTask()         const { return Attributes & WRNIsTask; }
  bool getIsTeams()        const { return Attributes & WRNIsTeams; }

  /// Routines to get WRN derived attributes
  bool getIsParLoop()      const { return  getIsPar()  && getIsOmpLoop(); }
  bool getIsParSections()  const { return  getIsPar()  && getIsSections(); }
  bool getIsTaskloop()     const { return  getIsTask() && getIsOmpLoop(); }
  bool getIsWksLoop()      const { return !getIsTask() && getIsOmpLoop(); }

  /// Routine to check if the WRN needs global thread-id during codegen.
  /// Currently only SIMD and FLUSH constructs don't need the thread-id.
  bool needsTID()          const { return SubClassID != WRNVecLoop &&
                                          SubClassID != WRNFlush; }

  /// Routine to check if the WRN needs the BID during codegen.
  /// The BID is the second parameter in a parallel entry, so this routine
  /// is equivalent to getIsPar() or getIsTeams(). In other words, it is
  /// true only for PARALLEL or TEAMS directives and combined/composite
  /// directives that have the PARALLEL or TEAMS keyword.
  bool needsBID() const { return getIsPar() || getIsTeams(); }

  /// Routines to set/get DirID
  void setDirID(int ID)          { DirID = ID; }
  int  getDirID()          const { return DirID; }

  void setMayHaveOMPCritical(bool Value = true) { MayHaveOMPCritical = Value; }
  bool mayHaveOMPCritical() const { return MayHaveOMPCritical; }

  // Derived Class Enumeration

  /// An enumeration to keep track of the concrete subclasses of
  /// WRegionNode
  enum WRegionNodeKind{
                                      // WRNAttribute:
    // These require outlining:

    WRNParallel,                      // IsPar
    WRNParallelLoop,                  // IsPar, IsOmpLoop
    WRNParallelSections,              // IsPar, IsOmpLoop, IsSections
    WRNParallelWorkshare,             // IsPar, IsOmpLoop
    WRNTeams,                         // IsTeams
    WRNDistributeParLoop,             // IsPar, IsOmpLoop, IsDistribute
    WRNTarget,                        // IsTarget
    WRNTargetData,                    // IsTarget
    WRNTargetEnterData,               // IsTarget
    WRNTargetExitData,                // IsTarget
    WRNTargetUpdate,                  // IsTarget
    WRNTargetVariant,
    WRNTask,                          // IsTask
    WRNTaskloop,                      // IsTask, IsOmpLoop

    // These don't require outlining:

    WRNGenericLoop,                   // IsOmpLoop
    WRNVecLoop,                       // IsOmpLoop
    WRNWksLoop,                       // IsOmpLoop
    WRNSections,                      // IsOmpLoop, IsSections
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
    WRNSection,
#endif // INTEL_FEATURE_CSA
#endif // INTEL_CUSTOMIZATION
    WRNWorkshare,                     // IsOmpLoop
    WRNDistribute,                    // IsOmpLoop, IsDistribute
    WRNAtomic,
    WRNBarrier,
    WRNCancel,
    WRNCritical,
    WRNFlush,
    WRNOrdered,
    WRNMaster,
    WRNSingle,
    WRNTaskgroup,
    WRNTaskwait,
    WRNTaskyield
  };

  /// WRN primary attributes
  enum WRNAttributes : uint32_t {
    WRNIsDistribute = 0x00000001,
    WRNIsPar        = 0x00000002,
    WRNIsOmpLoop    = 0x00000004,
    WRNIsSections   = 0x00000008,
    WRNIsTarget     = 0x00000010,
    WRNIsTask       = 0x00000020,
    WRNIsTeams      = 0x00000040
  };

private:
  /// \name Clause qualifier parsing related Utilities
  /// @{

  /// Extract the operands for a list-type clause.
  /// This is called by WRegionNode::handleQualOpndList()
  template <typename ClauseTy>
#if INTEL_CUSTOMIZATION
  // Functions like this are not static with INTEL_CUSTOMIZATION as they
  // need to access the CurrentBundleDDRefs SmallVector while parsing clauses.
         void extractQualOpndList(const Use *Args, unsigned NumArgs,
#else
  static void extractQualOpndList(const Use *Args, unsigned NumArgs,
#endif // INTEL_CUSTOMIZATION
                                  int ClauseID, ClauseTy &C);

  // The following interface uses ClauseInfo instead of ClauseID to support
  // the "ByRef" attribute.
  template <typename ClauseTy>
#if INTEL_CUSTOMIZATION
         void extractQualOpndList(const Use *Args, unsigned NumArgs,
#else
  static void extractQualOpndList(const Use *Args, unsigned NumArgs,
#endif // INTEL_CUSTOMIZATION
                                  const ClauseSpecifier &ClauseInfo,
                                  ClauseTy &C);

  template <typename ClauseItemTy>
#if INTEL_CUSTOMIZATION
         void extractQualOpndListNonPod(const Use *Args, unsigned NumArgs,
#else
  static void extractQualOpndListNonPod(const Use *Args, unsigned NumArgs,
#endif // INTEL_CUSTOMIZATION
                                        const ClauseSpecifier &ClauseInfo,
                                        Clause<ClauseItemTy> &C);

  /// Extract operands from a map clause
  static void extractMapOpndList(const Use *Args, unsigned NumArgs,
                                 const ClauseSpecifier &ClauseInfo,
                                 MapClause &C, unsigned MapKind);

  /// Extract operands from a depend clause
  static void extractDependOpndList(const Use *Args, unsigned NumArgs,
                                    const ClauseSpecifier &ClauseInfo,
                                    DependClause &C, bool IsIn);

  /// Extract operands from a linear clause
#if INTEL_CUSTOMIZATION
         void extractLinearOpndList(const Use *Args, unsigned NumArgs,
#else
  static void extractLinearOpndList(const Use *Args, unsigned NumArgs,
#endif // INTEL_CUSTOMIZATION
                                    const ClauseSpecifier &ClauseInfo,
                                    LinearClause &C);

  /// Extract operands from a reduction clause
#if INTEL_CUSTOMIZATION
         void extractReductionOpndList(const Use *Args, unsigned NumArgs,
#else
  static void extractReductionOpndList(const Use *Args, unsigned NumArgs,
#endif // INTEL_CUSTOMIZATION
                                       const ClauseSpecifier &ClauseInfo,
                                       ReductionClause &C, int ReductionKind,
                                       bool IsInreduction);

  /// Extract operands from a schedule clause
  static void extractScheduleOpndList(ScheduleClause &Sched, const Use *Args,
                                      const ClauseSpecifier &ClauseInfo,
                                      WRNScheduleKind Kind);
  /// @}

}; // class WRegionNode

// Printing routines to help dump WRN content

/// Auxiliary function to print a BB in a WRN dump.
///
/// If BB is null:
///  * Verbosity == 0: exit without printing anything
///  * Verbosity >= 1: print "Title: NULL BBlock"
///
/// If BB is not null:
///  * Verbosity <= 1: : print BB->getName()
///  * Verbosity >= 2: : print *BB (dumps the Bblock content)
extern void printBB(StringRef Title, BasicBlock *BB, formatted_raw_ostream &OS,
                    int Indent, unsigned Verbosity=1);

/// Auxiliary function to print a Value in a WRN dump.
///
/// If Val is null:
///  * Verbosity == 0: exit without printing anything
///  * Verbosity >= 1: print "Title: NULL Value"
///
/// If Val is not null:
///  * print *Val regardless of Verbosity
extern void printVal(StringRef Title, Value *Val, formatted_raw_ostream &OS,
                     int Indent, unsigned Verbosity=1);

/// Auxiliary function to print an ArrayRef of Values in a WRN dump.
///
/// If an element Val in Vals is undef/null:
///  * Verbosity == 0: don't printing anything
///  * Verbosity >= 1: print "UNSPECIFIED"
///
/// If the element Val is not undef/null:
///  * print it irrespective of verbosity
extern void printValList(StringRef Title, ArrayRef<Value *> const &Vals,
                         formatted_raw_ostream &OS, int Indent,
                         unsigned Verbosity = 1);

/// Auxiliary function to print an Int in a WRN dump.
///
/// If \p Num < \p Min:
///  * \p Verbosity == 0: exit without printing anything
///  * \p Verbosity >= 1: print "Title: UNSPECIFIED"
///
/// If \p Num >= \p Min:
///  * print "Title: <Num>"
extern void printInt(StringRef Title, int Num, formatted_raw_ostream &OS,
                     int Indent, unsigned Verbosity=1, int Min=1);

/// Auxiliary function to print a boolean in a WRN dump.
///
/// If \p Verbosity == 0 and \p Flag is `false`:
/// * don't print anything
///
/// otherwise:
/// * print "Title: true/false"
extern void printBool(StringRef Title, bool Flag, formatted_raw_ostream &OS,
                      int Indent, unsigned Verbosity=1);

/// Auxiliary function to print a String for dumping certain clauses.
/// E.g., for the DEFAULT clause we may print "NONE", "SHARED", "PRIVATE", etc.
///
/// If \p Str == "UNSPECIFIED"  (happens when the clause is not specified):
///  * Verbosity == 0: exit without printing anything
///  * Verbosity >= 1: print "Title: UNSPECIFIED"
///
/// Else
///  * print "Title: <Str>"
extern void printStr(StringRef Title, StringRef Str, formatted_raw_ostream &OS,
                     int Indent, unsigned Verbosity=1);

} // End vpo namespace

} // End llvm namespace

#endif // LLVM_ANALYSIS_VPO_WREGIONNODE_H
#endif // INTEL_COLLAB
