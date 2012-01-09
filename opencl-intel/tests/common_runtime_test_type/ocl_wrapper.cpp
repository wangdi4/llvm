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

#include "ocl_wrapper.h"
#include "common_methods.h"

#include <iostream>
#include <fstream>

// getPlatformIDs - calls and validates clGetPlatformIDs
void getPlatformIDs(cl_uint num_entries, cl_platform_id* platforms, cl_uint* num_platforms)
{
	if(NULL==platforms)
	{
		ASSERT_TRUE(false) << "Null argument provided";
	}
	ASSERT_EQ(CL_SUCCESS, clGetPlatformIDs (num_entries, platforms, num_platforms)) << "clGetPlatformIDs failed";
}

// getSpecificDevice - returns platfrom and device ids for device of type device_type
void getSpecificDevice(cl_platform_id* platform, cl_device_id* device, cl_device_type device_type)
{
	if(NULL==platform || NULL==device)
	{
		ASSERT_TRUE(false) << "Null argument provided";
	}
	cl_platform_id platforms[4];
	cl_uint num_entries = 4;
	cl_uint num_platforms = 0;
	cl_device_id devices[2];
	cl_uint num_devices = 0;
	cl_device_type param_value;
	size_t param_value_size = sizeof(cl_device_type);
	// get at most 4 platfroms avilable
	getPlatformIDs(num_entries, platforms, &num_platforms);
	// more than 1 platfrom found
	for(unsigned int i=0; i<num_platforms; ++i){
		// iterate over all platfroms
		num_entries = 2;
		// expect return of at most 2 devices
		getDeviceIDs(platforms[i], device_type, num_entries, devices, &num_devices);
		for(unsigned int j=0; j<num_devices; ++j){
			// iterate over all devices returned
			// obtain their device info for CL_DEVICE_TYPE
			getDeviceInfo(devices[j], CL_DEVICE_TYPE,  param_value_size, &param_value);
			if(device_type == param_value){
				// found requested device
				*platform = platforms[i];
				*device = devices[j];
				return;
			}
		}
	}
	//	did not find requested device
	ASSERT_TRUE(false) << "Did not find reqested device";
}

//	getCPUDevice - returns CPU platfrom and device
void getCPUDevice(cl_platform_id* platform, cl_device_id* device){
	if(NULL==platform||NULL==device)
	{
		ASSERT_TRUE(false) << "Null argument provided";
	}
	ASSERT_NO_FATAL_FAILURE(getSpecificDevice(platform, device, CL_DEVICE_TYPE_CPU));
}

//	getGPUDevice - returns GPU platfrom and device
void getGPUDevice(cl_platform_id* platform, cl_device_id* device){
	if(NULL==platform || NULL==device)
	{
		ASSERT_TRUE(false) << "Null argument provided";
	}
	ASSERT_NO_FATAL_FAILURE(getSpecificDevice(platform, device, CL_DEVICE_TYPE_GPU));
}

//	getCPUGPUDevices - returns CPU and GPU platfrom and devices (index 0 for CPU device and index 1 for GPU device)
//	devices must be an array of at least 2 elements
void getCPUGPUDevices(cl_platform_id* platform, cl_device_id* devices)
{
	if(NULL==platform || NULL==devices)
	{
		ASSERT_TRUE(false) << "Null argument provided";
	}
	cl_platform_id platforms[4];
	cl_uint num_entries = 4;
	cl_uint num_platforms = 0;
	cl_uint num_devices = 0;
	cl_device_type param_value;
	size_t param_value_size = sizeof(cl_device_type);
	// get at most 4 platfroms avilable
	ASSERT_NO_FATAL_FAILURE(getPlatformIDs(num_entries, platforms, &num_platforms));
	// more than 1 platfrom found
	// reset all devices
	for(int d=0;d<2;++d){
		devices[d]=0;
	}	
	for(unsigned int i=0; i<num_platforms; ++i){
		// iterate over all platfroms
		platform[0] = platforms[i];
		// reset all devices
		for(int d=0;d<2;++d){
			devices[d]=0;
		}
		ASSERT_TRUE(num_entries>=2) << "Platform returned " << num_entries << "deviuces";
		// expect return of at most 2 devices
		cl_device_id* tmpDevices = NULL;
		tmpDevices = new cl_device_id[num_entries];
		getDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, num_entries, tmpDevices, &num_devices);
		for(unsigned int j=0; j<num_devices; ++j){
			// iterate over all devices returned
			// obtain their device info for CL_DEVICE_TYPE
			
			getDeviceInfo(tmpDevices[j], CL_DEVICE_TYPE,  param_value_size, &param_value);
			if(CL_DEVICE_TYPE_CPU == param_value){
				// found requested device				
				devices[0] = tmpDevices[j];
			}
			if(CL_DEVICE_TYPE_GPU == param_value){
				// found requested device				
				devices[1] = tmpDevices[j];
			}
			if(0!=devices[0] && 0!=devices[1]){
				// found both devices on the same platform
				if(NULL!=tmpDevices){
					delete[] tmpDevices;
					tmpDevices = NULL;
				}
				return;
			}
		}
		if(NULL!=tmpDevices){
			delete[] tmpDevices;
			tmpDevices = NULL;
		}
	}
	//	did not find requested device
	ASSERT_TRUE(false) << "Did not find reqested device";
}

// getPlatformInfo - calls and validates clGetPlatformInfo
void getPlatformInfo(cl_platform_id platform, cl_platform_info param_name, size_t param_value_size, void *param_value)
{
	EXPECT_EQ(CL_SUCCESS, clGetPlatformInfo(platform, param_name, param_value_size, param_value, NULL)) << "clGetPlatformInfo failed";
}

//	getDeviceIDs - calls and validates clGetDeviceIDs 
void getDeviceIDs(cl_platform_id platform, cl_device_type device_type, cl_uint num_entries, cl_device_id* devices, 
				  cl_uint* num_devices)
{
	ASSERT_EQ(CL_SUCCESS, clGetDeviceIDs(platform, device_type, num_entries, devices, num_devices)) << "clGetDeviceIDs failed";
}

