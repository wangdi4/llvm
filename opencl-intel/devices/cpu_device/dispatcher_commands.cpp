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

#if defined(USE_GPA)
	#include "tal.h"
	#include <ittnotify.h>
#endif

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

/**
 * Debug prints flag. Required for (weird) platforms like Linux, where our logger does not work.
 */
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
	DispatcherCommand(pTD)
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

bool ReadWriteMemObject::Execute()
{
	cl_dev_cmd_param_rw*	cmdParams = (cl_dev_cmd_param_rw*)m_pCmd->params;
	cl_mem_obj_descriptor*	pMemObj;
	SMemCpyParams			sCpyParam;

	NotifyCommandStatusChanged(m_pCmd, CL_RUNNING, CL_DEV_SUCCESS);

	cmdParams->memObj->clDevMemObjGetDescriptor(CL_DEVICE_TYPE_CPU, 0, (cl_dev_memobj_handle*)&pMemObj);

	void*	pObjPtr;
	size_t* pObjPitchPtr = cmdParams->memobj_pitch[0] ? cmdParams->memobj_pitch : pMemObj->pitch;

#ifdef _DEBUG_PRINT
	printf("--> ReadWriteMemObject(start), cmdid:%d\n", m_pCmd->id);
#endif


	// Request access on default device

	sCpyParam.uiDimCount = cmdParams->dim_count;
	pObjPtr = MemoryAllocator::CalculateOffsetPointer(pMemObj->pData, sCpyParam.uiDimCount, cmdParams->origin, pObjPitchPtr, pMemObj->uiElementSize);

	// Set Source/Destination
	memcpy(sCpyParam.vRegion, cmdParams->region, sizeof(sCpyParam.vRegion));
	sCpyParam.vRegion[0] = cmdParams->region[0] * pMemObj->uiElementSize;

	// In case the pointer parameter (Destination for CMD_READ and Source for CMD_WRITE) has pitch properties,
	// we need to consider that too.
	size_t ptrOffset =	cmdParams->ptr_origin[2] * cmdParams->pitch[1] + \
						cmdParams->ptr_origin[1] * cmdParams->pitch[0] + \
						cmdParams->ptr_origin[0];

	if ( CL_DEV_CMD_READ == m_pCmd->type )
	{
		sCpyParam.pSrc = (cl_char*)pObjPtr;
		memcpy(sCpyParam.vSrcPitch, pObjPitchPtr, sizeof(sCpyParam.vSrcPitch));
		sCpyParam.pDst = (cl_char*)((size_t)cmdParams->ptr + ptrOffset);
		memcpy(sCpyParam.vDstPitch, cmdParams->pitch, sizeof(sCpyParam.vDstPitch));
	}
	else
	{
		sCpyParam.pSrc = (cl_char*)((size_t)cmdParams->ptr + ptrOffset);
		memcpy(sCpyParam.vSrcPitch, cmdParams->pitch, sizeof(sCpyParam.vSrcPitch));
		sCpyParam.pDst = (cl_char*)pObjPtr;
		memcpy(sCpyParam.vDstPitch, pObjPitchPtr, sizeof(sCpyParam.vDstPitch));
	}

	// Execute copy routine
#if defined(USE_GPA)
	if ((NULL != m_pGPAData) && (m_pGPAData->bUseGPA))
	{

		__itt_set_track(NULL);

		__itt_task_begin(m_pGPAData->pDeviceDomain, __itt_null, __itt_null, ( CL_DEV_CMD_READ == m_pCmd->type ? m_pGPAData->pReadHandle : m_pGPAData->pWriteHandle ));

		TAL_SetNamedTaskColor((CL_DEV_CMD_READ == m_pCmd->type ? "Read" : "Write"), 255, 0, 0);

		switch(cmdParams->dim_count)
		{
#if defined(_M_X64)
		case 1:
			__itt_metadata_add(m_pGPAData->pDeviceDomain, __itt_null, m_pGPAData->pSizeHandle, __itt_metadata_u64, 1, &sCpyParam.vRegion[0]);
			break;
		case 2:
			__itt_metadata_add(m_pGPAData->pDeviceDomain, __itt_null, m_pGPAData->pWidthHandle, __itt_metadata_u64 , 1, &sCpyParam.vRegion[0]);
			__itt_metadata_add(m_pGPAData->pDeviceDomain, __itt_null, m_pGPAData->pHeightHandle, __itt_metadata_u64 , 1, &cmdParams->region[1]);
			break;
		case 3:
			__itt_metadata_add(m_pGPAData->pDeviceDomain, __itt_null, m_pGPAData->pWidthHandle, __itt_metadata_u64 , 1, &sCpyParam.vRegion[0]);
			__itt_metadata_add(m_pGPAData->pDeviceDomain, __itt_null, m_pGPAData->pHeightHandle, __itt_metadata_u64 , 1, &cmdParams->region[1]);
			__itt_metadata_add(m_pGPAData->pDeviceDomain, __itt_null, m_pGPAData->pDepthHandle, __itt_metadata_u64 , 1, &cmdParams->region[2]);
			break;
#else
		case 1:
			__itt_metadata_add(m_pGPAData->pDeviceDomain, __itt_null, m_pGPAData->pSizeHandle, __itt_metadata_u32 , 1, &sCpyParam.vRegion[0]);
			break;
		case 2:
			__itt_metadata_add(m_pGPAData->pDeviceDomain, __itt_null, m_pGPAData->pWidthHandle, __itt_metadata_u32 , 1, &sCpyParam.vRegion[0]);
			__itt_metadata_add(m_pGPAData->pDeviceDomain, __itt_null, m_pGPAData->pHeightHandle, __itt_metadata_u32 , 1, &cmdParams->region[1]);
			break;
		case 3:
			__itt_metadata_add(m_pGPAData->pDeviceDomain, __itt_null, m_pGPAData->pWidthHandle, __itt_metadata_u32 , 1, &sCpyParam.vRegion[0]);
			__itt_metadata_add(m_pGPAData->pDeviceDomain, __itt_null, m_pGPAData->pHeightHandle, __itt_metadata_u32 , 1, &cmdParams->region[1]);
			__itt_metadata_add(m_pGPAData->pDeviceDomain, __itt_null, m_pGPAData->pDepthHandle, __itt_metadata_u32 , 1, &cmdParams->region[2]);
			break;
#endif
		}
	}
#endif

	clCopyMemoryRegion(&sCpyParam);

#if defined(USE_GPA)
	if ((NULL != m_pGPAData) && (m_pGPAData->bUseGPA))
	{
		__itt_set_track(NULL);
		__itt_task_end(m_pGPAData->pDeviceDomain);
	}
#endif

#ifdef _DEBUG_PRINT
	printf("--> ReadWriteMemObject(end), cmdid:%d(%d)\n", m_pCmd->id, CL_DEV_SUCCESS);
#endif
	NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, CL_DEV_SUCCESS);
	return true;
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
	DispatcherCommand(pTD)
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

	return CL_DEV_SUCCESS;
}

