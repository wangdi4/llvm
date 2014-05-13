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

File Name:  MICProgramBuilder.h

\*****************************************************************************/
#pragma once

#include "Compiler.h"
#include "MICCompiler.h"
#include "ModuleJITHolder.h"
#include "ProgramBuilder.h"
#ifdef OCL_DEV_BACKEND_PLUGINS
#include "plugin_manager.h"
#endif
#include "IMICCompilerConfig.h"

#include <string>

namespace llvm {
    class Function;
    class LLVMModuleJITHolder;
    class Module;
}

namespace Intel { namespace OpenCL { namespace DeviceBackend {
class IAbstractBackendFactory;
class ICLDevBackendOptions;
class Kernel;
class KernelJITProperties;
class KernelProperties;
class KernelSet;
class MICKernel;
class MICProgram;
class Program;
class ProgramBuildResult;

//*****************************************************************************************
// Provides the module optimization and code generation functionality.
//
class MICProgramBuilder : public ProgramBuilder
{
public:
    /**
     * Ctor
     */
    MICProgramBuilder(IAbstractBackendFactory* pBackendFactory, const IMICCompilerConfig& pConfig);
    ~MICProgramBuilder();

    // checks if the given program has an object binary to be loaded from
    virtual bool CheckIfProgramHasCachedExecutable(Program* pProgram) const {return false;}
    // reloads the program from his object binary
    virtual void ReloadProgramFromCachedExecutable(Program* pProgram);
    // builds object binary for the built program
    virtual void BuildProgramCachedExecutable(ObjectCodeCache* pCache, Program* pProgram) const; 

protected:

    Compiler* GetCompiler() { return &m_compiler; }
    const Compiler* GetCompiler() const { return &m_compiler; }

    KernelSet* CreateKernels(Program* pProgram,
                             llvm::Module* pModule,
                             ProgramBuildResult& buildResult) const;

    void PostOptimizationProcessing(Program* pProgram, llvm::Module* spModule, const ICLDevBackendOptions* pOptions) const;

    /// @brief create mapper from block to Kernel
    virtual IBlockToKernelMapper * CreateBlockToKernelMapper(Program* pProgram,
      const llvm::Module* pModule) const;

    /// @brief Post build step. Used for creating IBlockToKernelMapper object on CPU
    /// For MIC currently it does nothing
    virtual void PostBuildProgramStep(Program* pProgram, llvm::Module* pModule,
      const ICLDevBackendOptions* pOptions) const {};

private:

    MICKernel* CreateKernel(llvm::Function* pFunc, const std::string& funcName, KernelProperties* pProps) const;

    void AddKernelJIT( const MICProgram* pProgram, Kernel* pKernel, llvm::Module* pModule, llvm::Function* pFunc, KernelJITProperties* pProps) const;

    void CopyJitHolder(llvm::LLVMModuleJITHolder* from, ModuleJITHolder* to) const;

    // Klockwork Issue
    MICProgramBuilder ( const MICProgramBuilder& x );

    // Klockwork Issue
    MICProgramBuilder& operator= ( const MICProgramBuilder& x );
private:
    std::string m_ErrorStr;
    MICCompiler m_compiler;
    #ifdef OCL_DEV_BACKEND_PLUGINS
    mutable Intel::OpenCL::PluginManager   m_pluginManager;
    #endif
};

}}}
