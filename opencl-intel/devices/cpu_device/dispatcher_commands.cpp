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
#include "cpu_logger.h"
#include "cpu_dev_limits.h"
#include "wg_context.h"
#include "cl_dev_backend_api.h"
#include "cl_sys_defines.h"
#include "ocl_itt.h"


#define getR(color) ((color >> 16) & 0xFF)
#define getG(color) ((color >> 8) & 0xFF)
#define getB(color) (color & 0xFF)

// This string size is calculated as 6x(the maximal number of digits assuming size_t is 64 bit) + 8
#define GPA_RANGE_STRING_SIZE 130

#define COLOR(R,G,B)	(((R)<<16) + ((G)<<8) + (B))
using namespace Intel::OpenCL::CPUDevice;
using namespace Intel::OpenCL;
unsigned int NDRange::RGBTable[COLOR_TABLE_SIZE] = {
	COLOR(204,0,102),	COLOR(153,51,153),	COLOR(102,51,204),	COLOR(51,0,253),
	COLOR(102,153,255),	COLOR(0,204,204),	COLOR(153,255,204),	COLOR(0,153,102),
	COLOR(0,51,0),		COLOR(204,255,51),	COLOR(255,255,0),	COLOR(204,51,0),
	COLOR(255,153,255),	COLOR(204,102,255),	COLOR(51,0,51),		COLOR(51,0,153),
	COLOR(0,102,204),	COLOR(51,255,255),	COLOR(0,204,153),	COLOR(0,204,102),
	COLOR(102,204,51),	COLOR(153,204,0),	COLOR(255,255,102),	COLOR(255,102,0),
	COLOR(255,153,204),	COLOR(204,51,204),	COLOR(153,51,255),	COLOR(51,51,102),
	COLOR(0,0,204),		COLOR(153,255,255),	COLOR(51,255,204),	COLOR(51,255,153),
	COLOR(102,255,51),	COLOR(51,102,0),	COLOR(255,255,204),	COLOR(255,102,51),
	COLOR(102,0,51),	COLOR(153,0,102),	COLOR(102,0,102),	COLOR(102,102,255),
	COLOR(0,0,153),		COLOR(51,204,255),	COLOR(0,51,51),		COLOR(153,204,153),
	COLOR(0,204,51),	COLOR(102,153,51),	COLOR(153,153,102),	COLOR(204,102,51),
	COLOR(153,0,51),	COLOR(255,0,102),	COLOR(204,0,204),	COLOR(102,0,204),
	COLOR(51,51,255),	COLOR(0,51,102),	COLOR(0,153,153),	COLOR(0,255,153),
	COLOR(0,153,51),	COLOR(0,153,0),		COLOR(204,255,0),	COLOR(255,204,0),		
	COLOR(204,204,204),	COLOR(153,153,153),	COLOR(102,102,102),	COLOR(51,51,51)};

AtomicCounter NDRange::RGBTableCounter;

//#define _DEBUG_PRINT
///////////////////////////////////////////////////////////////////////////
// Base dispatcher command
DispatcherCommand::DispatcherCommand(TaskDispatcher* pTD) :
m_pTaskDispatcher(pTD), m_pLogDescriptor(pTD->m_pLogDescriptor)
{
	m_iLogHandle = pTD->m_iLogHandle;

	m_pMemAlloc = pTD->m_pMemoryAllocator;

	m_pGPAData = pTD->m_pGPAData;
}

inline WGContext* DispatcherCommand::GetWGContext(unsigned int id)
{
	return m_pTaskDispatcher->GetWGContext(id);
}

void DispatcherCommand::NotifyCommandStatusChanged(cl_dev_cmd_desc* cmd, unsigned uStatus, int iErr)
{
	m_pTaskDispatcher->NotifyCommandStatusChange(cmd, uStatus, iErr);
}

///////////////////////////////////////////////////////////////////////////
// Affinitizable command
AffinitizableCommand::AffinitizableCommand(TaskDispatcher *pTD) : DispatcherCommand(pTD)
{
    assert(pTD);
    m_affinityMask = pTD->getAffinityMask();
}

void AffinitizableCommand::AffinitizeToTask()
{
    if (NULL != m_affinityMask)
    {
        clSetThreadAffinityMask(m_affinityMask);
    }
}


///////////////////////////////////////////////////////////////////////////
// OCL Read/Write buffer execution

cl_dev_err_code ReadWriteMemObject::Create(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, ITaskBase* *pTask)
{
	ReadWriteMemObject* pCommand = new ReadWriteMemObject(pTD);
	if (NULL == pCommand)
	{
		return CL_DEV_OUT_OF_MEMORY;
	}
#ifdef _DEBUG
	cl_dev_err_code rc;
	rc = pCommand->CheckCommandParams(pCmd);
	if( CL_DEV_FAILED(rc))
	{
		delete pCommand;
		return rc;
	}
#endif

	pCommand->m_pCmd = pCmd;
	assert(pTask);
	*pTask = static_cast<ITaskBase*>(pCommand);

	return CL_DEV_SUCCESS;
}

ReadWriteMemObject::ReadWriteMemObject(TaskDispatcher* pTD) :
	AffinitizableCommand(pTD)
{
}

cl_dev_err_code ReadWriteMemObject::CheckCommandParams(cl_dev_cmd_desc* cmd)
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

	cl_dev_err_code ret = m_pMemAlloc->ValidateObject(cmdParams->memObj);
	if ( CL_DEV_FAILED(ret) )
	{
		return ret;
	}

	for (unsigned int i=0; i< cmdParams->dim_count-1; i++)
	{
		if(0 == cmdParams->pitch[i])
		{
			return CL_DEV_INVALID_VALUE;
		}
	}

	// Check Region
	return CL_DEV_SUCCESS;
}

