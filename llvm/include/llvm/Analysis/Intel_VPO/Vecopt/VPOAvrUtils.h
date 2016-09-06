//===-- VPOAvrUtils.h -------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file defines the utilies class for AVR nodes.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANAYSIS_VPO_AVR_UTILS_H
#define LLVM_ANAYSIS_VPO_AVR_UTILS_H

#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvr.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrFunction.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrIf.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrLoop.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrStmt.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrSwitch.h"
#include "llvm/Support/Compiler.h"

namespace llvm { // LLVM Namespace

class LoopInfo;

namespace vpo { // VPO Vectorizer Namespace

// Avr Iterator Type
typedef AVRContainerTy::iterator AvrItr;

/// \brief This class defines the utilies for AVR nodes.
///
/// It contains functions which are used to create, modify, and destroy
/// AVR nodes.
///
class AVRUtils {

private:
  // Internal implementations of utility helper functions, not meant
  // to be called externally. The public interfaces of this class
  // are wrappers to these private utility helpers.

  /// \brief Internal helper function for removing and deleting avrs
  /// and sequences of avrs.
  static AVRContainerTy *removeInternal(AvrItr Begin, AvrItr End,
                                        AVRContainerTy *MoveContainer,
                                        bool Delete);

  /// \brief Internal function used to insert a single node into the
  /// vector graph. DestinationParent is the Parent node which contains
  /// DestinationContainer. DestinationContainer is the container that
  /// where NewNode is inserted into at InsertionPosition.
  static void insertSingleton(AVR *DestinationParent,
                              AVRContainerTy &DestinationContainer,
                              AvrItr InsertionPostition, AVR *NewNode);

  /// \brief Internal function used to insert a sequence of nodes into
  /// the vector graph. Destination Parent is the Parent node which
  /// contains DestinationContainer. DestinationContainer is the
  /// where the sequence of nodes in SourceContainer is inserted into
  /// at InsertionPosition.
  static void insertSequence(AVR *DestinationParent,
                             AVRContainerTy &DestinationContainer,
                             AVRContainerTy *SourceContainer,
                             AvrItr InsertionPosition);

  /// \brief Internal helper function to resolve mismatched lexical nesting
  /// levels of avr (First, Last) sequences.
  static bool resolveCommonLexicalParent(AVR *First, AVR **Last);

  /// \brief Internal helper function to obtain the container in Parent
  /// node which contains iterator Child.
  /// This utility has a costly search overhead, avoid if possible.
  //  TODO: Unify children containers.
  static AVRContainerTy *getChildrenContainer(AVR *Parent, AvrItr Child);

public:
  // Creation Utilities

  /// \brief Returns a new AVRFunction node.
  static AVRFunction *createAVRFunction(Function *OrigF,
                                        const LoopInfo *LpInfo);

  /// \brief Returns a new AVRLoop node.
  static AVRLoop *createAVRLoop(const Loop *Lp);

  /// \brief Returns a new AVRLoop node.
  static AVRLoop *createAVRLoop(WRNVecLoopNode *WrnSimdNode);

  /// \brief Returns a new AVRWrn node.
  static AVRWrn *createAVRWrn(WRNVecLoopNode *WrnSimdNode);

  /// \brief Returns a new AVRLoop node.
  static AVRLoop *createAVRLoop();

  /// \brief Returns a new AVRBranch node.
  static AVRBranch *createAVRBranch(AVRLabel *Sucessor);

  /// \brief Returns a new AVRNOP node.
  static AVRNOP *createAVRNOP();

  // Modification Utilities

  /// \brief Sets AvrAssign's LHS to Node.
  static void setAVRAssignLHS(AVRAssign *AvrAssign, AVR *Node);

  /// \brief Sets AvrAssign's RHS to Node.
  static void setAVRAssignRHS(AVRAssign *AvrAssign, AVR *Node);

  /// \brief Sets AvrPhi's LHS to Node.
  static void setAVRPhiLHS(AVRPhi *APhi, AVRValue *AValue);

  /// \brief Add an incoming AVRValue (from AVRLabel) to an AVRPhi.
  static void addAVRPhiIncoming(AVRPhi *APhi, AVRValue *AValue,
                                AVRLabel *ALabel);

  /// \brief Add a case to the ASwitch node
  static void addCase(AVRSwitch *ASwitch);

  /// \brief set Avr nodes's SLEV data.
  static void setSLEV(AVR *Avr, const SLEV& Slev);

