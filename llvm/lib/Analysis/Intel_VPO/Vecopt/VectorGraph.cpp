//===----------------------------------------------------------------------===//
//
//   Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_VPO/Vecopt/VectorGraph.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VectorGraphUtils.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/LoopInfo.h"

#define DEBUG_TYPE "vector-graph-node"

using namespace llvm;

unsigned VGNode::GlobalNumber(0);

VGNode::VGNode(unsigned SCID)
    : SubClassID(SCID), Parent(nullptr), Predicate(nullptr)
{ Number = GlobalNumber++; }

VGNode::VGNode(const VGNode &Obj)
  : SubClassID(Obj.getVGID()) {}

void VGNode::destroyAll() {}

void VGNode::destroy() {
  delete this;
}

void VGNode::setNumber() {}

void VGNode::dump() const {
  formatted_raw_ostream OS(dbgs());
  print(OS, 1);
}

VGLoop *VGNode::getParentLoop() const {
  return nullptr;
}

// TODO Lp is no longer needed
VGBlock *VGLoop::getOrInsertBlock(Loop *Lp, BasicBlock *BB) {

  VGBlock* VBlock;

  std::map<BasicBlock *, VGBlock *>::iterator BlockIt;
  BlockIt = BlockMap.find(BB);

  if (BlockIt == BlockMap.end()) {
    VBlock = VectorGraphUtils::createVGBlock(BB);
    BlockMap[BB] = VBlock;
    Size++;
  } else {
    VBlock = BlockIt->second;
  }

  return VBlock;
}

VGBlock *VGLoop::getBlock(Loop *Lp, BasicBlock *BB) {

  assert(Lp->contains(BB) && "Loop does not contain BasicBlock");

  auto BlockIt = BlockMap.find(BB);
  assert(BlockIt != BlockMap.end() && "Block not found in BlockMap");

  return BlockIt->second;
}

void VGLoop::addSuccessors(Loop *Lp, VGBlock *VGB) {

  BasicBlock *BB = VGB->getBasicBlock();

  for (succ_iterator SI = succ_begin(BB), SE = succ_end(BB); SI != SE; ++SI) {
    //errs() << "Successor: " << (*SI)->getName() << "\n";
    VGBlock* Successor = getOrInsertBlock(Lp, *SI);
    // We now build the VGraph for any block inside the loop + the first
    // level of successors outside of the loop.
    // In this way, we do not have to create a dummy block and we won't have
    // to do anything special in the future if we introduce support for loops
    // with early exits that jump to the loop exit block (break statements).
    // This decision was made to fix the issues found in insertLoopExitBlock
    // (see comments below).
    //if (Successor)
      VectorGraphUtils::addSuccessor(VGB, Successor);
  }
}

// We no longer need this function. We use, fixLoopExit instead.
//void VGLoop::insertLoopExitBlock(Loop *Lp) {
  // PostDominatorTree construction requires a single exit node. A dummy VGBlock
  // exit is created here.

  // When we have a conditional branch in the latch, the successor that is
  // outside of the loop is not created because it's not considered part of
  // the loop. This means that we do not have this successor in the VGraph
  // representation and we need to add a dummy VGBlock/successor.
  // When the branch is unconditional, the successor is inside of the loop
  // and we do not need to add the dummy VGBlock/successor.
  // TODO: Critical edges that jump outside of the loop may also need a dummy
  // VGBlock. If the critical edge jumps to the loop exit, the dummy block
  // for the critical edge should be the same as the dummy block for the
  // loop latch.
//
//  BasicBlock *Latch = Lp->getLoopLatch(); 
//  assert (Latch && "Loop does not have a single latch");
//
//  Instruction *Terminator = Latch->getTerminator();
//  assert (isa<BranchInst>(Terminator) && "Expected a BranchInstr");
//  BranchInst *Branch = cast<BranchInst>(Terminator);
//
//  if (Branch->isConditional()) {
//    VGBlock *VGLoopLatch = BlockMap[Latch];
//    Exit = VectorGraphUtils::createVGBlock(nullptr);
//    VectorGraphUtils::addSuccessor(VGLoopLatch, Exit);
//  }
//}

// We remove the exit block from the BlockMap as it is not part of the loop.
void VGLoop::fixLoopExit(Loop *Lp) {
  BasicBlock *ExitBB = Lp->getUniqueExitBlock();
  assert(ExitBB && "Expected only one exit block");

  // Mark the exit VGBlock.
  Exit = BlockMap[ExitBB];

  // Remove exit from BlockMap.
  BlockMap.erase(ExitBB);
}

