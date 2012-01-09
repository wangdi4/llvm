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

#include "gpa_tests.h"

static void test_native_kernel_fn( void *userData )
{
	struct arg_struct {
		cl_int * source;
		cl_int * dest;
		cl_int count;
	} *args = (arg_struct *)userData;
	
	args->dest[ 0 ] = args->source[ 0 ];
		
}

//| enqueueCommands - creates kernel with properties queue_properties and enqueues the following commands:
//|		clEnqueueReadBuffer
//|		clEnqueueWriteBuffer
//|		clEnqueueReadBufferRect
//|		clEnqueueWriteBufferRect
//|		clEnqueueCopyBuffer
//|		clEnqueueCopyBufferRect
//|		clEnqueueMapBuffer
//|		clEnqueueUnmapMemObject
//|		clEnqueueNDRangeKernel
//|		clEnqueueTask
//|		clEnqueueNativeKernel
//|		clEnqueueMarker
//|		clEnqueueWaitForEvents
//|		clEnqueueBarrier
//|		clEnqueueNDRangeKernel
//|		clEnqueueReadImage
//|		clEnqueueWriteImage
//|		clEnqueueCopyImage
//|		clEnqueueCopyImageToBuffer
//|		clEnqueueCopyBufferToImage
//|		clEnqueueMapImage
//|		clEnqueueUnmapMemObject //TODO
void enqueueCommands(OpenCLDescriptor& ocl_descriptor, cl_command_queue_properties queue_properties)
{
	// create and initialize array
	size_t width = 2;
	size_t height = 2;
	size_t depth = 2;
	int arraySize = (int)(width*height*depth);
	int value = 5;

	size_t origin[] = {0,0,0};
	size_t rect_region[] = {width*sizeof(cl_int4),height,depth};

	// create and initialize array
	size_t image_width = width;
	size_t image_height = height;
	size_t image_depth = depth;
	size_t region[] = {image_width,image_height,depth};
	size_t row_pitch = 0;
	size_t slice_pitch = 0;

	// input and output arrays (for readability)
	// all writes and create functions use input_array
	// all reads use output_array
	DynamicArray<cl_int4> input_array(arraySize);
	DynamicArray<cl_int4> output_array(arraySize);

	// top be used for mappings
	cl_int4* mapped_pointer_to_buffer = NULL;
	cl_int4* mapped_pointer_to_image = NULL;

	cl_image_format image_format;
	image_format.image_channel_order = CL_RGBA;
	image_format.image_channel_data_type = CL_SIGNED_INT32;
	
	cl_mem buffers[] = {0,0};
	cl_mem images[] = {0,0};

	// setup CPU device 
	ASSERT_NO_FATAL_FAILURE(getCPUDevice(&ocl_descriptor.platform, &ocl_descriptor.device));
	
	// create context
	cl_context_properties properties[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)ocl_descriptor.platform, 0};
	ASSERT_NO_FATAL_FAILURE(createContext(&ocl_descriptor.context, properties, 1, &ocl_descriptor.device, NULL, NULL));
		
	//	create and build program
	ASSERT_NO_FATAL_FAILURE(createAndBuildProgramWithSource("kernels.cl", &ocl_descriptor.program, 
		ocl_descriptor.context, 1, &ocl_descriptor.device, NULL, NULL, NULL));
		
	// create queue
	ASSERT_NO_FATAL_FAILURE(createCommandQueue(&ocl_descriptor.queue, ocl_descriptor.context, 
		ocl_descriptor.device, queue_properties));

	// create buffers and images
	for(int i=0; i<2; ++i)
	{
		// create buffer
		ASSERT_NO_FATAL_FAILURE(createBuffer(&buffers[i], 
			ocl_descriptor.context, CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR, 
			sizeof(cl_int4)*arraySize, input_array.dynamic_array));

		// create image
		ASSERT_NO_FATAL_FAILURE(createImage3D(&images[i], ocl_descriptor.context, 
			CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR, 
			&image_format, image_width, image_height, image_depth, row_pitch, slice_pitch, 
			input_array.dynamic_array));
	}

	// create kernel
	ASSERT_NO_FATAL_FAILURE(createKernel(&ocl_descriptor.kernel ,ocl_descriptor.program, "kernel_a"));
	
	// set kernel arguments
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernel, 0, sizeof(cl_mem), &buffers[0]));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernel, 1, sizeof(int), &value));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernel, 2, sizeof(int), &arraySize));

	// set work dimensions
	cl_uint work_dim = 1;
	size_t global_work_size = 1;

	// read from buffer[0]
	ASSERT_NO_FATAL_FAILURE(enqueueReadBuffer(ocl_descriptor.queue, 
		buffers[0], CL_FALSE, 0,sizeof(cl_int4)*arraySize, 
		output_array.dynamic_array, 
		0, NULL, NULL));
	
	// write from input array to buffer[1]
	ASSERT_NO_FATAL_FAILURE(enqueueWriteBuffer(ocl_descriptor.queue, 
		buffers[1], CL_FALSE, 0,sizeof(cl_int4)*arraySize, 
		input_array.dynamic_array, 
		0, NULL, NULL));

	// read rect from buffer[0] to array 0
	ASSERT_NO_FATAL_FAILURE(enqueueReadBufferRect(ocl_descriptor.queue, 
		buffers[0], CL_FALSE,
		origin, origin, rect_region,
		row_pitch, slice_pitch, row_pitch, slice_pitch, 
		output_array.dynamic_array, 
		0, NULL, NULL));

	// write from array 1 from buffer 1
	ASSERT_NO_FATAL_FAILURE(enqueueWriteBufferRect(ocl_descriptor.queue, 
		buffers[1], CL_FALSE,
		origin, origin, rect_region,
		row_pitch, slice_pitch, row_pitch, slice_pitch, 
		input_array.dynamic_array, 
		0, NULL, NULL));

	// copy buffer 0 to buffer 1
	ASSERT_NO_FATAL_FAILURE(enqueueCopyBuffer(ocl_descriptor.queue, 
		buffers[0], buffers[1],
		0, 0, sizeof(cl_int4)*arraySize, 
		0, NULL, NULL));

	// copy from 0 to 1
	ASSERT_NO_FATAL_FAILURE(enqueueCopyBufferRect(ocl_descriptor.queue, buffers[0], 
		buffers[1], origin, origin, rect_region,	
		row_pitch, slice_pitch, row_pitch, slice_pitch,
		0, NULL, NULL));

	
	// map buffer 0
	ASSERT_NO_FATAL_FAILURE(enqueueMapBuffer(&mapped_pointer_to_buffer, ocl_descriptor.queue, 
		buffers[0], CL_FALSE, CL_MAP_READ, 0, sizeof(cl_int4)*arraySize, 
		0, NULL, NULL));	

	ASSERT_NO_FATAL_FAILURE(enqueueUnmapMemObject(ocl_descriptor.queue, buffers[0], 
		mapped_pointer_to_buffer, 
		0, NULL, NULL));

	// enqueue kernel
	ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(ocl_descriptor.queue, ocl_descriptor.kernel, 
		1, NULL, &global_work_size, NULL, 
		0, NULL, NULL));

	// enqueue task
	ASSERT_NO_FATAL_FAILURE(enqueueTask(ocl_descriptor.queue, ocl_descriptor.kernel, 
		0, NULL, NULL));

	// enqueue native kernel
	struct arg_struct
	{
		cl_mem inputStream;
		cl_mem outputStream;
		cl_int count;
	} args;
	// Set up the arrays to call with
	args.inputStream = buffers[ 0 ];
	args.outputStream = buffers[ 1 ];
	args.count = arraySize;
	void * memLocs[ 2 ] = { &args.inputStream, &args.outputStream };
	/*ASSERT_EQ(CL_SUCCESS,  clEnqueueNativeKernel(ocl_descriptor.queue, test_native_kernel_fn, 
									  &args, sizeof( args ),
									  2, &buffers[0],
									  (const void **)memLocs, 
									  0, NULL, NULL));
	*/
	// enqueue marker
	cl_event command_event = 0;
	ASSERT_NO_FATAL_FAILURE(enqueueMarker(ocl_descriptor.queue, &command_event));

	// wait for completion of device_done_event
	ASSERT_NO_FATAL_FAILURE(waitForEvents(1, &command_event)) << "waitForEvents failed";

	// enqueue barrier
	ASSERT_EQ(CL_SUCCESS, clEnqueueBarrier(ocl_descriptor.queue));

	// enqueue kernel again
	ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(ocl_descriptor.queue, ocl_descriptor.kernel, 
		1, NULL, &global_work_size, NULL, 
		0, NULL, NULL));

	// read image 0
	ASSERT_NO_FATAL_FAILURE(enqueueReadImage(ocl_descriptor.queue, images[0], 
		CL_FALSE, origin, region, row_pitch, slice_pitch, output_array.dynamic_array, 
		0, NULL, NULL));

	// write image 1
	ASSERT_NO_FATAL_FAILURE(enqueueWriteImage(ocl_descriptor.queue, images[1], 
		CL_FALSE, origin, region, row_pitch, slice_pitch, input_array.dynamic_array, 
		0, NULL, NULL));

	// copy from image 0 to image 1
	ASSERT_NO_FATAL_FAILURE(enqueueCopyImage(ocl_descriptor.queue, 
		images[0], images[1], 
		origin, origin, region, 
		0, NULL, NULL));

	// copy from image 0 to buffer 0
	ASSERT_NO_FATAL_FAILURE(enqueueCopyImageToBuffer(ocl_descriptor.queue, 
		images[0], buffers[0],
		origin, region, 0, 
		0, NULL, NULL));

	// copy from buffer 1 to image 1
	ASSERT_NO_FATAL_FAILURE(enqueueCopyBufferToImage(ocl_descriptor.queue, 
		buffers[1], images[1],
		0, origin, region, 
		0, NULL, NULL));

	// map image 0
	ASSERT_NO_FATAL_FAILURE(enqueueMapImage(&mapped_pointer_to_image, ocl_descriptor.queue, 
		images[0], CL_FALSE, CL_MAP_READ, 
		origin, region, &row_pitch, &slice_pitch,
		0, NULL, NULL));

	// unmap image 0 from array 3
	ASSERT_NO_FATAL_FAILURE(enqueueUnmapMemObject(ocl_descriptor.queue, images[0], 
		mapped_pointer_to_image, 
		0, NULL, NULL));

	for(int i=0; i<2; ++i)
	{
		// release buffers
		if(0!=buffers[i])
		{
			EXPECT_EQ(CL_SUCCESS, clReleaseMemObject(buffers[i])) << "clReleaseMemObject failed";
			buffers[i] = 0;
		}

		// release images
		if(0!=images[i])
		{
			EXPECT_EQ(CL_SUCCESS, clReleaseMemObject(images[i])) << "clReleaseMemObject failed";
			images[i]
			= 0;
		}
	}
}

