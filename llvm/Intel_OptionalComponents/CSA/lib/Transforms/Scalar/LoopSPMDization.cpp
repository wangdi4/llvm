//===--- LoopSPMDization.cpp - Loop SPMDization pass ------------*- C++ -*-===//
//
// Copyright (C) 2017-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//===----------------------------------------------------------------------===//
//
// This pass implements the Loop SPMDization transformation that distributes
// loop iterations and assigns them to multiple loops. These loops can run in
// parallel. Three approaches are implemented: the cyclic approach where each
// loop has an iteration stride of k, the blocking approach where each loop
// iterates over contiguous #iterations/NPEs iterations, and the hybrid approach
// where each loop iterates over a chunk size of iterations.
//
//===----------------------------------------------------------------------===//

#include "Intel_CSA/CSAIRPasses.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/ScalarEvolutionExpander.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Utils/UnrollLoop.h"

#include <vector>

using namespace llvm;
#define DEBUG_TYPE "csa-spmdization"

#define SPMD_CYCLIC 1
#define SPMD_BLOCKING 2
#define SPMD_HYBRID 3

// -mllvm -zero-trip-checks=nested|flat option is used to generation either
// nested zero checks (better for performance when trip counts are zeros but
// generate more integer operations) or flat checks that generates less integer
// operations

// Nested Example:
// if(n>0)
//   for(i=0; i<n; i+=k)
//     sum1++
//   if(n>1)
//     for(i=1; i<n; i+=k)
//       sum2++
//     if(n>2)
//       for(i=2; i<n; i+=k)
//         sum3++
// sum = sum1+sum2+sum3;

// Flat Example:
// if(n>0)
//   for(i=0; i<n; i+=k)
//     sum1++;
// if(n>1)
//   for(i=1; i<n; i+=k)
//     sum2++;
// if(n>2)
//   for(i=2; i<n; i+=k)
//     sum3++;
// Sum = sum1+sum2+sum3
enum ZTCMode { Nested = 0, Flat = 1 };
static cl::opt<ZTCMode> ZTCType(
    "spmd-zero-trip-checks", cl::Hidden,
    cl::desc("CSA Specific: Type of zero trip checks. 0 == Nested, 1 == Flat"),
    cl::values(
        clEnumValN(Nested, "nested",
                   "nested ifs for the zero trip checks for each spmd worker"),
        clEnumValN(Flat, "flat",
                   "flat ifs for the zero trip checks for each spmd worker "
                   "(default)")),
    cl::init(ZTCMode::Flat));

namespace {
class LoopSPMDization : public LoopPass {
  LLVMContext Context;

public:
  static char ID;
  LoopSPMDization() : LoopPass(ID) {
    initializeLoopSPMDizationPass(*PassRegistry::getPassRegistry());
  }

private:
  int next_token;
  unsigned spmd_approach;
  Value *steptimesk;
  Value *StepPE0;
  Value *NewInitV;
  Value *Cond;
  Value *nbyk;
  Value *UpperBound;
  Value *LowerBound;
  Value *TripCountV;
  bool fixReductionsIfAny(Loop *L, Loop *OrigL, BasicBlock *E,
                          BasicBlock *AfterLoop, int PE, int NPEs,
                          std::vector<Value *> &ReduceVarExitOrig,
                          std::vector<Instruction *> &ReduceVarOrig,
                          std::vector<Instruction *> &OldInst);
  bool fixReductionsFlatIfAny(Loop *L, Loop *OrigL, BasicBlock *E,
                              BasicBlock *AfterLoop, int PE, int NPEs,
                              std::vector<Value *> &ReduceVarExitOrig,
                              std::vector<Instruction *> &ReduceVarOrig,
                              std::vector<Instruction *> &OldInst);
  void lowerSPMDWorkerNum(Loop *L, int PE);
  void cleanSPMDWorkerNum(llvm::SmallVector<Loop *, 4> &toClean);
  void setLoopAlreadySPMDized(Loop *L);
  void AddUnrollDisableMetadata(Loop *L);
  bool FindReductionVariables(Loop *L, ScalarEvolution *SE,
                              std::vector<Value *> &ReduceVarExitOrig,
                              std::vector<Instruction *> &ReduceVarOrig);
  unsigned FindReductionVectorsSize(Loop *L);
  PHINode *getInductionVariable(Loop *L, ScalarEvolution *SE);
  bool TransformLoopInitandStep(Loop *L, ScalarEvolution *SE, int PE, int NPEs,
                                int Chunk_Size);
  bool TransformLoopInitandBound(Loop *L, ScalarEvolution *SE, int PE,
                                 int NPEs);
  void AddHybridLoopLevel(Loop *L, int Chunk_Size, PHINode *ClonedInductionPHI,
                          ScalarEvolution *SE, DominatorTree *DT, LoopInfo *LI);
  int GetMinLoopIterations(Loop *L, ScalarEvolution *SE);
  void AddZeroTripCountCheck(Loop *L, ScalarEvolution *SE, int PE, int NPEs,
                             BasicBlock *AfterLoop, DominatorTree *DT,
                             LoopInfo *LI);
  bool AddParallelIntrinsicstoLoop(Loop *L, LLVMContext &context, Module *M,
                                   BasicBlock *OrigPH, BasicBlock *E);
  IntrinsicInst *detectSPMDIntrinsic(Loop *L, LoopInfo *LI, DominatorTree *DT,
                                     PostDominatorTree *PDT, int &NPEs,
                                     int &Chunk_Size);

  bool runOnLoop(Loop *L, LPPassManager &) override {

    // Skip SPMDization if optnone is set; this makes it possible to use
    // things like OptBisect with SPMDization.
    if (skipLoop(L))
      return false;
    if (MDNode *LoopID = L->getLoopID())
      if (GetUnrollMetadata(LoopID, "llvm.loop.spmd.disable")) {
        return true;
      }
    LoopInfo *LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
    DominatorTree *DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
    PostDominatorTree *PDT =
        &getAnalysis<PostDominatorTreeWrapperPass>().getPostDomTree();
    ScalarEvolution *SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();

    LLVMContext &context = L->getHeader()->getContext();
    Function *F = L->getHeader()->getParent();
    Module *M = F->getParent();
    OptimizationRemarkEmitter ORE(F);

    ValueToValueMapTy VMap;
    BasicBlock *OrigPH = L->getLoopPreheader();
    Loop *OrigL = L;
    spmd_approach = 0;
    int NPEs;
    int Chunk_Size;
    IntrinsicInst *found_spmd =
        detectSPMDIntrinsic(L, LI, DT, PDT, NPEs, Chunk_Size);
    if (!found_spmd)
      return false;
    if (NPEs == 1) {
      // Add CSA parallel intrinsics:
      AddParallelIntrinsicstoLoop(L, context, M, OrigPH, L->getExitBlock());
      ORE.emit(
          OptimizationRemark(DEBUG_TYPE, "", L->getStartLoc(), L->getHeader())
          << "Number of workers is one, loop SPMDization is equivalent to "
             "__buitlin_csa_parallel_loop() in this "
             "case.");
      return false;
    }
    if (NPEs < 1) {
      errs() << "\n";
      errs().changeColor(raw_ostream::BLUE, true);
      errs() << "!! WARNING: BAD CSA SPMD INTRINSIC !!";
      errs().resetColor();
      errs() << " Number of workers should be a positive integer.\n"
                "This call will be ignored.\n\n";
      return false;
    }
    if (Chunk_Size == 0)
      spmd_approach = SPMD_BLOCKING;
    else if (Chunk_Size == 1)
      spmd_approach = SPMD_CYCLIC;
    else if (Chunk_Size > 1)
      spmd_approach = SPMD_HYBRID;
    else {
      errs() << "\n";
      errs().changeColor(raw_ostream::BLUE, true);
      errs() << "!! WARNING: BAD CSA SPMD INTRINSIC !!";
      errs().resetColor();
      errs() << " Second argument should be 0 for Blocked, 1 for Cyclic, "
                "or > 1 for Hybrid.\n"
                "This call will be ignored.\n\n";
      return false;
    }
    if (!L->getExitBlock()) {
      //   ORE.emit(
      //   OptimizationRemarkMissed(DEBUG_TYPE, "Unstructured Code",
      //   L->getStartLoc(), L->getHeader())
      //   << "Unable to perform loop SPMDization as directed by the pragma"
      //   "because loop body has unstructured code.");
      errs() << "\n";
      errs().changeColor(raw_ostream::BLUE, true);
      errs() << "!! WARNING: COULD NOT PERFORM SPMDization !!\n";
      errs().resetColor();
      errs() << R"help(The SPMDization loop body has unstructured code.

Branches to or from an OpenMP structured block are illegal

)help";
      return false;
    }
    unsigned r = FindReductionVectorsSize(L);
    std::vector<Value *> ReduceVarExitOrig(r);
    std::vector<Instruction *> ReduceVarOrig(r);
    // there is OldInst foreach reduction variable
    std::vector<Instruction *> OldInsts(r);
    if (!FindReductionVariables(L, SE, ReduceVarExitOrig, ReduceVarOrig))
      return false;
    // retrieve the minimum number of iterations of the loop in order to assess
    // whether to insert the zero trip count check or not
    int min_iterations = GetMinLoopIterations(L, SE);
    if (spmd_approach == SPMD_CYCLIC) {
      if (!TransformLoopInitandStep(L, SE, 0, NPEs, Chunk_Size)) {
        return false;
      }
    } else if (spmd_approach == SPMD_HYBRID) {
      // Save the loop iterator before it gets modified to be used for the
      // creation of the inner loop
      PHINode *InductionPHI = getInductionVariable(L, SE);
      if (!InductionPHI) {
        errs() << "\n";
        errs().changeColor(raw_ostream::BLUE, true);
        errs() << "!! WARNING: COULD NOT PERFORM SPMDization !!\n";
        errs().resetColor();
        errs() << R"help(
Failed to find the loop induction variable.

)help";
        LLVM_DEBUG(dbgs() << "Failed to find the loop induction variable \n");
        return false;
      }
      PHINode *ClonedInductionPHI = cast<PHINode>(InductionPHI->clone());

      if (!TransformLoopInitandStep(L, SE, 0, NPEs, Chunk_Size)) {
        return false;
      }
      // Create a loop inside L that goes from L iterator to
      // iterator+chunk_size
      AddHybridLoopLevel(L, Chunk_Size, ClonedInductionPHI, SE, DT, LI);
    } else if (spmd_approach == SPMD_BLOCKING) {
      const DataLayout &DL = L->getHeader()->getModule()->getDataLayout();
      SCEVExpander Expander(*SE, DL, "loop-SPMDization");
      BranchInst *PreHeaderBR =
          cast<BranchInst>(L->getLoopPreheader()->getTerminator());
      const SCEV *BECountSC = SE->getBackedgeTakenCount(L);

      // Sometimes SCEV can't figure out the backedge taken count; bail and
      // print a warning if that happens.
      if (isa<SCEVCouldNotCompute>(BECountSC)) {
        errs() << "\n";
        errs().changeColor(raw_ostream::BLUE, true);
        errs() << "!! WARNING: COULD NOT PERFORM SPMDization !!";
        errs().resetColor();
        errs() << R"help(

We were unable to determine an expression for the trip count of a loop for which
blocking SPMDization was requested. Please simplify the loop control logic or
try a different SPMDization strategy instead.

)help";
        return false;
      }

