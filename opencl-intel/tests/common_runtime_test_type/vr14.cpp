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

class VR14: public CommonRuntime{};

//|	TEST: VR14.KernelCompilation (TC-76)
//|
//|	Purpose
//|	-------
//|	
//|	Verify the ability to create kernel objects for all kernel functions in a shared program
//|
//|	Method
//|	------
//|
//|	1. Build program with source of 10 kernels on both CPU and GPU
//|	2. Create 10 kernels for that program (will create for both CPU and GPU)
//|	
//|	Pass criteria
//|	-------------
//|
//|	Verify that valid non-zero kernel objects are returned
//|
TEST_F(VR14, KernelCompilation) 
{ 
	// create OpenCL queues, program and context
	ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueues(ocl_descriptor, "simple_kernels.cl"));

	cl_kernel kernels[10];
	for(int i=0; i<10; ++i)
	{
		std::stringstream ss;
		ss << "kernel_" << i;
		// create kernels
		kernels[i] = 0;
		ASSERT_NO_FATAL_FAILURE(createKernel(&kernels[i], ocl_descriptor.program, ss.str().c_str()));
	}
}

//|	TEST: VR14.KernelCompilationIfDefIfPart (TC-77) see more tests below for complete picture
//|
//|	Purpose
//|	-------
//|	
//|	Verify that the kernel objects are not created for any kernel functions in shared program
//|	that do not have the same function definition across all devices for which the program executable
//| has been successfully built
//|
//|	Method
//|	------
//|
//|	1. Create program with kernel source including two different kernel definitions (different number of arguments) for 
//|		the same kernel name (with if-def).
//|	2. Build program for both defices using ONLY "if" kernel version
//| 3. Create kernel (for both devices)
//|	
//|	Pass criteria
//|	-------------
//|
//|	Verify that valid non-zero kernel object is returned
TEST_F(VR14, KernelCompilationIfDefIfPart)
{
	// get pltfrom and device ids
	ASSERT_NO_FATAL_FAILURE(getCPUGPUDevices(ocl_descriptor.platforms, ocl_descriptor.devices));
	// cpu is at index 0, gpu is at index 1

	// create context
	cl_context_properties properties[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)ocl_descriptor.platforms[0], 0};
	ASSERT_NO_FATAL_FAILURE(createContext(&ocl_descriptor.context, properties, 2, ocl_descriptor.devices, NULL, NULL));

	// create program
	ASSERT_NO_FATAL_FAILURE(createProgramWithSourceFromKernelName(&ocl_descriptor.program, ocl_descriptor.context,
		"simple_kernels.cl"));

	// build program on both devices with IF definition
	ASSERT_NO_FATAL_FAILURE(buildProgram (&ocl_descriptor.program, 2, ocl_descriptor.devices, "-D FOR_CPU", NULL, NULL));
	// create kernel, expect success
	ASSERT_NO_FATAL_FAILURE(createKernel(&ocl_descriptor.kernels[0] , ocl_descriptor.program, "kernel_ifdef"));
}

//|	TEST: VR14.KernelCompilationIfDefElsePart (TC-77) see tests above and below for complete picture
//|
//|	Purpose
//|	-------
//|	
//|	Verify that the kernel objects are not created for any kernel functions in shared program
//|	that do not have the same function definition across all devices for which the program executable
//| has been successfully built
//|
//|	Method
//|	------
//|
//|	1. Create program with kernel source including two different kernel definitions (different number of arguments) for 
//|		the same kernel name (with if-def).
//|	2. Build program for both defices using ONLY "else" kernel version
//| 3. Create kernel (for both devices)
//|	
//|	Pass criteria
//|	-------------
//|
//|	Verify that valid non-zero kernel object is returned
TEST_F(VR14, KernelCompilationIfDefElsePart)
{
	// get pltfrom and device ids
	ASSERT_NO_FATAL_FAILURE(getCPUGPUDevices(ocl_descriptor.platforms, ocl_descriptor.devices));
	// cpu is at index 0, gpu is at index 1

	// create context
	cl_context_properties properties[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)ocl_descriptor.platforms[0], 0};
	ASSERT_NO_FATAL_FAILURE(createContext(&ocl_descriptor.context, properties, 2, ocl_descriptor.devices, NULL, NULL));

	// create program
	ASSERT_NO_FATAL_FAILURE(createProgramWithSourceFromKernelName(&ocl_descriptor.program, ocl_descriptor.context,
		"simple_kernels.cl"));

	// build program on both devices with ELSE definition
	ASSERT_NO_FATAL_FAILURE(buildProgram (&ocl_descriptor.program, 2, ocl_descriptor.devices, NULL, NULL, NULL));
	ASSERT_NO_FATAL_FAILURE(createKernel(&ocl_descriptor.kernels[0] , ocl_descriptor.program, "kernel_ifdef"));
}

//|	TEST: VR14.KernelCompilationIfDefBoth (TC-77) see tests above for complete picture
//|
//|	Purpose
//|	-------
//|	
//|	Verify that the kernel objects are not created for any kernel functions in shared program
//|	that do not have the same function definition across all devices for which the program executable
//| has been successfully built
//|
//|	Method
//|	------
//|
//|	1. Create program with kernel source including two different kernel definitions (different number of arguments) for 
//|		the same kernel name (with if-def).
//|	2. Build program for for one device using "if" build option and "else" build option for the other device
//| 3. Create kernel (for both devices)
//|	
//|	Pass criteria
//|	-------------
//|
//|	Verify that kernel creation returned CL_INVALID_KERNEL_DEFINITION
TEST_F(VR14, KernelCompilationIfDefBoth)
{
	// get pltfrom and device ids
	ASSERT_NO_FATAL_FAILURE(getCPUGPUDevices(ocl_descriptor.platforms, ocl_descriptor.devices));
	// cpu is at index 0, gpu is at index 1

	// create context
	cl_context_properties properties[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)ocl_descriptor.platforms[0], 0};
	ASSERT_NO_FATAL_FAILURE(createContext(&ocl_descriptor.context, properties, 2, ocl_descriptor.devices, NULL, NULL));

	// create program
	ASSERT_NO_FATAL_FAILURE(createProgramWithSourceFromKernelName(&ocl_descriptor.program, ocl_descriptor.context,
		"simple_kernels.cl"));

	// build on each device ine with IF and another in ELSE source
	ASSERT_NO_FATAL_FAILURE(buildProgram (&ocl_descriptor.program, 1, &ocl_descriptor.devices[0], "-D FOR_CPU", NULL, NULL));
	ASSERT_NO_FATAL_FAILURE(buildProgram (&ocl_descriptor.program, 1, &ocl_descriptor.devices[1], NULL, NULL, NULL));

	cl_int errcode_ret = CL_SUCCESS;
	ocl_descriptor.kernels[0] = clCreateKernel (ocl_descriptor.program, "kernel_ifdef", &errcode_ret);

	ASSERT_EQ(CL_INVALID_KERNEL_DEFINITION, errcode_ret) << "clCreateKernel did not return CL_INVALID_KERNEL_DEFINITION";
}
