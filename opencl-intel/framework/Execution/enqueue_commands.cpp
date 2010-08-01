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
//For debug
#include <stdio.h>
#include <windows.h>
#include <process.h>
#include <assert.h>

using namespace Intel::OpenCL::Framework;



/******************************************************************
 * Static function to be used by all commands that need to write/read data
 ******************************************************************/
static void create_dev_cmd_rw(
    cl_dev_mem          clDevMemHndl,
    cl_mem_object_type  clMemObjType,
    void*               pData,
    const size_t*       pszOrigin,
    const size_t*       pszRegion,
    size_t              szRowPitch,
    size_t              szSlicePitch,
    cl_dev_cmd_type     clCmdType,
    cl_dev_cmd_id       clCmdId,
	cl_dev_cmd_desc*    pDevCmd
    )
{
        // Create Read command
        cl_dev_cmd_param_rw* pRWParams   = new cl_dev_cmd_param_rw;
        
        memset(pDevCmd, 0, sizeof(cl_dev_cmd_desc));
        memset(pRWParams, 0, sizeof(cl_dev_cmd_param_rw));
        
        pRWParams->memObj = clDevMemHndl;
        pRWParams->ptr = pData;

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
            if( i < uiDimCount )
            {
                pRWParams->origin[i] = pszOrigin[i];
                pRWParams->region[i] = pszRegion[i];
            }
            else
            {
                pRWParams->origin[i] = 0;
                pRWParams->region[i] = 1;
            }
        }

        pRWParams->pitch[0] = szRowPitch;
        pRWParams->pitch[1] = szSlicePitch;
        pRWParams->dim_count = uiDimCount;

        pDevCmd->type = clCmdType;
        pDevCmd->id = clCmdId;
        pDevCmd->params = pRWParams;
        pDevCmd->param_size = sizeof(cl_dev_cmd_param_rw);
}




/******************************************************************
 *
 ******************************************************************/
Command::Command():
    m_pEvent(NULL),
    m_clDevCmdListId(0),
	m_pDevice(NULL),
	m_pCommandQueue(NULL),
    m_bIsFlushed(false),
	m_returnCode(1),
	m_iId(-1)
{
	memset(&m_DevCmd, 0, sizeof(cl_dev_cmd_desc));

	INIT_LOGGER_CLIENT(L"Command Logger Client",LL_DEBUG);
}

/******************************************************************
 *
 ******************************************************************/
Command::~Command()
{
    m_pEvent =  NULL;
	m_pDevice = NULL;
	m_pCommandQueue = NULL;

	RELEASE_LOGGER_CLIENT;
}

/******************************************************************
 *
 ******************************************************************/
