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
//  lrb_mem_trabsactor.h
//  Implementation of the Class LrbCommandsList
//  Created on:      Aug-2009
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////
#if !defined(__LRB_MEM_TRANSACTOR_H__)
#define __LRB_MEM_TRANSACTOR_H__

#include <cl_thread.h>
#include <cl_device_api.h>
#include <cl_synch_objects.h>
#include <list>

using namespace Intel::OpenCL::Utils;

namespace Intel { namespace OpenCL { namespace LRBAgent {

    // Forward declarations
    class XNWrapper;
    struct CommandEntry;
    class LrbMemoryManager;

    /**********************************************************************************************
    * Class name:    LrbMemTransactor
    *
    * Description:    
    *  - The Memory transactor worker thread is responsible to execute memory transaction commands.
    *    Each Read/Write commands are blocking and uses the XNWrapper API to Read/Write
    *    The thread run function is waiting on TrasactionsQueue of host commands (Read/Write) to execute them
    *    on the host.
    *
    * Author:        Arnon Peleg
    * Date:          Aug. 2009
    /**********************************************************************************************/    
    class LrbMemTransactor : public OclThread
    {
    public:
        LrbMemTransactor( LrbMemoryManager* pMemoryManager);
        ~LrbMemTransactor();

        int             AddTransaction(CommandEntry* pCommand);     
        int             Signal()                                   { return (int)m_cond.Signal();}
        int             CancelProcessing();

    private:
        int             Run();                  // The actual thread running loop.
        CommandEntry*   GetNextTransaction();

        // Private members
        LrbMemoryManager*   m_pMemoryManager;

        // Transaction queues
        OclCondition        m_cond;             // Condition variable that is used to block calls to GetNextTransaction.
        OclMutex            m_commandsLocker;   // Lock/Unlock access to the queue elements
        std::list<CommandEntry*> m_cmdsList;         // List of commands that are ready for processing.
    };

}}};    // Intel::OpenCL::LRBAgent
#endif // !defined(__LRB_MEM_TRANSACTOR_H__)
