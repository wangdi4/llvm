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
#include "MetaDataApi.h"
#include "CompilationUtils.h"

#include "llvm/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Constants.h"

#include <string.h>

using namespace Intel::OpenCL::DeviceBackend;

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

void KernelAnalysis::fillSyncUsersFuncs() {
  FSet barrierRootSet;

  CompilationUtils::FunctionSet oclFunction;

  // Get all synchronize built-ins declared in module
  CompilationUtils::getAllSyncBuiltinsDcls(oclFunction, m_M);

  barrierRootSet.insert(oclFunction.begin(), oclFunction.end());

  LoopUtils::fillFuncUsersSet(barrierRootSet, m_unsupportedFunc);
}

void KernelAnalysis::fillUnsupportedTIDFuncs() {
  FSet directTIDUsers;
  std::string LID = CompilationUtils::mangledGetLID();
  std::string GID = CompilationUtils::mangledGetGID();
  fillUnsupportedTIDFuncs(LID.c_str(), directTIDUsers);
  fillUnsupportedTIDFuncs(GID.c_str(), directTIDUsers);
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

  Intel::MetaDataUtils mdUtils(m_M);

  fillKernelCallers();
  fillSyncUsersFuncs();
  fillUnsupportedTIDFuncs();

  for (FVec::iterator fit = m_kernels.begin(), fe = m_kernels.end();
       fit != fe; ++fit) {
    Function *F = *fit;
    if (!F) {
      assert(false && "Kernel Metadata contains NULL kernel");
      continue;
    }
    if(m_unsupportedFunc.count(F)) {
      mdUtils.getOrInsertKernelsInfoItem(F)->setNoBarrierPath(false);
    } else {
      mdUtils.getOrInsertKernelsInfoItem(F)->setNoBarrierPath(true);
    }
  }

  //Save Metadata to the module
  mdUtils.save(m_M->getContext());
  return (m_kernels.size() != 0);
}

void KernelAnalysis::print(raw_ostream &OS, const Module *M) const {
  if ( !M ) return;
  Intel::MetaDataUtils mdUtils(const_cast<Module*>(M));

  OS << "\nKernelAnalysis\n";

  for (unsigned i=0, e = m_kernels.size(); i<e; ++i) {
    Function *F = m_kernels[i];
    if (!F) continue;

    std::string funcName = F->getName().str();
    Intel::KernelInfoMetaDataHandle kimd = mdUtils.getKernelsInfoItem(F);
    if (kimd->getNoBarrierPath()) {
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





