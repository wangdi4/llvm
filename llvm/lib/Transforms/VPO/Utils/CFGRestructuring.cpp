//===---------- CFGRestructuring.cpp - Restructures CFG *- C++ -*----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
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
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/VPO/Utils/VPOUtils.h"
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
// Each "directive_begin" or "directive_end" has a "directive_qual_end"
// associated to indicate the end of this directive. Here are some examples,
// where the strings are represented by metadata in the real code:
//
// Example 1:
//
// call void @llvm.intel.directive("DIR_BEGIN")
// ...
// call void @llvm.intel.directive.qual("DIR_QUAL_END")
//
//
// Example 2:
//
// call void @llvm.intel.directive("DIR_END")
// ...
// call void @llvm.intel.directive.qual("DIR_QUAL_END")
//
void VPOUtils::CFGRestructuring(
  Function &F,
  DominatorTree *DT,
  LoopInfo *LI
)
{

  DEBUG(dbgs() << "VPO CFG Restructuring \n");

  // Find all the intrinsic instructions, including directive_begin,
  // directive_end, directive_qual_end, and bookkeep them in
  // InstructionsToSplit.
  std::set<Instruction* > InstructionsToSplit;
  InstructionsToSplit.clear();
  for (Function::iterator B = F.begin(), BE = F.end(); B != BE; ++B)
    for (BasicBlock::iterator I = B->begin(), IE = B->end(); I != IE; ++I)
      if (IntrinsicInst* Inst = dyn_cast<IntrinsicInst>(&*I)) 
        // To-do:
        //
        // Use the following code to find directive_qual_end once Matt
        // modifies getDirectiveMetadataString to support intel_directive_qual
        //
        // if (Inst->getIntrinsicID() == 
        //     Intrinsic::intel_directive_qual) {
        //
        if (Inst->getIntrinsicID() == Intrinsic::intel_directive) {
          StringRef DirString = getDirectiveMetadataString(Inst);
          if ((DirString == "dir.simd") || (DirString == "dir.simd.end") ||
              (DirString == "dir.qual.end")) {
            InstructionsToSplit.insert(I);
          }
        }

  // Now, go through InstructionsToSplit and do the splitting around
  // directive_begin or directive_end, or the next instruction right after
  // directive_qual_end.

  unsigned counter = 0;

  for (Instruction *I: InstructionsToSplit) {
    counter ++;

    // Get the basic block of this instruction first.
    BasicBlock* BB = I->getParent();

    // If directive_begin or directive_end is the first instruction of a basic
    // block, we can just skip splitting around it. Note that,
    // directive_qual_end will never be the first instruction of a basic block,
    // since it is always paired with directive_begin or directive_end.
    if (I != BB->begin()) {
      Instruction* SplitPoint = I;
      StringRef DirString = getDirectiveMetadataString(dyn_cast<IntrinsicInst>(I));
      if (DirString == "dir.qual.end") {
        BasicBlock::iterator inst = I;
        SplitPoint = ++inst;
      }
      BasicBlock* newBB = SplitBlock(BB, SplitPoint, DT, LI);
      newBB->setName(DirString + "." + Twine(counter));
    }
  }
}





