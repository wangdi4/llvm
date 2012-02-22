/*****************************************************************************\

Copyright (c) Intel Corporation (2011-2012).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  OpenCLCPUBackendRunner.h

\*****************************************************************************/
#ifndef OPENCL_CPU_BACKEND_RUNNER_H
#define OPENCL_CPU_BACKEND_RUNNER_H

#include "OpenCLBackendRunner.h"
#include "OpenCLProgram.h"
#include "OpenCLProgramConfiguration.h"
#include "OpenCLRunConfiguration.h"
#include "mem_utils.h"
#include "auto_ptr_ex.h"

#include "cl_dev_backend_api.h"

using namespace Intel::OpenCL::DeviceBackend;

namespace Validation
{
    typedef auto_ptr_ex<ICLDevBackendCompilationService, ReleaseDP<ICLDevBackendCompilationService> > ICLDevBackendCompileServicePtr;
    typedef auto_ptr_ex<ICLDevBackendExecutionService, ReleaseDP<ICLDevBackendExecutionService> > ICLDevBackendExecutionServicePtr;
    typedef auto_ptr_ex<ICLDevBackendImageService, ReleaseDP<ICLDevBackendImageService> > ICLDevBackendImageServicePtr;
    typedef auto_ptr_ex<ICLDevBackendProgram_, ReleaseDP<ICLDevBackendProgram_> > ICLDevBackendProgramPtr;
    typedef auto_ptr_ex<ICLDevBackendBinary_, ReleaseDP<ICLDevBackendBinary_> > ICLDevBackendBinaryPtr;
    typedef auto_ptr_ex<ICLDevBackendExecutable_, ReleaseDP<ICLDevBackendExecutable_> > ICLDevBackendExecutablePtr;

    class IBufferContainerList;

    /// @brief This class enables to run a single OpenCL test
    class OpenCLCPUBackendRunner : public OpenCLBackendRunner
    {
    public:
        OpenCLCPUBackendRunner();
        ~OpenCLCPUBackendRunner();

        /// @brief Builds and executes a single test program
        /// @param [OUT] runResult Result of test program execution
        /// @param [IN] program Test program to execute
        /// @param [IN] programConfig Configuration of the test program
        /// @param [IN] runConfig Configuration of the test run
        virtual void Run(IRunResult* runResult,
                         const IProgram* program,
                         const IProgramConfiguration* programConfig,
                         const IRunComponentConfiguration* runConfig);

        /// @brief Load the Volcano output from the file.
        /// @param [OUT] runResult Result of test program execution
        /// @param [IN] config Configuration of the test run
        virtual void LoadOutput(IRunResult* pRunResult, const IProgramConfiguration* pConfig){}

        /// @brief Store the Volcano output to the file.
        /// @param [IN] runResult Result of test program execution
        /// @param [IN] config Configuration of the test run
        virtual void StoreOutput(const IRunResult* pRunResult, const IProgramConfiguration* pConfig) const {}
    private:
        /// @brief Executes OpenCL test kernel (actually executes all the kernels in the given program)
        /// @param [IN] input  input buffers container
        /// @param [OUT] runResult This method updates runResult execution time.
        /// @param [IN] program  back-end program to run the kernels from
        /// @param [IN] pExecutionService back-end execution service to use for execution
        /// @param [IN] oclConfig OpenCL configuration of the test run.
        /// @param [IN] runConfig OpenCL back-end configuration of the test run.
        void ExecuteKernel(IBufferContainerList& input,
                           IRunResult * runResult,  
                           ICLDevBackendProgram_* program,
                           ICLDevBackendExecutionService* pExecutionService,
                           ICLDevBackendImageService* pImageService,
                           OpenCLKernelConfiguration * oclConfig,
                           const BERunOptions* runConfig);
    };
}



#endif // OPENCL_CPU_BACKEND_RUNNER_H
