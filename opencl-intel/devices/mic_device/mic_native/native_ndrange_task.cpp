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
//  native_ndrange_task.cpp
/////////////////////////////////////////////////////////////

#include "native_ndrange_task.h"
#include "native_program_service.h"
#include "native_thread_pool.h"
#include "device_queue.h"
#include "thread_local_storage.h"
#include "native_common_macros.h"
#include "tbb_memory_allocator.h"
#include "hw_exceptions_handler.h"
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
void execute_command_ndrange(uint32_t        in_BufferCount,
                            void**           in_ppBufferPointers,
                            uint64_t*        in_pBufferLengths,
                            void*            in_pMiscData,
                            uint16_t         in_MiscDataLength,
                            void*            in_pReturnValue,
                            uint16_t         in_ReturnValueLength)
{
    MicInfoLog(MicNativeLogDescriptor::getLoggerClient(), MicNativeLogDescriptor::getClientId(), "%s", "[MIC SERVER] enter execute_NDRange");
    assert( in_ReturnValueLength == 0 && "We should not return value for command");
    assert( in_MiscDataLength >= sizeof(ndrange_dispatcher_data) && "Size of input MiscData doesn't match");

    ndrange_dispatcher_data* ndrangeDispatchData = (ndrange_dispatcher_data*)in_pMiscData;

    QueueOnDevice* pQueue = (QueueOnDevice*)(ndrangeDispatchData->deviceQueuePtr);
    assert(nullptr != pQueue && "pQueue must be valid");

	cl_dev_err_code err = CL_DEV_SUCCESS;

    if (pQueue->IsAsyncExecution())
    {
        SharedPtr<NDRangeTask> ndRangeTask = NDRangeTask::Allocate(in_BufferCount, in_ppBufferPointers, ndrangeDispatchData, in_MiscDataLength, pQueue);

        err = ndRangeTask->getTaskError();
        if ( CL_DEV_SUCCEEDED(err) )
        {
            err = pQueue->Execute(ndRangeTask->GetAsTaskHandlerBase());
        }
    }
    else
    {
        // In case of immediate queue, can allocate the task on the stack!
        NDRangeTask ndRangeTask(in_BufferCount, in_ppBufferPointers, ndrangeDispatchData, in_MiscDataLength, pQueue);

        err = ndRangeTask.getTaskError();
        if ( CL_DEV_SUCCEEDED(err) )
        {
	        // Increase ref. count to prevent undesired deletion
		    ndRangeTask.IncRefCnt();
            err = pQueue->Execute(ndRangeTask.GetAsTaskHandlerBase());
        }
    }

    *((cl_dev_err_code*)in_pReturnValue) = err;
    MicInfoLog(MicNativeLogDescriptor::getLoggerClient(), MicNativeLogDescriptor::getClientId(), "%s", "[MIC SERVER] exit execute_NDRange");
}

NDRangeTask::NDRangeTask( uint32_t lockBufferCount, void** pLockBuffers, ndrange_dispatcher_data* pDispatcherData, size_t uiDispatchSize, QueueOnDevice* pQueue ) :
    TaskHandler<NDRangeTask, ndrange_dispatcher_data >(lockBufferCount, pLockBuffers, pDispatcherData, uiDispatchSize, pQueue),
      m_pKernel(nullptr), m_pRunner(nullptr), m_pKernelArgs(nullptr), m_pUniformArgs(nullptr),
      m_flushAtExit(false), m_bSecureExecution(gMicExecEnvOptions.kernel_safe_mode)
#ifdef ENABLE_MIC_TRACER
    ,m_tbb_perf_data(*this)
#endif    
#ifdef USE_ITT
    ,m_pIttKernelName(nullptr), m_pIttKernelDomain(nullptr)
#endif
{
}

__thread ICLDevBackendKernelRunner::ICLDevExecutionState NDRangeTask::m_tExecutionState;

NDRangeTask::~NDRangeTask()
{
#ifdef ENABLE_MIC_TRACER
    m_tbb_perf_data.PerfDataFini();
#endif

    if ( m_pKernelArgs != (((char*)m_pDispatcherData) + sizeof(ndrange_dispatcher_data)) )
    {
        Intel::OpenCL::TaskExecutor::ScalableMemAllocator::scalableAlignedFree(m_pKernelArgs);
        m_pKernelArgs = nullptr;
    }
}