bool CopyMemObject::Execute()
{
	cl_dev_cmd_param_copy*	cmdParams = (cl_dev_cmd_param_copy*)m_pCmd->params;
	cl_mem_obj_descriptor*	pSrcMemObj;;
	cl_mem_obj_descriptor*	pDstMemObj;
	SMemCpyParams			sCpyParam;

	cmdParams->srcMemObj->clDevMemObjGetDescriptor(CL_DEVICE_TYPE_CPU, 0, (cl_dev_memobj_handle*)&pSrcMemObj);
	cmdParams->dstMemObj->clDevMemObjGetDescriptor(CL_DEVICE_TYPE_CPU, 0, (cl_dev_memobj_handle*)&pDstMemObj);

	size_t  uiSrcElementSize = pSrcMemObj->uiElementSize;
	size_t	uiDstElementSize = pDstMemObj->uiElementSize;

	// Objects has to have same element size or buffer<->image
	if( (uiDstElementSize != uiSrcElementSize) &&
		(1 != uiDstElementSize) && (1 != uiSrcElementSize) )
	{
#ifdef _DEBUG_PRINT
		printf("--> CopyMemObject(fail,3), cmdid:%d(%d)\n", m_pCmd->id, CL_DEV_INVALID_COMMAND_PARAM);
#endif
		NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, (cl_int)CL_DEV_INVALID_COMMAND_PARAM);
		return true;
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
	memcpy(sCpyParam.vSrcPitch, cmdParams->src_pitch[0] ? cmdParams->src_pitch : pSrcMemObj->pitch, sizeof(sCpyParam.vSrcPitch));
	memcpy(sCpyParam.vDstPitch, cmdParams->dst_pitch[0] ? cmdParams->dst_pitch : pDstMemObj->pitch, sizeof(sCpyParam.vDstPitch));

	sCpyParam.pSrc = (cl_char*)MemoryAllocator::CalculateOffsetPointer(pSrcMemObj->pData, cmdParams->src_dim_count, cmdParams->src_origin, sCpyParam.vSrcPitch, pSrcMemObj->uiElementSize);
	sCpyParam.pDst = (cl_char*)MemoryAllocator::CalculateOffsetPointer(pDstMemObj->pData, cmdParams->dst_dim_count, cmdParams->dst_origin, sCpyParam.vDstPitch, pDstMemObj->uiElementSize);

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


	//If row_pitch (or input_row_pitch) is set to 0, the appropriate row pitch is calculated
	//based on the size of each element in bytes multiplied by width.
	memcpy(sCpyParam.vRegion, cmdParams->region, sizeof(sCpyParam.vRegion));
	sCpyParam.vRegion[0] *= uiSrcElementSize;

	// Execute copy routine
#if defined(USE_GPA)
	if ((NULL != m_pGPAData) && (m_pGPAData->bUseGPA))
	{

		__itt_set_track(NULL);
		__itt_task_begin(m_pGPAData->pDeviceDomain, __itt_null, __itt_null, m_pGPAData->pCopyHandle);
		TAL_SetNamedTaskColor("Copy", 255, 0, 0);

		switch(cmdParams->src_dim_count)
		{
#if defined(_M_X64)
		case 1:
			
			__itt_metadata_add(m_pGPAData->pDeviceDomain, __itt_null, m_pGPAData->pSizeHandle, __itt_metadata_u64 , 1, &sCpyParam.vRegion[0]);
			break;
		case 2:
			__itt_metadata_add(m_pGPAData->pDeviceDomain, __itt_null, m_pGPAData->pWidthHandle, __itt_metadata_u64 , 1, &sCpyParam.vRegion[0]);
			__itt_metadata_add(m_pGPAData->pDeviceDomain, __itt_null, m_pGPAData->pHeightHandle, __itt_metadata_u64 , 1, &cmdParams->region[1]);
			break;
		case 3:
			__itt_metadata_add(m_pGPAData->pDeviceDomain, __itt_null, m_pGPAData->pWidthHandle, __itt_metadata_u64 , 1, &sCpyParam.vRegion[0]);
			__itt_metadata_add(m_pGPAData->pDeviceDomain, __itt_null, m_pGPAData->pHeightHandle, __itt_metadata_u64 , 1, &cmdParams->region[1]);
			__itt_metadata_add(m_pGPAData->pDeviceDomain, __itt_null, m_pGPAData->pDepthHandle, __itt_metadata_u64 , 1, &cmdParams->region[2]);
			break;
#else
		case 1:
			
			__itt_metadata_add(m_pGPAData->pDeviceDomain, __itt_null, m_pGPAData->pSizeHandle, __itt_metadata_u32 , 1, &sCpyParam.vRegion[0]);
			break;
		case 2:
			__itt_metadata_add(m_pGPAData->pDeviceDomain, __itt_null, m_pGPAData->pWidthHandle, __itt_metadata_u32 , 1, &sCpyParam.vRegion[0]);
			__itt_metadata_add(m_pGPAData->pDeviceDomain, __itt_null, m_pGPAData->pHeightHandle, __itt_metadata_u32 , 1, &cmdParams->region[1]);
			break;
		case 3:
			__itt_metadata_add(m_pGPAData->pDeviceDomain, __itt_null, m_pGPAData->pWidthHandle, __itt_metadata_u32 , 1, &sCpyParam.vRegion[0]);
			__itt_metadata_add(m_pGPAData->pDeviceDomain, __itt_null, m_pGPAData->pHeightHandle, __itt_metadata_u32 , 1, &cmdParams->region[1]);
			__itt_metadata_add(m_pGPAData->pDeviceDomain, __itt_null, m_pGPAData->pDepthHandle, __itt_metadata_u32 , 1, &cmdParams->region[2]);
			break;
#endif
		}
	}
#endif	
	
	// Execute copy routine
	clCopyMemoryRegion(&sCpyParam);

#if defined(USE_GPA)
	if ((NULL != m_pGPAData) && (m_pGPAData->bUseGPA))
	{
		__itt_set_track(NULL);
		__itt_task_end(m_pGPAData->pDeviceDomain);
	} 
#endif

#ifdef _DEBUG_PRINT
	printf("--> CopyMemObject(end), cmd_id:%d(%d)\n", m_pCmd->id, CL_DEV_SUCCESS);
#endif
	NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, CL_DEV_SUCCESS);
	return true;
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
	char*	pArgV = new char[cmdParams->args];
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
	DispatcherCommand(pTD)
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

	return CL_DEV_SUCCESS;
}

