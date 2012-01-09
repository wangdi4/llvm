// Copyright (c) 2006-2012 Intel Corporation
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
//
// ocl_wrapper.h


#ifndef OCL_WRAPPER_GTEST_
#define OCL_WRAPPER_GTEST_

#include <CL/cl.h>
#include <CL/cl_ext.h>
#include <gtest/gtest.h>
#include "dynamic_array.h"

template <typename T>
class DynamicArray;

/**
 *	OpenCLDescriptor - encapsulates commonly used OpenCL objects.
 *	Some assertions might throw, this class' will make sure these OpenCL objects are always released.
 **/
class OpenCLDescriptor{
public:
	cl_platform_id platforms[2];
	cl_device_id devices[2];

	cl_context context;
	cl_context cpu_context;
	cl_context gpu_context;
	cl_command_queue queues[2];
	cl_mem in_common_buffer;
	cl_mem out_common_buffer;
	cl_mem buffers[2];
	cl_mem in_common_sub_buffer;

	cl_program program;
	cl_kernel kernels[2];

public:
	OpenCLDescriptor()
	{
		// initialize to 0 all OpenCL objects
		context = 0;
		cpu_context = 0;
		gpu_context = 0;
		in_common_buffer = 0;
		out_common_buffer = 0;
		in_common_sub_buffer = 0;
		program = 0;

		for(int i=0; i<2; ++i){
			platforms[i]=0;
			devices[i] = 0;
			queues[i] = 0;
			kernels[i] = 0;
			buffers[i] = 0;
		}
	}
	
	~OpenCLDescriptor()
	{
		// release in_common_sub_buffer
		if(0!=in_common_sub_buffer){
			EXPECT_EQ(CL_SUCCESS, clReleaseMemObject(in_common_sub_buffer)) << "clReleaseMemObject failed";
			in_common_sub_buffer = 0;
		}
		// release out_common_buffer
		if(0!=out_common_buffer){
			EXPECT_EQ(CL_SUCCESS, clReleaseMemObject(out_common_buffer)) << "clReleaseMemObject failed";
			out_common_buffer = 0;
		}
		// release in_common_buffer
		if(0!=in_common_buffer){
			EXPECT_EQ(CL_SUCCESS, clReleaseMemObject(in_common_buffer)) << "clReleaseMemObject failed";
			in_common_buffer = 0;
		}
		for(int i=0; i<2; ++i)
		{
			// release buffers[i]
			if(0!=buffers[i])
			{
				EXPECT_EQ(CL_SUCCESS, clReleaseMemObject(buffers[i])) << "clReleaseMemObject failed";
				buffers[i] = 0;
			}
			// release kernels
			if (0 != kernels[i])
			{
				EXPECT_EQ(CL_SUCCESS, clReleaseKernel(kernels[i])) << "clReleaseKernel failed";
				kernels[i] = 0;
			}				
		}
		// release program
		if (0 != program)
		{
			EXPECT_EQ(CL_SUCCESS, clReleaseProgram(program)) << "clReleaseProgram failed";
			program = 0;
		}
		// release queues
		for(int i=0; i<2; ++i){
			if (0 != queues[i])
			{
				EXPECT_EQ(CL_SUCCESS, clReleaseCommandQueue(queues[i])) << "clReleaseCommandQueue failed";
				queues[i] = 0;
			}
		}
		// release context
		if (0 != context)
		{
			EXPECT_EQ(CL_SUCCESS, clReleaseContext(context)) << "clReleaseContext failed";
			context = 0;
		}
		if (0 != cpu_context)
		{
			EXPECT_EQ(CL_SUCCESS, clReleaseContext(cpu_context)) << "clReleaseContext failed";
			cpu_context = 0;
		}
		if (0 != gpu_context)
		{
			EXPECT_EQ(CL_SUCCESS, clReleaseContext(gpu_context)) << "clReleaseContext failed";
			gpu_context = 0;
		}
	}
};

/**
 * Encapsulates main OpenCL function calls and their validation
 **/

