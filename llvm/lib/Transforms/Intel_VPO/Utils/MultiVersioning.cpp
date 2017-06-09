//===---- MultiVersioning.cpp - Create Multiple Versions for a WRegion ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// \file
// This file implements the multi-versioning member function of VPOUtils class,
// which creates two versions of code for a WRegion. It transforms a WRegion,
// denoted by BBSet, to the following:
//
// if (Condition)
//   BBSet
// else
//   Cloned BBSet
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/Function.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Intel_VPO/Utils/VPOUtils.h"
#include "llvm/Transforms/Utils/Intel_GeneralUtils.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/ADT/Twine.h"


using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vpo-multiversioning"

/// \brief The implementation is similar to cloneLoopWithPreheader() in
/// lib/Transforms/Utils/CloneFunction.cpp
/// and versionLoop() in lib/Transforms/Utils/LoopVersioning.cpp
/// 
void VPOUtils::cloneBBSet(
  SmallVectorImpl<BasicBlock *> &BBSet,
  SmallVectorImpl<BasicBlock *> &ClonedBBSet,
  ValueToValueMapTy &VMap,
  const Twine &NameSuffix,
  Function* F,
  ClonedCodeInfo *CodeInfo
)
{
  for (BasicBlock *BB : BBSet) {

    // Create a new basic block and copy instructions into it. VMap will keep
    // instruction mapping too (by CloneBasicBlock).
    BasicBlock *NewBB = CloneBasicBlock(BB, VMap, NameSuffix, F, CodeInfo);

    // Add basic block to the mapping.
    VMap[BB] = NewBB;

    ClonedBBSet.push_back(NewBB);
  }

  // Remap the values in the cloned blocks, e.g., branching to the right place.
  remapInstructionsInBlocks(ClonedBBSet, VMap);
}

/// \brief The implementation is similar to  
/// findDefsUsedOutsideOfLoop in lib/Transforms/Utils/LoopUtils.cpp
///
void VPOUtils::findDefsUsedOutsideOfRegion(
  SmallVectorImpl<BasicBlock *> &BBSet,
  SmallVectorImpl<Instruction *> &LiveOut
)
{
  for (auto *Block : BBSet)
    for (auto &Inst : *Block) {
      auto Users = Inst.users();
      if (std::any_of(Users.begin(), Users.end(), [&](User *U) {
          auto *Use = cast<Instruction>(U);
          return std::find(BBSet.begin(), BBSet.end(), Use->getParent()) 
                   == BBSet.end();
          }))
        // If any of the uses is outside the region (not in BBSet), push
        // it into LiveOut.
        LiveOut.push_back(&Inst);
    }
}

/// \brief See addPHINodes() in lib/Transforms/Utils/LoopVersioning.cpp
/// 
/// For example:
/// 
/// ExitBB     ClonedExitBB
///   \              /
///    \            /
///     \          /
///      PHIBlock:
///   t1 = PHI(t1_0, t1_1)
///   t2 = PHI(t2_0, t2_1)
///
/// t1_0 and t2_0 are from BBSet which are live out.
/// t1_1 and t2_1 are corresponding values from ClonedBBSet.
/// We need to add PHI instructions for (t1_0, t1_1) and (t2_0, t2_1) in the
/// PHIBlock, and change the following references to t1_0 and t2_0 with t1
/// and t2.
/// 
void VPOUtils::addPHINodes(
  ValueToValueMapTy &VMap,
  SmallVectorImpl<BasicBlock *> &BBSet,
  SmallVectorImpl<Instruction *> &LiveOut)
{
  // ExitBB is ensured to be the last item in BBSet.
  BasicBlock *ExitBB = BBSet.back();
  BasicBlock *ClonedExitBB = cast<BasicBlock>(VMap[ExitBB]);
  // PHIBlock is the 'NewTail'.
  BasicBlock *PHIBlock = ExitBB->getSingleSuccessor();

  for (auto *Inst: LiveOut) {
    auto *ClonedInst = cast<Instruction>(VMap[Inst]);
    PHINode *PH = 
        PHINode::Create(Inst->getType(), 2, Inst->getName() + ".multi.phi",
                         &PHIBlock->front());

    for (auto *User : Inst->users())
      if (std::find(BBSet.begin(), BBSet.end(),
          cast<Instruction>(User)->getParent()) == BBSet.end())
        User->replaceUsesOfWith(Inst, PH);

    PH->addIncoming(Inst, ExitBB);
    PH->addIncoming(ClonedInst, ClonedExitBB);
  }
}

