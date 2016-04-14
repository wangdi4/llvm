//===-------- HLNodeUtils.h - Utilities for HLNode class ---*- C++ -*------===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
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

#include "llvm/Support/Compiler.h"
#include <set>

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/NoFolder.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeVisitor.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRFramework.h"

namespace llvm {

class Function;
class BasicBlock;
class Instruction;

namespace loopopt {

class HIRCreation;
class HIRFramework;

/// \brief Defines utilities for HLNode class
///
/// It contains a bunch of static member functions which manipulate HLNodes.
/// It does not store any state.
///
class HLNodeUtils : public HIRUtils {
private:
  /// \brief Do not allow instantiation.
  HLNodeUtils() = delete;

  /// \brief Used to create dummy LLVM instructions corresponding to new HIR
  /// instructions. Dummy instructions are appended to the function entry
  /// bblock. IRBuilder by default uses constant folding which needs to be
  /// suppressed for dummy instructions so we use NoFolder class instead.
  typedef IRBuilder<true, NoFolder> DummyIRBuilderTy;
  static DummyIRBuilderTy *DummyIRBuilder;
  /// \brief Points to first dummy instruction of the function.
  static Instruction *FirstDummyInst;
  /// \brief Points to last dummy instruction of the function.
  static Instruction *LastDummyInst;

  friend class HIRCreation;
  friend class HIRCleanup;
  friend class HIRLoopFormation;
  friend class HIRFramework;

  /// \brief Visitor for clone sequence.
  struct CloneVisitor;

  struct LoopFinderUpdater;

  template <bool Force = true> struct TopSorter;

  template <typename T> static void checkHLLoopTy() {
    // Assert to check that the type is HLLoop. Type can be const or non-const.
    static_assert(std::is_same<typename std::remove_const<
                                   typename std::remove_pointer<T>::type>::type,
                               HLLoop>::value,
                  "Type should be HLLoop * or const HLLoop *.");
  }

  /// \brief An enumeration to denote what level to visit. Used internally by
  /// LoopLevel Visitor.
  enum VisitKind { Innermost, All, Level };

  /// \brief Visitor to gather loops with specified level.
  template <typename T, VisitKind VL>
  struct LoopLevelVisitor final : public HLNodeVisitorBase {

    SmallVectorImpl<T> &LoopContainer;
    const HLNode *SkipNode;
    unsigned Level;

    LoopLevelVisitor(SmallVectorImpl<T> &Loops, unsigned Lvl = 0)
        : LoopContainer(Loops), SkipNode(nullptr), Level(Lvl) {
      checkHLLoopTy<T>();
      bool IsLevelVisit = (VL == VisitKind::Level);
      (void)IsLevelVisit;
      assert((!IsLevelVisit || isLoopLevelValid(Level)) &&
             " Level is out of range.");
    }

    void visit(T Loop) {
      switch (VL) {
      case VisitKind::All:
        // Gather all loops.
        LoopContainer.push_back(Loop);
        if (Loop->isInnermost()) {
          SkipNode = Loop;
        }
        break;
      case VisitKind::Innermost:
        // Gather only innermost loops.
        if (Loop->isInnermost()) {
          LoopContainer.push_back(Loop);
          SkipNode = Loop;
        }
        break;
      case VisitKind::Level:
        // Gather loops with specified Level.
        if (Loop->getNestingLevel() == Level) {
          LoopContainer.push_back(Loop);
          SkipNode = Loop;
        } else if (Loop->isInnermost()) {
          SkipNode = Loop;
        }
        break;
      default:
        llvm_unreachable("Invalid Visit Kind.");
      }
    }

    void visit(const HLNode *Node) {}
    void postVisit(const HLNode *Node) {}

    bool skipRecursion(const HLNode *Node) const override {
      assert(Node && "Null node found.");
      return (Node == SkipNode);
    }
  };

  /// \brief Updates first and last dummy inst of the function.
  static void setFirstAndLastDummyInst(Instruction *Inst);

  /// \brief Initializes static members for this function.
  static void initialize();

  /// \brief Returns a new HLRegion. Only used by framework.
  static HLRegion *createHLRegion(IRRegion *IRReg);

  /// \brief Returns a new HLLabel. Only used by framework.
  static HLLabel *createHLLabel(BasicBlock *SrcBB);

  /// \brief Returns a new external HLGoto that branches outside of HLRegion.
  /// Only used by framework.
  static HLGoto *createHLGoto(BasicBlock *TargetBB);

  /// \brief Returns a new HLInst. Only used by framework.
  static HLInst *createHLInst(Instruction *In);

  /// \brief Returns a new HLLoop created from an underlying LLVM loop. Only
  /// used by framework.
  static HLLoop *createHLLoop(const Loop *LLVMLoop);

  /// \brief Destroys all HLNodes, called during framework cleanup.
  static void destroyAll();

  /// \brief Performs sanity checking on unary instruction operands.
  static void checkUnaryInstOperands(RegDDRef *LvalRef, RegDDRef *RvalRef,
                                     Type *DestTy);

