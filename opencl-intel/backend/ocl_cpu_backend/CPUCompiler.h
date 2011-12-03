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

File Name: CPUCompiler.h

\*****************************************************************************/
#pragma once

#include <assert.h>
#include <string>
#include "Compiler.h"
#include "CPUDetect.h"
#include "exceptions.h"
#include "CompilerConfig.h"
#include "Kernel.h"
#include "Optimizer.h"
#include "llvm/Support/raw_ostream.h"

namespace llvm {
    class ExecutionEngine;
    class Module;
    class MemoryBuffer;
    class Type;
}

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class BuiltinLibrary;
class BuiltinModule;
class CompilerConfig;

//*****************************************************************************************
// Provides the module optimization and code generation functionality.
//
class CPUCompiler: public Compiler
{
public:
    /**
     * Ctor
     */
    CPUCompiler(const CompilerConfig& pConfig);
    virtual ~CPUCompiler();

    unsigned int GetTypeAllocSize(const llvm::Type* pType);

    // Returns pointer to jitted function if function hasn't been compiled
    // Otherwise function is jitted and pointer is returned
    void *GetPointerToFunction(llvm::Function *pf);

    // TODO: Hack. Need redesign.
    // Execution engine can be created with Built-ins module or images module.
    // By default execution engine is created in constructor for built-ins module
    // In order to skip built-ins module creation in images we need
    // to expose that interface and createExecution engine after constuctor is called.
    virtual void CreateExecutionEngine( llvm::Module* pModule );

    uint64_t GetJitFunctionStackSize(const llvm::Function* pf);

    unsigned int GetJitFunctionSize(const llvm::Function* pf);

    virtual void freeMachineCodeForFunction(llvm::Function* pf);

protected:

    /**
     * Returns pointer to the RTL library module
     */
    llvm::Module* GetRtlModule() const;

    llvm::Module* ParseModuleIR(llvm::MemoryBuffer* pIRBuffer);

private:
    void SelectCpu( const std::string& cpuName, const std::string& cpuFeatures );

    llvm::ExecutionEngine* CreateCPUExecutionEngine( llvm::Module* pModule );

private:
    CompilerConfig         m_config;
    BuiltinModule*         m_pBuiltinModule;
    llvm::ExecutionEngine* m_pExecEngine;
};

}}}
