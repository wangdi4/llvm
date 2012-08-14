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

#include "common_runtime_tests.h"
#include "common_methods.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "dynamic_array.h"

class CRT12_VR_124_127: public CommonRuntime{};


//|	TEST: CRT12_VR_124_127.FillBufferSynchronized_vr124
//|
//|	Purpose
//|	-------
//|	
//|	Verify support for the use of events to synchronize between the host and devices when using the function clEnqueueFillBuffer() to queue a fill command to the command-queues. 
//|
//|	Method
//|	------
//|
//|	1. Create a shared context
//| 2. Create an out-of-order queue to the CPU device
//| 3. Create an in-order queue to the GPU device
//| 4. Create a user-event
//| 5. Create two buffers (CPU-Buffer and GPU-Buffer)
//| 6. Queue Fill-Buffer command for CPU-Buffer to the CPU commands-queue with the user-event in the event_wait_list and pointer to CPU-Event in event
//| 7. Queue Fill-Buffer command for GPU-Buffer to the GPU commands-queue with the user-event in the event_wait_list and pointer to GPU-Event in event
//| 8. Queue Read-Buffer command for CPU-Buffer to the CPU commands-queue with CPU-event and GPU-event in the  event_wait_list
//| 9. Check that none of the commands is executed, yet
//| 10. Set the user-event to CL_COMPLETE
//| 11. Check that all the commands were completed and the CPU-Buffer has the right pattern
//|
//|	Pass criteria
//|	-------------
//|
//|	The host should be able to receive events either from CPU device, GPU device, or both of them.
//|

TEST_F(CRT12_VR_124_127, DISABLED_FillBufferSynchronized_vr124 ){ //I think there's a dead lock on line 111
	DynamicArray<cl_int> buffer1(2);
	DynamicArray<cl_int> buffer2(2);
	cl_int pattern[2]={1,2};

	memset(buffer1.dynamic_array,0,sizeof(cl_int)*8);
	memset(buffer2.dynamic_array,0,sizeof(cl_int)*8);
	cl_event_info event_status = 0;
	cl_event user_event = 0;
	cl_event read_event = 0;
	//those two elements will be used as list, so do not separate!
	cl_event event_list[2];


	// set up platform and CPU and GPU  devices 
	//	CPU is at index 0, GPU is at index 1
	ASSERT_NO_FATAL_FAILURE(getCPUGPUDevices(ocl_descriptor.platforms, ocl_descriptor.devices));	
	
	// set up context
	cl_context_properties properties[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)ocl_descriptor.platforms[0], 0};
	ASSERT_NO_FATAL_FAILURE(createContext(&ocl_descriptor.context, properties, 2, ocl_descriptor.devices, NULL, NULL));
	
	// create an out-of-order queue to the CPU device
	ASSERT_NO_FATAL_FAILURE(createCommandQueue(ocl_descriptor.queues, ocl_descriptor.context, ocl_descriptor.devices[0], CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE));

	// create an in-order queue to the GPU device
	ASSERT_NO_FATAL_FAILURE(createCommandQueue(&ocl_descriptor.queues[1], ocl_descriptor.context, ocl_descriptor.devices[1], 0));
	
	//create user event 
	ASSERT_NO_FATAL_FAILURE(createUserEvent(&user_event, ocl_descriptor.context));

	// create the buffer objects
	ASSERT_NO_FATAL_FAILURE(createBuffer(&ocl_descriptor.buffers[0], ocl_descriptor.context, CL_MEM_READ_WRITE|CL_MEM_USE_HOST_PTR, sizeof(cl_int)*8, buffer1.dynamic_array));
	ASSERT_NO_FATAL_FAILURE(createBuffer(&ocl_descriptor.buffers[1],ocl_descriptor.context,CL_MEM_READ_WRITE|CL_MEM_USE_HOST_PTR,sizeof(cl_int)*8,buffer2.dynamic_array));
	
	//enqueue buffers 
	ASSERT_NO_FATAL_FAILURE(enqueueFillBuffer(ocl_descriptor.queues[0], ocl_descriptor.buffers[0], pattern, sizeof(cl_int)*2, 0, sizeof(cl_int)*4, 1, &user_event, &event_list[0]));
	ASSERT_NO_FATAL_FAILURE(enqueueFillBuffer(ocl_descriptor.queues[1],ocl_descriptor.buffers[1],pattern,sizeof(cl_int)*2,0,sizeof(cl_int)*4,1,&user_event,&event_list[1]));
	
	
	//read from CPU device
	ASSERT_NO_FATAL_FAILURE(enqueueReadBuffer(ocl_descriptor.queues[0],ocl_descriptor.buffers[0],CL_FALSE,0,sizeof(int)*2,buffer1.dynamic_array,2,event_list,NULL));
	ASSERT_NO_FATAL_FAILURE(enqueueReadBuffer(ocl_descriptor.queues[1],ocl_descriptor.buffers[1],CL_FALSE,0,sizeof(int)*2,buffer2.dynamic_array,2,event_list,&read_event));

	sleepMS(500);

	//check that the kernel was not executed
	clGetEventInfo(read_event,CL_EVENT_COMMAND_EXECUTION_STATUS,sizeof(cl_int),&event_status,NULL);
	ASSERT_EQ(CL_QUEUED,event_status);

	//activate the user event
	setUserEventStatus(user_event,CL_COMPLETE);
	
	// wait
	ASSERT_NO_FATAL_FAILURE(finish(ocl_descriptor.queues[0]));
	ASSERT_NO_FATAL_FAILURE(finish(ocl_descriptor.queues[1]));

	//check that the kernel was executed
	clGetEventInfo(read_event,CL_EVENT_COMMAND_EXECUTION_STATUS,sizeof(cl_int),&event_status,NULL);
	ASSERT_EQ(CL_COMPLETE,event_status);

	for(int i = 0; i < 2; i++){
		printf("%d",buffer1.dynamic_array[i]);
		printf("%d",buffer2.dynamic_array[i]);
	}
	//check that output is as predicted
	ASSERT_TRUE(buffer1.dynamic_array[0]==1);
	ASSERT_TRUE(buffer1.dynamic_array[1]==2);
	ASSERT_TRUE(buffer2.dynamic_array[0]==1);
	ASSERT_TRUE(buffer2.dynamic_array[1]==2);
	
	releaseEvent(user_event);
	releaseEvent(read_event);
	for(int i = 0 ; i < 2 ; i++)
	{
		releaseEvent(event_list[i]);
	}
}


