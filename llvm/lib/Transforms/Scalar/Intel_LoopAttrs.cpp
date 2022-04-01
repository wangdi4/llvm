//===------      Intel_LoopAttrs.cpp - Compute loop attributes      -*-----===//
//
// Copyright (C) 2021-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass goes through each loop in a function and checks which attributes
// or metadata can be added for the loop. For example:
//
//   13:                                               ; preds = %23, %10
//     %14 = phi i64 [ 0, %10 ], [ %15, %23 ]
//     %15 = add nuw nsw i64 %14, 1
//     %16 = getelementptr inbounds i64, i64* %4, i64 %15
//     %17 = load i64, i64* %16, align 8, !tbaa !14
//     %18 = icmp sgt i64 %17, %0
//     br i1 %18, label %19, label %23
//
//   19:                                               ; preds = %13
//     %20 = getelementptr inbounds i64, i64* %4, i64 %14
//     %21 = load i64, i64* %20, align 8, !tbaa !14
//     %22 = icmp sgt i64 %21, %0
//     br i1 %22, label %23, label %25
//
//   23:                                               ; preds = %19, %13
//     %24 = icmp eq i64 %15, %8
//     br i1 %24, label %27, label %13
//
//   25:                                               ; preds = %19
//     %26 = trunc i64 %14 to i32
//     br label %27
//
// The example above represents a loop that will terminate when %15 is equal
// to %8, or when %21 is equal to %0. This loop will be identified by the
// LoopInfo analysis and different attributes could be applied to it depending
// on the analysis performed. Also, this pass will add the proper attributes
// for the function if the attributes set for the loops can affect it.
//
// Current attributes supported:
//   * "mustprogress" : loop will never produce an infinite loop
//   * "prefer-function-level-region" : function can be treated as loop region
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Scalar/Intel_LoopAttrs.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/Intel_DopeVectorAnalysis.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/MustExecute.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;
using namespace dvanalysis;

#define DEBUG_TYPE "intel-loop-attrs"

// Control if the pass should apply the attributes
static cl::opt<bool>
    EnableIntelLoopAttrs("enable-loop-attrs", cl::init(true), cl::Hidden,
                         cl::desc("Enable Loop attributes"));

// Control if the "mustprogress" attribute should always be applied
static cl::opt<bool>
    ForceMustProgress("force-intel-must-progress", cl::init(false),
                      cl::Hidden, cl::desc("Apply mustprogress always"));

// Control the lowest number of dope vectors that a function must have
// as parameters for "prefer-function-level-region"
static cl::opt<uint64_t>
    MinDopeVectors("min-param-dope-vectors", cl::init(27),
                   cl::Hidden, cl::desc("Minimum number of dope vectors"));

// Percentage of BasicBlocks that represent loops in a Function
static cl::opt<double>
    PercentLoopsThreshold("prefer-func-level-threshold", cl::init(60.0),
    cl::Hidden, cl::desc("Percent of blocks that should be used in loops"));

// Control if "prefer-function-level-region" should always be applied if
// DTrans is enabled or not.
static cl::opt<bool>
    ForceFuncRegion("force-intel-prefer-func-level-region", cl::init(false),
    cl::Hidden, cl::desc("Apply prefer-function-level-region always"));

STATISTIC(NumMustProgressLoops, "Number of loops marked as mustprogress");
STATISTIC(NumMustProgressFuncs, "Number of functions marked as mustprogress");
STATISTIC(NumPreferFuncLevelRegion, "Number of functions marked as "
                                    "prefer-function-level-region");

// Helper class to handle the analyses used by this pass
class AnalysesHandler {
public:
  AnalysesHandler(LoopInfo *LI, ScalarEvolution *SE, bool EnableDTrans) :
                  LI(LI), SE(SE), EnableDTrans(EnableDTrans) { }

  // Return the LoopInfo analysis result
  auto getLoopInfo() { return LI; }

  // Return the scalar evolution analysis result
  auto getScalarEvolution() { return SE; }

  // Return true if the input function is main or one of its forms
  bool isMainEntryPoint(StringRef FuncName) {
    return WPUtils.isMainEntryPoint(FuncName);
  }

  // Return true if DTrans is enabled
  bool isDTransEnabled() { return EnableDTrans; }

private:
  LoopInfo *LI;
  ScalarEvolution *SE;
  WholeProgramUtils WPUtils;
  bool EnableDTrans;
};

// Helper class to set the attributes for a function
class LoopAttrsImpl {
public:
  LoopAttrsImpl(Function &F, AnalysesHandler &Anls) : F(F), Anls(Anls) {}
  bool run();

private:
  Function &F;
  AnalysesHandler &Anls;

