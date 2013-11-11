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
//  ExecutionTask.h
//  Implementation of the Class ExecutionTask
//  Class Object is responsible on execution of NDRange task
/////////////////////////////////////////////////////////////

#include "native_ndrange_task.h"
#include "native_program_service.h"
#include "native_thread_pool.h"
#include "wg_context.h"
#include "device_queue.h"
#include "thread_local_storage.h"
#include "native_common_macros.h"

#include "mic_logger.h"

// TODO: Move to system utils
#include "mic_native_logger.h"

#include <cl_dev_backend_api.h>
#include <cl_shared_ptr.hpp>

#include <sink/COIBuffer_sink.h>
#include <common/COIEvent_common.h>
#include <sink/COIPipeline_sink.h>

// TODO: remove from SVN
// #include "wg_context.h"

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
    assert(NULL != pQueue && "pQueue must be valid");

#ifdef _DEBUG
    TlsAccessor tlsAccessor;
    const QueueOnDevice* pTLSQueue = QueueOnDevice::getCurrentQueue( &tlsAccessor );
    assert( pTLSQueue == pQueue && "Queue handle doesn't match");
#endif


    NDRangeTask ndRangeTask(in_BufferCount, in_ppBufferPointers, ndrangeDispatchData, in_MiscDataLength);

    cl_dev_err_code err = ndRangeTask.getTaskError();
    if ( CL_DEV_SUCCEEDED(err) )
    {
        // Increase ref. count to prevent undesired deletion
        ndRangeTask.IncRefCnt();
        err = pQueue->Execute(ndRangeTask.GetAsTaskHandlerBase());
    }

    *((cl_dev_err_code*)in_pReturnValue) = err;
	MicInfoLog(MicNativeLogDescriptor::getLoggerClient(), MicNativeLogDescriptor::getClientId(), "%s", "[MIC SERVER] exit execute_NDRange");
}

NDRangeTask::NDRangeTask( uint32_t lockBufferCount, void** pLockBuffers, ndrange_dispatcher_data* pDispatcherData, size_t uiDispatchSize ) :
    TaskHandler<NDRangeTask, ndrange_dispatcher_data >(lockBufferCount, pLockBuffers, pDispatcherData, uiDispatchSize),
      m_kernel(NULL)
#ifndef __NEW_BE_API__
      ,m_pBinary(NULL), m_MemBuffCount(0), m_pMemBuffSizes(NULL)
#endif
      ,m_bSecureExecution(gMicExecEnvOptions.kernel_safe_mode)
#ifdef ENABLE_MIC_TRACER
    ,m_tbb_perf_data(*this)
#endif    
#ifdef USE_ITT
    ,m_pIttKernelName(NULL), m_pIttKernelDomain(NULL)
#endif
{
}

NDRangeTask::NDRangeTask( const NDRangeTask& o) :
    TaskHandler<NDRangeTask, ndrange_dispatcher_data >( o )
    , m_bSecureExecution(o.m_bSecureExecution)
#ifdef ENABLE_MIC_TRACER
    ,m_tbb_perf_data(*this)
#endif
#ifdef USE_ITT
    ,m_pIttKernelName(o.m_pIttKernelName), m_pIttKernelDomain(o.m_pIttKernelDomain)
#endif
{
}

NDRangeTask::~NDRangeTask()
{
#ifdef ENABLE_MIC_TRACER
    m_tbb_perf_data.PerfDataFini();
#endif    
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
        m_pIttKernelName = ProgramService::get_itt_kernel_name(m_dispatcherData->kernelAddress);
        m_pIttKernelDomain = ProgramService::get_itt_kernel_domain(m_dispatcherData->kernelAddress);
        // Use kernel specific domain if possible, if not available switch to global domain
        if ( NULL == m_pIttKernelDomain )
        {
            m_pIttKernelDomain = gMicGPAData.pDeviceDomain;
        }
    }
#endif

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( gMicGPAData.bUseGPA )
    {
      static __thread __itt_string_handle* pTaskName = NULL;
      if ( NULL == pTaskName )
      {
        pTaskName = __itt_string_handle_create("NDRangeTask::PrepareTask");
      }
      __itt_task_begin(gMicGPAData.pDeviceDomain, __itt_null, __itt_null, pTaskName);
    }
