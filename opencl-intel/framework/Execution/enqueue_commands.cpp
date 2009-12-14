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
#include "command_queue.h"
#include "queue_event.h"
#include "cl_memory_object.h"
#include "command_receiver.h"
#include "kernel.h"
#include "sampler.h"

//For debug
#include <stdio.h>
#include <windows.h>
#include <process.h>
#include <assert.h>

using namespace Intel::OpenCL::Framework;



/******************************************************************
 * Static function to be used by all commands that need to write/read data
 ******************************************************************/
static cl_dev_cmd_desc* create_dev_cmd_rw(
    cl_dev_mem          clDevMemHndl,
    cl_mem_object_type  clMemObjType,
    void*               pData,
    const size_t*       pszOrigin,
    const size_t*       pszRegion,
    size_t              szRowPitch,
    size_t              szSlicePitch,
    cl_dev_cmd_type     clCmdType,
    cl_dev_cmd_id       clCmdId         
    )
{
        // Create Read command
        cl_dev_cmd_desc*     pDevCmd     = new cl_dev_cmd_desc;
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

        return pDevCmd;
}




/******************************************************************
 *
 ******************************************************************/
Command::Command():
    m_pQueueEvent(NULL),
    m_pReceiver(NULL),
    m_pDevCmd(NULL),
    m_clDevCmdListId(0),
    m_bIsFlushed(false)
{
    m_pStatusChangeObserver = this;

	INIT_LOGGER_CLIENT(L"Command Logger Client",LL_DEBUG);
}

/******************************************************************
 *
 ******************************************************************/
Command::~Command()
{
    // The command delets its event
    if ( NULL != m_pQueueEvent) delete m_pQueueEvent;
    m_pQueueEvent =  NULL;
    m_pReceiver =    NULL;
    m_pDevCmd = NULL; // Should be released on CommandDone

	RELEASE_LOGGER_CLIENT;
}

/******************************************************************
 *
 ******************************************************************/
