//===-- IntelSNode.cpp - Structure Node Analysis
//--------------------------------------==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the implementation of the structure node analysis
// engine, which can be used by compiler analysis, high level transformation or
// program understanding such as pretty print.
//
//===----------------------------------------------------------------------===//
//
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/Analysis/Intel_SNode.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/Debug.h"
#include <algorithm>

using namespace llvm;

#define DEBUG_TYPE "sna"

INITIALIZE_PASS_BEGIN(SNodeAnalysis, "sna", "Structure Node Analysis", false,
                      true)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_END(SNodeAnalysis, "sna", "Structure Node Analysis", false,
                    true)

char SNodeAnalysis::ID = 0;

FunctionPass *llvm::createSNodeAnalysisPass() { return new SNodeAnalysis(); }

SNodeAnalysis::SNodeAnalysis() : FunctionPass(ID) {
  initializeSNodeAnalysisPass(*PassRegistry::getPassRegistry());
}

bool SNodeAnalysis::runOnFunction(Function &F) {
  this->F = &F;
  LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  doRoutineLevelSNodeAnalyses();
  return false;
}

void SNodeAnalysis::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequiredTransitive<LoopInfoWrapperPass>();
}

// creates a new SNode given an incoming SNode type.
SNode *SNodeAnalysis::genSNode(SNode::SNodeOp SnOp) {
  SNode *Snode;
  switch (SnOp) {
  case SNode::SN_BLOCK:
    Snode = new BlockSNode();
    break;
  case SNode::SN_LIST:
    Snode = new ListSNode();
    break;
  case SNode::SN_IF_THEN_ELSE:
    Snode = new IfThenElseSNode();
    break;
  default:
    llvm_unreachable("Unsupported SNode Type!");
  }
  Snode->setParentSn(Snode);
  SNodes.push_back(Snode);
  Snode->setSNodeNum(SNodes.size()-1);
  return Snode;
}

// Returns a new sn_block for the given basic block.
SNode *SNodeAnalysis::makeSnBlock(BasicBlock *Block) {
  SNode *Snode;
  BlockSNode *BSNode;

  Snode = genSNode(SNode::SN_BLOCK);
  BSNode = static_cast<BlockSNode *>(Snode);
  BB2SNodeMap.insert(std::make_pair(Block, BSNode));
  BSNode->setBblock(Block);
  Snode->setLoop(LI->getLoopFor(Block));
  return Snode;
}

// Returns the corresponding SNode for the given basic block.
BlockSNode *SNodeAnalysis::getSNodeForBlock(BasicBlock *Block) const {
  assert(BB2SNodeMap.find(Block) != BB2SNodeMap.end() &&
         "expected non-empty SNode for the given basic block");
  return BB2SNodeMap.find(Block)->second;
}

// Updates the succs of the sn_block according to its 
// basic block's succs.
void SNodeAnalysis::makeSNodeSuccList(BlockSNode *Snode) {
  BasicBlock *Block = Snode->getBblock();
  for (succ_iterator I = succ_begin(Block), E = succ_end(Block); I != E; ++I) {
    Snode->addSucc(getSNodeForBlock(*I));
  }
}

// Updates the preds of the sn_block accoridng to its 
// basic block's preds.
void SNodeAnalysis::makeSNodePredList(BlockSNode *Snode) {
  BasicBlock *Block = Snode->getBblock();
  for (pred_iterator I = pred_begin(Block), E = pred_end(Block); I != E; ++I) {
    Snode->addPred(getSNodeForBlock(*I));
  }
}

// Creates the SNode graph based on the basic block's CFG.
void SNodeAnalysis::makeSNodeGraph(BasicBlock *Block) {
  BlockSNode *BSnode;

  BSnode = getSNodeForBlock(Block);
  makeSNodeSuccList(BSnode);
  makeSNodePredList(BSnode);
}

// Returns the first basic block for a given SNode.
BasicBlock *SNode::getFirstBlock() const {
  if (getOp() == SN_BLOCK)
    return static_cast<const BlockSNode *>(this)->getBblock();
  return getFirstChild()->getFirstBlock();
}

