//===------ Intel_PartialInlineAnalysis.cpp ------------------------------===//
//
// Copyright (C) 2019-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// This file includes a set of analysis routines that are generally useful for
/// partial inlining, particularly heuristics.
///
// ===--------------------------------------------------------------------=== //

#include "llvm/Analysis/Intel_PartialInlineAnalysis.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"

using namespace llvm;

// Limit of basic blocks in a function that will be partially inlined by
// this pass
static cl::opt<unsigned> IntelPILimitBB("intel-pi-limit-bb", cl::init(10),
                                        cl::ReallyHidden);

// Limit of incoming values in a PHINode
static cl::opt<unsigned> IntelPILimitIncomingValues("intel-pi-limit-values",
                                                    cl::init(3),
                                                    cl::ReallyHidden);
namespace llvm {

// Check if at least one of the successors of the input branch
// instruction leads to an exit block. An exit block looks like this:
//
//  %7 = phi i1 [ true, %2 ], [ %5, %4 ]
//  ret i1 %7
//
// It has a return from a PHINode.
static bool goToExit(BranchInst *BrInst) {

  if (!BrInst)
    return false;

  // Check if this is a valid PHINode
  auto IsValidPHI = [](Instruction *Inst) {
    if (!Inst)
      return false;

    PHINode *PHI = dyn_cast<PHINode>(Inst);
    if (!PHI || PHI->getNumIncomingValues() > IntelPILimitIncomingValues)
      return false;

    return true;
  };

  // Check if at least one of the successors of the basic block is
  // an exit block
  for (BasicBlock *CurrBB : successors(BrInst->getParent())) {
    Instruction *FirstInst = &(CurrBB->front());
    if (!IsValidPHI(FirstInst))
      continue;
    if (FirstInst->getNumUses() != 1)
      continue;
    Instruction *UserInst = cast<Instruction>(FirstInst->user_back());
    if (isa<ReturnInst>(UserInst))
      return true;
  }

  return false;
}

// Traverse through the users of the input instruction and check
// if it leads to the exit basic block.
static bool isUsedForExit(Instruction *Inst,
                          SetVector<Instruction *> &VisitedInst) {

  if (!Inst)
    return false;

  for (User *User : Inst->users()) {
    Instruction *InstUsr = dyn_cast<Instruction>(User);
    if (!InstUsr || !VisitedInst.insert(InstUsr))
      continue;

    // If the instruction is used in a branch, then check if it
    // goes to the exit block
    if (BranchInst *BrInst = dyn_cast<BranchInst>(User)) {
      if (BrInst->isConditional() && goToExit(BrInst)) {
        return true;
      }
    }

    // Recurse to follow the path
    if (isUsedForExit(InstUsr, VisitedInst))
      return true;
  }

  return false;
}

// Given a User, find the PHINode that represents iterating through
// the argument until null. For example:
//
// define i1 @foo(%"struct.pov::Object_Struct"*) {
//  %3 = icmp eq %"struct.pov::Object_Struct"* %0, null
//  br i1 %3, label %12, label %4
//
// ; <label>:4:
//  %5 = phi %"struct.pov::Object_Struct"* [ %9, %4 ], [ %0, %2 ]
//  %6 = Some computation
//  ...
//  %9 = more computations
//  ...
//  %11 = some compare with null
//  br i1 %11, label %4, label %12
//
// ; <label>:12:
//  %13 = phi i1 [ true, %2 ], [ %6, %4 ]
//  ret i1 %14
// }
//
// The PHINode %5 returns an incoming Value either from the argument
// (%0 in %2) or from iterating (%9 in %4). This function will return %5
// for the previous case.
static Instruction *getPHIUser(User *Usr, LoopInfo &LI,
                               SetVector<Instruction *> &VisitedInst) {

  if (isa<PHINode>(Usr)) {
    Instruction *PHIUsr = cast<Instruction>(Usr);

    BasicBlock *BB = PHIUsr->getParent();
    Loop *LoopBB = LI.getLoopFor(BB);

    if (LoopBB)
      return PHIUsr;
  }

  Instruction *Inst = dyn_cast<Instruction>(Usr);

  if (!Inst || !VisitedInst.insert(Inst))
    return nullptr;

  for (User *InstUsr : Inst->users()) {

    if (Instruction *PHI = getPHIUser(InstUsr, LI, VisitedInst))
      return PHI;
  }

  return nullptr;
}

// Return true if the input Value (argument) that is used as an iterator and
// then goes into an exit block for the given function.
static bool identifyLoopRegion(Function &F, Value *ArgVal,
                               LoopInfoFuncType &GetLoopInfo) {

  if (!ArgVal)
    return false;

  LoopInfo &LI = (GetLoopInfo)(F);

  if (LI.empty())
    return false;

  SetVector<Instruction *> VisitedInst;
  SetVector<Instruction *> VisitedInst2;

  for (User *User : ArgVal->users()) {

    if (Instruction *Inst = getPHIUser(User, LI, VisitedInst)) {
      VisitedInst2.clear();

      if (isUsedForExit(Inst, VisitedInst2))
        return true;
    }
  }
  return false;
}

// Given an input function, return the argument that is used to
// branch into an exit block. For example:
//
// define i1 @foo(%"struct.pov::Object_Struct"*) {
//  %3 = icmp eq %"struct.pov::Object_Struct"* %0, null
//  br i1 %3, label %12, label %4
//
// ; <label>:4:
//  %5 = phi %"struct.pov::Object_Struct"* [ %9, %4 ], [ %0, %2 ]
//  %6 = Some computation
//  ...
//  br i1 %11, label %4, label %12
//
// ; <label>:12:
//  %13 = phi i1 [ true, %2 ], [ %6, %4 ]
//  ret i1 %14
// }
//
// For function @foo, the argument %0 is used in the branch
// after %3. That branch will go into the basic block %12,
// which is an exit block.
static Value *identifyEntryRegion(Function &F,
                                  bool PrepareForLTO) {

  BasicBlock *EntryBB = &(F.getEntryBlock());
  SetVector<Instruction *> VisitedInst;
  BasicBlock *SuccBB = EntryBB->getSingleSuccessor();
  for (auto &Arg : F.args()) {

    Value *ArgVal = dyn_cast<Value>(&Arg);
    if (!ArgVal)
      continue;

    Type *ArgTy = ArgVal->getType();
    if (!ArgTy->isPointerTy())
      continue;

    for (User *User : ArgVal->users()) {
      if (Instruction *Inst = dyn_cast<Instruction>(User)) {
        BasicBlock *BB = Inst->getParent();
        if (BB != EntryBB && (!PrepareForLTO || BB != SuccBB))
          continue;
        if (isUsedForExit(Inst, VisitedInst)) {
          return ArgVal;
        }
      }
    }
  }
  return nullptr;
}

// Return true if the the input function has at least one argument
// that will be used to iterate through a loop and exit. In simple
// words, this function looks something like this:
//
// define i1 @foo(%"struct.pov::Object_Struct"*) {
//  %3 = icmp eq %"struct.pov::Object_Struct"* %0, null
//  br i1 %3, label %15, label %4
//
// ; <label>:4:
//  %5 = phi %"struct.pov::Object_Struct"* [ %9, %4 ], [ %0, %2 ]
//  ...
//  br i1 %8, label %4, label %10
//
// ; <label>:10:
//  %11 = phi i1 [ true, %2 ], [ false, %4 ]
//  ret i1 %11
// }
//
// The argument %0 in @foo is used to check if it is going into the
// loop in block %4 or exit in %10.
static bool canSplitFunctionIntoRegions(Function &F,
                                        LoopInfoFuncType &GetLoopInfo,
                                        bool PrepareForLTO) {

  if (F.size() >= IntelPILimitBB)
    return false;

  if (F.doesNotReturn())
    return false;

  if (F.arg_empty())
    return false;

  // Identify the argument that is used to branch into the exit
  // block
  Value *ArgValue = identifyEntryRegion(F, PrepareForLTO);

  // Identify if the argument is used to go into an exit block
  if (!ArgValue)
    return false;

  // Identify if the argument is used to go into a loop and
  // the loop goes into the exit block
  if (!identifyLoopRegion(F, ArgValue, GetLoopInfo))
    return false;

  return true;
}

// Return false if there is an attribute in the function that prevents
// the partial inlining or there is an indirect call site, else
// return true.
static bool checkFunctionProperties(Function &F) {

  // Attributes checked:
  //
  //  always inline
  //  no inline
  //  inline
  //  always inline recursive                (Intel specific)
  //  inline recursive                       (Intel specific)
  //  preferred for partial inlining         (Intel specific)
  //  outline function from partial inlining (Intel specific)
  auto CheckAttributes = [](const AttributeList &AttrList) {
    if (AttrList.hasFnAttribute(Attribute::AlwaysInline) ||
      AttrList.hasFnAttribute(Attribute::NoInline) ||
      AttrList.hasFnAttribute(Attribute::InlineHint) ||
      AttrList.hasFnAttribute(Attribute::AlwaysInlineRecursive) ||
      AttrList.hasFnAttribute(Attribute::InlineHintRecursive) ||
      AttrList.hasFnAttribute("prefer-partial-inline-inlined-clone") ||
      AttrList.hasFnAttribute("prefer-partial-inline-outlined-func"))
      return true;
    return false;
  };

  // First check the function attributes
  const AttributeList &FuncAttrs = F.getAttributes();
  if (CheckAttributes(FuncAttrs))
    return false;

  for (User *User : F.users()) {

    if (CallInst *Call = dyn_cast<CallInst>(User)) {

      // Make sure is a direct call
      if (Call->isIndirectCall() || Call->getCalledFunction() != &F)
        return false;

      // Check the attributes in the callsite
      const AttributeList &CallAttrs = Call->getAttributes();
      if (CheckAttributes(CallAttrs))
        return false;

      continue;
    }
    // Else, ignore those useless Users that weren't removed from a
    // previous transformation
    else if (Value *UselessVal = dyn_cast<Value>(User))
      if (UselessVal->getNumUses() == 0)
        continue;
    return false;
  }

  return true;
}

extern bool isIntelPartialInlineCandidate(Function *F,
                                          LoopInfoFuncType &GetLoopInfo,
                                          bool PrepareForLTO) {
  if (F == nullptr)
    return false;

  Type *RetType = F->getReturnType();

  // We are looking for simple functions that return true or false
  if (!RetType->isIntegerTy(1))
    return false;

  if (F->user_empty())
    return false;

  // Check the function attributes and call sites
  if (!checkFunctionProperties(*F)) {
    return false;
  }

  // Check if the function can be split into entry, loop and exit regions
  return canSplitFunctionIntoRegions(*F, GetLoopInfo, PrepareForLTO);
}

} // namespace llvm
