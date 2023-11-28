//===- CSALoopIntrinsicExpander.cpp - Expand loop intrinsics --------===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------===//
//
// This pass locates loops marked with CSA-specific loop intrinsics and
// expands them into their underlying representations. It must be run
// before any passes that could move the intrinsics around, but must also
// run after loop simplification (since it expects normal form loops),
// memory to register promotion (so that it can identify real memory
// operations rather than ones that just access local variables), and the
// Fortran intrinsic converter pass (so that it recognizes Fortran
// "intrinsics" too).
//
//===----------------------------------------------------------------===//

#include "CSALoopIntrinsicExpander.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/IntrinsicsCSA.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Utils/LoopUtils.h"

#include <algorithm>
#include <cassert>
#include <iterator>

#define DEBUG_TYPE "csa-loop-intrinsic-expander"

STATISTIC(NumLoopIntrinsicExpansions,
          "Number of parallel loop intrinsics expanded");
STATISTIC(NumSPMDIntrinsicExpansions, "Number of SPMD intrinsics expanded");
STATISTIC(NumILPLIntrinsicExpansions,
          "Number of pipeline loop intrinsics expanded");

using namespace llvm;

namespace {

// An RAII type for temporarily creating a dummy exit node in a function which
// can be used to temporarily re-route edges that should be ignored. Usage
// example:
//
// {
//   DummyExitNode dummy_exit {F, DT, PDT};
//
//   // The dummy exit node has now been added and can be accessed via the
//   implicit
//   // conversion.
//   do_bb_stuff(dummy_exit);
//
//   // The dummy exit node is automatically removed from the function at the
//   end
//   // of the scope
// }
class DummyExitNode {
  BasicBlock *dummy_exit;
  DominatorTree *DT;
  PostDominatorTree *PDT;

public:
  DummyExitNode(Function &F, DominatorTree *DT_in, PostDominatorTree *PDT_in)
      : dummy_exit{BasicBlock::Create(F.getContext(), "dummy_exit", &F)},
        DT{DT_in}, PDT{PDT_in} {
    PDT->recalculate(F);
  }
  ~DummyExitNode() {
    Function &F = *dummy_exit->getParent();
    dummy_exit->eraseFromParent();
    PDT->recalculate(F);
  }
  operator BasicBlock *() { return dummy_exit; }
};

// An RAII type for re-routing an edge. This implementation assumes that no
// edges exist from the current from block to the new to one before this
// rerouting and will produce the wrong result when undoing the rerouting if any
// do. Usage example:
//
// {
//   TempEdgeRerouter edge_rerouter {from, to, new_to, DT, PDT};
//
//   // The edge is re-routed here and the (post)dominator trees are updated to
//   // reflect that change.
//
//   // The edge will atomatically be returned to its original state at the end
//   of
//   // the scope.
// }
class TempEdgeRerouter {
  BasicBlock *from, *old_to, *to;
  DominatorTree *DT;
  PostDominatorTree *PDT;

public:
  TempEdgeRerouter(BasicBlock *from_in, BasicBlock *to_in,
                   BasicBlock *new_to_in, DominatorTree *DT_in,
                   PostDominatorTree *PDT_in)
      : from{from_in}, old_to{to_in}, to{new_to_in}, DT{DT_in}, PDT{PDT_in} {
    from->getTerminator()->replaceUsesOfWith(old_to, to);
    DT->deleteEdge(from, old_to);
    PDT->deleteEdge(from, old_to);
    DT->insertEdge(from, to);
    PDT->insertEdge(from, to);
  }
  ~TempEdgeRerouter() {
    from->getTerminator()->replaceUsesOfWith(to, old_to);
    DT->deleteEdge(from, to);
    PDT->deleteEdge(from, to);
    DT->insertEdge(from, old_to);
    PDT->insertEdge(from, old_to);
  }
};

struct CSALoopIntrinsicExpander : FunctionPass {
  static char ID;

