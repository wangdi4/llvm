//===--- CanonExprUtils.cpp - Implements CanonExprUtils class -------------===//
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
// This file implements CanonExprUtils class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_LoopAnalysis/Utils/BlobUtils.h"

#include "llvm/Support/ErrorHandling.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRParser.h"
#include "llvm/Analysis/TargetTransformInfo.h"

#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

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

unsigned BlobUtils::findTempBlobIndex(unsigned Symbase) const {
  return getHIRParser().findTempBlobIndex(Symbase);
}

unsigned BlobUtils::findOrInsertTempBlobIndex(unsigned Symbase) {
  return getHIRParser().findOrInsertTempBlobIndex(Symbase);
}

unsigned BlobUtils::findOrInsertBlob(BlobTy Blob) {
  assert(!isa<SCEVConstant>(Blob) && "constant blob not expected!");
  return getHIRParser().findOrInsertBlob(Blob, InvalidSymbase);
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

class UndefFinder {
private:
  bool Found;

public:
  UndefFinder() : Found(false) {}

  bool follow(const SCEV *SC) {

    if (BlobUtils::isUndefBlob(SC)) {
      Found = true;
    }

    return !isDone();
  }

  bool foundUndef() const { return Found; }
  bool isDone() const { return foundUndef(); }
};

bool BlobUtils::containsUndef(BlobTy Blob) {
  UndefFinder UF;
  SCEVTraversal<UndefFinder> Finder(UF);
  Finder.visitAll(Blob);

  return UF.foundUndef();
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

bool BlobUtils::isConstantDataBlob(BlobTy Blob, ConstantData **Val) {
  auto UnknownSCEV = dyn_cast<SCEVUnknown>(Blob);

  if (!UnknownSCEV) {
    return false;
  }

  if (auto Const = dyn_cast<ConstantData>(UnknownSCEV->getValue())) {
    if (Val) {
      *Val = Const;
    }
    return true;
  }

  return false;
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

BlobTy BlobUtils::createGlobalVarBlob(GlobalVariable *Global, bool Insert,
                                      unsigned *NewBlobIndex) {
  unsigned Symbase = getHIRParser().getHIRFramework().getNewSymbase();
  return getHIRParser().createBlob(Global, Symbase, Insert, NewBlobIndex);
}

BlobTy BlobUtils::createConstantBlob(Constant *Const, bool Insert,
                                     unsigned *NewBlobIndex) {
  return getHIRParser().createBlob(Const, ConstantSymbase, Insert,
                                   NewBlobIndex);
}

BlobTy BlobUtils::createBlob(int64_t Val, Type *Ty, bool Insert,
                             unsigned *NewBlobIndex) {
  return getHIRParser().createBlob(Val, Ty, Insert, NewBlobIndex);
}

BlobTy BlobUtils::createUndefBlob(Type *Ty, bool Insert,
                                  unsigned *NewBlobIndex) {
  Value *UndefValue = UndefValue::get(Ty);
  return createBlob(UndefValue, ConstantSymbase, Insert, NewBlobIndex);
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

bool BlobUtils::mayContainUDivByZero(BlobTy Blob) {
  bool ContainsUDiv = SCEVExprContains(Blob, [](const SCEV *S) {
    // TODO: May allow divisions if proven to be non-zero.
    // with ScalarEvolution::isKnownNonZero()

    auto *UDiv = dyn_cast<SCEVUDivExpr>(S);
    if (!UDiv) {
      return false;
    }

    auto *RHS = dyn_cast<SCEVConstant>(UDiv->getRHS());
    return !RHS || RHS->isZero();
  });

  return ContainsUDiv;
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

Value *BlobUtils::getTempOrUndefBlobValue(BlobTy Blob) {

  assert((isTempBlob(Blob) || isUndefBlob(Blob)) && "Not Temp nor Undef Blob");
  if (isTempBlob(Blob)) {
    return getTempBlobValue(Blob);
  }
  if (auto *UnknownSCEV = dyn_cast<SCEVUnknown>(Blob)) {
    return UnknownSCEV->getValue();
  }
  if (auto *ConstantSCEV = dyn_cast<SCEVConstant>(Blob)) {
    return ConstantSCEV->getValue();
  }
  llvm_unreachable("Blob should have a value");
  return nullptr;
}

Value *BlobUtils::getTempOrUndefBlobValue(unsigned BlobIndex) const {
  return getTempOrUndefBlobValue(getBlob(BlobIndex));
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
  const TargetTransformInfo *TTI;
  unsigned NumOperations;

public:
  BlobOperationsCounter(const TargetTransformInfo *TTI)
      : TTI(TTI), NumOperations(0) {}

  void visitConstant(const SCEVConstant *Constant) {}

  void visitUnknown(const SCEVUnknown *Unknown) {}

  void visitTruncateExpr(const SCEVTruncateExpr *Trunc) {
    auto Op = Trunc->getOperand();
    if (!TTI || (TTI->getOperationCost(Instruction::Trunc, Trunc->getType(),
                                       Op->getType()) !=
                 TargetTransformInfo::TargetCostConstants::TCC_Free)) {
      ++NumOperations;
    }
    visit(Op);
  }

  void visitZeroExtendExpr(const SCEVZeroExtendExpr *ZExt) {
    auto Op = ZExt->getOperand();
    if (!TTI || (TTI->getOperationCost(Instruction::ZExt, ZExt->getType(),
                                       Op->getType()) !=
                 TargetTransformInfo::TargetCostConstants::TCC_Free)) {
      ++NumOperations;
    }
    visit(Op);
  }

  void visitSignExtendExpr(const SCEVSignExtendExpr *SExt) {
    auto Op = SExt->getOperand();
    if (!TTI || (TTI->getOperationCost(Instruction::SExt, SExt->getType(),
                                       Op->getType()) !=
                 TargetTransformInfo::TargetCostConstants::TCC_Free)) {
      ++NumOperations;
    }
    visit(Op);
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

  void visitSMinExpr(const SCEVSMinExpr *SMin) { visitNAryExpr(SMin); }

  void visitUMinExpr(const SCEVUMinExpr *UMin) { visitNAryExpr(UMin); }

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

unsigned BlobUtils::getNumOperations(BlobTy Blob,
                                     const TargetTransformInfo *TTI) {
  BlobOperationsCounter BOC(TTI);
  BOC.visit(Blob);

  return BOC.getNumOperations();
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

bool BlobUtils::isInstBlob(BlobTy Blob) {
  return isa<Instruction>(cast<SCEVUnknown>(Blob)->getValue());
}

bool BlobUtils::isUMinBlob(BlobTy Blob) {
  return isa<SCEVUMinExpr>(Blob);
}

bool BlobUtils::isUMaxBlob(BlobTy Blob) {
  return isa<SCEVUMaxExpr>(Blob);
}

bool BlobUtils::getMinBlobValue(BlobTy Blob, int64_t &Val) const {
  return getHIRParser().getMinBlobValue(Blob, Val);
}

bool BlobUtils::getMaxBlobValue(BlobTy Blob, int64_t &Val) const {
  return getHIRParser().getMaxBlobValue(Blob, Val);
}
