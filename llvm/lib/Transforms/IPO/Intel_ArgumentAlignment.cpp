//===---- Intel_ArgumentAlignment.cpp - Intel Compute Alignment      -*----===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
// This pass checks if the alignment of the argument is being computed inside
// a function. If the alignemnt is a known constant at compile time, then
// it tries to do a constant replacement. After this pass runs, the instruction
// simplification and the call graph simplification passes will run to do a
// constant propapagation. For example, consider the following function:
//
// define internal fastcc void @foo(i8*, i64) {
// entry:
//   %2 = ptrtoint i8* %0 to i64
//   %3 = and i64 %2, 7
//   %4 = icmp eq i64 %3, 0
//   br i1 %4, label %if_bb, label %end
//
// if_bb:
//   %5 = phi i8* [ %0, %entry ], [ %8, %else_bb ]
//   %6 = phi i1  [ %4, %entry ], [ %11, %else_bb ]
//   br i1 %6, label %else_bb, label %end
//
// else_bb:
//   %7 = sub i64 0, %1
//   %8 = getelementptr inbounds i8, i8* %5, i64 %7
//   %9 = ptrtoint i8* %8 to i64
//   %10 = and i64 %9, 7
//   %11 = icmp eq i64 %10, 0
//   %12 = lshr i64 %7, 3
//   %13 = icmp ult i64 %12, 7
//   br i1 %13, label %if_bb, label %end
//
// end:
//   ret void
// }
//
// define void @bar(i64, i64) {
// entry:
//   %2 = tail call noalias i8* @calloc(i64 %0, i64 8)
//   tail call fastcc void @foo(i8* %2, i64 %1)
//   ret void
// }
//
// The argument %0 in the above example is used to compute a MOD operation
// (%2 to %4 and %9 to %11). From the call site to @foo in @bar we can see
// that the actual argument (%2 in @bar) is and allocation with size 8. This
// means that memory for %2 in @bar will be aligned by 8 bytes, therefore the
// pointer-to-integer in %2 at @foo will be a multiple of 8. In simple words,
// the AND operation in %3 will always be 0 and %4 will always be true. The
// same happens for the AND in %10 and the icmp in %11.
//
// This pass will check the allocation site for the actual argument (%1 in
// @bar from the exmaple), if it is constant across the whole program and
// the alignment is computed then replace the variable in the AND operation
// with a constant. Function @foo will look as follows after the transformation.
//
// define internal fastcc void @foo(i8* %0, i64 %1) #0 {
// entry:
//   %2 = ptrtoint i8* %0 to i64
//   %3 = and i64 8, 7
//   %4 = icmp eq i64 %3, 0
//   br i1 %4, label %if_bb, label %end
//
// if_bb:                                   ; preds = %else_bb, %entry
//   %5 = phi i8* [ %0, %entry ], [ %8, %else_bb ]
//   %6 = phi i1 [ %4, %entry ], [ %11, %else_bb ]
//   br i1 %6, label %else_bb, label %end
//
// else_bb:                                 ; preds = %if_bb
//   %7 = sub i64 0, %1
//   %8 = getelementptr inbounds i8, i8* %5, i64 %7
//   %9 = ptrtoint i8* %8 to i64
//   %10 = and i64 8, 7
//   %11 = icmp eq i64 %10, 0
//   %12 = lshr i64 %7, 3
//   %13 = icmp ult i64 %12, 7
//   tail call fastcc void @foo(i8* %8, i64 %7)
//   br i1 %13, label %if_bb, label %end
//
// end:                                     ; preds = %else_bb, %if_bb, %entry
//   ret void
// }
//
// Notice now that variables in %3 and %10 were replaced with a constant 8.
// After this pass we run the instruction simplification and call graph
// simplification to clean up the IR.
//
// define internal fastcc void @foo(i8* %0, i64 %1) #0 {
// entry:
//   br label %if_bb
//
// if_bb:                                            ; preds = %entry, %if_bb
//   %2 = phi i8* [ %0, %entry ], [ %4, %if_bb ]
//   %3 = sub i64 0, %1
//   %4 = getelementptr inbounds i8, i8* %2, i64 %3
//   %5 = lshr i64 %3, 3
//   %6 = icmp ult i64 %5, 7
//   br i1 %6, label %if_bb, label %end
//
// end:                                              ; preds = %if_bb
//   ret void
// }
//
// Now @foo got simplified to a function with fewer branches.
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/IPO/Intel_ArgumentAlignment.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/MemoryBuiltins.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/Type.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/InstSimplifyPass.h"
#include "llvm/Transforms/Utils/Local.h"
#include <queue>

using namespace llvm;

#define DEBUG_TYPE "intel-argument-alignment"

