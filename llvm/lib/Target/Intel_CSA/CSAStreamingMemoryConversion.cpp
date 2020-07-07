//===- CSAStreamingMemoryConversion.cpp - Streaming operations -*- C++ -*--===//
//
// Copyright (C) 2017-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements a pass that converts memory operations to streaming
// memory loads and stores where applicable.
//
//===----------------------------------------------------------------------===//

#include "CSA.h"
#include "llvm/ADT/Optional.h"
#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/OptimizationRemarkEmitter.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/CodeGen/MachineMemOperand.h" // To get CSA_LOCAL_CACHE_METADATA_KEY.
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IntrinsicsCSA.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/ScalarEvolutionExpander.h"
#include "llvm/Transforms/Utils/LoopUtils.h"

using namespace llvm;

#define DEBUG_TYPE "csa-streammem"
#define PASS_DESC "CSA: Streaming memory conversion in IR"

static cl::opt<bool> DisableStreamingMemory(
  "csa-disable-streammem", cl::Hidden,
  cl::desc("CSA Specific: disable streaming memory conversion"));

static cl::opt<bool> EnableLargerStrides(
  "csa-enable-all-strides", cl::Hidden,
  cl::desc("CSA Specific: enable streaming memory even if stride is not 1"));

static cl::opt<bool> EnableWideLoads(
  "csa-enable-wide-loads", cl::Hidden,
  cl::desc("CSA Specific: enable wide loads using sld64x2"),
  cl::init(false));

static cl::opt<bool> EnableAllLoops(
  "csa-streammem-expensive", cl::Hidden,
  cl::desc("CSA Specific: enable streaming memory even if trip counts are expensive"));

extern cl::opt<bool> ILPLUseCompletionBuf;

static cl::opt<unsigned> NumVCPRUnits("csa-max-vcpr", cl::Hidden, cl::init(256),
  cl::desc("Maximum number of VCPR units to generate for"));

static cl::opt<unsigned> NumVCPFUnits("csa-max-vcpf", cl::Hidden, cl::init(256),
  cl::desc("Maximum number of VCPF units to generate for"));

namespace llvm {
void initializeCSAStreamingMemoryPass(PassRegistry &);
}

namespace {
struct StreamingMemoryDetails {
  const SCEV *Base;
  int64_t Stride;

  CallInst *InOrd;
  CallInst *OutOrd;

  Type *MemTy;
  Instruction *MemInst;
};

struct WideLoadDetails {
  const SCEV *Address;

  CallInst *InOrd;
  CallInst *OutOrd;

  Type *MemTy;
  Instruction *MemInst;
};

class CSAStreamingMemoryImpl {
  DominatorTree &DT;
  LoopInfo &LI;
  OptimizationRemarkEmitter &ORE;
  ScalarEvolution &SE;
  BlockFrequencyInfo &BFI;
  SCEVExpander Expander;
  DenseMap<BasicBlock *, const SCEV *> ExecCounts;

  const char *LocalCacheKey;

  bool isPipelinedLoop(Loop *L);

  Optional<StreamingMemoryDetails> getLegalStream(Value *Pointer,
      Instruction *MemInst);
  Optional<WideLoadDetails> getLegalWideLoad(Value *Pointer,
      Instruction *MemInst);
  CallInst *getInordEdge(Instruction *MemInst);
  CallInst *getOutordEdge(Instruction *MemInst);

  Value *createLic(Type *LicType, Instruction *PushIP, Instruction *PopIP,
      const Twine &LicName, Value *Push);

  void makeStreaming(StreamingMemoryDetails &Details);
  bool attemptWide(StreamingMemoryDetails &A, StreamingMemoryDetails &B);
  bool attemptWideLoad(WideLoadDetails &A, WideLoadDetails &B);

  void reportSuccess(Instruction *MemInst) {
    OptimizationRemark R(DEBUG_TYPE, "StreamingMemory", MemInst);
    ORE.emit(R << "converted to streaming memory reference");
  };
  void reportFailure(Instruction *MemInst, const char *message) {
    OptimizationRemarkMissed R(DEBUG_TYPE, "StreamingMemory", MemInst);
    ORE.emit(R << "streaming memory conversion failed: " << message);
  };

  unsigned NextLicId = 0;

  void calculateResourceUsage(Function &F);
  // These are the counts of the resource units we're consuming to go to and
  // from the RAFs.
  unsigned NumToMem, NumFromMem;

public:
  CSAStreamingMemoryImpl(DominatorTree &DT, LoopInfo &LI, ScalarEvolution &SE,
      OptimizationRemarkEmitter &ORE, BlockFrequencyInfo &BFI)
    : DT(DT), LI(LI), ORE(ORE), SE(SE), BFI(BFI),
      Expander(SE, SE.getDataLayout(), "streammem"),
      LocalCacheKey(CSA_LOCAL_CACHE_METADATA_KEY) {
        // Cause AddRecExprs to be expanded as phi loops rather than mul/adds.
        Expander.disableCanonicalMode();
      }

  bool run(Function &F);
  bool runOnLoop(Loop *L);
};

struct CSAStreamingMemory : public FunctionPass {
  static char ID;

  explicit CSAStreamingMemory() : FunctionPass(ID) {
    initializeCSAStreamingMemoryPass(*PassRegistry::getPassRegistry());
  }
  StringRef getPassName() const override { return PASS_DESC; }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequired<OptimizationRemarkEmitterWrapperPass>();
    AU.addRequired<ScalarEvolutionWrapperPass>();
    AU.addRequired<BlockFrequencyInfoWrapperPass>();
    AU.setPreservesAll();
  }

  bool runOnFunction(Function &F) override;
  bool runOnLoop(Loop *L);
};