  /// \brief Sets AvrLoop's zero trip test
  static void setZeroTripTest(AVRLoop *AvrLoop, AVRIf *IfZtt);

  // Insertion Utilities

  /// \brief Inserts an unlinked Node after InsertionPosition in vector graph.
  /// This interface contains a costly look-up for InsertionPosition's
  /// children container. Use alternative interface with InsertionPosition's
  /// children container passed as argurment, resort to this
  /// interface as a last option.
  static void insertAfter(AvrItr InsertionPosition, AVR *Node);

  /// \brief Inserts an unlinked Node before InsertionPosition in vector graph.
  /// This interface contains a costly look-up for InsertionPosition's
  /// children container. Use alternative interface with InsertionPosition's
  /// children container passed as argurment, resort to this
  /// interface as a last option.
  static void insertBefore(AvrItr InsertionPosition, AVR *Node);

  /// \brief Inserts unlinked nodes in NodeContainer after InsertionPosition
  /// in vector graph.
  /// The contents of NodeContainer will be empty after insertion.
  /// This interface contains a costly look-up for InsertionPosition's
  /// children container. Use alternative interface with InsertionPosition's
  /// children container passed as argurment, resort to this
  /// interface as a last option.
  static void insertAfter(AvrItr InsertionPos, AVRContainerTy *NodeContainer);

  /// \brief Inserts unlinked nodes in NodeContainer after InsertionPosition
  /// in vector graph.
  /// The contents of NodeContainer will be empty after insertion.
  /// This interface contains a costly look-up for InsertionPosition's
  /// children container. Use alternative interface with InsertionPosition's
  /// children container passed as argurment, resort to this
  /// interface as a last option.
  static void insertBefore(AvrItr InsertionPos, AVRContainerTy *NodeContainer);

  /// \brief Inserts an unlinked Node after InsertionPosition in the specified
  /// DestinationContainer.
  static void insertAfter(AvrItr InsertionPosition,
                          AVRContainerTy &DestinationContainer, AVR *Node);

  /// \brief Inserts an unlinked Node before InsertionPosition in the specified
  /// DestinationContainer.
  static void insertBefore(AvrItr InsertionPosition,
                           AVRContainerTy &DestinationContainer, AVR *Node);

  /// \brief Inserts unlinked nodes in NodeContainer after InsertionPosition
  /// in DestinationContainer.
  /// The contents of NodeContainer will be empty after insertion.
  static void insertAfter(AvrItr InsertionPosition,
                          AVRContainerTy &DestinationContainer,
                          AVRContainerTy *SourceContainer);

  /// \brief Inserts unlinked nodes in NodeContainer before InsertionPosition
  /// in DestinationContainer.
  /// The contents of NodeContainer will be empty after insertion.
  static void insertBefore(AvrItr InsertionPosition,
                           AVRContainerTy &DestinationContainer,
                           AVRContainerTy *SourceContainer);

  /// \brief Inserts Node as first child in (Parent) node's children.
  static void insertFirstChild(AVR *Parent, AVR *Node);

  /// \brief Inserts Node as last child in (Parent) node's children.
  static void insertLastChild(AVR *Parent, AVR *Node);

  /// \brief Inserts Node as first child in loop (Parent) node's children.
  static void insertFirstChild(AVRLoop *Parent, AVR *Node);

  /// \brief Inserts Node as last child in loop (Parent) node's children.
  static void insertLastChild(AVRLoop *Parent, AVR *Node);

  /// \brief Inserts unlinked Nodes in NodeContainer as first children of loop
  /// node, Parent. The order of NodeContainer is insertion order. The contents
  /// of NodeContainer will be empty after insertion.
  static void insertFirstChildren(AVRLoop *Parent,
                                  AVRContainerTy *NodeContainer);

  /// \brief Inserts unlinked Nodes in NodeContainer as last children of loop
  /// node, Parent. The order of NodeContainer is insertion order. The contents
  /// of NodeContainer will be empty after insertion.
  static void insertLastChildren(AVRLoop *Parent,
                                 AVRContainerTy *NodeContainer);

  /// \brief Inserts Node as first child in function (Parent) node's children.
  static void insertFirstChild(AVRFunction *Parent, AVR *Node);

  /// \brief Inserts Node as last child in function (Parent) node's children.
  static void insertLastChild(AVRFunction *Parent, AVR *NewNode);

