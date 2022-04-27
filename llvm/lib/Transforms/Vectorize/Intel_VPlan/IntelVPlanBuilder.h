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
/// It provides an instruction-level API for generating VPInstructions.
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANBUILDER_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANBUILDER_H

#include "IntelVPlan.h"

namespace llvm {
namespace vpo {

class VPBuilder {
protected:
  VPBasicBlock *BB = nullptr;
  VPBasicBlock::iterator InsertPt = VPBasicBlock::iterator();
  DebugLoc CurDbgLocation;

  VPInstruction *createInstruction(unsigned Opcode, Type *BaseTy,
                                   ArrayRef<VPValue *> Operands,
                                   const Twine &Name = "") {
    assert((!Instruction::isBinaryOp(Opcode) || Operands.size() == 2) &&
           "Expected 2 operands");
    assert((!Instruction::isUnaryOp(Opcode) || Operands.size() == 1) &&
           "Expected 1 operand");
    assert((Opcode != Instruction::GetElementPtr &&
            Opcode != Instruction::Load && Opcode != Instruction::Store) &&
           "Expected to be handled elsewhere!");

    VPInstruction *Instr = new VPInstruction(Opcode, BaseTy, Operands);
    insert(Instr);
    Instr->setName(Name);
    return Instr;
  }

  VPInstruction *createInstruction(unsigned Opcode, Type *BaseTy,
                                   std::initializer_list<VPValue *> Operands) {
    return createInstruction(Opcode, BaseTy, ArrayRef<VPValue *>(Operands));
  }

public:
  VPBuilder() {}

  /// Clear the insertion point: created instructions will not be
  /// inserted into a block.
  void clearInsertionPoint() {
    BB = nullptr;
    InsertPt = VPBasicBlock::iterator();
  }

  VPBasicBlock *getInsertBlock() const { return BB; }
  VPBasicBlock::iterator getInsertPoint() const { return InsertPt; }

  /// Insert and return the specified instruction. Appropriate debug location is
  /// also set for the instruction if available.
  VPInstruction *insert(VPInstruction *I) const {
    if (BB)
      BB->insert(I, InsertPt);
    if (CurDbgLocation)
      I->setDebugLocation(CurDbgLocation);
    return I;
  }

  /// InsertPoint - A saved insertion point.
  class VPInsertPoint {
    VPBasicBlock *Block = nullptr;
    VPBasicBlock::iterator Point;

  public:
    /// Creates a new insertion point which doesn't point to anything.
    VPInsertPoint() = default;

    /// Creates a new insertion point at the given location.
    VPInsertPoint(VPBasicBlock *InsertBlock, VPBasicBlock::iterator InsertPoint)
        : Block(InsertBlock), Point(InsertPoint) {}

    /// Returns true if this insert point is set.
    bool isSet() const { return (Block != nullptr); }

    VPBasicBlock *getBlock() const { return Block; }
    VPBasicBlock::iterator getPoint() const { return Point; }
  };

  /// Sets the current insert point to a previously-saved location.
  VPBuilder &restoreIP(VPInsertPoint IP) {
    if (IP.isSet())
      setInsertPoint(IP.getBlock(), IP.getPoint());
    else
      clearInsertionPoint();
    return *this;
  }

  /// This specifies that created VPInstructions should be appended to the end
  /// of the specified block.
  VPBuilder &setInsertPoint(VPBasicBlock *TheBB) {
    assert(TheBB && "Attempting to set a null insert point");
    BB = TheBB;
    InsertPt = BB->terminator();
    return *this;
  }

  /// This specifies that created instructions should be inserted
  /// before the specified instruction.
  VPBuilder &setInsertPoint(VPInstruction *I) {
    BB = I->getParent();
    InsertPt = I->getIterator();
    return *this;
  }

  /// This specifies that created instructions should be inserted at the
  /// specified point.
  VPBuilder &setInsertPoint(VPBasicBlock *TheBB, VPBasicBlock::iterator IP) {
    BB = TheBB;
    InsertPt = IP;
    return *this;
  }

