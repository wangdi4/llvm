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

class VR11: public CommonRuntime{};

//|	TEST: VR11.CPUOnlyBuild (TC-66) 
//|
//|	Purpose
//|	-------
//|	
//|	Verify the ability to build (complie and link) a program executable
//|	for CPU device only from pogram source.
//| 
//|	Method
//|	------
//|
//|	1.	Create program object from source and build it on CPU device only.
//|	
//|	Pass criteria
//|	-------------
//|
//|	Validate that build on CPU was successfull
//|
TEST_F(VR11, CPUOnlyBuild)
{
	// get pltfrom and device id
	ASSERT_NO_FATAL_FAILURE(getCPUGPUDevices(ocl_descriptor.platforms, ocl_descriptor.devices));

	// create context for CPU
	ASSERT_NO_FATAL_FAILURE(createContext(&ocl_descriptor.context, 0, 2, ocl_descriptor.devices, NULL, NULL));

	// create and build program for CPU
	ASSERT_NO_FATAL_FAILURE( createAndBuildProgramWithSource("simple_kernels.cl", &ocl_descriptor.program, 
		ocl_descriptor.context,	1, &ocl_descriptor.devices[0], NULL, NULL,  NULL));
}

//|	TEST: VR11.GPUOnlyBuild (TC-67) 
//|
//|	Purpose
//|	-------
//|	
//|	Verify the ability to build (complie and link) a program executable
//|	for GPU device only from pogram source.
//| 
//|	Method
//|	------
//|
//|	1.	Create program object from source and build it on GPU device only.
//|	
//|	Pass criteria
//|	-------------
//|
//|	Validate that build on GPU was successfull
//|
TEST_F(VR11, GPUOnlyBuild)
{
	// get pltfrom and device id
	ASSERT_NO_FATAL_FAILURE(getCPUGPUDevices(ocl_descriptor.platforms, ocl_descriptor.devices));

	// create context for CPU
	ASSERT_NO_FATAL_FAILURE(createContext(&ocl_descriptor.context, 0, 2, ocl_descriptor.devices, NULL, NULL));

	// create and build program for CPU
	ASSERT_NO_FATAL_FAILURE( createAndBuildProgramWithSource("simple_kernels.cl", &ocl_descriptor.program, 
		ocl_descriptor.context,	1, &ocl_descriptor.devices[1], NULL, NULL,  NULL));
}

//|	TEST: VR11.CPUGPUBuild (TC-68) 
//|
//|	Purpose
//|	-------
//|	
//|	Verify the ability to build (complie and link) a program executable
//|	for CPU and GPU devices from pogram source.
//| 
//|	Method
//|	------
//|
//|	1.	Create program object from source and build it on CPU and GPU device only.
//|	
//|	Pass criteria
//|	-------------
//|
//|	Validate that build on CPU and GPU was successfull
//|
TEST_F(VR11, CPUGPUBuild)
{
	// get pltfrom and device id
	ASSERT_NO_FATAL_FAILURE(getCPUGPUDevices(ocl_descriptor.platforms, ocl_descriptor.devices));

	// create context for CPU
	ASSERT_NO_FATAL_FAILURE(createContext(&ocl_descriptor.context, 0, 2, ocl_descriptor.devices, NULL, NULL));

	// create and build program for CPU
	ASSERT_NO_FATAL_FAILURE( createAndBuildProgramWithSource("simple_kernels.cl", &ocl_descriptor.program, 
		ocl_descriptor.context,	2, ocl_descriptor.devices, NULL, NULL,  NULL));
}

//|	TEST: VR11.CPUGPUBuildIfDef (TC-69) 
//|
//|	Purpose
//|	-------
//|	
//|	Verify the ability to build (compiles & links) a program executable from program source
//|	for both CPU and GPU devices where the program include ifdef statement (for device type)
//|	to differentiate the code per device type 
//| 
//|	Method
//|	------
//|
//|	1.	Create a program object and load the source code specified by the text strings into a program object
//|	2.	Build (compiles & links) a program executable from the program source for CPU device
//|	3.	Build (compiles & links) a program executable from the program source for GPU device
//|	
//|	Pass criteria
//|	-------------
//|
//|	A valid non-zero program object should be returned and the programs should be built successfully.
//|
TEST_F(VR11, CPUGPUBuildIfDef)
{
  // get pltfrom and device id
  ASSERT_NO_FATAL_FAILURE(getCPUGPUDevices(ocl_descriptor.platforms, ocl_descriptor.devices));

  // create context for CPU
  ASSERT_NO_FATAL_FAILURE(createContext(&ocl_descriptor.context, 0, 2, ocl_descriptor.devices, NULL, NULL));

  // create and build program for CPU and GPU
  // create and build program for CPU and GPU
  ASSERT_NO_FATAL_FAILURE( createAndBuildProgramWithSource("simple_kernels_ifdef.cl", &ocl_descriptor.program, 
    ocl_descriptor.context,	2, ocl_descriptor.devices, NULL, NULL,  NULL));
}

