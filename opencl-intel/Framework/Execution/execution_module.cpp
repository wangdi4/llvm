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
//  execution_module.cpp
//  Implementation of the Class ExecutionModule
//  Created on:      23-Dec-2008 3:23:00 PM
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////
#include "execution_module.h"
#include "platform_module.h"
#include "context_module.h"
#include "events_manager.h"
#include "ocl_command_queue.h"
#include "context.h"
#include "enqueue_commands.h"
#include "cl_memory_object.h"
#include "kernel.h"
#include <cl_objects_map.h>
#include <logger.h>

using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;


/******************************************************************
 * Constructor. Only assign pointers, for objects initilaztion use
 * Initialize function immediately. otherwise, the class behaviour
 * is undefined and function calls may crash the system.
 ******************************************************************/
ExecutionModule::ExecutionModule( PlatformModule *pPlatformModule, ContextModule* pContextModule ):
    m_pPlatfromModule(pPlatformModule),
    m_pContextModule(pContextModule),
    m_pOclCommandQueueMap(NULL),
    m_pEventsManager(NULL)
{
	m_pLoggerClient = new LoggerClient(L"Context Module Logger Client",LL_DEBUG);
	InfoLog(m_pLoggerClient, L"ExecutionModule created");
}

/******************************************************************
 * 
 ******************************************************************/
ExecutionModule::~ExecutionModule()
{
    delete m_pLoggerClient;
    // TODO: clear all resources!
}

/******************************************************************
 * This function initialize the execution modeule.
 * If this function fails, the object must be released. 
 * If the caller will not release it, other function will terminate
 * the application.
 ******************************************************************/
cl_err_code ExecutionModule::Initialize()
{
    m_pOclCommandQueueMap = new OCLObjectsMap();
    m_pEventsManager = new EventsManager();
	
    if ( (NULL == m_pOclCommandQueueMap) || ( NULL == m_pEventsManager))
	{
		return CL_ERR_FAILURE;
	}
	return CL_SUCCESS;
}

/******************************************************************
 * 
 ******************************************************************/
cl_command_queue ExecutionModule::CreateCommandQueue(    
    cl_context                  clContext,
    cl_device_id                clDevice,
    cl_command_queue_properties clQueueProperties,
    cl_int*                     pErrRet             
    )
{
    cl_int      iQueueID   = CL_INVALID_HANDLE;
    Context*    pContext   = NULL;
    cl_int      errVal     = CheckCreateCommandQueueParams(clContext, clDevice, clQueueProperties, &pContext);

    // If we are here, all parameters are valid, create the queue
    if( CL_SUCCEEDED(errVal))
    {
        OclCommandQueue* pCommandQueue = new OclCommandQueue(pContext, clDevice, clQueueProperties, m_pEventsManager);
        pCommandQueue->Retain();
        // TODO: gaurd ObjMap... better doing so inside the map        
        m_pOclCommandQueueMap->AddObject((OCLObject*)pCommandQueue);
        errVal = pCommandQueue->Initialize();
        if(CL_SUCCEEDED(errVal))
        {
            iQueueID = pCommandQueue->GetId();
        }
    }
    if (pErrRet) *pErrRet = errVal;
    return (cl_command_queue)iQueueID;
}

/******************************************************************
 * 
 ******************************************************************/
cl_err_code ExecutionModule::CheckCreateCommandQueueParams( cl_context clContext, cl_device_id clDevice, cl_command_queue_properties clQueueProperties, Context** ppContext)
{
    cl_int errVal = CL_SUCCESS;

    // The nested if sentece below validate input parameters;
    *ppContext = m_pContextModule->GetContext(clContext);
    if (NULL == *ppContext)
    {
        errVal = CL_INVALID_CONTEXT;
    }
    // Check if the device is valid
    else if( ! ((*ppContext)->CheckDevices(1, &clDevice)))
    {
        errVal = CL_INVALID_DEVICE;        
    }
    else if ( (clQueueProperties & 0xFFFFFFFF) > ( CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | 
                                                   CL_QUEUE_PROFILING_ENABLE ) ) 
    {
        errVal = CL_INVALID_VALUE;
    }
    return errVal;
}



/******************************************************************
 * This function returns a pointer to a command queue.
 * If the command queue is not available a NULL value is returned.
 ******************************************************************/
