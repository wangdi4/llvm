#if INTEL_FEATURE_SW_ADVANCED
//===- Intel_FunctionRecognizer.cpp - Function Recognizer -------------===//
//
// Copyright (C) 2020-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===-------------------------------------------------------------------===//
//
// This pass recognizes Functions and marks them with appropriate Function
// attributes.
//
//===-------------------------------------------------------------------===//

#include "llvm/Transforms/Scalar/Intel_FunctionRecognizer.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/IR/Value.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/Scalar.h"
#include <stdint.h>

using namespace llvm;
using namespace PatternMatch;

#define DEBUG_TYPE "functionrecognizer"

#define FXNREC_VERBOSE "functionrecognizer-verbose"

STATISTIC(NumFunctionsRecognized, "Number of Functions Recognized");

//
// NOTE: The examples below use typed pointers. I will change them to opaque
// pointers when -opaque-pointers is the default for xmain.
//

//
// Utility functions used by multiple function recognizers
//

using DenseMapBBToI = DenseMap<BasicBlock *, int64_t>;
using DenseMapBBToV = DenseMap<BasicBlock *, Value *>;
using SmValVec = SmallVector<Value *, 3>;
using SmValVecImpl = SmallVectorImpl<Value *>;

//
// Search 'BBI' and if it has exactly two StoreInsts, and no other
// Instruction that writes to memory, return 'true'. If we return 'true',
// we set the values of 'ST1' and 'ST2' to the two StoreInsts found.
//
static bool getTwoStores(BasicBlock *BBI, StoreInst **SI1, StoreInst **SI2) {
  *SI1 = nullptr;
  *SI2 = nullptr;
  for (auto &I : *BBI) {
    if (auto SI = dyn_cast<StoreInst>(&I)) {
      if (!*SI1)
        *SI1 = SI;
      else if (!*SI2)
        *SI2 = SI;
      else
        return false;
    } else if (I.mayWriteToMemory()) {
      return false;
    }
  }
  return SI1 && SI2;
}

//
// Return 'true' if 'BBI' matches a BasicBlock of the form:
//
//    br label 'BBO'
//
// If we return 'true', set the value of 'BBO'.
//
static bool isDirectBranchBlock(BasicBlock *BBI, BasicBlock **BBO) {
  auto BI = dyn_cast<BranchInst>(BBI->getTerminator());
  if (!BI || BI->isConditional())
    return false;
  *BBO = BI->getSuccessor(0);
  return true;
}

//
// If 'BBI' is terminated with a conditional test with the indicated 'Pred',
// return 'true', and set 'BIO' and 'ICO' to the BranchInst and the ICmpInst
// terminating 'BBI'.
//
static bool getBIAndIC(BasicBlock *BBI, ICmpInst::Predicate Pred,
                       BranchInst **BIO, ICmpInst **ICO) {
  auto BI = dyn_cast<BranchInst>(BBI->getTerminator());
  if (!BI || BI->isUnconditional())
    return false;
  auto IC = dyn_cast<ICmpInst>(BI->getCondition());
  if (!IC || IC->getPredicate() != Pred)
    return false;
  *BIO = BI;
  *ICO = IC;
  return true;
}

//
// Function recognizers
//

//
// Return 'true' if 'F' is a qsort compare.
//
static bool isQsortCompare(Function &F) {

  // Used to represent an invalid GEP index. It is passed to the functions
  // below to indicate that we don't yet know what GEP index should be used.
  // In this case, the function's formal parameter is set to the index value
  // found. Subsequent calls to the function pass the value that was found,
  // and a check is made that the seen value matches.
  //
  // For example,
  //   uint64_t Val = UnknownIndex;
  //   IsCVal(V, Val);
  // now Val has a valid known value on the next call
  //   IsCVal(V, Val);
  // we test if V has the value Val.
  //
  const uint64_t UnknownIndex = UINT64_MAX;

  //
  // Return 'true' if the Types of 'F' match the prototype of a qsort compare.
  //
  auto MatchesPrototype = [](Function &F) -> bool {
    if (F.isDeclaration() || F.isVarArg() || F.arg_size() != 2)
      return false;
    if (!F.getArg(0)->getType()->isPointerTy())
      return false;
    if (!F.getArg(1)->getType()->isPointerTy())
      return false;
    if (!F.getReturnType()->isIntegerTy(32))
      return false;
    return true;
  };

  //
  // If 'Val' == UnknownIndex, return 'true' if 'V' is a ConstantInt, and set
  // 'Val' to its value. If 'Val ' != UnknownIndex, return 'true' if 'V' is a
  // ConstantInt and has the value 'Val'.
  //
  const auto& tmp = UnknownIndex;
  auto IsCIVal = [tmp](Value *V, uint64_t &Val) -> bool {
    auto CI = dyn_cast<ConstantInt>(V);
    if (!CI)
      return false;
    if (Val == tmp) {
      Val = CI->getZExtValue();
      return true;
    }
    return CI->getZExtValue() == Val;
  };

  //
  // Match a sequence of the form:
  //
  // %0 = getelementptr inbounds %struct.S, %struct.S* 'RV', i32 0, i32 'Val'
  // %1 = load %struct.S*, %struct.S** 'V', align 8
  //
  // If 'Val' == UnknownIndex, and the sequence is matched, set 'Val' to the
  // value it has in the sequence and return 'RV'. If 'Val' != UnknownIndex,
  // the sequence is matched, and 'Val' has the indicated value, return 'RV'.
  //
  auto IsLoadGEPAtIndex = [&IsCIVal](Value *V, uint64_t &Val) -> Value * {
    auto LI = dyn_cast<LoadInst>(V);
    if (!LI)
      return nullptr;
    auto GEPI = dyn_cast<GetElementPtrInst>(LI->getPointerOperand());
    if (!GEPI || GEPI->getNumOperands() != 3)
      return nullptr;
    uint64_t TestVal = 0;
    if (!IsCIVal(GEPI->getOperand(1), TestVal))
      return nullptr;
    if (!IsCIVal(GEPI->getOperand(2), Val))
      return nullptr;
    return GEPI->getPointerOperand();
  };

  //
  // If 'IsShort' is 'true', match a sequence of the form:
  //   %0 = load %struct.S*, %struct.S** 'Arg', align 8
  //   %AC = getelementptr inbounds %struct.S, %struct.S* %0, i32 0, i32 'LV'
  //   'V' = load i64, i64* %AC, align 8
  // If 'IsShort' is 'false', match a sequence of the form:
  //   %8 = load %struct.S0*, %struct.S0** 'Arg', align 8
  //   %a = getelementptr inbounds %struct.S0, %struct.S0* %8, i32 0, i32 'LV'
  //   %9 = load %struct.S1*, %struct.S1** %a, align 8
  //   %id = getelementptr inbounds %struct.S1, %struct.S1* %9, i32 0, i32 0
  //   'V' = load i32, i32* %id, align 8
  // If 'LV' == UnknownIndex, and the sequence is matched, set 'LV' to the
  // discovered value and return 'true'. If 'LV' != UnknownIndex and the
  // sequence is matched with the given value of 'LV', return 'true'.
  //
  auto ValidateOp = [&IsLoadGEPAtIndex](Value *V, Argument *Arg, bool IsShort,
                                        uint64_t &LV) -> bool {
    if (!IsShort) {
      uint64_t TestVal = 0;
      V = IsLoadGEPAtIndex(V, TestVal);
      if (!V)
        return false;
    }
    V = IsLoadGEPAtIndex(V, LV);
    if (!V)
      return false;
    auto LI = dyn_cast<LoadInst>(V);
    if (!LI || LI->getPointerOperand() != Arg)
      return false;
    return true;
  };

  //
  // Match a BasicBlock 'BB0' which is sequence of the form:
  //   %cmp = icmp 'Pred' i64 'S0', 'S1'
  //   br i1 %cmp, label 'BBL', label 'BBR'
  // where 'S0' and 'S1' are sequences matched by ValidateOp() above.
  // 'IsShort' indicates whether a short or long sequence is expected, with
  // 'Arg0' terminating 'S0' and 'Arg1' terminating 'S1'. 'LV' is the
  // discovered or tested value of final GEP index in the sequence.
  // If 'Pred' == BAD_ICMP_PREDICATE, set 'Pred' to the predicate discovered,
  // set 'BBL' and 'BBR' and return 'true'. If 'Pred' != BAD_ICMP_PREDICATE,
  // the sequence is matched with the given value of 'Pred', set 'BBL' and
  // 'BBR' and return 'true'.
  //
  auto ValidateBBTest =
      [&ValidateOp](BasicBlock *BB0, Argument *Arg0, Argument *Arg1,
                    ICmpInst::Predicate &Pred, bool IsShort, uint64_t &LV,
                    BasicBlock **BBL, BasicBlock **BBR) -> bool {
    auto BI = dyn_cast<BranchInst>(BB0->getTerminator());
    if (!BI || BI->isUnconditional())
      return false;
    *BBL = BI->getSuccessor(0);
    *BBR = BI->getSuccessor(1);
    ICmpInst *IC = dyn_cast<ICmpInst>(BI->getCondition());
    if (!IC)
      return false;
    if (Pred == ICmpInst::BAD_ICMP_PREDICATE) {
      ICmpInst::Predicate ICP = IC->getPredicate();
      if (ICP != ICmpInst::ICMP_SLT && ICP != ICmpInst::ICMP_SGT)
        return false;
      Pred = IC->getSwappedPredicate();
    } else if (Pred != IC->getPredicate())
      return false;
    return ValidateOp(IC->getOperand(0), Arg0, IsShort, LV) &&
           ValidateOp(IC->getOperand(1), Arg1, IsShort, LV);
  };

  //
  // Match a BasicBlock 'BBI' which is a sequence of the form:
  //   br label 'BBRetTest'
  // where 'BBRetTest' is a BasicBlock which begins with a PHINode whose
  // incoming value from 'BBI' is 'PHIV'. If 'BBRetTest' == nullptr and the
  // sequence is matched, return the value of 'BBRetTest'. If 'BBRetTest' !=
  // nullptr and the sequence is matched with the given value of 'BBRetTest',
  // return 'BBRetTest'.
  // If 'BBRetTest' is returned, add a pair ('BBI', 'PHIV') to the map
  // 'BBRetMap', so we can check later if the correct incoming values and
  // BasicBlocks appear in the PHINode at the beginning of 'BBRetTest'.
  //
  auto ValidateBBGoToReturn = [](BasicBlock *BBI, int64_t PHIV,
                                 BasicBlock *BBRetTest,
                                 DenseMapBBToI &BBRetMap) -> BasicBlock * {
    auto BI = dyn_cast<BranchInst>(BBI->getTerminator());
    if (!BI || BI->isConditional())
      return nullptr;
    BasicBlock *BBO = BI->getSuccessor(0);
    if (BBRetTest && (BBO != BBRetTest))
      return nullptr;
    BBRetMap[BBI] = PHIV;
    return BBO;
  };

  //
  // Return 'true' if 'BBRet' has the desired form:
  //   %rv = phi i32 [ 'V0', 'BBR0' ], [ 'V1', 'BBR1' ], [ 'V2', 'BBR2' ],
  //     [ 'V3', 'BBR3' ]
  //   ret i32 %rv
  // where the correspondence between the incoming values and the BasicBlocks
  // for the PHINode is given by the map 'BBRetMap'.
  //
  auto ValidateReturn = [](BasicBlock *BBRet, DenseMapBBToI &BBRetMap) -> bool {
    auto RI = dyn_cast<ReturnInst>(BBRet->getTerminator());
    if (!RI)
      return false;
    auto PHIN = dyn_cast_or_null<PHINode>(RI->getReturnValue());
    if (!PHIN)
      return false;
    for (unsigned I = 0, E = PHIN->getNumIncomingValues(); I < E; ++I) {
      Value *VI = PHIN->getIncomingValue(I);
      BasicBlock *BBI = PHIN->getIncomingBlock(I);
      auto CI = dyn_cast<ConstantInt>(VI);
      if (!CI || BBRetMap[BBI] != CI->getSExtValue())
        return false;
    }
    return true;
  };

  BasicBlock *BB0 = &F.getEntryBlock();
  BasicBlock *BBL0 = nullptr;
  BasicBlock *BBR0 = nullptr;
  BasicBlock *BBL1 = nullptr;
  BasicBlock *BBR1 = nullptr;
  BasicBlock *BBL2 = nullptr;
  BasicBlock *BBR2 = nullptr;
  BasicBlock *BBRet = nullptr;
  DenseMapBBToI BBRetMap;

  // This is the main code for isQsortCompare().
  if (!MatchesPrototype(F))
    return false;
  Argument *A0 = F.getArg(0);
  Argument *A1 = F.getArg(1);
  ICmpInst::Predicate Pred = ICmpInst::BAD_ICMP_PREDICATE;
  uint64_t Index = UnknownIndex;
  uint64_t ZeroIndex = 0;
  if (!ValidateBBTest(BB0, A0, A1, Pred, true, Index, &BBL0, &BBR0))
    return false;
  BBRet = ValidateBBGoToReturn(BBL0, 1, nullptr, BBRetMap);
  if (!BBRet)
    return false;
  if (!ValidateBBTest(BBR0, A0, A1, Pred, true, Index, &BBL1, &BBR1))
    return false;
  if (!ValidateBBGoToReturn(BBL1, -1, BBRet, BBRetMap))
    return false;
  if (!ValidateBBTest(BBR1, A0, A1, Pred, false, ZeroIndex, &BBL2, &BBR2) &&
      !ValidateBBTest(BBR1, A0, A1, Pred, true, ZeroIndex, &BBL2, &BBR2))
    return false;
  int64_t LeftIndex = Pred == ICmpInst::ICMP_SLT ? -1 : 1;
  if (!ValidateBBGoToReturn(BBL2, LeftIndex, BBRet, BBRetMap))
    return false;
  int64_t RightIndex = Pred == ICmpInst::ICMP_SLT ? 1 : -1;
  if (!ValidateBBGoToReturn(BBR2, RightIndex, BBRet, BBRetMap))
    return false;
  if (!ValidateReturn(BBRet, BBRetMap))
    return false;
  return true;
}

//
// Return 'true' if 'F' is a qsort med3.
//
static bool isQsortMed3(Function &F) {

  //
  // Return 'true' if we match a sequence of the form:
  //
  //  'VC' = call i32 'AP'(i8* 'AL', i8* 'AR')
  //
  auto IsIndCmpCall = [](Value *VC, Argument *AP, Argument *AL,
                         Argument *AR) -> bool {
    auto CI = dyn_cast<CallInst>(VC);
    if (!CI || !CI->isIndirectCall() || CI->arg_size() != 2)
      return false;
    if (CI->getCalledOperand() != AP || CI->getArgOperand(0) != AL ||
        CI->getArgOperand(1) != AR)
      return false;
    return true;
  };

  //
  // Return 'true' if we match a sequence of the form:
  //
  //   %call = call i32 'AP'(i8* 'AL', i8* 'AR')
  //   'V0' = icmp 'Pred' i32 %call, 0
  //
  auto IsICmpIndCmp = [&IsIndCmpCall](Value *V0, Argument *AP, Argument *AL,
                                      Argument *AR,
                                      ICmpInst::Predicate Pred) -> bool {
    auto IC = dyn_cast<ICmpInst>(V0);
    if (!IC || IC->getPredicate() != Pred)
      return false;
    auto CI = dyn_cast<ConstantInt>(IC->getOperand(1));
    if (!CI || !CI->isZero())
      return false;
    return IsIndCmpCall(IC->getOperand(0), AP, AL, AR);
  };

  //
  // Return 'true' if we match a BasicBlock 'BBI' of the form:
  //
  //   %call = call i32 'AP'(i8* 'AL', i8* 'AR')
  //   %cmp = icmp 'Pred' i32 %call, 0
  //   br i1 %cmp, label 'BBL', label 'BBR'
  //
  // If we return 'true', set the values of 'BBL' and 'BBR'.
  //
  auto IsCmpBlock = [&IsICmpIndCmp](BasicBlock *BBI, Argument *AP, Argument *AL,
                                    Argument *AR, ICmpInst::Predicate Pred,
                                    BasicBlock **BBL,
                                    BasicBlock **BBR) -> bool {
    auto BI = dyn_cast<BranchInst>(BBI->getTerminator());
    if (!BI || BI->isUnconditional())
      return false;
    if (!IsICmpIndCmp(BI->getCondition(), AP, AL, AR, Pred))
      return false;
    *BBL = BI->getSuccessor(0);
    *BBR = BI->getSuccessor(1);
    return true;
  };

  //
  // Return 'true' if we match a BasicBlock 'BBI' of the form:
  //
  //   br label 'BBO'
  //
  // and indicate that 'BBO', which starts with a PHINode, will have an
  // incoming [Value *, BasicBlock *] pair of [ 'V0', 'BBI' ]. If we return
  // 'true', put an entry in the PHIMap to indicate this and set the value of
  // 'BBO'.
  //
  auto IsDirectBranchBlockWithValue = [](BasicBlock *BBI, Value *V0,
                                         DenseMapBBToV &PHIMap,
                                         BasicBlock **BBO) -> bool {
    auto BI = dyn_cast<BranchInst>(BBI->getTerminator());
    if (!BI || BI->isConditional())
      return false;
    PHIMap[BBI] = V0;
    *BBO = BI->getSuccessor(0);
    return true;
  };

  //
  // Return 'true' if we match a BasicBlock 'BBI' of the form:
  //
  //   %call = call i32 'CallP'(i8* 'CallL', i8* 'CallR')
  //   %cmp = icmp 'Pred' i32 %call, 0
  //   'SI' = select i1 %cmp, i8* 'SelL', i8* 'SelR'
  //   br label 'BBO'
  //
  // and indicate that 'BBO', which startes with a PHINode, will have an
  // incoming [Value *, BasicBlock *] pair of ['SI', 'BBI']. If we return
  // 'true', put an entry in the PHIMap to indicate this and set the value of
  // 'BBO'.
  //
  auto IsCmpSelBlock = [&IsICmpIndCmp](BasicBlock *BBI, Argument *SelL,
                                       Argument *SelR, ICmpInst::Predicate Pred,
                                       Argument *CallL, Argument *CallR,
                                       Argument *CallP, DenseMapBBToV &PHIMap,
                                       BasicBlock **BBO) -> bool {
    auto BI = dyn_cast<BranchInst>(BBI->getTerminator());
    if (!BI || BI->isConditional())
      return false;
    auto SI = dyn_cast_or_null<SelectInst>(BI->getPrevNonDebugInstruction());
    if (!SI || SI->getTrueValue() != SelL || SI->getFalseValue() != SelR)
      return false;
    if (!IsICmpIndCmp(SI->getCondition(), CallP, CallL, CallR, Pred))
      return false;
    *BBO = BI->getSuccessor(0);
    PHIMap[BBI] = SI;
    return true;
  };

  //
  // Return 'true' if 'BBPHI' has the desired form:
  //   %rv = phi i32 [ 'V0', 'BBR0' ], [ 'V1', 'BBR1' ], [ 'V2', 'BBR2' ],
  //     [ 'V3', 'BBR3' ]
  //   ret i32 %rv
  // where the correspondence between the incoming values and the BasicBlocks
  // for the PHINode is given by the map 'PHIMap'.
  //
  auto IsOKPHIBlock = [](BasicBlock *BBPHI, DenseMapBBToV &PHIMap) -> bool {
    auto RI = dyn_cast<ReturnInst>(BBPHI->getTerminator());
    if (!RI)
      return false;
    auto PHIN = dyn_cast_or_null<PHINode>(RI->getReturnValue());
    if (!PHIN)
      return false;
    for (unsigned I = 0, E = PHIN->getNumIncomingValues(); I < E; ++I) {
      Value *VI = PHIN->getIncomingValue(I);
      BasicBlock *BBI = PHIN->getIncomingBlock(I);
      if (PHIMap[BBI] != VI)
        return false;
    }
    return true;
  };

  //
  // If every use of 'F(3)' is the called operand of an indirect call,
  // mark those indirect calls with the 'must-be-qsort-compare' attribute.
  //
  auto MarkIndirectCalls = [](Function &F) -> bool {
    Argument *CU = F.getArg(3);
    for (User *U : CU->users()) {
      auto CB = dyn_cast<CallBase>(U);
      if (!CB || CB->getCalledOperand() != CU)
        return false;
    }
    for (User *U : CU->users())
      cast<CallBase>(U)->addFnAttr("must-be-qsort-compare");
    return true;
  };

  DenseMapBBToV PHIMap;

  // This is the main code for isQsortMed3().
  if (F.isDeclaration() || F.isVarArg() || F.arg_size() != 4)
    return false;
  if (!F.getReturnType()->isPointerTy())
    return false;
  for (unsigned I = 0; I < F.arg_size(); ++I)
    if (!F.getArg(I)->getType()->isPointerTy())
      return false;
  BasicBlock *BBE = &F.getEntryBlock();
  BasicBlock *BBT0 = nullptr;
  BasicBlock *BBF0 = nullptr;
  BasicBlock *BBT1 = nullptr;
  BasicBlock *BBF1 = nullptr;
  BasicBlock *BBT2 = nullptr;
  BasicBlock *BBF2 = nullptr;
  BasicBlock *BBS0 = nullptr;
  BasicBlock *BBS1 = nullptr;
  BasicBlock *BBU0 = nullptr;
  BasicBlock *BBPHI = nullptr;
  Argument *A0 = F.getArg(0);
  Argument *A1 = F.getArg(1);
  Argument *A2 = F.getArg(2);
  Argument *A3 = F.getArg(3);
  ICmpInst::Predicate PLT = ICmpInst::ICMP_SLT;
  ICmpInst::Predicate PGT = ICmpInst::ICMP_SGT;
  if (!IsCmpBlock(BBE, A3, A0, A1, PLT, &BBT0, &BBF0))
    return false;
  if (!IsCmpBlock(BBT0, A3, A1, A2, PLT, &BBT1, &BBF1))
    return false;
  if (!IsDirectBranchBlockWithValue(BBT1, A1, PHIMap, &BBPHI))
    return false;
  if (!IsCmpSelBlock(BBF1, A2, A0, PLT, A0, A2, A3, PHIMap, &BBS0) ||
      BBS0 != BBPHI)
    return false;
  if (!IsCmpBlock(BBF0, A3, A1, A2, PGT, &BBT2, &BBF2))
    return false;
  if (!IsDirectBranchBlockWithValue(BBT2, A1, PHIMap, &BBU0) || BBU0 != BBPHI)
    return false;
  if (!IsCmpSelBlock(BBF2, A0, A2, PLT, A0, A2, A3, PHIMap, &BBS1) ||
      BBS1 != BBPHI)
    return false;
  if (!IsOKPHIBlock(BBPHI, PHIMap))
    return false;
  if (!MarkIndirectCalls(F))
    return false;
  return true;
}