//|	TEST: CRT12_VR_124_127.MemoryObjectVisibilityReadWrite_vr126 (TC-1)
//|
//|	Purpose
//|	-------
//|	
//|	Verify that a buffer object created using the function clCreateBuffer()and the flag CL_MEM_READ_WRITE is visible to all devices in context 
//|
//|	Method
//|	------
//|
//|	1. Create a shared context queues etc
//| 2. Create two buffers (CPU-Buffer and GPU-Buffer)
//| 3. run a kernel that will change the memory object
//| 4. verify that the object changed.
//|
//|	Pass criteria
//|	-------------
//|
//|	the memory object is recognized in each of the devices.
//|

TEST_F(CRT12_VR_124_127, MemoryObjectVisibilityReadWrite_vr126){
	cl_uint work_dim = 1;
	size_t global_work_size = 1;
	size_t local_work_size = 1;
	cl_int num = 1;
	DynamicArray<cl_int> buffer1(1);
	DynamicArray<cl_int> buffer2(1);

	memset(buffer1.dynamic_array,0,sizeof(cl_int));
	memset(buffer2.dynamic_array,0,sizeof(cl_int));

	// set up shared context, program and queues with kernel1
	ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueues(ocl_descriptor, "simple_kernels.cl"));

	// create the buffer objects
	ASSERT_NO_FATAL_FAILURE(createBuffer(&ocl_descriptor.buffers[0], ocl_descriptor.context, CL_MEM_READ_WRITE|CL_MEM_USE_HOST_PTR, sizeof(cl_int), buffer1.dynamic_array));
	ASSERT_NO_FATAL_FAILURE(createBuffer(&ocl_descriptor.buffers[1],ocl_descriptor.context,CL_MEM_READ_WRITE|CL_MEM_USE_HOST_PTR,sizeof(cl_int),buffer2.dynamic_array));

	//create kernel and set arguments.
	ASSERT_NO_FATAL_FAILURE(createKernel(ocl_descriptor.kernels, ocl_descriptor.program, "kernel_1"));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], 0, sizeof(cl_mem), (void*)&ocl_descriptor.buffers[0]));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], 1, sizeof(cl_int), &num));

	// enqueue kernel on CPU 
	ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(ocl_descriptor.queues[0], ocl_descriptor.kernels[0], work_dim, 0, &global_work_size, &local_work_size,NULL,NULL,NULL));

	//read from CPU device
	ASSERT_NO_FATAL_FAILURE(enqueueReadBuffer(ocl_descriptor.queues[0],ocl_descriptor.buffers[0],CL_TRUE,0,sizeof(int)*1,buffer1.dynamic_array,NULL,NULL,NULL));

	//validate result
	ASSERT_EQ(num,buffer1.dynamic_array[0]);

	//set arguments for kernel
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0],0,sizeof(cl_mem),(void*)&ocl_descriptor.buffers[1]));

	// enqueue and run kernel on CPU 
	ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(ocl_descriptor.queues[1], ocl_descriptor.kernels[0], work_dim, 0, &global_work_size, &local_work_size,NULL,NULL,NULL));
	clFinish(ocl_descriptor.queues[1]);

	//read from CPU device
	ASSERT_NO_FATAL_FAILURE(enqueueReadBuffer(ocl_descriptor.queues[1],ocl_descriptor.buffers[1],CL_TRUE,0,sizeof(int)*1,buffer2.dynamic_array,NULL,NULL,NULL));
	ASSERT_EQ(num,buffer2.dynamic_array[0]);
}


