//===--------------- CommuteCond.cpp - CommuteCondPass-------------===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the DTrans CommuteCond optimization. This
// optimization does very simple IR transformations. Currently, it does
// swap operands of "And" and "Select" instructions in some rare cases.
// This transformation is not really related to DTrans even though it is
// implemented as a part of DTrans. There are mainly two reasons why it is
// implemented in DTrans.
//  1. Heuristic for this transformation is using DTrans infrastructure
//  2. Want to trigger this transformation only in special cases.
//
// Ex 1:
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
//
// Ex 2:
//  Before:
//    %cmp = icmp slt i64 %red_cost, 0
//    br i1 %cmp, label %land.lhs.true, label %lor.rhs
//  lor.rhs:
//    %cmp1 = icmp ne i64 %red_cost, 0
//    %cmp2 = icmp eq i16 %ident, 2
//    %sel1 = select i1 %cmp1, %cmp2, false
//    br i1 %sel1, label %l.end.true, label %l.end.false
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
//    %sel1 = select i1 %cmp2, %cmp1, false      ; Only instruction changed
//    br i1 %sel1, label %l.end.true, label %l.end.false
//  land.lhs.true:
//    %cmp3 = icmp eq i16 %ident, 1
//    br i1 %cmp3, label %l.end.true, label %l.end.false
//
// This transformation is beneficial only when second compare (i.e %cmp2 in
// the example) is false more times. In such cases, swapping of operands may
// improve performance because fewer comparisons and branches will be
// executed at runtime. This is the reason why we want to trigger this
// transformation only in specific cases using the heuristics.
//===----------------------------------------------------------------------===//
#include "Intel_DTrans/Transforms/CommuteCond.h"
#include "Intel_DTrans/Analysis/DTransInfoAdapter.h"

#include "llvm/Analysis/Intel_WP.h"
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

template <class InfoClass>
class CommuteCondImpl : public InstVisitor<CommuteCondImpl<InfoClass>> {
public:
  CommuteCondImpl(InfoClass &DTInfo) : DTInfo(DTInfo){};
  void visitAnd(Instruction &I) { processAndInst(I); }
  void visitSelect(SelectInst &I) { processSelectInst(I); }
  bool transform(void);

private:
  InfoClass &DTInfo;

  // Transformations will be applied for all instructions in this set. Only
  // "And" and "Select" instructions are added to this set currently.
  SmallPtrSet<Instruction *, 4> InstructionsToCommute;

  bool commuteOperandsOkay(Instruction &, Value *, Value *);
  void processAndInst(Instruction &);
  void processSelectInst(SelectInst &);
  bool checkHeuristics(Value *);
};

