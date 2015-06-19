//===-------- HLNodeUtils.h - Utilities for HLNode class --------*- C++ -*-===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file defines the utilities for HLNode class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_HLNODEUTILS_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_HLNODEUTILS_H

#include <set>
#include "llvm/Support/Compiler.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeVisitor.h"

namespace llvm {

class BasicBlock;
class Instruction;

namespace loopopt {

class HIRCreation;
class HIRParser;

/// \brief Defines utilities for HLNode class
///
/// It contains a bunch of static member functions which manipulate HLNodes.
/// It does not store any state.
///
class HLNodeUtils : public HLUtils {
private:
  /// \brief Do not allow instantiation.
  HLNodeUtils() = delete;

  friend class HIRCreation;
  friend class HIRCleanup;

  /// \brief Visitor for clone sequence
  struct CloneVisitor;

  struct LoopFinderUpdater;
  struct TopSorter;

  /// \brief Implementation of cloneSequence() which clones from Node1
  /// to Node2 and inserts into the CloneContainer.
  static void cloneSequenceImpl(HLContainerTy *CloneContainer,
                                const HLNode *Node1, const HLNode *Node2);

  /// \brief Destroys all HLNodes, called during framework cleanup.
  static void destroyAll();

  /// \brief Returns successor of Node assuming control flows in strict lexical
  /// order (by ignoring jumps(gotos)).
  /// This should only be called from HIRCleanup pass.
  static HLNode *getLexicalControlFlowSuccessor(HLNode *Node);

  /// Internal helper functions, not to be called directly.

  /// \brief Implements insert(before) functionality. Moves [First, last) from
  /// RemoveContainer to Parent's container. If RemoveContainer is null it
  /// assumes a range of 1(node). UpdateSeparator indicates whether separators
  /// used in containers should be updated. Additional arguments for updating
  /// postexit separator and switch's case number is required.
  static void insertImpl(HLNode *Parent, HLContainerTy::iterator Pos,
                         HLContainerTy *RemoveContainer,
                         HLContainerTy::iterator First,
                         HLContainerTy::iterator Last, bool UpdateSeparator,
                         bool PostExitSeparator = false, int CaseNum = -1);

  /// \brief Moves [First, last) from RemoveContainer to InsertContainer.
  /// If RemoveContainer is null it assumes a range of 1(node) and inserts
  /// First into InsertContainer..
  static void insertInternal(HLContainerTy &InsertContainer,
                             HLContainerTy::iterator Pos,
                             HLContainerTy *RemoveContainer,
                             HLContainerTy::iterator First,
                             HLContainerTy::iterator Last);

  /// \brief Updates nesting level and innermost flag for Loop.
  static void updateLoopInfo(HLLoop *Loop);

  /// \brief Helper function for recursively updating loop info for loops in
  /// [First, Last). This is called during insertion.
  static void updateLoopInfoRecursively(HLContainerTy::iterator First,
                                        HLContainerTy::iterator Last);

  /// \brief Implements insertAs*Child() functionality.
  static void insertAsChildImpl(HLNode *Parent, HLContainerTy *RemoveContainer,
                                HLContainerTy::iterator First,
                                HLContainerTy::iterator Last,
                                bool IsFirstChild);

  /// \brief Implements insertAs*Child() functionality for switch.
  static void insertAsChildImpl(HLSwitch *Switch,
                                HLContainerTy *RemoveContainer,
                                HLContainerTy::iterator First,
                                HLContainerTy::iterator Last, unsigned CaseNum,
                                bool isFirstChild);

  /// \brief Implements insertAs*Preheader*()/insertAs*Postexit*()
  /// functionality.
  static void insertAsPreheaderPostexitImpl(HLLoop *Loop,
                                            HLContainerTy *RemoveContainer,
                                            HLContainerTy::iterator First,
                                            HLContainerTy::iterator Last,
                                            bool IsPreheader,
                                            bool IsFirstChild);

