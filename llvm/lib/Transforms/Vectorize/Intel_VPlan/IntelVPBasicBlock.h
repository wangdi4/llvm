//===- IntelVPBasicBlock.h ------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPBASICBLOCK_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPBASICBLOCK_H

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/ilist.h"
#include "llvm/ADT/ilist_node.h"
#include "llvm/Analysis/LoopInfoImpl.h"

namespace llvm {
namespace vpo {

class VPlan;
class VPlanDivergenceAnalysis;
class VPLoopInfo;
class VPValue;
class VPDominatorTree;
class VPInstruction;
class VPOCodeGenHIR;
class VPPostDominatorTree;
class VPPHINode;
struct TripCountInfo;

/// VPBasicBlock represents a sequence of instructions that will appear
/// consecutively in a basic block of the vectorized version. The VPBasicBlock
/// takes care of the control-flow relations with other VPBasicBlock's. Note
/// that in contrast to the IR BasicBlock, a VPBasicBlock models its
/// control-flow edges with successor and predecessor VPBasicBlocks directly,
/// rather than through a Terminator branch or through predecessor branches that
/// "use" the VPBasicBlock.
class VPBasicBlock {
  friend class VPBlockUtils;

public:
  using VPInstructionListTy = iplist<VPInstruction>;

private:
  /// The list of VPInstructions, held in order of instructions to generate.
  VPInstructionListTy Instructions;

  std::string Name;

  // The parent of each VPBasicBlock is its Plan.
  VPlan *Parent = nullptr;

  /// List of predecessor blocks.
  SmallVector<VPBasicBlock *, 2> Predecessors;

  /// List of successor blocks.
  SmallVector<VPBasicBlock *, 2> Successors;

  /// Successor selector, null for zero or single successor blocks.
  VPValue *CondBit = nullptr;

  /// Current block predicate - null if the block does not need a predicate.
  VPValue *Predicate = nullptr;

public:
  /// Instruction iterators...
  using iterator = VPInstructionListTy::iterator;
  using const_iterator = VPInstructionListTy::const_iterator;
  using reverse_iterator = VPInstructionListTy::reverse_iterator;
  using const_reverse_iterator = VPInstructionListTy::const_reverse_iterator;

  //===--------------------------------------------------------------------===//
  /// Instruction iterator methods
  ///
  inline iterator begin() { return Instructions.begin(); }
  inline const_iterator begin() const { return Instructions.begin(); }
  inline iterator end() { return Instructions.end(); }
  inline const_iterator end() const { return Instructions.end(); }

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

  // Iterators and types to access Successors of a VPBasicBlock
  using SuccType = SmallVector<VPBasicBlock *, 2>;
  using succ_iterator = SuccType::iterator;
  using succ_const_iterator = SuccType::const_iterator;
  using succ_reverse_iterator = SuccType::reverse_iterator;
  using succ_const_reverse_iterator = SuccType::const_reverse_iterator;

  inline succ_iterator succ_begin() { return Successors.begin(); }
  inline succ_iterator succ_end() { return Successors.end(); }
  inline succ_const_iterator succ_begin() const { return Successors.begin(); }
  inline succ_const_iterator succ_end() const { return Successors.end(); }
  inline succ_reverse_iterator succ_rbegin() { return Successors.rbegin(); }
  inline succ_reverse_iterator succ_rend() { return Successors.rend(); }
  inline succ_const_reverse_iterator succ_rbegin() const {
    return Successors.rbegin();
  }
  inline succ_const_reverse_iterator succ_rend() const {
    return Successors.rend();
  }

  /// Add \p Successor as the last successor to this block.
  void appendSuccessor(VPBasicBlock *Successor);

  /// Add \p Predecessor as the last predecessor to this block.
  void appendPredecessor(VPBasicBlock *Predecessor);

  /// Remove \p Predecessor from the predecessors of this block.
  void removePredecessor(VPBasicBlock *Predecessor);

  /// Remove \p Successor from the successors of this block.
  void removeSuccessor(VPBasicBlock *Successor);

  const SmallVectorImpl<VPBasicBlock *> &getSuccessors() const {
    return Successors;
  }

