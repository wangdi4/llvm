//=-- Intel_IPODeadArgElimination.cpp - Simplified dead arg elimination -*-=//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass implements a simplified dead argument elimination with IPO
// analysis. The goal is to eliminate those arguments that initialize data but
// the actual value is not used across multiple functions. For example:
//
// define internal float @foo(float *%0, float *%1, i64 %2, i64 %3) {
//    %5 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(
//             i8 1, i64 %2, i64 %3, float* nonnull %0, i64 %2)
//    %6 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(
//             i8 0, i64 %2, i64 4, float* nonnull %5, i64 %2)
//    %7 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(
//             i8 2, i64 %2, i64 %3, float* nonnull %6, i64 %2)
//    %8 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(
//             i8 1, i64 %2, i64 %3, float* nonnull %7, i64 %2)
//    %9 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(
//             i8 0, i64 %2, i64 4, float* nonnull %8, i64 %2)
//    %10 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(
//             i8 2, i64 %2, i64 %3, float* nonnull %9, i64 %2)
//    store float 0.000000e+00, float* %10
//    %11 = load float, float* %1
//    ret float %11
// }
//
// define internal float @bar(float *%0, float *%1, i64 %2, i64 %3) {
//    %5 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(
//             i8 1, i64 %2, i64 %3, float* nonnull %0, i64 %2)
//    %6 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(
//             i8 0, i64 %2, i64 4, float* nonnull %5, i64 %2)
//    %7 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(
//             i8 2, i64 %2, i64 %3, float* nonnull %6, i64 %2)
//    %8 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(
//             i8 1, i64 %2, i64 %3, float* nonnull %7, i64 %2)
//    %9 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(
//             i8 0, i64 %2, i64 4, float* nonnull %8, i64 %2)
//    %10 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(
//             i8 2, i64 %2, i64 %3, float* nonnull %9, i64 %2)
//    store float 0.000000e+00, float* %10
//    %11 = load float, float* %1
//    ret float %11
// }
//
// define internal float @bas(float *%0, float *%1, i64 %2, i64 %3) {
//   %5 = alloca float, i64 %3
//   %6 = call float @foo(float *%5, float *%0, i64 %2, i64 %3)
//   %7 = call float @bar(float *%5, float *%1, i64 %2, i64 %3)
//   %8 = fadd float %6, %7
//   ret float %8
// }
//
// In the example above, the pointer %5 is allocated in @bas, initialized in
// @foo and then rewritten in @bar. Then, it isn't used anymore. Technically,
// the array becomes dead. This pass should be able to detect that the pointer
// will be dead and remove the subscript, store and allocate instructions. The
// IR should look as follows after the transformation:
//
// define internal float @foo(float *%0, i64 %1, i64 %2) {
//    %4= load float, float* %0
//    ret float %4
// }
//
// define internal float @bar(float *%0, i64 %1, i64 %2) {
//    %4 = load float, float* %0
//    ret float %4
// }
//
// define internal float @bas(float *%0, float *%1, i64 %2, i64 %3) {
//   %5 = call float @foo(float *%0, i64 %2, i64 %3)
//   %6 = call float @bar(float *%1, i64 %2, i64 %3)
//   %7 = fadd float %5, %6
//   ret float %7
// }
//
// The difference between this pass and the traditional argument promotion is
// that this is a more simplified pass. The traditional pass is very aggressive
// and can remove the AllocSize attribute from the arguments, which is
// important for other passes (e.g. loop-opt and vectorizer). Also, it can
// modify the return value for a function. We intend to run this pass very late
// in the LTO process in order to clean the IR before loop-opt.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/IPO/Intel_IPODeadArgElimination.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/Intel_Andersens.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/MemorySSA.h"
#include "llvm/InitializePasses.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/Intel_InlineReport.h"
#include "llvm/Transforms/IPO/Intel_MDInlineReport.h"

#include <stack>
#include <vector>

using namespace llvm;

#define DEBUG_TYPE "intel-ipo-dead-arg-elimination"

STATISTIC(NumOfFunctionsTransformed,
          "Number of functions that dead arg elimination was applied");
