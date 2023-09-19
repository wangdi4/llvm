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

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/MDBuilder.h"
#include "llvm/IR/Metadata.h" // needed for MetadataAsValue -> Value
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Utils/BuildLibCalls.h"

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

HLInst *HLNodeUtils::createUnaryHLInst(unsigned OpCode, RegDDRef *RvalRef,
                                       const Twine &Name, RegDDRef *LvalRef,
                                       Type *DestTy,
                                       const UnaryInstruction *OrigUnInst) {
  HLInst *HInst =
      createUnaryHLInstImpl(OpCode, RvalRef, Name, LvalRef, DestTy, nullptr);
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

    Type *RvalType = RvalRef->getDestType();
    auto DummyPtrType =
        PointerType::get(RvalType, RvalRef->getPointerAddressSpace());
    auto DummyPtrVal = UndefValue::get(DummyPtrType);

    InstVal = DummyIRBuilder->CreateLoad(RvalType, DummyPtrVal, false, Name);
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

  case Instruction::Freeze: {
    InstVal = DummyIRBuilder->CreateFreeze(DummyVal, Name);
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
    assert((DestTy != DummyVal->getType()) && "Bad bitcast type");
    InstVal = DummyIRBuilder->CreateCast((Instruction::CastOps)OpCode, DummyVal,
                                         DestTy, Name);
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
  Function *SSACopyFunc =
      Intrinsic::getDeclaration(&getModule(), Intrinsic::ssa_copy, Ty);
  CallInst *Inst = DummyIRBuilder->CreateCall(SSACopyFunc, {DummyVal}, Name);
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
  case Instruction::AddrSpaceCast:
    return createAddrSpaceCast(DestTy, Op, Name, LvalRef);
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
                                RegDDRef *LvalRef, MDNode *FPMathTag,
                                FastMathFlags FMF) {
  HLInst *HInst = createUnaryHLInstImpl(Instruction::FNeg, RvalRef, Name,
                                        LvalRef, nullptr, FPMathTag);
  copyFMFForHLInst(HInst, FMF);
  return HInst;
}

HLInst *HLNodeUtils::createFreeze(RegDDRef *RvalRef, const Twine &Name,
                                  RegDDRef *LvalRef) {
  return createUnaryHLInstImpl(Instruction::Freeze, RvalRef, Name, LvalRef,
                               nullptr, nullptr);
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
  RegDDRef *IdxDDref = getDDRefUtils().createConstDDRef(
      Type::getInt64Ty(getDDRefUtils().getContext()), Idx);

  return createExtractElementInst(OpRef, IdxDDref, Name, LvalRef);
}

HLInst *HLNodeUtils::createExtractElementInst(RegDDRef *OpRef, RegDDRef *IdxRef,
                                              const Twine &Name,
                                              RegDDRef *LvalRef) {

  assert(OpRef->getDestType()->isVectorTy() &&
         "Illegal operand types for extractelement");

  auto UndefVal = UndefValue::get(OpRef->getDestType());
  auto UndefIdx = UndefValue::get(IdxRef->getDestType());
  Value *InstVal =
      DummyIRBuilder->CreateExtractElement(UndefVal, UndefIdx, Name);
  Instruction *Inst = cast<Instruction>(InstVal);
  assert((!LvalRef || LvalRef->getDestType() == Inst->getType()) &&
         "Incompatible type of LvalRef");

  HLInst *HInst = createLvalHLInst(Inst, LvalRef);

  HInst->setOperandDDRef(OpRef, 1);
  HInst->setOperandDDRef(IdxRef, 2);

  return HInst;
}

HLInst *HLNodeUtils::createExtractValueInst(RegDDRef *OpRef,
                                            ArrayRef<unsigned> Idxs,
                                            const Twine &Name,
                                            RegDDRef *LvalRef) {
  auto UndefVal = UndefValue::get(OpRef->getDestType());
  Value *InstVal = DummyIRBuilder->CreateExtractValue(UndefVal, Idxs, Name);
  Instruction *Inst = cast<Instruction>(InstVal);
  assert((!LvalRef || LvalRef->getDestType() == Inst->getType()) &&
         "Incompatible type of LvalRef");

  HLInst *HInst = createLvalHLInst(Inst, LvalRef);

  HInst->setOperandDDRef(OpRef, 1);

  return HInst;
}

HLInst *HLNodeUtils::createInsertValueInst(RegDDRef *OpRef, RegDDRef *ValRef,
                                           ArrayRef<unsigned> Idxs,
                                           const Twine &Name,
                                           RegDDRef *LvalRef) {
  auto UndefVal = UndefValue::get(OpRef->getDestType());
  auto Val = UndefValue::get(ValRef->getDestType());

  Value *InstVal = DummyIRBuilder->CreateInsertValue(UndefVal, Val, Idxs, Name);
  Instruction *Inst = cast<Instruction>(InstVal);
  assert((!LvalRef || LvalRef->getDestType() == Inst->getType()) &&
         "Incompatible type of LvalRef");

  HLInst *HInst = createLvalHLInst(Inst, LvalRef);

  HInst->setOperandDDRef(OpRef, 1);
  HInst->setOperandDDRef(ValRef, 2);

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

HLInst *HLNodeUtils::createFPMathBinOp(unsigned OpCode, RegDDRef *OpRef1,
                                       RegDDRef *OpRef2, FastMathFlags FMF,
                                       const Twine &Name, RegDDRef *LvalRef) {
  HLInst *HInst = createBinaryHLInstImpl(OpCode, OpRef1, OpRef2, Name, LvalRef,
                                         false, false, nullptr);
  copyFMFForHLInst(HInst, FMF);
  return HInst;
}

HLInst *HLNodeUtils::createOverflowingBinOp(unsigned OpCode, RegDDRef *OpRef1,
                                            RegDDRef *OpRef2, bool HasNUW,
                                            bool HasNSW, const Twine &Name,
                                            RegDDRef *LvalRef) {
  HLInst *HInst = createBinaryHLInstImpl(OpCode, OpRef1, OpRef2, Name, LvalRef,
                                         HasNUW, HasNSW, nullptr);
  assert(isa<OverflowingBinaryOperator>(HInst->getLLVMInstruction()) &&
         "OverflowingBinaryOperator instruction expected here");
  return HInst;
}

HLInst *HLNodeUtils::createPossiblyExactBinOp(unsigned OpCode, RegDDRef *OpRef1,
                                              RegDDRef *OpRef2, bool IsExact,
                                              const Twine &Name,
                                              RegDDRef *LvalRef) {
  HLInst *HInst = createBinaryHLInstImpl(OpCode, OpRef1, OpRef2, Name, LvalRef,
                                         IsExact, false, nullptr);
  assert(isa<PossiblyExactOperator>(HInst->getLLVMInstruction()) &&
         "PossiblyExactOperator instruction expected here");
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
                               RegDDRef *LvalRef, FastMathFlags FMF) {

  checkBinaryInstOperands(nullptr, OpRef1, OpRef2);

  if (LvalRef) {
    assert((LvalRef->getDestType()->isIntOrIntVectorTy() &&
            LvalRef->getDestType()->getScalarSizeInBits() == 1) &&
           "LvalRef has invalid type!");
  }

  auto DummyVal = UndefValue::get(OpRef1->getDestType());

  Value *InstVal = nullptr;
  if (OpRef1->getDestType()->isIntOrIntVectorTy() ||
      OpRef1->getDestType()->isPtrOrPtrVectorTy()) {
    InstVal =
        DummyIRBuilder->CreateICmp(ICmpInst::ICMP_EQ, DummyVal, DummyVal, Name);
  } else {
    InstVal = DummyIRBuilder->CreateFCmp(FCmpInst::FCMP_TRUE, DummyVal,
                                         DummyVal, Name);
  }

  HLInst *HInst = createLvalHLInst(cast<Instruction>(InstVal), LvalRef);
  copyFMFForHLInst(HInst, FMF);
  HInst->setPredicate(Pred);

  HInst->setOperandDDRef(OpRef1, 1);
  HInst->setOperandDDRef(OpRef2, 2);

  return HInst;
}

HLInst *HLNodeUtils::createSelect(const HLPredicate &Pred, RegDDRef *OpRef1,
                                  RegDDRef *OpRef2, RegDDRef *OpRef3,
                                  RegDDRef *OpRef4, const Twine &Name,
                                  RegDDRef *LvalRef, FastMathFlags FMF) {

  // LvalRef, OpRef3 and OpRef4 should be the same type.
  checkBinaryInstOperands(LvalRef, OpRef3, OpRef4);
  // OpRef1 and OpRef2 should be the same type.
  checkBinaryInstOperands(nullptr, OpRef1, OpRef2);

  auto CmpVal =
      UndefValue::get(Type::getInt1Ty(getHIRFramework().getContext()));
  auto DummyVal = UndefValue::get(OpRef3->getDestType());

  Value *InstVal =
      DummyIRBuilder->CreateSelect(CmpVal, DummyVal, DummyVal, Name);

  HLInst *HInst = createLvalHLInst(cast<Instruction>(InstVal), LvalRef);
  copyFMFForHLInst(HInst, FMF);
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
    FunctionType *FTy, Value *Callee, ArrayRef<RegDDRef *> CallArgs,
    const Twine &Name, RegDDRef *LvalRef, ArrayRef<OperandBundleDef> Bundle,
    ArrayRef<RegDDRef *> BundelOps) {
  bool HasReturn = !FTy->getReturnType()->isVoidTy();
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
      FTy, Callee, Args, Bundle,
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

HLInst *HLNodeUtils::createCall(FunctionType *FTy, Value *Callee,
                                ArrayRef<RegDDRef *> CallArgs,
                                const Twine &Name, RegDDRef *LvalRef,
                                ArrayRef<OperandBundleDef> Bundle,
                                ArrayRef<RegDDRef *> BundleOps,
                                FastMathFlags FMF) {
  HLInst *HInst;
  CallInst *Call;
  std::tie(HInst, Call) =
      createCallImpl(FTy, Callee, CallArgs, Name, LvalRef, Bundle, BundleOps);
  copyFMFForHLInst(HInst, FMF);

  return HInst;
}

HLInst *HLNodeUtils::createStacksave(const DebugLoc &DLoc) {

  Type *PtrTy =
#ifndef INTEL_SYCL_OPAQUEPOINTER_READY
      getContext().supportsTypedPointers()
          ? Type::getInt8PtrTy(getContext(), 0)
          :
#endif // INTEL_SYCL_OPAQUEPOINTER_READY
          getDataLayout().getAllocaPtrType(getContext());
  Function *StacksaveFunc =
      Intrinsic::getDeclaration(&getModule(), Intrinsic::stacksave, {PtrTy});

  SmallVector<RegDDRef *, 1> Ops;
  CallInst *Call;
  HLInst *HInst;
  std::tie(HInst, Call) = createCallImpl(StacksaveFunc, Ops);

  Call->setDebugLoc(DLoc);

  return HInst;
}

HLInst *HLNodeUtils::createStackrestore(RegDDRef *AddrArg) {
  Function *StackrestoreFunc = Intrinsic::getDeclaration(
      &getModule(), Intrinsic::stackrestore, {AddrArg->getDestType()});

  SmallVector<RegDDRef *, 1> Ops = {AddrArg};
  CallInst *Call;
  HLInst *HInst;
  std::tie(HInst, Call) = createCallImpl(StackrestoreFunc, Ops);

  Call->setDebugLoc(AddrArg->getDebugLoc());

  return HInst;
}

HLInst *HLNodeUtils::createNoAliasScopeDeclInst(RegDDRef *ScopeList) {
  Function *NoAliasScopeDeclFunc = Intrinsic::getDeclaration(
      &getModule(), Intrinsic::experimental_noalias_scope_decl);

  SmallVector<RegDDRef *, 1> Ops = {ScopeList};
  CallInst *Call;
  HLInst *HInst;
  std::tie(HInst, Call) = createCallImpl(NoAliasScopeDeclFunc, Ops);

  Call->setDebugLoc(ScopeList->getDebugLoc());

  return HInst;
}

HLInst *HLNodeUtils::createDbgPuts(const TargetLibraryInfo &TLI,
                                   HLRegion *Region, StringRef Message) {
  auto &Ctx = getContext();
  auto &DRU = getDDRefUtils();

  if (!TLI.has(LibFunc_puts))
    return nullptr;

  StringRef PutsName = TLI.getName(LibFunc_puts);
  FunctionCallee PutsCallee = getModule().getOrInsertFunction(
      PutsName, Type::getInt32Ty(Ctx), Type::getInt8PtrTy(Ctx, 0));
  inferNonMandatoryLibFuncAttrs(&getModule(), PutsName, TLI);

  GlobalVariable *ConstStr =
      DummyIRBuilder->CreateGlobalString(Message, "hir.str");

  unsigned ConstStrBlobIndex = InvalidBlobIndex;
  getBlobUtils().createGlobalVarBlob(ConstStr, true, &ConstStrBlobIndex);
  assert(ConstStrBlobIndex != InvalidBlobIndex);

  Region->addLiveInTemp(getBlobUtils().getTempBlobSymbase(ConstStrBlobIndex),
                        ConstStr);

  Type *IntPtr = getDataLayout().getIntPtrType(Ctx, 0);

  RegDDRef *ConstStrRef =
      DRU.createAddressOfRef(ConstStr->getValueType(), ConstStrBlobIndex, 0, GenericRvalSymbase);

  auto &CU = getCanonExprUtils();
  ConstStrRef->addDimension(CU.createCanonExpr(IntPtr, 0));
  ConstStrRef->addDimension(CU.createCanonExpr(IntPtr, 0));

  {
    SmallVector<BlobDDRef *, 8> NewBlobs;
    ConstStrRef->updateBlobDDRefs(NewBlobs);
  }

  CallInst *Call;
  HLInst *HInst;
  std::tie(HInst, Call) = createCallImpl(PutsCallee, {ConstStrRef});

  if (const Function *F =
          dyn_cast<Function>(PutsCallee.getCallee()->stripPointerCasts())) {
    Call->setCallingConv(F->getCallingConv());
  }

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
  RegDDRef *IsVolatile =
      getDDRefUtils().createConstDDRef(Type::getInt1Ty(getContext()), false);

  Type *Tys[] = {StoreRef->getDestType(), LoadRef->getDestType(),
                 Size->getDestType()};

  Function *MemcpyFunc =
      Intrinsic::getDeclaration(&getModule(), Intrinsic::memcpy, Tys);

  SmallVector<RegDDRef *, 5> Ops = {StoreRef, LoadRef, Size, IsVolatile};

  CallInst *Call;
  HLInst *HInst;
  std::tie(HInst, Call) = createCallImpl(MemcpyFunc, Ops);

  MemCpyInst *MemCpyCall = cast<MemCpyInst>(Call);
  MemCpyCall->setSourceAlignment(MaybeAlign(LoadRef->getAlignment()));
  MemCpyCall->setDestAlignment(MaybeAlign(StoreRef->getAlignment()));

  Call->setDebugLoc(StoreRef->getDebugLoc());

  return HInst;
}

HLInst *HLNodeUtils::createMemset(RegDDRef *StoreRef, RegDDRef *Value,
                                  RegDDRef *Size) {
  RegDDRef *IsVolatile =
      getDDRefUtils().createConstDDRef(Type::getInt1Ty(getContext()), false);

  Type *Tys[] = {StoreRef->getDestType(), Size->getDestType()};
  Function *MemsetFunc =
      Intrinsic::getDeclaration(&getModule(), Intrinsic::memset, Tys);

  SmallVector<RegDDRef *, 5> Ops = {StoreRef, Value, Size, IsVolatile};

  CallInst *Call;
  HLInst *HInst;
  std::tie(HInst, Call) = createCallImpl(MemsetFunc, Ops);

  MemSetInst *MemSetCall = cast<MemSetInst>(Call);
  MemSetCall->setDestAlignment(MaybeAlign(StoreRef->getAlignment()));

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

HLInst *HLNodeUtils::createVectorInsert(RegDDRef *OpRef1, RegDDRef *SubVecRef,
                                        unsigned Idx, const Twine &Name,
                                        RegDDRef *LvalRef) {
  assert(OpRef1->getDestType()->isVectorTy() &&
         SubVecRef->getDestType()->isVectorTy() &&
         "Illegal operand types for vector.insert");
  SmallVector<Type *, 2> Tys = {OpRef1->getDestType(),
                                SubVecRef->getDestType()};
  Function *F = Intrinsic::getDeclaration(
      &getModule(), Intrinsic::vector_insert, Tys);
  RegDDRef *IdxRef =
      getDDRefUtils().createConstDDRef(Type::getInt64Ty(getContext()), Idx);
  return createCall(F, {OpRef1, SubVecRef, IdxRef}, Name, LvalRef);
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
                                 HLContainerTy::iterator Last,
                                 HLContainerTy *MoveContainer, bool Erase) {
  assert(!(MoveContainer && Erase) &&
         "Caller wants to move and erase nodes at the same time!");

  HLNode *Node;

  for (auto I = First, Next = I, E = Last; I != E; I = Next) {

    Next++;
    Node = &*I;

    if (!MoveContainer) {
      Container.remove(*Node);
    }

    if (Erase) {
      Node->getHLNodeUtils().destroy(Node);
    } else {
      /// Used to catch errors where user tries to insert an already linked
      /// node.
      Node->setParent(nullptr);
    }
  }

  if (MoveContainer) {
    MoveContainer->splice(MoveContainer->end(), Container, First, Last);
  }
}

void HLNodeUtils::removeImpl(HLContainerTy::iterator First,
                             HLContainerTy::iterator Last,
                             HLContainerTy *MoveContainer, bool Erase) {
  assert(!(MoveContainer && Erase) &&
         "Caller wants to move and erase nodes at the same time!");

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
    removeInternal(*OrigContainer, First, Last, nullptr, true);
  } else {
    removeInternal(*OrigContainer, First, Last, MoveContainer, false);
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

  // We do prev/next node checks for compile time optimization. Removing OldNode
  // first gets us a bigger top sort num range for new nodes. This makes it less
  // likely to hit the slower path in the top sort num range recomputation where
  // range isn't big enough to hold the new nodes. This is especially useful
  // when clients are using Marker node for replacement.
  // Top sort num can be 0 if utility is called from framework. In that case we
  // cannot use getPrevNode()/getNextNode().

  if (OldNode->getTopSortNum() != 0) {
    if (auto *NextNode = OldNode->getNextNode()) {
      remove(OldNode);
      insertBefore(NextNode, NewNode);

      return;

    } else if (auto *PrevNode = OldNode->getPrevNode()) {
      remove(OldNode);
      insertAfter(PrevNode, NewNode);

      return;
    }
  }

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
    if (auto Loop = dyn_cast<HLLoop>(Parent)) {
      if (std::next(Iter) != Loop->Children.end()) {
        Succ = &*(std::next(Iter));
        break;
      }

    } else if (auto Reg = dyn_cast<HLRegion>(Parent)) {
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

// Step of 2048 is sufficient to represent about 2 million nodes in a region.
static cl::opt<unsigned> TopSortNumSpacing(
    "hir-topsort-num-spacing", cl::init(2048), cl::Hidden,
    cl::desc("gap between top sort number of two consecutive HLNodes"));

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

  // Step of 0 implies default spacing.
  TopSorter(unsigned MinNum, unsigned Step = 0,
            const HLNode *AfterNode = nullptr)
      : MinNum(MinNum), TopSortNum(MinNum), AfterNode(AfterNode), Stop(false) {
    this->Step = Step ? Step : TopSortNumSpacing;
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

    if (Step != 0) {
      // Evenly assign nodes in [First, Last) top sort numbers in range (MinNum,
      // MaxNum)
      TopSorter<true> TS(MinNum, Step);
      HLNodeUtils::visitRange(TS, First, Last);
    } else {
      // There are more nodes in [First, Last) than the available top sort
      // numbers in range (MinNum, MaxNum) so reassign top sort numbers to nodes
      // starting from First until the end of the region or until top sort num
      // of subsequent number is already in order. We use half the default
      // spacing to make it more probable that we don't have to reassign to all
      // the subsequent nodes in the region.

      Step = TopSortNumSpacing / 2;

      // We need to force reassignment of the nodes in [First, Last) otheriwse
      // TopSorter might incorrectly use their existing (invalid) top sort num
      // value.
      TopSorter<true> TS1(MinNum, Step);
      HLNodeUtils::visitRange(TS1, First, Last);

      // Reassign to subseuquent nodes. This traverses the entire region but
      // starts reassignment from the 'Last' node. This might be expensive in
      // compile time if the region is big but the alternative is to complicate
      // the traversal algoritm.
      // TODO: Look into improving the traversal algorithm.
      TopSorter<false> TS2(MinNum + (NC.Count * Step), Step, &*std::prev(Last));
      HLNodeUtils::visit(TS2, First->getParentRegion());
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
  assert(Num != 0 && "Node does not have a top sort number!");

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

bool HLNodeUtils::isLexicalLastChildOfParent(const HLNode *Node) {
  auto *Parent = Node->getParent();

  if (auto *If = dyn_cast<HLIf>(Parent)) {
    return (Node == If->getLastThenChild()) || (Node == If->getLastElseChild());
  }

  if (auto *Switch = dyn_cast<HLSwitch>(Parent)) {
    if (Node == Switch->getLastDefaultCaseChild()) {
      return true;
    }

    for (unsigned I = 1, E = Switch->getNumCases(); I <= E; ++I) {
      if (Node == Switch->getLastCaseChild(I)) {
        return true;
      }
    }

    return false;
  }

  if (auto *Lp = dyn_cast<HLLoop>(Parent)) {
    return (Node == Lp->getLastChild());
  }

  return (Node == cast<HLRegion>(Parent)->getLastChild());
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

/// Returns \p Node if it is an HLLoop or the parent loop of \p Node if it is
/// not.
static const HLLoop *getNearestLoop(const HLNode *Node) {
  if (const auto *const NodeLp = dyn_cast<HLLoop>(Node))
    return NodeLp;
  if (isa<HLRegion>(Node))
    return nullptr;
  return Node->getParentLoop();
}

const HLNode *HLNodeUtils::getOutermostSafeParent(
    const HLNode *Node1, const HLNode *Node2, bool PostDomination,
    HIRLoopStatistics *HLS, const HLNode **LastParent1,
    SmallVectorImpl<const HLLoop *> &Parent1LoopsWithZtt) {
  const HLNode *Parent = Node1->getParent();
  const HLNode *FirstNode = nullptr, *LastNode = nullptr;

  *LastParent1 = Node1;

  // Try to move up the parent chain by crossing loops.
  while (Parent) {

    auto *Loop = dyn_cast<HLLoop>(Parent);

    if (!Loop) {
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

    // We are crossing over a loop with Ztt. Add it to the vector which will
    // be later used for checking whether Node2 is under the same Ztt
    // conditions as well.
    if (Loop->hasZtt()) {
      Parent1LoopsWithZtt.push_back(Loop);
    }

    *LastParent1 = Parent;
    Parent = Parent->getParent();
  }

  return Parent;
}

static void eraseIdenticalZttLoops(SmallVectorImpl<const HLLoop *> &ZttLoops,
                                   const HLLoop *RefZttLoop) {
  auto Iter = remove_if(ZttLoops, [RefZttLoop](const HLLoop *Loop) {
    return HLNodeUtils::areEqualZttConditions(RefZttLoop, Loop);
  });
  ZttLoops.erase(Iter, ZttLoops.end());
}

const HLNode *HLNodeUtils::getCommonDominatingParent(
    const HLNode *Parent1, const HLNode *LastParent1, const HLNode *Node2,
    bool PostDomination, HIRLoopStatistics *HLS, const HLNode **LastParent2,
    SmallVectorImpl<const HLLoop *> &Parent1LoopsWithZtt) {

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

    if (CommonParent == Parent1) {
      break;
    }

    auto *Parent2Lp = dyn_cast<HLLoop>(CommonParent);

    if (Parent2Lp && Parent2Lp->hasZtt()) {
      eraseIdenticalZttLoops(Parent1LoopsWithZtt, Parent2Lp);
    }

    *LastParent2 = CommonParent;
    CommonParent = CommonParent->getParent();
  }

  // Check whether Node2 is under the same conditions we crossed over to get to
  // outermost parent of Node1.
  return Parent1LoopsWithZtt.empty() ? CommonParent : nullptr;
}

// Checks whether Labels/Gotos between Node1 and Node2 disprove
// domination/post-domination.
static bool hasStructuredControlFlow(const HLNode *Node1, const HLNode *Node2,
                                     const HLNode *CommonParent,
                                     bool PostDomination,
                                     HIRLoopStatistics *HLS) {

  auto *ParentLp = getNearestLoop(CommonParent);

  auto &TLS = ParentLp ? HLS->getTotalStatistics(ParentLp)
                       : HLS->getTotalStatistics(Node1->getParentRegion());

  if (PostDomination) {
    if (!TLS.hasForwardGotos()) {
      return true;
    }
  } else if (!TLS.hasLabels()) {
    return true;
  }

  unsigned TSNum1 = Node1->getTopSortNum();
  unsigned TSNum2 = Node2->getTopSortNum();

  if (PostDomination) {
    // Check if there is a goto between Node2 and Node1 which jumps after Node1.
    // Note that Node1 comes lexically after Node2.
    //
    // Example-
    //
    //  A[i] = // Node2
    //  if () {
    //    goto L;
    //  }
    //  A[i] = // Node1
    //  L:
    for (auto *Goto : TLS.getForwardGotos()) {
      unsigned GotoTSNum = Goto->getTopSortNum();

      if (GotoTSNum < TSNum2) {
        continue;
      }

      if (GotoTSNum >= TSNum1) {
        break;
      }

      if (Goto->isExternal() ||
          Goto->getTargetLabel()->getTopSortNum() > TSNum1) {
        return false;
      }
    }

    return true;
  }

  // Check if there is a goto before Node1 which jumps between Node1 and Node2.
  // Note that Node2 comes lexically after Node1.
  // Example-
  //
  //  if () {
  //    goto L;
  //  }
  //  A[i] = // Node1
  //  L:
  //  A[i] = // Node2
  for (auto *Goto : TLS.getForwardGotos()) {
    if (Goto->getTopSortNum() > TSNum1) {
      break;
    }

    if (Goto->isExternal()) {
      continue;
    }

    unsigned LabelTSNum = Goto->getTargetLabel()->getTopSortNum();

    if (LabelTSNum > TSNum1 && LabelTSNum <= TSNum2) {
      return false;
    }
  }

  return true;
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

  assert(HLS && "Loop statistics is not available!");

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
  SmallVector<const HLLoop *, 4> Parent1LoopsWithZtt;

  const HLNode *Parent1 = getOutermostSafeParent(
      Node1, Node2, PostDomination, HLS, &LastParent1, Parent1LoopsWithZtt);

  assert(Parent1 && "Could not find appropriate parent for Node1!");

  const HLNode *LastParent2 = nullptr;
  const HLNode *CommonParent =
      getCommonDominatingParent(Parent1, LastParent1, Node2, PostDomination,
                                HLS, &LastParent2, Parent1LoopsWithZtt);

  // Couldn't find a common parent.
  if (!CommonParent) {
    return false;
  }

  if (!hasStructuredControlFlow(Node1, Node2, CommonParent, PostDomination,
                                HLS)) {
    return false;
  }

  const HLIf *IfParent = dyn_cast<HLIf>(CommonParent);
  const HLSwitch *SwitchParent = dyn_cast<HLSwitch>(CommonParent);

  // For region and loops parents we can deduce the result right away.
  if (!IfParent && !SwitchParent) {
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

class InvariantBlobChecker {
private:
  HLRegion *Reg;
  BlobUtils &BU;
  bool IsInvariant;

public:
  InvariantBlobChecker(HLRegion *Reg, BlobUtils &BU)
      : Reg(Reg), BU(BU), IsInvariant(true) {}

  bool follow(const SCEV *SC) {
    if (BU.isTempBlob(SC)) {
      unsigned Symbase = BU.findTempBlobSymbase(SC);

      if (!Reg->isInvariant(Symbase)) {
        IsInvariant = false;
      }

      return false;
    }

    return !isDone();
  }

  bool isDone() const { return !IsInvariant; }
  bool isInvariantBlob() const { return IsInvariant; }
};

bool HLNodeUtils::isRegionInvariant(HLRegion *Reg, BlobUtils &BU,
                                    unsigned BlobIdx) {

  auto *Blob = BU.getBlob(BlobIdx);

  // Ignore vector/fp constant blobs. Even though they are invariant, the
  // callers can't do anything with them.
  // TODO: This is a hack. We should bailout in the caller instead.
  if (BlobUtils::isConstantVectorBlob(Blob) ||
      BlobUtils::isConstantFPBlob(Blob)) {
    return false;
  }

  InvariantBlobChecker LBC(Reg, BU);
  SCEVTraversal<InvariantBlobChecker> Checker(LBC);
  Checker.visitAll(Blob);

  return LBC.isInvariantBlob();
}

bool HLNodeUtils::getMinBlobValue(unsigned BlobIdx, const HLNode *ParentNode,
                                  int64_t &Val) {
  VALType ValType = getMinMaxBlobValue(BlobIdx, ParentNode, Val);

  if (isMinValue(ValType)) {
    return true;
  }

  auto &BU = ParentNode->getBlobUtils();

  if (isRegionInvariant(ParentNode->getParentRegion(), BU, BlobIdx)) {
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
  if (isRegionInvariant(ParentNode->getParentRegion(), BU, BlobIdx) &&
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
    LLVM_FALLTHROUGH;
  case PredicateTy::ICMP_SLE: // <=
    break;
  case PredicateTy::ICMP_SGT: // >
    EqualOffset = -1;
    LLVM_FALLTHROUGH;
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

  LLVM_DEBUG(dbgs() << "\n\tget" << (IsMin ? "Min" : "Max")
                    << "Value() called for ";
             CE->dump(););

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

  auto *ParentLoop = isa<HLLoop>(ParentNode) ? cast<HLLoop>(ParentNode)
                                             : ParentNode->getParentLoop();

  unsigned Bitsize = 64;
  APInt MinOrMax(Bitsize, 0);

  for (auto Blob = CE->blob_begin(), E = CE->blob_end(); Blob != E; ++Blob) {
    unsigned Index = CE->getBlobIndex(Blob);
    int64_t CoeffVal = CE->getBlobCoeff(Blob);
    int64_t BlobVal;

    // If upper looks like this: (%n + -1 * %s)
    // and CE looks like this: (i1 + %s)
    //
    // We can ignore %s blob as it has already been taken into account in the
    // loop's legal max trip count.
    if (!IsMin && (CoeffVal == 1) && ParentLoop && !ParentLoop->isUnknown() &&
        (ParentLoop->getUpperCanonExpr()->getBlobCoeff(Index) == -1) &&
        (CE->getIVConstCoeff(ParentLoop->getNestingLevel()) == 1) &&
        (CE->getIVBlobCoeff(ParentLoop->getNestingLevel()) ==
         InvalidBlobIndex)) {
      continue;
    }

    if (!getMinMaxBlobVal(Index, CoeffVal, ParentNode, IsMin, BlobVal)) {
      return false;
    }

    // Calculate MinOrMax += (Coeff * BlobVal) checking for overflow.
    bool Overflow = false;
    APInt Coeff(Bitsize, CoeffVal);
    APInt BlobInt(Bitsize, BlobVal);
    BlobInt = BlobInt.smul_ov(Coeff, Overflow);
    if (Overflow)
      return false;
    MinOrMax = MinOrMax.sadd_ov(BlobInt, Overflow);
    if (Overflow)
      return false;
  }

  if (CE->hasIV()) {
    assert(ParentLoop && "Parent loop of CE containing IV not found!");

    auto *Lp = ParentLoop;
    for (unsigned Level = Lp->getNestingLevel(); Level > 0;
         --Level, Lp = Lp->getParentLoop()) {

      unsigned Index;
      int64_t CoeffVal, BlobVal = 1;

      CE->getIVCoeff(Level, &Index, &CoeffVal);

      if (!CoeffVal) {
        continue;
      }

      if (Index != InvalidBlobIndex &&
          !getMinMaxBlobVal(Index, CoeffVal, ParentNode, IsMin, BlobVal)) {
        return false;
      }

      if (!BlobVal) {
        continue;
      }

      bool HasPositiveCoeff = ((CoeffVal > 0) && (BlobVal > 0)) ||
                              ((CoeffVal < 0) && (BlobVal < 0));

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
      } else if (Lp->isUnknown() ||
                 !getMinMaxValueImpl(Lp->getUpperCanonExpr(), Lp, false,
                                     IsExact, BoundVal)) {
        if (IsExact)
          return false;

        int64_t LegalMaxTC = Lp->getLegalMaxTripCount();
        if (!LegalMaxTC)
          return false;
        BoundVal = LegalMaxTC - 1;
      }

      // Conservatively return false if bound is too big.
      // Honor the value if obtained using exact analysis.
      if (!IsExact && ((BoundVal < 0) || (BoundVal > UINT32_MAX))) {
        return false;
      }

      // Calculate MinOrMax += (CoeffVal * BlobVal * BoundVal) checking for
      // overflow.
      bool Overflow = false;
      APInt Coeff(Bitsize, CoeffVal);
      APInt BlobInt(Bitsize, BlobVal);
      BlobInt = BlobInt.smul_ov(Coeff, Overflow);
      if (Overflow)
        return false;
      APInt Bound(Bitsize, BoundVal);
      BlobInt = BlobInt.smul_ov(Bound, Overflow);
      if (Overflow)
        return false;
      MinOrMax = MinOrMax.sadd_ov(BlobInt, Overflow);
      if (Overflow)
        return false;
    }
  }

  bool Overflow = false;
  APInt Const(Bitsize, CE->getConstant());
  MinOrMax = MinOrMax.sadd_ov(Const, Overflow);
  if (Overflow)
    return false;

  int64_t DenomVal = CE->getDenominator();
  if (DenomVal != 1) {
    if (MinOrMax.isNegative() && CE->isUnsignedDiv()) {
      return false;
    }

    APInt Denom(Bitsize, DenomVal);
    MinOrMax = MinOrMax.sdiv_ov(Denom, Overflow);
    if (Overflow)
      return false;
  }

  unsigned SrcTypeWidth = CE->getSrcType()->getPrimitiveSizeInBits();
  unsigned DestTypeWidth = CE->getDestType()->getPrimitiveSizeInBits();
  unsigned MinWidth =
      (SrcTypeWidth < DestTypeWidth) ? SrcTypeWidth : DestTypeWidth;

  if (MinOrMax.getSignificantBits() > MinWidth)
    return false;

  Val = MinOrMax.getSExtValue();

  LLVM_DEBUG(dbgs() << "\t\t returned " << Val << "\n";);

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
  auto SrcTy = CE->getSrcType()->getScalarType();
  assert(SrcTy->isIntegerTy() && "CE does not have an integer type!");

  if (IgnoreWraparound) {
    return false;
  }

  if (!CE->hasIV(Level)) {
    return false;
  }

  if (CE->isTrunc()) {
    // For standalone IVs like trunc.i64.i32(i1), check whether the destination
    // type is big enough to fit the max legal trip count of the loop. If it is,
    // there can be no wraparound.
    unsigned LoopLevel;
    if (!ParentNode || !CE->isStandAloneIV(true, &LoopLevel)) {
      return true;
    }

    auto *ParentLp = ParentNode->getParentLoopAtLevel(LoopLevel);

    uint64_t LegalMaxTC = ParentLp->getLegalMaxTripCount();

    if (!LegalMaxTC) {
      return true;
    }

    unsigned DstTySize =
        CE->getDestType()->getScalarType()->getScalarSizeInBits();

    // CE is already trunc, hasSignedIV() is applicable only to
    // IV type, likely to be source type of the trunc.
    APInt MaxTypeVal = APInt::getMaxValue(DstTySize);

    return (MaxTypeVal.getZExtValue() < LegalMaxTC);
  }

  // We only handle zext for now as we have real test cases for it.
  // sext is problematic because no-wrap info is easily lost before loopopt so
  // we will become too conservative. Handling sext is left as a TODO
  // until we have a real test case.
  if (!CE->isZExt()) {
    return false;
  }

  if (FitsIn32Bits && SrcTy->getScalarSizeInBits() >= 32) {
    return false;
  }

  // Check whether the CE is in the range of its src type.

  // Negative values indicate wraparound for zext.
  if (!isKnownNonNegative(CE, ParentNode)) {
    return true;
  }

  auto *IntTy = cast<IntegerType>(SrcTy);
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

    assert(LHS->getDestType() == RHS->getDestType() && "LHS/RHS type mismatch");
    if (LHS->getSrcType() != LHS->getDestType() ||
        RHS->getSrcType() != RHS->getDestType()) {
      // With cast, just bail out.
      return false;
    }

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

const HLLoop *
HLNodeUtils::getHighestAncestorForPerfectLoopNest(const HLLoop *InnermostLoop) {

  assert(InnermostLoop);

  auto *ParLoop = InnermostLoop->getParentLoop();
  const HLLoop *OutermostLoop = nullptr;

  while (ParLoop && isPerfectLoopNest(ParLoop)) {
    OutermostLoop = ParLoop;
    ParLoop = ParLoop->getParentLoop();
  }

  return OutermostLoop;
}

/// Checks to see if MemRef has inner 2 Dimensionssize == tripcount, has
/// standalone IVs, and returns the IV levels.
/// Examples: Inner Access: A[i1][i2], or Diagonal Access: B[i3][i1][i1]
static bool isMatrixLikeAccess(const RegDDRef *MemRef, uint64_t TripCount,
                               unsigned *FirstLevel, unsigned *SecondLevel) {
  unsigned NumDims = MemRef->getNumDimensions();
  if (NumDims < 2) {
    return false;
  }

  // Allow Refs that have standalone IV for outer dims > 2 OR in case of
  // global array, outermost dimension index == 0
  for (unsigned Dim = 3; Dim <= NumDims; Dim++) {
    if ((Dim != NumDims) && !MemRef->getDimensionIndex(Dim)->isStandAloneIV()) {
      return false;
    } else if (Dim == NumDims &&
               !MemRef->getDimensionIndex(Dim)->isStandAloneIV() &&
               !MemRef->getDimensionIndex(Dim)->isZero()) {
      return false;
    }
  }

  if (MemRef->getNumDimensionElements(1) != TripCount ||
      MemRef->getNumDimensionElements(2) != TripCount) {
    return false;
  }

  if (!MemRef->getDimensionIndex(1)->isStandAloneIV(false, FirstLevel) ||
      !MemRef->getDimensionIndex(2)->isStandAloneIV(false, SecondLevel)) {
    return false;
  }

  return true;
}

/// Check the first outer loop for instruction that sets the diagonal of an
/// array like: Ident[i1][i1] = 1.0
/// Maintain a list of disqualifying stores. Disqualifies stores consists of
/// all stores not setting the diagonal, or ones that may write to same
/// location. Refs returned in \p DiagCandidates are checked to postdominate
/// innerloop, and will not collide with any other store.
void findOuterDiagInst(const HLLoop *Outer, uint64_t TripCount,
                       SmallVector<const RegDDRef *, 8> &DiagCandidates,
                       SmallSet<unsigned, 16> &DisqualifiedSymbases) {
  SmallSet<unsigned, 8> DiagRefSymbases;
  bool FoundInnerLoop = false;
  for (const auto &Node :
       make_range(Outer->child_rbegin(), Outer->child_rend())) {
    // Ensure only one innerloop seen in outerloop.
    if (isa<HLLoop>(&Node)) {
      if (!FoundInnerLoop) {
        FoundInnerLoop = true;
        continue;
      } else {
        DiagCandidates.clear();
        break;
      }
    }

    const auto *Inst = dyn_cast<HLInst>(&Node);
    if (!Inst) {
      DiagCandidates.clear();
      break;
    }

    const RegDDRef *LvalRef = Inst->getLvalDDRef();
    if (!LvalRef || !LvalRef->isMemRef()) {
      continue;
    }

    unsigned LvalSymbase = LvalRef->getSymbase();

    // Disqualify if store occurs before innerloop
    if (FoundInnerLoop) {
      DisqualifiedSymbases.insert(LvalSymbase);
    }

    // Bailout if symbase is a disqualifying store
    if (DisqualifiedSymbases.count(LvalSymbase)) {
      continue;
    }

    const RegDDRef *RvalRef = Inst->getRvalDDRef();
    if (!RvalRef || !RvalRef->getSingleCanonExpr()->isOne()) {
      DisqualifiedSymbases.insert(LvalSymbase);
      continue;
    }

    unsigned FirstLevel, SecondLevel;
    if (!isMatrixLikeAccess(LvalRef, TripCount, &FirstLevel, &SecondLevel)) {
      DisqualifiedSymbases.insert(LvalSymbase);
      continue;
    }

    // Check both IVLevels == OuterLevel
    if ((FirstLevel != SecondLevel) ||
        (FirstLevel != Outer->getNestingLevel())) {
      DisqualifiedSymbases.insert(LvalSymbase);
      continue;
    }

    // Disqualify refs with same symbase as previously found candidates.
    if (DiagRefSymbases.count(LvalSymbase)) {
      DisqualifiedSymbases.insert(LvalSymbase);
      continue;
    }

    LLVM_DEBUG(dbgs() << "Found Diag Inst in OuterLp: "; Inst->dump();
               dbgs() << "\n";);
    DiagCandidates.push_back(LvalRef);
    DiagRefSymbases.insert(LvalSymbase);
  }

  // Remove candidates that might have been disqualified later
  for (auto It = DiagCandidates.begin(); It != DiagCandidates.end();) {
    if (DisqualifiedSymbases.count((*It)->getSymbase())) {
      It = DiagCandidates.erase(It);
    } else {
      ++It;
    }
  }
}

/// Look for innermost store of zeros in the innermost Loop corresponding to
/// \p Diagonals. Anything not writing zero to A[i1][i2] or A[i2][i1]
/// disqualifies the symbase for A. \p DisqualifiedSymbases is re-used from
/// findOuterDiagInst()
void findInnerZeroInst(const HLLoop *Inner, uint64_t TripCount,
                       SmallVector<const RegDDRef *, 2> &Diagonals,
                       SmallVector<const RegDDRef *, 8> &DiagCandidates,
                       SmallSet<unsigned, 16> &DisqualifiedSymbases) {

  // Set of symbases for found memrefs assigned to zero
  SmallSet<unsigned, 8> ZeroRefSymbases;
  SmallVector<const RegDDRef *, 8> ZeroRefs;

  unsigned InnerLevel = Inner->getNestingLevel();
  unsigned OuterLevel = InnerLevel - 1;

  // Look for 2 groups of instrs. 2D memrefstores of all zeros, and memrefstores
  // of non-const values which disqualify all candidates
  // Candidates look like: A[i1][i2] = 0.0
  // Disqualifying stores might be: A[i1][i2] = C[i3] , A[i1] = *p, etc.
  for (const auto &Node :
       make_range(Inner->child_rbegin(), Inner->child_rend())) {
    const auto *Inst = dyn_cast<HLInst>(&Node);

    if (!Inst) {
      continue;
    }

    const RegDDRef *LvalRef = Inst->getLvalDDRef();
    if (!LvalRef || !LvalRef->isMemRef()) {
      continue;
    }

    unsigned LvalSymbase = LvalRef->getSymbase();

    // Bailout if symbase is a disqualifying store
    if (DisqualifiedSymbases.count(LvalSymbase)) {
      continue;
    }

    const RegDDRef *RvalRef = Inst->getRvalDDRef();
    if (!RvalRef || !RvalRef->isZero()) {
      DisqualifiedSymbases.insert(LvalSymbase);
      continue;
    }

    unsigned FirstLevel, SecondLevel;
    if (!isMatrixLikeAccess(LvalRef, TripCount, &FirstLevel, &SecondLevel)) {
      DisqualifiedSymbases.insert(LvalSymbase);
      continue;
    }

    // Ensure IV corresponds to the inner & outer loops. Either match:
    // A[i1][i2] or A[i2][i1] where i1 and i2 are inner & outer
    if (!((FirstLevel == InnerLevel && SecondLevel == OuterLevel) ||
          (FirstLevel == OuterLevel && SecondLevel == InnerLevel))) {
      DisqualifiedSymbases.insert(LvalSymbase);
      continue;
    }

    // Disqualify refs that share same symbase as previous candidates
    if (ZeroRefSymbases.count(LvalSymbase)) {
      DisqualifiedSymbases.insert(LvalSymbase);
      continue;
    }

    // Save ref to check for aliasing
    ZeroRefs.push_back(LvalRef);
    ZeroRefSymbases.insert(LvalSymbase);
    LLVM_DEBUG(dbgs() << "Found Zero Instruction: "; Inst->dump();
               dbgs() << "\n";);
  }

  // Final check on diags/zeros/disqualifiers for ID matrix
  for (auto &DiagRef : DiagCandidates) {
    unsigned DiagSymbase = DiagRef->getSymbase();

    // Candidate symbase must not be disqualified and must exist in
    // ZeroRefSymbases.
    if (DisqualifiedSymbases.count(DiagSymbase) ||
        !ZeroRefSymbases.count(DiagSymbase)) {
      continue;
    }

    // Ensure for the same symbase of diagref and zeroref that they are not
    // aliased and the diagref postdominates the zeroref
    bool IsAliased = false;
    auto DiagCE = DiagRef->getBaseCE();
    for (const auto &ZeroRef : ZeroRefs) {
      if (ZeroRef->getSymbase() == DiagSymbase) {
        if (!CanonExprUtils::areEqual(DiagCE, ZeroRef->getBaseCE(), false)) {
          IsAliased = true;
          break;
        }
      }
    }

    if (IsAliased) {
      continue;
    }

    Diagonals.push_back(DiagRef);
    LLVM_DEBUG(dbgs() << "Found Ident Matrix, DiagInst: ";
               DiagRef->getHLDDNode()->dump(); dbgs() << "\n";);
  }
}

/// 503.bwavs utility for identifying identity matrix. Looks for following:
/// do i=1,5
///   do j=1,5
///     A(j,i) = 0.0
///   enddo
///   A(i,i) = 1.0
/// enddo
/// Note: A could have 2+ dimensions and the i could be in another loopnest.
/// We try to handle cases where additional dimenions are considered.
/// The key is finding the diagonal instruction in the outer loop and
/// correpsonding zero instruction in the inner loop for the same ref.
/// We track symbases that stores any invalid Rvals at each step
/// and disqualify those symbases from final candidates.
void HLNodeUtils::findInner2DIdentityMatrix(
    HIRLoopStatistics *HLS, const HLLoop *InnerLp,
    SmallVector<const RegDDRef *, 2> &Diagonals) {
  assert(InnerLp && InnerLp->isInnermost() && "InnerLoop is null\n");
  assert(HLS && "Loop Statistics unavailable!\n");
  Diagonals.clear();

  if (!InnerLp->isDo() || !InnerLp->isNormalized()) {
    return;
  }

  const HLLoop *OuterLp = InnerLp->getParentLoop();
  if (!OuterLp || !OuterLp->isDo() || !OuterLp->isNormalized()) {
    return;
  }

  if (InnerLp->hasPreheader() || OuterLp->hasPreheader() ||
      InnerLp->hasPostexit() || OuterLp->hasPostexit()) {
    return;
  }

  uint64_t InnerLpTripCount = 0;
  uint64_t OuterLpTripCount = 0;

  if (!(InnerLp->isConstTripLoop(&InnerLpTripCount) &&
        OuterLp->isConstTripLoop(&OuterLpTripCount) &&
        InnerLpTripCount == OuterLpTripCount)) {
    return;
  }

  auto &OuterLS = HLS->getTotalStatistics(OuterLp);

  if (OuterLS.hasSwitches() || OuterLS.hasIfs() || OuterLS.hasCalls() ||
      OuterLS.hasLabels()) {
    return;
  }

  SmallVector<const RegDDRef *, 8> DiagCandidates;
  SmallSet<unsigned, 16> DisqualifiedSymbases;
  // Find diagonal inst in outer loop. Break once we see inner loop
  findOuterDiagInst(OuterLp, OuterLpTripCount, DiagCandidates,
                    DisqualifiedSymbases);

  if (DiagCandidates.empty()) {
    return;
  }

  // Find the corresponding instruction that zeros the matrix in the inner loop
  findInnerZeroInst(InnerLp, OuterLpTripCount, Diagonals, DiagCandidates,
                    DisqualifiedSymbases);

  if (!Diagonals.empty()) {
    LLVM_DEBUG(dbgs() << "Found Identity Matrix for Loop:\n"; OuterLp->dump(););
  }
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

  HLNode *Parent1 = Node1->getLexicalParent();
  HLNode *Parent2 = Node2->getLexicalParent();

  if (isa<HLRegion>(Parent1)) {
    return Parent1;
  }

  if (isa<HLRegion>(Parent2)) {
    return Parent2;
  }

  return getLexicalLowestCommonAncestor(Parent1, Parent2);
}

HLNode *HLNodeUtils::getLexicalLowestCommonAncestorParent(HLNode *Node1,
                                                          HLNode *Node2) {
  return const_cast<HLNode *>(getLexicalLowestCommonAncestorParent(
      static_cast<const HLNode *>(Node1), static_cast<const HLNode *>(Node2)));
}

const HLNode *HLNodeUtils::getLexicalLowestCommonAncestor(const HLNode *Node1,
                                                          const HLNode *Node2) {
  assert(Node1 && Node2 && "Node1 or Node2 is null!");

  unsigned TopSortNum1 = Node1->getTopSortNum();
  unsigned TopSortNum2 = Node2->getTopSortNum();

  // Set starting parent using the node which appears earlier in HIR.
  const HLNode *Parent = (TopSortNum1 < TopSortNum2) ? Node1 : Node2;
  unsigned LaterNodeTopSortNum = std::max(TopSortNum1, TopSortNum2);

  // Move up the parent chain until we find a parent wich also contains the node
  // which appears later in HIR.
  while (Parent->getMaxTopSortNum() < LaterNodeTopSortNum) {
    Parent = Parent->getParent();
  }

  return Parent;
}

HLNode *HLNodeUtils::getLexicalLowestCommonAncestor(HLNode *Node1,
                                                    HLNode *Node2) {
  return const_cast<HLNode *>(getLexicalLowestCommonAncestor(
      static_cast<const HLNode *>(Node1), static_cast<const HLNode *>(Node2)));
}

struct PredicateTraits {
  static unsigned getNumPredicates(const HLIf *If) {
    return If->getNumPredicates();
  }

  static HLIf::const_pred_iterator pred_begin(const HLIf *If) {
    return If->pred_begin();
  }

  static HLIf::const_pred_iterator pred_end(const HLIf *If) {
    return If->pred_end();
  }

  static const RegDDRef *
  getLHSPredicateOperandDDRef(const HLIf *If,
                              HLIf::const_pred_iterator CPredI) {
    return If->getLHSPredicateOperandDDRef(CPredI);
  }

  static const RegDDRef *
  getRHSPredicateOperandDDRef(const HLIf *If,
                              HLIf::const_pred_iterator CPredI) {
    return If->getRHSPredicateOperandDDRef(CPredI);
  }
};

struct ZttPredicateTraits {
  static unsigned getNumPredicates(const HLLoop *Loop) {
    return Loop->hasZtt() ? Loop->getNumZttPredicates() : 0;
  }

  static HLLoop::const_ztt_pred_iterator pred_begin(const HLLoop *Loop) {
    return Loop->ztt_pred_begin();
  }

  static HLLoop::const_ztt_pred_iterator pred_end(const HLLoop *Loop) {
    return Loop->ztt_pred_end();
  }

  static const RegDDRef *
  getLHSPredicateOperandDDRef(const HLLoop *Loop,
                              HLLoop::const_ztt_pred_iterator CPredI) {
    return Loop->getLHSZttPredicateOperandDDRef(CPredI);
  }

  static const RegDDRef *
  getRHSPredicateOperandDDRef(const HLLoop *Loop,
                              HLLoop::const_ztt_pred_iterator CPredI) {
    return Loop->getRHSZttPredicateOperandDDRef(CPredI);
  }
};

template <typename T, typename PT>
static bool areEqualConditionsImpl(const T *NodeA, const T *NodeB) {
  auto NumPredicates = PT::getNumPredicates(NodeA);

  if (PT::getNumPredicates(NodeA) != PT::getNumPredicates(NodeB)) {
    return false;
  }

  if (NumPredicates == 0) {
    return true;
  }

  for (auto IA = PT::pred_begin(NodeA), EA = PT::pred_end(NodeA),
            IB = PT::pred_begin(NodeB);
       IA != EA; ++IA, ++IB) {

    if (*IA != *IB) {
      return false;
    }

    if (!DDRefUtils::areEqual(PT::getLHSPredicateOperandDDRef(NodeA, IA),
                              PT::getLHSPredicateOperandDDRef(NodeB, IB)) ||
        !DDRefUtils::areEqual(PT::getRHSPredicateOperandDDRef(NodeA, IA),
                              PT::getRHSPredicateOperandDDRef(NodeB, IB))) {
      return false;
    }
  }

  return true;
}

bool HLNodeUtils::areEqualConditions(const HLIf *NodeA, const HLIf *NodeB) {
  return areEqualConditionsImpl<const HLIf, PredicateTraits>(NodeA, NodeB);
}

bool HLNodeUtils::areEqualZttConditions(const HLLoop *NodeA,
                                        const HLLoop *NodeB) {
  return areEqualConditionsImpl<const HLLoop, ZttPredicateTraits>(NodeA, NodeB);
}

bool HLNodeUtils::areEqualConditions(const HLSwitch *NodeA,
                                     const HLSwitch *NodeB) {
  return DDRefUtils::areEqual(NodeA->getConditionDDRef(),
                              NodeB->getConditionDDRef());
}

bool HLNodeUtils::areEqualConditions(const HLInst *SelectA,
                                     const HLInst *SelectB) {
  assert(isa<SelectInst>(SelectA->getLLVMInstruction()) &&
         "First argument is not a select instruction");
  assert(isa<SelectInst>(SelectB->getLLVMInstruction()) &&
         "Second argument is not a select instruction");

  // Select instructions have 5 DDRef:
  //   0 -> left hand side
  //   1 -> compare operand 1
  //   2 -> compare operand 2
  //   3 -> result if compare is true
  //   4 -> result if compare is false

  auto *OP1A = SelectA->getOperandDDRef(1);
  auto *OP2A = SelectA->getOperandDDRef(2);
  auto &PredA = SelectA->getPredicate();

  auto *OP1B = SelectB->getOperandDDRef(1);
  auto *OP2B = SelectB->getOperandDDRef(2);
  auto &PredB = SelectB->getPredicate();

  return (PredA == PredB && DDRefUtils::areEqual(OP1A, OP1B) &&
          DDRefUtils::areEqual(OP2A, OP2B));
}

bool HLNodeUtils::areEqualConditions(const HLIf *If, const HLInst *Select) {
  assert(isa<SelectInst>(Select->getLLVMInstruction()) &&
       "Select argument is not a select instruction");

  // NOTE: Select instructions have one condition only, therefore the HLIf
  // must have one predicate
  if (If->getNumPredicates() != 1)
    return false;

  auto *SelOP1 = Select->getOperandDDRef(1);
  auto *SelOP2 = Select->getOperandDDRef(2);
  auto &SelPred = Select->getPredicate();

  auto IfPred = If->pred_begin();
  auto *IfOP1 = If->getLHSPredicateOperandDDRef(IfPred);
  auto *IfOP2 = If->getRHSPredicateOperandDDRef(IfPred);

  return (*IfPred == SelPred && DDRefUtils::areEqual(IfOP1, SelOP1) &&
          DDRefUtils::areEqual(IfOP2, SelOP2));
}

bool HLNodeUtils::areEqualConditions(const HLInst *Select, const HLIf *If) {
  return areEqualConditions(If, Select);
}

// Collect the LHS ref, RHS ref and predicate, at position PredPos if the
// predicate position is available.
// NOTE: Perhaps this helper function can be added as a member of HLIf.
static void getIfPredicateOperandsAtPos(const HLIf *If, unsigned PredPos,
                                        RegDDRef **LHS, RegDDRef **RHS,
                                        HLPredicate &Pred) {

  assert(PredPos < If->getNumPredicates() &&
         "Trying to access a predicate outside of the range");

  // Collect the predicate that will be partially unswitched
  auto PredIt = If->pred_begin() + PredPos;
  *LHS = If->getLHSPredicateOperandDDRef(PredIt);
  *RHS = If->getRHSPredicateOperandDDRef(PredIt);
  Pred = *PredIt;
}

bool HLNodeUtils::areEqualConditionsAtPos(const HLIf *IfA, unsigned PosPredA,
                                          const HLIf *IfB, unsigned PosPredB) {

  RegDDRef *LHSRefA = nullptr, *RHSRefA = nullptr;
  HLPredicate PredA;

  RegDDRef *LHSRefB = nullptr, *RHSRefB = nullptr;
  HLPredicate PredB;

  getIfPredicateOperandsAtPos(IfA, PosPredA, &LHSRefA, &RHSRefA, PredA);
  getIfPredicateOperandsAtPos(IfB, PosPredB, &LHSRefB, &RHSRefB, PredB);

  assert(LHSRefA && RHSRefA && "Predicate for IfA collected incorrectly");
  assert(LHSRefB && RHSRefB && "Predicate for IfB collected incorrectly");

  return PredA == PredB && DDRefUtils::areEqual(LHSRefA, LHSRefB) &&
         DDRefUtils::areEqual(RHSRefA, RHSRefB);
}

HLNodeRangeTy HLNodeUtils::replaceNodeWithBody(HLIf *If, bool ThenBody) {

  auto NodeRange = ThenBody ? std::make_pair(If->then_begin(), If->then_end())
                            : std::make_pair(If->else_begin(), If->else_end());
  auto LastNode = std::prev(NodeRange.second);

  // Check for prev/next node is a compile time optimization. See comments in
  // replace().
  // Top sort num can be 0 if utility is called from framework. In that case we
  // cannot use getPrevNode()/getNextNode().
  if (If->getTopSortNum() != 0) {
    if (auto *NextNode = If->getNextNode()) {
      remove(If);
      moveBefore(NextNode, NodeRange.first, NodeRange.second);

      return make_range(NodeRange.first, std::next(LastNode));

    } else if (auto *PrevNode = If->getPrevNode()) {
      remove(If);
      moveAfter(PrevNode, NodeRange.first, NodeRange.second);

      return make_range(NodeRange.first, std::next(LastNode));
    }
  }

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

  // Check for prev/next node is a compile time optimization. See comments in
  // replace().
  // Top sort num can be 0 if utility is called from framework. In that case we
  // cannot use getPrevNode()/getNextNode().
  if (Switch->getTopSortNum() != 0) {
    if (auto *NextNode = Switch->getNextNode()) {
      remove(Switch);
      moveBefore(NextNode, NodeRange.first, NodeRange.second);

      return make_range(NodeRange.first, std::next(LastNode));

    } else if (auto *PrevNode = Switch->getPrevNode()) {
      remove(Switch);
      moveAfter(PrevNode, NodeRange.first, NodeRange.second);

      return make_range(NodeRange.first, std::next(LastNode));
    }
  }

  HLNodeUtils::moveAfter(Switch, NodeRange.first, NodeRange.second);
  HLNodeUtils::remove(Switch);

  return make_range(NodeRange.first, std::next(LastNode));
}

static HLNode *getLastNodeOrLabel(HLNode *Node) {
  HLNode *LastNode = nullptr;
  auto *NodeToRemove = Node->getNextNode();
  if (NodeToRemove && !isa<HLLabel>(NodeToRemove)) {
    auto *NextNode = NodeToRemove->getNextNode();
    while (NextNode && !isa<HLLabel>(NextNode)) {
      NodeToRemove = NextNode;
      NextNode = NextNode->getNextNode();
    }
    LastNode = NodeToRemove;
  }
  return LastNode;
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

      OptReportBuilder &ORBuilder =
          Loop->getHLNodeUtils().getHIRFramework().getORBuilder();

      ORBuilder(*Loop).preserveLostOptReport();

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
    int64_t UpperVal;
    bool HasNonSensicalBounds =
        (Loop->hasSignedIV() && !Loop->isUnknown() &&
         Loop->getUpperCanonExpr()->isIntConstant(&UpperVal) && (UpperVal < 0));

    if (HasNonSensicalBounds || (IsTrivialZtt && !ZttIsTrue)) {
      RedundantLoops++;
      notifyWillRemoveNode(Loop);

      SkipNode = Loop;

      OptReportBuilder &ORBuilder =
          Loop->getHLNodeUtils().getHIRFramework().getORBuilder();

      ORBuilder(*Loop).preserveLostOptReport();

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
    if (!If->isUnknownLoopBottomTest() && If->isKnownPredicate(&IsTrue)) {
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

      OptReportBuilder &ORBuilder =
          Loop->getHLNodeUtils().getHIRFramework().getORBuilder();

      ORBuilder(*Loop).preserveLostOptReport();

      // Do not extract postexit as they will become dead nodes because of goto.
      Loop->replaceByFirstIteration(false);
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

      // Explicitly visit post exit nodes because they will be extracted to the
      // parent loop body, which is already visited.
      if (Loop->hasPostexit()) {
        HLNodeUtils::visitRange(*this, Loop->post_begin(), Loop->post_end());
      }

      EmptyNodeRemoverVisitorImpl::postVisit(Loop);
      return;
    }

    // The loop will stay attached - handle as regular node.
    postVisitImpl(Loop);
  }

  // Remove nodes after IF-stmt if it does not fall through.
  void postVisit(HLIf *If) {
    LastNodeToRemove = nullptr;

    if (HLNodeUtils::hasGotoOnAllBranches(If)) {
      // remove all nodes after IF-stmt up to label or end of linear code.
      LastNodeToRemove = getLastNodeOrLabel(If);
    }

    postVisitImpl(If);
  }

  // Remove nodes after switch-stmt if it does not fall through.
  void postVisit(HLSwitch *Switch) {
    LastNodeToRemove = nullptr;

    if (HLNodeUtils::hasGotoOnAllBranches(Switch)) {
      // remove all nodes after switch up to label or end of linear code.
      LastNodeToRemove = getLastNodeOrLabel(Switch);
    }

    postVisitImpl(Switch);
  }

  template <typename NodeTy> void postVisit(NodeTy *Node) {
    LastNodeToRemove = nullptr;

    postVisitImpl(Node);
  }

  template <typename NodeTy> void postVisitImpl(NodeTy *Node) {
    assert(Node != SkipNode &&
           "Node is removed, should be no further actions.");

    IsJoinNode = true;

    EmptyNodeRemoverVisitorImpl::postVisit(Node);
  }

  static bool containsSideEffect(HLInst *Inst) {
    return Inst->isSideEffectsCallInst();
  }

  static bool containsSideEffect(HLDDNode *Node) {
    for (RegDDRef *Ref :
         make_range(Node->op_ddref_begin(), Node->op_ddref_end())) {
      // Record side effect of LVal memref.
      if (Ref->isMemRef() && Ref->isLval()) {
        return true;
      }
    }

    return false;
  }

  template <typename NodeTy> void recordSideEffectForNode(NodeTy *Node) {
    if (LoopSideEffects.empty()) {
      // Node is not inside any loop.
      return;
    }

    if (!LoopSideEffects.back().second) {
      LoopSideEffects.back().second = containsSideEffect(Node);
    }
  }

  void visit(HLDDNode *Node) {
    recordSideEffectForNode(Node);
    visit(static_cast<HLNode *>(Node));
  }

  void visit(HLInst *Inst) {
    recordSideEffectForNode(Inst);
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

namespace {

class TempDefFinder final : public HLNodeVisitorBase {
  SmallSet<unsigned, 4> &TempSymbases;
  SmallVector<unsigned, 4> FoundTempDefs;

public:
  TempDefFinder(SmallSet<unsigned, 4> &TempSymbases)
      : TempSymbases(TempSymbases) {}

  void visit(const HLNode *Node) {}
  void postVisit(const HLNode *Node) {}

  void visit(const HLInst *Inst);

  SmallVector<unsigned, 4> getFoundTempDefs() const { return FoundTempDefs; }
};

void TempDefFinder::visit(const HLInst *Inst) {
  auto LvalRef = Inst->getLvalDDRef();

  if (!LvalRef || !LvalRef->isTerminalRef()) {
    return;
  }

  unsigned TempDefSB = LvalRef->getSymbase();

  if (TempSymbases.count(TempDefSB)) {
    FoundTempDefs.push_back(TempDefSB);
  }
}

} // namespace

void HLNodeUtils::addCloningInducedLiveouts(HLLoop *LiveoutLoop,
                                            const HLLoop *OrigLoop) {
  // Creation of a new cloned loop (remainder loop, for example) can result in
  // additional liveouts from the lexically first loop. Consider this example
  // where t1 is livein but not liveout of the loop- DO i1
  //   t1 = t1 + ...
  // END DO
  //
  // After creating main and remainder loop, t1 becomes liveout of main loop.
  //
  // DO i1  << main loop
  //   t1 = t1 + ...
  // END DO
  //
  // DO i2  << remainder loop
  //   t1 = t1 + ...
  // END DO

  if (!OrigLoop) {
    OrigLoop = LiveoutLoop;
  }

  SmallSet<unsigned, 4> LiveoutCandidates;

  // Collect liveins which are not liveout of the loop.
  for (auto It = OrigLoop->live_in_begin(), E = OrigLoop->live_in_end();
       It != E; ++It) {
    unsigned Symbase = *It;

    if (!OrigLoop->isLiveOut(Symbase)) {
      LiveoutCandidates.insert(Symbase);
    }
  }

  if (LiveoutCandidates.empty()) {
    return;
  }

  TempDefFinder TDF(LiveoutCandidates);

  HLNodeUtils::visitRange(TDF, OrigLoop->child_begin(), OrigLoop->child_end());

  for (unsigned LiveoutSB : TDF.getFoundTempDefs()) {
    LiveoutLoop->addLiveOutTemp(LiveoutSB);
  }
}

void HLNodeUtils::eliminateRedundantGotos(
    const SmallVectorImpl<HLGoto *> &Gotos, RequiredLabelsTy &RequiredLabels) {
  for (auto *Goto : Gotos) {
    auto TargetLabel = Goto->getTargetLabel();

    HLNode *CurNode = Goto;

    if (!Goto->isAttached()) {
      HLNodeUtils::erase(Goto);
      continue;
    }

    // We either remove Goto as redundant by looking at its control flow
    // successors or link it to its target HLLabel.
    while (1) {
      auto *Successor = HLNodeUtils::getLexicalControlFlowSuccessor(CurNode);

      bool Erase = false, CheckNext = false;

      if (!Successor) {
        auto TargetBB = Goto->getTargetBBlock();
        if (TargetBB == Goto->getParentRegion()->getSuccBBlock()) {
          // Goto is redundant if it has no lexical successor and jumps to
          // region exit.
          Erase = true;
        }
      } else if (auto LabelSuccessor = dyn_cast<HLLabel>(Successor)) {

        if (TargetLabel == LabelSuccessor) {
          // Goto is redundant if its lexical successor is the same as its
          // target.
          Erase = true;
        } else {
          // If successor is a label, goto can still be redundant based on
          // label's successor.
          // Example-
          // goto L1; << This goto is redundant.
          // L2:
          // L1:
          CurNode = Successor;
          CheckNext = true;
        }
      } else if (auto GotoSuccessor = dyn_cast<HLGoto>(Successor)) {
        // If the successor is a goto which also jumps to the same label,
        // this goto is redundant.
        // Example-
        // goto L1; << This goto is redundant.
        // goto L1;
        auto SuccTargetLabel = GotoSuccessor->getTargetLabel();
        if (SuccTargetLabel == TargetLabel) {
          Erase = true;
        }
      }

      if (Erase) {
        HLNodeUtils::erase(Goto);
        break;

      } else if (!CheckNext) {
        if (TargetLabel)
          RequiredLabels.insert(TargetLabel);
        break;
      }
    }
  }
}

// Returns true for nodes that does not fall through on any path.
// E.g. for if:
// if (C1) {
//    ...
//    if (C3) {
//      ...
//      goto L1;
//    } else {
//      goto L2;
//    }
// } else if (C4) {
//    ...
//    if (C5) {
//      ...
//    }
//    ...
//    goto L3;
//  } else {
//    goto L4;
//  }
//  <some code>
//  L1:
//  L2:
//  L3:
//  L4:
//  ...
//
// Here we never reach <some code> after if, so <some code> can be cleaned up.
// Same logic applies for switches as well.
bool HLNodeUtils::hasGotoOnAllBranches(HLNode *Node) {
  assert(Node && "Expect non-null HLNode!\n");

  // Every last branch node must either be goto or another switch/if w/goto.
  // Bailout if inverse.
  if (auto *If = dyn_cast<HLIf>(Node)) {
    auto *IfLastThenChild = If->getLastThenChild();
    auto *IfLastElseChild = If->getLastElseChild();

    if (!IfLastThenChild ||
        (!isa<HLGoto>(IfLastThenChild) &&
         !HLNodeUtils::hasGotoOnAllBranches(IfLastThenChild)))
      return false;

    if (!IfLastElseChild ||
        (!isa<HLGoto>(IfLastElseChild) &&
         !HLNodeUtils::hasGotoOnAllBranches(IfLastElseChild)))
      return false;

    return true;
  } else if (auto *Switch = dyn_cast<HLSwitch>(Node)) {
    for (unsigned I = 1, E = Switch->getNumCases(); I <= E; ++I) {
      auto *LastChild = Switch->getLastCaseChild(I);
      if (!LastChild || (!isa<HLGoto>(LastChild) &&
                         !HLNodeUtils::hasGotoOnAllBranches(LastChild)))
        return false;
    }

    auto *LastChild = Switch->getLastDefaultCaseChild();
    if (!LastChild || (!isa<HLGoto>(LastChild) &&
                       !HLNodeUtils::hasGotoOnAllBranches(LastChild)))
      return false;

    return true;
  }

  return false;
}

bool HLNodeUtils::hasManyLifeTimeIntrinsics(const HLLoop *Loop) {
  const unsigned NumInstsThreshold = 50;

  if (Loop->getNumChildren() < NumInstsThreshold)
    return false;

  // Make sure if the first NumInstsThreshold instructions are
  // lifetime_start intrinsics.
  return std::all_of(Loop->child_begin(),
                      std::next(Loop->child_begin(), NumInstsThreshold),
               [](const HLNode& Node) {
               if (const HLInst* HInst = dyn_cast<HLInst>(&Node)) {
                 Intrinsic::ID Id;
                 return HInst->isIntrinCall(Id)
                   && Id == Intrinsic::lifetime_start;
               }
               return false; });

}
