#include "buffer_commands.h"
#include "memory_allocator.h"
#include "command_list.h"
#include "mic_device_interface.h"
#include "cl_sys_info.h"

#include <source/COIBuffer_source.h>
#include <source/COIProcess_source.h>

#include "cl_types.h"
#include "cl_utils.h"

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
	COINotificationCallbackSetContext(NULL);

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
		m_commandTracer.set_current_time_coi_execution_time_start();
		if ((m_pCmd->profiling) && (m_cmdRunningTime == 0))
		{
			m_cmdRunningTime = HostTime();
		}
		break;
	case BUFFER_OPERATION_COMPLETE:
		// Set end coi execution time for the tracer. (COI COMPLETED)
		m_commandTracer.set_current_time_coi_execution_time_end();
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
	COINotificationCallbackSetContext(this);
	CopyRegionInternal( pMemCopyInfo, chunk_consumer );
	COINotificationCallbackSetContext(NULL);

	// Set this object to be the context when notifying for event status change. (For the last command in case of rectangular operation, or for first and last command in case of regular operatiron)
	COINotificationCallbackSetContext(this);
    chunk_consumer->process_finish();
	COINotificationCallbackSetContext(NULL);
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

	COINotificationCallbackSetContext(this);

    COIRESULT coi_result = COIBufferSetState( 
                        mem_obj->clDevMemObjGetCoiBufferHandler(),  // Buffer to transfer
                        m_pCommandList->getDeviceProcess(),         // Target Device process 
                        COI_BUFFER_VALID,                           // Desired state in the target process
                        COI_BUFFER_MOVE,                            // Force data movement if required
                        1, &last_chunk_event,                       // array of dependencies
                        &transfer_event );                          // event to be signaled on completion

    assert( (COI_SUCCESS == coi_result) && "Wrong params for COIBufferSetState" );

	COINotificationCallbackSetContext(NULL);

    // this is an optimization - if failed, nothing to do
    return ((COI_SUCCESS == coi_result) ? transfer_event : last_chunk_event);
}

class ReadWriteMemoryChunk : public ProcessCommonMemoryChunk
{
public:
    ReadWriteMemoryChunk( const COIEVENT* external, COIBUFFER buffer, void* ptr, bool is_read ) :
        ProcessCommonMemoryChunk( external ),
        m_buffer(buffer), m_ptr( (char*)ptr ), m_is_read_mode(is_read) {};

protected:

    bool fire_action( const CommonMemoryChunk::Chunk& chunk, const COIEVENT* dependecies, uint32_t num_dependencies, COIEVENT* fired_event );

private:
    COIBUFFER m_buffer;
    char*     m_ptr;
    bool      m_is_read_mode;
};

bool ReadWriteMemoryChunk::fire_action( const CommonMemoryChunk::Chunk& chunk, const COIEVENT* dependecies, uint32_t num_dependencies, COIEVENT* fired_event )
{
    COIRESULT result;

	if ( m_is_read_mode )
	{
		result = COIBufferRead ( m_buffer,					// Buffer to read from.
							     chunk.from_offset,			// Location in the buffer to start reading from.
							     m_ptr+chunk.to_offset,		// A pointer to local memory that should be written into.
							     chunk.size,				// The number of bytes to write from coiBuffer into host
							     COI_COPY_UNSPECIFIED,		// The type of copy operation to use. (//TODO check option to change the type in order to improve performance)
							     num_dependencies,			// The number of dependencies specified.
							     dependecies,			    // An optional array of handles to previously created COIEVENT objects that this read operation will wait for before starting.
							     fired_event		        // An optional event to be signaled when the copy has completed.
							   );
	}
	else
	{
		result = COIBufferWrite (m_buffer,				    // Buffer to write to.
							     chunk.to_offset,			// Location in the buffer to start writing from.
							     m_ptr+chunk.from_offset,		// A pointer to local memory that should be read from.
							     chunk.size,				// The number of bytes to write from host into coiBuffer.
							     COI_COPY_UNSPECIFIED,		// The type of copy operation to use. (//TODO check option to change the type in order to improve performance)
							     num_dependencies,			// The number of dependencies specified.
							     dependecies,				// An optional array of handles to previously created COIEVENT objects that this write operation will wait for before starting.
							     fired_event		        // An optional event to be signaled when the copy has completed.
							   );
	}

	return (COI_SUCCESS == result);
}

