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

#pragma once
///////////////////////////////////////////////////////////
//  events_manager.h
//  Implementation of the Class EventsManager
//  Created on:      23-Dec-2008 3:22:59 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////
#if !defined(__OCL_EVENTS_MANAGER_H__)
#define __OCL_EVENTS_MANAGER_H__

#include <cl_types.h>

namespace Intel { namespace OpenCL { namespace Framework {
    // Forward declrations 
    class OCLObjectsMap;
    class QueueEvent;
    class OclEvent;
    class OclCommandQueue;

    /**********************************************************************************************
     * Class name:    EventsManager
     *
     * Description:    
     *      TODO
     *
     * Author:        Arnon Peleg
     * Date:        December 2008
    /**********************************************************************************************/    
    class EventsManager
    {
    public:
        EventsManager();
        virtual ~EventsManager();

        // OpenCL API Event related functions
        cl_err_code RetainEvent  (cl_event event);
        cl_err_code ReleaseEvent (cl_event event);
        cl_err_code GetEventInfo (cl_event event , cl_int iParamName, size_t szParamValueSize, void* pParamValue, size_t* pszParamValueSizeRet);
        cl_err_code GetEventProfilingInfo (cl_event event, cl_profiling_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet);
		cl_err_code WaitForEvents(cl_uint uiNumEvents, const cl_event* eventList );

        // Event handling functions
        QueueEvent* CreateEvent(cl_command_type eventCommandType, cl_event* pEventHndl, OclCommandQueue* pOclCommandQueue);
        cl_err_code RegisterEvents(QueueEvent* pEvent, cl_uint uiNumEvents, const cl_event* eventList, bool bRemoveEvents = false, cl_command_queue queueId = 0);
        cl_err_code ValidateEventsContext(cl_uint uiNumEvents, const cl_event* eventList, cl_context* pclEventsContext);

    private:
        OCLObjectsMap* m_pEvents;   // Holds the set of clEvents that exist.

        // Private handling functions
        OclEvent** GetEventsFromList( cl_uint uiNumEvents, const cl_event* eventList );


        // An EventManger object cannot be copied
        EventsManager(const EventsManager&);           // copy constructor
        EventsManager& operator=(const EventsManager&);// assignment operator

    };

}}};    // Intel::OpenCL::Framework
#endif  // !defined(__OCL_EVENTS_MANAGER_H__)
