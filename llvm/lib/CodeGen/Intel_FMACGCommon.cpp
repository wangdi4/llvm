//===- Intel_FMACGCommon.cpp - Fused Multiply Add optimization ------------===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file defines the common global FMA optimization part that is shared
// by all targets.
//
//===----------------------------------------------------------------------===//

#include "llvm/CodeGen/Intel_FMACGCommon.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Format.h"

#define DEBUG_TYPE "global-fma"

using namespace llvm;

/// This internal bit-set switch to control the amount of debug printings.
/// The bits are:
///   FMADbgLevel[0] - print the general debug messages.
///   FMADbgLevel[1] - print the pattern matching related messages.
///   FMADbgLevel[2] - print the messages during the fusing optimization.
static cl::opt<int> FMADbgLevel("global-fma-debug-level",
                                cl::desc("Control the amount of debug output."),
                                cl::init(-1), cl::Hidden);

raw_ostream &FMADbg::dbgs() {
  return (FMADbgLevel & FMADbg::Main) ? ::dbgs() : nulls();
}
raw_ostream &FMADbg::match() {
  if (FMADbgLevel & FMADbg::Matching) {
    ::dbgs() << "  MATCHING: ";
    return ::dbgs();
  }
  return nulls();
}
raw_ostream &FMADbg::fws() {
  if (FMADbgLevel & FMADbg::FWS) {
    ::dbgs() << "  FWS: ";
    return ::dbgs();
  }
  return nulls();
}

bool FMAPerfDesc::isBetterThan(const FMAPerfDesc &OtherDesc,
                               bool TuneForLatency,
                               bool TuneForThroughput) const {
  // Tuning for latency AND throughput means that the caller does not have
  // strong preferences and the choice should be made heuristically.
  if (TuneForLatency && TuneForThroughput) {
    TuneForLatency = false;
    TuneForThroughput = false;
  }

  unsigned NumOperations = getNumOperations();
  unsigned OtherNumOperations = OtherDesc.getNumOperations();

  if (TuneForThroughput) {
    // If the number of operations in this descriptor is smaller than
    // the number of operations in 'OtherDesc', then just return true.
    if (NumOperations != OtherNumOperations)
      return NumOperations < OtherNumOperations;

    // If the numbers of operations are equal, then return true or false
    // depending on which descriptor has smaller latency.
    if (Latency != OtherDesc.Latency)
      return Latency < OtherDesc.Latency;

    // Less FMAs is preferred because they clobber one of operands and thus
    // are less flexible than MUL/ADD/SUB operations.
    return NumFMA < OtherDesc.NumFMA;
  }

  if (TuneForLatency) {
    // If the latencies are different, then return true or false depending on
    // which descriptor has smaller latency.
    if (Latency != OtherDesc.Latency)
      return Latency < OtherDesc.Latency;

    // If the latencies are identical, then compare the numbers of operations.
    if (NumOperations != OtherNumOperations)
      return NumOperations < OtherNumOperations;

    // Less FMAs is preferred because they clobber one of operands and thus
    // are less flexible than MUL/ADD/SUB operations.
    return NumFMA < OtherDesc.NumFMA;
  }

  double LatencyImprovement;
  if (Latency < OtherDesc.Latency)
    LatencyImprovement = (double)OtherDesc.Latency / (double)Latency - 1.0;
  else
    LatencyImprovement = -((double)Latency / (double)OtherDesc.Latency - 1.0);

  double ThroughputImprovement;
  if (NumOperations < OtherNumOperations)
    ThroughputImprovement =
      (double)OtherNumOperations / (double)NumOperations - 1.0;
  else
    ThroughputImprovement =
      -((double)NumOperations / (double)OtherNumOperations - 1.0);

  double Improvement = LatencyImprovement + ThroughputImprovement;
  if (Improvement == 0)
    // Prefer to have less FMAs as FMAs are less flexible when they are
    // processed by memory-folding, coalescing and register allocation
    // optimizations.
    return NumFMA < OtherDesc.NumFMA;
  return LatencyImprovement + ThroughputImprovement > 0;
}

void FMAExprSP::initForEncodedDag(uint64_t EncodedDag) {
  assert(!Dag && "initForEncodedDag() is applied to initialized SP");
  Dag = new FMADag(EncodedDag);
  bool isOk = initForDag(*Dag);
  (void)isOk;
  assert(isOk && "Could not initialize SP for 64-bit encoded DAG.");

  canonize();
  computeShape();
}

void FMAExprSP::canonize() {
  FMAExprSPCommon::canonize();

  if (NumProducts == 0)
    return;

  // The base version of the canonize() method sorted the products.
  // If there are some equal products but having opposite signs, then the
  // products with negative signs got placed after products with positive
  // signs. For example: +abc+ad+ad-ad-ad+c.
  //
  // The loop below looks for and removes such products with opposite signs.
  for (unsigned ProdInd = 1; ProdInd < NumProducts; ProdInd++) {
    const FMAExprProduct *PrevProd = &Products[ProdInd - 1];
    const FMAExprProduct *CurProd = &Products[ProdInd];
    if (CurProd->Sign && !PrevProd->Sign &&
        CurProd->NumTerms == PrevProd->NumTerms) {

      // Compare the products.
      unsigned TermInd;
      for (TermInd = 0; TermInd < CurProd->NumTerms; TermInd++) {
        if (PrevProd->Terms[TermInd] != CurProd->Terms[TermInd])
          break;
      }

      // If the products are equal, then just remove both products.
      if (TermInd == CurProd->NumTerms) {
        // Ok, just remove two products now.
        for (unsigned i = ProdInd + 1; i < NumProducts; i++)
          Products[i - 2] = Products[i];
        NumProducts -= 2;

        // Two products with indices (ProdInd - 1) and (ProdInd) have been
        // removed. Go to the next loop iteration. Adjust the loop variable
        // to remove more products with opposite signs and not to skip any
        // optimizable cases.
        // For example:
        //    +abc+ad+ad-ad-ad+c // ProdInd here is equal to 3,
        //               ^^
        //    SP after removal of the products with indices 2 and 3:
        //    +abc+ad-ad+c
        //    ProdInd must be set to 1 here.
        // So, subtract 2 from ProdInd, but do that carefully, i.e. do not make
        // ProdInd negative.
        ProdInd--;
        if (ProdInd != 0)
          ProdInd--;
      }
    }
  }

  // Handle a special case. If all products got removed, then the result
  // sum of product is equal to zero.
  if (NumProducts == 0) {
    NumProducts = 1;
    Products[0].setSingleton(false, TermZERO);
  }
}

std::unique_ptr<unsigned[]> FMAExprSP::getTermsMappingToCompactTerms() {

  // First of all get the mask showing what terms are used.
  // For each of the used terms set the corresponding bit in the bit mask.
  SmallBitVector IsTermUsed(MaxNumOfUniqueTermsInSP);
  unsigned UsageMask = 0;

  for (unsigned ProdInd = 0; ProdInd < NumProducts; ProdInd++) {
    unsigned NumTerms = Products[ProdInd].NumTerms;
    uint8_t *Terms = Products[ProdInd].Terms;
    for (unsigned TermInd = 0; TermInd < NumTerms; TermInd++) {
      unsigned Term = Terms[TermInd];
      if (Term != TermZERO && Term != TermONE) {
        UsageMask |= 1 << Term;
        IsTermUsed[Term] = true;
      }
    }
  }

  // If the mask is full, then just return nullptr as the terms
  // mapping is not needed.
  if (((UsageMask + 1) & UsageMask) == 0)
    return nullptr;

  // Compact the term indices now.
  auto TermsMapping = std::make_unique<unsigned[]>(MaxNumOfUniqueTermsInSP);
  unsigned TheLastNewUsedTerm = 0;
  for (unsigned Term = 0; Term < MaxNumOfUniqueTermsInSP; Term++) {
    if (!IsTermUsed[Term])
      TermsMapping[Term] = ~0U;
    else {
      TermsMapping[Term] = TheLastNewUsedTerm;
      TheLastNewUsedTerm++;
    }
  }

  return TermsMapping;
}

