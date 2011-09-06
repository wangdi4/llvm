#include "execution_task.h"
#include "native_program_service.h"
#include "native_common_macros.h"

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
		res[i] = a[i] + b[i];
	}
}
// For testing only
//////////////////////////////////////////////////////

COINATIVELIBEXPORT
void execute_NDRange(uint32_t         in_BufferCount,
					 void**           in_ppBufferPointers,
					 uint64_t*        in_pBufferLengths,
					 void*            in_pMiscData,
					 uint16_t         in_MiscDataLength,
					 void*            in_pReturnValue,
					 uint16_t         in_ReturnValueLength)
{
	NATIVE_PRINTF("Enter execute_NDRange\n");
	assert(in_BufferCount >= AMOUNT_OF_CONSTANT_BUFFERS && "Should be at least AMOUNT_OF_CONSTANT_BUFFERS buffers (for dispatcher_data and for misc_data)");
	assert(in_pBufferLengths[DISPATCHER_DATA_INDEX] >= sizeof(dispatcher_data) && "in_pBufferLengths[DISPATCHER_DATA_INDEX] should be at least as the size of dispatcher_data");
	assert(in_pBufferLengths[MISC_DATA_INDEX] == sizeof(misc_data) && "in_pBufferLengths[MISC_DATA_INDEX] should be at least as the size of misc_data");
	dispatcher_data* tDispatcherData = (dispatcher_data*)(in_ppBufferPointers[DISPATCHER_DATA_INDEX]);
	misc_data* tMiscData = (misc_data*)(in_ppBufferPointers[MISC_DATA_INDEX]);
	// DO NOT delete this object, It will delete itself after kernel execution
	ExecutionTask* exeTask = ExecutionTask::ExecutionTaskFactory(tDispatcherData, tMiscData);
	if (NULL == exeTask)
	{
		NATIVE_PRINTF("ExecutionTask::ExecutionTaskFactory() Failed\n");
		return;
	}
	bool result = exeTask->init(in_BufferCount, in_ppBufferPointers, in_pBufferLengths);
	if (false == result)
	{
		NATIVE_PRINTF("ExecutionTask::init() Failed\n");
		return;
	}
	exeTask->runTask();
	NATIVE_PRINTF("Exit execute_NDRange\n");
}

ExecutionTask::ExecutionTask(dispatcher_data* dispatcherData, misc_data* miscData) : m_dispatcherData(dispatcherData), m_miscData(miscData), 
m_lockBufferCount(0), m_lockBufferPointers(NULL), m_lockBufferLengths(NULL), m_kernel(NULL), m_progamExecutableMemoryManager(NULL), m_lockedParams(NULL), m_signalBarrierFlag(false)
{
}

ExecutionTask::~ExecutionTask()
{
}

ExecutionTask* ExecutionTask::ExecutionTaskFactory(dispatcher_data* dispatcherData, misc_data* miscData)
{
	ExecutionTask* exeTask = NULL;
	exeTask = (dispatcherData->isInOrderQueue == true) ? (ExecutionTask*)new BlockingTask(dispatcherData, miscData) : (ExecutionTask*)new NonBlockingTask(dispatcherData, miscData);
	if (NULL == exeTask)
	{
		miscData->errCode = CL_DEV_OUT_OF_MEMORY;
	}
	return exeTask;
}