  /// \brief Performs sanity checking on binary instruction operands.
  static void checkBinaryInstOperands(RegDDRef *LvalRef, RegDDRef *OpRef1,
                                      RegDDRef *OpRef2);

  /// \brief Creates an HLInst for this Inst. It assigns LvalRef as the lval
  /// DDRef if it isn't null, otherwise, a new non-linear self-blob DDRef is
  /// created and assigned. It also updates first dummy instruction, if
  /// applicable.
  static HLInst *createLvalHLInst(Instruction *Inst, RegDDRef *LvalRef);

  /// \brief Creates an HLInst for this Inst. Used for void function call
  /// and other instructions that do not have Lvalue.
  /// It also updates first dummy instruction, if applicable.
  static HLInst *createNonLvalHLInst(Instruction *Inst);

  /// \brief Creates a unary instruction.
  static HLInst *createUnaryHLInst(unsigned OpCode, RegDDRef *RvalRef,
                                   const Twine &Name, RegDDRef *LvalRef,
                                   Type *DestTy);

  /// \brief Creates a binary instruction.
  static HLInst *createBinaryHLInstImpl(unsigned OpCode, RegDDRef *OpRef1,
                                        RegDDRef *OpRef2, const Twine &Name,
                                        RegDDRef *LvalRef, bool HasNUWOrExact,
                                        bool HasNSW, MDNode *FPMathTag);

  /// \brief Implementation of cloneSequence() which clones from Node1
  /// to Node2 and inserts into the CloneContainer.
  static void cloneSequenceImpl(HLContainerTy *CloneContainer,
                                const HLNode *Node1, const HLNode *Node2);

  /// \brief Returns successor of Node assuming control flows in strict lexical
  /// order (by ignoring jumps(gotos)).
  /// This should only be called from HIRCleanup pass.
  static HLNode *getLexicalControlFlowSuccessor(HLNode *Node);

  /// Internal helper functions, not to be called directly.

  /// \brief Implements insert(before) functionality. Moves [First, last) from
  /// OrigContainer to Parent's container. If OrigContainer is null it
  /// assumes a range of 1(node). UpdateSeparator indicates whether separators
  /// used in containers should be updated. Additional arguments for updating
  /// postexit separator and switch's case number is required.
  static void insertImpl(HLNode *Parent, HLContainerTy::iterator Pos,
                         HLContainerTy *OrigContainer,
                         HLContainerTy::iterator First,
                         HLContainerTy::iterator Last, bool UpdateSeparator,
                         bool PostExitSeparator = false, int CaseNum = -1);

  /// \brief Moves [First, last) from OrigContainer to InsertContainer.
  /// If OrigContainer is null it assumes a range of 1(node) and inserts
  /// First into InsertContainer..
  static void insertInternal(HLContainerTy &InsertContainer,
                             HLContainerTy::iterator Pos,
                             HLContainerTy *OrigContainer,
                             HLContainerTy::iterator First,
                             HLContainerTy::iterator Last);

  /// \brief Updates nesting level and innermost flag for Loop.
  static void updateLoopInfo(HLLoop *Loop);

  /// \brief Helper function for recursively updating loop info for loops in
  /// [First, Last). This is called during insertion.
  static void updateLoopInfoRecursively(HLContainerTy::iterator First,
                                        HLContainerTy::iterator Last);

  /// \brief Implements insertAs*Child() functionality.
  static void insertAsChildImpl(HLNode *Parent, HLContainerTy *OrigContainer,
                                HLContainerTy::iterator First,
                                HLContainerTy::iterator Last,
                                bool IsFirstChild);

  /// \brief Implements insertAs*Child() functionality for switch.
  static void insertAsChildImpl(HLSwitch *Switch, HLContainerTy *OrigContainer,
                                HLContainerTy::iterator First,
                                HLContainerTy::iterator Last, unsigned CaseNum,
                                bool isFirstChild);

  /// \brief Returns true if nodes are valid types as preheader/postexit nodes.
  static bool validPreheaderPostexitNodes(HLContainerTy::iterator First,
                                          HLContainerTy::iterator Last);

  /// \brief Implements insertAs*Preheader*()/insertAs*Postexit*()
  /// functionality.
  static void insertAsPreheaderPostexitImpl(
      HLLoop *Loop, HLContainerTy *OrigContainer, HLContainerTy::iterator First,
      HLContainerTy::iterator Last, bool IsPreheader, bool IsFirstChild);

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

  /// \brief Implements moveAs*Children() functionality for switch.
  static void moveAsChildrenImpl(HLSwitch *Switch,
                                 HLContainerTy::iterator First,
                                 HLContainerTy::iterator Last, unsigned CaseNum,
                                 bool isFirstChild);

  /// \brief Implements get*LexicalChild() functionality.
  static const HLNode *getLexicalChildImpl(const HLNode *Parent,
                                           const HLNode *Node, bool First);

  /// \brief Returns true if the lexical link have structured flow between
  /// Parent's first/last child and Node. The direction is dictated by
  /// UpwardTraversal flag. TargetNode is used for early termination of the
  /// traversal. Structured flow checks are different for domination and
  /// post-domination.
  static bool hasStructuredFlow(const HLNode *Parent, const HLNode *Node,
                                const HLNode *TargetNode, bool PostDomination,
                                bool UpwardTraversal);