bool isExpensiveSCEV(ScalarEvolution &SE, const SCEV *S) {
  // Check if there is a value that corresponds to S.
  auto *Set = SE.getSCEVValues(S);
  if (Set) {
    for (auto const &VOPair : *Set) {
      if (VOPair.first)
        return false;
    }
  }

  // In theory, we could say that * and / that can be peepholed to shift
  // operations are cheap.
  switch (S->getSCEVType()) {
  case scMulExpr: {
    auto Mul = cast<SCEVMulExpr>(S);
    // Multiplication by negative 1 is cheap.
    if (Mul->getNumOperands() == 2 && Mul->getOperand(0)->isAllOnesValue())
      return false;
    return true;
  }
  case scUDivExpr: {
  }
  case scCouldNotCompute:
    return true;
  }
  return false;
}

} // namespace

char CSAStreamingMemory::ID = 0;
INITIALIZE_PASS_BEGIN(CSAStreamingMemory, DEBUG_TYPE, PASS_DESC,
                      false, false)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(OptimizationRemarkEmitterWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolutionWrapperPass)
INITIALIZE_PASS_DEPENDENCY(BlockFrequencyInfoWrapperPass)
INITIALIZE_PASS_END(CSAStreamingMemory, DEBUG_TYPE, PASS_DESC,
                    false, false)

Pass *llvm::createCSAStreamingMemoryConversionPass() {
  return new CSAStreamingMemory();
}

bool CSAStreamingMemory::runOnFunction(Function &F) {
  bool Changed = false;

  if (skipFunction(F) || DisableStreamingMemory)
    return Changed;

  auto &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  auto &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  auto &SE = getAnalysis<ScalarEvolutionWrapperPass>().getSE();
  auto &ORE = getAnalysis<OptimizationRemarkEmitterWrapperPass>().getORE();
  auto &BFI = getAnalysis<BlockFrequencyInfoWrapperPass>().getBFI();

  return CSAStreamingMemoryImpl(DT, LI, SE, ORE, BFI).run(F);
}

bool CSAStreamingMemoryImpl::run(Function &F) {
  // Find the maximum LIC ID in the function.
  Function *LicInit = Intrinsic::getDeclaration(F.getParent(),
      Intrinsic::csa_lower_lic_init);
  NextLicId = 0;
  for (auto Use : LicInit->users()) {
    if (auto II = dyn_cast<CallInst>(Use)) {
      if (II->getParent()->getParent() == &F) {
        ConstantInt *LicNum = dyn_cast<ConstantInt>(II->getArgOperand(0));
        if (!LicNum) continue;
        NextLicId = std::max((unsigned)LicNum->getZExtValue(), NextLicId);
      }
    }
  }

  bool Changed = false;
  calculateResourceUsage(F);

  // Try to construct wide loads first if possible: a wide load actually
  // decreases the resource consumption compared to two regular loads.
  if (EnableWideLoads) {
    for (auto &BB : F) {
      SmallVector<WideLoadDetails, 8> PossibleWideLoads;
      for (auto &I : BB) {
        if (auto LI = dyn_cast<LoadInst>(&I)) {
          auto DetailsWide = getLegalWideLoad(LI->getPointerOperand(), LI);
          if (!DetailsWide)
            continue;
          PossibleWideLoads.push_back(*DetailsWide);
        }
      }

      // Check for wide load possibilities.
      int NumWideLoads = PossibleWideLoads.size();
      for (int i = NumWideLoads - 1; i > 0; i--) {
        for (int j = i - 1; j >= 0; j--) {
          bool Successful = attemptWideLoad(PossibleWideLoads[j],
            PossibleWideLoads[i]);
          if (Successful) {
            Changed = true;
            PossibleWideLoads.erase(PossibleWideLoads.begin() + i);
            PossibleWideLoads.erase(PossibleWideLoads.begin() + j);
            i--;
            break;
          }
        }
      }
    }
  }

  // Go through loops in order from highest frequency to lowest frequency. This
  // lets us know when to stop running streaming memory conversion.
  SmallVector<Loop *, 4> Loops = LI.getLoopsInPreorder();
  std::sort(Loops.begin(), Loops.end(), [=](Loop *A, Loop *B) {
    return BFI.getBlockFreq(A->getHeader()) > BFI.getBlockFreq(B->getHeader());
  });

  for (auto &L : Loops) {
    Changed |= runOnLoop(L);
  }

  LLVM_DEBUG(dbgs() << "Estimated usage after conversion of resources to be: "
      << "\n  VCPF: " << NumToMem << "\n  VCPR: " << NumFromMem << "\n");
  return Changed;
}

