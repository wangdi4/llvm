/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#include "ModuleCleanup.h"
#include "CompilationUtils.h"
#include "BlockUtils.h"
#include "OCLPassSupport.h"
#include "OclTune.h"

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/Support/InstIterator.h"


using namespace llvm;
using namespace Intel::OpenCL::DeviceBackend;

extern "C"
{
  ModulePass* createModuleCleanupPass(bool SpareOnlyWrappers=false) {
      return new intel::ModuleCleanup(SpareOnlyWrappers);
  }
}

namespace intel{

  char ModuleCleanup::ID = 0;
  /// Register pass to for opt
 OCL_INITIALIZE_PASS(ModuleCleanup, "module-cleanup", "Cleans OpenCL module: removes functions which are not kernels (or called by kernels)", false, false)

  bool ModuleCleanup::runOnModule(Module& M) {
    bool didDeleteAny = false;

    // Grab list of kernels from module
    if (SpareOnlyWrappers)
      CompilationUtils::getAllKernelWrappers(m_neededFuncsSet, &M);
    else
      CompilationUtils::getAllKernels(m_neededFuncsSet, &M);

    // Erase body of all functions which are not needed
    for (Module::iterator i = M.begin(), e = M.end(); i != e; ++i) {
      Function *func = &*i;
      if (func->isDeclaration()) continue; // skip externals
      if ( (CompilationUtils::getCLVersionFromModuleOrDefault(M) >=
                              OclVersion::CL_VER_2_0) &&
         BlockUtils::isBlockInvokeFunction(*func)) continue; // skip block_invoke functions
      if (isNeededByKernel(func)) continue; // skip needed functions
      func->deleteBody();
      // erase function stats from the metadata
      intel::Statistic::removeFunctionStats(*func);
      didDeleteAny = true;
    }
    return didDeleteAny;
  }

  // This is a recursive function which checks whether a function is used
  // by a kernel. It marks all such functions along the way, to save on
  // extra checks in follow up queries
  bool ModuleCleanup::isNeededByKernel(Function* func) {
    if (m_neededFuncsSet.count(func)) return true;

    // Getting here, this function is not a kernel. list its users..
    std::set<Function*> funcUsers;
    std::set<Function*>::iterator it, ie;
    SmallVector<Value*, 8> workList(func->use_begin(), func->use_end());
    std::set<Value *> visited; // list to avoid infinite loops

    while (workList.size()) {
      Value *user = workList.back();
      workList.pop_back();
      if (visited.count(user)) continue;
      visited.insert(user);

      Instruction *pI = dyn_cast<Instruction>(user);
      if (pI) {
        // Add underline function (of instruction) into list
        funcUsers.insert(pI->getParent()->getParent());
      } else {
        // This is not an instruction. Add it's users to the scanning list
        workList.append(user->use_begin(), user->use_end());
      }
    }

    // Scan all the users (functions), recursively
    for (it = funcUsers.begin(), ie = funcUsers.end(); it != ie; ++it) {
      bool isNeeded = isNeededByKernel(*it);
      if (isNeeded) {
        // Function "func" is needed! Add it to the list
        m_neededFuncsSet.insert(func);
        return true;
      }
    }

    // Scan did not find any kernel at it's root.
    return false;
  }

}
