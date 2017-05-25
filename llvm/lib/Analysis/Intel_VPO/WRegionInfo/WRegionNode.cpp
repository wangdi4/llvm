//===-- WRegionNode.cpp - Implements the WRegionNode class ----------------===//
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
//   This file implements the WRegionNode class.
//   It's the base class for WRN graph nodes, and should never be
//   instantiated directly.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/CFG.h"
#include "llvm/IR/Dominators.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionNode.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegion.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionUtils.h"

#define DEBUG_TYPE "vpo-wrnnode"

// Define this to 1 for OpenCL (eg, features/vpo branch)
// Define it to 0 for vpo-xmain
#define VPO_FOR_OPENCL 0

using namespace llvm;
using namespace llvm::vpo;

unsigned WRegionNode::UniqueNum(0);

std::unordered_map<int, StringRef> llvm::vpo::WRNName = {
    {WRegionNode::WRNParallel, "parallel"},
    {WRegionNode::WRNParallelLoop, "parallel loop"},
    {WRegionNode::WRNParallelSections, "parallel sections"},
    {WRegionNode::WRNParallelWorkshare, "parallel workshare"},
    {WRegionNode::WRNTeams, "teams"},
    {WRegionNode::WRNDistributeParLoop, "distribute parallel loop"},
    {WRegionNode::WRNTarget, "target"},
    {WRegionNode::WRNTargetData, "target data"},
    {WRegionNode::WRNTask, "task"},
    {WRegionNode::WRNTaskloop, "taskloop"},
    {WRegionNode::WRNVecLoop, "simd"},
    {WRegionNode::WRNWksLoop, "loop"},
    {WRegionNode::WRNSections, "sections"},
    {WRegionNode::WRNWorkshare, "workshare"},
    {WRegionNode::WRNDistribute, "distribute"},
    {WRegionNode::WRNSingle, "single"},
    {WRegionNode::WRNMaster, "master"},
    {WRegionNode::WRNAtomic, "atomic"},
    {WRegionNode::WRNBarrier, "barrier"},
    {WRegionNode::WRNCancel, "cancel"},
    {WRegionNode::WRNCritical, "critical"},
    {WRegionNode::WRNFlush, "flush"},
    {WRegionNode::WRNOrdered, "ordered"},
    {WRegionNode::WRNTaskgroup, "taskgroup"},
    {WRegionNode::WRNTaskwait, "taskwait"},
    {WRegionNode::WRNTaskyield, "taskyield"}};

// constructor for LLVM IR representation
WRegionNode::WRegionNode(unsigned SCID, BasicBlock *BB)
    : SubClassID(SCID), Attributes(0), EntryBBlock(BB) {
  setNextNumber();
  setParent(nullptr);
  setExitBBlock(nullptr);
  setIsFromHIR(false);
  resetBBSet();
}

// constructor for HIR representation
WRegionNode::WRegionNode(unsigned SCID) : SubClassID(SCID), Attributes(0) {
  setNextNumber();
  setParent(nullptr);
  setEntryBBlock(nullptr);
  setExitBBlock(nullptr);
  resetBBSet();
  setIsFromHIR(true);
}

