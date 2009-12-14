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
//  lrb_communicator.cpp
///////////////////////////////////////////////////////////
#include "lrb_communicator.h"
#include "lrb_xn_wrapper.h"
#include "lrb_agent_common.h"
#include "lrb_commands_list.h"
#include "lrb_command_executer.h"
#include "lrb_program_service.h"

#include <cl_types.h>

using namespace Intel::OpenCL::LRBAgent;
using namespace Intel::OpenCL::Utils;

/************************************************************************
 * Creates communicator object
 ************************************************************************/
LrbCommunicator::LrbCommunicator( 
    XNWrapper*                  pXnWarrper, 
    LrbCommandExecuter*         pCmdExecuter,
    LrbProgramService*          pProgService,
    fn_clDevCmdStatusChanged*   pclDevCmdStatusChanged
    ):
m_pXnWarraper(pXnWarrper),
m_pCmdExecuter(pCmdExecuter),
m_pProgService(pProgService),
m_pclDevCmdStatusChanged(pclDevCmdStatusChanged)
{
}

/************************************************************************
 * Destroys the communicator.
 * If the thread is running, the function wait for it completion (join...)
 * This is done through the parent object OclThread
 * The thread will stop when the native device is done using the NotifyProcessDone
 ************************************************************************/
LrbCommunicator::~LrbCommunicator()
{
    m_pXnWarraper->NotifyProcessDone();
}

/************************************************************************
 * Gets the next available job in the job list.
 * If there is no job, this function waits until job is ready or join
 * has been called.
 * If message was not available CL_ERR_FAILURE returns, else CL_SUCCESS and pReceivedMessage
 * is filled with the message content.
 * Note, if communicator was released during wait on ReceiveMessage, the function will return failed status.
 ************************************************************************/
int LrbCommunicator::GetNextMessage(CommandDoneMessage* pReceivedMessage)
{
    // Wait for next command.
    cl_int result = m_pXnWarraper->ReceiveMessageSync(pReceivedMessage);
    
    if( 0 != result )
    {
        return CL_ERR_FAILURE;
    }
    return CL_SUCCESS;
}

/************************************************************************
 * This is the working loop.
 * It runs for ever and receives next message all the time.
 * If GetNextMessage returns with error, the loop is terminated. For instance,
 * in case the the communicator is released (shutdown process)
 ************************************************************************/
int LrbCommunicator::Run()
{
    CommandDoneMessage nextCommand;
    int             status = CL_SUCCESS;

    // the infinite loop of execution
    while (1) 
    {

        // Get next command
        // If there isn't any command the function is blocked
        status = GetNextMessage(&nextCommand);
        if (CL_FAILED(status))
        {
            // There is no next message, get out of the loop
            break;
        }
        // if got here we have a new command notification in hands and can proceed it
        if( 0 == nextCommand.cmdHndl)
        {
            // NotifyDone...
            break;
        }

        // Check if it is a command or a build done notification
        if( STS_BUILD_DONE == nextCommand.status )
        {
            // Notify build done on the program that in the cmdHndl
            m_pProgService->NotifyBuildDone(nextCommand.cmdHndl);
            continue; // Be ready for next commands
        }

         
        // 1. Notify framework on change.
        // TODO: (1) Add error result to command (2) Add profiling handling.
        CommandEntry* pCmdEntry = (CommandEntry*)nextCommand.cmdHndl;
        cl_dev_cmd_desc* pCmdDec = pCmdEntry->pclCmd;
        m_pclDevCmdStatusChanged(pCmdDec->id, pCmdDec->data, nextCommand.status, CL_SUCCESS, 0 /* time=0*/ );

        // 2. Notify memory transactor to resolve dependencies.
        if ( STS_COMPLETE == nextCommand.status)
        {
            pCmdEntry->eState = CMD_COMPELETED;
            m_pCmdExecuter->SignalCommandDone(pCmdEntry->pCmdList);
        }        
    }
    return CL_SUCCESS;
}

