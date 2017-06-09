/*****************************************************************************\

Copyright (c) Intel Corporation (2010-2017).

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

#include "ICLDevBackendOptions.h"
#include "IAbstractBackendFactory.h"
#include "ICompilerConfig.h"

#include "Program.h"

namespace llvm {
    class Module;
    class Function;
    class MemoryBuffer;
}

namespace Intel { namespace OpenCL { namespace DeviceBackend {
class Program;
class KernelProperties;
class KernelSet;
class ProgramBuildResult;
class Compiler;
class ObjectCodeCache;

namespace Utils {
/// @returns the memory buffer of the Program object bytecode
llvm::MemoryBuffer* GetProgramMemoryBuffer(Program* pProgram);
/// @brief helper funtion to set RuntimeService in Kernel objects from KernelSet
void UpdateKernelsWithRuntimeService( const RuntimeServiceSharedPtr& rs, KernelSet * pKernels);
}

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
    virtual ~ProgramBuilder();

    /**
     * Build the given program using the supplied build options
     */
    cl_dev_err_code BuildProgram(Program* pProgram, const ICLDevBackendOptions* pOptions);

    /**
     * Parses the given program
     */
    void ParseProgram(Program* pProgram);

protected:

    virtual Compiler* GetCompiler() = 0;
    virtual const Compiler* GetCompiler() const = 0;

    virtual void PostOptimizationProcessing(Program* pProgram, llvm::Module* spModule, const ICLDevBackendOptions* pOptions) const = 0;

    virtual KernelSet* CreateKernels(Program* pProgram,
                             llvm::Module* pModule,
                             ProgramBuildResult& buildResult) const = 0;

    KernelJITProperties* CreateKernelJITProperties(unsigned int vectorSize) const;

    KernelProperties* CreateKernelProperties(const Program* pProgram,
                                             llvm::Function *func,
                                             const ProgramBuildResult& buildResult) const;

    // reloads the program from his object binary
    virtual bool ReloadProgramFromCachedExecutable(Program* pProgram) = 0;
    // builds object binary for the built program
    virtual void BuildProgramCachedExecutable(ObjectCodeCache* pCache, Program* pProgram) const = 0;

    /// @brief abstract factory method to create mapper from block to Kernel.
    /// Can be implemented differently for CPU and MIC.
    /// MIC will probably call this inside deserialization step
    /// CPU calls it inside PostOptimizationProcessing step
    /// When Block static resolution pass is ready we can implement is std::vector storage
    /// this will increase mapping performance in comparison to std::map
    /// @param pProgram
    /// @param pModule LLVM module
    /// @return IBlockToKernelMapper object
    virtual IBlockToKernelMapper * CreateBlockToKernelMapper(Program* pProgram, const llvm::Module* pModule) const = 0;

    /// @brief Post build step. Called inside BuildProgram() before exit
    virtual void PostBuildProgramStep(Program* pProgram, llvm::Module* pModule,
      const ICLDevBackendOptions* pOptions) const = 0;

    // pointer to the containers factory (not owned by this class)
    IAbstractBackendFactory* m_pBackendFactory;
    bool m_useVTune;

private:
    /// @brief Update the size of the variables in global adress space used by the program.
    void updateGlobalVariableTotalSize(Program* pProgram, llvm::Module* pModule);

    /// @brief Dump stats collected for module if requested
    void DumpModuleStats(llvm::Module* pModule);

    // base file name for stats
    std::string m_statFileBaseName;
    // Workload name for the stats
    std::string m_statWkldName;
};
}}}
