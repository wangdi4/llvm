//===----- HIRParser.cpp - Parses SCEVs into CanonExprs -------------------===//
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

#include "llvm/IR/Dominators.h"
#include "llvm/IR/GetElementPtrTypeIterator.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Operator.h"

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRParser.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRRegionIdentification.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Utils/CanonExprUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"

#include "HIRCreation.h"
#include "HIRLoopFormation.h"
#include "HIRScalarSymbaseAssignment.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "hir-parser"

static cl::opt<bool> RemoveDebugIntrinsics(
    DEBUG_TYPE "-remove-dbg-intrin", cl::init(true), cl::Hidden,
    cl::desc("Remove llvm.dbg.* intrinsics from HIR (default: true)"));

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
    auto BB = HIRC.getSrcBBlock(If);
    auto Term = BB->getTerminator();
    auto BrInst = dyn_cast<BranchInst>(Term);

    return cast<Instruction>(BrInst->getCondition());

  } else if (auto Switch = dyn_cast<HLSwitch>(CurNode)) {
    auto BB = HIRC.getSrcBBlock(Switch);
    return BB->getTerminator();
  }

  llvm_unreachable("Unexpected CurNode type!");
}

bool HIRParser::foundInBlobTable(unsigned Symbase) const {
  for (auto &BlobPair : BlobTable) {
    if (BlobPair.second == Symbase) {
      return true;
    }
  }

  return false;
}

bool HIRParser::validBlobSymbasePair(BlobTy Blob, unsigned Symbase) const {
  if (isTempBlob(Blob)) {
    assert((Symbase > GenericRvalSymbase) && "Temp has invalid symbase!");
    assert(!foundInBlobTable(Symbase) &&
           "Symbase is already present in blob table!");
  } else if (isa<SCEVConstant>(Blob)) {
    assert(Symbase == ConstantSymbase && "Constant must have correct symbase.");
  }

  return true;
}

unsigned HIRParser::findOrInsertBlobImpl(BlobTy Blob, unsigned Symbase,
                                         bool Insert, bool ReturnSymbase,
                                         BlobTy NewBlob) {
  assert(Blob && "Blob is null!");
  assert(!(NewBlob && ReturnSymbase) &&
         "Invalid arguments to findOrInsertBlobImpl()!");

  auto It = BlobToIndexMap.find(Blob);

  if (It != BlobToIndexMap.end()) {
    assert((getBlob(It->second) == It->first) &&
           "Inconsistent blob index mapping encountered!");

    // Replace existing Blob with NewBlob.
    if (NewBlob) {
      auto BlobIndex = It->second;
      BlobToIndexMap.erase(It);

      // Insert new blob BlobToIndexMap.
      BlobToIndexMap.insert(std::make_pair(NewBlob, BlobIndex));

      // Replace blob in BlobTable.
      BlobTable[BlobIndex - 1].first = NewBlob;

      return ReturnSymbase ? getTempBlobSymbase(BlobIndex) : BlobIndex;
    }

    return ReturnSymbase ? getTempBlobSymbase(It->second) : It->second;
  }

  if (Insert) {
    assert(!NewBlob && "Attempt to insert new blob!");
    assert(validBlobSymbasePair(Blob, Symbase) &&
           "Invalid Blob/Symbase combination!");

    BlobTable.push_back(std::make_pair(Blob, Symbase));

    unsigned Index = BlobTable.size();
    // Store blob ptr and index mapping for faster lookup.
    BlobToIndexMap.insert(std::make_pair(Blob, Index));

    // Store symbase to blob index mapping for faster lookup.
    if (Symbase > GenericRvalSymbase) {
      auto Ret = SymbaseToIndexMap.insert(std::make_pair(Symbase, Index));
      assert(Ret.second && "Duplicate insertion in symbase to index map!");
      (void)Ret;
    }

    return ReturnSymbase ? Symbase : Index;
  }

  return ReturnSymbase ? InvalidSymbase : InvalidBlobIndex;
}

unsigned HIRParser::findBlob(BlobTy Blob) {
  return findOrInsertBlobImpl(Blob, InvalidSymbase, false, false);
}

unsigned HIRParser::findTempBlobSymbase(BlobTy Blob) {
  return findOrInsertBlobImpl(Blob, InvalidSymbase, false, true);
}

unsigned HIRParser::findTempBlobIndex(unsigned Symbase) const {
  auto It = SymbaseToIndexMap.find(Symbase);

  return (It != SymbaseToIndexMap.end()) ? It->second : InvalidBlobIndex;
}

unsigned HIRParser::findOrInsertTempBlobIndex(unsigned Symbase) {

  auto It = SymbaseToIndexMap.find(Symbase);

  if (It != SymbaseToIndexMap.end()) {
    return It->second;
  }
  // Some lvals may not be parsed as blobs during parsing, insert them as blobs
  // now.
  assert((Symbase < ScalarSA.getMaxScalarSymbase()) &&
         "Blob index for symbase not found!");

  auto Val = ScalarSA.getBaseScalar(Symbase);
  auto Blob = SE.getUnknown(const_cast<Value *>(Val));

  return findOrInsertBlob(Blob, Symbase);
}

unsigned HIRParser::findOrInsertBlob(BlobTy Blob, unsigned Symbase) {
  return findOrInsertBlobImpl(Blob, Symbase, true, false);
}

unsigned HIRParser::updateTempBlob(BlobTy OldBlob, BlobTy NewBlob,
                                   unsigned Symbase) {
  return findOrInsertBlobImpl(OldBlob, Symbase, false, false, NewBlob);
}

BlobTy HIRParser::getBlob(unsigned Index) const {
  assert(isBlobIndexValid(Index) && "Index is out of bounds!");
  return BlobTable[Index - 1].first;
}

unsigned HIRParser::getTempBlobSymbase(unsigned Index) const {
  assert(isBlobIndexValid(Index) && "Index is out of bounds!");
  auto Symbase = BlobTable[Index - 1].second;

  assert(Symbase > ConstantSymbase && "Blob is not a temp!");
  return Symbase;
}

bool HIRParser::isBlobIndexValid(unsigned Index) const {
  return ((Index > InvalidBlobIndex) && (Index <= BlobTable.size()));
}

void HIRParser::mapBlobsToIndices(const SmallVectorImpl<BlobTy> &Blobs,
                                  SmallVectorImpl<unsigned> &Indices) {
  for (auto &I : Blobs) {
    unsigned Index = findBlob(I);
    assert(Index && "Could not find index of temp blob!");

    Indices.push_back(Index);
  }
}

bool HIRParser::isTempBlob(BlobTy Blob) {
  if (auto UnknownSCEV = dyn_cast<SCEVUnknown>(Blob)) {
    Type *Ty;
    Constant *FieldNo;

    if (!UnknownSCEV->isSizeOf(Ty) && !UnknownSCEV->isAlignOf(Ty) &&
        !UnknownSCEV->isOffsetOf(Ty, FieldNo) &&
        !HIRScalarSymbaseAssignment::isConstant(UnknownSCEV->getValue()) &&
        !BlobUtils::isMetadataBlob(Blob, nullptr)) {
      return true;
    }
  }

  return false;
}

void HIRParser::insertBlobHelper(BlobTy Blob, unsigned Symbase, bool Insert,
                                 unsigned *NewBlobIndex) {
  if (Insert) {
    Symbase = isa<SCEVConstant>(Blob) ? ConstantSymbase : Symbase;
    unsigned BlobIndex = findOrInsertBlob(Blob, Symbase);

    if (NewBlobIndex) {
      *NewBlobIndex = BlobIndex;
    }
  }
}

BlobTy HIRParser::createBlob(Value *Val, unsigned Symbase, bool Insert,
                             unsigned *NewBlobIndex) {
  assert(Val && "Value cannot be null!");

  auto Blob = SE.getUnknown(Val);

  insertBlobHelper(Blob, Symbase, Insert, NewBlobIndex);

  return Blob;
}

BlobTy HIRParser::createBlob(int64_t Val, Type *Ty, bool Insert,
                             unsigned *NewBlobIndex) {

  assert(Ty && "Type cannot be null!");
  auto Blob = SE.getConstant(Ty, Val, false);

  insertBlobHelper(Blob, ConstantSymbase, Insert, NewBlobIndex);

  return Blob;
}

BlobTy HIRParser::createAddBlob(BlobTy LHS, BlobTy RHS, bool Insert,
                                unsigned *NewBlobIndex) {
  assert(LHS && RHS && "Blob cannot be null!");

  auto Blob = SE.getAddExpr(LHS, RHS);

  insertBlobHelper(Blob, InvalidSymbase, Insert, NewBlobIndex);

  return Blob;
}

BlobTy HIRParser::createMinusBlob(BlobTy LHS, BlobTy RHS, bool Insert,
                                  unsigned *NewBlobIndex) {
  assert(LHS && RHS && "Blob cannot be null!");

  auto Blob = SE.getMinusSCEV(LHS, RHS);

  insertBlobHelper(Blob, InvalidSymbase, Insert, NewBlobIndex);

  return Blob;
}

BlobTy HIRParser::createMulBlob(BlobTy LHS, BlobTy RHS, bool Insert,
                                unsigned *NewBlobIndex) {
  assert(LHS && RHS && "Blob cannot be null!");

  auto Blob = SE.getMulExpr(LHS, RHS);

  insertBlobHelper(Blob, InvalidSymbase, Insert, NewBlobIndex);

  return Blob;
}

BlobTy HIRParser::createUDivBlob(BlobTy LHS, BlobTy RHS, bool Insert,
                                 unsigned *NewBlobIndex) {
  assert(LHS && RHS && "Blob cannot be null!");

  auto Blob = SE.getUDivExpr(LHS, RHS);

  insertBlobHelper(Blob, InvalidSymbase, Insert, NewBlobIndex);

  return Blob;
}

BlobTy HIRParser::createTruncateBlob(BlobTy Blob, Type *Ty, bool Insert,
                                     unsigned *NewBlobIndex) {
  assert(Blob && "Blob cannot be null!");
  assert(Ty && "Type cannot be null!");

  auto NewBlob = SE.getTruncateExpr(Blob, Ty);

  insertBlobHelper(NewBlob, InvalidSymbase, Insert, NewBlobIndex);

  return NewBlob;
}

BlobTy HIRParser::createZeroExtendBlob(BlobTy Blob, Type *Ty, bool Insert,
                                       unsigned *NewBlobIndex) {
  assert(Blob && "Blob cannot be null!");
  assert(Ty && "Type cannot be null!");

  auto NewBlob = SE.getZeroExtendExpr(Blob, Ty);

  insertBlobHelper(NewBlob, InvalidSymbase, Insert, NewBlobIndex);

  return NewBlob;
}

BlobTy HIRParser::createSignExtendBlob(BlobTy Blob, Type *Ty, bool Insert,
                                       unsigned *NewBlobIndex) {
  assert(Blob && "Blob cannot be null!");
  assert(Ty && "Type cannot be null!");

  auto NewBlob = SE.getSignExtendExpr(Blob, Ty);

  insertBlobHelper(NewBlob, InvalidSymbase, Insert, NewBlobIndex);

  return NewBlob;
}

BlobTy HIRParser::createCastBlob(BlobTy Blob, bool IsSExt, Type *Ty,
                                 bool Insert, unsigned *NewBlobIndex) {
  assert(Blob && "Blob cannot be null!");
  assert(Ty && "Type cannot be null!");
  assert(Ty->isIntegerTy() && "Invalid type!");

  BlobTy NewBlob = nullptr;

  if (Ty->getPrimitiveSizeInBits() >
      // In some case blob can be pointer type as SCEV is not very thorough in
      // changing pointer to integer type for cases like (ptr1 - ptr2 + 1).
      getDataLayout().getTypeSizeInBits(Blob->getType())) {
    NewBlob = IsSExt ? SE.getSignExtendExpr(Blob, Ty)
                     : SE.getZeroExtendExpr(Blob, Ty);
  } else {
    NewBlob = SE.getTruncateExpr(Blob, Ty);
  }

  insertBlobHelper(NewBlob, InvalidSymbase, Insert, NewBlobIndex);

  return NewBlob;
}

BlobTy HIRParser::createSMinBlob(BlobTy BlobA, BlobTy BlobB, bool Insert,
                                 unsigned *NewBlobIndex) {
  assert(BlobA && BlobB && "Blob cannot be null!");

  auto NewBlob = SE.getSMinExpr(BlobA, BlobB);

  insertBlobHelper(NewBlob, InvalidSymbase, Insert, NewBlobIndex);

  return NewBlob;
}

BlobTy HIRParser::createSMaxBlob(BlobTy BlobA, BlobTy BlobB, bool Insert,
                                 unsigned *NewBlobIndex) {
  assert(BlobA && BlobB && "Blob cannot be null!");

  auto NewBlob = SE.getSMaxExpr(BlobA, BlobB);

  insertBlobHelper(NewBlob, InvalidSymbase, Insert, NewBlobIndex);

  return NewBlob;
}

BlobTy HIRParser::createUMinBlob(BlobTy BlobA, BlobTy BlobB, bool Insert,
                                 unsigned *NewBlobIndex) {
  assert(BlobA && BlobB && "Blob cannot be null!");

  auto NewBlob = SE.getUMinExpr(BlobA, BlobB);

  insertBlobHelper(NewBlob, InvalidSymbase, Insert, NewBlobIndex);

  return NewBlob;
}

BlobTy HIRParser::createUMaxBlob(BlobTy BlobA, BlobTy BlobB, bool Insert,
                                 unsigned *NewBlobIndex) {
  assert(BlobA && BlobB && "Blob cannot be null!");

  auto NewBlob = SE.getUMaxExpr(BlobA, BlobB);

  insertBlobHelper(NewBlob, InvalidSymbase, Insert, NewBlobIndex);

  return NewBlob;
}

bool HIRParser::contains(BlobTy Blob, BlobTy SubBlob) const {
  assert(Blob && "Blob cannot be null!");
  assert(SubBlob && "SubBlob cannot be null!");

  return SE.hasOperand(Blob, SubBlob);
}

class HIRParser::TempBlobCollector {
private:
  const HIRParser &HIRP;
  SmallVectorImpl<BlobTy> &TempBlobs;

public:
  TempBlobCollector(const HIRParser &HIRP, SmallVectorImpl<BlobTy> &TempBlobs)
      : HIRP(HIRP), TempBlobs(TempBlobs) {}

  ~TempBlobCollector() {}

  bool follow(const SCEV *SC) const {

    if (HIRP.isTempBlob(SC)) {
      TempBlobs.push_back(SC);
    }

    return !isDone();
  }

  bool isDone() const { return false; }
};

void HIRParser::collectTempBlobs(BlobTy Blob,
                                 SmallVectorImpl<BlobTy> &TempBlobs) const {
  TempBlobCollector TBC(*this, TempBlobs);
  SCEVTraversal<TempBlobCollector> Collector(TBC);
  Collector.visitAll(Blob);
}

bool HIRParser::replaceTempBlob(unsigned BlobIndex, unsigned TempIndex,
                                BlobTy ByBlob, unsigned &NewBlobIndex,
                                int64_t &SimplifiedConstant) {
  auto TempBlob = getBlob(TempIndex);

  assert(isTempBlob(TempBlob) && "TempIndex is not a temp!");

  const SCEVConstant *ConstantBlob = dyn_cast<SCEVConstant>(ByBlob);

  if (BlobIndex == TempIndex) {
    if (ConstantBlob) {
      NewBlobIndex = InvalidBlobIndex;
      SimplifiedConstant = ConstantBlob->getAPInt().getSExtValue();
    } else {
      NewBlobIndex = findBlob(ByBlob);
    }

    return true;
  }

  auto Blob = getBlob(BlobIndex);

  Value *ReplaceByValue = ConstantBlob ? ConstantBlob->getValue()
                                       : cast<SCEVUnknown>(ByBlob)->getValue();

  ValueToValueMap Map;
  Map.insert(
      std::make_pair(cast<SCEVUnknown>(TempBlob)->getValue(), ReplaceByValue));

  auto NewBlob = SCEVParameterRewriter::rewrite(Blob, SE, Map, true);

  if (Blob == NewBlob) {
    NewBlobIndex = BlobIndex;
    return false;
  }

  if (const SCEVConstant *ConstantBlob = dyn_cast<SCEVConstant>(NewBlob)) {
    NewBlobIndex = InvalidBlobIndex;
    SimplifiedConstant = ConstantBlob->getAPInt().getSExtValue();
  } else {
    NewBlobIndex = findOrInsertBlob(NewBlob, InvalidSymbase);
  }

  return true;
}

bool HIRParser::replaceTempBlobByConstant(unsigned BlobIndex,
                                          unsigned TempIndex, int64_t Constant,
                                          unsigned &NewBlobIndex,
                                          int64_t &SimplifiedConstant) {
  auto TempBlob = getBlob(TempIndex);
  BlobTy ConstantBlob = SE.getConstant(TempBlob->getType(), Constant, true);
  return replaceTempBlob(BlobIndex, TempIndex, ConstantBlob, NewBlobIndex,
                         SimplifiedConstant);
}

static bool isUMaxBlob(BlobTy Blob) { return isa<SCEVUMaxExpr>(Blob); }
static bool isUMinBlob(BlobTy Blob) { return isa<SCEVUMinExpr>(Blob); }
static bool isMinMaxBlob(BlobTy Blob) { return isa<SCEVMinMaxExpr>(Blob); }

/// Returns true if this Blob represents a min/max expr with an AddRec
/// Operand.
static bool isMinMaxWithAddRecOperand(BlobTy Blob) {
  if (!isMinMaxBlob(Blob)) {
    return false;
  }

  for (const auto *Op : cast<SCEVNAryExpr>(Blob)->operands()) {
    if (isa<SCEVAddRecExpr>(Op)) {
      return true;
    }
  }

  return false;
}

bool HIRParser::getMinBlobValue(BlobTy Blob, int64_t &Val) const {
  auto Range = SE.getSignedRange(Blob);

  if (!Range.isFullSet()) {
    Val = Range.getSignedMin().getSExtValue();

    // Conservatively return false for large values to prevent overflow in
    // client side computation as well as to prevent use of overly conservative
    // values like INT_MIN/INT_MAX.
    if (Val < SHRT_MIN || Val > SHRT_MAX) {
      return false;
    }

    return true;
  }

  if (isUMaxBlob(Blob) || isUMinBlob(Blob)) {
    Val = 0;
    return true;
  }

  return false;
}

