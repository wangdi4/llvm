// Copyright (c) 2006-2008 Intel Corporation
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

///////////////////////////////////////////////////////////////////////////
//  dispatcher_commands.cpp
//  Implementation of the execution of internal task dispatcher commands
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "dispatcher_commands.h"
#include "task_dispatcher.h"
#include "cl_logger.h"
#include "cpu_dev_limits.h"

using namespace Intel::OpenCL::CPUDevice;
using namespace Intel::OpenCL;


///////////////////////////////////////////////////////////////////////////
// Base dispatcher command
DispatcherCommand::DispatcherCommand(TaskDispatcher* pTD, cl_dev_log_descriptor* pLogDesc, cl_int iLogHandle) :
m_pTaskDispatcher(pTD), m_iLogHandle(iLogHandle)
{
	if ( NULL != pLogDesc )
	{
		memcpy(&m_logDescriptor, pLogDesc, sizeof(cl_dev_log_descriptor));
	}
	else
	{
		memset(&m_logDescriptor, 0, sizeof(cl_dev_log_descriptor));
	}

	m_pMemAlloc = pTD->m_pMemoryAllocator;
	m_pTaskExec = pTD->m_pTaskExecutor;
	m_pProgService = pTD->m_pProgramService;
}

void DispatcherCommand::NotifyDispatcher(cl_dev_cmd_desc *pCmd)
{
	m_pTaskDispatcher->NotifyCommandCompletion(pCmd);
}


///////////////////////////////////////////////////////////////////////////
// OCL Read/Write buffer execution
ReadWriteMemObject::ReadWriteMemObject(TaskDispatcher* pTD, cl_dev_log_descriptor* pLogDesc, cl_int iLogHandle) :
	DispatcherCommand(pTD, pLogDesc, iLogHandle)
{
}

cl_int ReadWriteMemObject::CheckCommandParams(cl_dev_cmd_desc* cmd)
{
	if ( (CL_DEV_CMD_READ != cmd->type) && (CL_DEV_CMD_WRITE != cmd->type) )
	{
		return CL_DEV_INVALID_COMMAND_TYPE;
	}

	if ( sizeof(cl_dev_cmd_param_rw) != cmd->param_size )
	{
		return CL_DEV_INVALID_COMMAND_PARAM;
	}

	cl_dev_cmd_param_rw *cmdParams = (cl_dev_cmd_param_rw*)(cmd->params);

	cl_int ret = m_pMemAlloc->ValidateObject(cmdParams->memObj);
	if ( CL_DEV_FAILED(ret) )
	{
		return ret;
	}

	// Check Region
	return CL_DEV_SUCCESS;
}

cl_int ReadWriteMemObject::ExecuteCommand(cl_dev_cmd_desc* cmd, TTaskHandle* pDepList, unsigned int uiCount, TTaskHandle* pNewHandle)
{
	cl_dev_cmd_param_rw *cmdParams = (cl_dev_cmd_param_rw*)(cmd->params);
	
	SMemCpyParams*	pCpyParam = new SMemCpyParams;
	if ( NULL == pCpyParam )
	{
		ErrLog(m_logDescriptor, m_iLogHandle, L"Can't allocate memory for parameters");
		return CL_DEV_OUT_OF_MEMORY;
	}

	void*	pObjPtr;
	size_t	pPitch[MAX_WORK_DIM];
	size_t	pObjPitch[MAX_WORK_DIM];
	size_t  uiElementSize;
	bool    bValidCmdPitch = true;
	// Lock memory object
	cl_int ret = m_pMemAlloc->LockObject(cmdParams->memObj, cmdParams->dim_count, cmdParams->origin, &pObjPtr, pPitch, pObjPitch, &uiElementSize);
	if ( CL_DEV_FAILED(ret) )
	{
		delete pCpyParam;
		return ret;
	}

	pCpyParam->pCmd = cmd;
	//If row_pitch (or input_row_pitch) is set to 0, the appropriate row pitch is calculated
	//based on the size of each element in bytes multiplied by width.

	// Set Source/Destination
	pCpyParam->uiDimCount = cmdParams->dim_count;
	memcpy(pCpyParam->vRegion, cmdParams->region, sizeof(cmdParams->region));
	pCpyParam->vRegion[0] = cmdParams->region[0] * uiElementSize;
	
	for (unsigned int i=0; i< pCpyParam->uiDimCount-1; i++)
	{
		if(0 == cmdParams->pitch[i])
		{
			bValidCmdPitch = false;
		}
	}
	if ( CL_DEV_CMD_READ == cmd->type )
	{
		pCpyParam->pSrc = (cl_char*)pObjPtr;
		pCpyParam->pSrcPitch = pPitch;
		pCpyParam->pDst = (cl_char*)cmdParams->ptr;
		if(bValidCmdPitch)
		{
			pCpyParam->pDstPitch = cmdParams->pitch;
		}
		else
		{
			pCpyParam->pDstPitch = pObjPitch;
		}

	} else
	{
		pCpyParam->pSrc = (cl_char*)cmdParams->ptr;
		if(bValidCmdPitch)
		{
			pCpyParam->pSrcPitch = cmdParams->pitch;
		}
		else
		{
			pCpyParam->pSrcPitch = pObjPitch;
		}
		
		pCpyParam->pDst = (cl_char*)pObjPtr;
		pCpyParam->pDstPitch = pPitch;
	}

	// Currently handle Read/Write as single task
	// TODO: Check run Read/Write as kernel on multiple cores
	ETERetCode teRet;
	teRet = m_pTaskExec->ExecuteFunction(CopyMemoryBuffer, pCpyParam, sizeof(SMemCpyParams), &NotifyCommandCompletion, this, pDepList, uiCount, pNewHandle);
	if ( TE_SUCCESS != teRet )
	{
		m_pMemAlloc->UnLockObject(cmdParams->memObj, pObjPtr);
		ErrLog(m_logDescriptor, m_iLogHandle, L"Failed to execute function %X", teRet);
		return CL_DEV_ERROR_FAIL;
	}
	
	return CL_DEV_SUCCESS;
}

