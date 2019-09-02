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
#include "ICLDevBackendOptions.h"
#include "IAbstractBackendFactory.h"
#include "ICompilerConfig.h"

#include "RuntimeService.h"

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
    cl_dev_err_code BuildProgram(Program* pProgram, const ICLDevBackendOptions* pOptions, const char* pBuildOpts);

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
                             const char* pBuildOpts,
                             ProgramBuildResult& buildResult) const = 0;

    KernelJITProperties* CreateKernelJITProperties(unsigned int vectorSize) const;

    KernelProperties *
    CreateKernelProperties(const Program *pProgram, llvm::Function *func,
                           const CompilerBuildOptions &buildOptions,
                           const ProgramBuildResult &buildResult) const;

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
    DeviceMode m_targetDevice;
    int m_forcedPrivateMemorySize;
    int m_forcedWorkGroupSize;

private:
    /// @brief Dump stats collected for module if requested
    void DumpModuleStats(llvm::Module* pModule, bool isEqualizerStats = false);

    // base file name for stats
    std::string m_statFileBaseName;
    // Workload name for the stats
    std::string m_statWkldName;
};
}}}
