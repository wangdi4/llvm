// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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

#include "KernelAnalysis.h"
#include "OpenclRuntime.h"
#include "LoopUtils/LoopUtils.h"
#include "OCLPassSupport.h"
#include "MetadataAPI.h"
#include "CompilationUtils.h"

#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Constants.h"

#include <string.h>

using namespace Intel::OpenCL::DeviceBackend;
using namespace Intel::MetadataAPI;

namespace intel {

char KernelAnalysis::ID = 0;

OCL_INITIALIZE_PASS(KernelAnalysis, "kernel-analysis", "analyzes which function go in barrier route", false, false)

KernelAnalysis::KernelAnalysis() :
ModulePass(ID), m_M(nullptr)
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
  for (Function::user_iterator ui = F->user_begin(), ue = F->user_end();
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
  m_kernels = KernelList(&M).getList();

  fillKernelCallers();
  fillSyncUsersFuncs();
  fillUnsupportedTIDFuncs();

  for (auto *pFunc : m_kernels) {
    assert(pFunc && "nullptr is not expected in KernelList!");
    auto kimd = KernelInternalMetadataAPI(pFunc);
    if(m_unsupportedFunc.count(pFunc)) {
      kimd.NoBarrierPath.set(false);
    } else {
      kimd.NoBarrierPath.set(true);
    }
  }

  return (m_kernels.size() != 0);
}

void KernelAnalysis::print(raw_ostream &OS, const Module *M) const {
  if ( !M ) return;

  OS << "\nKernelAnalysis\n";

  for (unsigned i=0, e = m_kernels.size(); i<e; ++i) {
    Function *F = m_kernels[i];
    if (!F) continue;

    std::string funcName = F->getName().str();
    auto kimd = KernelInternalMetadataAPI(F);
    if (kimd.NoBarrierPath.get()) {
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





