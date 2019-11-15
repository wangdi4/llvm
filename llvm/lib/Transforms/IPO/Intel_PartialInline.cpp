//===------- Intel_PartialInline.cpp - Intel Partial Inlining      -*------===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
// This pass performs partial inlining for simple functions that return a
// boolean. The key idea is to identify those functions that will use at least
// one argument to iterate through a loop and then return true or false. The
// advantage of this optimization is that if the hot path is doesn't perform
// the loop, then we prevent the full inlining on these simple functions that
// could increase the size of the caller function and could lead to a slowdown.
// For example:
//
// define i1 @foo(%"struct.pov::Object_Struct"*) {
// ; <label> 2:
//  %3 = icmp eq %"struct.pov::Object_Struct"* %0, null
//  br i1 %3, label %12, label %4
//
// ; <label>:4:
//  %5 = phi %"struct.pov::Object_Struct"* [ %9, %4 ], [ %0, %2 ]
//  %6 = Some computation
//  ...
//  br i1 %11, label %4, label %12
//
// ; <label>:12:
//  %13 = phi i1 [ true, %2 ], [ %6, %4 ]
//  ret i1 %14
// }
//
// This is a simple function that takes the argument %0 and checks with null
// to decide if it is going through the loop or not. The compiler would like
// to inline this function since it is small. In this case, the following
// transformation will create the next function:
//
// define i1 @foo.1(%"struct::Object_Struct"*) {
// ; <label>:2:
//  %phitmp.loc = alloca i1
//  %3 = icmp eq %"struct.pov::Object_Struct"* %0, null
//  br i1 %3, label %6, label %4
//
// ; <label>:4:
//  call void @foo.1.for.body(%"struct.pov::Object_Struct"* %0, i1*phitmp.loc)
//  %phitmp.reload = load i1, i1* %phitmp.loc
//  br label %6
//
// ; <label>:6:
//  %7 = phi i1 [ true, %2 ], [ %phitmp.reload, %4 ]
//  ret i1 %7
// }
//
// This new function (@foo.1) first checks if the argument %0 will be null,
// if so then returns early, else calls @foo.1.for.body. The function
// @foo.1.for.body is the outlined function that will do the loop.
//
// define void @foo.1.for.body(%"struct::Object_Struct"*, i1* %phitmp.out) {
// newFuncRoot:
//  br label %for.body
//
// for.body.for.end_crit_edge.exitStub:
//  ret void
//
// for.body:
//  %5 = phi %"struct.pov::Object_Struct"* [ %9, %for.body ],
//                                         [ 0, %newFuncRoot ]
//  %6 = Some computation
//  ...
// %cmp = icmp eq %"struct.pov::Object_Struct"* %Var, null
//   br i1 %cmp, label %for.body.for.end_crit_edge.exitStub, label %for.body
// }
//
// All the call sites to @foo will be replaced with @foo.1. The inliner will
// treat @foo.1 as always inline, while @foo.1.for.body won't be inlined. This
// transformation will partially inline @foo into all its callers.
//
// define i1 @bar(%"struct::Object_Struct"*) #0 {
// entry:
//   %call = call zeroext i1 @foo(%struct.Node* %0)
//   ret i1 %call
// }
//
// In the previous example the partial inliner will replace the call site
// to @foo with @foo.1:
//
// define i1 @bar(%"struct_pov::Object_Struct"*) {
// entry:
//   %call = call zeroext i1 @foo.1(*struct_pov::Object_Struct %0)
//   ...
//   < do some computation with %call>
// }
//
// The full inlining pass will transform @bar as follow:
//
// define i1 @bar(%"struct_pov::Object_Struct"*) {
// ; <label>:2:
//  %phitmp.loc = alloca i1
//  %3 = icmp eq %"struct.pov::Object_Struct"* %0, null
//  br i1 %3, label %6, label %4
//
// ; <label>:4:
//  call void @foo.1.for.body(%"struct.pov::Object_Struct"* %0, i1*phitmp.loc)
//  %phitmp.reload = load i1, i1* %phitmp.loc
//  br label %6
//
// ; <label>:6:
//  %7 = phi i1 [ true, %2 ], [ %phitmp.reload, %4 ]
//  ..
//  < do some computation with %7>
// }
//
// Now @foo was partially inlined into @bar rather than fully inlined.
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/IPO/Intel_PartialInline.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Analysis/InlineCost.h"
#include "llvm/Analysis/Intel_PartialInlineAnalysis.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/Type.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/Intel_RegionSplitter.h"
#include <queue>
#include <set>

