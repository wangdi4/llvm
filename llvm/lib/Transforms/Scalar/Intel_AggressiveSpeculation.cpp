//===------ Intel_AggressiveSpeculation.cpp - Aggressive Speculation ------===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass attempts aggressive, larger-scale speculation based on the
// existence of strong hints, such as MD_unpredictable metadata.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Scalar/Intel_AggressiveSpeculation.h"

#include "llvm/Analysis/CFG.h"
#include "llvm/Analysis/Loads.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/Local.h"

using namespace llvm;

#define DEBUG_TYPE "aggressive-speculation"

static cl::opt<bool> DisableAggressiveSpeculation(
    "disable-aggressive-speculation", cl::Hidden,
    cl::desc("Do not perform aggressive speculation"));

static cl::opt<bool> UnprofitableAggressiveSpeculation(
    "unprofitable-aggressive-speculation", cl::Hidden,
    cl::desc("Do not restrict aggressive speculation to profitable cases"));

static cl::opt<bool>
    SpeculateRelatedLoads("aggressive-speculation-related-loads", cl::Hidden,
                          cl::init(true),
                          cl::desc("Consider one struct element's load to "
                                   "justify speculation of another"));

// If F1 is a function which itself does nothing but pass all of its arguments
// to another function F2 and then return the result, return F2. Otherwise,
// return nullptr.
static const Function *getTrivialCallSiteFunction(Function *F1) {
  if (F1->getInstructionCount() != 2)
    return nullptr;

  // This first instruction must be a call.
  const BasicBlock &BB = F1->getEntryBlock();
  const Instruction *I1 = BB.getFirstNonPHIOrDbg();
  if (!I1)
    return nullptr;

  const CallBase *C = dyn_cast<CallBase>(I1);
  if (!C)
    return nullptr;

  const Function *CalledF = C->getCalledFunction();
  // It should be calling something else directly -- not itself recursively.
  if (!CalledF || CalledF == F1)
    return nullptr;

  // The second instruction must be returning the result from the call.
  const Instruction *I2 = I1->getNextNonDebugInstruction();
  if (!I2)
    return nullptr;

  const ReturnInst *Ret = dyn_cast<ReturnInst>(I2);
  if (!Ret)
    return nullptr;

  // Verify we're returning the value from the call.
  if (Ret->getReturnValue() != C)
    return nullptr;

  // Finally, verify that we're passing along all arguments.
  if (F1->arg_size() != C->arg_size())
    return nullptr;

  for (unsigned I = 0, E = F1->arg_size(); I < E; ++I)
    if (C->getArgOperand(I) != F1->getArg(I))
      return nullptr;

  if (C->getCallingConv() != F1->getCallingConv())
    return nullptr;

  LLVM_DEBUG(dbgs() << "Note: reasoning as though a call to " << F1->getName()
                    << " is equivalent to a call to " << CalledF->getName()
                    << "\n");
  return CalledF;
}

// Check if two callees are actually or effectively calling the same function.
// If so, return the common underlying function. Otherwise, return nullptr.
//
// For example:
// int wrap(int i) {
//   return func(i);
// }
//
//    ...
//  int a = wrap(3); // Call site 1
//  int b = func(5); // Call site 2
//
//  Here, the two call sites are both effectively calling 'func'.
static Function *calleesAreEquivalent(CallBase *C1, CallBase *C2) {
  if (!C1 || !C2)
    return nullptr;

  auto *F1 = C1->getCalledFunction();
  if (!F1)
    return nullptr;

  auto *F2 = C2->getCalledFunction();
  if (!F2)
    return nullptr;

  if (F1->isVarArg() || F2->isVarArg())
    return nullptr;

  if (F1->getCallingConv() != F2->getCallingConv())
    return nullptr;

  if (F1 == F2)
    return F1;

  if (!C1->isNoInline() && getTrivialCallSiteFunction(F1) == F2)
    return F2;

  if (!C2->isNoInline() && getTrivialCallSiteFunction(F2) == F1)
    return F1;

  return nullptr;
}

