//|
//| TEST: DeviceFissionTest.fissionLogicTest
//|
//| Purpose 
//| -------
//|
//| Test logic usage of a device fission.
//|
//| Method
//| ------
//|
//| 1. Create sub devices with CL_DEVICE_PARTITION_BY_COUNTS property from root device.
//| 2. Create sub devices with CL_DEVICE_PARTITION_EQUALLY property from the first sub device.
//| 3. Enqueue execution of 2 different kernels: first kernel on one sub device,
//|    second kernel on the rest sub devices.
//| 4. read and validate the results.
//|
//| Pass criteria
//| -------------
//|
//| The data was correctly copied into the output buffer in each kernel.
//| Return true in case of SUCCESS.

#include <CL/cl.h>
#include "cl_types.h"
#include <stdio.h>
#include "FrameworkTest.h"

#define WORK_SIZE 1	
#define MAX_SOURCE_SIZE 2048


bool run_kernel1(cl_context& context,cl_device_id& device,cl_command_queue& cmd_queue, char* prog, int *out){
	cl_int err;
	cl_kernel kernel;
	cl_program program;
	bool res;

	program = clCreateProgramWithSource(context, 1, (const char**)&prog, NULL, &err);
	res = SilentCheck(L"clCreateProgramWithSource",CL_SUCCESS,err);
	if (!res) return res;

	err = clBuildProgram(program,0,NULL,NULL,NULL,NULL);
	cl_build_status build_status;
	err |= clGetProgramBuildInfo(program,device,CL_PROGRAM_BUILD_STATUS,MAX_SOURCE_SIZE,&build_status,NULL);	
	if (CL_SUCCESS != err || CL_BUILD_ERROR == build_status)
	{
		printf("\n build status is: %d \n",build_status);
		char err_str[MAX_SOURCE_SIZE];	// instead of dynamic allocation
		char* err_str_ptr=err_str;
		err = clGetProgramBuildInfo(program,device,CL_PROGRAM_BUILD_LOG,MAX_SOURCE_SIZE,err_str_ptr,NULL);
		if (err!=CL_SUCCESS)
			printf("Build Info error: %d \n",err);
		printf("%s \n",err_str_ptr);
		return res;
	}

	kernel = clCreateKernel(program,"fissionLogic1Test",&err);
	res = SilentCheck(L"clCreateKernel",CL_SUCCESS,err);
	if (!res) 
	{
		clReleaseProgram(program);	
		return res;
	}

	cl_mem buff = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int), NULL, &err); 
	res = SilentCheck(L"clCreateBuffer",CL_SUCCESS,err);
	if (!res) 
	{
		clReleaseKernel(kernel);
		clReleaseProgram(program);	
		return res;
	}

	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*) &buff);
	res = SilentCheck(L"clSetKernelArg",CL_SUCCESS,err);
	if (!res) 
	{
		clReleaseKernel(kernel);
		clReleaseProgram(program);	
		return res;
	}

	size_t global_work_size[1];
	global_work_size[0] = WORK_SIZE;

	err = clEnqueueNDRangeKernel(cmd_queue,kernel,1,NULL,global_work_size,NULL,0,NULL,NULL);
	res = SilentCheck(L"clEnqueueNDRangeKernel",CL_SUCCESS,err);
	if (!res) 
	{
		clReleaseKernel(kernel);
		clReleaseProgram(program);	
		return res;
	}
	*out = 0;

	err = clEnqueueReadBuffer(cmd_queue, buff, CL_TRUE, 0, sizeof(int), out, 0, NULL, NULL);
	err += (*out == 7);
	clFinish(cmd_queue);
	clReleaseKernel(kernel);
	clReleaseProgram(program);
	return ((CL_SUCCESS == err)? true : false);
}

