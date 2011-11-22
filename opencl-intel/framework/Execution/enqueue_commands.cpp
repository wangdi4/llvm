// Copyright (c) 2008-2009 Intel Corporation
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

///////////////////////////////////////////////////////////
//  enqueue_commands.cpp
//  Implementation of the Class ReadBufferCommand
//  Created on:      16-Dec-2008 10:11:31 AM
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////
#include "enqueue_commands.h"
#include "ocl_event.h"
#include "command_queue.h"
#include "kernel.h"
#include "sampler.h"
#include "events_manager.h"
#include "cl_sys_defines.h"
#include "MemoryAllocator/MemoryObject.h"
#include <Logger.h>

//For debug
#include <stdio.h>
#if defined (_WIN32)
#include <windows.h>
#include <process.h>
#else
#include <malloc.h>
#endif
#include <assert.h>
#include "execution_module.h"
#include "ocl_itt.h"


using namespace Intel::OpenCL::Framework;

using namespace Intel::OpenCL::Utils;

/******************************************************************
 * Static function to be used by all commands that need to write/read data
 ******************************************************************/
static void create_dev_cmd_rw(
    cl_uint				uiDimCount,
    void*               pPtr,
    const size_t*       pszMemObjOrigin,
	const size_t*       pszPtrOrigin,
    const size_t*       pszRegion,
    size_t              szPtrRowPitch,
    size_t              szPtrSlicePitch,
    size_t              szMemObjRowPitch,
    size_t              szMemObjSlicePitch,
    cl_dev_cmd_type     clCmdType,
    cl_dev_cmd_id       clCmdId,
	cl_dev_cmd_desc*     pDevCmd,
	cl_dev_cmd_param_rw* pRWParams
    )
{
        // Create Read command
        pRWParams->ptr = pPtr;

        cl_uint i;
        for( i=0; i<MAX_WORK_DIM; i++ )
        {
            pRWParams->origin[i] = pszMemObjOrigin[i];
            pRWParams->region[i] = pszRegion[i];

			if (pszPtrOrigin)
			{
				pRWParams->ptr_origin[i] = pszPtrOrigin[i];
			}
			else
			{
				pRWParams->ptr_origin[i] = 0;
			}

        }

        pRWParams->pitch[0] = szPtrRowPitch;
        pRWParams->pitch[1] = szPtrSlicePitch;

		pRWParams->memobj_pitch[0] = szMemObjRowPitch;
        pRWParams->memobj_pitch[1] = szMemObjSlicePitch;

        pRWParams->dim_count = uiDimCount;

        pDevCmd->type = clCmdType;
        pDevCmd->id = clCmdId;
        pDevCmd->params = pRWParams;
        pDevCmd->param_size = sizeof(cl_dev_cmd_param_rw);
}

/******************************************************************
 *
 ******************************************************************/
Command::Command( IOclCommandQueueBase* cmdQueue, ocl_entry_points * pOclEntryPoints ):
    OCLObjectBase("Command"),
    m_Event(cmdQueue, pOclEntryPoints),
    m_clDevCmdListId(0),
	m_pDevice(NULL),
	m_pCommandQueue(cmdQueue),
	m_returnCode(CL_INVALID_OPERATION)
{
	memset(&m_DevCmd, 0, sizeof(cl_dev_cmd_desc));

	m_iId = m_Event.GetId();
	m_Event.AddPendency(NULL);  // NULL because dependency is released in case of failure by IOclCommandQueueBase
	m_Event.SetCommand(this);

	assert(m_pCommandQueue);
	m_pCommandQueue->AddPendency(this);
	m_pDevice = m_pCommandQueue->GetDefaultDevice();

    m_pGpaCommand = NULL;
	INIT_LOGGER_CLIENT(TEXT("Command Logger Client"),LL_DEBUG);
}

/******************************************************************
 *
 ******************************************************************/
Command::~Command()
{
	m_pDevice = NULL;
	GPA_DestroyCommand();
	m_pCommandQueue->RemovePendency(this);
	m_pCommandQueue = NULL;

	RELEASE_LOGGER_CLIENT;
}

/******************************************************************
 *
 ******************************************************************/
//Todo: remove clCmdId param
cl_err_code Command::NotifyCmdStatusChanged(cl_dev_cmd_id clCmdId, cl_int iCmdStatus, cl_int iCompletionResult, cl_ulong ulTimer)
{
#if defined(USE_GPA)
	//char pMarkerString[64];
	const char* pCommandName;
	ocl_gpa_data* pGPAData = m_pCommandQueue->GetGPAData();
#endif
    cl_err_code res = CL_SUCCESS;
    switch(iCmdStatus)
    {
    case CL_QUEUED:
		// Nothing to do, not expected to be here at all
		break;
    case CL_SUBMITTED:
		// Nothing to do, not expected to be here at all
		m_Event.SetProfilingInfo(CL_PROFILING_COMMAND_SUBMIT, ulTimer);
        m_Event.SetColor(EVENT_STATE_LIME);
        LogDebugA("Command - SUBMITTED TO DEVICE  : %s (Id: %d)", GetCommandName(), m_iId);
        break;
    case CL_RUNNING:
        LogDebugA("Command - RUNNING  : %s (Id: %d)", GetCommandName(), m_iId);
		// Running marker
#if defined(USE_GPA)
		if ((NULL != pGPAData) && (pGPAData->bUseGPA))
		{
			if (pGPAData->cStatusMarkerFlags & GPA_SHOW_RUNNING_MARKER)
			{
				// Write this data to the thread track
				__itt_set_track(NULL);

				char pMarkerString[64] = "Running - ";
				pCommandName = this->GetCommandName();
				strcat_s(pMarkerString, 64,pCommandName);

				__itt_string_handle* pMarker = __itt_string_handle_createA(pMarkerString);

				//Due to a bug in GPA 4.0 the marker is within a task
				//Should be removed in GPA 4.1
				__itt_task_begin(pGPAData->pDeviceDomain, __itt_null, __itt_null, pGPAData->pMarkerHandle);
				
				__itt_marker(pGPAData->pDeviceDomain, __itt_null, pMarker, __itt_marker_scope_global);

				__itt_task_end(pGPAData->pDeviceDomain);
			}

			if ((pGPAData->bEnableContextTracing) && (NULL != m_pGpaCommand))
			{
				// Set custom track 
				__itt_set_track(m_pCommandQueue->GPA_GetQueue()->m_pTrack);

				__ittx_task_set_state(pGPAData->pContextDomain, m_pGpaCommand->m_CmdId, pGPAData->pRunningTaskState);
				
				// This feature does not work now due to GPA bug, should be added in later stages.
				//GPA_WriteCommandMetadata();
			}

		}  
#endif
		m_Event.SetProfilingInfo(CL_PROFILING_COMMAND_START, ulTimer);
        m_Event.SetColor(EVENT_STATE_GREEN);
        break;
    case CL_COMPLETE:
		assert(EVENT_STATE_BLACK != m_Event.GetColor());
		m_Event.SetProfilingInfo(CL_PROFILING_COMMAND_END, ulTimer);
	    // Complete command,
		// do that before set event, since side effect of SetEvent(black) may be deleting of this instance.
		// Is error
		if (CL_FAILED(iCompletionResult))
		{
			LogErrorA("Command - DONE - Failure  : %s (Id: %d)", GetCommandName(), m_iId);
			//assert(0 && "Command - DONE - Failure");
		}
		else
		{
			LogDebugA("Command - DONE - SUCCESS : %s (Id: %d)", GetCommandName(), m_iId);
		}
		m_returnCode = iCompletionResult;
        res = CommandDone();
        m_Event.SetColor(EVENT_STATE_BLACK);
// Complete marker
#if defined(USE_GPA)
		if ((NULL != pGPAData) && (pGPAData->bUseGPA))
		{
			if (pGPAData->cStatusMarkerFlags & GPA_SHOW_COMPLETED_MARKER)
			{
				// Write this data to the thread track
				__itt_set_track(NULL);

				char pMarkerString[64] = "Completed - ";
				pCommandName = this->GetCommandName();
				strcat_s(pMarkerString, 64,pCommandName);

				__itt_string_handle* pMarker = __itt_string_handle_createA(pMarkerString);

				//Due to a bug in GPA 4.0 the marker is within a task
				//Should be removed in GPA 4.1
				__itt_task_begin(pGPAData->pDeviceDomain, __itt_null, __itt_null, pMarker);
				
				__itt_marker(pGPAData->pDeviceDomain, __itt_null, pMarker, __itt_marker_scope_global);

				__itt_task_end(pGPAData->pDeviceDomain);
			}

			// Complete the running task on the context view and destroy the id
			if ((pGPAData->bEnableContextTracing) && (NULL != m_pGpaCommand))
			{
				// Set custom track 
				__itt_set_track(m_pCommandQueue->GPA_GetQueue()->m_pTrack);

				// Complete the running task on the context view and destroy the id
				__itt_task_end_overlapped(pGPAData->pContextDomain, m_pGpaCommand->m_CmdId);
				__itt_id_destroy(pGPAData->pContextDomain,m_pGpaCommand->m_CmdId);
			}
		}

#endif
		m_Event.RemovePendency(NULL);
		break;
    default:
        break;
    }
    return res;
}

cl_err_code	Command::GetMemObjectDescriptor(MemoryObject* pMemObj, IOCLDevMemoryObject* *ppMemObj)
{
	OclEvent* pObjEvent;
	cl_err_code res = pMemObj->GetDeviceDescriptor(m_pDevice, ppMemObj, &pObjEvent);
	if ( CL_FAILED(res) )
	{
		return res;
	}

	// No Event was created, we have the descriptor
	if ( CL_NOT_READY == res )
	{
		assert( NULL != pObjEvent);
		m_Event.AddDependentOn(pObjEvent);
	}

	return CL_SUCCESS;
}

void Command::GPA_InitCommand()
{
#if defined (USE_GPA)
	ocl_gpa_data* pGPAData = m_pCommandQueue->GetGPAData();
	if ((NULL != pGPAData) && (pGPAData->bUseGPA) && (pGPAData->bEnableContextTracing))
	{
		m_pGpaCommand = NULL;
		m_pGpaCommand = new ocl_gpa_command();
		if (NULL != m_pGpaCommand)
		{
			// Create task name strings
			const char* commandName = GPA_GetCommandName();
			if (NULL != commandName)
			{
				m_pGpaCommand->m_strCmdName = __itt_string_handle_createA(commandName);
			}
		}
	}
#endif
}