      const SCEV *TripCountSC =
          SE->getAddExpr(BECountSC, SE->getConstant(BECountSC->getType(), 1));
      TripCountV = Expander.expandCodeFor(TripCountSC, TripCountSC->getType(),
                                          PreHeaderBR);

      IRBuilder<> BPR(L->getLoopPreheader()->getTerminator());
      Value *nbykmightzero = BPR.CreateUDiv(
          TripCountV, ConstantInt::get(BECountSC->getType(), NPEs), ".nbyk");
      auto *IsZero = BPR.CreateICmpEQ(
          nbykmightzero, ConstantInt::get(BECountSC->getType(), 0));
      // If n by k is zero (there will be loops with zero trip count), each
      // loop will run at most one iteration
      nbyk = BPR.CreateSelect(IsZero, ConstantInt::get(BECountSC->getType(), 1),
                              nbykmightzero);

      TransformLoopInitandBound(L, SE, 0, NPEs);
    }

    setLoopAlreadySPMDized(L);

    BasicBlock *PH = SplitBlock(OrigPH, OrigPH->getTerminator(), DT, LI);
    PH->setName(L->getHeader()->getName() + ".ph");
    BasicBlock *OrigE = L->getExitBlock();

    Instruction *i = dyn_cast<Instruction>(OrigE->begin());
    BasicBlock *E = SplitBlock(OrigE, i, DT, LI);
    OrigE->setName(L->getHeader()->getName() + ".e");
    BasicBlock *AfterLoop = E;

    // Add CSA parallel intrinsics:
    AddParallelIntrinsicstoLoop(L, context, M, OrigPH, E);
    SmallVector<Value *, 128> NewReducedValues; // should be equal to NPEs
    SmallVector<Loop *, 4> toClean;
    toClean.push_back(OrigL);
    for (int PE = 1; PE < NPEs; PE++) {
      SmallVector<BasicBlock *, 8> NewLoopBlocks;
      BasicBlock *Exit = L->getExitBlock();
      // clone the exit block, to be attached to the cloned loop
      BasicBlock *NewE =
          CloneBasicBlock(Exit, VMap, ".PE" + std::to_string(PE), F);
      VMap[Exit] = NewE;

      Loop *NewLoop = cloneLoopWithPreheader(PH, OrigPH, L, VMap,
                                             ".PE" + std::to_string(PE), LI, DT,
                                             NewLoopBlocks);
      NewLoopBlocks.push_back(NewE);
      remapInstructionsInBlocks(NewLoopBlocks, VMap);
      // Update LoopInfo.
      if (OrigL->getParentLoop())
        OrigL->getParentLoop()->addBasicBlockToLoop(NewE, *LI);
      // Add DominatorTree node, update to correct IDom.
      BasicBlock *NLatch = NewLoop->getLoopLatch();
      BranchInst *NLatchBR = cast<BranchInst>(NLatch->getTerminator());
      // Calculate the immediate dominator of NewE
      if (NLatchBR->isConditional())
        DT->addNewBlock(NewE, NewLoop->getLoopLatch());
      else
        DT->addNewBlock(NewE, NewLoop->getHeader());

      // If __builtin_csa_spmd_worker_num is used in the loop
      // lower this to a constant (+1)
      lowerSPMDWorkerNum(NewLoop, PE);

      Instruction *ExitTerm = Exit->getTerminator();
      BranchInst::Create(NewLoop->getLoopPreheader(), Exit);
      ExitTerm->eraseFromParent();
      if (spmd_approach == SPMD_CYCLIC)
        TransformLoopInitandStep(NewLoop, SE, 1, NPEs, Chunk_Size);
      else if (spmd_approach == SPMD_HYBRID) {
        TransformLoopInitandStep(NewLoop, SE, 1, NPEs, Chunk_Size);
      } else if (spmd_approach == SPMD_BLOCKING)
        TransformLoopInitandBound(NewLoop, SE, PE, NPEs);

      if (min_iterations - 1 < PE) {
        AddZeroTripCountCheck(NewLoop, SE, PE, NPEs, AfterLoop, DT, LI);
        // flat vs. nested: The main difference between flat and nested here is
        // that the loop preheader (the zero trip check block) should branch to
        // the loop exit block (flat) rather than afterloop (nested)
        if (ZTCType == ZTCMode::Flat) {
          BasicBlock *ZTTBlock = L->getExitBlock()->getSingleSuccessor();
          assert(ZTTBlock && "ZTTBlock, the zero trip check BB is unique.");
          BranchInst *ZTTBR = cast<BranchInst>(ZTTBlock->getTerminator());
          if (ZTTBR->getSuccessor(0) == AfterLoop)
            ZTTBR->setSuccessor(0, NewLoop->getExitBlock());
          else
            ZTTBR->setSuccessor(1, NewLoop->getExitBlock());
        }
      } else {
        ORE.emit(
            OptimizationRemark(DEBUG_TYPE, "", L->getStartLoc(), L->getHeader())
            << "Loop SPMDization zero trip count check was not inserted for "
               "Worker# " +
                   std::to_string(PE) +
                   "  ,as directed by the builtin_assume or the constant value "
                   "of the upper bound of the loop.");
        AssumptionCache *AC =
            &getAnalysis<AssumptionCacheTracker>().getAssumptionCache(*F);
        Instruction *const terminator =
            NewLoop->getLoopPreheader()->getTerminator();
        IRBuilder<> Builder{terminator};
        Instruction *CondI = dyn_cast<Instruction>(Cond);
        auto *assume = Builder.CreateICmpSLT(NewInitV, CondI->getOperand(1));
        CallInst *workerAssume =
            Builder.CreateIntrinsic(Intrinsic::assume, {}, assume);
        AC->registerAssumption(workerAssume);
      }
      // This assumes menable-unsafe-fp-math is set
      // flat vs. nested: there are multiple small differences on how to handle
      // reductions so instead of having multiple cases in just one function, I
      // created two different functions that have some code duplications but
      // avoid confusion and bugs. Moreover, I suspect in the future, nested
      // generation will be gone, so i will just delete the other function
      bool success_p;
      if (ZTCType == ZTCMode::Nested)
        success_p =
            fixReductionsIfAny(NewLoop, OrigL, E, AfterLoop, PE, NPEs,
                               ReduceVarExitOrig, ReduceVarOrig, OldInsts);
      else //(ZTCType == ZTCMode::Flat)
        success_p =
            fixReductionsFlatIfAny(NewLoop, OrigL, E, AfterLoop, PE, NPEs,
                                   ReduceVarExitOrig, ReduceVarOrig, OldInsts);
      if (!success_p)
        return false;
      L = NewLoop;
      toClean.push_back(L);
      // NOTE: we do not need to call setLoopAlreadySPMDized()
      // for the new loop, because the metadata was set previously
      // for the original loop and it will be copied to each new loop
      // automatically, by cloneLoopWithPreheader().
    }

    if (ZTCType == ZTCMode::Nested) {
      // Fix missed Phi operands in AfterLoop
      BasicBlock::iterator bi, bie;
      for (bi = AfterLoop->begin(), bie = AfterLoop->end(); (bi != bie); ++bi) {
        PHINode *RedPhi = dyn_cast<PHINode>(&*bi);
        if (!RedPhi)
          continue;
        Value *RedV;
        if (RedPhi->getBasicBlockIndex(L->getExitBlock()) == -1)
          // Afterloop did not have a phi node
          RedPhi->setIncomingBlock(0, L->getExitBlock());

        RedV = RedPhi->getIncomingValueForBlock(L->getExitBlock());
        for (auto it = pred_begin(AfterLoop), et = pred_end(AfterLoop);
             it != et; ++it) {
          BasicBlock *predecessor = *it;
          if (RedPhi->getBasicBlockIndex(predecessor) == -1) {
            RedPhi->addIncoming(RedV, predecessor);
          }
        }
      }
    }
    cleanSPMDWorkerNum(toClean);
    ORE.emit(
        OptimizationRemark(DEBUG_TYPE, "", L->getStartLoc(), L->getHeader())
        << "Performed loop SPMDization as directed by the pragma.");
    return true;
  }
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    // getLoopAnalysisUsage(AU);
    AU.addRequired<AssumptionCacheTracker>();
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<PostDominatorTreeWrapperPass>();
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequired<ScalarEvolutionWrapperPass>();
    AU.addRequired<AAResultsWrapperPass>();
    AU.addRequiredID(LoopSimplifyID);
    AU.addRequiredID(LCSSAID);
  }
};
} // namespace

char LoopSPMDization::ID = 0;

INITIALIZE_PASS_BEGIN(LoopSPMDization, DEBUG_TYPE, "Loop SPMDization", false,
                      false)
INITIALIZE_PASS_DEPENDENCY(AssumptionCacheTracker)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopAccessLegacyAnalysis)
INITIALIZE_PASS_DEPENDENCY(LoopPass)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(PostDominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolutionWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopSimplify)
INITIALIZE_PASS_END(LoopSPMDization, DEBUG_TYPE, "Loop SPMDization", false,
                    false)

Pass *llvm::createLoopSPMDizationPass() { return new LoopSPMDization(); }

// Once the replacement of csa_spmd_worker_num() with csa_spmd_worker_num()+1 is
// done in LoopSPMDization::lowerSPMDWorkerNum, we finally replace the
// intrinsicwith zero as the +ones will generate the corresponding worker id
// num. If SPMDization is not performed for whatever reason, we should still
// remove these intrinsics. This is done in CSAIntrinsicCleaner.cpp
// (CSAIntrinsicCleaner::clean_spmd_worker_num)
void LoopSPMDization::cleanSPMDWorkerNum(
    llvm::SmallVector<Loop *, 4> &toClean) {
  SmallVector<Instruction *, 4> toDelete;
  for (Loop *L : toClean) {
    for (BasicBlock *const BB : L->getBlocks())
      for (Instruction &inst_it : *BB) {
        IntrinsicInst *const intr_inst = dyn_cast<IntrinsicInst>(&inst_it);
        if (intr_inst and
            intr_inst->getIntrinsicID() == Intrinsic::csa_spmd_worker_num) {
          Value *pe = llvm::ConstantInt::get(intr_inst->getType(), 0);
          inst_it.replaceAllUsesWith(pe);
          toDelete.push_back(&inst_it);
        }
      }
  }
  for (auto I : toDelete)
    I->eraseFromParent();
  return;
}

// Each SPMD loop PE is cloned from the precedent loop PE-1
// that's why a replacement of the csa_spmd_worker_num with +1
// is needed
void LoopSPMDization::lowerSPMDWorkerNum(Loop *L, int PE) {
  LLVMContext &Context = L->getHeader()->getContext();
  for (BasicBlock *const BB : L->getBlocks()) {
    for (Instruction &inst : *BB)
      if (IntrinsicInst *intr_inst = dyn_cast<IntrinsicInst>(&inst))
        if (intr_inst->getIntrinsicID() == Intrinsic::csa_spmd_worker_num) {
          Value *pe =
              llvm::ConstantInt::get(llvm::Type::getInt32Ty(Context), 1);
          IRBuilder<> Bnum(inst.getNextNode());
          Value *Num = Bnum.CreateAdd(intr_inst, pe, L->getName() + ".plusone");
          // Replace users of the intrinsic with Num
          for (auto UA = intr_inst->user_begin(), EA = intr_inst->user_end();
               UA != EA;) {
            Instruction *User = cast<Instruction>(*UA++);
            if (User != Num)
              for (unsigned m = 0; m < User->getNumOperands(); m++)
                if (User->getOperand(m) == dyn_cast<Value>(intr_inst)) {
                  User->setOperand(m, Num);
                }
          }
        }
  }
  return;
}

