//===- CSAIntrinsicCleaner.cpp - Clean unused CSA intrinsics --------===//
//
// Copyright (C) 2017-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
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
  cl::desc("CSA Specific: disables the check for iteration-local storage in "
           "parallelized loops")};

#define DEBUG_TYPE "csa-intrinsic-cleaner"

STATISTIC(NumSPMDizationsCleaned,
          "Number of unused SPMDization intrinsic pairs removed");

STATISTIC(NumSPMDWorkerNumCleaned,
          "Number of unused spmd_worker_num intrinsic removed");

STATISTIC(NumPipelineCleaned,
          "Number of unused pipeline_loop intrinsic pairs removed");

STATISTIC(NumPipelineDepthCleaned,
  "Number of unused pipeline_limited_loop intrinsic pairs removed");


namespace {

struct CSAIntrinsicCleaner : FunctionPass {
  static char ID;

  CSAIntrinsicCleaner() : FunctionPass{ID} {}

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    AU.addRequired<LoopInfoWrapperPass>();
  }

  bool runOnFunction(Function &) override;

  StringRef getPassName() const override {
    return "CSA: Clean unused CSA intrinsics";
  }

private:
  // Recursively checks loops for problematic iteration-local storage. If any is
  // found, this will print a noticeable warning and will return true.
  // Otherwise, it returns false.
  bool check_for_problematic_iter_storage(const Loop *);

  // Removes any unused spmdization intrinsic pairs from a function.
  bool clean_spmdization(Function &);
  // Removes any unused spmd_worker_num intrinsic from a function.
  bool clean_spmd_worker_num(Function &);
  // Removes any unused pipeline_loop intrinsic pairs from a function
  bool clean_pipeline(Function &);
  // Removes any unused pipeline_limited_loop intrinsic pairs from a function
  bool clean_pipeline_depth(Function &);
    // Create a LIC ID rather than the class used in the builtins
  bool expandLicQueueIntrinsics(Function &F);

};

char CSAIntrinsicCleaner::ID = 0;

bool CSAIntrinsicCleaner::runOnFunction(Function &F) {
  if (not DisableLoopStorageCheck) {
    for (const Loop *L : getAnalysis<LoopInfoWrapperPass>().getLoopInfo()) {
      if (check_for_problematic_iter_storage(L))
        break;
    }
  }
  return expandLicQueueIntrinsics(F) | clean_spmdization(F) |
    clean_spmd_worker_num(F) | clean_pipeline(F) | clean_pipeline_depth(F);
}