  /// \brief Returns the outermost parent of Node1 which is safe to be used for
  /// checking domination. We move up through constant trip count loops. Last
  /// parent indicates the path used to reach to the parent.
  static const HLNode *getOutermostSafeParent(const HLNode *Node1,
                                              const HLNode *Node2,
                                              bool PostDomination,
                                              const HLNode **LastParent1);

  /// \brief Internally used by domination utility to get to the common
  /// dominating parent. Last parent indicates the path used to reach to the
  /// parent.
  static const HLNode *getCommonDominatingParent(const HLNode *Parent1,
                                                 const HLNode *LastParent1,
                                                 const HLNode *Node2,
                                                 bool PostDomination,
                                                 const HLNode **LastParent2);

  /// \brief Implements domination/post-domination functionality.
  static bool dominatesImpl(const HLNode *Node1, const HLNode *Node2,
                            bool PostDomination, bool StrictDomination);

  /// \brief Move Loop Bounds, IVtype and ZTT, etc. from one loop to another
  static void moveProperties(HLLoop *SrcLoop, HLLoop *DstLoop);

  /// \brief Set TopSortNums for the first time
  static void initTopSortNum();

  /// \brief Called by the framework to update TopSortNum field for
  /// a range of HLNodes
  static void updateTopSortNum(const HLContainerTy &Container,
                               HLContainerTy::iterator First,
                               HLContainerTy::iterator Last);

  /// \brief Evenly sets TopSortNumbers from a range (MinNum, MaxNum) to
  /// subtrees [First, Last)
  static void distributeTopSortNum(HLContainerTy::iterator First,
                                   HLContainerTy::iterator Last,
                                   unsigned MinNum, unsigned MaxNum);

  /// \brief Implements get*LinkListNode() functionality.
  static HLNode *getLinkListNodeImpl(HLNode *Node, bool Prev);

  /// \brief Returns the previous node belonging to its parent if one exists,
  /// else returns nullptr.
  static HLNode *getPrevLinkListNode(HLNode *Node);

  /// \brief Returns the next node belonging to its parent if one exists, else
  /// returns nullptr.
  static HLNode *getNextLinkListNode(HLNode *Node);

public:
  /// \brief return true if non-zero
  static bool isKnownNonZero(const CanonExpr *CE,
                             const HLLoop *ParentLoop = nullptr);
  /// \brief return true if non-positive
  static bool isKnownNonPositive(const CanonExpr *CE,
                                 const HLLoop *ParentLoop = nullptr);
  /// \brief return true if non-negative
  static bool isKnownNonNegative(const CanonExpr *CE,
                                 const HLLoop *ParentLoop = nullptr);
  /// \brief return true if negative
  static bool isKnownNegative(const CanonExpr *CE,
                              const HLLoop *ParentLoop = nullptr);
  /// \brief return true if positive
  static bool isKnownPositive(const CanonExpr *CE,
                              const HLLoop *ParentLoop = nullptr);

  /// \brief Returns the first dummy instruction of the function.
  static Instruction *getFirstDummyInst() { return FirstDummyInst; }

  /// \brief Returns the last dummy instruction of the function.
  static Instruction *getLastDummyInst() { return LastDummyInst; }

  /// \brief Returns a new HLSwitch.
  static HLSwitch *createHLSwitch(RegDDRef *ConditionRef);

  /// \brief Returns a new HLLabel with custom name.
  static HLLabel *createHLLabel(const Twine &Name = "L");

  /// \brief Returns a new HLGoto that branches to HLLabel.
  static HLGoto *createHLGoto(HLLabel *TargetL);

  /// \brief Returns a new HLIf.
  static HLIf *createHLIf(CmpInst::Predicate FirstPred, RegDDRef *Ref1,
                          RegDDRef *Ref2);

  /// \brief Returns a new HLLoop.
  static HLLoop *createHLLoop(HLIf *ZttIf = nullptr,
                              RegDDRef *LowerDDRef = nullptr,
                              RegDDRef *UpperDDRef = nullptr,
                              RegDDRef *StrideDDRef = nullptr,
                              unsigned NumEx = 1);

  /// \brief Destroys the passed in HLNode.
  static void destroy(HLNode *Node);

  /// Utilities to create new HLInsts follow. Please note that LvalRef argument
  /// defaults to null and hence follows rval ref arguments in the function
  /// signature. A new non-linear self blob ref is created if the LvalRef is set
  /// to null.

  /// \brief Used to create copy instructions of the form: Lval = Rval;
  static HLInst *createCopyInst(RegDDRef *RvalRef, const Twine &Name = "copy",
                                RegDDRef *LvalRef = nullptr);

  /// \brief Creates a new Load instruction.
  static HLInst *createLoad(RegDDRef *RvalRef, const Twine &Name = "load",
                            RegDDRef *LvalRef = nullptr);

  /// \brief Creates a new Store instruction.
  static HLInst *createStore(RegDDRef *RvalRef, const Twine &Name = "store",
                             RegDDRef *LvalRef = nullptr);

