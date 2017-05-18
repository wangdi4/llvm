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

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/NoFolder.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRFramework.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeVisitor.h"

#include <set>

namespace llvm {

class Function;
class BasicBlock;
class Instruction;

namespace loopopt {

class HIRCreation;
class HIRFramework;
class CanonExprUtils;
class BlobUtils;
class HIRLoopStatistics;

/// Defines utilities for HLNode class and manages their creation/destruction.
/// It contains a bunch of member functions which manipulate HLNodes.
class HLNodeUtils {
private:
  /// Keeps track of HLNode objects.
  std::set<HLNode *> Objs;
  unsigned NextUniqueHLNodeNumber;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  // Stores names of cloned labels in HIR.
  StringSet<> LabelNames;
#endif

  DDRefUtils *DDRU;
  HIRFramework *HIRF;

  /// Used to create dummy LLVM instructions corresponding to new HIR
  /// instructions. Dummy instructions are appended to the function entry
  /// bblock. IRBuilder by default uses constant folding which needs to be
  /// suppressed for dummy instructions so we use NoFolder class instead.
  typedef IRBuilder<NoFolder> DummyIRBuilderTy;
  DummyIRBuilderTy *DummyIRBuilder;
  /// Points to first dummy instruction of the function.
  Instruction *FirstDummyInst;
  /// Points to last dummy instruction of the function.
  Instruction *LastDummyInst;
  // Used as a temporary marker by transformations to keep track of where the
  // insertion should take place.
  HLLabel *Marker;

  HLNodeUtils()
      : NextUniqueHLNodeNumber(0), DDRU(nullptr), DummyIRBuilder(nullptr),
        FirstDummyInst(nullptr), LastDummyInst(nullptr), Marker(nullptr) {}

  /// Make class uncopyable.
  HLNodeUtils(const HLNodeUtils &) = delete;
  void operator=(const HLNodeUtils &) = delete;

  friend class HIRCreation;
  friend class HIRCleanup;
  friend class HIRLoopFormation;
  friend class HIRParser;
  friend class HIRFramework;
  // Requires access to Objs.
  friend class HLNode;
  // Requires access to LabelNames.
  friend class HLLabel;

  // Resets the state.
  void reset(Function &F);

  /// Visitor for clone sequence.
  struct CloneVisitor;

  struct LoopFinderUpdater;

  template <bool Force = true> struct TopSorter;

  template <typename T> void checkHLLoopTy() {
    // Assert to check that the type is HLLoop. Type can be const or non-const.
    static_assert(std::is_same<typename std::remove_const<
                                   typename std::remove_pointer<T>::type>::type,
                               HLLoop>::value,
                  "Type should be HLLoop * or const HLLoop *.");
  }

  /// An enumeration to denote what level to visit. Used internally by LoopLevel
  /// Visitor.
  enum VisitKind { Innermost, All, Level };

  /// Visitor to gather loops with specified level.
  template <typename T, VisitKind VL>
  struct LoopLevelVisitor final : public HLNodeVisitorBase {
    HLNodeUtils &HNU;
    SmallVectorImpl<T> &LoopContainer;
    const HLNode *SkipNode;
    unsigned Level;