// Determine if two memory accesses are related, where the relation means that
// they're accessing the same allocation. This is similar but not identical to
// 'loadsAreRelated' in LICM.
//
// 2 memory accesses are related, if their pointer operands are GEPs that point
// into the same structure:
//   - base pointer of GEPs are the same
//   - the GEP base type is a structure
//   - the GEP offsets are constant indices
//   - the TBAA information exists and has the same base for both loads.
static bool accessesAreRelated(Value *P1, Value *P2, AAMDNodes &AAMD1,
                               AAMDNodes &AAMD2, const DataLayout &DL) {
  // Compare the basic GEP fields.
  auto *GEP1 = dyn_cast<GetElementPtrInst>(P1);
  auto *GEP2 = dyn_cast<GetElementPtrInst>(P2);
  if (!GEP1 || !GEP2)
    return false;
  if (GEP1->getPointerOperand() != GEP2->getPointerOperand())
    return false;
  if (!GEP1->isInBounds() || !GEP2->isInBounds())
    return false;

  // Constant index GEPs of the same base structure type
  if (!GEP1->hasAllConstantIndices() || !GEP2->hasAllConstantIndices())
    return false;
  auto *StrTy1 = dyn_cast<StructType>(GEP1->getSourceElementType());
  auto *StrTy2 = dyn_cast<StructType>(GEP2->getSourceElementType());
  if (!StrTy1 || !StrTy2 || StrTy1 != StrTy2)
    return false;

  // Check that the AA metadata also shows the same base type. Filters out
  // illegal casts.
  if (!AAMD1.TBAA || !AAMD2.TBAA)
    return false;
  if (AAMD1.TBAA->getNumOperands() < 3 || AAMD2.TBAA->getNumOperands() < 3)
    return false;
  if (AAMD1.TBAA->getOperand(0) != AAMD2.TBAA->getOperand(0))
    return false;

  return true;
}

// This is an adaptation of the reasoning in `loadHasSafeRelatedLoad` from
// LICM, but its interface looks more like `isSafeToLoadUnconditionally`. It
// similarly only searches only a portion of a BasicBlock and can use a store
// or a load.
static bool loadHasSafeRelatedAccess(LoadInst *SpecLoad, Type *Ty,
                                     Align Alignment, Instruction *CtxI,
                                     const DataLayout &DL) {

  Value *V = SpecLoad->getPointerOperand();

  TypeSize TySize = DL.getTypeStoreSize(Ty);
  if (TySize.isScalable())
    return false;
  APInt Size(DL.getIndexTypeSizeInBits(V->getType()), TySize.getFixedValue());

  BasicBlock::iterator BBI = CtxI->getIterator(),
                       E = CtxI->getParent()->begin();

  if (Size.getBitWidth() > 64)
    return false;
  const uint64_t LoadSize = Size.getZExtValue();

  while (BBI != E) {
    --BBI;

    // If we see a free or a call which may write to memory (i.e. which might do
    // a free) the pointer could be marked invalid.
    if (isa<CallInst>(BBI) && BBI->mayWriteToMemory() &&
        !isa<LifetimeIntrinsic>(BBI) && !isa<DbgInfoIntrinsic>(BBI))
      return false;

    Value *AccessedPtr;
    Type *AccessedTy;
    Align AccessedAlign;
    AAMDNodes AccessedAAMD;
    if (LoadInst *LI = dyn_cast<LoadInst>(BBI)) {
      // Ignore volatile loads. The execution of a volatile load cannot
      // be used to prove an address is backed by regular memory; it can,
      // for example, point to an MMIO register.
      if (LI->isVolatile())
        continue;
      AccessedPtr = LI->getPointerOperand();
      AccessedTy = LI->getType();
      AccessedAlign = LI->getAlign();
      AccessedAAMD = LI->getAAMetadata();
    } else if (StoreInst *SI = dyn_cast<StoreInst>(BBI)) {
      // Ignore volatile stores (see comment for loads).
      if (SI->isVolatile())
        continue;
      AccessedPtr = SI->getPointerOperand();
      AccessedTy = SI->getValueOperand()->getType();
      AccessedAlign = SI->getAlign();
      AccessedAAMD = SI->getAAMetadata();
    } else
      continue;

    if (AccessedAlign < Alignment)
      continue;

    if (LoadSize > DL.getTypeStoreSize(AccessedTy))
      continue;

    // Handle trivial cases.
    if (AccessedPtr == V) {
      LLVM_DEBUG(dbgs() << "The load:\n"
                        << *SpecLoad << "\n"
                        << "\t\tis speculatable because of dominating access "
                           "to the same struct element:\n"
                        << *BBI << "\n");
      return true;
    }

    AAMDNodes VAAMD = SpecLoad->getAAMetadata();
    if (accessesAreRelated(AccessedPtr, V, AccessedAAMD, VAAMD, DL)) {
      LLVM_DEBUG(dbgs() << "The load:\n"
                        << *SpecLoad << "\n"
                        << "\t\tis speculatable because of dominating access "
                           "to another struct element:\n"
                        << *BBI << "\n");
      return true;
    }
  }

  return false;
}