// getPlatformIDs - calls and validates clGetPlatformIDs
void getPlatformIDs(cl_uint num_entries, cl_platform_id* platforms, cl_uint* num_platforms);

// getPlatformInfo - calls and validates clGetPlatformInfo
void getPlatformInfo(cl_platform_id platform, cl_platform_info param_name, size_t param_value_size, void *param_value);

//	getDeviceIDs - calls and validates clGetDeviceIDs 
void getDeviceIDs(cl_platform_id platform, cl_device_type device_type, cl_uint num_entries, cl_device_id* devices,
	cl_uint* num_devices);
	
//	getCPUDevice - returns CPU platfrom and device
void getCPUDevice(cl_platform_id* platform, cl_device_id* device);

//	getGPUDevice - returns GPU platfrom and device
void getGPUDevice(cl_platform_id* platform, cl_device_id* device);

//	getCPUGPUDevices - returns CPU and GPU platfrom and devices (index 0 for CPU device and index 1 for GPU device)
//	devices must be an array of at least 2 elements
void getCPUGPUDevices(cl_platform_id* platform, cl_device_id* devices);

//	getDeviceInfo - calls and validates clGetDeviceInfo
void getDeviceInfo(cl_device_id device,  cl_device_info param_name, size_t param_value_size, void *param_value);

// createContext - calls and validates clCreateContext
void createContext(cl_context* context, cl_context_properties* properties, cl_uint num_devices, cl_device_id* devices, 
	void (CL_CALLBACK *pfn_notify)(const char *errinfo, const void *private_info, size_t cb, void *user_data), void *user_data);

// getContextInfo - calls and validates clGetContextInfo
void getContextInfo(cl_context* context, cl_context_info param_name, size_t param_value_size, void *param_value);

// createCommandQueue - calls and validates clCreateCommandQueue
void createCommandQueue(cl_command_queue* queue, cl_context context, cl_device_id device, 
	cl_command_queue_properties properties);
		
// createBuffer - calls and validates clCreateBuffer
void createBuffer(cl_mem* buffer, cl_context context, cl_mem_flags flags, size_t size, void *host_ptr);

// createBuffer - calls and validates clCreateSubBuffer
void createSubBuffer(cl_mem* sub_buffer, cl_mem buffer, cl_mem_flags flags, cl_buffer_create_type buffer_create_type, 
	const void *buffer_create_info);

//	createImage2D - calls and validates clCreateImage2D
void createImage2D(cl_mem* image2D, cl_context context, cl_mem_flags flags, 
	const cl_image_format *image_format, size_t image_width, 
	size_t image_height, size_t image_row_pitch, void *host_ptr);

//	createImage3D - calls and validates clCreateImage3D
void createImage3D(cl_mem* image3D, cl_context context, cl_mem_flags flags, const cl_image_format *image_format,
	size_t image_width, size_t image_height, size_t image_depth, size_t image_row_pitch, size_t image_slice_pitch,
	void *host_ptr);

//	createImage3D - calls and validates clCreateImage3D
void createImage3D(cl_mem* image3D, cl_context context, cl_mem_flags flags, const cl_image_format *image_format,
	size_t image_width, size_t image_height, size_t image_depth, size_t image_row_pitch, size_t image_slice_pitch,
	void *host_ptr);

// createProgramWithSource - calls and validates clCreateProgramWithSource
// uses programSource which is actual kernel source
void createProgramWithSource(cl_program* program, cl_context context,	cl_uint count, const char **programSource,
	const size_t *lengths);

// createProgramWithSourceFromKernelName - calls and validates clCreateProgramWithSource
// uses clFileName
void createProgramWithSourceFromKernelName(cl_program* program, cl_context context, 
	const char *sFileName);

// buildProgram - calls and validates clBuildProgram
void buildProgram (cl_program* program, cl_uint num_devices, const cl_device_id *device_list, const char *options,
	void (CL_CALLBACK *pfn_notify)(cl_program program, void *user_data), void *user_data);