VGLoop::VGLoop(Loop *Lp) : VGNode(VGNode::VGLoopNode), LLoop(Lp) {
  Size = 0;
  for (auto Itr = Lp->block_begin(), End = Lp->block_end(); Itr != End; ++Itr) {
    //errs() << "Block: " << **Itr << "\n";
    BasicBlock *BB = *Itr;
    VGBlock *VBlock = getOrInsertBlock(Lp, BB);
    assert(VBlock && "Basic block was not found in the loop");
    VectorGraphUtils::insertLastChild(this, VBlock);
    addSuccessors(Lp, VBlock);
  }

  // Set the VGLoop's loop latch
  assert(Lp->getLoopLatch() && "Single loop latch expected");
  LoopLatch = BlockMap[Lp->getLoopLatch()];

  fixLoopExit(Lp);

  // Mark the entry VGBlock.
  Entry = cast<VGBlock>(getFirstChild());

//#if !defined(NDEBUG)
//  for (auto Itr = child_begin(), End = child_end(); Itr != End; ++Itr) {
//
//    VGBlock *VGB = cast<VGBlock>(Itr);
//    const SmallVectorImpl<VGBlock*> &Preds = VGB->getPredecessors();
//    const SmallVectorImpl<VGBlock*> &Succs = VGB->getSuccessors();
//
//    errs() << "Predecessors for " << VGB->getBasicBlock()->getName() << ":\n";
//    for (unsigned i = 0; i < Preds.size(); i++) {
//      errs() << Preds[i]->getBasicBlock()->getName() << "\n";
//    }
//
//    errs() << "Successors for " << VGB->getBasicBlock()->getName() << ":\n";
//    for (unsigned i = 0; i < Succs.size(); i++) {
//      if (Succs[i]->getBasicBlock()) {
//        errs() << Succs[i]->getBasicBlock()->getName() << "\n";
//      }
//    }
//  }
//#endif
}

VGLoop *VGLoop::clone() const { return nullptr; }

VGNode *VGLoop::getFirstChild() {
  if (hasChildren()) {
    return &*child_begin();
  }
  return nullptr;
}

VGNode *VGLoop::getLastChild() {
  if (hasChildren()) {
    return &*(std::prev(child_end()));
  } else {
    return nullptr;
  }
}

void VGLoop::print(formatted_raw_ostream &OS, unsigned Depth) const {
#if !INTEL_PRODUCT_RELEASE  
  std::string Indent((Depth * TabLength), ' ');

  OS << Indent;
  OS << "(" << getNumber() << ") ";
  printNodeKind(OS);
  OS << Indent << "{\n";

  Depth++;

  // Print Loop Children
  for (auto Itr = child_begin(), End = child_end(); Itr != End; ++Itr)
    Itr->print(OS, Depth);

  OS << Indent << "}\n";
#endif // !INTEL_PRODUCT_RELEASE  
}

void VGLoop::printNodeKind(formatted_raw_ostream &OS) const {
#if !INTEL_PRODUCT_RELEASE  
  OS << "LOOP";
#endif // !INTEL_PRODUCT_RELEASE  
}

//----------------------------------------------//
VGBlock::VGBlock(BasicBlock *BB) : VGNode(VGNode::VGBlockNode),
				   BBlock(BB) {}

VGBlock *VGBlock::clone() const { return nullptr; }

void VGBlock::print(formatted_raw_ostream &OS, unsigned Depth) const {
#if !INTEL_PRODUCT_RELEASE  

  std::string Indent((Depth * TabLength), ' ');

  OS << Indent;
  OS << "(" << getNumber() << ") ";
  if (BBlock)
    OS << "\n" << "Label: " << *BBlock << "\n";
  printNodeKind(OS);
  OS << "{}\n";
#endif // !INTEL_PRODUCT_RELEASE  
}

void VGBlock::print(raw_ostream &OS, unsigned Depth) const {
#if !INTEL_PRODUCT_RELEASE  

  std::string Indent((Depth * TabLength), ' ');

  OS << Indent;
  OS << "(" << getNumber() << ") ";
  if (BBlock)
    OS << "\n" << "Label: " << *BBlock << "\n";
  printNodeKind(OS);
  OS << "{}\n";
#endif // !INTEL_PRODUCT_RELEASE  
}

void VGBlock::printNodeKind(formatted_raw_ostream &OS) const {
#if !INTEL_PRODUCT_RELEASE  
  OS << "Block";
#endif // !INTEL_PRODUCT_RELEASE  
}

void VGBlock::printNodeKind(raw_ostream &OS) const {
#if !INTEL_PRODUCT_RELEASE  
  OS << "Block";
#endif // !INTEL_PRODUCT_RELEASE  
}

//-----------Predicate Implementation----------//
VGPredicate::VGPredicate()
    : VGNode(VGNode::VGPredicateNode), isAllOnePredicate(true) {}

VGPredicate *VGPredicate::clone() const { return nullptr; }

void VGPredicate::print(formatted_raw_ostream &OS, unsigned Depth) const {
#if !INTEL_PRODUCT_RELEASE  
  std::string Indent((Depth * TabLength), ' ');

  OS << Indent;

  OS << "(" << getNumber() << ") ";
  OS << " {";
  OS << "P" << getNumber() << " := ";
  unsigned IncomingNum = IncomingPredicates.size();
  for (unsigned Ind = 0; Ind < IncomingNum; ++Ind) {
    if (Ind > 0)
      OS << " || ";
    auto &Incoming = IncomingPredicates[Ind];
    VGPredicate *Pred = Incoming.first; //std::get<0>(Incoming);
    Value *Cond = Incoming.second.first; //std::get<1>(Incoming);

    OS << "(" << Pred->getNumber() << ")";
    if (Cond) {
      OS << " && ";

      bool CondNeedsNegation = Incoming.second.second; //std::get<3>(Incoming);
      if (CondNeedsNegation) {
        OS << "!";
      }

      Cond->print(OS, 0);
    }
  }
  OS << "}";
  OS << "\n";
#endif // !INTEL_PRODUCT_RELEASE  
}

void VGPredicate::printNodeKind(formatted_raw_ostream &OS) const {
#if !INTEL_PRODUCT_RELEASE  
  OS << "Predicate";
#endif // !INTEL_PRODUCT_RELEASE  
}