  CSALoopIntrinsicExpander() : FunctionPass{ID} {}

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequired<DominatorTreeWrapperPass>();
  }

  bool runOnFunction(Function &) override;

  StringRef getPassName() const override {
    return "CSA: Expand CSA-specific loop intrinsics";
  }

private:
  LoopInfo *LI;
  DominatorTree *DT;
  PostDominatorTree *PDT;

  // The current begin/end blocks for the section. These should be initialized
  // to nullptr at the beginning of each loop expansion and are updated by
  // makeSectionInclude.
  BasicBlock *cur_section_begin, *cur_section_end;

  // The current loop being worked on, so that makeSectionInclude can check
  // whether basic blocks are part of it.
  const Loop *cur_loop;

  // Recursively iterates a loop and its subloops and attempts expansions.
  void recurseLoops(Loop *, BasicBlock *dummy_exit);

  // Checks whether a particular instruction is a parallel loop intrinsic.
  // If so, returns a pointer to it as an IntrinsicInst; otherwise returns nullptr.
  IntrinsicInst *isParLoopIntrinsic(Instruction &) const;

  // Checks whether a particular instruction is a local_cache intrinsic.
  // If so, returns a pointer to it as an IntrinsicInst; otherwise returns nullptr.
  IntrinsicInst *isLocalCacheIntrinsic(Instruction &) const;

  // Looks for a pipelining/parallel loop/SPMDization intrinsic in the
  // blocks before the loop. If none is found this will return nullptr.
  IntrinsicInst *detectParLoopIntrinsic(Loop *) const;

  // Looks for a local cache intrinsic in the blocks before the loop.
  // If none is found this will return nullptr.
  IntrinsicInst *detectLocalCacheIntrinsic(Loop *) const;

  // Looks for the specified IID intrinsic in the blocks before the loop.
  // If none is found, this will return nullptr.
  IntrinsicInst *detectIntrinsic(Loop *, Intrinsic::ID IID) const;

  // Attempts to expand a given loop. Returns true if it succeeded, false if it
  // ran into issues.
  bool expandParLoopIntrinsic(Loop *, IntrinsicInst *intr, IntrinsicInst *lcache_intr, BasicBlock *dummy_exit);

  // Attempts to expand a given loop. Returns true if it succeeded, false if it
  // ran into issues.
  bool expandLocalCacheIntrinsic(Loop *, IntrinsicInst *intr);

  // Attempts to expand the section to include a given block. If that is not
  // possible, this will return false.
  bool makeSectionInclude(BasicBlock *);

  // Checks whether a given block is already included in the section.
  bool isAlreadyInSection(BasicBlock *) const;

  // Determines whether an instruction might need ordering (and therefore should
  // be in a section). In general those instructions are just the ones that
  // might touch memory, but since mayReadOrWriteMemory doesn't pick up inline
  // assembly calls (which might contain loads) this function needs to check for
  // those explicitly.
  bool mayNeedOrdering(const Instruction *) const;

  // Removes all parallel loop/(unpaired) SPMDization intrinsics in a function.
  // Returns true if any intrinsics were removed, false otherwise.
  bool removeIntrinsics(Function &);
};

char CSALoopIntrinsicExpander::ID = 0;

bool CSALoopIntrinsicExpander::runOnFunction(Function &F) {

  // Go ahead and expand intrinsics, but only if optimizations are enabled.
  if (not skipFunction(F)) {

    LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
    DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
    PostDominatorTree local_pdt;
    PDT = &local_pdt;

    // Create a dummy exit here in anticipation of possible backedge rerouting.
    DummyExitNode dummy_exit{F, DT, PDT};

    for (Loop *L : *LI)
      recurseLoops(L, dummy_exit);
  }

  // Make sure to clean up all of the loop intrinsics whether or not they've
  // been expanded.
  return removeIntrinsics(F);
}