void Command::SetEvent(QueueEvent* queueEvent)
{ 
    m_pQueueEvent = queueEvent; 
    m_iId = queueEvent->GetId(); 
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code Command::NotifyCmdStatusChanged(cl_dev_cmd_id clCmdId, cl_int iCmdStatus, cl_int iCompletionResult, cl_ulong ulTimer)
{
    cl_err_code res = CL_SUCCESS;
    switch(iCmdStatus)
    {
    case CL_QUEUED:
		// Nothing to do, not expected to be here at all
		break;
    case CL_SUBMITTED:
		m_pQueueEvent->SetProfilingInfo(CL_PROFILING_COMMAND_SUBMIT, ulTimer);
        res = m_pQueueEvent->SetEventColor(EVENT_STATE_LIME);
        LogInfoA("Command - SUBMITTED TO DEVICE  : %s (Id: %d)", GetCommandName(), GetId());
        break;
    case CL_RUNNING:
        LogInfoA("Command - RUNNING  : %s (Id: %d)", GetCommandName(), GetId());
		m_pQueueEvent->SetProfilingInfo(CL_PROFILING_COMMAND_START, ulTimer);
        res = m_pQueueEvent->SetEventColor(EVENT_STATE_GREEN);
        break;
    case CL_COMPLETE:
		m_pQueueEvent->SetProfilingInfo(CL_PROFILING_COMMAND_END, ulTimer);
        // Complete command,
        // do that before set event, since side effect of SetEvent(black) may be deleting of this instance.
        // Is error
        if (CL_FAILED(iCompletionResult))
        {
            LogErrorA("Command - DONE - Failure  : %s (Id: %d)", GetCommandName(), GetId());
			//assert(0 && "Command - DONE - Failure");
        }
        else
        {
            LogInfoA("Command - DONE - SUCCESS : %s (Id: %d)", GetCommandName(), GetId());
        }
        res = CommandDone();
        res &= m_pQueueEvent->SetEventColor(EVENT_STATE_BLACK);
        break;
    default:        
        break;
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
    m_pSrcMemObj->AddPendency();
    m_pDstMemObj->AddPendency();
    return CL_SUCCESS;
}

/******************************************************************
 * Copy memory object asks device to perform copy only if both objects are on
 * the same device. Else, it read from 1 device and write to other. 
 * Either ways, the location of the destation data remain the same, unless 
 * it was never allocated before.
 *
 ******************************************************************/
cl_err_code CopyMemObjCommand::Execute()
{   
    cl_err_code res = CL_SUCCESS;
    // First check who is responsible for copy...
    cl_device_id clSrcMemObjLocation = m_pSrcMemObj->GetDataLocation();
    cl_device_id clDstMemObjLocation = m_pDstMemObj->GetDataLocation();

    // As long as multiple devices are not implemented always copy on device
    // This is to prevent the case that 0 == clDeviceDataLocation which is not implemented.
    // TODO: handle it correctly where there is more than one device... or for USE_HOST_PTR on host
    if(0 == clSrcMemObjLocation)
    {
        if (!m_pSrcMemObj->IsAllocated(m_clDeviceId))
        {
            // Allocate
            res = m_pSrcMemObj->CreateDeviceResource(m_clDeviceId);
            if( CL_FAILED(res))
            {
                return res;
            }
        }
        m_pSrcMemObj->SetDataLocation(m_clDeviceId);
        clSrcMemObjLocation = m_pSrcMemObj->GetDataLocation();
    }

    if(0 == clDstMemObjLocation)
    {
        if (!m_pDstMemObj->IsAllocated(m_clDeviceId))
        {
            // Allocate
            res = m_pDstMemObj->CreateDeviceResource(m_clDeviceId);
            if( CL_FAILED(res))
            {
                return res;
            }
        }
        m_pDstMemObj->SetDataLocation(m_clDeviceId);
        clDstMemObjLocation = m_pDstMemObj->GetDataLocation();
    }

    // Does src MemObj is allocated? If not, user problem, do nothing...
    if ( 0 == clSrcMemObjLocation && !(m_pSrcMemObj->IsAllocated(0)))
    {
        // Src is not allocate, return device is not executing error, 
        // The working thread will call set command color to black.
        return CL_ERR_FAILURE;
    }

    // If destination is not allocated, allocate it where the src is.
    if ( 0 == clDstMemObjLocation && !(m_pDstMemObj->IsAllocated(0)))
    {
        // Allocate
        res = m_pDstMemObj->CreateDeviceResource(clSrcMemObjLocation);        
        if( CL_FAILED(res))
        {
            return res;
        }
        clDstMemObjLocation = clSrcMemObjLocation;
    }

    if ( clDstMemObjLocation == clSrcMemObjLocation &&  0 == clSrcMemObjLocation )
    {   // Src = 0; Dst = 0
        res = CopyHost(); 
    }
    else if ( clDstMemObjLocation == clSrcMemObjLocation )
    {   // Src = Dst != 0
        res = CopyOnDevice(clSrcMemObjLocation);
    }
    else if ( 0 == clSrcMemObjLocation)
    {   // Src = 0; Dst != 0
        res = CopyFromHost(clDstMemObjLocation);
    }
    else if ( 0 == clDstMemObjLocation )
    {   // Src != 0; Dst = 0
        res = CopyToHost(clSrcMemObjLocation);
    }
    else
    {   // Src != Dst !=0
        res = CopyFromDevice(clSrcMemObjLocation, clDstMemObjLocation);
    }
    return res;
}

/******************************************************************
 * Copy memory objects on the host, no access to a device,
 * read data from one object and update the second object data.
 *
 * TODO: Add support to copy between images on Host
 ******************************************************************/
cl_err_code CopyMemObjCommand::CopyHost()
{
    void* pData = m_pDstMemObj->GetData(m_szDstOrigin);

    // copies region to pData which is a memory of the dst object.
    // Currently this function does not use pitches, hence support only buffers (no pitches)
    m_pSrcMemObj->ReadData(pData, m_szSrcOrigin, m_szRegion);

    // Set dst location to the host
    m_pDstMemObj->SetDataLocation(0);

    // Return failure only to signal the process that the command is done, no need to wait for device reaction.
    return CL_ERR_FAILURE;

}

/******************************************************************
 * Use device copy command to copy betweem the buffers.
 * Pre condition for this function is that the 2 buffers are allocated
 * in the device.
 ******************************************************************/
cl_err_code CopyMemObjCommand::CopyOnDevice(cl_device_id clDeviceId)
{
    m_pDevCmd = new cl_dev_cmd_desc;
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

    m_pDevCmd->type       = CL_DEV_CMD_COPY;
    m_pDevCmd->id         = (cl_dev_cmd_id)m_pQueueEvent->GetId();
    m_pDevCmd->params     = pCopyParams;
    m_pDevCmd->param_size = sizeof(cl_dev_cmd_param_copy);

    m_pDstMemObj->SetDataLocation(clDeviceId);
    // Sending 1 command to the device where the bufer is located now
    // Color will be changed only when command is submited in the device    
    LogInfoA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), GetId());

    return m_pReceiver->EnqueueDevCommands(clDeviceId, m_clDevCmdListId, m_pDevCmd, &m_pStatusChangeObserver, 1);
}

