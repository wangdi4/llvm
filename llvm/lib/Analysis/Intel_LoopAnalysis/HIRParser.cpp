//===----- HIRParser.cpp - Parses SCEVs into CanonExprs -------------------===//
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
// HIR is parsed on a region by region basis. Parsing is divided into two phases
// for maximum efficiency. The goal of parsing is to produce minimal HIR
// necessary to generate correct code. The two phases are described below-
//
// 1) In phase1 we visit all the HLNodes in the region and parse their operands.
// We only parse essential HLInsts in this phase. Essential HLInsts are the ones
// which cannot be eliminated by parsing at all, e.g. loads and stores. These
// HLInsts form the basis of parsing the rest of HLInsts in phase2. Phase1
// populates two data structures for use in phase2, a) A set of symbases
// required by the essential HLInsts and b) A map of lval symbases and
// associated HLInsts.
//
// 2) Using the two data structures populated by phase1, phase2 parses the rest
// of the required HLInsts and erases useless HLInsts. This process is recursive
// as parsing HLInsts associated with a set of required symbases can expose
// additional required symbases.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/LLVMContext.h"

#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/LoopInfo.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRParser.h"
#include "llvm/Analysis/Intel_LoopAnalysis/RegionIdentification.h"
#include "llvm/Analysis/Intel_LoopAnalysis/LoopFormation.h"
#include "llvm/Analysis/Intel_LoopAnalysis/ScalarSymbaseAssignment.h"
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
INITIALIZE_PASS_DEPENDENCY(ScalarEvolutionWrapperPass)
INITIALIZE_PASS_DEPENDENCY(RegionIdentification)
INITIALIZE_PASS_DEPENDENCY(HIRCreation)
INITIALIZE_PASS_DEPENDENCY(LoopFormation)
INITIALIZE_PASS_DEPENDENCY(ScalarSymbaseAssignment)
INITIALIZE_PASS_END(HIRParser, "hir-parser", "HIR Parser", false, true)

char HIRParser::ID = 0;

FunctionPass *llvm::createHIRParserPass() { return new HIRParser(); }

HIRParser::HIRParser()
    : FunctionPass(ID), CurNode(nullptr), CurRegion(nullptr), CurLevel(0) {
  initializeHIRParserPass(*PassRegistry::getPassRegistry());
}

void HIRParser::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequiredTransitive<LoopInfoWrapperPass>();
  AU.addRequiredTransitive<ScalarEvolutionWrapperPass>();
  AU.addRequiredTransitive<RegionIdentification>();
  AU.addRequiredTransitive<HIRCreation>();
  AU.addRequiredTransitive<LoopFormation>();
  AU.addRequiredTransitive<ScalarSymbaseAssignment>();
}

const Instruction *HIRParser::getCurInst() const {
  if (auto HLoop = dyn_cast<HLLoop>(CurNode)) {
    auto Lp = HLoop->getLLVMLoop();
    auto Latch = Lp->getLoopLatch();

    auto Term = Latch->getTerminator();
    auto BrInst = dyn_cast<BranchInst>(Term);

    assert(BrInst && "Loop latch is not a branch!");

    return cast<Instruction>(BrInst->getCondition());

  } else if (auto HInst = dyn_cast<HLInst>(CurNode)) {
    return HInst->getLLVMInstruction();

  } else if (auto If = dyn_cast<HLIf>(CurNode)) {
    auto BB = HIR->getSrcBBlock(If);
    auto Term = BB->getTerminator();
    auto BrInst = dyn_cast<BranchInst>(Term);

    return cast<Instruction>(BrInst->getCondition());

  } else if (auto Switch = dyn_cast<HLSwitch>(CurNode)) {
    auto BB = HIR->getSrcBBlock(Switch);
    return BB->getTerminator();
  }

  llvm_unreachable("Unexpected CurNode type!");
}

void HIRParser::insertHIRLval(const Value *Lval, unsigned Symbase) {
  ScalarSA->insertHIRLval(Lval, Symbase);
}

bool HIRParser::isConstantIntBlob(CanonExpr::BlobTy Blob, int64_t *Val) const {

  // Check if this Blob is of Constant Type
  const SCEVConstant *SConst = dyn_cast<SCEVConstant>(Blob);
  if (!SConst)
    return false;

  if (Val)
    *Val = getSCEVConstantValue(SConst);

  return true;
}

bool HIRParser::isTempBlob(CanonExpr::BlobTy Blob) const {
  if (auto UnknownSCEV = dyn_cast<SCEVUnknown>(Blob)) {
    Type *Ty;
    Constant *FieldNo;

    if (!UnknownSCEV->isSizeOf(Ty) && !UnknownSCEV->isAlignOf(Ty) &&
        !UnknownSCEV->isOffsetOf(Ty, FieldNo) &&
        !ScalarSA->isConstant(UnknownSCEV->getValue()) &&
        !isMetadataBlob(Blob, nullptr)) {
      return true;
    }
  }

  return false;
}

bool HIRParser::isGuaranteedProperLinear(CanonExpr::BlobTy TempBlob) const {
  assert(isTempBlob(TempBlob) && "Not a temp blob!");

  auto UnknownSCEV = cast<SCEVUnknown>(TempBlob);

  return !isa<Instruction>(UnknownSCEV->getValue());
}

bool HIRParser::isUndefBlob(CanonExpr::BlobTy Blob) const {
  Value *V = nullptr;

  if (auto *UnknownSCEV = dyn_cast<SCEVUnknown>(Blob)) {
    V = UnknownSCEV->getValue();
  } else if (auto *ConstantSCEV = dyn_cast<SCEVConstant>(Blob)) {
    V = ConstantSCEV->getValue();
  } else {
    return false;
  }

  assert(V && "Blob should have a value");
  return isa<UndefValue>(V);
}

bool HIRParser::isConstantFPBlob(CanonExpr::BlobTy Blob,
                                 ConstantFP **Val) const {
  if (auto UnknownSCEV = dyn_cast<SCEVUnknown>(Blob)) {
    if (auto P = dyn_cast<ConstantFP>(UnknownSCEV->getValue())) {
      if (Val) {
        *Val = P;
      }
      return true;
    }
  }

  return false;
}

bool HIRParser::isConstantVectorBlob(CanonExpr::BlobTy Blob,
                                     Constant **Val) const {
  if (auto UnknownSCEV = dyn_cast<SCEVUnknown>(Blob)) {
    if (auto P = dyn_cast<ConstantVector>(UnknownSCEV->getValue())) {
      if (Val) {
        *Val = P;
      }
      return true;
    }
    if (auto P = dyn_cast<ConstantDataVector>(UnknownSCEV->getValue())) {
      if (Val) {
        *Val = P;
      }
      return true;
    }
    if (auto P = dyn_cast<ConstantAggregateZero>(UnknownSCEV->getValue())) {
      if (Val) {
        *Val = P;
      }
      return true;
    }
  }

  return false;
}

bool HIRParser::isMetadataBlob(CanonExpr::BlobTy Blob,
                               MetadataAsValue **Val) const {
  if (auto UnknownSCEV = dyn_cast<SCEVUnknown>(Blob)) {
    if (auto *p = dyn_cast<MetadataAsValue>(UnknownSCEV->getValue())) {
      if (Val) {
        *Val = p;
      }
      return true;
    }
  }

  return false;
}

void HIRParser::insertBlobHelper(CanonExpr::BlobTy Blob, unsigned Symbase,
                                 bool Insert, unsigned *NewBlobIndex) {
  if (Insert) {
    unsigned BlobIndex = CanonExprUtils::findOrInsertBlob(Blob, Symbase);

    if (NewBlobIndex) {
      *NewBlobIndex = BlobIndex;
    }
  }
}

CanonExpr::BlobTy HIRParser::createBlob(Value *Val, unsigned Symbase,
                                        bool Insert, unsigned *NewBlobIndex) {
  assert(Val && "Value cannot be null!");

  auto Blob = SE->getUnknown(Val);

  insertBlobHelper(Blob, Symbase, Insert, NewBlobIndex);

  return Blob;
}

CanonExpr::BlobTy HIRParser::createBlob(int64_t Val, bool Insert,
                                        unsigned *NewBlobIndex) {
  Type *Int64Type = IntegerType::get(getContext(), 64);
  auto Blob = SE->getConstant(Int64Type, Val, false);

  insertBlobHelper(Blob, INVALID_SYMBASE, Insert, NewBlobIndex);

  return Blob;
}

CanonExpr::BlobTy HIRParser::createAddBlob(CanonExpr::BlobTy LHS,
                                           CanonExpr::BlobTy RHS, bool Insert,
                                           unsigned *NewBlobIndex) {
  assert(LHS && RHS && "Blob cannot be null!");

  auto Blob = SE->getAddExpr(LHS, RHS);

  insertBlobHelper(Blob, INVALID_SYMBASE, Insert, NewBlobIndex);

  return Blob;
}

CanonExpr::BlobTy HIRParser::createMinusBlob(CanonExpr::BlobTy LHS,
                                             CanonExpr::BlobTy RHS, bool Insert,
                                             unsigned *NewBlobIndex) {
  assert(LHS && RHS && "Blob cannot be null!");

  auto Blob = SE->getMinusSCEV(LHS, RHS);

  insertBlobHelper(Blob, INVALID_SYMBASE, Insert, NewBlobIndex);

  return Blob;
}

CanonExpr::BlobTy HIRParser::createMulBlob(CanonExpr::BlobTy LHS,
                                           CanonExpr::BlobTy RHS, bool Insert,
                                           unsigned *NewBlobIndex) {
  assert(LHS && RHS && "Blob cannot be null!");

  auto Blob = SE->getMulExpr(LHS, RHS);

  insertBlobHelper(Blob, INVALID_SYMBASE, Insert, NewBlobIndex);

  return Blob;
}

