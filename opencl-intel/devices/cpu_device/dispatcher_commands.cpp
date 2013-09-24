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
#include "cl_shared_ptr.hpp"
#include <builtin_kernels.h>
#include <cl_dev_backend_api.h>
#include <cl_sys_defines.h>
#include <ocl_itt.h>

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

using namespace Intel::OpenCL::BuiltInKernels;

/**
 * Debug prints flag. Required for (weird) platforms like Linux, where our logger does not work.
 */
//#define _DEBUG_PRINT


///////////////////////////////////////////////////////////////////////////
// Base dispatcher command
DispatcherCommand::DispatcherCommand(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd) :
m_pTaskDispatcher(pTD), m_pCmd(pCmd), m_bCompleted(false)
{
	assert(pTD && "Expected non NULL TaskDispatcher");
	assert(pCmd && "Expected non NLL command descriptor");
	m_pLogDescriptor = pTD->m_pLogDescriptor;
	m_iLogHandle = pTD->m_iLogHandle;
	m_pMemAlloc = pTD->m_pMemoryAllocator;
	m_pGPAData = pTD->m_pGPAData;
#if defined(USE_ITT)
    if ((NULL != m_pGPAData) && (m_pGPAData->bUseGPA))
    {
	    // unique ID to pass all tasks, and markers.
	    m_ittID = __itt_id_make(&m_ittID, (unsigned long long)this);
	    __itt_id_create(m_pGPAData->pDeviceDomain, m_ittID);
    }
#endif
}

DispatcherCommand::~DispatcherCommand()
{
#if defined(USE_ITT)
    if ((NULL != m_pGPAData) && (m_pGPAData->bUseGPA))
    {
	    __itt_id_destroy(m_pGPAData->pDeviceDomain, m_ittID);
    }
#endif
}

void DispatcherCommand::NotifyCommandStatusChanged(cl_dev_cmd_desc* cmd, unsigned uStatus, int iErr)
{
	if ( CL_COMPLETE == uStatus )
	{
		void* pTaskPtr = cmd->device_agent_data;
		// If the ITask pointer still exists we need reduce reference count
		// and release ITask object
		if ( NULL != pTaskPtr )
		{
			ITaskBase* pTask = static_cast<ITaskBase*>(pTaskPtr);
			pTask->Release();
		}
		m_bCompleted = true;
	}
	m_pTaskDispatcher->NotifyCommandStatusChange(cmd, uStatus, iErr);
}