bool CSAStreamingMemoryImpl::runOnLoop(Loop *L) {
  bool Changed = false;

  // Get the loop preheader, creating it if it doesn't exist.
  auto *Preheader = L->getLoopPreheader();
  if (!Preheader) {
    Preheader = InsertPreheaderForLoop(L, &DT, &LI, nullptr, false);
    Changed = true;
  }
  assert(Preheader && "How did we not create a preheader?");

  // Get the single exiting block. It's not necessarily an error if one doesn't
  // exist, but SCEV tends to fail to give us the results we need if there's
  // multiple exit blocks, so it's not worth continuing.
  auto *ExitingBlock = L->getExitingBlock();
  if (!ExitingBlock)
    return Changed;

  // Ditto for the latch block. Having one latch makes reasoning about execution
  // count of blocks much easier as well.
  auto *LatchBlock = L->getLoopLatch();
  if (!LatchBlock)
    return Changed;

  // Compute the execution count for the current loop. We need it for the length
  // parameter.
  const SCEV *BackedgeCount = SE.getBackedgeTakenCount(L);
  if (isa<SCEVCouldNotCompute>(BackedgeCount))
    return Changed;
  auto boundExpensive = [&](const SCEV *S) {
    return isExpensiveSCEV(SE, S);
  };
  if (SCEVExprContains(BackedgeCount, boundExpensive)) {
    LLVM_DEBUG(dbgs() << "Expensive execution count " << *BackedgeCount
        << " for loop " << *L);
    if (!EnableAllLoops)
      return Changed;
  }

  // Ensure that the count is an i64. If the backedge count is a really weird
  // size (like i128), give up on the loop.
  if (SE.getTypeSizeInBits(BackedgeCount->getType()) > 64) {
    LLVM_DEBUG(dbgs() << "Ignoring i128-based loop induction variable for loop "
        << *L);
    return Changed;
  }

  BackedgeCount = SE.getNoopOrZeroExtend(BackedgeCount,
      Type::getInt64Ty(SE.getContext()));

  // Ignore cases where the exiting block doesn't dominate the latch block. This
  // probably is unanalyzable via SCEV anyways, but it does ruin our easy notion
  // of when we execute the same number of times as the loop.
  if (!DT.dominates(ExitingBlock, LatchBlock))
    return Changed;

  // Get the exit block of the loop. Make sure it is dedicated to the loop.
  Changed |= formDedicatedExitBlocks(L, &DT, &LI, nullptr, false);
  auto *ExitBlock = L->getExitBlock();
  if (!ExitBlock)
    return Changed;

  // If the inner loop is to be pipelined, do not attempt to convert streaming
  // memory references inside of the loop.
  if (isPipelinedLoop(L)) {
    LLVM_DEBUG(dbgs() << "Ignoring ILPL-based loop " << *L);
    return Changed;
  }

  // If any nested loops are pipelined without order being restored (i.e., not
  // using a completion buffer), then do not add streaming ops to the outer
  // loop.
  if (! ILPLUseCompletionBuf) {
    for (Loop *childLoop : L->getSubLoops()) {
      if (isPipelinedLoop(childLoop)) {
        LLVM_DEBUG(dbgs() << "Ignoring loop containing ILPL-based loop " << *L);
        return Changed;
      }
    }
  }

  LLVM_DEBUG(dbgs() << "Searching for opportunities in " << *L);
  LLVM_DEBUG(dbgs() << "Backedge count is " << *BackedgeCount << "\n");

  IRBuilder<> Builder(Preheader);
  SmallVector<StreamingMemoryDetails, 8> PossibleStreams;

  for (auto *BB : L->blocks()) {
    // Skip the block if it is in an inner loop.
    if (LI.getLoopFor(BB) != L)
      continue;

    // If we dominate the latch and are not in an inner loop, then we execute
    // this block the same number of times as the loop latch will exit, except
    // possibly for if we exit the loop first.
    if (!DT.dominates(BB, LatchBlock))
      continue;

    const SCEV *ExecCount = nullptr;
    if (DT.dominates(BB, ExitingBlock)) {
      ExecCount = SE.getAddExpr(BackedgeCount,
          SE.getOne(BackedgeCount->getType()));
    } else if (DT.dominates(ExitingBlock, BB)) {
      ExecCount = BackedgeCount;
    } else {
      assert(false &&
          "Domination results for BB, Exiting, and Latch do not make sense");
      continue;
    }

    ExecCounts.insert(std::make_pair(BB, ExecCount));
    LLVM_DEBUG(dbgs() << "Execution count for " << BB->getName() << " is " <<
        *ExecCount << "\n");

    for (auto &I : *BB) {
      if (auto SI = dyn_cast<StoreInst>(&I)) {
        auto Details = getLegalStream(SI->getPointerOperand(), SI);
        if (!Details)
          continue;

        PossibleStreams.push_back(*Details);
      } else if (auto LI = dyn_cast<LoadInst>(&I)) {
        auto Details = getLegalStream(LI->getPointerOperand(), LI);
        if (!Details)
          continue;

        PossibleStreams.push_back(*Details);
      }
    }
  }

  // Check for wide streaming load possibilities. Since wide loads use the same
  // (or fewer, in a few cases) number of units as two regular loads, this
  // should never increase demanded resources.
  int NumStreams = PossibleStreams.size();
  for (int i = NumStreams - 1; i > 0; i--) {
    for (int j = 0; j < i; j++) {
      bool Successful = attemptWide(PossibleStreams[i], PossibleStreams[j]);
      if (Successful) {
        Changed = true;
        PossibleStreams.erase(PossibleStreams.begin() + i);
        PossibleStreams.erase(PossibleStreams.begin() + j);
        i--;
        break;
      }
    }
  }

  // Convert the other values into regular streaming loads/stores.
  for (auto Details : PossibleStreams) {
    if (!EnableLargerStrides && Details.Stride != 1) {
      reportFailure(Details.MemInst, "stride is not constant 1");
      continue;
    }
    if (NumToMem >= NumVCPFUnits || NumFromMem >= NumVCPRUnits) {
      reportFailure(Details.MemInst,
        "not converting due to memory resource exhaustion");
      continue;
    }
    makeStreaming(Details);
    Changed = true;
  }

  // If we replaced some memory accesses, try to delete any PHI nodes in the
  // loop header.
  if (Changed) {
    BasicBlock *Header = L->getHeader();
    for (Instruction *I = &Header->front(), *Next = I->getNextNode();
        isa<PHINode>(I); I = Next, Next = Next->getNextNode()) {
      RecursivelyDeleteDeadPHINode(cast<PHINode>(I));
    }
  }
  return Changed;
}

static bool isInLoop(Value *V, Loop *L) {
  if (auto I = dyn_cast<Instruction>(V)) {
    if (L->contains(I->getParent()))
      return true;
  }
  return false;
}

