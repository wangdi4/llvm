//===------- Intel_QsortRecognizer.cpp --------------------------------===//
//
// Copyright (C) 2019-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This implements a qsort recognition pass. It looks through the module and
// attempts to identify Functions that implement a qsort of the type described
// in the paper "Engineering a Sort Function" by Jon L. Bentley and M. Douglas
// McIlroy (in Software -- Practice and Experience, Volume 23, Issue 11). If
// such a Function is identified, it will mark it with the "is-qsort"
// Function attribute. This qsort is a well-tuned implementation of quicksort
// which degenerates to insertion sort for sufficiently small arrays.
//
//===----------------------------------------------------------------------===//
#include "llvm/Transforms/IPO/Intel_QsortRecognizer.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/IR/Type.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/Utils/Intel_IPOUtils.h"
#include <set>
#include <queue>

using namespace llvm;
using namespace PatternMatch;

#define DEBUG_TYPE "qsortrecognizer"

STATISTIC(QsortsRecognized, "Number of qsort functions recognized");

static cl::opt<bool> QsortTestPivot("qsort-test-pivot",
                                     cl::init(true), cl::ReallyHidden);

static cl::opt<bool> QsortTestInsert("qsort-test-insert",
                                     cl::init(true), cl::ReallyHidden);

static cl::opt<bool> QsortTestPivotMovers("qsort-test-pivot-movers",
                                          cl::init(true), cl::ReallyHidden);

static cl::opt<bool> QsortTestMin("qsort-test-min",
                                   cl::init(true), cl::ReallyHidden);

static cl::opt<bool> QsortTestSwap("qsort-test-swap",
                                    cl::init(true), cl::ReallyHidden);

static cl::opt<bool> QsortTestRecursion("qsort-test-recursion",
                                         cl::init(true), cl::ReallyHidden);

// Utilities to handle the argument's analysis
static IntelArgumentAlignmentUtils ArgUtil;

//
// Return 'true' if 'V' represents a PHINode and one of its incoming values
// is 'Arg'.
//
static bool isPHINodeWithArgIncomingValue(Value *V, Argument *Arg) {
  auto PHIN = dyn_cast<PHINode>(V);
  if (!PHIN)
    return false;
  auto Num = PHIN->getNumIncomingValues();
  for (unsigned I = 0; I < Num; ++I)
    if (PHIN->getIncomingValue(I) == Arg)
      return true;
  return false;
}

//
// Return 'true' if 'BBOLH' is the first basic block in an insertion sort of
// the array 'ArgArray' of size 'ArgSize'. The particular insertion sort
// recognized is a five basic block code fragment of the form:
//
//  %12 = getelementptr inbounds i8, i8* %7, i64 8
//
// 13: (BBOLH: Basic Block of Outer Loop Header)     ; preds = %29, %11
//  %14 = phi i8* [ %12, %11 ], [ %30, %29 ]
//  %15 = icmp ugt i8* %14, %7
//  br i1 %15, label %16, label %29
//
// 16: (BBILH: Basic Block of Inner Loop Header)     ; preds = %13, %23
//  %17 = phi i8* [ %18, %23 ], [ %14, %13 ]
//  %18 = getelementptr inbounds i8, i8* %17, i64 -8
//  %19 = bitcast i8* %18 to %struct.arc**
//  %20 = bitcast i8* %17 to %struct.arc**
//  %21 = tail call i32 @arc_compare(%struct.arc** nonnull %19,
//                                   %struct.arc** %20) #12
//  %22 = icmp sgt i32 %21, 0
//  br i1 %22, label %23, label %29
//
// 23: (BBIL: Basic Block of Inner Loop)             ; preds = %16
//  %24 = bitcast i8* %17 to i64*
//  %25 = load i64, i64* %24, align 8, !tbaa !7
//  %26 = bitcast i8* %18 to i64*
//  %27 = load i64, i64* %26, align 8, !tbaa !7
//  store i64 %27, i64* %24, align 8, !tbaa !7
//  store i64 %25, i64* %26, align 8, !tbaa !7
//  %28 = icmp ugt i8* %18, %7
//  br i1 %28, label %16, label %29
//
// 29: (BBOL: Basic Block of Outer Loop)             ; preds = %23, %16, %13
//  %30 = getelementptr inbounds i8, i8* %14, i64 8
//  %31 = icmp ult i8* %30, %9
//  br i1 %31, label %13, label %321
//   ...
// 321: (BBEnd: Exit Basic Block)        ; preds = %313, %252, %234, %29, %5
//  ret void

static bool isInsertionSort(BasicBlock *BBStart, Argument *ArgArray,
                            Argument *ArgSize) {

  // The Validate*() lambda functions below all return 'true' if the basic
  // block they are checking is validated (is proved to have the required
  // properties).

  //
  // Validate 'BBOLH', the outer loop header of the insertion sort. Recognize
  // and assign '*BBILH', its true successor and '*BBOL', its false successor.
  //
  auto ValidateBBOLH = [&ArgArray](BasicBlock *BBOLH, BasicBlock **BBILH,
                                   BasicBlock **BBOL) -> bool {
    auto BI = dyn_cast<BranchInst>(BBOLH->getTerminator());
    if (!BI || BI->isUnconditional() || BI->getNumSuccessors() != 2)
      return false;
    *BBILH = BI->getSuccessor(0);
    *BBOL = BI->getSuccessor(1);
    auto ICI = dyn_cast<ICmpInst>(BI->getCondition());
    if (!ICI || ICI->getPredicate() != ICmpInst::ICMP_UGT)
      return false;
    if (!isPHINodeWithArgIncomingValue(ICI->getOperand(1), ArgArray))
      return false;
    auto PHIN0 = dyn_cast<PHINode>(ICI->getOperand(0));
    if (!PHIN0 || PHIN0->getNumIncomingValues() != 2)
      return false;
    bool FoundPHI0 = false;
    bool FoundPHI1 = false;
    for (unsigned I = 0; I < 2; ++I) {
      auto GEP = dyn_cast<GetElementPtrInst>(PHIN0->getIncomingValue(I));
      if (!GEP || GEP->getNumOperands() != 2)
        return false;
      auto CI = dyn_cast<ConstantInt>(GEP->getOperand(1));
      if (!CI || CI->getZExtValue() != 8)
        return false;
      Value *V = GEP->getPointerOperand();
      if (!FoundPHI0 && isPHINodeWithArgIncomingValue(V, ArgArray))
        FoundPHI0 = true;
      else if (!FoundPHI1 && V == PHIN0)
        FoundPHI1 = true;
      else
        return false;
    }
    return FoundPHI0 && FoundPHI1;
  };

  //
  // Validate 'BBILH', the inner loop header of the insertion sort. Recognize
  // and assign its true successor '*BBIL', and validate that its false
  // successor is 'BBOL'. 'BBOLH' is the BasicBlock of the outer loop header.
  //
  auto ValidateBBILH = [](BasicBlock *BBILH, BasicBlock **BBIL,
                          BasicBlock *BBOL, BasicBlock *BBOLH) -> bool {
    auto BI = dyn_cast<BranchInst>(BBILH->getTerminator());
    if (!BI || BI->getNumSuccessors() != 2)
      return false;
    *BBIL = BI->getSuccessor(0);
    if (BI->getSuccessor(1) != BBOL)
      return false;
    auto IC = dyn_cast<ICmpInst>(BI->getCondition());
    if (!IC || IC->getPredicate() != ICmpInst::ICMP_SGT)
      return false;
    auto CIZ = dyn_cast<ConstantInt>(IC->getOperand(1));
    if (!CIZ || !CIZ->isZero())
      return false;
    auto AC = dyn_cast<CallInst>(IC->getOperand(0));
    if (!AC || AC->getNumArgOperands() != 2)
      return false;
    auto BC0 = dyn_cast<BitCastInst>(AC->getArgOperand(0));
    if (!BC0)
      return false;
    auto GEPI = dyn_cast<GetElementPtrInst>(BC0->getOperand(0));
    if (!GEPI || GEPI->getNumOperands() != 2)
      return false;
    if (GEPI != BBILH->front().getNextNonDebugInstruction())
      return false;
    auto CIG = dyn_cast<ConstantInt>(GEPI->getOperand(1));
    if (!CIG || CIG->getSExtValue() != -8)
      return false;
    auto PN = dyn_cast<PHINode>(GEPI->getPointerOperand());
    if (!PN || PN->getNumIncomingValues() != 2 || PN != &BBILH->front())
      return false;
    if (PN->getIncomingValue(0) != GEPI)
      return false;
    if (PN->getIncomingValue(1) != &BBOLH->front())
      return false;
    auto BC1 = dyn_cast<BitCastInst>(AC->getArgOperand(1));
    if (!BC1)
      return false;
    if (BC1->getOperand(0) != PN)
      return false;
    return true;
  };

  //
  // Validate 'BBIL', the inner loop block of the insertion sort.  Recognize
  // 'BBILH' as its true successor and 'BBOL' as its false successor.
  //
  auto ValidateBBIL = [&ArgArray](BasicBlock *BBIL, BasicBlock *BBILH,
                                  BasicBlock *BBOL) -> bool {
    auto BI = dyn_cast<BranchInst>(BBIL->getTerminator());
    if (!BI || BI->getNumSuccessors() != 2)
      return false;
    if (BI->getSuccessor(0) != BBILH)
      return false;
    if (BI->getSuccessor(1) != BBOL)
      return false;
    auto IC = dyn_cast<ICmpInst>(BI->getCondition());
    if (!IC || IC->getPredicate() != ICmpInst::ICMP_UGT)
      return false;
    if (IC->getOperand(0) != BBILH->front().getNextNonDebugInstruction())
      return false;
    if (!isPHINodeWithArgIncomingValue(IC->getOperand(1), ArgArray))
      return false;
    auto BC0 = dyn_cast<BitCastInst>(&BBIL->front());
    if (!BC0 || BC0->getOperand(0) != &BBILH->front())
      return false;
    auto BC0N = BC0->getNextNonDebugInstruction();
    if (!BC0N)
      return false;
    auto LI0 = dyn_cast<LoadInst>(BC0N);
    if (!LI0 || LI0->getPointerOperand() != BC0 || !LI0->hasOneUse())
      return false;
    auto LI0N = LI0->getNextNonDebugInstruction();
    if (!LI0N)
      return false;
    auto BC1 = dyn_cast<BitCastInst>(LI0N);
    auto BBILH2 = BBILH->front().getNextNonDebugInstruction();
    if (!BC1 || BC1->getOperand(0) != BBILH2)
      return false;
    auto SI0 = dyn_cast<StoreInst>(LI0->user_back());
    if (!SI0 || SI0->getValueOperand() != LI0 ||
        SI0->getPointerOperand() != BC1)
      return false;
    auto BC1N = BC1->getNextNonDebugInstruction();
    if (!BC1N)
      return false;
    auto LI1 = dyn_cast<LoadInst>(BC1N);
    if (!LI1 || LI1->getPointerOperand() != BC1 || !LI1->hasOneUse())
      return false;
    auto SI1 = dyn_cast<StoreInst>(LI1->user_back());
    if (!SI1 || SI1->getValueOperand() != LI1 ||
        SI1->getPointerOperand() != BC0)
      return false;
    return true;
  };

  //
  // Validate 'BBOL', the outer loop block of the insertion sort, whose true
  // successor should be 'BBOLH' and whose false successor should be recognized
  // and assigned as '*BBEnd'.
  //
  auto ValidateBBOL = [&ArgArray, &ArgSize](BasicBlock *BBOL,
                                            BasicBlock *BBOLH,
                                            BasicBlock **BBEnd) -> bool {
    Value *PV = nullptr;
    ConstantInt *CI = nullptr;
    auto BI = dyn_cast_or_null<BranchInst>(BBOL->getTerminator());
    if (!BI || BI->getNumSuccessors() != 2)
      return false;
    if (BI->getSuccessor(0) != BBOLH)
      return false;
    *BBEnd = BI->getSuccessor(1);
    auto IC = dyn_cast<ICmpInst>(BI->getCondition());
    if (!IC || IC->getPredicate() != ICmpInst::ICMP_ULT)
      return false;
    auto GEPIL = dyn_cast<GetElementPtrInst>(IC->getOperand(0));
    if (!GEPIL || GEPIL->getNumOperands() != 2)
      return false;
    auto CI1 = dyn_cast<ConstantInt>(GEPIL->getOperand(1));
    if (!CI1 || CI1->getZExtValue() != 8)
      return false;
    if (GEPIL->getPointerOperand() != &BBOLH->front())
      return false;
    auto CIL = dyn_cast<ConstantInt>(GEPIL->getOperand(1));
    if (!CIL || CIL->getZExtValue() != 8)
      return false;
    auto GEPIR = dyn_cast<GetElementPtrInst>(IC->getOperand(1));
    if (!GEPIR || GEPIR->getNumOperands() != 2)
      return false;
    if (!isPHINodeWithArgIncomingValue(GEPIR->getPointerOperand(), ArgArray))
      return false;
    if (!match(GEPIR->getOperand(1), m_Shl(m_Value(PV), m_ConstantInt(CI))) ||
        CI->getZExtValue() != 3)
      return false;
    return isPHINodeWithArgIncomingValue(PV, ArgSize);
  };

  // Validate 'BBEnd', the BasicBlock to which the insertion sort exits.
  auto ValidateBBEnd = [](BasicBlock *BBEnd) -> bool {
    if (BBEnd->empty())
      return false;
    auto RI = dyn_cast<ReturnInst>(&BBEnd->front());
    return RI && !RI->getReturnValue();
  };

  // Main code for isInsertionSort().
  // Validate each of the six basic blocks in the insertion sort.
  BasicBlock *BBOLH = BBStart;
  BasicBlock *BBILH = nullptr;
  BasicBlock *BBOL = nullptr;
  BasicBlock *BBIL = nullptr;
  BasicBlock *BBEnd = nullptr;
  if (!ValidateBBOLH(BBOLH, &BBILH, &BBOL))
    return false;
  if (!ValidateBBILH(BBILH, &BBIL, BBOL, BBOLH))
    return false;
  if (!ValidateBBIL(BBIL, BBILH, BBOL))
    return false;
  if (!ValidateBBOL(BBOL, BBOLH, &BBEnd))
    return false;
  if (!ValidateBBEnd(BBEnd))
    return false;
  return true;
}

