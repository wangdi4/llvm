//===--------- WRegionNode.h - W-Region Graph Node --------------*- C++ -*-===//
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
//   This file defines the W-Region Graph node.
//
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

#include "llvm/Transforms/Utils/Intel_GeneralUtils.h"
#include "llvm/Analysis/Intel_VPO/Utils/VPOAnalysisUtils.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionClause.h"

#include <set>
#include <unordered_map>

namespace llvm {

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

/// \brief WRegion Node base class
class WRegionNode {

public:

  /// Iterators to iterator over basic block set
  typedef WRegionBBSetTy::iterator bbset_iterator;
  typedef WRegionBBSetTy::const_iterator bbset_const_iterator;
  typedef WRegionBBSetTy::reverse_iterator bbset_reverse_iterator;
  typedef WRegionBBSetTy::const_reverse_iterator bbset_const_reverse_iterator;

private:
  /// \brief Make class uncopyable.
  void operator=(const WRegionNode &) = delete;

  /// Unique number associated with this WRegionNode.
  unsigned Number;

  /// Nesting level of the WRN node in the WRN graph. Outermost level is 0.
  unsigned Level;

  /// ID to differentitate between concrete subclasses.
  const unsigned SubClassID;

  /// The OMP_DIRECTIVES enum representing the OMP construct. This is useful
  /// for opt reporting, which can't use SubClassID because multiple 
  /// OMP_DIRECTIVES may map to the same SubClassID. For example, 
  ///   DIR_OMP_TARGET_DATA, DIR_OMP_TARGET_ENTER_DATA,  
  ///   DIR_OMP_TARGET_EXIT_DATA, and DIR_OMP_TARGET_UPDATE 
  /// all map to WRNTargetDataNode
  int DirID;
  
  /// Bit vector for attributes such as WRNIsParLoop or WRNIsTask.
  /// The enum WRNAttributes lists the attributes.
  uint32_t Attributes;

  /// Entry and Exit BBs of this WRN
  BasicBlock    *EntryBBlock;
  BasicBlock    *ExitBBlock;

  /// Set containing all the BBs in this WRN
  WRegionBBSetTy BBlockSet;

  /// Enclosing parent of WRegionNode in CFG.
  WRegionNode *Parent;

  /// Children container
  WRContainerTy Children;

  /// True if the WRN came from HIR; false otherwise
  bool IsFromHIR;

  /// Counter used for assigning unique numbers to WRegionNodes.
  static unsigned UniqueNum;

  /// \brief Sets the unique number associated with this WRegionNode.
  void setNextNumber() { Number = ++UniqueNum; }

  /// \brief Sets the flag to indicate if WRN came from HIR
  void setIsFromHIR(bool flag) { IsFromHIR = flag; }

  /// \brief Destroys all objects of this class. Should only be
  /// called after code gen.
  static void destroyAll();

protected:

  /// \brief constructors
  WRegionNode(unsigned SCID, BasicBlock *BB); // for LLVM IR
  WRegionNode(unsigned SCID);                 // for HIR only
  WRegionNode(WRegionNode *W);                // for both

  // copy constructor not needed (at least for now)
  // WRegionNode(const WRegionNode &WRegionNodeObj);

  /// \brief Destroys the object.
  void destroy();

  /// \brief Sets the nesting level of this WRegionNode.
  void setLevel(int K) { Level = K; }

  /// \brief Sets the entry(first) bblock of this region.
  void setEntryBBlock(BasicBlock *EntryBB) { EntryBBlock = EntryBB; }

  /// \brief Sets the exit(last) bblock of this region.
  void setExitBBlock(BasicBlock *ExitBB) { ExitBBlock = ExitBB; }

  /// \brief Sets the graph parent of this WRegionNode.
  void setParent(WRegionNode *P) { Parent = P; }

