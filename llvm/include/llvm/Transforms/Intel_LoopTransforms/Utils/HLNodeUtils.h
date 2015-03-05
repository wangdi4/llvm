//===-------- HLNodeUtils.h - Utilities for HLNode class --------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
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
#include "llvm/IR/Intel_LoopIR/HLRegion.h"
#include "llvm/IR/Intel_LoopIR/HLSwitch.h"
#include "llvm/IR/Intel_LoopIR/HLLabel.h"
#include "llvm/IR/Intel_LoopIR/HLGoto.h"
#include "llvm/IR/Intel_LoopIR/HLInst.h"
#include "llvm/IR/Intel_LoopIR/HLIf.h"
#include "llvm/IR/Intel_LoopIR/HLLoop.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeVisitor.h"

namespace llvm {

class BasicBlock;
class Instruction;

namespace loopopt {

/// \brief Defines utilities for HLNode class
///
/// It contains a bunch of static member functions which manipulate HLNodes.
/// It does not store any state.
///
class HLNodeUtils {
private:
  /// \brief Do not allow instantiation.
  HLNodeUtils() LLVM_DELETED_FUNCTION;

  struct LoopFinderUpdater;

  /// Internal helper functions, not to be called directly.

  /// \brief Implements insert functionality. Moves [First, last) from
  /// RemoveContainer to Parent's container. If RemoveContainer is null
  /// it assumes a range of 1(node). UpdateSeparator indicates whether
  /// separators used in containers should be updated. Additional
  /// flag for updating postexit separator is required.
  static void insertImpl(HLNode *Parent, HLContainerTy::iterator Pos,
                         HLContainerTy *RemoveContainer,
                         HLContainerTy::iterator First,
                         HLContainerTy::iterator Last, bool UpdateSeparator,
                         bool PostExitSeparator = false);

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

  /// \brief Implements insertAsChild*() functionality.
  static void insertAsChildImpl(HLNode *Parent, HLContainerTy *RemoveContainer,
                                HLContainerTy::iterator First,
                                HLContainerTy::iterator Last,
                                bool IsFirstChild);

  /// \brief Implements insertAsPreheader*()/insertAsPostexit*() functionality.
  static void insertAsPreheaderPostexitImpl(HLLoop *Loop,
                                            HLContainerTy *RemoveContainer,
                                            HLContainerTy::iterator First,
                                            HLContainerTy::iterator Last,
                                            bool IsPreheader,
                                            bool IsFirstChild);

  /// \brief Implements IsInPreheader*()/IsInPostexit*() functionality.
  static bool IsInPreheaderPostexitImpl(const HLLoop *Loop, const HLNode *Node,
                                        bool Preheader);

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

public:
  /// \brief Returns a new HLRegion.
  static HLRegion *createHLRegion(std::set<BasicBlock *> &OrigBBs,
                                  BasicBlock *EntryBB, BasicBlock *ExitBB);

  /// \brief Returns a new HLSwitch.
  static HLSwitch *createHLSwitch();

  /// \brief Returns a new HLLabel.
  static HLLabel *createHLLabel(BasicBlock *SrcBB);

  /// \brief Returns a new HLGoto.
  static HLGoto *createHLGoto(BasicBlock *TargetBB, HLLabel *TargetL = nullptr);

  /// \brief Returns a new HLInst.
  static HLInst *createHLInst(Instruction *In);

  /// \brief Returns a new HLIf.
  static HLIf *createHLIf(CmpInst::Predicate FirstPred, DDRef *Ref1,
                          DDRef *Ref2);

  /// \brief Returns a new HLLoop.
  static HLLoop *createHLLoop(HLIf *ZttIf = nullptr,
                              DDRef *LowerDDRef = nullptr,
                              DDRef *TripCountDDRef = nullptr,
                              DDRef *StrideDDRef = nullptr, bool isDoWh = false,
                              unsigned NumEx = 1);

  /// \brief Destroys the passed in HLNode.
  static void destroy(HLNode *Node);
  /// \brief Destroys all HLNodes. Should only be called after code gen.
  static void destroyAll();

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
      V.forwardVisitAll();
    } else {
      V.backwardVisitAll();
    }
  }