//
// Return a PHINode that represents the Argument 'A' in 'BBstart', if there
// is one.  For example, in BasicBlock 4, %5 represents argument %1, while %6
// represents argument %0. Note that the PHINode may merge the argument value
// with some other value.
//
// define internal fastcc void @qsort(i8* %0, i64 %1) unnamed_addr #11 {
//  %3 = icmp ult i64 %1, 7
//  br i1 %3, label %4, label %30
// 4:                                                ; preds = %193, %2
//  %5 = phi i64 [ %1, %2 ], [ %196, %193 ]
//  %6 = phi i8* [ %0, %2 ], [ %195, %193 ]
//
static Value *findPHINodeArgument(BasicBlock *BBStart, Argument *A) {
   for (auto &PHIN : BBStart->phis())
    for (unsigned I = 0; I < PHIN.getNumIncomingValues(); ++I)
      if (PHIN.getIncomingValue(I) == A)
        return &PHIN;
   return nullptr;
}

//
// Return the pivot value of the Qsort. The pivot computation begins at
// 'BBStart'. It must be a pointer to an element of the array whose base is
// 'ArgArray' and whose size is 'ArgSize'.
//
static Value *qsortPivot(BasicBlock *BBStart, Argument *ArgArray,
                         Argument *ArgSize) {
  //
  // Return a PHINode, to be tested as a validate pivot value. As a heuristic,
  // we use the first PHINode following the series of blocks that compute the
  // pivot value.
  //
  auto FindPivotCandidate = [](BasicBlock *BB) -> PHINode * {
    auto BI = dyn_cast<BranchInst>(BB->getTerminator());
    if (!BI)
      return nullptr;
    for (unsigned I = 0; I < BI->getNumSuccessors(); ++I) {
      BasicBlock *BBS = BI->getSuccessor(I);
      if (BBS->empty())
        continue;
      auto PN = dyn_cast<PHINode>(&(BBS->front()));
      if (PN)
        return PN;
    }
    return nullptr;
  };

  //
  // Create a list of pivot values 'PivotSet' for the array 'A'. These values
  // are all fed by PHINodes and/or SelectInsts to be values of the 'Pivot
  // Base'. The BasicBlocks in which the pivot values appear are saved in
  // 'BBPivotSet'.  The PHINodes and SelectInsts used to merge the pivot values
  // are saved in 'JoinSet', which is used to check that we don't traverse the
  // path to a pivot value more than once.
  //
  // For example,
  //   %31 = phi i8* [ %287, %285 ], [ %0, %2 ]
  //   %35 = getelementptr inbounds i8, i8* %31, i64 %34
  //   %40 = getelementptr inbounds i8, i8* %31, i64 %39
  //   %44 = getelementptr inbounds i8, i8* %31, i64 %43
  //   %46 = getelementptr inbounds i8, i8* %31, i64 %45
  //   %61 = select i1 %60, i8* %46, i8* %31
  //   %69 = select i1 %68, i8* %31, i8* %46
  //   %71 = phi i8* [ %61, %56 ], [ %69, %64 ], [ %44, %54 ], [ %44, %62 ]
  //   %73 = getelementptr inbounds i8, i8* %35, i64 %72
  //   %74 = getelementptr inbounds i8, i8* %35, i64 %43
  //   %89 = select i1 %88, i8* %74, i8* %73
  //   %97 = select i1 %96, i8* %73, i8* %74
  //   %99 = phi i8* [ %89, %84 ], [ %97, %92 ], [ %35, %82 ], [ %35, %90 ]
  //   %101 = getelementptr inbounds i8, i8* %40, i64 %100
  //   %102 = getelementptr inbounds i8, i8* %40, i64 %72
  //   %117 = select i1 %116, i8* %40, i8* %101
  //   %125 = select i1 %124, i8* %101, i8* %40
  //   %127 = phi i8* [ %40, %37 ], [ %117, %112 ], [ %125, %120 ],
  //     [ %102, %110 ], [ %102, %118 ]
  //   %128 = phi i8* [ %35, %37 ], [ %99, %112 ], [ %99, %120 ], [ %99, %110 ],
  //     [ %99, %118 ]
  //   %129 = phi i8* [ %31, %37 ], [ %71, %112 ], [ %71, %120 ], [ %71, %110 ],
  //     [ %71, %118 ]
  //   %144 = select i1 %143, i8* %127, i8* %129
  //   %152 = select i1 %151, i8* %129, i8* %127
  //   %154 = phi i8* [ %35, %30 ], [ %144, %139 ], [ %152, %147 ],
  //     [ %128, %137 ], [ %128, %145 ]
  // The pivot values selected are %31, %35, %40, %44, %46, %73, %74, %101, and
  // %102.
  //
  std::function<void(Value *, Value *, SmallPtrSetImpl<Value *> *,
                     SmallPtrSetImpl<Value *> *,
                     SmallPtrSetImpl<BasicBlock *> *)>
      MakePivotList = [&MakePivotList](
                          Value *A, Value *PivotBase,
                          SmallPtrSetImpl<Value *> *PivotSet,
                          SmallPtrSetImpl<Value *> *JoinSet,
                          SmallPtrSetImpl<BasicBlock *> *BBPivotSet) {
        if (PivotBase == A)
          return;
        if (auto PN = dyn_cast<PHINode>(PivotBase)) {
          if (!JoinSet->insert(PN).second)
            return;
          for (unsigned I = 0; I < PN->getNumIncomingValues(); ++I)
            MakePivotList(A, PN->getIncomingValue(I), PivotSet, JoinSet,
                          BBPivotSet);
        } else if (auto SI = dyn_cast<SelectInst>(PivotBase)) {
          if (!JoinSet->insert(SI).second)
            return;
          MakePivotList(A, SI->getTrueValue(), PivotSet, JoinSet, BBPivotSet);
          MakePivotList(A, SI->getFalseValue(), PivotSet, JoinSet, BBPivotSet);
        } else {
          if (PivotSet->insert(PivotBase).second) {
            LLVM_DEBUG({
              dbgs() << "QsortRec: Pivot: ";
              PivotBase->dump();
            });
            auto II = dyn_cast<Instruction>(PivotBase);
            if (II)
              BBPivotSet->insert(II->getParent());
          }
        }
      };

  //
  // Return 'true' if no Instruction in 'F' may write to memory.
  //
  auto IsNoStoreFunction = [](Function *F) -> bool {
    for (auto &I : instructions(F))
      if (I.mayWriteToMemory())
        return false;
    return true;
  };

  //
  // Return 'true' if no BasicBlock in 'BBPivotSet' assigns to memory. (This
  // is used to ensure that the computation of the pivot value is a "pure"
  // computation.
  //
  auto ValidateBBsForPivots =
      [&IsNoStoreFunction](SmallPtrSetImpl<BasicBlock *> *BBPivotSet) -> bool {
    SmallPtrSet<Function *, 8> NoStoreFunctions;
    for (BasicBlock *BB : *BBPivotSet)
      for (auto &I : *BB) {
        if (isa<StoreInst>(&I))
          return false;
        auto CB = dyn_cast<CallBase>(&I);
        if (CB) {
          Function *CF = CB->getCalledFunction();
          if (!CF)
            return false;
          if (NoStoreFunctions.find(CF) != NoStoreFunctions.end())
            continue;
          if (!IsNoStoreFunction(CF))
            return false;
          NoStoreFunctions.insert(CF);
        }
      }
    return true;
  };

  //
  // Return 'true' if 'V' represents the offset into "middle" of an array of
  // of length 'N'.
  //
  auto IsMiddleOffset = [](Value *V, Value *N) -> bool {
    auto SO = dyn_cast<ShlOperator>(V);
    if (!SO)
      return false;
    auto SCI = dyn_cast<ConstantInt>(SO->getOperand(1));
    if (!SCI || SCI->getZExtValue() != 3)
      return false;
    auto LO = dyn_cast<LShrOperator>(SO->getOperand(0));
    if (!LO)
      return false;
    auto LCI = dyn_cast<ConstantInt>(LO->getOperand(1));
    if (!LCI || LCI->getZExtValue() != 1)
      return false;
    return LO->getOperand(0) == N;
  };

  //
  // Return 'true' if 'V' is a byte-flattened GEP returning the address of the
  // "middle" element of the array 'A' of length 'N'.
  //
  auto IsMiddleAddress = [&IsMiddleOffset](Value *V, Value *A,
                                           Value *N) -> bool {
    auto GEPI = dyn_cast<GetElementPtrInst>(V);
    if (!GEPI || GEPI->getNumOperands() != 2)
      return false;
    if (GEPI->getPointerOperand() != A)
      return false;
    if (!IsMiddleOffset(GEPI->getOperand(1), N))
      return false;
    return true;
  };

  //
  // Return 'true' if 'V' represents the offset into last element of an array
  // of length 'N'.
  //
  auto IsHigherOffset = [](Value *V, Value *N) -> bool {
    auto AO = dyn_cast<BinaryOperator>(V);
    if (!AO || AO->getOpcode() != Instruction::Add)
      return false;
    auto ACI = dyn_cast<ConstantInt>(AO->getOperand(1));
    if (!ACI || ACI->getSExtValue() != -8)
      return false;
    auto SO = dyn_cast<ShlOperator>(AO->getOperand(0));
    if (!SO)
      return false;
    auto SCI = dyn_cast<ConstantInt>(SO->getOperand(1));
    if (!SCI || SCI->getZExtValue() != 3)
      return false;
    return SO->getOperand(0) == N;
  };

  //
  // Return 'true' if 'V' is a byte-flattened GEP returning the address of the
  // last element of the array 'A' of length 'N'.
  //
  auto IsHigherAddress = [&IsHigherOffset](Value *V, Value *A,
                                           Value *N) -> bool {
    auto GEPI = dyn_cast<GetElementPtrInst>(V);
    if (!GEPI || GEPI->getNumOperands() != 2)
      return false;
    if (GEPI->getPointerOperand() != A)
      return false;
    if (!IsHigherOffset(GEPI->getOperand(1), N))
      return false;
    return true;
  };

  //
  // Return 'true' if 'V' represents 'D' == ('N'/8)*8, which is 'N' rounded
  // down to the nearest multiple of 8. (This is useful because the addresses
  // of the array elements of the insertion sort we are recognizing are
  // multiples of 8.)
  //
  auto IsPlusD = [](Value *V, Value *N) -> bool {
    auto AO = dyn_cast<BinaryOperator>(V);
    if (!AO || AO->getOpcode() != Instruction::And)
      return false;
    auto ACI = dyn_cast<ConstantInt>(AO->getOperand(1));
    if (!ACI || ACI->getSExtValue() != -8)
      return false;
    return AO->getOperand(0) == N;
  };

  //
  // Return 'true' if 'V' represents 2*D == 2*('N'/8)*8.
  //
  auto IsPlus2D = [&IsPlusD](Value *V, Value *N) -> bool {
    auto SO = dyn_cast<ShlOperator>(V);
    if (!SO)
      return false;
    auto SCI = dyn_cast<ConstantInt>(SO->getOperand(1));
    if (!SCI || SCI->getZExtValue() != 1)
      return false;
    return IsPlusD(SO->getOperand(0), N);
  };

  //
  // Return 'true' if 'V' represents -D == -('N'/8)*8.
  //
  auto IsMinusD = [&IsPlusD](Value *V, Value *N) -> bool {
    auto AO = dyn_cast<BinaryOperator>(V);
    if (!AO || AO->getOpcode() != Instruction::Sub)
      return false;
    auto ACI = dyn_cast<ConstantInt>(AO->getOperand(0));
    if (!ACI || !ACI->isZeroValue())
      return false;
    return IsPlusD(AO->getOperand(1), N);
  };

  //
  // Return 'true' if 'V' represents -2*D == -2*('N'/8)*8.
  //
  auto IsMinus2D = [&IsPlus2D](Value *V, Value *N) -> bool {
    auto AO = dyn_cast<BinaryOperator>(V);
    if (!AO || AO->getOpcode() != Instruction::Sub)
      return false;
    auto ACI = dyn_cast<ConstantInt>(AO->getOperand(0));
    if (!ACI || !ACI->isZeroValue())
      return false;
    return IsPlus2D(AO->getOperand(1), N);
  };

  // The set of pivot values which we want to determine are valid indices into
  // the 'ArgArray' of size 'ArgSize'.
  SmallPtrSet<Value *, 8> PivotSet;
  // The BasicBlocks to which the members of the PivotSet belong.
  SmallPtrSet<BasicBlock *, 8> BBPivotSet;
  // The PHINodes and SelectInsts that are traversed while determining the
  // set of pivot values.
  SmallPtrSet<Value *, 8> JoinSet;
  // Find the PHINode that represents 'ArrayArray' in the main loop of the
  // qsort. The qsort has a main loop because tail recursion elimination has
  // been used to eliminate one of its recursive calls.
  Value *A = findPHINodeArgument(BBStart, ArgArray);
  if (!A)
    return nullptr;
  // Find the PHINode that represents 'ArraySize' in the main loop of the
  // qsort.
  Value *N = findPHINodeArgument(BBStart, ArgSize);
  if (!N)
    return nullptr;
  // Find a candidate for the pivot in the qsort.
  PHINode *PivotBase = FindPivotCandidate(BBStart);
  if (!PivotBase)
    return nullptr;
  // Trace PHINodes and SelectInsts to find pivot values to test.  Each pivot
  // value must be the address of an element in the array beeing sorted.
  MakePivotList(A, PivotBase, &PivotSet, &JoinSet, &BBPivotSet);
  // Make sure that the basic blocks in which the pivot elements appear do not
  // store to memory.  This ensures that the pivot value computation is "pure".
  if (!ValidateBBsForPivots(&BBPivotSet))
    return nullptr;
  // Iterate through the pivot values, checking that each represents a valid
  // array address.
  Value *MidBase = nullptr;
  Value *HighBase = nullptr;
  for (auto Pivot : PivotSet) {
    LLVM_DEBUG(dbgs() << "QsortRec: Check: " << *Pivot << "\n");
    // The array address itself is a valid pivot value.
    if (Pivot == A)
      continue;
    // All other pivot values are expected to be byte flattened GEPs, with a
    // 'Base' and offset 'V'.
    auto GEPI = dyn_cast<GetElementPtrInst>(Pivot);
    if (!GEPI || GEPI->getNumOperands() != 2 || !GEPI->isInBounds()) {
      LLVM_DEBUG(dbgs() << "QsortRec: Bad Pivot: " << *Pivot << "\n");
      return nullptr;
    }
    Value *Base = GEPI->getPointerOperand();
    Value *V = GEPI->getOperand(1);
    if (Base == A) {
      // The 'Base' is the array address. Check for the following valid pivot
      // values:
      //   (1) The index of the "middle" value: 8*(N/2)
      //   (2) The index of the last value: 8*N-8
      //   (3) D or 2*D, where D == N/8*8.
      // Record (1) or (2) if we see either of them, as they may be used as the
      // base for other pivot values.
      if (IsMiddleOffset(V, N)) {
        MidBase = GEPI;
      } else if (IsHigherOffset(V, N)) {
        HighBase = GEPI;
      } else if (!IsPlus2D(V, N) && !IsPlusD(V, N)) {
        LLVM_DEBUG(dbgs() << "QsortRec: Bad Pivot: " << *Pivot << "\n");
        return nullptr;
      }
    } else if (Base == MidBase || IsMiddleAddress(Base, A, N)) {
      // The 'Base' is the index of the "middle" value. Check for -D and D,
      // which are indices of valid pivot values.
      MidBase = Base;
      if (!IsMinusD(V, N) && !IsPlusD(V, N)) {
        LLVM_DEBUG(dbgs() << "QsortRec: Bad Pivot: " << *Pivot << "\n");
        return nullptr;
      }
    } else if (Base == HighBase || IsHigherAddress(Base, A, N)) {
      // The 'Base' is the index of the last value. Check for the -2*D and -D,
      // which are indices of valid pivot values.
      HighBase = Base;
      if (!IsMinus2D(V, N) && !IsMinusD(V, N)) {
        LLVM_DEBUG(dbgs() << "QsortRec: Bad Pivot: " << *Pivot << "\n");
        return nullptr;
      }
    }
  }
  LLVM_DEBUG(dbgs() << "QsortRec: " << BBStart->getParent()->getName()
                    << " passed pivot test\n");
  return PivotBase;
}