//|	TEST: CRT12_VR_124_127.MemoryObjectVisibilityReadOnly_vr126 (TC-2)
//|
//|	Purpose
//|	-------
//|	
//|	Verify that a buffer object created using the function clCreateBuffer()and the flag CL_MEM_READ_ONLY is visible to all devices in context 
//|
//|	Method
//|	------
//|
//|	1. Create a shared context queues etc
//| 2. Create three buffers (CPU-Buffer, GPU-Buffer, and a return result buffer)
//| 3. run a kernel that will change the result buffer according to the input buffer 
//| 4. verify that the object changed.
//|
//|	Pass criteria
//|	-------------
//|
//|	the memory object is recognized in each of the devices.
//|

//maybe I should use one memory object?
TEST_F(CRT12_VR_124_127, MemoryObjectVisibilityReadOnly_vr126){
	cl_uint work_dim = 1;
	size_t global_work_size = 1;
	size_t local_work_size = 1;
	cl_int num = 1;
	DynamicArray<cl_int> buffer1(1);
	DynamicArray<cl_int> buffer2(1);
	DynamicArray<cl_int> buffer3(1);

	memset(buffer1.dynamic_array,1,sizeof(cl_int));
	memset(buffer2.dynamic_array,0,sizeof(cl_int));
	memset(buffer3.dynamic_array,0,sizeof(cl_int));

	buffer1.dynamic_array[0]=1;
	buffer2.dynamic_array[0]=1;

	// set up shared context, program and queues with kernel1
	ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueues(ocl_descriptor, "read_only_kernel.cl"));

	// create the buffer objects
	ASSERT_NO_FATAL_FAILURE(createBuffer(&ocl_descriptor.buffers[0], ocl_descriptor.context, CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR, sizeof(cl_int), buffer1.dynamic_array));
	//ASSERT_NO_FATAL_FAILURE(createBuffer(&ocl_descriptor.buffers[1],ocl_descriptor.context,CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR,sizeof(cl_int),buffer2.dynamic_array));
	ASSERT_NO_FATAL_FAILURE(createBuffer(&ocl_descriptor.out_common_buffer,ocl_descriptor.context,CL_MEM_READ_WRITE|CL_MEM_USE_HOST_PTR,sizeof(cl_int),buffer2.dynamic_array));

	//create kernel and set arguments.
	ASSERT_NO_FATAL_FAILURE(createKernel(ocl_descriptor.kernels, ocl_descriptor.program, "kernel_0"));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], 0, sizeof(cl_mem), (void*)&ocl_descriptor.buffers[0]));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], 1, sizeof(cl_mem), (void*)&ocl_descriptor.out_common_buffer));

	// enqueue kernel on CPU 
	ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(ocl_descriptor.queues[0], ocl_descriptor.kernels[0], work_dim, 0, &global_work_size, &local_work_size,NULL,NULL,NULL));

	//read from CPU device
	ASSERT_NO_FATAL_FAILURE(enqueueReadBuffer(ocl_descriptor.queues[0],ocl_descriptor.out_common_buffer,CL_TRUE,0,sizeof(int)*1,buffer3.dynamic_array,NULL,NULL,NULL));

	//validate result
	ASSERT_EQ(num,buffer3.dynamic_array[0]);

	//set arguments for GPU
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0],0,sizeof(cl_mem),(void*)&ocl_descriptor.buffers[1]));

	// enqueue kernel on CPU 
	ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(ocl_descriptor.queues[1], ocl_descriptor.kernels[0], work_dim, 0, &global_work_size, &local_work_size,NULL,NULL,NULL));

	//read from CPU device
	ASSERT_NO_FATAL_FAILURE(enqueueReadBuffer(ocl_descriptor.queues[1],ocl_descriptor.out_common_buffer,CL_TRUE,0,sizeof(int)*1,buffer3.dynamic_array,NULL,NULL,NULL));
	ASSERT_EQ(num,buffer3.dynamic_array[0]);
}