/// \brief Wrap up the WRN creation now that we have the ExitBB. If the WRN is
/// a loop construct, this routine also calls the associated Loop from the
/// LoopInfo.
void WRegionNode::finalize(BasicBlock *ExitBB) {
  setExitBBlock(ExitBB);
  if (getIsOmpLoop()) {
    LoopInfo *LI = getLoopInfo();
    assert(LI && "LoopInfo not present in a loop construct");
    BasicBlock *EntryBB = getEntryBBlock();
    Loop *Lp = IntelGeneralUtils::getLoopFromLoopInfo(LI, EntryBB, ExitBB);

    // Do not assert for loop-type constructs when Lp==NULL because transforms
    // before Paropt may have optimized away the loop.
    setLoop(Lp);

    if (Lp)
      DEBUG(dbgs() << "\n=== finalize WRN: found loop : " << *Lp << "\n");
    else
      DEBUG(dbgs() << "\n=== finalize WRN: loop not found. Optimized away?\n");

#if VPO_FOR_OPENCL
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
            CallInst *Call = dyn_cast<CallInst>(&I);
            // DEBUG(dbgs() << "Found Call: " << *Call << "\n");
            assert(Call->getNumArgOperands()==2 &&
                   "__read_pipe_2_bl_intel() is expected to have 2 operands");
            Value *V = Call->getArgOperand(1); // second operand
            AllocaInst *Alloca = VPOAnalysisUtils::findAllocaInst(V);
            assert (Alloca && 
                    "Alloca not found for __read_pipe_2_bl_intel operand");
            if (Alloca) {
              // DEBUG(dbgs() << "Found Alloca: " << *Alloca << "\n");
              if (!contains(Alloca->getParent())) {
                // Alloca is outside of the WRN, so privatize it
                PC.add(Alloca);
                // DEBUG(dbgs() << "Will privatize: " << *Alloca << "\n");
              }
              // else do nothing: the alloca is inside the WRN hence it is
              // already private
            }
          }
      resetBBSet();
    }
#endif //VPO_FOR_OPENCL
  }
}

/// \brief Populates BBlockSet with BBs in the WRN from EntryBB to ExitBB.
void WRegionNode::populateBBSet(void) {
  BasicBlock *EntryBB = getEntryBBlock();
  BasicBlock *ExitBB = getExitBBlock();

  assert(EntryBB && "Missing EntryBB!");
  assert(ExitBB && "Missing ExitBB!");
  resetBBSet();
  IntelGeneralUtils::collectBBSet(EntryBB, ExitBB, BBlockSet);
}

void WRegionNode::populateBBSetIfEmpty(void) {
  if (isBBSetEmpty())
    populateBBSet();
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
                                                   bool Verbose) const {
  // Print BEGIN <directive_name>
  printBegin(OS, Depth);

  // Print WRN contents specific to a given derived class. If the derived
  // class does not define printExtra(), then this does nothing.
  printExtra(OS, Depth+1, Verbose);

  // Print WRN contents: Clauses, BBlocks, Loop, Children, etc.
  printBody(OS, true, Depth+1, Verbose);

  // Print END <directive_name>
  printEnd(OS, Depth);
}

void WRegionNode::printBegin(formatted_raw_ostream &OS, unsigned Depth) const {
  int Id = getDirID(); 
  StringRef DirName = VPOAnalysisUtils::getDirectiveName(Id);
  OS.indent(2*Depth) << "BEGIN " << DirName <<" ID=" << getNumber() << " {\n\n";
}

void WRegionNode::printEnd(formatted_raw_ostream &OS, unsigned Depth) const {
  int Id = getDirID(); 
  StringRef DirName = VPOAnalysisUtils::getDirectiveName(Id);
  OS.indent(2*Depth) << "} END " << DirName <<" ID=" << getNumber() << "\n\n";
}

void WRegionNode::printBody(formatted_raw_ostream &OS, bool PrintChildren, 
                                       unsigned Depth, bool Verbose) const {
  printClauses(OS, Depth, Verbose);

  if (getIsFromHIR())
    printHIR(OS, Depth, Verbose); // defined by derived WRN
  else {
    printEntryExitBB(OS, Depth, Verbose);
    if (getIsOmpLoop())
      printLoopBB(OS, Depth, Verbose);
  }

  if (PrintChildren)
    printChildren(OS, Depth, Verbose);
}