void Command::GPA_DestroyCommand()
{
#if defined (USE_GPA)
	ocl_gpa_data* pGPAData = m_pCommandQueue->GetGPAData();
	if ((NULL != pGPAData) && (pGPAData->bUseGPA) && (pGPAData->bEnableContextTracing))
	{
		delete m_pGpaCommand;
	}
#endif
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code CopyToHostCommand::CommandDone()
{
	// Set location to HOST
	cl_err_code err = m_pMemObj->UpdateLocation(NULL);
	if ( CL_FAILED(err))
	{
		ReadMemObjCommand::CommandDone();
		return err;
	}
	return ReadMemObjCommand::CommandDone();
}

cl_err_code MemoryCommand::CopyToHost(
						MemoryObject*	pSrcMemObj,
						QueueEvent**		pEvent)
{
	cl_err_code res = CL_SUCCESS;

	FissionableDevice* pSrcDevice = pSrcMemObj->GetLocation();
	assert(pSrcDevice != NULL); // Check we got here with NULL
	if ( NULL == pSrcDevice ) // The buffer is already on HOST
	{
		return CL_SUCCESS;
	}

	void* pData = pSrcMemObj->GetBackingStoreData(NULL);

	size_t origin[MAX_WORK_DIM] = {0};
	size_t region[MAX_WORK_DIM];
	size_t rowPitch, slicePitch;
	pSrcMemObj->GetLayout(region, &rowPitch, &slicePitch);

	MemoryCommand* pReadMemObjCmd = new ReadMemObjCommand(m_pCommandQueue, (ocl_entry_points*)((_cl_command_queue_int*)m_pCommandQueue->GetHandle())->dispatch, pSrcMemObj, origin, region, rowPitch, slicePitch, pData);
	if (!pReadMemObjCmd)
	{
		return CL_OUT_OF_HOST_MEMORY;
	}

	QueueEvent* pQueueEvent = pReadMemObjCmd->GetEvent();
	if (!pQueueEvent)
	{
		delete pReadMemObjCmd;
		return CL_OUT_OF_HOST_MEMORY;
	}

	res = pReadMemObjCmd->Init();
	if (CL_FAILED(res))
	{
		delete pReadMemObjCmd;
		return res;
	}

	cl_dev_cmd_desc* pReadDevCmd =  &pReadMemObjCmd->m_DevCmd;
	cl_dev_cmd_param_rw* pRWParams =  &pReadMemObjCmd->m_rwParams;

	OclEvent* pObjEvent;
	res = pSrcMemObj->GetDeviceDescriptor(pSrcDevice, &pRWParams->memObj, &pObjEvent);
	if (CL_FAILED(res))
	{
		delete pReadMemObjCmd;
		return res;
	}

	// copy from device to host
	create_dev_cmd_rw(
		m_commandType == CL_COMMAND_READ_BUFFER_RECT ? MAX_WORK_DIM  : pSrcMemObj->GetNumDimensions(),
		pData, origin, NULL, region, 0, 0, 0, 0,
		CL_DEV_CMD_READ,
		(cl_dev_cmd_id)pQueueEvent->GetId(),
		pReadDevCmd,
		pRWParams);

	pReadDevCmd->data = static_cast<ICmdStatusChangedObserver*>(pReadMemObjCmd);
	res = pSrcDevice->GetDeviceAgent()->clDevCommandListExecute(NULL, &pReadDevCmd, 1);
	if (CL_FAILED(res))
	{
		return res;
	}

	*pEvent = pQueueEvent;
	return res;
}

cl_err_code MemoryCommand::CopyFromHost(
						void* pSrcData,
						MemoryObject* pDstMemObj,
						const size_t* pSrcOrigin,
						const size_t* pDstOrigin,
						const size_t* pRegion,
						const size_t  szSrcRowPitch,
						const size_t  szSrcSlicePitch,
						const size_t  szDstRowPitch,
						const size_t  szDstSlicePitch,
						QueueEvent**  pEvent)
{
	cl_err_code res = CL_SUCCESS;



	if (*pEvent != NULL)
	{
		OclEvent* pObjEvent;
		res = pDstMemObj->GetDeviceDescriptor(m_pDevice, &m_rwParams.memObj, &pObjEvent);
		if (CL_FAILED(res))
		{
			return res;
		}

		cl_dev_cmd_desc *pDevCmd = &m_DevCmd;
		create_dev_cmd_rw(
				m_commandType == CL_COMMAND_WRITE_BUFFER_RECT ? MAX_WORK_DIM  : pDstMemObj->GetNumDimensions(),
				(void*)pSrcData, pDstOrigin, pSrcOrigin, pRegion, szSrcRowPitch, szSrcSlicePitch, szDstRowPitch, szDstSlicePitch,
				CL_DEV_CMD_WRITE,
				(cl_dev_cmd_id)(*pEvent)->GetId(),
				pDevCmd,
				&m_rwParams) ;

		LogDebugA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), m_iId);
		// Sending 1 command to the device where the buffer is located now
		m_Event.SetEventQueue(m_pCommandQueue);
		pDevCmd->profiling = (m_pCommandQueue->IsProfilingEnabled() ? true : false );
		pDevCmd->data			= static_cast<ICmdStatusChangedObserver*>(this);
		res = m_pDevice->GetDeviceAgent()->clDevCommandListExecute(m_clDevCmdListId, &pDevCmd, 1);
	}
	else
	{
		MemoryCommand* pWriteMemObjCmd = new WriteMemObjCommand(m_pCommandQueue, (ocl_entry_points*)(((_cl_command_queue_int*)m_pCommandQueue->GetHandle())->dispatch),
			false, pDstMemObj, pDstOrigin, pRegion, szDstRowPitch, szDstSlicePitch, pSrcData, pSrcOrigin, szSrcRowPitch, szSrcSlicePitch);
		if (!pWriteMemObjCmd)
		{
			return CL_OUT_OF_HOST_MEMORY;
		}
		cl_event waitEvent = NULL;
		QueueEvent* pQueueEvent = pWriteMemObjCmd->GetEvent();
		m_pCommandQueue->GetEventsManager()->RegisterQueueEvent(pQueueEvent, &waitEvent);
		res = pWriteMemObjCmd->Init();
		if (CL_FAILED(res))
		{
			delete pWriteMemObjCmd;
			return res;
		}

		cl_dev_cmd_desc* pWriteDevCmd =  &pWriteMemObjCmd->m_DevCmd;
		cl_dev_cmd_param_rw* pRWParams = &pWriteMemObjCmd->m_rwParams;

		OclEvent* pObjEvent;
		res = pDstMemObj->GetDeviceDescriptor(m_pDevice, &pRWParams->memObj, &pObjEvent);
		if (CL_FAILED(res))
		{
			delete pWriteMemObjCmd;
			return res;
		}

		// copy from host to device
		create_dev_cmd_rw(
			m_commandType == CL_COMMAND_WRITE_BUFFER_RECT ? MAX_WORK_DIM  : pDstMemObj->GetNumDimensions(),
			(void*)pSrcData, pDstOrigin, pSrcOrigin, pRegion, szSrcRowPitch, szSrcSlicePitch, szDstRowPitch, szDstSlicePitch,
			CL_DEV_CMD_WRITE,
			(cl_dev_cmd_id)pQueueEvent->GetId(),
			pWriteDevCmd,
			pRWParams);

		// Set new location
		pDstMemObj->UpdateLocation(m_pDevice);

		pWriteDevCmd->data = static_cast<ICmdStatusChangedObserver*>(pWriteMemObjCmd);
		res = m_pDevice->GetDeviceAgent()->clDevCommandListExecute(NULL, &pWriteDevCmd, 1);
		if (CL_FAILED(res)) { return res; }

		*pEvent = pQueueEvent;
	}
	return res;
}

#ifdef USE_PREPARE_ON_DEVICE
cl_err_code MemoryCommand::PrepareOnDevice(
						MemoryObject* pSrcMemObj,
						const size_t* pSrcOrigin,
						const size_t* pRegion,
						QueueEvent**	pEvent)
{
	cl_err_code res = CL_SUCCESS;


	Device* pQueueDevice= m_pCommandQueue->GetQueueDeviceHandle();
	Device* srcDeviceId = pSrcMemObj->GetLocation();

	if (srcDeviceId != 0 && srcDeviceId != queueDeviceId)
	{
		res = CopyToHost(pSrcMemObj, pEvent);
	}
	else if (srcDeviceId == 0 && pSrcMemObj->IsAllocated(0))
	{
		void* pSrcData = pSrcMemObj->GetBackingStoreData(NULL);

		size_t origin[MAX_WORK_DIM] = {0};
		size_t region[MAX_WORK_DIM];
		size_t rowPitch, slicePitch;
		pSrcMemObj->GetLayout(region, &rowPitch, &slicePitch);

		res = CopyFromHost(pSrcData, pSrcMemObj, origin, origin, region, rowPitch, slicePitch, 0, 0, pEvent);
	}
	return res;
}
#endif

// in case of an image array we need to substitute the MemoryObject representing the array with the indexed 2D image.
static MemoryObject* GetImageFromArray(MemoryObject* memObj, size_t index)
{
    if (CL_MEM_OBJECT_IMAGE2D_ARRAY == memObj->GetType())
    {
        return dynamic_cast<IMemoryObjectArray*>(memObj)->GetMemObject(index);
    }

	return memObj;
}