// Returns the last basic block of the last child SNode. 
// The significance of this block varies depending on the SNode type.
BasicBlock *SNode::getLastBlock() const {
  if (getOp() == SN_BLOCK)
    return static_cast<const BlockSNode *>(this)->getBblock();
  return getLastChild()->getLastBlock();
}

// If the SNode has only one pred, returns true and gets the pred SNode.
bool SNode::hasOnePred(SNode **SnPred1) {
  if (Pred.size() != 1) 
    return false;
  
  *SnPred1 = Pred.front();
  return true;
}

// If the number of successors is 1, returns true
// and updates *Pred1 with the unique predecessor.
bool SNode::hasOneSucc(SNode **SnSucc1) {
  if (Succ.size() != 1) 
    return false;
  
  *SnSucc1 = Succ.front();
  return true;
}

// If the number of predecessors is 2, returns true 
// and updates the *Pred1 and *Pred2 with the predecessors.
bool SNode::hasTwoPred(SNode **SnPred1, SNode **SnPred2) {
  if (Pred.size() != 2) 
    return false;
  
  *SnPred1 = Pred.front();
  *SnPred2 = Pred.back();
  return true;
}

// If the number of successors is more than 2 and the successors
// are unique, returns true and replaces the list SnSuccs with 
// the successors.
// This routine returns false when the successors are not unique.
// For example, there could be three successors 
// arcs, but if there are only two actual unique successor 
// blocks, then this should return false.
bool SNode::hasMoreThanTwoSucc(SNodeListTy *SnSuccs) {
  if (Succ.size() <= 2) 
    return false;

  assert(SnSuccs->empty() && "expected the incoming set is empty");
  
  SmallPtrSet<SNode *, 8> Visited;
  for (snode_list_iterator I = Succ.begin(), E = Succ.end(); I != E; ++I) {
    if (!Visited.count(*I)) 
      Visited.insert(*I);
    else
      return false;
    SnSuccs->push_back(*I);
  }
  return true;
}

// If the number of successors is 2, returns true
// and updates the *Succ1 and *Succ2 with the successors.
// For a node that has a true and false target, the true target 
// would always be returned in SnSucc1, and the false target 
// would always be returned in SnSucc2.
bool SNode::hasTwoSucc(SNode **SnSucc1, SNode **SnSucc2) {
  if (Succ.size() != 2) 
    return false;
  
  SNode *FirstSucc = Succ.front();
  SNode *SecondSucc = Succ.back();

  if (getTrueTarget() != nullptr &&
      SecondSucc == getTrueTarget()) {
    *SnSucc1 = SecondSucc;
    *SnSucc2 = FirstSucc;
  } else {
    *SnSucc1 = FirstSucc;
    *SnSucc2 = SecondSucc;
  }
  return true;
}

// Returns the true target basic block for BlockSNode
const SNode* BlockSNode::getTrueTarget() const {
  BranchInst *BI = dyn_cast<BranchInst>(B->getTerminator());
  if (B->getTerminator()->getNumSuccessors() == 2 && BI) {
    assert(getFirstSucc()->getFirstBlock() == BI->getSuccessor(0) && 
           "expected the first successor SNode is the true target"); 

    return getFirstSucc();
  }
  
  return nullptr;
}

// Returns the false target basic block for BlockSNode
const SNode* BlockSNode::getFalseTarget() const {
  BranchInst *BI = dyn_cast<BranchInst>(B->getTerminator());
  if (B->getTerminator()->getNumSuccessors() == 2 && BI) {
    assert(getLastSucc()->getFirstBlock() == BI->getSuccessor(1) &&
           "expected the second successor SNode is the false target"); 
    return getLastSucc();
  }
  
  return nullptr;
}

// Returns true if the last SNode is sn_block or sn_or.
bool SNodeAnalysis::TailSnIsSnblockOrSnor(SNode *Snode) {
  const SNode *LastSn = Snode;

  while (LastSn->getOp() == SNode::SN_LIST) {
    LastSn = LastSn->getLastChild();
  }

  if (LastSn->getOp() == SNode::SN_BLOCK) 
    return true;

  return false;
}