void Command::SetEvent(QueueEvent* queueEvent)
{ 
    m_pEvent = queueEvent;
	m_pEvent->AddPendency();
    m_iId = queueEvent->GetId();
	m_pEvent->SetCommand(this);
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
		m_pEvent->SetProfilingInfo(CL_PROFILING_COMMAND_SUBMIT, ulTimer);
        m_pEvent->SetColor(EVENT_STATE_LIME);
        LogDebugA("Command - SUBMITTED TO DEVICE  : %s (Id: %d)", GetCommandName(), m_iId);
        break;
    case CL_RUNNING:
        LogDebugA("Command - RUNNING  : %s (Id: %d)", GetCommandName(), m_iId);
		m_pEvent->SetProfilingInfo(CL_PROFILING_COMMAND_START, ulTimer);
        m_pEvent->SetColor(EVENT_STATE_GREEN);
        break;
    case CL_COMPLETE:
		m_pEvent->SetProfilingInfo(CL_PROFILING_COMMAND_END, ulTimer);
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
        m_pEvent->SetColor(EVENT_STATE_BLACK);
		m_pEvent->RemovePendency();
        break;
    default:        
        break;
    }
    return res;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code Command::CopyToHost(						
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
	
	Command* pReadMemObjCmd = new ReadMemObjCommand(pSrcMemObj, origin, region, rowPitch, slicePitch, pData);		
	pReadMemObjCmd->SetDevice(pSrcDevice);
	
	cl_event waitEvent = NULL;
	
	QueueEvent* pQueueEvent = m_pCommandQueue->GetEventsManager()->CreateQueueEvent(
			pReadMemObjCmd->GetCommandType(), &waitEvent, NULL,
			(ocl_entry_points*)(((_cl_object*)m_pCommandQueue->GetHandle())->dispatch));
	
	if (!pQueueEvent)
	{
		return CL_OUT_OF_HOST_MEMORY;
	}

	pReadMemObjCmd->SetEvent(pQueueEvent);
	res = pReadMemObjCmd->Init();
	if (CL_FAILED(res)) 
	{ 		
		return res; 
	}
		
	cl_dev_cmd_desc* pReadDevCmd =  &pReadMemObjCmd->m_DevCmd;		
	memset(pReadDevCmd,0,sizeof(cl_dev_cmd_desc));	
				
	// copy from host to device
	create_dev_cmd_rw(            
		pSrcMemObj->GetDeviceMemoryHndl(srcDeviceId), 
		pSrcMemObj->GetType(),
		pData, origin, region, 0, 0,
		CL_DEV_CMD_READ,
		(cl_dev_cmd_id)pQueueEvent->GetId(), 
		pReadDevCmd);		

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
cl_err_code Command::CopyFromHost(
						void* pSrcData,
						MemoryObject* pSrcMemObj, 
						const size_t* pSrcOrigin, 						
						const size_t* pRegion,						
						QueueEvent**	pEvent)
{
	cl_err_code res = CL_SUCCESS;

	size_t origin[MAX_WORK_DIM] = {0};
	size_t region[MAX_WORK_DIM];
	size_t rowPitch, slicePitch;
	pSrcMemObj->GetLayout(region, &rowPitch, &slicePitch);

	if (!pSrcOrigin)
		pSrcOrigin = origin;
	if (!pRegion)
		pRegion = region;

	if (*pEvent != NULL)
	{		
		cl_dev_cmd_desc *pDevCmd = &m_DevCmd;
		create_dev_cmd_rw(
				pSrcMemObj->GetDeviceMemoryHndl(m_pDevice->GetHandle()), 
				pSrcMemObj->GetType(),
				(void*)pSrcData, pSrcOrigin, pRegion, rowPitch, slicePitch,
				CL_DEV_CMD_WRITE,
				(cl_dev_cmd_id)(*pEvent)->GetId(),
				pDevCmd) ;
		pSrcMemObj->SetDataLocation(m_pDevice->GetHandle());
		LogDebugA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), m_iId);
		// Sending 1 command to the device where the buffer is located now
		m_pEvent->SetEventQueue(m_pCommandQueue);
		pDevCmd->profiling = (m_pCommandQueue->IsProfilingEnabled() ? true : false );
		pDevCmd->data			= static_cast<ICmdStatusChangedObserver*>(this);
		res = m_pDevice->GetDeviceAgent()->clDevCommandListExecute(m_clDevCmdListId, &pDevCmd, 1);
	}
	else
	{		
		Command* pWriteMemObjCmd = new WriteMemObjCommand(pSrcMemObj, origin, pRegion, rowPitch, slicePitch, pSrcData);
		pWriteMemObjCmd->SetCommandQueue(m_pCommandQueue);					
		pWriteMemObjCmd->SetDevice(m_pDevice);
		
		cl_event waitEvent = NULL;
		QueueEvent* pQueueEvent = m_pCommandQueue->GetEventsManager()->CreateQueueEvent(
				pWriteMemObjCmd->GetCommandType(), &waitEvent, 
				NULL, (ocl_entry_points*)(((_cl_object*)m_pCommandQueue->GetHandle())->dispatch));
				
		if (!pQueueEvent)
		{
			return CL_OUT_OF_HOST_MEMORY;
		}

		pWriteMemObjCmd->SetEvent(pQueueEvent);
		res = pWriteMemObjCmd->Init();
		if (CL_FAILED(res)) 
		{ 
			return res; 
		}
			
		cl_dev_cmd_desc* pWriteDevCmd =  &pWriteMemObjCmd->m_DevCmd;		
		memset(pWriteDevCmd,0,sizeof(cl_dev_cmd_desc));	
					
		// copy from host to device
		create_dev_cmd_rw(            
			pSrcMemObj->GetDeviceMemoryHndl(m_pDevice->GetHandle()), 
			pSrcMemObj->GetType(),
			pSrcData, pSrcOrigin, pRegion, rowPitch, slicePitch,
			CL_DEV_CMD_WRITE,
			(cl_dev_cmd_id)pQueueEvent->GetId(), 
			pWriteDevCmd);

		// Set new location			
		pSrcMemObj->SetDataLocation(m_pDevice->GetHandle());

		pWriteDevCmd->data = static_cast<ICmdStatusChangedObserver*>(pWriteMemObjCmd);		
		res = m_pDevice->GetDeviceAgent()->clDevCommandListExecute(NULL, &pWriteDevCmd, 1);							
		if (CL_FAILED(res)) { return res; }							

		*pEvent = pQueueEvent;	
	}
	return res;
}

cl_err_code Command::PrepareOnDevice(
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
		res = CopyFromHost(pSrcData, pSrcMemObj, NULL, NULL, pEvent);				
	}

	
	return res;
}
/******************************************************************
 *
 ******************************************************************/
