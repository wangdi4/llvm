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

#include "buffer_commands.h"
#include "memory_allocator.h"
#include "command_list.h"

#include <cl_sys_info.h>
#include <cl_types.h>
#include <cl_utils.h>

#include <source/COIBuffer_source.h>
#include <source/COIProcess_source.h>


#include <cstring>

using namespace Intel::OpenCL::MICDevice;

const unsigned int MAX_DEPENDENCIES_ARRAY_COUNT = (unsigned int)(4096 / sizeof(COIEVENT));

template <class T>
void ProcessMemoryChunk<T>::process_finish( void )
{
    // current chunk may not be ready to fire, only when no work fired at all
    if (false == m_current_chunk.isReadyToFire())
    {
        assert( (!work_dispatched()) && "current chunk may be ready to fire, false == m_current_chunk.isReadyToFire() means that no work fired at all" );
        return;
    }

    // Fire current chunk and force dependency on all accumulated events.
    fire_current_chunk( true );
}

template <class T>
void ProcessMemoryChunk<T>::fire_current_chunk( bool force )
{
    //
    // State machine:
    //    NOT_INIT          - still no chunk was fired to COI
    //    SINGLE_CHUNK_MODE - single chunk was fired to COI, dependency array not reserved
    //    MULTI_CHUNK_MODE  - at least 2 chunks fired to COI, dependency array already reserved
    //
    // In all states additional optional external dependency may exists, that should be added to each
    // COI command invocation
    //

    COIEVENT* use_dependencies = NULL;
    uint32_t  num_dependencies = 0;
    bool      flush_dependencies = false;

    COIEVENT  double_dependency[2];

    if (m_error_occured)
    {
        return;
    }

    assert( (true == m_current_chunk.isReadyToFire()) && "This code might not me called with non ready to fire chunk - it should be screened before!");

    switch (m_state)
    {
        case NOT_INIT:
            m_state = SINGLE_CHUNK_MODE;
            break;

        case SINGLE_CHUNK_MODE:
            // single chunk already fired
            if (force)
            {
                // it's just a second and the last chunk
                if (NULL == m_external_dependency)
                {
                    num_dependencies = 1;
                    use_dependencies = &m_last_dependency;
                }
                else
                {
                    double_dependency[0] = *m_external_dependency;
                    double_dependency[1] = m_last_dependency;
                    num_dependencies = 2;
                    use_dependencies = double_dependency;
                }
            }
            else
            {
                // it's a second but not last chunk - prepare for more
                m_dependencies.reserve( MAX_DEPENDENCIES_ARRAY_COUNT );

                if (NULL != m_external_dependency)
                {
                    m_dependencies.push_back( *m_external_dependency );
                }
                m_dependencies.push_back( m_last_dependency );
                m_state = MULTI_CHUNK_MODE;
            }
            break;

        case MULTI_CHUNK_MODE:
            if (force || (MAX_DEPENDENCIES_ARRAY_COUNT >= m_dependencies.size()))
            {
                use_dependencies = &(m_dependencies[0]);
                num_dependencies = m_dependencies.size();
                flush_dependencies = true;
            }
            break;

        default:
            assert( 0 && "Unknown state" );
    }

    COIEVENT fired_event;

    bool ok;

    m_total_chunks_processed ++;
    m_total_size_processed += m_current_chunk.getSize();

    if (NULL != use_dependencies)
    {
        ok = fire_action( m_current_chunk, use_dependencies, num_dependencies, &fired_event );
    }
    else
    {
        ok = fire_action( m_current_chunk,
                          m_external_dependency, (NULL == m_external_dependency) ? 0 : 1,
                          &fired_event );
    }

    // Nullify NotificationCallBack context such that the next command will not set this command context. (Actually it is in order to avoid many changes in command status in case of rectangular operation,
    // We want to set context to the first and the last command only.
    Command::unregisterProfilingContext();

    if (!ok)
    {
        m_error_occured = true;
        return;
    }

    // override last dependency. In the case of SINGLE_CHUNK_MODE with forcing - override used dependency
    m_last_dependency = fired_event;

    if (MULTI_CHUNK_MODE == m_state)
    {
        if (flush_dependencies)
        {
            // remove all dependencies - we already used them
            m_dependencies.clear();
            if (NULL != m_external_dependency)
            {
                m_dependencies.push_back( *m_external_dependency );
            }
        }

        m_dependencies.push_back( fired_event );
    }

    // ensure not firing current chunk twice
    m_current_chunk.reset();
}


void ProcessCommonMemoryChunk::process_chunk( CommonMemoryChunk::Chunk& chunk )
{
    if (error_occured())
    {
        return;
    }

    if (0 == chunk.size)
    {
        return;
    }

    // optimization - try to join chunks before firing COI commands
    if (((m_current_chunk.from_offset + m_current_chunk.size) == chunk.from_offset) &&
        ((m_current_chunk.to_offset   + m_current_chunk.size) == chunk.to_offset))
    {
        m_current_chunk.size += chunk.size;
        return;
    }

    // Do not fire the last chunk when first calling to this method because the chunk value is set after this call.
    if (m_readyToFireChunk)
    {
        // cannot join chunks - need to fire
        fire_current_chunk( false );
    }

    m_readyToFireChunk = true;

    // update the current chunk to be the new chunk. (After firing the old one)
    memcpy(&m_current_chunk, &chunk, sizeof(CommonMemoryChunk::Chunk));
}

