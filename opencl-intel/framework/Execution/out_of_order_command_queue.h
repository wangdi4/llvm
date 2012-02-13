// Copyright (c) 2008-2012 Intel Corporation
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
#pragma once

#include <cl_types.h>
#include "enqueue_commands.h"
#include "ocl_command_queue.h"
#include "command_queue.h"

namespace Intel { namespace OpenCL { namespace Framework {
    class OclEvent;
    class EventsManager;
    class Device;

    /************************************************************************
     * OutOfOrderCommandQueue implements an OpenCL out of order Command Queue
    ************************************************************************/       
	class OutOfOrderCommandQueue : public IOclCommandQueueBase
	{
	public:
		OutOfOrderCommandQueue(
			Context*                    pContext,
			cl_device_id                clDefaultDeviceID, 
			cl_command_queue_properties clProperties,
			EventsManager*              pEventManager,
			ocl_entry_points *			pOclEntryPoints
			);
		~OutOfOrderCommandQueue();

		cl_err_code Initialize();
		virtual cl_err_code Enqueue(Command* cmd);
		virtual cl_err_code EnqueueWaitForEvents(Command* cmd);
        virtual cl_err_code EnqueueMarkerWaitForEvents(Command* marker);
        virtual cl_err_code EnqueueBarrierWaitForEvents(Command* barrier);

		virtual cl_err_code Flush(bool bBlocking);
		virtual cl_err_code NotifyStateChange( QueueEvent* pEvent, OclEventStateColor prevColor, OclEventStateColor newColor);
		// No need for explicit "send commands to device" method, commands are submitted as they become ready
		virtual cl_err_code SendCommandsToDevice() {return CL_SUCCESS; }

	protected:
		virtual cl_err_code AddDependentOnAll(Command* cmd);

        //Inherited from OCLObject
        virtual void NotifyInvisible();

		void                Submit(Command* cmd);

    // At all times, points to a command that depends on everything enqueued since the last time clEnqueueBarrier/Marker was enqueued to this queue
		Intel::OpenCL::Utils::AtomicPointer<Command> m_depOnAll;
		Intel::OpenCL::Utils::AtomicCounter m_commandsInExecution;
		Intel::OpenCL::Utils::AtomicPointer<Command> m_lastBarrier;
		// Is meant to optimize away flushes made to an empty queue
		Intel::OpenCL::Utils::AtomicCounter m_unflushedCommands;

	};
}}}    // Intel::OpenCL::Framework
