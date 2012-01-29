//|
//| TEST: API test
//|
//| Purpose 
//| -------
//|
//| Test framework API, according to 1.1 spec.
//|
//| Method
//| ------
//|
//| Pass criteria
//| -------------
//|
//| Return true in case of SUCCESS.

#include <CL/cl.h>
#include "cl_types.h"
#include <stdio.h>
#include "FrameworkTest.h"

#define WORK_SIZE 1	
#define MAX_SOURCE_SIZE 2048

bool run_kernel(cl_context& context,cl_device_id& device,cl_command_queue& cmd_queue, char* prog,char *out){
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

	kernel = clCreateKernel(program,"apiTest",&err);
	res = SilentCheck(L"clCreateKernel",CL_SUCCESS,err);
	if (!res) 
	{
		clReleaseProgram(program);	
		return res;
	}

	// CSSD100007132
	cl_ulong memSize;
	size_t	wgSize[3];
	size_t	retSize;
	err = clGetKernelWorkGroupInfo(kernel, NULL, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &wgSize[0], &retSize);
	res &= Check(L"clGetKernelWorkGroupInfo",CL_SUCCESS,err);
	err = clGetKernelWorkGroupInfo(kernel, NULL, CL_KERNEL_LOCAL_MEM_SIZE, sizeof(cl_ulong), &memSize, &retSize);
	res &= Check(L"clGetKernelWorkGroupInfo",CL_SUCCESS,err);
	err = clGetKernelWorkGroupInfo(kernel, NULL, CL_KERNEL_COMPILE_WORK_GROUP_SIZE, sizeof(size_t[3]), wgSize, &retSize);
	res &= Check(L"clGetKernelWorkGroupInfo",CL_SUCCESS,err);
	err = clGetKernelWorkGroupInfo(kernel, NULL, CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, sizeof(size_t), &wgSize[0], &retSize);
	res &= Check(L"clGetKernelWorkGroupInfo",CL_SUCCESS,err);
	err = clGetKernelWorkGroupInfo(kernel, NULL, CL_KERNEL_PRIVATE_MEM_SIZE, sizeof(cl_ulong), &memSize, &retSize);
	res &= Check(L"clGetKernelWorkGroupInfo",CL_SUCCESS,err);
	// CSSD100011902
	wgSize[0] = -1;
	retSize = 0;
	err = clGetKernelWorkGroupInfo(kernel, NULL, CL_KERNEL_COMPILE_WORK_GROUP_SIZE, sizeof(size_t), wgSize, &retSize);
	res &= Check(L"clGetKernelWorkGroupInfo",CL_INVALID_VALUE ,err);
	res &= CheckSize(L"clGetKernelWorkGroupInfo", -1, wgSize[0]);
	res &= CheckSize(L"clGetKernelWorkGroupInfo", 0, retSize);
	char numArgs = -1;
	retSize = 0;
	err = clGetKernelInfo(kernel, CL_KERNEL_NUM_ARGS, sizeof(char), &numArgs, &retSize);
	res &= Check(L"clGetKernelInfo",CL_INVALID_VALUE ,err);
	res &= CheckCondition(L"clGetKernelInfo", -1 == numArgs);
	res &= CheckSize(L"clGetKernelInfo", 0, retSize);

	if (!res) 
	{
		clReleaseKernel(kernel);
		clReleaseProgram(program);	
		return res;
	}
    
	// create buffer - should fail
	cl_mem buff = clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_READ_WRITE, sizeof(int), NULL, &err); 
	res = SilentCheck(L"clCreateBuffer, CSSD100006060",CL_INVALID_VALUE,err);
	if (!res) 
	{
		clReleaseKernel(kernel);
		clReleaseProgram(program);	
		return res;
	}
	// create buffer - should succeed
	buff = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(char), NULL, &err); 
	res = SilentCheck(L"clCreateBuffer",CL_SUCCESS,err);
	if (!res) 
	{
		clReleaseKernel(kernel);
		clReleaseProgram(program);	
		return res;
	}

	// create sub buffer - should fail
	cl_mem buffy = clCreateSubBuffer(0, CL_MEM_WRITE_ONLY, CL_BUFFER_CREATE_TYPE_REGION ,NULL, &err); 
	res = SilentCheck(L"clCreateSubBuffer, CSSD100006061",CL_INVALID_MEM_OBJECT,err);
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
        clReleaseMemObject(buff);
		clReleaseKernel(kernel);
		clReleaseProgram(program);	
		return res;
	}

	size_t global_work_size[1];
	global_work_size[0] = WORK_SIZE;

	cl_event ndEvent;

	//err = clEnqueueNDRangeKernel(cmd_queue,kernel,1,NULL,global_work_size,NULL,1,NULL, &ndEvent);
	//sres = SilentCheck(L"clEnqueueNDRangeKernel",CL_INVALID_EVENT_WAIT_LIST,err);

	err = clEnqueueNDRangeKernel(cmd_queue,kernel,1,NULL,global_work_size,NULL,0,NULL, &ndEvent);
	res = SilentCheck(L"clEnqueueNDRangeKernel",CL_SUCCESS,err);
	if (!res) 
	{
        clReleaseMemObject(buff);
		clReleaseKernel(kernel);
		clReleaseProgram(program);	
		return res;
	}

	cl_event ev;
	//enqueue read buffer - should fail
	err = clEnqueueReadBuffer(cmd_queue, buff, CL_TRUE, 0, sizeof(char), out, 1, NULL, &ev);
	res = SilentCheck(L"SilentCheck, CSSD100006062",CL_INVALID_EVENT_WAIT_LIST,err);
	//enqueue read buffer - should fail
	err &= clEnqueueReadBuffer(cmd_queue, buff, CL_TRUE, 0, sizeof(char), out, 1, NULL, NULL);
	res &= SilentCheck(L"clEnqueueReadBuffer, CSSD100006062",CL_INVALID_EVENT_WAIT_LIST,err);
	//enqueue read buffer - should succeed
	err = clEnqueueReadBuffer(cmd_queue, buff, CL_TRUE, 0, sizeof(char), out, 0, NULL, NULL);
	res &= SilentCheck(L"clEnqueueReadBuffer",CL_SUCCESS,err);

	err = clEnqueueCopyBuffer(cmd_queue, buff,buff,0, 0, sizeof(char), 0, NULL, NULL);
	res &= SilentCheck(L"clEnqueueCopyBuffer, CSSD100006063",CL_MEM_COPY_OVERLAP,err);

	clFinish(cmd_queue);
    clReleaseMemObject(buff);
	SilentCheck(L"Second clReleaseMemObject(buff)", CL_INVALID_MEM_OBJECT, clReleaseMemObject(buff)); // CSSD100005934
	clReleaseKernel(kernel);
	SilentCheck(L"Second clReleaseKernel()", CL_INVALID_KERNEL, clReleaseKernel(kernel)); // CSSD100005934
	clReleaseProgram(program);
	SilentCheck(L"Second clReleaseProgram()", CL_INVALID_PROGRAM, clReleaseProgram(program)); // CSSD100005934
	return res;
}

