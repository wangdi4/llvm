//|
//| TEST: DeviceFissionTest.fissionTwoQueuesTest
//|
//| Purpose 
//| -------
//|
//| Test the ability to execute commands on two command queues for the same sub-device. 
//|
//| Method
//| ------
//|
//| 1. Create a sub device with CL_DEVICE_PARTITION_BY_COUNTS_EXT property from root device.
//| 2. Create two command queues on the sub-device. 
//| 3. Execute a kernel on the first command queue and release it. Do the same for the second command queue.
//| 4. read the results.
//|
//| Pass criteria
//| -------------
//|
//| Return true in case of SUCCESS.

#include <CL/cl.h>
#include <CL/cl_ext.h>
#include <stdio.h>
#include "FrameworkTest.h"

#define WORK_SIZE 1	
#define MAX_SOURCE_SIZE 2048

bool fission_two_queues_test()
{
	printf("---------------------------------------\n");
	printf("fission two queues test\n");
	printf("---------------------------------------\n");
	bool bResult = true;
	cl_context context=NULL;
	cl_command_queue cmd_queue[2];
	cl_device_id device=NULL;
	cl_platform_id platform=NULL;
	cl_int err;

	//init platform
	err = clGetPlatformIDs(1,&platform,NULL);
	bResult = SilentCheck(L"clGetPlatformIDs",CL_SUCCESS,err);
	if (!bResult)	return bResult;

	// init Devices (only one CPU...)
	err = clGetDeviceIDs(platform,CL_DEVICE_TYPE_DEFAULT,1,&device,NULL);
	bResult = SilentCheck(L"clGetDeviceIDs",CL_SUCCESS,err);
	if (!bResult)	return bResult;

	cl_uint numComputeUnits;
	err = clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &numComputeUnits, NULL);
	bResult = SilentCheck(L"clGetDeviceInfo(CL_DEVICE_MAX_COMPUTE_UNITS)",CL_SUCCESS,err);
	if (!bResult)	return bResult;

	if (numComputeUnits < 2)
	{
    	printf("Not enough compute units to create sub-device, tast passing vacuously\n");
		return true;
	}
	cl_device_id                     subdevice_id;
	cl_uint                          num_entries  = 1;
	cl_uint                          num_devices  = 1;
	cl_device_partition_property_ext properties[] = {CL_DEVICE_PARTITION_BY_COUNTS_EXT, 1, CL_PARTITION_BY_COUNTS_LIST_END_EXT, CL_PROPERTIES_LIST_END_EXT};

	err = clCreateSubDevicesEXT(device, properties, num_entries, &subdevice_id, &num_devices);
	bResult = SilentCheck(L"clCreateSubDevicesEXT",CL_SUCCESS,err);
	if (!bResult)	return bResult;

	//Create a context with the sub-device
	context = clCreateContext(NULL,1, &subdevice_id, NULL, NULL, &err);
	bResult = SilentCheck(L"clCreateContext",CL_SUCCESS,err);
	if (!bResult)	return bResult;

	//Create a command queue for the first device
	cmd_queue[0] = clCreateCommandQueue(context, subdevice_id, 0, &err);
	bResult = SilentCheck(L"clCreateCommandQueue - first queue",CL_SUCCESS,err);
	if (!bResult) return bResult;

	//Create a command queue for the second device
	cmd_queue[1] = clCreateCommandQueue(context, subdevice_id, 0, &err);
	bResult = SilentCheck(L"clCreateCommandQueue - second queue",CL_SUCCESS,err);
	if (!bResult) return bResult;

	char* ocl_test_program= "__kernel void writeThree(__global char *a) { a[get_global_id(0)] = 3; }";

	cl_kernel  kernel;
	cl_program program;

	program = clCreateProgramWithSource(context, 1, (const char**)&ocl_test_program, NULL, &err);
	bResult = SilentCheck(L"clCreateProgramWithSource", CL_SUCCESS, err);
	if (!bResult) return bResult;

	err = clBuildProgram(program,0,NULL,NULL,NULL,NULL);
	cl_build_status build_status;
	err |= clGetProgramBuildInfo(program, subdevice_id, CL_PROGRAM_BUILD_STATUS, MAX_SOURCE_SIZE, &build_status, NULL);	
	if (CL_SUCCESS != err || CL_BUILD_ERROR == build_status)
	{
		printf("\n build status is: %d \n",build_status);
		char err_str[MAX_SOURCE_SIZE];	// instead of dynamic allocation
		char* err_str_ptr=err_str;
		err = clGetProgramBuildInfo(program,device,CL_PROGRAM_BUILD_LOG,MAX_SOURCE_SIZE,err_str_ptr,NULL);
		if (err!=CL_SUCCESS)
			printf("Build Info error: %d \n",err);
		printf("%s \n",err_str_ptr);
		return false;
	}

	kernel = clCreateKernel(program,"writeThree",&err);
	bResult = SilentCheck(L"clCreateKernel",CL_SUCCESS,err);
	if (!bResult) return bResult;

	char input[WORK_SIZE];
	memset(input, 0, WORK_SIZE);
	cl_mem buf = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, WORK_SIZE, input, &err);
	bResult = SilentCheck(L"clCreateBuffer",CL_SUCCESS,err);
	if (!bResult) return bResult;

	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &buf);
	bResult = SilentCheck(L"clSetKernelArg",CL_SUCCESS,err);
	if (!bResult) return bResult;

	size_t globalSize = WORK_SIZE;
	err = clEnqueueNDRangeKernel(cmd_queue[0], kernel, 1, NULL, &globalSize, NULL, 0, NULL, NULL);
	bResult = SilentCheck(L"clEnqueueNDRangeKernel",CL_SUCCESS,err);
	if (!bResult) return bResult;

	err = clEnqueueReadBuffer(cmd_queue[0], buf, CL_TRUE, 0, WORK_SIZE, input, 0, NULL, NULL);
	bResult = SilentCheck(L"clEnqueueReadBuffer",CL_SUCCESS,err);
	if (!bResult) return bResult;

	for (size_t i = 0; i < WORK_SIZE; ++i)
	{
		if (3 != input[i])
		{
			return false;
		}
	}
	//Now do the same for the second queue after releasing the first one
	err = clReleaseCommandQueue(cmd_queue[0]);
	bResult = SilentCheck(L"clReleaseCommandQueue [0]",CL_SUCCESS,err);

	memset(input, 0, WORK_SIZE);
	err = clEnqueueWriteBuffer(cmd_queue[1], buf, CL_FALSE, 0, WORK_SIZE, input, 0, NULL, NULL);
	bResult = SilentCheck(L"clEnqueueReadBuffer",CL_SUCCESS,err);
	if (!bResult) return bResult;

	err = clEnqueueNDRangeKernel(cmd_queue[1], kernel, 1, NULL, &globalSize, NULL, 0, NULL, NULL);
	bResult = SilentCheck(L"clEnqueueNDRangeKernel",CL_SUCCESS,err);
	if (!bResult) return bResult;

	err = clEnqueueReadBuffer(cmd_queue[1], buf, CL_TRUE, 0, WORK_SIZE, input, 0, NULL, NULL);
	bResult = SilentCheck(L"clEnqueueReadBuffer",CL_SUCCESS,err);
	if (!bResult) return bResult;
	for (size_t i = 0; i < WORK_SIZE; ++i)
	{
		if (3 != input[i])
		{
			return false;
		}
	}

	err      = clReleaseCommandQueue(cmd_queue[1]);
	bResult &= SilentCheck(L"clReleaseCommandQueue [1]",CL_SUCCESS,err);
	err      = clReleaseProgram(program);
	bResult &= SilentCheck(L"clReleaseProgram",CL_SUCCESS,err);
	err      = clReleaseKernel(kernel);
	bResult &= SilentCheck(L"clReleaseKernel",CL_SUCCESS,err);
	err      = clReleaseMemObject(buf);
	bResult &= SilentCheck(L"clReleaseMemObject",CL_SUCCESS,err);
	err      = clReleaseContext(context);
	bResult &= SilentCheck(L"clReleaseContext",CL_SUCCESS,err);
	err      = clReleaseDeviceEXT(subdevice_id);
	bResult &= SilentCheck(L"clReleaseDeviceEXT",CL_SUCCESS,err);

	if (bResult)
	{
		printf("---------------------------------------\n");
	    printf("fission two queues test passed\n");
	    printf("---------------------------------------\n");
	}

	return bResult;
}
