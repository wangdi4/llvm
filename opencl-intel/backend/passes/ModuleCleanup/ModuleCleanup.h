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

File Name:  ModuleCleanup.h

\*****************************************************************************/

#ifndef __MODULE_CLEANUP_H__
#define __MODULE_CLEANUP_H__

#include <llvm/Pass.h>
#include <llvm/ADT/SmallSet.h>

namespace intel{
    using namespace llvm;

    /// @brief Cleans OpenCL module: removes functions which are not kernels (or called by kernels)
    class ModuleCleanup : public ModulePass
    {
    public:
        /// @brief Constructor
        ModuleCleanup(SmallVectorImpl<Function*> &vectFunctions)
          : ModulePass(ID), m_pVectFunctions(&vectFunctions) {}

        /// @brief LLVM Module pass entry
        /// @param M Module to transform
        /// @returns true if changed
        bool runOnModule(Module& M);

    private:
        /// @brief Go over metadata + vectorized kernels list, and extract kernels.
        void obtainKernelsList();

        /// @brief Scan recursively if function is used by kernel.
        bool isNeededByKernel(Function* func);

    public:
        /// @brief Pass identification, replacement for typeid
        static char ID;

    private:
        /// @brief Current module
        Module *m_module;

        /// @brief List of all vectorized kernels in module
        SmallVectorImpl<Function*> *m_pVectFunctions;

        /// @brief Set of all functions which are deemed necessary
        SmallSet<Function*, 32> m_neededFuncsSet;

    };


} // namespace Intel { namespace OpenCL { namespace DeviceBackend  {

#endif // __MODULE_CLEANUP_H__