void WRegionNode::printClauses(formatted_raw_ostream &OS, 
                               unsigned Depth, bool Verbose) const {
  bool PrintedSomething = false;

  if (hasSchedule()) {
    getSchedule().print(OS, Depth, Verbose);
    PrintedSomething = true;
  }
  if (hasShared()) {
    getShared().print(OS, Depth, Verbose);
    PrintedSomething = true;
  }
  if (hasPrivate()) {
    getPriv().print(OS, Depth, Verbose);
    PrintedSomething = true;
  }
  if (hasFirstprivate()) {
    getFpriv().print(OS, Depth, Verbose);
    PrintedSomething = true;
  }
  if (hasLastprivate()) {
    getLpriv().print(OS, Depth, Verbose);
    PrintedSomething = true;
  }
  if (hasReduction()) {
    getRed().print(OS, Depth, Verbose);
    PrintedSomething = true;
  }
  if (hasCopyin()) {
    getCopyin().print(OS, Depth, Verbose);
    PrintedSomething = true;
  }
  if (hasCopyprivate()) {
    getCpriv().print(OS, Depth, Verbose);
    PrintedSomething = true;
  }
  if (hasLinear()) {
    getLinear().print(OS, Depth, Verbose);
    PrintedSomething = true;
  }
  if (hasUniform()) {
    getUniform().print(OS, Depth, Verbose);
    PrintedSomething = true;
  }
  if (hasMap()) {
    getMap().print(OS, Depth, Verbose);
    PrintedSomething = true;
  }
  if (hasIsDevicePtr()) {
    getIsDevicePtr().print(OS, Depth, Verbose);
    PrintedSomething = true;
  }
  if (hasUseDevicePtr()) {
    getUseDevicePtr().print(OS, Depth, Verbose);
    PrintedSomething = true;
  }
  if (hasDepend()) {
    getDepend().print(OS, Depth, Verbose);
    PrintedSomething = true;
  }
  if (hasDepSink()) {
    getDepSink().print(OS, Depth, Verbose);
    PrintedSomething = true;
  }
  if (hasAligned()) {
    getAligned().print(OS, Depth, Verbose);
    PrintedSomething = true;
  }
  if (hasFlush()) {
    getFlush().print(OS, Depth, Verbose);
    PrintedSomething = true;
  }
  if (PrintedSomething)
    OS << "\n";
}

// Auxiliary function to print BB:
//   Verbose == true:  print *BB
//   Verbose == false: print BB->getName()
void printBB(BasicBlock *BB, formatted_raw_ostream &OS, int Indent,
                                                        bool Verbose) {
  if (!BB)
    OS << "NULL BBlock\n";

  if (Verbose) {
    OS << "\n";
    OS.indent(Indent) << *BB << "\n";
  } else
    OS << BB->getName() << "\n";
}

void WRegionNode::printEntryExitBB(formatted_raw_ostream &OS, unsigned Depth,
                                   bool Verbose) const {
  if (getIsFromHIR()) // HIR representation; no BBs to print
    return;

  int Ind = 2*Depth;

  BasicBlock *EntryBB = getEntryBBlock();
  BasicBlock *ExitBB = getExitBBlock();
  assert (EntryBB && "Entry BB is null!");
  assert (ExitBB && "Exit BB is null!");

  OS.indent(Ind) << "EntryBB: ";
  printBB(EntryBB, OS, Ind, Verbose);

  OS.indent(Ind) << "ExitBB: ";
  printBB(ExitBB, OS, Ind, Verbose);

  if (Verbose) {
    OS.indent(Ind) << "BBSet";
    if (!isBBSetEmpty()) {
      OS << ":\n";
      for (BasicBlock *BB : BBlockSet ) {
        OS.indent(Ind+2) << BB->getName() << "\n";
        // OS.indent(Ind+2) << *BB << "\n";
      }
    } else
      OS << "is empty\n";
  }
  OS << "\n";
}

