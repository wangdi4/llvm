//===- VPOParoptTransform.cpp - Transformation of W-Region for threading --===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation. and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
// Authors:
// --------
// Xinmin Tian (xinmin.tian@intel.com)
//
// Major Revisions:
// ----------------
// Dec 2015: Initial Implementation of MT-code generation (Xinmin Tian)
//
//===----------------------------------------------------------------------===//
///
/// \file
/// VPOParoptTransform.cpp implements the interface to outline a work
/// region formed from parallel loop/regions/tasks into a new function,
/// replacing it with a call to the threading runtime call by passing new
/// function pointer to the runtime for parallel execution.
///
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_VPO/Paropt/VPOParoptTransform.h"
#include "llvm/Transforms/Intel_VPO/Paropt/VPOParoptAtomics.h"
#include "llvm/Transforms/Intel_VPO/Paropt/VPOParopt.h"
#include "llvm/Transforms/Intel_VPO/Utils/VPOUtils.h"

#include "llvm/IR/Function.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Support/Debug.h"

#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Constants.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetSubtargetInfo.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/CodeExtractor.h"

#include "llvm/PassAnalysisSupport.h"

#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpander.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegion.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionNode.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionUtils.h"

#include "llvm/Transforms/Utils/Intel_GeneralUtils.h"
#include "llvm/Transforms/Utils/Intel_IntrinsicUtils.h"

#include <algorithm>
#include <set>

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vpo-paropt-transform"

//
// Use with the WRNVisitor class (in WRegionUtils.h) to walk the WRGraph
// (DFS) to gather all WRegion Nodes;
//
class VPOWRegionVisitor {

public:
  WRegionListTy &WRegionList;

  VPOWRegionVisitor(WRegionListTy &WL) : WRegionList(WL) {}

  void preVisit(WRegionNode *W) {}

  // use DFS visiting of WRegionNode
  void postVisit(WRegionNode *W) { WRegionList.push_back(W); }

  bool quitVisit(WRegionNode *W) { return false; }
};

void VPOParoptTransform::gatherWRegionNodeList() {
  DEBUG(dbgs() << "\nSTART: Gather WRegion Node List\n");

  VPOWRegionVisitor Visitor(WRegionList);
  WRegionUtils::forwardVisit(Visitor, WI->getWRGraph());

  DEBUG(dbgs() << "\nEND: Gather WRegion Node List\n");
  return;
}