bool FMADag::isLastUse(unsigned NodeInd, unsigned OpndInd) const {
  bool OpndIsTerm;
  unsigned SearchedOpnd = getOperand(NodeInd, OpndInd, &OpndIsTerm);

  for (unsigned OI = 0; OI < OpndInd; OI++) {
    bool IsTerm;
    unsigned ExprOrTerm = getOperand(NodeInd, OI, &IsTerm);
    if (IsTerm == OpndIsTerm && ExprOrTerm == SearchedOpnd)
      return false;
  }

  for (unsigned NI = 0; NI < NodeInd; NI++) {
    for (unsigned OI = 0; OI < 3; OI++) {
      bool IsTerm;
      unsigned ExprOrTerm = getOperand(NI, OI, &IsTerm);
      if (IsTerm == OpndIsTerm && ExprOrTerm == SearchedOpnd)
        return false;
    }
  }
  return true;
}

bool FMADag::isMul(unsigned NodeInd) const {
  bool AIsTerm, BIsTerm, CIsTerm;

  // If A is 0.0 or 1.0 then it is not a MUL operation.
  unsigned A = getOperand(NodeInd, 0, &AIsTerm);
  if (AIsTerm && (A == FMADagCommon::TermZERO || A == FMADagCommon::TermONE))
    return false;

  // If B is 0.0 or 1.0 then it is not a MUL operation.
  unsigned B = getOperand(NodeInd, 1, &BIsTerm);
  if (BIsTerm && (B == FMADagCommon::TermZERO || B == FMADagCommon::TermONE))
    return false;

  // If C is NOT 0.0 then it is not a MUL operation.
  unsigned C = getOperand(NodeInd, 2, &CIsTerm);
  if (!CIsTerm || C != FMADagCommon::TermZERO)
    return false;

  return true;
}

bool FMADag::isAdd(unsigned NodeInd) const {
  bool AIsTerm, BIsTerm, CIsTerm;

  // If A is 0.0 then it is not an ADD operation.
  unsigned A = getOperand(NodeInd, 0, &AIsTerm);
  bool AIsZero = false, AIsOne = false;
  if (AIsTerm) {
    AIsZero = A == FMADagCommon::TermZERO;
    AIsOne = A == FMADagCommon::TermONE;
  }
  if (AIsZero)
    return false;

  // If B is 0.0 then it is not an ADD operation.
  unsigned B = getOperand(NodeInd, 1, &BIsTerm);
  bool BIsZero = false, BIsOne = false;
  if (BIsTerm) {
    BIsZero = B == FMADagCommon::TermZERO;
    BIsOne = B == FMADagCommon::TermONE;
  }
  if (BIsZero)
    return false;

  // At least one of A and B must be equal to 1.0 in ADD operation.
  if (!AIsOne && !BIsOne)
    return false;

  // If C is 0.0 then it is not an ADD operation.
  unsigned C = getOperand(NodeInd, 2, &CIsTerm);
  if (CIsTerm && C == FMADagCommon::TermZERO)
    return false;

  return true;
}

bool FMADag::isFMA(unsigned NodeInd) const {
  bool AIsTerm, BIsTerm, CIsTerm;

  // If A is 0.0 or 1.0 then it is not an FMA operation.
  unsigned A = getOperand(NodeInd, 0, &AIsTerm);
  if (AIsTerm && (A == FMADagCommon::TermZERO || A == FMADagCommon::TermONE))
    return false;

  // If B is 0.0 or 1.0 then it is not an FMA operation.
  unsigned B = getOperand(NodeInd, 1, &BIsTerm);
  if (BIsTerm && (B == FMADagCommon::TermZERO || B == FMADagCommon::TermONE))
    return false;

  // If C is 0.0 then it is not an FMA operation.
  unsigned C = getOperand(NodeInd, 2, &CIsTerm);
  if (CIsTerm && C == FMADagCommon::TermZERO)
    return false;

  return true;
}

unsigned FMADag::getLatency(unsigned MulLatency, unsigned AddSubLatency,
                            unsigned FMALatency, unsigned NodeInd) const {
  unsigned Latency = 0;
  for (unsigned OpndInd = 0; OpndInd < 3; OpndInd++) {
    bool OpndIsTerm;
    unsigned Opnd = getOperand(NodeInd, OpndInd, &OpndIsTerm);
    if (!OpndIsTerm)
      Latency = std::max(
          Latency, getLatency(MulLatency, AddSubLatency, FMALatency, Opnd));
  }

  if (isMul(NodeInd))
    Latency += MulLatency;
  else if (isAdd(NodeInd))
    Latency += AddSubLatency;
  else if (isFMA(NodeInd))
    Latency += FMALatency;
  else
    llvm_unreachable("Dag has obvious inefficiencies.");

  return Latency;
}

void FMAPatterns::init() {
  auto *SP = acquireSP(Dags.back().front());
  LargestAvailableShape = SP->Shape;
}

const FMAExprSP *FMAPatterns::acquireSP(uint64_t EncodedDag) {
  auto &SP = EncodedDagToSPMap[EncodedDag];
  if (!SP) {
    SP = std::make_unique<FMAExprSP>();
    SP->initForEncodedDag(EncodedDag);
  }
  return SP.get();
}

const FMAPatterns::FMAPatternsSet *
FMAPatterns::getDagsForShape(uint64_t Shape) {
  unsigned First = 0, Last = getNumShapes() - 1;

  // If the passed 'Shape' is bigger than the biggest available shape in
  // the storage, then just exit early and skip the binary search.
  auto *SP = acquireSP(Dags[Last].front());
  if (Shape > SP->Shape)
    return nullptr;
  if (Shape == SP->Shape)
    return &Dags[Last];

  while (First < Last) {
    // Check the SHAPE of a set of DAGs in the middle of the search scope.
    unsigned Middle = (First + Last) / 2;
    SP = acquireSP(Dags[Middle].front());
    uint64_t CurShape = SP->Shape;

    // If the searched SHAPE is found, then return the whole set of DAGs having
    // the same SHAPE.
    if (Shape == CurShape)
      return &Dags[Middle];

    // Halve the search scope and continue the binary search.
    if (Shape < CurShape)
      Last = Middle;
    else
      First = Middle + 1;
  }

  return nullptr;
}

