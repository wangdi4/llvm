//===----------- Intel_TileMVInlMarker.cpp --------------------------------===//
//
// Copyright (C) 2020-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
// This transformation applies multiversioning to a collection of BasicBlocks
// in a key function of an application, and marks certain callsites for
// inlining in the true branch of the multiversioned code. The inlined
// functions have loops that are good candidates for tiling.
// The inliner inlines the marked functions. Loop Opt performs the tiling.
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/IPO/Intel_TileMVInlMarker.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/Utils/Intel_IPOUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"

using namespace llvm;
using namespace llvm::PatternMatch;

#define DEBUG_TYPE "tilemvinlmarker"

// Enable the optimization for testing purposes
static cl::opt<bool> TileCandidateTest("tile-candidate-test", cl::init(false),
                                       cl::ReallyHidden);
//
// Minimum number of tile candidate functions for this transformation.
//
static cl::opt<unsigned>
    TileCandidateMin("tile-candidate-min", cl::init(6), cl::ReallyHidden,
                     cl::desc("Minimum number of tile candidate functions"));

//
// A function can only be considered a tile candidate if it has at least
// this many arguments.
//
static cl::opt<unsigned> TileCandidateArgMin(
    "tile-candidate-arg-min", cl::init(8), cl::ReallyHidden,
    cl::desc("Minimum number of args in a tile candidate function"));

//
// A function can only be considered a tile candidate if it has at least
// this many array arguments.
//
static cl::opt<unsigned> TileCandidateSubArgMin(
    "tile-candidate-sub-arg-min", cl::init(6), cl::ReallyHidden,
    cl::desc("Minimum number of subscript args in a tile candidate function"));

//
// Utility functions
//

//
// Return the unique caller of 'F', if there is a unique caller.
//
static Function *uniqueCaller(Function &F) {
  Function *Caller = nullptr;
  for (User *U : F.users()) {
    auto CB = dyn_cast<CallInst>(U);
    if (!CB || Caller)
      return nullptr;
    Caller = CB->getCaller();
  }
  return Caller;
}

//
// Return the unique callsite of 'F', if there is a unique callsite.
//
static CallInst *uniqueCallSite(Function &F) {
  CallInst *CISave = nullptr;
  for (User *U : F.users()) {
    auto CI = dyn_cast<CallInst>(U);
    if (!CI || CISave)
      return nullptr;
    CISave = CI;
  }
  return CISave;
}

//
// Class to perform the analysis, multiversioning, and marking functions
// for inling that make up this transformation.
//
class TileMVInlMarker {

public:
  // Lambda function to identify the LoopInfo of the input function
  using LoopInfoFuncType = std::function<LoopInfo &(Function &)>;

  // Main constructor
  TileMVInlMarker(Module &M, LoopInfoFuncType &LoopInfoFunc,
                  std::function<DominatorTree &(Function &)> *GetDT,
                  std::function<PostDominatorTree &(Function &)> *GetPDT,
                  WholeProgramInfo *WPInfo)
      : M(M), GetLoopInfo(LoopInfoFunc), GetDT(GetDT), GetPDT(GetPDT),
        WPInfo(WPInfo) {}

  // Run the tile multiversioning and inline marker
  bool runImpl();

private:
  Module &M;
  LoopInfoFuncType &GetLoopInfo;
  std::function<DominatorTree &(Function &)> *GetDT;
  std::function<PostDominatorTree &(Function &)> *GetPDT;
  WholeProgramInfo *WPInfo;

  // A collection of key Function "roots" over which the analysis proceeds.
  // The 'MainRoot' and 'SubRoot' are Functions which call tile choices,
  // Functions on which tiling will be performed in LoopOpt. 'NewMainRoot'
  // and 'NewSubRoot' are clones of the above Functions that will not be
  // tiled.
  Function *MainRoot = nullptr;
  Function *SubRoot = nullptr;
  Function *NewMainRoot = nullptr;
  Function *NewSubRoot = nullptr;

  // A set of non-tile candidates. These are Functions which have doubly
  // subscripted arrays. We should not mark these for tiling.
  SetVector<Function *> NonTileChoices;

  // A set of tile candidates.  These are Functions which have arrays that
  // are indexed by both "i" and "i+1" or "i-1", where "i" is the loop index.
  // The loops enclosing these arrays are good candidates for tiling. We
  // use the presence of tile candidates as an initial step in determining
  // if we will attempt the transformation.
  SmallPtrSet<Function *, 10> TileCandidates;

  // A set of tile choices. These are the Functions which we will mark for
  // inlining so that their code can be exposed to Loop Opt for tiling.
  SmallSetVector<Function *, 8> TileChoices;

  // A set of Values that represent loop indices in the Function currently
  // being analyzed as a tile candidate.
  SmallPtrSet<Value *, 10> LoopIndices;

  // A set of Values that represent the above loop indices plus or minus one.
  SmallPtrSet<Value *, 10> LoopOffsets;

  // States for a state machine used to determine if a Value is a loop
  // index or loop index plus an offset. The process is more complicated
  // than it would be if the loops were canonicalized. A Value is recognized
  // as a loop index if it:
  //   (1) Appears in a comparison in the loop latch block
  //   (2) Appears in a cycle of Values which are PHIs and an increment
  // To simplify the analysis, we assume that the loop index is always
  // on the left hand side of the loop latch comparison, and that there
  // are no more than two PHIs traversed in the cycle before we get back
  // to the original Value.
  //
  // The states in the state machine are:
  //   TS_Start: Start state. We still need to find the increment and
  //     are allowed to find one additional PHI.
  //   TS_FoundInc: We have found the increment, and still are allowed
  //     to find one additional PHI.
  //   TS_FoundPHI: We have found the additional PHI, and now must find the
  //     the increment.
  //   TS_FoundBoth: We have found the PHI and increment, and the input
  //     must now cycle around to the original value.
  //
  // For example, consider the loop:
  //   9:
  //     %10 = phi i64 [ 3, %6 ], [ %14, %9 ]
  //     %14 = add nuw nsw i64 %10, 1
  //     ...
  //     %22 = icmp eq i64 %14, %8
  //     br i1 %22, label %23, label %9
  // We start in state TS_Start with %14 as the proposed loop index. We then
  // note that the definition of %14 includes the increment value of %10. This
  // moves us to state TS_FoundInc. Looking at the definition of %10, we scan
  // the inputs of the PHI node and see that %14 is one of them. So, we have
  // identified %14 as the loop index and %10 as the incremented value.
  //
  enum TestState { TS_Start, TS_FoundInc, TS_FoundPHI, TS_FoundBoth };

  // Typedef for a triple describing the current Value, the previous
  // Value, and the current state in the loop index recognizing state
  // machine.
  typedef std::tuple<Value *, Value *, TestState> TestStackTuple;

  // A stack of triples being processed through the loop index discovering
  // state machine.
  std::stack<TestStackTuple> TestStack;

  // Return 'true' if 'Arg' is a tile subscript argument. This is an argument
  // representing the address of an array of type "double" which is the pointer
  // operand of a SubscriptInst.
  bool isTileSubscriptArg(Argument &Arg);

  // Return 'true' if 'F' has a unique (only one) tile subscript argument.
  bool hasUniqueTileSubscriptArg(Function &F);

  // Return 'true' if 'F' passes a preliminary parameter test on the way of
  // being qualified as a tile candidate.
  bool passesParameterTest(Function &F);