//
// ParPrepare mode: 
//   Perform Paropt prepare transformations for lowering and privitizing   
//
// ParTrans mode: 
//   Paropt transformations for loop partitioning and outlining
//
bool VPOParoptTransform::paroptTransforms() {

  LLVMContext &C = F->getContext();
  const DataLayout &DL = F->getParent()->getDataLayout();

  bool Changed = false;

  BasicBlock::iterator I = F->getEntryBlock().begin();

  // Setup Anchor Instuction Point
  Instruction *AI = &*I;

  //
  // Create the LOC structure. The format is based on OpenMP KMP library
  //
  // typedef struct {
  //   kmp_int32 reserved_1;   // might be used in Fortran
  //   kmp_int32 flags;        // also f.flags; KMP_IDENT_xxx flags;
  //                           // KMP_IDENT_KMPC identifies this union member
  //   kmp_int32 reserved_2;   // not really used in Fortran any more
  //   kmp_int32 reserved_3;   // source[4] in Fortran, do not use for C++
  //   char      *psource;
  // } ident_t;
  //
  // The bits that the flags field can hold are defined as KMP_IDENT_* before.
  //
  // Note: IdentTy needs to be an anonymous struct. This is because if we use
  // a named struct type, then different Types are created for each function
  // encountered. For example, consider a module with two functions `foo1()`
  // and `foo2()`. When handling foo1(), a named struct type `ident_t` would
  // be created and used for generating function declarations and calls for
  // KMPC routines such as `__kmpc_global_thread_num(ident_t*)`. When it comes
  // to handling `foo2()`, a new named IdentTy would be created, say
  // `ident_t.0`, but when trying to emit a call to `__kmpc_global_thread_num`,
  // there would be a type mismatch between the expected argument type in the
  // declaration (ident_t *) and actual type of the argument (ident_t.0 *).
  IdentTy = StructType::get(C, {Type::getInt32Ty(C),   // reserved_1
                                Type::getInt32Ty(C),   // flags
                                Type::getInt32Ty(C),   // reserved_2
                                Type::getInt32Ty(C),   // reserved_3
                                Type::getInt8PtrTy(C)} // *psource
                            );

  StringRef S = F->getName();

  if (!S.compare_lower(StringRef("@main"))) {
    CallInst *RI = VPOParoptUtils::genKmpcBeginCall(F, AI, IdentTy);
    RI->insertBefore(AI);

    for (BasicBlock &I : *F) {
      if (isa<ReturnInst>(I.getTerminator())) {
        Instruction *Inst = I.getTerminator();

        CallInst *RI = VPOParoptUtils::genKmpcEndCall(F, Inst, IdentTy);
        RI->insertBefore(Inst);
      }
    }
  }

  if (WI->WRGraphIsEmpty()) {
    DEBUG(dbgs() << "\n... No WRegion Candidates for Parallelization ...\n\n");
    return Changed;
  }

  Type *Int32Ty = Type::getInt32Ty(C);

  TidPtr = new AllocaInst(Int32Ty, DL.getAllocaAddrSpace(), "tid.addr", AI);
  TidPtr->setAlignment(4);

  if ((Mode & OmpPar) && (Mode & ParTrans)) {
    BidPtr = new AllocaInst(Int32Ty, DL.getAllocaAddrSpace(), "bid.addr", AI);
    BidPtr->setAlignment(4);
  }

  CallInst *RI = VPOParoptUtils::genKmpcGlobalThreadNumCall(F, AI, IdentTy);
  RI->insertBefore(AI);

  StoreInst *Tmp0 = new StoreInst(RI, TidPtr, false, AI);
  Tmp0->setAlignment(4);

  // Constant Definitions
  ConstantInt *ValueZero = ConstantInt::get(Type::getInt32Ty(C), 0);

  if ((Mode & OmpPar) && (Mode & ParTrans)) {
    StoreInst *Tmp1 = new StoreInst(ValueZero, BidPtr, false, AI);
    Tmp1->setAlignment(4);
  }

  gatherWRegionNodeList();

  //
  // Walk throught W-Region list, the outlining / lowering is performed from
  // inner to outer
  //
  for (auto I = WRegionList.begin(), E = WRegionList.end(); I != E; ++I) {

    WRegionNode *W = *I;

    switch (W->getWRegionKindID()) {

    // Parallel constructs need to perform outlining
    case WRegionNode::WRNParallel:
      DEBUG(dbgs() << "\n WRegionNode::WRNParallel - Transformation \n\n");
      Changed = genPrivatizationCode(W);

      if ((Mode & OmpPar) && (Mode & ParTrans))
        Changed |= genMultiThreadedCode(W);
      break;
    case WRegionNode::WRNParallelLoop:
      DEBUG(dbgs() << "\n WRegionNode::WRNParallelLoop - Transformation \n\n");

      if ((Mode & OmpPar) && (Mode & ParTrans))
        Changed = genLoopSchedulingCode(W);
  
      Changed |= genPrivatizationCode(W);

      if ((Mode & OmpPar) && (Mode & ParTrans))
        Changed |= genMultiThreadedCode(W);

    case WRegionNode::WRNParallelSections:
      break;

    // Task constructs need to perform outlining
    case WRegionNode::WRNTask:
    case WRegionNode::WRNTaskloop:
      break;

    // Constructs do not need to perform outlining
    case WRegionNode::WRNAtomic:
      DEBUG(dbgs() << "\nWRegionNode::WRNAtomic - Transformation \n\n");
      if (Mode & ParPrepare) 
        Changed |= VPOParoptAtomics::handleAtomic(dyn_cast<WRNAtomicNode>(W),
                                                  IdentTy, TidPtr);
      break;
    case WRegionNode::WRNVecLoop:
    case WRegionNode::WRNWksLoop:
    case WRegionNode::WRNSections:
      break;
    case WRegionNode::WRNSingle:
      DEBUG(dbgs() << "\nWRegionNode::WRNSingle - Transformation \n\n");
      if (Mode & ParPrepare) 
        Changed = genSingleThreadCode(W);
      break;
    case WRegionNode::WRNMaster:
      DEBUG(dbgs() << "\nWRegionNode::WRNMaster - Transformation \n\n");
      if (Mode & ParPrepare) 
        Changed |= genMasterThreadCode(W);
      break;
    case WRegionNode::WRNBarrier:
    case WRegionNode::WRNCancel:
    case WRegionNode::WRNCritical:
      DEBUG(dbgs() << "\nWRegionNode::WRNCritical - Transformation \n\n");
      if (Mode & ParPrepare) 
        Changed |= genCriticalCode(dyn_cast<WRNCriticalNode>(W));
      break;
    case WRegionNode::WRNFlush:
      break;
    case WRegionNode::WRNOrdered:
      DEBUG(dbgs() << "\nWRegionNode::WRNOrdered - Transformation \n\n");
      if (Mode & ParPrepare) 
        Changed = genOrderedThreadCode(W);
      break;
    case WRegionNode::WRNTaskgroup:
      break;
    default:
      break;
    }
  }

  for (WRegionNode *R : WRegionList)
    delete R;
  WRegionList.clear();

  return Changed;
}

bool VPOParoptTransform::genPrivatizationCode(WRegionNode *W) {

  SetVector<Value *> PrivUses;

  bool Changed = false;

  BasicBlock *EntryBB = W->getEntryBBlock();

  DEBUG(dbgs() << "\n WRegionNode: Invoke Privatization \n\n");

  // Return false when W-Region is empty
  if (W->isBBSetEmpty())
    return Changed;

  // Process all PrivateItems in the private clause  
  if (auto PrivClause = W->getPriv()) {

    // Walk through each PrivateItem list in the private clause to perform 
    // privatization for each Value item
    for (PrivateItem *PrivI : PrivClause->items()) {

      AllocaInst *PrivInst;
      AllocaInst *NewPrivInst;

      if (isa<Argument>(PrivI->getOrig())) { 
        // PrivItem can be a function argument
        DEBUG(dbgs() << " Private Argument: " << *PrivI->getOrig() << "\n");
      }
      else if ((PrivInst = dyn_cast<AllocaInst>(PrivI->getOrig()))) {

        // DEBUG(dbgs() << "Private Instruction Defs: " << *PrivInst << "\n");

        // Generate a new Alloca instruction as privatization action
        NewPrivInst = (AllocaInst *)PrivInst->clone();

        // Add 'priv' suffix for the new alloca instruction  
        if (PrivInst->hasName())
          NewPrivInst->setName(PrivInst->getName()+".priv");

        Instruction *InsertPt = &EntryBB->front();
        NewPrivInst->insertAfter(InsertPt);

        for (auto *User : PrivInst->users()) {
          auto II = W->bbset_begin(); 
          auto IE = W->bbset_end();

          // Instruction *UI = dyn_cast<Instruction>(User);

          // Collect all USEs of PrivItems in the W-Region
          if (std::find(II, IE, cast<Instruction>(User)->getParent()) != IE) {
            if (!PrivUses.count(User)) { 
              // DEBUG(dbgs() << "Inst uses PrivItem: " << *UI << "\n");
              PrivUses.insert(User);
            }
          }
        }
      }

      // Replace all USEs of each PrivItem with its new PrivItem in the 
      // W-Region (parallel loop/region/section ... etc.)
      for (SetVector<Value *>::const_iterator I = PrivUses.begin(), 
           E = PrivUses.end(); I != E; ++I) {

        Value *V = *I;

        Instruction *UI = dyn_cast<Instruction>(V);
        UI->replaceUsesOfWith(PrivInst, NewPrivInst);
        // DEBUG(dbgs() << "New Instruction uses PrivItem: " << *UI << "\n");
      }
    }
    Changed = true;
  }

  // After Privatization is done, the SCEV should be re-generated 
  if (WRNParallelLoopNode *WL = dyn_cast<WRNParallelLoopNode>(W)) {
    Loop *L = WL->getLoop();

    if (SE)
       SE->forgetLoop(L);
  }
  return Changed;
}

