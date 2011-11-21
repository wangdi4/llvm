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
#include "command_queue.h"
#include "in_order_command_queue.h"
#include "out_of_order_command_queue.h"
#include "immediate_command_queue.h"
#include "user_event.h"
#include "Context.h"
#include "enqueue_commands.h"
#include "MemoryAllocator/MemoryObject.h"

#if defined (_WIN32)
#include "gl_mem_objects.h"
#include "gl_commands.h"
#endif
#include "kernel.h"
#include "Device.h"
#include "cl_sys_defines.h"

#include <CL/cl_ext.h>
#if defined (DX9_MEDIA_SHARING)
#include "d3d9_sharing/d3d9_context.h"
#include "d3d9_sharing/d3d9_resource.h"
#include "d3d9_sharing/d3d9_sync_d3d9_resources.h"
#endif
#include <cassert>
#include <cl_objects_map.h>
#include <Logger.h>
#include <cl_local_array.h>

using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;


#define SetIfZero(X,VALUE) {if ((X)==0) (X)=(VALUE);}
#define CheckIfAnyDimIsZero(X) (((X)[0] == 0) || ((X)[1] == 0) || ((X)[2] == 0))

/******************************************************************
 * Constructor. Only assign pointers, for objects initilaztion use
 * Initialize function immediately. otherwise, the class behaviour
 * is undefined and function calls may crash the system.
 ******************************************************************/
ExecutionModule::ExecutionModule( PlatformModule *pPlatformModule, ContextModule* pContextModule ):
    m_bUseTaskalyzer(false),
    m_pPlatfromModule(pPlatformModule),
    m_pContextModule(pContextModule),
    m_pOclCommandQueueMap(NULL),
    m_pEventsManager(NULL)
{
	INIT_LOGGER_CLIENT(L"ExecutionModel",LL_DEBUG);

    LOG_DEBUG(TEXT("%S"), TEXT("ExecutionModule created"));
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
cl_err_code ExecutionModule::Initialize(ocl_entry_points * pOclEntryPoints, OCLConfig * pOclConfig, ocl_gpa_data * pGPAData)
{
    m_pOclCommandQueueMap = new OCLObjectsMap<_cl_command_queue_int>();
    m_pEventsManager = new EventsManager();

	m_pOclEntryPoints = pOclEntryPoints;

	// initialize GPA data
	m_pGPAData = pGPAData;
    
    if ( (NULL == m_pOclCommandQueueMap) || ( NULL == m_pEventsManager))
    {
        return CL_ERR_FAILURE;
    }
	m_bUseTaskalyzer = pOclConfig->UseGPA();
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
    cl_command_queue iQueueID   = CL_INVALID_HANDLE;
    Context*    pContext   = NULL;
    cl_int      errVal     = CheckCreateCommandQueueParams(clContext, clDevice, clQueueProperties, &pContext);

    // If we are here, all parameters are valid, create the queue
    if( CL_SUCCEEDED(errVal))
    {
		IOclCommandQueueBase* pCommandQueue;
        if (clQueueProperties & CL_QUEUE_THREAD_LOCAL_EXEC_ENABLE_INTEL)
        {
            pCommandQueue = new ImmediateCommandQueue(pContext, clDevice, clQueueProperties, m_pEventsManager, m_pOclEntryPoints);
        }
        else
        {
		    if (clQueueProperties & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE)
		    {
			    pCommandQueue = new OutOfOrderCommandQueue(pContext, clDevice, clQueueProperties, m_pEventsManager, m_pOclEntryPoints);
		    }
		    else
		    {
			    pCommandQueue = new InOrderCommandQueue(pContext, clDevice, clQueueProperties, m_pEventsManager, m_pOclEntryPoints);
		    }
        }

		if ( NULL != pCommandQueue )
		{
			errVal = pCommandQueue->Initialize();
			if(CL_SUCCEEDED(errVal))
			{
				// TODO: guard ObjMap... better doing so inside the map        
				m_pOclCommandQueueMap->AddObject((OCLObject<_cl_command_queue_int>*)pCommandQueue);
				iQueueID = pCommandQueue->GetHandle();
                
                // this is the first place where we are sure that the commmand queue was created
                errVal = pCommandQueue->GPA_InitializeQueue();
			}
			else
			{
				pCommandQueue->Release();
			}
		}
		else
		{
			errVal = CL_OUT_OF_HOST_MEMORY;
		}
    }
    if (pErrRet) *pErrRet = errVal;

    return iQueueID;
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
                                                   CL_QUEUE_PROFILING_ENABLE              |
                                                   CL_QUEUE_THREAD_LOCAL_EXEC_ENABLE_INTEL ) ) 
    {
        errVal = CL_INVALID_VALUE;
    }
    return errVal;
}

/******************************************************************
 * This function returns a pointer to a command queue.
 * If the command queue is not available a NULL value is returned.
 ******************************************************************/
