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
//  out_of_order_queue.cpp
//  Implementation of the Class OutOfOrderQueue
//  Created on:      25-Mar-2009
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////
#include "out_of_order_command_queue.h"
#include "queue_event.h"
#include "enqueue_commands.h"
#include "device.h"
#include "events_manager.h"
#include <assert.h>

using namespace Intel::OpenCL::Framework;

/******************************************************************
 *
 ******************************************************************/
OutOfOrderCommandQueue::OutOfOrderCommandQueue( EventsManager* pEventsManager, Device* pDevice, OclCommandQueue* pOclCommandQueue ):
    InOrderCommandQueue(pEventsManager, pDevice, pOclCommandQueue),
    m_pSynchBarrierEvent(NULL)
{
}

/******************************************************************
 *
 ******************************************************************/
OutOfOrderCommandQueue::~OutOfOrderCommandQueue()
{
}

/******************************************************************
 * Add command:
 *
 * New command is either green or red. However if a previous barrier
 * or WaitOnEvent was issued, the command must register on this event.
 *
 * The following invariants are enforced to sync events:
 *      1. Barrier and Marker must register on all previous commands until the previous Barrier command.
 *      2. If Barrier or WaitOnEvents were queued, all commands need to register on this command.
 *         Excluding the Marker & Barrier that are dependent on all (section 1 above)
 *      3. WaitOnEvents will always be registered on the previous WaitOnEvents/Barrier.
 *         Therefore, new commands can register in the last Barrier/WaitOnEvents and keep previous ones behavior.
 *
 * To verify that commands are executed before an internal sync object (Barrier/WaitOnEvents),
 * The implementation inserts a flush command in front of each of those commands.
 ******************************************************************/