bool ProcessCommonMemoryChunk::processActionOptimized(cl_dev_cmd_type type, void* readBuff, size_t readOffset, void* writeBuff, size_t writeOffset, size_t size,
    const COIEVENT* dependecies, uint32_t num_dependencies, COIEVENT* fired_event, bool forceValidOnSingleDevice)
{
    COIRESULT coi_result = COI_SUCCESS;
    vector<COIEVENT> setStateEventsArr;
    if ((CL_DEV_CMD_READ == type) /* TODO Remove '|| (CL_DEV_CMD_COPY == type)' when COI will fix the deadlock */ || (CL_DEV_CMD_COPY == type))
    {
        forceValidOnSingleDevice = false;
    }
    // Force validity on target device and invalidate on other devices.
    if (forceValidOnSingleDevice)
    {
        MICDevMemoryObject* pWriteMemObj = static_cast<MICDevMemoryObject*>(writeBuff);
        assert(pWriteMemObj && "Only Write and Copy commands can force transfer, so the writeBuff must be of type MICDevMemoryObject");
        if (NULL == pWriteMemObj)
        {
            return false;
        }
        if (pWriteMemObj->isRootBuffer())    /* TODO Remove the 'if' when COI will fix the sub-buffers set buffer state issue */
        {
            // In case of overwriting the whole buffer, than we should not move the data.
            COI_BUFFER_MOVE_FLAG moveFlag = ((0 == writeOffset) && (size == pWriteMemObj->GetRawDataSize())) ? COI_BUFFER_NO_MOVE : COI_BUFFER_MOVE;
            assert(m_processOfTarget && "In case of Write / Copy commands, m_processOfTarget must be set");
            vector<COIPROCESS> targetBuffProcesses = pWriteMemObj->get_active_processes();
            // targetBuffProcesses.size() must be greater than 0, becasue it exist at least in the current queue MIC device.
            assert(targetBuffProcesses.size() > 0 && "targetBuffProcesses must be greater than 0");
            if (0 == targetBuffProcesses.size())
            {
                return false;
            }
            targetBuffProcesses.push_back(COI_PROCESS_SOURCE);
            setStateEventsArr.resize(targetBuffProcesses.size());
            const COIBUFFER& targetCoiBuffer = pWriteMemObj->clDevMemObjGetCoiBufferHandler();
            assert(m_processOfTarget != NULL && m_processOfTarget != COI_PROCESS_SOURCE && "m_processOfTarget must be MIC Device process");
            // Set target buffer valid on m_processOfTarget.
            coi_result = COIBufferSetState( 
                                                    targetCoiBuffer,                    // Buffer to transfer
                                                    m_processOfTarget,                    // Target Device process 
                                                    COI_BUFFER_VALID,                    // Desired state in the target process
                                                    moveFlag,                            // Force data movement if required
                                                    num_dependencies, dependecies,        // array of dependencies
                                                    &(setStateEventsArr[0]) );
            assert(COI_SUCCESS == coi_result && "COIBufferSetState failed");
            if (COI_SUCCESS != coi_result)
            {
                return false;
            }
            // Set target buffer invalid on all devices that are not m_processOfTarget
            unsigned int eventsArrIndex = 1;
            for (unsigned int i = 0; i < targetBuffProcesses.size(); i++)
            {
                if (m_processOfTarget != targetBuffProcesses[i])
                {
                    coi_result = COIBufferSetState( 
                                                    targetCoiBuffer,                    // Buffer to transfer
                                                    targetBuffProcesses[i],                // Target Device process 
                                                    COI_BUFFER_INVALID,                    // Desired state in the target process
                                                    COI_BUFFER_NO_MOVE,                    // Force data movement if required
                                                    1, &(setStateEventsArr[0]),            // array of dependencies
                                                    &(setStateEventsArr[eventsArrIndex]) );
                
                    assert(COI_SUCCESS == coi_result && "COIBufferSetState failed");
                    if (COI_SUCCESS != coi_result)
                    {
                        assert(eventsArrIndex > 0 && "eventsArrIndex must be greater than 0 because the prev. SetBufferState");
                        *fired_event = setStateEventsArr[eventsArrIndex - 1];
                        return false;
                    }
                    eventsArrIndex ++;
                }
            }
            assert(eventsArrIndex == setStateEventsArr.size() && "eventsArrIndex must be as the size of the ALL COIProcesses");
        }
    }
    COIEVENT* tDependecies = (COIEVENT*)dependecies;
    uint32_t tNumDependencies = num_dependencies;
    if (setStateEventsArr.size() > 0)
    {
        tDependecies = &(setStateEventsArr[0]);
        tNumDependencies = setStateEventsArr.size();
    }
    switch (type)
    {
    case CL_DEV_CMD_READ:
        {
            if (NULL == m_memObjOfHostPtr)
            {
                if (false == MICDevMemoryObject::getMemObjFromMapBuffersPool(writeBuff, size, &m_memObjOfHostPtr))
                {
                    m_memObjOfHostPtr = NULL;
                }
            }
            // If we found OCLBuffer that writeBuff is mapped to...
            if (m_memObjOfHostPtr) 
            {
                void* memObjPtr = m_memObjOfHostPtr->clDevMemObjGetDescriptorRaw().pData;
                size_t memObjSize = m_memObjOfHostPtr->GetRawDataSize();
                if (((size_t)writeBuff + writeOffset >= (size_t)memObjPtr) && ((size_t)writeBuff + writeOffset + size <= (size_t)memObjPtr + memObjSize))
                {
                    // Can convert the read operation to COPY operation. (The Buffer 'm_memObjOfTargetPtr' mapped to the host so it must exist in the host)
                    return processActionOptimized(CL_DEV_CMD_COPY, readBuff, readOffset, m_memObjOfHostPtr, writeOffset + ((size_t)writeBuff - (size_t)memObjPtr), size, tDependecies, tNumDependencies, fired_event, false);
                }
            }
            // else execute regular read
            MICDevMemoryObject* pReadMemObj = static_cast<MICDevMemoryObject*>(readBuff);
            assert(pReadMemObj && "In Read Command, readBuff must be of type MICDevMemoryObject");
            if (NULL == pReadMemObj)
            {
                return false;
            }
            coi_result = COIBufferRead (pReadMemObj->clDevMemObjGetCoiBufferHandler(),        // Buffer to read from.
                                        readOffset,                                            // Location in the buffer to start reading from.
                                        (void*)((size_t)writeBuff + writeOffset),            // A pointer to local memory that should be written into.
                                        size,                                                // The number of bytes to write from coiBuffer into host
                                        COI_COPY_UNSPECIFIED,                                // The type of copy operation to use. (//TODO check option to change the type in order to improve performance)
                                        tNumDependencies,                                    // The number of dependencies specified.
                                        tDependecies,                                        // An optional array of handles to previously created COIEVENT objects that this read operation will wait for before starting.
                                        fired_event                                            // An optional event to be signaled when the copy has completed.
                                        );
            
            assert(COI_SUCCESS == coi_result && "COIBufferRead failed");
            if (COI_SUCCESS != coi_result)
            {
                return false;
            }
            break;
        }
    case CL_DEV_CMD_WRITE:
        {
            if (NULL == m_memObjOfHostPtr)
            {
                if (false == MICDevMemoryObject::getMemObjFromMapBuffersPool(readBuff, size, &m_memObjOfHostPtr))
                {
                    m_memObjOfHostPtr = NULL;
                }
            }
            // If we found OCLBuffer that readBuff is mapped to...
            if (m_memObjOfHostPtr) 
            {
                void* memObjPtr = m_memObjOfHostPtr->clDevMemObjGetDescriptorRaw().pData;
                size_t memObjSize = m_memObjOfHostPtr->GetRawDataSize();
                if (((size_t)readBuff + readOffset >= (size_t)memObjPtr) && ((size_t)readBuff + readOffset + size <= (size_t)memObjPtr + memObjSize))
                {
                    // Can convert the write operation to COPY operation.
                    return processActionOptimized(CL_DEV_CMD_COPY, m_memObjOfHostPtr, readOffset + ((size_t)readBuff - (size_t)memObjPtr), writeBuff, writeOffset, size, tDependecies, tNumDependencies, fired_event, false);
                }
            }
            // else execute regular write
            MICDevMemoryObject* pWriteMemObj = static_cast<MICDevMemoryObject*>(writeBuff);
            assert(pWriteMemObj && "In Write Command, writeBuff must be of type MICDevMemoryObject");
            if (NULL == pWriteMemObj)
            {
                if (setStateEventsArr.size() > 0)
                {
                    *fired_event = setStateEventsArr[setStateEventsArr.size() - 1];
                }
                return false;
            }
            coi_result = COIBufferWrite (pWriteMemObj->clDevMemObjGetCoiBufferHandler(),    // Buffer to write to.
                                         writeOffset,                                        // Location in the buffer to start writing from.
                                         (void*)((size_t)readBuff + readOffset),            // A pointer to local memory that should be read from.
                                         size,                                                // The number of bytes to write from host into coiBuffer.
                                         COI_COPY_UNSPECIFIED,                                // The type of copy operation to use. (//TODO check option to change the type in order to improve performance)
                                         tNumDependencies,                                    // The number of dependencies specified.
                                         tDependecies,                                        // An optional array of handles to previously created COIEVENT objects that this write operation will wait for before starting.
                                         fired_event                                        // An optional event to be signaled when the copy has completed.
                                        );

            assert(COI_SUCCESS == coi_result && "COIBufferWrite failed");
            if (COI_SUCCESS != coi_result)
            {
                if (setStateEventsArr.size() > 0)
                {
                    *fired_event = setStateEventsArr[setStateEventsArr.size() - 1];
                }
                return false;
            }
            break;
        }
    case CL_DEV_CMD_COPY:
        {
            MICDevMemoryObject* pWriteMemObj = static_cast<MICDevMemoryObject*>(writeBuff);
            MICDevMemoryObject* pReadMemObj = static_cast<MICDevMemoryObject*>(readBuff);
            assert(pWriteMemObj && pReadMemObj && "In Copy Command, both pReadMemObj and writeBuff must be of type MICDevMemoryObject");
            if ((NULL == pWriteMemObj) || (NULL == pReadMemObj))
            {
                if (setStateEventsArr.size() > 0)
                {
                    *fired_event = setStateEventsArr[setStateEventsArr.size() - 1];
                }
                return false;
            }
            coi_result = COIBufferCopy (
                                        pWriteMemObj->clDevMemObjGetCoiBufferHandler(),         // Buffer to copy into.
                                        pReadMemObj->clDevMemObjGetCoiBufferHandler(),            // Buffer to copy from.
                                        writeOffset,                                            // Location in the destination buffer to start writing to.
                                        readOffset,                                                // Location in the source buffer to start reading from.
                                        size,                                                    // The number of bytes to copy from coiBufferSrc into coiBufferSrc.
                                        COI_COPY_UNSPECIFIED,                                    // The type of copy operation to use.
                                        tNumDependencies,                                        // The number of dependencies.
                                        tDependecies,                                            // An optional array of handles to previously created COIEVENT objects that this copy operation will wait for before starting.
                                        fired_event                                                // An optional event to be signaled when the copy has completed.
                                        );
            assert(COI_SUCCESS == coi_result && "COIBufferCopy failed");
            if (COI_SUCCESS != coi_result)
            {
                if (setStateEventsArr.size() > 0)
                {
                    *fired_event = setStateEventsArr[setStateEventsArr.size() - 1];
                }
                return false;
            }
            break;
        }
    default:
        assert(0 && "Error Unknow command_type\n");
    }
    return true;
}


