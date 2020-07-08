#if INTEL_COLLAB
//===-- WRegionNode.cpp - Implements the WRegionNode class ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//   This file implements the WRegionNode class.
//   It's the base class for WRN graph nodes, and should never be
//   instantiated directly.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/CFG.h"
#include "llvm/IR/Dominators.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionNode.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegion.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionUtils.h"

#define DEBUG_TYPE "vpo-wrnnode"

using namespace llvm;
using namespace llvm::vpo;

unsigned WRegionNode::UniqueNum(0);

DenseMap<int, StringRef> llvm::vpo::WRNName = {
    {WRegionNode::WRNParallel, "parallel"},
    {WRegionNode::WRNParallelLoop, "parallel loop"},
    {WRegionNode::WRNParallelSections, "parallel sections"},
    {WRegionNode::WRNParallelWorkshare, "parallel workshare"},
    {WRegionNode::WRNTeams, "teams"},
    {WRegionNode::WRNDistributeParLoop, "distribute parallel loop"},
    {WRegionNode::WRNTarget, "target"},
    {WRegionNode::WRNTargetData, "target data"},
    {WRegionNode::WRNTargetEnterData, "target enter data"},
    {WRegionNode::WRNTargetExitData, "target exit data"},
    {WRegionNode::WRNTargetUpdate, "target update"},
    {WRegionNode::WRNTargetVariant, "target variant dispatch"},
    {WRegionNode::WRNTask, "task"},
    {WRegionNode::WRNTaskloop, "taskloop"},
    {WRegionNode::WRNVecLoop, "simd"},
    {WRegionNode::WRNWksLoop, "loop"},
    {WRegionNode::WRNSections, "sections"},
    {WRegionNode::WRNGenericLoop, "generic loop"},
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
    {WRegionNode::WRNSection, "section"},
#endif // INTEL_FEATURE_CSA
#endif // INTEL_CUSTOMIZATION
    {WRegionNode::WRNWorkshare, "workshare"},
    {WRegionNode::WRNDistribute, "distribute"},
    {WRegionNode::WRNAtomic, "atomic"},
    {WRegionNode::WRNBarrier, "barrier"},
    {WRegionNode::WRNCancel, "cancel"},
    {WRegionNode::WRNCritical, "critical"},
    {WRegionNode::WRNFlush, "flush"},
    {WRegionNode::WRNOrdered, "ordered"},
    {WRegionNode::WRNMaster, "master"},
    {WRegionNode::WRNSingle, "single"},
    {WRegionNode::WRNTaskgroup, "taskgroup"},
    {WRegionNode::WRNTaskwait, "taskwait"},
    {WRegionNode::WRNTaskyield, "taskyield"}};

// constructor for LLVM IR representation
WRegionNode::WRegionNode(unsigned SCID, BasicBlock *BB)
    : SubClassID(SCID), Attributes(0), EntryBBlock(BB) {
  setEntryDirective(nullptr);
  setExitDirective(nullptr);
  setNextNumber();
  setParent(nullptr);
  setExitBBlock(nullptr);
#if INTEL_CUSTOMIZATION
  setIsFromHIR(false);
#endif // INTEL_CUSTOMIZATION
  resetBBSet();
}

#if INTEL_CUSTOMIZATION
// constructor for HIR representation
WRegionNode::WRegionNode(unsigned SCID) : SubClassID(SCID), Attributes(0) {
  setNextNumber();
  setParent(nullptr);
  setEntryBBlock(nullptr);
  setEntryDirective(nullptr);
  setExitDirective(nullptr);
  setExitBBlock(nullptr);
  resetBBSet();
  setIsFromHIR(true);
}
#endif // INTEL_CUSTOMIZATION

/// Wrap up the WRN creation now that we have the ExitDir. Perform these
/// tasks to finalize the WRN construction:
/// 1. Update the WRN's ExitDir and ExitBB
/// 2. Some clause operands appear in multiple clauses (eg firstprivate and
//     lastprivate). Mark the affected ClauseItems accordingly.
/// 3. If the WRN is for a loop construct:
///    3a. Find the associated Loop from the LoopInfo.
///    3b. If the WRN is a taskloop, set its SchedCode for grainsize/numtasks.
void WRegionNode::finalize(Instruction *ExitDir, DominatorTree *DT) {
  setExitDirective(ExitDir);
  BasicBlock *ExitBB = ExitDir->getParent();
  setExitBBlock(ExitBB);

  // Firstprivate and lastprivate clauses may have the same item X
  // Firstprivate and map clauses may have the same item Y
  // Update the IsInFirstprivate/Lastprivate/Map flags of the clauses
  bool hasLastprivate = (canHaveLastprivate() && !getLpriv().empty());
  bool hasMap = (canHaveMap() && !getMap().empty());
  bool hasUDP = (canHaveUseDevicePtr() && !getUseDevicePtr().empty());
  if ((hasLastprivate || hasMap) && canHaveFirstprivate()) {
    for (FirstprivateItem *FprivI : getFpriv().items()) {
      Value *Orig = FprivI->getOrig();
      if (hasLastprivate) {
        LastprivateItem *LprivI =
                              WRegionUtils::wrnSeenAsLastprivate(this, Orig);
        if (LprivI != nullptr) {
           // Orig appears in both firstprivate and lastprivate clauses
           FprivI->setInLastprivate(LprivI);
           LprivI->setInFirstprivate(FprivI);
           LLVM_DEBUG(dbgs() << "Found (" << *Orig
                             << ") in both Firstprivate and Lastprivate\n");
        }
      }
      if (hasMap) {
        MapItem *MapI = WRegionUtils::wrnSeenAsMap(this, Orig);
        if (MapI != nullptr) {
           // Orig appears in both firstprivate and map clauses
           FprivI->setInMap(MapI);
           MapI->setInFirstprivate(FprivI);
           LLVM_DEBUG(dbgs() << "Found (" << *Orig
                             << ") in both Firstprivate and Map\n");
        }
      }
    }
  }

  // UseDevicePtr clause items might also be present in Map clause. Update
  // InMap/InUseDevicePtr fields for them.
  if (hasUDP && hasMap) {
    for (UseDevicePtrItem *UDPI : getUseDevicePtr().items()) {
      // For use_device_ptr:ptr_to_ptr(i32** %x), the mapped value needs to be
      // a load from %x (e.g %x.val = load i32*, i32** %x), and not %x itself.
      // It's not reliable to search for a map on a load, using %x. So, we skip
      // it. We'll create another map for this case in
      // VPOParoptTransform::addMapForUseDevicePtr().
      if (UDPI->getIsPointerToPointer())
        continue;

      Value *Orig = UDPI->getOrig();
      MapItem *MapI = WRegionUtils::wrnSeenAsMap(this, Orig);
      if (!MapI)
        continue;

      // Orig appears in both use_device_ptr and map clauses
      UDPI->setInMap(MapI);
      MapI->setInUseDevicePtr(UDPI);
      LLVM_DEBUG(dbgs() << "Found (" << *Orig
                        << ") in both UseDevicePtr and Map\n");
    }
  }

  // Update the InAllocate fields of [first|last]private and reduction
  bool hasAllocate = (canHaveAllocate() && !getAllocate().empty());
  if (hasAllocate) {
    bool hasPrivate = (canHavePrivate() && !getPriv().empty());
    bool hasFirstprivate = (canHaveFirstprivate() && !getFpriv().empty());
    // hasLastprivate is already computed
    bool hasReduction = (canHaveReduction() && !getRed().empty());
    for (AllocateItem *AllocI : getAllocate().items()) {
      Value *Orig = AllocI->getOrig();

      if (hasPrivate) {
        PrivateItem *PrivI = WRegionUtils::wrnSeenAsPrivate(this, Orig);
        if (PrivI) {
          PrivI->setInAllocate(AllocI);
          LLVM_DEBUG(dbgs() << "Found (" << *Orig
                            << ") in both Allocate and Private\n");
          continue;
        }
      }

      if (hasFirstprivate) {
        FirstprivateItem *FprivI =
            WRegionUtils::wrnSeenAsFirstprivate(this, Orig);
        if (FprivI) {
          FprivI->setInAllocate(AllocI);
          LastprivateItem *LprivI = nullptr;
          if (hasLastprivate) {
            // Update lastprivate clause if Orig also appears in it
            LprivI = FprivI->getInLastprivate();
            if (LprivI)
              LprivI->setInAllocate(AllocI);
          }
          if (LprivI)
            LLVM_DEBUG(dbgs() << "Found (" << *Orig
                              << ") in both Allocate and Firstprivate\n");
          else
            LLVM_DEBUG(dbgs()
                       << "Found (" << *Orig
                       << ") in Allocate, Firstprivate, and Lastprivate\n");
          continue;
        }
      }

      if (hasLastprivate) {
        LastprivateItem *LprivI =
            WRegionUtils::wrnSeenAsLastprivate(this, Orig);
        if (LprivI) {
          LprivI->setInAllocate(AllocI);
          LLVM_DEBUG(dbgs() << "Found (" << *Orig
                            << ") in both Allocate and Lastprivate\n");
          continue;
        }
      }

      if (hasReduction) {
        ReductionItem *RedI = WRegionUtils::wrnSeenAsReduction(this, Orig);
        if (RedI) {
          RedI->setInAllocate(AllocI);
          LLVM_DEBUG(dbgs() << "Found (" << *Orig
                            << ") in both Allocate and Reduction\n");
        }
      }
    } // for (AllocateItem *AllocI : getAllocate().items())
  }   // if (hastAllocate)

  if (getIsOmpLoop()) {
    LoopInfo *LI = getWRNLoopInfo().getLoopInfo();
    assert(LI && "LoopInfo not present in a loop construct");
    BasicBlock *EntryBB = getEntryBBlock();
    Loop *Lp = GeneralUtils::getLoopFromLoopInfo(LI, DT, EntryBB, ExitBB);

    // Do not assert for loop-type constructs when Lp==NULL because transforms
    // before Paropt may have optimized away the loop.
    getWRNLoopInfo().setLoop(Lp);

    if (Lp)
      LLVM_DEBUG(dbgs() << "\n=== finalize WRN: found loop : " << *Lp << "\n");
    else
      LLVM_DEBUG(
          dbgs() << "\n=== finalize WRN: loop not found. Optimized away?\n");

    // For taskloop, the runtime has a parameter for either Grainsize or
    // NumTasks, which is chosen by the parameter SchedCode:
    //   SchedCode==1 means Grainsize is used
    //   SchedCode==2 means NumTasks is used
    //   SchedCode==0 means neither is used
    // If both Grainsize and NumTasks are specified, then Grainsize prevails.
    if (getWRegionKindID() == WRNTaskloop) {
      if (getGrainsize() != nullptr)
        setSchedCode(1);
      else if (getNumTasks() != nullptr)
        setSchedCode(2);
      else
        setSchedCode(0);
    }
#if INTEL_CUSTOMIZATION

    // For OpenCL, the vectorizer requires that the second operand of
    // __read_pipe_2_bl_intel() be privatized. The code below will look at
    // each occurrence of such a call in the WRN, and find the corresponding
    // AllocaInst of its second operand. If the Alloca is outside of the WRN,
    // then we add it to the PRIVATE list so it will be privatized in the
    // VPOParoptPrepare phase.
    if (getWRegionKindID() == WRNVecLoop) {
      PrivateClause &PC = getPriv();
      populateBBSet();
      for (BasicBlock *BB : BBlockSet)
        for (Instruction &I : *BB)
          if (VPOAnalysisUtils::isCallOfName(&I, "__read_pipe_2_bl_intel")) {
            CallInst *Call = cast<CallInst>(&I);
            // LLVM_DEBUG(dbgs() << "Found Call: " << *Call << "\n");
            assert(Call->getNumArgOperands()==2 &&
                   "__read_pipe_2_bl_intel() is expected to have 2 operands");
            Value *V = Call->getArgOperand(1); // second operand
            AllocaInst *Alloca = VPOAnalysisUtils::findAllocaInst(V);
            assert (Alloca &&
                    "Alloca not found for __read_pipe_2_bl_intel operand");
            if (Alloca) {
              // LLVM_DEBUG(dbgs() << "Found Alloca: " << *Alloca << "\n");
              if (!contains(Alloca->getParent())) {
                // Alloca is outside of the WRN, so privatize it
                PC.add(Alloca);
                // LLVM_DEBUG(dbgs() << "Will privatize: " << *Alloca << "\n");
              }
              // else do nothing: the alloca is inside the WRN hence it is
              // already private
            }
          }
      resetBBSet();
    }
#endif // INTEL_CUSTOMIZATION
  } // if (getIsOmpLoop())

  // All target constructs except for "target data" are task-generating
  // constructs. Furthermore, when the construct has a nowait or depend clause,
  // then the resulting task is not undeferred (ie, asynchronous offloading).
  // We want to set the "IsTask" attribute of these target constructs to
  // facilitate code generation.
  if (getIsTarget() && getWRegionKindID() != WRNTargetData &&
                    getWRegionKindID() != WRNTargetVariant) {
    assert(canHaveDepend() && "Corrupt WRN? Depend Clause should be allowed");
    if (getNowait() || !getDepend().empty()) {
      // TODO: turn on this code after verifying that task codegen supports it
      // setIsTask();
    }
  }
}