//	getDeviceInfo - calls and validates clGetDeviceInfo
void getDeviceInfo(cl_device_id device,  cl_device_info param_name, size_t param_value_size, void *param_value)
{
	EXPECT_EQ(CL_SUCCESS, clGetDeviceInfo (device ,param_name ,param_value_size, param_value, NULL)) << "clGetDeviceInfo failed";
}

// createContext - calls and validates clCreateContext
void createContext(cl_context* context, cl_context_properties* properties, cl_uint num_devices, cl_device_id* devices, 
				   void (CL_CALLBACK *pfn_notify)(const char *errinfo, const void *private_info, size_t cb, void *user_data), void *user_data)
{
	if(NULL==context){
		ASSERT_TRUE(false) << "Null argument provided";
	}
	cl_int errcode_ret = CL_SUCCESS;
	*context = clCreateContext (properties,num_devices,devices,pfn_notify,user_data,&errcode_ret);
	ASSERT_EQ(CL_SUCCESS, errcode_ret) << "clCreateContext failed";
	ASSERT_NE((cl_context)0, *context) << "clCreateContext returned 0 as context value";
}

// getContextInfo - calls and validates clGetContextInfo
void getContextInfo(cl_context* context, cl_context_info param_name, size_t param_value_size, void *param_value)
{
	EXPECT_EQ(CL_SUCCESS, clGetContextInfo (*context, param_name, param_value_size, param_value, NULL)) << "clGetContextInfo failed";
}

// createCommandQueue - calls and validates clCreateCommandQueue
void createCommandQueue(cl_command_queue* queue, cl_context context, cl_device_id device, 
						cl_command_queue_properties properties)
{
	if(NULL==queue){
		ASSERT_TRUE(false) << "Null argument provided";
	}
	cl_int errcode_ret = CL_SUCCESS;
	*queue = clCreateCommandQueue (context, device, properties, &errcode_ret);
	ASSERT_EQ(CL_SUCCESS, errcode_ret) << "clCreateCommandQueue failed";
	ASSERT_NE((cl_command_queue)0, *queue) << "clCreateCommandQueue returned 0 as queue value";
}

// createBuffer - calls and validates clCreateBuffer
void createBuffer(cl_mem* buffer, cl_context context, cl_mem_flags flags, size_t size, void *host_ptr)
{
	if(NULL==buffer){
		ASSERT_TRUE(false) << "Null argument provided";
	}
	cl_int errcode_ret = CL_SUCCESS;
	*buffer = clCreateBuffer (context, flags, size, host_ptr, &errcode_ret);
	ASSERT_EQ(CL_SUCCESS, errcode_ret) << "clCreateBuffer failed";
	ASSERT_NE((cl_mem)0, *buffer) << "clCreateBuffer returned 0 as buffer value";
}

// createBuffer - calls and validates clCreateSubBuffer
void createSubBuffer(cl_mem* sub_buffer, cl_mem buffer, cl_mem_flags flags, cl_buffer_create_type buffer_create_type, 
	const void *buffer_create_info)
{
	if(NULL==sub_buffer){
		ASSERT_TRUE(false) << "Null argument provided";
	}
	cl_int errcode_ret = CL_SUCCESS;
	*sub_buffer = clCreateSubBuffer (buffer, flags, buffer_create_type, buffer_create_info,	&errcode_ret);
	ASSERT_EQ(CL_SUCCESS, errcode_ret) << "clCreateSubBuffer failed";
	ASSERT_NE((cl_mem)0, *sub_buffer) << "clCreateSubBuffer returned 0 as sub-buffer value";
}

//	createImage2D - calls and validates clCreateImage2D
void createImage2D(cl_mem* image2D, cl_context context, cl_mem_flags flags, const cl_image_format *image_format, 
				   size_t image_width, 
	size_t image_height, size_t image_row_pitch, void *host_ptr)
{
	if(NULL==image2D){
		ASSERT_TRUE(false) << "Null argument provided";
	}
	cl_int errcode_ret = CL_SUCCESS;
	*image2D = clCreateImage2D (context, flags, image_format, image_width, image_height, image_row_pitch,
		host_ptr, &errcode_ret);
	ASSERT_EQ(CL_SUCCESS, errcode_ret) << "clCreateImage2D failed";
	ASSERT_NE((cl_mem)0, *image2D) << "clCreateImage2D returned 0 as image2D value";
}

//	createImage3D - calls and validates clCreateImage3D
void createImage3D(cl_mem* image3D, cl_context context, cl_mem_flags flags, const cl_image_format *image_format,
	size_t image_width, size_t image_height, size_t image_depth, size_t image_row_pitch, size_t image_slice_pitch,
	void *host_ptr)
{
	if(NULL==image3D){
		ASSERT_TRUE(false) << "Null argument provided";
	}
	cl_int errcode_ret = CL_SUCCESS;
	*image3D = clCreateImage3D (context, flags, image_format, image_width, image_height, image_depth, 
		image_row_pitch, image_slice_pitch, host_ptr, &errcode_ret);
	ASSERT_EQ(CL_SUCCESS, errcode_ret) << "clCreateImage3D failed";
	ASSERT_NE((cl_mem)0, *image3D) << "clCreateImage3D returned 0 as image3D value";
}

// createProgramWithSource - calls and validates clCreateProgramWithSource
// uses programSource which is actual kernel source
void createProgramWithSource(cl_program* program, cl_context context,	cl_uint count, const char **programSource,
	const size_t *lengths)
{
	if(NULL ==program || NULL ==programSource){
		ASSERT_TRUE(false) << "Null argument provided";
	}
	cl_int errcode_ret = CL_SUCCESS;
	
	*program = clCreateProgramWithSource (context, 1, programSource, NULL, &errcode_ret);
	
	ASSERT_EQ(CL_SUCCESS, errcode_ret) << "clCreateProgramWithSource failed";
	ASSERT_NE((cl_program)0, *program) << "clCreateProgramWithSource returned 0 as program value";
}

