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
#include "device.h"
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
	INIT_LOGGER_CLIENT(L"ExecutionModel",LL_DEBUG);

    LOG_DEBUG(L"ExecutionModule created");
}

/******************************************************************
 * 
 ******************************************************************/
ExecutionModule::~ExecutionModule()
{
	RELEASE_LOGGER_CLIENT;
    // TODO: clear all resources!
}

/******************************************************************
 * This function initialize the execution modeule.
 * If this function fails, the object must be released. 
 * If the caller will not release it, other function will terminate
 * the application.
 ******************************************************************/
cl_err_code ExecutionModule::Initialize(ocl_entry_points * pOclEntryPoints)
{
    m_pOclCommandQueueMap = new OCLObjectsMap();
    m_pEventsManager = new EventsManager();

	m_pOclEntryPoints = pOclEntryPoints;
    
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
        errVal = pCommandQueue->Initialize();
        if(CL_SUCCEEDED(errVal))
        {
			// TODO: guard ObjMap... better doing so inside the map        
			m_pOclCommandQueueMap->AddObject((OCLObject*)pCommandQueue);
            iQueueID = pCommandQueue->GetId();
        }
		else
		{
			delete pCommandQueue;
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

    // The nested if sentence below validate input parameters;
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
	LOG_INFO(L"Enter");
    cl_err_code errVal = CL_SUCCESS;
    OclCommandQueue* pOclCommandQueue = GetCommandQueue(clCommandQueue);
    if (NULL == pOclCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }
    errVal = pOclCommandQueue->Release();
	LOG_DEBUG(L"OCL Queue ref count - %d", pOclCommandQueue->GetReferenceCount() );   
    if ( CL_SUCCEEDED(errVal) && 
		( 0 == pOclCommandQueue->GetReferenceCount())    
		)
    {
        // Check if the command has fully released, and if true, destroy it and remove it        
		LOG_DEBUG(L"Remove queue from the map");   
        m_pOclCommandQueueMap->RemoveObject((cl_uint)clCommandQueue, NULL); //TODO: guard this access        
        pOclCommandQueue->CleanFinish(); // The CleanFinish signals the queue to finish the commands and to release itself.
    }
	LOG_INFO(L"Exit - retVal=0x%X", errVal);
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
    OclCommandQueue* pOclCommandQueue = GetCommandQueue(clCommandQueue);
    if (NULL == pOclCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    // Set old properties if not NULL
    if(NULL != pclOldProperties)
    {
        *pclOldProperties = 0x0;
        if( pOclCommandQueue->IsProfilingEnabled() ) 
        {
            *pclOldProperties |= CL_QUEUE_PROFILING_ENABLE;
        }
        if( pOclCommandQueue->IsOutOfOrderExecModeEnabled() ) 
        {
            *pclOldProperties |= CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE;
        }
    }

    if( (0 == clProperties) && ( CL_TRUE == bEnable))
    {
        // Disable all like in clProperties0 in clCreateQueue
        pOclCommandQueue->EnableProfiling( false );
        pOclCommandQueue->EnableOutOfOrderExecMode( false );
    }
    else
    {
		// Check that only properties that are defined by the spec are in use
		cl_command_queue_properties mask = ( CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE 
										   | CL_QUEUE_PROFILING_ENABLE );

        if ( ( clProperties & mask ) == 0 )
        {
            return CL_INVALID_VALUE;
        }

		mask ^= 0xFFFFFFFFFFFFFFFF; //mask = not(mask)

        if ( ( clProperties & mask ) != 0 )
        {
            return CL_INVALID_VALUE;
        }

        // Check that the queue supports those properties
        if( ! pOclCommandQueue->IsPropertiesSupported(clProperties) )
        {
            return CL_INVALID_QUEUE_PROPERTIES;
        }

        if(CL_QUEUE_PROFILING_ENABLE == (clProperties & CL_QUEUE_PROFILING_ENABLE))
        {
            // Asked to set profiling
            pOclCommandQueue->EnableProfiling( bEnable );
        }
        
        if( CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE == (clProperties & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE))
        {
            // Asked to set execution mdoe
            pOclCommandQueue->EnableOutOfOrderExecMode( bEnable );  
        }
    }
    return CL_SUCCESS;
}

/******************************************************************
 * On flush, the implementation always create a flush command
 * Enqueue 
 ******************************************************************/
cl_err_code ExecutionModule::Flush ( cl_command_queue clCommandQueue )
{
	cl_start;
    cl_err_code res = CL_SUCCESS;
    OclCommandQueue* pOclCommandQueue = GetCommandQueue(clCommandQueue);
    if (NULL == pOclCommandQueue)
    {
        res = CL_INVALID_COMMAND_QUEUE;
    }
    else
    {
        res = pOclCommandQueue->Flush();
    }
    cl_return res;
}

/******************************************************************
 * This is a blocking function, works like flush, but since
 * it is blocking, the OclQueue handles the flush and wait mechanism
 ******************************************************************/
cl_err_code ExecutionModule::Finish ( cl_command_queue clCommandQueue)
{
    cl_err_code res = CL_SUCCESS;
    OclCommandQueue* pOclCommandQueue = GetCommandQueue(clCommandQueue);
    if (NULL == pOclCommandQueue)
    {
        res = CL_INVALID_COMMAND_QUEUE;
    }
    else
    {
        res = pOclCommandQueue->Finish();
    }
    return res;
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
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
   } 

    // Create Command
    Command* pMarkerCommand = new MarkerCommand();
    errVal = pMarkerCommand->Init();
    if(CL_SUCCEEDED(errVal))
    {
        errVal = pCommandQueue->EnqueueCommand(pMarkerCommand, false, 0, NULL, pEvent);
        if(CL_FAILED(errVal))
        {
            // Enqueue failed, free resources
            pMarkerCommand->CommandDone();
            delete pMarkerCommand;
        }
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
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

	if ( (NULL == cpEventList) || (0 == uiNumEvents) )
	{
		return CL_INVALID_VALUE;
	}

    // Create Command
    Command* pWaitForEventsCommand = new WaitForEventsCommand();
    errVal = pWaitForEventsCommand->Init();
    if(CL_SUCCEEDED(errVal))
    {
        errVal = pCommandQueue->EnqueueCommand(pWaitForEventsCommand, CL_FALSE, uiNumEvents, cpEventList, NULL);
        if(CL_FAILED(errVal))
        {
            // Enqueue failed, free resources
            pWaitForEventsCommand->CommandDone();
            delete pWaitForEventsCommand;
        }
    }    
	if( CL_INVALID_EVENT_WAIT_LIST == errVal )
	{
		errVal = CL_INVALID_EVENT;
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
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    // Create Command
    Command* pBarrierCommand = new BarrierCommand();
    errVal = pBarrierCommand->Init();
    if(CL_SUCCEEDED(errVal))
    {
        errVal = pCommandQueue->EnqueueCommand(pBarrierCommand, CL_FALSE, 0, NULL, NULL);
        if(CL_FAILED(errVal))
        {
            // Enqueue failed, free resources
            pBarrierCommand->CommandDone();
            delete pBarrierCommand;
        }
    }    
    return  errVal;
}


/******************************************************************
 * 
 ******************************************************************/
cl_err_code ExecutionModule::WaitForEvents( cl_uint uiNumEvents, const cl_event* cpEventList )
{
	cl_start;
    cl_err_code errVal;
    if ( 0 == uiNumEvents)
        return CL_INVALID_VALUE;

    // Validate event context
    cl_context clEventsContext = 0;
    errVal = m_pEventsManager->ValidateEventsContext(uiNumEvents, cpEventList, &clEventsContext);
    if ( CL_FAILED(errVal) )
    {
        return errVal;
    }

    // Before waiting all on events, the function need to flush all relevant queues, 
    // Since the dependencies between events in different queues is unknown it is better
    // to flush all queues in the context.
    OCLObject* pObj = NULL;	
	for (cl_uint ui=0; ui<m_pOclCommandQueueMap->Count(); ++ui)
	{
		errVal = m_pOclCommandQueueMap->GetObjectByIndex(ui, &pObj);
        if ( CL_FAILED(errVal) )
        {
            return errVal;
        }
        OclCommandQueue* pQueue = (OclCommandQueue*)pObj;

        cl_context queueContext = pQueue->GetContextId();
        if(queueContext == clEventsContext)
        {
            // Flush
            pQueue->Flush();
        }
    }

    // This call is blocking.    
    errVal = m_pEventsManager->WaitForEvents(uiNumEvents, cpEventList);
    if ( CL_INVALID_EVENT_WAIT_LIST == errVal )
    {
        return CL_INVALID_EVENT;
    }
    cl_return CL_SUCCESS;
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

    if (!(pBuffer->CheckBounds(&szOffset, &szCb)))
    {
        // Out of bounds check.
        return CL_INVALID_VALUE;
    }

    Command* pEnqueueReadBufferCmd = new ReadBufferCommand(pBuffer, szOffset, szCb, pOutData);
    errVal = pEnqueueReadBufferCmd->Init();
    if(CL_SUCCEEDED(errVal))
    {
        errVal = pCommandQueue->EnqueueCommand(pEnqueueReadBufferCmd, bBlocking, uNumEventsInWaitList, cpEeventWaitList, pEvent);
        if(CL_FAILED(errVal))
        {
            // Enqueue failed, free resources
            pEnqueueReadBufferCmd->CommandDone();
            delete pEnqueueReadBufferCmd;
        }
    }    
    return  errVal;
}


/******************************************************************
 * 
 ******************************************************************/
cl_err_code ExecutionModule::EnqueueWriteBuffer(cl_command_queue clCommandQueue, cl_mem clBuffer, cl_bool bBlocking, size_t szOffset, size_t szCb, const void* cpSrcData, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent)
{
	cl_start;
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

    if (!(pBuffer->CheckBounds(&szOffset, &szCb)))
    {
        // Out of bounds check.
        return CL_INVALID_VALUE;
    }

    Command* pWriteBufferCmd = new WriteBufferCommand(pBuffer, szOffset, szCb, cpSrcData);
    errVal = pWriteBufferCmd->Init();
    if(CL_SUCCEEDED(errVal))
    {
        errVal = pCommandQueue->EnqueueCommand(pWriteBufferCmd, bBlocking, uNumEventsInWaitList, cpEeventWaitList, pEvent);
        if(CL_FAILED(errVal))
        {
            // Enqueue failed, free resources
            pWriteBufferCmd->CommandDone();
            delete pWriteBufferCmd;
        }
    }    
    cl_return  errVal;
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
    if (!(pSrcBuffer->CheckBounds(&szSrcOffset,&szCb)) || !(pDstBuffer->CheckBounds(&szDstOffset,&szCb)))
    {
        return CL_INVALID_VALUE;
    }

    if( clSrcBuffer == clDstBuffer)
    {
        // Check overlapping
        if (CheckMemoryObjectOverlapping(pSrcBuffer, &szSrcOffset, &szDstOffset, &szCb))
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
        if(CL_FAILED(errVal))
        {
            // Enqueue failed, free resources
            pCopyBufferCommand->CommandDone();
            delete pCopyBufferCommand;
        }
    }    
    return  errVal;
}

/******************************************************************
 * 
 ******************************************************************/
void * ExecutionModule::EnqueueMapBuffer(cl_command_queue clCommandQueue, cl_mem clBuffer, cl_bool bBlockingMap, cl_map_flags clMapFlags, size_t szOffset, size_t szCb, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent, cl_int* pErrcodeRet)
{   
	cl_int err = CL_SUCCESS;
	if (NULL == pErrcodeRet)
	{
		pErrcodeRet = &err;
	}
    OclCommandQueue* pCommandQueue = GetCommandQueue(clCommandQueue);
    if (NULL == pCommandQueue)
    {
        *pErrcodeRet = CL_INVALID_COMMAND_QUEUE;
        return NULL;
    }

    // Check that flags CL_MAP_READ or CL_MAP_WRITE only
    if (  (CL_MAP_READ | CL_MAP_WRITE | clMapFlags) != (CL_MAP_READ | CL_MAP_WRITE) )
    {
        *pErrcodeRet = CL_INVALID_VALUE;
        return NULL;
    }


    MemoryObject* pBuffer = m_pContextModule->GetMemoryObject(clBuffer);    
    if (NULL == pBuffer)
    {
        *pErrcodeRet =  CL_INVALID_MEM_OBJECT;
        return NULL;
    }

    if (pBuffer->GetContextId() != pCommandQueue->GetContextId())
    {
        *pErrcodeRet = CL_INVALID_CONTEXT;
        return NULL;
    }
    
    if (pBuffer->GetSize() < (szOffset+szCb))
    {
        // Out of bounds check.
         *pErrcodeRet =  CL_INVALID_VALUE;
         return NULL;
    }
    
    MapBufferCommand* pMapBufferCommand = new MapBufferCommand( pBuffer, clMapFlags, szOffset, szCb);
    // Must set device Id before init for buffer resource allocation.
    pMapBufferCommand->SetCommandDeviceId(pCommandQueue->GetQueueDeviceId());
    *pErrcodeRet = pMapBufferCommand->Init();
    // Get pointer for mapped region since it is allocated on init. Execute will lock the region
    // Note that if EnqueueCommand successed, by the time it returns, the command may be deleted already.
    void* mappedRegion = pMapBufferCommand->GetMappedRegion();
    if(CL_SUCCEEDED(*pErrcodeRet))
    {
        *pErrcodeRet = pCommandQueue->EnqueueCommand(pMapBufferCommand, bBlockingMap, uNumEventsInWaitList, cpEeventWaitList, pEvent);
        if(CL_FAILED(*pErrcodeRet))
        {
            // Enqueue failed, free resources
            pMapBufferCommand->CommandDone();
            delete pMapBufferCommand;
            return NULL;
        }
        return mappedRegion;
    }  
    else
    {
        return  NULL;
    }
}

/******************************************************************
 * 
 ******************************************************************/
cl_err_code ExecutionModule::EnqueueUnmapMemObject(cl_command_queue clCommandQueue,cl_mem clMemObj, void* mappedPtr, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent)
{
    cl_err_code errVal;
    OclCommandQueue* pCommandQueue = GetCommandQueue(clCommandQueue);
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }
    
    MemoryObject* pMemObject = m_pContextModule->GetMemoryObject(clMemObj);    
    if (NULL == pMemObject)
    {
        return  CL_INVALID_MEM_OBJECT;
    }

    if (pMemObject->GetContextId() != pCommandQueue->GetContextId())
    {
        return CL_INVALID_CONTEXT;
    }
    
    Command* pUnmapMemObjectCommand = new UnmapMemObjectCommand( pMemObject, mappedPtr);
    // Must set device Id before init for buffer resource allocation.
    pUnmapMemObjectCommand->SetCommandDeviceId(pCommandQueue->GetQueueDeviceId());
    errVal = pUnmapMemObjectCommand->Init();
    if(CL_SUCCEEDED(errVal))
    {
        errVal = pCommandQueue->EnqueueCommand(pUnmapMemObjectCommand, CL_FALSE /*never blocks*/, uNumEventsInWaitList, cpEeventWaitList, pEvent);
        if(CL_FAILED(errVal))
        {
            // Enqueue failed, free resources
            pUnmapMemObjectCommand->CommandDone();
            delete pUnmapMemObjectCommand;
        }
    }  
    
    return  errVal;
}

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

	if ( NULL == cpszGlobalWorkSize )
	{
		return CL_INVALID_GLOBAL_WORK_SIZE;
	}

	for ( cl_uint ui = 0; ui < uiWorkDim; ui++ )
	{
		if ( cpszGlobalWorkSize[ui] == 0 )
		{
			return CL_INVALID_GLOBAL_WORK_SIZE;
		}
	}

    OclCommandQueue* pCommandQueue = GetCommandQueue(clCommandQueue);
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }
    cl_device_id clDeviceId = pCommandQueue->GetQueueDeviceId();
    Kernel* pKernel = m_pContextModule->GetKernel(clKernel);
    if (NULL == pKernel)
    {
        return CL_INVALID_KERNEL;
    }

    if ((cl_context)(pKernel->GetContext()->GetId()) != pCommandQueue->GetContextId())
    {
        return CL_INVALID_CONTEXT;
    }

    // CL_INVALID_PROGRAM_EXECUTABLE if there is no successfully built program
    // executable available for device associated with command_queue.
    if(!pKernel->IsValidExecutable(clDeviceId))
    {
        return CL_INVALID_PROGRAM_EXECUTABLE;
    }
    

    // CL_INVALID_KERNEL_ARGS if the kernel argument values have not been specified.
    if(!pKernel->IsValidKernelArgs())
    {
        return CL_INVALID_KERNEL_ARGS;
    }

    //
    // Query kernel info to validate input params
    //
    size_t szWorkGroupSize = 0;
    size_t szComplieWorkGroupSize[3] = {0};
    pKernel->GetWorkGroupInfo(pCommandQueue->GetQueueDeviceId(), CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &szWorkGroupSize, NULL);
    pKernel->GetWorkGroupInfo(pCommandQueue->GetQueueDeviceId(), CL_KERNEL_COMPILE_WORK_GROUP_SIZE, sizeof(size_t) * 3, szComplieWorkGroupSize, NULL);
    cl_uint ui=0;

    // If the work-group size is not specified in kernel using the above attribute qualifier (0, 0,0) 
    // is returned in szComplieWorkGroupSize
    if( ! ( (0 == szComplieWorkGroupSize[0]) && 
            (0 == szComplieWorkGroupSize[1]) &&
            (0 == szComplieWorkGroupSize[2])))
    {
        // case kernel using the __attribute__((reqd_work_group_size(X, Y, Z))) qualifier in program source.
        if (  NULL == cpszLocalWorkSize )
        {
            return CL_INVALID_WORK_GROUP_SIZE;
        }
        else
        {
            for( ui=0; ui<uiWorkDim; ui++)
            {
                if( szComplieWorkGroupSize[ui] != cpszLocalWorkSize[ui])
                {
                    return CL_INVALID_WORK_GROUP_SIZE;
                }
            }

        }
    }
    // Check that if local_work_size is specified and number of workitems specified by 
    // global_work_size is not evenly divisable by size of work-group given by local_work_size
    if( NULL != cpszGlobalWorkSize && NULL != cpszLocalWorkSize )
    {
        for( ui=0; ui<uiWorkDim; ui++)
        {
            if( ( cpszLocalWorkSize[ui] == 0 ) || ( 0 != (cpszGlobalWorkSize[ui] % cpszLocalWorkSize[ui]) ) )
            {
                return CL_INVALID_WORK_GROUP_SIZE;
            }
        }
    }
        
    // CL_INVALID_WORK_GROUP_SIZE if local_work_size is specified and the total number of work-items in the work-group
    // computed as local_work_size[0] * …local_work_size[work_dim – 1] is greater than the value specified by 
    // CL_DEVICE_MAX_WORK_GROUP_SIZE in table 4.3.
    Device* pDevice = NULL;
    cl_err_code err = m_pPlatfromModule->GetDevice(clDeviceId, &pDevice);
    if(CL_FAILED(err))
    {
        return CL_INVALID_CONTEXT;
    }

    if( NULL != cpszLocalWorkSize )
    {
        size_t szDeviceMaxWorkGroupSize = 0;
        size_t szWorkGroupSize = 1;
        pDevice->GetInfo(CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &szDeviceMaxWorkGroupSize, NULL);
        for( ui=0; ui<uiWorkDim; ui++)
        {
            szWorkGroupSize *= cpszLocalWorkSize[ui];
        }
        if( szWorkGroupSize > szDeviceMaxWorkGroupSize )
        {
            return CL_INVALID_WORK_GROUP_SIZE;
        }
        // CL_INVALID_WORK_ITEM_SIZE if the number of work-items specified in any of
        // local_work_size[0], … local_work_size[work_dim – 1] is greater than the corresponding
        // values specified by CL_DEVICE_MAX_WORK_ITEM_SIZES[0], …. CL_DEVICE_MAX_WORK_ITEM_SIZES[work_dim – 1].
        cl_uint uiMaxWorkItemDim = 0;
        pDevice->GetInfo(CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(cl_uint), &uiMaxWorkItemDim, NULL);
        size_t* pszMaxWorkItemSizes = new size_t[uiMaxWorkItemDim];
        pDevice->GetInfo(CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(size_t)*uiMaxWorkItemDim, pszMaxWorkItemSizes, NULL);

        for( ui =0; ui<uiWorkDim; ui++)
        {
            if( cpszLocalWorkSize[ui] > pszMaxWorkItemSizes[ui])
            {
                delete[] pszMaxWorkItemSizes;
                return CL_INVALID_WORK_ITEM_SIZE;
            }
        }
        delete[] pszMaxWorkItemSizes;
    }

    
    // TODO: create buffer resources in advance, if they are not exists,
    //      On error return: CL_OUT_OF_RESOURCES

    Command* pNDRangeKernelCmd = new NDRangeKernelCommand(pKernel, uiWorkDim, cpszGlobalWorkOffset, cpszGlobalWorkSize, cpszLocalWorkSize); 
    // Must set device Id before init for buffer resource allocation.
    pNDRangeKernelCmd->SetCommandDeviceId(pCommandQueue->GetQueueDeviceId());
    errVal = pNDRangeKernelCmd->Init();
    if( CL_SUCCEEDED (errVal) )
    {
        errVal = pCommandQueue->EnqueueCommand(pNDRangeKernelCmd, false/*never blocking*/, uNumEventsInWaitList, cpEeventWaitList, pEvent);
        if(CL_FAILED(errVal))
        {
            // Enqueue failed, free resources
            pNDRangeKernelCmd->CommandDone();
            delete pNDRangeKernelCmd;
        }
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

	// CL_INVALID_PROGRAM_EXECUTABLE if there is no successfully built program
    // executable available for device associated with command_queue.
	cl_device_id clDeviceId = pCommandQueue->GetQueueDeviceId();
	if(!pKernel->IsValidExecutable(clDeviceId))
    {
        return CL_INVALID_PROGRAM_EXECUTABLE;
    }

	// CL_INVALID_KERNEL_ARGS if the kernel argument values have not been specified.
	if(!pKernel->IsValidKernelArgs())
    {
        return CL_INVALID_KERNEL_ARGS;
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
        if(CL_FAILED(errVal))
        {
            // Enqueue failed, free resources
            pTaskCommand->CommandDone();
            delete pTaskCommand;
        }
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
    
	if(NULL == pMemObjectsList)
	{
		return CL_OUT_OF_HOST_MEMORY;
	}
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
	if(NULL == pNativeKernelCommand)
	{
		return CL_OUT_OF_HOST_MEMORY;
	}
    // Must set device Id befor init for buffer resource allocation.
    pNativeKernelCommand->SetCommandDeviceId(pCommandQueue->GetQueueDeviceId());
    errVal = pNativeKernelCommand->Init();
    if( CL_SUCCEEDED (errVal) )
    {
        errVal = pCommandQueue->EnqueueCommand(pNativeKernelCommand, false/*never blocking*/, uNumEventsInWaitList, cpEeventWaitList, pEvent);
        if(CL_FAILED(errVal))
        {
            // Enqueue failed, free resources
            pNativeKernelCommand->CommandDone();
            delete pNativeKernelCommand;
        }
    }    
    return  errVal;
}

/******************************************************************
 *
 ******************************************************************/
inline cl_err_code ExecutionModule::Check2DImageParameters( MemoryObject* pImage, const size_t szOrigin[3], const size_t szRegion[3])
{
    cl_err_code errVal = CL_SUCCESS;
    if( CL_MEM_OBJECT_IMAGE2D == pImage->GetType())
    {
        // 2D image check
        if( 0 != szOrigin[2])
        {
            errVal = CL_INVALID_VALUE;
        }
        if( 1 != szRegion[2] )
        {
            errVal = CL_INVALID_VALUE;
        }
    }
    return errVal;
}

/******************************************************************
 * Returns true if regions in pMemObj overlap
 ******************************************************************/
inline bool ExecutionModule::CheckMemoryObjectOverlapping(MemoryObject* pMemObj, const size_t* szSrcOrigin, const size_t* szDstOrigin, const size_t* szRegion)
{
    bool isOverlaps = false;
    cl_mem_object_type memObjType = pMemObj->GetType();
    switch(memObjType)
    {
    case CL_MEM_OBJECT_IMAGE3D:
        if ( (szDstOrigin[2] < (szSrcOrigin[2] + szRegion[2])) && 
             ((szDstOrigin[2]+szRegion[2]) > szSrcOrigin[2]))
        {
            isOverlaps = true;
        }
    case CL_MEM_OBJECT_IMAGE2D:
        if ( (szDstOrigin[1] < (szSrcOrigin[1] + szRegion[1])) && 
             ((szDstOrigin[1]+szRegion[1]) > szSrcOrigin[1]))
        {
            isOverlaps = true;
        }
    case CL_MEM_OBJECT_BUFFER:
        if ( (szDstOrigin[0] < (szSrcOrigin[0] + szRegion[0])) && 
             ((szDstOrigin[0]+szRegion[0]) > szSrcOrigin[0]))
        {
            isOverlaps = true;
        }
    default:
        break;
    }
    return isOverlaps;
}

/******************************************************************
 *
 ******************************************************************/
inline size_t ExecutionModule::CalcRegionSizeInBytes(MemoryObject* pImage, const size_t* szRegion)
{
    size_t szPixelByteSize = 0;
    size_t szRegionSizeInBytes = 0;
    cl_err_code errVal  = CL_SUCCESS;
    cl_mem_object_type memObjType = pImage->GetType();
    if( memObjType != CL_MEM_OBJECT_BUFFER )
    {
        // Note, we already checked that szRegion[2] == 1, so same calculation is valide for 2D/3D images
        errVal = pImage->GetImageInfo( CL_IMAGE_ELEMENT_SIZE, sizeof(size_t), &szPixelByteSize, NULL);
        if(CL_SUCCEEDED(errVal))
        {
            szRegionSizeInBytes = szRegion[0] * szRegion[1] * szRegion[2] * szPixelByteSize;
        }
    }
    return szRegionSizeInBytes;
}

/******************************************************************
 *
 ******************************************************************/
inline cl_err_code ExecutionModule::CheckImageFormat( MemoryObject* pSrcImage, MemoryObject* pDstImage)
{
    cl_err_code errVal;
    cl_image_format clSrcFormat;
    cl_image_format clDstFormat;

    errVal = pSrcImage->GetImageInfo( CL_IMAGE_FORMAT, sizeof(cl_image_format), &clSrcFormat, NULL);
    if ( CL_SUCCEEDED(errVal) )
    {
        errVal = pDstImage->GetImageInfo( CL_IMAGE_FORMAT, sizeof(cl_image_format), &clDstFormat, NULL);
    }
    if ( CL_SUCCEEDED(errVal))
    {
        // Check formats
        if ( ( clSrcFormat.image_channel_order != clDstFormat.image_channel_order ) ||
             ( clSrcFormat.image_channel_data_type != clDstFormat.image_channel_data_type )
             )
        {
            errVal = CL_IMAGE_FORMAT_MISMATCH;
        }        
    }
    return errVal;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code ExecutionModule::EnqueueReadImage(
                                cl_command_queue clCommandQueue, 
                                cl_mem           clImage,
                                cl_bool          bBlocking, 
                                const size_t     szOrigin[3],
                                const size_t     szRegion[3],
                                size_t           szRowPitch,
                                size_t           szSlicePitch,
                                void*            pOutData, 
                                cl_uint          uNumEventsInWaitList, 
                                const cl_event*  cpEeventWaitList, 
                                cl_event*        pEvent      
                                )
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
    
    MemoryObject* pImage = m_pContextModule->GetMemoryObject(clImage);
    if (NULL == pImage)
    {
        return CL_INVALID_MEM_OBJECT;
    }

    if (pImage->GetContextId() != pCommandQueue->GetContextId())
    {
        return CL_INVALID_CONTEXT;
    }

    if (!(pImage->CheckBounds(szOrigin, szRegion))                     ||
        CL_FAILED(Check2DImageParameters(pImage, szOrigin, szRegion))  ||
        ((CL_MEM_OBJECT_IMAGE2D == pImage->GetType()) && (0 != szSlicePitch))
        )
    {
        return CL_INVALID_VALUE;
    }

    Command* pReadImageCmd  = new ReadImageCommand(pImage, szOrigin, szRegion, szRowPitch, szSlicePitch, pOutData);
    errVal = pReadImageCmd->Init();
    if(CL_SUCCEEDED(errVal))
    {
        errVal = pCommandQueue->EnqueueCommand(pReadImageCmd, bBlocking, uNumEventsInWaitList, cpEeventWaitList, pEvent);
        if(CL_FAILED(errVal))
        {
            // Enqueue failed, free resources
            pReadImageCmd->CommandDone();
            delete pReadImageCmd;
        }
    }    
    return  errVal;
}


/******************************************************************
 *
 ******************************************************************/
cl_err_code ExecutionModule::EnqueueWriteImage(
                                cl_command_queue clCommandQueue, 
                                cl_mem           clImage,
                                cl_bool          bBlocking, 
                                const size_t     szOrigin[3],
                                const size_t     szRegion[3],
                                size_t           szRowPitch,
                                size_t           szSlicePitch,
                                const void *     cpSrcData,
                                cl_uint          uNumEventsInWaitList, 
                                const cl_event*  cpEeventWaitList, 
                                cl_event*        pEvent
                                )
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
    
    MemoryObject* pImage = m_pContextModule->GetMemoryObject(clImage);
    if (NULL == pImage)
    {
        return CL_INVALID_MEM_OBJECT;
    }

    if (pImage->GetContextId() != pCommandQueue->GetContextId())
    {
        return CL_INVALID_CONTEXT;
    }

    if (!(pImage->CheckBounds(szOrigin, szRegion))                     ||
        CL_FAILED(Check2DImageParameters(pImage, szOrigin, szRegion))  ||
        ((CL_MEM_OBJECT_IMAGE2D == pImage->GetType()) && (0 != szSlicePitch))
        )
    {
        return CL_INVALID_VALUE;
    }

    Command* pWriteImageCmd  = new WriteImageCommand(pImage, szOrigin, szRegion, szRowPitch, szSlicePitch, cpSrcData);
    errVal = pWriteImageCmd->Init();
    if(CL_SUCCEEDED(errVal))
    {
        errVal = pCommandQueue->EnqueueCommand(pWriteImageCmd, bBlocking, uNumEventsInWaitList, cpEeventWaitList, pEvent);
        if(CL_FAILED(errVal))
        {
            // Enqueue failed, free resources
            pWriteImageCmd->CommandDone();
            delete pWriteImageCmd;
        }
    }    
    return  errVal;
}


/******************************************************************
 *
 ******************************************************************/
cl_err_code ExecutionModule::EnqueueCopyImage(
                                cl_command_queue clCommandQueue,
                                cl_mem           clSrcImage,
                                cl_mem           clDstImage,
                                const size_t     szSrcOrigin[3],
                                const size_t     szDstOrigin[3],
                                const size_t     szRegion[3],
                                cl_uint          uNumEventsInWaitList, 
                                const cl_event*  cpEeventWaitList, 
                                cl_event*        pEvent      
                                )
{
    cl_err_code errVal = CL_SUCCESS;
    OclCommandQueue* pCommandQueue = GetCommandQueue(clCommandQueue);
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }
    
    MemoryObject* pSrcImage = m_pContextModule->GetMemoryObject(clSrcImage);
    MemoryObject* pDstImage = m_pContextModule->GetMemoryObject(clDstImage);
    if (NULL == pSrcImage || NULL == pDstImage)
    {
        return CL_INVALID_MEM_OBJECT;
    }

    if (pSrcImage->GetContextId() != pCommandQueue->GetContextId()  ||
        pSrcImage->GetContextId() != pDstImage->GetContextId() 
        )
    {
        return CL_INVALID_CONTEXT;
    }
    
    // Check format
    errVal = CheckImageFormat(pSrcImage, pDstImage);
    if(CL_FAILED(errVal))
    {
        return CL_IMAGE_FORMAT_MISMATCH;
    }

    // Check boundaries.
    if (!(pSrcImage->CheckBounds(szSrcOrigin,szRegion)) || !(pDstImage->CheckBounds(szDstOrigin,szRegion)))
    {
        return CL_INVALID_VALUE;
    }

    if( CL_FAILED(Check2DImageParameters(pSrcImage, szSrcOrigin, szRegion))  ||
        CL_FAILED(Check2DImageParameters(pDstImage, szDstOrigin, szRegion))  
        )
    {
        return CL_INVALID_VALUE;
    }

    // Check overlapping
    if( clSrcImage == clDstImage)
    {
        if (CheckMemoryObjectOverlapping(pSrcImage, szSrcOrigin, szDstOrigin, szRegion))
        {
            return CL_MEM_COPY_OVERLAP;
        }
    }

    //
    // Input parameters validated, enqueue the command
    //
    Command* pCopyImageCmd = new CopyImageCommand(pSrcImage, pDstImage, szSrcOrigin, szDstOrigin, szRegion);
    errVal = pCopyImageCmd->Init();
    if(CL_SUCCEEDED(errVal))
    {
        // Enqueue copy command, never blocking
        errVal = pCommandQueue->EnqueueCommand(pCopyImageCmd, CL_FALSE, uNumEventsInWaitList, cpEeventWaitList, pEvent);
        if(CL_FAILED(errVal))
        {
            // Enqueue failed, free resources
            pCopyImageCmd->CommandDone();
            delete pCopyImageCmd;
        }
    }    
    return  errVal;
}