//|	TEST: CRT12_VR_124_127.MemoryObjectVisibilityWriteOnly_vr126 (TC-3)
//|
//|	Purpose
//|	-------
//|	
//|	Verify that a buffer object created using the function clCreateBuffer()and the flag CL_MEM_HOST_WRITE_ONLY is visible to all devices in context 
//|
//|	Method
//|	------
//|
//|	1. Create a shared context queues etc
//| 2. Create two buffers (CPU-Buffer and GPU-Buffer)
//| 3. run a kernel that will change the memory object
//| 4. verify that the object changed.
//|
//|	Pass criteria
//|	-------------
//|
//|	the memory object is recognized in each of the devices.
//|

TEST_F(CRT12_VR_124_127, MemoryObjectVisibilityWriteOnly_vr126){
	cl_uint work_dim = 1;
	size_t global_work_size = 1;
	size_t local_work_size = 1;
	cl_int num = 1;
	DynamicArray<cl_int> buffer1(1);
	DynamicArray<cl_int> buffer2(1);

	memset(buffer1.dynamic_array,0,sizeof(cl_int));
	memset(buffer2.dynamic_array,0,sizeof(cl_int));

	// set up shared context, program and queues with kernel1
	ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueues(ocl_descriptor, "simple_kernels.cl"));

	// create the buffer objects
	ASSERT_NO_FATAL_FAILURE(createBuffer(&ocl_descriptor.buffers[0], ocl_descriptor.context, CL_MEM_WRITE_ONLY|CL_MEM_USE_HOST_PTR, sizeof(cl_int), buffer1.dynamic_array));
	ASSERT_NO_FATAL_FAILURE(createBuffer(&ocl_descriptor.buffers[1],ocl_descriptor.context,CL_MEM_WRITE_ONLY|CL_MEM_USE_HOST_PTR,sizeof(cl_int),buffer2.dynamic_array));

	//create kernel and set arguments.
	ASSERT_NO_FATAL_FAILURE(createKernel(ocl_descriptor.kernels, ocl_descriptor.program, "kernel_1"));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], 0, sizeof(cl_mem), (void*)&ocl_descriptor.buffers[0]));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], 1, sizeof(cl_int), &num));

	// enqueue kernel on CPU 
	ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(ocl_descriptor.queues[0], ocl_descriptor.kernels[0], work_dim, 0, &global_work_size, &local_work_size,NULL,NULL,NULL));

	//read from CPU device
	ASSERT_NO_FATAL_FAILURE(enqueueReadBuffer(ocl_descriptor.queues[0],ocl_descriptor.buffers[0],CL_TRUE,0,sizeof(int)*1,buffer1.dynamic_array,NULL,NULL,NULL));

	//validate result
	ASSERT_EQ(num,buffer1.dynamic_array[0]);

	//set arguments for GPU
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0],0,sizeof(cl_mem),(void*)&ocl_descriptor.buffers[1]));

	// enqueue and run kernel on GPU 
	ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(ocl_descriptor.queues[1], ocl_descriptor.kernels[0], work_dim, 0, &global_work_size, &local_work_size,NULL,NULL,NULL));
	clFinish(ocl_descriptor.queues[1]);
	//read from CPU device
	ASSERT_NO_FATAL_FAILURE(enqueueReadBuffer(ocl_descriptor.queues[1],ocl_descriptor.buffers[1],CL_TRUE,0,sizeof(int)*1,buffer2.dynamic_array,NULL,NULL,NULL));
	ASSERT_EQ(num,buffer2.dynamic_array[0]);
}



