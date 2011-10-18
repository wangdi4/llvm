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
	// If the dispatcher_data is not in in_pMiscData than it suppose to be at in_ppBufferPointers[in_BufferCount - 1] (The last buffer)
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
		// The misc_data is in in_ppBufferPointers[in_BufferCount - 1] in case of dispatcher_data is in in_pMiscData and in in_ppBufferPointers[in_BufferCount - 2] in case of dispatcher_data is in in_ppBufferPointers[in_BufferCount - 1].
		assert(in_BufferCount > tMiscDataBufferIndex);
		assert(in_pBufferLengths[tMiscDataBufferIndex] == sizeof(misc_data) && "in_pBufferLengths[tMiscDataBufferIndex] should be as the size of misc_data");
		tMiscData = (misc_data*)in_ppBufferPointers[tMiscDataBufferIndex];
	}

	// DO NOT delete this object, It will delete itself after kernel execution
	ExecutionTask* exeTask = ExecutionTask::ExecutionTaskFactory(tDispatcherData, tMiscData);
	if (NULL == exeTask)
	{
		NATIVE_PRINTF("ExecutionTask::ExecutionTaskFactory() Failed\n");
		return;
	}
	bool result = exeTask->init(in_BufferCount, in_ppBufferPointers, in_pBufferLengths, in_pMiscData, in_MiscDataLength);
	if (false == result)
	{
		NATIVE_PRINTF("ExecutionTask::init() Failed\n");
		return;
	}
	exeTask->runTask();
}

ExecutionTask::ExecutionTask(dispatcher_data* dispatcherData, misc_data* miscData) : m_dispatcherData(dispatcherData), m_miscData(miscData),
m_lockBufferCount(0), m_lockBufferPointers(NULL), m_lockBufferLengths(NULL), m_kernel(NULL), m_pBinary(NULL), m_progamExecutableMemoryManager(NULL),
m_MemBuffCount(0), m_pMemBuffSizes(NULL), m_lockedParams(NULL), m_signalBarrierFlag(false)
{
}

