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
  VPInstruction *createInstruction(unsigned Opcode, Type *BaseTy,
                                   ArrayRef<VPValue *> Operands,
                                   const Twine &Name = "") {
    VPInstruction *Instr = new VPInstruction(Opcode, BaseTy, Operands);
    if (BB)
      BB->insert(Instr, InsertPt);
    Instr->setName(Name);
    return Instr;
  }

  VPInstruction *createInstruction(unsigned Opcode, Type *BaseTy,
                                   std::initializer_list<VPValue *> Operands) {
    return createInstruction(Opcode, BaseTy, ArrayRef<VPValue *>(Operands));
  }

#else
  VPInstruction *createInstruction(unsigned Opcode,
                                   std::initializer_list<VPValue *> Operands) {
    VPInstruction *Instr = new VPInstruction(Opcode, Operands);
    BB->insert(Instr, InsertPt);
    return Instr;
  }
#endif //INTEL_CUSTOMIZATION

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

  void setInsertPointFirstNonPhi(VPBasicBlock *TheBB) {
    BB = TheBB;
    VPBasicBlock::iterator IP = TheBB->begin();
    while (IP != TheBB->end() && isa<VPPHINode>(*IP))
      ++IP;
    InsertPt = IP;
  }

  // Create an N-ary operation with \p Opcode, \p Operands and set \p Inst as
  // its underlying Instruction.
  VPValue *createNaryOp(unsigned Opcode, Type *BaseTy,
                        ArrayRef<VPValue *> Operands,
                        Instruction *Inst = nullptr) {
    VPInstruction *NewVPInst = createInstruction(Opcode, BaseTy, Operands);
    if (Inst)
      NewVPInst->setUnderlyingValue(*Inst);
    return NewVPInst;
  }
  VPValue *createNaryOp(unsigned Opcode, Type *BaseTy,
                        std::initializer_list<VPValue *> Operands,
                        Instruction *Inst = nullptr) {
    return createNaryOp(Opcode, BaseTy, ArrayRef<VPValue *>(Operands), Inst);
  }

  // Create a VPInstruction with \p LHS and \p RHS as operands and Add opcode.
  // For now, no no-wrap flags are used since they cannot be modeled in VPlan.
  VPValue *createAdd(VPValue *LHS, VPValue *RHS, const Twine &Name = "") {
    return createInstruction(Instruction::BinaryOps::Add, LHS->getType(),
                             {LHS, RHS}, Name);
  }

  VPValue *createAllZeroCheck(VPValue *Operand, const Twine &Name = "") {
    return createInstruction(VPInstruction::AllZeroCheck, Operand->getType(),
                             {Operand}, Name);
  }

  VPValue *createAnd(VPValue *LHS, VPValue *RHS, const Twine &Name = "") {
    return createInstruction(Instruction::BinaryOps::And, LHS->getType(),
                             {LHS, RHS}, Name);
  }

  VPValue *createNot(VPValue *Operand, const Twine &Name = "") {
    return createInstruction(VPInstruction::Not, Operand->getType(), {Operand},
                             Name);
  }

  VPValue *createOr(VPValue *LHS, VPValue *RHS, const Twine &Name = "") {
    return createInstruction(Instruction::BinaryOps::Or, LHS->getType(),
                             {LHS, RHS}, Name);
  }

  VPValue *createPred(VPValue *Operand) {
    return createInstruction(VPInstruction::Pred, Operand->getType(),
                             {Operand});
  }

  VPValue *createSelect(VPValue *Mask, VPValue *Tval, VPValue *Fval,
                        const Twine &Name = "") {
    return createInstruction(Instruction::Select, Tval->getType(),
                             {Mask, Tval, Fval}, Name);
  }
#else

  VPValue *createNot(VPValue *Operand) {
    return createInstruction(VPInstruction::Not, {Operand});
  }

  VPValue *createAnd(VPValue *LHS, VPValue *RHS) {
    return createInstruction(Instruction::BinaryOps::And, {LHS, RHS});
  }

  VPValue *createOr(VPValue *LHS, VPValue *RHS) {
    return createInstruction(Instruction::BinaryOps::Or, {LHS, RHS});
  }
#endif //INTEL_CUSTOMIZATION