  /// \brief Implements remove functionality. Removes [First, last) and destroys
  /// them if Erase is set. If erase isn't set and MoveContainer isn't null they
  /// are moved to MoveContainer. Otherwise, nodes are removed without
  /// destroying them.
  static void removeImpl(HLContainerTy::iterator First,
                         HLContainerTy::iterator Last,
                         HLContainerTy *MoveContainer, bool Erase = false);

  /// \brief Removes [First, Last) from Container. Also destroys them is Erase
  /// is set.
  static void removeInternal(HLContainerTy &Container,
                             HLContainerTy::iterator First,
                             HLContainerTy::iterator Last, bool Erase);

  /// \brief Returns true if a loop is found in range [First, Last).
  static bool foundLoopInRange(HLContainerTy::iterator First,
                               HLContainerTy::iterator Last);

  /// \brief Update the goto branches with new labels.
  static void updateGotos(GotoContainerTy *GotoList, LabelMapTy *LabelMap);

public:
  /// \brief Returns a new HLRegion.
  static HLRegion *createHLRegion(IRRegion *IRReg);

  /// \brief Returns a new HLSwitch.
  static HLSwitch *createHLSwitch(RegDDRef *ConditionRef);

  /// \brief Returns a new HLLabel.
  static HLLabel *createHLLabel(BasicBlock *SrcBB);

  /// \brief Returns a new HLGoto.
  static HLGoto *createHLGoto(BasicBlock *TargetBB, HLLabel *TargetL = nullptr);

  /// \brief Returns a new HLInst.
  static HLInst *createHLInst(Instruction *In);

  /// \brief Returns a new HLIf.
  static HLIf *createHLIf(CmpInst::Predicate FirstPred, RegDDRef *Ref1,
                          RegDDRef *Ref2);

  /// \brief Returns a new HLLoop created from an underlying LLVM loop.
  static HLLoop *createHLLoop(const Loop *LLVMLoop, bool IsDoWh = false);

  /// \brief Returns a new HLLoop.
  static HLLoop *createHLLoop(HLIf *ZttIf = nullptr,
                              RegDDRef *LowerDDRef = nullptr,
                              RegDDRef *UpperDDRef = nullptr,
                              RegDDRef *StrideDDRef = nullptr,
                              bool IsDoWh = false, unsigned NumEx = 1);

  /// \brief Creates a clones sequence from Node1 to Node2, including both
  /// the nodes and all the nodes in between them. If Node2 is null or Node1
  /// equals Node2, then the utility just clones Node1 and inserts into the
  /// CloneContainer. This utility does not support Region cloning.
  static void cloneSequence(HLContainerTy *CloneContainer, const HLNode *Node1,
                            const HLNode *Node2 = nullptr);

  /// \brief Destroys the passed in HLNode.
  static void destroy(HLNode *Node);

  /// \brief Visits the passed in HLNode.
  template <typename HV>
  static void visit(HV *Visitor, HLNode *Node, bool Recursive = true,
                    bool Forward = true) {
    HLNodeVisitor<HV> V(Visitor);
    V.visit(Node, Recursive, Forward);
  }

  /// \brief Visits HLNodes in the range [begin, end). The direction is
  /// specified using Forward flag.
  template <typename HV>
  static void visit(HV *Visitor, HLContainerTy::iterator Begin,
                    HLContainerTy::iterator End, bool Recursive = true,
                    bool Forward = true) {
    HLNodeVisitor<HV> V(Visitor);

    if (Forward) {
      V.forwardVisit(Begin, End, Recursive);
    } else {
      V.backwardVisit(Begin, End, Recursive);
    }
  }

  /// \brief Visits all HLNodes in the HIR. The direction is specified using
  /// Forward flag.
  template <typename HV>
  static void visitAll(HV *Visitor, bool Forward = true) {
    HLNodeVisitor<HV> V(Visitor);

    if (Forward) {
      V.forwardVisitAll(getHIRParserPtr());
    } else {
      V.backwardVisitAll(getHIRParserPtr());
    }
  }

