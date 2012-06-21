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

class VR4: public CommonRuntime{};

//|	
//|	NEED TO UNCOMMENT CODE IN THE LAST 3 FUNCTIONS IN ORDER TO RUN THESE TEST
//|	WERE NOT ABLE TO TEST THIS ON AMD
//|

//|	initExpectedSharedExtenstions - helper function - add extensions to be supported by both CPU and GPU here
//|	Currently there are no extensions common runtime on AMD supports that appear in the validation requirenemnts paper
//|
static std::vector<std::string> initExpectedSharedExtenstions()
{
	std::vector<std::string> vec;
	// ADD EXTENSIONS HERE
	vec.push_back("cl_khr_global_int32_base_atomics");
	vec.push_back("cl_khr_global_int32_extended_atomics");
	vec.push_back("cl_khr_local_int32_base_atomics");
	vec.push_back("cl_khr_local_int32_extended_atomics");
	vec.push_back("cl_khr_byte_addressable_store");
	vec.push_back("cl_khr_gl_sharing");	
        
        /// ToDo: 
        ///     Disabled for now; enable the following 
        ///     later when supported by the underlying
        ///     platforms.
	
    // vec.push_back("cl_intel_printf");
	// vec.push_back("cl_intel_dx9_sharing");
	return vec;
};
//|	initExpectedCPUExtenstions - helper function - add extensions to be supported by CPU here
//|
static std::vector<std::string> initExpectedCPUExtenstions()
{
	std::vector<std::string> vec;
	// ADD EXTENSIONS HERE
	vec.push_back("cl_khr_global_int32_base_atomics");
	vec.push_back("cl_khr_global_int32_extended_atomics");
	vec.push_back("cl_khr_local_int32_base_atomics");
	vec.push_back("cl_khr_local_int32_extended_atomics");
	vec.push_back("cl_khr_fp64");
	vec.push_back("cl_khr_byte_addressable_store");
	vec.push_back("cl_khr_gl_sharing");		
	vec.push_back("cl_ext_device_fission");
	vec.push_back("cl_intel_printf");
	vec.push_back("cl_intel_dx9_media_sharing");

        /// ToDo: 
        ///     Disabled for now; enable the following 
        ///     later when supported by the underlying
        ///     platforms.
    //vec.push_back("cl_ext_immediate_execution");

	return vec;
};
//|	initExpectedGPUExtenstions - helper function - add extensions to be supported by GPU here
//|
static std::vector<std::string> initExpectedGPUExtenstions()
{
	std::vector<std::string> vec;
	// ADD EXTENSIONS HERE
    vec.push_back("cl_khr_icd");
	vec.push_back("cl_khr_global_int32_base_atomics");
	vec.push_back("cl_khr_global_int32_extended_atomics");
	vec.push_back("cl_khr_local_int32_base_atomics");
	vec.push_back("cl_khr_local_int32_extended_atomics");
	vec.push_back("cl_khr_byte_addressable_store");
	vec.push_back("cl_khr_gl_sharing");
	vec.push_back("cl_intel_dx9_media_sharing");
	vec.push_back("cl_khr_3d_image_writes");
	vec.push_back("cl_khr_d3d10_sharing");
	
        /// ToDo: 
        ///     Disabled for now; enable the following 
        ///     later when supported by the underlying
        ///     platforms.
    //vec.push_back("cl_khr_gl_event");
	//vec.push_back("cl_intel_printf");
    //vec.push_back("cl_intel_packed_yuv");
	return vec;
};

//|	TEST: VR4.SharedExtensions (TC-7)
//|
//|	Purpose
//|	-------
//|	
//|	Verify that the platfrom returns all extensions that should be supported for both CPU and GPU
//|
//|	Method
//|	------
//|
//|	1. Call clGetPlatformInfo() with CL_PLATFORM_EXTENSIONS and retrieve all available extensions of this platform
//|	2. Compare all exntensions recieved from clGetPlatformInfo to expected extensions that both devices should support 
//|	
//|	Pass criteria
//|	-------------
//|
//|	All extensions that both CPU and GPU should support are returned by getPlatformInfo with CL_PLATFORM_EXTENSIONS
//|
TEST_F(VR4, SharedExtensions){
	// set up platform, expect return of a single platform
	cl_uint num_entries = 1;
	cl_uint num_platforms = 0;
	cl_uint num_devices = 0;
	ASSERT_NO_FATAL_FAILURE(getPlatformIDs(num_entries, ocl_descriptor.platforms, &num_platforms));
	
	// get platform exntensions shared for CPU and GPU
	size_t param_value_size = sizeof(char) * 1000;
	char* param_value = new char[1000];
	ASSERT_NO_FATAL_FAILURE(getPlatformInfo(ocl_descriptor.platforms[0], CL_PLATFORM_EXTENSIONS, param_value_size, param_value));

	// get all expected extensions
	std::vector<std::string> expected = initExpectedSharedExtenstions();
	// get string vector in which each element is a single extension from extensions found earlier for this platform
	std::vector<std::string> foundExt = getAllStrings(param_value);
	delete[] param_value;
	// check that all required extensions were found

	// UNCOMMENT THIS
	assertVectorInclusion(expected, foundExt);
}