// Populates BBlockSet with BBs in the WRN from EntryBB to ExitBB.
bool WRegionNode::populateBBSet(bool Always) {
  BasicBlock *EntryBB = getEntryBBlock();
  BasicBlock *ExitBB = getExitBBlock();

  assert(EntryBB && "Missing EntryBB!");
  assert(ExitBB && "Missing ExitBB!");

  if (Always || isBBSetEmpty()) {
    resetBBSet();
    GeneralUtils::collectBBSet(EntryBB, ExitBB, BBlockSet);
    return true;
  }

  return false;
}

// After CFGRestructuring, the EntryBB should have a single predecessor
BasicBlock *WRegionNode::getPredBBlock() const {
  auto PredI = pred_begin(EntryBBlock);
  auto TempPredI = PredI;
  ++TempPredI;
  assert((TempPredI == pred_end(EntryBBlock)) &&
         "Region has more than one predecessor!");
  return *PredI;
}

// After CFGRestructuring, the ExitBB should have a single successor
BasicBlock *WRegionNode::getSuccBBlock() const {
  auto SuccI = succ_begin(ExitBBlock);
  auto TempSuccI = SuccI;
  ++TempSuccI;

  assert((TempSuccI == succ_end(ExitBBlock)) &&
         "Region has more than one successor!");
  return *SuccI;
}

WRegionNode *WRegionNode::getFirstChild() {
  if (hasChildren()) {
    return *(Children.begin());
  }
  return nullptr;
}

WRegionNode *WRegionNode::getLastChild() {
  if (hasChildren()) {
    return Children.back();
  }
  return nullptr;
}

// Default print() routine for WRegionNode. This routine is invoked for
// printing the WRN unless the specialized WRegion defines its own print().
void WRegionNode::print(formatted_raw_ostream &OS, unsigned Depth,
                                                   unsigned Verbosity) const {
  // Print BEGIN <directive_name>
  printBegin(OS, Depth);

  // Print WRN contents specific to a given derived class. If the derived
  // class does not define printExtra(), then this does nothing.
  printExtra(OS, Depth+1, Verbosity);

  // Print WRN contents: Clauses, BBlocks, Loop, Children, etc.
  printBody(OS, true, Depth+1, Verbosity);

  // Print END <directive_name>
  printEnd(OS, Depth);
}

void WRegionNode::printBegin(formatted_raw_ostream &OS, unsigned Depth) const {
  int Id = getDirID();
  StringRef DirName = VPOAnalysisUtils::getOmpDirectiveName(Id);
  OS.indent(2*Depth) << "BEGIN " << DirName <<" ID=" << getNumber() << " {\n\n";
}

void WRegionNode::printEnd(formatted_raw_ostream &OS, unsigned Depth) const {
  int Id = getDirID();
  StringRef DirName = VPOAnalysisUtils::getOmpDirectiveName(Id);
  OS.indent(2*Depth) << "} END " << DirName <<" ID=" << getNumber() << "\n\n";
}

void WRegionNode::printBody(formatted_raw_ostream &OS, bool PrintChildren,
                            unsigned Depth, unsigned Verbosity) const {
  printClauses(OS, Depth, Verbosity);

#if INTEL_CUSTOMIZATION
  if (getIsFromHIR())
    printHIR(OS, Depth, Verbosity); // defined by derived WRN
  else
#endif // INTEL_CUSTOMIZATION
  {
    printEntryExitBB(OS, Depth, Verbosity);
    if (getIsOmpLoop())
      printLoopBB(OS, Depth, Verbosity);
  }

  if (PrintChildren)
    printChildren(OS, Depth, Verbosity);
}


void WRegionNode::printClauses(formatted_raw_ostream &OS,
                               unsigned Depth, unsigned Verbosity) const {
  bool PrintedSomething = false;

  if (canHaveDistSchedule())
    PrintedSomething |= getDistSchedule().print(OS, Depth, Verbosity);

  if (canHaveSchedule())
    PrintedSomething |= getSchedule().print(OS, Depth, Verbosity);

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
  if (canHaveWorkerSchedule())
    PrintedSomething |= getWorkerSchedule().print(OS, Depth, Verbosity);
#endif // INTEL_FEATURE_CSA
#endif // INTEL_CUSTOMIZATION

  if (canHaveShared())
    PrintedSomething |= getShared().print(OS, Depth, Verbosity);

  if (canHavePrivate())
    PrintedSomething |= getPriv().print(OS, Depth, Verbosity);

  if (canHaveFirstprivate())
    PrintedSomething |= getFpriv().print(OS, Depth, Verbosity);

  if (canHaveLastprivate())
    PrintedSomething |= getLpriv().print(OS, Depth, Verbosity);

  if (canHaveInReduction())
    PrintedSomething |= getInRed().print(OS, Depth, Verbosity);

  if (canHaveReduction())
    PrintedSomething |= getRed().print(OS, Depth, Verbosity);

  if (canHaveAllocate())
    PrintedSomething |= getAllocate().print(OS, Depth, Verbosity);

  if (canHaveCopyin())
    PrintedSomething |= getCopyin().print(OS, Depth, Verbosity);

  if (canHaveCopyprivate())
    PrintedSomething |= getCpriv().print(OS, Depth, Verbosity);

  if (canHaveLinear())
    PrintedSomething |= getLinear().print(OS, Depth, Verbosity);

  if (canHaveUniform())
    PrintedSomething |= getUniform().print(OS, Depth, Verbosity);

  if (canHaveMap())
    PrintedSomething |= getMap().print(OS, Depth, Verbosity);

  if (canHaveIsDevicePtr())
    PrintedSomething |= getIsDevicePtr().print(OS, Depth, Verbosity);

  if (canHaveUseDevicePtr())
    PrintedSomething |= getUseDevicePtr().print(OS, Depth, Verbosity);

  if (canHaveDepend())
    PrintedSomething |= getDepend().print(OS, Depth, Verbosity);

  if (canHaveDepSrcSink())
    PrintedSomething |= getDepSink().print(OS, Depth, Verbosity);

  if (canHaveDepSrcSink())
    PrintedSomething |= getDepSource().print(OS, Depth, Verbosity);

  if (canHaveAligned())
    PrintedSomething |= getAligned().print(OS, Depth, Verbosity);

  if (canHaveFlush())
    PrintedSomething |= getFlush().print(OS, Depth, Verbosity);

  if (PrintedSomething)
    OS << "\n";
}

// Verbosity <= 1:         print BB name for EntryBB/ExitBB
// Verbosity == 2: above + print BB content for EntryBB/ExitBB
// Verbosity == 3: above + print BB name for all BBs in BBSet
// Verbosity >= 4: above + print BB content for all BBs in BBSet
void WRegionNode::printEntryExitBB(formatted_raw_ostream &OS, unsigned Depth,
                                   unsigned Verbosity) const {
#if INTEL_CUSTOMIZATION
  if (getIsFromHIR()) // HIR representation; no BBs to print
    return;
#endif // INTEL_CUSTOMIZATION

  int Ind = 2*Depth;

  BasicBlock *EntryBB = getEntryBBlock();
  BasicBlock *ExitBB = getExitBBlock();
  assert (EntryBB && "Entry BB is null!");
  assert (ExitBB && "Exit BB is null!");

  vpo::printBB("EntryBB", EntryBB, OS, Ind, Verbosity);
  vpo::printBB("ExitBB",  ExitBB,  OS, Ind, Verbosity);

  if (Verbosity >=3) {
    OS.indent(Ind) << "BBSet";
    if (!isBBSetEmpty()) {
      OS << ":\n";
      for (BasicBlock *BB : BBlockSet ) {
        if (Verbosity == 3)
          OS.indent(Ind+2) << BB->getName() << "\n"; // print names only
        else // Verbosity >=4
          OS.indent(Ind+2) << *BB << "\n";           // print BB contents
      }
    } else
      OS << " is empty\n";
  }
  OS << "\n";
}

