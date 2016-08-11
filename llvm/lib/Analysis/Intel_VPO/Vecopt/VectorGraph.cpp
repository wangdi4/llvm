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
using namespace llvm::vpo;

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

VGBlock* VGLoop::getOrInsertBlock(Loop *Lp, BasicBlock *BB) {

  VGBlock* VBlock = nullptr;

  if (Lp->contains(BB)) {
    std::map<BasicBlock*, VGBlock*>::iterator BlockIt;
    BlockIt = BlockMap.find(BB);

    if (BlockIt == BlockMap.end()) {
      VBlock = VectorGraphUtils::createVGBlock(BB);
      BlockMap[BB] = VBlock;
      Size++;
    } else {
      VBlock = BlockMap[BB];
    }
  }

  return VBlock;
}

void VGLoop::addSuccessors(Loop *Lp, VGBlock *VGB) {

  BasicBlock *BB = VGB->getBasicBlock();

  for (succ_iterator SI = succ_begin(BB), SE = succ_end(BB); SI != SE; ++SI) {
    errs() << "Successor: " << (*SI)->getName() << "\n";
    VGBlock* Successor = getOrInsertBlock(Lp, *SI);
    if (Successor)
      VectorGraphUtils::addSuccessor(VGB, Successor);
  }
}

void VGLoop::insertLoopExitBlock(Loop *Lp) {
  // PostDominatorTree construction requires a single exit node. A dummy VGBlock
  // exit is created here.
  BasicBlock *Latch = Lp->getLoopLatch(); 
  VGBlock *VGLoopLatch = BlockMap[Latch];
  Exit = VectorGraphUtils::createVGBlock(nullptr);
  VectorGraphUtils::addSuccessor(VGLoopLatch, Exit);
}

VGLoop::VGLoop(Loop *Lp) : VGNode(VGNode::VGLoopNode), LLoop(Lp) {
  Size = 0;
  for (auto Itr = Lp->block_begin(), End = Lp->block_end(); Itr != End; ++Itr) {
    errs() << "Block: " << **Itr << "\n";
    BasicBlock *BB = *Itr;
    VGBlock *VBlock = getOrInsertBlock(Lp, BB);
    assert(VBlock && "Basic block was not found in the loop");
    VectorGraphUtils::insertLastChild(this, VBlock);
    addSuccessors(Lp, VBlock);
  }

  insertLoopExitBlock(Lp);

  // Mark the entry VGBlock.
  Entry = cast<VGBlock>(getFirstChild());

#if !defined(NDEBUG)
  for (auto Itr = child_begin(), End = child_end(); Itr != End; ++Itr) {

    VGBlock *VGB = cast<VGBlock>(Itr);
    const SmallVectorImpl<VGBlock*> &Preds = VGB->getPredecessors();
    const SmallVectorImpl<VGBlock*> &Succs = VGB->getSuccessors();

    errs() << "Predecessors for " << VGB->getBasicBlock()->getName() << ":\n";
    for (unsigned i = 0; i < Preds.size(); i++) {
      errs() << Preds[i]->getBasicBlock()->getName() << "\n";
    }

    errs() << "Successors for " << VGB->getBasicBlock()->getName() << ":\n";
    for (unsigned i = 0; i < Succs.size(); i++) {
      if (Succs[i]->getBasicBlock()) {
        errs() << Succs[i]->getBasicBlock()->getName() << "\n";
      }
    }
  }
#endif
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
}

void VGLoop::printNodeKind(formatted_raw_ostream &OS) const { OS << "LOOP"; }

//----------------------------------------------//
VGBlock::VGBlock(BasicBlock *BB) : VGNode(VGNode::VGBlockNode),
				   BBlock(BB) {}

VGBlock *VGBlock::clone() const { return nullptr; }

void VGBlock::print(formatted_raw_ostream &OS, unsigned Depth) const {

  std::string Indent((Depth * TabLength), ' ');

  OS << Indent;
  OS << "(" << getNumber() << ") ";
  if (BBlock)
    OS << "\n" << "Label: " << *BBlock << "\n";
  printNodeKind(OS);
  OS << "{}\n";
}

void VGBlock::print(raw_ostream &OS, unsigned Depth) const {

  std::string Indent((Depth * TabLength), ' ');

  OS << Indent;
  OS << "(" << getNumber() << ") ";
  if (BBlock)
    OS << "\n" << "Label: " << *BBlock << "\n";
  printNodeKind(OS);
  OS << "{}\n";
}

void VGBlock::printNodeKind(formatted_raw_ostream &OS) const { OS << "Block"; }

void VGBlock::printNodeKind(raw_ostream &OS) const { OS << "Block"; }

//-----------Predicate Implementation----------//
VGPredicate::VGPredicate() : VGNode(VGNode::VGPredicateNode) {}

VGPredicate *VGPredicate::clone() const { return nullptr; }

void VGPredicate::print(formatted_raw_ostream &OS, unsigned Depth) const {}

void VGPredicate::printNodeKind(formatted_raw_ostream &OS) const {
  OS << "Predicate";
}
