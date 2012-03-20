#include <algorithm>
#include <stdint.h>
#include <cstring>
#include <assert.h>
#include <stdio.h>

#include <sink/COIPipeline_sink.h>

#include "native_buffer_commands.h"
#include "mic_device_interface.h"

using namespace std;

using namespace Intel::OpenCL::MICDeviceNative;

extern void execute_command(uint32_t					in_BufferCount,
							 void**						in_ppBufferPointers,
							 uint64_t*					in_pBufferLengths,
							 void*						in_pMiscData,
							 uint16_t					in_MiscDataLength,
							 void*						in_pReturnValue,
							 uint16_t					in_ReturnValueLength,
							 TaskHandler::TASK_TYPES	taskType);


FillMemObjTask::FillMemObjTask() : m_pFillMemObjDispatcherData(NULL), m_fillBufPtr(NULL)
{
}

cl_dev_err_code FillMemObjTask::init(TaskHandler* pTaskHandler)
{
	assert(pTaskHandler);

	m_pCommandTracer = &(pTaskHandler->m_commandTracer);
	// Set total buffers size and num of buffers for the tracer.
	m_pCommandTracer->add_delta_num_of_buffer_sent_to_device((pTaskHandler->m_lockBufferCount));
	unsigned long long bufSize = 0;
	for (unsigned int i = 0; i < pTaskHandler->m_lockBufferCount; i++)
	{
		bufSize = pTaskHandler->m_lockBufferLengths[i];
		m_pCommandTracer->add_delta_buffers_size_sent_to_device(bufSize);
	}

	m_pFillMemObjDispatcherData = (fill_mem_obj_dispatcher_data*)(pTaskHandler->m_dispatcherData);
	assert(m_pFillMemObjDispatcherData);
	// According to the implementation only one preExeDirective should be. it suppose to be BUFFER directive
	assert(m_pFillMemObjDispatcherData->preExeDirectivesCount == 1);
	// get the preExeDirective
	directive_pack preExeDirective = ((directive_pack*)((char*)m_pFillMemObjDispatcherData + m_pFillMemObjDispatcherData->preExeDirectivesArrOffset))[0];
	assert(preExeDirective.id == BUFFER);
	// According to the implementation the buffer should be in index 0
	assert(preExeDirective.bufferDirective.bufferIndex == 0);
	assert(preExeDirective.bufferDirective.bufferIndex < pTaskHandler->m_lockBufferCount);
	// Get the pointer of the buffer we like to fill
	m_fillBufPtr = (char*)(pTaskHandler->m_lockBufferPointers[preExeDirective.bufferDirective.bufferIndex]);

	return CL_DEV_SUCCESS;
}

void FillMemObjTask::finish(TaskHandler* pTaskHandler)
{
	fill_mem_obj_dispatcher_data* pDispatcherData = (fill_mem_obj_dispatcher_data*)(pTaskHandler->m_dispatcherData);
	assert(pDispatcherData);
	COIEVENT completionBarrier;
	bool findBarrier = false;
	// Perform post exexution directive.
	unsigned int numOfPostExeDirectives = pDispatcherData->postExeDirectivesCount;
	if (numOfPostExeDirectives > 0)
	{
		assert(numOfPostExeDirectives == 1);
		// get the postExeDirective (It should be BARRIER according to the implementation)
		directive_pack postExeDirective = ((directive_pack*)((char*)pDispatcherData + pDispatcherData->postExeDirectivesArrOffset))[0];
		assert(postExeDirective.id == BARRIER);
		findBarrier = true;
		// set the signal user barrier (Will signal on destruction)
		completionBarrier = postExeDirective.barrierDirective.end_barrier;
	}
	// Last command, Do NOT call any method of this object after it perform.
	pTaskHandler->FinishTask(completionBarrier, findBarrier);
}


TBBFillMemObjTask::TBBFillMemObjTask() : FillMemObjTask(), m_pTaskExecutor(NULL)
{
}

cl_dev_err_code TBBFillMemObjTask::init(TaskHandler* pTaskHandler)
{
	// Call my parent (NDRangeTask init() method)
	cl_dev_err_code result = FillMemObjTask::init(pTaskHandler);
	if (CL_DEV_FAILED(result))
	{
		m_pTaskExecutor = NULL;
		return result;
	}

	// According to TBB documentation "Always allocate memory for task objects using special overloaded new operators (11.3.2) provided by the library, otherwise the results are undefined.
	// Destruction of a task is normally implicit. (When "execute()" method completes)
	m_pTaskExecutor = new (tbb::task::allocate_root()) TBBFillMemObjExecutor(this, pTaskHandler);
	assert(m_pTaskExecutor);
	if (NULL == m_pTaskExecutor)
	{
		finish(pTaskHandler);
		return CL_DEV_OUT_OF_MEMORY;
	}

	return CL_DEV_SUCCESS;
}

