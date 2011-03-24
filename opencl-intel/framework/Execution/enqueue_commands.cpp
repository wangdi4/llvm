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
#include "cl_memory_object.h"
#include "command_queue.h"
#include "kernel.h"
#include "sampler.h"
#include "events_manager.h"
#include "cl_sys_defines.h"
//For debug
#include <stdio.h>
#if defined (_WIN32)
#include <windows.h>
#include <process.h>
#else
#include <malloc.h>
#endif
#include <assert.h>
#include <cl_buffer.h>
#include "execution_module.h"
#if defined(USE_GPA)  
	#include <ittnotify.h>
	//#include "tal\tal.h"
#endif
using namespace Intel::OpenCL::Framework;

/******************************************************************
 * Static function to be used by all commands that need to write/read data
 ******************************************************************/
static void create_dev_cmd_rw(
    cl_dev_mem          clDevMemHndl,
    cl_mem_object_type  clMemObjType,
    void*               pPtr,
    const size_t*       pszMemOrigin,
	const size_t*       pszPtrOrigin,    
    const size_t*       pszRegion,
    size_t              szRowPitch,
    size_t              szSlicePitch,	
    size_t              szMemRowPitch,
    size_t              szMemSlicePitch,	
    cl_dev_cmd_type     clCmdType,
    cl_dev_cmd_id       clCmdId,
	cl_dev_cmd_desc*     pDevCmd,
	cl_dev_cmd_param_rw* pRWParams
    )
{
        // Create Read command
        pRWParams->memObj = clDevMemHndl;
        pRWParams->ptr = pPtr;

        cl_uint uiDimCount = 0;
        switch(clMemObjType)
        {
        case CL_MEM_OBJECT_BUFFER:
            uiDimCount = 1;
            break;
        case CL_MEM_OBJECT_IMAGE2D:
            uiDimCount = 2;
            break;
        case CL_MEM_OBJECT_IMAGE3D:
            uiDimCount = 3;
            break;
        default:
            break;
        }

        cl_uint i;
        for( i=0; i<MAX_WORK_DIM; i++ )
        {
            pRWParams->origin[i] = pszMemOrigin[i];
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
		
        pRWParams->pitch[0] = szRowPitch;
        pRWParams->pitch[1] = szSlicePitch;

		pRWParams->memobj_pitch[0] = szMemRowPitch;
        pRWParams->memobj_pitch[1] = szMemSlicePitch;

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
    m_Event(cmdQueue, pOclEntryPoints),
    m_clDevCmdListId(0),
	m_pDevice(NULL),
	m_pCommandQueue(cmdQueue),
	m_returnCode(1)
{
	memset(&m_DevCmd, 0, sizeof(cl_dev_cmd_desc));

	m_iId = m_Event.GetId();
	m_Event.AddPendency();
	m_Event.SetCommand(this);

	assert(m_pCommandQueue);
	m_pCommandQueue->AddPendency();
	INIT_LOGGER_CLIENT(L"Command Logger Client",LL_DEBUG);
}

/******************************************************************
 *
 ******************************************************************/
Command::~Command()
{
	m_pDevice = NULL;
	m_pCommandQueue->RemovePendency();
	m_pCommandQueue = NULL;

	RELEASE_LOGGER_CLIENT;
}

/******************************************************************
 *
 ******************************************************************/
//Todo: remove clCmdId param
cl_err_code Command::NotifyCmdStatusChanged(cl_dev_cmd_id clCmdId, cl_int iCmdStatus, cl_int iCompletionResult, cl_ulong ulTimer)
{
    cl_err_code res = CL_SUCCESS;
    switch(iCmdStatus)
    {
    case CL_QUEUED:
		// Nothing to do, not expected to be here at all
		break;
    case CL_SUBMITTED:
		m_Event.SetProfilingInfo(CL_PROFILING_COMMAND_SUBMIT, ulTimer);
        m_Event.SetColor(EVENT_STATE_LIME);
        LogDebugA("Command - SUBMITTED TO DEVICE  : %s (Id: %d)", GetCommandName(), m_iId);
        break;
    case CL_RUNNING:
        LogDebugA("Command - RUNNING  : %s (Id: %d)", GetCommandName(), m_iId);
		m_Event.SetProfilingInfo(CL_PROFILING_COMMAND_START, ulTimer);
        m_Event.SetColor(EVENT_STATE_GREEN);
        break;
    case CL_COMPLETE:
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
// finish marker
#if defined(USE_GPA)
		__itt_domain* domain;

		if (m_pCommandQueue->IsTaskalyzerEnabled())
		{
			__itt_string_handle* marker = __itt_string_handle_createA("Done");
			__itt_string_handle* pshCtor = __itt_string_handle_createA("Marker");
			
			domain = __itt_domain_createA("OpenCL.Domain.Global");
			assert(NULL != domain);

			//due to a bug in GPA 4.0 the marker is within a task 
			__itt_task_begin(domain, __itt_null, __itt_null, pshCtor);
			
			__itt_marker(domain, __itt_null, marker, __itt_marker_scope_global);

			__itt_task_end(domain);
		}
#endif
		m_Event.RemovePendency();
        break;
    default:        
        break;
    }
    return res;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code MemoryCommand::CopyToHost(						
						MemoryObject*	pSrcMemObj,					
						QueueEvent**		pEvent)
{
	cl_err_code res = CL_SUCCESS;

	cl_device_id srcDeviceId = pSrcMemObj->GetDataLocation(m_pDevice->GetHandle());

	Device* pSrcDevice;		
	Context *pContext = (Context*)pSrcMemObj->GetContext();				
	res = pContext->GetDevice(srcDeviceId, &pSrcDevice);
	if (CL_FAILED(res)) { return res; }		

	
	void* pData = pSrcMemObj->GetData(0);

	size_t origin[MAX_WORK_DIM] = {0};
	size_t region[MAX_WORK_DIM];
	size_t rowPitch, slicePitch;
	pSrcMemObj->GetLayout(region, &rowPitch, &slicePitch);
	
	MemoryCommand* pReadMemObjCmd = new ReadMemObjCommand(m_pCommandQueue, (ocl_entry_points*)((_cl_command_queue_int*)m_pCommandQueue->GetHandle())->dispatch, pSrcMemObj, origin, region, rowPitch, slicePitch, pData);		
	if (!pReadMemObjCmd)
	{
		return CL_OUT_OF_HOST_MEMORY;
	}
	pReadMemObjCmd->SetDevice(pSrcDevice);
	
	QueueEvent* pQueueEvent = pReadMemObjCmd->GetEvent();
	
	if (!pQueueEvent)
	{
		delete pReadMemObjCmd;
		return CL_OUT_OF_HOST_MEMORY;
	}

	res = pReadMemObjCmd->Init();
	if (CL_FAILED(res)) 
	{ 		
		return res; 
	}
		
	cl_dev_cmd_desc* pReadDevCmd =  &pReadMemObjCmd->m_DevCmd;		
	cl_dev_cmd_param_rw* pRWParams =  &pReadMemObjCmd->m_rwParams;
					
	// copy from host to device
	create_dev_cmd_rw(            
		pSrcMemObj->GetDeviceMemoryHndl(srcDeviceId), 
		pSrcMemObj->GetType(),
		pData, origin, NULL, region, 0, 0, 0, 0,
		CL_DEV_CMD_READ,
		(cl_dev_cmd_id)pQueueEvent->GetId(), 
		pReadDevCmd, 
		pRWParams);		

	pSrcMemObj->SetDataLocation(0);

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
						MemoryObject* pSrcMemObj, 
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
		cl_dev_cmd_desc *pDevCmd = &m_DevCmd;
		create_dev_cmd_rw(
				pSrcMemObj->GetDeviceMemoryHndl(m_pDevice->GetHandle()), 
				pSrcMemObj->GetType(),			
				(void*)pSrcData, pDstOrigin, pSrcOrigin, pRegion, szSrcRowPitch, szSrcSlicePitch, szDstRowPitch, szDstSlicePitch,
				CL_DEV_CMD_WRITE,
				(cl_dev_cmd_id)(*pEvent)->GetId(),
				pDevCmd,
				&m_rwParams) ;
		pSrcMemObj->SetDataLocation(m_pDevice->GetHandle());
		LogDebugA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), m_iId);
		// Sending 1 command to the device where the buffer is located now
		m_Event.SetEventQueue(m_pCommandQueue);
		pDevCmd->profiling = (m_pCommandQueue->IsProfilingEnabled() ? true : false );
		pDevCmd->data			= static_cast<ICmdStatusChangedObserver*>(this);
		res = m_pDevice->GetDeviceAgent()->clDevCommandListExecute(m_clDevCmdListId, &pDevCmd, 1);
	}
	else
	{		
		MemoryCommand* pWriteMemObjCmd = new WriteMemObjCommand(m_pCommandQueue, (ocl_entry_points*)(((_cl_command_queue_int*)m_pCommandQueue->GetHandle())->dispatch), pSrcMemObj, pDstOrigin, pRegion, szDstRowPitch,szDstSlicePitch,pSrcData,pSrcOrigin,szSrcRowPitch,szSrcSlicePitch);		
		if (!pWriteMemObjCmd)
		{
			return CL_OUT_OF_HOST_MEMORY;
		}
		pWriteMemObjCmd->SetDevice(m_pDevice);
		
		cl_event waitEvent = NULL;
		QueueEvent* pQueueEvent = pWriteMemObjCmd->GetEvent();
		m_pCommandQueue->GetEventsManager()->RegisterQueueEvent(pQueueEvent, &waitEvent);
		res = pWriteMemObjCmd->Init();
		if (CL_FAILED(res)) 
		{ 
			return res; 
		}
			
		cl_dev_cmd_desc* pWriteDevCmd =  &pWriteMemObjCmd->m_DevCmd;		
		cl_dev_cmd_param_rw* pRWParams = &pWriteMemObjCmd->m_rwParams;
					
		// copy from host to device
		create_dev_cmd_rw(            
			pSrcMemObj->GetDeviceMemoryHndl(m_pDevice->GetHandle()), 
			pSrcMemObj->GetType(),
			(void*)pSrcData, pDstOrigin, pSrcOrigin, pRegion, szSrcRowPitch, szSrcSlicePitch, szDstRowPitch, szDstSlicePitch,			
			CL_DEV_CMD_WRITE,
			(cl_dev_cmd_id)pQueueEvent->GetId(), 
			pWriteDevCmd, 
			pRWParams);

		// Set new location			
		pSrcMemObj->SetDataLocation(m_pDevice->GetHandle());

		pWriteDevCmd->data = static_cast<ICmdStatusChangedObserver*>(pWriteMemObjCmd);		
		res = m_pDevice->GetDeviceAgent()->clDevCommandListExecute(NULL, &pWriteDevCmd, 1);							
		if (CL_FAILED(res)) { return res; }							

		*pEvent = pQueueEvent;	
	}
	return res;
}

