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
    m_fillBufPtr((char*)(pLockBuffers[0]))
{
    if ( lockBufferCount!=1 )
    {
        assert( 0 && "FillMemObjTask expects only single buffer" );
        setTaskError(CL_DEV_ERROR_FAIL);
    }
}

FillMemObjTask::FillMemObjTask( const FillMemObjTask& o) :
    TaskHandler<FillMemObjTask, fill_mem_obj_dispatcher_data >( o ),
    m_fillBufPtr(o.m_fillBufPtr)
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

bool FillMemObjTask::Execute()
{
    // Notify start if exists
    if ( NULL!= m_dispatcherData->startEvent.isRegistered )
    {
        COIEventSignalUserEvent(m_dispatcherData->startEvent.cmdEvent);
    }

#ifdef ENABLE_MIC_TRACER
    commandTracer().set_current_time_tbb_exe_in_device_time_start();
#endif
    
    // Copy the arrived dispatcher data to memFillInfo which contain the fill mem info. 
    // TODO: Why need additional record?
    fill_mem_obj_dispatcher_data memFillInfo;
    memcpy(&memFillInfo, m_dispatcherData, sizeof(fill_mem_obj_dispatcher_data));
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
    // TODO: Why need to copy whole structure if only dim_count is changed
    //       Maybe pass it as separate paramter
    memcpy(&sRecParam, pMemFillInfo, sizeof(fill_mem_obj_dispatcher_data));
    sRecParam.dim_count = pMemFillInfo->dim_count - 1;

    // Make recursion
    for(unsigned int i=0; i < pMemFillInfo->vRegion[sRecParam.dim_count]; ++i)
    {
        executeInternal( buffPtr, &sRecParam, lastChunk, pattern);
        sRecParam.from_offset += pMemFillInfo->vFromPitch[sRecParam.dim_count-1];
    }
}
