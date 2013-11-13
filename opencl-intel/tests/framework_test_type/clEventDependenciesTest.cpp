//|
//| TEST: FrameworkTest.eventDependencies
//|
//| Purpose 
//| -------
//|
//| Test regression for a bug uncovered by the events subtest of conformance
//|
//| Method
//| ------
//|
//| 1. Create an OOO queue and two kernels
//| 2. Enqueue a long chain of alternating dependent kernels
//| 3. read the results.
//|
//| Pass criteria
//| -------------
//|
//| No crash.
//| Return true in case of SUCCESS.

#include <CL/cl.h>
#include "FrameworkTest.h"

#define TEST_SIZE 100
#define TEST_COUNT 100000

extern cl_device_type gDeviceType;

bool EventDependenciesTest()
{
    printf("---------------------------------------\n");
    printf("event dependencies test\n");
    printf("---------------------------------------\n");
    bool bResult = true;
    cl_context context;
    cl_command_queue cmd_queue;
    cl_device_id device;
    cl_platform_id platform;
    cl_int err;
    cl_command_queue_properties queue_properties;

    //init platform
    err = clGetPlatformIDs(1,&platform,NULL);
    bResult = SilentCheck(L"clGetPlatformIDs",CL_SUCCESS,err);
    if (!bResult)    return bResult;

    // init device
    err     = clGetDeviceIDs(platform,gDeviceType,1,&device,NULL);
    bResult = SilentCheck(L"clGetDeviceIDs",CL_SUCCESS,err);
    if (!bResult)    return bResult;

    // Check for OOO support
    err     = clGetDeviceInfo(device, CL_DEVICE_QUEUE_ON_HOST_PROPERTIES, sizeof(cl_command_queue_properties), &queue_properties, NULL);
    bResult = SilentCheck(L"clGetDeviceInfo(CL_DEVICE_QUEUE_ON_HOST_PROPERTIES)",CL_SUCCESS,err);
    if (!bResult)    return bResult;

    if (!(queue_properties & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE))
    {
        printf("OOO not supported, test passing vacuously\n");
        return true;
    }

    // No need for any other properties
    queue_properties = CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE;

    //Create a context 
    context = clCreateContext(NULL,1, &device, NULL, NULL, &err);
    bResult = SilentCheck(L"clCreateContext",CL_SUCCESS,err);
    if (!bResult)    return bResult;

    //Create a command queue
    cmd_queue = clCreateCommandQueue(context, device, queue_properties, &err);
    bResult = SilentCheck(L"clCreateCommandQueue",CL_SUCCESS,err);
    if (!bResult)    return bResult;

    const char* ocl_test_program= "__kernel void k1(__global int* a, int b) {}";

    cl_kernel  k1;
    cl_event   events[TEST_COUNT + 1];
    cl_program program;

    int i, loop_count, event_count, expected_value;
    int max_count = TEST_SIZE;
    
    // Create a buffer
    cl_mem data = clCreateBuffer(context, CL_MEM_READ_WRITE, TEST_SIZE*sizeof(cl_int), NULL, &err);
    bResult     = SilentCheck(L"clCreateBuffer",CL_SUCCESS,err);
    if (!bResult)    return bResult;
    
    // Zero init the values
    cl_int *values = new cl_int[TEST_SIZE];
    for (i = 0; i < (int)TEST_SIZE; i++)
        values[i] = 0;
    expected_value = 0;
    
    // Build the kernels
    program = clCreateProgramWithSource(context, 1, &ocl_test_program, NULL, &err);
    bResult = SilentCheck(L"clCreateProgramWithSource",CL_SUCCESS,err);
    if (!bResult)    return bResult;

    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    bResult = SilentCheck(L"clBuildProgram",CL_SUCCESS,err);
    if (!bResult)    return bResult;
    
    k1      = clCreateKernel(program, "k1", &err);
    bResult = SilentCheck(L"clCreateKernel: k1",CL_SUCCESS,err);
    if (!bResult)    return bResult;
        
    err     = clSetKernelArg(k1, 0, sizeof(data), &data);
    err    |= clSetKernelArg(k1, 1, sizeof(max_count), &max_count);
    bResult = SilentCheck(L"clSetKernelArg: k1",CL_SUCCESS,err);
    if (!bResult)    return bResult;
    event_count = 0;

    err = clEnqueueWriteBuffer(cmd_queue, data, CL_FALSE, 0, TEST_SIZE*sizeof(cl_int), values, 0, NULL, events + event_count);
    bResult = SilentCheck(L"clEnqueueWriteBuffer",CL_SUCCESS,err);
    if (!bResult)    return bResult;

    // check that we issue an error in case that event refers to an element in event_wait_list
    err = clEnqueueWriteBuffer(cmd_queue, data, CL_FALSE, 0, TEST_SIZE * sizeof(cl_int), values, 1, events + event_count, events + event_count);
    bResult = SilentCheck(L"clEnqueueWriteBuffer", CL_INVALID_EVENT, err);
    if (!bResult)
        return bResult;

    expected_value = 1;
    size_t global_size[1] = {TEST_SIZE};

    printf("Starting iterations....\n");fflush(0);
    for (loop_count=0; loop_count<TEST_COUNT; loop_count++) 
    {
        // Execute kernel 1
        event_count++;
        err = clEnqueueNDRangeKernel(cmd_queue, k1, 1, NULL, global_size, NULL, 1, events + event_count - 1, events + event_count);
        bResult = SilentCheck(L"clEnqueueNDRangeKernel: k1",CL_SUCCESS,err);
        if (!bResult)    return bResult;
        if ( (loop_count % 10000) == 0 ) {printf(".");fflush(0);}
    }
    printf("\nCompleted\n");fflush(0);

    err = clEnqueueReadBuffer(cmd_queue, data, CL_TRUE, 0, TEST_SIZE*sizeof(cl_int), values, 1, events + event_count, NULL);
    bResult = SilentCheck(L"clEnqueueReadBuffer",CL_SUCCESS,err);
    if (!bResult)    return bResult;

    printf("----------> Test completed <----------------\n");fflush(0);
    delete[] values;
    for (i = 0; i<TEST_COUNT+1; i++) 
    {
        clReleaseEvent(events[i]);
    }
    clReleaseKernel(k1);
    clReleaseProgram(program);
    clReleaseMemObject(data);
    clReleaseCommandQueue(cmd_queue);
    clReleaseContext(context);
    return true;
}