bool VPOParoptTransform::genLoopSchedulingCode(WRegionNode *W) {

  const DataLayout &DL = F->getParent()->getDataLayout();
  bool Changed = false;

  WRNParallelLoopNode *WL = dyn_cast<WRNParallelLoopNode>(W);
  Loop *L = WL->getLoop();

  BasicBlock *EntryBB = W->getEntryBBlock();
  BasicBlock *ExitBB = W->getExitBBlock();

  DEBUG(dbgs() << "--- Parallel For LoopInfo: \n" << *L);
  DEBUG(dbgs() << "--- Loop Preheader: " << *(L->getLoopPreheader()) << "\n");
  DEBUG(dbgs() << "--- Loop Header: " << *(L->getHeader()) << "\n");
  DEBUG(dbgs() << "--- Loop Latch: " << *(L->getLoopLatch()) << "\n\n");

#if 0
  DEBUG(dbgs() << "---- Loop Induction: "
               << *(L->getCanonicalInductionVariable()) << "\n\n");
  L->dump();
#endif

  assert(L->isLoopSimplifyForm() && "should follow from addRequired<>");

  BasicBlock *LoopPreheader = L->getLoopPreheader();
  BasicBlock *LoopHeader = L->getHeader();
  BasicBlock *LoopLatch = L->getLoopLatch();

  StoreInst  *InitInst = nullptr;
  BranchInst *ExitBrInst = nullptr;

  // 
  // This is initial implementation of parallel loop scheduling to get 
  // a simple loop to work end-to-end.
  // 
  // TBD: handle all loop forms: Top test loop, bottom test loop, with 
  // PHI and without PHI nodes as SCEV bails out for many cases
  //

  // Identify branch instruction of loop latch
  if (L->isLoopExiting(LoopHeader)) {
    ExitBrInst = dyn_cast<BranchInst>(&*LoopHeader->rbegin());
    //LoopCount = SE->getExitCount(L, LoopHeader);
  } else if (L->isLoopExiting(LoopLatch)) {
    ExitBrInst = dyn_cast<BranchInst>(&*LoopLatch->rbegin());
    //LoopCount = SE->getExitCount(L, LoopLatch);
  }

  Value *LeftValue = nullptr;
  Value *RightValue = nullptr;

  Value *LpIndex = nullptr;
  Value *InitVal = nullptr;
  Value *InitInx = nullptr;

  IntegerType *IndValTy = nullptr;

  ICmpInst *CondInst = dyn_cast<ICmpInst>(ExitBrInst->getCondition());

  if (CondInst && isa<IntegerType>(CondInst->getOperand(0)->getType())) {

    // DEBUG(dbgs() << "---- Loop Control Instruction: " << *CondInst << "\n");
    // ICmpInst::Predicate Pred = CondInst->getPredicate();

    // Identify the loop index through loop control instruction
    LeftValue = CondInst->getOperand(0);
    const SCEV *LeftSCEV = SE->getSCEV(LeftValue);

    RightValue = CondInst->getOperand(1);
    const SCEV *RightSCEV = SE->getSCEV(RightValue);

    if (SE->isLoopInvariant(LeftSCEV, L)) {
      IndValTy = cast<IntegerType>(LeftValue->getType());
      DEBUG(dbgs() << "---- Is Loop Inv Left SCEV: " << *LeftSCEV << "\n");

      if (LoadInst *LoadInd = dyn_cast<LoadInst>(RightValue)) {
        LpIndex = LoadInd->getOperand(1);
        // DEBUG(dbgs() << "---- Loop Index : " << *LpIndex << "\n");
      }

    } else if (SE->isLoopInvariant(RightSCEV, L)) {
      IndValTy = cast<IntegerType>(LeftValue->getType());
      DEBUG(dbgs() << "---- Is Loop Inv Right SCEV: " << *RightSCEV << "\n");

      if (LoadInst *LoadInd = dyn_cast<LoadInst>(LeftValue)) {
        LpIndex = LoadInd->getOperand(0);
        // DEBUG(dbgs() << "---- Loop Index : " << *LpIndex << "\n");
      }
    }
  } 

  if (LoopPreheader) {
    InitInst = dyn_cast<StoreInst>(&*LoopPreheader->begin());
    InitVal = InitInst->getOperand(0);

    InitInx = InitInst->getOperand(1);

    if (LpIndex && InitInx && LpIndex == InitInx)  
      DEBUG(dbgs() << "---- Find loop index : " << *InitInx << "\n\n");
    else 
      return Changed;
  }

  Instruction *InsertPt = dyn_cast<Instruction>(&*EntryBB->rbegin());

  AllocaInst *IsLastVal = new AllocaInst(IndValTy, DL.getAllocaAddrSpace(),
                                         "is.last", InsertPt);
  IsLastVal->setAlignment(4);

  AllocaInst *LowerBnd = new AllocaInst(IndValTy, DL.getAllocaAddrSpace(),
                                        "lower.bnd", InsertPt);
  LowerBnd->setAlignment(4);

  AllocaInst *UpperBnd = new AllocaInst(IndValTy, DL.getAllocaAddrSpace(),
                                        "upper.bnd", InsertPt);
  UpperBnd->setAlignment(4);

  AllocaInst *Stride = new AllocaInst(IndValTy, DL.getAllocaAddrSpace(),
                                      "stride", InsertPt);
  Stride->setAlignment(4);

  // UpperD is for distribtue loop
  AllocaInst *UpperD = new AllocaInst(IndValTy, DL.getAllocaAddrSpace(),
                                      "upperD", InsertPt);
  UpperD->setAlignment(4);

  // Constant Definitions
  ConstantInt *ValueZero = ConstantInt::get(IndValTy, 0);
  ConstantInt *ValueOne  = ConstantInt::get(IndValTy, 1);

  // For now, set the default schedule type static_even.     
  // TBD: to get Schedule type and chunk information from W-Region node
  ConstantInt *SchedType = ConstantInt::get(IndValTy, 34);

  StoreInst *Tmp0 = new StoreInst(InitVal, LowerBnd, false, InsertPt);
  Tmp0->setAlignment(4);

  StoreInst *Tmp1 = new StoreInst(RightValue, UpperBnd, false, InsertPt);
  Tmp1->setAlignment(4);

  StoreInst *Tmp2 = new StoreInst(ValueOne, Stride, false, InsertPt);
  Tmp2->setAlignment(4);

  StoreInst *Tmp3 = new StoreInst(RightValue, UpperD, false, InsertPt);
  Tmp3->setAlignment(4);

  StoreInst *Tmp4 = new StoreInst(ValueZero, IsLastVal, false, InsertPt);
  Tmp4->setAlignment(4);

  LoadInst *LoadTid = new LoadInst(TidPtr, "my.tid", InsertPt);
  LoadTid->setAlignment(4);

  CallInst* StaticInitCall = VPOParoptUtils::genKmpcStaticInit(W, IdentTy,
                               LoadTid, SchedType, IsLastVal, LowerBnd, 
                               UpperBnd, Stride, ValueOne, ValueOne, InsertPt);

  StaticInitCall->setCallingConv(CallingConv::C);

  LoadInst *LoadLB = new LoadInst(LowerBnd, "lb.new", InsertPt);
  LoadLB->setAlignment(4);

  LoadInst *LoadUB = new LoadInst(UpperBnd, "ub.new", InsertPt);
  LoadUB->setAlignment(4);

  InitInst->setOperand(0, LoadLB);
  CondInst->setOperand(1, LoadUB);

  InsertPt = dyn_cast<Instruction>(&*ExitBB->rbegin());
  CallInst* StaticFiniCall = VPOParoptUtils::genKmpcStaticFini(W, IdentTy,
                               LoadTid, InsertPt);

  StaticFiniCall->setCallingConv(CallingConv::C);
  Changed = true;
  return Changed;
}