CopyMemObjCommand::CopyMemObjCommand( 
                        MemoryObject*   pSrcMemObj, 
                        MemoryObject*   pDstMemObj, 
                        const size_t*   szSrcOrigin, 
                        const size_t*   szDstOrigin,
                        const size_t*   szRegion ):
    m_pSrcMemObj(pSrcMemObj),
    m_pDstMemObj(pDstMemObj)
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
        break;
    }
    
    cl_uint i;

    for( i=0; i<MAX_WORK_DIM; i++ )
    {
        if( i < m_uiSrcNumDims )
        {
            m_szSrcOrigin[i] = szSrcOrigin[i];
        }
        else
        {
            m_szSrcOrigin[i] = 0;
        }
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
        break;
    }

    for( i=0; i<MAX_WORK_DIM; i++ )
    {
        if( i < m_uiDstNumDims )
        {
            m_szDstOrigin[i] = szDstOrigin[i];
        }
        else
        {
            m_szDstOrigin[i] = 0;
        }
    }

    //
    // Set region, accurding to the bigger object, for exmple: 
    // If buffer->image it is image(2D/3D) dimensions.
    // If buffer->buffer it is 1 dimensions.
    //
    size_t regionNumDim = ( ( m_uiSrcNumDims > m_uiDstNumDims ) ?  m_uiSrcNumDims : m_uiDstNumDims ) ;
    for( i=0; i<MAX_WORK_DIM; i++ )
    {
        if( i < regionNumDim )
        {
            m_szRegion[i] = szRegion[i];
        }
        else
        {
            m_szRegion[i] = 0;
        }
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
		m_pEvent->SetColor(EVENT_STATE_BLACK);		
		return CL_SUCCESS;
	}

	/// first, make sure m_pDstMemObj resides on target device.
	/// for example, if m_pDstMemObj resides on different device this funciton will now copy
	/// the memory object to host, then called again to copy from host to the target device.
	/// notice we are returning CL_NOT_READY in cas extra operation is required.
	QueueEvent* pDepEvent = NULL;	
	PrepareOnDevice(m_pDstMemObj, NULL, NULL, &pDepEvent);
	if (pDepEvent)
	{
		m_pEvent->SetColor(EVENT_STATE_RED);
		m_pEvent->AddDependentOn(pDepEvent);		
		return CL_NOT_READY;
	}

	/// at this phase we know the m_pDstMemObj is valid on target device
	
	cl_device_id clDeviceId = m_pCommandQueue->GetQueueDeviceHandle();
	cl_device_id clSrcMemObjLoc = m_pSrcMemObj->GetDataLocation(clDeviceId);
    cl_device_id clDstMemObjLoc = m_pDstMemObj->GetDataLocation(clDeviceId);
		
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
			m_pEvent->SetColor(EVENT_STATE_RED);
			m_pEvent->AddDependentOn(pDepEvent);		
			return CL_NOT_READY;
		}
	}
	else if (bSrcOnRuntime)
	{
		void* pData = m_pSrcMemObj->GetData(m_szSrcOrigin);    						
		res = CopyFromHost(pData, m_pDstMemObj, m_szDstOrigin, m_szRegion, &m_pEvent);		
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
 * Use device copy command to copy betweem the buffers.
 * Pre condition for this function is that the 2 buffers are allocated
 * in the device.
 ******************************************************************/
cl_err_code CopyMemObjCommand::CopyOnDevice(cl_device_id clDeviceId)
{
    cl_dev_cmd_desc *m_pDevCmd = &m_DevCmd;
    cl_dev_cmd_param_copy* pCopyParams   = new cl_dev_cmd_param_copy;
    
    memset(m_pDevCmd, 0, sizeof(cl_dev_cmd_desc));
    memset(pCopyParams, 0, sizeof(cl_dev_cmd_param_copy));
    
    pCopyParams->srcMemObj      = m_pSrcMemObj->GetDeviceMemoryHndl(clDeviceId);
    pCopyParams->dstMemObj      = m_pDstMemObj->GetDeviceMemoryHndl(clDeviceId);
    pCopyParams->src_dim_count  = m_uiSrcNumDims;
    pCopyParams->dst_dim_count  = m_uiDstNumDims;
    // set all values
    for( int i=0; i< MAX_WORK_DIM; i++ )
    {
        pCopyParams->src_origin[i]  = m_szSrcOrigin[i];
        pCopyParams->dst_origin[i]  = m_szDstOrigin[i];
        pCopyParams->region[i]      = m_szRegion[i];
    }

    m_pDevCmd->type			= CL_DEV_CMD_COPY;
    m_pDevCmd->id			= (cl_dev_cmd_id)m_pEvent->GetId();
    m_pDevCmd->params		= pCopyParams;
    m_pDevCmd->param_size	= sizeof(cl_dev_cmd_param_copy);
	m_pDevCmd->profiling	= (m_pCommandQueue->IsProfilingEnabled() ? true : false );
	m_pDevCmd->data			= static_cast<ICmdStatusChangedObserver*>(this);

    m_pDstMemObj->SetDataLocation(clDeviceId);
	m_pEvent->SetEventQueue(m_pCommandQueue);
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

	cl_dev_cmd_desc *m_pDevCmd = &m_DevCmd;
    // Delete allocated resources
    if ( NULL != m_pDevCmd->params )
    {
        delete m_pDevCmd->params;
		m_pDevCmd->params = NULL;
    }
    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
CopyBufferCommand::CopyBufferCommand(
            MemoryObject*   pSrcBuffer, 
            MemoryObject*   pDstBuffer, 
            size_t          szSrcOffset, 
            size_t          szDstOffset,
            size_t          szCb
            ): CopyMemObjCommand(pSrcBuffer, pDstBuffer, &szSrcOffset, &szDstOffset, &szCb)
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
CopyImageCommand::CopyImageCommand(
    MemoryObject*   pSrcImage,
    MemoryObject*   pDstImage,
    const size_t*   pszSrcOrigin,
    const size_t*   pszDstOrigin,
    const size_t*   pszRegion
    ): CopyMemObjCommand(pSrcImage, pDstImage, pszSrcOrigin, pszDstOrigin, pszRegion)
{
	m_commandType = CL_COMMAND_COPY_IMAGE;
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
    MemoryObject*   pSrcBuffer, 
    MemoryObject*   pDstImage, 
    size_t          szSrcOffset, 
    const size_t*   pszDstOrigin, 
    const size_t*   pszDstRegion
    ): CopyMemObjCommand(pSrcBuffer, pDstImage, &szSrcOffset, pszDstOrigin, pszDstRegion)
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
    MemoryObject*   pSrcImage, 
    MemoryObject*   pDstBuffer, 
    const size_t*   pszSrcOrigin, 
    const size_t*   pszSrcRegion,
    size_t          szDstOffset
    ): CopyMemObjCommand(pSrcImage, pDstBuffer, pszSrcOrigin, &szDstOffset, pszSrcRegion)
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
MapBufferCommand::MapBufferCommand(MemoryObject* pBuffer, cl_map_flags clMapFlags, size_t szOffset, size_t szCb):
    MapMemObjCommand(pBuffer, clMapFlags, NULL, NULL, NULL, NULL)
{
    size_t* pOrigin = new size_t;
    *pOrigin = szOffset;
    m_pOrigin = pOrigin;
    
    size_t* pRegion = new size_t;
    *pRegion = szCb;
    m_pRegion = pRegion;
}

/******************************************************************
 *
 ******************************************************************/
MapBufferCommand::~MapBufferCommand()
{
    delete m_pOrigin;
    delete m_pRegion;
}

/******************************************************************
 *
 ******************************************************************/
MapImageCommand::MapImageCommand(
            MemoryObject*   pImage,
            cl_map_flags    clMapFlags, 
            const size_t*   pOrigin, 
            const size_t*   pRegion,
            size_t*         pszImageRowPitch,
            size_t*         pszImageSlicePitch
            ):
MapMemObjCommand(pImage, clMapFlags, pOrigin, pRegion, pszImageRowPitch, pszImageSlicePitch)
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
            MemoryObject*   pMemObj,
            cl_map_flags    clMapFlags, 
            const size_t*   pOrigin, 
            const size_t*   pRegion,
            size_t*         pszImageRowPitch,
            size_t*         pszImageSlicePitch
            ):
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
    // TODO: Add support for multiple device.
    // What happens when data is not on the same device???
	QueueEvent* pMemObjEvent = NULL;
	cl_err_code res = CL_SUCCESS;
	
	res = PrepareOnDevice(m_pMemObj, m_pOrigin, m_pRegion, &pMemObjEvent);
	if (CL_FAILED(res)) { return res; }

	if (pMemObjEvent)
	{	
		m_pEvent->AddFloatingDependence();
		m_pEvent->SetColor(EVENT_STATE_RED);				
		m_pEvent->AddDependentOn(pMemObjEvent);				
		m_pEvent->RemoveFloatingDependence();		
		return CL_NOT_READY;
	}		

	cl_device_id clDeviceId = m_pCommandQueue->GetQueueDeviceHandle();
	cl_dev_cmd_desc *m_pDevCmd = &m_DevCmd;
    // Prepare command. 
    // Anyhow we send the map command to the device though  we expect that on write
    // there is nothing to do, and on read the device may need to copy from device memory to host memory
    memset(m_pDevCmd, 0, sizeof(cl_dev_cmd_desc));
    m_pDevCmd->id          = (cl_dev_cmd_id)m_pEvent->GetId();
    m_pDevCmd->type        = CL_DEV_CMD_MAP;
    m_pDevCmd->param_size  = sizeof(cl_dev_cmd_param_map);
    m_pDevCmd->params      = m_pMemObj->GetMappedRegionInfo(clDeviceId, m_pMappedRegion);

	m_pDevCmd->profiling = (m_pCommandQueue->IsProfilingEnabled() ? true : false );
	m_pDevCmd->data			= static_cast<ICmdStatusChangedObserver*>(this);

	m_pEvent->SetEventQueue(m_pCommandQueue);
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
UnmapMemObjectCommand::UnmapMemObjectCommand(MemoryObject* pMemObject, void* pMappedRegion):
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
    memset(m_pDevCmd, 0, sizeof(cl_dev_cmd_desc));

    m_pDevCmd->id          = (cl_dev_cmd_id)m_pEvent->GetId();
    m_pDevCmd->type        = CL_DEV_CMD_UNMAP;
    m_pDevCmd->param_size  = sizeof(cl_dev_cmd_param_map);
    m_pDevCmd->params      = m_pMemObject->GetMappedRegionInfo(clDeviceId, m_pMappedRegion);
	m_pDevCmd->data			= static_cast<ICmdStatusChangedObserver*>(this);

    // Color will be changed only when command is submitted in the device    
    LogDebugA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), m_iId);

	m_pMemObject->SetDataLocation(clDeviceId);
	m_pEvent->SetEventQueue(m_pCommandQueue);
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
           void              (*pUserFnc)(void *), 
           void*               pArgs, 
           size_t              szCbArgs,
           cl_uint             uNumMemObjects,
           MemoryObject**      ppMemObjList,
           const void**        ppArgsMemLoc):
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
    size_t clMemSize = sizeof(cl_mem);
    size_t clDevMemSize = sizeof(cl_dev_mem);
    size_t szCbNewArgsSize = m_szCbArgs + m_uNumMemObjects * ( clDevMemSize - clMemSize);
	cl_device_id clDeviceId = m_pCommandQueue->GetQueueDeviceHandle();

    void*   pNewArgs = malloc(szCbNewArgsSize);
	if(NULL == pNewArgs)
	{
		 return CL_MEM_OBJECT_ALLOCATION_FAILURE;
	}
    void**  ppNewArgsMemLoc = (void**)malloc(sizeof(void*) * m_uNumMemObjects);

	if(NULL == ppNewArgsMemLoc)
	{
		 return CL_MEM_OBJECT_ALLOCATION_FAILURE;
	}

    // Set the parameters for the device
    cl_uint i;
    void* pCurrentArgsLocation = pNewArgs;
    size_t szCbToCopy = 0;
    size_t szCbMaxToCopy = szCbNewArgsSize;

    for( i=0; i < m_uNumMemObjects; i++ )
    {
        // Check that mem object is allocated on device, if not allocate resource
        MemoryObject* pMemObj = m_ppMemObjList[i];
        if (!(pMemObj->IsAllocated(clDeviceId)))
        {
            // Allocate
            res = pMemObj->CreateDeviceResource(clDeviceId);
            if( CL_FAILED(res))
            {
                free(pNewArgs);
                free(ppNewArgsMemLoc);
                return CL_MEM_OBJECT_ALLOCATION_FAILURE;
            }
        }
        // Set the new args list
        cl_dev_mem clDevMemHndl = pMemObj->GetDeviceMemoryHndl(clDeviceId);
        if( 0 == i )
        {
            szCbToCopy = (cl_uchar*)m_ppArgsMemLoc[i] - (cl_uchar*)m_pArgs;
            memcpy_s(pCurrentArgsLocation, szCbMaxToCopy, m_pArgs, szCbToCopy);
            szCbMaxToCopy -= szCbToCopy;
        }
        else
        {
            szCbToCopy = (cl_uchar*)(m_ppArgsMemLoc[i]) - ((cl_uchar*)(m_ppArgsMemLoc[i-1]) + clMemSize);
            memcpy_s(pCurrentArgsLocation, szCbMaxToCopy, (void*)((cl_uchar*)(m_ppArgsMemLoc[i-1]) + clMemSize), szCbToCopy);
            szCbMaxToCopy -= szCbToCopy;
        }
        ppNewArgsMemLoc[i] = (void*)((cl_uchar*)pCurrentArgsLocation + szCbToCopy);
        memcpy_s(ppNewArgsMemLoc[i], szCbMaxToCopy, &clDevMemHndl, clDevMemSize);
        szCbMaxToCopy -= clDevMemSize;
        pCurrentArgsLocation = (cl_uchar*)ppNewArgsMemLoc[i] + clDevMemSize;

        // Set buffers pendencies
        pMemObj->AddPendency();  
        // Set new location of the buffers, the device, we have n
        // TODO: need prefetching in Native Kernel in case buffers are in different device.
    }

    // Copy the end of the original args
    size_t szLastBytesToCopy = (cl_uchar*)m_pArgs + m_szCbArgs - ((cl_uchar*)(m_ppArgsMemLoc[i-1]) + clMemSize);
    memcpy_s(pCurrentArgsLocation, szCbMaxToCopy, (void*)((cl_uchar*)(m_ppArgsMemLoc[i-1]) + clMemSize),  szLastBytesToCopy);
    szCbMaxToCopy -= szLastBytesToCopy;
    assert( 0 == szCbMaxToCopy);

    //
    // Prepare the device command
    //
	cl_dev_cmd_desc *m_pDevCmd = &m_DevCmd;
    cl_dev_cmd_param_native* pNativeKernelParam = new cl_dev_cmd_param_native;
        
    memset(m_pDevCmd, 0, sizeof(cl_dev_cmd_desc));
    memset(pNativeKernelParam, 0, sizeof(cl_dev_cmd_param_native));

    pNativeKernelParam->args     = szCbNewArgsSize;
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
	cl_device_id clDeviceId = m_pCommandQueue->GetQueueDeviceHandle();

	// Fill command descriptor
	cl_dev_cmd_desc *m_pDevCmd = &m_DevCmd;
    m_pDevCmd->id = (cl_dev_cmd_id)m_pEvent->GetId();

    // Sending the queue command
    // TODO: Handle the case were buffers are located in different device. Prefetching
    for( unsigned int i=0; i < m_uNumMemObjects; i++ )
    {
        // Check that mem object is allocated on device, if not allocate resource
        MemoryObject* pMemObj = m_ppMemObjList[i];
	    pMemObj->SetDataLocation(clDeviceId);
	}

    LogDebugA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), m_iId);

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
    if( NULL != m_pDevCmd->params )
    {
        cl_dev_cmd_param_native* pNativeKernelParam = (cl_dev_cmd_param_native*)m_pDevCmd->params;
        free(pNativeKernelParam->argv);
        free(pNativeKernelParam->mem_loc);
        delete pNativeKernelParam;
    }

    // Remove buffers pendencies
    for( cl_uint i=0; i < m_uNumMemObjects; i++ )
    {
        // Check that mem object is allocated on device, if not allocate resource
        MemoryObject* pMemObj = m_ppMemObjList[i];    
        pMemObj->RemovePendency();
    }
    free(m_ppMemObjList);

    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