//getProgramInfo - calls and validates clGetProgramInfo
void getProgramInfo(cl_program program, cl_program_info param_name,
                         size_t param_value_size, void *param_value, size_t *param_value_size_ret);

// getProgramBuildInfo - calls and validates clGetProgramBuildInfo
void getProgramBuildInfo(cl_program program, cl_device_id device, cl_program_build_info param_name,
	size_t param_value_size, void *param_value, size_t *param_value_size_ret);

// createAndBuildProgramWithSource - calls and validates clCreateProgramWithSource and clBuildProgram using kernel file name
void createAndBuildProgramWithSource(const char* sFileName, cl_program* program, cl_context context, 
	cl_uint num_devices, const cl_device_id *device_list, const char *options, void (CL_CALLBACK *pfn_notify)(cl_program program, void *user_data), 
	void *user_data);

// createAndBuildProgramWithStringSource - calls and validates clCreateBuffer clCreateProgramWithSource and clBuildProgram using kernel source
void createAndBuildProgramWithStringSource(const char* kernelSource, cl_program* program, cl_context context,
	cl_uint num_devices, const cl_device_id *device_list, const char *options, 
	void (CL_CALLBACK *pfn_notify)(cl_program program, void *user_data), 
	void *user_data);

//createProgramWithBinary - calls and validates clCreateProgramWithBinary
void createProgramWithBinary(cl_program* program, cl_context context, cl_uint num_devices, const cl_device_id *device_list, const size_t *lengths,
                             const unsigned char **binaries, cl_int *binary_status);

// createKernel - calls and validates clCreateKernel
void createKernel(cl_kernel* kernel , cl_program program, const char *kernel_name);

// createKernelsInProgram - calls and validates clCreateKernelsInProgram
void createKernelsInProgram(cl_program program, cl_uint num_kernels, cl_kernel *kernels, cl_uint *num_kernels_ret);

// setKernelArg - calls and validates clSetKernelArg
void setKernelArg(cl_kernel kernel, cl_uint arg_index, size_t arg_size, const void *arg_value);

// enqueueNDRangeKernel - calls and validates clEnqueueNDRangeKernel
void enqueueNDRangeKernel(cl_command_queue command_queue, cl_kernel kernel, 
	cl_uint work_dim, const size_t *global_work_offset, const size_t *global_work_size, const size_t *local_work_size, 
	cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *clevent);

// createUserEvent - calls and validates clCreateUserEvent
void createUserEvent(cl_event* user_event, cl_context context);

// createUserEvent - calls and validates createSampler
void createSampler (cl_sampler* sampler, cl_context context, cl_bool normalized_coords, 
	cl_addressing_mode addressing_mode, cl_filter_mode filter_mode);

// setUserEventStatus - calls and validates clSetUserEventStatus
void setUserEventStatus(cl_event user_event, cl_int execution_status);

//	getEventInfo - calls and validates clGetEventInfo
void getEventInfo(cl_event clevent, cl_event_info param_name, size_t param_value_size, void *param_value);

// validateQueuedOrSubmitted - validates that clevent is either CL_QUEUED or CL_SUBMITTED
void validateQueuedOrSubmitted(cl_event clevent);

// waitForEvents - calls and validates clWaitForEvents
void waitForEvents(cl_uint num_events, const cl_event *event_list);

// enqueueMapBuffer - calls and validates clEnqueueMapBuffer
template<typename T>
void enqueueMapBuffer(T** arrayToMap, cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_map, 
	cl_map_flags map_flags, size_t offset, size_t cb, cl_uint num_events_in_wait_list, 
	const cl_event *event_wait_list, cl_event *end_event)
{
	if(NULL==arrayToMap){
		ASSERT_TRUE(false) << "Null argument provided";
	}
		cl_int errcode_ret = CL_SUCCESS;
	*arrayToMap = (T*)clEnqueueMapBuffer (command_queue, buffer, blocking_map, map_flags, 
		offset, cb, num_events_in_wait_list,  event_wait_list, end_event, &errcode_ret);
	ASSERT_EQ(CL_SUCCESS, errcode_ret) << "clEnqueueMapBuffer failed";
}