bool VPOParoptTransform::genMultiThreadedCode(WRegionNode *W) {
  bool Changed = false;

  // brief extract a W-Region to generate a function
  CodeExtractor CE(makeArrayRef(W->bbset_begin(), W->bbset_end()), DT, false);

  assert(CE.isEligible());

  // Set up Fn Attr for the new function
  if (Function *NewF = CE.extractCodeRegion()) {

    // Set up the Calling Convention used by OpenMP Runtime Library
    CallingConv::ID CC = CallingConv::C;

    DT->verifyDomTree();

    // Adjust the calling convention for both the function and the
    // call site.
    NewF->setCallingConv(CC);

    assert(NewF->hasOneUse() && "New function should have one use");
    User *U = NewF->user_back();

    CallInst *NewCall = cast<CallInst>(U);
    NewCall->setCallingConv(CC);

    CallSite CS(NewCall);

    unsigned int TidArgNo = 0;
    bool IsTidArg = false;

    for (auto I = CS.arg_begin(), E = CS.arg_end(); I != E; ++I) {
      if (*I == TidPtr)  {
        IsTidArg = true;
        DEBUG(dbgs() << " NewF Tid Argument: " << *(*I) << "\n");
        break;
      }
      ++TidArgNo;
    }

    // Finalized multithreaded Function declaration and definition
    Function *MTFn = finalizeExtractedMTFunction(W, NewF, IsTidArg, TidArgNo);

    std::vector<Value *> MTFnArgs;

    // Pass tid and bid arguments.
    MTFnArgs.push_back(TidPtr);
    MTFnArgs.push_back(BidPtr);
    genThreadedEntryActualParmList(W, MTFnArgs);

    DEBUG(dbgs() << " New Call to MTFn: " << *NewCall << "\n"); 
    // Pass all the same arguments of the extracted function.
    for (auto I = CS.arg_begin(), E = CS.arg_end(); I != E; ++I) {
      if (*I != TidPtr)  {
        DEBUG(dbgs() << " NewF Arguments: " << *(*I) << "\n"); 
        MTFnArgs.push_back((*I));
      }
    }

    CallInst *MTFnCI = CallInst::Create(MTFn, MTFnArgs, "", NewCall);
    MTFnCI->setCallingConv(CS.getCallingConv());

    // Copy isTailCall attribute
    if (NewCall->isTailCall())
      MTFnCI->setTailCall();

    MTFnCI->setDebugLoc(NewCall->getDebugLoc());

    // MTFnArgs.clear();

    if (!NewCall->use_empty())
      NewCall->replaceAllUsesWith(MTFnCI);

    // Keep the orginal extraced function name after finalization
    MTFnCI->takeName(NewCall);

    // Remove the orginal serial call to extracted NewF from the program,
    // reducing the use-count of NewF
    NewCall->eraseFromParent();

    // Finally, nuke the original extracted function.
    NewF->eraseFromParent();

    // Geneate _kmpc_fork_call for multithreaded execution of MTFn call
    CallInst* ForkCI = genForkCallInst(W, MTFnCI);

    // Geneate __kmpc_ok_to_fork test Call Instruction
    CallInst* ForkTestCI = VPOParoptUtils::genKmpcForkTest(W, IdentTy, ForkCI);

    // 
    // Genrerate __kmpc_ok_to_fork test for taking either __kmpc_fork_call
    // or serial call branch, and update CFG and DomTree  
    // 
    //  ForkTestBB(codeRepl)
    //         /    \
    //        /      \
    // ThenForkBB   ElseCallBB 
    //        \       / 
    //         \     /
    //  SuccessorOfThenForkBB
    //
    BasicBlock *ForkTestBB = ForkTestCI->getParent();

    BasicBlock *ForkBB = ForkCI->getParent();

    BasicBlock *ThenForkBB = SplitBlock(ForkBB, ForkCI, DT, LI);
    ThenForkBB->setName("if.then.fork." + Twine(W->getNumber()));

    BasicBlock *CallBB = MTFnCI->getParent();

    BasicBlock *ElseCallBB = SplitBlock(CallBB, MTFnCI, DT, LI);
    ElseCallBB->setName("if.else.call." + Twine(W->getNumber()));

    Function *F = ForkTestBB->getParent();
    LLVMContext &C = F->getContext();

    ConstantInt *ValueOne = ConstantInt::get(Type::getInt32Ty(C), 1);

    TerminatorInst *TermInst = ForkTestBB->getTerminator();

    ICmpInst* CondInst = new ICmpInst(TermInst, ICmpInst::ICMP_EQ, 
                                      ForkTestCI, ValueOne, "");

    TerminatorInst *NewTermInst = BranchInst::Create(ThenForkBB, ElseCallBB,
                                                     CondInst);
    ReplaceInstWithInst(TermInst, NewTermInst);

    TerminatorInst *NewForkBI = BranchInst::Create(
                                  ElseCallBB->getTerminator()->getSuccessor(0));

    ReplaceInstWithInst(ThenForkBB->getTerminator(), NewForkBI);


    DT->changeImmediateDominator(ForkTestCI->getParent(),
                                 ThenForkBB->getTerminator()->getSuccessor(0));

    // Remove the serial call to MTFn function from the program, reducing
    // the use-count of MTFn
    // MTFnCI->eraseFromParent();

    // Remove calls to directive intrinsics since the LLVM back end does not
    // know how to translate them.
    VPOUtils::stripDirectives(W);

    Changed = true;
  }

  return Changed;
}

