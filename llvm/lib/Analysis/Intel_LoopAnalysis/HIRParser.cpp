//===----- HIRParser.cpp - Parses SCEVs into CanonExprs -----*- C++ -*-----===//
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
// This file implements the HIRParser pass.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/Operator.h"
#include "llvm/IR/LLVMContext.h"

#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/LoopInfo.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRParser.h"
#include "llvm/Analysis/Intel_LoopAnalysis/LoopFormation.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Passes.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLUtils.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "hir-parser"

INITIALIZE_PASS_BEGIN(HIRParser, "hir-parser", "HIR Parser", false, true)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolution)
INITIALIZE_PASS_DEPENDENCY(HIRCreation)
INITIALIZE_PASS_DEPENDENCY(LoopFormation)
INITIALIZE_PASS_END(HIRParser, "hir-parser", "HIR Parser", false, true)

char HIRParser::ID = 0;

// define the static pointer
HIRParser *HLUtils::HIRParPtr = nullptr;

FunctionPass *llvm::createHIRParserPass() { return new HIRParser(); }

HIRParser::HIRParser() : FunctionPass(ID), CurLevel(0) {
  initializeHIRParserPass(*PassRegistry::getPassRegistry());
}

void HIRParser::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequiredTransitive<LoopInfoWrapperPass>();
  AU.addRequiredTransitive<ScalarEvolution>();
  AU.addRequiredTransitive<HIRCreation>();
  AU.addRequiredTransitive<LoopFormation>();
}

unsigned HIRParser::findBlob(CanonExpr::BlobTy Blob) {
  return CanonExpr::findBlob(Blob);
}

unsigned HIRParser::findOrInsertBlob(CanonExpr::BlobTy Blob) {
  return CanonExpr::findOrInsertBlob(Blob);
}

CanonExpr::BlobTy HIRParser::getBlob(unsigned BlobIndex) {
  return CanonExpr::getBlob(BlobIndex);
}

bool HIRParser::isConstIntBlob(CanonExpr::BlobTy Blob, int64_t *Val) {

  // Check if this Blob is of Constant Type
  const SCEVConstant *SConst = dyn_cast<SCEVConstant>(Blob);
  if (!SConst)
    return false;

  if (Val)
    *Val = getSCEVConstantValue(SConst);

  return true;
}

CanonExpr::BlobTy HIRParser::createBlob(int64_t Val, bool Insert,
                                        unsigned *NewBlobIndex) {
  Type *Int64Type = IntegerType::get(getGlobalContext(), 64);
  auto Blob = SE->getConstant(Int64Type, Val, false);

  if (Insert) {
    unsigned BlobIndex = findOrInsertBlob(Blob);

    if (NewBlobIndex) {
      *NewBlobIndex = BlobIndex;
    }
  }

  return Blob;
}

CanonExpr::BlobTy HIRParser::createAddBlob(CanonExpr::BlobTy LHS,
                                           CanonExpr::BlobTy RHS, bool Insert,
                                           unsigned *NewBlobIndex) {
  assert(LHS && RHS && "Blob cannot by null!");
  unsigned BlobIndex;

  auto Blob = SE->getAddExpr(LHS, RHS);

  if (Insert) {
    BlobIndex = findOrInsertBlob(Blob);

    if (NewBlobIndex) {
      *NewBlobIndex = BlobIndex;
    }
  }

  return Blob;
}

CanonExpr::BlobTy HIRParser::createMinusBlob(CanonExpr::BlobTy LHS,
                                             CanonExpr::BlobTy RHS, bool Insert,
                                             unsigned *NewBlobIndex) {
  assert(LHS && RHS && "Blob cannot by null!");
  unsigned BlobIndex;

  auto Blob = SE->getMinusSCEV(LHS, RHS);

  if (Insert) {
    BlobIndex = findOrInsertBlob(Blob);

    if (NewBlobIndex) {
      *NewBlobIndex = BlobIndex;
    }
  }

  return Blob;
}

