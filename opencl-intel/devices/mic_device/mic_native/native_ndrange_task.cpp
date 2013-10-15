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
#include "native_common_macros.h"
#include "native_thread_pool.h"
#include "wg_context.h"
#include "hw_exceptions_handler.h"
#include "thread_local_storage.h"
#include "cl_shared_ptr.hpp"
#include "mic_logger.h"
#include "mic_native_logger.h"

#include <sink/COIBuffer_sink.h>
#include <common/COIEvent_common.h>
#include <sink/COIPipeline_sink.h>

using namespace Intel::OpenCL::MICDeviceNative;

// Execute NDRange task.
COINATIVELIBEXPORT
void execute_NDRange(uint32_t         in_BufferCount,
					 void**           in_ppBufferPointers,
					 uint64_t*        in_pBufferLengths,
					 void*            in_pMiscData,
					 uint16_t         in_MiscDataLength,
					 void*            in_pReturnValue,
					 uint16_t         in_ReturnValueLength)
{
	MicInfoLog(MicNativeLogDescriptor::getLoggerClient(), MicNativeLogDescriptor::getClientId(), "%s", "[MIC SERVER] enter execute_NDRange");
	QueueOnDevice::execute_command(in_BufferCount, in_ppBufferPointers, in_pBufferLengths, 
                                   in_pMiscData, in_MiscDataLength, 
                                   in_pReturnValue, in_ReturnValueLength, 
                                   NDRANGE_TASK_TYPE);
	MicInfoLog(MicNativeLogDescriptor::getLoggerClient(), MicNativeLogDescriptor::getClientId(), "%s", "[MIC SERVER] exit execute_NDRange");
}

NDRangeTask::NDRangeTask( const QueueOnDevice& queue ) : TaskHandler( queue ),
    m_commandIdentifier((cl_dev_cmd_id)-1), m_kernel(NULL), m_pBinary(NULL),
    m_MemBuffCount(0), m_pMemBuffSizes(NULL), m_dim(0), m_lockedParams(NULL)
#ifdef ENABLE_MIC_TRACER
    ,m_tbb_perf_data(*this)
#endif    
#ifdef USE_ITT
    ,m_pIttKernelName(NULL), m_pIttKernelDomain(NULL)
#endif
{
}

NDRangeTask::~NDRangeTask()
{
	if (m_pMemBuffSizes)
	{
		delete [] m_pMemBuffSizes;
	}
#ifdef ENABLE_MIC_TRACER
    m_tbb_perf_data.PerfDataFini();
#endif    
}

