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
    assert(nullptr != pQueue && "pQueue must be valid");

    cl_dev_err_code err = CL_DEV_SUCCESS;

    if (pQueue->IsAsyncExecution())
    {
        SharedPtr<FillMemObjTask> fillMemObjTask = FillMemObjTask::Allocate(in_BufferCount, in_ppBufferPointers, fillMemObjDispatchData, in_MiscDataLength, pQueue);

        err = fillMemObjTask->getTaskError();
        if ( CL_DEV_SUCCEEDED(err) )
        {
            err = pQueue->Execute(fillMemObjTask->GetAsTaskHandlerBase());
        }
	}
    else
    {
        FillMemObjTask fillMemObjTask(in_BufferCount, in_ppBufferPointers, fillMemObjDispatchData, in_MiscDataLength, pQueue);
        // Increase ref. count to prevent undesired deletion
        fillMemObjTask.IncRefCnt();
    
        err = fillMemObjTask.getTaskError();
        if ( CL_DEV_SUCCEEDED(err) )
        {
            err = pQueue->Execute(fillMemObjTask.GetAsTaskHandlerBase());
        }
    }

    *((cl_dev_err_code*)in_pReturnValue) = err;
}


FillMemObjTask::FillMemObjTask( uint32_t lockBufferCount, void** pLockBuffers, fill_mem_obj_dispatcher_data* pDispatcherData, size_t uiDispatchSize, QueueOnDevice* pQueue) :
    TaskHandler<FillMemObjTask, fill_mem_obj_dispatcher_data >(lockBufferCount, pLockBuffers, pDispatcherData, uiDispatchSize, pQueue),
    m_fillBufPtr((char*)(pLockBuffers[0])), m_fillBufPtrAnchor((char*)(pLockBuffers[0]))
#ifdef USE_ITT
    ,m_pIttFillBufferName(nullptr), m_pIttFillBufferDomain(nullptr)
#endif
{
    if ( lockBufferCount!=1 )
    {
        assert( 0 && "FillMemObjTask expects only single buffer" );
        setTaskError(CL_DEV_ERROR_FAIL);
    }
#ifdef USE_ITT
    if ( gMicGPAData.bUseGPA)
    {
	    m_pIttFillBufferDomain = __itt_domain_create("com.intel.opencl.device.mic.fill_mem_obj");
		// Use fillBuffer specific domain if possible, if not available switch to global domain
        if ( nullptr == m_pIttFillBufferDomain )
        {
            m_pIttFillBufferDomain = gMicGPAData.pDeviceDomain;
        }
		m_pIttFillBufferName = __itt_string_handle_create("FillMemObj");
    }
#endif
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

	m_bufferPointers = nullptr;

    return true;
}

void FillMemObjTask::Cancel()
{
    m_pQueue->CancelTask(this);
#ifdef USE_ITT
    if ( gMicGPAData.bUseGPA)
    {
        __itt_task_end(m_pIttFillBufferDomain);
    }
#endif
}

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
static __thread unsigned int master_id = 0;
#endif

