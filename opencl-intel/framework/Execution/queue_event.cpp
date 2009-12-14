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
//  queue_event.cpp
//  Implementation of the Class QueueEvent
//  Created on:      23-Dec-2008 3:23:06 PM
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////
#include "queue_event.h"
#include "event_color_change_observer.h"
#include "ocl_command_queue.h"
#include <cl_sys_info.h>

// For debugging
#include <assert.h>
#include "cl_utils.h"

using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;

cl_uint     QueueEvent::m_uiInstanceCounter = 1;
OclMutex    QueueEvent::m_instanceCounterLocker;

/******************************************************************
 * Constructor
 ******************************************************************/
QueueEvent::QueueEvent(OclCommandQueue* pQueue):
	m_stateColor(EVENT_STATE_YELLOW),
	m_uDependencyCount(0),
	m_colorChangeObserver(NULL),
	m_pEventQueue(pQueue)
{
	m_observersList.clear();
	// Lock access to the counter
	OclAutoMutex CS(&m_instanceCounterLocker);
	cl_start;
	m_iId = m_uiInstanceCounter;
	m_uiInstanceCounter++;

	// set the queued time in the profiling info data structure
	if (m_pEventQueue && m_pEventQueue->IsProfilingEnabled())
	{
		SetProfilingInfo(CL_PROFILING_COMMAND_QUEUED, HostTime());
		SetProfilingInfo(CL_PROFILING_COMMAND_SUBMIT, 0);
		SetProfilingInfo(CL_PROFILING_COMMAND_START,  0);
		SetProfilingInfo(CL_PROFILING_COMMAND_END,    0);
	}
	cl_return;
}


/******************************************************************
 * Destructor
 ******************************************************************/
QueueEvent::~QueueEvent()
{
    EventCompleted();
    m_colorChangeObserver = NULL;
}

/******************************************************************
 * Use this function to register this event as dependent on the 
 * input event: pDependsOnEvent;
 *
 ******************************************************************/
void QueueEvent::SetDependentOn( QueueEvent* pDependsOnEvent )
{
    if ( pDependsOnEvent->RegisterEventDoneObserver(this) )
    {
        { OclAutoMutex CS(&m_locker);
        // Increment dependency count;
        ++m_uDependencyCount;
        }
        SetEventColor( EVENT_STATE_RED);
    }
}

/******************************************************************
 * An object is registered on this event's completion status.
 * When this event is done, is expected to notify all observers.
 *
 ******************************************************************/
bool QueueEvent::RegisterEventDoneObserver(IEventDoneObserver* observer)
{
	cl_start;
    OclAutoMutex CS(&m_locker);
    if (m_stateColor == EVENT_STATE_BLACK)
    {
        // Don't register on event that is done already
        return false;
    }
    m_observersList.push_back(observer);
    cl_return true;
}

/******************************************************************
 * Remove the input observer from the list, if exists.
 *
 ******************************************************************/
void QueueEvent::UnRegisterEventDoneObserver( IEventDoneObserver* observer )
{
    // Don't remove when EventCompleted is running.
    OclAutoMutex CS(&m_codeLocker);
    list<IEventDoneObserver*>::iterator iter = m_observersList.begin();
    
    // loop until find
    for ( ; iter != m_observersList.end(); iter++ ) 
    {
        IEventDoneObserver* listObserver = *iter;
        if( listObserver == observer )
        {
            // They both point to the same object, found, erase, break
            m_observersList.erase(iter);
            break;
        }
    }
}

/******************************************************************
 * An object is registered on this event's color change.
 *
 ******************************************************************/
void QueueEvent::RegisterEventColorChangeObserver( IEventColorChangeObserver* observer )
{
    OclAutoMutex CS(&m_codeLocker);
    m_colorChangeObserver = observer;
}

/******************************************************************
 * Private method to use in order to change event state.
 * Private methods are not locked. Verify that the function is used within a CS
 ******************************************************************/
