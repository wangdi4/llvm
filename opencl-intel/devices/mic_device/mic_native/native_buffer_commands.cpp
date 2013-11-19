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

#include "native_buffer_commands.h"
#include "mic_device_interface.h"
#include "task_handler.h"

#include <cl_shared_ptr.hpp>

#include <sink/COIPipeline_sink.h>

#include <algorithm>
#include <stdint.h>
#include <cstring>
#include <assert.h>
#include <stdio.h>

using namespace std;

using namespace Intel::OpenCL::MICDeviceNative;
using namespace Intel::OpenCL::MICDevice;

#ifdef _DEBUG
using namespace Intel::OpenCL::UtilsNative;
#endif

#define USE_SINGLE_WORKER_FOR_BUFF_SIZE 131072
#define KILOBYTE *1024
#define MEGABYTE *1024 KILOBYTE

const FillMemObjTask::tasksForWorkerConfStruct FillMemObjTask::m_tasksForWorkerConf[] = { 
	{ 128 KILOBYTE, 1 },
	{ 8   MEGABYTE, 4 },
	{ 64  MEGABYTE, 16 },
	{ (size_t)-1  , 64 }
};

COINATIVELIBEXPORT
void execute_command_fill_mem_object(uint32_t         in_BufferCount,
                     void**           in_ppBufferPointers,
                     uint64_t*        in_pBufferLengths,
                     void*            in_pMiscData,
                     uint16_t         in_MiscDataLength,
                     void*            in_pReturnValue,
                     uint16_t         in_ReturnValueLength)
{
    assert( in_ReturnValueLength == 0 && "We should not return value for command");
    assert( in_MiscDataLength == sizeof(fill_mem_obj_dispatcher_data) && "Size of input MiscData doesn't match");

    fill_mem_obj_dispatcher_data* fillMemObjDispatchData = (fill_mem_obj_dispatcher_data*)in_pMiscData;

    QueueOnDevice* pQueue = (QueueOnDevice*)fillMemObjDispatchData->deviceQueuePtr;
    assert(NULL != pQueue && "pQueue must be valid");

#ifdef _DEBUG
    TlsAccessor tlsAccessor;
    const QueueOnDevice* pTLSQueue = QueueOnDevice::getCurrentQueue( &tlsAccessor );
    assert( pTLSQueue == pQueue && "Queue handle doesn't match");
#endif

    FillMemObjTask fillMemObjTask(in_BufferCount, in_ppBufferPointers, fillMemObjDispatchData, in_MiscDataLength);
    // Increase ref. count to prevent undesired deletion
    fillMemObjTask.IncRefCnt();

    cl_dev_err_code err = fillMemObjTask.getTaskError();
    if ( CL_DEV_SUCCEEDED(err) )
    {
        err = pQueue->Execute(fillMemObjTask.GetAsTaskHandlerBase());
    }

    *((cl_dev_err_code*)in_pReturnValue) = err;
}


FillMemObjTask::FillMemObjTask( uint32_t lockBufferCount, void** pLockBuffers, fill_mem_obj_dispatcher_data* pDispatcherData, size_t uiDispatchSize) :
    TaskHandler<FillMemObjTask, fill_mem_obj_dispatcher_data >(lockBufferCount, pLockBuffers, pDispatcherData, uiDispatchSize),
    m_fillBufPtr((char*)(pLockBuffers[0])), m_fillBufPtrAnchor((char*)(pLockBuffers[0]))
{
    if ( lockBufferCount!=1 )
    {
        assert( 0 && "FillMemObjTask expects only single buffer" );
        setTaskError(CL_DEV_ERROR_FAIL);
    }
}

FillMemObjTask::FillMemObjTask( const FillMemObjTask& o) :
    TaskHandler<FillMemObjTask, fill_mem_obj_dispatcher_data >( o ),
    m_fillBufPtr(o.m_fillBufPtr), m_fillBufPtrAnchor(o.m_fillBufPtrAnchor)
{
}

bool FillMemObjTask::PrepareTask()
{
#ifdef ENABLE_MIC_TRACER
    // Set total buffers size and num of buffers for the tracer.
    commandTracer().add_delta_num_of_buffer_sent_to_device(m_lockBufferCount);
    unsigned long long bufSize = 0;
    for (unsigned int i = 0; i < m_lockBufferCount; i++)
    {
        bufSize = m_lockBufferLengths[i];
        commandTracer().add_delta_buffers_size_sent_to_device(bufSize);
    }
#endif

    return true;
}