  // Process a triple in the 'TS_Start' state for the loop index recognition
  // state machine.
  bool processLoopCaseStart(Function &F, TestStackTuple &Item, Value *BOV);

  // Process a triple in the 'TS_FoundInc' state for the loop index recognition
  // state machine.
  bool processLoopCaseFoundInc(Function &F, TestStackTuple &Item, Value *BOV);

  // Process a triple in the 'TS_FoundPHI' state for the loop index recognition
  // state machine.
  bool processLoopCaseFoundPHI(Function &F, TestStackTuple &Item, Value *BOV);

  // Process a triple in the 'TS_FoundBoth' state for the loop index
  // recognition state machine.
  bool processLoopCaseFoundBoth(Function &F, TestStackTuple &Item, Value *BOV);

  // Process 'L' with the intent of finding the loop index and loop index
  // plus or minus offset for 'L' in 'F'.
  bool processLoop(Function &F, Loop &L);

  // Process all of the subloops of 'L' in 'F', with the intent of determining
  // the loop index and loop index plus or minus offset Values for each
  // subloop.
  void processAllSubLoops(Function &F, Loop &L);

  // Return 'true' if 'Arg' is a tile candidate argument.  This is an
  // Argument representing the pointer operand in a SubscriptInst which is
  // indexed by a loop index and the loop index plus or minus an offset.
  bool isTileCandidateArg(Argument &Arg);

  // Return 'true' if 'F' is a tile candidate.
  bool isTileCandidate(Function &F);

  // Return 'true' if 'Arg' is a tile candidate argument.  This is an
  // Argument representing the pointer operand in a SubscriptInst which is
  // indexed by a loop index and the loop index plus or minus an offset.
  bool isNonTileCandidateArg(Argument &Arg);

  // Return 'true' if 'F' is a non-tile candidate.
  bool isNonTileCandidate(Function &F);

  // Return the number of tile candidates. The transformation will only be
  // performed if enough tile candidates are found.
  unsigned identifyTileCandidates(void);

  // Identify 'MainRoot' and 'SubRoot'. Each root is a Function which calls
  // other Functions which are tile choices.  The tile choices are Functions
  // which will be marked for inlining to facilitate tiling in Loop Opt.
  // Return 'true' if the 'MainRoot' and 'SubRoot' are identified.
  bool identifyTileRoots(void);

  // Create a initial set of tile choices.
  void makeTileChoices(Function *Root, Function *SubRoot);

  // Refine the initial set of tile choices by removing some of them for the
  // set.
  void siftTileChoices(Function *F);

  // Find the non-tile choices. These are functions that should not be tiled
  // and should not be called in the high performace version of the code.
  void makeNonTileChoices(Function &Root);

  // A mapping of GlobalVariables which appear in the 'Root' Function
  // to Values in which those GlobalVariables appear and which control the
  // IR where the bulk of the tiling computations occur.
  MapVector<GlobalVariable *, Value *> GVM;

  // A map from the Values in 'GVM' to either 'true' or 'false', depending
  // on whether the 'true' or 'false' sense of the Value controls the tile
  // computations.
  MapVector<Value *, bool> CM;

  // Similar to the above map, but contains extra Values for which its
  // GlobalVariable already appears in the GVM.
  MapVector<Value *, bool> CMExtra;

  // Compute the maps 'GVM' and 'CM', which are used to compute the multi-
  // versioning expression surrounding the the key computation IR for tiling.
  void findGVMandCM();

  // Return 'true' if none of the GlobalVariables in 'GVM' are modified by
  // 'MainRoot'.
  bool validateGVM();

  // Return the multiversioning condition between the high performance and
  // default version of the code. This condition will be placed in 'CondBB'.
  Value *makeConditionFromGlobals(BasicBlock *CondBB);

  // Transform the call to the 'MainRoot' function by conditioning it on the
  // multiversioning test. A copy of 'MainRoot' is cloned, which is used for
  // the default version of 'MainRoot' and a copy of 'SubRoot' is cloned which
  // is called from the clone of 'MainRoot'.
  void cloneCallToRoot();

  // Simplify the conditionals in 'MainRoot' using the 'CM' and 'CMExtra' maps.
  void simplifyConditionalsInRoot();

  // Mark the calls to the TileChoices in the high performance version of
  // the code.
  void markTileChoicesForInlining();

#ifndef NDEBUG
  // Dump out a message indicating that 'LI' is a loop index and 'LO' is that
  // loop index plus or minus and offset in 'F'.
  void dumpLoopIndexPair(Function &F, Value *LI, Value *LO);

  // Dump out the tile choices.
  void dumpTileChoices(void);

  // Dump out the non-tile candidates.
  void dumpNonTileChoices(void);

  // Dump out the global variable map 'GVM'.
  void dumpGVM(void);

  // Dump out the condition map 'CM'.
  void dumpCM(void);

  // Dump the names of the key root Functions and the Functions they call
  // with IR.
  void dumpKeyFunctionNamesAndCalls();
#endif // NDEBUG
};

bool TileMVInlMarker::isTileSubscriptArg(Argument &Arg) {
  Type *Ty = Arg.getType();
  if (!Ty->isPointerTy() || !Ty->getPointerElementType()->isDoubleTy())
    return false;
  for (User *U : Arg.users()) {
    auto SI = dyn_cast<SubscriptInst>(U);
    if (SI && SI->getPointerOperand() == &Arg)
      return true;
  }
  return false;
}

bool TileMVInlMarker::hasUniqueTileSubscriptArg(Function &F) {
  bool TileSubscriptArgSeen = false;
  for (auto &Arg : F.args()) {
    if (isTileSubscriptArg(Arg)) {
      if (TileSubscriptArgSeen)
        return false;
      TileSubscriptArgSeen = true;
    }
  }
  return TileSubscriptArgSeen;
}

bool TileMVInlMarker::passesParameterTest(Function &F) {
  unsigned Count = 0;
  for (auto &Arg : F.args())
    if (isTileSubscriptArg(Arg))
      if (++Count >= TileCandidateSubArgMin)
        return true;
  return false;
}

#ifndef NDEBUG
void TileMVInlMarker::dumpLoopIndexPair(Function &F, Value *LI, Value *LO) {
  LLVM_DEBUG({
    dbgs() << "TMVINL: " << F.getName() << " Loop Index ";
    LI->dump();
    dbgs() << "TMVINL: " << F.getName() << " Loop Inc ";
    LO->dump();
  });
}
#endif // NDEBUG

bool TileMVInlMarker::processLoopCaseStart(Function &F, TestStackTuple &Item,
                                           Value *BOV) {
  Value *X = nullptr;
  if (match(std::get<0>(Item), m_Add(m_Value(X), m_One()))) {
    auto PHI0 = dyn_cast<PHINode>(X);
    if (PHI0) {
      TestStack.push(std::make_tuple(PHI0, std::get<0>(Item), TS_FoundInc));
      return false;
    }
  }
  auto PHIN = dyn_cast<PHINode>(std::get<0>(Item));
  if (!PHIN)
    return false;
  for (unsigned I = 0, E = PHIN->getNumIncomingValues(); I < E; ++I) {
    Value *TV = PHIN->getIncomingValue(I);
    auto PHI0 = dyn_cast<PHINode>(TV);
    if (PHI0 && PHI0 != BOV) {
      TestStack.push(std::make_tuple(PHI0, nullptr, TS_FoundPHI));
      continue;
    }
    Value *Y = nullptr;
    if (match(TV, m_Add(m_Value(Y), m_One()))) {
      if (Y == BOV) {
        LLVM_DEBUG(dumpLoopIndexPair(F, BOV, TV));
        LoopIndices.insert(BOV);
        LoopOffsets.insert(TV);
        return true;
      }
      auto PHI1 = dyn_cast<PHINode>(Y);
      if (PHI1) {
        TestStack.push(std::make_tuple(PHI1, TV, TS_FoundBoth));
        continue;
      }
    }
  }
  return false;
}