cl_err_code OutOfOrderCommandQueue::AddCommand( Command* pCommand )
{
    QueueEvent*     pEvent          = pCommand->GetEvent();
    cl_command_type clCommandType   = pCommand->GetCommandType();

    LogDebugA("Command - ADD   : %s (Id: %d)", pCommand->GetCommandName(), pCommand->GetId());

    { OclAutoMutex CS(&m_listsLocker); // Start critical section
    // In this point the list is locked and therefore we can be sure that
    // the command state (color) will not be changed by other thread, since the 
    // change of color will be blocked on NotifyColorChange.

    // Debug - only green and red commands
    assert( pEvent->IsColorNotBlock(EVENT_STATE_RED) || pEvent->IsColorNotBlock(EVENT_STATE_YELLOW) );

    // Phase 1: Resolve internal dependencies as result of Marker/Barrier/WaitOnEvents Commands
    ApplyOutOfOrderCommandsPolicy(pCommand);

    // Phase 2: Enter into the queue
    // Red commands to the waiting list, green to the ready list
    pCommand->SetDevCmdListId(m_clCurrentDevCmdList);
    if ( pEvent->IsColorNotBlock(EVENT_STATE_RED) )
    {
        m_waitingCmdsList.push_back(pCommand);
    }
    else 
    {
        // ready for process
        if ( DEVICE_EXECUTION_TYPE == pCommand->GetExecutionType())
        {
            m_uiCmdListCnt++;
        }
        m_readyCmdsList.push_back(pCommand);
    }

    } // End of critical section

    // Signal in case there are commands in the ready list
    if( ! m_readyCmdsList.empty() )
    {
        Signal();
    }

    return CL_SUCCESS;
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
 *          A new green command moves from the waiting list to the ready list.
 *          It will then be available for the GetNextCommand.
 *      Green->Lime:
 *          A command is issued on the device - Nothing to do, GetNext already put it in the device list
 *      Lime->Gray
 *          State on the device changes - nothing to do, still in the device list
 *      All (ex Red) ->Black
 *          Need to find the command in the lists according to its previous state (only an hint)
 *          Remove it and move it to the black list. Never delete the command itself since this function
 *          is called within the command's event. The black list garbage collector will do so later
 *          In case of runtime commands (Marker/Barrier/WaitOnEvents/Flush) handle the queue synch events
 * 
 * Note: Event is CONST. Any call to cpEvent methods that are not const may result deadlock.
 ******************************************************************/
cl_err_code OutOfOrderCommandQueue::NotifyEventColorChange( 
            const QueueEvent*       cpEvent, 
            QueueEventStateColor    prevColor,  
            QueueEventStateColor    newColor )
{
    cl_err_code res = CL_SUCCESS;

    Command* pCommand = NULL; 

    bool            bIsFlushed = false;

    //
    // CASE 1: Was Red command
    //
    if ( EVENT_STATE_RED == prevColor )
    {
        // Start Critical Section, lock lists only when they are about to be changed.
        OclAutoMutex CS(&m_listsLocker); 
        // Red must change to green
        // just remove from the waiting list and insert at the back of the ready list
        pCommand = RemoveFromCommandList( cpEvent, m_waitingCmdsList );
        // pCommand must exists and new color must be green in order to move it to the ready list
        if ( NULL != pCommand && EVENT_STATE_YELLOW == newColor )
        {
            if ( DEVICE_EXECUTION_TYPE == pCommand->GetExecutionType())
            {
                // Set device list for device commands
                m_uiCmdListCnt++;
            }
            m_readyCmdsList.push_back(pCommand);
            // TODO: Make internal flush more optimized
            // Currently, to resolve dependencies between queues, we put internal flush after each device ready command
            PushInternalFlush();
        }
    }
    //
    // Case 2: newColor is black, command is done
    //
    else if ( EVENT_STATE_BLACK == newColor )
    {
        // Start Critical Section, lock lists only when they are about to be changed.        
        {
        OclAutoMutex CS(&m_listsLocker); 
        // 1st: check in device list, the most likely option
        pCommand = RemoveFromCommandList( cpEvent, m_deviceCmdsList );
        if ( NULL == pCommand )
        {
            // 2nd: runtime list, Popped it out and handle OutOfOrder special commands.
            pCommand = RemoveFromCommandList( cpEvent, m_runtimeCmdsList );
            assert( (NULL != pCommand) && "Command changed to black from waiting/ready list is not expected" );
            HandleRuntimeCommandExecuted(pCommand); 
            if( CL_COMMAND_INTERNAL_FLUSH == pCommand->GetCommandType())
            {
                bIsFlushed = true;
            }
        }
        if ( NULL != pCommand )
        {
            // Before clean up, if a command turned black, it may resolve dependencies and 
            // ready list may not be empty. Good place to try and enter a flush
            // Currently no need to do so since we push any green command, see above.
            // TODO: Solve internal flush issues
            // PushInternalFlush();
            // add to black list
            OclAutoMutex CS(&m_blackCmdsListLocker);
            m_blackCmdsList.push_back(pCommand);
        }
        } // end CS
        if( bIsFlushed)
        {
            // Flush out side of any locks
            res = FlushDone(m_clCurrentDevCmdList);        
        }
    }
    // All other options - do nothing 
    return res;
}

/******************************************************************
 * If bBlocking true, this function returned only when all command
 * were completed on the relevant device.
 * User can use this function for finish in addition to the flush method
 * 
 * When a flush is called
 *  - A flush command is enters to the queue.
 *  - The queue is marked to be flushed as soon as possible. identified by m_uiFlushCnt > 0 
 *  - The flush command is dependent on all previous commands. When they all done, the flush is done
 *  - An immediate internal flush is enter to issue all currently available commands.
 *
 ******************************************************************/
cl_err_code OutOfOrderCommandQueue::Flush( bool bBlocking )
{
    cl_err_code res = CL_SUCCESS;
    cl_event    clEvent;
    QueueEvent* pQueueEvent = NULL;
    
    Command* pFlushCommand = new FlushCommand();
    pFlushCommand->Init();
    // If blocking (finish), use event manager mechanism to wait on.
    // creates the command's event
    if (bBlocking)
    {
        pQueueEvent = m_pEventsManager->CreateEvent(pFlushCommand->GetCommandType(), &clEvent, m_pOclCommandQueue);
    }
    else
    {
        pQueueEvent = m_pEventsManager->CreateEvent(pFlushCommand->GetCommandType(), NULL, m_pOclCommandQueue);
    }
    pQueueEvent->RegisterEventColorChangeObserver( m_pColorChangeObserver );
    pFlushCommand->SetEvent(pQueueEvent);

    // Start work on the lists
    { OclAutoMutex CS (&m_listsLocker); // Start of critical section
    // Add the flush command    
    AddCommand(pFlushCommand);
    m_uiFlushCnt++;
    PushInternalFlush();
    } // End of CS

    // Signal because PushInternal may means to flush now all ready commands.
    if( ! m_readyCmdsList.empty() )
    {
        Signal();
    }


    //
    // If blocking, wait on event to done,
    //
    if ( bBlocking )
    {
        m_pEventsManager->WaitForEvents(1,&clEvent);
        m_pEventsManager->ReleaseEvent(clEvent);
    }
    return res;
}

/******************************************************************
 * This function apply the OutOfOrder policy as follows:
 *  1.  When command type is Marker or Barrier, apply dependency on all previous commands
 *      until the previous Barrier/Marker
 *  2.  When m_pSynchBarrierEvent is not NULL (some previous Barrier/WaitOnEvnents issues),
 *      Set dependency on.
 *  3.  If command is Barrier/WaitOnEvnents
 *      a. Insert flush command
 *      b. Set it as m_pSynchBarrierEvebt
 * 
 ******************************************************************/
void OutOfOrderCommandQueue::ApplyOutOfOrderCommandsPolicy( Command* pCommand )
{
    QueueEvent*     pEvent          = pCommand->GetEvent();
    cl_command_type clCommandType   = pCommand->GetCommandType();
 
    if ( CL_COMMAND_MARKER  == clCommandType || 
         CL_COMMAND_BARRIER == clCommandType ||
         CL_COMMAND_FLUSH   == clCommandType
        )
    {
        SetDependentOnAll(pEvent);
    }
    else
    {
        // Register on m_pSynchBarrierEvent
        if ( NULL != m_pSynchBarrierEvent)
        {
            pEvent->SetDependentOn(m_pSynchBarrierEvent);
        }
    }

    if ( CL_COMMAND_BARRIER         == clCommandType ||
         CL_COMMAND_WAIT_FOR_EVENTS == clCommandType )
    {
        // Insert an aSynch flush command
        Flush(false);
        m_pSynchBarrierEvent = pEvent;
    }
}

/******************************************************************
 * pCommand is a run time command that just has done and is expected to
 * be handle respectively.
 * 
 * If command is m_pSynchBarrierEvent (Barrier, WaitOnEvents) clear it. 
 * If it is a "User" flush command all previous commands issued, reduce the counter.
 * If an internal flush command turned black it means that a flush need to issue, release the list
 * 
 ******************************************************************/
void OutOfOrderCommandQueue::HandleRuntimeCommandExecuted( Command* pCommand)
{
    if( pCommand->GetEvent() == m_pSynchBarrierEvent )
    {
        m_pSynchBarrierEvent = NULL;
    }
    else if( CL_COMMAND_FLUSH == pCommand->GetCommandType())
    {
        m_uiFlushCnt--;
    }
}

