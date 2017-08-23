//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "PostDominanceFrontier.h"
#include "llvm/Analysis/DominanceFrontierImpl.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/CFG.h"
#include "llvm/Support/Debug.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SetOperations.h"
#include "OCLPassSupport.h"
#include "InitializePasses.h"


/// Register pass to for opt
// static llvm::RegisterPass<intel::PostDominanceFrontier> PostDominanceFrontierPass("postdomfrontier", "Post-Dominance Frontier Construction", true, true);

using namespace llvm;

namespace llvm {
template class DominanceFrontierBase<BasicBlock>;
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
