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

#include "llvm/IR/Function.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/VPO/Utils/VPOUtils.h"


using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vpo-cfgrestructuring"

void VPOUtils::CFGRestructuring(
  Function &F,
  DominatorTree *DT,
  LoopInfo *LI
)
{

  DEBUG(dbgs() << "VPO CFG Restructuring \n");

  // Add comments on how CFG restructuring works
  //
/* Enable it when Matt adds directive intrinsics
 *
  for (Function::iterator b = F.begin(), be = F.end(); b != be; ++b) {
      for (BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; ++i) {
          if (CallInst* callInst = dyn_cast<CallInst>(&*i)) {
              if (it is directive_begin, or the next instruction of
                 directive begin, or directive_end, or the next instruction of
                 directive_end) {
                  SplitBlock(b, i, DT, LI);
              }
          }
      }
  }
*/
}

