// INTEL CONFIDENTIAL
//
// Copyright 2011-2018 Intel Corporation.
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
    typedef auto_ptr_ex<ICLDevBackendKernel_, ReleaseDP<ICLDevBackendKernel_> > ICLDevBackendKernelPtr;

    class IBufferContainerList;

    /// @brief This class enables to run a single OpenCL test
    class OpenCLCPUBackendRunner : public OpenCLBackendRunner
    {
    public:
        OpenCLCPUBackendRunner(const BERunOptions& runConfig);
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
                           ICLDevBackendImageService* pImageService,
                           OpenCLKernelConfiguration * oclConfig,
                           const BERunOptions* runConfig);

        void GetMemoryBuffersDescriptions(size_t* IN pBufferSizes, 
                                          size_t* INOUT pBufferCount );

    };
}



#endif // OPENCL_CPU_BACKEND_RUNNER_H