#endif

    cl_work_description_type tWorkDesc;

    m_dispatcherData->convertToClWorkDescriptionType(tWorkDesc);
    tWorkDesc.minWorkGroupNum = gMicExecEnvOptions.min_work_groups_number;

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( gMicGPAData.bUseGPA )
    {
      static __thread __itt_string_handle* pTaskName = NULL;
      if ( NULL == pTaskName )
      {
        pTaskName = __itt_string_handle_create("NDRangeTask::InitTask()->create_binary()");
      }
      __itt_task_begin(gMicGPAData.pDeviceDomain, __itt_null, __itt_null, pTaskName);
    }
#endif

    m_kernel = ProgramService::GetKernelPointer(m_dispatcherData->kernelAddress);
    // Now we should patch memory object pointers
    const cl_kernel_argument* pParamInfo = m_kernel->GetKernelParams();
    unsigned int        numMemArgs  = m_kernel->GetMemoryObjectArgumentCount();
    const unsigned int* pMemArgsInx = m_kernel->GetMemoryObjectArgumentIndexes();
    size_t              argSize     = m_kernel->GetArgumentBufferSize();

    char* kernelParams = ((char*)m_dispatcherData) + sizeof(ndrange_dispatcher_data);
    unsigned int currentLockedBuffer = 0;
    for(unsigned int i=0; i<numMemArgs; ++i)
    {
        void** loc = (void**)(kernelParams+pParamInfo[pMemArgsInx[i]].offset_in_bytes);
        // Skip NULL buffers
        if ( NULL == *loc )
            continue;
        *loc = m_bufferPointers[currentLockedBuffer];
        currentLockedBuffer++;
    }

    // Create the binary
    ProgramService& tProgramService = ProgramService::getInstance();
    cl_dev_err_code errCode = tProgramService.create_binary(m_kernel, kernelParams, argSize, &tWorkDesc, &m_pBinary);
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( gMicGPAData.bUseGPA )
    {
        __itt_task_end(gMicGPAData.pDeviceDomain);
    }
#endif
    if ( CL_DEV_FAILED(errCode) )
    {
        setTaskError(errCode);
        NATIVE_PRINTF("NDRangeTask::Init - ProgramService.create_binary() failed\n");
        return false;
    }

#ifndef __NEW_BE_API__
    // This code should be removed after moving to new BE API
    // This array will be calculated on the host
    const size_t* pWGSize = m_pBinary->GetWorkGroupSize();
    // Calculate the region of each dimension in the task.
    unsigned int i = 0;
    for (i = 0; i < m_dispatcherData->kernelArgs.WorkDim; ++i)
    {
        m_dispatcherData->kernelArgs.WGCount[i] = (uint64_t)((m_dispatcherData->kernelArgs.GlobalSize[i])/(uint64_t)(pWGSize[i]));
    }
#endif

#ifdef ENABLE_MIC_TRACER
    // Set kernel name for the tracer.
    commandTracer().set_kernel_name((char*)(m_kernel->GetKernelName()));
#endif
	
    // Update buffer parameters
    m_pBinary->GetMemoryBuffersDescriptions(NULL, &m_MemBuffCount);
    m_pMemBuffSizes = new size_t[m_MemBuffCount];
    if (NULL == m_pMemBuffSizes)
    {
        setTaskError( CL_DEV_OUT_OF_MEMORY );
        NATIVE_PRINTF("NDRangeTask::Init - Allocation of m_pMemBuffSizes failed\n");
        return false;
    }
    m_pBinary->GetMemoryBuffersDescriptions(m_pMemBuffSizes, &m_MemBuffCount);

#ifdef ENABLE_MIC_TRACER
    unsigned int i = 0;
    for (i = 0; i < m_dim; ++i)
    {
        // Set global work size in dimension "i" for the tracer.
        commandTracer().set_global_work_size(m_dispatcherData->UniformKernelArgs.GlobalSize[i], i);
        // Set WG size in dimension "i" for the tracer.
        commandTracer().set_work_group_size(m_dispatcherData->UniformKernelArgs.LocalSize[i], i);
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
        __itt_frame_begin_v3(m_pIttKernelDomain, NULL);
    }