/******************************************************************
 * This function takes the source data from the source buffer host location
 * and write it to the dst buffer in device clDstDeviceId;
 *
 * TODO: Add support images CopyFromHost, current version valid for buffers only
 ******************************************************************/
cl_err_code CopyMemObjCommand::CopyFromHost(cl_device_id clDstDeviceId)
{
    void* pData = m_pSrcMemObj->GetData(m_szSrcOrigin);    

    // Initiate read from device
    // Currnetly this function does not use pitches, hence support only buffers (no pitches)
    // TODO: Add support for images
    m_pDevCmd = create_dev_cmd_rw(            
            m_pDstMemObj->GetDeviceMemoryHndl(clDstDeviceId), 
            m_pDstMemObj->GetType(),
            pData, m_szDstOrigin, m_szRegion, 0, 0,
            CL_DEV_CMD_WRITE,
            (cl_dev_cmd_id)m_pQueueEvent->GetId());

    // Set new location
    m_pDstMemObj->SetDataLocation(clDstDeviceId);

    // Sending 1 command to the src device
    // Change status of the command to Gray before handle by the device
    // Color will be changed only when command is submited in the device    
    LogInfoA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), GetId());

    return m_pReceiver->EnqueueDevCommands(clDstDeviceId, m_clDevCmdListId, m_pDevCmd, &m_pStatusChangeObserver, 1);
}

/******************************************************************
 * This function copies the data from the clSrcDeviceId device
 * to the dst buffer local memory.
 *
 * TODO: Add support images CopyToHost, current version valid for buffers only
 ******************************************************************/
cl_err_code CopyMemObjCommand::CopyToHost(cl_device_id clSrcDeviceId)
{
    void* pData = m_pDstMemObj->GetData(m_szDstOrigin);

    // Initiate read from device
    // Currnetly this function does not use pitches, hence support only buffers (no pitches)
    m_pDevCmd = create_dev_cmd_rw(
            m_pSrcMemObj->GetDeviceMemoryHndl(clSrcDeviceId), 
            m_pSrcMemObj->GetType(),
            pData, m_szSrcOrigin, m_szRegion, 0, 0,
            CL_DEV_CMD_READ,
            (cl_dev_cmd_id)m_pQueueEvent->GetId());

    // Set new location on the host
    m_pDstMemObj->SetDataLocation(0);

    // Sending 1 command to the src device
    // Change status of the command to Gray before handle by the device
    // Color will be changed only when command is submited in the device    
    return m_pReceiver->EnqueueDevCommands(clSrcDeviceId, m_clDevCmdListId, m_pDevCmd, &m_pStatusChangeObserver, 1);
}

/******************************************************************
 * Reads data from srcBuffer in device clSrcBufferLocation and
 * write data to dstBuffer in device clDstBufferLocation
 *
 * TODO: Add support to CopyFromDevice.
 ******************************************************************/