// convert init/write/read intrinsics to lower_init/lower_write/lower_read intrinsic
bool CSAIntrinsicCleaner::expandLicQueueIntrinsics(Function &F) {

  LLVMContext &CTX = F.getContext();
  Module *M = F.getParent();
  unsigned licNum = 0;
  SmallVector<Instruction *, 4> toDelete;
  const auto errorMessage = [](StringRef msg1, StringRef msg2,
                               Instruction *inst, Instruction *use,
                               StringRef msg3, Instruction *first) {
                        errs() << "\n";
                        errs().changeColor(raw_ostream::RED, true);
                        errs() << msg1;
                        errs().resetColor();
                        const DebugLoc &loc1 = inst->getDebugLoc();
                        if (loc1) {
                          errs() << msg2;
                          loc1.print(errs());
                        }
                        if(use) {
                          const DebugLoc &loc2 = use->getDebugLoc();
                          if (loc2) {
                            errs() << "\nwas detected at \n";
                            loc2.print(errs());
                          }
                        }
                        if(first) {
                          const DebugLoc &loc3 = first->getDebugLoc();
                          if (loc3) {
                            errs() << msg3;
                            loc3.print(errs());
                          }
                        }
                        errs() << "\n";
                        report_fatal_error(""); //to exit the compilation
                 };
  for (auto &BB : F) {
    for (auto &I : BB) {
      auto intrinsic = dyn_cast<IntrinsicInst>(&I);
      if (!intrinsic)
        continue;

      if (intrinsic->getIntrinsicID() != Intrinsic::csa_lic_init)
        continue;
      IntrinsicInst *write = nullptr, *read = nullptr;
      for (auto user : intrinsic->users()) {
        if (auto useIntrinsic = dyn_cast<IntrinsicInst>(user)) {
          if (useIntrinsic->getIntrinsicID() == Intrinsic::csa_lic_write) {
            if (write) {
              errorMessage("!! ERROR: Can only have one write for a LIC queue !!\n",
                           "\n The extra write for stream:\n",
                           intrinsic, useIntrinsic,
                           "\n The first write is at: \n", write);
            }
            write = useIntrinsic;
            continue;
          }
          if (useIntrinsic->getIntrinsicID() == Intrinsic::csa_lic_read) {
            if (read) {
              errorMessage("!! ERROR: Can only have one read for a LIC queue !!\n",
                           "\n The extra read for stream:\n",
                           intrinsic, useIntrinsic,
                           "\n The first read is at: \n", read);
            }
            read = useIntrinsic;
            continue;
          }
        }
        errs() << "\n";
        errs().changeColor(raw_ostream::RED, true);
        if(dyn_cast<CallInst>(user)) {
          errs() << "!! ERROR: LIC streams are used by a call.";
          errs() << " Add __attribute__((always_inline)) to the function definition\n";
        }
        else if(dyn_cast<SelectInst>(user)) {
          errs() << "!! ERROR: LIC streams are used by a select instruction\n";
          errs() << " Add the option -mllvm -simplifycfg-sink-common=0 \n";
        }
        else if(dyn_cast<StoreInst>(user)) {
          errs() << "!! ERROR: LIC streams are used by a store instruction\n";
          errs() << " Add the option -mllvm -hir-complete-unroll-loop-trip-threshold=256 \n";
          errs() << " If the option is already there, make sure unrolling actually happened \n";
          errs() << " using the option -qopt-report=3 \n";
        }
        errorMessage("LIC streams can only have writes/reads !!\n",
                     "\n The illegal use for stream:\n",
                     intrinsic, dyn_cast<Instruction>(user), "", nullptr);
      }
      if(!write || !read) {
        errorMessage("!! ERROR: LIC streams must have one write and one read !!\n",
                     "", intrinsic, nullptr, "", nullptr);
      }
      auto licID = ConstantInt::get(IntegerType::getInt32Ty(CTX), licNum++);
      CallInst::Create(
          Intrinsic::getDeclaration(M, Intrinsic::csa_lower_lic_init),
          { licID, I.getOperand(0), I.getOperand(1), I.getOperand(2) }, "", &I);
      CallInst::Create(
          Intrinsic::getDeclaration(M, Intrinsic::csa_lower_lic_write,
                                    write->getFunctionType()->getParamType(1)),
          { licID, write->getOperand(1) }, "", write);
      auto newRead = CallInst::Create(
          Intrinsic::getDeclaration(M, Intrinsic::csa_lower_lic_read,
            read->getFunctionType()->getReturnType()),
          { licID }, "", read);
      read->replaceAllUsesWith(newRead);
      read->eraseFromParent();
      write->eraseFromParent();
      toDelete.push_back(&I);
    }
  }

  for (auto I : toDelete)
    I->eraseFromParent();
  return licNum != 0;
}

// Determines whether there are lifetime start intrinsics anywhere in L. If
// there is it will be returned; otherwise, nullptr will be.
const IntrinsicInst *find_lifetime_start(const Loop *L) {
  for (const BasicBlock *const BB : L->blocks()) {
    for (const Instruction &instr : *BB) {
      const IntrinsicInst *const intr_inst = dyn_cast<IntrinsicInst>(&instr);
      if (intr_inst and
          intr_inst->getIntrinsicID() == Intrinsic::lifetime_start)
        return intr_inst;
    }
  }
  return nullptr;
}

// Checks whether BB is part of a subloop of L.
bool belongs_to_subloop(const Loop *L, const BasicBlock *BB) {
  for (const Loop *const subloop : L->getSubLoops())
    if (subloop->contains(BB))
      return true;
  return false;
}

// Determines whether there are parallel sections in L (but not its subloops).
bool has_parallel_section(const Loop *L) {
  for (const BasicBlock *const BB : L->blocks()) {
    for (const Instruction &instr : *BB) {
      if (belongs_to_subloop(L, BB))
        continue;
      const IntrinsicInst *const intr_inst = dyn_cast<IntrinsicInst>(&instr);
      if (intr_inst and
          intr_inst->getIntrinsicID() == Intrinsic::csa_parallel_section_entry)
        return true;
    }
  }
  return false;
}

