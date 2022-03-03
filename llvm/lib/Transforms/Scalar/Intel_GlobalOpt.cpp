//===- Intel_GlobalOpt.cpp - Optimize Global Variables --------------------===//
//
// Copyright (C) 2016-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass performs alloca promotion for simple global
// variables that are not escaped, live-in to the function, or loaded through
// recursion.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Scalar/Intel_GlobalOpt.h"
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
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/GlobalStatus.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"
#include "llvm/Analysis/ScalarEvolution.h"
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
class NonLTOGlobalOptImpl {
  AliasAnalysis &AA;
  DominatorTree &DT;

public:
  NonLTOGlobalOptImpl(AliasAnalysis &AA, DominatorTree &DT)
    : AA(AA), DT(DT) {}
  bool run(Function &F);

private:
  bool processInternalGlobal(GlobalVariable *GV, const GlobalStatus &GS);
  bool isGVLegalToBePromoted(GlobalVariable *GV,
                             SmallPtrSetImpl<Instruction *> &Stores,
                             SmallPtrSetImpl<Instruction *> &GVUsers);
  bool analyzeUseOfGV(const Value *V, SmallPtrSetImpl<Instruction *> &Stores,
                      SmallPtrSetImpl<Instruction *> &GVUsers, bool ExpectSt);
  void replaceUseOfGV(Value *Old, Value *New);
};
}

// Replaces the use of non-escaped global variable with register
void NonLTOGlobalOptImpl::replaceUseOfGV(Value *V, Value *New) {
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
bool NonLTOGlobalOptImpl::analyzeUseOfGV(const Value *V,
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
bool NonLTOGlobalOptImpl::isGVLegalToBePromoted(
    GlobalVariable *GV,
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

// For the scalar global variable, the compiler checks whether
// it is legal to be promoted into the register. If so,
// replace the use of global variable with registers.
bool NonLTOGlobalOptImpl::processInternalGlobal(GlobalVariable *GV,
                                            const GlobalStatus &GS) {
  SmallPtrSet<Instruction *, 8> Stores;
  SmallPtrSet<Instruction *, 8> GVUsers;

  if (GV->getValueType()->isSingleValueType() &&
      GV->getType()->getAddressSpace() == 0) {

    if (!isGVLegalToBePromoted(GV, Stores, GVUsers))
      return false;

    LLVM_DEBUG(dbgs() << "Promoting: " << GV->getName() << "\n");

    Instruction &FirstI = const_cast<Instruction &>(
        *GS.AccessingFunction->getEntryBlock().begin());
    Type *ElemTy = GV->getValueType();
    const DataLayout &DL = GS.AccessingFunction->getParent()->getDataLayout();
    AllocaInst *Alloca =
        new AllocaInst(ElemTy,
                       DL.getAllocaAddrSpace(),
                       nullptr,
                       GV->getName(), &FirstI);
    if (!isa<UndefValue>(GV->getInitializer()))
      new StoreInst(GV->getInitializer(), Alloca, &FirstI);
    replaceUseOfGV(GV, Alloca);
    GV->eraseFromParent();
    return true;
  }
  return false;
}

// Promotes the global variables into registers if it is legal.
bool NonLTOGlobalOptImpl::run(Function &F) {
  bool Changed = false;
  Module *M = F.getParent();

  // The intel-GlobalOpt should abort if the ReturnsTwice functions such as
  // setjmp are present.
  if (F.callsFunctionThatReturnsTwice())
    return Changed;

  unsigned NumBBsHasMoreThanTwoIncomingEdges = 0;
  unsigned NumBBsHasTwoIncomingEdges = 0;
  unsigned NumInsns = 0;

  auto FIsLocalNotAddrTaken = [](Function &F) {
    if (!F.isDSOLocal())
      return false;
    for (auto &U : F.uses())
      if (!isa<CallBase>(U.getUser()))
        return false;
    return true;
  };

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

      // CMPLRLLVM-24756: If this function may recurse, we cannot replace
      // globals with stack.
      if (auto *CI = dyn_cast<CallInst>(I)) {
        if (!F.doesNotRecurse()) {
          auto *Callee = CI->getCalledFunction();
          // 526.blender: assume some cases don't recurse.
          // Notably, if F is local, not address-taken and the call is to a
          // non-local function.
          // setjmp has already been checked.
          if (!Callee || Callee->isIntrinsic() ||
              (FIsLocalNotAddrTaken(F) && Callee->isDeclaration()))
            continue;
          LLVM_DEBUG(dbgs() << F.getName() << " may recurse on callee: ");
          LLVM_DEBUG(dbgs() << Callee->getName() << "\n");
          return Changed;
        }
      }
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

  unsigned NumToBeTransformed=0;
  if (PossibleBailOut) {
    for (Module::global_iterator GVI = M->global_begin(), E = M->global_end();
         GVI != E;) {
      GlobalVariable *GV = &*(GVI++);
      if (AA.escapes(GV))
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
    if (AA.escapes(GV))
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

    Changed |= processInternalGlobal(GV, GS);
    LocalNumConverted++;
    NumConverted++; // For statistics.
  }
  return Changed;
}

PreservedAnalyses
NonLTOGlobalOptPass::run(Function &F, FunctionAnalysisManager &AM) {
  auto &AA = AM.getResult<AAManager>(F);
  auto &DT = AM.getResult<DominatorTreeAnalysis>(F);

  if (!NonLTOGlobalOptImpl(AA, DT).run(F))
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserveSet<CFGAnalyses>();
  PA.preserve<AAManager>();
  PA.preserve<BasicAA>();
  PA.preserve<GlobalsAA>();
  PA.preserve<ScalarEvolutionAnalysis>();
  PA.preserve<SCEVAA>();
  PA.preserve<AndersensAA>();
  return PA;
}

namespace {
class NonLTOGlobalOptLegacyPass : public FunctionPass {
public:
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
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
  NonLTOGlobalOptLegacyPass() : FunctionPass(ID) {
    initializeNonLTOGlobalOptLegacyPassPass(*PassRegistry::getPassRegistry());
  }
  bool runOnFunction(Function &F) override;
};
}

char NonLTOGlobalOptLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(NonLTOGlobalOptLegacyPass, "nonltoglobalopt",
                      "Global Variable Optimizer under -O2 and above", false,
                      false)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(BasicAAWrapperPass)
INITIALIZE_PASS_DEPENDENCY(AAResultsWrapperPass)
INITIALIZE_PASS_DEPENDENCY(GlobalsAAWrapperPass)
INITIALIZE_PASS_DEPENDENCY(AndersensAAWrapperPass)
INITIALIZE_PASS_END(NonLTOGlobalOptLegacyPass, "nonltoglobalopt",
                    "Global Variable Optimizer under -O2 and above", false,
                    false)

FunctionPass *llvm::createNonLTOGlobalOptimizerPass() {
  return new NonLTOGlobalOptLegacyPass();
}

bool NonLTOGlobalOptLegacyPass::runOnFunction(Function &F) {
  if (skipFunction(F))
    return false;

  auto &AA = getAnalysis<AAResultsWrapperPass>().getAAResults();
  auto &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();

  return NonLTOGlobalOptImpl(AA, DT).run(F);
}
