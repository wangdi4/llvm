//===------- HLNodeUtils.cpp - Implements HLNodeUtils class ---------------===//
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
// This file implements HLNodeUtils class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_LoopAnalysis/HIRLoopStatistics.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRInvalidationUtils.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Metadata.h" // needed for MetadataAsValue -> Value
#include "llvm/Support/Debug.h"

#include <memory>

#define DEBUG_TYPE "hir-hlnode-utils"
using namespace llvm;
using namespace loopopt;

CanonExprUtils &HLNodeUtils::getCanonExprUtils() {
  return getDDRefUtils().getCanonExprUtils();
}

const CanonExprUtils &HLNodeUtils::getCanonExprUtils() const {
  return getDDRefUtils().getCanonExprUtils();
}

BlobUtils &HLNodeUtils::getBlobUtils() {
  return getDDRefUtils().getBlobUtils();
}
const BlobUtils &HLNodeUtils::getBlobUtils() const {
  return getDDRefUtils().getBlobUtils();
}

Function &HLNodeUtils::getFunction() const {
  return getHIRFramework().getFunction();
}

Module &HLNodeUtils::getModule() const { return getHIRFramework().getModule(); }

LLVMContext &HLNodeUtils::getContext() const {
  return getHIRFramework().getContext();
}

const DataLayout &HLNodeUtils::getDataLayout() const {
  return getHIRFramework().getDataLayout();
}

void HLNodeUtils::reset(Function &F) {
  DDRU = nullptr;

  DummyIRBuilder = new DummyIRBuilderTy(F.getContext());
  DummyIRBuilder->SetInsertPoint(F.getEntryBlock().getTerminator());

  FirstDummyInst = nullptr;
  LastDummyInst = nullptr;
  Marker = nullptr;
}

HLRegion *HLNodeUtils::createHLRegion(IRRegion &IRReg) {
  return new HLRegion(*this, IRReg);
}

HLSwitch *HLNodeUtils::createHLSwitch(RegDDRef *ConditionRef) {
  return new HLSwitch(*this, ConditionRef);
}

HLLabel *HLNodeUtils::createHLLabel(BasicBlock *SrcBB) {
  return new HLLabel(*this, SrcBB);
}

HLLabel *HLNodeUtils::createHLLabel(const Twine &Name) {
  return new HLLabel(*this, Name);
}

HLGoto *HLNodeUtils::createHLGoto(BasicBlock *TargetBB) {
  return new HLGoto(*this, TargetBB);
}

HLGoto *HLNodeUtils::createHLGoto(HLLabel *TargetL) {
  return new HLGoto(*this, TargetL);
}

HLInst *HLNodeUtils::createHLInst(Instruction *Inst) {
  return new HLInst(*this, Inst);
}

HLIf *HLNodeUtils::createHLIf(const HLPredicate &FirstPred, RegDDRef *Ref1,
                              RegDDRef *Ref2) {
  return new HLIf(*this, FirstPred, Ref1, Ref2);
}

HLLoop *HLNodeUtils::createHLLoop(const Loop *LLVMLoop) {
  return new HLLoop(*this, LLVMLoop);
}

HLLoop *HLNodeUtils::createHLLoop(HLIf *ZttIf, RegDDRef *LowerDDRef,
                                  RegDDRef *UpperDDRef, RegDDRef *StrideDDRef,
                                  unsigned NumEx) {
  return new HLLoop(*this, ZttIf, LowerDDRef, UpperDDRef, StrideDDRef, NumEx);
}

void HLNodeUtils::destroy(HLNode *Node) {
  auto Count = Objs.erase(Node);
  assert(Count && "Node not found in objects!");

  delete Node;
}

void HLNodeUtils::setFirstAndLastDummyInst(Instruction *Inst) {
  if (!FirstDummyInst) {
    FirstDummyInst = Inst;
  }

  LastDummyInst = Inst;
}

void HLNodeUtils::destroyAll() {
  for (auto &I : Objs) {
    delete I;
  }

  Objs.clear();

  delete DummyIRBuilder;
}

void HLNodeUtils::checkUnaryInstOperands(RegDDRef *LvalRef, RegDDRef *RvalRef,
                                         Type *DestTy) {
  assert(RvalRef && "Rval is null!");

  if (LvalRef) {
    bool SameType = DestTy ? (LvalRef->getDestType() == DestTy)
                           : (LvalRef->getDestType() == RvalRef->getDestType());
    (void)SameType;
    assert(SameType && "Operand types do not match!");
  }
}

void HLNodeUtils::checkBinaryInstOperands(RegDDRef *LvalRef, RegDDRef *OpRef1,
                                          RegDDRef *OpRef2) {
  assert(OpRef1 && OpRef2 && "Operands are null!");

  auto ElTy = OpRef1->getDestType();

  (void)ElTy;
  assert((ElTy == OpRef2->getDestType()) && "Operand types do not match!");
  assert((!LvalRef || (ElTy == LvalRef->getDestType())) &&
         "Operand types do not match!");
}

HLInst *HLNodeUtils::createLvalHLInst(Instruction *Inst, RegDDRef *LvalRef) {

  setFirstAndLastDummyInst(Inst);

  auto HInst = createHLInst(Inst);

  if (!LvalRef) {
    LvalRef = getDDRefUtils().createSelfBlobRef(Inst);
  }

  HInst->setLvalDDRef(LvalRef);

  return HInst;
}

HLInst *HLNodeUtils::createNonLvalHLInst(Instruction *Inst) {

  setFirstAndLastDummyInst(Inst);

  return createHLInst(Inst);
}

HLInst *HLNodeUtils::createUnaryHLInst(unsigned OpCode, RegDDRef *RvalRef,
                                       const Twine &Name, RegDDRef *LvalRef,
                                       Type *DestTy) {
  Value *InstVal = nullptr;
  Instruction *Inst = nullptr;
  HLInst *HInst = nullptr;

  checkUnaryInstOperands(LvalRef, RvalRef, DestTy);

  // Create dummy val.
  auto DummyVal = UndefValue::get(RvalRef->getDestType());

  switch (OpCode) {
  case Instruction::Load: {
    assert(RvalRef->isMemRef() &&
           "Rval of load instruction should be a mem ref!");

    auto DummyPtrType =
        PointerType::get(RvalRef->getDestType(),
                         RvalRef->getBaseDestType()->getPointerAddressSpace());
    auto DummyPtrVal = UndefValue::get(DummyPtrType);

    InstVal = DummyIRBuilder->CreateLoad(DummyPtrVal, false, Name);
    break;
  }

  case Instruction::Store: {
    assert(LvalRef && LvalRef->isMemRef() &&
           "Lval of store instruction should be a non-null mem ref");

    auto DummyPtrType =
        PointerType::get(LvalRef->getDestType(),
                         LvalRef->getBaseDestType()->getPointerAddressSpace());
    auto DummyPtrVal = UndefValue::get(DummyPtrType);

    InstVal = DummyIRBuilder->CreateStore(DummyVal, DummyPtrVal);
    break;
  }

  case Instruction::Trunc:
  case Instruction::ZExt:
  case Instruction::SExt:
  case Instruction::FPToUI:
  case Instruction::FPToSI:
  case Instruction::UIToFP:
  case Instruction::SIToFP:
  case Instruction::FPTrunc:
  case Instruction::FPExt:
  case Instruction::PtrToInt:
  case Instruction::IntToPtr:
  case Instruction::AddrSpaceCast: {

    InstVal = DummyIRBuilder->CreateCast((Instruction::CastOps)OpCode, DummyVal,
                                         DestTy, Name);
    break;
  }

  case Instruction::BitCast: {
    // IRBuilder CreateCast returns input value when DestType is the same as
    // the value's type. In such cases, the return value is not guaranteed to
    // be an instruction as isa<Instruction>(DummyVal) is not necessarily true.
    // We take care of such cases by forcing creation of a copy Instruction
    // here.
    if (DestTy == DummyVal->getType()) {
      return createCopyInst(RvalRef, Name, LvalRef);
    } else {
      InstVal = DummyIRBuilder->CreateCast((Instruction::CastOps)OpCode,
                                           DummyVal, DestTy, Name);
    }
    break;
  }

  default:
    assert(false && "Instruction not handled!");
  }

  assert(isa<Instruction>(InstVal) && "Expected instruction!");
  Inst = cast<Instruction>(InstVal);

  HInst = createLvalHLInst(Inst, LvalRef);
  HInst->setRvalDDRef(RvalRef);

  return HInst;
}

Instruction *HLNodeUtils::createCopyInstImpl(Type *Ty, const Twine &Name) {
  // Create dummy val.
  auto DummyVal = UndefValue::get(Ty);

  // Cannot use IRBuilder here as it returns the same value for casts with
  // identical src and dest types.
  Value *InstVal = CastInst::Create(Instruction::BitCast, DummyVal,
                                    DummyVal->getType(), Name);

  auto Inst = cast<Instruction>(InstVal);
  Inst->insertBefore(&*(DummyIRBuilder->GetInsertPoint()));

  return Inst;
}

RegDDRef *HLNodeUtils::createTemp(Type *Ty, const Twine &Name) {
  auto Inst = createCopyInstImpl(Ty, Name);

  return getDDRefUtils().createSelfBlobRef(Inst);
}

HLInst *HLNodeUtils::createCopyInst(RegDDRef *RvalRef, const Twine &Name,
                                    RegDDRef *LvalRef) {
  checkUnaryInstOperands(LvalRef, RvalRef, nullptr);

  auto Inst = createCopyInstImpl(RvalRef->getDestType(), Name);

  auto HInst = createLvalHLInst(Inst, LvalRef);
  HInst->setRvalDDRef(RvalRef);

  return HInst;
}

HLInst *HLNodeUtils::createLoad(RegDDRef *RvalRef, const Twine &Name,
                                RegDDRef *LvalRef) {
  return createUnaryHLInst(Instruction::Load, RvalRef, Name, LvalRef, nullptr);
}

HLInst *HLNodeUtils::createStore(RegDDRef *RvalRef, const Twine &Name,
                                 RegDDRef *LvalRef) {
  return createUnaryHLInst(Instruction::Store, RvalRef, Name, LvalRef, nullptr);
}

HLInst *HLNodeUtils::createTrunc(Type *DestTy, RegDDRef *RvalRef,
                                 const Twine &Name, RegDDRef *LvalRef) {
  return createUnaryHLInst(Instruction::Trunc, RvalRef, Name, LvalRef, DestTy);
}

HLInst *HLNodeUtils::createZExt(Type *DestTy, RegDDRef *RvalRef,
                                const Twine &Name, RegDDRef *LvalRef) {
  return createUnaryHLInst(Instruction::ZExt, RvalRef, Name, LvalRef, DestTy);
}

HLInst *HLNodeUtils::createSExt(Type *DestTy, RegDDRef *RvalRef,
                                const Twine &Name, RegDDRef *LvalRef) {
  return createUnaryHLInst(Instruction::SExt, RvalRef, Name, LvalRef, DestTy);
}

HLInst *HLNodeUtils::createFPToUI(Type *DestTy, RegDDRef *RvalRef,
                                  const Twine &Name, RegDDRef *LvalRef) {
  return createUnaryHLInst(Instruction::FPToUI, RvalRef, Name, LvalRef, DestTy);
}

HLInst *HLNodeUtils::createFPToSI(Type *DestTy, RegDDRef *RvalRef,
                                  const Twine &Name, RegDDRef *LvalRef) {
  return createUnaryHLInst(Instruction::FPToSI, RvalRef, Name, LvalRef, DestTy);
}

HLInst *HLNodeUtils::createUIToFP(Type *DestTy, RegDDRef *RvalRef,
                                  const Twine &Name, RegDDRef *LvalRef) {
  return createUnaryHLInst(Instruction::UIToFP, RvalRef, Name, LvalRef, DestTy);
}

HLInst *HLNodeUtils::createSIToFP(Type *DestTy, RegDDRef *RvalRef,
                                  const Twine &Name, RegDDRef *LvalRef) {
  return createUnaryHLInst(Instruction::SIToFP, RvalRef, Name, LvalRef, DestTy);
}

HLInst *HLNodeUtils::createFPTrunc(Type *DestTy, RegDDRef *RvalRef,
                                   const Twine &Name, RegDDRef *LvalRef) {
  return createUnaryHLInst(Instruction::FPTrunc, RvalRef, Name, LvalRef,
                           DestTy);
}

HLInst *HLNodeUtils::createFPExt(Type *DestTy, RegDDRef *RvalRef,
                                 const Twine &Name, RegDDRef *LvalRef) {
  return createUnaryHLInst(Instruction::FPExt, RvalRef, Name, LvalRef, DestTy);
}

HLInst *HLNodeUtils::createCastHLInst(Type *DestTy, unsigned Opcode,
                                      RegDDRef *Op, const Twine &Name,
                                      RegDDRef *LvalRef) {
  switch (Opcode) {
  case Instruction::FPToSI:
    return createFPToSI(DestTy, Op, Name, LvalRef);
  case Instruction::FPToUI:
    return createFPToUI(DestTy, Op, Name, LvalRef);
  case Instruction::SIToFP:
    return createSIToFP(DestTy, Op, Name, LvalRef);
  case Instruction::UIToFP:
    return createUIToFP(DestTy, Op, Name, LvalRef);
  case Instruction::FPExt:
    return createFPExt(DestTy, Op, Name, LvalRef);
  case Instruction::FPTrunc:
    return createFPTrunc(DestTy, Op, Name, LvalRef);
  case Instruction::SExt:
    return createSExt(DestTy, Op, Name, LvalRef);
  case Instruction::ZExt:
    return createZExt(DestTy, Op, Name, LvalRef);
  case Instruction::Trunc:
    return createTrunc(DestTy, Op, Name, LvalRef);
  case Instruction::BitCast:
    return createBitCast(DestTy, Op, Name, LvalRef);
  case Instruction::PtrToInt:
    return createPtrToInt(DestTy, Op, Name, LvalRef);
  case Instruction::IntToPtr:
    return createIntToPtr(DestTy, Op, Name, LvalRef);
  default:
    llvm_unreachable("Unexpected cast opcode");
  }
}

HLInst *HLNodeUtils::createPtrToInt(Type *DestTy, RegDDRef *RvalRef,
                                    const Twine &Name, RegDDRef *LvalRef) {
  return createUnaryHLInst(Instruction::PtrToInt, RvalRef, Name, LvalRef,
                           DestTy);
}

HLInst *HLNodeUtils::createIntToPtr(Type *DestTy, RegDDRef *RvalRef,
                                    const Twine &Name, RegDDRef *LvalRef) {
  return createUnaryHLInst(Instruction::IntToPtr, RvalRef, Name, LvalRef,
                           DestTy);
}

HLInst *HLNodeUtils::createBitCast(Type *DestTy, RegDDRef *RvalRef,
                                   const Twine &Name, RegDDRef *LvalRef) {
  return createUnaryHLInst(Instruction::BitCast, RvalRef, Name, LvalRef,
                           DestTy);
}

HLInst *HLNodeUtils::createAddrSpaceCast(Type *DestTy, RegDDRef *RvalRef,
                                         const Twine &Name, RegDDRef *LvalRef) {
  return createUnaryHLInst(Instruction::AddrSpaceCast, RvalRef, Name, LvalRef,
                           DestTy);
}