cl_dev_err_code DispatcherCommand::ExtractNDRangeParams(void* pTargetTaskParam)
{
    cl_dev_cmd_param_kernel *cmdParams = (cl_dev_cmd_param_kernel*)m_pCmd->params;
    const ICLDevBackendKernel_* pKernel = ((const ProgramService::KernelMapEntry*)cmdParams->kernel)->pBEKernel;
    const char*	pKernelParams = (const char*)cmdParams->arg_values;

    unsigned                    uiNumArgs = pKernel->GetKernelParamsCount();
    const cl_kernel_argument*   pArgs = pKernel->GetKernelParams();
    size_t						stOffset = 0;

    // Copy initial values
    memcpy(pTargetTaskParam, cmdParams->arg_values, cmdParams->arg_size);
    char* pLockedParams = (char*)pTargetTaskParam;

    // Lock required memory objects
    for(unsigned int i=0; i<uiNumArgs; ++i)
    {
        // Argument is buffer object or local memory size
        if ( ( CL_KRNL_ARG_PTR_GLOBAL == pArgs[i].type ) ||
            ( CL_KRNL_ARG_PTR_CONST == pArgs[i].type ) ||
            ( CL_KRNL_ARG_PTR_IMG_1D == pArgs[i].type ) ||
            ( CL_KRNL_ARG_PTR_IMG_1D_ARR == pArgs[i].type ) ||
            ( CL_KRNL_ARG_PTR_IMG_1D_BUF == pArgs[i].type ) ||
            ( CL_KRNL_ARG_PTR_IMG_2D_ARR == pArgs[i].type ) ||
            ( CL_KRNL_ARG_PTR_IMG_2D_ARR_DEPTH == pArgs[i].type ) ||
            ( CL_KRNL_ARG_PTR_IMG_2D == pArgs[i].type ) ||
            ( CL_KRNL_ARG_PTR_IMG_2D_DEPTH == pArgs[i].type ) ||
            ( CL_KRNL_ARG_PTR_IMG_3D == pArgs[i].type )
            )
        {
            IOCLDevMemoryObject *memObj = *((IOCLDevMemoryObject**)(pKernelParams+stOffset));
            assert( ((CL_KRNL_ARG_PTR_CONST == pArgs[i].type) || (CL_KRNL_ARG_PTR_GLOBAL == pArgs[i].type) || (NULL != memObj)) &&
                "NULL is not allowed for non buffer arguments");
            if (NULL != memObj)
            {
                memObj->clDevMemObjGetDescriptor(CL_DEVICE_TYPE_CPU, 0, (cl_dev_memobj_handle*)(pLockedParams+stOffset));
                assert( (*((cl_mem_obj_descriptor**)(pLockedParams+stOffset)))->pData != NULL &&
                  "Passing NULL data object for execution");
            }
            else
            {
                *(cl_dev_memobj_handle*)(pLockedParams + stOffset) = NULL;
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

    return CL_DEV_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////
// CommandBaseClass
template <class ITaskClass>
	bool CommandBaseClass<ITaskClass>::SetAsSyncPoint()
{
	long prev = m_aIsSyncPoint.exchange(1);
	return 1 == prev;
}

template <class ITaskClass>
	bool CommandBaseClass<ITaskClass>::CompleteAndCheckSyncPoint()
{
	// The queue some how need to be signaled stop processing of the job list.
	// On other side RT should be aware that the command was done.
	// And it should be done by single instruction, otherwise there is a race
	long prev = m_aIsSyncPoint.exchange(1);
	return 1 == prev;
}

///////////////////////////////////////////////////////////////////////////
// OCL Read/Write buffer execution

cl_dev_err_code ReadWriteMemObject::Create(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, SharedPtr<ITaskBase>* pTask, const SharedPtr<ITaskList>& pList)
{
	ReadWriteMemObject* pCommand = new ReadWriteMemObject(pTD, pCmd);
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

	assert(pTask);
	*pTask = pCommand;

	return CL_DEV_SUCCESS;
}

ReadWriteMemObject::ReadWriteMemObject(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd) :
	CommandBaseClass<ITask>(pTD, pCmd)
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
	printf("--> ReadWriteMemObject(start), cmdid:%p\n", m_pCmd->id);
#endif


	// Request access on default device

	sCpyParam.uiDimCount = cmdParams->dim_count;
	pObjPtr = MemoryAllocator::CalculateOffsetPointer(pMemObj->pData, sCpyParam.uiDimCount, cmdParams->origin, pObjPitchPtr, pMemObj->uiElementSize);

	// Set Source/Destination
	MEMCPY_S(sCpyParam.vRegion, sizeof(sCpyParam.vRegion), cmdParams->region, sizeof(sCpyParam.vRegion));
	sCpyParam.vRegion[0] = cmdParams->region[0] * pMemObj->uiElementSize;

	// In case the pointer parameter (Destination for CMD_READ and Source for CMD_WRITE) has pitch properties,
	// we need to consider that too.
	size_t ptrOffset =	cmdParams->ptr_origin[2] * cmdParams->pitch[1] + \
						cmdParams->ptr_origin[1] * cmdParams->pitch[0] + \
						cmdParams->ptr_origin[0];

	if ( CL_DEV_CMD_READ == m_pCmd->type )
	{
		sCpyParam.pSrc = (cl_char*)pObjPtr;
		MEMCPY_S(sCpyParam.vSrcPitch, sizeof(sCpyParam.vSrcPitch), pObjPitchPtr, sizeof(sCpyParam.vSrcPitch));
		sCpyParam.pDst = (cl_char*)((size_t)cmdParams->ptr + ptrOffset);
		MEMCPY_S(sCpyParam.vDstPitch, sizeof(sCpyParam.vDstPitch), cmdParams->pitch, sizeof(sCpyParam.vDstPitch));
	}
	else
	{
		sCpyParam.pSrc = (cl_char*)((size_t)cmdParams->ptr + ptrOffset);
		MEMCPY_S(sCpyParam.vSrcPitch, sizeof(sCpyParam.vSrcPitch), cmdParams->pitch, sizeof(sCpyParam.vSrcPitch));
		sCpyParam.pDst = (cl_char*)pObjPtr;
		MEMCPY_S(sCpyParam.vDstPitch, sizeof(sCpyParam.vDstPitch), pObjPitchPtr, sizeof(sCpyParam.vDstPitch));
	}

	// Execute copy routine
#if defined(USE_ITT)
	if ((NULL != m_pGPAData) && (m_pGPAData->bUseGPA))
	{

#if defined(USE_GPA)
		__itt_set_track(NULL);
#endif
		__itt_task_begin(m_pGPAData->pDeviceDomain, m_ittID, __itt_null, ( CL_DEV_CMD_READ == m_pCmd->type ? m_pGPAData->pReadHandle : m_pGPAData->pWriteHandle ));

#if defined(USE_GPA)
		TAL_SetNamedTaskColor((CL_DEV_CMD_READ == m_pCmd->type ? "Read" : "Write"), 255, 0, 0);
#endif

		// Copy dimensions to 64 bit, for uniformity.
		cl_ulong copyParams64[MAX_WORK_DIM];
		copyParams64[0] = sCpyParam.vRegion[0];
		copyParams64[1] = cmdParams->region[1];
		copyParams64[2] = cmdParams->region[2];

        __itt_metadata_add(m_pGPAData->pDeviceDomain, m_ittID, m_pGPAData->pSizeHandle, __itt_metadata_u64, cmdParams->dim_count, copyParams64);
	}
#endif // ITT

	clCopyMemoryRegion(&sCpyParam);

#if defined(USE_ITT)
	if ((NULL != m_pGPAData) && (m_pGPAData->bUseGPA))
	{
#if defined(USE_GPA)
		__itt_set_track(NULL);
#endif
		__itt_task_end(m_pGPAData->pDeviceDomain);
	}
#endif // ITT

#ifdef _DEBUG_PRINT
	printf("--> ReadWriteMemObject(end), cmdid:%p(%d)\n", m_pCmd->id, CL_DEV_SUCCESS);
#endif
	NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, CL_DEV_SUCCESS);

	return true;
}

///////////////////////////////////////////////////////////////////////////
// OCL Copy memory object execution
cl_dev_err_code CopyMemObject::Create(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, SharedPtr<ITaskBase>* pTask, const SharedPtr<ITaskList>& pList)
{
	CopyMemObject* pCommand = new CopyMemObject(pTD, pCmd);
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

	assert(pTask);
	*pTask = static_cast<ITaskBase*>(pCommand);

	return CL_DEV_SUCCESS;
}

CopyMemObject::CopyMemObject(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd) :
	CommandBaseClass<ITask>(pTD, pCmd)
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
		printf("--> CopyMemObject(fail,3), cmdid:%p(%d)\n", m_pCmd->id, CL_DEV_INVALID_COMMAND_PARAM);
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
	MEMCPY_S(sCpyParam.vSrcPitch,  sizeof(sCpyParam.vSrcPitch), cmdParams->src_pitch[0] ? cmdParams->src_pitch : pSrcMemObj->pitch, sizeof(sCpyParam.vSrcPitch));
	MEMCPY_S(sCpyParam.vDstPitch, sizeof(sCpyParam.vDstPitch), cmdParams->dst_pitch[0] ? cmdParams->dst_pitch : pDstMemObj->pitch, sizeof(sCpyParam.vDstPitch));

	sCpyParam.pSrc = (cl_char*)MemoryAllocator::CalculateOffsetPointer(pSrcMemObj->pData, cmdParams->src_dim_count, cmdParams->src_origin, sCpyParam.vSrcPitch, pSrcMemObj->uiElementSize);
	sCpyParam.pDst = (cl_char*)MemoryAllocator::CalculateOffsetPointer(pDstMemObj->pData, cmdParams->dst_dim_count, cmdParams->dst_origin, sCpyParam.vDstPitch, pDstMemObj->uiElementSize);

	sCpyParam.uiDimCount = min(cmdParams->src_dim_count, cmdParams->dst_dim_count);
	if(cmdParams->dst_dim_count != cmdParams->src_dim_count)
	{
		//Buffer to image
		if (CL_MEM_OBJECT_BUFFER == pSrcMemObj->memObjType)
		{
			uiSrcElementSize = uiDstElementSize;
			sCpyParam.uiDimCount = cmdParams->dst_dim_count;
			sCpyParam.vSrcPitch[0] = cmdParams->region[0] * uiDstElementSize;
			sCpyParam.vSrcPitch[1] = sCpyParam.vSrcPitch[0] * cmdParams->region[1];
		}
		if (CL_MEM_OBJECT_BUFFER == pDstMemObj->memObjType)
		{
			//When destination is buffer the memcpy will be done as if the buffer is an image with height=1
			sCpyParam.uiDimCount = cmdParams->src_dim_count;
			sCpyParam.vDstPitch[0] = cmdParams->region[0] * uiSrcElementSize;
			sCpyParam.vDstPitch[1] = sCpyParam.vDstPitch[0] * cmdParams->region[1];
		}
	}


	//If row_pitch (or input_row_pitch) is set to 0, the appropriate row pitch is calculated
	//based on the size of each element in bytes multiplied by width.
	MEMCPY_S(sCpyParam.vRegion, sizeof(sCpyParam.vRegion), cmdParams->region, sizeof(sCpyParam.vRegion));
	sCpyParam.vRegion[0] *= uiSrcElementSize;

#if defined(USE_ITT)
	// Execute copy routine
	if ((NULL != m_pGPAData) && (m_pGPAData->bUseGPA))
	{
#if defined(USE_GPA)
		__itt_set_track(NULL);
#endif
		__itt_task_begin(m_pGPAData->pDeviceDomain, m_ittID, __itt_null, m_pGPAData->pCopyHandle);
#if defined(USE_GPA)
		TAL_SetNamedTaskColor("Copy", 255, 0, 0);
#endif

		// Copy dimensions to 64 bit, for uniformity.
		cl_ulong copyParams64[MAX_WORK_DIM];
		copyParams64[0] = sCpyParam.vRegion[0];
		copyParams64[1] = cmdParams->region[1];
		copyParams64[2] = cmdParams->region[2];

        __itt_metadata_add(m_pGPAData->pDeviceDomain, m_ittID, m_pGPAData->pSizeHandle, __itt_metadata_u64 , cmdParams->src_dim_count, copyParams64);
	}
#endif // ITT
	
	// Execute copy routine
	clCopyMemoryRegion(&sCpyParam);

#if defined(USE_ITT)
	if ((NULL != m_pGPAData) && (m_pGPAData->bUseGPA))
	{
#if defined(USE_GPA)
		__itt_set_track(NULL);
#endif
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
cl_dev_err_code NativeFunction::Create(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, SharedPtr<ITaskBase>* pTask, const SharedPtr<ITaskList>& pList)
{
	NativeFunction* pCommand = new NativeFunction(pTD, pCmd);
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

	cl_dev_cmd_param_native *cmdParams = (cl_dev_cmd_param_native*)pCmd->params;

	// Create temporal buffer for execution
	char*	pArgV = new char[cmdParams->args];
	if ( NULL == pArgV )
	{
#ifdef _DEBUG
		CpuErrLog(pCommand->m_pLogDescriptor, pCommand->m_iLogHandle, TEXT("%s"), TEXT("Can't allocate memory for parameters"));
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

NativeFunction::NativeFunction(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd) :
	CommandBaseClass<ITask>(pTD, pCmd)
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
cl_dev_err_code MapMemObject::Create(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, SharedPtr<ITaskBase>* pTask, const SharedPtr<ITaskList>& pList)
{
	MapMemObject* pCommand = new MapMemObject(pTD, pCmd);
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

	assert(pTask);
	*pTask = static_cast<ITaskBase*>(pCommand);
	return CL_DEV_SUCCESS;
}

MapMemObject::MapMemObject(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd) :
	CommandBaseClass<ITask>(pTD, pCmd)
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
	printf("--> MapMemObject(start), cmdid:%p\n", m_pCmd->id);
#endif

#if defined(USE_ITT)
	// Write Map task to ITT trace
	if ((NULL != m_pGPAData) && (m_pGPAData->bUseGPA))
	{
#if defined(USE_GPA)
		__itt_set_track(NULL);
#endif
		__itt_task_begin(m_pGPAData->pDeviceDomain, m_ittID, __itt_null, m_pGPAData->pMapHandle);
#if defined(USE_GPA)
		TAL_SetNamedTaskColor("Map", 255, 0, 0);
#endif
	}

	if ((NULL != m_pGPAData) && (m_pGPAData->bUseGPA))
	{
#if defined(USE_GPA)
		__itt_set_track(NULL);
#endif
		__itt_task_end(m_pGPAData->pDeviceDomain);
	} 
#endif

#ifdef _DEBUG_PRINT
	printf("--> MapMemObject(end), cmdid:%p\n", m_pCmd->id);
#endif

	NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, CL_DEV_SUCCESS);

	return true;
}

///////////////////////////////////////////////////////////////////////////
// OCL Unmap buffer execution
//////////////////////////////////////////////////////////////////////////
cl_dev_err_code UnmapMemObject::Create(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, SharedPtr<ITaskBase>* pTask, const SharedPtr<ITaskList>& pList)
{
	UnmapMemObject* pCommand = new UnmapMemObject(pTD, pCmd);
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

	assert(pTask);
	*pTask = static_cast<ITaskBase*>(pCommand);
	return CL_DEV_SUCCESS;
}

UnmapMemObject::UnmapMemObject(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd) :
	CommandBaseClass<ITask>(pTD, pCmd)
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
	printf("--> UnmapMemObject(start), cmdid:%p\n", m_pCmd->id);
#endif

#ifdef _DEBUG_PRINT
	printf("--> UnmapMemObject(end), cmdid:%p\n", m_pCmd->id);
#endif

	NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, CL_DEV_SUCCESS);

	return true;
}

///////////////////////////////////////////////////////////////////////////
// OCL Kernel execution
Intel::OpenCL::Utils::AtomicCounter	NDRange::s_lGlbNDRangeId;

cl_dev_err_code NDRange::Create(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, SharedPtr<ITaskBase>* pTask, const SharedPtr<ITaskList>& pList)
{
#ifdef __INCLUDE_MKL__
	// First to check if the required NDRange is one of the built-in kernels
	ICLDevBackendKernel_* pKernel = (ICLDevBackendKernel_*)(((cl_dev_cmd_param_kernel*)pCmd->params)->kernel);
	const ICLDevBackendKernelProporties* pProperties = pKernel->GetKernelProporties();
	
	assert( NULL != pProperties && "Kernel properties always shall exist");

	// Built-in kernel currently returns -1 for Execution lenght properties.
	if ( (size_t)-1 == pProperties->GetKernelExecutionLength() )
	{
		return NativeKernelTask::Create(pTD, pCmd, pTask);
	}
#endif
	pCmd->id = (cl_dev_cmd_id)((long)pCmd->id & ~(1L << (sizeof(long) * 8 - 1)));	// device NDRange IDs have their MSB set, while in host NDRange IDs they're reset
	NDRange* pCommand = new NDRange(pTD, pCmd, pList, NULL, pList->GetNDRangeChildrenTaskGroup());
#ifdef _DEBUG
	cl_dev_err_code rc;
	rc = pCommand->CheckCommandParams(pCmd);
	if( CL_DEV_FAILED(rc))
	{
		delete pCommand;
		return rc;
	}
#endif

	assert(pTask);
	*pTask = static_cast<ITaskBase*>(pCommand);

	return CL_DEV_SUCCESS;
}

cl_dev_err_code NDRange::CreateBinary(const ICLDevBackendKernel_* kernel, size_t szArgSize, const cl_work_description_type* workDesc, ICLDevBackendBinary_** pBinary)
{
	ICLDevBackendExecutionService* pExecutionService = m_pTaskDispatcher->getProgramService()->GetExecutionService();
	assert( NULL!=pExecutionService && "NULL==pExecutionService is not expected");
	return pExecutionService->CreateBinary(kernel, m_pLockedParams, szArgSize, workDesc, pBinary);
}

THREAD_LOCAL WGContext* NDRange::sm_pCurrentWgContext = NULL;

NDRange::NDRange(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, const SharedPtr<ITaskList>& pList, const SharedPtr<KernelCommand>& parent, const SharedPtr<ITaskGroup>& childrenTaskGroup) :
	CommandBaseClass<ITaskSet>(pTD, pCmd), KernelCommand(pList, parent, childrenTaskGroup, this), m_lastError(CL_DEV_SUCCESS), m_pBinary(NULL),
	m_pMemBuffSizes(NULL), m_numThreads(0), m_bEnablePredictablePartitioning(false)
{
#ifdef _DEBUG
	memset(m_pLockedParams, 0x88, sizeof(m_pLockedParams));
	m_lFinish.exchange(0);
	m_lAttaching.exchange(0);
	m_lExecuting.exchange(0);
#endif
	m_numThreads = pTD->getNumberOfThreads();
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


    const ICLDevBackendKernel_* pKernel = ((const ProgramService::KernelMapEntry*)cmdParams->kernel)->pBEKernel;
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
    const ICLDevBackendKernel_* pKernel = ((const ProgramService::KernelMapEntry*)cmdParams->kernel)->pBEKernel;

#ifdef _DEBUG_PRINT
    printf("--> Init(start):%s, id(%d)\n", pKernel->GetKernelName(), (int)m_pCmd->id);
#endif

    if ( CL_DEV_FAILED(m_lastError) )
    {
        return m_lastError;
    }

    NotifyCommandStatusChanged(m_pCmd, CL_RUNNING, CL_DEV_SUCCESS);

#ifdef USE_ITT
    // Start execution task
    if ((NULL != m_pGPAData) && (m_pGPAData->bUseGPA))
    {
        cl_dev_cmd_param_kernel *cmdParams = (cl_dev_cmd_param_kernel*)m_pCmd->params;

        __itt_task_begin(m_pGPAData->pDeviceDomain, m_ittID, __itt_null,
            ((const ProgramService::KernelMapEntry*)cmdParams->kernel)->ittTaskNameHandle);
    }
#endif
    cl_dev_err_code clRet = ExtractNDRangeParams(m_pLockedParams);
    if ( CL_DEV_FAILED(clRet) )
    {
        m_lastError = clRet;
        NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, clRet);
        return clRet;
    }

    cl_work_description_type workDesc;
    //TODO: Find more elegant solution for filling the workDesc structure
    //      Probably by making it part of cmdParams.
    workDesc.workDimension = cmdParams->work_dim;
    MEMCPY_S(workDesc.globalWorkOffset, sizeof(workDesc.globalWorkOffset), cmdParams->glb_wrk_offs, sizeof(size_t) * MAX_WORK_DIM);
    MEMCPY_S(workDesc.globalWorkSize, sizeof(workDesc.globalWorkSize), cmdParams->glb_wrk_size, sizeof(size_t)* MAX_WORK_DIM);
    MEMCPY_S(workDesc.localWorkSize, sizeof(workDesc.localWorkSize), cmdParams->lcl_wrk_size, sizeof(size_t)* MAX_WORK_DIM);

    //TODO: calibrate
    workDesc.minWorkGroupNum = m_pTaskDispatcher->getNumberOfThreads();

    // Create an "Binary" for these parameters
    clRet = CreateBinary(pKernel, cmdParams->arg_size, &workDesc, &m_pBinary);
    if ( CL_DEV_FAILED(clRet) )
    {
        m_lastError = clRet;
        NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, clRet);
        return clRet;
    }

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
  
    //TODO: might want to revisit these restrictions in the future
    m_bEnablePredictablePartitioning = ((1 == dimCount) && (region[0] == m_numThreads) && m_pTaskDispatcher->isPredictablePartitioningAllowed() );
    m_bWGExecuted.init(m_numThreads, false);

