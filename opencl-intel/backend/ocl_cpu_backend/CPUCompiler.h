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

#include "llvm/Support/raw_ostream.h"

#include <assert.h>
#include <string>

namespace llvm {
    class ExecutionEngine;
    class Module;
    class MemoryBuffer;
    class Type;
    class JITEventListener;
}

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class BuiltinLibrary;
class BuiltinModule;

//*****************************************************************************************
// Provides the module optimization and code generation functionality.
//
class CPUCompiler: public Compiler
{
public:
    /**
     * Ctor
     */
    CPUCompiler(const ICompilerConfig& pConfig);
    virtual ~CPUCompiler();

    // Returns pointer to jitted function if function hasn't been compiled
    // Otherwise function is jitted and pointer is returned
    void *GetPointerToFunction(llvm::Function *pf);

    // Create execution engine for the given module
    // Execution engine depends on module configuration
    virtual void CreateExecutionEngine( llvm::Module* pModule );

    // Get execution engine
    virtual void *GetExecutionEngine() { return m_pExecEngine; }

    virtual void freeMachineCodeForFunction(llvm::Function* pf) const;

    void DumpJIT( llvm::Module* pModule, const std::string& filename) const;

protected:

    /**
     * Returns pointer to the RTL library module
     */
    llvm::Module* GetRtlModule() const;


private:
    // Disable the copy ctor and assignment operator
    CPUCompiler( const CPUCompiler& );
    bool operator = ( const CPUCompiler& );

    void SelectCpu( const std::string& cpuName, const std::string& cpuFeatures );

    llvm::ExecutionEngine* CreateCPUExecutionEngine( llvm::Module* pModule ) const;

    BuiltinModule*         m_pBuiltinModule;
    llvm::ExecutionEngine* m_pExecEngine;

    llvm::JITEventListener* m_pVTuneListener;
};

}}}
