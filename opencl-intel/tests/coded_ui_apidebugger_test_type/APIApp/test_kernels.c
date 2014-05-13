/******************************************************************
//
//  OpenCL Conformance Tests
// 
//  Copyright:	(c) 2008-2013 by Apple Inc. All Rights Reserved.
//
******************************************************************/

#include "testBase.h"
#include "harness/conversions.h"

extern cl_uint gRandomSeed;

const char *sample_single_test_kernel[] = {
"__kernel void sample_test(__global float *src, __global int *dst)\n"
"{\n"
"    int  tid = get_global_id(0);\n"
"\n"
"    dst[tid] = (int)src[tid];\n"
"\n"
"}\n" };

const char *sample_struct_test_kernel[] = {
"typedef struct {\n"
"__global int *A;\n"
"__global int *B;\n"
"} input_pair_t;\n"
"\n"
"__kernel void sample_test(__global input_pair_t *src, __global int *dst)\n"
"{\n"
"    int  tid = get_global_id(0);\n"
"\n"
"    dst[tid] = src->A[tid] + src->B[tid];\n"
"\n"
"}\n" };

const char *sample_struct_array_test_kernel[] = {
"typedef struct {\n"
"int A;\n"
"int B;\n"
"} input_pair_t;\n"
"\n"
"__kernel void sample_test(__global input_pair_t *src, __global int *dst)\n"
"{\n"
"    int  tid = get_global_id(0);\n"
"\n"
"    dst[tid] = src[tid].A + src[tid].B;\n"
"\n"
"}\n" };

const char *sample_const_test_kernel[] = {
"__kernel void sample_test(__constant int *src1, __constant int *src2, __global int *dst)\n"
"{\n"
"    int  tid = get_global_id(0);\n"
"\n"
"    dst[tid] = src1[tid] + src2[tid];\n"
"\n"
"}\n" };

const char *sample_const_global_test_kernel[] = {
"__constant int addFactor = 1024;\n"
"__kernel void sample_test(__global int *src1, __global int *dst)\n"
"{\n"
"    int  tid = get_global_id(0);\n"
"\n"
"    dst[tid] = src1[tid] + addFactor;\n"
"\n"
"}\n" };

const char *sample_two_kernel_program[] = {
"__kernel void sample_test(__global float *src, __global int *dst)\n"
"{\n"
"    int  tid = get_global_id(0);\n"
"\n"
"    dst[tid] = (int)src[tid];\n"
"\n"
"}\n",
"__kernel void sample_test2(__global int *src, __global float *dst)\n"
"{\n"
"    int  tid = get_global_id(0);\n"
"\n"
"    dst[tid] = (float)src[tid];\n"
"\n"
"}\n" };