using namespace llvm;
using namespace InlineReportTypes;

#define DEBUG_TYPE "intel_partialinline"

// Limit the number of functions that will be partially inlined by this pass
static cl::opt<unsigned> IntelPIMaxFuncs("intel-pi-max-funcs", cl::init(4),
                                         cl::ReallyHidden);

// Enable the optimization for testing purposes
static cl::opt<bool> IntelPITesting("intel-pi-test", cl::init(false),
                                    cl::ReallyHidden);

// Enable the optimization if -qopt-mem-layout-trans > 0
static cl::opt<bool> DTransOptEnabled("dtrans-partial-inline",
                                      cl::init(true), cl::Hidden);

// Class for handling the partial inliner
class PartialInliner {

public:
  // Lambda function to identify the LoopInfo of the input function
  using LoopInfoFuncType = std::function<LoopInfo &(Function &)>;

  // Simple constructor used in the derived class FunctionCloner
  PartialInliner(Module &Mod, LoopInfoFuncType &LoopInfoFunc)
      : M(Mod), GetLoopInfo(LoopInfoFunc) {}

  // Main constructor
  PartialInliner(Module &Mod, LoopInfoFuncType &LoopInfoFunc,
                 std::function<BlockFrequencyInfo &(Function &)> *GetBFI,
                 std::function<BranchProbabilityInfo &(Function &)> *GetBPI,
                 std::function<DominatorTree &(Function &)> *GetDT,
                 WholeProgramInfo *WPInfo)
      : M(Mod), GetLoopInfo(LoopInfoFunc), GetBFI(GetBFI), GetBPI(GetBPI),
        GetDT(GetDT), WPInfo(WPInfo) {}

  // Run the partial inliner
  bool runImpl();

protected:
  Module &M;
  LoopInfoFuncType &GetLoopInfo;

private:
  // Return true if the function can be split into three regions:
  //   Argument read
  //   Loop
  //   Exit
  // Else, return false.
  bool canSplitFunctionIntoRegions(Function &F);

  // Return false if there is an attribute in the function that prevents
  // the partial inlining or if there is at least one indirect call to the
  // input function, else return true.
  bool checkFunctionProperties(Function &F);

  // Return the PHINode, as an Instruction, that identifies if the Value
  // used for iteration comes from an argument
  Instruction *getPHIUser(User *Usr, LoopInfo &LI,
                          SetVector<Instruction *> &VisitedInst);

  // Return true if the input branch instruction leads to the basic
  // block that will exit the function, else return false.
  bool goToExit(BranchInst *BrInst);

  // Return the argument that is used to branch into an exit block
  Value *identifyEntryRegion(Function &F);

  // Return true if the input argument is used to iterate through a
  // loop and the loop goes to the exit region. Else return false.
  bool identifyLoopRegion(Function &F, Value *ArgValue);

  // Return true if the input function is a candidate to apply the simple
  // partial inliner, else return false.
  bool isPartialInlineCandidate(Function &F, Module &M);

  // Return true if traversing the users of the input instruction leads to
  // the exit basic block, else return false.
  bool isUsedForExit(Instruction *Inst, SetVector<Instruction *> &VisitedInst);

  std::function<BlockFrequencyInfo &(Function &)> *GetBFI;
  std::function<BranchProbabilityInfo &(Function &)> *GetBPI;
  std::function<DominatorTree &(Function &)> *GetDT;

  WholeProgramInfo *WPInfo;

};

// Helper class used to create the clones and replace the call sites
class FunctionCloner : public PartialInliner {

private:

