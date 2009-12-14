// Copyright (c) 2008-2009 Intel Corporation
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
//  lrb_memory_manager.h
//  
//  Created on:      1-JuLY-2009
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////
#if !defined(__LRB_COMMAND_EXECUTER_H__)
#define __LRB_COMMAND_EXECUTER_H__

#include "lrb_commands_list.h"
#include <cl_device_api.h>
#include <cl_table.h>

#include <map>

namespace Intel { namespace OpenCL { namespace LRBAgent {

    //
    // Forward declaration
    //
    class XNWrapper;
    class LrbMemoryManager;
    class LrbCommandsList;

    /**********************************************************************************************
     * Class name:    LrbCommandExecuter
     *
     * Description:    
     *      This class is responsible to allocate resources for the OpenCL Memory object.
     *      In addition, it is up to the manager to synchronize accesses to any memory object.
     *
     * Author:        Arnon Peleg
     * Date:          July 2009
    /**********************************************************************************************/    

    class LrbCommandExecuter
    {
    public:

        LrbCommandExecuter( XNWrapper* pXnWrapper, LrbMemoryManager* pMemoryMgr );
        ~LrbCommandExecuter();

	    cl_int CreateCommandList  ( cl_dev_cmd_list_props props, cl_dev_cmd_list* pList );
	    cl_int ReleaseCommandList ( cl_dev_cmd_list list );
	    cl_int RetainCommandList  ( cl_dev_cmd_list list );
	    cl_int FlushCommandList   ( cl_dev_cmd_list list );
        cl_int ExecuteCommands    ( cl_dev_cmd_list list, cl_dev_cmd_desc** ppCmds, cl_uint uiCount);
        cl_int SignalCommandDone  ( LrbCommandsList* pList, bool bIsBlockedCommand = false );

    private:
        Intel::OpenCL::Utils::ClTable*  m_pListsTable;  // Table that holds all lists.
        XNWrapper*                      m_pXnWrapper;
        LrbMemoryManager*               m_pMemoryMgr;

        // Private functions
        cl_int ProcessBlockedCommand( LrbCommandsList* pList );

    };

}}};    // Intel::OpenCL::LRBAgent
#endif // __LRB_COMMAND_EXECUTER_H__