CanonExpr::BlobTy HIRParser::createUDivBlob(CanonExpr::BlobTy LHS,
                                            CanonExpr::BlobTy RHS, bool Insert,
                                            unsigned *NewBlobIndex) {
  assert(LHS && RHS && "Blob cannot be null!");

  auto Blob = SE->getUDivExpr(LHS, RHS);

  insertBlobHelper(Blob, INVALID_SYMBASE, Insert, NewBlobIndex);

  return Blob;
}

CanonExpr::BlobTy HIRParser::createTruncateBlob(CanonExpr::BlobTy Blob,
                                                Type *Ty, bool Insert,
                                                unsigned *NewBlobIndex) {
  assert(Blob && "Blob cannot be null!");
  assert(Ty && "Type cannot be null!");

  auto NewBlob = SE->getTruncateExpr(Blob, Ty);

  insertBlobHelper(NewBlob, INVALID_SYMBASE, Insert, NewBlobIndex);

  return NewBlob;
}

CanonExpr::BlobTy HIRParser::createZeroExtendBlob(CanonExpr::BlobTy Blob,
                                                  Type *Ty, bool Insert,
                                                  unsigned *NewBlobIndex) {
  assert(Blob && "Blob cannot be null!");
  assert(Ty && "Type cannot be null!");

  auto NewBlob = SE->getZeroExtendExpr(Blob, Ty);

  insertBlobHelper(NewBlob, INVALID_SYMBASE, Insert, NewBlobIndex);

  return NewBlob;
}

CanonExpr::BlobTy HIRParser::createSignExtendBlob(CanonExpr::BlobTy Blob,
                                                  Type *Ty, bool Insert,
                                                  unsigned *NewBlobIndex) {
  assert(Blob && "Blob cannot be null!");
  assert(Ty && "Type cannot be null!");

  auto NewBlob = SE->getSignExtendExpr(Blob, Ty);

  insertBlobHelper(NewBlob, INVALID_SYMBASE, Insert, NewBlobIndex);

  return NewBlob;
}

bool HIRParser::contains(CanonExpr::BlobTy Blob, CanonExpr::BlobTy SubBlob) {
  assert(Blob && "Blob cannot be null!");
  assert(SubBlob && "SubBlob cannot be null!");

  return SE->hasOperand(Blob, SubBlob);
}

class HIRParser::TempBlobCollector {
private:
  const HIRParser *HIRP;
  SmallVectorImpl<CanonExpr::BlobTy> &TempBlobs;

public:
  TempBlobCollector(const HIRParser *HIRP,
                    SmallVectorImpl<CanonExpr::BlobTy> &TempBlobs)
      : HIRP(HIRP), TempBlobs(TempBlobs) {}

  ~TempBlobCollector() {}

  bool follow(const SCEV *SC) const {

    if (HIRP->isTempBlob(SC)) {
      TempBlobs.push_back(SC);
    }

    return !isDone();
  }

  bool isDone() const { return false; }
};

void HIRParser::collectTempBlobs(
    CanonExpr::BlobTy Blob,
    SmallVectorImpl<CanonExpr::BlobTy> &TempBlobs) const {
  TempBlobCollector TBC(this, TempBlobs);
  SCEVTraversal<TempBlobCollector> Collector(TBC);
  Collector.visitAll(Blob);
}

unsigned HIRParser::getMaxScalarSymbase() const {
  return ScalarSA->getMaxScalarSymbase();
}

struct HIRParser::Phase1Visitor final : public HLNodeVisitorBase {
  HIRParser *HIRP;

  Phase1Visitor(HIRParser *Parser) : HIRP(Parser) {}

  void visit(HLRegion *Reg) { HIRP->parse(Reg); }
  void postVisit(HLRegion *Reg) { HIRP->postParse(Reg); }

  void visit(HLLoop *HLoop) { HIRP->parse(HLoop); }
  void postVisit(HLLoop *HLoop) { HIRP->postParse(HLoop); }

  void visit(HLIf *If) { HIRP->parse(If); }
  void postVisit(HLIf *If) { HIRP->postParse(If); }

  void visit(HLSwitch *Switch) { HIRP->parse(Switch); }
  void postVisit(HLSwitch *Switch) { HIRP->postParse(Switch); }

  void visit(HLInst *HInst) { HIRP->parse(HInst, true, 0); }
  void visit(HLLabel *Label) { HIRP->parse(Label); }
  void visit(HLGoto *Goto) { HIRP->parse(Goto); }
};

class HIRParser::BaseSCEVCreator
    : public SCEVVisitor<BaseSCEVCreator, const SCEV *> {
private:
  const HIRParser *HIRP;

public:
  BaseSCEVCreator(const HIRParser *HIRP) : HIRP(HIRP) {}

  const SCEV *visitConstant(const SCEVConstant *Const) { return Const; }

  const SCEV *visitTruncateExpr(const SCEVTruncateExpr *Trunc) {
    const SCEV *Operand = visit(Trunc->getOperand());
    return HIRP->SE->getTruncateExpr(Operand, Trunc->getType());
  }

  const SCEV *visitZeroExtendExpr(const SCEVZeroExtendExpr *ZExt) {
    const SCEV *Operand = ZExt->getOperand();

    // In some cases we have a value for zero extension of linear SCEV but not
    // the linear SCEV itself because the original src code IV has been widened
    // by induction variable simplification. So we look for such values here.
    auto AddRec = dyn_cast<SCEVAddRecExpr>(Operand);

    if (AddRec) {
      if (auto SubSCEV = getSubstituteSCEV(ZExt)) {
        return SubSCEV;
      }
    }

    return HIRP->SE->getZeroExtendExpr(visit(Operand), ZExt->getType());
  }

  const SCEV *visitSignExtendExpr(const SCEVSignExtendExpr *SExt) {
    const SCEV *Operand = visit(SExt->getOperand());
    return HIRP->SE->getSignExtendExpr(Operand, SExt->getType());
  }

  const SCEV *visitAddExpr(const SCEVAddExpr *Add) {
    SmallVector<const SCEV *, 2> Operands;

    for (unsigned I = 0, E = Add->getNumOperands(); I < E; ++I) {
      Operands.push_back(visit(Add->getOperand(I)));
    }

    return HIRP->SE->getAddExpr(Operands);
  }

  const SCEV *visitMulExpr(const SCEVMulExpr *Mul) {
    SmallVector<const SCEV *, 2> Operands;
    unsigned NumOp = Mul->getNumOperands();

    // This is to catch cases like this-
    //
    // %126 = trunc i64 %indvars.iv857 to i32
    //   -->  {0,+,2}<%for.body.525>
    // %rem530815 = and i32 %126, 30
    //   -->  (2 * (zext i4 {0,+,1}<%for.body.525> to i32))
    //
    // TODO: investigate SCEV representation of bitwise operators in detail.
    if (NumOp == 2) {
      auto ZExt = dyn_cast<SCEVZeroExtendExpr>(Mul->getOperand(1));

      if (ZExt && isa<SCEVAddRecExpr>(ZExt->getOperand())) {
        if (auto SubSCEV = getSubstituteSCEV(Mul)) {
          return SubSCEV;
        }
      }
    }

    for (unsigned I = 0; I < NumOp; ++I) {
      Operands.push_back(visit(Mul->getOperand(I)));
    }

    return HIRP->SE->getMulExpr(Operands);
  }

  const SCEV *visitUDivExpr(const SCEVUDivExpr *UDiv) {
    return HIRP->SE->getUDivExpr(visit(UDiv->getLHS()), visit(UDiv->getRHS()));
  }

  const SCEV *visitSMaxExpr(const SCEVSMaxExpr *SMax) {
    SmallVector<const SCEV *, 2> Operands;

    for (unsigned I = 0, E = SMax->getNumOperands(); I < E; ++I) {
      Operands.push_back(visit(SMax->getOperand(I)));
    }

    return HIRP->SE->getSMaxExpr(Operands);
  }

  const SCEV *visitUMaxExpr(const SCEVUMaxExpr *UMax) {
    SmallVector<const SCEV *, 2> Operands;

    for (unsigned I = 0, E = UMax->getNumOperands(); I < E; ++I) {
      Operands.push_back(visit(UMax->getOperand(I)));
    }

    return HIRP->SE->getUMaxExpr(Operands);
  }

  /// Returns the SCEVUnknown version of the value which represents this AddRec.
  const SCEV *visitAddRecExpr(const SCEVAddRecExpr *AddRec) {
    const SCEV *SubSCEV = getSubstituteSCEV(AddRec);
    assert(SubSCEV && "Instuction corresponding to linear SCEV not found!");

    return SubSCEV;
  }

  /// Returns the SCEV of the base value associated with the incoming SCEV's
  /// value.
  const SCEV *visitUnknown(const SCEVUnknown *Unknown) {
    if (HIRP->isTempBlob(Unknown)) {
      const Value *Val = Unknown->getValue();

      const Value *BaseVal = HIRP->ScalarSA->getBaseScalar(Val);

      if (BaseVal != Val) {
        return HIRP->SE->getUnknown(const_cast<Value *>(BaseVal));
      }
    }

    return Unknown;
  }

  const SCEV *visitCouldNotCompute(const SCEVCouldNotCompute *Expr) {
    llvm_unreachable("SCEVCouldNotCompute encountered!");
  }

  /// Returns a substitute SCEV for SC. Returns null if it cannot do so.
  const SCEV *getSubstituteSCEV(const SCEV *SC);

  /// Recursive function to trace back from the current instruction to find an
  /// instruction which can represent SC with a combination of basic operations
  /// like truncation, negation etc applied on top of the SCEV. We are trying to
  /// reverse engineer SCEV analysis here.
  const Instruction *findOrigInst(const Instruction *CurInst, const SCEV *SC,
                                  bool *IsTruncation, bool *IsNegation,
                                  SCEVConstant **ConstMultiplier,
                                  SCEV **Additive) const;

  /// Returns true if NewSCEV can replace OrigSCEV in the SCEV tree with a
  /// combination of basic operations like truncation, negation etc applied on
  /// top of NewSCEV. To replace a linear AddRec type OrigSCEV, NewSCEV should
  /// have identical operands (except the fist operand) and have identical or
  /// stronger wrap flags.
  bool isReplacable(const SCEV *OrigSCEV, const SCEV *NewSCEV,
                    bool *IsTruncation, bool *IsNegation,
                    SCEVConstant **ConstMultiplier, SCEV **Additive) const;

  /// Implements AddRec specific checks for replacement.
  bool isReplacableAddRec(const SCEVAddRecExpr *OrigAddRec,
                          const SCEVAddRecExpr *NewAddRec,
                          SCEVConstant **ConstMultiplier,
                          SCEV **Additive) const;
};

