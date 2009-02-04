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
//  ocl_command_queue.cpp
//  Implementation of the Class OclCommandQueue
//  Created on:      23-Dec-2008 3:23:05 PM
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////
#include "ocl_command_queue.h"
#include "context.h"
#include "events_manager.h"
#include "enqueue_commands.h"
#include "queue_event.h"
#include "queue_worker_thread.h"
#include "in_order_queue.h"
#include "out_of_order_queue.h"

using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;

/******************************************************************
 * Command queue constructor
 ******************************************************************/
OclCommandQueue::OclCommandQueue(
    Context*                    pContext, 
    cl_device_id                clDefaultDeviceID,
    cl_command_queue_properties clProperties,
    EventsManager*              pEventsManager
    ):
    m_pContext(pContext),
    m_pEventsManager(pEventsManager),
    m_clDefaultDeviceId(clDefaultDeviceID),
    m_bCleanFinish(false),
    m_pQueueWorkerThread(NULL),
    m_pCommandQueue(NULL)
{
    m_pContext->GetDeviceByIndex((cl_uint)clDefaultDeviceID, &m_pDefaultDevice);    
    // Set queue options
    m_bOutOfOrderEnabled = ((clProperties & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE) ? true : false);
    m_bProfilingEnabled  = ((clProperties & CL_QUEUE_PROFILING_ENABLE) ? true : false );
    // Set logger
	m_pLoggerClient = new LoggerClient(L"OclCommandQueue Logger Client",LL_DEBUG);
	InfoLog(m_pLoggerClient, L"OclCommandQueue created");
}

/******************************************************************
 *
 ******************************************************************/
OclCommandQueue::~OclCommandQueue()
{
    // The constructor stops and deletes the worker thread
    // If there are still commands inside the queue, they are deleted.
    if (m_pQueueWorkerThread)
    {
        m_pQueueWorkerThread->Join();
        delete m_pQueueWorkerThread;
        m_pQueueWorkerThread = NULL;
    }

    if (m_pCommandQueue)
    {
        m_pCommandQueue->Release();
        delete m_pCommandQueue;
        m_pCommandQueue = NULL;
    }
    m_pContext = NULL;
    m_pDefaultDevice = NULL;
}

void ReleaseWorkerThread();

/******************************************************************
 * This function initilaizes the command,
 * Including: create the right queue and start the worker that should
 * process all commands. 
 ******************************************************************/
cl_err_code OclCommandQueue::Initialize()
{
    cl_err_code errVal;
    // Create worker thread and run it.
    if ( m_bOutOfOrderEnabled )
    {
        // Out of order is not supported yet.
        return CL_INVALID_OPERATION;
    }
    else
    {        
        m_pCommandQueue = new InOrderQueue();        
    }    
    m_pQueueWorkerThread = new QueueWorkerThread;
    errVal = m_pQueueWorkerThread->Init(m_pCommandQueue);
    if( CL_SUCCEEDED(errVal))
    {
        int startRet = m_pQueueWorkerThread->Start();
        if ( startRet < 0 )
        {
            errVal = CL_ERR_EXECUTION_FAILED;
        }
    }
    return errVal;
}

/******************************************************************
 * CleanFinish signals the command queue to finish processing all its 
 * commands, and when the queue is empty to release all its resources.
 * The function is return immediately, but after it returns any instance of
 * ir reference to this object is considered invalid and is no longer
 * accessable.
 * 
 ******************************************************************/
