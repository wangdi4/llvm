//===- IntelGlobalOpt.cpp - Optimize Global Variables
//--------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
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
  bool DominateF;
  if (!analyzeUseOfGV(GV, Stores, GVUsers, false)) {
    return false;
  }

  // If a load cannot be dominated by a store, the register promtion
  // should give up.
  if (!GVUsers.empty()) {
    for (auto GVUsr : GVUsers) {
      DominateF = false;
      if (!Stores.empty()) {
        for (auto St : Stores) {
          if (DT.dominates(St, GVUsr)) {
            DominateF = true;
          }
        }
      }
      if (DominateF == false) {
        return false;
      }
    }
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
  bool Changed = false;
  Module *M = F.getParent();

  for (Function::iterator B = F.begin(), BE = F.end(); B != BE; ++B) {
    for (BasicBlock::iterator I = B->begin(), IE = B->end(); I != IE; ++I) {
      if (dyn_cast<FenceInst>(I))
        return Changed;
    }
  }

  AA = &getAnalysis<AAResultsWrapperPass>().getAAResults();
  DominatorTree &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();
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

    Changed |= processInternalGlobal(GV, GS, DT);
  }
  return Changed;
}