// Specify the size of the alignment we are looking for
static cl::opt<unsigned> IntelArgAlignmentSize("intel-argument-alignment-size",
                                               cl::init(8), cl::ReallyHidden);

// Helper class to store the arguments that will be aligned
class AlignedArgument {

public:
  // Function to be trasformed
  Function *Func;

  // Argument being aligned
  Argument *Arg;

  // Instructions to be replaced with a constant
  SmallVector<Instruction *, 2> InstSet;

  AlignedArgument(Function *Func, Argument *Arg,
                  SetVector<Instruction *> &InputInstSet)
      : Func(Func), Arg(Arg) {
    std::copy(InputInstSet.begin(), InputInstSet.end(),
              std::inserter(InstSet, InstSet.begin()));
  }

  ~AlignedArgument();
};

class ArgumentAlignment {

public:
  ArgumentAlignment(Module &M, WholeProgramInfo *WPInfo,
                    std::function<const TargetLibraryInfo &(Function &)> GetTLI)
      : M(M), WPInfo(WPInfo), GetTLI(GetTLI) {}

  bool runImpl();

  ~ArgumentAlignment() {
    for (auto *Candidate : Candidates)
      delete Candidate;
  }

private:
  Module &M;
  WholeProgramInfo *WPInfo;
  SetVector<AlignedArgument *> Candidates;
  std::function<const TargetLibraryInfo &(Function &)> GetTLI;

  // Find the candidates to be aligned
  void collectCandidates();

  // Analyze the candidates, if the candidate doesn't pass the
  // analysis then remove it
  void analyzeCandidates(
      std::function<const TargetLibraryInfo &(Function &)> GetTLI);

  // Apply transformation
  void applyTransformation();

}; // ArgumentAlignment

// Return true if the input instruction is a pointer-to-integer
// and the destination is i64.
static bool isValidPtrToInt(Instruction *Inst) {

  if (!Inst)
    return false;

  PtrToIntInst *PtrToInt = dyn_cast<PtrToIntInst>(Inst);
  if (!PtrToInt)
    return false;

  Type *DestType = PtrToInt->getDestTy();

  // TODO: For now the optimization works for i64.
  if (!DestType->isIntegerTy(64))
    return false;

  return true;
}

// Destructor for AlignedArgumet class
AlignedArgument::~AlignedArgument() {
  Func = nullptr;
  Arg = nullptr;
  InstSet.clear();
}

// Return true if the input instruction is an AND
// operation with a constant, and the constant is not 0.
static bool isValidANDOperation(Instruction *Inst) {

  if (!Inst)
    return false;

  BinaryOperator *AndInst = dyn_cast<BinaryOperator>(Inst);

  if (!AndInst)
    return false;

  if (AndInst->getOpcode() != Instruction::BinaryOps::And)
    return false;

  // Collect for and i64 %x, CONSTANT
  Value *Var = AndInst->getOperand(0);
  Constant *Const = dyn_cast<Constant>(AndInst->getOperand(1));

  // Both operands in the AND aren't constants or
  // both operands are constants
  if ((!Const && !isa<Constant>(Var)) || (Const && isa<Constant>(Var)))
    return false;

  // Collect and CONSTANT, %x in case is the other way
  if (!Const) {
    // We prove before that Var is a constant
    Const = cast<Constant>(Var);
    Var = AndInst->getOperand(1);
  }

  // TODO: for now we care for 64 bits
  if (!Var->getType()->isIntegerTy(64))
    return false;

  if (Const->isZeroValue())
    return false;

  return true;
}

// Return true if the input instruction is a compare with 0
static bool isValidCompare(Instruction *Inst) {

  if (!Inst)
    return false;

  ICmpInst *Cmpr = dyn_cast<ICmpInst>(Inst);
  if (!Cmpr)
    return false;

  if (!Cmpr->isEquality())
    return false;

  Constant *Const = dyn_cast<Constant>(Cmpr->getOperand(1));
  if (!Const)
    return false;

  if (!Const->isZeroValue())
    return false;

  return true;
}