bool run_kernel2(cl_context& context,cl_device_id& device,cl_command_queue& cmd_queue, char* prog, int *out){
	cl_int err;
	cl_kernel kernel;
	cl_program program;
	bool res;

	program = clCreateProgramWithSource(context, 1, (const char**)&prog, NULL, &err);
	res = SilentCheck(L"clCreateProgramWithSource",CL_SUCCESS,err);
	if (!res) return res;

	err = clBuildProgram(program,0,NULL,NULL,NULL,NULL);
	cl_build_status build_status;
	err |= clGetProgramBuildInfo(program,device,CL_PROGRAM_BUILD_STATUS,MAX_SOURCE_SIZE,&build_status,NULL);	
	if (CL_SUCCESS != err || CL_BUILD_ERROR == build_status)
	{
		printf("\n build status is: %d \n",build_status);
		char err_str[MAX_SOURCE_SIZE];	// instead of dynamic allocation
		char* err_str_ptr=err_str;
		err = clGetProgramBuildInfo(program,device,CL_PROGRAM_BUILD_LOG,MAX_SOURCE_SIZE,err_str_ptr,NULL);
		if (err!=CL_SUCCESS)
			printf("Build Info error: %d \n",err);
		printf("%s \n",err_str_ptr);
		return res;
	}

	kernel = clCreateKernel(program,"fissionLogic2Test",&err);
	res = SilentCheck(L"clCreateKernel",CL_SUCCESS,err);
	if (!res) 
	{
		clReleaseProgram(program);	
		return res;
	}

	cl_mem buff = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int), NULL, &err); 
	res = SilentCheck(L"clCreateBuffer",CL_SUCCESS,err);
	if (!res) 
	{
		clReleaseKernel(kernel);
		clReleaseProgram(program);	
		return res;
	}

	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*) &buff);
	res = SilentCheck(L"clSetKernelArg",CL_SUCCESS,err);
	if (!res) 
	{
		clReleaseKernel(kernel);
		clReleaseProgram(program);	
		return res;
	}

	size_t global_work_size[1];
	global_work_size[0] = WORK_SIZE;

	err = clEnqueueNDRangeKernel(cmd_queue,kernel,1,NULL,global_work_size,NULL,0,NULL,NULL);
	res = SilentCheck(L"clEnqueueNDRangeKernel",CL_SUCCESS,err);
	if (!res) 
	{
		clReleaseKernel(kernel);
		clReleaseProgram(program);	
		return res;
	}
	*out = 0;

	err = clEnqueueReadBuffer(cmd_queue, buff, CL_TRUE, 0, sizeof(int), out, 0, NULL, NULL);
	err += (*out == 3);
	clFinish(cmd_queue);
	clReleaseKernel(kernel);
	clReleaseProgram(program);
	return ((CL_SUCCESS == err)? true : false);
}
#include <iostream>
using namespace std;
bool fission_logic_test(){
#if 0
//fission start
    cl_int status = CL_SUCCESS;

    char deviceExtensions[512];

	cl_device_id m_device_id;
	cl_platform_id m_platform = 0;
	clGetPlatformIDs(1, &m_platform, NULL);
	clGetDeviceIDs(m_platform, CL_DEVICE_TYPE_CPU, 1, &m_device_id, NULL);

    status = clGetDeviceInfo(m_device_id, CL_DEVICE_EXTENSIONS, 512, deviceExtensions, NULL);
	if(!SilentCheck(L"clGetDeviceInfo 2 failed.(CL_DEVICE_EXTENSIONS)", CL_SUCCESS, status)) return false;

    // Check if device fission is supported
    if(!strstr(deviceExtensions, "cl_ext_device_fission"))
    {
        cout << "Device does not support cl_ext_device_fission extension!" << endl;
        return false;
    }

	cl_device_partition_property partitionPrtyEq[3] =
    {
        CL_DEVICE_PARTITION_EQUALLY, 1, 0
    };
	cl_device_partition_property partitionPrtyAff[3] =
    {
        CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN_EXT, CL_AFFINITY_DOMAIN_NEXT_FISSIONABLE_EXT, 0
    };

	bool affinityFissionFlag = false;
	std::vector<cl_device_id> subDevices;
	std::vector<cl_command_queue> subCmdQueues;

	size_t numSubDevices = 1;

	status = clCreateSubDevices(m_device_id, partitionPrtyEq, 0, NULL, &numSubDevices);
	if(!SilentCheck(L"clCreateSubDevices failed.", CL_SUCCESS, status))
        return false;

	subDevices.resize(numSubDevices);
    //status = pfn_clCreateSubDevices(m_device_id, partitionPrtyEq, numSubDevices, &subDevices.front(), &numSubDevices); //NULL);
	status = clCreateSubDevices(m_device_id, partitionPrtyEq, numSubDevices, &subDevices.front(), &numSubDevices); //NULL);
    if(!SilentCheck(L"clCreateSubDevices failed.", CL_SUCCESS, status))
        return false;

    cl_context_properties cps[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)m_platform, 0 };
	// Create context for sub-devices, Setup sub-device cmd queues
    cl_context subContext = clCreateContext(cps, numSubDevices, &subDevices.front(), NULL, NULL, &status);
	if(!SilentCheck(L"clCreateContext failed.", CL_SUCCESS, status))
        return false;

    for(cl_uint i = 0; i < numSubDevices; i++)
    {
        // Create command queue 
        subCmdQueues.push_back( clCreateCommandQueue(subContext, subDevices[i], 0, &status) );
		if(!SilentCheck(L"clCreateCommandQueue failed.", CL_SUCCESS, status))
			return false;
    }