/******************************************************************
 *
 ******************************************************************/
cl_err_code ExecutionModule::EnqueueCopyImageToBuffer(
                                cl_command_queue clCommandQueue,
                                cl_mem           clSrcImage,
                                cl_mem           clDstBuffer,
                                const size_t     szSrcOrigin[3],
                                const size_t     szRegion[3],
                                size_t           szDstOffset,
                                cl_uint          uNumEventsInWaitList, 
                                const cl_event*  cpEeventWaitList, 
                                cl_event*        pEvent
                                )
{
    cl_err_code errVal = CL_SUCCESS;
    OclCommandQueue* pCommandQueue = GetCommandQueue(clCommandQueue);
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }
    
    MemoryObject* pSrcImage = m_pContextModule->GetMemoryObject(clSrcImage);
    MemoryObject* pDstBuffer = m_pContextModule->GetMemoryObject(clDstBuffer);
    if (NULL == pSrcImage || NULL == pDstBuffer)
    {
        return CL_INVALID_MEM_OBJECT;
    }

    if (pSrcImage->GetContextId() != pCommandQueue->GetContextId()  ||
        pSrcImage->GetContextId() != pDstBuffer->GetContextId() 
        )
    {
        return CL_INVALID_CONTEXT;
    }    

    if( CL_FAILED(Check2DImageParameters(pSrcImage, szSrcOrigin, szRegion)) )
    {
        return CL_INVALID_VALUE;
    }

    // Calculate dst_cb
    size_t szDstCb = CalcRegionSizeInBytes(pSrcImage, szRegion);

    // Check boundaries.
    if (!(pSrcImage->CheckBounds(szSrcOrigin,szRegion)) || !(pDstBuffer->CheckBounds(&szDstOffset,&szDstCb)))
    {
        return CL_INVALID_VALUE;
    }

    //
    // Input parameters validated, enqueue the command
    //
    Command* pCopyImageToBufferCmd = new CopyImageToBufferCommand(pSrcImage, pDstBuffer, szSrcOrigin, szRegion, szDstOffset);
    errVal = pCopyImageToBufferCmd->Init();
    if(CL_SUCCEEDED(errVal))
    {
        // Enqueue copy command, never blocking
        errVal = pCommandQueue->EnqueueCommand(pCopyImageToBufferCmd, CL_FALSE, uNumEventsInWaitList, cpEeventWaitList, pEvent);
        if(CL_FAILED(errVal))
        {
            // Enqueue failed, free resources
            pCopyImageToBufferCmd->CommandDone();
            delete pCopyImageToBufferCmd;
        }
    }    
    return  errVal;
}


