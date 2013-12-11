#include "execution_task.h"
#include "native_program_service.h"
#include "native_common_macros.h"
#include "mic_tracer.h"
#include "native_buffer_commands.h"
#include "native_globals.h"

#include <sink/COIBuffer_sink.h>
#include <common/COIEvent_common.h>

#include <sink/COIPipeline_sink.h>
#include <sink/COIProcess_sink.h>

#include <malloc.h>
#include <cstring>
#include <map>
#include <fstream>
#include <assert.h>


#include <stdlib.h>
#include <omp.h>
//#define OMP_SCHED "auto"
//#define OMP_SCHED "dynamic"
//#define OMP_SCHED "static"
#define OMP_SCHED "guided"

//#define KMP_AFFINITY "verbose,granularity=fine,scatter"
#define KMP_AFFINITY "granularity=fine,scatter"
//#define KMP_AFFINITY "granularity=fine,compact"


using namespace Intel::OpenCL::MICDeviceNative;

Intel::OpenCL::MICDevice::mic_exec_env_options Intel::OpenCL::MICDeviceNative::gMicExecEnvOptions;


///////////////////////////////////////////////////////
// For testing only
unsigned int commandNum = 0;

void foo(char* blob)
{
	char* tempBlob = blob;
	unsigned int size = *(unsigned int*)tempBlob;
	tempBlob += sizeof(unsigned int);
	unsigned int* a = *((unsigned int**)tempBlob);
	tempBlob += sizeof(unsigned int*);
	unsigned int* b = *((unsigned int**)tempBlob);
	tempBlob += sizeof(unsigned int*);
	unsigned int* res = *((unsigned int**)tempBlob);
	tempBlob += sizeof(unsigned int*);
	unsigned int testFlag = *(unsigned int*)tempBlob;
	if (55555 != testFlag)
	{
		NATIVE_PRINTF("ERROR\n");
		fflush(0);
		assert(0);
	}
	for (unsigned int i = 0; i < size; i++)
	{
		res[i] = a[i] + b[i] + commandNum;
	}
	commandNum ++;
}
// For testing only
//////////////////////////////////////////////////////

volatile unsigned int resume_server_execution = 0;

// Array of WGContext for each worker thread. When task completes, traversing over this array and calling "Relase()" method for each object that is not NULL.
static WGContext* g_contextArray = NULL;


// Initialize the device thread pool. Call it immediately after process creation.
COINATIVELIBEXPORT
void init_device(uint32_t         in_BufferCount,
			     void**           in_ppBufferPointers,
				 uint64_t*        in_pBufferLengths,
				 void*            in_pMiscData,
				 uint16_t         in_MiscDataLength,
				 void*            in_pReturnValue,
				 uint16_t         in_ReturnValueLength)
{
	assert(in_MiscDataLength == sizeof(mic_exec_env_options));
	assert(in_ReturnValueLength == sizeof(cl_dev_err_code));

	cl_dev_err_code* pErr = (cl_dev_err_code*)in_pReturnValue;
	*pErr = CL_DEV_SUCCESS;
	
	// The mic_exec_env_options input.
	mic_exec_env_options* tEnvOptions = (mic_exec_env_options*)in_pMiscData;
	assert(tEnvOptions);
	if (tEnvOptions->stop_at_load)
	{
		printf("********* DEVICE STOPPED PLEASE ATTACH TO PID = %d ************\n", getpid());
		fflush(stdout);
		while (resume_server_execution == 0) {};
	}
	gMicExecEnvOptions = *tEnvOptions;
	assert((gMicExecEnvOptions.num_of_worker_threads > 0) && (gMicExecEnvOptions.num_of_worker_threads < MIC_NATIVE_MAX_WORKER_THREADS));

	*pErr = ProgramService::createProgramService();
	if ( CL_DEV_FAILED(*pErr) )
	{
		return;
	}

	g_contextArray = new WGContext[MIC_NATIVE_MAX_WORKER_THREADS];
	if ( NULL == g_contextArray )
	{
		ProgramService::releaseProgramService();
		*pErr = CL_DEV_OUT_OF_MEMORY;
		return;
	}

	// Create thread pool singleton instance.
	ThreadPool* pThreadPool = ThreadPool::getInstance();
	if (NULL == pThreadPool)
	{
		delete g_contextArray;
		ProgramService::releaseProgramService();
		*pErr = CL_DEV_OUT_OF_MEMORY;
		return;
	}
	// Initialize the thread pool with "numOfWorkers" workers.
	if (false == pThreadPool->init())
	{
		delete g_contextArray;
		ProgramService::releaseProgramService();
		ThreadPool::releaseSingletonInstance();
		*pErr = CL_DEV_ERROR_FAIL;
		return;
	}
}

// release the device thread pool. Call it before process destruction.
COINATIVELIBEXPORT
void release_device(uint32_t         in_BufferCount,
					 void**           in_ppBufferPointers,
					 uint64_t*        in_pBufferLengths,
					 void*            in_pMiscData,
					 uint16_t         in_MiscDataLength,
					 void*            in_pReturnValue,
					 uint16_t         in_ReturnValueLength)
{
	if ( NULL != g_contextArray )
	{
		delete []g_contextArray;
	}
	// Release the extra arena allocated on init_device() by pThreadPool->init()
	ThreadPool::getInstance()->unregisterMasterThread();
	// Release the thread pool singleton.
	ThreadPool::releaseSingletonInstance();
	ProgramService::releaseProgramService();
}

// Initialize current pipeline command queue. Call it after Pipeline creation of Command list.
COINATIVELIBEXPORT
void init_commands_queue(uint32_t         in_BufferCount,
				         void**           in_ppBufferPointers,
				         uint64_t*        in_pBufferLengths,
				         void*            in_pMiscData,
				         uint16_t         in_MiscDataLength,
				         void*            in_pReturnValue,
				         uint16_t         in_ReturnValueLength)
{
    assert( sizeof(INIT_QUEUE_ON_DEVICE_STRUCT) == in_MiscDataLength );
    assert( NULL != in_pMiscData );

	INIT_QUEUE_ON_DEVICE_STRUCT* data = (INIT_QUEUE_ON_DEVICE_STRUCT*)in_pMiscData;

    ThreadPool* pool = ThreadPool::getInstance();
	pool->registerMasterThread( data->is_in_order_queue );

    assert( NULL == QueueOnDevice::getCurrentQueue() && "Queue is already set");
    if ( NULL != QueueOnDevice::getCurrentQueue() )
    {
    	
    	*((cl_dev_err_code*)in_pReturnValue) = CL_DEV_INVALID_OPERATION;
    	return;
    }

    QueueOnDevice* pQueue = new QueueOnDevice( data->is_in_order_queue );
    if ( NULL == pQueue )
    {
    	*((cl_dev_err_code*)in_pReturnValue) = CL_DEV_OUT_OF_MEMORY;
    	return;
    }

    // This is TBB master thread. Need to allocate local WG context and store it TLS.
	WGContext* pCtx = new WGContext();
	assert(NULL!=pCtx && "Failed to allocate execution context (pCtx)");	
    if ( NULL == pCtx )
    {
    	delete pQueue;
    	*((cl_dev_err_code*)in_pReturnValue) = CL_DEV_OUT_OF_MEMORY;
    	return;
    }

    // Store local WG context
    TlsAccessor tlsAccessor;
	NDrangeTls ndRangeTls(&tlsAccessor);
	ndRangeTls.setTls(NDrangeTls::WG_CONTEXT, pCtx);

    QueueOnDevice::setCurrentQueue( pQueue );
    pool->wakeup_all();
    
}

