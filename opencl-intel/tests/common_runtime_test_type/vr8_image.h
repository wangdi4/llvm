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
// vc8_image.h

#ifndef IMAGE_READ_TEST_
#define IMAGE_READ_TEST_

#include "common_runtime_tests.h"

// The following tests bodies serve both regular root devices and fission subdevices tests

// The tests in vr_8* files take care of all the test cases TC-48 to TC-56 as well as TC-118 and TC-119

//	If new image format tests should be added:
//	1.	sub-class ImageTypedCommonRuntime - for example refer to vr8_RGBA_CL_FLOAT.cpp
//	2.	set needed image_format in sub-class' SetUp function with image_channel_order 
//		and image_channel_data_type
//	3.	add appropriate buffer type (SINGLE TYPE) - for example for RGBA's "CL_FLOAT" - 
//		will need to add "cl_float4" buffer type with the following syntax:
//			typedef ::testing::Types<cl_float4> imageTypes;
//			TYPED_TEST_CASE(ImageRGBA_CL_FLOAT, imageTypes);
//	4. call the below template functions as in ImageRGBA_CL_FLOAT examples
//  5. for some other formats need non-vector arrays in host, refer to examples with CL_R in their title
//  6. for how to use formats with cl_half refer to tests with cl_half in their title

// correctHalf - if called with cl_half will return 4 iff needToCorrect is true, otherwise will return 1
// For some image formats arrays of vectors of 4 are used, while for some cl_half formats need scalar arrays 
// in host.
// This function auto corrects half array's size

static int correctArraySize(cl_image_format image_format){
	if(CL_RGBA==image_format.image_channel_order && CL_HALF_FLOAT == image_format.image_channel_data_type)
	{
		return 4;
	}
	if(CL_RG==image_format.image_channel_order)
	{
		return 2;
	}
	return 1;
}

static int getDimetionSize(cl_image_format image_format)
{
	return 4;
}

//	If new image format tests should be added:
//	1.	sub-class ImageTypedCommonRuntime - for example refer to ImageRGBA_CL_FLOAT (in vc8_imageCL_FLOAT.cpp)
//	2.	set needed image_format in sub-class' SetUp function with image_channel_order and image_channel_data_type
//	3.	add appropriate buffer type (SINGLE TYPE) - for example for "CL_FLOAT" - will need to add "cl_float4" 
//		(and no other type)	buffer type	with the following syntax:
//			typedef ::testing::Types<cl_float4> imageTypes;
//			TYPED_TEST_CASE(ImageRGBA_CL_FLOAT, imageTypes);
//	4. Call the below template functions as in ImageRGBA_CL_FLOAT examples 

//|	Purpose
//|	-------
//|	
//|	Verify that the content of an OpenCL 2d image is visible to all devices in the context.
//|
//|	Method
//|	------
//|
//|	1. Create shared input cl_mem 2d image on a shared context (shared for CPU and GPU) with CL_MEM_USE_HOST_PTR
//|	2. For each device create separate output buffer
//|	2. Create for each device a kernel which sums all elements of input buffer into a float
//|	3. Run kernels on CPU and GPU
//|	4. Validate that all sum of all elements was obtained
//|	
//|	Pass criteria
//|	-------------
//|
//|	Validate that each device is able to read all image elements
//|
template<typename T>
void test2DUseHostPtr(OpenCLDescriptor& ocl_descriptor, cl_image_format image_format, const char* kernelName, float divisor=1, int succDevicesNum = 2, bool isHalf=false)
{
	// check if image format is supported
	bool isSupported = false;
	ASSERT_NO_FATAL_FAILURE(isImageFormatSupportedByRequiredDevices(
		CL_MEM_READ_WRITE|CL_MEM_USE_HOST_PTR, 
		CL_MEM_OBJECT_IMAGE2D, image_format, succDevicesNum, &isSupported));

	if(false == isSupported)
	{
		// this image format is not supported for the required devices
		// return
		return;
	}

	// this image format is supported for the required devices
	// resume test

	size_t image_width = getDimetionSize(image_format);
	size_t image_height = getDimetionSize(image_format);
	size_t image_row_pitch = 0;
	size_t origin[] = {0,0,0};
	size_t region[] = {image_width,image_height,1};

	// set work dimensions
	cl_uint work_dim = 1;
	size_t global_work_size = 1;
	// create and initialize array
	int arraySize = (int)(image_width*image_height*correctArraySize(image_format));

	DynamicArray<T> input_array(arraySize, isHalf);
	//input_array.printArrayContent();

	// create OpenCL queues, program and context
	ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueues(ocl_descriptor, "read_image_no_3d_writes.cl"));

	// create 2d image
	ASSERT_NO_FATAL_FAILURE(createImage2D(&ocl_descriptor.in_common_buffer, ocl_descriptor.context, 
		CL_MEM_READ_WRITE|CL_MEM_USE_HOST_PTR, 
		&image_format, image_width, image_height, image_row_pitch, input_array.dynamic_array));

	// set up manual destruction
	ASSERT_NO_FATAL_FAILURE(setDeleteArrayOnCallback(ocl_descriptor.in_common_buffer, input_array));
	
	// setup and execute kernels, read and validate results
	ASSERT_NO_FATAL_FAILURE(executeSetAndExecuteKernelsSum(kernelName, 
		input_array, ocl_descriptor.in_common_buffer, ocl_descriptor.buffers, 
		ocl_descriptor.kernels, ocl_descriptor.queues, ocl_descriptor.context, ocl_descriptor.program, divisor, succDevicesNum));
}