const SCEV *HIRParser::BaseSCEVCreator::getSubstituteSCEV(const SCEV *SC) {
  const Instruction *OrigInst = nullptr;
  SCEV *Additive = nullptr;
  SCEVConstant *ConstMultiplier = nullptr;
  bool IsNegation = false;
  bool IsTruncation = false;

  OrigInst = findOrigInst(nullptr, SC, &IsTruncation, &IsNegation,
                          &ConstMultiplier, &Additive);

  if (!OrigInst) {
    return nullptr;
  }

  auto NewSCEV = HIRP->SE->getUnknown(const_cast<Instruction *>(OrigInst));

  if (IsTruncation) {
    assert(!IsNegation && !ConstMultiplier && !Additive &&
           "Unexpected substitute SCEV!");
    NewSCEV = HIRP->SE->getTruncateExpr(NewSCEV, SC->getType());

  } else {
    // NOTE: The order of negation, multiplication and addition matters.
    if (IsNegation) {
      NewSCEV = HIRP->SE->getNegativeSCEV(NewSCEV);
    }

    if (ConstMultiplier) {
      NewSCEV = HIRP->SE->getMulExpr(ConstMultiplier, NewSCEV);
    }

    if (Additive) {
      NewSCEV = HIRP->SE->getAddExpr(Additive, NewSCEV);
    }
  }

  // Convert value to base value before returning.
  return visit(NewSCEV);
}

const Instruction *HIRParser::BaseSCEVCreator::findOrigInst(
    const Instruction *CurInst, const SCEV *SC, bool *IsTruncation,
    bool *IsNegation, SCEVConstant **ConstMultiplier, SCEV **Additive) const {

  bool IsLiveInCopy = false;

  if (!CurInst) {
    CurInst = HIRP->getCurInst();
    IsLiveInCopy = HIRP->SE->isHIRLiveInCopyInst(CurInst);
  }

  // We should not be checking the SCEV of the livein copy instruction as it
  // should inherit the SCEV of the rval.
  if (!IsLiveInCopy && HIRP->SE->isSCEVable(CurInst->getType())) {
    auto CurSCEV = HIRP->SE->getSCEV(const_cast<Instruction *>(CurInst));

    if (isReplacable(SC, CurSCEV, IsTruncation, IsNegation, ConstMultiplier,
                     Additive)) {
      return CurInst;
    }
  }

  auto ParentBB = CurInst->getParent();
  Loop *Lp = nullptr;

  // Is this a phi node that occurs in loop header?
  bool IsHeaderPhi = isa<PHINode>(CurInst) &&
                     (Lp = HIRP->LI->getLoopFor(ParentBB)) &&
                     (Lp->getHeader() == ParentBB);

  for (auto I = CurInst->op_begin(), E = CurInst->op_end(); I != E; ++I) {

    auto OPInst = dyn_cast<Instruction>(I);

    if (!OPInst) {
      continue;
    }

    // Avoid cycles while tracing back.
    if (IsHeaderPhi &&
        HIRP->LI->getLoopFor(ParentBB) ==
            HIRP->LI->getLoopFor(OPInst->getParent())) {
      continue;
    }

    // Limit trace back to these instruction types. They roughly correspond to
    // instruction types in SE->createSCEV().
    if (!isa<BinaryOperator>(OPInst) && !isa<CastInst>(OPInst) &&
        !isa<GetElementPtrInst>(OPInst) && !isa<PHINode>(OPInst) &&
        !isa<SelectInst>(OPInst)) {
      continue;
    }

    auto OrigInst = findOrigInst(OPInst, SC, IsTruncation, IsNegation,
                                 ConstMultiplier, Additive);

    if (OrigInst) {
      return OrigInst;
    }
  }

  return nullptr;
}

bool HIRParser::BaseSCEVCreator::isReplacable(
    const SCEV *OrigSCEV, const SCEV *NewSCEV, bool *IsTruncation,
    bool *IsNegation, SCEVConstant **ConstMultiplier, SCEV **Additive) const {

  // We got an exact match.
  if (NewSCEV == OrigSCEV) {
    return true;
  }

  auto OrigAddRec = dyn_cast<SCEVAddRecExpr>(OrigSCEV);
  auto NewAddRec = dyn_cast<SCEVAddRecExpr>(NewSCEV);

  // TODO: extend for other SCEV types.
  if (!OrigAddRec || !NewAddRec) {
    return false;
  }

  // Not an exact match, continue matching loop and operands.
  if (NewAddRec->getLoop() != OrigAddRec->getLoop()) {
    return false;
  }

  if (NewAddRec->getNumOperands() != OrigAddRec->getNumOperands()) {
    return false;
  }

  Type *NewType = NewAddRec->getType();
  Type *OrigType = OrigAddRec->getType();

  // When a IV is ANDed with a constant whose value is (2^N - 1) it is
  // transformed by SCEV into a truncated IV with type 'iN' and then zero
  // extended again. For example-
  // %rem82 = and i32 %add79, 31
  // -->  (zext i5 {{0,+,1}<%for.cond.43.preheader>,+,1}<%for.inc.87> to i32)
  //
  // To catch this case we need to compare the truncated form of NewSCEV.
  if (NewType != OrigType) {

    if (!NewType->isIntegerTy() || !OrigType->isIntegerTy()) {
      return false;
    }

    if (NewType->getPrimitiveSizeInBits() <
        OrigType->getPrimitiveSizeInBits()) {
      return false;
    }

    NewAddRec =
        cast<SCEVAddRecExpr>(HIRP->SE->getTruncateExpr(NewAddRec, OrigType));

    if (isReplacableAddRec(OrigAddRec, NewAddRec, nullptr, nullptr)) {
      *IsTruncation = true;
      return true;
    }

    return false;
  }

  if (isReplacableAddRec(OrigAddRec, NewAddRec, ConstMultiplier, Additive)) {
    return true;
  }

  // If the IV of an outer loop is used as the initial value of the inner loop
  // it is negated during the backedge calculation for the inner loop.
  // Negation is also used during min(a, b) construction. This pattern is found
  // with a cmp and select instruction. min(a, b) is constructed as: (-1 + -1 *
  // max(-a-1, -b-1)).
  // Therefore, to find a substitute we need to test the negation too.
  NewAddRec = cast<SCEVAddRecExpr>(HIRP->SE->getNegativeSCEV(NewAddRec));

  if (isReplacableAddRec(OrigAddRec, NewAddRec, ConstMultiplier, Additive)) {
    *IsNegation = true;
    return true;
  }

  return false;
}

bool HIRParser::BaseSCEVCreator::isReplacableAddRec(
    const SCEVAddRecExpr *OrigAddRec, const SCEVAddRecExpr *NewAddRec,
    SCEVConstant **ConstMultiplier, SCEV **Additive) const {

  const SCEVConstant *Mul = nullptr;
  const SCEV *Add = nullptr;

  // Get constant multiplier, if any.
  if (NewAddRec->getOperand(1) != OrigAddRec->getOperand(1)) {

    if (!ConstMultiplier) {
      return false;
    }

    auto NewOp = dyn_cast<SCEVConstant>(NewAddRec->getOperand(1));
    auto OrigOp = dyn_cast<SCEVConstant>(OrigAddRec->getOperand(1));

    if (!NewOp || !OrigOp) {
      return false;
    }

    Mul = cast<SCEVConstant>(HIRP->SE->getConstant(cast<ConstantInt>(
        ConstantExpr::getSDiv(OrigOp->getValue(), NewOp->getValue()))));

    NewAddRec = cast<SCEVAddRecExpr>(HIRP->SE->getMulExpr(NewAddRec, Mul));
  }

  // Get invariant additive, if any.
  if (NewAddRec->getOperand(0) != OrigAddRec->getOperand(0)) {

    if (!Additive) {
      return false;
    }

    Add = HIRP->SE->getMinusSCEV(OrigAddRec->getOperand(0),
                                 NewAddRec->getOperand(0));
  }

  // Now match operands
  for (unsigned I = 1, E = NewAddRec->getNumOperands(); I < E; ++I) {
    if (NewAddRec->getOperand(I) != OrigAddRec->getOperand(I)) {
      return false;
    }
  }

  // Now we look for identical or stricter wrap flags on NewAddRec.

  // If OrigAddRec has NUW, NewAddRec should have it too.
  if (OrigAddRec->getNoWrapFlags(SCEV::FlagNUW) &&
      !NewAddRec->getNoWrapFlags(SCEV::FlagNUW)) {
    return false;
  }

  // If OrigAddRec has NSW, NewAddRec should have it too.
  if (OrigAddRec->getNoWrapFlags(SCEV::FlagNSW) &&
      !NewAddRec->getNoWrapFlags(SCEV::FlagNSW)) {
    return false;
  }

  // If OrigAddRec has NW, NewAddRec can cover it with any of NUW, NSW or NW.
  if (OrigAddRec->getNoWrapFlags(SCEV::FlagNW) &&
      !NewAddRec->getNoWrapFlags(
          (SCEV::NoWrapFlags)(SCEV::FlagNUW | SCEV::FlagNSW | SCEV::FlagNW))) {
    return false;
  }

  if (Mul) {
    *ConstMultiplier = const_cast<SCEVConstant *>(Mul);
  }

  if (Add) {
    *Additive = const_cast<SCEV *>(Add);
  }

  return true;
}