//|	TEST: VR11.GPUErrorKernelOnCPU (TC-70) see also other tests below  
//|
//|	Purpose
//|	-------
//|	
//|	Verify that a program is sucessfully built (compile and links)
//|	only if it is successfully built for all the required devices.
//| 
//|	Method
//|	------
//|
//|	1.	Create program object from source code which includes a statement
//|		which is erroneous on GPU device (and correct on CPU).
//| 2.	Build program executable from the program source  CPU device only.
//|	
//|	Pass criteria
//|	-------------
//|
//|	Validate that build on CPU was successfull
//|
TEST_F(VR11, GPUErrorKernelOnCPU)
{
	// get pltfrom and device ids
	ASSERT_NO_FATAL_FAILURE(getCPUGPUDevices(ocl_descriptor.platforms, ocl_descriptor.devices));
	// cpu is at index 0, gpu is at index 1

	// create context for CPU
	ASSERT_NO_FATAL_FAILURE(createContext(&ocl_descriptor.context, 0, 2, ocl_descriptor.devices, NULL, NULL));

	// create and build program for CPU
	ASSERT_NO_FATAL_FAILURE( createAndBuildProgramWithSource("gpu_error_kernel.cl", &ocl_descriptor.program, 
		ocl_descriptor.context,	1, &ocl_descriptor.devices[0], NULL, NULL,  NULL));
}

//|	TEST: VR11.GPUErrorKernelOnCPU (TC-70)
//|
//|	Purpose
//|	-------
//|	
//|	Verify that a program is sucessfully built (compile and links)
//|	only if it is successfully built for all the required devices.
//| 
//|	Method
//|	------
//|
//|	1.	Create program object from source code which includes a statement
//|		which is erroneous on GPU device (and correct on CPU).
//| 2.	Build program executable from the program source  GPU device only.
//|	
//|	Pass criteria
//|	-------------
//|
//|	Validate that build on GPU was not successfull, value CL_BUILD_PROGRAM_FAILURE was returned
//|
TEST_F(VR11, GPUErrorKernelOnGPU)
{
	// get pltfrom and device ids
	ASSERT_NO_FATAL_FAILURE(getCPUGPUDevices(ocl_descriptor.platforms, ocl_descriptor.devices));
	// cpu is at index 0, gpu is at index 1

	// create context for GPU
	ASSERT_NO_FATAL_FAILURE(createContext(&ocl_descriptor.context, 0, 2, ocl_descriptor.devices, NULL, NULL));
	
	// create program on GPU context
	ASSERT_NO_FATAL_FAILURE(createProgramWithSourceFromKernelName(&ocl_descriptor.program, ocl_descriptor.context,
		"gpu_error_kernel.cl"));

	// build for GPU device
	cl_int errcode_ret = clBuildProgram (ocl_descriptor.program, 1, &ocl_descriptor.devices[1], NULL, NULL, NULL);	
	
	// expect failure
	ASSERT_EQ(CL_BUILD_PROGRAM_FAILURE, errcode_ret)  << "clBuildProgram did not fail on GPU";
}

//|	TEST: VR11.GPUErrorKernelOnCPU (TC-70)
//|
//|	Purpose
//|	-------
//|	
//|	Verify that a program is sucessfully built (compile and links)
//|	only if it is successfully built for all the required devices.
//| 
//|	Method
//|	------
//|
//|	1.	Create program object from source code which includes a statement
//|		which is erroneous on GPU device (and correct on CPU).
//| 2.	Build program executable from the program source on CPU and GPU devices
//|	
//|	Pass criteria
//|	-------------
//|
//|	Validate that build was not successfull, value CL_BUILD_PROGRAM_FAILURE was returned
//|
TEST_F(VR11, GPUErrorKernelOnCPUGPU)
{
	// get pltfrom and device ids
	ASSERT_NO_FATAL_FAILURE(getCPUGPUDevices(ocl_descriptor.platforms, ocl_descriptor.devices));
	// cpu is at index 0, gpu is at index 1

	// create shared context
	ASSERT_NO_FATAL_FAILURE(createContext(&ocl_descriptor.context, 0, 2, ocl_descriptor.devices, NULL, NULL));

	// vreate and build program for CPU and GPU
	ASSERT_NO_FATAL_FAILURE(createProgramWithSourceFromKernelName(&ocl_descriptor.program, ocl_descriptor.context,
		 "gpu_error_kernel.cl"));
	cl_int errcode_ret = clBuildProgram (ocl_descriptor.program, 2, ocl_descriptor.devices, NULL, NULL, NULL);	

	// expect failure
	ASSERT_EQ(CL_BUILD_PROGRAM_FAILURE, errcode_ret)  << "clBuildProgram did not fail on CPU and GPU";

	// assert build success on CPU
	cl_build_status build_status = 0;
	ASSERT_NO_FATAL_FAILURE(getProgramBuildInfo(ocl_descriptor.program, ocl_descriptor.devices[0], 
		CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status), &build_status, NULL));

	ASSERT_EQ(CL_BUILD_SUCCESS, build_status);

	// assert build failure on GPU
	ASSERT_NO_FATAL_FAILURE(getProgramBuildInfo(ocl_descriptor.program, ocl_descriptor.devices[1], 
		CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status), &build_status, NULL));
	ASSERT_NE(CL_BUILD_SUCCESS, build_status);
}

