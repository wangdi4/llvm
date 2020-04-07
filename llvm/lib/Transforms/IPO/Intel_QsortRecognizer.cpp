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
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/Utils/Intel_IPOUtils.h"
#include <set>
#include <queue>

using namespace llvm;
using namespace PatternMatch;

#define DEBUG_TYPE "qsortrecognizer"

STATISTIC(QsortsRecognized, "Number of qsort functions recognized");

//
// Options to enable LIT testing of various subcomponents of qsort
// recognition.  Use -qsort-unit-test to enable unit (LIT) testing,
// then select the subcomponents you want to test using the other
// options below.
//

static cl::opt<bool> QsortUnitTest("qsort-unit-test",
                                   cl::init(false), cl::ReallyHidden);

static cl::opt<bool> QsortTestPivot("qsort-test-pivot",
                                    cl::init(false), cl::ReallyHidden);

static cl::opt<bool> QsortTestInsert("qsort-test-insert",
                                     cl::init(false), cl::ReallyHidden);

static cl::opt<bool> QsortTestPivotMovers("qsort-test-pivot-movers",
                                          cl::init(false), cl::ReallyHidden);

static cl::opt<bool> QsortTestPivotSorter("qsort-test-pivot-sorter",
                                          cl::init(false), cl::ReallyHidden);

static cl::opt<bool> QsortTestMin("qsort-test-min",
                                   cl::init(false), cl::ReallyHidden);

static cl::opt<bool> QsortTestSwap("qsort-test-swap",
                                    cl::init(false), cl::ReallyHidden);

static cl::opt<bool> QsortTestRecursion("qsort-test-recursion",
                                         cl::init(false), cl::ReallyHidden);

static cl::opt<bool> QsortTestCompareFunc("qsort-test-compare-func",
                                         cl::init(false), cl::ReallyHidden);

static cl::opt<bool> QsortTestStoreSwaps("qsort-test-store-swaps",
                                         cl::init(false), cl::ReallyHidden);
// Utilities to handle the argument's analysis
static IntelArgumentAlignmentUtils ArgUtil;

//
// Return a pointer to the first non-debug Instruction in 'BB'.
//
static Instruction *firstNonDbgInst(BasicBlock *BB) {
  BasicBlock::iterator I = BB->begin();
  while (isa<DbgInfoIntrinsic>(I))
    ++I;
  return &*I;
}

//
// Return 'true' if 'V' represents a PHINode and one of its incoming values
// is 'Arg'.
//
static bool isPHINodeWithArgIncomingValue(Value *V, Argument *Arg) {
  auto PHIN = dyn_cast_or_null<PHINode>(V);
  if (!PHIN || !Arg)
    return false;
  auto Num = PHIN->getNumIncomingValues();
  for (unsigned I = 0; I < Num; ++I)
    if (PHIN->getIncomingValue(I) == Arg)
      return true;
  return false;
}

// Return true if the input function is a compare function. A compare function
// has two inputs with the same type and two pointers that load from each
// input. Then, a Value is collected from the loaded pointer through a GEP.
// These loaded Values from the GEPs are compared against each other with a
// signed "lower than" (SLT) or signed "greater than" (SGT) operations. The
// function must return 1 or -1, or the result from a select instruction that
// choose between -1 or 1.
static bool isCompareFunction(Function *CompareFunc) {
  if (!CompareFunc)
    return false;

  Argument *Arg0 = CompareFunc->getArg(0);
  Argument *Arg1 = CompareFunc->getArg(1);
  if (Arg0->getType() != Arg1->getType())
    return false;

  // Return the operand 0 from the input Value if:
  //  * Val is a Load instruction and the operand is a GEP, BitCast or Argument
  //  * Val is a BitCast and operand is a Load
  //  * Val is a GEP, the operand 0 is a Load, and the operands 1 and 2 are
  //    constants
  // Else return nullptr.
  auto GetOperand = [](Value *Val) -> Value * {
    Value *RetVal = nullptr;
    if (!Val)
      return nullptr;

    // A load must come from a BitCast, a GEP or the argument itself
    if (LoadInst *Load = dyn_cast<LoadInst>(Val)) {
      RetVal = Load->getOperand(0);
      if (!isa<GetElementPtrInst>(RetVal) && !isa<Argument>(RetVal) &&
          !isa<BitCastInst>(RetVal))
        return nullptr;
    }

    // A BitCast must come from a load
    else if (BitCastInst *BitCast = dyn_cast<BitCastInst>(Val)) {
      RetVal = BitCast->getOperand(0);
      if (!isa<LoadInst>(RetVal))
        return nullptr;
    }

    // Check the GEP
    else if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(Val)) {
      // Collecting from a structure and GEPPos must not be set
      if (GEP->getNumOperands() != 3)
        return nullptr;

      ConstantInt *Pos = dyn_cast<ConstantInt>(GEP->getOperand(2));
      ConstantInt *PosStruct = dyn_cast<ConstantInt>(GEP->getOperand(1));

      // GEP is not constant
      if (!Pos || !PosStruct)
          return nullptr;

      RetVal = GEP->getOperand(0);
      if (!isa<LoadInst>(RetVal))
        return nullptr;
    }

    return RetVal;
  };

  // Return true if tracing from the input Value Val0 reaches the Arg0, and
  // tracing Val1 reaches Arg1. Also, each collected instruction during the
  // traversal must match the type. If the instructions are GEPs then
  // operands 1 and 2 must be the same constants.
  auto CheckValuesWithArguments = [GetOperand](Value *Val0, Argument *Arg0,
                                               Value *Val1, Argument *Arg1) {

    if (!Val0 || !Val1 || !Arg0 || !Arg1)
      return false;

    Value *CurrVal0 = Val0;
    Value *CurrVal1 = Val1;

    while (CurrVal0 && CurrVal1) {

      if (CurrVal0->getType() != CurrVal1->getType())
        return false;

      if (CurrVal0 == cast<Value>(Arg0) && CurrVal1 == cast<Value>(Arg1))
        return true;

      if (isa<GetElementPtrInst>(CurrVal0) &&
          isa<GetElementPtrInst>(CurrVal1)) {
        GetElementPtrInst *GEP0 = cast<GetElementPtrInst>(CurrVal0);
        GetElementPtrInst *GEP1 = cast<GetElementPtrInst>(CurrVal1);

        // Collecting from a structure and GEPPos must not be set
        if (GEP0->getNumOperands() != 3 || GEP1->getNumOperands() != 3)
          return false;

        // Operand 1 and 2 in GEP0 must be constant values
        if (!isa<ConstantInt>(GEP0->getOperand(1)) ||
            !isa<ConstantInt>(GEP0->getOperand(2)))
          return false;

        // Operand 1 and 2 in GEP1 must be constant values
        if (!isa<ConstantInt>(GEP1->getOperand(1)) ||
            !isa<ConstantInt>(GEP1->getOperand(2)))
          return false;

        if ((GEP0->getOperand(1) != GEP1->getOperand(1)) ||
            (GEP0->getOperand(2) != GEP1->getOperand(2)))
          return false;

        CurrVal0 = GetOperand(CurrVal0);
        CurrVal1 = GetOperand(CurrVal1);
      }

      else if ((isa<LoadInst>(CurrVal0) && isa<LoadInst>(CurrVal1)) ||
          (isa<BitCastInst>(CurrVal0) && isa<BitCastInst>(CurrVal1))) {
        CurrVal0 = GetOperand(CurrVal0);
        CurrVal1 = GetOperand(CurrVal1);
      } else {
        return false;
      }
    }

    return false;
  };

  // Return true if the input Value compares the entries of the arguments.
  // For example:
  //
  // define internal i32 @compare(%some.structure** %0, %some.structure** %1) {
  //   %3 = load %some.structure*, %some.structure** %0
  //   %4 = getelementptr inbounds %some.structure, %some.structure* %3, i64 0,
  //        i32 2
  //   %5 = load i64, i64* %4
  //   %6 = load %some.structure*, %some.structure** %1
  //   %7 = getelementptr inbounds %some.structure, %some.structure* %6, i64 0,
  //        i32 2
  //   %8 = load i64, i64* %7
  //   %9 = icmp slt i64 %5, %8
  //
  // Consider that Cond is %9 from the example above, Arg0 is %0 and Arg is %1.
  // The following function will check that is a "less than" or "greater than"
  // compare from Values loaded from these arguments. Also, both Values
  // must be loaded from the same entry, this means that the GEP must use
  // the same constant operands (second operand in %4 and %7).
  auto IsValidCompare = [CheckValuesWithArguments](Value *Cond, Argument *Arg0,
                                                   Argument *Arg1,
                                                   unsigned *MaxSgt,
                                                   unsigned *MaxSlt) {

    if (!Cond || !Arg0 || !Arg1)
      return false;

    Value *LVal = nullptr;
    Value *RVal = nullptr;
    ICmpInst::Predicate Pred = ICmpInst::BAD_ICMP_PREDICATE;

    if (!match(Cond, m_ICmp(Pred, m_Value(LVal), m_Value(RVal))))
      return false;

    // Is a "less than" or "greater than" operation.
    if (Pred == ICmpInst::ICMP_SLT) {
      (*MaxSlt)++;
      if (*MaxSlt > 2)
        return false;
    }

    else if (Pred == ICmpInst::ICMP_SGT) {
      (*MaxSgt)++;
      if (*MaxSgt > 2)
        return false;
    } else {
      return false;
    }

    if (!CheckValuesWithArguments(LVal, Arg0, RVal, Arg1))
      return false;

    return true;
  };

  // Collect the 1 or -1 from a ConstantInt and return it, else
  // return 0
  auto GetConstantOne = [](Value *Val) {
    if (!Val)
      return 0;

    ConstantInt *Const = dyn_cast<ConstantInt>(Val);

    if (!Const)
      return 0;

    int SignedOne = Const->getSExtValue();

    if (SignedOne == 1 || SignedOne == -1)
      return SignedOne;

    return 0;
  };

  // Returns true if "Val" is SelectInst that returns either 1 or -1
  // based on argument comparison condition.
  // Ex:
  //     %21 = icmp sgt i32 %16, %20
  //     %22 = select i1 %21, i32 1, i32 -1
  auto CheckSelect = [&, IsValidCompare, GetConstantOne](
                         Value *Val, unsigned *MaxSgtP, unsigned *MaxSltP) {
    auto *SelInst = dyn_cast<SelectInst>(Val);
    if (!SelInst)
      return false;
    int FalseConst = GetConstantOne(SelInst->getFalseValue());
    int TrueConst = GetConstantOne(SelInst->getTrueValue());

    // Each side is either -1 or 1
    if (FalseConst == 0 || TrueConst == 0)
      return false;
    // Both sides are the same
    if (FalseConst == TrueConst)
      return false;
    // The condition must use values pointing to the arguments
    Value *Cond = SelInst->getCondition();
    if (!IsValidCompare(Cond, Arg0, Arg1, MaxSgtP, MaxSltP))
      return false;
    return true;
  };

  // There will be one use for each argument
  if (!Arg0->hasOneUse() || !Arg1->hasOneUse())
    return false;

  unsigned MaxSgt = 0;
  unsigned MaxSlt = 0;
  bool FoundOne = false;
  bool FoundMinusOne = false;
  bool FoundSelect = false;

  for (auto &BB : *CompareFunc) {
    Instruction *Terminator = BB.getTerminator();

    // Check the branch
    if (BranchInst *Branch = dyn_cast<BranchInst>(Terminator)) {
      // Unconditional branch should lead to the exit block
      if (Branch->isUnconditional()) {
        BasicBlock *ExitBB = Terminator->getSuccessor(0);
        // Deal with the return later
        if (!isa<ReturnInst>(ExitBB->getTerminator()))
          return false;
        continue;
      }

      // A conditional branch is comparing two Values that come
      // from the input arguments
      Value *Cond = Branch->getCondition();
      if (!IsValidCompare(Cond, Arg0, Arg1, &MaxSgt, &MaxSlt)) {
        return false;
      }
    }

    else if (ReturnInst *Ret = dyn_cast<ReturnInst>(Terminator)) {

      // The below two patterns are allowed for ReturnInst.
      // Pattern 1:
      //      %21 = icmp sgt i32 %16, %20
      //      %22 = select i1 %21, i32 1, i32 -1
      //      ret i32 %22
      //
      //    23:
      //      %24 = phi i32 [ 1, %2 ], [ -1, %10 ]
      //      ret i32 %24
      //
      //
      // Pattern 2:
      // The Value returned comes from a PHI Node that checks for
      // 1, -1 or a Select instruction. For example:
      //
      //     %21 = icmp sgt i32 %16, %20
      //     %22 = select i1 %21, i32 1, i32 -1
      //     br label %23
      //
      //   23:
      //     %24 = phi i32 [ 1, %2 ], [ -1, %10 ], [ %22, %12 ]
      //     ret i32 %24
      Value *RVal = Ret->getReturnValue();
      if (!RVal)
        return false;
      if (!FoundSelect && CheckSelect(RVal, &MaxSgt, &MaxSlt)) {
        FoundSelect = true;
        continue;
      }

      PHINode *RetPHI = dyn_cast<PHINode>(RVal);

      if (!RetPHI)
        return false;

      unsigned NumIncomingValues = RetPHI->getNumIncomingValues();

      if (NumIncomingValues > 3)
        return false;

      for (unsigned Entry = 0; Entry < NumIncomingValues; Entry++) {
        Value *Val = RetPHI->getIncomingValue(Entry);
        if (!FoundOne && GetConstantOne(Val) == 1)
          FoundOne = true;
        else if (!FoundMinusOne && GetConstantOne(Val) == -1)
          FoundMinusOne = true;
        else if (!FoundSelect && CheckSelect(Val, &MaxSgt, &MaxSlt))
          FoundSelect = true;
        else
          return false;
      }
    } else {
      return false;
    }
  }

  if (!FoundOne || !FoundMinusOne || !FoundSelect)
    return false;
  // Make sure that we found the correct number of "less than" and "greater
  // than" operations.
  if (MaxSgt == 2 && MaxSlt == 1)
    return true;
  else if (MaxSlt == 2 && MaxSgt == 1)
    return true;

  // Compare function not found.
  return false;
}