bool HIRParser::getMaxBlobValue(BlobTy Blob, int64_t &Val) const {
  auto Range = SE.getSignedRange(Blob);

  if (Range.isFullSet()) {
    return false;
  }

  Val = Range.getSignedMax().getSExtValue();

  // Conservatively return false for large values to prevent overflow in client
  // side computation as well as to prevent use of overly conservative values
  // like INT_MIN/INT_MAX.
  if (Val < SHRT_MIN || Val > SHRT_MAX) {
    return false;
  }

  return true;
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

class LiveInChecker {
private:
  const HLRegion *Reg;
  bool IsLiveIn;

public:
  LiveInChecker(const HLRegion *Reg) : Reg(Reg), IsLiveIn(true) {}

  bool follow(const SCEV *SC) {
    auto *UnknownSC = dyn_cast<SCEVUnknown>(SC);

    if (!UnknownSC) {
      return true;
    }

    auto Inst = dyn_cast<Instruction>(UnknownSC->getValue());

    if (!Inst) {
      return false;
    }

    IsLiveIn = !Reg->containsBBlock(Inst->getParent());
    return false;
  }

  bool isDone() const { return !isLiveIn(); }

  bool isLiveIn() const { return IsLiveIn; }
};

static bool isRegionLiveIn(const HLRegion *Reg, const SCEV *SC) {
  LiveInChecker LIC(Reg);
  SCEVTraversal<LiveInChecker> Checker(LIC);
  Checker.visitAll(SC);

  return LIC.isLiveIn();
}

/// This class is used to process blob which is being added to the CanonExpr.
/// It performs several functions-
/// 1) Reverse engineers SCEVAddRecExprs to SCEVUnknowns.
/// 2) Sets/updates defined at level for CanonExpr based on temp blobs present
/// in the blob.
/// 3) Identifies and marks region livein temps.
/// 4) Maps values into their base values. This is done to maintain a 1:1
/// mapping between blob index and symbase.
///
/// TODO: ScalarEvolution now maintains a reverse mapping of SCEV to set of
/// Values. Investigate if the new setup can replace the reverse engineering
/// setup.
class HIRParser::BlobProcessor : public SCEVRewriteVisitor<BlobProcessor> {
private:
  HIRParser *HIRP;
  CanonExpr *CE;
  unsigned NestingLevel;
  bool SafeMode;
  bool Failed;

  // Used to mark visited instructions during traceback in findOrigInst().
  SmallPtrSet<const Instruction *, 16> VisitedInsts;

public:
  BlobProcessor(HIRParser *HIRPar, CanonExpr *CE, unsigned Level)
      : SCEVRewriteVisitor(HIRPar->SE), HIRP(HIRPar), CE(CE),
        NestingLevel(Level), SafeMode(false), Failed(false) {}

  /// Returns true if \p Blob can be processed without encountering failure.
  bool canProcessSafely(BlobTy Blob);

  /// Processes \p Blob and returns the resulting mapped blob.
  BlobTy process(BlobTy Blob) { return visit(Blob); }

  // Returns a mapped SCEV for \p MinMax which would result in a cleaner HIR.
  const SCEV *getProfitableMinMaxExprMapping(const SCEV *MinMax);

  /// Override base class functions.
  const SCEV *visitAddExpr(const SCEVAddExpr *Add);
  const SCEV *visitZeroExtendExpr(const SCEVZeroExtendExpr *ZExt);
  const SCEV *visitMulExpr(const SCEVMulExpr *Mul);
  const SCEV *visitAddRecExpr(const SCEVAddRecExpr *AddRec);
  const SCEV *visitSMaxExpr(const SCEVSMaxExpr *Max);
  const SCEV *visitSMinExpr(const SCEVSMinExpr *Max);
  const SCEV *visitUMaxExpr(const SCEVUMaxExpr *Max);
  const SCEV *visitUMinExpr(const SCEVUMinExpr *Max);

  /// Returns the SCEV of the base value associated with the incoming SCEV's
  /// value. All the temp blob related processing is performed here.
  const SCEV *visitUnknown(const SCEVUnknown *Unknown);

  /// Returns a substitute SCEV for SC. Returns null if it cannot do so.
  const SCEV *getSubstituteSCEV(const SCEV *SC);

  /// Searches for an intruction corresponding to SC in the ValueSet maintained
  /// by ScalarEvolution.
  const Instruction *searchSCEVValues(const SCEV *SC) const;

  /// Recursive function to trace back from the current instruction to find an
  /// instruction which can represent SC with a combination of basic operations
  /// like truncation, negation etc applied on top of the SCEV. We are trying to
  /// reverse engineer SCEV analysis here.
  const Instruction *findOrigInst(const Instruction *CurInst, const SCEV *SC,
                                  bool *IsTruncOrSExt, bool *IsZExt,
                                  bool *IsNegation,
                                  SCEVConstant **ConstMultiplier,
                                  SCEV **Additive);

  /// Returns true if NewSCEV can replace OrigSCEV in the SCEV tree with a
  /// combination of basic operations like truncation, negation etc applied on
  /// top of NewSCEV. To replace a linear AddRec type OrigSCEV, NewSCEV should
  /// have identical operands (except the fist operand) and have identical or
  /// stronger wrap flags.
  bool isReplacable(const SCEV *OrigSCEV, const SCEV *NewSCEV,
                    bool *IsTruncOrSExt, bool *IsZExt, bool *IsNegation,
                    SCEVConstant **ConstMultiplier, SCEV **Additive) const;

  /// Returns the SCEVConstant representing signed division result of LHS and
  /// RHS.
  const SCEVConstant *getSDiv(const SCEVConstant *LHS,
                              const SCEVConstant *RHS) const;

  /// Returns constant multiplier which when applied to AddRec may yield
  /// MulAddRec, otherwise returns nullptr. This function does not perform all
  /// sanity checks so the returned result may be incorrect. Caller is
  /// responsible for sanity checking.
  const SCEVConstant *
  getPossibleMultiplier(const SCEVAddRecExpr *AddRec,
                        const SCEVAddRecExpr *MulAddRec) const;

  /// Implements AddRec specific checks for replacement.
  bool isReplacableAddRec(const SCEVAddRecExpr *OrigAddRec,
                          const SCEVAddRecExpr *NewAddRec,
                          SCEV::NoWrapFlags WrapFlags,
                          SCEVConstant **ConstMultiplier,
                          SCEV **Additive) const;
};

bool HIRParser::BlobProcessor::canProcessSafely(BlobTy Blob) {
  SafeMode = true;

  process(Blob);

  SafeMode = false;

  bool HasFailed = Failed;
  Failed = false;

  return !HasFailed;
}

const SCEV *
HIRParser::BlobProcessor::getProfitableMinMaxExprMapping(const SCEV *MinMax) {
  // This mapping is for profitability (not legality) so we can skip it in safe
  // mode.
  if (SafeMode) {
    return nullptr;
  }

  // This mapping recovers original (select) instruction from min exprs with
  // AddRec operands. This is more profitable as it avoids creation of IV blobs.
  if (isMinMaxWithAddRecOperand(MinMax)) {
    if (auto SubSCEV = getSubstituteSCEV(MinMax)) {
      return SubSCEV;
    }
  }

  // Avoids region livein max operations.
  if (isMinMaxBlob(MinMax) && isRegionLiveIn(HIRP->CurRegion, MinMax)) {
    if (auto SubSCEV = getSubstituteSCEV(MinMax)) {
      return SubSCEV;
    }
  }

  return nullptr;
}

const SCEV *HIRParser::BlobProcessor::visitAddExpr(const SCEVAddExpr *Add) {
  if (auto *MappedSC = getProfitableMinMaxExprMapping(Add)) {
    return MappedSC;
  }

  return SCEVRewriteVisitor<BlobProcessor>::visitAddExpr(Add);
}

const SCEV *
HIRParser::BlobProcessor::visitZeroExtendExpr(const SCEVZeroExtendExpr *ZExt) {
  // In some cases we have a value for zero extension of linear SCEV but not
  // the linear SCEV itself because the original src code IV has been widened
  // by induction variable simplification. So we look for such values here.
  if (isa<SCEVAddRecExpr>(ZExt->getOperand())) {
    if (auto SubSCEV = getSubstituteSCEV(ZExt)) {
      return SubSCEV;
    }
  }

  return SCEVRewriteVisitor<BlobProcessor>::visitZeroExtendExpr(ZExt);
}

const SCEV *HIRParser::BlobProcessor::visitMulExpr(const SCEVMulExpr *Mul) {
  // This is to catch cases like this-
  //
  // %126 = trunc i64 %indvars.iv857 to i32
  //   -->  {0,+,2}<%for.body.525>
  // %rem530815 = and i32 %126, 30
  //   -->  (2 * (zext i4 {0,+,1}<%for.body.525> to i32))
  //
  // TODO: investigate SCEV representation of bitwise operators in detail.
  if (Mul->getNumOperands() == 2) {
    auto ZExt = dyn_cast<SCEVZeroExtendExpr>(Mul->getOperand(1));

    if (ZExt && isa<SCEVAddRecExpr>(ZExt->getOperand())) {
      if (auto SubSCEV = getSubstituteSCEV(Mul)) {
        return SubSCEV;
      }
    }
  }

  if (auto *MappedSC = getProfitableMinMaxExprMapping(Mul)) {
    return MappedSC;
  }

  return SCEVRewriteVisitor<BlobProcessor>::visitMulExpr(Mul);
}

/// Returns the SCEVUnknown version of the value which represents this AddRec.
const SCEV *
HIRParser::BlobProcessor::visitAddRecExpr(const SCEVAddRecExpr *AddRec) {
  const SCEV *SubSCEV = getSubstituteSCEV(AddRec);

  if (!SubSCEV) {

    if (SafeMode) {
      Failed = true;
      SubSCEV = AddRec;

    } else {
      llvm_unreachable("Instuction corresponding to linear SCEV not found!");
    }
  }

  return SubSCEV;
}

const SCEV *HIRParser::BlobProcessor::visitSMaxExpr(const SCEVSMaxExpr *Max) {
  // This mapping recovers original (select) instruction from max exprs with
  // AddRec operands. This is more profitable as it avoids creation of IV blobs.

  if (auto *MappedSC = getProfitableMinMaxExprMapping(Max)) {
    return MappedSC;
  }

  return SCEVRewriteVisitor<BlobProcessor>::visitSMaxExpr(Max);
}

const SCEV *HIRParser::BlobProcessor::visitSMinExpr(const SCEVSMinExpr *Min) {
  // This mapping recovers original (select) instruction from max exprs with
  // AddRec operands. This is more profitable as it avoids creation of IV blobs.

  if (auto *MappedSC = getProfitableMinMaxExprMapping(Min)) {
    return MappedSC;
  }

  return SCEVRewriteVisitor<BlobProcessor>::visitSMinExpr(Min);
}

const SCEV *HIRParser::BlobProcessor::visitUMaxExpr(const SCEVUMaxExpr *Max) {
  // This mapping recovers original (select) instruction from max exprs with
  // AddRec operands. This is more profitable as it avoids creation of IV blobs.

  if (auto *MappedSC = getProfitableMinMaxExprMapping(Max)) {
    return MappedSC;
  }

  return SCEVRewriteVisitor<BlobProcessor>::visitUMaxExpr(Max);
}

const SCEV *HIRParser::BlobProcessor::visitUMinExpr(const SCEVUMinExpr *Min) {
  // This mapping recovers original (select) instruction from max exprs with
  // AddRec operands. This is more profitable as it avoids creation of IV blobs.

  if (auto *MappedSC = getProfitableMinMaxExprMapping(Min)) {
    return MappedSC;
  }

  return SCEVRewriteVisitor<BlobProcessor>::visitUMinExpr(Min);
}

const SCEV *HIRParser::BlobProcessor::visitUnknown(const SCEVUnknown *Unknown) {
  auto BaseBlob = Unknown;

  if (!SafeMode && HIRP->isTempBlob(Unknown)) {
    BaseBlob = HIRP->processTempBlob(Unknown, CE, NestingLevel);
  }

  return BaseBlob;
}

const SCEV *HIRParser::BlobProcessor::getSubstituteSCEV(const SCEV *SC) {
  const Instruction *OrigInst = nullptr;
  SCEV *Additive = nullptr;
  SCEVConstant *ConstMultiplier = nullptr;
  bool IsNegation = false;
  bool IsTruncOrSExt = false;
  bool IsZExt = false;

  if (SafeMode && Failed) {
    return nullptr;
  }

  OrigInst = findOrigInst(nullptr, SC, &IsTruncOrSExt, &IsZExt, &IsNegation,
                          &ConstMultiplier, &Additive);

  if (!OrigInst) {
    return nullptr;
  }

  auto NewSCEV = HIRP->SE.getUnknown(const_cast<Instruction *>(OrigInst));

  // NOTE: The order of truncation, negation, multiplication and addition
  // matters.
  if (IsTruncOrSExt) {
    if (NewSCEV->getType()->getPrimitiveSizeInBits() <
        SC->getType()->getPrimitiveSizeInBits()) {
      NewSCEV = HIRP->SE.getSignExtendExpr(NewSCEV, SC->getType());
    } else {
      NewSCEV = HIRP->SE.getTruncateExpr(NewSCEV, SC->getType());
    }

  } else if (IsZExt) {
    NewSCEV = HIRP->SE.getZeroExtendExpr(NewSCEV, SC->getType());
  }

  if (IsNegation) {
    NewSCEV = HIRP->SE.getNegativeSCEV(NewSCEV);
  }

  if (ConstMultiplier) {
    NewSCEV = HIRP->SE.getMulExpr(ConstMultiplier, NewSCEV);
  }

  if (Additive) {
    NewSCEV = HIRP->SE.getAddExpr(Additive, NewSCEV);
  }

  // Convert value to base value before returning.
  return visit(NewSCEV);
}

const Instruction *
HIRParser::BlobProcessor::searchSCEVValues(const SCEV *SC) const {
  auto ValSet = HIRP->SE.getSCEVValues(SC);

  if (!ValSet) {
    return nullptr;
  }

  auto CurInst = HIRP->getCurInst();

  // Look for an instruction in the set which dominates current instruction as
  // it should appear lexically before the current instruction.
  for (auto &ValOffsetPair : (*ValSet)) {

    if (ValOffsetPair.second) {
      continue;
    }

    auto Inst = dyn_cast<Instruction>(ValOffsetPair.first);

    if (!Inst) {
      continue;
    }

    if (!HIRP->SE.getHIRMetadata(Inst, ScalarEvolution::HIRLiveKind::LiveIn) &&
        HIRP->DT.dominates(Inst, CurInst)) {
      return Inst;
    }
  }

  return nullptr;
}

const Instruction *HIRParser::BlobProcessor::findOrigInst(
    const Instruction *CurInst, const SCEV *SC, bool *IsTruncOrSExt,
    bool *IsZExt, bool *IsNegation, SCEVConstant **ConstMultiplier,
    SCEV **Additive) {

  bool IsLiveInCopy = false;
  bool FirstInst = false;

  if (!CurInst) {
    // Check if ScalarEvolution can provide OrigInst for us.
    if (auto OrigInst = searchSCEVValues(SC)) {
      return OrigInst;
    }

    CurInst = HIRP->getCurInst();
    IsLiveInCopy =
        HIRP->SE.getHIRMetadata(CurInst, ScalarEvolution::HIRLiveKind::LiveIn);
    FirstInst = true;

    // We just started the traceback, clear previous entries.
    VisitedInsts.clear();
  }

  // We should not be checking the SCEV of the livein copy instruction as it
  // should inherit the SCEV of the rval.
  if (!IsLiveInCopy && HIRP->SE.isSCEVable(CurInst->getType())) {
    auto CurSCEV = HIRP->getSCEV(const_cast<Instruction *>(CurInst));

    // First instruction can only be an exact match. A partial match means that
    // an operand has been parsed in terms of the instruction it is used in!
    // Consider this CurInst-
    // %a = %b + 1
    // The SCEV form of %a might be (%b + 1). This doesn't mean that we can
    // reverse engineer %b as (%a - 1)!
    if (FirstInst) {
      if (HIRP->parsingScalarLval() && (CurSCEV == SC)) {
        return CurInst;
      }
      // Original instruction should dominate the current instruction.
    } else if (HIRP->DT.dominates(CurInst, HIRP->getCurInst()) &&
               isReplacable(SC, CurSCEV, IsTruncOrSExt, IsZExt, IsNegation,
                            ConstMultiplier, Additive)) {
      return CurInst;
    }
  }

  // Insert CurInst in visited instruction set.
  VisitedInsts.insert(CurInst);

  for (auto I = CurInst->op_begin(), E = CurInst->op_end(); I != E; ++I) {

    auto OpInst = dyn_cast<Instruction>(I);

    if (!OpInst) {
      continue;
    }

    // Skip if already visited.
    if (VisitedInsts.count(OpInst)) {
      continue;
    }

    // Limit trace back to these instruction types. They roughly correspond to
    // instruction types in SE.createSCEV().
    // Looks like ScalarEvolution can parse CmpInsts, if necessary to compute
    // backedge taken count of loops, so including it in the list as well.
    if (!isa<BinaryOperator>(OpInst) && !isa<CastInst>(OpInst) &&
        !isa<GEPOrSubsOperator>(OpInst) && !isa<PHINode>(OpInst) &&
        !isa<SelectInst>(OpInst) && !isa<CmpInst>(OpInst)) {
      continue;
    }

    auto OrigInst = findOrigInst(OpInst, SC, IsTruncOrSExt, IsZExt, IsNegation,
                                 ConstMultiplier, Additive);

    if (OrigInst) {
      return OrigInst;
    }
  }

  return nullptr;
}

static bool isReplacableUsingConstantAdditive(const SCEV *OrigSCEV,
                                              const SCEV *NewSCEV,
                                              ScalarEvolution &SE,
                                              SCEV **Additive) {

  if (OrigSCEV->getType() != NewSCEV->getType()) {
    return false;
  }

  auto *Add = SE.getMinusSCEV(OrigSCEV, NewSCEV);

  if (!isa<SCEVConstant>(Add)) {
    return false;
  }

  *Additive = const_cast<SCEV *>(Add);

  return true;
}

bool HIRParser::BlobProcessor::isReplacable(const SCEV *OrigSCEV,
                                            const SCEV *NewSCEV,
                                            bool *IsTruncOrSExt, bool *IsZExt,
                                            bool *IsNegation,
                                            SCEVConstant **ConstMultiplier,
                                            SCEV **Additive) const {

  bool TruncOrSExt = false;
  bool ZExt = false;

  // We got an exact match.
  if (NewSCEV == OrigSCEV) {
    return true;
  }

  auto OrigAddRec = dyn_cast<SCEVAddRecExpr>(OrigSCEV);
  auto NewAddRec = dyn_cast<SCEVAddRecExpr>(NewSCEV);

  if (!OrigAddRec || !NewAddRec) {
    if (isReplacableUsingConstantAdditive(OrigSCEV, NewSCEV, HIRP->SE,
                                          Additive)) {
      return true;
    }

    NewSCEV = HIRP->SE.getNegativeSCEV(NewSCEV);

    if (isReplacableUsingConstantAdditive(OrigSCEV, NewSCEV, HIRP->SE,
                                          Additive)) {
      *IsNegation = true;
      return true;
    }

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

  // Wrap flags may get modified during truncation/negation so we store the
  // orignal ones and pass them for comparison.
  auto WrapFlags = NewAddRec->getNoWrapFlags();

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

      auto ExtAddRec = dyn_cast<SCEVAddRecExpr>(
          HIRP->SE.getSignExtendExpr(NewAddRec, OrigType));

      if (!ExtAddRec) {
        ExtAddRec = dyn_cast<SCEVAddRecExpr>(
            HIRP->SE.getZeroExtendExpr(NewAddRec, OrigType));
        ZExt = true;
      } else {
        TruncOrSExt = true;
      }

      if (ExtAddRec) {
        NewAddRec = ExtAddRec;
      } else {
        return false;
      }

    } else {

      NewAddRec = dyn_cast<SCEVAddRecExpr>(
          HIRP->SE.getTruncateExpr(NewAddRec, OrigType));

      // In some case truncation of an AddRec returns a non-AddRec SCEV. For
      // example-
      // trunc i32 {0,+,2^30} to i16 -> 0
      // As the truncated stride evaluates to 0.
      if (!NewAddRec) {
        return false;
      }

      TruncOrSExt = true;
    }
  }

  if (isReplacableAddRec(OrigAddRec, NewAddRec, WrapFlags, ConstMultiplier,
                         Additive)) {
    *IsTruncOrSExt = TruncOrSExt;
    *IsZExt = ZExt;
    return true;
  }

  // If the IV of an outer loop is used as the initial value of the inner loop
  // it is negated during the backedge calculation for the inner loop.
  // Negation is also used during min(a, b) construction. This pattern is found
  // with a cmp and select instruction. min(a, b) is constructed as: (-1 + -1 *
  // max(-a-1, -b-1)).
  // Therefore, to find a substitute we need to test the negation too.
  NewAddRec = cast<SCEVAddRecExpr>(HIRP->SE.getNegativeSCEV(NewAddRec));

  if (isReplacableAddRec(OrigAddRec, NewAddRec, WrapFlags, ConstMultiplier,
                         Additive)) {
    *IsTruncOrSExt = TruncOrSExt;
    *IsZExt = ZExt;
    *IsNegation = true;
    return true;
  }

  return false;
}

const SCEVConstant *
HIRParser::BlobProcessor::getSDiv(const SCEVConstant *LHS,
                                  const SCEVConstant *RHS) const {
  return cast<SCEVConstant>(HIRP->SE.getConstant(cast<ConstantInt>(
      ConstantExpr::getSDiv(LHS->getValue(), RHS->getValue()))));
}