// Return the number of MIN operations computed in the input function.
// This function is looking for the following:
//
//    %282 = ptrtoint i8* %229 to i64
//    %283 = ptrtoint i8* %230 to i64
//    %284 = sub i64 %282, %283
//    %285 = ptrtoint i8* %233 to i64
//    %286 = sub i64 %285, %282
//    %287 = add i64 %286, -8
//    %288 = icmp slt i64 %284, %287
//    %289 = select i1 %288, i64 %284, i64 %287
//    %290 = icmp eq i64 %289, 0
//
// It will look for the Select instruction (%289) and will identify the
// source PtrToInt instructions for the operands %284 and %287. These are
// %282, %283 and %285. Then it will check that these instructions actually
// point to the input ArgArray and they use the ArgSize to access an
// element in a GEP.
//
// The input vector (SizesVector) will be used to store the operands of the
// Select instructions found. We are going to use them later in the process
// that counts the recursions. This is because the operands in the Select
// instructions represent the new sizes that will be used in the recursions.
static unsigned countMinComputations(Function *F, Argument *ArgArray,
                             SetVector<Value *> &PointersToArgs,
                             SmallVector<Value*, 2> &SizesVect) {

  if (!F)
    return 0;

  // Go through the input basic block and find the Select instruction.
  // The Block must have one select instruction, else return nullptr.
  auto FindSelectInst = [](BasicBlock &BB) {
    SelectInst *SelInst = nullptr;

    for (auto &Inst : BB) {
      SelectInst *TempSelInst = dyn_cast<SelectInst>(&Inst);
      if (TempSelInst) {
        if (SelInst) {
          SelInst = nullptr;
          break;
        }
        else {
          SelInst = TempSelInst;
        }
      }
    }
    return SelInst;
  };

  // Check that the Select instruction comes from comparing a "less than"
  // operation between the two operands. For example:
  //
  //   %288 = icmp slt i64 %284, %287
  //   %289 = select i1 %288, i64 %284, i64 %287
  //   %290 = icmp eq i64 %289, 0
  //
  // The input select instruction will be %289. The following function will
  // check if the operands of the select instruction (%284 and %287) are
  // used in the same icmp instruction of %288. This basically means a
  // computation of minimum (if %284 is lower than %287 than choose it,
  // else take %287). Then, check if the selected instruction is compared
  // against 0 (%290).
  auto IsValidSelect = [](SelectInst *SelInst) {

    // Check that the operand 0 is compared using an ICmp whose predicate
    // is "less than" (%288)
    ICmpInst *Cmpr = dyn_cast<ICmpInst>(SelInst->getOperand(0));
    if (!Cmpr || Cmpr->getPredicate() != ICmpInst::ICMP_SLT)
      return false;

    // The operands of the ICmp (%288) must be the same as the
    // Select instruction (%284 and %287)
    if (Cmpr->getOperand(0) != SelInst->getOperand(1) &&
        Cmpr->getOperand(1) != SelInst->getOperand(2))
      return false;

    // The next instruction is an ICmp checking if the selected
    // value is 0 (%290)
    Instruction *CmpSel = dyn_cast_or_null<Instruction>(
        SelInst->getNextNonDebugInstruction());
    if (!CmpSel)
      return false;

    ICmpInst::Predicate Pred;
    Value *ICmpLHS = nullptr;
    if (!match(CmpSel, m_ICmp(Pred, m_Value(ICmpLHS), m_Zero())))
      return false;

    return ICmpLHS == SelInst;
  };

  // TODO: For now this function is for testing only. It should be removed
  // or replaced once the analysis to collect PointersToArgs is done.
  //
  // Check that all the operands in the Select instruction are
  // pointer arithmetic. Also, collect the PtrToInt instructions
  // related to the pointer arithmetic, later we will prove
  // that they refer to the array that is being sorted.
  // Basically, we are looking for this:
  //
  //   %282 = ptrtoint i8* %229 to i64
  //   %283 = ptrtoint i8* %230 to i64
  //   %284 = sub i64 %282, %283
  //   %285 = ptrtoint i8* %233 to i64
  //   %286 = sub i64 %285, %282
  //   %287 = add i64 %286, -8
  //   %288 = icmp slt i64 %284, %287
  //   %289 = select i1 %288, i64 %284, i64 %287
  //
  // The operands 1 and 2 in %289 are %284 and %287. %284 is pointer
  // arithmetic between %282 and %283. Then %287 is an add, but %286 is
  // another pointer arithmetic between %285 and %282. SetPtr will
  // collect %282, %283 and %285.
  auto CollectPtrsToArgs = [](SelectInst *SelInst,
                              SetVector<PtrToIntInst *> &SetPtr) {

    unsigned NumOperands = SelInst->getNumOperands();

    for (unsigned CurrOperand = 1; CurrOperand < NumOperands; CurrOperand++) {
      Instruction *Inst =
          dyn_cast<Instruction>(SelInst->getOperand(CurrOperand));

      if (!Inst) {
        SetPtr.clear();
        return;
      }

      // The add is just basically moving the pointer calculated
      if (Inst->getOpcode() == Instruction::Add) {
        // Check that we are moving -8 (the -8 comes from the argument
        // alignment)
        ConstantInt *Const = dyn_cast<ConstantInt>(Inst->getOperand(1));
        if (!Const || Const->getSExtValue() != -8) {
          SetPtr.clear();
          return;
        }

        Inst = dyn_cast<Instruction>(Inst->getOperand(0));
        if (!Inst) {
          SetPtr.clear();
          return;
        }
      }

      // Check the subtraction. All the operands must be PtrToInt, or
      // a PHINode where the incoming values are PtrToInt instructions.
      if (Inst->getOpcode() == Instruction::Sub) {
        // Binary operations has only 2 operands
        for (unsigned Operand = 0; Operand < 2; Operand++) {
          if (PtrToIntInst *CurrPtr =
              dyn_cast<PtrToIntInst>(Inst->getOperand(Operand))) {
            SetPtr.insert(CurrPtr);
          }

          else if (PHINode *PHI =
              dyn_cast<PHINode>(Inst->getOperand(Operand))) {

            unsigned NumIncomingValues = PHI->getNumIncomingValues();
            for (unsigned Entry = 0; Entry < NumIncomingValues; Entry++) {
              PtrToIntInst *CurrPtr =
                  dyn_cast<PtrToIntInst>(PHI->getIncomingValue(Entry));
              if (!CurrPtr) {
                SetPtr.clear();
                return;
              }
              SetPtr.insert(CurrPtr);
            }
          } else {
            SetPtr.clear();
            return;
          }
        }
      }
      // Something else is going on.
      else {
        SetPtr.clear();
        return;
      }
    }
  };

  // TODO: For now this function is for testing only. It should be removed
  // once we implement the process to identify the pointers to arg.
  // All PtrToIntInst used in the SelectInst must refer to the beginning of
  // the array that we are sorting and must use the size to refer data in
  // the array. Basically, it will use the array and the size to compute
  // the MIN.
  auto PtrToIntInstRefersToArgs = [ArgArray](
      SetVector<PtrToIntInst *> &SetPtr) {
    if (SetPtr.empty())
      return false;

    Value *ArgArrValue = cast<Value>(ArgArray);

    for (auto PtrInst : SetPtr) {
      // If the pointer points directly to the argument then it must be
      // the array.
      if (Argument *Arg = dyn_cast<Argument>(PtrInst->getOperand(0))) {
        if (Arg != ArgArray)
          return false;
      }

      // Else, the pointer must get information from the array at position
      // size
      else if (!ArgUtil.valueRefersToArg(PtrInst, ArgArrValue))
        return false;
    }

    return true;
  };

  // Return true if the input value is used in a direct call to F.
  // We will finalize the analysis during the process that count the
  // numbers of recursive call
  auto CheckForDirectCall = [F](Value *Operand) {
    if (!Operand)
      return false;

    // There is only one user for Operand in this case
    if (!Operand->hasOneUse())
      return false;

    CallBase *RecCall = dyn_cast<CallBase>(Operand->user_back());

    if (!RecCall || RecCall->getCalledFunction() != F)
      return false;

    return true;
  };

  // Return true if the User of the Value will be a PHINode.
  // We will finalize the analysis during the process that counts the
  // number of recursive calls.
  auto CheckForTailRecursion = [F](Value *Operand) {
    if (!Operand)
      return false;

    for (User *User : Operand->users())
      if (isa<PHINode>(User))
        return true;

    return false;
  };

  // Return 0 if the input operand will be used in a the direct recursive call.
  // Return 1 if the input operand will be used in a PHI Node (tail recursion).
  // Return -1 if nothing was found.
  auto OperandUsedInRecursion =
      [CheckForDirectCall, CheckForTailRecursion](Value *Operand) {
    if (!Operand)
      return -1;

    for (User *User : Operand->users()) {

      Value *Val = cast<Value>(User);
      Value *TempOperand = nullptr;
      ConstantInt *Const = nullptr;
      if (!match(Val, m_LShr(m_Value(TempOperand), m_ConstantInt(Const))) ||
        Const->getZExtValue() != 3)
        continue;

      if (CheckForDirectCall(Val))
        return 0;

      if (CheckForTailRecursion(Val))
        return 1;
    }

    return -1;
  };

  unsigned MINCounter = 0;

  // TODO: This is only for testing process. Once the whole qsort
  // identifier is completed, this part needs to be removed since
  // PointersToArgs must not be an empty set. The following part
  // will identify the pointers used in the Select instruction found.
  bool TestingEnabled = PointersToArgs.size() == 0 && QsortTestMin;
  if (PointersToArgs.size() == 0 && !TestingEnabled)
    return false;

  Value *DirectRecCallSize = nullptr;
  Value *TailRecCallSize = nullptr;

  // Go through each of the basic blocks in the function and check
  // if there is a computation of MIN
  for (auto &BB : *F) {

    if (!isa<PtrToIntInst>(BB.getFirstNonPHIOrDbg()))
      continue;

    if (SelectInst *SelInst = FindSelectInst(BB)) {
      if (IsValidSelect(SelInst)) {

        SetVector<PtrToIntInst *> SetPtr;

        // TODO: This is only for testing process. This part should
        // be removed and must be replaced with an analysis which proves
        // that the operands of the Select instructions come from a pointer
        // arithmetic operation that uses the Values in PointersToArg.
        if (TestingEnabled) {
          CollectPtrsToArgs(SelInst, SetPtr);

          if (SetPtr.empty())
            continue;
        } else {
          return 0;
        }

        LLVM_DEBUG({
          dbgs() << "QsortRec: Checking computation of MIN in "
                 << F->getName() <<"\n";
          BB.dump();
        });

        // TODO: This function will be removed once we implement the process
        // that collects all the pointers to the arg. For now it is just used
        // as a testing mechanisim to make sure we collected the correct
        // values in the test case.
        // All the pointers must refer to the array that is being sorted
        if (!PtrToIntInstRefersToArgs(SetPtr)) {
          LLVM_DEBUG(dbgs() << "QsortRec: Computation of MIN in "
                            << F->getName() << " FAILED Test.\n");
          continue;
        }

        // If we need to catch the recursive calls then collect the possible
        // sizes
        if (QsortTestRecursion) {

          int Op1Result = OperandUsedInRecursion(SelInst->getOperand(1));
          int Op2Result = OperandUsedInRecursion(SelInst->getOperand(2));

          // We shouldn't be able to catch any other size used in a recursive
          // call once all sizes are set
          if (((Op1Result == 0 || Op2Result == 0) && DirectRecCallSize) ||
              ((Op1Result == 1 || Op2Result == 1) && TailRecCallSize)) {
            LLVM_DEBUG(dbgs() << "QsortRec: Computation of MIN in "
                              << F->getName() << " FAILED Test.\n");
            return 0;
          }

          // Direct call site gets the size from operand 2
          if (!DirectRecCallSize && Op2Result == 0)
              DirectRecCallSize = SelInst->getOperand(2);

          // Tail recursion gets the size from operand 1
          else if (!TailRecCallSize && Op1Result == 1)
              TailRecCallSize = SelInst->getOperand(1);

          if (!DirectRecCallSize && !TailRecCallSize) {
            LLVM_DEBUG(dbgs() << "QsortRec: Computation of MIN in "
                              << F->getName() << " FAILED Test.\n");
            continue;
          }
        }
        LLVM_DEBUG(dbgs() << "QsortRec: Computation of MIN in "
                          << F->getName() << " PASSED Test.\n");

        MINCounter++;
      }
    }

    if (MINCounter > 2)
      return 0;
  }

  if (QsortTestRecursion) {
    if (!DirectRecCallSize || !TailRecCallSize) {
      LLVM_DEBUG(dbgs() << "QsortRec: Computation of MIN in "
                        << F->getName() << " FAILED Test.\n");
      return 0;
    }

    SizesVect.push_back(DirectRecCallSize);
    SizesVect.push_back(TailRecCallSize);
  }

  return MINCounter;
}

