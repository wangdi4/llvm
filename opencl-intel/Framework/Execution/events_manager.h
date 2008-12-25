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
//  events_manager.h
//  Implementation of the Class EventsManager
//  Created on:      23-Dec-2008 3:22:59 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////
#if !defined(__OCL_EVENTS_MANAGER_H__)
#define __OCL_EVENTS_MANAGER_H__

#include <cl_framework.h>

namespace Intel { namespace OpenCL { namespace Framework {
    // Forward declrations 
    class OCLObjectsMap;
    class QueueEvent;
    struct HndlsList;

    class EventsManager
    {
    public:
	    EventsManager();
	    virtual ~EventsManager();

	    cl_int      RegisterEvents(QueueEvent* event,  HndlsList* event_wait_list);
	    QueueEvent* CreateEvent(cl_command_type eventCommandType, cl_command_queue eventQueueHndl, cl_event pEventHndl);
	    void        EventStatusChange(cl_event eventId, cl_int commandStatus);
	    void        WaitForEvents(cl_uint num_events, const cl_event event_list);

    private:
   	    OCLObjectsMap* m_OclObjectsMap;

    };

}}};    // Intel::OpenCL::Framework
#endif  // !defined(__OCL_EVENTS_MANAGER_H__)