const SCEVConstant *HIRParser::BlobProcessor::getPossibleMultiplier(
    const SCEVAddRecExpr *AddRec, const SCEVAddRecExpr *MulAddRec) const {
  unsigned NumOperands = AddRec->getNumOperands();
  assert((NumOperands == MulAddRec->getNumOperands()) && "Operand mismatch!");

  // There may be cases where we still don't catch the multiplier.
  // TODO: extend it when a test case is available.
  const SCEVConstant *Mul = nullptr;

  // We use the last operand of the recurrences to find the multiplier as this
  // operand represents a non-zero stride for the recurrence.
  auto LastOp = AddRec->getOperand(NumOperands - 1);
  auto LastMulOp = MulAddRec->getOperand(NumOperands - 1);

  auto ConstStride = dyn_cast<SCEVConstant>(LastOp);
  auto MulConstStride = dyn_cast<SCEVConstant>(LastMulOp);

  // Looking for this condition-
  // AddRec: {0,+,1}
  // MulAddRec: {0,+,2}
  if (ConstStride && MulConstStride) {
    assert(!ConstStride->getValue()->isZero() &&
           "Stride of add recurrence is zero!");
    assert(!MulConstStride->getValue()->isZero() &&
           "Stride of add recurrence is zero!");

    Mul = getSDiv(MulConstStride, ConstStride);

    if (Mul->isZero()) {
      return nullptr;
    }

    return Mul;

  } else if (ConstStride || MulConstStride) {
    // No match if only one of them is a constant.
    return nullptr;
  }

  // Looking for this condition-
  // AddRec: {0,+,%size_x} or {0,+,(2 * %size_x)}
  // MulAddRec: {0,+,(4 * %size_x)}
  auto MulStrideOp = dyn_cast<SCEVMulExpr>(LastMulOp);

  if (!MulStrideOp || !isa<SCEVConstant>(MulStrideOp->getOperand(0))) {
    return nullptr;
  }

  MulConstStride = cast<SCEVConstant>(MulStrideOp->getOperand(0));

  auto StrideOp = dyn_cast<SCEVMulExpr>(LastOp);
  if (!StrideOp || !isa<SCEVConstant>(StrideOp->getOperand(0))) {
    Mul = MulConstStride;

  } else {
    ConstStride = cast<SCEVConstant>(StrideOp->getOperand(0));
    Mul = getSDiv(MulConstStride, ConstStride);

    if (Mul->isZero()) {
      return nullptr;
    }
  }

  return Mul;
}

bool HIRParser::BlobProcessor::isReplacableAddRec(
    const SCEVAddRecExpr *OrigAddRec, const SCEVAddRecExpr *NewAddRec,
    SCEV::NoWrapFlags WrapFlags, SCEVConstant **ConstMultiplier,
    SCEV **Additive) const {

  assert(ConstMultiplier && "Invalid argument!");
  assert(Additive && "Invalid argument!");

  const SCEVConstant *Mul = nullptr;
  const SCEV *Add = nullptr;
  unsigned NumOperands = OrigAddRec->getNumOperands();

  // Get constant multiplier, if any.
  if (NewAddRec->getOperand(NumOperands - 1) !=
      OrigAddRec->getOperand(NumOperands - 1)) {

    Mul = getPossibleMultiplier(NewAddRec, OrigAddRec);

    if (!Mul) {
      return false;
    }

    NewAddRec = cast<SCEVAddRecExpr>(HIRP->SE.getMulExpr(NewAddRec, Mul));
  }

  // Get invariant additive, if any.
  if (NewAddRec->getOperand(0) != OrigAddRec->getOperand(0)) {

    Add = HIRP->SE.getMinusSCEV(OrigAddRec->getOperand(0),
                                NewAddRec->getOperand(0));
  }

  // Now match operands
  for (unsigned I = 1; I < NumOperands; ++I) {
    if (NewAddRec->getOperand(I) != OrigAddRec->getOperand(I)) {
      return false;
    }
  }

  // Now we look for identical or stricter wrap flags on NewAddRec.
  // Disabling this check because ScalarEvolution is very conservative in
  // propagating wrap flags.

  // If OrigAddRec has NUW, NewAddRec should have it too.
  // if (OrigAddRec->getNoWrapFlags(SCEV::FlagNUW) &&
  //    !(WrapFlags & SCEV::FlagNUW)) {
  //  return false;
  //}
  //
  // If OrigAddRec has NSW, NewAddRec should have it too.
  // if (OrigAddRec->getNoWrapFlags(SCEV::FlagNSW) &&
  //    !(WrapFlags & SCEV::FlagNSW)) {
  //  return false;
  //}
  //
  // If OrigAddRec has NW, NewAddRec can cover it with any of NUW, NSW or NW.
  // if (OrigAddRec->getNoWrapFlags(SCEV::FlagNW) &&
  //    !(WrapFlags &
  //      (SCEV::NoWrapFlags)(SCEV::FlagNUW | SCEV::FlagNSW | SCEV::FlagNW))) {
  //  return false;
  //}

  if (Mul) {
    *ConstMultiplier = const_cast<SCEVConstant *>(Mul);
  }

  if (Add) {
    *Additive = const_cast<SCEV *>(Add);
  }

  return true;
}

void HIRParser::printScalar(raw_ostream &OS, unsigned Symbase) const {
  if (Symbase < ScalarSA.getMaxScalarSymbase()) {
    ScalarSA.getBaseScalar(Symbase)->printAsOperand(OS, false);
    return;
  }

  unsigned Index = findTempBlobIndex(Symbase);
  auto Blob = getBlob(Index);

  assert(isa<SCEVUnknown>(Blob) && "Unexpected blob type!");

  cast<SCEVUnknown>(Blob)->getValue()->printAsOperand(OS, false);
}

void HIRParser::printBlob(raw_ostream &OS, BlobTy Blob) const {

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
    } else if (isa<SCEVSMinExpr>(NArySCEV)) {
      OS << "smin(";
      OpStr = ", ";
    } else if (isa<SCEVUMinExpr>(NArySCEV)) {
      OS << "umin(";
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
  auto Symbase = ScalarSA.getScalarSymbase(Inst, CurRegion->getIRRegion());

  if (Symbase && CurRegion->isLiveOut(Symbase)) {
    return true;
  }

  return false;
}

bool HIRParser::isEssential(const Instruction *Inst) const {
  bool Ret = false;

  // TODO: Add exception handling and other miscellaneous instruction types
  // later.
  if (isa<LoadInst>(Inst) || isa<StoreInst>(Inst) ||
      (isa<CallInst>(Inst) && !isa<SubscriptInst>(Inst))) {
    Ret = true;
  } else if (isRegionLiveOut(Inst)) {
    Ret = true;
  } else if (!SE.isSCEVable(Inst->getType())) {
    Ret = true;
  }

  return Ret;
}

const SCEV *HIRParser::getSCEV(Value *Val) const {
  return SE.getSCEVForHIR(Val, CurOutermostLoop);
}

int64_t HIRParser::getSCEVConstantValue(const SCEVConstant *ConstSCEV) const {
  return ConstSCEV->getValue()->getSExtValue();
}

void HIRParser::parseConstOrDenom(const SCEVConstant *ConstSCEV, CanonExpr *CE,
                                  bool IsDenom) {
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
    if (CanonExprUtils::hasNonLinearSemantics(DefLevel, NestingLevel)) {
      // Make non-linear instead.
      CE->setNonLinear();
    } else if (DefLevel > CE->getDefinedAtLevel()) {
      CE->setDefinedAtLevel(DefLevel);
    }
  }
}

void HIRParser::cacheTempBlobLevel(unsigned Index, unsigned NestingLevel,
                                   unsigned DefLevel) {
  // -1 indicates non-linear blob
  unsigned Level =
      getCanonExprUtils().hasNonLinearSemantics(DefLevel, NestingLevel)
          ? NonLinearLevel
          : DefLevel;
  CurTempBlobLevelMap.insert(std::make_pair(Index, Level));
}

unsigned HIRParser::findOrInsertBlobWrapper(BlobTy Blob) {
  unsigned Symbase = InvalidSymbase;

#if !defined(NDEBUG)
  // Correct symbase is only required in debug mode to pass asserts.
  if (isTempBlob(Blob)) {
    auto Temp = cast<SCEVUnknown>(Blob)->getValue();
    Symbase = ScalarSA.getScalarSymbase(Temp, CurRegion->getIRRegion());

    assert((Symbase != InvalidSymbase) &&
           "Temp blob hasn't been assigned a symbase!");
  }
#endif

  return findOrInsertBlob(Blob, Symbase);
}

unsigned HIRParser::getOrAssignSymbase(const Value *Temp, unsigned *BlobIndex) {
  const Value *OldTemp = nullptr;

  auto Symbase = ScalarSA.getOrAssignScalarSymbase(
      Temp, CurRegion->getIRRegion(), &OldTemp);

  if (!OldTemp) {
    return Symbase;
  }

  auto OldTempBlob = SE.getUnknown(const_cast<Value *>(OldTemp));
  auto BaseTemp = ScalarSA.getBaseScalar(Symbase);
  auto NewTempBlob = SE.getUnknown(const_cast<Value *>(BaseTemp));
  auto Index = updateTempBlob(OldTempBlob, NewTempBlob, Symbase);

  if (BlobIndex) {
    *BlobIndex = Index;
  }

  return Symbase;
}

unsigned HIRParser::processInstBlob(const Instruction *Inst,
                                    const Instruction *BaseInst,
                                    unsigned Symbase) {
  unsigned DefLevel = 0;
  bool IsRegionLivein = false;
  HLLoop *LCALoop = nullptr;

  auto ParentBB = Inst->getParent();
  Loop *DefLp = LI.getLoopFor(Inst->getParent());

  HLLoop *DefLoop = DefLp ? LF.findHLLoop(DefLp) : nullptr;

  HLLoop *UseLoop = isa<HLLoop>(CurNode) ? cast<HLLoop>(CurNode)
                                         : CurNode->getLexicalParentLoop();

  // Set region livein and def level.
  if (!CurRegion->containsBBlock(ParentBB)) {
    CurRegion->addLiveInTemp(Symbase, Inst);
    IsRegionLivein = true;
    assert(!DefLoop && "Livein value cannot come from another region!");

  } else if ((LCALoop =
                  HLNodeUtils::getLowestCommonAncestorLoop(DefLoop, UseLoop))) {
    // If the current node where the blob is used and the blob definition are
    // both in some HLLoop, the defined at level should be the lowest common
    // ancestor loop. For example-
    //
    // DO i1
    //   DO i2
    //     t1 = ...
    //   END DO
    //
    //   DO i2
    //     A[i2] = t1; // t1 is defined at level 1 for this loop.
    //   END DO
    // END DO
    //
    DefLevel = LCALoop->getNestingLevel();
  }

  // Set loop livein/liveout as applicable.

  // For loop livein/liveout analysis we need to set defining loop based on
  // BaseInst as it represents Inst in HIR.
  if (!IsRegionLivein && (Inst != BaseInst)) {
    DefLp = LI.getLoopFor(BaseInst->getParent());
    DefLoop = DefLp ? LF.findHLLoop(DefLp) : nullptr;

    LCALoop = HLNodeUtils::getLowestCommonAncestorLoop(UseLoop, DefLoop);
  }

  // This if-else case handles liveins/liveouts caused by SSA deconstruction.
  if (DefLoop && isa<PHINode>(BaseInst) &&
      (BaseInst->getParent() == DefLp->getHeader())) {
    // If this is a phi in the loop header, it should be added as a livein
    // temp in defining loop since header phis are deconstructed as follows-
    // Before deconstruction-
    //
    // L:
    //   %t1 = phi [ %init, %step]
    //   ... = %t1
    // goto L:
    //
    //
    // After deconstruction-
    //
    // %t1 = %init
    // L:
    //   ... = %t1
    //   %t1 = %t1 + %step
    // goto L:
    //
    DefLoop->addLiveInTemp(Symbase);

  } else if (SE.getHIRMetadata(BaseInst,
                               ScalarEvolution::HIRLiveKind::LiveIn)) {
    ScalarSA.handleLoopExitLiveoutPhi(dyn_cast<PHINode>(BaseInst), Symbase);
  }

  // Loop livein/liveouts are processed per use (except for the special cases
  // handled above) so we skip the definitions (scalar lvals).
  if (!parsingScalarLval()) {
    // Add temp as livein into UseLoop and all its parent loops till we reach
    // LCA loop.
    while (UseLoop != LCALoop) {
      UseLoop->addLiveInTemp(Symbase);
      UseLoop = UseLoop->getParentLoop();
    }

    // Add temp as livein into DefLoop and all its parent loops till we reach
    // LCA loop.
    while (DefLoop != LCALoop) {
      DefLoop->addLiveOutTemp(Symbase);
      DefLoop = DefLoop->getParentLoop();
    }
  }

  return DefLevel;
}

const SCEVUnknown *HIRParser::processTempBlob(const SCEVUnknown *TempBlob,
                                              CanonExpr *CE,
                                              unsigned NestingLevel) {
  unsigned DefLevel = 0;
  unsigned Index = InvalidBlobIndex;

  auto Temp = TempBlob->getValue();

  // Get symbase and base temp.
  auto Symbase = getOrAssignSymbase(Temp, &Index);
  auto BaseTemp = ScalarSA.getBaseScalar(Symbase);

  auto BaseTempBlob =
      (Temp == BaseTemp)
          ? TempBlob
          : cast<SCEVUnknown>(SE.getUnknown(const_cast<Value *>(BaseTemp)));

  // Insert blob, if Index was not provided by getOrAssignSymbase().
  if (Index == InvalidBlobIndex) {
    Index = findOrInsertBlob(BaseTempBlob, Symbase);
  }

  if (auto Inst = dyn_cast<Instruction>(Temp)) {
    DefLevel = processInstBlob(Inst, cast<Instruction>(BaseTemp), Symbase);
  } else {
    // Mark non-instruction blobs as livein to region and parent loops.
    CurRegion->addLiveInTemp(Symbase, Temp);

    HLLoop *UseLoop = isa<HLLoop>(CurNode) ? cast<HLLoop>(CurNode)
                                           : CurNode->getLexicalParentLoop();

    while (UseLoop) {
      UseLoop->addLiveInTemp(Symbase);
      UseLoop = UseLoop->getParentLoop();
    }
  }

  setCanonExprDefLevel(CE, NestingLevel, DefLevel);

  // RegDDRef.
  cacheTempBlobLevel(Index, NestingLevel, DefLevel);

  // Return base temp.
  return BaseTempBlob;
}

bool HIRParser::breakConstantMultiplierMulBlob(const SCEVMulExpr *MulBlob,
                                               int64_t *Multiplier,
                                               BlobTy *NewBlob) {
  // If there is a constant, it will be the first operand due to operand
  // ordering.
  if (auto *ConstOp = dyn_cast<SCEVConstant>(MulBlob->getOperand(0))) {
    SmallVector<BlobTy, 4> Ops;

    for (auto I = MulBlob->op_begin() + 1, E = MulBlob->op_end(); I != E; ++I) {
      Ops.push_back(*I);
    }

    *Multiplier = getSCEVConstantValue(ConstOp);
    *NewBlob = SE.getMulExpr(Ops, MulBlob->getNoWrapFlags());
    return true;
  }

  // We can extract the multiplier from individual operands and multiply them.
  // For example, 2 can be extracted from the 2nd operand of mul below-
  // (zext(%t1) * (2 * %t2 + 2* %t3))

  int64_t LocalMultiplier = 1;
  SmallVector<BlobTy, 4> Ops;
  bool FoundMultiplier = false;

  for (auto I = MulBlob->op_begin(), E = MulBlob->op_end(); I != E; ++I) {
    BlobTy Op = *I;

    int64_t OpMultiplier;
    BlobTy NewOp;

    if (!breakConstantMultiplierCommutativeBlob(Op, &OpMultiplier, &NewOp)) {
      Ops.push_back(Op);
      continue;
    }

    APInt LocalMul(64, LocalMultiplier, true);
    APInt OpMul(64, OpMultiplier, true);

    bool Overflow;
    LocalMul = LocalMul.smul_ov(OpMul, Overflow);

    if (Overflow) {
      Ops.push_back(Op);
      continue;
    }

    FoundMultiplier = true;
    LocalMultiplier = LocalMul.getSExtValue();
    Ops.push_back(NewOp);
  }

  if (!FoundMultiplier) {
    return false;
  }

  *Multiplier = LocalMultiplier;
  *NewBlob = SE.getMulExpr(Ops, MulBlob->getNoWrapFlags());
  return true;
}

bool HIRParser::breakConstantMultiplierCommutativeBlob(BlobTy Blob,
                                                       int64_t *Multiplier,
                                                       BlobTy *NewBlob,
                                                       bool IsTop) {

  if (auto *Const = dyn_cast<SCEVConstant>(Blob)) {
    if (!IsTop) {
      *Multiplier = getSCEVConstantValue(Const);
      *NewBlob = SE.getConstant(Const->getType(), 1, true);
      return true;
    }

    return false;
  }

  if (auto *MulBlob = dyn_cast<SCEVMulExpr>(Blob)) {
    return breakConstantMultiplierMulBlob(MulBlob, Multiplier, NewBlob);
  }

  auto *CommutativeBlob = dyn_cast<SCEVCommutativeExpr>(Blob);

  if (!CommutativeBlob) {
    return false;
  }

  // The multiplier may be folded into the constant so we have to recurse into
  // the blob. Ex: (-8 + 8 * %t)

  int64_t LocalMultiplier = 0;
  SmallVector<BlobTy, 4> Ops;
  SmallVector<int64_t, 4> OpMultipliers;

  for (auto I = CommutativeBlob->op_begin(), E = CommutativeBlob->op_end();
       I != E; ++I) {

    BlobTy Op = *I;

    BlobTy NewOp;
    int64_t OpMultiplier;

    if (!breakConstantMultiplierCommutativeBlob(Op, &OpMultiplier, &NewOp) ||
        (OpMultiplier == LONG_MIN)) {
      return false;
    }

    if (!LocalMultiplier) {
      LocalMultiplier = std::llabs(OpMultiplier);
    } else {
      LocalMultiplier =
          CanonExprUtils::gcd(LocalMultiplier, std::llabs(OpMultiplier));
    }

    if (LocalMultiplier == 1) {
      return false;
    }

    Ops.push_back(NewOp);
    OpMultipliers.push_back(OpMultiplier);
  }

  // If the overall multiplier is less than the original multiplier of the
  // operand, push it back into the operand.
  for (unsigned I = 0, Num = Ops.size(); I < Num; ++I) {

    if (LocalMultiplier != OpMultipliers[I]) {
      auto *UnusedMultiplier = SE.getConstant(
          Ops[I]->getType(), OpMultipliers[I] / LocalMultiplier, true);

      auto *CommutativeOp = dyn_cast<SCEVCommutativeExpr>(Ops[I]);

      Ops[I] = SE.getMulExpr(Ops[I], UnusedMultiplier,
                             CommutativeOp ? CommutativeOp->getNoWrapFlags()
                                           : SCEV::FlagAnyWrap);
    }
  }

  *Multiplier = LocalMultiplier;

  switch (CommutativeBlob->getSCEVType()) {
  case scAddExpr:
    *NewBlob = SE.getAddExpr(Ops, CommutativeBlob->getNoWrapFlags());
    break;
  case scUMaxExpr:
    *NewBlob = SE.getUMaxExpr(Ops);
    break;
  case scSMaxExpr:
    *NewBlob = SE.getSMaxExpr(Ops);
    break;
  case scUMinExpr:
    *NewBlob = SE.getUMinExpr(Ops);
    break;
  case scSMinExpr:
    *NewBlob = SE.getSMinExpr(Ops);
    break;
  default:
    llvm_unreachable("Unexpected scev type!");
  }

  return true;
}

void HIRParser::breakConstantMultiplierBlob(BlobTy Blob, int64_t *Multiplier,
                                            BlobTy *NewBlob) {

  if (breakConstantMultiplierCommutativeBlob(Blob, Multiplier, NewBlob, true)) {
    return;
  }

  *Multiplier = 1;
  *NewBlob = Blob;
  return;
}