void ReadWriteMemObject::Execute()
{
	cl_dev_cmd_param_rw*	cmdParams = (cl_dev_cmd_param_rw*)m_pCmd->params;
	SMemCpyParams			sCpyParam;
	void*	pObjPtr;
	size_t  uiElementSize;


#ifdef _DEBUG_PRINT
	printf("--> ReadWriteMemObject(start), cmdid:%d\n", m_pCmd->id);
#endif

	// Lock memory object
	// cmdParams->memobj_pitch for Buffers already has valid values, either zero or non-zero for BufferRect calls.
	// cmdParams->memobj_pitch for Non-Buffers is being updated inside with correct values.
	cl_dev_err_code ret = m_pMemAlloc->LockObject(cmdParams->memObj, cmdParams->dim_count, cmdParams->origin, &pObjPtr, cmdParams->memobj_pitch, &uiElementSize);

	if ( CL_DEV_FAILED(ret) )
	{
#ifdef _DEBUG
		CpuErrLog(m_pLogDescriptor, m_iLogHandle,
			TEXT("Failed lock memory object, rc=%x"), ret);
#endif
#ifdef _DEBUG_PRINT
		printf("--> ReadWriteMemObject(fail), cmdid:%d(%d)\n", m_pCmd->id, ret);
#endif
		NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, ret);
        return;
	}

	NotifyCommandStatusChanged(m_pCmd, CL_RUNNING, CL_DEV_SUCCESS);





	sCpyParam.uiDimCount = cmdParams->dim_count;
	// in case of buffers, we need to consider the case of BufferRect execution.
	if (cmdParams->dim_count == 1)
	{
		for (int i= MAX_WORK_DIM; i > 1 ; i--)
		{
			// find first dimension where region has non default value;
			// we need to process up to this dimension.
			if (cmdParams->region[i-1] != 1)
			{
				sCpyParam.uiDimCount = i;
				break;
			}
		}
	}

	// Set Source/Destination
	memcpy(sCpyParam.vRegion, cmdParams->region, sizeof(sCpyParam.vRegion));
	sCpyParam.vRegion[0] = cmdParams->region[0] * uiElementSize;

	// In case the pointer parameter (Destination for CMD_READ and Source for CMD_WRITE) has pitch properties,
	// we need to consider that too.
	size_t ptrOffset =	cmdParams->ptr_origin[2] * cmdParams->pitch[1] + \
						cmdParams->ptr_origin[1] * cmdParams->pitch[0] + \
						cmdParams->ptr_origin[0];


	if ( CL_DEV_CMD_READ == m_pCmd->type )
	{

		sCpyParam.pSrc = (cl_char*)pObjPtr;
		memcpy(sCpyParam.vSrcPitch, cmdParams->memobj_pitch, sizeof(sCpyParam.vSrcPitch));
		sCpyParam.pDst = (cl_char*)((size_t)cmdParams->ptr + ptrOffset);
		memcpy(sCpyParam.vDstPitch, cmdParams->pitch, sizeof(sCpyParam.vDstPitch));
	}
	else
	{
		sCpyParam.pSrc = (cl_char*)((size_t)cmdParams->ptr + ptrOffset);
		memcpy(sCpyParam.vSrcPitch, cmdParams->pitch, sizeof(sCpyParam.vSrcPitch));
		sCpyParam.pDst = (cl_char*)pObjPtr;
		memcpy(sCpyParam.vDstPitch, cmdParams->memobj_pitch, sizeof(sCpyParam.vDstPitch));
	}

	// Execute copy routine
#if defined(USE_GPA)
	if ((NULL != m_pGPAData) && (m_pGPAData->bUseGPA))
	{
		__itt_task_begin(m_pGPAData->pDomain, __itt_null, __itt_null, ( CL_DEV_CMD_READ == m_pCmd->type ? m_pGPAData->pReadHandle : m_pGPAData->pWriteHandle ));

		TAL_SetNamedTaskColor((CL_DEV_CMD_READ == m_pCmd->type ? "Read" : "Write"), 255, 0, 0);

		switch(cmdParams->dim_count)
		{
#if defined(_M_X64)
		case 1:
			__itt_metadata_add(m_pGPAData->pDomain, __itt_null, m_pGPAData->pSizeHandle, __itt_metadata_u64, 1, &sCpyParam.vRegion[0]);
			break;
		case 2:
			__itt_metadata_add(m_pGPAData->pDomain, __itt_null, m_pGPAData->pWidthHandle, __itt_metadata_u64 , 1, &sCpyParam.vRegion[0]);
			__itt_metadata_add(m_pGPAData->pDomain, __itt_null, m_pGPAData->pHeightHandle, __itt_metadata_u64 , 1, &cmdParams->region[1]);
			break;
		case 3:
			__itt_metadata_add(m_pGPAData->pDomain, __itt_null, m_pGPAData->pWidthHandle, __itt_metadata_u64 , 1, &sCpyParam.vRegion[0]);
			__itt_metadata_add(m_pGPAData->pDomain, __itt_null, m_pGPAData->pHeightHandle, __itt_metadata_u64 , 1, &cmdParams->region[1]);
			__itt_metadata_add(m_pGPAData->pDomain, __itt_null, m_pGPAData->pDepthHandle, __itt_metadata_u64 , 1, &cmdParams->region[2]);
			break;
#else
		case 1:
			__itt_metadata_add(m_pGPAData->pDomain, __itt_null, m_pGPAData->pSizeHandle, __itt_metadata_u32 , 1, &sCpyParam.vRegion[0]);
			break;
		case 2:
			__itt_metadata_add(m_pGPAData->pDomain, __itt_null, m_pGPAData->pWidthHandle, __itt_metadata_u32 , 1, &sCpyParam.vRegion[0]);
			__itt_metadata_add(m_pGPAData->pDomain, __itt_null, m_pGPAData->pHeightHandle, __itt_metadata_u32 , 1, &cmdParams->region[1]);
			break;
		case 3:
			__itt_metadata_add(m_pGPAData->pDomain, __itt_null, m_pGPAData->pWidthHandle, __itt_metadata_u32 , 1, &sCpyParam.vRegion[0]);
			__itt_metadata_add(m_pGPAData->pDomain, __itt_null, m_pGPAData->pHeightHandle, __itt_metadata_u32 , 1, &cmdParams->region[1]);
			__itt_metadata_add(m_pGPAData->pDomain, __itt_null, m_pGPAData->pDepthHandle, __itt_metadata_u32 , 1, &cmdParams->region[2]);
			break;
#endif
		}
	}
#endif

	MemoryAllocator::CopyMemoryBuffer(&sCpyParam);

#if defined(USE_GPA)
	if ((NULL != m_pGPAData) && (m_pGPAData->bUseGPA))
	{
		__itt_task_end(m_pGPAData->pDomain);
	}
#endif

	ret = m_pMemAlloc->UnLockObject(cmdParams->memObj, pObjPtr);
#ifdef _DEBUG
	if ( CL_DEV_FAILED(ret) )
	{
		CpuErrLog(m_pLogDescriptor, m_iLogHandle,
			TEXT("Can't unlock memory object, rc=%x"), ret);
	}
#endif
#ifdef _DEBUG_PRINT
	printf("--> ReadWriteMemObject(end), cmdid:%d(%d)\n", m_pCmd->id, ret);
#endif
	NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, ret);
}

///////////////////////////////////////////////////////////////////////////
// OCL Copy memory object execution
cl_dev_err_code CopyMemObject::Create(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, ITaskBase* *pTask)
{
	CopyMemObject* pCommand = new CopyMemObject(pTD);
	if (NULL == pCommand)
	{
		return CL_DEV_OUT_OF_MEMORY;
	}
#ifdef _DEBUG
	cl_dev_err_code rc;
	rc = pCommand->CheckCommandParams(pCmd);
	if( CL_DEV_FAILED(rc))
	{
		delete pCommand;
		return rc;
	}
#endif

	pCommand->m_pCmd = pCmd;
	assert(pTask);
	*pTask = static_cast<ITaskBase*>(pCommand);

	return CL_DEV_SUCCESS;
}

CopyMemObject::CopyMemObject(TaskDispatcher* pTD) :
	AffinitizableCommand(pTD)
{
}

cl_dev_err_code CopyMemObject::CheckCommandParams(cl_dev_cmd_desc* cmd)
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

	cl_dev_err_code ret = m_pMemAlloc->ValidateObject(cmdParams->dstMemObj);
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