class HIRParser::BlobLevelSetter {
private:
  HIRParser *HIRP;
  CanonExpr *CExpr;
  unsigned Level;

public:
  BlobLevelSetter(HIRParser *Par, CanonExpr *CE, unsigned NestingLevel)
      : HIRP(Par), CExpr(CE), Level(NestingLevel) {}
  ~BlobLevelSetter() {}

  bool follow(const SCEV *SC) const {

    assert(!isa<SCEVAddRecExpr>(SC) && "AddRec found inside blob!");

    if (HIRP->isTempBlob(SC)) {
      HIRP->setTempBlobLevel(cast<SCEVUnknown>(SC), CExpr, Level);
    }

    return !isDone();
  }

  bool isDone() const { return false; }
};

void HIRParser::printScalar(raw_ostream &OS, unsigned Symbase) const {
  ScalarSA->getBaseScalar(Symbase)->printAsOperand(OS, false);
}

void HIRParser::printBlob(raw_ostream &OS, CanonExpr::BlobTy Blob) const {

  if (isa<SCEVConstant>(Blob)) {
    OS << *Blob;

  } else if (auto CastSCEV = dyn_cast<SCEVCastExpr>(Blob)) {
    auto SrcType = CastSCEV->getOperand()->getType();
    auto DstType = CastSCEV->getType();

    if (isa<SCEVZeroExtendExpr>(CastSCEV)) {
      OS << "zext.";
    } else if (isa<SCEVSignExtendExpr>(CastSCEV)) {
      OS << "sext.";
    } else if (isa<SCEVTruncateExpr>(CastSCEV)) {
      OS << "trunc.";
    } else {
      llvm_unreachable("Unexptected casting operation!");
    }

    OS << *SrcType << "." << *DstType << "(";
    printBlob(OS, CastSCEV->getOperand());
    OS << ")";

  } else if (auto NArySCEV = dyn_cast<SCEVNAryExpr>(Blob)) {
    const char *OpStr;

    if (isa<SCEVAddExpr>(NArySCEV)) {
      OS << "(";
      OpStr = " + ";
    } else if (isa<SCEVMulExpr>(NArySCEV)) {
      OS << "(";
      OpStr = " * ";
    } else if (isa<SCEVSMaxExpr>(NArySCEV)) {
      OS << "smax(";
      OpStr = ", ";
    } else if (isa<SCEVUMaxExpr>(NArySCEV)) {
      OS << "umax(";
      OpStr = ", ";
    } else {
      llvm_unreachable("Blob contains AddRec!");
    }

    for (auto I = NArySCEV->op_begin(), E = NArySCEV->op_end(); I != E; ++I) {
      printBlob(OS, *I);

      if (std::next(I) != E) {
        OS << OpStr;
      }
    }
    OS << ")";

  } else if (auto UDivSCEV = dyn_cast<SCEVUDivExpr>(Blob)) {
    OS << "(";
    printBlob(OS, UDivSCEV->getLHS());
    OS << " /u ";
    printBlob(OS, UDivSCEV->getRHS());
    OS << ")";
  } else if (isa<SCEVUnknown>(Blob)) {
    OS << *Blob;
  } else {
    llvm_unreachable("Unknown Blob type!");
  }
}

bool HIRParser::isRegionLiveOut(const Instruction *Inst) const {
  auto Symbase = ScalarSA->getScalarSymbase(Inst);

  if (Symbase && CurRegion->isLiveOut(Symbase)) {
    return true;
  }

  return false;
}

bool HIRParser::isEssential(const Instruction *Inst) const {
  bool Ret = false;

  // TODO: Add exception handling and other miscellaneous instruction types
  // later.
  if (isa<LoadInst>(Inst) || isa<StoreInst>(Inst) || isa<CallInst>(Inst)) {
    Ret = true;
  } else if (isRegionLiveOut(Inst)) {
    Ret = true;
  } else if (!SE->isSCEVable(Inst->getType())) {
    Ret = true;
  }

  return Ret;
}

int64_t HIRParser::getSCEVConstantValue(const SCEVConstant *ConstSCEV) const {
  return ConstSCEV->getValue()->getSExtValue();
}

void HIRParser::parseConstOrDenom(const SCEVConstant *ConstSCEV, CanonExpr *CE,
                                  bool IsDenom) {
  if (isUndefBlob(ConstSCEV)) {
    CE->setContainsUndef();
  }

  auto Const = getSCEVConstantValue(ConstSCEV);

  if (IsDenom) {
    assert((CE->getDenominator() == 1) &&
           "Attempt to overwrite non-unit denominator!");
    CE->setDenominator(Const);
  } else {
    CE->setConstant(CE->getConstant() + Const);
  }
}

void HIRParser::parseConstant(const SCEVConstant *ConstSCEV, CanonExpr *CE) {
  parseConstOrDenom(ConstSCEV, CE, false);
}

void HIRParser::parseDenominator(const SCEVConstant *ConstSCEV, CanonExpr *CE) {
  parseConstOrDenom(ConstSCEV, CE, true);
}

void HIRParser::setCanonExprDefLevel(CanonExpr *CE, unsigned NestingLevel,
                                     unsigned DefLevel) const {
  // If the CE is already non-linear, DefinedAtLevel cannot be refined any
  // further.
  if (!CE->isNonLinear()) {
    if (DefLevel >= NestingLevel) {
      // Make non-linear instead.
      CE->setNonLinear();
    } else if (DefLevel > CE->getDefinedAtLevel()) {
      CE->setDefinedAtLevel(DefLevel);
    }
  }
}

void HIRParser::addTempBlobEntry(unsigned Index, unsigned NestingLevel,
                                 unsigned DefLevel) {
  // -1 indicates non-linear blob
  int Level = (DefLevel >= NestingLevel) ? -1 : DefLevel;

  CurTempBlobLevelMap.insert(std::make_pair(Index, Level));
}

unsigned HIRParser::findOrInsertBlobWrapper(CanonExpr::BlobTy Blob,
                                            unsigned *SymbasePtr) {
  unsigned Symbase = INVALID_SYMBASE;

  if (isTempBlob(Blob)) {
    auto Temp = cast<SCEVUnknown>(Blob)->getValue();
    Symbase = ScalarSA->getOrAssignScalarSymbase(Temp);
  }

  if (SymbasePtr) {
    *SymbasePtr = Symbase;
  }

  return CanonExprUtils::findOrInsertBlob(Blob, Symbase);
}

void HIRParser::setTempBlobLevel(const SCEVUnknown *TempBlobSCEV, CanonExpr *CE,
                                 unsigned NestingLevel) {
  unsigned DefLevel = 0;
  HLLoop *HLoop;

  auto Temp = TempBlobSCEV->getValue();
  unsigned Symbase;
  auto Index = findOrInsertBlobWrapper(TempBlobSCEV, &Symbase);

  if (auto Inst = dyn_cast<Instruction>(Temp)) {
    auto Lp = LI->getLoopFor(Inst->getParent());

    if (Lp && (HLoop = LF->findHLLoop(Lp))) {
      DefLevel = HLoop->getNestingLevel();

    } else if (!CurRegion->containsBBlock(Inst->getParent())) {
      // Blob lies outside the region.

      // Add it as a livein temp.
      CurRegion->addLiveInTemp(Symbase, Temp);

      // Workaround to mark blob as linear even if the nesting level is zero.
      NestingLevel++;
    }
  } else {
    // Blob is some global value. Global values are not marked livein.
    // Workaround to mark blob as linear even if the nesting level is zero.
    NestingLevel++;
  }

  setCanonExprDefLevel(CE, NestingLevel, DefLevel);

  // Cache blob level for later reuse in population of BlobDDRefs for this
  // RegDDRef.
  addTempBlobEntry(Index, NestingLevel, DefLevel);

  // Basically this is not so good place to handle UndefValues, but this is done
  // here to avoid additional traverse of SCEV to found undefined its parts.
  if (isUndefBlob(TempBlobSCEV)) {
    CE->setContainsUndef();
  }

  // Add blob symbase as required.
  RequiredSymbases.insert(Symbase);
}

void HIRParser::breakConstantMultiplierBlob(CanonExpr::BlobTy Blob,
                                            int64_t *Multiplier,
                                            CanonExpr::BlobTy *NewBlob) {

  if (auto MulSCEV = dyn_cast<SCEVMulExpr>(Blob)) {
    for (auto I = MulSCEV->op_begin(), E = MulSCEV->op_end(); I != E; ++I) {
      auto ConstSCEV = dyn_cast<SCEVConstant>(*I);

      if (!ConstSCEV) {
        continue;
      }

      *Multiplier = getSCEVConstantValue(ConstSCEV);
      *NewBlob = SE->getUDivExactExpr(Blob, ConstSCEV);
      return;
    }
  }

  *Multiplier = 1;
  *NewBlob = Blob;
  return;
}

void HIRParser::parseBlob(CanonExpr::BlobTy Blob, CanonExpr *CE, unsigned Level,
                          unsigned IVLevel) {
  int64_t Multiplier;
  unsigned Index;

  // Create base version of the blob.
  BaseSCEVCreator BSC(this);
  Blob = BSC.visit(Blob);

  CanonExpr::BlobTy NewBlob;
  breakConstantMultiplierBlob(Blob, &Multiplier, &NewBlob);

  Index = findOrInsertBlobWrapper(NewBlob);

  if (IVLevel) {
    assert(!CE->hasIVConstCoeff(IVLevel) && !CE->hasIVBlobCoeff(IVLevel) &&
           "Canon Expr already has a coeff for this IV!");
    CE->setIVCoeff(IVLevel, Index, Multiplier);

  } else {
    CE->addBlob(Index, Multiplier);
  }

  // Set defined at level.
  BlobLevelSetter BLS(this, CE, Level);
  SCEVTraversal<BlobLevelSetter> LevelSetter(BLS);
  LevelSetter.visitAll(Blob);
}