#ifdef _DEBUG_PRINT
    printf("--> Init(done):%s\n", pKernel->GetKernelName());
#endif
    m_lNDRangeId = s_lGlbNDRangeId++;
    return CL_DEV_SUCCESS;
}

bool NDRange::Finish(FINISH_REASON reason)
{
    // KernelCommand stuff:
    WaitForChildrenCompletion();

    // regular stuff:
#ifdef _DEBUG
    long lVal = (m_lExecuting.test_and_set(0, 0) | m_lAttaching.test_and_set(0, 0));
    assert(lVal == 0);
    m_lFinish.exchange(1);
#endif

#ifdef _DEBUG_PRINT
    cl_dev_cmd_param_kernel *cmdParams = (cl_dev_cmd_param_kernel*)m_pCmd->params;
    const ICLDevBackendKernel_* pKernel = ((const ProgramService::KernelMapEntry*)cmdParams->kernel)->pBEKernel;
#endif
#ifdef _DEBUG
    lVal = (m_lExecuting.test_and_set(0, 0) | m_lAttaching.test_and_set(0, 0));
    assert(lVal == 0);
#endif

    NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, m_lastError);
    SignalComplete(FINISH_COMPLETED == reason ? CL_DEV_SUCCESS : CL_DEV_ERROR_FAIL);