cl_err_code MemoryCommand::PrepareOnDevice(
						MemoryObject* pSrcMemObj, 						
						const size_t* pSrcOrigin, 						
						const size_t* pRegion,					
						QueueEvent**	pEvent)
{
	cl_err_code res = CL_SUCCESS;

	
	cl_device_id srcDeviceId = pSrcMemObj->GetDataLocation(m_pDevice->GetHandle());
	cl_device_id queueDeviceId = m_pCommandQueue->GetQueueDeviceHandle();	

	if (srcDeviceId != 0 && srcDeviceId != queueDeviceId)
	{				
		res = CopyToHost(pSrcMemObj, pEvent);				
	}	
	else if (srcDeviceId == 0 && pSrcMemObj->IsAllocated(0))
	{						
		void* pSrcData = pSrcMemObj->GetData(NULL);		

		size_t origin[MAX_WORK_DIM] = {0};
		size_t region[MAX_WORK_DIM];
		size_t rowPitch, slicePitch;
		pSrcMemObj->GetLayout(region, &rowPitch, &slicePitch);
		
		res = CopyFromHost(pSrcData, pSrcMemObj, origin, origin, region, rowPitch, slicePitch, 0, 0, pEvent);		
	}									
	return res;
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
    m_pSrcMemObj(pSrcMemObj),
    m_pDstMemObj(pDstMemObj),
	m_szSrcRowPitch(szSrcRowPitch),
	m_szSrcSlicePitch(szSrcSlicePitch),
	m_szDstRowPitch(szDstRowPitch),
	m_szDstSlicePitch(szDstSlicePitch)
{  
    //
    // Set source offsets
    //
    switch(pSrcMemObj->GetType())
    {
    case CL_MEM_OBJECT_BUFFER:
        m_uiSrcNumDims = 1;
        break;
    case CL_MEM_OBJECT_IMAGE2D:
        m_uiSrcNumDims = 2;
        break;
    case CL_MEM_OBJECT_IMAGE3D:
        m_uiSrcNumDims = 3;
        break;
    default:
		m_uiSrcNumDims = 0;
        break;
    }
    
    cl_uint i;

    for( i=0; i<MAX_WORK_DIM; i++ )
    {
        m_szSrcOrigin[i] = szSrcOrigin[i];        
    }

    //
    // Set dst offsets
    //
    switch(pDstMemObj->GetType())
    {
    case CL_MEM_OBJECT_BUFFER:
        m_uiDstNumDims = 1;
        break;
    case CL_MEM_OBJECT_IMAGE2D:
        m_uiDstNumDims = 2;
        break;
    case CL_MEM_OBJECT_IMAGE3D:
        m_uiDstNumDims = 3;
        break;
    default:
		m_uiDstNumDims = 0;
        break;
    }

    for( i=0; i<MAX_WORK_DIM; i++ )
    {
		m_szDstOrigin[i] = szDstOrigin[i];        
    }

    //
    // Set region, according to the bigger object, for exmple: 
    // If buffer->image it is image(2D/3D) dimensions.
    // If buffer->buffer it is 1 dimensions.
    //
    for( i=0; i<MAX_WORK_DIM; i++ )
    {
		m_szRegion[i] = szRegion[i];        
    }
	
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
	cl_device_id clDeviceId = m_pCommandQueue->GetQueueDeviceHandle();
	if (!m_pDstMemObj->IsAllocated(clDeviceId))
    {
        // Allocate
        cl_err_code res = m_pDstMemObj->CreateDeviceResource(clDeviceId);
        if( CL_FAILED(res))
        {
            return res;
        }
    }

    m_pSrcMemObj->AddPendency();
    m_pDstMemObj->AddPendency();
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
		m_Event.RemovePendency();
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
	
	cl_device_id clDeviceId = m_pCommandQueue->GetQueueDeviceHandle();
	cl_device_id clSrcMemObjLoc = m_pSrcMemObj->GetDataLocation(clDeviceId);
		
	bool bSrcOnDifferentDevice	= (clSrcMemObjLoc != 0 && clSrcMemObjLoc != clDeviceId);
	bool bSrcOnRuntime			= (clSrcMemObjLoc == 0 && m_pSrcMemObj->IsAllocated(0));
	bool bSrcOnTargetDevice		= (clSrcMemObjLoc == clDeviceId);

	if (bSrcOnDifferentDevice)
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
		void* pData = m_pSrcMemObj->GetData(NULL);    								
		QueueEvent* pEvent = GetEvent();
		res = CopyFromHost(pData, m_pDstMemObj, m_szSrcOrigin, m_szDstOrigin, m_szRegion, m_szSrcRowPitch, m_szSrcSlicePitch, m_szDstRowPitch, m_szDstSlicePitch, &pEvent);		
		if (CL_FAILED(res)) 
		{ 
			return res; 
		}		
		return CL_SUCCESS;
	}
	
	else if (bSrcOnTargetDevice)
	{			
		res = CopyOnDevice(clDeviceId);			
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
 * Use device copy command to copy betweem the buffers.
 * Pre condition for this function is that the 2 buffers are allocated
 * in the device.
 ******************************************************************/
cl_err_code CopyMemObjCommand::CopyOnDevice(cl_device_id clDeviceId)
{
    cl_dev_cmd_desc *m_pDevCmd = &m_DevCmd;
    cl_dev_cmd_param_copy* pCopyParams   = &m_copyParams;
    
    pCopyParams->srcMemObj      = m_pSrcMemObj->GetDeviceMemoryHndl(clDeviceId);
    pCopyParams->dstMemObj      = m_pDstMemObj->GetDeviceMemoryHndl(clDeviceId);
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
	
    m_pDstMemObj->SetDataLocation(clDeviceId);
	m_Event.SetEventQueue(m_pCommandQueue);
    // Sending 1 command to the device where the buffer is located now
    // Color will be changed only when command is submitted in the device    
    LogDebugA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), m_iId);

	return m_pDevice->GetDeviceAgent()->clDevCommandListExecute(m_clDevCmdListId, &m_pDevCmd, 1);
}



