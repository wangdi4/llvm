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

//| enqueueAll
//|	1. Create 2 OpenCL command queues: in-order and out of order queues	
//|	2. For each queue create OpenCL buffer and kernel
//|	3. Create uder event
//| 4. For in order queue:
//|		i. Enqueue write to buffer dependant on status of user event from step 3 
//|		ii. Enqueue NDRange dependant on status of user event from step 3 
//|		iii. Enqueue read from buffer dependant on status of user event from step 3 
//| 4. For out of order queue:
//|		i. Enqueue write to buffer dependant on completion of command from step 4.i 
//|		ii. Enqueue NDRange dependant on completion of command from step 4.ii 
//|		iii. Enqueue read from dependant on completion of command from step 4.iii 
//|	5. Set user event to CL_COMPLETE
//|	6. Wait for all queues to finish their commands with clFinish
void enqueueAll(OpenCLDescriptor& ocl_descriptor)
{
	// create and initialize array
	int arraySize = 2;
	int value = 5;
	DynamicArray<cl_int> input_array(arraySize);
	
	// Arrays and OpenCL object for in order and out of order queues
	// 0 index is for in order queue
	// 1 index if doe out of order queue
	DynamicArray<cl_int> output_array1(arraySize);
	DynamicArray<cl_int> output_array2(arraySize);
	cl_mem buffers[] = {0,0};
	cl_kernel kernels[] = {0,0};
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

	// create buffers and kernels
	for(int i=0; i<2; ++i)
	{
		// create buffer
		ASSERT_NO_FATAL_FAILURE(createBuffer(&buffers[i], 
			ocl_descriptor.context, CL_MEM_READ_WRITE|CL_MEM_ALLOC_HOST_PTR, 
			sizeof(cl_int)*arraySize, NULL));	
		
		// create kernels
		ASSERT_NO_FATAL_FAILURE(createKernel(&kernels[i] ,ocl_descriptor.program, "kernel_a"));
	
		// set kernel arguments
		ASSERT_NO_FATAL_FAILURE(setKernelArg(kernels[i], 0, sizeof(cl_mem), &buffers[i]));
		ASSERT_NO_FATAL_FAILURE(setKernelArg(kernels[i], 1, sizeof(int), &value));
		ASSERT_NO_FATAL_FAILURE(setKernelArg(kernels[i], 2, sizeof(int), &arraySize));		
	}
	
	// create user event
	cl_event user_event = 0;
	int numEvents = 3;
	cl_event command_done_event[3];
	for(int i=0; i<numEvents; ++i)
	{
		command_done_event[i] = 0;
	}
	
	ASSERT_NO_FATAL_FAILURE(createUserEvent(&user_event, ocl_descriptor.context));

	// set work dimensions
	cl_uint work_dim = 1;
	size_t global_work_size = 1;

	// enqueue in order commands to be executed with dependency on user event
	// write from input array to buffer
	ASSERT_NO_FATAL_FAILURE(enqueueWriteBuffer(queues[0], 
		buffers[0], CL_FALSE, 0,sizeof(cl_int)*arraySize, 
		input_array.dynamic_array, 
		1, &user_event, &command_done_event[0]));

	// enqueue kernel
	ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(queues[0], kernels[0], 
		1, NULL, &global_work_size, NULL, 
		1, &user_event, &command_done_event[1]));

	// read from buffer
	ASSERT_NO_FATAL_FAILURE(enqueueReadBuffer(queues[0], 
		buffers[0], CL_FALSE, 0,sizeof(cl_int)*arraySize, 
		output_array1.dynamic_array, 
		1, &user_event, &command_done_event[2]));
	
	// enqueue out of order coomands with dependency on completion of corresponding in-order queue commands
	// write from input array to buffer
	ASSERT_NO_FATAL_FAILURE(enqueueWriteBuffer(queues[1], 
		buffers[1], CL_FALSE, 0,sizeof(cl_int)*arraySize, 
		input_array.dynamic_array, 
		1, &command_done_event[0], NULL));

	// enqueue kernel
	ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(queues[1], kernels[1], 
		1, NULL, &global_work_size, NULL, 
		1, &command_done_event[1], NULL));

	// read from buffer
	ASSERT_NO_FATAL_FAILURE(enqueueReadBuffer(queues[1], 
		buffers[1], CL_FALSE, 0,sizeof(cl_int)*arraySize, 
		output_array2.dynamic_array, 
		1, &command_done_event[2], NULL));
	
	// set user_event as CL_COMPLETE
	// so that commands in in-order queue can begin execution
	// wait till all enqueued commands are finished on both queues
	setUserEventStatus(user_event, CL_COMPLETE);
	for(int i=0; i<2; ++i)
	{
		clFinish(queues[i]);
	}

	for(int i=0; i<2; ++i)
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
		// release queus
		if (0 != queues[i])
		{
			EXPECT_EQ(CL_SUCCESS, clReleaseCommandQueue(queues[i])) << "clReleaseCommandQueue failed";
			queues[i] = 0;
		}
	}
}

//|	TEST: GPA.TC5_CommandExecutionInstrumentation1 (TC-5)
//|
//|	Purpose
//|	-------
//|	
//|	Test OpenCL command's execution instrumentation
//|
//|	Method
//|	------
//|
//|	1. Create 2 OpenCL command queues: in-order and out of order queues	
//|	2. For each queue create OpenCL buffer and kernel
//|	3. Create uder event
//| 4. For in order queue:
//|		i. Enqueue write to buffer dependant on status of user event from step 3 
//|		ii. Enqueue NDRange dependant on status of user event from step 3 
//|		iii. Enqueue read from buffer dependant on status of user event from step 3 
//| 4. For out of order queue:
//|		i. Enqueue write to buffer dependant on completion of command from step 4.i 
//|		ii. Enqueue NDRange dependant on completion of command from step 4.ii 
//|		iii. Enqueue read from dependant on completion of command from step 4.iii 
//|	5. Set user event to CL_COMPLETE
//|	6. Wait for all queues to finish their commands with clFinish
//|	
//|	
//|	Pass criteria
//|	-------------
//|
//|	
//|
TEST_F(GPA, TC5_CommandExecutionInstrumentation1)
{
	enqueueAll(ocl_descriptor);
}

//|	TEST: GPA.TC5_CommandExecutionInstrumentation2 (TC-6)
//|
//|	Purpose
//|	-------
//|	
//|	Test OpenCL command's execution instrumentation
//|
//|	Method
//|	------
//|
//|	1. Create 2 OpenCL command queues: in-order and out of order queues	
//|	2. For each queue create OpenCL buffer and kernel
//|	3. Create uder event
//| 4. For in order queue:
//|		i. Enqueue write to buffer dependant on status of user event from step 3 
//|		ii. Enqueue NDRange dependant on status of user event from step 3 
//|		iii. Enqueue read from buffer dependant on status of user event from step 3 
//| 4. For out of order queue:
//|		i. Enqueue write to buffer dependant on completion of command from step 4.i 
//|		ii. Enqueue NDRange dependant on completion of command from step 4.ii 
//|		iii. Enqueue read from dependant on completion of command from step 4.iii 
//|	5. Set user event to CL_COMPLETE
//|	6. Wait for all queues to finish their commands with clFinish
//|	
//|	
//|	Pass criteria
//|	-------------
//|
//|	
//|
TEST_F(GPA, TC5_CommandExecutionInstrumentation2)
{
	enqueueAll(ocl_descriptor);
}