  // Return true if at least one loop was marked "mustprogress"
  bool loopsMustProgress();

  // Return true if the function can be marked as
  // "prefer-function-level-region"
  bool preferFunctionLevelRegionDueToDV();
};

// Return true if at least one loop in function F can be marked
// "mustprogress". For example:
//
//   13:                                               ; preds = %23, %10
//     %14 = phi i64 [ 0, %10 ], [ %15, %23 ]
//     %15 = add nuw nsw i64 %14, 1
//     %16 = getelementptr inbounds i64, i64* %4, i64 %15
//     %17 = load i64, i64* %16, align 8, !tbaa !14
//     %18 = icmp sgt i64 %17, %0
//     br i1 %18, label %19, label %23
//
//   19:                                               ; preds = %13
//     %20 = getelementptr inbounds i64, i64* %4, i64 %14
//     %21 = load i64, i64* %20, align 8, !tbaa !14
//     %22 = icmp sgt i64 %21, %0
//     br i1 %22, label %23, label %25
//
//   23:                                               ; preds = %19, %13
//     %24 = icmp eq i64 %15, %8
//     br i1 %24, label %27, label %13
//
//   25:                                               ; preds = %19
//     %26 = trunc i64 %14 to i32
//     br label %27
//
// The example above represents a loop that will terminate when %15 is equal
// to %8, or when %21 is equal to %0. Because %8 and %0 are invariant inside
// the loop, then it means that it will terminate. In this case the loop will
// be set as "mustprogress".
//
// If all loops are set as "mustprogress" or there are no loops in the function,
// then the pass will set the attribute "mustprogress" in the function.
bool LoopAttrsImpl::loopsMustProgress() {

  // Given the operands from an icmp instruction, check if OpZero was created
  // in the loop Lp, and OpOne never changes in the loop. This function looks
  // for the following:
  //
  //   %17 = load i64, i64* %16, align 8, !tbaa !14
  //   %18 = icmp sgt i64 %17, %0
  //   br i1 %18, label %19, label %23
  //
  // Assume that the instructions above are in a basic block contained by a
  // loop. The instruction %17 changes in the loop, %0 is an argument and
  // its value won't change in the loop. In this case the exit condition
  // in the branch will be reached at certain point.
  auto LoopBoundsNeverChange = [](Value *OpZero,
                                   Value *OpOne,
                                   Loop *Lp) -> bool {

    // Operand 0 must be created in the loop
    Instruction *InstZero = dyn_cast<Instruction>(OpZero);
    if (!InstZero)
      return false;

    if (!Lp->contains(InstZero->getParent()))
      return false;

    // Operand 1 shouldn't be created in the loop, and the only
    // use in the loop is to compare for exiting the loop
    if (auto *I = dyn_cast<Instruction>(OpOne)) {
      if (Lp->contains(I->getParent()))
        return false;
    } else if (!isa<Argument>(OpOne) && !isa<ConstantInt>(OpOne)) {
      return false;
    }

    // The only users inside the loop must be for branching
    for (auto *U : OpOne->users()) {
      auto *I = dyn_cast<Instruction>(U);
      if (!I)
        return false;

      // If the use is not inside the loop continue
      auto *BB = I->getParent();
      if (!Lp->contains(BB))
        continue;

      auto *Br = dyn_cast<BranchInst>(BB->getTerminator());
      if (!Br)
        return false;

      // The only use in the loop is for branching
      if (!I->hasOneUser() || I->user_back() != Br)
        return false;
    }

    return true;
  };

  // Return true if there is no call instruction in the input
  // basic block
  auto CallInBlock = [](BasicBlock &BB) -> bool {
    for (auto &I : BB) {
      if (isa<CallBase>(&I))
        return true;
    }
    return false;
  };

  // Return true if the input loop has bounds, the bounds won't change inside
  // the loop and there are no nested loops.
  auto IsSimpleLoopAndMustProgress = [&LoopBoundsNeverChange,
      &CallInBlock](Loop *Lp, ScalarEvolution *SE) -> bool {

    // Nested loops aren't allowed at the moment.
    if (Lp->getSubLoops().size() != 0 || Lp->getParentLoop())
      return false;

    // The loop must have an exit block.
    if (Lp->hasNoExitBlocks())
      return false;

    // There shouldn't be any call inside the loop
    for (auto *BB : Lp->blocks())
      if (CallInBlock(*BB))
        return false;

    // The conditions to exit the loop must never change inside the loop.
    SmallVector<BasicBlock *, 3> ExitBlocks;
    Lp->getExitingBlocks(ExitBlocks);
    for (auto *BB : ExitBlocks) {
      auto *Br = dyn_cast<BranchInst>(BB->getTerminator());
      if (!Br || Br->isUnconditional())
        return false;

      ICmpInst *IC = dyn_cast<ICmpInst>(Br->getCondition());
      if (!IC)
        return false;

      if (!LoopBoundsNeverChange(IC->getOperand(0), IC->getOperand(1), Lp) &&
          !LoopBoundsNeverChange(IC->getOperand(1), IC->getOperand(0), Lp))
        return false;
    }

    // Use the scalar evolution analysis to find if the bounds could be
    // computed.
    auto *SCEVResult = SE->getSymbolicMaxBackedgeTakenCount(Lp);
    return !isa<SCEVCouldNotCompute>(SCEVResult);
  };

  // If the function doesn't have IR then the libfuncs table should take care
  // of marking it as "mustprogress". Also, if the function is "mustprogress"
  // then there is nothing to do.
  if (F.isDeclaration() || F.mustProgress())
    return false;

  // If the function is not main, is dead and ForceMustProgress is disabled
  // then don't apply the attribute. This function will be deleted.
  if (!Anls.isMainEntryPoint(F.getName()) &&
      F.user_empty() && !ForceMustProgress)
    return false;

  LLVM_DEBUG(dbgs() << "Loop \"mustprogress\" results for function "
                    << F.getName() << ":\n");

  bool FunctionMustProgress = true;
  auto *LI = Anls.getLoopInfo();

  // If the loop info is empty then there is nothing else to do.
  if (LI->empty())
    return false;

  // If the function contains an irreducible control flow then we can't
  // add the "mustprogress" attribute.
  if (mayContainIrreducibleControl(F, LI))
    return false;

  // Don't set "mustprogress" attribute for main
  if (Anls.isMainEntryPoint(F.getName()))
    FunctionMustProgress = false;

  auto *SE = Anls.getScalarEvolution();
  bool Changed = false;
  for (auto &BB : F) {

    bool FoundCallInBlock = false;

    // Check if there is any call instruction inside the function, if so
    // then the function can't be marked as "mustprogress"
    if (CallInBlock(BB)) {
      FoundCallInBlock = true;
      FunctionMustProgress = false;
    }

    // Check if the current basic block represents a loop.
    // If so then check if the bounds could be computed
    // and set "mustprogress" for it.
    Loop *Lp = LI->getLoopFor(&BB);
    if (Lp && !FoundCallInBlock) {

      // If loop is "mustprogress" then there is nothing to do.
      MDNode *LoopMustProgressAttr =
          llvm::findOptionMDForLoop(Lp, "llvm.loop.mustprogress");
      if (LoopMustProgressAttr)
        continue;

      bool LoopMustProgress = IsSimpleLoopAndMustProgress(Lp, SE);
      if (LoopMustProgress) {
        Lp->setLoopMustProgress();
        Changed = true;
        NumMustProgressLoops++;
      } else {
        // Bounds couldn't be collected
        FunctionMustProgress = false;
      }

      LLVM_DEBUG(dbgs() << "    Result for function " << F.getName() << ", "
                        << "loop in BB " << BB.getNameOrAsOperand() << ": "
                        << " \"mustprogress\" loop attribute"
                        << (LoopMustProgress ? " " : " NOT ") << "added\n");
    }
  }

  // Set the function attribute if all loops "mustprogress"
  if (FunctionMustProgress) {
    F.setMustProgress();
    NumMustProgressFuncs++;
    LLVM_DEBUG(dbgs() << "  \"mustprogress\" function attribute added to "
                      << "function " << F.getName() << "\n");
  }

  LLVM_DEBUG(dbgs() << "\n");

  return Changed;
}