IOclCommandQueueBase* ExecutionModule::GetCommandQueue(cl_command_queue clCommandQueue)
{
    cl_err_code errCode;            
    OCLObject<_cl_command_queue_int>*  pOclObject = NULL;
    errCode = m_pOclCommandQueueMap->GetOCLObject((_cl_command_queue_int*)clCommandQueue, &pOclObject);
    if (CL_FAILED(errCode))
    {
        return NULL;
    }
    IOclCommandQueueBase* pCommandQueue = dynamic_cast<IOclCommandQueueBase*>(pOclObject);
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
	cl_err_code errCode = Flush(clCommandQueue);
	if ( CL_FAILED(errCode) )
	{
		return errCode;
	}
	errCode = m_pOclCommandQueueMap->ReleaseObject((_cl_command_queue_int*)clCommandQueue);
	if (0 != errCode)
	{
		return CL_INVALID_COMMAND_QUEUE;
	}
	return CL_SUCCESS;
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
    IOclCommandQueueBase* pOclCommandQueue = GetCommandQueue(clCommandQueue);
    if (NULL == pOclCommandQueue)
    {
        res = CL_INVALID_COMMAND_QUEUE;
    }
    else
    {
        res = pOclCommandQueue->Flush(true);
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
	cl_event dummy = NULL;
	IOclCommandQueueBase* pCommandQueue = GetCommandQueue(clCommandQueue);
	if (NULL == pCommandQueue)
	{
		return CL_INVALID_COMMAND_QUEUE;
	}

	res = EnqueueMarker(clCommandQueue, &dummy);
	if (CL_FAILED(res))
	{
		return res;
	}

	OclEvent* pDummyEvent = m_pEventsManager->GetEvent(dummy);
	assert(pDummyEvent);
	bool bres = pCommandQueue->WaitForCompletion(pDummyEvent);
	m_pEventsManager->ReleaseEvent(dummy);

	return bres ? CL_SUCCESS : CL_OUT_OF_RESOURCES;
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

	IOclCommandQueueBase* pCommandQueue = GetCommandQueue(clCommandQueue);
	if (NULL == pCommandQueue)
	{
		return CL_INVALID_COMMAND_QUEUE;
	}

	// Create Command
	Command* pMarkerCommand = new MarkerCommand(pCommandQueue, (ocl_entry_points*)((_cl_command_queue_int *)pCommandQueue->GetHandle())->dispatch);
	if (NULL == pMarkerCommand)
	{
		return CL_OUT_OF_HOST_MEMORY;
	}

	errVal = pMarkerCommand->Init();
	if ( CL_FAILED(errVal) )
	{
		delete pMarkerCommand;
		return errVal;
	}

	QueueEvent* pMarkerEvent = pMarkerCommand->GetEvent();
	m_pEventsManager->RegisterQueueEvent(pMarkerEvent, pEvent);

	errVal = pCommandQueue->EnqueueMarker(pMarkerCommand);
	if(CL_FAILED(errVal))
	{
		m_pEventsManager->ReleaseEvent(pMarkerEvent->GetHandle());
		pMarkerCommand->CommandDone();
		delete pMarkerCommand;
		return errVal;
	}

	return errVal;
}

/******************************************************************
 * 
 ******************************************************************/
cl_err_code ExecutionModule::EnqueueWaitForEvents(cl_command_queue clCommandQueue, cl_uint uiNumEvents, const cl_event* cpEventList)
{
    cl_err_code errVal;
	if ( (NULL == cpEventList) || (0 == uiNumEvents) )
	{
		return CL_INVALID_VALUE;
	}

	IOclCommandQueueBase* pCommandQueue = GetCommandQueue(clCommandQueue);
	// Create Command
	if (NULL == pCommandQueue)
	{
		return CL_INVALID_COMMAND_QUEUE;
	}
	
	Command* pWaitForEventsCommand = new WaitForEventsCommand(pCommandQueue, (ocl_entry_points*)((_cl_command_queue_int *)pCommandQueue->GetHandle())->dispatch);
	if (NULL == pWaitForEventsCommand)
	{
		return CL_OUT_OF_HOST_MEMORY;
	}

	errVal = pWaitForEventsCommand->Init();
	if ( CL_FAILED(errVal) )
	{
		delete pWaitForEventsCommand;
		return errVal;
	}

	errVal = pCommandQueue->EnqueueWaitEvents(pWaitForEventsCommand, uiNumEvents, cpEventList);
	if ( CL_FAILED(errVal) )
	{
		pWaitForEventsCommand->CommandDone();
		delete pWaitForEventsCommand;
		return errVal;
	}

	return errVal;
}

/******************************************************************
 * 
 ******************************************************************/
cl_err_code ExecutionModule::EnqueueBarrier(cl_command_queue clCommandQueue)
{
	cl_err_code errVal;
	IOclCommandQueueBase* pCommandQueue = GetCommandQueue(clCommandQueue);
	if (NULL == pCommandQueue)
	{
		return CL_INVALID_COMMAND_QUEUE;
	}

	// Create Command
	Command* pBarrierCommand = new BarrierCommand(pCommandQueue, (ocl_entry_points*)((_cl_command_queue_int *)pCommandQueue->GetHandle())->dispatch);
	if (NULL == pBarrierCommand)
	{
		return CL_OUT_OF_HOST_MEMORY;
	}

	errVal = pBarrierCommand->Init();
	if(CL_FAILED(errVal))
	{
		delete pBarrierCommand;
		return errVal;
	}

	QueueEvent* pBarrierEvent = pBarrierCommand->GetEvent();
	m_pEventsManager->RegisterQueueEvent(pBarrierEvent, NULL);

	errVal = pCommandQueue->EnqueueBarrier(pBarrierCommand);
	if(CL_FAILED(errVal))
	{
		m_pEventsManager->ReleaseEvent(pBarrierEvent->GetHandle());
		pBarrierCommand->CommandDone();
		delete pBarrierCommand;
		return errVal;
	}

	return errVal;
}


/******************************************************************
 * 
 ******************************************************************/
cl_err_code ExecutionModule::WaitForEvents( cl_uint uiNumEvents, const cl_event* cpEventList )
{
	cl_start;
    cl_err_code errVal = CL_SUCCESS;
    if ( 0 == uiNumEvents || NULL == cpEventList)
        return CL_INVALID_VALUE;

    // Validate event context
    cl_context clEventsContext = 0;
	OclEvent* pEvent = m_pEventsManager->GetEvent(cpEventList[0]);
	if ( NULL == pEvent )
	{
		return CL_INVALID_EVENT_WAIT_LIST;
	}

	clEventsContext = pEvent->GetContextHandle();

    // Before waiting all on events, the function need to flush all relevant queues, 
    // Since the dependencies between events in different queues is unknown it is better
    // to flush all queues in the context.
	FlushAllQueuesForContext(clEventsContext);

    // This call is blocking.    
    errVal = m_pEventsManager->WaitForEvents(uiNumEvents, cpEventList);
    if ( CL_INVALID_EVENT_WAIT_LIST == errVal )
    {
        return CL_INVALID_EVENT;
    }
    cl_return errVal;
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
cl_event ExecutionModule::CreateUserEvent(cl_context context, cl_int * errcode_ret)
{
	cl_int   err = CL_SUCCESS;
	cl_event evt = (cl_event)0;
	//Validate the context is legit
	Context* pContext = m_pContextModule->GetContext(context);
	if (NULL == pContext)
	{
		err = CL_INVALID_CONTEXT;
	}
	else
	{
		UserEvent* pUserEvent  = m_pEventsManager->CreateUserEvent(context);
		if (pUserEvent)
		{
			evt                = pUserEvent->GetHandle();
		}
		else
		{
			err = CL_OUT_OF_HOST_MEMORY;
		}
	}

	if (NULL != errcode_ret)
	{
		*errcode_ret = err;
	}
	return evt;

}

/******************************************************************
* 
******************************************************************/
cl_int ExecutionModule::SetUserEventStatus(cl_event evt, cl_int status)
{
	UserEvent* pUserEvent = m_pEventsManager->GetUserEvent(evt);
	if (NULL == pUserEvent)
	{
		return CL_INVALID_EVENT;
	}

	if ((status != CL_COMPLETE) && (status > 0))
	{
		return CL_INVALID_VALUE;
	}

	if (pUserEvent->GetEventCurrentStatus() != CL_SUBMITTED)
	{
		return CL_INVALID_OPERATION;
	}

	pUserEvent->SetComplete(status);
	return CL_SUCCESS;
}
/******************************************************************
* 
******************************************************************/
cl_err_code ExecutionModule::SetEventCallback(cl_event evt, cl_int status, void (CL_CALLBACK *fn)(cl_event, cl_int, void *), void *userData)
{
	return m_pEventsManager->SetEventCallBack(evt, status, fn, userData);
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

    IOclCommandQueueBase* pCommandQueue = GetCommandQueue(clCommandQueue);
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }
    
    MemoryObject* pBuffer = m_pContextModule->GetMemoryObject(clBuffer);
    if (NULL == pBuffer)
    {
        return CL_INVALID_MEM_OBJECT;
    }

    if (pBuffer->GetContext()->GetId() != pCommandQueue->GetContextId())
    {
        return CL_INVALID_CONTEXT;
    }

    if (CL_SUCCESS != (errVal = pBuffer->CheckBounds(&szOffset, &szCb)))
    {
        // Out of bounds check.
        return errVal;
    }
	
	if ((NULL == cpEeventWaitList && (0 < uNumEventsInWaitList)) || (cpEeventWaitList && (0 == uNumEventsInWaitList)))
	{
		return CL_INVALID_EVENT_WAIT_LIST;
	}

	const size_t pszOrigin[3] = {szOffset, 0 , 0};
	const size_t pszRegion[3] = {szCb, 1, 1};

	Command* pEnqueueReadBufferCmd = new ReadBufferCommand(pCommandQueue, m_pOclEntryPoints, pBuffer, pszOrigin, pszRegion, pOutData);
	if (NULL == pEnqueueReadBufferCmd)
	{
		return CL_OUT_OF_HOST_MEMORY;
	}

    errVal = pEnqueueReadBufferCmd->Init();
	if ( CL_FAILED(errVal) )
	{
		delete pEnqueueReadBufferCmd;
	    return  errVal;
	}

    errVal = pCommandQueue->EnqueueCommand(pEnqueueReadBufferCmd, bBlocking, uNumEventsInWaitList, cpEeventWaitList, pEvent);
    if(CL_FAILED(errVal))
    {
        // Enqueue failed, free resources
        pEnqueueReadBufferCmd->CommandDone();
        delete pEnqueueReadBufferCmd;
    }
    return  errVal;
}

/******************************************************************
 * 
 ******************************************************************/
cl_err_code ExecutionModule::EnqueueReadBufferRect(
						cl_command_queue	clCommandQueue, 
						cl_mem				clBuffer, 
						cl_bool				bBlocking, 
						const size_t		szBufferOrigin[MAX_WORK_DIM], 
						const size_t		szHostOrigin[MAX_WORK_DIM], 
						const size_t		region[MAX_WORK_DIM], 
						size_t				buffer_row_pitch, 
						size_t				buffer_slice_pitch, 
						size_t				host_row_pitch, 
						size_t				host_slice_pitch, 
						void*				pOutData, 
						cl_uint				uNumEventsInWaitList, 
						const cl_event*		cpEeventWaitList, 
						cl_event*			pEvent)
{
	cl_err_code errVal = CL_SUCCESS;
	
    if (NULL == pOutData)
    {
        return CL_INVALID_VALUE;
    }

    IOclCommandQueueBase* pCommandQueue = GetCommandQueue(clCommandQueue);
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }
    
    MemoryObject* pBuffer = m_pContextModule->GetMemoryObject(clBuffer);
    if (NULL == pBuffer)
    {
        return CL_INVALID_MEM_OBJECT;
    }
	
    if (pBuffer->GetContext()->GetId() != pCommandQueue->GetContextId())
    {
        return CL_INVALID_CONTEXT;
    }

	if (CheckIfAnyDimIsZero(region)														||
		(buffer_row_pitch	!=0 && buffer_row_pitch		<region[0])						|| 
		(host_row_pitch		!=0 && host_row_pitch		<region[0])						|| 
		(buffer_slice_pitch	!=0 && buffer_slice_pitch	<(region[1]*buffer_row_pitch))	|| 
		(host_slice_pitch	!=0 && host_slice_pitch		<(region[1]*host_row_pitch))  
		)
	{
		return CL_INVALID_VALUE;
	}

	SetIfZero(buffer_row_pitch		, region[0]);
	SetIfZero(host_row_pitch		, region[0]);
	SetIfZero(buffer_slice_pitch	, region[1] * buffer_row_pitch);
	SetIfZero(host_slice_pitch		, region[1] * host_row_pitch);		

    if (CL_SUCCESS != (errVal = pBuffer->CheckBoundsRect(szBufferOrigin, region, buffer_row_pitch, buffer_slice_pitch)))
    {
        // Out of bounds check.
        return errVal;
    }
	
	// Is Sub-buffer
	if ( NULL != pBuffer->GetParent() )
	{		
		if (!pBuffer->IsSupportedByDevice(pCommandQueue->GetDefaultDevice()))
		{
			return CL_MISALIGNED_SUB_BUFFER_OFFSET;
		}
	}
	       

	Command* pEnqueueReadBufferRectCmd = new ReadBufferRectCommand(pCommandQueue, m_pOclEntryPoints, pBuffer, szBufferOrigin, szHostOrigin, region, buffer_row_pitch, 
		buffer_slice_pitch, host_row_pitch, host_slice_pitch, pOutData);	
	if(NULL == pEnqueueReadBufferRectCmd)
	{
		 return CL_OUT_OF_HOST_MEMORY;
	}

    errVal = pEnqueueReadBufferRectCmd->Init();
	if ( CL_FAILED(errVal) )
	{
		delete pEnqueueReadBufferRectCmd;
	    return  errVal;
	}

    errVal = pCommandQueue->EnqueueCommand(pEnqueueReadBufferRectCmd, bBlocking, uNumEventsInWaitList, cpEeventWaitList, pEvent);
    if(CL_FAILED(errVal))
    {
        // Enqueue failed, free resources
        pEnqueueReadBufferRectCmd->CommandDone();
        delete pEnqueueReadBufferRectCmd;
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

    IOclCommandQueueBase* pCommandQueue = GetCommandQueue(clCommandQueue);
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }
    
    MemoryObject* pBuffer = m_pContextModule->GetMemoryObject(clBuffer);
    if (NULL == pBuffer)
    {
        return CL_INVALID_MEM_OBJECT;
    }

    if (pBuffer->GetContext()->GetId() != pCommandQueue->GetContextId())
    {
        return CL_INVALID_CONTEXT;
    }

    if (CL_SUCCESS != (errVal = pBuffer->CheckBounds(&szOffset, &szCb)))
    {
        // Out of bounds check.
        return errVal;
    }

	if ((NULL == cpEeventWaitList && (0 < uNumEventsInWaitList)) || (cpEeventWaitList && (0 == uNumEventsInWaitList)))
	{
		return CL_INVALID_EVENT_WAIT_LIST;
	}

	const size_t pszOrigin[3] = {szOffset, 0 , 0};
	const size_t pszRegion[3] = {szCb, 1, 1};

	Command* pWriteBufferCmd = new WriteBufferCommand(pCommandQueue, m_pOclEntryPoints, bBlocking, pBuffer, pszOrigin, pszRegion, cpSrcData);
	if (NULL == pWriteBufferCmd)
	{
		return CL_OUT_OF_HOST_MEMORY;
	}

    errVal = pWriteBufferCmd->Init();
	if ( CL_FAILED(errVal) )
	{
		delete pWriteBufferCmd;
	    return  errVal;
	}

    errVal = pCommandQueue->EnqueueCommand(pWriteBufferCmd, CL_FALSE, uNumEventsInWaitList, cpEeventWaitList, pEvent);
    if(CL_FAILED(errVal))
    {
		pWriteBufferCmd->CommandDone();
		delete pWriteBufferCmd;
    }

    return  errVal;
}

