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
//  queue_event.cpp
//  Implementation of the Class QueueEvent
//  Created on:      23-Dec-2008 3:23:06 PM
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////
#include <cassert>
#include "queue_event.h"
#include "command_queue.h"
#include "enqueue_commands.h"
#include "cl_shared_ptr.hpp"
#include <cl_sys_info.h>

// For debugging
#include <assert.h>
#include "cl_utils.h"

using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;

/******************************************************************
*
******************************************************************/
QueueEvent::QueueEvent(const SharedPtr<IOclCommandQueueBase>& cmdQueue) :
    OclEvent(cmdQueue?cmdQueue->GetParentHandle():CL_INVALID_HANDLE),
    m_bProfilingEnabled(false), m_pCommand(NULL), m_pEventQueue(cmdQueue),
    m_pEventQueueHandle(m_pEventQueue->GetHandle()), m_bEverIssuedToDevice(false)
{
    m_sProfilingInfo.m_ulCommandQueued    = 0;
    m_sProfilingInfo.m_ulCommandSubmit    = 0;
    m_sProfilingInfo.m_ulCommandStart     = 0;
    m_sProfilingInfo.m_ulCommandEnd       = 0;

    m_bCommandQueuedValid = false;
    m_bCommandSubmitValid = false;
    m_bCommandStartValid  = false;
    m_bCommandEndValid    = false;

    if (cmdQueue != NULL)
    {
        m_bProfilingEnabled = cmdQueue->IsProfilingEnabled() ? true : false;
        m_pGPAData = cmdQueue->GetGPAData();
    }
    else
    {
        m_pGPAData = NULL;
    }

#if defined(USE_ITT)
    if ((NULL != m_pGPAData) && (m_pGPAData->bUseGPA))
    {
        // unique ID to pass all tasks, and markers.
        m_ittID = __itt_id_make(&m_ittID, (unsigned long long)this);
        __itt_id_create(m_pGPAData->pDeviceDomain, m_ittID);
    }
#endif

}

/******************************************************************
*
******************************************************************/
QueueEvent::~QueueEvent()
{
#if defined(USE_ITT)
    if ((NULL != m_pGPAData) && (m_pGPAData->bUseGPA))
    {
        __itt_id_destroy(m_pGPAData->pDeviceDomain, m_ittID);
    }
#endif
}

/******************************************************************
*
******************************************************************/
cl_err_code QueueEvent::GetInfo(cl_int paramName, size_t paramValueSize, void * paramValue, size_t * paramValueSizeRet) const
{
    cl_err_code res = CL_SUCCESS;
    const void* localParamValue = NULL;
    size_t outputValueSize = 0;
    cl_int eventStatus = CL_QUEUED;
    cl_command_type  cmd_type;
    cl_command_queue cmd_queue;
    cl_context       context;

    switch (paramName)
    {
    case CL_EVENT_COMMAND_QUEUE:
        cmd_queue = GetEventQueueHandle();
        localParamValue = &cmd_queue;
        outputValueSize = sizeof(cl_command_queue);
        break;
    case CL_EVENT_CONTEXT:
        context = GetParentHandle();
        localParamValue = &context;
        outputValueSize = sizeof(cl_context);
        break;
    case CL_EVENT_COMMAND_TYPE:
        if (!m_pCommand)
        {
            //Todo: need to return CL_USER_COMMAND if user event
            return CL_INVALID_VALUE;
        }
        cmd_type        = m_pCommand->GetCommandType();
        localParamValue = &cmd_type;
        outputValueSize = sizeof(cl_command_type);
        break;
    case CL_EVENT_COMMAND_EXECUTION_STATUS:
        eventStatus = GetEventExecState();
        localParamValue = &eventStatus;
        outputValueSize = sizeof(cl_int); 
        break;
    case CL_EVENT_REFERENCE_COUNT:
        localParamValue = &m_uiRefCount;
        outputValueSize = sizeof(cl_uint);
        break;
    default:
        res = CL_INVALID_VALUE;
        break;
    }

    // check param_value_size
    if ( (NULL != paramValue) && (paramValueSize < outputValueSize))
    {
        res = CL_INVALID_VALUE;
    }
    else
    {
        if ( NULL != paramValue )
        {
            MEMCPY_S(paramValue, paramValueSize, localParamValue, outputValueSize);
        }
        if ( NULL != paramValueSizeRet )
        {
            *paramValueSizeRet = outputValueSize;
        }
    }
    return res;
}