CallInst* VPOParoptTransform::genForkCallInst(WRegionNode *W, CallInst *CI) {
  Module *M = F->getParent();
  LLVMContext &C = F->getContext();

  // Get MicroTask Function for __kmpc_fork_call
  Function *MicroTaskFn = CI->getCalledFunction();
  FunctionType *MicroTaskFnTy = MicroTaskFn->getFunctionType();

  // Get MicroTask Function for __kmpc_fork_call
  //
  // Need to add global_tid and bound_tid to Micro Task Function, 
  // finalizeExtractedMTFunction is implemented for adding Tid and Bid 
  // arguments :
  //   void (*kmpc_micro)(kmp_int32 global_tid, kmp_int32 bound_tid,...)
  //
  // geneate void __kmpc_fork_call(ident_t *loc,
  //                               kmp_int32 argc, (*kmpc_microtask)(), ...);
  //
  Type *ForkParams[] = {PointerType::getUnqual(IdentTy), Type::getInt32Ty(C),
                        PointerType::getUnqual(MicroTaskFnTy)};

  FunctionType *FnTy = FunctionType::get(Type::getVoidTy(C), ForkParams, true);

  Function *ForkCallFn = M->getFunction("__kmpc_fork_call");

  if (!ForkCallFn) {
    ForkCallFn = Function::Create(FnTy, GlobalValue::ExternalLinkage,
                                  "__kmpc_fork_call", M);
    ForkCallFn->setCallingConv(CallingConv::C);
  }

  AttributeList ForkCallFnAttr;
  SmallVector<AttributeList, 4> Attrs;

  AttributeList FnAttrSet;
  AttrBuilder B;
  FnAttrSet = AttributeList::get(C, ~0U, B);

  Attrs.push_back(FnAttrSet);
  ForkCallFnAttr = AttributeList::get(C, Attrs);

  ForkCallFn->setAttributes(ForkCallFnAttr);

  // get source location information from DebugLoc
  BasicBlock *EntryBB = W->getEntryBBlock();
  BasicBlock *ExitBB = W->getExitBBlock();

  GlobalVariable *KmpcLoc = VPOParoptUtils::genKmpcLocfromDebugLoc(
      F, CI, IdentTy, KMP_IDENT_KMPC, EntryBB, ExitBB);

  CallSite CS(CI);
  ConstantInt *NumArgs = ConstantInt::get(Type::getInt32Ty(C), 
                                          CS.getNumArgOperands()-2);

  std::vector<Value *> Params;
  Params.push_back(KmpcLoc);
  Params.push_back(NumArgs);
  Params.push_back(MicroTaskFn);

  auto InitArg = CS.arg_begin(); ++InitArg; ++InitArg; 

  for (auto I = InitArg, E = CS.arg_end(); I != E; ++I) {
    Params.push_back((*I));
  }

  CallInst *ForkCallInst = CallInst::Create(ForkCallFn, Params, "", CI);

  // CI->replaceAllUsesWith(NewCI);

  ForkCallInst->setCallingConv(CallingConv::C);
  ForkCallInst->setTailCall(false);

  return ForkCallInst;
}

