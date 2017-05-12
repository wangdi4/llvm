//===--- CanonExprUtils.cpp - Implements CanonExprUtils class -------------===//
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
// This file implements CanonExprUtils class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/ErrorHandling.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRFramework.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/BlobUtils.h"

using namespace llvm;
using namespace loopopt;

Function &BlobUtils::getFunction() const {
  return getHIRParser().getFunction();
}

Module &BlobUtils::getModule() const { return getHIRParser().getModule(); }

LLVMContext &BlobUtils::getContext() const {
  return getHIRParser().getContext();
}

const DataLayout &BlobUtils::getDataLayout() const {
  return getHIRParser().getDataLayout();
}

unsigned BlobUtils::findBlob(BlobTy Blob) {
  return getHIRParser().findBlob(Blob);
}

unsigned BlobUtils::findTempBlobSymbase(BlobTy Blob) {
  return getHIRParser().findTempBlobSymbase(Blob);
}

unsigned BlobUtils::findTempBlobIndex(unsigned Symbase) {
  return getHIRParser().findTempBlobIndex(Symbase);
}

unsigned BlobUtils::findOrInsertTempBlobIndex(unsigned Symbase) {
  return getHIRParser().findOrInsertTempBlobIndex(Symbase);
}

unsigned BlobUtils::findOrInsertBlob(BlobTy Blob) {
  return getHIRParser().findOrInsertBlob(Blob, InvalidBlobIndex);
}

void BlobUtils::mapBlobsToIndices(const SmallVectorImpl<BlobTy> &Blobs,
                                  SmallVectorImpl<unsigned> &Indices) {
  getHIRParser().mapBlobsToIndices(Blobs, Indices);
}

BlobTy BlobUtils::getBlob(unsigned BlobIndex) const {
  return getHIRParser().getBlob(BlobIndex);
}

unsigned BlobUtils::getTempBlobSymbase(unsigned BlobIndex) const {
  return getHIRParser().getTempBlobSymbase(BlobIndex);
}

bool BlobUtils::isBlobIndexValid(unsigned BlobIndex) const {
  return getHIRParser().isBlobIndexValid(BlobIndex);
}

void BlobUtils::printBlob(raw_ostream &OS, BlobTy Blob) const {
  getHIRParser().printBlob(OS, Blob);
}

void BlobUtils::printScalar(raw_ostream &OS, unsigned Symbase) const {
  getHIRParser().printScalar(OS, Symbase);
}

bool BlobUtils::isConstantIntBlob(BlobTy Blob, int64_t *Val) {
  const SCEVConstant *SConst = dyn_cast<SCEVConstant>(Blob);
  if (!SConst) {
    return false;
  }

  if (Val) {
    *Val = SConst->getValue()->getSExtValue();
  }

  return true;
}

bool BlobUtils::isTempBlob(BlobTy Blob) { return HIRParser::isTempBlob(Blob); }

bool BlobUtils::isGuaranteedProperLinear(BlobTy TempBlob) {
  assert(isTempBlob(TempBlob) && "Not a temp blob!");

  auto UnknownSCEV = cast<SCEVUnknown>(TempBlob);

  return !isa<Instruction>(UnknownSCEV->getValue());
}

bool BlobUtils::isUndefBlob(BlobTy Blob) {
  Value *Val = nullptr;

  if (auto *UnknownSCEV = dyn_cast<SCEVUnknown>(Blob)) {
    Val = UnknownSCEV->getValue();
  } else if (auto *ConstantSCEV = dyn_cast<SCEVConstant>(Blob)) {
    Val = ConstantSCEV->getValue();
  } else {
    return false;
  }

  assert(Val && "Blob should have a value");
  return isa<UndefValue>(Val);
}

bool BlobUtils::isConstantFPBlob(BlobTy Blob, ConstantFP **Val) {
  auto UnknownSCEV = dyn_cast<SCEVUnknown>(Blob);

  if (!UnknownSCEV) {
    return false;
  }

  auto FPVal = dyn_cast<ConstantFP>(UnknownSCEV->getValue());

  if (!FPVal) {
    return false;
  }

  if (Val) {
    *Val = FPVal;
  }

  return true;
}

