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

#include "llvm/ADT/DenseMap.h"
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

namespace llvm {
#if INTEL_CUSTOMIZATION

namespace loopopt {
class HLNode;
class HLInst;
class HLLoop;
}
#endif //INTEL_CUSTOMIZATION

namespace vpo {

extern DenseMap<int, StringRef> WRNName;

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

  /// Index of the current map Aggr (map-chain-link).
  unsigned CurrentMapAggrIndex = 0;

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

  /// Dominator Tree (initialized/used during transformation).
  DominatorTree* DT = nullptr;

  /// Insertion point for Allocas in case of VLA or a variable length array
  /// section
  Instruction *VlaAllocaInsertPt = nullptr;

  /// Counter used for assigning unique numbers to WRegionNodes.
  static unsigned UniqueNum;

  /// Sets the unique number associated with this WRegionNode.
  void setNextNumber() { Number = ++UniqueNum; }

  /// Returns the index of the next map-chain Aggr for the region.
  /// This is used in WRegion graph construction to track the
  /// indices of original map-chains sent from the frontend. The need for this
  /// is to track which Aggr in a map-chain a member-of field points to, so that
  /// the value of member-of can be updated correctly, if needed. e.g.
  /// \code
  ///                Map Aggrs                      Index
  ///             <a, b, 8, 0x22>                    1
  ///   +         <c, d, 8, 0x20>                    2
  ///   +- CHAIN: <e, f, 8, 0x00>                    3
  ///   +- CHAIN: <g, h, 8, 0x0003000000000001>      4
  ///
  /// \endcode
  /// Here, tracking the indices lets us know that Aggr 4 is a member-of Aggr 3.
  /// This information is later used while updating member-of flags in Paropt
  /// code-generation, if needed.
  unsigned getNextMapAggrIndex() { return ++CurrentMapAggrIndex; }

  /// True for implicit constructs emitted by the frontend.
  /// Examples:
  /// * implicit task around a dispatch construct
  /// * implicit taskgroup around a taskloop construct
  bool IsImplicit = false;

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

  /// Set whether this WRegionNode is for an implicit construct.
  void setIsImplicit(bool B) { IsImplicit = B; }

  /// Set the DominatorTree of this region.
  void setDT(DominatorTree *D) { DT = D; }

  /// Set the insertPt of Vla Alloca
  void setVlaAllocaInsertPt(Instruction *I) { VlaAllocaInsertPt = I; }

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
  void handleQual(const ClauseSpecifier &ClauseInfo);

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
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
  bool canHaveWorkerSchedule() const;
#endif // INTEL_FEATURE_CSA
#endif //INTEL_CUSTOMIZATION
  bool canHaveShared() const;
  bool canHavePrivate() const;
  bool canHaveFirstprivate() const;
  bool canHaveLastprivate() const;
  bool canHaveInReduction() const;
  bool canHaveReduction() const;
  bool canHaveReductionInscan() const;
  bool canHaveCopyin() const;
  bool canHaveCopyprivate() const;
  bool canHaveLinear() const;
  bool canHaveUniform() const;
  bool canHaveInclusive() const;
  bool canHaveExclusive() const;
  bool canHaveMap() const;
  bool canHaveIsDevicePtr() const;
  bool canHaveUseDevicePtr() const;
  bool canHaveSubdevice() const;
  bool canHaveInterop() const;
  bool canHaveInteropAction() const;
  bool canHaveDepend() const;
  bool canHaveDepSrcSink() const;
  bool canHaveAligned() const;
  bool canHaveNontemporal() const;
  bool canHaveFlush() const;
  bool canHaveData() const;
  bool canHaveCancellationPoints() const; ///< Constructs that can be cancelled
  bool canHaveCollapse() const;
  bool canHaveNowait() const;
  bool canHaveAllocate() const;
  bool canHaveOrderedTripCounts() const;
  bool canHaveIf() const;
  bool canHaveSizes() const;
  bool canHaveLivein() const;
#if INTEL_CUSTOMIZATION
  bool canHaveDoConcurrent() const;
#endif // INTEL_CUSTOMIZATION
  /// @}

  /// Returns `true` if the construct needs to be outlined into a separate
  /// function which is accepted by the OpenMP runtime.
  bool needsOutlining() const;

