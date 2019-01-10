//===----------- HIRLoopResource.cpp - Computes loop resources ------------===//
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
// This file implements the loop resource analysis pass.
//
//===----------------------------------------------------------------------===//

// TODO:
// 1. Add complex datatype support when available.
// 2. Add some platform specific support e.g. Atom floating to int
//    ratio is different from Haswell.
// 3. Think about if we want to assign specific cost to int ops such as sdiv.
// 4. Use branch probability information to denote paths of if's and switch.
// 5. Handle more generic types in HLInst such as Vector/Struct types.
// 6. Call instructions might need special handling based on arguments.

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopResource.h"

#include "llvm/ADT/SmallSet.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/TargetTransformInfo.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Passes.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/BlobUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/CanonExprUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"

#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/MathExtras.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "hir-loop-resource"

static cl::opt<bool> PrintTotalResource(
    "hir-print-total-resource", cl::init(false), cl::Hidden,
    cl::desc("Prints total loop resource instead of self loop resource"));

FunctionPass *llvm::createHIRLoopResourceWrapperPass() {
  return new HIRLoopResourceWrapperPass();
}

AnalysisKey HIRLoopResourceAnalysis::Key;
HIRLoopResource HIRLoopResourceAnalysis::run(Function &F,
                                             FunctionAnalysisManager &AM) {
  return HIRLoopResource(AM.getResult<HIRFrameworkAnalysis>(F),
                         AM.getResult<LoopAnalysis>(F),
                         AM.getResult<TargetIRAnalysis>(F));
}

char HIRLoopResourceWrapperPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRLoopResourceWrapperPass, "hir-loop-resource",
                      "Loop Resource Analysis", false, true)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_END(HIRLoopResourceWrapperPass, "hir-loop-resource",
                    "Loop Resource Analysis", false, true)

void HIRLoopResourceWrapperPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<HIRFrameworkWrapperPass>();
  AU.addRequired<LoopInfoWrapperPass>();
  AU.addRequired<TargetTransformInfoWrapperPass>();
}

bool HIRLoopResourceWrapperPass::runOnFunction(Function &F) {
  HLR.reset(new HIRLoopResource(
      getAnalysis<HIRFrameworkWrapperPass>().getHIR(),
      getAnalysis<LoopInfoWrapperPass>().getLoopInfo(),
      getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F)));
  // This is an on-demand analysis, so we don't perform any analysis here.
  return false;
}

void HIRLoopResourceWrapperPass::releaseMemory() { HLR.reset(); }

struct LoopResourceInfo::LoopResourceVisitor final : public HLNodeVisitorBase {
  HIRLoopResource &HLR;
  const TargetTransformInfo &TTI;
  const HLLoop *Lp;
  LoopResourceInfo *SelfLRI;
  LoopResourceInfo *ChildrenLRI;
  SmallSet<unsigned, 8> VersionedLoops;

  struct BlobCostEvaluator;

  LoopResourceVisitor(HIRLoopResource &HLR, const HLLoop *Lp,
                      LoopResourceInfo *SelfLRI, LoopResourceInfo *ChildrenLRI)
      : HLR(HLR), TTI(HLR.getTTI()), Lp(Lp), SelfLRI(SelfLRI),
        ChildrenLRI(ChildrenLRI) {
    assert((SelfLRI || ChildrenLRI) &&
           "At least one of self/children resource should be present!");
  }

  /// Sets the threshold cost for non-memory expensive instructions.
  /// Memory related instructions should not reach here.
  static unsigned getNormalizedCost(int Cost) {
    return std::min(Cost, (int)LoopResourceInfo::OperationCost::ExpensiveOp);
  }

  /// Main entry function to compute loop resource.
  void compute();

  /// Accounts for \p Num integer or FP predicates in self resource.
  void addPredicateOps(Type *Ty, unsigned Num);

  /// Returns the cost of the underlying operation of HLInst. For example, for
  /// binary operators the cost of add, mul, div operation etc is returned. For
  /// cast instructions, the cost of the cast is returned.
  /// This does not include DDRef operands' cost.
  unsigned getOperationCost(const HLInst *HInst) const;

  /// Evaluates cost of blob.
  void visit(unsigned BlobIndex);

  /// Helper to add cost of cast operation in CE.
  void addCastCost(const CanonExpr *CE);

  /// Helper to add cost of denominator in CE.
  void addDenominatorCost(const CanonExpr *CE);

  /// Evaluates cost of CE.
  void visit(const CanonExpr *CE);

