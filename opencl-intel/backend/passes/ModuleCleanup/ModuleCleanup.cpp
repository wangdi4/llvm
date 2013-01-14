/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#include "ModuleCleanup.h"

#include <llvm/Pass.h>
#include <llvm/Module.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Instructions.h>
#include <llvm/ADT/SmallSet.h>
#include <llvm/Support/InstIterator.h>


using namespace llvm;

namespace Intel { namespace OpenCL { namespace DeviceBackend  {

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
            // We expect the metadata nodes to be llvm::Function
            // In case the cast is wrong an assertion failure will be thrown
            Function *pWrapperFunc = llvm::cast<llvm::Function>(
                welt->getOperand(0)->stripPointerCasts());
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
