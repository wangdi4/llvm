//===-- BasicBlock.cpp - Implement BasicBlock related methods -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the BasicBlock class for the IR library.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/BasicBlock.h"
#include "SymbolTableListTraitsImpl.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/IRBuilder.h"     // INTEL
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Type.h"
#include <algorithm>

using namespace llvm;

ValueSymbolTable *BasicBlock::getValueSymbolTable() {
  if (Function *F = getParent())
    return F->getValueSymbolTable();
  return nullptr;
}

LLVMContext &BasicBlock::getContext() const {
  return getType()->getContext();
}

// Explicit instantiation of SymbolTableListTraits since some of the methods
// are not in the public header file...
template class llvm::SymbolTableListTraits<Instruction>;

BasicBlock::BasicBlock(LLVMContext &C, const Twine &Name, Function *NewParent,
                       BasicBlock *InsertBefore)
  : Value(Type::getLabelTy(C), Value::BasicBlockVal), Parent(nullptr) {

  if (NewParent)
    insertInto(NewParent, InsertBefore);
  else
    assert(!InsertBefore &&
           "Cannot insert block before another block with no function!");

  setName(Name);
}

void BasicBlock::insertInto(Function *NewParent, BasicBlock *InsertBefore) {
  assert(NewParent && "Expected a parent");
  assert(!Parent && "Already has a parent");

  if (InsertBefore)
    NewParent->getBasicBlockList().insert(InsertBefore->getIterator(), this);
  else
    NewParent->getBasicBlockList().push_back(this);
}

BasicBlock::~BasicBlock() {
  // If the address of the block is taken and it is being deleted (e.g. because
  // it is dead), this means that there is either a dangling constant expr
  // hanging off the block, or an undefined use of the block (source code
  // expecting the address of a label to keep the block alive even though there
  // is no indirect branch).  Handle these cases by zapping the BlockAddress
  // nodes.  There are no other possible uses at this point.
  if (hasAddressTaken()) {
    assert(!use_empty() && "There should be at least one blockaddress!");
    Constant *Replacement =
      ConstantInt::get(llvm::Type::getInt32Ty(getContext()), 1);
    while (!use_empty()) {
      BlockAddress *BA = cast<BlockAddress>(user_back());
      BA->replaceAllUsesWith(ConstantExpr::getIntToPtr(Replacement,
                                                       BA->getType()));
      BA->destroyConstant();
    }
  }

  assert(getParent() == nullptr && "BasicBlock still linked into the program!");
  dropAllReferences();
  InstList.clear();
}

void BasicBlock::setParent(Function *parent) {
  // Set Parent=parent, updating instruction symtab entries as appropriate.
  InstList.setSymTabObject(&Parent, parent);
}

iterator_range<filter_iterator<BasicBlock::const_iterator,
                               std::function<bool(const Instruction &)>>>
BasicBlock::instructionsWithoutDebug() const {
  std::function<bool(const Instruction &)> Fn = [](const Instruction &I) {
    return !isa<DbgInfoIntrinsic>(I);
  };
  return make_filter_range(*this, Fn);
}

iterator_range<filter_iterator<BasicBlock::iterator,
                               std::function<bool(Instruction &)>>>
BasicBlock::instructionsWithoutDebug() {
  std::function<bool(Instruction &)> Fn = [](Instruction &I) {
    return !isa<DbgInfoIntrinsic>(I);
  };
  return make_filter_range(*this, Fn);
}

filter_iterator<BasicBlock::const_iterator,
                std::function<bool(const Instruction &)>>::difference_type
BasicBlock::sizeWithoutDebug() const {
  return std::distance(instructionsWithoutDebug().begin(),
                       instructionsWithoutDebug().end());
}

void BasicBlock::removeFromParent() {
  getParent()->getBasicBlockList().remove(getIterator());
}

iplist<BasicBlock>::iterator BasicBlock::eraseFromParent() {
  return getParent()->getBasicBlockList().erase(getIterator());
}

/// Unlink this basic block from its current function and
/// insert it into the function that MovePos lives in, right before MovePos.
void BasicBlock::moveBefore(BasicBlock *MovePos) {
  MovePos->getParent()->getBasicBlockList().splice(
      MovePos->getIterator(), getParent()->getBasicBlockList(), getIterator());
}

/// Unlink this basic block from its current function and
/// insert it into the function that MovePos lives in, right after MovePos.
void BasicBlock::moveAfter(BasicBlock *MovePos) {
  MovePos->getParent()->getBasicBlockList().splice(
      ++MovePos->getIterator(), getParent()->getBasicBlockList(),
      getIterator());
}