void WRegionNode::printLoopBB(formatted_raw_ostream &OS, unsigned Depth,
                              unsigned Verbosity) const {
  if (getIsOmpLoop())
    getWRNLoopInfo().print(OS, Depth, Verbosity);
}

void WRegionNode::printChildren(formatted_raw_ostream &OS, unsigned Depth,
                                unsigned Verbosity) const {
  for (WRegionNode *W : Children)
    W->print(OS, Depth, Verbosity);
}

void WRegionNode::destroy() {
  // TODO: call destructor
}

void WRegionNode::destroyAll() {
  // TODO: implement this by recursive walk from top
}

void WRegionNode::dump(unsigned Verbosity) const {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  formatted_raw_ostream OS(dbgs());
  print(OS, 0, Verbosity);
#endif
}

//
// functions below are used to update WRNs with clause information
//

void WRegionNode::parseClause(const ClauseSpecifier &ClauseInfo,
                              const Use *Args, unsigned NumArgs) {
  int ClauseID = ClauseInfo.getId();

  // Classify the clause based on the number of arguments allowed by the
  // clause, which can be 0, 1, or a list. The utility getClauseType()
  // returns one of these:
  //    0: for clauses that take no arguments
  //    1: for clauses that take one argument only
  //    2: all other clauses (includes those that take a list)
  unsigned ClauseNumArgs = VPOAnalysisUtils::getClauseType(ClauseID);

  if (ClauseNumArgs == 0) {
    // The clause takes no arguments
    assert(NumArgs == 0 && "This clause takes no arguments.");
    handleQual(ClauseInfo);
  } else if (ClauseNumArgs == 1) {
    // The clause takes one argument only
    assert(NumArgs == 1 && "This clause takes one argument.");
    Value *V = Args[0];
    handleQualOpnd(ClauseID, V);
  } else {
    // The clause takes a list of arguments
    assert(NumArgs >= 1 && "This clause takes one or more arguments.");
    handleQualOpndList(&Args[0], NumArgs, ClauseInfo);
  }
}

#if __GNUC__ >= 7
// The switch ladders below uses implicit fallthrough for compactness.
// Please note the ordering when adding cases.
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#endif

void WRegionNode::handleQual(const ClauseSpecifier &ClauseInfo) {
  int ClauseID = ClauseInfo.getId();
  switch (ClauseID) {
  case QUAL_OMP_DEFAULT_NONE:
    setDefault(WRNDefaultNone);
    break;
  case QUAL_OMP_DEFAULT_SHARED:
    setDefault(WRNDefaultShared);
    break;
  case QUAL_OMP_DEFAULT_PRIVATE:
    setDefault(WRNDefaultPrivate);
    break;
  case QUAL_OMP_DEFAULT_FIRSTPRIVATE:
    setDefault(WRNDefaultFirstprivate);
    break;
  case QUAL_OMP_DEFAULTMAP_ALLOC:
  case QUAL_OMP_DEFAULTMAP_TO:
  case QUAL_OMP_DEFAULTMAP_FROM:
  case QUAL_OMP_DEFAULTMAP_TOFROM:
  case QUAL_OMP_DEFAULTMAP_FIRSTPRIVATE:
  case QUAL_OMP_DEFAULTMAP_NONE:
  case QUAL_OMP_DEFAULTMAP_DEFAULT:
  case QUAL_OMP_DEFAULTMAP_PRESENT:
  {
    WRNDefaultmapBehavior Behavior =
        WRNDefaultmapBehaviorFromClauseID[ClauseID];
    WRNDefaultmapCategory Category;

    if (ClauseInfo.getIsAggregate())
      Category = WRNDefaultmapAggregate;
#if INTEL_CUSTOMIZATION
    else if (ClauseInfo.getIsAllocatable())
      Category = WRNDefaultmapAllocatable;
#endif // INTEL_CUSTOMIZATION
    else if (ClauseInfo.getIsPointer())
      Category = WRNDefaultmapPointer;
    else if (ClauseInfo.getIsScalar())
      Category = WRNDefaultmapScalar;
    else
      // No category specified means implicit behavior applies to all vars
      Category = WRNDefaultmapAllVars;

    setDefaultmap(Category, Behavior);
    break;
  }
  case QUAL_OMP_NOWAIT:
    setNowait(true);
    break;
  case QUAL_OMP_UNTIED:
    setUntied(true);
    setTaskFlag(getTaskFlag() & ~WRNTaskFlag::Tied);
    break;
  case QUAL_OMP_TARGET_TASK:
    setIsTargetTask(true);
    break;
  case QUAL_OMP_READ_SEQ_CST:
    setHasSeqCstClause(true);
  case QUAL_OMP_READ:
    setAtomicKind(WRNAtomicRead);
    break;
  case QUAL_OMP_WRITE_SEQ_CST:
    setHasSeqCstClause(true);
  case QUAL_OMP_WRITE:
    setAtomicKind(WRNAtomicWrite);
    break;
  case QUAL_OMP_UPDATE_SEQ_CST:
    setHasSeqCstClause(true);
  case QUAL_OMP_UPDATE:
    setAtomicKind(WRNAtomicUpdate);
    break;
  case QUAL_OMP_CAPTURE_SEQ_CST:
    setHasSeqCstClause(true);
  case QUAL_OMP_CAPTURE:
    setAtomicKind(WRNAtomicCapture);
    break;
  case QUAL_OMP_MERGEABLE:
    setMergeable(true);
    break;
  case QUAL_OMP_NOGROUP:
    setNogroup(true);
    break;
  case QUAL_OMP_PROC_BIND_MASTER:
    setProcBind(WRNProcBindMaster);
    break;
  case QUAL_OMP_PROC_BIND_CLOSE:
    setProcBind(WRNProcBindClose);
    break;
  case QUAL_OMP_PROC_BIND_SPREAD:
    setProcBind(WRNProcBindSpread);
    break;
  case QUAL_OMP_ORDERED_THREADS:
    setIsDoacross(false);
    setIsThreads(true);
    break;
  case QUAL_OMP_ORDERED_SIMD:
    setIsDoacross(false);
    setIsSIMD(true);
    break;
  case QUAL_OMP_CANCEL_PARALLEL:
    setCancelKind(WRNCancelParallel);
    break;
  case QUAL_OMP_CANCEL_LOOP:
    setCancelKind(WRNCancelLoop);
    break;
  case QUAL_OMP_CANCEL_SECTIONS:
    setCancelKind(WRNCancelSections);
    break;
  case QUAL_OMP_CANCEL_TASKGROUP:
    setCancelKind(WRNCancelTaskgroup);
    break;
  case QUAL_OMP_BIND_TEAMS:
    setLoopBind(WRNLoopBindTeams);
    break;
  case QUAL_OMP_BIND_PARALLEL:
    setLoopBind(WRNLoopBindParallel);
    break;
  case QUAL_OMP_BIND_THREAD:
    setLoopBind(WRNLoopBindThread);
    break;
  case QUAL_OMP_ORDER_CONCURRENT:
    setLoopOrder(WRNLoopOrderConcurrent);
    break;
  case QUAL_OMP_OFFLOAD_KNOWN_NDRANGE:
    getWRNLoopInfo().setKnownNDRange();
    break;
  default:
    llvm_unreachable("Unknown ClauseID in handleQual()");
  }
}

void WRegionNode::handleQualOpnd(int ClauseID, Value *V) {
  // for clauses whose parameter are constant integer exprs,
  // we store the information as an int rather than a Value*,
  // so we must extract the integer N from V and store N.
  int64_t N = -1;
  ConstantInt *CI = dyn_cast<ConstantInt>(V);
  if (CI != nullptr)
    N = CI->getZExtValue();

  switch (ClauseID) {
  case QUAL_OMP_SIMDLEN:
    assert(N > 0 && "SIMDLEN must be positive");
    setSimdlen(N);
    break;
  case QUAL_OMP_SAFELEN:
    assert(N > 0 && "SAFELEN must be positive");
    setSafelen(N);
    break;
  case QUAL_OMP_COLLAPSE:
    assert(N > 0 && "COLLAPSE parameter must be positive");
    setCollapse(N);
    break;
  case QUAL_OMP_OFFLOAD_ENTRY_IDX:
    assert(CI && "OFFLOAD_ENTRY_IDX expected to be constant");
    setOffloadEntryIdx(N);
    break;
  case QUAL_OMP_IF:
    setIf(V);
    break;
  case QUAL_OMP_NAME: {
    // The operand is expected to be a constant string. Example:
    // "QUAL.OMP.NAME"([9 x i8] c"lock_name")
    assert(isa<ConstantDataSequential>(V) &&
           "QUAL_OMP_NAME opnd is not constant data.");
    ConstantDataSequential *CD = cast<ConstantDataSequential>(V);

    if (CD->isCString()) // Process as C string first, so that the nul
                         // bytes at the end are ignored. (e.g. c"lock_name\00")
      setUserLockName(CD->getAsCString());
    else if (CD->isString()) // Process as a regular string. (e.g. c"lock_name")
      setUserLockName(CD->getAsString());
    else
      llvm_unreachable("QUAL_OMP_NAME opnd is not a string.");

  } break;
  case QUAL_OMP_NUM_THREADS:
    setNumThreads(V);
    break;
    break;
  case QUAL_OMP_FINAL:
    setFinal(V);
    break;
  case QUAL_OMP_GRAINSIZE:
    setGrainsize(V);
    break;
  case QUAL_OMP_NUM_TASKS:
    setNumTasks(V);
    break;
  case QUAL_OMP_PRIORITY:
    setPriority(V);
    break;
  case QUAL_OMP_NUM_TEAMS:
    setNumTeams(V);
    break;
  case QUAL_OMP_THREAD_LIMIT:
    setThreadLimit(V);
    break;
  case QUAL_OMP_DEVICE:
    setDevice(V);
    break;
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
  case QUAL_OMP_SA_NUM_WORKERS:
    setNumWorkers(N);
    break;
  case QUAL_OMP_SA_PIPELINE:
    setPipelineDepth(N);
    break;
#endif // INTEL_FEATURE_CSA
#endif // INTEL_CUSTOMIZATION
  default:
    llvm_unreachable("Unknown ClauseID in handleQualOpnd()");
  }
}