// Updates the loop info for this SNode based on the given From SNode.
void SNode::setLoop(SNode *From) {
  setLoop(From->getLoop());
}


// Given a SNode, checks whether it has unique successor  
// which can form a list SNode with itself or not. If so, *SnNext is 
// updated as this successor SNode and returns true. Otherwise, 
// returns false
bool SNodeAnalysis::isSNodeList(SNode *Snode, SNode **SnNext) {
  
  *SnNext = nullptr;

  SNode *SnSucc = nullptr, *SnPred = nullptr;
  if (Snode->hasOneSucc(&SnSucc) && SnSucc != Snode &&
      SnSucc->hasOnePred(&SnPred) && SnPred == Snode) {
    *SnNext = SnSucc;
    return true;
  }
  return false;
}

// Given SNode Sn1 and Sn2 which is the unique successor of Sn1, 
// generates a list SNode whose children are in the order of Sn1 
// and Sn2.
// The preds and succs of new list SNode will be updated 
// according to the preds and succs of Sn1 and Sn2.
// The succ of Sn1 needs to be cleared. Same for Sn2's pred.
// TODO: If Sn2 is list SNode, it should only add its children 
// into the new list SNode.
// The new sn_list is in the following form.
// sn_list
//   sn1
//   sn2
//
SNode *SNodeAnalysis::genSNodeList(SNode *Sn1, SNode *Sn2) {
  SNode *Snode;

  Snode = genSNode(SNode::SN_LIST);
  Snode->addChild(Sn1);
  Snode->addChild(Sn2);
  Snode->setLoop(Sn1);

  Sn1->setParentSn(Snode);
  Sn2->setParentSn(Snode);

  Snode->inheritPredsNSuccs(Sn1, Sn2);

  Sn1->clearSucc();
  Sn2->clearPred();

  return Snode;
}

// Given a SNode, looks for its two successors to 
// see whether SNode and its two successors can form an 
// if-then-else control flow. If so, returns true and updates 
// *SnThen and *SnElse with its two successors. Otherwise returns false.
bool SNodeAnalysis::isSNodeIfThenElse(SNode *Snode, SNode **SnThen,
                                      SNode **SnElse) {
  if (!TailSnIsSnblockOrSnor(Snode)) {
    return false;
  }

  SNode *SnPred1, *SnPred2, *SnNext1, *SnNext2;

  *SnThen = *SnElse = nullptr;

  if (Snode->hasTwoSucc(SnThen, SnElse) && 
      (*SnThen)->hasOneSucc(&SnNext1) &&
      (*SnElse)->hasOneSucc(&SnNext2) && 
      SnNext1 == SnNext2 &&
      (*SnThen)->hasOnePred(&SnPred1) && 
      (*SnElse)->hasOnePred(&SnPred2)) 
    return true;
  
  return false;
}

// Given three SNodes SnIf, SnThen and SnElse, generates a sn_if_then_else 
// whose children are in the order of SnIf, SnThen and SnElse. 
// The preds and succs of new if-then-else SNode will be updated 
// according to the preds and succs of SnIf, SnThen and SnElse.
// The sn_if_then_else is in the following form.
//  sn_if_then_else
//    sn_if
//    sn_then
//    sn_else
//
SNode *SNodeAnalysis::genSNodeIfThenElse(SNode *SnIf, SNode *SnThen,
                                         SNode *SnElse) {
  SNode *Snode;

  Snode = genSNode(SNode::SN_IF_THEN_ELSE);
  Snode->addChild(SnIf);
  Snode->addChild(SnThen);
  Snode->addChild(SnElse);

  SnIf->setParentSn(Snode);
  SnThen->setParentSn(Snode);
  SnElse->setParentSn(Snode);

  Snode->setLoop(SnIf);

  Snode->inheritPredsNSuccs(SnIf, SnThen);

  // The pred set of the SnElse's successors needs to be updated
  // after the IfThenElseSNode is created. 
  Snode->deletePredsFromSucc(SnElse);

  SnIf->clearSucc();
  SnThen->clearPred();
  SnElse->clearPred();
  SnElse->clearSucc();

  return Snode;
}

