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

class VR18_Fission: public FissionWrapper{};

static void CL_CALLBACK test_native_kernel_fn( void *userData )
{
	struct arg_struct {
		cl_int * source;
		cl_int count;
		cl_int multBy;
	} *args = (arg_struct *)userData;
	
	for( cl_int i = 0; i < args->count; i++ )
	{
		args->source[ i ] *= args->multBy;
	}
}

//|	TEST: VR18_Fission.Native (TC-125)
//|
//|	Purpose
//|	-------
//|	
//|	Verify the ability to queue commands to execute native function on shared memory objects by CPU sub device
//|
//|	Method
//|	------
//|
//|	1. Use the root CPU device to create an array of 2 CPU subdevices
//|	2. Queue a command to execute a native function to command queue related to a CPU subdevice
//| 3. Try to queue a command to execute a native function to command queue related to the GPU root device
//|
//|	Pass criteria
//|	-------------
//|
//|	Validate that the native function on CPU device is executed successfully, and on GPU returns CL_INVALID_OPERATION
//|
TEST_F(VR18_Fission, Native)
{
	// initialize array
	size_t arraySize = 128;
	DynamicArray<cl_int> input_array(arraySize);
	DynamicArray<cl_int> output_array(arraySize);
	DynamicArray<cl_int> validation_array(arraySize);
	int multBy = 5;
	validation_array.multBy(multBy);

	// set up shared context, program and queues
	// get pltfrom and device ids
	ASSERT_NO_FATAL_FAILURE(createAndMergeWithGPU(ocl_descriptor));

	// create context
	ASSERT_NO_FATAL_FAILURE(createContext(&ocl_descriptor.context, 0, 2, ocl_descriptor.devices, NULL, NULL));

	for(int i=0; i<2; ++i)
	{
		// create queue
		ASSERT_NO_FATAL_FAILURE(createCommandQueue(&ocl_descriptor.queues[i], ocl_descriptor.context, ocl_descriptor.devices[i], 0));
	}

	struct arg_struct
	{
		cl_mem inputStream;
		cl_int count;
		cl_int multBy;
	} args;
				
	// create buffers
	ASSERT_NO_FATAL_FAILURE(createBuffer(&ocl_descriptor.in_common_buffer, ocl_descriptor.context, 
		CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR, sizeof(int)*input_array.dynamic_array_size, input_array.dynamic_array));

	// Set up the arrays to call with
	args.inputStream = ocl_descriptor.in_common_buffer;
	args.count = arraySize;
	args.multBy = multBy;
	
	void * memLocs[ 1 ] = { &args.inputStream };
		
	// Run the kernel on subdevice
	ASSERT_NO_FATAL_FAILURE(enqueueNativeKernel(ocl_descriptor.queues[0], test_native_kernel_fn, 
									  &args, sizeof( args ),
									  1, &ocl_descriptor.in_common_buffer,
									  (const void **)memLocs, 
									  0, NULL, NULL));

	// Finish and wait for the kernel to complete
	ASSERT_NO_FATAL_FAILURE(finish(ocl_descriptor.queues[0]));

	ASSERT_NO_FATAL_FAILURE(enqueueReadBuffer(ocl_descriptor.queues[0], ocl_descriptor.in_common_buffer, 
			 CL_TRUE, 0, sizeof(int)*output_array.dynamic_array_size, 
			 output_array.dynamic_array, 
			 0, NULL, NULL));

	// validate execution correctness
	ASSERT_NO_FATAL_FAILURE(validation_array.compareArray(output_array.dynamic_array, arraySize));

	// Run the kernel on GPU
	ASSERT_EQ(CL_INVALID_OPERATION, clEnqueueNativeKernel(ocl_descriptor.queues[1], test_native_kernel_fn, 
									  &args, sizeof( args ),
									  1, &ocl_descriptor.in_common_buffer,
									  (const void **)memLocs, 
									  0, NULL, NULL)) << "clEnqueueNativeKernel did not fail on GPU";
}