  const SmallVectorImpl<VPBasicBlock *> &getPredecessors() const {
    return Predecessors;
  }

  SmallVectorImpl<VPBasicBlock *> &getSuccessors() { return Successors; }

  SmallVectorImpl<VPBasicBlock *> &getPredecessors() { return Predecessors; }

#if INTEL_CUSTOMIZATION
  size_t getNumSuccessors() const { return Successors.size(); }

  size_t getNumPredecessors() const { return Predecessors.size(); }
#endif // INTEL_CUSTOMIZATION

  /// \Return the successor of this VPBasicBlock if it has a single successor.
  /// Otherwise return a null pointer.
  VPBasicBlock *getSingleSuccessor() const {
    return (Successors.size() == 1 ? *Successors.begin() : nullptr);
  }

  /// \return the predecessor of this VPBasicBlock if it has a single
  /// predecessor. Otherwise return a null pointer.
  VPBasicBlock *getSinglePredecessor() const {
    return (Predecessors.size() == 1 ? *Predecessors.begin() : nullptr);
  }

  /// If this basic block has a unique predecessor block, return the block,
  /// otherwise return a null pointer. Note that unique predecessor doesn't
  /// mean single edge, there can be multiple edges from the unique predecessor
  /// to this block (for example a switch statement with multiple cases having
  /// the same destination).
  const VPBasicBlock *getUniquePredecessor() const;

  /// connectBlocks should be used instead of this function when possible.
  /// Set a given VPBasicBlock \p Successor as the single successor of this
  /// VPBasicBlock. This VPBasicBlock is not added as predecessor of \p
  /// Successor. This VPBasicBlock must have no successors.
  void setOneSuccessor(VPBasicBlock *Successor);

  /// connectBlocks should be used instead of this function when possible.
  /// Set two given VPBasicBlocks \p IfTrue and \p IfFalse to be the two
  /// successors of this VPBasicBlock. This VPBasicBlock is not added as
  /// predecessor of \p IfTrue or \p IfFalse. This VPBasicBlock must have no
  /// successors. \p ConditionV is set as successor selector.
  void setTwoSuccessors(VPValue *ConditionV, VPBasicBlock *IfTrue,
                        VPBasicBlock *IfFalse);

  /// Set each VPBasicBlock in \p NewPreds as predecessor of this VPBasicBlock.
  /// This VPBasicBlock must have no predecessors. This VPBasicBlock is not
  /// added as successor of any VPBasicBlock in \p NewPreds.
  void setPredecessors(ArrayRef<VPBasicBlock *> NewPreds);

  /// Remove all the predecessor of this block.
  void clearPredecessors() { Predecessors.clear(); }

  /// Remove all the successors of this block and set its condition bit to null.
  void clearSuccessors() {
    Successors.clear();
    CondBit = nullptr;
  }

  auto getVPPhis() {
    auto AsVPPHINode = [](VPInstruction &Instruction) -> VPPHINode & {
      return cast<VPPHINode>(Instruction);
    };

    // If the block is empty or if it has no PHIs, return null range
    if (empty() || !isa<VPPHINode>(begin()))
      return map_range(make_range(end(), end()), AsVPPHINode);

    // Increment iterator till a non PHI VPInstruction is found
    iterator It = begin();
    while (It != end() && isa<VPPHINode>(It))
      ++It;

    return map_range(make_range(begin(), It), AsVPPHINode);
  }

  auto getVPPhis() const {
    auto AsVPPHINode =
        [](const VPInstruction &Instruction) -> const VPPHINode & {
      return cast<VPPHINode>(Instruction);
    };

    // If the block is empty or if it has no PHIs, return null range
    if (empty() || !isa<VPPHINode>(begin()))
      return map_range(make_range(end(), end()), AsVPPHINode);

    // Increment iterator till a non PHI VPInstruction is found
    const_iterator It = begin();
    while (It != end() && isa<VPPHINode>(It))
      ++It;

    return map_range(make_range(begin(), It), AsVPPHINode);
  }

