/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#include "KernelAnalysis.h"
#include "llvm/Instructions.h"
#include "OpenclRuntime.h"
#include "llvm/Support/raw_ostream.h"
#include <string.h>
#include "llvm/Constants.h"
#include "LoopUtils.h"

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

char intel::KernelAnalysis::ID = 0;
namespace intel {
KernelAnalysis::KernelAnalysis():
ModulePass(ID)
{
}

KernelAnalysis::~KernelAnalysis()
{
}

void KernelAnalysis::fillDirectUsers(FSet *funcs, FSet *userFuncs,
                                     FSet *newUsers) {
  // Go through all of the funcs.
  for (FSet::iterator fit = funcs->begin(), fe = funcs->end();
       fit!=fe ; ++fit) {
    Function *F = *fit;
    if (!F) continue;
    // Go through all the users of the function.
    for (Function::use_iterator ui = F->use_begin(), ue = F->use_end();
         ui != ue; ++ui ) {
      // If the user is a call add the function contains it to userFuncs
      CallInst *CI = dyn_cast<CallInst> (*ui);
      if (!CI) continue;
      Function *callingFunc = CI->getParent()->getParent();
      if (userFuncs->insert(callingFunc).second) {
        // If the function is new update the new user set.
        newUsers->insert(callingFunc);
      }
    }
  }
}

void KernelAnalysis::fillFuncUsersSet(FSet &roots, FSet &userFuncs) {
  FSet newUsers1, newUsers2;
  FSet *pNewUsers = &newUsers1;
  FSet *pRoots = &newUsers2;
  // First Get the direct users of the roots.
  fillDirectUsers(&roots, &userFuncs, pNewUsers);
  while (pNewUsers->size()) {
    // iteratively swap between the new users sets, and use the current
    // as the roots for the new direct users.
    std::swap(pNewUsers, pRoots);
    pNewUsers->clear();
    fillDirectUsers(pRoots, &userFuncs, pNewUsers);
  }
}

void KernelAnalysis::fillBarrierUsersFuncs() {
  Function *barrierDcl = m_M->getFunction(BARRIER_FUNC_NAME);
  if (barrierDcl) {
    FSet barrierRootSet;
    barrierRootSet.insert(barrierDcl);
    fillFuncUsersSet(barrierRootSet, m_unsupportedFunc);
  }
}

void KernelAnalysis::fillUnsupportedTIDFuncs() {
  FSet directTIDUsers;
  fillUnsupportedTIDFuncs(GET_LID_NAME, directTIDUsers);
  fillUnsupportedTIDFuncs(GET_GID_NAME, directTIDUsers);
  fillFuncUsersSet(directTIDUsers, m_unsupportedFunc);
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
    // The kernel has users meaning it is called by another kernel.
    // Since there is no barrier in it's start it will be executed
    // multiple time (because of the WG loop of the calling kernel)
    if (kernel && kernel->getNumUses()) {
        m_unsupportedFunc.insert(kernel);
    }
  }
  
  // Also can not use explicit loops on kernel callers since the barrier 
  // pass need to handle them in order to process the called kernels.
  FSet kernelSet (m_kernels.begin(), m_kernels.end());
  fillFuncUsersSet(kernelSet, m_unsupportedFunc);
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
    
    std::string funcName = F->getNameStr();
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
    
    std::string funcName = F->getNameStr();
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

static RegisterPass<intel::KernelAnalysis> barrier_users("kernel-analysis", 
                          "analyzes which function go in barrier route");




