//===--- VectorGraphUtils.cpp ---------------------------------------------===//
//===----------------------------------------------------------------------===//
#include "llvm/Analysis/Intel_VPO/Vecopt/VectorGraphUtils.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "vector-graph-utils"

using namespace llvm;

// Creation Utilities

VGLoop *VectorGraphUtils::createVGLoop(Loop *LP) {
  return new VGLoop(LP);
}

VGBlock *VectorGraphUtils::createVGBlock(BasicBlock *BB) {
  return new VGBlock(BB);
}

VGPredicate *VectorGraphUtils::createVGPredicate() {
  return new VGPredicate();
}

// Modification Utilities

void VectorGraphUtils::setPredicate(VGNode *Node, VGPredicate* Predicate) {
  Node->setPredicate(Predicate);
}

void VectorGraphUtils::addVGPredicateIncoming(VGPredicate *VPredicate,
                                              VGPredicate *IncomingPredicate,
                                              Value *IncomingCondition,
                                              bool CondNeedsNegation) {
  VPredicate->addIncoming(IncomingPredicate, IncomingCondition,
                          CondNeedsNegation);
}

void VectorGraphUtils::setBlockCondition(VGBlock* VBlock, Value *C) {
  VBlock->setCondition(C);
}

void VectorGraphUtils::addSuccessor(VGBlock* Block, VGBlock* Successor) {
  assert(Block && "Block is null");
  assert(Successor && "Successor is null");
  Block->addSuccessor(Successor);
}

void VectorGraphUtils::addSchedulingConstraint(VGBlock* Block, VGBlock* Constraint) {
  assert(Block && "Block is null");
  assert(Constraint && "Constraint is null");
  Block->addSchedulingConstraint(Constraint);
}

void VectorGraphUtils::setAllOnes(VGPredicate * Predicate, bool isAllOnes) {
  Predicate->setAllOnes(isAllOnes);
}

// Insertion Utilities

void VectorGraphUtils::insertSingleton(VGNode *DestinationParent,
                                       VectorGraphTy &DestinationContainer,
                                       VGItr InsertionPosition,
                                       VGNode *NewNode) {

  assert(DestinationParent && "Parent node is null!");
  NewNode->setParent(DestinationParent);
  DestinationContainer.insert(InsertionPosition, NewNode);
}

void VectorGraphUtils::insertSequence(VGNode *DestinationParent,
                                      VectorGraphTy &DestinationContainer,
                                      VectorGraphTy *SourceContainer,
                                      VGItr InsertionPosition) {

  VGItr SourceBegin = SourceContainer->begin();
  VGItr SourceEnd = SourceContainer->end();
  unsigned Distance = std::distance(SourceBegin, SourceEnd), I = 0;

  DestinationContainer.splice(InsertionPosition, *SourceContainer, SourceBegin,
                              SourceEnd);

  // Update the parents of top-most nodes.
  for (auto Itr = InsertionPosition; I < Distance; ++I, Itr--) {
    std::prev(Itr)->setParent(DestinationParent);
  }
}

void VectorGraphUtils::insertAfter(VGItr InsertionPosition, VGNode *Node) {
  VGNode *Parent = InsertionPosition->getParent();
  VectorGraphTy *Children = getChildrenContainer(Parent);
  insertSingleton(Parent, *Children, std::next(InsertionPosition), Node);
}

void VectorGraphUtils::insertBefore(VGItr InsertionPosition, VGNode *Node) {
  VGNode *Parent = InsertionPosition->getParent();
  VectorGraphTy *Children = getChildrenContainer(Parent);
  insertSingleton(Parent, *Children, InsertionPosition, Node);
}

void VectorGraphUtils::insertAfter(VGItr InsertionPosition,
                                   VectorGraphTy *NodeContainer) {
  VGNode *Parent = InsertionPosition->getParent();
  VectorGraphTy *Children = getChildrenContainer(Parent);
  insertSequence(Parent, *Children, NodeContainer,
                 std::next(InsertionPosition));
}

void VectorGraphUtils::insertBefore(VGItr InsertionPosition,
                                    VectorGraphTy *NodeContainer) {
  VGNode *Parent = InsertionPosition->getParent();
  VectorGraphTy *Children = getChildrenContainer(Parent);
  insertSequence(Parent, *Children, NodeContainer, InsertionPosition);
}

void VectorGraphUtils::insertAfter(VGItr InsertionPosition,
                                   VectorGraphTy &DestinationContainer,
                                   VGNode *Node) {
  VGNode *Parent = InsertionPosition->getParent();
  insertSingleton(Parent, DestinationContainer, std::next(InsertionPosition),
                  Node);
}

void VectorGraphUtils::insertBefore(VGItr InsertionPosition,
                                    VectorGraphTy &DestinationContainer,
                                    VGNode *Node) {
  VGNode *Parent = InsertionPosition->getParent();
  insertSingleton(Parent, DestinationContainer, InsertionPosition, Node);
}

void VectorGraphUtils::insertAfter(VGItr InsertionPosition,
                                   VectorGraphTy &DestinationContainer,
                                   VectorGraphTy *SourceContainer) {
  VGNode *Parent = InsertionPosition->getParent();
  insertSequence(Parent, DestinationContainer, SourceContainer,
                 std::next(InsertionPosition));
}

void VectorGraphUtils::insertBefore(VGItr InsertionPosition,
                                    VectorGraphTy &DestinationContainer,
                                    VectorGraphTy *SourceContainer) {
  VGNode *Parent = InsertionPosition->getParent();
  insertSequence(Parent, DestinationContainer, SourceContainer,
                 InsertionPosition);
}

void VectorGraphUtils::insertFirstChild(VGLoop *Parent, VGNode *Node) {
  insertSingleton(Parent, Parent->Children, Parent->child_begin(), Node);
}