  /// \brief Finish creating the WRN once its ExitBB is found. This routine
  /// calls WRN->setExitBBlock(ExitBB). In addition, if the WRN is a loop
  /// construct, this routine also calls IntelGeneralUtils::getLoopFromLoopInfo
  /// to find the Loop from LoopInfo
  void finalize(BasicBlock *ExitBB);

  //
  // Routines for parsing clauses
  //

  /// \brief Parse a clause in the llvm.intel.directive.qual* representation.
  void parseClause(const ClauseSpecifier &ClauseInfo, IntrinsicInst *Call);

  /// \brief Common code to parse a clause, used for both representations:
  /// llvm.intel.directive.qual* and directive.region.entry/exit.
  void parseClause(const ClauseSpecifier &ClauseInfo, const Use *Args, 
                   unsigned NumArgs);

  /// \brief Update WRN for clauses with no operands.
  void handleQual(int ClauseID);

  /// \brief Update WRN for clauses with one operand.
  void handleQualOpnd(int ClauseID, Value *V);

  /// \brief Update WRN for clauses with operand list.
  void handleQualOpndList(const Use *Args, unsigned NumArgs, 
                          const ClauseSpecifier &ClauseInfo);

  /// \brief Update WRN for clauses from the OperandBundles under the 
  /// directive.region.entry/exit representation
  void getClausesFromOperandBundles();

public:

  /// \brief Functions to check if the WRN allows a given clause type
  bool hasShared();
  bool hasPrivate();
  bool hasFirstprivate();
  bool hasLastprivate();
  bool hasReduction();
  bool hasCopyin();
  bool hasCopyprivate();
  bool hasLinear();
  bool hasUniform();
  
  // Below are virtual functions to get/set clause information of the WRN.
  // These routines should never be called; calling them indicates intention
  // to access clause info for a WRN that does not allow such clause (eg, a 
  // parallel construct does not take a collapse clause). These virtual 
  // functions defined in the base class will all emit an error message.
  // Note: The return stmt in the getters below prevent compiler warnings
  //       when building the compiler.
  void errorClause(StringRef ClauseName) const;
  void errorClause(int ClauseID) const;

