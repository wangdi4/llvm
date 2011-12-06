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

File Name:  ProgramBuilder.h

\*****************************************************************************/
#pragma once

#include <assert.h>
#include <string>
#include <vector>
#include "exceptions.h"
#include "cl_dev_backend_api.h"
#include "CompilationUtils.h"
#include "IAbstractBackendFactory.h"

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
class KernelSet;
class ProgramBuildResult;
class BuiltinLibrary;
class Compiler;
struct TLLVMKernelInfo;

//*****************************************************************************************
// Represent the buid specific options passed to the compiler upon BuildProgram call
// 
class ProgramBuilderBuildOptions
{
};


//*****************************************************************************************
// Provides the module optimization and code generation functionality. 
// 
class ProgramBuilder
{
public:
    /**
     * Ctor
     */
    ProgramBuilder(IAbstractBackendFactory* pBackendFactory);
    ~ProgramBuilder();

public:
    /**
     * Build the given program using the supplied build options
     */
    cl_dev_err_code BuildProgram(Program* pProgram, const ProgramBuilderBuildOptions* pOptions);

protected:

    virtual Compiler* GetCompiler()= 0;

    virtual void PostOptimizationProcessing(Program* pProgram, llvm::Module* spModule) = 0;

    virtual KernelSet* CreateKernels(const Program* pProgram,
                             llvm::Module* pModule, 
                             ProgramBuildResult& buildResult) = 0;

    KernelProperties* CreateKernelProperties(const Program* pProgram, llvm::MDNode *elt, const TLLVMKernelInfo& info);

protected:

    // pointer to the containers factory (not owned by this class)
    IAbstractBackendFactory* m_pBackendFactory; 

};

}}}