/******************************************************************
 * This function copies the data from the clSrcDeviceId device
 * to the dst buffer local memory.
 *
 * TODO: Add support images CopyToHost, current version valid for buffers only
 ******************************************************************/
cl_err_code CopyMemObjCommand::CommandDone()
{
    m_pSrcMemObj->RemovePendency();
    m_pDstMemObj->RemovePendency();

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
            ): CopyMemObjCommand(cmdQueue, pOclEntryPoints, pSrcBuffer, pDstBuffer, pszSrcOrigin, pszDstOrigin, pszRegion,szSrcRowPitch, szSrcSlicePitch, szDstRowPitch, szDstSlicePitch)
{
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
    m_origin  = szOffset;
    m_pOrigin = &m_origin;
    
	m_region = szCb;
    m_pRegion = &m_region;
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
    m_pMemObj(pMemObj),
    m_clMapFlags(clMapFlags),
    m_pOrigin(pOrigin),
    m_pRegion(pRegion),
    m_pszImageRowPitch(pszImageRowPitch),
    m_pszImageSlicePitch(pszImageSlicePitch)
{
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
    m_pMemObj->AddPendency();
	cl_device_id clDeviceId = m_pCommandQueue->GetQueueDeviceHandle();
    
    // First validate that bufer is allocated.
    if (!m_pMemObj->IsAllocated(clDeviceId))
    {
        // Allocate
        res = m_pMemObj->CreateDeviceResource(clDeviceId);
        if( CL_FAILED(res))
        {
			assert(0);
            return res;
        }
    }

    // Get pointer to the device
    m_pMappedRegion = m_pMemObj->CreateMappedRegion(clDeviceId, m_clMapFlags, m_pOrigin, m_pRegion, m_pszImageRowPitch, m_pszImageSlicePitch);
    if ( NULL == m_pMappedRegion )
    {
		assert(0);
        // Case of error
        return CL_MEM_OBJECT_ALLOCATION_FAILURE;
    }
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

	cl_device_id clDeviceId = m_pCommandQueue->GetQueueDeviceHandle();
	cl_dev_cmd_desc *m_pDevCmd = &m_DevCmd;
    // Prepare command. 
    // Anyhow we send the map command to the device though  we expect that on write
    // there is nothing to do, and on read the device may need to copy from device memory to host memory
    m_pDevCmd->id          = (cl_dev_cmd_id)m_Event.GetId();
    m_pDevCmd->type        = CL_DEV_CMD_MAP;
    m_pDevCmd->param_size  = sizeof(cl_dev_cmd_param_map);
    m_pDevCmd->params      = m_pMemObj->GetMappedRegionInfo(clDeviceId, m_pMappedRegion);

	m_pDevCmd->profiling = (m_pCommandQueue->IsProfilingEnabled() ? true : false );
	m_pDevCmd->data			= static_cast<ICmdStatusChangedObserver*>(this);

	m_Event.SetEventQueue(m_pCommandQueue);
	// Change status of the command to Gray before handle by the device
    // Color will be changed only when command is submitted in the device    
    LogDebugA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), m_iId);

	res = m_pDevice->GetDeviceAgent()->clDevCommandListExecute(m_clDevCmdListId, &m_pDevCmd, 1);			
	m_pMemObj->SetDataLocation(clDeviceId);
	return res;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code MapMemObjCommand::CommandDone()
{
    // Don't remove buffer pendency, the buffer should be alive at least until unmap is done.
    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
UnmapMemObjectCommand::UnmapMemObjectCommand(IOclCommandQueueBase* cmdQueue, ocl_entry_points* pOclEntryPoints, MemoryObject* pMemObject, void* pMappedRegion):
	Command(cmdQueue, pOclEntryPoints),
    m_pMemObject(pMemObject),
    m_pMappedRegion(pMappedRegion)
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
	cl_device_id clDeviceId = m_pCommandQueue->GetQueueDeviceHandle();
    void* pMappedRegionInfo = m_pMemObject->GetMappedRegionInfo(clDeviceId, m_pMappedRegion);
    if ( NULL == pMappedRegionInfo )
    {
        return CL_INVALID_VALUE;
    }
    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code UnmapMemObjectCommand::Execute()
{
	cl_device_id clDeviceId = m_pCommandQueue->GetQueueDeviceHandle();
	cl_dev_cmd_desc *m_pDevCmd = &m_DevCmd;

    // Create and send unmap command
    m_pDevCmd->id          = (cl_dev_cmd_id)m_Event.GetId();
    m_pDevCmd->type        = CL_DEV_CMD_UNMAP;
    m_pDevCmd->param_size  = sizeof(cl_dev_cmd_param_map);
    m_pDevCmd->params      = m_pMemObject->GetMappedRegionInfo(clDeviceId, m_pMappedRegion);
	m_pDevCmd->data			= static_cast<ICmdStatusChangedObserver*>(this);

    // Color will be changed only when command is submitted in the device    
    LogDebugA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), m_iId);

	m_pMemObject->SetDataLocation(clDeviceId);
	m_Event.SetEventQueue(m_pCommandQueue);
	m_pDevCmd->profiling = (m_pCommandQueue->IsProfilingEnabled() ? true : false );
	return m_pDevice->GetDeviceAgent()->clDevCommandListExecute(m_clDevCmdListId, &m_pDevCmd, 1);
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code UnmapMemObjectCommand::CommandDone()
{
    cl_device_id clDeviceId = m_pCommandQueue->GetQueueDeviceHandle();
    cl_err_code errVal;

    // Here we do the actual operation off releasing the mapped region.
    errVal = m_pMemObject->ReleaseMappedRegion(clDeviceId, m_pMappedRegion);
    m_pMemObject->RemovePendency();
    return errVal;
}

