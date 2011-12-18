#include "buffer_commands.h"
#include "memory_allocator.h"
#include "command_list.h"

#include <source/COIBuffer_source.h>

#include "cl_types.h"
#include "cl_utils.h"

#include <cstring>

using namespace Intel::OpenCL::MICDevice;

BufferCommands::BufferCommands(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd) : Command(pCommandList, pFrameworkCallBacks, pCmd)
{
}

BufferCommands::~BufferCommands()
{
}

void BufferCommands::calculateCopyRegion(mem_copy_info_struct* pMemCopyInfo, vector<void*>* vHostPtr, vector<uint64_t>* vCoiBuffOffset, vector<uint64_t>* vSize)
{
	// Leaf case 1D array only
	if ( 1 == pMemCopyInfo->uiDimCount )
	{
		unsigned int vectorSize = vHostPtr->size();
		// Optimization - if it is continues memory, merge the last memory chunk with this memory chunk.
		// TODO - There is option for one more optimization case (when pitch is different than raw size), currenntly it will not enter to the next optimization, while in some cases it
		// is possible to perform similar optimization if the pitch is similar on both memory objects and it is not rectangular command, than it is possible to read / write continuously.
		if ((vectorSize > 0) && 
			((uint64_t)((size_t)(vHostPtr->at(vectorSize - 1))) +  vSize->at(vectorSize - 1) == (uint64_t)((size_t)(pMemCopyInfo->pHostPtr))) && 
			(vCoiBuffOffset->at(vectorSize - 1) + vSize->at(vectorSize - 1) == pMemCopyInfo->pCoiBuffOffset))
		{
			vSize->at(vectorSize - 1) = vSize->at(vectorSize - 1) + pMemCopyInfo->vRegion[0];
			return;
		}
		vHostPtr->push_back(pMemCopyInfo->pHostPtr);
		vCoiBuffOffset->push_back(pMemCopyInfo->pCoiBuffOffset);
		vSize->push_back(pMemCopyInfo->vRegion[0]);
		return;
	}

	mem_copy_info_struct sRecParam;

	// Copy current parameters
	memcpy(&sRecParam, pMemCopyInfo, sizeof(mem_copy_info_struct));
	sRecParam.uiDimCount = pMemCopyInfo->uiDimCount-1;
	// Make recursion
	for(unsigned int i=0; i<pMemCopyInfo->vRegion[sRecParam.uiDimCount]; ++i)
	{
		calculateCopyRegion(&sRecParam, vHostPtr, vCoiBuffOffset, vSize);
		sRecParam.pHostPtr = sRecParam.pHostPtr + pMemCopyInfo->vHostPitch[sRecParam.uiDimCount-1];
		sRecParam.pCoiBuffOffset = sRecParam.pCoiBuffOffset + pMemCopyInfo->vCoiBuffPitch[sRecParam.uiDimCount-1];
	}
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

	notifyCommandStatusChanged(CL_RUNNING);

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

	// set host pointer with the claculated offset and copy the pitch
	sCpyParam.pHostPtr = (cl_char*)((size_t)cmdParams->ptr + ptrOffset);
	memcpy(sCpyParam.vHostPitch, cmdParams->pitch, sizeof(sCpyParam.vHostPitch));

	// set coiBuffer (objPtr) initial offset
	sCpyParam.pCoiBuffOffset = offset;
	memcpy(sCpyParam.vCoiBuffPitch, pObjPitchPtr, sizeof(sCpyParam.vCoiBuffPitch));

	// Get estimation for the amount of copy operations
	const unsigned int initVecSize = getEstimatedCopyOperationsAmount(sCpyParam);

	vector<void*> vHostPtr;
	vector<uint64_t> vCoiBuffOffset;
	vector<uint64_t> vSize;
	vHostPtr.reserve(initVecSize);
	vCoiBuffOffset.reserve(initVecSize);
	vSize.reserve(initVecSize);

	calculateCopyRegion(&sCpyParam, &vHostPtr, &vCoiBuffOffset, &vSize);

	assert(vHostPtr.size() >= 1);
	assert((vHostPtr.size() == vCoiBuffOffset.size()) && (vCoiBuffOffset.size() == vSize.size()));

	// Hold the size of the vectors returned (Must be >= 1) In case of == 1 it is regular read / write, otherwise it is rectangular read / write.
	size_t retVecSize = vHostPtr.size();

	// Get the COIBUFFER which represent this memory object
	COIBUFFER coiBuffer = pMicMemObj->clDevMemObjGetCoiBufferHandler();

	COIEVENT* barrier = NULL;
	unsigned int numDependecies = 0;
	m_pCommandSynchHandler->getLastDependentBarrier(m_pCommandList, &barrier, &numDependecies, false);

	COIRESULT result = COI_SUCCESS;

	// If read command
	if ( CL_DEV_CMD_READ == m_pCmd->type )
	{
		if (retVecSize > 1)
		{
			// TODO READRECTANGLE
			assert(0);
		}
		else
		{
			result = COIBufferRead ( coiBuffer,					// Buffer to read from.
								     vCoiBuffOffset[0],			// Location in the buffer to start reading from.
								     vHostPtr[0],				// A pointer to local memory that should be written into.
								     vSize[0],					// The number of bytes to write from coiBuffer into vHostPtr[0].
								     COI_COPY_UNSPECIFIED,		// The type of copy operation to use. (//TODO check option to change the type in order to improve performance)
								     numDependecies,			// The number of dependencies specified.
								     barrier,					// An optional array of handles to previously created COIEVENT objects that this read operation will wait for before starting.
								     &m_completionBarrier		// An optional event to be signaled when the copy has completed.
								   );
		}
	}
	else
	{
		// write command
		if (retVecSize > 1)
		{
			// TODO WRITERECTANGLE
			assert(0);
		}
		else
		{
			result = COIBufferWrite ( coiBuffer,				// Buffer to write to.
								     vCoiBuffOffset[0],			// Location in the buffer to start writing from.
								     vHostPtr[0],				// A pointer to local memory that should be read from.
								     vSize[0],					// The number of bytes to write from vHostPtr[0] into coiBuffer.
								     COI_COPY_UNSPECIFIED,		// The type of copy operation to use. (//TODO check option to change the type in order to improve performance)
								     numDependecies,			// The number of dependencies specified.
								     barrier,					// An optional array of handles to previously created COIEVENT objects that this write operation will wait for before starting.
								     &m_completionBarrier		// An optional event to be signaled when the copy has completed.
								   );
		}
	}

	if (COI_SUCCESS == result)
	{
		// Set m_completionBarrier to be the last barrier in case of InOrder CommandList.
		m_pCommandSynchHandler->setLastDependentBarrier(m_pCommandList, m_completionBarrier, false);
		// Register m_completionBarrier to NotificationPort
		m_pCommandList->getNotificationPort()->addBarrier(m_completionBarrier, this, NULL);
	}
	else
	{
		m_lastError = CL_DEV_ERROR_FAIL;
		notifyCommandStatusChanged(CL_COMPLETE);
		delete this;
		return CL_DEV_ERROR_FAIL;
	}

	return CL_DEV_SUCCESS;
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
		memcpy(sCpyParam.vHostPitch, cmdParams->src_pitch[0] ? cmdParams->src_pitch : pSrcMemObj.pitch, sizeof(sCpyParam.vHostPitch));
		memcpy(sCpyParam.vCoiBuffPitch, cmdParams->dst_pitch[0] ? cmdParams->dst_pitch : pDstMemObj.pitch, sizeof(sCpyParam.vCoiBuffPitch));

		sCpyParam.pHostPtr = (cl_char*)MemoryAllocator::CalculateOffset(cmdParams->src_dim_count, cmdParams->src_origin, sCpyParam.vHostPitch, pSrcMemObj.uiElementSize);
		sCpyParam.pCoiBuffOffset = (uint64_t)(MemoryAllocator::CalculateOffset(cmdParams->dst_dim_count, cmdParams->dst_origin, sCpyParam.vCoiBuffPitch, pDstMemObj.uiElementSize));

		sCpyParam.uiDimCount = min(cmdParams->src_dim_count, cmdParams->dst_dim_count);
		if(cmdParams->dst_dim_count != cmdParams->src_dim_count)
		{
			//Buffer to image
			if(1 == cmdParams->src_dim_count)
			{
				uiSrcElementSize = uiDstElementSize;
				sCpyParam.uiDimCount = cmdParams->dst_dim_count;
				sCpyParam.vHostPitch[0] = cmdParams->region[0] * uiDstElementSize;
				sCpyParam.vHostPitch[1] = sCpyParam.vHostPitch[0] * cmdParams->region[1];
			}
			if( 1 == cmdParams->dst_dim_count)
			{
				//When destination is buffer the memcpy will be done as if the buffer is an image with height=1
				sCpyParam.uiDimCount = cmdParams->src_dim_count;
				sCpyParam.vCoiBuffPitch[0] = cmdParams->region[0] * uiSrcElementSize;
				sCpyParam.vCoiBuffPitch[1] = sCpyParam.vCoiBuffPitch[0] * cmdParams->region[1];
			}
		}

		//If row_pitch (or input_row_pitch) is set to 0, the appropriate row pitch is calculated
		//based on the size of each element in bytes multiplied by width.
		memcpy(sCpyParam.vRegion, cmdParams->region, sizeof(sCpyParam.vRegion));
		sCpyParam.vRegion[0] *= uiSrcElementSize;

		// Get estimation for the amount of copy operations
		const unsigned int initVecSize = getEstimatedCopyOperationsAmount(sCpyParam);

		vector<void*> vHostPtrSrc;
		vector<uint64_t> vCoiBuffOffsetSrc;
		vector<uint64_t> vCoiBuffOffsetDst;
		vector<uint64_t> vSize;
		vHostPtrSrc.reserve(initVecSize);
		vCoiBuffOffsetSrc.reserve(initVecSize);
		vCoiBuffOffsetDst.reserve(initVecSize);
		vSize.reserve(initVecSize);

		calculateCopyRegion(&sCpyParam, &vHostPtrSrc, &vCoiBuffOffsetDst, &vSize);

		assert(vHostPtrSrc.size() >= 1);
		assert((vHostPtrSrc.size() == vCoiBuffOffsetDst.size()) && (vCoiBuffOffsetDst.size() == vSize.size()));

		// Hold the size of the vectors returned (Must be >= 1) In case of == 1 it is regular copy, otherwise it is rectangular copy.
		size_t retVecSize = vHostPtrSrc.size();

		// convert the void* offsets of the source to uint64_t offsets
		for (unsigned int i = 0; i < retVecSize; i++)
		{
			vCoiBuffOffsetSrc.push_back((size_t)(vHostPtrSrc[i]));
		}

		// Get the COIBUFFER which represent this memory object
		COIBUFFER coiBufferSrc = pMicMemObjSrc->clDevMemObjGetCoiBufferHandler();
		COIBUFFER coiBufferDst = pMicMemObjDst->clDevMemObjGetCoiBufferHandler();

		COIEVENT* barrier = NULL;
		unsigned int numDependecies = 0;
		m_pCommandSynchHandler->getLastDependentBarrier(m_pCommandList, &barrier, &numDependecies, false);

		COIRESULT result = COI_SUCCESS;

		if (retVecSize > 1)
		{
			// TODO RECTANGULAR COPY
			assert(0);
		}
		else
		{
			result = COIBufferCopy ( coiBufferDst,					// Buffer to copy into.
				                     coiBufferSrc,					// Buffer to copy from.
									 vCoiBuffOffsetDst[0],			// Location in the destination buffer to start writing to.
									 vCoiBuffOffsetSrc[0],			// Location in the source buffer to start reading from.
									 vSize[0],						// The number of bytes to copy from coiBufferSrc into coiBufferSrc.
									 COI_COPY_UNSPECIFIED,			// The type of copy operation to use.
									 numDependecies,				// The number of dependencies.
									 barrier,						// An optional array of handles to previously created COIEVENT objects that this copy operation will wait for before starting.
									 &m_completionBarrier		// An optional event to be signaled when the copy has completed.
									 );
		}
		if (COI_SUCCESS != result)
		{
			m_lastError = CL_DEV_ERROR_FAIL;
			break;
		}

		// Set m_completionBarrier to be the last barrier in case of InOrder CommandList.
		m_pCommandSynchHandler->setLastDependentBarrier(m_pCommandList, m_completionBarrier, false);
		// Register m_completionBarrier to NotificationPort
		m_pCommandList->getNotificationPort()->addBarrier(m_completionBarrier, this, NULL);

		return CL_DEV_SUCCESS;
	}
	while (0);

	notifyCommandStatusChanged(CL_COMPLETE);

	err = m_lastError;

	delete this;
	return err;
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

	sCpyParam.pCoiBuffOffset = MemoryAllocator::CalculateOffset(pMemObj.dim_count, cmdParams->origin, pMemObj.pitch, pMemObj.uiElementSize);
	memcpy(sCpyParam.vCoiBuffPitch, pMemObj.pitch, sizeof(sCpyParam.vCoiBuffPitch));

	// Setup data for copying
	// Set Source/Destination
	sCpyParam.uiDimCount = cmdParams->dim_count;
	memcpy(sCpyParam.vRegion, cmdParams->region, sizeof(sCpyParam.vRegion));
	sCpyParam.vRegion[0] = cmdParams->region[0] * pMemObj.uiElementSize;

	sCpyParam.pHostPtr = (cl_char*)cmdParams->ptr;
	memcpy(sCpyParam.vHostPitch, cmdParams->pitch, sizeof(sCpyParam.vHostPitch));

	// Get estimation for the amount of copy operations
	const unsigned int initVecSize = getEstimatedCopyOperationsAmount(sCpyParam);

	vector<void*> vHostPtr;
	vector<uint64_t> vCoiBuffOffset;
	vector<uint64_t> vSize;
	vHostPtr.reserve(initVecSize);
	vCoiBuffOffset.reserve(initVecSize);
	vSize.reserve(initVecSize);

	calculateCopyRegion(&sCpyParam, &vHostPtr, &vCoiBuffOffset, &vSize);

	assert(vHostPtr.size() >= 1);
	assert((vHostPtr.size() == vCoiBuffOffset.size()) && (vCoiBuffOffset.size() == vSize.size()));

	// Hold the size of the vectors returned (Must be >= 1) In case of == 1 it is regular read / write, otherwise it is rectangular read / write.
	size_t retVecSize = vHostPtr.size();

	// Get the COIBUFFER which represent this memory object
	COIBUFFER coiBuffer = pMicMemObj->clDevMemObjGetCoiBufferHandler();

	COIEVENT* barrier = NULL;
	unsigned int numDependecies = 0;
	m_pCommandSynchHandler->getLastDependentBarrier(m_pCommandList, &barrier, &numDependecies, false);

	SMemMapParams coiMapParam;

	// The do .... while (0) is a pattern when there are many failures points instead of goto operation use do ... while (0) with break commands.
	do
	{
		COIRESULT coiResult = COI_SUCCESS;

		if (retVecSize > 1)
		{
			// Rectangular map operations
			COIEVENT* mapBarriers = new COIEVENT[retVecSize];
			// Perform the first (retVecSize - 1) map operations. (asynchronous operations, only dependent on the previous command in the CommandList)
			for (unsigned int i = 0; i < retVecSize - 1; i++)
			{
				COIMAPINSTANCE currentMapInstance;
				coiResult = COIBufferMap ( coiBuffer,				            // Handle for the buffer to map.
										vCoiBuffOffset[i],                      // Offset into the buffer that a pointer should be returned for.
										vSize[i],				                // Length of the buffer area to map.
										COI_MAP_READ_WRITE,                     // The access type that is needed by the application.
										numDependecies,							// The number of dependencies specified in the barrier array.
										barrier,						        // An optional array of handles to previously created COIEVENT objects that this map operation will wait for before starting.
										&(mapBarriers[i]),				        // An optional pointer to a COIEVENT object that will be signaled when a map call with the passed in buffer would complete immediately, that is, the buffer memory has been allocated on the host and its contents updated.
										&currentMapInstance,				    // A pointer to a COIMAPINSTANCE which represents this mapping of the buffer
										&(vHostPtr[i])
										);

				if (COI_SUCCESS != coiResult)
				{
					m_lastError = CL_DEV_ERROR_FAIL;
					break;
				}
				if (false == coiMapParam.insertMapHandle(currentMapInstance))
				{
					m_lastError = CL_DEV_ERROR_FAIL;
					break;
				}
			}

			COIMAPINSTANCE currentMapInstance;
			// Perform the last map operation that depend on the previous (retVecSize - 1) map operations
			if (CL_DEV_SUCCESS == m_lastError)
			{
				coiResult = COIBufferMap ( coiBuffer,										// Handle for the buffer to map.
										vCoiBuffOffset[retVecSize - 1],						// Offset into the buffer that a pointer should be returned for.
										vSize[retVecSize - 1],								// Length of the buffer area to map.
										COI_MAP_READ_WRITE,									// The access type that is needed by the application.
										retVecSize - 1,										// The number of dependencies specified in the barrier array.
										mapBarriers,										// An optional array of handles to previously created COIEVENT objects that this map operation will wait for before starting.
										&m_completionBarrier,								// An optional pointer to a COIEVENT object that will be signaled when a map call with the passed in buffer would complete immediately, that is, the buffer memory has been allocated on the host and its contents updated.
										&currentMapInstance,								// A pointer to a COIMAPINSTANCE which represents this mapping of the buffer
										&(vHostPtr[retVecSize - 1])
										);
			}

			delete [] mapBarriers;

			if (COI_SUCCESS != coiResult)
			{
				m_lastError = CL_DEV_ERROR_FAIL;
				break;
			}
			if (false == coiMapParam.insertMapHandle(currentMapInstance))
			{
				m_lastError = CL_DEV_ERROR_FAIL;
				break;
			}

		}
		// Regular map operation
		else
		{
			COIMAPINSTANCE currentMapInstance;
			coiResult = COIBufferMap ( coiBuffer,				        // Handle for the buffer to map.
									vCoiBuffOffset[0],              // Offset into the buffer that a pointer should be returned for.
									vSize[0],				        // Length of the buffer area to map.
									COI_MAP_READ_WRITE,             // The access type that is needed by the application.
									numDependecies,                 // The number of dependencies specified in the barrier array.
									barrier,				        // An optional array of handles to previously created COIEVENT objects that this map operation will wait for before starting.
									&m_completionBarrier,           // An optional pointer to a COIEVENT object that will be signaled when a map call with the passed in buffer would complete immediately, that is, the buffer memory has been allocated on the host and its contents updated.
									&currentMapInstance,		    // A pointer to a COIMAPINSTANCE which represents this mapping of the buffer
									&(vHostPtr[0])
									);
			if (COI_SUCCESS != coiResult)
			{
				m_lastError = CL_DEV_ERROR_FAIL;
				break;
			}
			if (false == coiMapParam.insertMapHandle(currentMapInstance))
			{
				m_lastError = CL_DEV_ERROR_FAIL;
				break;
			}
		}

		SMemMapParamsList* coiMapParamList = MemoryAllocator::GetCoiMapParams(cmdParams);
		assert(coiMapParamList);
		coiMapParamList->push(coiMapParam);
	}
	while (0);

	if (CL_DEV_SUCCESS != m_lastError)
	{
		cl_dev_err_code tErr = m_lastError;
		notifyCommandStatusChanged(CL_COMPLETE);
		delete this;
		return tErr;
	}

	// Set m_completionBarrier to be the last barrier in case of InOrder CommandList.
	m_pCommandSynchHandler->setLastDependentBarrier(m_pCommandList, m_completionBarrier, false);
	// Register m_completionBarrier to NotificationPort
	m_pCommandList->getNotificationPort()->addBarrier(m_completionBarrier, this, NULL);

	return CL_DEV_SUCCESS;
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

	COIEVENT* barrier = NULL;
	unsigned int numDependecies = 0;
	m_pCommandSynchHandler->getLastDependentBarrier(m_pCommandList, &barrier, &numDependecies, false);

	SMemMapParamsList* coiMapParamList = MemoryAllocator::GetCoiMapParams(cmdParams);
	assert(coiMapParamList);

	// The do .... while (0) is a pattern when there are many failures points instead of goto operation use do ... while (0) with break commands.
	do
	{
		COIRESULT result = COI_SUCCESS;

		SMemMapParams coiMapParam;
		// Get arbitrary instance of SMemMapParams
		if (false == coiMapParamList->pop(&coiMapParam))
		{
			m_lastError = CL_DEV_ERROR_FAIL;
			break;
		}

		size_t numOfMapHandles = coiMapParam.getNumMapInstances();

		// Rectangular unmap operation
		if (numOfMapHandles > 1)
		{
			// In rectangular mapping must be more than one map handle
			COIEVENT* unmapBarriers = new COIEVENT[numOfMapHandles - 1];
			unsigned int index = 0;
			coiMapParam.initMapHandleIterator();
			while ((index < numOfMapHandles - 1) &&(coiMapParam.hasNextMapHandle()))
			{
				result = COIBufferUnmap ( coiMapParam.getNextMapHandle(),				 // Buffer map instance handle to unmap.
											numDependecies,								 // The number of dependencies specified in the barrier array.
											barrier,									 // An optional array of handles to previously created COIEVENT objects that this unmap operation will wait for before starting.
											&(unmapBarriers[index])							 // An optional pointer to a COIEVENT object that will be signaled when the unmap is complete.
											);
				if (COI_SUCCESS != result)
				{
					m_lastError = CL_DEV_ERROR_FAIL;
					break;
				}
				index ++;
			}

			if (CL_DEV_SUCCESS == m_lastError)
			{
				if ((index != numOfMapHandles - 1) || (false == coiMapParam.hasNextMapHandle()))
				{
					m_lastError = CL_DEV_ERROR_FAIL;
					delete [] unmapBarriers;
					break;
				}
				result = COIBufferUnmap ( coiMapParam.getNextMapHandle(),							// Buffer map instance handle to unmap.
										  numOfMapHandles - 1,									    // The number of dependencies specified in the barrier array.
										  unmapBarriers,											// An optional array of handles to previously created COIEVENT objects that this unmap operation will wait for before starting.
										  &m_completionBarrier										// An optional pointer to a COIEVENT object that will be signaled when the unmap is complete.
										);
			}
			delete [] unmapBarriers;
			if (COI_SUCCESS != result)
			{
				m_lastError = CL_DEV_ERROR_FAIL;
				break;
			}
		}
		// Regular unmap
		else
		{
			coiMapParam.initMapHandleIterator();
			if ((numOfMapHandles == 0) || (false == coiMapParam.hasNextMapHandle()))
			{
				m_lastError = CL_DEV_ERROR_FAIL;
				break;
			}
			result = COIBufferUnmap ( coiMapParam.getNextMapHandle(),          // Buffer map instance handle to unmap.
									  numDependecies,							// The number of dependencies specified in the barrier array.
									  barrier,									// An optional array of handles to previously created COIEVENT objects that this unmap operation will wait for before starting.
									  &m_completionBarrier						// An optional pointer to a COIEVENT object that will be signaled when the unmap is complete.
									);
			if (COI_SUCCESS != result)
			{
				m_lastError = CL_DEV_ERROR_FAIL;
				break;
			}
		}
	}
	while (0);

	if (CL_DEV_SUCCESS != m_lastError)
	{
		cl_dev_err_code tErr = m_lastError;
		notifyCommandStatusChanged(CL_COMPLETE);
		delete this;
		return tErr;
	}

	// Set m_completionBarrier to be the last barrier in case of InOrder CommandList.
	m_pCommandSynchHandler->setLastDependentBarrier(m_pCommandList, m_completionBarrier, false);
	// Register m_completionBarrier to NotificationPort
	m_pCommandList->getNotificationPort()->addBarrier(m_completionBarrier, this, NULL);

	return CL_DEV_SUCCESS;

}
