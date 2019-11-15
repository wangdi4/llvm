// INTEL CONFIDENTIAL
//
// Copyright 2012-2019 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "PostDominanceFrontier.h"
#include "llvm/Analysis/DominanceFrontierImpl.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/CFG.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/Debug.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SetOperations.h"
#include "OCLPassSupport.h"
#include "InitializePasses.h"


/// Register pass to for opt
// static llvm::RegisterPass<intel::PostDominanceFrontier> PostDominanceFrontierPass("postdomfrontier", "Post-Dominance Frontier Construction", true, true);

using namespace llvm;

namespace llvm {
template class DominanceFrontierBase<BasicBlock, true>;
template class ForwardDominanceFrontierBase<BasicBlock>;
}

namespace intel {

char PostDominanceFrontier::ID = 0;

OCL_INITIALIZE_PASS_BEGIN(PostDominanceFrontier, "postdomfrontier",
                "Post-Dominance Frontier Construction", true, true)
OCL_INITIALIZE_PASS_DEPENDENCY(PostDominatorTreeWrapperPass)
OCL_INITIALIZE_PASS_END(PostDominanceFrontier, "postdomfrontier",
                "Post-Dominance Frontier Construction", true, true)

PostDominanceFrontier::PostDominanceFrontier()
  : FunctionPass(ID),
    Base() {
  initializePostDominanceFrontierPass(*PassRegistry::getPassRegistry());
}

void PostDominanceFrontier::releaseMemory() {
  Base.releaseMemory();
}

bool PostDominanceFrontier::runOnFunction(Function &) {
  releaseMemory();
  Base.analyze(getAnalysis<PostDominatorTreeWrapperPass>().getPostDomTree());
  return false;
}

void PostDominanceFrontier::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<PostDominatorTreeWrapperPass>();
}

void PostDominanceFrontier::print(raw_ostream &OS, const Module *) const {
  Base.print(OS);
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void PostDominanceFrontier::dump() const {
  print(dbgs());
}
#endif

} // end namespace intel
