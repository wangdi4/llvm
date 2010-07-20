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
#include "command_queue.h"
#include <cl_sys_info.h>
#include "enqueue_commands.h"

// For debugging
#include <assert.h>
#include "cl_utils.h"

using namespace Intel::OpenCL::Framework;
QueueEvent::QueueEvent(IOclCommandQueueBase* cmdQueue) : m_complete(false), m_color(EVENT_STATE_WHITE), m_pEventQueue(cmdQueue)
{
}

QueueEvent::~QueueEvent()
{
}
//Todo: very unhappy about reinterpret cast here
//Need to consider aggregation over inheritance

void QueueEvent::AddDependentOn( OclEvent* pDependsOnEvent)
{
	//Must increase dependency list length before adding as listener
	//The other event may have completed already, in which case our callback will be called immediately
	//Which will decrease the depListLength.
	++m_depListLength;
	QueueEvent* pQueueDependsOnEvent = reinterpret_cast<QueueEvent*>(pDependsOnEvent);
	assert(pQueueDependsOnEvent);
	pQueueDependsOnEvent->AddCompleteListener(this);
}

//Important to increase the guard by count immediately, otherwise there's a race condition where the first dependency is finished before the second is registered
void QueueEvent::AddDependentOnMulti(unsigned int count, OclEvent** pDependencyList)
{
	m_depListLength.add(count);
	for (unsigned int i = 0; i < count; ++i)
	{
		QueueEvent* evt = reinterpret_cast<QueueEvent*>(pDependencyList[i]);
		if (evt != NULL)
		{
			evt->AddCompleteListener(this);
		}
	}
}

void QueueEvent::AddCompleteListener(IEventDoneObserver* listener)
{	
	++m_CompleteListenersGuard;	
	
	if (!m_complete)	
	{
		m_CompleteListeners.PushBack(listener);
		--m_CompleteListenersGuard;		
	}
	else //event completed while we were registering, notify immediately
	{			
		--m_CompleteListenersGuard;
		listener->NotifyEventDone(this);
	}
}

QueueEventStateColor QueueEvent::SetColor(QueueEventStateColor color)
{
	//Todo: do we need atomic exchange here?
	QueueEventStateColor oldColor = m_color;
	m_color = color;
	if (EVENT_STATE_BLACK == color)
	{		
		NotifyComplete();
	}	
	return oldColor;
}

//Notifies us that pEvent that we were listening on, has completed
cl_err_code QueueEvent::NotifyEventDone(QueueEvent* pEvent)
{
	if (0 == --m_depListLength)
	{
		NotifyReady(pEvent);
	}
	return CL_SUCCESS;
}

void QueueEvent::NotifyComplete()
{	
	//block further requests to add notifiers
	m_complete = true;	
	//loop until all pending addition requests are complete
	
	while (m_CompleteListenersGuard > 0) {}		
	
	//Notify everyone
	while (!m_CompleteListeners.IsEmpty())
	{
		IEventDoneObserver* listener = m_CompleteListeners.PopFront();
		assert(listener);
		listener->NotifyEventDone(this);
	}			

	if (m_pEventQueue)
	{	
		m_pEventQueue->NotifyStateChange(this, EVENT_STATE_GREEN, EVENT_STATE_BLACK);
	}	
	m_pCommand->Release();
	return;
}


void QueueEvent::NotifyReady(QueueEvent* pEvent)
{
	if (EVENT_STATE_RED == m_color)
	{		
		m_color = EVENT_STATE_YELLOW;						
		if ((pEvent) && (pEvent->GetEventQueue() && pEvent->GetEventQueue()->GetId() == GetEventQueue()->GetId()))
		{	
			//that event will notify my queue for me
			return;
		}
		//else, I have to notify the queue myself		
		m_pEventQueue->NotifyStateChange(this, EVENT_STATE_RED, EVENT_STATE_YELLOW);
	}

}