bool CSAIntrinsicCleaner::check_for_problematic_iter_storage(const Loop *L) {
  const IntrinsicInst *const lifetime_start = find_lifetime_start(L);
  if (lifetime_start and has_parallel_section(L)) {
    errs() << "\n";
    errs().changeColor(raw_ostream::BLUE, true);
    errs() << "!! WARNING: ITERATION-LOCAL STORAGE DETECTED IN A PARALLELIZED "
              "LOOP !!";
    errs().resetColor();
    const DebugLoc &loc = lifetime_start->getDebugLoc();
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

  for (const Loop *const subloop : L->getSubLoops()) {
    if (check_for_problematic_iter_storage(subloop))
      return true;
  }

  return false;
}

// Collects a set of all users of an instruction recursively. This set will also
// contain the original instruction.
void collect_users_recursively(Instruction *instr,
                               std::set<Instruction *> &insts) {
  using namespace std;
  const auto found = insts.lower_bound(instr);
  if (found != end(insts) and *found == instr)
    return;
  insts.insert(found, instr);
  for (User *const user : instr->users()) {
    if (Instruction *const user_inst = dyn_cast<Instruction>(user)) {
      collect_users_recursively(user_inst, insts);
    }
  }
}

// Erases an instruction along with all (recursive) users of it. The iterator
// pointing to the next location in the instruction's basic block is returned.
BasicBlock::iterator erase_with_all_uses(Instruction *instr) {
  std::set<Instruction *> users;
  collect_users_recursively(instr, users);
  users.erase(instr);
  for (Instruction *const to_erase : users)
    to_erase->eraseFromParent();
  return instr->eraseFromParent();
}

bool CSAIntrinsicCleaner::clean_spmdization(Function &F) {
  using namespace std;
  bool cleaned_spmdizations = false;
  for (BasicBlock &BB : F) {
    for (auto inst_it = begin(BB); inst_it != end(BB);) {
      IntrinsicInst *const intr_inst = dyn_cast<IntrinsicInst>(&*inst_it);
      if (intr_inst and
          intr_inst->getIntrinsicID() == Intrinsic::csa_spmdization_entry) {
        cleaned_spmdizations = true;
        ++NumSPMDizationsCleaned;
        inst_it = erase_with_all_uses(intr_inst);
      } else
        ++inst_it;
    }
  }
  return cleaned_spmdizations;
}

bool CSAIntrinsicCleaner::clean_spmd_worker_num(Function &F) {
  using namespace std;
  bool cleaned_spmd_worker = false;
  for (BasicBlock &BB : F) {
    for (auto inst_it = begin(BB); inst_it != end(BB);) {
      IntrinsicInst *const intr_inst = dyn_cast<IntrinsicInst>(&*inst_it);
      if (intr_inst and
          intr_inst->getIntrinsicID() == Intrinsic::csa_spmd_worker_num) {
        cleaned_spmd_worker = true;
        ++NumSPMDWorkerNumCleaned;
        inst_it = erase_with_all_uses(intr_inst);
      } else
        ++inst_it;
    }
  }
  return cleaned_spmd_worker;
}

bool CSAIntrinsicCleaner::clean_pipeline(Function &F) {
  using namespace std;
  bool cleaned_pipeline_loop = false;
  for (BasicBlock &BB : F) {
    for (auto inst_it = begin(BB); inst_it != end(BB);) {
      IntrinsicInst *const intr_inst = dyn_cast<IntrinsicInst>(&*inst_it);
      if (intr_inst and
          intr_inst->getIntrinsicID() == Intrinsic::csa_pipeline_loop_entry) {
        cleaned_pipeline_loop = true;
        ++NumPipelineCleaned;
        inst_it = erase_with_all_uses(intr_inst);
      } else
        ++inst_it;
    }
  }
  return cleaned_pipeline_loop;
}

bool CSAIntrinsicCleaner::clean_pipeline_depth(Function &F) {
  using namespace std;
  bool cleaned_pipeline_depth = false;
  for (BasicBlock &BB : F) {
    for (auto inst_it = begin(BB); inst_it != end(BB);) {
      IntrinsicInst *const intr_inst = dyn_cast<IntrinsicInst>(&*inst_it);
      if (intr_inst and
        intr_inst->getIntrinsicID() ==
        Intrinsic::csa_pipeline_limited_entry) {
        cleaned_pipeline_depth = true;
        ++NumPipelineDepthCleaned;
        inst_it = erase_with_all_uses(intr_inst);
      }
      else
        ++inst_it;
    }
  }
  return cleaned_pipeline_depth;
}

} // namespace

namespace llvm {
void initializeCSAIntrinsicCleanerPass(PassRegistry &);
}

static RegisterPass<CSAIntrinsicCleaner> rpinst{
  "csa-intrinsic-cleaner",
  "Remove unsed CSA intrinsics and find iteration-local storage"};

Pass *llvm::createCSAIntrinsicCleanerPass() {
  return new CSAIntrinsicCleaner();
}
