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

  VPValue *createSelect(VPValue *Mask, VPValue *Tval, VPValue *Fval,
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

  VPBlendInst *createBlendInstruction(Type *Ty, const Twine &Name = "") {
    auto *Blend = new VPBlendInst(Ty);
    Blend->setName(Name);
    insert(Blend);
    return Blend;
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
    insert(NewVPInst);
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

  // Build a single-dimensional VPSubscriptInst to represent a subscript
  // intrinsic call.
  VPSubscriptInst *createSubscriptInst(Type *BaseTy, unsigned Rank,
                                       VPValue *Lower, VPValue *Stride,
                                       VPValue *Base, VPValue *Index,
                                       Instruction *Inst = nullptr,
                                       const Twine &Name = "subscript") {
    VPSubscriptInst *NewSubscript =
        new VPSubscriptInst(BaseTy, Rank, Lower, Stride, Base, Index);
    NewSubscript->setName(Name);
    insert(NewSubscript);
    if (Inst)
      NewSubscript->setUnderlyingValue(*Inst);
    return NewSubscript;
  }

  // Build a multi-dimensional VPSubscriptInst to represent a combined
  // multi-dimensional array access implemented using subscript intrinsic calls.
  // TODO: Such an access would usually have multiple underlying instructions,
  // how to map the VPValue to multiple Values?
  VPSubscriptInst *createSubscriptInst(Type *BaseTy, unsigned NumDims,
                                       ArrayRef<VPValue *> Lowers,
                                       ArrayRef<VPValue *> Strides,
                                       VPValue *Base,
                                       ArrayRef<VPValue *> Indices,
                                       const Twine &Name = "subscript") {
    VPSubscriptInst *NewSubscript =
        new VPSubscriptInst(BaseTy, NumDims, Lowers, Strides, Base, Indices);
    NewSubscript->setName(Name);
    insert(NewSubscript);
    return NewSubscript;
  }

  // Build a multi-dimensional VPSubscriptInst to represent a combined
  // multi-dimensional array access implemented using subscript intrinsic calls
  // when each dimension has associated struct offsets.
  VPSubscriptInst *createSubscriptInst(
      Type *BaseTy, unsigned NumDims, ArrayRef<VPValue *> Lowers,
      ArrayRef<VPValue *> Strides, VPValue *Base, ArrayRef<VPValue *> Indices,
      VPSubscriptInst::DimStructOffsetsMapTy StructOffsets,
      VPSubscriptInst::DimTypeMapTy Types, const Twine &Name = "subscript") {
    VPSubscriptInst *NewSubscript = new VPSubscriptInst(
        BaseTy, NumDims, Lowers, Strides, Base, Indices, StructOffsets, Types);
    NewSubscript->setName(Name);
    insert(NewSubscript);
    return NewSubscript;
  }
  VPSubscriptInst *createInBoundsSubscriptInst(
      Type *BaseTy, unsigned NumDims, ArrayRef<VPValue *> Lowers,
      ArrayRef<VPValue *> Strides, VPValue *Base, ArrayRef<VPValue *> Indices,
      VPSubscriptInst::DimStructOffsetsMapTy StructOffsets,
      VPSubscriptInst::DimTypeMapTy Types, const Twine &Name = "subscript") {
    VPSubscriptInst *NewSubscript =
        createSubscriptInst(BaseTy, NumDims, Lowers, Strides, Base, Indices,
                            StructOffsets, Types, Name);
    NewSubscript->setIsInBounds(true);
    return NewSubscript;
  }

  // Build a VPCallInstruction for the LLVM-IR instruction \p Inst using callee
  // \p CalledValue and list of argument operands \p ArgList.
  VPInstruction *createCall(VPValue *CalledValue, ArrayRef<VPValue *> ArgList,
                            Instruction *Inst) {
    assert(Inst && "Cannot create VPCallInstruction without underlying IR.");
    CallInst *Call = cast<CallInst>(Inst);
    VPCallInstruction *NewVPCall =
        new VPCallInstruction(CalledValue, ArgList, Call);
    NewVPCall->setUnderlyingValue(*Call);
    NewVPCall->setName(Inst->getName());
    insert(NewVPCall);
    return NewVPCall;
  }

  // Reduction init/final
  VPInstruction *createReductionInit(VPValue *Identity, VPValue *Start,
                                     const Twine &Name = "") {
    VPInstruction *NewVPInst = Start ? new VPReductionInit(Identity, Start)
                                     : new VPReductionInit(Identity);
    NewVPInst->setName(Name);
    insert(NewVPInst);
    return NewVPInst;
  }

  VPReductionFinal *createReductionFinal(unsigned BinOp, VPValue *ReducVec,
                                         VPValue *StartValue, bool Sign,
                                         const Twine &Name = "") {
    VPReductionFinal *NewVPInst =
        new VPReductionFinal(BinOp, ReducVec, StartValue, Sign);
    NewVPInst->setName(Name);
    insert(NewVPInst);
    return NewVPInst;
  }

  VPReductionFinal *createReductionFinal(unsigned BinOp, VPValue *ReducVec,
                                         const Twine &Name = "") {
    VPReductionFinal *NewVPInst = new VPReductionFinal(BinOp, ReducVec);
    NewVPInst->setName(Name);
    insert(NewVPInst);
    return NewVPInst;
  }

  // Final value of index part of min/max+index
  VPReductionFinal *createReductionFinal(unsigned BinOp, VPValue *ReducVec,
                                         VPValue *ParentExit,
                                         VPReductionFinal *ParentFinal,
                                         bool Sign, const Twine &Name = "") {
    VPReductionFinal *NewVPInst =
        new VPReductionFinal(BinOp, ReducVec, ParentExit, ParentFinal, Sign);
    NewVPInst->setName(Name);
    insert(NewVPInst);
    return NewVPInst;
  }

  // Induction init/final
  VPInstruction *createInductionInit(VPValue *Start, VPValue *Step,
                                     Instruction::BinaryOps Opc,
                                     const Twine &Name = "") {
    VPInstruction *NewVPInst = new VPInductionInit(Start, Step, Opc);
    NewVPInst->setName(Name);
    insert(NewVPInst);
    return NewVPInst;
  }

  VPInstruction *createInductionInitStep(VPValue *Step,
                                         Instruction::BinaryOps Opcode,
                                         const Twine &Name = "") {
    VPInstruction *NewVPInst = new VPInductionInitStep(Step, Opcode);
    NewVPInst->setName(Name);
    insert(NewVPInst);
    return NewVPInst;
  }

  VPInstruction *createInductionFinal(VPValue *InducVec,
                                      const Twine &Name = "") {
    VPInstruction *NewVPInst = new VPInductionFinal(InducVec);
    NewVPInst->setName(Name);
    insert(NewVPInst);
    return NewVPInst;
  }

  VPInstruction *createInductionFinal(VPValue *Start, VPValue *Step,
                                      Instruction::BinaryOps Opcode,
                                      const Twine &Name = "") {
    VPInstruction *NewVPInst = new VPInductionFinal(Start, Step, Opcode);
    NewVPInst->setName(Name);
    insert(NewVPInst);
    return NewVPInst;
  }

  VPInstruction *createAllocaPrivate(const VPValue *AI) {
    VPInstruction *NewVPInst = new VPAllocatePrivate(AI->getType());
    insert(NewVPInst);
    NewVPInst->setName(AI->getName());
    return NewVPInst;
  }

  VPOrigTripCountCalculation *
  createOrigTripCountCalculation(Loop *OrigLoop, VPLoop *VPLp, Type *Ty,
                                 const Twine &Name = "orig.trip.count") {
    auto *OrigTC = new VPOrigTripCountCalculation(OrigLoop, VPLp, Ty);
    OrigTC->setName(Name);
    insert(OrigTC);
    return OrigTC;
  }

  VPVectorTripCountCalculation *
  createVectorTripCountCalculation(VPOrigTripCountCalculation *OrigTC,
                                   const Twine &Name = "vector.trip.count") {
    auto *TC = new VPVectorTripCountCalculation(OrigTC);
    TC->setName(Name);
    insert(TC);
    return TC;
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

  public:
    InsertPointGuard(VPBuilder &B)
        : Builder(B), Block(B.getInsertBlock()), Point(B.getInsertPoint()) {}

    InsertPointGuard(const InsertPointGuard &) = delete;
    InsertPointGuard &operator=(const InsertPointGuard &) = delete;

    ~InsertPointGuard() {
      Builder.restoreIP(VPInsertPoint(Block, Point));
    }
  };
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANBUILDER_H
