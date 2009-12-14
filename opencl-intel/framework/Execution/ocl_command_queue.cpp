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
#include "in_order_command_queue.h"
#include "out_of_order_command_queue.h"
#include "in_order_command_queue.h"
#include "out_of_order_command_queue.h"
#include "device.h"

// Debug
#include <assert.h>

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
    m_bReleasing(false),
    m_iNotifyChangeCnt(0),
    m_pQueueWorkerThread(NULL),
    m_pCommandQueue(NULL)
{
    m_clContextId = (cl_context)m_pContext->GetId();
    m_pContext->GetDevice(clDefaultDeviceID, &m_pDefaultDevice);    
    // Set queue options
    m_bOutOfOrderEnabled = ((clProperties & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE) ? true : false);
    m_bProfilingEnabled  = ((clProperties & CL_QUEUE_PROFILING_ENABLE) ? true : false );
    // Set logger

	INIT_LOGGER_CLIENT(L"OclCommandQueue Logger Client",LL_DEBUG);

	LOG_INFO(L"OclCommandQueue created: 0x%X", this);

	m_pHandle = new _cl_command_queue;
	m_pHandle->object = this;
}

/******************************************************************
 *
 ******************************************************************/
OclCommandQueue::~OclCommandQueue()
{   
    LOG_INFO(L"OclCommandQueue delete: 0x%X", this);
    ReleaseTheQueue();    
    m_pContext = NULL;
    m_pDefaultDevice = NULL;

    RELEASE_LOGGER_CLIENT;

	if (NULL != m_pHandle)
	{
		delete m_pHandle;
	}
}

/******************************************************************
 *
 ******************************************************************/
void OclCommandQueue::ReleaseTheQueue()
{    
    // Release the queue, it expected to notify the worker thread that processing is done.
    // The worker than will exit peacefully and will delete the queue
    if (NULL != m_pCommandQueue)
    {
        m_pCommandQueue->Release();
    }
    m_pQueueWorkerThread = NULL;
    m_pCommandQueue = NULL;
}

/******************************************************************
 * This function initializes the command,
 * Including: create the right queue and start the worker that should
 * process all commands. 
 ******************************************************************/
cl_err_code OclCommandQueue::Initialize()
{
	LOG_INFO(L"Enter");
    cl_err_code errVal;
    // Create worker thread and run it.
    if ( m_bOutOfOrderEnabled )
    {
		LOG_INFO(L"OOO queue");
        //m_pCommandQueue = new OutOfOrderQueue();
        m_pCommandQueue = new OutOfOrderCommandQueue( m_pEventsManager, m_pDefaultDevice, this );
    }
    else
    {        
		LOG_INFO(L"Inorder queue");
        // m_pCommandQueue = new InOrderQueue();        
        m_pCommandQueue = new InOrderCommandQueue( m_pEventsManager, m_pDefaultDevice, this);        
    }    
    m_pQueueWorkerThread = new QueueWorkerThread;
    errVal = m_pQueueWorkerThread->Init(m_pCommandQueue);
    if( CL_SUCCEEDED(errVal))
    {
		LOG_INFO(L"Starting working thread");
        int startRet = m_pQueueWorkerThread->Start();
        if ( startRet < 0 )
        {
            errVal = CL_ERR_EXECUTION_FAILED;
        }
    }
	LOG_INFO(L"Exit = %d", errVal);
    return errVal;
}

/******************************************************************
 * CleanFinish signals the command queue to finish processing all its 
 * commands, and when the queue is empty to release all its resources.
 * After calling this command it is assume that no user function has access
 * to the queue and therefore no new command will be enqueued.
 * Moreover, if the queue is already empty, it means that all commands in the queue
 * are done, and therefore a call to NotifyEventColorChange is not expected.
 *
 ******************************************************************/