void LoopSPMDization::setLoopAlreadySPMDized(Loop *L) {
  // Add SPMDization(disable) metadata to disable future SPMDization.
  SmallVector<Metadata *, 4> MDs;
  // Reserve first location for self reference to the LoopID metadata node.
  MDs.push_back(nullptr);

  LLVMContext &Context = L->getHeader()->getContext();
  SmallVector<Metadata *, 1> DisableOperands;
  DisableOperands.push_back(MDString::get(Context, "llvm.loop.spmd.disable"));
  MDNode *DisableNode = MDNode::get(Context, DisableOperands);
  MDs.push_back(DisableNode);

  if (MDNode *LoopID = L->getLoopID()) {
    // Preserve metadata of the original loop.
    for (unsigned i = 1; i < LoopID->getNumOperands(); ++i) {
      MDNode *Node = cast<MDNode>(LoopID->getOperand(i));
      MDs.push_back(Node);
    }
  }

  MDNode *NewLoopID = MDNode::get(Context, MDs);
  // Set operand 0 to refer to the loop id itself.
  NewLoopID->replaceOperandWith(0, NewLoopID);
  L->setLoopID(NewLoopID);
  return;
}

void LoopSPMDization::AddUnrollDisableMetadata(Loop *L) {
  SmallVector<Metadata *, 4> MDs;
  // Reserve first location for self reference to the LoopID metadata node.
  MDs.push_back(nullptr);

  LLVMContext &Context = L->getHeader()->getContext();
  SmallVector<Metadata *, 1> DisableOperands;
  DisableOperands.push_back(MDString::get(Context, "llvm.loop.unroll.disable"));
  MDNode *DisableNode = MDNode::get(Context, DisableOperands);
  MDs.push_back(DisableNode);

  MDNode *NewLoopID = MDNode::get(Context, MDs);
  // Set operand 0 to refer to the loop id itself.
  NewLoopID->replaceOperandWith(0, NewLoopID);
  L->setLoopID(NewLoopID);
  return;
}

unsigned LoopSPMDization::FindReductionVectorsSize(Loop *L) {
  unsigned r = 0;
  for (Instruction &I : *L->getHeader()) {
    if (isa<PHINode>(&I))
      r++;
  }
  return r;
}

bool LoopSPMDization::FindReductionVariables(
    Loop *L, ScalarEvolution *SE, std::vector<Value *> &ReduceVarExitOrig,
    std::vector<Instruction *> &ReduceVarOrig) {
  Function *F = L->getHeader()->getParent();
  OptimizationRemarkEmitter ORE(F);
  unsigned r = 0;
  for (Instruction &I : *L->getHeader()) {
    PHINode *Phi = dyn_cast<PHINode>(&I);
    if (!Phi)
      continue;
    RecurrenceDescriptor RedDes;
    if (RecurrenceDescriptor::isReductionPHI(Phi, L, RedDes) ||
        RecurrenceDescriptor::AddReductionVar(
            Phi, RecurrenceDescriptor::RecurrenceKind::RK_FloatMinMax, L, true,
            RedDes)) {
      Value *ReduceVar;
      PHINode *Phiop = Phi;
      PHINode *redoperation;
      if (Phi->getIncomingBlock(0) == L->getLoopPreheader()) {
        ReduceVar = Phi->getIncomingValue(1);
        redoperation = dyn_cast<PHINode>(Phiop->getIncomingValue(1));
        ReduceVarOrig[r] = dyn_cast<Instruction>(Phiop->getIncomingValue(1));
      } else {
        ReduceVar = Phi->getIncomingValue(0);
        redoperation = dyn_cast<PHINode>(Phiop->getIncomingValue(0));
        ReduceVarOrig[r] = dyn_cast<Instruction>(Phiop->getIncomingValue(0));
      }
      while (redoperation) {
        Phiop = redoperation;
        // We could choose 0 or 1 values but we test both to avoid cyclic Phis
        redoperation = dyn_cast<PHINode>(
            dyn_cast<Instruction>(Phiop->getIncomingValue(0)));
        if (redoperation) {
          redoperation = dyn_cast<PHINode>(
              dyn_cast<Instruction>(Phiop->getIncomingValue(1)));
          ReduceVarOrig[r] = dyn_cast<Instruction>(Phiop->getIncomingValue(1));
        } else
          ReduceVarOrig[r] = dyn_cast<Instruction>(Phiop->getIncomingValue(0));
      }
      BasicBlock::iterator i, ie;
      for (i = L->getExitBlock()->begin(), ie = L->getExitBlock()->end();
           (i != ie); ++i) {
        PHINode *PhiExit = dyn_cast<PHINode>(&*i);
        if (!PhiExit)
          continue;
        Instruction *ReduceVarExit =
            cast<Instruction>(PhiExit->getIncomingValue(0));
        if (ReduceVarExit == ReduceVar)
          ReduceVarExitOrig[r] = dyn_cast<Value>(PhiExit);
      }
    } else {
      // OptimizationRemark is not able to detect phi location
      // si we have to print the instruction that uses a phi
      // this might be a reduction that is not suported
      // or an operation that involves a loop carry dependency
      InductionDescriptor ID;
      if (!InductionDescriptor::isInductionPHI(Phi, L, SE, ID)) {
        errs() << "\n";
        errs().changeColor(raw_ostream::BLUE, true);
        errs() << "!! ERROR: COULD NOT PERFORM SPMDization !!\n";
        errs().resetColor();
        errs() << " Detected unsupported loop carried value in a loop marked "
                  "for SPMDization;"
               << " please remove the SPMDization marking"
               << " or, if this is a reduction, add –mllvm "
                  "–csa-omp-paropt-loop-splitting"
               << " along with the OpenMP reduction clause to detect more "
                  "reduction patterns \n\n";
        Instruction *op = cast<Instruction>(Phi);
        for (auto UA = Phi->user_begin(), EA = Phi->user_end(); UA != EA;) {
          op = cast<Instruction>(*UA++);
        }
        ORE.emit(OptimizationRemark(DEBUG_TYPE, "SPMD Reductions:", op)
                 << "The unsupported loop carried value in a loop marked"
                 << " for SPMDization is");
        return false;
      }
    }
    r++; // count also the non reduction phis
  }
  return true;
}

// Calculate the identity element of the reduction operation
// TODO: make it a more exhaustive set
Value *findReductionIdentity(PHINode *Phi, Instruction *Op) {
  Value *Ident;
  Type *Ty = Phi->getType();
  switch (Op->getOpcode()) {
  case Instruction::Add:
  case Instruction::FAdd:
  case Instruction::Sub:
  case Instruction::FSub: {
    Ident = Constant::getNullValue(Ty);
    break;
  }
  case Instruction::Or:
  case Instruction::Xor: {
    Ident = Constant::getNullValue(Ty);
    break;
  }
  case Instruction::Mul: {
    Ident = ConstantInt::get(Ty, 1);
    break;
  }
  case Instruction::FMul: {
    Ident = ConstantFP::get(Ty, 1);
    break;
  }
  case Instruction::And: {
    Ident = Constant::getAllOnesValue(Ty);
    break;
  }
  case Instruction::Select: {
    CmpInst *CmpInstruction;
    CmpInstruction = dyn_cast<CmpInst>(Op->getOperand(0));
    if (!CmpInstruction) {

      errs().changeColor(raw_ostream::GREEN, true);
      errs() << "!! ERROR: INCORRECT REDUCTION IDENTIFIED  !!\n";
      errs().resetColor();
      errs() << R"help(
                  Failed to get cmp instruction for min/max reduction.
                  )help";
      break;
    }

    unsigned int BitSize;
    BitSize = Ty->getPrimitiveSizeInBits();

    switch (CmpInstruction->getPredicate()) {
    case ICmpInst::ICMP_SGE:
    case ICmpInst::ICMP_SGT: {
      // Signed Int Max Reduction
      if (BitSize == 32) {
        Ident = ConstantInt::get(Ty, std::numeric_limits<int32_t>::min(), 1);
      } else if (BitSize == 16) { // Signed Short Int
        Ident = ConstantInt::get(Ty, std::numeric_limits<int16_t>::min(), 1);
      } else if (BitSize == 64) { // Signed Long Long Int
        Ident = ConstantInt::get(Ty, std::numeric_limits<int64_t>::min(), 1);
      }

      break;
    }

    case ICmpInst::ICMP_SLE:
    case ICmpInst::ICMP_SLT: {
      // Signed Int Min Reduction

      if (BitSize == 32) {
        Ident = ConstantInt::get(Ty, std::numeric_limits<int32_t>::max(), 1);
      } else if (BitSize == 16) { // Signed Short Int
        Ident = ConstantInt::get(Ty, std::numeric_limits<int16_t>::max(), 1);
      } else if (BitSize == 64) { // Signed Long Long Int
        Ident = ConstantInt::get(Ty, std::numeric_limits<int64_t>::max(), 1);
      }

      break;
    }

    case ICmpInst::ICMP_UGE:
    case ICmpInst::ICMP_UGT: {
      // Unsigned Int Max Reduction
      Ident = ConstantInt::get(Ty, 0, 0);
      break;
    }

    case ICmpInst::ICMP_ULE:
    case ICmpInst::ICMP_ULT: {
      // Unsigned Int Min Reduction
      if (BitSize == 32) {
        Ident = ConstantInt::get(Ty, std::numeric_limits<uint32_t>::max(), 0);
      } else if (BitSize == 16) { // Unsigned Short Int
        Ident = ConstantInt::get(Ty, std::numeric_limits<uint16_t>::max(), 0);
      } else if (BitSize == 64) { // Unsigned Long Long Int
        Ident = ConstantInt::get(Ty, std::numeric_limits<uint64_t>::max(), 0);
      }

      break;
    }

    case FCmpInst::FCMP_UGE:
    case FCmpInst::FCMP_UGT:
    case FCmpInst::FCMP_OGE:
    case FCmpInst::FCMP_OGT: {
      // Ordered or Unordered Float Max reduction
      if (Ty->isFloatTy()) {
        Ident = ConstantFP::get(Ty, (std::numeric_limits<float>::lowest()));
      } else if (Ty->isDoubleTy()) {
        // Double Max Reduction
        Ident = ConstantFP::get(Ty, (std::numeric_limits<double>::lowest()));
      } else {
        errs() << "\n";
        errs().changeColor(raw_ostream::BLUE, true);
        errs() << "!! ERROR: COULD NOT PERFORM SPMDization !!\n";
        errs().resetColor();
        errs() << R"help(
                     Failed to find correct floating point type for the reduction.
                     )help";
        Ident = nullptr;
      }
      break;
    }

    case FCmpInst::FCMP_ULE:
    case FCmpInst::FCMP_ULT:
    case FCmpInst::FCMP_OLE:
    case FCmpInst::FCMP_OLT: {
      // Ordered or Unordered Float Min reduction
      if (Ty->isFloatTy()) {
        Ident = ConstantFP::get(Ty, std::numeric_limits<float>::max());
      } else if (Ty->isDoubleTy()) {
        // Double Min Reduction
        Ident = ConstantFP::get(Ty, std::numeric_limits<double>::max());
      } else {
        errs() << "\n";
        errs().changeColor(raw_ostream::BLUE, true);
        errs() << "!! ERROR: COULD NOT PERFORM SPMDization !!\n";
        errs().resetColor();
        errs() << R"help(
                     Failed to find correct floating point type for the reduction.
                     )help";
        Ident = nullptr;
      }
      break;
    }

    default: {
      // Unknown predicate for Min/Max reduction
      errs().changeColor(raw_ostream::GREEN, true);
      errs() << "!! ERROR: COULD NOT PERFORM SPMDization  !!\n";
      errs().resetColor();
      errs() << R"help(
                  Failed to get predicate of comparision for min/max reduction.
                  )help";
      break;
    }
    }

    break;
  }
  default: {
    // Doesn't have an identity.
    errs() << "\n";
    errs().changeColor(raw_ostream::BLUE, true);
    errs() << "!! ERROR: COULD NOT PERFORM SPMDization !!\n";
    errs().resetColor();
    errs() << R"help(
                Failed to find the identity element of the reduction operation.

                )help";
    Ident = nullptr;
    break;
  }
  }
  return Ident;
}

