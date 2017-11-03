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
  unsigned SubClassID;

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
  bool hasSchedule() const;
  bool hasShared() const;
  bool hasPrivate() const;
  bool hasFirstprivate() const;
  bool hasLastprivate() const;
  bool hasReduction() const;
  bool hasCopyin() const;
  bool hasCopyprivate() const;
  bool hasLinear() const;
  bool hasUniform() const;
  bool hasMap() const;
  bool hasIsDevicePtr() const;
  bool hasUseDevicePtr() const;
  bool hasDepend() const;
  bool hasDepSink() const;
  bool hasAligned() const;
  bool hasFlush() const;
  
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
  virtual FirstprivateClause &getFpriv()     {WRNERROR(QUAL_OMP_FIRSTPRIVATE);}
  virtual FlushSet &getFlush()               {WRNERROR(QUAL_OMP_FLUSH);       }
  virtual IsDevicePtrClause &getIsDevicePtr()
                                            {WRNERROR(QUAL_OMP_IS_DEVICE_PTR);}
  virtual LastprivateClause &getLpriv()      {WRNERROR(QUAL_OMP_LASTPRIVATE); }
  virtual LinearClause &getLinear()          {WRNERROR(QUAL_OMP_LINEAR);      }
  virtual MapClause &getMap()                {WRNERROR("MAP");                }
  virtual PrivateClause &getPriv()           {WRNERROR(QUAL_OMP_PRIVATE);     }
  virtual ReductionClause &getRed()          {WRNERROR("REDUCTION");          }
  virtual ScheduleClause &getSchedule()      {WRNERROR("SCHEDULE");           }
       // ScheduleClause is not list-type, but has similar API so put here too
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
  virtual const ReductionClause &getRed() const
                                           {WRNERROR("REDUCTION");          }
  virtual const ScheduleClause &getSchedule() const
                                           {WRNERROR("SCHEDULE");           }
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
  virtual void setIsDepSource(bool F)           {WRNERROR("DEPEND(SOURCE)");  }
  virtual bool getIsDepSource()           const {WRNERROR("DEPEND(SOURCE)");  }
  virtual void setIsDoacross(bool F)         {WRNERROR("DEPEND(SOURCE|SINK)");}
  virtual bool getIsDoacross()         const {WRNERROR("DEPEND(SOURCE|SINK)");}
  virtual void setIsThreads(bool Flag)          {WRNERROR("THREADS/SIMD");    }
  virtual bool getIsThreads()             const {WRNERROR("THREADS/SIMD");    }
  virtual void setMergeable(bool Flag)          {WRNERROR(QUAL_OMP_MERGEABLE);}
  virtual bool getMergeable()             const {WRNERROR(QUAL_OMP_MERGEABLE);}
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
  virtual void setOrdered(int N)                {WRNERROR(QUAL_OMP_ORDERED);  }
  virtual int getOrdered()                const {WRNERROR(QUAL_OMP_ORDERED);  }
  virtual void setPriority(EXPR E)              {WRNERROR(QUAL_OMP_PRIORITY); }
  virtual EXPR getPriority()              const {WRNERROR(QUAL_OMP_PRIORITY); }
  virtual void setProcBind(WRNProcBindKind P)   {WRNERROR("PROC_BIND");       }
  virtual WRNProcBindKind setProcBind()   const {WRNERROR("PROC_BIND");       }
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

  // Loop & LoopInfo

  virtual void setLoop(Loop *L)                 {WRNERROR("LOOP");            }
  virtual Loop *getLoop()                 const {WRNERROR("LOOP");            }
  virtual void setLoopInfo(LoopInfo *LI)        {WRNERROR("LoopInfo");        }
  virtual LoopInfo *getLoopInfo()         const {WRNERROR("LoopInfo");        }

  // Task

  virtual void setTaskFlag(unsigned F)          {WRNERROR("TASK FLAG");       }
  virtual unsigned getTaskFlag()          const {WRNERROR("TASK FLAG");       }

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
  void dump(bool Verbose=false) const;

  /// \brief Default printer for WRegionNode. The derived WRegion can define
  /// its own print() routine to override this one.
  virtual void print(formatted_raw_ostream &OS, unsigned Depth, 
                     bool Verbose=false) const;

  /// \brief Prints "BEGIN  <DIRECTIVE_NAME> {"
  void printBegin(formatted_raw_ostream &OS, unsigned Depth) const;

  /// \brief Prints "} END  <DIRECTIVE_NAME>" 
  void printEnd(formatted_raw_ostream &OS, unsigned Depth) const;

  /// \brief This virtual function is intended for derived WRNs to print
  /// additional information specific to the derived WRN not covered by
  /// printBody() below.
  virtual void printExtra(formatted_raw_ostream &OS, unsigned Depth, 
                          bool Verbose=false) const {}

  /// \brief Prints content of the WRegionNode. 
  void printBody(formatted_raw_ostream &OS, bool PrintChildren, unsigned Depth, 
                 bool Verbose=false) const;

  /// \brief Prints content of list-type clauses in the WRN
  void printClauses(formatted_raw_ostream &OS, unsigned Depth, 
                    bool Verbose=false) const;

  /// \brief Prints EntryBB, ExitBB, and BBlockSet
  void printEntryExitBB(formatted_raw_ostream &OS, unsigned Depth, 
                        bool Verbose=false) const;

  /// \brief When IsFromHIR==true, prints EntryHLNode, ExitHLNode, and HLLoop
  /// This is virtual here; the derived WRNs supporting HIR have to provide the
  /// actual routine. Currently only WRNVecLoopNode uses HIR.
  virtual void printHIR(formatted_raw_ostream &OS, unsigned Depth,
                        bool Verbose=false) const {}

  /// \brief If IsOmpLoop==true, prints loop preheader, header, and latch BBs
  void printLoopBB(formatted_raw_ostream &OS, unsigned Depth,
                   bool Verbose=false) const;

  /// \brief Prints WRegionNode children.
  void printChildren(formatted_raw_ostream &OS, unsigned Depth, 
                     bool Verbose=false) const;

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
  void setWRegionKindID(unsigned ID) { SubClassID = ID; }

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
  void populateBBSetIfEmpty();

  void resetBBSet() { BBlockSet.clear(); }

  /// \brief Routines to set WRN primary attributes
  void setAttributes(unsigned A) { Attributes = A; }
  void setIsDistribute()         { Attributes |= WRNIsDistribute; }
  void setIsPar()                { Attributes |= WRNIsPar; }
  void setIsOmpLoop()            { Attributes |= WRNIsOmpLoop; }
  void setIsTarget()             { Attributes |= WRNIsTarget; }
  void setIsTask()               { Attributes |= WRNIsTask; }
  void setIsTeams()              { Attributes |= WRNIsTeams; }

  /// \brief Routines to get WRN primary attributes
  unsigned getAttributes() const { return Attributes; }
  bool getIsDistribute()   const { return Attributes & WRNIsDistribute; }
  bool getIsPar()          const { return Attributes & WRNIsPar; }
  bool getIsOmpLoop()      const { return Attributes & WRNIsOmpLoop; }
  bool getIsTarget()       const { return Attributes & WRNIsTarget; }
  bool getIsTask()         const { return Attributes & WRNIsTask; }
  bool getIsTeams()        const { return Attributes & WRNIsTeams; }

  /// \brief Routines to get WRN derived attributes
  bool getIsParLoop()      const { return  getIsPar()  && getIsOmpLoop(); }
  bool getIsTaskloop()     const { return  getIsTask() && getIsOmpLoop(); }
  bool getIsWksLoop()      const { return !getIsTask() && getIsOmpLoop(); }

  /// \brief Routine to check if the WRN needs global thread-id during codegen.
  /// Currently only SIMD and FLUSH constructs don't need the thread-id.
  bool needsTID()          const { return SubClassID != WRNVecLoop &&
                                          SubClassID != WRNFlush; }

  /// \brief Routine to check if the WRN needs the BID during codegen.
  /// The BID is the second parameter in a parallel entry, so this routine
  /// is equivalent to getIsPar(). In other words, it is true only for PARALLEL
  /// directives and combined/composite directives that have the PARALLEL
  /// keyword.
  bool needsBID()          const { return getIsPar(); }

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
    WRNParallelLoop,                  // IsPar, IsOmpLoop
    WRNParallelSections,              // IsPar, IsOmpLoop
    WRNParallelWorkshare,             // IsPar, IsOmpLoop
    WRNTeams,                         // IsTeams
    WRNDistributeParLoop,             // IsPar, IsOmpLoop, IsDistribute
    WRNTarget,                        // IsTarget, IsTask (if depend/nowait)
    WRNTargetData,                    // IsTarget, IsTask (if depend/nowait)
    WRNTask,                          // IsTask
    WRNTaskloop,                      // IsTask, IsOmpLoop

    // These don't require outlining:

    WRNVecLoop,                       // IsOmpLoop
    WRNWksLoop,                       // IsOmpLoop
    WRNSections,                      // IsOmpLoop
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

  /// \brief WRN primary attributes
  enum WRNAttributes : uint32_t {
    WRNIsDistribute = 0x00000001,
    WRNIsPar        = 0x00000002,
    WRNIsOmpLoop    = 0x00000004,
    WRNIsTarget     = 0x00000008,
    WRNIsTask       = 0x00000010,
    WRNIsTeams      = 0x00000020
  };
}; // class WRegionNode

} // End vpo namespace

} // End llvm namespace

#endif