// Return the number of data swaps that are occurring. This function
// looks for the following:
//
//   %300 = phi i64* [ %298, %291 ], [ %306, %299 ]
//   %301 = phi i64* [ %297, %291 ], [ %305, %299 ]
//   %303 = load i64, i64* %301, align 8, !tbaa !29
//   %304 = load i64, i64* %300, align 8, !tbaa !29
//   store i64 %304, i64* %301, align 8, !tbaa !29
//   store i64 %303, i64* %300, align 8, !tbaa !29
//
// %303 loads from %301 and %304 loads from %300. Then %303 is stored
// in %300 and %304 is stored in %301. Basically there is a data swap
// between %300 and %301. Also, it checks that %304 and %303 come from
// the argument (Arg).
static unsigned countSwapComputations(Function *F, Argument *Arg) {

  if (!F || !Arg)
    return false;

  // Collect the stores that are doing the swapping
  auto FindSwapStores = [](SmallVector<StoreInst *, 2> &StoresVect,
                           BasicBlock &BB) {

    SmallVector<LoadInst *, 2> LoadsVect;

    // Find the loads
    for (auto &Inst : BB) {

      // All loads must happen before the stores
      if (LoadsVect.size() < 2 && isa<StoreInst>(Inst))
        return false;

      LoadInst *LdInst = dyn_cast<LoadInst>(&Inst);

      // The only user for the load will be a store
      if (!LdInst)
        continue;

      if (!LdInst->hasOneUse())
        return false;

      StoreInst *StrInst = dyn_cast<StoreInst>(LdInst->user_back());

      if (!StrInst)
        continue;

      if (LoadsVect.size() == 2 || StoresVect.size() == 2)
        return false;

      LoadsVect.push_back(LdInst);
      StoresVect.push_back(StrInst);
    }

    if (LoadsVect.size() != 2 || StoresVect.size() != 2)
      return false;

    Value *FirstPtr = LoadsVect[0]->getOperand(0);
    Value *SecondPtr = LoadsVect[1]->getOperand(0);

    // The values in the loads come from a PHINode used in the iteration
    if (!isa<PHINode>(FirstPtr) || !isa<PHINode>(SecondPtr))
      return false;

    // Data is being swapped
    return (StoresVect[0]->getOperand(1) == SecondPtr &&
            StoresVect[1]->getOperand(1) == FirstPtr);
  };

  unsigned SwapCounter = 0;
  Value *ArgValue = cast<Value>(Arg);

  // Go through each of the basic blocks in the function and check
  // if there is a swap
  for (auto &BB : *F) {

    if (!isa<PHINode>(&(BB.front())))
      continue;

    SmallVector<StoreInst *, 2> StoreVect;

    if (!FindSwapStores(StoreVect, BB))
      continue;

    LLVM_DEBUG({
      dbgs() << "QsortRec: Checking computation of swap in "
             << F->getName() <<"\n";
      BB.dump();
    });

    // The values that are swapped must come from the same argument
    Value *FirstVal = StoreVect[0]->getOperand(0);
    Value *SecondVal = StoreVect[1]->getOperand(0);

    if (ArgUtil.valueRefersToArg(FirstVal, ArgValue) &&
        ArgUtil.valueRefersToArg(SecondVal, ArgValue)) {
      SwapCounter++;

      LLVM_DEBUG(dbgs() << "QsortRec: Computation of swap in "
                 << F->getName() << " PASSED Test.\n");
    }
    else {
      LLVM_DEBUG(dbgs() << "QsortRec: Computation of swap in "
                        << F->getName() << " FAILED Test.\n");
    }
  }

  return SwapCounter;
}