// called immediately after creation and after filling the COI-passed data
bool NDRangeTask::PrepareTask()
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

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( gMicGPAData.bUseGPA )
    {
      static __thread __itt_string_handle* pTaskName = nullptr;
      if ( nullptr == pTaskName )
      {
        pTaskName = __itt_string_handle_create("NDRangeTask::PrepareTask()");
      }
      __itt_task_begin(gMicGPAData.pDeviceDomain, __itt_null, __itt_null, pTaskName);
    }
#endif

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( gMicGPAData.bUseGPA )
    {
      static __thread __itt_string_handle* pTaskName = nullptr;
      if ( nullptr == pTaskName )
      {
        pTaskName = __itt_string_handle_create("NDRangeTask::PrepareTask()::PatchKernelArguments");
      }
      __itt_task_begin(gMicGPAData.pDeviceDomain, __itt_null, __itt_null, pTaskName);
    }
#endif

    m_pKernel = ProgramService::GetKernelPointer(m_pDispatcherData->kernelAddress);
    m_pRunner = m_pKernel->GetKernelRunner();

    // Now we should patch memory object pointers
    const cl_kernel_argument* pParamInfo = m_pKernel->GetKernelParams();
    unsigned int        numMemArgs  = m_pKernel->GetMemoryObjectArgumentCount();
    const unsigned int* pMemArgsInx = m_pKernel->GetMemoryObjectArgumentIndexes();
    size_t              argSize     = m_pKernel->GetExplicitArgumentBufferSize();

    m_pKernelArgs = ((char*)m_pDispatcherData) + sizeof(ndrange_dispatcher_data);
    // Check if alignment of kernel arguments is enough
    size_t argAlignment = m_pKernel->GetArgumentBufferRequiredAlignment();
    if ( 0 != ((size_t)m_pKernelArgs & (argAlignment-1)) )
    {
        // Need to allocate properly aligned arguments
        size_t allocSize = argSize + sizeof(cl_uniform_kernel_args);
        void* pNewKernelArgs = Intel::OpenCL::TaskExecutor::ScalableMemAllocator::scalableAlignedMalloc(allocSize, argAlignment);
        if ( nullptr== pNewKernelArgs )
        {
            assert ( 0 && "Failed to allocate aligned kernel arguments buffer");
            return false;
        }
        memcpy(pNewKernelArgs, m_pKernelArgs, allocSize);
        m_pKernelArgs = (char*)pNewKernelArgs;
    }

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

    m_pUniformArgs = reinterpret_cast<cl_uniform_kernel_args*>(m_pKernelArgs + argSize);
    m_pUniformArgs->RuntimeInterface = (void*)static_cast<ICLDevBackendDeviceAgentCallback*>(this);

#if 0
    printf("m_pUniformArgs->RuntimeInterface=%p\n", this);
    printf("Queue:%p, Kernel:%p(%p), SE:%d, EE:%d\n"\
        (void*)m_pDispatcherData->deviceQueuePtr, (void*)m_pDispatcherData->kernelAddress, m_pKernel, (int)m_pDispatcherData->startEvent.isRegistered, (int)m_pDispatcherData->endEvent.isRegistered,
        "\trunning on %d dims\n"
        "\tglobal size: [0]=%d, [1]=%d, [2]=%d\n"
        "\tuniform local size:     [0]=%d, [1]=%d, [2]=%d\n"
        "\tnon-uniform local size: [0]=%d, [1]=%d, [2]=%d\n"
        "\tRuntimeInterface=%p\n",
        (int)m_pUniformArgs->WorkDim,
        (int)m_pUniformArgs->GlobalSize[0], (int)m_pUniformArgs->GlobalSize[1], (int)m_pUniformArgs->GlobalSize[2],
        (int)m_pUniformArgs->LocalSize[UNIFORM_WG_SIZE_INDEX][0], (int)m_pUniformArgs->LocalSize[UNIFORM_WG_SIZE_INDEX][1], (int)m_pUniformArgs->LocalSize[UNIFORM_WG_SIZE_INDEX][2],
        (int)m_pUniformArgs->LocalSize[NONUNIFORM_WG_SIZE_INDEX][0], (int)m_pUniformArgs->LocalSize[NONUNIFORM_WG_SIZE_INDEX][1], (int)m_pUniformArgs->LocalSize[NONUNIFORM_WG_SIZE_INDEX][2],
        (void*)m_pUniformArgs->RuntimeInterface
        );fflush(0);
