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
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/LLVMContext.h"

#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/LoopInfo.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRParser.h"
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
INITIALIZE_PASS_DEPENDENCY(ScalarEvolution)
INITIALIZE_PASS_DEPENDENCY(ScalarSymbaseAssignment)
INITIALIZE_PASS_DEPENDENCY(HIRCreation)
INITIALIZE_PASS_DEPENDENCY(LoopFormation)
INITIALIZE_PASS_END(HIRParser, "hir-parser", "HIR Parser", false, true)

char HIRParser::ID = 0;

FunctionPass *llvm::createHIRParserPass() { return new HIRParser(); }

HIRParser::HIRParser() : FunctionPass(ID), CurLevel(0) {
  initializeHIRParserPass(*PassRegistry::getPassRegistry());
}

void HIRParser::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequiredTransitive<LoopInfoWrapperPass>();
  AU.addRequiredTransitive<ScalarEvolution>();
  AU.addRequiredTransitive<ScalarSymbaseAssignment>();
  AU.addRequiredTransitive<HIRCreation>();
  AU.addRequiredTransitive<LoopFormation>();
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
        !ScalarSA->isConstant(UnknownSCEV->getValue())) {
      return true;
    }
  }

  return false;
}

void HIRParser::insertBlobHelper(CanonExpr::BlobTy Blob, bool Insert,
                                 unsigned *NewBlobIndex) {
  if (Insert) {
    unsigned BlobIndex = CanonExprUtils::findOrInsertBlob(Blob);

    if (NewBlobIndex) {
      *NewBlobIndex = BlobIndex;
    }
  }
}

CanonExpr::BlobTy HIRParser::createBlob(Value *Val, bool Insert,
                                        unsigned *NewBlobIndex) {
  assert(Val && "Value cannot be null!");

  auto Blob = SE->getUnknown(Val);

  insertBlobHelper(Blob, Insert, NewBlobIndex);

  return Blob;
}

CanonExpr::BlobTy HIRParser::createBlob(int64_t Val, bool Insert,
                                        unsigned *NewBlobIndex) {
  Type *Int64Type = IntegerType::get(getContext(), 64);
  auto Blob = SE->getConstant(Int64Type, Val, false);

  insertBlobHelper(Blob, Insert, NewBlobIndex);

  return Blob;
}

CanonExpr::BlobTy HIRParser::createAddBlob(CanonExpr::BlobTy LHS,
                                           CanonExpr::BlobTy RHS, bool Insert,
                                           unsigned *NewBlobIndex) {
  assert(LHS && RHS && "Blob cannot by null!");

  auto Blob = SE->getAddExpr(LHS, RHS);

  insertBlobHelper(Blob, Insert, NewBlobIndex);

  return Blob;
}

CanonExpr::BlobTy HIRParser::createMinusBlob(CanonExpr::BlobTy LHS,
                                             CanonExpr::BlobTy RHS, bool Insert,
                                             unsigned *NewBlobIndex) {
  assert(LHS && RHS && "Blob cannot by null!");

  auto Blob = SE->getMinusSCEV(LHS, RHS);

  insertBlobHelper(Blob, Insert, NewBlobIndex);

  return Blob;
}

CanonExpr::BlobTy HIRParser::createMulBlob(CanonExpr::BlobTy LHS,
                                           CanonExpr::BlobTy RHS, bool Insert,
                                           unsigned *NewBlobIndex) {
  assert(LHS && RHS && "Blob cannot by null!");

  auto Blob = SE->getMulExpr(LHS, RHS);

  insertBlobHelper(Blob, Insert, NewBlobIndex);

  return Blob;
}

CanonExpr::BlobTy HIRParser::createUDivBlob(CanonExpr::BlobTy LHS,
                                            CanonExpr::BlobTy RHS, bool Insert,
                                            unsigned *NewBlobIndex) {
  assert(LHS && RHS && "Blob cannot by null!");

  auto Blob = SE->getUDivExpr(LHS, RHS);

  insertBlobHelper(Blob, Insert, NewBlobIndex);

  return Blob;
}

CanonExpr::BlobTy HIRParser::createTruncateBlob(CanonExpr::BlobTy Blob,
                                                Type *Ty, bool Insert,
                                                unsigned *NewBlobIndex) {
  assert(Blob && "Blob cannot by null!");
  assert(Ty && "Type cannot by null!");

  auto NewBlob = SE->getTruncateExpr(Blob, Ty);

  insertBlobHelper(NewBlob, Insert, NewBlobIndex);

  return NewBlob;
}

