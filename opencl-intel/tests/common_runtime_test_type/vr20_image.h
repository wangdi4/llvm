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
// vr20_image.h

#ifndef IMAGE_READ_WRITE_TEST_
#define IMAGE_READ_WRITE_TEST_

#include "vr8_image.h"

// The following tests bodies serve both regular root devices and fission subdevices tests

//|	TEST: CommonRuntime.test2DReadWriteCommands (TC-96)
//|
//|	Purpose
//|	-------
//|	
//|	Verify that all formats of 2D image objects can be shared by both devices
//|
//|	Method
//|	------
//|
//|	1. Create a shared context
//|	2. Create 2D image object with read-write privilege
//| 3. Queue to CPU device to read from image
//| 4. Queue to GPU device to read from image
//| 5. Queue to CPU device to write from image
//| 6. Queue to GPU device to read from image
//| 7. Queue to GPU device to write from image
//| 8. Queue to CPU device to read from image
//|	9. Expect all reads to returns correct values
//|	
//|	Pass criteria
//|	-------------
//|
//|	After each time the image context isd changed by one device and then it is read by the other device,
//| the reader should get the correct updated content
//|
template<typename T>
void test2DReadWriteCommands(OpenCLDescriptor& ocl_descriptor, cl_image_format image_format, const char* kernelName, bool isHalf = false)
{
	// check if image format is supported
	bool isSupported = false;
	ASSERT_NO_FATAL_FAILURE(isImageFormatSupportedByRequiredDevices(
		CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR, 
		CL_MEM_OBJECT_IMAGE2D, image_format, 2, &isSupported));

	if(false == isSupported)
	{
		// this image format is not supported for the required devices
		// return
		return;
	}

	// this image format is supported for the required devices
	// resume test

	size_t width = getDimetionSize(image_format);
	size_t height = getDimetionSize(image_format);
	size_t depth = 1;
	size_t row_pitch = 0;
	size_t slice_pitch = 0;
	size_t origin[] = {0,0,0};
	size_t region[] = {width,height,depth};

	// create and initialize all arrays (input and output)
	int arraySize = (int)(width*height*depth*correctArraySize(image_format));
	// input arrays (for "writes")
	DynamicArray<T> general_input_array(arraySize);
	DynamicArray<T> cpu_input_array(arraySize);
	DynamicArray<T> gpu_input_array(arraySize);

	// each input array is set to be different from others
	general_input_array.multBy(2);
	cpu_input_array.multBy(5);
	gpu_input_array.multBy(7);

	// device output arrays (for "reads")
	DynamicArray<T> cpu_array(arraySize);
	DynamicArray<T> gpu_array(arraySize);

	// create OpenCL queues, program and context
	ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueues(ocl_descriptor, "read_image_no_3d_writes.cl"));

	// create 2d image
	ASSERT_NO_FATAL_FAILURE(createImage2D(&ocl_descriptor.in_common_buffer, ocl_descriptor.context, 
		CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR, &image_format, width, height, row_pitch, 
		general_input_array.dynamic_array));

	// events
	cl_event user_event = 0;
	cl_event device_done_event[2];

	// read data
	for(int i=0; i<2; ++i)
	{
		// read image on ith queue
		ASSERT_NO_FATAL_FAILURE(enqueueReadImage(ocl_descriptor.queues[i], ocl_descriptor.in_common_buffer, 
			CL_FALSE, origin, region, row_pitch, slice_pitch, 
			((0==i)?cpu_array.dynamic_array:gpu_array.dynamic_array), 
			0, NULL, &device_done_event[i]));
	}
	// wait for reads to complete
	ASSERT_NO_FATAL_FAILURE(waitForEvents(2, device_done_event));

	// validate that correct values were read by both CPU and GPU
	for(int i=0; i<2; ++i)
	{
		ASSERT_NO_FATAL_FAILURE(((0==i)?cpu_array:gpu_array).compareArray(general_input_array.dynamic_array,arraySize));
	}

	for(int i=0; i<2; ++i)
	{
		// i device write, (1-i) device read
		ASSERT_NO_FATAL_FAILURE(enqueueWriteImage(ocl_descriptor.queues[i], ocl_descriptor.in_common_buffer, 
			CL_FALSE, origin, region, row_pitch, slice_pitch, 
			((0==i)?cpu_input_array.dynamic_array:gpu_input_array.dynamic_array), 
			0, NULL, &device_done_event[i]));

		ASSERT_NO_FATAL_FAILURE(enqueueReadImage(ocl_descriptor.queues[1-i], ocl_descriptor.in_common_buffer, 
			CL_FALSE, origin, region, row_pitch, slice_pitch, 
			((0==i)?cpu_array.dynamic_array:gpu_array.dynamic_array), 
			1, &device_done_event[i], &device_done_event[1-i]));

		// wait for write and read to complete
		ASSERT_NO_FATAL_FAILURE(waitForEvents(2, device_done_event));
		ASSERT_NO_FATAL_FAILURE(((0==i)?cpu_array:gpu_array).compareArray(((0==i)?cpu_input_array.dynamic_array:gpu_input_array.dynamic_array),arraySize));
	}
}

