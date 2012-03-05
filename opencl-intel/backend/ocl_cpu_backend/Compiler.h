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
#include "Optimizer.h"
#include "llvm/Support/raw_ostream.h"

namespace llvm {
    class ExecutionEngine;
    class LLVMContext;
    class Module;
    class Function;
    class MemoryBuffer;
    class Type;
}

extern bool s_ignore_termination;

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class BuiltinLibrary;
class BuiltinModule;
class CompilerConfig;
class ProgramBuildResult;


namespace Utils
{
/**
 * Class used to block the destruction of Compiler internal data 
 * after backend dll unload
 *
 * Problem: 
 * OCL SDK permit declaration of static OCL object (like Context)
 * that may hold the pointer to backend object (CompilerService for example)
 * The problem with such approach is that upon application termination the
 * backend dll could be unloaded prior to the above static object destruction.
 *
 * Solution: 
 * We define the special static object ( that would be initilized after 
 * any other static object in OCL SDK ). Upon backend dll unload this static object's
 * destructor will be called first and will set the special flag. Using this flag,
 * other backend objects (Compiler, CompilerService) may decide to abort their 
 * desctuction in order to avoid crashes.
 * 
 */
class TerminationBlocker
{
public:
    ~TerminationBlocker() { s_released = true; s_ignore_termination = true;}
    static bool IsReleased() { return s_released; }
private:
    static bool s_released;
};
}

//*****************************************************************************************
// Represent the buid specific options passed to the compiler upon BuildProgram call
// 
class CompilerBuildOptions
{
public:
    CompilerBuildOptions( bool debugInfo,
                          bool disableOpt,
                          bool relaxedMath,
                          bool libraryModule):
      m_debugInfo(debugInfo),
      m_disableOpt(disableOpt),
      m_relaxedMath(relaxedMath),
      m_libraryModule(libraryModule)
    {}

    bool GetDisableOpt()    const { return m_debugInfo; }
    bool GetDebugInfoFlag() const { return m_disableOpt; }
    bool GetRelaxedMath()   const { return m_relaxedMath; }
    bool GetlibraryModule()   const { return m_libraryModule; }

private:
    bool m_debugInfo; 
    bool m_disableOpt;
    bool m_relaxedMath;
    // Sets whether optimized code is library module or a set of kernels
    // If this options is set to true then some optimization passes will be skipped
    bool m_libraryModule;
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

    // Set the functions width vector. 
    // The passed pointer will be owned by the ProgramBuildResult
    void SetFunctionsWidths( FunctionWidthVector* pv);

    const FunctionWidthVector& GetFunctionsWidths() const;

    // Set the kernel information map. 
    // The passed pointer will be owned by the ProgramBuildResult
    void SetKernelsInfo( KernelsInfoMap* pKernelsInfo);

    KernelsInfoMap& GetKernelsInfo();

    std::map<std::string, unsigned int>& GetPrivateMemorySize();

private:
    cl_dev_err_code m_result;
    std::string m_buildLog;
    mutable llvm::raw_string_ostream m_logStream;
    FunctionWidthVector* m_pFunctionWidths; 
    KernelsInfoMap*      m_pKernelsInfo;
    std::map<std::string, unsigned int> m_privateMemorySizeMap;
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
    virtual ~Compiler();

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
    llvm::Module* BuildProgram(llvm::MemoryBuffer* pIRBuffer,
                      const CompilerBuildOptions* pOptions,
                      ProgramBuildResult* pResult) const;

    ECPU GetSelectedCPU() const
    {
        return m_selectedCpuId;
    }

    unsigned int GetSelectedCPUFeatures() const
    {
        return m_selectedCpuFeatures;
    }

    // TODO: make this method non-constant after re-design
    virtual void CreateExecutionEngine(llvm::Module* m) const = 0;

    virtual unsigned int GetTypeAllocSize(llvm::Type* pType) const = 0;

protected:
    /**
     * Returns pointer to the RTL library module
     */
    virtual llvm::Module* GetRtlModule() const = 0;

    llvm::Module* ParseModuleIR(llvm::MemoryBuffer* pIRBuffer) const;

    llvm::Module* CreateRTLModule(BuiltinLibrary* pLibrary) const;

protected:
    CompilerConfig         m_config;
    
    llvm::LLVMContext*     m_pLLVMContext;
    Intel::ECPU            m_selectedCpuId;
    unsigned int           m_selectedCpuFeatures;
    std::vector<std::string> m_forcedCpuFeatures;
};

}}}