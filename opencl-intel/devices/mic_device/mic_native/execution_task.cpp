#include "execution_task.h"
#include "native_program_service.h"
#include "native_common_macros.h"
#include "wg_context.h"

#include <sink/COIBuffer_sink.h>
#include <common/COIEvent_common.h>

#include <sink/COIPipeline_sink.h>
#include <sink/COIProcess_sink.h>

#include <malloc.h>
#include <cstring>
#include <assert.h>
 
using namespace Intel::OpenCL::MICDeviceNative;

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
	assert(in_MiscDataLength == sizeof(unsigned int));
	assert(in_ReturnValueLength == sizeof(cl_dev_err_code));
	// The amount of worker threads.
	unsigned int* numOfWorkers = (unsigned int*)in_pMiscData;
	assert((*numOfWorkers > 0) && (*numOfWorkers < MIC_NATIVE_MAX_WORKER_THREADS));
	cl_dev_err_code* pErr = (cl_dev_err_code*)in_pReturnValue;
	*pErr = CL_DEV_SUCCESS;
	// Create thread pool singleton instance.
	ThreadPool* pThreadPool = ThreadPool::getInstance();
	if (NULL == pThreadPool)
	{
		*pErr = CL_DEV_OUT_OF_MEMORY;
		return;
	}
	// Initialize the thread pool with "numOfWorkers" workers.
	if (false == pThreadPool->init(*numOfWorkers))
	{
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
	// Release the thread pool singleton.
	ThreadPool::releaseSingletonInstance();
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
	ThreadPool::getInstance()->registerMasterThread();
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
	ThreadPool::getInstance()->unregisterMasterThread();
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
	dispatcher_data* tDispatcherData = NULL;
	misc_data* tMiscData = NULL;
	// the buffer index of misc_data in case of it is not in "in_pReturnValue"
	uint32_t tMiscDataBufferIndex = in_BufferCount - 1;
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
	// If the misc_data is in in_pReturnValue
	if (in_pReturnValue)
	{
		assert(in_ReturnValueLength == sizeof(misc_data));
		tMiscData = (misc_data*)in_pReturnValue;
	}
	else
	{
		// The misc_data is in in_ppBufferPointers[in_BufferCount - 1] in case of dispatcher_data is in in_pMiscData and in in_ppBufferPointers[in_BufferCount - 2] in case of dispatcher_data is in in_ppBufferPointers[in_BufferCount - DISPATCHER_DATA -1].
		assert(in_BufferCount > tMiscDataBufferIndex);
		assert(in_pBufferLengths[tMiscDataBufferIndex] == sizeof(misc_data) && "in_pBufferLengths[tMiscDataBufferIndex] should be as the size of misc_data");
		tMiscData = (misc_data*)in_ppBufferPointers[tMiscDataBufferIndex];
	}
	
	// Set init value of misc_data.
	tMiscData->init();	

	// DO NOT delete this object, It will delete itself after kernel execution
	TaskHandler* taskHandler = TaskHandler::TaskFactory(TaskHandler::NDRANGE_TASK_TYPE, tDispatcherData, tMiscData);
	if (NULL == taskHandler)
	{
		NATIVE_PRINTF("TaskHandler::TaskFactory() Failed\n");
		return;
	}
	// Initialize the task brefore sending for execution.
	tMiscData->errCode = taskHandler->InitTask(tDispatcherData, in_BufferCount, in_ppBufferPointers, in_pBufferLengths, in_pMiscData, in_MiscDataLength);
	if (CL_DEV_FAILED(tMiscData->errCode))
	{
		NATIVE_PRINTF("TaskHandler::init() Failed\n");
		return;
	}
	// Send the task for execution.
	taskHandler->RunTask();
}



TaskHandler::TaskHandler() : m_dispatcherData(NULL), m_lockBufferCount(0), m_lockBufferPointers(NULL), m_lockBufferLengths(NULL), m_task(NULL)
{
}


