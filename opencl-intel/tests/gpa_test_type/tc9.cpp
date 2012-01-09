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


//|	enqueueAll
//|	1. Create an OpenCL queue with property queue_properties
//|	2. Create an OpneCL buffer
//|	3. Enqueue write buffer 100 times
//|	5. Wait for all commands to finish (clFinish)
//|	
void enqueueAll(OpenCLDescriptor& ocl_descriptor, cl_command_queue_properties queue_properties)
{
	// create and initialize array
	int arraySize = 2;
	int value = 5;
	DynamicArray<cl_int> input_array(arraySize);
	
	// num_commands - number of write buffer commands to be enqueued
	int num_commands = 100;

	// Arrays and OpenCL object for in order and out of order queues
	// first 100 queues are in-order, the other 100 are out-of-order
	DynamicArray<cl_int> output_array(arraySize);

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

	// create buffer
	ASSERT_NO_FATAL_FAILURE(createBuffer(&ocl_descriptor.buffer, 
		ocl_descriptor.context, CL_MEM_READ_WRITE|CL_MEM_ALLOC_HOST_PTR, 
		sizeof(cl_int)*arraySize, NULL));	
	
	for(int i=0; i<num_commands; ++i)
	{
		// enqueue in order commands to be executed with dependency on user event
		// write from input array to buffer
		ASSERT_NO_FATAL_FAILURE(enqueueWriteBuffer(ocl_descriptor.queue, 
			ocl_descriptor.buffer, CL_FALSE, 0,sizeof(cl_int)*arraySize, 
			input_array.dynamic_array, 
			0, NULL, NULL));
	}

	// wait until all enqueued commands are finished
	clFinish(ocl_descriptor.queue);
}

//|	TEST: GPA.TC9_ManyOpenCLComamndsOnInOrderQueue (TC-9)
//|
//|	Purpose
//|	-------
//|	
//|	Test the ability to support many OpenCL commands
//|
//|	Method
//|	------
//|	1. Create in-order queue
//|	2. Create an OpneCL buffer
//|	3. Enqueue write buffer 100 times
//|	5. Wait for all queued commands to finish (clFinish)
//|	
//|	Pass criteria
//|	-------------
//|
//|	
//|
TEST_F(GPA, TC9_ManyOpenCLComamndsOnInOrderQueue)
{
	enqueueAll(ocl_descriptor, CL_QUEUE_PROFILING_ENABLE);
}

//|	TEST: GPA.TC9_ManyOpenCLComamndsOnOutOfOrderQueue (TC-9)
//|
//|	Purpose
//|	-------
//|	
//|	Test the ability to support many OpenCL commands
//|
//|	Method
//|	------
//|	1. Create out-of-order queue
//|	2. Create an OpneCL buffer
//|	3. Enqueue write buffer 100 times
//|	5. Wait for all queued commands to finish (clFinish)
//|	
//|	Pass criteria
//|	-------------
//|
//|	
//|
TEST_F(GPA, TC9_ManyOpenCLComamndsOnOutOfOrderQueue)
{
	enqueueAll(ocl_descriptor, CL_QUEUE_PROFILING_ENABLE|CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE);
}

