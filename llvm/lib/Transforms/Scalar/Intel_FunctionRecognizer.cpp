//===- Intel_FunctionRecognizer.cpp - Function Recognizer -------------===//
//
// Copyright (C) 2020-2020 Intel Corporation. All rights reserved.
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
// Utility functions used by multiple function recognizers
//

using DenseMapBBToI = DenseMap<BasicBlock *, int64_t>;
using DenseMapBBToV = DenseMap<BasicBlock *, Value *>;

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
  auto IsCIVal = [&UnknownIndex](Value *V, uint64_t &Val) -> bool {
    auto CI = dyn_cast<ConstantInt>(V);
    if (!CI)
      return false;
    if (Val == UnknownIndex) {
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

  // This is the main code for isQsortCompare().
  BasicBlock *BB0 = &F.getEntryBlock();
  BasicBlock *BBL0 = nullptr;
  BasicBlock *BBR0 = nullptr;
  BasicBlock *BBL1 = nullptr;
  BasicBlock *BBR1 = nullptr;
  BasicBlock *BBL2 = nullptr;
  BasicBlock *BBR2 = nullptr;
  BasicBlock *BBRet = nullptr;
  DenseMapBBToI BBRetMap;
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

  // This is the main code for isQsortMed3().
  DenseMapBBToV PHIMap;
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
  return IsOKPHIBlock(BBPHI, PHIMap);
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
  // Note: the bircast instructions may be absent, in which case we set
  // 'V0' and 'V1' to nullptr.
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

  // This is the main code for isQsortSwapFunc().
  BasicBlock *BBE = &F.getEntryBlock();
  BasicBlock *BBL0 = nullptr;
  BasicBlock *BBR0 = nullptr;
  BasicBlock *BBL1 = nullptr;
  BasicBlock *BBR1 = nullptr;
  BasicBlock *BBRet = nullptr;
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
static bool isQsortSpecQsort(Function &F) {

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
    Value *V0 = nullptr;
    ConstantInt *CIDen = nullptr;
    ICmpInst::Predicate Pred = ICmpInst::BAD_ICMP_PREDICATE;
    if (!match(BI->getCondition(),
               m_ICmp(Pred,
                      m_URem(m_Sub(m_PtrToInt(m_Value(V0)), m_Zero()),
                             m_ConstantInt(CIDen)),
                      m_Zero())))
      return false;
    if (CIDen->getZExtValue() != UDen || V0 != PHIA ||
        Pred != ICmpInst::ICMP_NE )
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
    Value *V0 = nullptr;
    ConstantInt *CIDen = nullptr;
    ICmpInst::Predicate Pred = ICmpInst::BAD_ICMP_PREDICATE;
    if (!match(
            BI->getCondition(),
            m_ICmp(Pred, m_URem(m_Value(V0), m_ConstantInt(CIDen)), m_Zero())))
      return false;
    if (CIDen->getZExtValue() != UDen || V0 != F.getArg(2) ||
        Pred != ICmpInst::ICMP_NE)
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
    Value *V0 = nullptr;
    ConstantInt *CIDen = nullptr;
    Instruction *BC = BI->getPrevNonDebugInstruction();
    ICmpInst::Predicate Pred = ICmpInst::BAD_ICMP_PREDICATE;
    if (!BC || !match(BC, m_SelectCst<0, 1>(
                              m_ICmp(Pred, m_Value(V0), m_ConstantInt(CIDen)))))
      return false;
    if (CIDen->getZExtValue() != UDen || V0 != F.getArg(2) ||
        Pred != ICmpInst::ICMP_EQ)
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
  //    %cmp51 = icmp ugt i64 'PHIN', 7
  //    br i1 %cmp51, label 'BBL', label 'BBR'
  //
  //  If we return 'true', we assign values to 'BBL' and 'BBR'.
  //
  auto IsSizeTest = [](BasicBlock *BBI, PHINode *PHIN, uint64_t SmallSize,
                       BasicBlock **BBL, BasicBlock **BBR) -> bool {
    BranchInst *BI = nullptr;
    ICmpInst *IC = nullptr;
    if (!getBIAndIC(BBI, ICmpInst::ICMP_ULT, &BI, &IC))
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
    Value *V0 = nullptr;
    if (!match(GEP->getOperand(1), m_Sub(m_Zero(), m_Value(V0))))
      return false;
    if (V0 != F.getArg(2))
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
  //
  auto IsSwapLSChain = [](StoreInst *SI, Value **VLO, Value **VSO) -> bool {
    auto LI = dyn_cast<LoadInst>(SI->getValueOperand());
    if (!LI)
      return false;
    auto BC0 = dyn_cast<BitCastInst>(LI->getPointerOperand());
    if (!BC0)
      return false;
    auto BC1 = dyn_cast<BitCastInst>(SI->getPointerOperand());
    if (!BC1)
      return false;
    *VLO = BC0->getOperand(0);
    *VSO = BC1->getOperand(0);
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
  auto IsSwapBlock = [&IsSwapLSChain, IsSwapGEPChain](
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
  auto IsSwapFuncBlock = [&IsSwapGEPChain](Function &F, BasicBlock *BBI,
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
  auto IsInsertionSort = [&IsISInit, &IsISOTerm, &IsISITerm, &IsPHITest,
                          &IsSwapBlock, &IsSwapFuncBlock, &IsLoopIncBlock](
                             Function &F, BasicBlock *BBI, PHINode *PHIA,
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
    if (!IsSwapBlock(F, BBSW0, BBILI, PHIIL, TILong))
      return false;
    if (!IsPHITest(BBT1, PHIC1, &BBSW1, &BBSW2))
      return false;
    if (!IsSwapBlock(F, BBSW1, BBILI, PHIIL, llvm::Type::getInt32Ty(FC)))
      return false;
    if (!IsSwapFuncBlock(F, BBSW2, BBILI, PHIIL, PHIC0, PHIC1, FSwapFunc))
      return false;
    if (!IsLoopIncBlock(F, BBILI, BBIT, PHIIL, false))
      return false;
    if (!IsLoopIncBlock(F, BBOLI, BBOT, PHIOL, true))
      return false;
    *BBO = BBX;
    return true;
  };

  // This is the main code for isQsortSpecQsort().
  BasicBlock *BBE = &F.getEntryBlock();
  BasicBlock *BBL = nullptr;
  BasicBlock *BBX0 = nullptr;
  BasicBlock *BBX1 = nullptr;
  BasicBlock *BBX2 = nullptr;
  BasicBlock *BBIS = nullptr;
  PHINode *PHIA = nullptr;
  PHINode *PHIN = nullptr;
  PHINode *PHIC0 = nullptr;
  PHINode *PHIC1 = nullptr;
  Function *FSwapFunc = nullptr;
  DenseMapBBToV PHIMap;
  if (!MatchesPrototype(F)) {
    DEBUG_WITH_TYPE(FXNREC_VERBOSE,
                    dbgs() << "FXNREC: SPEC_QSORT: " << F.getName()
                           << ": Does not match prototype.\n");
    return false;
  }
  if (!isDirectBranchBlock(BBE, &BBL)) {
    DEBUG_WITH_TYPE(FXNREC_VERBOSE,
                    dbgs() << "FXNREC: SPEC_QSORT: " << F.getName()
                           << ": Is not direct branch block.\n");
    return false;
  }
  if (!GetKeyLoopPHIs(F, BBL, &PHIA, &PHIN)) {
    DEBUG_WITH_TYPE(FXNREC_VERBOSE,
                    dbgs() << "FXNREC: SPEC_QSORT: " << F.getName()
                           << ": No key loop PHIs.\n");
    return false;
  }
#ifdef _WIN32
  uint64_t BytesInLong = 4;
#else
  uint64_t BytesInLong = 8;
#endif // _WIN32
  if (!IsSwapInit(F, BBL, PHIA, BytesInLong, &PHIC0, &BBX0)) {
    DEBUG_WITH_TYPE(FXNREC_VERBOSE,
                    dbgs() << "FXNREC: SPEC_QSORT: " << F.getName()
                           << ": No first SWAPINIT.\n");
    return false;
  }
  if (!IsSwapInit(F, BBX0, PHIA, 4, &PHIC1, &BBX1)) {
    DEBUG_WITH_TYPE(FXNREC_VERBOSE,
                    dbgs() << "FXNREC: SPEC_QSORT: " << F.getName()
                           << ": No second SWAPINIT.\n");
    return false;
  }
  if (!IsSizeTest(BBX1, PHIN, 7, &BBIS, &BBX2)) {
    DEBUG_WITH_TYPE(FXNREC_VERBOSE,
                    dbgs() << "FXNREC: SPEC_QSORT: " << F.getName()
                           << ": No insert sort size test.\n");
    return false;
  }
  if (!IsInsertionSort(F, BBIS, PHIA, PHIN, PHIC0, PHIC1, &BBX2, &FSwapFunc)) {
    DEBUG_WITH_TYPE(FXNREC_VERBOSE,
                    dbgs() << "FXNREC: SPEC_QSORT: " << F.getName()
                           << ": No first insertion sort.\n");
    return false;
  }
  // More code to come here.
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
  if (isQsortSpecQsort(F)) {
    F.addFnAttr("is-qsort-spec_qsort");
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