bool TileMVInlMarker::processLoopCaseFoundInc(Function &F, TestStackTuple &Item,
                                              Value *BOV) {
  auto PHIN = cast<PHINode>(std::get<0>(Item));
  for (unsigned I = 0, E = PHIN->getNumIncomingValues(); I < E; ++I) {
    Value *TV = PHIN->getIncomingValue(I);
    if (TV == BOV) {
      LLVM_DEBUG(dumpLoopIndexPair(F, BOV, PHIN));
      LoopIndices.insert(BOV);
      LoopOffsets.insert(PHIN);
      return true;
    }
    auto PHI0 = dyn_cast<PHINode>(TV);
    if (PHI0) {
      TestStack.push(std::make_tuple(PHI0, std::get<1>(Item), TS_FoundBoth));
      continue;
    }
  }
  return false;
}

bool TileMVInlMarker::processLoopCaseFoundPHI(Function &F, TestStackTuple &Item,
                                              Value *BOV) {
  Value *Z = nullptr;
  if (!match(std::get<0>(Item), m_Add(m_Value(Z), m_One())) || Z != BOV)
    return false;
  LLVM_DEBUG(dumpLoopIndexPair(F, BOV, std::get<0>(Item)));
  LoopIndices.insert(BOV);
  LoopOffsets.insert(std::get<0>(Item));
  return true;
}

bool TileMVInlMarker::processLoopCaseFoundBoth(Function &F,
                                               TestStackTuple &Item,
                                               Value *BOV) {
  auto PHIN = cast<PHINode>(std::get<0>(Item));
  for (unsigned I = 0, E = PHIN->getNumIncomingValues(); I < E; ++I) {
    Value *TV = PHIN->getIncomingValue(I);
    if (TV != BOV)
      continue;
    LLVM_DEBUG(dumpLoopIndexPair(F, BOV, PHIN));
    LoopIndices.insert(BOV);
    LoopOffsets.insert(PHIN);
    return true;
  }
  return false;
}

bool TileMVInlMarker::processLoop(Function &F, Loop &L) {
  // Check the left side of a branch conditional for a loop index
  BasicBlock *BB = L.getLoopLatch();
  if (!BB)
    return false;
  auto BI = dyn_cast<BranchInst>(BB->getTerminator());
  if (!BI || BI->isUnconditional())
    return false;
  auto IC = dyn_cast<ICmpInst>(BI->getCondition());
  if (!IC)
    return false;
  switch (IC->getPredicate()) {
  case ICmpInst::ICMP_SLT:
  case ICmpInst::ICMP_SLE:
  case ICmpInst::ICMP_EQ:
    break;
  default:
    return false;
  }
  Value *BOV = IC->getOperand(0);
  // Load the state machine with the initial value.
  TestStack.push(std::make_tuple(BOV, nullptr, TS_Start));
  // Attempt to find the loop index and pre-index value.
  while (1) {
    if (TestStack.empty())
      return false;
    auto Item = TestStack.top();
    TestStack.pop();
    switch (std::get<2>(Item)) {
    case TS_Start:
      if (processLoopCaseStart(F, Item, BOV))
        return true;
      break;
    case TS_FoundInc:
      if (processLoopCaseFoundInc(F, Item, BOV))
        return true;
      break;
    case TS_FoundPHI:
      return processLoopCaseFoundPHI(F, Item, BOV);
    case TS_FoundBoth:
      return processLoopCaseFoundBoth(F, Item, BOV);
    default:
      assert(false && "No default case");
    }
  }
  return false;
}

void TileMVInlMarker::processAllSubLoops(Function &F, Loop &L) {
  for (auto LS = L.begin(), LF = L.end(); LS != LF; LS++) {
    processLoop(F, **LS);
    processAllSubLoops(F, **LS);
  }
}

bool TileMVInlMarker::isTileCandidateArg(Argument &Arg) {
  unsigned ZeroCount = 0;
  unsigned OffsetCount = 0;
  for (User *U : Arg.users()) {
    auto SI = dyn_cast<SubscriptInst>(U);
    if (!SI || SI->getPointerOperand() != &Arg)
      continue;
    Value *X = nullptr;
    ConstantInt *Y = nullptr;
    if (LoopIndices.count(SI->getIndex())) {
      ZeroCount++;
    } else if (LoopOffsets.count(SI->getIndex())) {
      OffsetCount++;
    } else if (match(SI->getIndex(), m_Add(m_Value(X), m_ConstantInt(Y)))) {
      if (LoopIndices.count(X)) {
        if (Y->isOne() || Y->isMinusOne())
          OffsetCount++;
      } else if (LoopOffsets.count(X)) {
        if (Y->isOne() || Y->isMinusOne())
          ZeroCount++;
      }
    }
  }
  LLVM_DEBUG(dbgs() << "TMVINL: " << Arg.getParent()->getName() << " Arg %"
                    << Arg.getArgNo() << "(" << ZeroCount << "," << OffsetCount
                    << ")\n");
  return ZeroCount && OffsetCount;
}

bool TileMVInlMarker::isNonTileCandidateArg(Argument &Arg) {
  for (User *U : Arg.users()) {
    auto SI = dyn_cast<SubscriptInst>(U);
    if (!SI || SI->getPointerOperand() != &Arg)
      continue;
    Value *V = SI->getIndex();
    auto W = dyn_cast<SExtInst>(V);
    if (W)
      V = W->getOperand(0);
    auto LI = dyn_cast<LoadInst>(V);
    if (!LI)
      continue;
    if (isa<SubscriptInst>(LI->getPointerOperand()))
      return true;
  }
  return false;
}

bool TileMVInlMarker::isNonTileCandidate(Function &F) {
  for (auto &Arg : F.args()) {
    if (isNonTileCandidateArg(Arg)) {
      LLVM_DEBUG(dbgs() << "TMVINL: Non Tile Candidate " << F.getName()
                        << "\n");
      return true;
    }
  }
  return false;
}

bool TileMVInlMarker::isTileCandidate(Function &F) {
  if (!passesParameterTest(F))
    return false;
  LoopIndices.clear();
  LoopOffsets.clear();
  LoopInfo &LI = (GetLoopInfo)(F);
  for (auto LS = LI.begin(), LF = LI.end(); LS != LF; LS++) {
    processLoop(F, **LS);
    processAllSubLoops(F, **LS);
  }
  for (auto &Arg : F.args()) {
    if (isTileCandidateArg(Arg)) {
      LLVM_DEBUG(dbgs() << "TMVINL: Tile Candidate " << F.getName() << "\n");
      return true;
    }
  }
  return false;
}

void TileMVInlMarker::makeNonTileChoices(Function &Root) {
  for (auto &F : M.functions()) {
    Function *Caller = uniqueCaller(F);
    if (Caller != &Root)
      continue;
    if (isNonTileCandidate(F))
      NonTileChoices.insert(&F);
  }
}

unsigned TileMVInlMarker::identifyTileCandidates(void) {
  for (auto &F : M.functions())
    if (isTileCandidate(F))
      TileCandidates.insert(&F);
  return TileCandidates.size();
}

