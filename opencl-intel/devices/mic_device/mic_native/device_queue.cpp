#include "device_queue.h"
#include "native_thread_pool.h"
#include "thread_local_storage.h"
#include "native_common_macros.h"
#include "cl_shared_ptr.hpp"

#include <sink/COIBuffer_sink.h>
#include <common/COIEvent_common.h>
#include <sink/COIPipeline_sink.h>

#include "native_ndrange_task.h"
#include "native_buffer_commands.h"

using namespace Intel::OpenCL::MICDeviceNative;
using namespace Intel::OpenCL::TaskExecutor;
using namespace Intel::OpenCL::Utils;

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

    TlsAccessor tlsAccessor;

    assert( NULL == QueueOnDevice::getCurrentQueue(&tlsAccessor) && "Queue is already set");
    if ( NULL != QueueOnDevice::getCurrentQueue(&tlsAccessor) )
    {
    	*((cl_dev_err_code*)in_pReturnValue) = CL_DEV_INVALID_OPERATION;
    	return;
    }

    QueueOnDevice* pQueue = NULL;
    
    ThreadPool* thread_pool = ThreadPool::getInstance();
    assert( (NULL != thread_pool) && "Thread pool not exists" );
    if ( NULL == thread_pool )
    {
    	*((cl_dev_err_code*)in_pReturnValue) = CL_DEV_INVALID_OPERATION;
    	return;
    }

    if (data->is_in_order_queue)
    {
        pQueue = new InOrderQueueOnDevice( *thread_pool );
    }
    else
    {
        pQueue = new OutOfOrderQueueOnDevice( *thread_pool );
    }
    
    if ( NULL == pQueue )
    {
    	*((cl_dev_err_code*)in_pReturnValue) = CL_DEV_OUT_OF_MEMORY;
    	return;
    }

    if (data->is_in_order_queue)
    {
        // This is TBB master thread that will be used in execution. Register it.
        ThreadPool* pool = ThreadPool::getInstance();
    	bool ok = pool->registerMasterThread( &tlsAccessor );

        if (!ok)
        {
            delete pQueue;
            *((cl_dev_err_code*)in_pReturnValue) = CL_DEV_OUT_OF_MEMORY;
            return;
        }
    }
    
    QueueOnDevice::setCurrentQueue( &tlsAccessor, pQueue );
    *((cl_dev_err_code*)in_pReturnValue) = CL_DEV_SUCCESS;
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
    bool        is_in_order = false;

    QueueOnDevice* pQueue = QueueOnDevice::getCurrentQueue( &tlsAccessor );
    QueueOnDevice::setCurrentQueue( &tlsAccessor, NULL );
    assert(NULL != pQueue && "pQueue must be valid");
    if ( NULL != pQueue )
    {
        is_in_order = pQueue->isInOrder();
    	delete pQueue;
    }

    if (is_in_order)
    {
	    ThreadPool* pool = ThreadPool::getInstance();
        pool->unregisterMasterThread( &tlsAccessor );
    }
}

SharedPtr<TaskHandler> TaskHandler::TaskFactory(TASK_TYPES taskType, QueueOnDevice& queue,
                                                dispatcher_data* dispatcherData, misc_data* miscData)
{
    SharedPtr<TaskHandler> task;
    
    switch ( taskType )
    {
        case NDRANGE_TASK_TYPE:
            task = NDRangeTask::Allocate( queue );
            break;

        case FILL_MEM_OBJ_TYPE:
            task = FillMemObjTask::Allocate( queue );
            break;

        default:
            assert( false && "Unknown taskType in Factory" );
            return NULL;
    }
    
	// if task creation failed.
	if (NULL == task)
	{
		miscData->errCode = CL_DEV_OUT_OF_MEMORY;
        queue.NotifyTaskAllocationFailed( dispatcherData, miscData );
		return NULL;
	}
    
	// Set arrival time to device for the tracer
	task->commandTracer().set_current_time_cmd_run_in_device_time_start();
	// Set command ID for the tracer
	task->commandTracer().set_command_id((size_t)(dispatcherData->commandIdentifier));

	return task;
}