HLInst *HLNodeUtils::createBinaryHLInstImpl(unsigned OpCode, RegDDRef *OpRef1,
                                            RegDDRef *OpRef2, const Twine &Name,
                                            RegDDRef *LvalRef,
                                            bool HasNUWOrExact, bool HasNSW,
                                            MDNode *FPMathTag) {
  Value *InstVal;
  Instruction *Inst;
  HLInst *HInst;

  checkBinaryInstOperands(LvalRef, OpRef1, OpRef2);

  // Create dummy val.
  auto DummyVal = UndefValue::get(OpRef1->getDestType());

  switch (OpCode) {
  case Instruction::Add: {
    assert(OpRef1->getDestType()->isIntOrIntVectorTy() &&
           "Operand is not an integer type!");
    InstVal = DummyIRBuilder->CreateAdd(DummyVal, DummyVal, Name, HasNUWOrExact,
                                        HasNSW);
    break;
  }

  case Instruction::FAdd: {
    assert(OpRef1->getDestType()->isFPOrFPVectorTy() &&
           "Operand is not a floating point type!");
    InstVal = DummyIRBuilder->CreateFAdd(DummyVal, DummyVal, Name, FPMathTag);
    break;
  }

  case Instruction::Sub: {
    assert(OpRef1->getDestType()->isIntOrIntVectorTy() &&
           "Operand is not an integer type!");
    InstVal = DummyIRBuilder->CreateSub(DummyVal, DummyVal, Name, HasNUWOrExact,
                                        HasNSW);
    break;
  }

  case Instruction::FSub: {
    assert(OpRef1->getDestType()->isFPOrFPVectorTy() &&
           "Operand is not a floating point type!");
    InstVal = DummyIRBuilder->CreateFSub(DummyVal, DummyVal, Name, FPMathTag);
    break;
  }

  case Instruction::Mul: {
    assert(OpRef1->getDestType()->isIntOrIntVectorTy() &&
           "Operand is not an integer type!");
    InstVal = DummyIRBuilder->CreateMul(DummyVal, DummyVal, Name, HasNUWOrExact,
                                        HasNSW);
    break;
  }

  case Instruction::FMul: {
    assert(OpRef1->getDestType()->isFPOrFPVectorTy() &&
           "Operand is not a floating point type!");
    InstVal = DummyIRBuilder->CreateFMul(DummyVal, DummyVal, Name, FPMathTag);
    break;
  }

  case Instruction::UDiv: {
    assert(OpRef1->getDestType()->isIntOrIntVectorTy() &&
           "Operand is not an integer type!");
    InstVal =
        DummyIRBuilder->CreateUDiv(DummyVal, DummyVal, Name, HasNUWOrExact);
    break;
  }

  case Instruction::SDiv: {
    assert(OpRef1->getDestType()->isIntOrIntVectorTy() &&
           "Operand is not an integer type!");
    InstVal =
        DummyIRBuilder->CreateSDiv(DummyVal, DummyVal, Name, HasNUWOrExact);
    break;
  }

  case Instruction::FDiv: {
    assert(OpRef1->getDestType()->isFPOrFPVectorTy() &&
           "Operand is not a floating point type!");
    InstVal = DummyIRBuilder->CreateFDiv(DummyVal, DummyVal, Name, FPMathTag);
    break;
  }

  case Instruction::URem: {
    assert(OpRef1->getDestType()->isIntOrIntVectorTy() &&
           "Operand is not an integer type!");
    InstVal = DummyIRBuilder->CreateURem(DummyVal, DummyVal, Name);
    break;
  }

  case Instruction::SRem: {
    assert(OpRef1->getDestType()->isIntOrIntVectorTy() &&
           "Operand is not an integer type!");
    InstVal = DummyIRBuilder->CreateSRem(DummyVal, DummyVal, Name);
    break;
  }

  case Instruction::FRem: {
    assert(OpRef1->getDestType()->isFPOrFPVectorTy() &&
           "Operand is not a floating point type!");
    InstVal = DummyIRBuilder->CreateFRem(DummyVal, DummyVal, Name, FPMathTag);
    break;
  }

  case Instruction::Shl: {
    assert(OpRef1->getDestType()->isIntOrIntVectorTy() &&
           "Operand is not an integer type!");
    InstVal = DummyIRBuilder->CreateShl(DummyVal, DummyVal, Name, HasNUWOrExact,
                                        HasNSW);
    break;
  }

  case Instruction::LShr: {
    assert(OpRef1->getDestType()->isIntOrIntVectorTy() &&
           "Operand is not an integer type!");
    InstVal =
        DummyIRBuilder->CreateLShr(DummyVal, DummyVal, Name, HasNUWOrExact);
    break;
  }

  case Instruction::AShr: {
    assert(OpRef1->getDestType()->isIntOrIntVectorTy() &&
           "Operand is not an integer type!");
    InstVal =
        DummyIRBuilder->CreateAShr(DummyVal, DummyVal, Name, HasNUWOrExact);
    break;
  }

  case Instruction::And: {
    assert(OpRef1->getDestType()->isIntOrIntVectorTy() &&
           "Operand is not an integer type!");
    InstVal = DummyIRBuilder->CreateAnd(DummyVal, DummyVal, Name);
    break;
  }

  case Instruction::Or: {
    assert(OpRef1->getDestType()->isIntOrIntVectorTy() &&
           "Operand is not an integer type!");
    InstVal = DummyIRBuilder->CreateOr(DummyVal, DummyVal, Name);
    break;
  }

  case Instruction::Xor: {
    assert(OpRef1->getDestType()->isIntOrIntVectorTy() &&
           "Operand is not an integer type!");
    InstVal = DummyIRBuilder->CreateXor(DummyVal, DummyVal, Name);
    break;
  }

  default:
    llvm_unreachable("Unknown binary op code!");
  }

  Inst = cast<Instruction>(InstVal);

  HInst = createLvalHLInst(Inst, LvalRef);

  HInst->setOperandDDRef(OpRef1, 1);
  HInst->setOperandDDRef(OpRef2, 2);

  return HInst;
}

HLInst *HLNodeUtils::createShuffleVectorInst(RegDDRef *OpRef1, RegDDRef *OpRef2,
                                             ArrayRef<uint32_t> Mask,
                                             const Twine &Name,
                                             RegDDRef *LvalRef) {
  assert(OpRef1->getDestType()->isVectorTy() &&
         OpRef1->getDestType() == OpRef2->getDestType() &&
         "Illegal operand types for shufflevector");

  auto OneVal = UndefValue::get(OpRef1->getDestType());

  Value *InstVal =
      DummyIRBuilder->CreateShuffleVector(OneVal, OneVal, Mask, Name);
  Instruction *Inst = cast<Instruction>(InstVal);

  assert((!LvalRef || LvalRef->getDestType() == Inst->getType()) &&
         "Incompatible type of LvalRef");

  HLInst *HInst = createLvalHLInst(Inst, LvalRef);

  HInst->setOperandDDRef(OpRef1, 1);
  HInst->setOperandDDRef(OpRef2, 2);
  Value *MaskVecValue = Inst->getOperand(2);
  RegDDRef *MaskVecDDRef;
  if (isa<ConstantAggregateZero>(MaskVecValue))
    MaskVecDDRef = getDDRefUtils().createConstDDRef(
        cast<ConstantAggregateZero>(MaskVecValue));
  else if (isa<ConstantDataVector>(MaskVecValue))
    MaskVecDDRef = getDDRefUtils().createConstDDRef(
        cast<ConstantDataVector>(MaskVecValue));
  else
    llvm_unreachable("Unexpected Mask vector type");
  HInst->setOperandDDRef(MaskVecDDRef, 3);
  return HInst;
}

HLInst *HLNodeUtils::createExtractElementInst(RegDDRef *OpRef, unsigned Idx,
                                              const Twine &Name,
                                              RegDDRef *LvalRef) {

  assert(OpRef->getDestType()->isVectorTy() &&
         "Illegal operand types for extractelement");

  auto OneVal = UndefValue::get(OpRef->getDestType());
  Value *InstVal = DummyIRBuilder->CreateExtractElement(OneVal, Idx, Name);
  Instruction *Inst = cast<Instruction>(InstVal);
  assert((!LvalRef || LvalRef->getDestType() == Inst->getType()) &&
         "Incompatible type of LvalRef");

  HLInst *HInst = createLvalHLInst(Inst, LvalRef);

  HInst->setOperandDDRef(OpRef, 1);

  RegDDRef *IdxDDref =
      getDDRefUtils().createConstDDRef(Inst->getOperand(1)->getType(), Idx);
  HInst->setOperandDDRef(IdxDDref, 2);

  return HInst;
}

HLInst *HLNodeUtils::createBinaryHLInst(unsigned OpCode, RegDDRef *OpRef1,
                                        RegDDRef *OpRef2, const Twine &Name,
                                        RegDDRef *LvalRef,
                                        const BinaryOperator *OrigBinOp) {
  HLInst *HInst;

  HInst = createBinaryHLInstImpl(OpCode, OpRef1, OpRef2, Name, LvalRef, false,
                                 false, nullptr);
  if (OrigBinOp) {
    BinaryOperator *NewBinOp = cast<BinaryOperator>(
        const_cast<Instruction *>(HInst->getLLVMInstruction()));
    NewBinOp->copyIRFlags(OrigBinOp);
  }

  return HInst;
}

HLInst *HLNodeUtils::createAdd(RegDDRef *OpRef1, RegDDRef *OpRef2,
                               const Twine &Name, RegDDRef *LvalRef,
                               bool HasNUW, bool HasNSW) {
  return createBinaryHLInstImpl(Instruction::Add, OpRef1, OpRef2, Name, LvalRef,
                                HasNUW, HasNSW, nullptr);
}

HLInst *HLNodeUtils::createFAdd(RegDDRef *OpRef1, RegDDRef *OpRef2,
                                const Twine &Name, RegDDRef *LvalRef,
                                MDNode *FPMathTag) {
  return createBinaryHLInstImpl(Instruction::FAdd, OpRef1, OpRef2, Name,
                                LvalRef, false, false, FPMathTag);
}

HLInst *HLNodeUtils::createSub(RegDDRef *OpRef1, RegDDRef *OpRef2,
                               const Twine &Name, RegDDRef *LvalRef,
                               bool HasNUW, bool HasNSW) {
  return createBinaryHLInstImpl(Instruction::Sub, OpRef1, OpRef2, Name, LvalRef,
                                HasNUW, HasNSW, nullptr);
}

HLInst *HLNodeUtils::createFSub(RegDDRef *OpRef1, RegDDRef *OpRef2,
                                const Twine &Name, RegDDRef *LvalRef,
                                MDNode *FPMathTag) {
  return createBinaryHLInstImpl(Instruction::FSub, OpRef1, OpRef2, Name,
                                LvalRef, false, false, FPMathTag);
}

HLInst *HLNodeUtils::createMul(RegDDRef *OpRef1, RegDDRef *OpRef2,
                               const Twine &Name, RegDDRef *LvalRef,
                               bool HasNUW, bool HasNSW) {
  return createBinaryHLInstImpl(Instruction::Mul, OpRef1, OpRef2, Name, LvalRef,
                                HasNUW, HasNSW, nullptr);
}

HLInst *HLNodeUtils::createFMul(RegDDRef *OpRef1, RegDDRef *OpRef2,
                                const Twine &Name, RegDDRef *LvalRef,
                                MDNode *FPMathTag) {
  return createBinaryHLInstImpl(Instruction::FMul, OpRef1, OpRef2, Name,
                                LvalRef, false, false, FPMathTag);
}

HLInst *HLNodeUtils::createUDiv(RegDDRef *OpRef1, RegDDRef *OpRef2,
                                const Twine &Name, RegDDRef *LvalRef,
                                bool IsExact) {
  return createBinaryHLInstImpl(Instruction::UDiv, OpRef1, OpRef2, Name,
                                LvalRef, IsExact, false, nullptr);
}

HLInst *HLNodeUtils::createSDiv(RegDDRef *OpRef1, RegDDRef *OpRef2,
                                const Twine &Name, RegDDRef *LvalRef,
                                bool IsExact) {
  return createBinaryHLInstImpl(Instruction::SDiv, OpRef1, OpRef2, Name,
                                LvalRef, IsExact, false, nullptr);
}

HLInst *HLNodeUtils::createFDiv(RegDDRef *OpRef1, RegDDRef *OpRef2,
                                const Twine &Name, RegDDRef *LvalRef,
                                MDNode *FPMathTag) {
  return createBinaryHLInstImpl(Instruction::FDiv, OpRef1, OpRef2, Name,
                                LvalRef, false, false, FPMathTag);
}

HLInst *HLNodeUtils::createURem(RegDDRef *OpRef1, RegDDRef *OpRef2,
                                const Twine &Name, RegDDRef *LvalRef) {
  return createBinaryHLInstImpl(Instruction::URem, OpRef1, OpRef2, Name,
                                LvalRef, false, false, nullptr);
}

HLInst *HLNodeUtils::createSRem(RegDDRef *OpRef1, RegDDRef *OpRef2,
                                const Twine &Name, RegDDRef *LvalRef) {
  return createBinaryHLInstImpl(Instruction::SRem, OpRef1, OpRef2, Name,
                                LvalRef, false, false, nullptr);
}

HLInst *HLNodeUtils::createFRem(RegDDRef *OpRef1, RegDDRef *OpRef2,
                                const Twine &Name, RegDDRef *LvalRef,
                                MDNode *FPMathTag) {
  return createBinaryHLInstImpl(Instruction::FRem, OpRef1, OpRef2, Name,
                                LvalRef, false, false, FPMathTag);
}

HLInst *HLNodeUtils::createShl(RegDDRef *OpRef1, RegDDRef *OpRef2,
                               const Twine &Name, RegDDRef *LvalRef,
                               bool HasNUW, bool HasNSW) {
  return createBinaryHLInstImpl(Instruction::Shl, OpRef1, OpRef2, Name, LvalRef,
                                HasNUW, HasNSW, nullptr);
}

HLInst *HLNodeUtils::createLShr(RegDDRef *OpRef1, RegDDRef *OpRef2,
                                const Twine &Name, RegDDRef *LvalRef,
                                bool IsExact) {
  return createBinaryHLInstImpl(Instruction::LShr, OpRef1, OpRef2, Name,
                                LvalRef, IsExact, false, nullptr);
}

HLInst *HLNodeUtils::createAShr(RegDDRef *OpRef1, RegDDRef *OpRef2,
                                const Twine &Name, RegDDRef *LvalRef,
                                bool IsExact) {
  return createBinaryHLInstImpl(Instruction::AShr, OpRef1, OpRef2, Name,
                                LvalRef, IsExact, false, nullptr);
}

HLInst *HLNodeUtils::createAnd(RegDDRef *OpRef1, RegDDRef *OpRef2,
                               const Twine &Name, RegDDRef *LvalRef) {
  return createBinaryHLInstImpl(Instruction::And, OpRef1, OpRef2, Name, LvalRef,
                                false, false, nullptr);
}

HLInst *HLNodeUtils::createOr(RegDDRef *OpRef1, RegDDRef *OpRef2,
                              const Twine &Name, RegDDRef *LvalRef) {
  return createBinaryHLInstImpl(Instruction::Or, OpRef1, OpRef2, Name, LvalRef,
                                false, false, nullptr);
}

HLInst *HLNodeUtils::createXor(RegDDRef *OpRef1, RegDDRef *OpRef2,
                               const Twine &Name, RegDDRef *LvalRef) {
  return createBinaryHLInstImpl(Instruction::Xor, OpRef1, OpRef2, Name, LvalRef,
                                false, false, nullptr);
}

HLInst *HLNodeUtils::createCmp(const HLPredicate &Pred, RegDDRef *OpRef1,
                               RegDDRef *OpRef2, const Twine &Name,
                               RegDDRef *LvalRef) {
  Value *InstVal;
  HLInst *HInst;

  checkBinaryInstOperands(nullptr, OpRef1, OpRef2);

  if (LvalRef) {
    auto LType = LvalRef->getDestType()->getScalarType();
    assert((LType->isIntegerTy() && (LType->getIntegerBitWidth() == 1)) &&
           "LvalRef has invalid type!");
  }

  auto DummyVal = UndefValue::get(OpRef1->getDestType());

  if (OpRef1->getDestType()->isIntOrIntVectorTy() ||
      OpRef1->getDestType()->isPtrOrPtrVectorTy()) {
    InstVal =
        DummyIRBuilder->CreateICmp(ICmpInst::ICMP_EQ, DummyVal, DummyVal, Name);
  } else {
    InstVal = DummyIRBuilder->CreateFCmp(FCmpInst::FCMP_TRUE, DummyVal,
                                         DummyVal, Name);
  }

  HInst = createLvalHLInst(cast<Instruction>(InstVal), LvalRef);
  HInst->setPredicate(Pred);

  HInst->setOperandDDRef(OpRef1, 1);
  HInst->setOperandDDRef(OpRef2, 2);

  return HInst;
}

HLInst *HLNodeUtils::createSelect(const HLPredicate &Pred, RegDDRef *OpRef1,
                                  RegDDRef *OpRef2, RegDDRef *OpRef3,
                                  RegDDRef *OpRef4, const Twine &Name,
                                  RegDDRef *LvalRef) {
  Value *InstVal;
  HLInst *HInst;

  // LvalRef, OpRef3 and OpRef4 should be the same type.
  checkBinaryInstOperands(LvalRef, OpRef3, OpRef4);
  // OpRef1 and OpRef2 should be the same type.
  checkBinaryInstOperands(nullptr, OpRef1, OpRef2);

  auto CmpVal =
      UndefValue::get(Type::getInt1Ty(getHIRFramework().getContext()));
  auto DummyVal = UndefValue::get(OpRef3->getDestType());

  InstVal = DummyIRBuilder->CreateSelect(CmpVal, DummyVal, DummyVal, Name);

  HInst = createLvalHLInst(cast<Instruction>(InstVal), LvalRef);
  HInst->setPredicate(Pred);

  HInst->setOperandDDRef(OpRef1, 1);
  HInst->setOperandDDRef(OpRef2, 2);
  HInst->setOperandDDRef(OpRef3, 3);
  HInst->setOperandDDRef(OpRef4, 4);

  return HInst;
}