  // Create the clone from the original
  Function *createClone(
                 std::function<BlockFrequencyInfo &(Function &)> *GetBFI,
                 std::function<BranchProbabilityInfo &(Function &)> *GetBPI,
                 std::function<DominatorTree &(Function &)> *GetDT);

  // Replace the callsites of the original function with the
  // new function
  void setCallSites();

  Function *Original = nullptr;   // Original function
  Function *Clone = nullptr;      // Clone function
  Function *Outlined = nullptr;   // Outlined function

  SetVector<CallInst *> CallSites;

public:
  FunctionCloner(Module &Mod, LoopInfoFuncType &LoopInfoFunc, Function *F,
                 std::function<BlockFrequencyInfo &(Function &)> *GetBFI,
                 std::function<BranchProbabilityInfo &(Function &)> *GetBPI,
                 std::function<DominatorTree &(Function &)> *GetDT);

  // Return the cloned version
  Function *getClone() { return Clone; }

  // Return true if a clone was made sucessfully, else return false
  bool isSetupComplete() { return Clone && Outlined; }

  // Revert the changes made by the cloner
  void revertTransformation();

  // Set the attributes that will indicate that the inliner should inline
  // the partially inlinable code.
  void setFunctionsAttributes();
};

// Main constructor for the function cloner. It will create the cloned
// version of the input function and the oulined function and replace
// the callsites.
FunctionCloner::FunctionCloner(Module &Mod, LoopInfoFuncType &LoopInfoFunc,
                 Function *F,
                 std::function<BlockFrequencyInfo &(Function &)> *GetBFI,
                 std::function<BranchProbabilityInfo &(Function &)> *GetBPI,
                 std::function<DominatorTree &(Function &)> *GetDT) :
                 PartialInliner(Mod, LoopInfoFunc), Original(F) {
  Clone = createClone(GetBFI, GetBPI, GetDT);
  if (isSetupComplete())
    setCallSites();
}

// Set the attibute 'prefer-partial-inline-inlined-clone' in the Clone
// function and 'prefer-partial-inline-outlined-func' in the Outlined
// function. The inlining pass will treat these attributes as follow:
//
//   * prefer-partial-inline-inlined-clone = always-inline
//   * prefer-partial-inline--outlined-func = no-inline
//
// For example, the optimization takes the following function:
//
// define i1 @foo(%"struct.pov::Object_Struct"*) {
//  %3 = icmp eq %"struct.pov::Object_Struct"* %0, null
//  br i1 %3, label %12, label %4
//
// ; <label>:4:
//  %5 = phi %"struct.pov::Object_Struct"* [ %9, %4 ], [ %0, %2 ]
//  %6 = Some computation
//  ...
//  br i1 %11, label %4, label %12
//
// ; <label>:12:
//  %13 = phi i1 [ true, %2 ], [ %6, %4 ]
//  ret i1 %14
// }
//
// And creates the following functions:
//
// define i1 @foo.1(%"struct::Object_Struct"*) {
// ; <label>:2:
//  %phitmp.loc = alloca i1
//  %3 = icmp eq %"struct.pov::Object_Struct"* %0, null
//  br i1 %3, label %6, label %4
//
// ; <label>:4:
//  call void @foo.1.for.body(%"struct.pov::Object_Struct"* %0, i1*phitmp.loc)
//  %phitmp.reload = load i1, i1* %phitmp.loc
//  br label %6
//
// ; <label>:6:
//  %7 = phi i1 [ true, %2 ], [ %phitmp.reload, %4 ]
//  ret i1 %7
// }
//
// define void @foo.1.for.body(%"struct::Object_Struct"*, i1* %phitmp.out) {
// newFuncRoot:
//  br label %for.body
//
// for.body.for.end_crit_edge.exitStub:
//  ret void
//
// for.body:
//  %5 = phi %"struct.pov::Object_Struct"* [ %9, %for.body ],
//                                         [ 0, %newFuncRoot ]
//  %6 = Some computation
//  ...
// %cmp = icmp eq %"struct.pov::Object_Struct"* %Var, null
//   br i1 %cmp, label %for.body.for.end_crit_edge.exitStub, label %for.body
// }
//
// The function @foo.1 will be set with the attribute
// 'prefer-partial-inline-inlined-clone', and @foo.1.for.body will be set with
// 'prefer-partial-inline-outlined-func'. The inliner will fully inline
// @foo.1 and won't inline @foo.1.for.body.
void FunctionCloner::setFunctionsAttributes() {

  Clone->addFnAttr("prefer-partial-inline-inlined-clone");
  Outlined->addFnAttr("prefer-partial-inline-outlined-func");

  // Remove the no-inline attribute from the outlined function since it
  // will be replaced with "prefer-partial-inline-outlined-func". The
  // no-inline was added by the RegionSplitter.
  if (Outlined->hasFnAttribute(Attribute::NoInline))
    Outlined->removeFnAttr(Attribute::NoInline);
}