void CopyMemObject::Execute()
{
	cl_dev_cmd_param_copy*	cmdParams = (cl_dev_cmd_param_copy*)m_pCmd->params;
	SMemCpyParams			sCpyParam;

	size_t  uiSrcElementSize, uiDstElementSize;
	cl_dev_err_code ret;

	// Lock src memory object
	ret = m_pMemAlloc->LockObject(cmdParams->srcMemObj, cmdParams->src_dim_count, cmdParams->src_origin,
		(void**)&sCpyParam.pSrc, cmdParams->src_pitch, &uiSrcElementSize);
	if ( CL_DEV_FAILED(ret) )
	{
#ifdef _DEBUG_PRINT
		printf("--> CopyMemObject(fail,1), cmdid:%d(%d)\n", m_pCmd->id, ret);
#endif
		NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, ret);
		return;
	}

	// Lock dst memory object
	ret = m_pMemAlloc->LockObject(cmdParams->dstMemObj, cmdParams->dst_dim_count, cmdParams->dst_origin,
		(void**)&sCpyParam.pDst, cmdParams->dst_pitch, &uiDstElementSize);
	if ( CL_DEV_FAILED(ret) )
	{
		m_pMemAlloc->UnLockObject(cmdParams->srcMemObj, sCpyParam.pSrc);
#ifdef _DEBUG_PRINT
		printf("--> CopyMemObject(fail,2), cmdid:%d(%d)\n", m_pCmd->id, ret);
#endif
		NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, ret);
		return;
	}

	// Objects has to have same element size or buffer<->image
	if( (uiDstElementSize != uiSrcElementSize) &&
		(1 != uiDstElementSize) && (1 != uiSrcElementSize) )
	{
		m_pMemAlloc->UnLockObject(cmdParams->srcMemObj, sCpyParam.pSrc);
		m_pMemAlloc->UnLockObject(cmdParams->dstMemObj, sCpyParam.pDst);
#ifdef _DEBUG_PRINT
		printf("--> CopyMemObject(fail,3), cmdid:%d(%d)\n", m_pCmd->id, ret);
#endif
		NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, (cl_int)CL_DEV_INVALID_COMMAND_PARAM);
		return;
	}

	// No we can notify that we are running
	NotifyCommandStatusChanged(m_pCmd, CL_RUNNING, CL_DEV_SUCCESS);

	//Options for different dimensions are
    //Copy a 2D image object to a 2D slice of a 3D image object.
    //Copy a 2D slice of a 3D image object to a 2D image object.
	//Copy 2D to 2D
	//Copy 3D to 3D
	//Copy 2D image to buffer
	//Copy 3D image to buffer
	//Buffer to image

	memcpy(sCpyParam.vSrcPitch, cmdParams->src_pitch, sizeof(sCpyParam.vSrcPitch));
	memcpy(sCpyParam.vDstPitch, cmdParams->dst_pitch, sizeof(sCpyParam.vDstPitch));

	sCpyParam.uiDimCount = min(cmdParams->src_dim_count, cmdParams->dst_dim_count);
	if(cmdParams->dst_dim_count != cmdParams->src_dim_count)
	{
		//Buffer to image
		if(1 == cmdParams->src_dim_count)
		{
			uiSrcElementSize = uiDstElementSize;
			sCpyParam.uiDimCount = cmdParams->dst_dim_count;
			sCpyParam.vSrcPitch[0] = cmdParams->region[0] * uiDstElementSize;
			sCpyParam.vSrcPitch[1] = sCpyParam.vSrcPitch[0] * cmdParams->region[1];
		}
		if( 1 == cmdParams->dst_dim_count)
		{
			//When destination is buffer the memcpy will be done as if the buffer is an image with height=1
			sCpyParam.uiDimCount = cmdParams->src_dim_count;
			sCpyParam.vDstPitch[0] = cmdParams->region[0] * uiSrcElementSize;
			sCpyParam.vDstPitch[1] = sCpyParam.vDstPitch[0] * cmdParams->region[1];
		}
	}
	else if (cmdParams->src_dim_count == 1)
	{
		// in case of buffers, we need to consider the case of BufferRect execution.
		for (int i= MAX_WORK_DIM; i > 1 ; i--)
		{
			// find first dimension where region has non default value;
			// we need to process up to this dimension.
			if (cmdParams->region[i-1] != 1)
			{
				sCpyParam.uiDimCount = i;
				break;
			}
		}
	}

	//If row_pitch (or input_row_pitch) is set to 0, the appropriate row pitch is calculated
	//based on the size of each element in bytes multiplied by width.
	memcpy(sCpyParam.vRegion, cmdParams->region, sizeof(sCpyParam.vRegion));
	sCpyParam.vRegion[0] *= uiSrcElementSize;

	// Execute copy routine
#if defined(USE_GPA)
	if ((NULL != m_pGPAData) && (m_pGPAData->bUseGPA))
	{
		__itt_task_begin(m_pGPAData->pDomain, __itt_null, __itt_null, m_pGPAData->pCopyHandle);

		TAL_SetNamedTaskColor("Copy", 255, 0, 0);

		switch(cmdParams->src_dim_count)
		{
#if defined(_M_X64)
		case 1:
			
			__itt_metadata_add(m_pGPAData->pDomain, __itt_null, m_pGPAData->pSizeHandle, __itt_metadata_u64 , 1, &sCpyParam.vRegion[0]);
			break;
		case 2:
			__itt_metadata_add(m_pGPAData->pDomain, __itt_null, m_pGPAData->pWidthHandle, __itt_metadata_u64 , 1, &sCpyParam.vRegion[0]);
			__itt_metadata_add(m_pGPAData->pDomain, __itt_null, m_pGPAData->pHeightHandle, __itt_metadata_u64 , 1, &cmdParams->region[1]);
			break;
		case 3:
			__itt_metadata_add(m_pGPAData->pDomain, __itt_null, m_pGPAData->pWidthHandle, __itt_metadata_u64 , 1, &sCpyParam.vRegion[0]);
			__itt_metadata_add(m_pGPAData->pDomain, __itt_null, m_pGPAData->pHeightHandle, __itt_metadata_u64 , 1, &cmdParams->region[1]);
			__itt_metadata_add(m_pGPAData->pDomain, __itt_null, m_pGPAData->pDepthHandle, __itt_metadata_u64 , 1, &cmdParams->region[2]);
			break;
#else
		case 1:
			
			__itt_metadata_add(m_pGPAData->pDomain, __itt_null, m_pGPAData->pSizeHandle, __itt_metadata_u32 , 1, &sCpyParam.vRegion[0]);
			break;
		case 2:
			__itt_metadata_add(m_pGPAData->pDomain, __itt_null, m_pGPAData->pWidthHandle, __itt_metadata_u32 , 1, &sCpyParam.vRegion[0]);
			__itt_metadata_add(m_pGPAData->pDomain, __itt_null, m_pGPAData->pHeightHandle, __itt_metadata_u32 , 1, &cmdParams->region[1]);
			break;
		case 3:
			__itt_metadata_add(m_pGPAData->pDomain, __itt_null, m_pGPAData->pWidthHandle, __itt_metadata_u32 , 1, &sCpyParam.vRegion[0]);
			__itt_metadata_add(m_pGPAData->pDomain, __itt_null, m_pGPAData->pHeightHandle, __itt_metadata_u32 , 1, &cmdParams->region[1]);
			__itt_metadata_add(m_pGPAData->pDomain, __itt_null, m_pGPAData->pDepthHandle, __itt_metadata_u32 , 1, &cmdParams->region[2]);
			break;
#endif
		}
	}
#endif	
	
	// Execute copy routine
	MemoryAllocator::CopyMemoryBuffer(&sCpyParam);

#if defined(USE_GPA)
	if ((NULL != m_pGPAData) && (m_pGPAData->bUseGPA))
	{
		__itt_task_end(m_pGPAData->pDomain);
	} 
#endif
	ret = m_pMemAlloc->UnLockObject(cmdParams->dstMemObj, sCpyParam.pDst);
#ifdef _DEBUG
	if ( CL_DEV_FAILED(ret) )
	{
		CpuErrLog(m_pLogDescriptor, m_iLogHandle,
			TEXT("Can't unlock destination memory object, rc=%x"), ret);
	}
