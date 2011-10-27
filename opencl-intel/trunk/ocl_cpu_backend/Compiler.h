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

File Name:  Compiler.h

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
#include "CompilationUtils.h"
#include "llvm/Support/raw_ostream.h"

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
class BuiltinLibrary;


//*****************************************************************************************
// Represent the buid specific options passed to the compiler upon BuildProgram call
// 
class CompilerBuildOptions
{
};

//*****************************************************************************************
// Represents the result of program build Holds the build result code and build log
// 
class ProgramBuildResult
{
public:
    ProgramBuildResult();

    bool Succeeded() const;
    
    bool Failed() const;
    
    std::string GetBuildLog() const;
    
    llvm::raw_ostream& LogS();

    void SetBuildResult( cl_dev_err_code code );
    
    cl_dev_err_code GetBuildResult();

private:
    cl_dev_err_code m_result;
    std::string m_buildLog;
    mutable llvm::raw_string_ostream m_logStream;
};

//*****************************************************************************************
// Provides the module optimization and code generation functionality. 
// 
class Compiler
{
public:
    /**
     * Ctor
     */
    Compiler(const CompilerConfig& pConfig);
    ~Compiler();

public:
    /**
     * Initializes the LLVM environment. 
     * Must be called from single threaded environment, before any 
     * instance of Compiler class are created
     */
    static void Init();
    /**
     * Terminate the LLVM environment.
     * Must be called from a single threaded environment, after all
     * the Compiler class instanced are destroyed
     */
    static void Terminate();

    /**
     * Build the given program using the supplied build options
     */
    cl_dev_err_code BuildProgram(Program* pProgram, const CompilerBuildOptions* pOptions);

    virtual llvm::Module* GetRtlModule() const = 0;

    llvm::LLVMContext*     GetLLVMContext() const { return m_pLLVMContext; }

protected:

    llvm::Module* ParseModuleIR(Program* pProgram);

    llvm::Module* CreateRTLModule(BuiltinLibrary* pLibrary);

    KernelProperties* CreateKernelProperties(const Program* pProgram, llvm::MDNode *elt, const TLLVMKernelInfo& info);

    virtual void PostOptimizationProcessing(Program* pProgram, llvm::Module* spModule) = 0;

    virtual KernelSet* CreateKernels(const Program* pProgram,
                             llvm::Module* pModule, 
                             ProgramBuildResult& buildResult, 
                             FunctionWidthVector& vectorizedFunctions, 
                             KernelsInfoMap& kernelsInfo,
                             size_t specialBufferStride) = 0;

protected:
    llvm::LLVMContext*     m_pLLVMContext;
    CompilerConfig         m_config;
    
    Intel::ECPU            m_selectedCpuId;
    unsigned int           m_selectedCpuFeatures;
    std::vector<std::string> m_forcedCpuFeatures;
};

}}}