// release current pipeline command queue. Call it before Pipeline destruction of Command list.
COINATIVELIBEXPORT
void release_commands_queue(uint32_t         in_BufferCount,
					        void**           in_ppBufferPointers,
					        uint64_t*        in_pBufferLengths,
					        void*            in_pMiscData,
					        uint16_t         in_MiscDataLength,
					        void*            in_pReturnValue,
					        uint16_t         in_ReturnValueLength)
{
	// For master thread we should access it's TLS to retrieve the WGContext
	TlsAccessor tlsAccessor;
	NDrangeTls ndRangeTls(&tlsAccessor);
	// Get the WGContext instance of this thread
	WGContext* pCtx = (WGContext*)ndRangeTls.getTls(NDrangeTls::WG_CONTEXT);
    assert(NULL != pCtx && "pCtx must be valid");
    if ( NULL != pCtx )
    {
		delete pCtx;
	}

	ThreadPool* pool = ThreadPool::getInstance();

    QueueOnDevice* pQueue = QueueOnDevice::getCurrentQueue();
    QueueOnDevice::setCurrentQueue( NULL );
    assert(NULL != pQueue && "pQueue must be valid");
    if ( NULL != pQueue )
    {
    	delete pQueue;
    }

    pool->unregisterMasterThread();

}


// Execute device utility function
COINATIVELIBEXPORT
void execute_device_utility( uint32_t         in_BufferCount,
        					 void**           in_ppBufferPointers,
        					 uint64_t*        in_pBufferLengths,
        					 void*            in_pMiscData,
        					 uint16_t         in_MiscDataLength,
        					 void*            in_pReturnValue,
        					 uint16_t         in_ReturnValueLength)
{
    //
    // Execute some management function on device as part of some queue
    //
    assert( ((NULL != in_pMiscData) && (sizeof(utility_function_options) == in_MiscDataLength)) 
                                            && "Wrong params to execute_device_utility" );

    if ((NULL == in_pMiscData) || (sizeof(utility_function_options) != in_MiscDataLength))
    {
        return;
    }

    utility_function_options* options = (utility_function_options*)in_pMiscData;

    switch (options->request)
    {
        case UTILITY_MEASURE_OVERHEAD:
            break;

        default:;
    }
}

void execute_command(uint32_t					in_BufferCount,
					 void**						in_ppBufferPointers,
					 uint64_t*					in_pBufferLengths,
					 void*						in_pMiscData,
					 uint16_t					in_MiscDataLength,
					 void*						in_pReturnValue,
					 uint16_t					in_ReturnValueLength,
					 TaskHandler::TASK_TYPES	taskType)
{
	dispatcher_data* tDispatcherData = NULL;
	misc_data* tMiscData = NULL;
	// the buffer index of misc_data in case that it is not in "in_pReturnValue"
	uint32_t tMiscDataBufferIndex = in_BufferCount - DISPATCHER_DATA - 1;
	// If the dispatcher_data is not in in_pMiscData than it suppose to be at in_ppBufferPointers[in_BufferCount - DISPATCHER_DATA - 1] (The last buffer)
	if (0 == in_MiscDataLength)
	{
		assert(in_BufferCount > 0);
		assert(in_pBufferLengths[in_BufferCount - 1] >= sizeof(dispatcher_data) && "in_pBufferLengths[in_BufferCount - 1] should be at least as the size of dispatcher_data");		
		tDispatcherData = (dispatcher_data*)in_ppBufferPointers[in_BufferCount - DISPATCHER_DATA - 1];
		tMiscDataBufferIndex --;
	}
	else
	{
		assert(in_MiscDataLength >= sizeof(dispatcher_data));
		tDispatcherData = (dispatcher_data*)in_pMiscData;
	}
	// If the misc_data is NOT in in_pReturnValue
	if (0 == in_ReturnValueLength)
	{
		// The misc_data is in in_ppBufferPointers[in_BufferCount - 1] in case of dispatcher_data is in in_pMiscData and in in_ppBufferPointers[in_BufferCount - 2] in case of dispatcher_data is in in_ppBufferPointers[in_BufferCount - DISPATCHER_DATA -1].
		assert(in_BufferCount > tMiscDataBufferIndex);
		assert(in_pBufferLengths[tMiscDataBufferIndex] == sizeof(misc_data) && "in_pBufferLengths[tMiscDataBufferIndex] should be as the size of misc_data");
		tMiscData = (misc_data*)in_ppBufferPointers[tMiscDataBufferIndex];
	}
	else
	{
		assert(in_ReturnValueLength == sizeof(misc_data));
		tMiscData = (misc_data*)in_pReturnValue;
	}
	
	// Set init value of misc_data.
	tMiscData->init();	

	// DO NOT delete this object, It will delete itself after kernel execution
	TaskHandler* taskHandler = TaskHandler::TaskFactory(taskType, QueueOnDevice::getCurrentQueue(), tDispatcherData, tMiscData);
	if (NULL == taskHandler)
	{
		NATIVE_PRINTF("TaskHandler::TaskFactory() Failed\n");
		return;
	}
	// Initialize the task brefore sending for execution.
	taskHandler->InitTask(tDispatcherData, tMiscData, in_BufferCount, in_ppBufferPointers, in_pBufferLengths, in_pMiscData, in_MiscDataLength);
	if (CL_DEV_FAILED(tMiscData->errCode))
	{
		NATIVE_PRINTF("TaskHandler::init() Failed\n");
		return;
	}
	// Send the task for execution.
	taskHandler->RunTask();
}

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
	execute_command(in_BufferCount, in_ppBufferPointers, in_pBufferLengths, in_pMiscData, in_MiscDataLength, in_pReturnValue, in_ReturnValueLength, TaskHandler::NDRANGE_TASK_TYPE);
}



TaskHandler::TaskHandler() : m_dispatcherData(NULL), m_miscData(NULL), m_lockBufferCount(0), m_lockBufferPointers(NULL), m_lockBufferLengths(NULL), m_task(NULL)
{
}

TaskHandler::~TaskHandler()
{
	// Set leaving time to device for the tracer
	m_commandTracer.set_current_time_cmd_run_in_device_time_end();
}