bool ExecutionTask::init(uint32_t in_BufferCount, void** in_ppBufferPointers, uint64_t* in_pBufferLengths)
{
	// First of all ask for buffers locking
	bool result = lockInputBuffers(in_BufferCount, in_ppBufferPointers, in_pBufferLengths);
	if (false == result)
	{
		// the setting of member m_miscData before deleting this object is o.k. because it is pointer of COIBUFFER
		m_miscData->errCode = CL_DEV_ERROR_FAIL;
		delete this;
		NATIVE_PRINTF("ExecutionTask::init - lockInputBuffers failed\n");
		return false;
	}
#ifndef NDRANGE_UNIT_TEST
	// Get kernel object
	result = ProgramService::getInstance().get_kernel(m_dispatcherData->kernelDirective.kernelAddress, (const ICLDevBackendKernel_**)&m_kernel, &m_progamExecutableMemoryManager);
	if (false == result)
	{
		// the setting of member m_miscData before deleting this object is o.k. because it is pointer of COIBUFFER
		m_miscData->errCode = CL_DEV_INVALID_KERNEL;
		delete this;
		NATIVE_PRINTF("ExecutionTask::init - ProgramService::getInstance().get_kernel failed\n");
		return false;
	}
#endif
	// Set kernel args blob (Still have to set the buffers pointer in the blob
	if (m_dispatcherData->kernelArgSize > 0)
	{
		m_lockedParams = (char*)((char*)(m_lockBufferPointers[DISPATCHER_DATA_INDEX]) + m_dispatcherData->kernelArgBlobOffset);
	}
	unsigned int numOfPreExeDirectives = m_dispatcherData->preExeDirectivesCount;
	if (numOfPreExeDirectives > 0)
	{
		// get the pointer to preExeDirectivesArr
		directive_pack* preExeDirectivesArr = (directive_pack*)((char*)(m_lockBufferPointers[DISPATCHER_DATA_INDEX]) + m_dispatcherData->preExeDirectivesArrOffset);
		// traverse over the preExeDirectivesArr
		for (unsigned int i = 0; i < numOfPreExeDirectives; i++)
		{
			switch ( preExeDirectivesArr[i].id )
			{
			case BUFFER:
				{
					assert(preExeDirectivesArr[i].bufferDirective.bufferIndex < in_BufferCount);
					void* memObj = m_lockBufferPointers[preExeDirectivesArr[i].bufferDirective.bufferIndex];
					// Copy the address of the memory object to the kernel args blob
					assert(preExeDirectivesArr[i].bufferDirective.offset_in_blob < m_dispatcherData->kernelArgSize);
					void** pTempLockedParams = (void**)(m_lockedParams + preExeDirectivesArr[i].bufferDirective.offset_in_blob);
					*pTempLockedParams = memObj;
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
	return true;
}

void ExecutionTask::finish()
{
	// Perform post exexution directives.
	unsigned int numOfPostExeDirectives = m_dispatcherData->postExeDirectivesCount;
	if (numOfPostExeDirectives > 0)
	{
		// get teh pointer to postExeDirectivesArr
		directive_pack* postExeDirectivesArr = (directive_pack*)((char*)(m_lockBufferPointers[0]) + m_dispatcherData->postExeDirectivesArrOffset);
		// traverse over the postExeDirectivesArr
		for (unsigned int i = 0; i < numOfPostExeDirectives; i++)
		{
			switch ( postExeDirectivesArr[i].id )
			{
			case BARRIER:
				{
					// set the signal user barrier (Will signal on destruction)
					m_completionBarrier = postExeDirectivesArr[i].barrierDirective.end_barrier;
					m_signalBarrierFlag = true;
					break;
				}
			case PRINTF:
				{
					// TODO
					assert(0);
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
	delete this;
}

void ExecutionTask::signalUserBarrierForCompletion()
{
	// If exist postExeDirective of Barrier
	if (m_signalBarrierFlag)
	{
		COIRESULT result = COIEventSignalUserEvent(m_completionBarrier);
		assert(result == COI_SUCCESS);
	}
}



BlockingTask::BlockingTask(dispatcher_data* dispatcherData, misc_data* miscData) : ExecutionTask(dispatcherData, miscData)
{
}

BlockingTask::~BlockingTask()
{
}

void BlockingTask::runTask()
{
	//TODO
	///////////////////////////////////////////////////////   TO DELETE
	foo(m_lockedParams);
	return;
	NATIVE_PRINTF("Running task\n");
	NATIVE_PRINTF("Kernel address - %ld\n", m_dispatcherData->kernelDirective.kernelAddress);
	directive_pack* directivesArr[2] = {(directive_pack*)((char*)(m_lockBufferPointers[0]) + m_dispatcherData->preExeDirectivesArrOffset)
		, (directive_pack*)((char*)(m_lockBufferPointers[0]) + m_dispatcherData->postExeDirectivesArrOffset)};
	unsigned int directivesCount[2] = {m_dispatcherData->preExeDirectivesCount, m_dispatcherData->postExeDirectivesCount};
	char title[2][50] = {"The PreExeDirectives are:\n\0", "The PostExeDirectives are:\n\0"};
	for (unsigned int j = 0; j < 2; j++)
	{
		NATIVE_PRINTF("%s", title[j]);
		for (unsigned int i = 0; i < directivesCount[j]; i++)
		{
			switch ( directivesArr[j][i].id )
			{
			case KERNEL:
				{
					NATIVE_PRINTF("Type kernel\n");
					NATIVE_PRINTF("Kernel address is %ld\n", directivesArr[j][i].kernelDirective.kernelAddress);
					break;
				}
			case BUFFER:
				{
					NATIVE_PRINTF("Type Buffer\n");
					NATIVE_PRINTF("Buffer index - %d, buffer offset - %ld, the local ptr - %p\n", directivesArr[j][i].bufferDirective.bufferIndex, directivesArr[j][i].bufferDirective.offset_in_blob, m_lockBufferPointers[directivesArr[j][i].bufferDirective.bufferIndex]);
					break;
				}
			case BARRIER:
				{
					NATIVE_PRINTF("Type Barrier\n");
					break;
				}
			case PRINTF:
				{
					NATIVE_PRINTF("Type Printf\n");
					NATIVE_PRINTF("Printf Buffer index - %d, Printf buffer size - %ld, the local ptr - %p\n", directivesArr[j][i].printfDirective.bufferIndex, directivesArr[j][i].printfDirective.size, m_lockBufferPointers[directivesArr[j][i].printfDirective.bufferIndex]);
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

	//////////////////////////////////////////////////////

	finish();
}

bool BlockingTask::lockInputBuffers(uint32_t in_BufferCount, void** in_ppBufferPointers, uint64_t* in_pBufferLengths)
{
	// Locking of input buffers is not needed in case of blocking task (Only save the pointer in order to use it in 'executePostExeConditions'
	m_lockBufferCount = in_BufferCount;
	m_lockBufferPointers = in_ppBufferPointers;
	m_lockBufferLengths = in_pBufferLengths;
	return true;
}



NonBlockingTask::NonBlockingTask(dispatcher_data* dispatcherData, misc_data* miscData) : ExecutionTask(dispatcherData, miscData)
{
}

NonBlockingTask::~NonBlockingTask()
{
	// Unlock and Release the input COIBUFFERs
	releaseResources();
	// Signal user barrier for completion (if needed). MUST DO IT AFTER 'COIBufferReleaseRef' OPERATION in relax COIBUFFER type
	signalUserBarrierForCompletion();
}

void NonBlockingTask::runTask()
{
		NATIVE_PRINTF("Entering NonBlockingTask::runTask()\n");
	//TODO
}

bool NonBlockingTask::lockInputBuffers(uint32_t in_BufferCount, void** in_ppBufferPointers, uint64_t* in_pBufferLengths)
{
	m_lockBufferCount = in_BufferCount;
	m_lockBufferPointers = (void**)malloc(sizeof(void*) * in_BufferCount);
	if (NULL == m_lockBufferPointers)
	{
		m_miscData->errCode = CL_DEV_OUT_OF_MEMORY;
		return false;
	}
	m_lockBufferLengths = (uint64_t*)malloc(sizeof(uint64_t) * in_BufferCount);
	if (NULL == m_lockBufferLengths)
	{
		m_miscData->errCode = CL_DEV_OUT_OF_MEMORY;
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
			m_miscData->errCode = CL_DEV_ERROR_FAIL;
			return false;
		}
		m_lockBufferPointers[i] = in_ppBufferPointers[i];
		m_lockBufferLengths[i] = in_pBufferLengths[i];
	}
	return true;
}

void NonBlockingTask::releaseResources()
{
	COIRESULT result = COI_SUCCESS;
	for (unsigned int i = 0; i < m_lockBufferCount; i++)
	{
		// decrement ref in order to release the buffer
		result = COIBufferReleaseRef(m_lockBufferPointers[i]);
		assert(result == COI_SUCCESS);
	}
	free(m_lockBufferPointers);
	free(m_lockBufferLengths);
}