  /// \brief Creates a new Trunc instruction.
  static HLInst *createTrunc(Type *DestTy, RegDDRef *RvalRef,
                             const Twine &Name = "trunc",
                             RegDDRef *LvalRef = nullptr);

  /// \brief Creates a new ZExt instruction.
  static HLInst *createZExt(Type *DestTy, RegDDRef *RvalRef,
                            const Twine &Name = "zext",
                            RegDDRef *LvalRef = nullptr);

  /// \brief Creates a new SExt instruction.
  static HLInst *createSExt(Type *DestTy, RegDDRef *RvalRef,
                            const Twine &Name = "sext",
                            RegDDRef *LvalRef = nullptr);

  /// \brief Creates a new FPToUI instruction.
  static HLInst *createFPToUI(Type *DestTy, RegDDRef *RvalRef,
                              const Twine &Name = "cast",
                              RegDDRef *LvalRef = nullptr);

  /// \brief Creates a new FPToSI instruction.
  static HLInst *createFPToSI(Type *DestTy, RegDDRef *RvalRef,
                              const Twine &Name = "cast",
                              RegDDRef *LvalRef = nullptr);

  /// \brief Creates a new UIToFP instruction.
  static HLInst *createUIToFP(Type *DestTy, RegDDRef *RvalRef,
                              const Twine &Name = "cast",
                              RegDDRef *LvalRef = nullptr);

  /// \brief Creates a new SIToFP instruction.
  static HLInst *createSIToFP(Type *DestTy, RegDDRef *RvalRef,
                              const Twine &Name = "cast",
                              RegDDRef *LvalRef = nullptr);
  /// \brief Creates a new FPTrunc instruction.
  static HLInst *createFPTrunc(Type *DestTy, RegDDRef *RvalRef,
                               const Twine &Name = "ftrunc",
                               RegDDRef *LvalRef = nullptr);

  /// \brief Creates a new FPExt instruction.
  static HLInst *createFPExt(Type *DestTy, RegDDRef *RvalRef,
                             const Twine &Name = "fext",
                             RegDDRef *LvalRef = nullptr);

  /// \brief Creates a new PtrToInt instruction.
  static HLInst *createPtrToInt(Type *DestTy, RegDDRef *RvalRef,
                                const Twine &Name = "cast",
                                RegDDRef *LvalRef = nullptr);

  /// \brief Creates a new IntToPtr instruction.
  static HLInst *createIntToPtr(Type *DestTy, RegDDRef *RvalRef,
                                const Twine &Name = "cast",
                                RegDDRef *LvalRef = nullptr);

  /// \brief Creates a new BitCast instruction.
  static HLInst *createBitCast(Type *DestTy, RegDDRef *RvalRef,
                               const Twine &Name = "cast",
                               RegDDRef *LvalRef = nullptr);

  /// \brief Creates a new AddrSpaceCast instruction.
  static HLInst *createAddrSpaceCast(Type *DestTy, RegDDRef *RvalRef,
                                     const Twine &Name = "cast",
                                     RegDDRef *LvalRef = nullptr);

  /// \brief Creates a new BinaryOperator with specified opcode. If
  /// OrigBinOp is not null, copy IR flags from OrigBinOp to the newly
  /// create instruction.
  static HLInst *createBinaryHLInst(unsigned OpCode, RegDDRef *OpRef1,
                                    RegDDRef *OpRef2,
                                    const Twine &Name = "",
                                    RegDDRef *LvalRef = nullptr,
                                    const BinaryOperator *OrigBinOp = nullptr);

  /// \brief Creates a new Cast instruction with specified opcode.
  static HLInst *createCastHLInst(Type *DestTy, unsigned OpCode,
                                  RegDDRef *OpRef, const Twine &Name = "",
                                  RegDDRef *LvalRef = nullptr);
  /// \brief Creates a new Add instruction.
  static HLInst *createAdd(RegDDRef *OpRef1, RegDDRef *OpRef2,
                           const Twine &Name = "add",
                           RegDDRef *LvalRef = nullptr, bool HasNUW = false,
                           bool HasNSW = false);

  /// \brief Creates a new FAdd instruction.
  static HLInst *createFAdd(RegDDRef *OpRef1, RegDDRef *OpRef2,
                            const Twine &Name = "fadd",
                            RegDDRef *LvalRef = nullptr,
                            MDNode *FPMathTag = nullptr);

  /// \brief Creates a new Sub instruction.
  static HLInst *createSub(RegDDRef *OpRef1, RegDDRef *OpRef2,
                           const Twine &Name = "sub",
                           RegDDRef *LvalRef = nullptr, bool HasNUW = false,
                           bool HasNSW = false);

  /// \brief Creates a new FSub instruction.
  static HLInst *createFSub(RegDDRef *OpRef1, RegDDRef *OpRef2,
                            const Twine &Name = "fsub",
                            RegDDRef *LvalRef = nullptr,
                            MDNode *FPMathTag = nullptr);