bool NativeFunction::Execute()
{
	cl_dev_cmd_param_native *cmdParams = (cl_dev_cmd_param_native*)m_pCmd->params;

	NotifyCommandStatusChanged(m_pCmd, CL_RUNNING, CL_DEV_SUCCESS);

	for(unsigned int i=0; (i<cmdParams->mem_num); ++i )
	{
		IOCLDevMemoryObject *memObj = *((IOCLDevMemoryObject**)((((char*)cmdParams->argv)+cmdParams->mem_offset[i])));
		
		cl_mem_obj_descriptor* memObjDesc;
		memObj->clDevMemObjGetDescriptor(CL_DEVICE_TYPE_CPU, 0, (cl_dev_memobj_handle*)&memObjDesc);

		void*	*pMemPtr = (void**)((cl_char*)m_pArgV+cmdParams->mem_offset[i]);
		*pMemPtr = memObjDesc->pData;
	}

	// Notify start execution
	NotifyCommandStatusChanged(m_pCmd, CL_RUNNING, CL_DEV_SUCCESS);

	// Execute native function
	fn_clNativeKernel *func = (fn_clNativeKernel*)cmdParams->func_ptr;
	func(m_pArgV);

	// Free memory allocated to execution parameters
	delete []m_pArgV;

	NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, CL_DEV_SUCCESS);

	return true;
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
	DispatcherCommand(pTD)
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

	return CL_DEV_SUCCESS;
}