TaskHandler* TaskHandler::TaskFactory(TASK_TYPES taskType, QueueOnDevice* queue,
                                      dispatcher_data* dispatcherData, misc_data* miscData)
{
	TaskContainerInterface* pTaskContainer = NULL;

    assert( (NULL == queue) || (queue->isInOrder() == dispatcherData->isInOrderQueue) );
    
	// If In order
	if (dispatcherData->isInOrderQueue)
	{
		switch (taskType)
		{
		case NDRANGE_TASK_TYPE:
			{
			pTaskContainer = new BlockingNDRangeTask;
			break;
			}
		case FILL_MEM_OBJ_TYPE:
			{
			pTaskContainer = new BlockingFillMemObjTask;
			break;
			}
		default:
			{
				assert(0);
			}
		}
	}
	// out of order
	else
	{
		switch (taskType)
		{
		case NDRANGE_TASK_TYPE:
			{
			pTaskContainer = new NonBlockingNDRangeTask;
			break;
			}
		case FILL_MEM_OBJ_TYPE:
			{
			pTaskContainer = new NonBlockingFillMemObjTask;
			break;
			}
		default:
			{
				assert(0);
			}
		}
	}
	// if task creation failed.
	if (NULL == pTaskContainer)
	{
		miscData->errCode = CL_DEV_OUT_OF_MEMORY;
		// Have to signal the COIEVENT for completion in case of NonBlockingTask (In other failures cases We will call the finish() method in order to do it - Currently We can't do it because there is no object).
		if (false == dispatcherData->isInOrderQueue)
		{
			unsigned int numOfPostExeDirectives = dispatcherData->postExeDirectivesCount;
			assert(numOfPostExeDirectives > 0);
			// get teh pointer to postExeDirectivesArr
			directive_pack* postExeDirectivesArr = (directive_pack*)((char*)dispatcherData + dispatcherData->postExeDirectivesArrOffset);
			// traverse over the postExeDirectivesArr
			for (unsigned int i = 0; i < numOfPostExeDirectives; i++)
			{
				// We like to find only the BARRIER directive in this case.
				if (BARRIER == postExeDirectivesArr[i].id)
				{
					// Signal user completion barrier
					COIRESULT coiErr = COIEventSignalUserEvent(postExeDirectivesArr[i].barrierDirective.barrier);
					assert(COI_SUCCESS == coiErr);
					break;
				}
			}
		}
		return NULL;
	}
	// Get "TaskHandler" representation from "TaskContainerInterface"
	TaskHandler* taskHandler = pTaskContainer->getMyTaskHandler();
	assert(taskHandler);
	assert(pTaskContainer->getMyTask());
	// Set "TaskInterface" in this "TaskHandler" object.
	taskHandler->setTaskInterface(pTaskContainer->getMyTask());
    taskHandler->setQueue(queue);

	// Set arrival time to device for the tracer
	taskHandler->m_commandTracer.set_current_time_cmd_run_in_device_time_start();
	// Set command ID for the tracer
	taskHandler->m_commandTracer.set_command_id((size_t)(dispatcherData->commandIdentifier));

	return taskHandler;
}

void TaskHandler::setTaskError(cl_dev_err_code errorCode)
{
	// If m_miscData defined and currently m_miscData->errCode in success mode than set it
	if ((CL_DEV_SUCCESS != errorCode) && (m_miscData) && (CL_DEV_SUCCESS == m_miscData->errCode))
	{
		m_miscData->errCode = errorCode;
	}
}

void BlockingTaskHandler::InitTask(dispatcher_data* dispatcherData, misc_data* miscData, uint32_t in_BufferCount, void** in_ppBufferPointers, uint64_t* in_pBufferLengths, void* in_pMiscData, uint16_t in_MiscDataLength)
{
	m_dispatcherData = dispatcherData;
	m_miscData = miscData;
	// Locking of input buffers is not needed in case of blocking task (Only save the pointer in order to use it later).
	m_lockBufferCount = in_BufferCount;
	m_lockBufferPointers = in_ppBufferPointers;
	m_lockBufferLengths = in_pBufferLengths;

	setTaskError( m_task->init(this) );
}

void BlockingTaskHandler::RunTask()
{
	m_task->run();
}

void BlockingTaskHandler::FinishTask(COIEVENT& completionBarrier, bool isLegalBarrier)
{
	assert(false == isLegalBarrier);
	// Delete this object as the last operation on it.
	delete this;
}

void NonBlockingTaskHandler::InitTask(dispatcher_data* dispatcherData, misc_data* miscData, uint32_t in_BufferCount, void** in_ppBufferPointers, uint64_t* in_pBufferLengths, void* in_pMiscData, uint16_t in_MiscDataLength)
{
	m_dispatcherData = dispatcherData;
	m_miscData = miscData;
	m_lockBufferCount = in_BufferCount;
	// If the client sent buffers, than We should copy their pointers and lock them. (In case of OOO)
	if (in_BufferCount > 0)
	{
		m_lockBufferPointers = new void*[in_BufferCount];
		if (NULL == m_lockBufferPointers)
		{
			setTaskError( CL_DEV_OUT_OF_MEMORY );
			m_task->finish(this);
			return;
		}
		m_lockBufferLengths = new uint64_t[in_BufferCount];
		if (NULL == m_lockBufferLengths)
		{
			setTaskError( CL_DEV_OUT_OF_MEMORY );
			m_task->finish(this);
			return;
		}
		COIRESULT result = COI_SUCCESS;
		// In case of non blocking task, shall lock all input buffers.
		for (unsigned int i = 0; i < in_BufferCount; i++)
		{
			// add ref in order to save the buffer on the device
			result = COIBufferAddRef(in_ppBufferPointers[i]);
			if (result != COI_SUCCESS)
			{
				setTaskError( CL_DEV_ERROR_FAIL );
				m_task->finish(this);
				return;
			}
			m_lockBufferPointers[i] = in_ppBufferPointers[i];
			m_lockBufferLengths[i] = in_pBufferLengths[i];
		}
	}

	// In case of Non blocking task when the dispatcher data was sent by "in_pMiscData" - We have to allocate memory for it and copy its content.
	// (misc_data will always transfer as COIBuffer in case of NonBlocking command)
	if (m_dispatcherData == in_pMiscData)
	{
		m_dispatcherData = NULL;
		m_dispatcherData = (dispatcher_data*)(new char[in_MiscDataLength]);
		if (NULL == m_dispatcherData)
		{
			setTaskError( CL_DEV_OUT_OF_MEMORY );
			m_task->finish(this);
			return;
		}
		memcpy(m_dispatcherData, in_pMiscData, in_MiscDataLength);
	}

	setTaskError( m_task->init(this) );
}


void NonBlockingTaskHandler::FinishTask(COIEVENT& completionBarrier, bool isLegalBarrier)
{
	// For asynch task We must have legal COIEVENT to signal.
	assert(isLegalBarrier);
	COIRESULT coiErr = COI_SUCCESS;
	// Release resources.
	if (m_lockBufferPointers)
	{
		// If Non blocking task and the dispatcher_data was delivered by in_pMiscData shall delete the allocation in "lockInputBuffers()"
		if ((m_dispatcherData) && (m_lockBufferCount > 0) && (m_dispatcherData != m_lockBufferPointers[m_lockBufferCount - DISPATCHER_DATA - 1]))
		{
			delete [] ((char*)m_dispatcherData);
		}
		for (unsigned int i = 0; i < m_lockBufferCount; i++)
		{
			// decrement ref in order to release the buffer
			coiErr = COIBufferReleaseRef(m_lockBufferPointers[i]);
			assert(COI_SUCCESS == coiErr);
		}
		delete [] m_lockBufferPointers;
	}
	if (m_lockBufferLengths)
	{
		delete [] m_lockBufferLengths;
	}

	// Copying the completion barrier to local barrier because have to delete "this" first in order to ensure printf flush before signaling.
	COIEVENT localCompletionBarrier = completionBarrier;

	// Delete this object as the last operation on it.
	delete this;

	// Signal user completion barrier
	coiErr = COIEventSignalUserEvent(localCompletionBarrier);
	assert(COI_SUCCESS == coiErr);
}


