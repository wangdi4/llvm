#include <CL/cl.h>
#include <stdio.h>
#include "FrameworkTest.h"
#include "FrameworkTestThreads.h"

class ShutdownThread : public SynchronizedThread
{
public:
    ShutdownThread() : SynchronizedThread() {}
	virtual ~ShutdownThread() {}

protected:
	virtual void ThreadRoutine();
};

void ShutdownThread::ThreadRoutine()
{
	cl_device_type    deviceType = CL_DEVICE_TYPE_CPU;
	cl_device_id      deviceId;
	cl_device_id*     devices = NULL;
	cl_uint           numDevices = 0;
	cl_context        context;
	cl_command_queue  commandQueue;
	cl_int            err = CL_SUCCESS;


	cl_platform_id platform = 0;
	bool bResult = true;

	err = clGetPlatformIDs(1, &platform, NULL);
	bResult &= SilentCheck(L"clGetPlatformIDs", CL_SUCCESS, err);

	if (!bResult)
	{
		return;
	}

	cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };

	/* Get the number of requested devices */
	err |= clGetDeviceIDs(platform,  deviceType, 0, NULL, &numDevices );
	devices = (cl_device_id *) malloc( numDevices * sizeof( cl_device_id ) );
	err |= clGetDeviceIDs(platform,  deviceType, numDevices, devices, NULL );
	deviceId = devices[0];
	free(devices);
	devices = NULL;

	if (err != CL_SUCCESS)
	{
		return;
	}

	context = clCreateContext(prop, 1, &deviceId, NULL, NULL, &err);
	if (err != CL_SUCCESS)
	{
		return;
	}

	commandQueue = clCreateCommandQueue(context, deviceId, 0, &err);
	if (err != CL_SUCCESS)
	{
		clReleaseContext(context);
		return;
	}

	// Create memory objects for test
	int data1 = 0xABCD;
	static int data2 = 0x1234;
    cl_mem mem1 = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int), NULL, &err);
	if (err != CL_SUCCESS)
	{
		clReleaseCommandQueue(commandQueue);
		clReleaseContext(context);
		return;
	}

    //Give all threads time to enter the pool
    err |= clEnqueueWriteBuffer(commandQueue, mem1, CL_FALSE, 0, sizeof(int), &data1, 0, NULL, NULL);
	err |= clFlush(commandQueue);
    clSleep(100);
    err |= clFinish(commandQueue);
    //Deliberate dangling command
	err |= clEnqueueReadBuffer(commandQueue, mem1, CL_FALSE, 0, sizeof(int), &data2, 0, NULL, NULL);

    err |= clFinish(commandQueue);  // TODO: remove this once TBB bug 1954 is fixed.

	err |= clReleaseMemObject(mem1);
	err |= clReleaseContext(context);
	err |= clReleaseCommandQueue(commandQueue);
    
	bResult &= SilentCheck(L"Releasing stuff", CL_SUCCESS, err);

	return;
}

bool ShutdownFromChildThread()
{
    printf("Begin bringup-shutdown from a child thread test\n");
    SynchronizedThread* pThread;
    SynchronizedThread::Signal();
	for (int i = 0; i < 10; ++i)
	{
        printf("Iteration %d\n", i);
        pThread = new ShutdownThread();
        if (NULL == pThread)
        {
            printf("Failed to allocate thread\n");
            return false;
        }
        pThread->Run();
        pThread->WaitForCompletion();
	}
    printf("test passed\n");
	return true;
}
