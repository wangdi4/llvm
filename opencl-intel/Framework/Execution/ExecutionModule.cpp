// Copyright (c) 2006-2007 Intel Corporation
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
//  ExecutionModule.cpp
//  Implementation of the Class ExecutionModule
//  Created on:      10-Dec-2008 2:11:51 PM
//  Original author: arnonp
///////////////////////////////////////////////////////////

#include "ExecutionModule.h"
using namespace Intel::OpenCL::Framework;


ExecutionModule::ExecutionModule()
{

}



ExecutionModule::~ExecutionModule()
{

}



cl_command_queue ExecutionModule::CreateCommandQueue(cl_context context, cl_device_id device, cl_command_queue_properties properties, cl_int* errcode_ret){

	return  NULL;
}


cl_int ExecutionModule::EnqueueWriteBuffer(cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_write, size_t offset, size_t cb, void* ptr, const cl_uint num_events_in_wait_list, const cl_event event_wait_list, cl_event event){

	return  NULL;
}


CommandQueue ExecutionModule::GetCommandQueue(cl_command_queue command_queue){

	return  NULL;
}


cl_int ExecutionModule::GetCommandQueueInfo(cl_command_queue command_queue, cl_command_queue_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret){

	return  NULL;
}


cl_int ExecutionModule::ReleaseCommandQueue(cl_command_queue command_queue)
{
	return  NULL;
}


cl_int ExecutionModule::RetainCommandQueue(cl_command_queue command_queue){

	return  NULL;
}


cl_int ExecutionModule::SetCommandQueueProperty(cl_command_queue command_queue, cl_command_queue_properties properties, cl_bool enable, cl_command_queue_properties* old_properties){

	return  NULL;
}