CanonExpr::BlobTy HIRParser::createZeroExtendBlob(CanonExpr::BlobTy Blob,
                                                  Type *Ty, bool Insert,
                                                  unsigned *NewBlobIndex) {
  assert(Blob && "Blob cannot by null!");
  assert(Ty && "Type cannot by null!");

  auto NewBlob = SE->getZeroExtendExpr(Blob, Ty);

  insertBlobHelper(NewBlob, Insert, NewBlobIndex);

  return NewBlob;
}

CanonExpr::BlobTy HIRParser::createSignExtendBlob(CanonExpr::BlobTy Blob,
                                                  Type *Ty, bool Insert,
                                                  unsigned *NewBlobIndex) {
  assert(Blob && "Blob cannot by null!");
  assert(Ty && "Type cannot by null!");

  auto NewBlob = SE->getSignExtendExpr(Blob, Ty);

  insertBlobHelper(NewBlob, Insert, NewBlobIndex);

  return NewBlob;
}

unsigned HIRParser::getMaxScalarSymbase() const {
  return ScalarSA->getMaxScalarSymbase();
}

unsigned HIRParser::getSymBaseForConstants() const {
  return ScalarSA->getSymBaseForConstants();
}

struct HIRParser::Visitor {
  HIRParser *HIRP;

  Visitor(HIRParser *Parser) : HIRP(Parser) {}

  void visit(HLRegion *Reg) { HIRP->parse(Reg); }
  void postVisit(HLRegion *Reg) { HIRP->postParse(Reg); }

  void visit(HLLoop *HLoop) { HIRP->parse(HLoop); }
  void postVisit(HLLoop *HLoop) { HIRP->postParse(HLoop); }

  void visit(HLIf *If) { HIRP->parse(If); }
  void postVisit(HLIf *If) { HIRP->postParse(If); }

  void visit(HLSwitch *Switch) { HIRP->parse(Switch); }
  void postVisit(HLSwitch *Switch) { HIRP->postParse(Switch); }

  void visit(HLInst *HInst) { HIRP->parse(HInst); }
  void visit(HLLabel *Label) { HIRP->parse(Label); }
  void visit(HLGoto *Goto) { HIRP->parse(Goto); }

  bool isDone() { return false; }
  bool skipRecursion(HLNode *Node) { return false; }
};

class HIRParser::PolynomialFinder {
private:
  bool Found;

public:
  PolynomialFinder() : Found(false) {}
  ~PolynomialFinder() {}

  bool follow(const SCEV *SC) {

    if (auto RecSCEV = dyn_cast<SCEVAddRecExpr>(SC)) {
      if (!RecSCEV->isAffine()) {
        Found = true;
      }
    }

    return !Found;
  }

  bool found() const { return Found; }
  bool isDone() const { return found(); }
};

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

void HIRParser::printScalar(raw_ostream &OS, unsigned Symbase,
                            bool Detailed) const {
  ScalarSA->getBaseScalar(Symbase)->printAsOperand(OS, Detailed);
}

void HIRParser::printBlob(raw_ostream &OS, CanonExpr::BlobTy Blob,
                          bool Detailed) const {

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
    printBlob(OS, CastSCEV->getOperand(), false);
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
    } else if (isa<SCEVSMaxExpr>(NArySCEV)) {
      OS << "umax(";
      OpStr = ", ";
    } else {
      assert(false && "Blob contains AddRec!");
    }

    for (auto I = NArySCEV->op_begin(), E = NArySCEV->op_end(); I != E; ++I) {
      printBlob(OS, *I, Detailed);

      if (std::next(I) != E) {
        OS << OpStr;
      }
    }
    OS << ")";

  } else if (auto UDivSCEV = dyn_cast<SCEVUDivExpr>(Blob)) {
    OS << "(";
    printBlob(OS, UDivSCEV->getLHS(), Detailed);
    OS << " /u ";
    printBlob(OS, UDivSCEV->getRHS(), Detailed);
    OS << ")";
  } else if (auto UnknownSCEV = dyn_cast<SCEVUnknown>(Blob)) {
    if (isTempBlob(Blob)) {
      auto Temp = ScalarSA->getBaseScalar(UnknownSCEV->getValue());
      Temp->printAsOperand(OS, Detailed);
    } else {
      OS << *Blob;
    }
  } else {
    llvm_unreachable("Unknown Blob type!");
  }
}