//|	TEST: CommonRuntime.test3DReadWriteCommands (TC-97)
//|
//|	Purpose
//|	-------
//|	
//|	Verify that all formats of 3D image objects can be shared by both devices
//|
//|	Method
//|	------
//|
//|	1. Create a shared context
//|	2. Create 3D image object with read-write privilege
//| 3. Queue to CPU device to read from image
//| 4. Queue to GPU device to read from image
//| 5. Queue to CPU device to write from image
//| 6. Queue to GPU device to read from image
//| 7. Queue to GPU device to write from image
//| 8. Queue to CPU device to read from image
//|	9. Expect all reads to returns correct values
//|	
//|	Pass criteria
//|	-------------
//|
//|	After each time the image context isd changed by one device and then it is read by the other device,
//| the reader should get the correct updated content
//|
template<typename T>
void test3DReadWriteCommands(OpenCLDescriptor& ocl_descriptor, cl_image_format image_format, const char* kernelName, bool isHalf = false)
{
	// check if image format is supported
	bool isSupported = false;
	ASSERT_NO_FATAL_FAILURE(isImageFormatSupportedByRequiredDevices(
		CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR, 
		CL_MEM_OBJECT_IMAGE3D, image_format, 2, &isSupported));

	if(false == isSupported)
	{
		// this image format is not supported for the required devices
		// return
		return;
	}

	// this image format is supported for the required devices
	// resume test

	size_t width = getDimetionSize(image_format);
	size_t height = getDimetionSize(image_format);
	size_t depth = getDimetionSize(image_format);
	size_t row_pitch = 0;
	size_t slice_pitch = 0;
	size_t origin[] = {0,0,0};
	size_t region[] = {width,height,depth};
	
	// create and initialize all arrays (input and output)
	int arraySize = (int)(width*height*depth*correctArraySize(image_format));
	// input arrays (for "writes")
	DynamicArray<T> general_input_array(arraySize);
	DynamicArray<T> cpu_input_array(arraySize);
	DynamicArray<T> gpu_input_array(arraySize);

	// each input array is set to be different from others
	general_input_array.multBy(2);
	cpu_input_array.multBy(5);
	gpu_input_array.multBy(7);

	// device output arrays (for "reads")
	//TODO: for CL_RG UNSIGNED and SIGNED INT32 crashed on destructors when sizes 
	//of cpu_array and gpu_array were arraySize
	//Need to test this case on IVB
	DynamicArray<T> cpu_array(arraySize*50);
	DynamicArray<T> gpu_array(arraySize*50);

	// create OpenCL queues, program and context
	ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueues(ocl_descriptor, "read_image_no_3d_writes.cl"));

	// create 2d image
	ASSERT_NO_FATAL_FAILURE(createImage3D(&ocl_descriptor.in_common_buffer, ocl_descriptor.context, 
		CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR, &image_format, width, height, depth, row_pitch, slice_pitch,
		general_input_array.dynamic_array));

	// events
	cl_event user_event = 0;
	cl_event device_done_event[2];

	// read data
	for(int i=0; i<2; ++i)
	{
		// read image on ith queue
		ASSERT_NO_FATAL_FAILURE(enqueueReadImage(ocl_descriptor.queues[i], ocl_descriptor.in_common_buffer, 
			CL_FALSE, origin, region, row_pitch, slice_pitch, 
			((0==i)?cpu_array.dynamic_array:gpu_array.dynamic_array), 
			0, NULL, &device_done_event[i]));
	}
	// wait for reads to complete
	ASSERT_NO_FATAL_FAILURE(waitForEvents(2, device_done_event));

	// validate that correct values were read by both CPU and GPU
	for(int i=0; i<2; ++i)
	{
		ASSERT_NO_FATAL_FAILURE(((0==i)?cpu_array:gpu_array).compareArray(general_input_array.dynamic_array,arraySize));
	}

	for(int i=0; i<2; ++i)
	{
		// i device write, (1-i) device read
		ASSERT_NO_FATAL_FAILURE(enqueueWriteImage(ocl_descriptor.queues[i], ocl_descriptor.in_common_buffer, 
			CL_FALSE, origin, region, row_pitch, slice_pitch, 
			((0==i)?cpu_input_array.dynamic_array:gpu_input_array.dynamic_array), 
			0, NULL, &device_done_event[i]));

		ASSERT_NO_FATAL_FAILURE(enqueueReadImage(ocl_descriptor.queues[1-i], ocl_descriptor.in_common_buffer, 
			CL_FALSE, origin, region, row_pitch, slice_pitch, 
			((0==i)?cpu_array.dynamic_array:gpu_array.dynamic_array), 
			1, &device_done_event[i], &device_done_event[1-i]));

		// wait for write and read to complete
		ASSERT_NO_FATAL_FAILURE(waitForEvents(2, device_done_event));
		ASSERT_NO_FATAL_FAILURE(((0==i)?cpu_array:gpu_array).compareArray(((0==i)?cpu_input_array.dynamic_array:gpu_input_array.dynamic_array),arraySize));
	}
}

