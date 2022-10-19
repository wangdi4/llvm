//===- Intel_ArrayUseAnalysis.cpp - Array Usage Analysis ------------------===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This analysis is a specialized analysis that determines what indexes of an
// array may be accessed by subsequent portions of the code.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_ArrayUseAnalysis.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/Analysis/Delinearization.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Analysis/PtrUseVisitor.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/InitializePasses.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Pass.h"

using namespace llvm;

#define DEBUG_TYPE "array-use"

AnalysisKey ArrayUseAnalysis::Key;

ArrayUse ArrayUseAnalysis::run(Function &F, FunctionAnalysisManager &AM) {
  return ArrayUse(F,
      AM.getResult<LoopAnalysis>(F),
      AM.getResult<ScalarEvolutionAnalysis>(F));
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
PreservedAnalyses ArrayUsePrinterPass::run(Function &F,
                                           FunctionAnalysisManager &AM) {
  AM.getResult<ArrayUseAnalysis>(F).print(OS);
  return PreservedAnalyses::all();
}
#endif

INITIALIZE_PASS_BEGIN(ArrayUseWrapperPass, "array-use",
                      "Array Use Analysis", false, true)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolutionWrapperPass)
INITIALIZE_PASS_END(ArrayUseWrapperPass, "array-use",
                    "Array Use Analysis", false, true)

char ArrayUseWrapperPass::ID = 0;

ArrayUseWrapperPass::ArrayUseWrapperPass() : FunctionPass(ID) {
  initializeArrayUseWrapperPassPass(*PassRegistry::getPassRegistry());
}

bool ArrayUseWrapperPass::runOnFunction(Function &F) {
  auto &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  auto &SE = getAnalysis<ScalarEvolutionWrapperPass>().getSE();
  AU.reset(new ArrayUse(F, LI, SE));
  return false;
}

void ArrayUseWrapperPass::releaseMemory() { AU.reset(); }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void ArrayUseWrapperPass::print(raw_ostream &OS, const Module *) const {
  AU->print(OS);
}
#endif

void ArrayUseWrapperPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequiredTransitive<LoopInfoWrapperPass>();
  AU.addRequiredTransitive<ScalarEvolutionWrapperPass>();
}

FunctionPass *llvm::createArrayUseWrapperPass() {
  return new ArrayUseWrapperPass();
}

ArrayRangeInfo ArrayRangeInfo::unionWith(const ArrayRangeInfo &Other, ScalarEvolution &SE) const {
  if (isEmpty() || Other.isFull()) return Other;
  if (isFull() || Other.isEmpty()) return *this;

  return ArrayRangeInfo::range(
      SE.getUMinExpr(Low, Other.Low),
      SE.getUMaxExpr(High, Other.High));
}

ArrayRangeInfo ArrayRangeInfo::difference(const ArrayRangeInfo &Other, ScalarEvolution &SE) const {
  if (isEmpty() || Other.isFull()) return ArrayRangeInfo::empty();
  if (Other.isEmpty()) return *this;
  if (*this == Other) return ArrayRangeInfo::empty();

  // This is a horribly imprecise implementation of difference. But it does not
  // appear that we have a need to do what ConstantRange does yet.
  if (isFull()) {
    LLVM_DEBUG(dbgs() << "Imprecise difference: full set - " << Other << "\n");
    return *this;
  }

  assert(Kind == RANGE && Other.Kind == RANGE &&
      "Computation should be ranged at this point.");

  // Convert the ranges to ConstantRange and do the math using that
  // implementation. SCEV values are converted to constants using SCEV's range
  // analysis. Recall that ConstantRange does exclusive ranges, while we do
  // inclusive ranges, so adaptation is necessary.
  auto makeRange = [&](const ArrayRangeInfo &Range) {
    auto x = ConstantRange(SE.getUnsignedRangeMin(Range.Low),
        ++SE.getUnsignedRangeMax(Range.High));
    return x;
  };
  ConstantRange Res = makeRange(*this).difference(makeRange(Other));
  const SCEV *ResLow = SE.getConstant(Res.getUnsignedMin());
  const SCEV *ResHigh = SE.getConstant(Res.getUnsignedMax());
  ArrayRangeInfo Final = Res.isEmptySet()
    ? ArrayRangeInfo::empty()
    : ArrayRangeInfo::range(ResLow, ResHigh);
  return Final;
}

