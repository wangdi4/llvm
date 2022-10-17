//===- IntelVPBasicBlock.h ------------------------------------------------===//
/* INTEL_CUSTOMIZATION */
/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2021 Intel Corporation
 *
 * This software and the related documents are Intel copyrighted materials, and
 * your use of them is governed by the express license under which they were
 * provided to you ("License"). Unless the License provides otherwise, you may not
 * use, modify, copy, publish, distribute, disclose or transmit this software or
 * the related documents without Intel's prior written permission.
 *
 * This software and the related documents are provided as is, with no express
 * or implied warranties, other than those that are expressly stated in the
 * License.
 */
/* end INTEL_CUSTOMIZATION */
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPBASICBLOCK_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPBASICBLOCK_H

#include "IntelVPlanValue.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/ilist.h"
#include "llvm/ADT/ilist_node.h"
#include "llvm/ADT/simple_ilist.h"
#include "llvm/Analysis/LoopInfoImpl.h"
#include "llvm/Support/BlockFrequency.h"

namespace llvm {

namespace vpo {
class VPBasicBlock;
class VPInstruction;
} // namespace vpo

template <> struct ilist_traits<vpo::VPInstruction> {
private:
  friend class vpo::VPBasicBlock; // Set by the owning VPBasicBlock.

  using instr_iterator =
      simple_ilist<vpo::VPInstruction, ilist_sentinel_tracking<true>>::iterator;

public:
  void addNodeToList(vpo::VPInstruction *VPInst);
  void removeNodeFromList(vpo::VPInstruction *VPInst);
  void transferNodesFromList(ilist_traits &FromList, instr_iterator First,
                             instr_iterator Last);
  void deleteNode(vpo::VPInstruction *VPInst);
};

namespace vpo {

class VPlan;
class VPlanDivergenceAnalysis;
class VPLoopInfo;
class VPValue;
class VPBranchInst;
class VPDominatorTree;
class VPOCodeGenHIR;
class VPPostDominatorTree;
class VPPHINode;
struct TripCountInfo;

// llvm::mapped_itrator has limited support of functions returning by value.
// TODO: Replace VPSuccIterator with llvm::mapped_iterator, once it is fixed.
template <typename ItTy, typename FuncTy,
          typename FuncReturnTy =
              decltype(std::declval<FuncTy>()(*std::declval<ItTy>()))>
class VPSuccIterator
    : public iterator_adaptor_base<
          VPSuccIterator<ItTy, FuncTy>, ItTy,
          typename std::iterator_traits<ItTy>::iterator_category,
          typename std::remove_reference<FuncReturnTy>::type,
          typename std::iterator_traits<ItTy>::difference_type,
          typename std::iterator_traits<ItTy>::pointer,
          typename std::remove_reference<FuncReturnTy>::type> {
public:
  VPSuccIterator(ItTy U, FuncTy F)
      : VPSuccIterator::iterator_adaptor_base(std::move(U)), F(std::move(F)) {}
  FuncReturnTy operator*() const { return F(*this->I); }

private:
  FuncTy F;
};

template <class ItTy, class FuncTy>
inline VPSuccIterator<ItTy, FuncTy> createVPSuccIterator(ItTy I, FuncTy F) {
  return VPSuccIterator<ItTy, FuncTy>(std::move(I), std::move(F));
}

template <class ContainerTy, class FuncTy>
auto createVPSuccRange(ContainerTy &&C, FuncTy F) {
  return make_range(createVPSuccIterator(C.begin(), F),
                    createVPSuccIterator(C.end(), F));
}

/// VPBasicBlock represents a sequence of instructions that will appear
/// consecutively in a basic block of the vectorized version.
/// Like the IR BasicBlock a VPBasicBlock models its control-flow edges
/// through a Terminator branch.
class VPBasicBlock
    : public ilist_node_with_parent<VPBasicBlock, VPlan,
                                    ilist_sentinel_tracking<true>>,
      public VPValue {
  friend class VPBlockUtils;

public:
  using VPInstructionListTy =
      ilist<VPInstruction, ilist_sentinel_tracking<true>>;
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  using VPValue::printAsOperand;
#endif // !NDEBUG || LLVM_ENABLE_DUMP

private:
  /// The list of VPInstructions, held in order of instructions to generate.
  VPInstructionListTy Instructions;

  // The parent of each VPBasicBlock is its Plan.
  VPlan *Parent = nullptr;

  /// Current block predicate - null if the block does not need a predicate.
  VPInstruction *BlockPredicate = nullptr;

  BasicBlock *OriginalBB = nullptr;

  // TODO: Not sure what other types of loop metadata we'd need. Most probably,
  // we need some abstraction on top of TripCountInfo (and maybe that struct
  // itself should be split in some way). The idea about this field is to have
  // something similar to LLVM IR's loop metadata on the backedge branch
  // instruction, so it will be filled for the latches only.
  std::unique_ptr<TripCountInfo> TCInfo;

  BlockFrequency BlockFreq;

  using AsVPPHINodeFunc = std::function<VPPHINode &(VPInstruction &)>;
  using AsVPPHINodeFuncConst =
      std::function<const VPPHINode &(const VPInstruction &)>;

public:
  /// Instruction iterators...
  using iterator = VPInstructionListTy::iterator;
  using const_iterator = VPInstructionListTy::const_iterator;
  using reverse_iterator = VPInstructionListTy::reverse_iterator;
  using const_reverse_iterator = VPInstructionListTy::const_reverse_iterator;

  using phi_iterator_range =
      iterator_range<mapped_iterator<iterator, AsVPPHINodeFunc, VPPHINode &>>;
  using phi_const_iterator_range = iterator_range<
      mapped_iterator<const_iterator, AsVPPHINodeFuncConst, const VPPHINode &>>;
  //===--------------------------------------------------------------------===//
  /// Instruction iterator methods
  ///
  inline iterator begin() { return Instructions.begin(); }
  inline const_iterator begin() const { return Instructions.begin(); }
  inline iterator end() { return Instructions.end(); }
  inline const_iterator end() const { return Instructions.end(); }
  iterator terminator();
  const_iterator terminator() const;

  inline reverse_iterator rbegin() { return Instructions.rbegin(); }
  inline const_reverse_iterator rbegin() const { return Instructions.rbegin(); }
  inline reverse_iterator rend() { return Instructions.rend(); }
  inline const_reverse_iterator rend() const { return Instructions.rend(); }

  inline size_t size() const { return Instructions.size(); }
  inline bool empty() const { return Instructions.empty(); }
  inline const VPInstruction &front() const { return Instructions.front(); }
  inline VPInstruction &front() { return Instructions.front(); }
  inline const VPInstruction &back() const { return Instructions.back(); }
  inline VPInstruction &back() { return Instructions.back(); }

  /// Returns a pointer to a member of the instruction list.
  static VPInstructionListTy VPBasicBlock::*getSublistAccess(VPInstruction *) {
    return &VPBasicBlock::Instructions;
  }

  static bool isDefaultName(StringRef &N) {
    return N.find_insensitive("bb") != StringRef::npos;
  }

  /// Replace \p OldSuccessor by \p NewSuccessor in Block's successor list.
  /// \p NewSuccessor will be inserted in the same position as \p OldSuccessor.
  void replaceSuccessor(VPBasicBlock *OldSuccessor, VPBasicBlock *NewSuccessor);

  VPBasicBlock *getSuccessor(unsigned idx) const;

  /// Set successor for \p idx position.
  void setSuccessor(unsigned idx, VPBasicBlock *NewSucc);

  auto getSuccessors() const {
    // std::function is required here to compile the code with MSVC.
    std::function<VPBasicBlock *(VPValue *)> F = [](VPValue *V) {
      return cast<VPBasicBlock>(V);
    };
    return createVPSuccRange(successors(), F);
  }

  // See comment about workaround below.
  static bool isBranchInst(const VPUser *U) { return isa<VPBranchInst>(U); }

  // Return iterator range of VPBasicBlock predecessors.
  // In make_filter_range, the static function is used instead of a lambda to
  // workaround a build error with Microsoft VS.
  auto getPredecessors() const {
    return map_range(make_filter_range(users(), isBranchInst), getVPUserParent);
  }

 #if INTEL_CUSTOMIZATION
  size_t getNumSuccessors() const;

  size_t getNumPredecessors() const {
    return llvm::count_if(users(),
                          [](const VPUser *U) { return isa<VPBranchInst>(U); });
  }
#endif // INTEL_CUSTOMIZATION

  /// \Return the successor of this VPBasicBlock if it has a single successor.
  /// Otherwise return a null pointer.
  VPBasicBlock *getSingleSuccessor() const;

  /// \return the predecessor of this VPBasicBlock if it has a single
  /// predecessor. Otherwise return a null pointer.
  VPBasicBlock *getSinglePredecessor() const;

  /// If this basic block has a unique predecessor block, return the block,
  /// otherwise return a null pointer. Note that unique predecessor doesn't
  /// mean single edge, there can be multiple edges from the unique predecessor
  /// to this block (for example a switch statement with multiple cases having
  /// the same destination).
  const VPBasicBlock *getUniquePredecessor() const;

  void insertBefore(VPBasicBlock *MovePos);
  void insertAfter(VPBasicBlock *MovePos);

  phi_iterator_range getVPPhis();
  phi_const_iterator_range getVPPhis() const;

  VPBasicBlock(const Twine &Name, VPlan *Plan);

  VPBasicBlock(const Twine &Name, LLVMContext *C);

  ~VPBasicBlock();

  /// Drop an existing terminator (if there is one) and append a new terminator
  /// instruction without successors.
  void setTerminator();

  /// Drop an existing terminator (if there is one) and append a new terminator
  /// instruction with a single successor.
  void setTerminator(VPBasicBlock *Succ);

  /// Drop an existing terminator (if there is one) and append a new terminator
  /// instruction with two successors and a condition bit.
  void setTerminator(VPBasicBlock *IfTrue, VPBasicBlock *IfFalse,
                     VPValue *Cond);

  // ilist should have access to VPBasicBlock node.
  friend struct ilist_traits<VPBasicBlock>;

  VPlan *getParent() { return Parent; }
  const VPlan *getParent() const { return Parent; }

  void setParent(VPlan *P) { Parent = P; }

  VPValue *getPredicate();
  const VPValue *getPredicate() const;
  VPInstruction *getBlockPredicate() { return BlockPredicate; }
  const VPInstruction *getBlockPredicate() const { return BlockPredicate; }
  void setBlockPredicate(VPInstruction *BlockPredicate);

  /// \Return the condition bit selecting the successor.
  VPValue *getCondBit();

  const VPValue *getCondBit() const;

  void setCondBit(VPValue *CB);

  // The routine is used by BlockT *LoopBase<BlockT, LoopT>::getLoopPreheader().
  // There are currently no constraints that prevent an instruction to be
  // hoisted into a VPBlockBase and we don't support exception handling which
  // would have prevented such hoisting.
  bool isLegalToHoistInto() { return true; }

  void insert(VPInstruction *Instruction, iterator InsertPt);

  /// Augment the existing instructions of a VPBasicBlock with an additional
  /// \p Instruction as the last Instruction (right before the terminator).
  void appendInstruction(VPInstruction *Instruction);

  /// Add \p Instruction after \p After. If \p After is null, \p Instruction
  /// will be inserted as the first instruction.
  void addInstructionAfter(VPInstruction *Instruction, VPInstruction *After);

  /// Augment the existing instructions of a VPBasicBlock with an additional
  /// \p Instruction at a position given by an existing instruction \p Before.
  /// \p If Before is null, \p Instruction is appended as the last instruction
  /// (right before the terminator instruction).
  void addInstruction(VPInstruction *Instruction,
                      VPInstruction *Before = nullptr);

  /// Remove the instruction from VPBasicBlock's instructions.
  void removeInstruction(VPInstruction *Instruction) {
    Instructions.remove(Instruction);
  }

  /// Unlinks a VPInstruction from Instructions list and adds the erased
  /// instruction in UnlinkedVPInsns vector. This happens because there was the
  /// following problem: a VPInstruction was erased, but its pointer remained in
  /// some data structures. There is a chance to create a new instruction with
  /// the same pointer as the one that was erased. In this case, the data
  /// structures might have wrong values. For this reason, we completely erase
  /// VPInstuctions only at the end of VPlan.
  void eraseInstruction(VPInstruction *Instruction);

  /// This drops all operand uses from instructions within this block, which is
  /// an essential step in breaking cyclic dependences between references when
  /// they are to be deleted.
  void dropAllReferences();

  /// The method which generates all new IR instructions that correspond to
  /// this VPBasicBlock in the vectorized version, thereby "executing" the
  /// VPlan.
  void execute(struct VPTransformState *State);
#if INTEL_CUSTOMIZATION
  void executeHIR(VPOCodeGenHIR *CG);
#endif

  /// Retrieve the list of VPInstructions that belong to this VPBasicBlock.
  const VPInstructionListTy &getInstructions() const { return Instructions; }
  VPInstructionListTy &getInstructions() { return Instructions; }

  /// Returns a range that iterates over non predicator related instructions
  /// in the VPBasicBlock.
  iterator_range<const_iterator> getNonPredicateInstructions() const;

  static bool classof(const VPValue *V) {
    return V->getVPValueID() == VPBasicBlockSC;
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void printAsOperand(raw_ostream &OS, bool PrintType) const {
    (void)PrintType;
    OS << getName();
  }

  void print(raw_ostream &OS, unsigned Indent,
             const Twine &NamePrefix = "") const;

  void print(raw_ostream &OS) const override { print(OS, 0); };
#endif // !NDEBUG || LLVM_ENABLE_DUMP
  void setOriginalBB(BasicBlock *BB) { OriginalBB = BB; }
  BasicBlock *getOriginalBB() const { return OriginalBB; }

  TripCountInfo *getTripCountInfo() { return TCInfo.get(); }
  const TripCountInfo *getTripCountInfo() const { return TCInfo.get(); }
  void setTripCountInfo(std::unique_ptr<TripCountInfo> TCInfo) {
    assert(!this->TCInfo && "Trip count info alread set!");
    this->TCInfo = std::move(TCInfo);
  }
  void moveTripCountInfoFrom(VPBasicBlock *OtherBB) {
    TCInfo = std::move(OtherBB->TCInfo);
  }
  void copyTripCountInfoFrom(VPBasicBlock *OtherBB) {
    if (OtherBB->TCInfo)
      this->TCInfo = std::make_unique<TripCountInfo>(*OtherBB->TCInfo);
    else
      this->TCInfo = nullptr;
  }

  VPBranchInst *getTerminator();
  const VPBranchInst *getTerminator() const;

  BlockFrequency getFrequency() const { return BlockFreq; }
  void setFrequency(BlockFrequency Freq) { BlockFreq = Freq; }

private:
  /// Create an IR BasicBlock to hold the instructions vectorized from this
  /// VPBasicBlock, and return it. Update the CFGState accordingly.
  BasicBlock *createEmptyBasicBlock(VPTransformState *State);

  /// Split this block before the VPInstruction pointer by the \p I, or before
  /// the implicit [conditional] jump represented by the successors.
  ///
  /// This routine also updates the CFG accordingly, i.e. moves
  /// successors/condition bits to the newly created block. If \p I points to a
  /// VPPHINode, the split happens after the last VPPHINode in the current block
  /// to preserve correctness.
  ///
  /// Block predicate instruction is NOT cloned, if original block contained,
  /// only one of the blocks will have it.
  ///
  /// VPHINodes in the successors of this block are also updated. In case of
  /// Blends, incoming blocks are updated in such a way that the block
  /// containing the block-predicate instruction after the split is used.
  VPBasicBlock *splitBlock(iterator I, const Twine &NewBBName = "");

  /// Worker for setTerminator() methods
  template <class... Args> void setTerminatorImpl(Args &&... args);

  VPUser::const_operand_range successors() const;

  static VPBasicBlock *getVPUserParent(VPUser *User);
};

/// Class that provides utilities for VPBasicBlocks in VPlan.
class VPBlockUtils {
public:
  VPBlockUtils() = delete;

  /// Insert NewBlock in Plan.
  static void insertBlockBefore(VPBasicBlock *NewBB, VPBasicBlock *BlockPtr);

  /// Insert NewBlock in Plan. If BlockPtr has more that one successors, its
  /// CondBit is propagated to NewBlock.
  static void insertBlockAfter(VPBasicBlock *NewBB, VPBasicBlock *BlockPtr);

  /// Returns true if the edge \p FromBlock -> \p ToBlock is a back-edge.
  static bool isBackEdge(const VPBasicBlock *FromBB, const VPBasicBlock *ToBB,
                         const VPLoopInfo *VPLI);

  /// Returns true if \p Block is a loop latch
  static bool blockIsLoopLatch(const VPBasicBlock *BB,
                               const VPLoopInfo *VPLInfo);

public:
  static VPBasicBlock *splitBlock(VPBasicBlock *BB,
                                  VPBasicBlock::iterator BeforeIt,
                                  VPLoopInfo *VPLInfo,
                                  VPDominatorTree *DomTree = nullptr,
                                  VPPostDominatorTree *PostDomTree = nullptr);
  /// Same as splitBlock, but the block containing instructions after \p
  /// BeforeIt gets the original pointer/name.
  ///
  /// In other words, Adjust \p BeforeIt not to point to the phi/blend if
  /// it happens to be so, create a new block in place of the \p BB, make it a
  /// single predecessor of \p BB and move all the instructions in range
  /// [BB->begin(), BeforeIt) into the newly created block.
  static VPBasicBlock *
  splitBlockHead(VPBasicBlock *BB, VPBasicBlock::iterator BeforeIt,
                 VPLoopInfo *VPLInfo, const Twine &Name = "",
                 VPDominatorTree *DomTree = nullptr,
                 VPPostDominatorTree *PostDomTree = nullptr);

  static VPBasicBlock *
  splitBlockBegin(VPBasicBlock *BB, VPLoopInfo *VPLInfo,
                  VPDominatorTree *DomTree = nullptr,
                  VPPostDominatorTree *PostDomTree = nullptr);
  static VPBasicBlock *
  splitBlockEnd(VPBasicBlock *BB, VPLoopInfo *VPLInfo,
                VPDominatorTree *DomTree = nullptr,
                VPPostDominatorTree *PostDomTree = nullptr);
  static VPBasicBlock *
  splitBlockAtPredicate(VPBasicBlock *BB, VPLoopInfo *VPLInfo,
                        VPDominatorTree *DomTree = nullptr,
                        VPPostDominatorTree *PostDomTree = nullptr);
  static VPBasicBlock *splitEdge(VPBasicBlock *From, VPBasicBlock *To,
                                 const Twine &Name, VPLoopInfo *VPLInfo,
                                 VPDominatorTree *DomTree = nullptr,
                                 VPPostDominatorTree *PostDomTree = nullptr);
};
} // namespace vpo
//===----------------------------------------------------------------------===//
// GraphTraits specializations for VPBasicBlock                               //
//===----------------------------------------------------------------------===//

// The following template specializations are implemented to support GraphTraits
// for VPBasicBlocks.
template <> struct GraphTraits<vpo::VPBasicBlock *> {
  using NodeRef = vpo::VPBasicBlock *;
  using ChildRangeType = decltype(std::declval<NodeRef>()->getSuccessors());
  using ChildIteratorType = decltype(std::declval<ChildRangeType>().begin());

  static NodeRef getEntryNode(NodeRef N) { return N; }

  static inline ChildIteratorType child_begin(NodeRef N) {
    return N->getSuccessors().begin();
  }

  static inline ChildIteratorType child_end(NodeRef N) {
    return N->getSuccessors().end();
  }
};

// This specialization is for the ChildrenGetterTy from
// GenericIteratedDominanceFrontier.h. Clang's GraphTraits for clang::CFGBlock
// do the same trick.
// TODO: Consider fixing GenericIteratedDominanceFrontier.h during upstreaming
// instead.
template <>
struct GraphTraits<vpo::VPBasicBlock>
    : public GraphTraits<vpo::VPBasicBlock *> {};

// GraphTraits specialization for VPBasicBlocks using constant iterator.
template <> struct GraphTraits<const vpo::VPBasicBlock *> {
  using NodeRef = const vpo::VPBasicBlock *;
  using ChildRangeType = decltype(std::declval<NodeRef>()->getSuccessors());
  using ChildIteratorType = decltype(std::declval<ChildRangeType>().begin());

  static NodeRef getEntryNode(NodeRef N) { return N; }

  static inline ChildIteratorType child_begin(NodeRef N) {
    return N->getSuccessors().begin();
  }

  static inline ChildIteratorType child_end(NodeRef N) {
    return N->getSuccessors().end();
  }
};

// GraphTraits specialization for VPBasicBlocks using inverse iterator.
template <> struct GraphTraits<Inverse<vpo::VPBasicBlock *>> {
  using NodeRef = vpo::VPBasicBlock *;
  using ChildRangeType = decltype(std::declval<NodeRef>()->getPredecessors());
  using ChildIteratorType = decltype(std::declval<ChildRangeType>().begin());

  static NodeRef getEntryNode(Inverse<NodeRef> N) { return N.Graph; }

  static inline ChildIteratorType child_begin(NodeRef N) {
    return N->getPredecessors().begin();
  }

  static inline ChildIteratorType child_end(NodeRef N) {
    return N->getPredecessors().end();
  }
};
} // namespace llvm
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPBASICBLOCK_H
