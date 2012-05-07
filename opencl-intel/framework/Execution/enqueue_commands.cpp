// Copyright (c) 2008-2012 Intel Corporation
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
#include "context_module.h"

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
#include "cl_sys_info.h"

#if defined(USE_GPA) 
#if defined(_M_X64)
    #define ITT_SIZE_T_METADATA_TYPE    __itt_metadata_u64
#else
    #define ITT_SIZE_T_METADATA_TYPE    __itt_metadata_u32
#endif
#endif


using namespace Intel::OpenCL::Framework;

using namespace Intel::OpenCL::Utils;

/******************************************************************
 *
 ******************************************************************/
Command::Command( IOclCommandQueueBase* cmdQueue, ocl_entry_points * pOclEntryPoints ):
    OCLObjectBase("Command"),
    m_Event(cmdQueue, pOclEntryPoints),
    m_clDevCmdListId(0),
	m_pDevice(NULL),
	m_pCommandQueue(cmdQueue),
	m_returnCode(CL_INVALID_OPERATION),
	m_memory_objects_acquired(false)
{
	memset(&m_DevCmd, 0, sizeof(cl_dev_cmd_desc));

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

    assert( (false == m_memory_objects_acquired) && "RelinquishMemoryObjects() was not called!");
	RELEASE_LOGGER_CLIENT;
}

cl_err_code Command::EnqueueSelf(cl_bool bBlocking, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent)
{
    if (NULL != cpEeventWaitList && NULL != pEvent && pEvent >= cpEeventWaitList && pEvent < &cpEeventWaitList[uNumEventsInWaitList])
    {
        // the spec says we should check this, but doesn't actually specify the exact error code
        return CL_INVALID_EVENT;
    }
	// 'this' may disapper during Enqueue if it was successful!
	return GetCommandQueue()->EnqueueCommand( this, bBlocking, uNumEventsInWaitList, cpEeventWaitList, pEvent );
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
        m_Event.SetEventState(EVENT_STATE_ISSUED_TO_DEVICE);
        LogDebugA("Command - SUBMITTED TO DEVICE  : %s (Id: %d)", GetCommandName(), m_Event.GetId());
        break;
    case CL_RUNNING:
        LogDebugA("Command - RUNNING  : %s (Id: %d)", GetCommandName(), m_Event.GetId());
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
        m_Event.SetEventState(EVENT_STATE_EXECUTING_ON_DEVICE);
        break;
    case CL_COMPLETE:
		assert(EVENT_STATE_DONE != m_Event.GetEventState());
		m_Event.SetProfilingInfo(CL_PROFILING_COMMAND_END, ulTimer);
	    // Complete command,
		// do that before set event, since side effect of SetEvent(black) may be deleting of this instance.
		// Is error
		if (CL_FAILED(iCompletionResult))
		{
			LogErrorA("Command - DONE - Failure  : %s (Id: %d)", GetCommandName(), m_Event.GetId());
			//assert(0 && "Command - DONE - Failure");
		}
		else
		{
			LogDebugA("Command - DONE - SUCCESS : %s (Id: %d)", GetCommandName(), m_Event.GetId());
		}
		m_returnCode = iCompletionResult;
        res = CommandDone();
        m_Event.SetEventState(EVENT_STATE_DONE);
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
    if (NULL == pMemObj)
    {
        ppMemObj = NULL;
    }
    else
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
    }
	return CL_SUCCESS;
}

// return true if ready
inline
bool Command::AcquireSingleMemoryObject( MemoryObjectArg& arg, FissionableDevice* pDev  )
{
    assert( NULL != arg.pMemObj );
    OclEvent* mem_event = arg.pMemObj->LockOnDevice( pDev, arg.access_rights );

    if (NULL != mem_event)
    {
        m_Event.AddDependentOn( mem_event );
    }

    return (NULL == mem_event);
}

cl_err_code Command::AcquireMemoryObjectsInt( MemoryObjectArgList* pList, MemoryObjectArg* pSingle, FissionableDevice* pDev )
{
    if ( m_memory_objects_acquired )
    {
        return CL_SUCCESS;
    }

    m_memory_objects_acquired = true;

    if (NULL == pDev)
    {
        pDev = m_pDevice;
    }
    
    bool ready = true;

    if (NULL != pList)
    {
        MemoryObjectArgList::iterator it     = pList->begin();
        MemoryObjectArgList::iterator it_end = pList->end();

        for (; it != it_end; ++it )
        {
            MemoryObjectArg& arg = *it;
            ready = ready & AcquireSingleMemoryObject( arg, pDev );
        }
    }
    else
    {
        assert( NULL != pSingle);
        ready = AcquireSingleMemoryObject( *pSingle, pDev );
    }

    if (false == ready)
    {       
		m_Event.SetEventState(EVENT_STATE_HAS_DEPENDENCIES);
    }

    return (ready) ? CL_SUCCESS : CL_NOT_READY;
}

void Command::RelinquishMemoryObjectsInt( MemoryObjectArgList* pList, MemoryObjectArg* pSingle, FissionableDevice* pDev )
{
    if ( !m_memory_objects_acquired )
    {
        return;
    }

    m_memory_objects_acquired = false;

    if (NULL == pDev)
    {
        pDev = m_pDevice;
    }

    if (NULL != pList)
    {      
        MemoryObjectArgList::iterator it     = pList->begin();
        MemoryObjectArgList::iterator it_end = pList->end();

        for (; it != it_end; ++it )
        {
            MemoryObjectArg& arg = *it;            
            arg.pMemObj->UnLockOnDevice( pDev, arg.access_rights ); 
        }
    }
    else
    {
        assert( NULL != pSingle );
        pSingle->pMemObj->UnLockOnDevice( pDev, pSingle->access_rights );
    }
}