// Handling of reductions in the case of flat Zero trip checks
bool LoopSPMDization::fixReductionsFlatIfAny(
    Loop *L, Loop *OrigL, BasicBlock *E, BasicBlock *AfterLoop, int PE,
    int NPEs, std::vector<Value *> &ReduceVarExitOrig,
    std::vector<Instruction *> &ReduceVarOrig,
    std::vector<Instruction *> &OldInsts) {
  BasicBlock *PredAfterLoop;
  PredAfterLoop = L->getExitBlock();
  // This can be used in the future when preheader of the inner created loop is
  // properly propagated after cloning which is not the case: clone loop
  // function is buggy, preheader of inner loop is not found properly
  // Loop *ReductionLoop;
  //   if (spmd_approach == SPMD_HYBRID) {
  //   Header = Header->getSingleSuccessor();
  //   Preheader = Preheader->getSingleSuccessor();
  //   for (Loop *SL : L->getSubLoops()) {
  //     ReductionLoop = SL;
  //     Header = ReductionLoop->getHeader();
  //     //This generate null currently because clone loop function did not
  // properly keep the preheader info for the inner loop
  //     //Preheader = ReductionLoop->getLoopPreheader();
  //   }
  // }
  // else
  //   ReductionLoop = L; // then use reduction loop instead of L

  unsigned r = 0;
  for (Instruction &I : *L->getHeader()) {
    PHINode *Phi = dyn_cast<PHINode>(&I);
    if (!Phi)
      continue;
    // This (isReductionPHI) does not work with  the hybrid approach. Normally,
    // we would test the reduction in the inner loop (reductionloop) but since
    // clone loop function is buggy, preheader of inner loop is not found
    // properly. That's why I added a hack using the r reductions found
    // previously in find reduction function
    // RecurrenceDescriptor RedDes;
    //  if(RecurrenceDescriptor::isReductionPHI(Phi, ReductionLoop, RedDes)) {

    if (ReduceVarOrig[r]) {
      Instruction *ReduceVar;
      if (Phi->getIncomingBlock(0) == L->getLoopPreheader()) {
        ReduceVar = cast<Instruction>(Phi->getIncomingValue(1));
        // initialize the reduction on PE!=0 to identity
        Value *Ident = findReductionIdentity(Phi, ReduceVarOrig[r]);
        if (!Ident)
          return false;
        Phi->setIncomingValue(0, Ident);
      } else {
        ReduceVar = cast<Instruction>(Phi->getIncomingValue(0));
        // initialize the reduction on PE!=0 to identity
        Value *Ident = findReductionIdentity(Phi, ReduceVarOrig[r]);
        if (!Ident)
          return false;
        Phi->setIncomingValue(1, Ident);
      }
      // Flat: Update the exitblock with different reduced values (identity or
      // the result)
      BasicBlock *BBExit = L->getExitBlock();
      IRBuilder<> BExit(BBExit->getFirstNonPHI());
      PHINode *IfRedPhi =
          BExit.CreatePHI(ReduceVar->getType(), 1, Phi->getName() + "New");
      IfRedPhi->addIncoming(ReduceVar, L->getLoopLatch());
      for (auto IT = pred_begin(BBExit), ET = pred_end(BBExit); IT != ET;
           ++IT) {
        BasicBlock *Predecessor = *IT;
        // this is the predecessor coming from the zero trip count gard
        // block
        if (IfRedPhi->getBasicBlockIndex(Predecessor) == -1) {
          Value *Ident = findReductionIdentity(IfRedPhi, ReduceVarOrig[r]);
          if (!Ident)
            return false;
          IfRedPhi->addIncoming(Ident, Predecessor);
        }
      }
      // Flat: update the afterloop basic block with the the new reduced value
      // (IfRedPhi) created in the exit block
      BasicBlock::iterator I, IE;
      for (I = AfterLoop->begin(), IE = AfterLoop->end(); I != IE; ++I) {
        PHINode *PhiExit = dyn_cast<PHINode>(&*I);
        Instruction *NewInstPhi;
        PHINode *NewPhi;
        Instruction *ReduceVarExit;
        IRBuilder<> B(AfterLoop->getFirstNonPHI());
        bool FoundP = false;
        // look for use of the reduced value
        if (!PhiExit) {
          for (unsigned M = 0; M < I->getNumOperands(); M++) {
            if (I->getOperand(M) == ReduceVarExitOrig[r]) {
              ReduceVarExit = dyn_cast<Instruction>(I->getOperand(M));
              if (PE == 1) {
                PhiExit = B.CreatePHI(ReduceVar->getType(), 1,
                                      Phi->getName() + "orig");
                PhiExit->addIncoming(ReduceVarExitOrig[r], PredAfterLoop);
                I->setOperand(M, PhiExit);
              }
              NewPhi =
                  B.CreatePHI(ReduceVar->getType(), 1, Phi->getName() + "red");
              NewPhi->addIncoming(IfRedPhi, PredAfterLoop);
              NewInstPhi = dyn_cast<Instruction>(NewPhi);
              FoundP = true;
            }
          }

        } else { // There is an actual Phi node for the reduction var
          if (PhiExit->getNumIncomingValues() >= 2)
            ReduceVarExit = dyn_cast<Instruction>(PhiExit->getIncomingValue(1));
          else
            ReduceVarExit = dyn_cast<Instruction>(PhiExit->getIncomingValue(0));
          if (ReduceVarExit) {
            if (dyn_cast<Value>(ReduceVarExit) == ReduceVarExitOrig[r]) {
              NewInstPhi = PhiExit->clone();
              NewPhi = cast<PHINode>(NewInstPhi);

              if (PhiExit->getNumIncomingValues() >= 2) {
                NewPhi->setIncomingValue(1, IfRedPhi);
                NewPhi->setIncomingBlock(1, L->getExitBlock());
              } else {
                NewPhi->setIncomingValue(0, IfRedPhi);
                NewPhi->setIncomingBlock(0, L->getExitBlock());
              }
              AfterLoop->getInstList().insert(B.GetInsertPoint(), NewInstPhi);
              FoundP = true;
            }
          }
        }
        // AfterLoop does not contain a use or a phi of use
        BasicBlock::iterator II = I;
        if (!FoundP && II == (--(AfterLoop->end()))) {
          if (PE == 1) {
            PhiExit =
                B.CreatePHI(ReduceVar->getType(), 1, Phi->getName() + "orig");
            PhiExit->addIncoming(ReduceVarExitOrig[r], OrigL->getExitBlock());
          }
          NewPhi = B.CreatePHI(ReduceVar->getType(), 1, Phi->getName() + "red");
          NewPhi->addIncoming(IfRedPhi, PredAfterLoop);
          NewInstPhi = cast<Instruction>(NewPhi);
          FoundP = true;
        }
        if (FoundP) {
          // Phi corresponding to first cloned loop is already there
          if (PE == 1) {
            OldInsts[r] = PhiExit;
            B.SetInsertPoint(AfterLoop->getFirstNonPHI());
          } else
            B.SetInsertPoint(OldInsts[r]->getNextNode());

          Instruction *NewInst;
          Instruction *NewCmpInst;
          if (ReduceVarOrig[r]->getOpcode() == Instruction::Select) {
            // For Min-Max reduction new cmp instruction should also be added
            CmpInst *RedCmpInst;
            RedCmpInst = dyn_cast<CmpInst>(ReduceVarOrig[r]->getOperand(0));
            if (!RedCmpInst) {
              errs() << "\n";
              errs().changeColor(raw_ostream::BLUE, true);
              errs() << "!! WARNING: COULD NOT PERFORM SPMDization !!\n";
              errs().resetColor();
              errs() << R"help(
                         Failed to get compare insstruction for min/max reduction to insert at the end
                       )help";
              return false;
            }
            NewCmpInst = RedCmpInst->clone();
            AfterLoop->getInstList().insert(B.GetInsertPoint(), NewCmpInst);

            // Add select instruction after the cmp instruction
            NewInst = ReduceVarOrig[r]->clone();
            OldInsts[r]->replaceAllUsesWith(NewInst);
            ReduceVarExitOrig[r]->replaceUsesOutsideBlock(NewInst, AfterLoop);

            NewCmpInst->setOperand(1, OldInsts[r]);
            NewCmpInst->setOperand(0, NewInstPhi);
            NewInst->setOperand(0, NewCmpInst);
            NewInst->setOperand(2, OldInsts[r]);
            NewInst->setOperand(1, NewInstPhi);

          } else {
            // Other Reductions
            NewInst = ReduceVarOrig[r]->clone();
            OldInsts[r]->replaceAllUsesWith(NewInst);
            ReduceVarExitOrig[r]->replaceUsesOutsideBlock(NewInst, AfterLoop);
            NewInst->setOperand(1, OldInsts[r]);
            NewInst->setOperand(0, NewInstPhi);
          }
          AfterLoop->getInstList().insert(B.GetInsertPoint(), NewInst);
          OldInsts[r] = NewInst;
          break;
        }
      }
      // r++;
    } // end for r
    // Because we counted also non reduction phis for the hybrid approach
    r++;
  }
  return true;
}