// Return true if at least there is call site to a compare function, and
// the compare function is the only function called across F (except the
// recursive call). Else return false.
static bool findCompareFunction(Function *F) {
  if (!F)
    return false;

  // Compare function (there is only one across the whole qsort)
  Function *CompareFunction = nullptr;

  for (auto &I : instructions(F)) {
    CallBase *Call = dyn_cast<CallBase>(&I);
    if (!Call)
      continue;

    if (isa<DbgInfoIntrinsic>(I))
      continue;

    // All calls must be a direct call
    if (Call->isIndirectCall()) {
      CompareFunction = nullptr;
      break;
    }

    Function *CalledFunc = Call->getCalledFunction();
    if (CalledFunc == F)
      continue;

    // If the compare function is set then check with the
    // current called function
    if (CompareFunction) {
      if (CalledFunc != CompareFunction) {
        CompareFunction = nullptr;
        break;
      }
      continue;
    }

    // Else check that is a compare function
    if (!isCompareFunction(CalledFunc)) {
      CompareFunction = nullptr;
      break;
    }

    CompareFunction = CalledFunc;
  }

  LLVM_DEBUG({
    dbgs() << "QsortRec: Compare Function"
           << (CompareFunction ? " " : " NOT ") << "found";

    if (CompareFunction)
      dbgs() << ": " << CompareFunction->getName();

    dbgs() << "\n";
  });

  return CompareFunction;
}