void CSALoopIntrinsicExpander::recurseLoops(Loop *L, BasicBlock *dummy_exit) {

  // Handle the subloops here.
  for (Loop *const subloop : L->getSubLoops())
    recurseLoops(subloop, dummy_exit);

  // This pass is only equipped to handle normal form loops. This shouldn't be
  // an issue if LoopSimplify is run before this pass, but this is a check just
  // to make sure.
  if (not L->isLoopSimplifyForm()) {
    LLVM_DEBUG(errs() <<
               "NON-LOOPSIMPLIFIED LOOP FOUND!\nDid LoopSimplify run?\n");
    return;
  }

  // If the loop should be expanded, expand it or complain if there's something
  // wrong with it.
  IntrinsicInst *parloop_intr = detectParLoopIntrinsic(L);
  IntrinsicInst *lcache_intr  = detectLocalCacheIntrinsic(L);

  if (parloop_intr) {
    // There is a parallel loop intrisics on the loop.
    bool ret = expandParLoopIntrinsic(L, parloop_intr, lcache_intr, dummy_exit);
    if (not ret) {
      errs() << "\n";
      errs().changeColor(raw_ostream::BLUE, true);
      errs() << "!! WARNING: COULD NOT PARALLELIZE LOOP !!";
      errs().resetColor();
#if RAVI
      const DebugLoc& loc = parloop_intrinsic->getDebugLoc();
      if (loc) {
        errs() << R"help(
We were unable to automatically identify a unique section for the loop at
)help";
        loc.print(errs());
      } else {
        errs() << R"help(
We were unable to automatically identify a unique section for a loop marked
with a CSA loop builtin. Use -g for location information.)help";
      }
#endif
      errs() << R"help(

This was likely caused by either having multiple loop exits or by having memory
operations in the loop control. Please mark the regions and sections for this
loop explicitly with __builtin_csa_parallel_{region,section}_{entry,exit}()
instead.

)help";
      return;
    }
  }
  else if (lcache_intr) {
  // There are no parallel loop intrisics on the loop, but there
  // is a local cache intrinsic, expand the local cache intrinsic.
    expandLocalCacheIntrinsic(L, lcache_intr);
  }
}

IntrinsicInst *
CSALoopIntrinsicExpander::isParLoopIntrinsic(Instruction &inst) const {
  IntrinsicInst *const intr_inst = dyn_cast<IntrinsicInst>(&inst);
  if (intr_inst and
      (intr_inst->getIntrinsicID() == Intrinsic::csa_parallel_loop or
       intr_inst->getIntrinsicID() == Intrinsic::csa_pipeline_loop or
       intr_inst->getIntrinsicID() == Intrinsic::csa_spmdization or
       intr_inst->getIntrinsicID() == Intrinsic::csa_spmd))
    return intr_inst;
  return nullptr;
}

IntrinsicInst *
CSALoopIntrinsicExpander::isLocalCacheIntrinsic(Instruction &inst) const {
  IntrinsicInst *const intr_inst = dyn_cast<IntrinsicInst>(&inst);
  if (intr_inst and
      intr_inst->getIntrinsicID() == Intrinsic::csa_local_cache)
    return intr_inst;
  return nullptr;
}

IntrinsicInst *CSALoopIntrinsicExpander::detectParLoopIntrinsic(Loop *L) const {

  // Start iterating backwards at the preheader.
  for (BasicBlock *cur_block = L->getLoopPreheader();
       cur_block and
       LI->getLoopFor(cur_block) == LI->getLoopFor(L->getLoopPreheader());
       cur_block = cur_block->getSinglePredecessor()) {

    // Look for an intrinsic call with one of the right IDs.
    for (Instruction &inst : *cur_block) {
      if (IntrinsicInst *const intr_inst = isParLoopIntrinsic(inst)) {
        return intr_inst;
      }
    }
  }

  return nullptr;
}

