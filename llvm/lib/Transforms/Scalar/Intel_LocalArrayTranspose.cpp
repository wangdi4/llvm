//===-- Intel_LocalArrayTransposePass.cpp - LocalArrayTransposePass -------===//
//
// Copyright (C) 2023 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
// Implements analysis and transformation for transpose of local Fortran
// style allocate dope vector based arrays.

#include "llvm/Transforms/Scalar/Intel_LocalArrayTranspose.h"
#include "llvm/Analysis/Intel_DopeVectorAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/IPO.h"

using namespace llvm;
using namespace dvanalysis;

#define DEBUG_TYPE "local-array-transpose"

bool LocalArrayTransposePass::isSquareAllocatableArray(
    AllocaInst *AI,
    std::function<const TargetLibraryInfo &(Function &)> GetTLI) {
  uint32_t ArRank = 0;
  Type *AIType = AI->getAllocatedType();
  const DataLayout &DL = AI->getFunction()->getParent()->getDataLayout();
  if (!isDopeVectorType(AIType, DL, &ArRank))
    return false;
  if (ArRank != 2)
    return false;
  DopeVectorAnalyzer DVAI(AI, nullptr, GetTLI);
  DVAI.analyze(/*ForCreation=*/true, /*IsLocal=*/true,
               /*AfterInstCombine*/ true);
  if (!DVAI.getIsValid())
    return false;
  Value *NILB = DVAI.getNonInitLowerBound(0);
  if (!NILB || NILB != DVAI.getNonInitLowerBound(1))
    return false;
  Value *NIEX = DVAI.getNonInitExtent(0);
  if (!NIEX || NIEX != DVAI.getNonInitExtent(1))
    return false;
  Value *NIS0 = DVAI.getNonInitStride(0);
  if (!NIS0)
    return false;
  auto S0 = dyn_cast<ConstantInt>(NIS0);
  if (!S0)
    return false;
  Value *V = DVAI.getNonInitStride(1);
  if (!V)
    return false;
  auto BO = dyn_cast<BinaryOperator>(V);
  if (!BO)
    return false;
  // Check that Stride[1] = Stride[0] * Extent[0]
  Value *V0 = nullptr;
  Value *V1 = nullptr;
  ConstantInt *CI1 = nullptr;
  unsigned CV = 0;
  Value *NCI = nullptr;
  switch (BO->getOpcode()) {
  case Instruction::Mul:
    V0 = BO->getOperand(0);
    V1 = BO->getOperand(1);
    if (auto CI0 = dyn_cast<ConstantInt>(V0)) {
      CV = CI0->getZExtValue();
      NCI = V1;
    } else if (auto CI1 = dyn_cast<ConstantInt>(V1)) {
      CV = CI1->getZExtValue();
      NCI = V0;
    } else {
      return false;
    }
    break;
  case Instruction::Shl:
    CI1 = dyn_cast<ConstantInt>(BO->getOperand(1));
    if (!CI1)
      return false;
    CV = 1 << CI1->getZExtValue();
    NCI = BO->getOperand(0);
    break;
  default:
    return false;
  }
  if (CV != S0->getZExtValue())
    return false;
  if (NCI != NIEX)
    return false;
  return true;
}

CallBase *
LocalArrayTransposePass::findAllocHandle(GetElementPtrInst *GEPI,
                                         const TargetLibraryInfo &TLI) {
  CallBase *CBRet = nullptr;
  for (User *U : GEPI->users())
    if (auto CB = dyn_cast<CallBase>(U)) {
      Function *F = CB->getCalledFunction();
      if (!F)
        continue;
      LibFunc TheLibFunc;
      if (TLI.getLibFunc(F->getName(), TheLibFunc) && TLI.has(TheLibFunc) &&
          TheLibFunc == LibFunc_for_alloc_allocatable_handle) {
        if (CB->getArgOperand(1) != GEPI)
          continue;
        if (CBRet)
          return nullptr;
        CBRet = CB;
      }
    }
  return CBRet;
}

CallBase *
LocalArrayTransposePass::findDeallocHandle(GetElementPtrInst *GEPI,
                                           const TargetLibraryInfo &TLI) {
  CallBase *CBRet = nullptr;
  for (User *U0 : GEPI->users())
    if (auto LI = dyn_cast<LoadInst>(U0))
      for (User *U1 : LI->users())
        if (auto CB = dyn_cast<CallBase>(U1)) {
          Function *F = CB->getCalledFunction();
          if (!F)
            continue;
          LibFunc TheLibFunc;
          if (TLI.getLibFunc(F->getName(), TheLibFunc) && TLI.has(TheLibFunc) &&
              TheLibFunc == LibFunc_for_dealloc_allocatable_handle) {
            if (CB->getArgOperand(0) != LI)
              continue;
            if (CBRet)
              return nullptr;
            CBRet = CB;
          }
        }
  return CBRet;
}