// createProgramWithSourceFromKernelName - calls and validates clCreateProgramWithSource
// uses clFileName
void createProgramWithSourceFromKernelName(cl_program* program, cl_context context, 
	const char *sFileName)
{
	if(NULL==sFileName || NULL ==program){
		ASSERT_TRUE(false) << "Null argument provided";
	}
	cl_int errcode_ret = CL_SUCCESS;
	const char* kernelSource = NULL;
	// read kernels file
	ASSERT_NO_FATAL_FAILURE(fileToBuffer(&kernelSource, sFileName));
	// create program
	ASSERT_NO_FATAL_FAILURE(createProgramWithSource(program, context, 1, &kernelSource, NULL));
}

// buildProgram - calls and validates clBuildProgram
void buildProgram (cl_program* program, cl_uint num_devices, const cl_device_id *device_list, const char *options,
	void (CL_CALLBACK *pfn_notify)(cl_program program, void *user_data), void *user_data)
{
	if(NULL==program || NULL==device_list){
		ASSERT_TRUE(false) << "Null argument provided";
	}
	cl_int errcode_ret = clBuildProgram (*program, num_devices, device_list, options, pfn_notify, user_data);
	if(NULL!=device_list)
	{
		if(CL_SUCCESS != errcode_ret)
		{
			//	prints build fail log
			cl_int logStatus;
			char * buildLog = NULL;
			size_t buildLogSize = 0;
			logStatus = clGetProgramBuildInfo(*program, device_list[0], CL_PROGRAM_BUILD_LOG, buildLogSize, buildLog, &buildLogSize);

			buildLog = (char*) malloc(buildLogSize);
			memset(buildLog, 0, buildLogSize);

			logStatus = clGetProgramBuildInfo(*program, device_list[0], CL_PROGRAM_BUILD_LOG, buildLogSize, buildLog, NULL);

			std::cout << " \n\t\t\tBUILD LOG\n";
			std::cout << " ************************************************\n";
			std::cout << buildLog << std::endl;
			std::cout << " ************************************************\n";
			free(buildLog);
		}
	}
	ASSERT_EQ(CL_SUCCESS, errcode_ret)  << "clBuildProgram failed";
}

// getProgramInfo - calls and validates clGetProgramInfo
void getProgramInfo( cl_program program, cl_program_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret )
{
  cl_int errcode_ret = clGetProgramInfo(program, param_name, param_value_size, param_value, param_value_size_ret);
  ASSERT_EQ(CL_SUCCESS, errcode_ret)  << "clGetProgramInfo failed";
}

// getProgramBuildInfo - calls and validates clGetProgramBuildInfo
void getProgramBuildInfo(cl_program program, cl_device_id device, cl_program_build_info param_name,
	size_t param_value_size, void *param_value, size_t *param_value_size_ret)
{
	cl_int errcode_ret = clGetProgramBuildInfo(program, device, param_name, param_value_size, param_value,
		param_value_size_ret);
	ASSERT_EQ(CL_SUCCESS, errcode_ret)  << "clGetProgramBuildInfo failed";
}

// createAndBuildProgramWithSource - calls and validates clCreateBuffer clCreateProgramWithSource and clBuildProgram using kernel file name
void createAndBuildProgramWithSource(const char* sFileName, cl_program* program, cl_context context,
	cl_uint num_devices, const cl_device_id *device_list, const char *options, 
	void (CL_CALLBACK *pfn_notify)(cl_program program, void *user_data),  void *user_data)
{
	if(NULL==sFileName || NULL ==program || NULL==device_list){
		ASSERT_TRUE(false) << "Null argument provided";
	}
	cl_int errcode_ret = CL_SUCCESS;
	const char* kernelSource = NULL;
	// read kernels file
	ASSERT_NO_FATAL_FAILURE(fileToBuffer(&kernelSource, sFileName));
	ASSERT_NO_FATAL_FAILURE(createAndBuildProgramWithStringSource(kernelSource, program, context, num_devices, device_list, options, pfn_notify,
		user_data));
	if(NULL!=kernelSource)
	{
		delete[] kernelSource;
		kernelSource = NULL;
	}
}

// createAndBuildProgramWithStringSource - calls and validates clCreateBuffer clCreateProgramWithSource and clBuildProgram using kernel source
void createAndBuildProgramWithStringSource(const char* kernelSource, cl_program* program, cl_context context,
	cl_uint num_devices, const cl_device_id *device_list, const char *options, 
	void (CL_CALLBACK *pfn_notify)(cl_program program, void *user_data), 
void *user_data)
{
	// create program
	ASSERT_NO_FATAL_FAILURE(createProgramWithSource(program, context,	
		1, &kernelSource, NULL));

	// build program
	ASSERT_NO_FATAL_FAILURE( buildProgram (program, num_devices, device_list, options, pfn_notify, user_data));
}

// createProgramWithBinary - calls and validates clCreateProgramWithBinary
void createProgramWithBinary( cl_program* program, cl_context context, cl_uint num_devices, const cl_device_id *device_list, const size_t *lengths,
                               const unsigned char **binaries, cl_int *binary_status )
{
	if(NULL == program){
		ASSERT_TRUE(false) << "Null argument provided";
	}
	cl_int errcode_ret = CL_SUCCESS;

	*program = clCreateProgramWithBinary(context, num_devices, device_list, lengths, binaries, binary_status, binary_status);

	ASSERT_EQ(CL_SUCCESS, errcode_ret) << "clCreateProgramWithBinary failed";
	ASSERT_NE((cl_program)0, *program) << "clCreateProgramWithBinary returned 0 as program value";
}

// createKernel - calls and validates clCreateKernel
void createKernel(cl_kernel* kernel , cl_program program, const char *kernel_name)
{
	if(NULL==kernel){
		ASSERT_TRUE(false) << "Null argument provided";
	}
	cl_int errcode_ret = CL_SUCCESS;
	*kernel = clCreateKernel (program, kernel_name, &errcode_ret);
	ASSERT_EQ(CL_SUCCESS, errcode_ret) << "clCreateKernel failed";
	ASSERT_NE((cl_kernel)0, *kernel) << "clCreateKernel returned 0 as kernel value";
}