// Return the number of recursions that could happen in the
// input function. This function checks both forms of recursion:
// direct call and implicit recursion. The direct call is found
// when the function calls itself and uses the same formal
// argument as an actual argument in the call site. The implicit
// recursion happens when there is a basic block that jumps to
// the same block as the entry block, and moves the data in the
// input argument. The recursions will use the new sizes (SizesVector)
// as actual arguments. We already proved that these new sizes are
// related to the array argument and size argument during the computaion
// of MIN operations.
static unsigned countRecursions(Function *F, Argument *ArgArr,
                                Argument *ArgSize,
                                SmallVector<Value *, 2> &SizesVector) {

  if (!F || !ArgArr || SizesVector.size() != 2)
    return 0;

  // The new size is a lshr operation over one of the Values we computed
  // before. Return true if the input Value is one of these sizes.
  auto IsNewInputSize = [SizesVector](Value *Val, bool IsDirectCall) {
    if (!Val)
      return false;

    Value *NewSize = nullptr;
    ConstantInt *Const = nullptr;
    if (!match(Val, m_LShr(m_Value(NewSize), m_ConstantInt(Const))) ||
        Const->getZExtValue() != 3)
      return false;

    // If it is a direct call, then the recursion must match entry 0
    if (IsDirectCall)
      return NewSize == SizesVector[0];

    return NewSize == SizesVector[1];

  };

  // Return the call site that produces the recursive call
  auto FindRecursiveCall = [F, ArgArr]() {
    CallBase *RecursiveCall = nullptr;

    for (User *User : F->users()) {
      CallBase *CurrCall = dyn_cast<CallBase>(User);

      if (!CurrCall)
        continue;

      Function *Caller = CurrCall->getCaller();
      // There should be only one recursive call
      if (F == Caller) {
        if (RecursiveCall) {
          RecursiveCall = nullptr;
          break;
        }
        RecursiveCall = CurrCall;
      }
    }

    return RecursiveCall;
  };

  // Return the incoming Value from a PHI Node that is the opposite
  // of the input Argument. For example:
  //
  //   Input: %1
  //   PHINode: %6 = phi i64 [ %1, %2 ], [ %318, %315 ]
  //
  //   Return: %318
  auto GetIncomingValueFromPHI = [](PHINode *PHI, Argument *Arg) {

    Value *IncomingVal = nullptr;

    if (!Arg)
      return IncomingVal;

    if (PHI->getNumIncomingValues() != 2)
      return IncomingVal;

    Value *ArgVal = cast<Value>(Arg);

    if (PHI->getIncomingValue(0) != ArgVal)
      IncomingVal = PHI->getIncomingValue(0);

    else if (PHI->getIncomingValue(1) != ArgVal)
      IncomingVal = PHI->getIncomingValue(1);

    return IncomingVal;
  };

  // Find the instructions that represent the actual arguments in the
  // tail recursion
  auto FindTailRecursion = [F, ArgArr, ArgSize, GetIncomingValueFromPHI](
                           SmallVector<Value *, 2> &ArgsCollected) {

    // Actual arguments
    Value *ActualArgArr = nullptr;
    Value *ActualArgSize = nullptr;

    // Find the entry basic block
    BasicBlock &EntryBB = F->getEntryBlock();
    BranchInst *EntryBranch =
      dyn_cast_or_null<BranchInst>(EntryBB.getTerminator());

    if (!EntryBranch)
      return;

    // All successors to the entry basic block will have the
    // same predecessors: the entry block and the block that
    // produces the new arguments for the tail recursion
    for (auto SuccBB : EntryBranch->successors()) {
      if (!SuccBB->hasNPredecessors(2))
        return;

      PHINode *PHIArgArr =
          dyn_cast<PHINode>(findPHINodeArgument(SuccBB, ArgArr));

      if (!PHIArgArr)
        return;

      PHINode *PHIArgSize =
          dyn_cast<PHINode>(findPHINodeArgument(SuccBB, ArgSize));

      if (!PHIArgSize)
        return;

      // The values for the array and the size are in a PHI node
      Value *TempActualArr = GetIncomingValueFromPHI(PHIArgArr, ArgArr);
      Value *TempActualSize = GetIncomingValueFromPHI(PHIArgSize, ArgSize);

      // Nothing was found
      if (!TempActualArr || !TempActualSize)
        return;

      // The temporary actual array must be the same as the actual array
      if (ActualArgArr && ActualArgArr != TempActualArr)
        return;

      // The temporary actual size must be the same as the actual array
      if (ActualArgSize && ActualArgSize != TempActualSize)
        return;

      ActualArgSize = TempActualSize;
      ActualArgArr = TempActualArr;
    }

    if (!ActualArgSize || !ActualArgArr)
      return;

    Instruction *ArrInst = dyn_cast<Instruction>(ActualArgArr);
    Instruction *SizeInst = dyn_cast<Instruction>(ActualArgSize);

    if (!ArrInst || !SizeInst)
      return;

    // Basic block that produces the tail recursion
    BasicBlock *TailBlock = ArrInst->getParent();

    // Both actual arguments must come from the same basic block
    if (TailBlock != SizeInst->getParent())
      return;

    BranchInst *TailBranch = dyn_cast<BranchInst>(TailBlock->getTerminator());

    // The block that produces the tail recursion must have the same number
    // of successors as the entry block
    if (!TailBranch ||
        TailBranch->getNumSuccessors() != EntryBranch->getNumSuccessors())
      return;

    // All successors of the tail branch must be the same as the
    // entry block
    for (auto TailSucc : TailBranch->successors())
      if (TailSucc != EntryBranch->getSuccessor(0) &&
          TailSucc != EntryBranch->getSuccessor(1))
        return;

    ArgsCollected.push_back(ActualArgArr);
    ArgsCollected.push_back(ActualArgSize);
  };

  BasicBlock &EntryBB = F->getEntryBlock();
  BranchInst *EntryBranch =
      dyn_cast_or_null<BranchInst>(EntryBB.getTerminator());

  if (!EntryBranch || EntryBranch->getNumSuccessors() != 2)
    return 0;

  // One recursion is in form of callsite
  CallBase *RecursiveCall = FindRecursiveCall();

  if (!RecursiveCall)
    return 0;

  unsigned ArgArrNo = ArgArr->getArgNo();
  unsigned ArgSizeNo = ArgSize->getArgNo();
  unsigned NumRecursions = 0;

  // The actual arguments for the recursive call site must point to the
  // formal arguments.
  Value *ActualArgArr = RecursiveCall->getArgOperand(ArgArrNo);
  Value *ActualArgSize = RecursiveCall->getArgOperand(ArgSizeNo);
  Value *ArgValue = cast<Value>(ArgArr);

  LLVM_DEBUG({
    dbgs() << "QsortRec: Recursion candidate in " << F->getName() << "\n\n"
           << *RecursiveCall << "\n\n";
  });

  // Check the formal argument against the actual
  if (!ArgUtil.valueRefersToArg(ActualArgArr, ArgValue) ||
      !IsNewInputSize(ActualArgSize, true /* IsDirectCall */)) {
    LLVM_DEBUG(dbgs() << "QsortRec: Recursions in "
                      << F->getName() << " FAILED Test.\n");
    return 0;
  }

  LLVM_DEBUG(dbgs() << "QsortRec: Recursions in "
                    << F->getName() << " PASSED Test.\n");

  NumRecursions++;

  ActualArgArr = nullptr;
  ActualArgSize = nullptr;

  SmallVector<Value *, 2> ArgsCollected;

  FindTailRecursion(ArgsCollected);

  if (ArgsCollected.size() != 2)
    return NumRecursions;

  ActualArgArr = ArgsCollected[0];
  ActualArgSize = ArgsCollected[1];

  if (ActualArgArr && ActualArgSize) {

    LLVM_DEBUG({
        dbgs() << "QsortRec: Recursion candidate in " << F->getName();
        Instruction *ArgInst = cast<Instruction>(ActualArgArr);
        ArgInst->getParent()->dump();
    });

    // ActualArgArr must point to the input array and
    // ActualArgSize must be one of the new sizes
    if (ArgUtil.valueRefersToArg(ActualArgArr, ArgValue) &&
        IsNewInputSize(ActualArgSize, false /* IsDirectCall */)) {
      LLVM_DEBUG(dbgs() << "QsortRec: Recursions in "
                        << F->getName() << " PASSED Test.\n");
      NumRecursions++;
    }
    else {
      LLVM_DEBUG(dbgs() << "QsortRec: Recursions in "
                        << F->getName() << " FAILED Test.\n");
    }
  }

  return NumRecursions;
}

