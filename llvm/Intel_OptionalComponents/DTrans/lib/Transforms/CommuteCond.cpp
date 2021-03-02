//===--------------- CommuteCond.cpp - CommuteCondPass-------------===//
//
// Copyright (C) 2020-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the DTrans CommuteCond optimization. This
// optimization does very simple IR transformations. Currently, it does
// swap operands of "And" instruction in some rare cases. This transformation
// is not really related to DTrans even though it is implemented as a part
// of DTrans. There are mainly two reasons why it is implemented in DTrans.
//  1. Heuristic for this transformation is using DTrans infrastructure
//  2. Want to trigger this transformation only in special cases.
//
//  Before:
//    %cmp = icmp slt i64 %red_cost, 0
//    br i1 %cmp, label %land.lhs.true, label %lor.rhs
//  lor.rhs:
//    %cmp1 = icmp ne i64 %red_cost, 0
//    %cmp2 = icmp eq i16 %ident, 2
//    %and1 = and i1 %cmp1, %cmp2
//    br i1 %and1, label %l.end.true, label %l.end.false
//  land.lhs.true:
//    %cmp3 = icmp eq i16 %ident, 1
//    br i1 %cmp3, label %l.end.true, label %l.end.false
//
//  After: (Operands of %and1 are swapped)
//    %cmp = icmp slt i64 %red_cost, 0
//    br i1 %cmp, label %land.lhs.true, label %lor.rhs
//  lor.rhs:
//    %cmp1 = icmp ne i64 %red_cost, 0
//    %cmp2 = icmp eq i16 %ident, 2
//    %and1 = and i1 %cmp2, %cmp1           ; Only instruction changed
//    br i1 %and1, label %l.end.true, label %l.end.false
//  land.lhs.true:
//    %cmp3 = icmp eq i16 %ident, 1
//    br i1 %cmp3, label %l.end.true, label %l.end.false
//
// Short-circuiting code is generated for the "And" instruction (i.e %and1)
// during ISel pass of CodeGen.
// This transformation is beneficial only when second compare (i.e %cmp2 in
// the example) is false more times. In such cases, swapping of operands may
// improve performance because fewer comparisons and branches will be
// executed at runtime. This is the reason why we want to trigger this
// transformation only in specific cases using the heuristics.
//===----------------------------------------------------------------------===//
#include "Intel_DTrans/Transforms/CommuteCond.h"
#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "Intel_DTrans/DTransCommon.h"

#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;
using namespace PatternMatch;

#define DEBUG_TYPE "dtrans-commutecond"

// Maximum number of possible constants allowed for a field of struct.
// This is used in heuristics.
constexpr static int MaxPossibleConstantsAllowed = 3;

// Ignore heuristics to trigger the transformation if this option is
// true.
static cl::opt<bool>
    DTransCommuteCondIgnoreHeuristic("dtrans-commute-cond-ignore-heuristic",
                                     cl::init(false), cl::ReallyHidden);

namespace {

// This pass is treated as ModulePass even though it is not necessary to
// avoid running FunctionPass in the middle of all other DTrans ModulePasses.
// Legacy pass manager wrapper for invoking the CommuteCond pass.
class DTransCommuteCondWrapper : public ModulePass {
private:
  dtrans::CommuteCondPass Impl;

public:
  static char ID;
  DTransCommuteCondWrapper() : ModulePass(ID) {
    initializeDTransCommuteCondWrapperPass(*PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;

    DTransAnalysisInfo &DTInfo =
        getAnalysis<DTransAnalysisWrapper>().getDTransInfo(M);

    WholeProgramInfo &WPInfo =
        getAnalysis<WholeProgramWrapperPass>().getResult();

    return Impl.runImpl(M, DTInfo, WPInfo);
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DTransAnalysisWrapper>();
    AU.addRequired<WholeProgramWrapperPass>();
    // Swapping operands of AND should not invalidate any analysis.
    AU.setPreservesAll();
  }
};

} // end anonymous namespace

char DTransCommuteCondWrapper::ID = 0;
INITIALIZE_PASS_BEGIN(DTransCommuteCondWrapper, "dtrans-commutecond",
                      "DTrans CommuteCond", false, false)
INITIALIZE_PASS_DEPENDENCY(DTransAnalysisWrapper)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_END(DTransCommuteCondWrapper, "dtrans-commutecond",
                    "DTrans CommuteCond", false, false)

ModulePass *llvm::createDTransCommuteCondWrapperPass() {
  return new DTransCommuteCondWrapper();
}

namespace llvm {

namespace dtrans {

class CommuteCondImpl : public InstVisitor<CommuteCondImpl> {
public:
  CommuteCondImpl(DTransAnalysisInfo &DTInfo) : DTInfo(DTInfo){};
  void visitAnd(Instruction &I) { processAndInst(I); }
  bool transformAnds(void);

private:
  DTransAnalysisInfo &DTInfo;