bool TileMVInlMarker::identifyTileRoots(void) {
  DenseMap<Function *, unsigned> Callers;
  Function *CallerMax = nullptr;
  unsigned ValueMax = 0;
  for (auto *F : TileCandidates) {
    Function *Caller = uniqueCaller(*F);
    if (!Caller)
      return false;
    Callers[Caller] = Callers[Caller] + 1;
    if (Callers[Caller] > ValueMax) {
      CallerMax = Caller;
      ValueMax = Callers[Caller];
    }
  }
  if (!CallerMax)
    return false;
  Function *Main = uniqueCaller(*CallerMax);
  if (!Main)
    return false;
  if (!WPUtils.isMainEntryPoint(Main->getName()))
    return false;
  Function *CallerMaxSub = nullptr;
  for (auto *F : TileCandidates) {
    Function *Caller = uniqueCaller(*F);
    assert(Caller != nullptr && "Expecting unique caller");
    if (Caller == CallerMax)
      continue;
    if (CallerMaxSub && (Caller != CallerMaxSub))
      return false;
    CallerMaxSub = Caller;
    Caller = uniqueCaller(*Caller);
    if (Caller == CallerMax)
      continue;
    return false;
  }
  if (!CallerMaxSub)
    return false;
  MainRoot = CallerMax;
  SubRoot = CallerMaxSub;
  return true;
}

void TileMVInlMarker::makeTileChoices(Function *Root, Function *SubRoot) {
  for (auto &I : instructions(Root)) {
    auto CB = dyn_cast<CallBase>(&I);
    if (!CB)
      continue;
    Function *Callee = CB->getCalledFunction();
    if (!Callee || Callee->isDeclaration())
      continue;
    if (Callee != SubRoot &&
        (hasUniqueTileSubscriptArg(*Callee) ||
         std::distance(Callee->arg_begin(), Callee->arg_end()) >=
                 TileCandidateArgMin &&
             IPOUtils::isLeafFunction(*Callee))) {
      TileChoices.insert(Callee);
    }
  }
}

//
// If 'BB' starts with a call to a Function, return a pointer to that Function.
//
static Function *getTargetCall(BasicBlock *BB) {
  if (BB->empty())
    return nullptr;
  BasicBlock::iterator I = BB->begin();
  while (isa<DbgInfoIntrinsic>(I))
    ++I;
  auto CB = dyn_cast<CallBase>(&*I);
  if (!CB)
    return nullptr;
  return CB->getCalledFunction();
}

void TileMVInlMarker::siftTileChoices(Function *F) {
  for (auto &BB : *F) {
    auto BI = dyn_cast<BranchInst>(BB.getTerminator());
    if (!BI || BI->isUnconditional())
      continue;
    auto LI = dyn_cast<LoadInst>(BI->getCondition());
    if (!LI || !isa<GlobalVariable>(LI->getPointerOperand()))
      continue;
    Function *TrueF = getTargetCall(BI->getSuccessor(0));
    if (!TrueF || !TileChoices.count(TrueF))
      continue;
    Function *FalseF = getTargetCall(BI->getSuccessor(1));
    if (!FalseF || !TileChoices.count(FalseF))
      continue;
    TileChoices.remove(FalseF);
  }
}

#ifndef NDEBUG
void TileMVInlMarker::dumpTileChoices(void) {
  for (auto *F : TileChoices)
    dbgs() << "TMVINL: Tile Choice " << F->getName() << "\n";
}

void TileMVInlMarker::dumpNonTileChoices(void) {
  for (auto *F : NonTileChoices)
    dbgs() << "TMVINL: Non Tile Choice " << F->getName() << "\n";
}

void TileMVInlMarker::dumpGVM(void) {
  for (auto &Pair : GVM) {
    dbgs() << "TMVINL: GVMAP " << Pair.first->getName() << "\n";
    dbgs() << "TMVINL: ";
    Pair.second->dump();
  }
}

void TileMVInlMarker::dumpCM(void) {
  for (auto &Pair : CM) {
    dbgs() << "TMVINL: CONDMAP " << (Pair.second ? "T " : "F ");
    Value *V = Pair.first;
    V->dump();
    auto BO = dyn_cast<ICmpInst>(V);
    if (BO) {
      auto LI0 = dyn_cast<LoadInst>(BO->getOperand(0));
      if (LI0) {
        dbgs() << "TMVINL: LI ";
        LI0->dump();
      }
      auto LI1 = dyn_cast<LoadInst>(BO->getOperand(1));
      if (LI1) {
        dbgs() << "TMVINL: LI ";
        LI1->dump();
      }
    }
  }
}

void TileMVInlMarker::dumpKeyFunctionNamesAndCalls() {
  //
  // Dump the name of a Function and the Functions it calls that have IR.
  //
  auto dumpFunctionNameAndCalls = [](const char Title[], Function *F) {
    if (!F)
      return;
    dbgs() << "TMVINL: " << Title << " " << F->getName() << "\n";
    for (auto &I : instructions(F)) {
      auto CB = dyn_cast<CallBase>(&I);
      if (CB && !isa<IntrinsicInst>(CB)) {
        Function *Callee = CB->getCalledFunction();
        if (Callee && !Callee->isDeclaration()) {
          dbgs() << "TMVINL:  ";
          if (CB->hasFnAttr("prefer-inline-tile-choice"))
            dbgs() << "T ";
          dbgs() << Callee->getName() << "\n";
        }
      }
    }
  };

  dumpFunctionNameAndCalls("Root:", MainRoot);
  dumpFunctionNameAndCalls("SubRoot:", SubRoot);
  dumpFunctionNameAndCalls("NewRoot:", NewMainRoot);
  dumpFunctionNameAndCalls("NewSubRoot:", NewSubRoot);
}

#endif // NDEBUG

