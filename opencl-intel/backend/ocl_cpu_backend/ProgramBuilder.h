// INTEL CONFIDENTIAL
//
// Copyright 2010 Intel Corporation.
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
#include "IAbstractBackendFactory.h"
#include "ICLDevBackendOptions.h"
#include "ICompilerConfig.h"
#include "RuntimeService.h"

namespace llvm {
class Module;
class Function;
} // namespace llvm

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {
class Program;
class KernelProperties;
class KernelSet;
class ProgramBuildResult;
class Compiler;
class ObjectCodeCache;

namespace Utils {
/// @brief helper funtion to set RuntimeService in Kernel objects from KernelSet
void UpdateKernelsWithRuntimeService(const RuntimeServiceSharedPtr &rs,
                                     KernelSet *pKernels);

/// Apply runtime configurations to kernels.
void UpdateKernelsWithRuntimeConfig(const ICompilerConfig *Config,
                                    KernelSet *Kernels);
} // namespace Utils

//******************************************************************************
// Provides the module optimization and code generation functionality.
//
class ProgramBuilder {
public:
  /**
   * Ctor
   */
  ProgramBuilder(IAbstractBackendFactory *pBackendFactory,
                 std::unique_ptr<ICompilerConfig> config);
  virtual ~ProgramBuilder();

  /**
   * Build the given program using the supplied build options
   */
  cl_dev_err_code BuildProgram(Program *pProgram,
                               const ICLDevBackendOptions *pOptions,
                               const char *pBuildOpts);

  /// Finalize program, so that it is ready to create kernel from it.
  cl_dev_err_code FinalizeProgram(Program *Prog);

  /**
   * Parses the given program
   */
  void ParseProgram(Program *pProgram);

  /// Build backend library program and kernels.
  void BuildLibraryProgram(Program *Prog, std::string &KernelNames);

  /// Generate IR/Asm/Bin dump filename.
  std::string generateDumpFilename(const std::string &hash, unsigned fileId,
                                   const std::string &suffix) const;

protected:
  virtual Compiler *GetCompiler() = 0;
  virtual const Compiler *GetCompiler() const = 0;

  virtual void PostOptimizationProcessing(Program *pProgram) const = 0;

  virtual void JitProcessing(Program *program,
                             const ICLDevBackendOptions *options,
                             std::unique_ptr<llvm::TargetMachine> targetMachine,
                             ObjectCodeCache *objCache,
                             ProgramBuildResult &buildResult) = 0;

  virtual std::unique_ptr<KernelSet>
  CreateKernels(Program *pProgram, const char *pBuildOpts,
                ProgramBuildResult &buildResult) const = 0;

  KernelJITProperties *CreateKernelJITProperties(unsigned int vectorSize) const;

  KernelProperties *
  CreateKernelProperties(const Program *pProgram, llvm::Function *func,
                         const CompilerBuildOptions &buildOptions,
                         const ProgramBuildResult &buildResult) const;

  // reloads the program from his object binary
  virtual bool ReloadProgramFromCachedExecutable(Program *pProgram) = 0;
  // builds object binary for the built program
  virtual void BuildProgramCachedExecutable(ObjectCodeCache *pCache,
                                            Program *pProgram) const = 0;

  // pointer to the containers factory (not owned by this class)
  IAbstractBackendFactory *m_pBackendFactory;
  std::unique_ptr<ICompilerConfig> m_config;
  DeviceMode m_targetDevice;

private:
  /// @brief Dump stats collected for module if requested
  void DumpModuleStats(Program *program, llvm::Module *pModule,
                       bool isEqualizerStats = false);

  // Prefix of file name for IR/Asm/Bin dump.
  std::string m_dumpFilenamePrefix;
  // Workload name for the stats
  std::string m_statWkldName;
};
} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