inline
void Command::prepare_command_descriptor( cl_dev_cmd_type type, void* params, size_t params_size )
{
    m_DevCmd.id          = (cl_dev_cmd_id)m_Event.GetId(); // event ID is set inside queue, so we cannot save it in constructor
    m_DevCmd.type        = type;
    m_DevCmd.param_size  = params_size;
    m_DevCmd.params      = params;

	m_DevCmd.profiling   = (m_pCommandQueue->IsProfilingEnabled() ? true : false );
	m_DevCmd.data	     = static_cast<ICmdStatusChangedObserver*>(this);

	m_Event.SetEventQueue(m_pCommandQueue);
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
 * function to be used by all commands that need to write/read data
 ******************************************************************/
void MemoryCommand::create_dev_cmd_rw(
    cl_uint				uiDimCount,
    void*               pPtr,
    const size_t*       pszMemObjOrigin,
	const size_t*       pszPtrOrigin,
    const size_t*       pszRegion,
    size_t              szPtrRowPitch,
    size_t              szPtrSlicePitch,
    size_t              szMemObjRowPitch,
    size_t              szMemObjSlicePitch,
    cl_dev_cmd_type     clCmdType
    )
{
        // Create Read command
        m_rwParams.ptr = pPtr;

        cl_uint i;
        for( i=0; i<MAX_WORK_DIM; i++ )
        {
            m_rwParams.origin[i] = pszMemObjOrigin[i];
            m_rwParams.region[i] = pszRegion[i];

			if (pszPtrOrigin)
			{
				m_rwParams.ptr_origin[i] = pszPtrOrigin[i];
			}
			else
			{
				m_rwParams.ptr_origin[i] = 0;
			}

        }

        m_rwParams.pitch[0] = szPtrRowPitch;
        m_rwParams.pitch[1] = szPtrSlicePitch;

		m_rwParams.memobj_pitch[0] = szMemObjRowPitch;
        m_rwParams.memobj_pitch[1] = szMemObjSlicePitch;

        m_rwParams.dim_count = uiDimCount;

        prepare_command_descriptor(clCmdType, &m_rwParams, sizeof(cl_dev_cmd_param_rw));
}

/******************************************************************
 *
 ******************************************************************/
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

	m_pSrcMemObj = pSrcMemObj;
    m_pDstMemObj = pDstMemObj;

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

    bool override_target = m_pDstMemObj->IsWholeObjectCovered(m_uiDstNumDims, m_szDstOrigin, m_szRegion);

    m_objs.push_back( MemoryObjectArg( m_pSrcMemObj, MemoryObject::READ_ONLY ) );
    m_objs.push_back( MemoryObjectArg( m_pDstMemObj, override_target ? MemoryObject::WRITE_ENTIRE : MemoryObject::READ_WRITE) );

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
    if (CL_NOT_READY == AcquireMemoryObjects(m_objs))
    {
        return CL_NOT_READY;
    }
        
	cl_err_code res = CL_SUCCESS;

	/// at this phase we know the m_pDstMemObj is valid on target device
	res = CopyOnDevice(m_pDevice);

	return res;
}

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

    prepare_command_descriptor( CL_DEV_CMD_COPY, pCopyParams, sizeof(cl_dev_cmd_param_copy));

    // Sending 1 command to the device where the buffer is located now
    // Color will be changed only when command is submitted in the device
    LogDebugA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), m_Event.GetId());

	m_Event.AddPendency(this);
	m_pDstMemObj->AddPendency(this);
	cl_dev_err_code devErr = m_pDevice->GetDeviceAgent()->clDevCommandListExecute(m_clDevCmdListId, &m_pDevCmd, 1);
	m_pDstMemObj->RemovePendency(this);
	m_Event.RemovePendency(this);

	return CL_DEV_SUCCEEDED(devErr) ? CL_SUCCESS : CL_OUT_OF_RESOURCES;
}

/******************************************************************
 * This function copies the data from the clSrcDeviceId device
 * to the dst buffer local memory.
 *
 ******************************************************************/
cl_err_code CopyMemObjCommand::CommandDone()
{
    RelinquishMemoryObjects(m_objs);
    
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
    m_pHostDataPtr(NULL),
    m_pActualMappingDevice(NULL),
    m_ExecutionType( DEVICE_EXECUTION_TYPE ),
	m_pOclEntryPoints(pOclEntryPoints),
	m_pPostfixCommand(NULL),
    m_bResourcesAllocated(false)
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

    m_pMemObj.pMemObj = pMemObj;
    m_pMemObj.access_rights = MemoryObject::READ_WRITE;
}

/******************************************************************
 *
 ******************************************************************/
MapMemObjCommand::~MapMemObjCommand()
{
    if (m_bResourcesAllocated)
    {
        // Init was done, but execute was not called
        if (NULL != m_pPostfixCommand)
		{
            delete m_pPostfixCommand;
        }
        
        m_pMemObj.pMemObj->ReleaseMappedRegion( m_pMappedRegion, m_pHostDataPtr );
        m_pMemObj.pMemObj->RemovePendency(this);        
    }
}

/******************************************************************
 * On command initilazation a pointer to the mapped region is returned
 *
 ******************************************************************/