bool HIRParser::parseBlob(BlobTy Blob, CanonExpr *CE, unsigned Level,
                          unsigned IVLevel, bool IndicateFailure) {
  // Process and create base version of the blob.
  int64_t Multiplier;

  // We need two different BlobProcessor objects, one for safe mode processing
  // and the other for regular mode processing because the base class of
  // BlobProcessor (SCEVRewriteVisitor) caches mapping results which differ in
  // the two modes.
  // TODO: Safe mode doesn't require CE and level and hence can have a simpler
  // constructor which is cleaner. Whether we can use a single safe mode and
  // regular mode BlobProcessor object for HIRParser needs to be investigated.
  if (IndicateFailure &&
      !BlobProcessor(this, CE, Level).canProcessSafely(Blob)) {
    return false;
  }

  auto NewBlob = BlobProcessor(this, CE, Level).process(Blob);
  breakConstantMultiplierBlob(NewBlob, &Multiplier, &NewBlob);

  unsigned Index = findOrInsertBlobWrapper(NewBlob);

  if (IVLevel) {
    CE->addIV(IVLevel, Index, Multiplier);
  } else {
    CE->addBlob(Index, Multiplier);
  }

  return true;
}

// Validates SCEV returned by getSCEVAtScope(). If the returned SCEV contains a
// phi which is defined in the current loop, we invalidate the SCEV as it can
// potentially cause live range issues.
//
// NOTE: This check does not belong to parser. It is the job of SSA
// deconstruction to only let valid incoming SCEVs to parser but the solutions
// I can think of are either too conservative or too compile time expensive.
//
// TODO: Find a way to handle this in SSA deconstruction.
class HIRParser::ScopeSCEVValidator {
private:
  const HIRParser &HIRP;
  bool IsValid;
  const Instruction *CurInst;
  const Loop *CurLp;

public:
  ScopeSCEVValidator(const HIRParser &HIRP) : HIRP(HIRP), IsValid(true) {
    CurInst = HIRP.getCurInst();
    CurLp = HIRP.LI.getLoopFor(CurInst->getParent());
  }

  bool follow(const SCEV *SC) {

    if (!HIRP.isTempBlob(SC)) {
      return true;
    }

    auto Val = cast<SCEVUnknown>(SC)->getValue();
    auto Phi = dyn_cast<PHINode>(Val);

    if (!Phi) {
      return true;
    }

    if (!CurLp->contains(Phi)) {
      return true;
    }

    IsValid = false;
    return IsValid;
  }

  bool isDone() const { return !isValid(); }

  bool isValid() const { return IsValid; }
};

bool HIRParser::isValidScopeSCEV(const SCEV *SC) const {
  ScopeSCEVValidator SSV(*this);
  SCEVTraversal<ScopeSCEVValidator> Validator(SSV);
  Validator.visitAll(SC);

  return SSV.isValid();
}

const SCEV *HIRParser::getSCEVAtScope(const SCEV *SC) const {
  auto ParHLoop = CurNode->getLexicalParentLoop();
  const Loop *ParLoop = ParHLoop ? ParHLoop->getLLVMLoop() : nullptr;

  auto NewSC = SE.getSCEVAtScopeForHIR(SC, ParLoop, CurOutermostLoop);

  return isValidScopeSCEV(NewSC) ? NewSC : SC;
}

bool HIRParser::parseAddRec(const SCEVAddRecExpr *RecSCEV, CanonExpr *CE,
                            unsigned Level, bool IndicateFailure) {
  auto Lp = RecSCEV->getLoop();
  auto HLoop = LF.findHLLoop(Lp);

  assert(HLoop && "Could not find HIR loop!");

  auto BaseSCEV = RecSCEV->getOperand(0);
  auto BaseAddRec = dyn_cast<SCEVAddRecExpr>(BaseSCEV);
  auto StepSCEV = RecSCEV->getOperand(1);
  auto StepAddRec = dyn_cast<SCEVAddRecExpr>(StepSCEV);

  // Sometimes when you multiply affine AddRecs, the base of the resulting
  // AddRec can become non-affine which would not correspond to any value in
  // the IR. In this case we need a lookahead.
  //
  // Example 1:
  // V1 = i1, V2 = (i1 + i2), V1 * V2 = i1*i1 + i1*i2
  // Here (i1 * i1) becomes non-affine base but it cannot be reverse
  // engineered as there is no value corresponding to this SCEV.
  //
  // Example 2:
  // V1 = ((i1 + 1)*i2), V2 = i1, V1 * V2 = ((i1*i1 + i1) * i2)
  // Here (i1 * i1) becomes the non-affine step but it cannot be reverse
  // engineered as there is no value corresponding to this SCEV.
  // Note that i1 * i2 is still an affine AddRec even though it is non-linear.
  // This is because it is represented in SCEV form as follows:
  // {0, +, {0,+,1}<i1> }<i2>
  if (!RecSCEV->isAffine() || (BaseAddRec && !BaseAddRec->isAffine()) ||
      (StepAddRec && !StepAddRec->isAffine())) {

    return parseBlob(RecSCEV, CE, Level, 0, IndicateFailure);

  } else if (!getHLNodeUtils().contains(HLoop, CurNode)) {
    // If the use is outside the loop, use the 'at scope'(exit value)
    // information.

    auto NewSC = getSCEVAtScope(RecSCEV);
    auto NewAddRec = dyn_cast<SCEVAddRecExpr>(NewSC);

    // If getSCEVAtScope() returned a valid SCEV...
    if (!NewAddRec || (NewAddRec->getLoop() != Lp)) {
      // Parsing is more likely to fail with 'at scope' information. So we
      // create a new CE and invoke parsing in failure indication mode. If it
      // does fail, we fall back to parsing original SCEV as blob.
      std::unique_ptr<CanonExpr> NewCE(getCanonExprUtils().createExtCanonExpr(
          CE->getSrcType(), CE->getDestType(), CE->isSExt()));

      if (parseRecursive(NewSC, NewCE.get(), Level, false, true, true)) {
        getCanonExprUtils().add(CE, NewCE.get());
      } else {
        return parseBlob(RecSCEV, CE, Level, 0, IndicateFailure);
      }

    } else {
      return parseBlob(RecSCEV, CE, Level, 0, IndicateFailure);
    }

  } else {
    // Convert AddRec into CanonExpr IV.

    if (!parseRecursive(BaseSCEV, CE, Level, false, true, IndicateFailure)) {
      return false;
    }

    // Set constant IV coeff.
    if (isa<SCEVConstant>(StepSCEV)) {
      auto Coeff = getSCEVConstantValue(cast<SCEVConstant>(StepSCEV));
      CE->addIV(HLoop->getNestingLevel(), 0, Coeff);
    }
    // Set blob IV coeff.
    else {
      return parseBlob(StepSCEV, CE, Level, HLoop->getNestingLevel(),
                       IndicateFailure);
    }
  }

  return true;
}

bool HIRParser::parseMul(const SCEVMulExpr *MulSCEV, CanonExpr *CE,
                         unsigned Level, bool IndicateFailure) {

  // If mul looks like this:
  // {0,+,1} * %a
  //
  // Then it can be parsed into a CanonExpr IV term like this:
  // %a * i1.
  //
  // We create new auxiliary CEs to parse the IV and blob term. These two CEs
  // are then multiplied and added to the original CE.

  // The last CanonExpr::add() will not do the right thing in the presence of
  // denominator so we skip the logic.
  if ((CE->getDenominator() != 1) || (MulSCEV->getNumOperands() != 2)) {
    return parseBlob(MulSCEV, CE, Level, 0, IndicateFailure);
  }

  auto AddRecSCEV = dyn_cast<SCEVAddRecExpr>(MulSCEV->getOperand(0));

  if (!AddRecSCEV) {
    return parseBlob(MulSCEV, CE, Level, 0, IndicateFailure);
  }

  std::unique_ptr<CanonExpr> AddRecCE(
      getCanonExprUtils().createCanonExpr(CE->getSrcType()));

  if (!parseAddRec(AddRecSCEV, AddRecCE.get(), Level, IndicateFailure)) {
    return parseBlob(MulSCEV, CE, Level, 0, IndicateFailure);
  }

  std::unique_ptr<CanonExpr> BlobCE(
      getCanonExprUtils().createCanonExpr(CE->getSrcType()));

  if (!parseBlob(MulSCEV->getOperand(1), BlobCE.get(), Level, 0,
                 IndicateFailure)) {
    return parseBlob(MulSCEV, CE, Level, 0, IndicateFailure);
  }

  if (!AddRecCE->multiplyByConstant(BlobCE->getSingleBlobCoeff()) ||
      !AddRecCE->multiplyByBlob(BlobCE->getSingleBlobIndex())) {
    return parseBlob(MulSCEV, CE, Level, 0, IndicateFailure);
  }

  if (!CanonExprUtils::add(CE, AddRecCE.get())) {
    return parseBlob(MulSCEV, CE, Level, 0, IndicateFailure);
  }

  // Update def level of CE using blob's def level.
  setCanonExprDefLevel(CE, Level, BlobCE->getDefinedAtLevel());

  return true;
}

bool HIRParser::parseRecursive(const SCEV *SC, CanonExpr *CE, unsigned Level,
                               bool IsTop, bool UnderCast,
                               bool IndicateFailure) {

  if (auto ConstSCEV = dyn_cast<SCEVConstant>(SC)) {
    parseConstant(ConstSCEV, CE);
    return true;

  } else if (isa<SCEVUnknown>(SC)) {
    parseBlob(SC, CE, Level);
    return true;

  } else if (auto CastSCEV = dyn_cast<SCEVCastExpr>(SC)) {

    bool IsTrunc = isa<SCEVTruncateExpr>(CastSCEV);
    auto *OpTy = CastSCEV->getOperand()->getType();
    // If trunc's operand type does not fit in 64 bits, we want to parse it as a
    // blob as CanonExpr cannot hold constants which are outside this range.
    if (IsTop && !UnderCast &&
        (!IsTrunc || OpTy->getPrimitiveSizeInBits() <= 64)) {
      CE->setSrcType(OpTy);
      CE->setExtType(isa<SCEVSignExtendExpr>(CastSCEV));
      return parseRecursive(CastSCEV->getOperand(), CE, Level, true, true,
                            IndicateFailure);
    } else {
      return parseBlob(CastSCEV, CE, Level, 0, IndicateFailure);
    }

  } else if (auto AddSCEV = dyn_cast<SCEVAddExpr>(SC)) {

    if (isMinMaxWithAddRecOperand(AddSCEV)) {
      return parseBlob(AddSCEV, CE, Level, 0, IndicateFailure);

    } else {
      for (auto I = AddSCEV->op_begin(), E = AddSCEV->op_end(); I != E; ++I) {
        if (!parseRecursive(*I, CE, Level, false, UnderCast, IndicateFailure)) {
          return false;
        }
      }
      return true;
    }

  } else if (auto MulSCEV = dyn_cast<SCEVMulExpr>(SC)) {

    return parseMul(MulSCEV, CE, Level, IndicateFailure);

  } else if (auto UDivSCEV = dyn_cast<SCEVUDivExpr>(SC)) {
    if (!IsTop) {
      return parseBlob(SC, CE, Level, 0, IndicateFailure);
    }

    auto ConstDenomSCEV = dyn_cast<SCEVConstant>(UDivSCEV->getRHS());

    // If the denominator is constant and is not minimum 64 bit signed value,
    // move it into CE's denominator. Negative denominators are negated and
    // stored as positive integers but we cannot negate INT_MIN so we make it
    // a blob.
    if (ConstDenomSCEV && ((ConstDenomSCEV->getValue()->getBitWidth() < 64) ||
                           !ConstDenomSCEV->getValue()->isMinValue(true))) {
      parseDenominator(ConstDenomSCEV, CE);
      return parseRecursive(UDivSCEV->getLHS(), CE, Level, false, UnderCast,
                            IndicateFailure);
    } else {
      return parseBlob(SC, CE, Level, 0, IndicateFailure);
    }

  } else if (auto RecSCEV = dyn_cast<SCEVAddRecExpr>(SC)) {
    return parseAddRec(RecSCEV, CE, Level, IndicateFailure);

  } else if (isa<SCEVMinMaxExpr>(SC)) {
    // TODO: extend DDRef representation to handle min/max.
    return parseBlob(SC, CE, Level, 0, IndicateFailure);
  }

  llvm_unreachable("Unexpected SCEV type!");
}

CanonExpr *HIRParser::parseAsBlob(const Value *Val, unsigned Level,
                                  IntegerType *FinalIntTy) {

  auto *ValTy = Val->getType();
  bool RequiresCasting = FinalIntTy && (FinalIntTy != ValTy);

  CanonExpr *CE =
      getCanonExprUtils().createCanonExpr(RequiresCasting ? FinalIntTy : ValTy);
  auto *BlobSCEV = SE.getUnknown(const_cast<Value *>(Val));

  if (RequiresCasting) {
    BlobSCEV = SE.getTruncateOrSignExtend(BlobSCEV, FinalIntTy);
  }

  parseBlob(BlobSCEV, CE, Level);

  return CE;
}

// TODO: use SCEVExprContains() instead, when available.
class CastedAddRecChecker {
  Type *CastSrcTy;
  bool Found;

public:
  CastedAddRecChecker(Type *CastSrcTy) : CastSrcTy(CastSrcTy), Found(false) {}

  bool follow(const SCEV *SC) {

    auto CastSC = dyn_cast<SCEVCastExpr>(SC);

    if (!CastSC) {
      return true;
    }

    auto Op = CastSC->getOperand();

    if (!isa<SCEVAddRecExpr>(Op)) {
      return true;
    }

    if (Op->getType() != CastSrcTy) {
      return true;
    }

    Found = true;
    return false;
  }

  bool found() const { return Found; }

  bool isDone() const { return found(); }
};

bool HIRParser::containsCastedAddRec(const CastInst *CI, const SCEV *SC) const {
  // If the SCEV of this cast instruction contains an explicit cast for an
  // AddRec (outer loop IV), it is better to parse the cast explicitly otherwise
  // the outer loop IV will be parsed as a blob. Consider this cast-
  // %idxprom = sext i32 %t to i64
  // The SCEV for %idxprom looks like this-
  // {cast i32 to i64 {0,+,1}<%for.outer>, +, 1}<nuw><nsw><%for.inner> (i64
  // type)
  // The SCEV for %t looks like this-
  // {{0,+,1}<%for.outer>, +, 1}<nuw><nsw><%for.inner> (i32 type)
  //
  // If we strip the cast explitly, it will be parsed as: sext.i32.i64(i1 + i2).
  // Otherwise it will be parsed as: i2 + sext.i32.i64(%b), where %b represents
  // i1 (outer loop IV).

  CastedAddRecChecker CARC(CI->getSrcTy());
  SCEVTraversal<CastedAddRecChecker> Checker(CARC);
  Checker.visitAll(SC);

  return CARC.found();
}

bool HIRParser::isCastedFromLoopIVType(const CastInst *CI,
                                       const SCEV *SC) const {
  // For cast instructions which cast from loop IV's type to some other
  // type, we want to explicitly hide the cast and parse the value in IV's type.
  // This allows more opportunities for canon expr merging. Consider the
  // following cast-
  // %idxprom = sext i32 %i.01 to i64
  // Here %i.01 is the loop IV whose SCEV looks like this:
  // {0,+,1}<nuw><nsw><%for.body> (i32 type)
  // The SCEV of %idxprom doesn't have a cast and it looks like this:
  // {0,+,1}<nuw><nsw><%for.body> (i64 type)
  // We instead want %idxprom to be considered as a cast: sext i32
  // {0,+,1}<nuw><nsw><%for.body> to i64

  // Ignore if SCEV form of CI is already a cast. Top cast can be handled by
  // parseRecursive().
  if (isa<SCEVCastExpr>(SC)) {
    return false;
  }

  auto ParentLoop = getCurNode()->getParentLoop();
  return (ParentLoop && (ParentLoop->getIVType() == CI->getSrcTy()));
}

bool HIRParser::shouldParseWithoutCast(const CastInst *CI, bool IsTop) const {
  if (!IsTop || !CI) {
    return false;
  }

  if (!isa<SExtInst>(CI) && !isa<ZExtInst>(CI) && !isa<TruncInst>(CI)) {
    return false;
  }

  auto SC = getSCEV(const_cast<CastInst *>(CI));

  if (isCastedFromLoopIVType(CI, SC) || containsCastedAddRec(CI, SC)) {
    return true;
  }

  return false;
}

CanonExpr *HIRParser::parse(const Value *Val, unsigned Level, bool IsTop,
                            IntegerType *FinalIntTy) {
  CanonExpr *CE = nullptr;
  const Value *OrigVal = Val;
  auto *ValTy = Val->getType();

  // Parse as blob if the type is not SCEVable.
  // This is currently for handling floating types.
  if (!SE.isSCEVable(ValTy)) {
    assert(!FinalIntTy && "Cannot convert value to integer type! ");
    CE = parseAsBlob(Val, Level);

  } else if (ValTy->isPointerTy()) {
    assert(!FinalIntTy && "Cannot convert value to integer type! ");

    if (isa<ConstantPointerNull>(Val)) {
      // Create null CE to represent a null pointer.
      CE = getCanonExprUtils().createCanonExpr(ValTy);
    } else {
      // Force pointer values to be parsed as blobs. This is for handling lvals
      // but pointer blobs can occur in loop upper as well. CG will have to do
      // special processing for pointers contained in upper.
      CE = parseAsBlob(Val, Level);
    }

  } else {

    bool EnableCastHiding = IsTop;
    const CastInst *CI = dyn_cast<CastInst>(Val);
    bool RequiresCasting = (FinalIntTy && (FinalIntTy != ValTy));

    if (!RequiresCasting && shouldParseWithoutCast(CI, IsTop)) {
      EnableCastHiding = false;
      Val = CI->getOperand(0);

      CE = getCanonExprUtils().createExtCanonExpr(
          CI->getSrcTy(), CI->getDestTy(), isa<SExtInst>(CI));

    } else {
      CE = getCanonExprUtils().createCanonExpr(RequiresCasting ? FinalIntTy
                                                               : ValTy);
    }

    auto SC = getSCEV(const_cast<Value *>(Val));

    if (RequiresCasting) {
      SC = SE.getTruncateOrSignExtend(SC, FinalIntTy);
    }

    if (parseRecursive(SC, CE, Level, IsTop, !EnableCastHiding, true)) {
      parseMetadata(OrigVal, CE);
    } else {
      getCanonExprUtils().destroy(CE);
      CE = parseAsBlob(OrigVal, Level, FinalIntTy);
    }
  }

  assert((CE->getDestType() == OrigVal->getType() ||
          (CE->getDestType() == FinalIntTy)) &&
         "CE and Val types do not match!");

  return CE;
}

void HIRParser::clearTempBlobLevelMap() { CurTempBlobLevelMap.clear(); }

void HIRParser::populateBlobDDRefs(RegDDRef *Ref, unsigned Level) {

  SmallVector<unsigned, 8> BlobIndices;

  if (CurTempBlobLevelMap.empty()) {
    return;
  }

  // Some of the parsed blobs can get cancelled due to index merging or SCEV
  // simplification so we need to check whether there is a mismatch in collected
  // blobs and actual blobs present in the DDRef.
  //
  // Here's a very simple made up example composed of multiple GEPs-
  //
  // %p = GEP @A, %1
  // %2 = sub 0, %1
  // %q = GEP %p, %2
  //
  // When parsing %q, we parse %p (@A + %1) and %2 (-1 * %1) separately and then
  // merge them. On merging %1 will cancel out.
  //
  Ref->collectTempBlobIndices(BlobIndices);

  if (BlobIndices.size() == CurTempBlobLevelMap.size()) {
    // No mismatch, populate all the blobs present in the map.
    for (auto const &I : CurTempBlobLevelMap) {
      auto Blob = getBlob(I.first);
      (void)Blob;
      assert(isa<SCEVUnknown>(Blob) && "Unexpected temp blob!");

      auto BRef = getDDRefUtils().createBlobDDRef(I.first, I.second);
      Ref->addBlobDDRef(BRef);
    }

  } else {
    // Mismatch in number of blobs, only add blobs which are actually present in
    // the DDRef.
    assert((BlobIndices.size() < CurTempBlobLevelMap.size()) &&
           "Inconsistent blob parsing!");

    for (auto &I : BlobIndices) {
      auto It = CurTempBlobLevelMap.find(I);
      assert((It != CurTempBlobLevelMap.end()) && "Blob not found!");

      auto BRef = getDDRefUtils().createBlobDDRef(It->first, It->second);
      Ref->addBlobDDRef(BRef);
    }

    // Since some of the blobs cancelled out, the def level of CEs in the Ref
    // requires updation.
    Ref->updateDefLevelInternal(Level);
  }
}