//|	TEST: GPA.TC3_InOrderQueueCommands (TC-3)
//|
//|	Purpose
//|	-------
//|	
//|	Test OpenCL commands instrumentation with in order queue
//|
//|	Method
//|	------
//|
//|	1. Create OpenCL in order queue
//| 2. Create 2 OpenCL buffers, 2 images and a kernel.
//| 3. Enqueue all type of OpenCL commands (with no dependencies) to the OpenCL queue.
//|		See the list of enqueued commands in documentation of enqueueCommands function.
//|
//|	Pass criteria
//|	-------------
//|
//|	
//|
TEST_F(GPA, TC3_InOrderQueueCommands)
{	
	ASSERT_NO_FATAL_FAILURE(enqueueCommands(ocl_descriptor, CL_QUEUE_PROFILING_ENABLE));
}

//|	TEST: GPA.TC4_OutOfOrderQueueCommands (TC-4)
//|
//|	Purpose
//|	-------
//|	
//|	Test OpenCL commands instrumentation with out of order queue
//|
//|	Method
//|	------
//|
//|	1. Create OpenCL out of order queue
//| 2. Create 2 OpenCL buffers, 2 images and a kernel.
//| 3. Enqueue all type of OpenCL commands (with no dependencies) to the OpenCL queue.
//|		See the list of enqueued commands in documentation of enqueueCommands function.
//|
//|	Pass criteria
//|	-------------
//|
//|	
//|
TEST_F(GPA, TC4_OutOfOrderQueueCommands)
{	
	ASSERT_NO_FATAL_FAILURE(enqueueCommands(ocl_descriptor, CL_QUEUE_PROFILING_ENABLE|CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE));
}
