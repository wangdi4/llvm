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
#include "llvm/IR/InstIterator.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/SmallVector.h"


using namespace llvm;
using namespace Intel::OpenCL::DeviceBackend;

extern "C"
{
  ModulePass* createModuleCleanupPass(bool SpareOnlyWrappers=false) {
      return new intel::ModuleCleanup(SpareOnlyWrappers);
  }
}

namespace intel{

  /// Delete function body preserving attached Metadata
  static void deleteFunctionBody(Function *Func) {
    SmallVector<std::pair<unsigned, MDNode *>, 8> MDs;
    Func->getAllMetadata(MDs);

    Func->deleteBody();
    assert(!Func->getMetadata("kernel_wrapper")
      && "The surrounding code is not required anymore!");

    for (auto &MD : MDs) {
      Func->setMetadata(MD.first, MD.second);
    }
  }

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

      // TODO: replace by a check for globals initializers
      // keep pipes ctor, it is called by runStaticConstructorsDestructors
      if (func->getName() == "__global_pipes_ctor")
        continue;

      std::set<Value *> visited; // list to avoid infinite loops
      if (isNeededByKernel(func, visited)) continue; // skip needed functions
      deleteFunctionBody(func);
      // erase function stats from the metadata
      intel::Statistic::removeFunctionStats(*func);
      didDeleteAny = true;
    }

    // Erase global constants which are not needed.
    std::vector<GlobalVariable*> toDelete;
    for (Module::global_iterator I = M.global_begin(), E = M.global_end();
               I != E; ++I) {
      assert(isa<GlobalVariable>(*I) && "Global variable is expected.");
      GlobalVariable *GVar = cast<GlobalVariable>(I);
      // TODO: remove once SPIR 2.0 format for blocks is used.
      // Clang produces _NSConcreteGlobalBlock unresolved external variable which is not used by OpenCL.
      // MCJIT do not compile a module with unresolved external symbols, so we delete _NSConcreteGlobalBlock and replace all usages with undef.
      if (GVar->hasExternalLinkage() &&
         (GVar->getName() == "_NSConcreteGlobalBlock" || GVar->getName() == "_NSConcreteStackBlock")) {
        GVar->replaceAllUsesWith(UndefValue::get(GVar->getType()));
        toDelete.push_back(GVar);
        continue;
      }
      if (!GVar->isConstant()) continue;
      // Remove only image callback table and only if it's not used in the kernels.
      if (GVar->getName() != "coord_translate_i_callback" &&
          GVar->getName() != "soa4_coord_translate_i_callback" &&
          GVar->getName() != "soa8_coord_translate_i_callback" &&
          GVar->getName() != "soa16_coord_translate_i_callback") continue;
      // WORKAROUND: Even when image callback is inlined the use list for the callback table is not empty.
      // The check here should be like this:
      // if (!GVar->user_empty()) continue;
      // TODO: Check why not all uses were dropped when all calls were inlined.
      // Meanwhile we check that all functions that uses our global variable are used by some kernel.
      GVar->removeDeadConstantUsers();
      if(GVar->user_empty())
          toDelete.push_back(GVar);
    }
    for (std::vector<GlobalVariable*>::iterator i = toDelete.begin(), ie = toDelete.end(); i != ie; ++i) {
      (*i)->eraseFromParent();
      didDeleteAny = true;
    }
    return didDeleteAny;
  }

  // This is a recursive function which checks whether a function is used
  // by a kernel. It marks all such functions along the way, to save on
  // extra checks in follow up queries
  bool ModuleCleanup::isNeededByKernel(Function* func, std::set<Value*> &visited) {
    if (m_neededFuncsSet.count(func)) return true;

    // Getting here, this function is not a kernel. list its users..
    std::set<Function*> funcUsers;
    std::set<Function*>::iterator it, ie;
    SmallVector<Value*, 8> workList(func->users());

    while (workList.size()) {
      Value *user = workList.back();
      workList.pop_back();
      if (visited.insert(user).second == false) continue;

      Instruction *pI = dyn_cast<Instruction>(user);
      if (pI) {
        // Add underline function (of instruction) into list
        funcUsers.insert(pI->getParent()->getParent());
      } else {
        // This is not an instruction. Add it's users to the scanning list
        workList.append(user->user_begin(), user->user_end());
      }
    }

    // Scan all the users (functions), recursively
    for (it = funcUsers.begin(), ie = funcUsers.end(); it != ie; ++it) {
      bool isNeeded = isNeededByKernel(*it, visited);
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