void HIRParser::parseRecursive(const SCEV *SC, CanonExpr *CE, unsigned Level,
                               bool IsTop, bool UnderCast) {
  if (auto ConstSCEV = dyn_cast<SCEVConstant>(SC)) {
    parseConstant(ConstSCEV, CE);
  } else if (isa<SCEVUnknown>(SC)) {
    parseBlob(SC, CE, Level);
  } else if (auto CastSCEV = dyn_cast<SCEVCastExpr>(SC)) {

    // Look ahead and check if this zero extension cast contains a non-generable
    // IV inside. We need to parse the top most convert as a blob to aviod cases
    // where the linear SCEV has no corresponding value associated with it due
    // to IV widening.
    if (isa<SCEVZeroExtendExpr>(CastSCEV)) {
      const SCEV *Operand = CastSCEV->getOperand();
      auto RecSCEV = dyn_cast<SCEVAddRecExpr>(Operand);

      if (RecSCEV && RecSCEV->isAffine()) {
        auto Lp = RecSCEV->getLoop();
        auto HLoop = LF->findHLLoop(Lp);

        if (!HLoop || !HLNodeUtils::contains(HLoop, CurNode)) {
          parseBlob(CastSCEV, CE, Level);
          return;
        }
      }
    }

    if (IsTop && !UnderCast) {
      CE->setSrcType(CastSCEV->getOperand()->getType());
      CE->setExtType(isa<SCEVSignExtendExpr>(CastSCEV));
      parseRecursive(CastSCEV->getOperand(), CE, Level, true, true);
    } else {
      parseBlob(CastSCEV, CE, Level);
    }

  } else if (auto AddSCEV = dyn_cast<SCEVAddExpr>(SC)) {
    for (auto I = AddSCEV->op_begin(), E = AddSCEV->op_end(); I != E; ++I) {
      parseRecursive(*I, CE, Level, false, UnderCast);
    }

  } else if (isa<SCEVMulExpr>(SC)) {
    parseBlob(SC, CE, Level);

  } else if (auto UDivSCEV = dyn_cast<SCEVUDivExpr>(SC)) {
    if (IsTop) {
      // If the denominator is constant, move it into CE's denominator.
      if (auto ConstDenomSCEV = dyn_cast<SCEVConstant>(UDivSCEV->getRHS())) {
        parseDenominator(ConstDenomSCEV, CE);
        parseRecursive(UDivSCEV->getLHS(), CE, Level, false, UnderCast);
      } else {
        parseBlob(SC, CE, Level);
      }
    } else {
      parseBlob(SC, CE, Level);
    }

  } else if (auto RecSCEV = dyn_cast<SCEVAddRecExpr>(SC)) {

    auto Lp = RecSCEV->getLoop();
    auto HLoop = LF->findHLLoop(Lp);

    if (!RecSCEV->isAffine() || !HLoop ||
        !HLNodeUtils::contains(HLoop, CurNode)) {
      parseBlob(SC, CE, Level);

    } else {

      // Break linear addRec into base and step.
      auto BaseSCEV = RecSCEV->getOperand(0);
      auto StepSCEV = RecSCEV->getOperand(1);

      parseRecursive(BaseSCEV, CE, Level, false, UnderCast);

      // Set constant IV coeff.
      if (isa<SCEVConstant>(StepSCEV)) {
        auto Coeff = getSCEVConstantValue(cast<SCEVConstant>(StepSCEV));
        CE->addIV(HLoop->getNestingLevel(), 0, Coeff);
      }
      // Set blob IV coeff.
      else {
        parseBlob(StepSCEV, CE, Level, HLoop->getNestingLevel());
      }
    }

  } else if (isa<SCEVSMaxExpr>(SC) || isa<SCEVUMaxExpr>(SC)) {
    // TODO: extend DDRef representation to handle min/max.
    parseBlob(SC, CE, Level);

  } else {
    llvm_unreachable("Unexpected SCEV type!");
  }
}

CanonExpr *HIRParser::parseAsBlob(const Value *Val, unsigned Level) {
  CanonExpr *CE = CanonExprUtils::createCanonExpr(Val->getType());
  auto BlobSCEV = SE->getUnknown(const_cast<Value *>(Val));

  parseBlob(BlobSCEV, CE, Level);

  return CE;
}

CanonExpr *HIRParser::parse(const Value *Val, unsigned Level) {
  CanonExpr *CE = nullptr;

  // Parse as blob if the type is not SCEVable.
  // This is currently for handling floating types.
  if (!SE->isSCEVable(Val->getType())) {
    CE = parseAsBlob(Val, Level);

  } else {

    // For cast instructions which cast from loop IV's type to some other type,
    // we want to explicitly hide the cast and parse the value in IV's type.
    // This allows more opportunities for canon expr merging. Consider the
    // following cast-
    // %idxprom = sext i32 %i.01 to i64
    // Here %i.01 is the loop IV whose SCEV looks like this:
    // {0,+,1}<nuw><nsw><%for.body> (i32 type)
    // The SCEV of %idxprom doesn't have a cast and it looks like this:
    // {0,+,1}<nuw><nsw><%for.body> (i64 type)
    // We instead want %idxprom to be considered as a cast: sext i32
    // {0,+,1}<nuw><nsw><%for.body> to i64
    auto CI = dyn_cast<CastInst>(Val);
    auto ParentLoop = getCurNode()->getParentLoop();
    bool UnderCast = false;

    if (CI && ParentLoop && (ParentLoop->getIVType() == CI->getSrcTy()) &&
        (isa<SExtInst>(CI) || isa<ZExtInst>(CI) || isa<TruncInst>(CI))) {
      Val = CI->getOperand(0);
      CE = CanonExprUtils::createExtCanonExpr(CI->getSrcTy(), CI->getDestTy(),
                                              isa<SExtInst>(CI));
      UnderCast = true;
    } else {
      CE = CanonExprUtils::createCanonExpr(Val->getType());
    }

    auto SC = SE->getSCEV(const_cast<Value *>(Val));
    parseRecursive(SC, CE, Level, true, UnderCast);
  }

  return CE;
}

void HIRParser::clearTempBlobLevelMap() { CurTempBlobLevelMap.clear(); }

void HIRParser::populateBlobDDRefs(RegDDRef *Ref) {
  for (auto I = CurTempBlobLevelMap.begin(), E = CurTempBlobLevelMap.end();
       I != E; ++I) {
    auto Blob = CanonExprUtils::getBlob(I->first);
    (void)Blob;
    assert(isa<SCEVUnknown>(Blob) && "Unexpected temp blob!");

    auto BRef = DDRefUtils::createBlobDDRef(I->first, I->second);
    Ref->addBlobDDRef(BRef);
  }
}

RegDDRef *HIRParser::createLowerDDRef(Type *IVType) {
  auto Ref = DDRefUtils::createConstDDRef(IVType, 0);
  return Ref;
}

RegDDRef *HIRParser::createStrideDDRef(Type *IVType) {
  auto Ref = DDRefUtils::createConstDDRef(IVType, 1);
  return Ref;
}

RegDDRef *HIRParser::createUpperDDRef(const SCEV *BETC, unsigned Level,
                                      Type *IVType) {
  const Value *Val;

  clearTempBlobLevelMap();

  if (auto ConstSCEV = dyn_cast<SCEVConstant>(BETC)) {
    Val = ConstSCEV->getValue();
  } else if (auto UnknownSCEV = dyn_cast<SCEVUnknown>(BETC)) {
    Val = UnknownSCEV->getValue();
  } else {
    Val = ScalarSA->getGenericLoopUpperVal();
  }

  auto Symbase = ScalarSA->getOrAssignScalarSymbase(Val);

  auto Ref = DDRefUtils::createRegDDRef(Symbase);
  auto CE = CanonExprUtils::createCanonExpr(IVType);
  auto BETCType = BETC->getType();

  // If there is a type mismatch, make upper the same type as IVType.
  if (BETCType != IVType) {
    if (IVType->getPrimitiveSizeInBits() > BETCType->getPrimitiveSizeInBits()) {
      BETC = SE->getZeroExtendExpr(BETC, IVType);
    } else {
      BETC = SE->getTruncateExpr(BETC, IVType);
    }
  }

  // We pass underCast as 'true' as we don't want to hide the topmost cast for
  // upper.
  parseRecursive(BETC, CE, Level, true, true);

  Ref->setSingleCanonExpr(CE);

  // Update DDRef's symbase to blob's symbase for self-blob DDRefs.
  if (CE->isSelfBlob()) {
    Ref->setSymbase(CanonExprUtils::getBlobSymbase(CE->getSingleBlobIndex()));
  } else {
    populateBlobDDRefs(Ref);
  }

  return Ref;
}

void HIRParser::parse(HLLoop *HLoop) {

  setCurNode(HLoop);

  auto Lp = HLoop->getLLVMLoop();
  assert(Lp && "HLLoop doesn't contain LLVM loop!");
  auto IVType = HLoop->getIVType();

  // Upper should be parsed after imcrementing level.
  CurLevel++;

  if (SE->hasLoopInvariantBackedgeTakenCount(Lp)) {
    auto BETC = SE->getBackedgeTakenCount(Lp);

    // Initialize Lower to 0.
    auto LowerRef = createLowerDDRef(IVType);
    HLoop->setLowerDDRef(LowerRef);

    // Initialize Stride to 1.
    auto StrideRef = createStrideDDRef(IVType);
    HLoop->setStrideDDRef(StrideRef);

    // Set the upper bound
    auto UpperRef = createUpperDDRef(BETC, CurLevel, IVType);
    HLoop->setUpperDDRef(UpperRef);
  }
  // TODO: assert that SIMD loops are always DO loops.
}