/******************************************************************
 *
 ******************************************************************/
cl_err_code ExecutionModule::EnqueueCopyBufferToImage(
                                cl_command_queue clCommandQueue,
                                cl_mem           clSrcBuffer,
                                cl_mem           clDstImage,
                                size_t           szSrcOffset,
                                const size_t     szDstOrigin[3],
                                const size_t     szRegion[3],
                                cl_uint          uNumEventsInWaitList,
                                const cl_event*  cpEeventWaitList, 
                                cl_event*        pEvent
                                )
{
    cl_err_code errVal = CL_SUCCESS;
    OclCommandQueue* pCommandQueue = GetCommandQueue(clCommandQueue);
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }
    
    MemoryObject* pSrcBuffer = m_pContextModule->GetMemoryObject(clSrcBuffer);
    MemoryObject* pDstImage = m_pContextModule->GetMemoryObject(clDstImage);
    if (NULL == pSrcBuffer || NULL == pDstImage)
    {
        return CL_INVALID_MEM_OBJECT;
    }

    if (pSrcBuffer->GetContextId() != pCommandQueue->GetContextId()  ||
        pSrcBuffer->GetContextId() != pDstImage->GetContextId() 
        )
    {
        return CL_INVALID_CONTEXT;
    }    

    if( CL_FAILED(Check2DImageParameters(pDstImage, szDstOrigin, szRegion)) )
    {
        return CL_INVALID_VALUE;
    }

    // Calculate dst_cb
    size_t szDstCb = CalcRegionSizeInBytes(pDstImage, szRegion);

    // Check boundaries.
    if (!(pSrcBuffer->CheckBounds(&szSrcOffset,&szDstCb)) || !(pDstImage->CheckBounds(szDstOrigin,szRegion)))
    {
        return CL_INVALID_VALUE;
    }

    //
    // Input parameters validated, enqueue the command
    //
    Command* pCopyBufferToImageCmd = new CopyBufferToImageCommand(pSrcBuffer, pDstImage, szSrcOffset, szDstOrigin, szRegion);
    errVal = pCopyBufferToImageCmd->Init();
    if(CL_SUCCEEDED(errVal))
    {
        // Enqueue copy command, never blocking
        errVal = pCommandQueue->EnqueueCommand(pCopyBufferToImageCmd, CL_FALSE, uNumEventsInWaitList, cpEeventWaitList, pEvent);
        if(CL_FAILED(errVal))
        {
            // Enqueue failed, free resources
            pCopyBufferToImageCmd->CommandDone();
            delete pCopyBufferToImageCmd;
        }
    }    
    return  errVal;
}