CopyMemObjCommand::CopyMemObjCommand(
				  IOclCommandQueueBase* cmdQueue,
				  ocl_entry_points *    pOclEntryPoints,
                        MemoryObject*   pSrcMemObj,
                        MemoryObject*   pDstMemObj,
                        const size_t*   szSrcOrigin,
                        const size_t*   szDstOrigin,
                        const size_t*   szRegion,
						const size_t	szSrcRowPitch	= 0,
						const size_t	szSrcSlicePitch = 0,
						const size_t	szDstRowPitch	= 0,
						const size_t	szDstSlicePitch	= 0):
    MemoryCommand(cmdQueue, pOclEntryPoints),
	m_szSrcRowPitch(szSrcRowPitch),
	m_szSrcSlicePitch(szSrcSlicePitch),
	m_szDstRowPitch(szDstRowPitch),
	m_szDstSlicePitch(szDstSlicePitch)
{
    for( int i=0; i<MAX_WORK_DIM; i++ )
    {
        m_szSrcOrigin[i] = szSrcOrigin[i];
    }

    for( int i=0; i<MAX_WORK_DIM; i++ )
    {
		m_szDstOrigin[i] = szDstOrigin[i];
    }

    for( int i=0; i<MAX_WORK_DIM; i++ )
    {
		m_szRegion[i] = szRegion[i];
    }

	m_pSrcMemObj = GetImageFromArray(pSrcMemObj, m_szSrcOrigin[2]);
    m_pDstMemObj = GetImageFromArray(pDstMemObj, m_szDstOrigin[2]);

	m_uiSrcNumDims = m_pSrcMemObj->GetNumDimensions();
	m_uiDstNumDims = m_pDstMemObj->GetNumDimensions();

}

/******************************************************************
 *
 ******************************************************************/
CopyMemObjCommand::~CopyMemObjCommand()
{
}

/******************************************************************
 * Just mark the memory object as use. The actual copy is determined on execution.
 ******************************************************************/
cl_err_code CopyMemObjCommand::Init()
{
    cl_err_code res = m_pDstMemObj->CreateDeviceResource(m_pDevice);
    if( CL_FAILED(res))
    {
        return res;
    }

	// Initialize GPA data
	GPA_InitCommand();

    m_pSrcMemObj->AddPendency(this);
    m_pDstMemObj->AddPendency(this);
    return CL_SUCCESS;
}

/******************************************************************
 * Copy memory object asks device to perform copy only if both objects are on
 * the same device. Else, it read from 1 device and write to other.
 * Either ways, the location of the destination data remain the same, unless
 * it was never allocated before.
 *
 ******************************************************************/
cl_err_code CopyMemObjCommand::Execute()
{
	cl_err_code res = CL_SUCCESS;

	// Check if this command has become MARKER because of returning CL_DONE_ON_RUNTIME previously.
	// if Yes; then this is the second time Execute() is being called and we don't to do anything,
	// we already executed before.
	if (m_commandType == CL_COMMAND_MARKER)
	{
		m_returnCode = CL_SUCCESS;
		m_Event.SetColor(EVENT_STATE_BLACK);
		m_Event.RemovePendency(NULL);
		return CL_SUCCESS;
	}

	/// first, make sure m_pDstMemObj resides on target device.
	/// for example, if m_pDstMemObj resides on different device this funciton will now copy
	/// the memory object to host, then called again to copy from host to the target device.
	/// notice we are returning CL_NOT_READY in cas extra operation is required.
	QueueEvent* pDepEvent = NULL;
#ifdef USE_PREPARE_ON_DEVICE

	/// first, make sure m_pDstMemObj resides on target device.
	/// for example, if m_pDstMemObj resides on different device this funciton will now copy
	/// the memory object to host, then called again to copy from host to the target device.
	/// notice we are returning CL_NOT_READY in cas extra operation is required.

	PrepareOnDevice(m_pDstMemObj, NULL, NULL, &pDepEvent);
	if (pDepEvent)
	{
		m_pEvent->SetColor(EVENT_STATE_RED);
		m_pEvent->AddDependentOn(pDepEvent);
		m_pCommandQueue->GetEventsManager()->ReleaseEvent(pDepEvent->GetHandle());
		return CL_NOT_READY;
	}
#endif

	/// at this phase we know the m_pDstMemObj is valid on target device
	FissionableDevice* pSrcDevice = m_pSrcMemObj->GetLocation();
	FissionableDevice* pDstDevice = m_pDevice;

	bool bSrcOnRuntime = (NULL == pSrcDevice);
	bool bNeedInvolveRuntime = ( (NULL != pSrcDevice) && !m_pSrcMemObj->IsSharedWith(pDstDevice));
	bool bCopyOnDevice = ( (NULL != pSrcDevice) && m_pSrcMemObj->IsSharedWith(pDstDevice));

	if (bNeedInvolveRuntime)
	{
		/// if m_pSrcMemObj is on different device than the target device, we
		/// copy it to host first and then update m_pDstMemObj from that copy.
		CopyToHost(m_pSrcMemObj, &pDepEvent);
		if (pDepEvent)
		{
			m_Event.SetColor(EVENT_STATE_RED);
			m_Event.AddDependentOn(pDepEvent);
			m_pCommandQueue->GetEventsManager()->ReleaseEvent(pDepEvent->GetHandle());
			return CL_NOT_READY;
		}
	}
	else if (bSrcOnRuntime)
	{
//		// For buffers which are in sync with the host we can can simply copy the raw data from the memory using Write operation.
//		if (m_pSrcMemObj->GetType() == CL_MEM_OBJECT_BUFFER)
//		{
			void* pData = m_pSrcMemObj->GetBackingStoreData(NULL);
			QueueEvent* pEvent = GetEvent();
			res = CopyFromHost(pData, m_pDstMemObj, m_szSrcOrigin, m_szDstOrigin, m_szRegion, m_szSrcRowPitch, m_szSrcSlicePitch, m_szDstRowPitch, m_szDstSlicePitch, &pEvent);
//		} else
//		{
//			// We cannot copy images from host using Write operation, since images has their own ElementSize/Pitches ... and write operation won't
//			// work there
//			res = CopyOnDevice(clDeviceId);
//		}
		if (CL_FAILED(res))
		{
			return res;
		}
		return CL_SUCCESS;
	}
	else if (bCopyOnDevice)
	{
		res = CopyOnDevice(pDstDevice);
		if (CL_FAILED(res)) { return res; }
	}
	else
	{
		// do nothing, return CL_DONE_ON_RUTIME to signal for the queue, that
		// the command hasn't been forwarded to the device and finished at runtime level.
		return CL_DONE_ON_RUNTIME;
	}

	return res;
}

/******************************************************************
 * Copy memory objects on the host, no access to a device,
 * read data from one object and update the second object data.
 *
 * TODO: Add support to copy between images on Host
 ******************************************************************/

/******************************************************************
 * Use device copy command to copy between the buffers.
 * Pre condition for this function is that the 2 buffers are allocated
 * in the device.
 ******************************************************************/
cl_err_code CopyMemObjCommand::CopyOnDevice(FissionableDevice* pDevice)
{
    cl_dev_cmd_desc *m_pDevCmd = &m_DevCmd;
    cl_dev_cmd_param_copy* pCopyParams   = &m_copyParams;

	OclEvent* pSrcObjEvent;
	cl_err_code clErr = m_pSrcMemObj->GetDeviceDescriptor(pDevice, &(pCopyParams->srcMemObj), &pSrcObjEvent);
	if ( CL_FAILED(clErr) )
	{
		return clErr;
	}
	OclEvent* pDstObjEvent;
    clErr = m_pDstMemObj->GetDeviceDescriptor(pDevice, &(pCopyParams->dstMemObj), &pDstObjEvent);
	if ( CL_FAILED(clErr) )
	{
		return clErr;
	}

    pCopyParams->src_dim_count  = m_uiSrcNumDims;
    pCopyParams->dst_dim_count  = m_uiDstNumDims;

	pCopyParams->src_pitch[0] = m_szSrcRowPitch;
	pCopyParams->src_pitch[1] = m_szSrcSlicePitch;
	pCopyParams->dst_pitch[0] = m_szDstRowPitch;
	pCopyParams->dst_pitch[1] = m_szDstSlicePitch;

    // set all values
    for( int i=0; i< MAX_WORK_DIM; i++ )
    {
        pCopyParams->src_origin[i]  = m_szSrcOrigin[i];
        pCopyParams->dst_origin[i]  = m_szDstOrigin[i];
        pCopyParams->region[i]      = m_szRegion[i];
    }

    m_pDevCmd->type			= CL_DEV_CMD_COPY;
    m_pDevCmd->id			= (cl_dev_cmd_id)m_Event.GetId();
    m_pDevCmd->params		= pCopyParams;
    m_pDevCmd->param_size	= sizeof(cl_dev_cmd_param_copy);
	m_pDevCmd->profiling	= (m_pCommandQueue->IsProfilingEnabled() ? true : false );
	m_pDevCmd->data			= static_cast<ICmdStatusChangedObserver*>(this);

	m_Event.SetEventQueue(m_pCommandQueue);
    // Sending 1 command to the device where the buffer is located now
    // Color will be changed only when command is submitted in the device
    LogDebugA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), m_iId);

	cl_dev_err_code devErr = m_pDevice->GetDeviceAgent()->clDevCommandListExecute(m_clDevCmdListId, &m_pDevCmd, 1);
	if ( CL_DEV_FAILED(devErr) )
	{
		return CL_OUT_OF_RESOURCES;
	}

	m_pDstMemObj->UpdateLocation(m_pDevice);

	return CL_SUCCESS;
}

/******************************************************************
 * This function copies the data from the clSrcDeviceId device
 * to the dst buffer local memory.
 *
 * TODO: Add support images CopyToHost, current version valid for buffers only
 ******************************************************************/