bool ArrayRangeInfo::operator==(const ArrayRangeInfo &Other) const {
  return Kind == Other.Kind && Low == Other.Low && High == Other.High;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void ArrayRangeInfo::print(raw_ostream &OS) const {
  if (isEmpty()) {
    OS << "empty set";
  } else if (isFull()) {
    OS << "full set";
  } else {
    OS << "[" << *Low << ", " << *High << "]";
  }
}
#endif

/// Helper class to implement getPointerUses correctly.
class PointerUseHelper : public PtrUseVisitor<PointerUseHelper> {
  friend class PtrUseVisitor<PointerUseHelper>;
  friend class InstVisitor<PointerUseHelper>;
  using BaseT = PtrUseVisitor<PointerUseHelper>;

  void visitLoadInst(LoadInst &LI) {
    Uses.push_back(&LI);
  }
  void visitStoreInst(StoreInst &SI) {
    // Ignore stores if the use is in the value being stored instead of the
    // store itself.
    if (SI.getValueOperand() == *U)
      return;
    Uses.push_back(&SI);
  }

  void visitCallInst(CallInst &I) {
    processCallBase(&I);
  }

  void visitInvokeInst(InvokeInst &I) {
    processCallBase(&I);
  }

  void processCallBase(CallBase *Call) {
    // Special-handle qsort.
    if (Call->hasFnAttr("is-qsort")) {
      Uses.push_back(Call);
      return;
    }

    // All other cases, treat this as escaping. We could check for capture
    // read/only information, but that doesn't buy us anything yet.
    PI.setEscaped(Call);
  }

  void visitInstruction(Instruction &I) {
    PI.setAborted(&I);
  }

public:
  PointerUseHelper(const DataLayout &DL) : BaseT(DL) {}
  std::vector<Instruction *> Uses;
};

/// Look through all cast instructions to get all of the uses of a pointer-typed
/// instruction. If the pointer escapes, or some unanalyzed instruction is
/// present, then return an empty list instead.
static std::vector<Instruction *> getPointerUses(Instruction &I,
    const DataLayout &DL) {
  if (!I.getType()->isPointerTy()) {
    // If there's exactly one use, and it's a pointer type, use that.
    if (!I.hasOneUse())
      return {};
    if (auto UseI = dyn_cast<PtrToIntInst>(&I))
      return getPointerUses(*UseI, DL);

    // Can't do anything.
    return {};
  }
  PointerUseHelper Helper(DL);
  auto PI = Helper.visitPtr(I);
  if (PI.isAborted() || PI.isEscaped()) {
    LLVM_DEBUG({
      if (auto AI = PI.getAbortingInst())
        dbgs() << "  aborted because of unknown use: " << *AI << "\n";
      if (auto EI = PI.getEscapingInst())
        dbgs() << "  aborted because of escaping use: " << *EI << "\n";
    });
    return {};
  }
  return std::move(Helper.Uses);
}

/// Represents a summary of array define/usage information for the dataflow
/// information.
struct GenKillInfo {
  bool IsGen;
  ArrayRangeInfo Range;
  /// The point in a basic block where this information applies.
  Instruction *AtPoint;

  GenKillInfo() = delete;
  GenKillInfo(const ArrayRangeInfo &Range, Instruction *AtPoint, bool IsGen)
    : IsGen(IsGen), Range(Range), AtPoint(AtPoint) {}
};

/// Compute and save the results of a backwards dataflow analysis to query how
/// much of the array will be used after any given point.
///
/// For each basic block, we store the gen/kill information of all instructions
/// in the block (in reverse order) and the range that exists at the end of the
/// block. Any given point in the block can be reconstructed by walking
/// backwards from the live out until the point in question.
class ArrayUseInfo::RangeDataflow {
  ScalarEvolution &SE;

  DenseMap<BasicBlock *, SmallVector<GenKillInfo, 4>> BlockMap;
  DenseMap<BasicBlock *, ArrayRangeInfo> LiveOuts;

public:
  bool Valid = false;

  RangeDataflow(ScalarEvolution &SE, const ArrayUse &AU)
    : SE(SE) {}

  void noteGenKillInfo(GenKillInfo GKI, Instruction *Point) {
    BlockMap[Point->getParent()].emplace_back(std::move(GKI));
  }

  void performDataflow(Instruction &Alloca) {
    Function &F = *Alloca.getParent()->getParent();

    // Before doing dataflow, sort the gen/kill info in backwards order for the
    // basic block.
    for (auto &BB : F) {
      auto &InstArray = BlockMap[&BB];
      llvm::sort(InstArray.begin(), InstArray.end(),
          [](const GenKillInfo &A, const GenKillInfo &B) {
            return B.AtPoint->comesBefore(A.AtPoint);
          });
    }

    LLVM_DEBUG(dbgs() << "Running dataflow analysis to find array usage...\n");

    // Size live-ins/live-outs to the size of the function. Don't initialize
    // them with any values, though--they'll start as the empty set by default.
    DenseMap<BasicBlock *, ArrayRangeInfo> LiveIns;
    LiveIns.reserve(F.size());
    LiveOuts.reserve(F.size());

    // Main dataflow analysis loop.
    bool Changed;
    do {
      Changed = false;
      for (const auto &BB : post_order(&F)) {
        ArrayRangeInfo In = ArrayRangeInfo::empty();
        for (auto Succ : successors(BB)) {
          In = In.unionWith(LiveIns[Succ], SE);
        }
        LiveOuts[BB] = In;

        ArrayRangeInfo Out = In;
        for (auto &GKI : BlockMap[BB]) {
          if (GKI.IsGen)
            Out = Out.unionWith(GKI.Range, SE);
          else
            Out = Out.difference(GKI.Range, SE);
        }
        auto &OldOut = LiveIns[BB];
        if (OldOut != Out) {
          Changed = true;
          OldOut = Out;
        }
      }
    } while (Changed);

    LLVM_DEBUG({
      dbgs() << "Computed dataflow results:\n";
      for (auto &BB : F) {
        dbgs() << "Gen/kill for " << BB.getName() << ":\n";
        dbgs() << "  live out: ";
        LiveOuts[&BB].print(dbgs());
        dbgs() << "\n";
        for (auto &GKI : BlockMap[&BB]) {
          dbgs() << (GKI.IsGen ? "  load " : "  store ");
          GKI.Range.print(dbgs());
          dbgs() << "\n";
        }
        dbgs() << "  live in: ";
        LiveIns[&BB].print(dbgs());
        dbgs() << "\n";
      }
    });

    Valid = true;
  }

  ArrayRangeInfo getUseAfter(Instruction &I) {
    BasicBlock *BB = I.getParent();
    ArrayRangeInfo Range = LiveOuts[BB];
    auto &BBInfo = BlockMap[BB];

    // Fastest path: no instructions change in block -> return live out.
    if (BBInfo.empty())
      return Range;

    // Fast path: Assume I is in the list of Gen/kill info.
    for (auto &GKI : BBInfo) {
      if (GKI.AtPoint == &I)
        return Range;
      if (GKI.IsGen)
        Range = Range.unionWith(GKI.Range, SE);
      else
        Range = Range.difference(GKI.Range, SE);
    }

    // We don't have I in the list. walk the BB backwards into the instruction.
    Range = LiveOuts[BB];
    auto GKIPtr = BBInfo.begin();
    for (auto &BBInst : reverse(*BB)) {
      // If this is in the gen/kill info, apply the update.
      if (GKIPtr->AtPoint == &BBInst) {
        if (GKIPtr->IsGen)
          Range = Range.unionWith(GKIPtr->Range, SE);
        else
          Range = Range.difference(GKIPtr->Range, SE);
        // If that's it, return current info.
        if (++GKIPtr == BBInfo.end())
          return Range;
      }

      if (&BBInst == &I)
        return Range;
    }
    return ArrayRangeInfo::full();
  }
};

std::unique_ptr<ArrayUseInfo::RangeDataflow>
ArrayUseInfo::computeDataflow(ScalarEvolution &SE, const ArrayUse &AU) {
  Instruction &I = *cast<Instruction>(Source);
  RangeDataflow *DataflowResults = new RangeDataflow(SE, AU);
  LLVM_DEBUG(dbgs() << "Computing dataflow for " << I << "\n");

  // Start by computing the uses of the array.
  std::vector<Instruction *> Uses = getPointerUses(I, SE.getDataLayout());
  if (Uses.empty()) {
    return std::unique_ptr<RangeDataflow>(DataflowResults);
  }

  auto addUse = [&](const ArrayRangeInfo &Range, Instruction *U,
      Instruction *Point) {
    GenKillInfo GKI(Range, Point, isa<LoadInst>(U));
    DataflowResults->noteGenKillInfo(std::move(GKI), Point);
  };

  for (Instruction *U : Uses) {
    ArrayRangeInfo Range = AU.getRangeUse(*U);
    LLVM_DEBUG(dbgs() << "Found range " << Range << " for use " << *U << "\n");

    bool UsedIndirect = false;
    if (CheckIndirectUses && isa<LoadInst>(U)) {
      std::vector<Instruction *> IndirectUses =
        getPointerUses(*U, SE.getDataLayout());
      if (!IndirectUses.empty()) {
        UsedIndirect = true;
        LLVM_DEBUG(dbgs() << "Using indirect uses instead:\n");
        for (Instruction *IndirectUse : IndirectUses) {
          LLVM_DEBUG(dbgs() << "  Found indirect use " << *IndirectUse << "\n");
          addUse(Range, IndirectUse, U);
        }
      }
    }

    if (!UsedIndirect)
      addUse(Range, U, U);
  }

  DataflowResults->performDataflow(I);
  return std::unique_ptr<RangeDataflow>(DataflowResults);
}

ArrayUseInfo::ArrayUseInfo(Value *Source, const SCEV *Size, bool CheckIndirect)
  : Source(Source), Size(Size), CheckIndirectUses(CheckIndirect) {}
ArrayUseInfo::~ArrayUseInfo() = default;

std::unique_ptr<ArrayUseInfo> ArrayUseInfo::make(Value *Source,
                                                 ScalarEvolution &SE) {
  if (auto AI = dyn_cast<AllocaInst>(Source)) {
    const SCEV *ArraySize;
    if (AI->isArrayAllocation())
      ArraySize = SE.getSCEV(AI->getArraySize());
    else if (AI->getAllocatedType()->isArrayTy()) {
      ArraySize = SE.getConstant(
          Type::getInt64Ty(Source->getContext()),
          AI->getAllocatedType()->getArrayNumElements());
    } else {
      return nullptr;
    }

    bool CheckIndirect =
      AI->getAllocatedType()->getArrayElementType()->isPointerTy();

    return std::make_unique<ArrayUseInfo>(Source, ArraySize, CheckIndirect);
  } else {
    // We could support global arrays in this analysis. But PtrUseVisitor, which
    // we use for tracking, tracks starting from an Instruction, making it more
    // difficult to do the analysis. For now these are unsupported.
    // Additionally, we could check for memory allocation routines here, but
    // until there is found value to do so, these are also not supported.
    return nullptr;
  }
}

ArrayRangeInfo ArrayUseInfo::getRangeUseAfterPoint(Instruction *I) const {
  assert(DataflowResults && "Dataflow analysis should have been initialized");
  if (!DataflowResults->Valid)
    return ArrayRangeInfo::full();

  return DataflowResults->getUseAfter(*I);
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void ArrayUseInfo::print(raw_ostream &OS) const {
  OS << "array at " << *Source << " (size " << *Size << ")";
}
#endif

ArrayUse::ArrayUse(Function &F, LoopInfo &LI, ScalarEvolution &SE)
:
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  F(F),
#endif
  LI(LI), SE(SE), ArrayUseMap(std::make_unique<ArrayUse::ArrayMapTy>()) {
}

ArrayRangeInfo ArrayUse::getRangeUse(Instruction &I) const {
  if (isa<CallInst>(&I))
    return ArrayRangeInfo::full();
  Value *Addr = getLoadStorePointerOperand(&I);
  assert(Addr && "Should not query this if not a pointer");

  const SCEV *Value = SE.getSCEV(Addr);
  ArrayUseInfo *UseInfoPtr = getSourceArray(Addr);
  assert(UseInfoPtr && "We should have already added this to the size");
  const SCEV *Base = SE.getPointerBase(Value);
  const SCEV *PtrOffset = SE.getMinusSCEV(Value, Base, SCEV::FlagNW);
  const SCEV *ElemSize = SE.getElementSize(&I);

  // There are two routines in ScalarEvolution that attempt to divide an
  // expression by a constant. SCEVDivision is the more aggressive one (it
  // doesn't rely on wrapping flags for correctness), but it is not directly
  // exposed. We use findArrayDimensions instead to get to this routine.
  SmallVector<const SCEV *, 1> Terms, Sizes;
  Terms.push_back(PtrOffset);
  findArrayDimensions(SE, Terms, Sizes, ElemSize);
  const SCEV *IdxOffset = Sizes.empty() ? SE.getUDivExpr(PtrOffset, ElemSize) :
    Sizes[0];

  // Get range information from the index expression.
  return getRangeForSCEV(IdxOffset, UseInfoPtr->Size);
}

ArrayRangeInfo ArrayUse::getRangeForSCEV(const SCEV *S, const SCEV *Size) const {
  // Constant -> return the known constant range.
  if (auto Const = dyn_cast<SCEVConstant>(S)) {
    return ArrayRangeInfo::range(Const, Const);
  }

  // AddRec -> the lower bound is at the start, upper bound is max BE count.
  if (auto AddRec = dyn_cast<SCEVAddRecExpr>(S)) {
    const SCEV *Base = AddRec->getStart();
    const SCEV *Limit = SE.getConstantMaxBackedgeTakenCount(AddRec->getLoop());
    const SCEV *Last = AddRec->evaluateAtIteration(Limit, SE);
    return ArrayRangeInfo::range(Base, Last);
  }

  // Add -> try to add the ranges together, and clip the result to the known
  // valid maximum for the size.
  if (auto AddExpr = dyn_cast<SCEVAddExpr>(S)) {
    const SCEV *Zero = SE.getZero(Size->getType());
    SmallVector<const SCEV *, 2> LowOps, HighOps;
    for (auto Op : AddExpr->operands()) {
      ArrayRangeInfo RI = getRangeForSCEV(Op, Size);
      // If the range is empty, then we're probably into undefined range, as the
      // pointer has gone out of range of the array. Propagate empty back up to
      // the full expression.
      if (RI.isEmpty())
        return ArrayRangeInfo::empty();
      if (RI.isFull()) {
        LowOps.push_back(Zero);
        HighOps.push_back(Size);
      } else {
        LowOps.push_back(RI.getLowIndex());
        HighOps.push_back(RI.getHighIndex());
      }
    }

    // Add the ranges together, and clip them to the range [0, N). If the
    // resulting expression is too complicated, then pessimize it to [0, N).
    const SCEV *Low = SE.getSMaxExpr(SE.getAddExpr(LowOps, SCEV::FlagNUW),
        Zero);
    const SCEV *High = SE.getSMinExpr(SE.getAddExpr(HighOps, SCEV::FlagNUW),
        Size);
    if (isa<SCEVMinMaxExpr>(Low))
      Low = Zero;
    if (isa<SCEVMinMaxExpr>(High))
      High = Size;

    if (Low == Zero && High == Size)
      return ArrayRangeInfo::full();
    return ArrayRangeInfo::range(Low, High);
  }

  // If we can't do anything, try asking SCEV if it knows anything about the
  // range.
  if (auto UnknownExpr = dyn_cast<SCEVUnknown>(S)) {
    Value *V = UnknownExpr->getValue();
    ConstantRange Range = SE.getUnsignedRange(S);
    // TODO: This method call below should be a part of SCEV's getUnsignedRange,
    // but doing so causes as-yet unanalyzed regressions.
    // Check the loop-header phis for a more refined range based on the loop
    // increasing.
    if (auto Phi = dyn_cast<PHINode>(V)) {
      if (LI.isLoopHeader(Phi->getParent())) {
        ConstantRange PhiRange = SE.getRangeBoundedByLoop(*Phi);
        Range = Range.intersectWith(PhiRange);
      }
    }

    // Map the range to an ArrayRangeInfo.
    if (Range.isFullSet())
      return ArrayRangeInfo::full();
    else if (Range.isEmptySet())
      return ArrayRangeInfo::empty();
    else {
      APInt Upper = Range.getUpper();
      return ArrayRangeInfo::range(SE.getConstant(Range.getLower()),
          SE.getConstant(--Upper));
    }
  }

  LLVM_DEBUG(dbgs() << "Unknown SCEV range analysis: " << *S << "\n");
  return ArrayRangeInfo::full();
}

ArrayUseInfo *ArrayUse::getSourceArray(Value *V) const {
  const SCEV *Base = SE.getPointerBase(SE.getSCEV(V));
  assert(Base && "Should have SCEV result for source value.");

  auto BaseVal = dyn_cast<SCEVUnknown>(Base);
  if (!BaseVal)
    return nullptr;

  Value *ArrayVal = BaseVal->getValue();
  auto &UseInfoPtr = (*ArrayUseMap)[ArrayVal];
  if (!UseInfoPtr) {
    UseInfoPtr = ArrayUseInfo::make(ArrayVal, SE);
    if (UseInfoPtr) {
      UseInfoPtr->DataflowResults = UseInfoPtr->computeDataflow(SE, *this);
    }
  }
  return UseInfoPtr.get();
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void ArrayUse::print(raw_ostream &OS) const {
  OS << "Classifying array use for: ";
  F.printAsOperand(OS, /*PrintType=*/false);
  OS << "\n";
  for (Instruction &I : instructions(F)) {
    Value *Addr = getLoadStorePointerOperand(&I);
    if (!Addr) {
      if (auto CI = dyn_cast<CallInst>(&I)) {
        Function *Target = CI->getCalledFunction();
        if (Target && Target->getName().startswith("spec_qsort"))
          Addr = CI->getArgOperand(0);
      }
    }
    if (!Addr)
      continue;
    const ArrayUseInfo *AUI = getSourceArray(Addr);
    if (!AUI) {
      continue;
    }
    OS << I << "\n  -->  ";
    AUI->print(OS);
    OS << " " << getRangeUse(I);
    OS << "\n       only using " << AUI->getRangeUseAfterPoint(&I) << "\n";
  }
}
#endif

