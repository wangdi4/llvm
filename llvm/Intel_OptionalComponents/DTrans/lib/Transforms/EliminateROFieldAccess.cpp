//===--------------------   EliminateROFieldAccess.cpp   ------------------===//
//
// Copyright (C) 2018-2023 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
//  This file implements a pass that creates a shortcut over unreachable code
//  accessing a field of a structure that was only read.
//
//  Ex: struct S has no SafetyData violations and 'field' is only read.
//
//  int foo(struct S *base) {
//   int x = 0;
//   if (base != NULL && base->field != NULL)
//     x = base->field();
//   else
//     x = malloc);
//   return x;
//  }
//
//
//   transforms to
//
//  int foo(struct S *base) {
//   int x = 0;
//   x = malloc();
//   return x;
//  }
//
//
//   in CFG:
//
//   +--------------------+
//   |  x = 0;            |
//   |  if (base == NULL) |
//   +--------------------+
//   |                    |
//   | T                  | F
//   V                    V
//   +---------------+  T +--------------------------+
//   | x = malloc(); |<---| if (base->field == NULL) |
//   +---------------+    +--------------------------+
//   |                    | F
//   |                    V
//   |                   +--------------------+
//   V                   | x = base->field(); |
//   +-----------+<------+--------------------+
//   | return x; |
//   +-----------+
//
//   transforms to:
//
//   +--------------------+
//   |  x = 0;            |
//   +--------------------+
//   |
//   V
//   +----------------+
//   |  x = malloc(); |
//   +----------------+
//   |
//   V
//   +-----------+
//   | return x; |
//   +-----------+
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/EliminateROFieldAccess.h"
#include "Intel_DTrans/Analysis/DTrans.h"
#include "Intel_DTrans/Analysis/DTransInfoAdapter.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/Utils/Local.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/IR/Value.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;
using namespace PatternMatch;

#define DEBUG_TYPE "elim-ro-field-access"

namespace {

template <class InfoClass>
struct EliminateROFieldAccessImpl {
  EliminateROFieldAccessImpl(InfoClass &DTransInfo)
      : DTransInfo(DTransInfo){};
  bool run(Module &M, WholeProgramInfo &WPInfo);
  bool visit(BasicBlock *BB);
  bool checkSecondIfBB(BasicBlock *SecondIfBB, Value *BaseOp);

private:
  InfoClass &DTransInfo;
};
} // namespace

// Check if instruction is a cmp of the pointer to the nullptr. Returns a
// pointer operand from cmp.
static Value *isCmpPtrToNull(ICmpInst *CI) {
  if (!CI)
    return nullptr;

  CmpInst::Predicate Pred;
  Value *OpPtr;

  if (!match(CI, m_ICmp(Pred, m_Value(OpPtr), m_Zero())))
    if (!match(CI, m_ICmp(Pred, m_Zero(), m_Value(OpPtr))))
      return nullptr;

  if (Pred != CmpInst::ICMP_EQ && Pred != CmpInst::ICMP_NE)
    return nullptr;

  if (!OpPtr)
    return nullptr;

  if (isa<PointerType>(OpPtr->getType()))
    return OpPtr;

  return nullptr;
}