// createKernelsInProgram - calls and validates clCreateKernelsInProgram
void createKernelsInProgram(cl_program program, cl_uint num_kernels, cl_kernel *kernels, cl_uint *num_kernels_ret)
{
	ASSERT_EQ(CL_SUCCESS, clCreateKernelsInProgram(program, num_kernels, kernels, num_kernels_ret)) << "clCreateKernelsInProgram faled"; 
}

// setKernelArg - calls and validates clSetKernelArg
void setKernelArg(cl_kernel kernel, cl_uint arg_index, size_t arg_size, const void *arg_value)
{
	ASSERT_EQ(CL_SUCCESS, clSetKernelArg (kernel, arg_index, arg_size, arg_value)) << "clSetKernelArg failed";
}

// enqueueNDRangeKernel - calls and validates clEnqueueNDRangeKernel
void enqueueNDRangeKernel(cl_command_queue command_queue, cl_kernel kernel, 
	cl_uint work_dim, const size_t *global_work_offset, const size_t *global_work_size, const size_t *local_work_size, 
	cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *clevent)
{
	ASSERT_EQ(CL_SUCCESS, clEnqueueNDRangeKernel(command_queue, kernel, work_dim, global_work_offset, global_work_size, local_work_size, 
		num_events_in_wait_list, event_wait_list, clevent)) << "clEnqueueNDRangeKernel failed";
}

// createUserEvent - calls and validates clCreateUserEvent
void createUserEvent(cl_event* user_event, cl_context context)
{
	if(NULL==user_event){
		ASSERT_TRUE(false) << "Null argument provided";
	}
	cl_int errcode_ret = CL_SUCCESS;
	*user_event = clCreateUserEvent(context, &errcode_ret);
	ASSERT_EQ(CL_SUCCESS, errcode_ret) << "clCreateUserEvent failed";
	ASSERT_NE((cl_event)0, *user_event) << "clCreateUserEvent returned 0 as event value";
}

// createSampler - calls and validates clCreateSampler
void createSampler (cl_sampler* sampler, cl_context context, cl_bool normalized_coords, 
	cl_addressing_mode addressing_mode, cl_filter_mode filter_mode)
{
	if(NULL==sampler){
		ASSERT_TRUE(false) << "Null argument provided";
	}
	cl_int errcode_ret = CL_SUCCESS;
	*sampler = clCreateSampler(context, normalized_coords, addressing_mode, filter_mode, &errcode_ret);
	ASSERT_EQ(CL_SUCCESS,errcode_ret) << "clCreateSampler failed";
}

// setUserEventStatus - calls and validates clSetUserEventStatus
void setUserEventStatus(cl_event user_event, cl_int execution_status)
{
	ASSERT_EQ(CL_SUCCESS, clSetUserEventStatus (user_event, execution_status)) << "clSetUserEventStatus failed";
}

//	getEventInfo - calls and validates clGetEventInfo
void getEventInfo(cl_event clevent, cl_event_info param_name, size_t param_value_size, void *param_value){ 
	ASSERT_EQ(CL_SUCCESS, clGetEventInfo (clevent, param_name,  param_value_size, param_value, NULL)) << "clGetEventInfo failed";
}

// validateQueuedOrSubmitted - validates that clevent is either CL_QUEUED or CL_SUBMITTED
void validateQueuedOrSubmitted(cl_event clevent)
{
	size_t	param_value_size = sizeof(cl_int);
	cl_int param_value_event_status = 0;
	ASSERT_NO_FATAL_FAILURE(getEventInfo(clevent, CL_EVENT_COMMAND_EXECUTION_STATUS, param_value_size, &param_value_event_status));
	if(CL_QUEUED != param_value_event_status && CL_SUBMITTED != param_value_event_status)
	{
		EXPECT_TRUE(false) << "Not  is CL_QUEUED and not CL_SUBMITTED";
	}
}

// waitForEvents - calls and validates clWaitForEvents
void waitForEvents(cl_uint num_events, const cl_event *event_list)
{
	ASSERT_EQ(CL_SUCCESS, clWaitForEvents (num_events, event_list)) << "clWaitForEvents failed";
}

// enqueueUnmapMemObject - calls and validates clEnqueueUnmapMemObject
void enqueueUnmapMemObject(cl_command_queue command_queue, cl_mem memobj, void *mapped_ptr,
	cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *user_event)
{
	ASSERT_EQ(CL_SUCCESS, clEnqueueUnmapMemObject(command_queue, memobj, mapped_ptr, 
		num_events_in_wait_list, event_wait_list, user_event))<<"clEnqueueUnmapMemObject failed";
}

// enqueueReadBuffer - calls and validates clEnqueueReadBuffer
void enqueueReadBuffer(cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_read, size_t offset, size_t cb, 
	void *ptr, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *end_event)
{
	ASSERT_EQ(CL_SUCCESS, clEnqueueReadBuffer(command_queue, buffer,  blocking_read, offset, cb, ptr,
		num_events_in_wait_list, event_wait_list, end_event)) << "clEnqueueReadBuffer failed";;
}

// enqueueWriteBuffer - calls and validates clEnqueueWriteBuffer
void enqueueWriteBuffer(cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_write, size_t offset, size_t cb, 
	void *ptr, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *end_event)
{
	ASSERT_EQ(CL_SUCCESS, clEnqueueWriteBuffer(command_queue, buffer, blocking_write, offset, cb, ptr,
		num_events_in_wait_list, event_wait_list, end_event)) << "clEnqueueWriteBuffer failed";;
}

// enqueueReadImage - calls and validates clEnqueueReadImage
void enqueueReadImage(cl_command_queue command_queue, cl_mem image, cl_bool blocking_read, const size_t origin[3],
	const size_t region[3], size_t row_pitch, size_t slice_pitch, void *ptr, cl_uint num_events_in_wait_list, 
	const cl_event *event_wait_list, cl_event *end_event)
{
	ASSERT_EQ(CL_SUCCESS, clEnqueueReadImage(command_queue, image, blocking_read, origin, region,
		row_pitch, slice_pitch, ptr, num_events_in_wait_list, event_wait_list, end_event));
}