TaskHandler* TaskHandler::TaskFactory(TASK_TYPES taskType, dispatcher_data* dispatcherData, misc_data* miscData)
{
	TaskContainerInterface* pTaskContainer = NULL;
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
					COIRESULT coiErr = COIEventSignalUserEvent(postExeDirectivesArr[i].barrierDirective.end_barrier);
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

	return taskHandler;
}




cl_dev_err_code BlockingTaskHandler::InitTask(dispatcher_data* dispatcherData, uint32_t in_BufferCount, void** in_ppBufferPointers, uint64_t* in_pBufferLengths, void* in_pMiscData, uint16_t in_MiscDataLength)
{
	m_dispatcherData = dispatcherData;
	// Locking of input buffers is not needed in case of blocking task (Only save the pointer in order to use it later).
	m_lockBufferCount = in_BufferCount;
	m_lockBufferPointers = in_ppBufferPointers;
	m_lockBufferLengths = in_pBufferLengths;

	return m_task->init(this);
}

void BlockingTaskHandler::RunTask()
{
	m_task->run();
}

void BlockingTaskHandler::FinishTask(COIEVENT* completionBarrier)
{
	assert(NULL == completionBarrier);
	// Delete this object as the last operation on it.
	delete this;
}




cl_dev_err_code NonBlockingTaskHandler::InitTask(dispatcher_data* dispatcherData, uint32_t in_BufferCount, void** in_ppBufferPointers, uint64_t* in_pBufferLengths, void* in_pMiscData, uint16_t in_MiscDataLength)
{
	m_dispatcherData = dispatcherData;
	m_lockBufferCount = in_BufferCount;
	// If the client sent buffers, than We should copy their content and lock them. (In case of OOO)
	if (in_BufferCount > 0)
	{
		m_lockBufferPointers = new void*[in_BufferCount];
		if (NULL == m_lockBufferPointers)
		{
			m_task->finish(this);
			return CL_DEV_OUT_OF_MEMORY;
		}
		m_lockBufferLengths = new uint64_t[in_BufferCount];
		if (NULL == m_lockBufferLengths)
		{
			m_task->finish(this);
			return CL_DEV_OUT_OF_MEMORY;
		}
		COIRESULT result = COI_SUCCESS;
		// In case of non blocking task, shall lock all input buffers.
		for (unsigned int i = 0; i < in_BufferCount; i++)
		{
			// add ref in order to save the buffer on the device
			result = COIBufferAddRef(in_ppBufferPointers[i]);
			if (result != COI_SUCCESS)
			{
				m_task->finish(this);
				return CL_DEV_ERROR_FAIL;
			}
			m_lockBufferPointers[i] = in_ppBufferPointers[i];
			m_lockBufferLengths[i] = in_pBufferLengths[i];
		}
	}

	// In case of Non blocking task when the dispatcher data was sent by "in_pMiscData" - We have to allocate memory for it and copy its content.
	if (m_dispatcherData == in_pMiscData)
	{
		m_dispatcherData = NULL;
		m_dispatcherData = (dispatcher_data*)(new char[in_MiscDataLength]);
		if (NULL == m_dispatcherData)
		{
			m_task->finish(this);
			return CL_DEV_OUT_OF_MEMORY;
		}
		memcpy(m_dispatcherData, in_pMiscData, in_MiscDataLength);
	}

	return m_task->init(this);
}


void NonBlockingTaskHandler::FinishTask(COIEVENT* completionBarrier)
{
	// For asynch task We must have legal COIEVENT to signal.
	assert(completionBarrier);
	COIEVENT tCompletionBarrier = *completionBarrier;
	// Release resources.
	if (m_lockBufferPointers)
	{
		// If Non blocking task and the dispatcher_data was delivered by in_pMiscData shall delete the allocation in "lockInputBuffers()"
		if ((m_dispatcherData) && (m_lockBufferCount > 0) && (m_dispatcherData != m_lockBufferPointers[m_lockBufferCount - DISPATCHER_DATA - 1]))
		{
			delete [] ((char*)m_dispatcherData);
		}
		COIRESULT result = COI_SUCCESS;
		for (unsigned int i = 0; i < m_lockBufferCount; i++)
		{
			// decrement ref in order to release the buffer
			COIBufferReleaseRef(m_lockBufferPointers[i]);
		}
		delete [] m_lockBufferPointers;
	}
	if (m_lockBufferLengths)
	{
		delete [] m_lockBufferLengths;
	}

	// Signal user completion barrier
	COIRESULT coiErr = COIEventSignalUserEvent(tCompletionBarrier);
	assert(COI_SUCCESS == coiErr);

	// Delete this object as the last operation on it.
	delete this;
}



