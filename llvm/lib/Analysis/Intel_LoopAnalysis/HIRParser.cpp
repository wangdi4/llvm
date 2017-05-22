//===----- HIRParser.cpp - Parses SCEVs into CanonExprs -------------------===//
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
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Operator.h"

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRCreation.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRLoopFormation.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRParser.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRRegionIdentification.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRScalarSymbaseAssignment.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Passes.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "hir-parser"

INITIALIZE_PASS_BEGIN(HIRParser, "hir-parser", "HIR Parser", false, true)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolutionWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRRegionIdentification)
INITIALIZE_PASS_DEPENDENCY(HIRCreation)
INITIALIZE_PASS_DEPENDENCY(HIRLoopFormation)
INITIALIZE_PASS_DEPENDENCY(HIRScalarSymbaseAssignment)
INITIALIZE_PASS_END(HIRParser, "hir-parser", "HIR Parser", false, true)

static cl::opt<bool> RemoveDebugIntrinsics(
    DEBUG_TYPE "-remove-dbg-intrin", cl::init(true), cl::Hidden,
    cl::desc("Remove llvm.dbg.* intrinsics from HIR (default: true)"));

char HIRParser::ID = 0;

FunctionPass *llvm::createHIRParserPass() { return new HIRParser(); }

HIRParser::HIRParser()
    : FunctionPass(ID), DDRU(*this), CurNode(nullptr), CurRegion(nullptr),
      CurOutermostLoop(nullptr), CurLevel(0) {
  initializeHIRParserPass(*PassRegistry::getPassRegistry());
}

void HIRParser::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequiredTransitive<DominatorTreeWrapperPass>();
  AU.addRequiredTransitive<LoopInfoWrapperPass>();
  AU.addRequiredTransitive<ScalarEvolutionWrapperPass>();
  AU.addRequiredTransitive<HIRRegionIdentification>();
  AU.addRequiredTransitive<HIRCreation>();
  AU.addRequiredTransitive<HIRLoopFormation>();
  AU.addRequiredTransitive<HIRScalarSymbaseAssignment>();
}

HLNodeUtils &HIRParser::getHLNodeUtils() { return HIR->getHLNodeUtils(); }

HLContainerTy::iterator HIRParser::hir_begin() { return HIR->begin(); }
HLContainerTy::const_iterator HIRParser::hir_cbegin() const {
  return HIR->begin();
}
HLContainerTy::iterator HIRParser::hir_end() { return HIR->end(); }
HLContainerTy::const_iterator HIRParser::hir_cend() const { return HIR->end(); }

HLContainerTy::reverse_iterator HIRParser::hir_rbegin() {
  return HIR->rbegin();
}
HLContainerTy::const_reverse_iterator HIRParser::hir_crbegin() const {
  return HIR->rbegin();
}
HLContainerTy::reverse_iterator HIRParser::hir_rend() { return HIR->rend(); }
HLContainerTy::const_reverse_iterator HIRParser::hir_crend() const {
  return HIR->rend();
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

      return BlobIndex;
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

    return Index;
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
  assert((Symbase < getMaxScalarSymbase()) &&
         "Blob index for symbase not found!");

  auto Val = ScalarSA->getBaseScalar(Symbase);
  auto Blob = SE->getUnknown(const_cast<Value *>(Val));

  return findOrInsertBlob(Blob, Symbase);
}

unsigned HIRParser::findOrInsertBlob(BlobTy Blob, unsigned Symbase) {
  return findOrInsertBlobImpl(Blob, Symbase, true, false);
}

unsigned HIRParser::updateBlob(BlobTy OldBlob, BlobTy NewBlob,
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

  assert(Symbase != InvalidSymbase && "Blob is not a temp!");
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
    unsigned BlobIndex = findOrInsertBlob(Blob, Symbase);

    if (NewBlobIndex) {
      *NewBlobIndex = BlobIndex;
    }
  }
}

BlobTy HIRParser::createBlob(Value *Val, unsigned Symbase, bool Insert,
                             unsigned *NewBlobIndex) {
  assert(Val && "Value cannot be null!");

  auto Blob = SE->getUnknown(Val);

  insertBlobHelper(Blob, Symbase, Insert, NewBlobIndex);

  return Blob;
}

BlobTy HIRParser::createBlob(int64_t Val, Type *Ty, bool Insert,
                             unsigned *NewBlobIndex) {

  assert(Ty && "Type cannot be null!");
  auto Blob = SE->getConstant(Ty, Val, false);

  insertBlobHelper(Blob, InvalidSymbase, Insert, NewBlobIndex);

  return Blob;
}

BlobTy HIRParser::createAddBlob(BlobTy LHS, BlobTy RHS, bool Insert,
                                unsigned *NewBlobIndex) {
  assert(LHS && RHS && "Blob cannot be null!");

  auto Blob = SE->getAddExpr(LHS, RHS);

  insertBlobHelper(Blob, InvalidSymbase, Insert, NewBlobIndex);

  return Blob;
}

BlobTy HIRParser::createMinusBlob(BlobTy LHS, BlobTy RHS, bool Insert,
                                  unsigned *NewBlobIndex) {
  assert(LHS && RHS && "Blob cannot be null!");

  auto Blob = SE->getMinusSCEV(LHS, RHS);

  insertBlobHelper(Blob, InvalidSymbase, Insert, NewBlobIndex);

  return Blob;
}

BlobTy HIRParser::createMulBlob(BlobTy LHS, BlobTy RHS, bool Insert,
                                unsigned *NewBlobIndex) {
  assert(LHS && RHS && "Blob cannot be null!");

  auto Blob = SE->getMulExpr(LHS, RHS);

  insertBlobHelper(Blob, InvalidSymbase, Insert, NewBlobIndex);

  return Blob;
}

BlobTy HIRParser::createUDivBlob(BlobTy LHS, BlobTy RHS, bool Insert,
                                 unsigned *NewBlobIndex) {
  assert(LHS && RHS && "Blob cannot be null!");

  auto Blob = SE->getUDivExpr(LHS, RHS);

  insertBlobHelper(Blob, InvalidSymbase, Insert, NewBlobIndex);

  return Blob;
}

BlobTy HIRParser::createTruncateBlob(BlobTy Blob, Type *Ty, bool Insert,
                                     unsigned *NewBlobIndex) {
  assert(Blob && "Blob cannot be null!");
  assert(Ty && "Type cannot be null!");

  auto NewBlob = SE->getTruncateExpr(Blob, Ty);

  insertBlobHelper(NewBlob, InvalidSymbase, Insert, NewBlobIndex);

  return NewBlob;
}

BlobTy HIRParser::createZeroExtendBlob(BlobTy Blob, Type *Ty, bool Insert,
                                       unsigned *NewBlobIndex) {
  assert(Blob && "Blob cannot be null!");
  assert(Ty && "Type cannot be null!");

  auto NewBlob = SE->getZeroExtendExpr(Blob, Ty);

  insertBlobHelper(NewBlob, InvalidSymbase, Insert, NewBlobIndex);

  return NewBlob;
}