void ReadWriteMemObject::NotifyCommandCompletion(TTaskHandle hTask, void* pParams, size_t size, void* pData)
{
	ReadWriteMemObject*		pThis = (ReadWriteMemObject*)pData;
	SMemCpyParams*			pCpyParam = (SMemCpyParams*)pParams;
	cl_dev_cmd_desc*		pCmd = pCpyParam->pCmd;
	cl_dev_cmd_param_rw*	cmdParams = (cl_dev_cmd_param_rw*)pCmd->params;
	void*					pObjPtr;

	if ( CL_DEV_CMD_READ == pCmd->type )
	{
		pObjPtr = pCpyParam->pSrc;
	} else
	{
		pObjPtr = pCpyParam->pDst;
	}

	cl_int ret = pThis->m_pMemAlloc->UnLockObject(cmdParams->memObj, pObjPtr);
	if ( CL_DEV_FAILED(ret) )
	{
		ErrLog(pThis->m_logDescriptor, pThis->m_iLogHandle, L"Can't unlock memory object");
	}

	delete pCpyParam;

	pThis->NotifyDispatcher(pCmd);
}

void Intel::OpenCL::CPUDevice::CopyMemoryBuffer(SMemCpyParams* pCopyCmd)
{
	// Copy 1D array only
	if ( 1 == pCopyCmd->uiDimCount )
	{
		memcpy(pCopyCmd->pDst, pCopyCmd->pSrc, pCopyCmd->vRegion[0]);
		return;
	}

	SMemCpyParams sRecParam;

	// Copy current parameters
	memcpy(&sRecParam, pCopyCmd, sizeof(SMemCpyParams));
	sRecParam.uiDimCount = pCopyCmd->uiDimCount-1;
	// Make recoursion
	for(unsigned int i=0; i<pCopyCmd->vRegion[sRecParam.uiDimCount]; ++i)
	{
		CopyMemoryBuffer(&sRecParam);
		sRecParam.pSrc = sRecParam.pSrc + pCopyCmd->pSrcPitch[sRecParam.uiDimCount-1];
		sRecParam.pDst = sRecParam.pDst + pCopyCmd->pDstPitch[sRecParam.uiDimCount-1];
	}
}

///////////////////////////////////////////////////////////////////////////
// OCL Copy memory object execution
CopyMemObject::CopyMemObject(TaskDispatcher* pTD, cl_dev_log_descriptor* pLogDesc, cl_int iLogHandle) :
	DispatcherCommand(pTD, pLogDesc, iLogHandle)
{
}

cl_int CopyMemObject::CheckCommandParams(cl_dev_cmd_desc* cmd)
{
	if(CL_DEV_CMD_COPY != cmd->type)
	{
		return CL_DEV_INVALID_COMMAND_TYPE;
	}

	if ( sizeof(cl_dev_cmd_param_copy) != cmd->param_size )
	{
		return CL_DEV_INVALID_COMMAND_PARAM;
	}

	cl_dev_cmd_param_copy *cmdParams = (cl_dev_cmd_param_copy*)(cmd->params);

	cl_int ret = m_pMemAlloc->ValidateObject(cmdParams->dstMemObj);
	if ( CL_DEV_FAILED(ret) )
	{
		return ret;
	}

	ret = m_pMemAlloc->ValidateObject(cmdParams->srcMemObj);
	if ( CL_DEV_FAILED(ret) )
	{
		return ret;
	}

	return CL_DEV_SUCCESS;
}