// Generates the SNode block for every basic block and updates
// the preds and succs based on the CFG.
void SNodeAnalysis::createSNodeBlocks() {
  for (Function::iterator B = F->begin(), Be = F->end(); B != Be; ++B) {
    makeSnBlock(&*B);
  }

  for (Function::iterator B = F->begin(), Be = F->end(); B != Be; ++B) {
    makeSNodeGraph(&*B);
  }
}


// This is the kernel function of the SNodeInfo analysis. The basic idea
// is to do patten match, generate the structured SNode and keep
// merging.
void SNodeAnalysis::doRoutineLevelSNodeAnalyses() {
  BasicBlock *Block;
  SNode *Snode, *Sn1, *Sn2;
  SmallVector<BasicBlock *, 32> BasicBlockVector;
  SNodeListTy SnSucc;

  Loop2SNodeMap.clear();
  BB2SNodeMap.clear();
  createSNodeBlocks();
  for (df_iterator<BasicBlock *> I = df_begin(&F->getEntryBlock()),
       E = df_end(&F->getEntryBlock()); I != E; ++I) {
    BasicBlockVector.push_back(*I);
  }

  for (auto I = BasicBlockVector.rbegin(), E = BasicBlockVector.rend();
       I != E; I++) {
    Block = *I;

    Snode = getSNodeForBlock(Block);

    // This loop takes Snode, and combines it with neighboring
    // Snodes until no more can be done.  In order it tests 
    // for various types of Snodes that can be created, and 
    // as soon as a possible combination is found that 
    // transformation is done, and it loops back to the top.
    // If no combination is done, then we are done with the loop.
    while (1) {
      if (isSNodeList(Snode, &Sn1) == true) {
        Snode = genSNodeList(Snode, Sn1);
        continue;
      }
      if (isSNodeIfThenElse(Snode, &Sn1, &Sn2) == true) {
        Snode = genSNodeIfThenElse(Snode, Sn1, Sn2);
        continue;
      }
      break;
    }
  }
  assert(Snode && "Expected non-empty incoming entry SNode");
  EntrySNode = Snode;
}

void SNodeAnalysis::releaseMemory() {
  for (snode_vector_iterator I = SNodes.begin(), E = SNodes.end(); I != E;
       I++) {
    (*I)->releaseMemory();
  }
  for (snode_vector_iterator I = SNodes.begin(), E = SNodes.end(); I != E;
       I++) {
    delete *I;
  }
  BB2SNodeMap.clear();
  Loop2SNodeMap.clear();
  SNodes.clear();
}

void SNodeAnalysis::printIndent(int Indent, raw_ostream &OS) const {
  for (int I = 0; I < Indent; I++)
    OS << " ";
}

// Prints the SNode's unique id.
void SNodeAnalysis::printSNodeNum(const SNode *Snode, raw_ostream &OS) const {
  OS << "SN[" << Snode->getSNodeNum() << "]";
}

// Prints the succs' id of a SNode.
void SNodeAnalysis::printSNodeListStructureForSucc(const SNode *Snode,
                                                   raw_ostream &OS) const {
  for (const_snode_list_iterator I = Snode->snSuccCBegin(), 
       E = Snode->snSuccCEnd();
       I != E; ++I) {
    printSNodeNum(*I, OS);
    OS << " ";
  }
}

// Prints the preds'id of a SNode.
void SNodeAnalysis::printSNodeListStructureForPred(const SNode *Snode,
                                                   raw_ostream &OS) const {
  for (const_snode_list_iterator I = Snode->snPredCBegin(), 
       E = Snode->snPredCEnd();
       I != E; ++I) {
    printSNodeNum(*I, OS);
    OS << " ";
  }
}

// Prints the basic block name.
void SNodeAnalysis::printBlockName(const BasicBlock *Bb, raw_ostream &OS) const {
  OS << "B[";
  Bb->printAsOperand(OS, false);
  OS << "]";
}