//|	Purpose
//|	-------
//|	
//|	Verify that the content of an OpenCL 2d image is visible to all devices in the context.
//|
//|	Method
//|	------
//|
//|	1. Create shared input cl_mem 2d image on a shared context (shared for CPU and GPU) with CL_MEM_ALLOC_HOST_PTR
//|	2. For each device create separate output buffer
//|	2. Create for each device a kernel which sums all elements of input buffer into a float
//|	3. Run kernels on CPU and GPU
//|	4. Validate that all sum of all elements was obtained
//|	
//|	Pass criteria
//|	-------------
//|
//|	Validate that each device is able to read all image elements
//|
template<typename T>
void test2DAllocHostPtr(OpenCLDescriptor& ocl_descriptor, cl_image_format image_format, const char* kernelName, float divisor=1, int succDevicesNum = 2, bool isHalf=false)
{
	// check if image format is supported
	bool isSupported = false;
	ASSERT_NO_FATAL_FAILURE(isImageFormatSupportedByRequiredDevices(
		CL_MEM_READ_WRITE|CL_MEM_ALLOC_HOST_PTR, 
		CL_MEM_OBJECT_IMAGE2D, image_format, succDevicesNum, &isSupported));

	if(false == isSupported)
	{
		// this image format is not supported for the required devices
		// return
		return;
	}

	// this image format is supported for the required devices
	// resume test

	size_t image_width = getDimetionSize(image_format);
	size_t image_height = getDimetionSize(image_format);
	size_t image_row_pitch = 0;
	size_t image_slice_pitch = 0;
	size_t origin[] = {0,0,0};
	size_t region[] = {image_width,image_height,1};

	// set work dimensions
	cl_uint work_dim = 1;
	size_t global_work_size = 1;

	// create and initialize array
	int arraySize = (int)(image_width*image_height*correctArraySize(image_format));
	DynamicArray<T> input_array(arraySize, isHalf);

	// create OpenCL queues, program and context
	ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueues(ocl_descriptor, "read_image_no_3d_writes.cl"));

	// create 2d image
	ASSERT_NO_FATAL_FAILURE(createImage2D(&ocl_descriptor.in_common_buffer, ocl_descriptor.context, 
		CL_MEM_READ_WRITE|CL_MEM_ALLOC_HOST_PTR, 
		&image_format, image_width, image_height, image_row_pitch, NULL));

	// create queue, kernel and set kernel args for each device
	// write to image
	if(0==succDevicesNum || 2==succDevicesNum){
		ASSERT_NO_FATAL_FAILURE(enqueueWriteImage(ocl_descriptor.queues[0], 
			ocl_descriptor.in_common_buffer, 
			CL_TRUE, origin, region, image_row_pitch, image_slice_pitch, 
			input_array.dynamic_array, 0, NULL, NULL));
	}
	else{
		ASSERT_NO_FATAL_FAILURE(enqueueWriteImage(ocl_descriptor.queues[1], ocl_descriptor.in_common_buffer, 
			CL_TRUE, origin, region, image_row_pitch, image_slice_pitch, 
			input_array.dynamic_array, 0, NULL, NULL));
	}
	// setup and execute kernels, read and validate results
	ASSERT_NO_FATAL_FAILURE(executeSetAndExecuteKernelsSum(kernelName, 
		input_array, ocl_descriptor.in_common_buffer, ocl_descriptor.buffers, 
		ocl_descriptor.kernels, ocl_descriptor.queues, ocl_descriptor.context, ocl_descriptor.program, divisor, succDevicesNum));
}