  /// \name Utility Functions to get a Clause Pointer if supported by a WRegion.
  /// If not supported returns a nullptr.
  /// @{
  SharedClause *getSharedIfSupported() {
    return (canHaveShared() ? &getShared()
                            : static_cast<SharedClause *>(nullptr));
  }
  PrivateClause *getPrivIfSupported() {
    return (canHavePrivate() ? &getPriv()
                             : static_cast<PrivateClause *>(nullptr));
  }
  FirstprivateClause *getFprivIfSupported() {
    return (canHaveFirstprivate() ? &getFpriv()
                                  : static_cast<FirstprivateClause *>(nullptr));
  }
  LastprivateClause *getLprivIfSupported() {
    return (canHaveLastprivate() ? &getLpriv()
                                 : static_cast<LastprivateClause *>(nullptr));
  }
  ReductionClause *getRedIfSupported() {
    return (canHaveReduction() ? &getRed()
                               : static_cast<ReductionClause *>(nullptr));
  }
  LinearClause *getLinearIfSupported() {
    return (canHaveLinear() ? &getLinear()
                            : static_cast<LinearClause *>(nullptr));
  }
  MapClause *getMapIfSupported() {
    return (canHaveMap() ? &getMap() : static_cast<MapClause *>(nullptr));
  }

  DominatorTree *getDT() const { return DT; }

  Instruction *getVlaAllocaInsertPt() { return VlaAllocaInsertPt; }

  /// @}

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
  virtual NontemporalClause &getNontemporal() {WRNERROR(QUAL_OMP_NONTEMPORAL); }
  virtual AllocateClause &getAllocate()      {WRNERROR(QUAL_OMP_ALLOCATE);    }
  virtual CopyinClause &getCopyin()          {WRNERROR(QUAL_OMP_COPYIN);      }
  virtual CopyprivateClause &getCpriv()      {WRNERROR(QUAL_OMP_COPYPRIVATE); }
  virtual DataClause &getData()              {WRNERROR(QUAL_OMP_DATA);        }
  virtual DependClause &getDepend()          {WRNERROR("DEPEND");             }
  virtual DepSinkClause &getDepSink()        {WRNERROR("DEPEND(SINK:..)");    }
  virtual DepSourceClause &getDepSource()    {WRNERROR("DEPEND(SOURCE)");     }
  virtual FirstprivateClause &getFpriv()     {WRNERROR(QUAL_OMP_FIRSTPRIVATE);}
  virtual FlushSet &getFlush()               {WRNERROR(QUAL_OMP_FLUSH);       }
  virtual InteropClause &getInterop()        {WRNERROR(QUAL_OMP_INTEROP);     }
  virtual IsDevicePtrClause &getIsDevicePtr()
                                            {WRNERROR(QUAL_OMP_IS_DEVICE_PTR);}
  virtual LastprivateClause &getLpriv()      {WRNERROR(QUAL_OMP_LASTPRIVATE); }
  virtual LinearClause &getLinear()          {WRNERROR(QUAL_OMP_LINEAR);      }
  virtual LiveinClause &getLivein()          {WRNERROR(QUAL_OMP_LIVEIN);      }
  virtual MapClause &getMap()                {WRNERROR("MAP");                }
  virtual PrivateClause &getPriv()           {WRNERROR(QUAL_OMP_PRIVATE);     }
  virtual ReductionClause &getInRed()        {WRNERROR("IN_REDUCTION");       }
  virtual ReductionClause &getRed()          {WRNERROR("REDUCTION");          }
  virtual ScheduleClause &getSchedule()      {WRNERROR("SCHEDULE");           }
       // ScheduleClause is not list-type, but has similar API so put here too
  virtual ScheduleClause &getDistSchedule()   {WRNERROR("DIST_SCHEDULE");     }
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
  virtual ScheduleClause &getWorkerSchedule()  {WRNERROR("SA_SCHEDULE");      }
#endif // INTEL_FEATURE_CSA
#endif //INTEL_CUSTOMIZATION

  virtual SharedClause &getShared()          {WRNERROR(QUAL_OMP_SHARED);      }
  virtual SizesClause &getSizes()            {WRNERROR(QUAL_OMP_SIZES);       }
  virtual UniformClause &getUniform()        {WRNERROR(QUAL_OMP_UNIFORM);     }
  virtual InclusiveClause &getInclusive()    {WRNERROR(QUAL_OMP_INCLUSIVE);   }
  virtual ExclusiveClause &getExclusive()    {WRNERROR(QUAL_OMP_EXCLUSIVE);   }
  virtual UseDevicePtrClause &getUseDevicePtr()
                                           {WRNERROR(QUAL_OMP_USE_DEVICE_PTR);}
  virtual SubdeviceClause &getSubdevice()       {WRNERROR(QUAL_OMP_SUBDEVICE);}
  virtual InteropActionClause &getInteropAction() {WRNERROR("INTEROP_ACTION");}

