//|
//| TEST: DeviceFissionTest.fissionNumaTest
//|
//| Purpose 
//| -------
//|
//| Test the device fission with NUMA property.
//|
//| Method
//| ------
//|
//| 1. Check that the device support NUMA partition.
//| 2. Create sub devices with CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN_EXT, CL_DEVICE_AFFINITY_DOMAIN_NUMA property from root device.
//|
//| Pass criteria
//| -------------
//|
//| Return true in case of SUCCESS.

#include <CL/cl.h>
#include "cl_types.h"
#include <stdio.h>
#include "FrameworkTest.h"

bool executeKernel(cl_device_id device_id)
{
    static const size_t FISSION_NUMA_EXECUTION_GLOBAL_SIZE  = 16384;

    const char *ocl_test_program[] = {\
        "__kernel void copy (__global float* a, __global float* b)"\
        "{"\
        "int tid = get_global_id(0);"\
        "b[tid] = a[tid];"\
        "}"
    };

    cl_int       iRet;
    bool         bResult = true;
    cl_context   context;

    // create context
    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &iRet);
    bResult &= SilentCheck(L"clCreateContext",CL_SUCCESS, iRet);
    if (!bResult)
    {
        return bResult;
    }

    // create program with source
    cl_program program = clCreateProgramWithSource(context, 1, (const char**)&ocl_test_program, NULL, &iRet);
    bResult &= SilentCheck(L"clCreateProgramWithSource", CL_SUCCESS, iRet);

    iRet = clBuildProgram(program, NULL, NULL, NULL, NULL, NULL);
    bResult &= SilentCheck(L"clBuildProgram", CL_SUCCESS, iRet);

    //
    // From here down it is the program execution implementation
    //
    cl_float src[FISSION_NUMA_EXECUTION_GLOBAL_SIZE]; 
    cl_float dst[FISSION_NUMA_EXECUTION_GLOBAL_SIZE];

    cl_float init = 0.1f;
    for(int j = 0; j < FISSION_NUMA_EXECUTION_GLOBAL_SIZE; j++)
    {
        src[j] = init;
        dst[j] = 0.0f;
        init += 0.1f;
    }

    //
    // Create an in-order immediate queue
    //
    cl_command_queue queue1 = clCreateCommandQueue (context, device_id, 0, &iRet);
    bResult &= SilentCheck(L"clCreateCommandQueue", CL_SUCCESS, iRet);


    //
    // Create Kernel
    //
    cl_kernel kernel1 = clCreateKernel(program, "copy", &iRet);
    bResult &= SilentCheck(L"clCreateKernel - copy", CL_SUCCESS, iRet);

    //
    // Create buffers
    //
    size_t size = sizeof(cl_float);

    cl_mem buffer_src = clCreateBuffer(context, CL_MEM_READ_ONLY, size * FISSION_NUMA_EXECUTION_GLOBAL_SIZE, NULL, &iRet);
    bResult &= SilentCheck(L"clCreateBuffer - src", CL_SUCCESS, iRet);

    cl_mem buffer_dst = clCreateBuffer(context, CL_MEM_READ_WRITE, size * FISSION_NUMA_EXECUTION_GLOBAL_SIZE, NULL, &iRet);
    bResult &= SilentCheck(L"clCreateBuffer - dst", CL_SUCCESS, iRet);

    //
    // Set arguments
    //
    iRet = clSetKernelArg(kernel1, 0, sizeof(cl_mem), &buffer_src);
    bResult &= SilentCheck(L"clSetKernelArg - buffer_src", CL_SUCCESS, iRet);

    iRet = clSetKernelArg(kernel1, 1, sizeof(cl_mem), &buffer_dst);
    bResult &= SilentCheck(L"clSetKernelArg - buffer_dst", CL_SUCCESS, iRet);

    //
    // Execute commands - Write buffers
    //
    iRet = clEnqueueWriteBuffer (queue1, buffer_src, false, 0, size* FISSION_NUMA_EXECUTION_GLOBAL_SIZE, src, 0, NULL, NULL);
    bResult &= SilentCheck(L"clEnqueueWriteBuffer - src", CL_SUCCESS, iRet);

    iRet = clEnqueueWriteBuffer (queue1, buffer_dst, false, 0, size* FISSION_NUMA_EXECUTION_GLOBAL_SIZE, dst, 0, NULL, NULL);
    bResult &= SilentCheck(L"clEnqueueWriteBuffer - dst", CL_SUCCESS, iRet);
    //
    // Execute kernel
    //
    size_t global_work_size[1] = { FISSION_NUMA_EXECUTION_GLOBAL_SIZE };

    cl_event evt;
    iRet = clEnqueueNDRangeKernel(queue1, kernel1, 1, NULL, global_work_size, NULL, 0, NULL, &evt);
    bResult &= SilentCheck(L"clEnqueueNDRangeKernel", CL_SUCCESS, iRet);

	iRet = clEnqueueReadBuffer (queue1, buffer_dst, CL_TRUE,  0, size*FISSION_NUMA_EXECUTION_GLOBAL_SIZE, dst, 0, NULL, NULL);
    bResult &= SilentCheck(L"clEnqueueReadBuffer", CL_SUCCESS, iRet);    

    for (unsigned int i = 0; i < FISSION_NUMA_EXECUTION_GLOBAL_SIZE; ++i)
    {
        if (dst[i] != src[i])
        {
            printf("Validation failed for index %u\n", i);
            bResult = false;
            break;
        }
    }
    //
    // Release objects
    //
    iRet = clReleaseEvent(evt);
    bResult &= SilentCheck(L"clReleaseEvent", CL_SUCCESS, iRet);

    iRet = clReleaseMemObject(buffer_dst);
    bResult &= SilentCheck(L"clReleaseBuffer - buffer_dst", CL_SUCCESS, iRet);

    iRet = clReleaseMemObject(buffer_src);
    bResult &= SilentCheck(L"clReleaseBuffer - buffer_src", CL_SUCCESS, iRet);


    iRet = clReleaseKernel(kernel1);
    bResult &= SilentCheck(L"clReleaseKernel - kernel1", CL_SUCCESS, iRet);

    iRet = clReleaseProgram(program);
    bResult &= SilentCheck(L"clReleaseProgram - program", CL_SUCCESS, iRet);

    iRet = clReleaseCommandQueue(queue1);
    bResult &= SilentCheck(L"clReleaseCommandQueue - queue1", CL_SUCCESS, iRet);

    iRet = clReleaseContext(context);
    bResult &= SilentCheck(L"clReleaseContext - context", CL_SUCCESS, iRet);

    return bResult;
}