static bool areMergeParamsOutsideLoop(Value *V, Loop *L) {
  if (auto I = dyn_cast<IntrinsicInst>(V)) {
    if (I->getIntrinsicID() == Intrinsic::csa_all0) {
      // If the intrinsic is outside the loop, we're cool.
      if (!L->contains(I->getParent()))
        return true;

      // Loop through all the arguments to see if they're inside the loop. They
      // could in theory be other all0 calls themselves, so we have to recurse.
      for (auto &Arg : I->arg_operands()) {
        if (!areMergeParamsOutsideLoop(Arg, L))
          return false;
      }
      return true;
    }
  }
  return !isInLoop(V, L);
}

static bool anyUseInLoop(Value *V, Loop *L) {
  for (auto Use : V->users()) {
    bool InLoop = isInLoop(Use, L);

    // If the use is an all0, check if the all0 itself is used inside of the
    // loop.
    auto II = dyn_cast<IntrinsicInst>(Use);
    if (InLoop && II && II->getIntrinsicID() == Intrinsic::csa_all0)
      InLoop = anyUseInLoop(II, L);

    if (InLoop)
      return true;
  }
  return false;
}

Optional<StreamingMemoryDetails> CSAStreamingMemoryImpl::getLegalStream(
    Value *Pointer, Instruction *MemInst) {

  // Disable streaming if the instruction is in a local cache.
  if (MemInst->hasMetadata(LocalCacheKey)) return None;

  const SCEV *S = SE.getSCEV(Pointer);
  Loop *L = LI.getLoopFor(MemInst->getParent());
  LLVM_DEBUG(dbgs() << "Candidate for streaming memory at " << *MemInst << "\n");
  // The SCEV must be an affine add-rec expr within the loop in question.
  const SCEVAddRecExpr *SAddRec = dyn_cast<SCEVAddRecExpr>(S);
  if (!SAddRec || !SAddRec->isAffine() || SAddRec->getLoop() != L)
    return None;
  LLVM_DEBUG(dbgs() << "Recurrence is " << *S << "\n");

  // Get the stride of the streaming memory reference.
  Type *MemTy = Pointer->getType()->getPointerElementType();
  int64_t StrideVal;
  const SCEV *StrideInBytes = SAddRec->getStepRecurrence(SE);
  if (auto StrideConst = dyn_cast<SCEVConstant>(StrideInBytes)) {
    unsigned PtrSize = SE.getDataLayout().getTypeStoreSize(MemTy);
    StrideVal = StrideConst->getValue()->getSExtValue();
    if (StrideVal % PtrSize) {
      reportFailure(MemInst, "stride is not a multiple of value size");
      return None;
    }
    StrideVal /= PtrSize;
  } else {
    reportFailure(MemInst, "stride is not a constant value");
    return None;
  }
  LLVM_DEBUG(dbgs() << "Stride is " << StrideVal << "\n");

  // Get the input and output ordering edges.
  CallInst *InOrd = getInordEdge(MemInst);
  if (InOrd && !areMergeParamsOutsideLoop(InOrd->getArgOperand(0), L)) {
    reportFailure(MemInst, "memory ordering tokens are not loop-invariant");
    LLVM_DEBUG(dbgs() << "Input ordering edge is inside the loop, aborting\n");
    return None;
  }

  CallInst *OutOrd = getOutordEdge(MemInst);
  if (OutOrd && anyUseInLoop(OutOrd, L)) {
    reportFailure(MemInst, "memory ordering tokens are not loop-invariant");
    LLVM_DEBUG(dbgs() << "Output ordering edge is inside the loop, aborting\n");
    return None;
  }

  StreamingMemoryDetails Details = { SAddRec->getStart(), StrideVal, InOrd,
    OutOrd, MemTy, MemInst };
  LLVM_DEBUG(errs() << "Adding to streaming memory list\n");
  return Details;
}

Optional<WideLoadDetails> CSAStreamingMemoryImpl::getLegalWideLoad(
    Value *Pointer, Instruction *MemInst) {

  // Disable streaming if the instruction is in a local cache.
  if (MemInst->hasMetadata(LocalCacheKey)) return None;

  const SCEV *S = SE.getSCEV(Pointer);
  LLVM_DEBUG(dbgs() << "Candidate for wide load at " << *MemInst << "\n");
  if (!S)
    return None;
  LLVM_DEBUG(dbgs() << "Address is " << *S << "\n");
  Type *MemTy = Pointer->getType()->getPointerElementType();
  // Get the input and output ordering edges.
  CallInst *InOrd = getInordEdge(MemInst);
  CallInst *OutOrd = getOutordEdge(MemInst);
  WideLoadDetails Details = { S, InOrd, OutOrd, MemTy, MemInst };
  LLVM_DEBUG(errs() << "Adding to wide load list\n");
  return Details;
}

CallInst *CSAStreamingMemoryImpl::getInordEdge(Instruction *MemInst) {
  if (auto II = dyn_cast<IntrinsicInst>(MemInst->getPrevNode())) {
    if (II->getIntrinsicID() == Intrinsic::csa_inord) {
      return II;
    }
  }

  return nullptr;
}

CallInst *CSAStreamingMemoryImpl::getOutordEdge(Instruction *MemInst) {
  // Check for an outord instruction existing after the memory instruction. It's
  // possible that some code (*cough*SCEVExpander*cough*) might insert some
  // extra instruction, such as a bitcast, immediately after the load,
  // preventing the outord from being immediately adjacent. However, we do want
  // to ensure that the chain operand goes from the memory instruction to the
  // outord instruction, so if another instruction creates a chain, stop
  // searching.
  for (auto NextInst = MemInst->getNextNode(); NextInst;
      NextInst = NextInst->getNextNode()) {
    if (auto II = dyn_cast<IntrinsicInst>(NextInst)) {
      if (II->getIntrinsicID() == Intrinsic::csa_outord) {
        return II;
      }
    }
    if (NextInst->mayReadOrWriteMemory())
      break;
  }

  return nullptr;
}