bool MapMemObject::Execute()
{
	NotifyCommandStatusChanged(m_pCmd, CL_RUNNING, CL_DEV_SUCCESS);

#ifdef _DEBUG_PRINT
	printf("--> MapMemObject(start), cmdid:%d\n", m_pCmd->id);
#endif

	// Write Map task to TAL trace
#if defined(USE_GPA)
	if ((NULL != m_pGPAData) && (m_pGPAData->bUseGPA))
	{
		__itt_set_track(NULL);
		__itt_task_begin(m_pGPAData->pDeviceDomain, __itt_null, __itt_null, m_pGPAData->pMapHandle);
		TAL_SetNamedTaskColor("Map", 255, 0, 0);
	}
#endif

#if defined(USE_GPA)
	if ((NULL != m_pGPAData) && (m_pGPAData->bUseGPA))
	{
		__itt_set_track(NULL);
		__itt_task_end(m_pGPAData->pDeviceDomain);
	} 
#endif

#ifdef _DEBUG_PRINT
	printf("--> MapMemObject(end), cmdid:%d\n", m_pCmd->id);
#endif

	NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, CL_DEV_SUCCESS);
	return true;
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
	DispatcherCommand(pTD)
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

	return CL_DEV_SUCCESS;
}

bool UnmapMemObject::Execute()
{
	NotifyCommandStatusChanged(m_pCmd, CL_RUNNING, CL_DEV_SUCCESS);

#ifdef _DEBUG_PRINT
	printf("--> UnmapMemObject(start), cmdid:%d\n", m_pCmd->id);
#endif

#ifdef _DEBUG_PRINT
	printf("--> UnmapMemObject(end), cmdid:%d\n", m_pCmd->id);
#endif

	NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, CL_DEV_SUCCESS);
	return true;
}