ReadWriteMemObject::ReadWriteMemObject(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd) : BufferCommands(pCommandList, pFrameworkCallBacks, pCmd)
{
}

cl_dev_err_code ReadWriteMemObject::Create(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd, Command** pOutCommand)
{
	return verifyCreation(new ReadWriteMemObject(pCommandList, pFrameworkCallBacks, pCmd), pOutCommand);
}

cl_dev_err_code ReadWriteMemObject::execute()
{

	cl_dev_cmd_param_rw*	cmdParams = (cl_dev_cmd_param_rw*)m_pCmd->params;
	MICDevMemoryObject*     pMicMemObj;
	mem_copy_info_struct	sCpyParam;

	cl_dev_err_code err = cmdParams->memObj->clDevMemObjGetDescriptor(CL_DEVICE_TYPE_ACCELERATOR, 0, (cl_dev_memobj_handle*)&pMicMemObj);
	if (CL_DEV_SUCCESS != err)
	{
		return err;
	}

    bool error = false;

    do {
       	COIEVENT* barrier = NULL;
    	unsigned int numDependecies = 0;
    	m_pCommandSynchHandler->getLastDependentBarrier(m_pCommandList, &barrier, &numDependecies, false);

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
    	size_t ptrOffset =	cmdParams->ptr_origin[2] * cmdParams->pitch[1] + \
    						cmdParams->ptr_origin[1] * cmdParams->pitch[0] + \
    						cmdParams->ptr_origin[0];

        ReadWriteMemoryChunk copier(
                            barrier,
                            pMicMemObj->clDevMemObjGetCoiBufferHandler(), // Get the COIBUFFER which represent this memory object
                            cmdParams->ptr,
                            ( CL_DEV_CMD_READ == m_pCmd->type ) );

        if ( CL_DEV_CMD_READ == m_pCmd->type )
        {
			// Set command type for the tracer.
			m_commandTracer.set_command_type((char*)"Read");
        	// set coiBuffer (objPtr) initial offset
        	sCpyParam.from_Offset = offset;
        	memcpy(sCpyParam.vFromPitch, pObjPitchPtr, sizeof(sCpyParam.vFromPitch));

        	// set host pointer with the calculated offset and copy the pitch
        	sCpyParam.to_Offset = ptrOffset;
        	memcpy(sCpyParam.vToPitch, cmdParams->pitch, sizeof(sCpyParam.vToPitch));
        }
        else
        {
			// Set command type for the tracer.
			m_commandTracer.set_command_type((char*)"Write");
        	// set host pointer with the calculated offset and copy the pitch
        	sCpyParam.from_Offset = ptrOffset;
        	memcpy(sCpyParam.vFromPitch, cmdParams->pitch, sizeof(sCpyParam.vFromPitch));

        	// set coiBuffer (objPtr) initial offset
        	sCpyParam.to_Offset = offset;
        	memcpy(sCpyParam.vToPitch, pObjPitchPtr, sizeof(sCpyParam.vToPitch));
        }

		// Set start coi execution time for the tracer.
		m_commandTracer.set_current_time_coi_enqueue_command_time_start();

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

		// Set total amount of buffer operations for the Tracer.
		unsigned int amount = copier.get_total_amount_of_chunks();
		m_commandTracer.add_delta_num_of_buffer_operations(amount);
		// Set total size of buffer operations for the Tracer.
		unsigned long long size = copier.get_total_memory_processed_size();
		m_commandTracer.add_delta_buffer_operation_overall_size(size);

        m_completionBarrier.cmdEvent = copier.get_last_event();

        if (( CL_DEV_CMD_WRITE == m_pCmd->type ) && (0 == pMicMemObj->GetWriteMapsCount()))
        {
            m_completionBarrier.cmdEvent = ForceTransferToDevice( pMicMemObj, m_completionBarrier.cmdEvent );
        }
        
        m_lastError = CL_DEV_SUCCESS;

    } while (0);

	return executePostDispatchProcess(false, error);
}


class CopyMemoryChunk : public ProcessCommonMemoryChunk
{
public:
    CopyMemoryChunk( const COIEVENT* external, COIBUFFER from_buffer, COIBUFFER to_buffer ) :
        ProcessCommonMemoryChunk( external ),
        m_from_buffer(from_buffer), m_to_buffer(to_buffer) {};

protected:

    bool fire_action( const CommonMemoryChunk::Chunk& chunk, const COIEVENT* dependecies, uint32_t num_dependencies, COIEVENT* fired_event );

private:
    COIBUFFER m_from_buffer;
    COIBUFFER m_to_buffer;
};

bool CopyMemoryChunk::fire_action( const CommonMemoryChunk::Chunk& chunk, const COIEVENT* dependecies, uint32_t num_dependencies, COIEVENT* fired_event )
{
	COIRESULT result = COIBufferCopy (
                         m_to_buffer,           // Buffer to copy into.
	                     m_from_buffer,			// Buffer to copy from.
						 chunk.to_offset,		// Location in the destination buffer to start writing to.
						 chunk.from_offset,		// Location in the source buffer to start reading from.
						 chunk.size,			// The number of bytes to copy from coiBufferSrc into coiBufferSrc.
						 COI_COPY_UNSPECIFIED,	// The type of copy operation to use.
						 num_dependencies,		// The number of dependencies.
						 dependecies,			// An optional array of handles to previously created COIEVENT objects that this copy operation will wait for before starting.
						 fired_event		    // An optional event to be signaled when the copy has completed.
					);

	return (COI_SUCCESS == result);
}

CopyMemObject::CopyMemObject(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd) : BufferCommands(pCommandList, pFrameworkCallBacks, pCmd), m_srcBufferMirror(NULL)
{
}

CopyMemObject::~CopyMemObject()
{
	if (m_srcBufferMirror)
	{
		delete(m_srcBufferMirror);
	}
}

cl_dev_err_code CopyMemObject::Create(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd, Command** pOutCommand)
{
	return verifyCreation(new CopyMemObject(pCommandList, pFrameworkCallBacks, pCmd), pOutCommand);
}