std::unique_ptr<FMADag> FMAPatterns::getDagForBestSPMatch(const FMAExprSP &SP) {

  const FMAPatternsSet *DagsSet = getDagsForShape(SP.Shape);
  if (!DagsSet)
    return nullptr;

  LLVM_DEBUG(FMADbg::match() << "could find a set of DAGs for SHAPE("
                             << format_hex(SP.Shape, 2) << ")\n");

  // Find the best DAG for the given SP.
  std::unique_ptr<FMADagCommon> BestDag;
  for (const auto Dag64 : *DagsSet) {
    // FIXME: TermONE in C position is not yet supported, e.g. (A*B+1). Fix it.
    FMADag Dag(Dag64);

    auto *CandidateSP = acquireSP(Dag64);

    LLVM_DEBUG(FMADbg::match() << "let's try to match 2 SPs:\n");
    LLVM_DEBUG(FMADbg::match() << "  actual: " << SP);
    LLVM_DEBUG(FMADbg::match() << "  formal: " << *CandidateSP);

    FMASPToSPMatcher SPMatcher;
    if (auto *CandidateDag = SPMatcher.getDagToMatchSPs(*CandidateSP, SP)) {
      // Ok, we found Sum Of Products. Let's do some heuristical checks
      // and choose the best alternative here.

      // FIXME: currently we just choose the first one and return it.
      BestDag.reset(CandidateDag);
      break;
    }
  }

  if (BestDag)
    return std::make_unique<FMADag>(*BestDag);
  return nullptr;
}

bool FMANode::isZero() const {
  if (auto *Term = dyn_cast<FMAImmediateTerm>(this))
    return Term->isZero();
  return false;
}

bool FMANode::isOne() const {
  if (auto *Term = dyn_cast<FMAImmediateTerm>(this))
    return Term->isOne();
  return false;
}

FMAExpr::FMAExpr(MVT VT, FMABasicBlock *BB, MachineInstr *MI,
                 FMARegisterTerm *ResultTerm,
                 const std::array<FMANode *, 3u> &Ops, bool MulSign,
                 bool AddSign, unsigned IndexInBB)
    : FMANode(NK_Expr, BB, VT), MulSign(MulSign), AddSign(AddSign),
      Operands(Ops), IsFullyConsumedByKnownExpressions(false),
      IsMustBeOptimized(false), ResultTerm(ResultTerm), MI(MI),
      DbgValMI(nullptr), IndexInBB(IndexInBB) {

  assert(ResultTerm && "Unexpected result term in FMAExpr constructor.");

  for (auto *Op : Operands) {
    assert(Op && "Unexpected operands in FMAExpr constructor.");
    if (auto *Term = dyn_cast<FMATerm>(Op))
      if (!Term->isZero() && !Term->isOne())
        addToUsedTerms(cast<FMATerm>(Op));
  }
}

void FMAExpr::insertImmediateUsersOfTo(
         const FMANode *Node, SmallPtrSetImpl<const FMAExpr *> &ExprSet) const {
  SmallVector<const FMANode *, 16u> WorkList({this});
  do {
    auto *Expr = cast<FMAExpr>(WorkList.pop_back_val());
    for (auto *Opnd : Expr->Operands) {
      if (Opnd == Node) {
        ExprSet.insert(Expr);
        break;
      }
      if (isa<FMAExpr>(Opnd))
        WorkList.push_back(Opnd);
    }
  } while (!WorkList.empty());
}

void FMAExpr::putExprToExprSet(
    SmallPtrSetImpl<const FMAExpr *> &ExprSet) const {
  SmallVector<const FMANode *, 16u> WorkList({this});
  do {
    auto *Expr = cast<FMAExpr>(WorkList.pop_back_val());
    if (ExprSet.insert(Expr).second)
      copy_if(Expr->Operands, std::back_inserter(WorkList),
              [](FMANode *N) { return isa<FMAExpr>(N); });
  } while (!WorkList.empty());
}

void FMAExpr::print(raw_ostream &OS, bool PrintAttributes) const {
  bool IsFullyConsumed = isFullyConsumed();
  if (!IsFullyConsumed)
    OS << *ResultTerm << " = ";
  OS << (MulSign ? "FNM" : "FM") << (AddSign ? "S(" : "A(") << *Operands[0]
     << "," << *Operands[1] << "," << *Operands[2] << ")";
  if (PrintAttributes) {
    OS << " // Type: " << EVT(VT).getEVTString();
    if (!IsFullyConsumed)
      OS << "\n  MI: " << *MI;
    OS << "  UsedTerms: ";
    for (auto *T : UsedTerms)
      OS << *T << ", ";
    OS << "\n";
  }
}

unsigned FMAExpr::getUsedTermIndex(const FMATerm *Term) const {
  auto I = find(UsedTerms, Term);
  assert(I != UsedTerms.end() &&
         "Cannot find FMA term in the list of used terms.");
  return std::distance(UsedTerms.begin(), I);
}

void FMAExpr::addToUsedTerms(FMATerm *Term) {
  UsedTerms.insert(Term);
}

void FMAExpr::removeFromUsedTerms(FMARegisterTerm *Term) {
  auto Res = UsedTerms.remove(Term);
  assert(Res && "Cannot remove a term that is not in a list of used terms.");
  (void)Res;
}

FMAExprSP *FMAExpr::generateSPRecursively(
    const FMAExpr *RootFMAExpr,
    SmallDenseMap<const FMANode *, std::unique_ptr<FMAExprSP>> &Node2SP) const {
  // If the sum of products is already initialized for 'this' FMA expression,
  // then return it.
  if (auto &SP = Node2SP[this])
    return SP.get();

  SmallVector<FMAExprSP *, 3u> OperandSP;
  for (auto *Opnd : Operands) {
    FMAExprSP *OpnSP = nullptr;
    if (auto *Term = dyn_cast<FMATerm>(Opnd)) {
      auto &TermSP = Node2SP[Term];
      if (!TermSP) {
        if (Term->isZero())
          TermSP = std::make_unique<FMAExprSP>(FMAExprSPCommon::TermZERO);
        else if (Term->isOne())
          TermSP = std::make_unique<FMAExprSP>(FMAExprSPCommon::TermONE);
        else
          TermSP = std::make_unique<FMAExprSP>(RootFMAExpr->getUsedTermIndex(Term));
      }
      OpnSP = TermSP.get();
    } else if (auto *Expr = dyn_cast<FMAExpr>(Opnd))
      OpnSP = Expr->generateSPRecursively(RootFMAExpr, Node2SP);
    else
      llvm_unreachable("Unsupported node kind.");

    // Sums of products must be available for all operands. Otherwise,
    // it is impossible to generate a sum of products for expression.
    if (!OpnSP)
      return nullptr;

    OperandSP.push_back(OpnSP);
  }

  FMAExprSP MulSP;
  if (!MulSP.initForMul(*OperandSP[0], *OperandSP[1]))
    return nullptr;

  auto &SP = Node2SP[this] = std::make_unique<FMAExprSP>();
  if (!SP->initForAdd(MulSP, *OperandSP[2], MulSign, AddSign))
    return nullptr;

  return SP.get();
}

std::unique_ptr<FMAExprSP> FMAExpr::generateSP() const {
  // Exit early if the number of terms is obviously too big and
  // SP cannot be built.
  if (UsedTerms.size() > FMAExprSP::MaxNumOfUniqueTermsInSP)
    return nullptr;

  SmallDenseMap<const FMANode *, std::unique_ptr<FMAExprSP>> Node2SP;
  if (!generateSPRecursively(this, Node2SP))
    return nullptr;

  auto SP = std::move(Node2SP[this]);
  assert(SP && "SP was not generated");
  SP->canonize();
  SP->computeShape();
  return SP;
}

