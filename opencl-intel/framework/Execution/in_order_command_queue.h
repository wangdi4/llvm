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

        PREPARE_SHARED_PTR(InOrderCommandQueue)

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
