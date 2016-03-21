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

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"

#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "hlnode-utils"
using namespace llvm;
using namespace loopopt;

HLNodeUtils::DummyIRBuilderTy *HLNodeUtils::DummyIRBuilder(nullptr);
Instruction *HLNodeUtils::FirstDummyInst(nullptr);
Instruction *HLNodeUtils::LastDummyInst(nullptr);

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

HLIf *HLNodeUtils::createHLIf(PredicateTy FirstPred, RegDDRef *Ref1,
                              RegDDRef *Ref2) {
  return new HLIf(FirstPred, Ref1, Ref2);
}

HLLoop *HLNodeUtils::createHLLoop(const Loop *LLVMLoop) {
  return new HLLoop(LLVMLoop);
}

HLLoop *HLNodeUtils::createHLLoop(HLIf *ZttIf, RegDDRef *LowerDDRef,
                                  RegDDRef *UpperDDRef, RegDDRef *StrideDDRef,
                                  unsigned NumEx) {
  return new HLLoop(ZttIf, LowerDDRef, UpperDDRef, StrideDDRef, NumEx);
}

void HLNodeUtils::destroy(HLNode *Node) { Node->destroy(); }

void HLNodeUtils::setFirstAndLastDummyInst(Instruction *Inst) {
  if (!FirstDummyInst) {
    FirstDummyInst = Inst;
  }

  LastDummyInst = Inst;
}

void HLNodeUtils::initialize(Function &F) {

  DummyIRBuilder = new DummyIRBuilderTy(F.getContext());
  DummyIRBuilder->SetInsertPoint(F.getEntryBlock().getTerminator());

  FirstDummyInst = nullptr;
  LastDummyInst = nullptr;
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

  llvm_unreachable("Unhandled type!");

  return nullptr;
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
    LvalRef = DDRefUtils::createSelfBlobRef(Inst);
  }

  HInst->setLvalDDRef(LvalRef);

  return HInst;
}