// Return true if traversing from the input value Val we land in
// the input argument Arg. The traversals between the input Val
// and Arg are composed only of GEPs and PHI Nodes. All the paths
// between Val and the arguments must end in Arg, else return false.
static bool valueRefersToArg(Value *Val, Value *Arg) {

  if (!Val || !Arg)
    return false;

  Value *CurrVal = Val;
  std::queue<Value *> ValueQueue;
  SetVector<Value *> UsedValues;

  UsedValues.insert(CurrVal);
  ValueQueue.push(CurrVal);
  bool ArgFound = false;

  while (!ValueQueue.empty()) {

    Value *NewVal = ValueQueue.front();
    ValueQueue.pop();

    // If we reach a different argument then return false.
    if (isa<Argument>(NewVal)) {
      if (NewVal == Arg) {
        ArgFound = true;
        continue;
      }
      return false;
    }

    if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(NewVal)) {

      if (GEP->getNumOperands() != 2)
        return false;

      // The GEP must be an multiple of 8
      Type *SourceTy = GEP->getSourceElementType();
      if (!SourceTy->isIntegerTy(8))
        return false;

      // We are using an i64 to access the next element
      Type *EntryTy = GEP->getOperand(1)->getType();
      if (!EntryTy->isIntegerTy(64))
        return false;

      Value *Operand = GEP->getOperand(0);
      if (UsedValues.insert(Operand))
        ValueQueue.push(Operand);
    }

    // All the values in a PHI Node must land at the input argument
    else if (PHINode *PhiInst = dyn_cast<PHINode>(NewVal)) {

      unsigned NumIncomingVals = PhiInst->getNumIncomingValues();
      for (unsigned Entry = 0; Entry < NumIncomingVals; Entry++) {

        Value *PhiVal = PhiInst->getIncomingValue(Entry);
        if (PhiVal != CurrVal && UsedValues.insert(PhiVal))
          ValueQueue.push(PhiVal);
      }
    } else {
      return false;
    }
  }

  return ArgFound;
}

// Return true if the pointer in the input instruction refers to the input
// argument. Else return false. This function anlayses the following:
//
// define internal fastcc void @foo(i8*, i64) {
//
//    ...
//
// if_bb:
//  %5 = phi i8* [ %0, %entry ], [ %8, %else_bb ]
//
// else_bb:
//   %7 = sub i64 0, %1
//   %8 = getelementptr inbounds i8, i8* %5, i64 %7
//   %9 = ptrtoint i8* %8 to i64
//   ...
// }
//
// Notice that %9 is a pointer-to-int instruction of %8, and %8 is a GEP from
// %5. The PHI Node in %5 says that the pointer will come from the argument %0.
// This means that %8 is just moving the pointer to collect the data, but will
// keep pointing to the same chunk of memory that was pointed by %0.
static bool pointerRefersToArg(PtrToIntInst *Inst, Value *Arg) {

  if (!Arg || !Inst)
    return false;

  Value *CurrVal = Inst->getOperand(0);

  return valueRefersToArg(CurrVal, Arg);
}

// Traverse through the input Value to check if there is a MOD operation,
// if so then return the PtrToInt instruction. The input Value is expected
// to be an ICmpInst. This function looks for the following:
//
//   %9 = ptrtoint i8* %8 to i64
//   %10 = and i64 %9, 7
//   %11 = icmp eq i64 %10, 0
//
// From the example before, Val is the icmp instruction (%11). This function
// will collect the operand and make sure it lands at the pointer to integer
// instruction (%9). It will return the pointer to integer instruction that
// was found.
static PtrToIntInst *getPointerInstruction(Value *Val) {
  PtrToIntInst *NullInst = nullptr;

  if (!Val)
    return NullInst;

  Instruction *Inst = dyn_cast<Instruction>(Val);
  if (!isValidCompare(Inst))
    return NullInst;

  Inst = dyn_cast<Instruction>(Inst->getOperand(0));
  if (!isValidANDOperation(Inst))
    return NullInst;

  Inst = dyn_cast<Instruction>(Inst->getOperand(0));
  if (!isValidPtrToInt(Inst))
    return NullInst;

  return cast<PtrToIntInst>(Inst);
}

// This function traverses through the Users of the input instruction. Then, if
// a user is a PHI Node, all the incoming values for the PHI Node must be
// the same alignment computation for the input Argument. For example:
//
// define internal fastcc void @foo(i8*, i64) {
// entry:
//   %2 = ptrtoint i8* %0 to i64
//   %3 = and i64 %2, 7
//   %4 = icmp eq i64 %3, 0
//   br i1 %4, label %if_bb, label %end
//
// if_bb:
//   %5 = phi i8* [ %0, %entry ], [ %8, %else_bb ]
//   %6 = phi i1  [ %4, %entry ], [ %11, %else_bb ]
//   br i1 %6, label %else_bb, label %end
//
// else_bb:
//   %7 = sub i64 0, %1
//   %8 = getelementptr inbounds i8, i8* %5, i64 %7
//   %9 = ptrtoint i8* %8 to i64
//   %10 = and i64 %9, 7
//   %11 = icmp eq i64 %10, 0
//   %12 = lshr i64 %7, 3
//   %13 = icmp ult i64 %12, 7
//   br i1 %13, label %if_bb, label %end
//
// end:
//   ret void
// }
//
// The input instruction for this case will be %11, the icmp. The User
// of %11 is the PHI Node %6, with incoming Values %4 and %11. This function
// will collect both incoming values and find the pointer-to-integer related
// instructions (%2 and %9). Then it will check that the pointers refer to
// the input argument (%0). If so, then return true, else return false.
static bool checkIfPHINodePointsToArgument(Instruction *Inst, Argument *Arg,
                                           SetVector<Instruction *> &InstSet) {

  if (!Inst)
    return false;

  SetVector<Value *> ComputedValues;

  for (User *User : Inst->users()) {

    PHINode *PhiInst = dyn_cast<PHINode>(User);

    if (!PhiInst)
      continue;

    unsigned NumIncomingVals = PhiInst->getNumIncomingValues();

    for (unsigned Entry = 0; Entry < NumIncomingVals; Entry++) {

      Value *Val = PhiInst->getIncomingValue(Entry);
      assert(Val && "Value is expected in PHI node");

      // Skip the Value that is the same as the input instruction
      if (Val == cast<Value>(Inst))
        continue;

      // Skip repeated values
      if (!ComputedValues.insert(Val))
        continue;

      // Collect the pointer-to-integer instruction
      PtrToIntInst *NewInst = getPointerInstruction(Val);
      if (!NewInst) {
        InstSet.clear();
        return false;
      }

      // Check that the pointer refers to Arg
      if (!pointerRefersToArg(NewInst, dyn_cast<Value>(Arg))) {
        InstSet.clear();
        return false;
      }
      InstSet.insert(NewInst);
    }
  }

  return true;
}