bool fission_numa_test(){
	printf("---------------------------------------\n");
	printf("fission NUMA test\n");
	printf("---------------------------------------\n");
	bool bResult = true;
	cl_device_id device=NULL;
	cl_int err;
	cl_platform_id platform=NULL;

	//init platform
	err = clGetPlatformIDs(1,&platform,NULL);
	bResult = SilentCheck(L"clGetPlatformIDs",CL_SUCCESS,err);
	if (!bResult)	return bResult;

	// init Devices (only one CPU...)
	err = clGetDeviceIDs(platform,CL_DEVICE_TYPE_DEFAULT,1,&device,NULL);
	bResult = SilentCheck(L"clGetDeviceIDs",CL_SUCCESS,err);
	if (!bResult)	return bResult;

	//check that we support NUMA
	size_t actual_size;
	cl_device_partition_property prop[20];
	err = clGetDeviceInfo(device, CL_DEVICE_PARTITION_AFFINITY_DOMAIN, 20*sizeof(cl_device_partition_property), prop, &actual_size);
	bResult = SilentCheck(L"clGetDeviceInfo for selector CL_DEVICE_PARTITION_AFFINITY_DOMAIN",CL_SUCCESS,err);
	if (!bResult)	return bResult;

	if (0 < actual_size && CL_DEVICE_AFFINITY_DOMAIN_NUMA == prop[0])
	{
	
		cl_uint num_entries = 100;
		cl_device_id out_devices[100];
		cl_uint num_devices = 2;
		cl_device_partition_property properties[] = {CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN, CL_DEVICE_AFFINITY_DOMAIN_NUMA, 0};
		err = clCreateSubDevices(device, properties, num_entries, out_devices, &num_devices);
		bResult = SilentCheck(L"clCreateSubDevices",CL_SUCCESS,err);
		if (!bResult)	return bResult;
		for (size_t i = 0; i < 2; i++)
		{
			bResult &= executeKernel(out_devices[i]);
		}
		for (size_t i = 0; i < num_devices; i++)
		{
			clReleaseDevice(out_devices[i]);
		}
		if (!bResult)
		{
			return bResult;
		}
	}
	//include the case we don't support NUMA
	printf("\n---------------------------------------\n");
	printf("fission NUMA test succeeded!\n");
	printf("---------------------------------------\n");
	
	return true;
}