bool LocalArrayTransposePass::isValidSubscriptInst(SubscriptInst *SBI,
                                                   LoadInst *LI) {
  if (SBI->getPointerOperand() != LI)
    return false;
  if (!SBI->hasOneUse())
    return false;
  if (!isa<SubscriptInst>(SBI->user_back()))
    return false;
  return true;
}

bool LocalArrayTransposePass::isValidPHINode(PHINode *PHIN, CallBase *CB1) {
  unsigned NIV = PHIN->getNumIncomingValues();
  if (NIV != 2)
    return false;
  bool foundDealloc = false;
  BasicBlock *BBNull = nullptr;
  BasicBlock *BBNonNull = nullptr;
  for (unsigned I = 0, E = NIV; I < E; ++I) {
    Value *V = PHIN->getIncomingValue(I);
    if (auto CV = dyn_cast<Constant>(V)) {
      if (!CV->isNullValue())
        return false;
      BBNull = PHIN->getIncomingBlock(I);
      auto BI = dyn_cast<BranchInst>(BBNull->getTerminator());
      if (!BI || !BI->isUnconditional())
        return false;
      if (BI->getSuccessor(0) != PHIN->getParent())
        return false;
    } else {
      BBNonNull = PHIN->getIncomingBlock(I);
      Instruction *II = BBNonNull->getTerminator();
      for (; II; II = II->getPrevNonDebugInstruction())
        if (II == CB1) {
          foundDealloc = true;
          break;
        }
    }
  }
  auto BI = dyn_cast<BranchInst>(BBNonNull->getTerminator());
  if (!BI || !BI->isConditional())
    return false;
  if (BI->getSuccessor(0) != BBNull)
    return false;
  if (BI->getSuccessor(1) != PHIN->getParent())
    return false;
  auto ICI = dyn_cast<ICmpInst>(BI->getCondition());
  if (!ICI || ICI->getPredicate() != ICmpInst::ICMP_EQ)
    return false;
  if (ICI->getOperand(0) != CB1)
    return false;
  auto IC0 = dyn_cast<ConstantInt>(ICI->getOperand(1));
  if (!IC0 || !IC0->isZero())
    return false;
  return foundDealloc;
}

bool LocalArrayTransposePass::isValidStoreInst(StoreInst *SI,
                                               GetElementPtrInst *GEPI,
                                               const TargetLibraryInfo &TLI) {

  if (SI->getPointerOperand() != GEPI)
    return false;
  auto SV = dyn_cast<Constant>(SI->getValueOperand());
  if (!SV || !SV->isNullValue())
    return false;
  if (SI->getPrevNonDebugInstruction() == GEPI)
    return true;
  BasicBlock *BB = SI->getParent()->getSinglePredecessor();
  if (!BB)
    return false;
  CallBase *CBDealloc = nullptr;
  for (auto II = BB->getTerminator(); II; II = II->getPrevNonDebugInstruction())
    if (auto CB = dyn_cast<CallBase>(II)) {
      Function *F = CB->getCalledFunction();
      if (!F)
        continue;
      LibFunc TheLibFunc;
      if (TLI.getLibFunc(F->getName(), TheLibFunc) && TLI.has(TheLibFunc) &&
          TheLibFunc == LibFunc_for_dealloc_allocatable_handle) {
        CBDealloc = CB;
        break;
      }
    }
  if (!CBDealloc)
    return false;
  Value *V = CBDealloc->getArgOperand(0);
  if (auto PHIN = dyn_cast<PHINode>(V)) {
    for (unsigned I = 0, E = PHIN->getNumIncomingValues(); I < E; I++) {
      Value *VI = PHIN->getIncomingValue(I);
      if (auto CI = dyn_cast<Constant>(VI)) {
        if (!CI->isNullValue())
          return false;
      } else if (auto LI = dyn_cast<LoadInst>(VI)) {
        if (LI->getPointerOperand() != GEPI)
          return false;
      } else {
        return false;
      }
    }
  } else if (auto LI = dyn_cast<LoadInst>(V)) {
    if (LI->getPointerOperand() != GEPI)
      return false;
  } else {
    return false;
  }
  return true;
}