    LoopLevelVisitor(HLNodeUtils &HNU, SmallVectorImpl<T> &Loops,
                     unsigned Lvl = 0)
        : HNU(HNU), LoopContainer(Loops), SkipNode(nullptr), Level(Lvl) {
      HNU.checkHLLoopTy<T>();
      bool IsLevelVisit = (VL == VisitKind::Level);
      (void)IsLevelVisit;
      assert((!IsLevelVisit || CanonExprUtils::isValidLoopLevel(Level)) &&
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

  unsigned getUniqueHLNodeNumber() { return NextUniqueHLNodeNumber++; }

  /// Updates first and last dummy inst of the function.
  void setFirstAndLastDummyInst(Instruction *Inst);

  /// Returns a new HLRegion. Only used by framework.
  HLRegion *createHLRegion(IRRegion &IRReg);

  /// Returns a new HLLabel. Only used by framework.
  HLLabel *createHLLabel(BasicBlock *SrcBB);

  /// Returns a new external HLGoto that branches outside of HLRegion.
  /// Only used by framework.
  HLGoto *createHLGoto(BasicBlock *TargetBB);

  /// Returns a new HLInst. Only used by framework.
  HLInst *createHLInst(Instruction *In);

  /// Returns a new HLLoop created from an underlying LLVM loop.
  /// Only used by framework.
  HLLoop *createHLLoop(const Loop *LLVMLoop);

  /// Destroys all allocated memory, called by framework.
  void destroyAll();

  /// Performs sanity checking on unary instruction operands.
  void checkUnaryInstOperands(RegDDRef *LvalRef, RegDDRef *RvalRef,
                              Type *DestTy);

  /// Performs sanity checking on binary instruction operands.
  void checkBinaryInstOperands(RegDDRef *LvalRef, RegDDRef *OpRef1,
                               RegDDRef *OpRef2);

  /// Creates an HLInst for this Inst. It assigns LvalRef as the lval DDRef if
  /// it isn't null, otherwise, a new non-linear self-blob DDRef is created and
  /// assigned. It also updates first dummy instruction, if applicable.
  HLInst *createLvalHLInst(Instruction *Inst, RegDDRef *LvalRef);

  /// Creates an HLInst for this Inst. Used for void function call and other
  /// instructions that do not have Lvalue. It also updates first dummy
  /// instruction, if applicable.
  HLInst *createNonLvalHLInst(Instruction *Inst);

  /// Creates a unary instruction.
  HLInst *createUnaryHLInst(unsigned OpCode, RegDDRef *RvalRef,
                            const Twine &Name, RegDDRef *LvalRef, Type *DestTy);

  /// Creates a binary instruction.
  HLInst *createBinaryHLInstImpl(unsigned OpCode, RegDDRef *OpRef1,
                                 RegDDRef *OpRef2, const Twine &Name,
                                 RegDDRef *LvalRef, bool HasNUWOrExact,
                                 bool HasNSW, MDNode *FPMathTag);

  /// Creates and inserts a dummy copy instruction.
  Instruction *createCopyInstImpl(Type *Ty, const Twine &Name);

  /// Implementation of cloneSequence() which clones from Node1
  /// to Node2 and inserts into the CloneContainer.
  void cloneSequenceImpl(HLContainerTy *CloneContainer, const HLNode *Node1,
                         const HLNode *Node2, HLNodeMapper *NodeMapper);

  /// Returns successor of Node assuming control flows in strict lexical order
  /// (by ignoring jumps(gotos)).
  /// This should only be called from HIRCleanup pass.
  HLNode *getLexicalControlFlowSuccessor(HLNode *Node);

  /// Internal helper functions, not to be called directly.

  /// Implements insert(before) functionality. Moves [First, last) from
  /// OrigContainer to Parent's container. If OrigContainer is null it assumes a
  /// range of 1(node). UpdateSeparator indicates whether separators used in
  /// containers should be updated. Additional arguments for updating postexit
  /// separator and switch's case number is required.
  void insertImpl(HLNode *Parent, HLContainerTy::iterator Pos,
                  HLContainerTy *OrigContainer, HLContainerTy::iterator First,
                  HLContainerTy::iterator Last, bool UpdateSeparator,
                  bool PostExitSeparator = false, int CaseNum = -1);

  /// Moves [First, last) from OrigContainer to InsertContainer.
  /// If OrigContainer is null it assumes a range of 1(node) and inserts First
  /// into InsertContainer.
  void insertInternal(HLContainerTy &InsertContainer,
                      HLContainerTy::iterator Pos, HLContainerTy *OrigContainer,
                      HLContainerTy::iterator First,
                      HLContainerTy::iterator Last);

  /// Updates nesting level and innermost flag for Loop.
  void updateLoopInfo(HLLoop *Loop);

  /// Helper function for recursively updating loop info for loops in
  /// [First, Last). This is called during insertion.
  void updateLoopInfoRecursively(HLContainerTy::iterator First,
                                 HLContainerTy::iterator Last);

  /// Implements insertAs*Child() functionality.
  void insertAsChildImpl(HLNode *Parent, HLContainerTy *OrigContainer,
                         HLContainerTy::iterator First,
                         HLContainerTy::iterator Last, bool IsFirstChild);

  /// Implements insertAs*Child() functionality for switch.
  void insertAsChildImpl(HLSwitch *Switch, HLContainerTy *OrigContainer,
                         HLContainerTy::iterator First,
                         HLContainerTy::iterator Last, unsigned CaseNum,
                         bool isFirstChild);

  /// Returns true if nodes are valid types as preheader/postexit nodes.
  bool validPreheaderPostexitNodes(HLContainerTy::iterator First,
                                   HLContainerTy::iterator Last);

  /// Implements insertAs*Preheader*()/insertAs*Postexit*() functionality.
  void insertAsPreheaderPostexitImpl(HLLoop *Loop, HLContainerTy *OrigContainer,
                                     HLContainerTy::iterator First,
                                     HLContainerTy::iterator Last,
                                     bool IsPreheader, bool IsFirstChild);

  /// Implements remove functionality. Removes [First, last) and destroys them
  /// if Erase is set. If erase isn't set and MoveContainer isn't null they are
  /// moved to MoveContainer. Otherwise, nodes are removed without destroying
  /// them.
  static void removeImpl(HLContainerTy::iterator First,
                         HLContainerTy::iterator Last,
                         HLContainerTy *MoveContainer, bool Erase = false);

  /// Removes [First, Last) from Container. Also destroys them is Erase is set.
  static void removeInternal(HLContainerTy &Container,
                             HLContainerTy::iterator First,
                             HLContainerTy::iterator Last, bool Erase);

  /// Unlinks Node from HIR and destroys it.
  /// Note: This function is intentionally private. Transformations are not
  /// supposed to erase nodes as cleaning up erased nodes from HIR analyses
  /// requires implementation of a callback mechanism which doesn't seem worth
  /// it.
  static void erase(HLNode *Node);

  /// Unlinks [First, Last) from HIR and destroys them.
  /// Note: This function is intentionally private. Transformations are not
  /// supposed to erase nodes as cleaning up erased nodes from HIR analyses
  /// requires implementation of a callback mechanism which doesn't seem worth
  /// it.
  static void erase(HLContainerTy::iterator First,
                    HLContainerTy::iterator Last);

  /// Returns true if a loop is found in range [First, Last).
  static bool foundLoopInRange(HLContainerTy::iterator First,
                               HLContainerTy::iterator Last);

  /// Update the goto branches with new labels.
  void updateGotos(GotoContainerTy *GotoList, LabelMapTy *LabelMap);

  /// Implements moveAs*Children() functionality for switch.
  void moveAsChildrenImpl(HLSwitch *Switch, HLContainerTy::iterator First,
                          HLContainerTy::iterator Last, unsigned CaseNum,
                          bool isFirstChild);

  /// Implements get*LexicalChild() functionality.
  static const HLNode *getLexicalChildImpl(const HLNode *Parent,
                                           const HLNode *Node, bool First);

  /// Returns true if the lexical link have structured flow between Parent's
  /// first/last child and Node. The direction is dictated by UpwardTraversal
  /// flag. TargetNode is used for early termination of the traversal.
  /// Structured flow checks are different for domination and post-domination.
  static bool hasStructuredFlow(const HLNode *Parent, const HLNode *Node,
                                const HLNode *TargetNode, bool PostDomination,
                                bool UpwardTraversal, HIRLoopStatistics *HLS);

  /// Returns the outermost parent of Node1 which is safe to be used for
  /// checking domination. We move up through constant trip count loops. Last
  /// parent indicates the path used to reach to the parent.
  static const HLNode *getOutermostSafeParent(const HLNode *Node1,
                                              const HLNode *Node2,
                                              bool PostDomination,
                                              HIRLoopStatistics *HLS,
                                              const HLNode **LastParent1);

  /// Internally used by domination utility to get to the commona dominating
  /// parent. Last parent indicates the path used to reach to the parent.
  static const HLNode *
  getCommonDominatingParent(const HLNode *Parent1, const HLNode *LastParent1,
                            const HLNode *Node2, bool PostDomination,
                            HIRLoopStatistics *HLS, const HLNode **LastParent2);

  /// Implements domination/post-domination functionality.
  static bool dominatesImpl(const HLNode *Node1, const HLNode *Node2,
                            bool PostDomination, bool StrictDomination,
                            HIRLoopStatistics *HLS);

  /// Set TopSortNums for the first time
  void initTopSortNum();

  /// Called by the framework to update TopSortNum field for a range of HLNodes.
  void updateTopSortNum(const HLContainerTy &Container,
                        HLContainerTy::iterator First,
                        HLContainerTy::iterator Last);

  /// Evenly sets TopSortNumbers from a range (MinNum, MaxNum) to subtrees
  /// [First, Last).
  void distributeTopSortNum(HLContainerTy::iterator First,
                            HLContainerTy::iterator Last, unsigned MinNum,
                            unsigned MaxNum);

  /// Implements get*LinkListNode() functionality.
  HLNode *getLinkListNodeImpl(HLNode *Node, bool Prev);

  /// Returns the previous node belonging to its parent if one exists, else
  /// returns nullptr.
  HLNode *getPrevLinkListNode(HLNode *Node);

  /// Returns the next node belonging to its parent if one exists, else returns
  /// nullptr.
  HLNode *getNextLinkListNode(HLNode *Node);

  enum VALType : unsigned { IsUnknown, IsConstant, IsMax, IsMin };

  /// Returns true if \p ValType can represent a minimum value type.
  bool isMinValue(VALType ValType);

  /// Returns true if \p ValType can represent a maximum value type.
  bool isMaxValue(VALType ValType);

  /// Returns true if the value in question is known to be positive.
  bool isKnownPositive(VALType ValType, int64_t Val);

  /// Returns true if the value in question is known to be non-negative.
  bool isKnownNonNegative(VALType ValType, int64_t Val);

  /// Returns true if the value in question is known to be negative.
  bool isKnownNegative(VALType ValType, int64_t Val);

  /// Returns true if the value in question is known to be non-positive.
  bool isKnownNonPositive(VALType ValType, int64_t Val);

  /// Returns true if the value in question is known to be non-zero.
  bool isKnownNonZero(VALType ValType, int64_t Val);

  /// Returns true if the value in question is known to be positive or negative.
  bool isKnownPositiveOrNegative(VALType ValType, int64_t Val);

  // Get possible Minimum/Maximum value of canon.
  // If known, return ValueType and Value.
  // Return value indicates if Val is used as Constant, Min or Max.
  VALType getMinMaxBlobValue(unsigned BlobIdx, const CanonExpr *BoundCE,
                             int64_t &Val);

  VALType getMinMaxBlobValueFromPred(unsigned BlobIdx, PredicateTy Pred,
                                     const RegDDRef *Lhs, const RegDDRef *Rhs,
                                     int64_t &Val);

  template <typename PredIter, typename GetDDRefFunc>
  VALType getMinMaxBlobValueFromPredRange(unsigned BlobIdx, PredIter Begin,
                                          PredIter End, GetDDRefFunc GetDDRef,
                                          bool InvertPredicates, int64_t &Val);

  VALType getMinMaxBlobValue(unsigned BlobIdx, const HLNode *ParentNode,
                             int64_t &Val);

  bool getMinMaxBlobValue(unsigned BlobIdx, int64_t Coeff,
                          const HLNode *ParentNode, bool IsMin,
                          int64_t &BlobVal);

  /// Returns constant min or max value of CE based on its context (ParentNode)
  /// and IsMin paramter. \p IsExact specifies whether we can calculate inexact
  /// min/max in the presence of blobs.
  /// Value is returned in Val. Only handles IVs + constant for now.
  bool getMinMaxValueImpl(const CanonExpr *CE, const HLNode *ParentNode,
                          bool IsMin, bool IsExact, int64_t &Val);

  /// Checks if Loop has perfect/near-perfect loop properties.
  /// Expects non-innermost incoming \p Lp.
  /// Sets inner loop in \p InnerLp.
  /// Return true if it has perfect/near-perfect loop properties
  static bool hasPerfectLoopProperties(const HLLoop *Lp, const HLLoop **InnerLp,
                                       bool AllowNearPerfect,
                                       bool *IsNearPerfectLoop);

  template <bool IsMaxMode>
  static bool isInTopSortNumRangeImpl(const HLNode *Node,
                                      const HLNode *FirstNode,
                                      const HLNode *LastNode);

  /// Test the condition described by Pred, LHS and RHS.
  static bool getPredicateResult(APInt &LHS, PredicateTy Pred, APInt &RHS);

public:
  /// Returns the first dummy instruction of the function.
  Instruction *getFirstDummyInst() { return FirstDummyInst; }

  /// Returns the last dummy instruction of the function.
  Instruction *getLastDummyInst() { return LastDummyInst; }

  /// Returns marker node which is used as a placeholder by transformations. If
  /// the marker is null, a new one is created.
  /// NOTE: It is expected to be detached from HIR by the transformationns after
  /// use.
  HLNode *getOrCreateMarkerNode() {
    if (!Marker) {
      Marker = createHLLabel("marker");
    }
    return Marker;
  }

  /// Returns marker node.
  HLNode *getMarkerNode() { return Marker; }

  // Returns reference to DDRefUtils object.
  DDRefUtils &getDDRefUtils() {
    assert(DDRU && "Access to null DDRefUtils!");
    return *DDRU;
  }

  const DDRefUtils &getDDRefUtils() const {
    assert(DDRU && "Access to null DDRefUtils!");
    return *DDRU;
  }

  // Returns reference to DDRefUtils object.
  CanonExprUtils &getCanonExprUtils();
  const CanonExprUtils &getCanonExprUtils() const;

  // Returns reference to DDRefUtils object.
  BlobUtils &getBlobUtils();
  const BlobUtils &getBlobUtils() const;

  // Returns pointer to HIRFramework.
  HIRFramework &getHIRFramework() { return *HIRF; }

  // Returns pointer to HIRFramework.
  const HIRFramework &getHIRFramework() const { return *HIRF; }

  /// Returns Function object.
  Function &getFunction() const;

  /// Returns Module object.
  Module &getModule() const;

  /// Returns LLVMContext object.
  LLVMContext &getContext() const;

  /// Returns DataLayout object.
  const DataLayout &getDataLayout() const;

  /// Returns a new HLSwitch.
  HLSwitch *createHLSwitch(RegDDRef *ConditionRef);

  /// Returns a new HLLabel with custom name.
  HLLabel *createHLLabel(const Twine &Name = "L");

  /// Returns a new HLGoto that branches to HLLabel.
  HLGoto *createHLGoto(HLLabel *TargetL);

  /// Returns a new HLIf.
  HLIf *createHLIf(const HLPredicate &FirstPred, RegDDRef *Ref1,
                   RegDDRef *Ref2);

  /// Returns a new HLLoop.
  HLLoop *createHLLoop(HLIf *ZttIf = nullptr, RegDDRef *LowerDDRef = nullptr,
                       RegDDRef *UpperDDRef = nullptr,
                       RegDDRef *StrideDDRef = nullptr, unsigned NumEx = 1);

  /// Destroys the passed in HLNode.
  void destroy(HLNode *Node);

  /// Utilities to create new HLInsts follow. Please note that LvalRef argument
  /// defaults to null and hence follows rval ref arguments in the function
  /// signature. A new non-linear self blob ref is created if the LvalRef is set
  /// to null.

  /// Creates a new underlying instruction and returns a self-blob DDRef
  /// representing that instruction.
  /// TODO: Although this interface has nothing to do with HLNodes, all the
  /// underlying setup exists here. Can we move this to DDRefUtils()?
  RegDDRef *createTemp(Type *Ty, const Twine &Name = "temp");

  /// Used to create copy instructions of the form: Lval = Rval;
  HLInst *createCopyInst(RegDDRef *RvalRef, const Twine &Name = "copy",
                         RegDDRef *LvalRef = nullptr);

  /// Creates a new Load instruction.
  HLInst *createLoad(RegDDRef *RvalRef, const Twine &Name = "load",
                     RegDDRef *LvalRef = nullptr);

  /// Creates a new Store instruction.
  HLInst *createStore(RegDDRef *RvalRef, const Twine &Name = "store",
                      RegDDRef *LvalRef = nullptr);

  /// Creates a new Trunc instruction.
  HLInst *createTrunc(Type *DestTy, RegDDRef *RvalRef,
                      const Twine &Name = "trunc", RegDDRef *LvalRef = nullptr);

  /// Creates a new ZExt instruction.
  HLInst *createZExt(Type *DestTy, RegDDRef *RvalRef,
                     const Twine &Name = "zext", RegDDRef *LvalRef = nullptr);

  /// Creates a new SExt instruction.
  HLInst *createSExt(Type *DestTy, RegDDRef *RvalRef,
                     const Twine &Name = "sext", RegDDRef *LvalRef = nullptr);

  /// Creates a new FPToUI instruction.
  HLInst *createFPToUI(Type *DestTy, RegDDRef *RvalRef,
                       const Twine &Name = "cast", RegDDRef *LvalRef = nullptr);

  /// Creates a new FPToSI instruction.
  HLInst *createFPToSI(Type *DestTy, RegDDRef *RvalRef,
                       const Twine &Name = "cast", RegDDRef *LvalRef = nullptr);

  /// Creates a new UIToFP instruction.
  HLInst *createUIToFP(Type *DestTy, RegDDRef *RvalRef,
                       const Twine &Name = "cast", RegDDRef *LvalRef = nullptr);

  /// Creates a new SIToFP instruction.
  HLInst *createSIToFP(Type *DestTy, RegDDRef *RvalRef,
                       const Twine &Name = "cast", RegDDRef *LvalRef = nullptr);
  /// Creates a new FPTrunc instruction.
  HLInst *createFPTrunc(Type *DestTy, RegDDRef *RvalRef,
                        const Twine &Name = "ftrunc",
                        RegDDRef *LvalRef = nullptr);

  /// Creates a new FPExt instruction.
  HLInst *createFPExt(Type *DestTy, RegDDRef *RvalRef,
                      const Twine &Name = "fext", RegDDRef *LvalRef = nullptr);

  /// Creates a new PtrToInt instruction.
  HLInst *createPtrToInt(Type *DestTy, RegDDRef *RvalRef,
                         const Twine &Name = "cast",
                         RegDDRef *LvalRef = nullptr);

  /// Creates a new IntToPtr instruction.
  HLInst *createIntToPtr(Type *DestTy, RegDDRef *RvalRef,
                         const Twine &Name = "cast",
                         RegDDRef *LvalRef = nullptr);

  /// Creates a new BitCast instruction.
  HLInst *createBitCast(Type *DestTy, RegDDRef *RvalRef,
                        const Twine &Name = "cast",
                        RegDDRef *LvalRef = nullptr);

  /// Creates a new AddrSpaceCast instruction.
  HLInst *createAddrSpaceCast(Type *DestTy, RegDDRef *RvalRef,
                              const Twine &Name = "cast",
                              RegDDRef *LvalRef = nullptr);

  /// Creates a new BinaryOperator with specified opcode. If OrigBinOp is not
  /// null, copy IR flags from OrigBinOp to the newly create instruction.
  HLInst *createBinaryHLInst(unsigned OpCode, RegDDRef *OpRef1,
                             RegDDRef *OpRef2, const Twine &Name = "",
                             RegDDRef *LvalRef = nullptr,
                             const BinaryOperator *OrigBinOp = nullptr);

  /// Creates a new Cast instruction with specified opcode.
  HLInst *createCastHLInst(Type *DestTy, unsigned OpCode, RegDDRef *OpRef,
                           const Twine &Name = "", RegDDRef *LvalRef = nullptr);
  /// Creates a new Add instruction.
  HLInst *createAdd(RegDDRef *OpRef1, RegDDRef *OpRef2,
                    const Twine &Name = "add", RegDDRef *LvalRef = nullptr,
                    bool HasNUW = false, bool HasNSW = false);

  /// Creates a new FAdd instruction.
  HLInst *createFAdd(RegDDRef *OpRef1, RegDDRef *OpRef2,
                     const Twine &Name = "fadd", RegDDRef *LvalRef = nullptr,
                     MDNode *FPMathTag = nullptr);

  /// Creates a new Sub instruction.
  HLInst *createSub(RegDDRef *OpRef1, RegDDRef *OpRef2,
                    const Twine &Name = "sub", RegDDRef *LvalRef = nullptr,
                    bool HasNUW = false, bool HasNSW = false);

  /// Creates a new FSub instruction.
  HLInst *createFSub(RegDDRef *OpRef1, RegDDRef *OpRef2,
                     const Twine &Name = "fsub", RegDDRef *LvalRef = nullptr,
                     MDNode *FPMathTag = nullptr);

  /// Creates a new Mul instruction.
  HLInst *createMul(RegDDRef *OpRef1, RegDDRef *OpRef2,
                    const Twine &Name = "mul", RegDDRef *LvalRef = nullptr,
                    bool HasNUW = false, bool HasNSW = false);

  /// Creates a new FMul instruction.
  HLInst *createFMul(RegDDRef *OpRef1, RegDDRef *OpRef2,
                     const Twine &Name = "fmul", RegDDRef *LvalRef = nullptr,
                     MDNode *FPMathTag = nullptr);

  /// Creates a new UDiv instruction.
  HLInst *createUDiv(RegDDRef *OpRef1, RegDDRef *OpRef2,
                     const Twine &Name = "udiv", RegDDRef *LvalRef = nullptr,
                     bool IsExact = false);

  /// Creates a new SDiv instruction.
  HLInst *createSDiv(RegDDRef *OpRef1, RegDDRef *OpRef2,
                     const Twine &Name = "sdiv", RegDDRef *LvalRef = nullptr,
                     bool IsExact = false);

  /// Creates a new FDiv instruction.
  HLInst *createFDiv(RegDDRef *OpRef1, RegDDRef *OpRef2,
                     const Twine &Name = "fdiv", RegDDRef *LvalRef = nullptr,
                     MDNode *FPMathTag = nullptr);

  /// Creates a new URem instruction.
  HLInst *createURem(RegDDRef *OpRef1, RegDDRef *OpRef2,
                     const Twine &Name = "urem", RegDDRef *LvalRef = nullptr);

  /// Creates a new SRem instruction.
  HLInst *createSRem(RegDDRef *OpRef1, RegDDRef *OpRef2,
                     const Twine &Name = "srem", RegDDRef *LvalRef = nullptr);

  /// Creates a new FRem instruction.
  HLInst *createFRem(RegDDRef *OpRef1, RegDDRef *OpRef2,
                     const Twine &Name = "frem", RegDDRef *LvalRef = nullptr,
                     MDNode *FPMathTag = nullptr);

  /// Creates a new Shl instruction.
  HLInst *createShl(RegDDRef *OpRef1, RegDDRef *OpRef2,
                    const Twine &Name = "shl", RegDDRef *LvalRef = nullptr,
                    bool HasNUW = false, bool HasNSW = false);

  /// Creates a new LShr instruction.
  HLInst *createLShr(RegDDRef *OpRef1, RegDDRef *OpRef2,
                     const Twine &Name = "lshl", RegDDRef *LvalRef = nullptr,
                     bool IsExact = false);

  /// Creates a new AShr instruction.
  HLInst *createAShr(RegDDRef *OpRef1, RegDDRef *OpRef2,
                     const Twine &Name = "ashr", RegDDRef *LvalRef = nullptr,
                     bool IsExact = false);

  /// Creates a new And instruction.
  HLInst *createAnd(RegDDRef *OpRef1, RegDDRef *OpRef2,
                    const Twine &Name = "and", RegDDRef *LvalRef = nullptr);

  /// Creates a new Or instruction.
  HLInst *createOr(RegDDRef *OpRef1, RegDDRef *OpRef2, const Twine &Name = "or",
                   RegDDRef *LvalRef = nullptr);

  /// Creates a new Xor instruction.
  HLInst *createXor(RegDDRef *OpRef1, RegDDRef *OpRef2,
                    const Twine &Name = "xor", RegDDRef *LvalRef = nullptr);

  /// Creates a new Cmp instruction.
  HLInst *createCmp(const HLPredicate &Pred, RegDDRef *OpRef1, RegDDRef *OpRef2,
                    const Twine &Name = "cmp", RegDDRef *LvalRef = nullptr);

  /// Creates a new Select instruction.
  HLInst *createSelect(const HLPredicate &Pred, RegDDRef *OpRef1,
                       RegDDRef *OpRef2, RegDDRef *OpRef3, RegDDRef *OpRef4,
                       const Twine &Name = "select",
                       RegDDRef *LvalRef = nullptr);

  /// Creates a new Call instruction.
  HLInst *createCall(Function *F, const SmallVectorImpl<RegDDRef *> &CallArgs,
                     const Twine &Name = "call", RegDDRef *LvalRef = nullptr);

  /// Creates a new ShuffleVector instruction
  HLInst *createShuffleVectorInst(RegDDRef *OpRef1, RegDDRef *OpRef2,
                                  ArrayRef<uint32_t> Mask,
                                  const Twine &Name = "shuffle",
                                  RegDDRef *LvalRef = nullptr);

  /// Creates a new ExtractElement instruction
  HLInst *createExtractElementInst(RegDDRef *OpRef, unsigned Idx,
                                   const Twine &Name = "extract",
                                   RegDDRef *LvalRef = nullptr);

  /// Creates a clones sequence from Node1 to Node2, including both the nodes
  /// and all the nodes in between them. If Node2 is null or Node1 equals
  /// Node2, then the utility just clones Node1 and inserts into the
  /// CloneContainer. If \p NodeMapper is not null, every node will be mapped
  /// to the cloned node. This is used for accessing clones having original
  /// node pointers.
  /// This utility does not support Region cloning.
  void cloneSequence(HLContainerTy *CloneContainer, const HLNode *Node1,
                     const HLNode *Node2 = nullptr,
                     HLNodeMapper *NodeMapper = nullptr) {
    assert(Node1 && "Node1 is null!");
    assert(!isa<HLRegion>(Node1) && "Node1 - Region Cloning is not allowed.");
    assert((!Node2 || !isa<HLRegion>(Node2)) &&
           " Node 2 - Region Cloning is not allowed.");
    assert(CloneContainer && " Clone Container is null.");
    assert((!Node2 || (Node1->getParent() == Node2->getParent())) &&
           " Parent of Node1 and Node2 don't match.");
    cloneSequenceImpl(CloneContainer, Node1, Node2, NodeMapper);
  }

  /// Visits the passed in HLNode.
  template <bool Recursive = true, bool RecurseInsideLoops = true,
            bool Forward = true, typename HV, typename NodeTy,
            typename = IsHLNodeTy<NodeTy>>
  static void visit(HV &Visitor, NodeTy *Node) {
    HLNodeVisitor<HV, Recursive, RecurseInsideLoops, Forward> V(Visitor);
    V.visit(Node);
  }

  /// Visits HLNodes in the range [begin, end). The direction is specified using
  /// Forward flag. Recursion across all the HLNodes is specified using
  /// Recursive flag and Recursion inside HLLoops is specified using
  /// RecurseInsideLoops (which is only used when Recursive flag is set).
  template <bool Recursive = true, bool RecurseInsideLoops = true,
            bool Forward = true, typename HV, typename It,
            typename = IsHLNodeTy<typename It::value_type>>
  static void visitRange(HV &Visitor, It Begin, It End) {
    HLNodeVisitor<HV, Recursive, RecurseInsideLoops, Forward> V(Visitor);
    V.visitRange(Begin, End);
  }

  /// Visits HLNodes in the range [begin, end]. The direction is specified using
  /// Forward flag. This is overloaded to have begin and end as HLNode
  /// parameters.
  template <bool Recursive = true, bool RecurseInsideLoops = true,
            bool Forward = true, typename HV, typename NodeTy,
            typename = IsHLNodeTy<NodeTy>>
  static void visitRange(HV &Visitor, NodeTy *Begin, NodeTy *End) {
    assert(Begin && End && " Begin/End Node is null");
    visitRange<Recursive, RecurseInsideLoops, Forward>(
        Visitor, Begin->getIterator(), ++(End->getIterator()));
  }

  /// Visits all HLNodes in the HIR. The direction is specified using Forward
  /// flag.
  template <bool Recursive = true, bool RecurseInsideLoops = true,
            bool Forward = true, typename HV>
  void visitAll(HV &Visitor) {
    HLNodeVisitor<HV, Recursive, RecurseInsideLoops, Forward> V(Visitor);
    V.visitRange(getHIRFramework().hir_begin(), getHIRFramework().hir_end());
  }

  /// Visits HLNodes in the HIR in InnerToOuter loop hierarchy order. The
  /// direction is specified using Forward flag.
  template <typename HV, bool Forward = true>
  void visitInnerToOuter(HV &Visitor, HLNode *Node) {
    HLInnerToOuterLoopVisitor<HV, Forward> V(Visitor);
    V.visitRecurseInsideLoops(Node);
  }

  /// Visits all HLNodes in the HIR in InnerToOuter loop hierarchy order. The
  /// direction is specified using Forward flag.
  template <typename HV, bool Forward = true>
  void visitAllInnerToOuter(HV &Visitor) {
    HLInnerToOuterLoopVisitor<HV, Forward> V(Visitor);
    V.visitRangeRecurseInsideLoops(getHIRFramework().hir_begin(),
                                   getHIRFramework().hir_end());
  }

  /// Visits all HLNodes in the HIR in OuterToInner loop hierarchy order. The
  /// direction is specified using Forward flag.
  template <typename HV, bool Forward = true>
  void visitAllOuterToInner(HV &Visitor) {
    HLNodeVisitor<HV, true, true, Forward> V(Visitor);
    V.visit(getHIRFramework().hir_begin(), getHIRFramework().hir_end());
  }

  /// Inserts an unlinked Node before Pos in HIR.
  void insertBefore(HLNode *Pos, HLNode *Node);
  /// Inserts unlinked Nodes in NodeContainer before Pos in HIR.
  /// The contents of NodeContainer will be empty after insertion.
  void insertBefore(HLNode *Pos, HLContainerTy *NodeContainer);
  /// Inserts an unlinked Node after Pos in HIR.
  void insertAfter(HLNode *Pos, HLNode *Node);
  /// Inserts unlinked Nodes in NodeContainer after Pos in HIR.
  /// The contents of NodeContainer will be empty after insertion.
  void insertAfter(HLNode *Pos, HLContainerTy *NodeContainer);

  /// Inserts an unlinked Node as first child of parent region.
  void insertAsFirstChild(HLRegion *Reg, HLNode *Node);
  /// Inserts an unlinked Node as last child of parent region.
  void insertAsLastChild(HLRegion *Reg, HLNode *Node);

  /// Inserts an unlinked Node as first child of parent loop.
  void insertAsFirstChild(HLLoop *Loop, HLNode *Node);
  /// Inserts unlinked Nodes as first children of parent loop.
  /// The order of NodeContainer is insertion order.
  /// The contents of NodeContainer will be empty after insertion.
  void insertAsFirstChildren(HLLoop *Loop, HLContainerTy *NodeContainer);
  /// Inserts an unlinked Node as last child of parent loop.
  void insertAsLastChild(HLLoop *Loop, HLNode *Node);
  /// Inserts unlinked Nodes as last children of parent loop.
  /// The order of NodeContainer is insertion order.
  /// The contents of NodeContainer will be empty after insertion.
  void insertAsLastChildren(HLLoop *Loop, HLContainerTy *NodeContainer);

  /// Inserts an unlinked Node as first child of this If. The flag IsThenChild
  /// indicates whether this is to be inserted as then or else child.
  void insertAsFirstChild(HLIf *If, HLNode *Node, bool IsThenChild);
  /// Inserts an unlinked Node as last child of this If. The flaga IsThenChild
  /// indicates whether this is to be inserted as then or else child.
  void insertAsLastChild(HLIf *If, HLNode *Node, bool IsThenChild);

  /// Inserts an unlinked Node as first default case child of switch.
  void insertAsFirstDefaultChild(HLSwitch *Switch, HLNode *Node);
  /// Inserts an unlinked Node as last default case child of switch.
  void insertAsLastDefaultChild(HLSwitch *Switch, HLNode *Node);

  /// Inserts an unlinked Node as first CaseNum case child of switch.
  /// Range of CaseNum is [1, getNumCases()].
  void insertAsFirstChild(HLSwitch *Switch, HLNode *Node, unsigned CaseNum);
  /// Inserts an unlinked Node as last CaseNum case child of switch.
  /// Range of CaseNum is [1, getNumCases()].
  void insertAsLastChild(HLSwitch *Switch, HLNode *Node, unsigned CaseNum);

  /// Inserts an unlinked Node as first preheader node of Loop.
  void insertAsFirstPreheaderNode(HLLoop *Loop, HLNode *Node);
  /// Inserts an unlinked Node as last preheader node of Loop.
  void insertAsLastPreheaderNode(HLLoop *Loop, HLNode *Node);

  /// Inserts an unlinked Node as first postexit node of Loop.
  void insertAsFirstPostexitNode(HLLoop *Loop, HLNode *Node);
  /// Inserts an unlinked Node as last postexit node of Loop.
  void insertAsLastPostexitNode(HLLoop *Loop, HLNode *Node);

  /// Unlinks Node from its current position and inserts it before Pos in HIR.
  void moveBefore(HLNode *Pos, HLNode *Node);
  /// Unlinks Node from its current position and inserts it after Pos in HIR.
  void moveAfter(HLNode *Pos, HLNode *Node);

  /// Unlinks Node from its current position and inserts as first child of
  /// parent region.
  void moveAsFirstChild(HLRegion *Reg, HLNode *Node);
  /// Unlinks Node from its current position and inserts as last child of parent
  /// region.
  void moveAsLastChild(HLRegion *Reg, HLNode *Node);

  /// Unlinks Node from its current position and inserts as first child of
  /// parent loop.
  void moveAsFirstChild(HLLoop *Loop, HLNode *Node);
  /// Unlinks Node from its current position and inserts as last child of parent
  /// loop.
  void moveAsLastChild(HLLoop *Loop, HLNode *Node);

  /// Unlinks Node from its current position and inserts as first childa of this
  /// If. The flag IsThenChild indicates whether this is to be moved as then or
  /// else child.
  void moveAsFirstChild(HLIf *If, HLNode *Node, bool IsThenChild = true);
  /// Unlinks Node from its current position and inserts as last child of this
  /// If. The flag IsThenChild indicates whether this is to be moved as then or
  /// else child.
  void moveAsLastChild(HLIf *If, HLNode *Node, bool IsThenChild = true);

  /// Unlinks Node from its current position and inserts as first default case
  /// child of switch.
  void moveAsFirstDefaultChild(HLSwitch *Switch, HLNode *Node);
  /// Unlinks Node from its current position and inserts as last default case
  /// child of switch.
  void moveAsLastDefaultChild(HLSwitch *Switch, HLNode *Node);

  /// Unlinks Node from its current position and inserts as first CaseNum case
  /// child of switch.
  /// Range of CaseNum is [1, getNumCases()].
  void moveAsFirstChild(HLSwitch *Switch, HLNode *Node, unsigned CaseNum);
  /// Unlinks Node from its current position and inserts as last CaseNum case
  /// child of switch.
  /// Range of CaseNum is [1, getNumCases()].
  void moveAsLastChild(HLSwitch *Switch, HLNode *Node, unsigned CaseNum);

  /// Unlinks Node from its current position and inserts as first preheader node
  /// of Loop.
  void moveAsFirstPreheaderNode(HLLoop *Loop, HLNode *Node);
  /// Unlinks Node from its current position and inserts an last preheader node
  /// of Loop.
  void moveAsLastPreheaderNode(HLLoop *Loop, HLNode *Node);

  /// Unlinks Node from its current position and inserts as first postexit node
  /// of Loop.
  void moveAsFirstPostexitNode(HLLoop *Loop, HLNode *Node);
  /// Unlinks Node from its current position and inserts as last postexit node
  /// of Loop.
  void moveAsLastPostexitNode(HLLoop *Loop, HLNode *Node);

  /// Unlinks [First, Last) from their current position and inserts them before
  /// Pos.
  void moveBefore(HLNode *Pos, HLContainerTy::iterator First,
                  HLContainerTy::iterator Last);
  /// Unlinks [First, Last) from their current position and inserts them after
  /// Pos.
  void moveAfter(HLNode *Pos, HLContainerTy::iterator First,
                 HLContainerTy::iterator Last);

  /// Unlinks [First, Last) from their current position and inserts them at the
  /// begining of the parent region's children.
  void moveAsFirstChildren(HLRegion *Reg, HLContainerTy::iterator First,
                           HLContainerTy::iterator Last);
  /// Unlinks [First, Last) from their current position and inserts them at the
  /// end of the parent region's children.
  void moveAsLastChildren(HLRegion *Reg, HLContainerTy::iterator First,
                          HLContainerTy::iterator Last);

  /// Unlinks [First, Last) from their current position and inserts them at the
  /// begining of the parent loop's children.
  void moveAsFirstChildren(HLLoop *Loop, HLContainerTy::iterator First,
                           HLContainerTy::iterator Last);
  /// Unlinks [First, Last) from their current position and inserts them at the
  /// end of the parent loop's children.
  void moveAsLastChildren(HLLoop *Loop, HLContainerTy::iterator First,
                          HLContainerTy::iterator Last);

  /// Unlinks [First, Last) from their current position and inserts them at the
  /// begining of this If. The flag IsThenChild indicates whether they are to be
  /// moved as then or else children.
  void moveAsFirstChildren(HLIf *If, HLContainerTy::iterator First,
                           HLContainerTy::iterator Last,
                           bool IsThenChild = true);
  /// Unlinks [First, Last) from their current position and inserts them at the
  /// end of this If. The flag IsThenChild indicates whether they are to be
  /// moved as then or else children.
  void moveAsLastChildren(HLIf *If, HLContainerTy::iterator First,
                          HLContainerTy::iterator Last,
                          bool IsThenChild = true);

  /// Unlinks [First, Last) from their current position and inserts them at the
  /// beginning of default case child of switch.
  void moveAsFirstDefaultChildren(HLSwitch *Switch,
                                  HLContainerTy::iterator First,
                                  HLContainerTy::iterator Last);
  /// Unlinks [First, Last) from their current position and inserts them at the
  /// end of default case child of switch.
  void moveAsLastDefaultChildren(HLSwitch *Switch,
                                 HLContainerTy::iterator First,
                                 HLContainerTy::iterator Last);

  /// Unlinks [First, Last) from their current position and inserts them at the
  /// beginning of CasNum case child of switch.
  /// Range of CaseNum is [1, getNumCases()].
  void moveAsFirstChildren(HLSwitch *Switch, HLContainerTy::iterator First,
                           HLContainerTy::iterator Last, unsigned CaseNum);
  /// Unlinks [First, Last) from their current position and inserts them at the
  /// end of CasNum case child of switch.
  /// Range of CaseNum is [1, getNumCases()].
  void moveAsLastChildren(HLSwitch *Switch, HLContainerTy::iterator First,
                          HLContainerTy::iterator Last, unsigned CaseNum);

  /// Unlinks [First, Last) from their current position and inserts them at the
  /// beginning of Loop's preheader.
  void moveAsFirstPreheaderNodes(HLLoop *Loop, HLContainerTy::iterator First,
                                 HLContainerTy::iterator Last);
  /// Unlinks [First, Last) from their current position and inserts them at the
  /// end of Loop's preheader.
  void moveAsLastPreheaderNodes(HLLoop *Loop, HLContainerTy::iterator First,
                                HLContainerTy::iterator Last);

  /// Unlinks [First, Last) from their current position and inserts them at the
  /// beginning of Loop's postexit.
  void moveAsFirstPostexitNodes(HLLoop *Loop, HLContainerTy::iterator First,
                                HLContainerTy::iterator Last);
  /// Unlinks [First, Last) from their current position and inserts them at the
  /// end of Loop's postexit.
  void moveAsLastPostexitNodes(HLLoop *Loop, HLContainerTy::iterator First,
                               HLContainerTy::iterator Last);

  /// Unlinks Node from HIR.
  static void remove(HLNode *Node);

  /// Unlinks [First, Last) from HIR.
  static void remove(HLContainerTy::iterator First,
                     HLContainerTy::iterator Last);

  /// Unlinks [First, Last] from HIR.
  static void remove(HLNode *First, HLNode *Last);

  /// Unlinks [First, Last) from HIR and places then in the container.
  static void remove(HLContainerTy *Container, HLContainerTy::iterator First,
                     HLContainerTy::iterator Last);

  /// Unlinks [First, Last] from HIR and places then in the container.
  static void remove(HLContainerTy *Container, HLNode *First, HLNode *Last);

  /// Replaces OldNode by an unlinked NewNode.
  void replace(HLNode *OldNode, HLNode *NewNode);

  /// Returns true if Node is in the top sort num range [\p FirstNode, \p
  /// LastNode]. The \p FirstNode could be a nullptr, the method will return
  /// false in this case.
  static bool isInTopSortNumRange(const HLNode *Node, const HLNode *FirstNode,
                                  const HLNode *LastNode);

  /// Returns true if \p Node top sort number is in range
  /// [FirstNode->getMinTopSortNum(), LastNode->getMaxTopSortNum].
  /// The \p FirstNode could be a nullptr, the method will return false in this
  /// case.
  static bool isInTopSortNumMaxRange(const HLNode *Node,
                                     const HLNode *FirstNode,
                                     const HLNode *LastNode);

  /// Returns the first lexical child of the parent w.r.t Node. For example, if
  /// parent is a loop and Node lies in postexit, the function will return the
  /// first postexit node. If Node is null, it returns the absolute first/last
  /// child in the parent's container.
  static const HLNode *getFirstLexicalChild(const HLNode *Parent,
                                            const HLNode *Node = nullptr);
  static HLNode *getFirstLexicalChild(HLNode *Parent, HLNode *Node = nullptr);

  /// Returns the last lexical child of the parent w.r.t Node. For example, if
  /// parent is a loop and Node lies in postexit, the function will return the
  /// last postexit node. If Node is null, it returns the absolute first/last
  /// child in the parent's container.
  static const HLNode *getLastLexicalChild(const HLNode *Parent,
                                           const HLNode *Node = nullptr);
  static HLNode *getLastLexicalChild(HLNode *Parent, HLNode *Node = nullptr);

  /// Returns true if Node1 can be proven to dominate Node2, otherwise
  /// conservatively returns false.
  /// \p HLS is used to produce faster results. A valid value can (and should)
  /// be passed by clients whenever the loop is in a valid state. This is true
  /// for all analysis passes and analysis part of the transformations. Null
  /// should be passed if the loop was modified but has not yet been invalidated
  /// by a transformation (invalid state).
  static bool dominates(const HLNode *Node1, const HLNode *Node2,
                        HIRLoopStatistics *HLS);

  /// This is identical to dominates() except the case where Node1 == Node2, in
  /// which case it return false.
  static bool strictlyDominates(const HLNode *Node1, const HLNode *Node2,
                                HIRLoopStatistics *HLS);

  /// Returns true if Node1 can be proven to post-dominate Node2, otherwise
  /// conservatively returns false.
  static bool postDominates(const HLNode *Node1, const HLNode *Node2,
                            HIRLoopStatistics *HLS);

  /// This is identical to postDominates() except the case where Node1 == Node2,
  /// in which case it return false.
  static bool strictlyPostDominates(const HLNode *Node1, const HLNode *Node2,
                                    HIRLoopStatistics *HLS);

  /// Checks if \p Node1 and \p Node2 are "equivalent" in terms of CFG: namely
  /// if \p Node1 is reached/accessed anytime \p Node2 is reached/Accessed and
  /// vice versa. This allows placing/accessing the nodes together in the same
  /// location.
  /// Returns false if there may exist a scenario/path in which Node1 is
  /// reached/accessed and Node2 isn't, or the other way around.
  /// Note: In the presence of complicated unstructured code (containing
  /// gotos/labels) this function will conservatively return false.
  static bool canAccessTogether(const HLNode *Node1, const HLNode *Node2,
                                HIRLoopStatistics *HLS);

  /// Returns true if Parent contains Node. IncludePrePostHdr indicates whether
  /// loop should be considered to contain preheader/postexit nodes.
  static bool contains(const HLNode *Parent, const HLNode *Node,
                       bool IncludePrePostHdr = false);

  /// Gathers the innermost loops across regions and stores them into the loop
  /// vector. If Node is not specified, it will search for all Regions
  void gatherInnermostLoops(SmallVectorImpl<HLLoop *> &Loops,
                            HLNode *Node = nullptr) {

    LoopLevelVisitor<HLLoop *, VisitKind::Innermost> LoopVisit(*this, Loops);
    if (Node) {
      visit(LoopVisit, Node);
    } else {
      visitAll(LoopVisit);
    }
  }

  void gatherInnermostLoops(SmallVectorImpl<const HLLoop *> &Loops,
                            const HLNode *Node = nullptr) {

    LoopLevelVisitor<const HLLoop *, VisitKind::Innermost> LoopVisit(*this,
                                                                     Loops);
    if (Node) {
      visit(LoopVisit, Node);
    } else {
      visitAll(LoopVisit);
    }
  }

  /// \brief Gathers the outermost loops (or highest level loops with Level 1)
  /// across regions and stores them into the loop vector.
  template <typename T> void gatherOutermostLoops(SmallVectorImpl<T> &Loops) {
    // Level 1 denotes outermost loops
    LoopLevelVisitor<T, VisitKind::Level> LoopVisit(*this, Loops, 1);
    visitAll(LoopVisit);
  }

  /// Gathers loops inside the Node with specified Level and stores them in the
  /// Loops vector.
  template <typename T>
  void gatherLoopsWithLevel(HLNode *Node, SmallVectorImpl<T> &Loops,
                            unsigned Level) {
    assert(Node && " Node is null.");
    HLLoop *Loop = dyn_cast<HLLoop>(Node);
    (void)Loop;
    assert((!Loop || !Loop->isInnermost()) &&
           " Gathering loops inside innermost loop.");
    assert(CanonExprUtils::isValidLoopLevel(Level) &&
           " Level is out of range.");
    LoopLevelVisitor<T, VisitKind::Level> LoopVisit(*this, Loops, Level);
    visit(LoopVisit, Node);
  }

  /// Constant Node version of gatherLoopsWithLevel.
  template <typename T>
  void gatherLoopsWithLevel(const HLNode *Node, SmallVectorImpl<T> &Loops,
                            unsigned Level) {
    static_assert(std::is_const<typename std::remove_pointer<T>::type>::value,
                  "Type of SmallVector parameter should be const HLLoop *.");
    gatherLoopsWithLevel(const_cast<HLNode *>(Node), Loops, Level);
  }

  /// Gathers all the loops across regions and stores them in the Loops vector.
  template <typename T> void gatherAllLoops(SmallVectorImpl<T> &Loops) {
    LoopLevelVisitor<T, VisitKind::All> LoopVisit(*this, Loops);
    visitAll(LoopVisit);
  }

  /// Gathers all the loops inside the Node and stores them in the Loops vector.
  template <typename T>
  void gatherAllLoops(HLNode *Node, SmallVectorImpl<T> &Loops) {
    assert(Node && " Node is null.");
    LoopLevelVisitor<T, VisitKind::All> LoopVisit(*this, Loops);
    visit(LoopVisit, Node);
  }

  /// Constant Node version of gatherAllLoops.
  template <typename T>
  void gatherAllLoops(const HLNode *Node, SmallVectorImpl<T> &Loops) {
    static_assert(std::is_const<typename std::remove_pointer<T>::type>::value,
                  "Type of SmallVector parameter should be const HLLoop *.");
    gatherAllLoops(const_cast<HLNode *>(Node), Loops);
  }

  /// Returns true if Loop is a perfect Loop nest. Also returns the
  /// innermost loop.
  /// Asserts if incoming loop is innermost.
  /// TODO: AllowPrePostHdr is unused, remove it?
  static bool isPerfectLoopNest(const HLLoop *Loop,
                                const HLLoop **InnermostLoop = nullptr,
                                bool AllowPrePostHdr = false,
                                bool AllowTriangularLoop = false,
                                bool AllowNearPerfect = false,
                                bool *IsNearPerfect = nullptr);

  /// Any memref with non-unit stride?
  /// Will take innermost loop for now.
  /// Used mostly for blocking / interchange.
  static bool hasNonUnitStrideRefs(const HLLoop *Loop);

  /// Find node receiving the load
  /// e.g.   t0 = a[i] ;
  ///         ...
  ///        t1 = t0
  ///  returns t1 = t0
  HLInst *findForwardSubInst(const DDRef *LRef,
                             SmallVectorImpl<HLInst *> &ForwardSubInsts);

  /// Returns the lowest common ancestor loop of Lp1 and Lp2. Returns null if
  /// there is no such parent loop.
  static const HLLoop *getLowestCommonAncestorLoop(const HLLoop *Lp1,
                                                   const HLLoop *Lp2);
  static HLLoop *getLowestCommonAncestorLoop(HLLoop *Lp1, HLLoop *Lp2);

  /// Returns true if the minimum value of blob can be evaluated. Returns the
  /// minimum value in \p Val.
  bool getMinBlobValue(unsigned BlobIdx, const HLNode *ParentNode,
                       int64_t &Val);

  /// Returns true if the maximum value of blob can be evaluated. Returns the
  /// maximum value in \p Val.
  bool getMaxBlobValue(unsigned BlobIdx, const HLNode *ParentNode,
                       int64_t &Val);

  /// Returns true if blob is known to be non-zero.
  bool isKnownNonZero(unsigned BlobIdx, const HLNode *ParentNode);

  /// Returns true if blob is known to be non-positive. Returns max value in \p
  /// MaxVal.
  bool isKnownNonPositive(unsigned BlobIdx, const HLNode *ParentNode,
                          int64_t &MaxVal);

  /// Returns true if blob is known to be non-negative. Returns min value in \p
  /// MinVal.
  bool isKnownNonNegative(unsigned BlobIdx, const HLNode *ParentNode,
                          int64_t &MinVal);

  /// Returns true if blob is known to be negative. Returns max value in \p
  /// MaxVal.
  bool isKnownNegative(unsigned BlobIdx, const HLNode *ParentNode,
                       int64_t &MaxVal);

  /// Returns true if blob is known to be positive. Returns min value in \p
  /// MinVal.
  bool isKnownPositive(unsigned BlobIdx, const HLNode *ParentNode,
                       int64_t &MinVal);

  /// Returns true if blob is known to be positive or negative. Returns min/max
  /// value in \p MinMAxVal.
  bool isKnownPositiveOrNegative(unsigned BlobIdx, const HLNode *ParentNode,
                                 int64_t &MinMaxVal);

  /// Returns true if exact minimum value of \p CE can be evaluated. Exact value
  /// means that there is no approximation due to presence of blobs. Returns the
  /// minimum value in \p Val.
  bool getExactMinValue(const CanonExpr *CE, const HLNode *ParentNode,
                        int64_t &Val);

  /// Returns true if exact maximum value of \p CE can be evaluated. Exact value
  /// means that there is no approximation due to presence of blobs. Returns the
  /// maximum value in \p Val.
  bool getExactMaxValue(const CanonExpr *CE, const HLNode *ParentNode,
                        int64_t &Val);

  /// Returns true if the predicate can be evaluated. The predicate result value
  /// will be stored into the /p Result.
  static bool isKnownPredicate(const CanonExpr *LHS, PredicateTy Pred,
                               const CanonExpr *RHS, bool *Result);

  /// Returns true if minimum value of \p CE can be evaluated. Returns the
  /// minimum value in \p Val.
  bool getMinValue(const CanonExpr *CE, const HLNode *ParentNode, int64_t &Val);

  /// Returns true if maximum value of \p CE can be evaluated. Returns the
  /// maximum value in \p Val.
  bool getMaxValue(const CanonExpr *CE, const HLNode *ParentNode, int64_t &Val);

  /// Returns true if non-zero.
  bool isKnownNonZero(const CanonExpr *CE, const HLNode *ParentNode = nullptr);

  /// Returns true if non-positive.
  bool isKnownNonPositive(const CanonExpr *CE,
                          const HLNode *ParentNode = nullptr);

  /// Returns true if non-negative.
  bool isKnownNonNegative(const CanonExpr *CE,
                          const HLNode *ParentNode = nullptr);

  /// Returns true if negative.
  bool isKnownNegative(const CanonExpr *CE, const HLNode *ParentNode = nullptr);

  /// Returns true if positive.
  bool isKnownPositive(const CanonExpr *CE, const HLNode *ParentNode = nullptr);

  /// Returns true if positive or negative.
  bool isKnownPositiveOrNegative(const CanonExpr *CE,
                                 const HLNode *ParentNode = nullptr);

  // Returns true if both HLIf nodes are equal.
  static bool areEqual(const HLIf *NodeA, const HLIf *NodeB);

  // Replaces \p If with its *then* or *else* body.
  // Returns iterator range [FirstBodyChild, LastBodyChild) in the destination
  // container.
  static NodeRangeTy replaceNodeWithBody(HLIf *If, bool ThenBody);

  // Replaces \p Switch with the body of the case with \p CaseNum.
  // Zero \p CaseNum corresponds to the default case.
  // Returns iterator range [FirstCaseChild, LastCaseChild) in the destination
  // container.
  static NodeRangeTy replaceNodeWithBody(HLSwitch *Switch, unsigned CaseNum);

  /// Recursively traverse the HIR from the /p Node and remove empty HLLoops and
  /// empty HLIfs.
  ///
  /// Note: This function is placed here because the Framework uses it to
  /// get rid of incoming empty HLIfs.
  static bool removeEmptyNodes(HLNode *Node,
                               bool RemoveEmptyParentNodes = true);

  /// Recursively traverse the HIR range [\p Begin, \p End) and remove empty
  /// HLLoops and empty HLIfs.
  static bool removeEmptyNodesRange(HLContainerTy::iterator Begin,
                                    HLContainerTy::iterator End,
                                    bool RemoveEmptyParentNodes = true);

  /// Removes: 1) HLIfs that always evaluates as either true or false;
  ///          2) HLLoops those trip count is constant and equals zero.
  ///          3) Empty HLNodes
  /// Returns true whenever nodes were removed.
  /// If \p RemoveEmptyParentNodes is true, the utility also removes parent
  /// nodes if they become empty.
  /// The utility invalidates analysis for the changed loops and regions.
  static bool removeRedundantNodes(HLNode *Node,
                                   bool RemoveEmptyParentNodes = true);
  static bool removeRedundantNodesRange(HLContainerTy::iterator Begin,
                                        HLContainerTy::iterator End,
                                        bool RemoveEmptyParentNodes = true);
};

} // End namespace loopopt

} // End namespace llvm

#endif
