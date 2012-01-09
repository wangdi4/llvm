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

//|	TEST: GPA.TC1_GPAConfigurationFlag (TC-1)
//|
//|	Purpose
//|	-------
//|	
//|	Test the GPA configuration flag
//|
//|	Method
//|	------
//|
//|	Run OpenCL kernel without enabling GPA instrumentation (profiling)
//|	
//|	Pass criteria
//|	-------------
//|
//|	
//|
TEST_F(GPA, TC1_GPAConfigurationFlag)
{	
	int arraySize = 2;
	int value = 5;
	// setup CPU device 
	ASSERT_NO_FATAL_FAILURE(getCPUDevice(&ocl_descriptor.platform, &ocl_descriptor.device));
	
	// create context
	cl_context_properties properties[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)ocl_descriptor.platform, 0};
	ASSERT_NO_FATAL_FAILURE(createContext(&ocl_descriptor.context, properties, 1, &ocl_descriptor.device, NULL, NULL));
		
	// create buffer
	ASSERT_NO_FATAL_FAILURE(createBuffer(&ocl_descriptor.buffer, ocl_descriptor.context, 
		CL_MEM_READ_WRITE|CL_MEM_ALLOC_HOST_PTR, sizeof(cl_int)*arraySize, NULL));

	//	create and build program
	ASSERT_NO_FATAL_FAILURE(createAndBuildProgramWithSource("kernels.cl", &ocl_descriptor.program, 
		ocl_descriptor.context, 1, &ocl_descriptor.device, NULL, NULL, NULL));
		
	// create queue
	ASSERT_NO_FATAL_FAILURE(createCommandQueue(&ocl_descriptor.queue, ocl_descriptor.context, 
		ocl_descriptor.device, CL_QUEUE_PROFILING_ENABLE));
		
	// create kernel
	ASSERT_NO_FATAL_FAILURE(createKernel(&ocl_descriptor.kernel , ocl_descriptor.program, "kernel_a"));

	// set kernel arguments
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernel, 0, sizeof(cl_mem), &ocl_descriptor.buffer));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernel, 1, sizeof(int), &value));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernel, 2, sizeof(int), &arraySize));
		
	// work dimension 
	cl_uint work_dim = 1;
	// global work size
	size_t global_work_size[] = {1};
	cl_event device_done_event = 0;

	// enqueue kernel
	ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(ocl_descriptor.queue, ocl_descriptor.kernel, 
		1, NULL, &global_work_size[0], NULL, 0, NULL, &device_done_event));
	
	// wait for completion of kernel
	ASSERT_NO_FATAL_FAILURE(waitForEvents(1, &device_done_event));
}