NDRangeKernelCommand::NDRangeKernelCommand(
    Kernel*         pKernel,
    cl_uint         uiWorkDim,
    const size_t*   cpszGlobalWorkOffset, 
    const size_t*   cpszGlobalWorkSize, 
    const size_t*   cpszLocalWorkSize
    ):
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
    cl_err_code res;
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

    cl_uint i;
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
                    return res;
                }
            }

            szCurrentLocation += szSize;
            m_MemOclObjects.push_back(pMemObj);
        }
		else if ( pArg->IsSampler() )
		{
            szSize   = sizeof(cl_uint);
			OCLObject<_cl_sampler>* pSampler = reinterpret_cast<OCLObject<_cl_sampler>*>(pArg->GetValue());
			pSampler->AddPendency();
            szCurrentLocation += szSize;
            m_NonMemOclObjects.push_back(reinterpret_cast<OCLObject<_cl_mem> *>(pSampler));
		}
		else
        {
            //Just calculate the size for allocation
            szCurrentLocation += pArg->GetSize();
        }
    }
    
    // Setup Kernel parameters
	cl_dev_cmd_desc *m_pDevCmd = &m_DevCmd;
    cl_dev_cmd_param_kernel* pKernelParam = new cl_dev_cmd_param_kernel;

	// TODO: We are going to fill values, why we need memset
    memset(m_pDevCmd, 0, sizeof(cl_dev_cmd_desc));
    memset(pKernelParam, 0, sizeof(cl_dev_cmd_param_kernel));

  
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
        pKernelParam->glb_wrk_offs[i] = 0;
        pKernelParam->glb_wrk_size[i] = m_cpszGlobalWorkSize[i];
        // If m_cpszLocalWorkSize == NULL, set to 0. Agent is expected to handle lcl_wrk_size 0 as NULL
        if ( NULL != m_cpszLocalWorkSize )
        {
            pKernelParam->lcl_wrk_size[i] = m_cpszLocalWorkSize[i];
        }
        else
        {
            pKernelParam->lcl_wrk_size[i] = 0;
        }
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
	bool hasDepends = false;
	// Set location

	list<OCLObject<_cl_mem>*>::iterator it;
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

	cl_dev_cmd_desc *m_pDevCmd = &m_DevCmd;
    cl_dev_cmd_param_kernel* pKernelParam = (cl_dev_cmd_param_kernel*)m_pDevCmd->params;
    // Fill command descriptor
    m_pDevCmd->id = (cl_dev_cmd_id)m_pEvent->GetId();
	cl_device_id clDeviceId = m_pCommandQueue->GetQueueDeviceHandle();

    pKernelParam->kernel = m_pKernel->GetDeviceKernelId(clDeviceId);

    // Color will be changed only when command is submitted in the device    
    
    // Sending the queue command
    // TODO: Handle the case were buffers are located in different device.
    LogDebugA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), m_iId);

	m_pDevCmd->profiling = (m_pCommandQueue->IsProfilingEnabled() ? true : false );
	m_pDevCmd->data			= static_cast<ICmdStatusChangedObserver*>(this);
	m_pEvent->SetEventQueue(m_pCommandQueue);
	return m_pDevice->GetDeviceAgent()->clDevCommandListExecute(m_clDevCmdListId, &m_pDevCmd, 1);
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code NDRangeKernelCommand::CommandDone()
{
    // Clear all resources
    // Remove object pendencies

    list<OCLObject<_cl_mem>*>::iterator it;
    for( it = m_MemOclObjects.begin(); it != m_MemOclObjects.end(); it++)
    {
        OCLObject<_cl_mem>* obj = *it;
        obj->RemovePendency();
    }
    m_MemOclObjects.clear();
	
    for( it = m_NonMemOclObjects.begin(); it != m_NonMemOclObjects.end(); it++)
    {
        OCLObject<_cl_mem>* obj = *it;
        obj->RemovePendency();
    }
    m_NonMemOclObjects.clear();

    // Delete local command
	cl_dev_cmd_desc *m_pDevCmd = &m_DevCmd;
    cl_dev_cmd_param_kernel* pKernelParam = (cl_dev_cmd_param_kernel*)m_pDevCmd->params;
    delete[] pKernelParam->arg_values;
    delete pKernelParam;

	// Remove ownership from the object
	m_pKernel->RemovePendency();

    return CL_SUCCESS;
}


