//===------- Intel_IPCloningAnalysis.cpp - IP Cloning Analysis -*------===//
//
// Copyright (C) 2016-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
// This file does perform IP Cloning.
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/IPO/Intel_IPCloning.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/Intel_AggInline.h"
#include "llvm/Analysis/Intel_Andersens.h"
#include "llvm/Analysis/Intel_IPCloningAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/Type.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include <sstream>
#include <string>

using namespace llvm;
using namespace llvm::llvm_cloning_analysis;

#define DEBUG_TYPE "ipcloning"

namespace llvm {
namespace llvm_cloning_analysis {
// Option to trace IP Cloning
cl::opt<bool> IPCloningTrace("print-ip-cloning", cl::ReallyHidden);
}
}

// Enable Loop related heuristic for Cloning.
static cl::opt<bool> IPCloningLoopHeuristic("ip-cloning-loop-heuristic",
                                   cl::init(true), cl::ReallyHidden);

// Enable switch related heuristic for Cloning.
static cl::opt<bool> IPCloningSwitchHeuristic("ip-cloning-switch-heuristic",
                                   cl::init(true), cl::ReallyHidden);

// Enable IF related heuristic for Cloning.
static cl::opt<bool> IPCloningIFHeuristic("ip-cloning-if-heuristic",
                                   cl::init(true), cl::ReallyHidden);

// Maximum number of formal uses explored while collecting formals that
// are candidates for cloning using different heuristics.
static cl::opt<unsigned> IPCloningNumOfFormalUsesExploredLimit(
        "ip-cloning-num-formal-uses-explored-limit",
                                   cl::init(30), cl::ReallyHidden);

// Maximum allowed number of PHI nodes at any Callsite for specialization
// cloning.
static cl::opt<unsigned> IPSpeCloningPhiLimit(
          "ip-spe-cloning-phi-limit", cl::init(3), cl::ReallyHidden);

// List of uses of a formal that will become potentail constant values
// after cloning.
SmallPtrSet<Value*, 16> PotentialConstValuesAfterCloning;

// Returns true if 'I' is safe instruction for specialization cloning.
// It is used to decide whether a formal is valid to enable
// specialization cloning.
//
static bool isSpecializationCloningSafeInst(Instruction* I) {
  if (!isa<ICmpInst>(I))
    return true;

  return false;
}

// Returns true if it safe to enable specialization cloning for given
// 'Arg' formal.
//
static bool isSpecializationCloningSafeArgument(Argument* Arg) {

  auto PTy = Arg->getType();
  // Non pointer is always safe.
  if (!PTy->isPointerTy()) return true;

  // Check for pointer to char array.
  if (!isPointerToCharArray(PTy)) return false;

  // Returns true if no uses.
  if (Arg->use_empty()) return true;

  // Check for this attribute that indicates never to escape from the callee.
  if (!Arg->hasNoCaptureAttr()) return false;

  // Check for this attribute that indicates that the function does not
  // write through this pointer argument
  if (!Arg->onlyReadsMemory()) return false;

  // May not require below checks since NoCapture and OnlyReads attributes
  // but it doesn't hurt.
  SmallVector<LoadInst*, 16> Loads;

  // Just look at all uses to makes sure it is not escaped.
  for (Use &U : Arg->uses()) {
    User *UR = U.getUser();
    if (LoadInst *LI = dyn_cast<LoadInst>(UR)) {
      if (!LI->isSimple()) return false;
      Loads.push_back(LI);
    } else if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(UR)) {
      for (User *GEPU : GEP->users())
        if (LoadInst *LI = dyn_cast<LoadInst>(GEPU)) {
          if (!LI->isSimple()) return false;
          Loads.push_back(LI);
        } else {
          return false;
        }
    } else {
      return false;
    }
  }

  if (Loads.empty()) return true;

  for (LoadInst *Load : Loads) {
    if (!isSpecializationCloningSafeInst(Load)) {
      return false;
    }
  }
  return true;
}

// Returns true if all PHINodes in 'PhiValues' are defined in same
// BasicBlock. This is used as one of the constraints to collect
// argument sets for specialization cloning.
//
static bool allPhisDefinedInSameBB(SmallPtrSet<Value *, 8> &PhiValues) {
  BasicBlock *BB = nullptr;
  for (auto I = PhiValues.begin(), E = PhiValues.end(); I != E; ++I) {
    auto Inst = cast<Instruction>(*I);
    if (BB == nullptr) {
       BB = Inst->getParent();
       continue;
    }
    if (BB != Inst->getParent()) {
      return false;
    }
  }
  return true;
}