static bool isKnownNonNegativeForLoop(ScalarEvolution &SE, const Loop *Lp,
                                      const SCEV *SC) {
  if (SE.isKnownNonNegative(SC)) {
    return true;
  }

  return SE.isLoopEntryGuardedByCond(Lp, ICmpInst::ICMP_SGE, SC,
                                     SE.getZero(SC->getType()));
}

RegDDRef *HIRParser::createUpperDDRef(const SCEV *BETC, unsigned Level,
                                      Type *IVType, const Loop *Lp) {
  clearTempBlobLevelMap();

  auto CE = getCanonExprUtils().createCanonExpr(IVType);
  auto BETCType = BETC->getType();

  assert((!BETCType->isPointerTy() ||
          (getDataLayout().getTypeSizeInBits(BETCType) ==
           IVType->getPrimitiveSizeInBits())) &&
         "Loop with pointer type BETC does not have pointer sized IV!");

  // If there is a type mismatch, make upper the same type as IVType.
  if (!BETCType->isPointerTy() && (BETCType != IVType)) {

    if (IVType->getPrimitiveSizeInBits() > BETCType->getPrimitiveSizeInBits()) {
      // isKnownNonNegativeForLoop() check is not powerful enought to work for
      // triangular loops where backedge depends on outer loop IV but this
      // should not be a problem because induction variable simplification pass
      // seems to extend the loop upper along with the IV. Previously, it was
      // truncating the extended IV to compare it with the loop upper. Some of
      // the lit tests using the old setup needed to be changed. Since zperf run
      // showed no degradations I am leaving this as a TODO.
      BETC = isKnownNonNegativeForLoop(SE, Lp, BETC)
                 ? SE.getSignExtendExpr(BETC, IVType)
                 : SE.getZeroExtendExpr(BETC, IVType);
    } else {
      BETC = SE.getTruncateExpr(BETC, IVType);
    }
  }

  // We pass underCast as 'true' as we don't want to hide the topmost cast for
  // upper.
  if (!parseRecursive(BETC, CE, Level, true, true, true)) {
    getCanonExprUtils().destroy(CE);
    return nullptr;
  }

  // Symbase for self-blobs is updated after parsing below.
  unsigned Symbase =
      isa<SCEVConstant>(BETC) ? ConstantSymbase : GenericRvalSymbase;

  auto Ref = getDDRefUtils().createRegDDRef(Symbase);
  Ref->setSingleCanonExpr(CE);

  int64_t UpperVal;

  // If upper is negative, make it positive if it fits in signed 64 bits.
  if (CE->isIntConstant(&UpperVal) && (UpperVal < 0)) {
    auto SrcTy = CE->getSrcType();
    unsigned BitWidth = SrcTy->getPrimitiveSizeInBits();

    if (BitWidth < 64) {
      // In modular arithmetic with modulus N: (a == a + N).
      // Here N is 2 ^ bitwidth.
      UpperVal = UpperVal + (1ULL << BitWidth);
      CE->setConstant(UpperVal);
    }
  }

  // Update DDRef's symbase to blob's symbase for self-blob DDRefs.
  if (CE->isSelfBlob()) {
    Ref->setSymbase(getTempBlobSymbase(CE->getSingleBlobIndex()));
  } else {
    populateBlobDDRefs(Ref, Level);
  }

  populateRequiredSymbases(Ref);

  return Ref;
}

void HIRParser::parse(HLRegion *Reg) {
  CurRegion = Reg;
}

void HIRParser::parse(HLLoop *HLoop) {

  setCurNode(HLoop);

  auto Lp = HLoop->getLLVMLoop();
  assert(Lp && "HLLoop doesn't contain LLVM loop!");
  auto IVType = HLoop->getIVType();

  // Upper should be parsed after incrementing level.
  ++CurLevel;

  if (1 == CurLevel) {
    CurOutermostLoop = Lp;
  }

  auto BETC = SE.getBackedgeTakenCountForHIR(Lp, CurOutermostLoop);
  bool IsUnknown = isa<SCEVCouldNotCompute>(BETC);

  if (!IsUnknown) {
    auto UpperRef = createUpperDDRef(BETC, CurLevel, IVType, Lp);

    if (!UpperRef) {
      // Parsing for upper failed. Treat loop as unknown as a backup option.
      IsUnknown = true;

      // Add the explicit loop label and bottom test back to the loop.
      LF.reattachLoopLabelAndBottomTest(HLoop);

    } else {
      // Initialize Lower to 0.
      auto LowerRef = createLowerDDRef(IVType);
      HLoop->setLowerDDRef(LowerRef);

      // Initialize Stride to 1.
      auto StrideRef = createStrideDDRef(IVType);
      HLoop->setStrideDDRef(StrideRef);

      HLoop->setUpperDDRef(UpperRef);

      unsigned MaxTC;

      // Set small max trip count if available from scalar evolution.
      if (!UpperRef->isIntConstant() &&
          (MaxTC = SE.getSmallConstantMaxTripCount(const_cast<Loop *>(Lp)))) {
        HLoop->setMaxTripCountEstimate(MaxTC);
      }
    }
  }

  if (IsUnknown) {
    // Initialize Stride to 0 for unknown loops.
    auto ZeroRef = getDDRefUtils().createConstDDRef(IVType, 0);

    // Set lower, stride and upper to 0. The main check for unknown loops is
    // having a stride of 0. Upper and lower are set to avoid ddref traversal
    // failure for HLDDNodes (on encountering null refs).
    HLoop->setLowerDDRef(ZeroRef);
    HLoop->setStrideDDRef(ZeroRef->clone());
    HLoop->setUpperDDRef(ZeroRef->clone());
  }

  // TODO: assert that SIMD loops are always DO loops.

  // Parse ztt.
  if (HLoop->hasZtt()) {
    parse(HLoop->getZtt(), HLoop);
  }
}

void HIRParser::postParse(HLLoop *HLoop) {
  if (1 == CurLevel) {
    CurOutermostLoop = nullptr;
  }

  --CurLevel;
}

void HIRParser::parseCompare(const Value *Cond, unsigned Level,
                             SmallVectorImpl<HLPredicate> &Preds,
                             SmallVectorImpl<RegDDRef *> &Refs,
                             bool AllowMultiplePreds) {

  assert(Cond->getType()->getScalarType()->isIntegerTy(1) &&
         "Condition should be i1 or <n x i1> type!");

  if (auto *CInst = dyn_cast<CmpInst>(Cond)) {

    // Suppress traceback if CInst's operand's type is not supported.
    if (RI.isSupported(CInst->getOperand(0)->getType(), false) &&
        RI.isSupported(CInst->getOperand(1)->getType(), false)) {

      Preds.push_back(
          {CInst->getPredicate(), parseFMF(CInst), CInst->getDebugLoc()});

      Refs.push_back(createRvalDDRef(CInst, 0, Level));
      Refs.push_back(createRvalDDRef(CInst, 1, Level));
      return;
    }
  }

  const BinaryOperator *BOp = nullptr;

  if (AllowMultiplePreds && (BOp = dyn_cast<BinaryOperator>(Cond)) &&
      (BOp->getOpcode() == Instruction::And)) {
    auto Op1 = BOp->getOperand(0);
    auto Op2 = BOp->getOperand(1);

    // Do not bring in '&&' conditions from outside the region.
    if (CurRegion->containsBBlock(BOp->getParent()) &&
        RI.isSupported(Op1->getType(), false) &&
        RI.isSupported(Op2->getType(), false)) {
      parseCompare(Op1, Level, Preds, Refs, true);
      parseCompare(Op2, Level, Preds, Refs, true);
      return;
    }
  }

  if (isa<UndefValue>(Cond)) {
    Preds.push_back(UNDEFINED_PREDICATE);
    Refs.push_back(getDDRefUtils().createUndefDDRef(Cond->getType()));
    Refs.push_back(getDDRefUtils().createUndefDDRef(Cond->getType()));
    return;
  }

  if (auto *ConstVal = dyn_cast<ConstantInt>(Cond)) {
    if (ConstVal->isOneValue()) {
      Preds.push_back(PredicateTy::FCMP_TRUE);
    } else {
      assert(ConstVal->isZeroValue() && "Unexpected compare condition!");
      Preds.push_back(PredicateTy::FCMP_FALSE);
    }
    Refs.push_back(getDDRefUtils().createUndefDDRef(Cond->getType()));
    Refs.push_back(getDDRefUtils().createUndefDDRef(Cond->getType()));
    return;
  }

  auto *CompareExpr = dyn_cast<ConstantExpr>(Cond);
  if (CompareExpr && CompareExpr->isCompare()) {
    Preds.push_back(static_cast<PredicateTy>(CompareExpr->getPredicate()));
    Refs.push_back(createScalarDDRef(CompareExpr->getOperand(0), Level));
    Refs.push_back(createScalarDDRef(CompareExpr->getOperand(1), Level));
    return;
  }

  // We do not understand the condition. Fall back to parsing it as-
  // (Cond != 0)
  Preds.push_back(PredicateTy::ICMP_NE);
  Refs.push_back(createScalarDDRef(Cond, Level));
  Refs.push_back(getDDRefUtils().createConstDDRef(Cond->getType(), 0));
}

void HIRParser::parseCompare(const Value *Cond, unsigned Level,
                             HLPredicate *Pred, RegDDRef **LHSDDRef,
                             RegDDRef **RHSDDRef) {
  SmallVector<HLPredicate, 1> Preds;
  SmallVector<RegDDRef *, 2> Refs;

  parseCompare(Cond, Level, Preds, Refs, false);
  assert((Preds.size() == 1) && "Single predicate expected!");

  *Pred = Preds[0];
  *LHSDDRef = Refs[0];
  *RHSDDRef = Refs[1];
}

void HIRParser::parse(HLIf *If, HLLoop *HLoop) {
  SmallVector<HLPredicate, 4> Preds;
  SmallVector<RegDDRef *, 8> Refs;

  setCurNode(If);

  auto SrcBB = HIRC.getSrcBBlock(If);
  assert(SrcBB && "Could not find If's src basic block!");

  auto BeginPredIter = If->pred_begin();
  auto LoopTerm = cast<BranchInst>(SrcBB->getTerminator());
  auto IfCond = LoopTerm->getCondition();

  // Allow single predicate in unknown loop bottom test. This makes life easier
  // for unroller.
  parseCompare(IfCond, CurLevel, Preds, Refs, !If->isUnknownLoopBottomTest());
  assert(!Preds.empty() && "No predicates found for compare instruction!");
  assert((Refs.size() == (2 * Preds.size())) &&
         "Mismatch between number of predicates and DDRefs!");

  if (HLoop) {
    if (LF.requiresZttInversion(HLoop)) {
      Preds[0].Kind = CmpInst::getInversePredicate(Preds[0].Kind);
      assert((Preds.size() == 1) &&
             "Single predicate expected for inversion candidates!");
    }

    HLoop->replaceZttPredicate(BeginPredIter, Preds[0]);
    HLoop->setZttPredicateOperandDDRef(Refs[0], BeginPredIter, true);
    HLoop->setZttPredicateOperandDDRef(Refs[1], BeginPredIter, false);
  } else {
    If->replacePredicate(BeginPredIter, Preds[0]);
    If->setPredicateOperandDDRef(Refs[0], BeginPredIter, true);
    If->setPredicateOperandDDRef(Refs[1], BeginPredIter, false);
  }

  for (unsigned I = 1, E = Preds.size(); I < E; ++I) {
    HLoop ? HLoop->addZttPredicate(Preds[I], Refs[2 * I], Refs[2 * I + 1])
          : If->addPredicate(Preds[I], Refs[2 * I], Refs[2 * I + 1]);
  }
}

void HIRParser::postParse(HLIf *If) {

  auto PredIter = If->pred_begin();
  unsigned NumPreds = If->getNumPredicates();

  // If 'then' is empty, move 'else' children to 'then' by inverting predicate.
  if ((NumPreds == 1) && !If->hasThenChildren()) {
    If->invertPredicate(PredIter);
    HLNodeUtils::moveAsFirstThenChildren(If, If->else_begin(), If->else_end());
    return;
  }

  // Converts-
  //
  // if () {
  //   goto bb;
  // } else {
  //   A[i] = t;
  // }
  //
  // To-
  //
  // if () {
  //   goto bb;
  // }
  // A[i] = t;
  if (auto Goto = dyn_cast_or_null<HLGoto>(If->getLastThenChild())) {
    (void)Goto;
    HLNodeUtils::moveAfter(If, If->else_begin(), If->else_end());
    return;
  }

  // Converts-
  //
  // if (Cond) {
  //   A[i] = t;
  // } else {
  //   goto bb;
  // }
  //
  // To-
  //
  // if (!Cond) {
  //   goto bb;
  // }
  // A[i] = t;
  if (auto Goto = dyn_cast_or_null<HLGoto>(If->getLastElseChild())) {
    (void)Goto;
    if (NumPreds == 1) {
      If->invertPredicate(PredIter);

      HLNodeUtils::moveAfter(If, If->then_begin(), If->then_end());
      HLNodeUtils::moveAsFirstThenChildren(If, If->else_begin(),
                                           If->else_end());
    }
  }
}

void HIRParser::parse(HLSwitch *Switch) {
  RegDDRef *CaseValRef = nullptr;
  unsigned CaseNum = 1;

  setCurNode(Switch);

  auto SrcBB = HIRC.getSrcBBlock(Switch);
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
    CaseValRef = createScalarDDRef(I->getCaseValue(), CurLevel);
    Switch->setCaseValueDDRef(CaseValRef, CaseNum);
  }
}

unsigned HIRParser::getPointerElementSize(Type *Ty) const {
  assert(isa<PointerType>(Ty) && "Invalid type!");

  auto ElTy = cast<PointerType>(Ty)->getElementType();

  return ElTy->isSized() ? getCanonExprUtils().getTypeSizeInBytes(ElTy) : 0;
}

const Value *HIRParser::getHeaderPhiOperand(const PHINode *Phi,
                                            bool IsInit) const {
  assert(RI.isHeaderPhi(Phi) && "Phi is not a header phi!");
  assert((Phi->getNumIncomingValues() == 2) &&
         "Unexpected number of header phi predecessors!");

  auto *Lp = LI.getLoopFor(Phi->getParent());
  auto *LatchBlock = Lp->getLoopLatch();

  auto *IncomingBlock = Phi->getIncomingBlock(0);

  if (IncomingBlock == LatchBlock) {
    return IsInit ? Phi->getIncomingValue(1) : Phi->getIncomingValue(0);
  } else {
    return IsInit ? Phi->getIncomingValue(0) : Phi->getIncomingValue(1);
  }
}

const Value *HIRParser::getHeaderPhiInitVal(const PHINode *Phi) const {
  return getHeaderPhiOperand(Phi, true);
}

const Value *HIRParser::getHeaderPhiUpdateVal(const PHINode *Phi) const {
  return getHeaderPhiOperand(Phi, false);
}

static bool hasNonGEPAccess(const Instruction *AddRecPhi,
                            const Instruction *PhiUpdateInst) {
  auto Inst = PhiUpdateInst;

  // Trace pointers starting from PhiUpdateInst until we reach AddRecPhi.
  while (Inst != AddRecPhi) {
    if (auto GEPInst = dyn_cast<GEPOrSubsOperator>(Inst)) {
      Inst = cast<Instruction>(GEPInst->getPointerOperand());
    } else {
      // Some other kind of instruction is involved, probably a bitcast
      // instruction.
      return true;
    }
  }

  return false;
}

CanonExpr *HIRParser::createHeaderPhiIndexCE(const PHINode *Phi,
                                             unsigned Level) {
  auto UpdateVal = getHeaderPhiUpdateVal(Phi);

  auto PhiSCEV = getSCEV(const_cast<PHINode *>(Phi));
  auto UpdateSCEV = getSCEV(const_cast<Value *>(UpdateVal));

  // Create stride as (update - phi). For example-
  // PhiSCEV: {%ptr,+,4)
  // UpdateSCEV : {(%ptr + 4),+,4)
  // StrideSCEV : 4
  auto StrideSCEV = SE.getMinusSCEV(UpdateSCEV, PhiSCEV);
  auto StrideTy = StrideSCEV->getType();
  assert(StrideTy->isIntegerTy() && "stride is not an integer!");

  // Create index as {0,+,stride}
  auto InitSCEV = SE.getConstant(StrideTy, 0);
  auto *Lp = LI.getLoopFor(Phi->getParent());

  // Stride can be variant in some weird cases when AddRec was formed using
  // control flow analysis. Refer to parser test- variant-addrec-stride.ll
  if (!SE.isLoopInvariant(StrideSCEV, Lp)) {
    return nullptr;
  }

  auto IndexSCEV =
      SE.getAddRecExpr(InitSCEV, StrideSCEV, Lp,
                       cast<SCEVAddRecExpr>(PhiSCEV)->getNoWrapFlags());

  std::unique_ptr<CanonExpr> IndexCE(
      getCanonExprUtils().createCanonExpr(StrideTy));

  // Disable cast hiding to prevent possible merging issues.
  if (!parseRecursive(IndexSCEV, IndexCE.get(), Level, true, true, true)) {
    return nullptr;
  }

  auto PhiTy = Phi->getType();

  int64_t OrigDenom = IndexCE->getDenominator();
  (void)OrigDenom;

  // Divide by element size to convert byte offset to number of elements.
  IndexCE->divide(getPointerElementSize(PhiTy));
  // Index is already multiplied by element size as the SCEV form is in bytes so
  // it should be okay to simplify denominator.
  IndexCE->simplify(true, true);

  // Bail out if element size does not divide stride evenly and Phi has an
  // unusual access pattern.
  if ((IndexCE->getDenominator() != 1) &&
      hasNonGEPAccess(Phi, cast<Instruction>(UpdateVal))) {
    return nullptr;
  }

  assert(IndexCE->getDenominator() <= OrigDenom &&
         "Could not simplify denominator!");

  return IndexCE.release();
}

void HIRParser::mergeIndexCE(CanonExpr *IndexCE1, const CanonExpr *IndexCE2) {
  // The behavior we want here is somewhere between relaxed and non-relaxed
  // mode. We only tolerate merging zero of a different type.
  // TODO: look into directly using add() utility.

  if (IndexCE2->isZero()) {
    return;
  }

  if (IndexCE1->isZero()) {
    IndexCE1->setSrcType(IndexCE2->getSrcType());
    IndexCE1->setDestType(IndexCE2->getDestType());
    IndexCE1->setExtType(IndexCE2->isSExt());
  }

  assert(CanonExprUtils::mergeable(IndexCE1, IndexCE2) &&
         "Indices cannot be merged!");
  CanonExprUtils::add(IndexCE1, IndexCE2);
}

void HIRParser::populateOffsets(const GEPOrSubsOperator *GEPOp,
                                SmallVectorImpl<int64_t> &Offsets) {

  Offsets.clear();

  // First index can never be a structure field offset.
  Offsets.push_back(-1);

  unsigned NumOp = GEPOp->getNumIndices();
  auto CurTy = cast<PointerType>(GEPOp->getPointerOperand()->getType())
                   ->getElementType();

  // Ignore first index.
  for (unsigned I = 1; I < NumOp; ++I) {
    assert(isa<GEPOperator>(GEPOp) && "Only GEP operators expected here");

    if (auto SeqTy = dyn_cast<SequentialType>(CurTy)) {
      CurTy = SeqTy->getElementType();
      Offsets.push_back(-1);

    } else {
      assert(isa<StructType>(CurTy) && "Unexpected type encountered!");
      auto StrucTy = cast<StructType>(CurTy);

      auto IndexOperand = GEPOp->getIndex(I);

      assert(isa<ConstantInt>(IndexOperand) &&
             "Structure offset is not a constant!");
      auto OffsetVal = cast<ConstantInt>(IndexOperand)->getZExtValue();

      CurTy = StrucTy->getElementType(OffsetVal);
      Offsets.push_back(OffsetVal);
    }
  }
}