cl_int CopyMemObject::ExecuteCommand(cl_dev_cmd_desc* cmd, TTaskHandle* pDepList, unsigned int uiCount, TTaskHandle* pNewHandle)
{
	cl_dev_cmd_param_copy *cmdParams = (cl_dev_cmd_param_copy*)(cmd->params);
	
	SMemCpyParams*	pCpyParam = new SMemCpyParams;
	if ( NULL == pCpyParam )
	{
		ErrLog(m_logDescriptor, m_iLogHandle, L"Can't allocate memory for parameters");
		return CL_DEV_OUT_OF_MEMORY;
	}

	void*	pSrcObjPtr;
	void*	pDstObjPtr;
	size_t  uiSrcElementSize, uiDstElementSize;
    size_t	pSrcPitch[MAX_WORK_DIM], pDstPitch[MAX_WORK_DIM];
	
	// Lock src memory object
	cl_int ret = m_pMemAlloc->LockObject(cmdParams->srcMemObj, cmdParams->src_dim_count, cmdParams->src_origin, &pSrcObjPtr, pSrcPitch, NULL, &uiSrcElementSize);
	if ( CL_DEV_FAILED(ret) )
	{
		return ret;
	}

	// Lock dst memory object
	ret = m_pMemAlloc->LockObject(cmdParams->dstMemObj, cmdParams->dst_dim_count, cmdParams->dst_origin, &pDstObjPtr, pDstPitch, NULL, &uiDstElementSize);
	if ( CL_DEV_FAILED(ret) )
	{
		m_pMemAlloc->UnLockObject(cmdParams->srcMemObj, pSrcObjPtr);
		return ret;
	}
	if(uiDstElementSize != uiSrcElementSize && 1 != uiDstElementSize && 1 != uiSrcElementSize)
	{
		m_pMemAlloc->UnLockObject(cmdParams->srcMemObj, pSrcObjPtr);
		m_pMemAlloc->UnLockObject(cmdParams->dstMemObj, pDstObjPtr);
		return CL_DEV_INVALID_COMMAND_PARAM;
	}
	
	//Options for different dimenssions are
    //Copy a 2D image object to a 2D slice of a 3D image object.
    //Copy a 2D slice of a 3D image object to a 2D image object.
	//Copy 2D to 2D
	//Copy 3D to 3D
	//Copy 2D image to buffer
	//Copy 3D image to buffer
	//Buffer to image
	
	pCpyParam->uiDimCount = min(cmdParams->src_dim_count, cmdParams->dst_dim_count);
	if(cmdParams->dst_dim_count != cmdParams->src_dim_count)
	{
		//Buffer to image
		if(1 == cmdParams->src_dim_count)
		{
			uiSrcElementSize = uiDstElementSize;
			pCpyParam->uiDimCount = cmdParams->dst_dim_count;
			pSrcPitch[0] = cmdParams->region[0] * uiDstElementSize;
			pSrcPitch[1] = pSrcPitch[0] * cmdParams->region[1];			
		}
		if( 1 == cmdParams->dst_dim_count)
		{
			//When destination is buffer the memcpy will be done as if the buffer is an image with height=1 
			pCpyParam->uiDimCount = cmdParams->src_dim_count;
			pDstPitch[0] = cmdParams->region[0] * uiSrcElementSize;
			pDstPitch[1] = pDstPitch[0] * cmdParams->region[1];
		}
	}
	
	//If row_pitch (or input_row_pitch) is set to 0, the appropriate row pitch is calculated
	//based on the size of each element in bytes multiplied by width.
	memcpy(pCpyParam->vRegion, cmdParams->region, sizeof(cmdParams->region));
	pCpyParam->vRegion[0] *= uiSrcElementSize;
	
	pCpyParam->pSrc = (cl_char*)pSrcObjPtr;
	pCpyParam->pSrcPitch = pSrcPitch;
	pCpyParam->pDst = (cl_char*)pDstObjPtr;
	pCpyParam->pDstPitch = pDstPitch;


	pCpyParam->pCmd = cmd;

	// Currently handle Read/Write as single task
	// TODO: Check run Read/Write as kernel on multiple cores
	ETERetCode teRet;
	teRet = m_pTaskExec->ExecuteFunction(CopyMemoryBuffer, pCpyParam, sizeof(SMemCpyParams), &NotifyCommandCompletion, this, pDepList, uiCount, pNewHandle);
	if ( TE_SUCCESS != teRet )
	{
		ErrLog(m_logDescriptor, m_iLogHandle, L"Failed to execute function %X", teRet);
		m_pMemAlloc->UnLockObject(cmdParams->srcMemObj, pSrcObjPtr);
		m_pMemAlloc->UnLockObject(cmdParams->dstMemObj, pDstObjPtr);	
		return CL_DEV_ERROR_FAIL;
	}
	
	return CL_DEV_SUCCESS;
}

