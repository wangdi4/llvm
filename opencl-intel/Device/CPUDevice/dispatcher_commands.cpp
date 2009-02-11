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

void DispatcherCommand::NotifyDispatcher()
{
	m_pTaskDispatcher->NotifyCommandCompletion(m_pCmd);
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

	// Lock memory object
	cl_int ret = m_pMemAlloc->LockObject(cmdParams->memObj, cmdParams->dim_count, cmdParams->origin, &pObjPtr, pPitch);
	if ( CL_DEV_FAILED(ret) )
	{
		return ret;
	}

	// Set Source/Destination
	pCpyParam->pRegion = cmdParams->region;
	pCpyParam->uiDimCount = cmdParams->dim_count;
	if ( CL_DEV_CMD_READ == cmd->type )
	{
		pCpyParam->pSrc = (cl_char*)pObjPtr;
		pCpyParam->pSrcPitch = pPitch;
		pCpyParam->pDst = (cl_char*)cmdParams->ptr;
		pCpyParam->pDstPitch = cmdParams->pitch;
	} else
	{
		pCpyParam->pSrc = (cl_char*)cmdParams->ptr;
		pCpyParam->pSrcPitch = cmdParams->pitch;
		pCpyParam->pDst = (cl_char*)pObjPtr;
		pCpyParam->pDstPitch = pPitch;
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

void ReadWriteMemObject::NotifyCommandCompletion(TTaskHandle hTask, void* pParams, size_t size, void* pData)
{
	ReadWriteMemObject*		pThis = (ReadWriteMemObject*)pData;
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

void ReadWriteMemObject::CopyMemoryBuffer(SMemCpyParams* pCopyCmd)
{
	// Copy 1D array only
	if ( 1 == pCopyCmd->uiDimCount )
	{
		memcpy(pCopyCmd->pDst, pCopyCmd->pSrc, pCopyCmd->pRegion[0]);
		return;
	}

	SMemCpyParams sRecParam;

	// Copy current parameters
	memcpy(&sRecParam, pCopyCmd, sizeof(SMemCpyParams));
	sRecParam.uiDimCount = pCopyCmd->uiDimCount-1;
	// Make recoursion
	for(unsigned int i=0; i<pCopyCmd->pRegion[sRecParam.uiDimCount]; ++i)
	{
		CopyMemoryBuffer(&sRecParam);

		sRecParam.pSrc += pCopyCmd->pSrcPitch[sRecParam.uiDimCount];
		sRecParam.pDst += pCopyCmd->pDstPitch[sRecParam.uiDimCount];
	}
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
	const cl_kernel_argument*	pArgTypes = pKernel->GetKernelArgs();

	if ( cmdParams->arg_count != uiNumArgs )
	{
		return CL_DEV_INVALID_COMMAND_PARAM;
	}

	cl_char*	pCurrParamPtr = (cl_char*)cmdParams->arg_values;
	size_t		stOffset = 0;
	// Check kernel parameters and memory buffers
	for(unsigned int i=0; i<uiNumArgs; ++i)
	{
		// Check same parameter type
		if ( (cmdParams->args[i].type != pArgTypes[i].type) || (cmdParams->args[i].size_in_bytes != pArgTypes[i].size_in_bytes) )
		{
			return CL_DEV_INVALID_COMMAND_PARAM;
		}
		// Argument is buffer object or local memory size
		if ( (CL_KRNL_ARG_PTR_LOCAL == pArgTypes[i].type) ||
			(CL_KRNL_ARG_PTR_GLOBAL == pArgTypes[i].type) ||
			(CL_KRNL_ARG_PTR_CONST == pArgTypes[i].type)
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
			stOffset += cmdParams->args[i].size_in_bytes;
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
	cl_uint						uiNumArgs = cmdParams->arg_count;
	const cl_kernel_argument*	pArgs = cmdParams->args;
	cl_char*					pCurrParamPtr = (cl_char*)cmdParams->arg_values;
	size_t						stOffset = 0;
	cl_char*					pNewParamPtr = (cl_char*)malloc(cmdParams->arg_size);

	if ( NULL == pNewParamPtr )
	{
		return CL_DEV_OUT_OF_MEMORY;
	}

	void*						*pLocalPtr = (void**)malloc(sizeof(void*)*cmdParams->arg_count);
	cl_uint						uiLocalCount = 0;
	if ( NULL == pLocalPtr )
	{
		free(pNewParamPtr);
		return CL_DEV_OUT_OF_MEMORY;
	}

	memcpy(pNewParamPtr, pCurrParamPtr, cmdParams->arg_size);

	// Lock memory buffers and local memory
	for(unsigned int i=0; i<uiNumArgs; ++i)
	{
		// Argument is buffer object or local memory size
		if ( ( CL_KRNL_ARG_PTR_LOCAL == pArgs[i].type ) ||
			 ( CL_KRNL_ARG_PTR_GLOBAL == pArgs[i].type ) ||
			 ( CL_KRNL_ARG_PTR_CONST == pArgs[i].type )
			)

		{
			cl_dev_mem memObj = (cl_dev_mem)*((void**)(pCurrParamPtr+stOffset));
			if ( -1 != memObj->allocId )	// Required local memory size
			{
				// Lock memory object / Get pointer
				clRet = m_pMemAlloc->LockObject(memObj, -1, NULL, (void**)(pNewParamPtr+stOffset), NULL);
				if ( CL_DEV_FAILED(clRet) )
				{
					free(pLocalPtr);
					free(pNewParamPtr);
					return clRet;
				}
			} else
			{
				size_t locSize = (size_t)memObj->objHandle; 
				// strict local buffer size to cache line
				locSize += (CPU_DCU_LINE_SIZE-1);
				locSize &= ~(CPU_DCU_LINE_SIZE);
				pLocalPtr[uiLocalCount] = (void*)locSize;
				++uiLocalCount;
			}
			stOffset += sizeof(void*);
		}
		else
		{
			stOffset += cmdParams->args[i].size_in_bytes;
		}
	}

	m_pCmd = cmd;
	// Allocate new Task instance
	STaskDescriptor*	pTask = new STaskDescriptor;
	if ( NULL == pTask )
	{
		free(pNewParamPtr);
		return CL_DEV_OUT_OF_MEMORY;
	}

	// Set kernel information
	pTask->sKernelParam.pfnKernelFunc = pKernel->GetHandle();
	pTask->sKernelParam.pParams = pNewParamPtr;
	pTask->sKernelParam.szParamSize = cmdParams->arg_size;
	pTask->sKernelParam.uiNumLocal = uiLocalCount;
	pTask->sKernelParam.pLocalPtr = pLocalPtr;

	// Set WG information
	pTask->sWorkingDim.iWorkDim = cmdParams->work_dim;
	memcpy(pTask->sWorkingDim.viOffset, cmdParams->glb_wrk_size, sizeof(int)*cmdParams->work_dim);
	memcpy(pTask->sWorkingDim.viGlobalSize, cmdParams->glb_wrk_size, sizeof(int)*cmdParams->work_dim);
	memcpy(pTask->sWorkingDim.viLocalSize, cmdParams->lcl_wrk_size, sizeof(int)*cmdParams->work_dim);

	ETERetCode ret;
	ret = m_pTaskExec->ExecuteTask(pTask, NotifyCommandCompletion, this, pDepList, uiCount, pNewHandle);
	if ( TE_SUCCESS != ret )
	{
		ErrLog(m_logDescriptor, m_iLogHandle, L"Failed to execute function %X", ret);
		return CL_DEV_ERROR_FAIL;
	}

	return CL_DEV_SUCCESS;
}

void KernelCommand::NotifyCommandCompletion(TTaskHandle hTask, STaskDescriptor* psTaskDescriptor, void* pData)
{
	KernelCommand*		pThis = (KernelCommand*)pData;
	cl_dev_cmd_desc*	pCmd = pThis->m_pCmd;
	cl_dev_cmd_param_kernel *cmdParams = (cl_dev_cmd_param_kernel*)pCmd->params;

	// Check kernel paramters
	cl_uint						uiNumArgs = cmdParams->arg_count;
	const cl_kernel_argument*	pArgs = cmdParams->args;
	cl_char*					pCurrParamPtr = (cl_char*)cmdParams->arg_values;
	size_t						stOffset = 0;
	cl_char*					pParamPtr = (cl_char*)psTaskDescriptor->sKernelParam.pParams;

	// Unlock memory buffers
	for(unsigned int i=0; i<uiNumArgs; ++i)
	{
		// Argument is buffer object or local memory size
		if ( ( CL_KRNL_ARG_PTR_LOCAL == pArgs[i].type ) ||
			 ( CL_KRNL_ARG_PTR_GLOBAL == pArgs[i].type ) ||
			 ( CL_KRNL_ARG_PTR_CONST == pArgs[i].type )
			)
		{
			cl_dev_mem memObj = (cl_dev_mem)*((void**)(pCurrParamPtr+stOffset));

			if ( -1 != memObj->allocId )	// Required local memory size
			{
				// UnLock memory object / Get pointer
				cl_int clRet = pThis->m_pMemAlloc->UnLockObject(memObj,(void*)(pParamPtr+stOffset));
				if ( CL_DEV_FAILED(clRet) )
				{
					ErrLog(pThis->m_logDescriptor, pThis->m_iLogHandle, L"Can't unlock memory object");
				}
			}
			stOffset += sizeof(void*);
		}
		else
		{
			stOffset += cmdParams->args[i].size_in_bytes;
		}
	}

	// Free memory allocated to execution parameters
	free(psTaskDescriptor->sKernelParam.pParams);
	free(psTaskDescriptor->sKernelParam.pLocalPtr);

	delete psTaskDescriptor;

	pThis->NotifyDispatcher();

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
	void*	pArgV = malloc(cmdParams->args);
	if ( NULL == pArgV )
	{
		ErrLog(m_logDescriptor, m_iLogHandle, L"Can't allocate memory for parameters");
		return CL_DEV_OUT_OF_MEMORY;
	}

	// Prepare native Task for execution
	memcpy(pArgV, cmdParams->argv, cmdParams->args);

	// Lock Memory objects handles
	for(unsigned int i=0; i<cmdParams->mem_num; ++i )
	{
		cl_dev_mem memObj = (cl_dev_mem)cmdParams->mem_loc[i];
		size_t	Offset = (size_t)cmdParams->mem_loc[i] - (size_t)cmdParams->argv;
		void*	*pMemPtr = (void**)((cl_char*)pArgV+Offset);

		cl_int ret = m_pMemAlloc->LockObject(memObj, -1, NULL, pMemPtr, NULL);
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