//
// Return 'true' if 'BBStart' begins a series of BasicBlocks which move the
// elements of the array 'ArgArray' on the left or right side of the pivot.
// There are two such "pivot movers" in qsort, (1) one that increments a
// pointer as it adds elements with value less than the pivot to the left,
// and (2) one that decrements a pointer as it adds elements with a value
// greater than the pivot to the right. 'IsUp' is 'true for (1) and 'false'
// for (2).
//
// For example, the code for the type (2) pivot mover looks like:
//
// 194: (BBStart: Initial BasicBlock)                ; preds = %189, %173, %166
//  %195 = phi i32 [ %167, %166 ], [ %176, %173 ], [ %190, %189 ]
//  %196 = phi i8* [ %170, %166 ], [ %175, %173 ], [ %192, %189 ]
//  %197 = phi i8* [ %171, %166 ], [ %174, %173 ], [ %191, %189 ]
//  %198 = icmp ugt i8* %196, %169
//     NOTE: %169 is the "LimitPHI" value, which does not change in the loop
//  br i1 %198, label %227, label %199
//
// 199: (BBLHeader: Loop Header)                     ; preds = %194, %215
//  %200 = phi i8* [ %218, %215 ], [ %169, %194 ]
//  %201 = phi i8* [ %217, %215 ], [ %168, %194 ]
//  %202 = phi i32 [ %216, %215 ], [ %195, %194 ]
//  %203 = bitcast i8* %200 to %struct.arc**
//  %204 = bitcast i8* %34 to %struct.arc**
//  %205 = tail call i32 @arc_compare.54.83.112(struct.arc** %203,
//     %struct.arc** %204) #12
//  %206 = icmp sgt i32 %205, -1
//  br i1 %206, label %207, label %220
//
// 207: (BBLTest: Loop Test)                         ; preds = %199
//  %208 = icmp eq i32 %205, 0
//  br i1 %208, label %209, label %215
//
// 209: (BBLSwap: Loop Swap of Array Elements)       ; preds = %207
//  %210 = bitcast i8* %200 to i64*
//  %211 = load i64, i64* %210, align 8, !tbaa !7
//  %212 = bitcast i8* %201 to i64*
//  %213 = load i64, i64* %212, align 8, !tbaa !7
//  store i64 %213, i64* %210, align 8, !tbaa !7
//  store i64 %211, i64* %212, align 8, !tbaa !7
//  %214 = getelementptr inbounds i8, i8* %201, i64 -8, !intel-tbaa !4
//    NOTE: %201 is the "InnerPHI" value
//  br label %215
//
// 215: (BBLLatch: Loop Latch)                       ; preds = %209, %207
//  %216 = phi i32 [ 1, %209 ], [ %202, %207 ]
//  %217 = phi i8* [ %214, %209 ], [ %201, %207 ]
//  %218 = getelementptr inbounds i8, i8* %200, i64 -8, !intel-tbaa !4
//    NOTE: %200 is the "OuterPHI" value
//  %219 = icmp ugt i8* %196, %218
//  br i1 %219, label %227, label %199
//    ...
// 220: (BBInnerExit: Inner Exit from the Pivit Mover)
//    ...
// 227: (BBOuterExit: Outer Exit from the Pivit Mover)
//    ...
//
static bool isPivotMover(BasicBlock *BBStart, Argument *ArgArray, bool IsUp) {

  //
  // Validate 'BBStart', the initial BasicBlock which tests '*OuterPHI'
  // against '*LimitPHI'. Recognize and set '*OuterPHI', '*LimitPHI',
  // and the true and false successors of 'BBStart', which are '**BBOuterExit'
  // and '**BBLHeader'.
  //
  auto ValidateBBStart = [](BasicBlock *BBStart, bool IsUp,
                            PHINode **OuterPHI, PHINode **LimitPHI,
                            BasicBlock **BBOuterExit,
                            BasicBlock **BBLHeader) -> bool {
    auto BI = dyn_cast_or_null<BranchInst>(BBStart->getTerminator());
    if (!BI || BI->isUnconditional())
      return false;
    *BBOuterExit = BI->getSuccessor(0);
    *BBLHeader = BI->getSuccessor(1);
    auto IC = dyn_cast<ICmpInst>(BI->getCondition());
    if (!IC || IC->getPredicate() != CmpInst::ICMP_UGT)
      return false;
    auto PHIN0 = dyn_cast<PHINode>(IC->getOperand(0));
    if (!PHIN0)
      return false;
    auto PHIN1 = dyn_cast<PHINode>(IC->getOperand(1));
    if (!PHIN1)
      return false;
    *LimitPHI = IsUp ? PHIN1 : PHIN0;
    PHINode *OuterPHIBase = IsUp ? PHIN0 : PHIN1;
    *OuterPHI = nullptr;
    for (User *U : OuterPHIBase->users()) {
      auto PHIT = dyn_cast<PHINode>(U);
      if (!PHIT || PHIT->getParent() != *BBLHeader)
        continue;
      if (*OuterPHI)
        return false;
      *OuterPHI = PHIT;
    }
    if (!*OuterPHI)
      return false;
    return true;
  };

  //
  // Validate 'BBLHeader', which is loop header for the pivot mover loop.
  // Here the values of the elements at 'OuterPHI' and 'ArgArray' are
  // compared using the '*CIOut' comparison function, which is set. The
  // true and false successors of 'BBLHeader' which are '*BBLTest' and
  // '*BBInnerExit' are also recognized and set. (Note that the pivot
  // element is the beginning of 'ArgArray' at this point in time.)
  //
  auto ValidateBBLHeader = [](BasicBlock *BBLHeader, bool IsUp,
                              PHINode *OuterPHI, Argument *ArgArray,
                              BasicBlock **BBLTest, BasicBlock **BBInnerExit,
                              CallInst **CIOut) -> bool {
    auto BI = dyn_cast_or_null<BranchInst>(BBLHeader->getTerminator());
    if (!BI || BI->isUnconditional())
      return false;
    *BBLTest = BI->getSuccessor(0);
    *BBInnerExit = BI->getSuccessor(1);
    auto IC = dyn_cast<ICmpInst>(BI->getCondition());
    auto CP = IsUp ? CmpInst::ICMP_SLT : CmpInst::ICMP_SGT;
    if (!IC || IC->getPredicate() != CP)
      return false;
    int64_t CV = IsUp ? 1 : -1;
    auto CI = dyn_cast<ConstantInt>(IC->getOperand(1));
    if (!CI || CI->getSExtValue() != CV)
      return false;
    auto CB = dyn_cast<CallInst>(IC->getOperand(0));
    if (!CB || CB->getNumArgOperands() != 2)
      return false;
    auto BC0 = dyn_cast<BitCastInst>(CB->getArgOperand(0));
    if (!BC0 || BC0->getOperand(0) != OuterPHI)
      return false;
    auto BC1 = dyn_cast<BitCastInst>(CB->getArgOperand(1));
    if (!BC1)
      return false;
    if (!isPHINodeWithArgIncomingValue(BC1->getOperand(0), ArgArray))
      return false;
    *CIOut = CB;
    return true;
  };

  //
  // Validate 'BBLTest', the test block of the pivot mover loop, which
  // tests whether the array elements should be swapped. 'CIIn' the result
  // of the comparison of the array elements. The true and false successors
  // of 'BBLTest' are recognized and set as '*BBLSwap' and '*BBLLatch'.
  //
  auto ValidateBBLTest = [](BasicBlock *BBLTest, CallInst *CIIn,
                            BasicBlock **BBLSwap, BasicBlock **BBLLatch)
                            -> bool {
    auto BI = dyn_cast_or_null<BranchInst>(BBLTest->getTerminator());
    if (!BI || BI->isUnconditional())
      return false;
    *BBLSwap = BI->getSuccessor(0);
    *BBLLatch = BI->getSuccessor(1);
    auto IC = dyn_cast<ICmpInst>(BI->getCondition());
    if (!IC || IC->getPredicate() != CmpInst::ICMP_EQ)
      return false;
    if (IC->getOperand(0) != CIIn)
      return false;
    auto CI = dyn_cast<ConstantInt>(IC->getOperand(1));
    if (!CI || !CI->isZero())
      return false;
    return true;
  };

  //
  // Validate 'ValidateBBLSwap', the block which swaps the array elements
  // in the pivot mover loop. Recognize that its single successor is
  // 'BBLLatch' and that it swaps the pivot and the array element at
  // 'OuterPHI' while advancing '*InnerPHI', which is recognized and set.
  //
  auto ValidateBBLSwap = [](BasicBlock *BBLSwap, BasicBlock *BBLLatch,
                            bool IsUp, PHINode *OuterPHI, PHINode **InnerPHI)
                            -> bool {
    auto BI = dyn_cast_or_null<BranchInst>(BBLSwap->getTerminator());
    if (!BI || !BI->isUnconditional())
      return false;
    if (BI->getSuccessor(0) != BBLLatch)
      return false;
    auto BC0 = dyn_cast<BitCastInst>(&BBLSwap->front());
    if (!BC0)
      return false;
    auto PHIN0 = dyn_cast<PHINode>(BC0->getOperand(0));
    if (!PHIN0)
      return false;
    auto L0 = dyn_cast_or_null<LoadInst>(BC0->getNextNonDebugInstruction());
    if (!L0 || L0->getPointerOperand() != BC0)
      return false;
    auto BC1 = dyn_cast_or_null<BitCastInst>(L0->getNextNonDebugInstruction());
    if (!BC1)
      return false;
    auto PHIN1 = dyn_cast<PHINode>(BC1->getOperand(0));
    if (!PHIN1)
      return false;
    auto L1 = dyn_cast_or_null<LoadInst>(BC1->getNextNonDebugInstruction());
    if (!L1 || L1->getPointerOperand() != BC1)
      return false;
    if (IsUp) {
      if (PHIN1 != OuterPHI)
        return false;
      *InnerPHI = PHIN0;
    } else {
      if (PHIN0 != OuterPHI)
        return false;
      *InnerPHI = PHIN1;
    }
    auto S0 = dyn_cast<StoreInst>(L1->getNextNonDebugInstruction());
    if (!S0 || S0->getPointerOperand() != BC0 || S0->getValueOperand() != L1)
      return false;
    auto S1 = dyn_cast<StoreInst>(S0->getNextNonDebugInstruction());
    if (!S1 || S1->getPointerOperand() != BC1 || S1->getValueOperand() != L0)
      return false;
    auto GEP = dyn_cast<GetElementPtrInst>(S1->getNextNonDebugInstruction());
    if (!GEP || GEP->getNumOperands() != 2)
      return false;
    auto PHIN2 = dyn_cast<PHINode>(GEP->getOperand(0));
    if (!PHIN2 || PHIN2 != *InnerPHI)
      return false;
    auto CI = dyn_cast<ConstantInt>(GEP->getOperand(1));
    auto CP = IsUp ? 8 : -8;
    if (!CI || CI->getSExtValue() != CP)
      return false;
    return true;
  };

  //
  // Validate 'ValidateBBLLatch', which is the loop latch block of the pivot
  // mover loop. Recognize that its true and false successors are 'BBOuterExit'
  // and 'BBLHeader' and that it advances 'InnerPHI', while testing it against
  // 'LimitPHI'.
  //
  auto ValidateBBLLatch = [](BasicBlock *BBLLatch, BasicBlock *BBOuterExit,
                             BasicBlock *BBLHeader, bool IsUp,
                             PHINode *OuterPHI, PHINode *LimitPHI) -> bool {
    auto BI = dyn_cast_or_null<BranchInst>(BBLLatch->getTerminator());
    if (!BI || BI->isUnconditional())
      return false;
    if (BI->getSuccessor(0) != BBOuterExit)
      return false;
    if (BI->getSuccessor(1) != BBLHeader)
      return false;
    auto IC = dyn_cast<ICmpInst>(BI->getCondition());
    if (!IC || IC->getPredicate() != CmpInst::ICMP_UGT)
      return false;
    auto GEPIndex = IsUp ? 0 : 1;
    auto PHIIndex = IsUp ? 1 : 0;
    auto CP = IsUp ? 8 : -8;
    auto GEP = dyn_cast<GetElementPtrInst>(IC->getOperand(GEPIndex));
    if (!GEP || GEP->getNumOperands() != 2)
      return false;
    if (GEP->getPointerOperand() != OuterPHI)
      return false;
    auto CI = dyn_cast<ConstantInt>(GEP->getOperand(1));
    if (!CI || CI->getSExtValue() != CP)
      return false;
    if (IC->getOperand(PHIIndex) != LimitPHI)
      return false;
    return true;
  };

  // Main code for isPivotMover().
  BasicBlock *BBLHeader = nullptr;
  BasicBlock *BBLTest = nullptr;
  BasicBlock *BBLSwap = nullptr;
  BasicBlock *BBLLatch = nullptr;
  BasicBlock *BBOuterExit = nullptr;
  BasicBlock *BBInnerExit = nullptr;
  CallInst *CI = nullptr;
  PHINode *OuterPHI = nullptr;
  PHINode *InnerPHI = nullptr;
  PHINode *LimitPHI = nullptr;
  // Validate each of the five basic blocks in the pivot mover.
  if (!ValidateBBStart(BBStart, IsUp, &OuterPHI, &LimitPHI, &BBOuterExit,
      &BBLHeader))
    return false;
  if (!ValidateBBLHeader(BBLHeader, IsUp, OuterPHI, ArgArray, &BBLTest,
      &BBInnerExit, &CI))
    return false;
  if (!ValidateBBLTest(BBLTest, CI, &BBLSwap, &BBLLatch))
    return false;
  if (!ValidateBBLSwap(BBLSwap, BBLLatch, IsUp, OuterPHI, &InnerPHI))
    return false;
  if (!ValidateBBLLatch(BBLLatch, BBOuterExit, BBLHeader, IsUp, OuterPHI,
      LimitPHI))
    return false;
  return true;
}

