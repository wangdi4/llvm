//===- IntelGlobalOpt.cpp - Optimize Global Variables----------------------===//
//
// Copyright (C) 2016-2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass performs register promotion for simple global
// variables that are not escaped.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/Intel_Andersens.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/ScalarEvolutionAliasAnalysis.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/GlobalStatus.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"
using namespace llvm;

#define DEBUG_TYPE "nonltoglobalopt"

static cl::opt<unsigned>
    PhiBBsThreshold("phi-bb-threshold", cl::Hidden, cl::init(3),
                    cl::desc("Control the number of the BBs having more than "
                             "two incoming edges (default = 3)"));
static cl::opt<double> CodeSizeRatioThreshold(
    "cs-size-ratio-threshold", cl::Hidden, cl::init(0.096),
    cl::desc("Control the ratio of code size (default = 0.096)"));

static cl::opt<unsigned>
    NumVarsThreshold("num-vars-threshold", cl::Hidden, cl::init(5),
                    cl::desc("Control the number of static vars "
                    "converted (default = 5)"));

static cl::opt<unsigned>
    NumInsnsThreshold("num-insns-threshold", cl::Hidden, cl::init(50),
                    cl::desc("Control the number of instruction in "
                    "complex BB (default = 50)"));

STATISTIC(NumConverted, "Number of static variable converted");

namespace {
struct NonLTOGlobalOpt : public FunctionPass {
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    AU.addPreservedID(LoopSimplifyID);
    AU.addPreservedID(LCSSAID);
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<AAResultsWrapperPass>();
    AU.addPreserved<AAResultsWrapperPass>();
    AU.addPreserved<BasicAAWrapperPass>();
    AU.addPreserved<GlobalsAAWrapperPass>();
    AU.addPreserved<ScalarEvolutionWrapperPass>();
    AU.addPreserved<SCEVAAWrapperPass>();
    AU.addPreserved<AndersensAAWrapperPass>();
  };
  static char ID;
  NonLTOGlobalOpt() : FunctionPass(ID) {
    initializeNonLTOGlobalOptPass(*PassRegistry::getPassRegistry());
  }
  bool runOnFunction(Function &F);

private:
  AliasAnalysis *AA;
  bool processInternalGlobal(GlobalVariable *GV, const GlobalStatus &GS,
                             DominatorTree &DT);
  bool isGVLegalToBePromoted(GlobalVariable *GV, DominatorTree &DT,
                             SmallPtrSetImpl<Instruction *> &Stores,
                             SmallPtrSetImpl<Instruction *> &GVUsers);
  bool analyzeUseOfGV(const Value *V, SmallPtrSetImpl<Instruction *> &Stores,
                      SmallPtrSetImpl<Instruction *> &GVUsers, bool ExpectSt);
  void replaceUseOfGV(Value *Old, Value *New);
};
}

char NonLTOGlobalOpt::ID = 0;
INITIALIZE_PASS_BEGIN(NonLTOGlobalOpt, "nonltoglobalopt",
                      "Global Variable Optimizer under -O2 and above", false,
                      false)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(BasicAAWrapperPass)
INITIALIZE_PASS_DEPENDENCY(AAResultsWrapperPass)
INITIALIZE_PASS_DEPENDENCY(GlobalsAAWrapperPass)
INITIALIZE_PASS_DEPENDENCY(AndersensAAWrapperPass)
INITIALIZE_PASS_END(NonLTOGlobalOpt, "nonltoglobalopt",
                    "Global Variable Optimizer under -O2 and above", false,
                    false)

FunctionPass *llvm::createNonLTOGlobalOptimizerPass() {
  return new NonLTOGlobalOpt();
}

// Replaces the use of non-escaped global variable with register
void NonLTOGlobalOpt::replaceUseOfGV(Value *V, Value *New) {
  while (!V->user_empty()) {
    if (ConstantExpr *C = dyn_cast<ConstantExpr>(V->user_back())) {
      for (Use &U1 : C->uses()) {
        User *UR = U1.getUser();
        Instruction *I = dyn_cast<Instruction>(UR);
        Instruction *CastI =
            CastInst::CreatePointerCast(New, C->getType(), Twine(""), I);
        assert(I && "Expected non-empty incoming instruction");
        I->setOperand(1, CastI);
      }
      if (C->use_empty())
        C->destroyConstant();

    } else if (Instruction *User = cast<Instruction>(V->user_back())) {
      User->replaceUsesOfWith(V, New);
    }
  }
}

// Returns false if the use of global varialbe or
// the address is unexpected.
bool NonLTOGlobalOpt::analyzeUseOfGV(const Value *V,
                                     SmallPtrSetImpl<Instruction *> &Stores,
                                     SmallPtrSetImpl<Instruction *> &GVUsers,
                                     bool ExpectSt) {
  for (const Use &U : V->uses()) {
    User *UR = U.getUser();
    if (const ConstantExpr *CE = dyn_cast<ConstantExpr>(UR)) {
      if (ExpectSt) {
        return false;
      }
      if (CE->getOpcode() != Instruction::BitCast ||
          !CE->getType()->isPointerTy()) {
        return false;
      }
      if (!analyzeUseOfGV(CE, Stores, GVUsers, true)) {
        return false;
      }
    } else if (Instruction *I = dyn_cast<Instruction>(UR)) {
      if (StoreInst *SI = dyn_cast<StoreInst>(I)) {
        Stores.insert(SI);
      } else {
        if (ExpectSt) {
          return false;
        }
        GVUsers.insert(I);
      }
    } else {
      return false;
    }
  }
  return true;
}