IntrinsicInst *CSALoopIntrinsicExpander::detectLocalCacheIntrinsic(Loop *L) const {

  // Start iterating backwards at the preheader
  for (BasicBlock *cur_block = L->getLoopPreheader();
       cur_block and
       LI->getLoopFor(cur_block) == LI->getLoopFor(L->getLoopPreheader());
       cur_block = cur_block->getSinglePredecessor()) {

    // Look for an intrinsic call with the right ID.
    for (Instruction &inst : *cur_block) {
      if (IntrinsicInst *const intr_inst = isLocalCacheIntrinsic(inst)) {
        return intr_inst;
      }
    }
  }

  return nullptr;
}


IntrinsicInst *CSALoopIntrinsicExpander::detectIntrinsic(Loop *L, Intrinsic::ID IID) const {

  // Start iterating backwards at the preheader.
  for (BasicBlock *cur_block = L->getLoopPreheader();
       cur_block and
       LI->getLoopFor(cur_block) == LI->getLoopFor(L->getLoopPreheader());
       cur_block = cur_block->getSinglePredecessor()) {

    // Look for IID intrinsic call.
    for (Instruction &inst : *cur_block) {
      IntrinsicInst *const intr_inst = dyn_cast<IntrinsicInst>(&inst);
      if (intr_inst and
          (intr_inst->getIntrinsicID() == IID)) {
        return intr_inst;
      }
    }
  }

  return nullptr;
}

bool CSALoopIntrinsicExpander::removeIntrinsics(Function &F) {
  using namespace std;
  bool removed_any = false;
  for (BasicBlock &cur_block : F) {
    for (auto it = begin(cur_block); it != end(cur_block);) {
      if ((isParLoopIntrinsic(*it)) ||
          (isLocalCacheIntrinsic(*it))) {
        it          = it->eraseFromParent();
        removed_any = true;
      } else
        ++it;
    }
  }
  return removed_any;
}