// In case something goes wrong in the analysis, then return the call sites
// to the original function, and delete the clone and outlined functions.
void FunctionCloner::revertTransformation() {

  if (!Clone || !Original || !Outlined)
    return;

  for (CallInst *Call : CallSites)
    Call->setCalledFunction(Original);

  Clone->eraseFromParent();
  Outlined->eraseFromParent();
}

// Replace the callsites of the original function with the cloned function
void FunctionCloner::setCallSites() {
  if (!Clone || !Outlined)
    return;

  for (CallInst *Call : CallSites) {

    // If the caller is the original (recursive call), then don't
    // change the callsite
    if (Call->getCaller() == Original || Call->getCaller() == Outlined ||
        Call->getCaller() == Clone)
      continue;

    Call->setCalledFunction(Clone);
  }
}

// Create a new function that will be inlined and the outlined function.
// These new functions will look as follows:
//
// define i1 @foo.1(%"struct::Object_Struct"*) {
// ; <label>:2:
//  %phitmp.loc = alloca i1
//  %3 = icmp eq %"struct.pov::Object_Struct"* %0, null
//  br i1 %3, label %6, label %4
//
// ; <label>:4:
//  call void @foo.1.for.body(%"struct.pov::Object_Struct"* %0, i1*phitmp.loc)
//  %phitmp.reload = load i1, i1* %phitmp.loc
//  br label %6
//
// ; <label>:6:
//  %7 = phi i1 [ true, %2 ], [ %phitmp.reload, %4 ]
//  ret i1 %7
// }
//
// define void @foo.1.for.body(%"struct::Object_Struct"*, i1* %phitmp.out) {
// newFuncRoot:
//  br label %for.body
//
// for.body.for.end_crit_edge.exitStub:
//  ret void
//
// for.body:
//  %5 = phi %"struct.pov::Object_Struct"* [ %9, %for.body ],
//                                         [ 0, %newFuncRoot ]
//  %6 = Some computation
//  ...
// %cmp = icmp eq %"struct.pov::Object_Struct"* %Var, null
//   br i1 %cmp, label %for.body.for.end_crit_edge.exitStub, label %for.body
// }
// The loop inside the function @foo will be replaced (@foo.1) with a call to
// the outlined function (@foo.1.for.body). The inliner will take care of
// partially inlining the inlinable region.
Function *FunctionCloner::createClone(
                 std::function<BlockFrequencyInfo &(Function &)> *GetBFI,
                 std::function<BranchProbabilityInfo &(Function &)> *GetBPI,
                 std::function<DominatorTree &(Function &)> *GetDT) {

  if (!Original)
    return nullptr;

  // Return true if the branch is being compared with null
  auto IsValidBranch = [](BranchInst *BrInst) {
    if (!BrInst)
      return false;

    if (BrInst->isUnconditional())
      return false;

    ICmpInst *Cond = dyn_cast<ICmpInst>(BrInst->getCondition());
    if (!Cond || Cond->getPredicate() != ICmpInst::ICMP_EQ)
      return false;

    Value *ArgVal = dyn_cast<Value>(Cond->getOperand(0));
    if (!ArgVal)
      return false;

    if (!(ArgVal->getType())->isPointerTy())
      return false;

    Constant *NullVal = dyn_cast<Constant>(Cond->getOperand(1));
    if (!NullVal)
      return false;

    return NullVal->isNullValue();
  };

  BasicBlock *EntryOriginal = &(Original->getEntryBlock());
  BranchInst *Branch = dyn_cast<BranchInst>(EntryOriginal->getTerminator());
  if (!Branch || !IsValidBranch(Branch)) {
    return nullptr;
  }

  ValueToValueMapTy VMap;
  Function *NewFn = CloneFunction(Original, VMap);

  BasicBlock *EntryBB = &(NewFn->getEntryBlock());
  BasicBlock *ExitBB = &(NewFn->back());

  LoopInfo &LI = (GetLoopInfo)(*NewFn);

  if (LI.empty()) {
    NewFn->eraseFromParent();
    return nullptr;
  }

  Loop *FirstLoop = nullptr;

  BlockFrequencyInfo &BFI = (*GetBFI)(*NewFn);
  BranchProbabilityInfo &BPI = (*GetBPI)(*NewFn);
  DominatorTree &DT = (*GetDT)(*NewFn);

  RegionSplitter Splinter(DT, BFI, BPI);
  SplinterRegionT Region;

  for (BasicBlock &BB : *NewFn) {

    if (&BB == EntryBB || &BB == ExitBB)
      continue;

    // Collect the basic blocks that compose the loop
    Loop *LoopBB = LI.getLoopFor(&BB);
    if (LoopBB) {
      Region.insert(&BB);
      if (!FirstLoop)
        FirstLoop = LoopBB;
    }
  }

  if (!FirstLoop) {
    NewFn->eraseFromParent();
    return nullptr;
  }

  // Fix the basic blocks if there is a single entry and a single exit
  if (Splinter.isSingleEntrySingleExit(Region))
    Outlined = Splinter.splitRegion(Region);
  // Else, use the loop body of there are multiple exits
  else
    Outlined = Splinter.splitRegion(*FirstLoop);

  if (Outlined) {
    // Store the CallSites from the original function
    for (User *User : Original->users()) {
      CallInst *Call = dyn_cast<CallInst>(User);
      if (!Call)
        continue;

      CallSites.insert(Call);
    }
  }
  else {
    NewFn->eraseFromParent();
  }

  return NewFn;
}