// Handling of reductions in the case of nested zero trip checks where more
// integer operations are generated
bool LoopSPMDization::fixReductionsIfAny(
    Loop *L, Loop *OrigL, BasicBlock *E, BasicBlock *AfterLoop, int PE,
    int NPEs, std::vector<Value *> &ReduceVarExitOrig,
    std::vector<Instruction *> &ReduceVarOrig,
    std::vector<Instruction *> &OldInsts) {
  BasicBlock *PredAfterLoop;
  PredAfterLoop = L->getExitBlock();
  // This can be used in the future when preheader of the inner created loop is
  // properly propagated after cloning which is not the case: clone loop
  // function is buggy, preheader of inner loop is not found properly
  //    Loop *ReductionLoop;
  //   if (spmd_approach == SPMD_HYBRID) {
  //   Header = Header->getSingleSuccessor();
  //   Preheader = Preheader->getSingleSuccessor();
  //   for (Loop *SL : L->getSubLoops()) {
  //     ReductionLoop = SL;
  //     Header = ReductionLoop->getHeader();
  //     //This generate null currently because clone loop function did not
  // properly keep the preheader info for the inner loop
  //     //Preheader = ReductionLoop->getLoopPreheader();
  //   }
  // }
  // else
  //   ReductionLoop = L; // then use reduction loop instead of L
  //
  unsigned r = 0;
  for (Instruction &I : *L->getHeader()) {
    PHINode *Phi = dyn_cast<PHINode>(&I);
    if (!Phi)
      continue;
    // This (isReductionPHI) does not work with  the hybrid approach. Normally,
    // we would test the reduction in the inner loop (reductionloop) but since
    // clone loop function is buggy, preheader of inner loop is not found
    // properly. That's why I added a hack using the r reductions found
    // previously in find reduction function
    // RecurrenceDescriptor RedDes;
    // if (RecurrenceDescriptor::isReductionPHI(Phi, ReductionLoop, RedDes)) {
    if (ReduceVarOrig[r]) {
      Instruction *ReduceVar;
      if (Phi->getIncomingBlock(0) == L->getLoopPreheader()) {
        ReduceVar = cast<Instruction>(Phi->getIncomingValue(1));
        // initialize the reduction on PE!=0 to identity
        Value *Ident = findReductionIdentity(Phi, ReduceVarOrig[r]);
        if (!Ident)
          return false;
        Phi->setIncomingValue(0, Ident);
      } else {
        ReduceVar = cast<Instruction>(Phi->getIncomingValue(0));
        // initialize the reduction on PE!=0 to identity
        Value *Ident = findReductionIdentity(Phi, ReduceVarOrig[r]);
        if (!Ident)
          return false;
        Phi->setIncomingValue(1, Ident);
      }
      BasicBlock::iterator i, ie;
      for (i = AfterLoop->begin(), ie = AfterLoop->end(); (i != ie); ++i) {
        PHINode *PhiExit = dyn_cast<PHINode>(&*i);
        Instruction *NewInstPhi;
        PHINode *NewPhi;
        Instruction *ReduceVarExit;
        IRBuilder<> B(AfterLoop->getFirstNonPHI());
        bool FoundP = false;
        // look for use of the reduced value
        if (!PhiExit) {
          for (unsigned m = 0; m < i->getNumOperands(); m++) {
            if (i->getOperand(m) == ReduceVarExitOrig[r]) {
              ReduceVarExit = dyn_cast<Instruction>(i->getOperand(m));
              if (PE == 1) {
                PhiExit = B.CreatePHI(ReduceVar->getType(), 1,
                                      Phi->getName() + "orig");
                PhiExit->addIncoming(ReduceVarExitOrig[r], PredAfterLoop);
                i->setOperand(m, PhiExit);
              }
              NewPhi =
                  B.CreatePHI(ReduceVar->getType(), 1, Phi->getName() + "red");
              NewPhi->addIncoming(ReduceVar, PredAfterLoop);
              NewInstPhi = dyn_cast<Instruction>(NewPhi);
              FoundP = true;
            }
          }

        } else { // There is an actual Phi node for the reduction var
          if (PhiExit->getNumIncomingValues() >= 2) // == 2) zero trip change
            ReduceVarExit = dyn_cast<Instruction>(PhiExit->getIncomingValue(1));
          else
            ReduceVarExit = dyn_cast<Instruction>(PhiExit->getIncomingValue(0));
          if (ReduceVarExit) {
            if (dyn_cast<Value>(ReduceVarExit) == ReduceVarExitOrig[r]) {
              NewInstPhi = PhiExit->clone();
              NewPhi = cast<PHINode>(NewInstPhi);

              if (PhiExit->getNumIncomingValues() >= 2) // ==2
                NewPhi->setIncomingValue(1, ReduceVar);
              else
                NewPhi->setIncomingValue(0, ReduceVar);

              AfterLoop->getInstList().insert(B.GetInsertPoint(), NewInstPhi);
              FoundP = true;
            }
          }
        }
        // AfterLoop does not contain a use or a phi of use
        BasicBlock::iterator II = i;
        if (!FoundP && II == (--(AfterLoop->end()))) {
          if (PE == 1) {
            PhiExit =
                B.CreatePHI(ReduceVar->getType(), 1, Phi->getName() + "orig");
            PhiExit->addIncoming(ReduceVarExitOrig[r], PredAfterLoop);
          }
          NewPhi = B.CreatePHI(ReduceVar->getType(), 1, Phi->getName() + "red");
          NewPhi->addIncoming(ReduceVar, PredAfterLoop);
          NewInstPhi = dyn_cast<Instruction>(NewPhi);
          FoundP = true;
        }
        if (FoundP) {
          // Handling of the new branches related to the zero trip count
          BasicBlock *BB = AfterLoop;
          for (auto it = pred_begin(BB), et = pred_end(BB); it != et; ++it) {
            BasicBlock *predecessor = *it;
            { // this is the predecessor coming from the zero trip count gard
              // block
              if (NewPhi->getBasicBlockIndex(predecessor) == -1 &&
                  predecessor != PredAfterLoop) {
                Value *Ident = findReductionIdentity(NewPhi, ReduceVarOrig[r]);
                if (Ident)
                  NewPhi->addIncoming(Ident, predecessor);
                else
                  return false;
              }
            }
          }
          // Phi corresponding to first cloned loop is already there
          if (PE == 1) {
            OldInsts[r] = PhiExit;
            B.SetInsertPoint(AfterLoop->getFirstNonPHI());
          } else
            B.SetInsertPoint(OldInsts[r]->getNextNode());

          Instruction *NewInst;
          Instruction *NewCmpInst;
          if (ReduceVarOrig[r]->getOpcode() == Instruction::Select) {
            // For Min-Max reduction new cmp instruction should also be added
            CmpInst *RedCmpInst;
            RedCmpInst = dyn_cast<CmpInst>(ReduceVarOrig[r]->getOperand(0));
            if (!RedCmpInst) {
              errs() << "\n";
              errs().changeColor(raw_ostream::BLUE, true);
              errs() << "!! WARNING: COULD NOT PERFORM SPMDization !!\n";
              errs().resetColor();
              errs() << R"help(
                         Failed to get compare instruction for min/max reduction to insert at the end
                       )help";
              return false;
            }
            NewCmpInst = RedCmpInst->clone();
            AfterLoop->getInstList().insert(B.GetInsertPoint(), NewCmpInst);

            // Add select instruction after the compare instruction
            NewInst = ReduceVarOrig[r]->clone();
            OldInsts[r]->replaceAllUsesWith(NewInst);
            ReduceVarExitOrig[r]->replaceUsesOutsideBlock(NewInst, AfterLoop);

            NewCmpInst->setOperand(1, OldInsts[r]);
            NewCmpInst->setOperand(0, NewInstPhi);
            NewInst->setOperand(0, NewCmpInst);
            NewInst->setOperand(2, OldInsts[r]);
            NewInst->setOperand(1, NewInstPhi);

          } else {
            // Other Reductions
            NewInst = ReduceVarOrig[r]->clone();
            OldInsts[r]->replaceAllUsesWith(NewInst);
            ReduceVarExitOrig[r]->replaceUsesOutsideBlock(NewInst, AfterLoop);
            NewInst->setOperand(1, OldInsts[r]);
            NewInst->setOperand(0, NewInstPhi);
          }

          AfterLoop->getInstList().insert(B.GetInsertPoint(), NewInst);
          OldInsts[r] = NewInst;
          break;
        }
      }
      // r++;
    } // end for r
    // Because we counted also non reduction phis for the hybrid approach
    r++;
  }
  return true;
}

// This routine should made generic and be declared somewhere as public to be
// used here and in csa backend (Target/Intel_CSA/CSALoopIntrinsicExpander.cpp)
IntrinsicInst *LoopSPMDization::detectSPMDIntrinsic(Loop *L, LoopInfo *LI,
                                                    DominatorTree *DT,
                                                    PostDominatorTree *PDT,
                                                    int &NPEs,
                                                    int &Chunk_Size) {

  // Attempts to match a valid SPMDization entry/exit pair with an exit in a
  // given basic block.
  const auto match_pair_from_block =
      [=, &NPEs, &Chunk_Size](BasicBlock *BB) -> IntrinsicInst * {
    using namespace llvm::PatternMatch;

    // The block must be exactly one loop level above the loop.
    if (LI->getLoopDepth(BB) != L->getLoopDepth() - 1)
      return nullptr;

    // And it should post-dominate the loop in order to have a correct exit in
    // it.
    if (!PDT->dominates(BB, L->getHeader()))
      return nullptr;

    // Try to find an exit with a paired entry.
    for (Instruction &exit : *BB) {
      Instruction *entry = nullptr;
      uint64_t NPEs_64;
      uint64_t Chunk_64;
      if (!match(&exit, m_Intrinsic<Intrinsic::csa_spmdization_exit>(
                            m_Instruction(entry))) ||
          !match(entry, m_Intrinsic<Intrinsic::csa_spmdization_entry>(
                            m_ConstantInt(NPEs_64), m_ConstantInt(Chunk_64))))
        continue;

      // If one is found, make sure that the entry block is also one loop level
      // above the loop and dominates the loop.
      const BasicBlock *const entry_block = entry->getParent();
      if (LI->getLoopDepth(entry_block) != L->getLoopDepth() - 1)
        continue;
      if (!DT->dominates(entry_block, L->getHeader()))
        continue;

      IntrinsicInst *const entry_intr = dyn_cast<IntrinsicInst>(entry);
      assert(entry_intr && "Entry intrinsic is not an intrinsic??");
      NPEs = NPEs_64;
      Chunk_Size = Chunk_64;
      return entry_intr;
    }

    return nullptr;
  };

  // If there is a parent loop, only look inside of it for exits. Otherwise,
  // look through the entire function.
  if (Loop *const L_parent = L->getParentLoop()) {
    for (BasicBlock *const BB : L_parent->getBlocks()) {
      if (IntrinsicInst *const intr = match_pair_from_block(BB))
        return intr;
    }
  } else {
    for (BasicBlock &BB : *L->getHeader()->getParent()) {
      if (IntrinsicInst *const intr = match_pair_from_block(&BB))
        return intr;
    }
  }

  return nullptr;
}

// This routine has been copied from LoopInterchange.cpp. It has then been
// modified to accomodate the type of induction variables we are insterested in
// handling for SPMDization
PHINode *LoopSPMDization::getInductionVariable(Loop *L, ScalarEvolution *SE) {
  PHINode *InnerIndexVar = L->getCanonicalInductionVariable();
  if (InnerIndexVar)
    return InnerIndexVar;
  if (L->getLoopLatch() == nullptr || L->getLoopPredecessor() == nullptr)
    return nullptr;
  for (BasicBlock::iterator I = L->getHeader()->begin(); isa<PHINode>(I); ++I) {
    PHINode *PhiVar = cast<PHINode>(I);
    Type *PhiTy = PhiVar->getType();
    if (!PhiTy->isIntegerTy() && !PhiTy->isFloatingPointTy() &&
        !PhiTy->isPointerTy())
      return nullptr;
    if (!PhiTy->isIntegerTy())
      continue;
    if (!SE->isSCEVable(PhiVar->getType()))
      continue;
    const SCEVAddRecExpr *AddRec =
        dyn_cast<SCEVAddRecExpr>(SE->getSCEV(PhiVar));
    if (!AddRec || !AddRec->isAffine())
      continue;
    const SCEV *Step = AddRec->getStepRecurrence(*SE);
    if (!isa<SCEVConstant>(Step))
      continue;
    // Found the induction variable.
    // FIXME: Handle loops with more than one induction variable. Note that,
    // currently, legality makes sure we have only one induction variable.
    return PhiVar;
  }
  return nullptr;
}