const Module *BasicBlock::getModule() const {
  return getParent()->getParent();
}

const Instruction *BasicBlock::getTerminator() const {
  if (InstList.empty() || !InstList.back().isTerminator())
    return nullptr;
  return &InstList.back();
}

const CallInst *BasicBlock::getTerminatingMustTailCall() const {
  if (InstList.empty())
    return nullptr;
  const ReturnInst *RI = dyn_cast<ReturnInst>(&InstList.back());
  if (!RI || RI == &InstList.front())
    return nullptr;

  const Instruction *Prev = RI->getPrevNode();
  if (!Prev)
    return nullptr;

  if (Value *RV = RI->getReturnValue()) {
    if (RV != Prev)
      return nullptr;

    // Look through the optional bitcast.
    if (auto *BI = dyn_cast<BitCastInst>(Prev)) {
      RV = BI->getOperand(0);
      Prev = BI->getPrevNode();
      if (!Prev || RV != Prev)
        return nullptr;
    }
  }

  if (auto *CI = dyn_cast<CallInst>(Prev)) {
    if (CI->isMustTailCall())
      return CI;
  }
  return nullptr;
}

const CallInst *BasicBlock::getTerminatingDeoptimizeCall() const {
  if (InstList.empty())
    return nullptr;
  auto *RI = dyn_cast<ReturnInst>(&InstList.back());
  if (!RI || RI == &InstList.front())
    return nullptr;

  if (auto *CI = dyn_cast_or_null<CallInst>(RI->getPrevNode()))
    if (Function *F = CI->getCalledFunction())
      if (F->getIntrinsicID() == Intrinsic::experimental_deoptimize)
        return CI;

  return nullptr;
}

const Instruction* BasicBlock::getFirstNonPHI() const {
  for (const Instruction &I : *this)
    if (!isa<PHINode>(I))
      return &I;
  return nullptr;
}

const Instruction* BasicBlock::getFirstNonPHIOrDbg() const {
  for (const Instruction &I : *this)
    if (!isa<PHINode>(I) && !isa<DbgInfoIntrinsic>(I))
      return &I;
  return nullptr;
}

const Instruction* BasicBlock::getFirstNonPHIOrDbgOrLifetime() const {
  for (const Instruction &I : *this) {
    if (isa<PHINode>(I) || isa<DbgInfoIntrinsic>(I))
      continue;

    if (I.isLifetimeStartOrEnd())
      continue;

    return &I;
  }
  return nullptr;
}

BasicBlock::const_iterator BasicBlock::getFirstInsertionPt() const {
  const Instruction *FirstNonPHI = getFirstNonPHI();
  if (!FirstNonPHI)
    return end();

  const_iterator InsertPt = FirstNonPHI->getIterator();
  if (InsertPt->isEHPad()) ++InsertPt;
  return InsertPt;
}

void BasicBlock::dropAllReferences() {
  for (Instruction &I : *this)
    I.dropAllReferences();
}

/// If this basic block has a single predecessor block,
/// return the block, otherwise return a null pointer.
const BasicBlock *BasicBlock::getSinglePredecessor() const {
  const_pred_iterator PI = pred_begin(this), E = pred_end(this);
  if (PI == E) return nullptr;         // No preds.
  const BasicBlock *ThePred = *PI;
  ++PI;
  return (PI == E) ? ThePred : nullptr /*multiple preds*/;
}

/// If this basic block has a unique predecessor block,
/// return the block, otherwise return a null pointer.
/// Note that unique predecessor doesn't mean single edge, there can be
/// multiple edges from the unique predecessor to this block (for example
/// a switch statement with multiple cases having the same destination).
const BasicBlock *BasicBlock::getUniquePredecessor() const {
  const_pred_iterator PI = pred_begin(this), E = pred_end(this);
  if (PI == E) return nullptr; // No preds.
  const BasicBlock *PredBB = *PI;
  ++PI;
  for (;PI != E; ++PI) {
    if (*PI != PredBB)
      return nullptr;
    // The same predecessor appears multiple times in the predecessor list.
    // This is OK.
  }
  return PredBB;
}

bool BasicBlock::hasNPredecessors(unsigned N) const {
  return hasNItems(pred_begin(this), pred_end(this), N);
}

bool BasicBlock::hasNPredecessorsOrMore(unsigned N) const {
  return hasNItemsOrMore(pred_begin(this), pred_end(this), N);
}