// Generates the acutal parameters in the outlined function for
// copyin variables.
void VPOParoptTransform::genThreadedEntryActualParmList(WRegionNode *W,
                                          std::vector<Value *>& MTFnArgs) {

  if (auto CP = W->getCopyin()) {
    for (auto C : CP->items()) {
      MTFnArgs.push_back(C->getOrig());
    }
  }
}

// Generates the formal parameters in the outlined function for 
// copyin variables. It can be extended for other variables including
// firstprivte, shared and etc. 
void VPOParoptTransform::genThreadedEntryFormalParmList(WRegionNode *W,
                                          std::vector<Type *>& ParamsTy) {

  if (auto CP = W->getCopyin()) {
    for (auto C : CP->items()) {
      ParamsTy.push_back(C->getOrig()->getType());
    }
  }
}

// Fix the name of copyin formal parameters for outlined function.
void VPOParoptTransform::fixThreadedEntryFormalParmName(WRegionNode *W,
                                                        Function *NFn) {

  if (auto CP = W->getCopyin()) {
    Function::arg_iterator NewArgI = NFn->arg_begin();
    ++NewArgI;
    ++NewArgI;
    for (auto C : CP->items()) {
      NewArgI->setName("tpv_"+C->getOrig()->getName());
      ++NewArgI;
    }
  }
}

// Emit the code for copyin variable. One example is as follows.
//   %0 = ptrtoint i32* %tpv_a to i64
//   %1 = icmp ne i64 %0, ptrtoint (i32* @a to i64)
//   br i1 %1, label %copyin.not.master, label %copyin.not.master.end
//
// copyin.not.master:                                ; preds = %newFuncRoot
//   %2 = bitcast i32* %tpv_a to i8*
//   call void @llvm.memcpy.p0i8.p0i8.i32(i8* bitcast (i32* @a to i8*), i8* %2, i32 4, i32 4, i1 false)
//   br label %copyin.not.master.end
//
// copyin.not.master.end:                            ; preds = %newFuncRoot, %copyin.not.master
//
void VPOParoptTransform::genTpvCopyIn(WRegionNode *W,
                                      Function *NFn) {

  if (auto CP = W->getCopyin()) {
    Function::arg_iterator NewArgI = NFn->arg_begin();
    ++NewArgI;
    ++NewArgI;
    const DataLayout NDL=NFn->getParent()->getDataLayout();
    bool FirstArg = true;

    for (auto C : CP->items()) {
      TerminatorInst *Term;
      if (FirstArg) {
        FirstArg = false;

        IRBuilder<> Builder(&NFn->getEntryBlock());
        Builder.SetInsertPoint(NFn->getEntryBlock().getTerminator());

        // The instruction to cast the tpv pointer to int for later comparison
        // instruction. One example is as follows.
        //   %0 = ptrtoint i32* %tpv_a to i64
        Value *TpvArg = Builder.CreatePtrToInt(
                                        &*NewArgI,Builder.getIntPtrTy(NDL));
        Value *OldTpv = Builder.CreatePtrToInt(
                                        C->getOrig(),Builder.getIntPtrTy(NDL));

        // The instruction to compare between the address of tpv formal
        // arugment and the tpv accessed in the outlined function. 
        // One example is as follows.
        //   %1 = icmp ne i64 %0, ptrtoint (i32* @a to i64)
        Value *PtrCompare = Builder.CreateICmpNE(TpvArg, OldTpv);
        Term = 
          SplitBlockAndInsertIfThen(PtrCompare,
                                    NFn->getEntryBlock().getTerminator(),
                                    false);

        // Set the name for the newly generated basic blocks.
        Term->getParent()->setName("copyin.not.master");
        NFn->getEntryBlock().getTerminator()
            ->getSuccessor(1)->setName("copyin.not.master.end");
      }
      VPOParoptUtils::genMemcpy(C->getOrig(),
                                &*NewArgI,
                                NDL,
                                Term->getParent());

      ++NewArgI;
    }
  }
}