/******************************************************************
 *
 ******************************************************************/
void * ExecutionModule::EnqueueMapImage(
    cl_command_queue    clCommandQueue, 
    cl_mem              clImage, 
    cl_bool             bBlockingMap, 
    cl_map_flags        clMapFlags, 
    const size_t        szOrigin[3], 
    const size_t        szRegion[3], 
    size_t*             pszImageRowPitch, 
    size_t*             pszImageSlicePitch, 
    cl_uint             uNumEventsInWaitList, 
    const cl_event*     cpEeventWaitList, 
    cl_event*           pEvent, 
    cl_int*             pErrcodeRet)
{
	cl_int err = CL_SUCCESS;
	if (NULL == pErrcodeRet)
	{
		pErrcodeRet = &err;
	}
	else
	{
		*pErrcodeRet = CL_SUCCESS;
	}

    OclCommandQueue* pCommandQueue = GetCommandQueue(clCommandQueue);
    MemoryObject* pImage = m_pContextModule->GetMemoryObject(clImage);    

    if (NULL == pCommandQueue)
    {
        *pErrcodeRet = CL_INVALID_COMMAND_QUEUE;
    }
    else if (NULL == pImage)
    {
        *pErrcodeRet =  CL_INVALID_MEM_OBJECT;
    }
    else if (  (CL_MAP_READ | CL_MAP_WRITE | clMapFlags) != (CL_MAP_READ | CL_MAP_WRITE) )
    {
        // Check that flags CL_MAP_READ or CL_MAP_WRITE only
        *pErrcodeRet = CL_INVALID_VALUE;
    }
    else if (pImage->GetContextId() != pCommandQueue->GetContextId())
    {
        *pErrcodeRet = CL_INVALID_CONTEXT;
        return NULL;
    }    
    else if (!(pImage->CheckBounds(szOrigin, szRegion))                 ||
        CL_FAILED(Check2DImageParameters(pImage, szOrigin, szRegion))   ||
        (NULL == pszImageRowPitch)                                      ||
        ((CL_MEM_OBJECT_IMAGE3D == pImage->GetType()) && (NULL == pszImageSlicePitch))  
        )
    {
        *pErrcodeRet = CL_INVALID_VALUE;
    }

    if(CL_FAILED(*pErrcodeRet))
    {
        return NULL;
    }
        
    MapImageCommand* pMapImageCmd = new MapImageCommand( pImage, clMapFlags, szOrigin, szRegion, pszImageRowPitch, pszImageSlicePitch);
    // Must set device Id before init for image resource allocation.
    pMapImageCmd->SetCommandDeviceId(pCommandQueue->GetQueueDeviceId());
    *pErrcodeRet = pMapImageCmd->Init();
    // Get pointer for mapped region since it is allocated on init. Execute will lock the region
    // Note that if EnqueueCommand successed, by the time it returns, the command may be deleted already.
    void* mappedRegion = pMapImageCmd->GetMappedRegion();
    if(CL_SUCCEEDED(*pErrcodeRet))
    {
        *pErrcodeRet = pCommandQueue->EnqueueCommand(pMapImageCmd, bBlockingMap, uNumEventsInWaitList, cpEeventWaitList, pEvent);
        if(CL_FAILED(*pErrcodeRet))
        {
            // Enqueue failed, free resources
            pMapImageCmd->CommandDone();
            delete pMapImageCmd;
            return NULL;
        }
        return mappedRegion;
    }  
    else
    {
        return  NULL;
    }
}


