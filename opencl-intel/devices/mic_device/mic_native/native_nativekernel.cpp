// Copyright (c) 2006-2013 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

/////////////////////////////////////////////////////////////
//  native_nativekernel_task.cpp
/////////////////////////////////////////////////////////////

#include "native_nativekernel_task.h"
#include "native_program_service.h"
#include "device_queue.h"
#include "native_common_macros.h"
#include "mic_logger.h"

// TODO: Move to system utils
#include "mic_native_logger.h"

#include <cl_dev_backend_api.h>
#include <cl_shared_ptr.hpp>

#include <common/COIEvent_common.h>
#include <sink/COIBuffer_sink.h>
#include <sink/COIPipeline_sink.h>
#include <sink/COIProcess_sink.h>

using namespace Intel::OpenCL::MICDeviceNative;
using namespace Intel::OpenCL::MICDevice;
using namespace Intel::OpenCL::TaskExecutor;

///////////////////////////////////////////////////////////////////////////////
// Device side entry point for NDRange invocation
COINATIVELIBEXPORT
void execute_command_nativekernel(uint32_t   in_BufferCount,
                            void**           in_ppBufferPointers,
                            uint64_t*        in_pBufferLengths,
                            void*            in_pMiscData,
                            uint16_t         in_MiscDataLength,
                            void*            in_pReturnValue,
                            uint16_t         in_ReturnValueLength)
{
    MicInfoLog(MicNativeLogDescriptor::getLoggerClient(), MicNativeLogDescriptor::getClientId(), "%s", "[MIC SERVER] enter execute_native_kernel");
    assert( in_ReturnValueLength == 0 && "We should not return value for command");
    assert( in_MiscDataLength >= sizeof(ndrange_dispatcher_data) && "Size of input MiscData doesn't match");

    ndrange_dispatcher_data* ndrangeDispatchData = (ndrange_dispatcher_data*)in_pMiscData;

    QueueOnDevice* pQueue = (QueueOnDevice*)(ndrangeDispatchData->deviceQueuePtr);
    assert(nullptr != pQueue && "pQueue must be valid");
    
    cl_dev_err_code err = CL_DEV_SUCCESS;

    if (pQueue->IsAsyncExecution())
    {
        SharedPtr<NativeKernelTask> nativeKernelTask = NativeKernelTask::Allocate(in_BufferCount, in_ppBufferPointers, ndrangeDispatchData, in_MiscDataLength, pQueue);

        err = nativeKernelTask->getTaskError();
        if ( CL_DEV_SUCCEEDED(err) )
        {
            err = pQueue->Execute(nativeKernelTask->GetAsTaskHandlerBase());
        }
    }
    else
    {
        NativeKernelTask nativeKernelTask(in_BufferCount, in_ppBufferPointers, ndrangeDispatchData, in_MiscDataLength, pQueue);

        cl_dev_err_code err = nativeKernelTask.getTaskError();
        if ( CL_DEV_SUCCEEDED(err) )
        {
            // Increase ref. count to prevent undesired deletion
            nativeKernelTask.IncRefCnt();
            err = pQueue->Execute(nativeKernelTask.GetAsTaskHandlerBase());
        }
	}

    *((cl_dev_err_code*)in_pReturnValue) = err;
    MicInfoLog(MicNativeLogDescriptor::getLoggerClient(), MicNativeLogDescriptor::getClientId(), "%s", "[MIC SERVER] exit execute_NDRange");
}

NativeKernelTask::NativeKernelTask( uint32_t lockBufferCount, void** pLockBuffers, ndrange_dispatcher_data* pDispatcherData, size_t uiDispatchSize, QueueOnDevice* pQueue ) :
    TaskHandler<NativeKernelTask, ndrange_dispatcher_data >(lockBufferCount, pLockBuffers, pDispatcherData, uiDispatchSize, pQueue)
#ifdef ENABLE_MIC_TRACER
    ,m_tbb_perf_data(*this)
#endif
#ifdef USE_ITT
    ,m_pIttKernelName(nullptr), m_pIttKernelDomain(nullptr)
#endif
{
}

NativeKernelTask::~NativeKernelTask()
{
#ifdef ENABLE_MIC_TRACER
    m_tbb_perf_data.PerfDataFini();
#endif
}