Function *VPOParoptTransform::finalizeExtractedMTFunction(
  WRegionNode *W,
  Function *Fn, 
  bool IsTidArg, unsigned int TidArgNo) {

  LLVMContext &C = Fn->getContext();

  // Computing a new prototype for the function, which is the same as
  // the old function with two new parameters for passing tid and bid
  // required by OpenMP runtime library.
  FunctionType *FnTy = Fn->getFunctionType();

  std::vector<Type *> ParamsTy;

  ParamsTy.push_back(PointerType::getUnqual(Type::getInt32Ty(C)));
  ParamsTy.push_back(PointerType::getUnqual(Type::getInt32Ty(C)));

  genThreadedEntryFormalParmList(W, ParamsTy);

  unsigned int TidParmNo = 0;
  for (auto ArgTyI = FnTy->param_begin(), ArgTyE = FnTy->param_end();
       ArgTyI != ArgTyE; ++ArgTyI) {

    // Matching formal argument and actual argument for Thread ID
    if (!IsTidArg || TidParmNo != TidArgNo) 
      ParamsTy.push_back(*ArgTyI);

    ++TidParmNo;
  }

  Type *RetTy = FnTy->getReturnType();
  FunctionType *NFnTy = FunctionType::get(RetTy, ParamsTy, false);

  // Create the new function body and insert it into the module...
  Function *NFn = Function::Create(NFnTy, Fn->getLinkage());

  NFn->copyAttributesFrom(Fn);
  NFn->addFnAttr("mt-func", "true");

  Fn->getParent()->getFunctionList().insert(Fn->getIterator(), NFn);
  NFn->takeName(Fn);

  // Since we have now created the new function, splice the body of the old
  // function right into the new function, leaving the old rotting hulk of
  // the function empty.
  NFn->getBasicBlockList().splice(NFn->begin(), Fn->getBasicBlockList());

  // Loop over the argument list, transferring uses of the old arguments over
  // to the new arguments, also transferring over the names as well.
  Function::arg_iterator NewArgI = NFn->arg_begin();


  // The first argument is *tid - thread id argument
  NewArgI->setName("tid");
  ++NewArgI;

  // The second argument is *bid - binding thread id argument
  NewArgI->setName("bid");
  ++NewArgI;

  fixThreadedEntryFormalParmName(W, NFn);
  genTpvCopyIn(W, NFn);

  // For each argument, move the name and users over to the new version.
  TidParmNo = 0;
  for (Function::arg_iterator I = Fn->arg_begin(), 
                              E = Fn->arg_end(); I != E; ++I) {
    // Matching formal argument and actual argument for Thread ID
    if (IsTidArg && TidParmNo == TidArgNo) {
      Function::arg_iterator TidArgI = NFn->arg_begin();
      I->replaceAllUsesWith(&*TidArgI);
      TidArgI->takeName(&*I);
    } else {
      I->replaceAllUsesWith(&*NewArgI);
      NewArgI->takeName(&*I);
      ++NewArgI;
    }
    ++TidParmNo;
  }

  DenseMap<const Function *, DISubprogram *> FunctionDIs;

  // Patch the pointer to LLVM function in debug info descriptor.
  auto DI = FunctionDIs.find(Fn);
  if (DI != FunctionDIs.end()) {
    DISubprogram *SP = DI->second;
    // SP->replaceFunction(NFn);

    // Ensure the map is updated so it can be reused on non-varargs argument
    // eliminations of the same function.
    FunctionDIs.erase(DI);
    FunctionDIs[NFn] = SP;
  }
  return NFn;
}

// Generate code for master/end master construct and update LLVM control-flow 
// and dominator tree accordingly 
bool VPOParoptTransform::genMasterThreadCode(WRegionNode *W) {
  bool Changed = false;

  BasicBlock *EntryBB = W->getEntryBBlock();
  BasicBlock *ExitBB = W->getExitBBlock();

  Instruction *InsertPt = dyn_cast<Instruction>(&*EntryBB->rbegin());

  // Geneate __kmpc_master Call Instruction
  CallInst* MasterCI = VPOParoptUtils::genKmpcMasterOrEndMasterCall(W, 
                         IdentTy, TidPtr, InsertPt, true);
  MasterCI->insertBefore(InsertPt);

  //DEBUG(dbgs() << " MasterCI: " << *MasterCI << "\n\n");

  Instruction *InsertEndPt = dyn_cast<Instruction>(&*ExitBB->rbegin());

  // Geneate __kmpc_end_master Call Instruction
  CallInst* EndMasterCI = VPOParoptUtils::genKmpcMasterOrEndMasterCall(W, 
                            IdentTy, TidPtr, InsertEndPt, false);
  EndMasterCI->insertBefore(InsertEndPt);

  // Generate (int)__kmpc_master(&loc, tid) test for executing code using 
  // Master thread. 
  //
  // __kmpc_master return: 1: master thread, 0: non master thread 
  // 
  //      MasterBBTest
  //         /    \
  //        /      \
  //   MasterBB   emptyBB 
  //        \      / 
  //         \    /
  //   SuccessorOfMasterBB
  //
  BasicBlock *MasterTestBB = MasterCI->getParent();
  BasicBlock *MasterBB = EndMasterCI->getParent();

  BasicBlock *ThenMasterBB = MasterTestBB->getTerminator()->getSuccessor(0);
  BasicBlock *SuccEndMasterBB = MasterBB->getTerminator()->getSuccessor(0);

  ThenMasterBB->setName("if.then.master." + Twine(W->getNumber()));

  Function *F = MasterTestBB->getParent();
  LLVMContext &C = F->getContext();

  ConstantInt *ValueOne = ConstantInt::get(Type::getInt32Ty(C), 1);

  TerminatorInst *TermInst = MasterTestBB->getTerminator();

  ICmpInst* CondInst = new ICmpInst(TermInst, ICmpInst::ICMP_EQ, 
                                    MasterCI, ValueOne, "");

  TerminatorInst *NewTermInst = BranchInst::Create(ThenMasterBB, 
                                                   SuccEndMasterBB, CondInst);
  ReplaceInstWithInst(TermInst, NewTermInst);

  DT->changeImmediateDominator(MasterCI->getParent(),
                               ThenMasterBB->getTerminator()->getSuccessor(0));

  // Remove calls to directive intrinsics since the LLVM back end does not
  // know how to translate them.
  VPOUtils::stripDirectives(W);

  return Changed;
}