bool CSALoopIntrinsicExpander::expandParLoopIntrinsic(Loop *L, 
                                                      IntrinsicInst *intr,
                                                      IntrinsicInst *lcache_intr,
                                                      BasicBlock *dummy_exit) {
  using namespace std;

  LLVMContext &context = L->getHeader()->getContext();
  Module *module       = L->getHeader()->getParent()->getParent();

  // Check whether there is a local cache intrinsic on the loop, in
  // addition to the intrinsic (intr) being handled now.
  Value *lcache_size;
  Value *lcache_id;
  CallInst *lcache_region_begin;

  if (lcache_intr) {
    assert(lcache_intr->getNumArgOperands() == 1 && "Bad local cache intrinsic?");
    lcache_size = lcache_intr->getArgOperand(0);
    lcache_id   = ConstantInt::get(IntegerType::get(context, 32), 0);
  }

  // If intr is a pipelining intrinsic, it needs to have its own entry and exit
  // inserted for the prep pass later. We don't need to find a parallel section
  // for it.
  if (intr->getIntrinsicID() == Intrinsic::csa_pipeline_loop) {
    Instruction *const preheader_terminator =
      L->getLoopPreheader()->getTerminator();
    assert(intr->getNumArgOperands() == 1 && "Bad pipeline_loop intrinsic?");
    Value *depthValue        = intr->getArgOperand(0);
    ConstantInt *parentDepth = dyn_cast<ConstantInt>(depthValue);
    assert(parentDepth && "Bad pipeline loop intrinsic arguments?");

    if (parentDepth->isNegative()) {
      errs() << "\n";
      errs().changeColor(raw_ostream::BLUE, true);
      errs() << "!! WARNING: COULD NOT PIPELINE INNER LOOP  !!";
      errs().resetColor();
      const DebugLoc &loc = intr->getDebugLoc();
      if (loc) {
        errs() << "\nThe loop at:\n  ";
        loc.print(errs());
        errs() << R"help(
was marked with __builtin_csa_pipeline_loop() but the specified depth depth
was less than zero. The number specified should be zero, indicating that the
compiler should choose piplining depth, or greater than zero, indicating the
specific number of concurrent instances allowed.

)help";
      } else {
        errs() << R"help(
A loop was marked with __builtin_csa_pipeline_loop() but the specified
depth depth was less than zero. The number specified should be zero, indicating
that the compiler should choose piplining depth, or greater than zero,
indicating the specific number of concurrent instances allowed. Re-run with -g
to see more location information.

)help";
      }
      // Note: returning "true" here because we did not fail to identify a
      // memory section.
      return true;
    }

    CallInst *const ilpl_entry = IRBuilder<>{preheader_terminator}.CreateCall(
      Intrinsic::getDeclaration(module, Intrinsic::csa_pipeline_loop_entry),
      intr->getArgOperand(0), "ilpl_entry");
    SmallVector<BasicBlock *, 2> exits;

    if (lcache_intr) {
      lcache_region_begin =  IRBuilder<>{preheader_terminator}.CreateCall(
         Intrinsic::getDeclaration(module, Intrinsic::csa_local_cache_region_begin),
         {lcache_size, lcache_id}, "lcache_region_begin");
    }

    L->getExitBlocks(exits);
    for (BasicBlock *const exit : exits) {
      IRBuilder<>{exit->getFirstNonPHI()}.CreateCall(
        Intrinsic::getDeclaration(module, Intrinsic::csa_pipeline_loop_exit),
        ilpl_entry);

      if (lcache_intr) {
        IRBuilder<>{exit->getFirstNonPHI()}.CreateCall(
            Intrinsic::getDeclaration(module, Intrinsic::csa_local_cache_region_end),
            {lcache_region_begin, lcache_id});
      }
    }

    // This is all that is needed for marking pipeline loops.
    ++NumILPLIntrinsicExpansions;
    return true;
  } // if (intr->getIntrinsicID() == Intrinsic::csa_pipeline_loop)

  cur_section_begin = cur_section_end = nullptr;
  cur_loop                            = L;

  {
    // Temporarily reroute the backedge to a dummy exit to avoid having sections
    // going across it while determining section extents.
    TempEdgeRerouter edge_rerouter{L->getLoopLatch(), L->getHeader(),
                                   dummy_exit, DT, PDT};

    // The section must contain the preheaders and exit blocks of every subloop.
    for (Loop *const subloop : L->getSubLoops()) {
      if (not makeSectionInclude(subloop->getLoopPreheader()))
        return false;
      SmallVector<BasicBlock *, 2> exits;
      subloop->getExitBlocks(exits);
      for (BasicBlock *const exit : exits) {
        if (not makeSectionInclude(exit))
          return false;
      }
    }

    // It should also contain any blocks with possible memory operations.
    for (auto block_it = L->block_begin(); block_it != L->block_end();
         ++block_it) {
      LLVM_DEBUG(errs() << "Looking at " << (*block_it)->getName() << "\n");

      // Skip any blocks that are already in the section.
      if (isAlreadyInSection(*block_it)) {
        LLVM_DEBUG(errs() << " already in section\n");
        continue;
      }

      for (const Instruction &instr : **block_it) {
        if (mayNeedOrdering(&instr)) {
          LLVM_DEBUG(errs() << " has memory operation:" << instr << "\n");
          if (not makeSectionInclude(*block_it))
            return false;
          break;
        }
      }
    }
  }

  // If intr is an SPMDization intrinsic, it needs to have its own entry and
  // exit inserted for the SPMDization pass later.
  if (intr->getIntrinsicID() == Intrinsic::csa_spmdization ||
      intr->getIntrinsicID() == Intrinsic::csa_spmd) {
    Instruction *const preheader_terminator =
      L->getLoopPreheader()->getTerminator();
    assert(intr->getNumArgOperands() == 2 && "Bad SPMDization intrinsic?");
    // In order to have one pair of spmd entry and exit, we transform here
    // spmdization to spmd intrinsic
    Value *chunk_size;
    if (intr->getIntrinsicID() == Intrinsic::csa_spmdization) {
      const auto emit_spmdization_warning = []() {
        errs() << "\n";
        errs().changeColor(raw_ostream::BLUE, true);
        errs() << "!! WARNING: BAD CSA SPMD INTRINSIC !!";
        errs().resetColor();
        return false;
      };

      const auto approach = dyn_cast<ConstantExpr>(intr->getArgOperand(1));
      if (not approach)
        return emit_spmdization_warning();

      // spmdization's string parameter is expected to be passed through a GEP.
      if (approach->getOpcode() != Instruction::GetElementPtr)
        return emit_spmdization_warning();

      // ...and to also be a constant C string.
      GlobalVariable *glob_arg =
        dyn_cast<GlobalVariable>(approach->getOperand(0));
      if (not glob_arg or not glob_arg->isConstant())
        return emit_spmdization_warning();
      const auto glob_array =
        dyn_cast_or_null<ConstantDataArray>(glob_arg->getInitializer());
      if (not glob_array or not glob_array->isCString())
        return emit_spmdization_warning();

      const StringRef user_approach = glob_array->getAsCString();
      if (user_approach.compare_insensitive("cyclic") == 0) {
        chunk_size = ConstantInt::get(IntegerType::get(context, 32), 1);
      } else if (user_approach.compare_insensitive("blocked") == 0 ||
                 user_approach.compare_insensitive("blocking") == 0 ||
                 user_approach.compare_insensitive("block") == 0) {
        chunk_size = ConstantInt::get(IntegerType::get(context, 32), 0);
      } else if (user_approach.compare_insensitive("hybrid") == 0) {
        // When using this SPMD syntax, we assume a fixed chunk size of 8
        chunk_size = ConstantInt::get(IntegerType::get(context, 32), 8);
      } else {
        errs() << "\n";
        errs().changeColor(raw_ostream::BLUE, true);
        errs() << "!! WARNING: BAD CSA SPMD INTRINSIC !!";
        errs().resetColor();
        errs() << " Second argument should be Cyclic, Block, Blocked, "
                  "Blocking, or Hybrid.\n"
                  "This call will be ignored.\n\n";
        return false;
      }
    } else {
      chunk_size = intr->getOperand(1);
    }

    CallInst *const spmdization_entry =
      IRBuilder<>{preheader_terminator}.CreateCall(
            Intrinsic::getDeclaration(module, Intrinsic::csa_spmdization_entry),
            {intr->getArgOperand(0), chunk_size}, "spmdization_entry");

    SmallVector<BasicBlock *, 2> exits;
    L->getExitBlocks(exits);
    for (BasicBlock *const exit : exits) {
      IRBuilder<>{exit->getFirstNonPHI()}.CreateCall(
        Intrinsic::getDeclaration(module, Intrinsic::csa_spmdization_exit),
        spmdization_entry);
    }

    // If there is a local cache intrinsic on the loop (in addition to
    // the SPMDization intrinsic), then add csa_spmdization_local_cache
    // instrinsic which will be expanded in LoopSPMDization.cpp
    if (lcache_intr) {
      IRBuilder<>{preheader_terminator}.CreateCall(
         Intrinsic::getDeclaration(module, Intrinsic::csa_spmdization_local_cache),
         {spmdization_entry, lcache_size});
    }

    // If there wasn't any memory use in the loop, the parallel region/section
    // expansion can just be ignored.
    if (not cur_section_begin or not cur_section_end)
      return true;
  } // end if (intr->getIntrinsicID() == Intrinsic::csa_spmdization ...

  // If this is a parallel loop intrinsic, it doesn't make much sense to expand
  // it if there weren't any memory references. Just emit a warning about that.
  else if (not cur_section_begin or not cur_section_end) {

    errs() << "\n";
    errs().changeColor(raw_ostream::BLUE, true);
    errs() << "!! WARNING: NO MEMORY OPERATIONS IN LOOP !!";
    errs().resetColor();
    const DebugLoc &loc = intr->getDebugLoc();
    if (loc) {
      errs() << "\nThe loop at:\n  ";
      loc.print(errs());
      errs() << R"help(
was marked with __builtin_csa_parallel_loop() but does not appear to contain
any memory operations that could be parallelized. This is probably a mistake.

)help";
    } else {
      errs() << R"help(
A loop marked with __builtin_csa_parallel_loop() does not appear to contain
any memory operations that could be parallelized. This is probably a mistake.
Re-run with -g to see more location information.

)help";
    }

    return true;
  } // end if (not cur_section_begin or not cur_section_end)

  // There should be enough information at this point to finish the expansion;
  // go ahead and update the statistic.
  ++(intr->getIntrinsicID() == Intrinsic::csa_parallel_loop
       ? NumLoopIntrinsicExpansions
       : NumSPMDIntrinsicExpansions);

  // The csa.parallel.region.entry intrinsic goes at the end of the preheader.
  Instruction *const preheader_terminator =
    L->getLoopPreheader()->getTerminator();

  CallInst *const region_entry = IRBuilder<>{preheader_terminator}.CreateCall(
    Intrinsic::getDeclaration(module, Intrinsic::csa_parallel_region_entry),
    ConstantInt::get(IntegerType::get(context, 32), 0), "clie_pre");
  const int region_token = context.getMDKindID(region_entry->getName()) + 1000;
  region_entry->setOperand(
    0, ConstantInt::get(IntegerType::get(context, 32), region_token));

  if (lcache_intr && (intr->getIntrinsicID() == Intrinsic::csa_parallel_loop)) {
    lcache_region_begin = IRBuilder<>{preheader_terminator}.CreateCall(
       Intrinsic::getDeclaration(module, Intrinsic::csa_local_cache_region_begin),
       {lcache_size, lcache_id}, "lcache_region_begin");
  }

  // The csa.parallel.region.exit intrinsic goes at the first merge point after
  // all the exits.
  SmallVector<BasicBlock *, 2> exits;
  L->getExitBlocks(exits);
  if (!exits.empty()) {
    BasicBlock *CommonPostDom = exits[0];
    for (BasicBlock *Exit : exits)
      CommonPostDom = PDT->findNearestCommonDominator(Exit, CommonPostDom);

    if (CommonPostDom) {
      IRBuilder<>{CommonPostDom->getFirstNonPHI()}.CreateCall(
        Intrinsic::getDeclaration(module, Intrinsic::csa_parallel_region_exit),
        region_entry);

      if (lcache_intr && (intr->getIntrinsicID() == Intrinsic::csa_parallel_loop)) {
        IRBuilder<>{CommonPostDom->getFirstNonPHI()}.CreateCall(
            Intrinsic::getDeclaration(module, Intrinsic::csa_local_cache_region_end),
            {lcache_region_begin, lcache_id});
      }
    } else {
      for (BasicBlock * Exit : exits) {
        IRBuilder<>{Exit->getFirstNonPHI()}.CreateIntrinsic(
          Intrinsic::csa_parallel_region_exit, {}, region_entry);

        if (lcache_intr && (intr->getIntrinsicID() == Intrinsic::csa_parallel_loop)) {
          IRBuilder<>{Exit->getFirstNonPHI()}.CreateCall(
              Intrinsic::getDeclaration(module, Intrinsic::csa_local_cache_region_end),
              {lcache_region_begin, lcache_id});
	}
      }
    }
  }

  // The csa.parallel.section.entry instrinsic goes before the first memory
  // instruction (or terminator) of the section start block.
  const auto first_mem =
    find_if(cur_section_begin->getFirstInsertionPt(), cur_section_begin->end(),
            [this](Instruction &instr) {
              return mayNeedOrdering(&instr) or instr.isTerminator();
            });
  CallInst *const section_entry = IRBuilder<>{&*first_mem}.CreateCall(
    Intrinsic::getDeclaration(module, Intrinsic::csa_parallel_section_entry),
    region_entry, "clie_pse");

  // The csa.parallel.section.exit intrinsic goes after the last memory
  // instruction (or at the beginning) of the section end block.
  const auto last_mem =
    find_if(cur_section_end->rbegin(), cur_section_end->rend(),
            [this](Instruction &instr) { return mayNeedOrdering(&instr); });
  const auto insert_point = last_mem != cur_section_end->rend()
                              ? &*prev(last_mem)
                              : cur_section_end->getFirstNonPHI();
  IRBuilder<>{insert_point}.CreateCall(
    Intrinsic::getDeclaration(module, Intrinsic::csa_parallel_section_exit),
    section_entry);

  // The loop intrinsic has now been expanded.
  return true;
}