// The canonize() method may remove some of terms completely,
// For example,
//     Before: +ab+c-c+d
//     After : +ab+d
// The term 'c' got totally removed here. Let's compact the terms
// in SP and in 'this' FMAExpr.
void FMAExpr::compactTerms(FMAExprSP &SP) {
  if (auto TermsMapping = SP.getTermsMappingToCompactTerms()) {
    LLVM_DEBUG(FMADbg::match() << "  Need to compact terms in EXPR: "
                               << *this << "\n");
    LLVM_DEBUG(FMADbg::match() << "  SP before compact: " << SP << "\n");

    SP.doTermsMapping(TermsMapping.get());

    LLVM_DEBUG(FMADbg::match() << "  SP after compact: " << SP << "\n");

    // Now delete the unused terms from the vector UsedTerms.
    unsigned TermsMappingIndex = 0;
    for (auto I = UsedTerms.begin(); I != UsedTerms.end();) {
      // Terms mapping has the value ~0U if the corresponding term must be
      // removed.
      if (TermsMapping[TermsMappingIndex] == ~0U) {
        LLVM_DEBUG(FMADbg::match() << "  Remove the term from UsedTerms: " << **I);
        I = UsedTerms.erase(I);
      } else
        I++;
      TermsMappingIndex++;
    }
  }
}

void FMAExpr::replaceAllUsesOfWith(FMANode *Old, FMANode *New) {
  SmallVector<FMAExpr *, 16u> WorkList({this});
  do {
    for (auto &Opnd : WorkList.pop_back_val()->Operands)
      if (Opnd == Old)
        Opnd = New;
      else if (auto *Expr = dyn_cast<FMAExpr>(Opnd))
        WorkList.push_back(Expr);
  } while (!WorkList.empty());
}

void FMAExpr::startConsume(FMAExpr &FWSExpr,
                           SmallVectorImpl<FMATerm *> &UsedTermsBackUp) {
  // 1. Change the corresponding operands/terms with the reference to FWSExpr.
  FMARegisterTerm *FWSTerm = FWSExpr.getResultTerm();
  replaceAllUsesOfWith(FWSTerm, &FWSExpr);

  // 2. Save the list of used terms before the consumption to make it
  // possible to revert the changes in it done by this step 1.
  UsedTermsBackUp.assign(UsedTerms.begin(), UsedTerms.end());

  // 3. Remove FWSTerm from the set of used terms as it just got substituted by
  // the expression FWSExpr.
  removeFromUsedTerms(FWSTerm);

  // 4. Add terms used by 'FWSExpr' to the list of terms used by the current
  // FMA expression.
  addToUsedTerms(FWSExpr.UsedTerms);
}

void FMAExpr::cancelConsume(FMAExpr &FWSExpr,
                            SmallVectorImpl<FMATerm *> &UsedTermsBackUp) {
  FMARegisterTerm *FWSTerm = FWSExpr.getResultTerm();
  replaceAllUsesOfWith(&FWSExpr, FWSTerm);

  // Restore used terms from backup
  UsedTerms.clear();
  UsedTerms.insert(UsedTermsBackUp.begin(), UsedTermsBackUp.end());
}

void FMAExpr::commitConsume(FMAExpr &FWSExpr, bool HasOtherFMAUsers) {
  if (!HasOtherFMAUsers)
    FWSExpr.markAsFullyConsumedByKnownExpressions();

  if (!HasOtherFMAUsers && !FWSExpr.hasUnknownUsers()) {

    LLVM_DEBUG(FMADbg::fws() << "  !! Remove all USED terms from EXPR: "
                             << FWSExpr << "\n");
    // FWSExpr does not need to keep the list of used terms anymore.
    FWSExpr.UsedTerms.clear();

    // The last user of FWSExpr gets information about machine instructions
    // associated with the included/fused expression.
    ConsumedMIs.push_back(FWSExpr.getMI());
    ConsumedMIs.splice(ConsumedMIs.end(), FWSExpr.ConsumedMIs);
    if (FWSExpr.DbgValMI)
      ConsumedMIs.push_back(FWSExpr.DbgValMI);
  }
}

bool FMAExpr::consume(FMAExpr &FWSExpr, const FMAPatterns &Patterns,
                      bool HasOtherFMAUsers) {
  SmallVector<FMATerm *, 16u> UsedTermsBackUp;
  startConsume(FWSExpr, UsedTermsBackUp);

  // If the new expression is too big and cannot be be optimized, then
  // cancel the changes done at startConsume().
  if (isExprTooLarge(Patterns)) {
    cancelConsume(FWSExpr, UsedTermsBackUp);
    return false;
  }

  commitConsume(FWSExpr, HasOtherFMAUsers);

  LLVM_DEBUG(FMADbg::fws() << "  ->After consuming expr: " << *this << "\n\n");
  return true;
}

unsigned FMAExpr::getLatency(unsigned AddSubLatency, unsigned MulLatency,
                             unsigned FMALatency) const {
  unsigned MaxOperandLatency = 0;
  for (auto *Opnd : Operands)
    if (auto *Expr = dyn_cast<FMAExpr>(Opnd)) {
      unsigned OperandLatency =
          Expr->getLatency(AddSubLatency, MulLatency, FMALatency);
      MaxOperandLatency = std::max(MaxOperandLatency, OperandLatency);
    }

  if (Operands[0]->isZero() || Operands[1]->isZero())
    // This FMA is actually a term. It adds nothing to the returned latency.
    return MaxOperandLatency;

  if (Operands[0]->isOne() || Operands[1]->isOne()) {
    if (!Operands[2]->isZero())
      return MaxOperandLatency + AddSubLatency;
  } else if (Operands[2]->isZero())
    return MaxOperandLatency + MulLatency;
  else
    return MaxOperandLatency + FMALatency;

  return MaxOperandLatency;
}

FMAExpr *FMAExpr::findFWSCandidate(
    const SmallPtrSetImpl<FMAExpr *> &BadFWSCandidates,
    const SmallPtrSetImpl<FMAExpr *> &InefficientFWSCandidates,
    bool CanConsumeIfOneUser,
    SmallPtrSetImpl<FMAExpr *> &UsersSet) {
  auto *FMABB = getFMABB();

  // Walk through all terms used by the current FMA, find those that are the
  // results of other FMAs.
  for (FMATerm *Term : UsedTerms) {
    FMAExpr *TermDefFMA = FMABB->findDefiningFMA(Term);
    if (TermDefFMA == nullptr)
      continue;

    // This expression cannot be removed even if it gets consumed by another
    // one. It still may be possible to proceed and get some performance,
    // but those are very rare corner cases, which though complicate
    // the optimization alot and create bigger risks.
    if (TermDefFMA->hasUnknownUsers()) {
      LLVM_DEBUG(FMADbg::fws()
                     << "  Candidate is skipped as having unknown users: "
                     << *Term << "\n\n");
      continue;
    }

    if (BadFWSCandidates.count(TermDefFMA)) {
      LLVM_DEBUG(FMADbg::fws() << "  Candidate is skipped as BAD candidate: "
                               << *Term << "\n\n");
      continue;
    }

    if (!CanConsumeIfOneUser && InefficientFWSCandidates.count(TermDefFMA)) {
      LLVM_DEBUG(FMADbg::fws() << "  Candidate is skipped as INEFFICIENT: "
                               << *Term << "\n\n");
      continue;
    }

    // This place would be good for doing safety check verifying that it is Ok
    // to use the virtual register associated with 'Term' at the point where
    // the machine instruction associated with this FMA expression is located.
    // For FMARegisterTerm we can just use the virtual register as it is SSA
    // form and the register is virtual at this phase. For FMAMemoryTerm the
    // virtual register is not assigned yet, but it is going to be defined
    // right before the load instruction associated with FMAMemoryTerm and
    // it should be Ok to use such term in 'this' FMA expression.
    // So, no checks are performed here.

    FMABB->findKnownUsers(Term, UsersSet);

    // Skip the candidate if it fusible into 2 or more users in
    // the mode that enables FWS only candidates with 1 user.
    if (CanConsumeIfOneUser && UsersSet.size() > 1) {
      LLVM_DEBUG(FMADbg::fws()
                     << "  Candidate is skipped as HAVING MANY USERS: "
                     << *Term << "\n");
      continue;
    }

    return TermDefFMA;
  }

  return nullptr;
}