// Traverse the input instruction and check if MOD is being
// computed. Basically we are looking for the following:
//
//  define void foo(i8* %0) {
//     %2 = ptrtoint i8* %0 to i64
//     %3 = and i64 %2, 7
//     %4 = icmp eq i64 %3, 0
//
// The argument %0 is being transformed into an integer and that
// integer is being computed against constants. If so, then return
// the ICmpInst instruction.
static Instruction *checkIfModIsComputed(Instruction *Inst) {

  Instruction *NullInst = nullptr;

  if (!Inst)
    return NullInst;

  for (User *User : Inst->users()) {

    if (isValidPtrToInt(Inst) &&
        isValidANDOperation(dyn_cast<Instruction>(User)))
      if (Instruction *NewInst = checkIfModIsComputed(cast<Instruction>(User)))
        return NewInst;

    if (isValidANDOperation(Inst) &&
        isValidCompare(dyn_cast<Instruction>(User)))
      return cast<Instruction>(User);
  }

  return NullInst;
}

// Return true if the argument is used to compute data alignment. For example:
//
// define internal fastcc void @foo(i8*, i64) {
// entry:
//   %2 = ptrtoint i8* %0 to i64
//   %3 = and i64 %2, 7
//   %4 = icmp eq i64 %3, 0
//   br i1 %4, label %if_bb, label %end
//
// if_bb:
//   %5 = phi i8* [ %0, %entry ], [ %8, %else_bb ]
//   %6 = phi i1  [ %4, %entry ], [ %11, %else_bb ]
//   br i1 %6, label %else_bb, label %end
//
// else_bb:
//   %7 = sub i64 0, %1
//   %8 = getelementptr inbounds i8, i8* %5, i64 %7
//   %9 = ptrtoint i8* %8 to i64
//   %10 = and i64 %9, 7
//   %11 = icmp eq i64 %10, 0
//   %12 = lshr i64 %7, 3
//   %13 = icmp ult i64 %12, 7
//   br i1 %13, label %if_bb, label %end
//
// end:
//   ret void
// }
//
// The computation that is being done from %2 to %4 and %9 to %11 are
// operations on constants. If we know the alignment of %0 at compile time
// then these values will be constant across the whole program.
static bool checkArgument(Argument *Arg, Function *Func,
                          SetVector<Instruction *> &InstSet) {

  // If the argument is not a size multiple of 8, return false.
  auto GetSourceType = [](Type *PtrTy) {
    Type *CurrType = PtrTy;

    while (CurrType && CurrType->isPointerTy())
      CurrType = CurrType->getPointerElementType();

    return CurrType;
  };

  Type *ArgSourceType = GetSourceType(Arg->getType());

  if (!ArgSourceType->isIntegerTy(8))
    return false;

  for (User *User : Arg->users()) {
    Instruction *Inst = dyn_cast<Instruction>(User);
    if (!Inst || !isa<PtrToIntInst>(Inst))
      continue;

    // Collect the direct user of the input argument. Basically
    // look for this:
    //
    //   %3 = ptrtoint i8* %0 to i64
    //   %4 = and i64 %3, 7
    //   %5 = icmp eq i64 %4, 0
    if (Instruction *OutInst = checkIfModIsComputed(Inst)) {

      // Now look for other places were it is being done.
      // Basically look for this:

      // ...
      //   %5 = phi i8* [ %0, %entry ], [ %8, %else_bb ]
      // ...
      //
      //   %8 = getelementptr inbounds i8, i8* %5, i64 %7
      //   %9 = ptrtoint i8* %8 to i64
      //   %10 = and i64 %9, 7
      //   %11 = icmp eq i64 %10, 0
      if (checkIfPHINodePointsToArgument(OutInst, Arg, InstSet))
        InstSet.insert(Inst);
    }
  }

  return !InstSet.empty();
}