int test_get_kernel_info(cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements)
{
	int error;
	cl_program program, testProgram;
	cl_context testContext;
	cl_kernel kernel;
	cl_char name[ 512 ];
	cl_uint numArgs, numInstances;
	size_t paramSize;
	size_t kernel_max_workgroup_size;
	
	/* Create reference */
	if( create_single_kernel_helper( context, &program, &kernel, 1, sample_single_test_kernel, "sample_test" ) != 0 )
	{
		return -1;
	}

	error = clGetKernelWorkGroupInfo(kernel, deviceID, CL_KERNEL_WORK_GROUP_SIZE, NULL, &kernel_max_workgroup_size, NULL);
	error = clGetKernelWorkGroupInfo(kernel, deviceID, CL_KERNEL_WORK_GROUP_SIZE, sizeof(kernel_max_workgroup_size), &kernel_max_workgroup_size, NULL);
	test_error( error, "clGetKernelWorkGroupInfo failed for CL_KERNEL_WORK_GROUP_SIZE");
	log_info("The CL_KERNEL_WORK_GROUP_SIZE for the kernel is %d.\n", (int)kernel_max_workgroup_size);

	cl_kernel_arg_address_qualifier address_qualifier = 0;
	error = clGetKernelArgInfo( kernel, (cl_uint)0, CL_KERNEL_ARG_ADDRESS_QUALIFIER, NULL, &address_qualifier, &paramSize );
	error = clGetKernelArgInfo( kernel, (cl_uint)0, CL_KERNEL_ARG_ADDRESS_QUALIFIER, sizeof address_qualifier, &address_qualifier, &paramSize );
	test_error( error, "Unable to get argument address qualifier" );

	error = clGetKernelInfo( kernel, NULL, NULL, 0, &paramSize );
	error = clGetKernelInfo( kernel, CL_KERNEL_FUNCTION_NAME, NULL, 0, &paramSize );
	test_error( error, "Unable to get kernel function name param size" );
	if( paramSize != strlen( "sample_test" ) + 1 )
	{
		log_error( "ERROR: Kernel function name param returns invalid size (expected %d, got %d)\n", (int)strlen( "sample_test" ) + 1, (int)paramSize );
		return -1;
	}
	
	error = clGetKernelInfo( kernel, CL_KERNEL_FUNCTION_NAME, sizeof( name ), name, NULL );
	test_error( error, "Unable to get kernel function name" );
	if( strcmp( (char *)name, "sample_test" ) != 0 )
	{
		log_error( "ERROR: Kernel function name returned invalid value (expected sample_test, got %s)\n", (char *)name );
		return -1;
	}
	

	error = clGetKernelInfo( kernel, CL_KERNEL_NUM_ARGS, 0, NULL, &paramSize );
	test_error( error, "Unable to get kernel arg count param size" );
	if( paramSize != sizeof( numArgs ) )
	{
		log_error( "ERROR: Kernel arg count param returns invalid size (expected %d, got %d)\n", (int)sizeof( numArgs ), (int)paramSize );
		return -1;
	}
	
	error = clGetKernelInfo( kernel, CL_KERNEL_NUM_ARGS, sizeof( numArgs ), &numArgs, NULL );
	test_error( error, "Unable to get kernel arg count" );
	if( numArgs != 2 )
	{
		log_error( "ERROR: Kernel arg count returned invalid value (expected %d, got %d)\n", 2, numArgs );
		return -1;
	}
	

	error = clGetKernelInfo( kernel, CL_KERNEL_REFERENCE_COUNT, 0, NULL, &paramSize );
	test_error( error, "Unable to get kernel reference count param size" );
	if( paramSize != sizeof( numInstances ) )
	{
		log_error( "ERROR: Kernel reference count param returns invalid size (expected %d, got %d)\n", (int)sizeof( numInstances ), (int)paramSize );
		return -1;
	}

	error = clGetKernelInfo( kernel, CL_KERNEL_REFERENCE_COUNT, sizeof( numInstances ), &numInstances, NULL );
	test_error( error, "Unable to get kernel reference count" );
	

	error = clGetKernelInfo( kernel, CL_KERNEL_PROGRAM, NULL, 0, &paramSize );
	test_error( error, "Unable to get kernel program param size" );
	if( paramSize != sizeof( testProgram ) )
	{
		log_error( "ERROR: Kernel program param returns invalid size (expected %d, got %d)\n", (int)sizeof( testProgram ), (int)paramSize );
		return -1;
	}
	
	error = clGetKernelInfo( kernel, CL_KERNEL_PROGRAM, sizeof( testProgram ), &testProgram, NULL );
	test_error( error, "Unable to get kernel program" );
	if( testProgram != program )
	{
		log_error( "ERROR: Kernel program returned invalid value (expected %p, got %p)\n", program, testProgram );
		return -1;
	}
	
	error = clGetKernelInfo( kernel, CL_KERNEL_CONTEXT, sizeof( testContext ), &testContext, NULL );
	test_error( error, "Unable to get kernel context" );
	if( testContext != context )
	{
		log_error( "ERROR: Kernel context returned invalid value (expected %p, got %p)\n", context, testContext );
		return -1;
	}
		
	/* Release memory */
	clReleaseKernel( kernel );
	clReleaseProgram( program );
	return 0;
}

