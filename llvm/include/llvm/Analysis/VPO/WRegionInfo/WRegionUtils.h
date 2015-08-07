//===----- WRegionNodeUtils.h - Utilities for WRegionNodeNode class -----*- C++ -*-===//
//
//   Copyright (C) 2015 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
// This file defines the utilities for W-Region Node class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANAYSIS_VPO_WREGIONUTILS_H
#define LLVM_ANAYSIS_VPO_WREGIONUTILS_H

#include "llvm/Support/Compiler.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionNode.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegion.h"

namespace llvm  { 

namespace vpo { 

/// \brief Enumeration for types of WRegionNode Graph Insert/Remove/Update 
enum OpType {
  FirstChild, 
  LastChild, 
  Append, 
  Prepend
};

/// \brief This class defines the utilies for WRegionNode nodes.
class WRegionUtils {

  typedef WRContainerTy::iterator WrnIter;

private:
  /// \brief Do not allow instantiation.
  //WRegionNodeUtils() LLVM_DELETED_FUNCTION;

  /// \brief Destroys all HLNodes, called during framework cleanup.
  static void destroyAll();

public:

  friend class WRegionNode;

  /// It contains functions which are used to create, modify, and destroy
  /// WRegionNode.

  /// Insertion Utilities -- To Do: Define More Utilities

  /// \brief Standard Insert Utility
  static void insertWRegionNode(WRegionNode *Parent, 
                                WrnIter Pos, WrnIter W, OpType Op);

  /// \brief Inserts new wrn as the first child in Parent wrn.
  static void insertFirstChild(WRegionNode *Parent, WrnIter W);

  /// \brief Inserts new wrn as the last child in Parent wrn.
  static void insertLastChild(WRegionNode *Parent, WrnIter W);

  /// \brief Inserts an unlinked WRegion Node after pos in WRegion Node list.
  static void insertAfter(WrnIter Pos, WRegionNode *W);

  /// \brief Inserts an unlinked WRegion Node before pos in WRegion Node list.
  static void insertBefore(WrnIter Pos, WRegionNode *W);

  /// Creation Utilities

  /// \brief Returns a new node derived from WRegionNode node that
  /// matches the construct type based on DirString. 
  //  (eg create a WRNParRegion node if DirString is "dir.parallel")
  static WRegionNode *createWRegion(StringRef DirString, BasicBlock *EntryBB);

  /// Utilities to handle directives & clauses 

  static bool isEndDirective(StringRef DirString);
  static void handleDirQual(IntrinsicInst *Intrin, WRegionNode *W);
  static void handleDirQualOpnd(IntrinsicInst *Intrin, WRegionNode *W);
  static void handleDirQualOpndList(IntrinsicInst *Intrin, WRegionNode *W);

  /// Removal Utilities

  /// \brief Destroys the passed in WRegion node.
  static void destroy(WRegionNode *wrn);

  /// \brief Unlinks WRegion node from avr list.
  static void remove(WRegionNode *wrn);

  /// \brief Unlinks wrn node from wrn list and destroys it.
  static void erase(WRegionNode *wrn);

  /// \brief Unlinks [First, Last) from WRegionNode list and destroys them.
  static void erase(WrnIter First, WrnIter Last);

  /// \brief Replaces OldNode by an unlinked NewNode.
  static void replace(WRegionNode *OldW, WRegionNode *NewW);
};

} // End VPO Namespace

} // End LLVM Namespace

#endif // LLVM_ANAYSIS_VPO_WREGIONUTILS_H