Value *CSAStreamingMemoryImpl::createLic(Type *LicType, Instruction *PushIP,
    Instruction *PopIP, const Twine &LicName, Value *Push) {
  // Get an insertion point for the LIC declaration.
  Instruction *IP = DT.findNearestCommonDominator(PushIP->getParent(),
      PopIP->getParent())->getFirstNonPHI();
  IRBuilder<> Builder(IP);

  // Create the LIC.
  unsigned LicSize = SE.getDataLayout().getTypeStoreSize(LicType);
  Value *Lic = Builder.getInt32(++NextLicId);
  Builder.CreateIntrinsic(Intrinsic::csa_lower_lic_init, {},
      { Lic, Builder.getInt8(LicSize),
        Builder.getInt64(0), Builder.getInt64(0) });

  // Insert the LIC push.
  Builder.SetInsertPoint(PushIP);
  Builder.CreateIntrinsic(Intrinsic::csa_lower_lic_write, { LicType },
      { Lic, Push });

  // Insert the LIC pop.
  Builder.SetInsertPoint(PopIP);
  Value *PoppedValue = Builder.CreateIntrinsic(Intrinsic::csa_lower_lic_read,
      { LicType }, { Lic }, nullptr, LicName + ".pop");

  return PoppedValue;
}

static void collectNonLoopLeaves(IntrinsicInst *All0, Loop *L,
    SmallVectorImpl<Value *> &Leaves) {
  for (auto &MergedValue : All0->arg_operands()) {
    auto I = dyn_cast<Instruction>(MergedValue);
    // This probably shouldn't ever be the case.
    if (!I) {
      assert(false && "All0 inputs should only be from instructions.");
      continue;
    }

    if (L->contains(I->getParent())) {
      auto II = dyn_cast<IntrinsicInst>(MergedValue);
      assert(II && II->getIntrinsicID() == Intrinsic::csa_all0 &&
          "Non-loop-invariant in the all0");
      collectNonLoopLeaves(II, L, Leaves);
    } else {
      Leaves.push_back(I);
    }
  }
}

static Value *makeMerge(Instruction *InsertPoint, ArrayRef<Value *> Values) {
  if (Values.size() == 1) {
    return Values[0];
  } else {
    Module *M = InsertPoint->getParent()->getParent()->getParent();
    Value *NewAll0 = CallInst::Create(
        Intrinsic::getDeclaration(M, Intrinsic::csa_all0),
        Values, "newmergeord", InsertPoint);
    LLVM_DEBUG(dbgs() << "Created new merge: " << *NewAll0 << "\n");
    return NewAll0;
  }
}

static void fixMergeUses(CallInst *InOrder, Loop *L) {
  Value *OrderEdge = InOrder->getArgOperand(0);
  auto II = dyn_cast<IntrinsicInst>(OrderEdge);
  if (!II || II->getIntrinsicID() != Intrinsic::csa_all0) {
    // This is not a call to all0. By the earlier checks, we should dominate the
    // loop header at this point, so we shouldn't have to do anything.
#ifndef NDEBUG
    if (auto I = dyn_cast<Instruction>(OrderEdge))
      assert(!L->contains(I->getParent()) &&
          "We should have caught this ordering violation earlier");
#endif
    return;
  }

  // Outside the loop, we're good.
  if (!L->contains(II->getParent()))
    return;

  // Generate a merge using all of the values from outside the loop. This is
  // equivalent to the original merge value.
  SmallVector<Value *, 4> NewMerge;
  collectNonLoopLeaves(II, L, NewMerge);
  II->replaceAllUsesWith(makeMerge(InOrder, NewMerge));
  II->eraseFromParent();
}

static void pushOutMerges(Value *OutOrd, Loop *L) {
  SmallVector<IntrinsicInst *, 4> MergeUses;
  for (auto Use : OutOrd->users()) {
    // The use is outside the loop, no need to fix it up.
    if (!isInLoop(Use, L)) {
      continue;
    }

    // If the use is inside the loop, then it should only be an all0.
    auto II = dyn_cast<IntrinsicInst>(Use);
    assert(II && II->getIntrinsicID() == Intrinsic::csa_all0 &&
        "We should not have any use where this is not the case");

    MergeUses.push_back(II);
  }

  for (auto II : MergeUses) {
    // Make all uses of this merge be outside the loop.
    pushOutMerges(II, L);

    // Merge all the other values in the loop as necessary.
    SmallVector<Value *, 4> LoopMergeParams;
    for (auto &MergedValue : II->arg_operands()) {
      if (MergedValue != OutOrd)
        LoopMergeParams.push_back(MergedValue);
    }
    Value *LoopMergedValue = makeMerge(II, LoopMergeParams);

    // Merge the new outorder with the loop-merged value in the exit block of
    // the loop.
    Value *NewMerge = makeMerge(L->getExitBlock()->getFirstNonPHI(),
        { LoopMergedValue, OutOrd });
    II->replaceAllUsesWith(NewMerge);
    II->eraseFromParent();
  }
}