//
// Return 'true' if 'F' is recognized as a qsort like the one that appears in
// standard C library.
//
// TODO: The implementation is not finished yet. At this point we only recognize
// computation of the pivot value and the insertion sorts which are called
// when the qsort recurses down to be applied to an array of size smaller than
//'SmallSize'
//
static bool isQsort(Function *F) {

  // Any array smaller than this size will be sorted by insertion sort.
  const unsigned SmallSize = 7;

  //
  // Return 'true' if the entry block of 'F' tests if 'ArgSize' is less than
  // 'SmallSize'. If so, set '*BBSmallSort' to the true branch out of the entry
  // block and '*BBLargeSort' to the false branch out of the entry block.
  //
  auto IsSmallCountTest = [](Function *F, Argument *ArgSize,
                             unsigned SmallSize, BasicBlock **BBSmallSort,
                             BasicBlock **BBLargeSort) -> bool {
    BasicBlock &BBEntry = F->getEntryBlock();
    auto BI = dyn_cast_or_null<BranchInst>(BBEntry.getTerminator());
    if (!BI || BI->isUnconditional() || BI->getNumSuccessors() != 2)
      return false;
    auto ICmp = dyn_cast<ICmpInst>(BI->getCondition());
    if (!ICmp || ICmp->getPredicate() != ICmpInst::ICMP_ULT)
      return false;
    if (ICmp->getOperand(0) != ArgSize)
      return false;
    auto CI = dyn_cast<ConstantInt>(ICmp->getOperand(1));
    if (!CI || CI->getZExtValue() != SmallSize)
      return false;
    *BBSmallSort = BI->getSuccessor(0);
    *BBLargeSort = BI->getSuccessor(1);
    LLVM_DEBUG(dbgs() << "QsortRec: " << F->getName()
                      << ": Found small test\n");
    return true;
  };

  //
  // Return 'true' if all PHINodes which include 'A' are equivalent (meaning
  // that they have the same incoming node, basic block pairs).
  //
  auto AllPHINodesEquivalent = [](Argument *A) -> bool {
    std::set<std::pair<Value *, BasicBlock *>> PHIS;
    for (User *U : A->users()) {
      auto PHIN = dyn_cast<PHINode>(U);
      if (!PHIN)
        continue;
      if (PHIS.empty()) {
        for (unsigned I = 0; I < PHIN->getNumIncomingValues(); ++I)
          PHIS.insert({PHIN->getIncomingValue(I), PHIN->getIncomingBlock(I)});
      } else {
        if (PHIN->getNumIncomingValues() != PHIS.size())
          return false;
        for (unsigned I = 0; I < PHIN->getNumIncomingValues(); ++I)
          if (PHIS.find({PHIN->getIncomingValue(I), PHIN->getIncomingBlock(I)})
              == PHIS.end())
            return false;
      }
    }
    return true;
  };

  //
  // Using 'BBTest' as a starting point, return a good heuristic candidate
  // for the first BasicBlock in an insertion sort, if 'BBTest' was a good
  // starting point for finding one. (A good heuristic candidate is one where
  // the size of the array being sorted is smaller than 'SmallSize'.)
  //
  auto FindInsertionSortCandidate = [](BasicBlock *BBTest,
                                     unsigned SmallSize) -> BasicBlock * {
    auto BI = dyn_cast_or_null<BranchInst>(BBTest->getTerminator());
    if (!BI || BI->isUnconditional())
      return nullptr;
    auto IC = dyn_cast<ICmpInst>(BI->getCondition());
    if (!IC || IC->getPredicate() != ICmpInst::ICMP_SGT)
      return nullptr;
    auto CI = dyn_cast<ConstantInt>(IC->getOperand(1));
    if (!CI || CI->getZExtValue() != SmallSize+1)
      return nullptr;
    BasicBlock *BBS = BI->getSuccessor(0);
    auto BIT = dyn_cast_or_null<BranchInst>(BBS->getTerminator());
    if (!BIT)
      return nullptr;
    return BIT->isUnconditional() ? BIT->getSuccessor(0) : BBS;
  };

  // Return the number of insertion sorts recognized.
  auto CountInsertionSorts = [&FindInsertionSortCandidate](Function *F,
                                                           Argument *ArgArray,
                                                           Argument *ArgSize,
                                                           unsigned SmallSize)
                                                           -> unsigned {
    unsigned InsertionCount = 0;
    for (auto &BBTest : *F) {
      BasicBlock *BBStart = FindInsertionSortCandidate(&BBTest, SmallSize);
      if (BBStart) {
        LLVM_DEBUG({
          dbgs() << "QsortRec: Checking Insertion Sort Candidate in "
                 << F->getName() << "\n";
          BBStart->dump();
        });
        if (!isInsertionSort(BBStart, ArgArray, ArgSize)) {
          LLVM_DEBUG(dbgs() << "QsortRec: Insertion Sort Candidate in "
                            << F->getName() << " FAILED Test.\n");
          return 0;
        }
        LLVM_DEBUG(dbgs() << "QsortRec: Insertion Sort Candidate in "
                          << F->getName() << " PASSED Test.\n");
        if (++InsertionCount > 2)
          return 0;
      }
    }
    return InsertionCount;
  };

  //
  // Using 'BBTest' as a starting point, return a good heuristic candidate
  // for the first BasicBlock of a pivot mover loop, if 'BBTest' was a good
  // starting point for finding one.
  //
  auto FindPivotMoverCandidate = [](BasicBlock *BBTest, bool IsUp)
                                    -> BasicBlock * {
    // Find the BasicBlock from which to look for the candidate.
    auto BI = dyn_cast_or_null<BranchInst>(BBTest->getTerminator());
    if (!BI || BI->isUnconditional())
      return nullptr;
    auto IC = dyn_cast<ICmpInst>(BI->getCondition());
    if (!IC)
      return nullptr;
    auto CP = IsUp ? CmpInst::ICMP_SLT : CmpInst::ICMP_SGT;
    if (IC->getPredicate() != CP)
      return nullptr;
    auto CI = dyn_cast<ConstantInt>(IC->getOperand(1));
    if (!CI)
      return nullptr;
    auto CV = IsUp ? 1 : -1;
    if (!CI || CI->getSExtValue() != CV)
      return nullptr;
    // Return the first predecessor whose first non-PHI instruction is
    // an ICmpInst.
    for (BasicBlock *BB : predecessors(BBTest)) {
      auto BI = dyn_cast<BranchInst>(BB->getTerminator());
      if (!BI || !isa<ICmpInst>(BB->getFirstNonPHI()))
        continue;
      return BB;
    }
    return nullptr;
  };

  //
  // Return the number of valid pivot movers recognized. We expect to
  // see one of type (1) [IsUp == true] and one of type (2) [IsUp == false].
  // If we see more than one of each, we return 0. If all goes well,
  // this function returns 2.
  //
  auto CountPivotMovers = [&FindPivotMoverCandidate](Function *F,
                                                     Argument *ArgArray)
                                                     -> unsigned {
    unsigned UpCount = 0;
    unsigned DownCount = 0;
    for (auto &BBTest : *F) {
      BasicBlock *BBStartUp = FindPivotMoverCandidate(&BBTest, true);
      BasicBlock *BBStartDown = FindPivotMoverCandidate(&BBTest, false);
      assert((!BBStartUp || !BBStartDown) && "Expecting only one candidate");
      if (!BBStartUp && !BBStartDown)
        continue;
      BasicBlock *BBStart = BBStartUp ? BBStartUp : BBStartDown;
      bool IsUp = BBStartUp;
      if (BBStart) {
        LLVM_DEBUG({
          dbgs() << "QsortRec: Checking Pivot Mover Candidate in "
                 << F->getName() << "\n";
          BBStart->dump();
        });
        if (!isPivotMover(BBStart, ArgArray, IsUp)) {
          LLVM_DEBUG(dbgs() << "QsortRec: Pivot Mover Candidate in "
                            << F->getName() << " FAILED Test.\n");
          return 0;
        }
        if (IsUp)
          UpCount++;
        else
          DownCount++;
        LLVM_DEBUG(dbgs() << "QsortRec: Pivot Mover Candidate in "
                          << F->getName() << " PASSED Test "
                          << (IsUp ? "(UP)" : "(DOWN)") << "\n");
        if (UpCount > 1 || DownCount > 1)
          return 0;
      }
    }
    return UpCount + DownCount;
  };

  // Main code for isQsort().
  // Exclude obvious cases.
  if (F->isDeclaration() || F->isVarArg() || F->arg_size() != 2)
    return false;
  Argument *ArgArray = F->getArg(0);
  Argument *ArgSize = F->getArg(1);
  BasicBlock *BBSmallSort = nullptr;
  BasicBlock *BBLargeSort = nullptr;

  // This vector will hold the new sizes for the recursion. Entry 0
  // represents the size used in the direct recursion and entry 1
  // represents the size used in the tail recursion.
  SmallVector<Value *, 2> NewSizesVector;
  SetVector<Value *> PointersToArg;

  // Validate that the code branches to special case (insertion sort) for
  // sufficiently small arrays.
  if (!IsSmallCountTest(F, ArgSize, SmallSize, &BBSmallSort, &BBLargeSort))
    return false;
  // Check that all PHINodes that include the arguments are equivalent.
  if (!AllPHINodesEquivalent(ArgArray) || !AllPHINodesEquivalent(ArgSize))
    return false;
  // Find the pivot for the qsort.
  if (QsortTestPivot && !qsortPivot(BBLargeSort, ArgArray, ArgSize))
    return false;
  // The logic is not complete. For now, just look for two places where the
  // insertion sort is invoked.
  if (QsortTestInsert) {
    unsigned ISCount = CountInsertionSorts(F, ArgArray, ArgSize, SmallSize);
    if (ISCount < 2)
      return false;
  }
  // The logic is not complete. For now, just look for two pivot mover loops,
  // one for each direction (IsUp and !IsUp).
  if (QsortTestPivotMovers) {
    unsigned PMCount = CountPivotMovers(F, ArgArray);
    if (PMCount < 2)
      return false;
  }

  // TODO: We need to add an analysis that collects all the pointers to
  // ArgArray before calling the MIN counter. Basically, we need to proof that
  // the Values used to compute MIN are ArgArray, or ArgArray + (something
  // between 8 to ArgSize).
  if (QsortTestMin) {
    unsigned MINCount = countMinComputations(F, ArgArray, PointersToArg,
                                             NewSizesVector);
    if (MINCount < 2)
      return false;
  }

  if (QsortTestSwap) {
    unsigned SwapCount = countSwapComputations(F, ArgArray);
    if (SwapCount < 2)
      return false;
  }

  if (QsortTestRecursion) {
    unsigned RecCount = countRecursions(F, ArgArray, ArgSize, NewSizesVector);
    if (RecCount < 2)
      return false;
  }

  return true;
}

