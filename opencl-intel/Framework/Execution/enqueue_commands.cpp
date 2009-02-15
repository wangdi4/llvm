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
CopyBufferCommand::CopyBufferCommand(MemoryObject* srcBuffer, MemoryObject* dstBuffer, size_t srcOffset, size_t dstOffset, size_t cb)
{
}


/******************************************************************
 *
 ******************************************************************/
CopyBufferCommand::~CopyBufferCommand(){

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
MapImageCommand::~MapImageCommand(){

}

/******************************************************************
 *
 ******************************************************************/
MarkerCommand::MarkerCommand(){

}

/******************************************************************
 *
 ******************************************************************/
MarkerCommand::~MarkerCommand(){

}

/******************************************************************
 *
 ******************************************************************/
BarrierCommand::BarrierCommand(){

}

/******************************************************************
 *
 ******************************************************************/
BarrierCommand::~BarrierCommand(){

}

/******************************************************************
 *
 ******************************************************************/
NativeKernelCommand::NativeKernelCommand(void* usrfunc, void* args, size_t cbArgs, cl_uint numMemObjects, const MemoryObject* memObjList, const void** args_mem_loc){

}

/******************************************************************
 *
 ******************************************************************/
NativeKernelCommand::~NativeKernelCommand(){

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
 *
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
            pBuffer->AddPendency();
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
        cl_dev_mem clDevMem = m_pBuffer->GetDeviceMemoryHndl(clDeviceDataLocation);

        // Create Read command
        m_pDevCmd     = new cl_dev_cmd_desc;
	    cl_dev_cmd_param_rw* pRWParams   = new cl_dev_cmd_param_rw;
    	
        memset(m_pDevCmd, 0, sizeof(cl_dev_cmd_desc));
	    memset(pRWParams, 0, sizeof(cl_dev_cmd_param_rw));
	    
        pRWParams->memObj = clDevMem;
	    pRWParams->ptr = m_pDst;
	    pRWParams->dim_count = 1;
	    pRWParams->region[0] = m_szCb;
	    pRWParams->origin[0] = m_szOffset;
	    
        m_pDevCmd->type = CL_DEV_CMD_READ;
	    m_pDevCmd->id = (cl_dev_cmd_id)m_pQueueEvent->GetId();
	    m_pDevCmd->params = pRWParams;
	    m_pDevCmd->param_size = sizeof(cl_dev_cmd_param_rw);

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
TaskCommand::TaskCommand(Kernel* kernel){

}

/******************************************************************
 *
 ******************************************************************/
TaskCommand::~TaskCommand(){

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
WaitForEventsCommand::WaitForEventsCommand(){

}


/******************************************************************
 *
 ******************************************************************/
WaitForEventsCommand::~WaitForEventsCommand(){

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

    // TODO: Uri Q: when to update the buffer on the location of the device.
    // Do we want to save copy in the buffer itself?
    m_pBuffer->SetDataLocation(m_clDeviceId);
        
    // Create Write command
    cl_dev_cmd_desc*        m_pDevCmd   = new cl_dev_cmd_desc;
    cl_dev_cmd_param_rw*    pRWParams   = new cl_dev_cmd_param_rw;
    	
    memset(m_pDevCmd, 0, sizeof(cl_dev_cmd_desc));
	memset(pRWParams, 0, sizeof(cl_dev_cmd_param_rw));
	    
     cl_dev_mem clDevMem = m_pBuffer->GetDeviceMemoryHndl(m_clDeviceId);
    
    pRWParams->memObj   = clDevMem;
	pRWParams->ptr      = (void*)m_cpSrc;
    pRWParams->dim_count = 1;
	pRWParams->region[0] = m_szCb;
	pRWParams->origin[0] = m_szOffset;
	    
    m_pDevCmd->type = CL_DEV_CMD_WRITE;
    m_pDevCmd->id = (cl_dev_cmd_id)m_pQueueEvent->GetId();
    m_pDevCmd->params = pRWParams;
	m_pDevCmd->param_size = sizeof(cl_dev_cmd_param_rw);

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