bool HIRParser::representsStructOffset(const GEPOrSubsOperator *GEPOp) {
  if (!isa<GEPOperator>(GEPOp)) {
    // Struct offsets may not be represented by Subscript intrinsics.
    return false;
  }

  SmallVector<int64_t, 8> Offsets;
  populateOffsets(GEPOp, Offsets);

  return (Offsets[GEPOp->getNumOperands() - 2] != -1);
}

bool HIRParser::isValidGEPOp(const GEPOrSubsOperator *GEPOp,
                             bool SkipLiveRangeCheck) const {

  auto GEPInst = dyn_cast<Instruction>(GEPOp);

  if (!SkipLiveRangeCheck && GEPInst &&
      SE.getHIRMetadata(GEPInst, ScalarEvolution::HIRLiveKind::LiveRange)) {
    return false;
  }

  // Unsupported types for instructions inside the region has already been
  // checked by region identification pass.
  return ((GEPInst && CurRegion->containsBBlock(GEPInst->getParent())) ||
          !HIRRegionIdentification::containsUnsupportedTy(GEPOp));
}

// Populates \p IndexedTypes vector with types being indexed by the \p GEPOp.
// ex.:
//   -- gep [10 x { i8, float }]* %p, 0, i, 1
//
//   { [10 x { i8, float }]*,
//     [10 x { i8, float }],
//           { i8, float },
//                 float
//   }
//
// Note: the pointer operand type is always added as a first element.
static void populateIndexedTypes(const GEPOperator *GEPOp,
                                 SmallVectorImpl<Type *> &IndexedTypes) {
  IndexedTypes.push_back(GEPOp->getPointerOperandType());
  for (auto I = gep_type_begin(GEPOp), E = gep_type_end(GEPOp); I != E; ++I) {
    IndexedTypes.push_back(I.getIndexedType());
  }
}

class DimInfo {
  Type *Ty = nullptr;
  Value *Stride = nullptr;

  SmallVector<Value *, 4> Indices;
  SmallVector<Value *, 4> IndicesLB;

  bool HasZeroIndex = false;

public:
  Type *getType() const { return Ty; }
  void setType(Type *Ty) { this->Ty = Ty; }

  Value *getStride() const { return Stride; }
  void setStride(Value *Stride) { this->Stride = Stride; }

  // Adds an \p Idx to the dimension. Later these indices will be merged into a
  // single CanonExpr.
  // Note: Nullptr LB means that it's zero based.
  void addIndex(Value *Idx, Value *LB) {
    // Do not collect more than one zero index.

    if (!LB) {
      LB = ConstantInt::get(Idx->getType(), 0);
    }

    if (auto *ConstIdx = dyn_cast<ConstantInt>(Idx)) {
      APInt ZeroBasedIndex = ConstIdx->getValue();

      auto *ConstLB = dyn_cast_or_null<ConstantInt>(LB);
      if (ConstLB && !ConstLB->isZero()) {
        unsigned DiffWidth = std::max(Idx->getType()->getPrimitiveSizeInBits(),
                                      LB->getType()->getPrimitiveSizeInBits());

        bool SubOverflow = false;
        ZeroBasedIndex = ZeroBasedIndex.sextOrSelf(DiffWidth).ssub_ov(
            ConstLB->getValue().sextOrSelf(DiffWidth), SubOverflow);
        assert(!SubOverflow && "Unexpected (index - lower) overflow");
      }

      if (ZeroBasedIndex.isNullValue()) {
        if (Indices.size() > 0) {
          return;
        }

        HasZeroIndex = true;
        Indices.push_back(Idx);
        IndicesLB.push_back(LB);
        return;
      }
    }

    if (HasZeroIndex) {
      HasZeroIndex = false;
      Indices.clear();
      IndicesLB.clear();
    }

    Indices.push_back(Idx);
    IndicesLB.push_back(LB);
  }

  void clear() {
    Ty = nullptr;
    Stride = nullptr;
    Indices.clear();
    IndicesLB.clear();
  }

  bool isDefined() const {
    assert((!Stride == Indices.empty()) &&
           "Indices found in not populated dimension");
    return Stride;
  }

  bool isZero() const {
    assert(isDefined() && "Expected a populated dimension");
    return HasZeroIndex;
  }

  const ArrayRef<Value *> indices() const { return Indices; }
  const ArrayRef<Value *> indicesLB() const { return IndicesLB; }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  LLVM_DUMP_METHOD
  void print(raw_ostream &OS) const {
    OS << "(";
    Ty->print(OS);
    OS << ")";
    OS << "[";
    for (auto *IdxLB : IndicesLB) {
      IdxLB->printAsOperand(OS, false);
      OS << ",";
    }
    OS << ":";
    for (auto *Idx : Indices) {
      Idx->printAsOperand(OS, false);
      OS << ",";
    }
    OS << ":";
    Stride->printAsOperand(OS, false);
    OS << "]";
  }

  LLVM_DUMP_METHOD
  void dump() const {
    print(dbgs());
    dbgs() << "\n";
  }
#endif
};

// Represents information about single array object that contains information
// about dimensions and optional struct offsets for that array.
class ArrayInfo {
  unsigned MinRank = -1;

  SmallVector<DimInfo, 4> Dimensions;
  SmallVector<unsigned, 4> StructOffsets;

public:
  using iterator = decltype(Dimensions)::const_iterator;

  SmallVectorImpl<unsigned> &offsets() { return StructOffsets; }
  const SmallVectorImpl<unsigned> &offsets() const { return StructOffsets; }

  DimInfo &getOrCreate(unsigned Idx) {
    if (Dimensions.size() <= Idx) {
      Dimensions.resize(Idx + 1);
    }

    MinRank = std::min(MinRank, Idx);

    return Dimensions[Idx];
  }

  void pop_back() {
    Dimensions.pop_back();
  }

  const DimInfo &getDim(unsigned Idx) const {
    assert(Idx >= getMinRank() && Idx <= getMaxRank() && "Invalid rank");
    return Dimensions[Idx];
  }

  const DimInfo &getHighestDim() const { return Dimensions[getMaxRank()]; }
  const DimInfo &getLowestDim() const { return Dimensions[getMinRank()]; }

  unsigned getMinRank() const { return MinRank; }
  unsigned getMaxRank() const { return Dimensions.size() - 1; }

  iterator begin() const { return Dimensions.begin(); }
  iterator end() const { return Dimensions.end(); }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  LLVM_DUMP_METHOD
  void print(raw_ostream &OS) const {
    for (auto &Dim : reverse(Dimensions)) {
      Dim.print(dbgs());
      dbgs() << " | ";
    }

    for (auto Offset : StructOffsets) {
      OS << "." << Offset;
    }
  }

  LLVM_DUMP_METHOD
  void dump() const {
    print(dbgs());
    dbgs() << "\n";
  }
#endif
};

// A chain of GEP operators and Subscript intrinsics that may be
// parsed as a single RegDDRef.
//
// It organizes information on indices in groups that correspond to the
// same arrays. Struct offsets are stored as trailing offsets to the arrays.
//
// Assume a reference: (%p)[A].0[B][C].1.2[D]
//                         |---||--------||-|
//
// During parsing, each |--| block was represented by ArrayInfo, where each
// ArrayInfo was built of Dimensions and Offsets-
//
//   Dims: {[r:0,s:100,i:A,lb:0]}                     Offsets: {0}
//   Dims: {[r:1,s:10,i:B,lb:0], [r:0,s:1,i:C,lb:0]}  Offsets: {1, 2}
//   Dims: {[r:0,s:1,i:D,lb:0]}                       Offsets: {}
//
class HIRParser::GEPChain {
  const GEPOrSubsOperator *Base;
  IntegerType *OffsetTy;
  std::list<ArrayInfo> Arrays;

  SmallVector<GEPOperator *, 1> RestructuredGeps;

  // Extend the current chain with the next \p GEPOp.
  // Returns false if next gep or subs operator is not compatible with current
  // chain.
  bool extend(const HIRParser &Parser, const GEPOrSubsOperator *GEPOp);

  void setBase(const GEPOrSubsOperator *BaseOp) { Base = BaseOp; }

  std::list<ArrayInfo> parseGEPOp(const SubscriptInst *GEPOp);
  std::list<ArrayInfo> parseGEPOp(const HIRParser &Parser,
                                  const GEPOperator *GEPOp);

  // Returns [ArrayInfo] representation for the \p GEPOp;
  std::list<ArrayInfo> parseGEPOp(const HIRParser &Parser,
                                  const GEPOrSubsOperator *GEPOp) {
    auto *Subs = dyn_cast<SubscriptInst>(GEPOp);
    if (Subs) {
      return parseGEPOp(Subs);
    }

    return parseGEPOp(Parser, cast<GEPOperator>(GEPOp));
  }

  // Return true if next information about indices may be merged into a single
  // chain.
  bool isCompatible(const ArrayInfo &NextAI) const;

public:
  using iterator = decltype(Arrays)::const_iterator;

  IntegerType *getOffsetTy() const { return OffsetTy; }

  GEPChain(const HIRParser &Parser, const GEPOrSubsOperator *GEPOp);
  ~GEPChain() {
    for (auto *V : RestructuredGeps) {
      V->deleteValue();
    }
  }

  const GEPOrSubsOperator *getBase() const { return Base; }

  iterator begin() const { return Arrays.begin(); }
  iterator end() const { return Arrays.end(); }

  const ArrayInfo &front() const { return Arrays.front(); }
  const ArrayInfo &back() const { return Arrays.back(); }
};

HIRParser::GEPChain::GEPChain(const HIRParser &Parser,
                              const GEPOrSubsOperator *GEPOp) {
  OffsetTy = Type::getIntNTy(
      Parser.getContext(),
      Parser.getDataLayout().getTypeSizeInBits(GEPOp->getType()));

  extend(Parser, GEPOp);
  setBase(GEPOp);

  while (const GEPOrSubsOperator *NextGEPOp =
             dyn_cast<GEPOrSubsOperator>(GEPOp->getPointerOperand())) {

    if (!Parser.isValidGEPOp(NextGEPOp)) {
      break;
    }

    auto *Subs = dyn_cast<SubscriptInst>(GEPOp);
    auto *NextSubs = dyn_cast<SubscriptInst>(NextGEPOp);

    // Stop trace back if the highest index looks like casted IV: sext(i1).
    // This is a profitability check to form more linear indices.
    // If we trace back, we may have to add offsets to casted IV which will make
    // the index non-linear.
    if ((!Subs && !NextSubs) ||
        (Subs && NextSubs && Subs->getRank() == NextSubs->getRank())) {
      auto *HighestIndex = Parser.getSCEV(GEPOp->getIndex(0));
      auto CastSCEV = dyn_cast<SCEVCastExpr>(HighestIndex);
      if (CastSCEV && isa<SCEVAddRecExpr>(CastSCEV->getOperand())) {
        break;
      }
    }

    if (!extend(Parser, NextGEPOp)) {
      break;
    }

    GEPOp = NextGEPOp;
    setBase(GEPOp);
  }
}

std::list<ArrayInfo> HIRParser::GEPChain::parseGEPOp(const SubscriptInst *Sub) {
  ArrayInfo Arr;

  DimInfo &Dim = Arr.getOrCreate(Sub->getTypeRank());
  Dim.setType(Sub->getType());
  Dim.setStride(Sub->getStride());
  Dim.addIndex(Sub->getIndex(), Sub->getLowerBound());

  return { Arr };
}

/// This function returns restructured one-past-the-end GEPOperator if
/// applicable, otherwise returns nullptr.
///
/// For a global pointer of type *[10 x i32], given a reference to one past the
/// end element like &A[0][10], optimizer can turn it into equivalent &A[1][0].
/// This is problematic for DD so we reverts it into the original form.
static GEPOperator *
restructureOnePastTheEndGEP(const GEPOperator *GEPOrSubsOp) {

  // As far as I know, this restructuring only happens with constant gep exprs.
  // Assuming this for GEP instructions might not be correct.
  if (isa<Instruction>(GEPOrSubsOp)) {
    return nullptr;
  }

  const GEPOperator *GEPOp =
      cast<GEPOperator>(static_cast<const Operator *>(GEPOrSubsOp));

  if (GEPOp->getNumIndices() != 2) {
    return nullptr;
  }

  auto *FirstIndex = dyn_cast<ConstantInt>(*GEPOp->idx_begin());

  if (!FirstIndex || !FirstIndex->isOne()) {
    return nullptr;
  }

  auto *SecondIndex = dyn_cast<ConstantInt>(*(GEPOp->idx_begin() + 1));

  if (!SecondIndex || !SecondIndex->isZero()) {
    return nullptr;
  }

  auto *ArrTy = dyn_cast<ArrayType>(GEPOp->getSourceElementType());

  if (!ArrTy) {
    return nullptr;
  }

  SmallVector<Value *, 8> Indices;

  // Make first index zero.
  Indices.push_back(SecondIndex);

  // Second index is equal to number of elements.
  Indices.push_back(
      ConstantInt::get(SecondIndex->getType(), ArrTy->getNumElements()));

  IRBuilder<NoFolder> Builder(ArrTy->getContext());

  return cast<GEPOperator>(Builder.CreateGEP(
      const_cast<Value *>(GEPOp->getPointerOperand()), Indices));
}

std::list<ArrayInfo> HIRParser::GEPChain::parseGEPOp(const HIRParser &Parser,
                                                     const GEPOperator *GEPOp) {
  assert(GEPOp->getNumIndices() > 0 && "Corrupted GEP operator");

  std::list<ArrayInfo> Output = { ArrayInfo() };

  GEPOperator *RestructuredGep = restructureOnePastTheEndGEP(GEPOp);
  if (RestructuredGep) {
    RestructuredGeps.push_back(RestructuredGep);
    GEPOp = RestructuredGep;
  }

  SmallVector<Type *, 8> IndexedTypes;
  populateIndexedTypes(GEPOp, IndexedTypes);

  bool CreateNewArray = false;
  ArrayInfo *Arr = &Output.back();

  auto GetTypeRank = [](Type *DimElemTy) {
    unsigned TypeRank = 0;
    while (DimElemTy->isArrayTy()) {
      ++TypeRank;
      DimElemTy = DimElemTy->getArrayElementType();
    }
    return TypeRank;
  };

  unsigned Rank = GetTypeRank(IndexedTypes[1]);

  for (unsigned I = 0, N = GEPOp->getNumIndices(); I < N; ++I) {
    Value *Index = *(GEPOp->idx_begin() + I);

    Type *DimTy = IndexedTypes[I];
    Type *DimElemTy = IndexedTypes[I + 1];

    if (DimTy->isStructTy()) {
      CreateNewArray = true;
      Arr->offsets().push_back(cast<ConstantInt>(Index)->getZExtValue());
      continue;
    }

    if (CreateNewArray) {
      // Create dimension for index coming just after struct offset.
      Output.push_back({});
      Arr = &Output.back();

      // Calculate maximum rank of the next array.
      Rank = GetTypeRank(DimElemTy);

      CreateNewArray = false;
    }

    auto &Dim = Arr->getOrCreate(Rank);
    Dim.setType(DimTy);
    Dim.addIndex(Index, nullptr);

    auto Stride = DimElemTy->isSized()
                      ? Parser.getCanonExprUtils().getTypeSizeInBytes(DimElemTy)
                      : 0;

    Dim.setStride(ConstantInt::get(OffsetTy, Stride));

    --Rank;
  }

  return Output;
}

// Every GEP or subscript has information about a stride for particular
// dimension. Adding a gep to an existing chain extends our knowledge of memory
// configuration - it may add a new dimension or provide a more specific
// dimension type.
//
// Each new gep should either correspond to the existing information or add new
// information, and new information should not conflict with existing one.
//
// Ex.: assume the following chain-
//
//    %0 = gep [1000 x i32], [1000 x i32]* %p, 0, 0
//    %1 = subs %0, r:1, s:40, i
//    %2 = subs %1, r:0, s:4, j
//
// While parsing first two subscripts we may conclude some memory is accessed in
// a 2D manner with a configuration-
//
//    [i:stride 40][j:stride 4]
//
// and having next operator: %0, which correspond to 1D access manner with
// stride info-
//
//    [0:stride 4000][0:stride 4]
//
// we may not be able to represent access in terms of %p, but rather represent
// it in terms of %0: (%0)[i:40][j:4].
//
// This is because new gep stride of dimension one (4000) conflicts with
// existing knowledge about the stride (40).
//
// However we may still represent it as (%p)[0:4000][i:40][j:4],
// but we would loose type consistency between dimensions.
bool HIRParser::GEPChain::isCompatible(const ArrayInfo &NextArr) const {
  const ArrayInfo &CurArr = Arrays.front();

  // If NextArr has a struct offset and current first first index is not
  // zero then we have an unconventional structure access and the GEPs cannot
  // be merged. For example-
  //
  // %struct.IspComplex = type { float, float }
  //
  // %53 = gep %struct.IspComplex, %struct.IspComplex* %P, i64 %i, i32 0
  // %add.ptr.i.i = gep float, float* %53, i64 %j
  //
  // In the above example the first GEP references a float field in the
  // structure but the second GEP treats it as a floating point array whose
  // %j'th element is being accessed.
  if (!NextArr.offsets().empty()) {
    return CurArr.getHighestDim().isZero();
  }

  // Check that strides match for existing ranks.
  // Note: it's possible that NextArr will not share any ranks with CurArr.
  // For example it happens in the chain of subscripts-
  //   %0 = subs %p, r:1, s1, i1
  //   %1 = subs %0, r:0, s2, i2
  // %0 subscript will not share any ranks with existing information.
  unsigned MaxCommonRank = std::min(NextArr.getMaxRank(), CurArr.getMaxRank());
  unsigned MinCommonRank = std::max(NextArr.getMinRank(), CurArr.getMinRank());
  for (auto Rank = MinCommonRank; Rank <= MaxCommonRank; ++Rank) {
    auto *S1 = CurArr.getDim(Rank).getStride();
    auto *S2 = NextArr.getDim(Rank).getStride();

    // Skip gaps in current array info.
    if (!S1) {
      // Example chain for (%p)[a+x][b][c+y]:
      //   %0 = gep [10 x [10 x i8]]* %p, a, b, c
      //   %1 = subs %0, r:2, 100, x
      //   %2 = subs %1, r:0, 1, y
      //
      // This condition is true when %2 and %1 are already parsed into CurrArr
      // and %0 is in NextArr.
      continue;
    }

    assert(S2 && "Unexpected stride gaps in NextArr");

    if (S1 != S2) {
      return false;
    }
  }

  return true;
}