STATISTIC(NumOfActualParamsRemoved,
          "Number of actual parameters removed");

// Enable deleting the calls to functions that initialize dead arrays
static cl::opt<bool>
    EnableDeadArgEliminationStore("enable-ipo-dead-arg-elimination",
                                  cl::init(true), cl::ReallyHidden);

// Helper class that implements dead argument elimination.
class IPDeadArgElimination {
public:
  IPDeadArgElimination(Module &M) : M(M) {}

  bool runImpl(WholeProgramInfo &WPInfo);

private:
  Module &M;
  // Map from a function to the set of its arguments that could be deleted.
  // The entries for this map are added during data collection.
  DenseMap<Function *, SetVector<Argument *>> DeadArgsCandidatesMap;

  // Map from a function to the set of its arguments that will be deleted.
  // The entries for this map are added during the analysis process.
  DenseMap<Function *, SetVector<Argument *>> DeadArraysArgResult;

  // Collect possible candidates
  void collectData(Function &F);
  void analyzeArguments();
  bool analyzeOneArgument(Argument *Arg, SetVector<Argument *> &DeadArgs);
  bool removeDeadArgs(Function *F, SetVector<Argument *> &DeadArgs,
                      SetVector<Value *> &DeadActualVect);
  bool applyTransformation();
};

// This function will collect all the possible arguments that can be deleted.
// For now, it searches where an argument is used in a chain of subscript
// instructions to access an array entry where data will be written.
// For example:
//
// define internal float @foo(float *%0, float *%1, i64 %2, i64 %3) {
//    %5 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(
//             i8 1, i64 %2, i64 %3, float* nonnull %0, i64 %2)
//    %6 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(
//             i8 0, i64 %2, i64 4, float* nonnull %5, i64 %2)
//    %7 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(
//             i8 2, i64 %2, i64 %3, float* nonnull %6, i64 %2)
//    %8 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(
//             i8 1, i64 %2, i64 %3, float* nonnull %7, i64 %2)
//    %9 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(
//             i8 0, i64 %2, i64 4, float* nonnull %8, i64 %2)
//    %10 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(
//             i8 2, i64 %2, i64 %3, float* nonnull %9, i64 %2)
//    store float 0.000000e+00, float* %10
//    %11 = load float, float* %1
//    ret float %11
// }
//
// The argument %0 in function @foo is used in a chain of subscript
// instructions to (from %5 to %10) in order to write some information. This
// means that function @foo is a candidiate where %0 can be removed.
void IPDeadArgElimination::collectData(Function &F) {

  // This lambda function will return true if the input argument is a candidate
  // for dead argument elimination. Else, return false.
  auto IsDeadArg = [](Argument &Arg) -> bool {
    Value *CurrVal = &Arg;
    Value *PrevVal = nullptr;
    bool AccessToArray = false;

    // Traverse through the chain of users of the argument. Each instruction
    // must have an unique user.
    while (CurrVal && CurrVal->hasOneUser()) {
      Value *CurrUser = CurrVal->user_back();

      if (isa<StoreInst>(CurrUser)) {
        // If the user is an Store instruction then we are done. Store doesn't
        // have users.
        PrevVal = CurrVal;
        CurrVal = CurrUser;
        break;
      } else if (auto *SI = dyn_cast<SubscriptInst>(CurrUser)) {
        // If the user is a Subscript instruction then the pointer operand
        // must be the current value. Else, the current value for something
        // else.
        if (SI->getPointerOperand() != CurrVal)
          return false;
        PrevVal = CurrVal;
        CurrVal = cast<Value>(SI);
        AccessToArray = true;
      } else {
        // Another use we don't know yet. We can expand this in the future to
        // analyze dead Load and GEP instructions.
        CurrVal = nullptr;
        return false;
      }
    }

    // The store instruction must be used to store a constant into the pointer
    // operand. In the example above, the store instruction is storing 0 into
    // %9, which comes from a series of Subscript instructions pointing to the
    // argument %0.
    //
    // NOTE: This is conservative, in theory we can get rid of anything stored
    // since the information won't be used.
    auto *StoreI = dyn_cast<StoreInst>(CurrVal);
    if (AccessToArray && StoreI && StoreI->getPointerOperand() == PrevVal &&
        (isa<ConstantInt>(StoreI->getValueOperand()) ||
         isa<ConstantFP>(StoreI->getValueOperand())))
      return true;

    return false;
  };

  // All the callers of the function must be the same.
  Function *Caller = nullptr;
  for (auto *U : F.users()) {
    CallBase *Call = dyn_cast<CallBase>(U);
    if (!Call)
      return;
    if (!Caller)
      Caller = Call->getCaller();
    else if (Caller != Call->getCaller())
      return;
  }

  if (!Caller)
    return;

  // If the argument has the AllocSize attribute then we stop the
  // transformation for the current function. This is an important attribute
  // for other transformations. We may relax this in the future by updating
  // this attribute with a new list of parameters.
  if (F.hasFnAttribute(Attribute::AllocSize))
    return;

  SetVector<Argument *> DeadArgs;
  for (Argument &Arg : F.args()) {
    // The argument must be used only in a chain of instructions, therefore
    // it must have only one user.
    if (!Arg.hasOneUser())
      continue;

    // Check if the argument is dead.
    if (IsDeadArg(Arg))
      DeadArgs.insert(&Arg);
  }

  // Insert in the candidates map.
  if (!DeadArgs.empty())
    DeadArgsCandidatesMap.insert({&F, DeadArgs});
}

