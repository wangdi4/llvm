// Copyright (c) 2008-2012 Intel Corporation
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

#pragma once

#include "CL/cl_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

// temporary header file for OpenCL 2.0 defintions

#define CL_API_SUFFIX__VERSION_2_0

#define CL_DEVICE_SVM_CAPABILITIES	0x104C

#define CL_COMMAND_SVM_FREE                         0x1209
#define CL_COMMAND_SVM_MEMCPY                       0x1210
#define CL_COMMAND_SVM_MEMFILL                      0x1211
#define CL_COMMAND_SVM_MAP                          0x1212
#define CL_COMMAND_SVM_UNMAP                        0x1213

#define CL_DEVICE_MAX_PIPE_ARGS                     0x1050
#define CL_DEVICE_PIPE_MAX_ACTIVE_RESERVATIONS      0x1051
#define CL_DEVICE_PIPE_MAX_PACKET_SIZE              0x1052

#define CL_MEM_OBJECT_PIPE                          0x10F7

/* cl_pipe_info */
#define CL_PIPE_PACKET_SIZE                         0x1120
#define CL_PIPE_MAX_PACKETS                         0x1121
#define CL_PIPE_UNIFORM_RES_PKT_CNT                 0x1122

#define CL_DEVICE_MAX_GLOBAL_VARIABLE_SIZE          0x10000
#define CL_DEVICE_GLOBAL_VARIABLE_SHARING           0x10001
#define CL_PROGRAM_BUILD_GLOBAL_VARIABLE_TOTAL_SIZE 0x10002

// #define CL_MEM_OBJECT_IMAGE2D_BUFFER                0x10ABC

// type definitions

typedef int cl_svm_mem_flags;
/* First three are already defined in cl.h as part of cl_mem_flags:
CL_MEM_READ_WRITE				= 1 << 0,
CL_MEM_WRITE_ONLY				= 1 << 1,
CL_MEM_READ_ONLY				= 1 << 2, */	

#define CL_DEVICE_SVM_COARSE_GRAIN_BUFFER               (1 << 0) 
#define	CL_MEM_SVM_FINE_GRAIN_BUFFER	                (1 << 3)
#define CL_MEM_SVM_ATOMICS				                (1 << 4)

typedef int cl_device_svm_capabilities;
#define CL_DEVICE_SVM_FINE_GRAIN_BUFFER	(1 << 0)
#define CL_DEVICE_SVM_FINE_GRAIN_SYSTEM	(1 << 1)
#define CL_DEVICE_SVM_ATOMICS			(1 << 2)

typedef int cl_kernel_exec_info;
#define CL_KERNEL_EXEC_INFO_SVM_PTRS                  0x10E1 
#define CL_KERNEL_EXEC_INFO_SVM_FINE_GRAIN_SYSTEM     0x10E2

typedef cl_bitfield         cl_pipe_attributes;
typedef cl_uint             cl_pipe_info;

typedef intptr_t cl_sampler_properties;


// API functions

extern CL_API_ENTRY void* CL_API_CALL
clSVMAlloc(cl_context /*context*/,
		   cl_svm_mem_flags /*flags*/,
		   size_t /*size*/,
		   unsigned int /*alignment*/) CL_API_SUFFIX__VERSION_2_0;

extern CL_API_ENTRY void CL_API_CALL
clSVMFree(cl_context /*context*/,
		  void* /*svm_pointer*/) CL_API_SUFFIX__VERSION_2_0;

extern CL_API_ENTRY cl_int CL_API_CALL
clEnqueueSVMFree(cl_command_queue /*command_queue*/,
				 cl_uint /*num_svm_pointers*/,
				 void* /*svm_pointers*/[],
				 void (CL_CALLBACK* /*pfn_free_func*/)(
					   cl_command_queue /*queue*/,
					   cl_uint /*num_svm_pointers*/,
					   void* /*svm_pointers*/[],
					   void* /*user_data*/),
				 void* /*user_data*/,
				 cl_uint /*num_events_in_wait_list*/,
				 const cl_event* /*event_wait_list*/,
				 cl_event* /*event*/) CL_API_SUFFIX__VERSION_2_0;

extern CL_API_ENTRY cl_int CL_API_CALL
clEnqueueSVMMemcpy(cl_command_queue /*command_queue*/,
				   cl_bool /*blocking_copy*/,
				   void* /*dst_ptr*/,
				   const void* /*src_ptr*/,
				   size_t /*size*/,
				   cl_uint /*num_events_in_wait_list*/,
				   const cl_event* /*event_wait_list*/,
				   cl_event* /*event*/) CL_API_SUFFIX__VERSION_2_0;

extern CL_API_ENTRY cl_int CL_API_CALL
clEnqueueSVMMemFill(cl_command_queue /*command_queue*/,
					void* /*svm_ptr*/,
					const void* /*pattern*/,
					size_t /*pattern_size*/,
					size_t /*size*/,
					cl_uint /*num_events_in_wait_list*/,
					const cl_event* /*event_wait_list*/,
					cl_event* /*event*/) CL_API_SUFFIX__VERSION_2_0;

extern CL_API_ENTRY cl_int CL_API_CALL
clEnqueueSVMMap(cl_command_queue /*command_queue*/,
				cl_bool /*blocking_map*/,
				cl_map_flags /*map_flags*/,
				void* /*svm_ptr*/,
				size_t /*size*/,
				cl_uint /*num_events_in_wait_list*/,
				const cl_event* /*event_wait_list*/,
				cl_event* /*event*/) CL_API_SUFFIX__VERSION_2_0;

extern CL_API_ENTRY cl_int CL_API_CALL
clEnqueueSVMUnmap(cl_command_queue /*command_queue*/,
				  void* /*svm_ptr*/,
				  cl_uint /*num_events_in_wait_list*/,
				  const cl_event* /*event_wait_list*/,
				  cl_event* /*event*/) CL_API_SUFFIX__VERSION_2_0;

extern CL_API_ENTRY cl_int CL_API_CALL
clSetKernelArgSVMPointer(cl_kernel /*kernel*/,
						 cl_uint /*arg_index*/,
						 const void* /*arg_value*/) CL_API_SUFFIX__VERSION_2_0;

extern CL_API_ENTRY cl_int CL_API_CALL
clSetKernelExecInfo(cl_kernel /*kernel*/,
					cl_kernel_exec_info /*param_name*/,
					size_t /*param_value_size*/,
					const void* /*param_value*/) CL_API_SUFFIX__VERSION_2_0;

extern CL_API_ENTRY cl_mem CL_API_CALL
    clCreatePipe(cl_context context,
    cl_mem_flags flags, 
    cl_uint pipe_packet_size,
    cl_uint pipe_max_packets,
    const cl_pipe_attributes *attributres,
    cl_int *errcode_ret) CL_API_SUFFIX__VERSION_2_0;

extern CL_API_ENTRY cl_int CL_API_CALL
    clGetPipeInfo(cl_mem pipe,
    cl_pipe_info param_name,
    size_t param_value_size,
    void *param_value,
    size_t *param_value_size_ret) CL_API_SUFFIX__VERSION_2_0;
					
#ifdef __cplusplus
}
#endif