// Attempt to speculatively execute conditional successors of BB.
//
// The following CFG shape is currently supported:
//       [BB]
//      /    \
//  [Succ0]  [Succ1]
//      \    /
//    [Converge]
//
//  ...where the result will essentially be:
//
//       [BB]
//        |
//     [Succ0
//      Succ1]
//        |
//    [Converge]
//
//  I.e., the code in successor blocks will be unconditionally executed.
//  Dataflow into Converge is preserved by inserting select instructions
//  using the original branch condition.
//
// Each instruction in each successor must be classified as
// 1) speculatable,
// 2) mergeable with an instruction of the same operation in the other
//    successor(s), or,
// 3) neither.
//
// Each speculatable instruction from each successor is moved into the new
// combined basic block. A set of mergeable instructions (one from each
// successor) across all successor results in only a single instruction in the
// new combined basic block, possibly with select instructions to choose
// operands.
//
// This implementation can only recognize mergeable function calls, and only up
// to one. All other instructions must be deemed to be speculatable. If any of
// these other instructions are not provably specualatable, we can't do
// anything.
//
// The example above and this implementation are concerned with two successors,
// but in theory trees of selects could be implemented to support more.
static bool speculateSuccessors(BasicBlock *BB, const DominatorTree *DT) {

  // Ensure all successors have no other predecessors.
  unsigned NumSuccessors = 0;
  for (BasicBlock *SuccBB : successors(BB)) {
    if (!SuccBB->hasNPredecessors(1))
      return false;
    NumSuccessors++;
  }

  // The current implementation expects specifically 2 successors.
  if (NumSuccessors != 2)
    return false;

  // We need a branch instruction with a condition.
  BranchInst *BI = dyn_cast<BranchInst>(BB->getTerminator());
  if (!BI)
    return false;

  // Ensure all successors share a single successor.
  BasicBlock *SuccBB0 = *(successors(BB).begin());
  BasicBlock *ConvergeBB = SuccBB0->getSingleSuccessor();
  if (!ConvergeBB)
    return false;
  for (BasicBlock *SuccBB : successors(BB))
    if (SuccBB->getSingleSuccessor() != ConvergeBB)
      return false;

  SmallVector<CallBase *> CallsToMerge;
  for (BasicBlock *SuccBB : successors(BB)) {
    for (Instruction &I : *SuccBB) {
      // Terminators are understood to be unconditional branches to ConvergeBB.
      if (I.isTerminator())
        break;

      // For an arbitrary instruction to be speculated it must be deemed safe
      // by ValueTracking's analysis.
      if (isSafeToSpeculativelyExecute(&I))
        continue;

      // Alternatively, if it's a Load, we'll try some additional analysis to
      // see if we can prove it's safe to hoist.
      if (auto *LI = dyn_cast<LoadInst>(&I)) {
        // Ensure that this load isn't required to specifically not be
        // speculated, e.g., by ASAN.
        if (mustSuppressSpeculation(*LI))
          return false;

        // Ask isSafeToLoadUnconditionally, which is a little more
        // expensive than isSafeToSpeculativelyExecute.
        const DataLayout &DL = LI->getModule()->getDataLayout();
        if (isSafeToLoadUnconditionally(LI->getPointerOperand(), LI->getType(),
                                        LI->getAlign(), DL, BB->getTerminator(),
                                        nullptr, DT))
          continue;

        // Finally, we can try our own analysis intended to compliment
        // isSafeToLoadUnconditionally. This mainly differs in that it will
        // consider a dominating access to a *different* struct element to
        // guarantee safety as long as TBAA with the IR types that the same
        // structure is being accessed. This is a bit of a grey area, so it can
        // be disabled with an option.
        if (SpeculateRelatedLoads &&
            loadHasSafeRelatedAccess(LI, LI->getType(), LI->getAlign(),
                                     BB->getTerminator(), DL))
          continue;

        return false;
      }

      // The only non-speculative possibility is merging calls. We collect them
      // here, but determine whether or not they can actually be merged later.
      if (auto *C = dyn_cast<CallBase>(&I)) {
        // This shouldn't include debug intrinsics, as they're safe to
        // speculate.
        CallsToMerge.push_back(C);
        continue;
      }

      // This instruction is not safe to speculate and is is not a call which
      // we might be able to merge. Nothing to do.
      return false;
    }
  }

  // If we encountered calls which we would need to merge, determine whether or
  // not that's possible here.
  Function *MergedCallee = nullptr;
  if (!CallsToMerge.empty()) {
    // Only support one call in each block, for now.
    if (CallsToMerge.size() % NumSuccessors != 0)
      return false;
    if (CallsToMerge.size() > NumSuccessors)
      return false;
    if (CallsToMerge[0]->getParent() == CallsToMerge[1]->getParent())
      return false;

    for (CallBase *C : CallsToMerge) {
      if (C->isInlineAsm() || C->cannotMerge() || C->isConvergent() ||
          C->isIndirectCall())
        return false;

      if (any_of(C->operands(),
                 [](const Value *Op) { return Op->getType()->isTokenTy(); }))
        return false;
    }
    MergedCallee = calleesAreEquivalent(CallsToMerge[0], CallsToMerge[1]);
    if (!MergedCallee)
      return false;
  }

  // We've now decided to hoist the two successors into the common predecessor.
  //
  // In general this is complicated because we need to decide on sets of
  // instructions (one from each successor) to merge with some degree of
  // argument selection. Any instructions which aren't merged will be
  // unconditionally executed in the new basic block.
  //
  // This implementation simplifies the problem by only supporting 0 or 1 merged
  // calls. The remaining code is speculated. However, subsequent scalar
  // optimization passes will typically improve obvious cases such that they're
  // ultimately merged.
  //
  // This will proceed in 3 steps:
  // 1. If there were calls, split each predecessor at the call in order to
  //    hoist all instructions which may be used by the calls first.
  // 2. Next, merge any calls into a single call, generating select
  //    instructions to route speculated arguments.
  // 3. Finally, hoist any code following the calls.

  for (auto *SuccBB : successors(BB)) {
    // Split the successor at the call, if any.
    if (!CallsToMerge.empty()) {
      unsigned SuccIdx = GetSuccessorNumber(BB, SuccBB);
      Instruction *CallI = CallsToMerge[SuccIdx];
      SuccBB->splitBasicBlock(CallI, "call.split");
    }
    // Hoist the pre-call instructions, or, the entire successor if there are no
    // non-mergeable calls.
    hoistAllInstructionsInto(BB, BB->getTerminator(), SuccBB);
  }

  // If there were no non-speculatable calls, we're done.
  if (CallsToMerge.empty())
    return true;

  // If there were non-speculatable calls, merge them such that we emit only
  // one call.
  SmallVector<Value *, 4> NewOperands;
  auto *Call0 = CallsToMerge[0];

  // Move the merged call instruction into place so that we can ensure it has a
  // debug location.
  Call0->moveBefore(BB->getTerminator());

  // In general we could select the actual callee, but this could result in
  // indirect calls. For now we avoid this by ensuring that the call sites are
  // effectively calling the same callee.
  assert(MergedCallee && "Expected a single common callee");
  for (auto *C : CallsToMerge)
    C->setCalledFunction(MergedCallee);

  for (unsigned O = 0, E = Call0->getNumOperands(); O != E; ++O) {
    bool NeedSelect = any_of(CallsToMerge, [Call0, O](const Instruction *I) {
      return I->getOperand(O) != Call0->getOperand(O);
    });
    if (!NeedSelect) {
      NewOperands.push_back(Call0->getOperand(O));
      continue;
    }

    // Otherwise, create a new select to choose between operands.
    auto *Op = Call0->getOperand(O);
    assert(!Op->getType()->isTokenTy() && "Can't PHI tokens!");
    // This assumes NumSuccessors == 2, specifically.
    assert(NumSuccessors == 2 && "Unexpected number of successors");
    // Given BI, IRBuilder will preserve its branch metadata (e.g., !prof and
    // !unpredictable). Other passes such as SimplifyCFG do this, too.
    IRBuilder<> Builder(Call0);
    Value *S = Builder.CreateSelect(BI->getCondition(), Call0->getOperand(O),
                                    CallsToMerge[1]->getOperand(O),
                                    Op->getName() + ".hoist", BI);
    NewOperands.push_back(S);
  }

  // Leave a single call site with select-ed operands.
  for (unsigned O = 0, E = Call0->getNumOperands(); O != E; ++O) {
    Call0->getOperandUse(O).set(NewOperands[O]);
  }
  for (auto *C : CallsToMerge) {
    if (C == Call0)
      continue;
    Call0->applyMergedLocation(Call0->getDebugLoc(), C->getDebugLoc());
    C->replaceAllUsesWith(Call0);
    C->eraseFromParent();
  }

  // Finally, since we split the original blocks at the calls, hoist the second
  // piece of each block.
  for (BasicBlock *SuccBB : successors(BB)) {
    hoistAllInstructionsInto(BB, BB->getTerminator(),
                             SuccBB->getUniqueSuccessor());
    MergeBasicBlockIntoOnlyPred(SuccBB->getUniqueSuccessor());
  }

  return true;
}

PreservedAnalyses AggressiveSpeculationPass::run(Function &F,
                                                 FunctionAnalysisManager &AM) {
  bool Changed = false;

  if (DisableAggressiveSpeculation)
    return PreservedAnalyses::all();

  const auto &DT = AM.getResult<DominatorTreeAnalysis>(F);
  const auto &TTI = AM.getResult<TargetIRAnalysis>(F);
  SmallVector<BasicBlock *, 1> BlocksToSimplify;

  for (auto &BB : F) {
    // Only concern ourselves with unpredictable branches.
    if (!UnprofitableAggressiveSpeculation &&
        !BB.getTerminator()->getMetadata(LLVMContext::MD_unpredictable))
      continue;

    const bool Speculated = speculateSuccessors(&BB, &DT);
    if (Speculated) {
      Changed = true;
      LLVM_DEBUG(dbgs() << "Speculated successors of BB " << BB.getName()
                        << "\n");
      BlocksToSimplify.push_back(&BB);
    }
  }

  if (Changed) {
    for (BasicBlock *BB : BlocksToSimplify)
      simplifyCFG(BB, TTI);
    removeUnreachableBlocks(F);
    return PreservedAnalyses::none();
  }

  return PreservedAnalyses::all();
}