// Add the function attribute "prefer-function-level-region" if the function
// has a high number of dope vectors as parameters, and they are used in many
// nested loops. If the array pointer of a dope vector is used in multiple
// nested loops, then there is a chance that these loops may have a similar
// trip count, or they are touching the same data. The loop optimizer may want
// to apply some optimizations in this case (e.g. fuse, peeling, tiling). If
// the function has many dope vectors with the same property then the loop
// optimizer may want to treat the entire function as the loop region to
// perform optimizations across all loops.
bool LoopAttrsImpl::preferFunctionLevelRegionDueToDV() {

  // Enum to identify if there are 0, 1 or more than 1 nested loop
  enum NumberOfNestedLoopsResult {
    ZeroLoops = 0,        // No nested loops were found
    OneLoop,              // One nested loop was found
    ManyLoops             // More than one nested loop was found
  };

  // If Lp is in a loop nest and is not the outermost loop, then traverse
  // through the parent loops and collect the outermost. Else return Lp.
  auto GetOutermostLoop = [](Loop *Lp) -> Loop * {
    Loop *OutermostLoop = nullptr;
    Loop *CurrLoop = Lp;
    while (CurrLoop) {
      OutermostLoop = CurrLoop;
      CurrLoop = OutermostLoop->getParentLoop();
    }

    return OutermostLoop;
  };

  // Given an argument that is a dope vector, collect the number
  // of nested loops where the array pointer is used and return
  // the NumberOfNestedLoopsResult
  auto CheckLoopsForDopeVectorArgs = [&GetOutermostLoop](Argument *Arg,
      DenseMap<BasicBlock*, Loop*> &TotalLoops) -> NumberOfNestedLoopsResult {

    // Should be marked as "ptrnoalias", "assume_shape" and
    // "noalias"
    if (!Arg->hasAttribute("ptrnoalias") ||
        !Arg->hasAttribute("assumed_shape") ||
        !Arg->hasNoAliasAttr())
      return ZeroLoops;

    uint64_t LoopUse = 0;
    SetVector<Loop*> OutermostLoopsVisited;

    for (auto *U : Arg->users()) {

      GEPOperator *GEPO = dyn_cast<GEPOperator>(U);
      if (!GEPO)
        continue;

      DopeVectorFieldType DVFieldType =
        DopeVectorAnalyzer::identifyDopeVectorField(*GEPO);

      // We are just interested in which loops the array is used
      if (DVFieldType != DopeVectorFieldType::DV_ArrayPtr)
        continue;

      for (auto *GEPUse : GEPO->users()) {

        // Load the array
        LoadInst *LoadI = dyn_cast<LoadInst>(GEPUse);
        if (!LoadI)
          continue;

        // The users (most likely subscript instructions) are in a loop
        for (auto *LoadUse : LoadI->users()) {
          auto *InstUser = dyn_cast<Instruction>(LoadUse);
          if (!InstUser)
            continue;

          if (TotalLoops.count(InstUser->getParent()) == 0)
            continue;

          Loop *Lp = TotalLoops[InstUser->getParent()];
          assert (Lp && "Null loop in the total loops map");

          // We are interested in nested loops
          if (!Lp->isInnermost() && !Lp->isOutermost())
            continue;

          // Collect the outermost loop
          Loop *OutermostLoop = GetOutermostLoop(Lp);

          // Don't count repeated entries
          if (OutermostLoopsVisited.insert(OutermostLoop))
            LoopUse++;

          // More than 1 loop
          if (LoopUse >= 2)
            return ManyLoops;
        }
      }
    }

    // One loop
    if (LoopUse == 1)
      return OneLoop;

    // Zero loop
    return ZeroLoops;
  };

  // DTrans must be enabled
  if (!Anls.isDTransEnabled() && !ForceFuncRegion)
    return false;

  // Dope vectors are only for Fortran
  if (!F.isFortran())
    return false;

  // Do not apply to _MAIN
  if (Anls.isMainEntryPoint(F.getName()))
    return false;

  auto *LI = Anls.getLoopInfo();
  uint64_t NumOfDopeVectorsWithOneLoop = 0;
  uint64_t NumOfDopeVectorsWithManyLoops = 0;
  uint64_t NumOfDopeVectorsWithZeroLoops = 0;
  SetVector<Argument*> DopeVectorArgs;

  // Collect all the arguments that are dope vectors
  for (auto &Arg : F.args()) {

    if(Arg.user_empty())
      continue;

    Type *ArgType = Arg.getType();
    if (!ArgType->isPointerTy())
      continue;

    // We are interested in the arguments that are marked as "assumed_shape"
    // and "ptrnoalias" since they could be dope vectors.
    if (!Arg.hasAttribute("ptrnoalias") ||
        !Arg.hasAttribute("assumed_shape"))
      continue;

    DopeVectorArgs.insert(&Arg);
  }

  if (DopeVectorArgs.empty())
    return false;

  // Collect all the basic blocks that are used for loops
  DenseMap<BasicBlock*, Loop*> TotalLoops;
  for (auto &BB : F.getBasicBlockList()) {
    Loop *Lp = LI->getLoopFor(&BB);
    if (Lp)
      TotalLoops.insert({&BB, Lp});
  }

  // There are no loops, nothing to do
  if (TotalLoops.empty())
    return false;

  // If the percent of block used for loops is lower than the threshold then
  // don't do anything. We are expecting that the function has a large number
  // of loops.
  double PercentOfLoops = ((double)TotalLoops.size() / (double)F.size()) * 100;
  if (PercentOfLoops < PercentLoopsThreshold)
    return false;

  // Traverse through the arguments, if it is a dope vector then
  // collect the number of loops
  for (auto *Arg : DopeVectorArgs) {

    // Collect if the dope vector is used in many loops
    NumberOfNestedLoopsResult NumberOfLoops =
        CheckLoopsForDopeVectorArgs(Arg, TotalLoops);
    switch(NumberOfLoops) {
    case ZeroLoops:
      NumOfDopeVectorsWithZeroLoops++;
      break;
    case OneLoop:
      NumOfDopeVectorsWithOneLoop++;
      break;
    case ManyLoops:
      NumOfDopeVectorsWithManyLoops++;
      break;
    }
  }

  uint64_t TotalDopeVectorsWithLoops =
      NumOfDopeVectorsWithOneLoop + NumOfDopeVectorsWithManyLoops;

  // The number of dope vectors used in loops must be higher than the
  // threshold
  if (TotalDopeVectorsWithLoops < MinDopeVectors)
    return false;

  // The number of dope vectors used in loops should be higher than the number
  // of dope vectors not used in loops
  if (TotalDopeVectorsWithLoops < NumOfDopeVectorsWithZeroLoops)
    return false;

  // The number of dope vectors used in multiple loops should be higher than
  // the number of dope vectors used in one loop
  if (NumOfDopeVectorsWithManyLoops < NumOfDopeVectorsWithOneLoop)
    return false;

  F.addFnAttr("prefer-function-level-region");
  NumPreferFuncLevelRegion++;

  LLVM_DEBUG(dbgs() << "Attribute \"prefer-function-level-region\" added to "
                    << "function " << F.getName() << "\n");

  return true;
}