// This routine collects uses of 'V' that are SExt/ZExt instructions
// and adds them to PotentialConstValuesAfterCloning. 'NumUsesExplored'
// is used to limit number of uses explored.
//
// This basically helps to handle cases like below:
//   define internal fastcc void @bar(i32 %ub) unnamed_addr #1 {
//   entry:
//     %add = add i32 %ub, 20
//     ...
//     %wide.trip.count = zext i32 %add to i64  ; It is used as TripCount
//     ...
//     %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
//
static void collectSextZextAsPotentialConstants(Value* V,
                                                unsigned& NumUsesExplored) {
  for (auto *U : V->users()) {

    if (NumUsesExplored >= IPCloningNumOfFormalUsesExploredLimit)
      break;

    NumUsesExplored++;

    if (isa<SExtInst>(U) || isa<ZExtInst>(U)) {

      PotentialConstValuesAfterCloning.insert(U);
      LLVM_DEBUG(dbgs() <<  "     SExt/ZExt:  " << *U << "\n");
    }
  }
}

// It collects uses of given formal variable 'V' that will become
// constant values after cloning.
//
static void collectPotentialConstantsAfterCloning(Value *V) {
  unsigned NumUsesExplored = 0;

  // Add formal value as potential constant value after cloning
  PotentialConstValuesAfterCloning.insert(V);
  LLVM_DEBUG(dbgs() <<  "     Added original formal:  " << *V << "\n");

  // Look at all uses of formal value and try to find potential
  // constant values
  for (auto *U : V->users()) {

    // Avoid huge lists
    if (NumUsesExplored >= IPCloningNumOfFormalUsesExploredLimit)
      break;

    NumUsesExplored++;

    if (isa<UnaryInstruction>(U) || isa<CastInst>(U) || isa<BitCastInst>(U)) {
      // Add simple Unary operator as potential constants
      PotentialConstValuesAfterCloning.insert(U);
      LLVM_DEBUG(dbgs() <<  "     Unary:  " << *U << "\n");
      // Consider SExt/ZExt as potential constants
      collectSextZextAsPotentialConstants(U, NumUsesExplored);
    }
    else if (isa<BinaryOperator>(U)) {
      Value *LHS = U->getOperand(0), *RHS = U->getOperand(1);
      // Add it if other operand is constant
      if (isa<Constant>(LHS) || isa<Constant>(RHS)) {
        PotentialConstValuesAfterCloning.insert(U);
        LLVM_DEBUG(dbgs() <<  "     Binary:   " << *U << "\n");
        // Consider SExt/ZExt as potential constants
        collectSextZextAsPotentialConstants(U, NumUsesExplored);
      }
    }
  }
}

// Returns true if user 'User' of 'V' satisfies IF related heuristics
// For now, it returns true if 'User' is IcmpInst and the result is used
// by any BranchInst.
//
//  Ex:  Returns true for below example
//    V = formal + 20;
//    User:  if (V  <  30) {
//           }
//
static bool applyIFHeurstic(Value *User, Value *V) {

  if (!IPCloningIFHeuristic)
    return false;
  auto ICmp = dyn_cast<ICmpInst>(User);
  if (!ICmp)
    return false;
  for (auto *W : ICmp->users()) {
    auto BI = dyn_cast<BranchInst>(W);
    if (!BI || !BI->isConditional())
      continue;
    LLVM_DEBUG({
      dbgs() << "  Used in IF: " << *User << "\n";
      dbgs() << "      Branch: " << *BI << "\n";
    });
    return true;
  }
  return false;
}

// Returns condition of given Loop 'L' if it finds. Otherwise, returns
// nullptr.
//
static ICmpInst *getLoopTest(Loop *L) {
  if (!L->getExitingBlock()) return nullptr;
  if (!L->getExitingBlock()->getTerminator()) return nullptr;

  BranchInst *BI = dyn_cast<BranchInst>(L->getExitingBlock()->getTerminator());

  // Branch may not have condition in some rare cases.
  // Ex:
  //    for (i = 0; ; i++) {
  //      unsigned test = EH_RETURN_DATA_REGNO (i);
  //      if (test == INVALID_REGNUM)    break;
  //      if (test == (unsigned) regno) return 1;
  //    }
  //
  if (!BI || !BI->isConditional()) return nullptr;
  return dyn_cast<ICmpInst>(BI->getCondition());
}