//|	TEST: VR4.CPUExtensions (TC-8)
//|
//|	Purpose
//|	-------
//|	
//|	Verify that the CPU device returns all extensions that should be supported by CPU
//|
//|	Method
//|	------
//|
//|	1. Call clGetDeviceInfo() with CL_DEVICE_EXTENSIONS and retrieve all available extensions of this CPU device
//|	2. Compare all exntensions recieved from clGetDeviceInfo to expected extensions that CPU should support 
//|	
//|	Pass criteria
//|	-------------
//|
//|	All extensions that CPU returns all required extensions
//|
TEST_F(VR4, CPUExtensions){
	// get CPU device
	ASSERT_NO_FATAL_FAILURE(getCPUDevice(&ocl_descriptor.platforms[0], &ocl_descriptor.devices[0]));

	size_t param_value_size = sizeof(char) * 1000;
	char* param_value = new char[1000];
	// get extensions supported by this CPU device
	ASSERT_NO_FATAL_FAILURE(getDeviceInfo(ocl_descriptor.devices[0],  CL_DEVICE_EXTENSIONS, param_value_size, param_value));

	// get all expected CPU extensions
	std::vector<std::string> expected = initExpectedCPUExtenstions();
	// get string vector in which each element is a single extension from extensions found earlier for this device
	std::vector<std::string> foundExt = getAllStrings(param_value);

	delete[] param_value;
	// check that all required extensions were found

	// UNCOMMENT THIS
	assertVectorInclusion(expected, foundExt);
}

//|	TEST: VR4.GPUExtensions (TC-9)
//|
//|	Purpose
//|	-------
//|	
//|	Verify that the GPU device returns all extensions that should be supported by GPU
//|
//|	Method
//|	------
//|
//|	1. Call clGetDeviceInfo() with CL_DEVICE_EXTENSIONS and retrieve all available extensions of this GPU device
//|	2. Compare all exntensions recieved from clGetDeviceInfo to expected extensions that GPU should support 
//|	
//|	Pass criteria
//|	-------------
//|
//|	All extensions that CPU returns all required extensions
//|
TEST_F(VR4, GPUExtensions){
	// get CPU device
	ASSERT_NO_FATAL_FAILURE(getGPUDevice(&ocl_descriptor.platforms[0], &ocl_descriptor.devices[0]));

	size_t param_value_size = sizeof(char) * 1000;
	char* param_value = new char[1000];
	// get extensions supported by this CPU device
	ASSERT_NO_FATAL_FAILURE(getDeviceInfo(ocl_descriptor.devices[0],  CL_DEVICE_EXTENSIONS, param_value_size, param_value));

	// get all expected extensions
	std::vector<std::string> expected = initExpectedGPUExtenstions();
	// get string vector in which each element is a single extension from extensions found earlier for this device
	std::vector<std::string> foundExt = getAllStrings(param_value);
	delete[] param_value;
	// check that all required extensions were found
	
	// UNCOMMENT THIS
	assertVectorInclusion(expected, foundExt);
}

//TODO: Uncomment the following code when this extension is supported