//
// Return 'true' if 'F' is a qsort swapfunc.
//
static bool isQsortSwapFunc(Function &F) {

  //
  // Return 'true' if the Types of 'F' match the prototype of a qsort swapfunc.
  //
  auto MatchesPrototype = [](Function &F) -> bool {
    if (F.isDeclaration() || F.isVarArg() || F.arg_size() != 5)
      return false;
    for (unsigned I = 0, E = F.arg_size(); I < E; ++I) {
      Type *TyArg = F.getArg(I)->getType();
      if (I < 2) {
        if (!TyArg->isPointerTy())
          return false;
      } else if (!TyArg->isIntegerTy(32)) {
        return false;
      }
    }
    if (!F.getReturnType()->isVoidTy())
      return false;
    return true;
  };

  //
  // Return 'true' if 'BBI' matches a sequence of the form:
  //
  //     %cmp = icmp 'Pred' i32 'AI', 1
  //      br i1 %cmp, label 'BBOL', label 'BBOR'
  //
  // If we return 'true', we set the values of 'BBOL' and 'BBOR'.
  //
  auto IsArgCmpBlock = [](BasicBlock *BBI, Argument *AI,
                          ICmpInst::Predicate Pred, BasicBlock **BBOL,
                          BasicBlock **BBOR) -> bool {
    BranchInst *BI = nullptr;
    ICmpInst *IC = nullptr;
    if (!getBIAndIC(BBI, Pred, &BI, &IC))
      return false;
    if (IC->getOperand(0) != AI)
      return false;
    auto CI = dyn_cast<ConstantInt>(IC->getOperand(1));
    if (!CI || !CI->isOne())
      return false;
    *BBOL = BI->getSuccessor(0);
    *BBOR = BI->getSuccessor(1);
    return true;
  };

  //
  // Return 'true' if 'BBI' in 'F' matches a sequence of the form:
  //
  //   %conv = sext i32 'F(2)' to i64
  //   'V2' = udiv i64 %conv, 'UDivDen'
  //   'V0' = bitcast i8* 'F(0)' to i64*
  //   'V1' = bitcast i8* 'F(1)' to i64*
  //   br label 'BBO'
  //
  // If we return 'true', we set the values of 'V0', 'V1', 'V2', and 'BBO'.
  // Note: the bitcast instructions may be absent, in which case we set
  // 'V0' and 'V1' to nullptr.
  //
  // NOTE: Bitcasts may not be present when -opaque-pointers becomes
  // the default.
  //
  auto IsSFDoPreH = [](Function &F, BasicBlock *BBI, uint64_t UDivDen,
                       Value **V0, Value **V1, Value **V2,
                       BasicBlock **BBO) -> bool {
    *V0 = nullptr;
    *V1 = nullptr;
    *V2 = nullptr;
    auto BI = dyn_cast<BranchInst>(BBI->getTerminator());
    if (!BI || BI->isConditional())
      return false;
    *BBO = BI->getSuccessor(0);
    Instruction *BP = BI->getPrevNonDebugInstruction();
    if (!BP)
      return false;
    if (auto BC1 = dyn_cast<BitCastInst>(BP)) {
      if (BC1->getOperand(0) != F.getArg(1))
        return false;
      *V1 = BC1;
      BP = BP->getPrevNonDebugInstruction();
      if (!BP)
        return false;
    }
    if (auto BC0 = dyn_cast<BitCastInst>(BP)) {
      if (BC0->getOperand(0) != F.getArg(0))
        return false;
      *V0 = BC0;
      BP = BP->getPrevNonDebugInstruction();
      if (!BP)
        return false;
    }
    Value *V2P = BP;
    if (auto TI = dyn_cast<TruncInst>(BP))
      BP = dyn_cast<Instruction>(TI->getOperand(0));
    auto DVO = dyn_cast<UDivOperator>(BP);
    if (!DVO)
      return false;
    auto CI = dyn_cast<ConstantInt>(DVO->getOperand(1));
    if (!CI || CI->getZExtValue() != UDivDen)
      return false;
    auto SXI = dyn_cast<SExtInst>(DVO->getOperand(0));
    if (!SXI || SXI->getOperand(0) != F.getArg(2))
      return false;
    *V2 = V2P;
    return true;
  };

  //
  // Return 'true' if 'BBI' in 'F' matches a sequence of the form:
  //
  //  'BBI':
  //    'VI' = bitcast i8* %a to 'DType'*
  //    br label 'BBD'
  //  'BBD':
  //    'PHIL' = phi 'DType'* [ 'VI', 'BBI' ], [ 'GEPI', 'BBD' ]
  //    'LI' = load 'DType', 'DType' 'PHIL', align 8
  //    'SI': store 'DType' 'LI', 'DType'* 'PHIS', align 8
  //    'GEPI' = getelementptr inbounds 'DType', 'DType'* 'PHIL', i32 1
  //    br i1 %cmp2, label 'BBD', label 'BBX'
  //
  // If we return 'true', we set the values of 'PHIS' and 'PHIL'.
  // Note: we don't match the branch labels here.  They are shown only
  // for context.
  //
  // NOTE: Bitcasts may not be present when -opaque-pointers becomes
  // the default.
  //
  auto IsLSChain = [](BasicBlock *BBI, BasicBlock *BBD, StoreInst *SI,
                      Value *VI, Type *DType, PHINode **PHIS,
                      PHINode **PHIL) -> bool {
    auto LI = dyn_cast<LoadInst>(SI->getValueOperand());
    if (!LI || LI->getType() != DType)
      return false;
    auto PHIN = dyn_cast<PHINode>(LI->getPointerOperand());
    if (!PHIN || PHIN->getNumIncomingValues() != 2)
      return false;
    if (PHIN->getIncomingValue(0) != VI)
      return false;
    if (PHIN->getIncomingBlock(0) != BBI)
      return false;
    if (PHIN->getIncomingBlock(1) != BBD)
      return false;
    auto GEPI = dyn_cast<GetElementPtrInst>(PHIN->getIncomingValue(1));
    if (!GEPI || GEPI->getNumOperands() != 2 ||
        GEPI->getPointerOperand() != PHIN || GEPI->getParent() != BBD)
      return false;
    auto CI = dyn_cast<ConstantInt>(GEPI->getOperand(1));
    if (!CI || !CI->isOne())
      return false;
    auto PHISPO = dyn_cast<PHINode>(SI->getPointerOperand());
    if (!PHISPO)
      return false;
    *PHIS = PHISPO;
    *PHIL = PHIN;
    return true;
  };

  //
  // Return 'true' if 'BBD' is a single BasicBlock do-loop with preheader
  // 'BBI' in 'F' that matches:
  //
  // 'BBI':
  //   %conv = sext i32 %n to i64
  //   'VN' = udiv i64 %conv, 8
  //   'V0' = bitcast i8* %a to i64*
  //   'V1' = bitcast i8* %b to i64*
  //   br label %do.body
  // 'BBD':
  //   %pj.0 = phi 'DType'* [ 'V1', 'BBI' ], [ %incdec.ptr1, 'BBD' ]
  //   %pi.0 = phi 'DType'* [ 'V0', 'BBI' ], [ %incdec.ptr, 'BBD' ]
  //   %i.0 = phi i64 [ %div, 'BBI' ], [ %dec, 'BBD' ]
  //   %2 = load 'DType', 'DType'* %pi.0, align 8, !tbaa !8
  //   %3 = load 'DType', 'DType'* %pj.0, align 8, !tbaa !8
  //   %incdec.ptr = getelementptr inbounds 'DType', 'DType'* %pi.0, i32 1
  //   store 'DType' %3, 'DType'* %pi.0, align 8, !tbaa !8
  //   %incdec.ptr1 = getelementptr inbounds i64, i64* %pj.0, i32 1
  //   store 'DType' %2, 'DType'* %pj.0, align 8, !tbaa !8
  //   %dec = add nsw i64 %i.0, -1
  //   %cmp2 = icmp sgt i64 %dec, 0
  //   br i1 %cmp2, label 'BBD', label 'BBX'
  // 'BBX':
  //
  // If we return 'true', set the value of 'BBX'.
  // Note: Here we only match the do-loop itself. We match the do-loop and
  // its preheader and exit branch below.
  //
  // NOTE: Bitcasts may not be present when -opaque-pointers becomes
  // the default.
  //
  auto IsSFDoBody = [&IsLSChain](Function &F, BasicBlock *BBD, BasicBlock *BBI,
                                 Type *DType, Value *V0, Value *V1, Value *VN,
                                 BasicBlock **BBX) -> bool {
    BranchInst *BI = nullptr;
    ICmpInst *IC = nullptr;
    if (!getBIAndIC(BBD, ICmpInst::ICMP_SGT, &BI, &IC))
      return false;
    if (BI->getSuccessor(0) != BBD)
      return false;
    auto CIZ = dyn_cast<ConstantInt>(IC->getOperand(1));
    if (!CIZ || !CIZ->isZero())
      return false;
    auto BO = dyn_cast<BinaryOperator>(IC->getOperand(0));
    if (!BO || BO->getOpcode() != Instruction::Add)
      return false;
    auto CIM1 = dyn_cast<ConstantInt>(BO->getOperand(1));
    if (!CIM1 || !CIM1->isMinusOne())
      return false;
    auto PHIIV = dyn_cast<PHINode>(BO->getOperand(0));
    if (!PHIIV || PHIIV->getNumIncomingValues() != 2)
      return false;
    if (PHIIV->getIncomingValue(0) != VN || PHIIV->getIncomingBlock(0) != BBI)
      return false;
    if (PHIIV->getIncomingValue(1) != BO || PHIIV->getIncomingBlock(1) != BBD)
      return false;
    StoreInst *SI0 = nullptr;
    StoreInst *SI1 = nullptr;
    if (!getTwoStores(BBD, &SI0, &SI1))
      return false;
    PHINode *PHIS0 = nullptr;
    PHINode *PHIS1 = nullptr;
    PHINode *PHIL0 = nullptr;
    PHINode *PHIL1 = nullptr;
    Value *VI1 = V1 ? V1 : F.getArg(1);
    if (!IsLSChain(BBI, BBD, SI0, VI1, DType, &PHIS0, &PHIL0))
      return false;
    Value *VI0 = V0 ? V0 : F.getArg(0);
    if (!IsLSChain(BBI, BBD, SI1, VI0, DType, &PHIS1, &PHIL1))
      return false;
    if (PHIS1 != PHIL0 || PHIS0 != PHIL1)
      return false;
    auto V0I = dyn_cast<Instruction>(VI0);
    if (!isa<Argument>(VI0) && (!V0I || V0I->getParent() != BBI))
      return false;
    auto V1I = dyn_cast<Instruction>(VI1);
    if (!isa<Argument>(VI1) && (!V1I || V1I->getParent() != BBI))
      return false;
    auto VNI = dyn_cast<Instruction>(VN);
    if (!VNI || VNI->getParent() != BBI)
      return false;
    *BBX = BI->getSuccessor(1);
    return true;
  };

  //
  // Return 'BBR' if 'BBI' is the preheader of a single BasicBlock do-loop
  // in 'F' that matches:
  //
  // 'BBI':
  //   %conv = sext i32 %n to i64
  //   'VN' = udiv i64 %conv, 'UDivDen'
  //   'V0' = bitcast i8* %a to i64*
  //   'V1' = bitcast i8* %b to i64*
  //   br label %do.body
  // 'BBD':
  //   %pj.0 = phi 'DType'* [ 'V1', 'BBI' ], [ %incdec.ptr1, 'BBD' ]
  //   %pi.0 = phi 'DType'* [ 'V0', 'BBI' ], [ %incdec.ptr, 'BBD' ]
  //   %i.0 = phi i64 [ %div, 'BBI' ], [ %dec, 'BBD' ]
  //   %2 = load 'DType', 'DType'* %pi.0, align 8, !tbaa !8
  //   %3 = load 'DType', 'DType'* %pj.0, align 8, !tbaa !8
  //   %incdec.ptr = getelementptr inbounds 'DType', 'DType'* %pi.0, i32 1
  //   store 'DType' %3, 'DType'* %pi.0, align 8, !tbaa !8
  //   %incdec.ptr1 = getelementptr inbounds i64, i64* %pj.0, i32 1
  //   store 'DType' %2, 'DType'* %pj.0, align 8, !tbaa !8
  //   %dec = add nsw i64 %i.0, -1
  //   %cmp2 = icmp sgt i64 %dec, 0
  //   br i1 %cmp2, label 'BBD', label 'BBX'
  // 'BBX':
  //   br label 'BBR'
  //
  // otherwise, return nullptr. Furthermore, if 'BBRet' is not nullptr,
  // 'BBRet' must match 'BBR', or we return nullptr.
  //
  // NOTE: Bitcasts may not be present when -opaque-pointers becomes
  // the default.
  //
  auto IsDoLoop = [&IsSFDoPreH,
                   &IsSFDoBody](Function &F, BasicBlock *BBI, BasicBlock *BBRet,
                                uint64_t UDivDen, Type *DType) -> BasicBlock * {
    Value *A0 = nullptr;
    Value *A1 = nullptr;
    Value *A2 = nullptr;
    BasicBlock *BBD = nullptr;
    BasicBlock *BBX = nullptr;
    BasicBlock *BBR = nullptr;
    if (!IsSFDoPreH(F, BBI, UDivDen, &A0, &A1, &A2, &BBD))
      return nullptr;
    if (!IsSFDoBody(F, BBD, BBI, DType, A0, A1, A2, &BBX))
      return nullptr;
    if (!isDirectBranchBlock(BBX, &BBR))
      return nullptr;
    if (BBRet && (BBR != BBRet))
      return nullptr;
    return BBR;
  };

  //
  // Return 'true' if 'BBRet' ends in a void return.
  //
  auto IsOKRet = [](BasicBlock *BBRet) -> bool {
    auto RI = dyn_cast<ReturnInst>(BBRet->getTerminator());
    return RI && !RI->getReturnValue();
  };

  BasicBlock *BBE = &F.getEntryBlock();
  BasicBlock *BBL0 = nullptr;
  BasicBlock *BBR0 = nullptr;
  BasicBlock *BBL1 = nullptr;
  BasicBlock *BBR1 = nullptr;
  BasicBlock *BBRet = nullptr;

  // This is the main code for isQsortSwapFunc().
  if (!MatchesPrototype(F))
    return false;
  if (!IsArgCmpBlock(BBE, F.getArg(3), ICmpInst::ICMP_SLE, &BBL0, &BBR0))
    return false;
  LLVMContext &FC = F.getContext();
  Type *TI32 = llvm::Type::getInt32Ty(FC);
#if _WIN32
  const uint64_t BitsInLong = 4;
  Type *TILong = TI32;
#else
  const uint64_t BitsInLong = 8;
  Type *TILong = llvm::Type::getInt64Ty(FC);
#endif // _WIN32
  BBRet = IsDoLoop(F, BBL0, nullptr, BitsInLong, TILong);
  if (!BBRet)
    return false;
  if (!IsArgCmpBlock(BBR0, F.getArg(4), ICmpInst::ICMP_SLE, &BBL1, &BBR1))
    return false;
  if (!IsDoLoop(F, BBL1, BBRet, 4, TI32))
    return false;
  if (!IsDoLoop(F, BBR1, BBRet, 1, llvm::Type::getInt8Ty(FC)))
    return false;
  if (!IsOKRet(BBRet))
    return false;
  return true;
}