//|	TEST: CommonRuntime.test2DReadWriteThroughKernel (TC-98)
//|
//|	Purpose
//|	-------
//|	
//|	Verify that all formats of 2D image objects can be shared and modified by both devices inside kernels
//|
//|	Method
//|	------
//|
//|	1. Create a shared context
//|	2. Create 2D image object with read-write privileges (will be shared by CPU and GPU).
//|		This image will be modified by both CPU and GPU. Each device will take turns in modifying
//|		this image elements and validate that it is able to see modifications made previously by all devices.
//| 3. Create read-only 2D images - one for CPU and one for GPU - which before invocation of each kernel
//|		will contain precise expected input at that point of execution.
//|		These images will be used for validation ONLY. 
//|		Inside kernels - validate that image created at step 2 and appropriate image from step 3
//|		contain the same elements. This will validate that at each point of execution - kernel
//|		is able to see correct content of image from step 2.
//|	4. Enqueue kernel to CPU. 
//|		This kernel will validate that image from step 2 contains what it's 
//|		supposed to contain at this point of execution,	which is the original content of
//|		image.
//|		Then it will multiply each element of image from step 2 by 3.
//|		Host then will validate that indeed each element of image from step 2 is now multiplied by 3
//|		and that validation in kernel itself succeeded as well 
//|		(kernel returns by reference result of its own validation).
//|	4. Enqueue kernel to GPU. 
//|		This kernel will validate that image from step 2 contains what it's 
//|		supposed to contain at this point of execution - each element from step 2 is multiplied by 3.
//|		Then it will multiply each element of image by 5.
//|		Host then will validate that indeed each element of image is now multiplied by 3*5
//|		and that validation in kernel itself succeeded as well.
//|	4. Enqueue kernel to CPU. 
//|		This kernel will validate that image from step 2 contains what it's 
//|		supposed to contain at this point of execution - each element from step 2 is multiplied by 3*5.
//|		Then it will multiply each element of image by 3.
//|		Host then will validate that the validation in kernel itself succeeded.
//|	
//|	Pass criteria
//|	-------------
//|
//|	After each time the image context is changed by one of the devices and then read by another,
//| the reader should be able to get the updated content.
//|
template<typename T>
void test2DReadWriteThroughKernel(OpenCLDescriptor& ocl_descriptor, cl_image_format image_format, const char* kernelName, bool isHalf = false)
{
	// check if image format is supported
	bool isSupported = false;
	ASSERT_NO_FATAL_FAILURE(isImageFormatSupportedByRequiredDevices(
		CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR, 
		CL_MEM_OBJECT_IMAGE2D, image_format, 2, &isSupported));

	if(false == isSupported)
	{
		// this image format is not supported for the required devices
		// return
		return;
	}

	// this image format is supported for the required devices
	// resume test

	// image properties
	size_t width = getDimetionSize(image_format);
	size_t height = getDimetionSize(image_format);
	size_t depth = 1;
	size_t row_pitch = 0;
	size_t slice_pitch = 0;
	size_t origin[] = {0,0,0};
	size_t region[] = {width,height,depth};

	// create and initialize arrays
	int arraySize = (int)(width*height*depth*correctArraySize(image_format));
	// input_array - with which will create image
	DynamicArray<T> input_array(arraySize);
	// cpu_first_input_array - expected input in first kernel invocation on CPU
	DynamicArray<T> cpu_first_input_array(arraySize);
	// gpu_first_input_array - expected input in first kernel invocation on GPU
	DynamicArray<T> gpu_first_input_array(arraySize);
	// cpu_second_input_array - expected input in second kernel invocation on CPU
	DynamicArray<T> cpu_second_input_array(arraySize);

	// multiplyBy - values each device will multiply by
	// devices will take turns in modifying image content
	// device [i] will take turns and multiply each element by multiplyBy[i]
	// meaning:
	// first CPU will multiply by multiplyBy[0]
	// then GPU will multiply by multiplyBy[1]
	// so that after first CPU's iteration expect to see input_array's elements multiplied by multiplyBy[0]
	// and after GPU's iteration - expect to see input_array's elements multiplied by multiplyBy[0]*multiplyBy[1]
	cl_int multiplyBy[] = {3,5};
	
	// adjust values of gpu_first_input_array and cpu_second_input_array for future comparison
	gpu_first_input_array.multBy(multiplyBy[0], true);
	cpu_second_input_array.multBy(multiplyBy[0]*multiplyBy[1], true);
	
	// there will be two validation passes:
	// 1. on host-side - with read buffer into cpu_array and gpu_array, 
	// validates that kernels give correct output
	// 2. on kernel-side - with 0 written upon success to result[i] for device [i], 
	// validates that kernels "see" correct input
	//TODO: for CL_RG UNSIGNED and SIGNED INT32 crashed on destructors when sizes 
	//of cpu_array and gpu_array were arraySize
	//Need to test this case on IVB
	DynamicArray<T> cpu_array(arraySize);
	DynamicArray<T> gpu_array(arraySize);
	T* arrayToMapTo = NULL;
	cl_int result[]={-1,-1};

	// create OpenCL queues, program and context
	ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueues(ocl_descriptor, "read_image_no_3d_writes.cl"));

	// create 2d image
	ASSERT_NO_FATAL_FAILURE(createImage2D(&ocl_descriptor.in_common_buffer, ocl_descriptor.context, 
		CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR, &image_format, width, height, row_pitch, input_array.dynamic_array));
	
	// set work dimensions
	cl_uint work_dim = 1;
	size_t global_work_size = 1;

	cl_mem mem_result[] = {0,0};
	// first kernel invocations on CPU and GPU
	for(int i=0; i<2; ++i)    
	{
		// create kernel
		ASSERT_NO_FATAL_FAILURE(createKernel(&ocl_descriptor.kernels[i] ,ocl_descriptor.program, kernelName));

		// create image for inside-kernel validation - to test that kernels "sees" correct input
		ASSERT_NO_FATAL_FAILURE(createImage2D(&ocl_descriptor.buffers[i], ocl_descriptor.context, 
			CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR, &image_format, width, height, 0/*row_pitch*/, 
			(0==i)?(cpu_first_input_array.dynamic_array):(gpu_first_input_array.dynamic_array)));

		// create mem_result buffer
		ASSERT_NO_FATAL_FAILURE(createBuffer(&mem_result[i], ocl_descriptor.context, 
			CL_MEM_READ_WRITE|CL_MEM_ALLOC_HOST_PTR, sizeof(cl_int), NULL));

		// set kernel arguments
		ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[i], 0, sizeof(cl_mem), &ocl_descriptor.in_common_buffer));
		ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[i], 1, sizeof(cl_mem), &ocl_descriptor.in_common_buffer));
		ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[i], 2, sizeof(cl_mem), &ocl_descriptor.buffers[i]));
		ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[i], 3, sizeof(cl_mem), &mem_result[i]));
		ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[i], 4, sizeof(cl_int), &multiplyBy[i]));

		// enqueue kernel
		ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(ocl_descriptor.queues[i], ocl_descriptor.kernels[i], 1, NULL, &global_work_size, NULL, 0, NULL,  NULL));

		ASSERT_NO_FATAL_FAILURE(enqueueReadImage(ocl_descriptor.queues[i], ocl_descriptor.in_common_buffer, 
			CL_FALSE, origin, region, 0/*row_pitch*/, slice_pitch, ((0==i)?cpu_array.dynamic_array:gpu_array.dynamic_array), 
			0, NULL, NULL));

		// read result
		ASSERT_NO_FATAL_FAILURE(enqueueReadBuffer(ocl_descriptor.queues[i], mem_result[i], CL_TRUE, 0, sizeof(cl_int), 
			&result[i], 0, NULL, NULL));

		// validate that kernel's validation succeeded - result[0] must contain 0
		ASSERT_EQ(0,result[i]) << "For device " << i << " kernel's internal validation failed (first round)";

		// host-side validation
		// on CPU will expect to get gpu_first_input_array (GPU's input)
		// on GPU will expect to get cpu_second_input_array (second iteration's CPU input)
		ASSERT_NO_FATAL_FAILURE(((0==i)?cpu_array:gpu_array).compareArray((0==i)?gpu_first_input_array.dynamic_array:cpu_second_input_array.dynamic_array,arraySize));

		// ensures that in_common_buffer's content is updated so the next time it's accessed within inside a kernel
		// all its elements are updated
		ASSERT_NO_FATAL_FAILURE(enqueueMapImage(&arrayToMapTo, ocl_descriptor.queues[i], 
			ocl_descriptor.in_common_buffer, CL_FALSE, CL_MAP_WRITE, origin, region, &row_pitch, &slice_pitch,
			0, NULL, NULL));
		ASSERT_NO_FATAL_FAILURE(enqueueUnmapMemObject(ocl_descriptor.queues[i], ocl_descriptor.in_common_buffer, 
			arrayToMapTo, 0, NULL, NULL));        
	}
	// create image for inside-kernel validation - to test that kernels "sees" correct input
	ASSERT_NO_FATAL_FAILURE(createImage2D(&ocl_descriptor.buffers[0], ocl_descriptor.context, 
		CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR, &image_format, width, height, 0/*row_pitch*/, 
		cpu_second_input_array.dynamic_array));

	// set kernel arguments
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], 0, sizeof(cl_mem), &ocl_descriptor.in_common_buffer));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], 1, sizeof(cl_mem), &ocl_descriptor.in_common_buffer));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], 2, sizeof(cl_mem), &ocl_descriptor.buffers[0]));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], 3, sizeof(cl_mem), &mem_result[0]));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], 4, sizeof(cl_int), &multiplyBy[0]));

	// second kernel invocation on CPU
	ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(ocl_descriptor.queues[0], ocl_descriptor.kernels[0], 1, NULL, &global_work_size, NULL, 0, NULL,  NULL));

	ASSERT_NO_FATAL_FAILURE(enqueueReadImage(ocl_descriptor.queues[0], ocl_descriptor.in_common_buffer, 
		CL_FALSE, origin, region, 0/*row_pitch*/, slice_pitch, cpu_array.dynamic_array, 
		0, NULL, NULL));

	// read result
	ASSERT_NO_FATAL_FAILURE(enqueueReadBuffer(ocl_descriptor.queues[0], mem_result[0], CL_TRUE, 0, sizeof(cl_int), 
		&result[0], 0, NULL, NULL));

	// validate that kernel's validation succeeded - result[0] must contain 0
	ASSERT_EQ(0,result[0]) << "For device 0 kernel's internal validation failed";

	// ensures that in_common_buffer's content is updated so the next time it's accessed within inside a kernel
	// all its elements are updated
	ASSERT_NO_FATAL_FAILURE(enqueueMapImage(&arrayToMapTo, ocl_descriptor.queues[0], 
		ocl_descriptor.in_common_buffer, CL_FALSE, CL_MAP_READ, origin, region, &row_pitch, &slice_pitch,
		0, NULL, NULL));
	ASSERT_NO_FATAL_FAILURE(enqueueUnmapMemObject(ocl_descriptor.queues[0], ocl_descriptor.in_common_buffer, 
		arrayToMapTo, 0, NULL, NULL));

	// release mem_result
	for(int i=0; i<2; ++i)
	{
		if(0!=mem_result[i])
		{
			EXPECT_EQ(CL_SUCCESS, clReleaseMemObject(mem_result[i])) << "clReleaseMemObject failed";
			mem_result[0] = 0;
		}
	}
}