bool FMAExpr::isExprTooLarge(const FMAPatterns &Patterns) const {
  auto SP = generateSP();
  return !SP || Patterns.getLargestShape() < SP->Shape;
}

FMAExpr *FMABasicBlock::createFMA(MVT VT, MachineInstr *MI,
                                  FMARegisterTerm *ResTerm,
                                  const std::array<FMANode *, 3u> &Ops,
                                  bool MulSign, bool AddSign) {
  auto *Expr =
      new FMAExpr(VT, this, MI, ResTerm, Ops, MulSign, AddSign, FMAs.size());
  FMAs.emplace_back(Expr);
  TermToDefFMA[ResTerm] = Expr;
  return Expr;
}

void FMABasicBlock::findKnownUsers(const FMATerm *Term,
                                   SmallPtrSetImpl<FMAExpr *> &UsersSet) const {
  UsersSet.clear();
  for (auto &Expr : FMAs)
    if (Expr->isUserOf(Term))
      UsersSet.insert(Expr.get());
}

/// Loop over all of the basic blocks, performing the FMA optimization for
/// each block separately.
bool GlobalFMA::runOnMachineFunction(MachineFunction &MF) {
  bool EverMadeChangeInFunc = false;

  // Process all basic blocks.
  for (auto &MB : MF)
    if (optBasicBlock(MB))
      EverMadeChangeInFunc = true;

  LLVM_DEBUG(FMADbg::dbgs() << "********** Global FMA **********\n");
  if (EverMadeChangeInFunc) {
    LLVM_DEBUG(MF.print(FMADbg::dbgs()));
  }

  return EverMadeChangeInFunc;
}

/// Loop over all of the instructions in the basic block, optimizing
/// MUL/ADD/FMA expressions. Return true iff any changes in the machine
/// operation were done.
bool GlobalFMA::optBasicBlock(MachineBasicBlock &MBB) {
  LLVM_DEBUG(FMADbg::dbgs()
                 << "\n**** RUN FMA OPT FOR ANOTHER BASIC BLOCK ****\n");

  // Save the dump of the basic block, we may want to print it after the basic
  // block is changed by this optimization.
  std::string LogBBStr = "";
  raw_string_ostream LogBB(LogBBStr);
  LLVM_DEBUG(LogBB << MBB);

  // Find MUL/ADD/SUB/FMA/etc operations in the input machine instructions
  // and create internal FMA structures for them.
  // Exit if there are not enough optimizable expressions.
  auto FMABB = parseBasicBlock(MBB);
  if (!FMABB)
    return false;

  // Run the FMA optimization and dump the debug messages if the optimization
  // produced any changes in IR.
  bool EverMadeChangeInBB = optParsedBasicBlock(*FMABB);

  if (EverMadeChangeInBB) {
    LLVM_DEBUG(FMADbg::dbgs() << "Basic block before Global FMA opt:\n"
                              << LogBB.str() << "\n");
    LLVM_DEBUG(FMADbg::dbgs() << "\nBasic block after Global FMA opt:\n"
                              << MBB << "\n");
  }
  return EverMadeChangeInBB;
}

std::unique_ptr<FMADag>
GlobalFMA::getDagForExpression(FMAExpr &Expr, bool DoCompactTerms) const {
  LLVM_DEBUG(FMADbg::match() << "    Find DAG for FMA EXPR:\n";
             FMADbg::match() << "    " << Expr << "\n");
  auto SP = Expr.generateSP();
  if (!SP) {
    LLVM_DEBUG(FMADbg::match() << "    Could not compute SP.\n");
    return nullptr;
  }

  // The returned SP might have some opportunities for terms compact.
  // For example, for initial FMAExpr expression
  //   +ab+c-c+d
  // the returned SP may be shorter and the term 'c' is not used anymore:
  //   +ab+d
  // Let's compact the terms in SP and in 'this' FMAExpr.
  if (DoCompactTerms)
    Expr.compactTerms(*SP);

  LLVM_DEBUG(FMADbg::match() << "Computed SP is: " << *SP);
  LLVM_DEBUG(FMADbg::match() << "SHAPE: " << format_hex(SP->Shape, 2) << "\n");

  return Patterns->getDagForBestSPMatch(*SP);
}

std::unique_ptr<FMADag>
GlobalFMA::getDagForFusedExpression(FMAExpr &UserExpr, FMAExpr &FWSExpr) const {
  SmallVector<FMATerm *, 16u> UsedTermsBackUp;
  UserExpr.startConsume(FWSExpr, UsedTermsBackUp);

  auto Dag = getDagForExpression(UserExpr, false);
  UserExpr.cancelConsume(FWSExpr, UsedTermsBackUp);

  return Dag;
}

bool GlobalFMA::optParsedBasicBlock(FMABasicBlock &FMABB) {
  bool EverMadeChangeInBB = false;
  doFWS(FMABB);

  LLVM_DEBUG(FMADbg::dbgs()
                 << "\nFMA-STEP3: DO PATTERN MATCHING AND CODE-GEN:\n");
  for (auto &Expr : FMABB.getFMAs()) {
    if (!Expr->isOptimizable())
      continue;

    LLVM_DEBUG(FMADbg::dbgs() << "  Optimize FMA EXPR:\n  " << *Expr << "\n");
    auto Dag = getDagForExpression(*Expr, true);
    if (!Dag) {
      Expr->unsetLastUseMIsForRegisterTerms();
      continue;
    }

    LLVM_DEBUG(FMADbg::dbgs()
                   << "  CONGRATULATIONS! A searched DAG was found:\n    "
                   << *Dag);

    if (Expr->isMustBeOptimized()) {
      LLVM_DEBUG(FMADbg::dbgs() << "  EXPR is marked as MUST BE OPTIMIZED"
                                   " presumably by FWS module.\n");
    } else if (isDagBetterThanInitialExpr(*Dag, *Expr)) {
      // FIXME: Currently, the setting of the latency vs throughput priorities
      // is set only accordingly to the internal switch value.
      // For some target architectures (e.g. in-order targets) the throughput
      // aspects should be more important.
      // Also, this place should be updated after the latency vs throughput
      // analysis of the optimized basic block and the data dependencies analysis
      // in the optimized expression are implemented.
      LLVM_DEBUG(FMADbg::dbgs() << "  DAG IS better than the initial EXPR.\n");
    } else {
      LLVM_DEBUG(FMADbg::dbgs()
                     << "  DAG is NOT better than the initial EXPR.\n\n");
      Expr->unsetLastUseMIsForRegisterTerms();
      continue;
    }

    EverMadeChangeInBB = true;
    generateOutputIR(*Expr, *Dag);
    LLVM_DEBUG(FMADbg::dbgs() << "\n");
  }

  FMABB.setIsKilledAttributeForTerms();

  LLVM_DEBUG(FMADbg::dbgs() << "\nFMA-STEP3 IS DONE. Machine basic block IS "
                            << (EverMadeChangeInBB ? "" : "NOT ")
                            << "UPDATED.\n\n");
  return EverMadeChangeInBB;
}

