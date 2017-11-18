//===- CSAInnerLoopPrep.cpp - ------------------------------*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
// This is a pass which attempts to re-express an inner loop and its
// corresponding parallel outer loop such that inner loop outputs may be
// emitted in any order if multiple outer loop invocations are allowed
// concurrent/pipelined use.  This is in lieu of a reorder buffer on the
// outputs of the inner loop, since that doesn't exist just yet. Also note that
// at the IR level what this pass does is correct but silly: it's doing the
// opposite of LICM (ableit selectively) in order to allow more parallelism in
// the final dataflow graph.
//
// Example from the inner loop pipelining design doc:
//
//    __builtin_csa_parallel_loop();
//    for (int i = 0; i < N; ++i) {
//      float vi = A[i];
//      for (int j = 0; j < M; ++j) { // <- inner loop to be pipelined
//        [code using v and j and producing ri;]
//    }
//    A[i] = ri;
//
// Here, if the inner loop is pipelined, ri may be emitted in different orders
// with respect to i. However, the address of A[i], since it was not an output
// of the inner loop, is expecting ri values in the original order.
//
// This pass will notice that the store to memory uses output of the inner loop
// (ri), but that A[i] may correspond to a different "i" if the inner loop's
// outputs are reordered. It will arrange to fix this by making A[i] trivially
// an output of the inner loop:
//
//    __builtin_csa_parallel_loop();
//    for (int i = 0; i < N; ++i) {
//      float vi = A[i];
//      for (int j = 0; j < M; ++j) { // <- inner loop to be pipelined
//        [code using v and j and producing ri;]
//        [** code saving the address A[i] to ai; **]
//    }
//    *ai = ri;
//
//  Now, after pipelining, ri/ai may come out in any order with respect to i,
//  but that's OK: the "i" for each output generation matches so the result
//  will be correct.
//
//===----------------------------------------------------------------------===//
//
// \file
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/APInt.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/CodeGen/StackProtector.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
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
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/CodeExtractor.h"
#include "llvm/Transforms/Utils/SSAUpdater.h"
using namespace llvm;
using namespace llvm::PatternMatch;

#define DEBUG_TYPE "csa-ilpl-prep"
#define PASS_DESC                                                              \
  "Identify and prepare inner loops for pipelining of multiple executions"

static cl::opt<bool> EnableILPLPrep(
    "csa-ilpl-prep", cl::Hidden,
    cl::desc(
        "CSA Specific: enable/disable inner loop pipelining preparation pass"),
    cl::init(false));

static cl::alias EnableILPL(
    "csa-ilpl",
    cl::desc("Enable inner loop pipelining. (Alias for -csa-ilpl-prep.)"),
    cl::aliasopt(EnableILPLPrep));

static cl::opt<int> DefaultDegreeOfPipeliningParallelism(
    "csa-ilpl-tokens", cl::Hidden,
    cl::desc(
        "CSA Specific: degree of concurrency allowed by inner loop pipelining"),
    cl::init(32));

namespace llvm {
void initializeCSAInnerLoopPrepPass(PassRegistry &);
Pass *createCSAInnerLoopPrepPass();
} // namespace llvm

namespace {
struct CSAInnerLoopPrep : public LoopPass {
  static char ID;

  typedef SetVector<Value *> ValueSet;
  explicit CSAInnerLoopPrep() : LoopPass(ID) {
    initializeCSAInnerLoopPrepPass(*PassRegistry::getPassRegistry());
  }
  StringRef getPassName() const override { return PASS_DESC; }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<PostDominatorTreeWrapperPass>();
    // AU.addPreserved<DominatorTreeWrapperPass>();
    AU.addRequired<LoopInfoWrapperPass>();
    // AU.addPreserved<LoopInfoWrapperPass>();
  }

  bool runOnLoop(Loop *L, LPPassManager &) override;

  DominatorTree *DT;
  PostDominatorTree *PDT;
  LoopInfo *LI;
  Loop *getParallelOuterLoop(Loop *L);
  bool containsPipelinedLoop(Loop *L);

  // Eventually, particularly when the simulator switches to default to shallow
  // LICs, this needs to be implemented.
  unsigned getMaxDegreeOfParallelism(Loop *L, Loop *parallelLoop) {
    return DefaultDegreeOfPipeliningParallelism;
  };
};
} // namespace

char CSAInnerLoopPrep::ID = 0;
INITIALIZE_PASS_BEGIN(CSAInnerLoopPrep, "csa-inner-loop-prep", PASS_DESC, false,
                      false)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_END(CSAInnerLoopPrep, "csa-inner-loop-prep", PASS_DESC, false,
                    false)

Pass *llvm::createCSAInnerLoopPrepPass() { return new CSAInnerLoopPrep(); }