void TBBFillMemObjTask::run()
{
	// Call explicitly to 'tbb::task::execute()'
	m_pTaskExecutor->execute();
	// In case of calling 'execute()' explicitly We should destroy the tbb::task explicitly also.
	tbb::task::destroy(*m_pTaskExecutor);
	m_pTaskExecutor = NULL;
}

TBBFillMemObjTask::TBBFillMemObjExecutor::TBBFillMemObjExecutor(TBBFillMemObjTask* pTbbFillMemObjTask, TaskHandler* pTaskHandler) : m_pTbbFillMemObjTask(pTbbFillMemObjTask), m_taskHandler(pTaskHandler)
{
}

tbb::task* TBBFillMemObjTask::TBBFillMemObjExecutor::execute()
{
	// Copy the arrived dispatcher data to memFillInfo which contain the fill mem info. 
	fill_mem_obj_dispatcher_data memFillInfo;
	memcpy(&memFillInfo, m_pTbbFillMemObjTask->m_pFillMemObjDispatcherData, sizeof(fill_mem_obj_dispatcher_data));
	// Represent the current pattern
	chunk_struct pattern;
	pattern.fromPtr = memFillInfo.pattern;
	pattern.size = memFillInfo.pattern_size;
	// Represent the lastChunk that didn't fill yet. (The ptr and its size)
	chunk_struct lastChunk;
	lastChunk.fromPtr = m_pTbbFillMemObjTask->m_fillBufPtr + memFillInfo.from_offset;
	lastChunk.size = 0;

	executeInternal(m_pTbbFillMemObjTask->m_fillBufPtr, &memFillInfo, &lastChunk, &pattern);
	// Call to fill the last chunk.
	copyPatternOnContRegion(&lastChunk, &pattern);
	return NULL;
}

void TBBFillMemObjTask::TBBFillMemObjExecutor::copyPatternOnContRegion(chunk_struct* chunk, chunk_struct* pattern)
{
	char* offset = chunk->fromPtr;
	uint64_t sizeCopied = 0;
	uint64_t currCopySize = min(chunk->size, pattern->size);
	if (currCopySize <= 0)
	{
		return;
	}
	// Copy to buffer from "offset" "currCopySize" bytes from "pattern"
	memcpy(offset, pattern->fromPtr, currCopySize);
	sizeCopied += currCopySize;
	// If all req. size filled than finish.
	if (sizeCopied >= chunk->size)
	{
		return;
	}
	// otherwise replace pattern pointer to be the last copied ptr in order to be able to copy larger pattern in next steps.
	pattern->fromPtr = chunk->fromPtr;
	while (sizeCopied < chunk->size)
	{
		// advance the offset location
		offset += currCopySize;
		currCopySize = min(chunk->size - sizeCopied, pattern->size);
		memcpy(offset, pattern->fromPtr, currCopySize);
		sizeCopied += currCopySize;
		// Update the pattern size because We can fill larger size in next steps.
		pattern->size = sizeCopied;
	}
}

void TBBFillMemObjTask::TBBFillMemObjExecutor::executeInternal(char* buffPtr, fill_mem_obj_dispatcher_data* pMemFillInfo, chunk_struct* lastChunk, chunk_struct* pattern)
{
	// Leaf
	if (pMemFillInfo->dim_count == 1)
	{
		// optimization to accumulate chunks if the last chunk ends in current chunk location.
		if (lastChunk->fromPtr + lastChunk->size == buffPtr + pMemFillInfo->from_offset)
		{
			lastChunk->size += pMemFillInfo->vRegion[0];
		}
		else
		{
			// First copy the pattern to old chunk
			copyPatternOnContRegion(lastChunk, pattern);
			// update the location and size of the new chunk
			lastChunk->fromPtr = buffPtr + pMemFillInfo->from_offset;
			lastChunk->size = pMemFillInfo->vRegion[0];
		}
		return;
	}
	
	fill_mem_obj_dispatcher_data sRecParam;

	// Copy current parameters
	memcpy(&sRecParam, pMemFillInfo, sizeof(fill_mem_obj_dispatcher_data));
	sRecParam.dim_count = pMemFillInfo->dim_count - 1;

	// Make recursion
	for(unsigned int i=0; i < pMemFillInfo->vRegion[sRecParam.dim_count]; ++i)
	{
		executeInternal( buffPtr, &sRecParam, lastChunk, pattern);
		sRecParam.from_offset += pMemFillInfo->vFromPitch[sRecParam.dim_count-1];
	}
}

COINATIVELIBEXPORT
void fill_mem_object(uint32_t         in_BufferCount,
					 void**           in_ppBufferPointers,
					 uint64_t*        in_pBufferLengths,
					 void*            in_pMiscData,
					 uint16_t         in_MiscDataLength,
					 void*            in_pReturnValue,
					 uint16_t         in_ReturnValueLength)
{
	execute_command(in_BufferCount, in_ppBufferPointers, in_pBufferLengths, in_pMiscData, in_MiscDataLength, in_pReturnValue, in_ReturnValueLength, TaskHandler::FILL_MEM_OBJ_TYPE);
}
