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

#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionNode.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionUtils.h"

#include "llvm/Transforms/Intel_VPO/Paropt/VPOParoptTransform.h"

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

  VPOWRegionVisitor WV(WRegionList);
  WRNVisitor<VPOWRegionVisitor> WRegionGather(WV);
  WRegionGather.forwardVisit(WI->getWRGraph());

  DEBUG(dbgs() << "\nEND: Gather WRegion Node List\n");
  return;
}

bool VPOParoptTransform::ParoptTransformer() {

  LLVMContext &C = F->getContext();
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
  // The bits that the flags field can hold are defined as KMP_IDENT_* before
  //
  Type *IdentFieldTy[] = {Type::getInt32Ty(C),    // reserved_1
                          Type::getInt32Ty(C),    // flags
                          Type::getInt32Ty(C),    // reserved_2
                          Type::getInt32Ty(C),    // reserved_3
                          Type::getInt8PtrTy(C)}; // *psource

  IdentTy = StructType::create(ArrayRef<Type *>(IdentFieldTy, 5), "ident_t",
                               false); // isPacked = false

  StringRef S = F->getName();

  if (!S.compare_lower(StringRef("@main"))) {
    CallInst *RI = VPOParoptUtils::genRTLKmpcBeginCall(F, AI, IdentTy);
    RI->insertBefore(AI);

    for (BasicBlock &I : *F) {
      if (isa<ReturnInst>(I.getTerminator())) {
        Instruction *Inst = I.getTerminator();

        CallInst *RI = VPOParoptUtils::genRTLKmpcEndCall(F, Inst, IdentTy);
        RI->insertBefore(Inst);
      }
    }
  }

  if (WI->WRGraphIsEmpty()) {
    DEBUG(dbgs() << "\n ... No WRegion Candidates for Parallelization ...\n");
    return Changed;
  }

  Type *Int32Ty = Type::getInt32Ty(C);

  TidPtr = new AllocaInst(Int32Ty, "tid", AI);
  TidPtr->setAlignment(4);

  BidPtr = new AllocaInst(Int32Ty, "bid", AI);
  BidPtr->setAlignment(4);

  CallInst *RI = VPOParoptUtils::genRTLKmpcGlobalThreadNumCall(F, AI, IdentTy);
  RI->insertBefore(AI);

  StoreInst *Tmp0 = new StoreInst(RI, TidPtr, false, AI);
  Tmp0->setAlignment(4);

  // Constant Definitions
  ConstantInt *ValueZero = ConstantInt::get(Type::getInt32Ty(C), 0);

  StoreInst *Tmp1 = new StoreInst(ValueZero, BidPtr, false, AI);
  Tmp1->setAlignment(4);

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
      genMultiThreadedCode(W);
      break;
    case WRegionNode::WRNParallelLoop:
    case WRegionNode::WRNParallelSections:
      break;

    // Task constructs need to perform outlining
    case WRegionNode::WRNTask:
    case WRegionNode::WRNTaskLoop:
      break;

    // Constructs do not need to perform outlining
    case WRegionNode::WRNVecLoop:
    case WRegionNode::WRNWksLoop:
    case WRegionNode::WRNWksSections:
    case WRegionNode::WRNSection:
    case WRegionNode::WRNSingle:
    case WRegionNode::WRNMaster:
    case WRegionNode::WRNAtomic:
    case WRegionNode::WRNBarrier:
    case WRegionNode::WRNCancel:
    case WRegionNode::WRNCritical:
    case WRegionNode::WRNFlush:
    case WRegionNode::WRNOrdered:
    case WRegionNode::WRNTaskgroup:
      break;
    default:
      break;
    }
  }

  for (auto &R : WRegionList)
    delete R;

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

    // Finalized multithreaded Function declaration and definition
    Function *MTFn = finalizeExtractedMTFunction(NewF);

    std::vector<Value *> MTFnArgs;

    // Pass tid and bid arguments.
    MTFnArgs.push_back(TidPtr);
    MTFnArgs.push_back(BidPtr);

    CallSite CS(NewCall);

    // Pass all the same arguments of the extracted function.
    for (auto I = CS.arg_begin(), E = CS.arg_end(); I != E; ++I) {
      MTFnArgs.push_back((*I));
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
    genForkCallInst(W, MTFnCI);

    // Remove the serial call to MTFn function from the program, reducing
    // the use-count of MTFn
    MTFnCI->eraseFromParent();

    Changed = true;
  }

  return Changed;
}

