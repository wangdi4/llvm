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

File Name:  CPUCompiler.h

\*****************************************************************************/
#pragma once

#include <assert.h>
#include <string>
#include "CPUDetect.h"
#include "exceptions.h"
#include "CompilerConfig.h"
#include "cl_dev_backend_api.h"
#include "Kernel.h"
#include "Optimizer.h"
#include "Compiler.h"

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
class CompilerConfig;
class Program;
class Kernel;
class KernelProperties;
class KernelGroupSet;
class ProgramBuildResult;


//*****************************************************************************************
// Provides the module optimization and code generation functionality. 
// 
class CPUCompiler : public Compiler
{

public:
    /**
     * Ctor
     */
    CPUCompiler(IAbstractBackendFactory* pBackendFactory, const CompilerConfig& pConfig);
    ~CPUCompiler();

    /**
     * Build the given program using the supplied build options
     */
    cl_dev_err_code BuildProgram(Program* pProgram, const CompilerBuildOptions* pOptions);

    llvm::Module* GetRtlModule() const;

    KernelSet* CreateKernels(const Program* pProgram,
                             llvm::Module* pModule, 
                             ProgramBuildResult& buildResult, 
                             FunctionWidthVector& vectorizedFunctions, 
                             KernelsInfoMap& kernelsInfo,
                             size_t specialBufferStride);

    void PostOptimizationProcessing(Program* pProgram, llvm::Module* spModule) { }

private:
    void SelectCpu( const std::string& cpuName, const std::string& cpuFeatures );

    llvm::ExecutionEngine* CreateExecutionEngine( llvm::Module* pModule );

    KernelJITProperties* CreateKernelJITProperties(llvm::Module* pModule, 
                                                   llvm::Function* pFunc,
                                                   const TLLVMKernelInfo& info);

    Kernel* CreateKernel(llvm::Function* pFunc, const std::string& funcName, const std::string& args, KernelProperties* pProps);

    size_t ResolveFunctionCalls(llvm::Module* pModule, llvm::Function* pFunc);

    void AddKernelJIT( Kernel* pKernel, llvm::Module* pModule, llvm::Function* pFunc, KernelJITProperties* pProps);

    // Klockwork Issue
    CPUCompiler ( const CPUCompiler& x );

    // Klockwork Issue
    CPUCompiler& operator= ( const CPUCompiler& x );

private:
    BuiltinModule*         m_pBuiltinModule;
    llvm::ExecutionEngine* m_pExecEngine;
};

}}}