// Parses the \p GEPOp and updates chain state if GEPOp is compatible.
bool HIRParser::GEPChain::extend(const HIRParser &Parser,
                                 const GEPOrSubsOperator *GEPOp) {
  std::list<ArrayInfo> NextArrays = parseGEPOp(Parser, GEPOp);

  if (Arrays.empty()) {
    Arrays = std::move(NextArrays);
    return true;
  }

  ArrayInfo &NextArr = NextArrays.back();

  if (!isCompatible(NextArr)) {
    return false;
  }

  ArrayInfo &CurArr = Arrays.front();

  // Merging of NextArr into CurArr-
  if (!NextArr.offsets().empty()) {
    // ex.: (%p)[0][a].1[b]
    //
    // %struct = type { i32, [4 x i8] }
    // %0 = gep [10 x %struct]* %p, 0, a, 1
    // %1 = gep [4 x i8]*, %0, 0, b
    //
    // NextArr: [s:80,i:0][s:8,i:a].1   // NextArr contain offsets.
    // CurArr:  [s:4,i:0][s:1,i:b]
    //
    // Note: NextArr with struct offsets would be compatible only if the first
    // subscript of CurArr is zero.
    //
    // In this case we just need to drop highest dimension of CurrArr and
    // attach NextArr to the CurArr-
    //
    // CurArr:  [s:80,i:0][s:8,i:a].1[s:1,i:b]
    //

    // If CurArr contains a single dimension we don't want to loose existing
    // information about its struct offsets.
    if (CurArr.getMaxRank() == CurArr.getMinRank()) {
      // ex.: (%p)[0][a].1.5
      //
      // %struct = type { i32, %another_struct }
      // %0 = gep [10 x %struct]* %p, 0, a, 1
      // %1 = gep %another_struct*, %0, 0, 5
      //
      // NextArr: [s:80,i:0][s:8,i:a].1
      // CurArr:  [s:4,i:0].5
      //
      // In that case we copy existing offsets from CurArr to NextArr and
      // attach NextArr to the CurArr-
      //
      // CurArr:  [s:80,i:0][s:8,i:a].1.5

      // Append CurArr struct offsets
      NextArr.offsets().append(CurArr.offsets().begin(),
                               CurArr.offsets().end());
      CurArr.offsets().clear();
    }

    CurArr.pop_back();
  } else {
    // Both CurrArr and NextArr have information about the same array.
    // We need to merge dimensions of NextArr into CurArr.
    //
    // ex.: (%p)[a+x][b][c+y]:
    // %0 = gep [10 x [10 x i8]]* %p, a, b, c
    // %1 = subs %0, r:2, 100, x
    // %2 = subs %1, r:0, 1, y
    //
    // Assume we already parsed %2, %1 and %0 is next operator.
    //
    // Ranks:        2          1        0
    // NextArr: [s:100,i:a][s:10,i:b][s:1,i:c]
    // CurArr:  [s:100,i:x]          [s:1,i:y]

    for (unsigned I = NextArr.getMinRank(), E = NextArr.getMaxRank(); I <= E;
         ++I) {
      auto &CurDim = CurArr.getOrCreate(I);
      auto &NextDim = NextArr.getDim(I);

      CurDim.setType(NextDim.getType());
      CurDim.setStride(NextDim.getStride());

      for (auto Idx : zip(NextDim.indices(), NextDim.indicesLB())) {
        CurDim.addIndex(std::get<0>(Idx), std::get<1>(Idx));
      }
    }

    // Remove merged array info.
    NextArrays.pop_back();
  }

  Arrays.splice(Arrays.begin(), NextArrays);

  return true;
}

const GEPOrSubsOperator *
HIRParser::getBaseGEPOp(const GEPOrSubsOperator *GEPOp) const {
  // TODO: implement caching for GEP chains.

  return GEPChain(*this, GEPOp).getBase();
}

// Consider the following sequence of GEPs-
// %arrayidx = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x
// i32]]* @B, i64 0, i64 %i
// %arrayidx5 = getelementptr inbounds [100 x i32], [100 x i32]* %arrayidx,
// i64 0, i64 %j
//
// %0 = load i32, i32* %arrayidx5, align 4
//
// This is how the dimensions are created-
// 1) Start processing %arrayidx5's operands in reverse and create a
//   dimension for %j. Store operand '0' till the processing of next GEP.
// 2) Start processing %arrayidx's operands in reverse. Add the last index %i
//   to the stored '0' operand from %arrayidx5 and create a new dimension with
//   the resulting value.
// 3) In the end, create additional dimension for the remaining '0' operand.
//
// The parsed DDRef looks like this- (@B][0][%i][%j]
void HIRParser::populateRefDimensions(RegDDRef *Ref,
                                      const GEPOrSubsOperator *GEPOp,
                                      unsigned Level,
                                      bool RequiresIndexMerging) {

  GEPChain Chain(*this, GEPOp);
  auto *OffsetTy = Chain.getOffsetTy();

  // If Ref has existing dimensions we may have to start from merging in the
  // highest dimension, but only if the first dimension we just parsed does not
  // contain struct offsets.
  bool MergeInHighestDimension =
      (Ref->getNumDimensions() != 0) && Chain.back().offsets().empty();

  for (auto &Arr : reverse(Chain)) {

    for (auto &Dim : Arr) {
      if (!Dim.isDefined()) {
        continue;
      }

      CanonExpr *IndexCE = nullptr;
      CanonExpr *LowerCE = nullptr;

      bool IsTop = (Dim.indices().size() == 1) && !MergeInHighestDimension &&
                   !RequiresIndexMerging;

      for (const auto &PairV : zip(Dim.indices(), Dim.indicesLB())) {
        std::unique_ptr<CanonExpr> ICE(
            parse(std::get<0>(PairV), Level, IsTop, OffsetTy));
        std::unique_ptr<CanonExpr> LCE(
            parse(std::get<1>(PairV), Level, IsTop, OffsetTy));

        if (IndexCE) {
          mergeIndexCE(IndexCE, ICE.get());
          mergeIndexCE(LowerCE, LCE.get());
        } else {
          IndexCE = ICE.release();
          LowerCE = LCE.release();
        }
      }

      // Merge into highest dimension as requested, but only if there are no
      // struct offsets.
      if (MergeInHighestDimension) {
        assert(Arr.offsets().empty() && "Can not contain struct offset");
        mergeIndexCE(Ref->getDimensionIndex(Ref->getNumDimensions()), IndexCE);
        mergeIndexCE(Ref->getDimensionLower(Ref->getNumDimensions()), LowerCE);
        Ref->setDimensionType(Ref->getNumDimensions(), Dim.getType());
        MergeInHighestDimension = false;
        continue;
      }

      CanonExpr *StrideCE = parse(Dim.getStride(), Level, true, OffsetTy);

      ArrayRef<unsigned> StructOffsets;
      if (&Dim == &Arr.getLowestDim()) {
        StructOffsets = Arr.offsets();
      }

      Ref->addDimensionHighest(IndexCE, StructOffsets, LowerCE, StrideCE,
                               Dim.getType());
    }
  }
}

void HIRParser::addPhiBaseGEPDimensions(const GEPOrSubsOperator *GEPOp,
                                        const GEPOrSubsOperator *InitGEPOp,
                                        RegDDRef *Ref, CanonExpr *IndexCE,
                                        CanonExpr *StrideCE, Type *DimType,
                                        unsigned Level) {
  // First populate the dimensions using the GEPOperator that we started
  // parsing from and then merge IndexCE into resulting Ref's highest dimension.
  if (GEPOp || (Ref->getNumDimensions() != 0)) {
    if (GEPOp) {
      populateRefDimensions(Ref, GEPOp, Level, !IndexCE->isZero());
    }

    auto HighestDimCE = Ref->getDimensionIndex(Ref->getNumDimensions());
    mergeIndexCE(HighestDimCE, IndexCE);
    getCanonExprUtils().destroy(IndexCE);
  } else {
    Ref->addDimensionHighest(IndexCE, {}, nullptr, StrideCE, DimType);
  }

  // Extra dimensions are involved when the initial value of BasePhi is computed
  // using an array/structure like the following-
  // %p.07 = phi i32* [ %incdec.ptr, %for.body ], [ getelementptr inbounds ([50
  // x i32], [50 x i32]* @A, i64 0, i64 10), %entry ]
  if (InitGEPOp) {
    populateRefDimensions(Ref, InitGEPOp, Level, false);
  }
}

const Value *
HIRParser::getValidPhiBaseVal(const Value *PhiInitVal,
                              const GEPOrSubsOperator **InitGEPOp) const {

  *InitGEPOp = nullptr;

  auto GEPOp = dyn_cast<GEPOrSubsOperator>(PhiInitVal);

  if (!GEPOp) {
    return PhiInitVal;
  }

  // A phi init GEP representing an offset cannot be merged into the ref as it
  // represents an unconventional access.
  if (representsStructOffset(GEPOp) || !isValidGEPOp(GEPOp)) {
    // If this is an instruction, we can use it as the base.
    if (isa<Instruction>(PhiInitVal)) {
      return PhiInitVal;
    }

    // PhiInitVal is a constant expr, return null to indicate that the phi
    // itself should act as the base.
    return nullptr;
  }

  *InitGEPOp = GEPOp;
  return getBaseGEPOp(GEPOp)->getPointerOperand();
}

RegDDRef *HIRParser::createPhiBaseGEPDDRef(const PHINode *BasePhi,
                                           const GEPOrSubsOperator *GEPOp,
                                           unsigned Level) {
  const PHINode *CurBasePhi = BasePhi;
  const Value *BaseVal = nullptr;
  bool IsInBounds = false;

  if (GEPOp) {
    IsInBounds = GEPOp->isInBounds();
  } else {
    // Use tha phi update value to apply inbounds.
    // Technically, it is possible for the initial val of phi (first iteration
    // of loop) to not be 'inbounds' and the rest to be inbounds but not sure if
    // it is even possible to generate such IR from the frontend.
    auto *UpdateVal = getHeaderPhiUpdateVal(BasePhi);

    if (auto *GEPInst = dyn_cast<GetElementPtrInst>(UpdateVal)) {
      IsInBounds = GEPInst->isInBounds();
    }
  }

  RegDDRef *Ref = getDDRefUtils().createRegDDRef(0);

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

  auto OffsetTy = Type::getIntNTy(
      getContext(), getDataLayout().getTypeSizeInBits(CurBasePhi->getType()));

  // A phi can be initialized using another phi so we should trace back.
  do {
    const GEPOrSubsOperator *InitGEPOp = nullptr;
    CanonExpr *IndexCE = nullptr;
    auto SC = getSCEV(const_cast<PHINode *>(CurBasePhi));

    if (auto RecSCEV = dyn_cast<SCEVAddRecExpr>(SC)) {
      const Value *PhiInitVal = getHeaderPhiInitVal(CurBasePhi);

      if (RecSCEV->isAffine() &&
          (BaseVal = getValidPhiBaseVal(PhiInitVal, &InitGEPOp))) {
        IndexCE = createHeaderPhiIndexCE(CurBasePhi, Level);
      }
    }

    // Non-linear base is parsed as base + zero offset: (%p)[0].
    if (!IndexCE) {
      BaseVal = CurBasePhi;
      IndexCE = getCanonExprUtils().createCanonExpr(OffsetTy);
    }

    Type *CurBasePhiTy = CurBasePhi->getType();

    CanonExpr *StrideCE = getCanonExprUtils().createCanonExpr(
        OffsetTy, 0, getPointerElementSize(CurBasePhiTy));

    addPhiBaseGEPDimensions(GEPOp, InitGEPOp, Ref, IndexCE, StrideCE,
                            CurBasePhiTy, Level);

    GEPOp = nullptr;
  } while ((CurBasePhi != BaseVal) &&
           (CurBasePhi = dyn_cast<PHINode>(BaseVal)) &&
           CurRegion->containsBBlock(CurBasePhi->getParent()));

  auto BaseCE = parse(BaseVal, Level);

  Ref->setBaseCE(BaseCE);
  Ref->setInBounds(IsInBounds);

  return Ref;
}

RegDDRef *HIRParser::createRegularGEPDDRef(const GEPOrSubsOperator *GEPOp,
                                           unsigned Level) {
  auto Ref = getDDRefUtils().createRegDDRef(0);

  const GEPOrSubsOperator *BaseGEPOp = getBaseGEPOp(GEPOp);
  auto BaseVal = BaseGEPOp->getPointerOperand();

  // TODO: This can be improved by first checking if the original SCEV can be
  // handled.
  CanonExpr *BaseCE = parse(BaseVal, Level);
  Ref->setBaseCE(BaseCE);

  populateRefDimensions(Ref, GEPOp, Level, false);

  Ref->setInBounds(GEPOp->isInBounds());

  return Ref;
}

RegDDRef *HIRParser::createSingleElementGEPDDRef(const Value *GEPVal,
                                                 unsigned Level) {

  auto Ref = getDDRefUtils().createRegDDRef(0);
  auto GEPTy = GEPVal->getType();
  auto OffsetTy =
      Type::getIntNTy(getContext(), getDataLayout().getTypeSizeInBits(GEPTy));

  // TODO: This can be improved by first checking if the original SCEV can be
  // handled.
  auto BaseCE = parse(GEPVal, Level);
  Ref->setBaseCE(BaseCE);

  // Create Index of zero.
  auto IndexCE = getCanonExprUtils().createCanonExpr(OffsetTy);

  Ref->addDimension(IndexCE);

  // Single element is always in bounds.
  Ref->setInBounds(true);

  return Ref;
}

// NOTE: AddRec->delinearize() doesn't work with constant bound arrays.
// TODO: handle struct GEPs.
RegDDRef *HIRParser::createGEPDDRef(const Value *GEPVal, unsigned Level,
                                    bool IsUse) {
  const PHINode *BasePhi = nullptr;
  const Value *OrigGEPVal = GEPVal;
  RegDDRef *Ref = nullptr;

  // Incoming IR may be bitcasting the GEP before loading/storing into it. If so
  // we store the type of the GEP in BaseCE src type and the eventual load/store
  // type in BaseCE dest type.
  Type *DestTy = GEPVal->getType();
  bool HasDestTy = false;

  clearTempBlobLevelMap();

  // Trace though consecutive bitcast operators until we hit something else.
  while (auto BCOp = dyn_cast<BitCastOperator>(GEPVal)) {

    if (auto BCInst = dyn_cast<Instruction>(BCOp)) {
      if (SE.getHIRMetadata(BCInst, ScalarEvolution::HIRLiveKind::LiveOut)) {
        break;
      }
    }

    auto Opnd = BCOp->getOperand(0);
    auto OpTy = Opnd->getType();
    if (!RI.isSupported(OpTy, true)) {
      break;
    }

    // Suppress tracing back to a function pointer type.
    if (OpTy->getPointerElementType()->isFunctionTy()) {
      break;
    }

    HasDestTy = true;
    GEPVal = Opnd;
  }

  const GEPOrSubsOperator *GEPOp = dyn_cast<GEPOrSubsOperator>(GEPVal);

  // Try to get to the phi associated with this GEP.
  if (GEPOp) {
    // Do not cross the live range indicator for GEP uses (load/store/bitcast).
    if (isValidGEPOp(GEPOp, !IsUse)) {
      BasePhi = dyn_cast<PHINode>(getBaseGEPOp(GEPOp)->getPointerOperand());
    } else {
      GEPOp = nullptr;
    }
  } else {
    BasePhi = dyn_cast<PHINode>(GEPVal);
  }

  if (BasePhi && RI.isHeaderPhi(BasePhi)) {
    Ref = createPhiBaseGEPDDRef(BasePhi, GEPOp, Level);
  } else if (GEPOp) {
    Ref = createRegularGEPDDRef(GEPOp, Level);
  } else {
    Ref = createSingleElementGEPDDRef(GEPVal, Level);
  }

  if (HasDestTy) {
    Ref->setBitCastDestType(DestTy);
  }

  populateBlobDDRefs(Ref, Level);

  populateRequiredSymbases(Ref);

  // Add a mapping for getting the original pointer value for the Ref.
  GEPRefToPointerMap.insert(
      std::make_pair(Ref, const_cast<Value *>(OrigGEPVal)));

  return Ref;
}

// Returns true if lval ref of \p Inst contains a blob and rval doesn't. This
// usually happens when the rvals have a linear form but the combination
// doesn't. For example- t1 = i1 * i2 linear form of t1 is t2 * i2 where t2 is
// the underlying LLVM value representing i1.
//
static bool hasLvalRvalBlobMismatch(const HLInst *HInst,
                                    const RegDDRef *LvalRef) {

  for (auto BlobIt = LvalRef->blob_begin(), EIt = LvalRef->blob_end();
       BlobIt != EIt; ++BlobIt) {
    unsigned BlobIndex = (*BlobIt)->getBlobIndex();

    bool FoundBlob = false;
    for (auto RefIt = HInst->rval_op_ddref_begin(),
              E = HInst->rval_op_ddref_end();
         RefIt != E; ++RefIt) {
      auto *Ref = *RefIt;

      assert(Ref->isTerminalRef() &&
             "unexpected rval ref for non self blob lval!");

      if (Ref->usesTempBlob(BlobIndex)) {
        FoundBlob = true;
        break;
      }
    }

    if (!FoundBlob) {
      return true;
    }
  }

  return false;
}

RegDDRef *HIRParser::createScalarDDRef(const Value *Val, unsigned Level,
                                       HLInst *LvalInst) {
  CanonExpr *CE;

  clearTempBlobLevelMap();

  bool IsLval = (LvalInst != nullptr);
  ParsingScalarLval = IsLval;

  auto Symbase = getOrAssignSymbase(Val);
  auto Ref = getDDRefUtils().createRegDDRef(Symbase);

  CE = parse(Val, Level);

  Ref->setSingleCanonExpr(CE);

  bool IsSelfBlob = CE->isSelfBlob();

  if (IsSelfBlob) {
    unsigned SB = getTempBlobSymbase(CE->getSingleBlobIndex());

    // Update rval DDRef's symbase to blob's symbase for self-blob DDRefs.
    if (!IsLval) {
      Ref->setSymbase(SB);
    }
    // If lval DDRef's symbase and blob's symbase don't match, we need to add a
    // blob DDRef.
    else if (Symbase != SB) {
      populateBlobDDRefs(Ref, Level);
    }

  } else if (CE->isConstant()) {
    if (!IsLval) {
      Ref->setSymbase(ConstantSymbase);
    }

  } else {
    // Assign a generic symbase to non self-blob rvals to avoid unnecessary dd
    // edges.
    if (!IsLval) {
      Ref->setSymbase(GenericRvalSymbase);
    }
    populateBlobDDRefs(Ref, Level);
  }

  if (IsLval && !IsSelfBlob && hasLvalRvalBlobMismatch(LvalInst, Ref)) {
    Ref->makeSelfBlob(true);
  }

  populateRequiredSymbases(Ref);

  ParsingScalarLval = false;

  return Ref;
}

void HIRParser::populateRequiredSymbases(const RegDDRef *Ref) {

  if (Ref->isSelfBlob()) {
    RequiredSymbases.insert(Ref->getSymbase());

  } else {
    for (auto BIt = Ref->blob_begin(), E = Ref->blob_end(); BIt != E; ++BIt) {
      RequiredSymbases.insert((*BIt)->getSymbase());
    }
  }
}

RegDDRef *HIRParser::createRvalDDRef(const Instruction *Inst, unsigned OpNum,
                                     unsigned Level) {
  RegDDRef *Ref;
  auto OpVal = Inst->getOperand(OpNum);
  auto OpTy = OpVal->getType();

  // Parse function pointer rvals as scalars. Pointer arithemetic (GEP) is not
  // expected on them.
  if (OpTy->isPointerTy() && OpTy->getPointerElementType()->isFunctionTy()) {
    Ref = createScalarDDRef(OpVal, Level);

  } else if (auto LInst = dyn_cast<LoadInst>(Inst)) {
    Ref = createGEPDDRef(LInst->getPointerOperand(), Level, true);

    Ref->setVolatile(LInst->isVolatile());
    Ref->setAlignment(LInst->getAlignment());

    parseMetadata(LInst, Ref);

  } else if (isa<GEPOrSubsOperator>(Inst)) {
    Ref = createGEPDDRef(Inst, Level, false);
    Ref->setAddressOf(true);

    parseMetadata(Inst, Ref);

  } else if (OpTy->isPointerTy() && !isa<ConstantPointerNull>(OpVal)) {
    Ref = createGEPDDRef(OpVal, Level, true);
    Ref->setAddressOf(true);

  } else {
    Ref = createScalarDDRef(OpVal, Level);
  }

  return Ref;
}

RegDDRef *HIRParser::createLvalDDRef(HLInst *HInst, unsigned Level) {
  RegDDRef *Ref;
  auto *Inst = HInst->getLLVMInstruction();

  if (auto SInst = dyn_cast<StoreInst>(Inst)) {
    Ref = createGEPDDRef(SInst->getPointerOperand(), Level, true);

    Ref->setVolatile(SInst->isVolatile());
    Ref->setAlignment(SInst->getAlignment());

    parseMetadata(Inst, Ref);

  } else {
    Ref = createScalarDDRef(Inst, Level, HInst);
  }

  return Ref;
}

bool HIRParser::isLiveinCopy(const HLInst *HInst) {
  return HInst->isCopyInst() &&
         SE.getHIRMetadata(HInst->getLLVMInstruction(),
                           ScalarEvolution::HIRLiveKind::LiveIn);
}

bool HIRParser::isLiveoutCopy(const HLInst *HInst) {
  return HInst->isCopyInst() &&
         SE.getHIRMetadata(HInst->getLLVMInstruction(),
                           ScalarEvolution::HIRLiveKind::LiveOut);
}