bool CSAInnerLoopPrep::runOnLoop(Loop *L, LPPassManager &) {
  bool Changed = false;

  if (not EnableILPLPrep)
    return Changed;

  if (skipLoop(L))
    return Changed;

  DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  PDT = &getAnalysis<PostDominatorTreeWrapperPass>().getPostDomTree();
  LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();

  if (containsPipelinedLoop(L)) {
    DEBUG(errs() << "Won't try to pipeline loop " << L->getHeader()->getName()
                 << "; it contains a pipelined loop.\n");
    return Changed;
  }

  DEBUG(errs() << "Looking for parallel outer loop containing "
               << L->getHeader()->getName() << "(depth " << L->getLoopDepth()
               << ")\n");
  Loop *outerLoop = getParallelOuterLoop(L);
  if (not outerLoop)
    return Changed;

  // TODO: look at pipelining with respect to parallel outer loops which are
  // more than one nesting level out. For now, require the outer loop to the
  // immediate parent.
  if (outerLoop != L->getParentLoop()) {
    DEBUG(errs() << "Found a outer parallel loop, but it's not the immediate "
                    "parent. This is not yet supported.\n");
    return Changed;
  }

  DEBUG(errs() << "Remark: preparing to pipeline " << L->getHeader()->getName()
               << "(depth " << L->getLoopDepth() << ") with respect to "
               << outerLoop->getHeader()->getName() << " (depth "
               << outerLoop->getLoopDepth() << ")\n");

  DEBUG(errs() << "Looking at loop " << L->getHeader()->getName() << ".\n");
  std::vector<BasicBlock *> region(L->getBlocks());
  DEBUG(errs() << "Extractor region contains " << region.size() << " BBs.\n");
  CodeExtractor Extractor(ArrayRef<BasicBlock *>(region), DT);

  // Use CodeExtractor to do inputs/outputs/allocas analysis for us?
  ValueSet inputs, outputs, allocas;
  Extractor.findInputsOutputs(inputs, outputs, allocas);
  DEBUG(errs() << "\textractor found: " << inputs.size() << " inputs; "
               << outputs.size() << " outputs; " << allocas.size()
               << " allocas\n");

  DEBUG(errs() << "\t\tinputs: ");
  for (Value *v : inputs) {
    DEBUG(errs() << "\t\t\t" << *v << "\n");
  }
  DEBUG(errs() << "\n");

  DEBUG(errs() << "\t\toutputs: ");
  for (Value *v : outputs) {
    DEBUG(errs() << "\t\t\t" << *v << "\n");
  }
  DEBUG(errs() << "\n");

  DEBUG(errs() << "\t\tallocas: ");
  for (Value *v : allocas) {
    DEBUG(errs() << "\t\t\t" << *v << "\n");
  }
  DEBUG(errs() << "\n");

  ValueSet effectiveOutputs(outputs);
  ValueSet newOutputs;
  std::set<Instruction *> needsRepeating;
  std::map<Instruction *, std::set<Use *>> repeatForUses;

  // Essentially, we're trying to explore the dataflow subgraph which consumes
  // the inner loop outputs, searching for any uses in the subgraph of data
  // produced by the outer loop. These outer loop values are then repeated
  // through the inner loop, trivially making them outputs of the inner loop as
  // well and ensuring that they correspond to the same outer loop iteration.
  do {

    // Include any outputs which were discovered in a previous iteration.
    effectiveOutputs.insert(newOutputs.begin(), newOutputs.end());
    newOutputs.clear();

    // Search beginning from our known effective outputs.
    for (Value *v : effectiveOutputs) {

      std::set<User *> visitedUsers;
      SmallVector<User *, 4> userQueue(v->users());
      // Consider all users of this inner loop output.
      while (!userQueue.empty()) {
        User *user = userQueue.back();
        userQueue.pop_back();
        visitedUsers.insert(user);
        Instruction *i = dyn_cast<Instruction>(user);
        assert(i);

        // If we've already considered this user's operands, there's no need to
        // do it again.
        if (effectiveOutputs.count(i))
          continue;

        // If the user dominates the value being used, then this is a loop
        // backedge we don't want to follow.
        if (Instruction *def = dyn_cast<Instruction>(v))
          if (DT->dominates(i, def))
            continue;

        // We're only interested in users which are in the parent loop.
        if (outerLoop->contains(i) and not L->contains(i)) {

          DEBUG(errs() << "Considering operands of " << *i << "\n");
          for (Use &use : i->operands()) {
            Value *usev = use.get();
            // No need to repeat if this use is another effective output or
            // completely invariant to the outer loop.
            if (effectiveOutputs.count(usev))
              continue;
            if (outerLoop->isLoopInvariant(usev))
              continue;
            Instruction *repeatInst = dyn_cast<Instruction>(usev);
            assert(repeatInst);

            if (!DT->dominates(repeatInst->getParent(), L->getHeader())) {
              // If this value's def doesn't dominate the inner loop, then we
              // can't repeat it there. However, we still want to explore its
              // operands. Do this if we haven't already.
              if (!visitedUsers.count(repeatInst)) {
                DEBUG(errs()
                      << "Would repeat " << *usev
                      << ", but not dominated. Will visit its operands.\n");
                userQueue.push_back(repeatInst);
              }
            } else {
              // We've found a value which is used alongside an inner loop
              // output and appears to have been generated as part of the outer
              // loop. This is effectively some context which needs to be kept
              // with the outputs of the inner loop.
              DEBUG(errs() << "Decided to repeat " << *usev << ".\n");
              needsRepeating.insert(repeatInst);
              repeatForUses[repeatInst].insert(&use);
            }
          }

          newOutputs.insert(i);
          DEBUG(errs() << "Need to include " << *i
                       << " in effective outputs.\n");
        }
      }
    }
  } while (newOutputs.size());

  /*
  if (needsRepeating.size() > 0) {
    DEBUG(errs() << "Before repeats:\n");
    DEBUG(L->getHeader()->getParent()->viewCFG());
  }
  */

  for (Instruction *outerToRepeat : needsRepeating) {
    PHINode *repeatIV =
        PHINode::Create(outerToRepeat->getType(), 2, "outerRepeat",
                        L->getHeader()->getFirstNonPHI());
    Changed = true;
    for (auto it = pred_begin(L->getHeader()), et = pred_end(L->getHeader());
         it != et; ++it) {
      BasicBlock *pred = *it;
      repeatIV->addIncoming(outerToRepeat, pred);
    }

    SSAUpdater updater;
    updater.Initialize(outerToRepeat->getType(), "repeatedSSA");
    updater.AddAvailableValue(outerToRepeat->getParent(), outerToRepeat);
    updater.AddAvailableValue(repeatIV->getParent(), repeatIV);

    for (Use *u : repeatForUses[outerToRepeat])
      updater.RewriteUseAfterInsertions(*u);
  }

  // Note that this loop was selected and prepared.
  LLVMContext &Context = L->getHeader()->getContext();
  Module *m = L->getHeader()->getParent()->getParent();
  IRBuilder<>{L->getHeader()->getTerminator()}.CreateCall(
      Intrinsic::getDeclaration(m, Intrinsic::csa_pipelineable_loop),
      ConstantInt::get(IntegerType::get(Context, 64),
                       getMaxDegreeOfParallelism(L, outerLoop)));

  /*
  DEBUG(L->getHeader()->getParent()->viewCFG());
  */

  return Changed;
}