cl_dev_err_code CopyMemObject::execute()
{
	cl_dev_cmd_param_copy*	cmdParams = (cl_dev_cmd_param_copy*)m_pCmd->params;
	MICDevMemoryObject*     pMicMemObjSrc;
	MICDevMemoryObject*     pMicMemObjDst;
	mem_copy_info_struct	sCpyParam;  // Assume in this case that the source is hostPtr and the destination is coiBuffer (Will convert later the results of the source)

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
	size_t	uiDstElementSize = pDstMemObj.uiElementSize;

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

       	COIEVENT* barrier = NULL;
    	unsigned int numDependecies = 0;
    	m_pCommandSynchHandler->getLastDependentBarrier(m_pCommandList, &barrier, &numDependecies, false);

        assert( (numDependecies <= 1) && "Previous command list dependencies may not be more than 1" );

        if (numDependecies > 1)
        {
    		m_lastError = CL_DEV_NOT_SUPPORTED;
            break;
        }

		// Set command type for the tracer.
		m_commandTracer.set_command_type((char*)"Copy");

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
												 barrier,															// An optional array of handles to previously created COIEVENT objects that this read operation will wait for before starting.
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
												pMicMemObjDst->clDevMemObjGetCoiBufferHandler(), // Get the COIBUFFER which represent this memory object
												m_srcBufferMirror,
												false );

		}
		else	// Regular copy from different source and destination COIBuffers.
		{
			pCopier = new CopyMemoryChunk(
											barrier,
											pMicMemObjSrc->clDevMemObjGetCoiBufferHandler(), // Get the COIBUFFER which represent this memory object
											pMicMemObjDst->clDevMemObjGetCoiBufferHandler()  // Get the COIBUFFER which represent this memory object
										 );
		}

		assert(pCopier);
		if (NULL == pCopier)
		{
			m_lastError = CL_DEV_OUT_OF_MEMORY;
			break;
		}

		// Set start coi execution time for the tracer.
		m_commandTracer.set_current_time_coi_enqueue_command_time_start();

        CopyRegion( &sCpyParam, pCopier );

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

		// Set total amount of buffer operations for the Tracer.
		unsigned int amount = pCopier->get_total_amount_of_chunks();
		m_commandTracer.add_delta_num_of_buffer_operations(amount);
		// Set total size of buffer operations for the Tracer.
		unsigned long long size = pCopier->get_total_memory_processed_size();
		m_commandTracer.add_delta_buffer_operation_overall_size(size);

        m_completionBarrier.cmdEvent = pCopier->get_last_event();
        
        if (0 == pMicMemObjDst->GetWriteMapsCount())
        {
            m_completionBarrier.cmdEvent = ForceTransferToDevice( pMicMemObjDst, m_completionBarrier.cmdEvent );
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
	COIRESULT coiResult = COIBufferMap ( m_buffer,				    // Handle for the buffer to map.
							   chunk.from_offset,                   // Offset into the buffer that a pointer should be returned for.
							   chunk.size,				            // Length of the buffer area to map.
							   m_mapType,							// The access type that is needed by the application.
							   num_dependencies,					// The number of dependencies specified in the barrier array.
							   dependecies,					        // An optional array of handles to previously created COIEVENT objects that this map operation will wait for before starting.
							   fired_event,							// An optional pointer to a COIEVENT object that will be signaled when a map call with the passed in buffer would complete immediately, that is, the buffer memory has been allocated on the host and its contents updated.
							   &currentMapInstance,				    // A pointer to a COIMAPINSTANCE which represents this mapping of the buffer
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

cl_dev_err_code MapMemObject::Create(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd, Command** pOutCommand)
{
	return verifyCreation(new MapMemObject(pCommandList, pFrameworkCallBacks, pCmd), pOutCommand);
}

cl_dev_err_code MapMemObject::execute()
{
	cl_dev_cmd_param_map*	cmdParams = (cl_dev_cmd_param_map*)(m_pCmd->params);
	mem_copy_info_struct	sCpyParam;
	MICDevMemoryObject*     pMicMemObj;

	// Request access on default device
	cl_dev_err_code err = cmdParams->memObj->clDevMemObjGetDescriptor(CL_DEVICE_TYPE_ACCELERATOR, 0, (cl_dev_memobj_handle*)&pMicMemObj);
	if (CL_DEV_SUCCESS != err)
	{
		return err;
	}

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
	    COIEVENT* barrier = NULL;
	    unsigned int numDependecies = 0;
	    m_pCommandSynchHandler->getLastDependentBarrier(m_pCommandList, &barrier, &numDependecies, false);

	    assert( (numDependecies <= 1) && "Previous command list dependencies may not be more than 1" );

        if (numDependecies > 1)
        {
            m_lastError = CL_DEV_NOT_SUPPORTED;
            break;
        }

		// Set command type for the tracer.
		m_commandTracer.set_command_type((char*)"Map");

		MapMemoryChunk copier(
                               barrier,
                               pMicMemObj->clDevMemObjGetCoiBufferHandler(), // Get the COIBUFFER which represent this memory object
                               cmdParams->ptr,
							   COI_MAP_READ_WRITE, 
                               MemoryAllocator::GetCoiMapParams(cmdParams) ); // Get the 'SMemMapParamsList' pointer of this memory object

		// Set start coi execution time for the tracer.
		m_commandTracer.set_current_time_coi_enqueue_command_time_start();

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

		// Set total amount of buffer operations for the Tracer.
		unsigned int amount = copier.get_total_amount_of_chunks();
		m_commandTracer.add_delta_num_of_buffer_operations(amount);
		// Set total size of buffer operations for the Tracer.
		unsigned long long size = copier.get_total_memory_processed_size();
		m_commandTracer.add_delta_buffer_operation_overall_size(size);

        m_completionBarrier.cmdEvent = copier.get_last_event();
        pMicMemObj->IncWriteMapsCount();
        m_lastError = CL_DEV_SUCCESS;
	}
	while (0);

	return executePostDispatchProcess(false, error);
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
	COIRESULT coiResult = COIBufferUnmap ( chunk.coi_map_instance,		            // Buffer map instance handle to unmap.
											num_dependencies,						// The number of dependencies specified in the barrier array.
											dependecies,							// An optional array of handles to previously created COIEVENT objects that this unmap operation will wait for before starting.
											fired_event								// An optional pointer to a COIEVENT object that will be signaled when the unmap is complete.
										  );

	assert(COI_SUCCESS == coiResult);
	return (COI_SUCCESS == coiResult);
}



UnmapMemObject::UnmapMemObject(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd) : BufferCommands(pCommandList, pFrameworkCallBacks, pCmd)
{
}

cl_dev_err_code UnmapMemObject::Create(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd, Command** pOutCommand)
{
	return verifyCreation(new UnmapMemObject(pCommandList, pFrameworkCallBacks, pCmd), pOutCommand);
}

cl_dev_err_code UnmapMemObject::execute()
{
	cl_dev_cmd_param_map*	cmdParams = (cl_dev_cmd_param_map*)(m_pCmd->params);
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
		COIEVENT* barrier = NULL;
		unsigned int numDependecies = 0;
		m_pCommandSynchHandler->getLastDependentBarrier(m_pCommandList, &barrier, &numDependecies, false);

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

		// Set command type for the tracer.
		m_commandTracer.set_command_type((char*)"Unmap");

		UnmapMemoryChunk unmapper( barrier );

		// Set start coi execution time for the tracer.
		m_commandTracer.set_current_time_coi_enqueue_command_time_start();

		// Set this object to be the context when notifying for event status change. (For the first command in case of rectangular operation)
		COINotificationCallbackSetContext(this);
		// Init map handler iterator and traversing over it.
		coiMapParam.initMapHandleIterator();
		while (coiMapParam.hasNextMapHandle())
		{
			UnmapMemoryChunkStruct::Chunk tChunk(coiMapParam.getNextMapHandle());
			unmapper.process_chunk(tChunk);
			COINotificationCallbackSetContext(NULL);
		}

		// Set this object to be the context when notifying for event status change. (For the last command in case of rectangular operation, or for first and last command in case of regular operatiron)
		COINotificationCallbackSetContext(this);
		// Call to 'process_finish()' in order to complete the last unmap operations.
		unmapper.process_finish();
		COINotificationCallbackSetContext(NULL);

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

		// Set total amount of buffer operations for the Tracer.
		unsigned int amount = unmapper.get_total_amount_of_chunks();
		m_commandTracer.add_delta_num_of_buffer_operations(amount);
		// Set total size of buffer operations for the Tracer.
		unsigned long long size = unmapper.get_total_memory_processed_size();
		m_commandTracer.add_delta_buffer_operation_overall_size(size);

        m_completionBarrier.cmdEvent = unmapper.get_last_event();
        
        if (0 == pMicMemObj->DecWriteMapsCount())
        {
            m_completionBarrier.cmdEvent = ForceTransferToDevice( pMicMemObj, m_completionBarrier.cmdEvent );
        }
        
        m_lastError = CL_DEV_SUCCESS;
	}
	while (0);

	return executePostDispatchProcess(false, error);
}