void WRegionNode::printLoopBB(formatted_raw_ostream &OS, unsigned Depth,
                                                         bool Verbose) const {
  if (!getIsOmpLoop())
    return;

  int Ind = 2*Depth;

  Loop *L = getLoop();

  if (!L) {
    OS.indent(Ind) << "Loop is missing; may be optimized away.\n";
    return;
  }

  OS.indent(Ind) << "Loop Preheader: ";
  printBB(L->getLoopPreheader(), OS, Ind, Verbose);

  OS.indent(Ind) << "Loop Header: ";
  printBB(L->getHeader(), OS, Ind, Verbose);

  OS.indent(Ind) << "Loop Latch: ";
  printBB(L->getLoopLatch(), OS, Ind, Verbose);

  OS << "\n";
}

void WRegionNode::printChildren(formatted_raw_ostream &OS, unsigned Depth,
                                bool Verbose) const {
  for (WRegionNode *W : Children)
    W->print(OS, Depth, Verbose);
}

void WRegionNode::destroy() {
  // TODO: call destructor
}

void WRegionNode::destroyAll() {
  // TODO: implement this by recursive walk from top
}

void WRegionNode::dump(bool Verbose) const {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  formatted_raw_ostream OS(dbgs());
  print(OS, 0, Verbose);
#endif
}

//
// functions below are used to update WRNs with clause information
//

// Parse the clause in the llvm.intel.directive.qual* representation.
void WRegionNode::parseClause(const ClauseSpecifier &ClauseInfo,
                              IntrinsicInst *Call){

  // Get argument list from the intrinsic call
  const Use *Args = Call->getOperandList();

  // Skip Args[0] as it's the clause name metadata; hence the -1 below
  unsigned NumArgs = Call->getNumArgOperands() - 1;

  parseClause(ClauseInfo, &Args[1], NumArgs);
}

// Common code to parse the clause. This routine is used for both
// representations: llvm.intel.directive.qual* and directive.region.entry/exit.
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
    handleQual(ClauseID);
  } else if (ClauseNumArgs == 1) {
    // The clause takes one argument only
    assert(NumArgs == 1 && "This clause takes one argument.");
    Value *V = (Value*)(Args[0]);
    handleQualOpnd(ClauseID, V);
  } else {
    // The clause takes a list of arguments
    assert(NumArgs >= 1 && "This clause takes one or more arguments.");
    handleQualOpndList(&Args[0], NumArgs, ClauseInfo);
  }
}


void WRegionNode::handleQual(int ClauseID) {
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
  case QUAL_OMP_DEFAULTMAP_TOFROM_SCALAR:
    setDefaultmapTofromScalar(true);
    break;
  case QUAL_OMP_NOWAIT:
    setNowait(true);
    break;
  case QUAL_OMP_UNTIED:
    setUntied(true);
    setTaskFlag(getTaskFlag() & ~WRNTaskFlag::Tied);
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
    setIsThreads(false);
    break;
  case QUAL_OMP_DEPEND_SOURCE:
    setIsDoacross(true);
    setIsDepSource(true);
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
  case QUAL_LIST_END: //TODO: remove this obsolete case
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
  if (CI != nullptr) {
    N = *((CI->getValue()).getRawData());
  }

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
  case QUAL_OMP_IF:
    setIf(V);
    break;
  case QUAL_OMP_NAME: {
    // The operand is expected to be a constant string. Example:
    // call void @llvm.intel.directive.qual.opnd.a9i8(metadata
    // !"QUAL.OMP.NAME", [9 x i8] c"lock_name")
    ConstantDataSequential *CD = dyn_cast<ConstantDataSequential>(V);
    assert((CD != nullptr && (CD->isString() || CD->isCString())) &&
           "QUAL_OMP_NAME opnd should be a constant string.");

    if (CD->isCString()) // Process as C string first, so that the nul
                         // bytes at the end are ignored. (e.g. c"lock_name\00")
      setUserLockName(CD->getAsCString());
    else if (CD->isString()) // Process as a regular string. (e.g. c"lock_name")
      setUserLockName(CD->getAsString());

  } break;
  case QUAL_OMP_NUM_THREADS:
    setNumThreads(V);
    break;
  case QUAL_OMP_ORDERED:
    assert(N > 0 && "ORDERED parameter must be positive");
    setOrdered(N);
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
  default:
    llvm_unreachable("Unknown ClauseID in handleQualOpnd()");
  }
}