HLInst *HLNodeUtils::createUnaryHLInst(unsigned OpCode, RegDDRef *RvalRef,
                                       const Twine &Name, RegDDRef *LvalRef,
                                       Type *DestTy, bool IsVolatile,
                                       unsigned Align) {
  Value *InstVal = nullptr;
  Instruction *Inst = nullptr;
  HLInst *HInst = nullptr;

  checkUnaryInstOperands(LvalRef, RvalRef, DestTy);

  // Create dummy val.
  auto ZeroVal = createZeroVal(RvalRef->getDestType());

  switch (OpCode) {
  case Instruction::Load: {
    assert(!RvalRef->isTerminalRef() &&
           "Rval of load instruction cannot be scalar!");

    if (LvalRef) {
      assert(LvalRef->isTerminalRef() &&
             "Lval of load instruction is not a scalar!");
    }

    auto NullPtr = createZeroVal(RvalRef->getDestType());

    if (Align) {
      InstVal =
          DummyIRBuilder->CreateAlignedLoad(NullPtr, Align, IsVolatile, Name);
    } else {
      InstVal = DummyIRBuilder->CreateLoad(NullPtr, IsVolatile, Name);
    }

    break;
  }

  case Instruction::Store: {
    if (LvalRef) {
      assert(!LvalRef->isTerminalRef() &&
             "Lval of store instruction cannot be scalar!");
    }

    auto NullPtr = createZeroVal(RvalRef->getDestType());

    if (Align) {
      InstVal = DummyIRBuilder->CreateAlignedStore(ZeroVal, NullPtr, Align,
                                                   IsVolatile);
    } else {
      InstVal = DummyIRBuilder->CreateStore(ZeroVal, NullPtr, IsVolatile);
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
  case Instruction::AddrSpaceCast: {

    InstVal = DummyIRBuilder->CreateCast((Instruction::CastOps)OpCode, ZeroVal,
                                         DestTy, Name);
    break;
  }

  default:
    assert(false && "Instruction not handled!");
  }

  Inst = cast<Instruction>(InstVal);

  HInst = createLvalHLInst(Inst, LvalRef);
  HInst->setRvalDDRef(RvalRef);

  return HInst;
}

HLInst *HLNodeUtils::createCopyInst(RegDDRef *RvalRef, const Twine &Name,
                                    RegDDRef *LvalRef) {
  Value *InstVal;
  Instruction *Inst;
  HLInst *HInst;

  checkUnaryInstOperands(LvalRef, RvalRef, nullptr);

  // Create dummy val.
  auto ZeroVal = createZeroVal(RvalRef->getDestType());

  // Cannot use IRBuilder here as it returns the same value for casts with
  // identical src and dest types.
  InstVal =
      CastInst::Create(Instruction::BitCast, ZeroVal, ZeroVal->getType(), Name);
  Inst = cast<Instruction>(InstVal);
  Inst->insertBefore(&*(DummyIRBuilder->GetInsertPoint()));

  HInst = createLvalHLInst(Inst, LvalRef);
  HInst->setRvalDDRef(RvalRef);

  return HInst;
}

HLInst *HLNodeUtils::createLoad(RegDDRef *RvalRef, const Twine &Name,
                                RegDDRef *LvalRef, bool IsVolatile,
                                unsigned Align) {
  return createUnaryHLInst(Instruction::Load, RvalRef, Name, LvalRef, nullptr,
                           IsVolatile, Align);
}

HLInst *HLNodeUtils::createStore(RegDDRef *RvalRef, const Twine &Name,
                                 RegDDRef *LvalRef, bool IsVolatile,
                                 unsigned Align) {
  return createUnaryHLInst(Instruction::Store, RvalRef, Name, LvalRef, nullptr,
                           IsVolatile, Align);
}

HLInst *HLNodeUtils::createTrunc(Type *DestTy, RegDDRef *RvalRef,
                                 const Twine &Name, RegDDRef *LvalRef) {
  return createUnaryHLInst(Instruction::Trunc, RvalRef, Name, LvalRef, DestTy,
                           false, 0);
}

HLInst *HLNodeUtils::createZExt(Type *DestTy, RegDDRef *RvalRef,
                                const Twine &Name, RegDDRef *LvalRef) {
  return createUnaryHLInst(Instruction::ZExt, RvalRef, Name, LvalRef, DestTy,
                           false, 0);
}

HLInst *HLNodeUtils::createSExt(Type *DestTy, RegDDRef *RvalRef,
                                const Twine &Name, RegDDRef *LvalRef) {
  return createUnaryHLInst(Instruction::SExt, RvalRef, Name, LvalRef, DestTy,
                           false, 0);
}

HLInst *HLNodeUtils::createFPToUI(Type *DestTy, RegDDRef *RvalRef,
                                  const Twine &Name, RegDDRef *LvalRef) {
  return createUnaryHLInst(Instruction::FPToUI, RvalRef, Name, LvalRef, DestTy,
                           false, 0);
}

HLInst *HLNodeUtils::createFPToSI(Type *DestTy, RegDDRef *RvalRef,
                                  const Twine &Name, RegDDRef *LvalRef) {
  return createUnaryHLInst(Instruction::FPToSI, RvalRef, Name, LvalRef, DestTy,
                           false, 0);
}

HLInst *HLNodeUtils::createUIToFP(Type *DestTy, RegDDRef *RvalRef,
                                  const Twine &Name, RegDDRef *LvalRef) {
  return createUnaryHLInst(Instruction::UIToFP, RvalRef, Name, LvalRef, DestTy,
                           false, 0);
}

HLInst *HLNodeUtils::createSIToFP(Type *DestTy, RegDDRef *RvalRef,
                                  const Twine &Name, RegDDRef *LvalRef) {
  return createUnaryHLInst(Instruction::SIToFP, RvalRef, Name, LvalRef, DestTy,
                           false, 0);
}

HLInst *HLNodeUtils::createFPTrunc(Type *DestTy, RegDDRef *RvalRef,
                                   const Twine &Name, RegDDRef *LvalRef) {
  return createUnaryHLInst(Instruction::FPTrunc, RvalRef, Name, LvalRef, DestTy,
                           false, 0);
}

HLInst *HLNodeUtils::createFPExt(Type *DestTy, RegDDRef *RvalRef,
                                 const Twine &Name, RegDDRef *LvalRef) {
  return createUnaryHLInst(Instruction::FPExt, RvalRef, Name, LvalRef, DestTy,
                           false, 0);
}

HLInst *HLNodeUtils::createPtrToInt(Type *DestTy, RegDDRef *RvalRef,
                                    const Twine &Name, RegDDRef *LvalRef) {
  return createUnaryHLInst(Instruction::PtrToInt, RvalRef, Name, LvalRef,
                           DestTy, false, 0);
}

HLInst *HLNodeUtils::createIntToPtr(Type *DestTy, RegDDRef *RvalRef,
                                    const Twine &Name, RegDDRef *LvalRef) {
  return createUnaryHLInst(Instruction::IntToPtr, RvalRef, Name, LvalRef,
                           DestTy, false, 0);
}

HLInst *HLNodeUtils::createBitCast(Type *DestTy, RegDDRef *RvalRef,
                                   const Twine &Name, RegDDRef *LvalRef) {
  return createUnaryHLInst(Instruction::BitCast, RvalRef, Name, LvalRef, DestTy,
                           false, 0);
}

HLInst *HLNodeUtils::createAddrSpaceCast(Type *DestTy, RegDDRef *RvalRef,
                                         const Twine &Name, RegDDRef *LvalRef) {
  return createUnaryHLInst(Instruction::AddrSpaceCast, RvalRef, Name, LvalRef,
                           DestTy, false, 0);
}

HLInst *HLNodeUtils::createBinaryHLInst(unsigned OpCode, RegDDRef *OpRef1,
                                        RegDDRef *OpRef2, const Twine &Name,
                                        RegDDRef *LvalRef, bool HasNUWOrExact,
                                        bool HasNSW, MDNode *FPMathTag) {
  Value *InstVal;
  Instruction *Inst;
  HLInst *HInst;

  checkBinaryInstOperands(LvalRef, OpRef1, OpRef2);

  // Create dummy val.
  auto OneVal = createOneVal(OpRef1->getDestType());

  switch (OpCode) {
  case Instruction::Add: {
    assert(OpRef1->getDestType()->isIntegerTy() &&
           "Operand is not an integer type!");
    InstVal =
        DummyIRBuilder->CreateAdd(OneVal, OneVal, Name, HasNUWOrExact, HasNSW);
    break;
  }

  case Instruction::FAdd: {
    assert(OpRef1->getDestType()->isFloatingPointTy() &&
           "Operand is not a floating point type!");
    InstVal = DummyIRBuilder->CreateFAdd(OneVal, OneVal, Name, FPMathTag);
    break;
  }

  case Instruction::Sub: {
    assert(OpRef1->getDestType()->isIntegerTy() &&
           "Operand is not an integer type!");
    InstVal =
        DummyIRBuilder->CreateSub(OneVal, OneVal, Name, HasNUWOrExact, HasNSW);
    break;
  }

  case Instruction::FSub: {
    assert(OpRef1->getDestType()->isFloatingPointTy() &&
           "Operand is not a floating point type!");
    InstVal = DummyIRBuilder->CreateFSub(OneVal, OneVal, Name, FPMathTag);
    break;
  }

  case Instruction::Mul: {
    assert(OpRef1->getDestType()->isIntegerTy() &&
           "Operand is not an integer type!");
    InstVal =
        DummyIRBuilder->CreateMul(OneVal, OneVal, Name, HasNUWOrExact, HasNSW);
    break;
  }

  case Instruction::FMul: {
    assert(OpRef1->getDestType()->isFloatingPointTy() &&
           "Operand is not a floating point type!");
    InstVal = DummyIRBuilder->CreateFMul(OneVal, OneVal, Name, FPMathTag);
    break;
  }

  case Instruction::UDiv: {
    assert(OpRef1->getDestType()->isIntegerTy() &&
           "Operand is not an integer type!");
    InstVal = DummyIRBuilder->CreateUDiv(OneVal, OneVal, Name, HasNUWOrExact);
    break;
  }

  case Instruction::SDiv: {
    assert(OpRef1->getDestType()->isIntegerTy() &&
           "Operand is not an integer type!");
    InstVal = DummyIRBuilder->CreateSDiv(OneVal, OneVal, Name, HasNUWOrExact);
    break;
  }

  case Instruction::FDiv: {
    assert(OpRef1->getDestType()->isFloatingPointTy() &&
           "Operand is not a floating point type!");
    InstVal = DummyIRBuilder->CreateFDiv(OneVal, OneVal, Name, FPMathTag);
    break;
  }

  case Instruction::URem: {
    assert(OpRef1->getDestType()->isIntegerTy() &&
           "Operand is not an integer type!");
    InstVal = DummyIRBuilder->CreateURem(OneVal, OneVal, Name);
    break;
  }

  case Instruction::SRem: {
    assert(OpRef1->getDestType()->isIntegerTy() &&
           "Operand is not an integer type!");
    InstVal = DummyIRBuilder->CreateSRem(OneVal, OneVal, Name);
    break;
  }

  case Instruction::FRem: {
    assert(OpRef1->getDestType()->isFloatingPointTy() &&
           "Operand is not a floating point type!");
    InstVal = DummyIRBuilder->CreateFRem(OneVal, OneVal, Name, FPMathTag);
    break;
  }

  case Instruction::Shl: {
    assert(OpRef1->getDestType()->isIntegerTy() &&
           "Operand is not an integer type!");
    InstVal =
        DummyIRBuilder->CreateShl(OneVal, OneVal, Name, HasNUWOrExact, HasNSW);
    break;
  }

  case Instruction::LShr: {
    assert(OpRef1->getDestType()->isIntegerTy() &&
           "Operand is not an integer type!");
    InstVal = DummyIRBuilder->CreateLShr(OneVal, OneVal, Name, HasNUWOrExact);
    break;
  }

  case Instruction::AShr: {
    assert(OpRef1->getDestType()->isIntegerTy() &&
           "Operand is not an integer type!");
    InstVal = DummyIRBuilder->CreateAShr(OneVal, OneVal, Name, HasNUWOrExact);
    break;
  }

  case Instruction::And: {
    assert(OpRef1->getDestType()->isIntegerTy() &&
           "Operand is not an integer type!");
    InstVal = DummyIRBuilder->CreateAnd(OneVal, OneVal, Name);
    break;
  }

  case Instruction::Or: {
    assert(OpRef1->getDestType()->isIntegerTy() &&
           "Operand is not an integer type!");
    InstVal = DummyIRBuilder->CreateOr(OneVal, OneVal, Name);
    break;
  }

  case Instruction::Xor: {
    assert(OpRef1->getDestType()->isIntegerTy() &&
           "Operand is not an integer type!");
    InstVal = DummyIRBuilder->CreateXor(OneVal, OneVal, Name);
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
                               const Twine &Name, RegDDRef *LvalRef,
                               bool HasNUW, bool HasNSW) {
  return createBinaryHLInst(Instruction::Add, OpRef1, OpRef2, Name, LvalRef,
                            HasNUW, HasNSW, nullptr);
}

HLInst *HLNodeUtils::createFAdd(RegDDRef *OpRef1, RegDDRef *OpRef2,
                                const Twine &Name, RegDDRef *LvalRef,
                                MDNode *FPMathTag) {
  return createBinaryHLInst(Instruction::FAdd, OpRef1, OpRef2, Name, LvalRef,
                            false, false, FPMathTag);
}

HLInst *HLNodeUtils::createSub(RegDDRef *OpRef1, RegDDRef *OpRef2,
                               const Twine &Name, RegDDRef *LvalRef,
                               bool HasNUW, bool HasNSW) {
  return createBinaryHLInst(Instruction::Sub, OpRef1, OpRef2, Name, LvalRef,
                            HasNUW, HasNSW, nullptr);
}

HLInst *HLNodeUtils::createFSub(RegDDRef *OpRef1, RegDDRef *OpRef2,
                                const Twine &Name, RegDDRef *LvalRef,
                                MDNode *FPMathTag) {
  return createBinaryHLInst(Instruction::FSub, OpRef1, OpRef2, Name, LvalRef,
                            false, false, FPMathTag);
}

HLInst *HLNodeUtils::createMul(RegDDRef *OpRef1, RegDDRef *OpRef2,
                               const Twine &Name, RegDDRef *LvalRef,
                               bool HasNUW, bool HasNSW) {
  return createBinaryHLInst(Instruction::Mul, OpRef1, OpRef2, Name, LvalRef,
                            HasNUW, HasNSW, nullptr);
}

HLInst *HLNodeUtils::createFMul(RegDDRef *OpRef1, RegDDRef *OpRef2,
                                const Twine &Name, RegDDRef *LvalRef,
                                MDNode *FPMathTag) {
  return createBinaryHLInst(Instruction::FMul, OpRef1, OpRef2, Name, LvalRef,
                            false, false, FPMathTag);
}

HLInst *HLNodeUtils::createUDiv(RegDDRef *OpRef1, RegDDRef *OpRef2,
                                const Twine &Name, RegDDRef *LvalRef,
                                bool IsExact) {
  return createBinaryHLInst(Instruction::UDiv, OpRef1, OpRef2, Name, LvalRef,
                            IsExact, false, nullptr);
}

HLInst *HLNodeUtils::createSDiv(RegDDRef *OpRef1, RegDDRef *OpRef2,
                                const Twine &Name, RegDDRef *LvalRef,
                                bool IsExact) {
  return createBinaryHLInst(Instruction::SDiv, OpRef1, OpRef2, Name, LvalRef,
                            IsExact, false, nullptr);
}

HLInst *HLNodeUtils::createFDiv(RegDDRef *OpRef1, RegDDRef *OpRef2,
                                const Twine &Name, RegDDRef *LvalRef,
                                MDNode *FPMathTag) {
  return createBinaryHLInst(Instruction::FDiv, OpRef1, OpRef2, Name, LvalRef,
                            false, false, FPMathTag);
}

HLInst *HLNodeUtils::createURem(RegDDRef *OpRef1, RegDDRef *OpRef2,
                                const Twine &Name, RegDDRef *LvalRef) {
  return createBinaryHLInst(Instruction::URem, OpRef1, OpRef2, Name, LvalRef,
                            false, false, nullptr);
}

HLInst *HLNodeUtils::createSRem(RegDDRef *OpRef1, RegDDRef *OpRef2,
                                const Twine &Name, RegDDRef *LvalRef) {
  return createBinaryHLInst(Instruction::SRem, OpRef1, OpRef2, Name, LvalRef,
                            false, false, nullptr);
}

HLInst *HLNodeUtils::createFRem(RegDDRef *OpRef1, RegDDRef *OpRef2,
                                const Twine &Name, RegDDRef *LvalRef,
                                MDNode *FPMathTag) {
  return createBinaryHLInst(Instruction::FRem, OpRef1, OpRef2, Name, LvalRef,
                            false, false, FPMathTag);
}

HLInst *HLNodeUtils::createShl(RegDDRef *OpRef1, RegDDRef *OpRef2,
                               const Twine &Name, RegDDRef *LvalRef,
                               bool HasNUW, bool HasNSW) {
  return createBinaryHLInst(Instruction::Shl, OpRef1, OpRef2, Name, LvalRef,
                            HasNUW, HasNSW, nullptr);
}

HLInst *HLNodeUtils::createLShr(RegDDRef *OpRef1, RegDDRef *OpRef2,
                                const Twine &Name, RegDDRef *LvalRef,
                                bool IsExact) {
  return createBinaryHLInst(Instruction::LShr, OpRef1, OpRef2, Name, LvalRef,
                            IsExact, false, nullptr);
}

HLInst *HLNodeUtils::createAShr(RegDDRef *OpRef1, RegDDRef *OpRef2,
                                const Twine &Name, RegDDRef *LvalRef,
                                bool IsExact) {
  return createBinaryHLInst(Instruction::AShr, OpRef1, OpRef2, Name, LvalRef,
                            IsExact, false, nullptr);
}

HLInst *HLNodeUtils::createAnd(RegDDRef *OpRef1, RegDDRef *OpRef2,
                               const Twine &Name, RegDDRef *LvalRef) {
  return createBinaryHLInst(Instruction::And, OpRef1, OpRef2, Name, LvalRef,
                            false, false, nullptr);
}

HLInst *HLNodeUtils::createOr(RegDDRef *OpRef1, RegDDRef *OpRef2,
                              const Twine &Name, RegDDRef *LvalRef) {
  return createBinaryHLInst(Instruction::Or, OpRef1, OpRef2, Name, LvalRef,
                            false, false, nullptr);
}

HLInst *HLNodeUtils::createXor(RegDDRef *OpRef1, RegDDRef *OpRef2,
                               const Twine &Name, RegDDRef *LvalRef) {
  return createBinaryHLInst(Instruction::Xor, OpRef1, OpRef2, Name, LvalRef,
                            false, false, nullptr);
}

HLInst *HLNodeUtils::createCmp(CmpInst::Predicate Pred, RegDDRef *OpRef1,
                               RegDDRef *OpRef2, const Twine &Name,
                               RegDDRef *LvalRef) {
  Value *InstVal;
  HLInst *HInst;

  checkBinaryInstOperands(nullptr, OpRef1, OpRef2);

  if (LvalRef) {
    assert((LvalRef->getDestType()->isIntegerTy() &&
            (LvalRef->getDestType()->getIntegerBitWidth() == 1)) &&
           "LvalRef has invalid type!");
  }

  auto ZeroVal = createZeroVal(OpRef1->getDestType());

  if (OpRef1->getDestType()->isIntegerTy() ||
      OpRef1->getDestType()->isPointerTy()) {
    InstVal =
        DummyIRBuilder->CreateICmp(ICmpInst::ICMP_EQ, ZeroVal, ZeroVal, Name);
  } else {
    InstVal =
        DummyIRBuilder->CreateFCmp(FCmpInst::FCMP_TRUE, ZeroVal, ZeroVal, Name);
  }

  HInst = createLvalHLInst(cast<Instruction>(InstVal), LvalRef);
  HInst->setPredicate(Pred);

  HInst->setOperandDDRef(OpRef1, 1);
  HInst->setOperandDDRef(OpRef2, 2);

  return HInst;
}

HLInst *HLNodeUtils::createSelect(CmpInst::Predicate Pred, RegDDRef *OpRef1,
                                  RegDDRef *OpRef2, RegDDRef *OpRef3,
                                  RegDDRef *OpRef4, const Twine &Name,
                                  RegDDRef *LvalRef) {
  Value *InstVal;
  HLInst *HInst;

  // LvalRef, OpRef3 and OpRef4 should be the same type.
  checkBinaryInstOperands(LvalRef, OpRef3, OpRef4);
  // OpRef1 and OpRef2 should be the same type.
  checkBinaryInstOperands(nullptr, OpRef1, OpRef2);

  auto CmpVal = createOneVal(Type::getInt1Ty(getHIRFramework()->getContext()));
  auto OpVal = createZeroVal(OpRef3->getDestType());

  InstVal = DummyIRBuilder->CreateSelect(CmpVal, OpVal, OpVal, Name);

  HInst = createLvalHLInst(cast<Instruction>(InstVal), LvalRef);
  HInst->setPredicate(Pred);

  HInst->setOperandDDRef(OpRef1, 1);
  HInst->setOperandDDRef(OpRef2, 2);
  HInst->setOperandDDRef(OpRef3, 3);
  HInst->setOperandDDRef(OpRef4, 4);

  return HInst;
}

struct HLNodeUtils::CloneVisitor final : public HLNodeVisitorBase {

  HLContainerTy *CloneContainer;
  GotoContainerTy *GotoList;
  LabelMapTy *LabelMap;

  CloneVisitor(HLContainerTy *Container, GotoContainerTy *GList,
               LabelMapTy *LMap)
      : CloneContainer(Container), GotoList(GList), LabelMap(LMap) {}

  void visit(const HLNode *Node) {
    CloneContainer->push_back(Node->cloneImpl(GotoList, LabelMap));
  }

  void postVisit(const HLNode *Node) {}

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

  HLNodeUtils::CloneVisitor CloneVisit(CloneContainer, &GotoList, &LabelMap);
  visitRange<false>(CloneVisit, Node1, Node2);
  CloneVisit.postVisitUpdate();
}

// Used for cloning a sequence of nodes from Node1 to Node2.
void HLNodeUtils::cloneSequence(HLContainerTy *CloneContainer,
                                const HLNode *Node1, const HLNode *Node2) {
  assert(Node1 && !isa<HLRegion>(Node1) &&
         " Node1 - Region Cloning is not allowed.");
  assert((!Node2 || !isa<HLRegion>(Node2)) &&
         " Node 2 - Region Cloning is not allowed.");
  assert(CloneContainer && " Clone Container is null.");
  assert((!Node2 || (Node1->getParent() == Node2->getParent())) &&
         " Parent of Node1 and Node2 don't match.");
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
struct HLNodeUtils::LoopFinderUpdater final : public HLNodeVisitorBase {

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
    InsertContainer.insert(Pos, &*(First));
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
      unsigned E = (CaseNum == -1) ? Switch->getNumCases() : CaseNum;
      for (unsigned I = 0; I < E; ++I) {
        if (Pos == Switch->CaseBegin[I]) {
          Switch->CaseBegin[I] = std::prev(Pos, Count);
        }
      }
    }

  } else {
    llvm_unreachable("Unknown parent type!");
  }

  assert(InsertContainer && "InsertContainer is null");

  // Update TopSortNum
  updateTopSortNum(*InsertContainer, First, Pos);

  // Update loop info for loops in this range.
  updateLoopInfoRecursively(std::prev(Pos, Count), Pos);
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
  insertImpl(Switch, isFirstChild ? Switch->case_child_begin_internal(CaseNum)
                                  : Switch->case_child_end_internal(CaseNum),
             OrigContainer, First, Last, (CaseNum != 0), false, CaseNum);
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
    Node = Container.remove(I);

    if (Erase) {
      destroy(Node);
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

void HLNodeUtils::remove(HLContainerTy *Container, HLNode *Node1,
                         HLNode *Node2) {
  assert(Node1 && Node2 && " Node1 or Node 2 cannot be null.");
  assert(Container && " Clone Container is null.");
  assert(!isa<HLRegion>(Node1) && !isa<HLRegion>(Node2) &&
         " Node1 or Node2 cannot be a HLRegion.");
  assert((Node1->getParent() == Node2->getParent()) &&
         " Parent of Node1 and Node2 don't match.");

  HLContainerTy::iterator ItStart(Node1);
  HLContainerTy::iterator ItEnd(Node2);
  removeImpl(ItStart, std::next(ItEnd), Container);
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
        if ((TempSucc != If->ElseBegin)) {
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
    auto FirstOrLastRegIter = Prev ? getHIRFramework()->hir_begin()
                                   : std::prev(getHIRFramework()->hir_end());
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

  if (Container.begin() != First) {
    PrevNum = getPrevLinkListNode(&*First)->getMaxTopSortNum();
  } else {
    PrevNum = Parent->getTopSortNum();
  }

  bool hasNextNode = Container.end() != Last;
  unsigned NextNum = hasNextNode ? Last->getTopSortNum() : 0;
  if (!NextNum) {
    // !isa<HLRegion>(Parent) - HLRegions are linked in the ilist, but we
    // should not iterate across regions. If we traced to an HLRegion,
    // this means that there is no next node in this region.
    for (; Parent && !isa<HLRegion>(Parent); Parent = Parent->getParent()) {
      if (auto NextNode = getNextLinkListNode(Parent)) {
        NextNum = NextNode->getTopSortNum();
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

// The visitor that sets TopSortNums and LexicalLastTopSortNum from MinNum
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

  TopSorter(unsigned MinNum, unsigned Step = 100,
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

bool HLNodeUtils::isInTopSortNumRange(const HLNode *Node,
                                      const HLNode *FirstNode,
                                      const HLNode *LastNode) {
  assert(Node && "Node is null!");

  if (!FirstNode) {
    return false;
  }

  assert(LastNode && "Last node is null!");

  unsigned Num = Node->getTopSortNum();
  unsigned FirstNum = FirstNode->getTopSortNum();
  unsigned LastNum = LastNode->getTopSortNum();

  return (Num >= FirstNum && Num <= LastNum);
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

  StructuredFlowChecker(bool PDom, const HLNode *TNode)
      : IsPDom(PDom), TargetNode(TNode), IsStructured(true), IsDone(false) {}

  void visit(const HLNode *Node) {
    if (Node == TargetNode) {
      IsDone = true;
      return;
    }

    if (!IsPDom) {
      if (isa<HLLabel>(Node)) {
        IsStructured = false;
      }
      return;
    }

    // Post domination logic.
    if (auto Goto = dyn_cast<HLGoto>(Node)) {
      if (Goto->isExternal()) {
        IsStructured = false;
        return;
      }

      auto Label = Goto->getTargetLabel();

      if (Label->getTopSortNum() > TargetNode->getTopSortNum()) {
        IsStructured = false;
      }

    } else if (auto Loop = dyn_cast<HLLoop>(Node)) {
      // Be conservative in the presence of multi-exit loops.
      if (Loop->getNumExits() > 1) {
        IsStructured = false;
      }
    }
  }

  void postVisit(const HLNode *) {}
  bool isDone() const override { return (IsDone || !IsStructured); }
  bool isStructured() { return IsStructured; }
};

bool HLNodeUtils::hasStructuredFlow(const HLNode *Parent, const HLNode *Node,
                                    const HLNode *TargetNode,
                                    bool PostDomination, bool UpwardTraversal) {
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

  StructuredFlowChecker SFC(PostDomination, TargetNode);

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
                           PostDomination)) {
      return nullptr;
    }

    *LastParent1 = Parent;
    Parent = Parent->getParent();
  }

  return Parent;
}

const HLNode *HLNodeUtils::getCommonDominatingParent(
    const HLNode *Parent1, const HLNode *LastParent1, const HLNode *Node2,
    bool PostDomination, const HLNode **LastParent2) {
  const HLNode *CommonParent = Node2->getParent();
  *LastParent2 = Node2;

  // Trace back Node2 to Parent1.
  while (CommonParent) {

    // Keep checking for structured flow for the nodes we come acoss while
    // moving up the chain.
    if (!hasStructuredFlow(CommonParent, *LastParent2, LastParent1,
                           PostDomination, !PostDomination)) {
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
                                bool PostDomination, bool StrictDomination) {

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
      getOutermostSafeParent(Node1, Node2, PostDomination, &LastParent1);

  // Could't find an appropriate parent for Node1.
  if (!Parent1) {
    return false;
  }

  const HLNode *LastParent2 = nullptr;
  const HLNode *CommonParent = getCommonDominatingParent(
      Parent1, LastParent1, Node2, PostDomination, &LastParent2);

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
  ;
}

bool HLNodeUtils::dominates(const HLNode *Node1, const HLNode *Node2) {
  return dominatesImpl(Node1, Node2, false, false);
}

bool HLNodeUtils::strictlyDominates(const HLNode *Node1, const HLNode *Node2) {
  return dominatesImpl(Node1, Node2, false, true);
}

bool HLNodeUtils::postDominates(const HLNode *Node1, const HLNode *Node2) {
  return dominatesImpl(Node1, Node2, true, false);
}

bool HLNodeUtils::strictlyPostDominates(const HLNode *Node1,
                                        const HLNode *Node2) {
  return dominatesImpl(Node1, Node2, true, true);
}

bool HLNodeUtils::canAccessTogether(const HLNode *Node1, const HLNode *Node2) {
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

  if (!(dominates(Node2, Node1) && postDominates(Node1, Node2)) &&
      !(dominates(Node1, Node2) && postDominates(Node2, Node1))) {
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
struct SwitchCallVisitor final : public HLNodeVisitorBase {
  bool IsSwitch, IsCall;

  void visit(const HLSwitch *Switch) { IsSwitch = true; }

  void visit(const HLInst *Inst) {
    if (Inst->isCallInst()) {
      IsCall = true;
    }
  }

  void visit(const HLNode *Node) {}
  void postVisit(const HLNode *) {}
  bool isDone() const override { return (IsSwitch || IsCall); }
  SwitchCallVisitor() : IsSwitch(false), IsCall(false) {}
};

bool HLNodeUtils::hasSwitchOrCall(const HLNode *NodeStart,
                                  const HLNode *NodeEnd,
                                  bool RecurseInsideLoops) {
  assert(NodeStart && NodeEnd && " Node Start/End is null.");
  SwitchCallVisitor SCVisit;
  if (RecurseInsideLoops) {
    HLNodeUtils::visitRange<true, true>(SCVisit, NodeStart, NodeEnd);
  } else {
    HLNodeUtils::visitRange<true, false>(SCVisit, NodeStart, NodeEnd);
  }
  return (SCVisit.IsSwitch || SCVisit.IsCall);
}

// Useful to detect if A(2 * N *I) will not be A(0) based on symbolic in
// UB.
// If the coeff could be  0, then we might end up with DV equals *
// e.g.
// If  UB = N  - 1 (UB cannot start with negative numbers in our framework),
// it implies N is at least 1
// Offset indicates the constant in CanonExpr. e.g. 3 as in  2 * i + 3
// then the coeff  2 * N is positive

static bool getMaxMinCoeffVal(int64_t Coeff, unsigned BlobIdx, int64_t Offset1,
                              int64_t UBCoeff, unsigned UBBlobIdx,
                              int64_t Offset2, int64_t *Val) {

#ifndef NDEBUG

  DEBUG(dbgs() << "\n\t in getMaxMinCoeffVal: input args " << Coeff << " "
               << BlobIdx << " " << Offset1 << " " << UBCoeff << " "
               << UBBlobIdx << " " << Offset2);
#endif

  if (BlobIdx != UBBlobIdx || UBCoeff == 0) {
    return false;
  }

  //  max/min value of UB =  - Offset2 / UBCoeff
  int64_t BlobVal = -Offset2 / UBCoeff;

  //  Substitute value in Blob

  *Val = Coeff * BlobVal + Offset1;
  return true;
}

// Get possible Max/Minimum  value of canon.
// Only handles single Blob + constant - No support for IV now
// If known, return true and Value.
// *IsMax=true returned indicates if Val is to be used as the Max
// or Min.

enum VALType : unsigned { IsConstant, IsMax, IsMin };

static bool getMaxMinValue(const CanonExpr *CE, int64_t *Val, VALType *ValType,
                           const HLLoop *ParentLoop) {

  const HLLoop *Loop;
  *ValType = VALType::IsConstant;

  if (CE->isNonLinear()) {
    return false;
  }

  if (CE->isIntConstant(Val)) {
    return true;
  }

  // Handles single blob + constant for now

  if (ParentLoop == nullptr || CE->numBlobs() != 1 || CE->hasIV() ||
      CE->getDenominator() != 1) {
    return false;
  }

  int64_t Coeff = CE->getSingleBlobCoeff();
  unsigned BlobIdx = CE->getSingleBlobIndex();

  assert(Coeff != 0 && "Blob cannot be zero here");

  for (Loop = ParentLoop; Loop != nullptr; Loop = Loop->getParentLoop()) {

    // Skip unknown loops and proceed to a parent loop as it can contain
    // target blob in the upper bound
    if (Loop->isUnknown()) {
      continue;
    }

    const CanonExpr *UB = Loop->getUpperCanonExpr();

    if (UB->numBlobs() != 1 || UB->hasIV()) {
      continue;
    }

    int64_t UBCoeff = UB->getSingleBlobCoeff();
    unsigned UBBlobIdx = UB->getSingleBlobIndex();

    // Notice that for A[ (2 *n +3)* i + 4]
    // Coeff = 2,  blob = n,  constant = 3
    // 4 is not being passed to here. The caller in DD Test
    // extracts 2 * n + 3 to form another canonexpr for the coeff
    // In  Upper bound (e.g. in the form 3 *N +4, our  LB is never negative),
    // positive coef (3) indicates what the min value of N could be
    // negative coef (e.g. as in 4 - 3 *N) indicates what the max value of N
    // could be.
    // Based on the UB,
    // postive / negative coeff in incoming CanonExpr determines if max or min
    // value can be computed1
    //  Coeff of N in  UB        Coeff in CanonExpr
    //  0                        NA
    //                           0      Is Constant
    //  > 0                      > 0    min value
    //  > 0                      < 0    max value
    //  < 0                      > 0    max value
    //  < 0                      < 0    min value

    if ((UBCoeff > 0 && (Coeff > 0)) || (UBCoeff < 0 && (Coeff < 0))) {
      *ValType = VALType::IsMin;
    } else {
      *ValType = VALType::IsMax;
    }

    if (getMaxMinCoeffVal(Coeff, BlobIdx, CE->getConstant(), UBCoeff, UBBlobIdx,
                          UB->getConstant(), Val)) {
      return true;
    }
  }

  return false;
}

bool HLNodeUtils::isKnownPositive(const CanonExpr *CE,
                                  const HLLoop *ParentLoop) {

  int64_t Val;
  VALType ValType;

  // If Min value is > 0  or IsConstant, then we can compare
  // Notice the checking is done by using  !IsMax
  if (getMaxMinValue(CE, &Val, &ValType, ParentLoop) &&
      ValType != VALType::IsMax && (Val > 0)) {
    return true;
  }
  return false;
}

bool HLNodeUtils::isKnownNonNegative(const CanonExpr *CE,
                                     const HLLoop *ParentLoop) {

  int64_t Val;
  VALType ValType;

  if (getMaxMinValue(CE, &Val, &ValType, ParentLoop) &&
      ValType != VALType::IsMax && (Val >= 0)) {
    return true;
  }
  return false;
}

bool HLNodeUtils::isKnownNegative(const CanonExpr *CE,
                                  const HLLoop *ParentLoop) {

  int64_t Val;
  VALType ValType;
  if (getMaxMinValue(CE, &Val, &ValType, ParentLoop) &&
      ValType != VALType::IsMin && (Val < 0)) {
    return true;
  }
  return false;
}

bool HLNodeUtils::isKnownNonPositive(const CanonExpr *CE,
                                     const HLLoop *ParentLoop) {

  int64_t Val;
  VALType ValType;

  if (getMaxMinValue(CE, &Val, &ValType, ParentLoop) &&
      ValType != VALType::IsMin && (Val < 1)) {
    return true;
  }
  return false;
}

bool HLNodeUtils::isKnownNonZero(const CanonExpr *CE,
                                 const HLLoop *ParentLoop) {

  int64_t Val;
  if (CE->isIntConstant(&Val) && (Val != 0)) {
    return true;
  }
  if (isKnownPositive(CE, ParentLoop) || isKnownNegative(CE, ParentLoop)) {
    return true;
  }
  return false;
}

///  Check if Loop has perfect loop nests
///  Default to allow pre and post header is false
///  Default to allow Triangular loop is false with  exceptions
///  made for first iteration
///  Does not consider innermost loops as perfect loops
bool HLNodeUtils::isPerfectLoopNest(const HLLoop *Loop,
                                    const HLLoop **InnermostLoop,
                                    bool AllowPrePostHdr,
                                    bool AllowTriangularLoop) {
  bool FirstIter = true;
  *InnermostLoop = nullptr;

  assert(Loop && "Loop must not be nullptr");

  for (const HLLoop *Lp = Loop; Lp != nullptr;
       Lp = dyn_cast<HLLoop>(Lp->getFirstChild())) {
    const CanonExpr *UpperBound;
    if (!Lp || !Lp->isDo()) {
      break;
    }
    if (!FirstIter && !AllowPrePostHdr &&
        (Lp->hasPreheader() || Lp->hasPostexit())) {
      break;
    }

    UpperBound = Lp->getUpperCanonExpr();

    if (!AllowTriangularLoop && !FirstIter && UpperBound->hasIV()) {
      //  okay for outermost loop UB to have iv
      break;
    }

    if (Lp->isInnermost()) {
      if (FirstIter) {
        return false;
      }
      *InnermostLoop = Lp;
      return true;
    }

    if (Lp->getNumChildren() != 1) {
      break;
    }

    FirstIter = false;
  }

  return false;
}

///  Move Ztt, Bounds DDRRef, OrigLoop, IvType
void HLNodeUtils::moveProperties(HLLoop *SrcLoop, HLLoop *DstLoop) {

  DstLoop->setIVType(SrcLoop->getIVType());
  DstLoop->removeZtt();

  if (SrcLoop->hasZtt()) {
    DstLoop->setZtt(SrcLoop->removeZtt());
  }

  // setLower etc. requires input DDRef not be attached to anything
  DstLoop->setLowerDDRef(SrcLoop->removeLowerDDRef());
  DstLoop->setUpperDDRef(SrcLoop->removeUpperDDRef());
  DstLoop->setStrideDDRef(SrcLoop->removeStrideDDRef());
  DstLoop->setLLVMLoop(SrcLoop->removeLLVMLoop());
}

/// Update Loop properties based on Input Permutations
/// Used by Loop Interchange now. Might be useful for loop blocking later
void HLNodeUtils::permuteLoopNests(
    HLLoop *Loop, SmallVector<HLLoop *, MaxLoopNestLevel> LoopPermutation) {

  SmallVector<HLLoop *, MaxLoopNestLevel> LoopSaved;
  HLLoop *DstLoop = Loop;

  for (auto &I : LoopPermutation) {
    HLLoop *LoopCopy = I->cloneCompleteEmptyLoop();
    LoopSaved.push_back(LoopCopy);
  }

  for (auto I = LoopPermutation.begin(), E = LoopPermutation.end(); I != E;
       ++I, DstLoop = dyn_cast<HLLoop>(DstLoop->getFirstChild())) {
    HLLoop *SrcLoop = nullptr;
    if (*I == DstLoop) {
      continue;
    }
    assert(isa<HLLoop>(DstLoop) && "Perfect loop nest expected");
    for (auto &L : LoopSaved) {
      if ((*I)->getNestingLevel() == L->getNestingLevel()) {
        SrcLoop = L;
        break;
      }
    }
    assert(SrcLoop && "Input Loop is null");
    assert(DstLoop != SrcLoop && "Dst, Src loop cannot be equal");
    moveProperties(SrcLoop, DstLoop);
  }
}
