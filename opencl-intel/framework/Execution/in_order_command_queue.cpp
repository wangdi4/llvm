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
//  in_order_command_queue.cpp
//  Implementation of the Class InOrderQueue
//  Created on:      23-Dec-2008 3:23:02 PM
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////
#include "in_order_command_queue.h"
#include "enqueue_commands.h"
#include "queue_event.h"
#include "device.h"
#include "events_manager.h"
#include "ocl_command_queue.h"
#include "context.h"

#include <assert.h>

using namespace Intel::OpenCL::Framework;

/******************************************************************
 * Constructor.
 * Creates the private objects
 ******************************************************************/
InOrderCommandQueue::InOrderCommandQueue( EventsManager* pEventsManager, Device* pDevice, OclCommandQueue* pOclCommandQueue ):
    m_clCurrentDevCmdList(0),
    m_pDevice(pDevice),
    m_pEventsManager(pEventsManager),
    m_pOclCommandQueue(pOclCommandQueue),
    m_pColorChangeObserver(pOclCommandQueue),
    m_bReleased(false),
    m_bIsPrevCmdFlush(false),
    m_uiFlushCnt(0),
    m_uiCmdListCnt(0),
	m_pContext(NULL)
{
    //  Create device list
    m_pDevice->CreateCommandList(CL_DEV_LIST_NONE, &m_clCurrentDevCmdList); // If failed, list still 0

	// Add dependency to context
	m_pContext = (Context*)m_pOclCommandQueue->GetContextId()->object;
	m_pContext->AddPendency();

    // Set logger
	INIT_LOGGER_CLIENT(L"ICommandQueue Logger Client",LL_DEBUG);

    LOG_INFO(L"ICommandQueue created: 0x%X", (unsigned long int)this);
}

/******************************************************************
 * Assumes release was already called
 ******************************************************************/
InOrderCommandQueue::~InOrderCommandQueue()
{
    // Close list if wasn't closed.
    // Never close it since the queue may be released after the device was closed.
    if ( 0 != m_clCurrentDevCmdList  )
    {
        m_pDevice->ReleaseCommandList(m_clCurrentDevCmdList);
        m_clCurrentDevCmdList = 0;
    }

	m_pContext->RemovePendency();

    LOG_INFO(L"InOrderCommandQueue deleted: 0x%X", (unsigned long int)this);    
    LOG_INFO(L"number of commands: Wait: %d, Ready: %d, Device: %d, Runtime: %d, Black: %d", (int)m_waitingCmdsList.size(), (int)m_readyCmdsList.size(), (int)m_deviceCmdsList.size(), (int)m_runtimeCmdsList.size(), (int)m_blackCmdsList.size());
    
	RELEASE_LOGGER_CLIENT;
}

/******************************************************************
 * This function returns the first command that is ready for processing
 * by device, to say the first command from the front that is green.
 * If the command is an dev_executed, the command is assigned to the device list
 * else it will be assigned to the runtime list. In both cases it is the command's Execute
 * function responsibility to update the right state properly, yet the queue
 * must put the commands in the right locations in order to keep the In-Order policy.
 * If there is no ready command, the thread is blocked on this command.
 *
 * This function is used also for garbage collection of done commands.
 * if a command is black and it is in the black list, it will be removed and deleted.
 *
 ******************************************************************/
Command* InOrderCommandQueue::GetNextCommand()
{
    Command* pNextCommand = NULL;
    COND_RESULT res = COND_RESULT_OK;
    
    // Garbage collection before locking the list to prevent calls to events blocking functions within
    // the list lock
    CleanBlackList();
    { OclAutoMutex CS(&m_listsLocker); // Start CS

    // This method loop on the ready list.
    while ( 
        m_readyCmdsList.empty()                 && 
        res != COND_RESULT_COND_BROADCASTED     &&  // Broadcast is used to terminate execution of the queue
        ! m_bReleased                               // If already released don't get into 
        )         
    {
        res = m_cond.Wait(&m_listsLocker);
    } 
    if(res!=COND_RESULT_COND_BROADCASTED && ! m_bReleased)
    { 
        assert(!m_readyCmdsList.empty() && "ready list is empty");
        // Found a command, pop it out and move it its next device
        pNextCommand = m_readyCmdsList.front();
        m_readyCmdsList.pop_front();
        
        if ( DEVICE_EXECUTION_TYPE == pNextCommand->GetExecutionType() )
        {
            // Will be executed on the device
            m_deviceCmdsList.push_back(pNextCommand);
        }
        else
        {
            // Send to the runtime list
			LOG_DEBUG(L"Added to runtime list:0x%X", pNextCommand);
            m_runtimeCmdsList.push_back(pNextCommand);
        }
    }

    } // End CS

    // Garbage collection also here
    CleanBlackList();

    return pNextCommand;
}