  /// \brief Inserts an unlinked Node before Pos in HIR.
  static void insertBefore(HLNode *Pos, HLNode *Node);
  /// \brief Inserts unlinked Nodes in NodeContainer before Pos in HIR.
  /// The contents of NodeContainer will be empty after insertion.
  static void insertBefore(HLNode *Pos, HLContainerTy *NodeContainer);
  /// \brief Inserts an unlinked Node after Pos in HIR.
  static void insertAfter(HLNode *Pos, HLNode *Node);
  /// \brief Inserts an unlinked Nodes in NodeContainer after Pos in HIR.
  /// The contents of NodeContainer will be empty after insertion.
  static void insertAfter(HLNode *Pos, HLContainerTy *NodeContainer);

  /// \brief Inserts an unlinked Node as first child of parent region.
  static void insertAsFirstChild(HLRegion *Reg, HLNode *Node);
  /// \brief Inserts an unlinked Node as last child of parent region.
  static void insertAsLastChild(HLRegion *Reg, HLNode *Node);

  /// \brief Inserts an unlinked Node as first child of parent loop.
  static void insertAsFirstChild(HLLoop *Loop, HLNode *Node);
  /// \brief Inserts an unlinked Node as last child of parent loop.
  static void insertAsLastChild(HLLoop *Loop, HLNode *Node);

  /// \brief Inserts an unlinked Node as first child of this If. The flag
  /// IsThenChild indicates whether this is to be inserted as then or else
  /// child.
  static void insertAsFirstChild(HLIf *If, HLNode *Node, bool IsThenChild);
  /// \brief Inserts an unlinked Node as last child of this If. The flag
  /// IsThenChild indicates whether this is to be inserted as then or else
  /// child.
  static void insertAsLastChild(HLIf *If, HLNode *Node, bool IsThenChild);

  /// \brief Inserts an unlinked Node as first default case child of switch.
  static void insertAsFirstDefaultChild(HLSwitch *Switch, HLNode *Node);
  /// \brief Inserts an unlinked Node as last default case child of switch.
  static void insertAsLastDefaultChild(HLSwitch *Switch, HLNode *Node);

  /// \brief Inserts an unlinked Node as first CaseNum case child of switch.
  /// Range of CaseNum is [1, getNumCases()].
  static void insertAsFirstChild(HLSwitch *Switch, HLNode *Node,
                                 unsigned CaseNum);
  /// \brief Inserts an unlinked Node as last CaseNum case child of switch.
  /// Range of CaseNum is [1, getNumCases()].
  static void insertAsLastChild(HLSwitch *Switch, HLNode *Node,
                                unsigned CaseNum);

  /// \brief Inserts an unlinked Node as first preheader node of Loop.
  static void insertAsFirstPreheaderNode(HLLoop *Loop, HLNode *Node);
  /// \brief Inserts an unlinked Node as last preheader node of Loop.
  static void insertAsLastPreheaderNode(HLLoop *Loop, HLNode *Node);

  /// \brief Inserts an unlinked Node as first postexit node of Loop.
  static void insertAsFirstPostexitNode(HLLoop *Loop, HLNode *Node);
  /// \brief Inserts an unlinked Node as last postexit node of Loop.
  static void insertAsLastPostexitNode(HLLoop *Loop, HLNode *Node);

  /// \brief Unlinks Node from its current position and inserts it before Pos
  /// in HIR.
  static void moveBefore(HLNode *Pos, HLNode *Node);
  /// \brief Unlinks Node from its current position and inserts it after Pos
  /// in HIR.
  static void moveAfter(HLNode *Pos, HLNode *Node);

  /// \brief Unlinks Node from its current position and inserts as first child
  /// of parent region.
  static void moveAsFirstChild(HLRegion *Reg, HLNode *Node);
  /// \brief Unlinks Node from its current position and inserts as last child
  /// of parent region.
  static void moveAsLastChild(HLRegion *Reg, HLNode *Node);