int FillMemObjTask::Init(size_t region[], unsigned int& regCount)
{
#if defined(USE_ITT)
    if ( gMicGPAData.bUseGPA )
    {
        __itt_frame_begin_v3(m_pIttFillBufferDomain, nullptr);
    }
#endif

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( gMicGPAData.bUseGPA )
    {
      static __thread __itt_string_handle* pTaskName = nullptr;
      if ( nullptr == pTaskName )
      {
        pTaskName = __itt_string_handle_create("FillMemObjTask::Init()");
      }
      __itt_task_begin(gMicGPAData.pDeviceDomain, __itt_null, __itt_null, pTaskName);
    }
#endif
	// Notify start if exists
    if ( m_pDispatcherData->startEvent.isRegistered )
    {
        COIEventSignalUserEvent(m_pDispatcherData->startEvent.cmdEvent);
    }

#ifdef ENABLE_MIC_TRACER
    commandTracer().set_current_time_tbb_exe_in_device_time_start();
#endif

    m_fillBufPtr = m_fillBufPtr + m_pDispatcherData->from_offset;
    unsigned int tPatternSize = m_pDispatcherData->pattern_size;
    assert((tPatternSize >= 1) && (tPatternSize <= MAX_PATTERN_SIZE));
    size_t tRegion = m_pDispatcherData->vRegion[0];
    assert((tRegion > 0) && (tRegion % tPatternSize == 0));
    memcpy(m_patternToUse, m_pDispatcherData->pattern, tPatternSize);
    assert(1 == m_pDispatcherData->dim_count && "Fill buffer can be one dimension");
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
        if (0 != numBytesCpyForAlignFix % m_pDispatcherData->pattern_size)
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
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
        if ( gMicGPAData.bUseGPA )
        {
           __itt_task_end(gMicGPAData.pDeviceDomain); //"FillMemObjTask::Init()"
        }
#endif

#ifdef USE_ITT
        if ( gMicGPAData.bUseGPA)
        {
          __itt_task_begin(m_pIttFillBufferDomain, __itt_null, __itt_null, m_pIttFillBufferName);
        }
#endif

		m_serialExecution = true;
		tPatternSize = m_pDispatcherData->pattern_size;

		if (tRegion < (MAX_PATTERN_SIZE + alignRestriction))
		{
			for (unsigned int i = 0; i < tRegion; i += tPatternSize)
			{
				memcpy(&(m_fillBufPtr[i]), m_pDispatcherData->pattern, tPatternSize);
			}
		}
		else
		{
			int i = intrinCopy(0, tRegion);
			for (; i <= (tRegion - tPatternSize); i += tPatternSize)
			{
				memcpy(&(m_fillBufPtr[i]), m_pDispatcherData->pattern, tPatternSize);
			}
            if (i < tRegion)
            {
                assert((tRegion - i) < tPatternSize);
                memcpy(&(m_fillBufPtr[i]), m_pDispatcherData->pattern, tRegion - i);
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

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( gMicGPAData.bUseGPA )
    {
         __itt_task_end(gMicGPAData.pDeviceDomain); //"FillMemObjTask::Init()"
    }
#endif

#ifdef USE_ITT
    if ( gMicGPAData.bUseGPA)
    {
        __itt_task_begin(m_pIttFillBufferDomain, __itt_null, __itt_null, m_pIttFillBufferName);
    }
#endif

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( gMicGPAData.bUseGPA )
    {
      static __thread __itt_string_handle* pTaskName = nullptr;
      master_id = GetThreadId();
      if ( nullptr == pTaskName )
      {
        pTaskName = __itt_string_handle_create("TBB::Distribute_Work");
      }
      __itt_task_begin(gMicGPAData.pDeviceDomain, __itt_null, __itt_null, pTaskName);
    }
#endif
	
	return 0;
}

bool FillMemObjTask::ExecuteIteration(size_t x, size_t y, size_t z, void* data_from_AttachToThread)
{
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    // Monitor only IN-ORDER queue
    if ( (gMicGPAData.bUseGPA) && (master_id==GetThreadId()) )
    {
        __itt_task_end(gMicGPAData.pDeviceDomain); // End of "TBB::Distribute_Work"
        // notify only once
        master_id = 0;
    }
#endif

#ifdef USE_ITT
    if ( gMicGPAData.bUseGPA )
    {
        __itt_task_begin(m_pIttFillBufferDomain, __itt_null, __itt_null, m_pIttFillBufferName);
    }
#endif
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( gMicGPAData.bUseGPA )
    {
        static __thread __itt_string_handle* pTaskName = nullptr;
        if ( nullptr == pTaskName )
        {
            pTaskName = __itt_string_handle_create("FillMemObjTask::ExecuteIteration()");
        }
        __itt_task_begin(gMicGPAData.pDeviceDomain, __itt_null, __itt_null, pTaskName);
    }
#endif

    const int numBytesToCopy = MAX_PATTERN_SIZE * m_numIterationsPerWorker;
    const int numBytesCopied = intrinCopy(numBytesToCopy * x, numBytesToCopy * (x + 1));
    assert((numBytesToCopy == numBytesCopied) && "each worker should copy (MAX_PATTERN_SIZE * m_numIterationsPerWorker) bytes");

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    // Monitor only IN-ORDER queue
    if ( gMicGPAData.bUseGPA )
    {
      __itt_task_end(gMicGPAData.pDeviceDomain); // "FillMemObjTask::ExecuteIteration()"
    }
#endif
#ifdef USE_ITT
    if ( gMicGPAData.bUseGPA)
    {
        __itt_task_end(m_pIttFillBufferDomain);
    }
#endif

    return (numBytesCopied == numBytesToCopy);
}

bool FillMemObjTask::Finish(Intel::OpenCL::TaskExecutor::FINISH_REASON reason)
{
    const unsigned int tSizeRemain = m_pDispatcherData->vRegion[0] - m_coveredSize;
    if (tSizeRemain > 0)
    {
        char* tFillBuffPtr = m_fillBufPtrAnchor + m_pDispatcherData->from_offset + m_coveredSize;
        const unsigned int patternSize = m_pDispatcherData->pattern_size;
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

    m_pQueue->FinishTask( this );

#ifdef USE_ITT
    if ( gMicGPAData.bUseGPA)
    {
        __itt_task_end(m_pIttFillBufferDomain);
        __itt_frame_end_v3(m_pIttFillBufferDomain, nullptr);
    }
#endif

    return m_serialExecution ? true : CL_DEV_SUCCEEDED( getTaskError() );
}