  /// Evaluates cost of Ref;
  void visit(const RegDDRef *Ref);

  // Ignore gotos/labels.
  void visit(const HLNode *Node) {}
  void postVisit(const HLNode *Node) {}

  bool visit(const HLDDNode *Node);
  void visit(const HLInst *HInst);

  void visit(const HLIf *If);
  void visit(const HLLoop *Lp);
};

struct LoopResourceInfo::LoopResourceVisitor::BlobCostEvaluator
    : public SCEVVisitor<BlobCostEvaluator> {
  const LoopResourceVisitor &LRV;

  BlobCostEvaluator(LoopResourceVisitor &LRV) : LRV(LRV) {}

  void visitConstant(const SCEVConstant *Constant) {}

  void visitTruncateExpr(const SCEVTruncateExpr *TruncBlob) {
    unsigned Cost = LRV.getNormalizedCost(
        LRV.TTI.getOperationCost(Instruction::Trunc, TruncBlob->getType(),
                                 TruncBlob->getOperand()->getType()));
    LRV.SelfLRI->addIntOps(Cost);

    visit(TruncBlob->getOperand());
  }

  void visitZeroExtendExpr(const SCEVZeroExtendExpr *ZExtBlob) {
    unsigned Cost = LRV.getNormalizedCost(
        LRV.TTI.getOperationCost(Instruction::ZExt, ZExtBlob->getType(),
                                 ZExtBlob->getOperand()->getType()));
    LRV.SelfLRI->addIntOps(Cost);

    visit(ZExtBlob->getOperand());
  }

  void visitSignExtendExpr(const SCEVSignExtendExpr *SExtBlob) {
    unsigned Cost = LRV.getNormalizedCost(
        LRV.TTI.getOperationCost(Instruction::SExt, SExtBlob->getType(),
                                 SExtBlob->getOperand()->getType()));
    LRV.SelfLRI->addIntOps(Cost);

    visit(SExtBlob->getOperand());
  }

  void visitNAryExpr(const SCEVNAryExpr *NAryExpr) {
    for (unsigned I = 0, E = NAryExpr->getNumOperands(); I < E; ++I) {
      visit(NAryExpr->getOperand(I));
    }
  }

  void visitAddExpr(const SCEVAddExpr *AddBlob) {
    unsigned Cost = LRV.getNormalizedCost(
        LRV.TTI.getOperationCost(Instruction::Add, AddBlob->getType()));
    LRV.SelfLRI->addIntOps(Cost, AddBlob->getNumOperands() - 1);

    visitNAryExpr(cast<SCEVNAryExpr>(AddBlob));
  }

  void visitMulExpr(const SCEVMulExpr *MulBlob) {
    unsigned Cost = LRV.getNormalizedCost(
        LRV.TTI.getOperationCost(Instruction::Mul, MulBlob->getType()));
    LRV.SelfLRI->addIntOps(Cost, MulBlob->getNumOperands() - 1);

    visitNAryExpr(cast<SCEVNAryExpr>(MulBlob));
  }

  void visitUDivExpr(const SCEVUDivExpr *UDivBlob) {
    unsigned OpCode;
    auto ConstDiv = dyn_cast<SCEVConstant>(UDivBlob->getRHS());

    if (ConstDiv && ConstDiv->getAPInt().isPowerOf2()) {
      OpCode = Instruction::LShr;
    } else {
      OpCode = Instruction::UDiv;
    }

    unsigned Cost = LRV.getNormalizedCost(
        LRV.TTI.getOperationCost(OpCode, UDivBlob->getType()));

    LRV.SelfLRI->addIntOps(Cost);

    visit(UDivBlob->getLHS());
    visit(UDivBlob->getRHS());
  }

  void visitAddRecExpr(const SCEVAddRecExpr *AddRec) {
    llvm_unreachable("Found AddRec blob!");
  }

  void visitSMaxExpr(const SCEVSMaxExpr *SMaxBlob) {
    unsigned Cost = LRV.getNormalizedCost(
        LRV.TTI.getOperationCost(Instruction::ICmp, SMaxBlob->getType()));
    LRV.SelfLRI->addIntOps(Cost, SMaxBlob->getNumOperands() - 1);

    visitNAryExpr(cast<SCEVNAryExpr>(SMaxBlob));
  }

  void visitUMaxExpr(const SCEVUMaxExpr *UMaxBlob) {
    unsigned Cost = LRV.getNormalizedCost(
        LRV.TTI.getOperationCost(Instruction::ICmp, UMaxBlob->getType()));
    LRV.SelfLRI->addIntOps(Cost, UMaxBlob->getNumOperands() - 1);

    visitNAryExpr(cast<SCEVNAryExpr>(UMaxBlob));
  }

  void visitUnknown(const SCEVUnknown *Unknown) {}

  void visitCouldNotCompute(const SCEVCouldNotCompute *CNC) {
    llvm_unreachable("Found could not compute!!");
  }
};