/// \brief Given a single-entry and single-exit region represented by BBSet,
/// generates the following code:
/// 
/// if (Cond)
///   BBSet;
/// else
///   ClonedBBSet;
///
/// BBSet must be single-entry and single-exit. Let OldHead and OldTail denote
/// the predecessor of EntryBB and the successor of ExitBB:
///
///    OldHead
///      |  \
///      |
///  -----------
///  | EntryBB |
///  | (region)|
///  | ExitBB  |
///  -----------
///      |
///      | /
///    OldTail
/// 
/// The function will first insert two empty basic blocks called NewHead and
/// NewTail as below:
///
///    OldHead
///      |  \
///      |
///    NewHead
///      |
///      |
///  -----------
///  | EntryBB |
///  | (region)|
///  | ExitBB  |
///  -----------
///      |
///      |
///    NewTail
///      |
///      |  /
///    OldTail
/// 
/// and then transforms the CFG to the following:
///
///           OldHead
///              |  \
///              |
///           NewHead 
///            /    \
///           /      \
///  -----------   -----------------
///  | EntryBB |   | ClonedEntryBB |
///  | (region)|   |(cloned region)|
///  | ExitBB  |   | ClonedExitBB  |
///  -----------   -----------------
///           \     /
///            \   /
///           NewTail
///              |
///              |  /
///           OldTail
///
/// The "if(Cond)" is a conditional branch instruction added to NewHead, and
/// PHI instructions are added to NewTail for all live-out values in BBSet.
///
/// The function optionally maintains Dominator Tree information, however
/// it does not maintain Loop information. The client needs to recompute loop
/// information if needed. 
///
/// Also, the WRegion information may need to be updated depending on the
/// client. For example, if the directives in the cloned WRegion is deleted,
/// which becomes serial code, then there is no need to update the WRegion
/// information, otherwise, a new WRegion is formed from the cloned code,
/// where its BBSet needs to be collected and the WRegion Tree needs to be
/// recomputed.
///
void VPOUtils::singleRegionMultiVersioning(
  BasicBlock *EntryBB,
  BasicBlock *ExitBB,
  SmallVectorImpl<BasicBlock*> &BBSet,
  Value *Cond,
  DominatorTree *DT
)
{
  assert(EntryBB && "no entry basic block for the region");
  assert(ExitBB && "no exit basic block for the region");

  assert(EntryBB->getSinglePredecessor() && "Not a single-entry region");
  assert(ExitBB->getSingleSuccessor() && "Not a single-exit region");

  // 1) Add NewHead and NewTail
  // 
  // EntryBB has a single predecessor, so split the top of the entryBB.
  // SplitBlock will return the bottom half of the original block as a new
  // block.
  BasicBlock *NewHead = EntryBB;
  EntryBB = SplitBlock(NewHead, &NewHead->front(), DT);
  // ExitBB has a single successor, so split it at the bottom of the exitBB.
  BasicBlock *NewTail = SplitBlock(ExitBB, ExitBB->getTerminator(), DT);

  // 2) Collect BasicBlock Set if it is empty, based on EntryBB and ExitBB.
  //
  if (BBSet.empty())
    IntelGeneralUtils::collectBBSet(EntryBB, ExitBB, BBSet); 

  // 3) Clone BBSet into the same function F.
  //
  ValueToValueMapTy VMap;
  VPOSmallVectorBB ClonedBBSet;
  Function* F = EntryBB->getParent();

  // DT and LT are not maintained, and the cloned basic blocks are attached
  // at the end of the function. The ClonedExitBB will branch to NewTail
  // automatically.
  cloneBBSet(BBSet, ClonedBBSet, VMap, ".clone", F); 

  // Move the cloned blocks from the end of the basic block list to the
  // proper place, which is right before NewTail. This is to maintain code
  // or instruction locality. It does not affect CFG.
  BasicBlock *ClonedEntryBB = ClonedBBSet.front();
  F->getBasicBlockList().splice(NewTail->getIterator(), 
                                F->getBasicBlockList(),
                                ClonedEntryBB->getIterator(),
                                F->end());

  // 4) Hook the cloned region to the CFG.
  //
  // Connect ClonedEntryBB to NewHead. ReplaceInstWithInst will also copy
  // the DebugLoc info. Note that, ClonedExitBB branches to NewTail
  // automatically after cloning.
  //
  TerminatorInst *OldTerm = NewHead->getTerminator();
  TerminatorInst *NewTerm = BranchInst::Create(
          /*ifTrue*/EntryBB, /*ifFalse*/ClonedEntryBB, Cond);
  ReplaceInstWithInst(OldTerm, NewTerm);

  // 5) Find live-out values in BBSet, and add PHI instructions for those
  // values in NewTail.
  //
  VPOSmallVectorInst LiveOut;
  findDefsUsedOutsideOfRegion(BBSet, LiveOut);
  addPHINodes(VMap, BBSet, LiveOut); 

  // 6) Update Dominator Tree information
  if (DT) {
    DT->addNewBlock(ClonedEntryBB, NewHead); 

    for (BasicBlock *BB : BBSet) {
      // ClonedEntryBB is dealt with already.
      if (BB != EntryBB) {
        // Do a one-to-one mapping change.
        BasicBlock *NewBB = cast<BasicBlock>(VMap[BB]);
        BasicBlock *IDomBB = DT->getNode(BB)->getIDom()->getBlock();
        DT->addNewBlock(NewBB, cast<BasicBlock>(VMap[IDomBB]));
      }
    }  

    // The two regions merge in the NewTail, dominated by the NewHead now.
    DT->changeImmediateDominator(NewTail, NewHead);
  }
}