//
// Return 'true' if 'F' is a qsort spec_qsort.
//
static bool isQsortSpecQsort(Function &F, Function **FSwapFunc,
                             Function **FMed3) {

  //
  // Return 'true' if the Types of 'F' match the prototype of a qsort swapfunc.
  //
  auto MatchesPrototype = [](Function &F) -> bool {
    if (F.isDeclaration() || F.isVarArg() || F.arg_size() != 4)
      return false;
    if (!F.getArg(0)->getType()->isPointerTy())
      return false;
    if (!F.getArg(1)->getType()->isIntegerTy(64))
      return false;
    if (!F.getArg(2)->getType()->isIntegerTy(64))
      return false;
    if (!F.getArg(3)->getType()->isPointerTy())
      return false;
    if (!F.getReturnType()->isVoidTy())
      return false;
    return true;
  };

  //
  // Return 'true' if 'BBL' in 'F' defines the key PHINodes 'PHIA' and
  // 'PHIN'
  //
  // 'PHIN'= phi i64 [ 'F(1)', %entry ], [ %div295, %if.then292 ]
  // 'PHIA' = phi i8* [ 'F(0)', %entry ], [ %add.ptr294, %if.then292 ]
  //
  // If we return 'true', set the values of 'PHIA' and 'PHIN'.
  //
  auto GetKeyLoopPHIs = [](Function &F, BasicBlock *BBL, PHINode **PHIA,
                           PHINode **PHIN) -> bool {
    BasicBlock *BBE = &F.getEntryBlock();
    for (PHINode &PHI : BBL->phis())
      for (unsigned I = 0, E = PHI.getNumIncomingValues(); I < E; ++I)
        if (PHI.getIncomingBlock(I) == BBE) {
          Value *PHIV = PHI.getIncomingValue(I);
          if (PHIV == F.getArg(0))
            *PHIA = &PHI;
          else if (PHIV == F.getArg(1))
            *PHIN = &PHI;
        }
    return *PHIA && *PHIN;
  };

  //
  // Return 'true' if 'BBI' matches a BasicBlock of the form:
  //
  //   %sub.ptr.lhs.cast = ptrtoint i8* 'PHIA' to i64
  //   %sub.ptr.sub = sub i64 %sub.ptr.lhs.cast, 0
  //   %rem = urem i64 %sub.ptr.sub, 'UDen'
  //   %tobool = icmp ne i64 %rem, 0
  //   br i1 %tobool, label 'BBL', label %'BBR'
  //
  // If we return 'true', set the values of 'BBL' and 'BBR' and place a
  // ['BBI', 2] entry in the 'PHIMap'.
  //
  auto IsAddrAlignTestBlock = [](BasicBlock *BBI, PHINode *PHIA, uint64_t UDen,
                                 DenseMapBBToV &PHIMap, BasicBlock **BBL,
                                 BasicBlock **BBR) -> bool {
    auto BI = dyn_cast<BranchInst>(BBI->getTerminator());
    if (!BI || BI->isUnconditional())
      return false;
    ConstantInt *CIDen = nullptr;
    ICmpInst::Predicate Pred = ICmpInst::BAD_ICMP_PREDICATE;
    if (!match(BI->getCondition(),
               m_ICmp(Pred,
                      m_URem(m_Sub(m_PtrToInt(m_Specific(PHIA)), m_Zero()),
                             m_ConstantInt(CIDen)),
                      m_Zero())))
      return false;
    if (CIDen->getZExtValue() != UDen || Pred != ICmpInst::ICMP_NE)
      return false;
    *BBL = BI->getSuccessor(0);
    *BBR = BI->getSuccessor(1);
    LLVMContext &FC = BBI->getContext();
    PHIMap[BBI] = ConstantInt::get(llvm::Type::getInt32Ty(FC), 2);
    return true;
  };

  //
  // Return 'true' if 'BBI' in 'F' matches a BasicBlock of the form:
  //
  //   %rem1 = urem i64 %es, 'UDen'
  //   %tobool2 = icmp ne i64 %rem1, 0
  //   br i1 %tobool2, label 'BBX', label 'BBO'
  //
  // If we return 'true', set the value of 'BBO' and place a ['BBI', 2]
  // entry in the 'PHIMap'.
  //
  auto IsESAlignTestBlock = [](Function &F, BasicBlock *BBI, uint64_t UDen,
                               DenseMapBBToV &PHIMap, BasicBlock *BBX,
                               BasicBlock **BBO) -> bool {
    auto BI = dyn_cast<BranchInst>(BBI->getTerminator());
    if (!BI || BI->isUnconditional())
      return false;
    ConstantInt *CIDen = nullptr;
    ICmpInst::Predicate Pred = ICmpInst::BAD_ICMP_PREDICATE;
    if (!match(BI->getCondition(),
               m_ICmp(Pred,
                      m_URem(m_Specific(F.getArg(2)), m_ConstantInt(CIDen)),
                      m_Zero())))
      return false;
    if (CIDen->getZExtValue() != UDen || Pred != ICmpInst::ICMP_NE)
      return false;
    if (BI->getSuccessor(0) != BBX)
      return false;
    *BBO = BI->getSuccessor(1);
    LLVMContext &FC = BBI->getContext();
    PHIMap[BBI] = ConstantInt::get(llvm::Type::getInt32Ty(FC), 2);
    return true;
  };

  //
  //  Return 'true' if 'BBI' in 'F' matches a BasicBlock of the form:
  //
  //      %cmp3 = icmp eq i64 %es,'UDen'
  //      'BC' = select i1 %cmp3, i32 0, i32 1
  //      br label 'BBX'
  //
  //  If we return 'true', place a ['BBI', 'BC'] entry in the 'PHIMap'.
  //
  auto IsSelBlock = [](Function &F, BasicBlock *BBI, uint64_t UDen,
                       DenseMapBBToV &PHIMap, BasicBlock *BBX) -> bool {
    auto BI = dyn_cast<BranchInst>(BBI->getTerminator());
    if (!BI || BI->isConditional())
      return false;
    ConstantInt *CIDen = nullptr;
    Instruction *BC = BI->getPrevNonDebugInstruction();
    ICmpInst::Predicate Pred = ICmpInst::BAD_ICMP_PREDICATE;
    if (!BC ||
        !match(BC, m_SelectCst<0, 1>(m_ICmp(Pred, m_Specific(F.getArg(2)),
                                            m_ConstantInt(CIDen)))))
      return false;
    if (CIDen->getZExtValue() != UDen || Pred != ICmpInst::ICMP_EQ)
      return false;
    if (BI->getSuccessor(0) != BBX)
      return false;
    PHIMap[BBI] = BC;
    return true;
  };

  //
  // Return 'true' if 'BBPHI' starts with a PHINode whose incoming Values
  // and BasicBlocks are given by the 'PHIMap'.
  //
  auto IsPHIJoin = [](BasicBlock *BBPHI, DenseMapBBToV &PHIMap,
                      PHINode **PHIO) -> bool {
    BasicBlock::phi_iterator CI = BBPHI->phis().begin();
    if (CI == BBPHI->phis().end())
      return false;
    PHINode *PHIN = &*CI;
    for (unsigned I = 0, E = PHIN->getNumIncomingValues(); I < E; ++I) {
      Value *VI = PHIN->getIncomingValue(I);
      BasicBlock *BBI = PHIN->getIncomingBlock(I);
      if (PHIMap[BBI] != VI)
        return false;
    }
    *PHIO = PHIN;
    return true;
  };

  //
  // Return 'true' if the four BasicBlock sequence beginning with 'BBI'
  // in 'F' matches a 'SWAPINIT(type, a, es)' where 'PHIA' is the PHINode
  // for 'a', 'UDen' is the size of the integer 'type' in bytes. Note that
  // 'a' and 'es' are the #0 and #2 arguments of 'F'.
  //
  // If we return 'true', set the value of 'BBO', which is the exit of the
  // BasicBlock sequence and 'PHIO', which is the joined PHI in 'BBO'.
  //
  auto IsSwapInit = [&IsAddrAlignTestBlock, &IsESAlignTestBlock, &IsSelBlock,
                     &IsPHIJoin](Function &F, BasicBlock *BBI, PHINode *PHIA,
                                 uint64_t UDen, PHINode **PHIO,
                                 BasicBlock **BBO) -> bool {
    BasicBlock *BBES = nullptr;
    BasicBlock *BBSel = nullptr;
    BasicBlock *BBX = nullptr;
    PHINode *PHI = nullptr;
    DenseMapBBToV PHIMap;
    if (!IsAddrAlignTestBlock(BBI, PHIA, UDen, PHIMap, &BBX, &BBES))
      return false;
    if (!IsESAlignTestBlock(F, BBES, UDen, PHIMap, BBX, &BBSel))
      return false;
    if (!IsSelBlock(F, BBSel, UDen, PHIMap, BBX))
      return false;
    if (!IsPHIJoin(BBX, PHIMap, &PHI))
      return false;
    *PHIO = PHI;
    *BBO = BBX;
    return true;
  };

  //
  //  Return 'true' if 'BBI' matches a BasicBlock of the form:
  //
  //    %cmp51 = icmp 'Pred' i64 'PHIN', 'SmallSize'
  //    br i1 %cmp51, label 'BBL', label 'BBR'
  //
  //  If we return 'true', we assign values to 'BBL' and 'BBR'.
  //
  auto IsSizeTest = [](BasicBlock *BBI, PHINode *PHIN, ICmpInst::Predicate Pred,
                       uint64_t SmallSize, BasicBlock **BBL,
                       BasicBlock **BBR) -> bool {
    BranchInst *BI = nullptr;
    ICmpInst *IC = nullptr;
    if (!getBIAndIC(BBI, Pred, &BI, &IC))
      return false;
    if (IC->getOperand(0) != PHIN)
      return false;
    auto CI = dyn_cast<ConstantInt>(IC->getOperand(1));
    if (!CI || CI->getZExtValue() != SmallSize)
      return false;
    *BBL = BI->getSuccessor(0);
    *BBR = BI->getSuccessor(1);
    return true;
  };

  //
  //  Return 'true' if 'BBI' in 'F' matches a BasicBlock of the form:
  //
  //    'V0' = getelementptr inbounds i8, i8* 'PHIA', i64 'F(2)'
  //    br label 'BBO'
  //
  //  If we return 'true', we assign values to 'V0' and 'BBO'.
  //
  auto IsISInit = [](Function &F, BasicBlock *BBI, PHINode *PHIA, Value **VO,
                     BasicBlock **BBO) -> bool {
    auto BI = dyn_cast<BranchInst>(BBI->getTerminator());
    if (!BI || BI->isConditional())
      return false;
    auto GEP = dyn_cast<GetElementPtrInst>(BI->getPrevNonDebugInstruction());
    if (!GEP || GEP->getNumOperands() != 2 ||
        GEP->getPointerOperand() != PHIA || GEP->getOperand(1) != F.getArg(2))
      return false;
    *VO = GEP;
    *BBO = BI->getSuccessor(0);
    return true;
  };

  //
  //  Return 'true' if 'BBI' in 'F' matches a BasicBlock of the form:
  //
  //    'PHIO' = phi i8* [ 'VI', %if.then ], [ %add.ptr46, 'BBOLI' ]
  //    %mul = mul i64 'PHIN', 'F(2)'
  //    %add.ptr19 = getelementptr inbounds i8, i8* 'PHIA', i64 %mul
  //    %cmp20 = icmp ult i8* 'PHIO', %add.ptr19
  //    br i1 %cmp20, label 'BBL', label 'BBR'
  //
  //  If we return 'true', we assign values to 'PHIO', 'BBOLI', 'BBL', and
  //  'BBR'.
  //
  auto IsISOTerm = [](Function &F, BasicBlock *BBI, PHINode *PHIA,
                      PHINode *PHIN, Value *VI, PHINode **PHIO,
                      BasicBlock **BBOLI, BasicBlock **BBL,
                      BasicBlock **BBR) -> bool {
    BranchInst *BI = nullptr;
    ICmpInst *IC = nullptr;
    if (!getBIAndIC(BBI, ICmpInst::ICMP_ULT, &BI, &IC))
      return false;
    auto PHI = dyn_cast<PHINode>(IC->getOperand(0));
    if (!PHI)
      return false;
    auto GEP = dyn_cast<GetElementPtrInst>(IC->getOperand(1));
    if (!GEP || GEP->getNumOperands() != 2 || GEP->getPointerOperand() != PHIA)
      return false;
    auto MUL = dyn_cast<BinaryOperator>(GEP->getOperand(1));
    if (!MUL || MUL->getOpcode() != Instruction::Mul ||
        MUL->getOperand(0) != PHIN || MUL->getOperand(1) != F.getArg(2))
      return false;
    if (PHI->getIncomingValue(0) != VI)
      return false;
    *PHIO = PHI;
    *BBOLI = PHI->getIncomingBlock(1);
    *BBL = BI->getSuccessor(0);
    *BBR = BI->getSuccessor(1);
    return true;
  };

  //
  //  Return 'true' if 'BBI' in 'F' matches a BasicBlock of the form:
  //
  //    'PHIO' = phi i8* [ %pm.0, %for.body ], [ %add.ptr44, %for.inc ]
  //    %cmp22 = icmp ugt i8* 'PHIO', 'PHIA'
  //    br i1 %cmp22, label 'BBL', label 'BBR'
  //
  //  If we return 'true', we assign values to 'PHIO', 'BBOLI', 'BBL', and
  //  'BBR'.
  //
  auto IsISITerm1 = [](Function &F, BasicBlock *BBI, PHINode *PHIA,
                       PHINode **PHIO, BasicBlock **BBL,
                       BasicBlock **BBR) -> bool {
    BranchInst *BI = nullptr;
    ICmpInst *IC = nullptr;
    if (!getBIAndIC(BBI, ICmpInst::ICMP_UGT, &BI, &IC))
      return false;
    auto PHI = dyn_cast<PHINode>(IC->getOperand(0));
    if (!PHI)
      return false;
    if (IC->getOperand(1) != PHIA)
      return false;
    *PHIO = PHI;
    *BBL = BI->getSuccessor(0);
    *BBR = BI->getSuccessor(1);
    return true;
  };

  //
  //  Return 'true' if 'BBI' in 'F' matches a BasicBlock of the form:
  //
  //    %idx.neg = sub i64 0, 'F(2)'
  //    %add.ptr23 = getelementptr inbounds i8, i8* 'PHIIL', i64 %idx.neg
  //    %call = call i32 'F(3)'(i8* %add.ptr23, i8* 'PHIIL')
  //    %cmp24 = icmp sgt i32 %call, 0
  //    br i1 %cmp24, label 'BBL', label 'BBR'
  //
  //  If we return 'true', we assign values to 'BBL' and 'BBR'.
  //
  auto IsISITerm2 = [](Function &F, BasicBlock *BBI, PHINode *PHIIL,
                       BasicBlock **BBL, BasicBlock **BBR) -> bool {
    BranchInst *BI = nullptr;
    ICmpInst *IC = nullptr;
    if (!getBIAndIC(BBI, ICmpInst::ICMP_SGT, &BI, &IC))
      return false;
    auto CI = dyn_cast<ConstantInt>(IC->getOperand(1));
    if (!CI || !CI->isZero())
      return false;
    auto CLI = dyn_cast<CallInst>(IC->getOperand(0));
    if (!CLI || CLI->getCalledOperand() != F.getArg(3) || CLI->arg_size() != 2)
      return false;
    if (CLI->getArgOperand(1) != PHIIL)
      return false;
    auto GEP = dyn_cast<GetElementPtrInst>(CLI->getArgOperand(0));
    if (!GEP || GEP->getNumOperands() != 2 || GEP->getPointerOperand() != PHIIL)
      return false;
    if (!match(GEP->getOperand(1), m_Sub(m_Zero(), m_Specific(F.getArg(2)))))
      return false;
    *BBL = BI->getSuccessor(0);
    *BBR = BI->getSuccessor(1);
    return true;
  };

  //
  //  Return 'true' if 'BBI' in 'F' starts a pair of BasicBlocks of the
  //  form:
  //
  //  'BBI':
  //    'PHIO' = phi i8* [ 'VI', %for.body ], [ %add.ptr44, 'BBILI' ]
  //    %cmp22 = icmp ugt i8* 'PHIO', 'PHIA'
  //    br i1 %cmp22, label %land.rhs, label 'BBOLI'
  //  land.rhs:
  //    %idx.neg = sub i64 0, 'F(2)'
  //    %add.ptr23 = getelementptr inbounds i8, i8* 'PHIO', i64 %idx.neg
  //    %call = call i32 'F(3)'(i8* %add.ptr23, i8* 'PHIO')
  //    %cmp24 = icmp sgt i32 %call, 0
  //    br i1 %cmp24, label 'BBL', label 'BBOLI'
  //
  //  If we return 'true', we assign values to 'PHIO', 'BBILI' and 'BBL'.
  //
  auto IsISITerm = [&IsISITerm1,
                    IsISITerm2](Function &F, BasicBlock *BBI, PHINode *PHIA,
                                Value *VI, BasicBlock *BBOLI, PHINode **PHIO,
                                BasicBlock **BBILI, BasicBlock **BBL) -> bool {
    PHINode *PHIIL = nullptr;
    BasicBlock *BBC = nullptr;
    BasicBlock *BBID = nullptr;
    BasicBlock *BBX0 = nullptr;
    BasicBlock *BBX1 = nullptr;
    if (!IsISITerm1(F, BBI, PHIA, &PHIIL, &BBC, &BBX0) || BBX0 != BBOLI)
      return false;
    if (PHIIL->getIncomingValue(0) != VI)
      return false;
    if (!IsISITerm2(F, BBC, PHIIL, &BBID, &BBX1) || BBX1 != BBX0)
      return false;
    *PHIO = PHIIL;
    *BBILI = PHIIL->getIncomingBlock(1);
    *BBL = BBID;
    return true;
  };

  //
  //  Return 'true' if 'BBI' in 'F' is a BasicBlock of the form:
  //
  //    %cmp26 = icmp eq i32 'PHIC', 0
  //    br i1 %cmp26, label 'BBL', label 'BBR'
  //
  //  If we return 'true', we assign values to 'BBL' and 'BBR'.
  //
  auto IsPHITest = [](BasicBlock *BBI, PHINode *PHIC, BasicBlock **BBL,
                      BasicBlock **BBR) -> bool {
    BranchInst *BI = nullptr;
    ICmpInst *IC = nullptr;
    if (!getBIAndIC(BBI, ICmpInst::ICMP_EQ, &BI, &IC))
      return false;
    auto CI = dyn_cast<ConstantInt>(IC->getOperand(1));
    if (!CI || !CI->isZero())
      return false;
    if (IC->getOperand(0) != PHIC)
      return false;
    *BBL = BI->getSuccessor(0);
    *BBR = BI->getSuccessor(1);
    return true;
  };

  //
  //  Return 'true' if 'SI' starts a chain of Instructions of the form;
  //
  //    %4 = bitcast i8* 'VLO' to i64*
  //    %5 = load i64, i64* %4, align 8
  //    %6 = bitcast i8* 'VSO' to i64*
  //    'SI': store i64 %5, i64* %6, align 8
  //
  //  If we return 'true', we assign values to 'VLO' and 'VSO'.
  //
  // NOTE: Bitcasts may not be present when -opaque-pointers becomes
  // the default.
  //
  auto IsSwapLSChain = [](StoreInst *SI, Value **VLO, Value **VSO) -> bool {
    auto LI = dyn_cast<LoadInst>(SI->getValueOperand());
    if (!LI)
      return false;
    auto BC0 = dyn_cast<BitCastInst>(LI->getPointerOperand());
    *VLO = BC0 ? BC0->getOperand(0) : LI->getPointerOperand();
    auto BC1 = dyn_cast<BitCastInst>(SI->getPointerOperand());
    *VSO = BC1 ? BC1->getOperand(0) : SI->getPointerOperand();
    return true;
  };

  //
  //  Return 'true' if 'VI' in 'F' starts a chain of Instructions of the form:
  //
  //    %idx.neg28 = sub i64 0, 'F(2)'
  //    'VI' = getelementptr inbounds i8, i8* 'PHI', i64 %idx.neg28
  //
  // if 'IsNegative' is 'true' and:
  //
  //    'VI' = getelementptr inbounds i8, i8* 'PHI', i64 'F(2)'
  //
  // if 'IsNegative' is 'false'.
  //
  auto IsSwapGEPChain = [](Function &F, Value *VI, PHINode *PHI,
                           bool IsNegative) -> bool {
    Value *V0 = nullptr;
    auto GEP = dyn_cast<GetElementPtrInst>(VI);
    if (!GEP || GEP->getNumOperands() != 2 || GEP->getPointerOperand() != PHI)
      return false;
    if (IsNegative) {
      if (!match(GEP->getOperand(1), m_Sub(m_Zero(), m_Value(V0))))
        return false;
    } else {
      V0 = GEP->getOperand(1);
    }
    if (V0 != F.getArg(2))
      return false;
    return true;
  };

  //
  //  Return 'true' if 'BBI' in 'F' is a BasicBlock of the form:
  //
  //    %2 = bitcast i8* 'PHIIL' to i64*
  //    %3 = load 'DType', 'DType'* %2, align 8
  //    %idx.neg28 = sub i64 0, 'F(2)'
  //    %add.ptr29 = getelementptr inbounds i8, i8* 'PHIIL', i64 %idx.neg28
  //    %4 = bitcast i8* %add.ptr29 to 'DType'*
  //    %5 = load 'DType', 'DType'* %4, align 8
  //    %6 = bitcast i8* 'PHIIL' to 'DType'*
  //    store 'DType' %5, i64* %6, align 8
  //    %idx.neg30 = sub i64 0, 'F(2)'
  //    %add.ptr31 = getelementptr inbounds i8, i8* 'PHIIL', i64 %idx.neg30
  //    %7 = bitcast i8* %add.ptr31 to 'DType'*
  //    store 'DType' %3, 'DType'* %7, align 8
  //    br label 'BBX'
  //
  // NOTE: Bitcasts may not be present when -opaque-pointers becomes
  // the default.
  //
  auto IsHardSwapBlock = [&IsSwapLSChain, IsSwapGEPChain](
                             Function &F, BasicBlock *BBI, BasicBlock *BBX,
                             PHINode *PHIIL, Type *DType) -> bool {
    StoreInst *SI0 = nullptr;
    StoreInst *SI1 = nullptr;
    Value *SI0LV = nullptr;
    Value *SI1LV = nullptr;
    Value *SI0SV = nullptr;
    Value *SI1SV = nullptr;
    auto BI = dyn_cast<BranchInst>(BBI->getTerminator());
    if (!BI || BI->isConditional() || BI->getSuccessor(0) != BBX)
      return false;
    if (!getTwoStores(BBI, &SI0, &SI1))
      return false;
    if (SI0->getValueOperand()->getType() != DType)
      return false;
    if (SI1->getValueOperand()->getType() != DType)
      return false;
    if (!IsSwapLSChain(SI0, &SI0LV, &SI0SV))
      return false;
    if (!IsSwapGEPChain(F, SI0LV, PHIIL, true))
      return false;
    if (SI0SV != PHIIL)
      return false;
    if (!IsSwapLSChain(SI1, &SI1LV, &SI1SV))
      return false;
    if (!IsSwapGEPChain(F, SI1SV, PHIIL, true))
      return false;
    if (SI1LV != PHIIL)
      return false;
    return true;
  };

  //
  //  Return 'true' if 'BBI' in 'F' is a BasicBlock of the form:
  //
  //    %idx.neg40 = sub i64 0, 'F(2)'
  //    %add.ptr41 = getelementptr inbounds i8, i8* 'PHIIL', i64 %idx.neg40
  //    %conv = trunc i64 'F(2)' to i32
  //    call void 'FSwapFunc'(i8* 'PHIIL', i8* %add.ptr41, i32 %conv,
  //        i32 'PHIC0', i32 'PHIC1')
  //    br label 'BBX'
  //
  // If we return 'true', set the value of 'FSwapFunc'.
  //
  auto IsHardSwapFuncBlock = [&IsSwapGEPChain](Function &F, BasicBlock *BBI,
                                               BasicBlock *BBX, PHINode *PHIIL,
                                               PHINode *PHIC0, PHINode *PHIC1,
                                               Function **FSwapFunc) -> bool {
    auto BI = dyn_cast<BranchInst>(BBI->getTerminator());
    if (!BI || BI->isConditional() || BI->getSuccessor(0) != BBX)
      return false;
    Instruction *BP = BI->getPrevNonDebugInstruction();
    auto CI = dyn_cast_or_null<CallInst>(BP);
    if (!CI || CI->isIndirectCall() || CI->arg_size() != 5)
      return false;
    if (CI->getArgOperand(0) != PHIIL)
      return false;
    if (!IsSwapGEPChain(F, CI->getArgOperand(1), PHIIL, true))
      return false;
    auto TI = dyn_cast<TruncInst>(CI->getArgOperand(2));
    if (!TI || TI->getOperand(0) != F.getArg(2))
      return false;
    if (CI->getArgOperand(3) != PHIC0 || CI->getArgOperand(4) != PHIC1)
      return false;
    *FSwapFunc = CI->getCalledFunction();
    return true;
  };

  //
  // Return 'true' if 'BBI' in 'F' is a BasicBlock of the form:
  //
  //   %idx.neg43 = sub i64 0, 'F(2)'
  //   'BP' = getelementptr inbounds i8, i8* 'PHIL', i64 %idx.neg43
  //   br label 'BBO'
  //
  // if 'IsOuterLoop' is 'false' and
  //
  //   'BP' = getelementptr inbounds i8, i8* 'PHIL', i64 'F(2)'
  //   br label 'BBO'
  //
  // if 'IsOuterLoop' is 'true'.
  //
  // Check also that the #1 input of 'PHIL' is ['BP', 'BBI'].
  //
  auto IsLoopIncBlock = [&IsSwapGEPChain](Function &F, BasicBlock *BBI,
                                          BasicBlock *BBO, PHINode *PHIL,
                                          bool IsOuterLoop) -> bool {
    auto BI = dyn_cast<BranchInst>(BBI->getTerminator());
    if (!BI || BI->isConditional())
      return false;
    Instruction *BP = BI->getPrevNonDebugInstruction();
    if (!BP)
      return false;
    if (!IsSwapGEPChain(F, BP, PHIL, !IsOuterLoop))
      return false;
    if (PHIL->getIncomingValue(1) != BP || PHIL->getIncomingBlock(1) != BBI)
      return false;
    return true;
  };

  //
  // Return 'true' if 'BBI' in 'F' starts an insertion sort, where 'PHIA'
  // is the beginning of the array being sorted, 'PHIN' is the length,
  // 'PHIC0' is the condition testing if we are sorting an array of 'long'
  // and 'PHIC1' is the condition testing if we are sorting an array of 'int'.
  //
  // If we return 'true' set the values of 'BBO', the BasicBlock exiting the
  // insertion sort and 'FSwapFunc', the Function that implements qsort
  // swapfunc.
  //
  auto IsInsertionSort =
      [&IsISInit, &IsISOTerm, &IsISITerm, &IsPHITest, &IsHardSwapBlock,
       &IsHardSwapFuncBlock,
       &IsLoopIncBlock](Function &F, BasicBlock *BBI, PHINode *PHIA,
                        PHINode *PHIN, PHINode *PHIC0, PHINode *PHIC1,
                        BasicBlock **BBO, Function **FSwapFunc) -> bool {
    Value *VI = nullptr;
    BasicBlock *BBOT = nullptr;
    BasicBlock *BBIT = nullptr;
    BasicBlock *BBOLI = nullptr;
    BasicBlock *BBILI = nullptr;
    BasicBlock *BBOD = nullptr;
    BasicBlock *BBT0 = nullptr;
    BasicBlock *BBT1 = nullptr;
    BasicBlock *BBX = nullptr;
    BasicBlock *BBSW0 = nullptr;
    BasicBlock *BBSW1 = nullptr;
    BasicBlock *BBSW2 = nullptr;
    PHINode *PHIOL = nullptr;
    PHINode *PHIIL = nullptr;
    if (!IsISInit(F, BBI, PHIA, &VI, &BBOT))
      return false;
    if (!IsISOTerm(F, BBOT, PHIA, PHIN, VI, &PHIOL, &BBOLI, &BBOD, &BBX))
      return false;
    if (!isDirectBranchBlock(BBOD, &BBIT))
      return false;
    if (!IsISITerm(F, BBIT, PHIA, PHIOL, BBOLI, &PHIIL, &BBILI, &BBT0))
      return false;
    if (!IsPHITest(BBT0, PHIC0, &BBSW0, &BBT1))
      return false;
    LLVMContext &FC = BBI->getContext();
#ifdef _WIN32
    Type *TILong = llvm::Type::getInt32Ty(FC);
#else
    Type *TILong = llvm::Type::getInt64Ty(FC);
#endif // _WIN32
    if (!IsHardSwapBlock(F, BBSW0, BBILI, PHIIL, TILong))
      return false;
    if (!IsPHITest(BBT1, PHIC1, &BBSW1, &BBSW2))
      return false;
    if (!IsHardSwapBlock(F, BBSW1, BBILI, PHIIL, llvm::Type::getInt32Ty(FC)))
      return false;
    if (!IsHardSwapFuncBlock(F, BBSW2, BBILI, PHIIL, PHIC0, PHIC1, FSwapFunc))
      return false;
    if (!IsLoopIncBlock(F, BBILI, BBIT, PHIIL, false))
      return false;
    if (!IsLoopIncBlock(F, BBOLI, BBOT, PHIOL, true))
      return false;
    *BBO = BBX;
    return true;
  };

  //
  // If 'VGEP' is a byte-flattened GEP with pointer operand 'GPO', return 'VGEP'
  // as a GetElementPtrInst.
  //
  auto GetBFGEP = [](Value *VGEP, Value *GPO) -> GetElementPtrInst * {
    auto GEP = dyn_cast_or_null<GetElementPtrInst>(VGEP);
    if (!GEP || GEP->getNumOperands() != 2 || GEP->getPointerOperand() != GPO)
      return nullptr;
    return GEP;
  };

  //
  // Return the first byte-flattened GEP before 'I' in a backward search of
  // 'I's BasicBlock, if the pointer operand of that byte-flattened GEP matches
  // 'GPO'.
  //
  auto GetPrevBFGEP = [&GetBFGEP](Instruction *I,
                                  Value *GPO) -> GetElementPtrInst * {
    Instruction *BP = I->getPrevNonDebugInstruction();
    while (BP && !isa<GetElementPtrInst>(BP))
      BP = BP->getPrevNonDebugInstruction();
    return BP ? GetBFGEP(BP, GPO) : nullptr;
  };

  //
  // If 'VGEP' is a byte-flattened GEP with pointer operand 'GPO', return the
  // offset of that byte-flattened GEP.
  //
  auto GetBFGEPO = [&GetBFGEP](Value *VGEP, Value *GPO) -> Value * {
    GetElementPtrInst *GEP = GetBFGEP(VGEP, GPO);
    return GEP ? GEP->getOperand(1) : nullptr;
  };

  //
  // If 'BBI' is terminated with a conditional test, which is preceded by
  // a byte-flattened GEP, return that byte-flattened GEP as a
  // GetElementPtrInst.
  //
  auto BottomBFGEP = [&GetBFGEP](BasicBlock *BBI,
                                 Value *GPO) -> GetElementPtrInst * {
    BranchInst *BI = nullptr;
    ICmpInst *IC = nullptr;
    if (!getBIAndIC(BBI, ICmpInst::ICMP_UGT, &BI, &IC))
      return nullptr;
    Instruction *BP = IC->getPrevNonDebugInstruction();
    return GetBFGEP(BP, GPO);
  };

  //
  // Return 'true' if 'BBI' in 'F' matches the sequence:
  //
  //   %div = udiv i64 'PHIN', 2
  //   %mul49 = mul i64 %div, 'F(2)'
  //   'VPMI' = getelementptr inbounds i8, i8* 'PHIA', i64 %mul49
  //
  // When we return 'true', set the value of 'VPMI'.
  //
  auto ComputesPM = [&BottomBFGEP](Function &F, BasicBlock *BBI, PHINode *PHIA,
                                   PHINode *PHIN, Value **VPMI) -> bool {
    GetElementPtrInst *GEP = BottomBFGEP(BBI, PHIA);
    if (!GEP)
      return false;
    ConstantInt *CI = nullptr;
    Value *GEPO = GEP->getOperand(1);
    if (!match(GEPO, m_Mul(m_UDiv(m_Specific(PHIN), m_ConstantInt(CI)),
                           m_Specific(F.getArg(2)))))
      return false;
    if (CI->getZExtValue() != 2)
      return false;
    *VPMI = GEP;
    return true;
  };

  //
  // Return 'true' if 'BBI' in 'F' matches the sequence:
  //
  //   %sub = sub i64 'PHIN', 1
  //   %mul54 = mul i64 %sub, 'F(2)'
  //   'VPNI' = getelementptr inbounds i8, i8* 'PHIA', i64 %mul54
  //
  // When we return 'true', set the value of 'VPNI'.
  //
  auto ComputesPN = [&BottomBFGEP](Function &F, BasicBlock *BBI, PHINode *PHIA,
                                   PHINode *PHIN, Value **VPNI) -> bool {
    GetElementPtrInst *GEP = BottomBFGEP(BBI, PHIA);
    if (!GEP)
      return false;
    Value *GEPO = GEP->getOperand(1);
    if (!match(GEPO, m_Mul(m_Sub(m_Specific(PHIN), m_One()),
                           m_Specific(F.getArg(2)))))
      return false;
    *VPNI = GEP;
    return true;
  };

  //
  // Return 'true' if traversing back from 'I' we find a CallInst representing
  // a direct call to a function with 4 arguments, the last of which is 'F(3)'.
  // Furthermore, if 'I' itself is a CallInst, ensure that the CallInst found
  // calls the same function as 'I'. If during the traversal, another
  // Instruction that may write memory is encountered, return 'false'. We will
  // also return 'true' if no such CallInst is found, as long as we don't
  // encounter another Instruction that may write memory. If an appropriate
  // CallInst is found, we set 'CO' to it.
  //
  auto GetPrevCallMatch = [](Function &F, Instruction *I,
                             CallInst **CO) -> bool {
    Instruction *BP = nullptr;
    Instruction *BPI = I->getPrevNonDebugInstruction();
    for (BP = BPI; BP; BP = BP->getPrevNonDebugInstruction()) {
      if (auto CI = dyn_cast<CallInst>(BP)) {
        if (CI->isIndirectCall() || CI->arg_size() != 4 ||
            CI->getArgOperand(3) != F.getArg(3))
          return false;
        if (auto PCI = dyn_cast<CallInst>(I))
          if (CI->getCalledFunction() != PCI->getCalledFunction())
            return false;
        *CO = CI;
        return true;
      } else if (BPI->mayWriteToMemory()) {
        return false;
      }
    }
    *CO = nullptr;
    return true;
  };

  //
  // Return 'true' if 'BBI' in 'F' matches a BasicBlock of the form:
  //
  //   %div59 = udiv i64 'PHIN', 8
  //   %mul60 = mul i64 %div59, 'F(2)'
  //   %add.ptr61 = getelementptr inbounds i8, i8* 'VPLI', i64 %mul60
  //   %mul62 = mul i64 2, %mul60
  //   %add.ptr63 = getelementptr inbounds i8, i8* 'VPLI', i64 %mul62
  //   'VPLO' = call i8* 'FMed3'(i8* %a.addr.0, i8* %add.ptr61, i8* %add.ptr63,
  //        i32 (i8*, i8*)* 'F(3)')
  //   %idx.neg65 = sub i64 0, %mul60
  //   %add.ptr66 = getelementptr inbounds i8, i8* 'VPMI', i64 %idx.neg65
  //   %add.ptr67 = getelementptr inbounds i8, i8* 'VPMI', i64 %mul60
  //   'VPMO' = call i8* 'FMed3'(i8* %add.ptr66, i8* 'VPMI', i8* %add.ptr67,
  //        i32 (i8*, i8*)* 'F(3)')
  //   %mul69 = mul i64 2, %mul60
  //   %idx.neg70 = sub i64 0, %mul69
  //   %add.ptr71 = getelementptr inbounds i8, i8* 'VPNI', i64 %idx.neg70
  //   %idx.neg72 = sub i64 0, %mul60
  //   %add.ptr73 = getelementptr inbounds i8, i8* 'VPNI', i64 %idx.neg72
  //   'VPNO' = call i8* 'FMed3'(i8* %add.ptr71, i8* %add.ptr73, i8* %add.ptr55,
  //        i32 (i8*, i8*)* 'F(3)')
  //   br label 'BBO'
  //
  // If we return 'true', set the values of 'VPLO', 'VPMO', 'VPNO', and
  // 'FMed3'.
  //
  auto IsMed3CallBlock = [&GetBFGEPO, &GetPrevCallMatch](
                             Function &F, BasicBlock *BBI, BasicBlock *BBO,
                             PHINode *PHIN, PHINode *VPLI, Value *VPMI,
                             Value *VPNI, Value **VPLO, Value **VPMO,
                             Value **VPNO, Function **FMed3) -> bool {
    auto BI = dyn_cast<BranchInst>(BBI->getTerminator());
    if (!BI || BI->isConditional() || BI->getSuccessor(0) != BBO)
      return false;
    CallInst *CI2 = nullptr;
    if (!GetPrevCallMatch(F, BI, &CI2) || !CI2)
      return false;
    Function *PFMed3 = CI2->getCalledFunction();
    if (CI2->getArgOperand(2) != VPNI)
      return false;
    Value *GEP20 = GetBFGEPO(CI2->getArgOperand(0), VPNI);
    ConstantInt *CNI2 = nullptr;
    Value *VD = nullptr;
    if (!GEP20 ||
        !match(GEP20, m_Sub(m_Zero(), m_Mul(m_ConstantInt(CNI2), m_Value(VD)))))
      return false;
    if (CNI2->getZExtValue() != 2)
      return false;
    ConstantInt *CNI8 = nullptr;
    if (!match(VD, m_Mul(m_UDiv(m_Specific(PHIN), m_ConstantInt(CNI8)),
                         m_Specific(F.getArg(2)))))
      return false;
    if (CNI8->getZExtValue() != 8)
      return false;
    Value *GEP21 = GetBFGEPO(CI2->getArgOperand(1), VPNI);
    if (!GEP21 || !match(GEP21, m_Sub(m_Zero(), m_Specific(VD))))
      return false;
    CallInst *CI1 = nullptr;
    if (!GetPrevCallMatch(F, CI2, &CI1) || !CI1)
      return false;
    if (CI1->getArgOperand(1) != VPMI)
      return false;
    Value *GEP10 = GetBFGEPO(CI1->getArgOperand(0), VPMI);
    if (!GEP10 || !match(GEP10, m_Sub(m_Zero(), m_Specific(VD))))
      return false;
    Value *GEP12 = GetBFGEPO(CI1->getArgOperand(2), VPMI);
    if (GEP12 != VD)
      return false;
    CallInst *CI0 = nullptr;
    if (!GetPrevCallMatch(F, CI1, &CI0) || !CI0)
      return false;
    if (CI0->getArgOperand(0) != VPLI)
      return false;
    Value *GEP01 = GetBFGEPO(CI0->getArgOperand(1), VPLI);
    if (GEP01 != VD)
      return false;
    Value *GEP02 = GetBFGEPO(CI0->getArgOperand(2), VPLI);
    if (!GEP02 || !match(GEP02, m_Mul(m_ConstantInt(CNI2), m_Specific(VD))))
      return false;
    if (CNI2->getZExtValue() != 2)
      return false;
    CallInst *CINull = nullptr;
    if (!GetPrevCallMatch(F, CI0, &CINull) || CINull)
      return false;
    *VPLO = CI0;
    *VPMO = CI1;
    *VPNO = CI2;
    *FMed3 = PFMed3;
    return true;
  };

  //
  // Return 'true' if 'I' matches a PHINode with 2 incoming values 'VI' and
  // 'VO'. If we return 'true', set 'PHIO' to 'I' cast as a PHINode'.
  //
  auto IsJoinPHI = [](Instruction *I, Value *VI, Value *VO,
                      PHINode **PHIO) -> bool {
    auto PHI = dyn_cast_or_null<PHINode>(I);
    if (!PHI || PHI->getNumIncomingValues() != 2)
      return false;
    if (PHI->getIncomingValue(0) != VO || PHI->getIncomingValue(1) != VI)
      return false;
    *PHIO = PHI;
    return true;
  };

  //
  // Return 'true' if 'BBI' in 'F' matches a BasicBlock of the form:
  //
  //   %pn.0 = phi i8* [ 'VPNO', %if.then58 ], [ 'VPNI', %if.then53 ]
  //   %pm.1 = phi i8* [ 'VPMO', %if.then58 ], [ 'VPMI', %if.then53 ]
  //   %pl.1 = phi i8* [ 'VPLO', %if.then58 ], [ 'VPLI', %if.then53 ]
  //   'VPMF' = call i8* 'FMed3'(i8* %pl.1, i8* %pm.1, i8* %pn.0,
  //        i32 (i8*, i8*)* 'F(3)')
  //   br label 'BBO'
  //
  // If we return 'true', set the value of 'VPMF'.
  //
  auto IsMed3JoinBlock = [&IsJoinPHI, GetPrevCallMatch](
                             Function &F, Function &FMed3, BasicBlock *BBI,
                             BasicBlock *BBO, Value *VPLI, Value *VPMI,
                             Value *VPNI, Value *VPLO, Value *VPMO, Value *VPNO,
                             Value **VPMF) -> bool {
    auto BI = dyn_cast<BranchInst>(BBI->getTerminator());
    if (!BI || BI->isConditional())
      return false;
    CallInst *CI = nullptr;
    if (!GetPrevCallMatch(F, BI, &CI))
      return false;
    if (CI->getCalledFunction() != &FMed3)
      return false;
    Instruction *BP = CI->getPrevNonDebugInstruction();
    PHINode *PHI2 = nullptr;
    if (!IsJoinPHI(BP, VPLI, VPLO, &PHI2) || CI->getArgOperand(0) != PHI2)
      return false;
    BP = BP->getPrevNonDebugInstruction();
    PHINode *PHI1 = nullptr;
    if (!IsJoinPHI(BP, VPMI, VPMO, &PHI1) || CI->getArgOperand(1) != PHI1)
      return false;
    BP = BP->getPrevNonDebugInstruction();
    PHINode *PHI0 = nullptr;
    if (!IsJoinPHI(BP, VPNI, VPNO, &PHI0) || CI->getArgOperand(2) != PHI0)
      return false;
    if (BP->getPrevNonDebugInstruction())
      return false;
    *VPMF = CI;
    return true;
  };

  //
  // Return 'true' if 'BBI' in 'F' represents the median computation that
  // appears in a spec_qsort. 'PHIA' is beginning of the array, and 'PHIN' is
  // the size of the array. If we return 'true', set 'VPMFO' to the median value
  // computed, 'VPMFI' to the previous value computed, 'BBO' to the BasicBlock
  // that exits the computation, and 'FMed3' to the function that should
  // implement 'med3'.
  //
  auto IsMedianComp =
      [&IsSizeTest, &ComputesPM, &ComputesPN, &IsMed3CallBlock,
       &IsMed3JoinBlock](Function &F, BasicBlock *BBI, PHINode *PHIA,
                         PHINode *PHIN, Value **VPMFI, Value **VPMFO,
                         BasicBlock **BBO, Function **FMed3) -> bool {
    BasicBlock *BBIT = nullptr;
    BasicBlock *BBX = nullptr;
    BasicBlock *BBCB = nullptr;
    BasicBlock *BBJB = nullptr;
    Function *FMed3X = nullptr;
    Value *VPMI = nullptr;
    Value *VPNI = nullptr;
    Value *VPLO = nullptr;
    Value *VPMO = nullptr;
    Value *VPNO = nullptr;
    Value *VPMOO = nullptr;
    if (!IsSizeTest(BBI, PHIN, ICmpInst::ICMP_UGT, 7, &BBIT, &BBX))
      return false;
    if (!IsSizeTest(BBIT, PHIN, ICmpInst::ICMP_UGT, 40, &BBCB, &BBJB))
      return false;
    if (!ComputesPM(F, BBI, PHIA, PHIN, &VPMI))
      return false;
    if (!ComputesPN(F, BBIT, PHIA, PHIN, &VPNI))
      return false;
    if (!IsMed3CallBlock(F, BBCB, BBJB, PHIN, PHIA, VPMI, VPNI, &VPLO, &VPMO,
                         &VPNO, &FMed3X))
      return false;
    if (!IsMed3JoinBlock(F, *FMed3X, BBJB, BBX, PHIA, VPMI, VPNI, VPLO, VPMO,
                         VPNO, &VPMOO))
      return false;
    *VPMFI = VPMI;
    *VPMFO = VPMOO;
    *BBO = BBX;
    *FMed3 = FMed3X;
    return true;
  };

  //
  // Return 'true' if 'BBI' is a BasicBlock of the form:
  //
  //   %14 = bitcast i8* 'VL' to 'DType'*
  //   %15 = load 'DType', 'DType'* %14, align 8
  //   %16 = bitcast i8* 'VR' to 'DType'*
  //   %17 = load 'DType', 'DType'* %16, align 8
  //   %18 = bitcast i8* 'VL' to 'DType'*
  //   store 'DType' %17, 'DType'* %18, align 8
  //   %19 = bitcast i8* 'VR' to 'DType'*
  //   store 'DType' %15, 'DType'* %19, align 8
  //   br label 'BBO'
  //
  // where 'DType' is the integer type of 'DSize' bytes.
  //
  // If we return 'true', set the value of 'BBO'.
  //
  // NOTE: Bitcasts may not be present when -opaque-pointers becomes
  // the default.
  //
  auto IsEasySwapBlock = [&IsSwapLSChain](BasicBlock *BBI, uint64_t DSize,
                                          Value *VL, Value *VR,
                                          BasicBlock **BBO) -> bool {
    auto BI = dyn_cast<BranchInst>(BBI->getTerminator());
    if (!BI || BI->isConditional())
      return false;
    StoreInst *SI0 = nullptr;
    StoreInst *SI1 = nullptr;
    if (!getTwoStores(BBI, &SI0, &SI1))
      return false;
    if (!SI0->getValueOperand()->getType()->isIntegerTy(8 * DSize))
      return false;
    if (!SI1->getValueOperand()->getType()->isIntegerTy(8 * DSize))
      return false;
    Value *S1LV = nullptr;
    Value *S1SV = nullptr;
    if (!IsSwapLSChain(SI1, &S1LV, &S1SV))
      return false;
    if (S1LV != VL || S1SV != VR)
      return false;
    Value *S0LV = nullptr;
    Value *S0SV = nullptr;
    if (!IsSwapLSChain(SI0, &S0LV, &S0SV))
      return false;
    if (S0LV != VR || S0SV != VL)
      return false;
    *BBO = BI->getSuccessor(0);
    return true;
  };

  //
  // Return 'true' if 'BBI' in 'F' is a BasicBlock of the form:
  //
  //   %conv88 = trunc i64 'F(2)' to i32
  //   call void 'FSwapFunc'(i8* 'VL', i8* 'VR', i32 %conv88, i32 'PHIC0',
  //       i32 'PHIC1')
  //   br label 'BBO'
  //
  auto IsEasySwapFuncBlock =
      [](Function &F, Function &FSwapFunc, BasicBlock *BBI, BasicBlock *BBO,
         Value *VL, Value *VR, PHINode *PHIC0, PHINode *PHIC1) -> bool {
    auto BI = dyn_cast<BranchInst>(BBI->getTerminator());
    if (!BI || BI->isConditional() || BI->getSuccessor(0) != BBO)
      return false;
    auto CI = dyn_cast_or_null<CallInst>(BI->getPrevNonDebugInstruction());
    if (!CI || CI->isIndirectCall() || CI->arg_size() != 5)
      return false;
    if (CI->getArgOperand(0) != VL || CI->getArgOperand(1) != VR ||
        CI->getArgOperand(3) != PHIC0 || CI->getArgOperand(4) != PHIC1)
      return false;
    auto TI = dyn_cast<TruncInst>(CI->getArgOperand(2));
    if (!TI || TI->getOperand(0) != F.getArg(2))
      return false;
    return true;
  };

  //
  // Return 'true' if 'BBI' in 'F' starts a sequence of BasicBlocks that
  // implement a swap, of longs, ints, or something else. The values pointed
  // to by 'VL' and 'VR' are swapped. 'PHIC0' is 'true' if we are swapping
  // long values. 'PHIC1' is 'true' if we are swapping int values. If we are
  // swapping something else, 'FSwapFunc' is used to do the swapping.
  // 'BytesInLong' is the number of bytes in a long. If we return 'true',
  // we set the value of 'BBO', the exit BasicBlock from the swap sequence.
  //
  auto IsEasySwap = [&IsPHITest, &IsEasySwapBlock, IsEasySwapFuncBlock](
                        Function &F, Function &FSwapFunc, BasicBlock *BBI,
                        Value *VL, Value *VR, PHINode *PHIC0, PHINode *PHIC1,
                        uint64_t BytesInLong, BasicBlock **BBO) -> bool {
    BasicBlock *BBSW0 = nullptr;
    BasicBlock *BBSW1 = nullptr;
    BasicBlock *BBSW2 = nullptr;
    BasicBlock *BBT0 = nullptr;
    BasicBlock *BBX = nullptr;
    BasicBlock *BBX0 = nullptr;
    if (!IsPHITest(BBI, PHIC0, &BBSW0, &BBT0))
      return false;
    if (!IsEasySwapBlock(BBSW0, BytesInLong, VL, VR, &BBX))
      return false;
    if (!IsPHITest(BBT0, PHIC1, &BBSW1, &BBSW2))
      return false;
    if (!IsEasySwapBlock(BBSW1, 4, VL, VR, &BBX0) || BBX0 != BBX)
      return false;
    if (!IsEasySwapFuncBlock(F, FSwapFunc, BBSW2, BBX, VL, VR, PHIC0, PHIC1))
      return false;
    *BBO = BBX;
    return true;
  };

  //
  // Return 'true' if 'BBI' contains a PHINode of the form:
  //
  //   'V0' = phi i8* [ 'VMPO', %if.end75 ], [ 'VMPI', %if.end48 ]
  //
  // If we return 'true', we set the value of 'VO'.
  //
  auto IsEasySwapInit0 = [&IsJoinPHI](BasicBlock *BBI, Value *VMFI, Value *VMFO,
                                      Value **VO) -> bool {
    BranchInst *BI = nullptr;
    ICmpInst *IC = nullptr;
    if (!getBIAndIC(BBI, ICmpInst::ICMP_EQ, &BI, &IC))
      return false;
    PHINode *PHI = nullptr;
    if (!IsJoinPHI(IC->getPrevNonDebugInstruction(), VMFO, VMFI, &PHI))
      return false;
    *VO = PHI;
    return true;
  };

  //
  // Return 'true' if 'BBI' in 'F' is a sequence swapping two values at
  // 'PHIA' and the join of 'VMFI' and 'VMFO'. 'PHIC0' indicates whether
  // we are swapping longs. 'PHIC1' indicates whether we are swapping ints.
  // If neither, 'FSwapFunc' is used to do the swapping. 'BytesInLong' is
  // the number of bytes in a long. If we return 'true', we set 'BBO' to the
  // exit BasicBlock of the sequence.
  //
  auto IsEasySwap0 = [&IsEasySwapInit0, &IsEasySwap](
                         Function &F, Function &FSwapFunc, BasicBlock *BBI,
                         PHINode *PHIA, Value *VMFI, Value *VMFO,
                         PHINode *PHIC0, PHINode *PHIC1, uint64_t BytesInLong,
                         BasicBlock **BBO) -> bool {
    Value *VMF = nullptr;
    if (!IsEasySwapInit0(BBI, VMFO, VMFI, &VMF))
      return false;
    uint64_t BIL = BytesInLong;
    BasicBlock *BBX = nullptr;
    if (!IsEasySwap(F, FSwapFunc, BBI, PHIA, VMF, PHIC0, PHIC1, BIL, &BBX))
      return false;
    *BBO = BBX;
    return true;
  };

  //
  // Return 'true' if 'BBI' in 'F' is a BasicBlock of the form:
  //
  //   'VPAI' = getelementptr inbounds i8, i8* 'PHIA', i64 'F(2)'
  //   %sub92 = sub i64 'PHIN', 1
  //   %mul93 = mul i64 %sub92, 'F(2)'
  //   'VPCI' = getelementptr inbounds i8, i8* 'PHIA', i64 %mul93
  //   br label 'BBO'
  //
  // If we return 'true', set the values of 'VPAI', 'VPCI' and 'BBO'.
  //
  auto IsMainInitBlock =
      [&GetBFGEP, &GetPrevBFGEP](Function &F, BasicBlock *BBI, PHINode *PHIA,
                                 PHINode *PHIN, Value **VPAI, Value **VPCI,
                                 BasicBlock **BBO) -> bool {
    auto BI = dyn_cast<BranchInst>(BBI->getTerminator());
    if (!BI || BI->isConditional())
      return false;
    Instruction *BP = BI->getPrevNonDebugInstruction();
    GetElementPtrInst *GEP1 = GetBFGEP(BP, PHIA);
    if (!GEP1)
      return false;
    if (!match(GEP1->getOperand(1), m_Mul(m_Sub(m_Specific(PHIN), m_One()),
                                          m_Specific(F.getArg(2)))))
      return false;
    GetElementPtrInst *GEP0 = GetPrevBFGEP(BP, PHIA);
    if (!GEP0 || GEP0->getOperand(1) != F.getArg(2))
      return false;
    *VPAI = GEP0;
    *VPCI = GEP1;
    *BBO = BI->getSuccessor(0);
    return true;
  };

  //
  // Return 'true' if 'BBI' is a BasicBlock of the form:
  //
  //   'PHISWC' = phi i32 [ 0, %if.end90 ], [ 1, %if.end169 ]
  //   'PHIPD' = phi i8* [ 'VPCI', %if.end90 ], [ %pd.1, %if.end169 ]
  //   'PHIPC' = phi i8* [ 'VPCI', %if.end90 ], [ %add.ptr172, %if.end169 ]
  //   'PHIPB' = phi i8* [ 'VPAI', %if.end90 ], [ %add.ptr170, %if.end169 ]
  //   'PHIPA' = phi i8* [ 'VPAI', %if.end90 ], [ %pa.1, %if.end169 ]
  //   br label 'BBO'
  //
  // If we return 'true' we set the values of 'PHIPA', 'PHIPBB', 'PHIPC',
  // 'PHIPD', 'PHISWC', and 'BBO'.
  //
  // Note: The incoming values on the right branch of the PHINodes in this
  // BasicBlock will be checked below in 'AreMainPHIsOK'.
  //
  auto IsMainHeadBlock = [](BasicBlock *BBI, Value *VPAI, Value *VPCI,
                            PHINode **PHIPA, PHINode **PHIPB, PHINode **PHIPC,
                            PHINode **PHIPD, PHINode **PHISWC,
                            BasicBlock **BBO) -> bool {
    auto BI = dyn_cast<BranchInst>(BBI->getTerminator());
    if (!BI || BI->isConditional())
      return false;
    auto PHIA0 = dyn_cast_or_null<PHINode>(BI->getPrevNonDebugInstruction());
    if (!PHIA0 || PHIA0->getNumIncomingValues() != 2 ||
        PHIA0->getIncomingValue(0) != VPAI)
      return false;
    auto PHIB0 = dyn_cast_or_null<PHINode>(PHIA0->getPrevNonDebugInstruction());
    if (!PHIB0 || PHIB0->getNumIncomingValues() != 2 ||
        PHIB0->getIncomingValue(0) != VPAI)
      return false;
    auto PHIC0 = dyn_cast_or_null<PHINode>(PHIB0->getPrevNonDebugInstruction());
    if (!PHIC0 || PHIC0->getNumIncomingValues() != 2 ||
        PHIC0->getIncomingValue(0) != VPCI)
      return false;
    auto PHID0 = dyn_cast_or_null<PHINode>(PHIC0->getPrevNonDebugInstruction());
    if (!PHID0 || PHID0->getNumIncomingValues() != 2 ||
        PHID0->getIncomingValue(0) != VPCI)
      return false;
    auto PHISW = dyn_cast_or_null<PHINode>(PHID0->getPrevNonDebugInstruction());
    if (!PHISW || PHISW->getNumIncomingValues() != 2)
      return false;
    auto CI0 = dyn_cast<ConstantInt>(PHISW->getIncomingValue(0));
    if (!CI0 || !CI0->isZero())
      return false;
    auto CI1 = dyn_cast<ConstantInt>(PHISW->getIncomingValue(1));
    if (!CI1 || !CI1->isOne())
      return false;
    *PHIPA = PHIA0;
    *PHIPB = PHIB0;
    *PHIPC = PHIC0;
    *PHIPD = PHID0;
    *PHISWC = PHISW;
    *BBO = BI->getSuccessor(0);
    return true;
  };

  //
  // Return 'true' if 'BBI' in 'F' is a BasicBlock of the form:
  //
  //   'CIO' = call i32 'F(3)'(i8* 'VL', i8* 'VR')
  //   %cmp100 = icmp 'Pred' i32 'CIO', 0
  //   br i1 %cmp100, label 'BBL', label 'BBR'
  //
  // If we return 'true', set the values of 'CIO', 'BBL' and 'BBR'.
  //
  auto IsPvtCallBlock = [](Function &F, BasicBlock *BBI, Value *VL, Value *VR,
                           ICmpInst::Predicate Pred, Value **CIO,
                           BasicBlock **BBL, BasicBlock **BBR) -> bool {
    BranchInst *BI = nullptr;
    ICmpInst *IC = nullptr;
    if (!getBIAndIC(BBI, Pred, &BI, &IC))
      return false;
    if (!match(IC->getOperand(1), m_Zero()))
      return false;
    auto CI = dyn_cast<CallInst>(IC->getOperand(0));
    if (!CI || CI->arg_size() != 2 || CI->getCalledOperand() != F.getArg(3))
      return false;
    if (CI->getArgOperand(0) != VL || CI->getArgOperand(1) != VR)
      return false;
    *CIO = CI;
    *BBL = BI->getSuccessor(0);
    *BBR = BI->getSuccessor(1);
    return true;
  };

  //
  // Return 'true' if 'BBI' is a BasicBlock of the form:
  //
  //   %cmp103 = icmp eq i32 'CCI', 0
  //   br i1 %cmp103, label 'BBL', label 'BBR'
  //
  // If we return 'true', set the values of 'BBL' and 'BBR'.
  //
  auto IsPvtCallResBlock = [](BasicBlock *BBI, Value *CII, BasicBlock **BBL,
                              BasicBlock **BBR) -> bool {
    BranchInst *BI = nullptr;
    ICmpInst *IC = nullptr;
    if (!getBIAndIC(BBI, ICmpInst::ICMP_EQ, &BI, &IC))
      return false;
    if (IC->getOperand(0) != CII || !match(IC->getOperand(1), m_Zero()))
      return false;
    *BBL = BI->getSuccessor(0);
    *BBR = BI->getSuccessor(1);
    return true;
  };

  //
  // Return 'true' if 'IsFwd' is 'true' and 'BBI' is a BasicBlock of the form:
  //
  //   'GEPO' = getelementptr inbounds i8, i8* 'PHII', i64 'F(2)'
  //   br label 'BBO'
  //
  // or if 'IsFwd' is 'false' and 'BBI' is a BasicBlock of the form:
  //
  //   %idx.neg147 = sub i64 0, 'F(2)'
  //   'GEPO' = getelementptr inbounds i8, i8* 'PHII', i64 %idx.neg147
  //   br label 'BBO'
  //
  // If we return 'true', set the values of 'GEPO' and 'BBO'.
  //
  auto IsPvtInnerIncBlock =
      [&GetBFGEP](Function &F, BasicBlock *BBI, PHINode *PHII, bool IsFwd,
                  GetElementPtrInst **GEPO, BasicBlock **BBO) -> bool {
    auto BI = dyn_cast<BranchInst>(BBI->getTerminator());
    if (!BI || BI->isConditional())
      return false;
    GetElementPtrInst *GEP = GetBFGEP(BI->getPrevNonDebugInstruction(), PHII);
    if (!GEP)
      return false;
    if (IsFwd) {
      if (GEP->getOperand(1) != F.getArg(2))
        return false;
    } else {
      if (!match(GEP->getOperand(1), m_Sub(m_Zero(), m_Specific(F.getArg(2)))))
        return false;
    }
    *GEPO = GEP;
    *BBO = BI->getSuccessor(0);
    return true;
  };

  //
  // Return 'true' if 'IsFwd' is 'true' and 'BBI' is a BasicBlock of the form:
  //
  //   'PHISWCO' = phi i32 [ 1, %if.end118 ], [ 'VSWPI', %while.body ]
  //   'PHIIO' = phi i8* [ 'GEPI', %if.end118 ], [ 'PHIII', %while.body ]
  //   'VOO' = getelementptr inbounds i8, i8* 'PHIOI', i64 'F(2)'
  //   br label 'BOO'
  //
  // or if 'IsFwd' is 'false' and 'BBI' is a BasicBlock of the form:
  //
  //   'PHISWCO' = phi i32 [ 1, %if.end146 ], [ 'VSWPI', %while.body130 ]
  //   'PHIIO' = phi i8* [ 'GEPI', %if.end146 ], [ 'PHIII', %while.body130 ]
  //   %idx.neg150 = sub i64 0, 'F(2)'
  //   'VOO' = getelementptr inbounds i8, i8* 'PHIOI', i64 %idx.neg150
  //   br label 'BOO'
  //
  // If we return 'true', set the values of 'PHISWCO', 'PHIIO', 'VOO', and
  // 'BBO'.
  //
  auto IsPvtOuterIncBlock = [&IsPvtInnerIncBlock](
                                Function &F, BasicBlock *BBI, PHINode *PHIII,
                                PHINode *PHIOI, Value *VSWPI, Value *GEPI,
                                bool IsFwd, PHINode **PHISWCO, PHINode **PHIIO,
                                Value **VOO, BasicBlock **BBO) -> bool {
    BasicBlock *BBX = nullptr;
    GetElementPtrInst *GEP = nullptr;
    if (!IsPvtInnerIncBlock(F, BBI, PHIOI, IsFwd, &GEP, &BBX))
      return false;
    Instruction *BP = BBI->getFirstNonPHI()->getPrevNonDebugInstruction();
    PHINode *PHI = dyn_cast_or_null<PHINode>(BP);
    if (!PHI || PHI->getNumIncomingValues() != 2)
      return false;
    if (PHI->getIncomingValue(0) != GEPI || PHI->getIncomingValue(1) != PHIII)
      return false;
    auto PHISWCX = dyn_cast_or_null<PHINode>(PHI->getPrevNonDebugInstruction());
    if (!PHISWCX || PHISWCX->getNumIncomingValues() != 2)
      return false;
    if (!match(PHISWCX->getIncomingValue(0), m_One()))
      return false;
    if (PHISWCX->getIncomingValue(1) != VSWPI)
      return false;
    *PHISWCO = PHISWCX;
    *PHIIO = PHI;
    *VOO = GEP;
    *BBO = BBX;
    return true;
  };

  //
  // Return 'true' if 'BBI' is a BasicBlock of the form:
  //
  //   'PHISWC1' = phi i32 [ 'PHISWC0', %for.cond95 ],
  //       [ %swap_cnt.2, %if.end120 ]
  //   'PHIPB1' = phi i8* [ 'PHIPB0', %for.cond95 ], [ %add.ptr121, %if.end120 ]
  //   'PHIPA1' = phi i8* [ 'PHIPA0', %for.cond95 ], [ %pa.2, %if.end120 ]
  //   %cmp96 = icmp ule i8* 'PHIPB1', 'PHIPC0'
  //   br i1 %cmp96, label 'BBL', label 'BBR'
  //
  // If we return 'true', we set the values of 'PHIPA1', 'PHIPB1', 'PHISWC1',
  // 'BBL', and 'BBR'.
  //
  // Note: The incoming values on the right branch of the PHINodes in this
  // BasicBlock are checked in the call to 'ArePivPHIsOK'.
  //
  auto IsFwdPivotTestBlock =
      [](BasicBlock *BBI, PHINode *PHIPA0, PHINode *PHIPB0, PHINode *PHIPC0,
         PHINode *PHISWC0, PHINode **PHIPA1, PHINode **PHIPB1,
         PHINode **PHISWC1, BasicBlock **BBL, BasicBlock **BBR) -> bool {
    BranchInst *BI = nullptr;
    ICmpInst *IC = nullptr;
    if (!getBIAndIC(BBI, ICmpInst::ICMP_ULE, &BI, &IC))
      return false;
    auto PHIPBX = dyn_cast<PHINode>(IC->getOperand(0));
    if (!PHIPBX || PHIPBX->getNumIncomingValues() != 2 ||
        PHIPBX->getIncomingValue(0) != PHIPB0)
      return false;
    if (IC->getOperand(1) != PHIPC0)
      return false;
    auto PHIPAX = dyn_cast_or_null<PHINode>(IC->getPrevNonDebugInstruction());
    if (!PHIPAX || PHIPAX->getNumIncomingValues() != 2 ||
        PHIPAX->getIncomingValue(0) != PHIPA0)
      return false;
    Instruction *BP = PHIPBX->getPrevNonDebugInstruction();
    auto PHISWCX = dyn_cast_or_null<PHINode>(BP);
    if (!PHISWCX || PHISWCX->getNumIncomingValues() != 2 ||
        PHISWCX->getIncomingValue(0) != PHISWC0)
      return false;
    *PHIPA1 = PHIPAX;
    *PHIPB1 = PHIPBX;
    *PHISWC1 = PHISWCX;
    *BBL = BI->getSuccessor(0);
    *BBR = BI->getSuccessor(1);
    return true;
  };

  //
  // Return 'true' if 'BBI' is a BasicBlock of the form:
  //
  //   %swap_cnt.1 = phi i32 [ %swap_cnt.0, %for.cond95 ],
  //       [ 'VSWCI', %if.end120 ]
  //   %pb.1 = phi i8* [ %pb.0, %for.cond95 ], [ 'VPBI', %if.end120 ]
  //   %pa.1 = phi i8* [ %pa.0, %for.cond95 ], [ 'VPAI', %if.end120 ]
  //   %cmp96 = icmp ule i8* %pb.1, %pc.0
  //   br i1 %cmp96, label %land.rhs98, label %while.end
  //
  // Note: This the same BasicBlock that we checked in 'IsFwdPivotTestBlock'
  // and 'IsBwdPivotTestBlock', but now we are checking the right inputs of
  // the PHINodes. We can use casts rather than dyn_casts because the
  // relative order of the Instructions in this BasicBlock were already
  // verified in 'IsFwdPivotTestBlock' and 'IsBwdPivotTestBlock'.
  //
  auto ArePivPHIsOK = [](BasicBlock *BBI, Value *VPAI, Value *VPBI,
                         Value *VSWCI) -> bool {
    auto BI = cast<BranchInst>(BBI->getTerminator());
    auto IC = cast<ICmpInst>(BI->getPrevNonDebugInstruction());
    auto PHIPA = cast<PHINode>(IC->getPrevNonDebugInstruction());
    if (PHIPA->getIncomingValue(1) != VPAI)
      return false;
    auto PHIPB = cast<PHINode>(PHIPA->getPrevNonDebugInstruction());
    if (PHIPB->getIncomingValue(1) != VPBI)
      return false;
    auto PHISWC = cast<PHINode>(PHIPB->getPrevNonDebugInstruction());
    if (PHISWC->getIncomingValue(1) != VSWCI)
      return false;
    return true;
  };

  //
  // Return 'true' if 'BBI' in 'F' matches the forward pivot loop in spec_qsort.
  // 'PHIA' is the base of the array. 'PHIPA0', 'PHIPB0', 'PHIPC0', and
  // 'PHISWCI' represent the values coming into the loop of 'pa', 'pb', 'pc',
  // and 'swap_cnt'. 'PHIC0' is 'true' if we are swapping values as longs.
  // 'PHIC1' is 'true' if we are swapping values as ints. If we are swapping
  // values as chars, 'FSwapFunc' is used to do the swapping. 'BytesInLong' is
  // the number of bytes in a long. If we return 'true', we set the values of
  // 'PHIPA1', 'PHIPB1', 'PHISWCO', and 'BBO', which are the values of 'pa',
  // 'pb', 'swap_cnt' and the exit BasicBlock out of the loop.
  //
  auto IsFwdPivotLoop =
      [&IsFwdPivotTestBlock, &IsPvtCallBlock, &IsPvtCallResBlock, &IsEasySwap,
       &IsPvtInnerIncBlock, IsPvtOuterIncBlock, ArePivPHIsOK](
          Function &F, Function &FSwapFunc, BasicBlock *BBI, PHINode *PHIA,
          PHINode *PHIPA0, PHINode *PHIPB0, PHINode *PHIPC0, PHINode *PHISWCI,
          PHINode *PHIC0, PHINode *PHIC1, uint64_t BytesInLong,
          PHINode **PHIPA1, PHINode **PHIPB1, PHINode **PHISWCO,
          BasicBlock **BBO) -> bool {
    BasicBlock *BBX = nullptr;
    BasicBlock *BBXT = nullptr;
    BasicBlock *BBCB = nullptr;
    BasicBlock *BBCRB = nullptr;
    BasicBlock *BBSWP = nullptr;
    BasicBlock *BBII = nullptr;
    BasicBlock *BBOI = nullptr;
    PHINode *PHIPAX = nullptr;
    PHINode *PHIPA2 = nullptr;
    PHINode *PHIPBX = nullptr;
    PHINode *PHISWC1 = nullptr;
    PHINode *PHISWC2 = nullptr;
    Value *CI = nullptr;
    Value *VPBN = nullptr;
    GetElementPtrInst *GEP = nullptr;
    if (!IsFwdPivotTestBlock(BBI, PHIPA0, PHIPB0, PHIPC0, PHISWCI, &PHIPAX,
                             &PHIPBX, &PHISWC1, &BBCB, &BBX))
      return false;
    if (!IsPvtCallBlock(F, BBCB, PHIPBX, PHIA, ICmpInst::ICMP_SLE, &CI, &BBCRB,
                        &BBXT) ||
        BBXT != BBX)
      return false;
    if (!IsPvtCallResBlock(BBCRB, CI, &BBSWP, &BBOI))
      return false;
    if (!IsEasySwap(F, FSwapFunc, BBSWP, PHIPAX, PHIPBX, PHIC0, PHIC1,
                    BytesInLong, &BBII))
      return false;
    if (!IsPvtInnerIncBlock(F, BBII, PHIPAX, true, &GEP, &BBOI))
      return false;
    if (!IsPvtOuterIncBlock(F, BBOI, PHIPAX, PHIPBX, PHISWC1, GEP, true,
                            &PHISWC2, &PHIPA2, &VPBN, &BBXT) ||
        BBXT != BBI)
      return false;
    if (!ArePivPHIsOK(BBI, PHIPA2, VPBN, PHISWC2))
      return false;
    *PHIPA1 = PHIPAX;
    *PHIPB1 = PHIPBX;
    *PHISWCO = PHISWC1;
    *BBO = BBX;
    return true;
  };

  //
  // Return 'true' if 'BBI' is a BasicBlock of the form:
  //
  //
  //   'PHISWC3' = phi i32 [ 'PHISWC1', %while.end ],
  //       [ %swap_cnt.4, %if.end149 ]
  //   'PHIPD1' = phi i8* [ 'PHIPD0', %while.end ], [ %pd.2, %if.end149 ]
  //   'PHIPC1' = phi i8* [ 'PHIPC0', %while.end ], [ %add.ptr151, %if.end149 ]
  //   %cmp123 = icmp ule i8* 'PHIPB1', 'PHIPC1'
  //   br i1 %cmp123, label 'BBL', label 'BBR'
  //
  // If we return 'true', we set the values of 'PHIPC1', 'PHIPD1', 'PHISWC3',
  // 'BBL', 'BBR'.
  //
  // Note: The incoming values on the right branch of the PHINodes in this
  // BasicBlock are checked in the call to 'ArePivPHIsOK'.
  //
  auto IsBwdPivotTestBlock =
      [](BasicBlock *BBI, PHINode *PHIPB1, PHINode *PHIPC0, PHINode *PHIPD0,
         PHINode *PHISWC1, PHINode **PHIPC1, PHINode **PHIPD1,
         PHINode **PHISWC3, BasicBlock **BBL, BasicBlock **BBR) -> bool {
    BranchInst *BI = nullptr;
    ICmpInst *IC = nullptr;
    if (!getBIAndIC(BBI, ICmpInst::ICMP_ULE, &BI, &IC))
      return false;
    if (IC->getOperand(0) != PHIPB1)
      return false;
    auto PHIPCX = dyn_cast<PHINode>(IC->getOperand(1));
    if (!PHIPCX || PHIPCX->getNumIncomingValues() != 2 ||
        PHIPCX->getIncomingValue(0) != PHIPC0)
      return false;
    Instruction *BP0 = PHIPCX->getPrevNonDebugInstruction();
    auto PHIPDX = dyn_cast_or_null<PHINode>(BP0);
    if (!PHIPDX || PHIPDX->getNumIncomingValues() != 2 ||
        PHIPDX->getIncomingValue(0) != PHIPD0)
      return false;
    Instruction *BP1 = PHIPDX->getPrevNonDebugInstruction();
    auto PHISWCX = dyn_cast_or_null<PHINode>(BP1);
    if (!PHISWCX || PHISWCX->getNumIncomingValues() != 2 ||
        PHISWCX->getIncomingValue(0) != PHISWC1)
      return false;
    *PHIPC1 = PHIPCX;
    *PHIPD1 = PHIPDX;
    *PHISWC3 = PHISWCX;
    *BBL = BI->getSuccessor(0);
    *BBR = BI->getSuccessor(1);
    return true;
  };

  //
  // Return 'true' if 'BBI' in 'F' matches the backward pivot loop in
  // spec_qsort. 'PHIA' is the base of the array. 'PHIPB1', 'PHIPC0', 'PHIPD0',
  // and 'PHISWCI' represent the values coming into the loop of 'pb', 'pc',
  // 'pd', and 'swap_cnt'. 'PHIC0' is 'true' if we are swapping values as
  // longs. 'PHIC1' is 'true' if we are swapping values as ints. If we are
  // swapping values as chars, 'FSwapFunc' is used to do the swapping.
  // 'BytesInLong' is the number of bytes in a long. If we return 'true', we set
  // the values of 'PHIPC1', 'PHIPD1', 'PHISWCO', and 'BBO', which are the
  // values of 'pc', 'pd', 'swap_cnt' and the exit BasicBlock out of the loop.
  //
  auto IsBwdPivotLoop =
      [&IsBwdPivotTestBlock, &IsPvtCallBlock, &IsPvtCallResBlock, &IsEasySwap,
       &IsPvtInnerIncBlock, IsPvtOuterIncBlock, ArePivPHIsOK](
          Function &F, Function &FSwapFunc, BasicBlock *BBI, PHINode *PHIA,
          PHINode *PHIPB1, PHINode *PHIPC0, PHINode *PHIPD0, PHINode *PHISWCI,
          PHINode *PHIC0, PHINode *PHIC1, uint64_t BytesInLong,
          PHINode **PHIPC1, PHINode **PHIPD1, PHINode **PHISWCO,
          BasicBlock **BBO) -> bool {
    BasicBlock *BBX = nullptr;
    BasicBlock *BBXT = nullptr;
    BasicBlock *BBCB = nullptr;
    BasicBlock *BBCRB = nullptr;
    BasicBlock *BBSWP = nullptr;
    BasicBlock *BBII = nullptr;
    BasicBlock *BBOI = nullptr;
    PHINode *PHIPCX = nullptr;
    PHINode *PHIPDX = nullptr;
    PHINode *PHIPD2 = nullptr;
    PHINode *PHISWC3 = nullptr;
    PHINode *PHISWC4 = nullptr;
    Value *CI = nullptr;
    Value *VPCN = nullptr;
    GetElementPtrInst *GEP = nullptr;
    if (!IsBwdPivotTestBlock(BBI, PHIPB1, PHIPC0, PHIPD0, PHISWCI, &PHIPCX,
                             &PHIPDX, &PHISWC3, &BBCB, &BBX))
      return false;
    if (!IsPvtCallBlock(F, BBCB, PHIPCX, PHIA, ICmpInst::ICMP_SGE, &CI, &BBCRB,
                        &BBXT) ||
        BBXT != BBX)
      return false;
    if (!IsPvtCallResBlock(BBCRB, CI, &BBSWP, &BBOI))
      return false;
    if (!IsEasySwap(F, FSwapFunc, BBSWP, PHIPCX, PHIPDX, PHIC0, PHIC1,
                    BytesInLong, &BBII))
      return false;
    if (!IsPvtInnerIncBlock(F, BBII, PHIPDX, false, &GEP, &BBOI))
      return false;
    if (!IsPvtOuterIncBlock(F, BBOI, PHIPDX, PHIPCX, PHISWC3, GEP, false,
                            &PHISWC4, &PHIPD2, &VPCN, &BBXT) ||
        BBXT != BBI)
      return false;
    if (!ArePivPHIsOK(BBI, VPCN, PHIPD2, PHISWC4))
      return false;
    *PHIPC1 = PHIPCX;
    *PHIPD1 = PHIPDX;
    *PHISWCO = PHISWC3;
    *BBO = BBX;
    return true;
  };

  //
  // Return 'true' if 'BBI' is a BasicBlock of the form:
  //
  //   %cmp153 = icmp ugt i8* 'PHIPB1', 'PHIPC1'
  //   br i1 %cmp153, label 'BBL', label 'BBR'
  //
  // If we return 'true', set the values of 'BBL' and 'BBR'.
  //
  auto IsMainExitBlock = [](BasicBlock *BBI, PHINode *PHIPB1, PHINode *PHIPC1,
                            BasicBlock **BBL, BasicBlock **BBR) -> bool {
    BranchInst *BI = nullptr;
    ICmpInst *IC = nullptr;
    if (!getBIAndIC(BBI, ICmpInst::ICMP_UGT, &BI, &IC))
      return false;
    if (IC->getOperand(0) != PHIPB1 || IC->getOperand(1) != PHIPC1)
      return false;
    *BBL = BI->getSuccessor(0);
    *BBR = BI->getSuccessor(1);
    return true;
  };

  //
  // Return 'true' if 'BBI' in 'F' is a BasicBlock of the form:
  //
  //   'VPBN' = getelementptr inbounds i8, i8* 'PHIB1', i64 'F(2)'
  //   %idx.neg171 = sub i64 0, 'F(2)'
  //   'VPCN'  = getelementptr inbounds i8, i8* 'PHIC1', i64 %idx.neg171
  //   br label 'BBO'
  //
  // If we return 'true', we set the values of 'VPBN' and 'VPCN'.
  //
  auto IsMainIncBlock =
      [&GetBFGEP, &GetPrevBFGEP](Function &F, BasicBlock *BBI, BasicBlock *BBO,
                                 PHINode *PHIPB1, PHINode *PHIPC1, Value **VPBN,
                                 Value **VPCN) -> bool {
    auto BI = dyn_cast<BranchInst>(BBI->getTerminator());
    if (!BI || BI->isConditional() || BI->getSuccessor(0) != BBO)
      return false;
    Instruction *BP = BI->getPrevNonDebugInstruction();
    GetElementPtrInst *GEP1 = GetBFGEP(BP, PHIPC1);
    if (!GEP1 ||
        !match(GEP1->getOperand(1), m_Sub(m_Zero(), m_Specific(F.getArg(2)))))
      return false;
    GetElementPtrInst *GEP0 = GetPrevBFGEP(GEP1, PHIPB1);
    if (!GEP0 || GEP0->getOperand(1) != F.getArg(2))
      return false;
    *VPBN = GEP0;
    *VPCN = GEP1;
    return true;
  };

  //
  // Return 'true' if 'BBI' is a BasicBlock of the form:
  //
  //   %swap_cnt.0 = phi i32 [ 0, %if.end90 ], [ 1, %if.end169 ]
  //   %pd.0 = phi i8* [ %add.ptr94, %if.end90 ], [ 'VPDI', %if.end169 ]
  //   %pc.0 = phi i8* [ %add.ptr94, %if.end90 ], [ 'VPCI', %if.end169 ]
  //   %pb.0 = phi i8* [ %add.ptr91, %if.end90 ], [ 'VPBI', %if.end169 ]
  //   %pa.0 = phi i8* [ %add.ptr91, %if.end90 ], [ 'VPAI', %if.end169 ]
  //   br label %while.cond
  //
  // Note: This the same BasicBlock that we checked in 'IsMainHeadBlock'
  // above, but now we are checking the right inputs of the PHINodes.
  // We can use casts rather than dyn_casts because the relative order of
  // the Instructions in this BasicBlock were already verified in
  // 'IsMainHeadBlock'.
  //
  auto AreMainPHIsOK = [](BasicBlock *BBI, Value *VPAI, Value *VPBI,
                          Value *VPCI, Value *VPDI) -> bool {
    Instruction *BP = BBI->getTerminator()->getPrevNonDebugInstruction();
    auto PHIA = cast<PHINode>(BP);
    if (PHIA->getIncomingValue(1) != VPAI)
      return false;
    auto PHIB = cast<PHINode>(PHIA->getPrevNonDebugInstruction());
    if (PHIB->getIncomingValue(1) != VPBI)
      return false;
    auto PHIC = cast<PHINode>(PHIB->getPrevNonDebugInstruction());
    if (PHIC->getIncomingValue(1) != VPCI)
      return false;
    auto PHID = cast<PHINode>(PHIC->getPrevNonDebugInstruction());
    if (PHID->getIncomingValue(1) != VPDI)
      return false;
    return true;
  };

  //
  // Return 'true' if 'BBI' in 'F' starts the main while loop in spec_qsort.
  // 'PHIA' is the beginning of the swapping array. 'PHIN' is its length.
  // 'PHIC0' indicates whether we are swapping longs. 'PHIC1' indicates
  // whether we are swapping ints. If neither, 'FSwapFunc' is used to do the
  // swapping. 'BytesInLong' is the number of bytes in a long. If we return
  // 'true', we set 'PHIPA1', 'PHIPB1', 'PHIPC1', 'PHIPD1', and 'PHISWC3' to
  // the values of 'pa', 'pb', 'pc', 'pd', and 'swap_cnt' out of the loop, and
  // 'BBO' to the exit BasicBlock of the main while loop.
  //
  auto IsMainWhileLoop =
      [&IsMainInitBlock, &IsMainHeadBlock, &IsFwdPivotLoop, &IsBwdPivotLoop,
       &IsMainExitBlock, &IsEasySwap, &IsMainIncBlock, &AreMainPHIsOK](
          Function &F, Function &FSwapFunc, BasicBlock *BBI, PHINode *PHIA,
          PHINode *PHIN, PHINode *PHIC0, PHINode *PHIC1, uint64_t BytesInLong,
          PHINode **PHIPA1, PHINode **PHIPB1, PHINode **PHIPC1,
          PHINode **PHIPD1, PHINode **PHISWC3, BasicBlock **BBO) -> bool {
    Value *VPAI = nullptr;
    Value *VPBN = nullptr;
    Value *VPCI = nullptr;
    Value *VPCN = nullptr;
    PHINode *PHIPA0 = nullptr;
    PHINode *PHIPAX = nullptr;
    PHINode *PHIPB0 = nullptr;
    PHINode *PHIPBX = nullptr;
    PHINode *PHIPC0 = nullptr;
    PHINode *PHIPCX = nullptr;
    PHINode *PHIPD0 = nullptr;
    PHINode *PHIPDX = nullptr;
    PHINode *PHISWC0 = nullptr;
    PHINode *PHISWC1 = nullptr;
    PHINode *PHISWCX = nullptr;
    BasicBlock *BBL = nullptr;
    BasicBlock *BBX = nullptr;
    BasicBlock *BBFPV = nullptr;
    BasicBlock *BBFPVX = nullptr;
    BasicBlock *BBBPV = nullptr;
    BasicBlock *BBBPVX = nullptr;
    BasicBlock *BBSWP = nullptr;
    BasicBlock *BBSWX = nullptr;
    if (!IsMainInitBlock(F, BBI, PHIA, PHIN, &VPAI, &VPCI, &BBL))
      return false;
    if (!IsMainHeadBlock(BBL, VPAI, VPCI, &PHIPA0, &PHIPB0, &PHIPC0, &PHIPD0,
                         &PHISWC0, &BBFPV))
      return false;
    if (!IsFwdPivotLoop(F, FSwapFunc, BBFPV, PHIA, PHIPA0, PHIPB0, PHIPC0,
                        PHISWC0, PHIC0, PHIC1, BytesInLong, &PHIPAX, &PHIPBX,
                        &PHISWC1, &BBFPVX)) {
      DEBUG_WITH_TYPE(FXNREC_VERBOSE,
                      dbgs() << "FXNREC: SPEC_QSORT: " << F.getName()
                             << ": Is not forward pivot loop.\n");
      return false;
    }
    if (!isDirectBranchBlock(BBFPVX, &BBBPV))
      return false;
    if (!IsBwdPivotLoop(F, FSwapFunc, BBBPV, PHIA, PHIPBX, PHIPC0, PHIPD0,
                        PHISWC1, PHIC0, PHIC1, BytesInLong, &PHIPCX, &PHIPDX,
                        &PHISWCX, &BBBPVX)) {
      DEBUG_WITH_TYPE(FXNREC_VERBOSE,
                      dbgs() << "FXNREC: SPEC_QSORT: " << F.getName()
                             << ": Is not backward pivot loop.\n");
      return false;
    }
    if (!IsMainExitBlock(BBBPVX, PHIPBX, PHIPCX, &BBX, &BBSWP))
      return false;
    if (!IsEasySwap(F, FSwapFunc, BBSWP, PHIPBX, PHIPCX, PHIC0, PHIC1,
                    BytesInLong, &BBSWX))
      return false;
    if (!IsMainIncBlock(F, BBSWX, BBL, PHIPBX, PHIPCX, &VPBN, &VPCN))
      return false;
    if (!AreMainPHIsOK(BBL, PHIPAX, VPBN, VPCN, PHIPDX))
      return false;
    *PHIPA1 = PHIPAX;
    *PHIPB1 = PHIPBX;
    *PHIPC1 = PHIPCX;
    *PHIPD1 = PHIPDX;
    *PHISWC3 = PHISWCX;
    *BBO = BBX;
    return true;
  };

  //
  // Return 'true' if 'VI' represents the expression:
  //   SV[0] - SV[1] - ... - SV[N-1] where N = SV.size()
  // If for any I, SV[I] is a pointer, it may be enclosed in a PtrToIntInst.
  //
  auto IsTestSubSeq = [](Value *VI, SmValVecImpl &SV) -> bool {
    auto BO = dyn_cast_or_null<BinaryOperator>(VI);
    for (unsigned I = SV.size() - 1; I > 0; --I) {
      if (!BO || BO->getOpcode() != Instruction::Sub)
        return false;
      Value *VT = BO->getOperand(1);
      auto PI = dyn_cast<PtrToIntInst>(VT);
      if (PI)
        VT = PI->getPointerOperand();
      if (VT != SV[I])
        return false;
      if (I == 1)
        break;
      auto BOO = dyn_cast<BinaryOperator>(BO->getOperand(0));
      if (!BOO)
        return false;
      BO = BOO;
    }
    Value *VT = BO->getOperand(0);
    auto PI = dyn_cast<PtrToIntInst>(VT);
    if (PI)
      VT = PI->getPointerOperand();
    return VT == SV[0];
  };

  //
  // Return 'true' if the last Instruction 'VO' above the terminator of 'BBI'
  // implements the expression 'SV[0] - ... - SV[N-1]'. If we return 'true',
  // set 'VO' and set 'BBO' to the unconditional exit of 'BBI'.
  //
  auto IsMinSide = [&IsTestSubSeq](BasicBlock *BBI, SmValVecImpl &SV,
                                   Value **VO, BasicBlock **BBO) -> bool {
    auto *BI = dyn_cast<BranchInst>(BBI->getTerminator());
    if (!BI || BI->isConditional())
      return false;
    Instruction *BP = BI->getPrevNonDebugInstruction();
    if (!IsTestSubSeq(BP, SV))
      return false;
    *VO = BP;
    *BBO = BI->getSuccessor(0);
    return true;
  };

  //
  // Return 'true' if 'BBI' represents a MIN operation of the form:
  //    'VROL' = SVL[0] - ... - SVL[N-1]
  //    'VROR' = SVR[0] - ... - SVR[N-1]
  //    if ('VROL' < 'VROR') then
  //      'R' = 'VROL'
  //    else
  //      'R = 'VROR'
  //    endif
  // If we return 'true', set the values of 'VROL' and 'VROR' and set 'BBO'
  // to the exit BasicBlock of this sequence.
  //
  auto IsMinToR = [&IsTestSubSeq, &IsMinSide](
                      BasicBlock *BBI, SmValVecImpl &SVL, SmValVecImpl &SVR,
                      Value **VROL, Value **VROR, BasicBlock **BBO) -> bool {
    BranchInst *BI = nullptr;
    ICmpInst *IC = nullptr;
    Value *VROLX = nullptr;
    Value *VRORX = nullptr;
    BasicBlock *BBLX = nullptr;
    BasicBlock *BBRX = nullptr;
    if (!getBIAndIC(BBI, ICmpInst::ICMP_SLT, &BI, &IC))
      return false;
    if (!IsTestSubSeq(IC->getOperand(0), SVL))
      return false;
    if (!IsTestSubSeq(IC->getOperand(1), SVR))
      return false;
    if (!IsMinSide(BI->getSuccessor(0), SVL, &VROLX, &BBLX))
      return false;
    if (!IsMinSide(BI->getSuccessor(1), SVR, &VRORX, &BBRX))
      return false;
    if (BBLX != BBRX)
      return false;
    *VROL = VROLX;
    *VROR = VRORX;
    *BBO = BBLX;
    return true;
  };

  //
  // Return 'true' if 'BBI' is the first BasicBlock in a set of three which
  // implements:
  //
  //   %sub.ptr.lhs.cast228 = ptrtoint i8* 'PHIPA1' to i64
  //   %sub.ptr.rhs.cast = ptrtoint i8* 'PHIPA' to i64
  //   %sub.ptr.sub229 = sub i64 %sub.ptr.lhs.cast228, %sub.ptr.rhs.cast
  //   %sub.ptr.lhs.cast230 = ptrtoint i8* 'PHIPB1' to i64
  //   %sub.ptr.rhs.cast231 = ptrtoint i8* 'PHIPA1'  to i64
  //   %sub.ptr.sub232 = sub i64 %sub.ptr.lhs.cast230, %sub.ptr.rhs.cast231
  //   %cmp233 = icmp slt i64 %sub.ptr.sub229, %sub.ptr.sub232
  //   br i1 %cmp233, label %cond.true235, label %cond.false239
  //
  // cond.true235:
  //   %sub.ptr.lhs.cast236 = ptrtoint i8* 'PHIPA1' to i64
  //   %sub.ptr.rhs.cast237 = ptrtoint i8* 'PHIPA' to i64
  //   'VROL'= sub i64 %sub.ptr.lhs.cast236, %sub.ptr.rhs.cast237
  //   br label 'BBO'
  //
  // cond.false239:
  //   %sub.ptr.lhs.cast240 = ptrtoint i8* 'PHIPB1' to i64
  //   %sub.ptr.rhs.cast241 = ptrtoint i8* 'PHIPA1' to i64
  //   'VROR' = sub i64 %sub.ptr.lhs.cast240, %sub.ptr.rhs.cast241
  //   br label 'BBO'
  //
  // If we return 'true', set the values of 'VROL', 'VROR', and 'BBO'.
  //
  auto IsMinToR0 = [&IsMinToR](BasicBlock *BBI, PHINode *PHIA, PHINode *PHIPA1,
                               PHINode *PHIPB1, Value **VROL, Value **VROR,
                               BasicBlock **BBO) -> bool {
    SmValVec SVL;
    SmValVec SVR;
    SVL.push_back(PHIPA1);
    SVL.push_back(PHIA);
    SVR.push_back(PHIPB1);
    SVR.push_back(PHIPA1);
    Value *VROLX = nullptr;
    Value *VRORX = nullptr;
    BasicBlock *BBOX = nullptr;
    if (!IsMinToR(BBI, SVL, SVR, &VROLX, &VRORX, &BBOX))
      return false;
    *VROL = VROLX;
    *VROR = VRORX;
    *BBO = BBOX;
    return true;
  };

  //
  // If the last PHINode in 'BBI' in 'F' represents the expression:
  //     %mul226 = mul i64 'PHIN', 'F(2)'
  //     'VANES' = getelementptr inbounds i8, i8* 'PHIA', i64 %mul226
  // then return that Value. (Note: VANES stands for "Value of A+N*ES".)
  //
  auto GetVANES = [&GetPrevBFGEP](Function &F, BasicBlock *BBI, PHINode *PHIA,
                                  PHINode *PHIN) -> Value * {
    auto GEP = GetPrevBFGEP(BBI->getTerminator(), PHIA);
    if (!GEP || !match(GEP->getOperand(1),
                       m_Mul(m_Specific(PHIN), m_Specific(F.getArg(2)))))
      return nullptr;
    return GEP;
  };

  //
  // Return 'true' if 'BBI' is the first BasicBlock in a set of three which
  // implements:
  //
  //   %sub.ptr.lhs.cast252 = ptrtoint i8* 'PHID1' to i64
  //   %sub.ptr.rhs.cast253 = ptrtoint i8* 'PHIC1' to i64
  //   %sub.ptr.sub254 = sub i64 %sub.ptr.lhs.cast252, %sub.ptr.rhs.cast253
  //   %sub.ptr.lhs.cast255 = ptrtoint i8* 'VANES' to i64
  //   %sub.ptr.rhs.cast256 = ptrtoint i8* 'PHIPD1'to i64
  //   %sub.ptr.sub257 = sub i64 %sub.ptr.lhs.cast255, %sub.ptr.rhs.cast256
  //   %sub258 = sub nsw i64 %sub.ptr.sub257, 'F(2)'
  //   %cmp259 = icmp slt i64 %sub.ptr.sub254, %sub258
  //   br i1 %cmp259, label %cond.true261, label %cond.false265
  //
  // cond.true261:
  //   %sub.ptr.lhs.cast262 = ptrtoint i8* 'PHIPD1' to i64
  //   %sub.ptr.rhs.cast263 = ptrtoint i8* 'PHIPC1' to i64
  //   'VROL' = sub i64 %sub.ptr.lhs.cast262, %sub.ptr.rhs.cast263
  //   br label 'BBO'
  //
  // cond.false265:
  //   %sub.ptr.lhs.cast266 = ptrtoint i8* 'VANES' to i64
  //   %sub.ptr.rhs.cast267 = ptrtoint i8* 'PHIPD1' to i64
  //   %sub.ptr.sub268 = sub i64 %sub.ptr.lhs.cast266, %sub.ptr.rhs.cast267
  //   'VROR' = sub nsw i64 %sub.ptr.sub268, 'F(2)'
  //   br label 'BBO'
  //
  // If we return 'true', set the values of 'VROL', 'VROR', 'VANES', and
  // 'BBO'.
  //
  auto IsMinToR1 = [&IsMinToR,
                    &GetVANES](Function &F, BasicBlock *BBI, BasicBlock *BBIP,
                               PHINode *PHIA, PHINode *PHIN, PHINode *PHIPC1,
                               PHINode *PHIPD1, Value **VROL, Value **VROR,
                               Value **VANES, BasicBlock **BBO) -> bool {
    SmValVec SVL;
    SmValVec SVR;
    Value *VANESX = GetVANES(F, BBIP, PHIA, PHIN);
    if (!VANESX)
      return false;
    SVL.push_back(PHIPD1);
    SVL.push_back(PHIPC1);
    SVR.push_back(VANESX);
    SVR.push_back(PHIPD1);
    SVR.push_back(F.getArg(2));
    Value *VROLX = nullptr;
    Value *VRORX = nullptr;
    BasicBlock *BBOX = nullptr;
    if (!IsMinToR(BBI, SVL, SVR, &VROLX, &VRORX, &BBOX))
      return false;
    *VANES = VANESX;
    *VROL = VROLX;
    *VROR = VRORX;
    *BBO = BBOX;
    return true;
  };

  //
  // Return 'true' if 'BBI' is the start of a pair of BasicBlocks that
  // implements the sequence:
  //
  //   %cond271 = phi i64 [ 'VROL', %cond.true261 ], [ 'VROR', %cond.false265 ]
  //   %cmp272 = icmp ugt i64 %cond271, 0
  //   br i1 %cmp272, label %if.then274, label 'BBO'
  //
  // if.then274:
  //   %idx.neg275 = sub i64 0, %cond271
  //   %add.ptr276 = getelementptr inbounds i8, i8* 'VR', i64 %idx.neg275
  //   %conv277 = trunc i64 %cond271 to i32
  //   call void 'FSwapFunc'(i8* 'VL', i8* %add.ptr276, i32 %conv277,
  //        i32 'PHIC0', i32 'PHIC1')
  //   br label 'BBO'
  //
  // If we return 'true', set the value of 'BBO'.
  //
  auto IsVecSwap = [&GetBFGEPO](Function &FSwapFunc, BasicBlock *BBI,
                                Value *VROL, Value *VROR, Value *VL, Value *VR,
                                PHINode *PHIC0, PHINode *PHIC1,
                                BasicBlock **BBO) -> bool {
    BranchInst *BI = nullptr;
    ICmpInst *IC = nullptr;
    if (!getBIAndIC(BBI, ICmpInst::ICMP_UGT, &BI, &IC))
      return false;
    if (!match(IC->getOperand(1), m_Zero()))
      return false;
    auto PHI = dyn_cast<PHINode>(IC->getOperand(0));
    if (!PHI || PHI->getNumIncomingValues() != 2 ||
        PHI->getIncomingValue(0) != VROL || PHI->getIncomingValue(1) != VROR)
      return false;
    BasicBlock *BBL = BI->getSuccessor(0);
    BasicBlock *BBR = BI->getSuccessor(1);
    auto BIL = dyn_cast<BranchInst>(BBL->getTerminator());
    if (!BIL || BIL->isConditional() || BIL->getSuccessor(0) != BBR)
      return false;
    auto CI = dyn_cast_or_null<CallInst>(BIL->getPrevNonDebugInstruction());
    if (!CI || CI->isIndirectCall() || CI->getCalledFunction() != &FSwapFunc)
      return false;
    if (CI->getArgOperand(0) != VL)
      return false;
    Value *VSUB = GetBFGEPO(CI->getArgOperand(1), VR);
    auto BO = dyn_cast_or_null<BinaryOperator>(VSUB);
    if (!BO || BO->getOpcode() != Instruction::Sub)
      return false;
    if (!match(BO->getOperand(0), m_Zero()) || (BO->getOperand(1) != PHI))
      return false;
    if (!match(CI->getArgOperand(2), m_Trunc(m_Specific(PHI))))
      return false;
    if (CI->getArgOperand(3) != PHIC0 || CI->getArgOperand(4) != PHIC1)
      return false;
    *BBO = BBR;
    return true;
  };

  //
  // Return 'true' if 'BBI' in 'F' is a BasicBlock of the form:
  //
  //   %sub.ptr.lhs.cast279 = ptrtoint i8* 'PHIPL' to i64
  //   %sub.ptr.rhs.cast280 = ptrtoint i8* 'PHIPR' to i64
  //   'VO' = sub i64 %sub.ptr.lhs.cast279, %sub.ptr.rhs.cast280
  //   %cmp282 = icmp ugt i64 'VO', 'F(2)'
  //   br i1 %cmp282, label 'BBL', label 'BBR'
  //
  // If we return 'true', set the values of 'VO', 'BBL', and 'BBR'.
  //
  auto IsTwoPtrCmpBlock = [](Function &F, BasicBlock *BBI, PHINode *PHIPL,
                             PHINode *PHIPR, Value **VO, BasicBlock **BBL,
                             BasicBlock **BBR) -> bool {
    BranchInst *BI = nullptr;
    ICmpInst *IC = nullptr;
    if (!getBIAndIC(BBI, ICmpInst::ICMP_UGT, &BI, &IC))
      return false;
    if (!match(IC->getOperand(1), m_Specific(F.getArg(2))))
      return false;
    if (!match(IC->getOperand(0), m_Sub(m_PtrToInt(m_Specific(PHIPL)),
                                        m_PtrToInt(m_Specific(PHIPR)))))
      return false;
    *VO = IC->getOperand(0);
    *BBL = BI->getSuccessor(0);
    *BBR = BI->getSuccessor(1);
    return true;
  };

  //
  // Return 'true' if 'BBI' in 'F' is a BasicBlock of the form:
  //
  //   %div285 = udiv i64 'VI', 'F(2)'
  //   call void @spec_qsort(i8* 'PHIA', i64 %div285, i64 'F(2)',
  //       i32 (i8*, i8*)* 'F(3)')
  //   br label 'BBO'
  //
  auto IsRecCall0Block = [](Function &F, BasicBlock *BBI, BasicBlock *BBO,
                            Value *VI, PHINode *PHIA) -> bool {
    auto BI = dyn_cast<BranchInst>(BBI->getTerminator());
    if (!BI || BI->isConditional() || BI->getSuccessor(0) != BBO)
      return false;
    auto CI = dyn_cast_or_null<CallInst>(BI->getPrevNonDebugInstruction());
    if (!CI || CI->isIndirectCall() || CI->getCalledFunction() != &F)
      return false;
    if (CI->getArgOperand(0) != PHIA)
      return false;
    if (!match(CI->getArgOperand(1),
               m_UDiv(m_Specific(VI), m_Specific(F.getArg(2)))))
      return false;
    if (CI->getArgOperand(2) != F.getArg(2))
      return false;
    if (CI->getArgOperand(3) != F.getArg(3))
      return false;
    return true;
  };

  //
  // Return 'true' if 'BBI' in 'F' is a BasicBlock of the form:
  //
  //   %idx.neg293 = sub i64 0, 'VI'
  //   'VPHIA0' = getelementptr inbounds i8, i8* 'VANES', i64 %idx.neg293
  //   'VPHIN0' = udiv i64 'VI', 'F(2)'
  //   br label 'BBO'
  //
  // If we return 'true', set the values of 'VPHIA0' and 'VPHIN0'.
  //
  auto IsRecCall1Block = [&GetBFGEP](Function &F, BasicBlock *BBI,
                                     BasicBlock *BBO, Value *VI, Value *VANES,
                                     Value **VPHIAO, Value **VPHINO) -> bool {
    auto BI = dyn_cast<BranchInst>(BBI->getTerminator());
    if (!BI || BI->isConditional() || BI->getSuccessor(0) != BBO)
      return false;
    Instruction *BP = BI->getPrevNonDebugInstruction();
    auto BD = dyn_cast_or_null<UDivOperator>(BP);
    if (!BD || BD->getOperand(0) != VI || BD->getOperand(1) != F.getArg(2))
      return false;
    GetElementPtrInst *GEP = GetBFGEP(BP->getPrevNonDebugInstruction(), VANES);
    if (!GEP || !match(GEP->getOperand(1), m_Sub(m_Zero(), m_Specific(VI))))
      return false;
    *VPHIAO = GEP;
    *VPHINO = BD;
    return true;
  };

  //
  // Return 'true' if the right-hand side values of the PHINodes 'PHIA'
  // and 'PHIN' are 'VPHIAI' and 'VPHINI', respectively.
  //
  auto AreKeyLoopPHIsOK = [](PHINode *PHIA, PHINode *PHIN, Value *VPHIAI,
                             Value *VPHINI) -> bool {
    return PHIA->getIncomingValue(1) == VPHIAI &&
           PHIN->getIncomingValue(1) == VPHINI;
  };

  //
  // Return 'true' if some use of arg #3 of 'F' is not:
  //  (1) Used in a recursive call to 'F'.
  //  (2) Arg #3 of 'FMed3'
  //  (3) Used as the called operand in any indirect call.
  //
  auto BadCmpArgUse = [](Function &F, Function &FMed3) -> bool {
    Argument *CU = F.getArg(3);
    for (User *U : CU->users()) {
      auto CB = dyn_cast<CallBase>(U);
      if (!CB)
        return false;
      if (Function *Callee = CB->getCalledFunction()) {
        if (Callee != &F && (Callee != &FMed3 || CB->getArgOperand(3) != CU))
          return false;
      } else {
        if (CB->getCalledOperand() != CU)
          return false;
      }
    }
    return true;
  };

  BasicBlock *BBE = &F.getEntryBlock();
  BasicBlock *BBL = nullptr;
  BasicBlock *BBX0 = nullptr;
  BasicBlock *BBX1 = nullptr;
  BasicBlock *BBX2 = nullptr;
  BasicBlock *BBX3 = nullptr;
  BasicBlock *BBX4 = nullptr;
  BasicBlock *BBX5 = nullptr;
  BasicBlock *BBXN = nullptr;
  BasicBlock *BBXNT = nullptr;
  BasicBlock *BBIS0 = nullptr;
  BasicBlock *BBIS1 = nullptr;
  BasicBlock *BBSW = nullptr;
  BasicBlock *BBPV = nullptr;
  BasicBlock *BBSWT = nullptr;
  BasicBlock *BBMIN0 = nullptr;
  BasicBlock *BBMIN1 = nullptr;
  BasicBlock *BBVSW0 = nullptr;
  BasicBlock *BBVSW1 = nullptr;
  BasicBlock *BBTPCB0 = nullptr;
  BasicBlock *BBTPCB1 = nullptr;
  BasicBlock *BBRC0 = nullptr;
  BasicBlock *BBRC1 = nullptr;
  PHINode *PHIA = nullptr;
  PHINode *PHIN = nullptr;
  PHINode *PHIC0 = nullptr;
  PHINode *PHIC1 = nullptr;
  PHINode *PHIPA1 = nullptr;
  PHINode *PHIPB1 = nullptr;
  PHINode *PHIPC1 = nullptr;
  PHINode *PHIPD1 = nullptr;
  PHINode *PHISWCX = nullptr;
  Value *VPMFI = nullptr;
  Value *VPMFO = nullptr;
  Value *VROL0 = nullptr;
  Value *VROR0 = nullptr;
  Value *VROL1 = nullptr;
  Value *VROR1 = nullptr;
  Value *VANES = nullptr;
  Value *VTP0 = nullptr;
  Value *VTP1 = nullptr;
  Value *VPHIAO = nullptr;
  Value *VPHINO = nullptr;
  Function *FSwapFuncX = nullptr;
  Function *FSwapFuncXT = nullptr;
  Function *FMed3X = nullptr;
  DenseMapBBToV PHIMap;

  // This is the main code for isQsortSpecQsort().
  if (!MatchesPrototype(F)) {
    DEBUG_WITH_TYPE(FXNREC_VERBOSE, dbgs()
                                        << "FXNREC: SPEC_QSORT: " << F.getName()
                                        << ": Does not match prototype.\n");
    return false;
  }
  if (!isDirectBranchBlock(BBE, &BBL)) {
    DEBUG_WITH_TYPE(FXNREC_VERBOSE,
                    dbgs() << "FXNREC: SPEC_QSORT: " << F.getName()
                           << ": Is not first direct branch block.\n");
    return false;
  }
  if (!GetKeyLoopPHIs(F, BBL, &PHIA, &PHIN)) {
    DEBUG_WITH_TYPE(FXNREC_VERBOSE, dbgs()
                                        << "FXNREC: SPEC_QSORT: " << F.getName()
                                        << ": No key loop PHIs.\n");
    return false;
  }
#ifdef _WIN32
  uint64_t BytesInLong = 4;
#else
  uint64_t BytesInLong = 8;
#endif // _WIN32
  if (!IsSwapInit(F, BBL, PHIA, BytesInLong, &PHIC0, &BBX0)) {
    DEBUG_WITH_TYPE(FXNREC_VERBOSE, dbgs()
                                        << "FXNREC: SPEC_QSORT: " << F.getName()
                                        << ": No first SWAPINIT.\n");
    return false;
  }
  if (!IsSwapInit(F, BBX0, PHIA, 4, &PHIC1, &BBX1)) {
    DEBUG_WITH_TYPE(FXNREC_VERBOSE, dbgs()
                                        << "FXNREC: SPEC_QSORT: " << F.getName()
                                        << ": No second SWAPINIT.\n");
    return false;
  }
  if (!IsSizeTest(BBX1, PHIN, ICmpInst::ICMP_ULT, 7, &BBIS0, &BBX2)) {
    DEBUG_WITH_TYPE(FXNREC_VERBOSE, dbgs()
                                        << "FXNREC: SPEC_QSORT: " << F.getName()
                                        << ": No insert sort size test.\n");
    return false;
  }
  if (!IsInsertionSort(F, BBIS0, PHIA, PHIN, PHIC0, PHIC1, &BBX3,
                       &FSwapFuncX)) {
    DEBUG_WITH_TYPE(FXNREC_VERBOSE, dbgs()
                                        << "FXNREC: SPEC_QSORT: " << F.getName()
                                        << ": No first insertion sort.\n");
    return false;
  }
  if (!isDirectBranchBlock(BBX3, &BBXN)) {
    DEBUG_WITH_TYPE(FXNREC_VERBOSE,
                    dbgs() << "FXNREC: SPEC_QSORT: " << F.getName()
                           << ": Is not second direct branch block.\n");
    return false;
  }
  if (!IsMedianComp(F, BBX2, PHIA, PHIN, &VPMFI, &VPMFO, &BBSW, &FMed3X)) {
    DEBUG_WITH_TYPE(FXNREC_VERBOSE, dbgs()
                                        << "FXNREC: SPEC_QSORT: " << F.getName()
                                        << ": Is not median computation.\n");
    return false;
  }
  if (!IsEasySwap0(F, *FSwapFuncX, BBSW, PHIA, VPMFI, VPMFO, PHIC0, PHIC1,
                   BytesInLong, &BBPV)) {
    DEBUG_WITH_TYPE(FXNREC_VERBOSE, dbgs()
                                        << "FXNREC: SPEC_QSORT: " << F.getName()
                                        << ": Is not first easy swap.\n");
    return false;
  }
  if (!IsMainWhileLoop(F, *FSwapFuncX, BBPV, PHIA, PHIN, PHIC0, PHIC1,
                       BytesInLong, &PHIPA1, &PHIPB1, &PHIPC1, &PHIPD1,
                       &PHISWCX, &BBSWT)) {
    DEBUG_WITH_TYPE(FXNREC_VERBOSE, dbgs()
                                        << "FXNREC: SPEC_QSORT: " << F.getName()
                                        << ": Is not main while loop.\n");
    return false;
  }
  if (!IsPHITest(BBSWT, PHISWCX, &BBIS1, &BBMIN0)) {
    DEBUG_WITH_TYPE(FXNREC_VERBOSE, dbgs()
                                        << "FXNREC: SPEC_QSORT: " << F.getName()
                                        << ": Is not swap_cnt test.\n");
    return false;
  }
  if (!IsInsertionSort(F, BBIS1, PHIA, PHIN, PHIC0, PHIC1, &BBX4,
                       &FSwapFuncXT) ||
      FSwapFuncXT != FSwapFuncX) {
    DEBUG_WITH_TYPE(FXNREC_VERBOSE, dbgs()
                                        << "FXNREC: SPEC_QSORT: " << F.getName()
                                        << ": No second insertion sort.\n");
    return false;
  }
  if (!isDirectBranchBlock(BBX4, &BBXNT) || BBXNT != BBXN) {
    DEBUG_WITH_TYPE(FXNREC_VERBOSE,
                    dbgs() << "FXNREC: SPEC_QSORT: " << F.getName()
                           << ": Is not second direct branch block.\n");
    return false;
  }
  if (!IsMinToR0(BBMIN0, PHIA, PHIPA1, PHIPB1, &VROL0, &VROR0, &BBVSW0)) {
    DEBUG_WITH_TYPE(FXNREC_VERBOSE, dbgs()
                                        << "FXNREC: SPEC_QSORT: " << F.getName()
                                        << ": No first MIN computation.\n");
    return false;
  }
  if (!IsVecSwap(*FSwapFuncX, BBVSW0, VROL0, VROR0, PHIA, PHIPB1, PHIC0, PHIC1,
                 &BBMIN1)) {
    DEBUG_WITH_TYPE(FXNREC_VERBOSE, dbgs()
                                        << "FXNREC: SPEC_QSORT: " << F.getName()
                                        << ": No first vecswap.\n");
    return false;
  }
  if (!IsMinToR1(F, BBMIN1, BBMIN0, PHIA, PHIN, PHIPC1, PHIPD1, &VROL1, &VROR1,
                 &VANES, &BBVSW1)) {
    DEBUG_WITH_TYPE(FXNREC_VERBOSE, dbgs()
                                        << "FXNREC: SPEC_QSORT: " << F.getName()
                                        << ": No second MIN computation.\n");
    return false;
  }
  if (!IsVecSwap(*FSwapFuncX, BBVSW1, VROL1, VROR1, PHIPB1, VANES, PHIC0, PHIC1,
                 &BBTPCB0)) {
    DEBUG_WITH_TYPE(FXNREC_VERBOSE, dbgs()
                                        << "FXNREC: SPEC_QSORT: " << F.getName()
                                        << ": No second vecswap.\n");
    return false;
  }
  if (!IsTwoPtrCmpBlock(F, BBTPCB0, PHIPB1, PHIPA1, &VTP0, &BBRC0, &BBTPCB1)) {
    DEBUG_WITH_TYPE(FXNREC_VERBOSE, dbgs()
                                        << "FXNREC: SPEC_QSORT: " << F.getName()
                                        << ": No first two ptr comp block.\n");
    return false;
  }
  if (!IsRecCall0Block(F, BBRC0, BBTPCB1, VTP0, PHIA)) {
    DEBUG_WITH_TYPE(FXNREC_VERBOSE, dbgs()
                                        << "FXNREC: SPEC_QSORT: " << F.getName()
                                        << ": No recursive call block.\n");
    return false;
  }
  if (!IsTwoPtrCmpBlock(F, BBTPCB1, PHIPD1, PHIPC1, &VTP1, &BBRC1, &BBX5)) {
    DEBUG_WITH_TYPE(FXNREC_VERBOSE, dbgs()
                                        << "FXNREC: SPEC_QSORT: " << F.getName()
                                        << ": No second two ptr comp block.\n");
    return false;
  }
  if (!IsRecCall1Block(F, BBRC1, BBL, VTP1, VANES, &VPHIAO, &VPHINO)) {
    DEBUG_WITH_TYPE(FXNREC_VERBOSE, dbgs()
                                        << "FXNREC: SPEC_QSORT: " << F.getName()
                                        << ": No iterative call block.\n");
    return false;
  }
  if (!AreKeyLoopPHIsOK(PHIA, PHIN, VPHIAO, VPHINO)) {
    DEBUG_WITH_TYPE(FXNREC_VERBOSE, dbgs()
                                        << "FXNREC: SPEC_QSORT: " << F.getName()
                                        << ": Key loop PHIs not OK.\n");
    return false;
  }
  if (!isDirectBranchBlock(BBX5, &BBXNT) || BBXNT != BBXN) {
    DEBUG_WITH_TYPE(FXNREC_VERBOSE,
                    dbgs() << "FXNREC: SPEC_QSORT: " << F.getName()
                           << ": Is not third direct branch block.\n");
    return false;
  }
  if (!BadCmpArgUse(F, *FMed3X)) {
    DEBUG_WITH_TYPE(FXNREC_VERBOSE, dbgs()
                                        << "FXNREC: SPEC_QSORT: " << F.getName()
                                        << ": Bad cmp arg use.\n");
    return false;
  }
  *FSwapFunc = FSwapFuncX;
  *FMed3 = FMed3X;
  return true;
}