// Return true if the input argument can be removed. For example:
//
// define internal float @foo(float *%0, float *%1, i64 %2, i64 %3) {
//    %5 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(
//             i8 1, i64 %2, i64 %3, float* nonnull %0, i64 %2)
//    %6 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(
//             i8 0, i64 %2, i64 4, float* nonnull %5, i64 %2)
//    %7 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(
//             i8 2, i64 %2, i64 %3, float* nonnull %6, i64 %2)
//    %8 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(
//             i8 1, i64 %2, i64 %3, float* nonnull %7, i64 %2)
//    %9 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(
//             i8 0, i64 %2, i64 4, float* nonnull %8, i64 %2)
//    %10 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(
//             i8 2, i64 %2, i64 %3, float* nonnull %9, i64 %2)
//    store float 0.000000e+00, float* %10
//    %11 = load float, float* %1
//    ret float %11
// }
//
// define internal float @bar(float *%0, float *%1, i64 %2, i64 %3) {
//    %5 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(
//             i8 1, i64 %2, i64 %3, float* nonnull %0, i64 %2)
//    %6 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(
//             i8 0, i64 %2, i64 4, float* nonnull %5, i64 %2)
//    %7 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(
//             i8 2, i64 %2, i64 %3, float* nonnull %6, i64 %2)
//    %8 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(
//             i8 1, i64 %2, i64 %3, float* nonnull %7, i64 %2)
//    %9 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(
//             i8 0, i64 %2, i64 4, float* nonnull %8, i64 %2)
//    %10 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(
//             i8 2, i64 %2, i64 %3, float* nonnull %9, i64 %2)
//    store float 0.000000e+00, float* %10
//    %11 = load float, float* %1
//    ret float %11
// }
//
// define internal float @bas(float *%0, float *%1, i64 %2, i64 %3) {
//   %5 = alloca float, i64 %3
//   %6 = call float @foo(float *%5, float *%0, i64 %2, i64 %3)
//   %7 = call float @bar(float *%5, float *%1, i64 %2, i64 %3)
//   %8 = fadd float %5, %6
//   ret float %8
// }
//
// The argument %0 in @foo can be removed because the only caller is @bas. Also
// the actual parameter is %5 in @bas and it is used as an actual for %0 in
// @bar, which is a candidate too. In this case, %0 from @foo and %0 from @bar
// will be added in DeadArraysArgsSet (this is to prevent redoing the same
// analysis when checking @bar after @foo).
bool IPDeadArgElimination::analyzeOneArgument(
    Argument *Arg, SetVector<Argument *> &DeadArraysArgsSet) {

  // This function will return the actual parameter at ArgNum in the call sites
  // for the input function. From the example, it should return %5 from the
  // call to @foo in @bas when ArgNum is 0.
  auto GetActualArg = [](Function *F, unsigned ArgNum) -> Value * {
    // The actual argument should be the same across all the call sites
    Function *Caller = nullptr;
    Value *ActualArg = nullptr;
    for (auto *U : F->users()) {
      auto *Call = dyn_cast<CallBase>(U);
      if (!Call)
        return nullptr;

      // There should be an unique caller, but there could be multiple
      // calls.
      if (!Caller)
        Caller = Call->getCaller();
      else if (Caller != Call->getCaller())
        return nullptr;

      // The actual should be the same across all the call sites.
      Value *TempActualArg = Call->getArgOperand(ArgNum);
      if (!ActualArg)
        ActualArg = TempActualArg;
      else if (ActualArg != TempActualArg)
        return nullptr;
    }

    if (!Caller || !ActualArg)
      return nullptr;

    return ActualArg;
  };

  assert(Arg && "Trying to analyze a possible dead argument that is null");

  // For now we are just considering arguments that are local variables
  // allocated in the caller function, but we can expand this analysis
  // to be recursive and collect multiple levels.
  Function *F = Arg->getParent();
  unsigned ArgNum = Arg->getArgNo();

  // Get the actual for the current formal. There must be only one actual
  // argument.
  Value *ActualArg = GetActualArg(F, ArgNum);
  if (!ActualArg || !isa<AllocaInst>(ActualArg))
    return false;

  SetVector<Argument *> TempArgsComputed;
  TempArgsComputed.insert(Arg);

  // All the users of the actual arg must be calls to function F or a call
  // in the candidates map. There is a chance that an actual parameter is used
  // in multiple functions. That is OK if the formal parameter is in the
  // candidates map and the only actual parameter for that formal is the
  // current value (ActualArg). In simple words, an actual parameter can be
  // used in multiple formal parameters as long they are candidates, but a
  // formal parameter must have one actual.
  for (auto *U : ActualArg->users()) {
    auto *Call = dyn_cast<CallBase>(U);
    if (!Call)
      return false;

    if (Call->isIndirectCall())
      return false;

    Function *CalledFunc = Call->getCalledFunction();
    if (!CalledFunc)
      return false;

    // Check if the function is a candidate, else fail
    if (DeadArgsCandidatesMap.count(CalledFunc) == 0)
      return false;

    // Find the formal argument in the new candidate. It must be unique.
    unsigned TempArgNo = 0;
    bool ArgFound = false;
    for (unsigned I = 0, E = Call->arg_size(); I < E; I++) {
      if (Call->getArgOperand(I) == ActualArg) {
        if (ArgFound)
          return false;
        ArgFound = true;
        TempArgNo = I;
      }
    }

    if (!ArgFound)
      return false;

    Argument *TempFormalArg = CalledFunc->getArg(TempArgNo);

    // If we computed this argument already, then skip it. We already proved
    // that the argument is unique.
    if (TempArgsComputed.count(TempFormalArg) != 0)
      continue;

    // The formal must be in the list of candidates, even for other functions.
    if (DeadArgsCandidatesMap[CalledFunc].count(TempFormalArg) == 0)
      return false;

    // The actual parameter for the called function must be the same as the
    // current actual.
    //
    // NOTE: We can relax this by removing the called function and the argument
    // from the candidates list. In that case we need to make sure that nothing
    // breaks in the generated code (e.g. guard the code that writes the data
    // if the actual parameter is not there).
    Value *TempActualArg = GetActualArg(CalledFunc, TempArgNo);
    if (TempActualArg != ActualArg)
      return false;

    // Insert the temporary formal into the list of arguments that will be
    // removed. This is to prevent recomputing it since we already proved
    // that the only actual argument for that parameter is the current actual.
    TempArgsComputed.insert(TempFormalArg);
  }

  // Insert all the arguments collected
  for (auto *CurrArg : TempArgsComputed)
    DeadArraysArgsSet.insert(CurrArg);

  // Arg will always be in TempArgsComputed, therefore it will
  // be inserted in DeadArraysArgsSet
  return true;
}