int test_set_kernel_arg_struct_array(cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements)
{
	int error;
	clProgramWrapper program;
	clKernelWrapper kernel;
	clMemWrapper			streams[2];
	size_t	threads[1], localThreads[1];
	cl_int outputData[10];
	int i;
    MTdata d;
	
	typedef struct img_pair_type
	{
		int A;
		int B;
	} image_pair_t;
	
	image_pair_t image_pair[ 10 ];
	

	/* Create a kernel to test with */
	if( create_single_kernel_helper( context, &program, &kernel, 1, sample_struct_array_test_kernel, "sample_test" ) != 0 )
	{
		return -1;
	}
	
	/* Create some I/O streams */
    d = init_genrand( gRandomSeed );
	for( i = 0; i < 10; i++ )
	{
		image_pair[i].A = (cl_int)genrand_int32(d);
		image_pair[i].A = (cl_int)genrand_int32(d);
	}
	free_mtdata(d); d = NULL;
    
	streams[0] = clCreateBuffer(context, (cl_mem_flags)(CL_MEM_COPY_HOST_PTR), sizeof(image_pair_t) * 10, NULL, &error);
	streams[0] = clCreateBuffer(context, (cl_mem_flags)(CL_MEM_COPY_HOST_PTR), sizeof(image_pair_t) * 10, (void *)image_pair, &error);
	test_error( error, "Creating test array failed" );
	streams[1] = clCreateBuffer(context, (cl_mem_flags)(CL_MEM_READ_WRITE),  sizeof(cl_int) * 10, NULL, &error);
	test_error( error, "Creating test array failed" );

	/* Set the arguments */
	error = clSetKernelArg(kernel, 3, sizeof( streams[0] ), &streams[0]);
	error = clSetKernelArg(kernel, 0, sizeof( streams[0] ), &streams[0]);
	test_error( error, "Unable to set indexed kernel arguments" );
	error = clSetKernelArg(kernel, 1, sizeof( streams[1] ), &streams[1]);
	test_error( error, "Unable to set indexed kernel arguments" );

	/* Test running the kernel and verifying it */
	threads[0] = (size_t)10;

	error = get_max_common_work_group_size( context, kernel, threads[0], &localThreads[0] );
	test_error( error, "Unable to get work group size to use" );

	error = clEnqueueNDRangeKernel( queue, kernel, 1, NULL, NULL, localThreads, 0, NULL, NULL );
	error = clEnqueueNDRangeKernel( queue, kernel, 1, NULL, threads, localThreads, 0, NULL, NULL );
	test_error( error, "Kernel execution failed" );

	error = clEnqueueReadBuffer( queue, NULL, CL_TRUE, 0, sizeof(cl_int)*10, (void *)outputData, 0, NULL, NULL );
	error = clEnqueueReadBuffer( queue, streams[1], CL_TRUE, 0, sizeof(cl_int)*10, (void *)outputData, 0, NULL, NULL );
	test_error( error, "Unable to get result data" );

    for (i=0; i<10; i++)
    {
        if (outputData[i] != image_pair[i].A + image_pair[i].B)
        {
			log_error( "ERROR: Data did not verify!\n" );
            return -1;
        }
    }
		
    return 0;
}

int test_create_kernels_in_program(cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements)
{
	int error;
	cl_program program;
	cl_kernel  kernel[3];
	unsigned int kernelCount;
	
	/* Create a test program */
	program = clCreateProgramWithSource( context, 3, sample_two_kernel_program, NULL, &error);
	program = clCreateProgramWithSource( context, 2, sample_two_kernel_program, NULL, &error);
	if( program == NULL || error != CL_SUCCESS )	
	{
		log_error( "ERROR: Unable to create test program!\n" );
		return -1;
	}	

	/* Build */
	error = clBuildProgram( NULL, 1, &deviceID, NULL, NULL, NULL );
	error = clBuildProgram( program, 1, &deviceID, NULL, NULL, NULL );
	test_error( error, "Unable to build test program" );

	/* Try getting the kernel count */
	error = clCreateKernelsInProgram( NULL, 0, NULL, &kernelCount );
	error = clCreateKernelsInProgram( program, 0, NULL, &kernelCount );
	test_error( error, "Unable to get kernel count for built program" );
	if( kernelCount != 2 )
	{
		log_error( "ERROR: Returned kernel count from clCreateKernelsInProgram is incorrect! (got %d, expected 2)\n", kernelCount );
		return -1;
	}
	
	/* Try actually getting the kernels */
	error = clCreateKernelsInProgram( program, 2, kernel, NULL );
	test_error( error, "Unable to get kernels for built program" );

	error = clRetainKernel(NULL);
	error = clRetainKernel(kernel[1]);

	error = clReleaseKernel( NULL );
	error = clReleaseKernel( kernel[0] );

	error = clReleaseKernel(kernel[1]);
	error = clReleaseKernel(kernel[1]);
	
	error = clRetainProgram( NULL );
	error = clRetainProgram( program );

	error = clReleaseProgram( NULL );
	error = clReleaseProgram( program );

	error = clReleaseProgram( program );

	return 0;
}