//|	Purpose
//|	-------
//|	
//|	Verify that the content of an OpenCL 2d image is visible to all devices in the context.
//|
//|	Method
//|	------
//|
//|	1. Create shared input cl_mem 2d image on a shared context (shared for CPU and GPU) with CL_MEM_COPY_HOST_PTR
//|	2. For each device create separate output buffer
//|	2. Create for each device a kernel which sums all elements of input buffer into a float
//|	3. Run kernels on CPU and GPU
//|	4. Validate that all sum of all elements was obtained
//|	
//|	Pass criteria
//|	-------------
//|
//|	Validate that each device is able to read all image elements
//|
template<typename T>
void test2DCopyHostPtr(OpenCLDescriptor& ocl_descriptor, cl_image_format image_format, const char* kernelName, float divisor=1, int succDevicesNum = 2, bool isHalf=false)
{
	// check if image format is supported
	bool isSupported = false;
	ASSERT_NO_FATAL_FAILURE(isImageFormatSupportedByRequiredDevices(
		CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR, 
		CL_MEM_OBJECT_IMAGE2D, image_format, succDevicesNum, &isSupported));

	if(false == isSupported)
	{
		// this image format is not supported for the required devices
		// return
		return;
	}

	// this image format is supported for the required devices
	// resume test

	size_t image_width = getDimetionSize(image_format);
	size_t image_height = getDimetionSize(image_format);
	size_t image_row_pitch = 0;
	size_t origin[] = {0,0,0};
	size_t region[] = {image_width,image_height,1};

	// set work dimensions
	cl_uint work_dim = 1;
	size_t global_work_size = 1;

	// create and initialize array
	int arraySize = (int)(image_width*image_height*correctArraySize(image_format));
	DynamicArray<T> input_array(arraySize, isHalf);

	// create OpenCL queues, program and context
	ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueues(ocl_descriptor, "read_image_no_3d_writes.cl"));

	// create 2d image
	ASSERT_NO_FATAL_FAILURE(createImage2D(&ocl_descriptor.in_common_buffer, ocl_descriptor.context, 
		CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR, 
		&image_format, image_width, image_height, image_row_pitch, input_array.dynamic_array));

	// setup and execute kernels, read and validate results
	ASSERT_NO_FATAL_FAILURE(executeSetAndExecuteKernelsSum(kernelName, 
		input_array, ocl_descriptor.in_common_buffer, ocl_descriptor.buffers, 
		ocl_descriptor.kernels, ocl_descriptor.queues, ocl_descriptor.context, ocl_descriptor.program, divisor, succDevicesNum));
}