template <typename ClauseTy>
void WRegionNode::extractQualOpndList(const Use *Args, unsigned NumArgs,
                                      int ClauseID, ClauseTy &C) {
  C.setClauseID(ClauseID);

  for (unsigned I = 0; I < NumArgs; ++I) {
    Value *V = Args[I];
    C.add(V);
  }
}

template <typename ClauseTy>
void WRegionNode::extractQualOpndList(const Use *Args, unsigned NumArgs,
                                      const ClauseSpecifier &ClauseInfo,
                                      ClauseTy &C) {
  bool IsUseDeviceAddr = false;
  int ClauseID = ClauseInfo.getId();
  if (ClauseID == QUAL_OMP_USE_DEVICE_ADDR) {
    ClauseID = QUAL_OMP_USE_DEVICE_PTR;
    IsUseDeviceAddr = true;
  }
  C.setClauseID(ClauseID);
  bool IsByRef = ClauseInfo.getIsByRef();
  bool IsPointerToPointer = ClauseInfo.getIsPointerToPointer();
  for (unsigned I = 0; I < NumArgs; ++I) {
    Value *V = Args[I];
    C.add(V);
    if (IsByRef)
      C.back()->setIsByRef(true);
    if (IsPointerToPointer)
      C.back()->setIsPointerToPointer(true);
    if (IsUseDeviceAddr) {
      UseDevicePtrItem *UDPI = cast<UseDevicePtrItem>(C.back());
      UDPI->setIsUseDeviceAddr(true);
    }
#if INTEL_CUSTOMIZATION
    if (!CurrentBundleDDRefs.empty() &&
        WRegionUtils::supportsRegDDRefs(ClauseID))
      C.back()->setHOrig(CurrentBundleDDRefs[I]);
    if (ClauseInfo.getIsF90DopeVector())
      C.back()->setIsF90DopeVector(true);
    C.back()->setIsWILocal(ClauseInfo.getIsWILocal());
#endif // INTEL_CUSTOMIZATION
  }
}

template <typename ClauseItemTy>
void WRegionNode::extractQualOpndListNonPod(const Use *Args, unsigned NumArgs,
                                            const ClauseSpecifier &ClauseInfo,
                                            Clause<ClauseItemTy> &C) {
  int ClauseID = ClauseInfo.getId();
  C.setClauseID(ClauseID);

  bool IsByRef = ClauseInfo.getIsByRef();
  bool IsConditional = ClauseInfo.getIsConditional();
  if (IsConditional)
    assert(ClauseID == QUAL_OMP_LASTPRIVATE &&
           "The CONDITIONAL keyword is for LASTPRIVATE clauses only");

  if (ClauseInfo.getIsNonPod()) {
    // NONPOD representation requires multiple args per var:
    //  - PRIVATE:      3 args : Var, Ctor, Dtor
    //  - FIRSTPRIVATE: 3 args : Var, CCtor, Dtor
    //  - LASTPRIVATE:  4 args : Var, Ctor, CopyAssign, Dtor
    if (ClauseID == QUAL_OMP_PRIVATE || ClauseID == QUAL_OMP_FIRSTPRIVATE)
      assert(NumArgs == 3 && "Expected 3 arguments for [FIRST]PRIVATE NONPOD");
    else if (ClauseID == QUAL_OMP_LASTPRIVATE)
      assert(NumArgs == 4 && "Expected 4 arguments for LASTPRIVATE NONPOD");
    else
      llvm_unreachable("NONPOD support for this clause type TBD");

    if (!Args[0] || isa<ConstantPointerNull>(Args[0])) {
      LLVM_DEBUG(dbgs() << __FUNCTION__ << " Ignoring null clause operand.\n");
      return;
    }
    ClauseItemTy *Item = new ClauseItemTy(Args);
    Item->setIsByRef(IsByRef);
    Item->setIsNonPod(true);
    if (IsConditional)
      Item->setIsConditional(true);
#if INTEL_CUSTOMIZATION
    if (ClauseInfo.getIsF90DopeVector())
      Item->setIsF90DopeVector(true);
    Item->setIsWILocal(ClauseInfo.getIsWILocal());
#endif // INTEL_CUSTOMIZATION
    C.add(Item);
  } else
    for (unsigned I = 0; I < NumArgs; ++I) {
      Value *V = Args[I];
      if (!V || isa<ConstantPointerNull>(V)) {
        LLVM_DEBUG(dbgs() << __FUNCTION__
                          << " Ignoring null clause operand.\n");
        continue;
      }
      C.add(V);
      if (IsByRef)
        C.back()->setIsByRef(true);
      if (IsConditional)
        C.back()->setIsConditional(true);
#if INTEL_CUSTOMIZATION
    if (!CurrentBundleDDRefs.empty() &&
        WRegionUtils::supportsRegDDRefs(ClauseID))
      C.back()->setHOrig(CurrentBundleDDRefs[I]);
    if (ClauseInfo.getIsF90DopeVector())
      C.back()->setIsF90DopeVector(true);
    C.back()->setIsWILocal(ClauseInfo.getIsWILocal());
#endif // INTEL_CUSTOMIZATION
    }
}

void WRegionNode::extractScheduleOpndList(ScheduleClause &Sched,
                                          const Use *Args,
                                          const ClauseSpecifier &ClauseInfo,
                                          WRNScheduleKind Kind) {
  // save the schedule kind
  Sched.setKind(Kind);

  Value *ChunkArg = Args[0];  // chunk size expr

  // save the chunk size expr
  Sched.setChunkExpr(ChunkArg);

  // If ChunkExpr is a constant expression, extract the constant and save it
  // in ChunkSize, which is initialized to -1 (an invalid chunk size) to
  // signify that ChunkExpr is not constant.
  // Examples:
  //   User's clause:        Clang sends:            Extracted ChunkSize here:
  //     schedule(static)      schedule(static,0)      ChunkSize ==  0
  //     schedule(static,2)    schedule(static,2)      ChunkSize ==  2
  //     schedule(static,x)    schedule(static,%x)     ChunkSize == -1
  // Therefore, a negative ChunkSize means that the chunk expression is a
  // symbolic expr whose value is unkown at compile time.
  int64_t ChunkSize = -1;
  ConstantInt *CI = dyn_cast<ConstantInt>(ChunkArg);
  if (CI != nullptr) {
    ChunkSize = CI->getZExtValue();
    LLVM_DEBUG(dbgs() << " Schedule chunk size is constant: " << ChunkSize
                      << "\n");
  }
  Sched.setChunk(ChunkSize);

  // Save schedule modifier info
  Sched.setIsSchedMonotonic(ClauseInfo.getIsScheduleMonotonic());
  Sched.setIsSchedNonmonotonic(ClauseInfo.getIsScheduleNonmonotonic());
  Sched.setIsSchedSimd(ClauseInfo.getIsScheduleSimd());

  // TODO: define the print() method for ScheduleClause to print stuff below
  LLVM_DEBUG(dbgs() << "=== " << ClauseInfo.getBaseName());
  LLVM_DEBUG(dbgs() << "  Chunk=" << *Sched.getChunkExpr());
  LLVM_DEBUG(dbgs() << "  Monotonic=" << Sched.getIsSchedMonotonic());
  LLVM_DEBUG(dbgs() << "  Nonmonotonic=" << Sched.getIsSchedNonmonotonic());
  LLVM_DEBUG(dbgs() << "  Simd=" << Sched.getIsSchedSimd() << "\n");

  return;
}

void WRegionNode::extractMapOpndList(const Use *Args, unsigned NumArgs,
                                     const ClauseSpecifier &ClauseInfo,
                                     MapClause &C, unsigned MapKind) {
  C.setClauseID(QUAL_OMP_MAP_TO); // dummy map clause id; details are in
                                  // the MapKind of each list item

  // Get map-type modifiers (always, close, present) from ClauseInfo
  if (ClauseInfo.getIsAlways())
    MapKind |= MapItem::WRNMapAlways;
  if (ClauseInfo.getIsClose())
    MapKind |= MapItem::WRNMapClose;
  if (ClauseInfo.getIsPresent())
    MapKind |= MapItem::WRNMapPresent;

  if (ClauseInfo.getIsArraySection()) {
    assert ((MapKind & MapItem::WRNMapUpdateTo ||
             MapKind & MapItem::WRNMapUpdateFrom) &&
             "Expected Map Chain instead of Array Section in a MAP clause");
    Value *V = Args[0];
    C.add(V);
    MapItem *MI = C.back();
    MI->setMapKind(MapKind);
    MI->setIsByRef(ClauseInfo.getIsByRef());
    ArraySectionInfo &ArrSecInfo = MI->getArraySectionInfo();

    assert((NumArgs == 3 * (cast<ConstantInt>(Args[1])->getZExtValue()) + 2) &&
           "Unexpected number of args for array section operand.");
    ArrSecInfo.populateArraySectionDims(Args, NumArgs);
  } else if (ClauseInfo.getIsMapAggrHead() || ClauseInfo.getIsMapAggr() ||
             NumArgs == 4) { // Map-chains with (BasePtr, SectionPtr,
                             // Size, MapType)
    // TODO: Remove handling of AGGR/AGGRHEAD type map-chains when clang only
    // sends in the updated map-chains with 4 element links.
    assert((NumArgs == 3 || NumArgs == 4) &&
           "Malformed MAP:AGGR[HEAD]/CHAIN clause");

    assert(!(MapKind & MapItem::WRNMapUpdateTo ||
             MapKind & MapItem::WRNMapUpdateFrom) &&
           "Unexpected Map Chain in a TO/FROM clause");

    // Create a MapAggr for: <BasePtr, SectionPtr, Size[, MapType]>.
    Value *BasePtr = (Value *)Args[0];
    Value *SectionPtr = (Value *)Args[1];
    Value *Size = (Value *)Args[2];
    uint64_t MapType = 0;
    bool AggrHasMapType = (NumArgs == 4);
    if (AggrHasMapType) {
      assert(isa<ConstantInt>(Args[3]) && "IR is corrupt");
      ConstantInt *CI = dyn_cast<ConstantInt>(Args[3]);
      MapType = CI->getZExtValue();
    }
    MapAggrTy *Aggr = new MapAggrTy(BasePtr, SectionPtr, Size, MapType);

    MapItem *MI;

    // Head of the updated map-chains does not have any modifier equivalent to
    // AGGRHEAD. Instead, only subsequent links in the chain have a CHAIN
    // modifier. Example: QUAL.OMP.MAP(BasePtr, SectionPtr, Size, MapType)
    // QUAL.OMP.MAP:CHAIN(...)
    bool AggrStartsNewStyleMapChain =
        (!ClauseInfo.getIsMapChainLink() && !ClauseInfo.getIsMapAggrHead() &&
         !ClauseInfo.getIsMapAggr() && AggrHasMapType);

    if (ClauseInfo.getIsMapAggrHead() || AggrStartsNewStyleMapChain) {
      // Start a new chain: Add a MapItem
      MI = new MapItem(Aggr);
      MI->setOrig(BasePtr);
      MI->setIsByRef(ClauseInfo.getIsByRef());
      C.add(MI);
    } else {         // Continue the chain for the last MapItem
      MI = C.back(); // Get the last MapItem in the MapClause
      MapChainTy &MapChain = MI->getMapChain();
      assert(MapChain.size() > 0 && "MAP:AGGR cannot start a chain");
      MapChain.push_back(Aggr);
    }
    MI->setMapKind(MapKind);
  } else
    // TODO: Remove this loop and add an assertion that non-chain maps should
    // each have their own clause string.
    // Scalar map items; create a MapItem for each of them.
    for (unsigned I = 0; I < NumArgs; ++I) {
      Value *V = Args[I];
      C.add(V);
      MapItem *MI = C.back();
      MI->setMapKind(MapKind);
      MI->setIsByRef(ClauseInfo.getIsByRef());
    }
}

