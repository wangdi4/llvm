/******************************************************************
 //
 //  OpenCL Conformance Tests
 //
 //  Copyright: (c) 2008-2013 by Apple Inc. All Rights Reserved.
 //
 ******************************************************************/

#include "testBase.h"
#include "harness/testHarness.h"


const char *sample_single_kernel[] = {
    "__kernel void sample_test(__global float *src, __global int *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = (int)src[tid];\n"
    "\n"
    "}\n" };

size_t sample_single_kernel_lengths[1];

const char *sample_two_kernels[] = {
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

size_t sample_two_kernel_lengths[2];

const char *sample_two_kernels_in_1[] = {
    "__kernel void sample_test(__global float *src, __global int *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = (int)src[tid];\n"
    "\n"
    "}\n"
    "__kernel void sample_test2(__global int *src, __global float *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = (float)src[tid];\n"
    "\n"
    "}\n" };

size_t sample_two_kernels_in_1_lengths[1];


const char *repeate_test_kernel =
"__kernel void test_kernel(__global int *src, __global int *dst)\n"
"{\n"
" dst[get_global_id(0)] = src[get_global_id(0)]+1;\n"
"}\n";


static const char *single_task_kernel[] = {
    "__kernel void sample_test(__global int *dst, int count)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    for( int i = 0; i < count; i++ )\n"
    "   dst[i] = tid + i;\n"
    "\n"
    "}\n" };

int test_enqueue_task(cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements)
{
    int error;
    clProgramWrapper program;
    clKernelWrapper kernel;
    clMemWrapper output;
    cl_int count;


    if( create_single_kernel_helper( context, &program, &kernel, 1, single_task_kernel, "sample_test" ) )
        return -1;

    // Create args
    count = 100;
    output = clCreateBuffer( context, (cl_mem_flags)(CL_MEM_READ_WRITE), sizeof( cl_int ) * count, NULL, &error );
    test_error( error, "Unable to create output buffer" );

    error = clSetKernelArg( kernel, 0, sizeof( cl_mem ), &output );
    test_error( error, "Unable to set kernel argument" );
    error = clSetKernelArg( kernel, 1, sizeof( cl_int ), &count );
    test_error( error, "Unable to set kernel argument" );

    // Run task
    error = clEnqueueTask( queue, NULL, 0, NULL, NULL );
    error = clEnqueueTask( queue, kernel, 0, NULL, NULL );
    test_error( error, "Unable to run task" );

    // Read results
    cl_int *results = (cl_int*)malloc(sizeof(cl_int)*count);
    error = clEnqueueReadBuffer( queue, output, CL_TRUE, 0, sizeof( cl_int ) * count, results, 0, NULL, NULL );
    test_error( error, "Unable to read results" );

    // Validate
    for( cl_int i = 0; i < count; i++ )
    {
        if( results[ i ] != i )
        {
            log_error( "ERROR: Task result value %d did not validate! Expected %d, got %d\n", (int)i, (int)i, (int)results[ i ] );
            free(results);
            return -1;
        }
    }

    /* All done */
    free(results);
    return 0;
}



#define TEST_SIZE 1000
int test_repeated_setup_cleanup(cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements)
{
    cl_context local_context;
    cl_command_queue local_queue;
    cl_program local_program;
    cl_kernel local_kernel;
    cl_mem local_mem_in, local_mem_out;
    cl_event local_event;
    size_t global_dim[3];
    int i, j, error;
    global_dim[0] = TEST_SIZE;
    global_dim[1] = 1; global_dim[2] = 1;
    cl_int *inData, *outData;
    cl_int status;

    inData = (cl_int*)malloc(sizeof(cl_int)*TEST_SIZE);
    outData = (cl_int*)malloc(sizeof(cl_int)*TEST_SIZE);
    for (i=0; i<TEST_SIZE; i++) {
        inData[i] = i;
    }


    for (i=0; i<1; i++) {
        memset(outData, 0, sizeof(cl_int)*TEST_SIZE);

        local_context = clCreateContext(NULL, 1, NULL, notify_callback, NULL, &error);
        local_context = clCreateContext(NULL, 1, &deviceID, notify_callback, NULL, &error);
        test_error( error, "clCreateContext failed");

        local_queue = clCreateCommandQueue(local_context, deviceID, 0, &error);
        test_error( error, "clCreateCommandQueue failed");

        local_program = clCreateProgramWithSource(local_context, 1, &repeate_test_kernel, NULL, &error);
        test_error( error, "clCreateProgramWithSource failed");

        error = clBuildProgram(local_program, 0, NULL, NULL, NULL, NULL);
        test_error( error, "clBuildProgram failed");

        local_kernel = clCreateKernel(NULL, "test_kernel", &error);
        local_kernel = clCreateKernel(local_program, "test_kernel", &error);
        test_error( error, "clCreateKernel failed");

        local_mem_in = clCreateBuffer(local_context, CL_MEM_READ_ONLY, TEST_SIZE*sizeof(cl_int), NULL, &error);
        test_error( error, "clCreateBuffer failed");

        local_mem_out = clCreateBuffer(local_context, CL_MEM_WRITE_ONLY, TEST_SIZE*sizeof(cl_int), NULL, &error);
        test_error( error, "clCreateBuffer failed");

        error = clEnqueueWriteBuffer(local_queue, NULL, CL_TRUE, 0, TEST_SIZE*sizeof(cl_int), inData, 0, NULL, NULL);
        error = clEnqueueWriteBuffer(local_queue, local_mem_in, CL_TRUE, 0, TEST_SIZE*sizeof(cl_int), inData, 0, NULL, NULL);
        test_error( error, "clEnqueueWriteBuffer failed");

        error = clEnqueueWriteBuffer(local_queue, local_mem_out, CL_TRUE, 0, TEST_SIZE*sizeof(cl_int), outData, 0, NULL, NULL);
        test_error( error, "clEnqueueWriteBuffer failed");

        error = clSetKernelArg(local_kernel, 0, sizeof(local_mem_in), &local_mem_in);
        test_error( error, "clSetKernelArg failed");

        error = clSetKernelArg(local_kernel, 1, sizeof(local_mem_out), &local_mem_out);
        test_error( error, "clSetKernelArg failed");

        error = clEnqueueNDRangeKernel(local_queue, local_kernel, 1, NULL, global_dim, NULL, 0, NULL, &local_event);
        test_error( error, "clEnqueueNDRangeKernel failed");

        error = clWaitForEvents(2, &local_event);
        error = clWaitForEvents(1, &local_event);
        test_error( error, "clWaitForEvents failed");

        error = clGetEventInfo(NULL, CL_EVENT_COMMAND_EXECUTION_STATUS, sizeof(status), &status, NULL);
        error = clGetEventInfo(local_event, CL_EVENT_COMMAND_EXECUTION_STATUS, sizeof(status), &status, NULL);
        test_error( error, "clGetEventInfo failed");

        if (status != CL_COMPLETE) {
            log_error( "Kernel execution not complete: status %d.\n", status);
            free(inData);
            free(outData);
            return -1;
        }

        error = clEnqueueReadBuffer(local_queue, local_mem_out, CL_TRUE, 0, TEST_SIZE*sizeof(cl_int), outData, 0, NULL, NULL);
        test_error( error, "clEnqueueReadBuffer failed");

        error = clReleaseEvent(NULL);
        error = clReleaseEvent(local_event);
        error = clReleaseMemObject(NULL);
        error = clReleaseMemObject(local_mem_in);
        clReleaseMemObject(local_mem_out);
        clReleaseKernel(local_kernel);
        clReleaseProgram(local_program);
        clReleaseCommandQueue(local_queue);
        clReleaseContext(local_context);

        for (j=0; j<TEST_SIZE; j++) {
            if (outData[j] != inData[j] + 1) {
                log_error("Results failed to validate at iteration %d. %d != %d.\n", i, outData[j], inData[j] + 1);
                free(inData);
                free(outData);
                return -1;
            }
        }
    }

    free(inData);
    free(outData);

    return 0;
}