void TileMVInlMarker::findGVMandCM() {

  //
  // Return 'true' if 'GV' should be inserted into 'GVM' with Value 'V' and 'V'
  // should be inserted into 'CM' with value 'Sense'. 'LeftLoad' is 'true' if
  // 'V' is an ICmpInst with a 'LoadInst' as operand(0). Also, set 'FoundMatch'
  // if we will do the insertion or if we found an entry in 'GV' and 'CM' that
  // already covers the value we are testing.
  //
  // NOTE: In order to keep the implementation simple, we only allow one Value
  // for each GlobalVariable in 'GVM'. Testing for this condition is done after
  // the call to 'ShouldInsert'.  This requirement could be relaxed if it is
  // found useful to do so.
  //
  auto ShouldInsert = [this](GlobalVariable *GV, Value *V, bool Sense,
                             bool LeftLoad, bool &FoundMatch) -> bool {
    FoundMatch = false;
    if (!GVM.count(GV))
      return true;
    // Get the constant and predicate for the new value.
    Value *OV = GVM[GV];
    bool OSense = CM[OV];
    bool NSense = Sense;
    ICmpInst::Predicate P = ICmpInst::ICMP_EQ;
    ConstantInt *CI = nullptr;
    auto LI = dyn_cast<LoadInst>(V);
    if (LI) {
      CI = ConstantInt::get(Type::getInt32Ty(GV->getContext()), 0);
    } else {
      auto IC = cast<ICmpInst>(V);
      P = IC->getPredicate();
      if (LeftLoad) {
        LI = cast<LoadInst>(IC->getOperand(0));
        CI = cast<ConstantInt>(IC->getOperand(1));
      } else {
        LI = cast<LoadInst>(IC->getOperand(1));
        CI = cast<ConstantInt>(IC->getOperand(0));
        NSense = !NSense;
      }
    }
    // Canonicalize the new value to match the sense of the old value.
    if (OSense != NSense)
      P = ICmpInst::getInversePredicate(P);
    // Get the constant and predicate for the old value.
    ConstantInt *OCI = nullptr;
    ICmpInst::Predicate OP = ICmpInst::ICMP_EQ;
    if (auto OLI = dyn_cast<LoadInst>(OV)) {
      OCI = ConstantInt::get(Type::getInt32Ty(GV->getContext()), 0);
    } else {
      auto OIC = cast<ICmpInst>(OV);
      OLI = dyn_cast<LoadInst>(OIC->getOperand(0));
      if (OLI) {
        OCI = cast<ConstantInt>(OIC->getOperand(1));
        OP = OIC->getPredicate();
      } else {
        OLI = cast<LoadInst>(OIC->getOperand(1));
        OCI = cast<ConstantInt>(OIC->getOperand(0));
        OP = OIC->getInversePredicate();
      }
    }
    // Insert if the constants don't match.
    if (CI != OCI)
      return true;
    // If the predicates match, no need to update.
    if (OP == P) {
      FoundMatch = true;
      return false;
    }
    // Determine if old condition is stricter that the new condition. If so,
    // there is no need to update.
    switch (P) {
    case ICmpInst::ICMP_SLE:
      if (OP == ICmpInst::ICMP_SLT || OP == ICmpInst::ICMP_EQ) {
        FoundMatch = true;
        return false;
      }
      break;
    case ICmpInst::ICMP_SGE:
      if (OP == ICmpInst::ICMP_SGT || OP == ICmpInst::ICMP_EQ) {
        FoundMatch = true;
        return false;
      }
      break;
    case ICmpInst::ICMP_ULE:
      if (OP == ICmpInst::ICMP_ULT || OP == ICmpInst::ICMP_EQ) {
        FoundMatch = true;
        return false;
      }
      break;
    case ICmpInst::ICMP_UGE:
      if (OP == ICmpInst::ICMP_UGT || OP == ICmpInst::ICMP_EQ) {
        FoundMatch = true;
        return false;
      }
      break;
    default:
      return true;
    }
    return true;
  };

  //
  // Return 'true' if the condition 'V' guards a 'BB' which contains a call to
  // one of the 'TileChoices' or 'NonTileChoices'. Also, update 'GVM' and 'CM'
  // as appropriate.  'GV' is the sole GlobalVariable in 'V'. 'Sense' is true
  // if we are testing if the guarding happens when 'V' is 'true'. 'LeftLoad'
  // is 'true' if 'V' is an ICmpInst with a LoadInst of 'GV' as operand(0).
  //
  auto TestAndAdd = [this, &ShouldInsert](BasicBlock *BB, GlobalVariable *GV,
                                          Value *V, bool Sense,
                                          bool LeftLoad) -> bool {
    bool FoundMatch = false;
    Function *F = getTargetCall(BB);
    if (TileChoices.count(F)) {
      if (ShouldInsert(GV, V, Sense, LeftLoad, FoundMatch)) {
        if (GVM.count(GV)) {
          CMExtra[V] = Sense;
        } else {
          GVM[GV] = V;
          CM[V] = Sense;
        }
      }
      if (FoundMatch)
        return true;
    }
    if (NonTileChoices.count(F)) {
      if (ShouldInsert(GV, V, !Sense, LeftLoad, FoundMatch)) {
        if (GVM.count(GV)) {
          CMExtra[V] = !Sense;
        } else {
          GVM[GV] = V;
          CM[V] = !Sense;
        }
      }
      if (FoundMatch)
        return true;
    }
    return false;
  };

  //
  // Return 'true' if either the true or false branch of 'BI', which has a
  // condition 'V' containing 'GV' guards a 'TileChoice' or 'NonTileChoice'.
  // Update 'GVM' and 'CM' as appropriate. If 'TrueOnly', only test the true
  // branch. If 'LeftLoad' is true, 'V' is an ICmpInst with a LoadInst of 'GV'
  // as operand(0).
  //
  auto TestAndAddBoth = [&TestAndAdd](BranchInst *BI, GlobalVariable *GV,
                                      Value *V, bool TrueOnly,
                                      bool LeftLoad) -> bool {
    if (TestAndAdd(BI->getSuccessor(0), GV, V, true, LeftLoad))
      return true;
    if (!TrueOnly && TestAndAdd(BI->getSuccessor(1), GV, V, false, LeftLoad))
      return true;
    return false;
  };

  //
  // Return 'true' if 'LIC' is a LoadInst contained in 'VC', a condition used
  // in 'BI', and 'BI' guards a 'TileChoice' or 'NonTileChoice'. Update 'GVM'
  // and 'CM' as appropriate. If 'TrueOnly', only test the true branch. If
  // 'LeftLoad' is true, 'V' is an ICmpInst with a LoadInst of 'GV' as
  // operand(0).
  //
  auto TestLoad = [&TestAndAddBoth](BranchInst *BI, Value *LIC, Value *VC,
                                    bool TrueOnly, bool LeftLoad) -> bool {
    auto LI = dyn_cast<LoadInst>(LIC);
    if (!LI)
      return false;
    auto GV = dyn_cast<GlobalVariable>(LI->getPointerOperand());
    if (!GV)
      return false;
    TestAndAddBoth(BI, GV, VC, TrueOnly, LeftLoad);
    return true;
  };

  //
  // Return 'true' if the condition 'VC' in 'BI' guards a 'TileChoice' or
  // 'NonTileChoice'. If 'TrueOnly', only test the true branch. Update 'GVM'
  // and 'CM' as appropriate.
  //
  auto TestCondition = [&TestLoad](BranchInst *BI, Value *VC,
                                   bool TrueOnly) -> bool {
    if (TestLoad(BI, VC, VC, TrueOnly, false))
      return true;
    auto BO = dyn_cast<ICmpInst>(VC);
    if (!BO)
      return false;
    if (isa<Constant>(BO->getOperand(1)) &&
        TestLoad(BI, BO->getOperand(0), VC, TrueOnly, true))
      return true;
    if (isa<Constant>(BO->getOperand(0)) &&
        TestLoad(BI, BO->getOperand(1), VC, TrueOnly, false))
      return true;
    return false;
  };

  //
  // Return 'true' if 'BB' contains a call to a key FortranIO function. For
  // now, we consider 'open', 'close', and 'stop'. The idea is that a
  // BasicBlock containing such a call is not on the hot path of the execution,
  // and should be excluded from the specialized version that is optimized
  // with tiling.
  //
  auto HasKeyFortranIOFunction = [](BasicBlock &BB) -> bool {
    for (auto &I : BB) {
      auto CB = dyn_cast<CallBase>(&I);
      if (!CB)
        continue;
      Function *F = CB->getCalledFunction();
      if (!F)
        continue;
      StringRef FN = F->getName();
      if (FN == "for_open" || FN == "for_close" || FN == "for_stop_core_quiet")
        return true;
    }
    return false;
  };

  //
  // Return 'true' if the condition 'VC', containing the LoadInst 'VC' with
  // the given 'Sense' will refine the set of conditions in 'GVM' and 'CM'.
  // If so, update 'GVM' and 'CM'.
  //
  auto TestLoadWithSuccessor = [this, &ShouldInsert](Value *LIC, Value *VC,
                                                     bool Sense,
                                                     bool LeftLoad) -> bool {
    bool FoundMatch = false;
    auto LI = dyn_cast<LoadInst>(LIC);
    if (!LI)
      return false;
    auto GV = dyn_cast<GlobalVariable>(LI->getPointerOperand());
    if (!GV)
      return false;
    if (ShouldInsert(GV, VC, !Sense, LeftLoad, FoundMatch)) {
      if (GVM.count(GV)) {
        CMExtra[VC] = !Sense;
      } else {
        GVM[GV] = VC;
        CM[VC] = !Sense;
      }
    }
    if (FoundMatch)
      return true;
    return false;
  };

  //
  // Return 'true' if the condition 'VC' with the given 'Sense' will refine
  // the set of conditions in 'GVM' and 'CM'. If so, update 'GVM' and 'CM'.
  //
  auto TestConditionWithSuccessor =
      [&TestLoadWithSuccessor](Value *VC, bool Sense) -> bool {
    if (TestLoadWithSuccessor(VC, VC, Sense, false))
      return true;
    auto BO = dyn_cast<ICmpInst>(VC);
    if (!BO)
      return false;
    if (isa<Constant>(BO->getOperand(1)) &&
        TestLoadWithSuccessor(BO->getOperand(0), VC, Sense, true))
      return true;
    if (isa<Constant>(BO->getOperand(0)) &&
        TestLoadWithSuccessor(BO->getOperand(1), VC, Sense, false))
      return true;
    return false;
  };

  //
  // Main code for findGVMandCM
  //

  //
  // Find the 'CallBlocks' which contain calls to Key Fortran IO functions
  // that we want to exclude from the version of the code optimizied for
  // tiling. While doing that, add to 'GVM' and 'CM' GlobalVariables and
  // Values that will guard the 'TileChoices' and 'NonTileChoices'.
  //
  SmallPtrSet<BasicBlock *, 10> CallBlocks;
  for (auto &BB : *MainRoot) {
    if (HasKeyFortranIOFunction(BB))
      CallBlocks.insert(&BB);
    auto BI = dyn_cast<BranchInst>(BB.getTerminator());
    if (!BI || BI->isUnconditional())
      continue;
    Value *VC = BI->getCondition();
    Value *V1 = nullptr;
    Value *V2 = nullptr;
    auto BO = dyn_cast<BinaryOperator>(VC);
    if (BO && BO->getOpcode() == Instruction::And &&
        match(VC, m_BinOp(m_Value(V1), m_Value(V2)))) {
      TestCondition(BI, V1, true);
      TestCondition(BI, V2, true);
    } else {
      TestCondition(BI, VC, false);
    }
  }

  //
  // Walk over the CallBlocks and derive conditions that will guard the
  // 'CallBlocks', updating 'GVM' and 'CM'. Use the DominatorTree to find
  // candidate BasicBlocks terminating in tests that can serve as suitable
  // guard tests. Use the PostdominatorTree to exclude unsuitable candidate
  // BasicBlocks.  For example:
  //     BB0: if C goto BB1 else goto BB2;
  //     BB1: <code> goto BB3;
  //     BB2: <code> goto BB4;
  //     BB4:
  // It might be thought that the 'true' test in BB0 is a good guard for
  // 'BB4', since 'BB0' dominates 'BB4'.  But 'BB2' postdominates 'BB4',
  // showing that this is a bad choice.
  //
  DominatorTree &DT = (*GetDT)(*MainRoot);
  PostDominatorTree &PDT = (*GetPDT)(*MainRoot);
  for (auto *BB : CallBlocks) {
    DomTreeNode *DTN = DT.getNode(BB);
    if (!DTN)
      continue;
    BasicBlock *BP = BB;
    for (DTN = DTN->getIDom(); DTN; DTN = DTN->getIDom()) {
      BasicBlock *BT = DTN->getBlock();
      auto BI = dyn_cast<BranchInst>(BT->getTerminator());
      if (!BI || BI->isUnconditional() || BI->getNumSuccessors() != 2)
        continue;
      Value *VC = BI->getCondition();
      BasicBlock *BBS0 = BI->getSuccessor(0);
      BasicBlock *BBS1 = BI->getSuccessor(1);
      if (BBS0 == BP && !PDT.dominates(BP, BBS1)) {
        if (TestConditionWithSuccessor(VC, true))
          break;
      } else if (BBS1 == BP && !PDT.dominates(BP, BBS0)) {
        if (TestConditionWithSuccessor(VC, false))
          break;
      }
      BP = BT;
    }
  }
}