  virtual void setFlush(FlushSet *S) {errorClause(QUAL_OMP_FLUSH);  }
  virtual FlushSet *getFlush() const {errorClause(QUAL_OMP_FLUSH);
                                              return nullptr;                 }
  virtual void setAligned(AlignedClause *A)  {errorClause(QUAL_OMP_ALIGNED);  }
  virtual AlignedClause *getAligned()  const {errorClause(QUAL_OMP_ALIGNED);
                                              return nullptr;                 }
  virtual void setAtomicKind(WRNAtomicKind A)   {errorClause("ATOMIC_KIND");  }
  virtual WRNAtomicKind getAtomicKind()   const {errorClause("ATOMIC_KIND");
                                                 return WRNAtomicUpdate;      }
  virtual void setHasSeqCstClause(bool SC)      {errorClause("SEQ_CST");      }
  virtual bool getHasSeqCstClause()       const {errorClause("SEQ_CST");
                                                 return false;                }
  virtual void setUserLockName(StringRef LN)    {errorClause(QUAL_OMP_NAME);  }
  virtual StringRef getUserLockName()     const {errorClause(QUAL_OMP_NAME);
                                                 return "";                   }
  virtual void setCancelKind(WRNCancelKind CK) {errorClause("CANCEL TYPE");   }
  virtual WRNCancelKind getCancelKind()  const {errorClause("CANCEL TYPE");
                                              return WRNCancelError;          }
  virtual void setCollapse(int N)            {errorClause(QUAL_OMP_COLLAPSE); }
  virtual int getCollapse()            const {errorClause(QUAL_OMP_COLLAPSE);
                                              return 0;                       }
  virtual void setCopyin(CopyinClause *C)    {errorClause(QUAL_OMP_COPYIN);   }
  virtual CopyinClause *getCopyin()    const {errorClause(QUAL_OMP_COPYIN);
                                              return nullptr;                 }
  virtual void setCpriv(CopyprivateClause *C)
                          {errorClause(QUAL_OMP_COPYPRIVATE);                 }
  virtual CopyprivateClause *getCpriv() const
                          {errorClause(QUAL_OMP_COPYPRIVATE); return nullptr; }
  virtual void setDefault(WRNDefaultKind T)  {errorClause("DEFAULT");         }
  virtual WRNDefaultKind getDefault()  const {errorClause("DEFAULT");
                                              return WRNDefaultAbsent;        }
  virtual void setDefaultmapTofromScalar(bool F) {errorClause("DEFAULTMAP");  }
  virtual bool getDefaultmapTofromScalar() const {errorClause("DEFAULTMAP");
                                              return false;                   }
  virtual void setDepend(DependClause *D)    {errorClause("DEPEND");          }
  virtual DependClause *getDepend()    const {errorClause("DEPEND");
                                              return nullptr;                 }
  virtual void setDepSink(DepSinkClause *D)  {errorClause("DEPEND(SINK:..)"); }
  virtual DepSinkClause *getDepSink()  const {errorClause("DEPEND(SINK:..)"); 
                                              return nullptr;                 }
  virtual void setDevice(EXPR E)             {errorClause(QUAL_OMP_DEVICE);   }
  virtual EXPR getDevice()             const {errorClause(QUAL_OMP_DEVICE);
                                              return nullptr;                 }
  virtual void setFinal(EXPR E)              {errorClause(QUAL_OMP_FINAL);    }
  virtual EXPR getFinal()              const {errorClause(QUAL_OMP_FINAL);
                                              return nullptr;                 }
  virtual void setFpriv(FirstprivateClause *F)
                          {errorClause(QUAL_OMP_FIRSTPRIVATE);                }
  virtual FirstprivateClause *getFpriv()const
                          {errorClause(QUAL_OMP_FIRSTPRIVATE); return nullptr;}
  virtual void setGrainsize(EXPR E)          {errorClause(QUAL_OMP_GRAINSIZE);}
  virtual EXPR getGrainsize()          const {errorClause(QUAL_OMP_GRAINSIZE);
                                              return nullptr;                 }
  virtual void setIf(EXPR E)                 {errorClause(QUAL_OMP_IF);       }
  virtual EXPR getIf()                 const {errorClause(QUAL_OMP_IF);
                                              return nullptr;                 }
  virtual void setIsDepSource(bool F)        {errorClause("DEPEND(SOURCE)");  }
  virtual bool getIsDepSource()        const {errorClause("DEPEND(SOURCE)");
                                              return false;                   }
  virtual void setIsDevicePtr(IsDevicePtrClause *IDP)
                        {errorClause(QUAL_OMP_IS_DEVICE_PTR);                 }
  virtual IsDevicePtrClause *getIsDevicePtr() const
                        {errorClause(QUAL_OMP_IS_DEVICE_PTR); return nullptr; }
  virtual void setIsDoacross(bool F)    {errorClause("DEPEND(SOURCE|SINK)");  }
  virtual bool getIsDoacross()    const {errorClause("DEPEND(SOURCE|SINK)");
                                              return false;                   }
  virtual void setIsThreads(bool Flag)       {errorClause("THREADS/SIMD");  }
  virtual bool getIsThreads()          const {errorClause("THREADS/SIMD");
                                              return false;                   }
  virtual void setLpriv(LastprivateClause *L)
                          {errorClause(QUAL_OMP_LASTPRIVATE);                 }
  virtual LastprivateClause *getLpriv()const
                          {errorClause(QUAL_OMP_LASTPRIVATE); return nullptr; }

