//===--- VectorGraphUtils.h -------------------------------------*- C++ -*-===//
//===----------------------------------------------------------------------===//
#ifndef LLVM_ANAYSIS_VECTOR_GRAPH_UTILS_H
#define LLVM_ANAYSIS_VECTOR_GRAPH_UTILS_H

#include "llvm/Analysis/Intel_VPO/Vecopt/VectorGraph.h"
#include "llvm/Support/Compiler.h"

namespace llvm { // LLVM Namespace

class LoopInfo;

namespace vpo { // VPO Vectorizer Namespace

// Vector Graph Iterator Type
typedef VectorGraphTy::iterator VGItr;

/// \brief This class defines the utilies for vector graph nodes.
class VectorGraphUtils {

private:
  // Internal implementations of utility helper functions, not meant
  // to be called externally. The public interfaces of this class
  // are wrappers to these private utility helpers.

  /// \brief Internal helper function for removing and deleting avrs
  /// and sequences of avrs.
  static VectorGraphTy *removeInternal(VGItr Begin, VGItr End,
                                       VectorGraphTy *MoveContainer,
                                       bool Delete);

  /// \brief Internal function used to insert a single node into the
  /// vector graph. DestinationParent is the Parent node which contains
  /// DestinationContainer. DestinationContainer is the container that
  /// where NewNode is inserted into at InsertionPosition.
  static void insertSingleton(VGNode *DestinationParent,
                              VectorGraphTy &DestinationContainer,
                              VGItr InsertionPostition, VGNode *NewNode);

  /// \brief Internal function used to insert a sequence of nodes into
  /// the vector graph. Destination Parent is the Parent node which
  /// contains DestinationContainer. DestinationContainer is the
  /// where the sequence of nodes in SourceContainer is inserted into
  /// at InsertionPosition.
  static void insertSequence(VGNode *DestinationParent,
                             VectorGraphTy &DestinationContainer,
                             VectorGraphTy *SourceContainer,
                             VGItr InsertionPosition);

  static VectorGraphTy *getChildrenContainer(VGNode *Parent);
public:
  // Creation Utilities

  /// \brief Returns a new VGLoop node.
  static VGLoop *createVGLoop(Loop *Lp);

  /// \brief Returns a new AVRBlock node.
  static VGBlock *createVGBlock(BasicBlock *BB);

  /// \brief Returns a new AVRBlock node.
  static VGPredicate *createVGPredicate();

  // Modification Utilities

  /// \brief set Avr nodes's predicate.
  static void setPredicate(VGNode *Node, VGPredicate* Predicate);

  /// \brief Add an incoming AVRValue (from AVRLabel) to an AVRPhi.
  static void addVGPredicateIncoming(VGPredicate *APredicate,
                                     VGPredicate *IncomingPredicate,
                                     Value *IncomingCondition);


  static void setBlockCondition(VGBlock* ABlock, Value *C);

  static void addSuccessor(VGBlock* Block, VGBlock* Successor);

  static void addSchedulingConstraint(VGBlock* Block, VGBlock* Constraint);

  // Insertion Utilities

  /// \brief Inserts an unlinked Node after InsertionPosition in vector graph.
  static void insertAfter(VGItr InsertionPosition, VGNode *Node);

  /// \brief Inserts an unlinked Node before InsertionPosition in vector graph.
  static void insertBefore(VGItr InsertionPosition, VGNode *Node);

  /// \brief Inserts unlinked nodes in NodeContainer after InsertionPosition
  /// in vector graph.
  static void insertAfter(VGItr InsertionPos, VectorGraphTy *NodeContainer);

  /// \brief Inserts unlinked nodes in NodeContainer after InsertionPosition
  /// in vector graph.
  static void insertBefore(VGItr InsertionPos, VectorGraphTy *NodeContainer);

  /// \brief Inserts an unlinked Node after InsertionPosition in the specified
  /// DestinationContainer.
  static void insertAfter(VGItr InsertionPosition,
                          VectorGraphTy &DestinationContainer, VGNode *Node);

  /// \brief Inserts an unlinked Node before InsertionPosition in the specified
  /// DestinationContainer.
  static void insertBefore(VGItr InsertionPosition,
                           VectorGraphTy &DestinationContainer, VGNode *Node);

  /// \brief Inserts unlinked nodes in NodeContainer after InsertionPosition
  /// in DestinationContainer.
  /// The contents of NodeContainer will be empty after insertion.
  static void insertAfter(VGItr InsertionPosition,
                          VectorGraphTy &DestinationContainer,
                          VectorGraphTy *SourceContainer);