void LoopResourceInfo::LoopResourceVisitor::visit(unsigned BlobIndex) {
  auto Blob = Lp->getBlobUtils().getBlob(BlobIndex);

  BlobCostEvaluator BCE(*this);
  BCE.visit(Blob);
}

void LoopResourceInfo::LoopResourceVisitor::addCastCost(const CanonExpr *CE) {
  auto SrcTy = CE->getSrcType();
  auto DestTy = CE->getDestType();

  if (SrcTy == DestTy) {
    return;
  }

  unsigned OpCode = CE->isTrunc()
                        ? Instruction::Trunc
                        : CE->isSExt() ? Instruction::SExt : Instruction::ZExt;
  unsigned Cost =
      getNormalizedCost(TTI.getOperationCost(OpCode, DestTy, SrcTy));

  SelfLRI->addIntOps(Cost);
}

void LoopResourceInfo::LoopResourceVisitor::addDenominatorCost(
    const CanonExpr *CE) {
  auto Denom = CE->getDenominator();

  if (Denom == 1) {
    return;
  }

  unsigned OpCode =
      CE->isSignedDiv()
          ? Instruction::SDiv
          : isPowerOf2_64(Denom) ? Instruction::LShr : Instruction::UDiv;
  unsigned Cost =
      getNormalizedCost(TTI.getOperationCost(OpCode, CE->getSrcType()));

  SelfLRI->addIntOps(Cost);
}

void LoopResourceInfo::LoopResourceVisitor::visit(const CanonExpr *CE) {
  auto SrcTy = CE->getSrcType();

  unsigned AddCost =
      getNormalizedCost(TTI.getOperationCost(Instruction::Add, SrcTy));
  unsigned MulCost =
      getNormalizedCost(TTI.getOperationCost(Instruction::Mul, SrcTy));
  unsigned ShlCost =
      getNormalizedCost(TTI.getOperationCost(Instruction::Shl, SrcTy));

  bool First = true;
  bool FoundAdditive = false;

  for (auto IV = CE->iv_begin(), E = CE->iv_end(); IV != E; ++IV) {
    unsigned BlobIndex;
    int64_t Coeff;

    CE->getIVCoeff(IV, &BlobIndex, &Coeff);

    if (!Coeff) {
      continue;
    }

    if (Coeff != 1) {
      isPowerOf2_64(Coeff) ? SelfLRI->addIntOps(ShlCost)
                           : SelfLRI->addIntOps(MulCost);
    }

    if (BlobIndex != InvalidBlobIndex) {
      visit(BlobIndex);
      SelfLRI->addIntOps(MulCost);
    }

    if (First) {
      FoundAdditive = true;
      First = false;
    } else {
      SelfLRI->addIntOps(AddCost);
    }
  }

  First = true;
  for (auto Blob = CE->blob_begin(), E = CE->blob_end(); Blob != E; ++Blob) {
    if (First) {
      if (FoundAdditive) {
        SelfLRI->addIntOps(AddCost);
      }
      FoundAdditive = true;
      First = false;
    } else {
      SelfLRI->addIntOps(AddCost);
    }

    visit(Blob->Index);

    if (Blob->Coeff != 1) {
      isPowerOf2_64(Blob->Coeff) ? SelfLRI->addIntOps(ShlCost)
                                 : SelfLRI->addIntOps(MulCost);
    }
  }

  if (FoundAdditive && CE->getConstant() != 0) {
    SelfLRI->addIntOps(AddCost);
  }

  addDenominatorCost(CE);

  addCastCost(CE);
}

