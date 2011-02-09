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
//  lrb_command_executer.cpp
///////////////////////////////////////////////////////////
#include "lrb_command_executer.h"
#include "lrb_xn_wrapper.h"
#include "lrb_commands_list.h"
#include "lrb_memory_manager.h"
#include <cl_table.h>

using namespace Intel::OpenCL::LRBAgent;
using namespace Intel::OpenCL::Utils;
using namespace std;

/************************************************************************
 * 
 ************************************************************************/
LrbCommandExecuter::LrbCommandExecuter( XNWrapper* pXnWrapper, LrbMemoryManager* pMemoryMgr ):
    m_pXnWrapper(pXnWrapper),
    m_pMemoryMgr(pMemoryMgr)
{
    m_pListsTable = new Intel::OpenCL::Utils::ClTable;
}

/************************************************************************
 * 
 ************************************************************************/
LrbCommandExecuter::~LrbCommandExecuter()
{
    // TODO: Clean up queues

    delete m_pListsTable;
    m_pListsTable = NULL;
}

/************************************************************************
 * 
 ************************************************************************/
cl_int LrbCommandExecuter::CreateCommandList( cl_dev_cmd_list_props clListProps, cl_dev_cmd_list* pListID )
{
    // TODO: support Out-Of-Order list. Currently all lists here are in-order
    LrbCommandsList* pNewList = new LrbCommandsList(m_pXnWrapper);
    unsigned long ulListId = m_pListsTable->Insert(pNewList);
    *pListID = (cl_dev_cmd_list)ulListId;
    return CL_SUCCESS;   
}

/************************************************************************
 * 
 ************************************************************************/
cl_int LrbCommandExecuter::ReleaseCommandList( cl_dev_cmd_list list )
{
    LrbCommandsList* pList = (LrbCommandsList*)m_pListsTable->Get((unsigned long)list);
    if( 0 == pList )
    {
        return CL_DEV_INVALID_VALUE;
    }

    cl_int result = pList->Release();
    //
    // If reference count is 9, release means flush the queue and mark it for deletion
    // TODO: Support list deletion
    //
    if( CL_SUCCESS == result )
    {
        if( 0 == pList->GetReferenceCount())
        {
            result = pList->FlushList();
            // TODO: Mark for deletion...
        }
    }
    return result;
}

/************************************************************************
 * 
 ************************************************************************/
cl_int LrbCommandExecuter::RetainCommandList( cl_dev_cmd_list list )
{
    LrbCommandsList* pList = (LrbCommandsList*)m_pListsTable->Get((unsigned long)list);
    if( 0 == pList )
    {
        return CL_DEV_INVALID_VALUE;
    }
    return pList->Retain();
}

/************************************************************************
 * 
 ************************************************************************/
cl_int LrbCommandExecuter::FlushCommandList( cl_dev_cmd_list list )
{
    LrbCommandsList* pList = (LrbCommandsList*)m_pListsTable->Get((unsigned long)list);
    if( 0 == pList )
    {
        return CL_DEV_INVALID_VALUE;
    }
    cl_int result = pList->FlushList();

    //
    // After flush, maybe there is blocking command. So if there is, put it in the
    // memory transaction lists
    //
    ProcessBlockedCommand(pList);

    return result;
}

/************************************************************************
 * The executer pushes all commands into the list in-order.
 * The order is the order of the list.
 * In addition the command is added to a command map that holds all commands
 * The command map list is used to get command on done event and to associate with its list
 ************************************************************************/
cl_int LrbCommandExecuter::ExecuteCommands( cl_dev_cmd_list list, cl_dev_cmd_desc** ppCmds, cl_uint uiCount)
{
    cl_int result = CL_SUCCESS;
    LrbCommandsList* pList = (LrbCommandsList*)m_pListsTable->Get((unsigned long)list);
    if( 0 == pList )
    {
        return CL_DEV_INVALID_VALUE;
    }
    for( cl_uint ui=0; ui<uiCount; ui++)
    {
        CommandEntry* pCommandEntry = new CommandEntry;
        pCommandEntry->pclCmd = ppCmds[ui];
        pCommandEntry->eState = CMD_QUEUED;
        pCommandEntry->pCmdList = pList;
        pCommandEntry->pMappedBufHndls = 0;
        pCommandEntry->uiNumOfBufHndls = 0;
        
        // Is host command??? Currently Read/Write/Map/Unmap are host commands.
        // TODO: Return pointer for map command.
        cl_dev_cmd_type cmdType = ppCmds[ui]->type;
        switch (cmdType)
        {
        case CL_DEV_CMD_READ:
        case CL_DEV_CMD_WRITE:
        case CL_DEV_CMD_MAP:
        case CL_DEV_CMD_UNMAP:
            // All of the above are host commands, e.g are executed on host
            pCommandEntry->bIsHostCommand = true;
        	break;
        default:
            // All others are executed on the device.
            pCommandEntry->bIsHostCommand = false;
        }

        // 1. Push entry to list
        result = pList->PushCommand(pCommandEntry);
        if( CL_SUCCESS != result )
        {
            // Remove 
            delete pCommandEntry;
            return result;
        }        
    }
    return result;
}

/************************************************************************
 * When command is done, the executer is signaled.
 * As a response, the executer is expected to purge the list that includes
 * the completed command, to pop out and to process (signal the Transactor)
 * Memory transaction command if available, and to flush all new available commands
 * in case that the completed command was a blocking command.
 *
 * Inputs:  uiList : The list to be handled.
            bIsBlockedCommand: If true, the completed command was blocking and we 
            should re-execute flushed commands.
 ************************************************************************/
cl_int LrbCommandExecuter::SignalCommandDone( LrbCommandsList* pList, bool bIsBlockedCommand )
{
    cl_int result = CL_SUCCESS;
    result = pList->PurgeList();
    if(CL_SUCCESS == result)
    {
        if (bIsBlockedCommand)
        {
            //
            // If true, Read/Write command was done and there are commands to be flushed
            //
            pList->ExecuteFlushedCmds();
        }
        result = ProcessBlockedCommand(pList);
    }
    return result;
}

/************************************************************************
 * This function check if there is a blocking command in the front of
 * the list (pList) that is ready for processing.
 * If there is, this command is popped out, is added to the memory transactions queue
 * and the Transactor queue is set to run
 ************************************************************************/
cl_int LrbCommandExecuter::ProcessBlockedCommand( LrbCommandsList* pList )
{
    cl_int result = CL_SUCCESS;
    CommandEntry* pBlockingCmd = NULL;

    result = pList->GetBlockHostCommand(&pBlockingCmd);
    if( CL_SUCCESS == result && NULL != pBlockingCmd )
    {
        // The next command is a blocking command
        result = m_pMemoryMgr->QueueMemoryTransaction(pBlockingCmd);
        if(CL_SUCCESS == result)
        {
            m_pMemoryMgr->SignalMemoryTransactor();
        }
    }
    return result;
}
