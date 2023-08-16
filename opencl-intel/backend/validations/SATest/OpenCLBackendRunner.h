// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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

#ifndef OPENCL_BACKEND_RUNNER_H
#define OPENCL_BACKEND_RUNNER_H

#include "IBufferContainerList.h"
#include "IProgramRunner.h"
#include "OpenCLProgram.h"
#include "OpenCLProgramConfiguration.h"
#include "OpenCLRunConfiguration.h"
#include "auto_ptr_ex.h"
#include "cl_dev_backend_api.h"
#include "mem_utils.h"
#include "llvm/IR/Module.h"

namespace Validation {

// FIXME 'using namespace' shouldn't be used in header file.
using namespace Intel::OpenCL::DeviceBackend;

/// @brief This class enables to run a single OpenCL test
class OpenCLBackendRunner : public IProgramRunner {
public:
  OpenCLBackendRunner(const BERunOptions &runConfig);
  ~OpenCLBackendRunner();

protected:
  /// @brief Loads the input buffer according to kernel configuration.
  /// @param pKernelConfig    [IN] kernel specific configuration
  /// @param pContainer       [INOUT] input data container
  void LoadInputBuffer(OpenCLKernelConfiguration *pKernelConfig,
                       IContainer *pContainer);

  /// @brief Creates OpenCL test program from binary buffer
  /// @param [IN] oclProgram OpenCL test program to execute
  /// @param [IN] pCompileService compile service to be used for creation
  /// @returns pointer to the created program
  ICLDevBackendProgram_ *
  CreateProgram(const OpenCLProgram *oclProgram,
                /*const*/ ICLDevBackendCompilationService *pCompileService);

  /// @brief Build OpenCL test program
  /// @param [IN]program - program to build
  /// @param [IN]pCompileServie - compilation service to use
  /// @param [OUT] runResult This method updates runResult build time
  /// @param [OUT] runConfig run time configuration to use
  /// @param [IN] pProgramConfig - program configuration to use
  void BuildProgram(ICLDevBackendProgram_ *program,
                    /*const*/ ICLDevBackendCompilationService *pCompileService,
                    IRunResult *runResult, const BERunOptions *runConfig,
                    const IProgramConfiguration *pProgramConfig);

  /// @brief Fills list of marks which arguments to ignore in comparator.
  /// @param [OUT] ignoreList The list of marks which arguments to ignore in
  /// comparator.
  /// @param [IN] pKernelArgs Descriptions of kernel's arguments.
  /// @param [IN] kernelNumArgs Number of kernel's arguments.
  void FillIgnoreList(std::vector<bool> &ignoreList,
                      const llvm::KernelArgument *pKernelArgs,
                      int kernelNumArgs);

protected:
  ICLDevBackendServiceFactory *m_pServiceFactory;
  llvm::Module *m_pModule;
  // needed for Random Data Generator
  uint64_t m_RandomDataGeneratorSeed;
};
} // namespace Validation

#endif // OPENCL_BACKEND_RUNNER_H