// Helper function to analyze all the candidate arguments.
void IPDeadArgElimination::analyzeArguments() {
  // Arguments that have passed analysis can be deleted. There is a chance
  // that the actual argument for one formal argument is used in multiple
  // formal arguments that we can remove. In this case store the precomputed
  // results to prevent computing the same.
  SetVector<Argument *> DeadArraysArgsSet;

  for (auto &Pair : DeadArgsCandidatesMap) {
    for (auto *Arg : Pair.second) {
      // If the argument is in DeadArraysArgsSet then it means that it was
      // computed already
      if (DeadArraysArgsSet.count(Arg) != 0 ||
          analyzeOneArgument(Arg, DeadArraysArgsSet))
        DeadArraysArgResult[Pair.first].insert(Arg);
    }
  }
}

// Return true if the arguments in DeadArgs were removed from the input
// function F. The vector DeadActualVect stores the actual parameters
// used for the dead arguments. These actual parameters will be removed
// later.
bool IPDeadArgElimination::removeDeadArgs(Function *F,
                                          SetVector<Argument *> &DeadArgs,
                                          SetVector<Value *> &DeadActualVect) {

  // Lambda function that removes all the instructions related to the input
  // argument. We don't need to check for the instructions' types, that was
  // done already in collectData.
  auto RemoveArgUsers = [](Argument *Arg) -> void {
    std::stack<Instruction *> InstStack;
    Value *CurrVal = Arg;
    // Collect all the instructions in a stack
    while (CurrVal && CurrVal->hasOneUser()) {
      Instruction *Inst = dyn_cast<Instruction>(CurrVal->user_back());
      assert(Inst && "Trying to remove an non-instruction user");
      InstStack.push(Inst);
      CurrVal = Inst;
    }

    assert(!InstStack.empty() && "List of instructions to remove is empty");

    // Remove the instructions from bottom to top
    while (!InstStack.empty()) {
      auto *Inst = InstStack.top();
      InstStack.pop();
      Inst->eraseFromParent();
    }
  };

  if (!F || DeadArgs.empty())
    return false;

  // First remove the instructions related to dead args. This simplifies
  // removing the dead arguments.
  for (auto *Arg : DeadArgs)
    RemoveArgUsers(Arg);

  // The rest of the code is related to creating a new function where the
  // arguments will be removed. Removing arguments is not updating the
  // list of arguments, we need to make a new function with a new list
  // of arguments and then copy the rest of the information from the
  // old function to the new function (attributes, metadata, etc.). Also,
  // we need to update the call sites with the new list of parameters.
  FunctionType *FTy = F->getFunctionType();
  Type *RetType = FTy->getReturnType();
  getInlineReport()->initFunctionClosure(F);

  // Vectors to store the the information for the live arguments
  std::vector<Type *> LiveArgsType;
  SmallVector<AttributeSet, 8> LiveArgsAttrVec;

  // Collect the arguments attribute list, return and function attributes
  const AttributeList &ArgsAttrList = F->getAttributes();
  AttrBuilder RAttrs(F->getContext(), ArgsAttrList.getRetAttrs());
  AttributeSet RetAttrs = AttributeSet::get(F->getContext(), RAttrs);
  AttributeSet FnAttrs = ArgsAttrList.getFnAttrs();

  // Collect the type and the attributes for the parameters that are live
  unsigned DeadArgI = 0;
  unsigned CurrDeadArgNo = DeadArgs[0]->getArgNo();
  unsigned DeadArgsSize = DeadArgs.size();
  for (unsigned ArgNo = 0, ArgsNum = F->arg_size(); ArgNo < ArgsNum; ArgNo++) {
    if (CurrDeadArgNo == ArgNo) {
      assert(DeadArgI < DeadArgsSize && "Accessing out of bounds for "
                                        "dead arguments");
      DeadArgI++;
      if (DeadArgI < DeadArgsSize)
        CurrDeadArgNo = DeadArgs[DeadArgI]->getArgNo();
      continue;
    }
    Argument *Arg = F->getArg(ArgNo);
    LiveArgsType.push_back(Arg->getType());
    LiveArgsAttrVec.push_back(ArgsAttrList.getParamAttrs(ArgNo));
  }

  assert((LiveArgsType.size() == LiveArgsAttrVec.size()) &&
         "Number of parameters is different than the number of attributes");

  // Create a new argument attributes list with the live arguments
  AttributeList NewArgsAttrList =
      AttributeList::get(F->getContext(), FnAttrs, RetAttrs, LiveArgsAttrVec);

  // Create a new function type with the live parameters
  FunctionType *NFTy =
      FunctionType::get(RetType, LiveArgsType, FTy->isVarArg());

  // Create the new function and set the attributes
  Function *NF =
      Function::Create(NFTy, F->getLinkage(), F->getAddressSpace());
  // Insert the function in the same place as the old function to prevent
  // non-deterministic IR.
  M.getFunctionList().insert(F->getIterator(), NF);
  // NOTE: copyAttributesFrom is not copying the llvm::Attributes, it is
  // copying information like visibility, linkage, etc.
  NF->copyAttributesFrom(F);
  NF->setComdat(F->getComdat());
  NF->setAttributes(NewArgsAttrList);
  NF->takeName(F);

  // Clone the metadata from the function F to the new function
  SmallVector<std::pair<unsigned, MDNode *>, 1> MDs;
  F->getAllMetadata(MDs);
  for (auto MD : MDs)
    NF->addMetadata(MD.first, *MD.second);

  // Set the inlining report for the new function
  getInlineReport()->replaceFunctionWithFunction(F, NF);
  getMDInlineReport()->replaceFunctionWithFunction(F, NF);

  // Update the call sites to function F
  std::vector<Value *> LiveArgs;
  LiveArgsAttrVec.clear();
  while (!F->use_empty()) {
    // Get the call, actual parameters and return attributes
    CallBase &Call = cast<CallBase>(*F->user_back());
    const AttributeList &CallAttrList = Call.getAttributes();
    AttrBuilder CallRAttrs(F->getContext(), CallAttrList.getRetAttrs());
    AttributeSet CallRetAttrs = AttributeSet::get(F->getContext(), CallRAttrs);
    AttributeSet CallFnAttrs = CallAttrList.getFnAttrs();

    // Collect the live actual parameters and the attributes
    DeadArgI = 0;
    CurrDeadArgNo = DeadArgs[0]->getArgNo();
    for (unsigned ArgNo = 0, ArgsNum = Call.arg_size(); ArgNo < ArgsNum;
         ArgNo++) {
      Value *ActualArg = Call.getArgOperand(ArgNo);
      if (CurrDeadArgNo == ArgNo) {
        assert(DeadArgI < DeadArgsSize && "Accessing out of bounds for "
                                          "dead arguments");
        DeadActualVect.insert(ActualArg);
        DeadArgI++;
        if (DeadArgI < DeadArgsSize)
          CurrDeadArgNo = DeadArgs[DeadArgI]->getArgNo();
        continue;
      }
      LiveArgs.push_back(ActualArg);
      LiveArgsAttrVec.push_back(CallAttrList.getParamAttrs(ArgNo));
    }

    assert(LiveArgsAttrVec.size() == LiveArgs.size() &&
           "Number of arguments is different compared with the number of "
           "attributes in the callsite");

    // Create the new attributes list
    AttributeList NewCallAttrList = AttributeList::get(
        F->getContext(), CallFnAttrs, CallRetAttrs, LiveArgsAttrVec);

    // Collect the operand bundles
    SmallVector<OperandBundleDef, 1> OpBundles;
    Call.getOperandBundlesAsDefs(OpBundles);

    // Generate the new call and insert it before the current call
    CallBase *NewCall = nullptr;
    if (InvokeInst *II = dyn_cast<InvokeInst>(&Call)) {
      NewCall = InvokeInst::Create(NF, II->getNormalDest(), II->getUnwindDest(),
                                   LiveArgs, OpBundles, "", &Call);
    } else {
      NewCall = CallInst::Create(NFTy, NF, LiveArgs, OpBundles, "", &Call);
      cast<CallInst>(NewCall)->setTailCallKind(
          cast<CallInst>(&Call)->getTailCallKind());
    }

    // Set the calling convention, attributes and copy the metadata
    NewCall->setCallingConv(Call.getCallingConv());
    NewCall->setAttributes(NewCallAttrList);

    SmallVector<std::pair<unsigned, MDNode *>, 1> MDsCall;
    Call.getAllMetadata(MDsCall);
    for (auto &MD : MDsCall)
      NewCall->setMetadata(MD.first, MD.second);

    getInlineReport()->replaceCallBaseWithCallBase(&Call, NewCall);
    getMDInlineReport()->replaceCallBaseWithCallBase(&Call, NewCall);

    // Replace the users if needed and take the name
    if (!Call.use_empty() || Call.isUsedByMetadata())
      Call.replaceAllUsesWith(NewCall);
    NewCall->takeName(&Call);

    // Delete the current call since it won't be used
    Call.eraseFromParent();
    LiveArgs.clear();
    LiveArgsAttrVec.clear();
  }

  // Move the basic blocks from function F to the new function
  NF->getBasicBlockList().splice(NF->begin(), F->getBasicBlockList());

  // Replace the users of the old arguments list with the new arguments
  DeadArgI = 0;
  CurrDeadArgNo = DeadArgs[0]->getArgNo();
  unsigned LiveArgI = 0;
  for (unsigned ArgNo = 0, ArgsNum = F->arg_size(); ArgNo < ArgsNum; ArgNo++) {
    if (CurrDeadArgNo == ArgNo) {
      assert(DeadArgI < DeadArgsSize && "Accessing out of bounds for "
                                        "dead arguments");
      DeadArgI++;
      if (DeadArgI < DeadArgsSize)
        CurrDeadArgNo = DeadArgs[DeadArgI]->getArgNo();
      continue;
    }
    Argument *Arg = F->getArg(ArgNo);
    Argument *LiveArg = NF->getArg(LiveArgI);
    Arg->replaceAllUsesWith(LiveArg);
    LiveArg->takeName(Arg);
    LiveArgI++;
  }

  LLVM_DEBUG({
    dbgs() << "    Function: " << NF->getName() << "\n"
           << "        Old number of arguments: " << F->arg_size() << "\n"
           << "        New number of arguments: " << NF->arg_size() << "\n"
           << "\n";
  });

  // Erase function F
  F->eraseFromParent();
  NumOfFunctionsTransformed++;

  return true;
}