void LoopResourceInfo::LoopResourceVisitor::visit(const RegDDRef *Ref) {

  if (Ref->isMemRef()) {
    bool IsFloat = Ref->getDestType()->isFPOrFPVectorTy();

    if (Ref->isLval()) {
      IsFloat ? ++SelfLRI->FPMemWrites : ++SelfLRI->IntMemWrites;
    } else {
      IsFloat ? ++SelfLRI->FPMemReads : ++SelfLRI->IntMemReads;
    }
  }
  // Ignore temp lvals.
  else if (Ref->isLval()) {
    return;
  }

  bool HasGEPInfo = Ref->hasGEPInfo();
  unsigned DimNum = 1;

  for (auto CEI = Ref->canon_begin(), CEE = Ref->canon_end(); CEI != CEE;
       ++CEI, ++DimNum) {
    visit(*CEI);

    if (HasGEPInfo && !(*CEI)->isZero()) {
      // Add cost for base + offset.
      unsigned Cost = getNormalizedCost(
          TTI.getOperationCost(Instruction::Add, (*CEI)->getDestType()));
      SelfLRI->addIntOps(Cost);

      auto Stride = Ref->getDimensionConstStride(DimNum);

      // Add cost of stride multiplication.
      Cost = getNormalizedCost(TTI.getOperationCost(
          isPowerOf2_64(Stride) ? Instruction::Shl : Instruction::Mul,
          (*CEI)->getDestType()));
      SelfLRI->addIntOps(Cost);
    }
  }
}

bool LoopResourceInfo::LoopResourceVisitor::visit(const HLDDNode *Node) {

  if (!SelfLRI) {
    return false;
  }

  for (auto RefIt = Node->op_ddref_begin(), E = Node->op_ddref_end();
       RefIt != E; ++RefIt) {
    visit(*RefIt);
  }

  return true;
}

unsigned LoopResourceInfo::LoopResourceVisitor::getOperationCost(
    const HLInst *HInst) const {

  auto Inst = HInst->getLLVMInstruction();
  auto InstTy = Inst->getType();
  Type *OpTy = nullptr;

  int Cost = LoopResourceInfo::OperationCost::BasicOp;

  if (isa<BinaryOperator>(Inst)) {
    Cost = TTI.getOperationCost(Inst->getOpcode(), InstTy);

  } else if (auto CInst = dyn_cast<CastInst>(Inst)) {
    OpTy = CInst->getSrcTy();
    Cost = TTI.getOperationCost(CInst->getOpcode(), InstTy, OpTy);

  } else if (isa<CmpInst>(Inst) || isa<SelectInst>(Inst)) {
    PredicateTy Pred = HInst->getPredicate();
    Cost = TTI.getOperationCost(
        CmpInst::isIntPredicate(Pred) ? Instruction::ICmp : Instruction::FCmp,
        InstTy);

  } else if (Inst->mayReadOrWriteMemory()) {
    return LoopResourceInfo::OperationCost::MemOp;
  }

  return getNormalizedCost(Cost);
}

void LoopResourceInfo::LoopResourceVisitor::visit(const HLInst *HInst) {

  // Return if we do not care for this node.
  if (!visit(cast<HLDDNode>(HInst))) {
    return;
  }

  // Ignore copy operation.
  if (HInst->isCopyInst()) {
    return;
  }

  auto Inst = HInst->getLLVMInstruction();

  // Load/store/GEP accounted for in DDRefs.
  if (isa<LoadInst>(Inst) || isa<StoreInst>(Inst) ||
      isa<GEPOrSubsOperator>(Inst)) {
    return;
  }

  unsigned Cost = getOperationCost(HInst);
  bool IsFP = false;

  if (isa<CmpInst>(Inst) || isa<SelectInst>(Inst)) {
    IsFP = CmpInst::isFPPredicate(HInst->getPredicate());
  } else if (HInst->hasLval()) {
    IsFP = HInst->getLvalDDRef()->getDestType()->isFPOrFPVectorTy();
  }

  IsFP ? SelfLRI->addFPOps(Cost) : SelfLRI->addIntOps(Cost);
}

void LoopResourceInfo::LoopResourceVisitor::addPredicateOps(Type *Ty,
                                                            unsigned Num) {
  unsigned Cost = 0;

  // Add number of logical operations as IntOps.
  if (Num > 1) {
    Cost = getNormalizedCost(TTI.getOperationCost(Instruction::And, Ty));
    SelfLRI->addIntOps(Cost, Num - 1);
  }

  // Add number of compares.
  if (Ty->isFPOrFPVectorTy()) {
    Cost = getNormalizedCost(TTI.getOperationCost(Instruction::FCmp, Ty));
    SelfLRI->addFPOps(Cost, Num);

  } else {
    Cost = getNormalizedCost(TTI.getOperationCost(Instruction::ICmp, Ty));
    SelfLRI->addIntOps(Cost, Num);
  }
}