/******************************************************************
 * 
 ******************************************************************/
cl_err_code ExecutionModule::GetEventProfilingInfo (cl_event clEvent, 
													cl_profiling_info clParamName, 
													size_t szParamValueSize, 
													void * pParamValue, 
													size_t * pszParamValueSizeRet)
{
    cl_err_code res = m_pEventsManager->GetEventProfilingInfo(clEvent, clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);
    return res;
}

/******************************************************************
 * 
 ******************************************************************/
cl_err_code ExecutionModule::EnqueueAcquireGLObjects(cl_command_queue clCommandQueue, 
													 cl_uint uiNumObjects, 
													 const cl_mem * pclMemObjects, 
													 cl_uint uiNumEventsInWaitList, 
													 const cl_event * pclEventWaitList, 
													 cl_event * pclEvent)
{
	return CL_ERR_NOT_IMPLEMENTED;
}

/******************************************************************
 * 
 ******************************************************************/
cl_err_code ExecutionModule::EnqueueReleaseGLObjects(cl_command_queue clCommandQueue, 
													 cl_uint uiNumObjects, 
													 const cl_mem * pclMemObjects, 
													 cl_uint uiNumEventsInWaitList, 
													 const cl_event * pclEventWaitList, 
													 cl_event * pclEvent)
{
	return CL_ERR_NOT_IMPLEMENTED;
}