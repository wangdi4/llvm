/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#include "KernelAnalysis.h"
#include "OpenclRuntime.h"
#include "LoopUtils.h"
#include "OCLPassSupport.h"

#include "llvm/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Constants.h"

#include <string.h>


extern "C" {
  void fillNoBarrierPathSet(const Module *M, std::set<std::string>& noBarrierPath)
  {
    noBarrierPath.clear();
    NamedMDNode *noBarrier =M->getNamedMetadata("cl.noBarrierPath.kernels");
    if (!noBarrier) return;
    assert (noBarrier->getNumOperands() == 1 &&
        "expected single operand pointing to set of names");
    MDNode *noBarrierNames = noBarrier->getOperand(0);
    for (unsigned i=0; i < noBarrierNames->getNumOperands(); ++i) {
      MDString *kernelName = dyn_cast<MDString>(noBarrierNames->getOperand(i));
      assert(kernelName && "operand is not an MDString");
      std::string funcName = kernelName->getString().str();
      noBarrierPath.insert(funcName);
    }
  }
}

namespace intel {

char KernelAnalysis::ID = 0;

OCL_INITIALIZE_PASS(KernelAnalysis, "kernel-analysis", "analyzes which function go in barrier route", false, false)

KernelAnalysis::KernelAnalysis():
ModulePass(ID)
{
}

KernelAnalysis::~KernelAnalysis()
{
}

void KernelAnalysis::fillBarrierUsersFuncs() {
  Function *barrierDcl = m_M->getFunction(BARRIER_FUNC_NAME);
  if (barrierDcl) {
    FSet barrierRootSet;
    barrierRootSet.insert(barrierDcl);
    LoopUtils::fillFuncUsersSet(barrierRootSet, m_unsupportedFunc);
  }
}

void KernelAnalysis::fillUnsupportedTIDFuncs() {
  FSet directTIDUsers;
  fillUnsupportedTIDFuncs(GET_LID_NAME, directTIDUsers);
  fillUnsupportedTIDFuncs(GET_GID_NAME, directTIDUsers);
  LoopUtils::fillFuncUsersSet(directTIDUsers, m_unsupportedFunc);
}

bool KernelAnalysis::isUnsupportedDim(Value *v) {
  ConstantInt *constDim = dyn_cast<ConstantInt>(v);
  // If arg is not a constant return true
  if (!constDim) return true;
  // Also if it is illegal constant
  unsigned dim =  constDim->getValue().getZExtValue();
  if (dim > 2) return true;
  return false;
}

void KernelAnalysis::fillUnsupportedTIDFuncs(const char *name,
                                             FSet &directTIDUsers) {
  Function *F = m_M->getFunction(name);
  if (!F) return;
  for (Function::use_iterator ui = F->use_begin(), ue = F->use_end();
       ui != ue; ++ui ) {
    CallInst *CI = dyn_cast<CallInst> (*ui);
    if (!CI) continue;
    // if the tid is variable add it to the set
    Function *callingFunc = CI->getParent()->getParent();
    directTIDUsers.insert(callingFunc);
    if ( isUnsupportedDim(CI->getArgOperand(0)) ) {
      m_unsupportedFunc.insert(CI->getParent()->getParent());
    }
  }
}

void KernelAnalysis::fillKernelCallers() {
  for (unsigned i=0, e=m_kernels.size(); i<e; ++i) {
    Function *kernel = m_kernels[i];
    if (!kernel) continue;
    FSet kernelRootSet;
    FSet kernelUsers;
    kernelRootSet.insert(kernel);
    LoopUtils::fillFuncUsersSet(kernelRootSet, kernelUsers);
    // The kernel has user functions meaning it is called by another kernel.
    // Since there is no barrier in it's start it will be executed
    // multiple time (because of the WG loop of the calling kernel)
    if (kernelUsers.size()) {
      m_unsupportedFunc.insert(kernel);
    }
  }

  // Also can not use explicit loops on kernel callers since the barrier
  // pass need to handle them in order to process the called kernels.
  FSet kernelSet (m_kernels.begin(), m_kernels.end());
  LoopUtils::fillFuncUsersSet(kernelSet, m_unsupportedFunc);
}

bool KernelAnalysis::runOnModule(Module& M) {
  m_M = &M;
  m_unsupportedFunc.clear();
  m_kernels.clear();
  LoopUtils::GetOCLKernel(M, m_kernels);

  fillKernelCallers();
  fillBarrierUsersFuncs();
  fillUnsupportedTIDFuncs();


  NamedMDNode *noBarrier = M.getOrInsertNamedMetadata("cl.noBarrierPath.kernels");
  SmallVector<Value *, 5> Operands;
  for (FVec::iterator fit = m_kernels.begin(), fe = m_kernels.end();
       fit != fe; ++fit) {
    Function *F = *fit;
    if (!F || m_unsupportedFunc.count(F)) continue;

    std::string funcName = F->getName().str();
    Operands.push_back(MDString::get(F->getContext(), funcName));
  }
  noBarrier->addOperand(MDNode::get(M.getContext(), Operands));
  return Operands.size();
}

void KernelAnalysis::print(raw_ostream &OS, const Module *M) const {
  if ( !M ) return;

  OS << "\nKernelAnalysis\n";
  std::set<std::string> noBarrierPath;
  fillNoBarrierPathSet(M, noBarrierPath);
  for (unsigned i=0, e = m_kernels.size(); i<e; ++i) {
    Function *F = m_kernels[i];
    if (!F) continue;

    std::string funcName = F->getName().str();
    if (noBarrierPath.count(funcName)) {
      OS << funcName << " yes\n";
    } else {
      OS << funcName << " no\n";
    }
  }
}

} //namespace intel
extern "C" ModulePass *createKernelAnalysisPass()
{
  return new intel::KernelAnalysis();
}