// enqueueWriteImage - calls and validates clEnqueueWriteImage
void enqueueWriteImage(cl_command_queue command_queue, cl_mem image, cl_bool blocking_write, const size_t origin[3],
	const size_t region[3], size_t input_row_pitch, size_t input_slice_pitch, const void * ptr,
	cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *end_event)
{
	ASSERT_EQ(CL_SUCCESS, clEnqueueWriteImage(command_queue, image, blocking_write, origin, region,
		input_row_pitch, input_slice_pitch, ptr, num_events_in_wait_list, event_wait_list, end_event));
}

// enqueueReadBufferRect - calls and validates clEnqueueReadBufferRect
void enqueueReadBufferRect(cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_read,
	const size_t buffer_origin[3], const size_t host_origin[3], const size_t region[3],
	size_t buffer_row_pitch, size_t buffer_slice_pitch, 
	size_t host_row_pitch, size_t host_slice_pitch,
	void *ptr, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *end_event)
{
	ASSERT_EQ(CL_SUCCESS, clEnqueueReadBufferRect(command_queue, buffer, blocking_read, buffer_origin,
		host_origin, region, buffer_row_pitch, buffer_slice_pitch, host_row_pitch, host_slice_pitch,
		ptr, num_events_in_wait_list, event_wait_list, end_event)) << "clEnqueueReadBufferRect failed";
}

// enqueueWriteBufferRect - calls and validates clEnqueueWriteBufferRect
void enqueueWriteBufferRect(cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_write,
	const size_t buffer_origin[3], const size_t host_origin[3], const size_t region[3],
	size_t buffer_row_pitch, size_t buffer_slice_pitch, 
	size_t host_row_pitch, size_t host_slice_pitch,
	void *ptr, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *end_event)
{
	ASSERT_EQ(CL_SUCCESS, clEnqueueWriteBufferRect(command_queue, buffer, blocking_write, buffer_origin,
		host_origin, region, buffer_row_pitch, buffer_slice_pitch, host_row_pitch, host_slice_pitch,
		ptr, num_events_in_wait_list, event_wait_list, end_event)) << "clEnqueueWriteBufferRect failed";
}

// enqueueCopyBuffer - calls and validates clEnqueueCopyBuffer
void enqueueCopyBuffer(cl_command_queue command_queue, cl_mem src_buffer, cl_mem dst_buffer,
	size_t src_offset, size_t dst_offset, size_t cb, cl_uint num_events_in_wait_list, 
	const cl_event *event_wait_list, cl_event *end_event)
{
	ASSERT_EQ(CL_SUCCESS, clEnqueueCopyBuffer(command_queue, src_buffer, dst_buffer, src_offset, dst_offset,
		cb, num_events_in_wait_list, event_wait_list, end_event)) << "clEnqueueCopyBuffer failed";
}

// enqueueCopyBufferRect - calls and validates clEnqueueCopyBufferRect
void enqueueCopyBufferRect(cl_command_queue command_queue, cl_mem src_buffer, cl_mem dst_buffer,
	const size_t src_origin[3], const size_t dst_origin[3], const size_t region[3],
	size_t src_row_pitch, size_t src_slice_pitch, size_t dst_row_pitch, size_t dst_slice_pitch,
	cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *end_event)
{
	ASSERT_EQ(CL_SUCCESS, clEnqueueCopyBufferRect(command_queue, src_buffer, dst_buffer,
		src_origin, dst_origin, region, src_row_pitch, src_slice_pitch,
		dst_row_pitch, dst_slice_pitch, num_events_in_wait_list, event_wait_list, end_event)) << "clEnqueueCopyBufferRect failed";
}

// enqueueCopyImage - calls and validates clEnqueueCopyImage
void enqueueCopyImage(cl_command_queue command_queue, cl_mem src_image, cl_mem dst_image, 
	const size_t src_origin[3], const size_t dst_origin[3], const size_t region[3],
	cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *end_event)
{
	ASSERT_EQ(CL_SUCCESS, clEnqueueCopyImage(command_queue, src_image, dst_image,
		src_origin, dst_origin, region, num_events_in_wait_list, event_wait_list, end_event)) << "clEnqueueCopyImage done";
}

// enqueueCopyImageToBuffer - calls and validates clEnqueueCopyImageToBuffer
void enqueueCopyImageToBuffer(cl_command_queue command_queue, cl_mem src_image, cl_mem dst_buffer,
	const size_t src_origin[3], const size_t region[3], size_t dst_offset, cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list, cl_event *end_event)
{
	ASSERT_EQ(CL_SUCCESS, clEnqueueCopyImageToBuffer(command_queue, src_image, dst_buffer, 
		src_origin, region, dst_offset, num_events_in_wait_list, event_wait_list, end_event)) << "clEnqueueCopyImageToBuffer failed";
}

// enqueueCopyBufferToImage - calls and validates clEnqueueCopyBufferToImage
void enqueueCopyBufferToImage(cl_command_queue command_queue, cl_mem src_buffer, cl_mem dst_image, 
	size_t src_offset, const size_t dst_origin[3], const size_t region[3], 
	cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *end_event)
{
	ASSERT_EQ(CL_SUCCESS, clEnqueueCopyBufferToImage(command_queue, src_buffer, dst_image,
		src_offset, dst_origin, region, 
		num_events_in_wait_list, event_wait_list, end_event)) << "clEnqueueCopyBufferToImage failed";
}

// enqueueTask - calls and validates clEnqueueTask
void enqueueTask(cl_command_queue command_queue, cl_kernel kernel, 
	cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *end_event)
{
	ASSERT_EQ(CL_SUCCESS, clEnqueueTask(command_queue, kernel, num_events_in_wait_list,
		event_wait_list, end_event)) << "clEnqueueTask failed";
}

// enqueueMarker - calls and validates clEnqueueMarker
void enqueueMarker(cl_command_queue command_queue, cl_event *end_event)
{
	ASSERT_EQ(CL_SUCCESS, clEnqueueMarker(command_queue, end_event)) << "clEnqueueMarker failed";
}