  // Transformations will be applied for all "And" instructions in this set.
  SmallPtrSet<Instruction *, 4> AndsToCommute;

  void processAndInst(Instruction &);
  bool checkHeuristics(Value *);
};

// Apply heuristics using DTrans infrastructure.
// Returns true if
//   1. "Val" is a field of struct
//   2. Set of possible constant values for the field is complete
//   3. Number of possible constant values doesn't exceed
//   MaxPossibleConstantsAllowed
bool CommuteCondImpl::checkHeuristics(Value *Val) {

  // Ignore heuristic if DTransCommuteCondIgnoreHeuristic is true.
  if (DTransCommuteCondIgnoreHeuristic)
    return true;

  auto *Load = dyn_cast<LoadInst>(Val);
  if (!Load)
    return false;

  auto LdInfo = DTInfo.getLoadElement(Load);
  Type *STy = dyn_cast_or_null<llvm::StructType>(LdInfo.first);
  if (!STy)
    return false;
  auto *StInfo = cast<dtrans::StructInfo>(DTInfo.getTypeInfo(STy));
  dtrans::FieldInfo &FI = StInfo->getField(LdInfo.second);

  // Don't allow non-integer types.
  if (!FI.getLLVMType()->isIntegerTy())
    return false;
  // All possible values should be known at compile-time.
  if (!FI.isValueSetComplete() ||
      FI.values().size() > MaxPossibleConstantsAllowed)
    return false;

  // We could add more checks here like below if we want
  // 1. Check for safety conditions of the struct
  // 2. Make sure constant values that are used in ICmp instructions (i.e C1
  // and C2 in processAndInst) are in FI.values().
  //
  return true;
}

// "I", which is an "And" instruction, is added to "AndsToCommute" if
// it is in the below pattern. Heuristics will be applied to reduce possible
// candidates.
//
//      %cmp0 = icmp slt i64 %red_cost, 0
//      br i1 %cmp0, label %land.lhs.true, label %lor.rhs
//    lor.rhs:
//      %cmp1 = icmp ne i64 %red_cost, 0
//      %cmp2 = icmp eq i16 %ident, C1
// I:   %and1 = and i1 %cmp1, %cmp2
//      br i1 %and1, label %l.end.true, label %l.end.false
//    land.lhs.true:
//      %cmp3 = icmp eq i16 %ident, C2
//      br i1 %cmp3, label %l.end.true, label %l.end.false
//
void CommuteCondImpl::processAndInst(Instruction &I) {
  // Check both operands of "I" are ICmpInst. Makes sure "I" and both operands
  // have single users.
  ICmpInst *AndOp0 = dyn_cast<ICmpInst>(I.getOperand(0));
  ICmpInst *AndOp1 = dyn_cast<ICmpInst>(I.getOperand(1));
  if (!AndOp0 || !AndOp1 || !I.hasOneUse() || !AndOp0->hasOneUse() ||
      !AndOp1->hasOneUse())
    return;

  // Check if "I" is used by BranchInst.
  auto BI = dyn_cast<BranchInst>(*I.user_begin());
  if (!BI)
    return;

  // Check for %cmp1 = icmp ne i64 %red_cost, 0
  Value *Cmp1Op;
  const APInt *C1;
  CmpInst::Predicate Cmp1Pred;
  if (!match(AndOp0, m_ICmp(Cmp1Pred, m_Value(Cmp1Op), m_ZeroInt())) ||
      Cmp1Pred != ICmpInst::ICMP_NE)
    return;
  // Check for %cmp2 = icmp eq i16 %ident, C1
  Value *Cmp2Op;
  CmpInst::Predicate Cmp2Pred;
  if (!match(AndOp1, m_ICmp(Cmp2Pred, m_Value(Cmp2Op), m_APInt(C1))) ||
      Cmp2Pred != ICmpInst::ICMP_EQ)
    return;

  BasicBlock *AndBB = BI->getParent();
  BasicBlock *AndPredBB = AndBB->getSinglePredecessor();
  if (!AndPredBB)
    return;
  auto AndPredBranchI = dyn_cast<BranchInst>(AndPredBB->getTerminator());
  if (!AndPredBranchI || !AndPredBranchI->isConditional())
    return;
  ICmpInst *Cmp0 = dyn_cast<ICmpInst>(AndPredBranchI->getCondition());
  if (!Cmp0)
    return;

  // Check for %cmp0 = icmp slt i64 %red_cost, 0
  CmpInst::Predicate Cmp0Pred;
  if (!match(Cmp0, m_ICmp(Cmp0Pred, m_Specific(Cmp1Op), m_ZeroInt())) ||
      Cmp0Pred != ICmpInst::ICMP_SLT)
    return;

  if (AndPredBranchI->getSuccessor(1) != AndBB)
    return;
  BasicBlock *TBB = AndPredBranchI->getSuccessor(0);
  auto *Cmp3 = dyn_cast_or_null<ICmpInst>(TBB->getFirstNonPHIOrDbg());
  if (!Cmp3)
    return;

  // Check for %cmp3 = icmp eq i16 %ident, C2
  CmpInst::Predicate Cmp3Pred;
  const APInt *C2;
  if (!match(Cmp3, m_ICmp(Cmp3Pred, m_Value(Cmp2Op), m_APInt(C2))) ||
      Cmp3Pred != ICmpInst::ICMP_EQ)
    return;
  if (!Cmp3->hasOneUse() || !isa<BranchInst>(*Cmp3->user_begin()))
    return;

  // Apply heuristics
  if (!checkHeuristics(Cmp2Op))
    return;

  AndsToCommute.insert(&I);
}

// Swap operands of all "And" instructions in "AndsToCommute".
bool CommuteCondImpl::transformAnds() {
  if (AndsToCommute.empty())
    return false;

  for (auto AndI : AndsToCommute) {
    LLVM_DEBUG(dbgs() << "  Transformed in " << AndI->getFunction()->getName()
                      << "\n");
    LLVM_DEBUG(dbgs() << "    Before: " << *AndI << "\n");
    // Swap operands of AndI.
    Instruction *NewA = BinaryOperator::CreateAnd(
        AndI->getOperand(1), AndI->getOperand(0), "", AndI);
    NewA->setDebugLoc(AndI->getDebugLoc());
    AndI->replaceAllUsesWith(NewA);
    NewA->takeName(AndI);
    AndI->eraseFromParent();
    LLVM_DEBUG(dbgs() << "    After: " << *AndI << "\n");
  }
  LLVM_DEBUG(dbgs() << "DTRANS CommuteCond: Transformations done\n");
  return true;
}

PreservedAnalyses CommuteCondPass::run(Module &M, ModuleAnalysisManager &AM) {
  auto &DTransInfo = AM.getResult<DTransAnalysis>(M);
  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);

  runImpl(M, DTransInfo, WPInfo);

  // Swapping operands of AND should not invalidate any analysis.
  return PreservedAnalyses::all();
}

bool CommuteCondPass::runImpl(Module &M, DTransAnalysisInfo &DTInfo,
                              WholeProgramInfo &WPInfo) {

  auto TTIAVX2 = TargetTransformInfo::AdvancedOptLevel::AO_TargetHasIntelAVX2;
  if (!WPInfo.isWholeProgramSafe() || !WPInfo.isAdvancedOptEnabled(TTIAVX2))
    return false;

  if (!DTInfo.useDTransAnalysis())
    return false;

  LLVM_DEBUG(dbgs() << "DTRANS CommuteCond: Started\n");
  CommuteCondImpl RCImpl(DTInfo);
  for (auto &F : M) {
    if (F.isDeclaration())
      continue;
    RCImpl.visit(F);
  }
  bool Changed = RCImpl.transformAnds();
  if (!Changed)
    LLVM_DEBUG(dbgs() << "DTRANS CommuteCond: No transformations\n");

  return Changed;
}

} // end namespace dtrans
} // end namespace llvm