// enqueueUnmapMemObject - calls and validates clEnqueueUnmapMemObject
void enqueueUnmapMemObject(cl_command_queue command_queue, cl_mem memobj, void *mapped_ptr,
	cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *user_event);

//	enqueueMapImage - calls and validates clEnqueueMapImage
template<typename T>
void enqueueMapImage(T** arrayToMap, cl_command_queue command_queue, cl_mem image, cl_bool blocking_map, 
	cl_map_flags map_flags, const size_t origin[3], const size_t region[3], 
	size_t *image_row_pitch, size_t *image_slice_pitch,
	cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *end_event)
	{
	if(NULL==arrayToMap){
		ASSERT_TRUE(false) << "Null argument provided";
	}
	cl_int errcode_ret = CL_SUCCESS;
	*arrayToMap = (T*)clEnqueueMapImage(command_queue, image, blocking_map, map_flags, origin, region, image_row_pitch,
		image_slice_pitch, num_events_in_wait_list, event_wait_list, end_event, &errcode_ret);
	ASSERT_EQ(CL_SUCCESS, errcode_ret) << "clEnqueueMapImage failed";
}

// enqueueReadBuffer - calls and validates clEnqueueReadBuffer
void enqueueReadBuffer(cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_read, size_t offset, size_t cb, 
	void *ptr, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *end_event);

// enqueueWriteBuffer - calls and validates clEnqueueWriteBuffer
void enqueueWriteBuffer(cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_write, size_t offset, size_t cb, 
	void *ptr, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *end_event);

// enqueueReadImage - calls and validates clEnqueueReadImage
void enqueueReadImage(cl_command_queue command_queue, cl_mem image, cl_bool blocking_read, const size_t origin[3],
	const size_t region[3], size_t row_pitch, size_t slice_pitch, void *ptr, cl_uint num_events_in_wait_list, 
	const cl_event *event_wait_list, cl_event *end_event);

// enqueueWriteImage - calls and validates clEnqueueWriteImage
void enqueueWriteImage(cl_command_queue command_queue, cl_mem image, cl_bool blocking_write, const size_t origin[3],
	const size_t region[3], size_t input_row_pitch, size_t input_slice_pitch, const void * ptr,
	cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *end_event);

// enqueueReadBufferRect - calls and validates clEnqueueReadBufferRect
void enqueueReadBufferRect(cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_read,
	const size_t buffer_origin[3], const size_t host_origin[3], const size_t region[3],
	size_t buffer_row_pitch, size_t buffer_slice_pitch, 
	size_t host_row_pitch, size_t host_slice_pitch,
	void *ptr, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *end_event);

// enqueueWriteBufferRect - calls and validates clEnqueueWriteBufferRect
void enqueueWriteBufferRect(cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_write,
	const size_t buffer_origin[3], const size_t host_origin[3], const size_t region[3],
	size_t buffer_row_pitch, size_t buffer_slice_pitch, 
	size_t host_row_pitch, size_t host_slice_pitch,
	void *ptr, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *end_event);

// enqueueCopyBuffer - calls and validates clEnqueueCopyBuffer
void enqueueCopyBuffer(cl_command_queue command_queue, cl_mem src_buffer, cl_mem dst_buffer,
	size_t src_offset, size_t dst_offset, size_t cb, cl_uint num_events_in_wait_list, 
	const cl_event *event_wait_list, cl_event *end_event);

// enqueueCopyBufferRect - calls and validates clEnqueueCopyBufferRect
void enqueueCopyBufferRect(cl_command_queue command_queue, cl_mem src_buffer, cl_mem dst_buffer,
	const size_t src_origin[3], const size_t dst_origin[3], const size_t region[3],
	size_t src_row_pitch, size_t src_slice_pitch, size_t dst_row_pitch, size_t dst_slice_pitch,
	cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *end_event);