ExecutionTask::~ExecutionTask()
{
	if (m_pMemBuffSizes)
	{
		delete [] m_pMemBuffSizes;
	}
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

bool ExecutionTask::init(uint32_t in_BufferCount, void** in_ppBufferPointers, uint64_t* in_pBufferLengths, void* in_pMiscData, uint16_t in_MiscDataLength)
{
	// First of all ask for buffers locking
	bool result = lockInputBuffers(in_BufferCount, in_ppBufferPointers, in_pBufferLengths, in_pMiscData, in_MiscDataLength);
	if (false == result)
	{
		// the setting of member m_miscData before deleting this object is o.k. because it is pointer of COIBUFFER
		m_miscData->errCode = CL_DEV_ERROR_FAIL;
		finish();
		NATIVE_PRINTF("ExecutionTask::init - lockInputBuffers failed\n");
		return false;
	}

	ProgramService& tProgramService = ProgramService::getInstance();
#ifndef NDRANGE_UNIT_TEST
	// Get kernel object
	result = tProgramService.get_kernel(m_dispatcherData->kernelDirective.kernelAddress, (const ICLDevBackendKernel_**)&m_kernel, &m_progamExecutableMemoryManager);
	if (false == result)
	{
		// the setting of member m_miscData before deleting this object is o.k. because it is pointer of COIBUFFER
		m_miscData->errCode = CL_DEV_INVALID_KERNEL;
		finish();
		NATIVE_PRINTF("ExecutionTask::init - ProgramService::getInstance().get_kernel failed\n");
		return false;
	}
#endif
	// Set kernel args blob (Still have to set the buffers pointer in the blob
	if (m_dispatcherData->kernelArgSize > 0)
	{
		m_lockedParams = (char*)((char*)m_dispatcherData + m_dispatcherData->kernelArgBlobOffset);
	}
	unsigned int numOfPreExeDirectives = m_dispatcherData->preExeDirectivesCount;
	if (numOfPreExeDirectives > 0)
	{
		// get the pointer to preExeDirectivesArr
		directive_pack* preExeDirectivesArr = (directive_pack*)((char*)m_dispatcherData + m_dispatcherData->preExeDirectivesArrOffset);
		// traverse over the preExeDirectivesArr
		for (unsigned int i = 0; i < numOfPreExeDirectives; i++)
		{
			switch ( preExeDirectivesArr[i].id )
			{
			case BUFFER:
				{
					assert(preExeDirectivesArr[i].bufferDirective.bufferIndex < in_BufferCount);
					// A pointer to memory data
					void* memObj = m_lockBufferPointers[preExeDirectivesArr[i].bufferDirective.bufferIndex];
					// A pointer to mem object descriptor.
					cl_mem_obj_descriptor* pMemObjDesc = &(preExeDirectivesArr[i].bufferDirective.mem_obj_desc);
					pMemObjDesc->pData = memObj;
					// Copy the address of the memory object to the kernel args blob
					assert(preExeDirectivesArr[i].bufferDirective.offset_in_blob < m_dispatcherData->kernelArgSize);
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
	m_dispatcherData->workDesc.convertToClWorkDescriptionType(&tWorkDesc);

	cl_dev_err_code errCode = tProgramService.create_binary(m_kernel, m_lockedParams, m_dispatcherData->kernelArgSize, &tWorkDesc, &m_pBinary);
    if ( CL_DEV_FAILED(errCode) )
	{
		m_miscData->errCode = errCode;
		finish();
		NATIVE_PRINTF("ExecutionTask::init - ProgramService.create_binary() failed\n");
		return false;
	}

	// Update buffer parameters
    m_pBinary->GetMemoryBuffersDescriptions(NULL, &m_MemBuffCount);
	m_pMemBuffSizes = new size_t[m_MemBuffCount];
	if ( NULL == m_pMemBuffSizes )
	{
		m_miscData->errCode = CL_DEV_OUT_OF_MEMORY;
		finish();
		NATIVE_PRINTF("ExecutionTask::init - Allocation of m_pMemBuffSizes failed\n");
		return false;
	}
    m_pBinary->GetMemoryBuffersDescriptions(m_pMemBuffSizes, &m_MemBuffCount);

	return true;
}

void ExecutionTask::finish()
{
	// Perform post exexution directives.
	unsigned int numOfPostExeDirectives = m_dispatcherData->postExeDirectivesCount;
	if (numOfPostExeDirectives > 0)
	{
		// get teh pointer to postExeDirectivesArr
		directive_pack* postExeDirectivesArr = (directive_pack*)((char*)m_dispatcherData + m_dispatcherData->postExeDirectivesArrOffset);
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

void ExecutionTask::getExecutionRegion(uint64_t region[])
{
	// copy execution parameters
	const size_t* pWGSize = m_pBinary->GetWorkGroupSize();
	cl_mic_work_description_type* pWorkDesc = &(m_dispatcherData->workDesc);
	unsigned int i = 0;
	for (i = 0; i < pWorkDesc->workDimension; ++i)
	{
		region[i] = (uint64_t)((pWorkDesc->globalWorkSize[i])/(uint64_t)(pWGSize[i]));
	}
	for (; i < MAX_WORK_DIM; ++i)
	{
		region[i] = 1;
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
	///////////////////////////////////////////////////////   TO DELETE
#ifdef NDRANGE_UNIT_TEST
	foo(m_lockedParams);
	finish();
	return;
#endif
	NATIVE_PRINTF("Running task\n");

	uint64_t dim[MAX_WORK_DIM];
	getExecutionRegion(dim);

	WGContext tCtx;
	// TODO AdirD  - Change the first arg to cmdId
	cl_dev_err_code ret = tCtx.CreateContext(0, m_pBinary, m_pMemBuffSizes, m_MemBuffCount);
	if ( CL_DEV_FAILED(ret) )
	{
		NATIVE_PRINTF("Failed to create new WG context\n");
		m_miscData->errCode = ret;
		return;
	}
	tCtx.GetExecutable()->PrepareThread();

    ICLDevBackendExecutable_* pExec = tCtx.GetExecutable();
    // Execute WG
    size_t groupId[MAX_WORK_DIM] = {0, 0, 0};
    for (; groupId[2] < dim[2]; ++groupId[2])
    {
        for (; groupId[1] < dim[1]; ++groupId[1])
        {
            for (; groupId[0] < dim[0]; ++groupId[0])
            {
                pExec->Execute(groupId, NULL, NULL);
            }
        }
    }
    tCtx.GetExecutable()->RestoreThreadState();

	finish();
}

bool BlockingTask::lockInputBuffers(uint32_t in_BufferCount, void** in_ppBufferPointers, uint64_t* in_pBufferLengths, void* in_pMiscData, uint16_t in_MiscDataLength)
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

bool NonBlockingTask::lockInputBuffers(uint32_t in_BufferCount, void** in_ppBufferPointers, uint64_t* in_pBufferLengths, void* in_pMiscData, uint16_t in_MiscDataLength)
{
	m_lockBufferCount = in_BufferCount;
	m_lockBufferPointers = new void*[in_BufferCount];
	if (NULL == m_lockBufferPointers)
	{
		m_miscData->errCode = CL_DEV_OUT_OF_MEMORY;
		return false;
	}
	m_lockBufferLengths = new uint64_t[in_BufferCount];
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

	// In case of Non blocking task when the dispatcher data was sent by "in_pMiscData" and Must allocate memory for it and copy it to there.
	if (m_dispatcherData == in_pMiscData)
	{
		m_dispatcherData = (dispatcher_data*)(new char[in_MiscDataLength]);
		memcpy(m_dispatcherData, in_pMiscData, in_MiscDataLength);
	}
	return true;
}

void NonBlockingTask::releaseResources()
{
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
			result = COIBufferReleaseRef(m_lockBufferPointers[i]);
			assert(result == COI_SUCCESS);
		}
		delete [] m_lockBufferPointers;
	}
	if (m_lockBufferLengths)
	{
		delete [] m_lockBufferLengths;
	}
}