cl_err_code MapMemObjCommand::Init()
{
    cl_err_code res;
    MemoryObject* pMemObj = m_pMemObj.pMemObj;
    pMemObj->AddPendency(NULL);   // NULL because this command won't be the one that will remove the dependency - the unmap command will

    res = pMemObj->CreateDeviceResource(m_pDevice);
    if( CL_FAILED(res))
    {
		pMemObj->RemovePendency(this);
		assert(0);
		return res;
	}

    const FissionableDevice* actual_dev = NULL;
    
    // Get pointer to the device
	cl_err_code err = pMemObj->CreateMappedRegion(m_pDevice, m_clMapFlags, m_szOrigin, m_szRegion, m_pszImageRowPitch, m_pszImageSlicePitch,
	                                                &m_pMappedRegion, &m_pHostDataPtr, &actual_dev ); 
    if ( CL_FAILED(err) )
    {
		assert(0);
        // Case of error
        return CL_MEM_OBJECT_ALLOCATION_FAILURE;
    }

    m_pActualMappingDevice = const_cast<FissionableDevice*>(actual_dev);
	
	// check whether postfix command should be run to update user mirror area
	if ((0 == (CL_MAP_WRITE_INVALIDATE_REGION & m_pMappedRegion->flags))	&&		// region was not mapped for overriding by host
		(pMemObj->IsSynchDataWithHostRequired( m_pMappedRegion, m_pHostDataPtr )))
	{
		m_pPostfixCommand = new PrePostFixRuntimeCommand( this, PrePostFixRuntimeCommand::POSTFIX_MODE, GetCommandQueue(), m_pOclEntryPoints );

		if (NULL != m_pPostfixCommand)
		{
			err = m_pPostfixCommand->Init();
			if ( CL_FAILED(err) )
			{
				delete m_pPostfixCommand;
				m_pPostfixCommand = NULL;
			}
		}
		else
		{
			err = CL_OUT_OF_HOST_MEMORY;
		}

		if (NULL == m_pPostfixCommand)
		{
			pMemObj->ReleaseMappedRegion( m_pMappedRegion, m_pHostDataPtr );
			pMemObj->RemovePendency(this);
			assert(0);
			return err;
		}
	}

    if ((CL_MAP_WRITE_INVALIDATE_REGION & m_clMapFlags) && 
        (pMemObj->IsWholeObjectCovered(pMemObj->GetNumDimensions(), m_szOrigin, m_szRegion)))
    {
        m_pMemObj.access_rights = MemoryObject::WRITE_ENTIRE;
    }

	// Initialize GPA data
	GPA_InitCommand();

    m_bResourcesAllocated = true;
    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code MapMemObjCommand::Execute()
{
    assert( (RUNTIME_EXECUTION_TYPE != m_ExecutionType) && "MapMemObjCommand::Execute() called after it was switch to RUNTIME mode" );

    if (RUNTIME_EXECUTION_TYPE == m_ExecutionType)
    {
        return CL_NOT_READY;
    }
    
    if (CL_NOT_READY == AcquireMemoryObjects( m_pMemObj, m_pActualMappingDevice))
    {
        return CL_NOT_READY;
    }
    
	cl_dev_cmd_desc *m_pDevCmd = &m_DevCmd;
    // Prepare command.
    // Anyhow we send the map command to the device though  we expect that on write
    // there is nothing to do, and on read the device may need to copy from device memory to host memory
    prepare_command_descriptor( CL_DEV_CMD_MAP, m_pMappedRegion, sizeof(cl_dev_cmd_param_map));

	// Change status of the command to Gray before handle by the device
    // Color will be changed only when command is submitted in the device
    LogDebugA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), m_Event.GetId());
	m_Event.AddPendency(this);
	m_pMemObj.pMemObj->AddPendency(this);

    cl_dev_cmd_list device_cmd_list = m_clDevCmdListId;
    if (m_pActualMappingDevice != m_pDevice)
    {
        // cross device mode
        m_ExecutionType = RUNTIME_EXECUTION_TYPE;
        // use hidden queue if map to another device 
        device_cmd_list = NULL;
        // ensure we will exist after device will call for completion so that runtime scheduler will see us
        m_Event.AddPendency(this);
    }
        
	cl_dev_err_code errDev = m_pActualMappingDevice->GetDeviceAgent()->clDevCommandListExecute(
                                        device_cmd_list,
                                        &m_pDevCmd, 1);
	m_pMemObj.pMemObj->RemovePendency(this);
	m_Event.RemovePendency(this);

    cl_err_code err = CL_DEV_SUCCEEDED(errDev) ? CL_SUCCESS : CL_OUT_OF_RESOURCES;
    
    if ((CL_SUCCESS == err) && (RUNTIME_EXECUTION_TYPE == m_ExecutionType))
    {
        // do not allow command batching until command completion
        err = CL_NOT_READY;
    }
    
	return err;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code MapMemObjCommand::CommandDone()
{
	if (m_pPostfixCommand)
	{
		// error enqueue or no enqueue at all
		m_pPostfixCommand->ErrorDone();
		delete m_pPostfixCommand;
		m_pPostfixCommand = NULL;
	}

    RelinquishMemoryObjects( m_pMemObj, m_pActualMappingDevice );
    // Don't remove buffer pendency, the buffer should be alive at least until unmap is done.

    m_bResourcesAllocated = false;
    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code MapMemObjCommand::EnqueueSelf(cl_bool bBlocking, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent)
{
	// 'this' may disapper during Enqueue if it was successful!
	cl_err_code err = CL_SUCCESS;

	if (m_pPostfixCommand)
	{
		cl_event		intermediate_pEvent;
		EventsManager*	event_manager = GetCommandQueue()->GetEventsManager();

		PrePostFixRuntimeCommand* postfix  = m_pPostfixCommand;
		m_pPostfixCommand = NULL;	// in the case 'this' will disappear

		// 'this' may disapper after the self-enqueue is successful!
		err = Command::EnqueueSelf( bBlocking, uNumEventsInWaitList, cpEeventWaitList, &intermediate_pEvent );
		if (CL_FAILED(err))
		{
			// enqueue unsuccessful - 'this' still alive
			m_pPostfixCommand = postfix; // restore
			return err;
		}

		err = postfix->EnqueueSelf( bBlocking, 1, &intermediate_pEvent, pEvent );
		if (CL_FAILED(err))
		{
			// oops, unsuccessfull, but we need to schedule postfix in any case as user need to get back
			// pEvent and be able to make other commands dependent on it
			if (NULL != pEvent)
			{
				// add manually and leave postfix floating
				postfix->ErrorEnqueue( &intermediate_pEvent, pEvent, err );
			}
			else
			{
				// 'this' may not exist already - remove postfix manually
				postfix->ErrorDone();
				delete postfix;
			}
		}
	
		// release intermediate event
		event_manager->ReleaseEvent( intermediate_pEvent );

		// return success in any case as the original command fired ok
		return CL_SUCCESS;
	}
	else
	{
		// 'this' may disapper after the self-enqueue is successful!
		return Command::EnqueueSelf( bBlocking, uNumEventsInWaitList, cpEeventWaitList, pEvent );
	}
}


/******************************************************************
 *
 ******************************************************************/
cl_err_code	MapMemObjCommand::PostfixExecute()
{
	cl_err_code err;

#if defined(USE_GPA)

	ocl_gpa_data* pGPAData = m_pCommandQueue->GetGPAData();

	if ((NULL != pGPAData) && (pGPAData->bUseGPA))
	{
		cl_mem_obj_descriptor*	pMemObj;
		size_t regionInBytes[MAX_WORK_DIM] = {m_pMappedRegion->region[0],m_pMappedRegion->region[1],m_pMappedRegion->region[2]};	

		// Calculate each region size in bytes
		m_pMappedRegion->memObj->clDevMemObjGetDescriptor(CL_DEVICE_TYPE_CPU, 0, (cl_dev_memobj_handle*)&pMemObj);
		for (unsigned int i = 0 ; i < m_pMappedRegion->dim_count ; ++i)
		{
			regionInBytes[i] *= pMemObj->uiElementSize;
		}

		// Start Sync Data GPA task
		__itt_set_track(NULL);
		__itt_task_begin(pGPAData->pDeviceDomain, __itt_null, __itt_null, pGPAData->pSyncDataHandle);
		TAL_SetNamedTaskColor("Sync Data", 255, 0, 0);

		// Add region metadata to the Sync Data task
		switch(m_pMappedRegion->dim_count)
		{
		case 3:
			__itt_metadata_add(pGPAData->pDeviceDomain, __itt_null, pGPAData->pDepthHandle, ITT_SIZE_T_METADATA_TYPE, 1, &regionInBytes[2]);
		case 2:
			__itt_metadata_add(pGPAData->pDeviceDomain, __itt_null, pGPAData->pHeightHandle, ITT_SIZE_T_METADATA_TYPE, 1, &regionInBytes[1]);
		case 1:
			__itt_metadata_add(pGPAData->pDeviceDomain, __itt_null, (m_pMappedRegion->dim_count > 1) ? pGPAData->pWidthHandle : pGPAData->pSizeHandle, ITT_SIZE_T_METADATA_TYPE, 1, &regionInBytes[0]);
		}
	}
#endif
	
	err = m_pMemObj.pMemObj->SynchDataToHost( m_pMappedRegion, m_pHostDataPtr );

#if defined(USE_GPA)
	if ((NULL != pGPAData) && (pGPAData->bUseGPA))
	{
		// End Sync Data GPA task
		__itt_set_track(NULL);
		__itt_task_end(pGPAData->pDeviceDomain);
	} 
#endif

	return err;
}

/******************************************************************
 *
 ******************************************************************/
UnmapMemObjectCommand::UnmapMemObjectCommand(IOclCommandQueueBase* cmdQueue, ocl_entry_points* pOclEntryPoints, 
											 MemoryObject* pMemObject, void* pMappedPtr):
	Command(cmdQueue, pOclEntryPoints),
    m_pMemObject(pMemObject, MemoryObject::READ_WRITE),
    m_pMappedPtr(pMappedPtr),
    m_pActualMappingDevice(NULL),
    m_ExecutionType( DEVICE_EXECUTION_TYPE ),
	m_pPrefixCommand(NULL),
	m_pOclEntryPoints(pOclEntryPoints),
	m_bResourcesAllocated(false)
{
}

/******************************************************************
 *
 ******************************************************************/
UnmapMemObjectCommand::~UnmapMemObjectCommand()
{
    if (m_bResourcesAllocated)
    {
        // Init was done, but execute was not called
        if (NULL != m_pPrefixCommand)
		{
            delete m_pPrefixCommand;
        }
        
        m_pMemObject.pMemObj->UndoMappedRegionInvalidation(m_pMappedRegion);
    }
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code UnmapMemObjectCommand::Init()
{
    const FissionableDevice* actual_dev;
    MemoryObject* pMemObj = m_pMemObject.pMemObj;
    
    /* First check the the region has been mapped - just get the 1st mapped region, the user should
        handle code with multiple map/unmap commands */
    bool bDiscardPreviousData = false;
    cl_err_code err = pMemObj->GetMappedRegionInfo(m_pDevice, m_pMappedPtr, 
                                                   &m_pMappedRegion, &actual_dev, &bDiscardPreviousData, true);

    if (CL_FAILED(err))
    {
        return err;
    }

    if (bDiscardPreviousData)
    {
        m_pMemObject.access_rights = MemoryObject::WRITE_ENTIRE;
    }
   
    m_pActualMappingDevice = const_cast<FissionableDevice*>(actual_dev);

	// check whether postfix command should be run to update user mirror area
	if ((0 != ((CL_MAP_WRITE|CL_MAP_WRITE_INVALIDATE_REGION) & m_pMappedRegion->flags)) && // region was mapped for writing on host 
		pMemObj->IsSynchDataWithHostRequired( m_pMappedRegion, m_pMappedPtr ))
	{
		m_pPrefixCommand = new PrePostFixRuntimeCommand( this, PrePostFixRuntimeCommand::PREFIX_MODE, GetCommandQueue(), m_pOclEntryPoints );

		if (NULL != m_pPrefixCommand)
		{
			err = m_pPrefixCommand->Init();
			if ( CL_FAILED(err) )
			{
				delete m_pPrefixCommand;
				m_pPrefixCommand = NULL;
			}
		}
		else
		{
			err = CL_OUT_OF_HOST_MEMORY;
		}

		if (NULL == m_pPrefixCommand)
		{
            pMemObj->UndoMappedRegionInvalidation(m_pMappedRegion);
            
			assert(0);
			return err;
		}
	}

	// Initialize GPA data
	GPA_InitCommand();

    m_bResourcesAllocated = true;
	return err;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code UnmapMemObjectCommand::Execute()
{
    assert( (RUNTIME_EXECUTION_TYPE != m_ExecutionType) && "UnmapMemObjectCommand::Execute() called after it was switch to RUNTIME mode" );

    if (RUNTIME_EXECUTION_TYPE == m_ExecutionType)
    {
        return CL_NOT_READY;
    }
    
    if (CL_NOT_READY == AcquireMemoryObjects( m_pMemObject, m_pActualMappingDevice))
    {
        return CL_NOT_READY;
    }
        
	cl_dev_cmd_desc *m_pDevCmd = &m_DevCmd;

    // Create and send unmap command
    prepare_command_descriptor( CL_DEV_CMD_UNMAP, m_pMappedRegion, sizeof(cl_dev_cmd_param_map));

    // Color will be changed only when command is submitted in the device
    LogDebugA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), m_Event.GetId());

    // we need to synchronize backing store data with host map data, but this
    // should be done immediately before command execution on device to support
    // native kernels that may access mapped object data
    // In order to do this we override Command::NotifyCmdStatusChanged(CL_RUNNING)
    // which is called by device immediately before execution start.
    
    cl_dev_cmd_list device_cmd_list = m_clDevCmdListId;
    if (m_pActualMappingDevice != m_pDevice)
    {
        // cross device mode
        m_ExecutionType = RUNTIME_EXECUTION_TYPE;
        // use hidden queue if map to another device 
        device_cmd_list = NULL;
        // ensure we will exist after device will call for completion so that runtime scheduler will see us
        m_Event.AddPendency(this);
    }
        
	cl_dev_err_code errDev = m_pActualMappingDevice->GetDeviceAgent()->clDevCommandListExecute(
                                    	device_cmd_list,
                                        &m_pDevCmd, 1);
	
    cl_err_code err = CL_DEV_SUCCEEDED(errDev) ? CL_SUCCESS : CL_OUT_OF_RESOURCES;

    if ((CL_SUCCESS == err) && (RUNTIME_EXECUTION_TYPE == m_ExecutionType))
    {
        // do not allow command batching until command completion
        err = CL_NOT_READY;
    }

    return err;    
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code	UnmapMemObjectCommand::PrefixExecute()
{
	cl_err_code err;

#if defined(USE_GPA)
	
	ocl_gpa_data* pGPAData = m_pCommandQueue->GetGPAData();

	if ((NULL != pGPAData) && (pGPAData->bUseGPA))
	{
		cl_mem_obj_descriptor*	pMemObj;
		size_t regionInBytes[MAX_WORK_DIM] = {m_pMappedRegion->region[0],m_pMappedRegion->region[1],m_pMappedRegion->region[2]};	

		// Calculate each region size in bytes
		m_pMappedRegion->memObj->clDevMemObjGetDescriptor(CL_DEVICE_TYPE_CPU, 0, (cl_dev_memobj_handle*)&pMemObj);
		for (unsigned int i = 0 ; i < m_pMappedRegion->dim_count ; ++i)
		{
			regionInBytes[i] *= pMemObj->uiElementSize;
		}

		// Start Sync Data GPA task
		__itt_set_track(NULL);
		__itt_task_begin(pGPAData->pDeviceDomain, __itt_null, __itt_null, pGPAData->pSyncDataHandle);
		TAL_SetNamedTaskColor("Sync Data", 255, 0, 0);

		// Add region metadata to the Sync Data task
		switch(m_pMappedRegion->dim_count)
		{
		case 3:
			__itt_metadata_add(pGPAData->pDeviceDomain, __itt_null, pGPAData->pDepthHandle, ITT_SIZE_T_METADATA_TYPE, 1, &regionInBytes[2]);
		case 2:
			__itt_metadata_add(pGPAData->pDeviceDomain, __itt_null, pGPAData->pHeightHandle, ITT_SIZE_T_METADATA_TYPE, 1, &regionInBytes[1]);
		case 1:
			__itt_metadata_add(pGPAData->pDeviceDomain, __itt_null, (m_pMappedRegion->dim_count > 1) ? pGPAData->pWidthHandle : pGPAData->pSizeHandle, ITT_SIZE_T_METADATA_TYPE, 1, &regionInBytes[0]);
		}
	}
#endif

	err = m_pMemObject.pMemObj->SynchDataFromHost( m_pMappedRegion, m_pMappedPtr );

#if defined(USE_GPA)
	if ((NULL != pGPAData) && (pGPAData->bUseGPA))
	{
		// End Sync Data GPA task

		__itt_set_track(NULL);
		__itt_task_end(pGPAData->pDeviceDomain);
	} 
#endif

	return err;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code UnmapMemObjectCommand::CommandDone()
{
    cl_err_code errVal;

	if (m_pPrefixCommand)
	{
		m_pPrefixCommand->ErrorDone();
		m_pPrefixCommand = NULL;
	}

    // Here we do the actual operation off releasing the mapped region.
    errVal = m_pMemObject.pMemObj->ReleaseMappedRegion(m_pMappedRegion, m_pMappedPtr, true);
    m_pMappedRegion = NULL;

    RelinquishMemoryObjects(m_pMemObject, m_pActualMappingDevice);
    m_pMemObject.pMemObj->RemovePendency(NULL); // NULL because this command wasn't the one that added the dependency in the first place - it was the map command

    m_bResourcesAllocated = false;
    return errVal;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code UnmapMemObjectCommand::EnqueueSelf(cl_bool bBlocking, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent)
{
	cl_err_code err;

	if (m_pPrefixCommand)
	{
		cl_event 			  intermediate_pEvent;
		EventsManager*	event_manager = GetCommandQueue()->GetEventsManager();

		err = m_pPrefixCommand->EnqueueSelf( bBlocking, uNumEventsInWaitList, cpEeventWaitList, &intermediate_pEvent );
		if (CL_FAILED(err))
		{
			return err;
		}

		// prefix starts it own life
		m_pPrefixCommand = NULL;

		// 'this' may disapper during Enqueue if it was successful!
		err = Command::EnqueueSelf( bBlocking, 1, &intermediate_pEvent, pEvent );

		// release intermediate event
		event_manager->ReleaseEvent( intermediate_pEvent );
	}
	else
	{
		// 'this' may disapper during Enqueue if it was successful!
		err = Command::EnqueueSelf( bBlocking, uNumEventsInWaitList, cpEeventWaitList, pEvent );
	}

	return err;
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

        m_MemOclObjects.push_back( MemoryObjectArg( pMemObj, MemoryObject::READ_WRITE ));
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
	cl_dev_cmd_param_native* pNativeKernelParam = &m_nativeParams;

    pNativeKernelParam->args     = m_szCbArgs;
    pNativeKernelParam->argv     = pNewArgs;
    pNativeKernelParam->func_ptr = m_pUserFnc;
    pNativeKernelParam->mem_num  = m_uNumMemObjects;
    pNativeKernelParam->mem_offset = ppNewArgsOffset;

    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code NativeKernelCommand::Execute()
{
    if (CL_NOT_READY == AcquireMemoryObjects(m_MemOclObjects))
    {
        return CL_NOT_READY;
    }
    
    LogDebugA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), m_Event.GetId());

	// Fill command descriptor
	cl_dev_cmd_desc *m_pDevCmd = &m_DevCmd;
    prepare_command_descriptor( CL_DEV_CMD_EXEC_NATIVE, &m_nativeParams, sizeof(cl_dev_cmd_param_native));

	return m_pDevice->GetDeviceAgent()->clDevCommandListExecute(m_clDevCmdListId, &m_pDevCmd, 1);
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code NativeKernelCommand::CommandDone()
{
    // Clean resources

	//Can be null of out of memory encountered during init
	if (NULL != m_nativeParams.argv)
    {
		char* temp = (char*)m_nativeParams.argv;
		delete[] temp;
	}
	if (NULL != m_nativeParams.mem_offset)
	{
		delete[] m_nativeParams.mem_offset;
    }

    RelinquishMemoryObjects(m_MemOclObjects);

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
            if (NULL != pMemObj)    // NULL argument is allowed
            {
                // Mark as used
                pMemObj->AddPendency(this);
                res = pMemObj->CreateDeviceResource(m_pDevice);
                if( CL_FAILED(res))
                {
                    break;
                }                
                m_MemOclObjects.push_back(MemoryObjectArg(pMemObj, MemoryObject::READ_WRITE));
            }
            szCurrentLocation += szSize;
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
	if (CL_NOT_READY == AcquireMemoryObjects(m_MemOclObjects) )
	{
        return CL_NOT_READY;
	}

	cl_dev_cmd_desc *m_pDevCmd = &m_DevCmd;
    // Fill command descriptor
    prepare_command_descriptor(CL_DEV_CMD_EXEC_KERNEL, &m_kernelParams, sizeof(cl_dev_cmd_param_kernel));

    cl_dev_cmd_param_kernel* pKernelParam = (cl_dev_cmd_param_kernel*)m_pDevCmd->params;
    pKernelParam->kernel = m_pKernel->GetDeviceKernelId(m_pDevice);

    // Color will be changed only when command is submitted in the device

    // Sending the queue command
    // TODO: Handle the case were buffers are located in different device.

    LogDebugA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), m_Event.GetId());

	return m_pDevice->GetDeviceAgent()->clDevCommandListExecute(m_clDevCmdListId, &m_pDevCmd, 1);
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code NDRangeKernelCommand::CommandDone()
{
    // Clear all resources
    // Remove object pendencies
    RelinquishMemoryObjects(m_MemOclObjects);

    MemoryObjectArgList::iterator mit;
    for( mit = m_MemOclObjects.begin(); mit != m_MemOclObjects.end(); mit++)
    {
        OCLObject<_cl_mem_int>* obj = mit->pMemObj;
        obj->RemovePendency(this);
    }
    m_MemOclObjects.clear();

    list<OCLObject<_cl_mem_int>*>::iterator it;
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
	cl_dev_cmd_desc *m_pDevCmd = &m_DevCmd;

    if (CL_NOT_READY == AcquireMemoryObjects( m_pMemObj, MemoryObject::READ_ONLY ))
    {
        return CL_NOT_READY;
    }

	OclEvent* pObjEvent;
	cl_err_code clErr = m_pMemObj->GetDeviceDescriptor(m_pDevice, &m_rwParams.memObj, &pObjEvent);
	if ( CL_FAILED(clErr) )
	{
		return clErr;
	}

    create_dev_cmd_rw(
		m_commandType == CL_COMMAND_READ_BUFFER_RECT ? MAX_WORK_DIM  : m_pMemObj->GetNumDimensions(),
		m_pDst, m_szOrigin, m_szDstOrigin, m_szRegion, m_szDstRowPitch, m_szDstSlicePitch, m_szMemObjRowPitch, m_szMemObjSlicePitch,
        CL_DEV_CMD_READ );

    LogDebugA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), m_Event.GetId());
    // Sending 1 command to the device where the buffer is located now

	return m_pDevice->GetDeviceAgent()->clDevCommandListExecute(m_clDevCmdListId, &m_pDevCmd, 1);

}

/******************************************************************
 *
 ******************************************************************/
cl_err_code ReadMemObjCommand::CommandDone()
{
    RelinquishMemoryObjects(m_pMemObj, MemoryObject::READ_ONLY);
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
 *  FillBufferCommand
 ******************************************************************/
FillBufferCommand::FillBufferCommand(IOclCommandQueueBase* cmdQueue, ocl_entry_points *pOclEntryPoints, MemoryObject *pBuffer, const void *pattern, size_t pattern_size, size_t offset, size_t size)
: FillMemObjCommand(cmdQueue, pOclEntryPoints, pBuffer, offset, size, pattern, pattern_size)
{
	m_commandType = CL_COMMAND_FILL_BUFFER;
}


FillBufferCommand::~FillBufferCommand()
{
}



/******************************************************************
 *  FillImageCommand
 ******************************************************************/
FillImageCommand::FillImageCommand( IOclCommandQueueBase* cmdQueue, ocl_entry_points *    pOclEntryPoints, MemoryObject*   pImg, const void *pattern, size_t pattern_size, const cl_uint num_of_dimms, const size_t *offset, const size_t *size)
: FillMemObjCommand(cmdQueue, pOclEntryPoints, pImg, offset, size, num_of_dimms, pattern, pattern_size)
{
	m_commandType = CL_COMMAND_FILL_IMAGE;
}

FillImageCommand::~FillImageCommand()
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
    m_pMemObj(pMemObj,MemoryObject::READ_WRITE),
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
    MemoryObject* pMemObj = m_pMemObj.pMemObj;
    
	// If we are blocking command, we need to allocate internal buffer
	if ( m_bBlocking )
	{
		SMemCpyParams sCpyParam;
		// We need to allocate only the amount to being copied
		size_t sizeToAlloc = pMemObj->GetPixelSize();
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
		sCpyParam.vRegion[0] *= pMemObj->GetPixelSize();
		sCpyParam.uiDimCount = m_commandType == CL_COMMAND_WRITE_BUFFER_RECT ? MAX_WORK_DIM  : pMemObj->GetNumDimensions();
		sCpyParam.pDst = (cl_char*)m_pTempBuffer;
		sCpyParam.vDstPitch[0] = sCpyParam.vRegion[0];
		sCpyParam.vDstPitch[1] = sCpyParam.vDstPitch[0] * m_szRegion[1];
		sCpyParam.pSrc = (cl_char*)m_cpSrc + m_szSrcOrigin[0]*pMemObj->GetPixelSize() + m_szSrcOrigin[1]*m_szSrcRowPitch + m_szSrcOrigin[2]*m_szSrcSlicePitch;
		sCpyParam.vSrcPitch[0] = m_szSrcRowPitch;
		sCpyParam.vSrcPitch[1] = m_szSrcSlicePitch;

		clCopyMemoryRegion(&sCpyParam);

		// Need to update source origin and pitch, now we will write from temporary buffer
		m_szSrcRowPitch = sCpyParam.vDstPitch[0];
		m_szSrcSlicePitch = sCpyParam.vDstPitch[1];
		memset(m_szSrcOrigin, 0, sizeof(m_szSrcOrigin));
	}

    // Allocate
    cl_err_code res = pMemObj->CreateDeviceResource(m_pDevice);
    if( CL_FAILED(res))
    {
        return res;
    }

	// Initialize GPA data
	GPA_InitCommand();

    pMemObj->AddPendency(this);

    if (pMemObj->IsWholeObjectCovered(
                            CL_COMMAND_WRITE_BUFFER_RECT ? MAX_WORK_DIM  : m_pMemObj.pMemObj->GetNumDimensions(),
                            m_szOrigin, m_szRegion))
    {
        m_pMemObj.access_rights = MemoryObject::WRITE_ENTIRE;
    }
    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code WriteMemObjCommand::Execute()
{  
    if (CL_NOT_READY == AcquireMemoryObjects( m_pMemObj ))
    {
        return CL_NOT_READY;
    }

    MemoryObject* pMemObj = m_pMemObj.pMemObj;
 	cl_dev_cmd_desc *m_pDevCmd = &m_DevCmd;

	/// memory object resides on target device, update it
	OclEvent* pObjEvent;
	cl_err_code clErr = pMemObj->GetDeviceDescriptor(m_pDevice, &m_rwParams.memObj, &pObjEvent);
	if ( CL_FAILED(clErr) )
	{
		return clErr;
	}

	create_dev_cmd_rw(
			m_commandType == CL_COMMAND_WRITE_BUFFER_RECT ? MAX_WORK_DIM  : pMemObj->GetNumDimensions(),
			m_bBlocking ? m_pTempBuffer : (void*)m_cpSrc,
			m_szOrigin, m_szSrcOrigin, m_szRegion, m_szSrcRowPitch, m_szSrcSlicePitch, m_szMemObjRowPitch, m_szMemObjSlicePitch,
			CL_DEV_CMD_WRITE ) ;

	LogDebugA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), m_Event.GetId());
	// Sending 1 command to the device where the buffer is located now

	m_Event.AddPendency(this);
	pMemObj->AddPendency(this);
	cl_dev_err_code errDev = m_pDevice->GetDeviceAgent()->clDevCommandListExecute(m_clDevCmdListId, &m_pDevCmd, 1);
	pMemObj->RemovePendency(this);
	m_Event.RemovePendency(this);
	return CL_DEV_SUCCEEDED(errDev) ? CL_SUCCESS : CL_OUT_OF_RESOURCES;
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

    RelinquishMemoryObjects(m_pMemObj);
    m_pMemObj.pMemObj->RemovePendency(this);

    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code RuntimeCommand::Execute()
{
	m_returnCode = 0;
    LogDebugA("Command - DONE  : %s (Id: %d)", GetCommandName(), m_Event.GetId());
    CommandDone();
	m_Event.SetEventState(EVENT_STATE_DONE);
	m_Event.RemovePendency(NULL);
    return CL_SUCCESS;
}


/******************************************************************
 *
 ******************************************************************/
FillMemObjCommand::FillMemObjCommand(
		IOclCommandQueueBase* cmdQueue,
		ocl_entry_points *    pOclEntryPoints,
		MemoryObject*   pMemObj,
		const size_t*   pszOffset,
		const size_t*   pszRegion,
		const cl_uint   numOfDimms,
		const void*     pattern,
		const size_t    pattern_size
	) :
	Command(cmdQueue, pOclEntryPoints),
	m_numOfDimms(numOfDimms), m_pattern_size(pattern_size),
	m_internalErr(CL_SUCCESS)
{
	m_commandType = CL_DEV_CMD_FILL_IMAGE;

	// Set region
    for( int i=0 ; i<MAX_WORK_DIM ; ++i)
    {
		m_szOffset[i] = pszOffset[i];
		m_szRegion[i] = pszRegion[i];
    }

    memcpy(m_pattern, pattern, m_pattern_size);
    m_pMemObj.pMemObj = pMemObj;
    pMemObj->AddPendency(this);
}

FillMemObjCommand::FillMemObjCommand(
            IOclCommandQueueBase* cmdQueue,
            ocl_entry_points *    pOclEntryPoints,
            MemoryObject*   pMemObj,
            const size_t    pszOffset,
            const size_t    pszRegion,
            const void*     pattern,
            const size_t    pattern_size
        ) :
        Command(cmdQueue, pOclEntryPoints),
        m_numOfDimms(1), m_pattern_size(pattern_size),
        m_internalErr(CL_SUCCESS)
{
	m_commandType = CL_DEV_CMD_FILL_BUFFER;

	// Set region
	m_szOffset[0] = pszOffset;
	m_szRegion[0] = pszRegion;

	memcpy(m_pattern, pattern, m_pattern_size);
	m_pMemObj.pMemObj = pMemObj;
	pMemObj->AddPendency(this);
}

/******************************************************************
 *
 ******************************************************************/
FillMemObjCommand::~FillMemObjCommand()
{
	m_pMemObj.pMemObj->RemovePendency(this);
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code FillMemObjCommand::Init()
{
	if (CL_SUCCESS != m_internalErr)
	{
		return m_internalErr;
	}

    // Allocate
    m_internalErr = m_pMemObj.pMemObj->CreateDeviceResource(m_pDevice);
    if( CL_FAILED(m_internalErr) )
    {
        return m_internalErr;
    }

	m_pMemObj.access_rights = (m_pMemObj.pMemObj->IsWholeObjectCovered( m_numOfDimms, m_szOffset, m_szRegion )) ?
                                            MemoryObject::WRITE_ENTIRE : MemoryObject::READ_WRITE;

	// Initialize GPA data
	GPA_InitCommand();
    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code FillMemObjCommand::Execute()
{
	if (CL_SUCCESS != m_internalErr)
	{
		return m_internalErr;
	}

	/// memory object resides on target device, update it
	OclEvent* pObjEvent;
    MemoryObject* pMemObj = m_pMemObj.pMemObj;
	cl_err_code clErr = pMemObj->GetDeviceDescriptor(m_pDevice, &(m_fillCmdParams.memObj), &pObjEvent);
	if ( CL_FAILED(clErr) )
	{
		return clErr;
	}

    if (CL_NOT_READY == AcquireMemoryObjects( m_pMemObj ))
    {
        return CL_NOT_READY;
    }

	m_fillCmdParams.dim_count    = m_numOfDimms;
    for( int i=0 ; i<MAX_WORK_DIM ; ++i)
    {
    	m_fillCmdParams.offset[i] = m_szOffset[i];
    	m_fillCmdParams.region[i] = m_szRegion[i];
    }
    // No need to copy m_pattern's content, since we are still in the host.
    assert(MAX_PATTERN_SIZE	>= m_pattern_size && "Trying to use a pattern larger than possible");
	m_fillCmdParams.pattern_size = m_pattern_size;
    memcpy(m_fillCmdParams.pattern, m_pattern, m_fillCmdParams.pattern_size);

	//FillMemObject::Create(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, ITaskBase* *pTask)
	prepare_command_descriptor((m_commandType == CL_DEV_CMD_FILL_BUFFER) ? CL_DEV_CMD_FILL_BUFFER : CL_DEV_CMD_FILL_IMAGE, 
	                            &m_fillCmdParams, sizeof(cl_dev_cmd_param_fill));

	LogDebugA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), m_Event.GetId());

	// Sending 1 command to the device where the buffer is located now
	cl_dev_cmd_desc* cmdPList[1] = {&m_DevCmd};

	m_Event.AddPendency(this);
	pMemObj->AddPendency(this);
	cl_dev_err_code errDev = m_pDevice->GetDeviceAgent()->clDevCommandListExecute(m_clDevCmdListId, cmdPList, 1);
	pMemObj->RemovePendency(this);
	m_Event.RemovePendency(this);

	return CL_DEV_SUCCEEDED(errDev) ? CL_SUCCESS : CL_OUT_OF_RESOURCES;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code FillMemObjCommand::CommandDone()
{
    RelinquishMemoryObjects(m_pMemObj);
    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
MigrateMemObjCommand::MigrateMemObjCommand(
		IOclCommandQueueBase*  cmdQueue,
		ocl_entry_points *     pOclEntryPoints,
		ContextModule*         pContextModule,
		cl_mem_migration_flags clFlags,
        cl_uint                uNumMemObjects,
        const cl_mem*          pMemObjects
	): 
	Command(cmdQueue, pOclEntryPoints),
    m_pMemObjects(pMemObjects), m_pContextModule( pContextModule )
{
    assert( 0 != uNumMemObjects );
    assert( NULL != pMemObjects );
    assert( NULL != pContextModule );

    memset( &m_migrateCmdParams, 0, sizeof(cl_dev_cmd_param_migrate));
    m_migrateCmdParams.flags    = clFlags;
    m_migrateCmdParams.mem_num  = uNumMemObjects;
}

/******************************************************************
 *
 ******************************************************************/
MigrateMemObjCommand::~MigrateMemObjCommand()
{
    MemoryObjectArgList::iterator it     = m_MemObjects.begin();
    MemoryObjectArgList::iterator it_end = m_MemObjects.end();

    for (; it != it_end; ++it)
    {
        MemoryObject* cur = it->pMemObj;
        cur->RemovePendency(this);
    }

    if (NULL != m_migrateCmdParams.memObjs)
    {
        delete [] m_migrateCmdParams.memObjs;
        m_migrateCmdParams.memObjs = NULL;
    }
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code MigrateMemObjCommand::Init()
{
    // Allocate
    m_migrateCmdParams.memObjs = new IOCLDevMemoryObject*[m_migrateCmdParams.mem_num];

    if (NULL == m_migrateCmdParams.memObjs)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    MemoryObject::MemObjUsage access = (0 != (m_migrateCmdParams.flags & CL_MIGRATE_MEM_OBJECT_CONTENT_UNDEFINED)) ?
                            MemoryObject::WRITE_ENTIRE : MemoryObject::READ_ONLY; // use READ_ONLY for optimization
    
    Context* const pQueueContext = m_pContextModule->GetContext(m_pCommandQueue->GetContextHandle());
    for (cl_uint i = 0; i < m_migrateCmdParams.mem_num; i++)
    {
        MemoryObject* pMemObj = m_pContextModule->GetMemoryObject(m_pMemObjects[i]);
        if (NULL == pMemObj)
        {
            return CL_INVALID_MEM_OBJECT;
        }
        if (pMemObj->GetContext() != pQueueContext)
        {
            return CL_INVALID_CONTEXT;
        }

        cl_err_code cl_err = pMemObj->CreateDeviceResource(m_pDevice);
        if (CL_FAILED(cl_err))
        {
            return cl_err;
        }
        
        cl_err = GetMemObjectDescriptor( pMemObj, &( m_migrateCmdParams.memObjs[i]));
        assert( CL_SUCCESS == cl_err );

        pMemObj->AddPendency(this);
        m_MemObjects.push_back( MemoryObjectArg(pMemObj, access ));
    }
    
	// Initialize GPA data
	GPA_InitCommand();
    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code MigrateMemObjCommand::Execute()
{
    if (CL_NOT_READY == AcquireMemoryObjects( m_MemObjects ))
    {
        return CL_NOT_READY;
    }

	prepare_command_descriptor(CL_DEV_CMD_MIGRATE, &m_migrateCmdParams, sizeof(cl_dev_cmd_param_migrate));

	LogDebugA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), m_Event.GetId());

	// Sending 1 command to the target device
	cl_dev_cmd_desc* cmdPList[1] = {&m_DevCmd};
	return m_pDevice->GetDeviceAgent()->clDevCommandListExecute(m_clDevCmdListId, cmdPList, 1);
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code MigrateMemObjCommand::CommandDone()
{
    RelinquishMemoryObjects(m_MemObjects);
    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code ErrorQueueEvent::ObservedEventStateChanged(OclEvent* pEvent, cl_int returnCode)
{
	return m_owner->GetEvent()->ObservedEventStateChanged( pEvent, m_owner->GetForcedErrorCode() ); 
}

/******************************************************************
 *
 ******************************************************************/
cl_context ErrorQueueEvent::GetContextHandle() const
{
	return m_owner->GetEvent()->GetContextHandle(); 
}

/******************************************************************
 *
 ******************************************************************/
cl_int     ErrorQueueEvent::GetReturnCode() const
{
	return m_owner->GetForcedErrorCode();
}

cl_err_code	ErrorQueueEvent::GetInfo(cl_int iParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet) const
{
	return m_owner->GetEvent()->GetInfo( iParamName, szParamValueSize, pParamValue, pszParamValueSizeRet );
}

/******************************************************************
 *
 ******************************************************************/
PrePostFixRuntimeCommand::PrePostFixRuntimeCommand( 
	Command* relatedUserCommand, Mode working_mode, IOclCommandQueueBase* cmdQueue, ocl_entry_points * pOclEntryPoints ): 
	RuntimeCommand(cmdQueue, pOclEntryPoints), 
	m_relatedUserCommand(relatedUserCommand), 
	m_working_mode( working_mode ),
	m_force_error_return(CL_SUCCESS)
{
	assert( NULL != m_relatedUserCommand );
	
	m_error_event.Init( this );
	m_error_event.AddPendency( this ); // ensure event will never be deleted externally

	m_task.Init( this );
}


/******************************************************************
 *
 ******************************************************************/
cl_err_code PrePostFixRuntimeCommand::Init()
{
	// related command should not disapper before I finished
	m_relatedUserCommand->GetEvent()->AddPendency( this );

	// Initialize GPA data
	GPA_InitCommand();
	return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code PrePostFixRuntimeCommand::CommandDone()
{
	QueueEvent* related_event = m_relatedUserCommand->GetEvent();

	// update times here
	if ( PREFIX_MODE == m_working_mode )
	{
		related_event->IncludeProfilingInfo( &m_Event );
	}
	else
	{
		m_Event.IncludeProfilingInfo( related_event );
	}

	related_event->RemovePendency( this );
	
	m_error_event.RemovePendency(this);

    LogDebugA("Command - DONE  : PrePostFixRuntimeCommand for %s (Id: %d)", GetCommandName(), m_Event.GetId());
	return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
void PrePostFixRuntimeCommand::ErrorDone()
{
	CommandDone();
	m_Event.RemovePendency(NULL);
}

/******************************************************************
 *
 ******************************************************************/
void PrePostFixRuntimeCommand::ErrorEnqueue(cl_event* intermediate_pEvent, cl_event* user_pEvent, cl_err_code err_to_force_return )
{
	m_force_error_return = err_to_force_return;

	// add manually and leave postfix floating
	EventsManager*	event_manager = GetCommandQueue()->GetEventsManager();

	event_manager->RegisterQueueEvent( &m_Event, user_pEvent );

	// 'this' may disapper right now
	cl_err_code err;
	err = event_manager->RegisterEvents( &m_error_event, 1, intermediate_pEvent );

	// in our case RegisterEvents() cannot return failure by construction
	assert( CL_SUCCEEDED( err ));
}

/******************************************************************
 *
 ******************************************************************/
void PrePostFixRuntimeCommand::DoAction()
{
	LogDebugA("PrePostFixRuntimeCommand - DoAction Started: PrePostFixRuntimeCommand for %s (Id: %d)", GetCommandName(), m_Event.GetId());

	m_returnCode = CL_SUCCESS;

	NotifyCmdStatusChanged(0, CL_RUNNING,  m_returnCode, Intel::OpenCL::Utils::HostTime());

	m_returnCode = ( PREFIX_MODE == m_working_mode ) ?
						m_relatedUserCommand->PrefixExecute() 
						:
						m_relatedUserCommand->PostfixExecute();

	NotifyCmdStatusChanged(0, CL_COMPLETE, m_returnCode, Intel::OpenCL::Utils::HostTime());

	LogDebugA("PrePostFixRuntimeCommand - DoAction Finished: PrePostFixRuntimeCommand for %s (Id: %d)", GetCommandName(), m_Event.GetId());
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code PrePostFixRuntimeCommand::Execute()
{
	cl_err_code			ret  = CL_SUCCESS;

	// prevent deletion of the command until task is deleted by TaskExecutor
	m_Event.AddPendency( this );
	unsigned int task_err = TaskExecutor::GetTaskExecutor()->Execute(&m_task);

	if (0 != task_err)
	{
		m_Event.RemovePendency( this );
		LogDebugA("PrePostFixRuntimeCommand - Execute: Task submission failed for PrePostFixRuntimeCommand for %s (Id: %d)", GetCommandName(), m_Event.GetId());

		ret = CL_OUT_OF_RESOURCES;
		m_returnCode = ret;	
		NotifyCmdStatusChanged(0, CL_COMPLETE, ret, Intel::OpenCL::Utils::HostTime());
	}

    return ret;
}

/******************************************************************
 *
 ******************************************************************/
bool RuntimeCommandTask::Execute()
{
	m_owner->DoAction();
	return true;
}

/******************************************************************
 *
 ******************************************************************/
long RuntimeCommandTask::Release() 
{
	m_owner->GetEvent()->RemovePendency(m_owner);
    return 0;
}