FMAPerfDesc GlobalFMA::getExprPerfDesc(const FMAExpr &Expr) const {
  unsigned NumAddSub = 0;
  unsigned NumMul = 0;
  unsigned NumFMA = 0;

  SmallPtrSet<const FMAExpr *, 16u> ExprSet;
  Expr.putExprToExprSet(ExprSet);
  for (const auto *E : ExprSet) {
    if (E->getOperand(0)->isZero() || E->getOperand(1)->isZero())
      // This FMA is actually a term. It adds nothing to the returned
      // statistics.
      continue;

    if (E->getOperand(0)->isOne() || E->getOperand(1)->isOne()) {
      if (!E->getOperand(2)->isZero())
        NumAddSub++;
    } else if (E->getOperand(2)->isZero())
      NumMul++;
    else
      NumFMA++;
  }

  unsigned Latency = Expr.getLatency(AddSubLatency, MulLatency, FMALatency);
  return FMAPerfDesc(Latency, NumAddSub, NumMul, NumFMA);
}

FMAPerfDesc GlobalFMA::getDagPerfDesc(const FMADag &Dag) const {
  unsigned NumAddSub = 0;
  unsigned NumMul = 0;
  unsigned NumFMA = 0;

  unsigned NumNodes = Dag.getNumNodes();
  for (unsigned NodeInd = 0; NodeInd < NumNodes; NodeInd++) {
    bool AIsTerm, BIsTerm, CIsTerm;
    unsigned A = Dag.getOperand(NodeInd, 0, &AIsTerm);
    unsigned B = Dag.getOperand(NodeInd, 1, &BIsTerm);
    unsigned C = Dag.getOperand(NodeInd, 2, &CIsTerm);

    bool AIsZero = AIsTerm && A == FMADagCommon::TermZERO;
    bool BIsZero = BIsTerm && B == FMADagCommon::TermZERO;
    bool CIsZero = CIsTerm && C == FMADagCommon::TermZERO;

    (void)AIsZero; (void)BIsZero;
    assert((!AIsZero && !BIsZero) && "DAG has obvious inefficiencies.");

    bool AIsOne = AIsTerm && A == FMADagCommon::TermONE;
    bool BIsOne = BIsTerm && B == FMADagCommon::TermONE;

    if (AIsOne || BIsOne) {
      assert(!CIsZero && "DAG has obvious inefficiencies.");
      NumAddSub++;
      // -A - C node requires 2 operations at the code-generation phase:
      //   T0 = A + C; T1 = 0 - T0;
      // Count the additional subtract operation here.
      if (Dag.getMulSign(NodeInd) && Dag.getAddSign(NodeInd))
        NumAddSub++;
    } else if (CIsZero) {
      // A*B requires 1 MUL operation at the code-generation phase.
      // -A*B requires 1 FMA operation: -A*B+0.
      if (Dag.getMulSign(NodeInd))
        NumFMA++;
      else
        NumMul++;
    } else
      NumFMA++;
  }
  unsigned Latency = Dag.getLatency(MulLatency, AddSubLatency, FMALatency);
  return FMAPerfDesc(Latency, NumAddSub, NumMul, NumFMA);
}

bool GlobalFMA::isDagBetterThanInitialExpr(const FMADag &Dag,
                                           const FMAExpr &Expr) const {
  FMAPerfDesc DagDesc = getDagPerfDesc(Dag);
  FMAPerfDesc ExprDesc = getExprPerfDesc(Expr);

  LLVM_DEBUG(FMADbg::dbgs() << "  Compare DAG and initial EXPR:\n"
                            << "    DAG  has: " << DagDesc << "\n"
                            << "    EXPR has: " << ExprDesc << "\n");

  // If the internal switch requires FMAs, then just return true.
  // This code is placed after the printings of the DAG/Expr properties
  // as the last may be interesting even if FMAs are forced.
  if (Control.ForceFMAs)
    return true;

  return DagDesc.isBetterThan(ExprDesc,
                              Control.TuneForLatency,
                              Control.TuneForThroughput);
}

bool GlobalFMA::isSafeToFuse(
    const FMATerm *T,
    const SmallPtrSetImpl<const FMAExpr *> &GoodUsersSet,
    const SmallPtrSetImpl<const FMAExpr *> &BadUsersSet) const {

  if (BadUsersSet.empty())
    return true;

  // To check if fusing of T into some/all of GoodUsersSet is safe,
  // we need to check that FMA expressions from GoodUsersSet do not
  // intersect (as result of FWS) with FMA expressions from BadUsersSet.
  //     T   A       T - is the candidate for fusing it may be replaced
  //      \ /            with some expression during FWS.
  //   Z   N   X     U - is immediate user of T, included into "good" expr G
  //    \ / \ /          and "bad" expr B.
  //     G   B       Replacing T would cause changes in both G and B.
  // 1) For each top-user G from GoodUsersSet find and add immediate users
  //    of T to GoodImmediateUsersSet.
  // 2) For each top-user B from BadUsersSet find and add immediate users
  //    of T to NotGoodImmediateUsersSet.
  // 3) Check if there are immediate users IU from GoodImmediateUsersSet
  //    that are also in BadImmediateUsersSet.
  //    If there are such, then fusing is unsafe as changing a good user
  //    would cause unwanted changes in a bad user.
  SmallPtrSet<const FMAExpr *, 16u> GoodImmediateUsersSet;
  SmallPtrSet<const FMAExpr *, 16u> NotGoodImmediateUsersSet;
  for (auto *G : GoodUsersSet)
    G->insertImmediateUsersOfTo(T, GoodImmediateUsersSet);
  for (auto *B : BadUsersSet)
    B->insertImmediateUsersOfTo(T, NotGoodImmediateUsersSet);

  for (auto *U : GoodImmediateUsersSet)
    if (NotGoodImmediateUsersSet.count(U))
      return false;

  return true;
}