/******************************************************************
 * Use commands lists of the parent. No dependencies in previous commands
 *
 * A new command state may be red or green.
 * If a new command is red (dependent) it is automatically enter into 
 * the waiting commands. Once the waiting commands is not empty all commands (red & green)
 * are pushed into the waiting list. The list are processed in-order. 
 * If a red command turned to be green command, the red one and all the green once in order above it 
 * move to the ready list. See NotifyColorChange for more information
 * 
 * At the end, if the ready list is not empty, the function signals the worker thread
 * to get next command.
 ******************************************************************/
cl_err_code InOrderCommandQueue::AddCommand(Command* pCommand)
{
	cl_start;
    QueueEvent* pEvent = pCommand->GetEvent();

    // Set the command list
    // If no device list is open, open a new one.
    
    LogDebugA("Command - ADD: %s (Id: %d)", pCommand->GetCommandName(), pCommand->GetId());

	bool bIsListEmpty = true;

    { OclAutoMutex CS(&m_listsLocker); // Start critical section
    // In this point the list is locked and therefore we can be sure that
    // the command state (color) will not change, since the change of color
    // will be blocked on NotifyColorChange.

	// TODO: Why we need this in in-order queue
    // Support OutOfOrder commands
    HandleOutOfOrderCommands(pCommand);
    pCommand->SetDevCmdListId(m_clCurrentDevCmdList);

	// TODO: Why we need this in in order queue, it's dependent by the definition
    if( CL_COMMAND_FINISH == pCommand->GetCommandType() )
    {
        // Finish is done when all are done
        SetDependentOnAll(pCommand->GetEvent());
    }

    // Debug - only green and red commands
    assert( pEvent->IsColorNotBlock(EVENT_STATE_RED) || pEvent->IsColorNotBlock(EVENT_STATE_YELLOW) );

	// Red command always pushed to the waiting list. 
	// Once the list is not empty all commands are pushed to it to keep In-Order
    if ( pEvent->IsColorNotBlock(EVENT_STATE_RED) || !(m_waitingCmdsList.empty()))
    {
        m_waitingCmdsList.push_back(pCommand);
    }
    else 
    {
		// TODO: Consider to send commands for execution from here,
		//		 no need in ready queue
        // ready for process
        if ( DEVICE_EXECUTION_TYPE == pCommand->GetExecutionType())
        {
			// TODO: No decrements for m_uiCmdListCnt
            m_uiCmdListCnt++;
        }
        m_readyCmdsList.push_back(pCommand);
    }

	bIsListEmpty =  m_readyCmdsList.empty();

    } // End of critical section

    // Signal in case there are commands in the ready list
    if( !bIsListEmpty )
    {
        Signal();
    }

    cl_return CL_SUCCESS;
}

/******************************************************************
 * This function set pEvent as dependent on all not completed
 * events in the queue.
 * If this function is called within a list locker context, the state
 * of the queue is stable. Do not call it outside of a critical section.
 * The function iterates all command from the top of the list until encounter
 * the previous Marker/Barrier/Flush command that were done the same
 * rutine to those who are below them in the queue.
 * 
 ******************************************************************/
void InOrderCommandQueue::SetDependentOnAll( QueueEvent* pEvent)
{
    // Stops at the first list that include the previous Marker/Barrier/Flush
    // or complete all if none
    if (SetDependentOnAllCommandsInList(pEvent, m_waitingCmdsList))
    {
        if(SetDependentOnAllCommandsInList(pEvent, m_readyCmdsList))
        {
            if(SetDependentOnAllCommandsInList(pEvent, m_deviceCmdsList))
            {
                // runtime list may include Marker/Barrier/Flush that alredy are done,
                // but were not executed. We register here only for consistency.
                SetDependentOnAllCommandsInList(pEvent, m_runtimeCmdsList);
            } 
        }
    }
}