OclCommandQueue* ExecutionModule::GetCommandQueue(cl_command_queue clCommandQueue)
{
    cl_err_code errCode;    		
    OCLObject*  pOclObject = NULL;
    errCode = m_pOclCommandQueueMap->GetOCLObject((cl_uint)clCommandQueue, &pOclObject);
    if (CL_FAILED(errCode))
	{
        return NULL;
    }
    OclCommandQueue* pCommandQueue = dynamic_cast<OclCommandQueue*>(pOclObject);
	return pCommandQueue;
}

/******************************************************************
 * 
 ******************************************************************/
cl_err_code ExecutionModule::RetainCommandQueue(cl_command_queue clCommandQueue)
{
    OclCommandQueue* pCommandQueue = GetCommandQueue(clCommandQueue);
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }
    pCommandQueue->Retain();
	return  CL_SUCCESS;
}

/******************************************************************
 * 
 ******************************************************************/
cl_err_code ExecutionModule::ReleaseCommandQueue(cl_command_queue clCommandQueue)
{
    cl_err_code errVal = CL_SUCCESS;
    OclCommandQueue* pOclCommandQueue = GetCommandQueue(clCommandQueue);
    if (NULL == pOclCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }
    errVal = pOclCommandQueue->Release();
    if (CL_SUCCEEDED(errVal))
    {
        // Check is the command has fully released, and if true, destroy it and remove it        
        m_pOclCommandQueueMap->RemoveObject((cl_uint)clCommandQueue, NULL); //TODO: guard this sccess        
        pOclCommandQueue->CleanFinish(); // The CleanFinish signals the queue to finish the commands and to release itself.
    }
	return  errVal;
}

/******************************************************************
 * 
 ******************************************************************/
cl_err_code ExecutionModule::GetCommandQueueInfo( cl_command_queue clCommandQueue, cl_command_queue_info clParamName, size_t szParamValueSize, void* pParamValue, size_t* pszParamValueSizeRet )
{
    cl_err_code res = CL_SUCCESS;
    OclCommandQueue* pOclCommandQueue = GetCommandQueue(clCommandQueue);
    if (NULL == pOclCommandQueue)
    {
        res = CL_INVALID_COMMAND_QUEUE;
    }
    else
    {
        res = pOclCommandQueue->GetInfo(clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);
    }
    return res;
}


/******************************************************************
 * 
 ******************************************************************/
cl_err_code ExecutionModule::SetCommandQueueProperty ( cl_command_queue clCommandQueue, cl_command_queue_properties clProperties, cl_bool bEnable, cl_command_queue_properties* pclOldProperties)
{
    ErrLog(m_pLoggerClient, L"Function: (%s), is not implemented", "SetCommandQueueProperty");
	return  CL_INVALID_OPERATION;	
}

/******************************************************************
 * 
 ******************************************************************/
cl_err_code ExecutionModule::EnqueueMarker(cl_command_queue clCommandQueue, cl_event *pEvent)
{
    cl_err_code errVal;

    if (NULL == pEvent)
    {
        return CL_INVALID_VALUE;
    }

    OclCommandQueue* pCommandQueue = GetCommandQueue(clCommandQueue);
    if (NULL == pCommandQueue || !(pCommandQueue->IsOutOfOrderExecModeEnabled()))
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    // Create Command
    Command* pMarkerCommand = new MarkerCommand();
    errVal = pMarkerCommand->Init();
    if(CL_SUCCEEDED(errVal))
    {
        errVal = pCommandQueue->EnqueueCommand(pMarkerCommand, false, 0, NULL, pEvent);
    }    
	return  errVal;
}

/******************************************************************
 * 
 ******************************************************************/
cl_err_code ExecutionModule::EnqueueWaitForEvents(cl_command_queue clCommandQueue, cl_uint uiNumEvents, const cl_event* cpEventList)
{
    cl_err_code errVal;
    OclCommandQueue* pCommandQueue = GetCommandQueue(clCommandQueue);
    if (NULL == pCommandQueue || !(pCommandQueue->IsOutOfOrderExecModeEnabled()))
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    // Create Command
    Command* pWaitForEventsCommand = new WaitForEventsCommand();
    errVal = pWaitForEventsCommand->Init();
    if(CL_SUCCEEDED(errVal))
    {
        errVal = pCommandQueue->EnqueueCommand(pWaitForEventsCommand, CL_FALSE, uiNumEvents, cpEventList, NULL);
    }    
	return  errVal;
}

/******************************************************************
 * 
 ******************************************************************/