  virtual void setLinear(LinearClause *L)    {errorClause(QUAL_OMP_LINEAR);   }
  virtual LinearClause *getLinear()    const {errorClause(QUAL_OMP_LINEAR);
                                              return nullptr;                 }
  virtual void setLoop(Loop *L)              {errorClause("LOOP");            }
  virtual Loop *getLoop()              const {errorClause("LOOP");
                                              return nullptr;                 }
  virtual void setLoopInfo(LoopInfo *LI)     {errorClause("LoopInfo");        }
  virtual LoopInfo *getLoopInfo()      const {errorClause("LoopInfo");
                                              return nullptr;                 }
  virtual void setMap(MapClause *M)          {errorClause("MAP");             }
  virtual MapClause *getMap()          const {errorClause("MAP");
                                              return nullptr;                 }
  virtual void setMergeable(bool Flag)       {errorClause(QUAL_OMP_MERGEABLE);}
  virtual bool getMergeable()          const {errorClause(QUAL_OMP_MERGEABLE);
                                              return false;                   }
  virtual void setNogroup(bool Flag)         {errorClause(QUAL_OMP_NOGROUP);  }
  virtual bool getNogroup()            const {errorClause(QUAL_OMP_NOGROUP);
                                              return false;                   }
  virtual void setNowait(bool Flag)          {errorClause(QUAL_OMP_NOWAIT);   }
  virtual bool getNowait()             const {errorClause(QUAL_OMP_NOWAIT);
                                              return false;                   }
  virtual void setNumTasks(EXPR E)           {errorClause(QUAL_OMP_NUM_TASKS);}
  virtual EXPR getNumTasks()           const {errorClause(QUAL_OMP_NUM_TASKS);
                                              return nullptr;                 }
  virtual void setNumTeams(EXPR E)           {errorClause(QUAL_OMP_NUM_TEAMS);}
  virtual EXPR getNumTeams()           const {errorClause(QUAL_OMP_NUM_TEAMS);
                                              return nullptr;                 }
  virtual void setNumThreads(EXPR E)       {errorClause(QUAL_OMP_NUM_THREADS);}
  virtual EXPR getNumThreads()       const {errorClause(QUAL_OMP_NUM_THREADS);
                                              return nullptr;                 }
  virtual void setOrdered(int N)             {errorClause(QUAL_OMP_ORDERED);  }
  virtual int getOrdered()             const {errorClause(QUAL_OMP_ORDERED);
                                              return 0;                       }
  virtual void setPriority(EXPR E)           {errorClause(QUAL_OMP_PRIORITY); }
  virtual EXPR getPriority()           const {errorClause(QUAL_OMP_PRIORITY);
                                              return nullptr;                 }
  virtual void setPriv(PrivateClause *P)     {errorClause(QUAL_OMP_PRIVATE);  }
  virtual PrivateClause *getPriv()     const {errorClause(QUAL_OMP_PRIVATE);  
                                              return nullptr;                 }
  virtual void setProcBind(WRNProcBindKind P){errorClause("PROC_BIND");       }
  virtual WRNProcBindKind setProcBind()const {errorClause("PROC_BIND");       
                                              return WRNProcBindAbsent;       }
  virtual void setRed(ReductionClause *R)    {errorClause("REDUCTION");       }
  virtual ReductionClause *getRed()    const {errorClause("REDUCTION");       
                                              return nullptr;}
  virtual void setSafelen(int N)             {errorClause(QUAL_OMP_SAFELEN);  }
  virtual int getSafelen()             const {errorClause(QUAL_OMP_SAFELEN);  
                                              return 0;                       }
  virtual void setSchedule(ScheduleClause S) {errorClause("SCHEDULE");        }
  virtual ScheduleClause &getSchedule()      {errorClause("SCHEDULE");  
                                              llvm_unreachable("Bad clause"); }
  virtual void setShared(SharedClause *S)    {errorClause(QUAL_OMP_SHARED);   }
  virtual SharedClause *getShared()    const {errorClause(QUAL_OMP_SHARED);
                                              return nullptr;                 }
  virtual void setSimdlen(int N)             {errorClause(QUAL_OMP_SIMDLEN);  }
  virtual int getSimdlen()             const {errorClause(QUAL_OMP_SIMDLEN);
                                              return 0;                       }
  virtual void setThreadLimit(EXPR E)     {errorClause(QUAL_OMP_THREAD_LIMIT);}
  virtual EXPR getThreadLimit()     const {errorClause(QUAL_OMP_THREAD_LIMIT);
                                              return nullptr;                 }
  virtual void setUniform(UniformClause *U)  {errorClause(QUAL_OMP_UNIFORM);  }
  virtual UniformClause *getUniform()  const {errorClause(QUAL_OMP_UNIFORM);
                                              return nullptr;                 }
  virtual void setUntied(bool Flag)          {errorClause(QUAL_OMP_UNTIED);   }
  virtual bool getUntied()             const {errorClause(QUAL_OMP_UNTIED);
                                              return false;                   }
  virtual void setUseDevicePtr(UseDevicePtrClause *UDP)
                        {errorClause(QUAL_OMP_USE_DEVICE_PTR);                }
  virtual UseDevicePtrClause *getUseDevicePtr() const
                        {errorClause(QUAL_OMP_USE_DEVICE_PTR); return nullptr;}
  // TODO: complete the list as we implement more WRN kinds
  