void QueueOnDevice::execute_command(
                     uint32_t                   in_BufferCount,
					 void**						in_ppBufferPointers,
					 uint64_t*					in_pBufferLengths,
					 void*						in_pMiscData,
					 uint16_t					in_MiscDataLength,
					 void*						in_pReturnValue,
					 uint16_t					in_ReturnValueLength,
					 TASK_TYPES	                taskType)
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

    TlsAccessor tls;
    QueueOnDevice* queue = QueueOnDevice::getCurrentQueue(&tls);

    assert( queue );
    if (NULL == queue)
    {
		NATIVE_PRINTF("Cannot find current queue in execute_command\n");
		return;
    }

	// DO NOT delete this object, It will delete itself after kernel execution
	SharedPtr<TaskHandler> taskHandler = TaskHandler::TaskFactory(taskType, *queue, tDispatcherData, tMiscData);
	if (NULL == taskHandler)
	{
		NATIVE_PRINTF("TaskHandler::TaskFactory() Failed\n");
		return;
	}

	// Initialize the task brefore sending for execution.
	bool ok = queue->InitTask( taskHandler, 
	                           tDispatcherData, tMiscData, 
	                           in_BufferCount, in_ppBufferPointers, in_pBufferLengths, 
	                           in_pMiscData, in_MiscDataLength );
	if (!ok)
	{
		NATIVE_PRINTF("TaskHandler::init() Failed\n");
		return;
	}
    
	// Send the task for execution.
	queue->RunTask(taskHandler);
}

TaskHandler::TaskHandler( const QueueOnDevice& queue ) : 
    m_queue(queue), m_errorCode(CL_DEV_SUCCESS),
    m_dispatcherData(NULL), m_miscData(NULL), 
    m_lockBufferCount(0), m_lockBufferPointers(NULL), 
    m_lockBufferLengths(NULL)
{
}

TaskHandler::~TaskHandler()
{
	// Set leaving time to device for the tracer
	m_commandTracer.set_current_time_cmd_run_in_device_time_end();
}

void TaskHandler::setTaskError(cl_dev_err_code errorCode)
{
    // set error code only if current state is success
    if ((CL_DEV_SUCCESS != errorCode) && (CL_DEV_SUCCESS == m_errorCode))
    {
        m_errorCode = errorCode;
        
    	// If m_miscData defined set it
    	if (m_miscData)
    	{
    		m_miscData->errCode = errorCode;
    	}
    }
}

void QueueOnDevice::RunTask( const SharedPtr<TaskHandler>& task_handler ) const
{
    SharedPtr<ITaskBase> pTask = task_handler.DynamicCast<ITaskBase>();
	assert(pTask);
	if ((NULL == pTask) || (NULL == m_task_list))
	{
		task_handler->setTaskError( CL_DEV_ERROR_FAIL );
		task_handler->FinishTask();
        return;
	}

    m_task_list->Enqueue( pTask );
    m_task_list->Flush();
}

InOrderQueueOnDevice::InOrderQueueOnDevice( ThreadPool& thread_pool ) : QueueOnDevice( thread_pool )
{
    m_task_list = m_thread_pool.getTaskExecutor().CreateTaskList( 
                            CommandListCreationParam( TE_CMD_LIST_IMMEDIATE,  gMicExecEnvOptions.tbb_scheduler ));

    if (NULL == m_task_list)
    {
        //Report Error
        NATIVE_PRINTF("Cannot create in-order TaskList\n");
    }
}

bool InOrderQueueOnDevice::InitTask(const SharedPtr<TaskHandler>& task_handler,
                                    dispatcher_data* dispatcherData, misc_data* miscData, 
                                    uint32_t in_BufferCount, void** in_ppBufferPointers, uint64_t* in_pBufferLengths, 
                                    void* in_pMiscData, uint16_t in_MiscDataLength) const
{
	task_handler->m_dispatcherData = dispatcherData;
	task_handler->m_miscData = miscData;
	// Locking of input buffers is not needed in case of blocking task (Only save the pointer in order to use it later).
	task_handler->m_lockBufferCount = in_BufferCount;
	task_handler->m_lockBufferPointers = in_ppBufferPointers;
	task_handler->m_lockBufferLengths = in_pBufferLengths;

    return task_handler->InitTask();
}

void InOrderQueueOnDevice::FinishTask(const SharedPtr<TaskHandler>& task_handler, COIEVENT& completionBarrier, bool isLegalBarrier) const
{
	assert(false == isLegalBarrier);
}

OutOfOrderQueueOnDevice::OutOfOrderQueueOnDevice( ThreadPool& thread_pool ) : QueueOnDevice( thread_pool )
{
    m_task_list = m_thread_pool.getTaskExecutor().CreateTaskList( TE_CMD_LIST_OUT_OF_ORDER );

    if (NULL == m_task_list)
    {
        //Report Error
        NATIVE_PRINTF("Cannot create out-of-order TaskList\n");
    }
}