void HIRParser::parseCompare(const Value *Cond, unsigned Level,
                             PredicateTy *Pred, RegDDRef **LHSDDRef,
                             RegDDRef **RHSDDRef) {

  if (auto CInst = dyn_cast<CmpInst>(Cond)) {
    *Pred = CInst->getPredicate();

    *LHSDDRef = createRvalDDRef(CInst, 0, Level);
    *RHSDDRef = createRvalDDRef(CInst, 1, Level);

    return;
  }

  if (isa<UndefValue>(Cond)) {
    *Pred = UNDEFINED_PREDICATE;
  } else if (auto ConstVal = dyn_cast<Constant>(Cond)) {
    if (ConstVal->isOneValue()) {
      *Pred = PredicateTy::FCMP_TRUE;
    } else if (ConstVal->isZeroValue()) {
      *Pred = PredicateTy::FCMP_FALSE;
    } else {
      llvm_unreachable("Unexpected conditional branch value");
    }
  } else {
    // TODO: Add parsing of predicates linked with && and ||
    assert(Cond->getType()->isIntegerTy(1) && "Cond should be an i1 type");
    *Pred = PredicateTy::ICMP_NE;
    *LHSDDRef = createScalarDDRef(Cond, Level);
    *RHSDDRef = DDRefUtils::createConstDDRef(Cond->getType(), 0);
    return;
  }

  *LHSDDRef = createUndefDDRef(Cond->getType());
  *RHSDDRef = createUndefDDRef(Cond->getType());
}

void HIRParser::parse(HLIf *If) {
  PredicateTy Pred;
  RegDDRef *LHSDDRef, *RHSDDRef;

  setCurNode(If);

  auto SrcBB = HIR->getSrcBBlock(If);
  assert(SrcBB && "Could not find If's src basic block!");

  auto BeginPredIter = If->pred_begin();
  auto IfCond = cast<BranchInst>(SrcBB->getTerminator())->getCondition();

  parseCompare(IfCond, CurLevel, &Pred, &LHSDDRef, &RHSDDRef);

  If->replacePredicate(If->pred_begin(), Pred);
  If->setPredicateOperandDDRef(LHSDDRef, BeginPredIter, true);
  If->setPredicateOperandDDRef(RHSDDRef, BeginPredIter, false);
}

void HIRParser::parse(HLSwitch *Switch) {
  RegDDRef *CaseValRef = nullptr;
  unsigned CaseNum = 1;

  setCurNode(Switch);

  auto SrcBB = HIR->getSrcBBlock(Switch);
  assert(SrcBB && "Could not find If's src basic block!");

  // For some reason switch case values cannot be accessed using the const
  // object.
  SwitchInst *SInst =
      const_cast<SwitchInst *>(cast<SwitchInst>(SrcBB->getTerminator()));

  Value *CondVal = SInst->getCondition();

  auto CondRef = createScalarDDRef(CondVal, CurLevel);

  Switch->setConditionDDRef(CondRef);

  for (auto I = SInst->case_begin(), E = SInst->case_end(); I != E;
       ++I, ++CaseNum) {
    CaseValRef = createScalarDDRef(I.getCaseValue(), CurLevel);
    Switch->setCaseValueDDRef(CaseValRef, CaseNum);
  }
}

void HIRParser::collectStrides(Type *GEPType,
                               SmallVectorImpl<uint64_t> &Strides) const {
  assert(isa<PointerType>(GEPType) && "GEP is not a pointer type!");
  GEPType = cast<PointerType>(GEPType)->getElementType();

  // Collect number of elements in each dimension
  for (; ArrayType *GEPArrType = dyn_cast<ArrayType>(GEPType);
       GEPType = GEPArrType->getElementType()) {
    Strides.push_back(GEPArrType->getNumElements());
  }

  assert((GEPType->isIntegerTy() || GEPType->isFloatingPointTy() ||
          GEPType->isPointerTy()) &&
         "Unexpected GEP type!");

  auto ElementSize = getDataLayout().getTypeSizeInBits(GEPType) / 8;

  // Multiply number of elements in each dimension by the size of each element
  // in the dimension.
  // We need to do a reverse traversal from the smallest(innermost) to
  // largest(outermost) dimension.
  for (auto I = Strides.rbegin(), E = Strides.rend(); I != E; ++I) {
    (*I) *= ElementSize;
    ElementSize = (*I);
  }

  Strides.push_back(ElementSize);
}

unsigned HIRParser::getBitElementSize(Type *Ty) const {
  assert(isa<PointerType>(Ty) && "Invalid type!");

  auto ElTy = cast<PointerType>(Ty)->getElementType();

  return getDataLayout().getTypeSizeInBits(ElTy);
}

class HIRParser::PointerBlobFinder {
private:
  const HIRParser *HIRP;
  const SCEV *PtrBlob;
  bool MultiplePtrBlobs;

public:
  PointerBlobFinder(const HIRParser *HIRP)
      : HIRP(HIRP), PtrBlob(nullptr), MultiplePtrBlobs(false) {}
  ~PointerBlobFinder() {}

  bool follow(const SCEV *SC) {

    if (HIRP->isTempBlob(SC) && isa<PointerType>(SC->getType())) {
      if (!PtrBlob) {
        PtrBlob = SC;
      } else {
        MultiplePtrBlobs = true;
        PtrBlob = nullptr;
      }
    }

    return !isDone();
  }

  bool isDone() const { return MultiplePtrBlobs; }

  const SCEV *getPointerBlob() const { return PtrBlob; }
};

const SCEV *HIRParser::findPointerBlob(const SCEV *PtrSCEV) const {
  PointerBlobFinder PBF(this);
  SCEVTraversal<PointerBlobFinder> Finder(PBF);
  Finder.visitAll(PtrSCEV);

  return PBF.getPointerBlob();
}

const Instruction *HIRParser::getHeaderPhiOperand(const PHINode *Phi,
                                                  bool IsInit) const {
  assert(RI->isHeaderPhi(Phi) && "Phi is not a header phi!");

  auto Lp = LI->getLoopFor(Phi->getParent());

  for (unsigned I = 0, E = Phi->getNumIncomingValues(); I != E; ++I) {
    auto PhiOp = Phi->getIncomingValue(I);

    assert(isa<Instruction>(PhiOp) &&
           "Header phi operand is not an instruction!");

    auto Inst = cast<Instruction>(PhiOp);

    if (IsInit) {
      if (LI->getLoopFor(Inst->getParent()) != Lp) {
        return Inst;
      }
    } else {
      if (LI->getLoopFor(Inst->getParent()) == Lp) {
        return Inst;
      }
    }
  }

  llvm_unreachable("Could not find appropriate header phi operand!");
}

const Instruction *HIRParser::getHeaderPhiInitInst(const PHINode *Phi) const {
  return getHeaderPhiOperand(Phi, true);
}

const Instruction *HIRParser::getHeaderPhiUpdateInst(const PHINode *Phi) const {
  return getHeaderPhiOperand(Phi, false);
}

CanonExpr *HIRParser::createHeaderPhiInitCE(const PHINode *Phi,
                                            unsigned Level) {
  const Instruction *InitInst = getHeaderPhiInitInst(Phi);

  auto InitCE = parseAsBlob(InitInst, Level);

  return InitCE;
}

CanonExpr *HIRParser::createHeaderPhiIndexCE(const PHINode *Phi,
                                             unsigned Level) {
  const Instruction *UpdateInst = getHeaderPhiUpdateInst(Phi);

  auto PhiSCEV = SE->getSCEV(const_cast<PHINode *>(Phi));
  auto UpdateSCEV = SE->getSCEV(const_cast<Instruction *>(UpdateInst));

  // Create stride as (update - phi). For example-
  // PhiSCEV: {%ptr,+,4)
  // UpdateSCEV : {(%ptr + 4),+,4)
  // StrideSCEV : 4
  auto StrideSCEV = SE->getMinusSCEV(UpdateSCEV, PhiSCEV);

  auto IndexTy = Type::getIntNTy(
      getContext(), getDataLayout().getTypeSizeInBits(Phi->getType()));

  // Create index as {0,+,stride}
  auto InitSCEV = SE->getConstant(IndexTy, 0);
  auto IndexSCEV =
      SE->getAddRecExpr(InitSCEV, StrideSCEV, LI->getLoopFor(Phi->getParent()),
                        cast<SCEVAddRecExpr>(PhiSCEV)->getNoWrapFlags());

  auto IndexCE = CanonExprUtils::createCanonExpr(IndexTy);

  parseRecursive(IndexSCEV, IndexCE, Level);

  return IndexCE;
}