#ifdef _DEBUG_PRINT
    printf("--> Finish(done):%s\n", pKernel->GetKernelName());
#endif
#ifdef _DEBUG
    m_lFinish.exchange(0);
#endif

    if ( NULL != m_pBinary )
    {
        m_pBinary->Release();
    }
    if ( NULL != m_pMemBuffSizes )
    {
        delete []m_pMemBuffSizes;
    }

#if defined(USE_ITT)
    if ((NULL != m_pGPAData) && (m_pGPAData->bUseGPA))
    {
        __itt_task_end(m_pGPAData->pDeviceDomain);
    }
#endif // ITT

	return true;
}


void* NDRange::AttachToThread(void* pWgContextBase, size_t uiNumberOfWorkGroups, size_t firstWGID[], size_t lastWGID[])
{
#ifdef _DEBUG_PRINT
    printf("AttachToThread %d, WrkId(%d), CmdId(%d)\n", (int)GetCurrentThreadId(), (int)uiWorkerId, (int)m_pCmd->id);
#endif

#ifdef _DEBUG
    long lVal = m_lFinish.test_and_set(0, 0);
    if ( lVal == 1 )
    {
        assert(0);
    }
    ++m_lAttaching ;
#endif
	
    if ( NULL == pWgContextBase )
    {
        CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Failed to retrive WG context"));
        m_lastError = (cl_int)CL_DEV_ERROR_FAIL;
#ifdef _DEBUG
        --m_lAttaching ;
#endif	
        return NULL;
    }

    WGContext* pCtx = static_cast<WGContext*>(reinterpret_cast<WGContextBase*>(pWgContextBase));
    pCtx->SetCurrentNDRange(this);
    sm_pCurrentWgContext = pCtx;
    if (m_lNDRangeId != pCtx->GetNDRCmdId() )
    {
        cl_dev_err_code ret = pCtx->CreateContext(m_lNDRangeId, m_pBinary, m_pMemBuffSizes, m_MemBuffCount);
        if ( CL_DEV_FAILED(ret) )
        {
            CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Failed to create new execution context, Id:%d, ERR:%x"), pCtx->GetThreadId(), ret);
			      m_lastError = (int)ret;
#ifdef _DEBUG
            --m_lAttaching ;
#endif
            return NULL;
        }
    }

    if (NULL == pCtx->GetExecutable())
    {
        return NULL;
    }
    pCtx->GetExecutable()->PrepareThread();

#ifdef USE_ITT
    // Start execution task
    if ((NULL != m_pGPAData) && (m_pGPAData->bUseGPA))
    {
        unsigned int uiWorkGroupSize = 1;
        const size_t*	pWGSize = m_pBinary->GetWorkGroupSize();
        cl_dev_cmd_param_kernel *cmdParams = (cl_dev_cmd_param_kernel*)m_pCmd->params;

        __itt_task_begin(m_pGPAData->pDeviceDomain, m_ittID, __itt_null,
		        ((const ProgramService::KernelMapEntry*)cmdParams->kernel)->ittTaskNameHandle);

        for (unsigned int i=0 ; i<cmdParams->work_dim ; ++i)
        {
            uiWorkGroupSize *= (unsigned int)pWGSize[i];
        }
		
        __itt_metadata_add(m_pGPAData->pDeviceDomain, m_ittID, m_pGPAData->pWorkGroupSizeHandle, __itt_metadata_u32 , 1, &uiWorkGroupSize);

        cl_ulong numOfWGs64 = uiNumberOfWorkGroups;
        __itt_metadata_add(m_pGPAData->pDeviceDomain, m_ittID, m_pGPAData->pNumberOfWorkGroupsHandle, __itt_metadata_u64 , 1, &numOfWGs64);

        // Make sure all values are 64 bit.
        cl_ulong firstWGID64[MAX_WORK_DIM];
        cl_ulong lastWGID64[MAX_WORK_DIM];
        for (int i=0 ; i < MAX_WORK_DIM ; ++i)
        {
            firstWGID64[i] = firstWGID[i];
            lastWGID64[i]  = lastWGID[i];
        }

        // Do not use string metadata, it is VERY slow.
        __itt_metadata_add(m_pGPAData->pDeviceDomain, m_ittID, m_pGPAData->pStartPos, __itt_metadata_u64 , cmdParams->work_dim, firstWGID64);
        __itt_metadata_add(m_pGPAData->pDeviceDomain, m_ittID, m_pGPAData->pEndPos, __itt_metadata_u64 , cmdParams->work_dim, lastWGID64);
    }
#endif // ITT

#ifdef _DEBUG
    -- m_lAttaching;
#endif

    return pCtx;
}