void VPOParoptTransform::genForkCallInst(WRegionNode *W, CallInst *CI) {
  Module *M = F->getParent();
  LLVMContext &C = F->getContext();

  // Get MicroTask Function for __kmpc_fork_call
  Function *MicroTaskFn = CI->getCalledFunction();
  FunctionType *MicroTaskFnTy = MicroTaskFn->getFunctionType();

  // Get MicroTask Function for __kmpc_fork_call
  //
  // TBD: Need to add global_tid and bound_tid to Micro Task Function
  // Build void (*kmpc_micro)(kmp_int32 global_tid, kmp_int32 bound_tid,...)
  // Type *MicroTaskParams[] = {Type::getInt32Ty(C)), Type::getInt32Ty(C))};
  //

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

  AttributeSet ForkCallFnAttr;
  SmallVector<AttributeSet, 4> Attrs;

  AttributeSet FnAttrSet;
  AttrBuilder B;
  FnAttrSet = AttributeSet::get(C, ~0U, B);

  Attrs.push_back(FnAttrSet);
  ForkCallFnAttr = AttributeSet::get(C, Attrs);

  ForkCallFn->setAttributes(ForkCallFnAttr);

  // get source location information from DebugLoc
  BasicBlock *EntryBB = W->getEntryBBlock();
  BasicBlock *ExitBB = W->getExitBBlock();

  AllocaInst *KmpcLoc = VPOParoptUtils::genKmpcLocfromDebugLoc(
      F, CI, IdentTy, KMP_IDENT_KMPC, EntryBB, ExitBB);

  ConstantInt *ValueTwo = ConstantInt::get(Type::getInt32Ty(C), 2);

  std::vector<Value *> Params;
  Params.push_back(KmpcLoc);
  Params.push_back(ValueTwo);
  Params.push_back(MicroTaskFn);

  CallSite CS(CI);

  for (auto I = CS.arg_begin(), E = CS.arg_end(); I != E; ++I) {
    Params.push_back((*I));
  }

  CallInst *ForkCallInst = CallInst::Create(ForkCallFn, Params, "", CI);

  // CI->replaceAllUsesWith(NewCI);

  ForkCallInst->setCallingConv(CallingConv::C);
  ForkCallInst->setTailCall(false);

  return;
}

Function *VPOParoptTransform::finalizeExtractedMTFunction(Function *Fn) {
  LLVMContext &C = Fn->getContext();

  // Computing a new prototype for the function, which is the same as
  // the old function with two new parameters for passing tid and bid
  // required by OpenMP runtime library.
  FunctionType *FnTy = Fn->getFunctionType();

  std::vector<Type *> ParamsTy;

  ParamsTy.push_back(PointerType::getUnqual(Type::getInt32Ty(C)));
  ParamsTy.push_back(PointerType::getUnqual(Type::getInt32Ty(C)));

  for (auto ArgTyI = FnTy->param_begin(), ArgTyE = FnTy->param_end();
       ArgTyI != ArgTyE; ++ArgTyI) {
    ParamsTy.push_back(*ArgTyI);
  }

  Type *RetTy = FnTy->getReturnType();
  FunctionType *NFnTy = FunctionType::get(RetTy, ParamsTy, false);

  // Create the new function body and insert it into the module...
  Function *NFn = Function::Create(NFnTy, Fn->getLinkage());

  NFn->copyAttributesFrom(Fn);

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

  for (Function::arg_iterator I = Fn->arg_begin(), E = Fn->arg_end(); I != E;
       ++I, ++NewArgI) {
    // Move the name and users over to the new version.
    I->replaceAllUsesWith(&*NewArgI);
    NewArgI->takeName(&*I);
  }

  DenseMap<const Function *, DISubprogram *> FunctionDIs;

  // Patch the pointer to LLVM function in debug info descriptor.
  auto DI = FunctionDIs.find(Fn);
  if (DI != FunctionDIs.end()) {
    DISubprogram *SP = DI->second;
    //    SP->replaceFunction(NFn);

    // Ensure the map is updated so it can be reused on non-varargs argument
    // eliminations of the same function.
    FunctionDIs.erase(DI);
    FunctionDIs[NFn] = SP;
  }
  return NFn;
}