// If this is a CSA local cache intrinsic, and the loop doesn't have
// any other intrinsic, then expand the local cache intrinsic here.
bool CSALoopIntrinsicExpander::expandLocalCacheIntrinsic(Loop *L, IntrinsicInst *intr) {
  using namespace std;

  LLVMContext &context = L->getHeader()->getContext();
  Module *module       = L->getHeader()->getParent()->getParent();

  assert(intr->getNumArgOperands() == 1 && "Bad local_cache intrinsic?");
  Value *lcache_size = intr->getArgOperand(0);
  Value *lcache_id   = ConstantInt::get(IntegerType::get(context, 32), 0);

  Instruction *const preheader_terminator = L->getLoopPreheader()->getTerminator();
  CallInst *const lcache_region_begin =
    IRBuilder<>{preheader_terminator}.CreateCall(
	 Intrinsic::getDeclaration(module, Intrinsic::csa_local_cache_region_begin),
         {lcache_size, lcache_id}, "lcache_region_begin");

  SmallVector<BasicBlock *, 2> exits;
  L->getExitBlocks(exits);
  for (BasicBlock *const exit : exits) {
     IRBuilder<>{exit->getFirstNonPHI()}.CreateCall(
          Intrinsic::getDeclaration(module, Intrinsic::csa_local_cache_region_end),
          {lcache_region_begin, lcache_id});
   }

   return true;
}