// This is the main function for partial inlining. The key idea is to identify
// small functions that have the following behavior:
//
// define i1 @foo(%"struct.pov::Object_Struct"*) {
//  %3 = icmp eq %"struct.pov::Object_Struct"* %0, null
//  br i1 %3, label %12, label %4
//
// ; <label>:4:
//  %5 = phi %"struct.pov::Object_Struct"* [ %9, %4 ], [ %0, %2 ]
//  %6 = Some computation
//  ...
//  br i1 %11, label %4, label %12
//
// ; <label>:12:
//  %13 = phi i1 [ true, %2 ], [ $6, %4 ]
//  ret i1 %14
// }
//
// The function can be split into three regions:
//   Argument region: the argument is a pointer that used to go into
//                    two basic blocks: the first branch is an basic
//                    block that will exit the function and the second
//                    one is a loop. In the example, the argument region
//                    is composed by the basic block %2.
//   Loop region:     A group of basic blocks that will iterate using
//                    the input argument (BasicBlock %4 from the example).
//   Exit region:     the basic block either returns a default value if the
//                    incoming path is from the entry basic block or
//                    has a computed value if the incoming path is
//                    from the loop.
//                    (e.g. BasicBlock %12 from the example)
//
// The functions that we are identifying here are small and inliner
// would fully inline them. The issue is that they can increase the size
// of the caller enough to cause a slow down if the loop isn't taken.
//
// The following optimization will create these functions:
//
// define i1 @foo.1(%"struct::Object_Struct"*) {
// ; <label>:2:
//  %phitmp.loc = alloca i1
//  %3 = icmp eq %"struct.pov::Object_Struct"* %0, null
//  br i1 %3, label %6, label %4
//
// ; <label>:4:
//  call void @foo.1.for.body(%"struct.pov::Object_Struct"* %0, i1*phitmp.loc)
//  %phitmp.reload = load i1, i1* %phitmp.loc
//  br label %6
//
// ; <label>:6:
//  %7 = phi i1 [ true, %2 ], [ %phitmp.reload, %4 ]
//  ret i1 %7
// }
//
// define void @foo.1.for.body(%"struct::Object_Struct"*, i1* %phitmp.out) {
// newFuncRoot:
//  br label %for.body
//
// for.body.for.end_crit_edge.exitStub:
//  ret void
//
// for.body:
//  %5 = phi %"struct.pov::Object_Struct"* [ %9, %for.body ],
//                                         [ 0, %newFuncRoot ]
//  %6 = Some computation
//  ...
// %cmp = icmp eq %"struct.pov::Object_Struct"* %Var, null
//   br i1 %cmp, label %for.body.for.end_crit_edge.exitStub, label %for.body
// }
//
// This new function (@foo.1) first checks if the argument %0 will be null,
// if so then returns early, else calls @foo.for.body. The function
// @foo.1.for.body is the outlined function that will do the loop.
//
// All the call sites to @foo will be replaced with @foo.1. The function
// @foo.1 is set to 'prefer-partial-inline-inlined-clone', while the
// function @foo.1.for.body is set as 'prefer-partial-inline-outlined-func'.
// These attributes can be seen as follows:
//
//   * prefer-partial-inline-inlined-clone = always-inline
//   * prefer-partial-inline-outlined-original = no-inline
//
// The new function (@foo.1) will be inlined by the actual inlining pass and
// the outlined function (@foo.1.for.body) won't be inlined. In simple words,
// @foo.1 represents the section that will be inlined during partial inlining,
// while @foo.1.for.body is the outlined function.
//
// NOTE: This pass won't do any inlining itself, it just set the attributes
// and do the proper transformations so that the actual inliner creates a
// partial inlining.
bool PartialInliner::runImpl() {

  // Check for AVX2 or higher
  auto TTIOptLevel = TargetTransformInfo::AdvancedOptLevel::AO_TargetHasAVX2;
  if (!WPInfo || !WPInfo->isAdvancedOptEnabled(TTIOptLevel))
    return false;

  // Check if we are running for testing, else check if DTrans is
  // enabled and if whole program is safe
  if (!IntelPITesting && (!DTransOptEnabled || !WPInfo->isWholeProgramSafe()))
    return false;

  SetVector<Function *> PartialInlineFuncs;

  // Identify the candidates for partial inlining
  for (Function &F : M) {

    if (F.isDeclaration())
      continue;

    if (isIntelPartialInlineCandidate(&F, GetLoopInfo, false)) {

      PartialInlineFuncs.insert(&F);

      if (PartialInlineFuncs.size() > IntelPIMaxFuncs) {
        LLVM_DEBUG(dbgs() << "Number of candidates exceed the max candidates"
                          << " for partial inlining\n");
        return false;
      }
    }
  }

  if (PartialInlineFuncs.empty()) {
    LLVM_DEBUG(dbgs() << "No candidates for partial inlining\n");
    return false;
  }

  LLVM_DEBUG({
    dbgs() << "Candidates for partial inlining: " << PartialInlineFuncs.size()
           << "\n";
    for (Function *F : PartialInlineFuncs)
      dbgs() << "    " << F->getName() << "\n";
  });

  // Apply the partial inlining
  for (Function *F : PartialInlineFuncs) {

    LLVM_DEBUG(dbgs() << "Analyzing Function: " << F->getName() << "\n");


    FunctionCloner Clone(M, GetLoopInfo, F, GetBFI, GetBPI, GetDT);
    if (!Clone.isSetupComplete()) {
      LLVM_DEBUG(dbgs() << "    Result: Function can't be cloned\n");
      continue;
    }

    InlineReason Reason;
    if (!isInlineViable(*(Clone.getClone()), Reason)) {
      LLVM_DEBUG(dbgs() << "    Result: Inline is not viable\n");
      Clone.revertTransformation();
    }
    else {
      Clone.setFunctionsAttributes();
      LLVM_DEBUG(dbgs() << "    Result: Can partial inline\n");
    }
  }

  return true;
}

