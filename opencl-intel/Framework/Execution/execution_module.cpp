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
//  execution_module.cpp
//  Implementation of the Class ExecutionModule
//  Created on:      23-Dec-2008 3:23:00 PM
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////
#include "execution_module.h"

using namespace Intel::OpenCL::Framework;

ExecutionModule::ExecutionModule(){

}

ExecutionModule::~ExecutionModule(){

}


cl_command_queue ExecutionModule::CreateCommandQueue(cl_context context, cl_device_id device, cl_command_queue_properties properties, cl_int* errcode_ret){

	return  NULL;
}


OclCommandQueue* ExecutionModule::GetCommandQueue(cl_command_queue command_queue){

	return NULL;
}


cl_err_code ExecutionModule::GetCommandQueueInfo(cl_command_queue command_queue, cl_command_queue_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret){

	return  CL_SUCCESS;
}


cl_err_code ExecutionModule::ReleaseCommandQueue(cl_command_queue command_queue){

	return  CL_SUCCESS;
}


cl_err_code ExecutionModule::RetainCommandQueue(cl_command_queue command_queue){

	return  CL_SUCCESS;
}


cl_err_code ExecutionModule::SetCommandQueueProperty(cl_command_queue command_queue, cl_command_queue_properties properties, cl_bool enable, cl_command_queue_properties* old_properties){

	return  CL_SUCCESS;
}


cl_err_code ExecutionModule::EnqueueReadBuffer(cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_read, size_t offset, size_t cb, void* ptr, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* pEvent){

	return  CL_SUCCESS;
}


cl_err_code ExecutionModule::EnqueueWriteBuffer(cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_write, size_t offset, size_t cb, const void* ptr, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* pEvent){

	return  CL_SUCCESS;
}


cl_err_code ExecutionModule::EnqueueReadImage(cl_command_queue command_queue, cl_mem image, cl_bool blocking_read, const size_t* origin[3], const size_t* region[3], size_t row_pitch, size_t slice_pitch, void* ptr, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* pEvent){

	return  CL_SUCCESS;
}


cl_err_code ExecutionModule::EnqueueWriteImage(cl_command_queue command_queue, cl_mem image, cl_bool blocking_write, const size_t* origin[3], const size_t* region[3], size_t row_pitch, size_t slice_pitch, const void* ptr, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* pEvent){

	return  CL_SUCCESS;
}


cl_err_code ExecutionModule::EnqueueCopyBuffer(cl_command_queue command_queue, cl_mem src_buffer, cl_mem dst_buffer, size_t src_offset, size_t dst_offset, size_t cb, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* pEvent){

	return  CL_SUCCESS;
}


cl_err_code ExecutionModule::EnqueueCopyImage(cl_command_queue command_queue, cl_mem src_image, cl_mem dst_image, const size_t* src_origin[3], const size_t* dst_origin[3], const size_t* region[3], cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* pEvent){

	return  CL_SUCCESS;
}


cl_err_code ExecutionModule::EnqueueCopyImageToBuffer(cl_command_queue command_queue, cl_mem src_image, cl_mem dst_buffer, const size_t* src_origin[3], const size_t* region[3], size_t dst_offset, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* pEvent){

	return  CL_SUCCESS;
}


cl_err_code ExecutionModule::EnqueueCopyBufferToImage(cl_command_queue command_queue, cl_mem src_buffer, cl_mem dst_image, size_t src_offset, const size_t* dst_origin[3], const size_t* region[3], cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* pEvent){

	return  CL_SUCCESS;
}


void * ExecutionModule::EnqueueMapBuffer(cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_map, cl_map_flags map_flags, size_t offset, size_t cb, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* pEvent, cl_int* errcode_ret){

	return  CL_SUCCESS;
}


void * ExecutionModule::EnqueueMapImage(cl_command_queue command_queue, cl_mem image, cl_bool blocking_map, cl_map_flags map_flags, const size_t* origin[3], const size_t* region[3], size_t* image_row_pitch, size_t* image_slice_pitch, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* pEvent, cl_int* errcode_ret){

	return  CL_SUCCESS;
}


cl_err_code ExecutionModule::EnqueueUnmapMemObject(cl_command_queue command_queue, cl_mem memobj, void* mapped_ptr, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event){

	return  CL_SUCCESS;
}


cl_err_code ExecutionModule::EnqueueNDRangeKernel(cl_command_queue command_queue, cl_kernel kernel, cl_uint work_dim, const size_t* global_work_offset, const size_t* global_work_size, const size_t* local_work_size, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* pEvent){

	return  CL_SUCCESS;
}


cl_err_code ExecutionModule::EnqueueTask(cl_command_queue command_queue, cl_kernel kernel, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* pEvent){

	return  CL_SUCCESS;
}


cl_err_code ExecutionModule::EnqueueNativeFnAsKernel(cl_command_queue command_queue, void (*user_func)(void *), void* args, size_t cb_args, cl_uint num_mem_objects, const cl_mem* mem_list, const void** args_mem_loc, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* pEvent){

	return  CL_SUCCESS;
}


cl_err_code ExecutionModule::EnqueueMarker(cl_command_queue command_queue, cl_event* pEvent){

	return  CL_SUCCESS;
}


cl_err_code ExecutionModule::EnqueueWaitForEvents(cl_command_queue command_queue, cl_uint num_events, const cl_event* event_list){

	return  CL_SUCCESS;
}


cl_err_code ExecutionModule::EnqueueBarrier(cl_command_queue command_queue){

	return  CL_SUCCESS;
}