void NDRange::DetachFromThread(void* pWgContext)
{
	// End execution task
#if defined(USE_ITT)
    if ((NULL != m_pGPAData) && (m_pGPAData->bUseGPA))
    {
        __itt_task_end(m_pGPAData->pDeviceDomain);
    }
#endif // ITT
    
    WGContext* const pCtx = reinterpret_cast<WGContext*>(pWgContext);
    assert(NULL != pCtx);
    if (NULL == pCtx->GetExecutable())
    {
        m_lastError = (cl_int)CL_DEV_ERROR_FAIL;
        return;
    }

    pCtx->GetExecutable()->RestoreThreadState();
    WgFinishedExecution();
    pCtx->SetCurrentNDRange(NULL);
    sm_pCurrentWgContext = NULL;
}

bool NDRange::ExecuteIteration(size_t x, size_t y, size_t z, void* pWgCtx)
{
#ifdef _DEBUG
	long lVal = m_lFinish.test_and_set(0, 0);
	assert(lVal == 0);
	++ m_lExecuting;
#endif

	assert(NULL != pWgCtx);
    WGContext* pWgContext = reinterpret_cast<WGContext*>(pWgCtx);
    ICLDevBackendExecutable_* pExec = pWgContext->GetExecutable();
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
#ifndef _WIN32	//Don't support this feature on Windows at the moment   
				//Optionally override the iteration to be executed if an affinity permutation is defined
	if (m_bEnablePredictablePartitioning)
	{
        assert((0 == y) && (0 == z));
	    unsigned int myGroupID = pWgContext->GetThreadId();
	    if (0 == m_bWGExecuted.bitTestAndSet(myGroupID))
	    {
		groupId[0] = myGroupID;
	    }
	    else // find an available work group
	    {
		for (unsigned int ui = 0; ui < m_numThreads; ++ui)
		{
	            if (0 == m_bWGExecuted.bitTestAndSet(ui))
		    {
		        groupId[0] = ui;
			break;
		    }
		}
	    }
        }
#endif
	if (NULL == pExec)
	{
		#ifdef _DEBUG
			-- m_lExecuting;
		#endif
		return true;
	}	
	pExec->Execute(groupId, NULL, NULL);
#ifdef _DEBUG
	-- m_lExecuting;
#endif
    return true;

}

