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
//  in_order_queue.cpp
//  Implementation of the Class InOrderQueue
//  Created on:      23-Dec-2008 3:23:02 PM
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////
#include "in_order_queue.h"
#include "enqueue_commands.h"
#include "queue_event.h"
#include <assert.h>

using namespace Intel::OpenCL::Framework;

/******************************************************************
 * Constructor.
 * Creates the private objects
 ******************************************************************/
InOrderQueue::InOrderQueue():
    m_bCleanUp(false)
{
    // Create synch objects
    m_pCond             = new OclCondition();
    m_pListLockerMutex  = new OclMutex();

    // Set logger
	m_pLoggerClient     = new LoggerClient(L"InOrderQueue Logger Client",LL_DEBUG);
	InfoLog(m_pLoggerClient, L"InOrderQueue created");
}

/******************************************************************
 *
 ******************************************************************/
InOrderQueue::~InOrderQueue()
{
    // First empty the list.
    Release();
   
    m_bCleanUp = true;

    // Clear resources 
    delete m_pCond;
    delete m_pListLockerMutex;
    delete m_pLoggerClient;

    m_pCond = NULL;
    m_pListLockerMutex = NULL;
    m_pLoggerClient = NULL;
}

/******************************************************************
 * This function adds command in the front of the waiting list.
 * If there is no event that is ready for processing, this command
 * becomes ready for execution.
 * 
 ******************************************************************/
cl_err_code InOrderQueue::PushFront(Command* pCommand)
{  
    if ( true == m_bCleanUp )
    {
        return CL_ERR_FAILURE;
    }
    // Start Critical Section - lock access to all lists.
    { OclAutoMutex CS(m_pListLockerMutex);

    StableLists();
    Command* pPrevCommand = NULL;
    Command* pNextCommand = NULL;

    if( ! m_waitingCmdsList.empty())
    {
        pNextCommand = m_waitingCmdsList.front();
    }

    if( ! m_readyCmdsList.empty())
    {
        pPrevCommand = m_readyCmdsList.back();
    }
    else if ( ! m_deviceCmdsList.empty())
    {
        pPrevCommand = m_deviceCmdsList.back();
    }

    if ( NULL == pPrevCommand )
    {
        // The command is ready for execute, no dependencies.
        pCommand->GetEvent()->SetEventColor(QueueEvent::EVENT_STATE_GREEN);
        m_readyCmdsList.push_back(pCommand);
    }
    else
    {
        // Get into the depenecy list
        pCommand->GetEvent()->SetEventColor(QueueEvent::EVENT_STATE_RED);
        pCommand->GetEvent()->SetDependentOn(pPrevCommand->GetEvent());
        m_waitingCmdsList.push_front(pCommand);
    }

    // If there is a command that waits on the queue, set it to be dependent in the new one
    if ( NULL != pNextCommand )
    {
        pNextCommand->GetEvent()->SetDependentOn(pCommand->GetEvent());
    }        
    }// End of critical section

    // New command may be already green, signal the worker to process it    
    m_pCond->Signal(); // TODO: Check races here, may create problems.

    return CL_SUCCESS;
}


/******************************************************************
 * This command returns the first command that is ready for processing
 * by device, to say the first command from the front that is green.
 * It is essential to change the command to lime before unlocking the list,
 * unless the command will be popped twice.
 * If there is no ready command, the thread is blocked on this command.
 * This function is used also for garbage collection of done commands.
 * if a command is black it will be removed and deleted.
 *

 ******************************************************************/
