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

class CRT12_VR274_280: public CommonRuntime{};



//|	TEST: CRT12_VR274_280.LinkProgramWithBuildBinaries (TC-1)
//|
//|	Purpose
//|	-------
//|	
//|	Verify that a number of programs (2) can be send to linkProgram without crushing.
//|
//|	Method
//|	------
//|
//|	1.	build a two kernels
//|	2.	Run  clLinkProgram()
//|	3.	Expect success
//|	
//|	Pass criteria
//|	-------------
//|
//|	program should succesfully create. 
//|

TEST_F(CRT12_VR274_280, LinkProgramWithBuildBinaries){
	cl_program extra_program;
	// set up shared context, program and queues with kernel1
	ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueues(ocl_descriptor, "simple_kernels.cl"));

	//	create and build program with kernel2
	ASSERT_NO_FATAL_FAILURE(createAndBuildProgramWithSource("simple_kernels2.cl", &extra_program, ocl_descriptor.context, 2, ocl_descriptor.devices, NULL, NULL, NULL));

	cl_program prog_list[2]={extra_program,ocl_descriptor.program};
	//link the two programs
	ASSERT_NO_FATAL_FAILURE(linkProgram(ocl_descriptor.context,2,ocl_descriptor.devices,NULL,2, prog_list, NULL));
}



//|	TEST: CRT12_VR274_280.LinkProgram_vr274 (TC-1)
//|
//|	Purpose
//|	-------
//|	
//|	Verify that a number of programs (2) can be linked into a single program.
//|
//|	Method
//|	------
//|
//|	1.	Compile a two kernels
//|	2.	Run  clLinkProgram()
//|	3.	Expect success
//|	
//|	Pass criteria
//|	-------------
//|
//|	program should succesfully create. 
//|

TEST_F(CRT12_VR274_280, LinkProgram_vr274){
	cl_program programs[2];
	// set up shared context, program and queues with kernel1
	ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueues(ocl_descriptor, "simple_kernels.cl"));

	//	create and build programs with 2 kernel
	
	ASSERT_NO_FATAL_FAILURE(createAndCompileProgramWithSource("simple_kernels.cl", &programs[0], ocl_descriptor.context, 2, ocl_descriptor.devices, NULL, NULL, NULL));
	ASSERT_NO_FATAL_FAILURE(createAndCompileProgramWithSource("simple_kernels2.cl", &programs[1], ocl_descriptor.context, 2, ocl_descriptor.devices, NULL, NULL, NULL));
	
	//link the two programs
	ASSERT_NO_FATAL_FAILURE(linkProgram(ocl_descriptor.context,2,ocl_descriptor.devices,NULL,2, programs, NULL));


}

//|	TEST: CRT12_VR274_280.LinkProgramForCPU_vr280 
//|
//|	Purpose
//|	-------
//|	
//|	Verify that a number of programs (2) can be linked into a single program.
//|
//|	Method
//|	------
//|
//|	1.	Compile a two kernels
//|	2.	Run  clLinkProgram()
//|	3.	run kernel on CPU
//|	4. run a kernel on GPU expect fail
//|	
//|	Pass criteria
//|	-------------
//|
//|	kernel should run on CPU but not on GPU. 
//|

TEST_F(CRT12_VR274_280, LinkProgramForCPU_vr280){
	cl_uint work_dim = 1;
	size_t global_work_size = 1;
	size_t local_work_size = 1;
	DynamicArray<cl_int> array(4);
	cl_program programs[2];
	cl_program linked_prog;
	cl_int one=1;
	// set up shared context, program and queues with kernel1
	ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueues(ocl_descriptor, "simple_kernels.cl"));

	//	create and build programs with 2 kernel	
	ASSERT_NO_FATAL_FAILURE(createAndCompileProgramWithSource("simple_kernels.cl", &programs[0], ocl_descriptor.context, 2, ocl_descriptor.devices, NULL, NULL, NULL));
	ASSERT_NO_FATAL_FAILURE(createAndCompileProgramWithSource("simple_kernels2.cl", &programs[1], ocl_descriptor.context, 2, ocl_descriptor.devices, NULL, NULL, NULL));
	
	//link the two programs
	ASSERT_NO_FATAL_FAILURE(linkProgram(ocl_descriptor.context,1,&ocl_descriptor.devices[0],NULL,2, programs, &linked_prog));

	//build the linked program
	ASSERT_NO_FATAL_FAILURE(buildProgram(&linked_prog,1,&ocl_descriptor.devices[0],NULL,NULL,NULL));
	//create buffer
	ASSERT_NO_FATAL_FAILURE(createBuffer(&ocl_descriptor.in_common_buffer,ocl_descriptor.context,CL_MEM_READ_WRITE|CL_MEM_USE_HOST_PTR,sizeof(cl_int4),array.dynamic_array));

	//create kernel 
	ASSERT_NO_FATAL_FAILURE(createKernel(ocl_descriptor.kernels,ocl_descriptor.program,"read_write_image1D_int4"));

	//set kernel arg
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0],0,sizeof(cl_mem),&ocl_descriptor.in_common_buffer));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0],1,sizeof(cl_int),&one));

	// enqueue kernel on CPU 
	ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(ocl_descriptor.queues[0], ocl_descriptor.kernels[0], work_dim, 0, &global_work_size, &local_work_size,0,NULL,NULL));

	//enqueue kernel on GPU
	ASSERT_NE(CL_SUCCESS,clEnqueueNDRangeKernel(ocl_descriptor.queues[1], ocl_descriptor.kernels[0], work_dim, 0, &global_work_size, &local_work_size,0,NULL,NULL)) << "clEnqueueNDRangeKernel should not succed";

}