void CSAStreamingMemoryImpl::makeStreaming(StreamingMemoryDetails &Details) {
  BasicBlock *BB = Details.MemInst->getParent();
  Loop *L = LI.getLoopFor(BB);
  const SCEV *ExecCount = ExecCounts[BB];

  // Generate the base and length values for the stream
  Instruction *DeloopedIP = L->getLoopPreheader()->getTerminator();
  IRBuilder<> Builder(DeloopedIP);
  Builder.SetCurrentDebugLocation(Details.MemInst->getDebugLoc());
  Value *Base = Expander.expandCodeFor(Details.Base,
      Details.MemTy->getPointerTo(), DeloopedIP);
  Value *Length = Expander.expandCodeFor(ExecCount, Builder.getInt64Ty(),
      DeloopedIP);

  Instruction *NewInst;
  Value *OldPointer;
  if (auto SI = dyn_cast<StoreInst>(Details.MemInst)) {
    OldPointer = SI->getPointerOperand();

    // Hook up the lic.
    Value *StoredValue = SI->getValueOperand();
    Value *StoreStream = createLic(Details.MemTy,
        SI, DeloopedIP, StoredValue->getName(), StoredValue);

    // Create the streaming store.
    NewInst = Builder.CreateIntrinsic(Intrinsic::csa_stream_store,
        { Details.MemTy },
        { StoreStream, Base, Length, Builder.getInt64(Details.Stride) });
  } else if (auto LI = dyn_cast<LoadInst>(Details.MemInst)) {
    OldPointer = LI->getPointerOperand();

    // Create the streaming load.
    NewInst = Builder.CreateIntrinsic(Intrinsic::csa_stream_load,
        { Details.MemTy },
        { Base, Length, Builder.getInt64(Details.Stride) },
        nullptr, LI->getName() + ".streamed");

    // Hook up the lic.
    Value *LoadResult = createLic(Details.MemTy,
        DeloopedIP, LI, LI->getName(), NewInst);
    LI->replaceAllUsesWith(LoadResult);
  } else {
    llvm_unreachable("Only loads and stores can be made into streaming memory");
    NewInst = nullptr;
  }

  NewInst->copyMetadata(*Details.MemInst, {LLVMContext::MD_nontemporal});

  LLVM_DEBUG(dbgs() << "Replaced " << *Details.MemInst << " with "
      << *NewInst << "\n");
  reportSuccess(Details.MemInst);

  // Adjust the ordering edges to the new instruction.
  if (Details.InOrd) {
    Details.InOrd->moveBefore(NewInst);
    fixMergeUses(Details.InOrd, L);
  }
  if (Details.OutOrd) {
    Details.OutOrd->moveAfter(NewInst);
    pushOutMerges(Details.OutOrd, L);
  }

  // Delete the old instruction.
  Details.MemInst->eraseFromParent();
  Details.MemInst = NewInst;

  // Clear out any dead stuff from the pointer.
  RecursivelyDeleteTriviallyDeadInstructions(OldPointer);

  // A new streaming load or streaming store increases the number of units used
  // by one if its size is not a small constant.
  NumToMem++;
}

bool CSAStreamingMemoryImpl::attemptWideLoad(WideLoadDetails &A,
    WideLoadDetails &B) {
  // Check that stride, length, and size are all equivalent.
  if (A.MemTy != B.MemTy)
    return false;
  // Are they both loads? (Only sldx2 is supported for now).
  if (!isa<LoadInst>(A.MemInst) || !isa<LoadInst>(B.MemInst))
    return false;

  // Find out if one is the base of the other.
  const SCEV *BaseDiff = SE.getMinusSCEV(A.Address, B.Address);
  const SCEVConstant *Constant = dyn_cast<SCEVConstant>(BaseDiff);
  int64_t PtrSize = SE.getDataLayout().getTypeStoreSize(A.MemTy);
  if (!Constant || abs(Constant->getValue()->getSExtValue()) != PtrSize)
    return false;

  bool IsALess = Constant->getValue()->isNegative();
  WideLoadDetails &Lo = IsALess ? A : B;
  WideLoadDetails &Hi = IsALess ? B : A;

  {
    OptimizationRemarkAnalysis RLo(DEBUG_TYPE, "StreamingMemory", Lo.MemInst);
    ORE.emit(RLo << "found candidate for wide load");
    OptimizationRemarkAnalysis RHi(DEBUG_TYPE, "StreamingMemory", Hi.MemInst);
    ORE.emit(RHi << "will be paired with this load");
  }

  // Compute if the input ordering edges are compatible. For compatible, we're
  // saying that they must both be %ign or both be the same value. Since these
  // values should end up in the same in alias set anyways, memory ordering is
  // not likely to create cases where there is a mismatch. Merging the values
  // if they don't match could well kill any performance gains of streaming
  // anyways.
  if (A.InOrd && B.InOrd) {
    if (A.InOrd->getArgOperand(0) != B.InOrd->getArgOperand(0)) {
      reportFailure(Lo.MemInst, "memory ordering tokens are not compatible");
      return false;
    }
  } else if (A.InOrd || B.InOrd) {
    reportFailure(Lo.MemInst, "memory ordering tokens are not compatible");
    return false;
  }

  // Compare output ordering edges for compatibility. Essentially the same rules
  // as above apply, although comparing uses for equivalency is more complex.
  if (A.OutOrd && B.OutOrd) {
    if (A.OutOrd->getNumUses() != B.OutOrd->getNumUses()) {
      reportFailure(Lo.MemInst, "memory ordering tokens are not compatible");
      return false;
    }
    auto UserCmp = [](const Use &U1, const Use &U2) {
      return (std::less<const User*>{})(U1.getUser(), U2.getUser());
    };
    A.OutOrd->sortUseList(UserCmp);
    B.OutOrd->sortUseList(UserCmp);
    if (!std::equal(A.OutOrd->user_begin(), A.OutOrd->user_end(),
                    B.OutOrd->user_begin())) {
      reportFailure(Lo.MemInst, "memory ordering tokens are not compatible");
      return false;
    }
  } else if (A.OutOrd || B.OutOrd) {
    reportFailure(Lo.MemInst, "memory ordering tokens are not compatible");
    return false;
  }

  // We have cleared all the checks. We can now create the operation.
  LLVM_DEBUG(dbgs() << "Found wide load\n");
  LLVM_DEBUG(dbgs() << "Lo: " << *Lo.MemInst << "\n");
  LLVM_DEBUG(dbgs() << "Hi: " << *Hi.MemInst << "\n");

  // Generate the base and length values for the stream
  IRBuilder<> Builder(Lo.MemInst);
  // Set the length to 2
  Value *Base = Expander.expandCodeFor(Lo.Address,
      Lo.MemTy->getPointerTo(), Lo.MemInst);
  Value *Length = Builder.getInt64(2);

  // Construct the wide streaming load.
  auto LI = dyn_cast<LoadInst>(Lo.MemInst);
  Value *OldLoPointer = LI->getPointerOperand();
  Value *OldHiPointer = dyn_cast<LoadInst>(Hi.MemInst)->getPointerOperand();
  Instruction *NewInst = Builder.CreateIntrinsic(Intrinsic::csa_stream_load_x2,
      { Lo.MemTy },
      { Base, Length, Builder.getInt64(1) },
      nullptr, LI->getName() + ".wideload");

  // Hook up lics for lo and hi.
  Value *LoResult = Builder.CreateExtractValue(NewInst, 0, Lo.MemInst->getName());
  Value *HiResult = Builder.CreateExtractValue(NewInst, 1, Hi.MemInst->getName());
  Lo.MemInst->replaceAllUsesWith(LoResult);
  Hi.MemInst->replaceAllUsesWith(HiResult);

  LLVM_DEBUG(dbgs() << "Replaced loads with " << *NewInst << "\n");
  reportSuccess(Lo.MemInst);

  // Adjust the ordering edges to the new instruction.
  if (Lo.InOrd) {
    Lo.InOrd->moveBefore(NewInst);
    Hi.InOrd->eraseFromParent();
  }
  if (Lo.OutOrd) {
    Lo.OutOrd->moveAfter(NewInst);
    Hi.OutOrd->replaceAllUsesWith(Lo.OutOrd);
    Hi.OutOrd->eraseFromParent();
  }

  // Delete the old instructions.
  Lo.MemInst->eraseFromParent();
  Lo.MemInst = NewInst;
  Hi.MemInst->eraseFromParent();
  Hi.MemInst = nullptr;

  // Clear out any dead stuff from the pointer.
  RecursivelyDeleteTriviallyDeadInstructions(OldLoPointer);
  RecursivelyDeleteTriviallyDeadInstructions(OldHiPointer);

  // A conversion to wide load scavenges one resource
  NumToMem--;

  return true;
}