  /// \brief Creates a new Mul instruction.
  static HLInst *createMul(RegDDRef *OpRef1, RegDDRef *OpRef2,
                           const Twine &Name = "mul",
                           RegDDRef *LvalRef = nullptr, bool HasNUW = false,
                           bool HasNSW = false);

  /// \brief Creates a new FMul instruction.
  static HLInst *createFMul(RegDDRef *OpRef1, RegDDRef *OpRef2,
                            const Twine &Name = "fmul",
                            RegDDRef *LvalRef = nullptr,
                            MDNode *FPMathTag = nullptr);

  /// \brief Creates a new UDiv instruction.
  static HLInst *createUDiv(RegDDRef *OpRef1, RegDDRef *OpRef2,
                            const Twine &Name = "udiv",
                            RegDDRef *LvalRef = nullptr, bool IsExact = false);

  /// \brief Creates a new SDiv instruction.
  static HLInst *createSDiv(RegDDRef *OpRef1, RegDDRef *OpRef2,
                            const Twine &Name = "sdiv",
                            RegDDRef *LvalRef = nullptr, bool IsExact = false);

  /// \brief Creates a new FDiv instruction.
  static HLInst *createFDiv(RegDDRef *OpRef1, RegDDRef *OpRef2,
                            const Twine &Name = "fdiv",
                            RegDDRef *LvalRef = nullptr,
                            MDNode *FPMathTag = nullptr);

  /// \brief Creates a new URem instruction.
  static HLInst *createURem(RegDDRef *OpRef1, RegDDRef *OpRef2,
                            const Twine &Name = "urem",
                            RegDDRef *LvalRef = nullptr);

  /// \brief Creates a new SRem instruction.
  static HLInst *createSRem(RegDDRef *OpRef1, RegDDRef *OpRef2,
                            const Twine &Name = "srem",
                            RegDDRef *LvalRef = nullptr);

  /// \brief Creates a new FRem instruction.
  static HLInst *createFRem(RegDDRef *OpRef1, RegDDRef *OpRef2,
                            const Twine &Name = "frem",
                            RegDDRef *LvalRef = nullptr,
                            MDNode *FPMathTag = nullptr);

  /// \brief Creates a new Shl instruction.
  static HLInst *createShl(RegDDRef *OpRef1, RegDDRef *OpRef2,
                           const Twine &Name = "shl",
                           RegDDRef *LvalRef = nullptr, bool HasNUW = false,
                           bool HasNSW = false);

  /// \brief Creates a new LShr instruction.
  static HLInst *createLShr(RegDDRef *OpRef1, RegDDRef *OpRef2,
                            const Twine &Name = "lshl",
                            RegDDRef *LvalRef = nullptr, bool IsExact = false);

  /// \brief Creates a new AShr instruction.
  static HLInst *createAShr(RegDDRef *OpRef1, RegDDRef *OpRef2,
                            const Twine &Name = "ashr",
                            RegDDRef *LvalRef = nullptr, bool IsExact = false);

  /// \brief Creates a new And instruction.
  static HLInst *createAnd(RegDDRef *OpRef1, RegDDRef *OpRef2,
                           const Twine &Name = "and",
                           RegDDRef *LvalRef = nullptr);

  /// \brief Creates a new Or instruction.
  static HLInst *createOr(RegDDRef *OpRef1, RegDDRef *OpRef2,
                          const Twine &Name = "or",
                          RegDDRef *LvalRef = nullptr);

  /// \brief Creates a new Xor instruction.
  static HLInst *createXor(RegDDRef *OpRef1, RegDDRef *OpRef2,
                           const Twine &Name = "xor",
                           RegDDRef *LvalRef = nullptr);

  /// \brief Creates a new Cmp instruction.
  static HLInst *createCmp(CmpInst::Predicate Pred, RegDDRef *OpRef1,
                           RegDDRef *OpRef2, const Twine &Name = "cmp",
                           RegDDRef *LvalRef = nullptr);

  /// \brief Creates a new Select instruction.
  static HLInst *createSelect(CmpInst::Predicate Pred, RegDDRef *OpRef1,
                              RegDDRef *OpRef2, RegDDRef *OpRef3,
                              RegDDRef *OpRef4, const Twine &Name = "select",
                              RegDDRef *LvalRef = nullptr);
  /// \brief Creates a new Call instruction.
  static HLInst *createCall(Function *F,
                            const SmallVectorImpl<RegDDRef*> &CallArgs,
                            const Twine &Name = "call",
                            RegDDRef *LvalRef = nullptr);

  /// \brief Creates a new ShuffleVector instruction
  static HLInst *CreateShuffleVectorInst(RegDDRef *OpRef1, RegDDRef *OpRef2,
                                         ArrayRef<int> Mask,
                                         const Twine &Name = "shuffle",
                                         RegDDRef *LvalRef = nullptr);

  /// \brief Creates a new ExtractElement instruction
  static HLInst *CreateExtractElementInst(RegDDRef *OpRef, unsigned Idx,
                                          const Twine &Name = "extract",
                                          RegDDRef *LvalRef = nullptr);

