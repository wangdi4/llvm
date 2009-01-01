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

using namespace Intel::OpenCL::Framework;

/******************************************************************
 *
 ******************************************************************/
QueueEvent::QueueEvent():
    m_stateColor(EVENT_STATE_YELLOW),
    m_uDependencyCount(0)  
{
    m_observersList.clear();
}

/******************************************************************
 *
 ******************************************************************/
QueueEvent::~QueueEvent()
{
    m_observersList.clear();
}

/******************************************************************
 * An object is registered on this event's completion status.
 * When this event is done, is expected to notify all observers.
 *
 ******************************************************************/
void QueueEvent::RegisterEventDoneObserver(IEventDoneObserver* observer)
{
    m_observersList.push_back(observer);
}

/******************************************************************
 *
 ******************************************************************/
void QueueEvent::SetEventColor(QueueEventStateColor color)
{
    m_stateColor = color;
    switch(color)
    {
    case EVENT_STATE_YELLOW:
    case EVENT_STATE_RED:
    case EVENT_STATE_GREEN:
    case EVENT_STATE_LIME:
    case EVENT_STATE_GRAY:
        // All events are fall through to this break;
        // Do nothing for now
        break;
    case EVENT_STATE_BLACK:
        EventCompleted();
        break;
    default:
        break;
    }
}

/******************************************************************
 * This function is being called when event is finished.
 * The function is expected to pop out all its observers and
 * to 
 * 
 ******************************************************************/
void QueueEvent::EventCompleted()
{
    list<IEventDoneObserver*>::iterator iter = m_observersList.begin();
    list<IEventDoneObserver*>::iterator last = m_observersList.end();
    
    // loop while there are more items
    for ( ; iter != last; iter++ ) 
    {
        IEventDoneObserver* observer = *iter;
        observer->NotifyEventDone(this);
    }

    // Clean the list, remove all pointers.
    m_observersList.clear();
}

/******************************************************************
 * When other event that this event is depends on is done, this function
 * is called.
 * This event decrement is dependcy count and if it's counter set to
 * 0, than change status to green.
 * 
 ******************************************************************/
cl_err_code QueueEvent::NotifyEventDone(QueueEvent* event)
{
    --m_uDependencyCount;
    if ( 0 == m_uDependencyCount )
    {
        m_stateColor = EVENT_STATE_GREEN;
    }
    return CL_SUCCESS;
}