// called immediately after creation and after filling the COI-passed data
bool NativeKernelTask::PrepareTask()
{
    // Set total buffers size and num of buffers for the tracer.
#ifdef ENABLE_MIC_TRACER
    commandTracer().add_delta_num_of_buffer_sent_to_device(m_bufferCount);
    unsigned long long bufSize = 0;
    for (unsigned int i = 0; i < m_bufferCount; i++)
    {
        bufSize = m_lockBufferLengths[i];
        commandTracer().add_delta_buffers_size_sent_to_device(bufSize);
    }
#endif

#ifdef USE_ITT
    if ( gMicGPAData.bUseGPA)
    {
        m_pIttKernelName = ProgramService::get_itt_kernel_name(m_pDispatcherData->kernelAddress);
        m_pIttKernelDomain = ProgramService::get_itt_kernel_domain(m_pDispatcherData->kernelAddress);
        // Use kernel specific domain if possible, if not available switch to global domain
        if ( nullptr == m_pIttKernelDomain )
        {
            m_pIttKernelDomain = gMicGPAData.pDeviceDomain;
        }
    }
#endif

    m_pKernel = static_cast<const Intel::OpenCL::BuiltInKernels::IBuiltInKernel*>(ProgramService::GetKernelPointer(m_pDispatcherData->kernelAddress));

    // Now we should patch memory object pointers
    const cl_kernel_argument* pParamInfo = m_pKernel->GetKernelParams();
    unsigned int        numMemArgs  = m_pKernel->GetMemoryObjectArgumentCount();
    const unsigned int* pMemArgsInx = m_pKernel->GetMemoryObjectArgumentIndexes();
    size_t              argSize     = m_pKernel->GetExplicitArgumentBufferSize();

    m_pKernelArgs = ((char*)m_pDispatcherData) + sizeof(ndrange_dispatcher_data);

    unsigned int currentLockedBuffer = 0;
    for(unsigned int i=0; i<numMemArgs; ++i)
    {
        void** loc = (void**)(m_pKernelArgs+pParamInfo[pMemArgsInx[i]].offset_in_bytes);
        // Skip NULL buffers
        if ( nullptr == *loc )
            continue;
        *loc = m_bufferPointers[currentLockedBuffer];
        currentLockedBuffer++;
    }

	m_bufferPointers = nullptr;

    return true;
}

bool NativeKernelTask::Execute()
{
#if defined(USE_ITT)
    if ( gMicGPAData.bUseGPA )
    {
        __itt_frame_begin_v3(m_pIttKernelDomain, nullptr);
    }
#endif

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( gMicGPAData.bUseGPA )
    {
      static __thread __itt_string_handle* pTaskName = nullptr;
      if ( nullptr == pTaskName )
      {
        pTaskName = __itt_string_handle_create("NativeKernelTask::Execute()");
      }
      __itt_task_begin(gMicGPAData.pDeviceDomain, __itt_null, __itt_null, pTaskName);
    }
#endif

#ifndef __OMP2TBB__
    cl_dev_err_code res = m_pKernel->Execute(m_pKernel, nullptr/* m_pTaskDispatcher->getOmpExecutionThread()*/);
#else
    cl_dev_err_code res = m_pKernel->Execute(m_pQueue->GetTaskList(), m_pKernelArgs);
#endif

#if defined(USE_ITT)
    if ( gMicGPAData.bUseGPA )
    {
#if defined(USE_GPA)
        __itt_set_track(nullptr);
#endif
        __itt_task_end(m_pIttKernelDomain); // BIKernel
    }
#endif // ITT


#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( gMicGPAData.bUseGPA )
    {
        __itt_task_end(m_pIttKernelDomain); // NativeKernelTask::Execute()
    }
#endif

    m_pQueue->FinishTask( this );

#ifdef USE_ITT
    if ( gMicGPAData.bUseGPA)
    {
        __itt_frame_end_v3(m_pIttKernelDomain, nullptr);
    }
#endif

    return true;
}

void NativeKernelTask::Cancel()
{
    m_pQueue->CancelTask(this);
#ifdef USE_ITT
    if ( gMicGPAData.bUseGPA )
    {
        __itt_task_end(m_pIttKernelDomain);
    }
#endif
}