cl_err_code QueueEvent::SetEventColor( QueueEventStateColor color)
{  
    cl_err_code res = CL_SUCCESS;
    QueueEventStateColor prevColor;
    IEventColorChangeObserver* observer;

    // Use codeLocker to keep against races
    { OclAutoMutex CS(&m_codeLocker);

    // Get observer locally
    observer = m_colorChangeObserver;

    { OclAutoMutex CS(&m_locker); // Critical Section - Lock access to m_stateColor

    if ( m_stateColor == color )
    {
        // Color doesn't change, nothing to do
        return res;
    }    
    prevColor = m_stateColor;
    m_stateColor = color;
    }

    if ( EVENT_STATE_BLACK == color )
    {
        // True only if this was the one instance where the event color turned black,
        EventCompleted();
    }
    
    // Notify color change,
    // Note, if color changed to black, the queue may delete the command
    // before returning from this function. As a result the locker as well as other private members may be invalid.
    // Therefore the code below is unprotected and uses only local variables. 
    // A race may occurs and we need to see wht will happen.
    // Moreover, is there an option that the observer (the queue) is already deleted? TODO: Check it out!
    } // End locker
    if ( NULL != observer)
    {
        res = observer->NotifyEventColorChange(this, prevColor, color);
    }   
    return res;
}

/******************************************************************
 * This function is being called when event is finished.
 * The function is expected to pop out all its observers.
 * TODO: Lock list from Add/Remove observers during execution
 * 
 ******************************************************************/
void QueueEvent::EventCompleted()
{
    // loop while there are more items
    while( !m_observersList.empty() )
    {
        IEventDoneObserver* observer = m_observersList.front();
        m_observersList.pop_front();
        observer->NotifyEventDone(this);
    }
}

/******************************************************************
 * When other event that this event is depends on is done, this function
 * is called.
 * This event decrement is dependcy count and if it's counter set to
 * 0, than change status to green. The event shall notify the 
 * 
 ******************************************************************/
cl_err_code QueueEvent::NotifyEventDone(QueueEvent* pEvent)
{
    cl_err_code res = CL_SUCCESS;
    bool bIsTurnedGreen;
    // Look access to m_uDependencyCount.
    { OclAutoMutex CS(&m_locker);
    --m_uDependencyCount;
    bIsTurnedGreen = ( 0 == m_uDependencyCount );
    }
    if ( bIsTurnedGreen )
    {
        res = SetEventColor(EVENT_STATE_YELLOW);
    }
    return res;
}

cl_ulong QueueEvent::GetProfilingInfo(cl_profiling_info clParamName) const
{
	switch ( clParamName )
	{
	case CL_PROFILING_COMMAND_QUEUED:
		return m_sProfilingInfo.m_ulCommandQueued;
	case CL_PROFILING_COMMAND_SUBMIT:
		return m_sProfilingInfo.m_ulCommandSubmit;
	case CL_PROFILING_COMMAND_START:
		return m_sProfilingInfo.m_ulCommandStart;
	case CL_PROFILING_COMMAND_END:
		return m_sProfilingInfo.m_ulCommandEnd;
	default:
		return 0;
	}
	return 0;
}
void QueueEvent::SetProfilingInfo(cl_profiling_info clParamName, cl_ulong ulData)
{
	switch ( clParamName )
	{
	case CL_PROFILING_COMMAND_QUEUED:
		m_sProfilingInfo.m_ulCommandQueued = ulData;
	case CL_PROFILING_COMMAND_SUBMIT:
		m_sProfilingInfo.m_ulCommandSubmit = ulData;
	case CL_PROFILING_COMMAND_START:
		m_sProfilingInfo.m_ulCommandStart = ulData;
	case CL_PROFILING_COMMAND_END:
		m_sProfilingInfo.m_ulCommandEnd = ulData;
	default:
		// Fall Through
		break;
	}
}