  /// \brief Inserts unlinked nodes in NodeContainer before InsertionPosition
  /// in DestinationContainer.
  /// The contents of NodeContainer will be empty after insertion.
  static void insertBefore(VGItr InsertionPosition,
                           VectorGraphTy &DestinationContainer,
                           VectorGraphTy *SourceContainer);

  /// \brief Inserts Node as first child in (Parent) node's children.
  static void insertFirstChild(VGNode *Parent, VGNode *Node);

  /// \brief Inserts Node as last child in (Parent) node's children.
  static void insertLastChild(VGNode *Parent, VGNode *Node);

  /// \brief Inserts Node as first child in loop (Parent) node's children.
  static void insertFirstChild(VGLoop *Parent, VGNode *Node);

  /// \brief Inserts Node as last child in loop (Parent) node's children.
  static void insertLastChild(VGLoop *Parent, VGNode *Node);

  /// \brief Inserts unlinked Nodes in NodeContainer as first children of loop
  /// node, Parent. The order of NodeContainer is insertion order. The contents
  /// of NodeContainer will be empty after insertion.
  static void insertFirstChildren(VGLoop *Parent,
                                  VectorGraphTy *NodeContainer);

  /// \brief Inserts unlinked Nodes in NodeContainer as last children of loop
  /// node, Parent. The order of NodeContainer is insertion order. The contents
  /// of NodeContainer will be empty after insertion.
  static void insertLastChildren(VGLoop *Parent,
                                 VectorGraphTy *NodeContainer);

  /// \brief Inserts Node as first child in block (Parent) node's children.
  //static void insertFirstChild(VGBlock *Parent, VGNode *Node);

  /// \brief Inserts Node as last child in block (Parent) node's children.
  //static void insertLastChild(VGBlock *Parent, VGNode *Node);

  /// \brief Inserts unlinked Nodes in NodeContainer as first children of
  /// block node, Parent. The order of NodeContainer is insertion order.
  /// The contents of NodeContainer will be empty after insertion.
  //static void insertFirstChildren(VGBlock *Parent,
  //                                VectorGraphTy *NodeContainer);

  /// \brief Inserts unlinked Nodes in NodeContainer as last children of
  /// block node, Parent. The order of NodeContainer is insertion order.
  /// The contents of NodeContainer will be empty after insertion.
  //static void insertLastChildren(VGBlock *Parent, VectorGraphTy *NodeContainer);

  // Move Utilities

  /// \brief Moves VGNode from current location to after InsertionPos.
  static void moveAfter(VGItr InsertionPos, VGNode *Node);

  /// \brief Unlinks [First, Last] from their current position and inserts them
  /// at the beginning of the parent loop's children.
  static void moveAsFirstChildren(VGLoop *ALoop, VGItr First, VGItr Last);

  /// \brief Unlinks [First, Last] from their current position and inserts them
  /// at the end of the parent loop's children.
  static void moveAsLastChildren(VGLoop *ALoop, VGItr First, VGItr Last);

  /// \brief Unlinks [First, Last] from its current location and inserts them
  /// at the beginning of the block's children.
  //static void moveAsFirstChildren(VGBlock *ABlock, VGItr First, VGItr Last);

  /// \brief Unlinks [First, Last] from its current location and inserts them
  /// at the end of the block's children.
  //static void moveAsLastChildren(VGBlock *ABlock, VGItr First, VGItr Last);

  // Removal Utilities

  /// \brief Destroys the passed in VGNode node.
  static void destroy(VGNode *Node);

  /// \brief Unlinks VGNode node from avr list.
  static void remove(VGNode *Node);

  /// \brief Unlinks VGNode nodes from Begin to End from the avr list.
  /// Returns a pointer to an AVRContainer of the removed sequence
  static void remove(VGItr Begin, VGItr End);

  /// \brief Unlinks the VGNode nodes which are inside AvrSequence from
  /// thier parent AVRnode container. Returns a pointer to removed container.
  static void remove(VectorGraphTy &VGSequence) {
    remove(VGSequence.begin(), VGSequence.end());
  }

  /// \brief Unlinks Avr node from avr list and destroys it.
  static void erase(VGNode *Node);

  /// \brief Unlinks [First, Last) from VGNode list and destroys them.
  static void erase(VGItr First, VGItr Last);

  /// \brief Replaces OldNode by an unlinked NewNode.
  static void replace(VGNode *OldNode, VGNode *NewNode);

};

} // End VPO Vectorizer Namespace
} // End LLVM Namespace

#endif // LLVM_ANAYSIS_VECTOR_GRAPH_UTILS_H
