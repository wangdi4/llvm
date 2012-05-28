/*****************************************************************************\

Copyright (c) Intel Corporation (2010-2012).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  ModuleCleanup.cpp 

\*****************************************************************************/
#include <llvm/Pass.h>
#include <llvm/Module.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Instructions.h>
#include <llvm/ADT/SmallSet.h>
#include <llvm/Support/InstIterator.h>



using namespace llvm;

namespace Intel { namespace OpenCL { namespace DeviceBackend  {


  // Cleans OpenCL module: removes functions which are not kernels (or called by kernels)
  class ModuleCleanup : public ModulePass 
  {
  public:
    ModuleCleanup(SmallVectorImpl<Function*> &vectFunctions) 
      : ModulePass(ID), m_pVectFunctions(&vectFunctions) {
    }

    bool runOnModule(Module& M);

  private:
    // Go over metadata + vectorized kernels list, and extract kernels.
    void obtainKernelsList();

    // Scan recursively if function is used by kernel.
    bool isNeededByKernel(Function* func);


  private:
    static char ID; // LLVM pass ID
    
    // Current module
    Module *m_module;

    // List of all vectorized kernels in module
    SmallVectorImpl<Function*> *m_pVectFunctions;

    // Set of all functions which are deemed necessary
    SmallSet<Function*, 32> m_neededFuncsSet;

  };

  char ModuleCleanup::ID = 0;

  ModulePass* createModuleCleanupPass(SmallVectorImpl<Function*> &vectFunctions)
  {
    return new ModuleCleanup(vectFunctions);
  }


  bool ModuleCleanup::runOnModule(Module& M)
  {
    bool didDeleteAny = false;
    m_module = &M;

    // Grab list of kernels from module
    obtainKernelsList();

    // Erase body of all functions which are not needed
    for (Module::iterator i = m_module->begin(), e = m_module->end(); i != e; ++i)
    {
      Function *func = &*i;
      if (func->isDeclaration()) continue; // skip externals
      if (isNeededByKernel(func)) continue; // skip needed functions
      func->deleteBody();
      didDeleteAny = true;
    }
    return didDeleteAny;
  }

  // This is a recursive function which checks whether a function is used
  // by a kernel. It marks all such functions along the way, to save on
  // extra checks in follow up queries
  bool ModuleCleanup::isNeededByKernel(Function* func)
  {
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
    for (it = funcUsers.begin(), ie = funcUsers.end(); it != ie; ++it)
    {
      bool isNeeded = isNeededByKernel(*it);
      if (isNeeded) 
      {
        // Function "func" is needed! Add it to the list
        m_neededFuncsSet.insert(func);
        return true; 
      }
    }

    // Scan did not find any kernel at it's root.
    return false;
  }


  // Go over metadata + vectorized kernels list, and extract kernels.
  void ModuleCleanup::obtainKernelsList()
  {
    // Scan metadata - Go over all kernels in module, and add them to neededFunctionsList
    NamedMDNode *WrapperMD = m_module->getNamedMetadata("opencl.wrappers");
    if (!WrapperMD) return;

    for (unsigned i = 0, e = WrapperMD->getNumOperands(); i != e; ++i)
    {
      MDNode *welt = WrapperMD->getOperand(i);
      Function *pWrapperFunc = llvm::dyn_cast<llvm::Function>(
        welt->getOperand(0)->stripPointerCasts());
      if (!pWrapperFunc) continue;

      m_neededFuncsSet.insert(pWrapperFunc);
    }

    // Go over all vectorized functions and add them to list as well
    for (unsigned i = 0; i < m_pVectFunctions->size(); ++i)
    {
      if ((*m_pVectFunctions)[i])
      {
        m_neededFuncsSet.insert((*m_pVectFunctions)[i]);
      }
    }
  }

}}}