#if 1
// TODO: investigate/fix this build issue
// Moved this here from WRegionUtils.cpp
// Having this in WRegionUtils.cpp caused link error:
//   undefined reference to `llvm::vpo::Clause<llvm::vpo::PrivateItem>*
//   llvm::vpo::WRegionUtils::extractQualOpndList<llvm::vpo::PrivateItem>(
//   llvm::IntrinsicInst*, llvm::vpo::Clause<llvm::vpo::PrivateItem>*)'

template <typename ClauseTy>
void WRegionUtils::extractQualOpndList(const Use *Args, unsigned NumArgs,
                                            int ClauseID, ClauseTy &C) {
  C.setClauseID(ClauseID);
  for (unsigned I = 0; I < NumArgs; ++I) {
    Value *V = (Value*) Args[I];
    C.add(V);
  }
}

template <typename ClauseTy>
void WRegionUtils::extractQualOpndList(const Use *Args, unsigned NumArgs,
                                       const ClauseSpecifier &ClauseInfo,
                                       ClauseTy &C) {
  int ClauseID = ClauseInfo.getId();
  bool IsConditional = ClauseInfo.getIsConditional();

  if (IsConditional)
    assert(ClauseID == QUAL_OMP_LASTPRIVATE && 
           "The CONDITIONAL keyword is for LASTPRIVATE clauses only");

  C.setClauseID(ClauseID);

  for (unsigned I = 0; I < NumArgs; ++I) {
    Value *V = (Value*) Args[I];
    C.add(V);
    if (IsConditional) {
      auto *I = C.back();
      I->setIsConditional(true);
    }
  }
}

void WRegionUtils::extractScheduleOpndList(ScheduleClause & Sched,
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
    ChunkSize = *((CI->getValue()).getRawData());
    DEBUG(dbgs() << " Schedule chunk size is constant: " << ChunkSize << "\n");
  }
  Sched.setChunk(ChunkSize);

  // Save schedule modifier info 
  Sched.setIsSchedMonotonic(ClauseInfo.getIsScheduleMonotonic());
  Sched.setIsSchedNonmonotonic(ClauseInfo.getIsScheduleNonmonotonic());
  Sched.setIsSchedSimd(ClauseInfo.getIsScheduleSimd());

  // TODO: define the print() method for ScheduleClause to print stuff below
  DEBUG(dbgs() << "=== "<< ClauseInfo.getBaseName());
  DEBUG(dbgs() << "  Chunk=" << *Sched.getChunkExpr());
  DEBUG(dbgs() << "  Monotonic=" << Sched.getIsSchedMonotonic());
  DEBUG(dbgs() << "  Nonmonotonic=" << Sched.getIsSchedNonmonotonic());
  DEBUG(dbgs() << "  Simd=" << Sched.getIsSchedSimd() << "\n");

  return;
}

void WRegionUtils::extractMapOpndList(const Use *Args, unsigned NumArgs,
                                      const ClauseSpecifier &ClauseInfo,
                                      MapClause &C, unsigned MapKind) {
  C.setClauseID(QUAL_OMP_MAP_TO); // dummy map clause id; details are in 
                                  // the MapKind of each list item

  if (ClauseInfo.getIsArraySection()) {
    //TODO: Parse array section arguments.
  } 
  else 
    for (unsigned I = 0; I < NumArgs; ++I) {
      Value *V = (Value*) Args[I];
      C.add(V);
      MapItem *MI = C.back();
      MI->setMapKind(MapKind);
    }
}

void WRegionUtils::extractDependOpndList(const Use *Args, unsigned NumArgs,
                                         const ClauseSpecifier &ClauseInfo,
                                         DependClause &C, bool IsIn) {
  C.setClauseID(QUAL_OMP_DEPEND_IN); // dummy depend clause id; 

  if (ClauseInfo.getIsArraySection()) {
    //TODO: Parse array section arguments.
  } 
  else
    for (unsigned I = 0; I < NumArgs; ++I) {
      Value *V = (Value*) Args[I];
      C.add(V);
      DependItem *DI = C.back();
      DI->setIsIn(IsIn);
    }
}