bool CSAStreamingMemoryImpl::attemptWide(StreamingMemoryDetails &A,
    StreamingMemoryDetails &B) {
  // Check that stride, length, and size are all equivalent.
  if (A.Stride != B.Stride || (A.Stride & 1) != 0 || A.MemTy != B.MemTy ||
      ExecCounts[A.MemInst->getParent()] != ExecCounts[B.MemInst->getParent()])
    return false;

  // Are they both loads? (Only sldx2 is supported for now).
  if (!isa<LoadInst>(A.MemInst) || !isa<LoadInst>(B.MemInst))
    return false;

  // Find out if one is the base of the other.
  const SCEV *BaseDiff = SE.getMinusSCEV(A.Base, B.Base);
  const SCEVConstant *Constant = dyn_cast<SCEVConstant>(BaseDiff);
  int64_t PtrSize = SE.getDataLayout().getTypeStoreSize(A.MemTy);
  if (!Constant || abs(Constant->getValue()->getSExtValue()) != PtrSize)
    return false;

  bool IsALess = Constant->getValue()->isNegative();
  StreamingMemoryDetails &Lo = IsALess ? A : B;
  StreamingMemoryDetails &Hi = IsALess ? B : A;

  {
    OptimizationRemarkAnalysis RLo(DEBUG_TYPE, "StreamingMemory", Lo.MemInst);
    ORE.emit(RLo << "found candidate for wide streaming load");
    OptimizationRemarkAnalysis RHi(DEBUG_TYPE, "StreamingMemory", Hi.MemInst);
    ORE.emit(RHi << "will be paired with this load");
  }

  // Compute if the input ordering edges are compatible. For compatible, we're
  // saying that they must both be %ign or both be the same value. Since these
  // values should end up in the same in alias set anyways, memory ordering is
  // not likely to create cases where there is a mismatch. Merging the values
  // if they don't match could well kill any performance gains of streaming
  // anyways.
  if (A.InOrd && B.InOrd) {
    if (A.InOrd->getArgOperand(0) != B.InOrd->getArgOperand(0)) {
      reportFailure(Lo.MemInst, "memory ordering tokens are not compatible");
      return false;
    }
  } else if (A.InOrd || B.InOrd) {
    reportFailure(Lo.MemInst, "memory ordering tokens are not compatible");
    return false;
  }

  // Compare output ordering edges for compatibility. Essentially the same rules
  // as above apply, although comparing uses for equivalency is more complex.
  if (A.OutOrd && B.OutOrd) {
    if (A.OutOrd->getNumUses() != B.OutOrd->getNumUses()) {
      reportFailure(Lo.MemInst, "memory ordering tokens are not compatible");
      return false;
    }
    auto UserCmp = [](const Use &U1, const Use &U2) {
      return (std::less<const User*>{})(U1.getUser(), U2.getUser());
    };
    A.OutOrd->sortUseList(UserCmp);
    B.OutOrd->sortUseList(UserCmp);
    if (!std::equal(A.OutOrd->user_begin(), A.OutOrd->user_end(),
                    B.OutOrd->user_begin())) {
      reportFailure(Lo.MemInst, "memory ordering tokens are not compatible");
      return false;
    }
  } else if (A.OutOrd || B.OutOrd) {
    reportFailure(Lo.MemInst, "memory ordering tokens are not compatible");
    return false;
  }

  // Check if we would generate a non-1 stride operation.
  if (!EnableLargerStrides && Lo.Stride != 2) {
    reportFailure(Lo.MemInst, "stride is not constant 1");
    return false;
  }

  // We have cleared all the checks. We can now create the operation.
  LLVM_DEBUG(dbgs() << "Found wide streaming load\n");
  LLVM_DEBUG(dbgs() << "Lo: " << *Lo.MemInst << "\n");
  LLVM_DEBUG(dbgs() << "Hi: " << *Hi.MemInst << "\n");

  BasicBlock *BB = Lo.MemInst->getParent();
  Loop *L = LI.getLoopFor(BB);
  const SCEV *ExecCount = ExecCounts[BB];

  // Double the length for each of the inputs.
  ExecCount = SE.getAddExpr(ExecCount, ExecCount);

  // Generate the base and length values for the stream
  Instruction *DeloopedIP = L->getLoopPreheader()->getTerminator();
  IRBuilder<> Builder(DeloopedIP);
  Builder.SetCurrentDebugLocation(DILocation::getMergedLocation(
    Lo.MemInst->getDebugLoc().get(),
    Hi.MemInst->getDebugLoc().get()));
  Value *Base = Expander.expandCodeFor(Lo.Base,
      Lo.MemTy->getPointerTo(), DeloopedIP);
  Value *Length = Expander.expandCodeFor(ExecCount, Builder.getInt64Ty(),
      DeloopedIP);

  // Construct the wide streaming load.
  auto LI = dyn_cast<LoadInst>(Lo.MemInst);
  Value *OldLoPointer = LI->getPointerOperand();
  Value *OldHiPointer = dyn_cast<LoadInst>(Hi.MemInst)->getPointerOperand();
  Instruction *NewInst = Builder.CreateIntrinsic(Intrinsic::csa_stream_load_x2,
      { Lo.MemTy },
      { Base, Length, Builder.getInt64(Lo.Stride / 2) },
      nullptr, LI->getName() + ".streamed");

  // Hook up lics for lo and hi.
  Value *LoResult = createLic(Lo.MemTy, DeloopedIP, LI, Lo.MemInst->getName(),
      Builder.CreateExtractValue(NewInst, 0, Lo.MemInst->getName()));
  Value *HiResult = createLic(Lo.MemTy, DeloopedIP, LI, Hi.MemInst->getName(),
      Builder.CreateExtractValue(NewInst, 1, Hi.MemInst->getName()));
  Lo.MemInst->replaceAllUsesWith(LoResult);
  Hi.MemInst->replaceAllUsesWith(HiResult);

  LLVM_DEBUG(dbgs() << "Replaced loads with " << *NewInst << "\n");
  reportSuccess(Lo.MemInst);

  // Adjust the ordering edges to the new instruction.
  if (Lo.InOrd) {
    Lo.InOrd->moveBefore(NewInst);
    Hi.InOrd->eraseFromParent();
    fixMergeUses(Lo.InOrd, L);
  }
  if (Lo.OutOrd) {
    Lo.OutOrd->moveAfter(NewInst);
    Hi.OutOrd->replaceAllUsesWith(Lo.OutOrd);
    Hi.OutOrd->eraseFromParent();
    pushOutMerges(Lo.OutOrd, L);
  }

  // Delete the old instructions.
  Lo.MemInst->eraseFromParent();
  Lo.MemInst = NewInst;
  Hi.MemInst->eraseFromParent();
  Hi.MemInst = nullptr;

  // Clear out any dead stuff from the pointer.
  RecursivelyDeleteTriviallyDeadInstructions(OldLoPointer);
  RecursivelyDeleteTriviallyDeadInstructions(OldHiPointer);
  return true;
}