//fission end

	return true;
#else
	printf("---------------------------------------\n");
	printf("fission logic test\n");
	printf("---------------------------------------\n");
	bool bResult = true;
	cl_context context=NULL;
	cl_command_queue cmd_queue[10]={NULL};
	cl_device_id device=NULL;
	cl_program program=NULL;
	cl_kernel kernel=NULL;
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

	cl_uint num_entries = 100;
	cl_device_id out_devices[100];
	cl_uint num_devices = 2;
	cl_uint tot_num_devices = 0;
	cl_device_partition_property properties1[] = {CL_DEVICE_PARTITION_BY_COUNTS, 4, 2, CL_DEVICE_PARTITION_BY_COUNTS_LIST_END, 0};
	cl_device_partition_property properties2[] = {CL_DEVICE_PARTITION_EQUALLY, 2, 0};

	err = clCreateSubDevices(device, properties1, num_entries, out_devices, &num_devices);
	bResult = SilentCheck(L"clCreateSubDevices",CL_SUCCESS,err);
	if (!bResult)	return bResult;

	tot_num_devices+= num_devices;

	err = clCreateSubDevices(out_devices[0], properties2, num_entries, (out_devices + num_devices), &num_devices);
	bResult = SilentCheck(L"clCreateSubDevices",CL_SUCCESS,err);
	if (!bResult)	return bResult;

	tot_num_devices+= num_devices;

	//init context
	context = clCreateContext(NULL,tot_num_devices ,out_devices ,NULL,NULL,&err);
	bResult = SilentCheck(L"clCreateContext",CL_SUCCESS,err);
	if (!bResult)	return bResult;

	//init Command Queue
	for (size_t i = 0; i < tot_num_devices; i++)
	{
		cmd_queue[i] = clCreateCommandQueue(context,out_devices[i],0,&err);
		bResult = SilentCheck(L"clCreateCommandQueue",CL_SUCCESS,err);
		if (!bResult){
			clReleaseContext(context);
			return bResult;
		}
	}	

	char* ocl_test1_program= {\
		"__kernel void fissionLogic1Test(__global int *d)\
		{d = 7;}"};

	char* ocl_test2_program= {\
		"__kernel void fissionLogic2Test(__global int *d)\
		{d = 3;}"};

	int res1, res2;

	bResult |= run_kernel1(context, out_devices[tot_num_devices-num_devices], cmd_queue[tot_num_devices-num_devices], ocl_test1_program, &res1);

	for (size_t i = (tot_num_devices-num_devices + 1); i < tot_num_devices; i++)
	{
		bResult |= run_kernel2(context, out_devices[i], cmd_queue[i], ocl_test2_program, &res2);
	}
	const char* res = ((bResult == true) ? "succeed" : "failed");
	printf("\n---------------------------------------\n");
	printf("fission logic test %s!\n", res);
	printf("---------------------------------------\n");

	for (size_t i = 0; i < num_devices; i++)
	{
		clFinish(cmd_queue[i]);
		clReleaseCommandQueue(cmd_queue[i]);
	}
	clReleaseContext(context);
	for (size_t i = 0; i < num_devices; i++)
	{
		clReleaseDevice(out_devices[i]);
	}
	return bResult;;
#endif
}