void TBBNonBlockingTaskHandler::RunTask()
{
	// Enqueue the task to tbb task queue, will execute it asynchronous,
	tbb::task::enqueue(*(((TBBNDRangeTask*)m_task)->getTaskExecutor()));
}




namespace Intel { namespace OpenCL { namespace MICDeviceNative {

	struct TaskLoopBody1D {
		TaskInterface* task;
		TaskLoopBody1D(TaskInterface* t) : task(t) {}
		void operator()(const tbb::blocked_range<int>& r) const {
			unsigned int uiWorkerId = ThreadPool::getInstance()->getWorkerID();
			size_t uiNumberOfWorkGroups = r.size();
            assert(uiNumberOfWorkGroups <= CL_MAX_INT32);

			if (CL_DEV_FAILED(task->attachToThread(uiWorkerId)))
			{
				assert(0);
				return;
			}
			for(size_t k = r.begin(), f = r.end(); k < f; k++ )
					task->executeIteration(k, 0, 0, uiWorkerId);
			task->detachFromThread(uiWorkerId);
		}
	};

	struct TaskLoopBody2D {
		TaskInterface* task;
		TaskLoopBody2D(TaskInterface* t) : task(t) {}
		void operator()(const tbb::blocked_range2d<int>& r) const {
			unsigned int uiWorkerId = ThreadPool::getInstance()->getWorkerID();
			size_t uiNumberOfWorkGroups = (r.rows().size())*(r.cols().size());
            assert(uiNumberOfWorkGroups <= CL_MAX_INT32);

			if (CL_DEV_FAILED(task->attachToThread(uiWorkerId)))
			{
				assert(0);
				return;
			}
			for(size_t j = r.rows().begin(), d = r.rows().end(); j < d; j++ )
				for(size_t k = r.cols().begin(), f = r.cols().end(); k < f; k++ )
					task->executeIteration(k, j, 0, uiWorkerId);
			task->detachFromThread(uiWorkerId);
		}
	};

	struct TaskLoopBody3D {
		TaskInterface* task;
		TaskLoopBody3D(TaskInterface* t) : task(t) {}
		void operator()(const tbb::blocked_range3d<int>& r) const {
			unsigned int uiWorkerId = ThreadPool::getInstance()->getWorkerID();
			size_t uiNumberOfWorkGroups = (r.pages().size())*(r.rows().size())*(r.cols().size());
            assert(uiNumberOfWorkGroups <= CL_MAX_INT32);

			if (CL_DEV_FAILED(task->attachToThread(uiWorkerId)))
			{
				assert(0);
				return;
			}
            for(size_t i = r.pages().begin(), e = r.pages().end(); i < e; i++ )
				for(size_t j = r.rows().begin(), d = r.rows().end(); j < d; j++ )
					for(size_t k = r.cols().begin(), f = r.cols().end(); k < f; k++ )
						task->executeIteration(k, j, i, uiWorkerId);
			task->detachFromThread(uiWorkerId);
		}
	};
}}};


NDRangeTask::NDRangeTask() : m_commandIdentifier((cl_dev_cmd_id)-1), m_kernel(NULL), m_pBinary(NULL), m_progamExecutableMemoryManager(NULL),
m_MemBuffCount(0), m_pMemBuffSizes(NULL), m_dim(0), m_lockedParams(NULL)
{
	// Nullify the ICLDevBackendExecutable_* array.
	memset((ICLDevBackendExecutable_**)m_contextExecutableArr, 0, sizeof(ICLDevBackendExecutable_*) * MIC_NATIVE_MAX_WORKER_THREADS);
}

