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

#include "CLBuiltinLICM.h"
#include "LoopUtils/LoopUtils.h"
#include "OCLPassSupport.h"
#include "InitializePasses.h"

#include "llvm/InitializePasses.h"

namespace intel {

char intel::CLBuiltinLICM::ID = 0;

OCL_INITIALIZE_PASS_BEGIN(CLBuiltinLICM, "CLBuiltinLICM", "hoist known uniform openCL builtins out of loops", false, false)
OCL_INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
OCL_INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
OCL_INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfo)
OCL_INITIALIZE_PASS_END(CLBuiltinLICM, "CLBuiltinLICM", "hoist known uniform openCL builtins out of loops", false, false)

CLBuiltinLICM::CLBuiltinLICM() : LoopPass(ID), m_rtServices(nullptr) {
  initializeCLBuiltinLICMPass(*PassRegistry::getPassRegistry());
}

bool CLBuiltinLICM::runOnLoop(Loop *L, LPPassManager &LPM) {
  //errs() << "CLBuiltinLICM on " << L->getHeader()->getNameStr() << "\n";
  if (!L->isLoopSimplifyForm()) return false;

  m_rtServices = static_cast<OpenclRuntime *>(getAnalysis<BuiltinLibInfo>().getRuntimeServices());
  assert(m_rtServices && "expected to have openCL runtime");

  m_preHeader = L->getLoopPreheader();
  m_curLoop = L;
  m_header = m_curLoop->getHeader();
  m_DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
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
      Instruction *I = &*II; II++;
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
  assert(nullptr != CI && "Cannot operate with the nullptr!");
  if (!CI->getCalledFunction()) {
    return false; // ignore indirect calls
  }

  std::string funcName = CI->getCalledFunction()->getName().str();
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