#endif
	ret = m_pMemAlloc->UnLockObject(cmdParams->srcMemObj, sCpyParam.pSrc);
#ifdef _DEBUG
	if ( CL_DEV_FAILED(ret) )
	{
		CpuErrLog(m_pLogDescriptor, m_iLogHandle,
			TEXT("Can't unlock source memory object, rc=%x"), ret);
	}
#endif

#ifdef _DEBUG_PRINT
	printf("--> CopyMemObject(end), cmdid:%d(%d)\n", m_pCmd->id, ret);
#endif
	NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, ret);
}

///////////////////////////////////////////////////////////////////////////
// OCL Native function execution
cl_dev_err_code NativeFunction::Create(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, ITaskBase* *pTask)
{
	NativeFunction* pCommand = new NativeFunction(pTD);
	if (NULL == pCommand)
	{
		return CL_DEV_OUT_OF_MEMORY;
	}
#ifdef _DEBUG
	cl_dev_err_code rc;
	rc = pCommand->CheckCommandParams(pCmd);
	if( CL_DEV_FAILED(rc))
	{
		delete pCommand;
		return rc;
	}
#endif

	pCommand->m_pCmd = pCmd;
	cl_dev_cmd_param_native *cmdParams = (cl_dev_cmd_param_native*)pCmd->params;

	// Create temporal buffer for execution
	void*	pArgV = new char[cmdParams->args];
	if ( NULL == pArgV )
	{
#ifdef _DEBUG
		CpuErrLog(pCommand->m_pLogDescriptor, pCommand->m_iLogHandle, TEXT("%S"), TEXT("Can't allocate memory for parameters"));
#endif
		delete pCommand;
		return CL_DEV_OUT_OF_MEMORY;
	}
	pCommand->m_pArgV = pArgV;
	MEMCPY_S(pArgV, cmdParams->args, cmdParams->argv, cmdParams->args);

	assert(pTask);
	*pTask = static_cast<ITaskBase*>(pCommand);
	return CL_DEV_SUCCESS;
}

NativeFunction::NativeFunction(TaskDispatcher* pTD) :
	AffinitizableCommand(pTD)
{
}

cl_dev_err_code	NativeFunction::CheckCommandParams(cl_dev_cmd_desc* cmd)
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

		cl_dev_err_code ret = m_pMemAlloc->ValidateObject(memObj);
		if ( CL_DEV_FAILED(ret) )
		{
			return ret;
		}
	}

	return CL_DEV_SUCCESS;
}

void NativeFunction::Execute()
{
	cl_dev_cmd_param_native *cmdParams = (cl_dev_cmd_param_native*)m_pCmd->params;

	// Lock Memory objects handles
	cl_dev_err_code ret = CL_DEV_SUCCESS;
	for(unsigned int i=0; (i<cmdParams->mem_num) && CL_DEV_SUCCEEDED(ret); ++i )
	{
		cl_dev_mem memObj = *((cl_dev_mem*)cmdParams->mem_loc[i]);
		size_t	Offset = (size_t)cmdParams->mem_loc[i] - (size_t)cmdParams->argv;
		void*	*pMemPtr = (void**)((cl_char*)m_pArgV+Offset);

		ret = m_pMemAlloc->LockObject(memObj, -1, NULL, pMemPtr, NULL, NULL);
		if ( CL_DEV_FAILED(ret) )
		{
			*pMemPtr = NULL;
		}
	}

	if ( CL_DEV_SUCCEEDED(ret) )
	{
		// Notify start execution
		NotifyCommandStatusChanged(m_pCmd, CL_RUNNING, CL_DEV_SUCCESS);

		// Execute native function
		fn_clNativeKernel *func = (fn_clNativeKernel*)cmdParams->func_ptr;
		func(m_pArgV);
	}

	// Unlock memory buffers
	for(unsigned int i=0; i<cmdParams->mem_num; ++i )
	{
		cl_dev_mem memObj = *((cl_dev_mem*)cmdParams->mem_loc[i]);

		size_t	Offset = (size_t)cmdParams->mem_loc[i] - (size_t)cmdParams->argv;
		void*	*pMemPtr = (void**)((cl_char*)m_pArgV + Offset);

		// defined private ret value
		if ( NULL != *pMemPtr )
		{
			cl_dev_err_code ret = m_pMemAlloc->UnLockObject(memObj, *pMemPtr);
			if ( CL_DEV_FAILED(ret) )
			{
				CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("Can't unlock memory object, rc=%x"), ret);
			}
		}
	}

	// Free memory allocated to execution parameters
	char* temp = (char*)m_pArgV;
	delete [] temp;

	NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, ret);

}

///////////////////////////////////////////////////////////////////////////
// OCL Map buffer execution
//////////////////////////////////////////////////////////////////////////
cl_dev_err_code MapMemObject::Create(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, ITaskBase* *pTask)
{
	MapMemObject* pCommand = new MapMemObject(pTD);
	if (NULL == pCommand)
	{
		return CL_DEV_OUT_OF_MEMORY;
	}
#ifdef _DEBUG
	cl_dev_err_code rc;
	rc = pCommand->CheckCommandParams(pCmd);
	if( CL_DEV_FAILED(rc))
	{
		delete pCommand;
		return rc;
	}
#endif

	pCommand->m_pCmd = pCmd;

	assert(pTask);
	*pTask = static_cast<ITaskBase*>(pCommand);
	return CL_DEV_SUCCESS;
}

MapMemObject::MapMemObject(TaskDispatcher* pTD) :
	AffinitizableCommand(pTD)
{
}

cl_dev_err_code MapMemObject::CheckCommandParams(cl_dev_cmd_desc* cmd)
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

	cl_dev_err_code ret = m_pMemAlloc->ValidateObject(cmdParams->memObj);
	if ( CL_DEV_FAILED(ret) )
	{
		return ret;
	}

	return CL_DEV_SUCCESS;
}

void MapMemObject::Execute()
{
	cl_dev_cmd_param_map*	cmdParams = (cl_dev_cmd_param_map*)(m_pCmd->params);
	SMemCpyParams			sCpyParam;

	size_t  uiElementSize;

	// Lock memory object
	cl_dev_err_code ret = m_pMemAlloc->LockObject(cmdParams->memObj, cmdParams->dim_count, cmdParams->origin,
		(void**)&sCpyParam.pSrc, sCpyParam.vSrcPitch, &uiElementSize);
	if ( CL_DEV_FAILED(ret) )
	{
		NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, ret);
		return;
	}
#if defined(USE_GPA)
	if ((NULL != m_pGPAData) && (m_pGPAData->bUseGPA))
	{
		__itt_task_begin(m_pGPAData->pDomain, __itt_null, __itt_null, m_pGPAData->pMapHandle);
		TAL_SetNamedTaskColor("Map", 255, 0, 0);
	}
