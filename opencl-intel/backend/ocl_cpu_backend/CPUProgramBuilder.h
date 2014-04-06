/*****************************************************************************\

Copyright (c) Intel Corporation (2010).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  CPUProgramBuilder.h

\*****************************************************************************/
#pragma once

#include "CPUCompiler.h"
#include "CPUProgram.h"
#include "ICompilerConfig.h"
#include "ProgramBuilder.h"
#include "cl_dev_backend_api.h"

#ifdef OCL_DEV_BACKEND_PLUGINS
#include "plugin_manager.h"
#endif
#include "ObjectCodeCache.h"

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

class BuiltinModule;
class Program;
class Kernel;
class KernelProperties;
class KernelGroupSet;
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

#ifdef ENABLE_KNL
    void LoadObject(Program* pProgram, llvm::Module* spModule, const char *start,
                    size_t size) const;
#endif // ENABLE_KNL

    /// @brief inherited method. create mapper from block to Kernel
    /// @param pProgram
    /// @param llvm module
    /// @return IBlockToKernelMapper object
    virtual IBlockToKernelMapper * CreateBlockToKernelMapper(Program* pProgram,
      const llvm::Module* pModule) const;

    /// @brief Post build step. Used for creating IBlockToKernelMapper object on CPU
    virtual void PostBuildProgramStep(Program* pProgram, llvm::Module* pModule,
      const ICLDevBackendOptions* pOptions) const ;

    // reloads the program container from the cached binary object
    virtual void ReloadProgramFromCachedExecutable(Program* pProgram);
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
