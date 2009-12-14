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
//  EventsManager.cpp
//  Implementation of the Class EventsManager
//  Created on:      23-Dec-2008 3:22:59 PM
//  Original author: Peleg, Arnon
//////////////////////////////////////////////////////////
#include "events_manager.h"
#include "ocl_event.h"
#include "queue_event.h"
#include "cl_objects_map.h"
#include "ocl_command_queue.h"
#include "cl_utils.h"

using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;

/******************************************************************
 *
 ******************************************************************/
EventsManager::EventsManager() 
{
    m_pEvents = new OCLObjectsMap();
}

/******************************************************************
 *
 ******************************************************************/
EventsManager::~EventsManager()
{
     // Need to clean event list. There is a possability that
    // the manager is delated before all events had been released.
    cl_uint     uiEventCount = m_pEvents->Count();
    for (cl_uint ui=0; ui<uiEventCount; ++ui)
    {
        OCLObject*  pOclObject = NULL;
        OclEvent*   pEvent  = NULL;
        cl_err_code res     = CL_SUCCESS;
        res = m_pEvents->GetObjectByIndex(ui, &pOclObject);
        if (CL_SUCCEEDED(res))
        {    
            pEvent = dynamic_cast<OclEvent*>(pOclObject);
            delete pEvent;
        }
    }
    m_pEvents->Clear();
    delete m_pEvents;
}


/******************************************************************
 *
 ******************************************************************/