// enqueueCopyImage - calls and validates clEnqueueCopyImage
void enqueueCopyImage(cl_command_queue command_queue, cl_mem src_image, cl_mem dst_image, 
	const size_t src_origin[3], const size_t dst_origin[3], const size_t region[3],
	cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *end_event);

// enqueueCopyImageToBuffer - calls and validates clEnqueueCopyImageToBuffer
void enqueueCopyImageToBuffer(cl_command_queue command_queue, cl_mem src_image, cl_mem dst_buffer,
	const size_t src_origin[3], const size_t region[3], size_t dst_offset, cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list, cl_event *end_event);

// enqueueCopyBufferToImage - calls and validates clEnqueueCopyBufferToImage
void enqueueCopyBufferToImage(cl_command_queue command_queue, cl_mem src_buffer, cl_mem dst_image, 
	size_t src_offset, const size_t dst_origin[3], const size_t region[3], 
	cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *end_event);

// enqueueTask - calls and validates clEnqueueTask
void enqueueTask(cl_command_queue command_queue, cl_kernel kernel, 
	cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *end_event);

// enqueueNativeKernel - calls and validates clEnqueueNativeKernel
/*void enqueueNativeKernel(cl_command_queue command_queue, void (*user_func)(void *) void *args, size_t cb_args,
	cl_uint num_mem_objects, const cl_mem *mem_list, const void **args_mem_loc,
	cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *end_event);*/

// enqueueMarker - calls and validates clEnqueueMarker
void enqueueMarker(cl_command_queue command_queue, cl_event *end_event);

//	setUpSingleContextProgramQueue - creates and validate a context, program and queue device of type device_type
void setUpSingleContextProgramQueue(const char* kernelFileName, cl_context* context, cl_program* program, 
	cl_command_queue* queue, cl_device_type device_type);

//	setUpContextProgramQueues - creates and validate shared context, program and separate queues for CPU and GPU on a single platform
void setUpContextProgramQueues(OpenCLDescriptor& ocl_descriptor, const char* kernelFileName);

//	setUpContextProgramQueues - creates and validate shared context, program and separate queues for CPU and GPU on a single platform
void setUpContextProgramQueuesFromStringSource(OpenCLDescriptor& ocl_descriptor, const char* kernelSource);

//	executeSetAndExecuteKernels - for both CPU and GPU devices creates and sets kernels with arguments
//		cl_mem input
//		cl_mem output
//		int size
//	These kernels are initialized with kernel kernelName, it copies content of input buffer into output buffer
//	Executes kernels, reads their contents and validates thir correctness
//	Buffer's element type in host is T (needed for validation)
template<typename T>
void executeSetAndExecuteKernelsCopy(const char* kernelName, DynamicArray<T>& input_array, cl_mem in_common_buffer, 
	cl_mem* buffers, cl_kernel* kernels, cl_command_queue* queues, cl_context context, cl_program program, int arraySize)
{
	// set work dimensions
	cl_uint work_dim = 1;
	size_t global_work_size = 1;

	// create queue, kernel and set kernel args for each device
	for(int i=0; i<2; ++i)
	{
		DynamicArray<T> output_array(arraySize);

		// create separate output buffer
		ASSERT_NO_FATAL_FAILURE(createBuffer(&buffers[i], context, CL_MEM_READ_WRITE|CL_MEM_ALLOC_HOST_PTR, 
			sizeof(T)*arraySize, NULL));

		// create kernel
		ASSERT_NO_FATAL_FAILURE(createKernel(&kernels[i] ,program, kernelName));
		// set kernel arguments
		ASSERT_NO_FATAL_FAILURE(setKernelArg(kernels[i], 0, sizeof(cl_mem), &in_common_buffer));
		ASSERT_NO_FATAL_FAILURE(setKernelArg(kernels[i], 1, sizeof(cl_mem), &buffers[i]));
		ASSERT_NO_FATAL_FAILURE(setKernelArg(kernels[i], 2, sizeof(cl_int), &arraySize));

		// enqueue kernel and map buffer
		ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(queues[i], kernels[i], 1, NULL, &global_work_size, NULL, 0, NULL,  NULL));

		// read from buffers
		ASSERT_NO_FATAL_FAILURE(enqueueReadBuffer(queues[i], buffers[i], CL_TRUE, 0,sizeof(T)*arraySize, 
			output_array.dynamic_array, 0, NULL, NULL));
		// will return when read from buffers are done

		// validate results
		ASSERT_NO_FATAL_FAILURE(input_array.compareArray(output_array));
	}
}