//	setUpContextProgramQueues - creates and validate shared context, program and separate queues for CPU and GPU on a single platform
void setUpContextProgramQueues(OpenCLDescriptor& ocl_descriptor, const char* kernelFileName)
{
	if(NULL==kernelFileName){
		ASSERT_TRUE(false) << "Null argument provided";
	}

	// get pltfrom and device ids
	ASSERT_NO_FATAL_FAILURE(getCPUGPUDevicesIfNotCreated(ocl_descriptor));
	// cpu is at index 0, gpu is at index 1

	// create context
	ASSERT_NO_FATAL_FAILURE(createContext(&ocl_descriptor.context, 0, 2, ocl_descriptor.devices, NULL, NULL));

	//	create and build program
	ASSERT_NO_FATAL_FAILURE(createAndBuildProgramWithSource(kernelFileName, &ocl_descriptor.program, ocl_descriptor.context, 2, ocl_descriptor.devices, NULL, NULL, NULL));

	// create queues
	for(int i=0; i<2; ++i)
	{
		// create queue
		ASSERT_NO_FATAL_FAILURE(createCommandQueue(&ocl_descriptor.queues[i], ocl_descriptor.context, ocl_descriptor.devices[i], 0));
	}
}

//	setUpSingleContextProgramQueue - creates and validate a context, program and queue device of type device_type
void setUpSingleContextProgramQueue(const char* kernelFileName, cl_context* context, cl_program* program, 
									 cl_command_queue* queues, cl_device_type device_type)
{
	cl_platform_id platforms[1];
	cl_device_id devices[1];

	// get pltfrom and device id
	ASSERT_NO_FATAL_FAILURE(getSpecificDevice(platforms, devices, device_type));

	// create context
	ASSERT_NO_FATAL_FAILURE(createContext(context, 0, 1, devices, NULL, NULL));

	//	create and build program
	ASSERT_NO_FATAL_FAILURE(createAndBuildProgramWithSource(kernelFileName, program, *context, 1, devices, NULL, NULL, NULL));

	// create queue
	ASSERT_NO_FATAL_FAILURE(createCommandQueue(&queues[0], *context, devices[0], 0));
}

//	setUpContextProgramQueues - creates and validate shared context, program and separate queues for CPU and GPU on a single platform
void setUpContextProgramQueuesFromStringSource(OpenCLDescriptor& ocl_descriptor, const char* kernelSource)
{
	// get pltfrom and device ids
	ASSERT_NO_FATAL_FAILURE(getCPUGPUDevicesIfNotCreated(ocl_descriptor));
	// cpu is at index 0, gpu is at index 1

	// create context
	cl_context_properties properties[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)ocl_descriptor.platforms[0], 0};
	ASSERT_NO_FATAL_FAILURE(createContext(&ocl_descriptor.context, properties, 2, ocl_descriptor.devices, NULL, NULL));

	//	create and build program
	ASSERT_NO_FATAL_FAILURE(createAndBuildProgramWithStringSource(kernelSource, &ocl_descriptor.program, ocl_descriptor.context, 2, ocl_descriptor.devices, NULL, NULL, NULL));

	// create queues
	for(int i=0; i<2; ++i)
	{
		// create queue
		ASSERT_NO_FATAL_FAILURE(createCommandQueue(&ocl_descriptor.queues[i], ocl_descriptor.context, ocl_descriptor.devices[i], 0));
	}
}

void initializeHalfArray(cl_half* input_array, int arraySize){
	//ASSERT_NE(0, input_array);
	cl_platform_id platforms[1];
	cl_device_id devices[1];
	cl_program program = 0;
	cl_context context = 0;
	cl_command_queue queue = 0;
	cl_mem buffer = 0;
	cl_kernel kernel = 0;

	// get pltfrom and device ids
	ASSERT_NO_FATAL_FAILURE(getCPUDevice(platforms, devices));
	// cpu is at index 0, gpu is at index 1

	// create context
	cl_context_properties properties[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)platforms[0], 0};
	ASSERT_NO_FATAL_FAILURE(createContext(&context, properties, 1, devices, NULL, NULL));

	//	create and build program
	ASSERT_NO_FATAL_FAILURE(createAndBuildProgramWithSource("init_half.cl", &program, context, 1, devices, NULL, NULL, NULL));

	// create queues
	// create queue
	ASSERT_NO_FATAL_FAILURE(createCommandQueue(&queue, context, devices[0], 0));

	// create buffer
	ASSERT_NO_FATAL_FAILURE(createBuffer(&buffer, context, 
		CL_MEM_READ_WRITE|CL_MEM_ALLOC_HOST_PTR, arraySize * sizeof(cl_half), NULL));

	// set work dimensions
	cl_uint work_dim = 1;
	size_t global_work_size = 1;

	// create kernel
	ASSERT_NO_FATAL_FAILURE(createKernel(&kernel ,program, "init_half"));

	// set kernel arguments
	ASSERT_NO_FATAL_FAILURE(setKernelArg(kernel, 0, sizeof(cl_mem), &buffer));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(kernel, 1, sizeof(int), &arraySize));
	
	// enqueue kernel and map buffer
	ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(queue, kernel, 1, NULL, &global_work_size, NULL, 0, NULL,  NULL));

	// read from buffers
	ASSERT_NO_FATAL_FAILURE(enqueueReadBuffer(queue, buffer, CL_TRUE, 0,sizeof(cl_half)*arraySize, 
		&input_array, 0, NULL, NULL));

	// release buffers[i]
	if(0!=buffer)
	{
		EXPECT_EQ(CL_SUCCESS, clReleaseMemObject(buffer)) << "clReleaseMemObject failed";
		buffer = 0;
	}
	// release kernels
	if (0 != kernel)
	{
		EXPECT_EQ(CL_SUCCESS, clReleaseKernel(kernel)) << "clReleaseKernel failed";
		kernel = 0;
	}				
	
	// release program
	if (0 != program)
	{
		EXPECT_EQ(CL_SUCCESS, clReleaseProgram(program)) << "clReleaseProgram failed";
		program = 0;
	}
	// release queues
	if (0 != queue)
	{
		EXPECT_EQ(CL_SUCCESS, clReleaseCommandQueue(queue)) << "clReleaseCommandQueue failed";
		queue = 0;
	}
	// release context
	if (0 != context)
	{
		EXPECT_EQ(CL_SUCCESS, clReleaseContext(context)) << "clReleaseContext failed";
		context = 0;
	}
}