  /// \brief Creates a clones sequence from Node1 to Node2, including both
  /// the nodes and all the nodes in between them. If Node2 is null or Node1
  /// equals Node2, then the utility just clones Node1 and inserts into the
  /// CloneContainer. This utility does not support Region cloning.
  static void cloneSequence(HLContainerTy *CloneContainer, const HLNode *Node1,
                            const HLNode *Node2 = nullptr);

  /// \brief Visits the passed in HLNode.
  template <bool Recursive = true, bool RecurseInsideLoops = true,
            bool Forward = true, typename HV, typename NodeTy,
            typename = IsHLNodeTy<NodeTy>>
  static void visit(HV &Visitor, NodeTy *Node) {
    HLNodeVisitor<HV, Recursive, RecurseInsideLoops, Forward> V(Visitor);
    V.visit(Node);
  }

  /// \brief Visits HLNodes in the range [begin, end). The direction is
  /// specified using Forward flag. Recursion across all the HLNodes is
  /// specified using Recursive flag and Recursion inside HLLoops is
  /// specified using RecurseInsideLoops (which is only used when
  /// Recursive flag is set).
  template <bool Recursive = true, bool RecurseInsideLoops = true,
            bool Forward = true, typename HV, typename NodeTy,
            typename = IsHLNodeTy<NodeTy>>
  static void visitRange(HV &Visitor, ilist_iterator<NodeTy> Begin,
                         ilist_iterator<NodeTy> End) {
    HLNodeVisitor<HV, Recursive, RecurseInsideLoops, Forward> V(Visitor);
    V.visitRange(Begin, End);
  }

  /// \brief Visits HLNodes in the range [begin, end]. The direction is
  /// specified using Forward flag. This is overloaded to have begin and
  /// end as HLNode parameters.
  template <bool Recursive = true, bool RecurseInsideLoops = true,
            bool Forward = true, typename HV, typename NodeTy,
            typename = IsHLNodeTy<NodeTy>>
  static void visitRange(HV &Visitor, NodeTy *Begin, NodeTy *End) {
    assert(Begin && End && " Begin/End Node is null");
    ilist_iterator<NodeTy> BeginIter(Begin);
    ilist_iterator<NodeTy> EndIter(End);
    EndIter++;
    visitRange<Recursive, RecurseInsideLoops, Forward>(Visitor, BeginIter,
                                                       EndIter);
  }

  /// \brief Visits all HLNodes in the HIR. The direction is specified using
  /// Forward flag.
  template <bool Recursive = true, bool RecurseInsideLoops = true,
            bool Forward = true, typename HV>
  static void visitAll(HV &Visitor) {
    HLNodeVisitor<HV, Recursive, RecurseInsideLoops, Forward> V(Visitor);
    V.visitRange(getHIRFramework()->hir_begin(), getHIRFramework()->hir_end());
  }

  /// \brief Visits HLNodes in the HIR in InnerToOuter loop hierarchy
  /// order. The direction is specified using Forward flag.
  template <typename HV, bool Forward = true>
  static void visitInnerToOuter(HV &Visitor, HLNode *Node) {
    HLInnerToOuterLoopVisitor<HV, Forward> V(Visitor);
    V.visitRecurseInsideLoops(Node);
  }

  /// \brief Visits all HLNodes in the HIR in InnerToOuter loop hierarchy
  /// order. The direction is specified using Forward flag.
  template <typename HV, bool Forward = true>
  static void visitAllInnerToOuter(HV &Visitor) {
    HLInnerToOuterLoopVisitor<HV, Forward> V(Visitor);
    V.visitRangeRecurseInsideLoops(getHIRFramework()->hir_begin(),
                                   getHIRFramework()->hir_end());
  }

  /// \brief Visits all HLNodes in the HIR in OuterToInner loop hierarchy
  /// order. The direction is specified using Forward flag.
  template <typename HV, bool Forward = true>
  static void visitAllOuterToInner(HV &Visitor) {
    HLNodeVisitor<HV, true, true, Forward> V(Visitor);
    V.visit(getHIRFramework()->hir_begin(), getHIRFramework()->hir_end());
  }

  /// \brief Inserts an unlinked Node before Pos in HIR.
  static void insertBefore(HLNode *Pos, HLNode *Node);
  /// \brief Inserts unlinked Nodes in NodeContainer before Pos in HIR.
  /// The contents of NodeContainer will be empty after insertion.
  static void insertBefore(HLNode *Pos, HLContainerTy *NodeContainer);
  /// \brief Inserts an unlinked Node after Pos in HIR.
  static void insertAfter(HLNode *Pos, HLNode *Node);
  /// \brief Inserts unlinked Nodes in NodeContainer after Pos in HIR.
  /// The contents of NodeContainer will be empty after insertion.
  static void insertAfter(HLNode *Pos, HLContainerTy *NodeContainer);

  /// \brief Inserts an unlinked Node as first child of parent region.
  static void insertAsFirstChild(HLRegion *Reg, HLNode *Node);
  /// \brief Inserts an unlinked Node as last child of parent region.
  static void insertAsLastChild(HLRegion *Reg, HLNode *Node);