//|	TEST: CRT12_VR_124_127.SubBufferObjectVisibilityReadWrite_vr127_VR127 (TC-1)
//|
//|	Purpose
//|	-------
//|	
//|	Verify that a buffer object created using the function clCreateBuffer()and the flag CL_MEM_HOST_WRITE_ONLY is visible to all devices in context 
//|
//|	Method
//|	------
//|
//|	1. Create a shared context queues etc
//| 2. Create two buffers (CPU-Buffer and GPU-Buffer)
//| 3. create a sub buffer for each buffer
//| 3. run a kernel that will change the sub buffer
//| 4. verify that the object changed.
//|
//|	Pass criteria
//|	-------------
//|
//|	the memory object is recognized in each of the devices.
//|

TEST_F(CRT12_VR_124_127, SubBufferVisibilityReadWrite_VR127){
	cl_uint work_dim = 1;
	size_t global_work_size = 1;
	size_t local_work_size = 1;
	cl_int num = 1;
	DynamicArray<cl_int> buffer1(2);
	DynamicArray<cl_int> buffer2(2);
	cl_buffer_region region;
	memset(buffer1.dynamic_array,0,sizeof(cl_int));
	memset(buffer2.dynamic_array,0,sizeof(cl_int));

	//initialize cl_buffer_region
	region.origin=0;
	region.size=sizeof(cl_int);

	// set up shared context, program and queues with kernel1
	ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueues(ocl_descriptor, "simple_kernels.cl"));

	// create the buffer objects
	ASSERT_NO_FATAL_FAILURE(createBuffer(&ocl_descriptor.buffers[0], ocl_descriptor.context, CL_MEM_READ_WRITE|CL_MEM_USE_HOST_PTR, sizeof(cl_int)*2, buffer1.dynamic_array));
	ASSERT_NO_FATAL_FAILURE(createBuffer(&ocl_descriptor.buffers[1],ocl_descriptor.context,CL_MEM_READ_WRITE|CL_MEM_USE_HOST_PTR,sizeof(cl_int)*2,buffer2.dynamic_array));

	//create sub buffer object.
	ASSERT_NO_FATAL_FAILURE(createSubBuffer(&ocl_descriptor.in_common_sub_buffer,ocl_descriptor.buffers[0],CL_MEM_READ_WRITE,CL_BUFFER_CREATE_TYPE_REGION,&region));
	
	//create kernel and set arguments.
	ASSERT_NO_FATAL_FAILURE(createKernel(ocl_descriptor.kernels, ocl_descriptor.program, "kernel_1"));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], 0, sizeof(cl_mem), (void*)&ocl_descriptor.in_common_sub_buffer));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], 1, sizeof(cl_int), &num));

	// enqueue kernel on CPU 
	ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(ocl_descriptor.queues[0], ocl_descriptor.kernels[0], work_dim, 0, &global_work_size, &local_work_size,NULL,NULL,NULL));

	//read from CPU device
	ASSERT_NO_FATAL_FAILURE(enqueueReadBuffer(ocl_descriptor.queues[0],ocl_descriptor.buffers[0],CL_TRUE,0,sizeof(int)*1,buffer1.dynamic_array,NULL,NULL,NULL));

	//validate result
	ASSERT_EQ(num,buffer1.dynamic_array[0]);

	//initialize parameters for GPU
	ASSERT_NO_FATAL_FAILURE(createSubBuffer(&ocl_descriptor.in_common_sub_buffer,ocl_descriptor.buffers[1],CL_MEM_READ_WRITE,CL_BUFFER_CREATE_TYPE_REGION,&region));
	//ASSERT_NO_FATAL_FAILURE(createSubBuffer(&ocl_descriptor.in_common_sub_buffer,ocl_descriptor.buffers[1],CL_MEM_READ_WRITE|CL_MEM_USE_HOST_PTR,CL_BUFFER_CREATE_TYPE_REGION,&region));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], 0, sizeof(cl_mem), (void*)&ocl_descriptor.in_common_sub_buffer));

	//set arguments for GPU
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0],0,sizeof(cl_mem),(void*)&ocl_descriptor.in_common_sub_buffer));

	// enqueue kernel on GPU 
	ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(ocl_descriptor.queues[1], ocl_descriptor.kernels[0], work_dim, 0, &global_work_size, &local_work_size,NULL,NULL,NULL));

	//read from GPU device
	ASSERT_NO_FATAL_FAILURE(enqueueReadBuffer(ocl_descriptor.queues[1],ocl_descriptor.buffers[1],CL_TRUE,0,sizeof(int)*1,buffer2.dynamic_array,NULL,NULL,NULL));
	ASSERT_EQ(num,buffer2.dynamic_array[0]);
}