// Go through each argument of the input function and check if the alignment
// of the argument is being computed. If so, then create a new AlignedArgument
// and store the function, the argument found and the pointer-to-integer
// instruction that will be replaced.
static void checkFunction(Function *Func,
                          SetVector<AlignedArgument *> &Candidates) {
  if (!Func)
    return;

  for (auto &Arg : Func->args()) {

    if (!Arg.getType()->isPointerTy())
      continue;

    SetVector<Instruction *> InstSet;
    if (checkArgument(&Arg, Func, InstSet)) {
      AlignedArgument *Candidate = new AlignedArgument(Func, &Arg, InstSet);
      Candidates.insert(Candidate);
    }
  }
}

// Traverse through each function in the module and identify if it is a
// candidate for replacing the memory alignment computation.
void ArgumentAlignment::collectCandidates() {

  for (auto &Func : M) {
    // There is no IR
    if (Func.isIntrinsic() || Func.isDeclaration())
      continue;

    // All callsites must be direct calls
    if (Func.hasAddressTaken())
      continue;

    // There must be at least one argument
    if (Func.arg_empty())
      continue;

    checkFunction(&Func, Candidates);
  }
}

// Checks if the input Value is an alloc site. An alloc site can be a LibFunc
// or an alloca. This function returns:
//
//   -1: Is not an alloc site or the LibFunc is not supported
//    0: The size allocated is 0 or NaN
//    X: The size allocated is bigger than 0.
static int
isValidAllocSite(Value *AllocSite,
                 std::function<const TargetLibraryInfo &(Function &)> GetTLI) {

  if (!AllocSite)
    return -1;

  if (CallBase *CallSite = dyn_cast<CallBase>(AllocSite)) {

    Function *Callee = CallSite->getCalledFunction();
    if (!Callee)
      return -1;

    const TargetLibraryInfo TLI = GetTLI(*Callee);
    // TODO: Maybe in a future we might want to extend this for malloc
    if (isCallocLikeFn(AllocSite, &TLI)) {
      assert((CallSite->arg_size() == 2) && "Calloc uses exactly 2 arguments");

      ConstantInt *SecondArg =
          dyn_cast<ConstantInt>(CallSite->getArgOperand(1));

      // Second argument is not a constant
      if (!SecondArg)
        return 0;

      // Second argument is 0, null or NaN
      if (SecondArg->isZeroValue() || SecondArg->isNaN())
        return 0;

      unsigned Val = SecondArg->getZExtValue();
      return (int)Val;
    }

    return -1;
  }

  else if (AllocaInst *Alloca = dyn_cast<AllocaInst>(AllocSite)) {
    unsigned Alignment = Alloca->getAlignment();

    return (int)Alignment;
  }

  return -1;
}

// Return true if the input call site is a recursive call to the
// input function, and if the actual argument is the same as the
// argument from the caller. This function looks for the following:
//
//   define void @foo(i8 *%0) {
//
//     ...
//     %100 = phi i8* [%200, %150], [%0, %1]
//     ...
//
//     %200 = getelementptr i8, i8* %100, i64 %199
//     tail call void @foo(i8* %200)
//
// In the example above, the actual argument (%200) is to the same block
// of memory allocated for the formal argument (%0). In this case, return
// true.
static bool checkRecursiveCall(CallBase *CallSite, Function *Func,
                               Argument *Arg) {

  if (!CallSite || !Func || !Arg)
    return false;

  Function *Caller = CallSite->getCaller();

  if (Caller != Func)
    return false;

  SetVector<Instruction *> UsedInst;
  int ArgNo = Arg->getArgNo();
  Value *CurrVal = CallSite->getArgOperand(ArgNo);

  return valueRefersToArg(CurrVal, Arg);
}