//|	TEST: CommonRuntime.test3DReadWriteThroughKernel (TC-99)
//|
//|	Purpose
//|	-------
//|	
//|	Verify that all formats of 3D image objects can be shared and modified by both devices inside kernels
//|
//|	Method
//|	------
//|
//|	1. Create a shared context
//|	2. Create 3D image object with read-write privileges (will be shared by CPU and GPU).
//|		This image will be modified by both CPU and GPU. Each device will take turns in modifying
//|		this image elements and validate that it is able to see modifications made previously by all devices.
//| 3. Create read-only 3D images - one for CPU and one for GPU - which before invocation of each kernel
//|		will contain precise expected input at that point of execution.
//|		These images will be used for validation ONLY. 
//|		Inside kernels - validate that image created at step 2 and appropriate image from step 3
//|		contain the same elements. This will validate that at each point of execution - kernel
//|		is able to see correct content of image from step 2.
//|	4. Enqueue kernel to CPU. 
//|		This kernel will validate that image from step 2 contains what it's 
//|		supposed to contain at this point of execution,	which is the original content of
//|		image.
//|		Then it will multiply each element of image from step 2 by 3.
//|		Host then will validate that indeed each element of image from step 2 is now multiplied by 3
//|		and that validation in kernel itself succeeded as well 
//|		(kernel returns by reference result of its own validation).
//|	4. Enqueue kernel to GPU. 
//|		This kernel will validate that image from step 2 contains what it's 
//|		supposed to contain at this point of execution - each element from step 2 is multiplied by 3.
//|		Then it will multiply each element of image by 5.
//|		Host then will validate that indeed each element of image is now multiplied by 3*5
//|		and that validation in kernel itself succeeded as well.
//|	4. Enqueue kernel to CPU. 
//|		This kernel will validate that image from step 2 contains what it's 
//|		supposed to contain at this point of execution - each element from step 2 is multiplied by 3*5.
//|		Then it will multiply each element of image by 3.
//|		Host then will validate that the validation in kernel itself succeeded.
//|	
//|	Pass criteria
//|	-------------
//|
//|	After each time the image context is changed by one of the devices and then read by another,
//| the reader should be able to get the updated content.
//|
template<typename T>
void test3DReadWriteThroughKernel(OpenCLDescriptor& ocl_descriptor, cl_image_format image_format, const char* kernelName, bool isHalf = false)
{
	// check if image format is supported
	bool isSupported = false;
	ASSERT_NO_FATAL_FAILURE(isImageFormatSupportedByRequiredDevices(
		CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR, 
		CL_MEM_OBJECT_IMAGE3D, image_format, 2, &isSupported));

	if(false == isSupported)
	{
		// this image format is not supported for the required devices
		// return
		return;
	}

	// this image format is supported for the required devices
	// check that extension is supported
	bool isExtensionSupported = false;
	ASSERT_NO_FATAL_FAILURE(isExtensionSupportedOnDevice("cl_khr_3d_image_writes", 2, &isExtensionSupported));
	if(false == isExtensionSupported)
	{
		// extension is not supported by both devices
		// return
		std::cout << "Extension cl_khr_3d_image_writes is not supported by both devices" << std::endl;
		return;
	}
	// extension is supported by both devices
	// resume test
	// image properties
	size_t width = getDimetionSize(image_format);
	size_t height = getDimetionSize(image_format);
	size_t depth = getDimetionSize(image_format);
	size_t row_pitch = 0;
	size_t slice_pitch = 0;
	size_t origin[] = {0,0,0};
	size_t region[] = {width,height,depth};

	// create and initialize arrays
	int arraySize = (int)(width*height*depth*correctArraySize(image_format));
	// input_array - with which will create image
	DynamicArray<T> input_array(arraySize, isHalf);
	// cpu_first_input_array - expected input in first kernel invocation on CPU
	DynamicArray<T> cpu_first_input_array(arraySize, isHalf);
	// gpu_first_input_array - expected input in first kernel invocation on GPU
	DynamicArray<T> gpu_first_input_array(arraySize, isHalf);
	// cpu_second_input_array - expected input in second kernel invocation on CPU
	DynamicArray<T> cpu_second_input_array(arraySize, isHalf);

	// multiplyBy - values each device will multiply by
	// devices will take turns in modifying image content
	// device [i] will take turns and multiply each element by multiplyBy[i]
	// meaning:
	// first CPU will multiply by multiplyBy[0]
	// then GPU will multiply by multiplyBy[1]
	// so that after first CPU's iteration expect to see input_array's elements multiplied by multiplyBy[0]
	// and after GPU's iteration - expect to see input_array's elements multiplied by multiplyBy[0]*multiplyBy[1]
	cl_int multiplyBy[] = {3,5};
	
	// adjust values of gpu_first_input_array and cpu_second_input_array for future comparison
	gpu_first_input_array.multBy(multiplyBy[0], true);
	cpu_second_input_array.multBy(multiplyBy[0]*multiplyBy[1], true);
	
	// there will be two validation passes:
	// 1. on host-side - with read buffer into cpu_array and gpu_array, 
	// validates that kernels give correct output
	// 2. on kernel-side - with 0 written upon success to result[i] for device [i], 
	// validates that kernels "see" correct input
	DynamicArray<T> cpu_array(arraySize);
	DynamicArray<T> gpu_array(arraySize);
	T* arrayToMapTo = NULL;
	cl_int result[]={-1,-1};

	//input_array.printArrayContent();

	// create OpenCL queues, program and context
	ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueues(ocl_descriptor, "read_image_with_3d_writes.cl"));

	// create 2d image	
	ASSERT_NO_FATAL_FAILURE(createImage3D(&ocl_descriptor.in_common_buffer, ocl_descriptor.context, 
		CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR, &image_format, width, height, depth, row_pitch, slice_pitch, 
		input_array.dynamic_array));

	// set work dimensions
	cl_uint work_dim = 1;
	size_t global_work_size = 1;

	cl_mem mem_result[] = {0,0};
	// first kernel invocations on CPU and GPU
	for(int i=0; i<2; ++i)
	{
		// create kernel
		ASSERT_NO_FATAL_FAILURE(createKernel(&ocl_descriptor.kernels[i] ,ocl_descriptor.program, kernelName));

		// create image for inside-kernel validation - to test that kernels "sees" correct input
		ASSERT_NO_FATAL_FAILURE(createImage3D(&ocl_descriptor.buffers[i], ocl_descriptor.context, 
			CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR, &image_format, width, height, depth, row_pitch, slice_pitch, 
			(0==i)?(cpu_first_input_array.dynamic_array):(gpu_first_input_array.dynamic_array)));

		// create mem_result buffer
		ASSERT_NO_FATAL_FAILURE(createBuffer(&mem_result[i], ocl_descriptor.context, 
			CL_MEM_READ_WRITE|CL_MEM_ALLOC_HOST_PTR, sizeof(cl_int), NULL));

		// set kernel arguments
		ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[i], 0, sizeof(cl_mem), &ocl_descriptor.in_common_buffer));
		ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[i], 1, sizeof(cl_mem), &ocl_descriptor.in_common_buffer));
		ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[i], 2, sizeof(cl_mem), &ocl_descriptor.buffers[i]));
		ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[i], 3, sizeof(cl_mem), &mem_result[i]));
		ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[i], 4, sizeof(cl_int), &multiplyBy[i]));

		// enqueue kernel
		ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(ocl_descriptor.queues[i], ocl_descriptor.kernels[i], 1, NULL, &global_work_size, NULL, 0, NULL,  NULL));

		ASSERT_NO_FATAL_FAILURE(enqueueReadImage(ocl_descriptor.queues[i], ocl_descriptor.in_common_buffer, 
			CL_FALSE, origin, region, row_pitch, slice_pitch, ((0==i)?cpu_array.dynamic_array:gpu_array.dynamic_array), 
			0, NULL, NULL));

		// read result
		ASSERT_NO_FATAL_FAILURE(enqueueReadBuffer(ocl_descriptor.queues[i], mem_result[i], CL_TRUE, 0, sizeof(cl_int), 
			&result[i], 0, NULL, NULL));

		// validate that kernel's validation succeeded - result[0] must contain 0
		ASSERT_EQ(0,result[i]) << "For device " << i << " kernel's internal validation failed";

		// host-side validation
		// on CPU will expect to get gpu_first_input_array (GPU's input)
		// on GPU will expect to get cpu_second_input_array (second iteration's CPU input)
		ASSERT_NO_FATAL_FAILURE(((0==i)?cpu_array:gpu_array).compareArray((0==i)?gpu_first_input_array.dynamic_array:cpu_second_input_array.dynamic_array,arraySize));
		
		// ensures that in_common_buffer's content is updated so the next time it's accessed within inside a kernel
		// all its elements are updated
		ASSERT_NO_FATAL_FAILURE(enqueueMapImage(&arrayToMapTo, ocl_descriptor.queues[i], 
			ocl_descriptor.in_common_buffer, CL_FALSE, CL_MAP_WRITE, origin, region, &row_pitch, &slice_pitch,
			0, NULL, NULL));
		ASSERT_NO_FATAL_FAILURE(enqueueUnmapMemObject(ocl_descriptor.queues[i], ocl_descriptor.in_common_buffer, 
			arrayToMapTo, 0, NULL, NULL));
	}
	// create image for inside-kernel validation - to test that kernels "sees" correct input
	ASSERT_NO_FATAL_FAILURE(createImage3D(&ocl_descriptor.buffers[0], ocl_descriptor.context, 
		CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR, &image_format, width, height, depth, 
		row_pitch, slice_pitch, cpu_second_input_array.dynamic_array));


	// set kernel arguments
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], 0, sizeof(cl_mem), &ocl_descriptor.in_common_buffer));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], 1, sizeof(cl_mem), &ocl_descriptor.in_common_buffer));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], 2, sizeof(cl_mem), &ocl_descriptor.buffers[0]));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], 3, sizeof(cl_mem), &mem_result[0]));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], 4, sizeof(cl_int), &multiplyBy[0]));

	// second kernel invocation on CPU
	ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(ocl_descriptor.queues[0], ocl_descriptor.kernels[0], 1, NULL, &global_work_size, NULL, 0, NULL,  NULL));

	ASSERT_NO_FATAL_FAILURE(enqueueReadImage(ocl_descriptor.queues[0], ocl_descriptor.in_common_buffer, 
		CL_FALSE, origin, region, row_pitch, slice_pitch, cpu_array.dynamic_array, 
		0, NULL, NULL));

	// read result
	ASSERT_NO_FATAL_FAILURE(enqueueReadBuffer(ocl_descriptor.queues[0], mem_result[0], CL_TRUE, 0, sizeof(cl_int), 
		&result[0], 0, NULL, NULL));

	// validate that kernel's validation succeeded - result[0] must contain 0
	ASSERT_EQ(0,result[0]) << "For device 0 kernel's internal validation failed";

	// ensures that in_common_buffer's content is updated so the next time it's accessed within inside a kernel
	// all its elements are updated
	ASSERT_NO_FATAL_FAILURE(enqueueMapImage(&arrayToMapTo, ocl_descriptor.queues[0], 
		ocl_descriptor.in_common_buffer, CL_FALSE, CL_MAP_READ, origin, region, &row_pitch, &slice_pitch,
		0, NULL, NULL));
	ASSERT_NO_FATAL_FAILURE(enqueueUnmapMemObject(ocl_descriptor.queues[0], ocl_descriptor.in_common_buffer, 
		arrayToMapTo, 0, NULL, NULL));

	// release mem_result
	for(int i=0; i<2; ++i)
	{
		if(0!=mem_result[i])
		{
			EXPECT_EQ(CL_SUCCESS, clReleaseMemObject(mem_result[i])) << "clReleaseMemObject failed";
			mem_result[0] = 0;
		}
	}
}

#endif /* IMAGE_READ_WRITE_TEST_ */