// Helper function to apply the transformation
bool IPDeadArgElimination::applyTransformation() {
  LLVM_DEBUG(dbgs() << "  Functions transformed:\n");
  bool Changed = false;
  // Actual parameters that could be removed after applying dead arguments
  // elimination. We won't delete them until all the functions are transformed
  // since multiple functions can use the argument.
  SetVector<Value *> DeadActualVect;
  for (auto &Pair : DeadArraysArgResult)
    Changed |= removeDeadArgs(Pair.first, Pair.second, DeadActualVect);

  LLVM_DEBUG(dbgs() << "    Total functions modified: "
                    << NumOfFunctionsTransformed << "\n\n");

  if (Changed) {
    LLVM_DEBUG({
      // Printing the debug information is done in another loop because two
      // instructions can be in the same function. Every time an instruction
      // is removed, the number changes and it would be difficult for
      // analyzing and debugging.
      dbgs() << "  Actual parameters removed:\n";
      for (auto *ActualArg : DeadActualVect) {
        auto *Inst = dyn_cast<Instruction>(ActualArg);
        if (!Inst || !Inst->user_empty())
          continue;
        LLVM_DEBUG(dbgs() << "    Function: " << Inst->getFunction()->getName()
                          << "\n" << "      Instruction: " << *Inst << "\n";);
      }
    });

    for (auto *ActualArg : DeadActualVect) {
      auto *Inst = dyn_cast<Instruction>(ActualArg);
      if (!Inst || !Inst->user_empty())
        continue;
      NumOfActualParamsRemoved++;
      Inst->eraseFromParent();
    }
    LLVM_DEBUG(dbgs() << "   Total of actual parameter removed: "
                      << NumOfActualParamsRemoved << "\n");
  }

  return Changed;
}

