//===- CSAInnerLoopPrep.cpp - ------------------------------*- C++ -*--===//
//
// Copyright (C) 2017-2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
// This pass identifies candidates for inner loop pipelining based on CSA IR
// intrinsics. Serial loop nested directly within loops marked as parallel
// (with __builtin_csa_parallel_loop) can be automatically selected, or loops
// marked with __builtin_csa_pipeline_loop can be manually selected. It serves
// as an IR interface to the backend's ILPL code generation.
//
// If a Loop is selected, a single pseudoinstruction is added which serves as a
// signal to the post-ISel backend that the loop was selected and should have
// its code generation done differently.
//
//===----------------------------------------------------------------------===//
//
// \file
//
//===----------------------------------------------------------------------===//

#include "CSATargetMachine.h"
#include "Intel_CSA/Transforms/Scalar/CSALowerParallelIntrinsics.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/OptimizationRemarkEmitter.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/Support/Debug.h"
using namespace llvm;
using namespace llvm::PatternMatch;

#define DEBUG_TYPE "csa-ilpl-prep"
#define REMARK_NAME "csa-ilpl"
#define PASS_DESC                                                              \
  "CSA: Identify and prepare inner loops for pipelining of multiple executions"

static cl::opt<bool> EnableILPLPrep(
  "csa-ilpl-prep", cl::Hidden,
  cl::desc(
    "CSA Specific: enable/disable inner loop pipelining preparation pass"),
  cl::init(true));

static cl::alias EnableILPL(
  "csa-ilpl",
  cl::desc("Enable inner loop pipelining. (Alias for -csa-ilpl-prep.)"),
  cl::aliasopt(EnableILPLPrep));

enum ILPLSelectionMode {
  automatic = 0,
  manual    = 1,
  both      = 2,
  disabled  = 3,
};

static cl::opt<ILPLSelectionMode> SelectionMode(
  "csa-ilpl-selection", cl::Hidden,
  cl::desc("CSA specific: choose how pipelineable inner loops are discovered"),
  cl::values(
    clEnumVal(automatic, "automatically choose inner loops to pipeline"),
    clEnumVal(manual, "pipeline according to __builtin_csa_pipellineable_loop"),
    clEnumVal(both,
              "use both specified and auto-discovered pipelining candidates"),
    clEnumVal(disabled, "disable inner loop pipelining")),
  cl::init(ILPLSelectionMode::manual));

static cl::opt<int> DefaultDegreeOfPipeliningParallelism(
  "csa-ilpl-tokens", cl::Hidden,
  cl::desc(
    "CSA Specific: degree of concurrency allowed by inner loop pipelining"),
  cl::init(32));

namespace {
struct CSAInnerLoopPrep : public FunctionPass {
  static char ID;

  explicit CSAInnerLoopPrep() : FunctionPass(ID) {
    initializeCSAInnerLoopPrepPass(*PassRegistry::getPassRegistry());
  }
  StringRef getPassName() const override { return PASS_DESC; }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<PostDominatorTreeWrapperPass>();
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequired<OptimizationRemarkEmitterWrapperPass>();
    AU.setPreservesAll();
  }

  bool runOnFunction(Function &F) override;
  bool runOnLoop(Loop *L);

  DominatorTree *DT;
  PostDominatorTree *PDT;
  LoopInfo *LI;
  OptimizationRemarkEmitter *ORE;
  bool isLoopMarkedParallel(Loop *L);
  bool containsPipelinedLoop(Loop *L);
  uint64_t automaticallyPipelineable(Loop *L);
  uint64_t programmerSpecifiedPipelineable(Loop *L);
  bool containsMemoryLifetimeMarkers(Loop *L);
  const DebugLoc &getAnyBlockLoc(BasicBlock*);

  // Eventually, particularly when the simulator switches to default to shallow
  // LICs, this needs to be implemented.
  unsigned getMaxDegreeOfParallelism(Loop *L) {
    if (containsMemoryLifetimeMarkers(L))
      return 1;
    return DefaultDegreeOfPipeliningParallelism;
  };
};
} // namespace

