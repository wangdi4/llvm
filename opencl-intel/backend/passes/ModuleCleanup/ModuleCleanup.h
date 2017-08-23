/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef __MODULE_CLEANUP_H__
#define __MODULE_CLEANUP_H__

#include <llvm/Pass.h>
#include "CompilationUtils.h"

#include <set>

namespace intel{
    using namespace llvm;

    /// @brief Cleans OpenCL module: removes functions which are not kernels (or called by kernels)
    class ModuleCleanup : public ModulePass
    {
    const bool SpareOnlyWrappers;
    public:
      // C-tor - ig SpareOnlyWrappers is true, only kernel wrapper will remain in the module, otherwise also
      // internal kernels will remain
      ModuleCleanup(bool SpareOnlyWrappers=true)
          : ModulePass(ID), SpareOnlyWrappers(SpareOnlyWrappers) {}

        /// @brief LLVM Module pass entry
        /// @param M Module to transform
        /// @returns true if changed
        bool runOnModule(Module& M);

    private:
        /// @brief Scan recursively if function is used by kernel.
        bool isNeededByKernel(Function* func, std::set<Value*> &visited);

    public:
        /// @brief Pass identification, replacement for typeid
        static char ID;

    private:
        /// @brief Current module
        Module *m_module;

        typedef Intel::OpenCL::DeviceBackend::CompilationUtils::FunctionSet FunctionSet;
        /// @brief Set of all functions which are deemed necessary
        FunctionSet m_neededFuncsSet;

    };


} // namespace Intel { namespace OpenCL { namespace DeviceBackend  {

#endif // __MODULE_CLEANUP_H__
