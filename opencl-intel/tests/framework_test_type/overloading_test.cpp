#include "CL/cl.h"
#include "cl_types.h"
#include <stdio.h>
#include <cassert>
#include "FrameworkTest.h"
#include "cl_sys_defines.h"

#define WORK_SIZE 1	
#define MAX_SOURCE_SIZE 2048

bool run_kernel_overloading(cl_context& context,cl_device_id& device,cl_command_queue& cmd_queue, char* prog, int *out){
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

	kernel = clCreateKernel(program,"overloadingTest",&err);
	res = SilentCheck(L"clCreateKernel",CL_SUCCESS,err);
	if (!res) 
	{
		clReleaseProgram(program);	
		return res;
	}

	cl_mem buff = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int), NULL, &err); 
	res = SilentCheck(L"clCreateKernel",CL_SUCCESS,err);
	if (!res) 
	{
		clReleaseKernel(kernel);
		clReleaseProgram(program);	
		return res;
	}

	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*) &buff);
	res = SilentCheck(L"clCreateKernel",CL_SUCCESS,err);
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

	err = clEnqueueReadBuffer(cmd_queue, buff, CL_TRUE, 0, sizeof(int), out, 0, NULL, NULL);

	clFinish(cmd_queue);
	clReleaseKernel(kernel);
	clReleaseProgram(program);
	return ((CL_SUCCESS == err)? true : false);
}

bool run_kernel_overloadingOrder(cl_context& context,cl_device_id& device,cl_command_queue& cmd_queue, char* prog, int *out){
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

	kernel = clCreateKernel(program,"overloadingOrderTest",&err);
	res = SilentCheck(L"clCreateKernel",CL_SUCCESS,err);
	if (!res) 
	{
		clReleaseProgram(program);	
		return res;
	}

	cl_mem buff = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int), NULL, &err); 
	res = SilentCheck(L"clCreateKernel",CL_SUCCESS,err);
	if (!res) 
	{
		clReleaseKernel(kernel);
		clReleaseProgram(program);	
		return res;
	}

	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*) &buff);
	res = SilentCheck(L"clCreateKernel",CL_SUCCESS,err);
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

	err = clEnqueueReadBuffer(cmd_queue, buff, CL_TRUE, 0, sizeof(int), out, 0, NULL, NULL);

	clFinish(cmd_queue);
	clReleaseKernel(kernel);
	clReleaseProgram(program);
	return ((CL_SUCCESS == err)? true : false);
}

