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


#pragma once

#include "cl_framework.h"
#include <cl_object.h>
#include <cl_synch_objects.h>
#include "event_done_observer.h"
#include <list>

namespace Intel { namespace OpenCL { namespace Framework {

	// Forward declarations
	class Command;
	class IOclCommandQueueBase;

	typedef enum
	{
		EVENT_STATE_WHITE,			// The command is just created
		EVENT_STATE_RED,			// The command is dependent on 1 or more other commands
		EVENT_STATE_YELLOW,			// The command is ready to be executed - no deps.
		EVENT_STATE_LIME,			// The command is issued to the device, but is not processed yet.
		EVENT_STATE_GREEN,			// The command is been executed on the device.
		EVENT_STATE_BLACK			// Command is done and ready for deletion.
	} OclEventStateColor;

#define OCL_EVENT_WAIT_SPIN         0
#define OCL_EVENT_WAIT_YIELD        1
#define OCL_EVENT_WAIT_OS_DEPENDENT 2

//#define OCL_EVENT_WAIT_STRATEGY OCL_EVENT_WAIT_SPIN
#ifdef WIN32
  #define OCL_EVENT_WAIT_STRATEGY OCL_EVENT_WAIT_YIELD
#else //For Linux, the CFS performs much better without active wait
  #define OCL_EVENT_WAIT_STRATEGY OCL_EVENT_WAIT_OS_DEPENDENT
#endif



	/**********************************************************************************************
	* struct name:	SProfilingInfo
	*
	* Description:	
	*      Holds the profiling information of the event
	* 
	* Author:		Uri Levy
	* Date:		May 2009
	**********************************************************************************************/
	struct SProfilingInfo
	{
		cl_ulong	m_ulCommandQueued;	// A 64-bit value that describes the current device time
		// counter in nanoseconds when the command identified by
		// event is enqueued in a command-queue by the host.

		cl_ulong	m_ulCommandSubmit;	// A 64-bit value that describes the current device time
		// counter in nanoseconds when the command identified by
		// event that has been enqueued is submitted by the host
		// to the device associated with the command queue.

		cl_ulong	m_ulCommandStart;	// A 64-bit value that describes the current device time
		// counter in nanoseconds when the command identified by
		// event starts execution on the device

		cl_ulong	m_ulCommandEnd;		// A 64-bit value that describes the current device time
		// counter in nanoseconds when the command identified by
		// event has finished execution on the device
	};

	/**********************************************************************************************
	* Class name:	OclEvent
	*
	* Description:	
	*      OclEvent controls the dependencies between execution commands. The queue implementation
	*      attaches an event to each command. 
	*      The event holds the execution state of the command, which is one of the following:
	*          - EVENT_STATE_RED:      The command is dependent on 1 or more other commands
	*          - EVENT_STATE_YELLOW:   The command is ready to be executed - no deps.
	*          - EVENT_STATE_LIME:     The command is issued to the device, but is not processed yet.
	*          - EVENT_STATE_GREEN:    The command is been executed on the device.
	*          - EVENT_STATE_BLACK:    Command is done and ready for deletion.
	* 
	* Author:		Doron Singer
	* Date:		July 2010
	**********************************************************************************************/	
	class OclEvent : public IEventDoneObserver, public OCLObject<_cl_event_int>
	{
	public:
		OclEvent();

		void                    AddCompleteListener(IEventDoneObserver* listener);
		void                    AddDependentOn( OclEvent* pDependsOnEvent );
		void                    AddDependentOnMulti(unsigned int count, OclEvent** pDependencyList);
		void                    AddFloatingDependence() { ++m_depListLength; }
		void                    RemoveFloatingDependence() { if (0 == --m_depListLength) NotifyReady(NULL); }
		OclEventStateColor      SetColor(OclEventStateColor newColor); //returns the previous color

		OclEventStateColor      GetColor() const                                    { return (OclEventStateColor)((long)m_color); }
		bool                    HasDependencies() const                             { return m_depListLength > 0;}

		// Implementation IEventDoneObserver
		virtual cl_err_code NotifyEventDone(OclEvent* pEvent, cl_int returnCode = CL_SUCCESS);

		// Blocking function, returns after NotifyComplete is done
		virtual void    Wait();

		cl_int GetEventCurrentStatus();


		// Get the context to which the event belongs.
		virtual cl_context GetContextHandle() const = 0;
		// Get the return code of the command associated with the event.
		virtual cl_int     GetReturnCode() const = 0; 

	protected:
		virtual ~OclEvent();

		virtual void   NotifyReady(OclEvent* pEvent); 
		virtual void   NotifyComplete(cl_int returnCode = CL_SUCCESS);

		//Some implementations of possible methods for waiting 
		void        WaitSpin();
		void        WaitYield();
		void        WaitOSEvent();

		// An OclEvent object cannot be copied
		OclEvent(const OclEvent&);           // copy constructor
		OclEvent& operator=(const OclEvent&);// assignment operator

		Intel::OpenCL::Utils::OclNaiveConcurrentQueue<IEventDoneObserver*>	m_CompleteListeners;
		Intel::OpenCL::Utils::AtomicCounter								m_CompleteListenersGuard;
		Intel::OpenCL::Utils::AtomicCounter								m_depListLength;
		volatile bool													m_complete;
		Intel::OpenCL::Utils::AtomicCounter								m_color;
		
#if OCL_EVENT_WAIT_STRATEGY == OCL_EVENT_WAIT_OS_DEPENDENT
		Intel::OpenCL::Utils::OclOsDependentEvent m_osEvent;
#endif

	};

}}}    // Intel::OpenCL::Framework

