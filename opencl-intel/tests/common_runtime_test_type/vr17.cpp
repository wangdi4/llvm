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

#include <sstream>
#include "common_runtime_tests.h"

class VR17: public CommonRuntime{};

//|	TEST: CommonRuntime.SharedTask (TC-86)
//|
//|	Purpose
//|	-------
//|	
//|	Verify the ability to queue commands to execute a shared task on both CPU and GPU devices
//|	
//|	Method
//|	------
//|
//|	1.	Create a kernel object from a shared program. The kernel is executed using a single work-item
//|	2.	Queue a Task command to a commands-queue related to the CPU device
//|	3.	Queue a Task command to a commands-queue related to the GPU device.
//|	
//|	Pass criteria
//|	-------------
//|
//|	The tasks (kernels) execution commands should be successfully queued.
//|

TEST_F(VR17, SharedTask)
{
	// set up OpenCL context, program and queues
	ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueues(ocl_descriptor, "shared_kernel.cl"));

	// create 2 shared kernels
	ASSERT_NO_FATAL_FAILURE(createKernel(&ocl_descriptor.kernels[0] , ocl_descriptor.program, "shared_kernel"));

	int arraySize = (size_t)128;
	DynamicArray<int> input_array(arraySize);
	// create shared buffer
	ASSERT_NO_FATAL_FAILURE(createBuffer(&ocl_descriptor.in_common_buffer, ocl_descriptor.context, 
		CL_MEM_READ_WRITE|CL_MEM_USE_HOST_PTR, sizeof(int)*input_array.dynamic_array_size, input_array.dynamic_array));

	// set up manual destruction of input_array
	ASSERT_NO_FATAL_FAILURE(setDeleteArrayOnCallback(ocl_descriptor.in_common_buffer, input_array));

	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], 0, sizeof(cl_mem), &ocl_descriptor.in_common_buffer));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], 1, sizeof(int), &input_array.dynamic_array_size));

	// create user event
	cl_event user_event = 0;
	cl_event device_done_event[] = {0,0};
	ASSERT_NO_FATAL_FAILURE(createUserEvent(&user_event, ocl_descriptor.context));

	// enqueue kernels
	for(int i=0; i<2; ++i)
	{
		ASSERT_NO_FATAL_FAILURE(enqueueTask(ocl_descriptor.queues[i], ocl_descriptor.kernels[0], 1, &user_event, &device_done_event[i]));
	}

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
	ASSERT_NO_FATAL_FAILURE(waitForEvents(2, device_done_event));
}
