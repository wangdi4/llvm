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

//|	TEST: GPA.TC7_CommandOVerlappingInstrumentation (TC-7)
//|
//|	Purpose
//|	-------
//|	
//|	Test OpenCL overlapping instrumentation
//|
//|	Method
//|	------
//|	1. Create 2 OpenCL queues: in-order and out-of-order
//|	2. Create 5 OpenCL kernels, 4 short ones and 1 long one (run-time)
//|	3. For each kernel create a buffer
//|	4. Enqueue long kernel to in-order queue
//| 5. Enqueue first short kernel into out-of-order queue
//|	6. Enqueue first 2 short kernels into out-of-order queue while their execution is dependant 
//|		on completion of long kernel from in-order queue
//| 7. Enqueue last 2 (third and forth) short kernels into out-of-order queue
//|	8. Wait for comamnds completion on both queues with clFinish
//|	
//|	Pass criteria
//|	-------------
//|
//|	
//|
TEST_F(GPA, TC7_CommandOVerlappingInstrumentation)
{
	// create and initialize array
	int arraySize = 256;
	int value = 5;
	// Arrays and OpenCL object for in order and out of order queues
	DynamicArray<cl_int> input_array(arraySize);
	// each buffer[i] is for kernel[i]
	cl_mem buffers[5];
	cl_kernel kernels[5];
	// in order queue will be at index 0, out of order at index 1
	cl_command_queue queues[] = {0,0};

	// setup CPU device 
	ASSERT_NO_FATAL_FAILURE(getCPUDevice(&ocl_descriptor.platform, &ocl_descriptor.device));
	
	// create context
	cl_context_properties properties[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)ocl_descriptor.platform, 0};
	ASSERT_NO_FATAL_FAILURE(createContext(&ocl_descriptor.context, properties, 1, &ocl_descriptor.device, NULL, NULL));
		
	//	create and build program
	ASSERT_NO_FATAL_FAILURE(createAndBuildProgramWithSource("kernels.cl", &ocl_descriptor.program, 
		ocl_descriptor.context, 1, &ocl_descriptor.device, NULL, NULL, NULL));

	// create in order queue at index 0
	ASSERT_NO_FATAL_FAILURE(createCommandQueue(&queues[0], ocl_descriptor.context, 
		ocl_descriptor.device, CL_QUEUE_PROFILING_ENABLE));

	// create out of order queue at index 1
	ASSERT_NO_FATAL_FAILURE(createCommandQueue(&queues[1], ocl_descriptor.context, 
		ocl_descriptor.device, CL_QUEUE_PROFILING_ENABLE|CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE));

	// for long kernel:
	// create buffer
	ASSERT_NO_FATAL_FAILURE(createBuffer(&buffers[0], 
		ocl_descriptor.context, CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR, 
		sizeof(cl_int)*arraySize, input_array.dynamic_array));	
		
	// create kernel
	ASSERT_NO_FATAL_FAILURE(createKernel(&kernels[0] ,ocl_descriptor.program, "kernel_long"));
	
	// set kernel arguments
	ASSERT_NO_FATAL_FAILURE(setKernelArg(kernels[0], 0, sizeof(cl_mem), &buffers[0]));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(kernels[0], 1, sizeof(int), &value));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(kernels[0], 2, sizeof(int), &arraySize));

	// for short kernels:
	for(int i=1; i<5; ++i)
	{
		// create buffer
		ASSERT_NO_FATAL_FAILURE(createBuffer(&buffers[i], 
			ocl_descriptor.context, CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR, 
			sizeof(cl_int)*arraySize, input_array.dynamic_array));	
		
		// create kernels
		ASSERT_NO_FATAL_FAILURE(createKernel(&kernels[i] ,ocl_descriptor.program, "kernel_a"));
	
		// set kernel arguments
		ASSERT_NO_FATAL_FAILURE(setKernelArg(kernels[i], 0, sizeof(cl_mem), &buffers[i]));
		ASSERT_NO_FATAL_FAILURE(setKernelArg(kernels[i], 1, sizeof(int), &value));
		ASSERT_NO_FATAL_FAILURE(setKernelArg(kernels[i], 2, sizeof(int), &arraySize));
	}

	cl_event command_done_event = 0;
	
	// set work dimensions
	cl_uint work_dim = 1;
	size_t global_work_size = 1;

	// enqueue long kernel to in-order queue
	ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(queues[0], kernels[0], 
		1, NULL, &global_work_size, NULL, 
		0, NULL, &command_done_event));

	// enqueue first short kernel to out-of-order queue
	ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(queues[1], kernels[1], 
		1, NULL, &global_work_size, NULL, 
		0, NULL, NULL));

	// enqueue first and second short kernels to out-of-order queue, 
	// dependant on completion of long kernel from in-order queue
	ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(queues[1], kernels[1], 
		1, NULL, &global_work_size, NULL, 
		1, &command_done_event, NULL));

	ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(queues[1], kernels[2], 
		1, NULL, &global_work_size, NULL, 
		1, &command_done_event, NULL));

	// enqueue third and forth short kernels to out-of-order queue
	ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(queues[1], kernels[3], 
		1, NULL, &global_work_size, NULL, 
		0, NULL, NULL));

	ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(queues[1], kernels[4], 
		1, NULL, &global_work_size, NULL, 
		0, NULL, NULL));
	
	// wait till all enqueued commands are finished on both queues
	for(int i=0; i<2; ++i)
	{
		clFinish(queues[i]);
	}

	for(int i=0; i<5; ++i)
	{
		// release buffers
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
	for(int i=0; i<2; ++i)
	{
		// release queus
		if (0 != queues[i])
		{
			EXPECT_EQ(CL_SUCCESS, clReleaseCommandQueue(queues[i])) << "clReleaseCommandQueue failed";
			queues[i] = 0;
		}
	}
}
