// INTEL CONFIDENTIAL
//
// Copyright 2010-2018 Intel Corporation.
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

#pragma once

#include "CPUCompiler.h"
#include "CPUProgram.h"
#include "ICompilerConfig.h"
#include "ProgramBuilder.h"
#include "cl_dev_backend_api.h"

#ifdef OCL_DEV_BACKEND_PLUGINS
#include "plugin_manager.h"
#endif

#include <string>

namespace llvm {
    class ExecutionEngine;
    class LLVMContext;
    class Module;
    class Program;
    class Function;
    class MemoryBuffer;
    class MDNode;
}

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class BuiltinModules;
class Program;
class Kernel;
class KernelProperties;
class KernelGroupSet;
class ObjectCodeCache;
class ProgramBuildResult;


//*****************************************************************************************
// Provides the module optimization and code generation functionality.
//
class CPUProgramBuilder : public ProgramBuilder
{

public:
    /**
     * Ctor
     */
    CPUProgramBuilder(IAbstractBackendFactory* pBackendFactory, const ICompilerConfig& pConfig);
    ~CPUProgramBuilder();

    Compiler* GetCompiler() { return &m_compiler; }
    const Compiler* GetCompiler() const { return &m_compiler; }

protected:

    KernelSet* CreateKernels(Program* pProgram,
                             llvm::Module* pModule,
                             ProgramBuildResult& buildResult) const;

    void PostOptimizationProcessing(Program* pProgram, llvm::Module* spModule, const ICLDevBackendOptions* pOptions) const;

    /// @brief inherited method. create mapper from block to Kernel
    /// @param pProgram
    /// @param llvm module
    /// @return IBlockToKernelMapper object
    virtual IBlockToKernelMapper * CreateBlockToKernelMapper(Program* pProgram,
      const llvm::Module* pModule) const;

    /// @brief Post build step. For CPU it is used for creating
    /// IBlockToKernelMapper object and running global ctors
    virtual void PostBuildProgramStep(Program* pProgram, llvm::Module* pModule,
      const ICLDevBackendOptions* pOptions) const ;

    // reloads the program container from the cached binary object
    virtual bool ReloadProgramFromCachedExecutable(Program* pProgram);
    // builds binary object for the built program
    virtual void BuildProgramCachedExecutable(ObjectCodeCache* pCache, Program* pProgram) const;

private:

    Kernel* CreateKernel(llvm::Function* pFunc, const std::string& funcName, KernelProperties* pProps) const;


    void AddKernelJIT(CPUProgram* pProgram, Kernel* pKernel, llvm::Module* pModule,
                      llvm::Function* pFunc, KernelJITProperties* pProps) const;

    // Klockwork Issue
    CPUProgramBuilder ( const CPUProgramBuilder& x );

    // Klockwork Issue
    CPUProgramBuilder& operator= ( const CPUProgramBuilder& x );

private:
    CPUCompiler m_compiler;
    #ifdef OCL_DEV_BACKEND_PLUGINS
    mutable Intel::OpenCL::PluginManager m_pluginManger;
    #endif
};

}}}