bool GlobalFMA::doFWSAndConsumeIfProfitable(
    FMAExpr &FWSExpr, const SmallPtrSetImpl<FMAExpr *> &UsersSet,
    SmallPtrSetImpl<FMAExpr *> &BadUsersSet,
    SmallPtrSetImpl<FMAExpr *> &InefficientUsersSet, bool CanConsumeIfOneUser) {

  SmallPtrSet<FMAExpr *, 16u> NeutralUsersSet;
  SmallPtrSet<FMAExpr *, 16u> GoodUsersSet;
  unsigned NumOtherFMAUsers = UsersSet.size();

  int NumAdditionalAddSub = 0;
  int NumAdditionalMul = 0;
  int NumAdditionalFMA = 0;

  bool Consumed = false;

  // CanConsumeIfOneUser means a special mode is ON, on which
  // only consumptions of expressions with 1 user are done even
  // they may look inefficient at this moment.
  if (CanConsumeIfOneUser && UsersSet.size() > 1)
    return false;

  int UserIndex = 0;
  for (auto *UserExpr : UsersSet) {
    UserIndex++;
    LLVM_DEBUG(FMADbg::fws() << "  Analyze user #" << UserIndex << "...\n";);
    if (BadUsersSet.count(UserExpr) ||
        (!CanConsumeIfOneUser && InefficientUsersSet.count(UserExpr)))
      continue;

    auto OriginalDag = getDagForExpression(*UserExpr, false);
    if (!OriginalDag) {
      // User cannot consume anything as it cannot have a DAG even without
      // fusing UserExpr and FWSExpr.
      LLVM_DEBUG(FMADbg::fws() << "  User is marked as BAD because "
                                  "getDagForExpression() returned NULL: "
                               << *UserExpr << "\n");
      BadUsersSet.insert(UserExpr);
      continue;
    }

    auto FusedDag = getDagForFusedExpression(*UserExpr, FWSExpr);
    if (!FusedDag) {
      // DAG could not be created for the fused expression (UserExpr + FWSExpr).
      LLVM_DEBUG(FMADbg::fws() << "  User is marked as BAD because "
                                  "getDagForFusedExpression() returned NULL: "
                               << *UserExpr << "\n");
      BadUsersSet.insert(UserExpr);
      continue;
    }

    FMAPerfDesc OriginalDesc = getDagPerfDesc(*OriginalDag);
    FMAPerfDesc FusedDesc = getDagPerfDesc(*FusedDag);
    LLVM_DEBUG(FMADbg::fws() << "  DAG before FWS: " << *OriginalDag;
               FMADbg::fws() << "  " << OriginalDesc << "\n");
    LLVM_DEBUG(FMADbg::fws() << "  DAG after FWS: " << *FusedDag;
               FMADbg::fws() << "  " << FusedDesc << "\n");
    // TODO: vklochko: The 2nd and 3rd arguments of the next call must depend
    // on the existence of recurrent terms used by the fused expression.
    // Currently, such analysis are not available.
    if (!FusedDesc.isBetterThan(OriginalDesc, false /*TuneForLatency */,
                                false /*TuneForThroughput*/)) {
      // UserExpr and FWSExpr potentially can be fused, but the DAG for
      // the fused expression does NOT seem better than the DAG of UserExpr.
      NeutralUsersSet.insert(UserExpr);
      NumAdditionalAddSub +=
          FusedDesc.getNumAddSub() - OriginalDesc.getNumAddSub();
      NumAdditionalMul += FusedDesc.getNumMul() - OriginalDesc.getNumMul();
      NumAdditionalFMA += FusedDesc.getNumFMA() - OriginalDesc.getNumFMA();
      LLVM_DEBUG(FMADbg::fws()
                     << "  Fusing is NOT efficient if keep FWS candidate. "
                        "Add the candidate to neutral users now.\n");
    } else {
      // If the fused DAG is better than the original DAG, then just
      // fuse UserExpr and FWSExpr.
      // Example:
      //    t1 = a-b; // has 2 users: t2 and some other-unknown.
      //    t2 = a-t1;
      // -->
      //    t1 = a-b;
      //    t2 = a-(a-b); // Better because it is equal to t2 = b;
      //
      // It is risky to perform the fusing right now and may lead to errors if
      // there are N > 1 users and both (a) and (b) are true:
      // a) some of users have got common subexpressions as result of FWS
      // b) fusing to some of users is not efficient.
      // In such cases fusing with a good user here would cause
      // unwanted/inefficient fusing with another user, but the real problem
      // is that the latter will not get terms used by the consumed expression
      // to the list of the consumer expression.
      // Thus we only register such good fuse opportunities now and do
      // the actual fusing later if it is safe.
      GoodUsersSet.insert(UserExpr);
      LLVM_DEBUG(FMADbg::fws() << "  Fusing seems efficient. Add to good users"
                                  " now for fusing later.\n");
    }
  }

  if (!GoodUsersSet.empty()) {
    bool SafeToFuse = UsersSet.size() == 1;
    if (!SafeToFuse) {
      // Check if is safe to fuse FWSExpr with GoodUsersSet expressions
      // i.e. that it would not cause unexpected changes in other not good
      // users: NotGoodUsersSet = UsersSet - GoodUsersSet, which also
      // must be equal to BadUsersSet + InefficientUsersSet + NeutralUsersSet.
      SmallPtrSet<const FMAExpr *, 16u> NotGoodUsersSet;
      NotGoodUsersSet.insert(BadUsersSet.begin(), BadUsersSet.end());
      NotGoodUsersSet.insert(NeutralUsersSet.begin(), NeutralUsersSet.end());
      NotGoodUsersSet.insert(InefficientUsersSet.begin(),
                             InefficientUsersSet.end());
      assert((NotGoodUsersSet.size() + GoodUsersSet.size() == UsersSet.size())
             && "Unexpected user sets.");
      SmallPtrSet<const FMAExpr *, 16u>
          CGoodUsersSet(GoodUsersSet.begin(), GoodUsersSet.end());
      SafeToFuse = isSafeToFuse(FWSExpr.getResultTerm(),
                                CGoodUsersSet, NotGoodUsersSet);
    }
    if (SafeToFuse) {
      for (auto *E : GoodUsersSet) {
        LLVM_DEBUG(FMADbg::fws() << "  Fuse into a good candidate now: "
                                  << *E << "\n";);
        NumOtherFMAUsers--;
        Consumed = E->consume(FWSExpr, *Patterns, NumOtherFMAUsers != 0);
        assert(Consumed && "FWS/consume must be possible.");
      }
    } else {
      NeutralUsersSet.insert(GoodUsersSet.begin(), GoodUsersSet.end());
      LLVM_DEBUG(FMADbg::fws() << "  Fusing in the following good candidates "
                                   "causes fusing in other users. "
                                   "Put them to neutral users now:\n";);
      LLVM_DEBUG(for (auto *E : GoodUsersSet) {FMADbg::fws() << *E << "\n";});
    }
  }

  // If there is nothing more to fuse, then just exit.
  if (NeutralUsersSet.empty())
    return Consumed;

  // If the FWS expression cannot be removed due to existing users, then exit.
  if (FWSExpr.hasUnknownUsers() || !BadUsersSet.empty()) {
    LLVM_DEBUG(FMADbg::fws()
                   << "  Fusing neutral users is inefficient or not doable.\n");
    InefficientUsersSet.insert(NeutralUsersSet.begin(), NeutralUsersSet.end());
    return Consumed;
  }

  // There were some UserExpr that could be fused with FWSExpr, but the fused
  // expressions are not better than the original UserExpr.
  //
  // Ok, if FWSExpr can be consumed by all such UserExpr expressions, then
  // FWSExpr can be just removed. It makes sense doing it ONLY if the benefit
  // from removing FWSExpr is bigger than the loses from fusing it to all users.
  // Example:
  //    t1 = a*b; // has 2 users: t2 and t3.
  //    t2 = t1+c;
  //    t3 = t1-c;
  // -->
  //    t2 = a*b+c;
  //    t3 = a*b-c;
  // t2 and t3 expressions did not get better (and did not get worse),
  // but consumption helped to eliminate the expression t1 = a*b.
  //
  // TODO: vklochko: The latency aspect is ignored for a while here:
  // 1. There are no loop dependencies analysis at this moment.
  // 2. Without loop dependencies, it is extremely hard to imagine a situation
  //    when latency aspect would have priority over throughput.
  int NumAdditionalOperations =
        NumAdditionalAddSub + NumAdditionalMul + NumAdditionalFMA;
  FMAPerfDesc FWSExprDesc = getExprPerfDesc(FWSExpr);

  if (NumAdditionalOperations < (int)FWSExprDesc.getNumOperations() ||
      (NumAdditionalOperations == (int)FWSExprDesc.getNumOperations() &&
       NumAdditionalFMA <= (int)FWSExprDesc.getNumFMA())) {
    // Fusing produces same or better expressions because one of these is true:
    // a) better: fused exprs have less operations than the original
    //    expr(s) + fws expr;
    // b) same-or-better: fusing keeps the smae number of operations,
    //    but it does not increase the number of heavy FMAs.
    ;
  }
  else if (CanConsumeIfOneUser && NeutralUsersSet.size() == 1) {
    // It is already known that the fusing FWSExpr and its only user does not
    // seem efficient (otherwise, the user of FWSExpr would not be in
    // NeutralUsersSet) and fusing would happen earlier.
    // The fusing still may be useful as it may give more opportunities for
    // further and more efficient fusing.
    auto User = *(NeutralUsersSet.begin());
    if (FWSExpr.isMustBeOptimized() || User->isMustBeOptimized()) {
      // If one of fused expressions is marked as MustBeOptimized,
      // then do not fuse! Otherwise, the result must be marked
      // as MustBeOptimized too, which cannot be cancelled and may lead
      // to to inefficient code-generation. For such cases it is better to
      // simply move such user to BadUsersSet as adding it
      // to InefficientUsersSet will let the FWSExpr be a candidate for FWS
      // again and again, which leads to endless loop during compilation.
      LLVM_DEBUG(FMADbg::fws() << "  Fusing into neutral users is inefficient "
                                  "or not doable.\n");
      BadUsersSet.insert(NeutralUsersSet.begin(), NeutralUsersSet.end());
      return Consumed;
    }
    // If reached this point, then fusing is approved.
  } else {
    // Do not fuse if the fused expression is not any better than the
    // original expression(s) + FWSExpr.
    InefficientUsersSet.insert(NeutralUsersSet.begin(), NeutralUsersSet.end());
    return Consumed;
  }

  // Fusing into more than 1 user already has been evaluated as efficient.
  // Thus, all users must be re-generated/optimized at the code-generation
  // phase. Otherwise, if re-generate/optimize only some of users
  // (not all of users), then:
  // a) It is an error as it is assumed at later stages that when one
  //    expression is optimized, then all previous MIs, from which FMA
  //    expression was composed, must be deleted/replaced by new MIs.
  //    This would remove MIs composing FWSExpr. Oops...
  // b) If handle (a) above somehow, then it would leave MIs creating
  //    FWSExpr, which does not make sense from efficiency point of view
  //    as the whole 'fusing  FWSExpr into several users' makes sense
  //    mostly/only because the original FWSExpr gets removed.
  // Also, if FWSExpr is already marked as MustBeOptimzed, then
  // the new fused expression(s) must get that attribute too.
  // Otherwise, there is some chance that the new fused expression
  // is not optimized/re-generated, which means FWSExpr also not
  // optimized/re-generated.
  LLVM_DEBUG(FMADbg::fws() << "  Fusing into " << NumOtherFMAUsers
                           << " neutral user(s) seems efficient.\n");
  bool MarkAsMustBeOptimized =
      NumOtherFMAUsers > 1 || FWSExpr.isMustBeOptimized();
  for (auto E : NeutralUsersSet) {
    LLVM_DEBUG(FMADbg::fws() << "  Fuse with the neutral user #"
                             << NumOtherFMAUsers << ": " << *E << "\n");
    NumOtherFMAUsers--;
    Consumed = E->consume(FWSExpr, *Patterns, NumOtherFMAUsers != 0);
    assert(Consumed && "FWS/consume must be possible.");
    if (MarkAsMustBeOptimized)
      E->markAsMustBeOptimized();
  }
  FWSExpr.markAsFullyConsumedByKnownExpressions();
  return Consumed;
}

