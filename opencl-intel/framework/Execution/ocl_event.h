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

#define OCL_EVENT_WAIT_SPIN         0
#define OCL_EVENT_WAIT_YIELD        1
#define OCL_EVENT_WAIT_OS_DEPENDENT 2

//#define OCL_EVENT_WAIT_STRATEGY OCL_EVENT_WAIT_SPIN
#define OCL_EVENT_WAIT_STRATEGY OCL_EVENT_WAIT_YIELD
//#define OCL_EVENT_WAIT_STRATEGY OCL_EVENT_WAIT_OS_DEPENDENT

namespace Intel { namespace OpenCL { namespace Framework {

	class Command;


    /**********************************************************************************************
     * Class name:    QueueEvent
     *
     * Description:    
     *      TODO
     *
     * Author:        Arnon Peleg
     * Date:        December 2008
    /**********************************************************************************************/    
    class OclEvent : public QueueEvent, public OCLObject<_cl_event>
    {

    public:
		OclEvent( IOclCommandQueueBase* cmdQueue, ocl_entry_points * pOclEntryPoints );
        virtual ~OclEvent();        
        void    Wait();

        cl_context       GetContextHandle() const;
        cl_command_queue GetQueueHandle() const;
		// OCLObject implementation
        cl_err_code GetInfo(cl_int iParamName, size_t szParamValueSize, void * paramValue, size_t * szParamValueSizeRet);

		// profiling support
		cl_err_code GetProfilingInfo(cl_profiling_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet);
		void		SetProfilingInfo(cl_profiling_info clParamName, cl_ulong ulData);

	protected:
        cl_int      GetEventCurrentStatus();
		//Some implementations of possible methods for waiting 
		void        WaitSpin();
		void        WaitYield();
		void        WaitOSEvent();
#if OCL_EVENT_WAIT_STRATEGY == OCL_EVENT_WAIT_OS_DEPENDENT
		//Overriding the event done implementation for OS Wait support
		virtual cl_err_code NotifyEventDone(QueueEvent* pEvent);
#endif
		SProfilingInfo		m_sProfilingInfo;
		bool				m_bProfilingEnabled;

        // An OclEvent object cannot be copied
        OclEvent(const OclEvent&);           // copy constructor
        OclEvent& operator=(const OclEvent&);// assignment operator
#if OCL_EVENT_WAIT_STRATEGY == OCL_EVENT_WAIT_OS_DEPENDENT
		Intel::OpenCL::Utils::OclOsDependentEvent m_osEvent;
#endif

    };

}}};    // Intel::OpenCL::Framework
#endif  // !defined(__OCL_OCL_EVENT_H__)