void LoopResourceInfo::LoopResourceVisitor::visit(const HLIf *If) {

  // Return if we do not care for this node.
  if (!visit(cast<HLDDNode>(If))) {
    return;
  }

  // TODO: use weighed average of then and else children?

  // Capture the number of compare and logical operations.
  addPredicateOps((*If->op_ddref_begin())->getDestType(),
                  If->getNumPredicates());
}

void LoopResourceInfo::LoopResourceVisitor::visit(const HLLoop *Lp) {

  if (SelfLRI) {
    auto MVTag = Lp->getMVTag();

    // We only need to consider one of the multiversioned children loops when
    // computing loop resource.
    if (MVTag) {
      if (VersionedLoops.count(MVTag)) {
        return;
      } else {
        VersionedLoops.insert(MVTag);
      }
    }

    if (visit(cast<HLDDNode>(Lp))) {
      // Add ztt predicates to self resource.
      if (Lp->hasZtt()) {
        addPredicateOps((*Lp->ztt_ddref_begin())->getDestType(),
                        Lp->getNumZttPredicates());
      }
    }
  }

  if (ChildrenLRI) {
    *ChildrenLRI += HLR.getTotalLoopResource(Lp);
  }
}

LoopResourceInfo &LoopResourceInfo::operator+=(const LoopResourceInfo &LRI) {
  IntOps += LRI.IntOps;
  IntOpsCost += LRI.IntOpsCost;
  FPOps += LRI.FPOps;
  FPOpsCost += LRI.FPOpsCost;
  IntMemReads += LRI.IntMemReads;
  IntMemWrites += LRI.IntMemWrites;
  FPMemReads += LRI.FPMemReads;
  FPMemWrites += LRI.FPMemWrites;

  return *this;
}

LoopResourceInfo &LoopResourceInfo::operator*=(unsigned Multiplier) {
  IntOps *= Multiplier;
  IntOpsCost *= Multiplier;
  FPOps *= Multiplier;
  FPOpsCost *= Multiplier;
  IntMemReads *= Multiplier;
  IntMemWrites *= Multiplier;
  FPMemReads *= Multiplier;
  FPMemWrites *= Multiplier;

  return *this;
}

void LoopResourceInfo::classify() {

  unsigned IntOpsCost = getIntOpsCost();
  unsigned FPOpsCost = getFPOpsCost();
  unsigned MemOpsCost = getMemOpsCost();

  if (MemOpsCost && (MemOpsCost >= FPOpsCost) && (MemOpsCost >= IntOpsCost)) {
    Bound = LoopResourceBound::Memory;

  } else if (FPOpsCost && (FPOpsCost >= IntOpsCost)) {
    Bound = LoopResourceBound::FP;

  } else {
    Bound = LoopResourceBound::Int;
  }
}

void LoopResourceInfo::LoopResourceVisitor::compute() {

  // Do not directly recurse inside children loops. Total resource is
  // recursively computed for children loops by the visitor using
  // getTotalLoopResource().
  Lp->getHLNodeUtils().visitRange<true, false>(*this, Lp->child_begin(),
                                               Lp->child_end());

  // Classify self reource into Mem bound, FP bound or Int bound.
  if (SelfLRI) {
    SelfLRI->classify();
  }
}

void LoopResourceInfo::print(formatted_raw_ostream &OS,
                             const HLLoop *Lp) const {

  // Indent at one level more than the loop nesting level.
  unsigned Depth = Lp->getNestingLevel() + 1;

  if (IntOps) {
    Lp->indent(OS, Depth);
    OS << "Integer Operations: " << IntOps << "\n";
  }
  if (IntOpsCost) {
    Lp->indent(OS, Depth);
    OS << "Integer Operations Cost: " << IntOpsCost << "\n";
  }

  if (FPOps) {
    Lp->indent(OS, Depth);
    OS << "Floating Point Operations: " << FPOps << "\n";
  }
  if (FPOpsCost) {
    Lp->indent(OS, Depth);
    OS << "Floating Point Operations Cost: " << FPOpsCost << "\n";
  }

  if (IntMemReads) {
    Lp->indent(OS, Depth);
    OS << "Integer Memory Reads: " << IntMemReads << "\n";
  }
  if (IntMemWrites) {
    Lp->indent(OS, Depth);
    OS << "Integer Memory Writes: " << IntMemWrites << "\n";
  }
  if (FPMemReads) {
    Lp->indent(OS, Depth);
    OS << "Floating Point Reads: " << FPMemReads << "\n";
  }
  if (FPMemWrites) {
    Lp->indent(OS, Depth);
    OS << "Floating Point Writes: " << FPMemWrites << "\n";
  }

  auto MemOpsCost = getMemOpsCost();
  if (MemOpsCost) {
    Lp->indent(OS, Depth);
    OS << "Memory Operations Cost: " << MemOpsCost << "\n";
  }

  Lp->indent(OS, Depth);

  switch (Bound) {
  case LoopResourceBound::Memory:
    OS << "Memory Bound \n";
    break;
  case LoopResourceBound::Int:
    OS << "Integer Bound \n";
    break;
  case LoopResourceBound::FP:
    OS << "Floating Point Bound \n";
    break;
  case LoopResourceBound::Unknown:
    OS << "Unknown Bound \n";
    break;
  default:
    llvm_unreachable("Unexpected loop resource bound type!");
  }
}

