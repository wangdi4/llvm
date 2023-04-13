//===- Predicator.cpp - Volcano predicator utility --------------*- C++ -*-===//
//
// Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/SYCLTransforms/Utils/Predicator.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"

namespace llvm {
namespace Predicator {

static constexpr int MaxNumBlocksInAnAllOnesByPass = 6;

// Volcano allOne.
bool isAllOne(StringRef Name) { return Name.startswith("__ocl_allOne"); }

// Volcano allZero.
bool isAllZero(StringRef Name) { return Name.startswith("__ocl_allZero"); }

// Volcano vectorizer call.
bool isMangledCall(StringRef Name) { return Name.contains("maskedf_"); }

// returns the terminator of BB if its a branch conditional on allones.
// otherwise returns NULL.
BranchInst *getAllOnesBranch(BasicBlock *BB) {
  Instruction *Term = BB->getTerminator();
  assert(Term && "terminator cannot be null");
  BranchInst *Br = dyn_cast<BranchInst>(Term);
  if (!Br)
    return nullptr;

  if (Br->isConditional()) {
    Value *Cond = Br->getCondition();
    CallInst *CondCall = dyn_cast<CallInst>(Cond);
    if (CondCall && CondCall->getCalledFunction())
      if (isAllOne(CondCall->getCalledFunction()->getName()))
        return Br;
  }
  return nullptr;
}

static bool isSingleBlockLoop(BasicBlock *BB) {
  for (auto *Pred : predecessors(BB))
    if (Pred == BB)
      return true;

  return false;
}

// recursively (by checking type of predecessors) gets the allones block type.
static AllOnesBlockType getAllOnesBlockTypeRec(BasicBlock *BB,
                                               int RecursionLevel) {
  // allones blocks doesn't contain loops other than single block loops.
  // if recursion level is high, then we are in a loop, so
  // this is not an allones block.
  if (RecursionLevel > MaxNumBlocksInAnAllOnesByPass + 1)
    return NONE;

  bool IsBlockALoop = isSingleBlockLoop(BB);
  BranchInst *AllOnesBranch = getAllOnesBranch(BB);
  // 1. first handle blocks that end with an allones branch.
  if (AllOnesBranch) {
    if (IsBlockALoop) {
      // note a block could also be SINGLE_BLOCK_LOOP_ALLONES
      // without terminating in an allones branch. (could be a uniform branch)
      return SINGLE_BLOCK_LOOP_ALLONES;
    }
    for (auto *Succ : AllOnesBranch->successors())
      if (isSingleBlockLoop(Succ))
        return SINGLE_BLOCK_LOOP_ENTRY;
    return ENTRY;
  }

  // 2. then handle blocks that are a self-loop.
  if (IsBlockALoop) {
    for (auto *Pred : predecessors(BB)) {
      if (Pred == BB)
        continue;

      AllOnesBlockType PredType =
          getAllOnesBlockTypeRec(Pred, RecursionLevel + 1);
      if (PredType == SINGLE_BLOCK_LOOP_ENTRY_TO_ORIGINAL)
        return SINGLE_BLOCK_LOOP_ORIGINAL;
      if (PredType == SINGLE_BLOCK_LOOP_ENTRY)
        return SINGLE_BLOCK_LOOP_ALLONES;
    }
    return NONE;
  }

  // 3. then handle all the rest.
  for (auto *Pred : predecessors(BB)) {
    assert(Pred != BB && "BB shouldn't be a self-loop!");
    AllOnesBlockType PredType =
        getAllOnesBlockTypeRec(Pred, RecursionLevel + 1);
    switch (PredType) {
    case NONE:
      return NONE;
    case ENTRY: {
      // either all ones or Original.
      BranchInst *EntryBranch = getAllOnesBranch(Pred);
      assert(EntryBranch && "expected a valid allones branch in Entry");
      assert(EntryBranch->getNumOperands() >= 3 &&
             "not enough operands for allones");
      if (EntryBranch->getOperand(2) == BB)
        return ALLONES;
      assert((EntryBranch->getOperand(1) == BB) && "should be this block");
      return ORIGINAL;
    }
    case ORIGINAL:
    case ALLONES:
      return EXIT;
    case EXIT:
      return NONE;
    case SINGLE_BLOCK_LOOP_ENTRY:
      // could not be SINGLE_BLOCK_LOOP_ALLONES, because that
      // is already handled when handling self-loop blocks.
      return SINGLE_BLOCK_LOOP_ENTRY_TO_ORIGINAL;
    case SINGLE_BLOCK_LOOP_ALLONES:
      return SINGLE_BLOCK_LOOP_TEST_ALLZEROES;
    case SINGLE_BLOCK_LOOP_ENTRY_TO_ORIGINAL:
      return SINGLE_BLOCK_LOOP_ORIGINAL;
    case SINGLE_BLOCK_LOOP_ORIGINAL:
      return SINGLE_BLOCK_LOOP_EXIT;
    case SINGLE_BLOCK_LOOP_EXIT:
      return NONE;
    default:
      break;
    }

    // if pred is SINGLE_BLOCK_LOOP_TEST_ALLZEROES,
    // then two options for type, we will find out using the other
    // predecessor.
  }
  return NONE;
}

AllOnesBlockType getAllOnesBlockType(BasicBlock *BB) {
  return getAllOnesBlockTypeRec(BB, 0);
}

// assumes it gets a SINGLE_BLOCK_LOOP_ORIGINAL block,
// and returns its SIGLE_BLOCK_LOOP_ALLONES twin.
BasicBlock *getAllOnesSingleLoopBlock(BasicBlock *OrigSingleLoop) {
  assert(getAllOnesBlockType(OrigSingleLoop) == SINGLE_BLOCK_LOOP_ORIGINAL &&
         "expected Original single block loop");

  pred_iterator PredIt = pred_begin(OrigSingleLoop);
  assert(PredIt != pred_end(OrigSingleLoop) &&
         "expected to find Entry to Original");
  BasicBlock *EntryToOriginal = *PredIt;
  assert(getAllOnesBlockType(EntryToOriginal) ==
             SINGLE_BLOCK_LOOP_ENTRY_TO_ORIGINAL &&
         "block type misfit");

  for (auto *Pred2 : predecessors(EntryToOriginal)) {
    if (getAllOnesBlockType(Pred2) != SINGLE_BLOCK_LOOP_ENTRY)
      continue;
    // go over successors of the Entry.
    for (auto *Succ : successors(Pred2))
      if (getAllOnesBlockType(Succ) == SINGLE_BLOCK_LOOP_ALLONES)
        return Succ;
  }

  llvm_unreachable("couldn't find single block loop allones");
  //  return OrigSingleLoop;
}

// assumes it gets an ORIGINAL,
// and returns its ENTRY predecessor.
BasicBlock *getEntryBlockFromOriginal(BasicBlock *Original) {
  assert(getAllOnesBlockType(Original) == ORIGINAL &&
         "expected Original block type");

  pred_iterator PredIt = pred_begin(Original);
  assert(PredIt != pred_end(Original) && "expected to find Entry");
  BasicBlock *Entry = *PredIt;
  assert(getAllOnesBlockType(Entry) == ENTRY && "block type misfit");

  return Entry;
}

// assumes it gets a SINGLE_BLOCK_LOOP_ORIGINAL block,
// and returns the corresponding SINGLE_BLOCK_LOOP_ENTRY.
BasicBlock *getEntryBlockFromLoopOriginal(BasicBlock *LoopOriginal) {
  assert(getAllOnesBlockType(LoopOriginal) == SINGLE_BLOCK_LOOP_ORIGINAL &&
         "expected Original single block loop");

  pred_iterator PredIt = pred_begin(LoopOriginal);
  assert(PredIt != pred_end(LoopOriginal) &&
         "expected to find Entry to Original");
  BasicBlock *EntryToOriginal = *PredIt;
  assert(getAllOnesBlockType(EntryToOriginal) ==
             SINGLE_BLOCK_LOOP_ENTRY_TO_ORIGINAL &&
         "block type misfit");

  for (auto *Pred2 : predecessors(EntryToOriginal))
    if (getAllOnesBlockType(Pred2) == SINGLE_BLOCK_LOOP_ENTRY)
      return Pred2;

  llvm_unreachable("failed to find Entry");
  //  return LoopOriginal;
}

} // namespace Predicator
} // namespace llvm