#endif

	// Different pointer to map, need copy data
	if (sCpyParam.pSrc != cmdParams->ptr)
	{
		// Setup data for copying
		// Set Source/Destination
		sCpyParam.uiDimCount = cmdParams->dim_count;
		memcpy(sCpyParam.vRegion, cmdParams->region, sizeof(sCpyParam.vRegion));
		sCpyParam.vRegion[0] = cmdParams->region[0] * uiElementSize;

		sCpyParam.pDst = (cl_char*)cmdParams->ptr;
		memcpy(sCpyParam.vDstPitch, cmdParams->pitch, sizeof(sCpyParam.vDstPitch));

#if defined(USE_GPA)
		// In case of data copy during Map, add size/dimention parameters to the task
		if ((NULL != m_pGPAData) && (m_pGPAData->bUseGPA))
		{
			switch(cmdParams->dim_count)
			{
#if defined(_M_X64)
			case 1:		
				__itt_metadata_add(m_pGPAData->pDomain, __itt_null, m_pGPAData->pSizeHandle, __itt_metadata_u64 , 1, &sCpyParam.vRegion[0]);
				break;
			case 2:
				__itt_metadata_add(m_pGPAData->pDomain, __itt_null, m_pGPAData->pWidthHandle, __itt_metadata_u64 , 1, &sCpyParam.vRegion[0]);
				__itt_metadata_add(m_pGPAData->pDomain, __itt_null, m_pGPAData->pHeightHandle, __itt_metadata_u64 , 1, &cmdParams->region[1]);
				break;
			case 3:
				__itt_metadata_add(m_pGPAData->pDomain, __itt_null, m_pGPAData->pWidthHandle, __itt_metadata_u64 , 1, &sCpyParam.vRegion[0]);
				__itt_metadata_add(m_pGPAData->pDomain, __itt_null, m_pGPAData->pHeightHandle, __itt_metadata_u64 , 1, &cmdParams->region[1]);
				__itt_metadata_add(m_pGPAData->pDomain, __itt_null, m_pGPAData->pDepthHandle, __itt_metadata_u64 , 1, &cmdParams->region[2]);
				break;
#else
			case 1:		
				__itt_metadata_add(m_pGPAData->pDomain, __itt_null, m_pGPAData->pSizeHandle, __itt_metadata_u32 , 1, &sCpyParam.vRegion[0]);
				break;
			case 2:
				__itt_metadata_add(m_pGPAData->pDomain, __itt_null, m_pGPAData->pWidthHandle, __itt_metadata_u32 , 1, &sCpyParam.vRegion[0]);
				__itt_metadata_add(m_pGPAData->pDomain, __itt_null, m_pGPAData->pHeightHandle, __itt_metadata_u32 , 1, &cmdParams->region[1]);
				break;
			case 3:
				__itt_metadata_add(m_pGPAData->pDomain, __itt_null, m_pGPAData->pWidthHandle, __itt_metadata_u32 , 1, &sCpyParam.vRegion[0]);
				__itt_metadata_add(m_pGPAData->pDomain, __itt_null, m_pGPAData->pHeightHandle, __itt_metadata_u32 , 1, &cmdParams->region[1]);
				__itt_metadata_add(m_pGPAData->pDomain, __itt_null, m_pGPAData->pDepthHandle, __itt_metadata_u32 , 1, &cmdParams->region[2]);
				break;
#endif
			}
		}
#endif
		// Execute copy routine
		MemoryAllocator::CopyMemoryBuffer(&sCpyParam);
	}

#if defined(USE_GPA)
	if ((NULL != m_pGPAData) && (m_pGPAData->bUseGPA))
	{
		__itt_task_end(m_pGPAData->pDomain);
	} 
#endif
	ret = m_pMemAlloc->UnLockObject(cmdParams->memObj, sCpyParam.pSrc);
	if ( CL_DEV_FAILED(ret) )
	{
		CpuErrLog(m_pLogDescriptor, m_iLogHandle,
			TEXT("Can't unlock memory object, rc=%x"), ret);
	}

	NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, ret);
}

///////////////////////////////////////////////////////////////////////////
// OCL Unmap buffer execution
//////////////////////////////////////////////////////////////////////////
cl_dev_err_code UnmapMemObject::Create(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, ITaskBase* *pTask)
{
	UnmapMemObject* pCommand = new UnmapMemObject(pTD);
	if (NULL == pCommand)
	{
		return CL_DEV_OUT_OF_MEMORY;
	}
#ifdef _DEBUG
	cl_dev_err_code rc;
	rc = pCommand->CheckCommandParams(pCmd);
	if( CL_DEV_FAILED(rc))
	{
		delete pCommand;
		return rc;
	}
#endif

	pCommand->m_pCmd = pCmd;

	assert(pTask);
	*pTask = static_cast<ITaskBase*>(pCommand);
	return CL_DEV_SUCCESS;
}

UnmapMemObject::UnmapMemObject(TaskDispatcher* pTD) :
	AffinitizableCommand(pTD)
{
}

cl_dev_err_code UnmapMemObject::CheckCommandParams(cl_dev_cmd_desc* cmd)
{
	if (CL_DEV_CMD_UNMAP != cmd->type)
	{
		return CL_DEV_INVALID_COMMAND_TYPE;
	}

	if ( sizeof(cl_dev_cmd_param_map) != cmd->param_size )
	{
		return CL_DEV_INVALID_COMMAND_PARAM;
	}

	cl_dev_cmd_param_map *cmdParams = (cl_dev_cmd_param_map*)(cmd->params);

	cl_dev_err_code ret = m_pMemAlloc->ValidateObject(cmdParams->memObj);
	if ( CL_DEV_FAILED(ret) )
	{
		return ret;
	}

	return CL_DEV_SUCCESS;
}

void UnmapMemObject::Execute()
{
	cl_dev_cmd_param_map*	cmdParams = (cl_dev_cmd_param_map*)(m_pCmd->params);

	SMemCpyParams			sCpyParam;
	size_t  uiElementSize;

	// Lock memory object
	cl_dev_err_code ret = m_pMemAlloc->LockObject(cmdParams->memObj, cmdParams->dim_count, cmdParams->origin,
		(void**)&sCpyParam.pDst, sCpyParam.vDstPitch, &uiElementSize);
	if ( CL_DEV_FAILED(ret) )
	{
		CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("Can't Lock memory object, rc=%x"), ret);
		NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, ret);
		return;
	}

	// Different pointer to map, need copy data
	if (sCpyParam.pDst != cmdParams->ptr)
	{
		// Setup data for copying
		// Set Source/Destination
		sCpyParam.uiDimCount = cmdParams->dim_count;
		memcpy(sCpyParam.vRegion, cmdParams->region, sizeof(sCpyParam.vRegion));
		sCpyParam.vRegion[0] = cmdParams->region[0] * uiElementSize;

		sCpyParam.pSrc = (cl_char*)cmdParams->ptr;
		memcpy(sCpyParam.vSrcPitch, cmdParams->pitch, sizeof(sCpyParam.vSrcPitch));

#if defined(USE_GPA)
		if ((NULL != m_pGPAData) && (m_pGPAData->bUseGPA))
		{
			__itt_task_begin(m_pGPAData->pDomain, __itt_null, __itt_null, m_pGPAData->pUnmapHandle);
			TAL_SetNamedTaskColor("Unmap", 255, 0, 0);
		}
#endif
		// Execute copy command
		MemoryAllocator::CopyMemoryBuffer(&sCpyParam);
#if defined(USE_GPA)
		if ((NULL != m_pGPAData) && (m_pGPAData->bUseGPA))
		{
			__itt_task_end(m_pGPAData->pDomain);
		}
#endif
	}

	ret = m_pMemAlloc->UnLockObject(cmdParams->memObj, sCpyParam.pDst);
	if ( CL_DEV_FAILED(ret) )
	{
		CpuErrLog(m_pLogDescriptor, m_iLogHandle,
			TEXT("Can't unlock memory object, rc=%x"), ret);
	}
	NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, ret);
}