namespace {

struct IntelPartialInlineLegacyPass : public ModulePass {
public:
  static char ID; // Pass identification, replacement for typeid
  IntelPartialInlineLegacyPass() : ModulePass(ID) {
    initializeIntelPartialInlineLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequired<WholeProgramWrapperPass>();
    AU.addRequired<BlockFrequencyInfoWrapperPass>();
    AU.addRequired<BranchProbabilityInfoWrapperPass>();
    AU.addRequired<DominatorTreeWrapperPass>();

    AU.addPreserved<WholeProgramWrapperPass>();
  }

  bool runOnModule(Module &M) override {

    if (skipModule(M))
      return false;

    // Lambda function to find the LoopInfo related to an input function
    PartialInliner::LoopInfoFuncType GetLoopInfo =
        [this](Function &F) -> LoopInfo & {
      return this->getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();
    };

    WholeProgramInfo *WPInfo =
        &getAnalysis<WholeProgramWrapperPass>().getResult();

    std::function<BlockFrequencyInfo &(Function &)> GetBFI =
      [this](Function &F) -> BlockFrequencyInfo & {
      return this->getAnalysis<BlockFrequencyInfoWrapperPass>(F).getBFI();
    };

    std::function<BranchProbabilityInfo &(Function &)> GetBPI =
      [this](Function &F) -> BranchProbabilityInfo & {
      return this->getAnalysis<BranchProbabilityInfoWrapperPass>(F).getBPI();
    };

    std::function<DominatorTree &(Function &)> GetDT =
      [this](Function &F) -> DominatorTree & {
      return this->getAnalysis<DominatorTreeWrapperPass>(F).getDomTree();
    };

    // Implementation of the optimization
    return PartialInliner(M, GetLoopInfo, &GetBFI, &GetBPI, &GetDT,
                          WPInfo).runImpl();
  }
};

} // namespace

char IntelPartialInlineLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(IntelPartialInlineLegacyPass, "intel-partialinline",
                      "Intel partial inlining", false, false)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_DEPENDENCY(BlockFrequencyInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(BranchProbabilityInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_END(IntelPartialInlineLegacyPass, "intel-partialinline",
                    "Intel partial inlining", false, false)

ModulePass *llvm::createIntelPartialInlineLegacyPass() {
  return new IntelPartialInlineLegacyPass();
}

IntelPartialInlinePass::IntelPartialInlinePass() {}

PreservedAnalyses IntelPartialInlinePass::run(Module &M,
                                              ModuleAnalysisManager &AM) {

  FunctionAnalysisManager &FAM =
      AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();

  PartialInliner::LoopInfoFuncType GetLoopInfo =
      [&FAM](Function &F) -> LoopInfo & {
    return FAM.getResult<LoopAnalysis>(F);
  };

  std::function<BlockFrequencyInfo &(Function &)> GetBFI =
      [&FAM](Function &F) -> BlockFrequencyInfo & {
    return FAM.getResult<BlockFrequencyAnalysis>(F);
  };

  std::function<BranchProbabilityInfo &(Function &)> GetBPI =
      [&FAM](Function &F) -> BranchProbabilityInfo & {
    return FAM.getResult<BranchProbabilityAnalysis>(F);
  };

  std::function<DominatorTree &(Function &)> GetDT =
      [&FAM](Function &F) -> DominatorTree & {
    return FAM.getResult<DominatorTreeAnalysis>(F);
  };

  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);

  if (!(PartialInliner(M, GetLoopInfo, &GetBFI, &GetBPI, &GetDT,
                       &WPInfo).runImpl()))
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();

  return PA;
}
