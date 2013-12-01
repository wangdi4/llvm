// Copyright (c) 2008-2013 Intel Corporation
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
//  command_queue.h
//  Implementation of the Class ICommandQueue
//  Created on:      23-Dec-2008 3:23:01 PM
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////

#pragma once

#include <cl_types.h>
#include "queue_event.h"
#include "ocl_command_queue.h"
#include "Context.h"

namespace Intel { namespace OpenCL { namespace Framework {

    //Forward declaration
    class Command;

    /**
     * 
     * 
     */
    class ICommandQueue
    {
    public:
        virtual cl_err_code Enqueue(Command* command)          = 0;
        virtual cl_err_code EnqueueWaitForEvents(Command* wfe) = 0;
        virtual cl_err_code EnqueueMarkerWaitForEvents(Command* marker) = 0;
        virtual cl_err_code EnqueueBarrierWaitForEvents(Command* barrier) = 0;
        virtual cl_err_code WaitForCompletion(const SharedPtr<QueueEvent>& pEvent ) = 0;
        
        virtual cl_err_code Flush(bool bBlocking)  = 0;
        virtual cl_err_code SendCommandsToDevice() = 0;
        virtual cl_err_code NotifyStateChange( const SharedPtr<QueueEvent>& pEvent, OclEventState prevColor, OclEventState newColor ) = 0;
    };

    class IOclCommandQueueBase : public OclCommandQueue, public ICommandQueue
    {
    public:

        PREPARE_SHARED_PTR(IOclCommandQueueBase) 
        virtual cl_err_code EnqueueCommand(Command* pCommand, cl_bool bBlocking, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent);
        virtual cl_err_code EnqueueWaitEvents(Command* wfe, cl_uint uNumEventsInWaitList, const cl_event* cpEventWaitList);
        virtual cl_err_code EnqueueMarkerWaitEvents(Command* cmd, cl_uint uNumEventsInWaitList, const cl_event* pEventWaitList);
        virtual cl_err_code EnqueueBarrierWaitEvents(Command* cmd, cl_uint uNumEventsInWaitList, const cl_event* pEventWaitList);
        virtual cl_err_code WaitForCompletion(const SharedPtr<QueueEvent>& pEvent );            
        virtual ocl_gpa_data* GetGPAData() const { return m_pContext->GetGPAData(); }

        virtual void        AddFloatingDependence(const SharedPtr<QueueEvent>& pCmdEvent) const { pCmdEvent->AddFloatingDependence(); }
        virtual void        RemoveFloatingDependence(const SharedPtr<QueueEvent>& pCmdEvent) const { pCmdEvent->RemoveFloatingDependence(); }

        // manage lifetime slightly differently from another ReferenceCounted objects
        virtual void EnterZombieState( EnterZombieStateLevel call_level );

    protected:

        IOclCommandQueueBase(
            SharedPtr<Context>          pContext,
            cl_device_id                clDefaultDeviceID,
            cl_command_queue_properties clProperties,
            EventsManager*              pEventManager
        ) : OclCommandQueue(pContext, clDefaultDeviceID, clProperties, pEventManager) {}

        virtual ~IOclCommandQueueBase() {}

        virtual  void BecomeVisible();        

    private:

        cl_err_code EnqueueWaitEventsProlog(const SharedPtr<QueueEvent>& pEvent, cl_uint uNumEventsInWaitList, const cl_event* pEventWaitList);
    };
}}}    // Intel::OpenCL::Framework