// Return true if there is a Store instruction that could modify the input
// Value. This Value starts as an argument and will reach to the call site of
// the input function Func. For example:
//
// define void @bar(%_struct.test** %0) {
//   ...
//   %158 = getelementptr inbounds %_struct.test*, %_struct.test** %0, i64 1
//   %159 = bitcast %_struct.test** %158 to i8*
//   tail call fastcc void @foo(i8* nonnull %159)
//   ...
// }
//
// Consider that Val is %0. The use of %0 is %158, then %159 and lands in the
// call site of @foo. Since nothing modified %0 along the way, then it is safe
// to assume that the memory that it is pointing to never changes.
static bool checkIfPtrIsModified(Value *Val, Function *Func) {

  if (!Val || !Func)
    return true;

  for (User *User : Val->users()) {

    if (isa<StoreInst>(User))
      return true;

    // Only collect the operand 0 from GEPs
    if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(User)) {
      // If the GEP is not inbounds, then it is not safe to assume that
      // the operation done in the GEP will be inside the memory space
      // allocated.
      if (!GEP->isInBounds())
        return true;

      if (GEP->getOperand(0) != Val ||
          !GEP->getSourceElementType()->isPointerTy())
        continue;
    }

    else if (isa<LoadInst>(User)) {
      // If the load comes from a GEP, then it means that we are dereferencing
      // a pointer. Therefore, any further use will be to the dereferenced
      // pointer and not to Val itself.
      if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(Val)) {

        // If the GEP is not inbounds, then it is not safe to assume that
        // the dereferenced pointer is inside the allocated memory space.
        if (!GEP->isInBounds())
          return true;

        continue;
      }
    }

    // If the BitCast is not a pointer, then there is nothing else to check
    else if (BitCastInst *BitCast = dyn_cast<BitCastInst>(User)) {
      if (!BitCast->getDestTy()->isPointerTy())
        continue;
    }

    // The call site must land in the input function Func.
    else if (CallBase *CallSite = dyn_cast<CallBase>(User)) {
      Function *CalledFunc = CallSite->getCalledFunction();
      if (!CalledFunc || CalledFunc != Func)
        return true;
      continue;
    }

    // Everything else can do something to the pointer that we don't want.
    // Just return here.
    else {
      return true;
    }

    if (checkIfPtrIsModified(cast<Value>(User), Func))
      return true;
  }

  return false;
}

// Return true if tracing from the input Value leads to an alloc site with an
// alignment that is multiple of IntelArgAlignmentSize. This function looks
// for the following:
//
// %44 = tail call noalias i8* @calloc(i64 %41, i64 8)
// tail call fastcc void @foo(i8* %44)
//
// The actual argument in the example above (%44) is a calloc with size
// %41 * 8, return true in this case.
//
// This function also looks for alloca instructions. For example:
//
// define void @baz() {
//  ...
//  %5 = alloca [4061 x %_struct.test*], align 16
//  ...
//  %22 = getelementptr inbounds [4061 x %_struct.test*],
//        [4061 x %_struct.test*]* %5, i64 0, i64 0
//  call @bar(%_struct.test** nonnull %22)
//  ...
// }
//
// define void @bar(%_struct.test** %0) {
//   ...
//   %158 = getelementptr inbounds %_struct.test*, %_struct.test** %0, i64 1
//   %159 = bitcast %_struct.test** %158 to i8*
//   tail call fastcc void @foo(i8* nonnull %159)
//   ...
// }
//
// @foo is the function we want to optimize in the example above. If we trace
// the actual argument in the callsite of @foo (%159), we can see its
// definition in @baz (%5) is an alloca instruction with an alignment of 16.
// Return true in this case.
static bool
checkAllocSite(CallBase *CallSite, Function *CandidateFunc, Value *Val,
               std::function<const TargetLibraryInfo &(Function &)> GetTLI) {

  if (!CallSite || !Val)
    return false;

  // Return the element type of a pointer or an element.
  auto GetSourceType = [](Type *PtrTy) {
    Type *CurrType = PtrTy;

    while (CurrType && (CurrType->isPointerTy() || CurrType->isArrayTy())) {

      if (ArrayType *ArrTy = dyn_cast<ArrayType>(CurrType))
        CurrType = ArrTy->getElementType();

      else
        CurrType = (cast<PointerType>(CurrType))->getPointerElementType();
    }
    return CurrType;
  };

  Type *CurrType = GetSourceType(Val->getType());
  Value *CurrVal = Val;

  while (CurrVal) {

    // Return true if the Value is an alloc site
    int AllocSize = isValidAllocSite(CurrVal, GetTLI);
    if (AllocSize >= 0) {
      if (AllocSize == 0)
        return false;

      return (AllocSize % IntelArgAlignmentSize == 0);
    }

    // Check if the value is a BitCast
    if (BitCastInst *Cast = dyn_cast<BitCastInst>(CurrVal)) {

      // The BitCast must be converting to the type we need
      if (GetSourceType(Cast->getDestTy()) != CurrType)
        return false;

      // We already proved that the main argument we care about is a multiple
      // of 8, so now just collect the bitcast and trace it.
      CurrType = GetSourceType(Cast->getSrcTy());
      CurrVal = Cast->getOperand(0);
      continue;
    }

    // Check if the Value is a GEP
    if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(CurrVal)) {
      Type *SourceType = GetSourceType(GEP->getSourceElementType());

      // We already proved that the main argument we care about is a multiple
      // of 8, therefore the GEP will either point to an i8* or a type that
      // will later be converted as an i8* by a BitCast.
      if (SourceType != CurrType)
        return false;

      ConstantInt *GEPEntry = nullptr;

      // Make sure that the entry we are accessing in the GEP is constant
      if (GEP->getNumOperands() < 3)
        GEPEntry = dyn_cast<ConstantInt>(GEP->getOperand(1));
      else
        GEPEntry = dyn_cast<ConstantInt>(GEP->getOperand(2));

      if (!GEPEntry)
        return false;

      CurrVal = GEP->getOperand(0);
      continue;
    }

    // Check if the Value is an Argument, is so then collect all
    // the call sites
    if (Argument *Arg = dyn_cast<Argument>(CurrVal)) {
      if (checkIfPtrIsModified(CurrVal, CandidateFunc))
        return false;

      Function *Func = Arg->getParent();
      if (!Func || Func->hasAddressTaken())
        return false;

      bool PassAllCallSites = false;
      unsigned ArgNo = Arg->getArgNo();
      for (User *User : Func->users()) {

        CallBase *NewCallSite = dyn_cast<CallBase>(User);

        if (!NewCallSite)
          return false;

        Function *Caller = NewCallSite->getCaller();
        // If we reach a recursive call or a chain of recursive calls then
        // there is nothing else to check.
        if (Caller == CandidateFunc || Caller == Func)
          continue;

        Value *ActualArg = NewCallSite->getArgOperand(ArgNo);

        // All callsites should lead to an alloc site
        PassAllCallSites =
            checkAllocSite(NewCallSite, CandidateFunc, ActualArg, GetTLI);

        if (!PassAllCallSites)
          return false;
      }

      return PassAllCallSites;
    }

    CurrVal = nullptr;
  }

  return false;
}

