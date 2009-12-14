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
//  out_of_order_command_queue.h
//  Implementation of the Class OutOfOrderCommandQueue
//  Created on:      23-Dec-2008 3:23:06 PM
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////

#if !defined(__OUT_OF_ORDER_COMMAND_QUEUE_H__)
#define __OUT_OF_ORDER_COMMAND_QUEUE_H__

#include <cl_types.h>
#include "enqueue_commands.h"
#include "in_order_command_queue.h"

namespace Intel { namespace OpenCL { namespace Framework {
    class QueueEvent;
    class EventsManager;
    class Device;
    class OclCommandQueue;

    /************************************************************************
     * OutOfOrderCommandQueue implements an out-of-order version of the InOrderCommandQueue
     *
     * The following new commands are supported:
     *
     * Marker -
     *  A marker is used to being notified when all previous commands were completed.
     *  Hence, When a marker command is queued, it registers on all non completed events (not black)
     *
     * Barrier -
     *  A Barrier let's all commands that come after it to wait until all command in front of it are
     *  completed. Hence, once a barrier is queued, it need to registers on all non completed events (not black),
     *  just like the marker. In addition, it is needed to be registerd that all commands after it will be dependent
     *  on it. 
     *  A barrier is transitive, to say that register on a previous barrier command menas that you are automaticly
     *  registered on all the command in front of you.
     *
     * WaitOnEvents - 
     *  Acts like barrier, but registered only on a specific list of events. This command is not transitive,
     *  but still all new commands need to be registered on it.
     *
     *  
     * What is deffernt from InOrderCommandQueue?
     * 
     * 1. Flush handling:
     *  When flush entered into the system the direct meaning is that all previous commands
     *  are expected to be issued on the device. 
     *  The actual state may be that commands are internaly dependent inside a queue. 
     *  As a reuslt, the flushing need to be split between.
     *  Hence, a flush command forks 2 states:
     *      1. Flush all ready commands
     *      2. Flush any chunk of ready commands that are ready on completion of other command.
     *         Until all previous commands of the flush were issued.
     *
     *  The implementation concept:
     *      1. On "User" Flush command, a flush counter is incremeants to identify that flush is required.
     *      2. A "User" Flush command is waiting as long as previous commands were not completed.
     *      3. As long as "User" Flush command exists (in the waiting list), the implementation issues
     *         "Internal" Flush command on the ready list in the following occurences:
     *              a. On "User" flush to issue all ready commands.
     *              b. On completion of other event, after all depends have been resolved.
     *          The concept is that ready list is executed in-order, therfore the flush will be executed
     *          only after all other ready commands were issued to the device
     *          Creation of new Internal list means that a new list need to be created and use from this point forward.
     *      4. When "Internal" Flush command is executed, it releases the device list (the actual flush).
     *         
     *      5. When the "User" Flush command is ready, there is no need in this flush cause all commands
     *         were issued and completed. The flush counter is decremented and the command is "deleted"
     *         immidately (move to the black list), this is done through the command execute (SetEventColor(black))
     *
    /************************************************************************/                 
    class OutOfOrderCommandQueue : public InOrderCommandQueue
    {

    public:
        OutOfOrderCommandQueue( EventsManager* pEventsManager, Device* pDevice, OclCommandQueue* pOclCommandQueue );
        virtual ~OutOfOrderCommandQueue();
        
        // All queue policy events are implemented.
        cl_err_code AddCommand(Command* command);
        cl_err_code Flush( bool bBlocking );
        virtual cl_err_code NotifyEventColorChange( 
            const QueueEvent*       cpEvent, 
            QueueEventStateColor    prevColor,  
            QueueEventStateColor    newColor );

    private:
        // members
        QueueEvent* m_pSynchBarrierEvent;   // If not null, any new command (excluding Marker & Barrier) need to be dependent on

        // private methods
        void        ApplyOutOfOrderCommandsPolicy(Command* pCommand);
        void        HandleRuntimeCommandExecuted( Command* pCommand );
        bool        IsReadyDeviceCommands();

        // A queue cannot be copied
        OutOfOrderCommandQueue(const OutOfOrderCommandQueue&);           // copy constructor
        OutOfOrderCommandQueue& operator=(const OutOfOrderCommandQueue&);// assignment operator

    };

}}};    // Intel::OpenCL::Framework
#endif  // !defined(__OUT_OF_ORDER_COMMAND_QUEUE_H__)
