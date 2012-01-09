//|
//| TEST: DeviceFissionTest.fissionThreadTest
//|
//| Purpose 
//| -------
//|
//| Test the device fission execution.
//|
//| Method
//| ------
//|
//| 1. Create sub devices with CL_DEVICE_PARTITION_EQUALLY_EXT property from root device.
//| 2. Validate the number of compute units in the sub device.
//| 3. Enqueue execution of a kernel on the sub device.
//| 4. read the results.
//|
//| Pass criteria
//| -------------
//|
//| Return true in case of SUCCESS.

#include "CL/cl.h"
#include "cl_types.h"
#include <stdio.h>
#include "FrameworkTest.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/syscall.h>
#endif

#define WORK_SIZE 1024	
#define MAX_SOURCE_SIZE 2048
#ifdef _WIN32
void CL_CALLBACK threadFunc(void *ppParam)
{
    cl_uint* pParam = *(static_cast<cl_uint **>(ppParam));
	*pParam = GetCurrentThreadId();
}
#else
void threadFunc(void *ppParam)
{
    cl_uint* pParam = *(static_cast<cl_uint **>(ppParam));
	*pParam = ((cl_uint)syscall(SYS_gettid));
}
#endif
bool run_kernel(cl_context& context,cl_device_id& device,cl_command_queue& cmd_queue){
	cl_int err;
	bool res;
	cl_uint thread_id[WORK_SIZE];
	thread_id[0] = 0;
	thread_id[1] = 1;
	thread_id[2] = 2;

	for (size_t i = 0; i < WORK_SIZE; ++i)
	{
        void* param = thread_id + i;
		err = clEnqueueNativeKernel(cmd_queue,threadFunc,&param, sizeof(cl_uint*),0,NULL,NULL,0,NULL,NULL);
		res = SilentCheck(L"clEnqueueNativeKernel",CL_SUCCESS,err);
		if (!res) 
		{
			return res;
		}
	}

	clFinish(cmd_queue);

	cl_uint tid1 = thread_id[0], tid2 = 0;
	bool has2 = false;
	for (size_t i = 1; i < WORK_SIZE; i++)
	{
		if (!has2 && (tid1 != thread_id[i]))
		{
			tid2 = thread_id[i];
			has2=true;
		}
		else if (has2 && (tid1 != thread_id[i]) && (tid2 != thread_id[i]))
		{
			return false;
		}

	}
	return true;
}

bool fission_thread_test(){
	printf("---------------------------------------\n");
	printf("fission thread test\n");
	printf("---------------------------------------\n");
	bool bResult = true;
	cl_context context=NULL;
	cl_command_queue cmd_queue = NULL;
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
	cl_device_partition_property_ext properties[] = {CL_DEVICE_PARTITION_EQUALLY_EXT, 2, CL_PROPERTIES_LIST_END_EXT};
	err = clCreateSubDevicesEXT(device, properties, num_entries, out_devices, &num_devices);
	bResult = SilentCheck(L"clCreateSubDevicesEXT",CL_SUCCESS,err);
	if (!bResult)	return bResult;


	//init context
	context = clCreateContext(NULL,1 ,&out_devices[0],NULL,NULL,&err);
	bResult = SilentCheck(L"clCreateContext",CL_SUCCESS,err);
	if (!bResult)	return bResult;

	//init Command Queue
	cmd_queue = clCreateCommandQueue(context,out_devices[0],CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE,&err);
	bResult = SilentCheck(L"clCreateCommandQueue",CL_SUCCESS,err);
	if (!bResult){
		clReleaseCommandQueue(cmd_queue);
		clReleaseContext(context);
		return bResult;
	}

	bResult = run_kernel(context, out_devices[0], cmd_queue);
	
	const char* res = ((bResult == true) ? "succeeded" : "failed");
	printf("\n---------------------------------------\n");
	printf("fission thread test %s!\n", res);
	printf("---------------------------------------\n");

	clFinish(cmd_queue);
	clReleaseCommandQueue(cmd_queue);
	clReleaseContext(context);
	for (size_t i = 0; i < num_devices; i++)
	{
		clReleaseDeviceEXT(out_devices[i]);
	}
	return bResult;
}