static bool FunctionRecognizerImpl(Function &F) {
  if (isQsortCompare(F)) {
    F.addFnAttr("is-qsort-compare");
    NumFunctionsRecognized++;
    LLVM_DEBUG(dbgs() << "FUNCTION-RECOGNIZER: FOUND QSORT-COMPARE "
                      << F.getName() << "\n");
    return true;
  }
  if (isQsortMed3(F)) {
    F.addFnAttr("is-qsort-med3");
    NumFunctionsRecognized++;
    LLVM_DEBUG(dbgs() << "FUNCTION-RECOGNIZER: FOUND QSORT-MED3 " << F.getName()
                      << "\n");
    return true;
  }
  if (isQsortSwapFunc(F)) {
    F.addFnAttr("is-qsort-swapfunc");
    NumFunctionsRecognized++;
    LLVM_DEBUG(dbgs() << "FUNCTION-RECOGNIZER: FOUND QSORT-SWAPFUNC "
                      << F.getName() << "\n");
    return true;
  }
  Function *FSwapFunc = nullptr;
  Function *FMed3 = nullptr;
  if (isQsortSpecQsort(F, &FSwapFunc, &FMed3)) {
    F.addFnAttr("is-qsort-spec_qsort");
    FSwapFunc->addFnAttr("must-be-qsort-swapfunc");
    FMed3->addFnAttr("must-be-qsort-med3");
    Argument *CmpUse = F.getArg(3);
    for (User *U : CmpUse->users()) {
      auto CB = cast<CallBase>(U);
      if (!CB->getCalledFunction() && CB->getCalledOperand() == CmpUse)
        CB->addFnAttr("must-be-qsort-compare");
    }
    NumFunctionsRecognized++;
    LLVM_DEBUG(dbgs() << "FUNCTION-RECOGNIZER: FOUND QSORT-SPEC_QSORT "
                      << F.getName() << "\n");
    return true;
  }
  return false;
}

namespace {

struct FunctionRecognizerLegacyPass : public FunctionPass {
  static char ID; // Pass identification, replacement for typeid
  FunctionRecognizerLegacyPass(void) : FunctionPass(ID) {
    initializeFunctionRecognizerLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }
  bool runOnFunction(Function &F) override {
    if (skipFunction(F))
      return false;
    return FunctionRecognizerImpl(F);
  }
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }
};

} // namespace

char FunctionRecognizerLegacyPass::ID = 0;
INITIALIZE_PASS(FunctionRecognizerLegacyPass, "functionrecognizer",
                "Function Recognizer", false, false)

FunctionPass *llvm::createFunctionRecognizerLegacyPass(void) {
  return new FunctionRecognizerLegacyPass();
}

PreservedAnalyses FunctionRecognizerPass::run(Function &F,
                                              FunctionAnalysisManager &AM) {
  FunctionRecognizerImpl(F);
  return PreservedAnalyses::all();
}
#endif // INTEL_FEATURE_SW_ADVANCED