#endif

    /// Call to BE to initialize JIT pointer
    cl_dev_err_code errCode = m_pRunner->InitRunner(m_pKernelArgs);
    if ( CL_DEV_FAILED(errCode) )
    {
        setTaskError(errCode);
        NATIVE_PRINTF("NDRangeTask::Init - InitRunner() failed\n");
        return false;
    }

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( gMicGPAData.bUseGPA )
    {
        __itt_task_end(gMicGPAData.pDeviceDomain);
    }
#endif

#ifdef ENABLE_MIC_TRACER
    // Set kernel name for the tracer.
    commandTracer().set_kernel_name((char*)(m_kernel->GetKernelName()));
#endif

#ifdef ENABLE_MIC_TRACER
    unsigned int i = 0;
    for (i = 0; i < m_dim; ++i)
    {
        // Set global work size in dimension "i" for the tracer.
        commandTracer().set_global_work_size(m_pDispatcherData->UniformKernelArgs.GlobalSize[i], i);
        // Set WG size in dimension "i" for the tracer.
        commandTracer().set_work_group_size(m_pDispatcherData->UniformKernelArgs.LocalSize[i], i);
    }
    for (; i < MAX_WORK_DIM; ++i)
    {
        // Set global work size in dimension "i" for the tracer.
        commandTracer().set_global_work_size(0, i);
        // Set WG size in dimension "i" for the tracer.
        commandTracer().set_work_group_size(0, i);
    }
#endif

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( gMicGPAData.bUseGPA )
    {
        __itt_task_end(gMicGPAData.pDeviceDomain);
    }
#endif

	m_bufferPointers = nullptr;

    return true;
}

#ifdef ENABLE_MIC_TRACER
struct TaskLoopBodyTrace {
public:
    static void loop_start( CommandTracer& cmdTracer, unsigned long long numOfWorkGroups )
    {
        thread_data().init( cmdTracer, numOfWorkGroups );
    }

    static void loop_end()
    {
        thread_data().finish();
    }
    
private:
    void finish()
    {
        unsigned long long end = CommandTracer::_RDTSC();
        unsigned long long delta = end - m_start;
    }

    void init(CommandTracer& cmdTracer, unsigned long long numOfWorkGroups)
    {
        
        m_commandTracer = &cmdTracer;
        m_start = CommandTracer::_RDTSC();
        m_cpuId = hw_cpu_idx();
        m_commandTracer->increment_thread_num_of_invocations(m_cpuId);
        m_commandTracer->add_delta_thread_num_wg_exe(numOfWorkGroups, m_cpuId);
    }

    static TaskLoopBodyTrace& thread_data() 
    {
        unsigned int uiWorkerId = Intel::OpenCL::MICDeviceNative::ThreadPool::getInstance()->getWorkerID();
        if (Intel::OpenCL::MICDeviceNative::ThreadPool::INVALID_WORKER_ID == uiWorkerId)
        {
            uiWorkerId = 0;
        }
        return gTaskLoopTracer[uiWorkerId];
    }

    CommandTracer*      m_commandTracer;
    unsigned int        m_cpuId;
    unsigned long long  m_start;

    
    static TaskLoopBodyTrace gTaskLoopTracer[ MIC_NATIVE_MAX_WORKER_THREADS ];
};

TaskLoopBodyTrace TaskLoopBodyTrace::gTaskLoopTracer[ MIC_NATIVE_MAX_WORKER_THREADS ];

#endif // ENABLE_MIC_TRACER

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
static __thread unsigned int master_id = 0;
#endif

// Initialization function. This functions is called before the "main loop"
// Generally initializes internal data structures
// Fills the buffer with 3D number of iterations to run
// Fills regCount with actual number of regions
// Returns 0 if initialization success, otherwise an error code
int NDRangeTask::Init(size_t region[], unsigned int& regCount)
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
        pTaskName = __itt_string_handle_create("NDRangeTask::Init()");
      }
      __itt_task_begin(gMicGPAData.pDeviceDomain, __itt_null, __itt_null, pTaskName);
    }