//
// Return 'true' if some Function in 'M' is recognized as a qsort. In such a
// case, set the 'is-qsort' attribute on the Function.
//
static bool QsortRecognizerImpl(Module &M) {
  bool SawQsort = false;
  for (auto &F : M.functions())
    if (isQsort(&F)) {
      F.addFnAttr("is-qsort");
      QsortsRecognized++;
      SawQsort = true;
    }
  if (SawQsort)
    LLVM_DEBUG(dbgs() << "FOUND QSORT\n");
  return SawQsort;
}

namespace {

struct QsortRecognizerLegacyPass : public ModulePass {
public:
  static char ID; // Pass identification, replacement for typeid
  QsortRecognizerLegacyPass(void) : ModulePass(ID) {
    initializeQsortRecognizerLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;
    return QsortRecognizerImpl(M);
  }
};

} // namespace

char QsortRecognizerLegacyPass::ID = 0;
INITIALIZE_PASS(QsortRecognizerLegacyPass, "qsortrecognizer", "QsortRecognizer",
                false, false)

ModulePass *llvm::createQsortRecognizerLegacyPass(void) {
  return new QsortRecognizerLegacyPass();
}

QsortRecognizerPass::QsortRecognizerPass(void) {}

PreservedAnalyses QsortRecognizerPass::run(Module &M,
                                           ModuleAnalysisManager &AM) {
  QsortRecognizerImpl(M);
  return PreservedAnalyses::all();
}