///////////////////////////////////////////////////////////////////////////
// OCL Kernel execution

cl_dev_err_code NDRange::Create(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, ITaskBase* *pTask)
{
	NDRange* pCommand = new NDRange(pTD);
	if (NULL == pCommand)
	{
		return CL_DEV_OUT_OF_MEMORY;
	}
#ifdef _DEBUG
	cl_dev_err_code rc;
	rc = pCommand->CheckCommandParams(pCmd);
	if( CL_DEV_FAILED(rc))
	{
		delete pCommand;
		return rc;
	}
#endif

	pCommand->m_pCmd = pCmd;
	assert(pTask);
	*pTask = static_cast<ITaskBase*>(pCommand);
	return CL_DEV_SUCCESS;
}

NDRange::NDRange(TaskDispatcher* pTD) :
DispatcherCommand(pTD), m_lastError(CL_DEV_SUCCESS), m_pBinary(NULL),
#if defined(USE_GPA)
//canceled
//m_talKernelNameHandle(NULL),
#endif
m_pMemBuffSizes(NULL)
{
#ifdef _DEBUG
	memset(m_pLockedParams, 0x88, sizeof(m_pLockedParams));
	m_lFinish.exchange(0);
	m_lAttaching.exchange(0);
	m_lExecuting.exchange(0);
#endif
//	QueryPerformanceFrequency(&freq);
    m_affinityMask = pTD->getAffinityMask();
}

void NDRange::Release()
{
	if ( NULL != m_pBinary )
	{
		m_pBinary->Release();
	}
	if ( NULL != m_pMemBuffSizes )
	{
		delete []m_pMemBuffSizes;
	}
	delete this;
}

cl_dev_err_code NDRange::CheckCommandParams(cl_dev_cmd_desc* cmd)
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


	const ICLDevBackendKernel*	pKernel = (const ICLDevBackendKernel*)(cmdParams->kernel);
	assert(pKernel);

	size_t	stLocMemSize = 0;

	// Check kernel parameters
	cl_uint						uiNumArgs;
	const cl_kernel_argument*	pArgs;
	pKernel->GetKernelParams(&pArgs, &uiNumArgs);

	cl_char*	pCurrParamPtr = (cl_char*)cmdParams->arg_values;
	size_t		stOffset = 0;
	// Check kernel parameters and memory buffers
	for(unsigned int i=0; i<uiNumArgs; ++i)
	{
		// Argument is buffer object or local memory size
		if ( CL_KRNL_ARG_PTR_GLOBAL <= pArgs[i].type )
		{
			cl_dev_mem memObj = (cl_dev_mem)*((void**)(pCurrParamPtr+stOffset));
			// Is valid memory object
			cl_dev_err_code clRet = m_pMemAlloc->ValidateObject(memObj);
			if ( CL_DEV_FAILED(clRet) )
			{
				return clRet;
			}
			stOffset += sizeof(void*);
		}
		else if (CL_KRNL_ARG_PTR_LOCAL == pArgs[i].type)
		{
			size_t origSize = ((size_t)*(((void**)(pCurrParamPtr+stOffset))));
			size_t locSize = ADJUST_SIZE_TO_MAXIMUM_ALIGN(origSize);
			stLocMemSize += locSize;
			stOffset += sizeof(void*);
		}
		else if (CL_KRNL_ARG_VECTOR == pArgs[i].type)
		{
			unsigned int uiSize = pArgs[i].size_in_bytes;
			uiSize = (uiSize & 0xFFFF) * (uiSize >> 16);
			stOffset += uiSize;
		}
		else if (CL_KRNL_ARG_SAMPLER == pArgs[i].type)
		{
			stOffset += sizeof(cl_int);
		}
		else
		{
			stOffset += pArgs[i].size_in_bytes;
		}
	}
	// Check parameters array size
	if ( stOffset != cmdParams->arg_size )
	{
		return CL_DEV_INVALID_COMMAND_PARAM;
	}

	// Check implicit memory sizes
	stLocMemSize += pKernel->GetImplicitLocalMemoryBufferSize();

	// Check if local memory size is enough for kernel
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

		const size_t	*pReqdWGSize = pKernel->GetRequiredWorkgroupSize();
		for(unsigned int i=0; i<cmdParams->work_dim; ++i)
		{
			if ( ((0 != cmdParams->lcl_wrk_size[i]) && (CPU_DEV_MAX_WI_SIZE < cmdParams->lcl_wrk_size[i])) ||
				( pReqdWGSize && (pReqdWGSize[i] != cmdParams->lcl_wrk_size[i]))
				)
			{
				return CL_DEV_INVALID_WRK_ITEM_SIZE;
			}

			stWGSize *= cmdParams->lcl_wrk_size[i];
		}

		if ( CPU_MAX_WORK_GROUP_SIZE < stWGSize )
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

int NDRange::Init(size_t region[], unsigned int &dimCount)
{

	cl_dev_cmd_param_kernel *cmdParams = (cl_dev_cmd_param_kernel*)m_pCmd->params;
	const ICLDevBackendKernel* pKernel = (ICLDevBackendKernel*)cmdParams->kernel;
	const char*	pKernelParams = (const char*)cmdParams->arg_values;

#ifdef _DEBUG_PRINT
	printf("--> Init(start):%s, id(%d)\n", pKernel->GetKernelName(), m_pCmd->id);
#endif

	NotifyCommandStatusChanged(m_pCmd, CL_RUNNING, CL_DEV_SUCCESS);

	unsigned					uiNumArgs;
	const cl_kernel_argument*	pArgs;
	pKernel->GetKernelParams(&pArgs, &uiNumArgs);
	size_t						stOffset = 0;

	// Copy initial values
	memcpy(m_pLockedParams, cmdParams->arg_values, cmdParams->arg_size);

	// Lock required memory objects
	for(unsigned int i=0; (i<uiNumArgs) && CL_DEV_SUCCEEDED(m_lastError); ++i)
	{
		// Argument is buffer object or local memory size
		if ( ( CL_KRNL_ARG_PTR_GLOBAL == pArgs[i].type ) ||
			( CL_KRNL_ARG_PTR_CONST == pArgs[i].type )
			)

		{
			cl_dev_mem memObj = (cl_dev_mem)*((void**)(pKernelParams+stOffset));
			// Lock memory object / Get pointer
			// Fill in the local parameters buffer the virtual pointer of the buffer
			m_lastError = m_pMemAlloc->LockObject(memObj, (cl_mem_obj_descriptor**)(m_pLockedParams+stOffset));
			if ( CL_DEV_FAILED(m_lastError) )
			{
				break;
			}
			stOffset += sizeof(void*);
		}
		else if ( ( CL_KRNL_ARG_PTR_IMG_2D == pArgs[i].type ) ||
			( CL_KRNL_ARG_PTR_IMG_3D == pArgs[i].type ) ||
            ( CL_KRNL_ARG_PTR_IMG_2D_ARR == pArgs[i].type )
			)
		{
			cl_dev_mem memObj = (cl_dev_mem)*((void**)(pKernelParams+stOffset));
			m_lastError = m_pMemAlloc->LockObject(memObj, (cl_mem_obj_descriptor**)(m_pLockedParams+stOffset));
			if ( CL_DEV_FAILED(m_lastError) )
			{
				break;
			}
			// Set a pointer to image here
			stOffset += sizeof(void*);
		}
		else if (CL_KRNL_ARG_PTR_LOCAL == pArgs[i].type)
		{
			stOffset += sizeof(void*);
		}
		else if (CL_KRNL_ARG_VECTOR == pArgs[i].type)
		{
			unsigned int uiSize = pArgs[i].size_in_bytes;
			uiSize = (uiSize & 0xFFFF) * (uiSize >> 16);
			stOffset += uiSize;
		}
		else if (CL_KRNL_ARG_SAMPLER == pArgs[i].type)
		{
			stOffset += sizeof(cl_int);
		}
		else
		{
			stOffset += pArgs[i].size_in_bytes;
		}
	}
	if ( CL_DEV_FAILED(m_lastError) )
	{
		return m_lastError;
	}

	// Create an "Binary" for these parameters
	cl_dev_err_code clRet = pKernel->CreateBinary(m_pLockedParams, cmdParams->arg_size,
								cmdParams->work_dim, cmdParams->glb_wrk_offs,
								cmdParams->glb_wrk_size, cmdParams->lcl_wrk_size,
								&m_pBinary);
	if ( CL_DEV_FAILED(clRet) )
	{
		m_lastError = clRet;
		return clRet;
	}

#if defined(USE_GPA)
	// This code was removed for the initial porting of TAL
	// to GPA 4.0 and might be used in later stages
//	if (m_bUseTaskalyzer)
//	{
//		m_talKernelNameHandle = TAL_GetStringHandle(m_pBinary->GetKernel()->GetKernelName());
//		m_talRGBColor = RGBTable[(RGBTableCounter++) % COLOR_TABLE_SIZE];
//	}
#endif
	// Update buffer parameters
	m_pBinary->GetMemoryBuffersDescriptions(NULL, NULL, &m_MemBuffCount);
	m_pMemBuffSizes = new size_t[m_MemBuffCount];
	if ( NULL == m_pMemBuffSizes )
	{
		m_lastError = (cl_int)CL_DEV_OUT_OF_MEMORY;
		return -1;
	}
	m_pBinary->GetMemoryBuffersDescriptions(m_pMemBuffSizes, NULL, &m_MemBuffCount);

	// copy execution parameters
	const size_t*	pWGSize = m_pBinary->GetWorkGroupSize();
	unsigned int i;
	for (i=0; i<cmdParams->work_dim; ++i)
	{
		region[i] = (unsigned int)((cmdParams->glb_wrk_size[i])/(pWGSize[i]));
	}
	for (; i<MAX_WORK_DIM; ++i)
	{
		region[i] = 1;
	}
	dimCount = cmdParams->work_dim;

#ifdef _DEBUG_PRINT
	printf("--> Init(done):%s\n", pKernel->GetKernelName());
#endif
	return CL_DEV_SUCCESS;
}