void FillMemObjTask::Cancel()
{
    // Notify start if exists
    if ( NULL!= m_dispatcherData->startEvent.isRegistered )
    {
        COIEventSignalUserEvent(m_dispatcherData->startEvent.cmdEvent);
    }

#ifdef ENABLE_MIC_TRACER
    commandTracer().set_current_time_tbb_exe_in_device_time_start();
    commandTracer().set_current_time_tbb_exe_in_device_time_end();
#endif

    setTaskError(CL_DEV_COMMAND_CANCELLED);
    // Notify finish if exists
    if ( NULL!= m_dispatcherData->endEvent.isRegistered )
    {
        COIEventSignalUserEvent(m_dispatcherData->endEvent.cmdEvent);
    }
}

int FillMemObjTask::Init(size_t region[], unsigned int& regCount)
{
	// Notify start if exists
    if ( NULL!= m_dispatcherData->startEvent.isRegistered )
    {
        COIEventSignalUserEvent(m_dispatcherData->startEvent.cmdEvent);
    }

#ifdef ENABLE_MIC_TRACER
    commandTracer().set_current_time_tbb_exe_in_device_time_start();
#endif

	m_fillBufPtr = m_fillBufPtr + m_dispatcherData->from_offset;
	unsigned int tPatternSize = m_dispatcherData->pattern_size;
	assert((tPatternSize >= 1) && (tPatternSize <= MAX_PATTERN_SIZE));
	size_t tRegion = m_dispatcherData->vRegion[0];
	assert((tRegion > 0) && (tRegion % tPatternSize == 0));
	memcpy(m_patternToUse, m_dispatcherData->pattern, tPatternSize);
    assert(1 == m_dispatcherData->dim_count && "Fill buffer can be one dimension");
	regCount = 1;
	while (tPatternSize < MAX_PATTERN_SIZE)
	{
		memcpy(&((char*)m_patternToUse)[tPatternSize], &((char*)m_patternToUse)[0], tPatternSize);
		tPatternSize += tPatternSize;
	}
    const size_t alignRestriction = sizeof(double) * 8;
    assert(alignRestriction <= MAX_PATTERN_SIZE && "Assume that the req. alignment is smaller or equal to MAX_PATTERN_SIZE");
    //Set buffer allignment if needed.
    size_t numBytesCpyForAlignFix = 0;
    if ((tRegion >= (MAX_PATTERN_SIZE + alignRestriction)) && (0 != ((size_t)m_fillBufPtr & (size_t)(alignRestriction - 1))))
    {
        numBytesCpyForAlignFix = alignRestriction - ((size_t)m_fillBufPtr & (size_t)(alignRestriction - 1));
        assert(((numBytesCpyForAlignFix > 0) && (numBytesCpyForAlignFix < alignRestriction)));
        memcpy(m_fillBufPtr, m_patternToUse, numBytesCpyForAlignFix);
        // if the amount of copied bytes are not dividing by original size of the pattern than we should arrange the pattern.
        if (0 != numBytesCpyForAlignFix % m_dispatcherData->pattern_size)
        {
            memcpy(m_patternToUse, ((char*)m_patternToUse) + numBytesCpyForAlignFix, MAX_PATTERN_SIZE - numBytesCpyForAlignFix);
            memcpy(((char*)m_patternToUse) + (MAX_PATTERN_SIZE - numBytesCpyForAlignFix), m_fillBufPtr, numBytesCpyForAlignFix);
        }
        m_fillBufPtr += numBytesCpyForAlignFix;
        tRegion -= numBytesCpyForAlignFix;
    }
	// Do serial
	if ((tRegion < gMicExecEnvOptions.min_buffer_size_parallel_fill) || (tRegion < (MAX_PATTERN_SIZE + alignRestriction)))
	{
		m_serialExecution = true;
		tPatternSize = m_dispatcherData->pattern_size;

		if (tRegion < (MAX_PATTERN_SIZE + alignRestriction))
		{
			for (unsigned int i = 0; i < tRegion; i += tPatternSize)
			{
				memcpy(&(m_fillBufPtr[i]), m_dispatcherData->pattern, tPatternSize);
			}
		}
		else
		{
			int i = intrinCopy(0, tRegion);
			for (; i <= (tRegion - tPatternSize); i += tPatternSize)
			{
				memcpy(&(m_fillBufPtr[i]), m_dispatcherData->pattern, tPatternSize);
			}
            if (i < tRegion)
            {
                assert((tRegion - i) < tPatternSize);
                memcpy(&(m_fillBufPtr[i]), m_dispatcherData->pattern, tRegion - i);
            }
		}
		m_coveredSize = tRegion + numBytesCpyForAlignFix;
		// TODO change the return value
		return -1;
	}
	// Parallel fill path.
	unsigned int pow2NumWorkers = (tRegion <= USE_SINGLE_WORKER_FOR_BUFF_SIZE) ? 1 : (gMicExecEnvOptions.num_of_cores * gMicExecEnvOptions.threads_per_core);
	if (gMicExecEnvOptions.max_workers_fill_buffer > 0) 
	{
		pow2NumWorkers = (gMicExecEnvOptions.num_of_cores * gMicExecEnvOptions.threads_per_core > gMicExecEnvOptions.max_workers_fill_buffer) ? 
			gMicExecEnvOptions.max_workers_fill_buffer : (gMicExecEnvOptions.num_of_cores * gMicExecEnvOptions.threads_per_core);
	}
	// Find the nearest power of 2 number of num threads.
	pow2NumWorkers --;
	pow2NumWorkers |= pow2NumWorkers >> 1;
	pow2NumWorkers |= pow2NumWorkers >> 2;
	pow2NumWorkers |= pow2NumWorkers >> 4;
	pow2NumWorkers |= pow2NumWorkers >> 8;
	pow2NumWorkers |= pow2NumWorkers >> 16;
	pow2NumWorkers ++;

	assert((pow2NumWorkers >= 1) && ((tRegion <= USE_SINGLE_WORKER_FOR_BUFF_SIZE) || ((pow2NumWorkers >= gMicExecEnvOptions.num_of_cores * gMicExecEnvOptions.threads_per_core) && (pow2NumWorkers / 2 < gMicExecEnvOptions.num_of_cores * gMicExecEnvOptions.threads_per_core))));

	assert(tRegion >= MAX_PATTERN_SIZE);
	size_t tNumTasksPerWorker = tRegion / (MAX_PATTERN_SIZE * pow2NumWorkers);
	if (tNumTasksPerWorker < 1)
	{
		tNumTasksPerWorker = 1;
	}

	int preferedTasksPerWorker = gMicExecEnvOptions.max_tasks_per_worker_fill_buffer;
	if (preferedTasksPerWorker <= 0)
	{
		assert(4 == sizeof(m_tasksForWorkerConf) / sizeof(tasksForWorkerConfStruct));
		preferedTasksPerWorker = m_tasksForWorkerConf[3].numTasks;
		for (unsigned int i = 0; i < 3; ++i)
		{
			if (tRegion <= m_tasksForWorkerConf[i].buffSize)
			{
				preferedTasksPerWorker = m_tasksForWorkerConf[i].numTasks;
				break;
			}
		}
	}

	m_numIterationsPerWorker = (tNumTasksPerWorker > preferedTasksPerWorker) ? tNumTasksPerWorker / preferedTasksPerWorker : 1;

	region[0] = tRegion / (MAX_PATTERN_SIZE * m_numIterationsPerWorker);
	m_coveredSize = (region[0] * MAX_PATTERN_SIZE * m_numIterationsPerWorker) + numBytesCpyForAlignFix;
	
	return 0;
}

