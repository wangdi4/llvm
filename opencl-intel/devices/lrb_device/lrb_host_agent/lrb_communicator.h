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
//  lrb_communicator.h
//  Implementation of the Class LrbCommandsList
//  Created on:      27-Jul-2009 1:02:41 PM
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////
#if !defined(__LRB_COMMUNICATOR_H__)
#define __LRB_COMMUNICATOR_H__

#include <cl_thread.h>
#include <cl_device_api.h>

namespace Intel { namespace OpenCL { namespace LRBAgent {

    // Forward declarations
    class   XNWrapper;
    class   LrbCommandExecuter;
    class   LrbProgramService;
    struct  CommandDoneMessage;

    /**********************************************************************************************
    * Class name:    LrbCommunicator
    *
    * Description:    
    *  - Listen to commands change status on the device
    *       - Resolve list internal dependencies as a result of memory transaction.
    *       - Signal Memory Transactor to maintain order
    *       - Serve all command lists in the process
    *  - Use XN0Message Send/Receive Async API
    *  - Notify command status change to the runtime
    *  - Run in its own worker thread
    *
    * Author:        Arnon Peleg
    * Date:          Aug. 2009
    /**********************************************************************************************/    

    class LrbCommunicator : public Intel::OpenCL::Utils::OclThread
    {
    public:
        LrbCommunicator( XNWrapper* pXnWarrper, LrbCommandExecuter* pCmdExecuter, LrbProgramService* pProgService, fn_clDevCmdStatusChanged*	pclDevCmdStatusChanged);
        ~LrbCommunicator();
        
    private:
        int Run();                  // The actual thread running loop.
        int GetNextMessage( CommandDoneMessage* pReceivedMessage);

        // Private members
        XNWrapper*                  m_pXnWarraper;
        LrbCommandExecuter*         m_pCmdExecuter;
        LrbProgramService*          m_pProgService;
        fn_clDevCmdStatusChanged*	m_pclDevCmdStatusChanged;
    };

}}};    // Intel::OpenCL::LRBAgent
#endif // !defined(__LRB_COMMUNICATOR_H__)