bool BlobUtils::isConstantVectorBlob(BlobTy Blob, Constant **Val) {
  auto UnknownSCEV = dyn_cast<SCEVUnknown>(Blob);

  if (!UnknownSCEV) {
    return false;
  }

  Constant *Const = nullptr;

  if ((Const = dyn_cast<ConstantVector>(UnknownSCEV->getValue())) ||
      (Const = dyn_cast<ConstantDataVector>(UnknownSCEV->getValue())) ||
      (Const = dyn_cast<ConstantAggregateZero>(UnknownSCEV->getValue()))) {
    if (Val) {
      *Val = Const;
    }
    return true;
  }

  return false;
}

bool BlobUtils::isMetadataBlob(BlobTy Blob, MetadataAsValue **Val) {

  auto UnknownSCEV = dyn_cast<SCEVUnknown>(Blob);

  if (!UnknownSCEV) {
    return false;
  }

  auto MetaVal = dyn_cast<MetadataAsValue>(UnknownSCEV->getValue());

  if (!MetaVal) {
    return false;
  }

  if (Val) {
    *Val = MetaVal;
  }

  return true;
}

bool BlobUtils::isSignExtendBlob(BlobTy Blob, BlobTy *Val) {
  if (auto CastSCEV = dyn_cast<SCEVSignExtendExpr>(Blob)) {
    if (Val) {
      *Val = CastSCEV->getOperand();
    }
    return true;
  }

  return false;
}

BlobTy BlobUtils::createBlob(Value *TempVal, unsigned Symbase, bool Insert,
                             unsigned *NewBlobIndex) {
  return getHIRParser().createBlob(TempVal, Symbase, Insert, NewBlobIndex);
}

BlobTy BlobUtils::createBlob(Value *Val, bool Insert, unsigned *NewBlobIndex) {
  return getHIRParser().createBlob(Val, InvalidSymbase, Insert, NewBlobIndex);
}

BlobTy BlobUtils::createBlob(int64_t Val, Type *Ty, bool Insert,
                             unsigned *NewBlobIndex) {
  return getHIRParser().createBlob(Val, Ty, Insert, NewBlobIndex);
}

BlobTy BlobUtils::createUndefBlob(Type *Ty, bool Insert,
                                  unsigned *NewBlobIndex) {
  Value *UndefValue = UndefValue::get(Ty);
  auto Blob = createBlob(UndefValue, false);
  unsigned BlobIndex = findBlob(Blob);

  if (Insert && BlobIndex == InvalidBlobIndex) {
    HIRSymbaseAssignment &HSA = getHIRSymbaseAssignment();
    return createBlob(UndefValue, HSA.getNewSymbase(), true, NewBlobIndex);
  }

  if (NewBlobIndex) {
    *NewBlobIndex = BlobIndex;
  }

  return Blob;
}

BlobTy BlobUtils::createAddBlob(BlobTy LHS, BlobTy RHS, bool Insert,
                                unsigned *NewBlobIndex) {
  return getHIRParser().createAddBlob(LHS, RHS, Insert, NewBlobIndex);
}

BlobTy BlobUtils::createMinusBlob(BlobTy LHS, BlobTy RHS, bool Insert,
                                  unsigned *NewBlobIndex) {
  return getHIRParser().createMinusBlob(LHS, RHS, Insert, NewBlobIndex);
}

BlobTy BlobUtils::createMulBlob(BlobTy LHS, BlobTy RHS, bool Insert,
                                unsigned *NewBlobIndex) {
  return getHIRParser().createMulBlob(LHS, RHS, Insert, NewBlobIndex);
}

BlobTy BlobUtils::createUDivBlob(BlobTy LHS, BlobTy RHS, bool Insert,
                                 unsigned *NewBlobIndex) {
  return getHIRParser().createUDivBlob(LHS, RHS, Insert, NewBlobIndex);
}

BlobTy BlobUtils::createTruncateBlob(BlobTy Blob, Type *Ty, bool Insert,
                                     unsigned *NewBlobIndex) {
  return getHIRParser().createTruncateBlob(Blob, Ty, Insert, NewBlobIndex);
}

