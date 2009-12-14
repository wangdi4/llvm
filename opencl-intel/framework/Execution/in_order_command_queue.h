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
//  in_order_command_queue.h
//  Implementation of the Class InOrderCommandQueue
//  Created on:      23-Dec-2008 3:23:02 PM
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////

#if !defined(__OCL_IN_ORDER_COMMAND_QUEUE_H__)
#define __OCL_IN_ORDER_COMMAND_QUEUE_H__

#include <cl_types.h>
#include <logger.h>
#include <cl_synch_objects.h>
#include "command_queue.h"
#include <list>

using namespace std;
using namespace Intel::OpenCL::Utils;

namespace Intel { namespace OpenCL { namespace Framework {

    class Device;
    class EventsManager;
    class IEventColorChangeObserver;
	class Context;

    /************************************************************************
     * InOrderCommandQueue is an ICommandQueue that implementes the InOrder queue policy
     * as it defined in the openCL spec. 
     * This implementation include Flush/Finish support
     *      
     * The queue gets notification whenever a command in the queue changes its state.
     * As a result, the queue update the state of its queues.
     *
     * The list are ordered. Hence, there is no implicit dependency between consecutive commands.
     *
     * The flush policy of this queue is to enqueue each ready command to a device list. 
     * When flush/finish command is identified, a new list is created and from that point onward 
     * command will be routed to the new list. When the flush command is ready to execute in the
     * in-order order, the previous list will be released and a true flush will be done by the 
     * device agent.
     *
    /************************************************************************/ 
    class InOrderCommandQueue : public ICommandQueue
    {

    public:
        InOrderCommandQueue(EventsManager* pEventsManager, Device* pDevice, OclCommandQueue* pOclCommandQueue);
        virtual ~InOrderCommandQueue();

        // ICommandQueue methods
        virtual cl_err_code Init()  { return CL_SUCCESS; }  // Nothing to do, lists are initialized on creation 
        virtual Command*    GetNextCommand();
        virtual cl_err_code AddCommand(Command* command);
        virtual cl_err_code Flush( bool bBlocking );
        virtual void        Signal();
        virtual bool        IsEmpty();
        virtual cl_uint     Size();
        virtual cl_err_code Release();

        virtual cl_err_code NotifyEventColorChange( 
            const QueueEvent*       cpEvent, 
            QueueEventStateColor    prevColor,  
            QueueEventStateColor    newColor );


    protected:
        // Members
        void*                       m_clCurrentDevCmdList; 
        Device*                     m_pDevice;
		Context*					m_pContext;
        EventsManager*              m_pEventsManager;
        OclCommandQueue*            m_pOclCommandQueue;
        IEventColorChangeObserver*  m_pColorChangeObserver; // Observer to be used by internal events
        OclCondition                m_cond;                 // Condition variable that is used to block calls to GetNextCommand.
        OclMutex                    m_listsLocker;          // Lock/Unlock access to the queue elements
        bool                        m_bReleased;            // Set to true on Release function. On release GetNext returns immediately.
        bool                        m_bIsPrevCmdFlush;      // Set to true if a flush command was entered. Else false
        cl_uint                     m_uiFlushCnt;           // Counts the times a flush has entered to the queue.
        cl_uint                     m_uiCmdListCnt;         // Counts the number of command that have already been assigned with the current cmdList

        // The queue is built from the following 5 lists:
        list<Command*>  m_waitingCmdsList;      // 1. List of commands that are still dependent on others.
        list<Command*>  m_readyCmdsList;        // 2. List of commands that can be issued on the device.
        list<Command*>  m_deviceCmdsList;       // 3. List of commands that are executed on the device.
        list<Command*>  m_runtimeCmdsList;      // 4. List of commands that are ready to be executed in the runtime scope only.
        list<Command*>  m_blackCmdsList;        // 5. List of commands that are done and can be deleted.
        
        // List lockers, to manage direct access to specific list
        OclMutex        m_blackCmdsListLocker;  // Lock access to the black list


        // Logger client for logging operations. DEBUGGING
		DECLARE_LOGGER_CLIENT;
       

        // Private functions
        void        PushInternalFlush();
        bool        CleanBlackList();  // Garbage collector, cleans all black commands
        void        ClearCommandList( list<Command*>& cmdList ); // Clean list of cmds immediately.
        Command*    RemoveFromCommandList( const QueueEvent* cpEvent, list<Command*>& cmdList ); // remove a command accurding to its event
        cl_err_code FlushDone(void* clDevCmdList);
        cl_err_code Finish();

        void        SetDependentOnAll( QueueEvent* pEvent );
        bool        SetDependentOnAllCommandsInList( QueueEvent* pEvent, list<Command*>& cmdList );
        bool        SetDependentLast( QueueEvent* pEvent, list<Command*>& cmdList );
        void        HandleOutOfOrderCommands(Command* pCommand);
        bool        IsReadyDeviceCommands();


        // A queue cannot be copied
        InOrderCommandQueue(const InOrderCommandQueue&);           // copy constructor
        InOrderCommandQueue& operator=(const InOrderCommandQueue&);// assignment operator
    };

}}};    // Intel::OpenCL::Framework
#endif  // !defined(__OCL_IN_ORDER_COMMAND_QUEUE_H__)