///////////////////////////////////////////////////////////////////////////
// OCL Fill buffer/image execution

cl_dev_err_code FillMemObject::Create(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, SharedPtr<ITaskBase>* pTask, const SharedPtr<ITaskList>& pList)
{
	FillMemObject* pCommand = new FillMemObject(pTD, pCmd);
	if (NULL == pCommand)
	{
		return CL_DEV_OUT_OF_MEMORY;
	}
#ifdef _DEBUG
	cl_dev_err_code rc;
	rc = pCommand->CheckCommandParams(pCmd);
	assert(CL_DEV_SUCCESS == rc);
#endif

	assert(pTask);
	*pTask = static_cast<ITaskBase*>(pCommand);

	return CL_DEV_SUCCESS;
}


FillMemObject::FillMemObject(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd) :
	CommandBaseClass<ITask>(pTD, pCmd)
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
	const cl_dev_cmd_param_fill* cmdParams = (cl_dev_cmd_param_fill*)m_pCmd->params;
	cl_mem_obj_descriptor*  pMemObj;

	NotifyCommandStatusChanged(m_pCmd, CL_RUNNING, CL_DEV_SUCCESS);

	cmdParams->memObj->clDevMemObjGetDescriptor(CL_DEVICE_TYPE_CPU, 0, (cl_dev_memobj_handle*)&pMemObj);