void VectorGraphUtils::insertLastChild(VGLoop *Parent, VGNode *Node) {
  insertSingleton(Parent, Parent->Children, Parent->child_end(), Node);
}

void VectorGraphUtils::insertFirstChildren(VGLoop *Parent,
                                           VectorGraphTy *NodeContainer) {
  insertSequence(Parent, Parent->Children, NodeContainer,
                 Parent->child_begin());
}

void VectorGraphUtils::insertLastChildren(VGLoop *Parent,
                                          VectorGraphTy *NodeContainer) {
  insertSequence(Parent, Parent->Children, NodeContainer, Parent->child_end());
}

/*
void VectorGraphUtils::insertFirstChild(VGBlock *Parent, VGNode *Node) {
  insertSingleton(Parent, Parent->Children, Parent->child_begin(), Node);
}

void VectorGraphUtils::insertLastChild(VGBlock *Parent, VGNode *Node) {
  insertSingleton(Parent, Parent->Children, Parent->child_end(), Node);
}

void VectorGraphUtils::insertFirstChildren(VGBlock *Parent,
                                           VectorGraphTy *NodeContainer) {
  insertSequence(Parent, Parent->Children, NodeContainer,
                 Parent->child_begin());
}

void VectorGraphUtils::insertLastChildren(VGBlock *Parent,
                                          VectorGraphTy *NodeContainer) {
  insertSequence(Parent, Parent->Children, NodeContainer, Parent->child_end());
}
*/

// Move Utilities

void VectorGraphUtils::moveAfter(VGItr InsertionPos, VGNode *Node) {
  remove(Node);
  insertAfter(InsertionPos, Node);
}

void VectorGraphUtils::moveAsFirstChildren(VGLoop *VLoop, VGItr First,
                                           VGItr Last) {
  VectorGraphTy TempContainer;
  VGItr InsertionPosition = VLoop->child_begin();

  assert((First->getParent()==Last->getParent()) &&
      "First and Last nodes do not share common parent node");

  removeInternal(First, Last, &TempContainer, false);
  insertSequence(VLoop, VLoop->Children, &TempContainer, InsertionPosition);
}


void VectorGraphUtils::moveAsLastChildren(VGLoop *VLoop, VGItr First,
                                          VGItr Last) {
  VectorGraphTy TempContainer;
  VGItr InsertionPosition = VLoop->child_end();

  assert((First->getParent()==Last->getParent()) &&
      "First and Last nodes do not share common parent node");

  removeInternal(First, Last, &TempContainer, false);
  insertSequence(VLoop, VLoop->Children, &TempContainer, InsertionPosition);
}

/*
void VectorGraphUtils::moveAsFirstChildren(VGBlock *VBlock, VGItr First, VGItr Last) {

  VectorGraphTy TempContainer;
  VGItr InsertionPosition = VBlock->child_end();

  assert((First->getParent()==Last->getParent()) &&
      "First and Last nodes do not share common parent node");

  removeInternal(First, Last, &TempContainer, false);
  insertSequence(VBlock, VBlock->Children, &TempContainer, InsertionPosition);
}

void VectorGraphUtils::moveAsLastChildren(VGBlock *VBlock, VGItr First,
                                          VGItr Last) {

  VectorGraphTy TempContainer;
  VGItr InsertionPosition = VBlock->child_end();

  assert((First->getParent()==Last->getParent()) &&
      "First and Last nodes do not share common parent node");

  removeInternal(VGItr(Begin), VGItr(End), &TempContainer, false);
  insertSequence(VBlock, VBlock->Children, &TempContainer, InsertionPosition);
}
*/

// Removal Utilities

void VectorGraphUtils::destroy(VGNode *Node) { Node->destroy(); }

VectorGraphTy *VectorGraphUtils::removeInternal(VGItr Begin, VGItr End,
                                                VectorGraphTy *MoveContainer,
                                                bool Delete) {
  VGNode *Parent = Begin->getParent();
  VectorGraphTy *OrigContainer = getChildrenContainer(Parent);

  assert(OrigContainer && "Container missing for node removal!");
  assert(getChildrenContainer(End->getParent()) == OrigContainer &&
         "Range exceeds a single container");

  // Removal of VGNode or VGNode Sequence does not require move to new location.
  if (!MoveContainer) {

    // Remove Singleton
    if (Begin == End) {

      OrigContainer->remove(Begin);

      if (Delete)
        destroy(&*Begin);

      return nullptr;
    }

    // Remove Sequence
    for (auto I = Begin, Next = I, E = End; I != E; I = Next) {

      Next++;
      VGNode *Node = OrigContainer->remove(I);

      if (Delete)
        destroy(Node);
    }
  } else {
    MoveContainer->splice(MoveContainer->end(), *OrigContainer, Begin, ++End);
  }

  return MoveContainer;
}

// Remove singleton vector graph node.
void VectorGraphUtils::remove(VGNode *Node) {
  assert(Node && "Missing Vector Graph Node!");
  removeInternal(VGItr(Node), VGItr(Node), nullptr, false);
}

// Remove sequence of vector graph nodes.
void VectorGraphUtils::remove(VGItr Begin, VGItr End) {
  assert(Begin->getParent() == End->getParent() &&
         "Cannot remove. Begin and End do not share common parent node!");
  removeInternal(Begin, End, nullptr, false);
}

// Helper Functions

VectorGraphTy *VectorGraphUtils::getChildrenContainer(VGNode *Parent) {

  if (VGLoop *LpNode = dyn_cast<VGLoop>(Parent))
    return &LpNode->Children;

  // Utilities should only insert/delete nodes in/from parent nodes with children
  // containers.
  llvm_unreachable("Vector graph node parent node has no children container.");
  return nullptr;
}