/******************************************************************
 * 
 ******************************************************************/
cl_err_code ExecutionModule::EnqueueWriteBufferRect(
						cl_command_queue	clCommandQueue, 
						cl_mem				clBuffer, 
						cl_bool				bBlocking, 
						const size_t		szBufferOrigin[MAX_WORK_DIM], 
						const size_t		szHostOrigin[MAX_WORK_DIM], 
						const size_t		region[MAX_WORK_DIM], 
						size_t				buffer_row_pitch, 
						size_t				buffer_slice_pitch, 
						size_t				host_row_pitch, 
						size_t				host_slice_pitch, 
						const void*			pOutData, 
						cl_uint				uNumEventsInWaitList, 
						const cl_event*		cpEeventWaitList, 
						cl_event*			pEvent)
{
	cl_start;
    cl_err_code errVal = CL_SUCCESS;

    if (NULL == pOutData)
    {
        return CL_INVALID_VALUE;
    }

    IOclCommandQueueBase* pCommandQueue = GetCommandQueue(clCommandQueue);
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }
    
    MemoryObject* pBuffer = m_pContextModule->GetMemoryObject(clBuffer);
    if (NULL == pBuffer)
    {
        return CL_INVALID_MEM_OBJECT;
    }

    if (pBuffer->GetContext()->GetId() != pCommandQueue->GetContextId())
    {
        return CL_INVALID_CONTEXT;
    }  

	if (CheckIfAnyDimIsZero(region)														||
		(buffer_row_pitch	!=0 && buffer_row_pitch		<region[0])						|| 
		(host_row_pitch		!=0 && host_row_pitch		<region[0])						|| 
		(buffer_slice_pitch	!=0 && buffer_slice_pitch	<(region[1]*buffer_row_pitch))	|| 
		(host_slice_pitch	!=0 && host_slice_pitch		<(region[1]*host_row_pitch))  
		)
	{
		return CL_INVALID_VALUE;
	}

	SetIfZero(buffer_row_pitch		, region[0]);
	SetIfZero(host_row_pitch		, region[0]);
	SetIfZero(buffer_slice_pitch	, region[1] * buffer_row_pitch);
	SetIfZero(host_slice_pitch		, region[1] * host_row_pitch);
	
	
    if (CL_SUCCESS != (errVal = pBuffer->CheckBoundsRect(szBufferOrigin, region, buffer_row_pitch, buffer_slice_pitch)))
    {
        // Out of bounds check.
        return errVal;
    }

	if ( NULL != pBuffer->GetParent() )
	{		
		if (!pBuffer->IsSupportedByDevice(pCommandQueue->GetDefaultDevice()))
		{
			return CL_MISALIGNED_SUB_BUFFER_OFFSET;
		}
	}
	       

	Command* pWriteBufferRectCmd = new WriteBufferRectCommand(pCommandQueue, m_pOclEntryPoints, bBlocking, pBuffer, szBufferOrigin, szHostOrigin, region, buffer_row_pitch, 
		buffer_slice_pitch, host_row_pitch, host_slice_pitch, pOutData);
	
	if (NULL == pWriteBufferRectCmd)
	{
		return CL_OUT_OF_HOST_MEMORY;
	}

    errVal = pWriteBufferRectCmd->Init();
	if ( CL_FAILED(errVal) )
	{
		delete pWriteBufferRectCmd;
	    return  errVal;
	}

    errVal = pCommandQueue->EnqueueCommand(pWriteBufferRectCmd, CL_FALSE, uNumEventsInWaitList, cpEeventWaitList, pEvent);
    if(CL_FAILED(errVal))
    {
		pWriteBufferRectCmd->CommandDone();
		delete pWriteBufferRectCmd;
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
    IOclCommandQueueBase* pCommandQueue = GetCommandQueue(clCommandQueue);
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

    if (pSrcBuffer->GetContext()->GetId() != pCommandQueue->GetContextId()  ||
        pSrcBuffer->GetContext()->GetId() != pDstBuffer->GetContext()->GetId() 
        )
    {
        return CL_INVALID_CONTEXT;
    }

    // Check boundaries.
    if (CL_SUCCESS != (errVal = pSrcBuffer->CheckBounds(&szSrcOffset,&szCb)))
    {
        return errVal;
    }
    if (CL_SUCCESS != (errVal = pDstBuffer->CheckBounds(&szDstOffset,&szCb)))
    {
        return errVal;
    }
	const size_t pszSrcOrigin[3] = { szSrcOffset, 0, 0 };
	const size_t pszDstOrigin[3] = { szDstOffset, 0, 0 };
	const size_t pszRegion[3] = { szCb, 1, 1 };

    if( clSrcBuffer == clDstBuffer)
    {
        // Check overlapping
        if (CheckMemoryObjectOverlapping(pSrcBuffer, pszSrcOrigin, pszDstOrigin, pszRegion))
        {
            return CL_MEM_COPY_OVERLAP;
        }
    }

    Command* pCopyBufferCommand = new CopyBufferCommand(pCommandQueue, m_pOclEntryPoints, pSrcBuffer, pDstBuffer, pszSrcOrigin, pszDstOrigin, pszRegion);
	if (NULL == pCopyBufferCommand)
	{
		return CL_OUT_OF_HOST_MEMORY;
	}

    errVal = pCopyBufferCommand->Init();
	if ( CL_FAILED(errVal) )
	{
		delete pCopyBufferCommand;
	    return  errVal;
	}

    // Enqueue copy command, never blocking
    errVal = pCommandQueue->EnqueueCommand(pCopyBufferCommand, CL_FALSE, uNumEventsInWaitList, cpEeventWaitList, pEvent);
    if(CL_FAILED(errVal))
    {
        // Enqueue failed, free resources
        pCopyBufferCommand->CommandDone();
        delete pCopyBufferCommand;
    }

	return  errVal;
}