void WRegionNode::extractDependOpndList(const Use *Args, unsigned NumArgs,
                                        const ClauseSpecifier &ClauseInfo,
                                        DependClause &C, bool IsIn) {
  C.setClauseID(QUAL_OMP_DEPEND_IN); // dummy depend clause id;

  if (ClauseInfo.getIsArraySection()) {
    Value *V = Args[0];
    C.add(V);
    DependItem *DI = C.back();
    DI->setIsIn(IsIn);
    DI->setIsByRef(ClauseInfo.getIsByRef());
    ArraySectionInfo &ArrSecInfo = DI->getArraySectionInfo();

    assert((NumArgs == 3 * (cast<ConstantInt>(Args[1])->getZExtValue()) + 2) &&
           "Unexpected number of args for array section operand.");
    ArrSecInfo.populateArraySectionDims(Args, NumArgs);
  }
  else
    for (unsigned I = 0; I < NumArgs; ++I) {
      Value *V = Args[I];
      C.add(V);
      DependItem *DI = C.back();
      DI->setIsIn(IsIn);
      DI->setIsByRef(ClauseInfo.getIsByRef());
    }
}

void WRegionNode::extractLinearOpndList(const Use *Args, unsigned NumArgs,
                                        const ClauseSpecifier &ClauseInfo,
                                        LinearClause &C) {
  C.setClauseID(QUAL_OMP_LINEAR);

  // The 'step' is always present in the IR coming from Clang, and it is the
  // last argument in the operand list. Therefore, NumArgs >= 2, and the step
  // is the Value in Args[NumArgs-1].
  assert(NumArgs >= 2 && "Missing 'step' for a LINEAR clause");
  Value *StepValue = Args[NumArgs-1];
  assert(StepValue != nullptr && "Null LINEAR 'step'");

  // The linear list items are in Args[0..NumArgs-2]
  for (unsigned I = 0; I < NumArgs-1; ++I) {
    Value *V = Args[I];
    if (!V || isa<ConstantPointerNull>(V)) {
      LLVM_DEBUG(dbgs() << __FUNCTION__ << " Ignoring null clause operand.\n");
      continue;
    }
    C.add(V);
    LinearItem *LI = C.back();
    LI->setStep(StepValue);
    LI->setIsByRef(ClauseInfo.getIsByRef());
    LI->setIsIV(ClauseInfo.getIsIV());
#if INTEL_CUSTOMIZATION
    if (!CurrentBundleDDRefs.empty() &&
        WRegionUtils::supportsRegDDRefs(C.getClauseID())) {
      LI->setHOrig(CurrentBundleDDRefs[I]);
      LI->setHStep(CurrentBundleDDRefs[NumArgs - 1]);
    }
#endif // INTEL_CUSTOMIZATION
  }
}

void WRegionNode::extractReductionOpndList(const Use *Args, unsigned NumArgs,
                                           const ClauseSpecifier &ClauseInfo,
                                           ReductionClause &C,
                                           int ReductionKind,
                                           bool IsInReduction) {
  C.setClauseID(QUAL_OMP_REDUCTION_ADD); // dummy reduction op
  bool IsUnsigned = ClauseInfo.getIsUnsigned();
  assert((!IsUnsigned ||
          (ReductionKind == ReductionItem::WRNReductionMax ||
           ReductionKind == ReductionItem::WRNReductionMin)) &&
         "The UNSIGNED modifier is for MIN/MAX reduction only");

  bool IsComplex = ClauseInfo.getIsComplex();
  assert((!IsComplex ||
          (ReductionKind == ReductionItem::WRNReductionAdd ||
           ReductionKind == ReductionItem::WRNReductionSub ||
           ReductionKind == ReductionItem::WRNReductionMult)) &&
         "The COMPLEX modifier is for ADD/SUB/MUL reduction only");

  if (ClauseInfo.getIsArraySection()) {
    Value *V = Args[0];
    if (!V || isa<ConstantPointerNull>(V)) {
      LLVM_DEBUG(dbgs() << __FUNCTION__ << " Ignoring null clause operand.\n");
      return;
    }
    C.add(V);
    ReductionItem *RI = C.back();
    RI->setType((ReductionItem::WRNReductionKind)ReductionKind);
    RI->setIsUnsigned(IsUnsigned);
    RI->setIsComplex(IsComplex);
    RI->setIsInReduction(IsInReduction);
    RI->setIsByRef(ClauseInfo.getIsByRef());

    ArraySectionInfo &ArrSecInfo = RI->getArraySectionInfo();
    // The number of non array section tuple arguments is 2 by default (base
    // pointer and dimension at the beginning). For UDR, it's 6, while there are
    // 4 additional arguments for constructor, destructor, combiner and
    // initializer at the end.
    assert((NumArgs ==
            3 * (cast<ConstantInt>(Args[1])->getZExtValue()) +
                ((ReductionKind == ReductionItem::WRNReductionUdr) ? 6 : 2)) &&
           "Unexpected number of args for array section operand.");

    if (ReductionKind == ReductionItem::WRNReductionUdr) {
      RI->setConstructor(
          dyn_cast<Function>(dyn_cast<Value>(Args[NumArgs - 4])));
      RI->setDestructor(dyn_cast<Function>(dyn_cast<Value>(Args[NumArgs - 3])));
      RI->setCombiner(dyn_cast<Function>(dyn_cast<Value>(Args[NumArgs - 2])));
      RI->setInitializer(
          dyn_cast<Function>(dyn_cast<Value>(Args[NumArgs - 1])));
    }

    ArrSecInfo.populateArraySectionDims(Args, NumArgs);
  } else {
    for (unsigned I = 0; I < NumArgs; ++I) {
      Value *V = Args[I];
      if (!V || isa<ConstantPointerNull>(V)) {
        LLVM_DEBUG(dbgs() << __FUNCTION__
                          << " Ignoring null clause operand.\n");
        continue;
      }
      C.add(V);
      ReductionItem *RI = C.back();
      RI->setType((ReductionItem::WRNReductionKind)ReductionKind);
      RI->setIsUnsigned(IsUnsigned);
      RI->setIsComplex(IsComplex);
      RI->setIsInReduction(IsInReduction);
      RI->setIsByRef(ClauseInfo.getIsByRef());
#if INTEL_CUSTOMIZATION
      if (!CurrentBundleDDRefs.empty() &&
          WRegionUtils::supportsRegDDRefs(C.getClauseID()))
        RI->setHOrig(CurrentBundleDDRefs[I]);
      if (ClauseInfo.getIsF90DopeVector())
        RI->setIsF90DopeVector(true);
#endif // INTEL_CUSTOMIZATION

      if (ReductionKind == ReductionItem::WRNReductionUdr) {
        assert(((I + 4) < NumArgs) &&
               "Incorrect arg size for User-defined reduction.");
        RI->setConstructor(dyn_cast<Function>(dyn_cast<Value>(Args[I + 1])));
        RI->setDestructor(dyn_cast<Function>(dyn_cast<Value>(Args[I + 2])));
        RI->setCombiner(dyn_cast<Function>(dyn_cast<Value>(Args[I + 3])));
        RI->setInitializer(dyn_cast<Function>(dyn_cast<Value>(Args[I + 4])));
        I += 4;
      }
    }
  }
}

//
// The code below was trying to get initializer and combiner from the LLVM IR.
// However, only UDR requires initializer and combiner function pointers, and
// the front-end has to provide them.
// For standard OpenMP reduction operations(ie, not UDR), the combiner and
// initializer operations are implied by the reduction operation and type
// of the reduction variable.
// Therefore, we should never have to use the code below.
//
#if 0
/// Fill reduction info in ReductionItem \pRI
static void setReductionItem(ReductionItem *RI, IntrinsicInst *Call) {
  auto usedInOnlyOnePhiNode = [](Value *V) {
    PHINode *Phi = 0;
    for (auto U : V->users())
      if (isa<PHINode>(U)) {
        if (Phi) // More than one Phi node
          return (PHINode *)nullptr;
        Phi = cast<PHINode>(U);
      }
    return Phi;
  };

  Value *RedVarPtr = RI->getOrig();
  assert(isa<PointerType>(RedVarPtr->getType()) &&
         "Variable specified in Reduction directive should be a pointer");

  for (auto U : RedVarPtr->users()) {
    if (!isa<LoadInst>(U))
      continue;
    if (auto PhiNode = usedInOnlyOnePhiNode(U)) {
      RI->setInitializer(U);
      if (PhiNode->getIncomingValue(0) == U)
        RI->setCombiner(PhiNode->getIncomingValue(1));
      else
        RI->setCombiner(PhiNode->getIncomingValue(0));
      break;
    }
  }
  assert(RI->getInitializer() && RI->getCombiner() &&
         "Reduction Item is not initialized");
  StringRef DirString = VPOAnalysisUtils::getDirectiveMetadataString(Call);
  int ReductionClauseID = VPOAnalysisUtils::getClauseID(DirString);
  RI->setType(ReductionItem::getKindFromClauseId(ReductionClauseID));
}
#endif