void ProcessUnmapMemoryChunk::process_chunk( UnmapMemoryChunkStruct::Chunk& chunk )
{
    if (error_occured())
    {
        return;
    }

    if (NULL == chunk.coi_map_instance)
    {
        return;
    }

    // Do not fire the last chunk when first calling to this method because the chunk value is set after this call.
    if (m_readyToFireChunk)
    {
        // cannot join chunks - need to fire
        fire_current_chunk( false );
    }

    m_readyToFireChunk = true;

    // update the current chunk to be the new chunk. (After firing the old one)
    m_current_chunk.coi_map_instance = chunk.coi_map_instance;
}



BufferCommands::BufferCommands(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd) : Command(pCommandList, pFrameworkCallBacks, pCmd)
{
}

BufferCommands::~BufferCommands()
{
}

void BufferCommands::eventProfilingCall(COI_NOTIFICATIONS& type)
{
    switch (type)
    {
    case RUN_FUNCTION_START:
        assert(0 && "This case should be implemented in Command object");
    case BUFFER_OPERATION_READY:
        // Set end coi execution time for the tracer. (COI RUNNING)
#ifdef ENABLE_MIC_TRACER
        m_commandTracer.set_current_time_coi_execution_time_start();
#endif
        if ((m_pCmd->profiling) && (m_cmdRunningTime == 0))
        {
            m_cmdRunningTime = HostTime();
        }
        break;
    case BUFFER_OPERATION_COMPLETE:
        // Set end coi execution time for the tracer. (COI COMPLETED)
#ifdef ENABLE_MIC_TRACER
        m_commandTracer.set_current_time_coi_execution_time_end();
#endif
        if (m_pCmd->profiling) 
        {
            m_cmdCompletionTime = HostTime();
        }
        break;
    case RUN_FUNCTION_COMPLETE:
    case USER_EVENT_SIGNALED:
    case RUN_FUNCTION_READY:
        assert(0 && "This case should be implemented in Command object");
    default:
        assert(0 && "Unknow COI_NOTIFICATIONS type");
    };
}

void BufferCommands::CopyRegion(mem_copy_info_struct* pMemCopyInfo, ProcessCommonMemoryChunk* chunk_consumer )
{
    // Set this object to be the context when notifying for event status change. (For the first command in case of rectangular operation)
    registerProfilingContext();
    CopyRegionInternal( pMemCopyInfo, chunk_consumer );
    unregisterProfilingContext();

    // Set this object to be the context when notifying for event status change. (For the last command in case of rectangular operation, or for first and last command in case of regular operatiron)
    registerProfilingContext();
    chunk_consumer->process_finish();
    unregisterProfilingContext();
}

void BufferCommands::CopyRegionInternal( mem_copy_info_struct* pMemCopyInfo, ProcessCommonMemoryChunk* chunk_consumer )
{
    // Leaf case 1D array only
    if ( 1 == pMemCopyInfo->uiDimCount )
    {
        CommonMemoryChunk::Chunk tChunk(pMemCopyInfo->from_Offset, pMemCopyInfo->to_Offset, pMemCopyInfo->vRegion[0]);
        chunk_consumer->process_chunk( tChunk );
        return;
    }

    mem_copy_info_struct sRecParam;

    // Copy current parameters
    memcpy(&sRecParam, pMemCopyInfo, sizeof(mem_copy_info_struct));
    sRecParam.uiDimCount = pMemCopyInfo->uiDimCount-1;

    // Make recursion
    for(unsigned int i=0; i<pMemCopyInfo->vRegion[sRecParam.uiDimCount]; ++i)
    {
        CopyRegionInternal( &sRecParam, chunk_consumer);
        sRecParam.from_Offset += pMemCopyInfo->vFromPitch[sRecParam.uiDimCount-1];
        sRecParam.to_Offset   += pMemCopyInfo->vToPitch[sRecParam.uiDimCount-1];
    }
}

COIEVENT BufferCommands::ForceTransferToDevice( const MICDevMemoryObject* mem_obj, COIEVENT& last_chunk_event )
{
    if (false == mem_obj->ImmediateTransferForced())
    {
        return last_chunk_event;
    }

    COIEVENT transfer_event;

    registerProfilingContext();

    COIRESULT coi_result = COIBufferSetState( 
                        mem_obj->clDevMemObjGetCoiBufferHandler(),  // Buffer to transfer
                        m_pCommandList->getDeviceProcess(),         // Target Device process 
                        COI_BUFFER_VALID,                           // Desired state in the target process
                        COI_BUFFER_MOVE,                            // Force data movement if required
                        1, &last_chunk_event,                       // array of dependencies
                        &transfer_event );                          // event to be signaled on completion

    assert( (COI_SUCCESS == coi_result) && "Wrong params for COIBufferSetState" );

    unregisterProfilingContext();

    // this is an optimization - if failed, nothing to do
    return ((COI_SUCCESS == coi_result) ? transfer_event : last_chunk_event);
}

class ReadWriteMemoryChunk : public ProcessCommonMemoryChunk
{
public:
    
    ReadWriteMemoryChunk( const COIEVENT* external, MICDevMemoryObject* memObj, void* ptr, bool is_read, COIPROCESS processOfTarget ) :
        ProcessCommonMemoryChunk( external, processOfTarget ),
        m_memObj(memObj), m_ptr( (char*)ptr ), m_is_read_mode(is_read) {};

protected:

    bool fire_action( const CommonMemoryChunk::Chunk& chunk, const COIEVENT* dependecies, uint32_t num_dependencies, COIEVENT* fired_event );

private:
    MICDevMemoryObject* m_memObj;
    char*     m_ptr;
    bool      m_is_read_mode;
};

bool ReadWriteMemoryChunk::fire_action( const CommonMemoryChunk::Chunk& chunk, const COIEVENT* dependecies, uint32_t num_dependencies, COIEVENT* fired_event )
{
    if ( m_is_read_mode )
    {
        return processActionOptimized(CL_DEV_CMD_READ, m_memObj, chunk.from_offset, m_ptr, chunk.to_offset, chunk.size, dependecies, num_dependencies, fired_event);
    }
    else
    {
        return processActionOptimized(CL_DEV_CMD_WRITE, m_ptr, chunk.from_offset, m_memObj, chunk.to_offset, chunk.size, dependecies, num_dependencies, fired_event);
    }
}

ReadWriteMemObject::ReadWriteMemObject(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd) : BufferCommands(pCommandList, pFrameworkCallBacks, pCmd), m_memObjOfHostPtr(NULL)
{
}

ReadWriteMemObject::~ReadWriteMemObject()
{
    // If I convert the Read / Write command to Copy with the mem object m_memObjOfHostPtr, than now I can return it back.
    if (m_memObjOfHostPtr)
    {
        m_memObjOfHostPtr->returnMemObjToMapBuffersPool();
    }
}

cl_dev_err_code ReadWriteMemObject::Create(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd, SharedPtr<Command>& pOutCommand)
{
    return verifyCreation(new ReadWriteMemObject(pCommandList, pFrameworkCallBacks, pCmd), pOutCommand);
}

cl_dev_err_code ReadWriteMemObject::execute()
{

    cl_dev_cmd_param_rw*    cmdParams = (cl_dev_cmd_param_rw*)m_pCmd->params;
    MICDevMemoryObject*     pMicMemObj;
    mem_copy_info_struct    sCpyParam;

    cl_dev_err_code err = cmdParams->memObj->clDevMemObjGetDescriptor(CL_DEVICE_TYPE_ACCELERATOR, 0, (cl_dev_memobj_handle*)&pMicMemObj);
    if (CL_DEV_SUCCESS != err)
    {
        return err;
    }

    bool error = false;

    do {
        COIEVENT barrier;
        unsigned int numDependecies = 0;
        m_pCommandList->getLastDependentBarrier(&barrier, &numDependecies, false);

        COIEVENT* pBarrier = (numDependecies == 0) ? NULL : &barrier;

        assert( (numDependecies <= 1) && "Previous command list dependencies may not be more than 1" );

        if (numDependecies > 1)
        {
            m_lastError = CL_DEV_NOT_SUPPORTED;
            break;
        }

        const cl_mem_obj_descriptor& pMemObj = pMicMemObj->clDevMemObjGetDescriptorRaw();

        size_t offset;
        const size_t* pObjPitchPtr = cmdParams->memobj_pitch[0] ? cmdParams->memobj_pitch : pMemObj.pitch;

        // copy the dimension value
        sCpyParam.uiDimCount = cmdParams->dim_count;
        offset = MemoryAllocator::CalculateOffset(sCpyParam.uiDimCount, cmdParams->origin, pObjPitchPtr, pMemObj.uiElementSize);

        // Set region
        memcpy(sCpyParam.vRegion, cmdParams->region, sizeof(sCpyParam.vRegion));
        sCpyParam.vRegion[0] = cmdParams->region[0] * pMemObj.uiElementSize;

        // In case the pointer parameter (Host pointer) has pitch properties,
        // we need to consider that too.
        size_t ptrOffset =    cmdParams->ptr_origin[2] * cmdParams->pitch[1] + \
                            cmdParams->ptr_origin[1] * cmdParams->pitch[0] + \
                            cmdParams->ptr_origin[0];

        ReadWriteMemoryChunk copier(
                            pBarrier,
                            pMicMemObj, 
                            cmdParams->ptr,
                            ( CL_DEV_CMD_READ == m_pCmd->type ),
                            ( CL_DEV_CMD_READ == m_pCmd->type ) ? NULL : m_pCommandList->getDeviceProcess());

        if ( CL_DEV_CMD_READ == m_pCmd->type )
        {
#ifdef ENABLE_MIC_TRACER
            // Set command type for the tracer.
            m_commandTracer.set_command_type((char*)"Read");
#endif
            // set coiBuffer (objPtr) initial offset
            sCpyParam.from_Offset = offset;
            memcpy(sCpyParam.vFromPitch, pObjPitchPtr, sizeof(sCpyParam.vFromPitch));

            // set host pointer with the calculated offset and copy the pitch
            sCpyParam.to_Offset = ptrOffset;
            memcpy(sCpyParam.vToPitch, cmdParams->pitch, sizeof(sCpyParam.vToPitch));
        }
        else
        {
#ifdef ENABLE_MIC_TRACER
            // Set command type for the tracer.
            m_commandTracer.set_command_type((char*)"Write");
#endif
            // set host pointer with the calculated offset and copy the pitch
            sCpyParam.from_Offset = ptrOffset;
            memcpy(sCpyParam.vFromPitch, cmdParams->pitch, sizeof(sCpyParam.vFromPitch));

            // set coiBuffer (objPtr) initial offset
            sCpyParam.to_Offset = offset;
            memcpy(sCpyParam.vToPitch, pObjPitchPtr, sizeof(sCpyParam.vToPitch));
        }

#ifdef ENABLE_MIC_TRACER
        // Set start coi execution time for the tracer.
        m_commandTracer.set_current_time_coi_enqueue_command_time_start();
#endif
        CopyRegion( &sCpyParam, &copier );

        m_memObjOfHostPtr = copier.getUsedMemObjOfHostPtr();

        if (copier.error_occured())
        {
            m_lastError = CL_DEV_ERROR_FAIL;
            break;
        }

        if (!copier.work_dispatched())
        {
            m_lastError = CL_DEV_SUCCESS;
            error = true;
            break;
        }

#ifdef ENABLE_MIC_TRACER
        // Set total amount of buffer operations for the Tracer.
        unsigned int amount = copier.get_total_amount_of_chunks();
        m_commandTracer.add_delta_num_of_buffer_operations(amount);
        // Set total size of buffer operations for the Tracer.
        unsigned long long size = copier.get_total_memory_processed_size();
        m_commandTracer.add_delta_buffer_operation_overall_size(size);
#endif
        m_endEvent.cmdEvent = copier.get_last_event();

        if (( CL_DEV_CMD_WRITE == m_pCmd->type ) && (0 == pMicMemObj->GetWriteMapsCount()))
        {
            m_endEvent.cmdEvent = ForceTransferToDevice( pMicMemObj, m_endEvent.cmdEvent );
        }
        
        m_lastError = CL_DEV_SUCCESS;

    } while (0);

    return executePostDispatchProcess(false, error);
}


class CopyMemoryChunk : public ProcessCommonMemoryChunk
{
public:
    CopyMemoryChunk( const COIEVENT* external, MICDevMemoryObject* from_memObj, MICDevMemoryObject* to_memObj, COIPROCESS processOfTarget ) :
        ProcessCommonMemoryChunk( external, processOfTarget ),
        m_from_memObj(from_memObj), m_to_memObj(to_memObj) {};

protected:

    bool fire_action( const CommonMemoryChunk::Chunk& chunk, const COIEVENT* dependecies, uint32_t num_dependencies, COIEVENT* fired_event );

private:
    MICDevMemoryObject* m_from_memObj;
    MICDevMemoryObject* m_to_memObj;
};

bool CopyMemoryChunk::fire_action( const CommonMemoryChunk::Chunk& chunk, const COIEVENT* dependecies, uint32_t num_dependencies, COIEVENT* fired_event )
{
    return processActionOptimized(CL_DEV_CMD_COPY, m_from_memObj, chunk.from_offset, m_to_memObj, chunk.to_offset, chunk.size, dependecies, num_dependencies, fired_event);
}

CopyMemObject::CopyMemObject(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd) : BufferCommands(pCommandList, pFrameworkCallBacks, pCmd), m_srcBufferMirror(NULL), m_memObjOfHostPtr(NULL)
{
}

CopyMemObject::~CopyMemObject()
{
    if (m_srcBufferMirror)
    {
        delete(m_srcBufferMirror);
    }
    // If I convert the Read / Write operation to Copy with the mem object m_memObjOfHostPtr, than now I can return it back.
    if (m_memObjOfHostPtr)
    {
        m_memObjOfHostPtr->returnMemObjToMapBuffersPool();
    }
}

cl_dev_err_code CopyMemObject::Create(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd, SharedPtr<Command>& pOutCommand)
{
    return verifyCreation(new CopyMemObject(pCommandList, pFrameworkCallBacks, pCmd), pOutCommand);
}

cl_dev_err_code CopyMemObject::execute()
{
    cl_dev_cmd_param_copy*    cmdParams = (cl_dev_cmd_param_copy*)m_pCmd->params;
    MICDevMemoryObject*     pMicMemObjSrc;
    MICDevMemoryObject*     pMicMemObjDst;
    mem_copy_info_struct    sCpyParam;  // Assume in this case that the source is hostPtr and the destination is coiBuffer (Will convert later the results of the source)

    cl_dev_err_code err = cmdParams->srcMemObj->clDevMemObjGetDescriptor(CL_DEVICE_TYPE_ACCELERATOR, 0, (cl_dev_memobj_handle*)&pMicMemObjSrc);
    if (CL_DEV_SUCCESS != err)
    {
        return err;
    }
    err = cmdParams->dstMemObj->clDevMemObjGetDescriptor(CL_DEVICE_TYPE_ACCELERATOR, 0, (cl_dev_memobj_handle*)&pMicMemObjDst);
    if (CL_DEV_SUCCESS != err)
    {
        return err;
    }

    const cl_mem_obj_descriptor& pSrcMemObj = pMicMemObjSrc->clDevMemObjGetDescriptorRaw();
    const cl_mem_obj_descriptor& pDstMemObj = pMicMemObjDst->clDevMemObjGetDescriptorRaw();

    size_t  uiSrcElementSize = pSrcMemObj.uiElementSize;
    size_t    uiDstElementSize = pDstMemObj.uiElementSize;

    bool error = false;

    // The do .... while (0) is a pattern when there are many failures points instead of goto operation use do ... while (0) with break commands.
    do
    {
        // Objects has to have same element size or buffer<->image
        if( (uiDstElementSize != uiSrcElementSize) &&
            (1 != uiDstElementSize) && (1 != uiSrcElementSize) )
        {
            m_lastError = CL_DEV_INVALID_COMMAND_PARAM;
            break;
        }
        //Options for different dimensions are
        //Copy a 2D image object to a 2D slice of a 3D image object.
        //Copy a 2D slice of a 3D image object to a 2D image object.
        //Copy 2D to 2D
        //Copy 3D to 3D
        //Copy 2D image to buffer
        //Copy 3D image to buffer
        //Buffer to image
        memcpy(sCpyParam.vFromPitch, cmdParams->src_pitch[0] ? cmdParams->src_pitch : pSrcMemObj.pitch, sizeof(sCpyParam.vFromPitch));
        memcpy(sCpyParam.vToPitch,   cmdParams->dst_pitch[0] ? cmdParams->dst_pitch : pDstMemObj.pitch, sizeof(sCpyParam.vToPitch));

        sCpyParam.from_Offset = MemoryAllocator::CalculateOffset(cmdParams->src_dim_count, cmdParams->src_origin, sCpyParam.vFromPitch, pSrcMemObj.uiElementSize);
        sCpyParam.to_Offset   = MemoryAllocator::CalculateOffset(cmdParams->dst_dim_count, cmdParams->dst_origin, sCpyParam.vToPitch, pDstMemObj.uiElementSize);

        sCpyParam.uiDimCount = min(cmdParams->src_dim_count, cmdParams->dst_dim_count);
        if(cmdParams->dst_dim_count != cmdParams->src_dim_count)
        {
            //Buffer to image
            if(1 == cmdParams->src_dim_count)
            {
                uiSrcElementSize = uiDstElementSize;
                sCpyParam.uiDimCount = cmdParams->dst_dim_count;
                sCpyParam.vFromPitch[0] = cmdParams->region[0] * uiDstElementSize;
                sCpyParam.vFromPitch[1] = sCpyParam.vFromPitch[0] * cmdParams->region[1];
            }
            if( 1 == cmdParams->dst_dim_count)
            {
                //When destination is buffer the memcpy will be done as if the buffer is an image with height=1
                sCpyParam.uiDimCount = cmdParams->src_dim_count;
                sCpyParam.vToPitch[0] = cmdParams->region[0] * uiSrcElementSize;
                sCpyParam.vToPitch[1] = sCpyParam.vToPitch[0] * cmdParams->region[1];
            }
        }

        //If row_pitch (or input_row_pitch) is set to 0, the appropriate row pitch is calculated
        //based on the size of each element in bytes multiplied by width.
        memcpy(sCpyParam.vRegion, cmdParams->region, sizeof(sCpyParam.vRegion));
        sCpyParam.vRegion[0] *= uiSrcElementSize;

        COIEVENT barrier;
        unsigned int numDependecies = 0;
        m_pCommandList->getLastDependentBarrier(&barrier, &numDependecies, false);

        COIEVENT* pBarrier = (numDependecies == 0) ? NULL : &barrier;

        assert( (numDependecies <= 1) && "Previous command list dependencies may not be more than 1" );

        if (numDependecies > 1)
        {
            m_lastError = CL_DEV_NOT_SUPPORTED;
            break;
        }

#ifdef ENABLE_MIC_TRACER
        // Set command type for the tracer.
        m_commandTracer.set_command_type((char*)"Copy");
#endif
        // Work around if the source buffer and the destination buffer are the same COI buffer, then execute the following:
        //  * Read the whole COIBuffer to temporary host buffer.
        //  * Write from the temporary host buffer instead of copy from source COIBuffer.
        ProcessCommonMemoryChunk* pCopier = NULL;
        COIEVENT initialReadBarrier;
        if (pMicMemObjSrc->clDevMemObjGetTopLevelCoiBufferHandler() == pMicMemObjDst->clDevMemObjGetTopLevelCoiBufferHandler())
        {
            // Allocate memory for source mirror buffer.
            m_srcBufferMirror = new char[pMicMemObjSrc->GetRawDataSize()];
            assert(m_srcBufferMirror);
            if (NULL == m_srcBufferMirror)
            {
                m_lastError = CL_DEV_OUT_OF_MEMORY;
                break;
            }

            COIRESULT coiResult = COIBufferRead ( pMicMemObjSrc->clDevMemObjGetCoiBufferHandler(),					// Buffer to read from.
												 0,																	// Location in the buffer to start reading from.
												 m_srcBufferMirror,													// A pointer to local memory that should be written into.
												 pMicMemObjSrc->GetRawDataSize(),									// The number of bytes to write from coiBuffer into host
												 COI_COPY_UNSPECIFIED,												// The type of copy operation to use. (//TODO check option to change the type in order to improve performance)
												 numDependecies,													// The number of dependencies specified.
												 pBarrier,															// An optional array of handles to previously created COIEVENT objects that this read operation will wait for before starting.
												 &initialReadBarrier										        // An optional event to be signaled when the copy has completed.
											   );

            assert(COI_SUCCESS == coiResult);
            if (COI_SUCCESS != coiResult)
            {
                m_lastError = CL_DEV_ERROR_FAIL;
                break;
            }

            pCopier = new ReadWriteMemoryChunk (
                                                &initialReadBarrier,
                                                pMicMemObjDst,
                                                m_srcBufferMirror,
                                                false,
                                                m_pCommandList->getDeviceProcess());

        }
        else	// Regular copy from different source and destination COIBuffers.
        {
            pCopier = new CopyMemoryChunk(
                          pBarrier,
                          pMicMemObjSrc,
                          pMicMemObjDst,
                          m_pCommandList->getDeviceProcess()
                         );
        }

        assert(pCopier);
        if (NULL == pCopier)
        {
            m_lastError = CL_DEV_OUT_OF_MEMORY;
            break;
        }

#ifdef ENABLE_MIC_TRACER
        // Set start coi execution time for the tracer.
        m_commandTracer.set_current_time_coi_enqueue_command_time_start();
#endif
        CopyRegion( &sCpyParam, pCopier );

        //TODO Remove it when COI will fix the COIBUFFERCOPY in same COIBUFFER.
        m_memObjOfHostPtr = pCopier->getUsedMemObjOfHostPtr();

        if (pCopier->error_occured())
        {
            m_lastError = CL_DEV_ERROR_FAIL;
            break;
        }

        if (!pCopier->work_dispatched())
        {
            m_lastError = CL_DEV_SUCCESS;
            error = true;
            break;
        }

#ifdef ENABLE_MIC_TRACER
        // Set total amount of buffer operations for the Tracer.
        unsigned int amount = pCopier->get_total_amount_of_chunks();
        m_commandTracer.add_delta_num_of_buffer_operations(amount);
        // Set total size of buffer operations for the Tracer.
        unsigned long long size = pCopier->get_total_memory_processed_size();
        m_commandTracer.add_delta_buffer_operation_overall_size(size);
#endif
        m_endEvent.cmdEvent = pCopier->get_last_event();
        
        if (0 == pMicMemObjDst->GetWriteMapsCount())
        {
            m_endEvent.cmdEvent = ForceTransferToDevice( pMicMemObjDst, m_endEvent.cmdEvent );
        }

        m_lastError = CL_DEV_SUCCESS;

    }
    while (0);

    return executePostDispatchProcess(false, error);
}


