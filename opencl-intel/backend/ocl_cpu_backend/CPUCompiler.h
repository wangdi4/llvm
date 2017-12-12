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

#include "Compiler.h"
#include "ICompilerConfig.h"

#include <string>

namespace llvm {
    class ExecutionEngine;
    class Module;
    class JITEventListener;
}

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class BuiltinLibrary;
class BuiltinModules;

//*****************************************************************************************
// Provides the module optimization and code generation functionality.
//
class CPUCompiler: public Compiler
{
public:
    CPUCompiler(const ICompilerConfig& pConfig);
    virtual ~CPUCompiler();

    // Disable the copy ctor and assignment operator
    CPUCompiler( const CPUCompiler& ) = delete;
    bool operator = ( const CPUCompiler& ) = delete;

    // Returns pointer to jitted function if function hasn't been compiled
    // Otherwise function is jitted and pointer is returned
    void *GetPointerToFunction(llvm::Function *pf);

    // Create execution engine for the given module
    // Execution engine depends on module configuration
    void CreateExecutionEngine( llvm::Module* pModule ) override;

    // Get execution engine
    void *GetExecutionEngine() override { return m_pExecEngine; }

    void DumpJIT( llvm::Module* pModule, const std::string& filename) const;

    void SetObjectCache(ObjectCodeCache* pCache) override;

protected:
    // Returns a list of pointers to the RTL library modules
    llvm::SmallVector<llvm::Module*, 2> GetBuiltinModuleList() const override;

private:
    void SelectCpu( const std::string& cpuName, const std::string& cpuFeatures );

    llvm::ExecutionEngine* CreateCPUExecutionEngine( llvm::Module* pModule ) const;

private:
    BuiltinModules*         m_pBuiltinModule;
    llvm::ExecutionEngine*  m_pExecEngine;

    llvm::JITEventListener* m_pVTuneListener;
};

}}}
