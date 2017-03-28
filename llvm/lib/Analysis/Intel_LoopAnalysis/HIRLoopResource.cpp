//===----------- HIRLoopResource.cpp - Computes loop resources ------------===//
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

#include "llvm/Analysis/Intel_LoopAnalysis/HIRLoopResource.h"

#include "llvm/ADT/SmallSet.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/TargetTransformInfo.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Passes.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/BlobUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

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

FunctionPass *llvm::createHIRLoopResourcePass() {
  return new HIRLoopResource();
}

char HIRLoopResource::ID = 0;
INITIALIZE_PASS_BEGIN(HIRLoopResource, "hir-loop-resource",
                      "Loop Resource Analysis", false, true)
INITIALIZE_PASS_DEPENDENCY(HIRFramework)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_END(HIRLoopResource, "hir-loop-resource",
                    "Loop Resource Analysis", false, true)

void HIRLoopResource::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<HIRFramework>();
  AU.addRequired<LoopInfoWrapperPass>();
  AU.addRequired<TargetTransformInfoWrapperPass>();
}

bool HIRLoopResource::runOnFunction(Function &F) {

  LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  TTI = &getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F);

  // This is an on-demand analysis, so we don't perform any analysis here.
  return false;
}

void HIRLoopResource::releaseMemory() {
  SelfResourceMap.clear();
  TotalResourceMap.clear();
}

struct LoopResourceInfo::LoopResourceVisitor final : public HLNodeVisitorBase {
  HIRLoopResource &HLR;
  const TargetTransformInfo &TTI;
  const HLLoop *Lp;
  LoopResourceInfo &SelfLRI;
  LoopResourceInfo *TotalLRI;
  SmallSet<unsigned, 8> VersionedLoops;

  struct BlobCostEvaluator;

  LoopResourceVisitor(HIRLoopResource &HLR, const HLLoop *Lp,
                      LoopResourceInfo &SelfLRI, LoopResourceInfo *TotalLRI)
      : HLR(HLR), TTI(*HLR.getTTI()), Lp(Lp), SelfLRI(SelfLRI),
        TotalLRI(TotalLRI) {}

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
    LRV.SelfLRI.addIntOps(Cost);

    visit(TruncBlob->getOperand());
  }

  void visitZeroExtendExpr(const SCEVZeroExtendExpr *ZExtBlob) {
    unsigned Cost = LRV.getNormalizedCost(
        LRV.TTI.getOperationCost(Instruction::ZExt, ZExtBlob->getType(),
                                 ZExtBlob->getOperand()->getType()));
    LRV.SelfLRI.addIntOps(Cost);

    visit(ZExtBlob->getOperand());
  }

  void visitSignExtendExpr(const SCEVSignExtendExpr *SExtBlob) {
    unsigned Cost = LRV.getNormalizedCost(
        LRV.TTI.getOperationCost(Instruction::SExt, SExtBlob->getType(),
                                 SExtBlob->getOperand()->getType()));
    LRV.SelfLRI.addIntOps(Cost);

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
    LRV.SelfLRI.addIntOps(Cost, AddBlob->getNumOperands() - 1);

    visitNAryExpr(cast<SCEVNAryExpr>(AddBlob));
  }

  void visitMulExpr(const SCEVMulExpr *MulBlob) {
    unsigned Cost = LRV.getNormalizedCost(
        LRV.TTI.getOperationCost(Instruction::Mul, MulBlob->getType()));
    LRV.SelfLRI.addIntOps(Cost, MulBlob->getNumOperands() - 1);

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

    LRV.SelfLRI.addIntOps(Cost);

    visit(UDivBlob->getLHS());
    visit(UDivBlob->getRHS());
  }

  void visitAddRecExpr(const SCEVAddRecExpr *AddRec) {
    llvm_unreachable("Found AddRec blob!");
  }

  void visitSMaxExpr(const SCEVSMaxExpr *SMaxBlob) {
    unsigned Cost = LRV.getNormalizedCost(
        LRV.TTI.getOperationCost(Instruction::ICmp, SMaxBlob->getType()));
    LRV.SelfLRI.addIntOps(Cost, SMaxBlob->getNumOperands() - 1);

    visitNAryExpr(cast<SCEVNAryExpr>(SMaxBlob));
  }

  void visitUMaxExpr(const SCEVUMaxExpr *UMaxBlob) {
    unsigned Cost = LRV.getNormalizedCost(
        LRV.TTI.getOperationCost(Instruction::ICmp, UMaxBlob->getType()));
    LRV.SelfLRI.addIntOps(Cost, UMaxBlob->getNumOperands() - 1);

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

  SelfLRI.addIntOps(Cost);
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

  SelfLRI.addIntOps(Cost);
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
      isPowerOf2_64(Coeff) ? SelfLRI.addIntOps(ShlCost)
                           : SelfLRI.addIntOps(MulCost);
    }

    if (BlobIndex != InvalidBlobIndex) {
      visit(BlobIndex);
      SelfLRI.addIntOps(MulCost);
    }

    if (First) {
      FoundAdditive = true;
      First = false;
    } else {
      SelfLRI.addIntOps(AddCost);
    }
  }

  First = true;
  for (auto Blob = CE->blob_begin(), E = CE->blob_end(); Blob != E; ++Blob) {
    if (First) {
      if (FoundAdditive) {
        SelfLRI.addIntOps(AddCost);
      }
      FoundAdditive = true;
      First = false;
    } else {
      SelfLRI.addIntOps(AddCost);
    }

    visit(Blob->Index);

    if (Blob->Coeff != 1) {
      isPowerOf2_64(Blob->Coeff) ? SelfLRI.addIntOps(ShlCost)
                                 : SelfLRI.addIntOps(MulCost);
    }
  }

  if (FoundAdditive && CE->getConstant() != 0) {
    SelfLRI.addIntOps(AddCost);
  }

  addDenominatorCost(CE);

  addCastCost(CE);
}