class MapMemoryChunk : public ProcessCommonMemoryChunk
{
public:
    MapMemoryChunk( const COIEVENT* external, COIBUFFER buffer, void* ptr, COI_MAP_TYPE map_type, SMemMapParamsList* coiMapParamList ) :
        ProcessCommonMemoryChunk( external ), 
        m_buffer(buffer), m_ptr((char*)ptr), m_mapType(map_type), m_pCoiMapParamList(coiMapParamList) {};

    void process_finish( void );

protected:

    bool fire_action( const CommonMemoryChunk::Chunk& chunk, const COIEVENT* dependecies, uint32_t num_dependencies, COIEVENT* fired_event );

private:
    COIBUFFER m_buffer;
    char* m_ptr;
    COI_MAP_TYPE m_mapType;
    SMemMapParamsList* m_pCoiMapParamList;
    SMemMapParams m_coiMapParam;

};

/* Overwrite my parent implementation. */
void MapMemoryChunk::process_finish()
{
    // Call parent 'process_finish()' first in order to execute the last chunk.
    ProcessCommonMemoryChunk::process_finish();

    assert(m_pCoiMapParamList);
    // Push the new SMemMapParams to SMemMapParamsList. (As the last operation)
    m_pCoiMapParamList->push(m_coiMapParam);
}

bool MapMemoryChunk::fire_action( const CommonMemoryChunk::Chunk& chunk, const COIEVENT* dependecies, uint32_t num_dependencies, COIEVENT* fired_event )
{
    COIMAPINSTANCE currentMapInstance;
    void* mapPointer = m_ptr+chunk.to_offset;
    COIRESULT coiResult = COIBufferMap ( m_buffer,                    // Handle for the buffer to map.
                               chunk.from_offset,                   // Offset into the buffer that a pointer should be returned for.
                               chunk.size,                            // Length of the buffer area to map.
                               m_mapType,                            // The access type that is needed by the application.
                               num_dependencies,                    // The number of dependencies specified in the barrier array.
                               dependecies,                            // An optional array of handles to previously created COIEVENT objects that this map operation will wait for before starting.
                               fired_event,                            // An optional pointer to a COIEVENT object that will be signaled when a map call with the passed in buffer would complete immediately, that is, the buffer memory has been allocated on the host and its contents updated.
                               &currentMapInstance,                    // A pointer to a COIMAPINSTANCE which represents this mapping of the buffer
                               &mapPointer
                             );

    assert(COI_SUCCESS == coiResult);
    bool result = (COI_SUCCESS == coiResult);
    if (result)
    {
        result = m_coiMapParam.insertMapHandle(currentMapInstance);
    }
    return result;
}


MapMemObject::MapMemObject(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd) : BufferCommands(pCommandList, pFrameworkCallBacks, pCmd)
{
}

cl_dev_err_code MapMemObject::Create(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd, SharedPtr<Command>& pOutCommand)
{
    return verifyCreation(new MapMemObject(pCommandList, pFrameworkCallBacks, pCmd), pOutCommand);
}