  VPBuilder &setInsertPointFirstNonPhi(VPBasicBlock *TheBB) {
    BB = TheBB;
    VPBasicBlock::iterator IP = TheBB->begin();
    while (IP != TheBB->end() && isa<VPPHINode>(*IP))
      ++IP;
    InsertPt = IP;
    return *this;
  }

  VPBuilder &setInsertPointAfterBlends(VPBasicBlock *TheBB) {
    BB = TheBB;
    VPBasicBlock::iterator IP = TheBB->begin();
    while (IP != TheBB->end() && (isa<VPPHINode>(*IP) || isa<VPBlendInst>(*IP)))
      ++IP;
    InsertPt = IP;
    return *this;
  }

  // Set location information used by debugging information.
  void setCurrentDebugLocation(DebugLoc L) { CurDbgLocation = std::move(L); }

  // Get location information used by debugging information.
  const DebugLoc &getCurrentDebugLocation() const { return CurDbgLocation; }

  // Create an N-ary operation with \p Opcode, \p Operands and set \p Inst as
  // its underlying Instruction.
  VPInstruction *createNaryOp(unsigned Opcode, Type *BaseTy,
                        ArrayRef<VPValue *> Operands,
                        Instruction *Inst = nullptr) {
    VPInstruction *NewVPInst = createInstruction(Opcode, BaseTy, Operands);
    if (Inst)
      NewVPInst->setUnderlyingValue(*Inst);
    return NewVPInst;
  }
  VPInstruction *createNaryOp(unsigned Opcode, Type *BaseTy,
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

  VPInstruction *createFAdd(VPValue *LHS, VPValue *RHS,
                            const Twine &Name = "") {
    assert(LHS->getType() == RHS->getType() &&
           "LHS and RHs do not have the same types.");
    return createInstruction(Instruction::FAdd, LHS->getType(), {LHS, RHS},
                             Name);
  }

  VPValue *createSub(VPValue *LHS, VPValue *RHS, const Twine &Name = "") {
     assert(LHS->getType() == RHS->getType() &&
           "LHS and RHS do not have the same types.");
     assert(LHS->getType()->isIntOrIntVectorTy() &&
            "Integer arithmetic operators only work with integral types!");
     return createInstruction(Instruction::BinaryOps::Sub, LHS->getType(),
                              {LHS, RHS}, Name);
  }

  VPInstruction *createFSub(VPValue *LHS, VPValue *RHS,
                            const Twine &Name = "") {
    assert(LHS->getType() == RHS->getType() &&
           "LHS and RHS do not have the same types.");
    assert(LHS->getType()->isFPOrFPVectorTy() &&
           "Floating-point arithmetic operators only work with floating-point types!");
    return createInstruction(Instruction::FSub, LHS->getType(), {LHS, RHS},
                             Name);
  }

  VPValue *createMul(VPValue *LHS, VPValue *RHS, const Twine &Name = "") {
    assert(LHS->getType() == RHS->getType() &&
           "LHS and RHs do not have the same types.");
    return createInstruction(Instruction::Mul, LHS->getType(), {LHS, RHS},
                             Name);
  }

  VPValue *createFMul(VPValue *LHS, VPValue *RHS, const Twine &Name = "") {
    assert(LHS->getType() == RHS->getType() &&
           "LHS and RHs do not have the same types.");
    return createInstruction(Instruction::FMul, LHS->getType(), {LHS, RHS},
                             Name);
  }

  VPValue *createSIToFp(VPValue *Val, Type *Ty) {
    if (Ty != Val->getType()) {
      Val = createNaryOp(Instruction::SIToFP, Ty, {Val});
    }
    return Val;
  }

  VPValue *createZExtOrTrunc(VPValue *Val, Type *DestTy) {
    if (Val->getType() == DestTy)
      return Val;
    assert(Val->getType()->isIntOrIntVectorTy() &&
           DestTy->isIntOrIntVectorTy() &&
           "Can only zero extend/truncate integers!");
    Type *VTy = Val->getType();
    unsigned Opcode = 0;
    if (VTy->getScalarSizeInBits() < DestTy->getScalarSizeInBits())
      Opcode = Instruction::ZExt;
    if (VTy->getScalarSizeInBits() > DestTy->getScalarSizeInBits())
      Opcode = Instruction::Trunc;

    Val = createNaryOp(Opcode, DestTy, {Val});
    return Val;
  }

  VPValue *createAllZeroCheck(VPValue *Operand, const Twine &Name = "") {
    return createInstruction(VPInstruction::AllZeroCheck, Operand->getType(),
                             {Operand}, Name);
  }

  VPValue *createAbs(VPValue *Operand, const Twine &Name = "") {
    return createInstruction(VPInstruction::Abs, Operand->getType(), {Operand},
                             Name);
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

  VPInstruction *createPred(VPValue *Operand) {
    return createInstruction(VPInstruction::Pred, Operand->getType(),
                             {Operand});
  }

  VPInstruction *createSelect(VPValue *Mask, VPValue *Tval, VPValue *Fval,
                        const Twine &Name = "") {
    return createInstruction(Instruction::Select, Tval->getType(),
                             {Mask, Tval, Fval}, Name);
  }

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

  /// Create VPCmpInst with its two operands.
  VPCmpInst *createCmpInst(CmpInst::Predicate Pred, VPValue *LeftOp,
                           VPValue *RightOp, const Twine &Name = "") {
    assert(LeftOp && RightOp && "VPCmpInst's operands can't be null!");
    VPCmpInst *Instr = new VPCmpInst(LeftOp, RightOp, Pred);
    Instr->setName(Name);
    insert(Instr);
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
    insert(NewVPPHINode);
    return NewVPPHINode;
  }

  // Build a VPGEPInstruction for the LLVM-IR instruction \p Inst using base
  // pointer \p Ptr and list of index operands \p IdxList
  VPGEPInstruction *createGEP(Type *SourceElementType, Type *ResultElementType,
                              VPValue *Ptr, ArrayRef<VPValue *> IdxList,
                              Instruction *Inst) {
    assert((Inst || Ptr->getType()->isPointerTy()) &&
           "Can't define type for GEP instruction");
    // TODO. Currently, it's expected that newly created GEP (e.g. w/o
    // underlying IR) is created for a non-array types. Need to handle those
    // arrays when simd reductions/privates will support arrays.
    Type *Ty = Inst ? Inst->getType() : Ptr->getType();
    auto *NewVPInst = new VPGEPInstruction(SourceElementType, ResultElementType,
                                           Ty, Ptr, IdxList);
    insert(NewVPInst);
    if (Inst)
      NewVPInst->setUnderlyingValue(*Inst);
    return NewVPInst;
  }

  // Build a new load VPInstruction from given pointer operand.
  VPLoadStoreInst *createLoad(Type *Ty, VPValue *Ptr,
                              Instruction *Inst = nullptr,
                              const Twine &Name = "load") {
    VPLoadStoreInst *NewLoad =
        new VPLoadStoreInst(Instruction::Load, Ty, {Ptr});
    NewLoad->setName(Name);
    insert(NewLoad);
    if (Inst) {
      NewLoad->setUnderlyingValue(*Inst);
      NewLoad->readUnderlyingMetadata();
    }
    return NewLoad;
  }

  // Build a new store VPInstruction using given value and pointer operand.
  VPLoadStoreInst *createStore(VPValue *Val, VPValue *Ptr,
                               Instruction *Inst = nullptr,
                               const Twine &Name = "store") {
    VPLoadStoreInst *NewStore = new VPLoadStoreInst(
        Instruction::Store, Type::getVoidTy(Val->getType()->getContext()),
        {Val, Ptr});
    NewStore->setName(Name);
    insert(NewStore);
    if (Inst) {
      NewStore->setUnderlyingValue(*Inst);
      NewStore->readUnderlyingMetadata();
    }
    return NewStore;
  }

  // Build a VPCallInstruction for the LLVM-IR instruction \p Inst using callee
  // \p CalledValue and list of argument operands \p ArgList.
  VPInstruction *createCall(VPValue *CalledValue, ArrayRef<VPValue *> ArgList,
                            Instruction *Inst) {
    assert(Inst && "Cannot create VPCallInstruction without underlying IR.");
    CallInst *Call = cast<CallInst>(Inst);
    VPCallInstruction *NewVPCall =
        new VPCallInstruction(CalledValue, ArgList, *Call);
    NewVPCall->setUnderlyingValue(*Call);
    NewVPCall->setName(Inst->getName());
    insert(NewVPCall);
    return NewVPCall;
  }

  VPInstruction *createReductionInit(VPValue *Identity, VPValue *Start,
                                     bool UseStart, const Twine &Name = "") {
    VPInstruction *NewVPInst = Start ? new VPReductionInit(Identity, Start)
                                     : new VPReductionInit(Identity, UseStart);
    NewVPInst->setName(Name);
    insert(NewVPInst);
    return NewVPInst;
  }

  template <class T, class NameType, class... Args>
  T *create(const NameType &Name, Args &&... args) {
    auto *New = new T(std::forward<Args>(args)...);
    New->setName(Name);
    insert(New);
    return New;
  }

  /// Create a cast of \p Val to \p Ty if needed.
  VPValue *createIntCast(VPValue *Val, Type *Ty) {
    if (Ty != Val->getType()) {
      assert((Ty->isIntegerTy() && Val->getType()->isIntegerTy()) &&
             "Expected integer type");
      unsigned Opcode = Ty->getPrimitiveSizeInBits() <
                                Val->getType()->getPrimitiveSizeInBits()
                            ? Instruction::Trunc
                            : Instruction::SExt;
      Val = createNaryOp(Opcode, Ty, {Val});
    }
    return Val;
  }

  VPValue *createFPCast(VPValue *Val, Type *Ty) {
    if (Ty != Val->getType()) {
      assert((Ty->isFloatTy() || Ty->isDoubleTy()) &&
             "Expected floating point type for Ty");
      assert((Val->getType()->isFloatTy() || Val->getType()->isDoubleTy()) &&
             "Expected floating point type for Val");
      unsigned Opcode = Ty->getPrimitiveSizeInBits() <
                                Val->getType()->getPrimitiveSizeInBits()
                            ? Instruction::FPTrunc
                            : Instruction::FPExt;
      Val = createNaryOp(Opcode, Ty, {Val});
    }
    return Val;
  }

  VPValue *createIntToFPCast(VPValue *Val, Type *Ty) {
    assert((Ty->getScalarSizeInBits() == 32 ||
            Ty->getScalarSizeInBits() == 64) &&
           "Expected 32 or 64 bit type size for fp conversion");
    VPValue *FPCast = createNaryOp(Instruction::SIToFP, Ty, {Val});
    return FPCast;
  }

  VPValue *createFPToIntCast(VPValue *Val, Type *Ty) {
    assert((Val->getType()->getScalarSizeInBits() == 32 ||
            Val->getType()->getScalarSizeInBits()  == 64) &&
           "Expected 32 or 64 bit type size for fp conversion");
    VPValue *IntCast = createNaryOp(Instruction::FPToSI, Ty, {Val});
    return IntCast;
  }

  //===--------------------------------------------------------------------===//
  // RAII helpers.
  //===--------------------------------------------------------------------===//

  // RAII object that stores the current insertion point and restores it when
  // the object is destroyed.
  class InsertPointGuard {
    VPBuilder &Builder;
    // TODO: AssertingVH<VPBasicBlock> Block;
    VPBasicBlock* Block;
    VPBasicBlock::iterator Point;
    DebugLoc DbgLoc;

  public:
    InsertPointGuard(VPBuilder &B)
        : Builder(B), Block(B.getInsertBlock()), Point(B.getInsertPoint()),
          DbgLoc(B.getCurrentDebugLocation()) {}

    InsertPointGuard(const InsertPointGuard &) = delete;
    InsertPointGuard &operator=(const InsertPointGuard &) = delete;

    ~InsertPointGuard() {
      Builder.restoreIP(VPInsertPoint(Block, Point));
      Builder.setCurrentDebugLocation(DbgLoc);
    }
  };
};
} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANBUILDER_H