bool IPDeadArgElimination::runImpl(WholeProgramInfo &WPInfo) {

  if (!EnableDeadArgEliminationStore)
    return false;

  LLVM_DEBUG(dbgs() << "Debug information for IPO dead arg elimination:\n");

  for (Function &F : M) {
    // The function must be local, no varargs and shouldn't be naked.
    if (F.isDeclaration() || F.arg_empty() ||
        !F.hasLocalLinkage() || F.isVarArg() || F.hasAddressTaken() ||
        F.hasFnAttribute(Attribute::Naked))
      continue;

    collectData(F);
  }

  LLVM_DEBUG({
    dbgs() << "  Candidates collected: " << DeadArgsCandidatesMap.size()
           << "\n";
    for (auto Pair : DeadArgsCandidatesMap) {
      dbgs() << "    Function: " << Pair.first->getName() << "\n";
      for (auto *Arg : Pair.second)
        dbgs() << "      Arg[" << Arg->getArgNo() << "]: " << *Arg << "\n";
      dbgs() << "\n";
    }
    dbgs() << "\n";
  });

  if (DeadArgsCandidatesMap.empty()) {
    LLVM_DEBUG(dbgs() << "    Total functions modified: "
                      << NumOfFunctionsTransformed << "\n\n");
    return false;
  }

  analyzeArguments();

  LLVM_DEBUG({
    dbgs() << "  Candidates after analysis: " << DeadArraysArgResult.size()
           << "\n";
    for (auto &Pair : DeadArraysArgResult) {
      dbgs() << "    Function: " << Pair.first->getName() << "\n";
      for (auto *Arg : Pair.second)
        dbgs() << "      Arg[" << Arg->getArgNo() << "]: " << *Arg << "\n";
      dbgs() << "\n";
    }
    dbgs() << "\n";
  });

  if (DeadArraysArgResult.empty()) {
    LLVM_DEBUG(dbgs() << "    Total functions modified: "
                      << NumOfFunctionsTransformed << "\n\n");
    return false;
  }

  bool Changed = applyTransformation();

  return Changed;
}