cl_err_code CopyMemObjCommand::CopyFromDevice(cl_device_id clSrcDeviceId, cl_device_id clDstDeviceId)
{
    assert("More than 1 device command per queue command is not implemented yet! can't copy between devices" && 0);
    return CL_ERR_FAILURE;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code CopyMemObjCommand::CommandDone()
{
    m_pSrcMemObj->RemovePendency();
    m_pDstMemObj->RemovePendency();

    // Delete allocated resources
    if ( NULL != m_pDevCmd )
    {
        if ( NULL != m_pDevCmd->params )
        {
            delete m_pDevCmd->params;
        }
        delete m_pDevCmd;
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
    
    // First validate that bufer is allocated.
    if (!m_pMemObj->IsAllocated(m_clDeviceId))
    {
        // Allocate
        res = m_pMemObj->CreateDeviceResource(m_clDeviceId);
        if( CL_FAILED(res))
        {
            return res;
        }
    }

    // Get pointer to the device
    m_pMappedRegion = m_pMemObj->CreateMappedRegion(m_clDeviceId, m_clMapFlags, m_pOrigin, m_pRegion, m_pszImageRowPitch, m_pszImageSlicePitch);
    if ( NULL == m_pMappedRegion )
    {
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
    // TODO: Add support for multipule device.
    // What happens when data is not on the same device???

    // Prepare command. 
    // Anyhow we send the map command to the device though  we expect that on write
    // there is nothing to do, and on read the device may need to copy from device memory to host memory
    m_pDevCmd = new cl_dev_cmd_desc;
    memset(m_pDevCmd, 0, sizeof(cl_dev_cmd_desc));
    m_pDevCmd->id          = (cl_dev_cmd_id)m_pQueueEvent->GetId();
    m_pDevCmd->type        = CL_DEV_CMD_MAP;
    m_pDevCmd->param_size  = sizeof(cl_dev_cmd_param_map);
    m_pDevCmd->params      = m_pMemObj->GetMappedRegionInfo(m_clDeviceId, m_pMappedRegion);
   

    // Change status of the command to Gray before handle by the device
    // Color will be changed only when command is submited in the device    
    LogInfoA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), GetId());

    return m_pReceiver->EnqueueDevCommands(m_clDeviceId, m_clDevCmdListId, m_pDevCmd, &m_pStatusChangeObserver, 1);
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code MapMemObjCommand::CommandDone()
{
    // Don't remove buffer pendency, the buffer should be alive at least until unmap is done.
    // Clear allocated data
    if( NULL != m_pDevCmd )
    {
        // Do not delete params since is local to the memory object.
        delete m_pDevCmd;
    }
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
    void* pMappedRegionInfo = m_pMemObject->GetMappedRegionInfo(m_clDeviceId, m_pMappedRegion);
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
    // Create and send unmap command
    m_pDevCmd = new cl_dev_cmd_desc;
    memset(m_pDevCmd, 0, sizeof(cl_dev_cmd_desc));

    m_pDevCmd->id          = (cl_dev_cmd_id)m_pQueueEvent->GetId();
    m_pDevCmd->type        = CL_DEV_CMD_UNMAP;
    m_pDevCmd->param_size  = sizeof(cl_dev_cmd_param_map);
    m_pDevCmd->params      = m_pMemObject->GetMappedRegionInfo(m_clDeviceId, m_pMappedRegion);

    // Color will be changed only when command is submited in the device    
    LogInfoA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), GetId());

	m_pMemObject->SetDataLocation(m_clDeviceId);

    return m_pReceiver->EnqueueDevCommands(m_clDeviceId, m_clDevCmdListId, m_pDevCmd, &m_pStatusChangeObserver, 1);
}