cl_dev_err_code MapMemObject::execute()
{
    cl_dev_cmd_param_map*    cmdParams = (cl_dev_cmd_param_map*)(m_pCmd->params);
    mem_copy_info_struct    sCpyParam;
    MICDevMemoryObject*     pMicMemObj;

    // Request access on default device
    cl_dev_err_code err = cmdParams->memObj->clDevMemObjGetDescriptor(CL_DEVICE_TYPE_ACCELERATOR, 0, (cl_dev_memobj_handle*)&pMicMemObj);
    if (CL_DEV_SUCCESS != err)
    {
        return err;
    }

    pMicMemObj->addMemObjToMapBuffersPool();

    const cl_mem_obj_descriptor& pMemObj = pMicMemObj->clDevMemObjGetDescriptorRaw();

    sCpyParam.from_Offset = MemoryAllocator::CalculateOffset(pMemObj.dim_count, cmdParams->origin, pMemObj.pitch, pMemObj.uiElementSize);
    memcpy(sCpyParam.vFromPitch, pMemObj.pitch, sizeof(sCpyParam.vFromPitch));

    // Setup data for copying
    // Set Source/Destination
    sCpyParam.uiDimCount = cmdParams->dim_count;
    memcpy(sCpyParam.vRegion, cmdParams->region, sizeof(sCpyParam.vRegion));
    sCpyParam.vRegion[0] = cmdParams->region[0] * pMemObj.uiElementSize;

    sCpyParam.to_Offset = 0;
    memcpy(sCpyParam.vToPitch, cmdParams->pitch, sizeof(sCpyParam.vToPitch));

    bool error = false;
    // The do .... while (0) is a pattern when there are many failures points instead of goto operation use do ... while (0) with break commands.
    do
    {
        COIEVENT barrier;
        unsigned int numDependecies = 0;
        m_pCommandList->getLastDependentBarrier(&barrier, &numDependecies, false);

        COIEVENT* pBarrier = (numDependecies == 0) ? NULL : &barrier;

        assert( (numDependecies <= 1) && "Previous command list dependencies may not be more than 1" );

        if (numDependecies > 1)
        {
            m_lastError = CL_DEV_NOT_SUPPORTED;
            break;
        }

        MapMemoryChunk copier(
                               pBarrier,
                               pMicMemObj->clDevMemObjGetCoiBufferHandler(), // Get the COIBUFFER which represent this memory object
                               cmdParams->ptr,
                               COI_MAP_READ_WRITE, 
                               MemoryAllocator::GetCoiMapParams(cmdParams) ); // Get the 'SMemMapParamsList' pointer of this memory object

#ifdef ENABLE_MIC_TRACER
        // Set command type for the tracer.
        m_commandTracer.set_command_type((char*)"Map");
        // Set start coi execution time for the tracer.
        m_commandTracer.set_current_time_coi_enqueue_command_time_start();
#endif
        CopyRegion( &sCpyParam, &copier );

        if (copier.error_occured())
        {
            m_lastError = CL_DEV_ERROR_FAIL;
            break;
        }

        if (!copier.work_dispatched())
        {
            m_lastError = CL_DEV_SUCCESS;
            error = true;
            break;
        }

#ifdef ENABLE_MIC_TRACER
        // Set total amount of buffer operations for the Tracer.
        unsigned int amount = copier.get_total_amount_of_chunks();
        m_commandTracer.add_delta_num_of_buffer_operations(amount);
        // Set total size of buffer operations for the Tracer.
        unsigned long long size = copier.get_total_memory_processed_size();
        m_commandTracer.add_delta_buffer_operation_overall_size(size);
#endif
        m_endEvent.cmdEvent = copier.get_last_event();
        pMicMemObj->IncWriteMapsCount();
        m_lastError = CL_DEV_SUCCESS;
    }
    while (0);

    return executePostDispatchProcess(false, error);
}

void MapMemObject::fireCallBack(void* arg)
{
    cl_dev_cmd_param_map*    cmdParams = (cl_dev_cmd_param_map*)(m_pCmd->params);
    MICDevMemoryObject*     pMicMemObj;
    cl_dev_err_code err = cmdParams->memObj->clDevMemObjGetDescriptor(CL_DEVICE_TYPE_ACCELERATOR, 0, (cl_dev_memobj_handle*)&pMicMemObj);
    assert(CL_DEV_SUCCESS == err);
    if (CL_DEV_SUCCESS == err)
    {
        pMicMemObj->setMemObjInMapBuffersPoolReady(m_pCommandList);
    }
    // Call parent fireCallBack
    Command::fireCallBack(arg);
}


class UnmapMemoryChunk : public ProcessUnmapMemoryChunk
{
public:
    UnmapMemoryChunk( const COIEVENT* external ) :
        ProcessUnmapMemoryChunk( external ) {};

protected:

    bool fire_action( const UnmapMemoryChunkStruct::Chunk& chunk, const COIEVENT* dependecies, uint32_t num_dependencies, COIEVENT* fired_event );

};

bool UnmapMemoryChunk::fire_action( const UnmapMemoryChunkStruct::Chunk& chunk, const COIEVENT* dependecies, uint32_t num_dependencies, COIEVENT* fired_event )
{
    if (NULL == chunk.coi_map_instance)
    {
        return false;
    }
    COIRESULT coiResult = COIBufferUnmap ( chunk.coi_map_instance,                    // Buffer map instance handle to unmap.
                                            num_dependencies,                        // The number of dependencies specified in the barrier array.
                                            dependecies,                            // An optional array of handles to previously created COIEVENT objects that this unmap operation will wait for before starting.
                                            fired_event                                // An optional pointer to a COIEVENT object that will be signaled when the unmap is complete.
                                          );

    assert(COI_SUCCESS == coiResult);
    return (COI_SUCCESS == coiResult);
}



UnmapMemObject::UnmapMemObject(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd) : BufferCommands(pCommandList, pFrameworkCallBacks, pCmd)
{
}

cl_dev_err_code UnmapMemObject::Create(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd, SharedPtr<Command>& pOutCommand)
{
    return verifyCreation(new UnmapMemObject(pCommandList, pFrameworkCallBacks, pCmd), pOutCommand);
}

cl_dev_err_code UnmapMemObject::execute()
{
    cl_dev_cmd_param_map*    cmdParams = (cl_dev_cmd_param_map*)(m_pCmd->params);
    MICDevMemoryObject*     pMicMemObj;

    // Request access on default device
    cl_dev_err_code err = cmdParams->memObj->clDevMemObjGetDescriptor(CL_DEVICE_TYPE_ACCELERATOR, 0, (cl_dev_memobj_handle*)&pMicMemObj);
    if (CL_DEV_SUCCESS != err)
    {
        return err;
    }

    bool error = false;
    // The do .... while (0) is a pattern when there are many failures points instead of goto operation use do ... while (0) with break commands.
    do
    {        
        COIEVENT barrier;
        unsigned int numDependecies = 0;
        m_pCommandList->getLastDependentBarrier(&barrier, &numDependecies, false);

        COIEVENT* pBarrier = (numDependecies == 0) ? NULL : &barrier;

        assert( (numDependecies <= 1) && "Previous command list dependencies may not be more than 1" );

        if (numDependecies > 1)
        {
            m_lastError = CL_DEV_NOT_SUPPORTED;
            break;
        }

        SMemMapParamsList* coiMapParamList = MemoryAllocator::GetCoiMapParams(cmdParams);
        assert(coiMapParamList);

        SMemMapParams coiMapParam;
        // Get arbitrary instance of SMemMapParams
        if (false == coiMapParamList->pop(&coiMapParam))
        {
            m_lastError = CL_DEV_ERROR_FAIL;
            break;
        }

        UnmapMemoryChunk unmapper( pBarrier );

#ifdef ENABLE_MIC_TRACER
        // Set command type for the tracer.
        m_commandTracer.set_command_type((char*)"Unmap");
        // Set start coi execution time for the tracer.
        m_commandTracer.set_current_time_coi_enqueue_command_time_start();
#endif

        // Set this object to be the context when notifying for event status change. (For the first command in case of rectangular operation)
        registerProfilingContext();
        // Init map handler iterator and traversing over it.
        coiMapParam.initMapHandleIterator();
        while (coiMapParam.hasNextMapHandle())
        {
            UnmapMemoryChunkStruct::Chunk tChunk(coiMapParam.getNextMapHandle());
            unmapper.process_chunk(tChunk);
            unregisterProfilingContext();
        }

        // Set this object to be the context when notifying for event status change. (For the last command in case of rectangular operation, or for first and last command in case of regular operatiron)
        registerProfilingContext();
        // Call to 'process_finish()' in order to complete the last unmap operations.
        unmapper.process_finish();
        unregisterProfilingContext();

        if (unmapper.error_occured())
        {
            m_lastError = CL_DEV_ERROR_FAIL;
            break;
        }

        if (!unmapper.work_dispatched())
        {
            m_lastError = CL_DEV_SUCCESS;
            error = true;
            break;
        }

#ifdef ENABLE_MIC_TRACER
        // Set total amount of buffer operations for the Tracer.
        unsigned int amount = unmapper.get_total_amount_of_chunks();
        m_commandTracer.add_delta_num_of_buffer_operations(amount);
        // Set total size of buffer operations for the Tracer.
        unsigned long long size = unmapper.get_total_memory_processed_size();
        m_commandTracer.add_delta_buffer_operation_overall_size(size);
#endif
        m_endEvent.cmdEvent = unmapper.get_last_event();
        
        if (0 == pMicMemObj->DecWriteMapsCount())
        {
            m_endEvent.cmdEvent = ForceTransferToDevice( pMicMemObj, m_endEvent.cmdEvent );
        }
        
        m_lastError = CL_DEV_SUCCESS;
    }
    while (0);

    return executePostDispatchProcess(false, error);
}