//
// Function checks that basic block consists of the following instructions:
//   addr = getelementptr %allocator, 0, const_idx
//   t = load addr
//   res = icmp eq t nullptr
//   br res, trueBB, falseBB
//
//  Last condition checks that structure has no safety check violations and
//  the field that is accessed is only read.
//  The same approach works for 'icmp ne t nullptr' but with reversed true and
//  false basic blocks.
template<class InfoClass>
bool EliminateROFieldAccessImpl<InfoClass>::checkSecondIfBB(BasicBlock
                                                                *SecondIfBB,
                                                            Value *BaseOp) {
  ICmpInst::Predicate Pred;
  Instruction *LoadAddr, *Load, *ICmp;
  BasicBlock *TrueBB = nullptr;
  BasicBlock *FalseBB = nullptr;

  if (!match(SecondIfBB->getTerminator(),
             m_Br(m_Instruction(ICmp), TrueBB, FalseBB))) {
    return false;
  }

  if (!ICmp->hasOneUse())
    return false;

  if (!match(ICmp, m_ICmp(Pred, m_Instruction(Load), m_Zero())))
    if (!match(ICmp, m_ICmp(Pred, m_Zero(), m_Instruction(Load))))
      return false;

  if (Load->getParent() != SecondIfBB)
    return false;

  if (Pred == CmpInst::ICMP_NE) {
    BasicBlock *Tmp = FalseBB;
    FalseBB = TrueBB;
    TrueBB = Tmp;
  } else if (Pred != CmpInst::ICMP_EQ)
    return false;

  auto LI = dyn_cast<LoadInst>(Load);
  if (!LI)
    return false;
  auto GEPI = dyn_cast<GetElementPtrInst>(LI->getPointerOperand());
  if (!GEPI || GEPI->getNumIndices() != 2 || !GEPI->hasAllConstantIndices())
    return false;
  auto CI0 = cast<ConstantInt>(GEPI->getOperand(1));
  if (!CI0 || !CI0->isZero())
    return false;
  unsigned Idx = cast<ConstantInt>(GEPI->getOperand(2))->getZExtValue();
  auto STy = dyn_cast<StructType>(GEPI->getSourceElementType());
  if (!STy || !DTransInfo.isFunctionPtr(STy, Idx))
    return false;

  if (!match(Load, m_Load(m_Instruction(LoadAddr))))
    return false;

  GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(LoadAddr);
  if (!GEP || !GEP->hasOneUse() || (GEP->getParent() != SecondIfBB))
    return false;

  if (GEP->getPointerOperand() != BaseOp ||
      !match(GEP->getOperand(1), m_Zero()))
    return false;

  if (SecondIfBB->size() != 4)
    return false;

  if (!DTransInfo.isReadOnlyFieldAccess(dyn_cast<LoadInst>(Load))) {
    LLVM_DEBUG(
        dbgs()
        << "DTRANS-ELIM-RO-FIELD-ACCESS: Safety check failed - skipping.\n");
    return false;
  }

  return true;
}

//
// Main function which updates CFG if original instructions fit the pattern.
// 1. The FirstIFBB should end with a compare of a pointer to a structure and a
// null pointer.
// 2. The true successor of FirstIFBB (SecondIfBB) should have single
// predecessor, it should also end with a comparison of a pointer to null.
// 3. The false successors of the FirstIfBB and SecondIfBB should be the same.
// The true successor of the SecondIfBB should have only one predecessor and
// successor.
//
//  The same approach works for 'icmp ne t nullptr' but with reversed true and
//  false basic blocks.
template <class InfoClass>
bool EliminateROFieldAccessImpl<InfoClass>::visit(BasicBlock *FirstIfBB) {
  BranchInst *BaseBrInst = dyn_cast<BranchInst>(FirstIfBB->getTerminator());
  if (!BaseBrInst || !BaseBrInst->isConditional())
    return false;

  if (BaseBrInst->getNumSuccessors() != 2)
    return false;

  ICmpInst *BaseCond = dyn_cast<ICmpInst>(BaseBrInst->getCondition());
  if (!BaseCond)
    return false;

  BasicBlock *MainBB, *SecondIfBB;
  if (BaseCond->getPredicate() == CmpInst::ICMP_EQ) {
    MainBB = BaseBrInst->getSuccessor(0);
    SecondIfBB = BaseBrInst->getSuccessor(1);
  } else {
    // ICMP_NE
    MainBB = BaseBrInst->getSuccessor(1);
    SecondIfBB = BaseBrInst->getSuccessor(0);
  }

  if (MainBB == SecondIfBB)
    return false;

  if (SecondIfBB->getSinglePredecessor() != FirstIfBB)
    return false;

  Value *BaseOp = isCmpPtrToNull(BaseCond);
  if (!BaseOp)
    return false;

  if (isa<PointerType>(BaseOp->getType()))
    if (!DTransInfo.isPtrToStruct(BaseOp))
      return false;

  LLVM_DEBUG(dbgs() << "DTRANS-ELIM-RO-FIELD-ACCESS: First IF BB is proven\n");

  BranchInst *FieldBrInst = dyn_cast<BranchInst>(SecondIfBB->getTerminator());
  if (!FieldBrInst || !FieldBrInst->isConditional())
    return false;

  if (FieldBrInst->getNumSuccessors() != 2)
    return false;

  ICmpInst *FieldCond = dyn_cast<ICmpInst>(FieldBrInst->getCondition());
  if (!FieldCond)
    return false;

  BasicBlock *TrueFieldBB, *UnreachableBB;
  if (FieldCond->getPredicate() == CmpInst::ICMP_EQ) {
    TrueFieldBB = FieldBrInst->getSuccessor(0);
    UnreachableBB = FieldBrInst->getSuccessor(1);
  } else {
    // ICMP_NE
    TrueFieldBB = FieldBrInst->getSuccessor(1);
    UnreachableBB = FieldBrInst->getSuccessor(0);
  }

  if (TrueFieldBB != MainBB)
    return false;

  SmallVector<BasicBlock *, 2> MainPreds(predecessors(MainBB));
  if (MainPreds.size() != 2)
    return false;

  if (MainBB == UnreachableBB)
    return false;

  if (UnreachableBB->getSinglePredecessor() != SecondIfBB)
    return false;

  BasicBlock *FinalBB = MainBB->getSingleSuccessor();
  if (!FinalBB || FinalBB != UnreachableBB->getSingleSuccessor())
    return false;

  Value *FieldOp = isCmpPtrToNull(FieldCond);
  if (!FieldOp)
    return false;

  // Check if SecondIfBB consists of 4 predefined instructions.
  // The aggregate type of BaseOp should have no safety check violations and the
  // field which is accessed in the SecondIfBB should be only read.
  if (!checkSecondIfBB(SecondIfBB, BaseOp)) {
    LLVM_DEBUG(dbgs() << "DTRANS-ELIM-RO-FIELD-ACCESS: Second IF BB "
                         "did not fit - skipping.\n");
    return false;
  }

  LLVM_DEBUG(dbgs() << "DTRANS-ELIM-RO-FIELD-ACCESS: Second IF BB is proven\n");
  // If all checks have passed, now clean up the code.
  // Delete first compare instruction and replace conditional branch with
  // unconditional.
  BranchInst *newBr = BranchInst::Create(MainBB);
  ReplaceInstWithInst(BaseBrInst, newBr);
  BaseCond->eraseFromParent();
  DeleteDeadBlock(SecondIfBB);
  DeleteDeadBlock(UnreachableBB);
  LLVM_DEBUG({
    dbgs() << "DTRANS-ELIM-RO-FIELD-ACCESS: Success\n";
    FirstIfBB->dump();
    MainBB->dump();
    FinalBB->dump();
  });

  return true;
}