  /// \brief Unlinks Node from its current position and inserts as first child
  /// of parent loop.
  static void moveAsFirstChild(HLLoop *Loop, HLNode *Node);
  /// \brief Unlinks Node from its current position and inserts as last child
  /// of parent loop.
  static void moveAsLastChild(HLLoop *Loop, HLNode *Node);

  /// \brief Unlinks Node from its current position and inserts as first child
  /// of this If. The flag IsThenChild indicates whether this is to be moved
  /// as then or else child.
  static void moveAsFirstChild(HLIf *If, HLNode *Node, bool IsThenChild = true);
  /// \brief Unlinks Node from its current position and inserts as last child
  /// of this If. The flag IsThenChild indicates whether this is to be moved
  /// as then or else child.
  static void moveAsLastChild(HLIf *If, HLNode *Node, bool IsThenChild = true);

  /// \brief Unlinks Node from its current position and inserts as first default
  /// case child of switch.
  static void moveAsFirstDefaultChild(HLSwitch *Switch, HLNode *Node);
  /// \brief Unlinks Node from its current position and inserts as last default
  /// case child of switch.
  static void moveAsLastDefaultChild(HLSwitch *Switch, HLNode *Node);

  /// \brief Unlinks Node from its current position and inserts as first CaseNum
  /// case child of switch.
  /// Range of CaseNum is [1, getNumCases()].
  static void moveAsFirstChild(HLSwitch *Switch, HLNode *Node,
                               unsigned CaseNum);
  /// \brief Unlinks Node from its current position and inserts as last CaseNum
  /// case child of switch.
  /// Range of CaseNum is [1, getNumCases()].
  static void moveAsLastChild(HLSwitch *Switch, HLNode *Node, unsigned CaseNum);

  /// \brief Unlinks Node from its current position and inserts as first
  /// preheader node of Loop.
  static void moveAsFirstPreheaderNode(HLLoop *Loop, HLNode *Node);
  /// \brief Unlinks Node from its current position and inserts an last
  /// preheader node of Loop.
  static void moveAsLastPreheaderNode(HLLoop *Loop, HLNode *Node);

  /// \brief Unlinks Node from its current position and inserts as first
  /// postexit node of Loop.
  static void moveAsFirstPostexitNode(HLLoop *Loop, HLNode *Node);
  /// \brief Unlinks Node from its current position and inserts as last
  /// postexit node of Loop.
  static void moveAsLastPostexitNode(HLLoop *Loop, HLNode *Node);

  /// \brief Unlinks [First, Last) from their current position and inserts them
  /// before Pos.
  static void moveBefore(HLNode *Pos, HLContainerTy::iterator First,
                         HLContainerTy::iterator Last);
  /// \brief Unlinks [First, Last) from their current position and inserts them
  /// after Pos.
  static void moveAfter(HLNode *Pos, HLContainerTy::iterator First,
                        HLContainerTy::iterator Last);

  /// \brief Unlinks [First, Last) from their current position and inserts them
  /// at the begining of the parent region's children.
  static void moveAsFirstChildren(HLRegion *Reg, HLContainerTy::iterator First,
                                  HLContainerTy::iterator Last);
  /// \brief Unlinks [First, Last) from their current position and inserts them
  /// at the end of the parent region's children.
  static void moveAsLastChildren(HLRegion *Reg, HLContainerTy::iterator First,
                                 HLContainerTy::iterator Last);

  /// \brief Unlinks [First, Last) from their current position and inserts them
  /// at the begining of the parent loop's children.
  static void moveAsFirstChildren(HLLoop *Loop, HLContainerTy::iterator First,
                                  HLContainerTy::iterator Last);
  /// \brief Unlinks [First, Last) from their current position and inserts them
  /// at the end of the parent loop's children.
  static void moveAsLastChildren(HLLoop *Loop, HLContainerTy::iterator First,
                                 HLContainerTy::iterator Last);