bool HIRParser::isPolyBlobDef(const Instruction *Inst) const {

  if (SE->isSCEVable(Inst->getType())) {
    auto SC = SE->getSCEV(const_cast<Instruction *>(Inst));

    // Instructions containing non-affine(polynomial) addRecs are made
    // blobs.
    PolynomialFinder PF;
    SCEVTraversal<PolynomialFinder> Searcher(PF);
    Searcher.visitAll(SC);

    if (PF.found()) {
      return true;
    }
  }

  return false;
}

bool HIRParser::isBlobDef(const Instruction *Inst) const {

  if (SE->isSCEVable(Inst->getType())) {
    auto SC = SE->getSCEV(const_cast<Instruction *>(Inst));
    if (isa<SCEVUnknown>(SC)) {
      return true;
    } else if (isPolyBlobDef(Inst)) {
      return true;
    }
  } else {
    // If it isn't SCEVable we cannot eliminate it, so we mark it as a blob
    // definition.
    return true;
  }

  return false;
}

bool HIRParser::isRegionLiveOut(const Instruction *Inst) const {
  auto Symbase = ScalarSA->getScalarSymbase(Inst);

  if (Symbase && CurRegion->isLiveOut(Symbase)) {
    return true;
  }

  return false;
}

bool HIRParser::isRequired(const Instruction *Inst) const {

  if (isa<LoadInst>(Inst) || isa<StoreInst>(Inst)) {
    return true;

  } else if (isRegionLiveOut(Inst)) {
    return true;

  } else if (auto CInst = dyn_cast<CmpInst>(Inst)) {
    if (hasPropagableUses(CInst)) {
      return false;
    }
    
    return true;
  }

  return isBlobDef(Inst);
}

bool HIRParser::hasPropagableUses(const CmpInst *CInst) const {

  for (auto I = CInst->user_begin(), E = CInst->user_end(); I != E; ++I) {
    if (auto UseInst = dyn_cast<Instruction>(*I)) {
      if (!isa<BranchInst>(UseInst) && !isa<SelectInst>(UseInst)) {
        return false;
      }
    } else {
      assert(false && "Use is not an instruction!");
    }
  }

  return true;
}

int64_t HIRParser::getSCEVConstantValue(const SCEVConstant *ConstSCEV) const {
  return ConstSCEV->getValue()->getSExtValue();
}