cl_err_code ExecutionModule::EnqueueBarrier(cl_command_queue clCommandQueue)
{
    cl_err_code errVal;
    OclCommandQueue* pCommandQueue = GetCommandQueue(clCommandQueue);
    if (NULL == pCommandQueue || !(pCommandQueue->IsOutOfOrderExecModeEnabled()))
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    // Create Command
    Command* pBarrierCommand = new BarrierCommand();
    errVal = pBarrierCommand->Init();
    if(CL_SUCCEEDED(errVal))
    {
        errVal = pCommandQueue->EnqueueCommand(pBarrierCommand, CL_FALSE, 0, NULL, NULL);
    }    
	return  errVal;
}


/******************************************************************
 * 
 ******************************************************************/
cl_err_code ExecutionModule::WaitForEvents( cl_uint uiNumEvents, const cl_event* cpEventList )
{
    if ( 0 == uiNumEvents)
        return CL_INVALID_VALUE;
    // This call is blocking.
    cl_err_code errVal = m_pEventsManager->WaitForEvents(uiNumEvents, cpEventList);
    if ( CL_INVALID_EVENT_WAIT_LIST == errVal )
    {
        return CL_INVALID_EVENT;
    }

    // TODO: Arnon API Q: Resolve the CL_INVALID_CONTEXT return value.
    // No other API calls that include event_wait_list refer to the problem that events are from different contexts.
    return CL_SUCCESS;
}

/******************************************************************
 * 
 ******************************************************************/
cl_err_code ExecutionModule::GetEventInfo( cl_event clEvent, cl_event_info clParamName, size_t szParamValueSize, void* pParamValue, size_t* pszParamValueSizeRet )
{
    cl_err_code res = m_pEventsManager->GetEventInfo(clEvent, clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);
    return res;
}

/******************************************************************
 * 
 ******************************************************************/
cl_err_code ExecutionModule::RetainEvent(cl_event clEevent)
{
    cl_err_code res = m_pEventsManager->RetainEvent(clEevent);
    if CL_FAILED(res)
    {
        res = CL_INVALID_EVENT;
    }
    return res;
}

/******************************************************************
 * 
 ******************************************************************/
cl_err_code ExecutionModule::ReleaseEvent(cl_event clEvent)
{
    cl_err_code res = m_pEventsManager->ReleaseEvent(clEvent);
    if CL_FAILED(res)
    {
        res = CL_INVALID_EVENT;
    }
    return res;
}

/******************************************************************
 * 
 ******************************************************************/
cl_err_code ExecutionModule::EnqueueReadBuffer(cl_command_queue clCommandQueue, cl_mem clBuffer, cl_bool bBlocking, size_t szOffset, size_t szCb, void* pOutData, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent)
{
    cl_err_code errVal = CL_SUCCESS;
    if (NULL == pOutData)
    {
        return CL_INVALID_VALUE;
    }

    OclCommandQueue* pCommandQueue = GetCommandQueue(clCommandQueue);
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }
    
    MemoryObject* pBuffer = m_pContextModule->GetMemoryObject(clBuffer);
    if (NULL == pBuffer)
    {
        return CL_INVALID_MEM_OBJECT;
    }

    if (pBuffer->GetContextId() != pCommandQueue->GetContextId())
    {
        return CL_INVALID_CONTEXT;
    }

    if (pBuffer->GetSize() < (szOffset+szCb))
    {
        // Out of bounds check.
        return CL_INVALID_VALUE;
    }

    Command* pEnqueueReadBufferCmd = new ReadBufferCommand(pBuffer, szOffset, szCb, pOutData);
    errVal = pEnqueueReadBufferCmd->Init();
    if(CL_SUCCEEDED(errVal))
    {
        errVal = pCommandQueue->EnqueueCommand(pEnqueueReadBufferCmd, bBlocking, uNumEventsInWaitList, cpEeventWaitList, pEvent);
    }    
	return  errVal;
}


/******************************************************************
 * 
 ******************************************************************/