void LocalArrayTransposePass::addCandidate(AllocaInst *AI) {
  Candidates.insert(AI);
}

bool LocalArrayTransposePass::PassesDominanceCheck(LoadInst *LI, CallBase *CB0,
                                                   CallBase *CB1,
                                                   DominatorTree &DT,
                                                   PostDominatorTree &PDT) {
  if (!DT.dominates(CB0->getParent(), LI->getParent()))
    return false;
  if (!PDT.dominates(CB1->getParent(), LI->getParent()))
    return false;
  return true;
}

bool LocalArrayTransposePass::isValidCandidate(
    AllocaInst *AI, DominatorTree &DT, PostDominatorTree &PDT,
    std::function<const TargetLibraryInfo &(Function &)> GetTLI) {
  const TargetLibraryInfo &TLI = GetTLI(*AI->getFunction());
  Type *AIType = AI->getAllocatedType();
  const DataLayout &DL = AI->getFunction()->getParent()->getDataLayout();
  if (!isDopeVectorType(AIType, DL))
    return false;
  for (User *U0 : AI->users()) {
    auto GEPI = dyn_cast<GetElementPtrInst>(U0);
    if (!GEPI || GEPI->getPointerOperand() != AI)
      return false;
    if (GEPI->hasAllZeroIndices()) {
      CallBase *CB0 = findAllocHandle(GEPI, TLI);
      if (!CB0)
        return false;
      CallBase *CB1 = findDeallocHandle(GEPI, TLI);
      if (!CB1)
        return false;
      for (User *U1 : GEPI->users()) {
        if (auto LI = dyn_cast<LoadInst>(U1)) {
          if (!PassesDominanceCheck(LI, CB0, CB1, DT, PDT))
            return false;
          for (User *U2 : LI->users()) {
            if (auto SBI = dyn_cast<SubscriptInst>(U2)) {
              if (!isValidSubscriptInst(SBI, LI))
                return false;
            } else if (auto PHIN = dyn_cast<PHINode>(U2)) {
              if (!isValidPHINode(PHIN, CB1))
                return false;
            } else if (U2 != CB1) {
              return false;
            }
          }
        } else if (auto SI = dyn_cast<StoreInst>(U1)) {
          if (!isValidStoreInst(SI, GEPI, TLI))
            return false;
        } else if (auto CB = dyn_cast<CallBase>(U1)) {
          if (CB != CB0)
            return false;
        } else {
          return false;
        }
      }
    }
  }
  return isSquareAllocatableArray(AI, GetTLI);
}

unsigned LocalArrayTransposePass::findValidCandidates(
    Function &F, DominatorTree &DT, PostDominatorTree &PDT,
    std::function<const TargetLibraryInfo &(Function &)> GetTLI) {
  unsigned Count = 0;
  Candidates.clear();
  for (auto &I : F.getEntryBlock())
    if (auto AI = dyn_cast<AllocaInst>(&I))
      if (isValidCandidate(AI, DT, PDT, GetTLI)) {
        addCandidate(AI);
        Count++;
      }
  return Count;
}

void LocalArrayTransposePass::printCandidates(Function &F, StringRef Banner) {
  if (Candidates.empty()) {
    LLVM_DEBUG(dbgs() << "LocalArrayTranspose: No " << Banner << " for "
                      << F.getName() << "\n");
    return;
  }
  LLVM_DEBUG({
    dbgs() << "LocalArrayTranspose: BEGIN " << Banner << " for " << F.getName()
           << "\n";
    for (auto AI : Candidates)
      AI->dump();
    dbgs() << "LocalArrayTranspose: END " << Banner << " for " << F.getName()
           << "\n";
  });
}