void WRegionUtils::extractReductionOpndList(const Use *Args, unsigned NumArgs,
                                      const ClauseSpecifier &ClauseInfo,
                                      ReductionClause &C, int ReductionKind) {
  C.setClauseID(QUAL_OMP_REDUCTION_ADD); // dummy reduction op
  bool IsUnsigned = ClauseInfo.getIsUnsigned();
  if (IsUnsigned)
    assert((ReductionKind==ReductionItem::WRNReductionMax ||
            ReductionKind==ReductionItem::WRNReductionMin) &&
            "The UNSIGNED modifier is for MIN/MAX reduction only");

  if (ClauseInfo.getIsArraySection()) {
    //TODO: Parse array section arguments.
  } 
  else
    for (unsigned I = 0; I < NumArgs; ++I) {
      Value *V = (Value*) Args[I];
      C.add(V);
      ReductionItem *RI = C.back();
      RI->setType((ReductionItem::WRNReductionKind)ReductionKind);
      RI->setIsUnsigned(IsUnsigned);
    }
}
#endif


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
/// \brief Fill reduction info in ReductionItem \pRI
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

  switch (ClauseID) {
  case QUAL_OMP_SHARED: {
    WRegionUtils::extractQualOpndList<SharedClause>(Args, NumArgs, ClauseID,
                                                    getShared());
    break;
  }
  case QUAL_OMP_PRIVATE: {
    WRegionUtils::extractQualOpndList<PrivateClause>(Args, NumArgs, ClauseID,
                                                     getPriv());
    break;
  }
  case QUAL_OMP_FIRSTPRIVATE: {
    WRegionUtils::extractQualOpndList<FirstprivateClause>(Args, NumArgs,
                                                       ClauseID, getFpriv());
    break;
  }
  case QUAL_OMP_LASTPRIVATE: {
    WRegionUtils::extractQualOpndList<LastprivateClause>(Args, NumArgs,
                                                     ClauseInfo, getLpriv());
    break;
  }
  case QUAL_OMP_COPYIN: {
    WRegionUtils::extractQualOpndList<CopyinClause>(Args, NumArgs, ClauseID,
                                                    getCopyin());
    break;
  }
  case QUAL_OMP_COPYPRIVATE: {
    WRegionUtils::extractQualOpndList<CopyprivateClause>(Args, NumArgs,
                                                       ClauseID, getCpriv());
    break;
  }
  case QUAL_OMP_DEPEND_IN:
  case QUAL_OMP_DEPEND_OUT:
  case QUAL_OMP_DEPEND_INOUT: {
    bool IsIn = ClauseID==QUAL_OMP_DEPEND_IN;
    WRegionUtils::extractDependOpndList(Args, NumArgs, ClauseInfo, getDepend(),
                                        IsIn);
    break;
  }
  case QUAL_OMP_DEPEND_SINK: {
    setIsDoacross(true);
    WRegionUtils::extractQualOpndList<DepSinkClause>(Args, NumArgs, ClauseID,
                                                     getDepSink());
    break;
  }
  case QUAL_OMP_IS_DEVICE_PTR: {
    WRegionUtils::extractQualOpndList<IsDevicePtrClause>(Args, NumArgs,
                                                 ClauseID, getIsDevicePtr());
    break;
  }
  case QUAL_OMP_USE_DEVICE_PTR: {
    WRegionUtils::extractQualOpndList<UseDevicePtrClause>(Args, NumArgs,
                                                ClauseID, getUseDevicePtr());
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
    WRegionUtils::extractMapOpndList(Args, NumArgs, ClauseInfo, getMap(),
                                     MapKind);
    break;
  }
  case QUAL_OMP_UNIFORM: {
    WRegionUtils::extractQualOpndList<UniformClause>(Args, NumArgs, ClauseID,
                                                     getUniform());
    break;
  }
  case QUAL_OMP_LINEAR: {
    WRegionUtils::extractQualOpndList<LinearClause>(Args, NumArgs, ClauseID,
                                                    getLinear());
    break;
  }
  case QUAL_OMP_ALIGNED: {
    WRegionUtils::extractQualOpndList<AlignedClause>(Args, NumArgs, ClauseID,
                                                     getAligned());
    break;
  }
  case QUAL_OMP_FLUSH: {
    WRegionUtils::extractQualOpndList<FlushSet>(Args, NumArgs, ClauseID,
                                                getFlush());
    break;
  }
  case QUAL_OMP_SCHEDULE_AUTO: {
    WRegionUtils::extractScheduleOpndList(getSchedule(), Args, ClauseInfo,
                                          WRNScheduleAuto);
    break;
  }
  case QUAL_OMP_SCHEDULE_DYNAMIC: {
    WRegionUtils::extractScheduleOpndList(getSchedule(), Args, ClauseInfo,
                                          WRNScheduleDynamic);
    break;
  }
  case QUAL_OMP_SCHEDULE_GUIDED: {
    WRegionUtils::extractScheduleOpndList(getSchedule(), Args, ClauseInfo,
                                          WRNScheduleGuided);
    break;
  }
  case QUAL_OMP_SCHEDULE_RUNTIME: {
    WRegionUtils::extractScheduleOpndList(getSchedule(), Args, ClauseInfo,
                                          WRNScheduleRuntime);
    break;
  }
  case QUAL_OMP_DIST_SCHEDULE_STATIC:
  case QUAL_OMP_SCHEDULE_STATIC: {
    WRegionUtils::extractScheduleOpndList(getSchedule(), Args, ClauseInfo,
                                          WRNScheduleStatic);
    break;
  }
  case QUAL_OMP_REDUCTION_ADD:
  case QUAL_OMP_REDUCTION_SUB:
  case QUAL_OMP_REDUCTION_MUL:
  case QUAL_OMP_REDUCTION_AND:
  case QUAL_OMP_REDUCTION_OR:
  case QUAL_OMP_REDUCTION_XOR:
  case QUAL_OMP_REDUCTION_BAND:
  case QUAL_OMP_REDUCTION_BOR:
  case QUAL_OMP_REDUCTION_MAX:
  case QUAL_OMP_REDUCTION_MIN:
  case QUAL_OMP_REDUCTION_UDR: {
    int ReductionKind = ReductionItem::getKindFromClauseId(ClauseID);
    assert(ReductionKind > 0 && "Bad reduction operation");
    WRegionUtils::extractReductionOpndList(Args, NumArgs, ClauseInfo, getRed(),
                                           ReductionKind);
    break;
  }
  default:
    llvm_unreachable("Unknown ClauseID in handleQualOpndList()");
    break;
  }
}