HLInst *HLNodeUtils::createCall(Function *Func,
                                const SmallVectorImpl<RegDDRef *> &CallArgs,
                                const Twine &Name, RegDDRef *LvalRef) {
  bool HasReturn = !Func->getReturnType()->isVoidTy();
  unsigned NumArgs = CallArgs.size();
  HLInst *HInst;
  SmallVector<Value *, 8> Args;

  for (unsigned I = 0; I < NumArgs; I++) {
    MetadataAsValue *Val = nullptr;
    Args.push_back(CallArgs[I]->isMetadata(&Val)
                       ? cast<Value>(Val)
                       : UndefValue::get(CallArgs[I]->getDestType()));
  }
  auto InstVal = DummyIRBuilder->CreateCall(
      Func, Args, HasReturn ? (Name.isTriviallyEmpty() ? "dummy" : Name) : "");
  if (HasReturn) {
    //    HInst = createLvalHLInst(cast<Instruction>(InstVal), LvalRef);
    HInst = createLvalHLInst(InstVal, LvalRef);
  } else {
    HInst = createNonLvalHLInst(InstVal);
  }

  // For non-void functions, the return DDRef position for the HLInst
  // representing the call is at index 0 and the 1st argument DDRef is
  // at position 1. For void functions, the DDRef for the first argument
  // starts at 0. ArgOffset gets added to the index of the loop below to
  // determine the correct DDRef index.
  unsigned ArgOffset = 0;
  if (HasReturn) {
    ArgOffset = 1;
  }

  for (unsigned I = 0; I < NumArgs; I++) {
    HInst->setOperandDDRef(CallArgs[I], I + ArgOffset);
  }
  return HInst;
}

struct HLNodeUtils::CloneVisitor final : public HLNodeVisitorBase {

  HLContainerTy *CloneContainer;
  GotoContainerTy *GotoList;
  LabelMapTy *LabelMap;
  HLNodeMapper *NodeMapper;

  CloneVisitor(HLContainerTy *Container, GotoContainerTy *GList,
               LabelMapTy *LMap, HLNodeMapper *NodeMapper)
      : CloneContainer(Container), GotoList(GList), LabelMap(LMap),
        NodeMapper(NodeMapper) {}

  void visit(const HLNode *Node) {
    CloneContainer->push_back(
        *HLNode::cloneBaseImpl(Node, GotoList, LabelMap, NodeMapper));
  }

  void postVisit(const HLNode *Node) {}
};

void HLNodeUtils::updateGotos(GotoContainerTy *GotoList, LabelMapTy *LabelMap) {

  for (auto Iter = GotoList->begin(), End = GotoList->end(); Iter != End;
       ++Iter) {
    HLGoto *Goto = *Iter;
    auto LabelIt = LabelMap->find(Goto->getTargetLabel());
    if (LabelIt != LabelMap->end()) {
      // Update the Goto branch to new label
      Goto->setTargetLabel(LabelIt->second);
    }
  }
}

void HLNodeUtils::cloneSequenceImpl(HLContainerTy *CloneContainer,
                                    const HLNode *Node1, const HLNode *Node2,
                                    HLNodeMapper *NodeMapper) {

  GotoContainerTy GotoList;
  LabelMapTy LabelMap;

  // Check for Node2 as nullptr or a single node
  if (!Node2 || (Node1 == Node2)) {
    CloneContainer->push_back(
        *HLNode::cloneBaseImpl(Node1, &GotoList, &LabelMap, NodeMapper));
    updateGotos(&GotoList, &LabelMap);
    return;
  }

  HLNodeUtils::CloneVisitor CloneVisit(CloneContainer, &GotoList, &LabelMap,
                                       NodeMapper);
  visitRange<false>(CloneVisit, Node1, Node2);
  updateGotos(&GotoList, &LabelMap);
}

/// \brief Helper for updating loop info during insertion/removal.
///
/// It operates under two modes: finder and updater.
/// Under finder mode, it looks for a loop. This is used by the caller to set
/// the innermost flag during node removal.
/// Under updater mode, it updates nesting level and innermost flag of
/// involved loops. This mode is used during node insertion.
///
struct HLNodeUtils::LoopFinderUpdater final : public HLNodeVisitorBase {

  bool FinderMode;
  bool FoundLoop;

  LoopFinderUpdater(bool IsFinder) : FinderMode(IsFinder), FoundLoop(false) {}

  bool foundLoop() { return FoundLoop; }

  void visit(HLLoop *Loop) {
    if (FinderMode) {
      FoundLoop = true;
    } else {
      Loop->getHLNodeUtils().updateLoopInfo(Loop);
    }
  }

  /// Catch-all visit functions
  void visit(HLNode *Node) {}
  void postVisit(HLNode *Node) {}

  bool isDone() const override {
    if (FinderMode && FoundLoop) {
      return true;
    }
    return false;
  }
};

void HLNodeUtils::updateLoopInfo(HLLoop *Loop) {
  HLLoop *ParentLoop = Loop->getParentLoop();

  if (ParentLoop) {
    Loop->setNestingLevel(ParentLoop->getNestingLevel() + 1);
    ParentLoop->setInnermost(false);
  } else {
    Loop->setNestingLevel(1);
  }
}

void HLNodeUtils::updateLoopInfoRecursively(HLContainerTy::iterator First,
                                            HLContainerTy::iterator Last) {

  HLNodeUtils::LoopFinderUpdater LoopUpdater(false);
  visitRange(LoopUpdater, First, Last);
}

void HLNodeUtils::insertInternal(HLContainerTy &InsertContainer,
                                 HLContainerTy::iterator Pos,
                                 HLContainerTy *OrigContainer,
                                 HLContainerTy::iterator First,
                                 HLContainerTy::iterator Last) {
  if (!OrigContainer) {
    InsertContainer.insert(Pos, *(First));
  } else {
    InsertContainer.splice(Pos, *OrigContainer, First, Last);
  }
}

/// This function needs to update separators appropriately during insertion.
/// We are basically simulating multiple linklists using a single linklist.
//
/// Here's an example of the the update logic-
///
/// Suppose we want to insert a HLNode in case 3 of the HLSwitch shown
/// below.
///
/// switch (cond) {
/// case 1:
///      N1;
///      break;
/// case 2:
///      break;
/// case 3:
///      break;
/// }
///
/// Since case 2 and 3 are both empty, CaseBegin of both of them points to
/// HLSwitch's Children.end(). Now, since we are inserting an HLNode in case 3
/// we need to update CaseBegin of both case 2 and 3 since case 2 comes
/// lexically before 3. CaseBegin of case 1 remains unchanged and points to N1.
///
/// Resulting switch-
///
/// switch (cond) {
/// case 1:
///      N1;
///      break;
/// case 2:
///      break;
/// case 3:
///      N2;
///      break;
/// }
///
/// This scenario only occurs when there are empty linklist(s) which precede the
/// one we are inserting in.
///
/// The logic is common to all parent HLNode types which hold multiple linklists
/// like HLIf, HLLoop etc.
void HLNodeUtils::insertImpl(HLNode *Parent, HLContainerTy::iterator Pos,
                             HLContainerTy *OrigContainer,
                             HLContainerTy::iterator First,
                             HLContainerTy::iterator Last, bool UpdateSeparator,
                             bool PostExitSeparator, int CaseNum) {

  // 'First' can be invalid if this function is called using empty
  // OrigContainer's begin(). We should bail out early in this case otherwise
  // 'First' may be dereferenced incorrectly.
  // NOTE: We cannot compare First and Last here as insertBefore()/insertAfter()
  //       set them to the same node.
  if (OrigContainer && OrigContainer->empty()) {
    return;
  }

  assert(!isa<HLRegion>(First) && "Transformations should not add/reorder "
                                  "regions!");

  assert(Parent && "Parent is missing!");
  unsigned Count = 1;

  if (OrigContainer) {
    Count = std::distance(First, Last);
  } else {
    assert(!First->getParent() &&
           "Node is already linked, please remove it first!!");
  }

  // Update parent of topmost nodes. Inner nodes' parent remains the same.
  unsigned I = 0;
  for (auto It = First; I < Count; ++I, ++It) {
    It->setParent(Parent);
  }

  HLContainerTy *InsertContainer = nullptr;

  if (auto Reg = dyn_cast<HLRegion>(Parent)) {
    InsertContainer = &Reg->Children;
    insertInternal(Reg->Children, Pos, OrigContainer, First, Last);
  } else if (auto Loop = dyn_cast<HLLoop>(Parent)) {
    InsertContainer = &Loop->Children;
    insertInternal(Loop->Children, Pos, OrigContainer, First, Last);

    if (UpdateSeparator) {
      if (Pos == Loop->ChildBegin) {
        Loop->ChildBegin = std::prev(Pos, Count);
      }
      if (PostExitSeparator && (Pos == Loop->PostexitBegin)) {
        Loop->PostexitBegin = std::prev(Pos, Count);
      }
    }

  } else if (auto If = dyn_cast<HLIf>(Parent)) {
    InsertContainer = &If->Children;
    insertInternal(If->Children, Pos, OrigContainer, First, Last);

    if (UpdateSeparator && (Pos == If->ElseBegin)) {
      If->ElseBegin = std::prev(Pos, Count);
    }

  } else if (auto Switch = dyn_cast<HLSwitch>(Parent)) {
    InsertContainer = &Switch->Children;
    insertInternal(Switch->Children, Pos, OrigContainer, First, Last);

    // Update all the empty cases which are lexically before this one.
    if (UpdateSeparator) {
      // CaseNum is set to -1 by insertBefore(). It is a bit wasteful but
      // functionally correct to check all the cases.
      // If CaseNum is 0 it's required to update all separators, as default
      // separator is the last one by implementation.
      bool UpdateAll = (CaseNum == -1 || CaseNum == 0);
      unsigned E = UpdateAll ? Switch->getNumCases() : CaseNum;
      for (unsigned I = 0; I < E; ++I) {
        if (Pos == Switch->CaseBegin[I]) {
          Switch->CaseBegin[I] = std::prev(Pos, Count);
        }
      }
      if (UpdateAll && Pos == Switch->DefaultCaseBegin) {
        Switch->DefaultCaseBegin = std::prev(Pos, Count);
      }
    }

  } else {
    llvm_unreachable("Unknown parent type!");
  }

  assert(InsertContainer && "InsertContainer is null");

  // Skip updating info for disconnected nodes. This can happen during cloning.
  // The info will be updated when they are finally attached.
  if (First->isAttached()) {
    // Update TopSortNum
    updateTopSortNum(*InsertContainer, First, Pos);

    // Update loop info for loops in this range.
    updateLoopInfoRecursively(First, Pos);
  }
}

void HLNodeUtils::insertBefore(HLNode *Pos, HLNode *Node) {
  assert(Pos && "Pos is null!");
  assert(Node && "Node is null!");
  insertImpl(Pos->getParent(), Pos->getIterator(), nullptr, Node->getIterator(),
             Node->getIterator(), true, true);
}

void HLNodeUtils::insertBefore(HLNode *Pos, HLContainerTy *NodeContainer) {
  assert(Pos && "Pos is null!");
  assert(NodeContainer && "NodeContainer is null!");

  insertImpl(Pos->getParent(), Pos->getIterator(), NodeContainer,
             NodeContainer->begin(), NodeContainer->end(), true, true);
}

/// This function doesn't require updating separators as they point to the
/// beginning of a lexical scope and hence will remain unaffected by this
/// operation.
void HLNodeUtils::insertAfter(HLNode *Pos, HLNode *Node) {
  assert(Pos && "Pos is null!");
  assert(Node && "Node is null!");
  HLContainerTy::iterator It(Pos);

  insertImpl(Pos->getParent(), std::next(It), nullptr, Node->getIterator(),
             Node->getIterator(), false);
}

void HLNodeUtils::insertAfter(HLNode *Pos, HLContainerTy *NodeContainer) {
  assert(Pos && "Pos is null!");
  assert(NodeContainer && "NodeContainer is null!");
  HLContainerTy::iterator It(Pos);

  insertImpl(Pos->getParent(), std::next(It), NodeContainer,
             NodeContainer->begin(), NodeContainer->end(), false);
}

void HLNodeUtils::insertAsChildImpl(HLNode *Parent,
                                    HLContainerTy *OrigContainer,
                                    HLContainerTy::iterator First,
                                    HLContainerTy::iterator Last,
                                    bool IsFirstChild) {
  assert(Parent && " Parent is null.");

  if (auto Reg = dyn_cast<HLRegion>(Parent)) {
    insertImpl(Reg, IsFirstChild ? Reg->child_begin() : Reg->child_end(),
               OrigContainer, First, Last, false);
  } else if (auto Loop = dyn_cast<HLLoop>(Parent)) {
    insertImpl(Loop, IsFirstChild ? Loop->child_begin() : Loop->child_end(),
               OrigContainer, First, Last, true, false);
  } else {
    llvm_unreachable("Parent can only be region or loop!");
  }
}

void HLNodeUtils::insertAsFirstChild(HLRegion *Reg, HLNode *Node) {
  assert(Node && "Node is null!");
  insertAsChildImpl(Reg, nullptr, Node->getIterator(), Node->getIterator(),
                    true);
}

void HLNodeUtils::insertAsLastChild(HLRegion *Reg, HLNode *Node) {
  assert(Node && "Node is null!");
  insertAsChildImpl(Reg, nullptr, Node->getIterator(), Node->getIterator(),
                    false);
}

void HLNodeUtils::insertAsFirstChild(HLLoop *Loop, HLNode *Node) {
  assert(Node && "Node is null!");
  insertAsChildImpl(Loop, nullptr, Node->getIterator(), Node->getIterator(),
                    true);
}

void HLNodeUtils::insertAsFirstChildren(HLLoop *Loop,
                                        HLContainerTy *NodeContainer) {
  assert(NodeContainer && "NodeContainer is null!");
  insertAsChildImpl(Loop, NodeContainer, NodeContainer->begin(),
                    NodeContainer->end(), true);
}

void HLNodeUtils::insertAsLastChild(HLLoop *Loop, HLNode *Node) {
  assert(Node && "Node is null!");
  insertAsChildImpl(Loop, nullptr, Node->getIterator(), Node->getIterator(),
                    false);
}

void HLNodeUtils::insertAsLastChildren(HLLoop *Loop,
                                       HLContainerTy *NodeContainer) {
  assert(NodeContainer && "NodeContainer is null!");
  insertAsChildImpl(Loop, NodeContainer, NodeContainer->begin(),
                    NodeContainer->end(), false);
}

void HLNodeUtils::insertAsFirstChild(HLIf *If, HLNode *Node, bool IsThenChild) {
  assert(If && "If is null!");
  assert(Node && "Node is null!");

  insertImpl(If, IsThenChild ? If->then_begin() : If->else_begin(), nullptr,
             Node->getIterator(), Node->getIterator(), !IsThenChild);
}

void HLNodeUtils::insertAsLastChild(HLIf *If, HLNode *Node, bool IsThenChild) {
  assert(If && "If is null!");
  assert(Node && "Node is null!");

  insertImpl(If, IsThenChild ? If->then_end() : If->else_end(), nullptr,
             Node->getIterator(), Node->getIterator(), !IsThenChild);
}

void HLNodeUtils::insertAsChildImpl(HLSwitch *Switch,
                                    HLContainerTy *OrigContainer,
                                    HLContainerTy::iterator First,
                                    HLContainerTy::iterator Last,
                                    unsigned CaseNum, bool isFirstChild) {
  insertImpl(Switch,
             isFirstChild ? Switch->case_child_begin_internal(CaseNum)
                          : Switch->case_child_end_internal(CaseNum),
             OrigContainer, First, Last, true, false, CaseNum);
}

void HLNodeUtils::insertAsFirstDefaultChild(HLSwitch *Switch, HLNode *Node) {
  assert(Node && "Node is null!");
  insertAsChildImpl(Switch, nullptr, Node->getIterator(), Node->getIterator(),
                    0, true);
}

void HLNodeUtils::insertAsLastDefaultChild(HLSwitch *Switch, HLNode *Node) {
  assert(Node && "Node is null!");
  insertAsChildImpl(Switch, nullptr, Node->getIterator(), Node->getIterator(),
                    0, false);
}

void HLNodeUtils::insertAsFirstChild(HLSwitch *Switch, HLNode *Node,
                                     unsigned CaseNum) {
  assert(Node && "Node is null!");
  assert((CaseNum > 0) && (CaseNum <= Switch->getNumCases()) &&
         "CaseNum is out of range!");
  insertAsChildImpl(Switch, nullptr, Node->getIterator(), Node->getIterator(),
                    CaseNum, true);
}

void HLNodeUtils::insertAsLastChild(HLSwitch *Switch, HLNode *Node,
                                    unsigned CaseNum) {
  assert(Node && "Node is null!");
  assert((CaseNum > 0) && (CaseNum <= Switch->getNumCases()) &&
         "CaseNum is out of range!");
  insertAsChildImpl(Switch, nullptr, Node->getIterator(), Node->getIterator(),
                    CaseNum, false);
}