#endif

    // Notify start if exists
    if ( m_pDispatcherData->startEvent.isRegistered )
    {
        COIEventSignalUserEvent(m_pDispatcherData->startEvent.cmdEvent);
    }

    regCount = m_pUniformArgs->WorkDim;
    size_t i;
    for(i=0;i<m_pUniformArgs->WorkDim;++i)
    {
        region[i] = m_pUniformArgs->WGCount[i];
    }
    for(;i<MAX_WORK_DIM;++i)
    {
        region[i] = 1;
    }

#if 0
    printf("running on %d dims\n"
           "\tglobal size: [0]=%d, [1]=%d, [2]=%d\n"
           "\tnumber of workgrops:[0]=%d, [1]=%d, [2]=%d\n"
           "\tuniform local size: [0]=%d, [1]=%d, [2]=%d\n"
           "\tnon-uniform local size: [0]=%d, [1]=%d, [2]=%d\n",
        (int)m_pUniformArgs->GlobalSize[0], (int)m_pUniformArgs->GlobalSize[1], (int)m_pUniformArgs->GlobalSize[2],
        (int)m_pUniformArgs->WGCount[0], (int)m_pUniformArgs->WGCount[1], (int)m_pUniformArgs->WGCount[2],
        (int)m_pUniformArgs->LocalSize[UNIFORM_WG_SIZE_INDEX][0],
        (int)m_pUniformArgs->LocalSize[UNIFORM_WG_SIZE_INDEX][1],
        (int)m_pUniformArgs->LocalSize[UNIFORM_WG_SIZE_INDEX][2],
        (int)m_pUniformArgs->LocalSize[NON_UNIFORM_WG_SIZE_INDEX][0],
        (int)m_pUniformArgs->LocalSize[NON_UNIFORM_WG_SIZE_INDEX][1],
        (int)m_pUniformArgs->LocalSize[NON_UNIFORM_WG_SIZE_INDEX][2]
        );fflush(0);
#endif

#ifdef ENABLE_MIC_TRACER
    commandTracer().set_current_time_tbb_exe_in_device_time_start();
#endif

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( gMicGPAData.bUseGPA )
    {
         __itt_task_end(gMicGPAData.pDeviceDomain); //"NDRangeTask::Init()"
    }
#endif

#ifdef USE_ITT
    if ( gMicGPAData.bUseGPA)
    {
        __itt_task_begin(m_pIttKernelDomain, __itt_null, __itt_null, m_pIttKernelName);
    }
#endif

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( gMicGPAData.bUseGPA )
    {
      static __thread __itt_string_handle* pTaskName = nullptr;
      master_id = GetThreadId();
      if ( nullptr == pTaskName )
      {
        pTaskName = __itt_string_handle_create("TBB::Distribute_Work");
      }
      __itt_task_begin(gMicGPAData.pDeviceDomain, __itt_null, __itt_null, pTaskName);
    }
#endif

    return 0;
}

// Returns void* to be passed to other, if attach process succeeded, otherwise NULL
void* NDRangeTask::AttachToThread(void* pWgContextBase, size_t uiNumberOfWorkGroups, size_t firstWGID[], size_t lastWGID[])
{
    cl_dev_err_code error = CL_DEV_SUCCESS;

#ifdef ENABLE_MIC_TRACER
    TaskLoopBodyTrace::loop_start(commandTracer(), uiNumberOfWorkGroups);
    m_tbb_perf_data.work_group_start();
#endif

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    // Monitor only IN-ORDER queue
    if ( gMicGPAData.bUseGPA && (master_id==GetThreadId()) )
    {
        __itt_task_end(gMicGPAData.pDeviceDomain); // End of "TBB::Distribute_Work"
        // notify only once
        master_id = 0;
    }
#endif

#ifdef USE_ITT
    if ( gMicGPAData.bUseGPA )
    {
        __itt_task_begin(m_pIttKernelDomain, __itt_null, __itt_null, m_pIttKernelName);
    }
#endif
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( gMicGPAData.bUseGPA )
    {
        static __thread __itt_string_handle* pTaskName = nullptr;
        if ( nullptr == pTaskName )
        {
            pTaskName = __itt_string_handle_create("NDRangeTask::AttachToThread()");
        }
        __itt_task_begin(gMicGPAData.pDeviceDomain, __itt_null, __itt_null, pTaskName);
    }
#endif

    // Prepare current thread context for execution
    error = m_pRunner->PrepareThreadState(m_tExecutionState);
    if (CL_DEV_FAILED(error))
    {
        setTaskError( error );
        return nullptr;
    }

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    // Monitor only IN-ORDER queue
    if ( gMicGPAData.bUseGPA )
    {
      __itt_task_end(gMicGPAData.pDeviceDomain); // "NDRangeTask::AttachToThread()"
    }
#endif

    return pWgContextBase;
}