void WRegionNode::getClausesFromOperandBundles() {
  // Under the directive.region.entry/exit representation the intrinsic
  // is alone in the EntryBB, so EntryBB->front() is the intrinsic call
  Instruction *I = &(getEntryBBlock()->front());
  IntrinsicInst *Call = dyn_cast<IntrinsicInst>(&*I);
  assert (Call && "Call not found for directive.region.entry()");

  unsigned i, NumOB = Call->getNumOperandBundles();

  // Index i start from 1 (not 0) because we want to skip the first
  // OperandBundle, which is the directive name.
  for(i=1; i<NumOB; ++i) {
    // BU is the ith OperandBundle, which represents a clause
    OperandBundleUse BU = Call->getOperandBundleAt(i);

    // The clause name is the tag name
    StringRef ClauseString = BU.getTagName();

    // Extract clause properties
    ClauseSpecifier ClauseInfo(ClauseString);

    // Get the argument list from the current OperandBundle
    ArrayRef<llvm::Use> Args = BU.Inputs;
    unsigned NumArgs = Args.size();   // BU.Inputs.size()

    const Use *ArgList = NumArgs==0 ? nullptr : &Args[0]; 

    // Parse the clause and update the WRN
    parseClause(ClauseInfo, ArgList, NumArgs);
  }
}

