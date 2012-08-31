/*********************************************************************************************
 * Copyright � 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#include "CLBuiltinLICM.h"
#include "LoopUtils.h"

namespace intel {

CLBuiltinLICM::CLBuiltinLICM():
LoopPass(ID), 
m_rtServices(static_cast<OpenclRuntime *>(RuntimeServices::get()))
{
  initializeDominatorTreePass(*PassRegistry::getPassRegistry());
  initializeLoopInfoPass(*PassRegistry::getPassRegistry());
  assert(m_rtServices && "expected to have openCL runtime");
}

bool CLBuiltinLICM::runOnLoop(Loop *L, LPPassManager &LPM) {
  //errs() << "CLBuiltinLICM on " << L->getHeader()->getNameStr() << "\n";
  if (!L->isLoopSimplifyForm()) return false;
  m_preHeader = L->getLoopPreheader();
  m_curLoop = L;
  m_header = m_curLoop->getHeader();
  m_DT = &getAnalysis<DominatorTree>();
  m_changed = false;

  // recursively scan the loop according to the dominator tree order.
  ScanLoop(m_DT->getNode(m_header));

  return m_changed;
}

void CLBuiltinLICM::ScanLoop(DomTreeNode *N) {
  assert(N != 0 && "Null dominator tree node?");
  BasicBlock *BB = N->getBlock();

  // If this subregion is outside the current loop exit.
  if (!m_curLoop->contains(BB)) return;

  // We don't analyze instruction in sub-loops.
  // Loop Passes goes from inner loop to outer loops, values from more inner
  // are already hoisted.
  if (!LoopUtils::inSubLoop(m_curLoop, BB)) {
    for (BasicBlock::iterator II = BB->begin(), E = BB->end(); II != E;) {
      Instruction *I = II++;
      if (CallInst *CI = dyn_cast<CallInst>(I)) {
        m_changed |= hoistCLBuiltin(CI);
      }
    }
  }

  // Go over blocks recursively according to Dominator tree.
  const std::vector<DomTreeNode*> &Children = N->getChildren();
  for (unsigned i = 0, e = Children.size(); i != e; ++i) ScanLoop(Children[i]);
}

bool CLBuiltinLICM::hoistCLBuiltin(CallInst *CI) {
  std::string funcName = CI->getCalledFunction()->getNameStr();
  // To hoist the call it should have no side effect.
  if (!m_rtServices->isSafeToSpeculativeExecute(funcName) ) return false;

  // All it's operands should be invariant.
  for (unsigned i=0; i<CI->getNumArgOperands(); ++i) {
    if (!m_curLoop->isLoopInvariant(CI->getArgOperand(i))) return false;
  }

  // Can hoist the call move it to the pre - header.
  CI->moveBefore(m_preHeader->getTerminator());
  return true;
}

} //namespace intel

extern "C" {
  Pass* createCLBuiltinLICMPass() {
    return new intel::CLBuiltinLICM();
  }
}

char intel::CLBuiltinLICM::ID = 0;
static RegisterPass<intel::CLBuiltinLICM>
CLBuiltinLICM("CLBuiltinLICM", "hoist known uniform openCL builtins out of loops");

