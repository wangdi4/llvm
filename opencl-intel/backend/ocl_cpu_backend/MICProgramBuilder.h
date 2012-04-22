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

#include <assert.h>
#include <string>
#include "exceptions.h"
#include "cl_dev_backend_api.h"
#include "ProgramBuilder.h"
#include "MICCompiler.h"

namespace llvm {
    class Module;
    class MICCodeGenerationEngine;
}

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class BuiltinModule;
class MICCompilerConfig;
class Program;
class Kernel;
class MICProgram;
class MICKernel;
class MICKernelJITProperties;


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

protected:

    const Compiler* GetCompiler() const
    {
        return &m_compiler;
    }

    KernelSet* CreateKernels(const Program* pProgram,
                             llvm::Module* pModule, 
                             ProgramBuildResult& buildResult) const;

    void PostOptimizationProcessing(Program* pProgram, llvm::Module* spModule) const;

private:
    MICKernelJITProperties* CreateKernelJITProperties(llvm::Module* pModule, 
                                                   llvm::Function* pFunc,
                                                   const TLLVMKernelInfo& info) const;

    MICKernel* CreateKernel(llvm::Function* pFunc, const std::string& funcName, KernelProperties* pProps) const;

    void AddKernelJIT( const MICProgram* pProgram, Kernel* pKernel, llvm::Module* pModule, llvm::Function* pFunc, MICKernelJITProperties* pProps) const;


    // Klockwork Issue
    MICProgramBuilder ( const MICProgramBuilder& x );

    // Klockwork Issue
    MICProgramBuilder& operator= ( const MICProgramBuilder& x );
private:
    std::string m_ErrorStr;
    MICCompiler m_compiler;
};

}}}