// New pass manager
PreservedAnalyses
IntelIPODeadArgEliminationPass::run(Module &M, ModuleAnalysisManager &AM) {

  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);
  IPDeadArgElimination DeleteDeadArgs(M);
  if (!DeleteDeadArgs.runImpl(WPInfo))
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  PA.preserve<GlobalsAA>();
  PA.preserve<AndersensAA>();
  PA.preserve<DominatorTreeAnalysis>();
  PA.preserve<LoopAnalysis>();
  return PA;
}

// Legacy pass manager
namespace {

class IntelIPODeadArgEliminationWrapper : public ModulePass {
public:
  static char ID;

  IntelIPODeadArgEliminationWrapper() : ModulePass(ID) {
    initializeIntelIPODeadArgEliminationWrapperPass(
        *PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;

    IPDeadArgElimination DeleteDeadArgs(M);
    auto WPInfo = getAnalysis<WholeProgramWrapperPass>().getResult();
    return DeleteDeadArgs.runImpl(WPInfo);
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<WholeProgramWrapperPass>();
    AU.addPreserved<WholeProgramWrapperPass>();
    AU.addPreserved<GlobalsAAWrapperPass>();
    AU.addPreserved<AndersensAAWrapperPass>();
    AU.addPreserved<DominatorTreeWrapperPass>();
    AU.addPreserved<LoopInfoWrapperPass>();
  }
};

} // end of anonymous namespace

char IntelIPODeadArgEliminationWrapper::ID = 0;

INITIALIZE_PASS_BEGIN(IntelIPODeadArgEliminationWrapper, DEBUG_TYPE,
                      "Intel IPO Dead Argument Elimination", false, false)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_END(IntelIPODeadArgEliminationWrapper, DEBUG_TYPE,
                    "Intel IPO Dead Argument Elimination", false, false)

ModulePass *llvm::createIntelIPODeadArgEliminationWrapperPass() {
  return new IntelIPODeadArgEliminationWrapper();
}