/******************************************************************
 * Use this function to set pEvent to be dependent on all commands
 * in CmdList. 
 * If one of the commands is Marker/Barrier than the function assumed
 * this command has already run the function and therefore it is dependent for now on.
 * In this case false is returned.
 * true returned if pEvent registered on all ocmmands in the list.
 *******************************************************************/
bool InOrderCommandQueue::SetDependentOnAllCommandsInList( QueueEvent* pEvent, list<Command*>& cmdList )
{
    // Go from the back
    list<Command*>::reverse_iterator riter;
    Command* pCommand = NULL;
    for ( riter = cmdList.rbegin(); riter != cmdList.rend(); riter++) 
    {
        cl_command_type clPrevCommandType   = (*riter)->GetCommandType();
        QueueEvent*     pPrevEvent =          (*riter)->GetEvent();
        
        pEvent->SetDependentOn(pPrevEvent);
        
        if ( CL_COMMAND_MARKER  == clPrevCommandType ||
             CL_COMMAND_BARRIER == clPrevCommandType )
        {
            // Done
            return false;
        }
    }
    return true;
}

/******************************************************************
 * Set pEvent to be dependent on the last device command that was 
 * enqueued and not yet finished
 ******************************************************************/
bool InOrderCommandQueue::SetDependentLast( QueueEvent* pEvent, list<Command*>& cmdList )
{
    // Iterate from the back
    list<Command*>::reverse_iterator riter;
    Command* pCommand = NULL;
    for ( riter = cmdList.rbegin(); riter != cmdList.rend(); riter++) 
    {
        if( DEVICE_EXECUTION_TYPE == (*riter)->GetExecutionType())
        {
            pEvent->SetDependentOn((*riter)->GetEvent());
            return true;
        }
    }
    return false;
}

/******************************************************************
 * In this functino, the InOrder queue applys its policy for
 * OutOfOrder commands handling.
 * 
 ******************************************************************/
void InOrderCommandQueue::HandleOutOfOrderCommands(Command* pCommand)
{
    QueueEvent* pEvent = pCommand->GetEvent();

    if ( CL_COMMAND_FLUSH == pCommand->GetCommandType() )
    {
        m_bIsPrevCmdFlush = true;
    }
    else if (m_bIsPrevCmdFlush)
    {
        m_bIsPrevCmdFlush = false;
        // Set dependent on the last device command which is the last command in the list for the device
        // Look for command in this order: waiting --> ready --> In device
        if( !(SetDependentLast(pEvent, m_waitingCmdsList)))
            if( ! ( SetDependentLast(pEvent, m_readyCmdsList)))
                        SetDependentLast(pEvent, m_deviceCmdsList);
    }

    // For Barrier and WaitForEvents do nothing. For commands inside a queue
    // the In-Order policy solves the dependencies. For WaitOnEvents in other
    // queues, the EnqueueCommands in the OclCommandQueue already solved the issue
    // So only marker is supported.
    if( CL_COMMAND_MARKER != pCommand->GetCommandType() )
    {
        return;
    }

    // Marker is only register on device commands in the queue that are not finished yet.
    // Since it is InOrder we can register on the last one only.
    bool bIsDependent = false;
    // Look for command in this order: waiting --> ready --> In device
    if( !(bIsDependent = SetDependentLast(pEvent, m_waitingCmdsList)))
        if( ! (bIsDependent = SetDependentLast(pEvent, m_readyCmdsList)))
            bIsDependent = SetDependentLast(pEvent, m_deviceCmdsList);
    // If is dependent add flush before to resolve dependencies.
    if (bIsDependent)
    {
        Flush(false);
    }
}

/******************************************************************
 * This function is called by an event when ever its state changes.
 * The actual policy of the queue is implemented in this function together
 * with the AddCommand and the GetNextCommand function.
 * When starting, the function lock the lists. As a result, each call to 
 * AddCommand and GetNextCommand is done on stable lits.
 *
 * The following changes are expected:
 *      Red->Green:
 *          If command it is in the front of waiting list, it can move from waiting list to ready list.
 *          As a result, In addition, all green commands in order until the next red command are ready to move too
 *          Else nothing to do (in-order dependency still exists)
 *      Green->Lime:
 *          A command is issued on the device - Nothing to do, GetNext already put it in the device list
 *      Lime->Gray
 *          State on the device changes - nothing to do, still in the device list
 *      All (ex Red) ->Black
 *          Need to find the command in the lists according to its previous state (only an hint)
 *          Remove it and move it to the black list. Never delete the command itself since this function
 *          is called within the command's event. The black list garbage collector will do so later
 * 
 * Note: Event is CONST. Any call to cpEvent methods that are not const may result deadlock.
 ******************************************************************/