BlobTy HIRParser::createSignExtendBlob(BlobTy Blob, Type *Ty, bool Insert,
                                       unsigned *NewBlobIndex) {
  assert(Blob && "Blob cannot be null!");
  assert(Ty && "Type cannot be null!");

  auto NewBlob = SE->getSignExtendExpr(Blob, Ty);

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
      Blob->getType()->getPrimitiveSizeInBits()) {
    NewBlob = IsSExt ? SE->getSignExtendExpr(Blob, Ty)
                     : SE->getZeroExtendExpr(Blob, Ty);
  } else {
    NewBlob = SE->getTruncateExpr(Blob, Ty);
  }

  insertBlobHelper(NewBlob, InvalidSymbase, Insert, NewBlobIndex);

  return NewBlob;
}

BlobTy HIRParser::createSMinBlob(BlobTy BlobA, BlobTy BlobB, bool Insert,
                                 unsigned *NewBlobIndex) {
  assert(BlobA && BlobB && "Blob cannot be null!");

  auto NewBlob = SE->getSMinExpr(BlobA, BlobB);

  insertBlobHelper(NewBlob, InvalidSymbase, Insert, NewBlobIndex);

  return NewBlob;
}

BlobTy HIRParser::createSMaxBlob(BlobTy BlobA, BlobTy BlobB, bool Insert,
                                 unsigned *NewBlobIndex) {
  assert(BlobA && BlobB && "Blob cannot be null!");

  auto NewBlob = SE->getSMaxExpr(BlobA, BlobB);

  insertBlobHelper(NewBlob, InvalidSymbase, Insert, NewBlobIndex);

  return NewBlob;
}

BlobTy HIRParser::createUMinBlob(BlobTy BlobA, BlobTy BlobB, bool Insert,
                                 unsigned *NewBlobIndex) {
  assert(BlobA && BlobB && "Blob cannot be null!");

  auto NewBlob = SE->getUMinExpr(BlobA, BlobB);

  insertBlobHelper(NewBlob, InvalidSymbase, Insert, NewBlobIndex);

  return NewBlob;
}

BlobTy HIRParser::createUMaxBlob(BlobTy BlobA, BlobTy BlobB, bool Insert,
                                 unsigned *NewBlobIndex) {
  assert(BlobA && BlobB && "Blob cannot be null!");

  auto NewBlob = SE->getUMaxExpr(BlobA, BlobB);

  insertBlobHelper(NewBlob, InvalidSymbase, Insert, NewBlobIndex);

  return NewBlob;
}