// Check all the callsites of each input candidate to make sure that they are
// alloc sites or recursive calls. If not, then remove the candidate.
void ArgumentAlignment::analyzeCandidates(
    std::function<const TargetLibraryInfo &(Function &)> GetTLI) {

  auto CandBegin = Candidates.begin();
  auto CandEnd = Candidates.end();
  auto Candidate = CandBegin;

  while (Candidate != CandEnd) {

    Function *Func = (*Candidate)->Func;
    unsigned ArgNo = (*Candidate)->Arg->getArgNo();
    bool ValidCandidate = true;

    for (User *User : Func->users()) {

      CallBase *CallSite = dyn_cast<CallBase>(User);
      SetVector<PHINode *> UsedPhis;

      if (!CallSite) {
        ValidCandidate = false;
        break;
      }

      // Check if the callsite is recursive call, or check if the
      // argument is from an alloc site
      Value *ActualArg = CallSite->getArgOperand(ArgNo);
      if (checkRecursiveCall(CallSite, Func, (*Candidate)->Arg) ||
          checkAllocSite(CallSite, Func, ActualArg, GetTLI))
        continue;

      ValidCandidate = false;
      break;
    }

    if (!ValidCandidate) {
      Candidate = Candidates.erase(Candidate);
      CandEnd = Candidates.end();
    } else {
      ++Candidate;
    }
  }
}

// Return true if the User of the input Use is an AND operantion that can be
// transformed.
static bool replaceCompare(Use &U) {
  User *UserU = U.getUser();

  if (!UserU)
    return false;

  Instruction *Inst = dyn_cast<Instruction>(UserU);

  if (!Inst)
    return false;

  return isValidANDOperation(Inst);
}

// Replace the users of the instructions that compute a MOD.
// This will modify the IR as follows:
//
// define internal fastcc void @foo(i8* %0, i64 %1) #0 {
// entry:
//   %2 = ptrtoint i8* %0 to i64
//   %3 = and i64 8, 7
//   %4 = icmp eq i64 %3, 0
//   br i1 %4, label %if_bb, label %end
//
// if_bb:                                   ; preds = %else_bb, %entry
//   %5 = phi i8* [ %0, %entry ], [ %8, %else_bb ]
//   %6 = phi i1 [ %4, %entry ], [ %11, %else_bb ]
//   br i1 %6, label %else_bb, label %end
//
// else_bb:                                 ; preds = %if_bb
//   %7 = sub i64 0, %1
//   %8 = getelementptr inbounds i8, i8* %5, i64 %7
//   %9 = ptrtoint i8* %8 to i64
//   %10 = and i64 8, 7
//   %11 = icmp eq i64 %10, 0
//   %12 = lshr i64 %7, 3
//   %13 = icmp ult i64 %12, 7
//   tail call fastcc void @foo(i8* %8, i64 %7)
//   br i1 %13, label %if_bb, label %end
//
// end:                                     ; preds = %else_bb, %if_bb, %entry
//   ret void
// }
//
// Notice that %3 and %10 are now constants.
void ArgumentAlignment::applyTransformation() {

  IRBuilder<> Builder(M.getContext());
  for (auto Candidate : Candidates) {

    // First replace the instructions with constants
    for (auto Inst : Candidate->InstSet)
      Inst->replaceUsesWithIf(Builder.getInt64(IntelArgAlignmentSize),
                              replaceCompare);

    // Then remove the dead code
    llvm::legacy::FunctionPassManager FuncPass(&M);
    FuncPass.add(createInstSimplifyLegacyPass());
    FuncPass.add(createCFGSimplificationPass());

    FuncPass.doInitialization();
    FuncPass.run(*(Candidate->Func));
    FuncPass.doFinalization();
  }
}