//	executeSetAndExecuteKernelsSum - for both CPU and GPU devices creates and sets kernels with arguments
//		cl_mem input
//		cl_mem sum
//		int size
//		
//	These kernels are initialized with kernel kernelName, kernels will evaluate sum of all elelemnts in input
//	image and write the result to sum.
//	Executes kernels, reads their contents and validates thir correctness
//	input is an image (2d or 3d) (in host T4) and sum in host is a pointer to an element of type float.
//	Kernels for 2d images assume that image coordinates are 2x2, and for 3d - 2x2x2
template<typename T>
void executeSetAndExecuteKernelsSum(const char* kernelName, DynamicArray<T>& input_array, 
	cl_mem& in_common_buffer, cl_mem* buffers, 
	cl_kernel* kernels, cl_command_queue* queues, cl_context context, cl_program program, 
	float divisor=1, int succDevicesNum = 2)
{
	// set work dimensions
	cl_uint work_dim = 1;
	size_t global_work_size = 1;

	// create queue, kernel and set kernel args for each device
	for(int i=0; i<2; ++i)
	{
		float sum = 0;

		// create separate output buffer
		ASSERT_NO_FATAL_FAILURE(createBuffer(&buffers[i], context, CL_MEM_READ_WRITE|CL_MEM_USE_HOST_PTR, 
			sizeof(cl_float), &sum));

		// create kernel
		ASSERT_NO_FATAL_FAILURE(createKernel(&kernels[i] ,program, kernelName));
		// set kernel arguments
		ASSERT_NO_FATAL_FAILURE(setKernelArg(kernels[i], 0, sizeof(cl_mem), &in_common_buffer));
		ASSERT_NO_FATAL_FAILURE(setKernelArg(kernels[i], 1, sizeof(cl_mem), &buffers[i]));

		if(i==succDevicesNum || 2==succDevicesNum)
		{
			// enqueue kernel and map buffer
			ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(queues[i], kernels[i], 1, NULL, &global_work_size, NULL, 0, NULL,  NULL));
		}
		else
		{
			// enqueue should fail
			ASSERT_NE(CL_SUCCESS, clEnqueueNDRangeKernel(queues[i], kernels[i], 1, NULL, &global_work_size, NULL, 0, NULL,  NULL)) << "clEnqueueNDRangeKernel did not fail";
			continue;
		}
		// read from buffers
		ASSERT_NO_FATAL_FAILURE(enqueueReadBuffer(queues[i], buffers[i], CL_TRUE, 0,sizeof(float), 
			&sum, 0, NULL, NULL));
		// will return when read from buffers are done
		if(1!=divisor){
			sum=sum*divisor;
		}
		// validate results
		if(succDevicesNum >1 || succDevicesNum==i){
			ASSERT_NO_FATAL_FAILURE(input_array.compareArraySum(sum));
		}
	}
}

// setMemObjectDestructorCallback - calls and validates clSetMemObjectDestructorCallback
void setMemObjectDestructorCallback(cl_mem memobj, 
	void (CL_CALLBACK *pfn_notify)(cl_mem memobj, void *user_data), void *user_data);

// deleteArray - deletes dynamic_array
void CL_CALLBACK deleteArray(cl_mem memobj, void *dynamic_array);