Command* InOrderQueue::GetNextCommand()
{
    Command* pNextCommand = NULL;
    COND_RESULT res = COND_RESULT_OK;
    m_pListLockerMutex->Lock();
    // This method loop on the ready list.
    // Each call to StableList, may enter object into the ready list.
    while (m_readyCmdsList.empty() && res != COND_RESULT_COND_BROADCASTED)
    {
        res = m_pCond->Wait(m_pListLockerMutex);
    }
    if(res!=COND_RESULT_COND_BROADCASTED)
    {
        // Found a command, pop is out and move it to the device
        pNextCommand = m_readyCmdsList.front();
        m_readyCmdsList.pop_front();
        pNextCommand->GetEvent()->SetEventColor(QueueEvent::EVENT_STATE_LIME);
        m_deviceCmdsList.push_back(pNextCommand);
    }
    m_pListLockerMutex->Unlock();
    return pNextCommand;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code InOrderQueue::AddCommand(Command* pCommand)
{       
    if ( true == m_bCleanUp )
    {
        return CL_ERR_FAILURE;
    }

    Command* pDependsOnCommand = NULL;
    QueueEvent* pEvent = pCommand->GetEvent();
    // Start critical section - must stable all list during processing
    { OclAutoMutex CS(m_pListLockerMutex);
    // To add command in a stable status the list must be stabled too.
    StableLists();
    // Find, if exists, a command to be depenent on,
    // This to apply the In-Order policy
    if( !m_waitingCmdsList.empty() )
    {
        pDependsOnCommand = m_waitingCmdsList.back();
    }
    else if ( !m_readyCmdsList.empty())
    {
        pDependsOnCommand = m_readyCmdsList.back();
    }
    else if ( !m_deviceCmdsList.empty())
    {
        pDependsOnCommand = m_deviceCmdsList.back();
    }
    if( NULL != pDependsOnCommand)
    {
        // new command are depent on something... 
        // register and enter to waiting list
        QueueEvent* pDependsOnEvent = pDependsOnCommand->GetEvent();
        pEvent->SetDependentOn(pDependsOnEvent);
        m_waitingCmdsList.push_back(pCommand);
    }
    else // Redey for imidate processing
    {
        pEvent->SetEventColor(QueueEvent::EVENT_STATE_GREEN);
        m_readyCmdsList.push_back(pCommand);
    }        
    }// End of Critical Section

    // New command may be already green, signal the worker to process it    
    m_pCond->Signal(); // TODO: Check races here, may create problems.

    return CL_SUCCESS;
}

/******************************************************************
 * Signals the queue to signal is waiting event
 * The queue uses this function to clean all its lists and to return them to
 * a blance state.
 ******************************************************************/
void InOrderQueue::Signal()
{
    if ( true == m_bCleanUp )
    {
        return;
    }

    {OclAutoMutex CS(m_pListLockerMutex);
    StableLists();
    }
    m_pCond->Signal();
}

/******************************************************************
 * Returns the lists to a stable state
 * Means, that each command is assigned to the right list.
 * 
 ******************************************************************/
void InOrderQueue::StableLists()
{
    if ( true == m_bCleanUp )
    {
        return;
    }

    // Go from bottom to top;

    // Clean black events (done events)
    list<Command*>::iterator iter  = m_deviceCmdsList.end();
    list<Command*>::iterator first = m_deviceCmdsList.begin();    
    // loop and clean;
    for ( ; iter != first; iter-- ) 
    {
        if((*iter)->GetEvent()->IsColor(QueueEvent::EVENT_STATE_BLACK))
        {
            delete (*iter);
            iter = m_deviceCmdsList.erase(iter);
        }
    }

    // Move green commands to ready list
    iter  = m_waitingCmdsList.end();
    first = m_waitingCmdsList.begin();    
    // loop and clean;
    for ( ; iter != first; iter-- ) 
    {
        Command* pCommand = *iter;
        if(pCommand->GetEvent()->IsColor(QueueEvent::EVENT_STATE_GREEN))
        {             
            iter = m_deviceCmdsList.erase(iter);
            m_readyCmdsList.push_back(pCommand);
        }
    }
}

/******************************************************************
 * Release all resources immediately.
 * If command is exist, it is lost forever...
 * Commands that are not black and are going to be deleted, must notify
 * that they are done.
 * This operation may last for long
 ******************************************************************/
cl_err_code InOrderQueue::Release()
{
    //Time to broadcasting, clear the list
    // and broadcast all waiters to stop
    // waiting and to return NULL Command
    Command* pCommand = NULL;    
    m_bCleanUp = true; // Setting this to true prevents deadlock on EventCompleted methods.
    { OclAutoMutex CS(m_pListLockerMutex);
    // Top-Down, start to free the waiting list
    while(!m_waitingCmdsList.empty())
    {
        pCommand = m_waitingCmdsList.front();
        pCommand->GetEvent()->EventCompleted();
        m_waitingCmdsList.pop_front();
        delete pCommand;
    }
    while(!m_readyCmdsList.empty())
    {
        pCommand = m_readyCmdsList.front();
        pCommand->GetEvent()->EventCompleted();
        m_readyCmdsList.pop_front();
        delete pCommand;
    }
    while(!m_deviceCmdsList.empty())
    {
        pCommand = m_deviceCmdsList.front();
        QueueEvent* pEvent = pCommand->GetEvent();
        if ( !(pEvent->IsColor(QueueEvent::EVENT_STATE_BLACK)))
        {
            pEvent->EventCompleted();
        }
        m_readyCmdsList.pop_front();
        delete pCommand;
    }

    } // End of Critical Section

    m_pCond->Broadcast();
    return CL_SUCCESS;
}

/******************************************************************
 * Returns true if all lists are empty. Note that this function
 * returns only the current status of the object. If other threads
 * result addition to a command to this queue, by the function returns
 * the queue may not be empty.
 * 
 ******************************************************************/
bool InOrderQueue::IsEmpty() const
{
    OclAutoMutex CS(m_pListLockerMutex);
    bool  bEmpty = (m_waitingCmdsList.empty() && m_readyCmdsList.empty() && m_deviceCmdsList.empty());
    return bEmpty;
}

/******************************************************************
 * Returns the sum of commands in all lists.
 * returns only the current status of the object. If other threads
 * result addition to a command to this queue, by the function returns
 * the queue size may changes.
 * 
 ******************************************************************/
cl_uint InOrderQueue::Size() const
{
    OclAutoMutex CS(m_pListLockerMutex);
    cl_uint uiSize =  m_waitingCmdsList.size() + m_readyCmdsList.size() + m_deviceCmdsList.size();
    return uiSize;
}
