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

// NUM_QUEUES - number of queues of each kind (in-order and out-of-order)
#define NUM_QUEUES 20

//|	TEST: GPA.TC8_ManyOpenCLComamndQueues (TC-8)
//|
//|	Purpose
//|	-------
//|	
//|	Test the ability to support many OpenCL command queues
//|
//|	Method
//|	------
//|	1. Create NUM_QUEUES in-order queues
//|	2. Create NUM_QUEUES out-of-order queues
//|	3. Create an OpneCL buffer and kernel
//|	4. For each queue created:
//|		i. Enqueue write buffer command
//|		ii. Enqueue NDRange
//|	5. Wait for all queues to finish their commands (clFinish)
//|	
//|	Pass criteria
//|	-------------
//|
//|	
//|
TEST_F(GPA, TC8_ManyOpenCLComamndQueues)
{
	// create and initialize array
	int arraySize = 2;
	int value = 5;
	DynamicArray<cl_int> input_array(arraySize);
	
	// Arrays and OpenCL object for in order and out of order queues
	// first 100 queues are in-order, the other 100 are out-of-order
	DynamicArray<cl_int> output_array(arraySize);
	cl_command_queue queues[NUM_QUEUES*2];

	// setup CPU device 
	ASSERT_NO_FATAL_FAILURE(getCPUDevice(&ocl_descriptor.platform, &ocl_descriptor.device));
	
	// create context
	cl_context_properties properties[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)ocl_descriptor.platform, 0};
	ASSERT_NO_FATAL_FAILURE(createContext(&ocl_descriptor.context, properties, 1, &ocl_descriptor.device, NULL, NULL));
		
	//	create and build program
	ASSERT_NO_FATAL_FAILURE(createAndBuildProgramWithSource("kernels.cl", &ocl_descriptor.program, 
		ocl_descriptor.context, 1, &ocl_descriptor.device, NULL, NULL, NULL));
		
	// create 100 in-order queue
	for(int i=0; i<NUM_QUEUES; ++i)
	{
		ASSERT_NO_FATAL_FAILURE(createCommandQueue(&queues[i], ocl_descriptor.context, 
			ocl_descriptor.device, CL_QUEUE_PROFILING_ENABLE));
	}

	// create 100 out-of-order queue
	for(int i=NUM_QUEUES; i<NUM_QUEUES*2; ++i)
	{
		ASSERT_NO_FATAL_FAILURE(createCommandQueue(&queues[i], ocl_descriptor.context, 
			ocl_descriptor.device, CL_QUEUE_PROFILING_ENABLE|CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE));
	}

	// create buffer
	ASSERT_NO_FATAL_FAILURE(createBuffer(&ocl_descriptor.buffer, 
		ocl_descriptor.context, CL_MEM_READ_WRITE|CL_MEM_ALLOC_HOST_PTR, 
		sizeof(cl_int)*arraySize, NULL));	
		
	// create kernel
	ASSERT_NO_FATAL_FAILURE(createKernel(&ocl_descriptor.kernel ,ocl_descriptor.program, "kernel_a"));
	
	// set kernel arguments
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernel, 0, sizeof(cl_mem), &ocl_descriptor.buffer));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernel, 1, sizeof(int), &value));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernel, 2, sizeof(int), &arraySize));	
	
	// set work dimensions
	cl_uint work_dim = 1;
	size_t global_work_size = 1;

	for(int i=0; i<NUM_QUEUES*2; ++i)
	{
		// enqueue in order commands to be executed with dependency on user event
		// write from input array to buffer
		ASSERT_NO_FATAL_FAILURE(enqueueWriteBuffer(queues[i], 
			ocl_descriptor.buffer, CL_FALSE, 0,sizeof(cl_int)*arraySize, 
			input_array.dynamic_array, 
			0, NULL, NULL));

		// enqueue kernel
		ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(queues[i], ocl_descriptor.kernel, 
			1, NULL, &global_work_size, NULL, 
			0, NULL, NULL));
	}

	// wait till all enqueued commands are finished on all queues
	for(int i=0; i<NUM_QUEUES*2; ++i)
	{
		clFinish(queues[i]);
	}

	for(int i=0; i<NUM_QUEUES*2; ++i)
	{
		// release queus
		if (0 != queues[i])
		{
			EXPECT_EQ(CL_SUCCESS, clReleaseCommandQueue(queues[i])) << "clReleaseCommandQueue failed";
			queues[i] = 0;
		}
	}
}
