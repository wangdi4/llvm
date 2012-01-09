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

#ifndef VR7_GTEST_
#define VR7_GTEST_

#include "common_runtime_tests.h"

//|	testDeviceBufferSynchBody - helper function for CPUThenGPUBufferSynch and GPUThenCPUBufferSynch tests
//|	Responsible for enqueueing kernels and map buffer with provided cl_event dependencies int he parameters
//|
//|	1. Call clGetPlatformIDs to obrain single platfrom
//|	2. Call clGetDeviceIDs to obtain CPU and GPU devices
//|	3. Call clCreateContext to obtain common context for CPU and GPU
//|	4. Create command queue for each device
//|	5. Create cl_mem buffer for each device
//|	6. Create and build cl_program for kernel "queue_synch" in file common_runtime_kernels.cl (should be located 
//|	in working directory)
//|	7. Create cl_kernel for each device

//|	In this function there are 2 devices, 2 queues and 2 kernels. For each device - it's OpenCL objects index in devices,
//|	queues and kernels is kept. For example:
//|	CPU index in devices is i <=> CPU index in queues is i <=> CPU index in kernels is i.
//|	
//|	Each device is supposed to execute it's kernel with it's own kernel parameters.
//|	Device with index firstDeviceIndex will execute it's own kernel, while device with index
//|	(1-firstDeviceIndex) will wait until firstDeviceIndex's kernel completes its execution with use of cl_event.
//|	
//|	1. Set kernel arguments for each kernel
//|	firstDeviceIndex's arguments are initial_pattern and first_replacement
//|	It will replace all occurances of initial_pattern with first_replacement
//|	(1-firstDeviceIndex)'s arguments are first_replacement and second_replacement
//|	It will replace all occurances of first_replacement with second_replacement
//|	This way when kernel for firstDeviceIndex is executed before (1-firstDeviceIndex) device
//|	We won't see any occrances of initial_pattern and first_replacement and only see second_replacement
//|	(the buffer is initialized with initial_pattern)
//|	
//|	2. Define 3 cl_events to be later used as events for completion of each kernel, and completion of map buffer
//|	
//|	3. Create user cl_event that will be used as precondition event for each kernel to be enqueued
//|
//|	4. For device firstDeviceIndex we call clEnqueueNDRangeKernel that waits for completion of user cl_event (from step 3) 
//|	and upon completion returns cl_event[firstDeviceIndex] (one of the events from step 2)
//|
//|	5. For device (1-firstDeviceIndex) we call clEnqueueNDRangeKernel that waits for completion of firstDeviceIndex
//|	(from step 4) by use of completion event of firstDeviceIndex's kernel - cl_event[firstDeviceIndex]. 
//|	(1-firstDeviceIndex)'s kernel returns upon completion cl_event[1-firstDeviceIndex]
//|
//|	6. Enqueue map buffer on (1-firstDeviceIndex)'s queue (as it's supposed to be executed last) with precondition
//|	cl_event[1-firstDeviceIndex] - waits for completion of (1-firstDeviceIndex)'s kernel. map buffer returns 
//|	cl_event[2] upon completion.
//|
//|	7. Wait certain amount of time
//|
//|	8. Validate that each kernel is still queued (by use of clGetEventInfo for each completion event)
//|
//|	9. Set status CL_COMPLETED for user event from step 3
//|
//|	10. Wait for completion of all events mentioned in step 2 (thus completion of all kernels and map buffer)
//|	
//|	11. Validade that buffer's content is what it should be - that we won't see any occrances of initial_pattern 
//|	and  first_replacement and only see second_replacement (since the buffer is initialized with initial_pattern)
//|
static void testDeviceBufferSynchBody(OpenCLDescriptor& ocl_descriptor, int firstDeviceIndex, 
	int initial_pattern, int first_replacement, int second_replacement)
{
	// initialize array
	size_t arraySize = 128;
	DynamicArray<int> input_array(arraySize, initial_pattern);

	// set up platform, expect return of a single platform
	ASSERT_NO_FATAL_FAILURE(getCPUGPUDevicesIfNotCreated(ocl_descriptor));
	
	// create shared context
	cl_context_properties properties[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)ocl_descriptor.platforms[0], 0};
	ASSERT_NO_FATAL_FAILURE(createContext(&ocl_descriptor.context, properties, 2, ocl_descriptor.devices, NULL, NULL));
			
	// create queues and buffers
	// get device info (to determine which device resides on what index)
	for(int i=0; i<2; ++i){
		ASSERT_NO_FATAL_FAILURE(createCommandQueue(&ocl_descriptor.queues[i], ocl_descriptor.context, ocl_descriptor.devices[i], 0));
		
	}
	// create shared buffer
	ASSERT_NO_FATAL_FAILURE(createBuffer(&ocl_descriptor.in_common_buffer, ocl_descriptor.context, CL_MEM_READ_WRITE|CL_MEM_USE_HOST_PTR, sizeof(int)*input_array.dynamic_array_size, input_array.dynamic_array));

	// set up manual destruction 
	ASSERT_NO_FATAL_FAILURE(setDeleteArrayOnCallback(ocl_descriptor.in_common_buffer, input_array));

	//	create and build program
	ASSERT_NO_FATAL_FAILURE(createAndBuildProgramWithSource("synch.cl", &ocl_descriptor.program, ocl_descriptor.context, 2, ocl_descriptor.devices, 
		NULL, NULL, NULL));
	
	// create kernels
	for(int i=0; i<2; ++i)
	{	
		ASSERT_NO_FATAL_FAILURE(createKernel(&ocl_descriptor.kernels[i] , ocl_descriptor.program, "queue_synch"));
	}

	// set kernel arguments for first device
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[firstDeviceIndex], 0, sizeof(cl_mem), &ocl_descriptor.in_common_buffer));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[firstDeviceIndex], 1, sizeof(int), &initial_pattern));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[firstDeviceIndex], 2, sizeof(int), &first_replacement));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[firstDeviceIndex], 3, sizeof(int), &input_array.dynamic_array_size));	

	// set kernel arguments for second device	
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[1-firstDeviceIndex], 0, sizeof(cl_mem), &ocl_descriptor.in_common_buffer));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[1-firstDeviceIndex], 1, sizeof(int), &first_replacement));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[1-firstDeviceIndex], 2, sizeof(int), &second_replacement));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[1-firstDeviceIndex], 3, sizeof(int), &input_array.dynamic_array_size));

	// create user event
	cl_event user_event = 0;
	cl_event device_done_event[] = {0,0,0};
	ASSERT_NO_FATAL_FAILURE(createUserEvent(&user_event, ocl_descriptor.context));

	// enqueue both kernels with required dependency (CPU then GPU)
	cl_uint work_dim = 1;
	size_t global_work_size = 1;
	ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(ocl_descriptor.queues[firstDeviceIndex], ocl_descriptor.kernels[firstDeviceIndex], 1, NULL, &global_work_size, NULL, 1, &user_event, &device_done_event[firstDeviceIndex]));
	ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(ocl_descriptor.queues[1-firstDeviceIndex], ocl_descriptor.kernels[1-firstDeviceIndex], 1, NULL, &global_work_size, NULL, 1, &device_done_event[firstDeviceIndex],  &device_done_event[1-firstDeviceIndex]));
	ASSERT_NO_FATAL_FAILURE(enqueueMapBuffer(&input_array.dynamic_array, ocl_descriptor.queues[1-firstDeviceIndex], ocl_descriptor.in_common_buffer, CL_FALSE, CL_MAP_READ, 0, sizeof(int)*input_array.dynamic_array_size, 1, &device_done_event[1-firstDeviceIndex], &device_done_event[2]));
	
	ASSERT_NO_FATAL_FAILURE(enqueueUnmapMemObject(ocl_descriptor.queues[1-firstDeviceIndex], ocl_descriptor.in_common_buffer, input_array.dynamic_array, 
		1, &device_done_event[2], NULL));

	// wait
	sleepMS(100);

	// check that both kernels are sill CL_QUEUED or CL_SUBMITTED
	for(int i=0; i<2; ++i)
	{
		ASSERT_NO_FATAL_FAILURE(validateQueuedOrSubmitted(device_done_event[i]));
	}

	// set user event as CL_COMPLETE
	ASSERT_NO_FATAL_FAILURE(setUserEventStatus(user_event, CL_COMPLETE));

	// wait for completion of kernels execution
	ASSERT_NO_FATAL_FAILURE(waitForEvents(3, device_done_event));
		
	// validate execution correctness
	ASSERT_NO_FATAL_FAILURE(input_array.compareArray(second_replacement));
}

