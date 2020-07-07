//===--- IRRegion.cpp - Implements the IRRegion class ---------------------===//
//
// Copyright (C) 2015-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the IRRegion class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLRegion.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/IRRegion.h"
#include "llvm/IR/CFG.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Utils/BlobUtils.h"

using namespace llvm;
using namespace llvm::loopopt;

IRRegion::IRRegion(BasicBlock *EntryBB, BasicBlock *ExitBB,
                   const RegionBBlocksTy &BBs,
                   const RegionBBlocksTy &NonLoopBBs,
                   ArrayRef<const Loop *> OutermostLoops,
                   bool IsMaterializationCandidate, bool IsFunctionLevel)
    : EntryBBlock(EntryBB), ExitBBlock(ExitBB), BBlocks(BBs),
      NonLoopBBlocks(NonLoopBBs),
      OutermostLps(OutermostLoops.begin(), OutermostLoops.end()),
      ParentRegion(nullptr),
      IsLoopMaterializationCandidate(IsMaterializationCandidate),
      IsFunctionLevel(IsFunctionLevel) {
  assert(EntryBB && "Entry basic block cannot be null!");
  BBlocksSet.insert(BBs.begin(), BBs.end());
}

IRRegion::IRRegion(IRRegion &&Reg)
    : EntryBBlock(Reg.EntryBBlock), ExitBBlock(Reg.ExitBBlock),
      BBlocks(std::move(Reg.BBlocks)), BBlocksSet(std::move(Reg.BBlocksSet)),
      NonLoopBBlocks(std::move(Reg.NonLoopBBlocks)),
      OutermostLps(std::move(Reg.OutermostLps)),
      LiveInMap(std::move(Reg.LiveInMap)),
      LiveOutMap(std::move(Reg.LiveOutMap)), ParentRegion(Reg.ParentRegion),
      IsLoopMaterializationCandidate(Reg.IsLoopMaterializationCandidate),
      IsFunctionLevel(Reg.IsFunctionLevel) {}