/******************************************************************
 * Command: ReadBufferCommand
 * The functions below implement the Read Buffer functinoality
 *
 ******************************************************************/
ReadBufferCommand::ReadBufferCommand(MemoryObject* pBuffer, size_t szOffset, size_t szCb, void* pDst)
:ReadMemObjCommand(pBuffer, &szOffset, &szCb, 0, 0, pDst)
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
ReadImageCommand::ReadImageCommand(
			MemoryObject*   pImage,
            const size_t*   pszOrigin,
            const size_t*   pszRegion,
            size_t          szRowPitch, 
            size_t          szSlicePitch, 
            void*           pDst)
:ReadMemObjCommand(pImage, pszOrigin, pszRegion, szRowPitch, szSlicePitch, pDst)
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
    MemoryObject*   pMemObj,
    const size_t*   pszOrigin,
    const size_t*   pszRegion,
    size_t          szRowPitch, 
    size_t          szSlicePitch, 
    void*           pDst
    ):
    m_pMemObj(pMemObj),
    m_szRowPitch(szRowPitch),
    m_szSlicePitch(szSlicePitch),
    m_pDst(pDst)
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
		if (i < uiDimCount)
		{
			m_szOrigin[i] = pszOrigin[i];
			m_szRegion[i] = pszRegion[i];
		}
		else
		{
			m_szOrigin[i] = 0;
			m_szRegion[i] = 0;
		}
    }

	if (pMemObj->GetType() != CL_MEM_OBJECT_BUFFER)
	{
		if( 0 == szRowPitch  )
		{
			// Get original image pitch
			m_szRowPitch = pMemObj->CalcRowPitchSize(pszRegion);
		}
		if( 0 == szSlicePitch )
		{
			// Get original image pitch
			m_szSlicePitch = pMemObj->CalcSlicePitchSize(pszRegion);
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
		m_pEvent->SetColor(EVENT_STATE_BLACK);		
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
		
        create_dev_cmd_rw(
            m_pMemObj->GetDeviceMemoryHndl(clDeviceDataLocation), 
            m_pMemObj->GetType(),
            m_pDst, m_szOrigin, m_szRegion, m_szRowPitch, m_szSlicePitch,
            CL_DEV_CMD_READ,
            (cl_dev_cmd_id)m_pEvent->GetId(),
			m_pDevCmd);

        LogDebugA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), m_iId);
		m_pEvent->SetEventQueue(m_pCommandQueue);
        // Sending 1 command to the device where the buffer is located now
		res = m_pDevCmd->profiling = (m_pCommandQueue->IsProfilingEnabled() ? true : false );
		m_pDevCmd->data			= static_cast<ICmdStatusChangedObserver*>(this);
		
		// Read the buffer from where the data is most valid
		// device with Id==clDeviceDataLocation might be different than m_pDevice, hence 
		// we don't necessarily read the data from m_pDevice.
		/*Device* pDevice;
		Context *pContext = (Context*)m_pMemObj->GetContext();				
		res = pContext->GetDevice(clDeviceDataLocation, &pDevice);
		if (CL_FAILED(res))
		{
			return res;
		}*/		
		res = m_pDevice->GetDeviceAgent()->clDevCommandListExecute(m_clDevCmdListId, &m_pDevCmd, 1);
    }
    return res;    

}

