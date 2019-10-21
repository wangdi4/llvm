//===------- HLNodeUtils.cpp - Implements HLNodeUtils class ---------------===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
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

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/MDBuilder.h"
#include "llvm/IR/Metadata.h" // needed for MetadataAsValue -> Value
#include "llvm/Support/Debug.h"
#include <llvm/IR/IntrinsicInst.h>

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

  DummyIRBuilder.reset(new DummyIRBuilderTy(F.getContext()));
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

HLGoto *HLNodeUtils::createHLGoto(BasicBlock *SrcBB, BasicBlock *TargetBB) {
  return new HLGoto(*this, SrcBB, TargetBB);
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
  (void)Count;
  assert(Count && "Node not found in objects!");

  delete Node;
}

void HLNodeUtils::setFirstAndLastDummyInst(Instruction *Inst) {
  if (!FirstDummyInst) {
    FirstDummyInst = Inst;
  }

  LastDummyInst = Inst;
}

HLNodeUtils::~HLNodeUtils() {
  for (auto &I : Objs) {
    delete I;
  }
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

HLInst *HLNodeUtils::createUnaryHLInst(
    unsigned OpCode, RegDDRef *RvalRef, const Twine &Name, RegDDRef *LvalRef,
    Type *DestTy, const UnaryInstruction *OrigUnInst) {
  HLInst *HInst = createUnaryHLInstImpl(OpCode, RvalRef, Name, LvalRef, DestTy,
                                        nullptr);
  if (OrigUnInst) {
    UnaryInstruction *NewUnInstr = cast<UnaryInstruction>(
        const_cast<Instruction *>(HInst->getLLVMInstruction()));
    NewUnInstr->copyIRFlags(OrigUnInst);
  }

  return HInst;
}

HLInst *HLNodeUtils::createUnaryHLInstImpl(unsigned OpCode, RegDDRef *RvalRef,
                                           const Twine &Name, RegDDRef *LvalRef,
                                           Type *DestTy, MDNode *FPMathTag) {
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

    auto DummyPtrType = PointerType::get(RvalRef->getDestType(),
                                         RvalRef->getPointerAddressSpace());
    auto DummyPtrVal = UndefValue::get(DummyPtrType);

    InstVal = DummyIRBuilder->CreateLoad(DummyPtrVal, false, Name);
    break;
  }

  case Instruction::Store: {
    assert(LvalRef && LvalRef->isMemRef() &&
           "Lval of store instruction should be a non-null mem ref");

    auto DummyPtrType = PointerType::get(LvalRef->getDestType(),
                                         LvalRef->getPointerAddressSpace());
    auto DummyPtrVal = UndefValue::get(DummyPtrType);

    InstVal = DummyIRBuilder->CreateStore(DummyVal, DummyPtrVal);
    break;
  }

  case Instruction::FNeg: {
    assert(RvalRef->getDestType()->isFPOrFPVectorTy() &&
           "Operand is not an float type!");
    InstVal = DummyIRBuilder->CreateFNeg(DummyVal, Name, FPMathTag);
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

  setFirstAndLastDummyInst(Inst);

  return Inst;
}

RegDDRef *HLNodeUtils::createTemp(Type *Ty, const Twine &Name) {
  auto Inst = createCopyInstImpl(Ty, Name);

  return getDDRefUtils().createSelfBlobRef(Inst);
}

unsigned HLNodeUtils::createAndReplaceTemp(RegDDRef *TempRef,
                                           const Twine &Name) {
  assert(TempRef && "TempRef is null!");
  assert(TempRef->isTerminalRef() && "TempRef is suppored to be a terminal!");

  auto Inst = createCopyInstImpl(TempRef->getDestType(), Name);

  unsigned Symbase = getHIRFramework().getNewSymbase();
  unsigned Index = 0;

  getBlobUtils().createBlob(Inst, Symbase, true, &Index);

  if (TempRef->isSelfBlob()) {
    TempRef->replaceSelfBlobIndex(Index);
  } else {
    TempRef->setSymbase(Symbase);
  }

  return Index;
}

HLInst *HLNodeUtils::createCopyInst(RegDDRef *RvalRef, const Twine &Name,
                                    RegDDRef *LvalRef) {
  checkUnaryInstOperands(LvalRef, RvalRef, nullptr);

  auto Inst = createCopyInstImpl(RvalRef->getDestType(), Name);

  auto HInst = createLvalHLInst(Inst, LvalRef);
  HInst->setRvalDDRef(RvalRef);

  return HInst;
}

unsigned HLNodeUtils::createAlloca(Type *Ty, HLRegion *Reg, const Twine &Name) {
  // Adjust the insertion point to pointer before the first dummy instruction as
  // we do not want alloca to be a dummy instruction.
  auto SavedInsertPt = DummyIRBuilder->GetInsertPoint();
  auto InsertBB = SavedInsertPt->getParent();

  // If we have inserted dummy instructions before, insert before them.
  if (FirstDummyInst) {
    DummyIRBuilder->SetInsertPoint(FirstDummyInst);
  }

  auto Inst = DummyIRBuilder->CreateAlloca(Ty, nullptr, Name);

  // Reset the insertion point.
  DummyIRBuilder->SetInsertPoint(InsertBB, SavedInsertPt);

  // Add a blob entry for the alloca instruction and return its index.
  unsigned AllocaBlobIndex;
  unsigned NewSymbase = getDDRefUtils().getNewSymbase();

  getBlobUtils().createBlob(Inst, NewSymbase, true, &AllocaBlobIndex);
  Reg->addLiveInTemp(NewSymbase, Inst);

  return AllocaBlobIndex;
}

HLInst *HLNodeUtils::createAlloca(Type *Ty, RegDDRef *ArraySizeRvalRef,
                                  const Twine &Name) {
  // Create dummy val.
  auto DummyVal = UndefValue::get(ArraySizeRvalRef->getDestType());

  Value *InstVal = DummyIRBuilder->CreateAlloca(Ty, DummyVal, Name);

  assert(isa<Instruction>(InstVal) && "Expected instruction!");
  Instruction *Inst = cast<Instruction>(InstVal);

  RegDDRef *LvalRef = nullptr;
  HLInst *HInst = createLvalHLInst(Inst, LvalRef);
  HInst->setRvalDDRef(ArraySizeRvalRef);

  return HInst;
}

HLInst *HLNodeUtils::createLoad(RegDDRef *RvalRef, const Twine &Name,
                                RegDDRef *LvalRef) {
  return createUnaryHLInstImpl(Instruction::Load, RvalRef, Name, LvalRef,
                               nullptr, nullptr);
}

HLInst *HLNodeUtils::createStore(RegDDRef *RvalRef, const Twine &Name,
                                 RegDDRef *LvalRef) {
  return createUnaryHLInstImpl(Instruction::Store, RvalRef, Name, LvalRef,
                               nullptr, nullptr);
}

HLInst *HLNodeUtils::createTrunc(Type *DestTy, RegDDRef *RvalRef,
                                 const Twine &Name, RegDDRef *LvalRef) {
  return createUnaryHLInstImpl(Instruction::Trunc, RvalRef, Name, LvalRef,
                               DestTy, nullptr);
}

HLInst *HLNodeUtils::createZExt(Type *DestTy, RegDDRef *RvalRef,
                                const Twine &Name, RegDDRef *LvalRef) {
  return createUnaryHLInstImpl(Instruction::ZExt, RvalRef, Name, LvalRef,
                               DestTy, nullptr);
}

HLInst *HLNodeUtils::createSExt(Type *DestTy, RegDDRef *RvalRef,
                                const Twine &Name, RegDDRef *LvalRef) {
  return createUnaryHLInstImpl(Instruction::SExt, RvalRef, Name, LvalRef,
                               DestTy, nullptr);
}

HLInst *HLNodeUtils::createFPToUI(Type *DestTy, RegDDRef *RvalRef,
                                  const Twine &Name, RegDDRef *LvalRef) {
  return createUnaryHLInstImpl(Instruction::FPToUI, RvalRef, Name, LvalRef,
                               DestTy, nullptr);
}

HLInst *HLNodeUtils::createFPToSI(Type *DestTy, RegDDRef *RvalRef,
                                  const Twine &Name, RegDDRef *LvalRef) {
  return createUnaryHLInstImpl(Instruction::FPToSI, RvalRef, Name, LvalRef,
                               DestTy, nullptr);
}

HLInst *HLNodeUtils::createUIToFP(Type *DestTy, RegDDRef *RvalRef,
                                  const Twine &Name, RegDDRef *LvalRef) {
  return createUnaryHLInstImpl(Instruction::UIToFP, RvalRef, Name, LvalRef,
                               DestTy, nullptr);
}

HLInst *HLNodeUtils::createSIToFP(Type *DestTy, RegDDRef *RvalRef,
                                  const Twine &Name, RegDDRef *LvalRef) {
  return createUnaryHLInstImpl(Instruction::SIToFP, RvalRef, Name, LvalRef,
                               DestTy, nullptr);
}

HLInst *HLNodeUtils::createFPTrunc(Type *DestTy, RegDDRef *RvalRef,
                                   const Twine &Name, RegDDRef *LvalRef) {
  return createUnaryHLInstImpl(Instruction::FPTrunc, RvalRef, Name, LvalRef,
                               DestTy, nullptr);
}

HLInst *HLNodeUtils::createFPExt(Type *DestTy, RegDDRef *RvalRef,
                                 const Twine &Name, RegDDRef *LvalRef) {
  return createUnaryHLInstImpl(Instruction::FPExt, RvalRef, Name, LvalRef,
                               DestTy, nullptr);
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
  return createUnaryHLInstImpl(Instruction::PtrToInt, RvalRef, Name, LvalRef,
                               DestTy, nullptr);
}

HLInst *HLNodeUtils::createIntToPtr(Type *DestTy, RegDDRef *RvalRef,
                                    const Twine &Name, RegDDRef *LvalRef) {
  return createUnaryHLInstImpl(Instruction::IntToPtr, RvalRef, Name, LvalRef,
                               DestTy, nullptr);
}

HLInst *HLNodeUtils::createBitCast(Type *DestTy, RegDDRef *RvalRef,
                                   const Twine &Name, RegDDRef *LvalRef) {
  return createUnaryHLInstImpl(Instruction::BitCast, RvalRef, Name, LvalRef,
                               DestTy, nullptr);
}

HLInst *HLNodeUtils::createAddrSpaceCast(Type *DestTy, RegDDRef *RvalRef,
                                         const Twine &Name, RegDDRef *LvalRef) {
  return createUnaryHLInstImpl(Instruction::AddrSpaceCast, RvalRef, Name,
                               LvalRef, DestTy, nullptr);
}

HLInst *HLNodeUtils::createFNeg(RegDDRef *RvalRef, const Twine &Name,
                                RegDDRef *LvalRef, MDNode *FPMathTag) {
  return createUnaryHLInstImpl(Instruction::FNeg, RvalRef, Name, LvalRef,
                               nullptr, FPMathTag);
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
                                             RegDDRef *Mask, const Twine &Name,
                                             RegDDRef *LvalRef) {
  assert(OpRef1->getDestType()->isVectorTy() &&
         OpRef1->getDestType() == OpRef2->getDestType() &&
         "Illegal operand types for shufflevector");

  auto UndefVal = UndefValue::get(OpRef1->getDestType());
  auto MaskUndefVal = UndefValue::get(Mask->getDestType());

  Value *InstVal = DummyIRBuilder->CreateShuffleVector(UndefVal, UndefVal,
                                                       MaskUndefVal, Name);
  Instruction *Inst = cast<Instruction>(InstVal);

  assert((!LvalRef || LvalRef->getDestType() == Inst->getType()) &&
         "Incompatible type of LvalRef");

  HLInst *HInst = createLvalHLInst(Inst, LvalRef);
  HInst->setOperandDDRef(OpRef1, 1);
  HInst->setOperandDDRef(OpRef2, 2);
  HInst->setOperandDDRef(Mask, 3);

  return HInst;
}

HLInst *HLNodeUtils::createInsertElementInst(RegDDRef *OpRef1,
                                             RegDDRef *ElDDRef, unsigned Idx,
                                             const Twine &Name,
                                             RegDDRef *LvalRef /* = nullptr*/) {

  assert(OpRef1->getDestType()->isVectorTy() &&
         ElDDRef->getDestType() == OpRef1->getDestType()->getScalarType() &&
         "Illegal operand types for insertelement");

  auto UndefVal = UndefValue::get(OpRef1->getDestType());
  auto ElVal = UndefValue::get(ElDDRef->getDestType());

  Value *InstVal =
      DummyIRBuilder->CreateInsertElement(UndefVal, ElVal, Idx, Name);
  Instruction *Inst = cast<Instruction>(InstVal);
  assert((!LvalRef || LvalRef->getDestType() == Inst->getType()) &&
         "Incompatible type of LvalRef");

  HLInst *HInst = createLvalHLInst(Inst, LvalRef);

  RegDDRef *IdxDDref =
      getDDRefUtils().createConstDDRef(Inst->getOperand(2)->getType(), Idx);

  HInst->setOperandDDRef(OpRef1, 1);
  HInst->setOperandDDRef(ElDDRef, 2);
  HInst->setOperandDDRef(IdxDDref, 3);

  return HInst;
}

HLInst *HLNodeUtils::createExtractElementInst(RegDDRef *OpRef, unsigned Idx,
                                              const Twine &Name,
                                              RegDDRef *LvalRef) {

  assert(OpRef->getDestType()->isVectorTy() &&
         "Illegal operand types for extractelement");

  auto UndefVal = UndefValue::get(OpRef->getDestType());
  Value *InstVal = DummyIRBuilder->CreateExtractElement(UndefVal, Idx, Name);
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

HLInst *HLNodeUtils::createNot(RegDDRef *OpRef1, const Twine &Name,
                               RegDDRef *LvalRef) {
  auto RefType = OpRef1->getDestType();
  auto OneRef = OpRef1->getDDRefUtils().createConstDDRef(RefType, -1);
  return createBinaryHLInstImpl(Instruction::Xor, OpRef1, OneRef, Name, LvalRef,
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
    assert((LvalRef->getDestType()->isIntOrIntVectorTy() &&
            LvalRef->getDestType()->getScalarSizeInBits() == 1) &&
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

static PredicateTy getMinMaxPredKind(RegDDRef *Ref, bool IsSigned,
                                     bool IsFPOrdered) {
  PredicateTy PredKind;

  if (Ref->getDestType()->isFloatingPointTy()) {
    PredKind = IsFPOrdered ? PredicateTy::FCMP_OLE : PredicateTy::FCMP_ULE;
  } else {
    PredKind = IsSigned ? PredicateTy::ICMP_SLE : PredicateTy::ICMP_ULE;
  }

  return PredKind;
}

HLInst *HLNodeUtils::createMin(RegDDRef *OpRef1, RegDDRef *OpRef2,
                               RegDDRef *LvalRef, bool IsSigned,
                               bool IsFPOrdered, FastMathFlags FMF,
                               const Twine &Name) {
  PredicateTy PredKind = getMinMaxPredKind(OpRef1, IsSigned, IsFPOrdered);

  HLPredicate Pred(PredKind, FMF);

  return createSelect(Pred, OpRef1, OpRef2, OpRef1->clone(), OpRef2->clone(),
                      Name, LvalRef);
}

HLInst *HLNodeUtils::createMax(RegDDRef *OpRef1, RegDDRef *OpRef2,
                               RegDDRef *LvalRef, bool IsSigned,
                               bool IsFPOrdered, FastMathFlags FMF,
                               const Twine &Name) {
  PredicateTy PredKind = getMinMaxPredKind(OpRef1, IsSigned, IsFPOrdered);

  HLPredicate Pred(PredKind, FMF);

  return createSelect(Pred, OpRef1, OpRef2, OpRef2->clone(), OpRef1->clone(),
                      Name, LvalRef);
}

std::pair<HLInst *, CallInst *> HLNodeUtils::createCallImpl(
    FunctionCallee Func, ArrayRef<RegDDRef *> CallArgs,
    const Twine &Name, RegDDRef *LvalRef, ArrayRef<OperandBundleDef> Bundle,
    ArrayRef<RegDDRef *> BundelOps) {
  bool HasReturn = !Func.getFunctionType()->getReturnType()->isVoidTy();
  unsigned NumArgs = CallArgs.size();
  HLInst *HInst;
  SmallVector<Value *, 8> Args;

  for (unsigned I = 0; I < NumArgs; I++) {
    MetadataAsValue *MVal = nullptr;
    int64_t IVal = 0;

    Value *Val = nullptr;
    RegDDRef *Arg = CallArgs[I];
    if (Arg->isMetadata(&MVal))
      Val = MVal;
    else if (Arg->isIntConstant(&IVal))
      Val = cast<Value>(ConstantInt::getSigned(Arg->getSrcType(), IVal));

    Args.push_back(Val ? Val : UndefValue::get(CallArgs[I]->getDestType()));
  }
  auto InstVal = DummyIRBuilder->CreateCall(
      Func, Args, Bundle,
      HasReturn ? (Name.isTriviallyEmpty() ? "dummy" : Name) : "");
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

  // Set bundle operands as DDRef operands.
  unsigned NumOps = InstVal->getNumOperands() - 1;
  for (unsigned I = NumArgs, J = 0; I < NumOps; I++) {
    HInst->setOperandDDRef(BundelOps[J++], I + ArgOffset);
  }

  return std::make_pair(HInst, InstVal);
}

HLInst *HLNodeUtils::createCall(FunctionCallee Func,
                                ArrayRef<RegDDRef *> CallArgs,
                                const Twine &Name, RegDDRef *LvalRef,
                                ArrayRef<OperandBundleDef> Bundle,
                                ArrayRef<RegDDRef *> BundelOps) {
  return createCallImpl(Func, CallArgs, Name, LvalRef, Bundle, BundelOps).first;
}

HLInst *HLNodeUtils::createStacksave(const DebugLoc &DLoc) {

  Function *StacksaveFunc =
      Intrinsic::getDeclaration(&getModule(), Intrinsic::stacksave);

  SmallVector<RegDDRef *, 1> Ops;
  CallInst *Call;
  HLInst *HInst;
  std::tie(HInst, Call) = createCallImpl(StacksaveFunc, Ops);

  Call->setDebugLoc(DLoc);

  return HInst;
}

HLInst *HLNodeUtils::createStackrestore(RegDDRef *AddrArg) {
  Function *StackrestoreFunc =
      Intrinsic::getDeclaration(&getModule(), Intrinsic::stackrestore);

  SmallVector<RegDDRef *, 1> Ops = {AddrArg};
  CallInst *Call;
  HLInst *HInst;
  std::tie(HInst, Call) = createCallImpl(StackrestoreFunc, Ops);

  Call->setDebugLoc(AddrArg->getDebugLoc());

  return HInst;
}

HLInst *HLNodeUtils::createPrefetch(RegDDRef *AddressRef, RegDDRef *RW,
                                    RegDDRef *Locality, RegDDRef *CacheTy) {

  Function *PrefetchFunc = Intrinsic::getDeclaration(
      &getModule(), Intrinsic::prefetch, AddressRef->getDestType());

  SmallVector<RegDDRef *, 4> Ops = {AddressRef, RW, Locality, CacheTy};

  CallInst *Call;
  HLInst *HInst;
  std::tie(HInst, Call) = createCallImpl(PrefetchFunc, Ops);

  Call->setDebugLoc(AddressRef->getDebugLoc());

  return HInst;
}

HLInst *HLNodeUtils::createMemcpy(RegDDRef *StoreRef, RegDDRef *LoadRef,
                                  RegDDRef *Size) {
  RegDDRef *IsVolatile = getDDRefUtils().createConstDDRef(
      Type::getInt1Ty(getContext()),
      LoadRef->isVolatile() || StoreRef->isVolatile());

  Type *Tys[] = {StoreRef->getDestType(), LoadRef->getDestType(),
                 Size->getDestType()};

  Function *MemcpyFunc =
      Intrinsic::getDeclaration(&getModule(), Intrinsic::memcpy, Tys);

  SmallVector<RegDDRef *, 5> Ops = {StoreRef, LoadRef, Size, IsVolatile};

  CallInst *Call;
  HLInst *HInst;
  std::tie(HInst, Call) = createCallImpl(MemcpyFunc, Ops);

  MemCpyInst *MemCpyCall = cast<MemCpyInst>(Call);
  MemCpyCall->setSourceAlignment(LoadRef->getAlignment());
  MemCpyCall->setDestAlignment(StoreRef->getAlignment());

  Call->setDebugLoc(StoreRef->getDebugLoc());

  return HInst;
}

HLInst *HLNodeUtils::createMemset(RegDDRef *StoreRef, RegDDRef *Value,
                                  RegDDRef *Size) {
  RegDDRef *IsVolatile = getDDRefUtils().createConstDDRef(
      Type::getInt1Ty(getContext()), StoreRef->isVolatile());

  Type *Tys[] = {StoreRef->getDestType(), Size->getDestType()};
  Function *MemsetFunc =
      Intrinsic::getDeclaration(&getModule(), Intrinsic::memset, Tys);

  SmallVector<RegDDRef *, 5> Ops = {StoreRef, Value, Size, IsVolatile};

  CallInst *Call;
  HLInst *HInst;
  std::tie(HInst, Call) = createCallImpl(MemsetFunc, Ops);

  MemSetInst *MemSetCall = cast<MemSetInst>(Call);
  MemSetCall->setDestAlignment(StoreRef->getAlignment());

  Call->setDebugLoc(StoreRef->getDebugLoc());

  return HInst;
}

HLInst *HLNodeUtils::createFPMinMaxVectorReduce(RegDDRef *VecRef,
                                                Intrinsic::ID VecReduceIntrin,
                                                bool NoNaN, RegDDRef *LvalRef) {
  Type *Tys[] = {VecRef->getDestType()};

  Function *VecReduceFunc =
      Intrinsic::getDeclaration(&getModule(), VecReduceIntrin, Tys);

  SmallVector<RegDDRef *, 1> Ops = {VecRef};

  CallInst *Call;
  HLInst *HInst;
  std::tie(HInst, Call) =
      createCallImpl(VecReduceFunc, Ops, "" /*Name*/, LvalRef);

  if (NoNaN) {
    FastMathFlags FMF;
    FMF.setNoNaNs();
    Call->setFastMathFlags(FMF);
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
      updateLoopInfo(Loop);
    }
  }

  /// Catch-all visit functions
  void visit(HLNode *Node) {}
  void postVisit(HLNode *Node) {}

  bool isDone() const {
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

void HLNodeUtils::insertAsFirstChildren(HLIf *If, HLContainerTy *NodeContainer,
                                        bool IsThenChild) {
  assert(If && "If is null!");
  assert(NodeContainer && "NodeContainer is null!");

  insertImpl(If, IsThenChild ? If->then_begin() : If->else_begin(),
             NodeContainer, NodeContainer->begin(), NodeContainer->end(),
             !IsThenChild);
}

void HLNodeUtils::insertAsLastChild(HLIf *If, HLNode *Node, bool IsThenChild) {
  assert(If && "If is null!");
  assert(Node && "Node is null!");

  insertImpl(If, IsThenChild ? If->then_end() : If->else_end(), nullptr,
             Node->getIterator(), Node->getIterator(), !IsThenChild);
}

void HLNodeUtils::insertAsLastChildren(HLIf *If, HLContainerTy *NodeContainer,
                                       bool IsThenChild) {
  assert(If && "If is null!");
  assert(NodeContainer && "NodeContainer is null!");

  insertImpl(If, IsThenChild ? If->then_end() : If->else_end(), NodeContainer,
             NodeContainer->begin(), NodeContainer->end(), !IsThenChild);
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

void HLNodeUtils::insertAsFirstPreheaderNodes(HLLoop *Loop,
                                              HLContainerTy *NodeContainer) {
  assert(NodeContainer && "NodeContainer is null!");
  insertAsPreheaderPostexitImpl(Loop, NodeContainer, NodeContainer->begin(),
                                NodeContainer->end(), true, true);
}

void HLNodeUtils::insertAsLastPreheaderNode(HLLoop *Loop, HLNode *Node) {
  assert(Node && "Node is null!");
  insertAsPreheaderPostexitImpl(Loop, nullptr, Node->getIterator(),
                                Node->getIterator(), true, false);
}

void HLNodeUtils::insertAsLastPreheaderNodes(HLLoop *Loop,
                                             HLContainerTy *NodeContainer) {
  assert(NodeContainer && "NodeContainer is null!");
  insertAsPreheaderPostexitImpl(Loop, NodeContainer, NodeContainer->begin(),
                                NodeContainer->end(), true, false);
}

void HLNodeUtils::insertAsFirstPostexitNode(HLLoop *Loop, HLNode *Node) {
  assert(Node && "Node is null!");
  insertAsPreheaderPostexitImpl(Loop, nullptr, Node->getIterator(),
                                Node->getIterator(), false, true);
}

void HLNodeUtils::insertAsFirstPostexitNodes(HLLoop *Loop,
                                             HLContainerTy *NodeContainer) {
  assert(NodeContainer && "NodeContainer is null!");
  insertAsPreheaderPostexitImpl(Loop, NodeContainer, NodeContainer->begin(),
                                NodeContainer->end(), false, true);
}

void HLNodeUtils::insertAsLastPostexitNode(HLLoop *Loop, HLNode *Node) {
  assert(Node && "Node is null!");
  insertAsPreheaderPostexitImpl(Loop, nullptr, Node->getIterator(),
                                Node->getIterator(), false, false);
}

void HLNodeUtils::insertAsLastPostexitNodes(HLLoop *Loop,
                                            HLContainerTy *NodeContainer) {
  assert(NodeContainer && "NodeContainer is null!");
  insertAsPreheaderPostexitImpl(Loop, NodeContainer, NodeContainer->begin(),
                                NodeContainer->end(), false, false);
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
    auto &HIRF = Node->getHLNodeUtils().getHIRFramework();

    auto FirstOrLastRegIter =
        Prev ? HIRF.hir_begin() : std::prev(HIRF.hir_end());
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

  bool isDone() const { return Stop; }

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

const HLNode *
HLNodeUtils::getImmediateChildContainingNode(const HLNode *ParentNode,
                                             const HLNode *Node) {
  assert(contains(ParentNode, Node) && "Node doesn't belong to a ParentNode");

  HLNode *Parent = Node->getParent();
  while (Parent != ParentNode) {
    Node = Parent;
    Parent = Node->getParent();
  }

  return Node;
}

HLNode *HLNodeUtils::getImmediateChildContainingNode(HLNode *ParentNode,
                                                     HLNode *Node) {
  return const_cast<HLNode *>(
      getImmediateChildContainingNode(static_cast<const HLNode *>(ParentNode),
                                      static_cast<const HLNode *>(Node)));
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

  bool isDone() const { return (IsDone || !isStructured()); }
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
      if (!TLS.hasForwardGotos()) {
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

    LastNode = Node ? Node : getLastLexicalChild(Parent);

  } else {
    FirstNode = Node ? Node : getFirstLexicalChild(Parent);

    LastNode = isa<HLLoop>(Parent) ? getLastLexicalChild(Parent)
                                   : getLastLexicalChild(Parent, Node);
  }

  assert((FirstNode && LastNode) && "Could not find first/last lexical child!");

  if (Node && (FirstNode == LastNode)) {
    // Both are set to 'Node' which we don't need to check.
    return true;
  }

  StructuredFlowChecker SFC(PostDomination, TargetNode,
                            FirstNode->getParentLoop(), HLS);

  // Don't need to recurse into loops.
  // TODO: We probably need to enhance it to recurse into multi-exit loops.
  if (UpwardTraversal) {
    // We want to traverse the range [FirstNode, LastNode) in the backward
    // direction. If Node is same as LastNode we skip it.
    auto EndIt = (Node == LastNode) ? LastNode->getIterator()
                                    : ++(LastNode->getIterator());

    visitRange<true, false, false>(SFC, FirstNode->getIterator(), EndIt);

  } else {
    // We want to traverse the range (FirstNode, LastNode] in the forward
    // direction. If Node is same as FirstNode we skip it.
    auto BeginIt = (Node != FirstNode) ? FirstNode->getIterator()
                                       : ++(FirstNode->getIterator());

    visitRange<true, false, true>(SFC, BeginIt, ++(LastNode->getIterator()));
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

  // Try to move up the parent chain by crossing loops without ztt.
  while (Parent) {

    auto *Loop = dyn_cast<HLLoop>(Parent);

    if (!Loop || !Loop->isDo() || Loop->hasZtt()) {
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
    if (isInTopSortNumMaxRange(Node2, FirstNode, LastNode)) {
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

  // For post-domination, Node2 itself needs to be checked in case it is a
  // parent node type like if/switch as they can contain gotos. In the example
  // below, Node1 does not post-dominate Node2 due to the goto.
  //
  // if (t > 0) { // Node2
  //   goto exit;
  // }
  // t2 = 0;      // Node1
  bool CheckNode2 = (PostDomination && Node2->isParentNode());

  // If Node2 itself is parent of Node1 then Node1 cannot post-dominate it if
  // Node2 is an If or Switch but it can post-dominate loop.
  if (CheckNode2 && (Node2 == Parent1) &&
      (isa<HLIf>(Node2) || isa<HLSwitch>(Node2))) {
    return nullptr;
  }

  const HLNode *CommonParent = CheckNode2 ? Node2 : Node2->getParent();
  *LastParent2 = CheckNode2 ? nullptr : Node2;

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

  auto *HLS = Node1->getHLNodeUtils()
                  .getHIRFramework()
                  .getHIRAnalysisProvider()
                  .get<HIRLoopStatistics>();

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
        isInTopSortNumMaxRange(LastParent1, IfParent->getFirstThenChild(),
                               IfParent->getLastThenChild());
    bool Node2Found =
        isInTopSortNumMaxRange(LastParent2, IfParent->getFirstThenChild(),
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

      bool Node1Found = isInTopSortNumMaxRange(
          LastParent1, SwitchParent->getFirstCaseChild(I),
          SwitchParent->getLastCaseChild(I));
      bool Node2Found = isInTopSortNumMaxRange(
          LastParent2, SwitchParent->getFirstCaseChild(I),
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

  // Use top sort num, if available.
  if (unsigned TSNum = Node->getTopSortNum()) {
    assert((isa<HLRegion>(Node) || Node->isAttached()) &&
           "It is illegal to call top sort number "
           "dependent utility on disconnected node!");
    return (TSNum >= Parent->getMinTopSortNum() &&
            TSNum <= Parent->getMaxTopSortNum());
  }

  while (Node) {
    if (Parent == Node) {
      return true;
    }
    Node = Node->getParent();
  }

  return false;
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

  LLVM_DEBUG(dbgs() << "\tin getMaxMinBlobValue: input args " << BlobIdx
                    << BoundCE->getSingleBlobCoeff() << " "
                    << BoundCE->getSingleBlobIndex() << " "
                    << BoundCE->getConstant() << "\n");

  auto BoundCoeff = BoundCE->getSingleBlobCoeff();
  auto BoundBlobIdx = BoundCE->getSingleBlobIndex();
  auto &BU = BoundCE->getBlobUtils();

  BlobTy Blob = BU.getBlob(BlobIdx);
  // Strip sign extend cast from Blob
  while (BlobUtils::isSignExtendBlob(Blob, &Blob))
    ;

  BlobTy BoundBlob = BU.getBlob(BoundBlobIdx);

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

class LiveInBlobChecker {
private:
  HLRegion *Reg;
  BlobUtils &BU;
  bool IsLiveIn;

public:
  LiveInBlobChecker(HLRegion *Reg, BlobUtils &BU)
      : Reg(Reg), BU(BU), IsLiveIn(true) {}

  bool follow(const SCEV *SC) {
    if (BU.isTempBlob(SC)) {
      unsigned Symbase = BU.findTempBlobSymbase(SC);

      if (!Reg->isLiveIn(Symbase)) {
        IsLiveIn = false;
      }

      return false;
    }

    return !isDone();
  }

  bool isDone() const { return !IsLiveIn; }
  bool isLiveInBlob() const { return IsLiveIn; }
};

static bool isRegionLiveIn(HLRegion *Reg, BlobUtils &BU, unsigned BlobIdx) {
  LiveInBlobChecker LBC(Reg, BU);
  SCEVTraversal<LiveInBlobChecker> Checker(LBC);
  Checker.visitAll(BU.getBlob(BlobIdx));

  return LBC.isLiveInBlob();
}

bool HLNodeUtils::getMinBlobValue(unsigned BlobIdx, const HLNode *ParentNode,
                                  int64_t &Val) {
  VALType ValType = getMinMaxBlobValue(BlobIdx, ParentNode, Val);

  if (isMinValue(ValType)) {
    return true;
  }

  auto &BU = ParentNode->getBlobUtils();

  if (isRegionLiveIn(ParentNode->getParentRegion(), BU, BlobIdx)) {
    return BU.getMinBlobValue(BlobIdx, Val);
  }

  if (BU.isUMaxBlob(BlobIdx) || BU.isUMinBlob(BlobIdx)) {
    Val = 0;
    return true;
  }

  return false;
}

bool HLNodeUtils::getMaxBlobValue(unsigned BlobIdx, const HLNode *ParentNode,
                                  int64_t &Val) {
  VALType ValType = getMinMaxBlobValue(BlobIdx, ParentNode, Val);

  if (isMaxValue(ValType)) {
    return true;
  }

  auto &BU = ParentNode->getBlobUtils();
  if (isRegionLiveIn(ParentNode->getParentRegion(), BU, BlobIdx) &&
      BU.getMaxBlobValue(BlobIdx, Val)) {
    return true;
  }

  return false;
}

bool HLNodeUtils::isKnownNonPositive(unsigned BlobIdx, const HLNode *ParentNode,
                                     int64_t &MaxVal) {
  if (!getMaxBlobValue(BlobIdx, ParentNode, MaxVal)) {
    return false;
  }

  return MaxVal <= 0;
}

bool HLNodeUtils::isKnownNegative(unsigned BlobIdx, const HLNode *ParentNode,
                                  int64_t &MaxVal) {
  if (!getMaxBlobValue(BlobIdx, ParentNode, MaxVal)) {
    return false;
  }

  return MaxVal < 0;
}

bool HLNodeUtils::isKnownPositive(unsigned BlobIdx, const HLNode *ParentNode,
                                  int64_t &MinVal) {
  if (!getMinBlobValue(BlobIdx, ParentNode, MinVal)) {
    return false;
  }

  return MinVal > 0;
}

bool HLNodeUtils::isKnownNonNegative(unsigned BlobIdx, const HLNode *ParentNode,
                                     int64_t &MinVal) {
  if (!getMinBlobValue(BlobIdx, ParentNode, MinVal)) {
    return false;
  }

  return MinVal >= 0;
}

bool HLNodeUtils::isKnownPositiveOrNegative(unsigned BlobIdx,
                                            const HLNode *ParentNode,
                                            int64_t &MinMaxVal) {
  return isKnownPositive(BlobIdx, ParentNode, MinMaxVal) ||
         isKnownNegative(BlobIdx, ParentNode, MinMaxVal);
}

bool HLNodeUtils::isKnownNonZero(unsigned BlobIdx, const HLNode *ParentNode) {
  int64_t MinMaxVal;
  return isKnownPositiveOrNegative(BlobIdx, ParentNode, MinMaxVal);
}

#if __GNUC__ >= 7
// The switch ladders below uses implicit fallthrough for compactness.
// Please note the ordering when adding cases.
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#endif

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
  std::unique_ptr<CanonExpr> ConditionCE(
      Lhs->getCanonExprUtils().cloneAndSubtract(
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

static bool getMinMaxBlobVal(unsigned BlobIdx, int64_t Coeff,
                             const HLNode *ParentNode, bool IsMin,
                             int64_t &BlobVal) {

  bool GetMinBlobVal = (Coeff > 0 && IsMin) || (Coeff < 0 && !IsMin);

  if (GetMinBlobVal) {
    if (!HLNodeUtils::getMinBlobValue(BlobIdx, ParentNode, BlobVal)) {
      return false;
    }
  } else if (!HLNodeUtils::getMaxBlobValue(BlobIdx, ParentNode, BlobVal)) {
    return false;
  }

  return true;
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

    if (!getMinMaxBlobVal(Index, Coeff, ParentNode, IsMin, BlobVal)) {
      return false;
    }

    MinOrMax += (Coeff * BlobVal);
  }

  if (CE->hasIV()) {
    auto ParentLoop = isa<HLLoop>(ParentNode) ? cast<HLLoop>(ParentNode)
                                              : ParentNode->getParentLoop();

    assert(ParentLoop && "Parent loop of CE containing IV not found!");

    auto *Lp = ParentLoop;
    for (unsigned Level = Lp->getNestingLevel(); Level > 0;
         --Level, Lp = Lp->getParentLoop()) {

      unsigned Index;
      int64_t Coeff, BlobVal = 1;

      CE->getIVCoeff(Level, &Index, &Coeff);

      if (!Coeff) {
        continue;
      }

      if (Index != InvalidBlobIndex &&
          !getMinMaxBlobVal(Index, Coeff, ParentNode, IsMin, BlobVal)) {
        return false;
      }

      if (!BlobVal) {
        continue;
      }

      bool HasPositiveCoeff =
          ((Coeff > 0) && (BlobVal > 0)) || ((Coeff < 0) && (BlobVal < 0));

      bool UseLowerBound =
          ((IsMin && HasPositiveCoeff) || (!IsMin && !HasPositiveCoeff));

      int64_t BoundVal;

      // We should be using min value of lower bound and max value of upper
      // bound to compute min/max.
      if (UseLowerBound) {
        if (!Lp->getLowerCanonExpr()->isIntConstant(&BoundVal)) {
          if (IsExact) {
            return false;
          }
          // The minimum value of lower bound of loops in HIR is 0.
          BoundVal = 0;
        }
      } else if (!getMinMaxValueImpl(Lp->getUpperCanonExpr(), Lp, false,
                                     IsExact, BoundVal)) {
        return false;
      }

      // Conservatively return false if bound is too big.
      if ((BoundVal < 0) || (BoundVal > UINT32_MAX)) {
        return false;
      }

      MinOrMax += (Coeff * BlobVal * BoundVal);
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

static cl::opt<bool> IgnoreWraparound("hir-ignore-wraparound", cl::init(false),
                                      cl::Hidden,
                                      cl::desc("Disables wraparound check."));

bool HLNodeUtils::mayWraparound(const CanonExpr *CE, unsigned Level,
                                const HLNode *ParentNode,
                                const bool FitsIn32Bits) {
  assert(CE->getSrcType()->isIntegerTy() &&
         "CE does not have an integer type!");

  if (IgnoreWraparound) {
    return false;
  }

  if (!CE->hasIV(Level)) {
    return false;
  }

  // Need a test case for truncation without wraparound.
  if (CE->isTrunc()) {
    return true;
  }

  // We only handle zext for now as we have real test cases for it.
  // sext is problematic because no-wrap info is easily lost before loopopt so
  // we will become too conservative. Handling sext is left as a TODO
  // until we have a real test case.
  if (!CE->isZExt()) {
    return false;
  }

  if (FitsIn32Bits && CE->getSrcType()->getScalarSizeInBits() >= 32) {
    return false;
  }

  // Check whether the CE is in the range of its src type.

  // Negative values indicate wraparound for zext.
  if (!isKnownNonNegative(CE, ParentNode)) {
    return true;
  }

  auto *IntTy = cast<IntegerType>(CE->getSrcType());
  unsigned Size = IntTy->getPrimitiveSizeInBits();

  int64_t MaxValForSrcType = APInt::getMaxValue(Size).getZExtValue();

  int64_t MaxVal;
  // Returns true only if we can prove it goes out of range.
  // Instcombine turns some sexts into zexts. Because of information loss, we
  // will become too conservative if we indicate wraparound when max value is
  // unknown.
  if (getMaxValue(CE, ParentNode, MaxVal)) {
    if (MaxVal > MaxValForSrcType) {
      return true;
    }
  } else if (Size < 32) {
    // Assuming wraparound for size less than 32 bits. Usually, the IV is 32
    // bits in source, hence there is higher likelyhood of a zext/sext to 64
    // bits. Returning the conservative answer can lead to performance
    // regressions. Can we do better?
    return true;
  }

  return false;
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

HLNodeRangeTy HLNodeUtils::getHIRRange() {
  return make_range(getHIRFramework().hir_begin(), getHIRFramework().hir_end());
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

// TODO: Consider removing AllowNearPerfect argument and
//       just use bool* IsNearPerfectLoop only instead
//       similar to the logic of its caller, IsPerfectLoopNest().
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

    } else if (!isa<HLInst>(NodeIt)) {
      return false;
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
///  An innermost loop can't be given as the first argument.
///  Default to allow Triangular loop is false with exceptions made for first
///  iteration.
///  IsNearPerfectNest is used as in-out arguement: if non-null,
///  and whether the loop nest is near perfect (as shown below) will be set
///  to the argument.
///  Note that this function returns false if the given loop nest is near
///  perfect. After all, near perfect is not perfect.
///
///     A near-perfect loop is this form:
///     (stmts found before or after only around the *innermost* loop)
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
///     s1, s2 are siblings or preheader/postexit. of the innermost loop.
///     This form may further be transformed to perfect loopnest by clients.
///     Will not make it too general because of compile time considerations
///     In addition, if this bool is on, it will indicate if Near Perfect Lp is
///     found.
bool HLNodeUtils::isPerfectLoopNest(const HLLoop *Loop,
                                    const HLLoop **InnermostLoop,
                                    bool AllowTriangularLoop,
                                    bool *IsNearPerfectNest) {

  assert(Loop && "Loop is null!");
  assert(!Loop->isInnermost() && "Innermost loop not expected!");
  if (!Loop->isDo()) {
    return false;
  }

  // Note the innermost loop never runs as Lp the following loop-body.
  // It can only be InnerLp.
  const bool &AllowNearPerfect = static_cast<bool>(IsNearPerfectNest);
  const HLLoop *Lp = Loop;
  bool IsNearPerfect = false;
  do {
    const HLLoop *InnerLp;
    if (!hasPerfectLoopProperties(Lp, &InnerLp, AllowNearPerfect,
                                  &IsNearPerfect)) {
      // We don't allow near-perfection to non-innermost loops
      // hasPerfectLoopProperties does not allow NearPerfect for
      // non-innermost loop anyway.
      return false;
    }

    if (!InnerLp->isDo()) {
      return false;
    }

    if (InnerLp->hasPreheader() || InnerLp->hasPostexit()) {
      // We don't allow near-perfection to non-innermost loops
      // If preheader and postexit exist for a non-innermost loop,
      // we don't need to look further.
      if (InnerLp->isInnermost() && AllowNearPerfect) {
        IsNearPerfect = true;
      } else {
        return false;
      }
    }

    // TODO: check if IV belongs to any loop within the perfect loopnest.
    if (!AllowTriangularLoop && InnerLp->isTriangularLoop()) {
      return false;
    }

    Lp = InnerLp;

  } while (!Lp->isInnermost());

  if (InnermostLoop) {
    *InnermostLoop = Lp;
  }
  if (IsNearPerfectNest) {
    *IsNearPerfectNest = IsNearPerfect;
  }

  // NearPerfect is not perfect.
  return IsNearPerfect ? false : true;
}

class NonUnitStrideMemRefs final : public HLNodeVisitorBase {
private:
  bool HasNonLinearLvalRef;
  unsigned LoopLevel;

public:
  bool HasNonUnitStride;
  NonUnitStrideMemRefs(const HLLoop *Loop)
      : HasNonLinearLvalRef(false), LoopLevel(Loop->getNestingLevel()),
        HasNonUnitStride(false) {}
  void visit(const HLNode *Node) {}
  void visit(const HLDDNode *Node);
  void postVisit(const HLNode *Node) {}
  void postVisit(const HLDDNode *Node) {}
  bool isDone() const { return HasNonLinearLvalRef; }
};

///  Make a quick pass here to save compile time:
///  If all memory references are in unit stride, there is no need to
///  proceed further
void NonUnitStrideMemRefs::visit(const HLDDNode *Node) {

  for (auto I = Node->ddref_begin(), E1 = Node->ddref_end(); I != E1; ++I) {
    if ((*I)->isTerminalRef()) {
      continue;
    }

    const RegDDRef *Ref = *I;
    const CanonExpr *FirstCE = nullptr;
    bool IsLvalMemRef = Ref->isLval() && Ref->isMemRef();

    for (unsigned I = 1, NumDims = Ref->getNumDimensions(); I <= NumDims; ++I) {
      const CanonExpr *IndexCE = Ref->getDimensionIndex(I);
      // Give up on Non-linear memref because it prevents Linear Trans
      if (IsLvalMemRef) {
        if (IndexCE->isNonLinear() ||
            Ref->getDimensionLower(I)->isNonLinear() ||
            Ref->getDimensionStride(I)->isNonLinear()) {
          HasNonLinearLvalRef = true;
          return;
        }
      }

      if (!FirstCE) {
        FirstCE = IndexCE;
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

const HLLoop *HLNodeUtils::getLowestCommonAncestorLoop(const HLLoop *Lp1,
                                                       const HLLoop *Lp2) {
  if (!Lp1 || !Lp2) {
    return nullptr;
  }

  assert((Lp1->getParentRegion() == Lp2->getParentRegion()) &&
         "Lp1 and Lp2 are not in the same region!");

  // Trivial case.
  if (Lp1 == Lp2) {
    return Lp1;
  }

  // Assuming that most loops in a region belong to the same loopnest and hence
  // will have a common parent, we follow this logic-
  //
  // 1) Move up the chain for the loop with a greater nesting level.
  // 2) Once the levels are equal, we move up the chain for both loops
  // simultaneously until we discover the common parent.

  unsigned Level1 = Lp1->getNestingLevel();
  unsigned Level2 = Lp2->getNestingLevel();

  while (Level1 > Level2) {
    Lp1 = Lp1->getParentLoop();
    --Level1;
  }

  while (Level2 > Level1) {
    Lp2 = Lp2->getParentLoop();
    --Level2;
  }

  assert(Level1 == Level2 && "Nesting level mismtach!");

  // Both loops have the same nesting level, so move up simultaneously.
  while (Lp1) {
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

const HLNode *
HLNodeUtils::getLexicalLowestCommonAncestorParent(const HLNode *Node1,
                                                  const HLNode *Node2) {
  assert(Node1 && Node2 && "Node1 or Node2 is null!");
  assert((Node1->getParentRegion() == Node2->getParentRegion()) &&
         "Node1 and Node2 are not in the same region!");

  // Represent node by its parent loop if it is in the loop preheader/postexit
  // to avoid checking for edges cases later on in the function.
  if (auto Inst = dyn_cast<HLInst>(Node1)) {
    if (Inst->isInPreheaderOrPostexit()) {
      Node1 = Node1->getParent();
    }
  }

  if (auto Inst = dyn_cast<HLInst>(Node2)) {
    if (Inst->isInPreheaderOrPostexit()) {
      Node2 = Node2->getParent();
    }
  }

  const HLNode *Parent = nullptr;
  unsigned TopSortNum1 = Node1->getTopSortNum();
  unsigned TopSortNum2 = Node2->getTopSortNum();

  unsigned LaterNodeTopSortNum;

  // Set starting parent using the node which appears earlier in HIR.
  if (TopSortNum1 < TopSortNum2) {
    Parent = Node1->getParent();
    LaterNodeTopSortNum = TopSortNum2;
  } else {
    Parent = Node2->getParent();
    LaterNodeTopSortNum = TopSortNum1;
  }

  // Move up the parent chain until we find a parent wich also contains the node
  // which appears later in HIR.
  while (Parent->getMaxTopSortNum() < LaterNodeTopSortNum) {
    Parent = Parent->getParent();
  }

  return Parent;
}

HLNode *HLNodeUtils::getLexicalLowestCommonAncestorParent(HLNode *Node1,
                                                          HLNode *Node2) {
  return const_cast<HLNode *>(getLexicalLowestCommonAncestorParent(
      static_cast<const HLNode *>(Node1), static_cast<const HLNode *>(Node2)));
}

bool HLNodeUtils::areEqualConditions(const HLIf *NodeA, const HLIf *NodeB) {
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

bool HLNodeUtils::areEqualConditions(const HLSwitch *NodeA,
                                     const HLSwitch *NodeB) {
  return DDRefUtils::areEqual(NodeA->getConditionDDRef(),
                              NodeB->getConditionDDRef());
}

HLNodeRangeTy HLNodeUtils::replaceNodeWithBody(HLIf *If, bool ThenBody) {

  auto NodeRange = ThenBody ? std::make_pair(If->then_begin(), If->then_end())
                            : std::make_pair(If->else_begin(), If->else_end());
  auto LastNode = std::prev(NodeRange.second);

  HLNodeUtils::moveAfter(If, NodeRange.first, NodeRange.second);
  HLNodeUtils::remove(If);

  return make_range(NodeRange.first, std::next(LastNode));
}

HLNodeRangeTy HLNodeUtils::replaceNodeWithBody(HLSwitch *Switch,
                                               unsigned CaseNum) {

  auto NodeRange = (CaseNum == 0)
                       ? std::make_pair(Switch->default_case_child_begin(),
                                        Switch->default_case_child_end())
                       : std::make_pair(Switch->case_child_begin(CaseNum),
                                        Switch->case_child_end(CaseNum));
  auto LastNode = std::prev(NodeRange.second);

  HLNodeUtils::moveAfter(Switch, NodeRange.first, NodeRange.second);
  HLNodeUtils::remove(Switch);

  return make_range(NodeRange.first, std::next(LastNode));
}

namespace {

STATISTIC(InvalidatedRegions, "Number of regions invalidated by utility");
STATISTIC(InvalidatedLoops, "Number of loops invalidated by utility");
STATISTIC(LoopsRemoved, "Number of empty Loops removed by utility");
STATISTIC(IfsRemoved, "Number of empty Ifs removed by utility");
STATISTIC(SwitchesRemoved, "Number of empty Switches removed by utility");
STATISTIC(RedundantLoops, "Number of redundant loops removed by utility");
STATISTIC(RedundantPredicates,
          "Number of redundant predicates removed by utility");
STATISTIC(RedundantInstructions,
          "Number of redundant instructions removed by utility");
STATISTIC(RedundantEarlyExitLoops,
          "Number of loops with unconditional exit removed by the utility");

static cl::opt<bool> DisableAggressiveRedundantLoopRemoval(
    "disable-hir-aggressive-redundant-loop-removal", cl::init(false),
    cl::Hidden, cl::desc("Disable aggressive redundant loop removal."));

class EmptyNodeRemoverVisitorImpl : public HLNodeVisitorBase {
protected:
  SmallPtrSet<HLNode *, 32> NodesToInvalidate;

  // The flag is used to determine the need of removing parent nodes and to
  // return a boolean result to the caller of the utility.
  bool Changed = false;

  void notifyWillRemoveNode(HLNode *Node) {
    // Node will be removed, no reason to invalidate.
    NodesToInvalidate.erase(Node);

    // Invalidate Node's parent.
    if (HLLoop *ParentLoop = Node->getParentLoop()) {
      NodesToInvalidate.insert(ParentLoop);
    } else if (HLRegion *Region = Node->getParentRegion()) {
      NodesToInvalidate.insert(Region);
    }
  }

  EmptyNodeRemoverVisitorImpl() {}

  ~EmptyNodeRemoverVisitorImpl() {
    for (HLNode *Node : NodesToInvalidate) {
      if (HLLoop *Loop = dyn_cast<HLLoop>(Node)) {
        HIRInvalidationUtils::invalidateBody(Loop);
        InvalidatedLoops++;
      } else if (HLRegion *Region = dyn_cast<HLRegion>(Node)) {
        HIRInvalidationUtils::invalidateNonLoopRegion(Region);
        InvalidatedRegions++;
      } else {
        llvm_unreachable("Unexpected HLNode kind to invalidate");
      }
    }
  }

public:
  void postVisit(HLLoop *Loop) {
    if (!Loop->hasChildren()) {
      notifyWillRemoveNode(Loop);

      Loop->extractPreheaderAndPostexit();

      LoopOptReportBuilder &LORBuilder =
          Loop->getHLNodeUtils().getHIRFramework().getLORBuilder();

      LORBuilder(*Loop).preserveLostLoopOptReport();

      HLNodeUtils::remove(Loop);
      Changed = true;

      LoopsRemoved++;
    }
  }

  void postVisit(HLIf *If) {
    bool HasThenChildren = If->hasThenChildren();
    bool HasElseChildren = If->hasElseChildren();

    if (!HasThenChildren && !HasElseChildren) {
      notifyWillRemoveNode(If);

      HLNodeUtils::remove(If);
      Changed = true;

      IfsRemoved++;
    } else if (!HasThenChildren && If->getNumPredicates() == 1) {
      HLNodeUtils::moveAsFirstThenChildren(If, If->else_begin(),
                                           If->else_end());
      If->invertPredicate(If->pred_begin());
    }
  }

  void postVisit(HLSwitch *Switch) {
    if (Switch->hasDefaultCaseChildren()) {
      return;
    }

    for (unsigned I = 1, E = Switch->getNumCases(); I <= E; ++I) {
      if (Switch->hasCaseChildren(I)) {
        return;
      }
    }

    notifyWillRemoveNode(Switch);

    HLNodeUtils::remove(Switch);
    Changed = true;

    SwitchesRemoved++;
  }

  void visit(HLNode *) {}
  void postVisit(HLNode *) {}

  void removeEmptyNode(HLNode *Node) {
    if (HLIf *If = dyn_cast<HLIf>(Node)) {
      postVisit(If);
    } else if (HLLoop *Loop = dyn_cast<HLLoop>(Node)) {
      postVisit(Loop);
    } else if (HLSwitch *Switch = dyn_cast<HLSwitch>(Node)) {
      postVisit(Switch);
    }
  }

  bool isChanged() const { return Changed; }

  // If Parent is nullptr, empty nodes will be removed until region.
  void removeEmptyNodesUntilParent(HLNode *Node, HLNode *Parent = nullptr) {
    if (!Node || isa<HLRegion>(Node) || Node == Parent) {
      return;
    }

    if (!Parent) {
      Parent = Node->getParentRegion();
    } else {
      assert(HLNodeUtils::contains(Parent, Node) &&
             "Node should be a child of Parent");
    }

    bool SavedChanged = Changed;

    while (Node != Parent && Changed) {
      HLNode *NextNode = Node->getParent();

      Changed = false;
      removeEmptyNode(Node);

      Node = NextNode;
    }

    Changed = Changed || SavedChanged;
  }
};

struct EmptyNodeRemoverVisitor final : public EmptyNodeRemoverVisitorImpl {};

class RedundantNodeRemoverVisitor final : public EmptyNodeRemoverVisitorImpl {
  // Stack of found loop side effects for loops currently in process.
  SmallVector<std::pair<HLLoop *, bool>, MaxLoopNestLevel> LoopSideEffects;

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

  // The goto candidates to be removed.
  SmallPtrSet<HLGoto *, 4> GotosToRemove;

  // Flag that enables HLLabel removal logic.
  HLNode *LabelSafeContainer;

  // Candidate labels to be removed.
  SmallPtrSet<HLLabel *, 8> Labels;

  // Number of jumps for each label.
  SmallDenseMap<HLLabel *, unsigned, 8> LabelJumps;

  // Indicates that the current node is in the junction point of multiple
  // blocks.
  bool IsJoinNode;

private:
  /// Returns true if the goto exits the loop.
  bool isLoopExitGoto(const HLLoop *Loop, const HLGoto *Goto) const {
    assert(Goto->getTopSortNum() > Loop->getMinTopSortNum() &&
           Goto->getTopSortNum() <= Loop->getMaxTopSortNum() &&
           "HLGoto doesn't belong to the loop");
    return Goto->isExternal() ||
           (Goto->getTargetLabel()->getTopSortNum() > Loop->getMaxTopSortNum());
  }

  // Tries to find lexical control flow successor of internal \p Goto by looking
  // for next node in the parent chain.
  static HLNode *getInternalGotoNodeSingleSuccessor(HLGoto *Goto) {
    assert(!Goto->isExternal() && "Expected internal HLGoto");
    HLNode *Node = Goto;
    HLNode *Parent, *Next;
    while (!(Next = Node->getNextNode()) && (Parent = Node->getParent())) {
      // Do not go over loops as they have back edges.
      if (isa<HLLoop>(Parent)) {
        break;
      }

      Node = Parent;
    }
    return Next;
  };

public:
  RedundantNodeRemoverVisitor()
      : SkipNode(nullptr), LastNodeToRemove(nullptr),
        LabelSafeContainer(nullptr), IsJoinNode(false) {}

  void visit(HLRegion *Region) {
    assert(LabelSafeContainer == nullptr &&
           "LabelSafeContainer should be not defined");
    LabelSafeContainer = Region;

    EmptyNodeRemoverVisitorImpl::visit(Region);
  }

  void postVisit(HLRegion *Region) {
    assert(LabelSafeContainer && "LabelSafeContainer should be defined");
    LabelSafeContainer = nullptr;

    assert(LoopSideEffects.empty() && "LoopSideEffects is out of sync");

    EmptyNodeRemoverVisitorImpl::postVisit(Region);
  }

  void visit(HLLoop *Loop) {
    // Loop may be removed as a dead code.
    visit(static_cast<HLDDNode *>(Loop));
    if (SkipNode == Loop) {
      return;
    }

    bool ZttIsTrue = false;
    bool IsTrivialZtt = Loop->isKnownZttPredicate(&ZttIsTrue);

    if (IsTrivialZtt && !ZttIsTrue) {
      RedundantLoops++;
      notifyWillRemoveNode(Loop);

      SkipNode = Loop;

      LoopOptReportBuilder &LORBuilder =
          Loop->getHLNodeUtils().getHIRFramework().getLORBuilder();

      LORBuilder(*Loop).preserveLostLoopOptReport();

      HLNodeUtils::remove(Loop);
      Changed = true;
      return;
    }

    // Remove trivial ztt
    if (IsTrivialZtt) {
      Loop->removeZtt();
    }

    // Loop will stay attached.

    if (LabelSafeContainer == nullptr) {
      LabelSafeContainer = Loop;
    }

    // Record new loop.
    LoopSideEffects.emplace_back(Loop, Loop->hasLiveOutTemps());
  }

  void visit(HLIf *If) {
    bool IsTrue;
    if (If->isKnownPredicate(&IsTrue)) {
      Changed = true;
      RedundantPredicates++;
      notifyWillRemoveNode(If);

      auto NodeRange = HLNodeUtils::replaceNodeWithBody(If, IsTrue);

      // Go over nodes that are not going to be removed. If the predicate is
      // always TRUE the else branch will be removed.
      HLNodeUtils::visitRange(*this, NodeRange.begin(), NodeRange.end());
      SkipNode = If;
      return;
    }

    visit(static_cast<HLDDNode *>(If));
  }

  void visit(HLSwitch *Switch) {
    int64_t ConditionConstValue;

    if (Switch->getConditionDDRef()->isIntConstant(&ConditionConstValue)) {
      Changed = true;
      RedundantPredicates++;
      notifyWillRemoveNode(Switch);

      unsigned FoundCase = 0;
      for (unsigned I = 1, E = Switch->getNumCases(); I <= E; ++I) {
        if (Switch->getConstCaseValue(I) == ConditionConstValue) {
          FoundCase = I;
          break;
        }
      }

      auto NodeRange = HLNodeUtils::replaceNodeWithBody(Switch, FoundCase);
      HLNodeUtils::visitRange(*this, NodeRange.begin(), NodeRange.end());
      SkipNode = Switch;
      return;
    }

    visit(static_cast<HLDDNode *>(Switch));
  }

  void removeSiblingGotosWithTarget(HLLabel *Label) {
    SmallVector<HLGoto *, 4> FoundGotos;

    // Collect all gotos pointing to Node which are direct or indirect siblings
    // to the target label.
    std::for_each(GotosToRemove.begin(), GotosToRemove.end(),
                  [Label, &FoundGotos](HLGoto *Goto) {
                    if (Goto->getTargetLabel() == Label &&
                        (getInternalGotoNodeSingleSuccessor(Goto) == Label)) {
                      FoundGotos.push_back(Goto);
                    }
                  });

    HLNode *LabelParent = Label->getParent();
    for (HLGoto *Goto : FoundGotos) {
      RedundantInstructions++;

      HLNode *Parent = Goto->getParent();

      HLNodeUtils::remove(Goto);
      Changed = true;

      LabelJumps[Label]--;
      GotosToRemove.erase(Goto);

      // Stop removing on LabelParent because at this point the Label will stay
      // attached and LabelParent may not be removed.
      removeEmptyNodesUntilParent(Parent, LabelParent);
    }
  }

  void visit(HLGoto *Goto) {
    if (Goto->isUnknownLoopBackEdge()) {
      return;
    }

    // Could remove GOTO if it's in a dead block.
    visit(static_cast<HLNode *>(Goto));

    // If goto is removed.
    if (SkipNode == Goto) {
      return;
    }

    HLNode *ContainerLastNode =
        HLNodeUtils::getLastLexicalChild(Goto->getParent(), Goto);

    if (!Goto->isExternal()) {
      GotosToRemove.insert(Goto);

      if (LabelSafeContainer) {
        LabelJumps[Goto->getTargetLabel()]++;
      }

      HLLabel *Label = Goto->getTargetLabel();
      HLNode *LabelContainerLastNode =
          HLNodeUtils::getLastLexicalChild(Label->getParent(), Label);

      if (LabelContainerLastNode == ContainerLastNode) {
        // Label and Goto are in the same container.
        LastNodeToRemove = Label;
        return;
      }
    }

    if (ContainerLastNode != Goto) {
      LastNodeToRemove = ContainerLastNode;
    }

    // Record side effect of multiple loop exits.
    // Note: Do not use Loop->isMultiexit() because the loop may be in
    // inconsistent state while removing redundant nodes.
    if (!LoopSideEffects.empty() && !LoopSideEffects.back().second) {
      if (isLoopExitGoto(LoopSideEffects.back().first, Goto)) {
        LoopSideEffects.back().second = true;
      }
    }
  }

  void visit(HLLabel *Label) {
    if (Label->isUnknownLoopHeaderLabel()) {
      return;
    }

    if (LastNodeToRemove == Label || IsJoinNode) {
      removeSiblingGotosWithTarget(Label);
    }

    auto MayRemoveLabel = [this](HLLabel *Label) {
      if (!LabelSafeContainer) {
        return false;
      }

      auto Iter = LabelJumps.find(Label);
      return ((Iter == LabelJumps.end()) || Iter->second == 0);
    };

    if (MayRemoveLabel(Label)) {
      if (LastNodeToRemove == nullptr) {
        LastNodeToRemove = Label;
      }
    } else {
      LastNodeToRemove = nullptr;
    }

    visit(static_cast<HLNode *>(Label));
  }

  void postVisit(HLLoop *Loop) {
    assert(LoopSideEffects.back().first == Loop &&
           "LoopSideEffects is out of sync");
    bool CurrentLoopSideEffect = LoopSideEffects.pop_back_val().second;

    if (LabelSafeContainer == Loop) {
      LabelSafeContainer = nullptr;
    }

    HLGoto *LastGoto = dyn_cast_or_null<HLGoto>(Loop->getLastChild());
    if (LastGoto) {
      // If there is a goto left at the end of the loop we have to convert loop
      // to a straight line code.

      // Stop removing nodes.
      LastNodeToRemove = nullptr;

      assert(isLoopExitGoto(Loop, LastGoto) &&
             "Non exit goto found at the end of the loop.");

      notifyWillRemoveNode(Loop);

      LoopOptReportBuilder &LORBuilder =
          Loop->getHLNodeUtils().getHIRFramework().getLORBuilder();

      LORBuilder(*Loop).preserveLostLoopOptReport();

      Loop->replaceByFirstIteration();
      RedundantEarlyExitLoops++;

      // Have to handle the label again in the context of parent loop.
      // But do not take into account previous jump;
      if (!LastGoto->isExternal()) {
        LabelJumps[LastGoto->getTargetLabel()]--;
      }

      visit(LastGoto);
      return;
    }

    // Remove loop if it doesn't produce any side effect.
    bool MayRemoveRedundantLoop =
        !DisableAggressiveRedundantLoopRemoval && !CurrentLoopSideEffect;

    if (!LoopSideEffects.empty()) {
      LoopSideEffects.back().second =
          LoopSideEffects.back().second || CurrentLoopSideEffect;
    }

    if (MayRemoveRedundantLoop) {
      HLNodeUtils::remove(Loop->child_begin(), Loop->child_end());
      EmptyNodeRemoverVisitorImpl::postVisit(Loop);
      return;
    }

    // The loop will stay attached - handle as regular node.
    postVisitImpl(Loop);
  }

  template <typename NodeTy> void postVisit(NodeTy *Node) {
    postVisitImpl(Node);
  }

  template <typename NodeTy> void postVisitImpl(NodeTy *Node) {
    assert(Node != SkipNode &&
           "Node is removed, should be no further actions.");

    IsJoinNode = true;

    LastNodeToRemove = nullptr;

    EmptyNodeRemoverVisitorImpl::postVisit(Node);
  }

  void visit(HLDDNode *Node) {
    // Record side effect of LVal or volatile memref.
    if (!LoopSideEffects.empty() && !LoopSideEffects.back().second) {
      for (RegDDRef *Ref : make_range(Node->ddref_begin(), Node->ddref_end())) {
        if (Ref->isMemRef() && (Ref->isVolatile() || Ref->isLval())) {
          LoopSideEffects.back().second = true;
        }
      }
    }

    visit(static_cast<HLNode *>(Node));
  }

  void visit(HLInst *Inst) {
    // Record side effect of calls.
    if (!LoopSideEffects.empty() && !LoopSideEffects.back().second) {
      if (Inst->isSideEffectsCallInst()) {
        LoopSideEffects.back().second = true;
      }
    }

    visit(static_cast<HLDDNode *>(Inst));
  }

  void visit(HLNode *Node) {
    if (LastNodeToRemove) {
      RedundantInstructions++;

      HLNodeUtils::remove(Node);
      Changed = true;

      // Do not recurse into removed nodes.
      SkipNode = Node;

      if (Node == LastNodeToRemove) {
        LastNodeToRemove = nullptr;
      }

      return;
    }

    // Unable to remove node means this is not a join point now.
    IsJoinNode = false;

    EmptyNodeRemoverVisitorImpl::visit(Node);
  }

  virtual bool skipRecursion(const HLNode *Node) const {
    return Node == SkipNode;
  }
};
} // namespace

template <typename VisitorTy>
static bool removeNodesRangeImpl(HLContainerTy::iterator Begin,
                                 HLContainerTy::iterator End,
                                 bool RemoveEmptyParentNodes) {
  if (Begin == End) {
    return false;
  }

#ifndef NDEBUG
  for (auto Iter = Begin; Iter != End; ++Iter) {
    LLVM_DEBUG(dbgs() << "While removing HIR nodes from <" << Iter->getNumber()
                      << ">:\n");
    LLVM_DEBUG(Iter->dump());
  }
#endif

  HLNode *Parent = Begin->getParent();

  VisitorTy V;
  HLNodeUtils::visitRange(V, Begin, End);

  if (RemoveEmptyParentNodes) {
    V.removeEmptyNodesUntilParent(Parent);
  }

  return V.isChanged();
}

template <typename VisitorTy>
static bool removeNodesImpl(HLNode *Node, bool RemoveEmptyParentNodes) {
  return removeNodesRangeImpl<VisitorTy>(
      Node->getIterator(), ++(Node->getIterator()), RemoveEmptyParentNodes);
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

struct UpdateLoopExitsVisitor final : public HLNodeVisitorBase {
  SmallVector<HLLoop *, MaxLoopNestLevel> Loops;

  void visit(HLLoop *Loop) {
    Loop->setNumExits(1);

    assert(Loop->hasChildren() && "Empty loops are unexpected.");

    Loops.push_back(Loop);
  }

  void postVisit(HLLoop *Loop) {
    assert(Loops.back() == Loop &&
           "Current loop is expected to be on top of the stack");
    Loops.pop_back();
  }

  void visit(HLGoto *Goto) {
    // Skip region level gotos.
    if (Loops.empty()) {
      return;
    }

    unsigned TargetTopsortNum =
        Goto->isExternal() ? -1 : Goto->getTargetLabel()->getTopSortNum();

    for (auto I = Loops.rbegin(), E = Loops.rend(); I != E; ++I) {
      HLLoop *Loop = *I;
      if (TargetTopsortNum > Loop->getMaxTopSortNum()) {
        Loop->setNumExits(Loop->getNumExits() + 1);
      } else {
        break;
      }
    }
  }

  void visit(HLNode *) {}
  void postVisit(HLNode *) {}
};

void HLNodeUtils::updateNumLoopExits(HLNode *Node) {
  UpdateLoopExitsVisitor V;
  HLNodeUtils::visit(V, Node);
}

template <typename T> void sortInTopOrderAndUniqHelper(T &Nodes) {
  auto NodeComparator = [](const HLNode *N1, const HLNode *N2) {
    return N1->getTopSortNum() < N2->getTopSortNum();
  };

  std::sort(Nodes.begin(), Nodes.end(), NodeComparator);
  auto Last = std::unique(Nodes.begin(), Nodes.end(),
                          [](const HLNode *N1, const HLNode *N2) {
                            return N1->getTopSortNum() == N2->getTopSortNum();
                          });

  Nodes.erase(Last, Nodes.end());
}

void HLNodeUtils::sortInTopOrderAndUniq(VecNodesTy &Nodes) {
  sortInTopOrderAndUniqHelper<VecNodesTy>(Nodes);
}

void HLNodeUtils::sortInTopOrderAndUniq(ConstVecNodesTy &Nodes) {
  sortInTopOrderAndUniqHelper<ConstVecNodesTy>(Nodes);
}

static void checkProfMetadata(MDNode *ProfileData) {
  assert(ProfileData->getNumOperands() == 3 &&
         isa<MDString>(ProfileData->getOperand(0)));

  MDString *MDName = cast<MDString>(ProfileData->getOperand(0));
  assert(MDName->getString() == "branch_weights");
  (void)MDName;
}

// Implementation-wise, it is almost the same as Instruction::swapProfMetadata.
MDNode *HLNodeUtils::swapProfMetadata(LLVMContext &Context,
                                      MDNode *ProfileData) {

  checkProfMetadata(ProfileData);

  // The first operand is the name. Fetch them backwards and build a new one.
  Metadata *Ops[] = {ProfileData->getOperand(0), ProfileData->getOperand(2),
                     ProfileData->getOperand(1)};

  return MDNode::get(Context, Ops);
}