void NDRange::Finish(FINISH_REASON reason)
{
#ifdef _DEBUG
	long lVal = (m_lExecuting.test_and_set(0, 0) | m_lAttaching.test_and_set(0, 0));
	assert(lVal == 0);
	m_lFinish.exchange(1);
#endif

	UnlockMemoryBuffers();

#ifdef _DEBUG_PRINT
	cl_dev_cmd_param_kernel *cmdParams = (cl_dev_cmd_param_kernel*)m_pCmd->params;
	const ICLDevBackendKernel* pKernel = (ICLDevBackendKernel*)cmdParams->kernel;
#endif
#ifdef _DEBUG
	lVal = (m_lExecuting.test_and_set(0, 0) | m_lAttaching.test_and_set(0, 0));
	assert(lVal == 0);
#endif

	NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, m_lastError);
#ifdef _DEBUG_PRINT
	printf("--> Finish(done):%s\n", pKernel->GetKernelName());
#endif
#ifdef _DEBUG
	m_lFinish.exchange(0);
#endif
}

void NDRange::UnlockMemoryBuffers()
{
	cl_dev_cmd_param_kernel *cmdParams = (cl_dev_cmd_param_kernel*)m_pCmd->params;
	const ICLDevBackendKernel* pKernel = (ICLDevBackendKernel*)cmdParams->kernel;
	const char*					pKernelParams = (const char*)cmdParams->arg_values;
	size_t stOffset = 0;
	unsigned					uiNumArgs;
	const cl_kernel_argument*	pArgs;

	pKernel->GetKernelParams(&pArgs, &uiNumArgs);

	// Unlock memory buffers
	for(unsigned int i=0; i<uiNumArgs; ++i)
	{
		// Argument is buffer object or local memory size
		if ( CL_KRNL_ARG_PTR_GLOBAL <= pArgs[i].type )
		{
			cl_dev_mem memObj = (cl_dev_mem)*((void**)(pKernelParams+stOffset));

			if ( ((cl_uint)-1 != memObj->allocId) && (NULL != (m_pLockedParams+stOffset)) )
			{
				// UnLock memory object / Get pointer
				cl_dev_err_code clRet = m_pMemAlloc->UnLockObject(memObj,(void*)(m_pLockedParams+stOffset));
				if ( CL_DEV_FAILED(clRet) )
				{
					CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("Can't unlock memory object"));
				}
			}
			*((void**)(m_pLockedParams+stOffset)) = NULL;
			stOffset += sizeof(void*);
		}
		else if ( CL_KRNL_ARG_PTR_LOCAL == pArgs[i].type )
		{
			stOffset += sizeof(void*);
		}
		else if (CL_KRNL_ARG_VECTOR == pArgs[i].type)
		{
			unsigned int uiSize = pArgs[i].size_in_bytes;
			uiSize = (uiSize & 0xFFFF) * (uiSize >> 16);
			stOffset += uiSize;
		}
		else if (CL_KRNL_ARG_SAMPLER == pArgs[i].type)
		{
			stOffset += sizeof(cl_int);
		}
		else
		{
			stOffset += pArgs[i].size_in_bytes;
		}
	}
}


