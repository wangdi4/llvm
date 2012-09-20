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
#include "Optimizer.h"

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
class ICompilerConfig;
class Program;
class Kernel;
class KernelProperties;
class KernelSet;
class ProgramBuildResult;
class BuiltinLibrary;
class Compiler;
struct TLLVMKernelInfo;

//*****************************************************************************************
// Provides the module optimization and code generation functionality. 
// 
class ProgramBuilder
{
public:
    /**
     * Ctor
     */
    ProgramBuilder(IAbstractBackendFactory* pBackendFactory, const ICompilerConfig& config);
    ~ProgramBuilder();

public:
    /**
     * Build the given program using the supplied build options
     */
    cl_dev_err_code BuildProgram(Program* pProgram, const ICLDevBackendOptions* pOptions);

protected:

    virtual Compiler* GetCompiler() = 0;
    virtual const Compiler* GetCompiler() const = 0;

    virtual void PostOptimizationProcessing(Program* pProgram, llvm::Module* spModule, const ICLDevBackendOptions* pOptions) const = 0;

    virtual KernelSet* CreateKernels(Program* pProgram,
                             llvm::Module* pModule, 
                             ProgramBuildResult& buildResult) const = 0;

    KernelJITProperties* CreateKernelJITProperties(unsigned int vectorSize) const;
    
    KernelProperties* CreateKernelProperties(const Program* pProgram, 
                                             Function *func, 
                                             Function *pWrapperFunc,
                                             const ProgramBuildResult& buildResult) const;


protected:

    // pointer to the containers factory (not owned by this class)
    IAbstractBackendFactory* m_pBackendFactory; 
    bool m_useVTune;
};

}}}