  /// \brief Inserts an unlinked Node as first child of parent loop.
  static void insertAsFirstChild(HLLoop *Loop, HLNode *Node);
  /// \brief Inserts unlinked Nodes as first children of parent loop.
  /// The order of NodeContainer is insertion order.
  /// The contents of NodeContainer will be empty after insertion.
  static void insertAsFirstChildren(HLLoop *Loop, HLContainerTy *NodeContainer);
  /// \brief Inserts an unlinked Node as last child of parent loop.
  static void insertAsLastChild(HLLoop *Loop, HLNode *Node);
  /// \brief Inserts unlinked Nodes as last children of parent loop.
  /// The order of NodeContainer is insertion order.
  /// The contents of NodeContainer will be empty after insertion.
  static void insertAsLastChildren(HLLoop *Loop, HLContainerTy *NodeContainer);

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

  /// \brief Unlinks a set for node from HIR and places then in the container.
  static void remove(HLContainerTy *Container, HLNode *Node1, HLNode *Node2);

  /// \brief Unlinks Node from HIR and destroys it.
  static void erase(HLNode *Node);
  /// \brief Unlinks [First, Last) from HIR and destroys them.
  static void erase(HLContainerTy::iterator First,
                    HLContainerTy::iterator Last);

  /// \brief Replaces OldNode by an unlinked NewNode.
  static void replace(HLNode *OldNode, HLNode *NewNode);

  /// \brief Returns true if Node is in the top sort num range [FirstNode,
  /// LastNode].
  static bool isInTopSortNumRange(const HLNode *Node, const HLNode *FirstNode,
                                  const HLNode *LastNode);

  /// \brief Returns true if the Loop level is in a valid range from
  /// [1, MaxLoopNestLevel].
  static bool isLoopLevelValid(unsigned Level) {
    return (Level > 0 && Level <= MaxLoopNestLevel);
  }

  /// \brief Returns the first lexical child of the parent w.r.t Node. For
  /// example, if parent is a loop and Node lies in postexit, the function will
  /// return the first postexit node. If Node is null, it returns the absolute
  /// first/last child in the parent's container.
  /// Please note that this function internally uses top sort num so it must be
  /// valid.
  static const HLNode *getFirstLexicalChild(const HLNode *Parent,
                                            const HLNode *Node = nullptr);
  static HLNode *getFirstLexicalChild(HLNode *Parent, HLNode *Node = nullptr);

  /// \brief Returns the last lexical child of the parent w.r.t Node. For
  /// example, if parent is a loop and Node lies in postexit, the function will
  /// return the last postexit node. If Node is null, it returns the absolute
  /// first/last child in the parent's container.
  /// Please note that this function internally uses top sort num so it must be
  /// valid.
  static const HLNode *getLastLexicalChild(const HLNode *Parent,
                                           const HLNode *Node = nullptr);
  static HLNode *getLastLexicalChild(HLNode *Parent, HLNode *Node = nullptr);

  /// \brief Returns true if Node1 can be proven to dominate Node2, otherwise
  /// conservatively returns false.
  static bool dominates(const HLNode *Node1, const HLNode *Node2);

  /// \brief This is identical to dominates() except the case where Node1 ==
  /// Node2, in which case it return false.
  static bool strictlyDominates(const HLNode *Node1, const HLNode *Node2);

  /// \brief Returns true if Node1 can be proven to post-dominate Node2,
  /// otherwise conservatively returns false.
  static bool postDominates(const HLNode *Node1, const HLNode *Node2);

  /// \brief This is identical to postDominates() except the case where Node1 ==
  /// Node2, in which case it return false.
  static bool strictlyPostDominates(const HLNode *Node1, const HLNode *Node2);

  /// \brief Checks if \p Node1 and \p Node2 are "equivalent" in terms of CFG:
  /// namely if \p Node1 is reached/accessed anytime \p Node2 is
  /// reached/Accessed and vice versa. This allows placing/accessing the nodes
  /// together in the same location.
  /// Returns false if there may exist a scenario/path in which Node1 is
  /// reached/accessed and Node2 isn't, or the other way around.
  /// Note: In the presence of complicated unstructured code (containing 
  /// gotos/labels) this function will conservatively return false.
  static bool canAccessTogether(const HLNode *Node1, const HLNode *Node2);

  /// \brief Returns true if Parent contains Node. IncludePrePostHdr indicates
  /// whether loop should be considered to contain preheader/postexit nodes.
  static bool contains(const HLNode *Parent, const HLNode *Node,
                       bool IncludePrePostHdr = true);

  /// \brief get parent loop for certain level, nullptr could be returned
  /// if input is invalid
  static const HLLoop *getParentLoopwithLevel(unsigned Level,
                                              const HLLoop *InnermostLoop);

  /// \brief Gathers the innermost loops across regions and stores them into
  /// the loop vector.
  template <typename T>
  static void gatherInnermostLoops(SmallVectorImpl<T> &Loops) {
    LoopLevelVisitor<T, VisitKind::Innermost> LoopVisit(Loops);
    HLNodeUtils::visitAll(LoopVisit);
  }