bool TileMVInlMarker::validateGVM() {

  //
  // If the user 'U' of 'BC' appears in a canonical, self-contained Fortran
  // read of a GlobalVariable, return the StoreInst that references that
  // GlobalVariable.
  //
  // Here is an example of the sequence of Instructions that this function is
  // intended to match:
  //
  // %126 = alloca { i8* }, align 8
  // %690 = getelementptr inbounds { i8* }, { i8* }* %126, i64 0, i32 0
  // store i8* bitcast (i32* @globalvar_mod_mp_pulse_ to i8*), i8** %690,
  //    align 8
  // %691 = bitcast { i8* }* %126 to i8*
  // %692 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_read_seq_lis(
  //    i8* nonnull %1333, i32 9, i64 1239157112576, i8* nonnull %686,
  //    i8* nonnull %691)
  //
  auto IsCanonicalFortranRead = [](BitCastOperator *BC,
                                   User *U) -> StoreInst * {
    auto SI = dyn_cast<StoreInst>(U);
    if (!SI || SI->getValueOperand() != BC)
      return nullptr;
    auto GEPI = dyn_cast<GetElementPtrInst>(SI->getPointerOperand());
    if (!GEPI || !GEPI->hasAllZeroIndices())
      return nullptr;
    auto AI = dyn_cast<AllocaInst>(GEPI->getPointerOperand());
    if (!AI)
      return nullptr;
    unsigned UserCount = 0;
    for (auto *V : AI->users()) {
      if (UserCount > 2)
        return nullptr;
      if (V == GEPI) {
        ++UserCount;
        continue;
      }
      auto BC = dyn_cast<BitCastInst>(V);
      if (BC) {
        if (!BC->hasOneUse())
          return nullptr;
        auto CI = dyn_cast<CallInst>(BC->user_back());
        if (!CI)
          return nullptr;
        Function *Callee = CI->getCalledFunction();
        if (!Callee || !Callee->isDeclaration() ||
            Callee->getName() != "for_read_seq_lis" || Callee->arg_size() < 5 ||
            CI->getArgOperand(4) != BC)
          return nullptr;
        ++UserCount;
        continue;
      }
      return nullptr;
    }
    return UserCount == 2 ? SI : nullptr;
  };

  //
  // Fill 'AS' with pointers to the Functions which assign the GlobalVariables
  // contained in 'GVM'. Return 'true' if 'AS' contains a complete set, 'false'
  // if we cannot determine the complete set.
  //
  auto FindASF =
      [this, &IsCanonicalFortranRead](SmallPtrSetImpl<Function *> &AS) -> bool {
    for (auto &Pair : GVM) {
      GlobalVariable *GV = Pair.first;
      for (auto *V : GV->users()) {
        if (isa<LoadInst>(V))
          continue;
        if (auto SI = dyn_cast<StoreInst>(V)) {
          if (SI->getPointerOperand() != GV)
            return false;
          AS.insert(SI->getFunction());
        } else if (auto BC = dyn_cast<BitCastOperator>(V)) {
          for (User *U : BC->users()) {
            StoreInst *SI = IsCanonicalFortranRead(BC, U);
            if (!SI)
              return false;
            AS.insert(SI->getFunction());
          }
        } else {
          return false;
        }
      }
    }
    return true;
  };

  //
  // Return 'true' if tracing a path from 'F' through any sequence of callers
  // never encounters 'Root'.
  //
  auto ToMainBeforeRoot = [](Function &F, Function &Root) -> bool {
    SmallPtrSet<Function *, 10> Visited;
    SmallVector<Function *, 10> Worklist;
    Worklist.push_back(&Root);
    while (!Worklist.empty()) {
      Function *G = Worklist.pop_back_val();
      Visited.insert(G);
      for (User *U : G->users()) {
        auto CB = dyn_cast<CallBase>(U);
        if (!CB || CB->getCalledFunction() != G)
          return false;
        Function *Caller = CB->getCaller();
        if (Caller == &Root)
          return false;
        if (!Visited.count(Caller))
          Worklist.push_back(Caller);
      }
    }
    return true;
  };

  //
  // Main code for TileMVInlMarker::validateGVM.
  //
  SmallPtrSet<Function *, 10> AS;
  if (!FindASF(AS))
    return false;
  for (auto *F : AS)
    if (!ToMainBeforeRoot(*F, *MainRoot))
      return false;
  return true;
}

