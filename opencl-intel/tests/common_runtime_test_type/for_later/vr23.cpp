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

#include "d3d9utils.h"
#ifdef WIN32

// TODO: this is a skeleton of basic test scenarios in Dx9. Tests requirenemnts were under
// modification when this code was written. 
// Need to uncomment and update the tests accordningly to future requirenemnts.


//|	TEST: CommonRuntime.Dx9WithCPUGPU 
//|
//|	Purpose
//|	-------
//|	
//|	
//|
//|	Method
//|	------
//|
//|	
//|	
//|	Pass criteria
//|	-------------
//|
//|	
//|
TEST_F(CommonRuntime, Dx9WithCPUGPU)
{
	clGetDeviceIDsFromDX9INTEL_fn clGetDeviceIDsFromD3D9Intel = NULL;
	clGetDeviceIDsFromD3D9Intel = (clGetDeviceIDsFromDX9INTEL_fn)clGetExtensionFunctionAddress("clGetDeviceIDsFromDX9INTEL");
	ASSERT_TRUE(NULL != clGetDeviceIDsFromD3D9Intel) << "clGetDeviceIDsFromD3D9Intel was returned as NULL from clGetExtensionFunctionAddress";

	//create D3D9 device
	CD3D9Wrapper d3d9Wrapper;
	ASSERT_TRUE(d3d9Wrapper.Init()) << "D3D9 init failed";

	cl_uint devicesNum = 0;
	// get platform
	cl_uint num_entries = 1;
	cl_uint num_platforms = 0;
	cl_uint num_devices = 0;
	ASSERT_NO_FATAL_FAILURE(getPlatformIDs(num_entries, ocl_descriptor.platforms, NULL));

	//	CPU is at index 0, GPU is at index 1
	ASSERT_NO_FATAL_FAILURE(getCPUGPUDevices(ocl_descriptor.platforms, ocl_descriptor.devices));

	//create CL context from D3D9 device
	cl_context_properties properties[] = {
		CL_CONTEXT_PLATFORM, (cl_context_properties) ocl_descriptor.platforms[0],
		CL_CONTEXT_D3D9_DEVICE_INTEL, (cl_context_properties) d3d9Wrapper.Device(),	0};
	ASSERT_NO_FATAL_FAILURE(createContext(&ocl_descriptor.context, properties, 2, ocl_descriptor.devices, NULL, NULL));

	// create corresponding devices
	ASSERT_EQ(CL_SUCCESS, clGetDeviceIDsFromD3D9Intel(ocl_descriptor.platforms[0], CL_D3D9_DEVICE_INTEL, d3d9Wrapper.Device(), CL_ALL_DEVICES_FOR_DX9_INTEL, 0, 0, &devicesNum)) << "clGetDeviceIDsFromD3D9Intel failed";
    
	ASSERT_EQ(2, devicesNum) << "Number of corresponding devices is " << devicesNum;
	cl_device_id devices[] = {0,0};
	ASSERT_EQ(CL_SUCCESS, clGetDeviceIDsFromD3D9Intel(ocl_descriptor.platforms[0], CL_D3D9_DEVICE_INTEL, d3d9Wrapper.Device(), CL_ALL_DEVICES_FOR_DX9_INTEL, devicesNum, &devices[0], 0)) << "clGetDeviceIDsFromD3D9Intel failed";

	// validate that both CPU and GPU devices were returned
	ASSERT_TRUE(devices[0] == ocl_descriptor.devices[0] || devices[1] == ocl_descriptor.devices[0]) << "CPU device is not corresponding to DX9 device";
	ASSERT_TRUE(devices[0] == ocl_descriptor.devices[1] || devices[1] == ocl_descriptor.devices[1]) << "GPU device is not corresponding to DX9 device";
}

//|	TEST: CommonRuntime.Dx9WithCPUOnly 
//|
//|	Purpose
//|	-------
//|	
//|	
//|
//|	Method
//|	------
//|
//|	
//|	
//|	Pass criteria
//|	-------------
//|
//|	
//|
TEST_F(CommonRuntime, Dx9WithCPUOnly)
{
	clGetDeviceIDsFromDX9INTEL_fn clGetDeviceIDsFromD3D9Intel = NULL;
	clGetDeviceIDsFromD3D9Intel = (clGetDeviceIDsFromDX9INTEL_fn)clGetExtensionFunctionAddress("clGetDeviceIDsFromDX9INTEL");
	ASSERT_TRUE(NULL != clGetDeviceIDsFromD3D9Intel) << "clGetDeviceIDsFromD3D9Intel was returned as NULL from clGetExtensionFunctionAddress";

	//create D3D9 device
	CD3D9Wrapper d3d9Wrapper;
	ASSERT_TRUE(d3d9Wrapper.Init()) << "D3D9 init failed";

	cl_uint devicesNum = 0;
	// get platform
	cl_uint num_entries = 1;
	cl_uint num_platforms = 0;
	cl_uint num_devices = 0;
	ASSERT_NO_FATAL_FAILURE(getPlatformIDs(num_entries, ocl_descriptor.platforms, NULL));

	//	CPU is at index 0, GPU is at index 1
	ASSERT_NO_FATAL_FAILURE(getCPUDevice(ocl_descriptor.platforms, ocl_descriptor.devices));

	//create CL context from D3D9 device
	cl_context_properties properties[] = {
		CL_CONTEXT_PLATFORM, (cl_context_properties) ocl_descriptor.platforms[0],
		CL_CONTEXT_D3D9_DEVICE_INTEL, (cl_context_properties) d3d9Wrapper.Device(),	0};
	ASSERT_NO_FATAL_FAILURE(createContext(&ocl_descriptor.context, properties, 1, ocl_descriptor.devices, NULL, NULL));

	// create corresponding devices
	ASSERT_EQ(CL_SUCCESS, clGetDeviceIDsFromD3D9Intel(ocl_descriptor.platforms[0], CL_D3D9_DEVICE_INTEL, d3d9Wrapper.Device(), CL_ALL_DEVICES_FOR_DX9_INTEL, 0, 0, &devicesNum)) << "clGetDeviceIDsFromD3D9Intel failed";
    
	ASSERT_EQ(1, devicesNum) << "Number of corresponding devices is " << devicesNum;
	cl_device_id devices[] = {0,0};
	ASSERT_EQ(CL_SUCCESS, clGetDeviceIDsFromD3D9Intel(ocl_descriptor.platforms[0], CL_D3D9_DEVICE_INTEL, d3d9Wrapper.Device(), CL_ALL_DEVICES_FOR_DX9_INTEL, devicesNum, &devices[0], 0)) << "clGetDeviceIDsFromD3D9Intel failed";

	// validate that CPU device was returned
	ASSERT_TRUE(devices[0] == ocl_descriptor.devices[0]) << "CPU device is not corresponding to DX9 device";
}

#endif

