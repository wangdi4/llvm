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
//  EventsManager.cpp
//  Implementation of the Class EventsManager
//  Created on:      23-Dec-2008 3:22:59 PM
//  Original author: Peleg, Arnon
//////////////////////////////////////////////////////////
#include "events_manager.h"

using namespace Intel::OpenCL::Framework;

/******************************************************************
 *
 ******************************************************************/
EventsManager::EventsManager():
    m_OclObjectsMap(NULL)
{
}

/******************************************************************
 *
 ******************************************************************/
EventsManager::~EventsManager()
{
}

/**
 * This function gets an handle to event and a list of event handlers that refer
 * to events that are equired to register as done observer on that event
 * 
 * a list of event handlers 
 */
cl_int EventsManager::RegisterEvents(QueueEvent* event, HndlsList* event_wait_list){

	return  CL_SUCCESS;
}


/**
 * This function creates event object. The Ocl event hadnle that can be used by
 * the user is assigned into the eventHndl.
 * The function returns pointer to a QueueEvent that is attached with the related
 * Command object.
 */
QueueEvent* EventsManager::CreateEvent(cl_command_type eventCommandType, cl_command_queue eventQueueHndl, cl_event pEventHndl){

	return  NULL;
}

/******************************************************************
 *
 ******************************************************************/
void EventsManager::EventStatusChange(cl_event eventId, cl_int commandStatus){

}

/******************************************************************
 *
 ******************************************************************/
void EventsManager::WaitForEvents(cl_uint num_events, const cl_event event_list)
{

}