// Add a second loop level that processes chunk size iterations
// Change loop cond and branch of the outer loop as the following
//
//
// for (i=0; i<n; i+=k) {
//    a[i] = b[i];
// }
// to
//  for(j=0; j<n; j+=chunk_size*k)  //cyclic loop
//      for(i=j; i<min(j+chunk_size, n); i+=1) //blocked loop
//          a[i] = b[i];

void LoopSPMDization::AddHybridLoopLevel(Loop *L, int Chunk_Size,
                                         PHINode *ClonedInductionPHI,
                                         ScalarEvolution *SE, DominatorTree *DT,
                                         LoopInfo *LI) {
  PHINode *InductionPHI = getInductionVariable(L, SE);
  BasicBlock *PreHeader = L->getLoopPreheader();
  BasicBlock *Header = L->getHeader();
  BasicBlock *Latch = L->getLoopLatch();
  BranchInst *LatchBR = cast<BranchInst>(Latch->getTerminator());
  BasicBlock *CondBlock;
  // retrieve the looping condition
  if (LatchBR->isConditional()) {
    CondBlock = Latch;
    Cond = LatchBR->getCondition();
  } else {
    CondBlock = Header;
    Cond = (cast<BranchInst>(Header->getTerminator()))->getCondition();
  }
  Instruction *CondI = dyn_cast<Instruction>(Cond);

  // retrieve the increment variable
  Instruction *NewInc;
  Instruction *OldInc;
  if (InductionPHI->getIncomingBlock(0) == PreHeader) {
    NewInc = dyn_cast<Instruction>(InductionPHI->getIncomingValue(1));
    OldInc = dyn_cast<Instruction>(ClonedInductionPHI->getIncomingValue(1));
  } else {
    NewInc = dyn_cast<Instruction>(InductionPHI->getIncomingValue(0));
    OldInc = dyn_cast<Instruction>(ClonedInductionPHI->getIncomingValue(0));
  }
  BranchInst *OldCondBlockBR = cast<BranchInst>(CondBlock->getTerminator());

  // Split the loop header into two blocks: the old loop header and the new
  // header (for the new loop)
  BasicBlock *NewHeader =
      SplitBlock(Header, dyn_cast<Instruction>(InductionPHI), DT, LI);
  Header->setName(L->getHeader()->getName() + ".hybrid.hr");

  // Update condblock based on the inner loop
  if (LatchBR->isConditional() && Latch != Header)
    CondBlock = Latch;
  else
    CondBlock = NewHeader;
  // Split the latch into two blocks: new header and LLatch that is the old loop
  // latch
  BasicBlock *LLatch = SplitBlock(
      CondBlock, dyn_cast<Instruction>(CondBlock->getTerminator()), DT, LI);
  LLatch->setName(CondBlock->getName() + ".hybrid.latch");
  IRBuilder<> Cblock(LLatch->getFirstNonPHI());
  NewInc->removeFromParent();
  Cblock.Insert(NewInc);
  CondI->removeFromParent();
  Cblock.Insert(CondI);

  // Update loop info with the created new loop
  Loop *NewLoop = LI->AllocateLoop();
  // Insert the new loop into the loop nest and register the new basic blocks
  // before calling any utilities such as SCEV that require valid LoopInfo.
  L->addChildLoop(NewLoop);
  LI->removeBlock(NewHeader);
  NewLoop->addBasicBlockToLoop(NewHeader, *LI);
  for (Loop::block_iterator I = L->block_begin(), E = L->block_end(); I != E;
       ++I) {
    if (*I != LLatch && *I != Header) {
      if (!NewLoop->contains(*I)) {
        NewLoop->addBlockEntry(*I);
      }
    }
  }
  // Disable full unrolling on NewLoop as it will just mean that we applied
  // cyclic plus unroll of chunk size on L. The decision might be revised for
  // wide st ld/st generation AddUnrollDisableMetadata(L);
  AddUnrollDisableMetadata(NewLoop);

  Instruction *ClonedOldInc = OldInc->clone();
  Instruction *ClonedBr = OldCondBlockBR->clone();
  // We use  original loop init and step (before transformloopinitandstep) and
  // convert them to the following for i=j; i<min(j+chunk_size*step, n); i+=step
  // this is the inserted blocking loop for the hybrid approach
  IRBuilder<> Bcondblock(CondBlock->getTerminator());
  IRBuilder<> Bheader(Header->getTerminator());

  CondBlock->getInstList().insert(Bcondblock.GetInsertPoint(), ClonedOldInc);
  // calculate BlockUpper = min(OldInc+chunk_size*step, N)
  Value *steptimeschunk =
      Bheader.CreateMul(OldInc->getOperand(1),
                        ConstantInt::get(InductionPHI->getType(), Chunk_Size),
                        InductionPHI->getName() + ".steptimeschunk");
  Value *BlockUpper = Bheader.CreateAdd(
      InductionPHI, steptimeschunk, InductionPHI->getName() + ".pluschunksize");
  // Use signed to take into account signed values of iterators
  auto *min = Bheader.CreateICmpSLT(BlockUpper, CondI->getOperand(1));
  BlockUpper = Bheader.CreateSelect(min, BlockUpper, CondI->getOperand(1));

  Instruction *ClonedCondI = CondI->clone();
  if (ClonedCondI->getOperand(0) == dyn_cast<Value>(NewInc)) {
    ClonedCondI->setOperand(0, ClonedOldInc);
    ClonedCondI->setOperand(1, BlockUpper);
  } else {
    ClonedCondI->setOperand(1, ClonedOldInc);
    ClonedCondI->setOperand(0, BlockUpper);
  }
  CondBlock->getInstList().insert(Bcondblock.GetInsertPoint(), ClonedCondI);
  if (OldCondBlockBR->getSuccessor(0) == L->getHeader()) {
    cast<BranchInst>(ClonedBr)->setSuccessor(0, NewHeader);
    cast<BranchInst>(ClonedBr)->setSuccessor(1, LLatch);
  } else {
    cast<BranchInst>(ClonedBr)->setSuccessor(0, LLatch);
    cast<BranchInst>(ClonedBr)->setSuccessor(1, NewHeader);
  }
  cast<BranchInst>(ClonedBr)->setCondition(ClonedCondI);
  ReplaceInstWithInst(CondBlock->getTerminator(), ClonedBr);
  // Add induction phi to the new header (header of newloop)
  ClonedOldInc->setOperand(0, ClonedInductionPHI);
  if (ClonedInductionPHI->getIncomingBlock(0) == PreHeader) {
    ClonedInductionPHI->setIncomingValue(1, ClonedOldInc);
    ClonedInductionPHI->setIncomingBlock(1, CondBlock);
    ClonedInductionPHI->setIncomingValue(0, InductionPHI);
    ClonedInductionPHI->setIncomingBlock(0, Header);
  } else {
    ClonedInductionPHI->setIncomingValue(0, ClonedOldInc);
    ClonedInductionPHI->setIncomingBlock(0, CondBlock);
    ClonedInductionPHI->setIncomingValue(1, InductionPHI);
    ClonedInductionPHI->setIncomingBlock(1, Header);
  }
  ClonedInductionPHI->insertBefore(NewHeader->getFirstNonPHI());

  // simply doing this (replacealluseswith) will break SSA in inductionPHI
  // (introduce recursive use). So we will have to do the replacement use by use
  // in the second coming loop
  // InductionPHI->replaceAllUsesWith(ClonedInductionPHI);
  // Move also other phis to NewHeader
  IRBuilder<> Bblock(NewHeader->getFirstNonPHI());
  BasicBlock::iterator bi, bie;
  for (bi = Header->begin(), bie = Header->end(); (bi != bie); ++bi) {
    PHINode *OtherPhi = dyn_cast<PHINode>(&*bi);
    if (!OtherPhi)
      break;
    if (OtherPhi != InductionPHI) {
      Instruction *BlockPhiInst = OtherPhi->clone();
      PHINode *BlockPhi = cast<PHINode>(BlockPhiInst);
      OtherPhi->replaceAllUsesWith(BlockPhi);
      NewHeader->getInstList().insert(Bblock.GetInsertPoint(), BlockPhi);
      if (BlockPhi->getIncomingBlock(0) == PreHeader) {
        BlockPhi->setIncomingBlock(0, Header);
        BlockPhi->setIncomingValue(0, OtherPhi);
        BlockPhi->setIncomingBlock(1, CondBlock);
      } else {
        BlockPhi->setIncomingBlock(1, Header);
        BlockPhi->setIncomingValue(1, OtherPhi);
        BlockPhi->setIncomingBlock(0, CondBlock);
      }
    }
  }
  for (auto UA = InductionPHI->user_begin(), EA = InductionPHI->user_end();
       UA != EA;) {
    Instruction *User_liveotherphi = cast<Instruction>(*UA++);
    // we replace uses of the induction variable in the blocks of the new inner
    // loop only
    if (User_liveotherphi->getParent() != Header &&
        User_liveotherphi->getParent() != LLatch) {
      for (unsigned m = 0; m < User_liveotherphi->getNumOperands(); m++)
        if (User_liveotherphi->getOperand(m) == dyn_cast<Value>(InductionPHI) &&
            User_liveotherphi != ClonedInductionPHI) {
          User_liveotherphi->setOperand(m, ClonedInductionPHI);
        }
    }
  }
  return;
}