  VPBasicBlock(const std::string &Name, VPInstruction *Instruction = nullptr)
      : Name(Name), CBlock(nullptr), TBlock(nullptr), FBlock(nullptr),
        OriginalBB(nullptr) {
    if (Instruction)
      appendInstruction(Instruction);
  }

  ~VPBasicBlock() { Instructions.clear(); }

  const std::string &getName() const { return Name; }

  void setName(const Twine &newName) { Name = newName.str(); }

  VPlan *getParent() { return Parent; }
  const VPlan *getParent() const { return Parent; }

  void setParent(VPlan *P) { Parent = P; }

  VPValue *getPredicate() { return Predicate; }

  const VPValue *getPredicate() const { return Predicate; }

  void setPredicate(VPValue *Pred) { Predicate = Pred; }

  /// \Return the condition bit selecting the successor.
  VPValue *getCondBit() { return CondBit; }

  const VPValue *getCondBit() const { return CondBit; }

  void setCondBit(VPValue *CB) { CondBit = CB; }

  // The routine is used by BlockT *LoopBase<BlockT, LoopT>::getLoopPreheader().
  // There are currently no constraints that prevent an instruction to be
  // hoisted into a VPBlockBase and we don't support exception handling which
  // would have prevented such hoisting.
  bool isLegalToHoistInto() { return true; }

  void insert(VPInstruction *Instruction, iterator InsertPt);

  /// Augment the existing instructions of a VPBasicBlock with an additional
  /// \p Instruction as the last Instruction.
  void appendInstruction(VPInstruction *Instruction);

  /// Add \p Instruction after \p After. If \p After is null, \p Instruction
  /// will be inserted as the first instruction.
  void addInstructionAfter(VPInstruction *Instruction, VPInstruction *After);

  /// Augment the existing instructions of a VPBasicBlock with an additional
  /// \p Instruction at a position given by an existing instruction \p Before.
  /// \p If Before is null, \p Instruction is appended as the last instruction.
  void addInstruction(VPInstruction *Instruction,
                      VPInstruction *Before = nullptr);

  void moveConditionalEOBTo(VPBasicBlock *ToBB);

  /// Remove the instruction from VPBasicBlock's instructions.
  void removeInstruction(VPInstruction *Instruction) {
    Instructions.remove(Instruction);
  }

  /// Remove the instruction from VPBasicBlock's instructions and destroy
  /// Instruction object.
  void eraseInstruction(VPInstruction *Instruction);

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

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void printAsOperand(raw_ostream &OS, bool PrintType) const {
    (void)PrintType;
    OS << getName();
  }

  void print(raw_ostream &OS, unsigned Indent = 0,
             const VPlanDivergenceAnalysis *DA = nullptr,
             const Twine &NamePrefix = "") const;

  void dump() const { print(dbgs()); };
#endif // !NDEBUG || LLVM_ENABLE_DUMP
  void setCBlock(BasicBlock *CB) { CBlock = CB; }
  void setFBlock(BasicBlock *FB) { FBlock = FB; }
  void setTBlock(BasicBlock *TB) { TBlock = TB; }
  BasicBlock *getCBlock() { return CBlock; }
  BasicBlock *getTBlock() { return TBlock; }
  BasicBlock *getFBlock() { return FBlock; }

  bool hasTrueEdge() { return CBlock && TBlock; }
  bool hasFalseEdge() { return CBlock && FBlock; }

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

  BasicBlock *CBlock;
  BasicBlock *TBlock;
  BasicBlock *FBlock;
  BasicBlock *OriginalBB;

  // TODO: Not sure what other types of loop metadata we'd need. Most probably,
  // we need some abstraction on top of TripCountInfo (and maybe that struct
  // itself should be split in some way). The idea about this field is to have
  // something similar to LLVM IR's loop metadata on the backedge branch
  // instruction, so it will be filled for the latches only.
  std::unique_ptr<TripCountInfo> TCInfo;
};

/// Class that provides utilities for VPBasicBlocks in VPlan.
class VPBlockUtils {
public:
  VPBlockUtils() = delete;

  /// Insert NewBlock in Plan.
  static void insertBlockBefore(VPBasicBlock *NewBB, VPBasicBlock *BlockPtr);