#endif

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( gMicGPAData.bUseGPA )
    {
      static __thread __itt_string_handle* pTaskName = NULL;
      if ( NULL == pTaskName )
      {
        pTaskName = __itt_string_handle_create("NDRangeTask::Init()");
      }
      __itt_task_begin(gMicGPAData.pDeviceDomain, __itt_null, __itt_null, pTaskName);
    }
#endif

    // Notify start if exists
    if ( m_dispatcherData->startEvent.isRegistered )
    {
        COIEventSignalUserEvent(m_dispatcherData->startEvent.cmdEvent);
    }

    regCount = m_dispatcherData->kernelArgs.WorkDim;
    for (unsigned int i = 0; i < regCount; ++i)
    {
        region[i] = m_dispatcherData->kernelArgs.WGCount[i];
    }

#if 0
    printf("running on %d dims, global size: [0]=%d, [1]=%d, [2]=%d, number of workgrops:[0]=%d, [1]=%d, [2]=%d, local size: [0]=%d, [1]=%d, [2]=%d\n",
        (int)regCount,
        (int)m_dispatcherData->kernelArgs.GlobalSize[0], (int)m_dispatcherData->kernelArgs.GlobalSize[1], (int)m_dispatcherData->kernelArgs.GlobalSize[2],
        (int)m_dispatcherData->kernelArgs.WGCount[0], (int)m_dispatcherData->kernelArgs.WGCount[1], (int)m_dispatcherData->kernelArgs.WGCount[2],
        (int)m_dispatcherData->kernelArgs.LocalSize[0], (int)m_dispatcherData->kernelArgs.LocalSize[1], (int)m_dispatcherData->kernelArgs.LocalSize[2]
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
      static __thread __itt_string_handle* pTaskName = NULL;
      master_id = GetThreadId();
      if ( NULL == pTaskName )
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
    if ( gMicGPAData.bUseGPA)
    {
        __itt_task_begin(m_pIttKernelDomain, __itt_null, __itt_null, m_pIttKernelName);
    }
#endif
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( gMicGPAData.bUseGPA )
    {
        static __thread __itt_string_handle* pTaskName = NULL;
        if ( NULL == pTaskName )
        {
            pTaskName = __itt_string_handle_create("NDRangeTask::AttachToThread()");
        }
        __itt_task_begin(gMicGPAData.pDeviceDomain, __itt_null, __itt_null, pTaskName);
    }
#endif

    assert( NULL!=pWgContextBase && "At this point pWgContext must be valid");
    if ( NULL == pWgContextBase)
    {
        setTaskError( CL_DEV_INVALID_OPERATION );
        return NULL;
    }

    WGContext* pContext = reinterpret_cast<WGContext*>(pWgContextBase);
    
    // If can NOT recycle the current context - This is the case when my current context is not the context of the next execution
    if ( ((cl_dev_cmd_id)m_dispatcherData->commandIdentifier) != pContext->GetCmdId())
    {
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
        if ( gMicGPAData.bUseGPA )
        {
            static __thread __itt_string_handle* pTaskName = NULL;
            if ( NULL == pTaskName )
            {
                pTaskName = __itt_string_handle_create("NDRangeTask::AttachToThread->UpdateContext()");
            }
	        __itt_task_begin(gMicGPAData.pDeviceDomain, __itt_null, __itt_null, pTaskName);
        }
#endif

    // Update context with new binary.
    error = pContext->UpdateContext((cl_dev_cmd_id)(m_dispatcherData->commandIdentifier), m_pBinary, m_pMemBuffSizes, m_MemBuffCount, &m_printHandle);
    if (CL_DEV_FAILED(error))
    {
        pContext->InvalidateContext();
        setTaskError( error );
        return NULL;
    }
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
        // Monitor only IN-ORDER queue
        if ( gMicGPAData.bUseGPA )
        {
            __itt_task_end(gMicGPAData.pDeviceDomain); // "NDRangeTask::AttachToThread->UpdateContext()"
        }
#endif

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
        if ( gMicGPAData.bUseGPA )
        {
            static __thread __itt_string_handle* pTaskName = NULL;
            if ( NULL == pTaskName )
            {
                pTaskName = __itt_string_handle_create("NDRangeTask::AttachToThread()->PrepareThreadState()");
            }
            __itt_task_begin(gMicGPAData.pDeviceDomain, __itt_null, __itt_null, pTaskName);
        }
#endif

        // Prepare current thread context for execution
        error = pContext->GetExecutable()->PrepareThread();
        if (CL_DEV_FAILED(error))
        {
            setTaskError( error );
            return NULL;
        }
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
        // Monitor only IN-ORDER queue
        if ( gMicGPAData.bUseGPA )
        {
            __itt_task_end(gMicGPAData.pDeviceDomain); // "NDRangeTask::AttachToThread()->PrepareThreadState()"
        }
#endif
    }

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    // Monitor only IN-ORDER queue
    if ( gMicGPAData.bUseGPA )
    {
      __itt_task_end(gMicGPAData.pDeviceDomain); // "NDRangeTask::AttachToThread()"
    }
#endif

    return pContext;
}

