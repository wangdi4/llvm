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
//  ocl_command_queue.h
//  Implementation of the Class OclCommandQueue
//  Created on:      23-Dec-2008 3:23:05 PM
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////

#if !defined(EA_FB595B53_73A0_449e_AEBE_A0575CD3E5FF__INCLUDED_)
#define EA_FB595B53_73A0_449e_AEBE_A0575CD3E5FF__INCLUDED_

#include <cl_framework.h>
#include "event_done_observer.h"
#include "command_receiver.h"

namespace Intel { namespace OpenCL { namespace Framework {

    // Forward declrations
    class Context;
    class Device;
    class QueueWorkingThread;
    class EventsManager;
    class ICommandQueue;
    class Command;

    class OclCommandQueue : public IEventDoneObserver, public ICommandReceiver
    {

    public:
	    OclCommandQueue(
            Context*                    context, 
            Device*                     device, 
            cl_command_queue_properties properties
            );
	    virtual ~OclCommandQueue();

	    cl_err_code     GetInfo(cl_command_queue_info param_name, size_t param_value_size, void * param_value, size_t* param_value_size_ret);
	    cl_err_code     SetProperties(cl_command_queue_properties properties, cl_command_queue_properties* old_properties);
	    cl_err_code     EnqueueCommand(Command* command, cl_bool blocking, const cl_event event_wait_list, cl_uint num_events_in_wait_list, cl_event pEvent);
	    cl_err_code     SetMarker(cl_event pEvent);
	    cl_err_code     SetBarrier();
        // Implement ICommandReceiver functions.
	    void            EnqueueDevCommands();
	    void            PushFrontCommand();
        // Implement IEventDoneObserver functions.
        cl_err_code     NotifyEventDone(QueueEvent* event) = 0;

    private:
        cl_err_code         ResolvedSynchEvents(cl_command_type commandType, QueueEvent* newEvent);

        // Private memebers
	    cl_command_queue    m_clCommandQueue;
        Context*            m_context;
	    Device*             m_device;
	    QueueWorkingThread* m_queueWorkingThread;
	    EventsManager*      m_eventsManager;
	    ICommandQueue*      m_IcommandQueue;
    };
}}};    // Intel::OpenCL::Framework
#endif  // !defined(EA_FB595B53_73A0_449e_AEBE_A0575CD3E5FF__INCLUDED_)
