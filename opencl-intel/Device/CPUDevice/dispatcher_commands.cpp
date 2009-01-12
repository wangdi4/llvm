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

using namespace Intel::OpenCL::CPUDevice;

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
}

void DispatcherCommand::NotifyDispatcher()
{
	m_pTaskDispatcher->NotifyCommandCompletion(m_pCmd->id);
}

///////////////////////////////////////////////////////////////////////////
// OCL Read/Write buffer execution
ReadWriteBuffer::ReadWriteBuffer(TaskDispatcher* pTD, cl_dev_log_descriptor* pLogDesc, cl_int iLogHandle) :
	DispatcherCommand(pTD, pLogDesc, iLogHandle)
{
}

cl_int ReadWriteBuffer::CheckCommandParams(cl_dev_cmd_desc* cmd)
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

	return CL_DEV_SUCCESS;
}

cl_int ReadWriteBuffer::ExecuteCommand(cl_dev_cmd_desc* cmd, TTaskHandle* pDepList, unsigned int uiCount, TTaskHandle* pNewHandle)
{
	cl_dev_cmd_param_rw *cmdParams = (cl_dev_cmd_param_rw*)(cmd->params);
	
	SMemCpyParams*	pCpyParam = new SMemCpyParams;
	if ( NULL == pCpyParam )
	{
		ErrLog(m_logDescriptor, m_iLogHandle, L"Can't allocate memory for parameters");
		return CL_DEV_OUT_OF_MEMORY;
	}

	void*	pObjPtr;
	size_t	stRowPitch, stSlicePitch;

	// Lock memory object
	cl_int ret = m_pMemAlloc->LockObject(cmdParams->memObj, cmdParams->origin, &pObjPtr, &stRowPitch, &stSlicePitch);
	if ( CL_DEV_FAILED(ret) )
	{
		return ret;
	}

	// Set Source/Destination
	memcpy(pCpyParam->pRegion, cmdParams->region, sizeof(pCpyParam->pRegion));
	if ( CL_DEV_CMD_READ == cmd->type )
	{
		pCpyParam->pSrc = pObjPtr;
		pCpyParam->stSrcRowPitch = stRowPitch;
		pCpyParam->stSrcSlicePitch = stSlicePitch;
		pCpyParam->pDst = cmdParams->ptr;
		pCpyParam->stDstRowPitch = cmdParams->row_pitch;
		pCpyParam->stDstSlicePitch = cmdParams->slice_pitch;
	} else
	{
		pCpyParam->pSrc = cmdParams->ptr;
		pCpyParam->stSrcRowPitch = cmdParams->row_pitch;
		pCpyParam->stSrcSlicePitch = cmdParams->slice_pitch;
		pCpyParam->pDst = pObjPtr;
		pCpyParam->stDstRowPitch = stRowPitch;
		pCpyParam->stDstSlicePitch = stSlicePitch;
	}

	m_pCmd = cmd;

	// Currently handle Read/Write as single task
	// TODO: Check run Read/Write as kernel on multiple cores
	ETERetCode teRet;
	teRet = m_pTaskExec->ExecuteFunction(CopyMemoryBuffer, pCpyParam, sizeof(SMemCpyParams), &NotifyCommandCompletion, this, pDepList, uiCount, pNewHandle);
	if ( TE_SUCCESS != teRet )
	{
		ErrLog(m_logDescriptor, m_iLogHandle, L"Failed to execute function %X", teRet);
		return CL_DEV_ERROR_FAIL;
	}
	
	return CL_DEV_SUCCESS;
}

void ReadWriteBuffer::NotifyCommandCompletion(TTaskHandle hTask, void* pParams, size_t size, void* pData)
{
	ReadWriteBuffer*		pThis = (ReadWriteBuffer*)pData;
	cl_dev_cmd_desc*		pCmd = pThis->m_pCmd;
	cl_dev_cmd_param_rw*	cmdParams = (cl_dev_cmd_param_rw*)pCmd->params;
	SMemCpyParams*			pCpyParam = (SMemCpyParams*)pParams;
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

	pThis->NotifyDispatcher();
}

void ReadWriteBuffer::CopyMemoryBuffer(SMemCpyParams* pCopyCmd)
{
	unsigned char *pSrcPlane = (unsigned char *)pCopyCmd->pSrc;
	unsigned char *pDstPlane = (unsigned char *)pCopyCmd->pDst;

	for(unsigned int iDepth=0; iDepth<pCopyCmd->pRegion[2]; ++iDepth)
	{
		unsigned char *pSrcLine = pSrcPlane;
		unsigned char *pDstLine = pDstPlane;

		for(unsigned int iHeight=0; iHeight<pCopyCmd->pRegion[1]; ++iHeight)
		{
			// TODO: use intrinsics
			memcpy(pDstLine, pSrcLine, pCopyCmd->pRegion[0]);
			pSrcLine += pCopyCmd->stSrcRowPitch;
			pDstLine += pCopyCmd->stDstRowPitch;
		}

		pSrcPlane += pCopyCmd->stSrcSlicePitch;
		pDstPlane += pCopyCmd->stSrcSlicePitch;
	}
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
		cl_dev_mem memObj = (cl_dev_mem)cmdParams->mem_loc[i];

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
	void*	pArgV = malloc(cmd->param_size);
	if ( NULL == pArgV )
	{
		ErrLog(m_logDescriptor, m_iLogHandle, L"Can't allocate memory for parameters");
		return CL_DEV_OUT_OF_MEMORY;
	}

	// Prepare native Task for execution

	// Lock Memory objects handles
	for(unsigned int i=0; i<cmdParams->mem_num; ++i )
	{
		cl_dev_mem memObj = (cl_dev_mem)cmdParams->mem_loc[i];
		size_t	Offset = (size_t)cmdParams->mem_loc[i] - (size_t)cmd->params;
		void*	*pMemPtr = (void**)((cl_char*)pArgV+Offset);

		cl_int ret = m_pMemAlloc->LockObject(memObj, NULL, pMemPtr, NULL, NULL);
		if ( CL_DEV_FAILED(ret) )
		{
			return ret;
		}
	}

	m_pCmd = cmd;

	ETERetCode ret;
	ret = m_pTaskExec->ExecuteFunction(func, pArgV, cmdParams->args, &NotifyCommandCompletion, this, pDepList, uiCount, pNewHandle);
	if ( TE_SUCCESS != ret )
	{
		ErrLog(m_logDescriptor, m_iLogHandle, L"Failed to execute function %X", ret);
		return CL_DEV_ERROR_FAIL;
	}

	return CL_DEV_SUCCESS;
}

void NativeFunction::NotifyCommandCompletion(TTaskHandle hTask, void* pParams, size_t size, void* pData)
{
	NativeFunction*		pThis = (NativeFunction*)pData;
	cl_dev_cmd_desc*	pCmd = pThis->m_pCmd;
	cl_dev_cmd_param_native *cmdParams = (cl_dev_cmd_param_native*)pCmd->params;

	// Unlock memory buffers
	for(unsigned int i=0; i<cmdParams->mem_num; ++i )
	{
		cl_dev_mem memObj = (cl_dev_mem)cmdParams->mem_loc[i];

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

	pThis->NotifyDispatcher();
}