//|	TEST: CRT12_VR_124_127.SubBufferVisibilityReadOnly_VR127 (TC-2)
//|
//|	Purpose
//|	-------
//|	
//|	Verify that a buffer object created using the function clCreateBuffer()and the flag CL_MEM_HOST_WRITE_ONLY is visible to all devices in context 
//|
//|	Method
//|	------
//|
//|	1. Create a shared context queues etc
//| 2. Create three buffers (CPU-Buffer, GPU-Buffer, and a return result buffer)
//| 3. run a kernel that will change the result buffer according to the input buffer 
//| 4. verify that the object changed.
//|
//|	Pass criteria
//|	-------------
//|
//|	the memory object is recognized in each of the devices.
//|

TEST_F(CRT12_VR_124_127, SubBufferVisibilityReadOnly_VR127){
	cl_uint work_dim = 1;
	size_t global_work_size = 1;
	size_t local_work_size = 1;
	cl_int num = 1;
	DynamicArray<cl_int> buffer1(2);
	DynamicArray<cl_int> buffer2(2);
	cl_buffer_region region;
	memset(buffer1.dynamic_array,0,sizeof(cl_int));
	memset(buffer2.dynamic_array,0,sizeof(cl_int));

	//initialize cl_buffer_region
	region.origin=0;
	region.size=sizeof(cl_int);

	// set up shared context, program and queues with kernel1
	ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueues(ocl_descriptor, "simple_kernels.cl"));

	// create the buffer objects
	ASSERT_NO_FATAL_FAILURE(createBuffer(&ocl_descriptor.buffers[0], ocl_descriptor.context, CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR, sizeof(cl_int)*2, buffer1.dynamic_array));
	ASSERT_NO_FATAL_FAILURE(createBuffer(&ocl_descriptor.buffers[1],ocl_descriptor.context,CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR,sizeof(cl_int)*2,buffer2.dynamic_array));

	//create sub buffer object.
	ASSERT_NO_FATAL_FAILURE(createSubBuffer(&ocl_descriptor.in_common_sub_buffer,ocl_descriptor.buffers[0],CL_MEM_READ_ONLY,CL_BUFFER_CREATE_TYPE_REGION,&region));
	
	//create kernel and set arguments.
	ASSERT_NO_FATAL_FAILURE(createKernel(ocl_descriptor.kernels, ocl_descriptor.program, "kernel_1"));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], 0, sizeof(cl_mem), (void*)&ocl_descriptor.in_common_sub_buffer));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], 1, sizeof(cl_int), &num));

	// enqueue kernel on CPU 
	ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(ocl_descriptor.queues[0], ocl_descriptor.kernels[0], work_dim, 0, &global_work_size, &local_work_size,NULL,NULL,NULL));

	//read from CPU device
	ASSERT_NO_FATAL_FAILURE(enqueueReadBuffer(ocl_descriptor.queues[0],ocl_descriptor.buffers[0],CL_TRUE,0,sizeof(int)*1,buffer1.dynamic_array,NULL,NULL,NULL));

	//validate result
	ASSERT_EQ(num,buffer1.dynamic_array[0]);

	//initialize parameters for GPU
	ASSERT_NO_FATAL_FAILURE(createSubBuffer(&ocl_descriptor.in_common_sub_buffer,ocl_descriptor.buffers[1],CL_MEM_READ_ONLY,CL_BUFFER_CREATE_TYPE_REGION,&region));
	//ASSERT_NO_FATAL_FAILURE(createSubBuffer(&ocl_descriptor.in_common_sub_buffer,ocl_descriptor.buffers[1],CL_MEM_READ_WRITE|CL_MEM_USE_HOST_PTR,CL_BUFFER_CREATE_TYPE_REGION,&region));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], 0, sizeof(cl_mem), (void*)&ocl_descriptor.in_common_sub_buffer));

	//set arguments for GPU
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0],0,sizeof(cl_mem),(void*)&ocl_descriptor.in_common_sub_buffer));

	// enqueue kernel on GPU 
	ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(ocl_descriptor.queues[1], ocl_descriptor.kernels[0], work_dim, 0, &global_work_size, &local_work_size,NULL,NULL,NULL));

	//read from GPU device
	ASSERT_NO_FATAL_FAILURE(enqueueReadBuffer(ocl_descriptor.queues[1],ocl_descriptor.buffers[1],CL_TRUE,0,sizeof(int)*1,buffer2.dynamic_array,NULL,NULL,NULL));
	ASSERT_EQ(num,buffer2.dynamic_array[0]);
}