void LoopResourceInfo::LoopResourceVisitor::visit(const RegDDRef *Ref) {

  if (Ref->isMemRef()) {
    bool IsFloat = Ref->getDestType()->isFPOrFPVectorTy();

    if (Ref->isLval()) {
      IsFloat ? ++SelfLRI.FPMemWrites : ++SelfLRI.IntMemWrites;
    } else {
      IsFloat ? ++SelfLRI.FPMemReads : ++SelfLRI.IntMemReads;
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
      SelfLRI.addIntOps(Cost);

      auto Stride = Ref->getDimensionStride(DimNum);

      // Add cost of stride multiplication.
      Cost = getNormalizedCost(TTI.getOperationCost(
          isPowerOf2_64(Stride) ? Instruction::Shl : Instruction::Mul,
          (*CEI)->getDestType()));
      SelfLRI.addIntOps(Cost);
    }
  }
}

bool LoopResourceInfo::LoopResourceVisitor::visit(const HLDDNode *Node) {

  // Valid SelfLRI is available.
  if (!SelfLRI.isUnknownBound()) {
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

  // Valid SelfLRI is available.
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
      isa<GetElementPtrInst>(Inst)) {
    return;
  }

  unsigned Cost = getOperationCost(HInst);
  bool IsFP = false;

  if (isa<CmpInst>(Inst) || isa<SelectInst>(Inst)) {
    IsFP = CmpInst::isFPPredicate(HInst->getPredicate());
  } else if (HInst->hasLval()) {
    IsFP = HInst->getLvalDDRef()->getDestType()->isFPOrFPVectorTy();
  }

  IsFP ? SelfLRI.addFPOps(Cost) : SelfLRI.addIntOps(Cost);
}

void LoopResourceInfo::LoopResourceVisitor::addPredicateOps(Type *Ty,
                                                            unsigned Num) {
  unsigned Cost = 0;

  // Add number of logical operations as IntOps.
  if (Num > 1) {
    Cost = getNormalizedCost(TTI.getOperationCost(Instruction::And, Ty));
    SelfLRI.addIntOps(Cost, Num - 1);
  }

  // Add number of compares.
  if (Ty->isFPOrFPVectorTy()) {
    Cost = getNormalizedCost(TTI.getOperationCost(Instruction::FCmp, Ty));
    SelfLRI.addFPOps(Cost, Num);

  } else {
    Cost = getNormalizedCost(TTI.getOperationCost(Instruction::ICmp, Ty));
    SelfLRI.addIntOps(Cost, Num);
  }
}

void LoopResourceInfo::LoopResourceVisitor::visit(const HLIf *If) {

  // Valid SelfLRI is available.
  if (!visit(cast<HLDDNode>(If))) {
    return;
  }

  // TODO: use weighed average of then and else children?

  // Capture the number of compare and logical operations.
  addPredicateOps((*If->op_ddref_begin())->getDestType(),
                  If->getNumPredicates());
}

void LoopResourceInfo::LoopResourceVisitor::visit(const HLLoop *Lp) {

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

  // No need to process children loops in self-only mode.
  if (TotalLRI) {
    *TotalLRI += HLR.getTotalLoopResource(Lp);
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
  if (SelfLRI.isUnknownBound()) {
    SelfLRI.classify();
  }

  // Add self reource to total resource and classify it.
  if (TotalLRI) {
    *TotalLRI += SelfLRI;
    TotalLRI->classify();
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

const LoopResourceInfo &HIRLoopResource::computeLoopResource(const HLLoop *Loop,
                                                             bool SelfOnly) {

  // These will be set below using the cache and SelfOnly paramter.
  LoopResourceInfo *TotalLRI = nullptr;

  // Get or Insert self resource.
  auto SelfPair =
      SelfResourceMap.insert(std::make_pair(Loop, LoopResourceInfo()));
  LoopResourceInfo &SelfLRI = SelfPair.first->second;

  if (!SelfOnly) {
    // Set TotalLRI to indicate that total resource need to be computed.
    auto TotalPair =
        TotalResourceMap.insert(std::make_pair(Loop, LoopResourceInfo()));
    TotalLRI = &TotalPair.first->second;
  }

  LoopResourceInfo::LoopResourceVisitor LRV(*this, Loop, SelfLRI, TotalLRI);

  LRV.compute();

  return SelfOnly ? SelfLRI : *TotalLRI;
}

const LoopResourceInfo &
HIRLoopResource::getSelfLoopResource(const HLLoop *Loop) {
  assert(Loop && " Loop parameter is null.");

  auto LRInfoIt = SelfResourceMap.find(Loop);

  // Return cached resource, if present.
  if (LRInfoIt != SelfResourceMap.end()) {
    return LRInfoIt->second;
  }

  // Compute and return a new reource.
  return computeLoopResource(Loop, true);
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

  // Compute and return a new reource.
  return computeLoopResource(Loop, false);
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
      TTI->getUserCost(&Inst));
}

unsigned HIRLoopResource::getLLVMLoopCost(const Loop &Lp) {
  unsigned Cost = 0;

  for (auto BB : Lp.blocks()) {
    if (LI->getLoopFor(BB) != &Lp) {
      continue;
    }

    for (const Instruction &Inst : *BB) {
      Cost += getOperationCost(Inst);
    }
  }

  return Cost;
}