cl_err_code ExecutionModule::EnqueueWriteBuffer(cl_command_queue clCommandQueue, cl_mem clBuffer, cl_bool bBlocking, size_t szOffset, size_t szCb, const void* cpSrcData, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent)
{
    cl_err_code errVal = CL_SUCCESS;
    if (NULL == cpSrcData)
    {
        return CL_INVALID_VALUE;
    }

    OclCommandQueue* pCommandQueue = GetCommandQueue(clCommandQueue);
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }
    
    MemoryObject* pBuffer = m_pContextModule->GetMemoryObject(clBuffer);
    if (NULL == pBuffer)
    {
        return CL_INVALID_MEM_OBJECT;
    }

    if (pBuffer->GetContextId() != pCommandQueue->GetContextId())
    {
        return CL_INVALID_CONTEXT;
    }

    if (pBuffer->GetSize() < (szOffset+szCb))
    {
        // Out of bounds check.
        return CL_INVALID_VALUE;
    }

    Command* pWriteBufferCmd = new WriteBufferCommand(pBuffer, szOffset, szCb, cpSrcData);
    errVal = pWriteBufferCmd->Init();
    if(CL_SUCCEEDED(errVal))
    {
        errVal = pCommandQueue->EnqueueCommand(pWriteBufferCmd, bBlocking, uNumEventsInWaitList, cpEeventWaitList, pEvent);
    }    
	return  errVal;
}


/******************************************************************
 * 
 ******************************************************************/
cl_err_code ExecutionModule::EnqueueCopyBuffer(
    cl_command_queue    clCommandQueue, 
    cl_mem              clSrcBuffer, 
    cl_mem              clDstBuffer, 
    size_t              szSrcOffset, 
    size_t              szDstOffset, 
    size_t              szCb, 
    cl_uint             uNumEventsInWaitList, 
    const cl_event*     cpEeventWaitList, 
    cl_event*           pEvent
    )
{
    cl_err_code errVal = CL_SUCCESS;
    OclCommandQueue* pCommandQueue = GetCommandQueue(clCommandQueue);
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }
    
    MemoryObject* pSrcBuffer = m_pContextModule->GetMemoryObject(clSrcBuffer);
    MemoryObject* pDstBuffer = m_pContextModule->GetMemoryObject(clDstBuffer);
    if (NULL == pSrcBuffer || NULL == pDstBuffer)
    {
        return CL_INVALID_MEM_OBJECT;
    }

    if (pSrcBuffer->GetContextId() != pCommandQueue->GetContextId()  ||
        pSrcBuffer->GetContextId() != pDstBuffer->GetContextId() 
        )
    {
        return CL_INVALID_CONTEXT;
    }

    // Check boundaries.
    if (pSrcBuffer->GetSize() < (szSrcOffset+szCb) || pDstBuffer->GetSize() < (szDstOffset+szCb))
    {
        return CL_INVALID_VALUE;
    }

    if( clSrcBuffer == clDstBuffer)
    {
        // check overlapping
        if ( szDstOffset < (szSrcOffset+szCb) && (szDstOffset+szCb) > szSrcOffset)
        {
            return CL_MEM_COPY_OVERLAP;
        }
    }

    Command* pCopyBufferCommand = new CopyBufferCommand(pSrcBuffer, pDstBuffer, szSrcOffset, szDstOffset, szCb);
    errVal = pCopyBufferCommand->Init();
    if(CL_SUCCEEDED(errVal))
    {
        // Enqueue copy command, never blocking
        errVal = pCommandQueue->EnqueueCommand(pCopyBufferCommand, CL_FALSE, uNumEventsInWaitList, cpEeventWaitList, pEvent);
    }    
	return  errVal;
}

/******************************************************************
 * 
 ******************************************************************/
