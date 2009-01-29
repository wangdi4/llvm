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
    cl_command_queue_properties clProperties
    ):
    m_pContext(pContext),
    m_clDefaultDeviceId(clDefaultDeviceID),
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
        m_pCommandQueue->Clear();
        delete m_pCommandQueue;
        m_pCommandQueue = NULL;
    }
    m_pContext = NULL;
    m_pDefaultDevice = NULL;
}
    

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
 * The Clean signals the queue to clean himself peacefully and to release itself.
 * TODO: Implement queue clean function.
 ******************************************************************/
cl_err_code OclCommandQueue::Clean()
{
//    ErrLog(m_pLoggerClient, L"OclCommandQueue::Clean function is not supported");
	return CL_ERR_FAILURE;	
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
 * This function is called when event in the queue is done.
 * the queue should use this notification to signal the worker thread
 * to continue processing
 ******************************************************************/
cl_err_code OclCommandQueue::NotifyEventDone(QueueEvent* event)
{
    ErrLog(m_pLoggerClient, L"OclCommandQueue::NotifyEventDone function is not implemented yet");
    return CL_ERR_FAILURE;
}

/******************************************************************
 *
 ******************************************************************/
void OclCommandQueue::EnqueueDevCommands()
{
	return ;
}

/******************************************************************
 *
 ******************************************************************/
void OclCommandQueue::PushFrontCommand()
{
}


/******************************************************************
 *
 ******************************************************************/
cl_err_code OclCommandQueue::EnqueueCommand(Command* command, cl_bool blocking, const cl_event event_wait_list, cl_uint num_events_in_wait_list, cl_event pEvent)
{
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
