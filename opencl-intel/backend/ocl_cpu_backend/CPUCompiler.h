// INTEL CONFIDENTIAL
//
// Copyright 2010-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#pragma once

#include "Compiler.h"
#include "ICompilerConfig.h"

#include "llvm/Target/TargetMachine.h"

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

    void DumpJIT( llvm::Module* pModule, const std::string& filename,
                  llvm::CodeGenFileType genType =
                  llvm::CGFT_AssemblyFile) const;
    void SetObjectCache(ObjectCodeCache* pCache) override;

    void SetBuiltinModules(const std::string& cpuName, const std::string& cpuFeatures) override;
protected:
    // Returns a list of pointers to the RTL library modules
    llvm::SmallVector<llvm::Module*, 2> GetBuiltinModuleList() const override;

private:
    void SelectCpu( const std::string& cpuName, const std::string& cpuFeatures );
    bool useLLDJITForExecution(llvm::Module* pModule) const;
    llvm::ExecutionEngine* CreateCPUExecutionEngine( llvm::Module* pModule ) const;

private:
    BuiltinModules*         m_pBuiltinModule;
    llvm::ExecutionEngine*  m_pExecEngine;

    llvm::JITEventListener* m_pVTuneListener;
};

}}}