void CopyMemObject::NotifyCommandCompletion(TTaskHandle hTask, void* pParams, size_t size, void* pData)
{
	ReadWriteMemObject*		pThis = (ReadWriteMemObject*)pData;
	SMemCpyParams*			pCpyParam = (SMemCpyParams*)pParams;
	cl_dev_cmd_desc*		pCmd = pCpyParam->pCmd;
	cl_dev_cmd_param_copy*	cmdParams = (cl_dev_cmd_param_copy*)pCmd->params;

	cl_int ret = pThis->m_pMemAlloc->UnLockObject(cmdParams->dstMemObj, pCpyParam->pDst);
	if ( CL_DEV_FAILED(ret) )
	{
		ErrLog(pThis->m_logDescriptor, pThis->m_iLogHandle, L"Can't unlock destination memory object");
	}

	ret = pThis->m_pMemAlloc->UnLockObject(cmdParams->srcMemObj, pCpyParam->pSrc);
	if ( CL_DEV_FAILED(ret) )
	{
		ErrLog(pThis->m_logDescriptor, pThis->m_iLogHandle, L"Can't unlock source memory object");
	}

	delete pCpyParam;

	pThis->NotifyDispatcher(pCmd);
}

///////////////////////////////////////////////////////////////////////////
// OCL Kernel execution
KernelCommand::KernelCommand(TaskDispatcher* pTD, cl_dev_log_descriptor* pLogDesc, cl_int iLogHandle) :
	DispatcherCommand(pTD, pLogDesc, iLogHandle)
{
}

cl_int KernelCommand::CheckCommandParams(cl_dev_cmd_desc* cmd)
{
	if ( (CL_DEV_CMD_EXEC_KERNEL != cmd->type) && (CL_DEV_CMD_EXEC_TASK != cmd->type) )
	{
		return CL_DEV_INVALID_COMMAND_TYPE;
	}

	if ( sizeof(cl_dev_cmd_param_kernel) != cmd->param_size )
	{
		return CL_DEV_INVALID_COMMAND_PARAM;
	}

	cl_dev_cmd_param_kernel *cmdParams = (cl_dev_cmd_param_kernel*)(cmd->params);


	const ICLDevKernel*	pKernel;

	// Check if requested kernel exists
	cl_int clRet = m_pProgService->GetKernelObject(cmdParams->kernel, &pKernel);
	if ( CL_DEV_FAILED(clRet) )
	{
		return CL_DEV_INVALID_KERNEL;
	}

	size_t	stLocMemSize = 0;

	// Check kernel paramters
	cl_uint						uiNumArgs = pKernel->GetNumArgs();
	const cl_kernel_argument*	pArgs = pKernel->GetKernelArgs();

	cl_char*	pCurrParamPtr = (cl_char*)cmdParams->arg_values;
	size_t		stOffset = 0;
	// Check kernel parameters and memory buffers
	for(unsigned int i=0; i<uiNumArgs; ++i)
	{
		// Argument is buffer object or local memory size
		if ( (CL_KRNL_ARG_PTR_LOCAL == pArgs[i].type) ||
			(CL_KRNL_ARG_PTR_GLOBAL == pArgs[i].type) ||
			(CL_KRNL_ARG_PTR_CONST == pArgs[i].type)
			)
		{
			cl_dev_mem memObj = (cl_dev_mem)*((void**)(pCurrParamPtr+stOffset));
			if ( -1 != memObj->allocId )	// Required local memory size
			{
				// Is valid memory object
				clRet = m_pMemAlloc->ValidateObject(memObj);
				if ( CL_DEV_FAILED(clRet) )
				{
					return clRet;
				}
			} else
			{
				size_t locSize = (size_t)memObj->objHandle; 
				// strict local buffer size to cache line
				locSize += (CPU_DCU_LINE_SIZE-1);
				locSize &= ~(CPU_DCU_LINE_SIZE);

				stLocMemSize += locSize;
			}
			stOffset += sizeof(void*);
		}
		else
		{
			stOffset += pArgs[i].size_in_bytes;
		}
	}
	// Check paramters array size 
	if ( stOffset != cmdParams->arg_size )
	{
		return CL_DEV_INVALID_COMMAND_PARAM;
	}

	// Check if local memory size is enought for kernel
	if ( CPU_DEV_LCL_MEM_SIZE < stLocMemSize )
	{
		return CL_DEV_INVALID_COMMAND_PARAM;
	}

	// Check Work-Group / Work-Item information
	if ( CL_DEV_CMD_EXEC_KERNEL == cmd->type )
	{
		// Check WG dimensions
		size_t	stWGSize = 1;

		if ( MAX_WORK_DIM < cmdParams->work_dim )
		{
			return CL_DEV_INVALID_WRK_DIM;
		}

		for(unsigned int i=0; i<cmdParams->work_dim; ++i)
		{
			if ( CPU_DEV_MAX_WI_SIZE < cmdParams->lcl_wrk_size[i] )
			{
				return CL_DEV_INVALID_WRK_ITEM_SIZE;
			}

			stWGSize *= cmdParams->lcl_wrk_size[i];
		}

		if ( CPU_DEV_MAX_WG_SIZE < stWGSize )
		{
			return CL_DEV_INVALID_WG_SIZE;
		}
	} else
	{
		// For Task one dimension is required
		if ( 1 != cmdParams->work_dim )
		{
			return CL_DEV_INVALID_WRK_DIM;
		}
		// Work Group size should be 1
		if ( 1 != cmdParams->lcl_wrk_size[0] )
		{
				return CL_DEV_INVALID_WRK_ITEM_SIZE;
		}
		// Work-Group size should be 1
		if ( 1 != cmdParams->glb_wrk_size[0] )
		{
			return CL_DEV_INVALID_WRK_DIM;
		}
	}


	return CL_DEV_SUCCESS;
}