/******************************************************************
*
******************************************************************/
cl_err_code QueueEvent::GetProfilingInfo(cl_profiling_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet)
{
    cl_err_code res = CL_SUCCESS;
    void* localParamValue = NULL;
    size_t outputValueSize = 0;
    cl_ulong ulProfilingInfo = 0;

    if (!m_bProfilingEnabled)
    {
        return CL_PROFILING_INFO_NOT_AVAILABLE;
    }

    cl_int eventStatus = GetEventExecState();

    switch (clParamName)
    {
    case CL_PROFILING_COMMAND_QUEUED:
        ulProfilingInfo = m_sProfilingInfo.m_ulCommandQueued;
        break;
    case CL_PROFILING_COMMAND_SUBMIT:
        if (eventStatus > CL_SUBMITTED)
        {
            return CL_PROFILING_INFO_NOT_AVAILABLE;
        }
        ulProfilingInfo = m_sProfilingInfo.m_ulCommandSubmit;
        break;
    case CL_PROFILING_COMMAND_START:
        if (eventStatus > CL_RUNNING)
        {
            return CL_PROFILING_INFO_NOT_AVAILABLE;
        }
        ulProfilingInfo = m_sProfilingInfo.m_ulCommandStart;
        break;
    case CL_PROFILING_COMMAND_END:
        if (eventStatus > CL_COMPLETE)
        {
            return CL_PROFILING_INFO_NOT_AVAILABLE;
        }
        ulProfilingInfo = m_sProfilingInfo.m_ulCommandEnd;
        break;
    default:
        return CL_INVALID_VALUE;
    }

    localParamValue = &ulProfilingInfo;
    outputValueSize = sizeof(cl_ulong);

    // check param_value_size
    if ( (NULL != pParamValue) && (szParamValueSize < outputValueSize))
    {
        res = CL_INVALID_VALUE;
    }
    else
    {
        if ( NULL != pParamValue )
        {
            MEMCPY_S(pParamValue, szParamValueSize, localParamValue, outputValueSize);
        }
        if ( NULL != pszParamValueSizeRet )
        {
            *pszParamValueSizeRet = outputValueSize;
        }
    }
    return res;
}

void QueueEvent::SetProfilingInfo(cl_profiling_info clParamName, cl_ulong ulData)
{
    switch ( clParamName )
    {
    case CL_PROFILING_COMMAND_QUEUED:
        if ( !m_bCommandQueuedValid || (m_sProfilingInfo.m_ulCommandQueued > ulData) )
        {
            m_sProfilingInfo.m_ulCommandQueued = ulData;
            m_bCommandQueuedValid = true;
        }
        break;

    case CL_PROFILING_COMMAND_SUBMIT:
        if ( !m_bCommandSubmitValid || ( m_sProfilingInfo.m_ulCommandSubmit > ulData ))
        {
            m_sProfilingInfo.m_ulCommandSubmit = ulData;
            m_bCommandSubmitValid = true;
        }
        break;

    case CL_PROFILING_COMMAND_START:
        if ( !m_bCommandStartValid || ( m_sProfilingInfo.m_ulCommandStart > ulData ))
        {
            m_sProfilingInfo.m_ulCommandStart = ulData;
            m_bCommandStartValid = true;
        }
        break;

    case CL_PROFILING_COMMAND_END:
        if ( !m_bCommandEndValid || (m_sProfilingInfo.m_ulCommandEnd < ulData))
        {
            m_sProfilingInfo.m_ulCommandEnd = ulData;
            m_bCommandEndValid = true;
        }
        break;
    default:
        break;
    }
}