/******************************************************************
 *
 ******************************************************************/
 cl_err_code UnmapMemObjectCommand::CommandDone()
 {
    cl_err_code errVal;
    if( NULL != m_pDevCmd )
    {
        // Delete local command
        delete m_pDevCmd;
    }

    // Here we do the actual operation off releasing the mapped region.
    errVal = m_pMemObject->ReleaseMappedRegion(m_clDeviceId, m_pMappedRegion);
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
        if (!(pMemObj->IsAllocated(m_clDeviceId)))
        {
            // Allocate
            res = pMemObj->CreateDeviceResource(m_clDeviceId);
            if( CL_FAILED(res))
            {
                free(pNewArgs);
                free(ppNewArgsMemLoc);
                return CL_MEM_OBJECT_ALLOCATION_FAILURE;
            }
        }
        // Set the new args list
        cl_dev_mem clDevMemHndl = pMemObj->GetDeviceMemoryHndl(m_clDeviceId);
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
        // TODO: need prefeatching in Native Kernel in case buffers are in different device.
    }

    // Copy the end of the original args
    size_t szLastBytesToCopy = (cl_uchar*)m_pArgs + m_szCbArgs - ((cl_uchar*)(m_ppArgsMemLoc[i-1]) + clMemSize);
    memcpy_s(pCurrentArgsLocation, szCbMaxToCopy, (void*)((cl_uchar*)(m_ppArgsMemLoc[i-1]) + clMemSize),  szLastBytesToCopy);
    szCbMaxToCopy -= szLastBytesToCopy;
    assert( 0 == szCbMaxToCopy);

    //
    // Prepare the device command
    //
    m_pDevCmd = new cl_dev_cmd_desc;
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
    // Fill command descriptor
    m_pDevCmd->id = (cl_dev_cmd_id)m_pQueueEvent->GetId();

    // Sending the queue command
    // TODO: Handle the case were buffers are located in different device. Prefatching
    for( unsigned int i=0; i < m_uNumMemObjects; i++ )
    {
        // Check that mem object is allocated on device, if not allocate resource
        MemoryObject* pMemObj = m_ppMemObjList[i];
	    pMemObj->SetDataLocation(m_clDeviceId);
	}

    LogInfoA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), GetId());

    return m_pReceiver->EnqueueDevCommands(m_clDeviceId, m_clDevCmdListId, m_pDevCmd, &m_pStatusChangeObserver, 1);
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code NativeKernelCommand::CommandDone()
{
    // Clean resources
    if( NULL != m_pDevCmd )
    {
        if( NULL != m_pDevCmd->params )
        {
            cl_dev_cmd_param_native* pNativeKernelParam = (cl_dev_cmd_param_native*)m_pDevCmd->params;
            free(pNativeKernelParam->argv);
            free(pNativeKernelParam->mem_loc);
            delete pNativeKernelParam;
        }
        delete m_pDevCmd;
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
    // Thus, we also create and set the device command approperly as much as we can.
	
	// Add ownership on the object
	m_pKernel->AddPendency();

    // Create args snapshot
    size_t szArgCount = m_pKernel->GetKernelArgsCount();
    const KernelArg* pArg = NULL;
    size_t szCurrentLocation =0;
    size_t szSize = 0;

    cl_uint i;
    // First calculate location and set objects
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
            if (!(pMemObj->IsAllocated(m_clDeviceId)))
            {
                // Allocate
                res = pMemObj->CreateDeviceResource(m_clDeviceId);
                if( CL_FAILED(res))
                {
                    return res;
                }
            }

            szCurrentLocation += szSize;
            m_OclObjects.push_back(pMemObj);
        }
		else if ( pArg->IsSampler() )
		{
            szSize   = sizeof(cl_uint);
			OCLObject* pSampler = (OCLObject*)pArg->GetValue();
			pSampler->AddPendency();
            szCurrentLocation += szSize;
            m_OclObjects.push_back(pSampler);
		}
		else
        {
            //Just calculate the size for allocation
            szCurrentLocation += pArg->GetSize();
        }
    }
    
    // Setup Kernel parameters
    m_pDevCmd = new cl_dev_cmd_desc;
    cl_dev_cmd_param_kernel* pKernelParam = new cl_dev_cmd_param_kernel;
        
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
            cl_dev_mem clDevMem = pMemObj->GetDeviceMemoryHndl(m_clDeviceId);
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
    cl_dev_cmd_param_kernel* pKernelParam = (cl_dev_cmd_param_kernel*)m_pDevCmd->params;
    // Fill command descriptor
    m_pDevCmd->id = (cl_dev_cmd_id)m_pQueueEvent->GetId();

    pKernelParam->kernel = m_pKernel->GetDeviceKernelId(m_clDeviceId);

    // Color will be changed only when command is submited in the device    
    
    // Sending the queue command
    // TODO: Handle the case were buffers are located in different device.

	// Set location
    size_t szArgCount = m_pKernel->GetKernelArgsCount();
    const KernelArg* pArg = NULL;
    for(size_t i=0; i< szArgCount; ++i)
    {
        pArg = m_pKernel->GetKernelArg(i);
        if(pArg->IsMemObject())
        {
            // Create buffer resources here if not available.
            MemoryObject* pMemObj = (MemoryObject*)pArg->GetValue();
	        pMemObj->SetDataLocation(m_clDeviceId);
		}
	}
    LogInfoA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), GetId());

    return m_pReceiver->EnqueueDevCommands(m_clDeviceId, m_clDevCmdListId, m_pDevCmd, &m_pStatusChangeObserver, 1);
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code NDRangeKernelCommand::CommandDone()
{
    // Clear all resources
    // Remove object pendencies

    list<OCLObject*>::iterator it;
    for( it = m_OclObjects.begin(); it != m_OclObjects.end(); it++)
    {
        OCLObject* obj = *it;
        obj->RemovePendency();
    }
    m_OclObjects.clear();

    // Delete local command
    cl_dev_cmd_param_kernel* pKernelParam = (cl_dev_cmd_param_kernel*)m_pDevCmd->params;
    delete[] pKernelParam->arg_values;
    delete pKernelParam;
    delete m_pDevCmd;
    m_pDevCmd = NULL;

	// Remove ownership from the object
	m_pKernel->RemovePendency();

    return CL_SUCCESS;
}


