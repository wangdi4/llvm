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

File Name: MICCompiler.h

\*****************************************************************************/
#pragma once

#include "Compiler.h"
#include "ICompilerConfig.h"
#include "IMICCompilerConfig.h"
#include "MICJITEngine/include/IFunctionAddressResolver.h"

#include <assert.h>
#include <string>

namespace llvm {
    class ExecutionEngine;
    class Module;
    class MemoryBuffer;
    class Type;
    class MICCodeGenerationEngine;
    class LLVMModuleJITHolder;
    class IFunctionAddressResolver;
}

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class BuiltinLibrary;
class BuiltinModule;

//*****************************************************************************************
// Wrapper of the Backend interface IDynamicFunctionsResolver to behave as
// llvm::IFunctionAddressResolver

class FunctionResolverWrapper : public llvm::IFunctionAddressResolver
{
public:
    FunctionResolverWrapper() : m_pFuncResolver(NULL) { }

    void SetResolver(IDynamicFunctionsResolver* pResolver)
    {
        m_pFuncResolver = pResolver;
    }

    unsigned long long int getFunctionAddress(const std::string& func) const
    {
        assert(m_pFuncResolver && "Resolver is null");
        return m_pFuncResolver->GetFunctionAddress(func);
    }
private:
    // not owned by the class
    IDynamicFunctionsResolver* m_pFuncResolver;
};

//*****************************************************************************************
// Provides the module optimization and code generation functionality.
//
class MICCompiler: public Compiler
{
public:
    /**
     * Ctor
     */
    MICCompiler(const IMICCompilerConfig& pConfig);
    virtual ~MICCompiler();

    llvm::LLVMModuleJITHolder* GetModuleHolder(llvm::Module& module,
        const std::string& dumpAsm) const;

    virtual void *GetExecutionEngine() { return m_pCGEngine; }

    /**
     * Returns pointer to the RTL library module
     */
    llvm::Module* GetRtlModule() const;

    virtual void SetObjectCache(ObjectCodeCache* pCache) { return ; }

protected:
    llvm::Module* ParseModuleIR(llvm::MemoryBuffer* pIRBuffer);

    // !! WORKAROUND !!
    // Execution engine can be created with Built-ins module or images module.
    // By default execution engine is created in constructor for built-ins module
    // In order to skip built-ins module creation in images we need
    // to expose that interface and createExecution engine after constuctor is called.
    virtual void CreateExecutionEngine( llvm::Module* pModule );

private:
    // Disable copy ctor and assignment operator
    MICCompiler( const MICCompiler& );
    bool operator = ( const MICCompiler& );

    void SelectMICConfiguration(const ICompilerConfig& config);

    llvm::MICCodeGenerationEngine* CreateMICCodeGenerationEngine( llvm::Module* pRtlModule ) const;

private:
    BuiltinModule*           m_pBuiltinModule;
    mutable std::string              m_ErrorStr;
    llvm::MICCodeGenerationEngine* m_pCGEngine;
    FunctionResolverWrapper  m_ResolverWrapper;
};

}}}
