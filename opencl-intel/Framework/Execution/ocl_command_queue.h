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

#if !defined(__OCL_OCL_COMMAND_QUEUE_H__)
#define __OCL_OCL_COMMAND_QUEUE_H__

#include <cl_types.h>
#include <logger.h>
#include "event_done_observer.h"
#include "command_receiver.h"
#include "cl_object.h"

namespace Intel { namespace OpenCL { namespace Framework {

    // Forward declrations
    class Context;
    class Device;
    class QueueWorkerThread;
    class EventsManager;
    class ICommandQueue;
    class Command;

	/**********************************************************************************************
	 * Class name:	OclCommandQueue
	 *
	 * Description:	
     *
            The command-queue can be used to queue a set of operations (referred to as commands) in order. Having multiple
            command-queues allows applications to queue multiple independent commands without
            requiring synchronization. Note that this should work as long as these objects are not being
            shared. Sharing of objects across multiple command-queues will require the application to
            perform appropriate synchronization

     *
	 * Author:		Arnon Peleg
	 * Date:		December 2008
	/**********************************************************************************************/	
    class OclCommandQueue : public IEventDoneObserver, public ICommandReceiver, public OCLObject
    {

    public:
	    OclCommandQueue(
            Context*                    pContext, 
            cl_device_id                clDefaultDeviceID, 
            cl_command_queue_properties clProperties
            );
	    virtual         ~OclCommandQueue();
        cl_err_code     Clean();        // The Clean signals the queue to clean himself peacefully and to release itself.
        cl_err_code     Initialize();   // Starts the queue. assign a working thread.
        cl_err_code     GetInfo( cl_command_queue_info clParamName, size_t szParamValueSize, void* pParamValue, size_t* pszParamValueSizeRet );	    
        cl_err_code     SetProperties(cl_command_queue_properties clProperties, cl_bool bEnable, cl_command_queue_properties* clOldProperties);

        /////////////////////////////////////////////
	    cl_err_code     EnqueueCommand(Command* command, cl_bool blocking, const cl_event event_wait_list, cl_uint num_events_in_wait_list, cl_event pEvent);
	    cl_err_code     SetMarker(cl_event pEvent);
	    cl_err_code     SetBarrier();

        // Implement ICommandReceiver functions.
	    void            EnqueueDevCommands();
	    void            PushFrontCommand();
        // Implement IEventDoneObserver functions.
        cl_err_code     NotifyEventDone(QueueEvent* event);

        cl_bool         IsProfilingEnabled() const              { return m_bProfilingEnabled;  }
        cl_bool         IsOutOfOrderExecModeEnabled() const     { return m_bOutOfOrderEnabled; }

    private:
        cl_err_code     ResolvedSynchEvents(cl_command_type commandType, QueueEvent* newEvent);

        // Private memebers
        Context*            m_pContext;
	    Device*             m_pDefaultDevice;
        cl_device_id        m_clDefaultDeviceId;
        cl_bool             m_bProfilingEnabled;
        cl_bool             m_bOutOfOrderEnabled;
	    QueueWorkerThread*  m_pQueueWorkerThread;
		ICommandQueue*      m_pCommandQueue;
        Intel::OpenCL::Utils::LoggerClient* m_pLoggerClient; // Logger client for logging operations.
    };
}}};    // Intel::OpenCL::Framework
#endif  // !defined(__OCL_OCL_COMMAND_QUEUE_H__)