// Returns true if user 'User' of 'V' satisfies LOOP related heuristics
// For now, it returns true if 'User' is conditional statement of a Loop
// or 'V' is used as UB.
//
//   Ex: Returns true for the below example
//          V = formal + 2;
//          for (;
//   User:            i < V; ) {
//             ...
//          }
//
static bool applyLoopHeuristic(Value *User, Value *V, LoopInfo* LI) {

  if (!IPCloningLoopHeuristic)
    return false;

  // Check if it is IcmpInst
  auto U = cast<Instruction>(User);
  if (!isa<ICmpInst>(U))
    return false;
  auto LHS = U->getOperand(0);
  auto RHS = U->getOperand(1);
  if (V != LHS && V != RHS)
    return false;

  // Check if IcmpInst is used as Loop condition
  BasicBlock *BB = U->getParent();
  if (!BB)
    return false;
  Loop *L = LI->getLoopFor(BB);
  if (L == nullptr)
    return false;
  ICmpInst *Cond = getLoopTest(L);
  if (!Cond)
    return false;
  if (Cond != U)
    return false;
  LLVM_DEBUG(dbgs() << "  Used in Loop: " << *U << "\n";);
  return true;
}

//
// Return 'true' if 'V' has some user that satisfies the loop heuristic test.
//
static bool applyLoopHeuristic(Value *V, LoopInfo* LI) {
  for (User *U : V->users())
    if (applyLoopHeuristic(U, V, LI))
      return true;
  return false;
}

// Returns true if user 'User' of 'V' satisfies SWITCH related heuristics
// For now, it returns true if 'User' is switch statement and 'V' is
// used as condition.
//
// Ex: Return true for the below example
//           V = formal + 1;
// User:     switch (V) {
//            ...
//           }
//
static bool applySwitchHeuristic(Value *User, Value *V) {

  if (!IPCloningSwitchHeuristic)
    return false;

  auto U = cast<Instruction>(User);

  // Check if 'V' is used as condition of SwitchInst
  if (!isa<SwitchInst>(U))
    return false;
  SwitchInst &SI = cast<SwitchInst>(*U);
  if (V != SI.getCondition())
    return false;

  LLVM_DEBUG(dbgs() << "  Used in Switch: " << *U << "\n");
  return true;
}

// Returns true if any user of 'V' satisfies any heuristics.
//
static bool applyIFSwitchHeuristics(Function &F, Value *V,
                                    unsigned &IFCount, unsigned &SwitchCount) {
  IFCount = 0;
  SwitchCount = 0;
  for (User *U : V->users()) {
    if (applyIFHeurstic(U, V))
      ++IFCount;
    if (applySwitchHeuristic(U, V))
      ++SwitchCount;
  }
  LLVM_DEBUG({
    if (IFCount || SwitchCount)
      dbgs() << "IFSwitch: " << F.getName() << " "
             << IFCount << " " << SwitchCount << "\n";
  });
  return IFCount + SwitchCount > 0;
}