// setMemObjectDestructorCallback - calls and validates clSetMemObjectDestructorCallback
void setMemObjectDestructorCallback(cl_mem memobj, 
	void (CL_CALLBACK *pfn_notify)(cl_mem memobj, void *user_data), void *user_data)
{
	ASSERT_EQ(CL_SUCCESS, clSetMemObjectDestructorCallback (memobj, pfn_notify, user_data)) << "clSetMemObjectDestructorCallback failed";
}

// deleteArray - deletes dynamic_array that was created in DynamicArray's class
void CL_CALLBACK deleteArray(cl_mem memobj, void *dynamic_array)
{
	//std::cout << "CALLBACK deleteArray" << std::endl;
	if(NULL==dynamic_array){
		return;
	}
#ifdef _WIN32
	_aligned_free(dynamic_array);
#else
	free(dynamic_array);
#endif
	dynamic_array = NULL;
}

// releaseSubBuffer - releases memobj_to_release in a callback of release of memobj
void CL_CALLBACK releaseSubBuffer(cl_mem memobj, void* memobj_to_release)
{
	 cl_mem* memobj_to_release_ = (cl_mem*)memobj_to_release;
	// release memobj_to_release
	if(0!=*memobj_to_release_){
		EXPECT_EQ(CL_SUCCESS, clReleaseMemObject(*memobj_to_release_)) << "clReleaseMemObject failed";
		*memobj_to_release_ = 0;
	}
}

// setReleaseMemObjOnCallback - releases memobj_to_release in a callback of release of memobj
// Recommended for sub-buffers of buffers created with CL_MEM_USE_HOST_PTR
void setReleaseMemObjOnCallback(cl_mem memobj, cl_mem* memobj_to_release)
{
	ASSERT_NO_FATAL_FAILURE(setMemObjectDestructorCallback(memobj, &releaseSubBuffer, memobj_to_release)); 
}

// getSupportedImageFormats - calls and validates clGetSupportedImageFormats
void getSupportedImageFormats(cl_context context, cl_mem_flags flags, cl_mem_object_type image_type,
	cl_uint num_entries, cl_image_format *image_formats, cl_uint *num_image_formats)
{
	ASSERT_EQ(CL_SUCCESS, clGetSupportedImageFormats(context, flags, image_type,
		num_entries, image_formats, num_image_formats));
}

// isImageFormatSupported - returns true iff context supports 
// image format defined by flags, image_type and wanted_image_format
// Required devices are determined by succDevicesNum
void isImageFormatSupported(cl_context context, cl_mem_flags flags, cl_mem_object_type image_type, 
	cl_image_format wanted_image_format, bool* isSupported)
{
	if(NULL==isSupported)
	{
		ASSERT_TRUE(false) << "Null argument provided";
	}
	*isSupported=false;
	cl_uint num_entries = 0;
	cl_uint num_image_formats = 0;

	// find out how many image formats are supported
	ASSERT_NO_FATAL_FAILURE(getSupportedImageFormats(context, flags, image_type,
		num_entries, NULL, &num_image_formats));

	// get all supported image formats
	cl_image_format *image_formats = NULL;
	image_formats = new cl_image_format[num_image_formats];
	ASSERT_NO_FATAL_FAILURE(getSupportedImageFormats(context, flags, image_type,
		num_image_formats, image_formats, NULL));
	
	// check if wanted_image_format is supported
	for(int i=0; i<num_image_formats; ++i)
	{
		if(wanted_image_format.image_channel_order == image_formats[i].image_channel_order &&
			wanted_image_format.image_channel_data_type == image_formats[i].image_channel_data_type)
		{
			*isSupported = true;
			break;
		}
	}
	delete[] image_formats;
}

// isImageFormatSupported - determines if CPU and GPU support given format
// ret_val_cpu = true iff given format is supported by CPU
// ret_val_gpu = true iff given format is supported by GPU
void isImageFormatSupported(cl_mem_flags flags, cl_mem_object_type image_type, 
	cl_image_format wanted_image_format, bool* ret_val_cpu, bool* ret_val_gpu)
{
	if(NULL==ret_val_cpu || NULL == ret_val_gpu)
	{
		ASSERT_TRUE(false) << "Null argument provided";
	}
	cl_platform_id platforms[] = {0,0};
	cl_device_id devices[] = {0,0};
	cl_context contexts[] = {0,0};

	// get both devices
	ASSERT_NO_FATAL_FAILURE(getCPUGPUDevices(platforms, devices));

	// get context for each device
	for(int i=0; i<2; ++i)
	{
		// checking this device's supported image formats
		ASSERT_NO_FATAL_FAILURE(createContext(&contexts[i], NULL, 1, &devices[i], NULL, NULL));
		ASSERT_NO_FATAL_FAILURE(isImageFormatSupported(contexts[i], flags, image_type, wanted_image_format, (0==i)?ret_val_cpu:ret_val_gpu));
		// release context
		if (0 != contexts[i])
		{
			EXPECT_EQ(CL_SUCCESS, clReleaseContext(contexts[i])) << "clReleaseContext failed";
			contexts[i] = 0;
		}
	}
}

// isImageFormatSupportedByRequiredDevices - checks if given image format is supported by requiredDevices
// if requiredDevices==0 then CPU needs to support this format
// if requiredDevices==1 then GPU needs to support this format
// if requiredDevices==2 then both CPU and GPU need to support this format
// ret_val - return value - true iff the given format is supported by the required devices
void isImageFormatSupportedByRequiredDevices(cl_mem_flags flags, cl_mem_object_type image_type, 
	cl_image_format wanted_image_format, int requiredDevices, bool* ret_val)
{
	if(NULL==ret_val)
	{
		ASSERT_TRUE(false) << "Null argument provided";
	}
	bool isSupportedByCPU = false;
	bool isSupportedByGPU = false;
	// check which devices support given format
	ASSERT_NO_FATAL_FAILURE(isImageFormatSupported(flags, image_type, 
		wanted_image_format, &isSupportedByCPU, &isSupportedByGPU));
	if((0==requiredDevices || 2==requiredDevices) && false == isSupportedByCPU)
	{
		// does npt support CPU
		std::cout << "Format is not supported on CPU" << std::endl;
		*ret_val = false;
		return;
	}
	if((1==requiredDevices || 2==requiredDevices) && false == isSupportedByGPU)
	{
		// does not support GPU
		std::cout << "Format is not supported on GPU" << std::endl;
		*ret_val = false;
		return;
	}
	*ret_val = true;
}