//|	sharedBufferCPUGPUBody 
//|
//|	Purpose
//|	-------
//|	
//|	Verify the ability of different devices (CPU and GPU) to work in synchronization on the same shared memory
//|
//|	Method   
//|	------
//|
//|	1. Create cl_mem buffer on a shared context (shared context for CPU and GPU) of size arraySize*2
//|	2. For each device create kernel with buffer from step 1 as a parameter
//| 3. Enqueue kernels on queues of each device while kernels are waiting for user event
//|	   Enqueue map buffer waiting for kernel en of execution events.
//|	4. Wait some time
//|	5. Verify that all events are Queued
//|	6. Set event from step 3 as COMPLETED
//|	7. Wait for kernels to finish execution (wait for events)
//|	
//|	CPU's kernel will set first half of the buffer with CPU's specific pattern, GPU will set second half of the buffer
//|	with it's own pattern.
//|
//|	Pass criteria
//|	-------------
//|
//|	Validate that first half of the buffer is filled with CPU's pattern and second with GPU's
//|
static void sharedBufferCPUGPUBody(OpenCLDescriptor& ocl_descriptor)
{
	// initial_pattern - initial array element content
	int initial_pattern = 1;
	// all elements of array matching initial_pattern will be replaced with cpu_replacement in CPU
	int replacement[] = {3,4};
	// all elements of array matching cpu_replacement will be replaced with gpu_replacement in GPU

	// initialize array
	int arraySize = (size_t)2;
	//	actuall array size is arraySize * 2
	DynamicArray<cl_int> input_array(arraySize*2, initial_pattern);
	DynamicArray<cl_int> output_array(arraySize*2);

	// index of CPU objects in arrays (for GPU index will be 1-cpuIndex)
	int cpuIndex = 0;

	// allocate OpenCL objects and determined devices order
	// set up platform, expect return of a single platform
	
	// setup devices
	//	CPU is at index 0, GPU is at index 1
	ASSERT_NO_FATAL_FAILURE(getCPUGPUDevicesIfNotCreated(ocl_descriptor));
	
	// create shared context
	cl_context_properties properties[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)ocl_descriptor.platforms[0], 0};
	ASSERT_NO_FATAL_FAILURE(createContext(&ocl_descriptor.context, properties, 2, ocl_descriptor.devices, NULL, NULL));
		
	cl_mem in_common_buffer = 0;
	cl_mem sub_buffers[]={0,0};

	// create shared buffer
	// actual size is x2 (half for CPU and half for GPU)
	ASSERT_NO_FATAL_FAILURE(createBuffer(&in_common_buffer, ocl_descriptor.context, 
		CL_MEM_READ_WRITE|CL_MEM_USE_HOST_PTR, sizeof(cl_int)*arraySize*2, input_array.dynamic_array));

	// set up manual destruction of input_array
	ASSERT_NO_FATAL_FAILURE(setDeleteArrayOnCallback(in_common_buffer, input_array));

	//	create and build program
	ASSERT_NO_FATAL_FAILURE(createAndBuildProgramWithSource("synch.cl", &ocl_descriptor.program, 
		ocl_descriptor.context, 2, ocl_descriptor.devices, NULL, NULL, NULL));
		
	// create queues, sub buffers and kernels
	for(int i=0; i<2; ++i)
	{	
		// create queues 
		ASSERT_NO_FATAL_FAILURE(createCommandQueue(&ocl_descriptor.queues[i], ocl_descriptor.context, ocl_descriptor.devices[i], 0));
		
		cl_buffer_region input_buffer_region = { 0, 2* arraySize * sizeof(cl_int) };
		ASSERT_NO_FATAL_FAILURE(createSubBuffer(&sub_buffers[i], in_common_buffer,			
            CL_MEM_READ_WRITE,
			CL_BUFFER_CREATE_TYPE_REGION, &input_buffer_region));

		ASSERT_NO_FATAL_FAILURE(createKernel(&ocl_descriptor.kernels[i] , ocl_descriptor.program, "shared_synch"));

		// set kernel arguments for CPU device
		ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[i], 0, sizeof(cl_mem), &sub_buffers[i]));
		ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[i], 1, sizeof(int), &initial_pattern));
		ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[i], 2, sizeof(int), &replacement[i]));
		ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[i], 3, sizeof(int), &arraySize));
	}
	
	// create user event
	cl_event user_event = 0;
	cl_event device_done_event[] = {0,0,0,0};
	ASSERT_NO_FATAL_FAILURE(createUserEvent(&user_event, ocl_descriptor.context));
	
	cl_uint work_dim = 1;
	// global size will cause kernels determine which device is executing them
	size_t global_work_size[] = {1,2};

	// enqueue both kernels and map buffer with required dependency on user_event
	for(int i=0; i<2; ++i)
	{
		ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(ocl_descriptor.queues[i], ocl_descriptor.kernels[i], 1, NULL, 
			&global_work_size[i], NULL, 1, &user_event, &device_done_event[i]));
		ASSERT_NO_FATAL_FAILURE(enqueueMapBuffer(&input_array.dynamic_array, ocl_descriptor.queues[i], 
		in_common_buffer, CL_FALSE, CL_MAP_READ, 0, sizeof(int)*arraySize*2, 1, 
		&device_done_event[i], &device_done_event[2+i]));

		ASSERT_NO_FATAL_FAILURE(enqueueUnmapMemObject(ocl_descriptor.queues[i], in_common_buffer, 
		input_array.dynamic_array, 1, &device_done_event[2+i], NULL));
	}
	
	// wait
	sleepMS(100);

	// check that all events are still CL_QUEUED or CL_SUBMITTED
	for(int i=0; i<4; ++i)
	{
		ASSERT_NO_FATAL_FAILURE(validateQueuedOrSubmitted(device_done_event[i]));
	}
	
	// set user event as CL_COMPLETE
	ASSERT_NO_FATAL_FAILURE(setUserEventStatus(user_event, CL_COMPLETE));
	
	// wait for completion of kernels execution and map buffer
	ASSERT_NO_FATAL_FAILURE(waitForEvents(4, &device_done_event[0]));

	// validate execution correctness
	// create and initialize host array for results comparison
	DynamicArray<int> comparison_array(arraySize*2);
	for(int j=0; j<2; ++j)
	{
		for(int i=0; i<arraySize; ++i)
		{
			comparison_array.dynamic_array[j*arraySize+i] = replacement[j];
		}
	}
	ASSERT_NO_FATAL_FAILURE(input_array.compareArray(comparison_array));

	for(int i=0; i<2; ++i)
	{
		if(0!=sub_buffers[i]){
			EXPECT_EQ(CL_SUCCESS, clReleaseMemObject(sub_buffers[i])) << "clReleaseMemObject failed";
			sub_buffers[i] = 0;
		}
	}

	if(0!=in_common_buffer){
		EXPECT_EQ(CL_SUCCESS, clReleaseMemObject(in_common_buffer)) << "clReleaseMemObject failed";
		in_common_buffer = 0;
	}
}

#endif /* VR7_GTEST_ */