bool LocalArrayTransposePass::isProfitableCandidate(AllocaInst *AI,
                                                    LoopInfo &LI) {

  // Return the loop depth of 'SBI'.
  auto GetLoopDepth = [](SubscriptInst *SBI, LoopInfo &LI) -> unsigned {
    unsigned LD = 0;
    if (auto SI = dyn_cast<Instruction>(SBI->getIndex()))
      LD = LI.getLoopDepth(SI->getParent());
    return LD;
  };

  // Return 'true' if transposing 'S0' and 'S1' should yield better
  // performance.
  auto Is2DTransposeCandidate = [&GetLoopDepth](SubscriptInst *S0,
                                                SubscriptInst *S1,
                                                LoopInfo &LI) -> bool {
    unsigned LD0 = GetLoopDepth(S0, LI);
    unsigned LD1 = GetLoopDepth(S1, LI);
    return LD0 && LD1 && (LD1 < LD0);
  };

  unsigned MLD = 0;
  bool IsCandidate = false;
  for (User *U0 : AI->users())
    if (auto GEPI = dyn_cast<GetElementPtrInst>(U0))
      if (GEPI->getPointerOperand() == AI && GEPI->hasAllZeroIndices())
        for (User *U1 : GEPI->users())
          if (auto LDI = dyn_cast<LoadInst>(U1))
            for (User *U2 : LDI->users())
              if (auto S0 = dyn_cast<SubscriptInst>(U2)) {
                if (!S0->hasOneUse())
                  return false;
                User *U3 = S0->user_back();
                if (auto S1 = dyn_cast<SubscriptInst>(U3)) {
                  unsigned LD = LI.getLoopDepth(S1->getParent());
                  if (LD > MLD) {
                    MLD = LD;
                    IsCandidate = true;
                  }
                  if ((LD == MLD) && !Is2DTransposeCandidate(S0, S1, LI)) {
                    IsCandidate = false;
                    break;
                  }
                }
              }
  return IsCandidate;
}

unsigned LocalArrayTransposePass::findProfitableCandidates(LoopInfo &LI) {
  Candidates.remove_if(
      [&](AllocaInst *AI) { return !isProfitableCandidate(AI, LI); });
  return Candidates.size();
}

void LocalArrayTransposePass::transformCandidate(AllocaInst *AI, LoopInfo &LI) {
  for (User *U0 : AI->users())
    if (auto GEPI = dyn_cast<GetElementPtrInst>(U0))
      if (GEPI->getPointerOperand() == AI && GEPI->hasAllZeroIndices())
        for (User *U1 : GEPI->users())
          if (auto LDI = dyn_cast<LoadInst>(U1))
            for (User *U2 : LDI->users())
              if (auto S0 = dyn_cast<SubscriptInst>(U2)) {
                assert(S0->hasOneUse() && "Expecting single use");
                User *U3 = S0->user_back();
                if (auto S1 = dyn_cast<SubscriptInst>(U3)) {
                  S0->removeFromParent();
                  S0->insertBefore(S1);
                  Value *V0 = S0->getIndex();
                  Value *V1 = S1->getIndex();
                  S1->setArgOperand(IndexOpNum, V0);
                  S0->setArgOperand(IndexOpNum, V1);
                  if (auto S0I = dyn_cast<Instruction>(S0->getIndex()))
                    if (LI.getLoopDepth(S0I->getParent()) <
                        LI.getLoopDepth(S0->getParent())) {
                      S0->removeFromParent();
                      S0->insertBefore(S0I->getParent()->getTerminator());
                    }
                }
              }
}

void LocalArrayTransposePass::transformCandidates(LoopInfo &LI) {
  for (auto AI : Candidates)
    transformCandidate(AI, LI);
}

bool LocalArrayTransposePass::runImpl(
    Function &F, DominatorTree &DT, PostDominatorTree &PDT, LoopInfo &LI,
    std::function<const TargetLibraryInfo &(Function &)> GetTLI) {
  unsigned Count = 0;
  Count = findValidCandidates(F, DT, PDT, GetTLI);
  printCandidates(F, "Valid Candidates");
  if (Count == 0)
    return false;
  Count = findProfitableCandidates(LI);
  printCandidates(F, "Profitable Candidates");
  if (Count == 0)
    return false;
  transformCandidates(LI);
  return true;
}

PreservedAnalyses LocalArrayTransposePass::run(Function &F,
                                               FunctionAnalysisManager &AM) {
  if (F.isDeclaration() || !F.isFortran())
    PreservedAnalyses::all();
  auto &DT = AM.getResult<DominatorTreeAnalysis>(F);
  auto &PDT = AM.getResult<PostDominatorTreeAnalysis>(F);
  auto &LI = AM.getResult<LoopAnalysis>(F);
  auto GetTLI = [&AM](const Function &F) -> TargetLibraryInfo & {
    return AM.getResult<TargetLibraryAnalysis>(*(const_cast<Function *>(&F)));
  };

  runImpl(F, DT, PDT, LI, GetTLI);
  return PreservedAnalyses::all();
}