///////////////////////////////////////////////////////////////////////////
// OCL Kernel execution
Intel::OpenCL::Utils::AtomicCounter	NDRange::s_lGlbNDRangeId;

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


    const ICLDevBackendKernel_* pKernel = (const ICLDevBackendKernel_*)(cmdParams->kernel);
    const ICLDevBackendKernelProporties* pKernelProps = pKernel->GetKernelProporties();
	assert(pKernel);

	size_t	stLocMemSize = 0;

	// Check kernel parameters
    cl_uint                     uiNumArgs = pKernel->GetKernelParamsCount();
    const cl_kernel_argument*   pArgs = pKernel->GetKernelParams();

	cl_char*	pCurrParamPtr = (cl_char*)cmdParams->arg_values;
	size_t		stOffset = 0;
	// Check kernel parameters and memory buffers
	for(unsigned int i=0; i<uiNumArgs; ++i)
	{
		// Argument is buffer object or local memory size
		if ( CL_KRNL_ARG_PTR_GLOBAL <= pArgs[i].type )
		{
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
    stLocMemSize += pKernelProps->GetImplicitLocalMemoryBufferSize();


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

        const size_t    *pReqdWGSize = pKernelProps->GetRequiredWorkGroupSize();
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
    const ICLDevBackendKernel_* pKernel = (ICLDevBackendKernel_*)cmdParams->kernel;
	const char*	pKernelParams = (const char*)cmdParams->arg_values;

#ifdef _DEBUG_PRINT
	printf("--> Init(start):%s, id(%d)\n", pKernel->GetKernelName(), (int)m_pCmd->id);
#endif

	NotifyCommandStatusChanged(m_pCmd, CL_RUNNING, CL_DEV_SUCCESS);

    unsigned                    uiNumArgs = pKernel->GetKernelParamsCount();
    const cl_kernel_argument*   pArgs = pKernel->GetKernelParams();
	size_t						stOffset = 0;

	// Copy initial values
	memcpy(m_pLockedParams, cmdParams->arg_values, cmdParams->arg_size);

	// Lock required memory objects
	for(unsigned int i=0; (i<uiNumArgs) && CL_DEV_SUCCEEDED(m_lastError); ++i)
	{
		// Argument is buffer object or local memory size
		if ( ( CL_KRNL_ARG_PTR_GLOBAL == pArgs[i].type ) ||
			( CL_KRNL_ARG_PTR_CONST == pArgs[i].type ) ||
			( CL_KRNL_ARG_PTR_IMG_2D == pArgs[i].type ) ||
			( CL_KRNL_ARG_PTR_IMG_3D == pArgs[i].type )
			)

		{
			IOCLDevMemoryObject *memObj = *((IOCLDevMemoryObject**)(pKernelParams+stOffset));
            if (NULL != memObj)
            {
			    memObj->clDevMemObjGetDescriptor(CL_DEVICE_TYPE_CPU, 0, (cl_dev_memobj_handle*)(m_pLockedParams+stOffset));
            }
            else
            {
                *(cl_dev_memobj_handle*)(m_pLockedParams + stOffset) = NULL;
            }
			stOffset += sizeof(IOCLDevMemoryObject*);
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

    ICLDevBackendExecutionService* pExecutionService = m_pTaskDispatcher->getProgramService()->GetExecutionService();
    assert(pExecutionService);

    cl_work_description_type workDesc;
    //TODO: Find more elegant solution for filling the workDesc structure
    //      Probably by making it part of cmdParams.
    workDesc.workDimension = cmdParams->work_dim;
    memcpy(workDesc.globalWorkOffset, cmdParams->glb_wrk_offs, sizeof(size_t) * MAX_WORK_DIM);
    memcpy(workDesc.globalWorkSize, cmdParams->glb_wrk_size, sizeof(size_t)* MAX_WORK_DIM);
    memcpy(workDesc.localWorkSize, cmdParams->lcl_wrk_size, sizeof(size_t)* MAX_WORK_DIM);

	// Create an "Binary" for these parameters
    cl_dev_err_code clRet = pExecutionService->CreateBinary(pKernel, 
                                                            m_pLockedParams, 
                                                            cmdParams->arg_size, 
                                                            &workDesc, 
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
    m_pBinary->GetMemoryBuffersDescriptions(NULL, &m_MemBuffCount);
	m_pMemBuffSizes = new size_t[m_MemBuffCount];
	if ( NULL == m_pMemBuffSizes )
	{
		m_lastError = (cl_int)CL_DEV_OUT_OF_MEMORY;
		return -1;
	}
    m_pBinary->GetMemoryBuffersDescriptions(m_pMemBuffSizes, &m_MemBuffCount);

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
	m_lNDRangeId = s_lGlbNDRangeId++;
	return CL_DEV_SUCCESS;
}

void NDRange::Finish(FINISH_REASON reason)
{
#ifdef _DEBUG
	long lVal = (m_lExecuting.test_and_set(0, 0) | m_lAttaching.test_and_set(0, 0));
	assert(lVal == 0);
	m_lFinish.exchange(1);
#endif

#ifdef _DEBUG_PRINT
	cl_dev_cmd_param_kernel *cmdParams = (cl_dev_cmd_param_kernel*)m_pCmd->params;
    const ICLDevBackendKernel_* pKernel = (ICLDevBackendKernel*)cmdParams->kernel;
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


int NDRange::AttachToThread(unsigned int uiWorkerId, size_t uiNumberOfWorkGroups, size_t firstWGID[], size_t lastWGID[])
{
#ifdef _DEBUG_PRINT
	printf("AttachToThread %d, WrkId(%d), CmdId(%d)\n", (int)GetCurrentThreadId(), (int)uiWorkerId, (int)m_pCmd->id);
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

	WGContext* pCtx = GetWGContext(uiWorkerId);
	if ( NULL == pCtx )
	{
		CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("Failed to retrive WG context, Id:%d"), uiWorkerId);
		m_lastError = (cl_int)CL_DEV_ERROR_FAIL;
#ifdef _DEBUG
	--m_lAttaching ;
#endif	
		return (cl_int)CL_DEV_ERROR_FAIL;
	}

	if (m_lNDRangeId != pCtx->GetNDRCmdId() )
	{
		cl_dev_err_code ret = pCtx->CreateContext(m_lNDRangeId, m_pBinary, m_pMemBuffSizes, m_MemBuffCount);
		if ( CL_DEV_FAILED(ret) )
		{
			CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("Failed to create new WG context, Id:%d, ERR:%x"), uiWorkerId, ret);
			m_lastError = (int)ret;
	#ifdef _DEBUG
			--m_lAttaching ;
	#endif
			return (int)ret;
		}
	}

	pCtx->GetExecutable()->PrepareThread();

	// Start execution task
#if defined(USE_GPA)
	if ((NULL != m_pGPAData) && (m_pGPAData->bUseGPA))
	{
		__itt_set_track(NULL);

		char pWGRangeString[GPA_RANGE_STRING_SIZE];
	
		unsigned int uiWorkGroupSize = 1;
		const size_t*	pWGSize = m_pBinary->GetWorkGroupSize();
		cl_dev_cmd_param_kernel *cmdParams = (cl_dev_cmd_param_kernel*)m_pCmd->params;
        const ICLDevBackendKernel_* pKernel = (ICLDevBackendKernel_*)cmdParams->kernel;


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
		
        __itt_string_handle* pKernelNameHandle = __itt_string_handle_createA(pKernel->GetKernelName());
		__itt_task_begin(m_pGPAData->pDeviceDomain, __itt_null, __itt_null, pKernelNameHandle);
		// This coloring will be enabled in the future
		//TAL_SetNamedTaskColor(m_pBinary->GetKernel()->GetKernelName(), getR(m_talRGBColor), getG(m_talRGBColor), getB(m_talRGBColor));
		for (unsigned int i=0 ; i<cmdParams->work_dim ; ++i)
		{
			uiWorkGroupSize *= (unsigned int)pWGSize[i];
		}
		
		__itt_metadata_add(m_pGPAData->pDeviceDomain, __itt_null, m_pGPAData->pWorkGroupSizeHandle, __itt_metadata_u32 , 1, &uiWorkGroupSize);

#if defined(_M_X64)
		__itt_metadata_add(m_pGPAData->pDeviceDomain, __itt_null, m_pGPAData->pNumberOfWorkGroupsHandle, __itt_metadata_u64 , 1, &uiNumberOfWorkGroups);
#else
		__itt_metadata_add(m_pGPAData->pDeviceDomain, __itt_null, m_pGPAData->pNumberOfWorkGroupsHandle, __itt_metadata_u32 , 1, &uiNumberOfWorkGroups);
#endif

			__itt_metadata_str_addA(m_pGPAData->pDeviceDomain, __itt_null, m_pGPAData->pWorkGroupRangeHandle, pWGRangeString, GPA_RANGE_STRING_SIZE);
		}
#endif

#ifdef _DEBUG
	-- m_lAttaching;
#endif

	return CL_DEV_SUCCESS;
}

int NDRange::DetachFromThread(unsigned int uiWorkerId)
{
	// End execution task
#if defined(USE_GPA)
	if ((NULL != m_pGPAData) && (m_pGPAData->bUseGPA))
	{
		__itt_set_track(NULL);
		__itt_task_end(m_pGPAData->pDeviceDomain);
	}
#endif
    WGContext* pCtx = GetWGContext(uiWorkerId);
    int ret = pCtx->GetExecutable()->RestoreThreadState();

    return ret;

}

void NDRange::ExecuteIteration(size_t x, size_t y, size_t z, unsigned int uiWorkerId)
{
#ifdef _DEBUG
	long lVal = m_lFinish.test_and_set(0, 0);
	assert(lVal == 0);
	++ m_lExecuting;
#endif

	assert(GetWGContext(uiWorkerId));
    ICLDevBackendExecutable_* pExec = GetWGContext(uiWorkerId)->GetExecutable();
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

void NDRange::ExecuteAllIterations(size_t* dims, unsigned int uiWorkerId)
{
#ifdef _DEBUG
    long lVal = m_lFinish.test_and_set(0, 0);
    assert(lVal == 0);
    ++ m_lExecuting;
#endif

    assert(GetWGContext(uiWorkerId));
    ICLDevBackendExecutable_* pExec = GetWGContext(uiWorkerId)->GetExecutable();
    // We always start from (0,0,0) and process whole WG
    // No Need in parameters now
#ifdef _DEBUG
    lVal = m_lFinish.test_and_set(0, 0);
    assert(lVal == 0);
#endif

    // Execute WG
    size_t groupId[MAX_WORK_DIM] = {0, 0, 0};
    for (; groupId[2] < dims[2]; ++groupId[2])
    {
        for (groupId[1] = 0; groupId[1] < dims[1]; ++groupId[1])
        {
            for (groupId[0] = 0; groupId[0] < dims[0]; ++groupId[0])
            {
                pExec->Execute(groupId, NULL, NULL);
            }
        }
    }

#ifdef _DEBUG
    -- m_lExecuting;
#endif

}

///////////////////////////////////////////////////////////////////////////
// OCL Fill buffer/image execution

cl_dev_err_code FillMemObject::Create(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, ITaskBase* *pTask)
{
	FillMemObject* pCommand = new FillMemObject(pTD);
	if (NULL == pCommand)
	{
		return CL_DEV_OUT_OF_MEMORY;
	}
#ifdef _DEBUG
	cl_dev_err_code rc;
	rc = pCommand->CheckCommandParams(pCmd);
	assert(CL_DEV_SUCCESS == rc);
#endif

	pCommand->m_pCmd = pCmd;
	assert(pTask);
	*pTask = static_cast<ITaskBase*>(pCommand);

	return CL_DEV_SUCCESS;
}


FillMemObject::FillMemObject(TaskDispatcher* pTD) :
	DispatcherCommand(pTD)
{
}

cl_dev_err_code FillMemObject::CheckCommandParams(cl_dev_cmd_desc* cmd)
{
	if ( (CL_DEV_CMD_FILL_BUFFER != cmd->type) && (CL_DEV_CMD_FILL_IMAGE != cmd->type) )
	{
		return CL_DEV_INVALID_COMMAND_TYPE;
	}

	if ( sizeof(cl_dev_cmd_param_fill) != cmd->param_size )
	{
		return CL_DEV_INVALID_COMMAND_PARAM;
	}

	cl_dev_cmd_param_fill *cmdParams = (cl_dev_cmd_param_fill*)(cmd->params);

	for (unsigned int i=0; i< cmdParams->dim_count-1; i++)
	{
		// offset may be 0, but region must be bigger than 0 for used dimmensions.
		if(0 == cmdParams->region[i])
		{
			return CL_DEV_INVALID_VALUE;
		}
	}

	// TODO: Check Region
	return CL_DEV_SUCCESS;
}

bool FillMemObject::Execute()
{
	cl_dev_cmd_param_fill*	cmdParams = (cl_dev_cmd_param_fill*)m_pCmd->params;
	cl_mem_obj_descriptor*  pMemObj;

	NotifyCommandStatusChanged(m_pCmd, CL_RUNNING, CL_DEV_SUCCESS);

	cmdParams->memObj->clDevMemObjGetDescriptor(CL_DEVICE_TYPE_CPU, 0, (cl_dev_memobj_handle*)&pMemObj);

#ifdef _DEBUG_PRINT
	printf("--> FillMemObject(start), cmdid:%d\n", m_pCmd->id);
#endif

	size_t depthStart = 0;
	size_t depthEnd = 1;
	if (cmdParams->dim_count > 2)
	{
		depthStart = cmdParams->offset[2];
		depthEnd = depthStart + cmdParams->region[2];
	}

	size_t heightStart = 0;
	size_t heightEnd = 1;
	if (cmdParams->dim_count > 1)
	{
		heightStart = cmdParams->offset[1];
		heightEnd = heightStart + cmdParams->region[1];
	}

	// width, unlike height and depth, is in bytes.
	size_t width =  cmdParams->region[0] * pMemObj->uiElementSize;

	// prepare copy buffer:
	char* fillBuf = (char*)malloc(width);
	if (NULL == fillBuf) return false;

    for (size_t offset=0 ; offset < width ; offset += cmdParams->pattern_size)
	{
		memcpy((fillBuf + offset), cmdParams->pattern, cmdParams->pattern_size);
	}

#ifdef _DEBUG_PRINT
	fprintf(stderr, "Going to fill [%lu,%lu,%lu] to [%lu,%lu,%lu]- with buffer of len %lu bytes\n", cmdParams->offset[0], heightStart, depthStart,
        cmdParams->offset[0] + cmdParams->region[0], heightEnd, depthEnd, width);
    fprintf(stderr, "dim_count is %u ; pitches: %lu, %lu ; element size: %u ; color: ", cmdParams->dim_count, pMemObj->pitch[0], pMemObj->pitch[1], pMemObj->uiElementSize);
    for (size_t i=0 ; i < cmdParams->pattern_size ; ++i)
    {
        fprintf(stderr, "%02x ", cmdParams->pattern[i]);
    }
    fprintf(stderr, "\n");
#endif
    
    size_t offset[MAX_WORK_DIM];
	offset[0] = cmdParams->offset[0];
	for (size_t depth = depthStart ; depth < depthEnd ; ++depth)
	{
		offset[2] = depth;
		for (size_t height = heightStart ; height < heightEnd ; ++height)
		{
			offset[1] = height;
			 void* dst = MemoryAllocator::CalculateOffsetPointer(pMemObj->pData, cmdParams->dim_count,
					offset, pMemObj->pitch, pMemObj->uiElementSize);
			memcpy(dst, fillBuf, width);
		}
	}

	free(fillBuf);

#ifdef _DEBUG_PRINT
	printf("--> FillMemObject(end), cmdid:%d(%d)\n", m_pCmd->id, CL_DEV_SUCCESS);
#endif
	NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, CL_DEV_SUCCESS);
	return true;
}