bool LoopSPMDization::TransformLoopInitandBound(Loop *L, ScalarEvolution *SE,
                                                int PE, int NPEs) {
  PHINode *InductionPHI = getInductionVariable(L, SE);
  BasicBlock *PreHeader = L->getLoopPreheader();
  BasicBlock *Header = L->getHeader();
  BranchInst *PreHeaderBR = cast<BranchInst>(PreHeader->getTerminator());
  BasicBlock *Latch = L->getLoopLatch();
  BranchInst *LatchBR = cast<BranchInst>(Latch->getTerminator());
  if (!InductionPHI) {
    LLVM_DEBUG(dbgs() << "Failed to find the loop induction variable "
                         "in one of the loops marked with SPMD intrinsic \n");
    return false;
  }
  IRBuilder<> B(PreHeaderBR);
  if (LatchBR->isConditional())
    Cond = LatchBR->getCondition();
  else
    Cond = (cast<BranchInst>(Header->getTerminator()))->getCondition();
  Instruction *CondI = cast<Instruction>(Cond);
  if (PE == 0) {
    if (InductionPHI->getIncomingBlock(0) == PreHeader) {
      LowerBound = InductionPHI->getIncomingValue(0);
    } else {
      LowerBound = InductionPHI->getIncomingValue(1);
    }
    UpperBound = CondI->getOperand(1);
    if (cast<IntegerType>(nbyk->getType())->getBitWidth() !=
        cast<IntegerType>(LowerBound->getType())->getBitWidth())
      nbyk = B.CreateZExtOrTrunc(nbyk, LowerBound->getType(),
                                 nbyk->getName() + ".trex");
  }
  // i = i+PE ==> i+ (k-1)n/NPEs ==> i+(k-1)*nbyNPEs
  Value *ktimesnbyk = B.CreateMul(ConstantInt::get(nbyk->getType(), PE), nbyk,
                                  InductionPHI->getName() + ".ktimesnbyk");
  Value *kplus1 = B.CreateAdd(ConstantInt::get(nbyk->getType(), PE),
                              ConstantInt::get(nbyk->getType(), 1),
                              InductionPHI->getName() + ".kplus1");
  Value *kplus1timesnbyk =
      B.CreateMul(kplus1, nbyk, InductionPHI->getName() + ".k+1xnbyk");
  Value *kplus1timesnbyk2 = B.CreateAdd(kplus1timesnbyk, LowerBound,
                                        InductionPHI->getName() + ".k+1xnbyk2");
  NewInitV =
      B.CreateAdd(LowerBound, ktimesnbyk, InductionPHI->getName() + ".init");
  if (InductionPHI->getIncomingBlock(0) == PreHeader) {
    InductionPHI->setIncomingValue(0, NewInitV);
  } else {
    InductionPHI->setIncomingValue(1, NewInitV);
  }
  // change bound (cond)
  if (dyn_cast<IntegerType>(kplus1timesnbyk2->getType())->getBitWidth() !=
      dyn_cast<IntegerType>(UpperBound->getType())->getBitWidth())
    kplus1timesnbyk2 =
        B.CreateZExtOrTrunc(kplus1timesnbyk2, UpperBound->getType(),
                            kplus1timesnbyk2->getName() + ".trex");

  // this handles the case where the loop enters with an init value equal to the
  // bound
  CmpInst *CmpCond = dyn_cast<CmpInst>(Cond);
  if (CmpCond->getPredicate() == CmpInst::ICMP_EQ ||
      CmpCond->getPredicate() == CmpInst::ICMP_NE) {
    if (LatchBR->getSuccessor(0) == L->getHeader())
      CmpCond->setPredicate(CmpInst::ICMP_SLT);
    else
      CmpCond->setPredicate(CmpInst::ICMP_SGE);
  }

  // Case where the loop iterator is USE after the comparison. This appears in
  // Fortran code where the loop is converted into a while loop. The iterator
  // gets incremented/decremented after the comparison. The new upper bound
  // needs to be decreased in order to avoid executing one iteration twice by
  // each two workers
  bool start_tracking = false;
  bool whileloop_p = false;
  BasicBlock::iterator bi, bie;
  for (bi = CmpCond->getParent()->begin(), bie = CmpCond->getParent()->end();
       (bi != bie); ++bi) {
    Instruction *neighbor = dyn_cast<Instruction>(&*bi);
    for (auto UA = (dyn_cast<Value>(InductionPHI))->user_begin(),
              EA = (dyn_cast<Value>(InductionPHI))->user_end();
         UA != EA;) {
      Instruction *use = cast<Instruction>(*UA++);
      if (start_tracking && neighbor == use) {
        if (use->getOpcode() == Instruction::Add) {
          whileloop_p = true;
          break;
        }
      }
    }
    if (neighbor == CmpCond)
      start_tracking = true;
  }
  if (whileloop_p)
    kplus1timesnbyk2 = B.CreateAdd(
        kplus1timesnbyk2, ConstantInt::get(kplus1timesnbyk2->getType(), -1),
        InductionPHI->getName() + ".k+1xnbykwhile");
  if (PE == NPEs - 1)
    kplus1timesnbyk2 = UpperBound;

  CondI->setOperand(1, kplus1timesnbyk2);
  return true;
}

bool LoopSPMDization::TransformLoopInitandStep(Loop *L, ScalarEvolution *SE,
                                               int PE, int NPEs,
                                               int Chunk_Size) {

  PHINode *InductionPHI = getInductionVariable(L, SE);
  BasicBlock *PreHeader = L->getLoopPreheader();
  BasicBlock *Header = L->getHeader();
  BranchInst *PreHeaderBR = cast<BranchInst>(PreHeader->getTerminator());
  BasicBlock *Latch = L->getLoopLatch();
  if (!InductionPHI) {
    errs() << "\n";
    errs().changeColor(raw_ostream::BLUE, true);
    errs() << "!! WARNING: COULD NOT PERFORM SPMDization !!\n";
    errs().resetColor();
    errs() << R"help(
Failed to find the loop induction variable.

)help";
    LLVM_DEBUG(dbgs() << "Failed to find the loop induction variable \n");
    return false;
  }
  Instruction *OldInc;
  Value *InitVar;
  if (InductionPHI->getIncomingBlock(0) == PreHeader) {
    OldInc = cast<Instruction>(InductionPHI->getIncomingValue(1));
    InitVar = InductionPHI->getIncomingValue(0);
  } else {
    OldInc = cast<Instruction>(InductionPHI->getIncomingValue(0));
    InitVar = InductionPHI->getIncomingValue(1);
  }
  IRBuilder<> B2(OldInc);
  if (PE == 0) {
    StepPE0 = OldInc->getOperand(1);
    steptimesk = B2.CreateMul(OldInc->getOperand(1),
                              ConstantInt::get(InductionPHI->getType(), NPEs),
                              InductionPHI->getName() + ".steptimesk");
    if (spmd_approach == SPMD_HYBRID)
      steptimesk = B2.CreateMul(
          steptimesk, ConstantInt::get(InductionPHI->getType(), Chunk_Size),
          InductionPHI->getName() + ".timeschunksize");
  }
  Value *NewInc = B2.CreateAdd(InductionPHI, steptimesk,
                               InductionPHI->getName() + ".next.spmd");

  IRBuilder<> B(PreHeaderBR);
  if (PE > 0) {
    Value *steptimespe =
        B.CreateMul(StepPE0, ConstantInt::get(InductionPHI->getType(), PE),
                    InductionPHI->getName() + ".steptimesPE");
    if (spmd_approach == SPMD_HYBRID)
      steptimespe = B.CreateMul(
          steptimespe, ConstantInt::get(InductionPHI->getType(), Chunk_Size),
          InductionPHI->getName() + ".timeschunksize");
    /*Value **/ NewInitV =
        B.CreateAdd(InitVar, steptimespe, InductionPHI->getName() + ".init",
                    dyn_cast<Instruction>(NewInc));
  } else
    NewInitV = InitVar;
  if (InductionPHI->getIncomingBlock(0) == PreHeader) {
    InductionPHI->setIncomingValue(0, NewInitV);
    InductionPHI->setIncomingValue(1, NewInc);
  } else {
    InductionPHI->setIncomingValue(1, NewInitV);
    InductionPHI->setIncomingValue(0, NewInc);
  }

  BranchInst *LatchBR = cast<BranchInst>(Latch->getTerminator());
  if (LatchBR->isConditional())
    Cond = LatchBR->getCondition();
  else
    Cond = (cast<BranchInst>(Header->getTerminator()))->getCondition();
  /*Value **/ // Cond = LatchBR->getCondition();
  Instruction *CondI = cast<Instruction>(Cond);
  bool CondFoundP = false;
  if (CondI->getOperand(0) == dyn_cast<Value>(OldInc) ||
      CondI->getOperand(1) == dyn_cast<Value>(OldInc))
    CondFoundP = true;
  else if (CondI->getOperand(0) == dyn_cast<Value>(InductionPHI) ||
           CondI->getOperand(1) == dyn_cast<Value>(InductionPHI)) {
    CondFoundP = true;
    OldInc = dyn_cast<Instruction>(InductionPHI);
    NewInc = dyn_cast<Value>(InductionPHI);
  } else {
    for (auto UA = (dyn_cast<Value>(OldInc))->user_begin(),
              EA = (dyn_cast<Value>(OldInc))->user_end();
         UA != EA;) {
      Instruction *User_OldInc = cast<Instruction>(*UA++);
      if (CondI->getOperand(0) == dyn_cast<Value>(User_OldInc) ||
          CondI->getOperand(1) == dyn_cast<Value>(User_OldInc)) {
        CondFoundP = true;
        for (unsigned m = 0; m < User_OldInc->getNumOperands(); m++)
          if (User_OldInc->getOperand(m) == dyn_cast<Value>(OldInc)) {
            User_OldInc->setOperand(m, NewInc);
            // OldInc->replaceAllUsesWith(NewInc);
            NewInc = User_OldInc;
            OldInc = User_OldInc;
          }
      }
    }
  }
  if (!CondFoundP) {
    errs() << "\n";
    errs().changeColor(raw_ostream::BLUE, true);
    errs() << "!! WARNING: COULD NOT PERFORM SPMDization !!\n";
    errs().resetColor();
    errs() << R"help(
Failed to find the loop latch condition.

)help";
    return false;
  } else {
    Value *TripCount = CondI->getOperand(1);
    Value *IdxCmp;
    CmpInst *CmpCond = dyn_cast<CmpInst>(Cond);
    Value *NewCondOp0, *NewCondOp1;
    if (CondI->getOperand(0) == dyn_cast<Value>(OldInc)) {
      NewCondOp0 = NewInc;
      NewCondOp1 = TripCount;
    } else {
      NewCondOp1 = NewInc;
      NewCondOp0 = TripCount;
    }
    if (CmpCond->getPredicate() == CmpInst::ICMP_EQ ||
        CmpCond->getPredicate() == CmpInst::ICMP_NE) {
      if (LatchBR->getSuccessor(0) == L->getHeader())
        IdxCmp = CmpInst::Create(Instruction::ICmp, CmpInst::ICMP_SLT,
                                 NewCondOp0, NewCondOp1, Cond->getName());

      else
        IdxCmp = CmpInst::Create(Instruction::ICmp, CmpInst::ICMP_SGE,
                                 NewCondOp0, NewCondOp1, Cond->getName());
      // Cond = dyn_cast<Value>(IdxCmp);
      ReplaceInstWithInst(CondI, dyn_cast<Instruction>(IdxCmp));
    } else { // in other cases, we keep the same predicate
      if (CondI->getOperand(0) == dyn_cast<Value>(OldInc))
        CondI->setOperand(0, NewInc);
      else
        CondI->setOperand(1, NewInc);
    }
  }
  return true;
}

// This routine returns the minimum iterations the user assumes the loop L will
// be executing in order to help removing the zero trip case when replicating
// loops among workers. This routines implements the following cases:

// case 1 lower bound is a constant and upper bound is a constant
//   This does not need builtin_assume insertion from the user
//   __builtin_csa_spmdization(3,"cyclic");
//   for (int64_t i = 2; i < 1024; i+=1)

// case 2: lower bound of loop is constant
//   builtin_assume(hi>3); // or (hi-2>1)
//   __builtin_csa_spmdization(3,"cyclic");
//   for (int64_t i = 2; i < hi; i+=1)

// Case 3: lower bound of loop is not a constant
//  builtin_assume(hi>lo);
//  __builtin_csa_spmdization(3,"cyclic");
//  for (int64_t i = lo; i < hi; i+=1)

// Case 4: lower bound of loop is not a constant
//  builtin_assume(hi-lo>2);
//  __builtin_csa_spmdization(3,"cyclic");
//  for (int64_t i = lo; i < hi; i+=1)

