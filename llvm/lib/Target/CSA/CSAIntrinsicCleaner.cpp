
//===- CSAIntrinsicCleaner.cpp - Clean unused CSA intrinsics --------===//
//
//===----------------------------------------------------------------===//
//
// This pass cleans up unused CSA intrinsics that shouldn't show up in
// the backend and detects any remaining iteration-local storage so that
// it can warn the user.
//
//===----------------------------------------------------------------===//

#include "CSAIntrinsicCleaner.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <set>

using namespace llvm;

static cl::opt<bool> DisableLoopStorageCheck{
  "csa-disable-loop-storage-check", cl::Hidden,
  cl::desc("CSA Specific: disables the check for iteration-local storage in parallelized loops")
};

#define DEBUG_TYPE "csa-intrinsic-cleaner"

STATISTIC(
  NumSPMDizationsCleaned, "Number of unused SPMDization intrinsic pairs removed"
);

namespace {

struct CSAIntrinsicCleaner : FunctionPass {
  static char ID;

  CSAIntrinsicCleaner() : FunctionPass{ID} {}

  void getAnalysisUsage(AnalysisUsage& AU) const override {
    AU.setPreservesCFG();
    AU.addRequired<LoopInfoWrapperPass>();
  }

  bool runOnFunction(Function&) override;

  StringRef getPassName() const override {
    return "Clean unused CSA intrinsics";
  }

private:

  // Recursively checks loops for problematic iteration-local storage. If any is
  // found, this will print a noticeable warning and will return true. Otherwise,
  // it returns false.
  bool check_for_problematic_iter_storage(const Loop*);

  // Removes any unused spmdization intrinsic pairs from a function.
  bool clean_spmdization(Function&);
};

char CSAIntrinsicCleaner::ID = 0;

bool CSAIntrinsicCleaner::runOnFunction(Function& F) {
  if (not DisableLoopStorageCheck) {
    for (const Loop* L : getAnalysis<LoopInfoWrapperPass>().getLoopInfo()) {
      if (check_for_problematic_iter_storage(L)) break;
    }
  }
  return clean_spmdization(F);
}

// Determines whether there are lifetime start intrinsics anywhere in L. If
// there is it will be returned; otherwise, nullptr will be.
const IntrinsicInst* find_lifetime_start(const Loop* L) {
  for (const BasicBlock*const BB : L->blocks()) {
    for (const Instruction& instr : *BB) {
      const IntrinsicInst*const intr_inst = dyn_cast<IntrinsicInst>(&instr);
      if (
        intr_inst and intr_inst->getIntrinsicID() == Intrinsic::lifetime_start
      ) return intr_inst;
    }
  }
  return nullptr;
}

// Checks whether BB is part of a subloop of L.
bool belongs_to_subloop(const Loop* L, const BasicBlock* BB) {
  for (const Loop*const subloop : L->getSubLoops())
    if (subloop->contains(BB))
      return true;
  return false;
}

// Determines whether there are parallel sections in L (but not its subloops).
bool has_parallel_section(const Loop* L) {
  for (const BasicBlock*const BB : L->blocks()) {
    for (const Instruction& instr : *BB) {
      if (belongs_to_subloop(L, BB)) continue;
      const IntrinsicInst*const intr_inst = dyn_cast<IntrinsicInst>(&instr);
      if (
        intr_inst
        and intr_inst->getIntrinsicID() == Intrinsic::csa_parallel_section_entry
      ) return true;
    }
  }
  return false;
}

bool CSAIntrinsicCleaner::check_for_problematic_iter_storage(const Loop* L) {
  const IntrinsicInst*const lifetime_start = find_lifetime_start(L);
  if (lifetime_start and has_parallel_section(L)) {
    errs() << "\n";
    errs().changeColor(raw_ostream::BLUE, true);
    errs() << "!! WARNING: ITERATION-LOCAL STORAGE DETECTED IN A PARALLELIZED LOOP !!";
    errs().resetColor();
    const DebugLoc& loc = lifetime_start->getDebugLoc();
    if (loc) {
      errs()
        << "\nIteration-local storage was detected in a parallelized loop at ";
      loc.print(errs());
    } else {
      errs() << R"help(
Iteration-local storage was detected in a parallelized loop. Run with -g for
location information)help";
    }
    errs() << R"help(

This is problematic because the storage isn't replicated across parallel loop
iterations, so if you run the output you are likely to see wrong results. Please
either allocate the storage outside of the loop or remove the parallel markings
from the loop.

In the unlikely event that this is a false positive, you can disable this check
by adding -mllvm -csa-disable-loop-storage-check to your csa-clang arguments.

)help";
    return true;
  }

  for (const Loop*const subloop : L->getSubLoops()) {
    if (check_for_problematic_iter_storage(subloop)) return true;
  }

  return false;
}

// Collects a set of all users of an instruction recursively. This set will also
// contain the original instruction.
void collect_users_recursively(
  Instruction* instr, std::set<Instruction*>& insts
) {
  using namespace std;
  const auto found = insts.lower_bound(instr);
  if (found != end(insts) and *found == instr) return;
  insts.insert(found, instr);
  for (User*const user : instr->users()) {
    if (Instruction*const user_inst = dyn_cast<Instruction>(user)) {
      collect_users_recursively(user_inst, insts);
    }
  }
}

// Erases an instruction along with all (recursive) users of it. The iterator
// pointing to the next location in the instruction's basic block is returned.
BasicBlock::iterator erase_with_all_uses(Instruction* instr) {
  std::set<Instruction*> users;
  collect_users_recursively(instr, users);
  users.erase(instr);
  for (Instruction*const to_erase : users) to_erase->eraseFromParent();
  return instr->eraseFromParent();
}

bool CSAIntrinsicCleaner::clean_spmdization(Function& F) {
  using namespace std;
  bool cleaned_spmdizations = false;
  for (BasicBlock& BB : F) {
    for (auto inst_it = begin(BB); inst_it != end(BB);) {
      IntrinsicInst*const intr_inst = dyn_cast<IntrinsicInst>(&*inst_it);
      if (
        intr_inst
        and intr_inst->getIntrinsicID() == Intrinsic::csa_spmdization_entry
      ) {
        cleaned_spmdizations = true;
        ++NumSPMDizationsCleaned;
        inst_it = erase_with_all_uses(intr_inst);
      } else ++inst_it;
    }
  }
  return cleaned_spmdizations;
}

}

namespace llvm {
void initializeCSAIntrinsicCleanerPass(PassRegistry&);
}

static RegisterPass<CSAIntrinsicCleaner> rpinst {
  "csa-intrinsic-cleaner",
  "Remove unsed CSA intrinsics and find iteration-local storage"
};

Pass* llvm::createCSAIntrinsicCleanerPass() {
  return new CSAIntrinsicCleaner();
}
