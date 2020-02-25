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
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/Utils/Intel_IPOUtils.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;
using namespace llvm::PatternMatch;

#define DEBUG_TYPE "tilemvinlmarker"

// Enable the optimization for testing purposes
static cl::opt<bool> TileCandidateTest("tile-candidate-test", cl::init(false),
                                       cl::ReallyHidden);
//
// Minimum number of tile candidate functions for this transformation.
//
static cl::opt<unsigned> TileCandidateMin(
    "tile-candidate-min", cl::init(6), cl::ReallyHidden,
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
     auto CB = dyn_cast<CallBase>(U);
     if (!CB || Caller)
       return nullptr;
     Caller = CB->getCaller();
   }
   return Caller;
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
                  WholeProgramInfo *WPInfo)
      : M(M), GetLoopInfo(LoopInfoFunc), WPInfo(WPInfo) {}

  // Run the tile multiversioning and inline marker
  bool runImpl();

private:
  Module &M;
  LoopInfoFuncType &GetLoopInfo;
  WholeProgramInfo *WPInfo;

  // A set of tile candidates.  These are Functions which have arrays that
  // are indexed by both "i" and "i+1" or "i-1", where "i" is the loop index.
  // The loops enclosing these arrays are good candidates for tiling. We
  // use the presence of tile candidates as an initial step in determining
  // if we will attempt the transformation.
  SmallPtrSet<Function *, 10> TileCandidates;

  // A set of tile choices. These are the Functions which we will mark for
  // inlining so that their code can be exposed to Loop Opt for tiling.
  SmallPtrSet<Function *, 10> TileChoices;

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

  // Return the number of tile candidates. The transformation will only be
  // performed if enough tile candidates are found.
  unsigned identifyTileCandidates(void);

  // A pair of tile roots. A tile root is a Function which calls other
  // Functions that are tile choices.  The tile choices are Functions which
  // will be marked for inlining to facilitate tiling in Loop Opt.
  std::pair<Function *, Function *> identifyTileRoots(void);

  // Create a initial set of tile choices.
  void makeTileChoices(Function *Root, Function *RootSub);

  // Refine the initial set of tile choices by removing some of them for the
  // set.
  void siftTileChoices(Function *F);

#ifndef NDEBUG
  // Dump out a message indicating that 'LI' is a loop index and 'LO' is that
  // loop index plus or minus and offset in 'F'.
  void dumpLoopIndexPair(Function &F, Value *LI, Value *LO);

  // Dump out the tile choices.
  void dumpTileChoices(void);
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

bool TileMVInlMarker::processLoopCaseStart(Function &F,
                                           TestStackTuple &Item,
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

bool TileMVInlMarker::processLoopCaseFoundInc(Function &F,
                                              TestStackTuple &Item,
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

bool TileMVInlMarker::processLoopCaseFoundPHI(Function &F,
                                              TestStackTuple &Item,
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
  LLVM_DEBUG(dbgs() << "TILEMVINL: " << Arg.getParent()->getName()
                    << " Arg %" << Arg.getArgNo()
                    << "(" << ZeroCount << "," << OffsetCount << ")\n");
  return ZeroCount && OffsetCount;
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
      LLVM_DEBUG(dbgs() << "TILEMVINL: Tile Candidate " << F.getName() << "\n");
      return true;
    }
  }
  return false;
}

unsigned TileMVInlMarker::identifyTileCandidates(void) {
  for (auto &F : M.functions())
    if (isTileCandidate(F))
      TileCandidates.insert(&F);
  return TileCandidates.size();
}

std::pair<Function *, Function *> TileMVInlMarker::identifyTileRoots(void) {
  DenseMap<Function *, unsigned> Callers;
  Function *CallerMax = nullptr;
  unsigned ValueMax = 0;
  for (auto *F : TileCandidates) {
    Function *Caller = uniqueCaller(*F);
    if (!Caller)
      continue;
    Callers[Caller] = Callers[Caller] + 1;
    if (Callers[Caller] > ValueMax) {
      CallerMax = Caller;
      ValueMax = Callers[Caller];
    }
  }
  if (!CallerMax)
    return std::make_pair(nullptr, nullptr);
  Function *Main = uniqueCaller(*CallerMax);
  if (!Main)
    return std::make_pair(nullptr, nullptr);
  if (!WPUtils.isMainEntryPoint(Main->getName()))
    return std::make_pair(nullptr, nullptr);
  Function *CallerMaxSub = nullptr;
  for (auto *F : TileCandidates) {
    Function *Caller = uniqueCaller(*F);
    if (Caller == CallerMax)
      continue;
    if (CallerMaxSub && (Caller != CallerMaxSub))
      return std::make_pair(nullptr, nullptr);
    CallerMaxSub = Caller;
    Caller = uniqueCaller(*Caller);
    if (Caller == CallerMax)
      continue;
    return std::make_pair(nullptr, nullptr);
  }
  if (!CallerMaxSub)
    return std::make_pair(nullptr, nullptr);
  return std::make_pair(CallerMax, CallerMaxSub);
}