template <class InfoClass>
bool EliminateROFieldAccessImpl<InfoClass>::run(Module &M,
                                                WholeProgramInfo &WPInfo) {

  if (!WPInfo.isWholeProgramSafe())
    return false;

  bool Changed = false;
  for (auto &F : M) {
    if (F.isDeclaration())
      continue;
    LLVM_DEBUG(dbgs() << "DTRANS-ELIM-RO-FIELD-ACCESS: Analysing "
                      << F.getName() << "\n");

    for (BasicBlock &BB : F)
      if (visit(&BB))
        Changed = true;
  }
  return Changed;
}

namespace llvm {
namespace dtrans {

bool EliminateROFieldAccessPass::runImpl(Module &M, DTransAnalysisInfo &DTInfo,
                                         WholeProgramInfo &WPInfo) {
  if (!DTInfo.useDTransAnalysis())
    return false;
  DTransAnalysisInfoAdapter AIAdaptor(DTInfo);
  EliminateROFieldAccessImpl<DTransAnalysisInfoAdapter>
      EROFieldAccessI(AIAdaptor);
  return EROFieldAccessI.run(M, WPInfo);
}

PreservedAnalyses EliminateROFieldAccessPass::run(Module &M,
                                                  ModuleAnalysisManager &AM) {
  auto &DTransInfo = AM.getResult<DTransAnalysis>(M);
  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);

  if (!runImpl(M, DTransInfo, WPInfo))
    return PreservedAnalyses::all();
  // TODO: Mark the actual preserved analyses.
  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}
} // namespace dtrans

namespace dtransOP {

bool EliminateROFieldAccessPass::runImpl(Module &M, DTransSafetyInfo &DTInfo,
                                         WholeProgramInfo &WPInfo) {
  if (!DTInfo.useDTransSafetyAnalysis())
    return false;
  DTransSafetyInfoAdapter AIAdaptor(DTInfo);
  EliminateROFieldAccessImpl<DTransSafetyInfoAdapter>
      EROFieldAccessI(AIAdaptor); 
  return EROFieldAccessI.run(M, WPInfo);
}

PreservedAnalyses EliminateROFieldAccessPass::run(Module &M,
                                                  ModuleAnalysisManager &AM) {
  auto &DTransInfo = AM.getResult<DTransSafetyAnalyzer>(M);
  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);

  if (!runImpl(M, DTransInfo, WPInfo))
    return PreservedAnalyses::all();
  // TODO: Mark the actual preserved analyses.
  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}
} // namespace dtransOP
} // namespace llvm