  /// Only these classes are allowed to create/modify/delete WRegionNode.
  friend class WRegionUtils;
  friend class WRegionCollection;  //temporary
  friend class VPOParoptTransform;

#if 1
  // WRegionNodes are destroyed in bulk using WRegionUtils::destroyAll()
  virtual ~WRegionNode() { Children.clear(); }
#else
  virtual ~WRegionNode() {}
#endif

  // Virtual Clone Method
  // virtual WRegionNode *clone() const = 0;

  /// \brief Returns the unique number associated with this WRegionNode.
  unsigned getNumber() const { return Number; }

  /// \brief Returns the nesting level of this WRegionNode.
  unsigned getLevel() const { return Level; }

  /// \brief Returns the flag that indicates if WRN came from HIR
  bool getIsFromHIR() const { return IsFromHIR; }

  /// \brief Dumps WRegionNode.
  void dump() const;

  /// \brief Prints WRegionNode.
  //  Actual code from derived class only
  virtual void print(formatted_raw_ostream &OS, unsigned Depth) const = 0;

  /// \brief Prints WRegionNode children.
  void printChildren(formatted_raw_ostream &OS, unsigned Depth) const;

  /// \brief Returns the predecessor bblock of this region.
  BasicBlock *getPredBBlock() const;

  /// \brief Returns the successor bblock of this region.
  BasicBlock *getSuccBBlock() const;

  /// \brief Returns the immediate enclosing parent of the WRegionNode.
  WRegionNode *getParent() const { return Parent; }

  /// \brief Children iterator methods
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

  /// \brief Returns true if it has children.
  bool hasChildren() const { return !Children.empty(); }

  /// \brief Returns the number of children.
  unsigned getNumChildren() const { return Children.size(); }

  /// \brief Returns the Children container (by ref)
  WRContainerImpl &getChildren() { return Children ; }

  /// \brief Returns the first child if it exists, otherwise returns null.
  WRegionNode *getFirstChild();

  /// \brief Returns the last child if it exists, otherwise returns null.
  WRegionNode *getLastChild();

  /// \brief Returns an ID for the concrete type of this object.
  ///
  /// This is used to implement the classof checks in LLVM and should't
  /// be used for any other purpose.
  unsigned getWRegionKindID() const { return SubClassID; }

  /// \brief Returns the name for this WRN based on its SubClassID
  StringRef getName() const;

  // Methods for BBlockSet

  /// \brief Returns the entry(first) bblock of this region.
  BasicBlock *getEntryBBlock() const { return EntryBBlock; }