  /// \brief Inserts unlinked Nodes in NodeContainer as first children of
  /// function node, Parent. The order of NodeContainer is insertion order.
  /// The contents of NodeContainer will be empty after insertion.
  static void insertFirstChildren(AVRFunction *Parent,
                                  AVRContainerTy *NodeContainer);

  /// \brief Inserts unlinked Nodes in NodeContainer as last children of
  /// function node, Parent. The order of NodeContainer is insertion order.
  /// The contents of NodeContainer will be empty after insertion.
  static void insertLastChildren(AVRFunction *Parent,
                                 AVRContainerTy *NodeContainer);

  /// \brief Inserts Node as first child in wrn (Parent) node's children.
  static void insertFirstChild(AVRWrn *Parent, AVR *Node);

  /// \brief Inserts Node as last child in wrn (Parent) node's children.
  static void insertLastChild(AVRWrn *Parent, AVR *Node);

  /// \brief Inserts unlinked Nodes in NodeContainer as first children of
  /// wrn node, Parent. The order of NodeContainer is insertion order.
  /// The contents of NodeContainer will be empty after insertion.
  static void insertFirstChildren(AVRWrn *Parent,
                                  AVRContainerTy *NodeContainer);

  /// \brief Inserts unlinked Nodes in NodeContainer as last children of
  /// wrn node, Parent. The order of NodeContainer is insertion order.
  /// The contents of NodeContainer will be empty after insertion.
  static void insertLastChildren(AVRWrn *Parent, AVRContainerTy *NodeContainer);

  /// \brief Inserts Node as first 'then' child in if (Parent) node's 'then'
  /// children.
  static void insertFirstThenChild(AVRIf *Parent, AVR *Node);

  /// \brief Inserts Node as last 'then' child in if (Parent) node's 'then'
  /// children.
  static void insertLastThenChild(AVRIf *Parent, AVR *Node);

  /// \brief Inserts unlinked Nodes in NodeContainer as first 'then' children of
  /// if node, Parent. The order of NodeContainer is insertion order.
  /// The contents of NodeContainer will be empty after insertion.
  static void insertFirstThenChildren(AVRIf *Parent,
                                      AVRContainerTy *NodeContainer);

  /// \brief Inserts unlinked Nodes in NodeContainer as last 'then' children of
  /// if node, Parent. The order of NodeContainer is insertion order.
  /// The contents of NodeContainer will be empty after insertion.
  static void insertLastThenChildren(AVRIf *Parent,
                                     AVRContainerTy *NodeContainer);

  /// \brief Inserts Node as first 'else' child in if (Parent) node's 'else'
  /// children.
  static void insertFirstElseChild(AVRIf *Parent, AVR *Node);

  /// \brief Inserts Node as last 'else' child in if (Parent) node's 'else'
  /// children.
  static void insertLastElseChild(AVRIf *Parent, AVR *Node);

  /// \brief Inserts unlinked Nodes in NodeContainer as first 'else' children of
  /// if node, Parent. The order of NodeContainer is insertion order.
  /// The contents of NodeContainer will be empty after insertion.
  static void insertFirstElseChildren(AVRIf *Parent,
                                      AVRContainerTy *NodeContainer);

  /// \brief Inserts unlinked Nodes in NodeContainer as last 'else' children of
  /// if node, Parent. The order of NodeContainer is insertion order.
  /// The contents of NodeContainer will be empty after insertion.
  static void insertLastElseChildren(AVRIf *Parent,
                                     AVRContainerTy *NodeContainer);

  /// \brief Inserts Node as the first child of switch (Parent) node's default
  /// case.
  static void insertFirstDefaultChild(AVRSwitch *Parent, AVR *Node);

  /// \brief Inserts Node as the last child of switch (Parent) node's default
  /// case.
  static void insertLastDefaultChild(AVRSwitch *Parent, AVR *Node);

  /// \brief Inserts unlinked Nodes in NodeContainer as first default children
  /// of switch node, Parent. The order of NodeContainer is insertion order.
  /// The contents of NodeContainer will be empty after insertion.
  static void insertFirstDefaultChildren(AVRSwitch *Parent,
                                         AVRContainerTy *NodeContainer);

  /// \brief Inserts unlinked Nodes in NodeContainer as last default children
  /// of switch node, Parent. The order of NodeContainer is insertion order.
  /// The contents of NodeContainer will be empty after insertion.
  static void insertLastDefaultChildren(AVRSwitch *Parent,
                                        AVRContainerTy *NodeContainer);