// Main function for handling the attributes
bool LoopAttrsImpl::run() {
  if (!EnableIntelLoopAttrs)
    return false;

  bool Changed = false;

  Changed |= loopsMustProgress();
  Changed |= preferFunctionLevelRegionDueToDV();

  return Changed;
}

// New pass manager
PreservedAnalyses IntelLoopAttrsPass::run(Function &F,
                                          FunctionAnalysisManager &AM) {

  auto *LI = &AM.getResult<LoopAnalysis>(F);
  auto *SE = &AM.getResult<ScalarEvolutionAnalysis>(F);

  AnalysesHandler Anls(LI, SE, EnableDTrans);
  LoopAttrsImpl LoopAttrs(F, Anls);

  if (!LoopAttrs.run())
    return PreservedAnalyses::all();

  auto PA = PreservedAnalyses();
  PA.preserve<ScalarEvolutionAnalysis>();
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}

// Legacy pass manager
namespace {

class IntelLoopAttrsWrapper : public FunctionPass {
  bool EnableDTrans;

public:
  static char ID;

  IntelLoopAttrsWrapper() : FunctionPass(ID), EnableDTrans(false) {
    initializeIntelLoopAttrsWrapperPass(*PassRegistry::getPassRegistry());
  }

  IntelLoopAttrsWrapper(bool EnableDTrans) : FunctionPass(ID),
                                             EnableDTrans(EnableDTrans) {
    initializeIntelLoopAttrsWrapperPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F))
      return false;

    auto *LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
    auto *SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();

    AnalysesHandler Anls(LI, SE, EnableDTrans);
    LoopAttrsImpl LoopAttrs(F, Anls);

    return LoopAttrs.run();
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addPreserved<WholeProgramWrapperPass>();
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequired<ScalarEvolutionWrapperPass>();
    AU.addPreserved<ScalarEvolutionWrapperPass>();
  }
};

} // end of anonymous namespace

char IntelLoopAttrsWrapper::ID = 0;

INITIALIZE_PASS_BEGIN(IntelLoopAttrsWrapper, DEBUG_TYPE, "Intel Loop Attrs",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolutionWrapperPass)
INITIALIZE_PASS_END(IntelLoopAttrsWrapper, DEBUG_TYPE, "Intel Loop Attrs",
                    false, false)

FunctionPass *llvm::createIntelLoopAttrsWrapperPass(bool EnableDTrans) {
  return new IntelLoopAttrsWrapper(EnableDTrans);
}