bool overloading_test(){
	printf("---------------------------------------\n");
	printf("overloading_test\n");
	printf("---------------------------------------\n");
	bool bResult = true;
	cl_context context=NULL;
	cl_command_queue cmd_queue=NULL;
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


	//init context
	context = clCreateContext(NULL,1,&device,NULL,NULL,&err);
	bResult = SilentCheck(L"clCreateContext",CL_SUCCESS,err);
	if (!bResult)	return bResult;

	//init Command Queue
	cmd_queue = clCreateCommandQueue(context,device,0,&err);
	bResult = SilentCheck(L"clCreateCommandQueue",CL_SUCCESS,err);
	if (!bResult){
		clReleaseContext(context);
		return bResult;
	}	
	char* ocl_test_program= {\
"int __attribute__ ((overloadable)) test_func(int a, float b)\
		{\
	return 1;\
}\
int __attribute__ ((overloadable)) test_func(int4 a, float4 b)\
{\
	return 2;\
}\
int __attribute__ ((overloadable)) test_func(float a, char b)\
{\
	return 3;\
}\
int __attribute__ ((overloadable)) test_func(float4 a, char4 b)\
{\
	return 4;\
}\
int __attribute__ ((overloadable)) test_func(double a, float b)\
{\
	return 5;\
}\
int __attribute__ ((overloadable)) test_func(double2 a, float4 b)\
{\
	return 6;\
}\
int __attribute__ ((overloadable)) test_func(double a, short b)\
{\
	return 7;\
}\
int __attribute__ ((overloadable)) test_func(double2 a, short4 b)\
{\
	return 8;\
}\
int __attribute__ ((overloadable)) test_func(int a, char b)\
{\
	return 9;\
}\
int __attribute__ ((overloadable)) test_func(int4 a, char4 b)\
{\
	return 10;\
}\
int __attribute__ ((overloadable)) test_func(char4 a, float b)\
		{\
	return 11;\
}\
int __attribute__ ((overloadable)) test_func(long2 a, float b)\
		{\
	return 12;\
}\
__kernel void overloadingTest(global int *result){	\
								   %s a;\
								   %s b;\
								   *result = test_func(a,b);\
								 }"};
	
		char* ocl_test_order_program= {\
"int __attribute__ ((overloadable)) test_func(char a, float b)\
		{\
	return 1;\
}\
int __attribute__ ((overloadable)) test_func(int4 a, float b)\
{\
	return 2;\
}\
int __attribute__ ((overloadable)) test_func(long2 a, double b)\
{\
	return 3;\
}\
__kernel void overloadingOrderTest(global int *result){	\
								   %s a;\
								   %s b;\
								   *result = test_func(a,b);\
								 }"};

	char* firstParam1[] = {"uchar", "int4", "float", "float4", "double", "double2", "double", "double2", "int", "int4"};
	char* secondParam1[] = {"float", "float4", "char", "char4", "float", "float4", "short", "short4", "char", "char4"};
	
	char* firstParam2[] = {"char", "int", "short", "int", "long", "long2"};
	char* secondParam2[] = {"float", "float", "double", "float", "double", "float"};

	char prog[MAX_SOURCE_SIZE];
	int res[sizeof(firstParam1) / sizeof(firstParam1[0])];
	bool flag = false;

	assert(sizeof(firstParam1) == sizeof(secondParam1));
	for( int i = 0; i < sizeof(firstParam1) / sizeof(firstParam1[0]); i++)
	{
		res[i] = 11;
		SPRINTF_S(prog, MAX_SOURCE_SIZE, ocl_test_program, firstParam1[i], secondParam1[i]);
		bResult = run_kernel_overloading(context, device, cmd_queue, prog, &(res[i]));
		if(!bResult) break;
		if (res[i] != (i+1) ) 
		{
			flag = true;
			printf("function with parameters (%s, %s) failed\n", firstParam1[i], secondParam1[i]);
			printf("expected: (%s, %s), called (%s, %s)\n", firstParam1[i], secondParam1[i], firstParam1[res[i]-1], secondParam1[res[i]-1]);
		}
	}
	printf("\n---------------------------------------\n");
	if(!flag) printf("overloading match test succeeded!\n");
	else printf("overloading match test failed!\n");
	printf("---------------------------------------\n");

	flag = false;
	int expected[] = {1,2,3,2,3,3};
	assert(sizeof(firstParam2) == sizeof(secondParam2));
	assert(sizeof(firstParam2) / sizeof(firstParam2[0]) == sizeof(expected) / sizeof(expected[0]));
	char* paramNames[] = { "function wasn't found", "(char, float)", "(int4, float)", "(long2, double)"};
	for( int i = 0; i < sizeof(expected) / sizeof(expected[0]); i++)
	{
		res[i] = 0;
		SPRINTF_S(prog, MAX_SOURCE_SIZE, ocl_test_order_program, firstParam2[i], secondParam2[i]);
		bResult = run_kernel_overloadingOrder(context, device, cmd_queue, prog, &(res[i]));
		if(!bResult) break;
		if (res[i] != expected[i] )
		{
			flag = true;
			printf("function with parameters (%s, %s) failed\n", firstParam2[i], secondParam2[i]);
			printf("expected: %s, called %s\n", paramNames[expected[i]], paramNames[res[i]]);
		}
	}
	printf("\n---------------------------------------\n");
	if(!flag) printf("overloading order test succeeded!\n");
	else printf("overloading order test failed!\n");
	printf("---------------------------------------\n");

	clFinish(cmd_queue);
	fflush(stdout);
	clReleaseCommandQueue(cmd_queue);
	clReleaseContext(context);
	return bResult;
}