/******************************************************************
 *
 ******************************************************************/
NativeKernelCommand::NativeKernelCommand(
	IOclCommandQueueBase* cmdQueue, 
	ocl_entry_points *    pOclEntryPoints,
	void              (*pUserFnc)(void *), 
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
	// Actually they are pointers, so the size should be the same
	// Prefer #if with #pragma error, but doesn't pass compiler
	if (sizeof(cl_mem) != sizeof(cl_dev_mem))
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

	void** ppNewArgsMemLoc = NULL;
	if (m_uNumMemObjects > 0)
	{
		ppNewArgsMemLoc = new void*[m_uNumMemObjects];

		if(NULL == ppNewArgsMemLoc)
		{
			delete []pNewArgs;
			return CL_OUT_OF_HOST_MEMORY;
		}
	}

	cl_device_id clDeviceId = m_pCommandQueue->GetQueueDeviceHandle();
    cl_uint i;
	for( i=0; i < m_uNumMemObjects; i++ )
	{
		// Check that mem object is allocated on device, if not allocate resource
		MemoryObject* pMemObj = m_ppMemObjList[i];
		if (!(pMemObj->IsAllocated(clDeviceId)))
		{
			res = pMemObj->CreateDeviceResource(clDeviceId);
			if( CL_FAILED(res))
			{
				break;
			}
		}

		size_t stObjOffset = (size_t)((char*)(m_ppArgsMemLoc[i]) - (char*)m_pArgs);
	    void* pCurrentArgsLocation = pNewArgs + stObjOffset;

		// Set the new args list
		cl_dev_mem clDevMemHndl = pMemObj->GetDeviceMemoryHndl(clDeviceId);
		*((cl_dev_mem*)pCurrentArgsLocation) = clDevMemHndl;
		ppNewArgsMemLoc[i] = pCurrentArgsLocation;

		// Set buffers pendencies
		pMemObj->AddPendency();  
	}

	// Need to rollback in case of error
	if ( CL_FAILED(res) )
	{
		if ( i == m_uNumMemObjects )
		{
			--i;
		}
		for( cl_uint j=0; j<i; ++j)
		{
			MemoryObject* pMemObj = m_ppMemObjList[i];
			pMemObj->RemovePendency();
		}

		delete []pNewArgs;
		delete []ppNewArgsMemLoc;
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
    pNativeKernelParam->mem_loc  = ppNewArgsMemLoc;

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

	cl_device_id clDeviceId = m_pCommandQueue->GetQueueDeviceHandle();

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
			pMemObj->SetDataLocation(m_pCommandQueue->GetQueueDeviceHandle());
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
	    pMemObj->SetDataLocation(clDeviceId);
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
	if (NULL != pNativeKernelParam->mem_loc)
	{
		delete[] pNativeKernelParam->mem_loc;
    }

    // Remove buffers pendencies
    for( cl_uint i=0; i < m_uNumMemObjects; i++ )
    {
        // Check that mem object is allocated on device, if not allocate resource
        MemoryObject* pMemObj = m_ppMemObjList[i];    
        pMemObj->RemovePendency();
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

	cl_device_id clDeviceId = m_pCommandQueue->GetQueueDeviceHandle();

	// Add ownership on the object
	m_pKernel->AddPendency();

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
            szSize   = sizeof(cl_dev_mem);
            // Create buffer resources here if not available.
            MemoryObject* pMemObj = (MemoryObject*)pArg->GetValue();
            // Mark as used
            pMemObj->AddPendency();
            if (!(pMemObj->IsAllocated(clDeviceId)))
            {
                // Allocate
                res = pMemObj->CreateDeviceResource(clDeviceId);
                if( CL_FAILED(res))
                {
					break;
                }
            }

            szCurrentLocation += szSize;
            m_MemOclObjects.push_back(pMemObj);
        }
		else if ( pArg->IsSampler() )
		{
            szSize   = sizeof(cl_uint);
			OCLObject<_cl_sampler_int>* pSampler = reinterpret_cast<OCLObject<_cl_sampler_int>*>(pArg->GetValue());
			pSampler->AddPendency();
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
	m_pKernel->GetWorkGroupInfo(m_pCommandQueue->GetQueueDeviceHandle(), CL_KERNEL_LOCAL_MEM_SIZE, sizeof(cl_ulong), &stImplicitSize, NULL);
	stImplicitSize += stTotalLocalSize;
	if ( stImplicitSize > m_pDevice->GetMaxLocalMemorySize() )
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
			    pMemObj->RemovePendency();
			} else if( pArg->IsSampler() )
			{
				OCLObject<_cl_sampler_int>* pSampler = reinterpret_cast<OCLObject<_cl_sampler_int>*>(pArg->GetValue());
				pSampler->RemovePendency();
			}
		}
		m_pKernel->RemovePendency();

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
            szArgSize = sizeof(cl_dev_mem);
            MemoryObject* pMemObj = (MemoryObject*)pArg->GetValue();
            cl_dev_mem clDevMem = pMemObj->GetDeviceMemoryHndl(clDeviceId);
            assert( 0 != clDevMem );
            memcpy(  pArgValuesCurrentLocation, &clDevMem , szArgSize);
        }
        else if( pArg->IsSampler() )
        {
            szArgSize = sizeof(cl_uint);
            Sampler* pSampler = (Sampler*)pArg->GetValue();
			cl_uint value = pSampler->GetValue();
			memcpy ( pArgValuesCurrentLocation, &value , szArgSize);
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
			pMemObj->SetDataLocation(m_pCommandQueue->GetQueueDeviceHandle());
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
		pMemObj->SetDataLocation(m_pCommandQueue->GetQueueDeviceHandle());
	}