  // list-type clauses (const getters)

  virtual const AlignedClause &getAligned() const
                                           {WRNERROR(QUAL_OMP_ALIGNED);     }
  virtual const NontemporalClause &getNontemporal() const
                                           {WRNERROR(QUAL_OMP_NONTEMPORAL); }
  virtual const AllocateClause &getAllocate() const
                                           {WRNERROR(QUAL_OMP_ALLOCATE);    }
  virtual const CopyinClause &getCopyin() const
                                           {WRNERROR(QUAL_OMP_COPYIN);      }
  virtual const CopyprivateClause &getCpriv() const
                                           {WRNERROR(QUAL_OMP_COPYPRIVATE); }
  virtual const DataClause &getData() const {WRNERROR(QUAL_OMP_DATA);       }
  virtual const DependClause &getDepend() const
                                           {WRNERROR("DEPEND");             }
  virtual const DepSinkClause &getDepSink() const
                                           {WRNERROR("DEPEND(SINK:..)");    }
  virtual const DepSourceClause &getDepSource() const
                                           {WRNERROR("DEPEND(SOURCE)");     }
  virtual const FirstprivateClause &getFpriv() const
                                           {WRNERROR(QUAL_OMP_FIRSTPRIVATE);}
  virtual const FlushSet &getFlush() const {WRNERROR(QUAL_OMP_FLUSH);       }
  virtual const InteropClause &getInterop() const
                                           {WRNERROR(QUAL_OMP_INTEROP);     }
  virtual const IsDevicePtrClause &getIsDevicePtr() const
                                          {WRNERROR(QUAL_OMP_IS_DEVICE_PTR);}
  virtual const LastprivateClause &getLpriv() const
                                           {WRNERROR(QUAL_OMP_LASTPRIVATE); }
  virtual const LinearClause &getLinear() const
                                           {WRNERROR(QUAL_OMP_LINEAR);      }
  virtual const LiveinClause &getLivein() const
                                           {WRNERROR(QUAL_OMP_LIVEIN);      }
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
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
  virtual const ScheduleClause &getWorkerSchedule() const
                                           {WRNERROR("SA_SCHEDULE")         }
#endif // INTEL_FEATURE_CSA
#endif //INTEL_CUSTOMIZATION
  virtual const SharedClause &getShared() const
                                           {WRNERROR(QUAL_OMP_SHARED);      }
  virtual const SizesClause &getSizes() const
                                           {WRNERROR(QUAL_OMP_SIZES);       }
  virtual const UniformClause &getUniform() const
                                           {WRNERROR(QUAL_OMP_UNIFORM);     }
  virtual const InclusiveClause &getInclusive() const
                                           {WRNERROR(QUAL_OMP_INCLUSIVE);   }
  virtual const ExclusiveClause &getExclusive() const
                                           {WRNERROR(QUAL_OMP_EXCLUSIVE);   }
  virtual const UseDevicePtrClause &getUseDevicePtr() const
                                         {WRNERROR(QUAL_OMP_USE_DEVICE_PTR);}
  virtual const SubdeviceClause &getSubdevice() const
                                              {WRNERROR(QUAL_OMP_SUBDEVICE);}
  virtual const InteropActionClause &getInteropAction() const
                                                {WRNERROR("INTEROP_ACTION");}

  // other clauses (both getters and setters)

