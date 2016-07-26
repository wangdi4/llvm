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

class VR6: public CommonRuntime{};

//|	TEST: VR6.SharedContextCPUGPU (TC-35)
//|
//|	Purpose
//|	-------
//|	
//|	Verify that it is possible to create common context with both CPU and GPU devices
//|
//|	Method
//|	------
//|
//|	1. Call clCreateContext() with property CL_CONTEXT_PLATFROM, common platfrom and devices CPU and GPU
//|	2. Call getContextInfo on the returned context
//|	3. Validate that the returned context has 2 CL_CONTEXT_NUM_DEVICES
//|	
//|	Pass criteria
//|	-------------
//|
//|	Common context has 2 CL_CONTEXT_NUM_DEVICES
//|
TEST_F(VR6, SharedContextCPUGPU)
{
	// set up platform and CPU and GPU  devices 
	//	CPU is at index 0, GPU is at index 1
	ASSERT_NO_FATAL_FAILURE(getCPUGPUDevices(ocl_descriptor.platforms, ocl_descriptor.devices));	
	
	// set up context
	cl_context_properties properties[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)ocl_descriptor.platforms[0], 0};
	ASSERT_NO_FATAL_FAILURE(createContext(&ocl_descriptor.context, properties, 2, ocl_descriptor.devices, NULL, NULL));

	// query context, obtain num of devices
	cl_uint param_value = 0;
	cl_device_id test_device_id[2];

	ASSERT_NO_FATAL_FAILURE(getContextInfo(&ocl_descriptor.context, CL_CONTEXT_NUM_DEVICES, sizeof(cl_uint), &param_value));
	ASSERT_NO_FATAL_FAILURE(getContextInfo(&ocl_descriptor.context, CL_CONTEXT_DEVICES, sizeof(cl_device_id)*2, test_device_id)); 

	EXPECT_EQ(2, param_value) << "Did not return context for 2 devices";
	if(false == ((test_device_id[0] == ocl_descriptor.devices[0] && test_device_id[1] == ocl_descriptor.devices[1]) || 
		(test_device_id[1] == ocl_descriptor.devices[0] && test_device_id[0] == ocl_descriptor.devices[1]))){
		EXPECT_TRUE(false)  << "Did not return context for 2 devices";
	}
}

//|	TEST: VR6.NotSharedContextsCPUGPU (TC-36)
//|
//|	Purpose
//|	-------
//|	
//|	Verify that it is possible to create non shared contexts for each device (CPU and GPU)
//|
//|	Method
//|	------
//|
//|	1. Call clCreateContext() for each device - for CPU and for GPU
//|	2. Call getContextInfo on each returned context
//|	3. Validate that each returned context has 1 CL_CONTEXT_NUM_DEVICES
//|	
//|	Pass criteria
//|	-------------
//|
//|	Each context has 1 CL_CONTEXT_NUM_DEVICES
//|
TEST_F(VR6, NotSharedContextsCPUGPU)
{
	// set up platform and CPU and GPU  devices 
	// CPU is at index 0, GPU is at index 1
	ASSERT_NO_FATAL_FAILURE(getCPUGPUDevices(ocl_descriptor.platforms, ocl_descriptor.devices));	
	
	// set up contexts for CPU and for GPU
	ASSERT_NO_FATAL_FAILURE(createContext(&ocl_descriptor.cpu_context, NULL, 1, &ocl_descriptor.devices[0], NULL, NULL));
	ASSERT_NO_FATAL_FAILURE(createContext(&ocl_descriptor.gpu_context, NULL, 1, &ocl_descriptor.devices[1], NULL, NULL));

	// query context, obtain num of devices
	cl_uint param_value = 0;
	cl_device_id test_device_id = 0;

	ASSERT_NO_FATAL_FAILURE(getContextInfo(&ocl_descriptor.cpu_context, CL_CONTEXT_NUM_DEVICES, sizeof(cl_uint), &param_value));
	ASSERT_NO_FATAL_FAILURE(getContextInfo(&ocl_descriptor.cpu_context, CL_CONTEXT_DEVICES, sizeof(cl_device_id), &test_device_id)); 
	EXPECT_EQ(1, param_value) << "Did not return context for CPU";
	EXPECT_EQ(ocl_descriptor.devices[0], test_device_id) << "Did not return context for CPU";

	ASSERT_NO_FATAL_FAILURE(getContextInfo(&ocl_descriptor.gpu_context, CL_CONTEXT_NUM_DEVICES, sizeof(cl_uint), &param_value)); 
	ASSERT_NO_FATAL_FAILURE(getContextInfo(&ocl_descriptor.gpu_context, CL_CONTEXT_DEVICES, sizeof(cl_device_id), &test_device_id)); 
	EXPECT_EQ(1, param_value) << "Did not return context for GPU";
	EXPECT_EQ(ocl_descriptor.devices[1], test_device_id) << "Did not return context for GPU";
}