//
// Return 'true' if 'I' is the beginning of six instruction swap sequence
// like this one:
//
//  %184 = bitcast i8* %174 to i64*
//  %185 = load i64, i64* %184, align 8
//  %186 = bitcast i8* %175 to i64*
//  %187 = load i64, i64* %186, align 8
//  store i64 %187, i64* %184, align 8
//  store i64 %185, i64* %186, align 8
//
// Set the input values in '*V0' and '*V1'. (In this example, they are
// %174 and %175.) Set '*NI' to the Instruction following the sequence.
//
static bool isSwapSequence(Instruction *I, Value **V0, Value **V1,
                           Instruction **NI,
                           SetVector<StoreInst *> &StoreInstructions) {

  auto BC0 = dyn_cast_or_null<BitCastInst>(I);
  if (!BC0)
    return false;
  *V0 = BC0->getOperand(0);
  auto L0 = dyn_cast_or_null<LoadInst>(BC0->getNextNonDebugInstruction());
  if (!L0 || L0->getPointerOperand() != BC0)
    return false;
  auto BC1 = dyn_cast_or_null<BitCastInst>(L0->getNextNonDebugInstruction());
  if (!BC1)
    return false;
  *V1 = BC1->getOperand(0);
  auto L1 = dyn_cast_or_null<LoadInst>(BC1->getNextNonDebugInstruction());
  if (!L1 || L1->getPointerOperand() != BC1)
    return false;
  auto S0 = dyn_cast<StoreInst>(L1->getNextNonDebugInstruction());
  if (!S0 || S0->getPointerOperand() != BC0 || S0->getValueOperand() != L1)
    return false;
  auto S1 = dyn_cast<StoreInst>(S0->getNextNonDebugInstruction());
  if (!S1 || S1->getPointerOperand() != BC1 || S1->getValueOperand() != L0)
    return false;
  *NI = S1->getNextNonDebugInstruction();

  StoreInstructions.insert(S0);
  StoreInstructions.insert(S1);

  return true;
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
                            Argument *ArgSize,
                            SetVector<StoreInst *> &StoreInstructions) {

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
    if (!BI || BI->isUnconditional())
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
    if (!BI || BI->isUnconditional())
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
    if (GEPI != firstNonDbgInst(BBILH)->getNextNonDebugInstruction())
      return false;
    auto CIG = dyn_cast<ConstantInt>(GEPI->getOperand(1));
    if (!CIG || CIG->getSExtValue() != -8)
      return false;
    auto PN = dyn_cast<PHINode>(GEPI->getPointerOperand());
    if (!PN || PN->getNumIncomingValues() != 2 || PN != firstNonDbgInst(BBILH))
      return false;
    if (PN->getIncomingValue(0) != GEPI)
      return false;
    if (PN->getIncomingValue(1) != firstNonDbgInst(BBOLH))
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
  auto ValidateBBIL = [&ArgArray, &StoreInstructions](BasicBlock *BBIL,
                                                      BasicBlock *BBILH,
                                   BasicBlock *BBOL) -> bool {
    auto BI = dyn_cast<BranchInst>(BBIL->getTerminator());
    if (!BI || BI->isUnconditional())
      return false;
    if (BI->getSuccessor(0) != BBILH)
      return false;
    if (BI->getSuccessor(1) != BBOL)
      return false;
    auto IC = dyn_cast<ICmpInst>(BI->getCondition());
    if (!IC || IC->getPredicate() != ICmpInst::ICMP_UGT)
      return false;
    if (IC->getOperand(0)
        != firstNonDbgInst(BBILH)->getNextNonDebugInstruction())
      return false;
    Value *V0 = nullptr;
    Value *V1 = nullptr;
    Instruction *NI = nullptr;
    if (!isPHINodeWithArgIncomingValue(IC->getOperand(1), ArgArray))
      return false;
    if (!isSwapSequence(firstNonDbgInst(BBIL), &V0, &V1, &NI,
                        StoreInstructions))
      return false;
    if (V0 != &BBILH->front())
      return false;
    if (V1 != BBILH->front().getNextNonDebugInstruction())
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
    auto BI = dyn_cast<BranchInst>(BBOL->getTerminator());
    if (!BI || BI->isUnconditional())
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
    if (GEPIL->getPointerOperand() != firstNonDbgInst(BBOLH))
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
    auto RI = dyn_cast<ReturnInst>(firstNonDbgInst(BBEnd));
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
      auto PN = dyn_cast<PHINode>(firstNonDbgInst(BBS));
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

// Return true if all store instructions in the input function are in the
// SetVector StoreInstructions. If so then it means that all StoreInst are
// used for swapping values.
static bool allStoresAreSwaps(Function *F,
                              SetVector<StoreInst*> &StoreInstructions) {

  if (!F || StoreInstructions.empty())
    return false;

  for (auto &Inst: instructions(F)) {
    StoreInst *Store = dyn_cast<StoreInst>(&Inst);
    if (!Store)
      continue;

    if (StoreInstructions.count(Store) == 0) {
      LLVM_DEBUG(dbgs() << "QsortRec: All store instructions"
                        << " are NOT swaps\n");
      return false;
    }
  }

  LLVM_DEBUG(dbgs() << "QsortRec: All store instructions"
                    << " are swaps\n");
  return true;
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
// It will look for the Select instruction (%289). Then it will check that
// the basic block represents a computation of MIN. This means that the
// block is computing the lowest value between (pa - a, pb - pa), or
// (pd - pc, pn - pd - 8).
//
// The input vector (SizesVector) will be used to store the operands of the
// Select instructions found. We are going to use them later in the process
// that counts the recursions. This is because the operands in the Select
// instructions represent the new sizes that will be used in the recursions.
//
// Also, we store the select instructions found because they represent the
// values used to compute the vecswap:
//
//   before the pivot (left side): from a to pb - r (SelectLeftOut)
//   after the pivot (right side): from pb to pn - r (SelectRightOut)
//
// The GEP that represents pn is stored (GEPpnOut) since we need it for the
// swaps too.
static unsigned countMinComputations(Function *F, Argument *ArgArray,
                             Argument *ArgSize, PHINode *PHIpa, PHINode *PHIpb,
                             PHINode *PHIpc, PHINode *PHIpd,
                             GetElementPtrInst **GEPpnOut,
                             SmallVector<Value*, 2> &SizesVect,
                             SelectInst **SelectLeftOut,
                             SelectInst **SelectRightOut) {

  if (!F)
    return 0;

  // Go through the input basic block and find the Select instruction.
  // The Block must have one select instruction, else return nullptr.
  auto FindSelectInst = [](BasicBlock &BB) {
    SelectInst *SelInst = nullptr;

    for (auto &Inst : BB.instructionsWithoutDebug()) {
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

  // Return true if the input Select instruction refers to the first
  // half of ArgArray (MIN(pa - a, pb - pa)). Else return false.
  auto CheckFirstHalf = [PHIpa, PHIpb](PHINode *ArgPHI, SelectInst *SelInst) {
    if (!ArgPHI || !SelInst)
      return false;

    BasicBlock *BB = SelInst->getParent();

    // Find pa - a
    PtrToIntInst *PtrPa = dyn_cast<PtrToIntInst>(firstNonDbgInst(BB));
    if (!PtrPa || PtrPa->getOperand(0) != PHIpa)
      return false;
    BinaryOperator *SubA =
        dyn_cast<BinaryOperator>(PtrPa->getNextNonDebugInstruction());
    if (!SubA || SubA->getOpcode() != Instruction::Sub)
      return false;
    if (SubA->getOperand(0) != PtrPa || SubA->getOperand(1) != ArgPHI)
      return false;

    // Find pb - pa
    PtrToIntInst *PtrPb =
        dyn_cast<PtrToIntInst>(SubA->getNextNonDebugInstruction());
    if (!PtrPb || PtrPb->getOperand(0) != PHIpb)
      return false;
    BinaryOperator *SubPa =
        dyn_cast<BinaryOperator>(PtrPb->getNextNonDebugInstruction());
    if (!SubPa || SubPa->getOpcode() != Instruction::Sub)
      return false;
    if (SubPa->getOperand(0) != PtrPb || SubPa->getOperand(1) != PtrPa)
      return false;

    // Compare if (pa - a) < (pb - pa)
    Value *CmprLT = SubPa->getNextNonDebugInstruction();
    ICmpInst::Predicate Pred;
    if (!match(CmprLT, m_ICmp(Pred, m_Specific(SubA), m_Specific(SubPa))))
      return false;

    if (Pred != ICmpInst::ICMP_SLT)
      return false;

    // Select between the lowest value (pa - a or pb - pa)
    if (SelInst->getCondition() != CmprLT ||
        SelInst->getTrueValue() != SubA ||
        SelInst->getFalseValue() != SubPa)
      return false;

    return true;
  };

  // Return true if the input Select instruction refers to the second
  // half of ArgArray (MIN(pd - pc, pn - pd - 8)). Else return false.
  auto CheckSecondHalf = [PHIpc, PHIpd, ArgArray, ArgSize, &GEPpnOut](
      SelectInst *SelInst) {
    if (!SelInst)
      return false;

    BasicBlock *BB = SelInst->getParent();

    // Find pd - pc
    PtrToIntInst *PtrPd = dyn_cast<PtrToIntInst>(firstNonDbgInst(BB));
    if (!PtrPd || PtrPd->getOperand(0) != PHIpd)
      return false;

    PtrToIntInst *PtrPc =
        dyn_cast<PtrToIntInst>(PtrPd->getNextNonDebugInstruction());
    if (!PtrPc || PtrPc->getOperand(0) != PHIpc)
      return false;

    BinaryOperator *SubPdPc =
        dyn_cast<BinaryOperator>(PtrPc->getNextNonDebugInstruction());
    if (!SubPdPc || SubPdPc->getOpcode() != Instruction::Sub)
      return false;
    if (SubPdPc->getOperand(0) != PtrPd || SubPdPc->getOperand(1) != PtrPc)
      return false;

    // Find pn - pd - 8
    PtrToIntInst *PtrPn =
        dyn_cast_or_null<PtrToIntInst>(
        SubPdPc->getNextNonDebugInstruction());
    if (!PtrPn)
      return false;

    // There must be only one GEP that represents pn
    if (*GEPpnOut)
      return false;

    // pn is a pointer pointing to the end of the array
    GetElementPtrInst *GEPArg =
        dyn_cast<GetElementPtrInst>(PtrPn->getOperand(0));
    if (!GEPArg || GEPArg->getNumOperands() != 2)
      return false;

    // operand 0 must be a user of ArgArray
    // (%34 = phi i8* [ %317, %315 ], [ %0, %2 ])
    if (!isPHINodeWithArgIncomingValue(GEPArg->getOperand(0), ArgArray))
      return false;

    // Operand 1 is a left shift of the argument size
    Value *ArgSizeUser = nullptr;
    ConstantInt *Const = nullptr;
    if (!match(GEPArg->getOperand(1),
        m_Shl(m_Value(ArgSizeUser), m_ConstantInt(Const))) ||
        Const->getZExtValue() != 3)
      return false;

    // operand 1 must point to the size
    // %35 = phi i64 [ %318, %315 ], [ %1, %2 ]
    if (!isPHINodeWithArgIncomingValue(ArgSizeUser, ArgSize))
      return false;

    *GEPpnOut = GEPArg;

    BinaryOperator *SubPnPd =
        dyn_cast<BinaryOperator>(PtrPn->getNextNonDebugInstruction());
    if (!SubPnPd || SubPnPd->getOpcode() != Instruction::Sub)
      return false;
    if (SubPnPd->getOperand(0) != PtrPn || SubPnPd->getOperand(1) != PtrPd)
      return false;

    BinaryOperator *AddMinus8 =
        dyn_cast<BinaryOperator>(SubPnPd->getNextNonDebugInstruction());
    ConstantInt *Minus8 = nullptr;
    if (!match(AddMinus8, m_Add(m_Specific(SubPnPd), m_ConstantInt(Minus8))) ||
        Minus8->getSExtValue() != -8)
      return false;

    // Compare if (pd - pc) < (pn - pd - 8)
    Value *CmprLT = AddMinus8->getNextNonDebugInstruction();
    ICmpInst::Predicate Pred;
    if (!match(CmprLT, m_ICmp(Pred,
        m_Specific(SubPdPc), m_Specific(AddMinus8))) ||
        Pred != ICmpInst::ICMP_SLT)
      return false;

    // Select between the lowest value (pd - pc or pn - pd - 8)
    return (SelInst->getCondition() == CmprLT &&
        SelInst->getTrueValue() == SubPdPc &&
        SelInst->getFalseValue() == AddMinus8);
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

  // Find the user that is PointerToInt and the get the PHINode that refers to
  // that user.
  auto FindPHIForArgArray = [ArgArray]() -> PHINode * {

    PHINode *PHI = nullptr;
    for (User *User : ArgArray->users()) {

      if (PtrToIntInst *PtrToInt = dyn_cast<PtrToIntInst>(User)) {
        if (PHI)
          return nullptr;

        // The pointer to int instruction will have only one user and must
        // be a PHINode
        if (!PtrToInt->hasOneUse())
          return nullptr;

        PHI = dyn_cast<PHINode>(PtrToInt->user_back());
        if (!PHI)
          continue;

        if (PHI->getNumIncomingValues() != 2)
          return nullptr;
      }
    }

    return PHI;
  };

  unsigned MINCounter = 0;

  Value *DirectRecCallSize = nullptr;
  Value *TailRecCallSize = nullptr;

  // Find the PHI for the pointer-to-int instruction that refers to
  // the array argument
  PHINode *PHIArgArray = FindPHIForArgArray();
  if (!PHIArgArray)
    return 0;

  // Go through each of the basic blocks in the function and check
  // if there is a computation of MIN
  for (auto &BB : *F) {

    if (!isa<PtrToIntInst>(BB.getFirstNonPHIOrDbg()))
      continue;

    if (SelectInst *SelInst = FindSelectInst(BB)) {
      if (IsValidSelect(SelInst)) {

        // Check if the select instruction represents the left side
        // of the pivot (MIN(pa - a, pb - pa)
        bool LocalFirstHalf = CheckFirstHalf(PHIArgArray, SelInst);
        if (LocalFirstHalf) {
          if (*SelectLeftOut)
            return MINCounter;
          *SelectLeftOut = SelInst;
        }

        // Check if the select instruction represents the right side
        // of the pivot
        bool LocalSecondHalf = CheckSecondHalf(SelInst);
        if (LocalSecondHalf) {
          if (*SelectRightOut)
            return MINCounter;
          *SelectRightOut = SelInst;
        }

        // Can't be both
        if (LocalFirstHalf && LocalSecondHalf)
          return 0;

        // Nothing found
        if (!LocalFirstHalf && !LocalSecondHalf)
          continue;

        LLVM_DEBUG({
          dbgs() << "QsortRec: Checking computation of MIN in "
                 << F->getName() <<"\n";
          BB.dump();
        });

        // collect the possible new sizes
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

        LLVM_DEBUG(dbgs() << "QsortRec: Computation of MIN in "
                        << F->getName() << " PASSED Test.\n");

        MINCounter++;
      }
    }

    if (MINCounter > 2)
      return 0;
  }

  // Both new sizes must be found
  if (!DirectRecCallSize || !TailRecCallSize) {
    LLVM_DEBUG(dbgs() << "QsortRec: Computation of MIN in "
                      << F->getName() << " FAILED Test.\n");
    return 0;
  }

  SizesVect.push_back(DirectRecCallSize);
  SizesVect.push_back(TailRecCallSize);

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
// between %300 and %301. Also, it checks that %301 comes from PHIpb
// or Arg, and %300 comes from a GEP where operand 0 is PHIpb (or GEPpn)
// and operand 1 is SelectLeft (or SelectRight).
//
// We are trying to catch here the swap before the pivot pb (from a to pn -r)
// and after the pivot (from pn to pn - r).
static unsigned countSwapComputations(Function *F, Argument *Arg,
    PHINode *PHIpb, GetElementPtrInst *GEPpn, SelectInst *SelectLeft,
    SelectInst *SelectRight, SetVector<StoreInst *> &StoreInstructions) {

  if (!F || !Arg)
    return false;

  // Return the incoming value of a PHINode that doesn't come from
  // a loop and is a BitCast
  auto GetNonRecursiveInstFromPHI = [](PHINode *PHI) -> BitCastInst * {
    if (!PHI || PHI->getNumIncomingValues() != 2)
      return nullptr;

    Value *Val1 = PHI->getIncomingValue(0);
    Value *Val2 = PHI->getIncomingValue(1);

    if (isa<GetElementPtrInst>(Val1) && isa<BitCastInst>(Val2)) {
      GetElementPtrInst *GEP = cast<GetElementPtrInst>(Val1);
      BitCastInst *BitCast = cast<BitCastInst>(Val2);

      if (GEP->getOperand(0) == PHI)
        return BitCast;
    }
    else if (isa<GetElementPtrInst>(Val2) && isa<BitCastInst>(Val1)) {
      GetElementPtrInst *GEP = cast<GetElementPtrInst>(Val2);
      BitCastInst *BitCast = cast<BitCastInst>(Val1);

      if (GEP->getOperand(0) == PHI)
        return BitCast;
    }
    return nullptr;
  };

  // Return true if the input store function is writing at the beginning
  // of the array's chunk (writing from to a or pb)
  auto IsPointingAtBeginning = [GetNonRecursiveInstFromPHI](
      StoreInst *Store, Value *ValStart) {
    if (!Store || !ValStart)
      return false;

    PHINode *PHIStored = dyn_cast<PHINode>(Store->getOperand(1));
    if (!PHIStored)
      return false;

    BitCastInst *BitCast = GetNonRecursiveInstFromPHI(PHIStored);
    if (!BitCast)
      return false;

    // Handle the case of pb
    if (BitCast->getOperand(0) == ValStart)
      return true;

    // Handle the case of a
    return isPHINodeWithArgIncomingValue(BitCast->getOperand(0),
                                         dyn_cast<Argument>(ValStart));
  };

  // Return true if the input store function is writing at the end of
  // array's chunk (writing to pb - SelectLeft or pn - SelectRight)
  auto IsPointingAtEnd = [GetNonRecursiveInstFromPHI](
      StoreInst *Store, Value *ValEnd, SelectInst *Select) {
    PHINode *PHIStored = dyn_cast<PHINode>(Store->getPointerOperand());
    if (!PHIStored)
      return false;

    BitCastInst *BitCast = GetNonRecursiveInstFromPHI(PHIStored);
    if (!BitCast)
      return false;

    GetElementPtrInst *GEP =
        dyn_cast<GetElementPtrInst>(BitCast->getOperand(0));
    if (!GEP)
      return false;

    // GEP pointing to pb or pn
    if (GEP->getNumOperands() != 2 || ValEnd != GEP->getOperand(0))
      return false;

    // Collecting from - r
    return match(GEP->getOperand(1), m_Sub(m_Zero(), m_Specific(Select)));
  };

  // Collect the stores that are doing the swapping
  auto FindSwapStores = [&StoreInstructions](
                            SmallVector<StoreInst *, 2> &StoresVect,
                            BasicBlock &BB) {

    SmallVector<LoadInst *, 2> LoadsVect;

    // Find the loads
    for (auto &Inst : BB.instructionsWithoutDebug()) {

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
      StoreInstructions.insert(StrInst);
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

  // Return true if Store1 represents the starting point to swap (a or pb)
  // and Store2 represents the end point (pb or pn), or vice-versa. Else
  // return false. Basically, we are checking that the swaps are happening
  // either in the left side of the pivot (from a to pb) or the right side
  // of the pivot (from pb to pn).
  auto CheckSwapWithPointers = [IsPointingAtBeginning, IsPointingAtEnd]
      (StoreInst *Store1, StoreInst *Store2, Value *Start,
      Value *End, SelectInst *Sel) {

    if (IsPointingAtBeginning(Store1, Start) &&
        IsPointingAtEnd(Store2, End, Sel))
      return true;

    if (IsPointingAtBeginning(Store2, Start) &&
        IsPointingAtEnd(Store1, End, Sel))
      return true;

    return false;
  };

  unsigned SwapCounter = 0;
  Value *ArgValue = cast<Value>(Arg);

  bool LeftSideFound = false;
  bool RightSideFound = false;

  // Go through each of the basic blocks in the function and check
  // if there is a swap
  for (auto &BB : *F) {

    if (!isa<PHINode>(firstNonDbgInst(&BB)))
      continue;

    SmallVector<StoreInst *, 2> StoreVect;

    if (!FindSwapStores(StoreVect, BB))
      continue;

    LLVM_DEBUG({
      dbgs() << "QsortRec: Checking computation of swap in "
             << F->getName() <<"\n";
      BB.dump();
    });

    // The values that are swapped must be pointing at the left side
    // of the pivot (from a to pb) and the right side of the pivot
    // (from pb to pn)

    // Check for a to pb
    bool LocalLeftSide = CheckSwapWithPointers(StoreVect[0], StoreVect[1],
                                               ArgValue, PHIpb, SelectLeft);

    if (LocalLeftSide) {
      if (LeftSideFound)
        return 0;
      LeftSideFound = LocalLeftSide;
    }

    // Check for pb to pn
    bool LocalRightSide = CheckSwapWithPointers(StoreVect[0], StoreVect[1],
                                                PHIpb, GEPpn, SelectRight);

    if (LocalRightSide) {
      if (RightSideFound)
        return 0;
      RightSideFound = LocalRightSide;
    }

    // Not a candidate
    if (!LocalLeftSide && !LocalRightSide) {
      LLVM_DEBUG(dbgs() << "QsortRec: Computation of swap in "
                        << F->getName() << " FAILED Test.\n");
      continue;
    }

    // Both can't be true
    if (LocalLeftSide == LocalRightSide) {
      LLVM_DEBUG(dbgs() << "QsortRec: Computation of swap in "
                        << F->getName() << " FAILED Test.\n");

      return 0;
    }

    SwapCounter++;

    LLVM_DEBUG(dbgs() << "QsortRec: Computation of swap in "
                 << F->getName() << " PASSED Test.\n");
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
      dyn_cast<BranchInst>(EntryBB.getTerminator());

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
      dyn_cast<BranchInst>(EntryBB.getTerminator());

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
    LLVM_DEBUG(dbgs() << "QsortRec: Recursion in "
                      << F->getName() << " FAILED Test.\n");
    return 0;
  }

  LLVM_DEBUG(dbgs() << "QsortRec: Recursion in "
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
      LLVM_DEBUG(dbgs() << "QsortRec: Recursion in "
                        << F->getName() << " PASSED Test.\n");
      NumRecursions++;
    }
    else {
      LLVM_DEBUG(dbgs() << "QsortRec: Recursion in "
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
// This routine has the following outputs, that are passed back as "**"
// parameters and are stitched together at the next level by the function
// isPivotSorter():
//   BBLH: BasicBlock for Loop Header
//   BBLL: BasicBlock for Loop Latch
//   BBOE: BasicBlock for Outer Exit
//   BBIE: BasicBlock for Inner Exit
//   OPHIIn: Input Value for Outer PHINode Index
//   IPHIIn: Input Value for Inner PHINode Index
//   OPHIOut: Output Value for Outer PHINode Index
//   IPHIOut: Output Value for Inner PHINode Index
//   OPHIMid: Middle Value for Outer PHINode Index
//   LPHI: Limit PHI Value
//
static bool isPivotMover(BasicBlock *BBStart, Argument *ArgArray, bool IsUp,
                         BasicBlock **BBLH, BasicBlock **BBLL,
                         BasicBlock **BBOE, BasicBlock **BBIE,
                         PHINode **OPHIIn, PHINode **IPHIIn,
                         PHINode **OPHIOut, PHINode **IPHIOut,
                         PHINode **OPHIMid, PHINode **LPHI,
                         SetVector<StoreInst *> &StoreInstructions) {
  //
  // Return the first GetElementPtrInst in 'BB'.
  //
  auto FindGEPInBasicBlock = [](BasicBlock *BB) -> GetElementPtrInst * {
    for (auto &I : BB->instructionsWithoutDebug()) {
      auto GEPI = dyn_cast<GetElementPtrInst>(&I);
      if (GEPI)
        return GEPI;
    }
    return nullptr;
  };

  //
  // Find and validate the control flow of each of the BasicBlocks for the
  // loop of which 'BBStart' is the preheader. Those BasicBlocks are:
  //   BBLHeader: the loop header
  //   BBLTest: the internal loop test block
  //   BBLSwap: the loop block which swaps array elements
  //   BBLLatch: the loop latch block
  //   BBOuterExit: the outer exit from the loop
  //   BBInnerExit: the inner exit from the loop
  // This founction also sets the 'OuterGEPInc' and 'InnerGEPInc' which
  // are used to increment the outer and inner loop indices.
  // Return 'true' if the control flow is as desired and the GEPs for the
  // outer and inner loop indices are found.
  //
  auto ValidateCF = [&FindGEPInBasicBlock](BasicBlock *BBStart,
                                           BasicBlock **BBLHeader,
                                           BasicBlock **BBLTest,
                                           BasicBlock **BBLSwap,
                                           BasicBlock **BBLLatch,
                                           BasicBlock **BBOuterExit,
                                           BasicBlock **BBInnerExit,
                                           GetElementPtrInst **OuterGEPInc,
                                           GetElementPtrInst **InnerGEPInc)
                                           -> bool {
    auto BI0 = dyn_cast<BranchInst>(BBStart->getTerminator());
    if (!BI0 || BI0->isUnconditional())
      return false;
    *BBOuterExit = BI0->getSuccessor(0);
    *BBLHeader = BI0->getSuccessor(1);
    auto BI1 = dyn_cast<BranchInst>((*BBLHeader)->getTerminator());
    if (!BI1 || BI1->isUnconditional())
      return false;
    *BBLTest = BI1->getSuccessor(0);
    *BBInnerExit = BI1->getSuccessor(1);
    auto BI2 = dyn_cast<BranchInst>((*BBLTest)->getTerminator());
    if (!BI2 || BI2->isUnconditional())
      return false;
    *BBLSwap = BI2->getSuccessor(0);
    *BBLLatch = BI2->getSuccessor(1);
    auto BI3 = dyn_cast<BranchInst>((*BBLSwap)->getTerminator());
    if (!BI3 || !BI3->isUnconditional() || BI3->getSuccessor(0) != *BBLLatch)
      return false;
    auto BI4 = dyn_cast<BranchInst>((*BBLLatch)->getTerminator());
    if (!BI4 || BI4->isUnconditional() ||
        BI4->getSuccessor(0) != *BBOuterExit ||
        BI4->getSuccessor(1) != *BBLHeader)
      return false;
    *OuterGEPInc = FindGEPInBasicBlock(*BBLLatch);
    if (!*OuterGEPInc)
      return false;
    *InnerGEPInc = FindGEPInBasicBlock(*BBLSwap);
    if (!*InnerGEPInc)
      return false;
    return true;
  };

  //
  // Starting with 'GEPI' trace the evolution of the outer PHI through
  // 'BBStart', 'BBLHeader', 'BBLLatch', and 'BBOuterExit' to produce the
  // values 'OuterPHIIn', 'OuterPHIMid', and 'OuterPHIOut'.
  //
  // For example:
  //
  //           while (pb <= pc && (cmp_result = cmp(pb, a)) <= 0) {
  //                      if (cmp_result == 0) {
  //                              swap_cnt = 1;
  //                              swap(pa, pb);
  //                              pa += es;
  //                      }
  //                      pb += es;
  //            }
  // Here 'pb' is the outer PHI and 'OuterPHIIn' is its value on entry to
  // the loop, 'OuterPHIMid' is its value inside the loop, and 'OuterPHIOut'
  // is its value on exit from the loop.
  //
  auto ValidateOuterPHI = [](GetElementPtrInst *GEPI, BasicBlock *BBStart,
                             BasicBlock *BBLHeader, BasicBlock *BBLLatch,
                             BasicBlock *BBOuterExit, PHINode **OuterPHIIn,
                             PHINode **OuterPHIMid, PHINode **OuterPHIOut)
                             -> bool {
    for (User *U : GEPI->users()) {
      auto PHIN0 = dyn_cast<PHINode>(U);
      if (!PHIN0)
        continue;
      if (PHIN0->getParent() == BBOuterExit)
        *OuterPHIOut = PHIN0;
      else if (PHIN0->getParent() == BBLHeader)
        *OuterPHIMid = PHIN0;
      if (*OuterPHIOut && *OuterPHIMid)
        break;
    }
    // CMPLRLLVM-11027: Add debug info for LIT test of the fix below.
    // *OuterPHIMid must be non-nullptr, but *OuterPHIOut may be nullptr.
    LLVM_DEBUG(dbgs() << "*OuterPHIOut "
                      << (*OuterPHIOut ? "IS NOT NULL\n" : "IS NULL\n"));
    LLVM_DEBUG(dbgs() << "*OuterPHIMid "
                      << (*OuterPHIMid ? "IS NOT NULL\n" : "IS NULL\n"));
    if (!*OuterPHIMid)
      return false;
    Value *V = (*OuterPHIMid)->getIncomingValueForBlock(BBStart);
    auto PHIN2 = dyn_cast<PHINode>(V);
    if (!PHIN2)
      return false;
    *OuterPHIIn = PHIN2;
    if (GEPI->getPointerOperand() != *OuterPHIMid)
      return false;
    return true;
  };

  //
  // Starting with 'GEPI' trace the evolution of the inner PHI through
  // 'BBStart', 'BBLHeader', 'BBLLatch', and 'BBOuterExit' to produce the
  // values 'InnerPHIIn', 'InnerPHIMid', and 'InnerPHIOut'.
  //
  // For example:
  //
  //           while (pb <= pc && (cmp_result = cmp(pb, a)) <= 0) {
  //                      if (cmp_result == 0) {
  //                              swap_cnt = 1;
  //                              swap(pa, pb);
  //                              pa += es;
  //                      }
  //                      pb += es;
  //            }
  // Here 'pa' is the outer PHI and 'InnerPHIIn' is its value on entry to
  // the loop, 'InnerPHIMid' is its value inside the loop, and 'InnerPHIOut'
  // is its value on exit from the loop.
  //
  auto ValidateInnerPHI = [](GetElementPtrInst *GEPI, BasicBlock *BBStart,
                            BasicBlock *BBLHeader, BasicBlock *BBLLatch,
                            BasicBlock *BBOuterExit, PHINode **InnerPHIIn,
                            PHINode **InnerPHIMid, PHINode **InnerPHIOut)
                            -> bool {
    if (!GEPI->hasOneUse())
      return false;
    auto PHIM0 = dyn_cast<PHINode>(GEPI->user_back());
    if (!PHIM0 || PHIM0->getParent() != BBLLatch)
      return false;
    for (User *U :  PHIM0->users()) {
      auto PHIN0 = dyn_cast<PHINode>(U);
      if (!PHIN0)
        continue;
      if (PHIN0->getParent() == BBLHeader)
        *InnerPHIMid = PHIN0;
      else if (PHIN0->getParent() == BBOuterExit)
        *InnerPHIOut = PHIN0;
      if (*InnerPHIMid && *InnerPHIOut)
        break;
    }
    if (!*InnerPHIMid || !*InnerPHIOut)
      return false;
    Value *V = (*InnerPHIMid)->getIncomingValueForBlock(BBStart);
    auto PHIN1 = dyn_cast<PHINode>(V);
    if (!PHIN1)
      return false;
    *InnerPHIIn = PHIN1;
    if (GEPI->getPointerOperand() != *InnerPHIMid)
      return false;
    return true;
  };

  //
  // Validate 'BBStart', the initial BasicBlock which tests '*OuterPHIIn'
  // against '*LimitPHI'. Recognize and set '*LimitPHI'.
  //
  auto ValidateBBStart = [](BasicBlock *BBStart, bool IsUp,
                            PHINode *OuterPHIIn, PHINode **LimitPHI) -> bool {
    auto BI = cast<BranchInst>(BBStart->getTerminator());
    assert(BI->isConditional() && "Expecting conditional BranchInst");
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
    if (OuterPHIIn != OuterPHIBase)
      return false;
    return true;
  };

  //
  // Validate 'BBLHeader', which is loop header for the pivot mover loop.
  // Here the values of the elements at 'OuterPHIMid' and 'ArgArray' are
  // compared using the '*CIOut' comparison function, which is set. (Note
  // that the pivot element is the beginning of 'ArgArray' at this point in
  // time.)
  //
  auto ValidateBBLHeader = [](BasicBlock *BBLHeader, bool IsUp,
                              PHINode *OuterPHIMid, Argument *ArgArray,
                              CallInst **CIOut) -> bool {
    auto BI = cast<BranchInst>(BBLHeader->getTerminator());
    assert(BI->isConditional() && "Expecting conditional BranchInst");
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
    if (!BC0 || BC0->getOperand(0) != OuterPHIMid)
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
  // of the comparison of the array elements.
  //
  auto ValidateBBLTest = [](BasicBlock *BBLTest, CallInst *CIIn) -> bool {
    auto BI = cast<BranchInst>(BBLTest->getTerminator());
    assert(BI->isConditional() && "Expecting conditional BranchInst");
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
  // in the pivot mover loop. Recognize that it swaps the pivot and the array
  // element at 'OuterPHIMid' while advancing '*InnerPHIMid', which is
  // recognized and set.
  //
  auto ValidateBBLSwap = [&StoreInstructions](BasicBlock *BBLSwap, bool IsUp,
                                              PHINode *OuterPHIMid,
                                              PHINode *InnerPHIMid)
                                              -> bool {
    Value *VPHIN0 = nullptr;
    Value *VPHIN1 = nullptr;
    Instruction *NI = nullptr;
    if (!isSwapSequence(firstNonDbgInst(BBLSwap), &VPHIN0, &VPHIN1, &NI,
                        StoreInstructions))
      return false;
    auto PHIN0 = dyn_cast<PHINode>(VPHIN0);
    if (!PHIN0)
      return false;
    auto PHIN1 = dyn_cast<PHINode>(VPHIN1);
    if (!PHIN1)
      return false;
    if (IsUp) {
      if (PHIN1 != OuterPHIMid || PHIN0 != InnerPHIMid)
        return false;
    } else {
      if (PHIN0 != OuterPHIMid || PHIN1 != InnerPHIMid)
        return false;
    }
    auto GEP = dyn_cast<GetElementPtrInst>(NI);
    if (!GEP || GEP->getNumOperands() != 2)
      return false;
    auto PHIN2 = dyn_cast<PHINode>(GEP->getPointerOperand());
    if (!PHIN2 || PHIN2 != InnerPHIMid)
      return false;
    auto CI = dyn_cast<ConstantInt>(GEP->getOperand(1));
    auto CP = IsUp ? 8 : -8;
    if (!CI || CI->getSExtValue() != CP)
      return false;
    return true;
  };

  //
  // Validate 'ValidateBBLLatch', which is the loop latch block of the pivot
  // mover loop. Recognize that it advances 'OuterPHIMid', while testing it
  // against 'LimitPHI'.
  //
  auto ValidateBBLLatch = [](BasicBlock *BBLLatch, bool IsUp,
                             PHINode *OuterPHIMid, PHINode *LimitPHI) -> bool {
    auto BI = cast<BranchInst>(BBLLatch->getTerminator());
    assert(BI->isConditional() && "Expecting conditional BranchInst");
    auto IC = dyn_cast<ICmpInst>(BI->getCondition());
    if (!IC || IC->getPredicate() != CmpInst::ICMP_UGT)
      return false;
    auto GEPIndex = IsUp ? 0 : 1;
    auto PHIIndex = IsUp ? 1 : 0;
    auto CP = IsUp ? 8 : -8;
    auto GEP = dyn_cast<GetElementPtrInst>(IC->getOperand(GEPIndex));
    if (!GEP || GEP->getNumOperands() != 2)
      return false;
    if (GEP->getPointerOperand() != OuterPHIMid)
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
  PHINode *OuterPHIIn = nullptr;
  PHINode *InnerPHIIn = nullptr;
  PHINode *OuterPHIMid = nullptr;
  PHINode *InnerPHIMid = nullptr;
  PHINode *OuterPHIOut = nullptr;
  PHINode *InnerPHIOut = nullptr;
  PHINode *LimitPHI = nullptr;
  GetElementPtrInst *OuterGEPInc = nullptr;
  GetElementPtrInst *InnerGEPInc = nullptr;
  // Validate the control flow of the pivot mover.
  if (!ValidateCF(BBStart, &BBLHeader, &BBLTest, &BBLSwap, &BBLLatch,
      &BBOuterExit, &BBInnerExit, &OuterGEPInc, &InnerGEPInc))
    return false;
  // Validate the inner and outer PHIs.
  if (!ValidateOuterPHI(OuterGEPInc, BBStart, BBLHeader, BBLLatch,
      BBOuterExit, &OuterPHIIn, &OuterPHIMid, &OuterPHIOut))
    return false;
  if (!ValidateInnerPHI(InnerGEPInc, BBStart, BBLHeader, BBLLatch,
      BBOuterExit, &InnerPHIIn, &InnerPHIMid, &InnerPHIOut))
    return false;
  // Validate each of the five basic blocks in the pivot mover.
  if (!ValidateBBStart(BBStart, IsUp, OuterPHIIn, &LimitPHI))
    return false;
  if (!ValidateBBLHeader(BBLHeader, IsUp, OuterPHIMid, ArgArray, &CI))
    return false;
  if (!ValidateBBLTest(BBLTest, CI))
    return false;
  if (!ValidateBBLSwap(BBLSwap, IsUp, OuterPHIMid, InnerPHIMid))
    return false;
  if (!ValidateBBLLatch(BBLLatch, IsUp, OuterPHIMid, LimitPHI))
    return false;
  // Write back computed BasicBlocks and PHINodes neeeed by others.
  *BBLH = BBLHeader;
  *BBLL = BBLLatch;
  *BBOE = BBOuterExit;
  *BBIE = BBInnerExit;
  *OPHIIn = OuterPHIIn;
  *OPHIMid = OuterPHIMid;
  *OPHIOut = OuterPHIOut;
  *IPHIIn = InnerPHIIn;
  *IPHIOut = InnerPHIOut;
  *LPHI = LimitPHI;
  return true;
}

//
// Return 'true' if we recognize the pivot sorter loop of the qsort whose
// preheader is 'BBStart' and whose arguments are 'ArrayArg' and 'ArraySize'
// and whose pivot value is 'Pivot'.  The loop has the form:
//        for (;;) {
//                // Forward pivot mover loop ('IsUp' is true)
//                while (pb <= pc && (cmp_result = cmp(pb, a)) <= 0) {
//                        if (cmp_result == 0) {
//                                swap_cnt = 1;
//                                swap(pa, pb);
//                                pa += es;
//                        }
//                        pb += es;
//                }
//                // Backward pivot mover loop ("IsUp' is false)
//                while (pb <= pc && (cmp_result = cmp(pc, a)) >= 0) {
//                        if (cmp_result == 0) {
//                                swap_cnt = 1;
//                                swap(pc, pd);
//                                pd -= es;
//                        }
//                        pc -= es;
//                }
//                if (pb > pc)
//                        break;
//                swap(pb, pc);
//                swap_cnt = 1;
//                pb += es;
//                pc -= es;
//        }
// The values of 'pa', 'pb', 'pc', and 'pd' upon exit from the pivot sorter
// loop as set in '*PHIpa', '*PHIpb', '*PHIpc', and '*PHIpd', as well as the
// BasicBlock '*BBLargeSort', the BasicBlock where the sort continues, and
// BasicBlock '*BBSmallSort', the BasicBlock where the sort may switch to a
// small insertion sort.
//
static bool isPivotSorter(BasicBlock *BBStart, Argument *ArgArray,
                          Argument *ArgSize, Value *Pivot,
                          BasicBlock **BBSmallSort, BasicBlock **BBLargeSort,
                          PHINode **PHIpa, PHINode **PHIpb, PHINode **PHIpc,
                          PHINode **PHIpd,
                          SetVector<StoreInst *> &StoreInstructions) {
  //
  // Validate 'BBStart', the preheader of the pivot sorter loop, and set
  // '*BBForwardPH', the preheader of the forward pivot mover loop.
  //
  auto ValidateBBStart = [&StoreInstructions](BasicBlock *BBStart,
                            Argument *ArgArray, Value *Pivot,
                            BasicBlock **BBForwardPH) -> bool {
    auto BI = dyn_cast<BranchInst>(BBStart->getTerminator());
    if (!BI || !BI->isUnconditional())
      return false;
    *BBForwardPH = BI->getSuccessor(0);
    if (Pivot != firstNonDbgInst(BBStart))
      return false;
    Instruction *SSI = firstNonDbgInst(BBStart)->getNextNonDebugInstruction();
    Value *V0 = nullptr;
    Value *V1 = nullptr;
    Instruction *N1 = nullptr;
    if (!isSwapSequence(SSI, &V0, &V1, &N1, StoreInstructions))
      return false;
    if (!isPHINodeWithArgIncomingValue(V0, ArgArray))
      return false;
    if (V1 != Pivot)
      return false;
    return true;
  };

  //
  // Return 'true' if 'V' is equal to 'ArgArray' + 8.
  //
  auto IsGEPForLowInit = [](Value *V, Argument *ArgArray) -> bool {
    auto GEPI = dyn_cast<GetElementPtrInst>(V);
    if (!GEPI || GEPI->getNumOperands() != 2)
      return false;
    auto CI = dyn_cast<ConstantInt>(GEPI->getOperand(1));
    if (!CI || CI->getZExtValue() != 8)
      return false;
    if (!isPHINodeWithArgIncomingValue(GEPI->getPointerOperand(), ArgArray))
      return false;
    return true;
  };

  //
  // Return 'true' if 'V' is equal to 'ArgArray' + 8 * ('ArgSize' - 1).
  //
  auto IsGEPForHighInit = [](Value *V, Argument *ArgArray,
                             Argument *ArgSize) -> bool {
    auto GEPI = dyn_cast<GetElementPtrInst>(V);
    if (!GEPI || GEPI->getNumOperands() != 2)
      return false;
    Value *V1 = nullptr;
    Value *V2 = nullptr;
    ConstantInt *C1 = nullptr;
    ConstantInt *C2 = nullptr;
    if (!match(GEPI->getOperand(1), m_Add(m_Value(V1),  m_ConstantInt(C1))) ||
        C1->getSExtValue() != -8)
      return false;
    if (!match(V1, m_Shl(m_Value(V2), m_ConstantInt(C2))) ||
        C2->getZExtValue() != 3)
      return false;
    if (!isPHINodeWithArgIncomingValue(V2, ArgSize))
      return false;
    if (!isPHINodeWithArgIncomingValue(GEPI->getPointerOperand(), ArgArray))
      return false;
    return true;
  };

  //
  // Validate 'BBForwardPH', the preheader of the forward pivot mover loop.
  // The loop helps sort an array 'ArgArray' of size 'ArgSize'.  The indices
  // of the loop are set in 'BBInit'. This function sets the following key
  // values in the forward pivot mover loop:
  //   BBLH: BasicBlock of loop header
  //   BBLL: BasicBlock of loop latch
  //   BBBackwardPH: BasicBlock of preheader of backward pivot mover loop
  //     (which is also the exit BasicBlock of the forward pivot mover loop)
  //   PHIpaOut, PHIpbOut, PHIpcOut: Values of 'pa', 'pb', and 'pc' on exit
  // to the forward pivot mover loop.
  //
  auto ValidateBBForwardPH = [&IsGEPForLowInit, &IsGEPForHighInit,
                              &StoreInstructions](BasicBlock *BBForwardPH,
                                 Argument *ArgArray, Argument *ArgSize,
                                 BasicBlock *BBInit, BasicBlock **BBLH,
                                 BasicBlock **BBLL, BasicBlock **BBBackwardPH,
                                 PHINode **PHIpaOut, PHINode **PHIpbOut,
                                 PHINode **PHIpcOut)
                                 -> bool {
    BasicBlock *LocalBBLH = nullptr;
    BasicBlock *LocalBBLL = nullptr;
    BasicBlock *BBOuterExit = nullptr;
    BasicBlock *BBInnerExit = nullptr;
    PHINode *LocalPHIpaIn = nullptr;
    PHINode *LocalPHIpbIn = nullptr;
    PHINode *LocalPHIpaOut = nullptr;
    PHINode *LocalPHIpbOut = nullptr;
    PHINode *LocalPHIpbMid = nullptr;
    PHINode *LocalPHIpc = nullptr;
    // Check that a pivot mover loop starts here.
    if (!isPivotMover(BBForwardPH, ArgArray, true, &LocalBBLH,
        &LocalBBLL, &BBOuterExit, &BBInnerExit, &LocalPHIpbIn, &LocalPHIpaIn,
        &LocalPHIpbOut, &LocalPHIpaOut, &LocalPHIpbMid, &LocalPHIpc,
        StoreInstructions)) {
      LLVM_DEBUG(dbgs() << "QsortRec: Pivot Mover Candidate FAILED Test.\n");
      return false;
    }
    LLVM_DEBUG(dbgs() << "QsortRec: Pivot Mover Candidate PASSED Test.\n");
    if (BBOuterExit != BBInnerExit)
      return false;
    // Check the initialization of 'pa', 'pb', and 'pc'.
    Value *VPHIpaIn = LocalPHIpaIn->getIncomingValueForBlock(BBInit);
    if (!VPHIpaIn || !IsGEPForLowInit(VPHIpaIn, ArgArray))
      return false;
    Value *VPHIpbIn = LocalPHIpbIn->getIncomingValueForBlock(BBInit);
    if (VPHIpbIn != VPHIpaIn)
      return false;
    Value *VPHIpcIn = LocalPHIpc->getIncomingValueForBlock(BBInit);
    if (!VPHIpcIn || !IsGEPForHighInit(VPHIpcIn, ArgArray, ArgSize))
      return false;
    *BBLH = LocalBBLH;
    *BBLL = LocalBBLL;
    *BBBackwardPH = BBOuterExit;
    *PHIpaOut = LocalPHIpaOut;
    *PHIpbOut = LocalPHIpbOut;
    *PHIpcOut = LocalPHIpc;
    return true;
  };

  //
  // Validate 'BBBackwardPH', the preheader of the backward pivot mover loop.
  // The loop helps sort an array 'ArgArray' of size 'ArgSize'.  The indices
  // of the loop are set in 'BBInit'. The values of 'pb' and 'pc' coming into
  // the loop are 'PHIpb' and 'PHIpc'.  This function sets the following key
  // values in the forward pivot mover loop:
  //   BBExit: Exit BasicBlock of loop
  //   BBLoopBack: Block which loops back to the loop header. It contains
  //     the swap of 'pb' and 'pc'
  //   BBLH: BasicBlock of loop header
  //   BBLL: BasicBlock of loop latch
  //   BBBackwardPH: BasicBlock of preheader of backward pivot mover loop
  //     (which is also the exit BasicBlock of the forward pivot mover loop)
  //   PHIpaOut, PHIpbOut, PHIpcOut: Values of 'pa', 'pb', and 'pc' on exit
  // to the forward pivot mover loop.
  //
  auto ValidateBBBackwardPH = [&IsGEPForHighInit, &StoreInstructions](
                                  BasicBlock *BBBackwardPH,
                                  Argument *ArgArray, Argument *ArgSize,
                                  BasicBlock *BBInit, PHINode *PHIpb,
                                  PHINode *PHIpc, BasicBlock **BBExit,
                                  BasicBlock **BBLoopBack, BasicBlock **BBLH,
                                  BasicBlock **BBLL, PHINode **PHIpcOut,
                                  PHINode **PHIpcMid, PHINode **PHIpdOut)
                                  -> bool {
    BasicBlock *LocalBBLH = nullptr;
    BasicBlock *LocalBBLL = nullptr;
    BasicBlock *BBOuterExit = nullptr;
    BasicBlock *BBInnerExit = nullptr;
    PHINode *LocalPHIpb = nullptr;
    PHINode *LocalPHIpcIn = nullptr;
    PHINode *LocalPHIpcOut = nullptr;
    PHINode *LocalPHIpcMid = nullptr;
    PHINode *LocalPHIpdIn = nullptr;
    PHINode *LocalPHIpdOut = nullptr;
    // Check that a pivot mover loop starts here.
    if (!isPivotMover(BBBackwardPH, ArgArray, false, &LocalBBLH,
        &LocalBBLL, &BBOuterExit, &BBInnerExit, &LocalPHIpcIn, &LocalPHIpdIn,
        &LocalPHIpcOut, &LocalPHIpdOut, &LocalPHIpcMid, &LocalPHIpb,
        StoreInstructions)) {
      LLVM_DEBUG(dbgs() << "QsortRec: Pivot Mover Candidate FAILED Test.\n");
      return false;
    }
    LLVM_DEBUG(dbgs() << "QsortRec: Pivot Mover Candidate PASSED Test.\n");
    // Check that the values of 'pb' and 'pc' out of the forward pivot mover
    // loop are transmitted into the backward pivot mover loop
    if (LocalPHIpb != PHIpb || LocalPHIpcIn != PHIpc)
      return false;
    // Check the initialization of 'pd'.
    Value *VPHIpd = LocalPHIpdIn->getIncomingValueForBlock(BBInit);
    if (!IsGEPForHighInit(VPHIpd, ArgArray, ArgSize))
      return false;
    *BBExit = BBOuterExit;
    *BBLoopBack = BBInnerExit;
    *BBLH = LocalBBLH;
    *BBLL = LocalBBLL;
    *PHIpcMid = LocalPHIpcMid;
    *PHIpcOut = LocalPHIpcOut;
    *PHIpdOut = LocalPHIpdOut;
    return true;
  };

  //
  // Validate 'BBLoopBack', which swaps 'pb' (in 'PHIpb') and 'pc' (in 'PHIpc')
  // and increments/decrements their values. Also verify that 'BBForwardPH' is
  // the unique successor of 'BBLoopBack'.
  //
  auto ValidateBBLoopBack = [&ArgArray, &ArgSize, &StoreInstructions]
                                (BasicBlock *BBLoopBack,
                                BasicBlock *BBForwardPH,
                                PHINode *PHIpb, PHINode *PHIpc) -> bool {
    auto BI = dyn_cast<BranchInst>(BBLoopBack->getTerminator());
    if (!BI || !BI->isUnconditional() || BI->getSuccessor(0) != BBForwardPH)
      return false;
    // Check the swap of 'pb' and 'pc'.
    Value *V0 = nullptr;
    Value *V1 = nullptr;
    Instruction *NI = nullptr;
    if (!isSwapSequence(firstNonDbgInst(BBLoopBack), &V0, &V1, &NI,
        StoreInstructions))
      return false;
    // Check the increment of 'pb' and the decrement of 'pc'.
    auto PHIN0 = dyn_cast<PHINode>(V0);
    if (!PHIN0 || PHIN0 != PHIpb)
      return false;
    auto PHIN1 = dyn_cast<PHINode>(V1);
    if (!PHIN1 || PHIN1 != PHIpc)
      return false;
    auto GEPIUp = dyn_cast<GetElementPtrInst>(NI);
    if (!GEPIUp || GEPIUp->getNumOperands() != 2 ||
        GEPIUp->getPointerOperand() != PHIN0)
      return false;
    auto CIUp = dyn_cast<ConstantInt>(GEPIUp->getOperand(1));
    if (!CIUp || CIUp->getSExtValue() != 8)
      return false;
    auto II = NI->getNextNonDebugInstruction();
    auto GEPIDown = dyn_cast_or_null<GetElementPtrInst>(II);
    if (!GEPIDown || GEPIDown->getNumOperands() != 2 ||
        GEPIDown->getPointerOperand() != PHIN1)
      return false;
    auto CIDown = dyn_cast<ConstantInt>(GEPIDown->getOperand(1));
    if (!CIDown || CIDown->getSExtValue() != -8)
      return false;
    return true;
  };

  //
  // Validate 'BBEXit', the exit block of the pivot sorter loop. This block
  // tests 'swap_cnt == 0'. We validate that 'swap_cnt' has the expected value
  // in each of the BasicBlocks of the pivot sorter loop which can set its
  // value: 'BBForwardPH', 'BBForwardLH', 'BBForwardLL', 'BBBackwardPH',
  // 'BBBackwardLH', 'BBBackwardLL', and 'BBLoopBack'. If 'BBExit' passes
  // validation, set '*BBSmallSort' the true branch of 'BBExit' and
  // '*BBLargeSort' to the false branch of 'BBExit'
  //
  auto ValidateBBExit = [](BasicBlock *BBExit,
                           BasicBlock *BBForwardPH, BasicBlock *BBForwardLH,
                           BasicBlock *BBForwardLL, BasicBlock *BBBackwardPH,
                           BasicBlock *BBBackwardLH, BasicBlock *BBBackwardLL,
                           BasicBlock *BBLoopBack, BasicBlock **BBSmallSort,
                           BasicBlock **BBLargeSort) -> bool {
    // Check 'swap_cnt == 0'.
    auto BI = dyn_cast<BranchInst>(BBExit->getTerminator());
    if (!BI || BI->isUnconditional())
      return false;
    ICmpInst::Predicate Pred;
    Value *VPHIN0 = nullptr;
    if (!match(BI->getCondition(), m_ICmp(Pred, m_Value(VPHIN0), m_Zero())))
      return false;
    if (Pred != CmpInst::ICMP_EQ)
      return false;
    // PHIN0: %228 = phi i32 [ %216, %215 ], [ %195, %194 ]
    auto PHIN0 = dyn_cast<PHINode>(VPHIN0);
    if (!PHIN0 || PHIN0->getNumIncomingValues() != 2 ||
        PHIN0->getParent() != BBExit)
      return false;
    // PHIN1: %216 = phi i32 [ 1, %209 ], [ %202, %207 ]
    auto PHIN1 = dyn_cast<PHINode>(PHIN0->getIncomingValue(0));
    if (!PHIN1 || PHIN0->getNumIncomingValues() != 2 ||
        PHIN1->getParent() != BBBackwardLL)
      return false;
    // PHIN2: %195 = phi i32 [ %167, %166 ], [ %176, %173 ], [ %190, %189 ]
    auto PHIN2 = dyn_cast<PHINode>(PHIN0->getIncomingValue(1));
    if (!PHIN2 || PHIN2->getNumIncomingValues() != 3 ||
        PHIN2->getParent() != BBBackwardPH)
      return false;
    auto C0 = dyn_cast<ConstantInt>(PHIN1->getIncomingValue(0));
    if (!C0 || !C0->isOne())
      return false;
    // PHIN3: %202 = phi i32 [ %216, %215 ], [ %195, %194 ]
    auto PHIN3 = dyn_cast<PHINode>(PHIN1->getIncomingValue(1));
    if (!PHIN3 || PHIN3->getNumIncomingValues() != 2 ||
        PHIN3->getParent() != BBBackwardLH)
      return false;
    // PHIN4: %167 = phi i32 [ 0, %156 ], [ 1, %220 ]
    auto PHIN4 = dyn_cast<PHINode>(PHIN2->getIncomingValue(0));
    if (!PHIN4 || PHIN4->getNumIncomingValues() != 2 ||
        PHIN4->getParent() != BBForwardPH)
      return false;
    // PHIN5: %176 = phi i32 [ %190, %189 ], [ %167, %166 ]
    auto PHIN5 = dyn_cast<PHINode>(PHIN2->getIncomingValue(1));
    if (!PHIN5 || PHIN5->getNumIncomingValues() != 2 ||
        PHIN5->getParent() != BBForwardLH)
      return false;
    // PHIN6: %190 = phi i32 [ 1, %183 ], [ %176, %181 ]
    auto PHIN6 = dyn_cast<PHINode>(PHIN2->getIncomingValue(2));
    if (!PHIN6 || PHIN6->getNumIncomingValues() != 2 ||
        PHIN6->getParent() != BBForwardLL)
      return false;
    auto PHIN7 = dyn_cast<PHINode>(PHIN3->getIncomingValue(0));
    if (!PHIN7 || PHIN7 != PHIN1)
      return false;
    auto PHIN8 = dyn_cast<PHINode>(PHIN3->getIncomingValue(1));
    if (!PHIN8 || PHIN8 != PHIN2)
      return false;
    auto C1 = dyn_cast<ConstantInt>(PHIN4->getIncomingValue(0));
    if (!C1 || !C1->isZero() || PHIN4->getIncomingBlock(0) == BBLoopBack)
      return false;
    auto C2 = dyn_cast<ConstantInt>(PHIN4->getIncomingValue(1));
    if (!C2 || !C2->isOne() || PHIN4->getIncomingBlock(1) != BBLoopBack)
      return false;
    auto PHIN9 = dyn_cast<PHINode>(PHIN5->getIncomingValue(0));
    if (!PHIN9 || PHIN9 != PHIN6)
      return false;
    auto PHIN10 = dyn_cast<PHINode>(PHIN5->getIncomingValue(1));
    if (!PHIN10 || PHIN10 != PHIN4)
      return false;
    auto C3 = dyn_cast<ConstantInt>(PHIN6->getIncomingValue(0));
    if (!C3 || !C3->isOne())
      return false;
    auto PHIN11 = dyn_cast<PHINode>(PHIN6->getIncomingValue(1));
    if (!PHIN11 || PHIN11 != PHIN5)
      return false;
    *BBSmallSort = BI->getSuccessor(0);
    *BBLargeSort = BI->getSuccessor(1);
    return true;
  };

  // Main code for isPivotSorter().
  BasicBlock *BBForwardPH = nullptr;
  BasicBlock *BBForwardLH = nullptr;
  BasicBlock *BBForwardLL = nullptr;
  BasicBlock *BBBackwardPH = nullptr;
  BasicBlock *BBBackwardLH = nullptr;
  BasicBlock *BBBackwardLL = nullptr;
  BasicBlock *BBLoopBack = nullptr;
  BasicBlock *BBExit = nullptr;
  BasicBlock *LocalBBSmallSort = nullptr;
  BasicBlock *LocalBBLargeSort = nullptr;
  PHINode *LocalPHIpa = nullptr;
  PHINode *LocalPHIpb = nullptr;
  PHINode *LocalPHIpc = nullptr;
  PHINode *LocalPHIpcOut = nullptr;
  PHINode *LocalPHIpcMid = nullptr;
  PHINode *LocalPHIpdOut = nullptr;
  // Validate each of the five basic blocks in the pivot sorter.
  if (!ValidateBBStart(BBStart, ArgArray, Pivot, &BBForwardPH))
    return false;
  if (!ValidateBBForwardPH(BBForwardPH, ArgArray, ArgSize, BBStart,
      &BBForwardLH, &BBForwardLL, &BBBackwardPH, &LocalPHIpa, &LocalPHIpb,
      &LocalPHIpc))
    return false;
  if (!ValidateBBBackwardPH(BBBackwardPH, ArgArray, ArgSize, BBStart,
      LocalPHIpb, LocalPHIpc, &BBExit, &BBLoopBack, &BBBackwardLH,
      &BBBackwardLL, &LocalPHIpcOut, &LocalPHIpcMid, &LocalPHIpdOut))
    return false;
  if (!ValidateBBLoopBack(BBLoopBack, BBForwardPH, LocalPHIpb, LocalPHIpcMid))
    return false;
  if (!ValidateBBExit(BBExit, BBForwardPH, BBForwardLH, BBForwardLL,
      BBBackwardPH, BBBackwardLH, BBBackwardLL, BBLoopBack, &LocalBBSmallSort,
      &LocalBBLargeSort))
    return false;
  *BBSmallSort = LocalBBSmallSort;
  *BBLargeSort = LocalBBLargeSort;
  *PHIpa = LocalPHIpa;
  *PHIpb = LocalPHIpb;
  *PHIpc = LocalPHIpcOut;
  *PHIpd = LocalPHIpdOut;
  return true;
}


//
// Return 'true' if 'F' is recognized as a qsort like the one that appears in
// standard C library.
//
static bool isQsort(Function *F) {

  // Any array smaller than this size will be sorted by insertion sort.
  const unsigned SmallSize = 7;

  //
  // Return 'true' if the entry block of 'F' tests if 'ArgSize' is less than
  // 'SmallSize'. If so, set '*BBSmallSort' to the true branch out of the entry
  // block and '*BBLargeSort' to the false branch out of the entry block.
  //
  auto IsSmallCountTest1 = [](Function *F, Argument *ArgSize,
                              unsigned SmallSize, BasicBlock **BBSmallSort,
                              BasicBlock **BBLargeSort) -> bool {
    BasicBlock &BBEntry = F->getEntryBlock();
    auto BI = dyn_cast<BranchInst>(BBEntry.getTerminator());
    if (!BI || BI->isUnconditional())
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
  // Return 'true' if 'BBExit' terminates in a test "8 * 'ArgSize' > 8".
  // (This test ensures that the algorithm returns if 'ArgSize' <= 1,
  // since there is no sorting left to do.) Set '*BBSort' to the true
  // successor of 'BBExit' and '*BBReturn' to the false successor of 'BBExit'.
  //
  auto IsSmallCountTest2 = [](BasicBlock *BBExit, Argument *ArgSize,
                              BasicBlock **BBSort, BasicBlock **BBReturn)
                              -> bool {
    BasicBlock *BBLocalExit = nullptr;
    auto BI = dyn_cast<BranchInst>(BBExit->getTerminator());
    if (!BI || BI->isUnconditional())
      return false;
    ICmpInst::Predicate Pred;
    Value *V0 = nullptr;
    ConstantInt *C0 = nullptr;
    ConstantInt *C1 = nullptr;
    if (!match(BI->getCondition(), m_ICmp(Pred, m_Shl(m_Value(V0),
        m_ConstantInt(C0)), m_ConstantInt(C1))) || C0->getZExtValue() != 3 ||
        C1->getZExtValue() != 8)
      return false;
    if (Pred != CmpInst::ICMP_SGT)
      return false;
    if (!isPHINodeWithArgIncomingValue(V0, ArgSize))
      return false;
    BBLocalExit = BI->getSuccessor(0);
    auto BIX = dyn_cast<BranchInst>(BBLocalExit->getTerminator());
    bool SkipABlock = false;
    if (BIX && BIX->isUnconditional()) {
      SkipABlock = true;
      for (auto &I : BBLocalExit->instructionsWithoutDebug())
        if (I.mayWriteToMemory()) {
          SkipABlock = false;
          break;
        }
    }
    *BBSort = SkipABlock ? BIX->getSuccessor(0) : BI->getSuccessor(0);
    *BBReturn = BI->getSuccessor(1);
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
    auto BI = dyn_cast<BranchInst>(BBTest->getTerminator());
    if (!BI || BI->isUnconditional())
      return nullptr;
    auto IC = dyn_cast<ICmpInst>(BI->getCondition());
    if (!IC || IC->getPredicate() != ICmpInst::ICMP_SGT)
      return nullptr;
    auto CI = dyn_cast<ConstantInt>(IC->getOperand(1));
    if (!CI || CI->getZExtValue() != SmallSize+1)
      return nullptr;
    BasicBlock *BBS = BI->getSuccessor(0);
    auto BIT = dyn_cast<BranchInst>(BBS->getTerminator());
    if (!BIT)
      return nullptr;
    return BIT->isUnconditional() ? BIT->getSuccessor(0) : BBS;
  };

  // Return the number of insertion sorts recognized.
  auto CountInsertionSorts = [&FindInsertionSortCandidate](Function *F,
                                 Argument *ArgArray, Argument *ArgSize,
                                 unsigned SmallSize,
                                 SetVector<StoreInst*> &StoreInstructions)
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
        if (!isInsertionSort(BBStart, ArgArray, ArgSize, StoreInstructions)) {
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
    auto BI = dyn_cast<BranchInst>(BBTest->getTerminator());
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
                               Argument *ArgArray, Argument *ArgSize,
                               SetVector<StoreInst *> &StoreInstructions)
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
        BasicBlock *LocalBBLH = nullptr;
        BasicBlock *LocalBBLL = nullptr;
        BasicBlock *BBOuterExit = nullptr;
        BasicBlock *BBInnerExit = nullptr;
        PHINode *LocalPHIpb = nullptr;
        PHINode *LocalPHIpcIn = nullptr;
        PHINode *LocalPHIpcOut = nullptr;
        PHINode *LocalPHIpcMid = nullptr;
        PHINode *LocalPHIpdIn = nullptr;
        PHINode *LocalPHIpdOut = nullptr;
        if (!isPivotMover(BBStart, ArgArray, IsUp, &LocalBBLH,
            &LocalBBLL, &BBOuterExit, &BBInnerExit, &LocalPHIpcIn,
            &LocalPHIpdIn, &LocalPHIpcOut, &LocalPHIpdOut, &LocalPHIpcMid,
            &LocalPHIpb, StoreInstructions)) {
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

  // Return true if the computation of MIN was successful. Also
  // collect the Select instruction that represents the left side of the
  // pivot and the right side of the pivot.
  auto FindMINComputation = [F](PHINode *PHIpa, PHINode *PHIpb,
                                PHINode *PHIpc, PHINode *PHIpd,
                                SmallVector<Value *, 2> &NewSizesVector,
                                Argument *ArgArray, Argument *ArgSize,
                                GetElementPtrInst **GEPpnOut,
                                SelectInst **SelectLeft,
                                SelectInst **SelectRight) {

    unsigned MINCount = countMinComputations(F, ArgArray, ArgSize, PHIpa,
                                             PHIpb, PHIpc, PHIpd, GEPpnOut,
                                             NewSizesVector, SelectLeft,
                                             SelectRight);

    return MINCount == 2;
  };

  // Return true if swapping the vectors before and after the pivot
  // was found
  auto FindSwapVect = [F](PHINode *PHIpb, GetElementPtrInst *GEPpn,
                          Argument *ArgArray, SelectInst *SelectLeft,
                          SelectInst *SelectRight,
                          SetVector<StoreInst *> &StoreInstructions) {

    unsigned SwapCount = countSwapComputations(F, ArgArray, PHIpb, GEPpn,
                                               SelectLeft, SelectRight,
                                               StoreInstructions);

    return SwapCount == 2;
  };

  // Return true if the recursions were found
  auto FindRecursions = [F](Argument *ArgArray, Argument *ArgSize,
                            SmallVector<Value *, 2> &NewSizesVector) {

    unsigned RecCount = countRecursions(F, ArgArray, ArgSize, NewSizesVector);

    return RecCount == 2;
  };

  // Main code for isQsort().
  // Exclude obvious cases.
  if (F->isDeclaration() || F->isVarArg() || F->arg_size() != 2)
    return false;
  Argument *ArgArray = F->getArg(0);
  Argument *ArgSize = F->getArg(1);
  BasicBlock *BBSmallSort = nullptr;
  BasicBlock *BBSmallTest = nullptr;
  BasicBlock *BBLargeSort = nullptr;

  // This vector will hold the new sizes for the recursion. Entry 0
  // represents the size used in the direct recursion and entry 1
  // represents the size used in the tail recursion.
  SmallVector<Value *, 2> NewSizesVector;
  PHINode *PHIpa = nullptr;
  PHINode *PHIpb = nullptr;
  PHINode *PHIpc = nullptr;
  PHINode *PHIpd = nullptr;
  SelectInst *SelectLeft = nullptr;
  SelectInst *SelectRight = nullptr;
  GetElementPtrInst *GEPpn = nullptr;

  SetVector<StoreInst *> StoreInstructions;

  BasicBlock *BBReturn = nullptr;
  // Check that all PHINodes that include the arguments are equivalent.
  if (!AllPHINodesEquivalent(ArgArray) || !AllPHINodesEquivalent(ArgSize)) {
    LLVM_DEBUG(dbgs() << "QsortRec: All Arg PHI nodes not equivalent in "
                      << F->getName() << "\n");
    return false;
  }
  // Validate that the code branches to special case (insertion sort) for
  // sufficiently small arrays.
  if (!IsSmallCountTest1(F, ArgSize, SmallSize, &BBSmallTest, &BBLargeSort)) {
    LLVM_DEBUG(dbgs() << "QsortRec: Could not find small count test in "
                      << F->getName() << "\n");
    return false;
  }
  // Unit tests for various pieces of qsort recognition
  if (QsortUnitTest) {
    bool SawFailure = false;
    if (QsortTestInsert || QsortTestStoreSwaps) {
      unsigned ISCount = CountInsertionSorts(F, ArgArray, ArgSize, SmallSize,
                                             StoreInstructions);
      SawFailure |= ISCount < 2;
    }
    if (QsortTestPivot)
      SawFailure |= !qsortPivot(BBLargeSort, ArgArray, ArgSize);
    if (QsortTestPivotMovers || QsortTestStoreSwaps) {
      unsigned PMCount = CountPivotMovers(F, ArgArray, ArgSize,
                                          StoreInstructions);
      SawFailure |= PMCount < 2;
    }
    if (QsortTestPivotSorter || QsortTestStoreSwaps ||
        QsortTestMin || QsortTestRecursion || QsortTestSwap) {
      Value *Pivot = qsortPivot(BBLargeSort, ArgArray, ArgSize);
      BasicBlock *BBStart = cast<Instruction>(Pivot)->getParent();
      BasicBlock *BBSmallTest = nullptr;
      BasicBlock *BBLargeSort = nullptr;
      bool NewFailure = !isPivotSorter(BBStart, ArgArray, ArgSize, Pivot,
        &BBSmallTest, &BBLargeSort, &PHIpa, &PHIpb, &PHIpc, &PHIpd,
        StoreInstructions);
      SawFailure |= NewFailure;
      LLVM_DEBUG({
        if (NewFailure)
          dbgs() << "QsortRec: PivotSorter in "
                 << F->getName() << " FAILED Test.\n";
        else
          dbgs() << "QsortRec: PivotSorter in "
                 << F->getName() << " PASSED Test.\n";
      });
    }
    if (QsortTestMin || QsortTestStoreSwaps ||
        QsortTestRecursion || QsortTestSwap) {
      unsigned MINCount = countMinComputations(F, ArgArray, ArgSize, PHIpa,
                                               PHIpb, PHIpc, PHIpd, &GEPpn,
                                               NewSizesVector, &SelectLeft,
                                               &SelectRight);
      SawFailure |= MINCount < 2;
    }
    if (QsortTestSwap || QsortTestStoreSwaps) {
      unsigned SwapCount = countSwapComputations(F, ArgArray, PHIpb, GEPpn,
                                                 SelectLeft, SelectRight,
                                                 StoreInstructions);
      SawFailure |= SwapCount < 2;
    }
    // Note: The store instructions are collected during the swaps, therefore
    // we need to enable the tests for the insertion sort, pivot movers,
    // pivot sorter and the vect swaps to compute the store instructions.
    if (QsortTestStoreSwaps) {
      SawFailure |= !allStoresAreSwaps(F, StoreInstructions);
    }
    if (QsortTestRecursion) {
      unsigned RecCount = countRecursions(F, ArgArray, ArgSize, NewSizesVector);
      SawFailure |= RecCount < 2;
    }
    if (QsortTestCompareFunc)
      SawFailure |= !findCompareFunction(F);
    LLVM_DEBUG({
      if (SawFailure)
        dbgs() << "FAILED QSORT RECOGNITION UNIT TESTS\n";
      else
        dbgs() << "PASSED QSORT RECOGNITION UNIT TESTS\n";
    });
    return !SawFailure;
  }
  if (!IsSmallCountTest2(BBSmallTest, ArgSize, &BBSmallSort, &BBReturn)) {
    LLVM_DEBUG(dbgs() << "QsortRec: Could not find small count test in "
                      << F->getName() << "\n");
    return false;
  }
  // Check that when array is small enough, an insertion sort is performed.
  if (!isInsertionSort(BBSmallSort, ArgArray, ArgSize, StoreInstructions)) {
    LLVM_DEBUG(dbgs() << "QsortRec: Insertion Sort Candidate in "
                      << F->getName() << " FAILED Test.\n");
    return false;
  }
  LLVM_DEBUG(dbgs() << "QsortRec: Insertion Sort Candidate in "
                    << F->getName() << " PASSED Test.\n");
  // Find the pivot for the qsort.
  Value *Pivot = qsortPivot(BBLargeSort, ArgArray, ArgSize);
  BasicBlock *BBStart = cast<Instruction>(Pivot)->getParent();

  // Find the sorting process
  if (!isPivotSorter(BBStart, ArgArray, ArgSize, Pivot, &BBSmallTest,
      &BBLargeSort, &PHIpa, &PHIpb, &PHIpc, &PHIpd, StoreInstructions)) {
    LLVM_DEBUG(dbgs() << "QsortRec: PivotSorter in "
                      << F->getName() << " FAILED Test.\n");
    return false;
  }
  LLVM_DEBUG(dbgs() << "QsortRec: PivotSorter in "
                    << F->getName() << " PASSED Test.\n");
  if (!IsSmallCountTest2(BBSmallTest, ArgSize, &BBSmallSort, &BBReturn)) {
    LLVM_DEBUG(dbgs() << "QsortRec: Could not find small count test in "
                      << F->getName() << "\n");
    return false;
  }

  // Check the insertion sort for small arrays
  if (!isInsertionSort(BBSmallSort, ArgArray, ArgSize, StoreInstructions)) {
    LLVM_DEBUG(dbgs() << "QsortRec: Insertion Sort Candidate in "
                    << F->getName() << " FAILED Test.\n");
    return false;
  }

  // Find that the compares in the left side and the right side of the pivot
  if (!FindMINComputation(PHIpa, PHIpb, PHIpc, PHIpd, NewSizesVector, ArgArray,
                          ArgSize, &GEPpn, &SelectLeft, &SelectRight)) {
    LLVM_DEBUG(dbgs() << "QsortRec: Computation of MIN in "
                      << F->getName() << " FAILED Test.\n");
    return false;
  }

  // Check that the swaps in the left side and the right side of the pivot
  // happen
  if (!FindSwapVect(PHIpb, GEPpn, ArgArray, SelectLeft, SelectRight,
                    StoreInstructions)) {
    LLVM_DEBUG(dbgs() << "QSortRec: Swap Vectors in "
                      << F->getName() << " FAILED Test.\n");
    return false;
  }

  // Collect the recursions (direct and tail)
  if (!FindRecursions(ArgArray, ArgSize, NewSizesVector)) {
    LLVM_DEBUG(dbgs() << "QSortRec: Recursions in "
                      << F->getName() << " FAILED Test.\n");
    return false;
  }

  // All store instructions must be swaps
  if (!allStoresAreSwaps(F, StoreInstructions)) {
    LLVM_DEBUG(dbgs() << "QSortRec: Check if Store Instructions are swaps in "
                      << F->getName() << " FAILED Test.\n");
    return false;
  }

  // Check that the only called functions are the compare function and F itself
  if (!findCompareFunction(F)) {
    LLVM_DEBUG(dbgs() << "QSortRec: Compare Function in "
                      << F->getName() << " FAILED Test.\n");
    return false;
  }

  LLVM_DEBUG(dbgs() << "QsortRec: Insertion Sort Candidate in "
                    << F->getName() << " PASSED Test.\n");
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