unsigned HIRParser::getNumRvalOperands(const Instruction *Inst) {
  unsigned NumOp;

  if (isa<GEPOrSubsOperator>(Inst)) {
    // GEP is represented as an assignment of address: %t = &A[i];
    NumOp = 1;
  } else if (auto CInst = dyn_cast<CallInst>(Inst)) {
    // Subtract 1 for the function itself.
    NumOp = CInst->getNumOperands() - 1;
  } else {
    NumOp = Inst->getNumOperands();

    // One of the operands of store is an lval in HIR.
    if (isa<StoreInst>(Inst)) {
      --NumOp;
    }
  }

  return NumOp;
}

FastMathFlags HIRParser::parseFMF(const CmpInst *Cmp) {
  if (auto *FCmp = dyn_cast_or_null<FPMathOperator>(Cmp)) {
    return FCmp->getFastMathFlags();
  }

  return FastMathFlags();
}

bool HIRParser::parsedDebugIntrinsic(const IntrinsicInst *Intrin) {
  if (!RemoveDebugIntrinsics) {
    return false;
  }

  const DbgVariableIntrinsic *DbgIntrin =
      dyn_cast<DbgVariableIntrinsic>(Intrin);

  if (!DbgIntrin) {
    return false;
  }

  Value *Variable = DbgIntrin->getVariableLocation(true);

  // Sometimes DbgIntrin can be @llvm.dbg.value(!{}, ...);
  // !{} is returned as nullptr, we have to ignore such intrinsic.
  if (Variable && isa<Instruction>(Variable)) {
    auto Symbase = getOrAssignSymbase(Variable);
    CurRegion->DbgIntrinMap[Symbase].push_back(DbgIntrin);
  }

  return true;
}

void HIRParser::addFakeRef(HLInst *HInst, const RegDDRef *AddressRef,
                           bool IsRval) {
  auto FakeRef = AddressRef->clone();

  // Reset AddressOf property as we want it to be a memref.
  FakeRef->setAddressOf(false);

  // Set all dimensions of ref to undef to make DD conservative.
  for (unsigned I = 1, NumDims = FakeRef->getNumDimensions(); I <= NumDims;
       ++I) {
    auto CE = FakeRef->getDimensionIndex(I);

    CE->clear();
    CE->setSrcType(CE->getDestType());

    Value *UndefVal = UndefValue::get(CE->getSrcType());
    BlobTy UndefBlob = SE.getUnknown(UndefVal);

    unsigned BlobIndex = findOrInsertBlob(UndefBlob, ConstantSymbase);
    CE->setBlobCoeff(BlobIndex, 1);
  }

  IsRval ? HInst->addFakeRvalDDRef(FakeRef) : HInst->addFakeLvalDDRef(FakeRef);

  // Need to update blobs in Ref since we cleared one of the CEs.
  SmallVector<BlobDDRef *, 1> BlobRefs;
  FakeRef->updateBlobDDRefs(BlobRefs);
  assert(BlobRefs.empty() && "New blobs not expected in fake ref!");

  // Copy ref -> value mapping for symbase assignment.
  GEPRefToPointerMap.insert(std::make_pair(FakeRef, getGEPRefPtr(AddressRef)));
}

static bool isDistributePoint(const IntrinsicInst *Intrin, bool &IsBegin) {
  if (!Intrin->hasOperandBundles()) {
    return false;
  }

  OperandBundleUse BU = Intrin->getOperandBundleAt(0);

  StringRef TagName = BU.getTagName();

  if (TagName.equals("DIR.PRAGMA.DISTRIBUTE_POINT")) {
    IsBegin = true;
    return true;

  } else if (TagName.equals("DIR.PRAGMA.END.DISTRIBUTE_POINT")) {
    IsBegin = false;
    return true;
  }

  return false;
}

bool HIRParser::processedRemovableIntrinsic(HLInst *HInst) {
  auto *Intrin = dyn_cast<IntrinsicInst>(HInst->getLLVMInstruction());

  if (!Intrin) {
    return false;
  }

  if (parsedDebugIntrinsic(Intrin)) {
    HLNodeUtils::erase(HInst);
    return true;
  }

  bool IsBegin;
  if (!isDistributePoint(Intrin, IsBegin)) {
    return false;
  }

  if (IsBegin) {
    // Delay processing after phase2 because at this point we don't know the
    // 'next' node.
    DistributePoints.push_back(HInst);
  } else {
    // Do not need region end directive.
    HLNodeUtils::erase(HInst);
  }

  return true;
}

void HIRParser::parse(HLInst *HInst, bool IsPhase1, unsigned Phase2Level) {
  auto Inst = HInst->getLLVMInstruction();
  unsigned Level;

  setCurNode(HInst);

  if (IsPhase1) {
    Level = CurLevel;

    if (processedRemovableIntrinsic(HInst)) {
      return;
    }

  } else {
    Level = Phase2Level;
    auto OuterLoop = HInst->getOutermostParentLoop();
    CurOutermostLoop = OuterLoop ? OuterLoop->getLLVMLoop() : nullptr;
  }

  if (IsPhase1 && !isEssential(Inst)) {
    // Postpone the processing of this instruction to Phase2.
    auto Symbase = getOrAssignSymbase(Inst);
    UnclassifiedSymbaseInsts[Symbase].push_back(std::make_pair(HInst, Level));
    return;
  }

  bool HasLval = HInst->hasLval();
  unsigned NumRvalOp = getNumRvalOperands(Inst);
  auto *Call = HInst->getCallInst();

  bool FakeDDRefsRequired = (Call && !Call->doesNotAccessMemory());
  bool IsReadOnly =
      (FakeDDRefsRequired && Call->hasFnAttr(Attribute::ReadOnly));

  unsigned NumArgOperands = Call ? Call->getNumArgOperands() : 0;

  // Process rvals
  for (unsigned I = 0; I < NumRvalOp; ++I) {

    if (isa<SelectInst>(Inst) && (I == 0)) {
      HLPredicate Pred;
      RegDDRef *LHSDDRef, *RHSDDRef;

      parseCompare(Inst->getOperand(0), Level, &Pred, &LHSDDRef, &RHSDDRef);

      HInst->setPredicate(Pred);
      HInst->setOperandDDRef(LHSDDRef, 1);
      HInst->setOperandDDRef(RHSDDRef, 2);
      continue;
    }

    RegDDRef *Ref = createRvalDDRef(Inst, I, Level);

    // To translate Instruction's operand number into HLInst's operand number we
    // add one offset each for having an lval and being a select instruction.
    auto OpNum = HasLval ? (isa<SelectInst>(Inst) ? (I + 2) : (I + 1)) : I;

    HInst->setOperandDDRef(Ref, OpNum);
    bool IsBundleOperand = (I >= NumArgOperands);

    if (FakeDDRefsRequired && Ref->isAddressOf() &&
        !Ref->accessesConstantArray() &&
        // Add fake DDRefs for bundle operands.
        (IsBundleOperand || !Call->paramHasAttr(I, Attribute::ReadNone))) {
      addFakeRef(HInst, Ref,
                 (!IsBundleOperand &&
                  (IsReadOnly || Call->paramHasAttr(I, Attribute::ReadOnly))));
    }
  }

  // Process lval
  if (HasLval) {
    HInst->setLvalDDRef(createLvalDDRef(HInst, Level));
  }

  // For indirect calls, set the function pointer as the last operand.
  if (Call && !Call->getCalledFunction()) {
    RegDDRef *Ref = createRvalDDRef(Call, NumRvalOp, Level);
    HInst->setOperandDDRef(Ref, NumRvalOp + HasLval);
  }

  if (auto CInst = dyn_cast<CmpInst>(Inst)) {
    HInst->setPredicate(
        {CInst->getPredicate(), parseFMF(CInst), CInst->getDebugLoc()});
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


  for (auto *Call : DistributePoints) {
    auto *NextNode = Call->getNextNode();
    assert(NextNode &&
           "Could not find next node of distribute point intrinsic!");
    assert(isa<HLDDNode>(NextNode) &&
           "Next node of distribute point intrinsic is not a HLDDNode!");

    cast<HLDDNode>(NextNode)->setDistributePoint(true);

    auto *ParentLoop = NextNode->getParentLoop();
    assert(ParentLoop && "Distribute point does not have parent loop!");

    ParentLoop->setHasDistributePoint(true);
    HLNodeUtils::erase(Call);
  }
}

void HIRParser::clearRegionData() {
  UnclassifiedSymbaseInsts.clear();
  DistributePoints.clear();
  // HIR cache built for one region may not be valid for another.
  SE.clearHIRCache();
}

void HIRParser::run() {
  // We parse one region at a time to preserve CurRegion during phase2.
  for (auto I = HIRF.get().hir_begin(), E = HIRF.get().hir_end(); I != E; ++I) {
    clearRegionData();

    // Start phase 1 of parsing.
    phase1Parse(&*I);

    // Start phase 2 of parsing.
    phase2Parse();
  }

  LF.eraseStoredLoopLabelsAndBottomTests();

  IsReady = true;
}

void HIRParser::parseMetadata(const Instruction *Inst, RegDDRef *Ref) {
  assert(Ref->hasGEPInfo() && "Ref is expected to be gep DDRef");

  const StoreInst *Store = dyn_cast<StoreInst>(Inst);
  const LoadInst *Load = Store ? nullptr : dyn_cast<LoadInst>(Inst);

  if (Store || Load) {
    Inst->getAllMetadataOtherThanDebugLoc(Ref->getGEPInfo()->MDNodes);
    Ref->setMemDebugLoc(Inst->getDebugLoc());

    const Value *Ptr =
        Store ? Store->getPointerOperand() : Load->getPointerOperand();

    if (auto *GepInst = dyn_cast<GetElementPtrInst>(Ptr)) {
      Ref->setGepDebugLoc(GepInst->getDebugLoc());
    } else if (auto *SubsInst = dyn_cast<SubscriptInst>(Ptr)) {
      Ref->setGepDebugLoc(SubsInst->getDebugLoc());
    }

  } else if (isa<GEPOrSubsOperator>(Inst)) {
    Ref->setGepDebugLoc(Inst->getDebugLoc());
  } else {
    llvm_unreachable("Unexpected instruction type.");
  }
}

void HIRParser::parseMetadata(const Value *Val, CanonExpr *CE) {
  if (auto *Inst = dyn_cast<Instruction>(Val)) {
    parseMetadata(Inst, CE);
  }
}

void HIRParser::parseMetadata(const Instruction *Inst, CanonExpr *CE) {
  CE->setDebugLoc(Inst->getDebugLoc());
}

unsigned HIRParser::getPointerDimensionSize(const Value *Ptr) const {
  if (!Ptr->getType()->isPointerTy()) {
    return 0;
  }

  // Trace back as far as possible, until we hit a GEP whose result type is an
  // array type.
  while (Ptr) {
    if (auto Phi = dyn_cast<PHINode>(Ptr)) {
      if (Phi->getNumIncomingValues() == 1) {
        Ptr = Phi->getIncomingValue(0);

      } else if (RI.isHeaderPhi(Phi)) {
        Ptr = getHeaderPhiInitVal(Phi);

      } else {
        // Give up on merge phis.
        return 0;
      }
    } else if (auto GEPOp = dyn_cast<GEPOperator>(Ptr)) {
      if (GEPOp->getNumOperands() == 2) {
        Ptr = GEPOp->getPointerOperand();
      } else {
        auto *ArrTy = dyn_cast<ArrayType>(GEPOp->getSourceElementType());
        return ArrTy ? ArrTy->getArrayNumElements() : 0;
      }
    } else {
      // Give up on other value types.
      return 0;
    }
  }

  return 0;
}

Optional<HIRParser::DelinearizedCoeffBlobIndex>
HIRParser::delinearizeBlobIndex(Type *IndexType, unsigned BlobIndex,
                                SmallVectorImpl<BlobTy> &DimSizes) {

  BlobUtils &BU = getBlobUtils();
  BlobTy Blob = (BlobIndex != InvalidBlobIndex) ? BU.getBlob(BlobIndex)
                                                : SE.getOne(IndexType);

  SmallVector<BlobTy, MaxLoopNestLevel> Subscripts;
  SE.computeAccessFunctions(Blob, Subscripts, DimSizes);

  // Failed to compute access functions.
  if (Subscripts.empty()) {
    return {};
  }

  unsigned LastDim = DimSizes.size();
  assert(Subscripts.size() <= LastDim &&
      "Subscripts size do not match generated ref");

  for (unsigned DimI = 0, DimE = Subscripts.size(); DimI < DimE; ++DimI) {
    int64_t NewCoeff = 0;
    unsigned NewBlobIndex = InvalidBlobIndex;
    const SCEV *Subscript = Subscripts[DimI];

    if (auto *ConstSubscript = dyn_cast<SCEVConstant>(Subscript)) {
      NewCoeff = getSCEVConstantValue(ConstSubscript);
      // Skip empty terms
      if (NewCoeff == 0) {
        continue;
      }

    } else {
      BlobTy NewBlob;
      breakConstantMultiplierBlob(Subscript, &NewCoeff, &NewBlob);
      NewBlobIndex = BU.findOrInsertBlob(NewBlob);
    }

    return HIRParser::DelinearizedCoeffBlobIndex(LastDim - DimI, NewCoeff,
                                                 NewBlobIndex);
  }

  return HIRParser::DelinearizedCoeffBlobIndex();
}

RegDDRef *HIRParser::delinearizeSingleRef(const RegDDRef *Ref,
                                          SmallVectorImpl<BlobTy> &Strides,
                                          SmallVectorImpl<BlobTy> &DimSizes) {
  BlobUtils &BU = getBlobUtils();
  DDRefUtils &DRU = getDDRefUtils();
  CanonExprUtils &CEU = getCanonExprUtils();

  const CanonExpr *LinearIndexCE = Ref->getSingleCanonExpr();

  RegDDRef *NewRef = DRU.createMemRef(Ref->getBasePtrBlobIndex(),
                                      Ref->getBaseCE()->getDefinedAtLevel(),
                                      Ref->getSymbase());

  Type *IndexType = LinearIndexCE->getSrcType();

  // Add dimensions using Strides.
  for (BlobTy Stride : Strides) {
    CanonExpr *CE = CEU.createCanonExpr(LinearIndexCE->getDestType());
    CE->setSrcType(IndexType);

    CanonExpr *StrideCE = CEU.createCanonExpr(LinearIndexCE->getDestType());
    if (Stride != nullptr) {
      StrideCE->setBlobCoeff(BU.findOrInsertBlob(Stride), 1);
    } else {
      StrideCE->setConstant(1);
    }

    Type *DimType = Ref->getBaseCE()->getDestType();
    NewRef->addDimension(CE, {}, CE->clone(), StrideCE, DimType);
  }

  // Ref may have innermost struct offset.
  NewRef->setTrailingStructOffsets(1, Ref->getTrailingStructOffsets(1));

  // Handle IV part
  unsigned IVLevel = 0;
  for (auto &IV :
      make_range(LinearIndexCE->iv_begin(), LinearIndexCE->iv_end())) {
    ++IVLevel;
    if (IV.Coeff == 0) {
      continue;
    }

    auto NewIndex = delinearizeBlobIndex(IndexType, IV.Index, DimSizes);
    if (!NewIndex) {
      return nullptr;
    }

    // Skip indices with no impact.
    if (NewIndex->Coeff == 0) {
      continue;
    }

    auto *Index = NewRef->getDimensionIndex(NewIndex->DimIndex);
    Index->addIV(IVLevel, NewIndex->BlobIndex, NewIndex->Coeff * IV.Coeff);
  }

  // Handle blob part
  for (auto &BlobCoeff :
      make_range(LinearIndexCE->blob_begin(), LinearIndexCE->blob_end())) {

    auto NewIndex = delinearizeBlobIndex(IndexType, BlobCoeff.Index, DimSizes);
    if (!NewIndex) {
      return nullptr;
    }

    // Skip indices with no impact.
    if (NewIndex->Coeff == 0) {
      continue;
    }

    auto *Index = NewRef->getDimensionIndex(NewIndex->DimIndex);
    if (NewIndex->BlobIndex != InvalidBlobIndex) {
      Index->addBlob(NewIndex->BlobIndex, NewIndex->Coeff * BlobCoeff.Coeff);
    } else {
      Index->addConstant(NewIndex->Coeff * BlobCoeff.Coeff, false);
    }
  }

  // Handle c0 part
  NewRef->getDimensionIndex(1)->addConstant(LinearIndexCE->getConstant(),
                                            false);

  return NewRef;
}

bool HIRParser::delinearizeRefs(ArrayRef<const loopopt::RegDDRef *> GepRefs,
                                SmallVectorImpl<loopopt::RegDDRef *> &OutRefs,
                                SmallVectorImpl<BlobTy> *DimSizes) {
  LLVM_DEBUG(dbgs() << "Refs delinearization\n");

  BlobUtils &BU = getBlobUtils();

  SmallVector<BlobTy, MaxLoopNestLevel> Strides;
  SmallVector<BlobTy, MaxLoopNestLevel> Sizes;

  // Collect all IV strides from refs.
  for (auto &Ref : GepRefs) {
    assert(!Ref->isTerminalRef() && "Expected non-terminal refs only");
    assert(Ref->getNumDimensions() == 1 && "Expected single dimension refs");

    Type *DimType = Ref->getBaseCE()->getDestType();
    if (DimType->getPointerElementType()->isAggregateType() ||
        Ref->getDimensionConstLower(1) != 0 ||
        Ref->getDimensionConstStride(1) == 0) {
      return false;
    }

    const CanonExpr *LinearIndexCE = Ref->getSingleCanonExpr();
    if (LinearIndexCE->getDenominator() != 1 ||
        LinearIndexCE->getSrcType() != LinearIndexCE->getDestType()) {
      return false;
    }

    for (auto IV :
         make_range(LinearIndexCE->iv_begin(), LinearIndexCE->iv_end())) {
      if (IV.Coeff == 0 || IV.Index == InvalidBlobIndex) {
        continue;
      }

      Strides.push_back(BU.getBlob(IV.Index));
    }
  }

  // No candidates for delinearization.
  if (Strides.size() < 2) {
    return false;
  }

  // Sort and uniq Strides and populate Sizes.
  SE.findArrayDimensions(Strides, Sizes, SE.getOne(Strides.front()->getType()));

  LLVM_DEBUG(dbgs() << "Strides:\n");
  LLVM_DEBUG(for (auto &Blob : Strides) { Blob->dump(); });

  if (Sizes.empty()) {
    LLVM_DEBUG(dbgs() << "Could not compute dimension sizes\n");
    return false;
  }

  LLVM_DEBUG(dbgs() << "Sizes:\n");
  LLVM_DEBUG(for (auto &Blob : Sizes) { Blob->dump(); });

  // Push nullptr to indicate innermost dimension.
  Strides.push_back(nullptr);
  assert(Strides.size() == Sizes.size() &&
         "Strides and Sizes should be equal in size");

  // Use pool for new refs to remove them if it's impossible to delinearize all
  // of them.
  SmallVector<std::unique_ptr<RegDDRef>, 8> NewRefs;
  NewRefs.reserve(GepRefs.size());

  // Create new multi-dimension refs.
  for (auto &Ref : GepRefs) {
    auto *NewRef = delinearizeSingleRef(Ref, Strides, Sizes);
    if (!NewRef) {
      return false;
    }

    NewRef->makeConsistent(GepRefs, Ref->getNodeLevel());
    NewRefs.emplace_back(NewRef);
  }

  LLVM_DEBUG(dbgs() << "Delinearized refs:\n");
  LLVM_DEBUG(for (auto Pair : zip(GepRefs, NewRefs)) {
    std::get<0>(Pair)->dump();
    dbgs() << " -> ";
    std::get<1>(Pair)->dump();
    dbgs() << "\n";
  });

  // Release refs to OutRefs.
  OutRefs.resize(NewRefs.size());
  std::transform(NewRefs.begin(), NewRefs.end(), OutRefs.begin(),
                 [](std::unique_ptr<RegDDRef> &Ref) { return Ref.release(); });

  if (DimSizes) {
    DimSizes->swap(Sizes);
  }

  return true;
}