/******************************************************************
 * Command: ReadBufferCommand
 * The functions below implement the Read Buffer functinoality
 *
 ******************************************************************/
ReadBufferCommand::ReadBufferCommand(MemoryObject* pBuffer, size_t szOffset, size_t szCb, void* pDst):
    m_pBuffer(pBuffer),
    m_szOffset(szOffset),
    m_szCb(szCb),
    m_pDst(pDst)
{
}

//////////////////
ReadBufferCommand::~ReadBufferCommand()
{
}

//////////////////
cl_err_code ReadBufferCommand::Init()
{
    m_pBuffer->AddPendency();
    return CL_SUCCESS;
}

/******************************************************************
 * Read buffer reads the data from the current device, eventhough it is in other
 * device that the command device.
 * 
 *
 ******************************************************************/
cl_err_code ReadBufferCommand::Execute()
{   
    cl_err_code res = CL_SUCCESS;

    cl_device_id clDeviceDataLocation = m_pBuffer->GetDataLocation();

    // As long as multiple buffers are not implemented always create on device
    // This is to prevent the case that 0 == clDeviceDataLocation which is not implemented.
    // TODO: handle it correctly where there is more than one device...
    // Note that for now use_host & copy_host may not work
    if(0 == clDeviceDataLocation)
    {
        if (!m_pBuffer->IsAllocated(m_clDeviceId))
        {
            // Allocate
            res = m_pBuffer->CreateDeviceResource(m_clDeviceId);
            if( CL_FAILED(res))
            {
                return res;
            }
        }
        m_pBuffer->SetDataLocation(m_clDeviceId);
        clDeviceDataLocation = m_pBuffer->GetDataLocation();
    }

    if(0 != clDeviceDataLocation)
    {
        m_pDevCmd = create_dev_cmd_rw(
            m_pBuffer->GetDeviceMemoryHndl(clDeviceDataLocation), 
            m_pBuffer->GetType(),
            m_pDst, &m_szOffset, &m_szCb, 0, 0,
            CL_DEV_CMD_READ,
            (cl_dev_cmd_id)m_pQueueEvent->GetId());

        LogInfoA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), GetId());

        // Sending 1 command to the device where the bufer is located now
        res = m_pReceiver->EnqueueDevCommands(clDeviceDataLocation, m_clDevCmdListId, m_pDevCmd, &m_pStatusChangeObserver, 1);
    }
    else
    {
        // TODO: Copy locali from the buffer. Currently not supported. return error        
        LogInfoA("Command - EXECUTE FAILED: %s (Id: %d)", GetCommandName(), GetId());
        res = CL_ERR_FAILURE;
    }
    return res;
}



/******************************************************************
 *
 ******************************************************************/