bool HLNodeUtils::validPreheaderPostexitNodes(HLContainerTy::iterator First,
                                              HLContainerTy::iterator Last) {

  for (auto I = First; I != Last; ++I) {
    if (!isa<HLInst>(*I)) {
      return false;
    }
  }

  return true;
}

void HLNodeUtils::insertAsPreheaderPostexitImpl(
    HLLoop *Loop, HLContainerTy *OrigContainer, HLContainerTy::iterator First,
    HLContainerTy::iterator Last, bool IsPreheader, bool IsFirstChild) {

  assert(validPreheaderPostexitNodes(First, Last) &&
         "Invalid preheader/postexit node encountered during insertion!");

  HLContainerTy::iterator Pos;

  Pos = IsPreheader ? (IsFirstChild ? Loop->pre_begin() : Loop->pre_end())
                    : (IsFirstChild ? Loop->post_begin() : Loop->post_end());

  insertImpl(Loop, Pos, OrigContainer, First, Last, !IsPreheader, !IsPreheader);
}

void HLNodeUtils::insertAsFirstPreheaderNode(HLLoop *Loop, HLNode *Node) {
  assert(Node && "Node is null!");
  insertAsPreheaderPostexitImpl(Loop, nullptr, Node->getIterator(),
                                Node->getIterator(), true, true);
}

void HLNodeUtils::insertAsLastPreheaderNode(HLLoop *Loop, HLNode *Node) {
  assert(Node && "Node is null!");
  insertAsPreheaderPostexitImpl(Loop, nullptr, Node->getIterator(),
                                Node->getIterator(), true, false);
}

void HLNodeUtils::insertAsFirstPostexitNode(HLLoop *Loop, HLNode *Node) {
  assert(Node && "Node is null!");
  insertAsPreheaderPostexitImpl(Loop, nullptr, Node->getIterator(),
                                Node->getIterator(), false, true);
}

void HLNodeUtils::insertAsLastPostexitNode(HLLoop *Loop, HLNode *Node) {
  assert(Node && "Node is null!");
  insertAsPreheaderPostexitImpl(Loop, nullptr, Node->getIterator(),
                                Node->getIterator(), false, false);
}

bool HLNodeUtils::foundLoopInRange(HLContainerTy::iterator First,
                                   HLContainerTy::iterator Last) {
  HLNodeUtils::LoopFinderUpdater LoopFinder(true);

  visitRange(LoopFinder, First, Last);

  return LoopFinder.foundLoop();
}

void HLNodeUtils::removeInternal(HLContainerTy &Container,
                                 HLContainerTy::iterator First,
                                 HLContainerTy::iterator Last, bool Erase) {
  HLNode *Node;

  for (auto I = First, Next = I, E = Last; I != E; I = Next) {

    Next++;
    Node = &*I;

    Container.remove(*Node);

    if (Erase) {
      Node->getHLNodeUtils().destroy(Node);
    } else {
      /// Used to catch errors where user tries to insert an already linked
      /// node.
      Node->setParent(nullptr);
    }
  }
}

void HLNodeUtils::removeImpl(HLContainerTy::iterator First,
                             HLContainerTy::iterator Last,
                             HLContainerTy *MoveContainer, bool Erase) {

  // Empty range should be handled before we decide to dereference 'First' as it
  // might be invalid.
  if (First == Last) {
    return;
  }

  // Even if the region becomes empty, we should not remove it. Instead, HIRCG
  // should short-circult entry/exit bblocks to remove the dead basic blocks.
  assert(!isa<HLRegion>(First) && "Regions cannot be removed!");

  HLNode *Parent;
  HLLoop *ParentLoop;
  HLContainerTy *OrigContainer;

  // When removing nodes we might have to set the innermost flag if the
  // ParentLoop becomes innermost loop. The precise condition where the flag
  // needs updating is when there is at least one loop in [First, last) and no
  // loop outside the range and inside ParentLoop.
  bool UpdateInnermostFlag;

  Parent = First->getParent();
  assert(Parent && "Parent is missing!");

  ParentLoop = First->getParentLoop();
  UpdateInnermostFlag = (ParentLoop && foundLoopInRange(First, Last));

  // The following if-else updates the separators. The only case where the
  // separator needs to be updated is if it points to 'First'. For more info on
  // updating separators, refer to insertImpl() comments.
  if (auto Reg = dyn_cast<HLRegion>(Parent)) {
    OrigContainer = &Reg->Children;
  } else if (auto Loop = dyn_cast<HLLoop>(Parent)) {
    OrigContainer = &Loop->Children;

    if (First == Loop->ChildBegin) {
      Loop->ChildBegin = Last;
    }
    if (First == Loop->PostexitBegin) {
      Loop->PostexitBegin = Last;
    }
  } else if (auto If = dyn_cast<HLIf>(Parent)) {
    OrigContainer = &If->Children;

    if (First == If->ElseBegin) {
      If->ElseBegin = Last;
    }
  } else if (auto Switch = dyn_cast<HLSwitch>(Parent)) {
    OrigContainer = &Switch->Children;

    for (unsigned I = 0, E = Switch->getNumCases(); I < E; ++I) {
      if (First == Switch->CaseBegin[I]) {
        Switch->CaseBegin[I] = Last;
      }
    }
    if (First == Switch->DefaultCaseBegin) {
      Switch->DefaultCaseBegin = Last;
    }

  } else {
    llvm_unreachable("Unknown parent type!");
  }

  if (Erase) {
    removeInternal(*OrigContainer, First, Last, true);
  } else if (!MoveContainer) {
    removeInternal(*OrigContainer, First, Last, false);
  } else {
    MoveContainer->splice(MoveContainer->end(), *OrigContainer, First, Last);
  }

  if (UpdateInnermostFlag &&
      !foundLoopInRange(ParentLoop->child_begin(), ParentLoop->child_end())) {
    ParentLoop->setInnermost(true);
  }
}

void HLNodeUtils::remove(HLNode *Node) {
  assert(Node && "Node is null!");

  HLContainerTy::iterator It(Node);
  removeImpl(It, std::next(It), nullptr);
}

void HLNodeUtils::remove(HLContainerTy::iterator First,
                         HLContainerTy::iterator Last) {
  removeImpl(First, Last, nullptr);
}

void HLNodeUtils::remove(HLNode *First, HLNode *Last) {
  assert(First && Last && " Node1 or Node 2 cannot be null.");
  assert(!isa<HLRegion>(First) && !isa<HLRegion>(Last) &&
         " Node1 or Node2 cannot be a HLRegion.");
  assert((First->getParent() == Last->getParent()) &&
         " Parent of Node1 and Node2 don't match.");

  HLContainerTy::iterator ItStart(First);
  HLContainerTy::iterator ItEnd(Last);

  removeImpl(ItStart, std::next(ItEnd), nullptr);
}

void HLNodeUtils::remove(HLContainerTy *Container, HLNode *First,
                         HLNode *Last) {
  assert(First && Last && " Node1 or Node 2 cannot be null.");
  assert(Container && " Clone Container is null.");
  assert(!isa<HLRegion>(First) && !isa<HLRegion>(Last) &&
         " Node1 or Node2 cannot be a HLRegion.");
  assert((First->getParent() == Last->getParent()) &&
         " Parent of Node1 and Node2 don't match.");

  HLContainerTy::iterator ItStart(First);
  HLContainerTy::iterator ItEnd(Last);
  removeImpl(ItStart, std::next(ItEnd), Container);
}

void HLNodeUtils::remove(HLContainerTy *Container,
                         HLContainerTy::iterator First,
                         HLContainerTy::iterator Last) {
  assert(Container && "Container is null.");
  removeImpl(First, Last, Container);
}

void HLNodeUtils::erase(HLContainerTy::iterator First,
                        HLContainerTy::iterator Last) {
  removeImpl(First, Last, nullptr, true);
}

void HLNodeUtils::erase(HLNode *Node) {
  assert(Node && "Node is null!");

  HLContainerTy::iterator It(Node);
  erase(It, std::next(It));
}

void HLNodeUtils::moveBefore(HLNode *Pos, HLContainerTy::iterator First,
                             HLContainerTy::iterator Last) {
  assert(Pos && "Pos is null!");

  HLContainerTy TempContainer;

  removeImpl(First, Last, &TempContainer);
  insertImpl(Pos->getParent(), Pos->getIterator(), &TempContainer,
             TempContainer.begin(), TempContainer.end(), true, true);
}

void HLNodeUtils::moveAfter(HLNode *Pos, HLContainerTy::iterator First,
                            HLContainerTy::iterator Last) {
  assert(Pos && "Pos is null!");

  HLContainerTy TempContainer;
  HLContainerTy::iterator It(Pos);

  removeImpl(First, Last, &TempContainer);
  insertImpl(Pos->getParent(), std::next(It), &TempContainer,
             TempContainer.begin(), TempContainer.end(), false);
}

void HLNodeUtils::moveBefore(HLNode *Pos, HLNode *Node) {
  assert(Node && "Node is null!");
  HLContainerTy::iterator It(Node);

  moveBefore(Pos, It, std::next(It));
}

void HLNodeUtils::moveAfter(HLNode *Pos, HLNode *Node) {
  assert(Node && "Node is null!");
  HLContainerTy::iterator It(Node);

  moveAfter(Pos, It, std::next(It));
}

void HLNodeUtils::moveAsFirstChild(HLRegion *Reg, HLNode *Node) {
  remove(Node);
  insertAsFirstChild(Reg, Node);
}

void HLNodeUtils::moveAsLastChild(HLRegion *Reg, HLNode *Node) {
  remove(Node);
  insertAsLastChild(Reg, Node);
}

void HLNodeUtils::moveAsFirstChild(HLLoop *Loop, HLNode *Node) {
  remove(Node);
  insertAsFirstChild(Loop, Node);
}

void HLNodeUtils::moveAsLastChild(HLLoop *Loop, HLNode *Node) {
  remove(Node);
  insertAsLastChild(Loop, Node);
}

void HLNodeUtils::moveAsFirstChild(HLIf *If, HLNode *Node, bool IsThenChild) {
  remove(Node);
  insertAsFirstChild(If, Node, IsThenChild);
}

void HLNodeUtils::moveAsLastChild(HLIf *If, HLNode *Node, bool IsThenChild) {
  remove(Node);
  insertAsLastChild(If, Node, IsThenChild);
}

void HLNodeUtils::moveAsFirstDefaultChild(HLSwitch *Switch, HLNode *Node) {
  remove(Node);
  insertAsFirstDefaultChild(Switch, Node);
}

void HLNodeUtils::moveAsLastDefaultChild(HLSwitch *Switch, HLNode *Node) {
  remove(Node);
  insertAsLastDefaultChild(Switch, Node);
}

void HLNodeUtils::moveAsFirstChild(HLSwitch *Switch, HLNode *Node,
                                   unsigned CaseNum) {
  remove(Node);
  insertAsFirstChild(Switch, Node, CaseNum);
}

void HLNodeUtils::moveAsLastChild(HLSwitch *Switch, HLNode *Node,
                                  unsigned CaseNum) {
  remove(Node);
  insertAsLastChild(Switch, Node, CaseNum);
}

void HLNodeUtils::moveAsFirstPreheaderNode(HLLoop *Loop, HLNode *Node) {
  remove(Node);
  insertAsFirstPreheaderNode(Loop, Node);
}

void HLNodeUtils::moveAsLastPreheaderNode(HLLoop *Loop, HLNode *Node) {
  remove(Node);
  insertAsLastPreheaderNode(Loop, Node);
}

void HLNodeUtils::moveAsFirstPostexitNode(HLLoop *Loop, HLNode *Node) {
  remove(Node);
  insertAsFirstPostexitNode(Loop, Node);
}

void HLNodeUtils::moveAsLastPostexitNode(HLLoop *Loop, HLNode *Node) {
  remove(Node);
  insertAsLastPostexitNode(Loop, Node);
}

void HLNodeUtils::moveAsFirstChildren(HLRegion *Reg,
                                      HLContainerTy::iterator First,
                                      HLContainerTy::iterator Last) {
  HLContainerTy TempContainer;

  removeImpl(First, Last, &TempContainer);
  insertAsChildImpl(Reg, &TempContainer, TempContainer.begin(),
                    TempContainer.end(), true);
}

void HLNodeUtils::moveAsLastChildren(HLRegion *Reg,
                                     HLContainerTy::iterator First,
                                     HLContainerTy::iterator Last) {
  HLContainerTy TempContainer;

  removeImpl(First, Last, &TempContainer);
  insertAsChildImpl(Reg, &TempContainer, TempContainer.begin(),
                    TempContainer.end(), false);
}

void HLNodeUtils::moveAsFirstChildren(HLLoop *Loop,
                                      HLContainerTy::iterator First,
                                      HLContainerTy::iterator Last) {
  HLContainerTy TempContainer;

  removeImpl(First, Last, &TempContainer);
  insertAsChildImpl(Loop, &TempContainer, TempContainer.begin(),
                    TempContainer.end(), true);
}

void HLNodeUtils::moveAsLastChildren(HLLoop *Loop,
                                     HLContainerTy::iterator First,
                                     HLContainerTy::iterator Last) {
  HLContainerTy TempContainer;

  removeImpl(First, Last, &TempContainer);
  insertAsChildImpl(Loop, &TempContainer, TempContainer.begin(),
                    TempContainer.end(), false);
}

void HLNodeUtils::moveAsFirstChildren(HLIf *If, HLContainerTy::iterator First,
                                      HLContainerTy::iterator Last,
                                      bool IsThenChild) {
  HLContainerTy TempContainer;

  removeImpl(First, Last, &TempContainer);
  insertImpl(If, IsThenChild ? If->then_begin() : If->else_begin(),
             &TempContainer, TempContainer.begin(), TempContainer.end(),
             !IsThenChild);
}

void HLNodeUtils::moveAsLastChildren(HLIf *If, HLContainerTy::iterator First,
                                     HLContainerTy::iterator Last,
                                     bool IsThenChild) {
  HLContainerTy TempContainer;

  removeImpl(First, Last, &TempContainer);
  insertImpl(If, IsThenChild ? If->then_end() : If->else_end(), &TempContainer,
             TempContainer.begin(), TempContainer.end(), !IsThenChild);
}

void HLNodeUtils::moveAsChildrenImpl(HLSwitch *Switch,
                                     HLContainerTy::iterator First,
                                     HLContainerTy::iterator Last,
                                     unsigned CaseNum, bool isFirstChild) {
  HLContainerTy TempContainer;

  removeImpl(First, Last, &TempContainer);
  insertAsChildImpl(Switch, &TempContainer, TempContainer.begin(),
                    TempContainer.end(), CaseNum, isFirstChild);
}

void HLNodeUtils::moveAsFirstDefaultChildren(HLSwitch *Switch,
                                             HLContainerTy::iterator First,
                                             HLContainerTy::iterator Last) {
  moveAsChildrenImpl(Switch, First, Last, 0, true);
}

void HLNodeUtils::moveAsLastDefaultChildren(HLSwitch *Switch,
                                            HLContainerTy::iterator First,
                                            HLContainerTy::iterator Last) {
  moveAsChildrenImpl(Switch, First, Last, 0, false);
}

void HLNodeUtils::moveAsFirstChildren(HLSwitch *Switch,
                                      HLContainerTy::iterator First,
                                      HLContainerTy::iterator Last,
                                      unsigned CaseNum) {
  assert((CaseNum > 0) && (CaseNum <= Switch->getNumCases()) &&
         "CaseNum is out of range!");
  moveAsChildrenImpl(Switch, First, Last, CaseNum, true);
}

void HLNodeUtils::moveAsLastChildren(HLSwitch *Switch,
                                     HLContainerTy::iterator First,
                                     HLContainerTy::iterator Last,
                                     unsigned CaseNum) {
  assert((CaseNum > 0) && (CaseNum <= Switch->getNumCases()) &&
         "CaseNum is out of range!");
  moveAsChildrenImpl(Switch, First, Last, CaseNum, false);
}

void HLNodeUtils::moveAsFirstPreheaderNodes(HLLoop *Loop,
                                            HLContainerTy::iterator First,
                                            HLContainerTy::iterator Last) {
  HLContainerTy TempContainer;

  removeImpl(First, Last, &TempContainer);
  insertAsPreheaderPostexitImpl(Loop, &TempContainer, TempContainer.begin(),
                                TempContainer.end(), true, true);
}

void HLNodeUtils::moveAsLastPreheaderNodes(HLLoop *Loop,
                                           HLContainerTy::iterator First,
                                           HLContainerTy::iterator Last) {
  HLContainerTy TempContainer;

  removeImpl(First, Last, &TempContainer);
  insertAsPreheaderPostexitImpl(Loop, &TempContainer, TempContainer.begin(),
                                TempContainer.end(), true, false);
}