cl_err_code InOrderCommandQueue::NotifyEventColorChange( 
            const QueueEvent*       cpEvent, 
            QueueEventStateColor    prevColor,  
            QueueEventStateColor    newColor )
{
    cl_err_code res = CL_SUCCESS;
    Command* pCommand = NULL; 

    bool            bIsFlushed = false;

	LOG_DEBUG(L"Enter - Event 0x%X, PrevColor: %d, newColor:%d", cpEvent, prevColor, newColor);

    //
    // CASE 1: Was Red command
    //
    if ( EVENT_STATE_RED == prevColor )
    {
		LOG_DEBUG(L"Prev color RED");
        // From now on, access is serial to keep the state of the lists valid
        OclAutoMutex CS(&m_listsLocker); // Start Critical Section
        // Just popped out the front of the waiting list until red
        // If the cpEvent is not at the top of the list, probably a red command is there
        // and the loop will end immediately
		LOG_DEBUG(L"Iterating waiting commands, search for YELLOW commands");
        while(!m_waitingCmdsList.empty())
        {
            pCommand = m_waitingCmdsList.front();
            if ( pCommand->GetEvent()->IsColorNotBlock(EVENT_STATE_YELLOW) )
            {
                // Move to ready list
                m_waitingCmdsList.pop_front();
                if ( DEVICE_EXECUTION_TYPE == pCommand->GetExecutionType())
                {
                   // Set device list for device commands
                   m_uiCmdListCnt++;
                }
                m_readyCmdsList.push_back(pCommand);
				// TODO: Why not to put internal flush only at the end of waiting list
                PushInternalFlush();
            }
            else
            {
                break;
            }
        }
		LOG_DEBUG(L"Exit");
        return res;
    }

    if ( EVENT_STATE_BLACK == newColor )
    {
		LOG_DEBUG(L"New color BLACK");

        { OclAutoMutex CS(&m_listsLocker); // Start Critical Section

        // 1st: check in device list, the most likely option
        pCommand = RemoveFromCommandList( cpEvent, m_deviceCmdsList );
        if ( NULL == pCommand )
        {
            // 2nd: runtime list, another good option
            pCommand = RemoveFromCommandList( cpEvent, m_runtimeCmdsList );
            if (NULL == pCommand)
            {
                // Getting here is an error.
                assert(0 && "Command changed to black from waiting/ready list is not expected" );
            }
			LOG_DEBUG(L"Command 0x%X was removed from run-time list", pCommand);
            if( CL_COMMAND_INTERNAL_FLUSH == pCommand->GetCommandType())
            {
                bIsFlushed = true;
            }
        }
        if ( NULL != pCommand )
        {
            // add to black list
            OclAutoMutex CS(&m_blackCmdsListLocker);
            m_blackCmdsList.push_back(pCommand);
        }
        } // End CS
        if ( bIsFlushed )
        {
            // Flush done out of locks to prevent flush to block
            res = FlushDone( m_clCurrentDevCmdList );
        }
		LOG_DEBUG(L"Exit");
        return res;
    }

	LOG_DEBUG(L"Exit");
    // All other options - do nothing 
    return res;
}
    
/******************************************************************
* Push an internal flush into the ready command list.
* When this command will be executed, the device list will be flushed
* 
******************************************************************/
void InOrderCommandQueue::PushInternalFlush()
{
    // An internal flush is required if m_uiFlushCnt > 0
    // However we can also check whether they are device command to be flushed
    if ( m_uiFlushCnt > 0 && IsReadyDeviceCommands() )
    {
        Command* pCommand = new InternalFlushCommand();
        pCommand->Init();

        QueueEvent* pQueueEvent = m_pEventsManager->CreateEvent(pCommand->GetCommandType(), NULL, m_pOclCommandQueue);
        // We can call an event call although the queue is locked since the event is still local and no other thread 
        // knows about its existence.
        pQueueEvent->RegisterEventColorChangeObserver( m_pColorChangeObserver );
        pCommand->SetEvent(pQueueEvent);

        pCommand->SetDevCmdListId(m_clCurrentDevCmdList);
        // Clean the list count
        m_uiCmdListCnt = 0;

		// Evgeny 13/09/09
		m_uiFlushCnt = 0;

        // Add command to the ready list
        LogInfoA("Command - ADD: %s (Id: %d)", pCommand->GetCommandName(), pCommand->GetId());

        m_readyCmdsList.push_back(pCommand);
    }
}