  /// \brief Gathers the outermost loops (or highest level loops with Level 1)
  /// across regions and stores them into the loop vector.
  template <typename T>
  static void gatherOutermostLoops(SmallVectorImpl<T> &Loops) {
    // Level 1 denotes outermost loops
    LoopLevelVisitor<T, VisitKind::Level> LoopVisit(Loops, 1);
    HLNodeUtils::visitAll(LoopVisit);
  }

  /// \brief Gathers loops inside the Node with specified Level and stores them
  /// in the Loops vector.
  template <typename T>
  static void gatherLoopsWithLevel(HLNode *Node, SmallVectorImpl<T> &Loops,
                                   unsigned Level) {
    assert(Node && " Node is null.");
    HLLoop *Loop = dyn_cast<HLLoop>(Node);
    (void)Loop;
    assert((!Loop || !Loop->isInnermost()) &&
           " Gathering loops inside innermost loop.");
    assert(isLoopLevelValid(Level) && " Level is out of range.");
    LoopLevelVisitor<T, VisitKind::Level> LoopVisit(Loops, Level);
    HLNodeUtils::visit(LoopVisit, Node);
  }

  /// \brief Constant Node version of gatherLoopsWithLevel.
  template <typename T>
  static void gatherLoopsWithLevel(const HLNode *Node,
                                   SmallVectorImpl<T> &Loops, unsigned Level) {
    static_assert(std::is_const<typename std::remove_pointer<T>::type>::value,
                  "Type of SmallVector parameter should be const HLLoop *.");
    gatherLoopsWithLevel(const_cast<HLNode *>(Node), Loops, Level);
  }

  /// \brief Gathers all the loops across regions and stores them in the
  ///  Loops vector.
  template <typename T> static void gatherAllLoops(SmallVectorImpl<T> &Loops) {
    LoopLevelVisitor<T, VisitKind::All> LoopVisit(Loops);
    HLNodeUtils::visitAll(LoopVisit);
  }

  /// \brief Gathers all the loops inside the Node and stores them
  /// in the Loops vector.
  template <typename T>
  static void gatherAllLoops(HLNode *Node, SmallVectorImpl<T> &Loops) {
    assert(Node && " Node is null.");
    LoopLevelVisitor<T, VisitKind::All> LoopVisit(Loops);
    HLNodeUtils::visit(LoopVisit, Node);
  }

  /// \brief Constant Node version of gatherAllLoops.
  template <typename T>
  static void gatherAllLoops(const HLNode *Node, SmallVectorImpl<T> &Loops) {
    static_assert(std::is_const<typename std::remove_pointer<T>::type>::value,
                  "Type of SmallVector parameter should be const HLLoop *.");
    gatherAllLoops(const_cast<HLNode *>(Node), Loops);
  }

  /// \brief Returns true if HLSwitch or HLCall exists between NodeStart and
  /// NodeEnd. RecurseInsideLoops flag denotes if we want to check switch
  /// or call inside nested loops. Usually, this flag is used to for
  /// optimizations where checks have already been made for child loops.
  // TODO: Move this utility to header file, if there are no users except
  // unrolling.
  static bool hasSwitchOrCall(const HLNode *NodeStart, const HLNode *NodeEnd,
                              bool RecurseInsideLoops = true);
  /// \brief Updates Loop properties (Bounds, etc) based on input Permutations
  ///   Used by Interchange now. Could be used later for blocking
  static void
  permuteLoopNests(HLLoop *Loop,
                   SmallVector<HLLoop *, MaxLoopNestLevel> LoopPermutation);

  /// \brief Returns true if Loop is a perfect Loop nest
  /// and the innermost loop
  static bool isPerfectLoopNest(const HLLoop *Loop,
                                const HLLoop **InnermostLoop,
                                bool AllowPrePostHdr = false,
                                bool AllowTriangularLoop = false,
                                bool AllowNearPerfect = false,
                                bool *IsNearPerfect = nullptr);

  /// \breif Check if Loop has perfect/near-perfect loop properties
  ///  set  innermost loop in *Lp if it is hit
  ///  Return true if it has perfect/near-perfect loop properties
  static bool hasPerfectLoopProperties(const HLNode *Node, const HLLoop **Lp,
                                       bool AllowNearPerfect,
                                       bool *IsNearPerfectLoop);

  ///  \brief Any memref with non-unit stride?
  ///   Will take innermost loop for now
  ///   used mostly for blocking / interchange
  static bool hasNonUnitStrideRefs(const HLLoop *Loop);
		
  /// \brief Find node receiving the load
  /// e.g.   t0 = a[i] ;
  ///         ...
  ///        t1 = t0
  ///  returns t1 = t0
  static HLInst *
  findForwardSubInst(const DDRef *LRef,
                     SmallVectorImpl<HLInst *> &ForwardSubInsts);



  /// \brief Returns the lowest common ancestor loop of Lp1 and Lp2. Returns
  /// null if there is no such parent loop.
  static const HLLoop *getLowestCommonAncestorLoop(const HLLoop *Lp1,
                                                   const HLLoop *Lp2);
  static HLLoop *getLowestCommonAncestorLoop(HLLoop *Lp1, HLLoop *Lp2);
};

} // End namespace loopopt

} // End namespace llvm

#endif