// isExtensionSupportedOnDevice - returns true iff all required devices support requiredExtension
// Required devices are determined by succDevicesNum
// succDevicesNum = 0 means that CPU is the required device
// succDevicesNum = 1 means that GPU is the required device
// succDevicesNum = 2 means that both CPU and GPU are required
void isExtensionSupportedOnDevice(const char* requiredExtension, int succDevicesNum, bool* ret_val)
{
	if(NULL==requiredExtension || NULL==ret_val)
	{
		ASSERT_TRUE(false) << "Null argument provided";
	}
	*ret_val = false;
	cl_platform_id platforms[] = {0,0};
	cl_device_id devices[] = {0,0};
	cl_context contexts[] = {0,0};

	bool isSupported[] = {false, false};
	size_t param_value_size = sizeof(char) * 1000;
	char* param_value = new char[1000];

	// get both devices
	ASSERT_NO_FATAL_FAILURE(getCPUGPUDevices(platforms, devices));

	for(int i=0; i<2; ++i)
	{
		if(2==succDevicesNum || i == succDevicesNum)
		{	
			// get extensions supported by this device
			ASSERT_NO_FATAL_FAILURE(getDeviceInfo(devices[i],  
				CL_DEVICE_EXTENSIONS, param_value_size, param_value));

			// get string vector in which each element is a single extension from extensions 
			//	found earlier for this device
			std::vector<std::string> foundExt = getAllStrings(param_value);
			
			// check if required extension is supported
			std::vector<std::string>::iterator itFound;
			for ( itFound=foundExt.begin() ; itFound < foundExt.end(); ++itFound )
			{
				if(0==itFound->compare(requiredExtension))
				{
					// found this extension
					isSupported[i] = true;
					break;
				}
			}
		}
	}
	delete[] param_value;
	*ret_val = isSupported[0] && isSupported[1];
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//	Fission functions
//////////////////////////////////////////////////////////////////////////////////////////////////

// releaseDevice - calls and validates clReleaseDevice
void releaseDevice(cl_device_id device )
{
	//ASSERT_EQ(CL_SUCCESS, clReleaseDeviceEXT(device ));
}

// retainDevice - calls and validates clRetainDevice
void retainDevice(cl_device_id device)
{
	//ASSERT_EQ(CL_SUCCESS, clRetainDeviceEXT(device));
}

// clCreateSubDevices - calls and validates clCreateSubDevices
void createSubDevices(cl_device_id in_device, 
	const cl_device_partition_property_ext * properties,
	cl_uint num_entries, cl_device_id *out_devices, cl_uint *num_devices)
{
	clCreateSubDevicesEXT_fn clCreateSubDevicesEXT_function = NULL;
	clCreateSubDevicesEXT_function = (clCreateSubDevicesEXT_fn)clGetExtensionFunctionAddress("clCreateSubDevicesEXT");
	ASSERT_TRUE(NULL != clCreateSubDevicesEXT_function) << "clCreateSubDevicesEXT was returned as NULL from clGetExtensionFunctionAddress";

	cl_int ret_val = clCreateSubDevicesEXT_function(in_device, properties,
		num_entries, out_devices, num_devices);
	ASSERT_EQ(CL_SUCCESS, ret_val);
}

// createPartitionByCounts - creates numSubDevices subdevices for root device in_device
// with property CL_DEVICE_PARTITION_BY_COUNTS_*
void createPartitionByCounts(cl_device_id in_device, cl_device_id* out_devices, cl_uint numSubDevices)
{
	if(NULL==out_devices)
	{
		ASSERT_TRUE(false) << "Null argument provided";
	}
	ASSERT_TRUE(0<numSubDevices) << "numSubDevices must be a positive integer"; 
	// get sub-devices
	cl_uint actual_num_devices = 0;
	cl_device_partition_property_ext properties[5];
	properties[0] = CL_DEVICE_PARTITION_BY_COUNTS_EXT;
	int i=0;
	for(i=1; i<numSubDevices+1; ++i)
	{
		properties[i] = 1;
	}
	properties[i] = CL_PARTITION_BY_COUNTS_LIST_END_EXT;
	properties[i+1] = CL_PROPERTIES_LIST_END_EXT;

	// query the number of available sub devices
	ASSERT_NO_FATAL_FAILURE(createSubDevices(in_device, properties, 0, 
		NULL, &actual_num_devices));

	if (actual_num_devices < 1)
	{
		std::cout << "Number of sub-devices set in configuration is " << numSubDevices << " while device can be partitioned  to " << actual_num_devices << std::endl;
		return;
	}
	// create sub devices
	ASSERT_NO_FATAL_FAILURE(createSubDevices(in_device, properties, numSubDevices, 
		out_devices, &actual_num_devices));
}

// getCPUGPUDevicesIfNotCreated - calls getCPUGPUDevices unless
// devices in ocl_descriptor were already initialized
// (in SetUp() devices are initialized to 0)
// This function is used in test bodies which can be called by both regular root devices
// as well as fission subdevices.
// In case of root devices - devices member of ocl_descriptor is uninitialized
// And in case of subdevices (or mixture of subdevices with root devices) -
// devices member of ocl_descriptor will already be uninitialized
void getCPUGPUDevicesIfNotCreated(OpenCLDescriptor& ocl_descriptor)
{
	if(0==ocl_descriptor.devices[0] || 0==ocl_descriptor.devices[1])
	{
		ASSERT_NO_FATAL_FAILURE(getCPUGPUDevices(ocl_descriptor.platforms, ocl_descriptor.devices));
	}
}