bool FillMemObjTask::ExecuteIteration(size_t x, size_t y, size_t z, void* data_from_AttachToThread)
{
    const int numBytesToCopy = MAX_PATTERN_SIZE * m_numIterationsPerWorker;
    const int numBytesCopied = intrinCopy(numBytesToCopy * x, numBytesToCopy * (x + 1));
    assert((numBytesToCopy == numBytesCopied) && "each worker should copy (MAX_PATTERN_SIZE * m_numIterationsPerWorker) bytes");
    return (numBytesCopied == numBytesToCopy);
}

bool FillMemObjTask::Finish(Intel::OpenCL::TaskExecutor::FINISH_REASON reason)
{
	const unsigned int tSizeRemain = m_dispatcherData->vRegion[0] - m_coveredSize;
	if (tSizeRemain > 0)
	{
		char* tFillBuffPtr = m_fillBufPtrAnchor + m_dispatcherData->from_offset + m_coveredSize;
		const unsigned int patternSize = m_dispatcherData->pattern_size;
		const char* pattern = (char*)m_patternToUse;
        unsigned int i = 0;
		for (; i <= (tSizeRemain - patternSize); i += patternSize)
		{
			memcpy(&(tFillBuffPtr[i]), pattern, patternSize);
		}
        if (i < tSizeRemain)
        {
            assert((tSizeRemain - i) < patternSize);
            memcpy(&(tFillBuffPtr[i]), pattern, tSizeRemain - i);
        }
	}

    // Release COI resources
    FiniTask();

#ifdef ENABLE_MIC_TRACER
    commandTracer().set_current_time_tbb_exe_in_device_time_end();
#endif

    // Notify finish if exists
    if ( NULL!= m_dispatcherData->endEvent.isRegistered )
    {
        COIEventSignalUserEvent(m_dispatcherData->endEvent.cmdEvent);
    }

    return m_serialExecution ? true : CL_DEV_SUCCEEDED( getTaskError() );
}
