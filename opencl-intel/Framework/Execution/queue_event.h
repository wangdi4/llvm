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
#include "event_done_observer.h"
#include <list>

using namespace std;

namespace Intel { namespace OpenCL { namespace Framework {

    // Forward declrations
    class Command;

	/**********************************************************************************************
	 * Class name:	QueueEvent
	 *
	 * Description:	
     *      TODO
     *
	 * Author:		Arnon Peleg
	 * Date:		December 2008
	/**********************************************************************************************/	
    class QueueEvent : public IEventDoneObserver
    {

    public:

	    enum QueueEventStateColor
	    {
		    EVENT_STATE_YELLOW,
		    EVENT_STATE_RED,
		    EVENT_STATE_GREEN,
		    EVENT_STATE_LIME,
		    EVENT_STATE_GRAY,
		    EVENT_STATE_BLACK
	    };
    	
        QueueEvent();
	    virtual ~QueueEvent();
    	
	    void        RegisterEventDoneObserver( IEventDoneObserver* observer );
	    void        SetEventColor( QueueEventStateColor color );
        void        IncrementDependencyCount()                              { ++m_uDependencyCount;} //TODO: synch???
        bool        IsColor( QueueEventStateColor color )                   { return (m_stateColor == color); }
        
        // Implementation IEventDoneObserver
        cl_err_code NotifyEventDone(QueueEvent* event);

    private:
	    QueueEventStateColor        m_stateColor;       // Holds the current status of the event.
        cl_uint                     m_uDependencyCount; // Count the number of events that this event has registered on
        list<IEventDoneObserver*>   m_observersList;    // List of ovservers; Notified on event done
        
        // Private functions
        void                        EventCompleted();


        // Synch objects for reentrant support
        // TODO: Add reentrant code to this class
    };

}}};    // Intel::OpenCL::Framework
#endif  // !defined(__OCL_QUEUE_EVENT__)