#endif
	
	cl_dev_cmd_desc *m_pDevCmd = &m_DevCmd;
    cl_dev_cmd_param_kernel* pKernelParam = (cl_dev_cmd_param_kernel*)m_pDevCmd->params;
    // Fill command descriptor
    m_pDevCmd->id = (cl_dev_cmd_id)m_Event.GetId();
	cl_device_id clDeviceId = m_pCommandQueue->GetQueueDeviceHandle();

    pKernelParam->kernel = m_pKernel->GetDeviceKernelId(clDeviceId);

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
        obj->RemovePendency();
    }
    m_MemOclObjects.clear();
	
    for( it = m_NonMemOclObjects.begin(); it != m_NonMemOclObjects.end(); it++)
    {
        OCLObject<_cl_mem_int>* obj = *it;
        obj->RemovePendency();
    }
    m_NonMemOclObjects.clear();

    // Delete local command
	cl_dev_cmd_desc *m_pDevCmd = &m_DevCmd;
    cl_dev_cmd_param_kernel* pKernelParam = (cl_dev_cmd_param_kernel*)m_pDevCmd->params;
	cl_char* temp = (cl_char*)pKernelParam->arg_values;
    delete[] temp;

	// Remove ownership from the object
	m_pKernel->RemovePendency();

    return CL_SUCCESS;
}


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
    m_szRowPitch(szRowPitch),
    m_szSlicePitch(szSlicePitch),
    m_pDst(pDst),
	m_szDstRowPitch(szDstRowPitch),
	m_szDstSlicePitch(szDstSlicePitch)
{
	cl_mem_object_type clMemObjType = m_pMemObj->GetType();

	cl_uint uiDimCount = 0;
    switch(clMemObjType)
    {
    case CL_MEM_OBJECT_BUFFER:
        uiDimCount = 1;
        break;
    case CL_MEM_OBJECT_IMAGE2D:
        uiDimCount = 2;
        break;
    case CL_MEM_OBJECT_IMAGE3D:
        uiDimCount = 3;
        break;
    default:
        break;
    }

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
			// Get original image pitch
			m_szDstRowPitch = pMemObj->CalcRowPitchSize(pszRegion);
		}
		if( 0 == szDstSlicePitch )
		{
			// Get original image pitch
			m_szDstSlicePitch = pMemObj->CalcSlicePitchSize(pszRegion);
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
	cl_device_id clDeviceId = m_pCommandQueue->GetQueueDeviceHandle();
	cl_device_id clDeviceDataLocation = m_pMemObj->GetDataLocation(clDeviceId);	
	
	if(0 == clDeviceDataLocation)
    {
        if (!m_pMemObj->IsAllocated(clDeviceId))
        {
            // Allocate
            cl_err_code res = m_pMemObj->CreateDeviceResource(clDeviceId);
            if( CL_FAILED(res))
            {
                return res;
            }
        }
        m_pMemObj->SetDataLocation(clDeviceId);
        clDeviceDataLocation = m_pMemObj->GetDataLocation();
    }

    m_pMemObj->AddPendency();
    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code ReadMemObjCommand::Execute()
{
    cl_err_code res = CL_SUCCESS;	
	cl_dev_cmd_desc *m_pDevCmd = &m_DevCmd;
	
	cl_device_id clDeviceId = m_pCommandQueue->GetQueueDeviceHandle();
    cl_device_id clDeviceDataLocation = m_pMemObj->GetDataLocation(clDeviceId);	
	

	// Check if this command has become MARKER because of returning CL_DONE_ON_RUNTIME previously.
	// if Yes; then this is the second time Execute() is being called and we don't to do anything,
	// we already executed before.
	if (m_commandType == CL_COMMAND_MARKER)
	{
		m_returnCode = CL_SUCCESS;
		m_Event.SetColor(EVENT_STATE_BLACK);		
		m_Event.RemovePendency();
		return CL_SUCCESS;
	}

	// We don't optimize the case of "clDeviceDataLocation == 0 && m_pMemObj->IsAllocated(0)" as long
	// we run on CPU only; since it case cause performance issues. for GPU device, its better
	// to used m_pMemObj->ReadData(..) in order to read the data.
    if (clDeviceDataLocation == 0 && !m_pMemObj->IsAllocated(0))
    {		
		// do nothing
		// data on runtime but isn't not valid, copying it is redundant				
		return CL_DONE_ON_RUNTIME;					
	}
	else
	{        
		assert(clDeviceDataLocation != 0 && "clDeviceDataLocation !=0 ");

		clDeviceDataLocation = clDeviceId;
        create_dev_cmd_rw(
            m_pMemObj->GetDeviceMemoryHndl(clDeviceDataLocation), 
            m_pMemObj->GetType(),            
			m_pDst, m_szOrigin, m_szDstOrigin, m_szRegion, m_szDstRowPitch, m_szDstSlicePitch, m_szRowPitch, m_szSlicePitch, 
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
    // TODO: copy data from dst to the local buffer.
    m_pMemObj->RemovePendency();
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
WriteBufferCommand::WriteBufferCommand(IOclCommandQueueBase* cmdQueue, ocl_entry_points* pOclEntryPoints, MemoryObject* pBuffer, const size_t* pszOffset, const size_t* pszCb, const void* cpSrc)
: WriteMemObjCommand(cmdQueue, pOclEntryPoints, pBuffer, pszOffset, pszCb, 0, 0, cpSrc)
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
            MemoryObject*     pBuffer, 
            const size_t      szBufferOrigin[3],
			const size_t      szSrcOrigin[3],
			const size_t	  szRegion[3],
			const size_t	  szBufferRowPitch,
			const size_t	  szBufferSlicePitch,
			const size_t	  szDstRowPitch,
			const size_t	  szDstSlicePitch,			            
            const void*       pDst
			):WriteMemObjCommand(cmdQueue, pOclEntryPoints, pBuffer, szBufferOrigin, szRegion, szBufferRowPitch, szBufferSlicePitch, pDst, szSrcOrigin, szDstRowPitch, szDstSlicePitch)
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
    MemoryObject*   pImage, 
    const size_t*   pszOrigin,
    const size_t*   pszRegion,
    size_t          szRowPitch,
    size_t          szSlicePitch,
    const void *    cpSrc                                 
    ): WriteMemObjCommand(cmdQueue, pOclEntryPoints, pImage,pszOrigin, pszRegion, 0, 0, cpSrc, NULL, szRowPitch, szSlicePitch)
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
    m_pMemObj(pMemObj),
    m_szRowPitch(szRowPitch),
    m_szSlicePitch(szSlicePitch),
    m_cpSrc(cpSrc),
	m_szSrcRowPitch(szSrcRowPitch),
	m_szSrcSlicePitch(szSrcSlicePitch)
{
	cl_mem_object_type clMemObjType = m_pMemObj->GetType();

	cl_uint uiDimCount = 0;
    switch(clMemObjType)
    {
    case CL_MEM_OBJECT_BUFFER:
        uiDimCount = 1;
        break;
    case CL_MEM_OBJECT_IMAGE2D:
        uiDimCount = 2;
        break;
    case CL_MEM_OBJECT_IMAGE3D:
        uiDimCount = 3;
        break;
    default:
        break;
    }

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
			m_szSrcRowPitch = pMemObj->CalcRowPitchSize(pszRegion);
		}
		if( 0 == szSrcSlicePitch )
		{
			// Get original image pitch
			m_szSrcSlicePitch = pMemObj->CalcSlicePitchSize(pszRegion);
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
	cl_device_id clDeviceId = m_pCommandQueue->GetQueueDeviceHandle();
	 // First validate that image is allocated.
    if (!m_pMemObj->IsAllocated(clDeviceId))
    {
        // Allocate
        cl_err_code res = m_pMemObj->CreateDeviceResource(clDeviceId);
        if( CL_FAILED(res))
        {
            return res;
        }
    }

    m_pMemObj->AddPendency();
    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code WriteMemObjCommand::Execute()
{
    cl_err_code res = CL_SUCCESS;
	cl_device_id clDeviceId = m_pCommandQueue->GetQueueDeviceHandle();
	m_pMemObj->GetDataLocation(m_pDevice->GetHandle());
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
	create_dev_cmd_rw(
			m_pMemObj->GetDeviceMemoryHndl(clDeviceId), 
			m_pMemObj->GetType(),
			(void*)m_cpSrc, m_szOrigin, m_szSrcOrigin, m_szRegion, m_szSrcRowPitch, m_szSrcSlicePitch, m_szRowPitch, m_szSlicePitch, 
			CL_DEV_CMD_WRITE,
			(cl_dev_cmd_id)m_Event.GetId(),
			m_pDevCmd, 
			&m_rwParams) ;

	m_pMemObj->SetDataLocation(clDeviceId);

	LogDebugA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), m_iId);
	m_Event.SetEventQueue(m_pCommandQueue);
	// Sending 1 command to the device where the buffer is located now
	m_pDevCmd->profiling = (m_pCommandQueue->IsProfilingEnabled() ? true : false );
	m_pDevCmd->data			= static_cast<ICmdStatusChangedObserver*>(this);


	res = m_pDevice->GetDeviceAgent()->clDevCommandListExecute(m_clDevCmdListId, &m_pDevCmd, 1);
	return res;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code WriteMemObjCommand::CommandDone()
{
    // TODO: copy data from dst to the local buffer.
    m_pMemObj->RemovePendency();
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
	m_Event.RemovePendency();
    return CL_SUCCESS;
}