  /// \brief Returns the exit(last) bblock of this region.
  BasicBlock *getExitBBlock() const { return ExitBBlock; }

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

  bool contains(BasicBlock *BB) {
    return std::count(BBlockSet.begin(), BBlockSet.end(), BB);
  }

  /// \brief Returns True if BasicBlockSet is empty.
  unsigned isBBSetEmpty() const { return BBlockSet.empty(); }

  /// \brief Returns the number of BasicBlocks in BBlockSet.
  unsigned getBBSetSize() const { return BBlockSet.size(); }

  /// \brief Populates BBlockSet with BBs in the WRN from EntryBB to ExitBB.
  void populateBBSet();

  void resetBBSet() { BBlockSet.clear(); }

  /// \brief Routines to set WRN primary attributes
  void setAttributes(unsigned A) { Attributes = A; }
  void setIsDistribute()         { Attributes |= WRNIsDistribute; }
  void setIsPar()                { Attributes |= WRNIsPar; }
  void setIsLoop()               { Attributes |= WRNIsLoop; }
  void setIsTarget()             { Attributes |= WRNIsTarget; }
  void setIsTask()               { Attributes |= WRNIsTask; }
  void setIsTeams()              { Attributes |= WRNIsTeams; }

  /// \brief Routines to get WRN primary attributes
  unsigned getAttributes() const { return Attributes; }
  bool getIsDistribute()   const { return Attributes & WRNIsDistribute; }
  bool getIsPar()          const { return Attributes & WRNIsPar; }
  bool getIsLoop()         const { return Attributes & WRNIsLoop; }
  bool getIsTarget()       const { return Attributes & WRNIsTarget; }
  bool getIsTask()         const { return Attributes & WRNIsTask; }
  bool getIsTeams()        const { return Attributes & WRNIsTeams; }

  /// \brief Routines to get WRN derived attributes
  bool getIsParLoop()      const { return  getIsPar()  && getIsLoop(); }
  bool getIsTaskloop()     const { return  getIsTask() && getIsLoop(); }
  bool getIsWksLoop()      const { return !getIsTask() && getIsLoop(); }

  /// \brief Routines to set/get DirID
  void setDirID(int ID)          { DirID = ID; }
  int  getDirID()          const { return DirID; }

  // Derived Class Enumeration

  /// \brief An enumeration to keep track of the concrete subclasses of 
  /// WRegionNode
  enum WRegionNodeKind{
                                      // WRNAttribute:
    // These require outlining:

    WRNParallel,                      // IsPar
    WRNParallelLoop,                  // IsPar, IsLoop
    WRNParallelSections,              // IsPar, IsLoop
    WRNParallelWorkshare,             // IsPar, IsLoop
    WRNTeams,                         // IsTeams
    WRNDistributeParLoop,             // IsPar, IsLoop, IsDistribute
    WRNTarget,                        // IsTarget, IsTask (if depend/nowait)
    WRNTargetData,                    // IsTarget, IsTask (if depend/nowait)
    WRNTask,                          // IsTask
    WRNTaskloop,                      // IsTask, IsLoop

    // These don't require outlining:

    WRNVecLoop,                       // IsLoop
    WRNWksLoop,                       // IsLoop
    WRNSections,                      // IsLoop
    WRNWorkshare,                     // IsLoop
    WRNDistribute,                    // IsLoop, IsDistribute
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

  /// \brief WRN primary attributes
  enum WRNAttributes : uint32_t {
    WRNIsDistribute = 0x00000001,
    WRNIsPar        = 0x00000002,
    WRNIsLoop       = 0x00000004,
    WRNIsTarget     = 0x00000008,
    WRNIsTask       = 0x00000010,
    WRNIsTeams      = 0x00000020
  };
}; // class WRegionNode

} // End vpo namespace

} // End llvm namespace

#endif