cl_err_code ExecutionModule::EnqueueUnmapMemObject(cl_command_queue command_queue, cl_mem memobj, void* mapped_ptr, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
{
    ErrLog(m_pLoggerClient, L"Function: (%s), is not implemented", "EnqueueUnmapMemObject");
	return  CL_INVALID_OPERATION;	
}


/******************************************************************
 * 
 ******************************************************************/

/******************************************************************
 * 
 ******************************************************************/
cl_err_code ExecutionModule::EnqueueNDRangeKernel(
    cl_command_queue clCommandQueue, 
    cl_kernel       clKernel,
    cl_uint         uiWorkDim,
    const size_t*   cpszGlobalWorkOffset, 
    const size_t*   cpszGlobalWorkSize, 
    const size_t*   cpszLocalWorkSize, 
    cl_uint         uNumEventsInWaitList, 
    const cl_event* cpEeventWaitList, 
    cl_event*       pEvent
    )
{
    cl_err_code errVal = CL_SUCCESS;

    if( uiWorkDim < 1 || uiWorkDim > 3)
    {
        return CL_INVALID_WORK_DIMENSION;
    }

    if ( NULL != cpszGlobalWorkOffset)
    {
     return CL_INVALID_GLOBAL_OFFSET;
    }

    OclCommandQueue* pCommandQueue = GetCommandQueue(clCommandQueue);
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }
    Kernel* pKernel = m_pContextModule->GetKernel(clKernel);
    if (NULL == pKernel)
    {
        return CL_INVALID_KERNEL;
    }

    if ((cl_context)(pKernel->GetContext()->GetId()) != pCommandQueue->GetContextId())
    {
        return CL_INVALID_CONTEXT;
    }

    // TODO: Handle those error values, probably through the kernel object...
    // CL_INVALID_KERNEL_ARGS
    // CL_INVALID_PROGRAM_EXECUTABLE
    // CL_INVALID_WORK_GROUP_SIZE
    // CL_INVALID_WORK_ITEM_SIZE
   
    // TODO: create buffer resources in advance, if they are not exists,
    //      On error return: CL_OUT_OF_RESOURCES


    Command* pNDRangeKernelCmd = new NDRangeKernelCommand(pKernel, uiWorkDim, cpszGlobalWorkOffset, cpszGlobalWorkSize, cpszLocalWorkSize); 
    // Must set device Id befor init for buffer resource allocation.
    pNDRangeKernelCmd->SetCommandDeviceId(pCommandQueue->GetQueueDeviceId());
    errVal = pNDRangeKernelCmd->Init();
    if( CL_SUCCEEDED (errVal) )
    {
        errVal = pCommandQueue->EnqueueCommand(pNDRangeKernelCmd, false/*never blocking*/, uNumEventsInWaitList, cpEeventWaitList, pEvent);
    }
    
	return  errVal;

}


/******************************************************************
 * 
 ******************************************************************/
cl_err_code ExecutionModule::EnqueueTask( cl_command_queue clCommandQueue, cl_kernel clKernel, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent)
{
    cl_err_code errVal = CL_SUCCESS;

    OclCommandQueue* pCommandQueue = GetCommandQueue(clCommandQueue);
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }
    Kernel* pKernel = m_pContextModule->GetKernel(clKernel);
    if (NULL == pKernel)
    {
        return CL_INVALID_KERNEL;
    }

    if ((cl_context)(pKernel->GetContext()->GetId()) != pCommandQueue->GetContextId())
    {
        return CL_INVALID_CONTEXT;
    }

    // TODO: Handle those error values, probably through the kernel object...
    // CL_INVALID_PROGRAM_EXECUTABLE
    // CL_INVALID_WORK_GROUP_SIZE

    Command* pTaskCommand = new TaskCommand(pKernel); 
    // Must set device Id befor init for buffer resource allocation.
    pTaskCommand->SetCommandDeviceId(pCommandQueue->GetQueueDeviceId());    
    errVal = pTaskCommand->Init();
    if(CL_SUCCEEDED(errVal))
    {
        errVal = pCommandQueue->EnqueueCommand(pTaskCommand, false/*never blocking*/, uNumEventsInWaitList, cpEeventWaitList, pEvent);
    }
    
	return  errVal;

}

/******************************************************************
 * 
 ******************************************************************/
cl_err_code ExecutionModule::EnqueueNativeKernel(cl_command_queue clCommandQueue, void (*pUserFnc)(void *), void* pArgs, size_t szCbArgs, cl_uint uNumMemObjects, const cl_mem* clMemList, const void** ppArgsMemLoc, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent)
{
    cl_err_code errVal = CL_SUCCESS;

    // First check NULL values:
    if (    ( NULL == pUserFnc)                                                     ||
            ( NULL == pArgs && ((szCbArgs > 0) || uNumMemObjects > 0 ))             ||
            ( NULL != pArgs && 0 == szCbArgs)                                       ||
            ( (uNumMemObjects >  0) && ( NULL == clMemList || NULL == ppArgsMemLoc))||
            ( (0 == uNumMemObjects) && ( NULL != clMemList || NULL != ppArgsMemLoc)) )
    {
        return CL_INVALID_VALUE;
    }

    OclCommandQueue* pCommandQueue = GetCommandQueue(clCommandQueue);
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    // Create MemoryObjects references
    MemoryObject** pMemObjectsList = (MemoryObject**)malloc(sizeof(MemoryObject*) * uNumMemObjects);
    
    cl_uint i;
    for( i=0; i < uNumMemObjects; i++ )
    {
        // Check that buffer is available
        pMemObjectsList[i] = m_pContextModule->GetMemoryObject(clMemList[i]);
        if ( NULL == pMemObjectsList[i] )
        {
            free(pMemObjectsList);
            return CL_INVALID_MEM_OBJECT;
        }
    }

    // TODO: Handle those error values, probably through the DEVICE object...
    // CL_INVALID_OPERATION
   
    Command* pNativeKernelCommand = new NativeKernelCommand( pUserFnc, pArgs, szCbArgs, uNumMemObjects, pMemObjectsList, ppArgsMemLoc );
    // Must set device Id befor init for buffer resource allocation.
    pNativeKernelCommand->SetCommandDeviceId(pCommandQueue->GetQueueDeviceId());
    errVal = pNativeKernelCommand->Init();
    if( CL_SUCCEEDED (errVal) )
    {
        errVal = pCommandQueue->EnqueueCommand(pNativeKernelCommand, false/*never blocking*/, uNumEventsInWaitList, cpEeventWaitList, pEvent);
    }
    
	return  errVal;

}