cl_int KernelCommand::ExecuteCommand(cl_dev_cmd_desc* cmd, TTaskHandle* pDepList, unsigned int uiCount, TTaskHandle* pNewHandle)
{
	cl_dev_cmd_param_kernel *cmdParams = (cl_dev_cmd_param_kernel*)(cmd->params);

	const ICLDevKernel* pKernel;

	// Check if requested kernel exists
	cl_int clRet = m_pProgService->GetKernelObject(cmdParams->kernel, &pKernel);
	if ( CL_DEV_FAILED(clRet) )
	{
		return CL_DEV_INVALID_KERNEL;
	}

	// Check kernel paramters
	cl_uint						uiNumArgs = pKernel->GetNumArgs();
	const cl_kernel_argument*	pArgs = pKernel->GetKernelArgs();
	char*						pKernelParams = (char*)cmdParams->arg_values;
	size_t						stOffset = 0;
	// TODO:	consider use memory buffer passed in cmdParams, don't allocate additional memory
	char*						pLocalParams = (char*)malloc(cmdParams->arg_size);

	if ( NULL == pLocalParams )
	{
		return CL_DEV_OUT_OF_MEMORY;
	}

	void*						*pLocalBuffers = (void**)malloc(sizeof(void*)*uiNumArgs);
	cl_uint						uiLocalCount = 0;
	if ( NULL == pLocalBuffers )
	{
		free(pLocalParams);
		return CL_DEV_OUT_OF_MEMORY;
	}

	// Initiate with zeros
	memset(pLocalParams, 0, cmdParams->arg_size);

	// Lock memory buffers and local memory
	for(unsigned int i=0; i<uiNumArgs; ++i)
	{
		// Argument is buffer object or local memory size
		if ( ( CL_KRNL_ARG_PTR_LOCAL == pArgs[i].type ) ||
			 ( CL_KRNL_ARG_PTR_GLOBAL == pArgs[i].type ) ||
			 ( CL_KRNL_ARG_PTR_CONST == pArgs[i].type )
			)

		{
			cl_dev_mem memObj = (cl_dev_mem)*((void**)(pKernelParams+stOffset));
			if ( -1 != memObj->allocId )	// Required local memory size
			{
				// Lock memory object / Get pointer
				clRet = m_pMemAlloc->LockObject(memObj, -1, NULL, (void**)(pLocalParams+stOffset), NULL, NULL, NULL);
				if ( CL_DEV_FAILED(clRet) )
				{
					UnlockMemoryBuffers(pKernel, pKernelParams, pLocalParams);
					free(pLocalBuffers);
					free(pLocalParams);
					return clRet;
				}
			} else
			{
				size_t locSize = (size_t)memObj->objHandle; 
				// strict local buffer size to cache line
				locSize += (CPU_DCU_LINE_SIZE-1);
				locSize &= ~(CPU_DCU_LINE_SIZE);
				pLocalBuffers[uiLocalCount] = (void*)locSize;
				++uiLocalCount;
			}
			stOffset += sizeof(void*);
		}
		else
		{
			memcpy(pLocalParams+stOffset, pKernelParams+stOffset, pArgs[i].size_in_bytes);
			stOffset += pArgs[i].size_in_bytes;
		}
	}

	// Allocate new Task instance
	STaskDescriptor*	pTask = new STaskDescriptor;
	if ( NULL == pTask )
	{
		// Undo operations
		UnlockMemoryBuffers(pKernel, pKernelParams, pLocalParams);
		free(pLocalBuffers);
		free(pLocalParams);
		return CL_DEV_OUT_OF_MEMORY;
	}

	SKernelExecData*	pExecData = new SKernelExecData;
	if ( NULL == pExecData )
	{
		// Undo operations
		UnlockMemoryBuffers(pKernel, pKernelParams, pLocalParams);
		delete pTask;
		free(pLocalBuffers);
		free(pLocalParams);
		return CL_DEV_OUT_OF_MEMORY;
	}

	pExecData->pCmd = cmd;
	pExecData->pCmdExecutor = this;

	// Set kernel information
	pTask->sKernelParam.pfnKernelFunc = pKernel->GetHandle();
	pTask->sKernelParam.pParams = pLocalParams;
	pTask->sKernelParam.szParamSize = cmdParams->arg_size;
	pTask->sKernelParam.uiNumLocal = uiLocalCount;
	pTask->sKernelParam.pLocalPtr = pLocalBuffers;

	// Set WG information
	pTask->sWorkingDim.iWorkDim = cmdParams->work_dim;
	memcpy(pTask->sWorkingDim.viOffset, cmdParams->glb_wrk_offs, sizeof(int)*cmdParams->work_dim);
	memcpy(pTask->sWorkingDim.viGlobalSize, cmdParams->glb_wrk_size, sizeof(int)*cmdParams->work_dim);
	memcpy(pTask->sWorkingDim.viLocalSize, cmdParams->lcl_wrk_size, sizeof(int)*cmdParams->work_dim);

	ETERetCode ret;
	ret = m_pTaskExec->ExecuteTask(pTask, NotifyCommandCompletion, pExecData, pDepList, uiCount, pNewHandle);
	if ( TE_SUCCESS != ret )
	{

		ErrLog(m_logDescriptor, m_iLogHandle, L"Failed to execute function %X", ret);
		UnlockMemoryBuffers(pKernel, pKernelParams, pLocalParams);
		delete pTask;
		free(pLocalBuffers);
		free(pLocalParams);
		delete pExecData;

		return CL_DEV_ERROR_FAIL;
	}

	return CL_DEV_SUCCESS;
}