// Legality check for the register promotion
bool NonLTOGlobalOpt::isGVLegalToBePromoted(
    GlobalVariable *GV, DominatorTree &DT,
    SmallPtrSetImpl<Instruction *> &Stores,
    SmallPtrSetImpl<Instruction *> &GVUsers) {
  if (!analyzeUseOfGV(GV, Stores, GVUsers, false)) {
    return false;
  }

  // If a load cannot be dominated by a store, the register promotion
  // should give up.
  for (auto GVUsr : GVUsers) {
    if (llvm::none_of(Stores,
                      [&](Instruction *St) { return DT.dominates(St, GVUsr); }))
      return false;
  }
  return true;
}

// For the scalar global varialbe, the compiler checkes whether
// it is legal to be promoted into the register. If so,
// replace the use of global variable with registers.
bool NonLTOGlobalOpt::processInternalGlobal(GlobalVariable *GV,
                                            const GlobalStatus &GS,
                                            DominatorTree &DT) {
  SmallPtrSet<Instruction *, 8> Stores;
  SmallPtrSet<Instruction *, 8> GVUsers;

  if (GV->getType()->getElementType()->isSingleValueType() &&
      GV->getType()->getAddressSpace() == 0) {

    if (!isGVLegalToBePromoted(GV, DT, Stores, GVUsers))
      return false;

    Instruction &FirstI = const_cast<Instruction &>(
        *GS.AccessingFunction->getEntryBlock().begin());
    Type *ElemTy = GV->getType()->getElementType();
    AllocaInst *Alloca =
        new AllocaInst(ElemTy, nullptr, GV->getName(), &FirstI);
    if (!isa<UndefValue>(GV->getInitializer()))
      new StoreInst(GV->getInitializer(), Alloca, &FirstI);
    replaceUseOfGV(GV, Alloca);
    GV->eraseFromParent();
    return true;
  }
  return false;
}

// Promotes the global variables into registers if it is legal.
bool NonLTOGlobalOpt::runOnFunction(Function &F) {
  if (skipFunction(F))
    return false;

  bool Changed = false;
  Module *M = F.getParent();


  // The intel-GlobalOpt should abort if the ReturnsTwice functions such as
  // setjmp are present.
  if (F.callsFunctionThatReturnsTwice())
    return Changed;

  unsigned NumBBsHasMoreThanTwoIncomingEdges = 0;
  unsigned NumBBsHasTwoIncomingEdges = 0;
  unsigned NumInsns = 0;

  for (Function::iterator B = F.begin(), BE = F.end(); B != BE; ++B) {
    bool BBHasTwoIncomingEdges = false;
    unsigned NumPreds = std::distance(pred_begin(&*B), pred_end(&*B));
    if (NumPreds > 2)
      NumBBsHasMoreThanTwoIncomingEdges++;
    else if (NumPreds == 2) {
      NumBBsHasTwoIncomingEdges++;
      BBHasTwoIncomingEdges = true;
    }
    for (BasicBlock::iterator I = B->begin(), IE = B->end(); I != IE; ++I) {
      if (dyn_cast<FenceInst>(I))
        return Changed;
      if (BBHasTwoIncomingEdges)
        NumInsns++;
    }
  }

  // The transformation of global variable to local variable may increase the
  // register pressure and thus cause performance regression in the case of
  // complex control flows. The code here is to inhibit the transformation under
  // this situation. This workaround should be removed after the register
  // allocation issue is fixed.

  bool PossibleBailOut = false;
  if (NumBBsHasMoreThanTwoIncomingEdges > PhiBBsThreshold ||
      (NumBBsHasMoreThanTwoIncomingEdges == PhiBBsThreshold &&
      NumInsns<=NumInsnsThreshold))
    return Changed;
  if (NumInsns > 0 && (double)NumBBsHasTwoIncomingEdges / (double)NumInsns >
                          CodeSizeRatioThreshold)
    PossibleBailOut = true;

  AA = &getAnalysis<AAResultsWrapperPass>().getAAResults();
  DominatorTree &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();

  unsigned NumToBeTransformed=0;
  if (PossibleBailOut) {
    for (Module::global_iterator GVI = M->global_begin(), E = M->global_end();
         GVI != E;) {
      GlobalVariable *GV = &*(GVI++);
      if (AA->escapes(GV))
        continue;
      NumToBeTransformed++;
    }
    if (NumToBeTransformed < NumVarsThreshold ||
        NumBBsHasMoreThanTwoIncomingEdges < PhiBBsThreshold)
      NumToBeTransformed = NumVarsThreshold/2;
    else
      NumToBeTransformed = NumVarsThreshold;
  }

  unsigned LocalNumConverted = 0;
  for (Module::global_iterator GVI = M->global_begin(), E = M->global_end();
       GVI != E;) {
    GlobalVariable *GV = &*(GVI++);
    if (AA->escapes(GV))
      continue;

    GlobalStatus GS;
    if (GlobalStatus::analyzeGlobal(GV, GS)) {
      continue;
    }

    if (GS.AccessingFunction != &F) {
      continue;
    }

    if (GV->isConstant() || !GV->hasInitializer())
      continue;

    if (PossibleBailOut && LocalNumConverted >= NumToBeTransformed)
      return Changed;

    Changed |= processInternalGlobal(GV, GS, DT);
    LocalNumConverted++;
    NumConverted++; // For statistics.
  }
  return Changed;
}
