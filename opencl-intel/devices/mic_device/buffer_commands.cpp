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
	cl_mem_obj_descriptor*	pMemObj;
	MICDevMemoryObject*     pMicMemObj = (MICDevMemoryObject*)*((IOCLDevMemoryObject**)cmdParams->memObj);
	mem_copy_info_struct	sCpyParam;

	notifyCommandStatusChanged(CL_RUNNING);

	pMicMemObj->clDevMemObjGetDescriptor(CL_DEVICE_TYPE_ACCELERATOR, 0, (cl_dev_memobj_handle*)&pMemObj);

	void*	pObjPtr;
	size_t* pObjPitchPtr = cmdParams->memobj_pitch[0] ? cmdParams->memobj_pitch : pMemObj->pitch;

	// copy the dimension value
	sCpyParam.uiDimCount = cmdParams->dim_count;
	// TODO change it to function that return the offset as uint64_t instead of void* which means that also the argument pMemObj->pData is redandent
	// Currently I send 0 in order to get this functionality (and cast pObjPtr to size_t)
	pObjPtr = MemoryAllocator::CalculateOffsetPointer(0 /*pMemObj->pData*/, sCpyParam.uiDimCount, cmdParams->origin, pObjPitchPtr, pMemObj->uiElementSize);

	// Set region
	memcpy(sCpyParam.vRegion, cmdParams->region, sizeof(sCpyParam.vRegion));
	sCpyParam.vRegion[0] = cmdParams->region[0] * pMemObj->uiElementSize;

	// In case the pointer parameter (Host pointer) has pitch properties,
	// we need to consider that too.
	size_t ptrOffset =	cmdParams->ptr_origin[2] * cmdParams->pitch[1] + \
						cmdParams->ptr_origin[1] * cmdParams->pitch[0] + \
						cmdParams->ptr_origin[0];

	// set host pointer with the claculated offset and copy the pitch
	sCpyParam.pHostPtr = (cl_char*)((size_t)cmdParams->ptr + ptrOffset);
	memcpy(sCpyParam.vHostPitch, cmdParams->pitch, sizeof(sCpyParam.vHostPitch));

	// set coiBuffer (objPtr) initial offset
	sCpyParam.pCoiBuffOffset = (size_t)pObjPtr;
	memcpy(sCpyParam.vCoiBuffPitch, pObjPitchPtr, sizeof(sCpyParam.vCoiBuffPitch));

	// Get estimation for the amount of copy operations
	const unsigned int initVecSize = getEstimatedCopyOperationsAmount(sCpyParam);

	vector<void*> vHostPtr(initVecSize);
	vector<uint64_t> vCoiBuffOffset(initVecSize);
	vector<uint64_t> vSize(initVecSize);

	calculateCopyRegion(&sCpyParam, &vHostPtr, &vCoiBuffOffset, &vSize);

	assert(vHostPtr.size() >= 1);
	assert((vHostPtr.size() == vCoiBuffOffset.size()) && (vCoiBuffOffset.size() == vSize.size()));

	// Hold the size of the vectors returned (Must be >= 1) In case of == 1 it is regular read / write, otherwise it is rectangular read / write.
	size_t retVecSize = vHostPtr.size();

	// Get the COIBUFFER which represent this memory object
	COIBUFFER coiBuffer = pMicMemObj->clDevMemObjGetCoiBufferHandler();

	COIEVENT* barrier = NULL;
	unsigned int numDependecies = 0;
	m_pCommandSynchHandler->getLastDependentBarrier(&barrier, &numDependecies, false);

	COIRESULT result = COI_SUCCESS;

	// If read command
	if ( CL_DEV_CMD_READ == m_pCmd->type )
	{
		if (retVecSize > 1)
		{
			// TODO READRECTANGLE
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

	cl_dev_err_code err = CL_DEV_SUCCESS;
	if (COI_SUCCESS == result)
	{
		// Set m_completionBarrier to be the last barrier in case of InOrder CommandList.
		m_pCommandSynchHandler->setLastDependentBarrier(m_completionBarrier, false);
		// Register m_completionBarrier to NotificationPort
		m_pCommandList->getNotificationPort()->addBarrier(m_completionBarrier, this, NULL);
	}
	else
	{
		err = CL_DEV_ERROR_FAIL;
		m_lastError = CL_DEV_ERROR_FAIL;
	}

	notifyCommandStatusChanged(CL_COMPLETE);

	if (CL_DEV_SUCCESS != err)
	{
		delete this;
	}

	return err;
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
	cl_mem_obj_descriptor*	pSrcMemObj;;
	cl_mem_obj_descriptor*	pDstMemObj;
	MICDevMemoryObject*     pMicMemObjSrc = (MICDevMemoryObject*)*((IOCLDevMemoryObject**)cmdParams->srcMemObj);
	MICDevMemoryObject*     pMicMemObjDst = (MICDevMemoryObject*)*((IOCLDevMemoryObject**)cmdParams->dstMemObj);
	mem_copy_info_struct	sCpyParam;  // Assume in this case that the source is hostPtr and the destination is coiBuffer (Will convert later the results of the source)

	pMicMemObjSrc->clDevMemObjGetDescriptor(CL_DEVICE_TYPE_ACCELERATOR, 0, (cl_dev_memobj_handle*)&pSrcMemObj);
	pMicMemObjDst->clDevMemObjGetDescriptor(CL_DEVICE_TYPE_CPU, 0, (cl_dev_memobj_handle*)&pDstMemObj);

	size_t  uiSrcElementSize = pSrcMemObj->uiElementSize;
	size_t	uiDstElementSize = pDstMemObj->uiElementSize;

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
		memcpy(sCpyParam.vHostPitch, cmdParams->src_pitch[0] ? cmdParams->src_pitch : pSrcMemObj->pitch, sizeof(sCpyParam.vHostPitch));
		memcpy(sCpyParam.vCoiBuffPitch, cmdParams->dst_pitch[0] ? cmdParams->dst_pitch : pDstMemObj->pitch, sizeof(sCpyParam.vCoiBuffPitch));

		// TODO change it to function that return the offset as uint64_t instead of void* which means that also the argument pMemObj->pData is redandent
		// Currently I send 0 in order to get this functionality (and cast pObjPtr to size_t)
		sCpyParam.pHostPtr = (cl_char*)MemoryAllocator::CalculateOffsetPointer(0 /*pSrcMemObj->pData*/, cmdParams->src_dim_count, cmdParams->src_origin, sCpyParam.vHostPitch, pSrcMemObj->uiElementSize);
		sCpyParam.pCoiBuffOffset = (uint64_t)((size_t)MemoryAllocator::CalculateOffsetPointer(0 /*pDstMemObj->pData*/, cmdParams->dst_dim_count, cmdParams->dst_origin, sCpyParam.vCoiBuffPitch, pDstMemObj->uiElementSize));

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

		vector<void*> vHostPtrSrc(initVecSize);
		vector<uint64_t> vCoiBuffOffsetSrc(initVecSize);
		vector<uint64_t> vCoiBuffOffsetDst(initVecSize);
		vector<uint64_t> vSize(initVecSize);

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
		m_pCommandSynchHandler->getLastDependentBarrier(&barrier, &numDependecies, false);

		COIRESULT result = COI_SUCCESS;

		if (retVecSize > 1)
		{
			// TODO RECTANGULAR COPY
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
		m_pCommandSynchHandler->setLastDependentBarrier(m_completionBarrier, false);
		// Register m_completionBarrier to NotificationPort
		m_pCommandList->getNotificationPort()->addBarrier(m_completionBarrier, this, NULL);

		notifyCommandStatusChanged(CL_COMPLETE);

		return CL_DEV_SUCCESS;
	}
	while (0);

	notifyCommandStatusChanged(CL_COMPLETE);
	
	cl_dev_err_code err = m_lastError;

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
	//TODO
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
	//TODO
	return CL_DEV_SUCCESS;
}