cl_err_code OclCommandQueue::CleanFinish()
{
	LOG_INFO(L"Enter");
    {OclAutoMutex CS(&m_finishLocker);
    m_bCleanFinish = true;
    m_bReleasing = (m_pCommandQueue->IsEmpty() && ( 0 == m_iNotifyChangeCnt) );
	LOG_DEBUG(L"OclCommandQueue CleanFinish called: 0x%X, bEmpty:%d, NtfCmdCnt:%d, bReleasing:%d",
		(unsigned long int)this, m_pCommandQueue->IsEmpty(), m_iNotifyChangeCnt, m_bReleasing);
    }
    //
    // If queue was already empty within the locked time above, delete your self.
    // Else, the delete will happen in the NotifyEventColorChange function. 
    //
    if(m_bReleasing)
    {
        // The queue was requested to finish and it is empty, safe to delete
		LOG_INFO(L"Deleted");
        delete this;
    }
    return CL_SUCCESS;    
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code OclCommandQueue::GetInfo( cl_command_queue_info clParamName, size_t szParamValueSize, void* pParamValue, size_t* pszParamValueSizeRet )
{
    cl_err_code res = CL_SUCCESS;
    void* localParamValue = NULL;
    size_t szOutputValueSize = 0;
    cl_command_queue_properties propreties;
    
    switch (clParamName)
    {
        case CL_QUEUE_CONTEXT:
            localParamValue = &m_clContextId;
            szOutputValueSize = sizeof(cl_command_queue);
            break;
        case CL_QUEUE_DEVICE:
            localParamValue = &m_clDefaultDeviceId;
            szOutputValueSize = sizeof(cl_device_id);
            break;
        case CL_QUEUE_REFERENCE_COUNT:
            localParamValue = &m_uiRefCount;
            szOutputValueSize = sizeof(cl_uint);
            break;
        case CL_QUEUE_PROPERTIES:
            {
            int iOutOfOrder  = (m_bOutOfOrderEnabled) ? 1 : 0;
            int iProfilingEn = (m_bProfilingEnabled)  ? 1 : 0;
            propreties = ((iOutOfOrder) | ( iProfilingEn<<1 ));
            localParamValue = &propreties;
            szOutputValueSize = sizeof(cl_command_queue_properties); 
            break;
            }
        default:
            res = CL_INVALID_VALUE;
            break;
    }

    // check param_value_size
    if ( (NULL != pParamValue) && (szParamValueSize < szOutputValueSize))
    {
        res = CL_INVALID_VALUE;
    }
    else
    {
        if ( NULL != pParamValue )
        {
            memcpy(pParamValue, localParamValue, szOutputValueSize);
        }
        if ( NULL != pszParamValueSizeRet )
        {
            *pszParamValueSizeRet = szOutputValueSize;
        }
    }
    return res;
}

/******************************************************************
 *
 ******************************************************************/
 cl_bool OclCommandQueue::EnableProfiling( cl_bool bEnabled )
 {
     // Profiling is not yet supported!!!
     // always return false
     return CL_FALSE;
 }

/******************************************************************
 *
 ******************************************************************/
cl_bool OclCommandQueue::EnableOutOfOrderExecMode( cl_bool bEnabled )
{
    cl_err_code res = CL_SUCCESS;
    if( bEnabled && !m_bOutOfOrderEnabled ||
        !bEnabled && m_bOutOfOrderEnabled
        )
    {   
        // Change is required

        // To change a queue do as follow:
        // 1. Finish current queue execution
        // 2. Join the worker thread, it will clean the queue
        // 3. Create new queue and new worker
        Finish();

        // Wait for empty, after finish need to verify that the finished command is free too;
        // Use busy wait since operation is quick, better than using more locks.
        while(!m_pCommandQueue->IsEmpty());
        
        ReleaseTheQueue();

        // Clean is done, create new
        if (!m_bOutOfOrderEnabled)
        {
            // from InOrder to OutOfOrder
            m_pCommandQueue = new OutOfOrderCommandQueue( m_pEventsManager, m_pDefaultDevice, this );
            m_bOutOfOrderEnabled = true;
        }
        else
        {
            // Change from OutOfOrder to InOrder
            m_pCommandQueue = new InOrderCommandQueue( m_pEventsManager, m_pDefaultDevice, this);        
            m_bOutOfOrderEnabled = false;
        }
        m_pQueueWorkerThread = new QueueWorkerThread;
        res = m_pQueueWorkerThread->Init(m_pCommandQueue);
        if( CL_SUCCEEDED(res))
        {
            int startRet = m_pQueueWorkerThread->Start();
            if ( startRet < 0 )
            {
                res = CL_ERR_EXECUTION_FAILED;
            }
        }
        if( CL_FAILED(res))
        {
            assert(0 && "SetQueueProperties faild, need to raise exception" );
        }
     }
     // Else do nothing remains the same
     return m_bOutOfOrderEnabled;
 }

/******************************************************************
 *
 ******************************************************************/
 cl_bool OclCommandQueue::IsPropertiesSupported( cl_command_queue_properties clProperties )
 {
    // Get device info
    cl_command_queue_properties clDeviceProperties;
    cl_err_code res = m_pDefaultDevice->GetInfo(CL_DEVICE_QUEUE_PROPERTIES, sizeof(cl_command_queue_properties), &clDeviceProperties, NULL);
    if( CL_SUCCEEDED(res) )
    {
        if( clProperties == (clDeviceProperties & clProperties) )
        {
            // properties are supported
            return true;
        }
    }
    return false;
 }

/******************************************************************
 * This function is called when event in the queue change its color.
 * The OclCommandQueue uses this notification for:
 *  1. To notify the queue, which in turn balance its internal state accordingly.
 *  2. Check if it is needed to be deleted
 *  3. Signal the queue to get next command for processing
 ******************************************************************/
cl_err_code OclCommandQueue::NotifyEventColorChange( const QueueEvent* cpEvent, QueueEventStateColor prevColor,  QueueEventStateColor newColor)
{
    cl_err_code res = CL_SUCCESS;
	int iLclCnt;
    bool bNeedToBeDeleted = false;
	LOG_INFO(L"Enter - this=0x%X, cpEvent=0x%X, prev=%d, new=%d",
		(unsigned long int)this, cpEvent, prevColor, newColor);
    { OclAutoMutex CS(&m_finishLocker);
		// We use iNotifyChangeCnt to count the number of calls to NotifyEventColorChange
		// As long as any other path of the execution is within this function we shouldn't
		// delete it. 
		// This counter enable release of the queue only in one place where the queue is ready
		LOG_INFO(L"Mutex Entry - m_iNotifyChangeCnt = %d", m_iNotifyChangeCnt);
		if (-1 != m_iNotifyChangeCnt)
		{
			m_iNotifyChangeCnt++;
		} else
		{
			LOG_INFO(L"Inc - m_iNotifyChangeCnt == -1");
		}
    }

	res = m_pCommandQueue->NotifyEventColorChange( cpEvent, prevColor, newColor );

	// This function deletes the queue only if it marked to deleted, but
	// did not started to delete itself on CleanFinish since the queue wasn't empty yet.
	//
	bool bQueueEmpty = m_pCommandQueue->IsEmpty();

	if ( bQueueEmpty )
    { OclAutoMutex CS(&m_finishLocker);
		bNeedToBeDeleted = ( m_bCleanFinish && !m_bReleasing && (1 == m_iNotifyChangeCnt));
		if(bNeedToBeDeleted)
		{
			LOG_INFO(L"To be deleted");
			if ( -1 == m_iNotifyChangeCnt )
			{
				LOG_INFO(L"Set - m_iNotifyChangeCnt == -1");
			}
			m_iNotifyChangeCnt = -1; // -1 means that this thread is releasing
		}
    } // End CS

    if( bNeedToBeDeleted)
    {
        // The queue was requested to finish and it is empty, safe to delete.
        // Delete your self and exit.
		LOG_INFO(L"Exit COMMAND_deleted - this=0x%X", (unsigned long int)this);
        delete this;
        return CL_SUCCESS;
    }
    
    // Signal the queue to get next command only if new color is yellow
    if ( EVENT_STATE_YELLOW == newColor )
    {
        m_pCommandQueue->Signal();
    }
    
    { OclAutoMutex CS(&m_finishLocker);
	if (-1 != m_iNotifyChangeCnt)
	{
		m_iNotifyChangeCnt--;
	} else
	{
		LOG_INFO(L"Dec - m_iNotifyChangeCnt == -1");
	}
	iLclCnt = m_iNotifyChangeCnt;
    }

	LOG_INFO(L"Exit - this=0x%X, Cnt:%d, res=%d", (unsigned long int)this, iLclCnt, res);

    return res;
}

/******************************************************************
 *
 ******************************************************************/
// TODO: It looks like irrelevant function, the device could be directly called by a command
cl_err_code OclCommandQueue::EnqueueDevCommands(
    cl_device_id                clDeviceId, 
    cl_dev_cmd_list             clDevCmdListId,
    cl_dev_cmd_desc*            clDevCmdDesc, 
    ICmdStatusChangedObserver** ppCmdStatusChangedObserver, 
    cl_uint                     uiCount 
    )
{
    cl_err_code res;
    // Get device
    Device* pDevice = NULL; 
	//TODO: Don't need that heavy conversion from DeviceId to device ptr
	//		The device ptr shell be kept internally not need in handle.
    res = m_pContext->GetDevice(clDeviceId, &pDevice);
    if ( CL_SUCCEEDED(res) && (NULL != pDevice) )
    {
		// set profiling request
        clDevCmdDesc->profiling = (IsProfilingEnabled() ? true : false );
        res = pDevice->CommandListExecute(clDevCmdListId, clDevCmdDesc, uiCount, ppCmdStatusChangedObserver);
    }
    return res;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code OclCommandQueue::EnqueueCommand(Command* pCommand, cl_bool bBlocking, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pUserEvent)
{
	cl_start;
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
    QueueEvent* pQueueEvent = m_pEventsManager->CreateEvent(pCommand->GetCommandType(), pEvent, this);
    
    // Register this queue as a change color observer of the event, to signal the actual queue
    pQueueEvent->RegisterEventColorChangeObserver(this);

    // Register this event on list of other events
    // If the queue state is InOrder, remove dependencies between events in the same queue
    bool bRemoveEvents = false;
    if( !m_bOutOfOrderEnabled)
    {
        bRemoveEvents = true;
    }

    errVal = m_pEventsManager->RegisterEvents(pQueueEvent, uNumEventsInWaitList, cpEeventWaitList, bRemoveEvents, (cl_command_queue)m_iId);
    if( CL_FAILED(errVal))
    {
        delete pQueueEvent;
        return errVal;
    }   

    // Set event and receiver of the command.
    pCommand->SetEvent(pQueueEvent);
    pCommand->SetReceiver(this);
    pCommand->SetCommandDeviceId(m_clDefaultDeviceId);
   
    errVal = m_pCommandQueue->AddCommand(pCommand);
    if (CL_FAILED(errVal))
    {
        // No need to delete event, already attached to the command and is free by command.
        return CL_ERR_FAILURE;
    }

    // If blocking, wait for object
    if(bBlocking)
    {
        // Make unblocking flush, the block is on the command itself.
        m_pCommandQueue->Flush(false);
        m_pEventsManager->WaitForEvents(1,pEvent);
        if(pUserEvent == NULL)
        {
            // The case where it use temp event for blocking
            m_pEventsManager->ReleaseEvent(waitEvent);
        }
    }
    cl_return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code OclCommandQueue::Flush()
{
    // Flush, no blocking
    return m_pCommandQueue->Flush(false);
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code OclCommandQueue::Finish()
{
    // blocking flush
    return m_pCommandQueue->Flush(true);
}
