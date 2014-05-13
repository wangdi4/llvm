/******************************************************************
 //
 //  OpenCL Conformance Tests
 // 
 //  Copyright:	(c) 2008-2013 by Apple Inc. All Rights Reserved.
 //
 ******************************************************************/

#include <stdio.h>
#include <stdlib.h>

#if !defined(_WIN32)
#include <stdbool.h>
#endif

#include <math.h>
#include <string.h>
#include "procs.h"
#include "harness/testHarness.h"

#if !defined(_WIN32)
#include <unistd.h>
#endif

// FIXME: To use certain functions in harness/imageHelpers.h
// (for example, generate_random_image_data()), the tests are required to declare
// the following variables (<rdar://problem/11111245>):
cl_device_type gDeviceType = CL_DEVICE_TYPE_DEFAULT;
bool gTestRounding = false;

basefn	basefn_list[] = {
	test_repeated_setup_cleanup,
	test_create_kernels_in_program,
	test_set_kernel_arg_struct_array,
	test_retain_queue_multiple,
	test_create_context_from_type,
	test_program_binary_create,
	test_get_sampler_info,
	test_get_buffer_info,
	test_native_kernel,
	test_enqueue_task,
	test_retain_mem_object_multiple,
	test_get_command_queue_info,
	test_get_platform_ids,
	test_get_kernel_info,
	test_bufferreadwriterect,
	test_arraycopy,
	test_arrayimagecopy,
	test_hostptr,
	test_imagearraycopy,
	test_simple_write_image_pitch,
	test_partition_equally,
	test_image_migrate,
	test_simple_link_with_callback,
	test_event_enqueue_marker,
	test_event_flush,
	test_event_enqueue_barrier_with_list,
	test_buffer_fill_struct,
	test_callbacks,
	//execute,
	
};


const char    *basefn_names[] = {
	"repeated_setup_cleanup",
	"create_kernels_in_program",
	"set_kernel_arg_struct_array",
	"retain_queue_multiple",
	"create_context_from_type",
	"program_binary_create",
	"get_sampler_info",
	"get_buffer_info",
	"native_kernel",
	"enqueue_task",
	"retain_mem_object_multiple",
	"get_command_queue_info",
	"get_platform_ids",
	"get_kernel_info",
	"bufferreadwriterect",
	"arraycopy",
	"arrayimagecopy",
	"hostptr",
	"imagearraycopy",
	"simple_write_image_pitch",
	"partition_equally",
	"image_migrate",
	"simple_link_with_callback",
	"event_enqueue_marker",
	"event_flush",
	"event_enqueue_barrier_with_list",
	"buffer_fill_struct",
	"callbacks",
	//"execute",
	"all",
};

ct_assert((sizeof(basefn_names) / sizeof(basefn_names[0]) - 1) == (sizeof(basefn_list) / sizeof(basefn_list[0])));

int	num_fns = sizeof(basefn_names) / sizeof(char *);

int main(int argc, const char *argv[])
{
	return runTestHarness( argc, argv, num_fns, basefn_list, basefn_names, false, false, 0 );
}