//|	Purpose
//|	-------
//|	
//|	Verify that the content of an OpenCL 3d image is visible to all devices in the context.
//|
//|	Method
//|	------
//|
//|	1. Create shared input cl_mem 3d image on a shared context (shared for CPU and GPU) with CL_MEM_USE_HOST_PTR
//|	2. For each device create separate output buffer
//|	2. Create for each device a kernel which sums all elements of input buffer into a float
//|	3. Run kernels on CPU and GPU
//|	4. Validate that all sum of all elements was obtained
//|	
//|	Pass criteria
//|	-------------
//|
//|	Validate that each device is able to read all image elements
//|
template<typename T>
void test3DUseHostPtr(OpenCLDescriptor& ocl_descriptor, cl_image_format image_format, const char* kernelName, float divisor=1, int succDevicesNum = 2, bool isHalf=false)
{
	// check if image format is supported
	bool isSupported = false;
	ASSERT_NO_FATAL_FAILURE(isImageFormatSupportedByRequiredDevices(
		CL_MEM_READ_WRITE|CL_MEM_USE_HOST_PTR, 
		CL_MEM_OBJECT_IMAGE3D, image_format, succDevicesNum, &isSupported));

	if(false == isSupported)
	{
		// this image format is not supported for the required devices
		// return
		return;
	}

	// this image format is supported for the required devices
	// resume test

	size_t image_width = getDimetionSize(image_format);
	size_t image_height = getDimetionSize(image_format);
	size_t image_depth = getDimetionSize(image_format);
	size_t image_row_pitch = 0;
	size_t image_slice_pitch = 0;
	size_t origin[] = {0,0,0};
	size_t region[] = {image_width,image_height,image_depth};

	// set work dimensions
	cl_uint work_dim = 1;
	size_t global_work_size = 1;
	
	// create and initialize array
	int arraySize = (int)(image_width*image_height*image_depth*correctArraySize(image_format));
	DynamicArray<T> input_array(arraySize, isHalf);

	// create OpenCL queues, program and context
	ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueues(ocl_descriptor, "read_image_no_3d_writes.cl"));

	// create 3d image
	ASSERT_NO_FATAL_FAILURE(createImage3D(&ocl_descriptor.in_common_buffer, ocl_descriptor.context, 
		CL_MEM_READ_WRITE|CL_MEM_USE_HOST_PTR, 
		&image_format, image_width, image_height, image_depth, image_row_pitch, image_slice_pitch, input_array.dynamic_array));

	// set up manual destruction
	ASSERT_NO_FATAL_FAILURE(setDeleteArrayOnCallback(ocl_descriptor.in_common_buffer, input_array));

	// setup and execute kernels, read and validate results
	ASSERT_NO_FATAL_FAILURE(executeSetAndExecuteKernelsSum(kernelName, 
		input_array, ocl_descriptor.in_common_buffer, ocl_descriptor.buffers, 
		ocl_descriptor.kernels, ocl_descriptor.queues, ocl_descriptor.context, ocl_descriptor.program, divisor, succDevicesNum));
}

//|	Purpose
//|	-------
//|	
//|	Verify that the content of an OpenCL 3d image is visible to all devices in the context.
//|
//|	Method
//|	------
//|
//|	1. Create shared input cl_mem 3d image on a shared context (shared for CPU and GPU) with CL_MEM_ALLOC_HOST_PTR
//|	2. For each device create separate output buffer
//|	2. Create for each device a kernel which sums all elements of input buffer into a float
//|	3. Run kernels on CPU and GPU
//|	4. Validate that all sum of all elements was obtained
//|	
//|	Pass criteria
//|	-------------
//|
//|	Validate that each device is able to read all image elements
//|
template<typename T>
void test3DAllocHostPtr(OpenCLDescriptor& ocl_descriptor, cl_image_format image_format, const char* kernelName, float divisor=1, int succDevicesNum = 2, bool isHalf=false)
{
	// check if image format is supported
	bool isSupported = false;
	ASSERT_NO_FATAL_FAILURE(isImageFormatSupportedByRequiredDevices(
		CL_MEM_READ_WRITE|CL_MEM_ALLOC_HOST_PTR, 
		CL_MEM_OBJECT_IMAGE3D, image_format, succDevicesNum, &isSupported));

	if(false == isSupported)
	{
		// this image format is not supported for the required devices
		// return
		return;
	}

	// this image format is supported for the required devices
	// resume test

	size_t image_width = getDimetionSize(image_format);
	size_t image_height = getDimetionSize(image_format);
	size_t image_depth = getDimetionSize(image_format);
	size_t image_row_pitch = 0;
	size_t origin[] = {0,0,0};
	size_t region[] = {image_width,image_height,image_depth};
	size_t r_image_row_pitch = 0;
	size_t r_image_slice_pitch = 0;

	// set work dimensions
	cl_uint work_dim = 1;
	size_t global_work_size = 1;
	
	// create and initialize array
	int arraySize = (int)(image_width*image_height*image_depth*correctArraySize(image_format));
	DynamicArray<T> input_array(arraySize, isHalf);

	// create OpenCL queues, program and context
	ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueues(ocl_descriptor, "read_image_no_3d_writes.cl"));

	// create 3d image
	ASSERT_NO_FATAL_FAILURE(createImage3D(&ocl_descriptor.in_common_buffer, ocl_descriptor.context, 
		CL_MEM_READ_WRITE|CL_MEM_ALLOC_HOST_PTR, 
		&image_format, image_width, image_height, image_depth, image_row_pitch, r_image_slice_pitch, NULL));

	if(0==succDevicesNum || 2==succDevicesNum){
		ASSERT_NO_FATAL_FAILURE(enqueueWriteImage(ocl_descriptor.queues[0], ocl_descriptor.in_common_buffer, CL_TRUE, origin, region, r_image_row_pitch, r_image_slice_pitch, 
			input_array.dynamic_array, 0, NULL, NULL));
	}else{
		ASSERT_NO_FATAL_FAILURE(enqueueWriteImage(ocl_descriptor.queues[1], ocl_descriptor.in_common_buffer, CL_TRUE, origin, region, r_image_row_pitch, r_image_slice_pitch, 
			input_array.dynamic_array, 0, NULL, NULL));
	}

	// setup and execute kernels, read and validate results
	ASSERT_NO_FATAL_FAILURE(executeSetAndExecuteKernelsSum(kernelName, 
		input_array, ocl_descriptor.in_common_buffer, ocl_descriptor.buffers, 
		ocl_descriptor.kernels, ocl_descriptor.queues, ocl_descriptor.context, ocl_descriptor.program, divisor, succDevicesNum));
}