void UnmapMemObject::fireCallBack(void* arg)
{
    cl_dev_cmd_param_map*    cmdParams = (cl_dev_cmd_param_map*)(m_pCmd->params);
    MICDevMemoryObject*     pMicMemObj;
    cl_dev_err_code err = cmdParams->memObj->clDevMemObjGetDescriptor(CL_DEVICE_TYPE_ACCELERATOR, 0, (cl_dev_memobj_handle*)&pMicMemObj);
    assert(CL_DEV_SUCCESS == err);
    if (CL_DEV_SUCCESS == err)
    {
        pMicMemObj->removeMemObjFromMapBuffersPool();
    }
    // Call parent fireCallBack
    Command::fireCallBack(arg);
}

//
//  Migrate Memory Object
//

MigrateMemObject::MigrateMemObject(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd) : BufferCommands(pCommandList, pFrameworkCallBacks, pCmd)
{
}

cl_dev_err_code MigrateMemObject::Create(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd, SharedPtr<Command>& pOutCommand)
{
    return verifyCreation(new MigrateMemObject(pCommandList, pFrameworkCallBacks, pCmd), pOutCommand);
}

cl_dev_err_code MigrateMemObject::init(vector<COIBUFFER>&    outCoiBuffsArr, 
                                       COI_BUFFER_MOVE_FLAG& outMoveDataFlag, COIPROCESS& outTargetProcess,
                                       COIBUFFER&            outLastBufferHandle )
{
    cl_dev_err_code             returnError = CL_DEV_SUCCESS;
    cl_dev_cmd_param_migrate*    cmdParams   = (cl_dev_cmd_param_migrate*)m_pCmd->params;

    if ((0 == cmdParams->mem_num) || (NULL == cmdParams->memObjs))
    {
        return CL_DEV_INVALID_VALUE;
    }

    outMoveDataFlag  = (0 != (cmdParams->flags & CL_MIGRATE_MEM_OBJECT_CONTENT_UNDEFINED)) ? COI_BUFFER_NO_MOVE : COI_BUFFER_MOVE;
    outTargetProcess = (0 != (cmdParams->flags & CL_MIGRATE_MEM_OBJECT_HOST)) ? COI_PROCESS_SOURCE : m_pCommandList->getDeviceProcess();
    
    outCoiBuffsArr.reserve( cmdParams->mem_num );

    size_t min_size     = ((size_t)-1); // set the maximum possible size_t value 
    outLastBufferHandle = NULL;

    // extract COI Buffer handles 
    for (cl_uint i = 0; i < cmdParams->mem_num; ++i)
    {
        MICDevMemoryObject*  pMicMemObj;
        returnError = cmdParams->memObjs[i]->clDevMemObjGetDescriptor(CL_DEVICE_TYPE_ACCELERATOR, 0, (cl_dev_memobj_handle*)&pMicMemObj);
        if (CL_DEV_FAILED(returnError))
        {
            break;
        }

        COIBUFFER coi_handle = pMicMemObj->clDevMemObjGetCoiBufferHandler();
        outCoiBuffsArr.push_back(coi_handle);

        // find the buffer with minimum size
        size_t raw_size = pMicMemObj->GetRawDataSize();
        if (raw_size < min_size)
        {
            min_size            = raw_size;
            outLastBufferHandle = coi_handle;
        }
    };

    if (CL_DEV_FAILED(returnError))
    {
        outCoiBuffsArr.clear();
    }
    m_lastError = returnError;

    return returnError;
}

cl_dev_err_code MigrateMemObject::execute()
{
    COIRESULT       result;

    // the COIBUFFERs to dispatch
    vector<COIBUFFER>     coiBuffsArr;
    COI_BUFFER_MOVE_FLAG  moveDataFlag;
    COIPROCESS            targetProcess;
    COIBUFFER             last_buffer_handle;

    m_lastError = CL_DEV_SUCCESS;

	do
	{
        COIEVENT barrier;
        unsigned int numDependecies = 0;
        m_pCommandList->getLastDependentBarrier(&barrier, &numDependecies, false);

        COIEVENT* pBarrier = (numDependecies == 0) ? NULL : &barrier;

        assert( (numDependecies <= 1) && "Previous command list dependencies may not be more than 1" );

        if (numDependecies > 1)
        {
            m_lastError = CL_DEV_NOT_SUPPORTED;
            break;
        }


        m_lastError = init(coiBuffsArr, moveDataFlag, targetProcess, last_buffer_handle);
        if (m_lastError != CL_DEV_SUCCESS)
        {
            break;
        }

#ifdef ENABLE_MIC_TRACER
        // Set command type for the tracer.
        m_commandTracer.set_command_type((char*)"MigrateMemObject");
        // Set start coi execution time for the tracer.
        m_commandTracer.set_current_time_coi_enqueue_command_time_start();
#endif
        // now enqueue all operations independently, except of the last, that should be dependent on all other
        vector<COIEVENT> independent_ops;
        independent_ops.reserve( coiBuffsArr.size() + numDependecies );
        if (NULL != pBarrier)
        {
            // make last operation dependent on queue barrier in case only a single buffer op is required
            independent_ops.push_back( *pBarrier );
        }

        vector<COIBUFFER>::iterator it     = coiBuffsArr.begin();
        vector<COIBUFFER>::iterator it_end = coiBuffsArr.end();

        for(; it != it_end; ++it)
        {
            COIEVENT intermediate_barrier;
            COIBUFFER buffer = *it;

            // skip the buffer that should be the last one
            if (buffer == last_buffer_handle)
            {
                continue;
            }
            
            result = COIBufferSetState(buffer, 
                                       targetProcess, COI_BUFFER_VALID, moveDataFlag, 
                                       numDependecies, pBarrier, 
                                       &intermediate_barrier);

            if (result != COI_SUCCESS)
            {
                assert( (result == COI_SUCCESS) && "COIBufferSetState() returned error for buffer migration" );
                m_lastError = CL_DEV_ERROR_FAIL;
                break;
            }

            independent_ops.push_back( intermediate_barrier );
        }

        COIEVENT* barrier_list = (independent_ops.size() > 0) ? &(independent_ops[0]) : NULL;
        
        result = COIBufferSetState(last_buffer_handle, 
                                   targetProcess, COI_BUFFER_VALID, moveDataFlag, 
                                   independent_ops.size(), barrier_list, 
                                   &(m_endEvent.cmdEvent));
        
        if (result != COI_SUCCESS)
        {
            assert( (result == COI_SUCCESS) && "COIBufferSetState() returned error for buffer migration" );
            m_lastError = CL_DEV_ERROR_FAIL;
            break;
        }
    }
    while (0);

    return executePostDispatchProcess(false, false);
}