bool CSALoopIntrinsicExpander::makeSectionInclude(BasicBlock *BB) {

  LLVM_DEBUG(errs() << "Trying to add " << BB->getName() << "...\n");

  // If there is no existing section, start a new one with just this block.
  if (not cur_section_begin or not cur_section_end) {
    cur_section_begin = cur_section_end = BB;
    LLVM_DEBUG(errs() << "Section started at " << BB->getName() << "\n");
    return true;
  }

  // Otherwise, try to extend the current one.
  cur_section_begin = DT->findNearestCommonDominator(cur_section_begin, BB);
  if (not cur_section_begin or not cur_loop->contains(cur_section_begin))
    return false;
  cur_section_end = PDT->findNearestCommonDominator(cur_section_end, BB);
  if (not cur_section_end or not cur_loop->contains(cur_section_end))
    return false;

  LLVM_DEBUG(errs() << "Section extended from " << cur_section_begin->getName()
             << " to " << cur_section_end->getName() << "\n");

  return true;
}

bool CSALoopIntrinsicExpander::isAlreadyInSection(BasicBlock *BB) const {
  return cur_section_begin and cur_section_end and
         DT->dominates(cur_section_begin, BB) and
         PDT->dominates(cur_section_end, BB);
}

bool CSALoopIntrinsicExpander::mayNeedOrdering(const Instruction *instr) const {
  if (instr->mayReadOrWriteMemory())
    return true;
  if (const CallInst *const call = dyn_cast<CallInst>(instr))
    return call->isInlineAsm();
  return false;
}

} // namespace

namespace llvm {
void initializeCSALoopIntrinsicExpanderPass(PassRegistry &);
}

static RegisterPass<CSALoopIntrinsicExpander> rpinst{
  "csa-loop-intrinsic-expander", "Expand CSA parallel loop intrinsics"};

Pass *llvm::createCSALoopIntrinsicExpanderPass() {
  return new CSALoopIntrinsicExpander();
}


