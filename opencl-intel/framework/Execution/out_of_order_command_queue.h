// INTEL CONFIDENTIAL
//
// Copyright 2008-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

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

        PREPARE_SHARED_PTR(OutOfOrderCommandQueue)
    
        static SharedPtr<OutOfOrderCommandQueue> Allocate(
            SharedPtr<Context>          pContext,
			cl_device_id                clDefaultDeviceID, 
			cl_command_queue_properties clProperties,
			EventsManager*              pEventManager)
        {
            return SharedPtr<OutOfOrderCommandQueue>(new OutOfOrderCommandQueue(pContext, clDefaultDeviceID, clProperties, pEventManager));
        }		
		
		~OutOfOrderCommandQueue();

		cl_err_code Initialize();
        long Release();
		virtual cl_err_code Enqueue(Command* cmd);
		virtual cl_err_code EnqueueWaitForEvents(Command* cmd);
        virtual cl_err_code EnqueueMarkerWaitForEvents(Command* marker);
        virtual cl_err_code EnqueueBarrierWaitForEvents(Command* barrier);

		virtual cl_err_code Flush(bool bBlocking);
		virtual cl_err_code NotifyStateChange( const SharedPtr<QueueEvent>& pEvent, OclEventState prevColor, OclEventState newColor);
		// No need for explicit "send commands to device" method, commands are submitted as they become ready
		virtual cl_err_code SendCommandsToDevice() {return CL_SUCCESS; }

	protected:

        OutOfOrderCommandQueue(
			SharedPtr<Context>                    pContext,
			cl_device_id                clDefaultDeviceID, 
			cl_command_queue_properties clProperties,
			EventsManager*              pEventManager
			);

		virtual cl_err_code AddDependentOnAll(Command* cmd);

		void                Submit(Command* cmd);

    // At all times, points to a command that depends on everything enqueued since the last time clEnqueueBarrier/Marker was enqueued to this queue
		Intel::OpenCL::Utils::AtomicPointer<Command> m_depOnAll;
		Intel::OpenCL::Utils::AtomicCounter 		 m_commandsInExecution;
		Intel::OpenCL::Utils::SharedPtr<OclEvent>    m_lastBarrier;
		Intel::OpenCL::Utils::OclSpinMutex			 m_muLastBarrer; //TODO: find better way to handle data race on lastBarrier
		// Is meant to optimize away flushes made to an empty queue
		Intel::OpenCL::Utils::AtomicCounter 		 m_unflushedCommands;
	};
}}}    // Intel::OpenCL::Framework
