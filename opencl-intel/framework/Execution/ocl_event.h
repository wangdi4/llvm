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
//  ocl_event.h
//  Implementation of the Class OclEvent
//  Created on:      23-Dec-2008 3:23:05 PM
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////

#if !defined(__OCL_OCL_EVENT_H__)
#define __OCL_OCL_EVENT_H__

#include <cl_types.h>
#include <cl_object.h>
#include <cl_synch_objects.h>
#include "queue_event.h"
#include "event_done_observer.h"

using namespace Intel::OpenCL::Utils;

namespace Intel { namespace OpenCL { namespace Framework {


    /**********************************************************************************************
     * Class name:    QueueEvent
     *
     * Description:    
     *      TODO
     *
     * Author:        Arnon Peleg
     * Date:        December 2008
    /**********************************************************************************************/    
    class OclEvent : public OCLObject, public IEventDoneObserver
    {

    public:
        OclEvent( QueueEvent* queueEvent, cl_command_type commandType);
        virtual ~OclEvent();        
        QueueEvent* GetQueueEvent() { return m_queueEvent;}
        void    Wait();

        cl_context       GetContextId() const { return m_contextID; }
        cl_command_queue GetQueueId() const   { return m_queueID; }

        // OCLObject implementation
        cl_err_code    GetInfo(cl_int iParamName, size_t szParamValueSize, void * paramValue, size_t * szParamValueSizeRet);

		// profiling support
		cl_err_code GetProfilingInfo(cl_profiling_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet);

        // IEventDoneObserver implementation
        cl_err_code NotifyEventDone(QueueEvent* event);

    private:
        cl_int      GetEventCurrentStatus();

        // Private members
        cl_command_type     m_commandType;      // The type of the command that is related to this event
        cl_command_queue    m_queueID;          // The queue ID of the related command
        cl_context          m_contextID;          // The queue ID of the related command
        QueueEvent*         m_queueEvent;       // Pointer to the relevant queue event
        OclCondition        m_eventDoneCond;    // A synch element to wait on until the event is done.

        // concurrent access
        OclMutex            m_eventLocker;

		// profiling information
		SProfilingInfo		m_sProfilingInfo;
		bool				m_bProfilingEnabled;


        // An OclEvent object cannot be copied
        OclEvent(const OclEvent&);           // copy constructor
        OclEvent& operator=(const OclEvent&);// assignment operator

    };

}}};    // Intel::OpenCL::Framework
#endif  // !defined(__OCL_OCL_EVENT_H__)
