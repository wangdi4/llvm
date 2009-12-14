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
//  lrb_commands_list.cpp
///////////////////////////////////////////////////////////
#include "lrb_commands_list.h"
#include "lrb_xn_wrapper.h"
#include "lrb_agent_common.h"

using namespace Intel::OpenCL::LRBAgent;

/******************************************************************
 *
 ******************************************************************/
LrbCommandsList::LrbCommandsList( XNWrapper* xnWrapper ):
    m_xnWrapper(xnWrapper),
    m_uiRefCount(0)
{
    Retain();
}

/******************************************************************
 *
 ******************************************************************/
LrbCommandsList::~LrbCommandsList()
{
    assert( !m_cmdsList.empty());
}

/************************************************************************
 * 
 ************************************************************************/
cl_int LrbCommandsList::Release()
{
    if (m_uiRefCount <= 0)
    {
        return CL_INVALID_OPERATION;
    }
    --m_uiRefCount;
    return CL_SUCCESS;
}

/************************************************************************
 * 
 ************************************************************************/
cl_int LrbCommandsList::Retain()
{
    ++m_uiRefCount;
    return CL_SUCCESS;
}


/******************************************************************
 * Add command to the back of the list.
 ******************************************************************/
cl_int LrbCommandsList::PushCommand(CommandEntry* pCmdEntry)
{
    OclAutoMutex CS(&m_muListLocker);
    m_cmdsList.push_back(pCmdEntry);
    return CL_SUCCESS;
}

/************************************************************************
 * Pop the command in the top of the list. Only command that is completed
 * can be popped. If the top command is not completed a NULL entry is returned
 * with CL_SUCCESS value.
 ************************************************************************/
cl_int LrbCommandsList::PopCommand(CommandEntry** OUT ppCmdEntry)
{
    OclAutoMutex CS(&m_muListLocker);
    CommandEntry* pEntry = NULL;
    *ppCmdEntry = NULL;
    if(!m_cmdsList.empty())
    {
        pEntry = m_cmdsList.front();
        if( CMD_COMPELETED == pEntry->eState)
        {
            // Pop the command and set return value;
            m_cmdsList.pop_front();
            *ppCmdEntry = pEntry;            
        }
    }
    return CL_SUCCESS;
}

/************************************************************************
 * Returns a command entry according to its id in ppCmdEntry.
 * If command does not exist a NULL entry is returned with CL_SUCCESS value.
 * Commands are traversed from the front to the back, since usually old commands
 * are more expected to be searched to.
 ************************************************************************/
cl_int LrbCommandsList::FindCommand( cl_dev_cmd_id clCmdId, CommandEntry** ppCmdEntry)
{
    *ppCmdEntry = NULL;
    OclAutoMutex CS(&m_muListLocker);
    list<CommandEntry*>::iterator iter;
    for ( iter = m_cmdsList.begin(); iter != m_cmdsList.end(); iter++) 
    {
        if ( clCmdId = (*iter)->pclCmd->id)
        {
            *ppCmdEntry = *iter;
            break;
        }
    }
    return CL_SUCCESS;
}

/************************************************************************
 * Returns the first command if it is a blocked.
 * If it isn't a block command a NULL value is returned.
 ************************************************************************/
cl_int LrbCommandsList::GetBlockHostCommand(CommandEntry** OUT ppCmdEntry)
{
    *ppCmdEntry = NULL;
    OclAutoMutex CS(&m_muListLocker);
    if(!m_cmdsList.empty())
    {
        CommandEntry* pEntry = m_cmdsList.front();        
        if( pEntry->bIsHostCommand      &&
            CMD_FLUSHED == pEntry->eState
            )
        {
            // OK, the front is a blocked command.
            *ppCmdEntry = pEntry;
            pEntry->eState = CMD_SUBMITTED;

        }
    }
    return CL_SUCCESS;
}

/************************************************************************
 * Working in dual ways.
 * 1st - mark all commands as flushed.
 * 2nd - offload device commands to the LRB native agent using xnWrapper
 ************************************************************************/
cl_int LrbCommandsList::FlushList()
{
    OclAutoMutex CS(&m_muListLocker);
    // Flush all
    list<CommandEntry*>::iterator iter;
    for ( iter = m_cmdsList.begin(); iter != m_cmdsList.end(); iter++) 
    {
        (*iter)->eState = CMD_FLUSHED;
    }
    return ExecuteFlushedCmds();
}

/************************************************************************
 * Offload ready (flushed, not blocking) device commands to the LRB native agent using xnWrapper
************************************************************************/
cl_int LrbCommandsList::ExecuteFlushedCmds()
{
    OclAutoMutex CS(&m_muListLocker);
    // 1. Iterates from the front and stop on blocking command.
    // 2. Prepare commands list to be used by xnWrapper to offload commands.
    list<CommandEntry*>::iterator blockIter;
    cl_uint uiCommandsCounter = 0;
    bool bToBreak = false;
    for ( blockIter = m_cmdsList.begin(); blockIter != m_cmdsList.end(); blockIter++) 
    {
        // while flushed, moved until host command.
        CommandEntry* cmdEntry = *blockIter;        
        switch(cmdEntry->eState)
        {
        case CMD_FLUSHED:
            if(cmdEntry->bIsHostCommand)
            {
                // Ok, block command..
                bToBreak = true;
                break;                
            }
            else
            {
                uiCommandsCounter++;
            }
            break;            
        case CMD_QUEUED:
            // Start of commands that were not flushed
            bToBreak = true;
            break;
        case CMD_COMPELETED: // Fall through
        case CMD_SUBMITTED:  // Fall through
        default:
            // Case in which command need to be ignored;
            bToBreak = false;
            break;
        }
        if(bToBreak)
        {
            break;
        }
    }

    // At this point uiCommandsCounter means how many execution ready commands
    // Are available from the front of the list
    if( 0 != uiCommandsCounter )
    {
        // Create command list
        CommandEntry** ppCmds = (CommandEntry**)malloc(sizeof(CommandEntry*)*uiCommandsCounter);
        list<CommandEntry*>::iterator iter;
        cl_uint uiListCommands = 0;
        for ( iter = m_cmdsList.begin(); uiListCommands < uiCommandsCounter ; iter++, uiListCommands++ ) 
        {
            CommandEntry* pCmd = (*iter);
            ppCmds[uiListCommands] = pCmd;
            pCmd->eState = CMD_SUBMITTED;
        }
        m_xnWrapper->ExecuteCommands(ppCmds, uiCommandsCounter);
        free(ppCmds);
    }
    return CL_SUCCESS;
}

/************************************************************************
 * This function pop out all completed commands. In particularly the one
 * cause the call for this command
 ************************************************************************/
cl_int LrbCommandsList::PurgeList()
{
    OclAutoMutex CS(&m_muListLocker);
    CommandEntry* pPoppedEntry = NULL;
    while (1)
    {
        PopCommand(&pPoppedEntry);
        if (NULL != pPoppedEntry)
        {
            // Verify that the command resources are free
            m_xnWrapper->UnmapCommand(pPoppedEntry);
            delete pPoppedEntry;
        }
        else
        {
            break;
        }
    }
    return CL_SUCCESS;
}
