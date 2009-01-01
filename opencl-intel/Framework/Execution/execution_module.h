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
//  execution_module.h
//  Implementation of the Class ExecutionModule
//  Created on:      23-Dec-2008 3:23:00 PM
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////
#if !defined(__OCL_EXECUTION_MODULE_H__)
#define __OCL_EXECUTION_MODULE_H__

#include <cl_types.h>


namespace Intel { namespace OpenCL { namespace Framework {
    // forward declrations
    class OCLObjectsMap;
    class EventsManager;
    class OclCommandQueue;

    /**
     * ExecutionModule class the platform module responsible of all execution related
     * operations. this might include queues events etc.
     */
    class ExecutionModule
    {

    public:

	    ExecutionModule();
	    virtual ~ExecutionModule();
    	
        cl_command_queue    CreateCommandQueue(cl_context context, cl_device_id device, cl_command_queue_properties properties, cl_int* errcode_ret);
	    OclCommandQueue*    GetCommandQueue(cl_command_queue command_queue);
	    cl_int              GetCommandQueueInfo(cl_command_queue command_queue, cl_command_queue_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret);
	    cl_int              ReleaseCommandQueue(cl_command_queue command_queue);
	    cl_int              RetainCommandQueue(cl_command_queue command_queue);
	    cl_int              SetCommandQueueProperty(cl_command_queue command_queue, cl_command_queue_properties properties, cl_bool enable, cl_command_queue_properties* old_properties);

        // Enqueue commands
	    cl_int EnqueueReadBuffer(cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_read, size_t offset, size_t cb, void* ptr, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* pEvent);
	    cl_int EnqueueWriteBuffer(cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_write, size_t offset, size_t cb, const void* ptr, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* pEvent);
	    cl_int EnqueueReadImage(cl_command_queue command_queue, cl_mem image, cl_bool blocking_read, const size_t* origin[3], const size_t* region[3], size_t row_pitch, size_t slice_pitch, void* ptr, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* pEvent);
	    cl_int EnqueueWriteImage(cl_command_queue command_queue, cl_mem image, cl_bool blocking_write, const size_t* origin[3], const size_t* region[3], size_t row_pitch, size_t slice_pitch, const void* ptr, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* pEvent);
	    cl_int EnqueueCopyBuffer(cl_command_queue command_queue, cl_mem src_buffer, cl_mem dst_buffer, size_t src_offset, size_t dst_offset, size_t cb, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* pEvent);
	    cl_int EnqueueCopyImage(cl_command_queue command_queue, cl_mem src_image, cl_mem dst_image, const size_t* src_origin[3], const size_t* dst_origin[3], const size_t* region[3], cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* pEvent);
	    cl_int EnqueueCopyImageToBuffer(cl_command_queue command_queue, cl_mem src_image, cl_mem dst_buffer, const size_t* src_origin[3], const size_t* region[3], size_t dst_offset, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* pEvent);
	    cl_int EnqueueCopyBufferToImage(cl_command_queue command_queue, cl_mem src_buffer, cl_mem dst_image, size_t src_offset, const size_t* dst_origin[3], const size_t* region[3], cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* pEvent);
	    void*  EnqueueMapBuffer(cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_map, cl_map_flags map_flags, size_t offset, size_t cb, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* pEvent, cl_int* errcode_ret);
	    void*  EnqueueMapImage(cl_command_queue command_queue, cl_mem image, cl_bool blocking_map, cl_map_flags map_flags, const size_t* origin[3], const size_t* region[3], size_t* image_row_pitch, size_t* image_slice_pitch, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* pEvent, cl_int* errcode_ret);
	    cl_int EnqueueUnmapMemObject(cl_command_queue command_queue, cl_mem memobj, void* mapped_ptr, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
	    cl_int EnqueueNDRangeKernel(cl_command_queue command_queue, cl_kernel kernel, cl_uint work_dim, const size_t* global_work_offset, const size_t* global_work_size, const size_t* local_work_size, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* pEvent);
	    cl_int EnqueueTask(cl_command_queue command_queue, cl_kernel kernel, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* pEvent);
	    cl_int EnqueueNativeFnAsKernel(cl_command_queue command_queue, void (*user_func)(void *), void* args, size_t cb_args, cl_uint num_mem_objects, const cl_mem* mem_list, const void** args_mem_loc, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* pEvent);
	    cl_int EnqueueMarker(cl_command_queue command_queue, cl_event* pEvent);
	    cl_int EnqueueWaitForEvents(cl_command_queue command_queue, cl_uint num_events, const cl_event* event_list);
	    cl_int EnqueueBarrier(cl_command_queue command_queue);

    private:
	    OCLObjectsMap*      m_oclCommandQueueMap;       // Holds the set of active queues
	    EventsManager*      m_EventsManager;
    };

}}};    // Intel::OpenCL::Framework
#endif  // !defined(__OCL_EXECUTION_MODULE_H__)