const BasicBlock *BasicBlock::getSingleSuccessor() const {
  succ_const_iterator SI = succ_begin(this), E = succ_end(this);
  if (SI == E) return nullptr; // no successors
  const BasicBlock *TheSucc = *SI;
  ++SI;
  return (SI == E) ? TheSucc : nullptr /* multiple successors */;
}

const BasicBlock *BasicBlock::getUniqueSuccessor() const {
  succ_const_iterator SI = succ_begin(this), E = succ_end(this);
  if (SI == E) return nullptr; // No successors
  const BasicBlock *SuccBB = *SI;
  ++SI;
  for (;SI != E; ++SI) {
    if (*SI != SuccBB)
      return nullptr;
    // The same successor appears multiple times in the successor list.
    // This is OK.
  }
  return SuccBB;
}

iterator_range<BasicBlock::phi_iterator> BasicBlock::phis() {
  PHINode *P = empty() ? nullptr : dyn_cast<PHINode>(&*begin());
  return make_range<phi_iterator>(P, nullptr);
}

/// This method is used to notify a BasicBlock that the
/// specified Predecessor of the block is no longer able to reach it.  This is
/// actually not used to update the Predecessor list, but is actually used to
/// update the PHI nodes that reside in the block.  Note that this should be
/// called while the predecessor still refers to this block.
///
void BasicBlock::removePredecessor(BasicBlock *Pred,
                                   bool KeepOneInputPHIs) {
  assert((hasNUsesOrMore(16)||// Reduce cost of this assertion for complex CFGs.
          find(pred_begin(this), pred_end(this), Pred) != pred_end(this)) &&
         "removePredecessor: BB is not a predecessor!");

#if INTEL_CUSTOMIZATION
  // TODO: The following issue happens in llorg too. It is documented in the
  // JIRA ticket CMPLRLLVM-7142. This fix is going to be proposed and added
  // into the open-source version. The Intel customization should be removed
  // during the merge between Xmain and llorg, as well the TODO comments.

  // Identify if there is any PHINode that carries a cycle and uses the
  // input BasicBlock as incoming value. A cycle is created when an
  // User of a PHINode is an incoming value of the same PHINode.
  // For example:
  //
  //   %pn = phi i8* [ %t1, %bb1 ]
  //
  //   %t1 = getelementptr inbounds i8, i8* %pn, i64 1
  //
  // Simplifying the previous instructions can lead to a broken SSA
  // form:
  //
  //   %t1 = getelementptr inbounds i8, i8* %t1, i64 1
  //
  // The following process will check if this type of cycles can happen
  // when removing a predecessor and turns off the simplification.
  // For example:
  //
  // bb1:
  //   ...
  //   br %cmp, label %while.body, %exit.bb
  //
  // while.body:
  //   %ptr.1 = phi i8* [ %call.i, %bb1],
  //                    [ %incdec.ptr1, %exit.bb ]
  //   ...
  //   br %cmp, label %bb1, label %anotherBB
  //
  // exit.bb:
  //   %incdec.ptr1 = getelementptr inbounds i8, i8* %ptr.1, i64 1
  //   ...
  //   br %cmp, label %while.body, label %someBB
  //
  // If removePredecessor is detaching %bb1 from %while.body then the
  // only incoming value available to %ptr.1 is [ %incdec.ptr1, %exit.bb ].
  // Simplifying the %ptr.1 can break the SSA form in the instruction
  // %incdec.ptr1 = getelementptr inbounds i8, i8* %ptr1, i64 1.
  auto IsCyclePhiNode = [](PHINode *PN, BasicBlock *BB) {
    // Must have 2 incoming values only
    if (PN->getNumIncomingValues() != 2)
      return false;;

    int BBidx = PN->getBasicBlockIndex(BB);

    if (BBidx < 0)
      return false;;

    // This is the Value that won't be removed
    unsigned ValIdx = BBidx == 0 ? 1 : 0;
    Value *Val = PN->getIncomingValue(ValIdx);

    // Check for cycle
    for (User *User : PN->users()) {
      if (Val == dyn_cast<Value>(User)) {
        return true;
      }
    }
    return false;
  };

  // Replace the input PHINode with a combination of load and store
  // instructions that use the input Value.
  auto SimplifyPHINode = [](PHINode *PN, Value *Val, IRBuilder<> &Builder) {
    BasicBlock *CurrBB = PN->getParent();
    Builder.SetInsertPoint(CurrBB->getFirstNonPHI());
    Value *Alloc = Builder.CreateAlloca(Val->getType());
    Builder.CreateStore(Val, Alloc);
    Value *Load = Builder.CreateLoad(Alloc);
    PN->replaceAllUsesWith(Load);
  };
#endif // INTEL_CUSTOMIZATION

  if (InstList.empty()) return;
  PHINode *APN = dyn_cast<PHINode>(&front());
  if (!APN) return;   // Quick exit.

  // If there are exactly two predecessors, then we want to nuke the PHI nodes
  // altogether.  However, we cannot do this, if this in this case:
  //
  //  Loop:
  //    %x = phi [X, Loop]
  //    %x2 = add %x, 1         ;; This would become %x2 = add %x2, 1
  //    br Loop                 ;; %x2 does not dominate all uses
  //
  // This is because the PHI node input is actually taken from the predecessor
  // basic block.  The only case this can happen is with a self loop, so we
  // check for this case explicitly now.
  //
  unsigned max_idx = APN->getNumIncomingValues();
  assert(max_idx != 0 && "PHI Node in block with 0 predecessors!?!?!");
  if (max_idx == 2) {
    BasicBlock *Other = APN->getIncomingBlock(APN->getIncomingBlock(0) == Pred);

    // Disable PHI elimination!
    if (this == Other) max_idx = 3;
  }

#if INTEL_CUSTOMIZATION
  // TODO: Remove the Intel customization after the fix for CMPLRLLVM-7142
  // is added in llorg.
  IRBuilder<> Builder(Pred->getModule()->getContext());
#endif
  // <= Two predecessors BEFORE I remove one?
  if (max_idx <= 2 && !KeepOneInputPHIs) {
    // Yup, loop through and nuke the PHI nodes
    while (PHINode *PN = dyn_cast<PHINode>(&front())) {
#if INTEL_CUSTOMIZATION
      // TODO: Remove the Intel customization after the fix for CMPLRLLVM-7142
      // is added in llorg.
      bool HasCycle = IsCyclePhiNode(PN, Pred);
#endif
      // Remove the predecessor first.
      PN->removeIncomingValue(Pred, !KeepOneInputPHIs);

      // If the PHI _HAD_ two uses, replace PHI node with its now *single* value
      if (max_idx == 2) {
#if INTEL_CUSTOMIZATION
        // TODO: Remove the Intel customization after the fix for CMPLRLLVM-7142
        // is added in llorg.
        if (PN->getIncomingValue(0) != PN) {
          if (HasCycle)
            // Simplify the PHINode with a combination of store and load
            // if there is a cycle.
            SimplifyPHINode(PN, PN->getIncomingValue(0), Builder);
          else
            PN->replaceAllUsesWith(PN->getIncomingValue(0));
        }
#endif // INTEL_CUSTOMIZATION
        else
          // We are left with an infinite loop with no entries: kill the PHI.
          PN->replaceAllUsesWith(UndefValue::get(PN->getType()));
        getInstList().pop_front();    // Remove the PHI node
      }

      // If the PHI node already only had one entry, it got deleted by
      // removeIncomingValue.
    }
  } else {
    // Okay, now we know that we need to remove predecessor #pred_idx from all
    // PHI nodes.  Iterate over each PHI node fixing them up
    PHINode *PN;
    for (iterator II = begin(); (PN = dyn_cast<PHINode>(II)); ) {
      ++II;
#if INTEL_CUSTOMIZATION
      // TODO: Remove the Intel customization after the fix for CMPLRLLVM-7142
      // is added in llorg.
      bool HasCycle = IsCyclePhiNode(PN, Pred);
#endif
      PN->removeIncomingValue(Pred, false);
      // If all incoming values to the Phi are the same, we can replace the Phi
      // with that value.
      Value* PNV = nullptr;
      if (!KeepOneInputPHIs && (PNV = PN->hasConstantValue()))
        if (PNV != PN) {
#if INTEL_CUSTOMIZATION
        // TODO: Remove the Intel customization after the fix for CMPLRLLVM-7142
        // is added in llorg.
          if (HasCycle)
            // Simplify the PHINode with a combination of store and load
            // if there is a cycle.
            SimplifyPHINode(PN, PNV, Builder);
          else
            PN->replaceAllUsesWith(PNV);
#endif // INTEL_CUSTOMIZATION
          PN->eraseFromParent();
        }
    }
  }
}