/******************************************************************
 * In flush, the queue creates a flush command.
 * The command is handled as any other commands. What mean is that it is handled in-order.
 * After a flush command is added, a new device list is created and all new commands are
 * assigned to the attached list. Note that commands are going to be issued into this list
 * according to the in-order policy and therefore it will happened only after the FlushCommand
 * is done.
 * 
 * If bBlocking true, this function returned only when all command
 * were completed on the relevant device - this is a finish behavior and a finish command is attached.
 *
 ******************************************************************/
cl_err_code InOrderCommandQueue::Flush( bool bBlocking )
{
	cl_start;
    cl_err_code res = CL_SUCCESS;
    QueueEvent* pQueueEvent = NULL;

    Command*    pFlushCommand = new FlushCommand();
    pQueueEvent = m_pEventsManager->CreateEvent(pFlushCommand->GetCommandType(), NULL, m_pOclCommandQueue);
    pQueueEvent->RegisterEventColorChangeObserver( m_pColorChangeObserver );

    pFlushCommand->SetEvent(pQueueEvent);
    pFlushCommand->Init();

    //
    // Add the command to the list
    //
    { OclAutoMutex CS (&m_listsLocker); // Start of critical section
    AddCommand(pFlushCommand);

    m_uiFlushCnt++;
    PushInternalFlush();
    } // End of CS

    //
    // If blocking, call finish,
    //
    if ( bBlocking )
    {
        res = Finish();
    }
    cl_return res;
}


/******************************************************************
 * Finish enqueues a runtime command that is done only after the last enqueued
 * device's command is done and wait for this runtime command to complete.
 * Since this is an in order queue, finish actually returned when all command are completed.
 *
 ******************************************************************/
cl_err_code InOrderCommandQueue::Finish()
{
    cl_err_code res = CL_SUCCESS;
    QueueEvent* pQueueEvent = NULL;

    // 
    cl_event    clEvent;
    Command*    pFinishCommand = new FinishCommand();
    // use event manager mechanism to wait on. creates the command's event
    pQueueEvent = m_pEventsManager->CreateEvent(pFinishCommand->GetCommandType(), &clEvent, m_pOclCommandQueue);
    pQueueEvent->RegisterEventColorChangeObserver( m_pColorChangeObserver );

    pFinishCommand->SetEvent(pQueueEvent);
    pFinishCommand->Init();

    //
    // Add the command to the list
    //
    res = AddCommand(pFinishCommand);

    // The AddCommand already set the current list for flush command, this list is used on done,
    // so time to set it to 0, and next AddCommand will open new one

    // Wait for finish
    if(CL_SUCCEEDED(res))
    {
        res = m_pEventsManager->WaitForEvents(1,&clEvent);
        m_pEventsManager->ReleaseEvent(clEvent);
    }
    return res;
}


/******************************************************************
 * This function is called by the flush command when it is executed.
 * By releasing the device list, the device is expected to flush all 
 * commands in the list.
 ******************************************************************/
cl_err_code InOrderCommandQueue::FlushDone(void* clDevCmdList)
{
	LOG_INFO(L"Enter - List=%X", clDevCmdList);
	int iRes = CL_SUCCESS;
    if( 0 != clDevCmdList)
    {
#ifdef _DEBUG
		{ OclAutoMutex CS(&m_listsLocker); // Start Critical Section
        list<Command*>::iterator iter;
        for ( iter = m_deviceCmdsList.begin(); iter != m_deviceCmdsList.end(); iter++) 
        {
            (*iter)->m_bIsFlushed = true;
        }
		}
#endif
		// Flush the list
        iRes = m_pDevice->FlushCommandList(clDevCmdList);       
    }
	LOG_INFO(L"Exit - List=%X, Res=%X", clDevCmdList, iRes);
    return iRes;
}

/******************************************************************
 *
 ******************************************************************/
void InOrderCommandQueue::Signal()
{
    m_cond.Signal();
}

/******************************************************************
 * Size of the queue is all commands in the queue excluding done commands
 * 
 ******************************************************************/
bool InOrderCommandQueue::IsEmpty()
{
    OclAutoMutex CS(&m_listsLocker);
    bool  bEmpty = (m_waitingCmdsList.empty() && m_readyCmdsList.empty() && m_deviceCmdsList.empty() && m_runtimeCmdsList.empty() );
    
    return bEmpty;
}

