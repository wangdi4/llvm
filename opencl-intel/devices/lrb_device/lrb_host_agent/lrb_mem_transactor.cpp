// Copyright (c) 2006-2009 Intel Corporation
// All rights reserved.
// 
// WARRANTY DISCLAIMER
// 
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING ANY WAY OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

///////////////////////////////////////////////////////////
//  lrb_mem_transactor.cpp
///////////////////////////////////////////////////////////
#include "lrb_mem_transactor.h"
#include "lrb_memory_manager.h"

#include <cl_types.h>

using namespace Intel::OpenCL::LRBAgent;
using namespace std;

/************************************************************************
* Creates LrbMemTransactor object
************************************************************************/
LrbMemTransactor::LrbMemTransactor( LrbMemoryManager* pMemoryManager ):
    m_pMemoryManager(pMemoryManager)
{
}

/************************************************************************
 * Destroys the Transactor.
 * Stop the processing thread by using broadcasting
 * 
 ************************************************************************/
LrbMemTransactor::~LrbMemTransactor()
{
    CancelProcessing();
}

/************************************************************************
 * Wait until thread is finishing
 ************************************************************************/
int LrbMemTransactor::CancelProcessing()
{
    { OclAutoMutex CS(&m_commandsLocker);
    m_join = true;
    }
    m_cond.Broadcast();    
    Join();
    return CL_SUCCESS;
}

/************************************************************************
 *
 ************************************************************************/
CommandEntry* LrbMemTransactor::GetNextTransaction()
{
    CommandEntry* pNextCommand = NULL;
    COND_RESULT res = COND_RESULT_OK;

    OclAutoMutex CS(&m_commandsLocker); // Start Critical section

    // This method loop on the ready list.
    while ( 
        m_cmdsList.empty()                  && 
        res != COND_RESULT_COND_BROADCASTED      // Broadcast is used to terminate execution of the queue
        )         
    {
        res = m_cond.Wait(&m_commandsLocker);
    } 

    if( res!=COND_RESULT_COND_BROADCASTED )
    { 
        assert(!m_cmdsList.empty() && "ready list is empty");
        // The list is not empty, pop out command and use it.
        pNextCommand = m_cmdsList.front();
        m_cmdsList.pop_front();
    }
    return pNextCommand;        
}

/************************************************************************
 * This is the working loop.
 *
 ************************************************************************/
int LrbMemTransactor::Run()
{
    CommandEntry* pNextCommand;
    int           status = CL_SUCCESS;

    // the infinite loop of execution
    while (1) 
    {
        // Get next command
        // If there isn't any command the function is blocked
        // If NULL is returned, the thread should finish
        pNextCommand = GetNextTransaction();
        if (NULL == pNextCommand )
        {
            // There is no next command, get out of the loop
            break;
        }
        // if we got here we have a memory transaction, let's process it
        m_pMemoryManager->ProcessMemoryTransaction(pNextCommand);        
    }
    return CL_SUCCESS;
}

/************************************************************************
 * 
 ************************************************************************/
int LrbMemTransactor::AddTransaction(CommandEntry* pCommand)
{
    OclAutoMutex CS(&m_commandsLocker); // Start Critical section
    m_cmdsList.push_back(pCommand);
    return CL_SUCCESS;
}