//
// TODO1: This implementation does not yet support nonPOD and array section
//        clause items. It also does not support the optional arguments at
//        the end of linear and aligned clauses.  We'll do that later.
//
void WRegionNode::handleQualOpndList(const Use *Args, unsigned NumArgs,
                                     const ClauseSpecifier &ClauseInfo) {
  int ClauseID = ClauseInfo.getId();
  bool IsInReduction = false;  // IN_REDUCTION clause?

  switch (ClauseID) {
  case QUAL_OMP_SHARED: {
    extractQualOpndList<SharedClause>(Args, NumArgs, ClauseInfo, getShared());
    break;
  }
  case QUAL_OMP_PRIVATE: {
    extractQualOpndListNonPod<PrivateItem>(Args, NumArgs, ClauseInfo,
                                           getPriv());
    break;
  }
  case QUAL_OMP_FIRSTPRIVATE: {
    extractQualOpndListNonPod<FirstprivateItem>(Args, NumArgs, ClauseInfo,
                                                getFpriv());
    break;
  }
  case QUAL_OMP_CANCELLATION_POINTS: {
    assert(canHaveCancellationPoints() &&
           "CANCELLATION.POINTS is not supported on this construct");
    for (unsigned I = 0; I < NumArgs; ++I) {
      assert(isa<AllocaInst>(Args[I]) &&
             "Unexpected operand in CANCELLATION.POINTS bundle.");
      auto *CPAlloca = cast<AllocaInst>(Args[I]);
      addCancellationPointAlloca(CPAlloca);

      // Cancellation Points in the IR look like:
      //
      // %cp = alloca i32            ; CPAlloca
      // ...
      // llvm.region.entry(...) [..."QUAL.OMP.CANCELLATION.POINTS"(%cp) ]
      // ...
      // %1 = __kmpc_cancel(...)     ; CancellationPoint
      // store %1, %cp               ; CPStore
      // ...
      for (auto &CPUse : CPAlloca->uses()) {
        User *CPUser = CPUse.getUser();
        if (StoreInst *CPStore = dyn_cast<StoreInst>(CPUser)) {
          Value *CancellationPoint = CPStore->getValueOperand();
          // Cancellation point may have been removed/replaced with undef by
          // some dead-code elimination optimization e.g.
          // if (expr)
          //   %1 = _kmpc_cancel(...)
          //
          // 'expr' may be always false, and %1 can be optimized away.
          if (!CancellationPoint)
            continue;

          assert(isa<CallInst>(CancellationPoint) &&
                 "Cancellation Point is not a Call.");

          addCancellationPoint(cast<CallInst>(CancellationPoint));
        }
      }
    }
    break;
  }
  case QUAL_OMP_LASTPRIVATE: {
    extractQualOpndListNonPod<LastprivateItem>(Args, NumArgs, ClauseInfo,
                                               getLpriv());
    break;
  }
  case QUAL_OMP_SUBDEVICE: {
    assert(NumArgs == 2 && "SubDevice clause expects two arguments.");
    setSubDeviceBase (Args[0]);
    setSubDeviceLength (Args[1]);
    break;
  }
  case QUAL_OMP_COPYIN: {
    extractQualOpndList<CopyinClause>(Args, NumArgs, ClauseInfo, getCopyin());
    break;
  }
  case QUAL_OMP_COPYPRIVATE: {
    extractQualOpndList<CopyprivateClause>(Args, NumArgs, ClauseInfo,
                                           getCpriv());
    break;
  }
  case QUAL_OMP_DEPEND_IN:
  case QUAL_OMP_DEPEND_OUT:
  case QUAL_OMP_DEPEND_INOUT: {
    bool IsIn = ClauseID==QUAL_OMP_DEPEND_IN;
    extractDependOpndList(Args, NumArgs, ClauseInfo, getDepend(), IsIn);
    break;
  }
  case QUAL_OMP_ORDERED: {
    assert(isa<ConstantInt>(Args[0]) &&
           "Non-constant Value of N for ordered(N).");
    ConstantInt *CI = cast<ConstantInt>(Args[0]);
    uint64_t N = CI->getZExtValue();
    setOrdered(N);

    if (N == 0)
      break;

    // Reaching here means we're looking at doacross loops.
    assert(NumArgs == N + 1 && "Unexpected number of args for Orderd(N).");

    for (unsigned I = 1; I < NumArgs; ++I)
      addOrderedTripCount(Args[I]);

  } break;
  case QUAL_OMP_DEPEND_SINK: {
    setIsDoacross(true);
    SmallVector<Value *, 3> SinkExprs;
    for (unsigned I = 0; I < NumArgs; ++I)
      SinkExprs.push_back(Args[I]);

    getDepSink().add(new DepSinkItem(std::move(SinkExprs)));
  } break;
  case QUAL_OMP_DEPEND_SOURCE: {
    setIsDoacross(true);
    assert(getDepSource().empty() &&
           "More than one 'depend(source)' on the same directive.");

    SmallVector<Value *, 3> SrcExprs;
    for (unsigned I = 0; I < NumArgs; ++I)
      SrcExprs.push_back(Args[I]);

    getDepSource().add(new DepSourceItem(std::move(SrcExprs)));
  } break;
  case QUAL_OMP_IS_DEVICE_PTR: {
    extractQualOpndList<IsDevicePtrClause>(Args, NumArgs, ClauseInfo,
                                           getIsDevicePtr());
    break;
  }
  case QUAL_OMP_USE_DEVICE_PTR:
  case QUAL_OMP_USE_DEVICE_ADDR: {
    extractQualOpndList<UseDevicePtrClause>(Args, NumArgs, ClauseInfo,
                                            getUseDevicePtr());
    break;
  }
  case QUAL_OMP_TO:
  case QUAL_OMP_FROM:
  case QUAL_OMP_MAP_TO:
  case QUAL_OMP_MAP_FROM:
  case QUAL_OMP_MAP_TOFROM:
  case QUAL_OMP_MAP_ALLOC:
  case QUAL_OMP_MAP_RELEASE:
  case QUAL_OMP_MAP_DELETE:
  case QUAL_OMP_MAP_ALWAYS_TO:
  case QUAL_OMP_MAP_ALWAYS_FROM:
  case QUAL_OMP_MAP_ALWAYS_TOFROM:
  case QUAL_OMP_MAP_ALWAYS_ALLOC:
  case QUAL_OMP_MAP_ALWAYS_RELEASE:
  case QUAL_OMP_MAP_ALWAYS_DELETE: {
    unsigned MapKind = MapItem::getMapKindFromClauseId(ClauseID);
    extractMapOpndList(Args, NumArgs, ClauseInfo, getMap(), MapKind);
    break;
  }
  case QUAL_OMP_UNIFORM: {
    extractQualOpndList<UniformClause>(Args, NumArgs, ClauseInfo, getUniform());
    break;
  }
  case QUAL_OMP_LINEAR: {
    extractLinearOpndList(Args, NumArgs, ClauseInfo, getLinear());
    break;
  }
  case QUAL_OMP_ALIGNED: {
    // IR: "QUAL.OMP.ALIGNED"(TYPE1** %ptr1, TYPE2** %ptr2, ..., i32 8)
    assert(NumArgs >= 2 && "Expected at least 2 arguments for ALIGNED clause");
    Value *AlignVal = Args[NumArgs - 1];
    assert(isa<ConstantInt>(AlignVal) &&
           "Alignment in an ALIGNED clause must be constant.");
    ConstantInt *CI = cast<ConstantInt>(AlignVal);
    int Alignment = CI->getZExtValue();
    AlignedClause &C = getAligned();
    for (unsigned I = 0; I < NumArgs - 1; ++I) {
      Value *V = Args[I];
      C.add(V);
      C.back()->setAlign(Alignment);
    }
    break;
  }
  case QUAL_OMP_FLUSH: {
    extractQualOpndList<FlushSet>(Args, NumArgs, ClauseID, getFlush());
    break;
  }
  case QUAL_OMP_ALLOCATE: {
    // IR:    "QUAL.OMP.ALLOCATE"(TYPE* %p, i64 %handle)
    //    or  "QUAL.OMP.ALLOCATE"(TYPE* %p)
    assert((NumArgs == 1 || NumArgs == 2) &&
           "Expected 1 or 2 arguments for ALLOCATED clause");
    Value *Var = Args[0];
    Value *AllocatorHandle = nullptr;
    if (NumArgs==2)
      AllocatorHandle = Args[1];
    AllocateClause &C = getAllocate();
    C.add(Var);
    C.back()->setAllocator(AllocatorHandle);
    break;
  }
  case QUAL_OMP_SCHEDULE_AUTO: {
    extractScheduleOpndList(getSchedule(), Args, ClauseInfo, WRNScheduleAuto);
    break;
  }
  case QUAL_OMP_SCHEDULE_DYNAMIC: {
    extractScheduleOpndList(getSchedule(), Args, ClauseInfo,
                            WRNScheduleDynamic);
    break;
  }
  case QUAL_OMP_SCHEDULE_GUIDED: {
    extractScheduleOpndList(getSchedule(), Args, ClauseInfo, WRNScheduleGuided);
    break;
  }
  case QUAL_OMP_SCHEDULE_RUNTIME: {
    extractScheduleOpndList(getSchedule(), Args, ClauseInfo,
                            WRNScheduleRuntime);
    break;
  }
  case QUAL_OMP_DIST_SCHEDULE_STATIC: {
    extractScheduleOpndList(getDistSchedule(), Args, ClauseInfo,
                            WRNScheduleDistributeStatic);
    break;
  }
  case QUAL_OMP_SCHEDULE_STATIC: {
    extractScheduleOpndList(getSchedule(), Args, ClauseInfo, WRNScheduleStatic);
    break;
  }
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
  case QUAL_OMP_SA_SCHEDULE_STATIC: {
    extractScheduleOpndList(getWorkerSchedule(), Args, ClauseInfo,
                            WRNScheduleStatic);
    break;
  }
#endif // INTEL_FEATURE_CSA
#endif // INTEL_CUSTOMIZATION
  case QUAL_OMP_INREDUCTION_ADD:
  case QUAL_OMP_INREDUCTION_SUB:
  case QUAL_OMP_INREDUCTION_MUL:
  case QUAL_OMP_INREDUCTION_AND:
  case QUAL_OMP_INREDUCTION_OR:
  case QUAL_OMP_INREDUCTION_BXOR:
  case QUAL_OMP_INREDUCTION_BAND:
  case QUAL_OMP_INREDUCTION_BOR:
#if INTEL_CUSTOMIZATION
  case QUAL_OMP_INREDUCTION_EQV:
  case QUAL_OMP_INREDUCTION_NEQV:
#endif // INTEL_CUSTOMIZATION
  case QUAL_OMP_INREDUCTION_MAX:
  case QUAL_OMP_INREDUCTION_MIN:
  case QUAL_OMP_INREDUCTION_UDR:
    IsInReduction = true;
    // FALLTHROUGH
  case QUAL_OMP_REDUCTION_ADD:
  case QUAL_OMP_REDUCTION_SUB:
  case QUAL_OMP_REDUCTION_MUL:
  case QUAL_OMP_REDUCTION_AND:
  case QUAL_OMP_REDUCTION_OR:
  case QUAL_OMP_REDUCTION_BXOR:
  case QUAL_OMP_REDUCTION_BAND:
  case QUAL_OMP_REDUCTION_BOR:
#if INTEL_CUSTOMIZATION
  case QUAL_OMP_REDUCTION_EQV:
  case QUAL_OMP_REDUCTION_NEQV:
#endif // INTEL_CUSTOMIZATION
  case QUAL_OMP_REDUCTION_MAX:
  case QUAL_OMP_REDUCTION_MIN:
  case QUAL_OMP_REDUCTION_UDR: {
    int ReductionKind = ReductionItem::getKindFromClauseId(ClauseID);
    assert(ReductionKind > 0 && "Bad reduction operation");
    if (IsInReduction)
      extractReductionOpndList(Args, NumArgs, ClauseInfo, getInRed(),
                               ReductionKind, true);
    else
      extractReductionOpndList(Args, NumArgs, ClauseInfo, getRed(),
                               ReductionKind, false);
    break;
  }
  case QUAL_OMP_NORMALIZED_IV:
    for (unsigned I = 0; I < NumArgs; ++I) {
      Value *V = Args[I];
      Constant *C = dyn_cast<Constant>(V);
      if (C && C->isNullValue()) {
        // After promoting %.omp.iv into a register, we change all pointers in
        // OMP.NORMALIZED.IV to "i8* null". In this case, do not populate
        // WRNLoopInfo's NormIV vector. This forces regularizeOMPLoop to bail
        // out early on subsequent calls.
        assert(I==0 && "malformed NORMALIZED_IV clause");
        // IVs are all null or nonnull; cannot have some null and some nonnull
        break;
      }
      // TODO: OPAQUEPOINTER: Add this information in the clause
      Type *VTy = isa<PointerType>(V->getType())
                      ? cast<PointerType>(V->getType())->getElementType()
                      : V->getType();
      getWRNLoopInfo().addNormIV(V, VTy);
    }
    break;
  case QUAL_OMP_NORMALIZED_UB:
    for (unsigned I = 0; I < NumArgs; ++I) {
      Value *V = Args[I];
      Constant *C = dyn_cast<Constant>(V);
      if (C && C->isNullValue()) {
        assert(I==0 && "malformed NORMALIZED_UB clause");
        break;
      }
      // TODO: OPAQUEPOINTER: Add this information in the clause
      Type *VTy = isa<PointerType>(V->getType())
                      ? cast<PointerType>(V->getType())->getElementType()
                      : V->getType();
      getWRNLoopInfo().addNormUB(V, VTy);
    }
    break;
  case QUAL_OMP_OFFLOAD_NDRANGE: {
    SmallVector<Value *, 3> NDRange;
    for (unsigned I = 0; I < NumArgs; ++I) {
      NDRange.push_back(Args[I]);
    }
    setUncollapsedNDRangeDimensions(NDRange);
    break;
  }
  case QUAL_OMP_JUMP_TO_END_IF:
    // Nothing to parse for this auxiliary clause.
    // It may exist after VPO Paropt prepare and before
    // VPO Paropt transform to guarantee that DCE does not
    // remove unreachable region exit directives.
    break;
  default:
    llvm_unreachable("Unknown ClauseID in handleQualOpndList()");
    break;
  }
}