  virtual void setAtomicKind(WRNAtomicKind A)   {WRNERROR("ATOMIC_KIND");     }
  virtual WRNAtomicKind getAtomicKind()   const {WRNERROR("ATOMIC_KIND");     }
  virtual void setCall(CallInst *CI)            {WRNERROR("DISPATCH CALL");   }
  virtual CallInst *getCall()             const {WRNERROR("DISPATCH CALL");   }
  virtual void setCancelKind(WRNCancelKind CK)  {WRNERROR("CANCEL TYPE");     }
  virtual WRNCancelKind getCancelKind()   const {WRNERROR("CANCEL TYPE");     }
#if INTEL_CUSTOMIZATION
  virtual void setIsDoConcurrent(bool B)    {WRNERROR(QUAL_EXT_DO_CONCURRENT);}
  virtual bool getIsDoConcurrent() const    {WRNERROR(QUAL_EXT_DO_CONCURRENT);}
#endif // INTEL_CUSTOMIZATION
  virtual void setCollapse(int N)               {WRNERROR(QUAL_OMP_COLLAPSE); }
  virtual int getCollapse()               const {WRNERROR(QUAL_OMP_COLLAPSE); }
  virtual void setDefault(WRNDefaultKind T)     {WRNERROR("DEFAULT");         }
  virtual WRNDefaultKind getDefault()     const {WRNERROR("DEFAULT");         }
  virtual void setDefaultmap(WRNDefaultmapCategory C, WRNDefaultmapBehavior B)
                                                {WRNERROR("DEFAULTMAP");      }
  virtual WRNDefaultmapBehavior getDefaultmap(WRNDefaultmapCategory C) const
                                                {WRNERROR("DEFAULTMAP");      }
  virtual void setDepArray(EXPR E)              {WRNERROR(QUAL_OMP_DEPARRAY); }
  virtual EXPR getDepArray()              const {WRNERROR(QUAL_OMP_DEPARRAY); }
  virtual void setDepArrayNumDeps(EXPR E)       {WRNERROR(QUAL_OMP_DEPARRAY); }
  virtual EXPR getDepArrayNumDeps()       const {WRNERROR(QUAL_OMP_DEPARRAY); }
  virtual void setDevice(EXPR E)                {WRNERROR(QUAL_OMP_DEVICE);   }
  virtual EXPR getDevice()                const {WRNERROR(QUAL_OMP_DEVICE);   }
  virtual void setFilter(EXPR E)                {WRNERROR(QUAL_OMP_FILTER);   }
  virtual EXPR getFilter()                const {WRNERROR(QUAL_OMP_FILTER);   }
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
  virtual void setIsSIMD(bool Flag)          {WRNERROR(QUAL_OMP_ORDERED_SIMD);}
  virtual bool getIsSIMD()             const {WRNERROR(QUAL_OMP_ORDERED_SIMD);}
  virtual void setIsTargetTask(bool Flag)     {WRNERROR(QUAL_OMP_TARGET_TASK);}
  virtual bool getIsTargetTask()        const {WRNERROR(QUAL_OMP_TARGET_TASK);}
  virtual void setIsThreads(bool Flag)          {WRNERROR("THREADS/SIMD");    }
  virtual bool getIsThreads()             const {WRNERROR("THREADS/SIMD");    }
  virtual void setMergeable(bool Flag)          {WRNERROR(QUAL_OMP_MERGEABLE);}
  virtual bool getMergeable()             const {WRNERROR(QUAL_OMP_MERGEABLE);}
  virtual void setIsTaskwaitNowaitTask(bool Flag)  {WRNERROR(QUAL_OMP_NOWAIT);}
  virtual bool getIsTaskwaitNowaitTask() const     {WRNERROR(QUAL_OMP_NOWAIT);}
  virtual void setNocontext(EXPR E)             {WRNERROR(QUAL_OMP_NOCONTEXT);}
  virtual EXPR getNocontext()             const {WRNERROR(QUAL_OMP_NOCONTEXT);}
  virtual void setNogroup(bool Flag)            {WRNERROR(QUAL_OMP_NOGROUP);  }
  virtual bool getNogroup()               const {WRNERROR(QUAL_OMP_NOGROUP);  }
  virtual void setNovariants(EXPR E)           {WRNERROR(QUAL_OMP_NOVARIANTS);}
  virtual EXPR getNovariants()           const {WRNERROR(QUAL_OMP_NOVARIANTS);}
  virtual void setNowait(bool Flag)             {WRNERROR(QUAL_OMP_NOWAIT);   }
  virtual bool getNowait()                const {WRNERROR(QUAL_OMP_NOWAIT);   }
  virtual void setNumTasks(EXPR E)              {WRNERROR(QUAL_OMP_NUM_TASKS);}
  virtual EXPR getNumTasks()              const {WRNERROR(QUAL_OMP_NUM_TASKS);}
  virtual void setNumTeams(EXPR E)              {WRNERROR(QUAL_OMP_NUM_TEAMS);}
  virtual void setNumTeamsType(Type *T)         {WRNERROR(QUAL_OMP_NUM_TEAMS);}
  virtual EXPR getNumTeams()              const {WRNERROR(QUAL_OMP_NUM_TEAMS);}
  virtual Type *getNumTeamsType()         const {WRNERROR(QUAL_OMP_NUM_TEAMS);}
  virtual void setNumThreads(EXPR E)          {WRNERROR(QUAL_OMP_NUM_THREADS);}
  virtual EXPR getNumThreads()          const {WRNERROR(QUAL_OMP_NUM_THREADS);}
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
  virtual void setNumWorkers(int E)        {WRNERROR(QUAL_OMP_SA_NUM_WORKERS);}
  virtual int  getNumWorkers()       const {WRNERROR(QUAL_OMP_SA_NUM_WORKERS);}
  virtual void setPipelineDepth(int E)        {WRNERROR(QUAL_OMP_SA_PIPELINE);}
  virtual int  getPipelineDepth()       const {WRNERROR(QUAL_OMP_SA_PIPELINE);}
#endif // INTEL_FEATURE_CSA
#endif //INTEL_CUSTOMIZATION
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
  virtual void setUncollapsedNDRangeDimensions(ArrayRef<Value *> Dims) {
    WRNERROR("OFFLOAD_NDRANGE");
  }
  virtual void setUncollapsedNDRangeTypes(ArrayRef<Type *> Types) {
    WRNERROR("OFFLOAD_NDRANGE");
  }
  virtual const SmallVectorImpl<Value *> &
      getUncollapsedNDRangeDimensions() const {
    WRNERROR("OFFLOAD_NDRANGE");
  }
  virtual const SmallVectorImpl<Type *> &
      getUncollapsedNDRangeTypes() const {
    WRNERROR("OFFLOAD_NDRANGE");
  }
  virtual void resetUncollapsedNDRange() {
    WRNERROR("OFFLOAD_NDRANGE");
  }
  virtual void setNDRangeDistributeDim(uint8_t Dim) {
    WRNERROR("NDRANGE_DISTRIBUTE_DIM");
  }
  virtual uint8_t getNDRangeDistributeDim() const {
    WRNERROR("NDRANGE_DISTRIBUTE_DIM");
  }
  virtual void setTreatDistributeParLoopAsDistribute(bool Flag) {
    WRNERROR("TREAT_DISTRIBUTE_PAR_LOOP_AS_DISTRIBUTE");
  }
  virtual bool getTreatDistributeParLoopAsDistribute() const {
    WRNERROR("TREAT_DISTRIBUTE_PAR_LOOP_AS_DISTRIBUTE");
  }
  virtual void setHasTeamsReduction() {
    WRNERROR("OFFLOAD_HAS_TEAMS_REDUCTION");
  }
  virtual bool getHasTeamsReduction() const {
    WRNERROR("OFFLOAD_HAS_TEAMS_REDUCTION");
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
  virtual void setThreadLimitType(Type *T)   {WRNERROR(QUAL_OMP_THREAD_LIMIT);}
  virtual EXPR getThreadLimit()        const {WRNERROR(QUAL_OMP_THREAD_LIMIT);}
  virtual Type *getThreadLimitType()   const {WRNERROR(QUAL_OMP_THREAD_LIMIT);}
  virtual void setUntied(bool Flag)             {WRNERROR(QUAL_OMP_UNTIED);   }
  virtual bool getUntied()                const {WRNERROR(QUAL_OMP_UNTIED);   }
  virtual void setUserLockName(StringRef LN)    {WRNERROR(QUAL_OMP_NAME);     }
  virtual StringRef getUserLockName()     const {WRNERROR(QUAL_OMP_NAME);     }
  virtual void setHint(uint32_t N)              {WRNERROR(QUAL_OMP_HINT);     }
  virtual uint32_t getHint()              const {WRNERROR(QUAL_OMP_HINT);     }

  // WRNLoopInfo

  virtual WRNLoopInfo &getWRNLoopInfo()         {WRNERROR("WRNLoopInfo");     }
  virtual const WRNLoopInfo &getWRNLoopInfo() const
                                                {WRNERROR("WRNLoopInfo");     }
  virtual int getOmpLoopDepth()           const {WRNERROR("Loop Depth");      }

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
  friend class VPOParoptUtils;
  friend class VPOUtils; // needed for renaming operands

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
  const WRContainerImpl &getChildren() const { return Children; }

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

  /// Similar to getName, but returns WRN name taking into account source
  /// language differences. For example loop directives in C use word 'for' and
  /// 'do' in Fortran.
  StringRef getSourceName() const;

  /// Returns whether the WRegionNode is for an implicit construct.
  bool getIsImplicit() const { return IsImplicit; }

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
  void setIsOmpLoopTransform()   { Attributes |= WRNIsOmpLoopTransform; }
  void setIsSections()           { Attributes |= WRNIsSections; }
  void setIsTarget()             { Attributes |= WRNIsTarget; }
  void setIsTask()               { Attributes |= WRNIsTask; }
  void setIsTeams()              { Attributes |= WRNIsTeams; }
  void setIsInterop()            { Attributes |= WRNIsInterop; }

  /// Routines to get WRN primary attributes
  unsigned getAttributes() const { return Attributes; }
  bool getIsDistribute()   const { return Attributes & WRNIsDistribute; }
  bool getIsPar()          const { return Attributes & WRNIsPar; }
  bool getIsOmpLoop()      const { return Attributes & WRNIsOmpLoop; }
  bool getIsOmpLoopTransform() const {
    return Attributes & WRNIsOmpLoopTransform;
  }
  bool getIsOmpLoopOrLoopTransform() const {
    return Attributes & (WRNIsOmpLoop | WRNIsOmpLoopTransform);
  }
  bool getIsSections()     const { return Attributes & WRNIsSections; }
  bool getIsTarget()       const { return Attributes & WRNIsTarget; }
  bool getIsTask()         const { return Attributes & WRNIsTask; }
  bool getIsTeams()        const { return Attributes & WRNIsTeams; }
  bool getIsInterop()      const { return Attributes & WRNIsInterop; }

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
    WRNTargetVariant,                 // IsTarget
    WRNDispatch,
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
    WRNTile,                          // IsOmpLoopTransform
    WRNAtomic,
    WRNBarrier,
    WRNCancel,
    WRNCritical,
    WRNFlush,
    WRNPrefetch,
    WRNOrdered,
    WRNMasked,
    WRNSingle,
    WRNTaskgroup,
    WRNTaskwait,
    WRNTaskyield,
    WRNInterop,
    WRNScope,
    WRNGuardMemMotion,
    WRNScan,
  };