CanonExpr::BlobTy HIRParser::createMulBlob(CanonExpr::BlobTy LHS,
                                           CanonExpr::BlobTy RHS, bool Insert,
                                           unsigned *NewBlobIndex) {
  assert(LHS && RHS && "Blob cannot by null!");
  unsigned BlobIndex;

  auto Blob = SE->getMulExpr(LHS, RHS);

  if (Insert) {
    BlobIndex = findOrInsertBlob(Blob);

    if (NewBlobIndex) {
      *NewBlobIndex = BlobIndex;
    }
  }

  return Blob;
}

CanonExpr::BlobTy HIRParser::createUDivBlob(CanonExpr::BlobTy LHS,
                                            CanonExpr::BlobTy RHS, bool Insert,
                                            unsigned *NewBlobIndex) {
  assert(LHS && RHS && "Blob cannot by null!");
  unsigned BlobIndex;

  auto Blob = SE->getUDivExpr(LHS, RHS);

  if (Insert) {
    BlobIndex = findOrInsertBlob(Blob);

    if (NewBlobIndex) {
      *NewBlobIndex = BlobIndex;
    }
  }

  return Blob;
}

CanonExpr::BlobTy HIRParser::createTruncateBlob(CanonExpr::BlobTy Blob,
                                                Type *Ty, bool Insert,
                                                unsigned *NewBlobIndex) {
  assert(Blob && "Blob cannot by null!");
  assert(Ty && "Type cannot by null!");
  unsigned BlobIndex;

  auto NewBlob = SE->getTruncateExpr(Blob, Ty);

  if (Insert) {
    BlobIndex = findOrInsertBlob(NewBlob);

    if (NewBlobIndex) {
      *NewBlobIndex = BlobIndex;
    }
  }

  return NewBlob;
}

CanonExpr::BlobTy HIRParser::createZeroExtendBlob(CanonExpr::BlobTy Blob,
                                                  Type *Ty, bool Insert,
                                                  unsigned *NewBlobIndex) {
  assert(Blob && "Blob cannot by null!");
  assert(Ty && "Type cannot by null!");
  unsigned BlobIndex;

  auto NewBlob = SE->getZeroExtendExpr(Blob, Ty);

  if (Insert) {
    BlobIndex = findOrInsertBlob(NewBlob);

    if (NewBlobIndex) {
      *NewBlobIndex = BlobIndex;
    }
  }

  return NewBlob;
}

CanonExpr::BlobTy HIRParser::createSignExtendBlob(CanonExpr::BlobTy Blob,
                                                  Type *Ty, bool Insert,
                                                  unsigned *NewBlobIndex) {
  assert(Blob && "Blob cannot by null!");
  assert(Ty && "Type cannot by null!");
  unsigned BlobIndex;

  auto NewBlob = SE->getSignExtendExpr(Blob, Ty);

  if (Insert) {
    BlobIndex = findOrInsertBlob(NewBlob);

    if (NewBlobIndex) {
      *NewBlobIndex = BlobIndex;
    }
  }

  return NewBlob;
}

int64_t HIRParser::getSCEVConstantValue(const SCEVConstant *ConstSCEV) const {
  return ConstSCEV->getValue()->getSExtValue();
}

void HIRParser::parseConstant(const SCEVConstant *ConstSCEV, CanonExpr *CE) {
  auto Const = getSCEVConstantValue(ConstSCEV);

  CE->setConstant(CE->getConstant() + Const);
}

