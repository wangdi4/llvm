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
//  events_manager.h
//  Implementation of the Class EventsManager
//  Created on:      23-Dec-2008 3:22:59 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////
#pragma once
#include "cl_framework.h"
#include "cl_objects_map.h"
#include "event_callback.h"
#include <list>

namespace Intel { namespace OpenCL { namespace Framework {
    // Forward declarations 
    template <class HandleType, class ObjectType> class OCLObjectsMap;
    class OclEvent;
    class BuildEvent;
    class QueueEvent;
    class UserEvent;
    class IOclCommandQueueBase;

    /**********************************************************************************************
     * Class name:    EventsManager
     *
     * Description:    
     *      TODO
     *
     * Author:        Arnon Peleg
     * Date:        December 2008
    **********************************************************************************************/    
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
        bool IsValidEventList(cl_uint uiNumEvents, const cl_event* eventList, std::vector<SharedPtr<OclEvent> >* pvOclEvents = NULL);

        // Event handling functions
        template<class EventClass>
        SharedPtr<EventClass> CreateEventClass(_cl_context_int* context)
            {
                SharedPtr<EventClass> pEvent = EventClass::Allocate(context);
                m_mapEvents.AddObject(pEvent);
                return pEvent;
            }

        template<class EventClass>
            SharedPtr<EventClass> GetEventClass(cl_event clEvent)
            {
                SharedPtr<OCLObject<_cl_event_int> > pOclObject = m_mapEvents.GetOCLObject((_cl_event_int*)clEvent);
                if (0 == pOclObject)
                {
                    return NULL;
                }
                return pOclObject.DynamicCast<EventClass>();
            }

        // return list of existing events. Assumes DisableNewEvents() was called before otherwise races may happen
        template<class EventClass>
            void GetAllEventClass(std::list<SharedPtr<EventClass> >& out_list)
            {
                GetAllEventClassFunction<EventClass> filter( out_list );
                m_mapEvents.ForEach( filter );
            }

        void        RegisterQueueEvent(const SharedPtr<QueueEvent>& pEvent, cl_event* pEventHndl);
        cl_err_code RegisterEvents(const SharedPtr<OclEvent>& pEvent, cl_uint uiNumEvents, const cl_event* eventList, bool bRemoveEvents = false, cl_int queueId = 0);

        void        ReleaseAllEvents( bool bTerminate );
        void        DisableNewEvents() { m_mapEvents.DisableAdding(); };
        void        EnableNewEvents() { m_mapEvents.EnableAdding(); };
        void        SetPreserveUserHandles() { m_mapEvents.SetPreserveUserHandles(); }

        cl_err_code SetEventCallBack(cl_event evt, cl_int execType, eventCallbackFn fn, void* pUserData);

    private:
        OCLObjectsMap<_cl_event_int> m_mapEvents;     // Holds the set of clEvents that exist.

        // Private handling functions
        bool GetEventsFromList( cl_uint uiNumEvents, const cl_event* eventList, std::vector<SharedPtr<OclEvent> >* pvOclEvents );

        template<class EventClass> class GetAllEventClassFunction
        {
        public:
            GetAllEventClassFunction( std::list<SharedPtr<EventClass> >& out_list ) : m_out_list(out_list) {};

            bool operator()(const SharedPtr<OCLObject<_cl_event_int> >& obj)
            {
                EventClass* pEventClass = dynamic_cast<EventClass*>(obj.GetPtr());
                if (NULL != pEventClass)
                {
                    m_out_list.push_back( pEventClass );
                }             
                return true;
            }

        private:
            std::list<SharedPtr<EventClass> >& m_out_list;
        };

        // An EventManger object cannot be copied
        EventsManager(const EventsManager&);           // copy constructor
        EventsManager& operator=(const EventsManager&);// assignment operator

    };

}}}    // Intel::OpenCL::Framework
