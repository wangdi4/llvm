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

File Name:  DeviceCommunicationService.h

\*****************************************************************************/
#ifndef DEVICE_COMMUNICATION_SERVICE_H
#define DEVICE_COMMUNICATION_SERVICE_H

#include "IRunResult.h"
#include "OpenCLRunConfiguration.h"
#include "OpenCLProgramConfiguration.h"
#include "IContainer.h"
#include "Performance.h"
#include "auto_ptr_ex.h"

// COI library headers
#include <source/COIProcess_source.h>
#include <source/COIPipeline_source.h>
#include <source/COIEngine_source.h>
#include "source/COIBuffer_source.h"
#include "source/COIEvent_source.h"

#include "MICNative/common.h"

#include <string>
#include <vector>

#include "cl_dev_backend_api.h"

namespace Validation
{
    enum DEVICE_SIDE_FUNCTION
    {
        GET_BACKEND_TARGET_DESCRIPTION_SIZE = 0,
        GET_BACKEND_TARGET_DESCRIPTION,
        EXECUTE_KERNELS,
        DEVICE_SIDE_FUNCTION_COUNT // used as a count of functions
    };

    // Warning! This wrapper is tunned to be used in DeviceCommunicationService only 
    // and it supports only limited usage scenarios.
    class COIProcessAndPipelineWrapper
    {
    public:
        COIProcessAndPipelineWrapper():m_created(false){}
        void Create(COIENGINE engine);
        ~COIProcessAndPipelineWrapper(void);
        COIPROCESS&  GetProcessHandler()  {return m_process;}
        COIPIPELINE& GetPipelineHandler() {return m_pipeline;}
    private:
        bool m_created;
        COIPROCESS  m_process;
        COIPIPELINE m_pipeline;
        COILIBRARY  m_library; // SVML built-ins library
    };

    // Warning! This wrapper is tunned to be used in DeviceCommunicationService only 
    // and it supports only limited usage scenarios. For instance there is no way to map more then one buffer.
    class COIBuffersWrapper
    {
    public:
        COIBuffersWrapper(){}
        void AddBuffer( size_t bufferSize, const COIPROCESS process, COI_ACCESS_FLAGS access, uint32_t flags = 0 );
        void CreateBufferFromMemory( size_t bufferSize, const COIPROCESS process, COI_ACCESS_FLAGS access, void* pData );
        void Map( COI_MAP_TYPE mapType, int numOfDepends, COIEVENT* dependencies, void** data, size_t id );
        void UnMap();
        size_t GetNumberOfBuffers() const {return m_buffers.size();}
        ~COIBuffersWrapper( void );
        COIBUFFER* GetBufferHandler(size_t id) {return &(m_buffers[id]);}
        COI_ACCESS_FLAGS* GetBufferAccessFlags(size_t id) {return &(m_flags[id]);}
    private:
        // TODO: implement correct copy constructor.
        // HACK!!! Prohibit copying because if it's copied after AddBuffer call Destroy method will be called twice.
        COIBuffersWrapper(const COIBuffersWrapper&){}

        std::vector<COIBUFFER> m_buffers;
        std::vector<COI_ACCESS_FLAGS> m_flags;
        COIMAPINSTANCE m_map;
    };

    /// @brief
    class DeviceCommunicationService
    {
    public:
        DeviceCommunicationService(void);
        ~DeviceCommunicationService(void);
        void Init(const BERunOptions *pRunConfig);
        void RunKernels(const BERunOptions *pRunConfig,
                        OpenCLProgramConfiguration *programConfig,
                        IRunResult* runResult,
                        Intel::OpenCL::DeviceBackend::ICLDevBackendProgram_* pProgram);
        void SerializeProgram(Intel::OpenCL::DeviceBackend::ICLDevBackendSerializationService* pSerializer,
            Intel::OpenCL::DeviceBackend::ICLDevBackendProgram_* pProgram);
    private:
        /// @brief Loads the input buffer according to kernel configuration.
        /// @param pKernelConfig    [IN] kernel specific configuration
        /// @param pContainer       [INOUT] input data container
        void LoadInputBuffer(OpenCLKernelConfiguration* pKernelConfig, IContainer* pContainer );

        void PrepareInputData( BufferContainerList& input,
            char **kernelArgValues,
            ICLDevBackendProgram_* pProgram,
            OpenCLKernelConfiguration *const& pKernelConfig,
            const BERunOptions * pRunConfig,
            IRunResult* runResult,
            DispatcherData& dispatcherData);

        void CopyOutputData( BufferContainer& output,
                       const ICLDevBackendKernel_* pKernel,
                       DispatcherData& dispatcherData,
                       size_t& coiFuncArgsId );

        COI_ISA_TYPE GetCOIISAType(std::string cpuArch);

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

#endif // DEVICE_COMMUNICATION_SERVICE_H