Value *TileMVInlMarker::makeConditionFromGlobals(BasicBlock *CondBB) {
  Value *TAnd = nullptr;
  for (auto &Pair : CM) {
    Value *V = Pair.first;
    bool Sense = Pair.second;
    auto LI = dyn_cast<LoadInst>(V);
    if (LI) {
      LoadInst *NewLI = cast<LoadInst>(LI->clone());
      CondBB->getInstList().push_back(NewLI);
      Constant *CZ = ConstantInt::get(LI->getType(), 0);
      CmpInst *CI = ICmpInst::Create(
          Instruction::ICmp, Sense ? ICmpInst::ICMP_NE : ICmpInst::ICMP_EQ,
          NewLI, CZ, "clone.tile.cmp", CondBB);
      CI->setDebugLoc(LI->getDebugLoc());
      if (TAnd) {
        BinaryOperator *BO =
            BinaryOperator::CreateAnd(TAnd, CI, ".clone.tile.and", CondBB);
        BO->setDebugLoc(LI->getDebugLoc());
      } else {
        TAnd = cast<Value>(CI);
      }
      continue;
    }
    auto IC = dyn_cast<ICmpInst>(V);
    if (IC) {
      auto LLI = dyn_cast<LoadInst>(IC->getOperand(0));
      if (LLI) {
        LoadInst *NewLLI = cast<LoadInst>(LLI->clone());
        CondBB->getInstList().push_back(NewLLI);
        CmpInst *CI = ICmpInst::Create(
            Instruction::ICmp,
            Sense ? IC->getPredicate() : IC->getInversePredicate(), NewLLI,
            IC->getOperand(1), "clone.tile.cmp", CondBB);
        CI->setDebugLoc(LLI->getDebugLoc());
        if (TAnd) {
          BinaryOperator *BO =
              BinaryOperator::CreateAnd(TAnd, CI, ".clone.tile.and", CondBB);
          BO->setDebugLoc(LLI->getDebugLoc());
        } else {
          TAnd = cast<Value>(CI);
        }
        continue;
      }
      auto LRI = dyn_cast<LoadInst>(IC->getOperand(1));
      if (LRI) {
        LoadInst *NewLRI = cast<LoadInst>(LRI->clone());
        CondBB->getInstList().push_back(NewLRI);
        CmpInst *CI = ICmpInst::Create(
            Instruction::ICmp,
            Sense ? IC->getPredicate() : IC->getInversePredicate(),
            IC->getOperand(0), NewLRI, "clone.tile.cmp", CondBB);
        CI->setDebugLoc(LRI->getDebugLoc());
        if (TAnd) {
          BinaryOperator *BO =
              BinaryOperator::CreateAnd(TAnd, CI, ".clone.tile.and", CondBB);
          BO->setDebugLoc(LRI->getDebugLoc());
        } else {
          TAnd = cast<Value>(CI);
        }
        continue;
      }
      assert(false && "Expecting LoadInst as operand");
    }
    assert(false && "Expecting LoadInst or ICmpInst");
  }
  return TAnd;
}

void TileMVInlMarker::cloneCallToRoot() {
  //
  // Redirect the call from 'NewRoot' to 'SubRoot' to be to 'NewSubRoot'.
  //
  auto FixSubRootCall = [](Function &NewRoot, Function &SubRoot,
                           Function &NewSubRoot) {
    auto UI = SubRoot.use_begin();
    auto UE = SubRoot.use_end();
    while (UI != UE) {
      Use &U = *UI;
      ++UI;
      auto CI = dyn_cast<CallInst>(U.getUser());
      if (CI && CI->getCalledFunction() == &SubRoot &&
          CI->getCaller() == &NewRoot) {
        U.set(&NewSubRoot);
        CI->setCalledFunction(&NewSubRoot);
      }
    }
  };

  // Clone the 'Root' and 'SubRoot'
  ValueToValueMapTy VMap;
  NewMainRoot = CloneFunction(MainRoot, VMap);
  NewSubRoot = CloneFunction(SubRoot, VMap);
  // Split the BasicBlock containing the call to 'Root' into three, so that
  // the call is in its own BasicBlock.
  CallInst *CI = uniqueCallSite(*MainRoot);
  assert(CI && "Expecting unique callsite for MainRoot");
  BasicBlock *OrigBB = CI->getParent();
  BasicBlock *OrigCallBB = OrigBB->splitBasicBlock(CI);
  Instruction *AfterCI = CI->getNextNonDebugInstruction();
  BasicBlock *TailBB = OrigCallBB->splitBasicBlock(AfterCI);
  // Create a BasicBlock to hold the multiversioning test and place the
  // multiversioning test into it.
  BasicBlock *CondBB = BasicBlock::Create(CI->getContext(), ".clone.tile.cond",
                                          OrigBB->getParent(), TailBB);
  Value *TAnd = makeConditionFromGlobals(CondBB);
  Constant *ConstantZero = ConstantInt::get(TAnd->getType(), 0);
  CmpInst *Cmp = CmpInst::Create(Instruction::ICmp, ICmpInst::ICMP_NE, TAnd,
                                 ConstantZero, ".clone.tile.cmp", CondBB);
  Cmp->setDebugLoc(CI->getDebugLoc());
  // Create a BasicBlock for the call to the cloned 'MainRoot' and place the
  // call to the cloned 'MainRoot' in it.
  BasicBlock *NewCallBB = BasicBlock::Create(
      CI->getContext(), ".clone.tile.call", OrigBB->getParent(), TailBB);
  std::vector<Value *> Args(CI->op_begin(), CI->op_end() - 1);
  std::string New_Name;
  New_Name = CI->hasName() ? CI->getName().str() + ".clone.tile.cs" : "";
  CallInst *NewCI = CallInst::Create(NewMainRoot, Args, New_Name, NewCallBB);
  NewCI->setDebugLoc(CI->getDebugLoc());
  NewCI->setCallingConv(CI->getCallingConv());
  NewCI->setAttributes(CI->getAttributes());
  // Patch up the control flow between the BasicBlocks.
  BranchInst *BI = BranchInst::Create(TailBB, NewCallBB);
  BI->setDebugLoc(CI->getDebugLoc());
  Instruction *BIDbg = &OrigBB->getInstList().back();
  OrigBB->getInstList().pop_back();
  BI = BranchInst::Create(CondBB, OrigBB);
  BI->setDebugLoc(BIDbg->getDebugLoc());
  BI = BranchInst::Create(OrigCallBB, NewCallBB, Cmp, CondBB);
  BI->setDebugLoc(Cmp->getDebugLoc());
  // If the cloned call has a return value, tie that return value and the
  // return value of the original together with a PHINode.
  if (!CI->getType()->isVoidTy()) {
    PHINode *RPHI =
        PHINode::Create(CI->getType(), 2, ".clone.tile.phi", &TailBB->front());
    RPHI->addIncoming(NewCI, NewCallBB);
    RPHI->setDebugLoc(CI->getDebugLoc());
    CI->replaceAllUsesWith(RPHI);
  }
  // Make the cloned 'Root' call the cloned 'SubRoot'.
  FixSubRootCall(*NewMainRoot, *SubRoot, *NewSubRoot);
}