cl_err_code OclCommandQueue::CleanFinish()
{
    m_bCleanFinish = true;
	return CL_SUCCESS;	
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code OclCommandQueue::GetInfo( cl_command_queue_info clParamName, size_t szParamValueSize, void* pParamValue, size_t* pszParamValueSizeRet )
{
    ErrLog(m_pLoggerClient, L"OclCommandQueue::GetInfo function is not supported");
	return CL_ERR_FAILURE;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code OclCommandQueue::SetProperties(cl_command_queue_properties clProperties, cl_bool bEnable, cl_command_queue_properties* clOldProperties)
{
    ErrLog(m_pLoggerClient, L"OclCommandQueue::SetProperties function is not supported");
	return CL_ERR_FAILURE;	
}

/******************************************************************
 * This function is called when event in the queue change its color.
 * the queue should use this notification to signal the worker thread
 * to continue processing
 ******************************************************************/
cl_err_code OclCommandQueue::NotifyEventColorChange(QueueEvent* pEvent)
{
    // Debug
    if (pEvent->IsColor(QueueEvent::EVENT_STATE_BLACK))
    {
        printf("==== DummyCommand(%3d) is executed on Queue (%3d) ==== \n", pEvent->GetId(), GetId());
    }

    // In case it the queue marked for finish and an event is done, check if device is empty
    // If it is broadcast thread to clean; be careful to delete your self.
    if(m_bCleanFinish && pEvent->IsColor(QueueEvent::EVENT_STATE_BLACK))
    {
        if(m_pCommandQueue->IsEmpty())
        {
            // The queue was requested to finish and it is empty, safe to release it.
            m_pQueueWorkerThread->CancelProcessing();
            m_pCommandQueue->Broadcast();
            //m_pCommandQueue->Release();
            // Delete your self and exit.
            //delete this;
            return CL_SUCCESS;
        }
    }

    // Signal only when there is a command ready.
    if(pEvent->IsColor(QueueEvent::EVENT_STATE_GREEN))
    {
        m_pCommandQueue->Signal();
    }
    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
void OclCommandQueue::EnqueueDevCommands()
{
    ErrLog(m_pLoggerClient, L"OclCommandQueue::EnqueueDevCommands function is not implemented yet");
    return;
}

/******************************************************************
 *
 ******************************************************************/
void OclCommandQueue::PushFrontCommand()
{
    ErrLog(m_pLoggerClient, L"OclCommandQueue::PushFrontCommand function is not implemented yet");
    return;
}


/******************************************************************
 *
 ******************************************************************/
cl_err_code OclCommandQueue::EnqueueCommand(Command* pCommand, cl_bool bBlocking, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pUserEvent)
{
    cl_err_code errVal = CL_SUCCESS;
    // If blocking and no event, than it is needed to create dummy cl_event for wait
    cl_event waitEvent;
    cl_event* pEvent;
    if( bBlocking && NULL == pUserEvent)
    {
        pEvent = &waitEvent;
    }
    else
    {
        pEvent = pUserEvent;
    }
    // creates the command's event
    QueueEvent* pQueueEvent = m_pEventsManager->CreateEvent(pCommand->GetCommandType(), (cl_command_queue)m_iId, pEvent);
    errVal = m_pEventsManager->RegisterEvents(pQueueEvent, uNumEventsInWaitList, cpEeventWaitList);
    if( CL_FAILED(errVal))
    {
        delete pQueueEvent;
        return errVal;
    }
    // Set event and receiver of the command.
    pCommand->SetEvent(pQueueEvent);
    pCommand->SetReceiver(this);
    // Register this queue as a change color observer of the event, to signal the actual queue
    pQueueEvent->RegisterEventColorChangeObserver(this);
    
    errVal = m_pCommandQueue->AddCommand(pCommand);
    if (CL_FAILED(errVal))
    {
        // TODO: clean up events etc...????
        return CL_ERR_FAILURE;
    }

    // If blocking, wait for object
    if(bBlocking)
    {
        m_pEventsManager->WaitForEvents(1,pEvent);
        if(pUserEvent == NULL)
        {
            // The case where er use temp event for blocking
            m_pEventsManager->ReleaseEvent(waitEvent);
        }
    }
    return CL_SUCCESS;
}

/**
 * Set a marker object on the current state of the queue
 */
cl_err_code OclCommandQueue::SetMarker(cl_event pEvent)
{
    return CL_SUCCESS;
}


/******************************************************************
 *
 ******************************************************************/
cl_err_code OclCommandQueue::SetBarrier()
{
    return CL_SUCCESS;
}


/**
 * This functions resolve synch events issues such as a barriar in the queue or In-
 * order queue.
 */
cl_err_code OclCommandQueue::ResolvedSynchEvents(cl_command_type commandType, QueueEvent* newEvent){

	return  CL_SUCCESS;
}
