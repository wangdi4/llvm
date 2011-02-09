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
//  lrb_commands_list.h
//  Implementation of the Class LrbCommandsList
//  Created on:      27-Jul-2009 1:02:41 PM
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////
#if !defined(__LRB_COMMAND_LIST_H__)
#define __LRB_COMMAND_LIST_H__

#include "cl_types.h"

#include <cl_object.h>
#include <list>
#include <cl_synch_objects.h>

using namespace Intel::OpenCL::Utils;
using namespace std;

namespace Intel { namespace OpenCL { namespace LRBAgent {

    //
    // Forward declarations
    //
    class XNWrapper;
    class LrbCommandsList;

    /**********************************************************************************************
     * Class name:    CommandState
     * Description:    
     * Author:        Arnon Peleg
     * Date:          July 2009
    /**********************************************************************************************/    
    enum CommandState
    {
        CMD_QUEUED,	    // Command was queued, this is the default state
	    CMD_FLUSHED,    // Command was marked to be flushed, but not yet flushed/executed
	    CMD_SUBMITTED,  // Command was submitted to the device or to the host for execution
        CMD_COMPELETED  // When completed, the command is signaled to be completed, but is removed from list in-order
    };

    /**********************************************************************************************
     * Class name:    CommandEntry
     * Description:    
     * Author:        Arnon Peleg
     * Date:          July 2009
    /**********************************************************************************************/    
    struct CommandEntry
    {
	    cl_dev_cmd_desc*    pclCmd;         // Pointer to the command structure
	    CommandState        eState;         // The command state
	    bool                bIsHostCommand; // If true, the command is expected to be execute on host.
        void*               pMappedBufHndls;// Array of mapped buffer that are used by the device to process the command.
        cl_uint             uiNumOfBufHndls;// The length of pMappedBufHndls;
        LrbCommandsList*    pCmdList;       // The list this command is associate with.
    };

    /**********************************************************************************************
     * Class name:    LrbCommandsList
     * Description:    
     * Author:        Arnon Peleg
     * Date:          July 2009
    /**********************************************************************************************/    
    class LrbCommandsList
    {
    public:
        LrbCommandsList( XNWrapper* xnWrapper );
        virtual ~LrbCommandsList();

        cl_int PushCommand(CommandEntry* IN pCmdEntry);
        cl_int PopCommand(CommandEntry** OUT ppCmdEntry);
        cl_int FindCommand( cl_dev_cmd_id IN id, CommandEntry** OUT ppCmdEntry);
        cl_int GetBlockHostCommand(CommandEntry** OUT ppCmdEntry);
        cl_int FlushList();
        cl_int ExecuteFlushedCmds();
        cl_int PurgeList();

        cl_int Release();
        cl_int Retain();
        cl_uint GetReferenceCount() const { return m_uiRefCount; }


    private:
        list<CommandEntry*> m_cmdsList;     // The list of commands
        OclMutex            m_muListLocker; // List access Mutex
        cl_uint             m_uiRefCount;   // Object reference count

        XNWrapper*          m_xnWrapper;
    };

}}};    // Intel::OpenCL::LRBAgent
#endif  // !defined(__LRB_COMMAND_LIST_H__)