namespace llvm {
namespace llvm_cloning_analysis {

// Return true if 'PTy' is pointer to array of chars.
//
extern bool isPointerToCharArray(Type* PTy) {
  // Not pointer?
  if (!isa<PointerType>(PTy)) return false;

  // Is it pointer to array?
  auto ATy = cast<PointerType>(PTy)->getElementType();
  if (!isa<ArrayType>(ATy)) return false;

  // Is is pointer to array of char?
  auto CTy = cast<ArrayType>(ATy)->getElementType();
  if (!CTy->isIntegerTy(8)) return false;

  return true;
}

// Returns any GEP operand of 'Phi' if it finds one. Otherwise, returns
// nullptr.
extern GetElementPtrInst* getAnyGEPAsIncomingValueForPhi(Value *Phi) {

  if (!isa<PHINode>(Phi)) return nullptr;

  PHINode* PN = cast<PHINode>(Phi);

  for (unsigned i = 0, e = PN->getNumIncomingValues(); i != e; ++i) {
    BasicBlock* PredBB = PN->getIncomingBlock(i);
    Value* Val = PN->getIncomingValueForBlock(PredBB);
    if (auto *GEP = dyn_cast<GetElementPtrInst>(Val))
      return GEP;
  }
  return nullptr;
}

// Returns true if 'Arg' is considered as constant for
// cloning based on SpecializationClone.
extern bool isConstantArgWorthyForSpecializationClone(Value *Arg) {
  Type *PhiTy = Arg->getType();

  if (PhiTy->isIntegerTy()) return true;
  if (!PhiTy->isPointerTy()) return false;

  Type *Ty = PhiTy->getPointerElementType();
  if (!Ty->isArrayTy()) return false;

  // Makes sure at least one operand of Phi is GEP for
  // pointer type arguments.
  if (getAnyGEPAsIncomingValueForPhi(Arg) == nullptr) return false;

  return true;
}

// Collect possible PHINode candidates for specialization cloning
// at CallBase 'CB' for routine 'F' and save them in 'PhiValues'.
//
extern bool collectPHIsForSpecialization(Function &F, CallBase &CB,
                                       SmallPtrSet<Value *, 8>& PhiValues) {

  LLVM_DEBUG(dbgs() << "   Analyzing for Spe Cloning: " << CB << "\n");

  auto CAI = CB.arg_begin();
  for (Function::arg_iterator AI = F.arg_begin(), E = F.arg_end();
       AI != E; ++AI, ++CAI) {

    if (!isa<PHINode>(*CAI))
      continue;

    if (!isConstantArgWorthyForSpecializationClone(*CAI))
      continue;

    if (!isSpecializationCloningSafeArgument(&*AI))
      continue;

    PhiValues.insert(*CAI);

  }

  if (PhiValues.size() == 0) {
    LLVM_DEBUG(dbgs() << "     Skip ... No PHIs selected for Spe cloning\n");
    return false;
  }

  if (PhiValues.size() > IPSpeCloningPhiLimit) {
    LLVM_DEBUG(dbgs() << "     Skip ... Too many PHIs selected for Spe "
                         "cloning \n");
    return false;
  }

  if (!allPhisDefinedInSameBB(PhiValues)) {
    LLVM_DEBUG(dbgs() << "     Skip ... Not all PHIs in same BB for Spe "
                         "cloning\n");
    return false;
  }
  return true;
}

// First, it collects uses of 'V' that will become constant values
// after cloning. Then, it applies heuristics for all potential
// constants. It returns true if any potential constant satisfies
// heuristics.
//
extern bool findPotentialConstsAndApplyHeuristics(Function &F, Value *V,
                                                  LoopInfo* LI, bool AfterInl,
                                                  bool IFSwitchHeuristic,
                                                  unsigned *IFCount,
                                                  unsigned *SwitchCount) {
  PotentialConstValuesAfterCloning.clear();
  collectPotentialConstantsAfterCloning(V);

  // Apply loop heuristic for all potential constant values
  for (Value *V1 : PotentialConstValuesAfterCloning)
    if (applyLoopHeuristic(V1, LI))
      return true;
  if (!AfterInl || !IFSwitchHeuristic)
    return false;

  // Apply if/switch heuristics for all potential constant values
  bool ReturnValue = false;
  unsigned LocIFCount = 0;
  unsigned LocSwitchCount = 0;
  for (Value *V1 : PotentialConstValuesAfterCloning) {
    ReturnValue |= applyIFSwitchHeuristics(F, V1, LocIFCount, LocSwitchCount);
    if (IFCount)
      *IFCount += LocIFCount;
    if (SwitchCount)
      *SwitchCount += LocSwitchCount;
  }
  return ReturnValue;
}

// 'PhiValues' are candidate arguments for specialization cloning at 'CB'
// CallBase of 'F'. LoopInfo 'LI' of 'F' is used to decide whether it is
// profitable to enable specialization cloning for candidate arguments in
// 'PhiValues'. This routine removes candidate arguments from 'PhiValues'
// if it finds it is not profitable to enable cloning. Returns false if
// all candidate arguments are removed from 'PhiValues'.
extern bool applyHeuristicsForSpecialization(Function &F, CallBase &CB,
                      SmallPtrSet<Value *, 8>& PhiValues, LoopInfo* LI) {
  auto CAI1 = CB.arg_begin();
  for (Function::arg_iterator AI = F.arg_begin(), E = F.arg_end();
     AI != E; ++AI, ++CAI1) {

    if (!PhiValues.count(*CAI1))
      continue;

    if ((&*AI)->getType()->isIntegerTy() &&
        !findPotentialConstsAndApplyHeuristics(F, &*AI, LI, false, false)) {
      PhiValues.erase(*CAI1);
    }
  }

  if (PhiValues.size() == 0) {
    LLVM_DEBUG(dbgs() << "     Skip ... No PHIs selected after applying "
                         "heuristics\n");
    return false;
  }
  return true;
}

// Return true if 'CB' is a candidate for specialization cloning.
// 'LI', which is LoopInfo of callee, is used to apply heuristics.
//
extern bool isCallCandidateForSpecialization(CallBase &CB, LoopInfo* LI) {
  SmallPtrSet<Value *, 8> PhiValues;

  Function *F = CB.getCalledFunction();
  if (!F)
    return false;

  PhiValues.clear();
  if (!collectPHIsForSpecialization(*F, CB, PhiValues))
    return false;
  if (!applyHeuristicsForSpecialization(*F, CB, PhiValues, LI))
    return false;
  return true;
}

}
}

