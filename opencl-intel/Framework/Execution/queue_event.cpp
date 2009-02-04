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

cl_uint QueueEvent::m_uiInstanceCounter = 0; //Debugging

/******************************************************************
 *
 ******************************************************************/
QueueEvent::QueueEvent():
    m_stateColor(EVENT_STATE_YELLOW),
    m_uDependencyCount(0)  
{
    m_observersList.clear();
    m_colorChangeObserversList.clear();

    m_iId = m_uiInstanceCounter;
    m_uiInstanceCounter++;
}

/******************************************************************
 *
 ******************************************************************/
QueueEvent::~QueueEvent()
{
    EventCompleted();
    m_colorChangeObserversList.clear();
}

/******************************************************************
 * Use this function to register this event as dependent on the 
 * input event: pDependsOnEvent;
 * If the user uses only RegisterEventDoneObserver for this operation,
 * the event will not be able when all the events that it is depend on are done.
 *
 ******************************************************************/
void QueueEvent::SetDependentOn( QueueEvent* pDependsOnEvent )
{
    m_stateColor = EVENT_STATE_RED;
    IncrementDependencyCount();
    pDependsOnEvent->RegisterEventDoneObserver(this);
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
 * Remove the input observer from the list, if exists.
 *
 ******************************************************************/
void QueueEvent::UnRegisterEventDoneObserver( IEventDoneObserver* observer )
{
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
 * When this event change color, is expected to notify all observers.
 *
 ******************************************************************/
void QueueEvent::RegisterEventColorChangeObserver( IEventColorChangeObserver* observer )
{    
    m_colorChangeObserversList.push_back(observer);
}

/******************************************************************
 *
 ******************************************************************/
void QueueEvent::SetEventColor(QueueEventStateColor color)
{
    // Use Retain/Release to prevent potential deletion by one of the observers.
    // TODO: Set Event reentrant.
    Retain();
    m_stateColor = color;
    switch(color)
    {
    case EVENT_STATE_BLACK:
        EventCompleted();
        // Fall through to color change
    case EVENT_STATE_YELLOW:
    case EVENT_STATE_RED:
    case EVENT_STATE_GREEN:
    case EVENT_STATE_LIME:
    case EVENT_STATE_GRAY:
        // All events are fall through to this break;
        EventColorChange();
        break;
    default:
        break;
    }
    Release();
}

/******************************************************************
 * This function is being called when event is finished.
 * The function is expected to pop out all its observers.
 * to 
 * 
 ******************************************************************/
void QueueEvent::EventCompleted()
{
    list<IEventDoneObserver*>::iterator iter = m_observersList.begin();
    
    // loop while there are more items
    for ( ; iter != m_observersList.end(); iter++ ) 
    {
        IEventDoneObserver* observer = *iter;
        // Note, may update the observers list
        observer->NotifyEventDone(this);
    }
    m_observersList.clear();
}


/******************************************************************
 * This function is being called when event change its color.
 * The function is expected to pop out all its observers and
 * 
 ******************************************************************/
void QueueEvent::EventColorChange()
{
    list<IEventColorChangeObserver*>::iterator iter = m_colorChangeObserversList.begin();
    list<IEventColorChangeObserver*>::iterator last = m_colorChangeObserversList.end();
    
    // loop while there are more items
    for ( ; iter != last; iter++ )
    {
        IEventColorChangeObserver* observer = *iter;
        observer->NotifyEventColorChange(this);
    }
}

/******************************************************************
 * When other event that this event is depends on is done, this function
 * is called.
 * This event decrement is dependcy count and if it's counter set to
 * 0, than change status to green. The event shall notify the 
 * 
 ******************************************************************/
cl_err_code QueueEvent::NotifyEventDone(QueueEvent* event)
{
    --m_uDependencyCount;
    if ( 0 == m_uDependencyCount )
    {
        SetEventColor(EVENT_STATE_GREEN);        
    }
    return CL_SUCCESS;
}