BlobTy BlobUtils::createZeroExtendBlob(BlobTy Blob, Type *Ty, bool Insert,
                                       unsigned *NewBlobIndex) {
  return getHIRParser().createZeroExtendBlob(Blob, Ty, Insert, NewBlobIndex);
}

BlobTy BlobUtils::createSignExtendBlob(BlobTy Blob, Type *Ty, bool Insert,
                                       unsigned *NewBlobIndex) {
  return getHIRParser().createSignExtendBlob(Blob, Ty, Insert, NewBlobIndex);
}

BlobTy BlobUtils::createCastBlob(BlobTy Blob, bool IsSExt, Type *Ty,
                                 bool Insert, unsigned *NewBlobIndex) {
  return getHIRParser().createCastBlob(Blob, IsSExt, Ty, Insert, NewBlobIndex);
}

BlobTy BlobUtils::createSMinBlob(BlobTy BlobA, BlobTy BlobB, bool Insert,
                                 unsigned *NewBlobIndex) {
  return getHIRParser().createSMinBlob(BlobA, BlobB, Insert, NewBlobIndex);
}

BlobTy BlobUtils::createSMaxBlob(BlobTy BlobA, BlobTy BlobB, bool Insert,
                                 unsigned *NewBlobIndex) {
  return getHIRParser().createSMaxBlob(BlobA, BlobB, Insert, NewBlobIndex);
}

BlobTy BlobUtils::createUMinBlob(BlobTy BlobA, BlobTy BlobB, bool Insert,
                                 unsigned *NewBlobIndex) {
  return getHIRParser().createUMinBlob(BlobA, BlobB, Insert, NewBlobIndex);
}

BlobTy BlobUtils::createUMaxBlob(BlobTy BlobA, BlobTy BlobB, bool Insert,
                                 unsigned *NewBlobIndex) {
  return getHIRParser().createUMaxBlob(BlobA, BlobB, Insert, NewBlobIndex);
}

bool BlobUtils::contains(BlobTy Blob, BlobTy SubBlob) const {
  return getHIRParser().contains(Blob, SubBlob);
}

void BlobUtils::collectTempBlobs(BlobTy Blob,
                                 SmallVectorImpl<BlobTy> &TempBlobs) const {
  getHIRParser().collectTempBlobs(Blob, TempBlobs);
}

void BlobUtils::collectTempBlobs(unsigned BlobIndex,
                                 SmallVectorImpl<unsigned> &TempBlobIndices) {
  SmallVector<BlobTy, 8> TempBlobs;

  collectTempBlobs(getBlob(BlobIndex), TempBlobs);
  mapBlobsToIndices(TempBlobs, TempBlobIndices);
}

bool BlobUtils::replaceTempBlob(unsigned BlobIndex, unsigned TempIndex,
                                unsigned NewTempIndex, unsigned &NewBlobIndex,
                                int64_t &SimplifiedConstant) {
  return getHIRParser().replaceTempBlob(BlobIndex, TempIndex,
                                        getBlob(NewTempIndex), NewBlobIndex,
                                        SimplifiedConstant);
}

bool BlobUtils::replaceTempBlob(unsigned BlobIndex, unsigned TempIndex,
                                int64_t Constant, unsigned &NewBlobIndex,
                                int64_t &SimplifiedConstant) {
  return getHIRParser().replaceTempBlobByConstant(
      BlobIndex, TempIndex, Constant, NewBlobIndex, SimplifiedConstant);
}

Value *BlobUtils::getTempBlobValue(BlobTy Blob) {
  assert(isTempBlob(Blob) && "Blob is not a temp blob");
  return cast<SCEVUnknown>(Blob)->getValue();
}

Value *BlobUtils::getTempBlobValue(unsigned BlobIndex) const {
  return getTempBlobValue(getBlob(BlobIndex));
}

class NestedBlobChecker {
private:
  unsigned NumSubBlobs;

public:
  NestedBlobChecker() : NumSubBlobs(0) {}

  bool follow(const SCEV *SC) {
    NumSubBlobs++;
    return !isDone();
  }