void QueueEvent::IncludeProfilingInfo( const SharedPtr<QueueEvent>& other )
{
    assert( NULL != other );

    if (other->m_bCommandQueuedValid)
    {
        SetProfilingInfo( CL_PROFILING_COMMAND_QUEUED, other->m_sProfilingInfo.m_ulCommandQueued );
    }

    if (other->m_bCommandSubmitValid)
    {
        SetProfilingInfo( CL_PROFILING_COMMAND_SUBMIT, other->m_sProfilingInfo.m_ulCommandSubmit );
    }

    if (other->m_bCommandStartValid)
    {
        SetProfilingInfo( CL_PROFILING_COMMAND_START,  other->m_sProfilingInfo.m_ulCommandStart );
    }

    if (other->m_bCommandEndValid)
    {
        SetProfilingInfo( CL_PROFILING_COMMAND_END,    other->m_sProfilingInfo.m_ulCommandEnd );
    }
}


/******************************************************************
*
******************************************************************/
cl_err_code QueueEvent::ObservedEventStateChanged(const SharedPtr<OclEvent>& pEvent, cl_int returnCode)
{
    if (CL_SUCCESS > returnCode)
    {
        // in case of error or finish
        unsigned long long ullHostTime = 0;
        if ( m_bProfilingEnabled )
        {
            ullHostTime = Intel::OpenCL::Utils::HostTime();
        }
        m_pCommand->NotifyCmdStatusChanged(0, CL_COMPLETE, returnCode, ullHostTime);
        //Everything else will be handled from the command routine
        return CL_SUCCESS;
    }
    //Else, fall back on regular routine
    return OclEvent::ObservedEventStateChanged(pEvent, returnCode);
}

OclEventState QueueEvent::SetEventState(OclEventState newColor)
{
    OclEventState retval = OclEvent::SetEventState(newColor);

    if (EVENT_STATE_ISSUED_TO_DEVICE == newColor)
    {
        m_bEverIssuedToDevice = true;
    }
    
#if defined(USE_ITT)
    if (EVENT_STATE_READY_TO_EXECUTE == newColor)
    {
        if ((NULL != m_pGPAData) && (m_pGPAData->bUseGPA))
        {
            if (m_pGPAData->cStatusMarkerFlags & ITT_SHOW_SUBMITTED_MARKER)
            {
                #if defined(USE_GPA)
                // Write this data to the thread track
                __itt_set_track(NULL);
                #endif

                char  strMarkerString[ITT_TASK_NAME_LEN];
                SPRINTF_S(strMarkerString, ITT_TASK_NAME_LEN, "Ready To Execute - %s",m_pCommand->GetCommandName());

                __itt_string_handle* pMarker = __itt_string_handle_create(strMarkerString);
                //Due to a bug in GPA 4.0 the marker is within a task
                //Should be removed in GPA 4.1 
                __itt_task_begin(m_pGPAData->pDeviceDomain, m_ittID, __itt_null, m_pGPAData->pMarkerHandle);
                
                __itt_marker(m_pGPAData->pDeviceDomain, m_ittID, pMarker, __itt_marker_scope_global);

                __itt_task_end(m_pGPAData->pDeviceDomain);
            }
        }
    }
#endif

    return retval;
}

void QueueEvent::DoneWithDependencies(const SharedPtr<OclEvent>& pEvent)
{
    if (EVENT_STATE_HAS_DEPENDENCIES == GetEventState())
    {
        SharedPtr<QueueEvent>                pQEvent  = pEvent.DynamicCast<QueueEvent>();
        SharedPtr<IOclCommandQueueBase> pQEventQueue  = NULL;
        if (NULL != pQEvent)
        {
            pQEventQueue = pQEvent->GetEventQueue();
        }

        bool bOOO       = m_pEventQueue->IsOutOfOrderExecModeEnabled();
        bool bSameQueue = ((NULL != pQEventQueue) && (pQEventQueue->GetId() == m_pEventQueue->GetId()));
        if (bSameQueue && !bOOO)
        {
            //If we're both on the same in-order command queue, the other event will flush it so I don't need to
            SetEventState(EVENT_STATE_READY_TO_EXECUTE);
        }
        else
        {
            //Else, I need to notify my queue that I'm ready to execute
            //I need to cache my own event queue since once I set my state to "ready to execute" I may complete and NULLify/deref my queue
            SharedPtr<IOclCommandQueueBase> pMyEventQueue = m_pEventQueue;
            SetEventState(EVENT_STATE_READY_TO_EXECUTE);
            pMyEventQueue->NotifyStateChange(this, EVENT_STATE_HAS_DEPENDENCIES, EVENT_STATE_READY_TO_EXECUTE);
        }
    }
}