// setDeleteArrayCallback - sets deleteArray function as callback for memobj's release for parameter input_array
// As a result input_array's destructor will have no affect. 
// Instead, descrutcion of input_array will be within callback function of memobj's release.
// This is recommended for cases when memobj was created with CL_MEM_USE_HOST_PTR on input_array.dynamic_array
template<typename T>
void setDeleteArrayOnCallback(cl_mem memobj, DynamicArray<T>& input_array)
{
	ASSERT_NO_FATAL_FAILURE(setMemObjectDestructorCallback(memobj,&deleteArray,input_array.dynamic_array)); 
	input_array.disableDestructor();
}

// setReleaseMemObjOnCallback - releases memobj_to_release in a callback of release of memobj
// Recommended for sub-buffers of buffers created with CL_MEM_USE_HOST_PTR
void setReleaseMemObjOnCallback(cl_mem memobj, cl_mem* memobj_to_release);

// getSupportedImageFormats - calls and validates clGetSupportedImageFormats
void getSupportedImageFormats(cl_context context, cl_mem_flags flags, cl_mem_object_type image_type,
	cl_uint num_entries, cl_image_format *image_formats, cl_uint *num_image_formats);

// isImageFormatSupported - returns true iff context supports 
// image format defined by flags, image_type and wanted_image_format
// Required devices are determined by succDevicesNum
void isImageFormatSupported(cl_context context, cl_mem_flags flags, cl_mem_object_type image_type, 
	cl_image_format wanted_image_format, bool* isSupported);

// isImageFormatSupported - determines if CPU and GPU support given format
// ret_val_cpu = true iff given format is supported by CPU
// ret_val_gpu = true iff given format is supported by GPU
void isImageFormatSupported(cl_mem_flags flags, cl_mem_object_type image_type, 
	cl_image_format wanted_image_format, bool* ret_val_cpu, bool* ret_val_gpu);

// isImageFormatSupportedByRequiredDevices - checks if given image format is supported by requiredDevices
// if requiredDevices==0 then CPU needs to support this format
// if requiredDevices==1 then GPU needs to support this format
// if requiredDevices==2 then both CPU and GPU need to support this format
// ret_val - return value - true iff the given format is supported by the required devices
void isImageFormatSupportedByRequiredDevices(cl_mem_flags flags, cl_mem_object_type image_type, 
	cl_image_format wanted_image_format, int requiredDevices, bool* ret_val);

// isExtensionSupportedOnDevice - returns true iff all required devices support requiredExtension
// Required devices are determined by succDevicesNum
// succDevicesNum = 0 means that CPU is the required device
// succDevicesNum = 1 means that GPU is the required device
// succDevicesNum = 2 means that both CPU and GPU are required
void isExtensionSupportedOnDevice(const char* requiredExtension, int succDevicesNum, bool* ret_val);

//////////////////////////////////////////////////////////////////////////////////////////////////
//	Fission functions
//////////////////////////////////////////////////////////////////////////////////////////////////

// releaseDevice - calls and validates clReleaseDevice
void releaseDevice(cl_device_id device );

// retainDevice - calls and validates clRetainDevice
void retainDevice(cl_device_id device);

// clCreateSubDevices - calls and validates clCreateSubDevices
void createSubDevices(cl_device_id in_device, 
	const cl_device_partition_property_ext * properties,
	cl_uint num_entries, cl_device_id *out_devices, cl_uint *num_devices);

// createPartitionByCounts - creates numSubDevices subdevices for root device in_device
// with property CL_DEVICE_PARTITION_BY_COUNTS_*
void createPartitionByCounts(cl_device_id in_device, cl_device_id* out_devices, cl_uint numSubDevices);

// getCPUGPUDevicesIfNotCreated - calls getCPUGPUDevices unless
// devices in ocl_descriptor were already initialized
// (in SetUp() devices are initialized to 0)
// This function is used in test bodies which can be called by both regular root devices tests
// as well as fission tests.
// In case of root devices - devices member of ocl_descriptor is uninitialized
// And in case of subdevices (or mixture of subdevices with root devices) -
// devices member of ocl_descriptor will already be already initialized
void getCPUGPUDevicesIfNotCreated(OpenCLDescriptor& ocl_descriptor);

#endif /* OCL_WRAPPER_GTEST_ */