void TileMVInlMarker::simplifyConditionalsInRoot() {

  //
  // Simplify the conditionals in 'Root' using 'CM'.
  //
  auto simplifyConditionalsWithMap = [](MapVector<Value *, bool> &CM) {
    for (auto &Pair : CM) {
      Value *V = Pair.first;
      bool Sense = Pair.second;
      Constant *CI = ConstantInt::get(V->getType(), (Sense ? 1 : 0));
      V->replaceAllUsesWith(CI);
    }
  };

  simplifyConditionalsWithMap(CM);
  simplifyConditionalsWithMap(CMExtra);
}

void TileMVInlMarker::markTileChoicesForInlining() {
  for (auto *F : TileChoices)
    for (User *U : F->users()) {
      auto CB = dyn_cast<CallBase>(U);
      if (CB && CB->getCalledFunction() == F &&
          (CB->getCaller() == MainRoot || CB->getCaller() == SubRoot)) {
        CB->addAttribute(llvm::AttributeList::FunctionIndex,
                         "prefer-inline-tile-choice");
        LLVM_DEBUG(dbgs() << "TMVINL: Marked " << CB->getCaller()->getName()
                          << " TO " << F->getName() << "FOR INLINING\n");
      }
    }
}

bool TileMVInlMarker::runImpl() {

  // Checks for whole program and AVX2 to limit scope.
  auto TTIOptLevel = TargetTransformInfo::AdvancedOptLevel::AO_TargetHasAVX2;
  if (!TileCandidateTest &&
      (!WPInfo || !WPInfo->isAdvancedOptEnabled(TTIOptLevel))) {
    //
    // I have removed the test !WPInfo->isWholeProgramSafe() for now as we
    // are not yet achieving whole program. I will add it back in once we are.
    // It is not needed for legality, it is just a screen.
    //
    LLVM_DEBUG(dbgs() << "TMVINL: Did not pass basic screen\n");
    return false;
  }

  unsigned TileCandidateCount = identifyTileCandidates();
  if (TileCandidateCount < TileCandidateMin) {
    LLVM_DEBUG(dbgs() << "TMVINL: Not enough tile candidates\n");
    return false;
  }
  if (!identifyTileRoots()) {
    LLVM_DEBUG(dbgs() << "TMVINL: Did not identify tile roots\n");
    return false;
  }
  makeTileChoices(MainRoot, SubRoot);
  makeTileChoices(SubRoot, nullptr);
  siftTileChoices(MainRoot);
  siftTileChoices(SubRoot);
  makeNonTileChoices(*MainRoot);
  LLVM_DEBUG(dumpTileChoices());
  LLVM_DEBUG(dumpNonTileChoices());
  findGVMandCM();
  LLVM_DEBUG(dumpGVM());
  LLVM_DEBUG(dumpCM());
  if (!validateGVM()) {
    LLVM_DEBUG(dbgs() << "TMVINL: Did not validate GVM\n");
    return false;
  }
  LLVM_DEBUG(dbgs() << "TMVINL: Validated GVM\n");
  if (CM.size())
    cloneCallToRoot();
  markTileChoicesForInlining();
  simplifyConditionalsInRoot();
  LLVM_DEBUG(dumpKeyFunctionNamesAndCalls());
  LLVM_DEBUG(dbgs() << "TMVINL: Multiversioning complete\n");
  return true;
}

namespace {

struct TileMVInlMarkerLegacyPass : public ModulePass {
public:
  static char ID; // Pass identification, replacement for typeid
  TileMVInlMarkerLegacyPass(void) : ModulePass(ID) {
    initializeTileMVInlMarkerLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<PostDominatorTreeWrapperPass>();
    AU.addRequired<WholeProgramWrapperPass>();
    AU.addPreserved<WholeProgramWrapperPass>();
  }

  bool runOnModule(Module &M) override {

    if (skipModule(M))
      return false;

    // Lambda function to find the LoopInfo related to an input function
    TileMVInlMarker::LoopInfoFuncType GetLoopInfo =
        [this](Function &F) -> LoopInfo & {
      return this->getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();
    };

    std::function<DominatorTree &(Function &)> GetDT =
        [this](Function &F) -> DominatorTree & {
      return this->getAnalysis<DominatorTreeWrapperPass>(F).getDomTree();
    };

    std::function<PostDominatorTree &(Function &)> GetPDT =
        [this](Function &F) -> PostDominatorTree & {
      return this->getAnalysis<PostDominatorTreeWrapperPass>(F)
          .getPostDomTree();
    };

    WholeProgramInfo *WPInfo =
        &getAnalysis<WholeProgramWrapperPass>().getResult();

    return TileMVInlMarker(M, GetLoopInfo, &GetDT, &GetPDT, WPInfo).runImpl();
  }
};

} // namespace

char TileMVInlMarkerLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(TileMVInlMarkerLegacyPass, "tilemvinlmarker",
                      "Tile Multiversioning and Inline Marker", false, false)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(PostDominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_END(TileMVInlMarkerLegacyPass, "tilemvinlmarker",
                    "Tile Multiversioning and Inline Marker", false, false)

ModulePass *llvm::createTileMVInlMarkerLegacyPass(void) {
  return new TileMVInlMarkerLegacyPass();
}

TileMVInlMarkerPass::TileMVInlMarkerPass(void) {}

PreservedAnalyses TileMVInlMarkerPass::run(Module &M,
                                           ModuleAnalysisManager &AM) {
  FunctionAnalysisManager &FAM =
      AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();

  TileMVInlMarker::LoopInfoFuncType GetLoopInfo =
      [&FAM](Function &F) -> LoopInfo & {
    return FAM.getResult<LoopAnalysis>(F);
  };

  std::function<DominatorTree &(Function &)> GetDT =
      [&FAM](Function &F) -> DominatorTree & {
    return FAM.getResult<DominatorTreeAnalysis>(F);
  };

  std::function<PostDominatorTree &(Function &)> GetPDT =
      [&FAM](Function &F) -> PostDominatorTree & {
    return FAM.getResult<PostDominatorTreeAnalysis>(F);
  };

  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);

  if (!TileMVInlMarker(M, GetLoopInfo, &GetDT, &GetPDT, &WPInfo).runImpl())
    PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}
