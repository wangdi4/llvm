/*****************************************************************************\

Copyright (c) Intel Corporation (2011).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  OpenCLProgramRunner.h

\*****************************************************************************/
#ifndef OPENCL_PROGRAM_RUNNER_H
#define OPENCL_PROGRAM_RUNNER_H

#include "IProgramRunner.h"
#include "OpenCLProgram.h"
#include "OpenCLProgramConfiguration.h"
#include "OpenCLRunConfiguration.h"
#include "mem_utils.h"

#include "cl_dev_backend_api.h"
#include "cl_device_api.h"
#include "CL/cl.h"

using namespace Intel::OpenCL::DeviceBackend;

namespace Validation
{
    typedef auto_ptr_ex<ICLDevBackendCompilationService, ReleaseDP<ICLDevBackendCompilationService> > ICLDevBackendCompileServicePtr;
    typedef auto_ptr_ex<ICLDevBackendExecutionService, ReleaseDP<ICLDevBackendExecutionService> > ICLDevBackendExecutionServicePtr;
    typedef auto_ptr_ex<ICLDevBackendProgram_, ReleaseDP<ICLDevBackendProgram_> > ICLDevBackendProgramPtr;
    typedef auto_ptr_ex<ICLDevBackendBinary_, ReleaseDP<ICLDevBackendBinary_> > ICLDevBackendBinaryPtr;
    typedef auto_ptr_ex<ICLDevBackendExecutable_, ReleaseDP<ICLDevBackendExecutable_> > ICLDevBackendExecutablePtr;

    class IBufferContainerList;

    /// @brief This class enables to run a single OpenCL test
    class OpenCLProgramRunner : public IProgramRunner
    {
    public:
        OpenCLProgramRunner();
        ~OpenCLProgramRunner();

        /// @brief Builds and executes a single test program
        /// @param [OUT] runResult Result of test program execution
        /// @param [IN] program Test program to execute
        /// @param [IN] programConfig Configuration of the test program
        /// @param [IN] runConfig Configuration of the test run
        virtual void Run(IRunResult* runResult,
                         IProgram* program,
                         IProgramConfiguration* programConfig,
                         const IRunComponentConfiguration* runConfig);

        /// @brief Load the Volcano output from the file.
        /// @param [OUT] runResult Result of test program execution
        /// @param [IN] config Configuration of the test run
        virtual void LoadOutput(IRunResult* pRunResult, IProgramConfiguration* pConfig){}

        /// @brief Store the Volcano output to the file.
        /// @param [IN] runResult Result of test program execution
        /// @param [IN] config Configuration of the test run
        virtual void StoreOutput(IRunResult* pRunResult, IProgramConfiguration* pConfig){}
    private:
        /// @brief Loads the input buffer according to kernel configuration.
        /// @param pKernelConfig    [IN] kernel specific configuration
        /// @param pContainer       [INOUT] input data container
        void LoadInputBuffer(OpenCLKernelConfiguration* pKernelConfig, IContainer* pContainer );

        /// @brief Creates OpenCL test program from binary buffer
        /// @param [IN] oclProgram OpenCL test program to execute
        /// @param [IN] pCompileService compile service to be used for creation
        /// @returns pointer to the created program
        ICLDevBackendProgram_* CreateProgram(OpenCLProgram * oclProgram, 
                                            ICLDevBackendCompilationService* pCompileService);

        /// @brief Build OpenCL test program
        /// @param [IN]program - program to build
        /// @param [IN]pCompileServie - compilation service to use
        /// @param [OUT] runResult This method updates runResult build time
        /// @param [OUT] runConfig run time configuration to use
        void BuildProgram( ICLDevBackendProgram_* program, 
                           ICLDevBackendCompilationService* pCompileService,
                           IRunResult * runResult, 
                           const BERunOptions* runConfig);

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
                           OpenCLKernelConfiguration * oclConfig,
                           const BERunOptions* runConfig);

        /// @brief Fills list of marks which arguments to ignore in comparator.
        /// @param [OUT] ignoreList The list of marks which arguments to ignore in comparator.
        /// @param [IN] pKernelArgs Descriptions of kernel's arguments.
        /// @param [IN] kernelNumArgs Number of kernel's arguments.
        void FillIgnoreList( std::vector<bool>& ignoreList, const cl_kernel_argument* pKernelArgs, int kernelNumArgs );

    private:
        ICLDevBackendServiceFactory* m_pServiceFactory;
    };
}



#endif // OPENCL_PROGRAM_RUNNER_H