bool HIRParser::contains(BlobTy Blob, BlobTy SubBlob) const {
  assert(Blob && "Blob cannot be null!");
  assert(SubBlob && "SubBlob cannot be null!");

  return SE->hasOperand(Blob, SubBlob);
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

  auto NewBlob = SCEVParameterRewriter::rewrite(Blob, *SE, Map, true);

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
  BlobTy ConstantBlob = SE->getConstant(TempBlob->getType(), Constant, true);
  return replaceTempBlob(BlobIndex, TempIndex, ConstantBlob, NewBlobIndex,
                         SimplifiedConstant);
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

bool HIRParser::isMinMaxWithAddRecOperand(const SCEV *SC) const {
  // Min is represented using !(Max) ==> (-1 -Max) so we call getNotSCEV() to
  // undo the original 'not' operation.
  if (isa<SCEVAddExpr>(SC)) {
    SC = SE->getNotSCEV(SC);
  }

  if (!isa<SCEVSMaxExpr>(SC) && !isa<SCEVUMaxExpr>(SC)) {
    return false;
  }

  for (const auto *Op : cast<SCEVNAryExpr>(SC)->operands()) {
    if (isa<SCEVAddRecExpr>(Op)) {
      return true;
    }
  }

  return false;
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
      : SCEVRewriteVisitor(*(HIRPar->SE)), HIRP(HIRPar), CE(CE),
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
  const SCEV *visitUMaxExpr(const SCEVUMaxExpr *Max);

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

  /// Returns constant multiplier which when applied to AddRec yields MulAddRec,
  /// if possible, otherwise returns nullptr.
  const SCEVConstant *getMultiplier(const SCEVAddRecExpr *AddRec,
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
  if (!HIRP->isMinMaxWithAddRecOperand(MinMax)) {
    return nullptr;
  }

  if (auto SubSCEV = getSubstituteSCEV(MinMax)) {
    return SubSCEV;
  }

  return nullptr;
}

const SCEV *HIRParser::BlobProcessor::visitAddExpr(const SCEVAddExpr *Add) {
  // This mapping recovers original (select) instruction from min exprs with
  // AddRec operands. This is more profitable as it avoids creation of IV blobs.
  const SCEV *MappedSC = nullptr;

  // This mapping is for profitability (not legality) so we can skip it in safe
  // mode.
  if (!SafeMode && (MappedSC = getProfitableMinMaxExprMapping(Add))) {
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
  const SCEV *MappedSC = nullptr;

  // This mapping is for profitability (not legality) so we can skip it in safe
  // mode.
  if (!SafeMode && (MappedSC = getProfitableMinMaxExprMapping(Max))) {
    return MappedSC;
  }

  return SCEVRewriteVisitor<BlobProcessor>::visitSMaxExpr(Max);
}

const SCEV *HIRParser::BlobProcessor::visitUMaxExpr(const SCEVUMaxExpr *Max) {
  // This mapping recovers original (select) instruction from max exprs with
  // AddRec operands. This is more profitable as it avoids creation of IV blobs.
  const SCEV *MappedSC = nullptr;

  // This mapping is for profitability (not legality) so we can skip it in safe
  // mode.
  if (!SafeMode && (MappedSC = getProfitableMinMaxExprMapping(Max))) {
    return MappedSC;
  }

  return SCEVRewriteVisitor<BlobProcessor>::visitUMaxExpr(Max);
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

  auto NewSCEV = HIRP->SE->getUnknown(const_cast<Instruction *>(OrigInst));

  // NOTE: The order of truncation, negation, multiplication and addition
  // matters.
  if (IsTruncOrSExt) {
    if (NewSCEV->getType()->getPrimitiveSizeInBits() <
        SC->getType()->getPrimitiveSizeInBits()) {
      NewSCEV = HIRP->SE->getSignExtendExpr(NewSCEV, SC->getType());
    } else {
      NewSCEV = HIRP->SE->getTruncateExpr(NewSCEV, SC->getType());
    }

  } else if (IsZExt) {
    NewSCEV = HIRP->SE->getZeroExtendExpr(NewSCEV, SC->getType());
  }

  if (IsNegation) {
    NewSCEV = HIRP->SE->getNegativeSCEV(NewSCEV);
  }

  if (ConstMultiplier) {
    NewSCEV = HIRP->SE->getMulExpr(ConstMultiplier, NewSCEV);
  }

  if (Additive) {
    NewSCEV = HIRP->SE->getAddExpr(Additive, NewSCEV);
  }

  // Convert value to base value before returning.
  return visit(NewSCEV);
}

const Instruction *
HIRParser::BlobProcessor::searchSCEVValues(const SCEV *SC) const {
  auto ValSet = HIRP->SE->getSCEVValues(SC);

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

    if (!HIRP->SE->getHIRMetadata(Inst, ScalarEvolution::HIRLiveKind::LiveIn) &&
        HIRP->DT->dominates(Inst, CurInst)) {
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
        HIRP->SE->getHIRMetadata(CurInst, ScalarEvolution::HIRLiveKind::LiveIn);
    FirstInst = true;

    // We just started the traceback, clear previous entries.
    VisitedInsts.clear();
  }

  // We should not be checking the SCEV of the livein copy instruction as it
  // should inherit the SCEV of the rval.
  if (!IsLiveInCopy && HIRP->SE->isSCEVable(CurInst->getType())) {
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
    } else if (HIRP->DT->dominates(CurInst, HIRP->getCurInst()) &&
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
    // instruction types in SE->createSCEV().
    if (!isa<BinaryOperator>(OpInst) && !isa<CastInst>(OpInst) &&
        !isa<GetElementPtrInst>(OpInst) && !isa<PHINode>(OpInst) &&
        !isa<SelectInst>(OpInst)) {
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
          HIRP->SE->getSignExtendExpr(NewAddRec, OrigType));

      if (!ExtAddRec) {
        ExtAddRec = dyn_cast<SCEVAddRecExpr>(
            HIRP->SE->getZeroExtendExpr(NewAddRec, OrigType));
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
          HIRP->SE->getTruncateExpr(NewAddRec, OrigType));

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
  NewAddRec = cast<SCEVAddRecExpr>(HIRP->SE->getNegativeSCEV(NewAddRec));

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
HIRParser::BlobProcessor::getMultiplier(const SCEVAddRecExpr *AddRec,
                                        const SCEVAddRecExpr *MulAddRec) const {

  const SCEVConstant *Mul = nullptr;

  auto Op = dyn_cast<SCEVConstant>(AddRec->getOperand(1));
  auto MulOp = dyn_cast<SCEVConstant>(MulAddRec->getOperand(1));

  if (Op && MulOp) {
    Mul = cast<SCEVConstant>(HIRP->SE->getConstant(cast<ConstantInt>(
        ConstantExpr::getSDiv(MulOp->getValue(), Op->getValue()))));

    if (Mul->isZero()) {
      return nullptr;
    }

    return Mul;
  }

  // Looking for this condition-
  // AddRec: {0,+,%size_x}<%for.cond.47.preheader>
  // MulAddRec: {0,+,(4 * %size_x)}<%for.cond.47.preheader>
  auto MulSCEV = dyn_cast<SCEVMulExpr>(MulAddRec->getOperand(1));

  if (!MulSCEV || (MulSCEV->getNumOperands() != 2) ||
      !isa<SCEVConstant>(MulSCEV->getOperand(0)) ||
      (MulSCEV->getOperand(1) != AddRec->getOperand(1))) {
    return nullptr;
  }

  Mul = cast<SCEVConstant>(MulSCEV->getOperand(0));

  return Mul;
}

bool HIRParser::BlobProcessor::isReplacableAddRec(
    const SCEVAddRecExpr *OrigAddRec, const SCEVAddRecExpr *NewAddRec,
    SCEV::NoWrapFlags WrapFlags, SCEVConstant **ConstMultiplier,
    SCEV **Additive) const {

  const SCEVConstant *Mul = nullptr;
  const SCEV *Add = nullptr;

  // Get constant multiplier, if any.
  if (NewAddRec->getOperand(1) != OrigAddRec->getOperand(1)) {

    if (!ConstMultiplier) {
      return false;
    }

    Mul = getMultiplier(NewAddRec, OrigAddRec);

    if (!Mul) {
      return false;
    }

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

  if (Symbase < getMaxScalarSymbase()) {
    ScalarSA->getBaseScalar(Symbase)->printAsOperand(OS, false);
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
  auto Symbase = ScalarSA->getScalarSymbase(Inst, CurRegion->getIRRegion());

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

const SCEV *HIRParser::getSCEV(Value *Val) const {
  return SE->getSCEVForHIR(Val, CurOutermostLoop);
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
    Symbase = ScalarSA->getScalarSymbase(Temp, CurRegion->getIRRegion());

    assert((Symbase != InvalidSymbase) &&
           "Temp blob hasn't been assigned a symbase!");
  }
#endif

  return findOrInsertBlob(Blob, Symbase);
}

unsigned HIRParser::getOrAssignSymbase(const Value *Temp, unsigned *BlobIndex) {
  const Value *OldTemp = nullptr;

  auto Symbase = ScalarSA->getOrAssignScalarSymbase(
      Temp, CurRegion->getIRRegion(), &OldTemp);

  if (!OldTemp) {
    return Symbase;
  }

  auto OldTempBlob = SE->getUnknown(const_cast<Value *>(OldTemp));
  auto BaseTemp = ScalarSA->getBaseScalar(Symbase);
  auto NewTempBlob = SE->getUnknown(const_cast<Value *>(BaseTemp));
  auto Index = updateBlob(OldTempBlob, NewTempBlob, Symbase);

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
  Loop *DefLp = LI->getLoopFor(Inst->getParent());

  HLLoop *DefLoop = DefLp ? LF->findHLLoop(DefLp) : nullptr;

  HLLoop *UseLoop = isa<HLLoop>(CurNode) ? cast<HLLoop>(CurNode)
                                         : CurNode->getLexicalParentLoop();

  // Set region livein and def level.
  if (!CurRegion->containsBBlock(ParentBB)) {
    CurRegion->addLiveInTemp(Symbase, Inst);
    IsRegionLivein = true;

    // If Inst is defined in another region, we need to add it as region
    // liveout.
    if (DefLoop) {
      DefLoop->getParentRegion()->addLiveOutTemp(Symbase, Inst);
    }

  } else if (DefLoop && UseLoop &&
             (LCALoop =
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

  // For livein/liveout analysis we need to set defining loop based on BaseInst
  // as it represents Inst in HIR.
  if (!IsRegionLivein && (Inst != BaseInst)) {
    DefLp = LI->getLoopFor(BaseInst->getParent());
    assert(DefLp && "Defining loop of BaseInst is null!");
    DefLoop = LF->findHLLoop(DefLp);
    assert(DefLoop && "Defining HLLoop of BaseInst is null!");

    if (UseLoop) {
      LCALoop = HLNodeUtils::getLowestCommonAncestorLoop(UseLoop, DefLoop);
    }
  }

  // Add temp as livein into current and all parent loops till we reach LCA
  // loop.
  while (UseLoop && (UseLoop != LCALoop)) {
    UseLoop->addLiveInTemp(Symbase);
    UseLoop = UseLoop->getParentLoop();
  }

  if (DefLoop) {
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
    if (isa<PHINode>(BaseInst) &&
        (BaseInst->getParent() == DefLp->getHeader())) {
      DefLoop->addLiveInTemp(Symbase);
    }

    // Instructions with livein metadata are deconstructed definitions (not
    // uses). Therefore, they should not be used to mark loop liveouts.
    if (!SE->getHIRMetadata(Inst, ScalarEvolution::HIRLiveKind::LiveIn)) {
      while (DefLoop != LCALoop) {
        DefLoop->addLiveOutTemp(Symbase);
        DefLoop = DefLoop->getParentLoop();
      }
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
  auto BaseTemp = ScalarSA->getBaseScalar(Symbase);

  auto BaseTempBlob =
      (Temp == BaseTemp)
          ? TempBlob
          : cast<SCEVUnknown>(SE->getUnknown(const_cast<Value *>(BaseTemp)));

  // Insert blob, if Index was not provided by getOrAssignSymbase().
  if (Index == InvalidBlobIndex) {
    Index = findOrInsertBlob(BaseTempBlob, Symbase);
  }

  if (auto Inst = dyn_cast<Instruction>(Temp)) {
    DefLevel = processInstBlob(Inst, cast<Instruction>(BaseTemp), Symbase);
  } else {
    // Blob is some global value. Global values are not marked livein.
  }

  setCanonExprDefLevel(CE, NestingLevel, DefLevel);

  // RegDDRef.
  cacheTempBlobLevel(Index, NestingLevel, DefLevel);

  // Add blob symbase as required.
  RequiredSymbases.insert(Symbase);

  // Return base temp.
  return BaseTempBlob;
}

void HIRParser::breakConstantMultiplierBlob(BlobTy Blob, int64_t *Multiplier,
                                            BlobTy *NewBlob) {

  if (auto MulSCEV = dyn_cast<SCEVMulExpr>(Blob)) {

    if (auto ConstSCEV = dyn_cast<SCEVConstant>(MulSCEV->getOperand(0))) {
      SmallVector<const SCEV *, 4> Ops;

      for(auto I = MulSCEV->op_begin()+1, E = MulSCEV->op_end(); I != E; ++I) {
        Ops.push_back(*I);
      }

      *Multiplier = getSCEVConstantValue(ConstSCEV);
      *NewBlob = SE->getMulExpr(Ops, MulSCEV->getNoWrapFlags());
      return;
    }
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
    CurLp = HIRP.LI->getLoopFor(CurInst->getParent());
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

  auto NewSC = SE->getSCEVAtScopeForHIR(SC, ParLoop, CurOutermostLoop);

  return isValidScopeSCEV(NewSC) ? NewSC : SC;
}

bool HIRParser::parseAddRec(const SCEVAddRecExpr *RecSCEV, CanonExpr *CE,
                            unsigned Level, bool UnderCast,
                            bool IndicateFailure) {
  auto Lp = RecSCEV->getLoop();
  auto HLoop = LF->findHLLoop(Lp);

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

    if (!parseRecursive(BaseSCEV, CE, Level, false, UnderCast,
                        IndicateFailure)) {
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

    if (IsTop && !UnderCast) {
      CE->setSrcType(CastSCEV->getOperand()->getType());
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

  } else if (isa<SCEVMulExpr>(SC)) {
    return parseBlob(SC, CE, Level, 0, IndicateFailure);

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
    return parseAddRec(RecSCEV, CE, Level, UnderCast, IndicateFailure);

  } else if (isa<SCEVSMaxExpr>(SC) || isa<SCEVUMaxExpr>(SC)) {
    // TODO: extend DDRef representation to handle min/max.
    return parseBlob(SC, CE, Level, 0, IndicateFailure);
  }

  llvm_unreachable("Unexpected SCEV type!");
}

CanonExpr *HIRParser::parseAsBlob(const Value *Val, unsigned Level) {
  CanonExpr *CE = getCanonExprUtils().createCanonExpr(Val->getType());
  auto BlobSCEV = SE->getUnknown(const_cast<Value *>(Val));

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

bool HIRParser::containsCastedAddRec(const CastInst *CI) const {
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

  auto SC = getSCEV(const_cast<CastInst *>(CI));

  CastedAddRecChecker CARC(CI->getSrcTy());
  SCEVTraversal<CastedAddRecChecker> Checker(CARC);
  Checker.visitAll(SC);

  return CARC.found();
}

bool HIRParser::isCastedFromLoopIVType(const CastInst *CI) const {
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

  if (isCastedFromLoopIVType(CI) || containsCastedAddRec(CI)) {
    return true;
  }

  return false;
}

CanonExpr *HIRParser::parse(const Value *Val, unsigned Level, bool IsTop) {
  CanonExpr *CE = nullptr;
  const Value *OrigVal = Val;

  // Parse as blob if the type is not SCEVable.
  // This is currently for handling floating types.
  if (!SE->isSCEVable(Val->getType())) {
    CE = parseAsBlob(Val, Level);

  } else if (Val->getType()->isPointerTy()) {
    if (isa<ConstantPointerNull>(Val)) {
      // Create null CE to represent a null pointer.
      CE = getCanonExprUtils().createCanonExpr(Val->getType());
    } else {
      // Force pointer values to be parsed as blobs. This is for handling lvals
      // but pointer blobs can occur in loop upper as well. CG will have to do
      // special processing for pointers contained in upper.
      CE = parseAsBlob(Val, Level);
    }

  } else {

    bool EnableCastHiding = IsTop;
    const CastInst *CI = dyn_cast<CastInst>(Val);

    if (shouldParseWithoutCast(CI, IsTop)) {
      EnableCastHiding = false;
      Val = CI->getOperand(0);

      CE = getCanonExprUtils().createExtCanonExpr(
          CI->getSrcTy(), CI->getDestTy(), isa<SExtInst>(CI));

    } else {
      CE = getCanonExprUtils().createCanonExpr(Val->getType());
    }

    auto SC = getSCEV(const_cast<Value *>(Val));

    if (parseRecursive(SC, CE, Level, IsTop, !EnableCastHiding, true)) {
      parseMetadata(OrigVal, CE);
    } else {
      getCanonExprUtils().destroy(CE);
      CE = parseAsBlob(OrigVal, Level);
    }
  }

  assert((CE->getDestType() == OrigVal->getType()) &&
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

RegDDRef *HIRParser::createUpperDDRef(const SCEV *BETC, unsigned Level,
                                      Type *IVType) {
  const Value *Val;
  unsigned Symbase = 0;
  clearTempBlobLevelMap();

  if (auto ConstSCEV = dyn_cast<SCEVConstant>(BETC)) {
    Val = ConstSCEV->getValue();
    Symbase = getOrAssignSymbase(Val);
  } else if (auto UnknownSCEV = dyn_cast<SCEVUnknown>(BETC)) {
    Val = UnknownSCEV->getValue();
    Symbase = getOrAssignSymbase(Val);
  } else {
    Symbase = GenericRvalSymbase;
  }

  auto Ref = getDDRefUtils().createRegDDRef(Symbase);
  auto CE = getCanonExprUtils().createCanonExpr(IVType);
  auto BETCType = BETC->getType();

  assert((!BETCType->isPointerTy() ||
          (getDataLayout().getTypeSizeInBits(BETCType) ==
           IVType->getPrimitiveSizeInBits())) &&
         "Loop with pointer type BETC does not have pointer sized IV!");

  // If there is a type mismatch, make upper the same type as IVType.
  if (!BETCType->isPointerTy() && (BETCType != IVType)) {

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

  return Ref;
}

void HIRParser::parse(HLRegion *Reg) {
  CurRegion = Reg;
  // HIR cache built for another region may not be valid for this one.
  SE->clearHIRCache();
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

  if (SE->hasLoopInvariantBackedgeTakenCount(Lp)) {
    auto BETC = SE->getBackedgeTakenCountForHIR(Lp, CurOutermostLoop);

    // Initialize Lower to 0.
    auto LowerRef = createLowerDDRef(IVType);
    HLoop->setLowerDDRef(LowerRef);

    // Initialize Stride to 1.
    auto StrideRef = createStrideDDRef(IVType);
    HLoop->setStrideDDRef(StrideRef);

    // Set the upper bound
    auto UpperRef = createUpperDDRef(BETC, CurLevel, IVType);
    HLoop->setUpperDDRef(UpperRef);

    unsigned MaxTC;

    // Set small max trip count if available from scalar evolution.
    if (!UpperRef->isIntConstant() &&
        (MaxTC = SE->getSmallConstantMaxTripCount(const_cast<Loop *>(Lp)))) {
      HLoop->setMaxTripCountEstimate(MaxTC);
    }

  } else {
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

  if (auto CInst = dyn_cast<CmpInst>(Cond)) {

    // Suppress traceback if CInst's operand's type is not supported.
    if (RI->isSupported(CInst->getOperand(0)->getType()) &&
        RI->isSupported(CInst->getOperand(1)->getType())) {

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
        RI->isSupported(Op1->getType()) && RI->isSupported(Op2->getType())) {
      parseCompare(Op1, Level, Preds, Refs, true);
      parseCompare(Op2, Level, Preds, Refs, true);
      return;
    }
  }

  if (isa<UndefValue>(Cond)) {
    Preds.push_back(UNDEFINED_PREDICATE);
    Refs.push_back(createUndefDDRef(Cond->getType()));
    Refs.push_back(createUndefDDRef(Cond->getType()));
  } else if (auto ConstVal = dyn_cast<Constant>(Cond)) {
    if (ConstVal->isOneValue()) {
      Preds.push_back(PredicateTy::FCMP_TRUE);
    } else if (ConstVal->isZeroValue()) {
      Preds.push_back(PredicateTy::FCMP_FALSE);
    } else {
      llvm_unreachable("Unexpected conditional branch value");
    }
    Refs.push_back(createUndefDDRef(Cond->getType()));
    Refs.push_back(createUndefDDRef(Cond->getType()));
  } else {
    assert(Cond->getType()->isIntegerTy(1) && "Cond should be an i1 type");
    Preds.push_back(PredicateTy::ICMP_NE);
    Refs.push_back(createScalarDDRef(Cond, Level));
    Refs.push_back(getDDRefUtils().createConstDDRef(Cond->getType(), 0));
  }
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

  auto SrcBB = HIR->getSrcBBlock(If);
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
    if (LF->requiresZttInversion(HLoop)) {
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

  // If 'then' is empty, move 'else' children to 'then' by inverting predicate.
  if (!If->hasThenChildren() && (If->getNumPredicates() == 1)) {
    If->invertPredicate(PredIter);
    getHLNodeUtils().moveAsFirstChildren(If, If->else_begin(), If->else_end(),
                                         true);
  }
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

unsigned HIRParser::getElementSize(Type *Ty) const {
  assert(isa<PointerType>(Ty) && "Invalid type!");

  auto ElTy = cast<PointerType>(Ty)->getElementType();

  return getDataLayout().getTypeSizeInBits(ElTy) / 8;
}

const Value *HIRParser::getHeaderPhiOperand(const PHINode *Phi,
                                            bool IsInit) const {
  assert(RI->isHeaderPhi(Phi) && "Phi is not a header phi!");

  auto Lp = LI->getLoopFor(Phi->getParent());

  for (unsigned I = 0, E = Phi->getNumIncomingValues(); I != E; ++I) {
    auto PhiOp = Phi->getIncomingValue(I);

    auto Inst = dyn_cast<Instruction>(PhiOp);

    if (IsInit) {
      if (!Inst || (LI->getLoopFor(Inst->getParent()) != Lp)) {
        return PhiOp;
      }
    } else {
      if (Inst && (LI->getLoopFor(Inst->getParent()) == Lp)) {
        return Inst;
      }
    }
  }

  llvm_unreachable("Could not find appropriate header phi operand!");
}

const Value *HIRParser::getHeaderPhiInitVal(const PHINode *Phi) const {
  return getHeaderPhiOperand(Phi, true);
}

const Value *HIRParser::getHeaderPhiUpdateVal(const PHINode *Phi) const {
  return getHeaderPhiOperand(Phi, false);
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
  auto StrideSCEV = SE->getMinusSCEV(UpdateSCEV, PhiSCEV);

  auto IndexTy = Type::getIntNTy(
      getContext(), getDataLayout().getTypeSizeInBits(Phi->getType()));

  // Create index as {0,+,stride}
  auto InitSCEV = SE->getConstant(IndexTy, 0);
  auto IndexSCEV =
      SE->getAddRecExpr(InitSCEV, StrideSCEV, LI->getLoopFor(Phi->getParent()),
                        cast<SCEVAddRecExpr>(PhiSCEV)->getNoWrapFlags());

  auto IndexCE = getCanonExprUtils().createCanonExpr(IndexTy);

  // Disable cast hiding to prevent possible merging issues.
  if (!parseRecursive(IndexSCEV, IndexCE, Level, true, true, true)) {
    getCanonExprUtils().destroy(IndexCE);
    return nullptr;
  }

  return IndexCE;
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

  assert(IndexCE1->getCanonExprUtils().mergeable(IndexCE1, IndexCE2) &&
         "Indices cannot be merged!");
  IndexCE1->getCanonExprUtils().add(IndexCE1, IndexCE2);
}

void HIRParser::populateOffsets(const GEPOperator *GEPOp,
                                SmallVectorImpl<int64_t> &Offsets) {

  Offsets.clear();

  // First index can never be a structure field offset.
  Offsets.push_back(-1);

  unsigned NumOp = GEPOp->getNumOperands();
  auto CurTy = cast<PointerType>(GEPOp->getPointerOperand()->getType())
                   ->getElementType();

  // Ignore pointer operand and first index.
  for (unsigned I = 2; I < NumOp; ++I) {

    if (auto SeqTy = dyn_cast<SequentialType>(CurTy)) {
      CurTy = SeqTy->getElementType();
      Offsets.push_back(-1);

    } else {
      assert(isa<StructType>(CurTy) && "Unexpected type encountered!");
      auto StrucTy = cast<StructType>(CurTy);
      auto Operand = GEPOp->getOperand(I);

      assert(isa<ConstantInt>(Operand) &&
             "Structure offset is not a constant!");
      auto OffsetVal = cast<ConstantInt>(Operand)->getZExtValue();

      CurTy = StrucTy->getElementType(OffsetVal);
      Offsets.push_back(OffsetVal);
    }
  }
}

bool HIRParser::representsStructOffset(const GEPOperator *GEPOp) {
  SmallVector<int64_t, 8> Offsets;
  populateOffsets(GEPOp, Offsets);

  return (Offsets[GEPOp->getNumOperands() - 2] != -1);
}

const GEPOperator *HIRParser::getBaseGEPOp(const GEPOperator *GEPOp) const {

  while (auto TempGEPOp = dyn_cast<GEPOperator>(GEPOp->getPointerOperand())) {
    const GetElementPtrInst *GEPInst;

    if ((GEPInst = dyn_cast<GetElementPtrInst>(TempGEPOp)) &&
        (SE->getHIRMetadata(GEPInst, ScalarEvolution::HIRLiveKind::LiveRange) ||
         !RI->isSupported(GEPInst->getPointerOperand()->getType()))) {
      break;
    }

    // If TempGEPOp's last index is an offset and GEPOp's first index is not
    // zero then we have an unconventional structure access and the GEPs cannot
    // be merged. For example-
    //
    // %struct.IspComplex = type { float, float }
    //
    // %53 = getelementptr inbounds %struct.IspComplex, %struct.IspComplex* %P,
    // i64 %i, i32 0
    // %add.ptr.i.i = getelementptr inbounds float, float* %53, i64 %j
    //
    // In the above example the first GEP references a float field in the
    // structure but the second GEP treats it as a floating point array whose
    // %j'th element is being accessed.
    if (representsStructOffset(TempGEPOp)) {
      auto PrevOp1 = GEPOp->getOperand(1);
      auto ConstOp = dyn_cast<ConstantInt>(PrevOp1);

      if (!ConstOp || !ConstOp->isZero()) {
        break;
      }
    }

    GEPOp = TempGEPOp;
  }

  return GEPOp;
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
void HIRParser::populateRefDimensions(RegDDRef *Ref, const GEPOperator *GEPOp,
                                      unsigned Level,
                                      bool RequiresIndexMerging) {

  const GEPOperator *BaseGEPOp = getBaseGEPOp(GEPOp);
  const GEPOperator *TempGEPOp = GEPOp;
  CanonExpr *PrevGEPFirstIndexCE = nullptr;
  SmallVector<int64_t, 8> Offsets;
  SmallVector<unsigned, 4> CurDimOffsets;

  // If Ref has existing dimensions we may have to start from merging in the
  // highest dimension.
  bool MergeInHighestDimension = (Ref->getNumDimensions() != 0);
  bool IsBaseGEPOp = false;

  do {
    // Ignore base pointer operand.
    unsigned GEPNumOp = TempGEPOp->getNumOperands() - 1;
    IsBaseGEPOp = (TempGEPOp == BaseGEPOp);

    populateOffsets(TempGEPOp, Offsets);

    if (Offsets[GEPNumOp - 1] != -1) {
      // If last index of this GEP represents a field offset, we ignore the
      // previous GEP's first index as it is redundant.
      if (PrevGEPFirstIndexCE) {
        assert(PrevGEPFirstIndexCE->isZero() &&
               "PrevGEPFirstIndexCE expected to be zero!");
        getCanonExprUtils().destroy(PrevGEPFirstIndexCE);
        PrevGEPFirstIndexCE = nullptr;
      }
      MergeInHighestDimension = false;
    }

    // Process GEP operands in reverse order (from lowest to highest dimension).
    for (auto I = GEPNumOp; I > 0; --I) {

      // This operand is a structure field offset. It will be added as a
      // trailing offset for the next dimension.
      if (Offsets[I - 1] != -1) {
        CurDimOffsets.insert(CurDimOffsets.begin(), Offsets[I - 1]);
        continue;
      }

      // Disable IsTop operations such as cast hiding and denominator parsing
      // for indices which need to be merged. For example, first and last
      // indices in multiple gep case.
      bool IsTop = (!MergeInHighestDimension &&
                    (!PrevGEPFirstIndexCE || PrevGEPFirstIndexCE->isZero()) &&
                    ((I != 1) || (IsBaseGEPOp && !RequiresIndexMerging)));

      CanonExpr *IndexCE = parse(TempGEPOp->getOperand(I), Level, IsTop);

      // Store the first GEP index in PrevGEPFirstIndexCE. It will be merged
      // into the last index of next GEP.
      if ((I == 1) && !IsBaseGEPOp) {
        if (PrevGEPFirstIndexCE) {
          mergeIndexCE(PrevGEPFirstIndexCE, IndexCE);
        } else {
          PrevGEPFirstIndexCE = IndexCE;
        }
        continue;
      }

      if (PrevGEPFirstIndexCE) {
        mergeIndexCE(IndexCE, PrevGEPFirstIndexCE);
        getCanonExprUtils().destroy(PrevGEPFirstIndexCE);
        PrevGEPFirstIndexCE = nullptr;
      }

      if (MergeInHighestDimension) {
        CanonExpr *HighestIndexCE =
            Ref->getDimensionIndex(Ref->getNumDimensions());
        mergeIndexCE(HighestIndexCE, IndexCE);
        getCanonExprUtils().destroy(IndexCE);
        MergeInHighestDimension = false;
        continue;
      }

      Ref->addDimension(IndexCE, &CurDimOffsets);
      CurDimOffsets.clear();
    }

  } while (!IsBaseGEPOp &&
           (TempGEPOp = dyn_cast<GEPOperator>(TempGEPOp->getPointerOperand())));
}

void HIRParser::addPhiBaseGEPDimensions(const GEPOperator *GEPOp,
                                        const GEPOperator *InitGEPOp,
                                        RegDDRef *Ref, CanonExpr *IndexCE,
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
    Ref->addDimension(IndexCE);
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
                              const GEPOperator **InitGEPOp) const {

  *InitGEPOp = nullptr;

  auto GEPOp = dyn_cast<GEPOperator>(PhiInitVal);

  if (!GEPOp) {
    return PhiInitVal;
  }

  const Instruction *GEPInst = nullptr;

  // A phi init GEP representing an offset cannot be merged into the ref as it
  // represents an unconventional access.
  if (representsStructOffset(GEPOp)) {
    // If this is an instruction, we can use it as the base.
    if (isa<GetElementPtrInst>(PhiInitVal)) {
      return PhiInitVal;
    }

    // PhiInitVal is a constant expr, return null to indicate that the phi
    // itself should act as the base.
    return nullptr;
  } else if ((GEPInst = dyn_cast<Instruction>(PhiInitVal)) &&
             SE->getHIRMetadata(GEPInst,
                                ScalarEvolution::HIRLiveKind::LiveRange)) {
    // Return the same value if it has live range metadata.
    return PhiInitVal;
  }

  *InitGEPOp = GEPOp;
  return getBaseGEPOp(GEPOp)->getPointerOperand();
}

RegDDRef *HIRParser::createPhiBaseGEPDDRef(const PHINode *BasePhi,
                                           const GEPOperator *GEPOp,
                                           unsigned Level) {
  const PHINode *CurBasePhi = BasePhi;
  const Value *BaseVal = nullptr;
  bool IsInBounds = GEPOp ? GEPOp->isInBounds() : false;

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

  // A phi can be initialized using another phi so we should trace back.
  do {
    Type *BaseTy = CurBasePhi->getType();
    const GEPOperator *InitGEPOp = nullptr;
    CanonExpr *IndexCE = nullptr;

    auto SC = getSCEV(const_cast<PHINode *>(CurBasePhi));

    if (auto RecSCEV = dyn_cast<SCEVAddRecExpr>(SC)) {
      const Value *PhiInitVal = getHeaderPhiInitVal(CurBasePhi);

      if (RecSCEV->isAffine() &&
          (BaseVal = getValidPhiBaseVal(PhiInitVal, &InitGEPOp))) {
        IndexCE = createHeaderPhiIndexCE(CurBasePhi, Level);

        if (IndexCE) {
          unsigned ElementSize = getElementSize(BaseTy);

          // Normalize with respect to element size.
          IndexCE->divide(ElementSize);
          IndexCE->simplify(true);
        }
      }

      // Use no wrap flags to set inbounds property.
      IsInBounds = IsInBounds || (RecSCEV->getNoWrapFlags(SCEV::FlagNUW) ||
                                  RecSCEV->getNoWrapFlags(SCEV::FlagNSW));
    }

    // Non-linear base is parsed as base + zero offset: (%p)[0].
    if (!IndexCE) {
      BaseVal = CurBasePhi;

      auto OffsetType = Type::getIntNTy(
          getContext(), getDataLayout().getTypeSizeInBits(BaseTy));
      IndexCE = getCanonExprUtils().createCanonExpr(OffsetType);
    }

    addPhiBaseGEPDimensions(GEPOp, InitGEPOp, Ref, IndexCE, Level);
    GEPOp = nullptr;

  } while ((CurBasePhi != BaseVal) &&
           (CurBasePhi = dyn_cast<PHINode>(BaseVal)) &&
           CurRegion->containsBBlock(CurBasePhi->getParent()));

  auto BaseCE = parse(BaseVal, Level);

  Ref->setBaseCE(BaseCE);
  Ref->setInBounds(IsInBounds);

  return Ref;
}

RegDDRef *HIRParser::createRegularGEPDDRef(const GEPOperator *GEPOp,
                                           unsigned Level) {
  auto Ref = getDDRefUtils().createRegDDRef(0);

  const GEPOperator *BaseGEPOp = getBaseGEPOp(GEPOp);
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
  const GEPOperator *GEPOp = nullptr;
  const Value *OrigGEPVal = GEPVal;
  RegDDRef *Ref = nullptr;
  Type *DestTy = nullptr;

  clearTempBlobLevelMap();

  ParsingScalarLval = false;

  // In some cases float* is converted into i32* before loading/storing. This
  // info is propagated into the BaseCE dest type.
  if (auto BCOp = dyn_cast<BitCastOperator>(GEPVal)) {
    if ((!isa<Instruction>(BCOp) ||
         !SE->getHIRMetadata(cast<Instruction>(BCOp),
                             ScalarEvolution::HIRLiveKind::LiveOut)) &&
        RI->isSupported(BCOp->getOperand(0)->getType())) {
      GEPVal = BCOp->getOperand(0);
      DestTy = BCOp->getDestTy();
    }
  }

  auto GEPInst = dyn_cast<Instruction>(GEPVal);

  // Try to get to the phi associated with this GEP.
  // Do not cross the live range indicator for GEP uses (load/store/bitcast).
  if ((!IsUse || !GEPInst ||
       !SE->getHIRMetadata(GEPInst, ScalarEvolution::HIRLiveKind::LiveRange)) &&
      (GEPOp = dyn_cast<GEPOperator>(GEPVal))) {

    BasePhi = dyn_cast<PHINode>(getBaseGEPOp(GEPOp)->getPointerOperand());

  } else if (GEPInst) {
    BasePhi = dyn_cast<PHINode>(GEPInst);
  }

  if (BasePhi && RI->isHeaderPhi(BasePhi)) {
    Ref = createPhiBaseGEPDDRef(BasePhi, GEPOp, Level);
  } else if (GEPOp) {
    Ref = createRegularGEPDDRef(GEPOp, Level);
  } else {
    Ref = createSingleElementGEPDDRef(GEPVal, Level);
  }

  if (DestTy) {
    Ref->setBaseDestType(DestTy);
  }

  populateBlobDDRefs(Ref, Level);

  // Add a mapping for getting the original pointer value for the Ref.
  GEPRefToPointerMap.insert(
      std::make_pair(Ref, const_cast<Value *>(OrigGEPVal)));

  return Ref;
}

RegDDRef *HIRParser::createUndefDDRef(Type *Ty) {
  Value *UndefVal = UndefValue::get(Ty);

  return getDDRefUtils().createConstDDRef(UndefVal);
}

RegDDRef *HIRParser::createScalarDDRef(const Value *Val, unsigned Level,
                                       bool IsLval) {
  CanonExpr *CE;

  clearTempBlobLevelMap();

  ParsingScalarLval = IsLval;

  auto Symbase = getOrAssignSymbase(Val);
  auto Ref = getDDRefUtils().createRegDDRef(Symbase);

  CE = parse(Val, Level);

  Ref->setSingleCanonExpr(CE);

  if (CE->isSelfBlob()) {
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

  return Ref;
}

RegDDRef *HIRParser::createRvalDDRef(const Instruction *Inst, unsigned OpNum,
                                     unsigned Level) {
  RegDDRef *Ref;
  auto OpVal = Inst->getOperand(OpNum);

  if (auto LInst = dyn_cast<LoadInst>(Inst)) {
    Ref = createGEPDDRef(LInst->getPointerOperand(), Level, true);

    Ref->setVolatile(LInst->isVolatile());
    Ref->setAlignment(LInst->getAlignment());

    parseMetadata(LInst, Ref);

  } else if (isa<GetElementPtrInst>(Inst)) {
    Ref = createGEPDDRef(Inst, Level, false);
    Ref->setAddressOf(true);

    parseMetadata(Inst, Ref);

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

    Ref->setVolatile(SInst->isVolatile());
    Ref->setAlignment(SInst->getAlignment());

    parseMetadata(Inst, Ref);

  } else {
    Ref = createScalarDDRef(Inst, Level, true);
  }

  return Ref;
}

bool HIRParser::isLiveinCopy(const HLInst *HInst) {
  return HInst->isCopyInst() &&
         SE->getHIRMetadata(HInst->getLLVMInstruction(),
                            ScalarEvolution::HIRLiveKind::LiveIn);
}

bool HIRParser::isLiveoutCopy(const HLInst *HInst) {
  return HInst->isCopyInst() &&
         SE->getHIRMetadata(HInst->getLLVMInstruction(),
                            ScalarEvolution::HIRLiveKind::LiveOut);
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

FastMathFlags HIRParser::parseFMF(const CmpInst *Cmp) {
  if (auto *FCmp = dyn_cast_or_null<FPMathOperator>(Cmp)) {
    return FCmp->getFastMathFlags();
  }

  return FastMathFlags();
}

bool HIRParser::parseDebugIntrinsic(HLInst *Inst) {
  if (!RemoveDebugIntrinsics) {
    return false;
  }

  const DbgInfoIntrinsic *DbgIntrin =
      dyn_cast<DbgInfoIntrinsic>(Inst->getLLVMInstruction());

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
    BlobTy UndefBlob = SE->getUnknown(UndefVal);

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

void HIRParser::parse(HLInst *HInst, bool IsPhase1, unsigned Phase2Level) {
  bool HasLval = false;
  auto Inst = HInst->getLLVMInstruction();
  unsigned Level;

  assert(!Inst->getType()->isVectorTy() && "Vector types not supported!");

  setCurNode(HInst);

  if (IsPhase1) {
    Level = CurLevel;

    if (parseDebugIntrinsic(HInst)) {
      getHLNodeUtils().erase(HInst);
      return;
    }

  } else {
    Level = Phase2Level;
    auto OuterLoop = HInst->getOutermostParentLoop();
    CurOutermostLoop = OuterLoop ? OuterLoop->getLLVMLoop() : nullptr;
  }

  // Process lval
  if (HInst->hasLval()) {
    HasLval = true;

    if (IsPhase1 && !isEssential(Inst)) {
      // Postpone the processing of this instruction to Phase2.
      auto Symbase = getOrAssignSymbase(Inst);
      UnclassifiedSymbaseInsts[Symbase].push_back(std::make_pair(HInst, Level));
      return;
    }

    HInst->setLvalDDRef(createLvalDDRef(Inst, Level));
  }

  unsigned NumRvalOp = getNumRvalOperands(HInst);
  auto Call = dyn_cast<CallInst>(Inst);

  bool FakeDDRefsRequired = (Call && !Call->doesNotAccessMemory());
  bool IsReadOnly =
      (FakeDDRefsRequired && Call->hasFnAttr(Attribute::ReadOnly));

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

    if (FakeDDRefsRequired && Ref->isAddressOf() &&
        !Ref->accessesConstantArray() &&
        !Call->paramHasAttr(I + 1, Attribute::ReadNone)) {
      addFakeRef(HInst, Ref, (IsReadOnly ||
                              Call->paramHasAttr(I + 1, Attribute::ReadOnly)));
    }
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
      getHLNodeUtils().erase(InstIt->first);
    }
  }

  UnclassifiedSymbaseInsts.clear();
}

bool HIRParser::runOnFunction(Function &F) {
  Func = &F;
  DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
  RI = &getAnalysis<HIRRegionIdentification>();
  HIR = &getAnalysis<HIRCreation>();
  LF = &getAnalysis<HIRLoopFormation>();
  ScalarSA = &getAnalysis<HIRScalarSymbaseAssignment>();

  // Connect contained DDRefUtils to HLNodeUtils object.
  HIR->getHLNodeUtils().DDRU = &DDRU;

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

  return false;
}

void HIRParser::releaseMemory() {
  /// Destroy all DDRefs and CanonExprs.
  DDRU.destroyAll();

  CurTempBlobLevelMap.clear();
  UnclassifiedSymbaseInsts.clear();
  RequiredSymbases.clear();
  BlobTable.clear();
  BlobToIndexMap.clear();
  SymbaseToIndexMap.clear();
  GEPRefToPointerMap.clear();
}

void HIRParser::print(raw_ostream &OS, const Module *M) const {
  print(true, OS, M);
}

void HIRParser::print(bool FrameworkDetails, raw_ostream &OS,
                      const Module *M) const {
  HIR->print(FrameworkDetails, OS, M);
}

// Verification is done by HIRVerifier.
void HIRParser::verifyAnalysis() const {}

void HIRParser::parseMetadata(const Instruction *Inst, RegDDRef *Ref) {
  assert(Ref->hasGEPInfo() && "Ref is expected to be gep DDRef");

  const StoreInst *Store = dyn_cast<StoreInst>(Inst);
  const LoadInst *Load = Store ? nullptr : dyn_cast<LoadInst>(Inst);

  if (Store || Load) {
    Inst->getAllMetadataOtherThanDebugLoc(Ref->getGEPInfo()->MDNodes);
    Ref->setMemDebugLoc(Inst->getDebugLoc());

    const GetElementPtrInst *GepInst = dyn_cast<GetElementPtrInst>(
        Store ? Store->getPointerOperand() : Load->getPointerOperand());

    if (GepInst) {
      Ref->setGepDebugLoc(GepInst->getDebugLoc());
    }

  } else if (isa<GetElementPtrInst>(Inst)) {
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

ArrayType *HIRParser::traceBackToArrayType(const Value *Ptr) const {
  if (!Ptr->getType()->isPointerTy()) {
    return nullptr;
  }

  // Trace back as far as possible, until we hit a GEP whose result type is an
  // array type.
  while (Ptr) {
    if (auto Phi = dyn_cast<PHINode>(Ptr)) {
      if (Phi->getNumIncomingValues() == 1) {
        Ptr = Phi->getIncomingValue(0);

      } else if (RI->isHeaderPhi(Phi)) {
        Ptr = getHeaderPhiInitVal(Phi);

      } else {
        // Give up on merge phis.
        return nullptr;
      }
    } else if (auto GEPOp = dyn_cast<GEPOperator>(Ptr)) {
      if (GEPOp->getNumOperands() == 2) {
        Ptr = GEPOp->getPointerOperand();
      } else {
        return dyn_cast<ArrayType>(GEPOp->getSourceElementType());
      }
    } else {
      // Give up on other value types.
      return nullptr;
    }
  }

  return nullptr;
}