void HIRParser::parseConstant(const SCEVConstant *ConstSCEV, CanonExpr *CE) {
  auto Const = getSCEVConstantValue(ConstSCEV);

  CE->setConstant(CE->getConstant() + Const);
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

void HIRParser::setTempBlobLevel(const SCEVUnknown *TempBlobSCEV, CanonExpr *CE,
                                 unsigned NestingLevel) {
  unsigned DefLevel = 0;
  HLLoop *HLoop;

  auto Temp = TempBlobSCEV->getValue();
  auto Symbase = ScalarSA->getOrAssignScalarSymbase(Temp);
  auto Index = CanonExprUtils::findOrInsertBlob(TempBlobSCEV);

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
}

void HIRParser::breakConstantMultiplierBlob(CanonExpr::BlobTy Blob,
                                            int64_t *Multiplier,
                                            unsigned *Index) {

  if (auto MulSCEV = dyn_cast<SCEVMulExpr>(Blob)) {
    for (auto I = MulSCEV->op_begin(), E = MulSCEV->op_end(); I != E; ++I) {
      auto ConstSCEV = dyn_cast<SCEVConstant>(*I);

      if (!ConstSCEV) {
        continue;
      }

      *Multiplier = getSCEVConstantValue(ConstSCEV);

      auto NewBlob = SE->getUDivExactExpr(Blob, ConstSCEV);
      *Index = CanonExprUtils::findOrInsertBlob(NewBlob);

      return;
    }
  }

  *Multiplier = 1;
  *Index = CanonExprUtils::findOrInsertBlob(Blob);
}

void HIRParser::parseBlob(CanonExpr::BlobTy Blob, CanonExpr *CE, unsigned Level,
                          unsigned IVLevel) {
  int64_t Multiplier;
  unsigned Index;

  breakConstantMultiplierBlob(Blob, &Multiplier, &Index);

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

// TODO: refine logic
// TODO: handle IV as blobs.
void HIRParser::parseRecursive(const SCEV *SC, CanonExpr *CE, unsigned Level,
                               bool IsTop) {

  if (auto ConstSCEV = dyn_cast<SCEVConstant>(SC)) {
    assert(IsTop && "Can't handle constants embedded in the SCEV tree!");
    parseConstant(ConstSCEV, CE);

  } else if (isa<SCEVUnknown>(SC)) {
    assert(IsTop && "Can't handle unknowns embedded in the SCEV tree!");
    parseBlob(SC, CE, Level);

  } else if (isa<SCEVCastExpr>(SC)) {
    assert(IsTop && "Can't handle casts embedded in the SCEV tree!");
    parseBlob(SC, CE, Level);

  } else if (auto AddSCEV = dyn_cast<SCEVAddExpr>(SC)) {
    assert(IsTop && "Can't handle adds embedded in the SCEV tree!");

    for (auto I = AddSCEV->op_begin(), E = AddSCEV->op_end(); I != E; ++I) {
      parseRecursive(*I, CE, Level, true);
    }

  } else if (isa<SCEVMulExpr>(SC)) {
    assert(IsTop && "Can't handle multiplies embedded in the SCEV tree!");
    parseBlob(SC, CE, Level);

  } else if (isa<SCEVUDivExpr>(SC)) {
    assert(IsTop && "Can't handle divisions embedded in the SCEV tree!");
    parseBlob(SC, CE, Level);

  } else if (auto RecSCEV = dyn_cast<SCEVAddRecExpr>(SC)) {
    assert(RecSCEV->isAffine() && "Non-affine AddRecs not expected!");

    auto Lp = RecSCEV->getLoop();
    auto HLoop = LF->findHLLoop(Lp);

    assert(HLoop && "Non-HIR loop IVs not handled!");

    // Break linear addRec into base and step
    auto BaseSCEV = RecSCEV->getOperand(0);
    auto StepSCEV = RecSCEV->getOperand(1);

    parseRecursive(BaseSCEV, CE, Level, IsTop);

    // Set constant IV coeff
    if (isa<SCEVConstant>(StepSCEV)) {
      auto Coeff = getSCEVConstantValue(cast<SCEVConstant>(StepSCEV));
      CE->addIV(HLoop->getNestingLevel(), 0, Coeff);
    }
    // Set blob IV coeff
    else {
      parseBlob(StepSCEV, CE, Level, HLoop->getNestingLevel());
    }

  } else if (isa<SCEVSMaxExpr>(SC) || isa<SCEVUMaxExpr>(SC)) {
    assert(IsTop && "Can't handle max embedded in the SCEV tree!");
    parseBlob(SC, CE, Level);

  } else {
    assert(false && "unexpected SCEV type!");
  }
}

void HIRParser::parseAsBlob(const Value *Val, CanonExpr *CE, unsigned Level) {
  auto BlobSCEV = SE->getUnknown(const_cast<Value *>(Val));
  CE->setType(BlobSCEV->getType());
  parseBlob(BlobSCEV, CE, Level);
}

CanonExpr *HIRParser::parse(const Value *Val, unsigned Level) {
  const SCEV *SC;
  CanonExpr *CE;

  if (auto Inst = dyn_cast<Instruction>(Val)) {
    // Parse polynomial blob definitions as (1 * blob)
    if (isPolyBlobDef(Inst)) {
      CE = CanonExprUtils::createCanonExpr(Val->getType());
      parseAsBlob(Val, CE, Level);
      return CE;
    }
  }

  // Parse as blob if the type is not SCEVable.
  // This is currently for handling floating types.
  if (!SE->isSCEVable(Val->getType())) {
    CE = CanonExprUtils::createCanonExpr(Val->getType());
    parseAsBlob(Val, CE, Level);
  } else {
    SC = SE->getSCEV(const_cast<Value *>(Val));
    CE = CanonExprUtils::createCanonExpr(SC->getType());
    parseRecursive(SC, CE, Level, true);
  }

  return CE;
}

void HIRParser::clearTempBlobLevelMap() { CurTempBlobLevelMap.clear(); }

void HIRParser::populateBlobDDRefs(RegDDRef *Ref) {
  for (auto I = CurTempBlobLevelMap.begin(), E = CurTempBlobLevelMap.end();
       I != E; ++I) {
    auto Blob = CanonExprUtils::getBlob(I->first);
    assert(isa<SCEVUnknown>(Blob) && "Unexpected temp blob!");

    auto Symbase =
        ScalarSA->getOrAssignScalarSymbase(cast<SCEVUnknown>(Blob)->getValue());
    auto CE = CanonExprUtils::createCanonExpr(Blob->getType());

    CE->addBlob(I->first, 1);

    if (-1 == I->second) {
      CE->setNonLinear();
    } else {
      CE->setDefinedAtLevel(I->second);
    }

    auto BRef = DDRefUtils::createBlobDDRef(Symbase, CE);
    Ref->addBlobDDRef(BRef);
  }
}

RegDDRef *HIRParser::createLowerDDRef(const SCEV *BETC) {
  auto CE = CanonExprUtils::createCanonExpr(BETC->getType());
  auto Ref = DDRefUtils::createRegDDRef(getSymBaseForConstants());
  Ref->setSingleCanonExpr(CE);

  return Ref;
}

RegDDRef *HIRParser::createStrideDDRef(const SCEV *BETC) {
  auto CE = CanonExprUtils::createCanonExpr(BETC->getType(), 0, 1);
  auto Ref = DDRefUtils::createRegDDRef(getSymBaseForConstants());
  Ref->setSingleCanonExpr(CE);

  return Ref;
}

RegDDRef *HIRParser::createUpperDDRef(const SCEV *BETC, unsigned Level) {
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
  auto CE = CanonExprUtils::createCanonExpr(BETC->getType());

  parseRecursive(BETC, CE, Level, true);
  Ref->setSingleCanonExpr(CE);

  if (!CE->isSelfBlob()) {
    populateBlobDDRefs(Ref);
  }

  return Ref;
}

void HIRParser::parse(HLLoop *HLoop) {

  auto Lp = HLoop->getLLVMLoop();
  assert(Lp && "HLLoop doesn't contain LLVM loop!");

  // Upper should be parsed after imcrementing level.
  CurLevel++;

  if (SE->hasLoopInvariantBackedgeTakenCount(Lp)) {
    auto BETC = SE->getBackedgeTakenCount(Lp);

    // Initialize Lower to 0.
    auto LowerRef = createLowerDDRef(BETC);
    HLoop->setLowerDDRef(LowerRef);

    // Initialize Stride to 1.
    auto StrideRef = createStrideDDRef(BETC);
    HLoop->setStrideDDRef(StrideRef);

    // Set the upper bound
    auto UpperRef = createUpperDDRef(BETC, CurLevel);
    HLoop->setUpperDDRef(UpperRef);
  }
}

void HIRParser::parseCompare(const Value *Cond, unsigned Level,
                             CmpInst::Predicate *Pred, RegDDRef **LHSDDRef,
                             RegDDRef **RHSDDRef) {

  *LHSDDRef = *RHSDDRef = nullptr;

  if (auto ConstVal = dyn_cast<Constant>(Cond)) {
    if (ConstVal->isOneValue()) {
      *Pred = CmpInst::Predicate::FCMP_TRUE;
      return;
    } else if (ConstVal->isZeroValue()) {
      *Pred = CmpInst::Predicate::FCMP_FALSE;
      return;
    } else {
      llvm_unreachable("Unexpected conditional branch value");
    }
  } else if (auto CInst = dyn_cast<CmpInst>(Cond)) {
    *Pred = CInst->getPredicate();

    if ((*Pred == CmpInst::Predicate::FCMP_TRUE) ||
        (*Pred == CmpInst::Predicate::FCMP_FALSE)) {
      return;
    }

    *LHSDDRef = createRvalDDRef(CInst, 0, Level);
    *RHSDDRef = createRvalDDRef(CInst, 1, Level);

  } else {
    llvm_unreachable("Unexpected i1 value type!");
  }
}

void HIRParser::parse(HLIf *If) {
  CmpInst::Predicate Pred;
  RegDDRef *LHSDDRef, *RHSDDRef;

  auto SrcBB = HIR->getSrcBBlock(If);
  assert(SrcBB && "Could not find If's src basic block!");

  auto BeginPredIter = If->pred_begin();
  auto IfCond = cast<BranchInst>(SrcBB->getTerminator())->getCondition();

  parseCompare(IfCond, CurLevel, &Pred, &LHSDDRef, &RHSDDRef);

  If->replacePredicate(If->pred_begin(), Pred);
  If->setPredicateOperandDDRef(LHSDDRef, BeginPredIter, true);
  If->setPredicateOperandDDRef(RHSDDRef, BeginPredIter, false);
}

void HIRParser::collectStrides(Type *GEPType,
                               SmallVectorImpl<uint64_t> &Strides) {
  assert(isa<PointerType>(GEPType));
  GEPType = cast<PointerType>(GEPType)->getElementType();

  if (ArrayType *GEPArrType = dyn_cast<ArrayType>(GEPType)) {
    GEPType = GEPArrType->getElementType();
  }

  // Collect number of elements in each dimension
  for (; ArrayType *GEPArrType = dyn_cast<ArrayType>(GEPType);
       GEPType = GEPArrType->getElementType()) {
    Strides.push_back(GEPArrType->getNumElements());
  }

  assert((GEPType->isIntegerTy() || GEPType->isFloatingPointTy()) &&
         "Unexpected GEP type!");

  auto ElementSize = GEPType->getPrimitiveSizeInBits() / 8;

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

const GEPOperator *HIRParser::findGEPOperator(const PHINode *PtrPhi) const {

  for (unsigned I = 0, E = PtrPhi->getNumIncomingValues(); I != E; ++I) {
    auto GEPInst = dyn_cast<GetElementPtrInst>(PtrPhi->getIncomingValue(I));

    if (!GEPInst) {
      continue;
    }

    if (LI->getLoopFor(GEPInst->getParent()) !=
        LI->getLoopFor(PtrPhi->getParent())) {
      continue;
    }

    return cast<GEPOperator>(GEPInst);
  }

  llvm_unreachable("Coundn't find GEP Operator for pointer phi!");
}

unsigned HIRParser::getBitElementSize(Type *Ty) const {
  assert(isa<PointerType>(Ty) && "Invalid type!");

  auto ElTy = cast<PointerType>(Ty)->getElementType();

  return getDataLayout().getTypeSizeInBits(ElTy);
}

RegDDRef *HIRParser::createPhiBaseGEPDDRef(const PHINode *BasePhi,
                                           const GEPOperator *GEPOp,
                                           unsigned Level) {
  CanonExpr *BaseCE = nullptr, *IndexCE = nullptr;

  auto Ref = DDRefUtils::createRegDDRef(0);
  auto SC = SE->getSCEV(const_cast<PHINode *>(BasePhi));
  unsigned BitElementSize = getBitElementSize(BasePhi->getType());
  unsigned ElementSize = BitElementSize / 8;

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
      auto BaseSCEV = RecSCEV->getOperand(0);
      auto OffsetSCEV = SE->getMinusSCEV(RecSCEV, BaseSCEV);

      BaseCE = CanonExprUtils::createCanonExpr(BaseSCEV->getType());
      IndexCE = CanonExprUtils::createCanonExpr(OffsetSCEV->getType());
      parseRecursive(BaseSCEV, BaseCE, Level, true);
      parseRecursive(OffsetSCEV, IndexCE, Level, true);

      // Normalize with repsect to element size.
      IndexCE->setDenominator(IndexCE->getDenominator() * ElementSize, true);
    }
  }

  // Non-linear base is parsed as base + zero offset: (%p)[0].
  if (!BaseCE) {
    BaseCE = CanonExprUtils::createCanonExpr(BasePhi->getType());
    parseAsBlob(BasePhi, BaseCE, Level);

    auto OffsetType = Type::getIntNTy(getContext(), BitElementSize);
    IndexCE = CanonExprUtils::createCanonExpr(OffsetType);
  }

  auto StrideCE =
      CanonExprUtils::createCanonExpr(IndexCE->getType(), 0, ElementSize);

  // Here we add the other operand of GEPOperator as an offset to the index.
  if (GEPOp) {
    assert((2 == GEPOp->getNumOperands()) &&
           "Unexpected number of GEP operands!");

    SC = SE->getSCEV(const_cast<Value *>(GEPOp->getOperand(1)));
    parseRecursive(SC, IndexCE, Level, true);
  } else {
    // If we traced back to phi without encountering GEPOperator, search for it
    // in the phi operands.
    GEPOp = findGEPOperator(BasePhi);
  }

  Ref->setBaseCE(BaseCE);
  Ref->addDimension(IndexCE, StrideCE);
  Ref->setInBounds(GEPOp->isInBounds());

  return Ref;
}

RegDDRef *HIRParser::createRegularGEPDDRef(const GEPOperator *GEPOp,
                                           unsigned Level) {
  SmallVector<uint64_t, 9> Strides;

  auto Ref = DDRefUtils::createRegDDRef(0);

  auto BaseCE = parse(GEPOp->getPointerOperand(), Level);
  Ref->setBaseCE(BaseCE);

  collectStrides(GEPOp->getPointerOperandType(), Strides);

  unsigned GEPNumOp = GEPOp->getNumOperands();
  unsigned Count = Strides.size();
  auto CI = dyn_cast<ConstantInt>(GEPOp->getOperand(1));

  // Check that the number of GEP operands match with the number of strides we
  // have collected, accounting for cases where the first GEP operand is zero.
  assert(((Count == GEPNumOp - 1) ||
          (CI && CI->isZero() && (Count == GEPNumOp - 2))) &&
         "Number of subscripts snd strides do not match!");

  for (auto I = GEPNumOp - 1; Count > 0; --I, --Count) {
    auto IndexCE = parse(GEPOp->getOperand(I), Level);

    auto StrideCE = CanonExprUtils::createCanonExpr(IndexCE->getType(), 0,
                                                    Strides[Count - 1]);
    Ref->addDimension(IndexCE, StrideCE);
  }

  Ref->setInBounds(GEPOp->isInBounds());

  return Ref;
}

RegDDRef *HIRParser::createSingleElementGEPDDRef(const Value *GEPVal,
                                                 unsigned Level) {

  auto Ref = DDRefUtils::createRegDDRef(0);

  auto BaseCE = parse(GEPVal, Level);
  auto BitElementSize = getBitElementSize(GEPVal->getType());
  auto OffsetType = Type::getIntNTy(getContext(), BitElementSize);

  // Create Index of zero.
  auto IndexCE = CanonExprUtils::createCanonExpr(OffsetType);
  auto StrideCE =
      CanonExprUtils::createCanonExpr(OffsetType, 0, BitElementSize / 8);

  Ref->setBaseCE(BaseCE);
  Ref->addDimension(IndexCE, StrideCE);

  // Single element is always in bounds.
  Ref->setInBounds(true);

  return Ref;
}

// NOTE: AddRec->delinearize() doesn't work with constant bound arrays.
// TODO: handle struct GEPs.
RegDDRef *HIRParser::createGEPDDRef(const Value *Val, unsigned Level) {
  const Value *GEPVal = nullptr;
  const PHINode *BasePhi = nullptr;
  const GEPOperator *GEPOp = nullptr;
  bool IsAddressOf = false;
  RegDDRef *Ref = nullptr;

  clearTempBlobLevelMap();

  if (auto SInst = dyn_cast<StoreInst>(Val)) {
    GEPVal = SInst->getPointerOperand();
  } else if (auto LInst = dyn_cast<LoadInst>(Val)) {
    GEPVal = LInst->getPointerOperand();
  } else if (auto GEPInst = dyn_cast<GetElementPtrInst>(Val)) {
    GEPVal = GEPInst->getPointerOperand();
    BasePhi = dyn_cast<PHINode>(GEPVal);
    GEPOp = cast<GEPOperator>(GEPInst);
    IsAddressOf = true;
  } else if ((GEPOp = dyn_cast<GEPOperator>(Val))) {
    GEPVal = GEPOp->getPointerOperand();
    BasePhi = dyn_cast<PHINode>(GEPVal);
    IsAddressOf = true;
  } else {
    llvm_unreachable("Unexpected instruction!");
  }

  // TODO: We need to store this casting information for correct code
  // generation. A possible way is to store this in the base CanonExpr's DestTy,
  // when SrcTy/DestTy is introduced for CanonExprs.
  // In some cases float* is converted into int32* before loading/storing.
  if (auto BCInst = dyn_cast<BitCastInst>(GEPVal)) {
    if (!SE->isHIRCopyInst(BCInst)) {
      GEPVal = BCInst->getOperand(0);
    }
  }

  // Try to get to the phi associated with this load/store.
  if (!IsAddressOf) {
    if ((GEPOp = dyn_cast<GEPOperator>(GEPVal))) {
      BasePhi = dyn_cast<PHINode>(GEPOp->getPointerOperand());
    } else {
      BasePhi = dyn_cast<PHINode>(GEPVal);
    }
  }

  if (BasePhi) {
    Ref = createPhiBaseGEPDDRef(BasePhi, GEPOp, Level);
  } else if (GEPOp) {
    Ref = createRegularGEPDDRef(GEPOp, Level);
  } else {
    Ref = createSingleElementGEPDDRef(GEPVal, Level);
  }

  Ref->setAddressOf(IsAddressOf);

  populateBlobDDRefs(Ref);

  return Ref;
}

RegDDRef *HIRParser::createScalarDDRef(const Value *Val, unsigned Level,
                                       bool IsLval) {
  CanonExpr *CE;

  clearTempBlobLevelMap();

  auto Symbase = ScalarSA->getOrAssignScalarSymbase(Val);
  auto Ref = DDRefUtils::createRegDDRef(Symbase);

  CE = parse(Val, Level);

  Ref->setSingleCanonExpr(CE);

  if (!CE->isSelfBlob()) {
    populateBlobDDRefs(Ref);
  }

  return Ref;
}

RegDDRef *HIRParser::createRvalDDRef(const Instruction *Inst, unsigned OpNum,
                                     unsigned Level) {
  RegDDRef *Ref;
  auto OpVal = Inst->getOperand(OpNum);

  if (isa<LoadInst>(Inst) || isa<GetElementPtrInst>(Inst)) {
    Ref = createGEPDDRef(Inst, Level);
  } else if (isa<GEPOperator>(OpVal)) {
    Ref = createGEPDDRef(OpVal, Level);
  } else {
    Ref = createScalarDDRef(OpVal, Level, false);
  }

  return Ref;
}

RegDDRef *HIRParser::createLvalDDRef(const Instruction *Inst, unsigned Level) {
  RegDDRef *Ref;

  if (!isRequired(Inst)) {
    return nullptr;

  } else if (isa<StoreInst>(Inst)) {
    Ref = createGEPDDRef(Inst, Level);
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

void HIRParser::parse(HLInst *HInst) {
  RegDDRef *Ref;
  bool HasLval = false;
  auto Inst = HInst->getLLVMInstruction();
  unsigned Level = CurLevel;

  assert(!Inst->getType()->isVectorTy() && "Vector types not supported!");

  if (HInst->isInPreheaderOrPostexit()) {
    --Level;
  }

  /// Process lval
  if (HInst->hasLval()) {
    HasLval = true;

    Ref = createLvalDDRef(Inst, Level);

    if (!Ref) {
      /// Eliminate useless instructions.
      EraseSet.push_back(HInst);
      return;
    }

    HInst->setLvalDDRef(Ref);
  }

  unsigned NumRvalOp = getNumRvalOperands(HInst);

  /// Process rvals
  for (unsigned I = 0; I < NumRvalOp; ++I) {

    if (isa<SelectInst>(Inst) && (I == 0)) {
      CmpInst::Predicate Pred;
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

void HIRParser::eraseUselessNodes() {
  for (auto &I : EraseSet) {
    HLNodeUtils::erase(I);
  }
}

bool HIRParser::runOnFunction(Function &F) {
  Func = &F;
  SE = &getAnalysis<ScalarEvolution>();
  LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  ScalarSA = &getAnalysis<ScalarSymbaseAssignment>();
  HIR = &getAnalysis<HIRCreation>();
  LF = &getAnalysis<LoopFormation>();

  HLUtils::setHIRParser(this);

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
  CurTempBlobLevelMap.clear();
}

LLVMContext &HIRParser::getContext() const { return Func->getContext(); }

const DataLayout &HIRParser::getDataLayout() const {
  return Func->getParent()->getDataLayout();
}

void HIRParser::print(raw_ostream &OS, const Module *M) const {
  HIR->printWithIRRegion(OS);
}

void HIRParser::verifyAnalysis() const {
  /// TODO: implement later
}