  /// \brief Inserts Node as the first child of switch (Parent) node's n-th
  /// case, where n-th is specified by CaseNum.
  static void insertFirstChild(AVRSwitch *Parent, AVR *Node, unsigned CaseNum);

  /// \brief Inserts Node as the last child of switch (Parent) node's n-th
  /// case, where n-th is specified by CaseNum.
  static void insertLastChild(AVRSwitch *Parent, AVR *Node, unsigned CaseNum);

  /// \brief Inserts unlinked Nodes in NodeContainer as first children of
  /// of switch (Parent) node's n-th case, where n-th is specified by CaseNum.
  /// The order of NodeContainer is insertion order.
  /// The contents of NodeContainer will be empty after insertion.
  static void insertFirstChildren(AVRSwitch *Parent,
                                  AVRContainerTy *NodeContainer,
                                  unsigned CaseNum);

  /// \brief Inserts unlinked Nodes in NodeContainer as last children of
  /// of switch (Parent) node's n-th case, where n-th is specified by CaseNum.
  /// The order of NodeContainer is insertion order.
  /// The contents of NodeContainer will be empty after insertion.
  static void insertLastChildren(AVRSwitch *Parent,
                                 AVRContainerTy *NodeContainer,
                                 unsigned CaseNum);

  /// \brief Inserts Node as the first child of loop (Parent) nodes's preheader
  /// children.
  static void insertFirstPreheaderChild(AVRLoop *Parent, AVR *Node);

  /// \brief Inserts Node as the last child of loop (Parent) nodes's preheader
  /// children.
  static void insertLastPreheaderChild(AVRLoop *Parent, AVR *Node);

  /// \brief Inserts unlinked Nodes in NodeContainer as first preheader children
  /// of loop node, Parent. The order of NodeContainer is insertion order.
  /// The contents of NodeContainer will be empty after insertion.
  static void insertFirstPreheaderChildren(AVRLoop *Parent,
                                           AVRContainerTy *NodeContainer);

  /// \brief Inserts unlinked Nodes in NodeContainer as last preheader children
  /// of loop node, Parent. The order of NodeContainer is insertion order.
  /// The contents of NodeContainer will be empty after insertion.
  static void insertLastPreheaderChildren(AVRLoop *Parent,
                                          AVRContainerTy *NodeContainer);

  /// \brief Inserts Node as the first child of loop (Parent) nodes's postexit
  /// children.
  static void insertFirstPostexitChild(AVRLoop *Parent, AVR *Node);

  /// \brief Inserts Node as the last child of loop (Parent) nodes's postexit
  /// children.
  static void insertLastPostexitChild(AVRLoop *Parent, AVR *Node);

  /// \brief Inserts unlinked Nodes in NodeContainer as first postexit children
  /// of loop node, Parent. The order of NodeContainer is insertion order.
  /// The contents of NodeContainer will be empty after insertion.
  static void insertFirstPostexitChildren(AVRLoop *Parent,
                                          AVRContainerTy *NodeContainer);

  /// \brief Inserts unlinked Nodes in NodeContainer as last postexit children
  /// of loop node, Parent. The order of NodeContainer is insertion order.
  /// The contents of NodeContainer will be empty after insertion.
  static void insertLastPostexitChildren(AVRLoop *Parent,
                                         AVRContainerTy *NodeContainer);

  // Move Utilities

  /// \brief Moves AVR from current location to after InsertionPos.
  static void moveAfter(AvrItr InsertionPos, AVR *Node);

  /// \brief Unlinks [First, Last] from their current position and inserts them
  /// at the begining of the parent loop's children.
  static void moveAsFirstChildren(AVRLoop *ALoop, AvrItr First, AvrItr Last);

  /// \brief Unlinks [First, Last] from their current position and inserts them
  /// at the beginning of ASwitch's CaseNum-nth case.
  static void moveAsFirstChildren(AVRSwitch *ASwitch, AvrItr First, AvrItr Last,
                                  unsigned CaseNum);

  /// \brief Unlinks [First, Last] from their current position and inserts them
  /// at the end of the parent loop's children.
  static void moveAsLastChildren(AVRLoop *ALoop, AvrItr First, AvrItr Last);