char CSAInnerLoopPrep::ID = 0;
INITIALIZE_PASS_BEGIN(CSAInnerLoopPrep, "csa-inner-loop-prep", PASS_DESC, false,
                      false)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(PostDominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(OptimizationRemarkEmitterWrapperPass)
INITIALIZE_PASS_END(CSAInnerLoopPrep, "csa-inner-loop-prep", PASS_DESC, false,
                    false)

Pass *llvm::createCSAInnerLoopPrepPass() { return new CSAInnerLoopPrep(); }

bool CSAInnerLoopPrep::runOnFunction(Function &F) {
  bool Changed = false;

  if (skipFunction(F))
    return Changed;

  if (not EnableILPLPrep or SelectionMode == ILPLSelectionMode::disabled)
    return Changed;

  DT  = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  PDT = &getAnalysis<PostDominatorTreeWrapperPass>().getPostDomTree();
  LI  = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  ORE = &getAnalysis<OptimizationRemarkEmitterWrapperPass>().getORE();

  for (Loop *L : LI->getLoopsInPreorder())
    Changed |= runOnLoop(L);

  return Changed;
}

// StructurizeCFG tends to remove the DebugLoc of the one terminator
// instruction that "Loop::getLoopLoc" looks for. Find ANYTHING else here.
const DebugLoc &CSAInnerLoopPrep::getAnyBlockLoc(BasicBlock *b) {
  for(Instruction &i : *b)
    if (const DebugLoc &iLoc = i.getDebugLoc())
      return iLoc;

  return b->getTerminator()->getDebugLoc();
}

bool CSAInnerLoopPrep::runOnLoop(Loop *L) {
  bool Changed         = false;
  bool IntrinsicDriven = false;

  Loop *outerLoop = L->getParentLoop();
  if (outerLoop == nullptr)
    return Changed;

  uint64_t pipeliningDepth = 1;

  if (containsPipelinedLoop(L)) {
    LLVM_DEBUG(errs() << "Won't try to pipeline loop " <<
               L->getHeader()->getName() <<
               "; it contains a pipelined loop.\n");
    return Changed;
  }

  if (pipeliningDepth == 1 and (SelectionMode == ILPLSelectionMode::manual or
                                SelectionMode == ILPLSelectionMode::both)) {
    pipeliningDepth = programmerSpecifiedPipelineable(L);
    if (pipeliningDepth > 1) {
      LLVM_DEBUG(errs() <<
                 "Programmer has specified " << L->getName() << "(depth "
                 << L->getLoopDepth() << ") as pipelineable with respect to "
                 << outerLoop->getName() << " (depth "
                 << outerLoop->getLoopDepth() << ").\n");
      IntrinsicDriven = true;
    }
  }

  if (pipeliningDepth == 1 and (SelectionMode == ILPLSelectionMode::automatic or
                                SelectionMode == ILPLSelectionMode::both)) {
    pipeliningDepth = automaticallyPipelineable(L);
    if (pipeliningDepth > 1) {
      LLVM_DEBUG(errs() << "Automatically discovered loop " << L->getName()
                 << "(depth " << L->getLoopDepth()
                 << ") as pipelineable with respect to "
                 << outerLoop->getName() << " (depth "
                 << outerLoop->getLoopDepth() << ").\n");
    }
  }

  if (pipeliningDepth == 1)
    return Changed;

  // Note that this loop was selected.
  LLVMContext &Context = L->getHeader()->getContext();
  Module *m            = L->getHeader()->getParent()->getParent();
  IRBuilder<>{L->getHeader()->getTerminator()}.CreateCall(
    Intrinsic::getDeclaration(m, Intrinsic::csa_pipelineable_loop_marker),
    ConstantInt::get(IntegerType::get(Context, 64), pipeliningDepth));
  Changed = true;

  // Sometimes StructurizeCFG has murdered our block terminator DebugLocs. Ask
  // Loop for its idea of a start location, but fall back to just anything in
  // the header otherwise.
  DebugLoc loopLoc = L->getStartLoc() ? L->getStartLoc() :
    getAnyBlockLoc(L->getHeader());
  const Value* loopCode = L->getHeader();

  using namespace ore;
  // Report the optimization.
  if (IntrinsicDriven) {
    ORE->emit(
      OptimizationRemark(REMARK_NAME, "ILPLPrepDone", loopLoc,
                         loopCode)
      << "selected loop for inner loop pipelining via directive with depth "
      << NV("PipeliningDepth", (unsigned)pipeliningDepth));
  } else {
    ORE->emit(
      OptimizationRemark(REMARK_NAME, "ILPLPrepDone", loopLoc,
                         loopCode)
      << "automatically selected loop for inner loop pipelining with depth "
      << NV("PipeliningDepth", (unsigned)pipeliningDepth));
  }

  return Changed;
}

bool CSAInnerLoopPrep::isLoopMarkedParallel(Loop *L) {
  if (auto *LoopID = L->getLoopID()) {
    LLVM_DEBUG(dbgs() << "Loop with metadata: "
               << L->getHeader()->getName() << "\n");
    for (unsigned Indx = 1; Indx < LoopID->getNumOperands(); ++Indx) {
      if (auto *T = dyn_cast<MDTuple>(LoopID->getOperand(Indx)))
        if (T->getNumOperands() != 0)
          if (auto *S = dyn_cast<MDString>(T->getOperand(0)))
            if (S->getString() == CSALoopTag::Parallel) {
              LLVM_DEBUG(dbgs() << "The loop is marked with Parallel.\n");
              return true;
            }
    }
  }

  for (BasicBlock *bb : L->blocks()) {
    for (Instruction &i : *bb) {
      Instruction *section_exit  = &i;
      Instruction *section_entry = nullptr;
      Instruction *region_entry  = nullptr;
      Value *regionId            = nullptr;
      if (match(section_exit, m_Intrinsic<Intrinsic::csa_parallel_section_exit>(
                                m_Instruction(section_entry))) and
          match(section_entry,
                m_Intrinsic<Intrinsic::csa_parallel_section_entry>(
                  m_Instruction(region_entry))) and
          match(region_entry, m_Intrinsic<Intrinsic::csa_parallel_region_entry>(
                                m_Value(regionId))) and
          LI->getLoopFor(section_entry->getParent()) == L and
          LI->getLoopFor(section_exit->getParent()) == L) {
        if (DT->dominates(section_entry->getParent(), L->getHeader()) and
            PDT->dominates(section_exit->getParent(), L->getHeader())) {
          LLVM_DEBUG(errs() << "Found (region id is " << *regionId << "):\n"
                     << "\t" << *region_entry << "\n"
                     << "\t\t" << *section_entry << " (depth "
                     << L->getLoopDepth() << ")\n"
                       << "\t\t" << *section_exit << "\n");
          return true;
        }
      }
    }
  }
  return false;
}

bool CSAInnerLoopPrep::containsPipelinedLoop(Loop *L) {
  if (nullptr == L)
    return false;

  for (BasicBlock *bb : L->blocks()) {
    for (Instruction &i : *bb) {
      Instruction *pipelineable_marker = &i;
      Value *numTokens                 = nullptr;
      if (match(pipelineable_marker,
                m_Intrinsic<Intrinsic::csa_pipelineable_loop_marker>(
                  m_Value(numTokens)))) {
        return true;
      }
    }
  }
  return false;
}

uint64_t CSAInnerLoopPrep::programmerSpecifiedPipelineable(Loop *L) {
  if (nullptr == L || nullptr == L->getParentLoop())
    return 1;

  for (BasicBlock *bb : L->getParentLoop()->blocks()) {
    for (Instruction &i : *bb) {
      Instruction *pipeline_loop_exit  = &i;
      Instruction *pipeline_loop_entry = nullptr;
      ConstantInt *pipeliningDepth     = nullptr;
      if (match(pipeline_loop_exit,
                m_Intrinsic<Intrinsic::csa_pipeline_loop_exit>(
                  m_Instruction(pipeline_loop_entry))) and
          match(pipeline_loop_entry,
                m_Intrinsic<Intrinsic::csa_pipeline_loop_entry>(
                  m_ConstantInt(pipeliningDepth)))) {

        // The parent loop has a "pipeline_loop" directive with it, referring
        // to some child loop. Is that child loop us?
        if (not DT->dominates(pipeline_loop_entry->getParent(), L->getHeader()) or
            not PDT->dominates(pipeline_loop_exit->getParent(), L->getHeader()))
          continue;

        // Also verify that the parent loop is marked as a parallel loop.
        Loop *parent = L->getParentLoop();
        if (not isLoopMarkedParallel(parent)) {

          // Sometimes StructurizeCFG has murdered our block terminator DebugLocs. Ask
          // Loop for its idea of a start location, but fall back to just anything in
          // the header otherwise.
          DebugLoc loopLoc = L->getStartLoc() ? L->getStartLoc() :
            getAnyBlockLoc(L->getHeader());

          // Report the missed optimization.
          ORE->emit(OptimizationRemarkMissed(REMARK_NAME,
                                             "ILPLDirectiveIgnored",
                                             loopLoc, L->getHeader())
                    << " ignoring pipelining directive; not in parallel loop");

          continue;
        }

        if (pipeliningDepth->isZero())
          return getMaxDegreeOfParallelism(L);
        else
          return pipeliningDepth->getZExtValue();
      }
    }
  }
  return 1;
}

uint64_t CSAInnerLoopPrep::automaticallyPipelineable(Loop *L) {
  if (nullptr == L)
    return 1;

  // This loop is itself marked as parallel, do not attept to pipeline.
  if (isLoopMarkedParallel(L))
    return 1;

  if (L->getParentLoop() and isLoopMarkedParallel(L->getParentLoop()))
    return getMaxDegreeOfParallelism(L);

  return 1;
}

bool CSAInnerLoopPrep::containsMemoryLifetimeMarkers(Loop *L) {
  assert(nullptr != L);

  for (BasicBlock *bb : L->blocks()) {
    for (Instruction &i : *bb) {
      Instruction *lifetime_start = &i;
      Value *allocSize            = nullptr;
      Value *memory               = nullptr;
      if (match(lifetime_start, m_Intrinsic<Intrinsic::lifetime_start>(
                                  m_Value(allocSize), m_Value(memory)))) {
        LLVM_DEBUG(errs() << "Found lifetime.start:\n\t" <<
                   *lifetime_start << "\n");
        return true;
      }
    }
  }

  return false;
}