void KernelCommand::UnlockMemoryBuffers(const ICLDevKernel* pKernel, const char* pKernelParams, const char* pLocalParams)
{
	// Check kernel paramters
	cl_uint						uiNumArgs = pKernel->GetNumArgs();
	const cl_kernel_argument*	pArgs = pKernel->GetKernelArgs();
	size_t						stOffset = 0;

	// Unlock memory buffers
	for(unsigned int i=0; i<uiNumArgs; ++i)
	{
		// Argument is buffer object or local memory size
		if ( ( CL_KRNL_ARG_PTR_LOCAL == pArgs[i].type ) ||
			 ( CL_KRNL_ARG_PTR_GLOBAL == pArgs[i].type ) ||
			 ( CL_KRNL_ARG_PTR_CONST == pArgs[i].type )
			)
		{
			cl_dev_mem memObj = (cl_dev_mem)*((void**)(pKernelParams+stOffset));

			if ( (-1 != memObj->allocId) && (NULL != (pLocalParams+stOffset)) )
			{
				// UnLock memory object / Get pointer
				cl_int clRet = m_pMemAlloc->UnLockObject(memObj,(void*)(pLocalParams+stOffset));
				if ( CL_DEV_FAILED(clRet) )
				{
					ErrLog(m_logDescriptor, m_iLogHandle, L"Can't unlock memory object");
				}
			}
			stOffset += sizeof(void*);
		}
		else
		{
			stOffset += pArgs[i].size_in_bytes;
		}
	}
}