void QueueEvent::NotifyComplete(cl_int returnCode /* = CL_SUCCESS */)
{
    NotifyObservers(returnCode);
    m_pEventQueue->NotifyStateChange(this, EVENT_STATE_EXECUTING_ON_DEVICE, EVENT_STATE_DONE);
    //No longer need my queue reference
    m_pEventQueue = NULL;
    MarkAsComplete();
}

void QueueEvent::SetEventQueue(const SharedPtr<IOclCommandQueueBase>& pQueue)
{
    m_pEventQueue = pQueue;
    m_pEventQueueHandle = m_pEventQueue->GetHandle();
}

cl_command_queue QueueEvent::GetQueueHandle() const
{
    return GetEventQueue()->GetHandle();
}
cl_int QueueEvent::GetReturnCode() const
{
    return m_pCommand->GetReturnCode();
}

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
void QueueEvent::Wait()
{
    if ((NULL != m_pGPAData) && (m_pGPAData->bUseGPA))
    {
      static __thread __itt_string_handle* pTaskName = NULL;
      if ( NULL == pTaskName )
      {
        pTaskName = __itt_string_handle_create("QueueEvent::Wait");
      }
      __itt_task_begin(m_pGPAData->pDeviceDomain, __itt_null, __itt_null, pTaskName);
    }

    OclEvent::Wait();
    
    if ((NULL != m_pGPAData) && (m_pGPAData->bUseGPA))
    {
      __itt_task_end(m_pGPAData->pDeviceDomain);
    }
    
}
#endif

void QueueEvent::operator delete(void* p)
{
    QueueEvent* const self = (QueueEvent*)p;
    if (!self->m_pCommand->IsBeingDeleted())
    {
        delete self->m_pCommand;  // this will delete myself as well, since m_pCommand aggregates me
    }
    OclEvent::operator delete(p);
}

void QueueEvent::AddProfilerMarker(const char* szMarkerName, int iMarkerMask)
{
#if defined(USE_ITT)
    if ((NULL != m_pGPAData) && (m_pGPAData->bUseGPA))
    {
        // unique ID to pass all tasks, and markers.
        __itt_id ittID;
        ittID = __itt_id_make(&ittID, (unsigned long long)m_pCommand);
        __itt_id_create(m_pGPAData->pDeviceDomain, ittID);

        if (m_pGPAData->cStatusMarkerFlags & ITT_SHOW_QUEUED_MARKER)
        {
            // TODO: No need to create string each time. Could be created per command.
            //       What about taking kenel name in case of NDRange
            char pMarkerString[ITT_TASK_NAME_LEN];
            SPRINTF_S(pMarkerString, ITT_TASK_NAME_LEN, "%s - %s", szMarkerName, m_pCommand->GetCommandName());

            __itt_string_handle* pMarker = __itt_string_handle_create(pMarkerString);

            //Due to a bug in GPA 4.0 the marker is within a task
            //Should be removed in GPA 4.1
            __itt_task_begin(m_pGPAData->pDeviceDomain, ittID, __itt_null, pMarker);
            __itt_marker(m_pGPAData->pDeviceDomain, ittID, pMarker, __itt_marker_scope_global);
#if 0 // Do we realy need this?
            cl_ushort isBlocking = bBlocking ? 1 : 0;
            __itt_metadata_add(m_pGPAData->pDeviceDomain, ittID, m_pGPAData->pIsBlocking, __itt_metadata_u16, 1, &isBlocking);
            __itt_metadata_add(m_pGPAData->pDeviceDomain, ittID, m_pGPAData->pNumEventsInWaitList, __itt_metadata_u32 , 1, &uNumEventsInWaitList);
#endif
            __itt_task_end(m_pGPAData->pDeviceDomain);
        }

        __itt_id_destroy(m_pGPAData->pDeviceDomain, ittID);
    }
#endif // ITT
}