void HLNodeUtils::moveAsFirstPostexitNodes(HLLoop *Loop,
                                           HLContainerTy::iterator First,
                                           HLContainerTy::iterator Last) {
  HLContainerTy TempContainer;

  removeImpl(First, Last, &TempContainer);
  insertAsPreheaderPostexitImpl(Loop, &TempContainer, TempContainer.begin(),
                                TempContainer.end(), false, true);
}

void HLNodeUtils::moveAsLastPostexitNodes(HLLoop *Loop,
                                          HLContainerTy::iterator First,
                                          HLContainerTy::iterator Last) {
  HLContainerTy TempContainer;

  removeImpl(First, Last, &TempContainer);
  insertAsPreheaderPostexitImpl(Loop, &TempContainer, TempContainer.begin(),
                                TempContainer.end(), false, false);
}

void HLNodeUtils::replace(HLNode *OldNode, HLNode *NewNode) {
  insertBefore(OldNode, NewNode);
  remove(OldNode);
}

HLNode *HLNodeUtils::getLexicalControlFlowSuccessor(HLNode *Node) {
  assert(Node && "Node is null!");
  assert(!isa<HLRegion>(Node) && "Regions don't have control flow successors!");

  HLNode *Succ = nullptr, *TempSucc = nullptr, *Parent = Node->getParent();
  HLContainerTy::iterator Iter(Node);

  // Keep moving up the parent chain till we find a successor.
  while (Parent) {
    if (auto Reg = dyn_cast<HLRegion>(Parent)) {
      if (std::next(Iter) != Reg->Children.end()) {
        Succ = &*(std::next(Iter));
        break;
      }

    } else if (auto If = dyn_cast<HLIf>(Parent)) {
      if (std::next(Iter) != If->Children.end()) {
        TempSucc = &*(std::next(Iter));

        /// Check whether we are crossing separators.
        if (If->ElseBegin == If->Children.end() ||
            (TempSucc != &*(If->ElseBegin))) {
          Succ = TempSucc;
          break;
        }
      }

    } else if (auto Switch = dyn_cast<HLSwitch>(Parent)) {

      if (std::next(Iter) != Switch->Children.end()) {
        TempSucc = &*(std::next(Iter));

        bool IsSeparator = false;
        /// Check whether we are crossing separators.
        for (unsigned I = 0, E = Switch->getNumCases(); I < E; ++I) {
          if ((Switch->CaseBegin[I] != Switch->Children.end()) &&
              (TempSucc == &*(Switch->CaseBegin[I]))) {
            IsSeparator = true;
            break;
          }
        }
        if ((Switch->DefaultCaseBegin != Switch->Children.end()) &&
            (TempSucc == &*(Switch->DefaultCaseBegin))) {
          IsSeparator = true;
        }

        if (!IsSeparator) {
          Succ = TempSucc;
          break;
        }
      }

    } else {
      llvm_unreachable("Unexpected node parent type!");
    }

    Iter = Parent->getIterator();
    Parent = Parent->getParent();
  }

  return Succ;
}

HLNode *HLNodeUtils::getLinkListNodeImpl(HLNode *Node, bool Prev) {
  assert(Node && "Node is null!");

  auto Parent = Node->getParent();

  if (!Parent) {
    assert(isa<HLRegion>(Node) && "getPrev() called on detached node!");
    auto FirstOrLastRegIter = Prev ? getHIRFramework().hir_begin()
                                   : std::prev(getHIRFramework().hir_end());
    auto NodeIter = Node->getIterator();

    if (NodeIter != FirstOrLastRegIter) {
      return Prev ? &*(std::prev(NodeIter)) : &*(std::next(NodeIter));
    }
  } else {
    auto FirstOrLastNode =
        Prev ? getFirstLexicalChild(Parent) : getLastLexicalChild(Parent);

    if (Node != FirstOrLastNode) {
      auto NodeIter = Node->getIterator();
      return Prev ? &*(std::prev(NodeIter)) : &*(std::next(NodeIter));
    }
  }

  return nullptr;
}

HLNode *HLNodeUtils::getPrevLinkListNode(HLNode *Node) {
  return getLinkListNodeImpl(Node, true);
}

HLNode *HLNodeUtils::getNextLinkListNode(HLNode *Node) {
  return getLinkListNodeImpl(Node, false);
}

// TopSortNum is a lexical number of an HLNode. The framework is responsible for
// maintaining lexical numbering after node insertion.
//
// Example: 200,300,400,500 are the topsort numbers.
//          (500) is a LexicalLastTopSortNum of all children
//
//          BEGIN REGION { }
// <23:200(500)>    + DO i1 = 0, 1, 1   <DO_LOOP>
// <24:300(500)>    |   + DO i2 = 0, 4, 1   <DO_LOOP>
// <09:400(400)>    |   |   %0 = (@A)[0][i1 + 2 * i2 + -1];
// <11:500(500)>    |   |   (@A)[0][i1 + 2 * i2] = %0;
// <24:300(500)>    |   + END LOOP
// <23:200(500)>    + END LOOP
//          END REGION
//
// -- When HIR tree is just created, TopSortNums are all zero. In this state
//    insertion does not involve numbers recalculation.
// -- After HIR is completely parsed the initTopSortNum() is called. The call
//    will set the sort numbers for all nodes with a fixed step (100).
// -- After that point an insertion of nodes will invoke TopSortNums
//    recalculation.
//
// After insertion:
// 1) Check if First->Parent()'s TopSortNumber is not zero. If true, we assume
//    that the TopSortNumbers are in order, except [First, Last] subtrees.
// 2) Get last lexical number of the previous node.
// 3) Get sort number of lexical next node.
// 4) Evenly distribute (PrevNum, NextNum) range of numbers between
//    inserted subtrees
// 5) If it's not possible - number [First, Last] nodes with a fixed step 1 and
//    move rest [Last+1, end()) forward until we get all nodes ordered.
void HLNodeUtils::updateTopSortNum(const HLContainerTy &Container,
                                   HLContainerTy::iterator First,
                                   HLContainerTy::iterator Last) {
  if (isa<HLRegion>(First)) {
    return;
  }

  HLNode *Parent = First->getParent();
  assert(Parent && "Encountered detached HLNode");

  // Return early if TopSortNums are not set yet
  if (Parent->getTopSortNum() == 0) {
    return;
  }

  unsigned PrevNum = 0;
  auto ParentLoop = dyn_cast<HLLoop>(Parent);

  // TODO: Simplify by creating getLexicalPrevNode()/getLexicalNextNode()
  // functions.
  if (Container.begin() != First) {
    // If we inserted nodes after the loop preheader, the previous num is loop's
    // top sort num.
    if (ParentLoop && (First == ParentLoop->pre_end())) {
      PrevNum = ParentLoop->getTopSortNum();
      // If we inserted at the beginning of the first non-default switch case,
      // switch is the lexically previous node.
    } else {
      PrevNum = getPrevLinkListNode(&*First)->getMaxTopSortNum();
    }
  } else {
    // If we inserted at the beginning of the preheader, the lexically previous
    // node is loop's lexically previous node.
    if (ParentLoop && (First != ParentLoop->child_begin())) {

      auto OuterParent = ParentLoop->getParent();
      auto OuterParentLoop = dyn_cast<HLLoop>(OuterParent);

      // If ParentLoop is the first child of OuterParentLoop, previous node is
      // the outer parent.
      HLNode *PrevNode =
          (OuterParentLoop && (OuterParentLoop->getFirstChild() == ParentLoop))
              ? nullptr
              : getPrevLinkListNode(Parent);

      PrevNum = PrevNode ? PrevNode->getMaxTopSortNum()
                         : OuterParent->getTopSortNum();
    } else {
      PrevNum = Parent->getTopSortNum();
    }
  }

  unsigned NextNum = 0;

  // If the last node inserted is the last preheader node, the next num should
  // be loop's top sort num.
  if (ParentLoop && (Last == ParentLoop->pre_end())) {
    NextNum = Parent->getTopSortNum();
  } else if (Container.end() != Last) {
    NextNum = Last->getMinTopSortNum();
  }

  if (!NextNum) {
    // !isa<HLRegion>(Parent) - HLRegions are linked in the ilist, but we
    // should not iterate across regions. If we traced to an HLRegion,
    // this means that there is no next node in this region.
    for (; Parent && !isa<HLRegion>(Parent); Parent = Parent->getParent()) {
      if (auto NextNode = getNextLinkListNode(Parent)) {
        NextNum = NextNode->getMinTopSortNum();
        break;
      }
    }
  }

  HLNodeUtils::distributeTopSortNum(First, Last, PrevNum, NextNum);
}

struct NodeCounter final : public HLNodeVisitorBase {
  unsigned Count;
  NodeCounter() : Count(0) {}

  void visit(HLNode *Node) { Count++; }
  void postVisit(HLNode *) {}
};

// The visitor that sets TopSortNums and MaxTopSortNum from MinNum
// with a fixed Step. If set, the numbering will be started AfterNode.
//
// Force:
//  false - stop numbering if already in order
//  true - update TopSortNum and LexicalLastTopSortNum for all nodes
template <bool Force>
struct HLNodeUtils::TopSorter final : public HLNodeVisitorBase {
  unsigned MinNum;
  unsigned Step;
  unsigned TopSortNum;
  const HLNode *AfterNode;
  bool Stop;

  // Step of 2048 is sufficient to represent about 2 million nodes in a region.
  TopSorter(unsigned MinNum, unsigned Step = 2048,
            const HLNode *AfterNode = nullptr)
      : MinNum(MinNum), TopSortNum(MinNum), AfterNode(AfterNode), Stop(false) {
    this->Step = Step ? Step : 1;
  }

  void visit(HLNode *Node) {
    if (AfterNode) {
      if (AfterNode == Node) {
        AfterNode = nullptr;
      }
      return;
    }

    assert(TopSortNum < UINT_MAX - Step && "Overflow in TopSortNum");
    TopSortNum += Step;

    if (Force || TopSortNum >= Node->getTopSortNum()) {
      Node->setTopSortNum(TopSortNum);
      if (Force || TopSortNum >= Node->getMaxTopSortNum()) {
        Node->setMaxTopSortNum(TopSortNum);
      }
    } else {
      Stop = true;
    }
  }

  void visit(HLRegion *Region) {
    TopSortNum = MinNum;
    visit(static_cast<HLNode *>(Region));
  }

  bool isDone() const override { return Stop; }

  void postVisit(HLNode *) {}
};

void HLNodeUtils::initTopSortNum() {
  TopSorter<true> TS(0);
  HLNodeUtils::visitAll(TS);
}

void HLNodeUtils::distributeTopSortNum(HLContainerTy::iterator First,
                                       HLContainerTy::iterator Last,
                                       unsigned MinNum, unsigned MaxNum) {
  // Zero MaxNum means that there is no upper limit. And we can number
  // [First, Last) nodes with a fixed step
  if (MaxNum) {
    NodeCounter NC;
    HLNodeUtils::visitRange(NC, First, Last);

    assert(MinNum < MaxNum && "MinNum should be always less than MaxNum");

    unsigned Step = (MaxNum - MinNum) / (NC.Count + 1);
    // number [First, Last) nodes
    TopSorter<true> TS(MinNum, Step);
    HLNodeUtils::visitRange(TS, First, Last);
    if (Step == 0) {
      TopSorter<false> TS(MinNum + NC.Count, 1, &*(std::prev(Last)));
      HLNodeUtils::visit(TS, First->getParentRegion());
    }
  } else {
    TopSorter<true> TS(MinNum);
    HLNodeUtils::visitRange(TS, First, Last);
  }
}

template <bool IsMaxMode>
bool HLNodeUtils::isInTopSortNumRangeImpl(const HLNode *Node,
                                          const HLNode *FirstNode,
                                          const HLNode *LastNode) {
  assert(Node && "Node is null!");

  if (!FirstNode) {
    return false;
  }

  assert(LastNode && "Last node is null!");

  unsigned Num = Node->getTopSortNum();
  unsigned FirstNum =
      IsMaxMode ? FirstNode->getMinTopSortNum() : FirstNode->getTopSortNum();
  unsigned LastNum =
      IsMaxMode ? LastNode->getMaxTopSortNum() : LastNode->getTopSortNum();

  return (Num >= FirstNum && Num <= LastNum);
}

bool HLNodeUtils::isInTopSortNumRange(const HLNode *Node,
                                      const HLNode *FirstNode,
                                      const HLNode *LastNode) {
  return isInTopSortNumRangeImpl<false>(Node, FirstNode, LastNode);
}

bool HLNodeUtils::isInTopSortNumMaxRange(const HLNode *Node,
                                         const HLNode *FirstNode,
                                         const HLNode *LastNode) {
  return isInTopSortNumRangeImpl<true>(Node, FirstNode, LastNode);
}

const HLNode *HLNodeUtils::getLexicalChildImpl(const HLNode *Parent,
                                               const HLNode *Node, bool First) {
  assert(Parent && "Parent is null!");

  if (auto Reg = dyn_cast<HLRegion>(Parent)) {
    return First ? Reg->getFirstChild() : Reg->getLastChild();
  } else if (auto Loop = dyn_cast<HLLoop>(Parent)) {

    if (!Node) {
      return First ? &*(Loop->Children.begin())
                   : &*(std::prev(Loop->Children.end()));
    }

    if (isInTopSortNumRange(Node, Loop->getFirstPreheaderNode(),
                            Loop->getLastPreheaderNode())) {
      return First ? Loop->getFirstPreheaderNode()
                   : Loop->getLastPreheaderNode();
    } else if (isInTopSortNumRange(Node, Loop->getFirstChild(),
                                   Loop->getLastChild())) {
      return First ? Loop->getFirstChild() : Loop->getLastChild();
    } else {
      return First ? Loop->getFirstPostexitNode() : Loop->getLastPostexitNode();
    }
  } else if (auto If = dyn_cast<HLIf>(Parent)) {

    if (!Node) {
      return First ? &*(If->Children.begin())
                   : &*(std::prev(If->Children.end()));
    }

    if (isInTopSortNumRange(Node, If->getFirstThenChild(),
                            If->getLastThenChild())) {
      return First ? If->getFirstThenChild() : If->getLastThenChild();
    } else {
      return First ? If->getFirstElseChild() : If->getLastElseChild();
    }
  } else {
    assert(isa<HLSwitch>(Parent) && "Unexpected parent type!");
    auto Switch = cast<HLSwitch>(Parent);

    if (!Node) {
      return First ? &*(Switch->Children.begin())
                   : &*(std::prev(Switch->Children.end()));
    }

    for (unsigned I = 1, E = Switch->getNumCases(); I <= E; I++) {
      if (isInTopSortNumRange(Node, Switch->getFirstCaseChild(I),
                              Switch->getLastCaseChild(I))) {
        return First ? Switch->getFirstCaseChild(I)
                     : Switch->getLastCaseChild(I);
      }
    }

    return First ? Switch->getFirstDefaultCaseChild()
                 : Switch->getLastDefaultCaseChild();
  }

  llvm_unreachable("Unexpected condition!");
}

const HLNode *HLNodeUtils::getFirstLexicalChild(const HLNode *Parent,
                                                const HLNode *Node) {
  return getLexicalChildImpl(Parent, Node, true);
}

HLNode *HLNodeUtils::getFirstLexicalChild(HLNode *Parent, HLNode *Node) {
  return const_cast<HLNode *>(getFirstLexicalChild(
      static_cast<const HLNode *>(Parent), static_cast<const HLNode *>(Node)));
}

const HLNode *HLNodeUtils::getLastLexicalChild(const HLNode *Parent,
                                               const HLNode *Node) {
  return getLexicalChildImpl(Parent, Node, false);
}

HLNode *HLNodeUtils::getLastLexicalChild(HLNode *Parent, HLNode *Node) {
  return const_cast<HLNode *>(getLastLexicalChild(
      static_cast<const HLNode *>(Parent), static_cast<const HLNode *>(Node)));
}

// For domination we care about single entry i.e. absence of labels in the scope
// of interest.
// For post domination we care about single exit i.e. absence of jumps from
// inside to outside the scope of interest.
// TODO: handle intrinsics/calls/exception handling semantics.
struct StructuredFlowChecker final : public HLNodeVisitorBase {
  bool IsPDom;
  const HLNode *TargetNode;
  bool IsStructured;
  bool IsDone;

  StructuredFlowChecker(bool PDom, const HLNode *TNode,
                        const HLLoop *ParentLoop, HIRLoopStatistics *HLS);

  // Returns true if visitor is done.
  bool visit(const HLNode *Node);

  void visit(const HLLabel *Label);
  void visit(const HLGoto *Goto);
  void visit(const HLLoop *Lp);

  void postVisit(const HLNode *) {}

  bool isDone() const override { return (IsDone || !isStructured()); }
  bool isStructured() const { return IsStructured; }
};