// Is called when the task will not be executed by the specific thread
void NDRangeTask::DetachFromThread(void* pWgContext)
{
    cl_dev_err_code error = CL_DEV_SUCCESS;

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( gMicGPAData.bUseGPA )
    {
        static __thread __itt_string_handle* pTaskName = NULL;
        if ( NULL == pTaskName )
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

    assert( NULL!=pWgContext && "At this point pWgContext must be valid");
    if ( NULL == pWgContext)
    {
        setTaskError( CL_DEV_INVALID_OPERATION );
      return false;
    }

    WGContext*     pContext       = reinterpret_cast<WGContext*>(pWgContext);
    ICLDevBackendExecutable_* pExec = pContext->GetExecutable();
    assert( NULL!=pExec && "At this point pExec must be valid");

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( gMicGPAData.bUseGPA )
    {
      static __thread __itt_string_handle* pTaskName = NULL;
      if ( NULL == pTaskName )
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
#ifdef __NEW_BE_API__
        m_kernel->Execute( &m_dispatcherData->kernelArgs, groupId );
#else
        error = pExec->Execute( groupId, NULL, NULL );
#endif
    } else
    {
        error = HWExceptionWrapper::Execute(m_kernel, pExec, &m_dispatcherData->kernelArgs, groupId);
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
    // Release COI resources, before signaling to runtime
    FiniTask();

#ifdef ENABLE_MIC_TRACER
    commandTracer().set_current_time_tbb_exe_in_device_time_end();
#endif

    // Notify end if exists
    if ( m_dispatcherData->endEvent.isRegistered )
    {
        COIEventSignalUserEvent(m_dispatcherData->endEvent.cmdEvent);
    }

#ifdef USE_ITT
    if ( gMicGPAData.bUseGPA)
    {
        __itt_task_end(m_pIttKernelDomain);
        __itt_frame_end_v3(m_pIttKernelDomain, NULL);
    }
#endif

    m_pBinary->Release();

    return CL_DEV_SUCCEEDED( getTaskError() );
}

void NDRangeTask::Cancel()
{
    // TODO: What if task already started
  // Notify end if exists
  if ( m_dispatcherData->startEvent.isRegistered )
  {
      COIEventSignalUserEvent(m_dispatcherData->startEvent.cmdEvent);
  }

#ifdef ENABLE_MIC_TRACER
    commandTracer().set_current_time_tbb_exe_in_device_time_start();
    commandTracer().set_current_time_tbb_exe_in_device_time_end();
#endif
    setTaskError( CL_DEV_COMMAND_CANCELLED );

    // Notify end if exists
    if (m_dispatcherData->endEvent.isRegistered )
    {
        COIEventSignalUserEvent(m_dispatcherData->endEvent.cmdEvent);
    }

#ifdef USE_ITT
    if ( gMicGPAData.bUseGPA)
    {
        __itt_task_end(m_pIttKernelDomain);
    }
#endif
}