  /// \brief Unlinks [First, Last] from their current position and inserts them
  /// at the end of ASwitch's CaseNum-nth case.
  static void moveAsLastChildren(AVRSwitch *ASwitch, AvrItr First, AvrItr Last,
                                 unsigned CaseNum);

  /// \brief Unlinks Node from its current location and inserts it as the
  /// first 'Then' child of AvrIf.
  static void moveAsFirstThenChild(AVRIf *AvrIf, AVR *Node);

  /// \brief Unlinks Node from its current location and inserts it as the
  /// last 'Then' child of AvrIf.
  static void moveAsLastThenChild(AVRIf *AvrIf, AVR *Node);

  /// \brief Unlinks Node from its current location and inserts it as the
  /// first 'Else' child of AvrIf.
  static void moveAsFirstElseChild(AVRIf *AvrIf, AVR *Node);

  /// \brief Unlinks Node from its current location and inserts it as the
  /// last 'Else' child of AvrIf.
  static void moveAsLastElseChild(AVRIf *AvrIf, AVR *Node);

  /// \brief Unlinks [First, Last] from their current location and inserts them
  /// at the beginning of 'Then' children of AvrIf.
  static void moveAsFirstThenChildren(AVRIf *AIf, AvrItr First, AvrItr Last);

  /// \brief Unlinks [First, Last] from their current location and inserts them
  /// at the beginning of 'Else' children of AvrIf.
  static void moveAsFirstElseChildren(AVRIf *AIf, AvrItr First, AvrItr Last);

  /// \brief Unlinks [First, Last] from their current location and inserts them
  /// at the beginning of ASwitch's default case.
  static void moveAsFirstDefaultChildren(AVRSwitch *ASwitch, AvrItr First,
                                         AvrItr Last);

  /// \brief Unlinks [First, Last] from their current location and inserts them
  /// at the end of ASwitch's default case.
  static void moveAsLastDefaultChildren(AVRSwitch *ASwitch, AvrItr First,
                                        AvrItr Last);

  /// \brief Unlinks [First, Last] from their current location and inserts them
  /// at the beginning of ALoop's preheader.
  static void moveAsFirstPreheaderChildren(AVRLoop *ALoop, AvrItr First,
                                           AvrItr Last);

  /// \brief Unlinks [First, Last] from their current location and inserts them
  /// at the beginning of ALoop's postexit.
  static void moveAsFirstPostexitChildren(AVRLoop *ALoop, AvrItr First,
                                          AvrItr Last);

  /// \brief Unlinks [First, Last] from their current location and inserts them
  /// at the end of ALoop's preheader.
  static void moveAsLastPreheaderChildren(AVRLoop *ALoop, AvrItr First,
                                          AvrItr Last);

  /// \brief Unlinks [First, Last] from their current location and inserts them
  /// at the end of ALoop's preheader.
  static void moveAsLastPostexitChildren(AVRLoop *ALoop, AvrItr First,
                                         AvrItr Last);

  // Removal Utilities

  /// \brief Destroys the passed in AVR node.
  static void destroy(AVR *Node);

  /// \brief Unlinks AVR node from avr list.
  static void remove(AVR *Node);

  /// \brief Unlinks AVR nodes from Begin to End from the avr list.
  /// Returns a pointer to an AVRContainer of the removed sequence
  static void remove(AvrItr Begin, AvrItr End);

  /// \brief Unlinks the AVR nodes which are inside AvrSequence from
  /// thier parent AVRnode container. Returns a pointer to removed container.
  static void remove(AVRContainerTy &AvrSequence) {
    remove(AvrSequence.begin(), AvrSequence.end());
  }

  /// \brief Unlinks Avr node from avr list and destroys it.
  static void erase(AVR *Node);

  /// \brief Unlinks [First, Last) from AVR list and destroys them.
  static void erase(AVRContainerTy::iterator First,
                    AVRContainerTy::iterator Last);

  /// \brief Replaces OldNode by an unlinked NewNode.
  static void replace(AVR *OldNode, AVR *NewNode);

  // Search Utilities

  /// \brief Returns true if the given Container contains Node as immediate
  /// child.
  /// Uses non-recursive search.
  static bool containsAvr(AVRContainerTy &Container, AVR *Node);

  /// \brief Retrun a pointer the AVRContainer which contains Avr.
  static AVRContainerTy *getAvrContainer(AVR *Avr);
};

} // End VPO Vectorizer Namespace
} // End LLVM Namespace

#endif // LLVM_ANAYSIS_VPO_AVR_UTILS_H