//
//  Migrate Memory Object
//

MigrateMemObject::MigrateMemObject(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd) : BufferCommands(pCommandList, pFrameworkCallBacks, pCmd)
{
}

cl_dev_err_code MigrateMemObject::Create(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd, Command** pOutCommand)
{
	return verifyCreation(new MigrateMemObject(pCommandList, pFrameworkCallBacks, pCmd), pOutCommand);
}

cl_dev_err_code MigrateMemObject::init(vector<COIBUFFER>&    outCoiBuffsArr, 
                                       COI_BUFFER_MOVE_FLAG& outMoveDataFlag, COIPROCESS& outTargetProcess,
                                       COIBUFFER&            outLastBufferHandle )
{
	cl_dev_err_code             returnError = CL_DEV_SUCCESS;
    cl_dev_cmd_param_migrate*	cmdParams   = (cl_dev_cmd_param_migrate*)m_pCmd->params;

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
		COIEVENT* barrier = NULL;
		unsigned int numDependecies = 0;
		m_pCommandSynchHandler->getLastDependentBarrier(m_pCommandList, &barrier, &numDependecies, false);

        assert( (numDependecies <= 1) && "Previous command list dependencies may not be more than 1" );

        if (numDependecies > 1)
        {
    		m_lastError = CL_DEV_NOT_SUPPORTED;
            break;
        }

		// Set command type for the tracer.
		m_commandTracer.set_command_type((char*)"MigrateMemObject");

		m_lastError = init(coiBuffsArr, moveDataFlag, targetProcess, last_buffer_handle);
		if (m_lastError != CL_DEV_SUCCESS)
		{
			break;
		}

		// Set start coi execution time for the tracer.
		m_commandTracer.set_current_time_coi_enqueue_command_time_start();

        // now enqueue all operations independently, except of the last, that should be dependent on all other
        vector<COIEVENT> independent_ops;
        independent_ops.reserve( coiBuffsArr.size() + numDependecies );
        if (NULL != barrier)
        {
            // make last operation dependent on queue barrier in case only a single buffer op is required
            independent_ops.push_back( *barrier );
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
                                       numDependecies, barrier, 
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
                                   m_pCommandSynchHandler->registerCompletionBarrier(m_completionBarrier));
        
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