const LoopResourceInfo &
HIRLoopResource::getSelfLoopResource(const HLLoop *Loop) {
  assert(Loop && " Loop parameter is null.");

  auto LRInfoIt = SelfResourceMap.find(Loop);

  // Return cached resource, if present.
  if (LRInfoIt != SelfResourceMap.end()) {
    return LRInfoIt->second;
  }

  LoopResourceInfo SelfLRI;

  LoopResourceInfo::LoopResourceVisitor LRV(*this, Loop, &SelfLRI, nullptr);

  LRV.compute();

  auto SelfPair = SelfResourceMap.insert(std::make_pair(Loop, SelfLRI));

  return SelfPair.first->second;
}

const LoopResourceInfo &
HIRLoopResource::getTotalLoopResource(const HLLoop *Loop) {
  assert(Loop && " Loop parameter is null.");

  // Self and total loop resource for innermost loops are the same.
  if (Loop->isInnermost()) {
    return getSelfLoopResource(Loop);
  }

  auto LRInfoIt = TotalResourceMap.find(Loop);

  // Return cached resource, if present.
  if (LRInfoIt != TotalResourceMap.end()) {
    return LRInfoIt->second;
  }

  // Check if self resource also needs to be computed. If so, we compute both
  // together to avoid traversing the loop body twice.
  bool HasSelfResource = SelfResourceMap.count(Loop);

  LoopResourceInfo SelfLRI, TotalLRI;
  LoopResourceInfo::LoopResourceVisitor LRV(
      *this, Loop, HasSelfResource ? nullptr : &SelfLRI, &TotalLRI);

  LRV.compute();

  // We need to retrieve the self resource of the loop again as previous
  // DenseMap entry might have been invalidated by the traversal (by creating
  // new entries in the map). insert() doesn't override existing entry so it is
  // okay to invoke it using empty SelfLRI.
  auto SelfPair = SelfResourceMap.insert(std::make_pair(Loop, SelfLRI));

  TotalLRI += SelfPair.first->second;
  TotalLRI.classify();

  auto TotalPair = TotalResourceMap.insert(std::make_pair(Loop, TotalLRI));

  return TotalPair.first->second;
}

void HIRLoopResource::print(formatted_raw_ostream &OS, const HLLoop *Lp) {
  const LoopResourceInfo &LRI =
      PrintTotalResource ? getTotalLoopResource(Lp) : getSelfLoopResource(Lp);
  LRI.print(OS, Lp);
}

void HIRLoopResource::markLoopBodyModified(const HLLoop *Loop) {
  assert(Loop && " Loop parameter is null.");

  // Remove current loop's self resource from the cache.
  SelfResourceMap.erase(Loop);

  // Remove current and parent loops total resource from the cache.
  while (Loop) {
    TotalResourceMap.erase(Loop);
    Loop = Loop->getParentLoop();
  }
}

unsigned HIRLoopResource::getOperationCost(const Instruction &Inst) const {

  // Special case memory operations.
  if (Inst.mayReadOrWriteMemory()) {
    return LoopResourceInfo::OperationCost::MemOp;
  }

  return LoopResourceInfo::LoopResourceVisitor::getNormalizedCost(
      TTI.getUserCost(&Inst));
}

unsigned HIRLoopResource::getLLVMLoopCost(const Loop &Lp) {
  unsigned Cost = 0;

  for (auto BB : Lp.blocks()) {
    if (LI.getLoopFor(BB) != &Lp) {
      continue;
    }

    for (const Instruction &Inst : *BB) {
      Cost += getOperationCost(Inst);
    }
  }

  return Cost;
}