bool WRegionNode::hasSchedule() const {
  unsigned SubClassID = getWRegionKindID();
  switch (SubClassID) {
  case WRNParallelLoop:
  case WRNDistributeParLoop:
  case WRNWksLoop:
    return true;
  }
  return false;
}

bool WRegionNode::hasShared() const {
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

bool WRegionNode::hasPrivate() const {
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
    return true;
  }
  return false;
}

bool WRegionNode::hasFirstprivate() const {
  unsigned SubClassID = getWRegionKindID();

  // similar to hasPrivate except for SIMD, 
  // which has Private but not Firstprivate
  if (SubClassID == WRNVecLoop)
    return false;
  return hasPrivate();
}

bool WRegionNode::hasLastprivate() const {
  unsigned SubClassID = getWRegionKindID();
  switch (SubClassID) {
  case WRNParallelLoop:
  case WRNDistributeParLoop:
  case WRNTaskloop:
  case WRNVecLoop:
  case WRNWksLoop:
  case WRNSections:
  case WRNDistribute:
    return true;
  }
  return false;
}

bool WRegionNode::hasReduction() const {
  unsigned SubClassID = getWRegionKindID();
  switch (SubClassID) {
  case WRNParallel:
  case WRNParallelLoop:
  case WRNParallelSections:
  case WRNParallelWorkshare:
  case WRNTeams:
  case WRNDistributeParLoop:
  // TODO: support OMP5.0 task/taskloop reduction 
  //  case WRNTask:
  //  case WRNTaskloop:
  case WRNVecLoop:
  case WRNWksLoop:
  case WRNSections:
    return true;
  }
  return false;
}

bool WRegionNode::hasCopyin() const {
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

bool WRegionNode::hasCopyprivate() const {
  unsigned SubClassID = getWRegionKindID();
  // only SINGLE can have a Copyprivate clause
  return SubClassID==WRNSingle;
}

bool WRegionNode::hasLinear() const {
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

bool WRegionNode::hasUniform() const {
  unsigned SubClassID = getWRegionKindID();
  // only SIMD can have a Uniform clause
  return SubClassID==WRNVecLoop;
}

bool WRegionNode::hasAligned() const {
  // only SIMD can have an Aligned clause
  return hasUniform();
}

bool WRegionNode::hasMap() const {
  // Only target-type constructs take map clauses
  return getIsTarget();
}

bool WRegionNode::hasIsDevicePtr() const {
  unsigned SubClassID = getWRegionKindID();
  // only WRNTargetNode can have a IsDevicePtr clause
  return SubClassID==WRNTarget;
}

bool WRegionNode::hasUseDevicePtr() const {
  unsigned SubClassID = getWRegionKindID();
  // only WRNTargetDataNode can have a UseDevicePtr clause
  return SubClassID==WRNTargetData;
}

bool WRegionNode::hasDepend() const {
  // Only task-type constructs take depend clauses
  return getIsTask();
}

bool WRegionNode::hasDepSink() const {
  unsigned SubClassID = getWRegionKindID();
  // only WRNOrderedNode can have a depend(sink : vec) clause
  // but only if its "IsDoacross" field is true
  if(SubClassID==WRNOrdered)
    return getIsDoacross();

  return false;
}

bool WRegionNode::hasFlush() const {
  unsigned SubClassID = getWRegionKindID();
  // only WRNFlushNode can have a flush set
  return SubClassID==WRNFlush;
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
  StringRef ClauseName = VPOAnalysisUtils::getClauseName(ClauseID);
  errorClause(ClauseName);
}