/// TODO: Add blob DDRef logic
/// TODO: Handle compound blobs
void HIRParser::parseBlob(const SCEV *BlobSCEV, CanonExpr *CE, unsigned Level) {
  auto Index = findOrInsertBlob(BlobSCEV);
  unsigned DefLevel = 0;
  HLLoop *HLoop;

  CE->addBlob(Index, 1);

  if (auto SingleBlobSCEV = dyn_cast<SCEVUnknown>(BlobSCEV)) {
    /// Skip DefLevel if CanonExpr is already non-linear.
    if (!CE->isNonLinear()) {
      if (auto Inst = dyn_cast<Instruction>(SingleBlobSCEV->getValue())) {
        auto Lp = LI->getLoopFor(Inst->getParent());

        if (Lp && (HLoop = LF->findHLLoop(Lp))) {
          DefLevel = HLoop->getNestingLevel();

          /// Make non-linear
          if (DefLevel == Level) {
            CE->setDefinedAtLevel(-1);
          } else if (DefLevel > (unsigned)CE->getDefinedAtLevel()) {
            CE->setDefinedAtLevel(DefLevel);
          }
        }
      }
    }
  } else {
    assert(false && "Can't handle compound blobs!");
  }
}

/// TODO: Make it recursive
RegDDRef *HIRParser::parseRecursive(const SCEV *SC, const SCEV *ElementSize,
                                    unsigned Level, bool IsErasable,
                                    bool IsTop) {

  if (auto ConstSCEV = dyn_cast<SCEVConstant>(SC)) {
    assert(IsTop && "Can't handle constant embedded in the SCEV tree!");

    auto CE = CanonExprUtils::createCanonExpr(ConstSCEV->getType());
    auto Ref = DDRefUtils::createRegDDRef(0);

    parseConstant(ConstSCEV, CE);
    Ref->addDimension(CE, nullptr);

    return Ref;
  } else if (isa<SCEVUnknown>(SC)) {
    assert(IsTop && "Can't handle blob embedded in the SCEV tree!");

    auto CE = CanonExprUtils::createCanonExpr(SC->getType());
    auto RRef = DDRefUtils::createRegDDRef(0);

    parseBlob(SC, CE, Level);
    RRef->addDimension(CE, nullptr);

    return RRef;
  }
  /// TODO: delinearlize multi-dim arrays
  /// NOTE: AddRec->delinearize() doesn't work with contant bound arrays
  else if (auto RecSCEV = dyn_cast<SCEVAddRecExpr>(SC)) {
    assert(IsTop && "Can't handle embedded AddRecs!");
    assert(RecSCEV->isAffine() && "Can't handle non-affine AddRecs!");

    if (IsErasable && RecSCEV->isAffine()) {
      assert(IsTop && "unexpected condition!");
      return nullptr;
    }

    CanonExpr *StrideCE = nullptr;
    int64_t Denom = 1;
    auto RRef = DDRefUtils::createRegDDRef(0);

    auto Lp = RecSCEV->getLoop();
    auto HLoop = LF->findHLLoop(Lp);

    assert(HLoop && "Non-HIR loop not handled!");

    auto OffsetSCEV = RecSCEV->getOperand(0);
    auto StepSCEV = RecSCEV->getOperand(1);

    if (ElementSize) {
      assert(isa<SCEVConstant>(ElementSize) &&
             "Can't handle non-constant element size!");

      StrideCE = CanonExprUtils::createCanonExpr(ElementSize->getType());
      parseConstant(cast<SCEVConstant>(ElementSize), StrideCE);
      Denom = StrideCE->getConstant();

      auto BaseSCEV = SE->getPointerBase(SC);
      assert(BaseSCEV && "Could not find pointer base!");
      assert(isa<SCEVUnknown>(BaseSCEV) && "Unexpected Pointer base type!");

      auto BaseCE = CanonExprUtils::createCanonExpr(BaseSCEV->getType());

      parseBlob(BaseSCEV, BaseCE, Level);
      RRef->setBaseCE(BaseCE);

      OffsetSCEV = SE->getMinusSCEV(OffsetSCEV, BaseSCEV);
    }

    assert(isa<SCEVConstant>(OffsetSCEV) && isa<SCEVConstant>(StepSCEV) &&
           "Can't handle non-constant base/step in AddRecs!");

    auto IndexCE = CanonExprUtils::createCanonExpr(StepSCEV->getType());
    auto Coeff = getSCEVConstantValue(cast<SCEVConstant>(StepSCEV));

    parseConstant(cast<SCEVConstant>(OffsetSCEV), IndexCE);
    IndexCE->addIV(HLoop->getNestingLevel(), Coeff);

    /// Normalize w.r.t element size.
    IndexCE->setDenominator(Denom);
    CanonExprUtils::simplify(IndexCE);

    RRef->addDimension(IndexCE, StrideCE);
    return RRef;
  }
  /// TODO: Add other types.
  else {
    assert(false && "SCEV type not handled!");
  }

  return nullptr;
}