//|	TEST: CRT12_VR_124_127.SubBufferVisibilityWriteOnly (TC-3)
//|
//|	Purpose
//|	-------
//|	
//|	Verify that a buffer object created using the function clCreateBuffer()and the flag CL_MEM_HOST_WRITE_ONLY is visible to all devices in context 
//|
//|	Method
//|	------
//|
//|	1. Create a shared context queues etc
//| 2. Create two buffers (CPU-Buffer and GPU-Buffer)
//| 3. run a kernel that will change the memory object
//| 4. verify that the object changed.
//|
//|	Pass criteria
//|	-------------
//|
//|	the memory object is recognized in each of the devices.
//|

TEST_F(CRT12_VR_124_127, SubBufferVisibilityWriteOnly_VR127){
	cl_uint work_dim = 1;
	size_t global_work_size = 1;
	size_t local_work_size = 1;
	cl_int num = 1;
	cl_buffer_region region;
	DynamicArray<cl_int> buffer1(1);
	DynamicArray<cl_int> buffer2(1);

	memset(buffer1.dynamic_array,0,sizeof(cl_int));
	memset(buffer2.dynamic_array,0,sizeof(cl_int));

	//initialize cl_buffer_region
	region.origin=0;
	region.size=sizeof(cl_int);

	// set up shared context, program and queues with kernel1
	ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueues(ocl_descriptor, "simple_kernels.cl"));

	// create the buffer objects
	ASSERT_NO_FATAL_FAILURE(createBuffer(&ocl_descriptor.buffers[0], ocl_descriptor.context, CL_MEM_WRITE_ONLY|CL_MEM_USE_HOST_PTR, sizeof(cl_int)*2, buffer1.dynamic_array));
	ASSERT_NO_FATAL_FAILURE(createBuffer(&ocl_descriptor.buffers[1],ocl_descriptor.context,CL_MEM_WRITE_ONLY|CL_MEM_USE_HOST_PTR,sizeof(cl_int)*2,buffer2.dynamic_array));

	//create kernel and set arguments.
	ASSERT_NO_FATAL_FAILURE(createKernel(ocl_descriptor.kernels, ocl_descriptor.program, "kernel_1"));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], 0, sizeof(cl_mem), (void*)&ocl_descriptor.buffers[0]));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], 1, sizeof(cl_int), &num));

	// enqueue kernel on CPU 
	ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(ocl_descriptor.queues[0], ocl_descriptor.kernels[0], work_dim, 0, &global_work_size, &local_work_size,NULL,NULL,NULL));

	//read from CPU device
	ASSERT_NO_FATAL_FAILURE(enqueueReadBuffer(ocl_descriptor.queues[0],ocl_descriptor.buffers[0],CL_TRUE,0,sizeof(int)*1,buffer1.dynamic_array,NULL,NULL,NULL));

	//validate result
	ASSERT_EQ(num,buffer1.dynamic_array[0]);

	//initialize parameters for GPU
	ASSERT_NO_FATAL_FAILURE(createSubBuffer(&ocl_descriptor.in_common_sub_buffer,ocl_descriptor.buffers[1],CL_MEM_WRITE_ONLY,CL_BUFFER_CREATE_TYPE_REGION,&region));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], 0, sizeof(cl_mem), (void*)&ocl_descriptor.in_common_sub_buffer));

	//set arguments for GPU
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0],0,sizeof(cl_mem),(void*)&ocl_descriptor.in_common_sub_buffer));

	// enqueue kernel on GPU 
	ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(ocl_descriptor.queues[1], ocl_descriptor.kernels[0], work_dim, 0, &global_work_size, &local_work_size,NULL,NULL,NULL));

	//read from GPU device
	ASSERT_NO_FATAL_FAILURE(enqueueReadBuffer(ocl_descriptor.queues[1],ocl_descriptor.buffers[1],CL_TRUE,0,sizeof(int)*1,buffer2.dynamic_array,NULL,NULL,NULL));
	ASSERT_EQ(num,buffer2.dynamic_array[0]);
}