bool OutOfOrderQueueOnDevice::InitTask( const SharedPtr<TaskHandler>& task_handler,
                                        dispatcher_data* dispatcherData, misc_data* miscData, 
                                        uint32_t in_BufferCount, void** in_ppBufferPointers, uint64_t* in_pBufferLengths, 
                                        void* in_pMiscData, uint16_t in_MiscDataLength) const
{
	task_handler->m_dispatcherData = dispatcherData;
	task_handler->m_miscData = miscData;
	task_handler->m_lockBufferCount = in_BufferCount;
	// If the client sent buffers, than We should copy their pointers and lock them. (In case of OOO)
	if (in_BufferCount > 0)
	{
		task_handler->m_lockBufferPointers = new void*[in_BufferCount];
		if (NULL == task_handler->m_lockBufferPointers)
		{
			task_handler->setTaskError( CL_DEV_OUT_OF_MEMORY );
			task_handler->FinishTask();
			return false;
		}
		task_handler->m_lockBufferLengths = new uint64_t[in_BufferCount];
		if (NULL == task_handler->m_lockBufferLengths)
		{
			task_handler->setTaskError( CL_DEV_OUT_OF_MEMORY );
			task_handler->FinishTask();
			return false;
		}
		COIRESULT result = COI_SUCCESS;
		// In case of non blocking task, shall lock all input buffers.
		for (unsigned int i = 0; i < in_BufferCount; i++)
		{
			// add ref in order to save the buffer on the device
			result = COIBufferAddRef(in_ppBufferPointers[i]);
			if (result != COI_SUCCESS)
			{
				task_handler->setTaskError( CL_DEV_ERROR_FAIL );
				task_handler->FinishTask();
				return false;
			}
			task_handler->m_lockBufferPointers[i] = in_ppBufferPointers[i];
			task_handler->m_lockBufferLengths[i] = in_pBufferLengths[i];
		}
	}

	// In case of Non blocking task when the dispatcher data was sent by "in_pMiscData" - We have to allocate memory for it and copy its content.
	// (misc_data will always transfer as COIBuffer in case of NonBlocking command)
	if (task_handler->m_dispatcherData == in_pMiscData)
	{
		task_handler->m_dispatcherData = NULL;
		task_handler->m_dispatcherData = (dispatcher_data*)(new char[in_MiscDataLength]);
		if (NULL == task_handler->m_dispatcherData)
		{
			task_handler->setTaskError( CL_DEV_OUT_OF_MEMORY );
			task_handler->FinishTask();
			return false;
		}
		memcpy(task_handler->m_dispatcherData, in_pMiscData, in_MiscDataLength);
	}

	return task_handler->InitTask();
}


void OutOfOrderQueueOnDevice::FinishTask(const SharedPtr<TaskHandler>& task_handler, COIEVENT& completionBarrier, bool isLegalBarrier) const
{
	// For asynch task We must have legal COIEVENT to signal.
	assert(isLegalBarrier);
	COIRESULT coiErr = COI_SUCCESS;
	// Release resources.
	if (task_handler->m_lockBufferPointers)
	{
		// If Non blocking task and the dispatcher_data was delivered by in_pMiscData shall delete the allocation in "lockInputBuffers()"
		if ((task_handler->m_dispatcherData) && (task_handler->m_lockBufferCount > 0) && 
            (task_handler->m_dispatcherData != task_handler->m_lockBufferPointers[task_handler->m_lockBufferCount - DISPATCHER_DATA - 1]))
		{
			delete [] ((char*)(task_handler->m_dispatcherData));
		}
		for (unsigned int i = 0; i < task_handler->m_lockBufferCount; i++)
		{
			// decrement ref in order to release the buffer
			coiErr = COIBufferReleaseRef(task_handler->m_lockBufferPointers[i]);
			assert(COI_SUCCESS == coiErr);
		}
		delete [] task_handler->m_lockBufferPointers;
	}
	if (task_handler->m_lockBufferLengths)
	{
		delete [] task_handler->m_lockBufferLengths;
	}

	// Signal user completion barrier
	coiErr = COIEventSignalUserEvent(completionBarrier);
	assert(COI_SUCCESS == coiErr);
}


void OutOfOrderQueueOnDevice::SignalTaskStart( const SharedPtr<TaskHandler>& task_handler ) const
{
	bool findBarrier = false;
	// find start barrier in pre exexution directives.
	unsigned int numOfPreExeDirectives = task_handler->m_dispatcherData->preExeDirectivesCount;
	if (numOfPreExeDirectives > 0)
	{
		// get teh pointer to postExeDirectivesArr
		directive_pack* preExeDirectivesArr = (directive_pack*)((char*)(task_handler->m_dispatcherData) +
		                                                        task_handler->m_dispatcherData->preExeDirectivesArrOffset);
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

void OutOfOrderQueueOnDevice::NotifyTaskAllocationFailed(  
                        dispatcher_data* dispatcherData, misc_data* miscData ) const
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

