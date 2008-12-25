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
//  ocl_command_queue.cpp
//  Implementation of the Class OclCommandQueue
//  Created on:      23-Dec-2008 3:23:05 PM
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////
#include "ocl_command_queue.h"

using namespace Intel::OpenCL::Framework;

/******************************************************************
 *
 ******************************************************************/
OclCommandQueue::OclCommandQueue(
    Context*                    context, 
    Device*                     device, 
    cl_command_queue_properties properties
    )
{
}

/******************************************************************
 *
 ******************************************************************/
OclCommandQueue::~OclCommandQueue()
{
}

/******************************************************************
 *
 ******************************************************************/
cl_int OclCommandQueue::GetInfo(cl_command_queue_info param_name, size_t param_value_size, void * param_value, size_t* param_value_size_ret){

	return  0;
}

/******************************************************************
 *
 ******************************************************************/
void OclCommandQueue::EnqueueDevCommands()
{
	return ;
}

/******************************************************************
 *
 ******************************************************************/
void OclCommandQueue::PushFrontCommand()
{
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code OclCommandQueue::SetProperties(cl_command_queue_properties properties, cl_command_queue_properties* old_properties){

	return  CL_SUCCESS;
}


/******************************************************************
 *
 ******************************************************************/
cl_err_code OclCommandQueue::EnqueueCommand(Command* command, cl_bool blocking, const cl_event event_wait_list, cl_uint num_events_in_wait_list, cl_event pEvent)
{
    return CL_SUCCESS;

}

/**
 * Set a marker object on the current state of the queue
 */
cl_err_code OclCommandQueue::SetMarker(cl_event pEvent)
{
    return CL_SUCCESS;
}


/******************************************************************
 *
 ******************************************************************/
cl_err_code OclCommandQueue::SetBarrier()
{
    return CL_SUCCESS;
}


/**
 * This functions resolve synch events issues such as a barriar in the queue or In-
 * order queue.
 */
cl_err_code OclCommandQueue::ResolvedSynchEvents(cl_command_type commandType, QueueEvent* newEvent){

	return  CL_SUCCESS;
}
