#include <algorithm>
#include <stdint.h>
#include <cstring>
#include <assert.h>
#include <stdio.h>

#include <sink/COIPipeline_sink.h>

#include "native_buffer_commands.h"
#include "mic_device_interface.h"
#include "cl_shared_ptr.hpp"

using namespace std;

using namespace Intel::OpenCL::MICDeviceNative;

FillMemObjTask::FillMemObjTask( const QueueOnDevice& queue ) : TaskHandler(queue), 
    m_pFillMemObjDispatcherData(NULL), m_fillBufPtr(NULL)
{
}

bool FillMemObjTask::InitTask()
{
	// Set total buffers size and num of buffers for the tracer.
	commandTracer().add_delta_num_of_buffer_sent_to_device(m_lockBufferCount);
	unsigned long long bufSize = 0;
	for (unsigned int i = 0; i < m_lockBufferCount; i++)
	{
		bufSize = m_lockBufferLengths[i];
		commandTracer().add_delta_buffers_size_sent_to_device(bufSize);
	}

	m_pFillMemObjDispatcherData = (fill_mem_obj_dispatcher_data*)m_dispatcherData;
	assert(m_pFillMemObjDispatcherData);
	// According to the implementation only one preExeDirective should be. it suppose to be BUFFER directive
	assert(m_pFillMemObjDispatcherData->preExeDirectivesCount == 1);
	// get the preExeDirective
	directive_pack preExeDirective = ((directive_pack*)((char*)m_pFillMemObjDispatcherData + m_pFillMemObjDispatcherData->preExeDirectivesArrOffset))[0];
	assert(preExeDirective.id == BUFFER);
	// According to the implementation the buffer should be in index 0
	assert(preExeDirective.bufferDirective.bufferIndex == 0);
	assert(preExeDirective.bufferDirective.bufferIndex < m_lockBufferCount);
	// Get the pointer of the buffer we like to fill
	m_fillBufPtr = (char*)(m_lockBufferPointers[preExeDirective.bufferDirective.bufferIndex]);

	return true;
}

void FillMemObjTask::FinishTask()
{
	fill_mem_obj_dispatcher_data* pDispatcherData = (fill_mem_obj_dispatcher_data*)(m_dispatcherData);
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
		completionBarrier = postExeDirective.barrierDirective.barrier;
	}
	// Last command, Do NOT call any method of this object after it perform.
	queue().FinishTask(this, completionBarrier, findBarrier);
}

bool FillMemObjTask::Execute()
{
    queue().SignalTaskStart( this );
    commandTracer().set_current_time_tbb_exe_in_device_time_start();
    
	// Copy the arrived dispatcher data to memFillInfo which contain the fill mem info. 
	fill_mem_obj_dispatcher_data memFillInfo;
	memcpy(&memFillInfo, m_pFillMemObjDispatcherData, sizeof(fill_mem_obj_dispatcher_data));
	// Represent the current pattern
	chunk_struct pattern;
	pattern.fromPtr = memFillInfo.pattern;
	pattern.size = memFillInfo.pattern_size;
	// Represent the lastChunk that didn't fill yet. (The ptr and its size)
	chunk_struct lastChunk;
	lastChunk.fromPtr = m_fillBufPtr + memFillInfo.from_offset;
	lastChunk.size = 0;

	executeInternal(m_fillBufPtr, &memFillInfo, &lastChunk, &pattern);
	// Call to fill the last chunk.
	copyPatternOnContRegion(&lastChunk, &pattern);

    commandTracer().set_current_time_tbb_exe_in_device_time_end();

    FinishTask();
    return true;
}


void FillMemObjTask::copyPatternOnContRegion(chunk_struct* chunk, chunk_struct* pattern)
{
	assert(pattern->size <= chunk->size);
	assert((chunk->size % pattern->size) == 0);
	const uint64_t totalSize = chunk->size;
	const uint64_t patternSize = pattern->size;
	char* chunkPtr = chunk->fromPtr;
	const char* patternPtr = pattern->fromPtr;
	for (uint64_t i = 0; i < totalSize; i += patternSize)
	{
		memcpy((chunkPtr + i), patternPtr, patternSize);
	}
}

void FillMemObjTask::executeInternal(char* buffPtr, fill_mem_obj_dispatcher_data* pMemFillInfo, chunk_struct* lastChunk, chunk_struct* pattern)
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
	QueueOnDevice::execute_command( in_BufferCount, in_ppBufferPointers, in_pBufferLengths, 
                                    in_pMiscData, in_MiscDataLength, 
                                    in_pReturnValue, in_ReturnValueLength, 
                                    FILL_MEM_OBJ_TYPE );
}