//|	TEST: ImageRGBA_CL_FLOAT.Image3CopyHostPtr
//|
//|	Purpose
//|	-------
//|	
//|	Verify that the content of an OpenCL 3d image is visible to all devices in the context.
//|
//|	Method
//|	------
//|
//|	1. Create shared input cl_mem 3d image on a shared context (shared for CPU and GPU) with CL_MEM_COPY_HOST_PTR
//|	2. For each device create separate output buffer
//|	2. Create for each device a kernel which sums all elements of input buffer into a float
//|	3. Run kernels on CPU and GPU
//|	4. Validate that all sum of all elements was obtained
//|	
//|	Pass criteria
//|	-------------
//|
//|	Validate that each device is able to read all image elements
//|
template<typename T>
void test3DCopyHostPtr(OpenCLDescriptor& ocl_descriptor, cl_image_format image_format, const char* kernelName, float divisor=1, int succDevicesNum = 2, bool isHalf=false)
{
	// check if image format is supported
	bool isSupported = false;
	ASSERT_NO_FATAL_FAILURE(isImageFormatSupportedByRequiredDevices(
		CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR, 
		CL_MEM_OBJECT_IMAGE3D, image_format, succDevicesNum, &isSupported));

	if(false == isSupported)
	{
		// this image format is not supported for the required devices
		// return
		return;
	}

	// this image format is supported for the required devices
	// resume test

	size_t image_width = getDimetionSize(image_format);
	size_t image_height = getDimetionSize(image_format);
	size_t image_depth = getDimetionSize(image_format);
	size_t image_row_pitch = 0;
	size_t origin[] = {0,0,0};
	size_t region[] = {image_width,image_height,image_depth};
	size_t r_image_row_pitch = 0;
	size_t r_image_slice_pitch = 0;

	// set work dimensions
	cl_uint work_dim = 1;
	size_t global_work_size = 1;
	
		// create and initialize array
	int arraySize = (int)(image_width*image_height*image_depth*correctArraySize(image_format));
	DynamicArray<T> input_array(arraySize, isHalf);

	// create OpenCL queues, program and context
	ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueues(ocl_descriptor, "read_image_no_3d_writes.cl"));

	// create 3d image
	ASSERT_NO_FATAL_FAILURE(createImage3D(&ocl_descriptor.in_common_buffer, ocl_descriptor.context, 
		CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR, 
		&image_format, image_width, image_height, image_depth, image_row_pitch, r_image_slice_pitch, input_array.dynamic_array));

	// setup and execute kernels, read and validate results
	ASSERT_NO_FATAL_FAILURE(executeSetAndExecuteKernelsSum(kernelName, 
		input_array, ocl_descriptor.in_common_buffer, ocl_descriptor.buffers, 
		ocl_descriptor.kernels, ocl_descriptor.queues, ocl_descriptor.context, ocl_descriptor.program, divisor, succDevicesNum));
}

#endif /* IMAGE_READ_TEST_ */