cl_err_code EventsManager::RetainEvent (cl_event clEvent)
{   
    cl_err_code res     = CL_SUCCESS;

    OCLObject*  pOclObject = NULL;
    res = m_pEvents->GetOCLObject((cl_uint)clEvent, &pOclObject);
    if (CL_SUCCEEDED(res))
    {
        OclEvent* pEvent = dynamic_cast<OclEvent*>(pOclObject);
        pEvent->Retain();
    }
    return res;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code EventsManager::ReleaseEvent(cl_event clEvent)
{
    cl_err_code res     = CL_SUCCESS;
    OCLObject*  pOclObject = NULL;

    res = m_pEvents->GetOCLObject((cl_uint)clEvent, &pOclObject);
    if (CL_SUCCEEDED(res))
    {
        OclEvent* pEvent = dynamic_cast<OclEvent*>(pOclObject);
        pEvent->Release();
        if ( 0 == pEvent->GetReferenceCount() )
        {
            // Remove this event completely,
            // The full meaning of remove act is that the event can not be accessed by
            // the user anymore, but it related command may still running.
            // TODO: Arnon API Q: should we delete only when the command is done?
            m_pEvents->RemoveObject((cl_uint)clEvent, NULL);
            delete pEvent;
        }
    }
    return res;
}

/******************************************************************
 *
 ******************************************************************/              
cl_err_code    EventsManager::GetEventInfo(cl_event clEvent , cl_int iParamName, size_t szParamValueSize, void * paramValue, size_t * szParamValueSizeRet)
{
    cl_err_code res         = CL_SUCCESS;
    OCLObject*  pOclObject  = NULL;

    res = m_pEvents->GetOCLObject((cl_uint)clEvent, &pOclObject);
    if (CL_SUCCEEDED(res))
    {
        OclEvent*   pEvent  = dynamic_cast<OclEvent*>(pOclObject);
        res = pEvent->GetInfo(iParamName, szParamValueSize, paramValue, szParamValueSizeRet);
    }   
    else
    {
        res = CL_INVALID_EVENT;
    }
    return res;
}
/******************************************************************
 *
 ******************************************************************/
cl_err_code EventsManager::GetEventProfilingInfo (cl_event clEvent, cl_profiling_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet)
{
    cl_err_code res         = CL_SUCCESS;
    OCLObject*  pOclObject  = NULL;

    res = m_pEvents->GetOCLObject((cl_uint)clEvent, &pOclObject);
    if (CL_SUCCEEDED(res))
    {
        OclEvent*   pEvent  = dynamic_cast<OclEvent*>(pOclObject);
        res = pEvent->GetProfilingInfo(clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);
    }   
    else
    {
        res = CL_INVALID_EVENT;
    }
    return res;
}

/******************************************************************
 * This function implemnts the API clWaitForEvents function
 * The host thread waits for commands identified by event objects in event_list to complete. 
 * The events specified in event_list act as synchronization points.
 *
 ******************************************************************/
cl_err_code EventsManager::WaitForEvents(cl_uint uiNumEvents, const cl_event* eventList)
{
	cl_start;
    if ( 0 == uiNumEvents || NULL == eventList )
         return CL_INVALID_EVENT_WAIT_LIST;

    // First validate that all ids in the event list exists
    OclEvent** vOclEvents = GetEventsFromList(uiNumEvents, eventList);
    if( NULL == vOclEvents)
        return CL_INVALID_EVENT_WAIT_LIST;

    // Wait on all events. Order doesn't matter since you always bonded to the longest event.
    // OclEvent wait on event that is done do nothing    
    for ( cl_uint ui = 0 ; ui < uiNumEvents; ui++)
    {
        vOclEvents[ui]->Wait();
    }
    
    delete[] vOclEvents;  
    cl_return CL_SUCCESS;
}

/******************************************************************
 * This function validates that all events in eventList has the same context 
 * If not CL_INVALID_CONTEXT is returned. On sucess CL_SUCCESS is returned
 * 
 ******************************************************************/
cl_err_code EventsManager::ValidateEventsContext(cl_uint uiNumEvents, const cl_event* eventList, cl_context* pclEventsContext)
{
    // First validate that all ids in the event list exists
    OclEvent** vOclEvents = GetEventsFromList(uiNumEvents, eventList);
    if( NULL == vOclEvents || 0 == uiNumEvents)
    {
        return CL_INVALID_EVENT;
    }

    cl_uint ui;

    // Next, check that events has the same context as the queued context
    cl_context queueContext = vOclEvents[0]->GetContextId();
    for ( ui =1; ui < uiNumEvents; ui++)
    {
        OclEvent* pOclEvent = vOclEvents[ui];
        if( queueContext != pOclEvent->GetContextId())
        {
            // Error
            delete[] vOclEvents;
            return CL_INVALID_CONTEXT;
        }
    }
    // Success
    delete[] vOclEvents;
    *pclEventsContext = queueContext;
    return CL_SUCCESS;
}

/******************************************************************
 * This function gets an handle to event that represents command
 * that is dependent on the command that are attached with the list
 * of events in the event list.
 * If bRemoveEvents is true, events that were allocated on queueId a
 * are removed from the list
 * 
 ******************************************************************/
cl_err_code EventsManager::RegisterEvents(QueueEvent* pEvent, cl_uint uiNumEvents, const cl_event* eventList, bool bRemoveEvents, cl_command_queue queueId)
{
	cl_start;
    // Check input pramaters
    if ( ( NULL == pEvent) ||
         ( (NULL == eventList) && ( 0 != uiNumEvents ) ) ||
         ( (NULL != eventList) && ( 0 == uiNumEvents ) )
         )
         return CL_INVALID_EVENT_WAIT_LIST;

    // If 0, no event list
    if (0 != uiNumEvents)
    {
        // First validate that all ids in the event list exists
        OclEvent** vOclEvents = GetEventsFromList(uiNumEvents, eventList);
        if( NULL == vOclEvents)
            return CL_INVALID_EVENT_WAIT_LIST;

        cl_uint ui;
        // Next, check that events has the same context as the queued context
        cl_context queueContext = pEvent->GetEventQueue()->GetContextId();
        for ( ui =0; ui < uiNumEvents; ui++)
        {
            OclEvent* pOclEvent = vOclEvents[ui];
            if( queueContext != pOclEvent->GetContextId())
            {
                // Error
                delete[] vOclEvents;
                return CL_INVALID_CONTEXT;
            }
        }
        
        // Register input event in the entire eventList
        // If bRemoveEvents is true, events that were allocated on queueId will not
        // be registered
        for ( ui =0; ui < uiNumEvents; ui++)
        {
            QueueEvent* pDependOnEvent = vOclEvents[ui]->GetQueueEvent();            
            if ( NULL != pDependOnEvent )
            {
                if(bRemoveEvents)
                {
                    if ( queueId == (cl_command_queue)(pDependOnEvent->GetEventQueue()->GetId()))
                    {
                        // Do not register
                        continue;
                    }
                }
                pEvent->SetDependentOn(pDependOnEvent);
            }        
        }
        delete[] vOclEvents;
    }
    cl_return  CL_SUCCESS;    
}

/******************************************************************
 * This function returns list of OclEvent objects corresponding
 * to the input parapters of eventId.
 * If one or more of the eventId in the eventList is not valid, NULL
 * is return.
 * On success a list is returned that need to be free by the caller.
 * 
 ******************************************************************/
OclEvent** EventsManager::GetEventsFromList( cl_uint uiNumEvents, const cl_event* eventList )
{
    cl_err_code res         = CL_SUCCESS;
    OCLObject*  pOclObject  = NULL;
	if(0 == uiNumEvents)
	{
		return NULL;
	}
    OclEvent**  vpOclEvents = new OclEvent*[uiNumEvents];

    for ( cl_uint ui = 0; ui < uiNumEvents; ui++)
    {
        res = m_pEvents->GetOCLObject((cl_int)(eventList[ui]), &pOclObject);
        vpOclEvents[ui] = dynamic_cast<OclEvent*>(pOclObject);
        if(CL_FAILED(res))
        {
            // Not valid, return
            delete[] vpOclEvents;
            return NULL;
        }
    } 
    return vpOclEvents;    
}

/******************************************************************
 * This function creates event object. The Ocl event hadnle that can be used by
 * the user is assigned into the eventHndl.
 * The function returns pointer to a QueueEvent that is attached with the related
 * Command object.
 ******************************************************************/
QueueEvent* EventsManager::CreateEvent(cl_command_type eventCommandType, cl_event* pEventHndl, OclCommandQueue* pOclCommandQueue)
{
	cl_start;
    QueueEvent* pNewEvent = new QueueEvent(pOclCommandQueue);
    if ( (NULL != pNewEvent) && (NULL != pEventHndl) )
    {
        OclEvent* pNewOclEvent = new OclEvent(pNewEvent, eventCommandType);
        // TODO: guard ObjMap... better doing so inside the map
        m_pEvents->AddObject(pNewOclEvent);
        *pEventHndl = (cl_event)pNewOclEvent->GetId();
        // Register the OclEvent on the QueueEvent
        pNewEvent->RegisterEventDoneObserver(pNewOclEvent);        
    }
    cl_return  pNewEvent;
}