void GlobalFMA::doFWS(FMABasicBlock &FMABB) {

  LLVM_DEBUG(FMADbg::dbgs() << "\nFMA-STEP2: DO FWS:\n");

  using FMAExprSet = SmallPtrSet<FMAExpr *, 8u>;
  SmallVector<FMAExprSet, 8u> BadFWSCandidates(FMABB.getFMAs().size());
  SmallVector<FMAExprSet, 8u> InefficientFWSCandidates(FMABB.getFMAs().size());

  bool Consumed = true;
  bool CanConsumeIfOneUser = false;
  while (Consumed) {
    Consumed = false;

    const auto &FMAs = FMABB.getFMAs();
    for (unsigned ExprIndex = 0; ExprIndex < FMAs.size(); ExprIndex++) {
      auto &Expr = FMAs[ExprIndex];
      if (Expr->isFullyConsumedByKnownExpressions())
        continue;

      LLVM_DEBUG(FMADbg::fws() << "Find terms that could be "
                                  "substituted by expressions in: "
                               << *Expr << "\n");
      SmallPtrSet<FMAExpr *, 16u> UsersSet;
      FMAExpr *FWSExpr = Expr->findFWSCandidate(
          BadFWSCandidates[ExprIndex], InefficientFWSCandidates[ExprIndex],
          CanConsumeIfOneUser, UsersSet);
      int FWSExprCandNum = 0;
      LLVM_DEBUG(if (!FWSExpr) FMADbg::fws() << "\n";);
      while (FWSExpr) {
        LLVM_DEBUG(FMADbg::fws() << "Found a FWS candidate(#"
                                 << FWSExprCandNum << "): " << *FWSExpr
                                 << "  fusible to " << UsersSet.size()
                                 << " user(s):\n");
        FWSExprCandNum++;
        LLVM_DEBUG(
            for (auto E : UsersSet) { FMADbg::fws() << "    " << *E << "\n"; });
        SmallPtrSet<FMAExpr *, 16u> BadUsersSet;
        SmallPtrSet<FMAExpr *, 16u> InefficientUsersSet;

        if (doFWSAndConsumeIfProfitable(*FWSExpr, UsersSet,
                                        BadUsersSet, InefficientUsersSet,
                                        CanConsumeIfOneUser)) {
          Consumed = true;

          // The expressions from UsersSet got changed, thus the candidates
          // previously registered as bad for them may become good now.
          for (auto *E : UsersSet) {
            unsigned EIndex = E->getIndexInBB();
            BadFWSCandidates[EIndex].clear();
            InefficientFWSCandidates[EIndex].clear();
            // Also each other expression referencing E from UsersSet as bad
            // needs to clean E from it's bad list.
            for (unsigned I = 0; I < FMABB.getFMAs().size(); I++) {
              BadFWSCandidates[I].erase(E);
              InefficientFWSCandidates[I].erase(E);
            }
          }
        }

        for (auto *E : BadUsersSet)
          BadFWSCandidates[E->getIndexInBB()].insert(FWSExpr);
        for (auto *E : InefficientUsersSet)
          InefficientFWSCandidates[E->getIndexInBB()].insert(FWSExpr);

        FWSExpr = Expr->findFWSCandidate(BadFWSCandidates[ExprIndex],
                                         InefficientFWSCandidates[ExprIndex],
                                         CanConsumeIfOneUser, UsersSet);
      }
    }

    if (!Consumed && !CanConsumeIfOneUser) {
      // Give temporary permit to do FWS for expressions having only one use
      // if the fused expression is not worse than two separate expressions
      // before fusing.
      LLVM_DEBUG(FMADbg::fws() << "!! Now set CanConsumeIfOneUser "
                                  "to TRUE and repeat.\n\n");
      CanConsumeIfOneUser = true;
      Consumed = true;
    } else if (Consumed && CanConsumeIfOneUser) {
      LLVM_DEBUG(FMADbg::fws() << "!! Now set CanConsumeIfOneUser "
                                  "to FALSE to give priority to cases with "
                                  "clean bonuses.\n\n");
      // 'CanConsumeIfOneUser' was a temporary permit to do FWS in cases where
      // it potentially gives more opportunities for doing beneficial FWS
      // cases. Revoke the permit and try to do efficient FWS cases now.
      CanConsumeIfOneUser = false;
    }
  } // end while (Consumed)
  LLVM_DEBUG(FMADbg::dbgs() << "\nFMA-STEP2 DONE. FMA basic block after FWS:\n"
                            << FMABB);
}