  /// WRN primary attributes
  enum WRNAttributes : uint32_t {
    WRNIsDistribute       = 0x00000001,
    WRNIsPar              = 0x00000002,
    WRNIsOmpLoop          = 0x00000004,
    WRNIsSections         = 0x00000008,
    WRNIsTarget           = 0x00000010,
    WRNIsTask             = 0x00000020,
    WRNIsTeams            = 0x00000040,
    WRNIsInterop          = 0x00000080,
    WRNIsOmpLoopTransform = 0x00000100
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
  void extractMapOpndList(const Use *Args, unsigned NumArgs,
                          const ClauseSpecifier &ClauseInfo, MapClause &C,
                          unsigned MapKind);

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
  void extractReductionOpndList(const Use *Args, unsigned NumArgs,
                                const ClauseSpecifier &ClauseInfo,
                                ReductionClause &C, int ReductionKind,
                                bool IsInreduction);

  /// Extract operands from an inclusive/exclusive clause.
  template <typename ClauseItemTy>
  void extractInclusiveExclusiveOpndList(const Use *Args, unsigned NumArgs,
                                         const ClauseSpecifier &ClauseInfo,
                                         Clause<ClauseItemTy> &C);

  /// Extract operands from a schedule clause
  static void extractScheduleOpndList(ScheduleClause &Sched, const Use *Args,
                                      const ClauseSpecifier &ClauseInfo,
                                      WRNScheduleKind Kind);

  /// Extract operands from an init clause
  static void extractInitOpndList(InteropActionClause &InteropAction,
                                  const Use *Args, unsigned NumArgs,
                                  const ClauseSpecifier &ClauseInfo);
  /// @}

}; // class WRegionNode

// Printing routines to help dump WRN content

/// Print the DEPARRAY(N, Array) qual
/// Returns true iff something was printed
extern bool printDepArray(WRegionNode const *W, formatted_raw_ostream &OS,
                          int Depth, unsigned Verbosity = 1);

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

/// Auxiliary function to print a range of Values in a WRN dump.
///
/// If Verbosity == 0 and both Val1 and Val2 are null:
///  * exit without printing anything
///
/// Else
///  * print:"Title(Val1:Val2)"
///   where Val1 and Val2 will be printed as UNSPECIFIED if equal to null

extern void printValRange(StringRef Title, Value *Val1 , Value *Val2,
                          formatted_raw_ostream &OS, int Indent,
                          unsigned Verbosity=1);

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
