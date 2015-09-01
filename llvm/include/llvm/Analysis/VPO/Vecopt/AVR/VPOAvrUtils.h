//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2015 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//   Source file:
//   ------------
//   VPOAvr.h -- Defines the utilities class for AVR nodes.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANAYSIS_VPO_AVR_UTILS_H
#define LLVM_ANAYSIS_VPO_AVR_UTILS_H

#include "llvm/Support/Compiler.h"
#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvr.h"
#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvrFunction.h"
#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvrLoop.h"
#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvrStmt.h"
#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvrIf.h"

namespace llvm { // LLVM Namespace

class LoopInfo;

namespace vpo {  // VPO Vectorizer Namespace

// Enumeration for types of AVR insertion
enum InsertType {FirstChild, LastChild, Append, Prepend};

typedef AVRContainerTy::iterator AvrItr;


/// \brief This class defines the utilies for AVR nodes.
///
/// It contains functions which are used to create, modify, and destroy
/// AVR nodes.
///
class AVRUtils {

private:

  // Internal implementations of utility helper functions, not meant
  // to be called externally.

  /// \brief Internal helper function for removing and deleting avrs
  /// and sequences of avrs.
  static AVRContainerTy *removeInternal(AVRContainerTy *Container,
                                        AvrItr Begin, AvrItr End,
                                        AVRContainerTy *MoveContainer, 
                                        bool Delete);

  static void insertAsChildInternal(AVRLoop *AvrLoop, AvrItr InsertionPos,
                                    AvrItr Begin, AvrItr End);


  static void insertAVRSeq(AVR *NewParent, AVRContainerTy &ToContainer,
                           AvrItr InsertionPos, AVRContainerTy *FromContainer,
                           AvrItr Begin, AvrItr End, InsertType Itype);

public:

  // Creation Utilities

  /// \brief Returns a new AVRFunction node.
  static AVRFunction *createAVRFunction(Function *OrigF, const LoopInfo *LpInfo);

  /// \brief Returns a new AVRFunction node.
  static AVRLoop *createAVRLoop(const Loop *Lp);

  /// \brief Returns a new AVRFunction node.
  static AVRLoop *createAVRLoop(WRNVecLoopNode *WrnSimdNode);

  /// \brief Returns a new AVRWrn node.
  static AVRWrn *createAVRWrn(WRNVecLoopNode *WrnSimdNode);

  /// \brief Returns a new AVRAssign node.
  static AVRAssign *createAVRAssign(Instruction *Inst);

  /// \brief Returns a new AVRLabel node.
  static AVRLabel *createAVRLabel(BasicBlock *Block);

  /// \brief Returns a new AVRPhi node.
  static AVRPhi *createAVRPhi(Instruction *Inst);

  /// \brief Returns a new AVRCall node.
  static AVRCall *createAVRCall(Instruction *Inst);

  /// \brief Returns a new AVRFBranch node.
  static AVRFBranch *createAVRFBranch(Instruction *Inst);

  /// \brief Returns a new AVRBackEdge node.
  static AVRBackEdge *createAVRBackEdge(Instruction *Inst);

  /// \brief Returns a new AVREntry node.
  static AVREntry *createAVREntry(Instruction *Inst);

  /// \brief Returns a new AVREntry node.
  static AVRReturn *createAVRReturn(Instruction *Inst);

  /// \brief Returns a new AVRLoop node.
  static AVRLoop *createAVRLoop();

  /// \brief Returns a new AVRIf node.
  static AVRIf *createAVRIf(Instruction *Inst);

  /// \brief Returns a new AVRExpr node.
  static AVRExpr *createAVRExpr();

  // Insertion Utilities

  /// \brief Standard Insert Utility
  static void insertAVR(AVR *Parent, AvrItr Postion, AvrItr NewAvr, InsertType Itype);

  /// \brief Inserts NewAvr node as the first child in Parent avr.
  static void insertFirstChildAVR(AVR *Parent, AvrItr NewAvr);

  /// \brief Inserts NewAvr node as the last child in Parent avr.
  static void insertLastChildAVR(AVR *Parent, AvrItr NewAvr);

  /// \brief Inserts an unlinked AVR node after InsertionPos in AVR list.
  static void insertAVRAfter(AvrItr InsertionPos, AVR *Node);

  /// \brief Inserts an unlinked AVR node before InsertionPos in AVR list.
  static void insertAVRBefore(AvrItr InsertionPos, AVR *Node);

  /// \brief Moves AVR from current location to after InsertionPos
  static void moveAfter(AvrItr InsertionPos, AVR *Node);

  /// \brief Unlinks [First, Last) from their current position and inserts them
  /// at the begining of the parent loop's children.
  static void moveAsFirstChildren(AVRLoop *ALoop, AvrItr First, AvrItr Last);

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
  static void remove(AVRContainerTy &AvrSequence)
  { remove(AvrSequence.begin(), AvrSequence.end()); }

  /// \brief Unlinks Avr node from avr list and destroys it.
  static void erase(AVR *Node);

  /// \brief Unlinks [First, Last) from AVR list and destroys them.
  static void erase(AVRContainerTy::iterator First,
                    AVRContainerTy::iterator Last);

  /// \brief Replaces OldNode by an unlinked NewNode.
  static void replace(AVR *OldNode, AVR *NewNode);

  // Search Utilities

  /// \brief Search Avr and its children and return AVRLabel corresponding
  /// to specified BB if found, nullptr if not found.
  static AVRLabel *getAvrLabelForBB(BasicBlock* BB, AVR* Avr);

  /// \brief Search Avr and its children and return AVRFbranch corresponding
  /// to specified Terminator if found, nullptr if not found.
  static AVRFBranch *getAvrBranchForTerm(Instruction* Terminator, AVR *Avr);

  /// \breif Retrun a pointer the AVR's children conatiner, nullptr otherwise.
  static AVRContainerTy *getAvrChildren(AVR *Avr);

};

} // End VPO Vectorizer Namespace
} // End LLVM Namespace

#endif // LLVM_ANAYSIS_VPO_AVR_UTILS_H


