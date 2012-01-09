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

#ifndef VR21_GTEST_
#define VR21_GTEST_

//|	TEST: CPUGPUsampler (TC-100)
//|
//|	Purpose
//|	-------
//|	
//|	Verify that an OpenCL sampler can be used by all 
//| different devices in a context (VR-21).
//|	Method
//|	------
//|
//|	1. Create a 3D image object
//|	2. Create a sampler object for the 3D image object
//|	3. Create a shared kernel that gets as arguments an
//|	   image object, a sampler object, and a buffer object
//|	   and uses the sampler to read the image and write the read
//|	   data to the buffer object.
//|	4. Queue the kernel execution command to the CPU commands-queue.
//|	5. Queue the Kernel execution command to the GPU commands-queue 
//|	   (with another buffer object).
//|
//|	Pass criteria
//|	-------------
//|
//|	The content of the destination buffer object should match the
//| content of the source 3D image object.
//|
static void testCPUGPUsamplerBody(OpenCLDescriptor& ocl_descriptor)
{
	// define the input image
	cl_int imageSize=4;//width and row dimensions
	cl_int imDepth=3;
	cl_int numChnl=4;
	cl_int numPixels=imageSize*imageSize*imDepth;
	cl_int i,j;
	size_t wkGrpDim[3]={imageSize,imageSize,imDepth};
	DynamicArray<cl_float> cpuInput_image(numPixels*numChnl,255.0f);
	DynamicArray<cl_float> gpuInput_image(numPixels*numChnl,255.0f);
	DynamicArray<cl_float> cpuOutput_image(numPixels*numChnl,0.0f);
	DynamicArray<cl_float> gpuOutput_image(numPixels*numChnl,0.0f);
	cl_float imInCpy[4*4*4*3];
	cl_float imOutCpy[4*4*4*3];
	cl_mem buffers[4];
	for(i=0;i<numPixels*numChnl;i++)
	{
		imInCpy[i]=255;
	}
	for(i=0;i<numPixels*numChnl;i+=numChnl*imDepth)
	{
		for(j=0;j<imDepth;j+=numChnl)
		{
			cpuInput_image.dynamic_array[i+j+1]=0;
			cpuInput_image.dynamic_array[i+j+3]=1;
			gpuInput_image.dynamic_array[i+j+1]=0;
			gpuInput_image.dynamic_array[i+j+3]=1;
			imInCpy[i+j+1]=0;
			imInCpy[i+j+3]=1;
		}
	}	

	// set up shared context, program and queues
	ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueues(ocl_descriptor, "samplerkernel.cl"));

	// Create the sampler
	cl_sampler sampler;
	
	ASSERT_NO_FATAL_FAILURE(createSampler(
		&sampler,
		ocl_descriptor.context,
		CL_FALSE,
		CL_ADDRESS_NONE,
		CL_FILTER_NEAREST));

	// Create the CPU 3D image object
	cl_image_format imFormat={CL_RGBA,CL_FLOAT};
	ASSERT_NO_FATAL_FAILURE(createImage3D(
		&buffers[0],
		ocl_descriptor.context,
		(CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR),
		&imFormat,
		imageSize,
		imageSize,
		imDepth,
		0,
		0,
		cpuInput_image.dynamic_array));
	
	// Create the CPU output buffer
	ASSERT_NO_FATAL_FAILURE(createBuffer(
		&buffers[1],
		ocl_descriptor.context, 
		(CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR), 
		sizeof(cl_float)*numPixels*numChnl, 
		cpuOutput_image.dynamic_array));

	// Create the GPU 3D image object
	ASSERT_NO_FATAL_FAILURE(createImage3D(
		&buffers[2],
		ocl_descriptor.context,
		(CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR),
		&imFormat,
		imageSize,
		imageSize,
		imDepth,
		0,
		0,
		gpuInput_image.dynamic_array));
	
	// Create the GPU output buffer
	ASSERT_NO_FATAL_FAILURE(createBuffer(
		&buffers[3],
		ocl_descriptor.context, 
		(CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR), 
		sizeof(cl_float)*numPixels*numChnl, 
		gpuOutput_image.dynamic_array));

	// createKernel - calls and validates clCreateKernel
	void createKernel(cl_kernel* kernel , cl_program program, const char *kernel_name);

	// Create the kernel
	ASSERT_NO_FATAL_FAILURE(createKernelsInProgram(
		ocl_descriptor.program,
		1,
		&ocl_descriptor.kernels[0],
		NULL));

	// Set kernel args
	ASSERT_NO_FATAL_FAILURE(setKernelArg(
		ocl_descriptor.kernels[0],
		0,
		sizeof(cl_mem),
		(void *)&buffers[0]));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(
		ocl_descriptor.kernels[0],
		1,
		sizeof(cl_sampler),
		(void *)&sampler));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(
		ocl_descriptor.kernels[0],
		2,
		sizeof(cl_mem),
		(void *)&buffers[1]));
	// Execute the kernel in the CPU
	ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(
		ocl_descriptor.queues[0],
		ocl_descriptor.kernels[0],
		3,
		NULL,
		wkGrpDim,
		NULL,
		NULL,
		NULL,
		NULL));
	// Read from the output buffer
	ASSERT_NO_FATAL_FAILURE(enqueueReadBuffer(
		ocl_descriptor.queues[0],
		buffers[1],
		CL_TRUE,
		0,
		sizeof(cl_float)*numPixels*numChnl,
		cpuOutput_image.dynamic_array,
		0,
		NULL,
		NULL));

	ASSERT_NO_FATAL_FAILURE(cpuInput_image.compareArray(
		cpuOutput_image.dynamic_array,
		cpuInput_image.dynamic_array_size));

	// Excute Kernel on the GPU
	// Set kernel args
	ASSERT_NO_FATAL_FAILURE(setKernelArg(
		ocl_descriptor.kernels[0],
		0,
		sizeof(cl_mem),
		(void *)&buffers[2]));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(
		ocl_descriptor.kernels[0],
		1,
		sizeof(cl_sampler),
		(void *)&sampler));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(
		ocl_descriptor.kernels[0],
		2,
		sizeof(cl_mem),
		(void *)&buffers[3]));
	// Execute the kernel in the CPU
	ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(
		ocl_descriptor.queues[1],
		ocl_descriptor.kernels[0],
		3,
		NULL,
		wkGrpDim,
		NULL,
		NULL,
		NULL,
		NULL));
	// Read from the output buffer
	ASSERT_NO_FATAL_FAILURE(enqueueReadBuffer(
		ocl_descriptor.queues[1],
		buffers[3],
		CL_TRUE,
		0,
		sizeof(cl_float)*numPixels*numChnl,
		gpuOutput_image.dynamic_array,
		0,
		NULL,
		NULL));

	ASSERT_NO_FATAL_FAILURE(gpuInput_image.compareArray(
		gpuOutput_image.dynamic_array,
		gpuInput_image.dynamic_array_size));
}

#endif /* VR21_GTEST_ */