#if INTEL_CUSTOMIZATION
  // Create a VPCmpInst with \p LeftOp and \p RightOp as operands, and \p CI's
  // predicate as predicate. \p CI is also set as underlying Instruction.
  VPCmpInst *createCmpInst(VPValue *LeftOp, VPValue *RightOp, CmpInst *CI) {
    // TODO: If a null CI is needed, please create a new interface.
    assert(CI && "CI can't be null.");
    VPCmpInst *VPCI =
        createCmpInst(CI->getPredicate(), LeftOp, RightOp);
    VPCI->setUnderlyingValue(*CI);
    return VPCI;
  }

  /// \brief Create VPCmpInst with its two operands.
  VPCmpInst *createCmpInst(CmpInst::Predicate Pred, VPValue *LeftOp,
                           VPValue *RightOp, const Twine &Name = "") {
    assert(LeftOp && RightOp && "VPCmpInst's operands can't be null!");
    VPCmpInst *Instr = new VPCmpInst(LeftOp, RightOp, Pred);
    Instr->setName(Name);
    if (BB)
      BB->insert(Instr, InsertPt);
    return Instr;
  }

  // Create dummy VPBranchInst instruction.
  VPBranchInst *createBr(Type *BaseTy) {
    VPBranchInst *Instr = new VPBranchInst(BaseTy);
    if (BB)
      BB->insert(Instr, InsertPt);
    return Instr;
  }

  VPPHINode *createPhiInstruction(Instruction *Inst, const Twine &Name = "") {
    assert(Inst != nullptr && "Instruction cannot be a nullptr");
    VPPHINode *NewVPPHINode = createPhiInstruction(Inst->getType(), Name);
    NewVPPHINode->setUnderlyingValue(*Inst);
    return NewVPPHINode;
  }

  VPPHINode *createPhiInstruction(Type *BaseTy, const Twine &Name = "") {
    VPPHINode *NewVPPHINode = new VPPHINode(BaseTy);
    NewVPPHINode->setName(Name);
    if (BB)
      BB->insert(NewVPPHINode, InsertPt);
    return NewVPPHINode;
  }

  // Build a VPGEPInstruction for the LLVM-IR instruction \p Inst using base
  // pointer \p Ptr and list of index operands \p IdxList
  VPInstruction *createGEP(VPValue *Ptr, ArrayRef<VPValue *> IdxList,
                           Instruction *Inst) {
    assert((Inst || Ptr->getType()->isPointerTy()) &&
           "Can't define type for GEP instruction");
    // TODO. Currently, it's expected that newly created GEP (e.g. w/o
    // underlying IR) is created for a non-array types. Need to handle those
    // arrays when simd reductions/privates will support arrays.
    Type *Ty = Inst ? Inst->getType() : Ptr->getType();
    VPInstruction *NewVPInst = new VPGEPInstruction(Ty, Ptr, IdxList);
    if (BB)
      BB->insert(NewVPInst, InsertPt);
    if (Inst)
      NewVPInst->setUnderlyingValue(*Inst);
    return NewVPInst;
  }

  // Build an inbounds VPGEPInstruction for the LLVM-IR instruction \p Inst
  // using base pointer \p Ptr and list of index operands \p IdxList
  VPInstruction *createInBoundsGEP(VPValue *Ptr, ArrayRef<VPValue *> IdxList,
                                   Instruction *Inst) {
    VPInstruction *NewVPInst = createGEP(Ptr, IdxList, Inst);
    cast<VPGEPInstruction>(NewVPInst)->setIsInBounds(true);
    return NewVPInst;
  }

  // Reduction init/final
  VPInstruction *createReductionInit(VPValue *Identity,
                                     VPValue *Start = nullptr) {
    VPInstruction *NewVPInst = Start ? new VPReductionInit(Identity, Start)
                                     : new VPReductionInit(Identity);
    if (BB)
      BB->insert(NewVPInst, InsertPt);
    return NewVPInst;
  }

  VPReductionFinal *createReductionFinal(unsigned BinOp, VPValue *ReducVec,
                                         VPValue *StartValue, bool Sign) {
    VPReductionFinal *NewVPInst =
        new VPReductionFinal(BinOp, ReducVec, StartValue, Sign);
    if (BB)
      BB->insert(NewVPInst, InsertPt);
    return NewVPInst;
  }

  VPReductionFinal *createReductionFinal(unsigned BinOp, VPValue *ReducVec) {
    VPReductionFinal *NewVPInst = new VPReductionFinal(BinOp, ReducVec);
    if (BB)
      BB->insert(NewVPInst, InsertPt);
    return NewVPInst;
  }

  // Final value of index part of min/max+index
  VPReductionFinal *createReductionFinal(unsigned BinOp, VPValue *ReducVec,
                                         VPValue *ParentExit,
                                         VPReductionFinal *ParentFinal,
                                         bool Sign) {
    VPReductionFinal *NewVPInst =
        new VPReductionFinal(BinOp, ReducVec, ParentExit, ParentFinal, Sign);
    if (BB)
      BB->insert(NewVPInst, InsertPt);
    return NewVPInst;
  }

  // Induction init/final
  VPInstruction *createInductionInit(VPValue *Start, VPValue *Step,
                                     Instruction::BinaryOps Opc) {
    VPInstruction *NewVPInst = new VPInductionInit(Start, Step, Opc);
    if (BB)
      BB->insert(NewVPInst, InsertPt);
    return NewVPInst;
  }

  VPInstruction *createInductionInitStep(VPValue *Step,
                                         Instruction::BinaryOps Opcode) {
    VPInstruction *NewVPInst = new VPInductionInitStep(Step, Opcode);
    if (BB)
      BB->insert(NewVPInst, InsertPt);
    return NewVPInst;
  }

  VPInstruction *createInductionFinal(VPValue *InducVec) {
    VPInstruction *NewVPInst = new VPInductionFinal(InducVec);
    if (BB)
      BB->insert(NewVPInst, InsertPt);
    return NewVPInst;
  }

  VPInstruction *createInductionFinal(VPValue *Start, VPValue *Step,
                                      Instruction::BinaryOps Opcode) {
    VPInstruction *NewVPInst = new VPInductionFinal(Start, Step, Opcode);
    if (BB)
      BB->insert(NewVPInst, InsertPt);
    return NewVPInst;
  }

  VPInstruction *createAllocaPrivate(Type *Ty, bool IsSOALayout) {
    VPInstruction *NewVPInst = new VPAllocatePrivate(Ty, IsSOALayout);
    if (BB)
      BB->insert(NewVPInst, InsertPt);
    return NewVPInst;
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