/******************************************************************
 *
 ******************************************************************/
cl_uint InOrderCommandQueue::Size()
{
    OclAutoMutex CS(&m_listsLocker);
    cl_uint uiSize =  m_waitingCmdsList.size() + m_readyCmdsList.size() + m_deviceCmdsList.size() + m_runtimeCmdsList.size();
    return uiSize;
}

/******************************************************************
 * Release all resources immediately.
 * If command is exist, it is lost forever... as well as it event
 * Note that if the device's command list is not empty in advanced, a call for 
 * NotifyEventColorChange may crushes execution.
 * Anyhow, Once entering into NotifyEventColorChange the state is safe since b_isReleasing is set.
 * 
 * When done, broadcast the condition. This is the only broadcast exists so the
 * user can count that broadcast means release.
 ******************************************************************/
cl_err_code InOrderCommandQueue::Release()
{
    // Time to broadcasting, clear the list
    // and broadcast all waiters to stop

    // Assert, this function should
    assert( IsEmpty() && "Try to release a non empty queue - not stable behaviour");

    // To prevent memory leaks only need to clear black list
    CleanBlackList();
    
    { OclAutoMutex CS(&m_listsLocker);
    // Mark to release
    m_bReleased = true;
    }

    //// Close list if wasn't closed.
    //// Never close it since the queue may be released after the device was closed.
    //if ( 0 != m_clCurrentDevCmdList  )
    //{
    //    m_pDevice->ReleaseCommandList(m_clCurrentDevCmdList);
    //    m_clCurrentDevCmdList = 0;
    //}


    // Signal the working thread to quit.
    m_cond.Broadcast();
    return CL_SUCCESS;
}

/******************************************************************
 * CleanBlackList: Private method
 * Garbage collection of all done commands.
 *
 ******************************************************************/
bool InOrderCommandQueue::CleanBlackList()
{
    // Clean black list
    // Other threads may add commands to the list, but only this function remove from the list.
    // We synch this loop since the stl::list is not a safe class. It may be that empty() return false
    // when other thread enters an object to the list, but on the front() command the actual object is not in
    // the list yet.
    m_blackCmdsListLocker.Lock();
    while(  !m_blackCmdsList.empty() )
    {        
        Command* pCommand = m_blackCmdsList.front();
        m_blackCmdsList.pop_front();
        m_blackCmdsListLocker.Unlock();

        delete (pCommand);
        m_blackCmdsListLocker.Lock();
    }
    m_blackCmdsListLocker.Unlock();
    return true;
}

/******************************************************************
 * ClearCommandList: A private inlined method
 * Clear a list of commands and delete all commands
 * This function change the state of an event. Therefore never call
 * this command within a list locked function, since it may result deadlock.
 *
 ******************************************************************/
inline void InOrderCommandQueue::ClearCommandList( list<Command*>& cmdList )
{
    while(!cmdList.empty())
    {
        Command* pCommand = cmdList.front();
        // Set event state black to virify that commands in other queues remain stable
        cmdList.pop_front();
        // This call deletes also the event and notify depends
        delete pCommand;
    }
}

/******************************************************************
 * RemoveFromCommandList: A private inlined method
 * Finds a command according to event Id in a list.
 * If found, erases the command from the list and returns a pointer to the command
 * If command not found, a NULL pointer is returned.
 *
 ******************************************************************/
inline  Command* InOrderCommandQueue::RemoveFromCommandList( const QueueEvent* cpEvent, list<Command*>& cmdList )
{
	// TODO: implement this function in a faster way,
	// Add another map or hash-map to improve
    list<Command*>::iterator iter;
    Command* pCommand = NULL;
    for ( iter = cmdList.begin(); iter != cmdList.end(); iter++) 
    {
        QueueEvent* pCurrentEvent = (*iter)->GetEvent();
        if ( cpEvent->GetId() == pCurrentEvent->GetId())
        {
            // Found
            pCommand = *iter;
            cmdList.erase(iter);
            break;            
        }
    }
    return pCommand;
}

/******************************************************************
 * Returns true if there are device commands that are ready and need
 * internal flush.
 *
 ******************************************************************/
bool InOrderCommandQueue::IsReadyDeviceCommands()
{
    return (m_uiCmdListCnt > 0 );
}