// called immediately after creation and after filling the COI-passed data
bool NDRangeTask::InitTask()
{
	// Set total buffers size and num of buffers for the tracer.
#ifdef ENABLE_MIC_TRACER
	commandTracer().add_delta_num_of_buffer_sent_to_device(m_lockBufferCount);
	unsigned long long bufSize = 0;
	for (unsigned int i = 0; i < m_lockBufferCount; i++)
	{
		bufSize = m_lockBufferLengths[i];
		commandTracer().add_delta_buffers_size_sent_to_device(bufSize);
	}
#endif

	ndrange_dispatcher_data* pDispatcherData = (ndrange_dispatcher_data*)(m_dispatcherData);
	assert(NULL!=pDispatcherData && "Dispatcher data is NULL");
	if ( NULL == pDispatcherData )
	{
	  return false;
	}

#ifndef NDRANGE_UNIT_TEST
	// Get kernel object
	bool result = ProgramService::get_kernel(pDispatcherData->kernelDirective.kernelAddress, (const ICLDevBackendKernel_**)&m_kernel);
	if (false == result)
	{
		setTaskError( CL_DEV_INVALID_KERNEL );
		NATIVE_PRINTF("NDRangeTask::Init - ProgramService::getInstance().get_kernel failed\n");
		return false;
	}
#endif

#ifdef USE_ITT
  if ( gMicGPAData.bUseGPA)
  {

	    m_pIttKernelName = ProgramService::get_itt_kernel_name(pDispatcherData->kernelDirective.kernelAddress);
	    m_pIttKernelDomain = ProgramService::get_itt_kernel_domain(pDispatcherData->kernelDirective.kernelAddress);
	    // Use kernel specific domain if possible, if not available switch to global domain
	    if ( NULL == m_pIttKernelDomain )
	    {
	        m_pIttKernelDomain = gMicGPAData.pDeviceDomain;
	    }

	    __itt_frame_begin_v3(m_pIttKernelDomain, NULL);
  }
#endif

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( gMicGPAData.bUseGPA )
    {
      static __thread __itt_string_handle* pTaskName = NULL;
      if ( NULL == pTaskName )
      {
        pTaskName = __itt_string_handle_create("NDRangeTask::InitTask");
      }
      __itt_task_begin(gMicGPAData.pDeviceDomain, __itt_null, __itt_null, pTaskName);
    }
#endif

	// Set command identifier
	m_commandIdentifier = pDispatcherData->commandIdentifier;

	// Set kernel arguments blob (Still have to set the buffers pointer in the blob)
	if (pDispatcherData->kernelArgSize > 0)
	{
		m_lockedParams = (char*)((char*)pDispatcherData + pDispatcherData->kernelArgBlobOffset);
	}

	unsigned int numOfPreExeDirectives = pDispatcherData->preExeDirectivesCount;
	// If there are pre executable directives to execute
	if (numOfPreExeDirectives > 0)
	{
		// get the pointer to preExeDirectivesArr
		directive_pack* preExeDirectivesArr = (directive_pack*)((char*)pDispatcherData + pDispatcherData->preExeDirectivesArrOffset);
		// traverse over the preExeDirectivesArr
		for (unsigned int i = 0; i < numOfPreExeDirectives; i++)
		{
			switch ( preExeDirectivesArr[i].id )
			{
			case BUFFER:
				{
					assert(preExeDirectivesArr[i].bufferDirective.bufferIndex < m_lockBufferCount);
					// A pointer to memory data
					void* memObj = m_lockBufferPointers[preExeDirectivesArr[i].bufferDirective.bufferIndex];
					// A pointer to mem object descriptor.
					cl_mem_obj_descriptor* pMemObjDesc = &(preExeDirectivesArr[i].bufferDirective.mem_obj_desc);
					pMemObjDesc->pData = memObj;
					// Copy the address of the memory object to the kernel args blob
					assert(preExeDirectivesArr[i].bufferDirective.offset_in_blob < pDispatcherData->kernelArgSize);
					void** pTempLockedParams = (void**)(m_lockedParams + preExeDirectivesArr[i].bufferDirective.offset_in_blob);
					*pTempLockedParams = (void*)pMemObjDesc;
					break;
				}
			case PRINTF:
			case BARRIER:
				{
					break;
				}
			default:
				{
					assert(0);
					break;
				}
			}
		}
	}

	cl_work_description_type tWorkDesc;
	pDispatcherData->workDesc.convertToClWorkDescriptionType(&tWorkDesc);
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
	ProgramService& tProgramService = ProgramService::getInstance();
	// Create the binary.
	cl_dev_err_code errCode = tProgramService.create_binary(m_kernel, m_lockedParams, pDispatcherData->kernelArgSize, &tWorkDesc, &m_pBinary);
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

	const size_t* pWGSize = m_pBinary->GetWorkGroupSize();
	cl_mic_work_description_type* pWorkDesc = &(pDispatcherData->workDesc);
	m_dim = pWorkDesc->workDimension;
	assert((m_dim >= 1) && (m_dim <= MAX_WORK_DIM));
	// Calculate the region of each dimension in the task.
	unsigned int i = 0;
	for (i = 0; i < m_dim; ++i)
	{
		m_region[i] = (uint64_t)((pWorkDesc->globalWorkSize[i])/(uint64_t)(pWGSize[i]));
#ifdef ENABLE_MIC_TRACER
		// Set global work size in dimension "i" for the tracer.
		commandTracer().set_global_work_size(pWorkDesc->globalWorkSize[i], i);
		// Set WG size in dimension "i" for the tracer.
		commandTracer().set_work_group_size(pWGSize[i], i);
#endif		
	}
	for (; i < MAX_WORK_DIM; ++i)
	{
		m_region[i] = 1;
#ifdef ENABLE_MIC_TRACER
		// Set global work size in dimension "i" for the tracer.
		commandTracer().set_global_work_size(0, i);
		// Set WG size in dimension "i" for the tracer.
		commandTracer().set_work_group_size(0, i);
#endif		
	}

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( gMicGPAData.bUseGPA )
    {
      __itt_task_end(gMicGPAData.pDeviceDomain);
    }
#endif

	return true;
}