  bool isDone() const { return isNestedBlob(); }
  bool isNestedBlob() const { return NumSubBlobs > 1; }
};

bool BlobUtils::isNestedBlob(BlobTy Blob) {
  NestedBlobChecker NBC;
  SCEVTraversal<NestedBlobChecker> Collector(NBC);
  Collector.visitAll(Blob);

  return NBC.isNestedBlob();
}

class BlobOperationsCounter : public SCEVVisitor<BlobOperationsCounter> {
private:
  unsigned NumOperations;

public:
  BlobOperationsCounter() : NumOperations(0) {}

  void visitConstant(const SCEVConstant *Constant) {}

  void visitUnknown(const SCEVUnknown *Unknown) {}

  void visitTruncateExpr(const SCEVTruncateExpr *Trunc) {
    ++NumOperations;
    visit(Trunc->getOperand());
  }

  void visitZeroExtendExpr(const SCEVZeroExtendExpr *ZExt) {
    ++NumOperations;
    visit(ZExt->getOperand());
  }

  void visitSignExtendExpr(const SCEVSignExtendExpr *SExt) {
    ++NumOperations;
    visit(SExt->getOperand());
  }

  void visitNAryExpr(const SCEVNAryExpr *NAry) {
    NumOperations += (NAry->getNumOperands() - 1);
    for (const auto *Op : NAry->operands()) {
      visit(Op);
    }
  }

  void visitAddExpr(const SCEVAddExpr *Add) { visitNAryExpr(Add); }

  void visitMulExpr(const SCEVMulExpr *Mul) { visitNAryExpr(Mul); }

  void visitSMaxExpr(const SCEVSMaxExpr *SMax) { visitNAryExpr(SMax); }

  void visitUMaxExpr(const SCEVUMaxExpr *UMax) { visitNAryExpr(UMax); }

  void visitAddRecExpr(const SCEVAddRecExpr *AddRec) {
    llvm_unreachable("AddRec not expected!");
  }

  void visitUDivExpr(const SCEVUDivExpr *Div) {
    ++NumOperations;
    visit(Div->getLHS());
    visit(Div->getRHS());
  }

  void visitCouldNotCompute(const SCEVCouldNotCompute *SC) {
    llvm_unreachable("Could not compute not expected!");
  }

  unsigned getNumOperations() const { return NumOperations; }
};

unsigned BlobUtils::getNumOperations(BlobTy Blob) {
  BlobOperationsCounter BOC;
  BOC.visit(Blob);

  return BOC.getNumOperations();
}

unsigned BlobUtils::getNumOperations(unsigned BlobIndex) const {
  return getNumOperations(getBlob(BlobIndex));
}

bool BlobUtils::getTempBlobMostProbableConstValue(BlobTy Blob, int64_t &Val) {
  Value *BlobVal = getTempBlobValue(Blob);

  PHINode *Phi = dyn_cast<PHINode>(BlobVal);
  if (!Phi || Phi->getNumIncomingValues() < 3) {
    return false;
  }

  typedef DenseMap<Value *, unsigned> HistTy;
  HistTy Hist;
  for (Value *Op : make_range(Phi->value_op_begin(), Phi->value_op_end())) {
    Hist[Op]++;
  }

  auto MinIter = std::max_element(
      Hist.begin(), Hist.end(),
      [](const HistTy::value_type &A, const HistTy::value_type &B) {
        return A.second < B.second;
      });

  assert(MinIter != Hist.end() && "No max element?");

  // Return false if the probability is less then 2/3.
  if (3 * MinIter->second < 2 * Phi->getNumIncomingValues()) {
    return false;
  }

  Value *ProbableValue = MinIter->first;

  ConstantInt *ConstValue = dyn_cast<ConstantInt>(ProbableValue);
  if (!ConstValue) {
    return false;
  }

  Val = ConstValue->getValue().getSExtValue();
  return true;
}

bool BlobUtils::getTempBlobMostProbableConstValue(unsigned BlobIndex,
                                                  int64_t &Val) const {
  return getTempBlobMostProbableConstValue(getBlob(BlobIndex), Val);
}
