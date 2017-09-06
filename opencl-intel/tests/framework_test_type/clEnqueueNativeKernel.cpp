#include <CL/cl.h>
#include <stdio.h>
#include "FrameworkTest.h"

extern cl_device_type gDeviceType;

struct arg_struct
{
	int* inp1;
	int* inp2;
	int val;
	int* output;
};

void CL_CALLBACK NativeFunc(void* args)
{
	if (!SilentCheck("args properly supplied", true, args != NULL))
	{
		return;
	}

	arg_struct* arguments = (arg_struct*)args;
	*arguments->output = *arguments->inp1 + *arguments->inp2 + arguments->val;
}

bool EnqueueNativeKernelTest()
{

	cl_device_id      deviceId;
	cl_device_id*     devices = NULL;
	cl_uint           numDevices = 0;
	cl_context        context;
	cl_command_queue  commandQueue;
	cl_int            err = CL_SUCCESS;


	cl_platform_id platform = 0;
	bool bResult = true;

	err = clGetPlatformIDs(1, &platform, NULL);
	bResult &= SilentCheck("clGetPlatformIDs", CL_SUCCESS, err);

	if (!bResult)
	{
		return bResult;
	}

	cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };

	/* Get the number of requested devices */
	err |= clGetDeviceIDs(platform,  gDeviceType, 0, NULL, &numDevices );
	devices = (cl_device_id *) malloc( numDevices * sizeof( cl_device_id ) );
	err |= clGetDeviceIDs(platform,  gDeviceType, numDevices, devices, NULL );
	deviceId = devices[0];
	free(devices);
	devices = NULL;

	if (err != CL_SUCCESS)
	{
		return false;
	}

	context = clCreateContext(prop, 1, &deviceId, NULL, NULL, &err);
	if (err != CL_SUCCESS)
	{
		return false;
	}

	commandQueue = clCreateCommandQueue(context, deviceId, 0, &err);
	if (err != CL_SUCCESS)
	{
		clReleaseContext(context);
		return false;
	}

	// Create memory objects for test
	int data1 = 0xABCD;
	int data2 = 0x1234;
	cl_mem mem1 = clCreateBuffer(context, CL_MEM_COPY_HOST_PTR, sizeof(int), &data1, &err);
	if (err != CL_SUCCESS)
	{
		clReleaseCommandQueue(commandQueue);
		clReleaseContext(context);
		return false;
	}
	cl_mem mem2 = clCreateBuffer(context, CL_MEM_COPY_HOST_PTR, sizeof(int), &data2, &err);
	if (err != CL_SUCCESS)
	{
		clReleaseMemObject(mem1);
		clReleaseCommandQueue(commandQueue);
		clReleaseContext(context);
		return false;
	}
	cl_mem res = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int), NULL, &err);
	if (err != CL_SUCCESS)
	{
		clReleaseMemObject(mem1);
		clReleaseMemObject(mem2);
		clReleaseCommandQueue(commandQueue);
		clReleaseContext(context);
		return false;
	}

	arg_struct args;
	args.val = 1234; 

	cl_mem memList[] = {mem1,mem2,res};
	void*  offsets[] = {&args.inp2,&args.inp1,&args.output};
	err |= clEnqueueNativeKernel(commandQueue, NativeFunc, &args, sizeof(arg_struct), 3, memList, (const void**)offsets, 0, NULL, NULL);
	int data3;
	err |= clEnqueueReadBuffer(commandQueue, res, CL_TRUE, 0, sizeof(int), &data3, 0, NULL, NULL);

	if ( CL_SUCCEEDED(err) )
	{
		bResult &= (data3 == (data1 + data2 + args.val));
	}

	err |= clReleaseMemObject(mem1);
	err |= clReleaseMemObject(mem2);
	err |= clReleaseMemObject(res);
	err |= clReleaseCommandQueue(commandQueue);
	err |= clReleaseContext(context);
	bResult &= SilentCheck("Releasing stuff", CL_SUCCESS, err);

	return bResult;
}