  /// \brief Inserts an unlinked Node before Pos in HIR.
  static void insertBefore(HLNode *Pos, HLNode *Node);
  /// \brief Inserts an unlinked Node after Pos in HIR.
  static void insertAfter(HLNode *Pos, HLNode *Node);

  /// \brief Inserts an unlinked Node as first child of this If. The flag
  /// IsThenChild indicates whether this is to be inserted as then or else
  /// child.
  static void insertAsFirstIfChild(HLIf *If, HLNode *Node,
                                   bool IsThenChild = true);
  /// \brief Inserts an unlinked Node as last child of this If. The flag
  /// IsThenChild indicates whether this is to be inserted as then or else
  /// child.
  static void insertAsLastIfChild(HLIf *If, HLNode *Node,
                                  bool IsThenChild = true);

  /// \brief Inserts an unlinked Node as first child of parent loop/region.
  /// Use specialized version of insert if extra info is required to insert
  /// children.
  static void insertAsFirstChild(HLNode *Parent, HLNode *Node);
  /// \brief Inserts an unlinked Node as last child of parent loop/region.
  /// Use specialized version of insert if extra info is required to insert
  /// children.
  static void insertAsLastChild(HLNode *Parent, HLNode *Node);

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
  /// of this If. The flag IsThenChild indicates whether this is to be moved
  /// as then or else child.
  static void moveAsFirstIfChild(HLIf *If, HLNode *Node,
                                 bool IsThenChild = true);
  /// \brief Unlinks Node from its current position and inserts as last child
  /// of this If. The flag IsThenChild indicates whether this is to be moved
  /// as then or else child.
  static void moveAsLastIfChild(HLIf *If, HLNode *Node,
                                bool IsThenChild = true);

  /// \brief Unlinks Node from its current position and inserts as first child
  /// of parent loop/region. Use specialized version of move if extra info is
  /// required to insert children.
  static void moveAsFirstChild(HLNode *Parent, HLNode *Node);
  /// \brief Unlinks Node from its current position and inserts as last child
  /// of parent loop/region. Use specialized version of move if extra info is
  /// required to insert children.
  static void moveAsLastChild(HLNode *Parent, HLNode *Node);

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
  /// at the begining of this If. The flag IsThenChild indicates whether they
  /// are to be moved as then or else children.
  static void moveAsFirstIfChildren(HLIf *If, HLContainerTy::iterator First,
                                    HLContainerTy::iterator Last,
                                    bool IsThenChild = true);
  /// \brief Unlinks [First, Last) from their current position and inserts them
  /// at the end of this If. The flag IsThenChild indicates whether they are to
  /// be moved as then or else children.
  static void moveAsLastIfChildren(HLIf *If, HLContainerTy::iterator First,
                                   HLContainerTy::iterator Last,
                                   bool IsThenChild = true);

  /// \brief Unlinks [First, Last) from their current position and inserts them
  /// at the begining of the parent loop/region's children. Use specialized
  /// version of move if extra info is required.
  static void moveAsFirstChildren(HLNode *Parent, HLContainerTy::iterator First,
                                  HLContainerTy::iterator Last);
  /// \brief Unlinks [First, Last) from their current position and inserts them
  /// at the end of the parent loop/region's children. Use specialized version
  /// of move if extra info is required.
  static void moveAsLastChildren(HLNode *Parent, HLContainerTy::iterator First,
                                 HLContainerTy::iterator Last);

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

  /// \brief Returns true if the Node is in Loop's preheader.
  static bool IsInPreheader(const HLLoop *Loop, const HLNode *Node);
  /// \brief Returns true if the Node is in Loop's postexit.
  static bool IsInPostexit(const HLLoop *Loop, const HLNode *Node);
  /// \brief Returns true if the Node is in Loop's preheader or postexit.
  static bool IsInPreheaderOrPostexit(const HLLoop *Loop, const HLNode *Node);
};

} // End namespace loopopt

} // End namespace llvm

#endif
