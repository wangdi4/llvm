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
//  Created on:      23-Dec-2008 3:23:06 PM
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////
#include "out_of_order_queue.h"
#include "queue_event.h"
#include "enqueue_commands.h"

//For debug, TODO: remove
#include <stdio.h>

using namespace Intel::OpenCL::Framework;

/******************************************************************
 *
 ******************************************************************/
OutOfOrderQueue::OutOfOrderQueue():
    m_pLastBarrierCmd(NULL)
{
}

/******************************************************************
 *
 ******************************************************************/
OutOfOrderQueue::~OutOfOrderQueue()
{
}

/******************************************************************
 * Clean the last barrier if black and use InOrder::StableLists to stable
 ******************************************************************/
bool OutOfOrderQueue::StableLists()
{
    if( (NULL != m_pLastBarrierCmd) &&
        m_pLastBarrierCmd->GetEvent()->IsColor(QueueEvent::EVENT_STATE_BLACK))
    {
        m_pLastBarrierCmd = NULL;
    }
    // For debug: 
    // printf("Call to StableLists\n");
    // The black commands are deleted.
    return InOrderQueue::StableLists();
}

/******************************************************************
 * TODO: Synchronize change of event and the status of the lists.
 ******************************************************************/
cl_err_code OutOfOrderQueue::AddCommand(Command* pCommand)
{
    if ( true == m_bCleanUp )
    {
        return CL_ERR_FAILURE;
    }
    QueueEvent* pEvent = pCommand->GetEvent();
    // Init to true in case that the event is not already dependent.
    bool bIsGreen = (!(pEvent->IsColor(QueueEvent::EVENT_STATE_RED)));

    // Start critical section - must stable all list during processing
    { OclAutoMutex CS(m_pListLockerMutex);
        // To add command in a stable status the list must be stabled too.
    StableLists();

    cl_command_type clCommandType = pCommand->GetCommandType();
    // All need to register on last Barrier
    // Todo block event from changing color during registration... need to be done inside event.
    if( NULL != m_pLastBarrierCmd )
    {
        pEvent->SetDependentOn(m_pLastBarrierCmd->GetEvent());
        bIsGreen = false;
    }

    switch(clCommandType)
    {
    case CL_COMMAND_MARKER:
        if(RegisterAsBarrier(pEvent))
        {
            // There was command before in the queue
            bIsGreen = false;
        }
        break;
    case CL_COMMAND_BARRIER:
        if(RegisterAsBarrier(pEvent))
        {
            // There was command before in the queue
            bIsGreen = false;
        }
        // Fall through to Wait For Events
    case CL_COMMAND_WAIT_FOR_EVENTS:
        // Wait for events was already set events in the EnqueueCommand function. Need only to register as next Barrier.
        m_pLastBarrierCmd = pCommand;
        // Fall through, all commands are inserted into queue 
    default:
        break;
    }
    // All - enter into Queue
    if (bIsGreen)
    {
        m_readyCmdsList.push_back(pCommand);
    }
    else
    {
        m_waitingCmdsList.push_back(pCommand);
    }

    }// End of Critical Section 

    if(bIsGreen)
    {
        // New command is green, signal event to change color and worker to process it
        pEvent->SetEventColor(QueueEvent::EVENT_STATE_GREEN);
    }
    return CL_SUCCESS;
}


/******************************************************************
 * Return true if the event was register on other events at least once.
 ******************************************************************/
bool OutOfOrderQueue::RegisterAsBarrier(QueueEvent* pEvent)
{
    bool isRegistered = false;
    
    // This event represents Barrier command, set dependecy on all queued commands.
       
    // Device list
    list<Command*>::iterator iter = m_deviceCmdsList.begin();
    while( iter != m_deviceCmdsList.end() )
    {
        Command* pPrevCommand = *iter;
        QueueEvent* pPrevEvent = pPrevCommand->GetEvent();

        if ( pPrevCommand != m_pLastBarrierCmd )
        {
            // Don't register on an event that you have already did
            pEvent->SetDependentOn(pPrevEvent);
            isRegistered = true;
        }

        if( CL_COMMAND_BARRIER == pPrevCommand->GetCommandType())
        {
            // A previous barrier, can stop here
            return isRegistered;
        }
        iter++;
    }

    // Ready list
    iter = m_readyCmdsList.begin();
    while( iter != m_readyCmdsList.end() )
    {
        Command* pPrevCommand = *iter;
        QueueEvent* pPrevEvent = pPrevCommand->GetEvent();

        if ( pPrevCommand != m_pLastBarrierCmd )
        {
            // Don't register on an event that you have already did
            pEvent->SetDependentOn(pPrevEvent);
            isRegistered = true;
        }

        if( CL_COMMAND_BARRIER == pPrevCommand->GetCommandType())
        {
            // A previous barrier, can stop here
            return isRegistered;
        }
        iter++;
    }

    // Waiting list
    iter = m_waitingCmdsList.begin();    
    while( iter != m_waitingCmdsList.end() )
    {
        Command* pPrevCommand = *iter;
        QueueEvent* pPrevEvent = pPrevCommand->GetEvent();

        if ( pPrevCommand != m_pLastBarrierCmd )
        {
            // Don't register on an event that you have already did
            pEvent->SetDependentOn(pPrevEvent);
            isRegistered = true;
        }

        if( CL_COMMAND_BARRIER == pPrevCommand->GetCommandType())
        {
            // A previous barrier, can stop here
            return isRegistered;
        }
        iter++;
    }


    return isRegistered;
}
