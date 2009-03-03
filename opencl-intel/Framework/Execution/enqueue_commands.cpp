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
#include "queue_event.h"
#include "cl_memory_object.h"
#include "command_receiver.h"
#include "kernel.h"

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
    cl_dev_mem      clDevMemHndl,
    void*           pData,
    size_t          szCb,
    size_t          szOffset,
    cl_dev_cmd_type clCmdType,
    cl_dev_cmd_id   clCmdId         
    )
{
        // Create Read command
        cl_dev_cmd_desc*     pDevCmd     = new cl_dev_cmd_desc;
	    cl_dev_cmd_param_rw* pRWParams   = new cl_dev_cmd_param_rw;
    	
        memset(pDevCmd, 0, sizeof(cl_dev_cmd_desc));
	    memset(pRWParams, 0, sizeof(cl_dev_cmd_param_rw));
	    
        pRWParams->memObj = clDevMemHndl;
	    pRWParams->ptr = pData;
	    pRWParams->dim_count = 1;
	    pRWParams->region[0] = szCb;
	    pRWParams->origin[0] = szOffset;
	    
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
    m_pDevCmd(NULL)
{
    m_pStatusChangeObserver = this;
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
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code Command::NotifyCmdStatusChanged(cl_dev_cmd_id clCmdId, cl_int iCmdStatus, cl_int iCompletionResult)
{
    switch(iCmdStatus)
    {
    case CL_QUEUED:
        // Fall through
    case CL_SUBMITTED:
        // Nothing to do, not expected to be here at all
        break;
    case CL_RUNNING:
        m_pQueueEvent->SetEventColor(QueueEvent::EVENT_STATE_GRAY);
        break;
    case CL_COMPLETE:
        // Complete command,
        // do that before set event, since side effect of SetEvent(black) may be deleting of this instance.
        CommandDone();
        m_pQueueEvent->SetEventColor(QueueEvent::EVENT_STATE_BLACK);
        break;
    default:        
        break;
    }
    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
CopyBufferToImageCommand::CopyBufferToImageCommand(
    MemoryObject*   srcBuffer, 
    MemoryObject*   dstImage, 
    size_t          srcOffset, 
    const size_t*   dstOrigin[3], 
    const size_t*   region[3]
    )
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
CopyBufferCommand::CopyBufferCommand( MemoryObject* pSrcBuffer, MemoryObject* pDstBuffer, size_t szSrcOffset, size_t szDstOffset, size_t szCb):
    m_pSrcBuffer(pSrcBuffer),
    m_pDstBuffer(pDstBuffer),
    m_szSrcOffset(szSrcOffset),
    m_szDstOffset(szDstOffset),
    m_szCb(szCb)
{   
}

/******************************************************************
 *
 ******************************************************************/
CopyBufferCommand::~CopyBufferCommand()
{
}

/******************************************************************
 * Just mark the buffers as use. The actual copy is determined on execution.
 ******************************************************************/
cl_err_code CopyBufferCommand::Init()
{
    m_pSrcBuffer->AddPendency();
    m_pDstBuffer->AddPendency();
    return CL_SUCCESS;
}

/******************************************************************
 * Copy buffer asks device to perform copy only if both buffers are on
 * the same device. Else, it read from 1 device and write to other. 
 * Either ways, the location of the destation data remain the same, unless 
 * it was never allocated before.
 *
 ******************************************************************/
cl_err_code CopyBufferCommand::Execute()
{
    cl_err_code res = CL_SUCCESS;
    // First check who is responsible for copy...
    cl_device_id clSrcBufferLocation = m_pSrcBuffer->GetDataLocation();
    cl_device_id clDstBufferLocation = m_pDstBuffer->GetDataLocation();

    // Does src buffer is allocated? If not, user problem, do nothing...
    if ( 0 == clSrcBufferLocation && !(m_pSrcBuffer->IsAllocated(0)))
    {
        // Src is not allocate, return device is not executing error, 
        // The working thread will call set command color to black.
        return CL_ERR_FAILURE;
    }

    // If destination is not allocated, allocate it where the src is.
    if ( 0 == clDstBufferLocation && !(m_pDstBuffer->IsAllocated(0)))
    {
        // Allocate
        res = m_pDstBuffer->CreateDeviceResource(clSrcBufferLocation);        
        if( CL_FAILED(res))
        {
            return res;
        }
        clDstBufferLocation = clSrcBufferLocation;
    }

    if ( clDstBufferLocation == clSrcBufferLocation &&  0 == clSrcBufferLocation )
    {   // Src = 0; Dst = 0
        res = CopyHost(); 
    }
    else if ( clDstBufferLocation == clSrcBufferLocation )
    {   // Src = Dst != 0
        res = CopyOnDevice(clSrcBufferLocation);
    }
    else if ( 0 == clSrcBufferLocation)
    {   // Src = 0; Dst != 0
        res = CopyFromHost(clDstBufferLocation);
    }
    else if ( 0 == clDstBufferLocation )
    {   // Src != 0; Dst = 0
        res = CopyToHost(clSrcBufferLocation);
    }
    else
    {   // Src != Dst !=0
        res = CopyFromDevice(clSrcBufferLocation, clDstBufferLocation);
    }
    return res;
}

/******************************************************************
 * Copy buffers on the host, no access to a device,
 * read data from one buffer and update the second buffer data.
 ******************************************************************/
cl_err_code CopyBufferCommand::CopyHost()
{
    void* pData = m_pDstBuffer->GetData();
    pData = (void*)((cl_uchar*)pData+m_szDstOffset);

    // copies m_szCb bytes to pData which is a memory of the dst buffer
    m_pSrcBuffer->ReadData(pData, m_szSrcOffset, m_szCb);

    // Set dst location to the host
    m_pDstBuffer->SetDataLocation(0);

    // Return failure only to signal the process that the command is done, no need to wait for device reaction.
    return CL_ERR_FAILURE;

}

/******************************************************************
 * Use device copy command to copy betweem the buffers.
 * Pre condition for this function is that the 2 buffers are allocated
 * in the device.
 ******************************************************************/
cl_err_code CopyBufferCommand::CopyOnDevice(cl_device_id clDeviceId)
{
    m_pDevCmd = new cl_dev_cmd_desc;
    cl_dev_cmd_param_copy* pCopyParams   = new cl_dev_cmd_param_copy;
	
    memset(m_pDevCmd, 0, sizeof(cl_dev_cmd_desc));
    memset(pCopyParams, 0, sizeof(cl_dev_cmd_param_copy));
    
    pCopyParams->srcMemObj      = m_pSrcBuffer->GetDeviceMemoryHndl(clDeviceId);
    pCopyParams->dstMemObj      = m_pDstBuffer->GetDeviceMemoryHndl(clDeviceId);
    pCopyParams->src_dim_count  = 1;
    pCopyParams->dst_dim_count  = 1;
    pCopyParams->src_origin[0]  = m_szSrcOffset;
    pCopyParams->dst_origin[0]  = m_szDstOffset;
    pCopyParams->region[0]      = m_szCb;
    
    m_pDevCmd->type       = CL_DEV_CMD_COPY;
    m_pDevCmd->id         = (cl_dev_cmd_id)m_pQueueEvent->GetId();
    m_pDevCmd->params     = pCopyParams;
    m_pDevCmd->param_size = sizeof(cl_dev_cmd_param_copy);

    m_pDstBuffer->SetDataLocation(clDeviceId);
    // Sending 1 command to the device where the bufer is located now
    m_pReceiver->EnqueueDevCommands(clDeviceId, m_pDevCmd, &m_pStatusChangeObserver, 1);
    
    return CL_SUCCESS;
}

/******************************************************************
 * This function takes the source data from the source buffer host location
 * and write it to the dst buffer in device clDstDeviceId;
 ******************************************************************/
cl_err_code CopyBufferCommand::CopyFromHost(cl_device_id clDstDeviceId)
{
    void* pData = m_pSrcBuffer->GetData();
    pData = (void*)(((cl_uchar*)pData)+m_szSrcOffset);

    // Initiate read from device
    m_pDevCmd = create_dev_cmd_rw(
            m_pDstBuffer->GetDeviceMemoryHndl(clDstDeviceId), 
            pData, m_szCb, m_szDstOffset, CL_DEV_CMD_WRITE,
            (cl_dev_cmd_id)m_pQueueEvent->GetId());

    // Set new location
    m_pDstBuffer->SetDataLocation(clDstDeviceId);

    // Sending 1 command to the src device
    m_pReceiver->EnqueueDevCommands(clDstDeviceId, m_pDevCmd, &m_pStatusChangeObserver, 1);
    return CL_SUCCESS;
}

/******************************************************************
 * This function copies the data from the clSrcDeviceId device
 * to the dst buffer local memory.
 ******************************************************************/
cl_err_code CopyBufferCommand::CopyToHost(cl_device_id clSrcDeviceId)
{
    void* pData = m_pDstBuffer->GetData();
    pData = (void*)(((cl_uchar*)pData)+m_szDstOffset);

    // Initiate read from device
    m_pDevCmd = create_dev_cmd_rw(
            m_pSrcBuffer->GetDeviceMemoryHndl(clSrcDeviceId), 
            pData, m_szCb, m_szSrcOffset, CL_DEV_CMD_READ,
            (cl_dev_cmd_id)m_pQueueEvent->GetId());

    // Set new location on the host
    m_pDstBuffer->SetDataLocation(0);

    // Sending 1 command to the src device
    m_pReceiver->EnqueueDevCommands(clSrcDeviceId, m_pDevCmd, &m_pStatusChangeObserver, 1);

    return CL_SUCCESS;
}

/******************************************************************
 * Reads data from srcBuffer in device clSrcBufferLocation and
 * write data to dstBuffer in device clDstBufferLocation
 ******************************************************************/
cl_err_code CopyBufferCommand::CopyFromDevice(cl_device_id clSrcDeviceId, cl_device_id clDstDeviceId)
{
    assert("More than 1 device command per queue command is not implemented yet! can't copy between devices" && 0);
    return CL_ERR_FAILURE;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code CopyBufferCommand::CommandDone()
{
    m_pSrcBuffer->RemovePendency();
    m_pDstBuffer->RemovePendency();

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
CopyImageCommand::CopyImageCommand(MemoryObject* image, const size_t* origin[3], const size_t* region[3], size_t row_pitch, size_t slice_pitch, void* dst){

}

/******************************************************************
 *
 ******************************************************************/
CopyImageCommand::~CopyImageCommand(){

}

/******************************************************************
 *
 ******************************************************************/
CopyImageToBufferCommand::CopyImageToBufferCommand(MemoryObject* srcImage, MemoryObject* dstBuffer, const size_t* srcOrigin[3], const size_t* region[3], size_t dstOffset){

}

/******************************************************************
 *
 ******************************************************************/
CopyImageToBufferCommand::~CopyImageToBufferCommand(){

}

/******************************************************************
 *
 ******************************************************************/
MapBufferCommand::MapBufferCommand(MemoryObject* buffer, cl_map_flags map_flags, size_t offset, size_t cb){

}

/******************************************************************
 *
 ******************************************************************/
MapBufferCommand::~MapBufferCommand(){

}

/******************************************************************
 *
 ******************************************************************/
MapImageCommand::MapImageCommand(MemoryObject* image, cl_map_flags map_flags, const size_t* origion[3], const size_t* region[3], size_t* image_row_pitch, size_t* image_slice_pitch){

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
MarkerCommand::MarkerCommand()
{
    //Nothing to do
}

/******************************************************************
 *
 ******************************************************************/
MarkerCommand::~MarkerCommand()
{
    //Nothing to do
}

/******************************************************************
 * Nothing to do except change color, the marker is used only by Runtime to synch points.
 ******************************************************************/
cl_err_code MarkerCommand::Execute()
{    
    // TODO: This may be deadlock due to Completed event. MUST resolve it.
    m_pQueueEvent->SetEventColor(QueueEvent::EVENT_STATE_BLACK);
    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
BarrierCommand::BarrierCommand()
{
    //Nothing to do
}

/******************************************************************
 *
 ******************************************************************/
BarrierCommand::~BarrierCommand()
{
    //Nothing to do
}

/******************************************************************
 * Nothing to do except change color, the marker is used only by Runtime to synch points.
 ******************************************************************/
cl_err_code BarrierCommand::Execute()
{    
    // TODO: This may be deadlock due to Completed event. MUST resolve it.
    m_pQueueEvent->SetEventColor(QueueEvent::EVENT_STATE_BLACK);
    return CL_SUCCESS;
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
    void**  ppNewArgsMemLoc = (void**)malloc(sizeof(void*) * m_uNumMemObjects);
    
    // Set the parameters for the device
    cl_uint i;
    void* pCurrentArgsLocation = pNewArgs;
    size_t szCbToCopy = 0;

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
            memcpy(pCurrentArgsLocation, m_pArgs, szCbToCopy);
        }
        else
        {
            szCbToCopy = (cl_uchar*)(m_ppArgsMemLoc[i]) - (cl_uchar*)(m_ppArgsMemLoc[i-1]) + clMemSize;
            memcpy(pCurrentArgsLocation, (void*)((cl_uchar*)(m_ppArgsMemLoc[i-1]) + clMemSize), szCbToCopy);
        }
        ppNewArgsMemLoc[i] = (void*)((cl_uchar*)pCurrentArgsLocation + szCbToCopy);
        memcpy(ppNewArgsMemLoc[i], &clDevMemHndl, clDevMemSize);
        pCurrentArgsLocation = (cl_uchar*)ppNewArgsMemLoc[i] + clDevMemSize;

        // Set buffers pendencies
        pMemObj->AddPendency();    
    }

    // Copy the end of the original args
    size_t szLastBytesToCopy = (cl_uchar*)m_pArgs + m_szCbArgs - (cl_uchar*)(m_ppArgsMemLoc[i-1]) + clMemSize;
    memcpy(pCurrentArgsLocation, (void*)((cl_uchar*)(m_ppArgsMemLoc[i]) + clMemSize),  szLastBytesToCopy);
    pCurrentArgsLocation = (cl_uchar*)pCurrentArgsLocation + szLastBytesToCopy;

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
    m_pReceiver->EnqueueDevCommands(m_clDeviceId, m_pDevCmd, &m_pStatusChangeObserver, 1);

    return CL_SUCCESS;
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
 * TODO: set buffers handles on Init, no need to TObjectArg, handles are of the current device
 ******************************************************************/
cl_err_code NDRangeKernelCommand::Init()
{
    cl_err_code res;
    // We have to use init to create a snapshot of the buffer kernels on enqueue
    // Thus, we also create and set the device command approperly as much as we can.

    // Create args snapshot
    size_t szArgCount = m_pKernel->GetKernelArgsCount();
    const KernelArg* pArg = NULL;
    size_t szCurrentLocation =0;    

    cl_uint i;
    for(i=0; i< szArgCount; i++)
    {
        pArg = m_pKernel->GetKernelArg(i);
        if(pArg->IsBuffer())
        {
            TObjectArg tObjectArg;
            tObjectArg.type     = KL_ARG_TYPE_BUFFER;
            tObjectArg.szOffset = szCurrentLocation;
            tObjectArg.szSize   = sizeof(cl_dev_mem);
            tObjectArg.pObject   = pArg->GetValue();
            // Create buffer resources here if not available.
            MemoryObject* pBuffer = (MemoryObject*)tObjectArg.pObject;
            // Mark as used
            pBuffer->AddPendency();

            if (!(pBuffer->IsAllocated(m_clDeviceId)))
            {
                // Allocate
                res = pBuffer->CreateDeviceResource(m_clDeviceId);
                if( CL_FAILED(res))
                {
                    return res;
                }
            }


            szCurrentLocation += tObjectArg.szSize;
            m_ObjectArgList.push_back(tObjectArg);

        }
        // Currently suuport only buffer
        // TODO: check other objects such as image, sampler
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
        if(pArg->IsBuffer())
        {
            szArgSize = sizeof(cl_dev_mem);
            // Get device buffer handle only on execute, since the current location is not necessarily the final one.
        }
        // TODO: check other objects such as image, sampler
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
	    pKernelParam->lcl_wrk_size[i] = m_cpszLocalWorkSize[i];
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
    //
    // Now is the time to set the objects
    // TODO: For each buffer check location and add commadn if needed
    //
    cl_dev_cmd_param_kernel* pKernelParam = (cl_dev_cmd_param_kernel*)m_pDevCmd->params;
    
    list<TObjectArg>::iterator it = m_ObjectArgList.begin();    

    while ( it != m_ObjectArgList.end()) 
    {
        TObjectArg& objectArg = *it;
        if (KL_ARG_TYPE_BUFFER == objectArg.type)
        {   
            MemoryObject* pBuffer = (MemoryObject*)(objectArg.pObject);


            // Assume buffer is available in the current location, done in init
            // TODO: Support buffers in different devices, need to prefatch.

            // TODO: Uri Q: when to update the buffer on the location of the device.
            pBuffer->SetDataLocation(m_clDeviceId);
            cl_dev_mem clDevMem = pBuffer->GetDeviceMemoryHndl(m_clDeviceId);

            assert( 0 != clDevMem );
            memcpy(  ((cl_char*)(pKernelParam->arg_values)) + (objectArg.szOffset), &clDevMem , objectArg.szSize);
        }
        else
        {
            // TODO: support samplers/images. Until than don't expect to be here
            assert(0);
        }
        it++;
    }

    // Fill command descriptor
	m_pDevCmd->id = (cl_dev_cmd_id)m_pQueueEvent->GetId();

    pKernelParam->kernel = m_pKernel->GetDeviceKernelId(m_clDeviceId);
    
    // Sending the queue command
    // TODO: Handle the case were buffers are located in different device.
    m_pReceiver->EnqueueDevCommands(m_clDeviceId, m_pDevCmd, &m_pStatusChangeObserver, 1);

    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code NDRangeKernelCommand::CommandDone()
{
    // Clear all resources
    list<TObjectArg>::iterator it = m_ObjectArgList.begin();

    while ( it != m_ObjectArgList.end()) 
    {
        TObjectArg& objectArg = *it;
        if (KL_ARG_TYPE_BUFFER == objectArg.type)
        {
            MemoryObject* buffer = (MemoryObject*)(objectArg.pObject);
            buffer->RemovePendency();
        }
        else
        {
            // TODO: support samplers/images. Until than don't expect to be here
            assert(0);
        }
        it++;
    }

    // Delete local command
    cl_dev_cmd_param_kernel* pKernelParam = (cl_dev_cmd_param_kernel*)m_pDevCmd->params;
    delete pKernelParam->arg_values;
    delete pKernelParam;
    delete m_pDevCmd;
    m_pDevCmd = NULL;

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
    if(0 != clDeviceDataLocation)
    {
        m_pDevCmd = create_dev_cmd_rw(
            m_pBuffer->GetDeviceMemoryHndl(clDeviceDataLocation), 
            m_pDst, m_szCb, m_szOffset, CL_DEV_CMD_READ,
            (cl_dev_cmd_id)m_pQueueEvent->GetId());

        // Sending 1 command to the device where the bufer is located now
        m_pReceiver->EnqueueDevCommands(clDeviceDataLocation, m_pDevCmd, &m_pStatusChangeObserver, 1);
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
ReadImageCommand::ReadImageCommand(MemoryObject* image, const size_t* origin[3], const size_t* region[3], size_t row_pitch, size_t slice_pitch, void* dst){

}

/******************************************************************
 *
 ******************************************************************/
ReadImageCommand::~ReadImageCommand(){

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
UnmapMemObjectCommand::UnmapMemObjectCommand(MemoryObject* memObject, void* mapped_ptr){

}

/******************************************************************
 *
 ******************************************************************/
UnmapMemObjectCommand::~UnmapMemObjectCommand(){

}

/******************************************************************
 *
 ******************************************************************/
WaitForEventsCommand::WaitForEventsCommand()
{
    // Nothing to do
}


/******************************************************************
 *
 ******************************************************************/
WaitForEventsCommand::~WaitForEventsCommand()
{
    // Nothing to do
}

/******************************************************************
 * Nothing to do except change color, the Barrier is used only by Runtime to synch points.
 ******************************************************************/
cl_err_code WaitForEventsCommand::Execute()
{    
    // TODO: This may be deadlock due to Completed event. MUST resolve it.
    m_pQueueEvent->SetEventColor(QueueEvent::EVENT_STATE_BLACK);
    return CL_SUCCESS;
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
            (void*)m_cpSrc, m_szCb, m_szOffset, CL_DEV_CMD_WRITE,
            (cl_dev_cmd_id)m_pQueueEvent->GetId()) ;

    m_pBuffer->SetDataLocation(m_clDeviceId);

    // Sending 1 command to the device where the bufer is located now
    m_pReceiver->EnqueueDevCommands(m_clDeviceId, m_pDevCmd, &m_pStatusChangeObserver, 1);

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
WriteImageCommand::WriteImageCommand(MemoryObject* image, const size_t* origin[3], const size_t* region[3], size_t row_pitch, size_t slice_pitch, const void* dst){

}

/******************************************************************
 *
 ******************************************************************/
WriteImageCommand::~WriteImageCommand(){

}


/******************************************************************
 *
 ******************************************************************/
unsigned int __stdcall DummyCommandThreadEntryPoint(void* threadObject)
{
    Sleep(300);
    Command* pCommand = (Command*)threadObject;
    pCommand->GetEvent()->SetEventColor(QueueEvent::EVENT_STATE_BLACK);
    return 1;
}


cl_err_code DummyCommand::Execute()
{
    // Start execution thread;
    _beginthreadex(NULL, 0, DummyCommandThreadEntryPoint, this, 0, NULL);
    return CL_SUCCESS;
}


