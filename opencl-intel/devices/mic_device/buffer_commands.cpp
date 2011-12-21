#include "buffer_commands.h"
#include "memory_allocator.h"
#include "command_list.h"

#include <source/COIBuffer_source.h>

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

void BufferCommands::CopyRegion(mem_copy_info_struct* pMemCopyInfo, ProcessCommonMemoryChunk* chunk_consumer )
{
    CopyRegionInternal( pMemCopyInfo, chunk_consumer );

    chunk_consumer->process_finish();
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
							     chunk.from_offset,			// Location in the buffer to start writing from.
							     m_ptr+chunk.to_offset,		// A pointer to local memory that should be read from.
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
    	notifyCommandStatusChanged(CL_RUNNING);

       	COIEVENT* barrier = NULL;
    	unsigned int numDependecies = 0;
    	m_pCommandSynchHandler->getLastDependentBarrier(m_pCommandList, &barrier, &numDependecies, false);

        assert( (numDependecies < 2) && "Previous command list dependencies may not be more than 1" );

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
        	// set coiBuffer (objPtr) initial offset
        	sCpyParam.from_Offset = offset;
        	memcpy(sCpyParam.vFromPitch, pObjPitchPtr, sizeof(sCpyParam.vFromPitch));

        	// set host pointer with the calculated offset and copy the pitch
        	sCpyParam.to_Offset = ptrOffset;
        	memcpy(sCpyParam.vToPitch, cmdParams->pitch, sizeof(sCpyParam.vToPitch));
        }
        else
        {
        	// set host pointer with the calculated offset and copy the pitch
        	sCpyParam.from_Offset = ptrOffset;
        	memcpy(sCpyParam.vFromPitch, cmdParams->pitch, sizeof(sCpyParam.vFromPitch));

        	// set coiBuffer (objPtr) initial offset
        	sCpyParam.to_Offset = offset;
        	memcpy(sCpyParam.vToPitch, pObjPitchPtr, sizeof(sCpyParam.vToPitch));
        }

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

        m_completionBarrier = copier.get_last_event();
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

CopyMemObject::CopyMemObject(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd) : BufferCommands(pCommandList, pFrameworkCallBacks, pCmd)
{
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

		// No we can notify that we are running
		notifyCommandStatusChanged(CL_RUNNING);

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

        assert( (numDependecies < 2) && "Previous command list dependencies may not be more than 1" );

        if (numDependecies > 1)
        {
    		m_lastError = CL_DEV_NOT_SUPPORTED;
            break;
        }

        CopyMemoryChunk copier(
                            barrier,
                            pMicMemObjSrc->clDevMemObjGetCoiBufferHandler(), // Get the COIBUFFER which represent this memory object
                            pMicMemObjDst->clDevMemObjGetCoiBufferHandler()  // Get the COIBUFFER which represent this memory object
                           );

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

        m_completionBarrier = copier.get_last_event();
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

	notifyCommandStatusChanged(CL_RUNNING);

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

	    assert( (numDependecies < 2) && "Previous command list dependencies may not be more than 1" );

        if (numDependecies > 1)
        {
            m_lastError = CL_DEV_NOT_SUPPORTED;
            break;
        }

		MapMemoryChunk copier(
                               barrier,
                               pMicMemObj->clDevMemObjGetCoiBufferHandler(), // Get the COIBUFFER which represent this memory object
                               cmdParams->ptr,
							   COI_MAP_READ_WRITE, 
                               MemoryAllocator::GetCoiMapParams(cmdParams) ); // Get the 'SMemMapParamsList' pointer of this memory object

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

        m_completionBarrier = copier.get_last_event();
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

	notifyCommandStatusChanged(CL_RUNNING);

	bool error = false;
	// The do .... while (0) is a pattern when there are many failures points instead of goto operation use do ... while (0) with break commands.
	do
	{		
		COIEVENT* barrier = NULL;
		unsigned int numDependecies = 0;
		m_pCommandSynchHandler->getLastDependentBarrier(m_pCommandList, &barrier, &numDependecies, false);

		assert( (numDependecies < 2) && "Previous command list dependencies may not be more than 1" );

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

		UnmapMemoryChunk unmapper( barrier );

		// Init map handler iterator and traversing over it.
		coiMapParam.initMapHandleIterator();
		while (coiMapParam.hasNextMapHandle())
		{
			UnmapMemoryChunkStruct::Chunk tChunk(coiMapParam.getNextMapHandle());
			unmapper.process_chunk(tChunk);
		}

		// Call to 'process_finish()' in order to complete the last unmap operations.
		unmapper.process_finish();

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

        m_completionBarrier = unmapper.get_last_event();
        m_lastError = CL_DEV_SUCCESS;
	}
	while (0);

	return executePostDispatchProcess(false, error);
}
