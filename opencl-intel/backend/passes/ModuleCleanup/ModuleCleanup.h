/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef __MODULE_CLEANUP_H__
#define __MODULE_CLEANUP_H__

#include <llvm/Pass.h>
#include <llvm/ADT/SmallSet.h>

namespace Intel { namespace OpenCL { namespace DeviceBackend  {
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


}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend  {

#endif // __MODULE_CLEANUP_H__