void HIRParser::parse(HLLoop *HLoop) {

  if (auto Lp = HLoop->getLLVMLoop()) {

    auto BETC = SE->getBackedgeTakenCount(Lp);

    if (!isa<SCEVCouldNotCompute>(BETC)) {

      // Set the lower bound
      // Initialize Lower to 0.
      auto LowerCE = CanonExprUtils::createCanonExpr(BETC->getType());
      auto LowerRef = DDRefUtils::createRegDDRef(0);
      LowerRef->addDimension(LowerCE, nullptr);
      HLoop->setLowerDDRef(LowerRef);

      // Set the stride
      // Initialize Stride to 1.
      auto StrideCE = CanonExprUtils::createCanonExpr(BETC->getType(), 0, 1);
      auto StrideRef = DDRefUtils::createRegDDRef(0);
      StrideRef->addDimension(StrideCE, nullptr);
      HLoop->setStrideDDRef(StrideRef);

      // Set the upper bound
      auto UpperDDRef = parseRecursive(BETC, nullptr, CurLevel, false);
      HLoop->setUpperDDRef(UpperDDRef);
    }
  } else {
    assert(false && "HLLoop doesn't contain LLVM loop!");
  }

  CurLevel++;
}

RegDDRef *HIRParser::createGEPRegDDRef(const SCEV *SC, const SCEV *ElementSize,
                                       const GEPOperator *GEPOp, unsigned Level,
                                       bool IsErasable) {
  assert(!isa<SCEVConstant>(SC) && "Lval is a constant!");
  assert(!isa<SCEVCouldNotCompute>(SC) && "Unexpected condition!");

  RegDDRef *Ref;

  if (isa<SCEVAddRecExpr>(SC)) {
    Ref = parseRecursive(SC, ElementSize, Level, IsErasable);

    if (!Ref) {
      return nullptr;
    }

    Ref->setInBounds(GEPOp->isInBounds());
  } else {
    /// handle SCEVAddExpr later
    assert(false && "Non AddRec GEPs not handled!");
  }

  return Ref;
}

RegDDRef *HIRParser::createRvalDDRef(const Value *Val, unsigned Level) {
  RegDDRef *Ref;

  if (auto LInst = dyn_cast<LoadInst>(Val)) {
    Val = LInst->getPointerOperand();
    auto SC = SE->getSCEV(const_cast<Value *>(Val));
    Ref =
        createGEPRegDDRef(SC, SE->getElementSize(const_cast<LoadInst *>(LInst)),
                          dyn_cast<GEPOperator>(Val), Level, false);
  } else {
    auto SC = SE->getSCEV(const_cast<Value *>(Val));
    Ref = parseRecursive(SC, nullptr, Level, false);
  }

  return Ref;
}