#ifdef _DEBUG_PRINT
	printf("--> FillMemObject(start), cmdid:%p\n", m_pCmd->id);
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
	const size_t width =  cmdParams->region[0] * pMemObj->uiElementSize;

	// prepare copy buffer:
	char* fillBuf = (char*)malloc(width);
	if (NULL == fillBuf) return false;

	CopyPattern(cmdParams->pattern, cmdParams->pattern_size, fillBuf, width);

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
	printf("--> FillMemObject(end), cmdid:%p(%d)\n", m_pCmd->id, CL_DEV_SUCCESS);
#endif
	NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, CL_DEV_SUCCESS);
	return true;
}

///////////////////////////////////////////////////////////////////////////
// OCL Migrate buffer/image execution

cl_dev_err_code MigrateMemObject::Create(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, SharedPtr<ITaskBase>* pTask, const SharedPtr<ITaskList>& pList)
{
	MigrateMemObject* pCommand = new MigrateMemObject(pTD, pCmd);
	if (NULL == pCommand)
	{
		return CL_DEV_OUT_OF_MEMORY;
	}
#ifdef _DEBUG
	cl_dev_err_code rc;
	rc = pCommand->CheckCommandParams(pCmd);
	assert(CL_DEV_SUCCESS == rc);
#endif

	assert(pTask);
	*pTask = static_cast<ITaskBase*>(pCommand);

	return CL_DEV_SUCCESS;
}

MigrateMemObject::MigrateMemObject(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd) :
	CommandBaseClass<ITask>(pTD, pCmd)
{
}

cl_dev_err_code MigrateMemObject::CheckCommandParams(cl_dev_cmd_desc* cmd)
{
	if ( sizeof(cl_dev_cmd_param_migrate) != cmd->param_size )
	{
		return CL_DEV_INVALID_COMMAND_PARAM;
	}

	cl_dev_cmd_param_migrate *cmdParams = (cl_dev_cmd_param_migrate*)(cmd->params);

    if (0 == cmdParams->mem_num)
    {
        return CL_DEV_INVALID_VALUE;
    }

	for (unsigned int i=0; i< cmdParams->mem_num; ++i)
	{
		if(NULL == cmdParams->memObjs[i])
		{
			return CL_DEV_INVALID_VALUE;
		}
	}

    if (0 != (cmdParams->flags & ~(CL_MIGRATE_MEM_OBJECT_HOST | CL_MIGRATE_MEM_OBJECT_CONTENT_UNDEFINED)))
    {
        return CL_DEV_INVALID_VALUE;
    }
    
	return CL_DEV_SUCCESS;
}

bool MigrateMemObject::Execute()
{
	//cl_dev_cmd_param_migrate* cmdParams = (cl_dev_cmd_param_migrate*)m_pCmd->params;

	NotifyCommandStatusChanged(m_pCmd, CL_RUNNING, CL_DEV_SUCCESS);

    // TODO: Numa implementation

	NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, CL_DEV_SUCCESS);
	return true;
}

// DeviceNDRange:

AtomicCounter DeviceNDRange::sm_cmdIdCnt;

