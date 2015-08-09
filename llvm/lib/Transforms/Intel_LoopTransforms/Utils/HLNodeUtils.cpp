//===------- HLNodeUtils.cpp - Implements HLNodeUtils class ---------------===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
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

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"

using namespace llvm;
using namespace loopopt;

HLNodeUtils::DummyIRBuilderTy *HLNodeUtils::DummyIRBuilder(nullptr);
Instruction *HLNodeUtils::FirstDummyInst(nullptr);

HLRegion *HLNodeUtils::createHLRegion(IRRegion *IRReg) {
  return new HLRegion(IRReg);
}

HLSwitch *HLNodeUtils::createHLSwitch(RegDDRef *ConditionRef) {
  return new HLSwitch(ConditionRef);
}

HLLabel *HLNodeUtils::createHLLabel(BasicBlock *SrcBB) {
  return new HLLabel(SrcBB);
}

HLLabel *HLNodeUtils::createHLLabel(const Twine &Name) {
  return new HLLabel(Name);
}

HLGoto *HLNodeUtils::createHLGoto(BasicBlock *TargetBB) {
  return new HLGoto(TargetBB);
}

HLGoto *HLNodeUtils::createHLGoto(HLLabel *TargetL) {
  return new HLGoto(TargetL);
}

HLInst *HLNodeUtils::createHLInst(Instruction *In) { return new HLInst(In); }

HLIf *HLNodeUtils::createHLIf(CmpInst::Predicate FirstPred, RegDDRef *Ref1,
                              RegDDRef *Ref2) {
  return new HLIf(FirstPred, Ref1, Ref2);
}

HLLoop *HLNodeUtils::createHLLoop(const Loop *LLVMLoop, bool IsDoWh) {
  return new HLLoop(LLVMLoop, IsDoWh);
}

HLLoop *HLNodeUtils::createHLLoop(HLIf *ZttIf, RegDDRef *LowerDDRef,
                                  RegDDRef *UpperDDRef, RegDDRef *StrideDDRef,
                                  bool IsDoWh, unsigned NumEx) {
  return new HLLoop(ZttIf, LowerDDRef, UpperDDRef, StrideDDRef, IsDoWh, NumEx);
}

void HLNodeUtils::destroy(HLNode *Node) { Node->destroy(); }

void HLNodeUtils::setFirstDummyInst(Instruction *Inst) {
  if (!FirstDummyInst) {
    FirstDummyInst = Inst;
  }
}

void HLNodeUtils::initialize(Function &F) {

  DummyIRBuilder = new DummyIRBuilderTy(F.getContext());
  DummyIRBuilder->SetInsertPoint(F.getEntryBlock().getTerminator());

  FirstDummyInst = nullptr;
}

void HLNodeUtils::destroyAll() {
  HLNode::destroyAll();
  delete DummyIRBuilder;
}

Value *HLNodeUtils::createZeroVal(Type *Ty) {
  return Constant::getNullValue(Ty);
}

Value *HLNodeUtils::createOneVal(Type *Ty) {
  if (Ty->isIntegerTy()) {
    return ConstantInt::get(Ty, 1);
  } else if (Ty->isFloatingPointTy()) {
    return ConstantFP::get(Ty, 1);
  }

  assert(false && "Unhandled type!");

  return nullptr;
}

void HLNodeUtils::checkUnaryInstOperands(RegDDRef *LvalRef, RegDDRef *RvalRef) {
  assert(RvalRef && "Rval is null!");

  auto ElTy = RvalRef->getElementType();

  assert((!LvalRef || (ElTy == LvalRef->getElementType())) &&
         "Operand types do not match!");
}

void HLNodeUtils::checkBinaryInstOperands(RegDDRef *LvalRef, RegDDRef *OpRef1,
                                          RegDDRef *OpRef2) {
  assert(OpRef1 && OpRef2 && "Operands are null!");

  auto ElTy = OpRef1->getElementType();

  assert((ElTy == OpRef2->getElementType()) && "Operand types do not match!");
  assert((!LvalRef || (ElTy == LvalRef->getElementType())) &&
         "Operand types do not match!");
}

HLInst *HLNodeUtils::createLvalHLInst(Instruction *Inst, RegDDRef *LvalRef) {

  setFirstDummyInst(Inst);

  auto HInst = createHLInst(Inst);

  if (!LvalRef) {
    LvalRef = DDRefUtils::createSelfBlobRef(Inst);
  }

  HInst->setLvalDDRef(LvalRef);

  return HInst;
}