  /// \brief Unlinks [First, Last) from their current position and inserts them
  /// at the begining of this If. The flag IsThenChild indicates whether they
  /// are to be moved as then or else children.
  static void moveAsFirstChildren(HLIf *If, HLContainerTy::iterator First,
                                  HLContainerTy::iterator Last,
                                  bool IsThenChild = true);
  /// \brief Unlinks [First, Last) from their current position and inserts them
  /// at the end of this If. The flag IsThenChild indicates whether they are to
  /// be moved as then or else children.
  static void moveAsLastChildren(HLIf *If, HLContainerTy::iterator First,
                                 HLContainerTy::iterator Last,
                                 bool IsThenChild = true);

  /// \brief Unlinks [First, Last) from their current position and inserts them
  /// at the beginning of default case child of switch.
  static void moveAsFirstDefaultChildren(HLSwitch *Switch,
                                         HLContainerTy::iterator First,
                                         HLContainerTy::iterator Last);
  /// \brief Unlinks [First, Last) from their current position and inserts them
  /// at the end of default case child of switch.
  static void moveAsLastDefaultChildren(HLSwitch *Switch,
                                        HLContainerTy::iterator First,
                                        HLContainerTy::iterator Last);

  /// \brief Unlinks [First, Last) from their current position and inserts them
  /// at the beginning of CasNum case child of switch.
  /// Range of CaseNum is [1, getNumCases()].
  static void moveAsFirstChildren(HLSwitch *Switch,
                                  HLContainerTy::iterator First,
                                  HLContainerTy::iterator Last,
                                  unsigned CaseNum);
  /// \brief Unlinks [First, Last) from their current position and inserts them
  /// at the end of CasNum case child of switch.
  /// Range of CaseNum is [1, getNumCases()].
  static void moveAsLastChildren(HLSwitch *Switch,
                                 HLContainerTy::iterator First,
                                 HLContainerTy::iterator Last,
                                 unsigned CaseNum);

  /// \brief Unlinks [First, Last) from their current position and inserts them
  /// at the beginning of Loop's preheader.
  static void moveAsFirstPreheaderNodes(HLLoop *Loop,
                                        HLContainerTy::iterator First,
                                        HLContainerTy::iterator Last);
  /// \brief Unlinks [First, Last) from their current position and inserts them
  /// at the end of Loop's preheader.
  static void moveAsLastPreheaderNodes(HLLoop *Loop,
                                       HLContainerTy::iterator First,
                                       HLContainerTy::iterator Last);

  /// \brief Unlinks [First, Last) from their current position and inserts them
  /// at the beginning of Loop's postexit.
  static void moveAsFirstPostexitNodes(HLLoop *Loop,
                                       HLContainerTy::iterator First,
                                       HLContainerTy::iterator Last);
  /// \brief Unlinks [First, Last) from their current position and inserts them
  /// at the end of Loop's postexit.
  static void moveAsLastPostexitNodes(HLLoop *Loop,
                                      HLContainerTy::iterator First,
                                      HLContainerTy::iterator Last);

  /// \brief Unlinks Node from HIR.
  static void remove(HLNode *Node);
  /// \brief Unlinks Node from HIR and destroys it.
  static void erase(HLNode *Node);
  /// \brief Unlinks [First, Last) from HIR and destroys them.
  static void erase(HLContainerTy::iterator First,
                    HLContainerTy::iterator Last);

  /// \brief Replaces OldNode by an unlinked NewNode.
  static void replace(HLNode *OldNode, HLNode *NewNode);

  /// \brief Reset TopSortNum
  static void resetTopSortNum();

  /// \brief get parent loop for certain level, nullptr could be returned
  /// if input is invalid
  static const HLLoop *getParentLoopwithLevel(unsigned level,
                                              const HLLoop *innermostLoop);
};

} // End namespace loopopt

} // End namespace llvm

#endif