/******************************************************************
 * THE FOLLOWING FUNCTIONS ARE NOT IMPLMENTED YET
 ******************************************************************/


cl_err_code ExecutionModule::EnqueueReadImage(cl_command_queue command_queue, cl_mem image, cl_bool blocking_read, const size_t* origin[3], const size_t* region[3], size_t row_pitch, size_t slice_pitch, void* ptr, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* pEvent)
{
    ErrLog(m_pLoggerClient, L"Function: (%s), is not implemented", "EnqueueReadImage");
	return  CL_INVALID_OPERATION;	
}


cl_err_code ExecutionModule::EnqueueWriteImage(cl_command_queue command_queue, cl_mem image, cl_bool blocking_write, const size_t* origin[3], const size_t* region[3], size_t row_pitch, size_t slice_pitch, const void* ptr, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* pEvent)
{
    ErrLog(m_pLoggerClient, L"Function: (%s), is not implemented", "EnqueueWriteImage");
	return  CL_INVALID_OPERATION;	
}


cl_err_code ExecutionModule::EnqueueCopyImage(cl_command_queue command_queue, cl_mem src_image, cl_mem dst_image, const size_t* src_origin[3], const size_t* dst_origin[3], const size_t* region[3], cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* pEvent)
{
    ErrLog(m_pLoggerClient, L"Function: (%s), is not implemented", "EnqueueCopyImage");
	return  CL_INVALID_OPERATION;	
}


cl_err_code ExecutionModule::EnqueueCopyImageToBuffer(cl_command_queue command_queue, cl_mem src_image, cl_mem dst_buffer, const size_t* src_origin[3], const size_t* region[3], size_t dst_offset, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* pEvent)
{
    ErrLog(m_pLoggerClient, L"Function: (%s), is not implemented", "EnqueueCopyImageToBuffer");
	return  CL_INVALID_OPERATION;	
}


cl_err_code ExecutionModule::EnqueueCopyBufferToImage(cl_command_queue command_queue, cl_mem src_buffer, cl_mem dst_image, size_t src_offset, const size_t* dst_origin[3], const size_t* region[3], cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* pEvent)
{
    ErrLog(m_pLoggerClient, L"Function: (%s), is not implemented", "EnqueueCopyBufferToImage");
	return  CL_INVALID_OPERATION;	
}


void * ExecutionModule::EnqueueMapBuffer(cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_map, cl_map_flags map_flags, size_t offset, size_t cb, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* pEvent, cl_int* errcode_ret)
{
    ErrLog(m_pLoggerClient, L"Function: (%s), is not implemented", "EnqueueMapBuffer");
    *errcode_ret = CL_INVALID_OPERATION;
	return  NULL;	
}


void * ExecutionModule::EnqueueMapImage(cl_command_queue command_queue, cl_mem image, cl_bool blocking_map, cl_map_flags map_flags, const size_t* origin[3], const size_t* region[3], size_t* image_row_pitch, size_t* image_slice_pitch, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* pEvent, cl_int* errcode_ret)
{
    ErrLog(m_pLoggerClient, L"Function: (%s), is not implemented", "EnqueueMapImage");
    *errcode_ret = CL_INVALID_OPERATION;
	return  NULL;	
}


/*******************************************************************
 * END OF UN IMPLEMENTED FUNCTUIONS
 *******************************************************************/