NDRangeTask::~NDRangeTask()
{
	if (m_pMemBuffSizes)
	{
		delete [] m_pMemBuffSizes;
	}
}

cl_dev_err_code NDRangeTask::init(TaskHandler* pTaskHandler)
{
	assert(pTaskHandler);
	dispatcher_data* pDispatcherData = pTaskHandler->m_dispatcherData;
	assert(pDispatcherData);
	ProgramService& tProgramService = ProgramService::getInstance();
#ifndef NDRANGE_UNIT_TEST
	// Get kernel object
	bool result = tProgramService.get_kernel(pDispatcherData->kernelDirective.kernelAddress, (const ICLDevBackendKernel_**)&m_kernel, &m_progamExecutableMemoryManager);
	if (false == result)
	{
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
				{
					//TODO
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

	// Create the binary.
	cl_dev_err_code errCode = tProgramService.create_binary(m_kernel, m_lockedParams, pDispatcherData->kernelArgSize, &tWorkDesc, &m_pBinary);
    if ( CL_DEV_FAILED(errCode) )
	{
		finish(pTaskHandler);
		NATIVE_PRINTF("NDRangeTask::Init - ProgramService.create_binary() failed\n");
		return errCode;
	}

	// Update buffer parameters
    m_pBinary->GetMemoryBuffersDescriptions(NULL, &m_MemBuffCount);
	m_pMemBuffSizes = new size_t[m_MemBuffCount];
	if (NULL == m_pMemBuffSizes)
	{
		finish(pTaskHandler);
		NATIVE_PRINTF("NDRangeTask::Init - Allocation of m_pMemBuffSizes failed\n");
		return CL_DEV_OUT_OF_MEMORY;
	}
    m_pBinary->GetMemoryBuffersDescriptions(m_pMemBuffSizes, &m_MemBuffCount);

	const size_t* pWGSize = m_pBinary->GetWorkGroupSize();
	cl_mic_work_description_type* pWorkDesc = &(pTaskHandler->m_dispatcherData->workDesc);
	m_dim = pWorkDesc->workDimension;
	assert((m_dim >= 1) && (m_dim <= MAX_WORK_DIM));
	// Calculate the region of each dimention in the task.
	unsigned int i = 0;
	for (i = 0; i < m_dim; ++i)
	{
		m_region[i] = (uint64_t)((pWorkDesc->globalWorkSize[i])/(uint64_t)(pWGSize[i]));
	}
	for (; i < MAX_WORK_DIM; ++i)
	{
		m_region[i] = 1;
	}

	return CL_DEV_SUCCESS;
}

void NDRangeTask::finish(TaskHandler* pTaskHandler)
{
	// First of all release all BE execution contexts (For each worker thread).
	// It is safe to do it now because now the execution of this task was compeleted.
	// We must do it now because, after this point there is option to delete the BE Program that create the resources for this execution object.
	for (unsigned int i = 0; i < MIC_NATIVE_MAX_WORKER_THREADS; i++)
	{
		// In case that worker thread with ID (i + 1) join the execution of this task.
		if (m_contextExecutableArr[i])
		{
			// Release BE executable context
			((ICLDevBackendExecutable_*)m_contextExecutableArr[i])->Release();
			m_contextExecutableArr[i] = NULL;
		}
	}

	dispatcher_data* pDispatcherData = pTaskHandler->m_dispatcherData;
	assert(pDispatcherData);
	COIEVENT* completionBarrier = NULL;
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
					// set the signal user barrier (Will signal on destruction)
					*completionBarrier = postExeDirectivesArr[i].barrierDirective.end_barrier;
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
	pTaskHandler->FinishTask(completionBarrier);
}


cl_dev_err_code NDRangeTask::attachToThread(unsigned int uiWorkerId)
{
	ThreadPool* pThreadPool = ThreadPool::getInstance();
	// Get the WGContext instance of this thread
	WGContext* pCtx = (WGContext*)(pThreadPool->getGeneralTls(GENERIC_TLS_STRUCT::NDRANGE_TLS_ENTRY));
	// If didn't created yet for this thread, create it and store it in its TLS.
	if (NULL == pCtx)
	{
		// This object will destruct before thread termination
		pCtx = new WGContext();
		if (NULL == pCtx)
		{
			return CL_DEV_OUT_OF_MEMORY;
		}
		pThreadPool->setGeneralTls(GENERIC_TLS_STRUCT::NDRANGE_TLS_ENTRY, pCtx);
	}
	// If can NOT recycle the current context - This is the case when my current context is not the context of the next execution
	if (m_commandIdentifier != pCtx->GetCmdId())
	{
		// if it is worker thread, Invalidate the previous BE executable context and Nullify m_contextExecutableArr[uiWorkerId-1]
		if (uiWorkerId > 0)
		{
			// Need to invalidate the context in order to release the old BE executable
			pCtx->InvalidateContext();
			m_contextExecutableArr[uiWorkerId-1] = NULL;
		}
		// Set a new Context.
		cl_dev_err_code ret = pCtx->CreateContext(m_commandIdentifier, m_pBinary, m_pMemBuffSizes, m_MemBuffCount);
		if (CL_DEV_FAILED(ret))
		{
			pCtx->InvalidateContext();
			return ret;
		}
		// Register the new BE executable context to m_contextExecutableArr[uiWorkerId-1] in order to release it when this task completes
		if (uiWorkerId > 0)
		{
			m_contextExecutableArr[uiWorkerId-1] = pCtx->GetExecutable();
			assert(m_contextExecutableArr[uiWorkerId-1]);
		}
	}
	pCtx->GetExecutable()->PrepareThread();

	return CL_DEV_SUCCESS;
}

cl_dev_err_code	NDRangeTask::detachFromThread(unsigned int uiWorkerId)
{
	ThreadPool* pThreadPool = ThreadPool::getInstance();
	// Get the WGContext object of this thread
	WGContext* pCtx = (WGContext*)(pThreadPool->getGeneralTls(GENERIC_TLS_STRUCT::NDRANGE_TLS_ENTRY));
	assert(pCtx);
	cl_dev_err_code ret = pCtx->GetExecutable()->RestoreThreadState();
    
	//For muster threads, must invalidate the context.
    if (0 == uiWorkerId)
    {
        pCtx->InvalidateContext();
    }
	
	return ret;
}

void NDRangeTask::executeIteration(size_t x, size_t y, size_t z, unsigned int uiWorkerId)
{
	// Get the WGContext object of this thread
	WGContext* pCtx = (WGContext*)(ThreadPool::getInstance()->getGeneralTls(GENERIC_TLS_STRUCT::NDRANGE_TLS_ENTRY));
	assert(pCtx);

    ICLDevBackendExecutable_* pExec = pCtx->GetExecutable();
	assert(pExec);

	// Execute WG
	size_t groupId[MAX_WORK_DIM] = {x, y, z};
	pExec->Execute(groupId, NULL, NULL);
}

bool NDRangeTask::constructTlsEntry(void** outEntry)
{
	WGContext* pContext = new WGContext();
	if (NULL == pContext)
	{
		return false;
	}
	*outEntry = pContext;
	return true;
}

void NDRangeTask::destructTlsEntry(void* pEntry)
{
	if (pEntry)
	{
		delete(((WGContext*)pEntry));
	}
}




TBBNDRangeTask::TBBNDRangeTask() : NDRangeTask(), m_pTaskExecutor(NULL)
{
}

cl_dev_err_code TBBNDRangeTask::init(TaskHandler* pTaskHandler)
{
	// According to TBB documentation "Always allocate memory for task objects using special overloaded new operators (11.3.2) provided by the library, otherwise the results are undefined.
	// Destruction of a task is normally implicit. (When "execute()" method completes)
	m_pTaskExecutor = new (tbb::task::allocate_root()) TBBNDRangeExecutor(this, pTaskHandler, m_dim, m_region);
	assert(m_pTaskExecutor);
	if (NULL == m_pTaskExecutor)
	{
		finish(pTaskHandler);
		return CL_DEV_OUT_OF_MEMORY;
	}

	// Call my parent (NDRangeTask init() method)
	cl_dev_err_code result = NDRangeTask::init(pTaskHandler);
	if (CL_DEV_FAILED(result))
	{
		tbb::task::destroy(*m_pTaskExecutor);
		m_pTaskExecutor = NULL;
		return result;
	}

	return CL_DEV_SUCCESS;
}


TBBNDRangeTask::TBBNDRangeExecutor::TBBNDRangeExecutor(TBBNDRangeTask* pTbbNDRangeTask, TaskHandler* pTaskHandler, const unsigned int& dim, uint64_t* region) : m_pTbbNDRangeTask(pTbbNDRangeTask), 
m_taskHandler(pTaskHandler), m_dim(dim), m_region(region)
{
}

tbb::task* TBBNDRangeTask::TBBNDRangeExecutor::execute()
{
	assert(m_pTbbNDRangeTask);
	assert(m_taskHandler);
#ifdef NDRANGE_UNIT_TEST
	foo(m_lockedParams);
	m_pTaskExecutor->finish(m_taskHandler);
	return NULL;
#endif

	if (1 == m_dim)
	{
		assert(m_region[0] <= CL_MAX_INT32);
		tbb::parallel_for(tbb::blocked_range<int>(0, (int)m_region[0]), TaskLoopBody1D(m_pTbbNDRangeTask), tbb::auto_partitioner());
	}
	else if (2 == m_dim)
	{
		assert(m_region[0] <= CL_MAX_INT32);
		assert(m_region[1] <= CL_MAX_INT32);
		tbb::parallel_for(tbb::blocked_range2d<int>(0, (int)m_region[1],
													0, (int)m_region[0]),
													TaskLoopBody2D(m_pTbbNDRangeTask), tbb::auto_partitioner());
	}
	else
	{
		assert(m_region[0] <= CL_MAX_INT32);
		assert(m_region[1] <= CL_MAX_INT32);
		assert(m_region[2] <= CL_MAX_INT32);
		tbb::parallel_for(tbb::blocked_range3d<int>(0, (int)m_region[2],
													0, (int)m_region[1],
													0, (int)m_region[0]),
													TaskLoopBody3D(m_pTbbNDRangeTask), tbb::auto_partitioner());
	}

	// Call to "finish()" as the last command in order to release resources and notify for completion (in case of OOO).
	m_pTbbNDRangeTask->finish(m_taskHandler);
	return NULL;
}




GENERIC_TLS_STRUCT::fnConstructorTls* GENERIC_TLS_STRUCT::constructorTlsArr[GENERIC_TLS_STRUCT::NUM_OF_GENERIC_TLS_ENTRIES] = {
	&NDRangeTask::constructTlsEntry		// NDRange task general TLS creator.
};

GENERIC_TLS_STRUCT::fnDestructorTls* GENERIC_TLS_STRUCT::destructorTlsArr[GENERIC_TLS_STRUCT::NUM_OF_GENERIC_TLS_ENTRIES] = {
	&NDRangeTask::destructTlsEntry		// NDRange task general TLS destructor.
};


ThreadPool* ThreadPool::m_singleThreadPool = NULL;

ThreadPool::ThreadPool() : m_numOfWorkers(0), m_NextWorkerID(1)
{
}

ThreadPool::~ThreadPool()
{
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
	


// TLS objects:
tbb::enumerable_thread_specific<unsigned int>*                          TBBThreadPool::t_uiWorkerId = NULL;
tbb::enumerable_thread_specific<tbb::task_scheduler_init*>*             TBBThreadPool::t_pScheduler = NULL;
tbb::enumerable_thread_specific<GENERIC_TLS_STRUCT::GENERIC_TLS_DATA>*	TBBThreadPool::t_generic = NULL;

bool TBBThreadPool::init(unsigned int numOfWorkers)
{
	assert(m_numOfWorkers == 0);
	assert(numOfWorkers > 0);

	assert(NULL == t_uiWorkerId);
	assert(NULL == t_pScheduler);
	assert(NULL == t_generic);

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

	m_numOfWorkers = numOfWorkers;
	// Set tbb observe - true
	observe(true);
	return true;
}

void TBBThreadPool::release()
{
	// DO NOTHING.
}

unsigned int TBBThreadPool::getWorkerID()
{
	bool alreadyHad = false;
	unsigned int& ret = t_uiWorkerId->local(alreadyHad);
	return alreadyHad ? ret : INVALID_WORKER_ID;
}

void TBBThreadPool::registerMasterThread()
{
	tbb::task_scheduler_init* pScheduler = getScheduler();
	// If the scheduler didn't set yet and I'm not a worker (I'm muster thread)
	if ( (NULL == pScheduler) && (!isWorkerScheduler()) )
	{
		setScheduler(new tbb::task_scheduler_init(m_numOfWorkers));
	}
}

void TBBThreadPool::unregisterMasterThread()
{
	tbb::task_scheduler_init* pScheduler = getScheduler();
	if (pScheduler)
	{
		delete pScheduler;
		setScheduler(NULL);
	}
	// Release my general TLS pointers.
	releaseGeneralTls();
}

void* TBBThreadPool::getGeneralTls(unsigned int index)
{
	assert(index < GENERIC_TLS_STRUCT::NUM_OF_GENERIC_TLS_ENTRIES);
	GENERIC_TLS_STRUCT::GENERIC_TLS_DATA& ret = t_generic->local();
	return ret.getElementAt(index);
}

void TBBThreadPool::setGeneralTls(unsigned int index, void* pGeneralTlsObj)
{
	assert(index < GENERIC_TLS_STRUCT::NUM_OF_GENERIC_TLS_ENTRIES);
	GENERIC_TLS_STRUCT::GENERIC_TLS_DATA& ret = t_generic->local();
	ret.setElementAt(index, pGeneralTlsObj);
}

void TBBThreadPool::on_scheduler_entry(bool is_worker)
{
	// uiWorkerId initiate with muster thread ID.
	unsigned int uiWorkerId = 0;
	// If worker thread and didn't set ID for it yet
	if ((is_worker) && (INVALID_WORKER_ID == getWorkerID()))
	{
		uiWorkerId = getNextWorkerID();
		setScheduler((tbb::task_scheduler_init*)INVALID_SCHEDULER_ID);
	}
	setWorkerID(uiWorkerId);
	
	// Run over all the general Tls constructors, call them and set them on this thread general TLS.
	for (unsigned int i = 0; i < GENERIC_TLS_STRUCT::NUM_OF_GENERIC_TLS_ENTRIES; i++)
	{
		void* tlsData = NULL;
		bool result = GENERIC_TLS_STRUCT::constructorTlsArr[i](&tlsData);
		assert(result);
		assert(NULL == getGeneralTls(i));
		setGeneralTls(i, tlsData);
	}
}
	
void TBBThreadPool::on_scheduler_exit(bool is_worker)
{
	// In this point We do it only for worker threads. (Muster threads do the same in "unregisterMasterThread()" method).
	if (is_worker)
	{
		setWorkerID(INVALID_WORKER_ID);
		setScheduler(NULL);
		releaseGeneralTls();
	}
}

tbb::task_scheduler_init* TBBThreadPool::getScheduler()
{
	bool alreadyHad = false;
	tbb::task_scheduler_init*& ret = t_pScheduler->local(alreadyHad);
	return alreadyHad ? ret : NULL;
}

void TBBThreadPool::releaseGeneralTls()
{
	// Run over all the general Tls destructors, call them and set the appropriate general TLS to NULL
	for (unsigned int i = 0; i < GENERIC_TLS_STRUCT::NUM_OF_GENERIC_TLS_ENTRIES; i++)
	{
		void* tlsData = getGeneralTls(i);
		if (tlsData)
		{
			GENERIC_TLS_STRUCT::destructorTlsArr[i](tlsData);
		}
		setGeneralTls(i, NULL);
	}
}