/******************************************************************
 *
 ******************************************************************/
cl_err_code ReadMemObjCommand::CommandDone()
{
	cl_dev_cmd_desc *m_pDevCmd = &m_DevCmd;
    // TODO: copy data from dst to the local buffer.
    m_pMemObj->RemovePendency();
    // Delete local command
    cl_dev_cmd_param_rw* pRWParams = (cl_dev_cmd_param_rw*)m_pDevCmd->params;	
	if (pRWParams)
	{
		delete pRWParams;
		pRWParams = NULL;
	}	
    return CL_SUCCESS;
}


/******************************************************************
 *
 ******************************************************************/
TaskCommand::TaskCommand(Kernel* pKernel):
    NDRangeKernelCommand(pKernel, 1, NULL, &m_szStaticWorkSize, &m_szStaticWorkSize),
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
WriteBufferCommand::WriteBufferCommand(MemoryObject* pBuffer, size_t szOffset, size_t szCb, const void* cpSrc)
: WriteMemObjCommand(pBuffer, &szOffset, &szCb, 0, 0, cpSrc)
{
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
WriteImageCommand::WriteImageCommand(
    MemoryObject*   pImage, 
    const size_t*   pszOrigin,
    const size_t*   pszRegion,
    size_t          szRowPitch,
    size_t          szSlicePitch,
    const void *    cpSrc                                 
    ): WriteMemObjCommand(pImage,pszOrigin, pszRegion, szRowPitch, szSlicePitch, cpSrc)
{
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
    MemoryObject*   pMemObj,
    const size_t*   pszOrigin,
    const size_t*   pszRegion,
    size_t          szRowPitch, 
    size_t          szSlicePitch, 
    const void*     cpSrc
    ):
    m_pMemObj(pMemObj),
    m_szRowPitch(szRowPitch),
    m_szSlicePitch(szSlicePitch),
    m_cpSrc(cpSrc)
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
		if (i < uiDimCount)
		{
			m_szOrigin[i] = pszOrigin[i];
			m_szRegion[i] = pszRegion[i];
		}
		else
		{
			m_szOrigin[i] = 0;
			m_szRegion[i] = 0;
		}
    }

	if (pMemObj->GetType() != CL_MEM_OBJECT_BUFFER)
	{
		if( 0 == szRowPitch  )
		{
			// Get original image pitch
			m_szRowPitch = pMemObj->CalcRowPitchSize(pszRegion);
		}
		if( 0 == szSlicePitch )
		{
			// Get original image pitch
			m_szSlicePitch = pMemObj->CalcSlicePitchSize(pszRegion);
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
	cl_device_id memObjDeviceId = m_pMemObj->GetDataLocation(m_pDevice->GetHandle());
	cl_dev_cmd_desc *m_pDevCmd = &m_DevCmd;

	 //  do change here: write to the last valid location, thought it might be the runtime.  same as read	
	QueueEvent* pMemObjEvent = NULL;		
	res = PrepareOnDevice(m_pMemObj, NULL, NULL, &pMemObjEvent);
	if (CL_FAILED(res)) { return res; }

	if (pMemObjEvent)
	{			
		m_pEvent->SetColor(EVENT_STATE_RED);
		m_pEvent->AddDependentOn(pMemObjEvent);		
		return CL_NOT_READY;
	}		


	/// memory object resides on target device, update it
	create_dev_cmd_rw(
			m_pMemObj->GetDeviceMemoryHndl(clDeviceId), 
			m_pMemObj->GetType(),
			(void*)m_cpSrc, m_szOrigin, m_szRegion, m_szRowPitch, m_szSlicePitch,
			CL_DEV_CMD_WRITE,
			(cl_dev_cmd_id)m_pEvent->GetId(),
			m_pDevCmd) ;

	m_pMemObj->SetDataLocation(clDeviceId);

	LogDebugA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), m_iId);
	m_pEvent->SetEventQueue(m_pCommandQueue);
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
	cl_dev_cmd_desc *m_pDevCmd = &m_DevCmd;
    // TODO: copy data from dst to the local buffer.
    m_pMemObj->RemovePendency();
    // Delete local command
    cl_dev_cmd_param_rw* pRWParams = (cl_dev_cmd_param_rw*)m_pDevCmd->params;
	if (pRWParams)
	{
		delete pRWParams;
	}
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
    return m_pEvent->SetColor(EVENT_STATE_BLACK);
}

#if 0
/******************************************************************
 *
 ******************************************************************/
unsigned int __stdcall DummyCommandThreadEntryPoint(void* threadObject)
{
    Sleep(300);
    Command* pCommand = (Command*)threadObject;
    pCommand->GetEvent()->SetColor(EVENT_STATE_BLACK);
    return 1;
}


cl_err_code DummyCommand::Execute()
{
    // Start execution thread;
    _beginthreadex(NULL, 0, DummyCommandThreadEntryPoint, this, 0, NULL);
    return CL_SUCCESS;
}
#endif
