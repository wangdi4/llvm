//===- VPlanBuilder.h - A VPlan utility for constructing VPInstructions ---===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file provides a VPlan-based builder utility analogous to IRBuilder.
/// It provides an instruction-level API for generating VPInstructions while
/// abstracting away the Recipe manipulation details.
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANBUILDER_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANBUILDER_H

#include "IntelVPlan.h"

namespace llvm {
namespace vpo {    

class VPBuilder {
#if INTEL_CUSTOMIZATION
protected:
#else
private:
#endif
  VPBasicBlock *BB = nullptr;
  VPBasicBlock::iterator InsertPt = VPBasicBlock::iterator();

#if INTEL_CUSTOMIZATION
  VPInstruction *createInstruction(unsigned Opcode,
                                   ArrayRef<VPValue *> Operands) {
    VPInstruction *Instr = new VPInstruction(Opcode, Operands);
    if (BB)
      BB->insert(Instr, InsertPt);
    return Instr;
  }
#endif

  VPInstruction *createInstruction(unsigned Opcode,
                                   std::initializer_list<VPValue *> Operands) {
    return createInstruction(Opcode, ArrayRef<VPValue *>(Operands));
  }

#if INTEL_CUSTOMIZATION
  /// \brief Create VPCmpInst with its two operands.
  VPCmpInst *createCmpInst(VPValue *LeftOp, VPValue *RightOp,
                           CmpInst::Predicate Pred) {
    assert(LeftOp && RightOp && "VPCmpInst's operands can't be null!");
    VPCmpInst *Instr = new VPCmpInst(LeftOp, RightOp, Pred);
    if (BB)
      BB->insert(Instr, InsertPt);
    return Instr;
  }
#endif

public:
  VPBuilder() {}

#if INTEL_CUSTOMIZATION
  /// \brief Clear the insertion point: created instructions will not be
  /// inserted into a block.
  void clearInsertionPoint() {
    BB = nullptr;
    InsertPt = VPBasicBlock::iterator();
  }

  VPBasicBlock *getInsertBlock() const { return BB; }
  VPBasicBlock::iterator getInsertPoint() const { return InsertPt; }

  /// \brief Insert and return the specified instruction.
  VPInstruction *insert(VPInstruction *I) const {
    BB->insert(I, InsertPt);
    return I;
  }

  /// InsertPoint - A saved insertion point.
  class VPInsertPoint {
    VPBasicBlock *Block = nullptr;
    VPBasicBlock::iterator Point;

  public:
    /// \brief Creates a new insertion point which doesn't point to anything.
    VPInsertPoint() = default;

    /// \brief Creates a new insertion point at the given location.
    VPInsertPoint(VPBasicBlock *InsertBlock, VPBasicBlock::iterator InsertPoint)
        : Block(InsertBlock), Point(InsertPoint) {}

    /// \brief Returns true if this insert point is set.
    bool isSet() const { return (Block != nullptr); }

    VPBasicBlock *getBlock() const { return Block; }
    VPBasicBlock::iterator getPoint() const { return Point; }
  };

  /// \brief Sets the current insert point to a previously-saved location.
  void restoreIP(VPInsertPoint IP) {
    if (IP.isSet())
      setInsertPoint(IP.getBlock(), IP.getPoint());
    else
      clearInsertionPoint();
  }
#endif

  /// This specifies that created VPInstructions should be appended to the end
  /// of the specified block.
  void setInsertPoint(VPBasicBlock *TheBB) {
    assert(TheBB && "Attempting to set a null insert point");
    BB = TheBB;
    InsertPt = BB->end();
  }
#if INTEL_CUSTOMIZATION
  /// \brief This specifies that created instructions should be inserted
  /// before the specified instruction.
  void setInsertPoint(VPInstruction *I) {
    BB = I->getParent();
    InsertPt = I->getIterator();
  }

  /// \brief This specifies that created instructions should be inserted at the
  /// specified point.
  void setInsertPoint(VPBasicBlock *TheBB, VPBasicBlock::iterator IP) {
    BB = TheBB;
    InsertPt = IP;
  }

  // Create an N-ary operation with \p Opcode, \p Operands and set \p Inst as
  // its underlying Instruction.
  VPValue *createNaryOp(unsigned Opcode, ArrayRef<VPValue *> Operands,
                        Instruction *Inst = nullptr) {
    VPInstruction *NewVPInst = createInstruction(Opcode, Operands);
    NewVPInst->setUnderlyingValue(Inst);
    return NewVPInst;
  }
  VPValue *createNaryOp(unsigned Opcode,
                        std::initializer_list<VPValue *> Operands,
                        Instruction *Inst = nullptr) {
    return createNaryOp(Opcode, ArrayRef<VPValue *>(Operands), Inst);
  }

  // Create a VPInstruction with \p LHS and \p RHS as operands and Add opcode.
  // For now, no no-wrap flags are used since they cannot be modeled in VPlan.
  VPValue *createAdd(VPValue *LHS, VPValue *RHS) {
    return createInstruction(Instruction::BinaryOps::Add, {LHS, RHS});
  }
#endif

  VPValue *createNot(VPValue *Operand) {
    return createInstruction(VPInstruction::Not, {Operand});
  }

  VPValue *createAnd(VPValue *LHS, VPValue *RHS) {
    return createInstruction(Instruction::BinaryOps::And, {LHS, RHS});
  }

  VPValue *createOr(VPValue *LHS, VPValue *RHS) {
    return createInstruction(Instruction::BinaryOps::Or, {LHS, RHS});
  }

#if INTEL_CUSTOMIZATION
  // Create a VPCmpInst with \p LeftOp and \p RightOp as operands, and \p CI's
  // predicate as predicate. \p CI is also set as underlying Instruction.
  VPCmpInst *createCmpInst(VPValue *LeftOp, VPValue *RightOp, CmpInst *CI) {
    // TODO: If a null CI is needed, please create a new interface.
    assert(CI && "CI can't be null.");
    assert(LeftOp && RightOp && "VPCmpInst's operands can't be null!");
    VPCmpInst *VPCI = createCmpInst(LeftOp, RightOp, CI->getPredicate());
    VPCI->setUnderlyingValue(CI);
    return VPCI;
  }

  // Create dummy VPBranchInst instruction.
  VPBranchInst *createBr() {
    VPBranchInst *Instr = new VPBranchInst();
    if (BB)
      BB->insert(Instr, InsertPt);
    return Instr;
  }

  //===--------------------------------------------------------------------===//
  // RAII helpers.
  //===--------------------------------------------------------------------===//

  // \brief RAII object that stores the current insertion point and restores it
  // when the object is destroyed.
  class InsertPointGuard {
    VPBuilder &Builder;
    // TODO: AssertingVH<VPBasicBlock> Block;
    VPBasicBlock* Block;
    VPBasicBlock::iterator Point;

  public:
    InsertPointGuard(VPBuilder &B)
        : Builder(B), Block(B.getInsertBlock()), Point(B.getInsertPoint()) {}

    InsertPointGuard(const InsertPointGuard &) = delete;
    InsertPointGuard &operator=(const InsertPointGuard &) = delete;

    ~InsertPointGuard() {
      Builder.restoreIP(VPInsertPoint(Block, Point));
    }
  };
#endif // INTEL_CUSTOMIZATION
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANBUILDER_H
