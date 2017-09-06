//===- CSAOMPAllocaTypeFixer.cpp - Fix OMP allocas ------------------===//
//
//===----------------------------------------------------------------===//
//
// This pass adds allocas with the correct types in order to overcome an
// issue with OpenMP offload where it will pass arguments with the wrong
// type and bitcast the value to the right one, which confuses mem2reg.
//
//===----------------------------------------------------------------===//

#include "CSAOMPAllocaTypeFixer.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Pass.h"

#include <algorithm>
#include <cassert>
#include <iterator>

#define DEBUG_TYPE "csa-omp-alloca-type-fixer"

STATISTIC(NumAllocasFixed, "Number of OpenMP bitcasted allocas fixed");

using namespace llvm;

namespace {

struct CSAOMPAllocaTypeFixer : FunctionPass {
  static char ID;

  CSAOMPAllocaTypeFixer() : FunctionPass{ID} {}

  void getAnalysisUsage(AnalysisUsage& AU) const override {
    AU.setPreservesCFG();
  }

  bool runOnFunction(Function&) override;

  StringRef getPassName() const override {
    return "Fix OpenMP bitcasted allocas";
  }
};

char CSAOMPAllocaTypeFixer::ID = 0;

bool CSAOMPAllocaTypeFixer::runOnFunction(Function& F) {
  using namespace std;

  bool fixed_allocas = false;

  // All new instructions should go at the end of the entry block
  IRBuilder<> builder {F.getEntryBlock().getTerminator()};

  for (Instruction& instr : F.getEntryBlock()) {

    // Only mess with bitcast instructions of alloca values which have no uses
    // outside of the entry block.
    BitCastInst*const bitcast = dyn_cast<BitCastInst>(&instr);
    if (not bitcast) continue;
    AllocaInst*const orig_alloca = dyn_cast<AllocaInst>(bitcast->getOperand(0));
    if (not orig_alloca) continue;
    const auto outside_block_use = find_if(
      begin(orig_alloca->users()), end(orig_alloca->users()),
      [&F](const User* user) {
        const Instruction*const user_inst = dyn_cast<Instruction>(user);
        return user_inst and user_inst->getParent() != &F.getEntryBlock();
      }
    );
    if (outside_block_use != end(orig_alloca->users())) continue;

    // Put in a new alloca to hold the casted value.
    const PointerType*const cast_ptr_type
      = dyn_cast<PointerType>(bitcast->getDestTy());
    AllocaInst*const new_alloca = builder.CreateAlloca(
      cast_ptr_type->getElementType(), nullptr, "omp_fixup"
    );

    // Update all of the post-entry uses of the bitcast to point to the new alloca instead.
    bitcast->replaceUsesOutsideBlock(new_alloca, &F.getEntryBlock());

    // Copy the value from the original alloca in there.
    LoadInst*const orig_val = builder.CreateLoad(
      bitcast, "omp_fixup_val"
    );
    builder.CreateStore(orig_val, new_alloca);

    // This alloca has now been fixed.
    ++NumAllocasFixed;
    fixed_allocas = true;
  }

  return fixed_allocas;
}

}

namespace llvm {
void initializeCSAOMPAllocaTypeFixerPass(PassRegistry&);
}

static RegisterPass<CSAOMPAllocaTypeFixer> rpinst {
  "csa-omp-alloca-type-fixer",
  "Fix OpenMP bitcasted allocas",
  true, // (Does not modify the CFG)
  false // (Not an analysis pass)
};

Pass* llvm::createCSAOMPAllocaTypeFixerPass() {
  return new CSAOMPAllocaTypeFixer();
}