RegDDRef *HIRParser::createPhiBaseGEPDDRef(const PHINode *BasePhi,
                                           const GEPOperator *GEPOp,
                                           unsigned Level) {
  SmallVector<uint64_t, 9> Strides;
  CanonExpr *BaseCE = nullptr, *IndexCE = nullptr;
  auto BaseTy = BasePhi->getType();

  auto Ref = DDRefUtils::createRegDDRef(0);
  auto SC = SE->getSCEV(const_cast<PHINode *>(BasePhi));
  const SCEV *BaseSCEV = nullptr;
  unsigned BitElementSize = getBitElementSize(BaseTy);
  unsigned ElementSize = BitElementSize / 8;
  bool IsInBounds = false;

  // If the base is linear, we separate it into a pointer base and a linear
  // offset. The linear offset is then moved into the index.
  // Example IR-
  //
  // for.body:
  //   %i.06 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  //   %p.addr.05 = phi i32* [ %p, %entry ], [ %incdec.ptr, %for.body ]
  //   store i32 %i.06, i32* %p.addr.05, align 4, !tbaa !1
  //   %incdec.ptr = getelementptr inbounds i32, i32* %p.addr.05, i64 1
  //   br i1 %exitcond, label %for.end, label %for.body
  //
  // In the above example the phi base %p.addr.05 is linear {%p,+,4}. We
  // separate it into ptr base %p and linear offset {0,+,4}. The linear offset
  // is then translated into a normalized index of i. The final mapped expr
  // looks like this: (%p)[i]
  if (auto RecSCEV = dyn_cast<SCEVAddRecExpr>(SC)) {

    if (RecSCEV->isAffine()) {
      // getPrimaryElementType() comparison is to guard against tracing through
      // bitcasts.
      if ((BaseSCEV = findPointerBlob(RecSCEV)) &&
          (RI->getPrimaryElementType(RecSCEV->getType()) ==
           RI->getPrimaryElementType(BasePhi->getType()))) {
        // Collect extra dimensions, if any.
        collectStrides(BaseSCEV->getType(), Strides);

        auto OffsetSCEV = SE->getMinusSCEV(RecSCEV, BaseSCEV);

        BaseCE = CanonExprUtils::createCanonExpr(BaseSCEV->getType());
        IndexCE = CanonExprUtils::createCanonExpr(OffsetSCEV->getType());
        parseRecursive(BaseSCEV, BaseCE, Level);
        parseRecursive(OffsetSCEV, IndexCE, Level);

        // Normalize with repsect to element size.
        IndexCE->multiplyDenominator(ElementSize, true);
      }
      // Decompose phi into base and index ourselves.
      else if (RI->isHeaderPhi(BasePhi)) {
        BaseCE = createHeaderPhiInitCE(BasePhi, Level);
        IndexCE = createHeaderPhiIndexCE(BasePhi, Level);

        // Normalize with repsect to element size.
        IndexCE->multiplyDenominator(ElementSize, true);
      }
    }

    // Use no wrap flags to set inbounds property.
    IsInBounds = (RecSCEV->getNoWrapFlags(SCEV::FlagNUW) ||
                  RecSCEV->getNoWrapFlags(SCEV::FlagNSW));
  }

  // Non-linear base is parsed as base + zero offset: (%p)[0].
  if (!BaseCE) {
    BaseCE = parseAsBlob(BasePhi, Level);

    auto OffsetType = Type::getIntNTy(
        getContext(), getDataLayout().getTypeSizeInBits(BaseTy));
    IndexCE = CanonExprUtils::createCanonExpr(OffsetType);
  }

  auto StrideCE =
      CanonExprUtils::createCanonExpr(IndexCE->getDestType(), 0, ElementSize);

  // Here we add the other operand of GEPOperator as an offset to the index.
  if (GEPOp) {
    assert((2 == GEPOp->getNumOperands()) &&
           "Unexpected number of GEP operands!");

    auto OffsetSC = SE->getSCEV(const_cast<Value *>(GEPOp->getOperand(1)));
    parseRecursive(OffsetSC, IndexCE, Level);
    IsInBounds = GEPOp->isInBounds();
  }

  Ref->setBaseCE(BaseCE);
  Ref->addDimension(IndexCE, StrideCE);

  unsigned NumDims = Strides.size();

  // Add zero indices for the extra dimensions.
  // Extra dimensions are involved when the initial value of BasePhi is computed
  // using an array like the following-
  // %p.07 = phi i32* [ %incdec.ptr, %for.body ], [ getelementptr inbounds ([50
  // x i32], [50 x i32]* @A, i64 0, i64 10), %entry ]
  if (NumDims > 1) {
    for (auto I = NumDims - 1; I > 0; --I) {
      IndexCE = CanonExprUtils::createCanonExpr(IndexCE->getDestType());
      StrideCE = CanonExprUtils::createCanonExpr(IndexCE->getDestType(), 0,
                                                 Strides[I - 1]);
      Ref->addDimension(IndexCE, StrideCE);
    }
  }

  Ref->setInBounds(IsInBounds);

  return Ref;
}

const GEPOperator *HIRParser::getBaseGEPOp(const GEPOperator *GEPOp) const {

  while (auto TempGEPOp = dyn_cast<GEPOperator>(GEPOp->getPointerOperand())) {
    GEPOp = TempGEPOp;
  }

  return GEPOp;
}

RegDDRef *HIRParser::createRegularGEPDDRef(const GEPOperator *GEPOp,
                                           unsigned Level) {
  SmallVector<uint64_t, 9> Strides;

  auto Ref = DDRefUtils::createRegDDRef(0);

  const GEPOperator *BaseGEPOp = getBaseGEPOp(GEPOp);
  auto BaseVal = BaseGEPOp->getPointerOperand();

  // TODO: This can be improved by first checking if the original SCEV can be
  // handled.
  CanonExpr *BaseCE = parseAsBlob(BaseVal, Level);
  Ref->setBaseCE(BaseCE);

  collectStrides(BaseGEPOp->getPointerOperandType(), Strides);

  unsigned Count = Strides.size();
  const GEPOperator *TempGEPOp = GEPOp;
  bool FirstGEP = true;

  // Consider the following sequence of GEPs-
  // %arrayidx = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x
  // i32]]* @B, i64 0, i64 %i
  // %arrayidx5 = getelementptr inbounds [100 x i32], [100 x i32]* %arrayidx,
  // i64 0, i64 %j
  //
  // %0 = load i32, i32* %arrayidx5, align 4
  //
  // This is how the dimensions are created-
  // 1) Start processing %arrayidx5's operands in reverse and create one
  // dimension each for %j and 0.
  // 2) Start processing %arrayidx's operands in reverse. The last index %i is
  // added to the last dimension created while processing %arrayidx5's operands
  // (0).
  // 3) Create additional dimension for the 0 operand.
  //
  // The parsed DDRef looks like this- (@B][0][%i][%j]
  do {
    // Ignore base pointer operand.
    unsigned GEPNumOp = TempGEPOp->getNumOperands() - 1;
    bool LastGEPIndex = true;

    // Process GEP operands in reverse order (from lowest to highest dimension).
    for (auto I = GEPNumOp; I > 0; --I) {
      // If this is the last GEP index of a previous GEP, we add it to the last
      // created index CE.
      if (!FirstGEP && LastGEPIndex) {
        CanonExpr *IndexCE = Ref->getDimensionIndex(Ref->getNumDimensions());
        auto SC = SE->getSCEV(const_cast<Value *>(TempGEPOp->getOperand(I)));

        parseRecursive(SC, IndexCE, Level, IndexCE->isZero());
      } else {
        // Create additional dimension for each encountered GEP index.
        CanonExpr *IndexCE = parse(TempGEPOp->getOperand(I), Level);
        CanonExpr *StrideCE = CanonExprUtils::createCanonExpr(
            IndexCE->getDestType(), 0, Strides[Count - 1]);
        Ref->addDimension(IndexCE, StrideCE);
        --Count;
      }
      LastGEPIndex = false;
    }

    FirstGEP = false;
  } while ((TempGEPOp = dyn_cast<GEPOperator>(TempGEPOp->getPointerOperand())));

  // Check that the number of GEP operands match with the number of strides we
  // have collected.
  assert((Count == 0) && "Number of subscripts and strides do not match!");

  Ref->setInBounds(GEPOp->isInBounds());

  return Ref;
}

RegDDRef *HIRParser::createSingleElementGEPDDRef(const Value *GEPVal,
                                                 unsigned Level) {

  auto Ref = DDRefUtils::createRegDDRef(0);
  auto GEPTy = GEPVal->getType();
  unsigned BitElementSize = getBitElementSize(GEPTy);
  auto OffsetTy =
      Type::getIntNTy(getContext(), getDataLayout().getTypeSizeInBits(GEPTy));

  // TODO: This can be improved by first checking if the original SCEV can be
  // handled.
  auto BaseCE = parseAsBlob(GEPVal, Level);
  Ref->setBaseCE(BaseCE);

  // Create Index of zero.
  auto IndexCE = CanonExprUtils::createCanonExpr(OffsetTy);
  auto StrideCE =
      CanonExprUtils::createCanonExpr(OffsetTy, 0, BitElementSize / 8);

  Ref->addDimension(IndexCE, StrideCE);

  // Single element is always in bounds.
  Ref->setInBounds(true);

  return Ref;
}

// NOTE: AddRec->delinearize() doesn't work with constant bound arrays.
// TODO: handle struct GEPs.
RegDDRef *HIRParser::createGEPDDRef(const Value *GEPVal, unsigned Level,
                                    bool IsUse) {
  const PHINode *BasePhi = nullptr;
  const GEPOperator *GEPOp = nullptr;
  RegDDRef *Ref = nullptr;
  Type *DestTy = nullptr;

  clearTempBlobLevelMap();

  // In some cases float* is converted into i32* before loading/storing. This
  // info is propagated into the BaseCE dest type.
  if (auto BCInst = dyn_cast<BitCastInst>(GEPVal)) {
    if (!SE->isHIRCopyInst(BCInst) &&
        RI->isSupported(BCInst->getOperand(0)->getType())) {
      GEPVal = BCInst->getOperand(0);
      DestTy = BCInst->getDestTy();
    }
  }

  auto GEPInst = dyn_cast<Instruction>(GEPVal);

  // Try to get to the phi associated with this GEP.
  // Do not cross the live range indicator for GEP uses (load/store/bitcast).
  if ((!IsUse || !GEPInst || !SE->isHIRLiveRangeIndicator(GEPInst)) &&
      (GEPOp = dyn_cast<GEPOperator>(GEPVal))) {

    BasePhi = dyn_cast<PHINode>(GEPOp->getPointerOperand());

  } else if (GEPInst) {
    BasePhi = dyn_cast<PHINode>(GEPInst);
  }

  if (BasePhi) {
    Ref = createPhiBaseGEPDDRef(BasePhi, GEPOp, Level);
  } else if (GEPOp) {
    Ref = createRegularGEPDDRef(GEPOp, Level);
  } else {
    Ref = createSingleElementGEPDDRef(GEPVal, Level);
  }

  if (DestTy) {
    Ref->setBaseDestType(DestTy);
  }

  populateBlobDDRefs(Ref);

  return Ref;
}