StructuredFlowChecker::StructuredFlowChecker(bool PDom, const HLNode *TNode,
                                             const HLLoop *ParentLoop,
                                             HIRLoopStatistics *HLS)
    : IsPDom(PDom), TargetNode(TNode), IsStructured(true), IsDone(false) {
  // Query HIRLoopStatistics for a possible faster response.
  if (HLS && ParentLoop) {
    if (IsPDom) {
      auto &TLS = HLS->getTotalLoopStatistics(ParentLoop);

      // Should we store statistics for multi-exit children loops to only
      // require self statistics?
      if (!TLS.hasGotos()) {
        IsDone = true;
      }
    } else {
      auto &SLS = HLS->getSelfLoopStatistics(ParentLoop);

      if (!SLS.hasLabels()) {
        IsDone = true;
      }
    }
  }
}

bool StructuredFlowChecker::visit(const HLNode *Node) {
  if (Node == TargetNode) {
    IsDone = true;
  }

  return IsDone;
}

void StructuredFlowChecker::visit(const HLLabel *Label) {
  if (visit(static_cast<const HLNode *>(Label)) || IsPDom) {
    return;
  }

  if (isa<HLLabel>(Label)) {
    IsStructured = false;
  }
}

void StructuredFlowChecker::visit(const HLGoto *Goto) {
  if (visit(static_cast<const HLNode *>(Goto)) || !IsPDom) {
    return;
  }

  if (Goto->isExternal()) {
    IsStructured = false;
    return;
  }

  auto Label = Goto->getTargetLabel();

  if (Label->getTopSortNum() > TargetNode->getTopSortNum()) {
    IsStructured = false;
  }
}

void StructuredFlowChecker::visit(const HLLoop *Lp) {
  if (visit(static_cast<const HLNode *>(Lp)) || !IsPDom) {
    return;
  }

  // Be conservative in the presence of multi-exit loops.
  if (Lp->getNumExits() > 1) {
    IsStructured = false;
  }
}

bool HLNodeUtils::hasStructuredFlow(const HLNode *Parent, const HLNode *Node,
                                    const HLNode *TargetNode,
                                    bool PostDomination, bool UpwardTraversal,
                                    HIRLoopStatistics *HLS) {
  const HLNode *FirstNode = nullptr, *LastNode = nullptr;

  // For parent loops we should retrieve the absolute first/last lexical child
  // of the loop rather than returning the first/last preheader/postexit child.
  // Consider a domination query for this case-
  // + DO LOOP
  // |  goto L:
  // |  Node1
  // |  L:
  // + END DO
  //   Node2
  //
  // Node2 lies in postexit so if we only check the postexit nodes of the loop
  // while tracing Node2 to the common parent of Node1 and itself (do loop), the
  // query will return true which would be wrong.
  if (UpwardTraversal) {
    FirstNode = isa<HLLoop>(Parent) ? getFirstLexicalChild(Parent)
                                    : getFirstLexicalChild(Parent, Node);
    LastNode = Node;
  } else {
    FirstNode = Node;
    LastNode = isa<HLLoop>(Parent) ? getLastLexicalChild(Parent)
                                   : getLastLexicalChild(Parent, Node);
  }

  assert((FirstNode && LastNode) && "Could not find first/last lexical child!");

  StructuredFlowChecker SFC(PostDomination, TargetNode,
                            FirstNode->getParentLoop(), HLS);

  // Don't need to recurse into loops.
  // Do a forward traversal when going down and vice versa.
  if (!UpwardTraversal) {
    visitRange<true, false, true>(SFC, FirstNode, LastNode);
  } else {
    visitRange<true, false, false>(SFC, FirstNode, LastNode);
  }

  return SFC.isStructured();
}

const HLNode *HLNodeUtils::getOutermostSafeParent(const HLNode *Node1,
                                                  const HLNode *Node2,
                                                  bool PostDomination,
                                                  HIRLoopStatistics *HLS,
                                                  const HLNode **LastParent1) {
  const HLNode *Parent = Node1->getParent();
  const HLNode *FirstNode = nullptr, *LastNode = nullptr;
  const HLNode *TargetNode;

  *LastParent1 = Node1;

  // Try to move up the parent chain by crossing constant trip count loops.
  while (Parent) {

    auto Loop = dyn_cast<HLLoop>(Parent);

    if (!Loop) {
      break;
    }

    if (!Loop->isDo()) {
      break;
    }

    auto UpperRef = Loop->getUpperDDRef();

    if (!UpperRef->isIntConstant()) {
      break;
    }

    if (PostDomination) {
      FirstNode = getFirstLexicalChild(Loop);
      LastNode = *LastParent1;
    } else {
      FirstNode = *LastParent1;
      LastNode = getLastLexicalChild(Loop);
    }

    // Node2 is in range, no need to move up the parent chain.
    if (isInTopSortNumRange(Node2, FirstNode, LastNode)) {
      break;
    }

    TargetNode = PostDomination ? *LastParent1 : nullptr;
    // Keep checking for structured flow for the nodes we come acoss while
    // moving up the chain.
    if (!hasStructuredFlow(Parent, *LastParent1, TargetNode, PostDomination,
                           PostDomination, HLS)) {
      return nullptr;
    }

    *LastParent1 = Parent;
    Parent = Parent->getParent();
  }

  return Parent;
}

const HLNode *HLNodeUtils::getCommonDominatingParent(
    const HLNode *Parent1, const HLNode *LastParent1, const HLNode *Node2,
    bool PostDomination, HIRLoopStatistics *HLS, const HLNode **LastParent2) {
  const HLNode *CommonParent = Node2->getParent();
  *LastParent2 = Node2;

  // Trace back Node2 to Parent1.
  while (CommonParent) {

    // Keep checking for structured flow for the nodes we come acoss while
    // moving up the chain.
    if (!hasStructuredFlow(CommonParent, *LastParent2, LastParent1,
                           PostDomination, !PostDomination, HLS)) {
      return nullptr;
    }

    if (CommonParent == Parent1) {
      break;
    }

    *LastParent2 = CommonParent;
    CommonParent = CommonParent->getParent();
  }

  return CommonParent;
}

bool HLNodeUtils::dominatesImpl(const HLNode *Node1, const HLNode *Node2,
                                bool PostDomination, bool StrictDomination,
                                HIRLoopStatistics *HLS) {

  assert(Node1 && Node2 && "Node is null!");

  assert(!isa<HLRegion>(Node1) && !isa<HLRegion>(Node2) &&
         "Domination w.r.t regions is meaningless!");
  assert((Node1->getParentRegion() == Node2->getParentRegion()) &&
         "Nodes do not belong to the same region!");

  unsigned Num1, Num2;

  Num1 = Node1->getTopSortNum();
  Num2 = Node2->getTopSortNum();

  if (Node1 == Node2) {
    return !StrictDomination;
  }

  if (PostDomination) {
    if (Num1 < Num2) {
      return false;
    }
  } else if (Num1 > Num2) {
    return false;
  }

  // We need to find out the common parent of Node1 and Node2 and their last
  // parents which tell us the path taken to reach the common parent.
  // The following example demonstrates the usage-
  //
  // if () {  // common parent
  //  N1;     // last parent of N1 is itself
  // }
  // else {
  //  if () { // last parent of N2
  //    N2;
  //  }
  // }
  const HLNode *LastParent1 = nullptr;
  const HLNode *Parent1 =
      getOutermostSafeParent(Node1, Node2, PostDomination, HLS, &LastParent1);

  // Could't find an appropriate parent for Node1.
  if (!Parent1) {
    return false;
  }

  const HLNode *LastParent2 = nullptr;
  const HLNode *CommonParent = getCommonDominatingParent(
      Parent1, LastParent1, Node2, PostDomination, HLS, &LastParent2);

  const HLIf *IfParent = dyn_cast_or_null<HLIf>(CommonParent);
  const HLSwitch *SwitchParent = dyn_cast_or_null<HLSwitch>(CommonParent);

  // Couldn't find a common parent.
  if (!CommonParent) {
    return false;
  }
  // For region and loops parents we can deduce the result right away.
  else if (!IfParent && !SwitchParent) {
    return true;

  } else if (IfParent) {
    // Check whether both nodes are in the then or else case.
    bool Node1Found =
        isInTopSortNumRange(LastParent1, IfParent->getFirstThenChild(),
                            IfParent->getLastThenChild());
    bool Node2Found =
        isInTopSortNumRange(LastParent2, IfParent->getFirstThenChild(),
                            IfParent->getLastThenChild());

    // Return false if only Node1 or Node2 was found.
    if (Node1Found ^ Node2Found) {
      return false;
    }

    return true;
  } else {
    assert(SwitchParent && "Unexpected parent node type!");
    // Check whether both nodes are in the same case.
    // The logic is conservative in that it assumes there is always a break
    // between cases and does not account for jumps between cases.
    // TODO: improve it later.
    for (unsigned I = 1, E = SwitchParent->getNumCases(); I <= E; I++) {

      bool Node1Found =
          isInTopSortNumRange(LastParent1, SwitchParent->getFirstCaseChild(I),
                              SwitchParent->getLastCaseChild(I));
      bool Node2Found =
          isInTopSortNumRange(LastParent2, SwitchParent->getFirstCaseChild(I),
                              SwitchParent->getLastCaseChild(I));

      // Return false if only Node1 or Node2 was found.
      if (Node1Found ^ Node2Found) {
        return false;
      } else if (Node1Found && Node2Found) {
        return true;
      }
    }

    // Both nodes weren't found in any switch case so they must be present in
    // the default case.
    return true;
  }

  llvm_unreachable("Unexpected condition encountered!");
}

bool HLNodeUtils::dominates(const HLNode *Node1, const HLNode *Node2,
                            HIRLoopStatistics *HLS) {
  return dominatesImpl(Node1, Node2, false, false, HLS);
}

bool HLNodeUtils::strictlyDominates(const HLNode *Node1, const HLNode *Node2,
                                    HIRLoopStatistics *HLS) {
  return dominatesImpl(Node1, Node2, false, true, HLS);
}

bool HLNodeUtils::postDominates(const HLNode *Node1, const HLNode *Node2,
                                HIRLoopStatistics *HLS) {
  return dominatesImpl(Node1, Node2, true, false, HLS);
}

bool HLNodeUtils::strictlyPostDominates(const HLNode *Node1,
                                        const HLNode *Node2,
                                        HIRLoopStatistics *HLS) {
  return dominatesImpl(Node1, Node2, true, true, HLS);
}

bool HLNodeUtils::canAccessTogether(const HLNode *Node1, const HLNode *Node2,
                                    HIRLoopStatistics *HLS) {
  // The dominance checks can return true for nodes under different parent
  // loops when the references are under constant bound loops. For the example
  // below these checks might incorrectly deduce that both references can be
  // accessed together whereas it is incorrect to hoist r2 due to different
  // loop bounds. We therefore limit the checks to the same parent loop for now.
  //
  // DO i1
  // DO i2 = 0, 5
  //  A[i2]  // r1
  // END DO
  //
  // DO i2 = 0, 100
  //  A[i2+1]  // r2
  // END DO
  // END DO
  HLLoop *ParentLoop1 = Node1->getParentLoop();
  HLLoop *ParentLoop2 = Node2->getParentLoop();
  if (!ParentLoop1 || !ParentLoop2 || ParentLoop1 != ParentLoop2) {
    return false;
  }

  // Try to save some dominance checks: if both references have the same
  // parent loop but not the same getParent(), we return early. This will catch
  // cases where the Nodes are under different HLIfs or only one of them is
  // under an HLIf.
  if (Node1->getParent() != Node2->getParent()) {
    return false;
  }

  if (!(dominates(Node2, Node1, HLS) && postDominates(Node1, Node2, HLS)) &&
      !(dominates(Node1, Node2, HLS) && postDominates(Node2, Node1, HLS))) {
    return false;
  }

  return true;
}

bool HLNodeUtils::contains(const HLNode *Parent, const HLNode *Node,
                           bool IncludePrePostHdr) {
  assert(Parent && "Parent is null!");
  assert(Node && "Node is null!");

  // Skip parent loop for preheader/postexit nodes if IncludePrePostHdr is set
  // to false.
  if (!IncludePrePostHdr) {
    auto Inst = dyn_cast<HLInst>(Node);

    if (Inst && Inst->isInPreheaderOrPostexit()) {
      Node = Node->getParent()->getParent();
    }
  }

  while (Node) {
    if (Parent == Node) {
      return true;
    }
    Node = Node->getParent();
  }

  return false;
}

bool HLNodeUtils::isMinValue(VALType ValType) {
  return ((ValType == VALType::IsMin) || (ValType == VALType::IsConstant));
}

bool HLNodeUtils::isMaxValue(VALType ValType) {
  return (ValType == VALType::IsMax || ValType == VALType::IsConstant);
}

bool HLNodeUtils::isKnownPositive(VALType ValType, int64_t Val) {
  return (isMinValue(ValType) && (Val > 0));
}

bool HLNodeUtils::isKnownNonNegative(VALType ValType, int64_t Val) {
  return (isMinValue(ValType) && (Val >= 0));
}

bool HLNodeUtils::isKnownNegative(VALType ValType, int64_t Val) {
  return (isMaxValue(ValType) && (Val < 0));
}

bool HLNodeUtils::isKnownNonPositive(VALType ValType, int64_t Val) {
  return (isMaxValue(ValType) && (Val <= 0));
}

bool HLNodeUtils::isKnownPositiveOrNegative(VALType ValType, int64_t Val) {
  return isKnownPositive(ValType, Val) || isKnownNegative(ValType, Val);
}

bool HLNodeUtils::isKnownNonZero(VALType ValType, int64_t Val) {
  return isKnownPositiveOrNegative(ValType, Val);
}

// Useful to detect if A(2 * N *I) will not be A(0) based on symbolic in UB.
// CE = c*b + c0
// BoundCE = c`*b` + c0` | where BoundCE >= 0
// b` = -c`/c0`;
// MinMaxVal = c*b` + c0;
// See more comments inside the method.
HLNodeUtils::VALType HLNodeUtils::getMinMaxBlobValue(unsigned BlobIdx,
                                                     const CanonExpr *BoundCE,
                                                     int64_t &Val) {
  if (BoundCE->numBlobs() != 1 || BoundCE->hasIV()) {
    return VALType::IsUnknown;
  }

  DEBUG(dbgs() << "\n\t in getMaxMinBlobValue: input args " << BlobIdx
               << BoundCE->getSingleBlobCoeff() << " "
               << BoundCE->getSingleBlobIndex() << " "
               << BoundCE->getConstant());

  auto BoundCoeff = BoundCE->getSingleBlobCoeff();
  auto BoundBlobIdx = BoundCE->getSingleBlobIndex();

  BlobTy Blob = getBlobUtils().getBlob(BlobIdx);
  // Strip sign extend cast from Blob
  while (BlobUtils::isSignExtendBlob(Blob, &Blob))
    ;

  BlobTy BoundBlob = getBlobUtils().getBlob(BoundBlobIdx);

  if (Blob != BoundBlob) {
    return VALType::IsUnknown;
  }

  auto BoundConstant = BoundCE->getConstant();

  // Max/Min value of Blob = - Bound constant / Bound coeff
  Val = -BoundConstant / BoundCoeff;

  // In  BoundCE (e.g. in the form 3 *N +4), positive coef (3) indicates what
  // the min value of N could be and negative coef (e.g. as in 4 - 3 *N)
  // indicates what the max value of N could be.
  return (BoundCoeff > 0) ? VALType::IsMin : VALType::IsMax;
}

bool HLNodeUtils::getMinBlobValue(unsigned BlobIdx, const HLNode *ParentNode,
                                  int64_t &Val) {
  VALType ValType = getMinMaxBlobValue(BlobIdx, ParentNode, Val);

  return isMinValue(ValType);
}

bool HLNodeUtils::getMaxBlobValue(unsigned BlobIdx, const HLNode *ParentNode,
                                  int64_t &Val) {
  VALType ValType = getMinMaxBlobValue(BlobIdx, ParentNode, Val);

  return isMaxValue(ValType);
}

bool HLNodeUtils::isKnownNonZero(unsigned BlobIdx, const HLNode *ParentNode) {
  int64_t Val;
  VALType ValType = getMinMaxBlobValue(BlobIdx, ParentNode, Val);

  return isKnownNonZero(ValType, Val);
}

bool HLNodeUtils::isKnownNonPositive(unsigned BlobIdx, const HLNode *ParentNode,
                                     int64_t &MaxVal) {
  VALType ValType = getMinMaxBlobValue(BlobIdx, ParentNode, MaxVal);

  return isKnownNonPositive(ValType, MaxVal);
}

bool HLNodeUtils::isKnownNegative(unsigned BlobIdx, const HLNode *ParentNode,
                                  int64_t &MaxVal) {
  VALType ValType = getMinMaxBlobValue(BlobIdx, ParentNode, MaxVal);

  return isKnownNegative(ValType, MaxVal);
}