void KernelCommand::NotifyCommandCompletion(TTaskHandle hTask, STaskDescriptor* psTaskDescriptor, void* pData)
{
	SKernelExecData*	pInstance = (SKernelExecData*)pData;
	KernelCommand*		pThis = pInstance->pCmdExecutor;
	cl_dev_cmd_desc*	pCmd = pInstance->pCmd;
	cl_dev_cmd_param_kernel *cmdParams = (cl_dev_cmd_param_kernel*)pCmd->params;

	const ICLDevKernel* pKernel;
	cl_int clRet = pThis->m_pProgService->GetKernelObject(cmdParams->kernel, &pKernel);
	assert(CL_DEV_SUCCEEDED(clRet));


	pThis->UnlockMemoryBuffers(pKernel, (const char*)cmdParams->arg_values, (const char*)psTaskDescriptor->sKernelParam.pParams);

	// Free memory allocated to execution parameters
	free(psTaskDescriptor->sKernelParam.pParams);
	free(psTaskDescriptor->sKernelParam.pLocalPtr);

	delete psTaskDescriptor;
	delete pInstance;

	pThis->NotifyDispatcher(pCmd);

}

///////////////////////////////////////////////////////////////////////////
// OCL Native function execution
NativeFunction::NativeFunction(TaskDispatcher* pTD, cl_dev_log_descriptor* pLogDesc, cl_int iLogHandle) :
	DispatcherCommand(pTD, pLogDesc, iLogHandle)
{
}

cl_int	NativeFunction::CheckCommandParams(cl_dev_cmd_desc* cmd)
{
	if ( CL_DEV_CMD_EXEC_NATIVE != cmd->type )
	{
		return CL_DEV_INVALID_COMMAND_TYPE;
	}

	if ( sizeof(cl_dev_cmd_param_native) != cmd->param_size )
	{
		return CL_DEV_INVALID_COMMAND_PARAM;
	}

	cl_dev_cmd_param_native *cmdParams = (cl_dev_cmd_param_native*)(cmd->params);

	if( NULL == cmdParams->func_ptr )
	{
		return CL_DEV_INVALID_COMMAND_PARAM;
	}

	// Check memory object handles
	for(unsigned int i=0; i<cmdParams->mem_num; ++i )
	{
		cl_dev_mem memObj = *((cl_dev_mem*)(cmdParams->mem_loc[i]));
        
		cl_int ret = m_pMemAlloc->ValidateObject(memObj);
		if ( CL_DEV_FAILED(ret) )
		{
			return ret;
		}
	}

	return CL_DEV_SUCCESS;
}

cl_int	NativeFunction::ExecuteCommand(cl_dev_cmd_desc* cmd, TTaskHandle* pDepList, unsigned int uiCount, TTaskHandle* pNewHandle)
{
	cl_dev_cmd_param_native *cmdParams = (cl_dev_cmd_param_native*)cmd->params;

	fn_clNativeKernel *func = (fn_clNativeKernel*)cmdParams->func_ptr;

	// Create temporal buffer for execution
	void*	pArgV = malloc(cmdParams->args);
	if ( NULL == pArgV )
	{
		ErrLog(m_logDescriptor, m_iLogHandle, L"Can't allocate memory for parameters");
		return CL_DEV_OUT_OF_MEMORY;
	}

	SNativeExecData*	pNativeData = new SNativeExecData;
	if ( NULL == pNativeData )
	{
		ErrLog(m_logDescriptor, m_iLogHandle, L"Can't allocate memory to store execution data");
		free(pArgV);
		return CL_DEV_OUT_OF_MEMORY;
	}

	// Prepare native Task for execution
	memcpy(pArgV, cmdParams->argv, cmdParams->args);

	pNativeData->pCmd = cmd;
	pNativeData->pCmdExecutor = this;

	// Lock Memory objects handles
	for(unsigned int i=0; i<cmdParams->mem_num; ++i )
	{
		cl_dev_mem memObj = *((cl_dev_mem*)cmdParams->mem_loc[i]);
		size_t	Offset = (size_t)cmdParams->mem_loc[i] - (size_t)cmdParams->argv;
		void*	*pMemPtr = (void**)((cl_char*)pArgV+Offset);

		cl_int ret = m_pMemAlloc->LockObject(memObj, -1, NULL, pMemPtr, NULL, NULL, NULL);
		if ( CL_DEV_FAILED(ret) )
		{
			return ret;
		}
	}

	ETERetCode ret;
	ret = m_pTaskExec->ExecuteFunction(func, pArgV, cmdParams->args, &NotifyCommandCompletion, pNativeData, pDepList, uiCount, pNewHandle);
	if ( TE_SUCCESS != ret )
	{
		ErrLog(m_logDescriptor, m_iLogHandle, L"Failed to execute function %X", ret);
		return CL_DEV_ERROR_FAIL;
	}

	return CL_DEV_SUCCESS;
}

