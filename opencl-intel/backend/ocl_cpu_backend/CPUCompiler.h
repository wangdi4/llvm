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

    unsigned int GetTypeAllocSize(llvm::Type* pType) const;

    // Returns pointer to jitted function if function hasn't been compiled
    // Otherwise function is jitted and pointer is returned
    void *GetPointerToFunction(llvm::Function *pf) const;

    // TODO: Hack. Need redesign.
    // Execution engine can be created with Built-ins module or images module.
    // By default execution engine is created in constructor for built-ins module
    // In order to skip built-ins module creation in images we need
    // to expose that interface and createExecution engine after constuctor is called.
    // TODO: make this method non-constant after re-design
    virtual void CreateExecutionEngine( llvm::Module* pModule ) const;

    uint64_t GetJitFunctionStackSize(const llvm::Function* pf) const;

    unsigned int GetJitFunctionSize(const llvm::Function* pf) const;

    virtual void freeMachineCodeForFunction(llvm::Function* pf) const;

protected:

    /**
     * Returns pointer to the RTL library module
     */
    llvm::Module* GetRtlModule() const;

    llvm::Module* ParseModuleIR(llvm::MemoryBuffer* pIRBuffer) const;

private:
    void SelectCpu( const std::string& cpuName, const std::string& cpuFeatures );

    llvm::ExecutionEngine* CreateCPUExecutionEngine( llvm::Module* pModule ) const;

private:
    CompilerConfig         m_config;
    BuiltinModule*         m_pBuiltinModule;
    // TODO: remove mutable after re-design
    mutable llvm::ExecutionEngine* m_pExecEngine;
};

}}}