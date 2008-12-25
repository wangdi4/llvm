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
//  ocl_event.h
//  Implementation of the Class OclEvent
//  Created on:      23-Dec-2008 3:23:05 PM
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////

#if !defined(EA_4AD93ADF_C1A7_489d_8BCD_70981B5660DF__INCLUDED_)
#define EA_4AD93ADF_C1A7_489d_8BCD_70981B5660DF__INCLUDED_

#include <cl_framework.h>
#include "..\OCLObject.h"
#include "event_done_observer.h"

///////////////////////////////////////////////////////////
namespace Intel { namespace OpenCL { namespace Framework {
///////////////////////////////////////////////////////////

    //Forward declrations
    class Event;
    class QueueEvent;


    class OclEvent : public OCLObject, public IEventDoneObserver
    {

    public:
	    OclEvent();
	    virtual ~OclEvent();
    	
        cl_err_code NotifyEventDone(QueueEvent* event);

    private:
	    cl_command_type     m_commandType;
	    cl_command_queue    m_queueID;
	    QueueEvent*         m_QueueEvent;
	    Event*              m_Event;        // A synch object to wait on when app use clWaitOnEvents
    };

}}};    // Intel::OpenCL::Framework
#endif  // !defined(EA_4AD93ADF_C1A7_489d_8BCD_70981B5660DF__INCLUDED_)