cl_err_code CopyMemObjCommand::CommandDone()
{
    m_pSrcMemObj->RemovePendency(this);
    m_pDstMemObj->RemovePendency(this);

    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
CopyBufferCommand::CopyBufferCommand(
	  IOclCommandQueueBase* cmdQueue,
	  ocl_entry_points *    pOclEntryPoints,
            MemoryObject*   pSrcBuffer,
            MemoryObject*   pDstBuffer,
            const size_t    pszSrcOrigin[3],
            const size_t    pszDstOrigin[3],
            const size_t    pszRegion[3]
            ): CopyMemObjCommand(cmdQueue, pOclEntryPoints, pSrcBuffer, pDstBuffer, pszSrcOrigin, pszDstOrigin, pszRegion)
{
	m_commandType = CL_COMMAND_COPY_BUFFER;
}

/******************************************************************
 *
 ******************************************************************/
 CopyBufferCommand::~CopyBufferCommand()
{
}


 /******************************************************************
 *
 ******************************************************************/
CopyBufferRectCommand::CopyBufferRectCommand(
	  IOclCommandQueueBase* cmdQueue,
	  ocl_entry_points *    pOclEntryPoints,
            MemoryObject*   pSrcBuffer,
            MemoryObject*   pDstBuffer,
            const size_t    pszSrcOrigin[3],
            const size_t    pszDstOrigin[3],
            const size_t    pszRegion[3],
			const size_t	szSrcRowPitch,
			const size_t	szSrcSlicePitch,
			const size_t	szDstRowPitch,
			const size_t	szDstSlicePitch
            ): CopyMemObjCommand(cmdQueue, pOclEntryPoints, pSrcBuffer, pDstBuffer, pszSrcOrigin,
				pszDstOrigin, pszRegion,szSrcRowPitch, szSrcSlicePitch, szDstRowPitch, szDstSlicePitch)
{
	m_uiSrcNumDims = MAX_WORK_DIM;
	m_uiDstNumDims = MAX_WORK_DIM;
	m_commandType = CL_COMMAND_COPY_BUFFER_RECT;
}

/******************************************************************
 *
 ******************************************************************/
 CopyBufferRectCommand::~CopyBufferRectCommand()
{
}

/******************************************************************
 *
 ******************************************************************/
CopyImageCommand::CopyImageCommand(
    IOclCommandQueueBase* cmdQueue,
    ocl_entry_points *    pOclEntryPoints,
    MemoryObject*   pSrcImage,
    MemoryObject*   pDstImage,
    const size_t*   pszSrcOrigin,
    const size_t*   pszDstOrigin,
    const size_t*   pszRegion
    ): CopyMemObjCommand(cmdQueue, pOclEntryPoints, pSrcImage, pDstImage, pszSrcOrigin, pszDstOrigin, pszRegion)
{
	m_commandType = CL_COMMAND_COPY_IMAGE;
	pSrcImage->GetLayout(NULL, &m_szSrcRowPitch, &m_szSrcSlicePitch);
}

/******************************************************************
 *
 ******************************************************************/
CopyImageCommand::~CopyImageCommand()
{
}

/******************************************************************
 *
 ******************************************************************/
CopyBufferToImageCommand::CopyBufferToImageCommand(
	IOclCommandQueueBase* cmdQueue,
	ocl_entry_points *    pOclEntryPoints,
    MemoryObject*   pSrcBuffer,
    MemoryObject*   pDstImage,
    size_t          pszSrcOffset[3],
    const size_t*   pszDstOrigin,
    const size_t*   pszDstRegion
    ): CopyMemObjCommand(cmdQueue, pOclEntryPoints, pSrcBuffer, pDstImage, pszSrcOffset, pszDstOrigin, pszDstRegion)
{
	m_commandType = CL_COMMAND_COPY_BUFFER_TO_IMAGE;
}

/******************************************************************
 *
 ******************************************************************/
CopyBufferToImageCommand::~CopyBufferToImageCommand()
{
}

/******************************************************************
 *
 ******************************************************************/
CopyImageToBufferCommand::CopyImageToBufferCommand(
	IOclCommandQueueBase* cmdQueue,
	ocl_entry_points *    pOclEntryPoints,
    MemoryObject*   pSrcImage,
    MemoryObject*   pDstBuffer,
    const size_t*   pszSrcOrigin,
    const size_t*   pszSrcRegion,
	size_t          pszDstOffset[3]
    ): CopyMemObjCommand(cmdQueue, pOclEntryPoints, pSrcImage, pDstBuffer, pszSrcOrigin, pszDstOffset, pszSrcRegion)
{
	m_commandType = CL_COMMAND_COPY_IMAGE_TO_BUFFER;
}

/******************************************************************
 *
 ******************************************************************/
CopyImageToBufferCommand::~CopyImageToBufferCommand()
{
}

/******************************************************************
 *
 ******************************************************************/
MapBufferCommand::MapBufferCommand(	IOclCommandQueueBase* cmdQueue, ocl_entry_points *    pOclEntryPoints,
                                    MemoryObject* pBuffer, cl_map_flags clMapFlags, size_t szOffset, size_t szCb):
    MapMemObjCommand(cmdQueue, pOclEntryPoints, pBuffer, clMapFlags, NULL, NULL, NULL, NULL)
{
    m_szOrigin[0] = szOffset;
    m_szRegion[0] = szCb;
}

/******************************************************************
 *
 ******************************************************************/
MapBufferCommand::~MapBufferCommand()
{
}

/******************************************************************
 *
 ******************************************************************/
MapImageCommand::MapImageCommand(
	 IOclCommandQueueBase*  cmdQueue,
	 ocl_entry_points *     pOclEntryPoints,
            MemoryObject*   pImage,
            cl_map_flags    clMapFlags,
            const size_t*   pOrigin,
            const size_t*   pRegion,
            size_t*         pszImageRowPitch,
            size_t*         pszImageSlicePitch
            ):
MapMemObjCommand(cmdQueue, pOclEntryPoints, pImage, clMapFlags, pOrigin, pRegion, pszImageRowPitch, pszImageSlicePitch)
{
}

/******************************************************************
 *
 ******************************************************************/
MapImageCommand::~MapImageCommand()
{
}

/******************************************************************
 *
 ******************************************************************/
MapMemObjCommand::MapMemObjCommand(
      IOclCommandQueueBase* cmdQueue,
      ocl_entry_points *    pOclEntryPoints,
            MemoryObject*   pMemObj,
            cl_map_flags    clMapFlags,
            const size_t*   pOrigin,
            const size_t*   pRegion,
            size_t*         pszImageRowPitch,
            size_t*         pszImageSlicePitch
            ):
    Command(cmdQueue, pOclEntryPoints),
    m_clMapFlags(clMapFlags),
    m_pszImageRowPitch(pszImageRowPitch),
    m_pszImageSlicePitch(pszImageSlicePitch),
    m_pHostDataPtr(NULL)
{
    for( cl_uint i =0; i<MAX_WORK_DIM; i++)
    {
		if ( NULL != pOrigin )
			m_szOrigin[i] = pOrigin[i];
		else
			m_szOrigin[i] = 0;

		if ( NULL != pRegion )
			m_szRegion[i] = pRegion[i];
		else
			m_szRegion[i] = 1;
    }

    m_pMemObj = GetImageFromArray(pMemObj, m_szOrigin[2]);
}

/******************************************************************
 *
 ******************************************************************/
MapMemObjCommand::~MapMemObjCommand()
{
}

/******************************************************************
 * On command initilazation a pointer to the mapped region is returned
 *
 ******************************************************************/
cl_err_code MapMemObjCommand::Init()
{
    cl_err_code res;
    m_pMemObj->AddPendency(NULL);   // NULL because this command won't be the one that will remove the dependency - the unmap command will

    res = m_pMemObj->CreateDeviceResource(m_pDevice);
    if( CL_FAILED(res))
    {
		m_pMemObj->RemovePendency(this);
		assert(0);
		return res;
	}

    // Get pointer to the device
	cl_err_code err = m_pMemObj->CreateMappedRegion(m_pDevice, m_clMapFlags, m_szOrigin, m_szRegion, m_pszImageRowPitch, m_pszImageSlicePitch,
	                                                &m_pMappedRegion, &m_pHostDataPtr);
    if ( CL_FAILED(err) )
    {
		assert(0);
        // Case of error
        return CL_MEM_OBJECT_ALLOCATION_FAILURE;
    }

	// Initialize GPA data
	GPA_InitCommand();

    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code MapMemObjCommand::Execute()
{
	cl_err_code res = CL_SUCCESS;


#ifdef USE_PREPARE_ON_DEVICE
    // TODO: Add support for multiple device.
    // What happens when data is not on the same device???
	QueueEvent* pMemObjEvent = NULL;
	res = PrepareOnDevice(m_pMemObj, m_pOrigin, m_pRegion, &pMemObjEvent);
	if (CL_FAILED(res)) { return res; }

	if (pMemObjEvent)
	{
		m_pEvent->AddFloatingDependence();
		m_pEvent->SetColor(EVENT_STATE_RED);
		m_pEvent->AddDependentOn(pMemObjEvent);
		m_pEvent->RemoveFloatingDependence();
		m_pCommandQueue->GetEventsManager()->ReleaseEvent(pMemObjEvent->GetHandle());
		return CL_NOT_READY;
	}
#endif

	cl_dev_cmd_desc *m_pDevCmd = &m_DevCmd;
    // Prepare command.
    // Anyhow we send the map command to the device though  we expect that on write
    // there is nothing to do, and on read the device may need to copy from device memory to host memory
    m_pDevCmd->id          = (cl_dev_cmd_id)m_Event.GetId();
    m_pDevCmd->type        = CL_DEV_CMD_MAP;
    m_pDevCmd->param_size  = sizeof(cl_dev_cmd_param_map);
    m_pDevCmd->params      = m_pMappedRegion;

	m_pDevCmd->profiling = (m_pCommandQueue->IsProfilingEnabled() ? true : false );
	m_pDevCmd->data			= static_cast<ICmdStatusChangedObserver*>(this);

	m_Event.SetEventQueue(m_pCommandQueue);
	// Change status of the command to Gray before handle by the device
    // Color will be changed only when command is submitted in the device
    LogDebugA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), m_iId);

	res = m_pDevice->GetDeviceAgent()->clDevCommandListExecute(m_clDevCmdListId, &m_pDevCmd, 1);
	m_pMemObj->UpdateLocation(m_pDevice);
	return res;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code MapMemObjCommand::CommandDone()
{
    // Synch data with user provided HostMapPtr
    cl_err_code res = m_pMemObj->SynchDataToHost( m_pMappedRegion, m_pHostDataPtr );

    // Don't remove buffer pendency, the buffer should be alive at least until unmap is done.
    return res;
}

/******************************************************************
 *
 ******************************************************************/
UnmapMemObjectCommand::UnmapMemObjectCommand(IOclCommandQueueBase* cmdQueue, ocl_entry_points* pOclEntryPoints, MemoryObject* pMemObject, void* pMappedPtr):
	Command(cmdQueue, pOclEntryPoints),
    m_pMemObject(pMemObject),
    m_pMappedPtr(pMappedPtr)
{
}

/******************************************************************
 *
 ******************************************************************/
UnmapMemObjectCommand::~UnmapMemObjectCommand()
{
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code UnmapMemObjectCommand::Init()
{
    // First check the the region has been mapped
    cl_err_code err = m_pMemObject->GetMappedRegionInfo(m_pDevice, m_pMappedPtr, &m_pMappedRegion);

	// Initialize GPA data
	GPA_InitCommand();

	return err;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code UnmapMemObjectCommand::Execute()
{
	cl_dev_cmd_desc *m_pDevCmd = &m_DevCmd;

    // Create and send unmap command
    m_pDevCmd->id          = (cl_dev_cmd_id)m_Event.GetId();
    m_pDevCmd->type        = CL_DEV_CMD_UNMAP;
    m_pDevCmd->param_size  = sizeof(cl_dev_cmd_param_map);
    m_pDevCmd->params      = m_pMappedRegion;
	m_pDevCmd->data			= static_cast<ICmdStatusChangedObserver*>(this);

    // Color will be changed only when command is submitted in the device
    LogDebugA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), m_iId);

	m_pMemObject->UpdateLocation(m_pDevice);
	m_Event.SetEventQueue(m_pCommandQueue);
	m_pDevCmd->profiling = (m_pCommandQueue->IsProfilingEnabled() ? true : false );

    // we need to synchronize backinjg store data with host map data, but this
    // should be done immediately before command execution on device to support
    // native kernels that may access mapped object data
    // In order to do this we override Command::NotifyCmdStatusChanged(CL_RUNNING)
    // which is called by device immediately before execution start.

	return m_pDevice->GetDeviceAgent()->clDevCommandListExecute(m_clDevCmdListId, &m_pDevCmd, 1);
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code UnmapMemObjectCommand::NotifyCmdStatusChanged(
                                           cl_dev_cmd_id clCmdId,
                                           cl_int iCmdStatus, cl_int iCompletionResult,
                                           cl_ulong ulTimer)
{
    if (CL_RUNNING == iCmdStatus)
    {
        // we are called from inside device and immediately before command start.
        // it's a time to synch host map data with Memory Object backing store
        m_pMemObject->SynchDataFromHost( m_pMappedRegion, m_pMappedPtr );
    }

    // propagate notification further
    return Command::NotifyCmdStatusChanged( clCmdId, iCmdStatus, iCompletionResult, ulTimer );
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code UnmapMemObjectCommand::CommandDone()
{
    cl_err_code errVal;

    // Here we do the actual operation off releasing the mapped region.
    errVal = m_pMemObject->ReleaseMappedRegion(m_pMappedRegion, m_pMappedPtr);

    m_pMemObject->RemovePendency(NULL); // NULL because this command wasn't the one that added the dependency in the first place - it was the map command
    return errVal;
}

/******************************************************************
 *
 ******************************************************************/
NativeKernelCommand::NativeKernelCommand(
	IOclCommandQueueBase* cmdQueue,
	ocl_entry_points *    pOclEntryPoints,
	void              (CL_CALLBACK*pUserFnc)(void *),
           void*               pArgs,
           size_t              szCbArgs,
           cl_uint             uNumMemObjects,
           MemoryObject**      ppMemObjList,
           const void**        ppArgsMemLoc):
Command(cmdQueue, pOclEntryPoints),
    m_pUserFnc(pUserFnc),
    m_pArgs(pArgs),
    m_szCbArgs(szCbArgs),
    m_uNumMemObjects(uNumMemObjects),
    m_ppMemObjList(ppMemObjList),
    m_ppArgsMemLoc(ppArgsMemLoc)
{
}

/******************************************************************
 *
 ******************************************************************/
NativeKernelCommand::~NativeKernelCommand()
{
}

/******************************************************************
 * On init the command validates the input buffers and creates new args list
 * the contains the device handlers of the buffers.
 ******************************************************************/
cl_err_code NativeKernelCommand::Init()
{
    cl_err_code res = CL_SUCCESS;
    // Create new arg list
	// Expect same size for cl_mem and cl_dev_mem
	STATIC_ASSERT(sizeof(cl_mem) == sizeof(cl_dev_memobj_handle*));
	if (sizeof(cl_mem) != sizeof(cl_dev_memobj_handle*))
	{
		return CL_INVALID_KERNEL_ARGS;
	}
    char*   pNewArgs = new char[m_szCbArgs];
	if(NULL == pNewArgs)
	{
		 return CL_OUT_OF_HOST_MEMORY;
	}

	// Now copy the whole buffer
	MEMCPY_S(pNewArgs, m_szCbArgs, m_pArgs, m_szCbArgs);

	size_t *ppNewArgsOffset = NULL;
	if (m_uNumMemObjects > 0)
	{
		ppNewArgsOffset = new size_t[m_uNumMemObjects];
		if(NULL == ppNewArgsOffset)
		{
			delete []pNewArgs;
			return CL_OUT_OF_HOST_MEMORY;
		}
	}

    cl_uint i;
	for( i=0; i < m_uNumMemObjects; i++ )
	{
		// Check that mem object is allocated on device, if not allocate resource
		MemoryObject* pMemObj = m_ppMemObjList[i];
		// Set buffers pendencies
		pMemObj->AddPendency(this);  
		// Check that mem object is allocated on device, if not allocate resource
		res = pMemObj->CreateDeviceResource(m_pDevice);
		if( CL_FAILED(res))
		{
			break;
		}

		size_t stObjOffset = (size_t)((char*)(m_ppArgsMemLoc[i]) - (char*)m_pArgs);
	    cl_dev_memobj_handle* pNewMemObjLocation = (cl_dev_memobj_handle*)(pNewArgs + stObjOffset);

		// Set the new args list
		IOCLDevMemoryObject* clDevMemHndl;
		OclEvent* pObjEvent;
		res = pMemObj->GetDeviceDescriptor(m_pDevice, &clDevMemHndl, &pObjEvent);
		if( CL_FAILED(res))
		{
			break;
		}
		*((cl_dev_memobj_handle*)pNewMemObjLocation) = clDevMemHndl;
		ppNewArgsOffset[i] = stObjOffset;
	}

	// Need to rollback in case of error
	if ( CL_FAILED(res) )
	{
		for( cl_uint j=0; j<i; ++j)
		{
			MemoryObject* pMemObj = m_ppMemObjList[j];
			pMemObj->RemovePendency(this);
		}

		delete []pNewArgs;
		delete []ppNewArgsOffset;
		return res;
	}

    //
    // Prepare the device command
    //
	cl_dev_cmd_desc *m_pDevCmd = &m_DevCmd;
	cl_dev_cmd_param_native* pNativeKernelParam = &m_nativeParams;

    pNativeKernelParam->args     = m_szCbArgs;
    pNativeKernelParam->argv     = pNewArgs;
    pNativeKernelParam->func_ptr = m_pUserFnc;
    pNativeKernelParam->mem_num  = m_uNumMemObjects;
    pNativeKernelParam->mem_offset = ppNewArgsOffset;

    m_pDevCmd->params = pNativeKernelParam;
    m_pDevCmd->param_size = sizeof(cl_dev_cmd_param_native);
    m_pDevCmd->type = CL_DEV_CMD_EXEC_NATIVE;

    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code NativeKernelCommand::Execute()
{
    LogDebugA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), m_iId);

	// Fill command descriptor
	cl_dev_cmd_desc *m_pDevCmd = &m_DevCmd;
	m_pDevCmd->id = (cl_dev_cmd_id)m_Event.GetId();

#ifdef USE_PREPARE_ON_DEVICE
	#error please review the code
    for( unsigned int i=0; i < m_uNumMemObjects; i++ )
    {
        MemoryObject* pMemObj = m_ppMemObjList[i];
		QueueEvent* pDepEvent = NULL;
		PrepareOnDevice(pMemObj, NULL, NULL, &pDepEvent);
		if (pDepEvent)
		{
			if (!hasDepends)
			{
				m_pEvent->AddFloatingDependence();
				m_pEvent->SetColor(EVENT_STATE_RED);
				hasDepends = true;
			}
			m_pEvent->AddDependentOn(pDepEvent);
			m_pCommandQueue->GetEventsManager()->ReleaseEvent(pDepEvent->GetHandle());
		}
		else
		{
			pMemObj->UpdateLocation(m_pCommandQueue->GetQueueDeviceHandle());
		}
	}
	if (hasDepends)
	{
		m_pEvent->RemoveFloatingDependence();
		return CL_NOT_READY;
	}
#else
    for( unsigned int i=0; i < m_uNumMemObjects; i++ )
    {
        MemoryObject* pMemObj = m_ppMemObjList[i];
	    pMemObj->UpdateLocation(m_pDevice);
	}
#endif

	m_pDevCmd->profiling	= (m_pCommandQueue->IsProfilingEnabled() ? true : false );
	m_pDevCmd->data			= static_cast<ICmdStatusChangedObserver*>(this);

	return m_pDevice->GetDeviceAgent()->clDevCommandListExecute(m_clDevCmdListId, &m_pDevCmd, 1);
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code NativeKernelCommand::CommandDone()
{
    // Clean resources
	cl_dev_cmd_desc *m_pDevCmd = &m_DevCmd;
	cl_dev_cmd_param_native* pNativeKernelParam = (cl_dev_cmd_param_native*)m_pDevCmd->params;
	//Can be null of out of memory encountered during init
	if (NULL != pNativeKernelParam->argv)
    {
		char* temp = (char*)pNativeKernelParam->argv;
		delete[] temp;
	}
	if (NULL != pNativeKernelParam->mem_offset)
	{
		delete[] pNativeKernelParam->mem_offset;
    }

    // Remove buffers pendencies
    for( cl_uint i=0; i < m_uNumMemObjects; i++ )
    {
        // Check that mem object is allocated on device, if not allocate resource
        MemoryObject* pMemObj = m_ppMemObjList[i];
        pMemObj->RemovePendency(this);
    }
	delete []m_ppMemObjList;

    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
NDRangeKernelCommand::NDRangeKernelCommand(
	IOclCommandQueueBase* cmdQueue,
	ocl_entry_points*     pOclEntryPoints,
    Kernel*         pKernel,
    cl_uint         uiWorkDim,
    const size_t*   cpszGlobalWorkOffset,
    const size_t*   cpszGlobalWorkSize,
    const size_t*   cpszLocalWorkSize
    ):
Command(cmdQueue, pOclEntryPoints),
m_pKernel(pKernel),
m_uiWorkDim(uiWorkDim),
m_cpszGlobalWorkOffset(cpszGlobalWorkOffset),
m_cpszGlobalWorkSize(cpszGlobalWorkSize),
m_cpszLocalWorkSize(cpszLocalWorkSize)
{
}

/******************************************************************
 *
 ******************************************************************/
NDRangeKernelCommand::~NDRangeKernelCommand()
{
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code NDRangeKernelCommand::Init()
{
    cl_err_code res = CL_SUCCESS;
    // We have to use init to create a snapshot of the buffer kernels on enqueue
    // Thus, we also create and set the device command appropriately as much as we can.

	// Add ownership on the object
	m_pKernel->AddPendency(this);

    // Create args snapshot
    size_t szArgCount = m_pKernel->GetKernelArgsCount();
    const KernelArg* pArg = NULL;
    size_t szCurrentLocation =0;
    size_t szSize = 0;
	size_t stTotalLocalSize = 0;

    size_t i;
    // First calculate location and set objects
	// TODO: Why we need two expensive passes, access to map, memcpy
	//		Join to single pass, consider build most of the buffer during SetKernelArgs
	//		Consider to add KernelArgument class, that can handle all argument cases
    for(i=0; i< szArgCount; i++)
    {
        pArg = m_pKernel->GetKernelArg(i);
        if(pArg->IsMemObject())
        {
            szSize = sizeof(IOCLDevMemoryObject*);
            // Create buffer resources here if not available.
            MemoryObject* pMemObj = (MemoryObject*)pArg->GetValue();
            // Mark as used
            pMemObj->AddPendency(this);
            res = pMemObj->CreateDeviceResource(m_pDevice);
            if( CL_FAILED(res))
            {
				break;
            }

            szCurrentLocation += szSize;
            m_MemOclObjects.push_back(pMemObj);
        }
		else if ( pArg->IsSampler() )
		{
            szSize   = sizeof(cl_uint);
			OCLObject<_cl_sampler_int>* pSampler = reinterpret_cast<OCLObject<_cl_sampler_int>*>(pArg->GetValue());
			pSampler->AddPendency(this);
            szCurrentLocation += szSize;
            m_NonMemOclObjects.push_back(reinterpret_cast<OCLObject<_cl_mem_int> *>(pSampler));
		}
		else if ( pArg->IsLocalPtr() )
		{
			stTotalLocalSize += *((size_t*)pArg->GetValue());
			szCurrentLocation += pArg->GetSize();
		}
		else
        {
            //Just calculate the size for allocation
            szCurrentLocation += pArg->GetSize();
        }
    }

	cl_ulong stImplicitSize = 0;
	m_pKernel->GetWorkGroupInfo(m_pDevice, CL_KERNEL_LOCAL_MEM_SIZE, sizeof(cl_ulong), &stImplicitSize, NULL);
	stImplicitSize += stTotalLocalSize;
	if ( stImplicitSize > m_pDevice->GetRootDevice()->GetMaxLocalMemorySize() )
	{
		res = CL_OUT_OF_RESOURCES;
	}

	if ( CL_FAILED(res) )
	{
		if ( i == szArgCount )	// Failed on local memory
		{
			--i;
		}
		// On Error we need to roll back
		for(size_t k=0; k<=i; ++k)
		{
	        pArg = m_pKernel->GetKernelArg(k);
		    if(pArg->IsMemObject())
			{
	            MemoryObject* pMemObj = (MemoryObject*)pArg->GetValue();
			    pMemObj->RemovePendency(this);
			} else if( pArg->IsSampler() )
			{
				OCLObject<_cl_sampler_int>* pSampler = reinterpret_cast<OCLObject<_cl_sampler_int>*>(pArg->GetValue());
				pSampler->RemovePendency(this);
			}
		}
		m_pKernel->RemovePendency(this);

		return res;
	}
    // Setup Kernel parameters
	cl_dev_cmd_desc *m_pDevCmd = &m_DevCmd;
    cl_dev_cmd_param_kernel* pKernelParam = &m_kernelParams;

    cl_char* pArgValues = new cl_char[szCurrentLocation];
    memset(pArgValues, 0, sizeof(cl_char)*szCurrentLocation);

    pKernelParam->arg_size = szCurrentLocation;
    pKernelParam->arg_values = (void*)pArgValues;

    size_t szArgSize = 0;
    cl_char* pArgValuesCurrentLocation = pArgValues;

    // Here set the arguments.
    for(i=0, szCurrentLocation=0; i< szArgCount; i++)
    {
        pArg = m_pKernel->GetKernelArg(i);
        if(pArg->IsMemObject())
        {
            szArgSize = sizeof(cl_dev_memobj_handle);
			IOCLDevMemoryObject* *devObjSrc = (IOCLDevMemoryObject**)pArgValuesCurrentLocation;
            MemoryObject* pMemObj = (MemoryObject*)pArg->GetValue();
			res = GetMemObjectDescriptor(pMemObj, devObjSrc);
            assert( CL_SUCCESS == res );
        }
        else if( pArg->IsSampler() )
        {
            szArgSize = sizeof(cl_uint);
            Sampler* pSampler = (Sampler*)pArg->GetValue();
			cl_uint value = pSampler->GetValue();
            *((cl_uint*)pArgValuesCurrentLocation) = value;
        }
        else
        {
            szArgSize = pArg->GetSize();
            // Copy data
            memcpy(pArgValuesCurrentLocation, pArg->GetValue(), szArgSize);
        }
        // increment pointer
        pArgValuesCurrentLocation += szArgSize;
    }


    // Fill specific command values
    pKernelParam->work_dim = m_uiWorkDim;
    for( cl_uint i=0; i < m_uiWorkDim; i++)
    {
		pKernelParam->glb_wrk_offs[i] = (NULL != m_cpszGlobalWorkOffset) ? m_cpszGlobalWorkOffset[i] : 0;
        pKernelParam->glb_wrk_size[i] = m_cpszGlobalWorkSize[i];
        // If m_cpszLocalWorkSize == NULL, set to 0. Agent is expected to handle lcl_wrk_size 0 as NULL
		pKernelParam->lcl_wrk_size[i] = (NULL != m_cpszLocalWorkSize) ? m_cpszLocalWorkSize[i] : 0;
    }

    m_pDevCmd->params = pKernelParam;
    m_pDevCmd->param_size = sizeof(cl_dev_cmd_param_kernel);
    m_pDevCmd->type = CL_DEV_CMD_EXEC_KERNEL;

	// Set GPA data
	GPA_InitCommand();

    return CL_SUCCESS;
}

/******************************************************************
 * TODO: Move buffer handles to init
 ******************************************************************/
cl_err_code NDRangeKernelCommand::Execute()
{
	// Set location

#ifdef USE_PREPARE_ON_DEVICE
	bool hasDepends = false;
	list<OCLObject<_cl_mem_int>*>::iterator it;
    for( it = m_MemOclObjects.begin(); it != m_MemOclObjects.end(); it++)
    {
		MemoryObject* pMemObj = (MemoryObject*)(*it);
		QueueEvent* pDepEvent = NULL;
		PrepareOnDevice(pMemObj, NULL, NULL, &pDepEvent);
		if (pDepEvent)
		{
			if (!hasDepends)
			{
				m_pEvent->AddFloatingDependence();
				m_pEvent->SetColor(EVENT_STATE_RED);
				hasDepends = true;
			}
			m_pEvent->AddDependentOn(pDepEvent);
			m_pCommandQueue->GetEventsManager()->ReleaseEvent(pDepEvent->GetHandle());
		}
		else
		{
			pMemObj->UpdateLocation(m_pCommandQueue->GetQueueDeviceHandle());
		}
	}
	if (hasDepends)
	{
		m_pEvent->RemoveFloatingDependence();
		return CL_NOT_READY;
	}
#else
	list<OCLObject<_cl_mem_int>*>::iterator it;
    for( it = m_MemOclObjects.begin(); it != m_MemOclObjects.end(); it++)
    {
		MemoryObject* pMemObj = (MemoryObject*)(*it);
		pMemObj->UpdateLocation(m_pDevice);
	}
#endif

	cl_dev_cmd_desc *m_pDevCmd = &m_DevCmd;
    cl_dev_cmd_param_kernel* pKernelParam = (cl_dev_cmd_param_kernel*)m_pDevCmd->params;
    // Fill command descriptor
    m_pDevCmd->id = (cl_dev_cmd_id)m_Event.GetId();

    pKernelParam->kernel = m_pKernel->GetDeviceKernelId(m_pDevice);

    // Color will be changed only when command is submitted in the device

    // Sending the queue command
    // TODO: Handle the case were buffers are located in different device.

    LogDebugA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), m_iId);

	m_pDevCmd->profiling = (m_pCommandQueue->IsProfilingEnabled() ? true : false );
	m_pDevCmd->data			= static_cast<ICmdStatusChangedObserver*>(this);
	m_Event.SetEventQueue(m_pCommandQueue);
	return m_pDevice->GetDeviceAgent()->clDevCommandListExecute(m_clDevCmdListId, &m_pDevCmd, 1);
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code NDRangeKernelCommand::CommandDone()
{
    // Clear all resources
    // Remove object pendencies

    list<OCLObject<_cl_mem_int>*>::iterator it;
    for( it = m_MemOclObjects.begin(); it != m_MemOclObjects.end(); it++)
    {
        OCLObject<_cl_mem_int>* obj = *it;
        obj->RemovePendency(this);
    }
    m_MemOclObjects.clear();

    for( it = m_NonMemOclObjects.begin(); it != m_NonMemOclObjects.end(); it++)
    {
        OCLObject<_cl_mem_int>* obj = *it;
        obj->RemovePendency(this);
    }
    m_NonMemOclObjects.clear();

    // Delete local command
	cl_dev_cmd_desc *m_pDevCmd = &m_DevCmd;
    cl_dev_cmd_param_kernel* pKernelParam = (cl_dev_cmd_param_kernel*)m_pDevCmd->params;
	cl_char* temp = (cl_char*)pKernelParam->arg_values;
    delete[] temp;

	// Remove ownership from the object
	m_pKernel->RemovePendency(this);

    return CL_SUCCESS;
}


/******************************************************************
 *
 ******************************************************************/
void NDRangeKernelCommand::GPA_WriteCommandMetadata()
{ 
#if defined (USE_GPA)
	ocl_gpa_data* pGPAData = m_pCommandQueue->GetGPAData();
	
	if ((NULL != pGPAData) && (pGPAData->bUseGPA) && (pGPAData->bEnableContextTracing))
	{
		// Set custom track 
		__itt_set_track(m_pCommandQueue->GPA_GetQueue()->m_pTrack);
		__itt_metadata_add(pGPAData->pContextDomain, m_pGpaCommand->m_CmdId, pGPAData->pWorkDimensionHandle, __itt_metadata_u32, 1,&m_uiWorkDim);

		GPA_WriteWorkMetadata(m_cpszGlobalWorkSize, pGPAData->pGlobalWorkSizeHandle);
		GPA_WriteWorkMetadata(m_cpszLocalWorkSize, pGPAData->pLocalWorkSizeHandle);
		GPA_WriteWorkMetadata(m_cpszGlobalWorkOffset, pGPAData->pGlobalWorkOffsetHandle);
	}
#endif
}
/******************************************************************
 *
 ******************************************************************/
#if defined (USE_GPA)
void NDRangeKernelCommand::GPA_WriteWorkMetadata(const size_t* pWorkMetadata, __itt_string_handle* stringHandle) const
{ 
	if (pWorkMetadata != NULL)
	{
		ocl_gpa_data* pGPAData = m_pCommandQueue->GetGPAData();
		std::stringstream ssWorkMetadata;

		// Set custom track 
		__itt_set_track(m_pCommandQueue->GPA_GetQueue()->m_pTrack);

		ssWorkMetadata << pWorkMetadata[0];
		for (unsigned int i = 1 ; i < m_uiWorkDim ; i++)
		{
			ssWorkMetadata << pWorkMetadata[i];
		}

		// Write Metadata to trace
		__itt_metadata_str_addA(pGPAData->pContextDomain, __itt_null, stringHandle, ssWorkMetadata.str().c_str(), ssWorkMetadata.str().size() + 1);
	}
}
#endif
/******************************************************************
 * Command: ReadBufferCommand
 * The functions below implement the Read Buffer functinoality
 *
 ******************************************************************/
ReadBufferCommand::ReadBufferCommand(IOclCommandQueueBase* cmdQueue, ocl_entry_points* pOclEntryPoints, MemoryObject* pBuffer, const size_t pszOffset[3], const size_t pszCb[3], void* pDst)
:ReadMemObjCommand(cmdQueue, pOclEntryPoints, pBuffer, pszOffset, pszCb, 0, 0, pDst)
{
	m_commandType = CL_COMMAND_READ_BUFFER;
}

//////////////////
ReadBufferCommand::~ReadBufferCommand()
{
}


/******************************************************************
 *
 ******************************************************************/
ReadBufferRectCommand::ReadBufferRectCommand(
	IOclCommandQueueBase* cmdQueue,
	ocl_entry_points *    pOclEntryPoints,
            MemoryObject*     pBuffer,
            const size_t      szBufferOrigin[3],
			const size_t      szDstOrigin[3],
			const size_t	  szRegion[3],
			const size_t	  szBufferRowPitch,
			const size_t	  szBufferSlicePitch,
			const size_t	  szDstRowPitch,
			const size_t	  szDstSlicePitch,
            void*             pDst
			):ReadMemObjCommand(cmdQueue, pOclEntryPoints, pBuffer, szBufferOrigin, szRegion, szBufferRowPitch, szBufferSlicePitch, pDst, szDstOrigin, szDstRowPitch, szDstSlicePitch)
{
	m_commandType = CL_COMMAND_READ_BUFFER_RECT;
}

ReadBufferRectCommand::~ReadBufferRectCommand()
{
}

/******************************************************************
 *
 ******************************************************************/
ReadImageCommand::ReadImageCommand(
								   IOclCommandQueueBase* cmdQueue,
								   ocl_entry_points *    pOclEntryPoints,
			MemoryObject*   pImage,
            const size_t*   pszOrigin,
            const size_t*   pszRegion,
            size_t          szRowPitch,
            size_t          szSlicePitch,
            void*           pDst)
:ReadMemObjCommand(cmdQueue, pOclEntryPoints, pImage, pszOrigin, pszRegion, 0, 0, pDst, NULL, szRowPitch, szSlicePitch)
{
	m_commandType = CL_COMMAND_READ_IMAGE;
}

/******************************************************************
 *
 ******************************************************************/
ReadImageCommand::~ReadImageCommand()
{
}
/******************************************************************
 *
 ******************************************************************/
ReadMemObjCommand::ReadMemObjCommand(
	IOclCommandQueueBase* cmdQueue,
	ocl_entry_points *    pOclEntryPoints,
    MemoryObject*   pMemObj,
    const size_t*   pszOrigin,
    const size_t*   pszRegion,
    size_t          szRowPitch,
    size_t          szSlicePitch,
    void*           pDst,
	const size_t*	pszDstOrigin,
	const size_t    szDstRowPitch,
    const size_t    szDstSlicePitch
    ):
	MemoryCommand(cmdQueue, pOclEntryPoints),
    m_pMemObj(pMemObj),
    m_szMemObjRowPitch(szRowPitch),
    m_szMemObjSlicePitch(szSlicePitch),
    m_pDst(pDst),
	m_szDstRowPitch(szDstRowPitch),
	m_szDstSlicePitch(szDstSlicePitch)
{
//	size_t uiDimCount = m_pMemObj->GetNumDimensions();

    // Set region
    for( cl_uint i =0; i<MAX_WORK_DIM; i++)
    {
		m_szOrigin[i] = pszOrigin[i];
		m_szRegion[i] = pszRegion[i];

		if (pszDstOrigin)
		{
			m_szDstOrigin[i] = pszDstOrigin[i];
		}
		else
		{
			m_szDstOrigin[i] = 0;
		}
	}

	if (pMemObj->GetType() != CL_MEM_OBJECT_BUFFER)
	{
		if( 0 == szDstRowPitch  )
		{
			m_szDstRowPitch = pszRegion[0] * pMemObj->GetPixelSize();
		}
		if( 0 == szDstSlicePitch )
		{
			m_szDstSlicePitch = m_szDstRowPitch * pszRegion[1];
		}
	}
}

/******************************************************************
 *
 ******************************************************************/
ReadMemObjCommand::~ReadMemObjCommand()
{
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code ReadMemObjCommand::Init()
{
	m_pMemObj->AddPendency(this);

	// Initialize GPA data
	GPA_InitCommand();

    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code ReadMemObjCommand::Execute()
{
    cl_err_code res = CL_SUCCESS;
	cl_dev_cmd_desc *m_pDevCmd = &m_DevCmd;

    FissionableDevice* clDeviceDataLocation = m_pMemObj->GetLocation();


	// Check if this command has become MARKER because of returning CL_DONE_ON_RUNTIME previously.
	// if Yes; then this is the second time Execute() is being called and we don't to do anything,
	// we already executed before.
	if (m_commandType == CL_COMMAND_MARKER)
	{
		m_returnCode = CL_SUCCESS;
		m_Event.SetColor(EVENT_STATE_BLACK);
		m_Event.RemovePendency(NULL);
		return CL_SUCCESS;
	}

	// We don't optimize the case of "clDeviceDataLocation == 0 && m_pMemObj->IsAllocated(0)" as long
	// we run on CPU only; since it case cause performance issues. for GPU device, its better
	// to used m_pMemObj->ReadData(..) in order to read the data.
    if ( NULL == clDeviceDataLocation )
    {
		// do nothing
		// data on runtime but isn't not valid, copying it is redundant
		return CL_DONE_ON_RUNTIME;
	}
	else
	{
		assert( clDeviceDataLocation == m_pDevice && "Currently only same device is supported");

		OclEvent* pObjEvent;
		cl_err_code clErr = m_pMemObj->GetDeviceDescriptor(clDeviceDataLocation, &m_rwParams.memObj, &pObjEvent);
		if ( CL_FAILED(clErr) )
		{
			return clErr;
		}

        create_dev_cmd_rw(
			m_commandType == CL_COMMAND_READ_BUFFER_RECT ? MAX_WORK_DIM  : m_pMemObj->GetNumDimensions(),
			m_pDst, m_szOrigin, m_szDstOrigin, m_szRegion, m_szDstRowPitch, m_szDstSlicePitch, m_szMemObjRowPitch, m_szMemObjSlicePitch,
            CL_DEV_CMD_READ,
            (cl_dev_cmd_id)m_Event.GetId(),
			m_pDevCmd,
			&m_rwParams);

        LogDebugA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), m_iId);
		m_Event.SetEventQueue(m_pCommandQueue);
        // Sending 1 command to the device where the buffer is located now
		res = m_pDevCmd->profiling = (m_pCommandQueue->IsProfilingEnabled() ? true : false );
		m_pDevCmd->data			= static_cast<ICmdStatusChangedObserver*>(this);

		// Read the buffer from where the data is most valid
		// device with Id==clDeviceDataLocation might be different than m_pDevice, hence
		// we don't necessarily read the data from m_pDevice.
		/*
		Device* pDevice;
		Context *pContext = (Context*)m_pMemObj->GetContext();
		res = pContext->GetDevice(clDeviceDataLocation, &pDevice);
		if (CL_FAILED(res))
		{
			return res;
		}
		*/
		res = m_pDevice->GetDeviceAgent()->clDevCommandListExecute(m_clDevCmdListId, &m_pDevCmd, 1);
    }
    return res;

}

/******************************************************************
 *
 ******************************************************************/
cl_err_code ReadMemObjCommand::CommandDone()
{
    m_pMemObj->RemovePendency(this);
    return CL_SUCCESS;
}


/******************************************************************
 *
 ******************************************************************/
TaskCommand::TaskCommand( IOclCommandQueueBase* cmdQueue, ocl_entry_points* pOclEntryPoints, Kernel* pKernel ):
    NDRangeKernelCommand(cmdQueue, pOclEntryPoints, pKernel, 1, NULL, &m_szStaticWorkSize, &m_szStaticWorkSize),
    m_szStaticWorkSize(1)
{

}

/******************************************************************
 *
 ******************************************************************/
TaskCommand::~TaskCommand()
{
}

/******************************************************************
 * initiate NDRangeKernel and change the device command type
 ******************************************************************/
cl_err_code TaskCommand::Init()
{
	cl_dev_cmd_desc *m_pDevCmd = &m_DevCmd;
    cl_err_code res = NDRangeKernelCommand::Init();
    if ( CL_SUCCEEDED (res) )
    {
        m_pDevCmd->type = CL_DEV_CMD_EXEC_KERNEL;
    }
    return res;
}

/******************************************************************
 *
 ******************************************************************/
WriteBufferCommand::WriteBufferCommand(IOclCommandQueueBase* cmdQueue, ocl_entry_points* pOclEntryPoints, cl_bool bBlocking, MemoryObject* pBuffer, const size_t* pszOffset, const size_t* pszCb, const void* cpSrc)
: WriteMemObjCommand(cmdQueue, pOclEntryPoints, bBlocking, pBuffer, pszOffset, pszCb, 0, 0, cpSrc)
{
	m_commandType = CL_COMMAND_WRITE_BUFFER;
}


/******************************************************************
 *
 ******************************************************************/
WriteBufferCommand::~WriteBufferCommand()
{
}


/******************************************************************
 *
 ******************************************************************/
WriteBufferRectCommand::WriteBufferRectCommand(
	    IOclCommandQueueBase* cmdQueue,
	    ocl_entry_points *    pOclEntryPoints,
            cl_bool			bBlocking,
			MemoryObject*     pBuffer,
            const size_t      szBufferOrigin[3],
			const size_t      szSrcOrigin[3],
			const size_t	  szRegion[3],
			const size_t	  szBufferRowPitch,
			const size_t	  szBufferSlicePitch,
			const size_t	  szDstRowPitch,
			const size_t	  szDstSlicePitch,
            const void*       pDst
			):
	WriteMemObjCommand(cmdQueue, pOclEntryPoints, bBlocking, pBuffer, szBufferOrigin, szRegion, szBufferRowPitch, szBufferSlicePitch, pDst, szSrcOrigin, szDstRowPitch, szDstSlicePitch)
{
	m_commandType = CL_COMMAND_WRITE_BUFFER_RECT;
}

WriteBufferRectCommand::~WriteBufferRectCommand()
{
}

/******************************************************************
 *
 ******************************************************************/
WriteImageCommand::WriteImageCommand(
	IOclCommandQueueBase* cmdQueue,
	ocl_entry_points *    pOclEntryPoints,
	cl_bool			bBlocking,
    MemoryObject*   pImage,
    const size_t*   pszOrigin,
    const size_t*   pszRegion,
    size_t          szRowPitch,
    size_t          szSlicePitch,
    const void *    cpSrc
    ): WriteMemObjCommand(cmdQueue, pOclEntryPoints, bBlocking, pImage,pszOrigin, pszRegion, 0, 0, cpSrc, NULL, szRowPitch, szSlicePitch)
{
	m_commandType = CL_COMMAND_WRITE_IMAGE;
}
/******************************************************************
 *
 ******************************************************************/
WriteImageCommand::~WriteImageCommand()
{
}

/******************************************************************
 *
 ******************************************************************/
WriteMemObjCommand::WriteMemObjCommand(
	IOclCommandQueueBase* cmdQueue,
	ocl_entry_points *    pOclEntryPoints,
	cl_bool			bBlocking,
    MemoryObject*   pMemObj,
    const size_t*   pszOrigin,
    const size_t*   pszRegion,
    size_t          szRowPitch,
    size_t          szSlicePitch,
    const void *    cpSrc,
	const size_t*   pszSrcOrigin,
	const size_t    szSrcRowPitch,
	const size_t    szSrcSlicePitch
    ):
	MemoryCommand(cmdQueue, pOclEntryPoints),
	m_bBlocking(bBlocking),
    m_szMemObjRowPitch(szRowPitch),
    m_szMemObjSlicePitch(szSlicePitch),
    m_cpSrc(cpSrc),
	m_szSrcRowPitch(szSrcRowPitch),
	m_szSrcSlicePitch(szSrcSlicePitch)
{
	// Set region
    for( cl_uint i =0; i<MAX_WORK_DIM; i++)
    {
		m_szOrigin[i] = pszOrigin[i];
		m_szRegion[i] = pszRegion[i];

		if (pszSrcOrigin)
		{
			m_szSrcOrigin[i] = pszSrcOrigin[i];
		}
		else
		{
			m_szSrcOrigin[i] = 0;
		}
    }

    m_pMemObj = GetImageFromArray(pMemObj, m_szOrigin[2]);

	if (pMemObj->GetType() != CL_MEM_OBJECT_BUFFER)
	{
		if( 0 == szSrcRowPitch  )
		{
			// Get original image pitch
			m_szSrcRowPitch = pszRegion[0]*pMemObj->GetPixelSize();
		}
		if( 0 == szSrcSlicePitch )
		{
			// Get original image pitch
			m_szSrcSlicePitch = m_szSrcRowPitch*pszRegion[1];
		}
	}
	else
	{
		// For buffers, if not set row_pitch == slice_pitch == lenght
		if ( 0 == m_szSrcRowPitch )
		{
			m_szSrcRowPitch = m_szRegion[0];
		}
		if ( 0 == m_szSrcSlicePitch )
		{
			m_szSrcSlicePitch = m_szRegion[0];
		}
	}
}

/******************************************************************
 *
 ******************************************************************/
WriteMemObjCommand::~WriteMemObjCommand()
{
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code WriteMemObjCommand::Init()
{
	// If we are blocking command, we need to allocate internal buffer
	if ( m_bBlocking )
	{
		SMemCpyParams sCpyParam;
		// We need to allocate only the amount to being copied
		size_t sizeToAlloc = m_pMemObj->GetPixelSize();
		for(size_t dim=0; dim<MAX_WORK_DIM; ++dim)
		{
			sizeToAlloc *= m_szRegion[dim];
			sCpyParam.vRegion[dim] = m_szRegion[dim];
		}

		m_pTempBuffer = malloc(sizeToAlloc);
		if ( NULL == m_pTempBuffer )
		{
			LogErrorA("Can't allocate temporary storage for blockng command (%s)", GetCommandName());
			return CL_OUT_OF_HOST_MEMORY;
		}

		// Copy data
		sCpyParam.vRegion[0] *= m_pMemObj->GetPixelSize();
		sCpyParam.uiDimCount = m_commandType == CL_COMMAND_WRITE_BUFFER_RECT ? MAX_WORK_DIM  : m_pMemObj->GetNumDimensions();
		sCpyParam.pDst = (cl_char*)m_pTempBuffer;
		sCpyParam.vDstPitch[0] = sCpyParam.vRegion[0];
		sCpyParam.vDstPitch[1] = sCpyParam.vDstPitch[0] * m_szRegion[1];
		sCpyParam.pSrc = (cl_char*)m_cpSrc + m_szSrcOrigin[0]*m_pMemObj->GetPixelSize() + m_szSrcOrigin[1]*m_szSrcRowPitch + m_szSrcOrigin[2]*m_szSrcSlicePitch;
		sCpyParam.vSrcPitch[0] = m_szSrcRowPitch;
		sCpyParam.vSrcPitch[1] = m_szSrcSlicePitch;

		clCopyMemoryRegion(&sCpyParam);

		// Need to update source origin and pitch, now we will write from temporary buffer
		m_szSrcRowPitch = sCpyParam.vDstPitch[0];
		m_szSrcSlicePitch = sCpyParam.vDstPitch[1];
		memset(m_szSrcOrigin, 0, sizeof(m_szSrcOrigin));
	}

    // Allocate
    cl_err_code res = m_pMemObj->CreateDeviceResource(m_pDevice);
    if( CL_FAILED(res))
    {
        return res;
    }

	// Initialize GPA data
	GPA_InitCommand();

    m_pMemObj->AddPendency(this);
    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code WriteMemObjCommand::Execute()
{
 	cl_dev_cmd_desc *m_pDevCmd = &m_DevCmd;

#ifdef USE_PREPARE_ON_DEVICE
	 //  do change here: write to the last valid location, thought it might be the runtime.  same as read
	QueueEvent* pMemObjEvent = NULL;
	res = PrepareOnDevice(m_pMemObj, NULL, NULL, &pMemObjEvent);
	if (CL_FAILED(res)) { return res; }

	if (pMemObjEvent)
	{
		m_pEvent->SetColor(EVENT_STATE_RED);
		m_pEvent->AddDependentOn(pMemObjEvent);
		m_pCommandQueue->GetEventsManager()->ReleaseEvent(pMemObjEvent->GetHandle());
		return CL_NOT_READY;
	}
#endif

	/// memory object resides on target device, update it
	OclEvent* pObjEvent;
	cl_err_code clErr = m_pMemObj->GetDeviceDescriptor(m_pDevice, &m_rwParams.memObj, &pObjEvent);
	if ( CL_FAILED(clErr) )
	{
		return clErr;
	}

	create_dev_cmd_rw(
			m_commandType == CL_COMMAND_WRITE_BUFFER_RECT ? MAX_WORK_DIM  : m_pMemObj->GetNumDimensions(),
			m_bBlocking ? m_pTempBuffer : (void*)m_cpSrc,
			m_szOrigin, m_szSrcOrigin, m_szRegion, m_szSrcRowPitch, m_szSrcSlicePitch, m_szMemObjRowPitch, m_szMemObjSlicePitch,
			CL_DEV_CMD_WRITE,
			(cl_dev_cmd_id)m_Event.GetId(),
			m_pDevCmd,
			&m_rwParams) ;

	LogDebugA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), m_iId);
	m_Event.SetEventQueue(m_pCommandQueue);
	// Sending 1 command to the device where the buffer is located now
	m_pDevCmd->profiling = (m_pCommandQueue->IsProfilingEnabled() ? true : false );
	m_pDevCmd->data			= static_cast<ICmdStatusChangedObserver*>(this);

	cl_dev_err_code errDev = m_pDevice->GetDeviceAgent()->clDevCommandListExecute(m_clDevCmdListId, &m_pDevCmd, 1);
	if ( CL_DEV_FAILED(errDev) )
	{
		return CL_OUT_OF_RESOURCES;
	}

	m_pMemObj->UpdateLocation(m_pDevice);

	return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code WriteMemObjCommand::CommandDone()
{
	if ( m_bBlocking && (NULL != m_pTempBuffer) )
	{
		free(m_pTempBuffer);
		m_pTempBuffer = NULL;
	}

    m_pMemObj->RemovePendency(this);

    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code RuntimeCommand::Execute()
{
	m_returnCode = 0;
    LogDebugA("Command - DONE  : %s (Id: %d)", GetCommandName(), m_iId);
    CommandDone();
	m_Event.SetColor(EVENT_STATE_BLACK);
	m_Event.RemovePendency(NULL);
    return CL_SUCCESS;
}