bool ArgumentAlignment::runImpl() {

  // Check for AVX2 or higher
  auto TTIOptLevel = TargetTransformInfo::AdvancedOptLevel::AO_TargetHasAVX2;
  if (!WPInfo || !WPInfo->isAdvancedOptEnabled(TTIOptLevel)) {
    LLVM_DEBUG({
      dbgs() << "Candidates for argument alignment: 0\n";
      dbgs() << "Reason: NOT AVX2\n";
    });
    return false;
  }

  // Check if whole program is safe
  if (!WPInfo->isWholeProgramSafe()) {
    LLVM_DEBUG({
      dbgs() << "Candidates for argument alignment: 0\n";
      dbgs() << "Reason: Whole program not safe\n";
    });
    return false;
  }

  collectCandidates();

  if (Candidates.empty()) {
    LLVM_DEBUG({
      dbgs() << "Candidates for argument alignment: 0\n";
      dbgs() << "Reason: Couldn't find at least one candidate\n";
    });
    return false;
  }

  analyzeCandidates(GetTLI);

  if (Candidates.empty()) {
    LLVM_DEBUG({
      dbgs() << "Candidates for argument alignment: 0\n";
      dbgs() << "Reason: Candidates didn't pass analysis\n";
    });
    return false;
  }

  LLVM_DEBUG({
    dbgs() << "Candidates for argument alignment: " << Candidates.size()
           << "\n";
    dbgs() << "Aligning by: " << IntelArgAlignmentSize << "\n";
    for (auto Candidate : Candidates) {
      dbgs() << "  Function: " << Candidate->Func->getName() << "\n";
      dbgs() << "    Instructions: "
             << "\n";
      for (auto Inst : Candidate->InstSet) {
        PtrToIntInst *Ptr = cast<PtrToIntInst>(Inst);
        dbgs() << "      " << *Inst << " " << Ptr->getPointerAddressSpace()
               << "\n";
      }
      dbgs() << "    Argument: " << *(Candidate->Arg) << "\n";
    }
  });

  applyTransformation();

  return true;
}

namespace {

struct IntelArgumentAlignmentLegacyPass : public ModulePass {
public:
  static char ID; // Pass identification, replacement for typeid
  IntelArgumentAlignmentLegacyPass() : ModulePass(ID) {
    initializeIntelArgumentAlignmentLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addPreserved<WholeProgramWrapperPass>();
    AU.addPreserved<TargetLibraryInfoWrapperPass>();
    AU.addRequired<WholeProgramWrapperPass>();
    AU.addRequired<TargetLibraryInfoWrapperPass>();
  }

  bool runOnModule(Module &M) override {

    if (skipModule(M))
      return false;

    WholeProgramInfo *WPInfo =
        &getAnalysis<WholeProgramWrapperPass>().getResult();

    auto GetTLI = [this](Function &F) -> const TargetLibraryInfo & {
      return this->getAnalysis<const TargetLibraryInfoWrapperPass>().getTLI(F);
    };

    // Implementation of the optimization
    return ArgumentAlignment(M, WPInfo, GetTLI).runImpl();
  }
};

} // namespace

char IntelArgumentAlignmentLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(IntelArgumentAlignmentLegacyPass,
                      "intel-argument-alignment", "Intel argument alignment",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_END(IntelArgumentAlignmentLegacyPass,
                    "intel-argument-alignment", "Intel argument alignment",
                    false, false)

ModulePass *llvm::createIntelArgumentAlignmentLegacyPass() {
  return new IntelArgumentAlignmentLegacyPass();
}

IntelArgumentAlignmentPass::IntelArgumentAlignmentPass() {}

PreservedAnalyses IntelArgumentAlignmentPass::run(Module &M,
                                                  ModuleAnalysisManager &AM) {

  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetTLI = [&FAM](Function &F) -> const TargetLibraryInfo & {
    return FAM.getResult<const TargetLibraryAnalysis>(F);
  };

  if (!ArgumentAlignment(M, &WPInfo, GetTLI).runImpl())
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  PA.preserve<TargetLibraryAnalysis>();

  return PA;
}