/******************************************************************
 * 
 ******************************************************************/

cl_err_code  ExecutionModule::EnqueueCopyBufferRect (
						cl_command_queue	clCommandQueue, 
						cl_mem				clSrcBuffer, 
						cl_mem				clDstBuffer, 
						const size_t		szSrcOrigin[MAX_WORK_DIM], 
						const size_t		szDstOrigin[MAX_WORK_DIM], 
						const size_t		region[MAX_WORK_DIM], 
						size_t				src_buffer_row_pitch, 
						size_t				src_buffer_slice_pitch, 
						size_t				dst_buffer_row_pitch, 
						size_t				dst_buffer_slice_pitch, 
						cl_uint				uNumEventsInWaitList, 
						const cl_event*		cpEeventWaitList, 
						cl_event* pEvent)
{
	cl_err_code errVal = CL_SUCCESS;
    IOclCommandQueueBase* pCommandQueue = GetCommandQueue(clCommandQueue);
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

    if (pSrcBuffer->GetContext()->GetId() != pCommandQueue->GetContextId()  ||
        pSrcBuffer->GetContext()->GetId() != pDstBuffer->GetContext()->GetId() 
        )
    {
        return CL_INVALID_CONTEXT;
    }

	if (CheckIfAnyDimIsZero(region)																	||
		(src_buffer_row_pitch	!=0 && src_buffer_row_pitch		<region[0])							|| 
		(dst_buffer_row_pitch	!=0 && dst_buffer_row_pitch		<region[0])							|| 
		(src_buffer_slice_pitch	!=0 && src_buffer_slice_pitch	<(region[1]*src_buffer_row_pitch))	|| 
		(dst_buffer_slice_pitch	!=0 && dst_buffer_slice_pitch	<(region[1]*dst_buffer_row_pitch))  
		)
	{
		
		return CL_INVALID_VALUE;
	}

	// Check boundaries.
	SetIfZero(src_buffer_row_pitch		, region[0]);
	SetIfZero(dst_buffer_row_pitch		, region[0]);
	SetIfZero(src_buffer_slice_pitch	, region[1] * src_buffer_row_pitch);
	SetIfZero(dst_buffer_slice_pitch	, region[1] * dst_buffer_row_pitch);
	
	
    if (CL_SUCCESS != (errVal = pSrcBuffer->CheckBoundsRect(szSrcOrigin, region, src_buffer_row_pitch, src_buffer_slice_pitch)) || 
		CL_SUCCESS != (errVal = pDstBuffer->CheckBoundsRect(szDstOrigin, region, dst_buffer_row_pitch, dst_buffer_slice_pitch)))

    {
        // Out of bounds check.
        return errVal;
    }

	if ( NULL != pSrcBuffer->GetParent())
	{		
		if (!pSrcBuffer->IsSupportedByDevice(pCommandQueue->GetDefaultDevice()))
		{
			return CL_MISALIGNED_SUB_BUFFER_OFFSET;
		}
	}

	if ( NULL != pDstBuffer->GetParent())
	{		
		if (!pDstBuffer->IsSupportedByDevice(pCommandQueue->GetDefaultDevice()))
		{
			return CL_MISALIGNED_SUB_BUFFER_OFFSET;
		}
	}
   
    if( clSrcBuffer == clDstBuffer)
    {
        // Check overlapping
        if (CheckMemoryObjectOverlapping(pSrcBuffer, szSrcOrigin, szDstOrigin, region))
        {
			// Rami todo
			//assert(false && "Rami added: CL_MEM_COPY_OVERLAP");
            return CL_MEM_COPY_OVERLAP;
        }
    }
	
    Command* pCopyBufferRectCommand = new CopyBufferRectCommand(pCommandQueue, m_pOclEntryPoints, pSrcBuffer, pDstBuffer, szSrcOrigin, szDstOrigin, region,
		src_buffer_row_pitch, src_buffer_slice_pitch, dst_buffer_row_pitch, dst_buffer_slice_pitch);
	if (NULL == pCopyBufferRectCommand)
	{
		return CL_OUT_OF_HOST_MEMORY;
	}

    errVal = pCopyBufferRectCommand->Init();
	if ( CL_FAILED(errVal) )
	{
		delete pCopyBufferRectCommand;
	    return  errVal;
	}

    // Enqueue copy command, never blocking
    errVal = pCommandQueue->EnqueueCommand(pCopyBufferRectCommand, CL_FALSE, uNumEventsInWaitList, cpEeventWaitList, pEvent);
    if(CL_FAILED(errVal))
    {
        // Enqueue failed, free resources
        pCopyBufferRectCommand->CommandDone();
        delete pCopyBufferRectCommand;
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
    IOclCommandQueueBase* pCommandQueue = GetCommandQueue(clCommandQueue);
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

    if (pBuffer->GetContext()->GetId() != pCommandQueue->GetContextId())
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
    
    MapBufferCommand* pMapBufferCommand = new MapBufferCommand(pCommandQueue, m_pOclEntryPoints, pBuffer, clMapFlags, szOffset, szCb);
    // Must set device Id before init for buffer resource allocation.
	if (NULL == pMapBufferCommand)
	{
		*pErrcodeRet = CL_OUT_OF_HOST_MEMORY;
		return NULL;
	}

    *pErrcodeRet = pMapBufferCommand->Init();
	if ( CL_FAILED(*pErrcodeRet))
    {
		delete pMapBufferCommand;
        return  NULL;
    }

	// Get pointer for mapped region since it is allocated on init. Execute will lock the region
    // Note that if EnqueueCommand succeeded, by the time it returns, the command may be deleted already.
    void* mappedPtr = pMapBufferCommand->GetMappedPtr();

    *pErrcodeRet = pCommandQueue->EnqueueCommand(pMapBufferCommand, bBlockingMap, uNumEventsInWaitList, cpEeventWaitList, pEvent);
    if(CL_FAILED(*pErrcodeRet))
    {
        // Enqueue failed, free resources
        pMapBufferCommand->CommandDone();
        delete pMapBufferCommand;
        return NULL;
    }
    return mappedPtr;
}

/******************************************************************
 * 
 ******************************************************************/
cl_err_code ExecutionModule::EnqueueUnmapMemObject(cl_command_queue clCommandQueue,cl_mem clMemObj, void* mappedPtr, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent)
{
    cl_err_code errVal;
    IOclCommandQueueBase* pCommandQueue = GetCommandQueue(clCommandQueue);
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }
    
    MemoryObject* pMemObject = m_pContextModule->GetMemoryObject(clMemObj);    
    if (NULL == pMemObject)
    {
        return  CL_INVALID_MEM_OBJECT;
    }

    if (pMemObject->GetContext()->GetId() != pCommandQueue->GetContextId())
    {
        return CL_INVALID_CONTEXT;
    }
    
    Command* pUnmapMemObjectCommand = new UnmapMemObjectCommand(pCommandQueue, m_pOclEntryPoints, pMemObject, mappedPtr);
    // Must set device Id before init for buffer resource allocation.
	if (NULL == pUnmapMemObjectCommand)
	{
		return CL_OUT_OF_HOST_MEMORY;
	}

    errVal = pUnmapMemObjectCommand->Init();
	if ( CL_FAILED(errVal) )
	{
		delete pUnmapMemObjectCommand;
	    return  errVal;
	}

    errVal = pCommandQueue->EnqueueCommand(pUnmapMemObjectCommand, CL_FALSE /*never blocks*/, uNumEventsInWaitList, cpEeventWaitList, pEvent);
    if(CL_FAILED(errVal))
    {
        // Enqueue failed, free resources
        pUnmapMemObjectCommand->CommandDone();
        delete pUnmapMemObjectCommand;
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

    IOclCommandQueueBase* pCommandQueue = GetCommandQueue(clCommandQueue);
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }
    Kernel* pKernel = m_pContextModule->GetKernel(clKernel);
    if (NULL == pKernel)
    {
        return CL_INVALID_KERNEL;
    }

    if (pKernel->GetContext()->GetId() != pCommandQueue->GetContextId())
    {
        return CL_INVALID_CONTEXT;
    }

	FissionableDevice* pDevice = pCommandQueue->GetDefaultDevice();
	
    // CL_INVALID_PROGRAM_EXECUTABLE if there is no successfully built program
    // executable available for device associated with command_queue.
    if(!pKernel->IsValidExecutable(pDevice))
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
    pKernel->GetWorkGroupInfo(pDevice, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &szWorkGroupSize, NULL);
    pKernel->GetWorkGroupInfo(pDevice, CL_KERNEL_COMPILE_WORK_GROUP_SIZE, sizeof(size_t) * 3, szComplieWorkGroupSize, NULL);
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
    // Check that if local_work_size is specified and number of work items specified by 
    // global_work_size is not evenly divisible by size of work-group given by local_work_size
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
		if (uiMaxWorkItemDim == 0)
		{
			return CL_INVALID_WORK_ITEM_SIZE;
		}
        clLocalArray<size_t> pszMaxWorkItemSizes(uiMaxWorkItemDim);
        pDevice->GetInfo(CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(size_t)*uiMaxWorkItemDim, pszMaxWorkItemSizes, NULL);

        for( ui =0; ui<uiWorkDim; ui++)
        {
            if( cpszLocalWorkSize[ui] > pszMaxWorkItemSizes[ui])
            {
                return CL_INVALID_WORK_ITEM_SIZE;
            }
        }
    }

    
    // TODO: create buffer resources in advance, if they are not exists,
    //      On error return: CL_OUT_OF_RESOURCES

    Command* pNDRangeKernelCmd = new NDRangeKernelCommand(pCommandQueue, m_pOclEntryPoints, pKernel, uiWorkDim, cpszGlobalWorkOffset, cpszGlobalWorkSize, cpszLocalWorkSize); 
	if ( NULL == pNDRangeKernelCmd )
	{
		return CL_OUT_OF_HOST_MEMORY;
	}
    // Must set device Id before init for buffer resource allocation.
	pNDRangeKernelCmd->SetDevice(pDevice);
    errVal = pNDRangeKernelCmd->Init();
	if ( CL_FAILED(errVal) )
	{
		delete pNDRangeKernelCmd;
	    return  errVal;
	}

    errVal = pCommandQueue->EnqueueCommand(pNDRangeKernelCmd, false/*never blocking*/, uNumEventsInWaitList, cpEeventWaitList, pEvent);
    if(CL_FAILED(errVal))
    {
        // Enqueue failed, free resources
        pNDRangeKernelCmd->CommandDone();
        delete pNDRangeKernelCmd;
    }

    return  errVal;
}