void WRegionNode::getClausesFromOperandBundles() {
  // Under the directive.region.entry/exit representation the intrinsic
  // is alone in the EntryBB, so EntryBB->front() is the intrinsic call
  BasicBlock *EntryBB = getEntryBBlock();
  assert(EntryBB && "Entry Bblock is null");

  Instruction *I = &(EntryBB->front());
  assert(isa<IntrinsicInst>(I) &&
         "Call not found for directive.region.entry()");

  IntrinsicInst *Call = cast<IntrinsicInst>(I);
  getClausesFromOperandBundles(Call);
}

#if INTEL_CUSTOMIZATION
void WRegionNode::getClausesFromOperandBundles(IntrinsicInst *Call,
                                               loopopt::HLInst *H) {
#else
void WRegionNode::getClausesFromOperandBundles(IntrinsicInst *Call) {
#endif // INTEL_CUSTOMIZATION
  unsigned i, NumOB = Call->getNumOperandBundles();

  // Index i start from 1 (not 0) because we want to skip the first
  // OperandBundle, which is the directive name.
  for (i = 1; i < NumOB; ++i) {
    // BU is the ith OperandBundle, which represents a clause
    OperandBundleUse BU = Call->getOperandBundleAt(i);

    // The clause name is the tag name
    StringRef ClauseString = BU.getTagName();

    // Extract clause properties
    ClauseSpecifier ClauseInfo(ClauseString);

    // Get the argument list from the current OperandBundle
    ArrayRef<llvm::Use> Args = BU.Inputs;
    unsigned NumArgs = Args.size(); // BU.Inputs.size()

    const Use *ArgList = NumArgs == 0 ? nullptr : &Args[0];
#if INTEL_CUSTOMIZATION
    if (H) {
      assert(NumOB == H->getNumOperandBundles() &&
             "Number of operand bundles mismatch between HLInst and "
             "IntrinsicInst.");
      CurrentBundleDDRefs.assign(H->bundle_op_ddref_begin(i),
                                 H->bundle_op_ddref_end(i));
    }
#endif // INTEL_CUSTOMIZATION

    // Parse the clause and update the WRN
    parseClause(ClauseInfo, ArgList, NumArgs);
  }
}

bool WRegionNode::canHaveSchedule() const {
  unsigned SubClassID = getWRegionKindID();
  switch (SubClassID) {
  case WRNParallelLoop:
  case WRNDistributeParLoop:
  case WRNWksLoop:
    return true;
  }
  return false;
}

bool WRegionNode::canHaveDistSchedule() const {
  // true for WRNDistribute and WRNDistributeParLoop
  return getIsDistribute();
}

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
bool WRegionNode::canHaveWorkerSchedule() const {
  unsigned SubClassID = getWRegionKindID();
  switch (SubClassID) {
  case WRNParallelLoop:
  case WRNWksLoop:
    return true;
  }
  return false;
}
#endif // INTEL_FEATURE_CSA
#endif // INTEL_CUSTOMIZATION

bool WRegionNode::canHaveShared() const {
  unsigned SubClassID = getWRegionKindID();
  switch (SubClassID) {
  case WRNParallel:
  case WRNParallelLoop:
  case WRNParallelSections:
  case WRNParallelWorkshare:
  case WRNTeams:
  case WRNDistributeParLoop:
  case WRNTask:
  case WRNTaskloop:
    return true;
  }
  return false;
}

bool WRegionNode::canHavePrivate() const {
  unsigned SubClassID = getWRegionKindID();
  switch (SubClassID) {
  case WRNParallel:
  case WRNParallelLoop:
  case WRNParallelSections:
  case WRNParallelWorkshare:
  case WRNTeams:
  case WRNDistributeParLoop:
  case WRNTarget:
  case WRNTask:
  case WRNTaskloop:
  case WRNVecLoop:
  case WRNWksLoop:
  case WRNSections:
  case WRNDistribute:
  case WRNSingle:
  case WRNGenericLoop:
    return true;
  }
  return false;
}

bool WRegionNode::canHaveFirstprivate() const {
  unsigned SubClassID = getWRegionKindID();

  // similar to canHavePrivate except for SIMD,
  // which has Private but not Firstprivate
  if (SubClassID == WRNVecLoop || SubClassID == WRNGenericLoop)
    return false;
  return canHavePrivate();
}

bool WRegionNode::canHaveLastprivate() const {
  unsigned SubClassID = getWRegionKindID();
  switch (SubClassID) {
  case WRNParallelLoop:
  case WRNParallelSections:
  case WRNDistributeParLoop:
  case WRNTaskloop:
  case WRNVecLoop:
  case WRNWksLoop:
  case WRNSections:
  case WRNDistribute:
  case WRNGenericLoop:
    return true;
  }
  return false;
}

bool WRegionNode::canHaveInReduction() const {
  unsigned SubClassID = getWRegionKindID();
  switch (SubClassID) {
  case WRNTask:       // OMP5.0 task's in_reduction clause
  case WRNTaskloop:   // OMP5.0 taskloop's in_reduction clause
    return true;
  }
  return false;
}

bool WRegionNode::canHaveReduction() const {
  unsigned SubClassID = getWRegionKindID();
  switch (SubClassID) {
  case WRNParallel:
  case WRNParallelLoop:
  case WRNParallelSections:
  case WRNParallelWorkshare:
  case WRNTeams:
  case WRNDistributeParLoop:
  case WRNTaskgroup:  // OMP5.0 taskgroup's task_reduction clause
  case WRNTaskloop:   // OMP5.0 taskloop's reduction clause
  case WRNVecLoop:
  case WRNWksLoop:
  case WRNSections:
  case WRNGenericLoop:
    return true;
  }
  return false;
}

bool WRegionNode::canHaveNowait() const {
  unsigned SubClassID = getWRegionKindID();
  switch (SubClassID) {
  case WRNTarget:
  case WRNTargetEnterData:
  case WRNTargetExitData:
  case WRNTargetUpdate:
  case WRNTargetVariant:
  case WRNWksLoop:
  case WRNSections:
  case WRNWorkshare:
  case WRNSingle:
    return true;
  }
  return false;
}

bool WRegionNode::canHaveCopyin() const {
  unsigned SubClassID = getWRegionKindID();
  switch (SubClassID) {
  case WRNParallel:
  case WRNParallelLoop:
  case WRNParallelSections:
  case WRNParallelWorkshare:
  case WRNDistributeParLoop:
    return true;
  }
  return false;
}

bool WRegionNode::canHaveCopyprivate() const {
  unsigned SubClassID = getWRegionKindID();
  // only SINGLE can have a Copyprivate clause
  return SubClassID==WRNSingle;
}

bool WRegionNode::canHaveLinear() const {
  unsigned SubClassID = getWRegionKindID();
  switch (SubClassID) {
  case WRNParallelLoop:
  case WRNDistributeParLoop:
  case WRNVecLoop:
  case WRNWksLoop:
    return true;
  }
  return false;
}

bool WRegionNode::canHaveUniform() const {
  unsigned SubClassID = getWRegionKindID();
  // only SIMD can have a Uniform clause
  return SubClassID==WRNVecLoop;
}

bool WRegionNode::canHaveAligned() const {
  // only SIMD can have an Aligned clause
  return canHaveUniform();
}

bool WRegionNode::canHaveMap() const {
  // Only target-type constructs take map clauses
  return getIsTarget();
}

bool WRegionNode::canHaveIsDevicePtr() const {
  unsigned SubClassID = getWRegionKindID();
  // only WRNTargetNode can have a IsDevicePtr clause
  return SubClassID==WRNTarget;
}

bool WRegionNode::canHaveUseDevicePtr() const {
  unsigned SubClassID = getWRegionKindID();
  switch (SubClassID) {
  case WRNTargetData:
  case WRNTargetVariant:
    return true;
  }
  return false;
}

bool WRegionNode::canHaveDepend() const {
  unsigned SubClassID = getWRegionKindID();
  switch (SubClassID) {
  case WRNTask:
  case WRNTarget:
  case WRNTargetEnterData:
  case WRNTargetExitData:
  case WRNTargetUpdate:
    return true;
  }
  return false;
}

bool WRegionNode::canHaveDepSrcSink() const {
  unsigned SubClassID = getWRegionKindID();
  // Only WRNOrderedNode can have a 'depend(src)' or 'depend(sink : vec)'
  // clause, but but only if its "IsDoacross" field is true.
  if(SubClassID==WRNOrdered)
    return getIsDoacross();

  return false;
}

bool WRegionNode::canHaveFlush() const {
  unsigned SubClassID = getWRegionKindID();
  // only WRNFlushNode can have a flush set
  return SubClassID==WRNFlush;
}

// Returns `true` if the Construct can be cancelled, and thus have
// Cancellation Points.
bool WRegionNode::canHaveCancellationPoints() const {
  unsigned SubClassID = getWRegionKindID();
  switch (SubClassID) {
  case WRNParallel:
  case WRNWksLoop:
  case WRNSections:
  case WRNTask:
  case WRNParallelLoop:
  case WRNParallelSections:
    return true;
  }
  return false;
}

bool WRegionNode::canHaveCollapse() const {
  unsigned SubClassID = getWRegionKindID();
  switch (SubClassID) {
  case WRNParallelLoop:
  case WRNDistributeParLoop:
  case WRNTaskloop:
  case WRNVecLoop:
  case WRNWksLoop:
  case WRNDistribute:
  case WRNGenericLoop:
    return true;
  }

  return false;
}

bool WRegionNode::canHaveAllocate() const {
  unsigned SubClassID = getWRegionKindID();
  switch (SubClassID) {
  case WRNParallel:
  case WRNParallelLoop:
  case WRNParallelSections:
  case WRNParallelWorkshare:
  case WRNTeams:
  case WRNDistributeParLoop:
  case WRNTarget:
  case WRNTask:
  case WRNTaskgroup:
  case WRNTaskloop:
  case WRNWksLoop:
  case WRNSections:
  case WRNDistribute:
  case WRNSingle:
    return true;
  }
  return false;
}

// Return true if the construct needs to be outlined for OpenMP runtime.
bool WRegionNode::needsOutlining() const {
  unsigned SubClassID = getWRegionKindID();
  switch (SubClassID) {
  case WRNParallel:
  case WRNParallelLoop:
  case WRNParallelSections:
  case WRNParallelWorkshare:
  case WRNDistributeParLoop:
  case WRNTask:
  case WRNTaskloop:
  case WRNTeams:
  case WRNTarget:
  case WRNTargetData:
    return true;
  }
  return false;
}

StringRef WRegionNode::getName() const {
  // good return llvm::vpo::WRNName[getWRegionKindID()];
  return WRNName[getWRegionKindID()];
}

void WRegionNode::errorClause(StringRef ClauseName) const {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  formatted_raw_ostream OS(dbgs());
  OS << "Error: " << getName() << " WRNs do not take " << ClauseName
     << " clauses.\n";
  // Example:
  // Error: simd WRNs do not take SHARED clauses.
  llvm_unreachable("Unexpected clause encountered!");
#endif
}

void WRegionNode::errorClause(int ClauseID) const {
  StringRef ClauseName = VPOAnalysisUtils::getOmpClauseName(ClauseID);
  errorClause(ClauseName);
}

// Printing routines to help dump WRN content

// Auxiliary function to print a BB in a WRN dump
// If BB is null:
//   Verbosity == 0: exit without printing anything
//   Verbosity >= 1: print "Title: NULL BBlock"
// If BB is not null:
//   Verbosity <= 1: : print BB->getName()
//   Verbosity >= 2: : print *BB (dumps the Bblock content)
void vpo::printBB(StringRef Title, BasicBlock *BB, formatted_raw_ostream &OS,
                  int Indent, unsigned Verbosity) {
  if (Verbosity==0 && !BB)
    return; // When Verbosity==0; print nothing if BB==nullptr

  OS.indent(Indent) << Title << ": ";
  if (!BB) {
    OS << "NULL BBlock\n";
    return;
  }

  if (Verbosity<=1) {
    OS << BB->getName() << "\n";
  } else { // Verbosity >= 2
    OS << "\n";
    OS.indent(Indent) << *BB << "\n";
  }
}

// Auxiliary function to print a Value in a WRN dump
// If Val is null:
//   Verbosity == 0: exit without printing anything
//   Verbosity >= 1: print "Title: UNSPECIFIED"
// If Val is not null:
//   print *Val regardless of Verbosity
void vpo::printVal(StringRef Title, Value *Val, formatted_raw_ostream &OS,
                   int Indent, unsigned Verbosity) {
  if (Verbosity==0 && !Val)
    return; // When Verbosity==0; print nothing if Val==nullptr

  OS.indent(Indent) << Title << ": ";
  if (!Val) {
    OS << "UNSPECIFIED\n";
    return;
  }
  OS << *Val << "\n";
}

// Auxiliary function to print a range of Values in a WRN dump.
void vpo::printValRange(StringRef Title, Value *Val1, Value *Val2,
                   formatted_raw_ostream &OS, int Indent, unsigned Verbosity) {
  if (Verbosity==0 && !Val1 && !Val2)
    return; // When Verbosity is 0, print nothing if both Vals are null

  OS.indent(Indent) << Title << "(";
  if (!Val1)
    OS << "UNSPECIFIED:";
  else
    OS << *Val1 << ":";
  if (!Val2) {
    OS << "UNSPECIFIED)\n";
    return;
  }
  OS << *Val2 << ")\n";
}

// Auxiliary function to print an ArrayRef of Values in a WRN dump
void vpo::printValList(StringRef Title, ArrayRef<Value *> const &Vals,
                       formatted_raw_ostream &OS, int Indent,
                       unsigned Verbosity) {

  if (Vals.empty())
    return; // print nothing if Vals is empty

  OS.indent(Indent) << Title << ":";

  for (auto *V : Vals) {
    if (V) {
      OS << " ";
      V->printAsOperand(OS);
    } else if (Verbosity >= 1) {
      OS << " UNSPECIFIED";
    }
  }
  OS << "\n";
}

// Auxiliary function to print an Int in a WRN dump
// If Num < Min:
//   Verbosity == 0: exit without printing anything
//   Verbosity >= 1: print "Title: UNSPECIFIED"
// If Num >= Min:
//   print "Title: Num"
// For clauses expecting positive constants (eg, COLLAPSE), use Min==1 (default)
// For those expecting non-negative constants (eg, OFFLOAD_ENTRY_IDX), use Min==0
//
void vpo::printInt(StringRef Title, int Num, formatted_raw_ostream &OS,
                   int Indent, unsigned Verbosity, int Min) {
  if (Verbosity==0 && Num < Min)
    return; // When Verbosity==0; print nothing if Num < Min

  OS.indent(Indent) << Title << ": ";
  if (Num < Min) {
    OS << "UNSPECIFIED\n";
    return;
  }
  OS << Num << "\n";
}

// Auxiliary function to print a boolean in a WRN dump
// If Verbosity == 0, don't print anything if Flag is false;
// otherwise, print "Title: true/false"
void vpo::printBool(StringRef Title, bool Flag, formatted_raw_ostream &OS,
                    int Indent, unsigned Verbosity) {
  if (Verbosity==0 && Flag==false)
    return; // When Verbosity==0; print nothing if Flag==false

  OS.indent(Indent) << Title << ": ";
  if (Flag)
    OS <<  "true\n";
  else
    OS <<  "false\n";
}

// Auxiliary function to print a String for dumping certain clauses. E.g.,
// for the DEFAULT clause we may print "NONE", "SHARED", "PRIVATE", etc.
// If <Str> == "UNSPECIFIED"  (happens when the clause is not specified)
//   Verbosity == 0: exit without printing anything
//   Verbosity >= 1: print "Title: UNSPECIFIED"
// Else
//   print "Title: Str"
void vpo::printStr(StringRef Title, StringRef Str, formatted_raw_ostream &OS,
                   int Indent, unsigned Verbosity) {
  if (Verbosity!=0 || Str!="UNSPECIFIED")
    OS.indent(Indent) << Title << ": " << Str << "\n";
}

#endif // INTEL_COLLAB