// Is called when the task will not be executed by the specific thread
void NDRangeTask::DetachFromThread(void* pWgContext)
{
    cl_dev_err_code error = CL_DEV_SUCCESS;

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( gMicGPAData.bUseGPA )
    {
        static __thread __itt_string_handle* pTaskName = nullptr;
        if ( nullptr == pTaskName )
        {
            pTaskName = __itt_string_handle_create("NDRangeTask::DettachFromThread()");
        }
        __itt_task_begin(gMicGPAData.pDeviceDomain, __itt_null, __itt_null, pTaskName);
    }
#endif

#ifdef ENABLE_MIC_TRACER
    m_tbb_perf_data.work_group_end();
    TaskLoopBodyTrace::loop_end();
#endif

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    // Monitor only IN-ORDER queue
    if ( gMicGPAData.bUseGPA )
    {
        __itt_task_end(gMicGPAData.pDeviceDomain); // "NDRangeTask::DettachFromThread()"
    }
#endif

#ifdef USE_ITT
    if ( gMicGPAData.bUseGPA)
    {
        __itt_task_end(m_pIttKernelDomain);
    }
#endif
}

// "Main loop"
// The function is called with different 'inx' parameters for each iteration number
bool NDRangeTask::ExecuteIteration(size_t x, size_t y, size_t z, void* pWgContext )
{
    cl_dev_err_code error = CL_DEV_SUCCESS;

#ifdef ENABLE_MIC_TRACER
    m_tbb_perf_data.append_data_item(m_dim, (unsigned int)x, (unsigned int)y, (unsigned int)z );
#endif    

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( gMicGPAData.bUseGPA )
    {
      static __thread __itt_string_handle* pTaskName = nullptr;
      if ( nullptr == pTaskName )
      {
        pTaskName = __itt_string_handle_create("NDRangeTask::ExecuteIteration->jitExecWapper().Execute()");
      }
      __itt_task_begin(gMicGPAData.pDeviceDomain, __itt_null, __itt_null, pTaskName);
    }
#endif

    size_t groupId[MAX_WORK_DIM] = {x, y, z};
    // Execute WG

    if ( !m_bSecureExecution )
    {
        m_pRunner->RunGroup(m_pKernelArgs, groupId, this );
    } else
    {
        error = HWExceptionWrapper::Execute(m_pRunner, m_pKernelArgs, groupId, this);
    }

    if (CL_DEV_FAILED(error))
    {
        setTaskError( error );
        return false;
    }

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    // Monitor only IN-ORDER queue
    if ( gMicGPAData.bUseGPA )
    {
      __itt_task_end(gMicGPAData.pDeviceDomain);
    }
#endif

    return true;
}

// Final stage, free execution resources
// Return false when command execution fails
bool NDRangeTask::Finish(FINISH_REASON reason)
{
    if ( m_flushAtExit )
    {
        fflush(stdout);
        COIRESULT coiErr = COIProcessProxyFlush();
        assert(COI_SUCCESS == coiErr && "Failed to Flush() printf buffer to host");
    }

    m_pQueue->FinishTask( this );

#ifdef USE_ITT
    if ( gMicGPAData.bUseGPA)
    {
        __itt_task_end(m_pIttKernelDomain);
        __itt_frame_end_v3(m_pIttKernelDomain, nullptr);
    }
#endif
    return CL_DEV_SUCCEEDED( getTaskError() );
}

void NDRangeTask::Cancel()
{
    m_pQueue->CancelTask(this);
#ifdef USE_ITT
    if ( gMicGPAData.bUseGPA)
    {
        __itt_task_end(m_pIttKernelDomain);
    }
#endif
}

int NDRangeTask::Print(const char* pBuffer, void* pHandle)
{
    assert( (void*)this == pHandle && "Invalid NDRange handle was provided to the callback");
    m_flushAtExit = true;
    return fputs(pBuffer, stdout);
}
