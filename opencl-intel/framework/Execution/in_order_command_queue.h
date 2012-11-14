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
//  in_order_command_queue.h
//  Implementation of the Class InOrderCommandQueue
//  Created on:      23-Dec-2008 3:23:02 PM
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////

#pragma once

#include <cl_types.h>
#include <Logger.h>
#include <cl_synch_objects.h>
#include "command_queue.h"
#include "ocl_command_queue.h"
#include "enqueue_commands.h"
#include <list>

using namespace std;

namespace Intel { namespace OpenCL { namespace Framework {

    class Device;
    class EventsManager;
	class Context;

    /************************************************************************
     * InOrderCommandQueue is an ICommandQueue that implements the InOrder queue policy
     * as it defined in the openCL spec. 
     * This implementation include Flush/Finish support
     *      
     * The queue gets notification whenever a command in the queue changes its state.
     * As a result, the queue update the state of its queues.
     *
     * The list are ordered. Hence, there is no implicit dependency between consecutive commands.
    ************************************************************************/ 
	class InOrderCommandQueue : public IOclCommandQueueBase
	{
	public:

        PREPARE_SHARED_PTR(InOrderCommandQueue);

        static SharedPtr<InOrderCommandQueue> Allocate(
            SharedPtr<Context>          pContext,
			cl_device_id                clDefaultDeviceID, 
			cl_command_queue_properties clProperties,
			EventsManager*              pEventManager)
        {
            return SharedPtr<InOrderCommandQueue>(new InOrderCommandQueue(pContext, clDefaultDeviceID, clProperties, pEventManager));
        }
		
		virtual ~InOrderCommandQueue();

		virtual cl_err_code Enqueue(Command* cmd);
		virtual cl_err_code EnqueueWaitForEvents(Command* cmd) {return Enqueue(cmd);}
        virtual cl_err_code EnqueueMarkerWaitForEvents(Command* marker) { return Enqueue(marker); }
        virtual cl_err_code EnqueueBarrierWaitForEvents(Command* barrier) { return Enqueue(barrier); }

		virtual cl_err_code Flush(bool bBlocking);
		virtual cl_err_code NotifyStateChange( const SharedPtr<QueueEvent>& pEvent, OclEventState prevColor, OclEventState newColor);
		virtual cl_err_code SendCommandsToDevice();

	protected:

        InOrderCommandQueue(
			SharedPtr<Context>                    pContext,
			cl_device_id                clDefaultDeviceID, 
			cl_command_queue_properties clProperties,
			EventsManager*              pEventManager
			);

#if defined TBB_BUG_SOLVED
		Intel::OpenCL::Utils::OclConcurrentQueue<CommandSharedPtr<> >	m_submittedQueue;
#else
		Intel::OpenCL::Utils::OclNaiveConcurrentQueue<CommandSharedPtr<> > m_submittedQueue;
#endif
		Intel::OpenCL::Utils::AtomicCounter					m_submittedQueueGuard;
		Intel::OpenCL::Utils::AtomicCounter					m_commandsInExecution;
	};

}}}    