cl_err_code ReadBufferCommand::CommandDone()
{
    // TODO: copy data from dst to the local buffer.
    m_pBuffer->RemovePendency();
    // Delete local command
    cl_dev_cmd_param_rw* pRWParams = (cl_dev_cmd_param_rw*)m_pDevCmd->params;
    delete pRWParams;
    delete m_pDevCmd;
    m_pDevCmd = NULL;

    return CL_SUCCESS;
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
    void*           pDst
    ):
    m_pImage(pImage),
    m_szRowPitch(szRowPitch),
    m_szSlicePitch(szSlicePitch),
    m_pDst(pDst)
{
    // Set region
    for( int i =0; i<MAX_WORK_DIM; i++)
    {
        m_szOrigin[i] = pszOrigin[i];
        m_szRegion[i] = pszRegion[i];
    }

    if( 0 == szRowPitch )
    {
        // Get original image pitch
        m_szRowPitch = pImage->CalcRowPitchSize(pszRegion);
    }
    if( 0 == szSlicePitch )
    {
        // Get original image pitch
        m_szSlicePitch = pImage->CalcSlicePitchSize(pszRegion);
    }
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
cl_err_code ReadImageCommand::Init()
{
    m_pImage->AddPendency();
    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code ReadImageCommand::Execute()
{
    cl_err_code res = CL_SUCCESS;
    cl_device_id clDeviceDataLocation = m_pImage->GetDataLocation();
    if(0 != clDeviceDataLocation)
    {
        m_pDevCmd = create_dev_cmd_rw(
            m_pImage->GetDeviceMemoryHndl(clDeviceDataLocation),
            m_pImage->GetType(),
            m_pDst, m_szOrigin, m_szRegion, m_szRowPitch, m_szSlicePitch,
            CL_DEV_CMD_READ,
            (cl_dev_cmd_id)m_pQueueEvent->GetId());

        LogInfoA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), GetId());

        // Sending 1 command to the device where the bufer is located now
        res = m_pReceiver->EnqueueDevCommands(clDeviceDataLocation, m_clDevCmdListId, m_pDevCmd, &m_pStatusChangeObserver, 1);
    }
    else
    {
        // TODO: Copy locali from the buffer. Currently not supported. return error
        res = CL_ERR_FAILURE;
    }

    return res;

}

/******************************************************************
 *
 ******************************************************************/
