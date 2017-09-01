//===---------- CFGRestructuring.cpp - Restructures CFG *- C++ -*----------===//
//
//   Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the CFGRestructuring member function of VPOUtils class.
// The function restructures the CFG for a routine, where each directive for
// Cilk, OpenMP, Offload, Vectorization is put into a standalone basic block.
// This is a pre-required process for constructing WRegion for a routine.
//
//===----------------------------------------------------------------------===//

#include <set>
#include "llvm/IR/Function.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/Intel_VPO/Utils/VPOAnalysisUtils.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Intel_VPO/Utils/VPOUtils.h"
#include "llvm/Transforms/Intel_VPO/VPOPasses.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/ADT/Twine.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vpo-cfgrestructuring"

// This function puts "directive_begin" and "directive_end" into standalone
// basic blocks. This is required by WRegion Construction.
//
// Each "directive_begin" or "directive_end" has a "directive_qual_list_end"
// associated to indicate the end of this directive. Here are some examples,
// where the strings are represented by metadata in the real code:
//
// Example 1:
//
// call void @llvm.intel.directive("DIR_BEGIN")
// ...
// call void @llvm.intel.directive("DIR_QUAL_LIST_END")
//
//
// Example 2:
//
// call void @llvm.intel.directive("DIR_END")
// ...
// call void @llvm.intel.directive("DIR_QUAL_LIST_END")
//
void VPOUtils::CFGRestructuring(Function &F, DominatorTree *DT, LoopInfo *LI) {

  DEBUG(dbgs() << "VPO CFG Restructuring \n");

  // Find all the intrinsic instructions, including directive_begin,
  // directive_end, directive_qual_list_end, and bookkeep them in
  // InstructionsToSplit.
  std::set<Instruction *> InstructionsToSplit;
  InstructionsToSplit.clear();
  for (Function::iterator B = F.begin(), BE = F.end(); B != BE; ++B)
    for (BasicBlock::iterator I = B->begin(), IE = B->end(); I != IE; ++I)
      if (IntrinsicInst *Inst = dyn_cast<IntrinsicInst>(&*I))
        if (Inst->getIntrinsicID() == Intrinsic::intel_directive) {
          StringRef DirString = 
                            VPOAnalysisUtils::getDirectiveMetadataString(Inst);
          if (VPOAnalysisUtils::isBeginOrEndDirective(DirString) || 
              VPOAnalysisUtils::isListEndDirective(DirString))
            InstructionsToSplit.insert(Inst);
        }

  // Now, go through InstructionsToSplit and do the splitting around
  // directive_begin or directive_end, or the next instruction right after
  // directive_qual_list_end.

  unsigned Counter = 0;

  for (Instruction *I : InstructionsToSplit) {
    Counter++;

    // Get the basic block of this instruction first.
    BasicBlock *BB = I->getParent();

    // If directive_begin or directive_end is the first instruction of a basic
    // block, we can just skip splitting around it. Note that,
    // directive_qual_list_end will never be the first instruction of a basic
    // block, since it is always paired with directive_begin or directive_end.
    if (I != &*(BB->begin())) {
      Instruction *SplitPoint = I;
      StringRef DirString =
       VPOAnalysisUtils::getDirectiveMetadataString(dyn_cast<IntrinsicInst>(I));
      if (VPOAnalysisUtils::isListEndDirective(DirString)) {
        //        BasicBlock::iterator Inst = I;
        BasicBlock::iterator Inst(I);
        SplitPoint = &*(++Inst);
      }
      BasicBlock *newBB = SplitBlock(BB, SplitPoint, DT, LI);
      newBB->setName(DirString + "." + Twine(Counter));
    }
  }
}

using namespace llvm;
using namespace llvm::vpo;

namespace {
class VPOCFGRestructuring : public FunctionPass {
public:
  static char ID; // Pass identification, replacement for typeid

  VPOCFGRestructuring() : FunctionPass(ID) {
    initializeVPOCFGRestructuringPass(*PassRegistry::getPassRegistry());
  }
  bool runOnFunction(Function &F) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
};
} // anonymous namespace

INITIALIZE_PASS_BEGIN(VPOCFGRestructuring, "vpo-cfg-restructuring",
                      "VPO CFG Restructuring", false, false)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_END(VPOCFGRestructuring, "vpo-cfg-restructuring",
                    "VPO CFGRestructuring", false, false)

char VPOCFGRestructuring::ID = 0;

bool VPOCFGRestructuring::runOnFunction(Function &F) {
  if (skipFunction(F))
    return false;

#if INTEL_PRODUCT_RELEASE
  // Set a flag to induce an error if anyone attempts to write the IR
  // to a file after this pass has been run.
  F.getParent()->setIntelProprietary();
#endif // INTEL_PRODUCT_RELEASE

  auto DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  auto LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();

  VPOUtils::CFGRestructuring(F, DT, LI);

  return true;
}

void VPOCFGRestructuring::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<DominatorTreeWrapperPass>();
  AU.addRequired<LoopInfoWrapperPass>();
}

FunctionPass *llvm::createVPOCFGRestructuringPass() {
  return new VPOCFGRestructuring();
}