bool api_test(){
	printf("---------------------------------------\n");
	printf("API test\n");
	printf("---------------------------------------\n");
	bool bResult = true;
	cl_context context=NULL;
	cl_command_queue cmd_queue=NULL;
	cl_device_id device=NULL;
	cl_program program=NULL;
	cl_kernel kernel=NULL;
	cl_int err;
	cl_uint num_devices = 1;
	cl_platform_id platform=NULL;
	cl_char user_data[8]; 

	//init platform
	err = clGetPlatformIDs(1,&platform,NULL);
	bResult = SilentCheck(L"clGetPlatformIDs",CL_SUCCESS,err);
	if (!bResult)	return bResult;

	// init Devices (only one CPU...) should fail
	err = clGetDeviceIDs(platform,CL_DEVICE_TYPE_ALL,0,&device,&num_devices);
	bResult = SilentCheck(L"clGetDeviceIDs, CSSD100006051:1",CL_INVALID_VALUE,err);
	if (!bResult)	return bResult;

	// init Devices (only one CPU...) should fail
	err = clGetDeviceIDs(platform,CL_DEVICE_TYPE_ALL,0,NULL,NULL);
	bResult = SilentCheck(L"clGetDeviceIDs, CSSD100006051:2",CL_INVALID_VALUE,err);
	if (!bResult)	return bResult;

	// init Devices (only one CPU...) should succeed
	err = clGetDeviceIDs(platform,CL_DEVICE_TYPE_DEFAULT,1,&device,NULL);
	bResult = SilentCheck(L"clGetDeviceIDs",CL_SUCCESS,err);
	if (!bResult)	return bResult;

	// init Devices (only one CPU...) should succeed
	err = clGetDeviceIDs(platform,CL_DEVICE_TYPE_ALL,1,NULL,&num_devices);
	bResult = SilentCheck(L"clGetDeviceIDs",CL_SUCCESS,err);
	if (!bResult)	return bResult;

	// clGetExtensionFunctionAddress(NULL), bug CSSD100007136
	void* invFunc = clGetExtensionFunctionAddress(NULL);
	bResult = CheckSize(L"clGetExtensionFunctionAddress(NULL)", (size_t)NULL, (size_t)invFunc);
	if (!bResult)	return bResult;

	// clGetDeviceInfo should fail
	size_t actual_size;
	char dummy[4];
	err = clGetDeviceInfo(device, CL_DEVICE_TYPE, 1, dummy, &actual_size);
	bResult = SilentCheck(L"clGetDeviceInfo, CSSD100006052",CL_INVALID_VALUE,err);
	if (!bResult)	return bResult;

	//init context should fail
	context = clCreateContext(NULL,1,&device,NULL,user_data,&err);
	bResult = SilentCheck(L"clCreateContext, CSSD100006053",CL_INVALID_VALUE,err);
	if (!bResult)	return bResult;

	//init context should fail
	context = clCreateContext(NULL,1,NULL,NULL,NULL,&err);
	bResult = SilentCheck(L"clCreateContext, CSSD100006054",CL_INVALID_VALUE,err);
	if (!bResult)	return bResult;

	//init context should fail
	context = clCreateContextFromType(NULL,CL_DEVICE_TYPE_ALL,NULL,user_data,&err);
	bResult = SilentCheck(L"clCreateContext, CSSD100006055",CL_INVALID_VALUE,err);
	if (!bResult)	return bResult;

	//init context should succeed
	context = clCreateContext(NULL,1,&device,NULL,NULL,&err);
	bResult = SilentCheck(L"clCreateContext",CL_SUCCESS,err);
	if (!bResult)	return bResult;

	// Check user event query CSSD100007133
	cl_event user_ev;
	user_ev = clCreateUserEvent(context, &err);
	bResult = SilentCheck(L"clCreateUserEvent",CL_SUCCESS,err);
	if (!bResult)	return bResult;

	cl_context evntCntx;
	size_t envtCntxSize;
	err = clGetEventInfo(user_ev,CL_EVENT_CONTEXT, sizeof(evntCntx), &evntCntx, &envtCntxSize);
	clReleaseEvent(user_ev);

	// Check info query
	bResult = Check(L"clGetEventInfo",CL_SUCCESS,err);
	if (!bResult)	return bResult;
	bResult = CheckHandle(L"clGetEventInfo", context, evntCntx);
	if (!bResult)	return bResult;
	bResult = CheckSize(L"clGetEventInfo",sizeof(evntCntx),envtCntxSize);
	if (!bResult)	return bResult;

	// CSSD100011903
	err = clWaitForEvents(1, NULL);
	bResult = Check(L"clWaitForEvents, CSSD100011903",CL_INVALID_VALUE,err);
	if (!bResult){
		return bResult;
	}	

	//init Command Queue - should fail
	cmd_queue = clCreateCommandQueue(context,device,0xf,&err);
	bResult = SilentCheck(L"clCreateCommandQueue, CSSD100006058",CL_INVALID_VALUE,err);
	if (!bResult){
		clReleaseContext(context);
		return bResult;
	}	

	//init Command Queue - should succeed
	cmd_queue = clCreateCommandQueue(context,device, 0/*CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE*/,&err);
	bResult = SilentCheck(L"clCreateCommandQueue",CL_SUCCESS,err);
	if (!bResult){
		clReleaseContext(context);
		return bResult;
	}

	char* ocl_test_program= {\
		"__kernel void apiTest(__global char  *fred)\
		{fred[0] = 0;}"};

	char p;

	bResult = run_kernel(context, device, cmd_queue, ocl_test_program, &p);
	printf("\n---------------------------------------\n");
	printf("API test succeeded!\n");
	printf("---------------------------------------\n");

	clFinish(cmd_queue);
	fflush(stdout);
	clReleaseCommandQueue(cmd_queue);
	SilentCheck(L"Second clReleaseCommandQueue()", CL_INVALID_COMMAND_QUEUE, clReleaseCommandQueue(cmd_queue)); // CSSD100005934
	clReleaseContext(context);
	err = clReleaseContext(context);
	bResult = SilentCheck(L"Second clReleaseContext()",CL_INVALID_CONTEXT,err); // CSSD100006057, CSSD100005934

	return bResult;
}