void NonBlockingTaskHandler::StartTaskSignaling()
{
	bool findBarrier = false;
	// find start barrier in pre exexution directives.
	unsigned int numOfPreExeDirectives = m_dispatcherData->preExeDirectivesCount;
	if (numOfPreExeDirectives > 0)
	{
		// get teh pointer to postExeDirectivesArr
		directive_pack* preExeDirectivesArr = (directive_pack*)((char*)m_dispatcherData + m_dispatcherData->preExeDirectivesArrOffset);
		// traverse over the postExeDirectivesArr
		for (unsigned int i = 0; i < numOfPreExeDirectives; i++)
		{
			if (BARRIER == preExeDirectivesArr[i].id)
			{
				// Signal user start barrier
				COIRESULT coiErr = COIEventSignalUserEvent(preExeDirectivesArr[i].barrierDirective.barrier);
				assert(COI_SUCCESS == coiErr);
				findBarrier = true;
				break;
			}
		}
	}
	assert(findBarrier);
}



void TBBNonBlockingTaskHandler::RunTask()
{
	TBBTaskInterface* pTbbTaskInter = dynamic_cast<TBBTaskInterface*>(m_task);
	assert(pTbbTaskInter);
	if (NULL == pTbbTaskInter)
	{
		setTaskError( CL_DEV_ERROR_FAIL );
		return m_task->finish(this);
	}
	// Enqueue the task to tbb task queue, will execute it asynchronous,
	tbb::task::enqueue(*(pTbbTaskInter->getTaskExecutorObj()));
}

NDRangeTask::NDRangeTask() : m_commandIdentifier((cl_dev_cmd_id)-1), m_kernel(NULL), m_pBinary(NULL), m_progamExecutableMemoryManager(NULL),
m_MemBuffCount(0), m_pMemBuffSizes(NULL), m_dim(0), m_lockedParams(NULL), m_pCommandTracer(NULL)
{
#ifdef ENABLE_MIC_TBB_TRACER
    PerfDataInit();
#endif // ENABLE_MIC_TBB_TRACER
}

NDRangeTask::~NDRangeTask()
{
	if (m_pMemBuffSizes)
	{
		delete [] m_pMemBuffSizes;
	}

#ifdef ENABLE_MIC_TBB_TRACER
    PerfDataFini((unsigned int)(size_t)m_commandIdentifier, (unsigned int)m_dim, m_region[0], m_region[1], m_region[2]);
#endif // ENABLE_MIC_TBB_TRACER
}

cl_dev_err_code NDRangeTask::init(TaskHandler* pTaskHandler)
{
	assert(pTaskHandler);

	m_pCommandTracer = &(pTaskHandler->m_commandTracer);
	// Set total buffers size and num of buffers for the tracer.
	m_pCommandTracer->add_delta_num_of_buffer_sent_to_device((pTaskHandler->m_lockBufferCount));

    m_pQueue = pTaskHandler->m_queue;
    
	unsigned long long bufSize = 0;
	for (unsigned int i = 0; i < pTaskHandler->m_lockBufferCount; i++)
	{
		bufSize = pTaskHandler->m_lockBufferLengths[i];
		m_pCommandTracer->add_delta_buffers_size_sent_to_device(bufSize);
	}

	ndrange_dispatcher_data* pDispatcherData = (ndrange_dispatcher_data*)(pTaskHandler->m_dispatcherData);
	assert(pDispatcherData);
	ProgramService& tProgramService = ProgramService::getInstance();
#ifndef NDRANGE_UNIT_TEST
	// Get kernel object
	bool result = tProgramService.get_kernel(pDispatcherData->kernelDirective.kernelAddress, (const ICLDevBackendKernel_**)&m_kernel, &m_progamExecutableMemoryManager);
	if (false == result)
	{
		pTaskHandler->setTaskError( CL_DEV_INVALID_KERNEL );
		finish(pTaskHandler);
		NATIVE_PRINTF("NDRangeTask::Init - ProgramService::getInstance().get_kernel failed\n");
		return CL_DEV_INVALID_KERNEL;
	}
#endif
	// Set command identifier
	m_commandIdentifier = pDispatcherData->commandIdentifier;

	// Set kernel args blob (Still have to set the buffers pointer in the blob)
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
					assert(preExeDirectivesArr[i].bufferDirective.bufferIndex < pTaskHandler->m_lockBufferCount);
					// A pointer to memory data
					void* memObj = pTaskHandler->m_lockBufferPointers[preExeDirectivesArr[i].bufferDirective.bufferIndex];
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

	// Create the binary.
	cl_dev_err_code errCode = tProgramService.create_binary(m_kernel, m_lockedParams, pDispatcherData->kernelArgSize, &tWorkDesc, &m_pBinary);
    if ( CL_DEV_FAILED(errCode) )
	{
		pTaskHandler->setTaskError(errCode);
		finish(pTaskHandler);
		NATIVE_PRINTF("NDRangeTask::Init - ProgramService.create_binary() failed\n");
		return errCode;
	}

	// Set kernel name for the tracer.
	m_pCommandTracer->set_kernel_name((char*)(m_kernel->GetKernelName()));

	// Update buffer parameters
    m_pBinary->GetMemoryBuffersDescriptions(NULL, &m_MemBuffCount);
	m_pMemBuffSizes = new size_t[m_MemBuffCount];
	if (NULL == m_pMemBuffSizes)
	{
		pTaskHandler->setTaskError( CL_DEV_OUT_OF_MEMORY );
		finish(pTaskHandler);
		NATIVE_PRINTF("NDRangeTask::Init - Allocation of m_pMemBuffSizes failed\n");
		return CL_DEV_OUT_OF_MEMORY;
	}
    m_pBinary->GetMemoryBuffersDescriptions(m_pMemBuffSizes, &m_MemBuffCount);

	const size_t* pWGSize = m_pBinary->GetWorkGroupSize();
	cl_mic_work_description_type* pWorkDesc = &(pDispatcherData->workDesc);
	m_dim = pWorkDesc->workDimension;
	assert((m_dim >= 1) && (m_dim <= MAX_WORK_DIM));
	// Calculate the region of each dimention in the task.
	unsigned int i = 0;
	for (i = 0; i < m_dim; ++i)
	{
		m_region[i] = (uint64_t)((pWorkDesc->globalWorkSize[i])/(uint64_t)(pWGSize[i]));
		
		// Set global work size in dimension "i" for the tracer.
		m_pCommandTracer->set_global_work_size(pWorkDesc->globalWorkSize[i], i);
		// Set WG size in dimension "i" for the tracer.
		m_pCommandTracer->set_work_group_size(pWGSize[i], i);
	}
	for (; i < MAX_WORK_DIM; ++i)
	{
		m_region[i] = 1;

		// Set global work size in dimension "i" for the tracer.
		m_pCommandTracer->set_global_work_size(0, i);
		// Set WG size in dimension "i" for the tracer.
		m_pCommandTracer->set_work_group_size(0, i);
	}
	return CL_DEV_SUCCESS;
}

