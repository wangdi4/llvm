//===-------- VPOAvrUtils.h - Utilities for Avr class -----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the utilities for AVR class.
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


namespace intel { // VPO Vectorizer Namespace

// Enumeration for types of AVR insertion
  enum InsertType {FirstChild, LastChild, Append, Prepend};

/// \brief This class defines the utilies for AVR nodes.
///
/// It contains functions which are used to create, modify, and destroy
/// AVR nodes.
///
class AVRUtils {

public:

  typedef AVRContainerTy::iterator AvrItr;

  // Creation Utilities

  /// \brief Returns a new AVRFunction node.
  static AVRFunction *createAVRFunction(Function *OrigF);
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
  // To Do: Define More Utilities

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

  // Removal Utilities

  /// \brief Destroys the passed in AVR node.
  static void destroy(AVR *Node);

  /// \brief Unlinks AVR node from avr list.
  static void remove(AVR *Node);

  /// \brief Unlinks Avr node from avr list and destroys it.
  static void erase(AVR *Node);

  /// \brief Unlinks [First, Last) from AVR list and destroys them.
  static void erase(AVRContainerTy::iterator First,
                    AVRContainerTy::iterator Last);

  /// \brief Replaces OldNode by an unlinked NewNode.
  static void replace(AVR *OldNode, AVR *NewNode);

};

} // End VPO Vectorizer Namespace

#endif // LLVM_ANAYSIS_VPO_AVR_UTILS_H