/*
//|	TEST: VR4.LogocalOperatorsForFloatsOnCPU (TC-24)
//|
//|	Purpose
//|	-------
//|	
//|	Verify that the extension logical operators for floating-point types is supported by CPU device
//|
//|	Method
//|	------
//|
//|	Create, compile and execute a CPU kernel including logical operators between floating-point variables (float)
//|		
//|	Pass criteria
//|	-------------
//|
//|	The kernel executes successfully
//|
TEST_F(VR4, LogocalOperatorsForFloatsOnCPU)
{
	cl_platform_id platform = 0;
	cl_device_id device = 0;

	// get pltfrom and device id
	ASSERT_NO_FATAL_FAILURE(getCPUDevice(&platform, &device));

	// create context
	cl_context_properties properties[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0};
	ASSERT_NO_FATAL_FAILURE(createContext(&ocl_descriptor.context, properties, 1, &device, NULL, NULL));

	//	create and build program
	ASSERT_NO_FATAL_FAILURE(createAndBuildProgramWithSource("float_logic.cl", &ocl_descriptor.program, ocl_descriptor.context, 1, &device, 
		NULL, NULL, NULL));

	// create kernel
	ASSERT_NO_FATAL_FAILURE(createKernel(&ocl_descriptor.kernels[0] , ocl_descriptor.program, "float_logic"));

	// create queue
	ASSERT_NO_FATAL_FAILURE(createCommandQueue(&ocl_descriptor.queues[0], ocl_descriptor.context, device, 0));

	// input array
	int arraySize = 4;
	DynamicArray<float> input_array(arraySize);
	// create shared buffer
	ASSERT_NO_FATAL_FAILURE(createBuffer(&ocl_descriptor.in_common_buffer, ocl_descriptor.context, CL_MEM_READ_WRITE|CL_MEM_USE_HOST_PTR, sizeof(float)*input_array.dynamic_array_size, input_array.dynamic_array));

	// set kernel argumanets
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], 0, sizeof(cl_mem), &ocl_descriptor.in_common_buffer));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], 1, sizeof(int), &input_array.dynamic_array_size));	

	// enqueue both kernels with required dependency (CPU then GPU)
	cl_uint work_dim = 1;
	size_t global_work_size = 1;
	ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(ocl_descriptor.queues[0], ocl_descriptor.kernels[0], 1, NULL, &global_work_size, NULL, 0, NULL, NULL));	
}

//|	TEST: VR4.LogocalOperatorsForFloatsOnGPU (TC-25)
//|
//|	Purpose
//|	-------
//|	
//|	Verify that the extension logical operators for floating-point types is not supported by GPU device
//|
//|	Method
//|	------
//|
//|	Create and compile a CPU kernel including logical operators between floating-point variables (float)
//|		
//|	Pass criteria
//|	-------------
//|
//|	Building OpenCL program should fail
//|
TEST_F(VR4, LogocalOperatorsForFloatsOnGPU)
{
	cl_platform_id platform = 0;
	cl_device_id device = 0;

	// get pltfrom and device id
	ASSERT_NO_FATAL_FAILURE(getCPUDevice(&platform, &device));

	// create context
	cl_context_properties properties[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0};
	ASSERT_NO_FATAL_FAILURE(createContext(&ocl_descriptor.context, properties, 1, &device, NULL, NULL));

	const char* kernelSource = NULL;
	// read kernels file
	ASSERT_NO_FATAL_FAILURE(fileToBuffer(&kernelSource, "float_logic.cl"));
	// create program
	ASSERT_NO_FATAL_FAILURE(createProgramWithSource(&ocl_descriptor.program, ocl_descriptor.context,	
		1, &kernelSource, NULL));
	// build program, expect failure
	ASSERT_NE(CL_SUCCESS, clBuildProgram (ocl_descriptor.program, 1, &device, 0, NULL, NULL));
	if(NULL!=kernelSource)
	{
		delete[] kernelSource;
		kernelSource = NULL;
	}
}*/

//TODO: currently GPU does not support printf extension
//When GPU will support this extension should uncomment the following code
/*
//|	TEST: VR4.PrintfOnCPUGPU (TC-26)
//|
//|	Purpose
//|	-------
//|	
//|	Verify that the extension cl_intel_printf is supported by both CPU and GPU devices
//|
//|	Method
//|	------
//|
//|	Create, compile and execute on both CPU and GPU kernel including call to function printf()
//|		
//|	Pass criteria
//|	-------------
//|
//|	The kernel executes successfully on both devices
//|
TEST_F(VR4, PrintfOnCPUGPU)
{
	cl_platform_id platform = 0;
	cl_device_id devices[] = {0,0};

	// get pltfrom and device id
	ASSERT_NO_FATAL_FAILURE(getCPUGPUDevices(&platform, devices));

	// create context
	cl_context_properties properties[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0};
	ASSERT_NO_FATAL_FAILURE(createContext(&ocl_descriptor.context, properties, 2, devices, NULL, NULL));

	//	create and build program
	ASSERT_NO_FATAL_FAILURE(createAndBuildProgramWithSource("printf_kernel.cl", &ocl_descriptor.program, 
		ocl_descriptor.context, 2, devices, NULL, NULL, NULL));

	// create kernel
	ASSERT_NO_FATAL_FAILURE(createKernel(&ocl_descriptor.kernels[0] , ocl_descriptor.program, "printf_kernel"));

	// input array
	int arraySize = 4;
	DynamicArray<cl_int> input_array(arraySize);
	// create shared buffer
	ASSERT_NO_FATAL_FAILURE(createBuffer(&ocl_descriptor.in_common_buffer, ocl_descriptor.context, CL_MEM_READ_WRITE|CL_MEM_USE_HOST_PTR, sizeof(cl_int)*input_array.dynamic_array_size, input_array.dynamic_array));

	// set kernel argumanets
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], 0, sizeof(cl_mem), &ocl_descriptor.in_common_buffer));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], 1, sizeof(int), &input_array.dynamic_array_size));	

	cl_uint work_dim = 1;
	size_t global_work_size = 1;

	for(int i=0; i<2; ++i)
	{
		// create queue
		ASSERT_NO_FATAL_FAILURE(createCommandQueue(&ocl_descriptor.queues[i], ocl_descriptor.context, devices[i], 0));

		// enqueue both kernels with required dependency (CPU then GPU)
		ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(ocl_descriptor.queues[i], ocl_descriptor.kernels[0], 1, NULL, &global_work_size, NULL, 0, NULL, NULL));	
		ASSERT_EQ(CL_SUCCESS, clFinish(ocl_descriptor.queues[i]));
		return;
	}
}
*/