/******************************************************************
 * 
 ******************************************************************/
cl_err_code ExecutionModule::EnqueueTask( cl_command_queue clCommandQueue, cl_kernel clKernel, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent)
{
    cl_err_code errVal = CL_SUCCESS;

    IOclCommandQueueBase* pCommandQueue = GetCommandQueue(clCommandQueue);
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }
    Kernel* pKernel = m_pContextModule->GetKernel(clKernel);
    if (NULL == pKernel)
    {
        return CL_INVALID_KERNEL;
    }

    if (pKernel->GetContext()->GetId() != pCommandQueue->GetContextId())
    {
        return CL_INVALID_CONTEXT;
    }

	// CL_INVALID_PROGRAM_EXECUTABLE if there is no successfully built program
    // executable available for device associated with command_queue.
	if(!pKernel->IsValidExecutable(pCommandQueue->GetDefaultDevice()) )
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

    Command* pTaskCommand = new TaskCommand(pCommandQueue, m_pOclEntryPoints, pKernel); 
    // Must set device Id before init for buffer resource allocation.
	if (NULL == pTaskCommand)
	{
		return CL_OUT_OF_HOST_MEMORY;
	}

    errVal = pTaskCommand->Init();
	if ( CL_FAILED(errVal) )
	{
		delete pTaskCommand;
	    return  errVal;
	}

    errVal = pCommandQueue->EnqueueCommand(pTaskCommand, false/*never blocking*/, uNumEventsInWaitList, cpEeventWaitList, pEvent);
    if(CL_FAILED(errVal))
    {
        // Enqueue failed, free resources
        pTaskCommand->CommandDone();
        delete pTaskCommand;
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

    IOclCommandQueueBase* pCommandQueue = GetCommandQueue(clCommandQueue);
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }
	MemoryObject** pMemObjectsList = NULL;
	if (uNumMemObjects > 0)
	{
		// Create MemoryObjects references
		pMemObjectsList = new MemoryObject*[uNumMemObjects];
	    
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
				delete[] pMemObjectsList;
				return CL_INVALID_MEM_OBJECT;
			}
		}
	}

    // TODO: Handle those error values, probably through the DEVICE object...
    // CL_INVALID_OPERATION
   
    Command* pNativeKernelCommand = new NativeKernelCommand(pCommandQueue, m_pOclEntryPoints, pUserFnc, pArgs, szCbArgs, uNumMemObjects, pMemObjectsList, ppArgsMemLoc );
	if(NULL == pNativeKernelCommand)
	{
		if ( NULL != pMemObjectsList )
		{
			delete []pMemObjectsList;
		}
		return CL_OUT_OF_HOST_MEMORY;
	}

    errVal = pNativeKernelCommand->Init();
	if ( CL_FAILED(errVal) )
	{
		if ( NULL != pMemObjectsList )
		{
			delete []pMemObjectsList;
		}
        delete pNativeKernelCommand;
	    return  errVal;
	}

    errVal = pCommandQueue->EnqueueCommand(pNativeKernelCommand, false/*never blocking*/, uNumEventsInWaitList, cpEeventWaitList, pEvent);
    if(CL_FAILED(errVal))
    {
        // Enqueue failed, free resources
		// pMemObjectsList is released in CommandDone()
        pNativeKernelCommand->CommandDone();
		if ( NULL != pMemObjectsList )
		{
			delete []pMemObjectsList;
		}
        delete pNativeKernelCommand;
    }

	return  errVal;
}