bool CSAStreamingMemoryImpl::isPipelinedLoop(Loop *L) {
  // We use a marker intrinsic in the loop header to identify pipelining, even
  // if automatic pipelining is enabled.
  for (auto &I : *L->getHeader()) {
    if (auto II = dyn_cast<IntrinsicInst>(&I)) {
      if (II->getIntrinsicID() == Intrinsic::csa_pipelineable_loop_marker)
        return true;
    }
  }

  return false;
}

void CSAStreamingMemoryImpl::calculateResourceUsage(Function &F) {
  NumToMem = NumFromMem = 0;
  // Each parameter and return result counts for resource usage as well.
  FunctionType *Ty = F.getFunctionType();
  if (!Ty->getReturnType()->isVoidTy()) {
    // XXX: struct return ty ABI
    NumToMem += 1;
  }
  // XXX: account for abi issues here.
  NumFromMem += Ty->getNumParams();

  for (auto &BB : F) {
    for (auto &I : BB) {
      if (auto *LI = dyn_cast<LoadInst>(&I)) {
        // XXX: estimate number of loads better (i.e., i128 -> 2 loads)
        (void)LI;
        unsigned NumLoads = 1;
        NumToMem += NumLoads;
        NumFromMem += NumLoads;
      } else if (auto *SI = dyn_cast<StoreInst>(&I)) {
        (void)SI;
        unsigned NumStores = 1;
        NumToMem += 2 * NumStores;
      } else if (isa<AtomicCmpXchgInst>(&I) || isa<AtomicRMWInst>(&I)) {
        // No need to estimate actual number of atomics--all of the atomics
        // should be legalized by the time we run, so every instruction will
        // map to a single atomic in the backend.
        NumToMem += 2;
        NumFromMem += 1;
      } else if (auto *II = dyn_cast<IntrinsicInst>(&I)) {
        // XXX: small length for streaming loads/stores.
        switch (II->getIntrinsicID()) {
          case Intrinsic::csa_stream_load:
            NumToMem += 2;
            NumFromMem += 1;
            break;
          case Intrinsic::csa_stream_store:
            NumToMem += 3;
            break;
          case Intrinsic::csa_stream_load_x2:
            NumToMem += 2;
            NumFromMem += 2;
            break;
          case Intrinsic::prefetch:
            NumToMem += 1;
            break;
          default:
            break;
        }
      }
    }
  }

  LLVM_DEBUG(dbgs() << "Estimated usage before conversion of resources to be: "
      << "\n  VCPF: " << NumToMem << "\n  VCPR: " << NumFromMem << "\n");
}