// Other cases will be implemented based on test cases
int LoopSPMDization::GetMinLoopIterations(Loop *L, ScalarEvolution *SE) {
  Function *F = L->getHeader()->getParent();
  PHINode *InductionPHI = getInductionVariable(L, SE);
  BasicBlock *PreHeader = L->getLoopPreheader();
  BasicBlock *Header = L->getHeader();
  BasicBlock *Latch = L->getLoopLatch();
  BranchInst *LatchBR = cast<BranchInst>(Latch->getTerminator());

  if (!InductionPHI) {
    LLVM_DEBUG(dbgs() << "Failed to find the loop induction variable "
                         "in one of the loops marked with SPMD intrinsic \n");
    return -1;
  }
  Value *Cond;
  if (LatchBR->isConditional())
    Cond = LatchBR->getCondition();
  else
    Cond = (cast<BranchInst>(Header->getTerminator()))->getCondition();
  Instruction *CondI = dyn_cast<Instruction>(Cond);
  if (!CondI)
    return -1;
  // TODO: Check the case when latch is driven by a <= instead of a < or !=
  Value *UpperBound = CondI->getOperand(1);
  if (InductionPHI->getIncomingBlock(0) == PreHeader) {
    LowerBound = InductionPHI->getIncomingValue(0);
  } else {
    LowerBound = InductionPHI->getIncomingValue(1);
  }
  // The builtin assume uses the same names as in C code
  // Retrieve lowerbound and upperbound if they are "hidden" in cast
  // instructions to match the names used in C
  Value *uppercast = UpperBound;
  if (Instruction *Iupper = dyn_cast<Instruction>(UpperBound)) {
    switch (Iupper->getOpcode()) {
    case Instruction::Trunc:
    case Instruction::ZExt:
    case Instruction::SExt:
      uppercast = Iupper->getOperand(0);
      break;
    }
  }
  Value *lowercast = LowerBound;
  if (Instruction *Ilower = dyn_cast<Instruction>(LowerBound)) {
    switch (Ilower->getOpcode()) {
    case Instruction::Trunc:
    case Instruction::ZExt:
    case Instruction::SExt:
      lowercast = Ilower->getOperand(0);
      break;
    }
  }

  // TODO:
  // This does only handle a unit step
  // The general formula is hi - lo < PE --> no zero trip count
  // this will be ajusted to hi - lo < PE*step
  AssumptionCache *AC =
      &getAnalysis<AssumptionCacheTracker>().getAssumptionCache(*F);
  // case 1 lower bound is a constant and upper bound is a constant
  if (ConstantInt *CI = dyn_cast<ConstantInt>(LowerBound)) {
    if (ConstantInt *CU = dyn_cast<ConstantInt>(UpperBound)) {
      int min_iterations = CU->getSExtValue();
      int lowerC = CI->getSExtValue();
      return min_iterations - lowerC;
    }
  }
  // case2: lower bound of loop is not a constant
  for (auto &AssumeVH : AC->assumptions()) {
    if (!AssumeVH)
      continue;
    auto *I = cast<CallInst>(AssumeVH);
    if (ICmpInst *ICL = dyn_cast<ICmpInst>(I->getArgOperand(0))) {
      Value *LHS = ICL->getOperand(0);
      Value *RHS = ICL->getOperand(1);
      CmpInst::Predicate Predicate = ICL->getPredicate();
      // case2 lower bound of loop is constant and upper bound is not
      if (ConstantInt *CI = dyn_cast<ConstantInt>(LowerBound)) {
        if (LHS == uppercast) {
          if (Predicate == ICmpInst::ICMP_UGT ||
              Predicate == ICmpInst::ICMP_SGT) {
            if (ConstantInt *CRHS = cast<ConstantInt>(RHS)) {
              int min_iterations = CRHS->getSExtValue();
              int lowerC = CI->getSExtValue();
              return min_iterations - lowerC;
            }
          }
        }
      }
      // lower bound is not a constant
      // case 3: yend>ybegin
      if (LHS == uppercast && RHS == lowercast) {
        if (Predicate == ICmpInst::ICMP_UGT || Predicate == ICmpInst::ICMP_SGT)
          return 0;
      }
      // lower bound is not a constant
      // Case 4: yend - ybegin> X
      if (Instruction *Isub = dyn_cast<Instruction>(LHS)) {
        if (Isub->getOpcode() == Instruction::Sub) {
          Value *upper = Isub->getOperand(0);
          Value *lower = Isub->getOperand(1);
          if (upper == uppercast && lower == lowercast) {
            if (isa<Constant>(RHS)) {
              // Right now we support only the greater than case
              if (Predicate == ICmpInst::ICMP_UGT ||
                  Predicate == ICmpInst::ICMP_SGT) {
                if (ConstantInt *CRHS = cast<ConstantInt>(RHS)) {
                  int min_iterations = CRHS->getSExtValue();
                  return min_iterations;
                }
                // if ((Predicate == ICmpInst::ICMP_EQ))
                // //TODO
                // else if (Predicate == ICmpInst::ICMP_ULT ||
                // Predicate == ICmpInst::ICMP_SLT)
                // //TODO
                // else if (Predicate == ICmpInst::ICMP_ULE ||
                // Predicate == ICmpInst::ICMP_SLE)
                // //TODO
                // else if (Predicate == ICmpInst::ICMP_UGE ||
                // Predicate == ICmpInst::ICMP_SGE)
                // TODO
              }
            }
          }
        }
      }
    }
  }
  return -1;
}

void LoopSPMDization::AddZeroTripCountCheck(Loop *L, ScalarEvolution *SE,
                                            int PE, int NPEs,
                                            BasicBlock *AfterLoop,
                                            DominatorTree *DT, LoopInfo *LI) {

  BasicBlock *PreHeader = L->getLoopPreheader();
  BranchInst *PreHeaderBR = cast<BranchInst>(PreHeader->getTerminator());
  BasicBlock *Latch = L->getLoopLatch();
  IRBuilder<> B(PreHeaderBR);
  BranchInst *LatchBR = cast<BranchInst>(Latch->getTerminator());
  Instruction *CondI = dyn_cast<Instruction>(Cond);

  Value *TripCount = CondI->getOperand(1);
  Value *IdxCmp;
  CmpInst *CmpCond = dyn_cast<CmpInst>(Cond);
  Instruction *CmpZeroTrip;
  Value *NewCondOp0, *NewCondOp1;
  if (cast<IntegerType>(NewInitV->getType())->getBitWidth() >
      cast<IntegerType>(TripCount->getType())->getBitWidth()) {
    auto *Trunc = B.CreateTrunc(NewInitV, TripCount->getType(),
                                NewInitV->getName() + ".trunk");
    NewInitV = Trunc;
  } else {
    auto *Trunc = B.CreateSExt(NewInitV, TripCount->getType(),
                               NewInitV->getName() + ".sext");
    NewInitV = Trunc;
  }

  NewCondOp1 = NewInitV;
  //do not use the one in cond if the loop is ZT in Blocking because it does
  //reflect loop count anymore
  if(spmd_approach == SPMD_BLOCKING)
    NewCondOp0 = UpperBound;
  else
    NewCondOp0 = TripCount;

  if (CmpCond->getPredicate() == CmpInst::ICMP_EQ ||
      CmpCond->getPredicate() == CmpInst::ICMP_NE) {
    if ((LatchBR->getSuccessor(0) == L->getHeader() &&
         CmpCond->getPredicate() == CmpInst::ICMP_EQ) ||
        (LatchBR->getSuccessor(0) != L->getHeader() &&
         CmpCond->getPredicate() == CmpInst::ICMP_NE))
      IdxCmp = CmpInst::Create(Instruction::ICmp, CmpInst::ICMP_SLT, NewCondOp0,
                               NewCondOp1, Cond->getName());

    else
      IdxCmp = CmpInst::Create(Instruction::ICmp, CmpInst::ICMP_SGE, NewCondOp0,
                               NewCondOp1, Cond->getName());
    CmpZeroTrip = cast<Instruction>(IdxCmp);
  } else { // in other cases, we keep the same predicate
    CmpZeroTrip = CondI->clone();
    IdxCmp = cast<Value>(CmpZeroTrip);
    if (cast<Instruction>(IdxCmp)->getOperand(1) == TripCount) {
      cast<Instruction>(IdxCmp)->setOperand(0, NewCondOp1);
      cast<Instruction>(IdxCmp)->setOperand(1, NewCondOp0);
    } else {
      cast<Instruction>(IdxCmp)->setOperand(1, NewCondOp1);
      cast<Instruction>(IdxCmp)->setOperand(0, NewCondOp0);
    }
  }
  PreHeader->getInstList().insert(B.GetInsertPoint(), CmpZeroTrip);

  // need to distringuish cases
  if (LatchBR->getSuccessor(0) == PreHeaderBR->getSuccessor(0))
    B.CreateCondBr(IdxCmp, PreHeaderBR->getSuccessor(0), AfterLoop);
  else
    B.CreateCondBr(IdxCmp, AfterLoop, PreHeaderBR->getSuccessor(0));

  PreHeaderBR->eraseFromParent();

  BasicBlock *NewPH = InsertPreheaderForLoop(L, DT, LI, nullptr, true);
  if (ZTCType == ZTCMode::Nested) {
    // Move section entry from .e block to the  new preheader to avoid bad
    // section placement
    IRBuilder<> BPH(NewPH->getFirstNonPHI());

    for (Instruction &inst : *NewPH->getSinglePredecessor())
      if (IntrinsicInst *intr_inst = dyn_cast<IntrinsicInst>(&inst))
        if (intr_inst->getIntrinsicID() ==
            Intrinsic::csa_parallel_section_entry) {
          (&inst)->moveBefore(NewPH->getFirstNonPHI());
          break;
        }
  }
  return;
}

bool LoopSPMDization::AddParallelIntrinsicstoLoop(Loop *L, LLVMContext &context,
                                                  Module *M, BasicBlock *OrigPH,
                                                  BasicBlock *E) {
  Function *FIntr =
      Intrinsic::getDeclaration(M, Intrinsic::csa_parallel_region_entry);
  Instruction *const header_terminator = OrigPH->getTerminator();
  Instruction *const preheader_terminator =
      L->getLoopPreheader()->getTerminator();
  CallInst *region_entry = IRBuilder<>{header_terminator}.CreateCall(
      FIntr, ConstantInt::get(IntegerType::get(context, 32), 1), "spmd_pre");
  std::string RegionName = region_entry->getName();
  next_token = context.getMDKindID(RegionName) + 1000;
  region_entry->setOperand(
      0, ConstantInt::get(IntegerType::get(context, 32), next_token));
  CallInst *section_entry = IRBuilder<>{preheader_terminator}.CreateCall(
      Intrinsic::getDeclaration(M, Intrinsic::csa_parallel_section_entry),
      region_entry, "spmd_pse");

  // IRBuilder<>{preheader_terminator}.CreateCall(
  //              Intrinsic::getDeclaration(M, Intrinsic::csa_parallel_loop));

  // The csa.parallel.region.exit intrinsic goes at the beginning of the loop
  // exit.
  SmallVector<BasicBlock *, 2> exits;
  L->getExitBlocks(exits);
  for (BasicBlock *const exit : exits) {
    IRBuilder<>{exit->getFirstNonPHI()}.CreateCall(
        Intrinsic::getDeclaration(M, Intrinsic::csa_parallel_section_exit),
        section_entry);
  }
  IRBuilder<>{E->getFirstNonPHI()}.CreateCall(
      Intrinsic::getDeclaration(M, Intrinsic::csa_parallel_region_exit),
      region_entry);

  return true;
}