IRRegion &IRRegion::operator=(IRRegion &&Reg) {
  EntryBBlock = Reg.EntryBBlock;
  ExitBBlock = Reg.ExitBBlock;
  BBlocks = std::move(Reg.BBlocks);
  BBlocksSet = std::move(Reg.BBlocksSet);
  NonLoopBBlocks = std::move(Reg.NonLoopBBlocks);
  OutermostLps = std::move(Reg.OutermostLps);
  LiveInMap = std::move(Reg.LiveInMap);
  LiveOutMap = std::move(Reg.LiveOutMap);
  ParentRegion = Reg.ParentRegion;
  IsFunctionLevel = Reg.IsFunctionLevel;
  IsLoopMaterializationCandidate = Reg.IsLoopMaterializationCandidate;
  return *this;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void IRRegion::dump() const { print(dbgs(), 0); }
#endif

void IRRegion::print(raw_ostream &OS, unsigned IndentWidth) const {
  OS.indent(IndentWidth) << "EntryBB: ";
  EntryBBlock->printAsOperand(OS, false);
  OS << "\n";

  OS.indent(IndentWidth) << "ExitBB: ";

  if (ExitBBlock) {
    ExitBBlock->printAsOperand(OS, false);
  } else {
    OS << ExitBBlock;
  }

  OS << "\n";
  OS.indent(IndentWidth) << "Member BBlocks: ";

  for (auto I = BBlocks.begin(), E = BBlocks.end(); I != E; ++I) {
    if (I != BBlocks.begin()) {
      OS << ", ";
    }
    (*I)->printAsOperand(OS, false);
  }

  OS << "\n";
  OS.indent(IndentWidth) << "LiveIns: ";

  for (auto I = LiveInMap.begin(), E = LiveInMap.end(); I != E; ++I) {
    if (I != LiveInMap.begin()) {
      OS << ", ";
    }

    if (ParentRegion) {
      ParentRegion->getBlobUtils().printScalar(OS, I->first);
    } else {
      OS << "I->first";
    }

    OS << "(";
    I->second->printAsOperand(OS, false);
    OS << ")";
  }

  OS << "\n";
  OS.indent(IndentWidth) << "LiveOuts: ";

  for (auto I = LiveOutMap.begin(), E = LiveOutMap.end(); I != E; ++I) {
    if (I != LiveOutMap.begin()) {
      OS << ", ";
    }

    if (I->second.size() > 1) {
      OS << "{";
    }

    int Count = 0;
    for (auto *Inst : I->second) {
      if (Count != 0) {
        OS << ", ";
      }
      Inst->printAsOperand(OS, false);
      ++Count;
    }

    if (I->second.size() > 1) {
      OS << "}";
    }

    OS << "(sym:" << I->first << ")";
  }

  OS << "\n";
}

BasicBlock *IRRegion::getPredBBlock() const {
  auto PredI = pred_begin(EntryBBlock);

  // Predecessor can be null for function level region.
  if (PredI == pred_end(EntryBBlock)) {
    return nullptr;
  }

  /// In some cases the entry bblock is also the loop header, so the predecessor
  /// can be the loop latch. We need to skip it, if that is the case.
  if (containsBBlock(*PredI)) {
    PredI++;
    auto TempPredI = PredI;

    (void)TempPredI;
    assert(!containsBBlock(*PredI) &&
           "Both region predecessors lie inside the reigon!");
    assert((++TempPredI == pred_end(EntryBBlock)) &&
           "Region has more than two predecessors!");
  }

  return *PredI;
}

BasicBlock *IRRegion::getSuccBBlock() const {
  if (!ExitBBlock) {
    return nullptr;
  }

  auto SuccI = succ_begin(ExitBBlock);

  // Exit bblock can be a function return bblock.
  if (SuccI == succ_end(ExitBBlock)) {
    return nullptr;
  }

  /// In some cases the exit bblock is also the loop latch, so the successor
  /// can be the loop header. We need to skip it, if that is the case.
  if (containsBBlock(*SuccI)) {
    SuccI++;
    auto TempSuccI = SuccI;

    (void)TempSuccI;
    assert(!containsBBlock(*SuccI) &&
           "Both region successors lie inside the reigon!");
    assert((++TempSuccI == succ_end(ExitBBlock)) &&
           "Region has more than two successors!");
  }

  return *SuccI;
}

void IRRegion::addLiveOutTemp(unsigned Symbase, const Instruction *Temp) {
  auto Ret = ReverseLiveOutMap.insert(std::make_pair(Temp, Symbase));
  assert((Ret.second || (Ret.first->second == Symbase)) &&
         "Inconsistent liveout value detected!");

  if (Ret.second) {
    LiveOutMap[Symbase].push_back(Temp);
  }
}

void IRRegion::removeLiveOutTemp(unsigned Symbase) {
  auto It = LiveOutMap.find(Symbase);
  assert((It != LiveOutMap.end()) && "Symbase is not liveout!");

  // Copy vector of temps as we are going to erase the iterator.
  auto TempVec = It->second;

  LiveOutMap.erase(It);

  for (auto *Temp : TempVec) {
    ReverseLiveOutMap.erase(Temp);
  }
}

void IRRegion::replaceLiveOutTemp(unsigned OldSymbase, unsigned NewSymbase) {
  auto It = LiveOutMap.find(OldSymbase);
  assert((It != LiveOutMap.end()) && "Old liveout temp not found!");

  // Copy vector of temps as we are going to erase the iterator.
  auto TempVec = It->second;

  LiveOutMap.erase(It);

  for (auto *Temp : TempVec) {
    ReverseLiveOutMap.erase(Temp);
    addLiveOutTemp(NewSymbase, Temp);
  }
}

void IRRegion::replaceEntryBBlock(BasicBlock *NewEntryBB) {
  BBlocksSet.erase(EntryBBlock);
  auto EntryIt = std::find(BBlocks.begin(), BBlocks.end(), EntryBBlock);
  assert(EntryIt != BBlocks.end() && "bblocks are out of sync!");

  *EntryIt = NewEntryBB;
  BBlocksSet.insert(NewEntryBB);

  auto NonLoopEntryIt =
      std::find(NonLoopBBlocks.begin(), NonLoopBBlocks.end(), EntryBBlock);

  if (NonLoopEntryIt != NonLoopBBlocks.end()) {
    *NonLoopEntryIt = NewEntryBB;
  }

  if (ExitBBlock == EntryBBlock) {
    ExitBBlock = NewEntryBB;
  }

  EntryBBlock = NewEntryBB;
}