RegDDRef *HIRParser::createUndefDDRef(Type *Ty) {
  Value *UndefVal = UndefValue::get(Ty);
  CanonExpr::BlobTy Blob = SE->getUnknown(UndefVal);

  auto Symbase = ScalarSA->getOrAssignScalarSymbase(UndefVal);

  RegDDRef *Ref = DDRefUtils::createRegDDRef(Symbase);
  CanonExpr *CE = CanonExprUtils::createCanonExpr(Ty);

  // Add an undef blob to the CE to maintain consistency.
  parseBlob(Blob, CE, 0);
  CE->setContainsUndef();

  Ref->setSingleCanonExpr(CE);

  return Ref;
}

RegDDRef *HIRParser::createScalarDDRef(const Value *Val, unsigned Level,
                                       bool IsLval) {
  CanonExpr *CE;

  clearTempBlobLevelMap();

  auto Symbase = ScalarSA->getOrAssignScalarSymbase(Val);
  auto Ref = DDRefUtils::createRegDDRef(Symbase);

  // Force pointer values to be parsed as blobs. This is for handling lvals but
  // pointer blobs can occur in loop upper as well. CG will have to do special
  // processing for pointers contained in upper.
  if (Val->getType()->isPointerTy()) {

    // Create null CE to represent a null pointer.
    if (isa<ConstantPointerNull>(Val)) {
      CE = CanonExprUtils::createCanonExpr(Val->getType());
    } else {
      CE = parseAsBlob(Val, Level);
    }
  } else {
    CE = parse(Val, Level);
  }

  Ref->setSingleCanonExpr(CE);

  if (CE->isSelfBlob()) {
    unsigned SB = CanonExprUtils::getBlobSymbase(CE->getSingleBlobIndex());

    // Update rval DDRef's symbase to blob's symbase for self-blob DDRefs.
    if (!IsLval) {
      Ref->setSymbase(SB);
    }
    // If lval DDRef's symbase and blob's symbase don't match, we need to add a
    // blob DDRef.
    else if (Symbase != SB) {
      populateBlobDDRefs(Ref);
    }

  } else if (CE->isConstant()) {
    if (!IsLval) {
      Ref->setSymbase(CONSTANT_SYMBASE);
    }

  } else {
    populateBlobDDRefs(Ref);
  }

  return Ref;
}

RegDDRef *HIRParser::createRvalDDRef(const Instruction *Inst, unsigned OpNum,
                                     unsigned Level) {
  RegDDRef *Ref;
  auto OpVal = Inst->getOperand(OpNum);

  if (auto LInst = dyn_cast<LoadInst>(Inst)) {
    Ref = createGEPDDRef(LInst->getPointerOperand(), Level, true);

  } else if (isa<GetElementPtrInst>(Inst)) {
    Ref = createGEPDDRef(Inst, Level, false);
    Ref->setAddressOf(true);

  } else if (OpVal->getType()->isPointerTy() &&
             !isa<ConstantPointerNull>(OpVal)) {
    Ref = createGEPDDRef(OpVal, Level, true);
    Ref->setAddressOf(true);

  } else {
    Ref = createScalarDDRef(OpVal, Level);
  }

  return Ref;
}

RegDDRef *HIRParser::createLvalDDRef(const Instruction *Inst, unsigned Level) {
  RegDDRef *Ref;

  if (auto SInst = dyn_cast<StoreInst>(Inst)) {
    Ref = createGEPDDRef(SInst->getPointerOperand(), Level, true);
  } else {
    Ref = createScalarDDRef(Inst, Level, true);
  }

  return Ref;
}

unsigned HIRParser::getNumRvalOperands(HLInst *HInst) {
  unsigned NumRvalOp = HInst->getNumOperands();

  if (HInst->hasLval()) {
    NumRvalOp--;
  }

  if (isa<SelectInst>(HInst->getLLVMInstruction())) {
    NumRvalOp--;
  }

  return NumRvalOp;
}

void HIRParser::parse(HLInst *HInst, bool IsPhase1, unsigned Phase2Level) {
  RegDDRef *Ref;
  bool HasLval = false;
  auto Inst = HInst->getLLVMInstruction();
  unsigned Level;

  assert(!Inst->getType()->isVectorTy() && "Vector types not supported!");

  setCurNode(HInst);

  if (IsPhase1) {
    Level = CurLevel;

    if (HInst->isInPreheaderOrPostexit()) {
      --Level;
    }
  } else {
    Level = Phase2Level;
  }

  // Process lval
  if (HInst->hasLval()) {
    HasLval = true;

    if (IsPhase1 && !isEssential(Inst)) {
      // Postpone the processing of this instruction to Phase2.
      auto Symbase = ScalarSA->getOrAssignScalarSymbase(Inst);
      UnclassifiedSymbaseInsts[Symbase].push_back(std::make_pair(HInst, Level));
      return;
    } else {
      Ref = createLvalDDRef(Inst, Level);
    }

    HInst->setLvalDDRef(Ref);
  }

  unsigned NumRvalOp = getNumRvalOperands(HInst);

  // Process rvals
  for (unsigned I = 0; I < NumRvalOp; ++I) {

    if (isa<SelectInst>(Inst) && (I == 0)) {
      PredicateTy Pred;
      RegDDRef *LHSDDRef, *RHSDDRef;

      parseCompare(Inst->getOperand(0), Level, &Pred, &LHSDDRef, &RHSDDRef);

      HInst->setPredicate(Pred);
      HInst->setOperandDDRef(LHSDDRef, 1);
      HInst->setOperandDDRef(RHSDDRef, 2);
      continue;
    }

    Ref = createRvalDDRef(Inst, I, Level);

    // To translate Instruction's operand number into HLInst's operand number we
    // add one offset each for having an lval and being a select instruction.
    auto OpNum = HasLval ? (isa<SelectInst>(Inst) ? (I + 2) : (I + 1)) : I;

    HInst->setOperandDDRef(Ref, OpNum);
  }

  if (auto CInst = dyn_cast<CmpInst>(Inst)) {
    HInst->setPredicate(CInst->getPredicate());
  }
}

void HIRParser::phase1Parse(HLNode *Node) {
  Phase1Visitor PV(this);
  HLNodeUtils::visit(PV, Node);
}

void HIRParser::phase2Parse() {

  // Keep iterating through required symbases until the container is empty.
  // Additional symbases might be added during parsing.
  while (!RequiredSymbases.empty()) {

    auto SymIt = RequiredSymbases.begin();
    auto Symbase = *SymIt;
    auto SymInstIt = UnclassifiedSymbaseInsts.find(Symbase);

    // Symbase has already been processed.
    if (SymInstIt == UnclassifiedSymbaseInsts.end()) {
      RequiredSymbases.erase(SymIt);
      continue;
    }

    // Parse instructions associated with this symbase. This can lead to the
    // discovery of additional required symbases.
    for (auto InstIt = SymInstIt->second.begin(),
              EndIt = SymInstIt->second.end();
         InstIt != EndIt; ++InstIt) {
      parse(InstIt->first, false, InstIt->second);
    }

    // Erase symbase entry after processing.
    UnclassifiedSymbaseInsts.erase(SymInstIt);

    // Cannot use SymIt here as it might have been invalidated with an insertion
    // into the set during parsing.
    RequiredSymbases.erase(Symbase);
  }

  // Erase the leftover unclassified HLInsts as they are not required.
  for (auto SymIt = UnclassifiedSymbaseInsts.begin(),
            E = UnclassifiedSymbaseInsts.end();
       SymIt != E; ++SymIt) {
    for (auto InstIt = SymIt->second.begin(), EndIt = SymIt->second.end();
         InstIt != EndIt; ++InstIt) {
      HLNodeUtils::erase(InstIt->first);
    }
  }

  UnclassifiedSymbaseInsts.clear();
}

bool HIRParser::runOnFunction(Function &F) {
  Func = &F;
  SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
  LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  RI = &getAnalysis<RegionIdentification>();
  HIR = &getAnalysis<HIRCreation>();
  LF = &getAnalysis<LoopFormation>();
  ScalarSA = &getAnalysis<ScalarSymbaseAssignment>();

  HLUtils::setHIRParser(this);

  // We parse one region at a time to preserve CurRegion during phase2.
  for (auto I = HIR->begin(), E = HIR->end(); I != E; ++I) {
    assert(UnclassifiedSymbaseInsts.empty() &&
           "UnclassifiedSymbaseInsts is not empty!");
    assert(RequiredSymbases.empty() && "RequiredSymbases is not empty!");

    // Start phase 1 of parsing.
    phase1Parse(&*I);

    // Start phase 2 of parsing.
    phase2Parse();
  }

  HLNodeUtils::initTopSortNum();

  return false;
}

void HIRParser::releaseMemory() {
  /// Destroy all DDRefs and CanonExprs.
  DDRefUtils::destroyAll();
  CanonExprUtils::destroyAll();

  CurTempBlobLevelMap.clear();
  UnclassifiedSymbaseInsts.clear();
  RequiredSymbases.clear();
}

LLVMContext &HIRParser::getContext() const { return Func->getContext(); }

const DataLayout &HIRParser::getDataLayout() const {
  return Func->getParent()->getDataLayout();
}

void HIRParser::print(raw_ostream &OS, const Module *M) const {
  HIR->printWithFrameworkDetails(OS);
}

// Verification is done by HIRVerifier.
void HIRParser::verifyAnalysis() const {}