// must be called at the very end of the ITaskBase finish stage and 
// must call to QueueOnDevice->FinishTask() at the very end of itself
void NDRangeTask::FinishTask()
{
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( gMicGPAData.bUseGPA )
    {
      static __thread __itt_string_handle* pTaskName = NULL;
      if ( NULL == pTaskName )
      {
        pTaskName = __itt_string_handle_create("NDRangeTask::FinishTask");
      }
      __itt_task_begin(gMicGPAData.pDeviceDomain, __itt_null, __itt_null, pTaskName);
    }
#endif

	// Release the binary.
	if (NULL != m_pBinary)
	{
		m_pBinary->Release();
	}

	ndrange_dispatcher_data* pDispatcherData = (ndrange_dispatcher_data*)(m_dispatcherData);
	assert(pDispatcherData);
	COIEVENT completionBarrier;
	bool findBarrier = false;
	// Perform post execution directives.
	unsigned int numOfPostExeDirectives = pDispatcherData->postExeDirectivesCount;
	if (numOfPostExeDirectives > 0)
	{
		// get the pointer to postExeDirectivesArr
		directive_pack* postExeDirectivesArr = (directive_pack*)((char*)pDispatcherData + pDispatcherData->postExeDirectivesArrOffset);
		// traverse over the postExeDirectivesArr
		for (unsigned int i = 0; i < numOfPostExeDirectives; i++)
		{
			switch ( postExeDirectivesArr[i].id )
			{
			case BARRIER:
				{
					findBarrier = true;
					// set the signal user barrier (Will signal on destruction)
					completionBarrier = postExeDirectivesArr[i].barrierDirective.barrier;
					break;
				}
			case PRINTF:
				{
					// TODO
					break;
				}
			default:
				{
					assert(0);
					break;
				}
			}
		}
	}

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( gMicGPAData.bUseGPA )
    {
      __itt_task_end(gMicGPAData.pDeviceDomain);
    }
#endif

#ifdef USE_ITT
  if ( gMicGPAData.bUseGPA)
  {
      __itt_frame_end_v3(m_pIttKernelDomain, NULL);
  }
#endif

	// Last command, Do NOT call any method of this object after it perform.
  // Evgeny: Why it passed as a smart pointer
	queue().FinishTask(this, completionBarrier, findBarrier);
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
        m_commandTracer->add_delta_time_thread_overall_time(delta, m_cpuId);
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
    regCount = m_dim;
    for (unsigned int i = 0; i < m_dim; ++i)
    {
        region[i] = m_region[i];
    }

    queue().SignalTaskStart( this );

#ifdef ENABLE_MIC_TRACER
    commandTracer().set_current_time_tbb_exe_in_device_time_start();
#endif

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    // Monitor only IN-ORDER queue
    if ( gMicGPAData.bUseGPA )
    {
      __itt_task_end(gMicGPAData.pDeviceDomain);
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
    cl_dev_err_code error;


#ifdef ENABLE_MIC_TRACER
    TaskLoopBodyTrace::loop_start(commandTracer(), uiNumberOfWorkGroups);
    m_tbb_perf_data.work_group_start();
#endif

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL) // End of "TBB::Distribute_Work"
    // Monitor only IN-ORDER queue
    if ( gMicGPAData.bUseGPA && (master_id==GetThreadId()) )
    {
        __itt_task_end(gMicGPAData.pDeviceDomain);
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
	if (m_commandIdentifier != pContext->GetCmdId())
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
		error = pContext->UpdateContext(m_commandIdentifier, m_kernel, m_pBinary, m_pMemBuffSizes, m_MemBuffCount, &m_printHandle);
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
        // Do it only for the first occurrence
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

	if ( gMicExecEnvOptions.kernel_safe_mode )
	{
        TlsAccessor tls;
        pContext->jitExecWapper().thread_init( &tls );
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
  cl_dev_err_code error;

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

    if ( gMicExecEnvOptions.kernel_safe_mode )
	{

    	assert( NULL!=pWgContext && "At this point pWgContext must be valid");
    	if ( NULL == pWgContext)
    	{
	    	setTaskError( CL_DEV_INVALID_OPERATION );
            return;
	    }
        WGContext* pContext = reinterpret_cast<WGContext*>(pWgContext);
        TlsAccessor tls;
        pContext->jitExecWapper().thread_fini( &tls );
    }		

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

	// Execute WG
	size_t groupId[MAX_WORK_DIM] = {x, y, z};
	cl_dev_err_code error;
	if ( !gMicExecEnvOptions.kernel_safe_mode )
	{
		error = pExec->Execute( groupId, NULL, NULL );
	} else
	{
	    error = pContext->jitExecWapper().Execute(pExec, groupId, NULL, NULL);
	}

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    // Monitor only IN-ORDER queue
    if ( gMicGPAData.bUseGPA )
    {
      __itt_task_end(gMicGPAData.pDeviceDomain);
    }
#endif

	if (CL_DEV_FAILED(error))
	{
		setTaskError( error );
		return false;
	}

    return true;
}

// Final stage, free execution resources
// Return false when command execution fails
bool NDRangeTask::Finish(FINISH_REASON reason)
{
#ifdef ENABLE_MIC_TRACER
    commandTracer().set_current_time_tbb_exe_in_device_time_end();
#endif
#ifdef USE_ITT
    if ( gMicGPAData.bUseGPA)
    {
        __itt_task_end(m_pIttKernelDomain);
    }
#endif

    FinishTask();
    return CL_DEV_SUCCEEDED( getTaskError() );
}

void NDRangeTask::Cancel()
{
    queue().SignalTaskStart( this );
#ifdef ENABLE_MIC_TRACER
    commandTracer().set_current_time_tbb_exe_in_device_time_start();
    commandTracer().set_current_time_tbb_exe_in_device_time_end();
#endif
    setTaskError( CL_DEV_COMMAND_CANCELLED );
#ifdef USE_ITT
    if ( gMicGPAData.bUseGPA)
    {
		__itt_task_end(m_pIttKernelDomain);
    }
#endif
    FinishTask();
}