// Apply heuristics using DTrans infrastructure.
// Returns true if
//   1. "Val" is a field of struct
//   2. Set of possible constant values for the field is complete
//   3. Number of possible constant values doesn't exceed
//   MaxPossibleConstantsAllowed
template <class InfoClass>
bool CommuteCondImpl<InfoClass>::checkHeuristics(Value *Val) {

  // Ignore heuristic if DTransCommuteCondIgnoreHeuristic is true.
  if (DTransCommuteCondIgnoreHeuristic)
    return true;

  auto *Load = dyn_cast<LoadInst>(Val);
  if (!Load)
    return false;

  auto LdInfo = DTInfo.getLoadElement(Load);
  StructType *STy = dyn_cast_or_null<llvm::StructType>(LdInfo.first);
  if (!STy)
    return false;
  dtrans::StructInfo *StInfo = DTInfo.getStructTypeInfo(STy);
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

// Returns true if "Op0" and "Op1", which are operands of "I", if they
// can be swapped by checking the pattern below. Heuristics will be applied
// to reduce possible candidates.
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
template <class InfoClass>
bool CommuteCondImpl<InfoClass>::commuteOperandsOkay(Instruction &I, Value *Op0,
                                          Value *Op1) {
  auto *AndOp0 = dyn_cast<ICmpInst>(Op0);
  auto *AndOp1 = dyn_cast<ICmpInst>(Op1);
  // Check both operands of "I" are ICmpInst. Makes sure "I" and both operands
  // have single users.
  if (!AndOp0 || !AndOp1 || !I.hasOneUse() || !AndOp0->hasOneUse() ||
      !AndOp1->hasOneUse())
    return false;

  // Check if "I" is used by BranchInst.
  auto BI = dyn_cast<BranchInst>(*I.user_begin());
  if (!BI)
    return false;

  // Check for %cmp1 = icmp ne i64 %red_cost, 0
  Value *Cmp1Op;
  const APInt *C1;
  CmpInst::Predicate Cmp1Pred;
  if (!match(AndOp0, m_ICmp(Cmp1Pred, m_Value(Cmp1Op), m_ZeroInt())) ||
      Cmp1Pred != ICmpInst::ICMP_NE)
    return false;
  // Check for %cmp2 = icmp eq i16 %ident, C1
  Value *Cmp2Op;
  CmpInst::Predicate Cmp2Pred;
  if (!match(AndOp1, m_ICmp(Cmp2Pred, m_Value(Cmp2Op), m_APInt(C1))) ||
      Cmp2Pred != ICmpInst::ICMP_EQ)
    return false;

  BasicBlock *AndBB = BI->getParent();
  BasicBlock *AndPredBB = AndBB->getSinglePredecessor();
  if (!AndPredBB)
    return false;
  auto AndPredBranchI = dyn_cast<BranchInst>(AndPredBB->getTerminator());
  if (!AndPredBranchI || !AndPredBranchI->isConditional())
    return false;
  ICmpInst *Cmp0 = dyn_cast<ICmpInst>(AndPredBranchI->getCondition());
  if (!Cmp0)
    return false;

  // Check for %cmp0 = icmp slt i64 %red_cost, 0
  CmpInst::Predicate Cmp0Pred;
  if (!match(Cmp0, m_ICmp(Cmp0Pred, m_Specific(Cmp1Op), m_ZeroInt())) ||
      Cmp0Pred != ICmpInst::ICMP_SLT)
    return false;

  if (AndPredBranchI->getSuccessor(1) != AndBB)
    return false;
  BasicBlock *TBB = AndPredBranchI->getSuccessor(0);
  auto *Cmp3 = dyn_cast_or_null<ICmpInst>(TBB->getFirstNonPHIOrDbg());
  if (!Cmp3)
    return false;

  // Check for %cmp3 = icmp eq i16 %ident, C2
  CmpInst::Predicate Cmp3Pred;
  const APInt *C2;
  if (!match(Cmp3, m_ICmp(Cmp3Pred, m_Value(Cmp2Op), m_APInt(C2))) ||
      Cmp3Pred != ICmpInst::ICMP_EQ)
    return false;
  if (!Cmp3->hasOneUse() || !isa<BranchInst>(*Cmp3->user_begin()))
    return false;

  // Apply heuristics
  if (!checkHeuristics(Cmp2Op))
    return false;

  return true;
}

// "I", which is an "And" instruction, is added to "InstructionsToCommute" if
// operands can be swapped.
// Ex:
//    %and1 = and i1 %cmp1, %cmp2
template <class InfoClass>
void CommuteCondImpl<InfoClass>::processAndInst(Instruction &I) {
  if (!commuteOperandsOkay(I, I.getOperand(0), I.getOperand(1)))
    return;
  InstructionsToCommute.insert(&I);
}

// "SI" is added to "InstructionsToCommute" if Condition and TrueValue operands
// can be swapped.
// Ex:
//    %sel1 = select i1 %cmp1, %cmp2, false
template <class InfoClass>
void CommuteCondImpl<InfoClass>::processSelectInst(SelectInst &SI) {
  // Check FalseValue of SI is false.
  Value *FalseVal = SI.getFalseValue();
  Type *Ty = FalseVal->getType();
  if (!Ty->isIntegerTy(1) || FalseVal != ConstantInt::getFalse(Ty))
    return;
  // Check types of all operands are same.
  Value *Cond = SI.getCondition();
  Value *TrueValue = SI.getTrueValue();
  if (Ty != Cond->getType() || Ty != TrueValue->getType())
    return;
  if (!commuteOperandsOkay(SI, Cond, TrueValue))
    return;
  InstructionsToCommute.insert(&SI);
}

// Swap operands of all "And" and "Select" instructions in
// "InstructionsToCommute".
template <class InfoClass> bool CommuteCondImpl<InfoClass>::transform() {
  if (InstructionsToCommute.empty())
    return false;

  for (auto I : InstructionsToCommute) {
    LLVM_DEBUG(dbgs() << "  Transformed in " << I->getFunction()->getName()
                      << "\n");
    LLVM_DEBUG(dbgs() << "    Before: " << *I << "\n");
    Instruction *NewI = nullptr;
    if (I->getOpcode() == Instruction::And) {
      // Swap operands of And.
      NewI =
          BinaryOperator::CreateAnd(I->getOperand(1), I->getOperand(0), "", I);
    } else if (auto *SI = dyn_cast<SelectInst>(I)) {
      // Swap Condition and TrueValue operands of SelectInst.
      NewI = SelectInst::Create(SI->getTrueValue(), SI->getCondition(),
                                SI->getFalseValue(), "", SI);
    }
    assert(NewI && "Expected non-null instruction");
    NewI->setDebugLoc(I->getDebugLoc());
    I->replaceAllUsesWith(NewI);
    NewI->takeName(I);
    I->eraseFromParent();
    LLVM_DEBUG(dbgs() << "    After: " << *NewI << "\n");
  }
  LLVM_DEBUG(dbgs() << "DTRANS CommuteCond: Transformations done\n");
  return true;
}

} // end anonymous namespace

namespace llvm {
namespace dtransOP {

PreservedAnalyses CommuteCondOPPass::run(Module &M, ModuleAnalysisManager &AM) {
  DTransSafetyInfo *DTransInfo = &AM.getResult<DTransSafetyAnalyzer>(M);
  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);

  runImpl(M, *DTransInfo, WPInfo);

  // Swapping operands of AND should not invalidate any analysis.
  return PreservedAnalyses::all();
}

bool CommuteCondOPPass::runImpl(Module &M, DTransSafetyInfo &DTInfo,
                                WholeProgramInfo &WPInfo) {

  auto TTIAVX2 = TargetTransformInfo::AdvancedOptLevel::AO_TargetHasIntelAVX2;
  if (!WPInfo.isWholeProgramSafe() || !WPInfo.isAdvancedOptEnabled(TTIAVX2))
    return false;

  if (!DTInfo.useDTransSafetyAnalysis())
    return false;

  LLVM_DEBUG(dbgs() << "DTRANS CommuteCondOP: Started\n");
  DTransSafetyInfoAdapter SIAdaptor(DTInfo);
  CommuteCondImpl<DTransSafetyInfoAdapter> RCImpl(SIAdaptor);
  for (auto &F : M) {
    if (F.isDeclaration())
      continue;
    RCImpl.visit(F);
  }
  bool Changed = RCImpl.transform();
  if (!Changed)
    LLVM_DEBUG(dbgs() << "DTRANS CommuteCondOP: No transformations\n");

  return Changed;
}

} // end namespace dtransOP

} // end namespace llvm