void NDRangeTask::finish(TaskHandler* pTaskHandler)
{
	// Release the binary.
	if (NULL != m_pBinary)
	{
		m_pBinary->Release();
	}

	ndrange_dispatcher_data* pDispatcherData = (ndrange_dispatcher_data*)(pTaskHandler->m_dispatcherData);
	assert(pDispatcherData);
	COIEVENT completionBarrier;
	bool findBarrier = false;
	// Perform post exexution directives.
	unsigned int numOfPostExeDirectives = pDispatcherData->postExeDirectivesCount;
	if (numOfPostExeDirectives > 0)
	{
		// get teh pointer to postExeDirectivesArr
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
	// Last command, Do NOT call any method of this object after it perform.
	pTaskHandler->FinishTask(completionBarrier, findBarrier);
}


cl_dev_err_code NDRangeTask::attachToThread(TlsAccessor* tlsAccessor, size_t uiWorkerId)
{
	WGContext* pCtx = NULL;
	if ( 0 == uiWorkerId )
	{
		// For master thread we should access it's TLS to retrieve the WGContext
		NDrangeTls ndRangeTls(tlsAccessor);
		// Get the WGContext instance of this thread
		pCtx = (WGContext*)ndRangeTls.getTls(NDrangeTls::WG_CONTEXT);
	}
	else
	{
		// For workers we should access the global array
		pCtx = &g_contextArray[uiWorkerId-1];
	}
	
	assert( NULL!=pCtx && "At this point pCtx must be valid");
	if ( NULL == pCtx)
	{
		return CL_DEV_INVALID_OPERATION;
	}

	// If can NOT recycle the current context - This is the case when my current context is not the context of the next execution
	if (m_commandIdentifier != pCtx->GetCmdId())
	{
		// Update context with new binary.
		cl_dev_err_code ret = pCtx->UpdateContext(m_commandIdentifier, m_pBinary, m_pMemBuffSizes, m_MemBuffCount, &m_printHandle);
		if (CL_DEV_FAILED(ret))
		{
			pCtx->InvalidateContext();
			return ret;
		}
	}
	
	// Prepare current thread context for execution
	return pCtx->GetExecutable()->PrepareThread();
}

cl_dev_err_code	NDRangeTask::detachFromThread(TlsAccessor* tlsAccessor, size_t uiWorkerId)
{
	WGContext* pCtx = NULL;
	if ( 0 == uiWorkerId )
	{
		// For master thread we should access it's TLS to retrieve the WGContext
		NDrangeTls ndRangeTls(tlsAccessor);
		// Get the WGContext instance of this thread
		pCtx = (WGContext*)ndRangeTls.getTls(NDrangeTls::WG_CONTEXT);
	}
	else
	{
		// For workers we should access the global array
		pCtx = &g_contextArray[uiWorkerId-1];
	}
	
	assert( NULL!=pCtx && "At this point pCtx must be valid");
	if ( NULL == pCtx)
	{
		return CL_DEV_INVALID_OPERATION;
	}

	// Restore execution state
	cl_dev_err_code ret = pCtx->GetExecutable()->RestoreThreadState();
   
	return ret;
}

cl_dev_err_code NDRangeTask::executeIteration(TlsAccessor* tlsAccessor, HWExceptionsJitWrapper& hw_wrapper, size_t x, size_t y, size_t z, size_t uiWorkerId)
{
	WGContext* pCtx = NULL;
	if ( 0 == uiWorkerId )
	{
		// For master thread we should access it's TLS to retrieve the WGContext
		NDrangeTls ndRangeTls(tlsAccessor);
		// Get the WGContext instance of this thread
		pCtx = (WGContext*)ndRangeTls.getTls(NDrangeTls::WG_CONTEXT);
	}
	else
	{
		// For workers we should access the global array
		pCtx = &g_contextArray[uiWorkerId-1];
	}
	
	assert( NULL!=pCtx && "At this point pCtx must be valid");
	if ( NULL == pCtx)
	{
		return CL_DEV_INVALID_OPERATION;
	}

    ICLDevBackendExecutable_* pExec = pCtx->GetExecutable();
	assert( NULL!=pExec && "At this point pExec must be valid");
	
	// Execute WG
	size_t groupId[MAX_WORK_DIM] = {x, y, z};
	return hw_wrapper.Execute(pExec, groupId, NULL, NULL);
}



TBBNDRangeTask::TBBNDRangeTask() : NDRangeTask(), TBBTaskInterface(), m_pTaskExecutor(NULL)
{
}

cl_dev_err_code TBBNDRangeTask::init(TaskHandler* pTaskHandler)
{
	// Call my parent (NDRangeTask init() method)
	cl_dev_err_code result = NDRangeTask::init(pTaskHandler);
	if (CL_DEV_FAILED(result))
	{
		m_pTaskExecutor = NULL;
		return result;
	}

	// According to TBB documentation "Always allocate memory for task objects using special overloaded new operators (11.3.2) provided by the library, otherwise the results are undefined.
	// Destruction of a task is normally implicit. (When "execute()" method completes)
	m_pTaskExecutor = new (tbb::task::allocate_root()) TBBNDRangeExecutor(this, pTaskHandler, m_dim, m_region);
	assert(m_pTaskExecutor);
	if (NULL == m_pTaskExecutor)
	{
		pTaskHandler->setTaskError( CL_DEV_OUT_OF_MEMORY );
		finish(pTaskHandler);
		return CL_DEV_OUT_OF_MEMORY;
	}

	return CL_DEV_SUCCESS;
}

void TBBNDRangeTask::run()
{
	// Call explicitly to 'tbb::task::execute()'
	m_pTaskExecutor->execute();
	// In case of calling 'execute()' explicitly We should destroy the tbb::task explicitly also.
	tbb::task::destroy(*m_pTaskExecutor);
	m_pTaskExecutor = NULL;
}


ThreadPool*     ThreadPool::m_singleThreadPool = NULL;
OclMutexNative  ThreadPool::m_workers_initialization_lock;
volatile bool   ThreadPool::m_workers_initialized = false;

ThreadPool::ThreadPool() : m_numOfWorkers(0), m_NextWorkerID(0), m_nextAffinitiesThreadIndex(0)
{
	pthread_mutex_init(&m_reserveHwThreadsLock, NULL);
}

ThreadPool::~ThreadPool()
{
	pthread_mutex_destroy(&m_reserveHwThreadsLock);
}

ThreadPool* ThreadPool::getInstance()
{
	if (NULL == m_singleThreadPool)
	{
		m_singleThreadPool = new TBBThreadPool();
		assert(m_singleThreadPool);
	}
	return m_singleThreadPool;
}

void ThreadPool::releaseSingletonInstance()
{
	if (m_singleThreadPool)
	{
		delete m_singleThreadPool;
		m_singleThreadPool = NULL;
	}
}

bool ThreadPool::initializeAffinityThreads()
{
	if (false == gMicExecEnvOptions.use_affinity)
	{
		return true;
	}
	map< unsigned int, vector<unsigned int> > coreToThreadsMap;
	map< unsigned int, vector<unsigned int> >::iterator it;
	ifstream ifs("/proc/cpuinfo", ifstream::in);
	if (ifs == NULL)
	{
		return false;
	}
	string title;
	int coreID = -1;
	int processorID = -1;
    char buff[1024];
	while (ifs.getline(buff,1024, ':'))
	{
		title = string(buff);
		if (title.find("processor") != string::npos)
		{
			assert(-1 == processorID);
			ifs.getline(buff,1024);
			processorID = atoi(buff);
			continue;
		}
		if (title.find("core id") != string::npos)
		{
			assert(-1 == coreID);
			assert(processorID >= 0);
			if ((coreID != -1) || (processorID < 0))
			{
				ifs.close();
				return false;
			}
			ifs.getline(buff,1024);
			coreID = atoi(buff);
			it = coreToThreadsMap.find(coreID);
			if (coreToThreadsMap.end() == it)
			{
				vector<unsigned int> tVec;
				tVec.push_back(processorID);
				coreToThreadsMap.insert( pair<unsigned int, vector<unsigned int> >(coreID, tVec) );
			}
			else
			{
				it->second.push_back(processorID);
			}
			coreID = -1;
			processorID = -1;
			continue;
		}
	}
	ifs.close();

	it = coreToThreadsMap.begin();
	if (coreToThreadsMap.end() != it)
	{
		unsigned int numOfCores = ((gMicExecEnvOptions.num_of_cores > 0) && (gMicExecEnvOptions.num_of_cores < coreToThreadsMap.size())) ? gMicExecEnvOptions.num_of_cores : coreToThreadsMap.size();
		unsigned int numOfThreadsPerCore = it->second.size();
		unsigned int firstCoreID = it->first;
		map< unsigned int, vector<unsigned int> >::reverse_iterator rit;
		rit = coreToThreadsMap.rbegin();
		unsigned int lastCoreID = rit->first;
		for (unsigned int i = 0; i < numOfThreadsPerCore; i++)
		{
			unsigned int currRegisterCores = 0;
			for (it = coreToThreadsMap.begin(); ((it != coreToThreadsMap.end()) && (currRegisterCores < numOfCores)); it++)
			{
				if (((gMicExecEnvOptions.ignore_core_0) && (firstCoreID == it->first)) ||
					((gMicExecEnvOptions.ignore_last_core) && (lastCoreID == it->first)))
				{
					continue;
				}
				assert(i < it->second.size());
				if (i >= it->second.size())
				{
					return false;
				}
				m_orderHwThreadsIds.push_back(it->second[i]);
				currRegisterCores ++;
			}
		}
	}

	// set 'gMicExecEnvOptions.num_of_worker_threads' to be the minimum between 'gMicExecEnvOptions.num_of_worker_threads' and 'm_orderHwThreadsIds.size()' --> the amount of HW threads.
	// because if user set 'CL_CONFIG_MIC_DEVICE_USE_AFFINITY=True' and 'CL_CONFIG_MIC_DEVICE_IGNORE_CORE_0=True' and/or 'CL_CONFIG_MIC_DEVICE_IGNORE_LAST_CORE=True' and did not set 'CL_CONFIG_MIC_DEVICE_NUM_WORKERS'
	// Than it will set automatically.
	if (gMicExecEnvOptions.num_of_worker_threads > m_orderHwThreadsIds.size())
	{
		gMicExecEnvOptions.num_of_worker_threads = m_orderHwThreadsIds.size();
	}

	//initializeReserveAffinityThreadIds();

	return true;
}

bool ThreadPool::setAffinityFromReservedIDs()
{
	bool result = false;
	pthread_mutex_lock(&m_reserveHwThreadsLock);
	pthread_t tOsThreadId = pthread_self();
	if ((m_reserveHwThreadsIDs.size() > 0) && (m_osThreadToHwThread.end() == m_osThreadToHwThread.find(tOsThreadId)))
	{
		unsigned int threadId = m_reserveHwThreadsIDs.back();
		m_reserveHwThreadsIDs.pop_back();
		if (setAffinityForCurrentThread(threadId))
		{
			m_osThreadToHwThread.insert(pair<pthread_t, unsigned int>(tOsThreadId, threadId));
			result = true;
		}
		else
		{
			m_reserveHwThreadsIDs.push_back(threadId);
		}
	}
	pthread_mutex_unlock(&m_reserveHwThreadsLock);
	return result;
}

bool ThreadPool::setAffinityForCurrentThread()
{
	if (0 == m_orderHwThreadsIds.size())
	{
		return true;
	}
	
	unsigned int index = m_nextAffinitiesThreadIndex++;
	index = index % m_orderHwThreadsIds.size();

	return setAffinityForCurrentThread(m_orderHwThreadsIds[index]);
}

bool ThreadPool::setAffinityForCurrentThread(unsigned int hwThreadId)
{
//	cpu_set_t affinityMask;
//	// CPU_ZERO initializes all the bits in the mask to zero.
//	CPU_ZERO(&affinityMask);
//
//	// CPU_SET sets only the bit corresponding to cpu.
//	CPU_SET(hwThreadId, &affinityMask);
//	
//	if (0 != sched_setaffinity( 0, sizeof(cpu_set_t), &affinityMask))
//	{
//		//Report Error
//		printf("WorkerThread SetThreadAffinityMask error: %d\n", errno);
//		return false;
//	}

	return true;
}

bool ThreadPool::releaseReservedAffinity()
{
	bool result = false;
	pthread_mutex_lock(&m_reserveHwThreadsLock);
	map<pthread_t, unsigned int>::iterator iter = m_osThreadToHwThread.find(pthread_self());
	if (iter !=  m_osThreadToHwThread.end())
	{
		m_reserveHwThreadsIDs.push_back(iter->second);
		m_osThreadToHwThread.erase(iter);
		result = true;
	}
	pthread_mutex_unlock(&m_reserveHwThreadsLock);
	return result;
}

void ThreadPool::wakeup_all()
{
    if (m_workers_initialized)
    {
        return;
    }

    OclAutoMutexNative lock( &m_workers_initialization_lock );

    if (m_workers_initialized)
    {
        return;
    }

    startup_all_workers();
    m_workers_initialized = true;
}


static pthread_key_t g_tls_key_idx;


bool TBBThreadPool::init()
{
#ifdef ENABLE_MIC_TBB_TRACER
    TaskInterface::PerfData::global_init();
#endif // ENABLE_MIC_TBB_TRACER

    setenv("KMP_AFFINITY",KMP_AFFINITY, 1);
    setenv("OMP_NESTED", "true", 1);
    setenv("OMP_SCHEDULE", OMP_SCHED, 1);
    setenv("OMP_THREAD_LIMIT", "240", 1);

	// Initialize a order list of HW threads numbers for affinity.
	if (false == initializeAffinityThreads())
	{
		return false;
	}
    
/*	assert(m_numOfWorkers == 0);

	assert(NULL == t_uiWorkerId);
	assert(NULL == t_pScheduler);
	assert(NULL == t_generic);

	// Initialize a order list of HW threads numbers for affinity.
	if (false == initializeAffinityThreads())
	{
		return false;
	}

	// initialize the TLS objects.
	if (NULL == t_uiWorkerId)
	{
		t_uiWorkerId = new tbb::enumerable_thread_specific<unsigned int>;
	}
	
	if (NULL == t_pScheduler)
	{
		t_pScheduler = new tbb::enumerable_thread_specific<tbb::task_scheduler_init*>;
	}

	if (NULL == t_generic)
	{
		t_generic = new tbb::enumerable_thread_specific<GENERIC_TLS_STRUCT::GENERIC_TLS_DATA>;
	}

	assert(t_uiWorkerId);
	assert(t_pScheduler);
	assert(t_generic);

	// In case of allocation failure.
	if ((NULL == t_uiWorkerId) || (NULL == t_pScheduler) || (NULL == t_generic))
	{
		if (t_uiWorkerId)
		{
			delete t_uiWorkerId;
		}
		if (t_pScheduler)
		{
			delete t_pScheduler;
		}
		if (t_generic)
		{
			delete t_generic;
		}
		return false;
	}

	m_numOfWorkers = gMicExecEnvOptions.num_of_worker_threads;
	// Set tbb observe - true
	observe(true);
	// Create extra arena in order to avoid worker threads termination when the last command queue terminates
    registerMasterThread(false);

*/

    pthread_key_create( &g_tls_key_idx, NULL );

    omp_set_num_threads(gMicExecEnvOptions.num_of_worker_threads);
        
	return true;
}

void TBBThreadPool::release()
{
//    m_workers_trapper.release();
	// DO NOTHING.
}

size_t TBBThreadPool::getWorkerID(TlsAccessor* pTlsAccessor)
{
//	bool alreadyHad = false;
//	unsigned int& ret = t_uiWorkerId->local(alreadyHad);
//	return alreadyHad ? ret : INVALID_WORKER_ID;

    unsigned int value = (unsigned int)(size_t)pthread_getspecific( g_tls_key_idx );

    if (0 == value)
    {
        on_scheduler_entry(true);
        value = (unsigned int)(size_t)pthread_getspecific( g_tls_key_idx );
    }
    return (value - 1);
}

void TBBThreadPool::setWorkerID(TlsAccessor* pTlsAccessor, size_t id) 
{ 
    pthread_setspecific( g_tls_key_idx, (void*)(id + 1) );
}


void TBBThreadPool::registerMasterThread(bool affinitize)
{
//	tbb::task_scheduler_init* pScheduler = getScheduler();
//	// If the scheduler didn't set yet and I'm not a worker (I'm muster thread)
//	if ( (NULL == pScheduler) && (!isWorkerScheduler()) )
//	{
//		// TBB can create more thread than req.
//		setScheduler(&tlsAccessor, new tbb::task_scheduler_init(m_numOfWorkers));
//	}
//	if (affinitize)
//	{
		setAffinityFromReservedIDs();
//	}

}

void TBBThreadPool::unregisterMasterThread()
{
//	tbb::task_scheduler_init* pScheduler = getScheduler();
//	if (pScheduler)
//	{
//		delete pScheduler;
//		setScheduler(NULL);
//	}
//	// Release my general TLS pointers.
//	releaseGeneralTls();
//	releaseReservedAffinity();
}

void TBBThreadPool::on_scheduler_entry(bool is_worker)
{
	// uiWorkerId initiate with muster thread ID.
	size_t uiWorkerId = 0;
	TlsAccessor tlsAccessor;
	// If worker thread and didn't set ID for it yet
	uiWorkerId = getNextWorkerID();
	setWorkerID(&tlsAccessor, uiWorkerId);
    uiWorkerId = getWorkerID(&tlsAccessor);

//    setAffinityForCurrentThread( m_orderHwThreadsIds[ omp_get_thread_num() ] );
        
}
	
void TBBThreadPool::on_scheduler_exit(bool is_worker)
{
	// In this point We do it only for worker threads. (Muster threads do the same in "unregisterMasterThread()" method).
	if (is_worker)
	{
		TlsAccessor tlsAccessor;
		setWorkerID(&tlsAccessor, INVALID_WORKER_ID);
//		setScheduler(NULL);
	}
}

////tbb::task_scheduler_init* TBBThreadPool::getScheduler()
////{
////	bool alreadyHad = false;
////	tbb::task_scheduler_init*& ret = t_pScheduler->local(alreadyHad);
////	return alreadyHad ? ret : NULL;
////}

//void TBBThreadPool::setScheduler(TlsAccessor* pTlsAccessor, tbb::task_scheduler_init* init) 
//{
//	TbbTls tls(pTlsAccessor);
//	tls.setTls(TbbTls::SCHEDULER, init);
//}

//void TBBThreadPool::setWorkerID(TlsAccessor* pTlsAccessor, size_t id)
//{
//	TbbTls tls(pTlsAccessor);
//
//	tls.setTls(TbbTls::WORKER_ID, (void*)id);
//}

void TBBThreadPool::initializeReserveAffinityThreadIds()
{
	// If there is threads IDs in 'm_orderHwThreadsIds' (If affinity is ON)
	if (m_orderHwThreadsIds.size() > 0)
	{
		// reserve the first thread ID for one master thread.
		m_reserveHwThreadsIDs.push_back(m_orderHwThreadsIds[0]);
		// remove the reserve ID from the general list. (such that no worker thread will affinities to it)
		m_orderHwThreadsIds.erase(m_orderHwThreadsIds.begin());
	}
}

tbb::task* TBBThreadPool::TrapperTask::execute () 
{
    --(m_owner.startup_workers_left);
    while (0 != m_owner.startup_workers_left)
    {
        hw_pause();
    }  
    m_owner.my_root->wait_for_all();
    --(m_owner.shutdown_workers_left);
    return NULL;
}

TBBThreadPool::TrapWorkers::TrapWorkers() : 
    my_root(NULL), 
    my_context(tbb::task_group_context::bound, tbb::task_group_context::default_traits | tbb::task_group_context::concurrent_wait),
    m_workers_count(0)
{
}

void TBBThreadPool::TrapWorkers::release()
{
    if (NULL == my_root)
    {
        return;
    }
    
    shutdown_workers_left = m_workers_count;
    my_root->decrement_ref_count();
    while (0 != shutdown_workers_left)
    {
        hw_pause();
    } 
    tbb::task::destroy(*my_root);
    my_root = NULL;
}

void TBBThreadPool::TrapWorkers::fire()
{
    m_workers_count = gMicExecEnvOptions.num_of_worker_threads-1;
    
    my_root = new ( tbb::task::allocate_root(my_context) ) tbb::empty_task;
    my_root->set_ref_count(2);
    startup_workers_left = m_workers_count;

    for ( unsigned int i = 0; i < m_workers_count; ++i )
	{
        tbb::task::spawn( *new(tbb::task::allocate_root()) TrapperTask(*this) );
	}

    while (0 != startup_workers_left)
    {
        hw_pause();
    }     

    if (false == gMicExecEnvOptions.trap_workers)
    {
        release();
    }
}

void TBBThreadPool::startup_all_workers()
{
//    m_workers_trapper.fire();
}

//
//
//  TBB thacer
//
//
#ifdef ENABLE_MIC_TBB_TRACER

pthread_key_t                               TaskInterface::PerfData::g_phys_processor_id_tls_key;

#define MIC_TBB_TRACER_PREFIX               "MIC_TBB_TRACER: "
#define MIC_TBB_TRACER_STREAM               stderr
#define MIC_TBB_TRACER_THREADS_PER_CORE     4

void TaskInterface::PerfData::construct(unsigned int worker_id)
{
    start_time = 0;
    end_time = 0;
    search_time = 0;
    processed_indices = NULL;
    processed_indices_limit = 0;
    processed_indices_current = 0;

    m_worker_id = worker_id;
}

void TaskInterface::PerfData::destruct()
{
    if (NULL != processed_indices)
    {
        free (processed_indices);
        construct(m_worker_id);
    }
}

void TaskInterface::PerfData::append_data_item( unsigned int n_coords, unsigned int col, unsigned int raw, unsigned int page )
{
    if (processed_indices_current >= processed_indices_limit)
    {
        resize(n_coords);
    }
    switch (n_coords)
    {
        default:
            break;
        case 3:
            processed_indices[processed_indices_current*n_coords+2] = page;
        case 2:
            processed_indices[processed_indices_current*n_coords+1] = raw;
        case 1:
            processed_indices[processed_indices_current*n_coords+0] = col;
            break;
         
    }
    ++processed_indices_current;
}

void TaskInterface::PerfData::dump_data_item( char* buffer, unsigned int n_coords, unsigned int index )
{
    switch (n_coords)
    {
        default:
            break;

        case 1:
            sprintf(buffer, " %d", processed_indices[index*n_coords+0]);
            break;
        case 2:
            sprintf(buffer, " %d:%d", processed_indices[index*n_coords+0], processed_indices[index*n_coords+1]);
            break;
        case 3:
            sprintf(buffer, " %d:%d:%d", processed_indices[index*n_coords+0], processed_indices[index*n_coords+1], processed_indices[index*n_coords+2]);
            break;                 
    }
}

void TaskInterface::PerfData::resize( unsigned int n_coords ) 
{
    processed_indices_limit += INDICES_DELTA;
    processed_indices = (unsigned int*)realloc(processed_indices, n_coords*sizeof(unsigned int)*processed_indices_limit);
    assert( NULL != processed_indices );
}

void TaskInterface::PerfData::work_group_start()
{
    if (!is_thread_recorded())
    {
        dump_thread_attach();
    }
    
    if (0 == start_time)
    {
        start_time = _RDTSC();
    }
    else
    {
        search_time += (_RDTSC() - end_time);
    }
}

void TaskInterface::PerfData::work_group_end()
{
    end_time = _RDTSC();
}

void TaskInterface::PerfData::global_init()
{
    pthread_key_create( &g_phys_processor_id_tls_key, NULL );
}

void TaskInterface::PerfData::getHwInfoForPhysProcessor( unsigned int processor, 
                                                         unsigned int& core_id, 
                                                         unsigned int& thread_id_on_core ) 
{
    core_id = processor / MIC_TBB_TRACER_THREADS_PER_CORE;
    thread_id_on_core = processor % MIC_TBB_TRACER_THREADS_PER_CORE;
}

void TaskInterface::PerfData::dump_thread_attach()
{
    unsigned int core_id;
    unsigned int thread_on_core_id;
    unsigned int hwThreadId = hw_cpu_idx();

    getHwInfoForPhysProcessor( hwThreadId, core_id, thread_on_core_id );

    fprintf(MIC_TBB_TRACER_STREAM, MIC_TBB_TRACER_PREFIX "THREAD %03d ATTACH_TO HW_CORE=%03d HW_THREAD_ON_CORE=%d\n", m_worker_id, core_id, thread_on_core_id );
    fflush(MIC_TBB_TRACER_STREAM);

    pthread_setspecific( g_phys_processor_id_tls_key, (void*)1 );
}

bool TaskInterface::PerfData::is_thread_recorded()
{
    return (bool)(size_t)pthread_getspecific( g_phys_processor_id_tls_key );
}

void TaskInterface::PerfDataInit()
{
    for (unsigned int i = 0; i < MIC_NATIVE_MAX_WORKER_THREADS; ++i)
    {
        m_perf_data[i].construct(i);
    }
}

void TaskInterface::PerfDataFini( unsigned int command_id, unsigned int dims, size_t dim_0, size_t dim_1, size_t dim_2 )
{
    
    vector<char> buffer;

    size_t buffer_capacity = 10240;
    buffer.resize( buffer_capacity );

    size_t cols  = 1;
    size_t raws  = 1;
    size_t pages = 1;

    if (1 == dims)
    {
        cols = dim_0;
    }
    else if (2 == dims)
    {
        cols = dim_1;
        raws = dim_0;
    }
    else
    {
        cols  = dim_2;
        raws  = dim_1;
        pages = dim_0;
    }

    fprintf(MIC_TBB_TRACER_STREAM, MIC_TBB_TRACER_PREFIX "NDRANGE %05d COORDINATES %d: COLS=%ld RAWS=%ld PAGES=%ld\n", 
                                command_id, dims, cols, raws, pages);
    fflush(MIC_TBB_TRACER_STREAM);

    for (int i = 0; i < MIC_NATIVE_MAX_WORKER_THREADS; ++i)
    {
        PerfData& data = m_perf_data[i];
        char* start = &(buffer[0]); 
        char* last = start;
        last[0] = '\0';
        
        for (unsigned int idx=0; idx<data.processed_indices_current; ++idx)
        {
            if ((last - start + 32) > buffer_capacity)
            {
                buffer_capacity *= 2;
                buffer.resize( buffer_capacity );

                char* old_start = start;
                start = &(buffer[0]); 
                last = start + (last-old_start);
            }

            data.dump_data_item( last, dims, idx );
            last += strlen(last);
        }
        fprintf(MIC_TBB_TRACER_STREAM, MIC_TBB_TRACER_PREFIX "NDRANGE %05d THREAD %03d: attach=%ld detach=%ld search=%ld indices: %s\n", 
                 command_id, i, data.start_time, data.end_time, data.search_time, start);
        fflush(MIC_TBB_TRACER_STREAM);

        data.destruct();
    }
}

#endif // ENABLE_MIC_TBB_TRACER