/******************************************************************
 *
 ******************************************************************/
inline cl_err_code ExecutionModule::Check2DImageParameters( MemoryObject* pImage, const size_t szOrigin[MAX_WORK_DIM], const size_t szRegion[MAX_WORK_DIM])
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
    if (CL_MEM_OBJECT_IMAGE2D_ARRAY == pImage->GetType())
    {
        if (1 != szRegion[2])
        {
            errVal = CL_INVALID_VALUE;
        }
    }
    return errVal;
}

inline bool DimensionsOverlap(size_t d1_min, size_t d1_max, size_t d2_min, size_t d2_max)
{
	assert (d1_max >= d1_min);
	assert (d2_max >= d2_min);
	if ((d1_min == d1_max) || (d2_min == d2_max))
	{
		return false;
	}
	return !((d1_min >= d2_max) || (d2_min >= d1_max));
}

/******************************************************************
 * Returns true if regions in pMemObj overlap
 ******************************************************************/
inline bool ExecutionModule::CheckMemoryObjectOverlapping(MemoryObject* pMemObj, const size_t* szSrcOrigin, const size_t* szDstOrigin, const size_t* szRegion)
{
    bool isOverlaps = true;
    cl_mem_object_type memObjType = pMemObj->GetType();
	const size_t src_min[] = {szSrcOrigin[0], szSrcOrigin[1], szSrcOrigin[2]};
	const size_t src_max[] = {szSrcOrigin[0]+szRegion[0], szSrcOrigin[1]+szRegion[1], szSrcOrigin[2]+szRegion[2]};

	const size_t dst_min[] = {szDstOrigin[0], szDstOrigin[1], szDstOrigin[2]};
	const size_t dst_max[] = {szDstOrigin[0]+szRegion[0], szDstOrigin[1]+szRegion[1], szDstOrigin[2]+szRegion[2]};    

	size_t dimensionsToCompare = 0;

    switch(memObjType)
    {
	case CL_MEM_OBJECT_IMAGE3D:
		dimensionsToCompare = 3;
		break;

	case CL_MEM_OBJECT_IMAGE2D_ARRAY:
		if (szSrcOrigin[2] != szDstOrigin[2])

		{
			// For image array with different image index, no need to compare any boundaries 
			// keep dimensionToCompare at 0.
			break;
		}
		dimensionsToCompare = 2;
		break;

	case CL_MEM_OBJECT_IMAGE2D:
		dimensionsToCompare = 2;
		break;

	case CL_MEM_OBJECT_BUFFER:
		dimensionsToCompare = 3;
		break;

    default:
		assert(0 && "Illegal type of memory object");
        break;
    }
	for (size_t dimension = 0; dimension < dimensionsToCompare; ++dimension)
	{
		isOverlaps &= DimensionsOverlap(src_min[dimension], src_max[dimension], dst_min[dimension], dst_max[dimension]);
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
inline cl_err_code ExecutionModule::CheckImageFormats( MemoryObject* pSrcImage, MemoryObject* pDstImage)
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
                                const size_t     szOrigin[MAX_WORK_DIM],
                                const size_t     szRegion[MAX_WORK_DIM],
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

    IOclCommandQueueBase* pCommandQueue = GetCommandQueue(clCommandQueue);
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }
    
    MemoryObject* pImage = m_pContextModule->GetMemoryObject(clImage);
    if (NULL == pImage)
    {
        return CL_INVALID_MEM_OBJECT;
    }

    if (pImage->GetContext()->GetId() != pCommandQueue->GetContextId())
    {
        return CL_INVALID_CONTEXT;
    }

    if (CL_SUCCESS != (errVal = pImage->CheckBounds(szOrigin, szRegion)))
    {
        return errVal;
    }
    if (CL_FAILED(Check2DImageParameters(pImage, szOrigin, szRegion))  ||
        ((CL_MEM_OBJECT_IMAGE2D == pImage->GetType()) && (0 != szSlicePitch))
        )
    {
        return CL_INVALID_VALUE;
    }

    Command* pReadImageCmd  = new ReadImageCommand(pCommandQueue, m_pOclEntryPoints, pImage, szOrigin, szRegion, szRowPitch, szSlicePitch, pOutData);
	if (NULL == pReadImageCmd)
	{
		return CL_OUT_OF_HOST_MEMORY;
	}

    errVal = pReadImageCmd->Init();
	if ( CL_FAILED(errVal) )
	{
        delete pReadImageCmd;
	    return  errVal;
	}

    errVal = pCommandQueue->EnqueueCommand(pReadImageCmd, bBlocking, uNumEventsInWaitList, cpEeventWaitList, pEvent);
    if(CL_FAILED(errVal))
    {
        // Enqueue failed, free resources
        pReadImageCmd->CommandDone();
        delete pReadImageCmd;
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
                                const size_t     szOrigin[MAX_WORK_DIM],
                                const size_t     szRegion[MAX_WORK_DIM],
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

    IOclCommandQueueBase* pCommandQueue = GetCommandQueue(clCommandQueue);
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }
    
    MemoryObject* pImage = m_pContextModule->GetMemoryObject(clImage);
    if (NULL == pImage)
    {
        return CL_INVALID_MEM_OBJECT;
    }

    if (pImage->GetContext()->GetId() != pCommandQueue->GetContextId())
    {
        return CL_INVALID_CONTEXT;
    }

    if (CL_SUCCESS != (errVal = pImage->CheckBounds(szOrigin, szRegion)))
    {
        return errVal;
    }
    if (CL_FAILED(Check2DImageParameters(pImage, szOrigin, szRegion))  ||
        ((CL_MEM_OBJECT_IMAGE2D == pImage->GetType()) && (0 != szSlicePitch))
        )
    {
        return CL_INVALID_VALUE;
    }

    Command* pWriteImageCmd  = new WriteImageCommand(pCommandQueue, m_pOclEntryPoints, bBlocking, pImage, szOrigin, szRegion, szRowPitch, szSlicePitch, cpSrcData);
	if (NULL == pWriteImageCmd)
	{
		return CL_OUT_OF_HOST_MEMORY;
	}

	errVal = pWriteImageCmd->Init();
	if ( CL_FAILED(errVal) )
	{
        delete pWriteImageCmd;
	    return  errVal;
	}

    errVal = pCommandQueue->EnqueueCommand(pWriteImageCmd, CL_FALSE, uNumEventsInWaitList, cpEeventWaitList, pEvent);
    if(CL_FAILED(errVal))
    {
        // Enqueue failed, free resources
        pWriteImageCmd->CommandDone();
        delete pWriteImageCmd;
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
                                const size_t     szSrcOrigin[MAX_WORK_DIM],
                                const size_t     szDstOrigin[MAX_WORK_DIM],
                                const size_t     szRegion[MAX_WORK_DIM],
                                cl_uint          uNumEventsInWaitList, 
                                const cl_event*  cpEeventWaitList, 
                                cl_event*        pEvent      
                                )
{
    cl_err_code errVal = CL_SUCCESS;
    IOclCommandQueueBase* pCommandQueue = GetCommandQueue(clCommandQueue);
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

    if (pSrcImage->GetContext()->GetId() != pCommandQueue->GetContextId()  ||
        pSrcImage->GetContext()->GetId() != pDstImage->GetContext()->GetId() 
        )
    {
        return CL_INVALID_CONTEXT;
    }
    
    // Check format
    errVal = CheckImageFormats(pSrcImage, pDstImage);
    if(CL_FAILED(errVal))
    {
        return CL_IMAGE_FORMAT_MISMATCH;
    }

    // Check boundaries.
    if (CL_SUCCESS != (errVal = pSrcImage->CheckBounds(szSrcOrigin,szRegion)) || CL_SUCCESS != (errVal = pDstImage->CheckBounds(szDstOrigin,szRegion)))
    {
        return errVal;
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
    Command* pCopyImageCmd = new CopyImageCommand(pCommandQueue, m_pOclEntryPoints, pSrcImage, pDstImage, szSrcOrigin, szDstOrigin, szRegion);
	if (NULL == pCopyImageCmd)
	{
		return CL_OUT_OF_HOST_MEMORY;
	}

    errVal = pCopyImageCmd->Init();
	if ( CL_FAILED(errVal) )
	{
        delete pCopyImageCmd;
	    return  errVal;
	}

    // Enqueue copy command, never blocking
    errVal = pCommandQueue->EnqueueCommand(pCopyImageCmd, CL_FALSE, uNumEventsInWaitList, cpEeventWaitList, pEvent);
    if(CL_FAILED(errVal))
    {
        // Enqueue failed, free resources
        pCopyImageCmd->CommandDone();
        delete pCopyImageCmd;
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
                                const size_t     szSrcOrigin[MAX_WORK_DIM],
                                const size_t     szRegion[MAX_WORK_DIM],
                                size_t           szDstOffset,
                                cl_uint          uNumEventsInWaitList, 
                                const cl_event*  cpEeventWaitList, 
                                cl_event*        pEvent
                                )
{
    cl_err_code errVal = CL_SUCCESS;
    IOclCommandQueueBase* pCommandQueue = GetCommandQueue(clCommandQueue);
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

    if (pSrcImage->GetContext()->GetId() != pCommandQueue->GetContextId()  ||
        pSrcImage->GetContext()->GetId() != pDstBuffer->GetContext()->GetId() 
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
    if (CL_SUCCESS != (errVal = pSrcImage->CheckBounds(szSrcOrigin,szRegion)) || CL_SUCCESS != (errVal = pDstBuffer->CheckBounds(&szDstOffset,&szDstCb)))
    {
        return errVal;
    }

    //
    // Input parameters validated, enqueue the command
    //
	size_t	pszDstOffset[3] = {szDstOffset,0,0};
    Command* pCopyImageToBufferCmd = new CopyImageToBufferCommand(pCommandQueue, m_pOclEntryPoints, pSrcImage, pDstBuffer, szSrcOrigin, szRegion, pszDstOffset/*szDstOffset*/);
	if (NULL == pCopyImageToBufferCmd)
	{
		return CL_OUT_OF_HOST_MEMORY;
	}

	errVal = pCopyImageToBufferCmd->Init();
	if ( CL_FAILED(errVal) )
	{
        delete pCopyImageToBufferCmd;
	    return  errVal;
	}

    // Enqueue copy command, never blocking
    errVal = pCommandQueue->EnqueueCommand(pCopyImageToBufferCmd, CL_FALSE, uNumEventsInWaitList, cpEeventWaitList, pEvent);
    if(CL_FAILED(errVal))
    {
        // Enqueue failed, free resources
        pCopyImageToBufferCmd->CommandDone();
        delete pCopyImageToBufferCmd;
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
                                const size_t     szDstOrigin[MAX_WORK_DIM],
                                const size_t     szRegion[MAX_WORK_DIM],
                                cl_uint          uNumEventsInWaitList,
                                const cl_event*  cpEeventWaitList, 
                                cl_event*        pEvent
                                )
{
    cl_err_code errVal = CL_SUCCESS;
    IOclCommandQueueBase* pCommandQueue = GetCommandQueue(clCommandQueue);
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

    if (pSrcBuffer->GetContext()->GetId() != pCommandQueue->GetContextId()  ||
        pSrcBuffer->GetContext()->GetId() != pDstImage->GetContext()->GetId() 
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
    if (CL_SUCCESS != (errVal = pSrcBuffer->CheckBounds(&szSrcOffset,&szDstCb)) || CL_SUCCESS != (errVal = pDstImage->CheckBounds(szDstOrigin,szRegion)))
    {
        return errVal;
    }

    //
    // Input parameters validated, enqueue the command
    //

	size_t	pszSrcOffset[3] = {szSrcOffset,0,0};
    Command* pCopyBufferToImageCmd = new CopyBufferToImageCommand(pCommandQueue, m_pOclEntryPoints, pSrcBuffer, pDstImage, pszSrcOffset, szDstOrigin, szRegion);
	if (NULL == pCopyBufferToImageCmd)	
	{
		return CL_OUT_OF_HOST_MEMORY;
	}

	errVal = pCopyBufferToImageCmd->Init();
	if ( CL_FAILED(errVal) )
	{
        delete pCopyBufferToImageCmd;
	    return  errVal;
	}

    // Enqueue copy command, never blocking
    errVal = pCommandQueue->EnqueueCommand(pCopyBufferToImageCmd, CL_FALSE, uNumEventsInWaitList, cpEeventWaitList, pEvent);
    if(CL_FAILED(errVal))
    {
        // Enqueue failed, free resources
        pCopyBufferToImageCmd->CommandDone();
        delete pCopyBufferToImageCmd;
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
    const size_t        szOrigin[MAX_WORK_DIM], 
    const size_t        szRegion[MAX_WORK_DIM], 
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

    IOclCommandQueueBase* pCommandQueue = GetCommandQueue(clCommandQueue);
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
    else if (pImage->GetContext()->GetId() != pCommandQueue->GetContextId())
    {
        *pErrcodeRet = CL_INVALID_CONTEXT;
        return NULL;
    }    
    else
    {
        if (CL_SUCCESS != (err = pImage->CheckBounds(szOrigin, szRegion)))
        {
            *pErrcodeRet = err;
        }
        else
        {
            if (CL_FAILED(Check2DImageParameters(pImage, szOrigin, szRegion))   ||
                (NULL == pszImageRowPitch)                                      ||
                ((CL_MEM_OBJECT_IMAGE3D == pImage->GetType()) && (NULL == pszImageSlicePitch))  
                )
            {
                *pErrcodeRet = CL_INVALID_VALUE;
            }
        }
    }

    if(CL_FAILED(*pErrcodeRet))
    {
        return NULL;
    }
        
    MapImageCommand* pMapImageCmd = new MapImageCommand(pCommandQueue, m_pOclEntryPoints, pImage, clMapFlags, szOrigin, szRegion, pszImageRowPitch, pszImageSlicePitch);
    // Must set device Id before init for image resource allocation.
	if (NULL == pMapImageCmd)
	{
		*pErrcodeRet = CL_OUT_OF_HOST_MEMORY;
		return NULL;
	}

	*pErrcodeRet = pMapImageCmd->Init();
    if(CL_FAILED(*pErrcodeRet))
    {
		delete pMapImageCmd;
        return  NULL;
    }

    // Get pointer for mapped region since it is allocated on init. Execute will lock the region
    // Note that if EnqueueCommand succeeded, by the time it returns, the command may be deleted already.
    void* mappedPtr = pMapImageCmd->GetMappedPtr();
    *pErrcodeRet = pCommandQueue->EnqueueCommand(pMapImageCmd, bBlockingMap, uNumEventsInWaitList, cpEeventWaitList, pEvent);
    if(CL_FAILED(*pErrcodeRet))
    {
        // Enqueue failed, free resources
        pMapImageCmd->CommandDone();
        delete pMapImageCmd;
        return NULL;
    }

    return mappedPtr;
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
cl_err_code ExecutionModule::EnqueueSyncGLObjects(cl_command_queue clCommandQueue,
													 cl_command_type cmdType, 
													 cl_uint uiNumObjects, 
													 const cl_mem * pclMemObjects, 
													 cl_uint uiNumEventsInWaitList, 
													 const cl_event * pclEventWaitList, 
													 cl_event * pclEvent)
{
#if defined (_WIN32) //TODO GL support for Linux
	cl_err_code errVal = CL_SUCCESS;
	if ( (NULL == pclMemObjects) || (0 == uiNumObjects) )
	{
		return CL_INVALID_VALUE;
	}

	IOclCommandQueueBase* pCommandQueue = GetCommandQueue(clCommandQueue);
	if (NULL == pCommandQueue)
	{
		return CL_INVALID_COMMAND_QUEUE;
	}

	GLContext* pContext = dynamic_cast<GLContext*>(m_pContextModule->GetContext(pCommandQueue->GetContextHandle()));
	if (NULL == pContext)
	{
		return CL_INVALID_CONTEXT;
	}
	
	GLMemoryObject* *pMemObjects = new GLMemoryObject*[uiNumObjects];
	if ( NULL == pMemObjects )
	{
		return CL_OUT_OF_HOST_MEMORY;
	}
	for(unsigned int i=0; i<uiNumObjects; ++i)
	{
		MemoryObject* pMemObj = m_pContextModule->GetMemoryObject(pclMemObjects[i]);
		if (NULL == pMemObj)
		{
			delete []pMemObjects;
			return CL_INVALID_MEM_OBJECT;
		}
		if (pMemObj->GetContext()->GetId() != pCommandQueue->GetContextId())
		{
			delete []pMemObjects;
			return CL_INVALID_CONTEXT;
		}
		// Check if it's a GL object
 		if ( NULL != (pMemObjects[i] = dynamic_cast<GLMemoryObject*>(pMemObj)) )
		{
			continue;
		}

		// If got here invalid GL buffer
		delete []pMemObjects;
		return CL_INVALID_GL_OBJECT;
	}

	Command* pAcquireCmd  = new SyncGLObjects(cmdType, pContext, pMemObjects, uiNumObjects, pCommandQueue, m_pOclEntryPoints);
	if (NULL == pAcquireCmd)
	{
		delete []pMemObjects;
		return CL_OUT_OF_HOST_MEMORY;
	}

	errVal = pAcquireCmd->Init();
	if ( CL_FAILED(errVal) )
	{
        delete pAcquireCmd;
		delete []pMemObjects;
	    return  errVal;
	}

	errVal = pCommandQueue->EnqueueCommand(pAcquireCmd, FALSE, uiNumEventsInWaitList, pclEventWaitList, pclEvent);
	if(CL_FAILED(errVal))
	{
		// Enqueue failed, free resources
		pAcquireCmd->CommandDone();
		delete pAcquireCmd;
	}

	delete []pMemObjects;
	return  errVal;
#else
	assert (0 && "NOT Implemented on Linux");
        return CL_OUT_OF_HOST_MEMORY;
#endif
}

cl_err_code ExecutionModule::FlushAllQueuesForContext(cl_context clEventsContext)
{
	cl_err_code errVal = CL_SUCCESS;
	OCLObject<_cl_command_queue_int>* pObj = NULL;	
	for (cl_uint ui=0; ui<m_pOclCommandQueueMap->Count(); ++ui)
	{
		errVal = m_pOclCommandQueueMap->GetObjectByIndex(ui, &pObj);
		if ( CL_FAILED(errVal) )
		{
			return errVal;
		}
		IOclCommandQueueBase* pQueue = (IOclCommandQueueBase*)pObj;

		cl_context queueContext = pQueue->GetContextHandle();
		if(queueContext == clEventsContext)
		{
			// Flush
			pQueue->Flush(false);
		}
	}
	return errVal;
}

#if defined (DX9_MEDIA_SHARING)
cl_int ExecutionModule::EnqueueSyncD3D9Objects(cl_command_queue clCommandQueue,
                                                 cl_command_type cmdType, cl_uint uiNumObjects,
                                                 const cl_mem *pclMemObjects,
                                                 cl_uint uiNumEventsInWaitList,
                                                 const cl_event *pclEventWaitList,
                                                 cl_event *pclEvent)
{
    cl_err_code errVal = CL_SUCCESS;
    if (NULL == pclMemObjects || 0 == uiNumObjects)
    {
        return CL_INVALID_VALUE;
    }

    IOclCommandQueueBase* const pCommandQueue = GetCommandQueue(clCommandQueue);
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    D3D9Context* const pContext = dynamic_cast<D3D9Context*>(m_pContextModule->GetContext(pCommandQueue->GetContextHandle()));
    if (NULL == pContext)
    {
        return CL_INVALID_CONTEXT;
    }

    D3D9Resource** const pMemObjects = new D3D9Resource*[uiNumObjects];
    if (NULL == pMemObjects)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }
    for (cl_uint i = 0; i < uiNumObjects; i++)
    {
        MemoryObject* const pMemObj = m_pContextModule->GetMemoryObject(pclMemObjects[i]);
        if (NULL == pMemObj)
        {
            delete[] pMemObjects;
            return CL_INVALID_MEM_OBJECT;
        }
        if (pMemObj->GetContext()->GetId() != pCommandQueue->GetContextId())
        {
            delete[] pMemObjects;
            return CL_INVALID_CONTEXT;
        }
        D3D9Resource* const pD3d9Resource = pMemObjects[i] = dynamic_cast<D3D9Resource*>(pMemObj);
        // Check if it's a Direct3D 9 object
        if (NULL != pD3d9Resource)
        {
            if (CL_COMMAND_ACQUIRE_DX9_OBJECTS_INTEL == cmdType && pD3d9Resource->IsAcquired())
            {
                delete[] pMemObjects;
                return CL_DX9_RESOURCE_ALREADY_ACQUIRED_INTEL;
            }
            if (CL_COMMAND_RELEASE_DX9_OBJECTS_INTEL == cmdType && !pD3d9Resource->IsAcquired())
            {
                delete[] pMemObjects;
                return CL_DX9_RESOURCE_NOT_ACQUIRED_INTEL;
            }
            continue;
        }
        // If we've got here invalid Direct3D 9 object
        delete[] pMemObjects;
        return CL_INVALID_MEM_OBJECT;
    }

    Command* const pAcquireCmd = new SyncD3D9Resources(pCommandQueue, m_pOclEntryPoints, pMemObjects, uiNumObjects, cmdType);
    if (NULL == pAcquireCmd)
    {
        delete []pMemObjects;
        return CL_OUT_OF_HOST_MEMORY;
    }

    errVal = pAcquireCmd->Init();
    if (CL_SUCCEEDED(errVal))
    {
        errVal = pCommandQueue->EnqueueCommand(pAcquireCmd, false, uiNumEventsInWaitList, pclEventWaitList, pclEvent);
        if (CL_FAILED(errVal))
        {
            // Enqueue failed, free resources. pAcquireCmd->CommandDone() was already called in EnqueueCommand.            
            delete pAcquireCmd;
        }
        else
        {
            /* set the acquired state here already, so that if clEnqueuAcquire/Release is called
                after this and the command itself has been executed yet, we still return an error */
            for (size_t i = 0; i < uiNumObjects; i++)   
            {
                pMemObjects[i]->SetAcquired(CL_COMMAND_ACQUIRE_DX9_OBJECTS_INTEL == cmdType);
            }
        }
    } else
    {
        delete pAcquireCmd;
    }

    delete[] pMemObjects;
    return errVal;
}
#endif