bool BasicBlock::canSplitPredecessors() const {
  const Instruction *FirstNonPHI = getFirstNonPHI();
  if (isa<LandingPadInst>(FirstNonPHI))
    return true;
  // This is perhaps a little conservative because constructs like
  // CleanupBlockInst are pretty easy to split.  However, SplitBlockPredecessors
  // cannot handle such things just yet.
  if (FirstNonPHI->isEHPad())
    return false;
  return true;
}

bool BasicBlock::isLegalToHoistInto() const {
  auto *Term = getTerminator();
  // No terminator means the block is under construction.
  if (!Term)
    return true;

  // If the block has no successors, there can be no instructions to hoist.
  assert(Term->getNumSuccessors() > 0);

  // Instructions should not be hoisted across exception handling boundaries.
  return !Term->isExceptionalTerminator();
}

/// This splits a basic block into two at the specified
/// instruction.  Note that all instructions BEFORE the specified iterator stay
/// as part of the original basic block, an unconditional branch is added to
/// the new BB, and the rest of the instructions in the BB are moved to the new
/// BB, including the old terminator.  This invalidates the iterator.
///
/// Note that this only works on well formed basic blocks (must have a
/// terminator), and 'I' must not be the end of instruction list (which would
/// cause a degenerate basic block to be formed, having a terminator inside of
/// the basic block).
///
BasicBlock *BasicBlock::splitBasicBlock(iterator I, const Twine &BBName) {
  assert(getTerminator() && "Can't use splitBasicBlock on degenerate BB!");
  assert(I != InstList.end() &&
         "Trying to get me to create degenerate basic block!");

  BasicBlock *New = BasicBlock::Create(getContext(), BBName, getParent(),
                                       this->getNextNode());

  // Save DebugLoc of split point before invalidating iterator.
  DebugLoc Loc = I->getDebugLoc();
  // Move all of the specified instructions from the original basic block into
  // the new basic block.
  New->getInstList().splice(New->end(), this->getInstList(), I, end());

  // Add a branch instruction to the newly formed basic block.
  BranchInst *BI = BranchInst::Create(New, this);
  BI->setDebugLoc(Loc);

  // Now we must loop through all of the successors of the New block (which
  // _were_ the successors of the 'this' block), and update any PHI nodes in
  // successors.  If there were PHI nodes in the successors, then they need to
  // know that incoming branches will be from New, not from Old (this).
  //
  New->replaceSuccessorsPhiUsesWith(this, New);
  return New;
}

