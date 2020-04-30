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

STATISTIC(NumFunctionsRecognized, "Number of Functions Recognized");

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
  auto ValidateBBGoToReturn =
      [](BasicBlock *BBI, int64_t PHIV, BasicBlock *BBRetTest,
         DenseMap<BasicBlock *, int64_t> &BBRetMap) -> BasicBlock * {
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
  auto ValidateReturn = [](BasicBlock *BBRet,
                           DenseMap<BasicBlock *, int64_t> &BBRetMap) -> bool {
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
  DenseMap<BasicBlock *, int64_t> BBRetMap;
  if (F.isDeclaration() || F.isVarArg() || F.arg_size() != 2)
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
  auto IsDirectBranchBlock = [](BasicBlock *BBI, Value *V0,
                                DenseMap<BasicBlock *, Value *> &PHIMap,
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
                                       Argument *CallP,
                                       DenseMap<BasicBlock *, Value *> &PHIMap,
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
  auto IsOKPHIBlock = [](BasicBlock *BBPHI,
                         DenseMap<BasicBlock *, Value *> &PHIMap) -> bool {
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
  DenseMap<BasicBlock *, Value *> PHIMap;
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
  if (!IsDirectBranchBlock(BBT1, A1, PHIMap, &BBPHI))
    return false;
  if (!IsCmpSelBlock(BBF1, A2, A0, PLT, A0, A2, A3, PHIMap, &BBS0) ||
      BBS0 != BBPHI)
    return false;
  if (!IsCmpBlock(BBF0, A3, A1, A2, PGT, &BBT2, &BBF2))
    return false;
  if (!IsDirectBranchBlock(BBT2, A1, PHIMap, &BBU0) || BBU0 != BBPHI)
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
    auto BI = dyn_cast<BranchInst>(BBI->getTerminator());
    if (!BI || BI->isUnconditional())
      return false;
    auto IC = dyn_cast<ICmpInst>(BI->getCondition());
    if (!IC || IC->getPredicate() != Pred || IC->getOperand(0) != AI)
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
  // Search 'BBI' and if it has two and only two StoreInsts, and no other
  // Instruction that writes to memory, return 'true'. If we return 'true',
  // we set the values of 'ST1' and 'ST2' to the two StoreInsts found.
  //
  auto GetTwoStores = [](BasicBlock *BBI, StoreInst **SI1,
                         StoreInst **SI2) -> bool {
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
  auto IsSFDoBody = [&GetTwoStores, &IsLSChain](Function &F, BasicBlock *BBD,
                                                BasicBlock *BBI, Type *DType,
                                                Value *V0, Value *V1, Value *VN,
                                                BasicBlock **BBX) -> bool {
    auto BI = dyn_cast<BranchInst>(BBD->getTerminator());
    if (!BI || BI->isUnconditional() || BI->getSuccessor(0) != BBD)
      return false;
    auto IC = dyn_cast<ICmpInst>(BI->getCondition());
    if (!IC || IC->getPredicate() != ICmpInst::ICMP_SGT)
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
    if (!GetTwoStores(BBD, &SI0, &SI1))
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
  // Return 'true' if 'BBI' matches a BasicBlock of the form:
  //
  //    br label 'BBO'
  //
  // If we return 'true', set the value of 'BBO'.
  //
  auto IsDirectBranch = [](BasicBlock *BBI, BasicBlock **BBO) -> bool {
    auto BI = dyn_cast<BranchInst>(BBI->getTerminator());
    if (!BI || BI->isConditional())
      return false;
    *BBO = BI->getSuccessor(0);
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
  auto IsDoLoop = [&IsSFDoPreH, &IsSFDoBody, &IsDirectBranch](
                      Function &F, BasicBlock *BBI, BasicBlock *BBRet,
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
    if (!IsDirectBranch(BBX, &BBR))
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
  if (F.isDeclaration() || F.isVarArg() || F.arg_size() != 5)
    return false;
  LLVMContext &FC = F.getContext();
  Type *TI32 = llvm::Type::getInt32Ty(FC);
  for (unsigned I = 0, E = F.arg_size(); I < E; ++I) {
    Type *TyArg = F.getArg(I)->getType();
    if (I < 2) {
      if (!TyArg->isPointerTy())
        return false;
    } else if (TyArg != TI32) {
      return false;
    }
  }
  if (!F.getReturnType()->isVoidTy())
    return false;
  BasicBlock *BBE = &F.getEntryBlock();
  BasicBlock *BBL0 = nullptr;
  BasicBlock *BBR0 = nullptr;
  BasicBlock *BBL1 = nullptr;
  BasicBlock *BBR1 = nullptr;
  BasicBlock *BBRet = nullptr;
  if (!IsArgCmpBlock(BBE, F.getArg(3), ICmpInst::ICMP_SLE, &BBL0, &BBR0))
    return false;
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
