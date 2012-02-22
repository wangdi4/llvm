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

File Name:  OpenCLMICBackendRunner.h

\*****************************************************************************/
#ifndef OPENCL_MIC_BACKEND_RUNNER_H
#define OPENCL_MIC_BACKEND_RUNNER_H

#include "IRunResult.h"
#include "IContainer.h"
#include "Performance.h"
#include "OpenCLBackendRunner.h"
#include "auto_ptr_ex.h"
#include "mem_utils.h"
#include "OpenCLProgramConfiguration.h"
#include "OpenCLProgram.h"
#include "OpenCLRunConfiguration.h"
#include "COIHelpers.h"

#include "cl_dev_backend_api.h"

// COI library headers
#include <source/COIProcess_source.h>
#include <source/COIPipeline_source.h>
#include <source/COIEngine_source.h>
#include "source/COIBuffer_source.h"
#include "source/COIEvent_source.h"

#include "MICNative/common.h"

#include <string>
#include <vector>

using namespace Intel::OpenCL::DeviceBackend;

namespace Validation
{
    enum DEVICE_SIDE_FUNCTION
    {
        INIT_DEVICE = 0,
        GET_BACKEND_TARGET_DESCRIPTION_SIZE,
        GET_BACKEND_TARGET_DESCRIPTION,
        EXECUTE_KERNELS,
        DEVICE_SIDE_FUNCTION_COUNT // used as a count of functions
    };

    /// @brief This class enables single OpenCL test execution using OpenCL MIC Device back-end.
    typedef auto_ptr_ex<ICLDevBackendCompilationService, ReleaseDP<ICLDevBackendCompilationService> > ICLDevBackendCompileServicePtr;
    typedef auto_ptr_ex<ICLDevBackendSerializationService, ReleaseDP<ICLDevBackendSerializationService> > ICLDevBackendSerializationServicePtr;
    typedef auto_ptr_ex<ICLDevBackendProgram_, ReleaseDP<ICLDevBackendProgram_> > ICLDevBackendProgramPtr;

    class OpenCLMICBackendRunner : public OpenCLBackendRunner
    {
    public:
        OpenCLMICBackendRunner(const IRunComponentConfiguration* pRunConfiguration);
        ~OpenCLMICBackendRunner();

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
        virtual void StoreOutput(const IRunResult* pRunResult, const IProgramConfiguration* pConfig) const{}

    private:
        void RunKernels(const BERunOptions *pRunConfig,
                        OpenCLProgramConfiguration *programConfig,
                        IRunResult* runResult,
                        Intel::OpenCL::DeviceBackend::ICLDevBackendProgram_* pProgram);
        void SerializeProgram(Intel::OpenCL::DeviceBackend::ICLDevBackendSerializationService* pSerializer,
            Intel::OpenCL::DeviceBackend::ICLDevBackendProgram_* pProgram);

        /// @brief Copies input data from Data Manager structure - BufferContainerList to COIBuffers.
        /// @param input           [IN] input data for test program
        /// @param kernelArgValues [OUT] Buffer with kernel name, kernel arguments which are passed by value and array of directives which defines buffers contant (kernel argument, "printf" buffer, etc.) 
        /// @param pProgram        [IN] pointer to the test program
        /// @param pKernelConfig   [IN] contains kernel name to run and workspace configuraton
        /// @param runResult       [OUT] initialized with input data and ignore list for comparator
        /// @param dispatcherData  [OUT] keeps details about data layout in COI buffers.
        void PrepareInputData( BufferContainerList& input,
            char **kernelArgValues,
            ICLDevBackendProgram_* pProgram,
            OpenCLKernelConfiguration *const& pKernelConfig,
            IRunResult* runResult,
            DispatcherData& dispatcherData);

        void CopyOutputData( BufferContainer& output,
                       const ICLDevBackendKernel_* pKernel,
                       size_t& coiFuncArgsId );


        COIProcessAndPipelineWrapper m_procAndPipe;
        // m_coiFuncArgs structure: 
        // 1. Program
        // 2. Per kernel
        //   2.1 Arguments passed through pointer in separate buffers.
        //   2.2 Buffer with kernel name, kernel usual args and directives
        // 3. dispatches
        COIBuffersWrapper m_coiFuncArgs;

        // Target machine description
        auto_ptr_ex<char, ArrayDP<char> > m_pTargetDesc;
        uint64_t m_targetDescSize;

        static const char* m_device_function_names[DEVICE_SIDE_FUNCTION_COUNT];
        COIFUNCTION m_device_functions[DEVICE_SIDE_FUNCTION_COUNT];
        Sample m_serializationTime;
    };
}

#endif // OPENCL_MIC_BACKEND_RUNNER_H