cl_err_code ReadImageCommand::CommandDone()
{
    // TODO: copy data from dst to the local buffer.
    m_pImage->RemovePendency();
    // Delete local command
    if(NULL != m_pDevCmd)
    {
        cl_dev_cmd_param_rw* pRWParams = (cl_dev_cmd_param_rw*)m_pDevCmd->params;
        delete pRWParams;
        delete m_pDevCmd;
        m_pDevCmd = NULL;
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
 * inititate NDRangeKernel and change the device command type
 ******************************************************************/
cl_err_code TaskCommand::Init()
{
    cl_err_code res = NDRangeKernelCommand::Init();
    if ( CL_SUCCEEDED (res) )
    {
        if ( NULL != m_pDevCmd)
        {
            m_pDevCmd->type = CL_DEV_CMD_EXEC_KERNEL;
        }
        else
        {
            res = CL_OUT_OF_HOST_MEMORY;
        }
    }
    return res;
}

/******************************************************************
 *
 ******************************************************************/
WriteBufferCommand::WriteBufferCommand(MemoryObject* pBuffer, size_t szOffset, size_t szCb, const void* pSrc):
    m_pBuffer(pBuffer),
    m_szOffset(szOffset),
    m_szCb(szCb),
    m_cpSrc(pSrc)
{
}


/******************************************************************
 *
 ******************************************************************/
WriteBufferCommand::~WriteBufferCommand()
{
}

//////////////////
cl_err_code WriteBufferCommand::Init()
{
    m_pBuffer->AddPendency();
    return CL_SUCCESS;
}

/******************************************************************
 * Write buffer writes the data to the command device. If the device buffer
 * was never allocated, it is created.
 * It should update the buffer when execute begins and update on Command Done
 * 
 *
 ******************************************************************/
cl_err_code WriteBufferCommand::Execute()
{
    cl_err_code res = CL_SUCCESS;
   
    // First validate that buffer is allocated.
    if (!m_pBuffer->IsAllocated(m_clDeviceId))
    {
        // Allocate
        res = m_pBuffer->CreateDeviceResource(m_clDeviceId);
        if( CL_FAILED(res))
        {
            return res;
        }
    }

    // TODO: Do we want to save copy in the buffer itself?
    m_pDevCmd = create_dev_cmd_rw(
            m_pBuffer->GetDeviceMemoryHndl(m_clDeviceId), 
            CL_MEM_OBJECT_BUFFER,
            (void*)m_cpSrc, &m_szOffset, &m_szCb, 0, 0,
            CL_DEV_CMD_WRITE,
            (cl_dev_cmd_id)m_pQueueEvent->GetId()) ;

    m_pBuffer->SetDataLocation(m_clDeviceId);

    LogInfoA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), GetId());

    // Sending 1 command to the device where the bufer is located now
    res = m_pReceiver->EnqueueDevCommands(m_clDeviceId, m_clDevCmdListId, m_pDevCmd, &m_pStatusChangeObserver, 1);

    return res;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code WriteBufferCommand::CommandDone()
{
    // TODO: Update buffer that transaction is done
    m_pBuffer->RemovePendency();
    if (NULL != m_pDevCmd)
    {
        // Delete local command
        cl_dev_cmd_param_rw* pRWParams = (cl_dev_cmd_param_rw*)m_pDevCmd->params;
        delete pRWParams;
        delete m_pDevCmd;
        m_pDevCmd = NULL;
    }
    return CL_SUCCESS;
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
    ):
    m_pImage(pImage),
    m_szRowPitch(szRowPitch),
    m_szSlicePitch(szSlicePitch),
    m_cpSrc(cpSrc)
{
    // Set region
    for( int i =0; i<MAX_WORK_DIM; i++)
    {
        m_szOrigin[i] = pszOrigin[i];
        m_szRegion[i] = pszRegion[i];
    }
    if( 0 == szRowPitch )
    {
        // Get original image pitch
        m_szRowPitch = pImage->CalcRowPitchSize(pszRegion);
    }
    if( 0 == szSlicePitch )
    {
        // Get original image pitch
        m_szSlicePitch = pImage->CalcSlicePitchSize(pszRegion);
    }

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
cl_err_code WriteImageCommand::Init()
{
    m_pImage->AddPendency();
    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code WriteImageCommand::Execute()
{
    cl_err_code res = CL_SUCCESS;
   
    // First validate that image is allocated.
    if (!m_pImage->IsAllocated(m_clDeviceId))
    {
        // Allocate
        res = m_pImage->CreateDeviceResource(m_clDeviceId);
        if( CL_FAILED(res))
        {
            return res;
        }
    }

    // TODO: Do we want to save copy in the buffer itself?
    m_pDevCmd = create_dev_cmd_rw(
            m_pImage->GetDeviceMemoryHndl(m_clDeviceId), 
            m_pImage->GetType(),
            (void*)m_cpSrc, m_szOrigin, m_szRegion, m_szRowPitch, m_szSlicePitch,
            CL_DEV_CMD_WRITE,
            (cl_dev_cmd_id)m_pQueueEvent->GetId()) ;

    m_pImage->SetDataLocation(m_clDeviceId);

    LogInfoA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), GetId());

    // Sending 1 command to the device where the bufer is located now
    res = m_pReceiver->EnqueueDevCommands(m_clDeviceId, m_clDevCmdListId, m_pDevCmd, &m_pStatusChangeObserver, 1);

    return res;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code WriteImageCommand::CommandDone()
{
    // TODO: Update buffer that transaction is done
    m_pImage->RemovePendency();
    if (NULL != m_pDevCmd)
    {
        // Delete local command
        cl_dev_cmd_param_rw* pRWParams = (cl_dev_cmd_param_rw*)m_pDevCmd->params;
        delete pRWParams;
        delete m_pDevCmd;
        m_pDevCmd = NULL;
    }
    return CL_SUCCESS;
}

/******************************************************************
 * 
 ******************************************************************/
cl_err_code RuntimeCommand::Execute()
{
    LogInfoA("Command - DONE  : %s (Id: %d)", GetCommandName(), GetId());
    CommandDone();
    return m_pQueueEvent->SetEventColor(EVENT_STATE_BLACK);
}

/******************************************************************
 *
 ******************************************************************/
unsigned int __stdcall DummyCommandThreadEntryPoint(void* threadObject)
{
    Sleep(300);
    Command* pCommand = (Command*)threadObject;
    pCommand->GetEvent()->SetEventColor(EVENT_STATE_BLACK);
    return 1;
}


cl_err_code DummyCommand::Execute()
{
    // Start execution thread;
    _beginthreadex(NULL, 0, DummyCommandThreadEntryPoint, this, 0, NULL);
    return CL_SUCCESS;
}