// Generate code for single/end single construct and update LLVM control-flow
// and dominator tree accordingly
bool VPOParoptTransform::genSingleThreadCode(WRegionNode *W) {
  bool Changed = false;

  BasicBlock *EntryBB = W->getEntryBBlock();
  BasicBlock *ExitBB = W->getExitBBlock();

  Instruction *InsertPt = dyn_cast<Instruction>(&*EntryBB->rbegin());

  // Geneate __kmpc_single Call Instruction
  CallInst* SingleCI = VPOParoptUtils::genKmpcSingleOrEndSingleCall(W,
                         IdentTy, TidPtr, InsertPt, true);
  SingleCI->insertBefore(InsertPt);

  Instruction *InsertEndPt = dyn_cast<Instruction>(&*ExitBB->rbegin());

  // Geneate __kmpc_end_single Call Instruction
  CallInst* EndSingleCI = VPOParoptUtils::genKmpcSingleOrEndSingleCall(W,
                            IdentTy, TidPtr, InsertEndPt, false);
  EndSingleCI->insertBefore(InsertEndPt);

  // Generate (int)__kmpc_single(&loc, tid) test for executing code using 
  // Single thread, the  __kmpc_single return: 
  // 
  //    1: the single region can be executed by the current encounting 
  //       thread in the team.
  // 
  //    0: the single region can not be executed by the current encounting 
  //       thread, as it has been executed by another thread in the team.
  //
  //      SingleBBTest
  //         /    \
  //        /      \
  //   SingleBB   emptyBB
  //        \      /
  //         \    /
  //   SuccessorOfSingleBB
  //
  BasicBlock *SingleTestBB = SingleCI->getParent();
  BasicBlock *EndSingleBB = EndSingleCI->getParent();

  BasicBlock *ThenSingleBB = SingleTestBB->getTerminator()->getSuccessor(0);
  BasicBlock *EndSingleSuccBB = EndSingleBB->getTerminator()->getSuccessor(0);

  ThenSingleBB->setName("if.then.single." + Twine(W->getNumber()));

  Function *F = SingleTestBB->getParent();
  LLVMContext &C = F->getContext();

  ConstantInt *ValueOne = ConstantInt::get(Type::getInt32Ty(C), 1);

  TerminatorInst *TermInst = SingleTestBB->getTerminator();

  ICmpInst* CondInst = new ICmpInst(TermInst, ICmpInst::ICMP_EQ,
                                    SingleCI, ValueOne, "");

  TerminatorInst *NewTermInst = BranchInst::Create(ThenSingleBB,
                                                   EndSingleSuccBB, CondInst);
  ReplaceInstWithInst(TermInst, NewTermInst);

  DT->changeImmediateDominator(SingleCI->getParent(),
                               ThenSingleBB->getTerminator()->getSuccessor(0));

  // Remove calls to directive intrinsics since the LLVM back end does not
  // know how to translate them.
  VPOUtils::stripDirectives(W);

  return Changed;
}

// Generate code for ordered/end ordered construct for preserving ordered
// region execution order
bool VPOParoptTransform::genOrderedThreadCode(WRegionNode *W) {
  bool Changed = false;

  BasicBlock *EntryBB = W->getEntryBBlock();
  BasicBlock *ExitBB = W->getExitBBlock();

  // Generate (void)__kmpc_ordered(&loc, tid) and 
  //          (void)__kmpc_end_ordered(&loc, tid) calls
  // for executing the ordered code region
  //
  //       OrderedBB
  //         /    \
  //        /      \
  //       BB ...  BB
  //        \      /
  //         \    /
  //      EndOrderedBB

  Instruction *InsertPt = dyn_cast<Instruction>(&*EntryBB->rbegin());

  // Geneate __kmpc_ordered Call Instruction
  CallInst* OrderedCI = VPOParoptUtils::genKmpcOrderedOrEndOrderedCall(W,
                          IdentTy, TidPtr, InsertPt, true);
  OrderedCI->insertBefore(InsertPt);

  Instruction *InsertEndPt = dyn_cast<Instruction>(&*ExitBB->rbegin());

  // Geneate __kmpc_end_ordered Call Instruction
  CallInst* EndOrderedCI = VPOParoptUtils::genKmpcOrderedOrEndOrderedCall(W,
                             IdentTy, TidPtr, InsertEndPt, false);
  EndOrderedCI->insertBefore(InsertEndPt);

  //BasicBlock *OrderedBB = OrderedCI->getParent();
  //DEBUG(dbgs() << " Ordered Entry BBlock: " << *OrderedBB << "\n\n");

  //BasicBlock *EndOrderedBB = EndOrderedCI->getParent();
  //DEBUG(dbgs() << " Ordered Exit BBlock: " << *EndOrderedBB << "\n\n");

  // Remove calls to directive intrinsics since the LLVM back end does not
  // know how to translate them.
  VPOUtils::stripDirectives(W);

  return Changed;
}

// Generates code for the OpenMP critical construct.
bool VPOParoptTransform::genCriticalCode(WRNCriticalNode *CriticalNode) {
  assert(CriticalNode != nullptr && "Critical node is null.");

  assert(IdentTy != nullptr && "IdentTy is null.");
  assert(TidPtr != nullptr && "TidPtr is null.");

  StringRef LockNameSuffix = CriticalNode->getUserLockName();

  bool CriticalCallsInserted =
      LockNameSuffix.empty()
          ? VPOParoptUtils::genKmpcCriticalSection(CriticalNode, IdentTy,
                                                   TidPtr)
          : VPOParoptUtils::genKmpcCriticalSection(CriticalNode, IdentTy,
                                                   TidPtr, LockNameSuffix);

  DEBUG(dbgs() << __FUNCTION__ << ": Handling of Critical Node: "
               << (CriticalCallsInserted ? "Successful" : "Failed") << ".\n");

  assert(CriticalCallsInserted && "Failed to create critical section. \n");

  // Remove calls to directive intrinsics since the LLVM back end does not
  // know how to translate them.
  if (CriticalCallsInserted)
    VPOUtils::stripDirectives(CriticalNode);

  return CriticalCallsInserted;
}