void TileMVInlMarker::makeTileChoices(Function *Root, Function *RootSub) {
  for (auto &I : instructions(Root)) {
    auto CB = dyn_cast<CallBase>(&I);
    if (!CB)
      continue;
    Function *Callee = CB->getCalledFunction();
    if (!Callee || Callee->isDeclaration())
      continue;
    if (Callee != RootSub && (hasUniqueTileSubscriptArg(*Callee) ||
        std::distance(Callee->arg_begin(), Callee->arg_end()) >=
        TileCandidateArgMin && IPOUtils::isLeafFunction(*Callee))) {
      TileChoices.insert(Callee);
    }
  }
}

void TileMVInlMarker::siftTileChoices(Function *F) {

  auto GetTargetCall = [](BasicBlock *BB) -> Function * {
    if (BB->empty())
      return nullptr;
    BasicBlock::iterator I = BB->begin();
    while (isa<DbgInfoIntrinsic>(I))
      ++I;
    auto CB = dyn_cast<CallBase>(&*I);
    if (!CB)
      return nullptr;
    return CB->getCalledFunction();
  };

  for (auto &BB : *F) {
    auto BI = dyn_cast<BranchInst>(BB.getTerminator());
    if (!BI || BI->isUnconditional())
      continue;
    auto LI = dyn_cast<LoadInst>(BI->getCondition());
    if (!LI || !isa<GlobalVariable>(LI->getPointerOperand()))
      continue;
    Function *TrueF = GetTargetCall(BI->getSuccessor(0));
    if (!TrueF || !TileChoices.count(TrueF))
      continue;
    Function *FalseF = GetTargetCall(BI->getSuccessor(1));
    if (!FalseF || !TileChoices.count(FalseF))
      continue;
    TileChoices.erase(FalseF);
  }
}

#ifndef NDEBUG
void TileMVInlMarker::dumpTileChoices(void) {
  for (auto *F : TileChoices)
    dbgs() << "TILEMVINL: Tile Choice " << F->getName() << "\n";
}
#endif // NDEBUG

bool TileMVInlMarker::runImpl() {

  // Checks for whole program and AVX2 to limit scope.
  auto TTIOptLevel = TargetTransformInfo::AdvancedOptLevel::AO_TargetHasAVX2;
  if (!TileCandidateTest &&
      (!WPInfo || !WPInfo->isAdvancedOptEnabled(TTIOptLevel)))
    //
    // I have removed the test !WPInfo->isWholeProgramSafe() for now as we
    // are not yet achieving whole program. I will add it back in once we are.
    // It is not needed for legality, it is just a screen.
    //
    return false;

  unsigned TileCandidateCount = identifyTileCandidates();
  if (TileCandidateCount < TileCandidateMin)
     return false;
  std::pair<Function *, Function *> Roots = identifyTileRoots();
  if (!Roots.first || !Roots.second)
     return false;
  makeTileChoices(Roots.first, Roots.second);
  makeTileChoices(Roots.second, nullptr);
  siftTileChoices(Roots.first);
  siftTileChoices(Roots.second);
  LLVM_DEBUG(dumpTileChoices());
  //
  // TODO: Add multiversioning and setting of callsites for inlining.
  //
  return false;
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

    WholeProgramInfo *WPInfo =
        &getAnalysis<WholeProgramWrapperPass>().getResult();

    return TileMVInlMarker(M, GetLoopInfo, WPInfo).runImpl();
  }
};

} // namespace

char TileMVInlMarkerLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(TileMVInlMarkerLegacyPass, "tilemvinlmarker",
                      "Tile Multiversioning and Inline Marker", false, false)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
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

  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);

  if (!TileMVInlMarker(M, GetLoopInfo, &WPInfo).runImpl())
    PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}