//|	TEST: CRT12_VR274_280.LinkProgramForGPU_vr281
//|
//|	Purpose
//|	-------
//|	
//|	Verify that a number of programs (2) can be linked into a single program.
//|
//|	Method
//|	------
//|
//|	1.	Compile a two kernels
//|	2.	Run  clLinkProgram()
//|	3.	run kernel on GPU
//|	4.  run a kernel on CPU 
//|	
//|	Pass criteria
//|	-------------
//|
//|	kernel should run on CPU and GPU. 
//|


TEST_F(CRT12_VR274_280, LinkProgramForGPU_vr281){
		cl_uint work_dim = 1;
	size_t global_work_size = 1;
	size_t local_work_size = 1;
	DynamicArray<cl_int> array(4);
	cl_program programs[2];
	cl_program linked_prog;
	cl_int one=1;
	// set up shared context, program and queues with kernel1
	ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueues(ocl_descriptor, "simple_kernels.cl"));

	//	create and build programs with 2 kernel	
	ASSERT_NO_FATAL_FAILURE(createAndCompileProgramWithSource("simple_kernels.cl", &programs[0], ocl_descriptor.context, 2, ocl_descriptor.devices, NULL, NULL, NULL));
	ASSERT_NO_FATAL_FAILURE(createAndCompileProgramWithSource("simple_kernels2.cl", &programs[1], ocl_descriptor.context, 2, ocl_descriptor.devices, NULL, NULL, NULL));
	
	//link the two programs
	ASSERT_NO_FATAL_FAILURE(linkProgram(ocl_descriptor.context,1,&ocl_descriptor.devices[0],NULL,2, programs, &linked_prog));

	//build the linked program
	ASSERT_NO_FATAL_FAILURE(buildProgram(&linked_prog,1,&ocl_descriptor.devices[1],NULL,NULL,NULL));
	//create buffer
	ASSERT_NO_FATAL_FAILURE(createBuffer(&ocl_descriptor.in_common_buffer,ocl_descriptor.context,CL_MEM_READ_WRITE|CL_MEM_USE_HOST_PTR,sizeof(cl_int4),array.dynamic_array));

	//create kernel 
	ASSERT_NO_FATAL_FAILURE(createKernel(ocl_descriptor.kernels,ocl_descriptor.program,"read_write_image1D_int4"));

	//set kernel arg
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0],0,sizeof(cl_mem),&ocl_descriptor.in_common_buffer));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0],1,sizeof(cl_int),&one));

	// enqueue kernel on GPU 
	ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(ocl_descriptor.queues[1], ocl_descriptor.kernels[0], work_dim, 0, &global_work_size, &local_work_size,0,NULL,NULL));

	// enqueue kernel on CPU 
	ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(ocl_descriptor.queues[0], ocl_descriptor.kernels[0], work_dim, 0, &global_work_size, &local_work_size,0,NULL,NULL));

}


//|	TEST: CRT12_VR274_280.LinkProgramForGPUCPU_vr282
//|
//|	Purpose
//|	-------
//|	
//|	Verify that a number of programs (2) can be linked into a single program.
//|
//|	Method
//|	------
//|
//|	1.	Compile a two kernels
//|	2.	Run  clLinkProgram()
//|	3.	run kernel on CPU
//|	4. run a kernel on GPU expect fail
//|	
//|	Pass criteria
//|	-------------
//|
//|	kernel should run on CPU but not on GPU. 
//|


TEST_F(CRT12_VR274_280, LinkProgramForGPUCPU_vr282){
	cl_uint work_dim = 1;
	size_t global_work_size = 1;
	size_t local_work_size = 1;
	DynamicArray<cl_int> array(4);
	cl_program extra_program;
	cl_int one=1;
	// set up shared context, program and queues with kernel1
	ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueues(ocl_descriptor, "simple_kernels.cl"));

	//	create and build program with kernel2
	ASSERT_NO_FATAL_FAILURE(createAndBuildProgramWithSource("simple_kernels2.cl", &extra_program, ocl_descriptor.context, 2, ocl_descriptor.devices, NULL, NULL, NULL));

	cl_program prog_list[2]={extra_program,ocl_descriptor.program};
	//link the two programs
	ASSERT_NO_FATAL_FAILURE(linkProgram(ocl_descriptor.context,2,ocl_descriptor.devices,NULL,2, prog_list, NULL));

	//create buffer
	ASSERT_NO_FATAL_FAILURE(createBuffer(&ocl_descriptor.in_common_buffer,ocl_descriptor.context,CL_MEM_READ_WRITE|CL_MEM_USE_HOST_PTR,sizeof(cl_int4),array.dynamic_array));

	//create kernel 
	ASSERT_NO_FATAL_FAILURE(createKernel(ocl_descriptor.kernels,ocl_descriptor.program,"read_write_image1D_int4"));

	//set kernel arg
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0],0,sizeof(cl_mem),&ocl_descriptor.in_common_buffer));
	ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0],1,sizeof(cl_int),&one));

	// enqueue kernel on GPU 
	ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(ocl_descriptor.queues[1], ocl_descriptor.kernels[0], work_dim, 0, &global_work_size, &local_work_size,0,NULL,NULL));

	//enqueue kernel on CPU
	ASSERT_NE(CL_SUCCESS,clEnqueueNDRangeKernel(ocl_descriptor.queues[0], ocl_descriptor.kernels[0], work_dim, 0, &global_work_size, &local_work_size,0,NULL,NULL)) << "clEnqueueNDRangeKernel should not succed";;

}