bool HLNodeUtils::isKnownPositive(unsigned BlobIdx, const HLNode *ParentNode,
                                  int64_t &MinVal) {
  VALType ValType = getMinMaxBlobValue(BlobIdx, ParentNode, MinVal);

  return isKnownPositive(ValType, MinVal);
}

bool HLNodeUtils::isKnownNonNegative(unsigned BlobIdx, const HLNode *ParentNode,
                                     int64_t &MinVal) {
  VALType ValType = getMinMaxBlobValue(BlobIdx, ParentNode, MinVal);

  return isKnownNonNegative(ValType, MinVal);
}

bool HLNodeUtils::isKnownPositiveOrNegative(unsigned BlobIdx,
                                            const HLNode *ParentNode,
                                            int64_t &MinMaxVal) {
  VALType ValType = getMinMaxBlobValue(BlobIdx, ParentNode, MinMaxVal);

  return isKnownPositiveOrNegative(ValType, MinMaxVal);
}

///  Check if Loop has perfect/near-perfect loop properties
HLNodeUtils::VALType
HLNodeUtils::getMinMaxBlobValueFromPred(unsigned BlobIdx, PredicateTy Pred,
                                        const RegDDRef *Lhs,
                                        const RegDDRef *Rhs, int64_t &Val) {
  if (!CmpInst::isIntPredicate(Pred) ||
      (!CmpInst::isSigned(Pred) && Pred != PredicateTy::ICMP_EQ)) {
    return VALType::IsUnknown;
  }

  if (!Lhs->isTerminalRef() || !Rhs->isTerminalRef()) {
    return VALType::IsUnknown;
  }

  VALType Ret = VALType::IsUnknown;
  int64_t EqualOffset = 0;

  switch (Pred) {
  case PredicateTy::ICMP_SLT: // <
    EqualOffset = -1;
  case PredicateTy::ICMP_SLE: // <=
    break;
  case PredicateTy::ICMP_SGT: // >
    EqualOffset = -1;
  case PredicateTy::ICMP_SGE: // >=
    std::swap(Lhs, Rhs);
    break;
  case PredicateTy::ICMP_EQ: // ==
    Ret = VALType::IsConstant;
    break;
  case PredicateTy::ICMP_NE: // !=
    return VALType::IsUnknown;
  default:
    llvm_unreachable("Unexpected predicate");
  }

  // Lhs < Rhs
  // CE = Rhs - Lhs
  std::unique_ptr<CanonExpr> ConditionCE(getCanonExprUtils().cloneAndSubtract(
      Rhs->getSingleCanonExpr(), Lhs->getSingleCanonExpr(), true));

  if (!ConditionCE.get()) {
    return VALType::IsUnknown;
  }

  ConditionCE->addConstant(EqualOffset, false);

  VALType BlobValueType = getMinMaxBlobValue(BlobIdx, ConditionCE.get(), Val);

  // Return BlobValueType if there is no information about blob or if it wasn't
  // previously recognized as IsConstant.
  if (BlobValueType == VALType::IsUnknown || Ret != VALType::IsConstant) {
    Ret = BlobValueType;
  }

  return Ret;
}

template <typename PredIter, typename GetDDRefFunc>
HLNodeUtils::VALType HLNodeUtils::getMinMaxBlobValueFromPredRange(
    unsigned BlobIdx, PredIter Begin, PredIter End, GetDDRefFunc GetDDRef,
    bool InvertPredicates, int64_t &Val) {
  VALType Ret = VALType::IsUnknown;

  for (; Begin != End; ++Begin) {
    PredicateTy Pred = *Begin;

    if (InvertPredicates) {
      Pred = CmpInst::getInversePredicate(Pred);
    }

    RegDDRef *Lhs = GetDDRef(Begin, true);
    RegDDRef *Rhs = GetDDRef(Begin, false);

    Ret = getMinMaxBlobValueFromPred(BlobIdx, Pred, Lhs, Rhs, Val);
    if (Ret != VALType::IsUnknown) {
      break;
    }
  }

  return Ret;
}

HLNodeUtils::VALType HLNodeUtils::getMinMaxBlobValue(unsigned BlobIdx,
                                                     const HLNode *ParentNode,
                                                     int64_t &Val) {

  assert(ParentNode && "No HLNode context specified!");

  VALType Ret = VALType::IsUnknown;

  for (const HLNode *Node = ParentNode, *PrevNode = nullptr; Node != nullptr;
       PrevNode = Node, Node = Node->getParent()) {

    if (const HLLoop *Loop = dyn_cast<HLLoop>(Node)) {
      if (Loop->isUnknown()) {
        continue;
      }

      if (Loop->hasZtt() &&
          (Ret = getMinMaxBlobValueFromPredRange(
               BlobIdx, Loop->ztt_pred_begin(), Loop->ztt_pred_end(),
               std::bind(&HLLoop::getZttPredicateOperandDDRef, Loop,
                         std::placeholders::_1, std::placeholders::_2),
               false, Val)) != VALType::IsUnknown) {
        return Ret;
      }

      // Try Upper bound
      if ((Ret = getMinMaxBlobValue(BlobIdx, Loop->getUpperCanonExpr(), Val)) !=
          VALType::IsUnknown) {
        return Ret;
      }

      continue;
    }

    // Can't check topmost If statement (ParentNode) as it is not impossible to
    // figure out whether blob came from Then or Else branch.
    if (!PrevNode) {
      continue;
    }

    if (const HLIf *If = dyn_cast<HLIf>(Node)) {
      // Have to invert predicates if blob is from Else branch.
      bool InvertPredicates = If->isElseChild(PrevNode);
      if (InvertPredicates && If->getNumPredicates() > 1) {
        // If Node in the else branch and number of predicates are greater then
        // one we can't definitely prove the range.
        continue;
      }

      if ((Ret = getMinMaxBlobValueFromPredRange(
               BlobIdx, If->pred_begin(), If->pred_end(),
               std::bind(&HLIf::getPredicateOperandDDRef, If,
                         std::placeholders::_1, std::placeholders::_2),
               InvertPredicates, Val)) != VALType::IsUnknown) {
        return Ret;
      }
    }
  }

  return Ret;
}

bool HLNodeUtils::getMinMaxBlobValue(unsigned BlobIdx, int64_t Coeff,
                                     const HLNode *ParentNode, bool IsMin,
                                     int64_t &BlobVal) {

  auto ValType = getMinMaxBlobValue(BlobIdx, ParentNode, BlobVal);

  if (Coeff > 0) {
    return IsMin ? isMinValue(ValType) : isMaxValue(ValType);
  } else {
    return IsMin ? isMaxValue(ValType) : isMinValue(ValType);
  }
}

bool HLNodeUtils::getMinMaxValueImpl(const CanonExpr *CE,
                                     const HLNode *ParentNode, bool IsMin,
                                     bool IsExact, int64_t &Val) {

  assert(CE && "CE is null!");

  if (!ParentNode) {
    return CE->isIntConstant(&Val);
  }

  // Cannot use verifyNestingLevel() here unless DD starts passing accurate
  // ParentNode.
  assert(CE->verifyIVs(ParentNode->getNodeLevel()) && "Invalid arguments!");

  if (CE->isNonLinear()) {
    return false;
  }

  if (IsExact && (CE->hasBlob() || CE->hasIVBlobCoeffs())) {
    return false;
  }

  int64_t MinOrMax = 0;

  for (auto Blob = CE->blob_begin(), E = CE->blob_end(); Blob != E; ++Blob) {
    unsigned Index = CE->getBlobIndex(Blob);
    int64_t Coeff = CE->getBlobCoeff(Blob);
    int64_t BlobVal;

    if (!getMinMaxBlobValue(Index, Coeff, ParentNode, IsMin, BlobVal)) {
      return false;
    }

    MinOrMax += (Coeff * BlobVal);
  }

  if (CE->hasIV()) {
    auto ParentLoop = isa<HLLoop>(ParentNode) ? cast<HLLoop>(ParentNode)
                                              : ParentNode->getParentLoop();

    for (auto Lp = ParentLoop; Lp != nullptr; Lp = Lp->getParentLoop()) {

      unsigned Level = Lp->getNodeLevel();
      unsigned Index;
      int64_t Coeff, BlobVal = 1, IVMax = 0;

      CE->getIVCoeff(Level, &Index, &Coeff);

      if (!Coeff) {
        continue;
      }

      if ((Index != InvalidBlobIndex) &&
          !getMinMaxBlobValue(Index, Coeff, ParentNode, IsMin, BlobVal)) {
        return false;
      }

      if (!BlobVal) {
        continue;
      }

      bool IsPositive =
          ((Coeff > 0) && (BlobVal > 0)) || ((Coeff < 0) && (BlobVal < 0));

      // Ignoring is equivalent to subtituting IV by zero (initial value of IV
      // in normalized loops).
      if ((IsMin && IsPositive) || (!IsMin && !IsPositive)) {

        if (!Lp->isNormalized()) {
          return false;
        } else {
          continue;
        }
      }

      auto UpperCE = Lp->getUpperCanonExpr();
      int64_t UpperVal;

      if (UpperCE->isIntConstant(&UpperVal)) {
        // Conservatively return false if upper is too big.
        if ((UpperVal < 0) || (UpperVal > UINT32_MAX)) {
          return false;
        }

        IVMax = UpperVal;

      } else if (!getMaxValue(UpperCE, Lp, IVMax)) {
        return false;
      }

      MinOrMax += (Coeff * BlobVal * IVMax);
    }
  }

  MinOrMax += CE->getConstant();

  if (CE->getDenominator() != 1) {
    if ((MinOrMax < 0) && CE->isUnsignedDiv()) {
      return false;
    }

    MinOrMax = MinOrMax / CE->getDenominator();
  }

  Val = MinOrMax;

  return true;
}

bool HLNodeUtils::getExactMinValue(const CanonExpr *CE,
                                   const HLNode *ParentNode, int64_t &Val) {
  return getMinMaxValueImpl(CE, ParentNode, true, true, Val);
}

bool HLNodeUtils::getExactMaxValue(const CanonExpr *CE,
                                   const HLNode *ParentNode, int64_t &Val) {
  return getMinMaxValueImpl(CE, ParentNode, false, true, Val);
}

bool HLNodeUtils::getMinValue(const CanonExpr *CE, const HLNode *ParentNode,
                              int64_t &Val) {
  return getMinMaxValueImpl(CE, ParentNode, true, false, Val);
}

bool HLNodeUtils::getMaxValue(const CanonExpr *CE, const HLNode *ParentNode,
                              int64_t &Val) {
  return getMinMaxValueImpl(CE, ParentNode, false, false, Val);
}

bool HLNodeUtils::isKnownPositive(const CanonExpr *CE,
                                  const HLNode *ParentNode) {
  int64_t Val;

  if (getMinValue(CE, ParentNode, Val) && (Val > 0)) {
    return true;
  }

  return false;
}

bool HLNodeUtils::isKnownNonNegative(const CanonExpr *CE,
                                     const HLNode *ParentNode) {
  int64_t Val;

  if (getMinValue(CE, ParentNode, Val) && (Val >= 0)) {
    return true;
  }

  return false;
}

bool HLNodeUtils::isKnownNegative(const CanonExpr *CE,
                                  const HLNode *ParentNode) {
  int64_t Val;

  if (getMaxValue(CE, ParentNode, Val) && (Val < 0)) {
    return true;
  }

  return false;
}

bool HLNodeUtils::isKnownNonPositive(const CanonExpr *CE,
                                     const HLNode *ParentNode) {
  int64_t Val;

  if (getMaxValue(CE, ParentNode, Val) && (Val <= 0)) {
    return true;
  }

  return false;
}

bool HLNodeUtils::isKnownPositiveOrNegative(const CanonExpr *CE,
                                            const HLNode *ParentNode) {

  if (isKnownPositive(CE, ParentNode) || isKnownNegative(CE, ParentNode)) {
    return true;
  }

  return false;
}

bool HLNodeUtils::isKnownNonZero(const CanonExpr *CE,
                                 const HLNode *ParentNode) {
  return isKnownPositiveOrNegative(CE, ParentNode);
}

bool HLNodeUtils::getPredicateResult(APInt &LHS, PredicateTy Pred, APInt &RHS) {
  switch (Pred) {
  case PredicateTy::ICMP_EQ:
    return LHS.eq(RHS);
  case PredicateTy::ICMP_NE:
    return LHS.ne(RHS);

  case PredicateTy::ICMP_SGE:
    return LHS.sge(RHS);
  case PredicateTy::ICMP_SGT:
    return LHS.sgt(RHS);
  case PredicateTy::ICMP_SLE:
    return LHS.sle(RHS);
  case PredicateTy::ICMP_SLT:
    return LHS.slt(RHS);

  case PredicateTy::ICMP_UGE:
    return LHS.uge(RHS);
  case PredicateTy::ICMP_UGT:
    return LHS.ugt(RHS);
  case PredicateTy::ICMP_ULE:
    return LHS.ule(RHS);
  case PredicateTy::ICMP_ULT:
    return LHS.ult(RHS);

  default:
    llvm_unreachable("Unsupported predicate");
  }
}

bool HLNodeUtils::isKnownPredicate(const CanonExpr *LHS, PredicateTy Pred,
                                   const CanonExpr *RHS, bool *Result) {
  if (Pred == PredicateTy::FCMP_FALSE) {
    *Result = false;
    return true;
  } else if (Pred == PredicateTy::FCMP_TRUE) {
    *Result = true;
    return true;
  }

  int64_t LHSVal, RHSVal;
  if (LHS->isIntConstant(&LHSVal) && RHS->isIntConstant(&RHSVal)) {
    bool IsSigned = CmpInst::isSigned(Pred);
    unsigned BitWidth = LHS->getDestType()->getIntegerBitWidth();

    assert(LHS->getSrcType() == LHS->getDestType() &&
           RHS->getSrcType() == RHS->getDestType() && "Cast is not expected");
    assert(LHS->getDestType() == RHS->getDestType() && "LHS/RHS type mismatch");

    APInt LHSAPInt(BitWidth, LHSVal, IsSigned);
    APInt RHSAPInt(BitWidth, RHSVal, IsSigned);

    *Result = getPredicateResult(LHSAPInt, Pred, RHSAPInt);
    return true;
  }

  bool IsTrueWhenEqual = CmpInst::isTrueWhenEqual(Pred);
  if ((IsTrueWhenEqual || CmpInst::isFalseWhenEqual(Pred)) &&
      CanonExprUtils::areEqual(LHS, RHS, false)) {
    *Result = IsTrueWhenEqual;
    return true;
  }

  // TODO: add support for IVs and Blobs

  return false;
}

bool HLNodeUtils::hasPerfectLoopProperties(const HLLoop *Lp,
                                           const HLLoop **InnerLp,
                                           bool AllowNearPerfect,
                                           bool *IsNearPerfectLoop) {
  assert(Lp && "Lp is null!");
  assert(!Lp->isInnermost() && "Innermost loop not expected!");

  unsigned NumChildren = 0;
  // For known benchmarks and compile time considerations, allow up to 6
  // children.
  unsigned MaxChildren = AllowNearPerfect ? 6 : 1;
  *InnerLp = nullptr;

  // Code below handles non-perfect loops

  for (auto NodeIt = Lp->child_begin(), E = Lp->child_end(); NodeIt != E;
       ++NodeIt) {
    if (isa<HLLoop>(NodeIt)) {
      if (*InnerLp) {
        return false;
      }
      *InnerLp = cast<HLLoop>(NodeIt);
    }

    if (++NumChildren > MaxChildren) {
      return false;
    }
  }

  if (!*InnerLp) {
    return false;
  }

  if (NumChildren > 1) {
    if (!(*InnerLp)->isInnermost()) {
      return false;
    }

    if (IsNearPerfectLoop) {
      *IsNearPerfectLoop = true;
    }
  }

  return true;
}

///  Check if Loop has perfect loop nests
///  Default to allow pre and post header is false
///  Default to allow Triangular loop is false with exceptions made for first
///  iteration.
///  Default for AllowNearPerfect is false
///  If AllowNearPerfect is true, this function will return true for
///     near-perfect loop of this form:
///     (stmts found before or after the innermost loop)
///
///     do i1
///       do i2
///         do i3
///           s1
///           do i4
///           end do
///           s2
///         end do
///       end do
///     end do
///
///     s1, s2 are siblings of the innermost loop.
///     This form may further be transformed to perfect loopnest.
///     Will not make it too general because of compile time considerations
///     In addition, if this bool is on, it will indicate if Near Perfect Lp is
///     found
///  endif
///
bool HLNodeUtils::isPerfectLoopNest(
    const HLLoop *Loop, const HLLoop **InnermostLoop, bool AllowPrePostHdr,
    bool AllowTriangularLoop, bool AllowNearPerfect, bool *IsNearPerfectLoop) {

  assert(Loop && "Loop is null!");
  assert(!Loop->isInnermost() && "Innermost loop not expected!");

  if (!Loop->isDo()) {
    return false;
  }

  if (IsNearPerfectLoop) {
    *IsNearPerfectLoop = false;
  }

  do {
    if (!hasPerfectLoopProperties(Loop, &Loop, AllowNearPerfect,
                                  IsNearPerfectLoop)) {
      return false;
    }

    if (!Loop->isDo()) {
      return false;
    }

    if (!AllowPrePostHdr && (Loop->hasPreheader() || Loop->hasPostexit())) {
      return false;
    }

    // TODO: check if IV belongs to any loop within the perfect loopnest.
    if (!AllowTriangularLoop && Loop->isTriangularLoop()) {
      return false;
    }

  } while (!Loop->isInnermost());

  if (InnermostLoop) {
    *InnermostLoop = Loop;
  }

  return true;
}