int NDRange::AttachToThread(unsigned int uiWorkerId, size_t uiNumberOfWorkGroups, size_t firstWGID[], size_t lastWGID[])
{
#ifdef _DEBUG_PRINT
	printf("AttachToThread %d, WrkId(%d), CmdId(%d)\n", GetCurrentThreadId(), uiWorkerId, m_pCmd->id);
#endif
	assert(uiWorkerId != (unsigned int)-1);

#ifdef _DEBUG
	long lVal = m_lFinish.test_and_set(0, 0);
	if ( lVal == 1 )
	{
		assert(0);
	}
	++m_lAttaching ;
#endif

    if (NULL != m_affinityMask)
    {
        clSetThreadAffinityMask(m_affinityMask);
    }

	WGContext* pCtx = GetWGContext(uiWorkerId);
	if ( NULL == pCtx )
	{
		CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("Failed to retrive WG context, Id:%d"), uiWorkerId);
		m_lastError = (cl_int)CL_DEV_ERROR_FAIL;
#ifdef _DEBUG
	--m_lAttaching ;
#endif	
        if (NULL != m_affinityMask)
        {
            clResetThreadAffinityMask();
        }
		return (cl_int)CL_DEV_ERROR_FAIL;
	}
	else if (m_pCmd->id == pCtx->GetCmdId() )
	{
		pCtx->GetExecutable()->PrepareThread();

		// Start execution task
#if defined(USE_GPA)
		if ((NULL != m_pGPAData) && (m_pGPAData->bUseGPA))
		{
			char pWGRangeString[GPA_RANGE_STRING_SIZE];

			unsigned int uiWorkGroupSize = 1;
			const size_t*	pWGSize = m_pBinary->GetWorkGroupSize();
			cl_dev_cmd_param_kernel *cmdParams = (cl_dev_cmd_param_kernel*)m_pCmd->params;

			switch(cmdParams->work_dim)
			{
			case 1:
				sprintf_s(pWGRangeString, "%d - %d", firstWGID[0], lastWGID[0]);
				break;
			case 2:
				sprintf_s(pWGRangeString, "%d.%d - %d.%d", firstWGID[0], firstWGID[1], lastWGID[0], lastWGID[1]);
				break;
			case 3:
				sprintf_s(pWGRangeString, "%d.%d.%d - %d.%d.%d", firstWGID[0], firstWGID[1], firstWGID[2], lastWGID[0], lastWGID[1], lastWGID[2]);
				break;
			}
			
			__itt_string_handle* pKernelNameHandle = __itt_string_handle_createA(m_pBinary->GetKernel()->GetKernelName());
			__itt_task_begin(m_pGPAData->pDomain, __itt_null, __itt_null, pKernelNameHandle);
			// This coloring will be enabled in the future
			//TAL_SetNamedTaskColor(m_pBinary->GetKernel()->GetKernelName(), getR(m_talRGBColor), getG(m_talRGBColor), getB(m_talRGBColor));
			for (unsigned int i=0 ; i<cmdParams->work_dim ; ++i)
			{
				uiWorkGroupSize *= pWGSize[i];
			}
			
			__itt_metadata_add(m_pGPAData->pDomain, __itt_null, m_pGPAData->pWorkGroupSizeHandle, __itt_metadata_u32 , 1, &uiWorkGroupSize);

#if defined(_M_X64)
			__itt_metadata_add(m_pGPAData->pDomain, __itt_null, m_pGPAData->pNumberOfWorkGroupsHandle, __itt_metadata_u64 , 1, &uiNumberOfWorkGroups);
#else
			__itt_metadata_add(m_pGPAData->pDomain, __itt_null, m_pGPAData->pNumberOfWorkGroupsHandle, __itt_metadata_u32 , 1, &uiNumberOfWorkGroups);
#endif

			__itt_metadata_str_addA(m_pGPAData->pDomain, __itt_null, m_pGPAData->pWorkGroupRangeHandle, pWGRangeString, GPA_RANGE_STRING_SIZE);
		}
#endif
#ifdef _DEBUG
	-- m_lAttaching;
#endif
		return CL_DEV_SUCCESS;
	}

	cl_dev_err_code ret = pCtx->CreateContext(m_pCmd->id,m_pBinary, m_pMemBuffSizes, m_MemBuffCount);
	if ( CL_DEV_FAILED(ret) )
	{
		CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("Failed to create new WG context, Id:%d, ERR:%x"), uiWorkerId, ret);
		m_lastError = (int)ret;
#ifdef _DEBUG
		--m_lAttaching ;
#endif
		return (int)ret;
	}
	pCtx->GetExecutable()->PrepareThread();

	// Start execution task
#if defined(USE_GPA)
		if ((NULL != m_pGPAData) && (m_pGPAData->bUseGPA))
		{
			char pWGRangeString[GPA_RANGE_STRING_SIZE];
		
			unsigned int uiWorkGroupSize = 1;
			const size_t*	pWGSize = m_pBinary->GetWorkGroupSize();
			cl_dev_cmd_param_kernel *cmdParams = (cl_dev_cmd_param_kernel*)m_pCmd->params;

			switch(cmdParams->work_dim)
			{
			case 1:
				sprintf_s(pWGRangeString, "%d - %d", firstWGID[0], lastWGID[0]);
				break;
			case 2:
				sprintf_s(pWGRangeString, "%d.%d - %d.%d", firstWGID[0], firstWGID[1], lastWGID[0], lastWGID[1]);
				break;
			case 3:
				sprintf_s(pWGRangeString, "%d.%d.%d - %d.%d.%d", firstWGID[0], firstWGID[1], firstWGID[2], lastWGID[0], lastWGID[1], lastWGID[2]);
				break;
			}
			
			__itt_string_handle* pKernelNameHandle = __itt_string_handle_createA(m_pBinary->GetKernel()->GetKernelName());
			__itt_task_begin(m_pGPAData->pDomain, __itt_null, __itt_null, pKernelNameHandle);
			// This coloring will be enabled in the future
			//TAL_SetNamedTaskColor(m_pBinary->GetKernel()->GetKernelName(), getR(m_talRGBColor), getG(m_talRGBColor), getB(m_talRGBColor));
			for (unsigned int i=0 ; i<cmdParams->work_dim ; ++i)
			{
				uiWorkGroupSize *= pWGSize[i];
			}
			
			__itt_metadata_add(m_pGPAData->pDomain, __itt_null, m_pGPAData->pWorkGroupSizeHandle, __itt_metadata_u32 , 1, &uiWorkGroupSize);

#if defined(_M_X64)
			__itt_metadata_add(m_pGPAData->pDomain, __itt_null, m_pGPAData->pNumberOfWorkGroupsHandle, __itt_metadata_u64 , 1, &uiNumberOfWorkGroups);
#else
			__itt_metadata_add(m_pGPAData->pDomain, __itt_null, m_pGPAData->pNumberOfWorkGroupsHandle, __itt_metadata_u32 , 1, &uiNumberOfWorkGroups);
#endif

			__itt_metadata_str_addA(m_pGPAData->pDomain, __itt_null, m_pGPAData->pWorkGroupRangeHandle, pWGRangeString, GPA_RANGE_STRING_SIZE);
		}
#endif

#ifdef _DEBUG
	-- m_lAttaching;
#endif

	return ret;
}

int NDRange::DetachFromThread(unsigned int uiWorkerId)
{
    if (NULL != m_affinityMask)
    {
        clResetThreadAffinityMask();
    }
	// End execution task
#if defined(USE_GPA)
	if ((NULL != m_pGPAData) && (m_pGPAData->bUseGPA))
	{
		__itt_task_end(m_pGPAData->pDomain);
	}
#endif
	return GetWGContext(uiWorkerId)->GetExecutable()->RestoreThreadState();
}

void NDRange::ExecuteIteration(size_t x, size_t y, size_t z, unsigned int uiWorkerId)
{
#ifdef _DEBUG
	long lVal = m_lFinish.test_and_set(0, 0);
	assert(lVal == 0);
	++ m_lExecuting;
#endif

	assert(GetWGContext(uiWorkerId));
	ICLDevBackendExecutable* pExec = GetWGContext(uiWorkerId)->GetExecutable();
	// We always start from (0,0,0) and process whole WG
	// No Need in parameters now
#ifdef _DEBUG
	const size_t*	pWGSize = m_pBinary->GetWorkGroupSize();
	cl_dev_cmd_param_kernel *cmdParams = (cl_dev_cmd_param_kernel*)m_pCmd->params;
	size_t tDimArr[3] = {x, y, z};
	for (unsigned int i=0; i<cmdParams->work_dim;++i)
	{
		unsigned int val = (unsigned int)((cmdParams->glb_wrk_size[i])/(pWGSize[i]));
		assert(tDimArr[i]<val);
	}
#endif

#ifdef _DEBUG
	lVal = m_lFinish.test_and_set(0, 0);
	assert(lVal == 0);
#endif

	// Execute WG
	size_t groupId[MAX_WORK_DIM] = {x, y, z};
	pExec->Execute(groupId, NULL, NULL);

#ifdef _DEBUG
	-- m_lExecuting;
#endif

}