void NativeFunction::NotifyCommandCompletion(TTaskHandle hTask, void* pParams, size_t size, void* pData)
{
	SNativeExecData*	pInstance = (SNativeExecData*)pData;
	NativeFunction*		pThis = pInstance->pCmdExecutor;
	cl_dev_cmd_desc*	pCmd = pInstance->pCmd;
	cl_dev_cmd_param_native *cmdParams = (cl_dev_cmd_param_native*)pCmd->params;

	// Unlock memory buffers
	for(unsigned int i=0; i<cmdParams->mem_num; ++i )
	{
		cl_dev_mem memObj = *((cl_dev_mem*)cmdParams->mem_loc[i]);

		size_t	Offset = (size_t)cmdParams->mem_loc[i] - (size_t)pCmd->params;
		void*	*pMemPtr = (void**)(cl_char*)pParams+Offset;

		cl_int ret = pThis->m_pMemAlloc->UnLockObject(memObj, *pMemPtr);
		if ( CL_DEV_FAILED(ret) )
		{
			ErrLog(pThis->m_logDescriptor, pThis->m_iLogHandle, L"Can't unlock memory object");
		}
	}

	// Free memory allocated to execution parameters
	free( pParams );
	delete pInstance;

	pThis->NotifyDispatcher(pCmd);
}

///////////////////////////////////////////////////////////////////////////
// OCL Map buffer execution
//////////////////////////////////////////////////////////////////////////
MapMemObject::MapMemObject(TaskDispatcher* pTD, cl_dev_log_descriptor* pLogDesc, cl_int iLogHandle) :
	DispatcherCommand(pTD, pLogDesc, iLogHandle)
{
}

cl_int MapMemObject::CheckCommandParams(cl_dev_cmd_desc* cmd)
{
	if (CL_DEV_CMD_MAP != cmd->type)
	{
		return CL_DEV_INVALID_COMMAND_TYPE;
	}

	if ( sizeof(cl_dev_cmd_param_map) != cmd->param_size )
	{
		return CL_DEV_INVALID_COMMAND_PARAM;
	}

	cl_dev_cmd_param_map *cmdParams = (cl_dev_cmd_param_map*)(cmd->params);

	cl_int ret = m_pMemAlloc->ValidateObject(cmdParams->memObj);
	if ( CL_DEV_FAILED(ret) )
	{
		return ret;
	}

	return CL_DEV_SUCCESS;
}

cl_int MapMemObject::ExecuteCommand(cl_dev_cmd_desc* cmd, TTaskHandle* pDepList, unsigned int uiCount, TTaskHandle* pNewHandle)
{
	cl_dev_cmd_param_map *cmdParams = (cl_dev_cmd_param_map*)(cmd->params);
	
	
	// Lock memory object
	cl_int ret = m_pMemAlloc->LockObject(cmdParams->memObj, cmdParams->dim_count, cmdParams->origin, &cmdParams->ptr, NULL, NULL, NULL);
	if ( CL_DEV_FAILED(ret) )
	{
		return ret;
	}
	
	//Notiofy command completion
	NotifyDispatcher(cmd);

	return CL_DEV_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////
// OCL Unmap buffer execution
//////////////////////////////////////////////////////////////////////////
UnmapMemObject::UnmapMemObject(TaskDispatcher* pTD, cl_dev_log_descriptor* pLogDesc, cl_int iLogHandle) :
	DispatcherCommand(pTD, pLogDesc, iLogHandle)
{
}

cl_int UnmapMemObject::CheckCommandParams(cl_dev_cmd_desc* cmd)
{
	if (CL_DEV_CMD_UNMAP != cmd->type)
	{
		return CL_DEV_INVALID_COMMAND_TYPE;
	}

	if ( sizeof(cl_dev_cmd_param_unmap) != cmd->param_size )
	{
		return CL_DEV_INVALID_COMMAND_PARAM;
	}

	cl_dev_cmd_param_unmap *cmdParams = (cl_dev_cmd_param_unmap*)(cmd->params);

	cl_int ret = m_pMemAlloc->ValidateObject(cmdParams->memObj);
	if ( CL_DEV_FAILED(ret) )
	{
		return ret;
	}

	//Notiofy command completion
	NotifyDispatcher(cmd);
	return CL_DEV_SUCCESS;
}

cl_int UnmapMemObject::ExecuteCommand(cl_dev_cmd_desc* cmd, TTaskHandle* pDepList, unsigned int uiCount, TTaskHandle* pNewHandle)
{
	cl_dev_cmd_param_unmap *cmdParams = (cl_dev_cmd_param_unmap*)(cmd->params);
	
	// Unlock memory object
	cl_int ret = m_pMemAlloc->UnLockObject(cmdParams->memObj, cmdParams->ptr);
	if ( CL_DEV_FAILED(ret) )
	{
		return ret;
	}


	return CL_DEV_SUCCESS;
}