// Prints the SNodes in hierarchical structure.
void SNodeAnalysis::printSNodeStructure(const SNode *Snode, int Level,
                                        raw_ostream &OS) const {
  if (Snode == nullptr)
    return;
  if (Snode->getParentSn() != Snode)
    assert(Snode->predIsEmpty() && Snode->succIsEmpty() &&
           "expected empty preds/succs for non-root Snode");
  printIndent(Level, OS);
  if (Snode->getOp() == SNode::SN_BLOCK) {
    OS << snOpName(Snode->getOp()) << " ";
    printSNodeNum(Snode, OS);
    OS << " ";
    printBlockName(Snode->getFirstBlock(), OS);
    OS << "\n";
  } else {
    OS << snOpName(Snode->getOp()) << " ";
    printSNodeNum(Snode, OS);
    if (Snode->numChildren()) {
      OS << "\n";
      for (const_snode_children_iterator I = Snode->childCBegin(), 
           E = Snode->childCEnd(); I != E;
           ++I) {
        printSNodeStructure(&*I, Level + 4, OS);
      }
    }
    printIndent(Level, OS);
    OS << "END  " << snOpName(Snode->getOp()) << " ";
    printSNodeNum(Snode, OS);
    OS << "\n";
  }
}

// Prints out single SNode information.
void SNodeAnalysis::dumpSingleSNode(const SNode *Snode,
                                    raw_ostream &OS) const {
  OS << snOpName(Snode->getOp()) << " ";
  printSNodeNum(Snode, OS);
  OS << " pred: ";
  printSNodeListStructureForPred(Snode, OS);
  OS << " succ: ";
  printSNodeListStructureForSucc(Snode, OS);
  OS << "( ";
  if (Snode->getOp() != SNode::SN_BLOCK) {
    for (const_snode_children_iterator I = Snode->childCBegin(), 
         E = Snode->childCEnd();
         I != E; ++I) {
      printSNodeNum(&*I, OS);
    }
  } else {
    printBlockName(Snode->getFirstBlock(), OS);
  }
  OS << " )";
  OS << "\n";
}

// Prints the SNodes in hierarchical structure. Please note that
// this routine is only called for Snodes at the top of the tree.
void SNodeAnalysis::dumpSNodeStructure(SNode *Snode, 
                                       raw_ostream &OS,
                                       SmallPtrSetImpl<SNode *> &Visited) {
  if (!Visited.count(Snode)) {
    Visited.insert(Snode);
    assert(Snode->getParentSn() == Snode && "unexpected non-root Snode");
    printSNodeNum(Snode, OS);

    OS << " pred: ";
    printSNodeListStructureForPred(Snode, OS);
    OS << " succ: ";
    printSNodeListStructureForSucc(Snode, OS);

    if (Snode->getOp() != SNode::SN_BLOCK) 
      OS << "\n";

    printSNodeStructure(Snode, 0, OS);
    for (snode_list_iterator I = Snode->snSuccBegin(),
         E = Snode->snSuccEnd();
         I != E; ++I) {
      dumpSNodeStructure(*I, OS, Visited);
    }
  }
}

// The utility to print out the SNodes without hierarchical tree.
static void dumpSNodes(raw_ostream &OS, SNodeAnalysis *SN) {
  for (const_snode_vector_iterator I = SN->begin(), E = SN->end(); I != E;
       ++I) 
    SN->dumpSingleSNode(*I, OS);
}

// This is the main driver to print out the SNode graph hierarchically.
static void dumpSNodesStructure(raw_ostream &OS, SNodeAnalysis *SN) {
  SmallPtrSet<SNode *, 8> Visited;

  Visited.clear();
  SN->dumpSNodeStructure(SN->getEntrySNode(), OS, Visited);
}

void SNodeAnalysis::print(raw_ostream &OS, const Module *M) const {
  dumpSNodesStructure(OS, const_cast<SNodeAnalysis *>(this));
}

void SNodeAnalysis::printSeq(raw_ostream &OS, const Module *M) const {
  dumpSNodes(OS, const_cast<SNodeAnalysis *>(this));
}