Loop *CSAInnerLoopPrep::getParallelOuterLoop(Loop *L) {
  if (nullptr == L)
    return nullptr;

  unsigned loopDepth = L->getLoopDepth();
  SmallVector<Loop *, 1> parallelOuters;

  // Start with the pipelining candidate loop itself. We check if L is parallel
  // so that we can avoid pipelining it if that's the case.
  Loop *outerLoop = L;
  // Start looking for the relevant intrinsics.
  while (outerLoop) {
    unsigned outerLoopDepth = outerLoop->getLoopDepth();
    for (BasicBlock *bb : outerLoop->blocks()) {
      for (Instruction &i : *bb) {
        Instruction *section_exit = &i;
        Instruction *section_entry = nullptr;
        Instruction *region_entry = nullptr;
        Value *regionId = nullptr;
        if (match(section_exit,
                  m_Intrinsic<Intrinsic::csa_parallel_section_exit>(
                      m_Instruction(section_entry))) and
            match(section_entry,
                  m_Intrinsic<Intrinsic::csa_parallel_section_entry>(
                      m_Instruction(region_entry))) and
            match(region_entry,
                  m_Intrinsic<Intrinsic::csa_parallel_region_entry>(
                      m_Value(regionId))) and
            LI->getLoopFor(section_entry->getParent()) == outerLoop and
            LI->getLoopFor(section_exit->getParent()) == outerLoop) {
          if (outerLoop == L) {
            // If we find that this loop is itself parallel, bail out.
            return nullptr;
          } else if (DT->dominates(section_entry->getParent(),
                                   L->getHeader()) and
                     PDT->dominates(section_exit->getParent(),
                                    L->getHeader())) {
            DEBUG(errs() << "Found (region id is " << *regionId << "):\n"
                         << "\t" << *region_entry << "\n"
                         << "\t\t" << *section_entry << " (depth "
                         << outerLoopDepth << ")\n"
                         << "\t\t\t"
                         << "(me at depth " << loopDepth << "?)\n"
                         << "\t\t" << *section_exit << "\n");
            parallelOuters.push_back(outerLoop);
          }
        }
      }
    }
    outerLoop = outerLoop->getParentLoop();
  }

  if (parallelOuters.size() > 0) {
    if (parallelOuters.size() > 1)
      DEBUG(errs() << "WARNING: found more than one (" << parallelOuters.size()
                   << ") parallel outer loop for inner loop "
                   << L->getHeader()->getName() << ". Choosing the nearest.\n");
    return *parallelOuters.begin();
  }
  return nullptr;
}

bool CSAInnerLoopPrep::containsPipelinedLoop(Loop *L) {
  if (nullptr == L)
    return false;

  for (BasicBlock *bb : L->blocks()) {
    for (Instruction &i : *bb) {
      Instruction *pipelineable_marker = &i;
      Value *numTokens = nullptr;
      if (match(pipelineable_marker,
                m_Intrinsic<Intrinsic::csa_pipelineable_loop>(
                    m_Value(numTokens)))) {
        return true;
      }
    }
  }
  return false;
}