  /// Insert NewBlock in Plan, connect NewBlock with given Preds of BlockPtr.
  /// Predecessors of BlockPtr that are not in the \p Preds will stay attached
  /// to BlockPtr.
  static void insertBlockBefore(VPBasicBlock *NewBB, VPBasicBlock *BlockPtr,
                                SmallVectorImpl<VPBasicBlock *> &Preds);

  /// Insert NewBlock in Plan. If BlockPtr has more that one successors, its
  /// CondBit is propagated to NewBlock.
  static void insertBlockAfter(VPBasicBlock *NewBB, VPBasicBlock *BlockPtr);

  /// Connect VPBasicBlocks \p From and \p To bi-directionally. Append \p To to
  /// the successors of \p From and \p From to the predecessors of \p To. Both
  /// VPBasicBlocks must have the same parent, which can be null. Both
  /// VPBasicBlocks can be already connected to other VPBlockBases.
  static void connectBlocks(VPBasicBlock *From, VPBasicBlock *To);

  /// Connect \p From to \p IfTrue and \p IfFalse bi-directionally. \p IfTrue
  /// and \p IfFalse are set as successors of \p From. \p From is set as
  /// predecessor of \p IfTrue and \p IfFalse. \p From must have no successors.
  static void connectBlocks(VPBasicBlock *From, VPValue *ConditionV,
                            VPBasicBlock *IfTrue, VPBasicBlock *IfFalse);

  /// Disconnect VPBasicBlocks \p From and \p To bi-directionally. Remove \p To
  /// from the successors of \p From and \p From from the predecessors of \p To.
  static void disconnectBlocks(VPBasicBlock *From, VPBasicBlock *To);

  // Replace \p OldSuccessor by \p NewSuccessor in Block's successor list.
  // \p NewSuccessor will be inserted in the same position as \p OldSuccessor.
  static void replaceBlockSuccessor(VPBasicBlock *BB,
                                    VPBasicBlock *OldSuccessor,
                                    VPBasicBlock *NewSuccessor);

  // Replace \p OldPredecessor by \p NewPredecessor in Block's predecessor list.
  // \p NewPredecessor will be inserted in the same position as \p
  // OldPredecessor.
  static void replaceBlockPredecessor(VPBasicBlock *BB,
                                      VPBasicBlock *OldPredecessor,
                                      VPBasicBlock *NewPredecessor);

  static void movePredecessor(VPBasicBlock *Pred, VPBasicBlock *From,
                              VPBasicBlock *To);

  static void movePredecessors(VPBasicBlock *From, VPBasicBlock *To,
                               SmallVectorImpl<VPBasicBlock *> &Predecessors);

  static void movePredecessors(VPBasicBlock *From, VPBasicBlock *To);

  static void moveSuccessors(VPBasicBlock *From, VPBasicBlock *To);

  /// Returns true if the edge \p FromBlock -> \p ToBlock is a back-edge.
  static bool isBackEdge(const VPBasicBlock *FromBB, const VPBasicBlock *ToBB,
                         const VPLoopInfo *VPLI);

  /// Returns true if \p Block is a loop latch
  static bool blockIsLoopLatch(const VPBasicBlock *BB,
                               const VPLoopInfo *VPLInfo);

private:
  static VPBasicBlock *splitBlock(VPBasicBlock *BB,
                                  VPBasicBlock::iterator BeforeIt,
                                  VPLoopInfo *VPLInfo,
                                  VPDominatorTree *DomTree = nullptr,
                                  VPPostDominatorTree *PostDomTree = nullptr);

public:
  static VPBasicBlock *
  splitBlockBegin(VPBasicBlock *BB, VPLoopInfo *VPLInfo,
                  VPDominatorTree *DomTree = nullptr,
                  VPPostDominatorTree *PostDomTree = nullptr);
  static VPBasicBlock *
  splitBlockEnd(VPBasicBlock *BB, VPLoopInfo *VPLInfo,
                VPDominatorTree *DomTree = nullptr,
                VPPostDominatorTree *PostDomTree = nullptr);
};
} // namespace vpo
} // namespace llvm
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPBASICBLOCK_H