cl_dev_cmd_desc* DeviceNDRange::InitCmdDesc(cl_dev_cmd_param_kernel& paramKernel, cl_dev_cmd_desc& cmdDesc, const Intel::OpenCL::DeviceBackend::ICLDevBackendKernel_* pKernel,
	const void* pContext, size_t szContextSize, const cl_work_description_type* pNdrange, const SharedPtr<ITaskList>& pList)
{
	paramKernel.kernel = pKernel;
	paramKernel.work_dim = pNdrange->workDimension;
	for (cl_uint i = 0; i < paramKernel.work_dim; i++)
	{
		paramKernel.glb_wrk_offs[i] = pNdrange->globalWorkOffset[i];
		paramKernel.glb_wrk_size[i] = pNdrange->globalWorkSize[i];
		paramKernel.lcl_wrk_size[i] = pNdrange->localWorkSize[i];
	}
	char* const pAllocatedContext = new char[szContextSize];
	paramKernel.arg_values = pAllocatedContext;
	paramKernel.arg_size = szContextSize;
	MEMCPY_S(pAllocatedContext, szContextSize, pContext, szContextSize);
	
	cmdDesc.type = CL_DEV_CMD_EXEC_KERNEL;
	cmdDesc.id = (cl_dev_cmd_id)(DeviceNDRange::GetNextCmdId() | (1L << (sizeof(long) * 8 - 1)));	// device NDRange IDs have their MSB set, while in host NDRange IDs they're reset
	cmdDesc.data = cmdDesc.device_agent_data = NULL;
	cmdDesc.profiling = pList->IsProfilingEnabled();
	cmdDesc.params = &paramKernel;
	cmdDesc.param_size = sizeof(paramKernel);
	return &cmdDesc;
}

int	DeviceNDRange::Init(size_t region[], unsigned int &regCount)
{
	const int res = NDRange::Init(region, regCount);
	StartExecutionProfiling();
	return res;
}

bool DeviceNDRange::Finish(FINISH_REASON reason)
{
	StopExecutionProfiling();
	const bool bRes = NDRange::Finish(reason);
	// IncRefCnt() was called in CommandBaseClass::CommandBaseClass, but since CPUDevice::clDevReleaseCommand are not called for device-side commands, we decrement the reference counter here
	DecRefCnt();
	return bRes;
}

#ifdef __INCLUDE_MKL__
/////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
cl_dev_err_code NativeKernelTask::Create(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, SharedPtr<ITaskBase>* pTask)
{
	NativeKernelTask* pCommand = new NativeKernelTask(pTD, pCmd);
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

	assert(pTask);
	*pTask = static_cast<ITaskBase*>(pCommand);

#if defined (USE_ITT)
    if ((NULL != pCommand->m_pGPAData) && (pCommand->m_pGPAData->bUseGPA))
    {
	    cl_dev_cmd_param_kernel *cmdParams = (cl_dev_cmd_param_kernel*)pCommand->m_pCmd->params;
	    const ICLDevBackendKernel_* pKernel = (ICLDevBackendKernel_*)cmdParams->kernel;

	    char  strTaskName[ITT_TASK_NAME_LEN];
	    SPRINTF_S(strTaskName, ITT_TASK_NAME_LEN, "%s(%lu)", pKernel->GetKernelName(), (cl_ulong)pCommand->m_pCmd->id);
	    pCommand->m_pTaskNameHandle = __itt_string_handle_create(strTaskName);
    }
#endif

	return CL_DEV_SUCCESS;
}

NativeKernelTask::NativeKernelTask(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd) :
	CommandBaseClass<ITask>(pTD, pCmd)
{
	// First to check if the required NDRange is one of the built-in kernels
	ICLDevBackendKernel_* pKernel = (ICLDevBackendKernel_*)(((cl_dev_cmd_param_kernel*)pCmd->params)->kernel);
	
	m_pBIKernel = dynamic_cast<IBuiltInKernel*>(pKernel);
}

cl_dev_err_code NativeKernelTask::CheckCommandParams(cl_dev_cmd_desc* cmd)
{
	return CL_DEV_SUCCESS;
}

bool NativeKernelTask::Execute()
{
	NotifyCommandStatusChanged(m_pCmd, CL_RUNNING, CL_DEV_SUCCESS);

#if defined(USE_ITT)
	// Start execution task
	if ((NULL != m_pGPAData) && (m_pGPAData->bUseGPA))
	{
		#if defined(USE_GPA)
		__itt_set_track(NULL);
		#endif

		__itt_task_begin(m_pGPAData->pDeviceDomain, m_ittID, __itt_null, m_pTaskNameHandle);
	}
#endif

	void* pParamBuffer = STACK_ALLOC(m_pBIKernel->GetParamSize());
	if ( NULL == pParamBuffer )
	{
		assert(0 && "alloca always success");
		NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, (int)CL_DEV_OUT_OF_MEMORY);
		return false;
	}

	cl_dev_err_code err = ExtractNDRangeParams(pParamBuffer);
	if ( CL_DEV_FAILED(err) )
	{
		STACK_FREE(pParamBuffer);
		NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, err);
		return false;
	}

	cl_dev_err_code res = m_pBIKernel->Execute((cl_dev_cmd_param_kernel*)m_pCmd->params, pParamBuffer, m_pTaskDispatcher->getOmpExecutionThread());

#if defined(USE_ITT)
	if ((NULL != m_pGPAData) && (m_pGPAData->bUseGPA))
	{
#if defined(USE_GPA)
		__itt_set_track(NULL);
#endif
		__itt_task_end(m_pGPAData->pDeviceDomain);
	}
#endif // ITT

	NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, res);
	STACK_FREE(pParamBuffer);

	// Return success even if BuiltIn kernel execution failed
	return true;
}
#endif