HLInst *HLNodeUtils::createUnaryHLInst(unsigned OpCode, RegDDRef *LvalRef,
                                       RegDDRef *RvalRef, const Twine &Name,
                                       Type *DestTy, bool IsVolatile,
                                       unsigned Align) {
  Value *InstVal = nullptr;
  Instruction *Inst = nullptr;
  HLInst *HInst = nullptr;
  const Twine NewName(Name.isTriviallyEmpty() ? "dummy" : Name);

  checkUnaryInstOperands(LvalRef, RvalRef);

  // Create dummy val.
  auto OneVal = createOneVal(RvalRef->getElementType());

  switch (OpCode) {
  case Instruction::Load: {
    assert(!RvalRef->isScalarRef() &&
           "Rval of load instruction cannot be scalar!");

    if (LvalRef) {
      assert(LvalRef->isScalarRef() &&
             "Lval of load instruction is not a scalar!");
    }

    auto NullPtr = createZeroVal(RvalRef->getType());

    if (Align) {
      InstVal = DummyIRBuilder->CreateAlignedLoad(NullPtr, Align, IsVolatile,
                                                  NewName);
    } else {
      InstVal = DummyIRBuilder->CreateLoad(NullPtr, IsVolatile, NewName);
    }

    break;
  }

  case Instruction::Store: {
    if (LvalRef) {
      assert(!LvalRef->isScalarRef() &&
             "Lval of store instruction cannot be scalar!");
    }

    auto NullPtr = createZeroVal(RvalRef->getType());

    if (Align) {
      InstVal = DummyIRBuilder->CreateAlignedStore(OneVal, NullPtr, Align,
                                                   IsVolatile);
    } else {
      InstVal = DummyIRBuilder->CreateStore(OneVal, NullPtr, IsVolatile);
    }

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
  case Instruction::BitCast:
  case Instruction::AddrSpaceCast:

    InstVal = DummyIRBuilder->CreateCast((Instruction::CastOps)OpCode, OneVal,
                                         DestTy, NewName);

  default:
    assert(false && "Instruction not handled!");
  }

  Inst = cast<Instruction>(InstVal);

  HInst = createLvalHLInst(Inst, LvalRef);
  HInst->setRvalDDRef(RvalRef);

  return HInst;
}

HLInst *HLNodeUtils::createCopyInst(RegDDRef *RvalRef, RegDDRef *LvalRef,
                                    const Twine &Name) {
  Value *InstVal;
  Instruction *Inst;
  HLInst *HInst;
  const Twine NewName(Name.isTriviallyEmpty() ? "dummy" : Name);

  checkUnaryInstOperands(LvalRef, RvalRef);

  // Create dummy val.
  auto OneVal = createOneVal(RvalRef->getElementType());

  // Cannot use IRBuilder here as it returns the same value for casts with
  // identical src and dest types.
  InstVal = CastInst::Create(Instruction::BitCast, OneVal, OneVal->getType(),
                             NewName);
  Inst = cast<Instruction>(InstVal);
  Inst->insertBefore(DummyIRBuilder->GetInsertPoint());

  HInst = createLvalHLInst(Inst, LvalRef);
  HInst->setRvalDDRef(RvalRef);

  return HInst;
}

HLInst *HLNodeUtils::createLoad(RegDDRef *RvalRef, RegDDRef *LvalRef,
                                const Twine &Name, bool IsVolatile,
                                unsigned Align) {
  return createUnaryHLInst(Instruction::Load, LvalRef, RvalRef, Name, nullptr,
                           IsVolatile, Align);
}

HLInst *HLNodeUtils::createStore(RegDDRef *RvalRef, RegDDRef *LvalRef,
                                 const Twine &Name, bool IsVolatile,
                                 unsigned Align) {
  return createUnaryHLInst(Instruction::Store, LvalRef, RvalRef, Name, nullptr,
                           IsVolatile, Align);
}

HLInst *HLNodeUtils::createTrunc(Type *DestTy, RegDDRef *RvalRef,
                                 RegDDRef *LvalRef, const Twine &Name) {
  return createUnaryHLInst(Instruction::Trunc, LvalRef, RvalRef, Name, DestTy,
                           false, 0);
}

HLInst *HLNodeUtils::createZExt(Type *DestTy, RegDDRef *RvalRef,
                                RegDDRef *LvalRef, const Twine &Name) {
  return createUnaryHLInst(Instruction::ZExt, LvalRef, RvalRef, Name, DestTy,
                           false, 0);
}

HLInst *HLNodeUtils::createSExt(Type *DestTy, RegDDRef *RvalRef,
                                RegDDRef *LvalRef, const Twine &Name) {
  return createUnaryHLInst(Instruction::SExt, LvalRef, RvalRef, Name, DestTy,
                           false, 0);
}

HLInst *HLNodeUtils::createFPToUI(Type *DestTy, RegDDRef *RvalRef,
                                  RegDDRef *LvalRef, const Twine &Name) {
  return createUnaryHLInst(Instruction::FPToUI, LvalRef, RvalRef, Name, DestTy,
                           false, 0);
}

HLInst *HLNodeUtils::createFPToSI(Type *DestTy, RegDDRef *RvalRef,
                                  RegDDRef *LvalRef, const Twine &Name) {
  return createUnaryHLInst(Instruction::FPToSI, LvalRef, RvalRef, Name, DestTy,
                           false, 0);
}

HLInst *HLNodeUtils::createUIToFP(Type *DestTy, RegDDRef *RvalRef,
                                  RegDDRef *LvalRef, const Twine &Name) {
  return createUnaryHLInst(Instruction::UIToFP, LvalRef, RvalRef, Name, DestTy,
                           false, 0);
}

HLInst *HLNodeUtils::createSIToFP(Type *DestTy, RegDDRef *RvalRef,
                                  RegDDRef *LvalRef, const Twine &Name) {
  return createUnaryHLInst(Instruction::SIToFP, LvalRef, RvalRef, Name, DestTy,
                           false, 0);
}

HLInst *HLNodeUtils::createFPTrunc(Type *DestTy, RegDDRef *RvalRef,
                                   RegDDRef *LvalRef, const Twine &Name) {
  return createUnaryHLInst(Instruction::FPTrunc, LvalRef, RvalRef, Name, DestTy,
                           false, 0);
}

HLInst *HLNodeUtils::createFPExt(Type *DestTy, RegDDRef *RvalRef,
                                 RegDDRef *LvalRef, const Twine &Name) {
  return createUnaryHLInst(Instruction::FPExt, LvalRef, RvalRef, Name, DestTy,
                           false, 0);
}

HLInst *HLNodeUtils::createPtrToInt(Type *DestTy, RegDDRef *RvalRef,
                                    RegDDRef *LvalRef, const Twine &Name) {
  return createUnaryHLInst(Instruction::PtrToInt, LvalRef, RvalRef, Name,
                           DestTy, false, 0);
}

HLInst *HLNodeUtils::createIntToPtr(Type *DestTy, RegDDRef *RvalRef,
                                    RegDDRef *LvalRef, const Twine &Name) {
  return createUnaryHLInst(Instruction::IntToPtr, LvalRef, RvalRef, Name,
                           DestTy, false, 0);
}

HLInst *HLNodeUtils::createBitCast(Type *DestTy, RegDDRef *RvalRef,
                                   RegDDRef *LvalRef, const Twine &Name) {
  return createUnaryHLInst(Instruction::BitCast, LvalRef, RvalRef, Name, DestTy,
                           false, 0);
}

HLInst *HLNodeUtils::createAddrSpaceCast(Type *DestTy, RegDDRef *RvalRef,
                                         RegDDRef *LvalRef, const Twine &Name) {
  return createUnaryHLInst(Instruction::AddrSpaceCast, LvalRef, RvalRef, Name,
                           DestTy, false, 0);
}

HLInst *HLNodeUtils::createBinaryHLInst(unsigned OpCode, RegDDRef *OpRef1,
                                        RegDDRef *OpRef2, RegDDRef *LvalRef,
                                        const Twine &Name, bool HasNUWOrExact,
                                        bool HasNSW, MDNode *FPMathTag) {
  Value *InstVal;
  Instruction *Inst;
  HLInst *HInst;
  const Twine NewName(Name.isTriviallyEmpty() ? "dummy" : Name);

  checkBinaryInstOperands(LvalRef, OpRef1, OpRef2);

  // Create dummy val.
  auto OneVal = createOneVal(OpRef1->getElementType());

  switch (OpCode) {
  case Instruction::Add: {
    assert(OpRef1->getElementType()->isIntegerTy() &&
           "Operand is not an integer type!");
    InstVal = DummyIRBuilder->CreateAdd(OneVal, OneVal, NewName, HasNUWOrExact,
                                        HasNSW);
    break;
  }

  case Instruction::FAdd: {
    assert(OpRef1->getElementType()->isFloatingPointTy() &&
           "Operand is not a floating point type!");
    InstVal = DummyIRBuilder->CreateFAdd(OneVal, OneVal, NewName, FPMathTag);
    break;
  }

  case Instruction::Sub: {
    assert(OpRef1->getElementType()->isIntegerTy() &&
           "Operand is not an integer type!");
    InstVal = DummyIRBuilder->CreateSub(OneVal, OneVal, NewName, HasNUWOrExact,
                                        HasNSW);
    break;
  }

  case Instruction::FSub: {
    assert(OpRef1->getElementType()->isFloatingPointTy() &&
           "Operand is not a floating point type!");
    InstVal = DummyIRBuilder->CreateFSub(OneVal, OneVal, NewName, FPMathTag);
    break;
  }

  case Instruction::Mul: {
    assert(OpRef1->getElementType()->isIntegerTy() &&
           "Operand is not an integer type!");
    InstVal = DummyIRBuilder->CreateMul(OneVal, OneVal, NewName, HasNUWOrExact,
                                        HasNSW);
    break;
  }

  case Instruction::FMul: {
    assert(OpRef1->getElementType()->isFloatingPointTy() &&
           "Operand is not a floating point type!");
    InstVal = DummyIRBuilder->CreateFMul(OneVal, OneVal, NewName, FPMathTag);
    break;
  }

  case Instruction::UDiv: {
    assert(OpRef1->getElementType()->isIntegerTy() &&
           "Operand is not an integer type!");
    InstVal =
        DummyIRBuilder->CreateUDiv(OneVal, OneVal, NewName, HasNUWOrExact);
    break;
  }

  case Instruction::SDiv: {
    assert(OpRef1->getElementType()->isIntegerTy() &&
           "Operand is not an integer type!");
    InstVal =
        DummyIRBuilder->CreateSDiv(OneVal, OneVal, NewName, HasNUWOrExact);
    break;
  }

  case Instruction::FDiv: {
    assert(OpRef1->getElementType()->isFloatingPointTy() &&
           "Operand is not a floating point type!");
    InstVal = DummyIRBuilder->CreateFDiv(OneVal, OneVal, NewName, FPMathTag);
    break;
  }

  case Instruction::URem: {
    assert(OpRef1->getElementType()->isIntegerTy() &&
           "Operand is not an integer type!");
    InstVal = DummyIRBuilder->CreateURem(OneVal, OneVal, NewName);
    break;
  }

  case Instruction::SRem: {
    assert(OpRef1->getElementType()->isIntegerTy() &&
           "Operand is not an integer type!");
    InstVal = DummyIRBuilder->CreateSRem(OneVal, OneVal, NewName);
    break;
  }

  case Instruction::FRem: {
    assert(OpRef1->getElementType()->isFloatingPointTy() &&
           "Operand is not a floating point type!");
    InstVal = DummyIRBuilder->CreateFRem(OneVal, OneVal, NewName, FPMathTag);
    break;
  }

  case Instruction::Shl: {
    assert(OpRef1->getElementType()->isIntegerTy() &&
           "Operand is not an integer type!");
    InstVal = DummyIRBuilder->CreateShl(OneVal, OneVal, NewName, HasNUWOrExact,
                                        HasNSW);
    break;
  }

  case Instruction::LShr: {
    assert(OpRef1->getElementType()->isIntegerTy() &&
           "Operand is not an integer type!");
    InstVal =
        DummyIRBuilder->CreateLShr(OneVal, OneVal, NewName, HasNUWOrExact);
    break;
  }

  case Instruction::AShr: {
    assert(OpRef1->getElementType()->isIntegerTy() &&
           "Operand is not an integer type!");
    InstVal =
        DummyIRBuilder->CreateAShr(OneVal, OneVal, NewName, HasNUWOrExact);
    break;
  }

  case Instruction::And: {
    assert(OpRef1->getElementType()->isIntegerTy() &&
           "Operand is not an integer type!");
    InstVal = DummyIRBuilder->CreateAnd(OneVal, OneVal, NewName);
    break;
  }

  case Instruction::Or: {
    assert(OpRef1->getElementType()->isIntegerTy() &&
           "Operand is not an integer type!");
    InstVal = DummyIRBuilder->CreateOr(OneVal, OneVal, NewName);
    break;
  }

  case Instruction::Xor: {
    assert(OpRef1->getElementType()->isIntegerTy() &&
           "Operand is not an integer type!");
    InstVal = DummyIRBuilder->CreateXor(OneVal, OneVal, NewName);
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

HLInst *HLNodeUtils::createAdd(RegDDRef *OpRef1, RegDDRef *OpRef2,
                               RegDDRef *LvalRef, const Twine &Name,
                               bool HasNUW, bool HasNSW) {
  return createBinaryHLInst(Instruction::Add, OpRef1, OpRef2, LvalRef, Name,
                            HasNUW, HasNSW, nullptr);
}

HLInst *HLNodeUtils::createFAdd(RegDDRef *OpRef1, RegDDRef *OpRef2,
                                RegDDRef *LvalRef, const Twine &Name,
                                MDNode *FPMathTag) {
  return createBinaryHLInst(Instruction::FAdd, OpRef1, OpRef2, LvalRef, Name,
                            false, false, FPMathTag);
}

HLInst *HLNodeUtils::createSub(RegDDRef *OpRef1, RegDDRef *OpRef2,
                               RegDDRef *LvalRef, const Twine &Name,
                               bool HasNUW, bool HasNSW) {
  return createBinaryHLInst(Instruction::Sub, OpRef1, OpRef2, LvalRef, Name,
                            HasNUW, HasNSW, nullptr);
}

HLInst *HLNodeUtils::createFSub(RegDDRef *OpRef1, RegDDRef *OpRef2,
                                RegDDRef *LvalRef, const Twine &Name,
                                MDNode *FPMathTag) {
  return createBinaryHLInst(Instruction::FSub, OpRef1, OpRef2, LvalRef, Name,
                            false, false, FPMathTag);
}

HLInst *HLNodeUtils::createMul(RegDDRef *OpRef1, RegDDRef *OpRef2,
                               RegDDRef *LvalRef, const Twine &Name,
                               bool HasNUW, bool HasNSW) {
  return createBinaryHLInst(Instruction::Mul, OpRef1, OpRef2, LvalRef, Name,
                            HasNUW, HasNSW, nullptr);
}

HLInst *HLNodeUtils::createFMul(RegDDRef *OpRef1, RegDDRef *OpRef2,
                                RegDDRef *LvalRef, const Twine &Name,
                                MDNode *FPMathTag) {
  return createBinaryHLInst(Instruction::FMul, OpRef1, OpRef2, LvalRef, Name,
                            false, false, FPMathTag);
}

HLInst *HLNodeUtils::createUDiv(RegDDRef *OpRef1, RegDDRef *OpRef2,
                                RegDDRef *LvalRef, const Twine &Name,
                                bool IsExact) {
  return createBinaryHLInst(Instruction::UDiv, OpRef1, OpRef2, LvalRef, Name,
                            IsExact, false, nullptr);
}

HLInst *HLNodeUtils::createSDiv(RegDDRef *OpRef1, RegDDRef *OpRef2,
                                RegDDRef *LvalRef, const Twine &Name,
                                bool IsExact) {
  return createBinaryHLInst(Instruction::SDiv, OpRef1, OpRef2, LvalRef, Name,
                            IsExact, false, nullptr);
}

HLInst *HLNodeUtils::createFDiv(RegDDRef *OpRef1, RegDDRef *OpRef2,
                                RegDDRef *LvalRef, const Twine &Name,
                                MDNode *FPMathTag) {
  return createBinaryHLInst(Instruction::FDiv, OpRef1, OpRef2, LvalRef, Name,
                            false, false, FPMathTag);
}

HLInst *HLNodeUtils::createURem(RegDDRef *OpRef1, RegDDRef *OpRef2,
                                RegDDRef *LvalRef, const Twine &Name) {
  return createBinaryHLInst(Instruction::URem, OpRef1, OpRef2, LvalRef, Name,
                            false, false, nullptr);
}

HLInst *HLNodeUtils::createSRem(RegDDRef *OpRef1, RegDDRef *OpRef2,
                                RegDDRef *LvalRef, const Twine &Name) {
  return createBinaryHLInst(Instruction::SRem, OpRef1, OpRef2, LvalRef, Name,
                            false, false, nullptr);
}

HLInst *HLNodeUtils::createFRem(RegDDRef *OpRef1, RegDDRef *OpRef2,
                                RegDDRef *LvalRef, const Twine &Name,
                                MDNode *FPMathTag) {
  return createBinaryHLInst(Instruction::FRem, OpRef1, OpRef2, LvalRef, Name,
                            false, false, FPMathTag);
}

HLInst *HLNodeUtils::createShl(RegDDRef *OpRef1, RegDDRef *OpRef2,
                               RegDDRef *LvalRef, const Twine &Name,
                               bool HasNUW, bool HasNSW) {
  return createBinaryHLInst(Instruction::Shl, OpRef1, OpRef2, LvalRef, Name,
                            HasNUW, HasNSW, nullptr);
}

HLInst *HLNodeUtils::createLShr(RegDDRef *OpRef1, RegDDRef *OpRef2,
                                RegDDRef *LvalRef, const Twine &Name,
                                bool IsExact) {
  return createBinaryHLInst(Instruction::LShr, OpRef1, OpRef2, LvalRef, Name,
                            IsExact, false, nullptr);
}

HLInst *HLNodeUtils::createAShr(RegDDRef *OpRef1, RegDDRef *OpRef2,
                                RegDDRef *LvalRef, const Twine &Name,
                                bool IsExact) {
  return createBinaryHLInst(Instruction::AShr, OpRef1, OpRef2, LvalRef, Name,
                            IsExact, false, nullptr);
}

HLInst *HLNodeUtils::createAnd(RegDDRef *OpRef1, RegDDRef *OpRef2,
                               RegDDRef *LvalRef, const Twine &Name) {
  return createBinaryHLInst(Instruction::And, OpRef1, OpRef2, LvalRef, Name,
                            false, false, nullptr);
}

HLInst *HLNodeUtils::createOr(RegDDRef *OpRef1, RegDDRef *OpRef2,
                              RegDDRef *LvalRef, const Twine &Name) {
  return createBinaryHLInst(Instruction::Or, OpRef1, OpRef2, LvalRef, Name,
                            false, false, nullptr);
}

HLInst *HLNodeUtils::createXor(RegDDRef *OpRef1, RegDDRef *OpRef2,
                               RegDDRef *LvalRef, const Twine &Name) {
  return createBinaryHLInst(Instruction::Xor, OpRef1, OpRef2, LvalRef, Name,
                            false, false, nullptr);
}

HLInst *HLNodeUtils::createCmp(CmpInst::Predicate Pred, RegDDRef *OpRef1,
                               RegDDRef *OpRef2, RegDDRef *LvalRef,
                               const Twine &Name) {
  Value *InstVal;
  HLInst *HInst;
  const Twine NewName(Name.isTriviallyEmpty() ? "dummy" : Name);

  checkBinaryInstOperands(nullptr, OpRef1, OpRef2);

  if (LvalRef) {
    assert((LvalRef->getType()->isIntegerTy() &&
            (LvalRef->getType()->getIntegerBitWidth() == 1)) &&
           "LvalRef has invalid type!");
  }

  auto OneVal = createOneVal(OpRef1->getElementType());

  if (OpRef1->getElementType()->isIntegerTy()) {
    InstVal =
        DummyIRBuilder->CreateICmp(ICmpInst::ICMP_EQ, OneVal, OneVal, NewName);
  } else {
    InstVal = DummyIRBuilder->CreateFCmp(FCmpInst::FCMP_TRUE, OneVal, OneVal,
                                         NewName);
  }

  HInst = createLvalHLInst(cast<Instruction>(InstVal), LvalRef);
  HInst->setPredicate(Pred);

  HInst->setOperandDDRef(OpRef1, 1);
  HInst->setOperandDDRef(OpRef2, 2);

  return HInst;
}

HLInst *HLNodeUtils::createSelect(CmpInst::Predicate Pred, RegDDRef *OpRef1,
                                  RegDDRef *OpRef2, RegDDRef *OpRef3,
                                  RegDDRef *OpRef4, RegDDRef *LvalRef,
                                  const Twine &Name) {
  Value *InstVal;
  HLInst *HInst;
  const Twine NewName(Name.isTriviallyEmpty() ? "dummy" : Name);

  // LvalRef, OpRef3 and OpRef4 should be the same type.
  checkBinaryInstOperands(LvalRef, OpRef3, OpRef4);
  // OpRef1 and OpRef2 should be the same type.
  checkBinaryInstOperands(nullptr, OpRef1, OpRef2);

  auto CmpVal = createOneVal(Type::getInt1Ty(getHIRParser()->getContext()));
  auto OpVal = createOneVal(OpRef3->getElementType());

  InstVal = DummyIRBuilder->CreateSelect(CmpVal, OpVal, OpVal, Name);

  HInst = createLvalHLInst(cast<Instruction>(InstVal), LvalRef);
  HInst->setPredicate(Pred);

  HInst->setOperandDDRef(OpRef1, 1);
  HInst->setOperandDDRef(OpRef2, 2);
  HInst->setOperandDDRef(OpRef3, 3);
  HInst->setOperandDDRef(OpRef4, 4);

  return HInst;
}

struct HLNodeUtils::CloneVisitor {

  HLContainerTy *CloneContainer;
  GotoContainerTy *GotoList;
  LabelMapTy *LabelMap;

  CloneVisitor(HLContainerTy *Container, GotoContainerTy *GList,
               LabelMapTy *LMap)
      : CloneContainer(Container), GotoList(GList), LabelMap(LMap) {}

  void visit(HLNode *Node) {
    CloneContainer->push_back(Node->cloneImpl(GotoList, LabelMap));
  }

  void postVisit(HLNode *Node) {}
  bool isDone() { return false; }
  bool skipRecursion(HLNode *Node) { return false; }
  void postVisitUpdate() { updateGotos(GotoList, LabelMap); }
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
                                    const HLNode *Node1, const HLNode *Node2) {

  GotoContainerTy GotoList;
  LabelMapTy LabelMap;

  // Check for Node2 as nullptr or a single node
  if (!Node2 || (Node1 == Node2)) {
    CloneContainer->push_back(Node1->cloneImpl(&GotoList, &LabelMap));
    updateGotos(&GotoList, &LabelMap);
    return;
  }

  HLContainerTy::iterator It1(const_cast<HLNode *>(Node1));
  HLContainerTy::iterator It2(const_cast<HLNode *>(Node2));

  HLNodeUtils::CloneVisitor CloneVisit(CloneContainer, &GotoList, &LabelMap);
  visit<HLNodeUtils::CloneVisitor>(&CloneVisit, It1, std::next(It2), false,
                                   true, true);
  CloneVisit.postVisitUpdate();
}

void HLNodeUtils::cloneSequence(HLContainerTy *CloneContainer,
                                const HLNode *Node1, const HLNode *Node2) {
  assert(Node1 && !isa<HLRegion>(Node1) &&
         " Node1 - Region Cloning is not allowed.");
  assert((!Node2 || !isa<HLRegion>(Node2)) &&
         " Node 2 - Region Cloning is not allowed.");
  assert(CloneContainer && " Clone Container is null.");
  cloneSequenceImpl(CloneContainer, Node1, Node2);
}

/// \brief Helper for updating loop info during insertion/removal.
///
/// It operates under two modes: finder and updater.
/// Under finder mode, it looks for a loop. This is used by the caller to set
/// the innermost flag during node removal.
/// Under updater mode, it updates nesting level and innermost flag of
/// involved loops. This mode is used during node insertion.
///
struct HLNodeUtils::LoopFinderUpdater {

  bool FinderMode;
  bool FoundLoop;

  LoopFinderUpdater(bool IsFinder) : FinderMode(IsFinder), FoundLoop(false) {}

  bool foundLoop() { return FoundLoop; }

  void visit(HLLoop *Loop) {
    if (FinderMode) {
      FoundLoop = true;
    } else {
      HLNodeUtils::updateLoopInfo(Loop);
    }
  }

  /// Catch-all visit functions
  void visit(HLNode *Node) {}
  void postVisit(HLNode *Node) {}

  bool isDone() {
    if (FinderMode && FoundLoop) {
      return true;
    }
    return false;
  }

  bool skipRecursion(HLNode *Node) { return false; }
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
  visit<HLNodeUtils::LoopFinderUpdater>(&LoopUpdater, First, Last);
}

void HLNodeUtils::insertInternal(HLContainerTy &InsertContainer,
                                 HLContainerTy::iterator Pos,
                                 HLContainerTy *OrigContainer,
                                 HLContainerTy::iterator First,
                                 HLContainerTy::iterator Last) {

  if (!OrigContainer) {
    InsertContainer.insert(Pos, First);
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

  assert(!isa<HLRegion>(First) && "Transformations should not add/reorder "
                                  "regions!");

  assert(Parent && "Parent is missing!");
  unsigned I = 0, Distance = 1;

  if (OrigContainer) {
    Distance = std::distance(First, Last);
  }
#ifndef NDEBUG
  else {
    assert(!First->getParent() &&
           "Node is already linked, please remove it first!!");
  }
#endif

  if (auto Reg = dyn_cast<HLRegion>(Parent)) {
    insertInternal(Reg->Children, Pos, OrigContainer, First, Last);
  } else if (auto Loop = dyn_cast<HLLoop>(Parent)) {
    insertInternal(Loop->Children, Pos, OrigContainer, First, Last);

    if (UpdateSeparator) {
      if (Pos == Loop->ChildBegin) {
        Loop->ChildBegin = std::prev(Pos, Distance);
      }
      if (PostExitSeparator && (Pos == Loop->PostexitBegin)) {
        Loop->PostexitBegin = std::prev(Pos, Distance);
      }
    }

  } else if (auto If = dyn_cast<HLIf>(Parent)) {
    insertInternal(If->Children, Pos, OrigContainer, First, Last);

    if (UpdateSeparator && (Pos == If->ElseBegin)) {
      If->ElseBegin = std::prev(Pos, Distance);
    }

  } else if (auto Switch = dyn_cast<HLSwitch>(Parent)) {
    insertInternal(Switch->Children, Pos, OrigContainer, First, Last);

    // Update all the empty cases which are lexically before this one.
    if (UpdateSeparator) {
      // CaseNum is set to -1 by insertBefore(). It is a bit wasteful but
      // functionally correct to check all the cases.
      unsigned E = (CaseNum == -1) ? Switch->getNumCases() : CaseNum;
      for (unsigned I = 0; I < E; ++I) {
        if (Pos == Switch->CaseBegin[I]) {
          Switch->CaseBegin[I] = std::prev(Pos, Distance);
        }
      }
    }

  } else {
    llvm_unreachable("Unknown parent type!");
  }

  // Update parent of topmost nodes. Inner nodes' parent remains the same.
  for (auto It = Pos; I < Distance; ++I, It--) {
    std::prev(It)->setParent(Parent);
  }

  // Update loop info for loops in this range.
  updateLoopInfoRecursively(std::prev(Pos, Distance), Pos);
}

void HLNodeUtils::insertBefore(HLNode *Pos, HLNode *Node) {
  assert(Pos && "Pos is null!");
  assert(Node && "Node is null!");
  insertImpl(Pos->getParent(), Pos, nullptr, Node, Node, true, true);
}

void HLNodeUtils::insertBefore(HLNode *Pos, HLContainerTy *NodeContainer) {
  assert(Pos && "Pos is null!");
  assert(NodeContainer && "NodeContainer is null!");

  insertImpl(Pos->getParent(), Pos, NodeContainer, NodeContainer->begin(),
             NodeContainer->end(), true, true);
}

/// This function doesn't require updating separators as they point to the
/// beginning of a lexical scope and hence will remain unaffected by this
/// operation.
void HLNodeUtils::insertAfter(HLNode *Pos, HLNode *Node) {
  assert(Pos && "Pos is null!");
  assert(Node && "Node is null!");
  HLContainerTy::iterator It(Pos);

  insertImpl(Pos->getParent(), std::next(It), nullptr, Node, Node, false);
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
  insertAsChildImpl(Reg, nullptr, Node, Node, true);
}

void HLNodeUtils::insertAsLastChild(HLRegion *Reg, HLNode *Node) {
  assert(Node && "Node is null!");
  insertAsChildImpl(Reg, nullptr, Node, Node, false);
}

void HLNodeUtils::insertAsFirstChild(HLLoop *Loop, HLNode *Node) {
  assert(Node && "Node is null!");
  insertAsChildImpl(Loop, nullptr, Node, Node, true);
}

void HLNodeUtils::insertAsFirstChildren(HLLoop *Loop,
                                        HLContainerTy *NodeContainer) {
  assert(NodeContainer && "NodeContainer is null!");
  insertAsChildImpl(Loop, NodeContainer, NodeContainer->begin(),
                    NodeContainer->end(), true);
}

void HLNodeUtils::insertAsLastChild(HLLoop *Loop, HLNode *Node) {
  assert(Node && "Node is null!");
  insertAsChildImpl(Loop, nullptr, Node, Node, false);
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
             Node, Node, !IsThenChild);
}

void HLNodeUtils::insertAsLastChild(HLIf *If, HLNode *Node, bool IsThenChild) {
  assert(If && "If is null!");
  assert(Node && "Node is null!");

  insertImpl(If, IsThenChild ? If->then_end() : If->else_end(), nullptr, Node,
             Node, !IsThenChild);
}

void HLNodeUtils::insertAsChildImpl(HLSwitch *Switch,
                                    HLContainerTy *OrigContainer,
                                    HLContainerTy::iterator First,
                                    HLContainerTy::iterator Last,
                                    unsigned CaseNum, bool isFirstChild) {
  insertImpl(Switch, isFirstChild ? Switch->case_child_begin_internal(CaseNum)
                                  : Switch->case_child_end_internal(CaseNum),
             OrigContainer, First, Last, (CaseNum != 0), false, CaseNum);
}

void HLNodeUtils::insertAsFirstDefaultChild(HLSwitch *Switch, HLNode *Node) {
  assert(Node && "Node is null!");
  insertAsChildImpl(Switch, nullptr, Node, Node, 0, true);
}

void HLNodeUtils::insertAsLastDefaultChild(HLSwitch *Switch, HLNode *Node) {
  assert(Node && "Node is null!");
  insertAsChildImpl(Switch, nullptr, Node, Node, 0, false);
}

void HLNodeUtils::insertAsFirstChild(HLSwitch *Switch, HLNode *Node,
                                     unsigned CaseNum) {
  assert(Node && "Node is null!");
  assert((CaseNum > 0) && (CaseNum <= Switch->getNumCases()) &&
         "CaseNum is out of range!");
  insertAsChildImpl(Switch, nullptr, Node, Node, CaseNum, true);
}

void HLNodeUtils::insertAsLastChild(HLSwitch *Switch, HLNode *Node,
                                    unsigned CaseNum) {
  assert(Node && "Node is null!");
  assert((CaseNum > 0) && (CaseNum <= Switch->getNumCases()) &&
         "CaseNum is out of range!");
  insertAsChildImpl(Switch, nullptr, Node, Node, CaseNum, false);
}

void HLNodeUtils::insertAsPreheaderPostexitImpl(
    HLLoop *Loop, HLContainerTy *OrigContainer, HLContainerTy::iterator First,
    HLContainerTy::iterator Last, bool IsPreheader, bool IsFirstChild) {

  HLContainerTy::iterator Pos;

  Pos = IsPreheader ? (IsFirstChild ? Loop->pre_begin() : Loop->pre_end())
                    : (IsFirstChild ? Loop->post_begin() : Loop->post_end());

  insertImpl(Loop, Pos, OrigContainer, First, Last, !IsPreheader, !IsPreheader);
}

void HLNodeUtils::insertAsFirstPreheaderNode(HLLoop *Loop, HLNode *Node) {
  assert(Node && "Node is null!");
  insertAsPreheaderPostexitImpl(Loop, nullptr, Node, Node, true, true);
}

void HLNodeUtils::insertAsLastPreheaderNode(HLLoop *Loop, HLNode *Node) {
  assert(Node && "Node is null!");
  insertAsPreheaderPostexitImpl(Loop, nullptr, Node, Node, true, false);
}

void HLNodeUtils::insertAsFirstPostexitNode(HLLoop *Loop, HLNode *Node) {
  assert(Node && "Node is null!");
  insertAsPreheaderPostexitImpl(Loop, nullptr, Node, Node, false, true);
}

void HLNodeUtils::insertAsLastPostexitNode(HLLoop *Loop, HLNode *Node) {
  assert(Node && "Node is null!");
  insertAsPreheaderPostexitImpl(Loop, nullptr, Node, Node, false, false);
}

bool HLNodeUtils::foundLoopInRange(HLContainerTy::iterator First,
                                   HLContainerTy::iterator Last) {
  HLNodeUtils::LoopFinderUpdater LoopFinder(true);

  visit<HLNodeUtils::LoopFinderUpdater>(&LoopFinder, First, Last);

  return LoopFinder.foundLoop();
}

void HLNodeUtils::removeInternal(HLContainerTy &Container,
                                 HLContainerTy::iterator First,
                                 HLContainerTy::iterator Last, bool Erase) {
  HLNode *Node;

  for (auto I = First, Next = I, E = Last; I != E; I = Next) {

    Next++;
    Node = Container.remove(I);

    if (Erase) {
      destroy(Node);
    }
#ifndef NDEBUG
    else {
      /// Used to catch errors where user tries to insert an already linked
      /// node.
      Node->setParent(nullptr);
    }
#endif
  }
}

void HLNodeUtils::removeImpl(HLContainerTy::iterator First,
                             HLContainerTy::iterator Last,
                             HLContainerTy *MoveContainer, bool Erase) {

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
  // updating separators, refer to InsertImpl() comments.
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
  insertImpl(Pos->getParent(), Pos, &TempContainer, TempContainer.begin(),
             TempContainer.end(), true, true);
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
        Succ = std::next(Iter);
        break;
      }

    } else if (auto If = dyn_cast<HLIf>(Parent)) {
      if (std::next(Iter) != If->Children.end()) {
        TempSucc = std::next(Iter);

        /// Check whether we are crossing separators.
        if ((TempSucc != If->ElseBegin)) {
          Succ = TempSucc;
          break;
        }
      }

    } else if (auto Switch = dyn_cast<HLSwitch>(Parent)) {

      if (std::next(Iter) != Switch->Children.end()) {
        TempSucc = std::next(Iter);

        bool IsSeparator = false;
        /// Check whether we are crossing separators.
        for (unsigned I = 0, E = Switch->getNumCases(); I < E; ++I) {
          if ((TempSucc == Switch->CaseBegin[I])) {
            IsSeparator = true;
            break;
          }
        }

        if (!IsSeparator) {
          Succ = TempSucc;
          break;
        }
      }

    } else {
      llvm_unreachable("Unexpected node parent type!");
    }

    Iter = Parent;
    Parent = Parent->getParent();
  }

  return Succ;
}

struct HLNodeUtils::TopSorter {
  unsigned TopSortNum;
  void visit(HLNode *Node) {
    if (isa<HLRegion>(Node)) {
      TopSortNum = 0;
    }
    TopSortNum += 50;
    Node->setTopSortNum(TopSortNum);
  }

  void postVisit(HLNode *) {}
  bool isDone() { return false; }
  bool skipRecursion(HLNode *Node) { return false; }
  TopSorter() : TopSortNum(0) {}
};

void HLNodeUtils::resetTopSortNum() {
  HLNodeUtils::TopSorter TS;
  HLNodeUtils::visitAll(&TS);
}

bool HLNodeUtils::strictlyDominates(HLNode *HIR1, HLNode *HIR2) {

  //  Is HIR1 strictly Dominates HIR2?
  //  based on HIR links and topsort order when dominators are not present.
  //  if HIR1 == HIR2, it will return false.
  //  for dd_refs in the same statement:   s = s + ..., caller needs to
  //  handle this case.
  //  Current code does not support constructs in switch

  unsigned Num1, Num2;
  Num1 = HIR1->getTopSortNum();
  Num2 = HIR2->getTopSortNum();
  if (Num1 >= Num2) {
    return false;
  }

  HLNode *Parent1 = HIR1->getParent();
  HLNode *Parent2 = HIR2->getParent();
  HLNode *tmpParent;

  if (isa<HLLoop>(Parent1) && isa<HLLoop>(Parent2)) {
    // immediate parent is loop and both HIR are in same loop
    if (Parent1 == Parent2) {
      return true;
    }
  }

  if (isa<HLLoop>(Parent1) || isa<HLIf>(Parent1)) {
    tmpParent = Parent2;
    while (tmpParent) {
      if (tmpParent == Parent1) {
        // immediate parent of HIR1 is Loop or If, trace back from HIR2 to
        // see if we can reach Parent1
        return true;
      }
      tmpParent = tmpParent->getParent();
    }
  }

  HLIf *If1 = dyn_cast<HLIf>(Parent1);
  HLIf *If2 = dyn_cast<HLIf>(Parent2);
  // When both are under the same IF stmt
  if (If1 == If2 && If1) {
    bool HIR1found = false;

    for (auto It = If1->then_begin(), E = If1->then_end(); It != E; ++It) {
      HLNode *HIRtemp = It;
      if (HIRtemp == HIR1) {
        HIR1found = true;
      } else if (HIRtemp == HIR2) {
        return HIR1found;
      }
    }
    if (HIR1found) {
      return false;
    }

    for (auto It = If1->else_begin(), E = If1->else_end(); It != E; ++It) {
      HLNode *HIRtemp = It;
      if (HIRtemp == HIR1) {
        HIR1found = true;
      } else if (HIRtemp == HIR2) {
        return HIR1found;
      }
    }
    if (HIR1found) {
      return false;
    }
    llvm_unreachable("Not expecting to be here");
  }

  // TODO: consider constant trip count across loops
  // This is not important and can defer

  return false;
}

//  Check if DDRef is contained in Loop
bool HLNodeUtils::LoopContainsDDRef(const HLLoop *Loop, const DDRef *DDref) {

  HLDDNode *DDNode = DDref->getHLDDNode();

  if (DDNode == Loop) {
    return true;
  }

  for (HLLoop *LP = DDNode->getParentLoop(); LP != nullptr;
       LP = LP->getParentLoop()) {
    if (LP == Loop) {
      return true;
    }
  }

  return false;
}

const HLLoop *HLNodeUtils::getParentLoopwithLevel(unsigned Level,
                                                  const HLLoop *InnermostLoop) {
  assert(InnermostLoop && " InnermostLoop is null.");
  assert(Level > 0 && Level <= MaxLoopNestLevel && " Level is invalid.");
  // level is at least 1
  // parentLoop is immediate parent
  // return nullptr for invalid inputs
  HLLoop *Loop;
  for (Loop = const_cast<HLLoop *>(InnermostLoop); Loop != nullptr;
       Loop = Loop->getParentLoop()) {
    if (Level == Loop->getNestingLevel()) {
      return Loop;
    }
  }
  return nullptr;
}

// Switch-Call Visitor to check if Switch or Call exists.
struct SwitchCallVisitor {
  bool IsSwitch, IsCall;

  void visit(HLSwitch *Switch) { IsSwitch = true; }

  void visit(HLInst *Inst) {
    if (Inst->isCallInst()) {
      IsCall = true;
    }
  }

  void visit(HLNode *Node) {}
  void postVisit(HLNode *) {}
  bool isDone() { return (IsSwitch || IsCall); }
  bool skipRecursion(HLNode *Node) { return false; }
  SwitchCallVisitor() : IsSwitch(false), IsCall(false) {}
};

bool HLNodeUtils::hasSwitchOrCall(const HLNode *NodeStart,
                                  const HLNode *NodeEnd,
                                  bool RecurseInsideLoops) {
  assert(NodeStart && NodeEnd && " Node Start/End is null.");
  SwitchCallVisitor SCVisit;
  HLNodeUtils::visit(&SCVisit, const_cast<HLNode *>(NodeStart),
                     const_cast<HLNode *>(NodeEnd), true, RecurseInsideLoops);
  return (SCVisit.IsSwitch || SCVisit.IsCall);
}

// Visitor to gather innermost loops.
struct InnermostLoopVisitor {

  SmallVectorImpl<const HLLoop *> *InnerLoops;
  HLNode *SkipNode;

  InnermostLoopVisitor(SmallVectorImpl<const HLLoop *> *Loops)
      : InnerLoops(Loops) {}

  void visit(HLLoop *L) {
    if (L->isInnermost()) {
      InnerLoops->push_back(L);
      SkipNode = L;
    }
  }
  void visit(HLNode *Node) {}
  void postVisit(HLNode *Node) {}
  bool isDone() { return false; }
  bool skipRecursion(HLNode *Node) { return (Node == SkipNode); }
};

void HLNodeUtils::gatherInnermostLoops(SmallVectorImpl<const HLLoop *> *Loops) {
  assert(Loops && " Loops parameter is null.");
  InnermostLoopVisitor LoopVisit(Loops);
  HLNodeUtils::visitAll<InnermostLoopVisitor>(&LoopVisit);
}

// Visitor to gather loops with specified level.
struct LoopLevelVisitor {

  SmallVectorImpl<const HLLoop *> *Loops;
  unsigned Level;
  HLNode *SkipNode;

  LoopLevelVisitor(SmallVectorImpl<const HLLoop *> *LoopContainer, unsigned Lvl)
      : Loops(LoopContainer), Level(Lvl) {}

  void visit(HLLoop *L) {
    if (L->getNestingLevel() == Level) {
      Loops->push_back(L);
      SkipNode = L;
    }
  }
  void visit(HLNode *Node) {}
  void postVisit(HLNode *Node) {}
  bool isDone() { return false; }
  bool skipRecursion(HLNode *Node) { return (Node == SkipNode); }
};

void HLNodeUtils::gatherOutermostLoops(SmallVectorImpl<const HLLoop *> *Loops) {
  assert(Loops && " Loops parameter is null.");
  // Level 1 denotes outermost loops
  LoopLevelVisitor LoopVisit(Loops, 1);
  HLNodeUtils::visitAll<LoopLevelVisitor>(&LoopVisit);
}

void HLNodeUtils::gatherLoopswithLevel(const HLNode *Node,
                                       SmallVectorImpl<const HLLoop *> *Loops,
                                       unsigned Level) {
  assert(Node && " Node is null.");
  assert(Loops && " Loops parameter is null.");
  assert(Level > 0 && Level <= MaxLoopNestLevel && " Level is out of range.");
  LoopLevelVisitor LoopVisit(Loops, Level);
  HLNodeUtils::visit<LoopLevelVisitor>(&LoopVisit, const_cast<HLNode *>(Node));
}