class NonUnitStrideMemRefs final : public HLNodeVisitorBase {
private:
  unsigned NumNonLinearLRefs;
  unsigned LoopLevel;

public:
  bool HasNonUnitStride;
  NonUnitStrideMemRefs(const HLLoop *Loop)
      : NumNonLinearLRefs(0), LoopLevel(Loop->getNestingLevel()),
        HasNonUnitStride(false) {}
  void visit(const HLNode *Node) {}
  void visit(const HLDDNode *Node);
  void postVisit(const HLNode *Node) {}
  void postVisit(const HLDDNode *Node) {}
  bool isDone() const override { return (NumNonLinearLRefs > 0); }
};

///  Make a quick pass here to save compile time:
///  If all memory references are in unit stride, there is no need to
///  proceed further
void NonUnitStrideMemRefs::visit(const HLDDNode *Node) {

  for (auto I = Node->ddref_begin(), E1 = Node->ddref_end(); I != E1; ++I) {
    if ((*I)->isTerminalRef()) {
      continue;
    }

    RegDDRef *RegDDRef = *I;
    const CanonExpr *FirstCE = nullptr;
    bool NonLinearLval = RegDDRef->isLval() && !RegDDRef->isTerminalRef();

    for (auto Iter = RegDDRef->canon_begin(), E2 = RegDDRef->canon_end();
         Iter != E2; ++Iter) {
      const CanonExpr *CE = *Iter;
      // Give up on Non-linear memref because it prevents Linear Trans
      if (NonLinearLval && CE->isNonLinear()) {
        NumNonLinearLRefs++;
        return;
      }
      if (!FirstCE) {
        FirstCE = CE;
      }
    }
    assert(FirstCE && "Not expecting First CE to be null");
    if (!FirstCE->hasIV()) {
      continue;
    }

    for (auto CurIVPair = FirstCE->iv_begin(), E3 = FirstCE->iv_end();
         CurIVPair != E3; ++CurIVPair) {
      if (FirstCE->getIVConstCoeff(CurIVPair) &&
          FirstCE->getLevel(CurIVPair) != LoopLevel) {
        // Note: var coeff will have non-zero coeff so checking for
        // const coeff is sufficient.
        // Continue to visit until hitting non-linear lval memref
        HasNonUnitStride = true;
      }
    }
  }
}

///   Any memref with non-unit stride?
///   Will take innermost loop for now
///   used mostly for blocking / interchange

bool HLNodeUtils::hasNonUnitStrideRefs(const HLLoop *Loop) {

  assert(Loop && "InnermostLoop must not be nullptr");
  assert(Loop->isInnermost() && "Loop must be innermost Loop");

  NonUnitStrideMemRefs NUS(Loop);
  HLNodeUtils::visit(NUS, Loop);
  return NUS.HasNonUnitStride;
}

/// t0 = a[i1];     LRef =
///  ...
/// t1  = t0        Node
/// Looking for Node (assuming  forward sub is not done)
HLInst *
HLNodeUtils::findForwardSubInst(const DDRef *LRef,
                                SmallVectorImpl<HLInst *> &ForwardSubInsts) {

  for (auto &Inst : ForwardSubInsts) {
    const RegDDRef *RRef = Inst->getRvalDDRef();
    if (RRef->getSymbase() == LRef->getSymbase()) {
      return Inst;
    }
  }
  return nullptr;
}

const HLLoop *HLNodeUtils::getLowestCommonAncestorLoop(const HLLoop *Lp1,
                                                       const HLLoop *Lp2) {
  assert(Lp1 && "Lp1 is null!");
  assert(Lp2 && "Lp2 is null!");
  assert((Lp1->getParentRegion() == Lp2->getParentRegion()) &&
         "Lp1 and Lp2 are not in the same region!");

  // Trivial case.
  if (Lp1 == Lp2) {
    return Lp1;
  }

  // Assuming that most loops in a region belong to the same loopnest and hence
  // will have a common parent, we follow this logic-
  // 1) Move up the chain for the loop with a greater nesting level.
  // 2) Once the levels are equal, we move up the chain for both loops
  // simultaneously until we discover the common parent.

  while (Lp1 && (Lp1->getNestingLevel() > Lp2->getNestingLevel())) {
    Lp1 = Lp1->getParentLoop();
  }

  while (Lp2 && (Lp2->getNestingLevel() > Lp1->getNestingLevel())) {
    Lp2 = Lp2->getParentLoop();
  }

  // Both loops have the same nesting level, so move up simultaneously.
  while (Lp1 && Lp2) {
    if (Lp1 == Lp2) {
      return Lp1;
    }

    Lp1 = Lp1->getParentLoop();
    Lp2 = Lp2->getParentLoop();
  }

  return nullptr;
}

HLLoop *HLNodeUtils::getLowestCommonAncestorLoop(HLLoop *Lp1, HLLoop *Lp2) {
  return const_cast<HLLoop *>(getLowestCommonAncestorLoop(
      static_cast<const HLLoop *>(Lp1), static_cast<const HLLoop *>(Lp2)));
}

bool HLNodeUtils::areEqual(const HLIf *NodeA, const HLIf *NodeB) {
  if (NodeA->getNumPredicates() != NodeB->getNumPredicates()) {
    return false;
  }

  for (auto IA = NodeA->pred_begin(), EA = NodeA->pred_end(),
            IB = NodeB->pred_begin();
       IA != EA; ++IA, ++IB) {

    if (*IA != *IB) {
      return false;
    }

    if (!DDRefUtils::areEqual(NodeA->getPredicateOperandDDRef(IA, true),
                              NodeB->getPredicateOperandDDRef(IB, true)) ||
        !DDRefUtils::areEqual(NodeA->getPredicateOperandDDRef(IA, false),
                              NodeB->getPredicateOperandDDRef(IB, false))) {
      return false;
    }
  }

  return true;
}

void HLNodeUtils::replaceNodeWithBody(HLIf *If, bool ThenBody) {
  HLNodeUtils &HNU = If->getHLNodeUtils();

  if (ThenBody) {
    HNU.moveAfter(If, If->then_begin(), If->then_end());
  } else {
    HNU.moveAfter(If, If->else_begin(), If->else_end());
  }

  HLNodeUtils::remove(If);
}

namespace {

STATISTIC(InvalidatedRegions, "Number of regions invalidated by utility");
STATISTIC(InvalidatedLoops, "Number of loops invalidated by utility");
STATISTIC(LoopsRemoved, "Number of empty Loops removed by utility");
STATISTIC(IfsRemoved, "Number of empty Ifs removed by utility");
STATISTIC(RedundantLoops, "Number of redundant loops removed by utility");
STATISTIC(RedundantPredicates,
          "Number of redundant predicates removed by utility");
STATISTIC(RedundantInstructions,
          "Number of redundant instructions removed by utility");

class EmptyNodeRemoverVisitorImpl : public HLNodeVisitorBase {
protected:
  SmallPtrSet<HLNode *, 32> NodesToInvalidate;
  bool Changed = false;

  void invalidateParent(HLNode *Node) {
    if (HLLoop *ParentLoop = Node->getParentLoop()) {
      NodesToInvalidate.insert(ParentLoop);
    } else if (HLRegion *Region = Node->getParentRegion()) {
      NodesToInvalidate.insert(Region);
    }
  }

  EmptyNodeRemoverVisitorImpl() {}

public:
  void postVisit(HLRegion *Region) {
    if (NodesToInvalidate.count(Region)) {
      HIRInvalidationUtils::invalidateNonLoopRegion(Region);
      InvalidatedRegions++;
    }
  }

  void postVisit(HLLoop *Loop) {
    if (!Loop->hasChildren()) {
      invalidateParent(Loop);

      Loop->extractPreheaderAndPostexit();
      HLNodeUtils::remove(Loop);
      LoopsRemoved++;
      Changed = true;
    } else {
      if (NodesToInvalidate.count(Loop)) {
        HIRInvalidationUtils::invalidateBody(Loop);
        InvalidatedLoops++;
      }
    }
  }

  void postVisit(HLIf *If) {
    if (!If->hasThenChildren() && !If->hasElseChildren()) {
      invalidateParent(If);

      HLNodeUtils::remove(If);
      IfsRemoved++;
      Changed = true;
    }
  }

  void visit(HLNode *) {}
  void postVisit(HLNode *) {}

  void removeEmptyNode(HLNode *Node) {
    if (HLIf *If = dyn_cast<HLIf>(Node)) {
      postVisit(If);
    } else if (HLLoop *Loop = dyn_cast<HLLoop>(Node)) {
      postVisit(Loop);
    } else if (HLRegion *Region = dyn_cast<HLRegion>(Node)) {
      postVisit(Region);
    }
  }

  bool isChanged() const { return Changed; }

  void removeParentEmptyNodes(HLNode *Parent) {
    if (!Parent || isa<HLRegion>(Parent)) {
      return;
    }

    HLRegion *Region = Parent->getParentRegion();

    bool SavedChanged = Changed;

    while (Parent != Region && Changed) {
      HLNode *NextParent = Parent->getParent();

      Changed = false;
      removeEmptyNode(Parent);

      Parent = NextParent;
    }
    removeEmptyNode(Parent);

    Changed = SavedChanged;
  }
};

struct EmptyNodeRemoverVisitor final : public EmptyNodeRemoverVisitorImpl {};

class RedundantNodeRemoverVisitor final : public EmptyNodeRemoverVisitorImpl {
  const HLNode *SkipNode;

  // The class also implements DCE logic. When encounter visit(HLGoto) the
  // LastNodeToRemove is set to the last node of the current container and the
  // visitor starts to remove every node until it reaches LastNodeToRemove or
  // HLLabel. Origin HLGoto is removed if the origin target is reached within
  // the same parent or it is a next node after parent.
  // HLLabels are removed when exiting the HLRegion or HLLoop.

  // The visitor will remove each HLNode until LastNodeToRemove and will set it
  // to nullptr.
  HLNode *LastNodeToRemove;

  // The goto candidate to be removed.
  HLGoto *GotoToRemove;

  // Flag that enables HLLabel removal logic.
  bool RemoveLabels;

  // Candidate labels to be removed.
  SmallPtrSet<HLLabel *, 8> Labels;

  // Number of jumps for each label.
  SmallDenseMap<HLLabel *, unsigned, 8> LabelJumps;

public:
  RedundantNodeRemoverVisitor()
      : SkipNode(nullptr), LastNodeToRemove(nullptr), GotoToRemove(nullptr),
        RemoveLabels(false) {}

  void visit(HLRegion *Region) {
    RemoveLabels = true;

    EmptyNodeRemoverVisitorImpl::visit(Region);
  }

  void postVisit(HLRegion *Region) {
    EmptyNodeRemoverVisitorImpl::postVisit(Region);

    if (RemoveLabels) {
      for (HLLabel *Label : Labels) {
        auto Iter = LabelJumps.find(Label);
        if (Iter == LabelJumps.end() || Iter->second == 0) {
          HLNodeUtils::remove(Label);
        }
      }

      RemoveLabels = false;
    }
  }

  void visit(HLLoop *Loop) {
    uint64_t TripCount;

    bool ConstTripLoop = Loop->isConstTripLoop(&TripCount, true);
    if (ConstTripLoop && TripCount == 0) {
      RedundantLoops++;
      invalidateParent(Loop);

      SkipNode = Loop;
      HLNodeUtils::remove(Loop);
      Changed = true;
    }

    visit(static_cast<HLDDNode *>(Loop));
  }

  void visit(HLIf *If) {
    bool IsTrue;
    if (If->isKnownPredicate(&IsTrue)) {
      RedundantPredicates++;
      invalidateParent(If);

      // Go over nodes that are not going to be removed. If the predicate is
      // always TRUE the else branch will be removed.
      if (IsTrue) {
        HLNodeUtils::visitRange(*this, If->then_begin(), If->then_end());
      } else {
        HLNodeUtils::visitRange(*this, If->else_begin(), If->else_end());
      }

      SkipNode = If;
      HLNodeUtils::replaceNodeWithBody(If, IsTrue);
      Changed = true;
    }

    visit(static_cast<HLDDNode *>(If));
  }

  void removeGotoIfPointsTo(HLNode *Node) {
    if (GotoToRemove && GotoToRemove->getTargetLabel() == Node) {
      RedundantInstructions++;
      HLNodeUtils::remove(GotoToRemove);
      LabelJumps[GotoToRemove->getTargetLabel()]--;
    }
  }

  void visit(HLGoto *Goto) {
    // Could remove GOTO if it's in a dead block.
    visit(static_cast<HLNode *>(Goto));

    if (LastNodeToRemove) {
      return;
    }

    if (RemoveLabels) {
      LabelJumps[Goto->getTargetLabel()]++;
    }

    if (!Goto->isExternal()) {
      GotoToRemove = Goto;
    }

    HLNode *ContainerLastNode =
        HLNodeUtils::getLastLexicalChild(Goto->getParent(), Goto);
    if (ContainerLastNode != Goto) {
      LastNodeToRemove = ContainerLastNode;
    }
  }

  void visit(HLLabel *Label) {
    if (RemoveLabels) {
      Labels.insert(Label);
    }

    removeGotoIfPointsTo(Label);
    GotoToRemove = nullptr;

    LastNodeToRemove = nullptr;

    visit(static_cast<HLNode *>(Label));
  }

  template <typename NodeTy>
  void postVisit(NodeTy *Node) {
    HLNode *NextNode = Node->getNextNode();
    if (NextNode) {
      removeGotoIfPointsTo(NextNode);
      GotoToRemove = nullptr;
    }

    LastNodeToRemove = nullptr;

    EmptyNodeRemoverVisitorImpl::postVisit(Node);
  }

  void visit(HLNode *Node) {
    if (LastNodeToRemove) {
      RedundantInstructions++;
      HLNodeUtils::remove(Node);

      // Do not recurse into removed nodes.
      SkipNode = Node;

      if (Node == LastNodeToRemove) {
        LastNodeToRemove = nullptr;
      }
    }

    EmptyNodeRemoverVisitorImpl::visit(Node);
  }

  virtual bool skipRecursion(const HLNode *Node) const {
    return Node == SkipNode;
  }
};

}

template <typename VisitorTy>
static bool removeNodesImpl(HLNode *Node, bool RemoveEmptyParentNodes) {
  HLNode *Parent = Node->getParent();

  VisitorTy V;
  HLNodeUtils::visit(V, Node);

  if (RemoveEmptyParentNodes) {
    V.removeParentEmptyNodes(Parent);
  }

  return V.isChanged();
}

template <typename VisitorTy>
static bool removeNodesRangeImpl(HLContainerTy::iterator Begin,
                                 HLContainerTy::iterator End,
                                 bool RemoveEmptyParentNodes) {
  if (Begin == End) {
    return false;
  }

  HLNode *Parent = Begin->getParent();

  VisitorTy V;
  HLNodeUtils::visitRange(V, Begin, End);

  if (RemoveEmptyParentNodes) {
    V.removeParentEmptyNodes(Parent);
  }

  return V.isChanged();
}

bool HLNodeUtils::removeEmptyNodes(HLNode *Node, bool RemoveEmptyParentNodes) {
  return removeNodesImpl<EmptyNodeRemoverVisitor>(Node, RemoveEmptyParentNodes);
}

bool HLNodeUtils::removeEmptyNodesRange(HLContainerTy::iterator Begin,
                                        HLContainerTy::iterator End,
                                        bool RemoveEmptyParentNodes) {
  return removeNodesRangeImpl<EmptyNodeRemoverVisitor>(Begin, End,
                                                       RemoveEmptyParentNodes);
}

bool HLNodeUtils::removeRedundantNodes(HLNode *Node,
                                       bool RemoveEmptyParentNodes) {
  return removeNodesImpl<RedundantNodeRemoverVisitor>(Node,
                                                      RemoveEmptyParentNodes);
}

bool HLNodeUtils::removeRedundantNodesRange(HLContainerTy::iterator Begin,
                                            HLContainerTy::iterator End,
                                            bool RemoveEmptyParentNodes) {
  return removeNodesRangeImpl<RedundantNodeRemoverVisitor>(
      Begin, End, RemoveEmptyParentNodes);
}