void BasicBlock::replacePhiUsesWith(BasicBlock *Old, BasicBlock *New) {
  // N.B. This might not be a complete BasicBlock, so don't assume
  // that it ends with a non-phi instruction.
  for (iterator II = begin(), IE = end(); II != IE; ++II) {
    PHINode *PN = dyn_cast<PHINode>(II);
    if (!PN)
      break;
    PN->replaceIncomingBlockWith(Old, New);
  }
}

void BasicBlock::replaceSuccessorsPhiUsesWith(BasicBlock *Old,
                                              BasicBlock *New) {
  Instruction *TI = getTerminator();
  if (!TI)
    // Cope with being called on a BasicBlock that doesn't have a terminator
    // yet. Clang's CodeGenFunction::EmitReturnBlock() likes to do this.
    return;
  llvm::for_each(successors(TI), [Old, New](BasicBlock *Succ) {
    Succ->replacePhiUsesWith(Old, New);
  });
}

void BasicBlock::replaceSuccessorsPhiUsesWith(BasicBlock *New) {
  this->replaceSuccessorsPhiUsesWith(this, New);
}

/// Return true if this basic block is a landing pad. I.e., it's
/// the destination of the 'unwind' edge of an invoke instruction.
bool BasicBlock::isLandingPad() const {
  return isa<LandingPadInst>(getFirstNonPHI());
}

/// Return the landingpad instruction associated with the landing pad.
const LandingPadInst *BasicBlock::getLandingPadInst() const {
  return dyn_cast<LandingPadInst>(getFirstNonPHI());
}

Optional<uint64_t> BasicBlock::getIrrLoopHeaderWeight() const {
  const Instruction *TI = getTerminator();
  if (MDNode *MDIrrLoopHeader =
      TI->getMetadata(LLVMContext::MD_irr_loop)) {
    MDString *MDName = cast<MDString>(MDIrrLoopHeader->getOperand(0));
    if (MDName->getString().equals("loop_header_weight")) {
      auto *CI = mdconst::extract<ConstantInt>(MDIrrLoopHeader->getOperand(1));
      return Optional<uint64_t>(CI->getValue().getZExtValue());
    }
  }
  return Optional<uint64_t>();
}

BasicBlock::iterator llvm::skipDebugIntrinsics(BasicBlock::iterator It) {
  while (isa<DbgInfoIntrinsic>(It))
    ++It;
  return It;
}