/// TODO: Move to an earlier pass
bool HIRParser::isRegionLiveOut(const Value *Val, bool IsCompare) {

  for (auto I = Val->user_begin(), E = Val->user_end(); I != E; ++I) {
    if (auto UseInst = dyn_cast<Instruction>(*I)) {

      /// TODO: Remove temporary workaround to get rid of compares.
      if (IsCompare) {
        if (!isa<BranchInst>(UseInst)) {
          return true;
        }
        if (!cast<BranchInst>(UseInst)->isConditional()) {
          return true;
        }
      }
      if (!CurRegion->containsBBlock(UseInst->getParent())) {
        return true;
      }
    } else {
      assert(false && "Use is not an instruction!");
    }
  }

  return false;
}

void HIRParser::parse(HLInst *HInst) {
  const Value *Val;
  RegDDRef *Ref;
  const SCEV *ElementSize = nullptr;
  bool IsErasable = true, HasLval = false;
  auto Inst = HInst->getLLVMInstruction();
  unsigned NumRvalOp = HInst->getNumOperands() - 1;
  unsigned Level = CurLevel;

  if (HInst->isInPreheaderOrPostexit()) {
    Level = CurLevel - 1;
  }

  /// Process lval
  if (HInst->hasLval()) {
    HasLval = true;
    Val = Inst;

    if (isRegionLiveOut(Val)) {
      IsErasable = false;
    }

    /// TODO: Remove temporary workaround to get rid of compares.
    if (isa<CmpInst>(Inst)) {
      assert(IsErasable && "Could not eliminate compare instruction!");
      EraseSet.push_back(HInst);
      return;
    } else if (auto SInst = dyn_cast<StoreInst>(Inst)) {
      Val = SInst->getPointerOperand();
      ElementSize = SE->getElementSize(const_cast<StoreInst *>(SInst));
      /// Cannot erase stores.
      IsErasable = false;
    } else if (auto LInst = dyn_cast<LoadInst>(Inst)) {
      /// TODO: Add logic to check whether load is used as a blob which makes
      /// it non-erasable.
      Val = LInst->getPointerOperand();
      ElementSize = SE->getElementSize(const_cast<LoadInst *>(LInst));
    }

    auto SC = SE->getSCEV(const_cast<Value *>(Val));

    if (ElementSize) {
      Ref = createGEPRegDDRef(SC, ElementSize, dyn_cast<GEPOperator>(Val),
                              Level, IsErasable);
    } else {
      Ref = parseRecursive(SC, nullptr, Level, IsErasable);
    }

    if (!Ref) {
      /// Eliminate linear lval instructions.
      /// TODO: eliminate all lvals other than non-affine AddRecs and unknowns
      /// both of which should become blob defs.
      EraseSet.push_back(HInst);
      return;
    }

    assert(!isa<LoadInst>(Inst) && "Non-linear load not handled!");

    HInst->setLvalDDRef(Ref);
  }

  /// Process rvals
  for (unsigned I = 0; I < NumRvalOp; ++I) {
    Val = Inst->getOperand(I);
    Ref = createRvalDDRef(Val, Level);
    assert(Ref && "Ref is null!");

    HInst->setOperandDDRef(Ref, HasLval ? I + 1 : I);
  }
}

void HIRParser::eraseUselessNodes() {
  for (auto &I : EraseSet) {
    HLNodeUtils::erase(I);
  }
}

bool HIRParser::runOnFunction(Function &F) {
  this->Func = &F;
  HIR = &getAnalysis<HIRCreation>();
  LF = &getAnalysis<LoopFormation>();
  SE = &getAnalysis<ScalarEvolution>();
  LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();

  HLUtils::setHIRParserPtr(this);

  Visitor PV(this);
  HLNodeUtils::visitAll(&PV);

  eraseUselessNodes();

  return false;
}

void HIRParser::releaseMemory() {
  /// Destroy all DDRefs and CanonExprs.
  DDRefUtils::destroyAll();
  CanonExprUtils::destroyAll();

  EraseSet.clear();
}

void HIRParser::print(raw_ostream &OS, const Module *M) const {
  /// TODO: implement later
}

void HIRParser::verifyAnalysis() const {
  /// TODO: implement later
}
