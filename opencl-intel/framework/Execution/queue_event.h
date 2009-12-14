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
//  queue_event.h
//  Implementation of the Class QueueEvent
//  Created on:      23-Dec-2008 3:23:06 PM
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////

#if !defined(__OCL_QUEUE_EVENT__)
#define __OCL_QUEUE_EVENT__

#include <cl_types.h>
#include <cl_object.h>
#include <cl_synch_objects.h>
#include "event_done_observer.h"
#include <list>

using namespace std;
using namespace Intel::OpenCL::Utils;

namespace Intel { namespace OpenCL { namespace Framework {

    // Forward declrations
    class Command;
    class IEventColorChangeObserver;
    class OclCommandQueue;

    typedef enum
    {
		EVENT_STATE_WHITE,			// The command is just created
	    EVENT_STATE_RED,			// The command is dependent on 1 or more other commands
	    EVENT_STATE_YELLOW,			// The command is ready to be executed - no deps.
	    EVENT_STATE_LIME,			// The command is issued to the device, but is not processed yet.
	    EVENT_STATE_GREEN,			// The command is been executed on the device.
	    EVENT_STATE_BLACK			// Command is done and ready for deletion.
    } QueueEventStateColor;


	/**********************************************************************************************
	 * struct name:	SProfilingInfo
	 *
	 * Description:	
     *      Holds the profiling information of the event
     * 
	 * Author:		Uri Levy
	 * Date:		May 2009
	/**********************************************************************************************/
	struct SProfilingInfo
	{
		cl_ulong	m_ulCommandQueued;	// A 64-bit value that describes the current device time
										// counter in nanoseconds when the command identified by
										// event is enqueued in a command-queue by the host.
		
		cl_ulong	m_ulCommandSubmit;	// A 64-bit value that describes the current device time
										// counter in nanoseconds when the command identified by
										// event that has been enqueued is submitted by the host
										// to the device associated with the commandqueue.
		
		cl_ulong	m_ulCommandStart;	// A 64-bit value that describes the current device time
										// counter in nanoseconds when the command identified by
										// event starts execution on the device
		
		cl_ulong	m_ulCommandEnd;		// A 64-bit value that describes the current device time
										// counter in nanoseconds when the command identified by
										// event has finished execution on the device
	};

	/**********************************************************************************************
	 * Class name:	QueueEvent
	 *
	 * Description:	
     *      QueueEvent controls the dependencies between execution commands. The queue implementation
     *      attaches an event to each command. 
     *      The event holds the execution state of the command, which is one of the following:
     *          - EVENT_STATE_RED:      The command is dependent on 1 or more other commands
     *          - EVENT_STATE_YELLOW:   The command is ready to be executed - no deps.
     *          - EVENT_STATE_LIME:     The command is issued to the device, but is not processed yet.
     *          - EVENT_STATE_GREEN:    The command is been executed on the device.
     *          - EVENT_STATE_BLACK:    Command is done and ready for deletion.
     * 
	 * Author:		Arnon Peleg
	 * Date:		December 2008
	/**********************************************************************************************/	
    class QueueEvent : public OCLObject, public IEventDoneObserver
    {

    public:
    	
		QueueEvent(OclCommandQueue* pQueue);
	    virtual ~QueueEvent();

	    bool                    RegisterEventDoneObserver( IEventDoneObserver* observer );
        void                    UnRegisterEventDoneObserver( IEventDoneObserver* observer );
	    void                    RegisterEventColorChangeObserver( IEventColorChangeObserver* observer );
        void                    SetDependentOn( QueueEvent* pDependsOnEvent );
	    cl_err_code             SetEventColor( QueueEventStateColor color );
        bool                    IsColor( QueueEventStateColor color )   {  OclAutoMutex CS(&m_locker); return (m_stateColor == color); }
        bool                    IsColorNotBlock( QueueEventStateColor color ) const { return (m_stateColor == color); }
        QueueEventStateColor    GetColor() const                                    { return m_stateColor; }
        OclCommandQueue*        GetEventQueue() const                               { return m_pEventQueue;}
        void                    SetEventQueue(OclCommandQueue* pQueue)              { m_pEventQueue = pQueue;}

        // Implementation IEventDoneObserver
        cl_err_code NotifyEventDone(QueueEvent* pEvent);

		// get/set the profiling data which assigned to the profiling parameter
		cl_ulong				GetProfilingInfo(cl_profiling_info clParamName) const;
		void					SetProfilingInfo(cl_profiling_info clParamName, cl_ulong ulData);

    private:
	    QueueEventStateColor            m_stateColor;           // Holds the current status of the event.
        cl_uint                         m_uDependencyCount;     // Count the number of events that this event has registered on

        OclMutex                        m_locker;               // Synch the access to the state memebers. 
        OclMutex                        m_codeLocker;           // Synch access to object methods to prevent races.

        list<IEventDoneObserver*>       m_observersList;        // List of ovservers; Notified on event done
        IEventColorChangeObserver*      m_colorChangeObserver;  // An observer that will be called when the status is changed
                                                                // Expected to be the command queue.
        OclCommandQueue*                m_pEventQueue;          // Pointer to the queue that this event was enqueued on  

		// profiling information
		SProfilingInfo		m_sProfilingInfo;

        // Private functinos
        void        EventCompleted();
       
        // The event id        
        static OclMutex     m_instanceCounterLocker;
        static cl_uint      m_uiInstanceCounter;

    };

}}};    // Intel::OpenCL::Framework
#endif  // !defined(__OCL_QUEUE_EVENT__)
