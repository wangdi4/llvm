/*****************************************************************************\

Copyright (c) Intel Corporation (2012).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  OpenCLBackendRunner.h

\*****************************************************************************/
#ifndef OPENCL_BACKEND_RUNNER_H
#define OPENCL_BACKEND_RUNNER_H

#include "IProgramRunner.h"
#include "OpenCLProgram.h"
#include "OpenCLProgramConfiguration.h"
#include "OpenCLRunConfiguration.h"
#include "mem_utils.h"
#include "auto_ptr_ex.h"
#include "IBufferContainerList.h"

#include "cl_dev_backend_api.h"
#include "llvm/IR/Module.h"

using namespace Intel::OpenCL::DeviceBackend;

namespace Validation
{
    /// @brief This class enables to run a single OpenCL test
    class OpenCLBackendRunner : public IProgramRunner
    {
    public:
        OpenCLBackendRunner(const BERunOptions& runConfig);
        ~OpenCLBackendRunner();

        /// @brief Builds and executes a single test program
        /// @param [OUT] runResult Result of test program execution
        /// @param [IN] program Test program to execute
        /// @param [IN] programConfig Configuration of the test program
        /// @param [IN] runConfig Configuration of the test run
        virtual void Run(IRunResult* runResult,
                         const IProgram* program,
                         const IProgramConfiguration* programConfig,
                         const IRunComponentConfiguration* runConfig) = 0;

        /// @brief Load the Volcano output from the file.
        /// @param [OUT] runResult Result of test program execution
        /// @param [IN] config Configuration of the test run
        virtual void LoadOutput(IRunResult* pRunResult, const IProgramConfiguration* pConfig) = 0;

        /// @brief Store the Volcano output to the file.
        /// @param [IN] runResult Result of test program execution
        /// @param [IN] config Configuration of the test run
        virtual void StoreOutput(const IRunResult* pRunResult, const IProgramConfiguration* pConfig) const = 0;
    protected:
        /// @brief Loads the input buffer according to kernel configuration.
        /// @param pKernelConfig    [IN] kernel specific configuration
        /// @param pContainer       [INOUT] input data container
        void LoadInputBuffer(OpenCLKernelConfiguration* pKernelConfig, IContainer* pContainer );

        /// @brief Creates OpenCL test program from binary buffer
        /// @param [IN] oclProgram OpenCL test program to execute
        /// @param [IN] pCompileService compile service to be used for creation
        /// @returns pointer to the created program
        ICLDevBackendProgram_* CreateProgram(const OpenCLProgram * oclProgram, 
                                            /*const*/ ICLDevBackendCompilationService* pCompileService);

        /// @brief Build OpenCL test program
        /// @param [IN]program - program to build
        /// @param [IN]pCompileServie - compilation service to use
        /// @param [OUT] runResult This method updates runResult build time
        /// @param [OUT] runConfig run time configuration to use
        /// @param [IN] pProgramConfig - program configuration to use
        void BuildProgram( ICLDevBackendProgram_* program, 
                           /*const*/ ICLDevBackendCompilationService* pCompileService,
                           IRunResult * runResult, 
                           const BERunOptions* runConfig,
                           const IProgramConfiguration* pProgramConfig);

        /// @brief Fills list of marks which arguments to ignore in comparator.
        /// @param [OUT] ignoreList The list of marks which arguments to ignore in comparator.
        /// @param [IN] pKernelArgs Descriptions of kernel's arguments.
        /// @param [IN] kernelNumArgs Number of kernel's arguments.
        void FillIgnoreList( std::vector<bool>& ignoreList, const cl_kernel_argument* pKernelArgs, int kernelNumArgs );

    protected:
        ICLDevBackendServiceFactory* m_pServiceFactory;
        llvm::Module* m_pModule;
        //needed for Random Data Generator
        uint64_t m_RandomDataGeneratorSeed;
    };
}

#endif // OPENCL_BACKEND_RUNNER_H
