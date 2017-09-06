#include "CL/cl.h"
#include "cl_types.h"
#include <stdio.h>
#include <time.h>
#include "FrameworkTest.h"
#include "FrameworkTestThreads.h"

extern cl_device_type gDeviceType;

/* Threads for MT context retain/release test */
class RetainReleaseTestThread : public SynchronizedThread
{
public:
	RetainReleaseTestThread(size_t iterations, size_t print_period, cl_context contextHandle) : 
            m_iterations(iterations), m_print_period(print_period), m_contextHandle(contextHandle) {}
	virtual ~RetainReleaseTestThread() {}

protected:
	virtual void ThreadRoutine();

	size_t     m_iterations;
    size_t     m_print_period;
	cl_context m_contextHandle;
};

void RetainReleaseTestThread::ThreadRoutine()
{
    size_t period = 0;
	for (size_t i = 0; i < m_iterations; ++i)
	{
        if (0 == period)
        {
            printf(".");fflush(0);
            period = m_print_period;
        }
        --period;
		clRetainContext(m_contextHandle);
		clReleaseContext(m_contextHandle);
	}
}

/* Threads for MT release objects test */
class ReleaseKernelArrayThread : public SynchronizedThread
{
public:
	ReleaseKernelArrayThread(cl_kernel* kernels, size_t count) : m_count(count), m_kernels(kernels) {}
	virtual ~ReleaseKernelArrayThread() {}

protected:
	virtual void ThreadRoutine();

	size_t     m_count;
	cl_kernel* m_kernels;
};
class ReleaseProgramArrayThread : public SynchronizedThread
{
public:
	ReleaseProgramArrayThread(cl_program* programs, size_t count) : m_count(count), m_programs(programs) {}
	virtual ~ReleaseProgramArrayThread() {}

protected:
	virtual void ThreadRoutine();

	size_t     m_count;
	cl_program* m_programs;
};
class ReleaseQueueArrayThread : public SynchronizedThread
{
public:
	ReleaseQueueArrayThread(cl_command_queue* queues, size_t count) : m_count(count), m_queues(queues) {}
	virtual ~ReleaseQueueArrayThread() {}

protected:
	virtual void ThreadRoutine();

	size_t            m_count;
	cl_command_queue* m_queues;
};
class ReleaseContextArrayThread : public SynchronizedThread
{
public:
	ReleaseContextArrayThread(cl_context* contexts, size_t count) : m_count(count), m_contexts(contexts) {}
	virtual ~ReleaseContextArrayThread() {}

protected:
	virtual void ThreadRoutine();

	size_t      m_count;
	cl_context* m_contexts;
};

void ReleaseKernelArrayThread::ThreadRoutine()
{
	for (size_t i = 0; i < m_count; ++i)
	{
		clReleaseKernel(m_kernels[i]);
	}
}

void ReleaseProgramArrayThread::ThreadRoutine()
{
	for (size_t i = 0; i < m_count; i += 2)
	{
		clReleaseProgram(m_programs[i]);
	}
	for (size_t i = 1; i < m_count; i += 2)
	{
		clReleaseProgram(m_programs[i]);
	}
}

void ReleaseQueueArrayThread::ThreadRoutine()
{
	for (size_t i = 0; i < m_count; ++i)
	{
		clReleaseCommandQueue(m_queues[i]);
	}
}

void ReleaseContextArrayThread::ThreadRoutine()
{
	for (size_t i = m_count; i > 0; --i)
	{
		clReleaseContext(m_contexts[i - 1]);
	}
}

/* Threads for MT dot product test */
class HelloWorldTestThread : public SynchronizedThread
{
public:
	HelloWorldTestThread(cl_context context, cl_command_queue queue, cl_program program, cl_mem param, int expected) : m_context(context), m_queue(queue), m_program(program), m_param(param), m_expectedResult(expected) {}
	virtual ~HelloWorldTestThread() {}

protected:
	virtual void ThreadRoutine();

	cl_context       m_context;
	cl_command_queue m_queue;
	cl_program       m_program;
	cl_mem           m_param;
	int              m_expectedResult;
};

void HelloWorldTestThread::ThreadRoutine()
{
	cl_int       err;
	int          kernelResult = 953;
	const size_t globalSize   = 1;

	cl_kernel    kernel       = clCreateKernel(m_program, "k", &err);
	SilentCheck("clCreateKernel", CL_SUCCESS, err);

	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &m_param);
	SilentCheck("clSetKernelArg", CL_SUCCESS, err);

	err = clEnqueueNDRangeKernel(m_queue, kernel, 1, NULL, &globalSize, NULL, 0, NULL, NULL);
	SilentCheck("clEnqueueNDRangeKernel", CL_SUCCESS, err);

	err = clEnqueueReadBuffer(m_queue, m_param, CL_TRUE, 0, sizeof(int), &kernelResult, 0, NULL, NULL);
	SilentCheck("clEnqueueReadBuffer", CL_SUCCESS, err);

	SilentCheck("Kernel result", m_expectedResult, kernelResult);

	clReleaseKernel(kernel);
}

/* Threads for MT full execution test */
class ConcurrentExecutionTestThread : public SynchronizedThread
{
public:
	ConcurrentExecutionTestThread(size_t i) : m_id(i) {}
	virtual ~ConcurrentExecutionTestThread() {}

protected:
	virtual void ThreadRoutine();

	size_t m_id;
};

void ConcurrentExecutionTestThread::ThreadRoutine()
{
	bool bResult = true;
	cl_int iRet  = CL_SUCCESS;

	cl_platform_id platform     = 0;
	cl_device_id   deviceId     = 0;
	const size_t globalSize     = 1;

	cl_context           context;
	cl_command_queue     queue;
	cl_program           program;
	cl_mem               param;
	cl_kernel            kernel;
	int                  kernelResult = 0x1234;

	const char* programSource    = "__kernel void k(__global int* p) { p[get_global_id(0)] = 8; }";

	iRet = clGetPlatformIDs(1, &platform, NULL);
	bResult &= SilentCheck("clGetPlatformIDs", CL_SUCCESS, iRet);

	cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };

	if (!bResult)
	{
		return;
	}

	context  = clCreateContextFromType(prop, gDeviceType, NULL, NULL, &iRet);
	bResult &= SilentCheck("Create Context from type (gDeviceType)", CL_SUCCESS, iRet);

	if (!bResult)
	{
		return;
	}

	iRet     = clGetDeviceIDs(platform, gDeviceType, 1, &deviceId, NULL);
	bResult &= SilentCheck("Get device ID (gDeviceType)", CL_SUCCESS, iRet);

	if (!bResult)
	{
		return;
	}
 
	queue    = clCreateCommandQueue(context, deviceId, 0, &iRet);
	bResult &= SilentCheck("Create command queue", CL_SUCCESS, iRet);

	if (!bResult)
	{
		return;
	}
	program  = clCreateProgramWithSource(context, 1, &programSource, NULL, &iRet);
	bResult &= SilentCheck("Create program with source", CL_SUCCESS, iRet);

	if (!bResult)
	{
		return;
	}
	iRet     = clBuildProgram(program, 1, &deviceId, NULL, NULL, NULL);
	bResult &= SilentCheck("Build program", CL_SUCCESS, iRet);

	if (!bResult)
	{
		return;
	}
	param    = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int), NULL, &iRet);
	bResult &= SilentCheck("Build clCreateBuffer", CL_SUCCESS, iRet);

	if (!bResult)
	{
		return;
	}

	kernel   = clCreateKernel(program, "k", &iRet);
	bResult &= SilentCheck("clCreateKernel", CL_SUCCESS, iRet);

	if (!bResult)
	{
		return;
	}

	iRet     = clSetKernelArg(kernel, 0, sizeof(cl_mem), &param);
	bResult &= SilentCheck("clSetKernelArg", CL_SUCCESS, iRet);

	if (!bResult)
	{
		return;
	}

	iRet     = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &globalSize, NULL, 0, NULL, NULL);
	bResult &= SilentCheck("clEnqueueNDRangeKernel", CL_SUCCESS, iRet);

	if (!bResult)
	{
		return;
	}

	iRet     = clEnqueueReadBuffer(queue, param, CL_TRUE, 0, sizeof(int), &kernelResult, 0, NULL, NULL);
	bResult &= SilentCheck("clEnqueueReadBuffer", CL_SUCCESS, iRet);

	bResult &= SilentCheck("Kernel result", 8, kernelResult);

	clReleaseKernel(kernel);
	clReleaseMemObject(param);
	clReleaseProgram(program);
	clReleaseCommandQueue(queue);
	clReleaseContext(context);
}

bool MultithreadedContextRefCount()
{
	cl_platform_id platform     = 0;
	cl_context     context;

	bool bResult = true;
	cl_int iRet  = CL_SUCCESS;

	const size_t numIterations   = 10000;
	const size_t print_period    = numIterations/10;
	const size_t initialRefCount = 1;
	const size_t numThreads      = 20;

	printf("Begin multi threaded context ref count test\n");

	iRet = clGetPlatformIDs(1, &platform, NULL);
	bResult &= Check("clGetPlatformIDs", CL_SUCCESS, iRet);

	if (!bResult)
	{
		return bResult;
	}

	cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };

	context = clCreateContextFromType(prop, gDeviceType, NULL, NULL, &iRet);
	bResult &= Check("Create Context from type (gDeviceType)", CL_SUCCESS, iRet);

	cl_uint uiCntxRefCnt = 0;
	iRet = clGetContextInfo(context, CL_CONTEXT_REFERENCE_COUNT, sizeof(cl_uint), &uiCntxRefCnt, NULL);
	if (CL_SUCCESS != iRet)
	{
		printf("clGetContextInfo = %s\n",ClErrTxt(iRet));
		return false;
	}

	printf("context ref count = %d\n",uiCntxRefCnt);
	if (uiCntxRefCnt != 1)
	{
		return false;
	}
	for (size_t i = 1; i < initialRefCount; ++i)
	{
		iRet = clRetainContext(context);
		if (CL_SUCCESS != iRet)
		{
			printf("clRetainContext = %s\n",ClErrTxt(iRet));
			return false;
		}
	}
	iRet = clGetContextInfo(context, CL_CONTEXT_REFERENCE_COUNT, sizeof(cl_uint), &uiCntxRefCnt, NULL);
	if (CL_SUCCESS != iRet)
	{
		printf("clGetContextInfo = %s\n",ClErrTxt(iRet));
		return false;
	}
	printf("context ref count (after single threaded retains) = %d\n",uiCntxRefCnt);
	if (uiCntxRefCnt != initialRefCount)
	{
		return false;
	}

	SynchronizedThread* threads[numThreads];
	for (size_t i = 0; i < numThreads; ++i)
	{
		threads[i] = new RetainReleaseTestThread(numIterations, print_period, context);
	}
    printf("Running\n");fflush(0);
	SynchronizedThreadPool pool;
	pool.Init(threads, numThreads);
	pool.StartAll();
	pool.WaitAll();
    printf("\nDone\n");fflush(0);
	for (size_t i = 0; i < numThreads; ++i)
	{
		delete threads[i];
	}

	iRet = clGetContextInfo(context, CL_CONTEXT_REFERENCE_COUNT, sizeof(cl_uint), &uiCntxRefCnt, NULL);
	if (CL_SUCCESS != iRet)
	{
		printf("clGetContextInfo = %s\n",ClErrTxt(iRet));
		return false;
	}
	printf("context ref count (after multi threaded retains) = %d\n",uiCntxRefCnt);
	if (uiCntxRefCnt != initialRefCount)
	{
		return false;
	}

	// Release contexts
	for(size_t i = initialRefCount; i > 0; --i)
	{
		clReleaseContext(context);
	}
	return true;
}



bool MultithreadedReleaseObjects()
{

	bool bResult = true;
	cl_int iRet  = CL_SUCCESS;

	const size_t numIterations   = 10;
	const size_t numObjects      = 10;

	cl_platform_id platform     = 0;
	cl_device_id   deviceId     = 0;

	cl_context           contexts[numObjects];
	cl_command_queue     queues[numObjects];
	cl_program           programs[numObjects];
	cl_kernel            kernels[numObjects];

	const char* programSource    = "__kernel void stam(__global int* p) { p[get_global_id(0)] += 2; }";

	printf("Begin multi threaded release objects test\n");

	iRet = clGetPlatformIDs(1, &platform, NULL);
	bResult &= SilentCheck("clGetPlatformIDs", CL_SUCCESS, iRet);

	if (!bResult)
	{
		return bResult;
	}

	cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };

	for (size_t iteration = 0; iteration < numIterations; ++iteration)
	{
		//Creation phase
		for (size_t object = 0; object < numObjects; ++object)
		{
			contexts[object] = clCreateContextFromType(prop, gDeviceType, NULL, NULL, &iRet);
			bResult &= SilentCheck("Create Context from type (gDeviceType)", CL_SUCCESS, iRet);

			iRet = clGetDeviceIDs(platform, gDeviceType, 1, &deviceId, NULL);
			bResult &= SilentCheck("Get device ID (gDeviceType)", CL_SUCCESS, iRet);

			queues[object] = clCreateCommandQueue(contexts[object], deviceId, 0, &iRet);
			bResult &= SilentCheck("Create command queue", CL_SUCCESS, iRet);

			programs[object] = clCreateProgramWithSource(contexts[object], 1, &programSource, NULL, &iRet);
			bResult &= SilentCheck("Create program with source", CL_SUCCESS, iRet);

			iRet = clBuildProgram(programs[object], 1, &deviceId, NULL, NULL, NULL);
			bResult &= SilentCheck("Build program", CL_SUCCESS, iRet);

			kernels[object] = clCreateKernel(programs[object], "stam", &iRet);
			bResult &= SilentCheck("Create Kernel", CL_SUCCESS, iRet);
		}

		if (!bResult)
		{
			return false;
		}
		printf("\rIteration %zu creation passed", iteration);

		//Destruction phase
		SynchronizedThread* threads[4];
		threads[0] = new ReleaseContextArrayThread(contexts, numObjects);
		threads[1] = new ReleaseKernelArrayThread(kernels, numObjects);
		threads[2] = new ReleaseQueueArrayThread(queues, numObjects);
		threads[3] = new ReleaseProgramArrayThread(programs, numObjects);

		SynchronizedThreadPool pool;
		pool.Init(threads, 4);
		pool.StartAll();
		pool.WaitAll();
		printf("  -  destruction passed");
		for (size_t i = 0; i < 4; ++i)
		{
			delete threads[i];
		}
	}

	return true;
}

bool MultithreadedHelloWorld()
{
	bool bResult = true;
	cl_int iRet  = CL_SUCCESS;

	const size_t numThreads      = 100;
	const size_t expectedResult  = 8; //remember to change it in programSource if you don't like this value

	cl_platform_id platform     = 0;
	cl_device_id   deviceId     = 0;

	cl_context           context;
	cl_command_queue     queue;
	cl_program           program;
	cl_mem               params[numThreads];

	const char* programSource    = "__kernel void k(__global int* p) { p[get_global_id(0)] = 8; }";

	printf("Begin multi threaded hello world test\n");

	iRet = clGetPlatformIDs(1, &platform, NULL);
	bResult &= SilentCheck("clGetPlatformIDs", CL_SUCCESS, iRet);

	if (!bResult)
	{
		return bResult;
	}

	cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };

	//Creation phase
	context  = clCreateContextFromType(prop, gDeviceType, NULL, NULL, &iRet);
	bResult &= SilentCheck("Create Context from type (gDeviceType)", CL_SUCCESS, iRet);

	iRet     = clGetDeviceIDs(platform, gDeviceType, 1, &deviceId, NULL);
	bResult &= SilentCheck("Get device ID (gDeviceType)", CL_SUCCESS, iRet);

	queue    = clCreateCommandQueue(context, deviceId, 0, &iRet);
	bResult &= SilentCheck("Create command queue", CL_SUCCESS, iRet);

	program  = clCreateProgramWithSource(context, 1, &programSource, NULL, &iRet);
	bResult &= SilentCheck("Create program with source", CL_SUCCESS, iRet);

	iRet     = clBuildProgram(program, 1, &deviceId, NULL, NULL, NULL);
	bResult &= SilentCheck("Build program", CL_SUCCESS, iRet);

	if (!bResult)
	{
		return false;
	}

	for (size_t i = 0; i < numThreads; ++i)
	{
		params[i] = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int), NULL, &iRet);
		bResult &= SilentCheck("Build clCreateBuffer", CL_SUCCESS, iRet);

	}
	if (!bResult)
	{
		return false;
	}

	SynchronizedThread* threads[numThreads];
	for (size_t i = 0; i < numThreads; ++i)
	{
		threads[i] = new HelloWorldTestThread(context, queue, program, params[i], expectedResult);
	}

	SynchronizedThreadPool pool;
	pool.Init(threads, numThreads);
	pool.StartAll();
	pool.WaitAll();

	for (size_t i = 0; i < numThreads; ++i)
	{
		delete threads[i];
		clReleaseMemObject(params[i]);
	}

	clReleaseProgram(program);
	clReleaseCommandQueue(queue);
	clReleaseContext(context);

	return true;
}

bool ConcurrentExecutionTest()
{
	const size_t numThreads = 1;

	SynchronizedThread* threads[numThreads];

	printf("Begin multithreaded concurrent execution test. Using %lu threads\n", numThreads);

	for (size_t i = 0; i < numThreads; ++i)
	{
		threads[i] = new ConcurrentExecutionTestThread(i);
	}

	SynchronizedThreadPool pool;
	pool.Init(threads, numThreads);
	pool.StartAll();
	pool.WaitAll();

	for (size_t i = 0; i < numThreads; ++i)
	{
		delete threads[i];
	}

	return true;
}

/* Threads for MT order violation test */
class OrderViolationThread : public SynchronizedThread
{
public:
	OrderViolationThread(int id, cl_context context, cl_command_queue queue, cl_program program, cl_mem param) :
	  m_context(context), m_queue(queue), m_program(program), m_param(param), m_id(id), m_bResult(false)
	{
		cl_int err;
		m_kernel1 = clCreateKernel(m_program, "k1", &err);
		SilentCheck("clCreateKernel", CL_SUCCESS, err);
		err = clSetKernelArg(m_kernel1, 0, sizeof(cl_mem), &m_param);
		SilentCheck("clSetKernelArg", CL_SUCCESS, err);
		err = clSetKernelArg(m_kernel1, 1, sizeof(cl_char), &m_id);
		SilentCheck("clSetKernelArg", CL_SUCCESS, err);

		m_kernel2 = clCreateKernel(m_program, "k2", &err);
		SilentCheck("clCreateKernel", CL_SUCCESS, err);
		err = clSetKernelArg(m_kernel2, 0, sizeof(cl_mem), &m_param);
		SilentCheck("clSetKernelArg", CL_SUCCESS, err);
		err = clSetKernelArg(m_kernel2, 1, sizeof(cl_char), &m_id);
		SilentCheck("clSetKernelArg", CL_SUCCESS, err);

	  }

	virtual ~OrderViolationThread()
	{
		clReleaseKernel(m_kernel1);
		clReleaseKernel(m_kernel2);
	}

	bool GetResult() const {return m_bResult;}

protected:
	virtual void ThreadRoutine();

	cl_context       m_context;
	cl_command_queue m_queue;
	cl_program       m_program;
	cl_kernel        m_kernel1;
	cl_kernel        m_kernel2;
	cl_mem           m_param;
	int				m_id;
	bool			m_bResult;
};

void OrderViolationThread::ThreadRoutine()
{
	cl_int       err;
	const size_t globalSize   = 1;

	cl_char* mapPtr = (cl_char*)clEnqueueMapBuffer(m_queue, m_param, CL_TRUE, CL_MAP_WRITE, 0, sizeof(cl_char), 0, NULL, NULL, &err);
	SilentCheck("clEnqueueMapBuffer", CL_SUCCESS, err);
	*mapPtr = (cl_char)(0x80+m_id);

	err = clEnqueueUnmapMemObject(m_queue, m_param, mapPtr, 0, NULL, NULL);
	SilentCheck("clEnqueueUnmapMemObject", CL_SUCCESS, err);

    // the second unmap should fail
    err = clEnqueueUnmapMemObject(m_queue, m_param, mapPtr, 0, NULL, NULL);
    SilentCheck("clEnqueueUnmapMemObject", CL_INVALID_VALUE, err);

	err = clEnqueueNDRangeKernel(m_queue, m_kernel1, 1, NULL, &globalSize, NULL, 0, NULL, NULL);
	SilentCheck("clEnqueueNDRangeKernel(1)", CL_SUCCESS, err);
	err = clEnqueueNDRangeKernel(m_queue, m_kernel2, 1, NULL, &globalSize, NULL, 0, NULL, NULL);
	SilentCheck("clEnqueueNDRangeKernel(2)", CL_SUCCESS, err);

	mapPtr = (cl_char*)clEnqueueMapBuffer(m_queue, m_param, CL_TRUE, CL_MAP_READ, 0, sizeof(cl_char), 0, NULL, NULL, &err);
	SilentCheck("clEnqueueMapBuffer", CL_SUCCESS, err);
	cl_char val = *mapPtr;
	m_bResult = (val == (cl_char)m_id);
	if ( !m_bResult )
	{
		printf("Validation failed on id=%d, expected %x, got %x\n", m_id, (unsigned int)m_id, (unsigned int)val);
	}

	err = clEnqueueUnmapMemObject(m_queue, m_param, mapPtr, 0, NULL, NULL);
	SilentCheck("clEnqueueUnmapMemObject", CL_SUCCESS, err);
}

bool MultithreadedOrderViolation()
{
	bool bResult = true;
	cl_int iRet  = CL_SUCCESS;

	const size_t numThreads     = 32;

	cl_platform_id platform     = 0;
	cl_device_id   deviceId     = 0;

	cl_context           context;
	cl_command_queue     queue;
	cl_program           program;
	cl_mem               params[numThreads];

	cl_char              data[numThreads];

	const char* programSource    = "__kernel void k2(__global char* p, char id) { if ((char)(0x40+id) == p[get_global_id(0)])\
													{p[get_global_id(0)] = id;} else {p[get_global_id(0)] = (char)(0x20+id);} }\
								   __kernel void k1(__global char* p, char id) { if ( (char)(0x80+id) == p[get_global_id(0)] )\
													{p[get_global_id(0)] = (char)(0x40+id);} else  {p[get_global_id(0)] = (char)(0x60+id);} }";

	printf("Begin order violation test\n");

	iRet = clGetPlatformIDs(1, &platform, NULL);
	bResult &= SilentCheck("clGetPlatformIDs", CL_SUCCESS, iRet);

	if (!bResult)
	{
		return bResult;
	}

	cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };

	//Creation phase
	context  = clCreateContextFromType(prop, gDeviceType, NULL, NULL, &iRet);
	bResult &= SilentCheck("Create Context from type (gDeviceType)", CL_SUCCESS, iRet);

	iRet     = clGetDeviceIDs(platform, gDeviceType, 1, &deviceId, NULL);
	bResult &= SilentCheck("Get device ID (gDeviceType)", CL_SUCCESS, iRet);

	queue    = clCreateCommandQueue(context, deviceId, 0, &iRet);
	bResult &= SilentCheck("Create command queue", CL_SUCCESS, iRet);

	program  = clCreateProgramWithSource(context, 1, &programSource, NULL, &iRet);
	bResult &= SilentCheck("Create program with source", CL_SUCCESS, iRet);

	iRet     = clBuildProgram(program, 1, &deviceId, NULL, NULL, NULL);
	bResult &= SilentCheck("Build program", CL_SUCCESS, iRet);

	if (!bResult)
	{
		return false;
	}

	for (size_t i = 0; i < numThreads; ++i)
	{
		params[i] = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(cl_char), &data[i], &iRet);
		bResult &= SilentCheck("Build clCreateBuffer", CL_SUCCESS, iRet);

	}
	if (!bResult)
	{
		return false;
	}

	SynchronizedThread* threads[numThreads];

	for (size_t i = 0; i < numThreads; ++i)
	{
		threads[i] = new OrderViolationThread((int)i,  context, queue, program, params[i]);
	}

	for (size_t j=0; j<100 && bResult; j++ )
	{
		if ( (j+1) % 10 == 0 )
		{
			printf(".");
		}

		SynchronizedThreadPool pool;
		pool.Init(threads, numThreads);
		pool.StartAll();
		pool.WaitAll();

		clFinish(queue);

		for (size_t i = 0; i < numThreads && bResult; ++i)
		{
			bResult &= static_cast<OrderViolationThread*>(threads[i])->GetResult();
			bResult &= ((cl_char)i == data[i]);
			if ( !bResult )
			{
				printf("\nValidation failed, it: %u, thrd: %u\n", (unsigned int)j, (unsigned int)i);
			}
		}

	}

	for (size_t i = 0; i < numThreads; ++i)
	{
		delete threads[i];
		clReleaseMemObject(params[i]);
	}

	clReleaseProgram(program);
	clFinish(queue);
	clReleaseCommandQueue(queue);
	clReleaseContext(context);

	printf("\nEnd order violation test, %d\n", (int)bResult);

	return bResult;
}


/* Threads for MT order violation test */
class BuildProgramThread : public SynchronizedThread
{
public:
	BuildProgramThread(int id, cl_context context, cl_device_id deviceId) :
	  m_id(id), m_context(context), m_deviceId(deviceId), m_bResult(false)
	{
    }

	virtual ~BuildProgramThread()
	{
	}

	bool GetResult() const {return m_bResult;}

protected:
	virtual void ThreadRoutine();

	int				m_id;
	cl_context      m_context;
	cl_device_id	m_deviceId;
	bool			m_bResult;

	static const char* s_programSource;
};

const char* BuildProgramThread::s_programSource    =
		"__kernel void k2(__global char* p, char id) { if ((char)(0x40+id) == p[get_global_id(0)])\
			{p[get_global_id(0)] = id;} else {p[get_global_id(0)] = (char)(0x20+id);} }";

void BuildProgramThread::ThreadRoutine()
{
	cl_program       program;
	cl_int iRet;

	program  = clCreateProgramWithSource(m_context, 1, &s_programSource, NULL, &iRet);
	m_bResult = SilentCheck("Create program with source", CL_SUCCESS, iRet);
	if ( !m_bResult )
	{
		return;
	}

	iRet     = clBuildProgram(program, 1, &m_deviceId, NULL, NULL, NULL);
	m_bResult &= SilentCheck("Build program", CL_SUCCESS, iRet);

	clReleaseProgram(program);
}

bool MultithreadedBuildTest()
{
	bool bResult = true;
	cl_int iRet  = CL_SUCCESS;

	const size_t numThreads     = 1;

	cl_platform_id platform     = 0;
	cl_device_id   deviceId     = 0;

	cl_context           context;

	printf("Begin Multi-Threaded build test\n");

	iRet = clGetPlatformIDs(1, &platform, NULL);
	bResult &= SilentCheck("clGetPlatformIDs", CL_SUCCESS, iRet);
	if (!bResult)
	{
		return bResult;
	}

	cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };

	//Creation phase
	context  = clCreateContextFromType(prop, gDeviceType, NULL, NULL, &iRet);
	bResult &= SilentCheck("Create Context from type (gDeviceType)", CL_SUCCESS, iRet);

	iRet     = clGetDeviceIDs(platform, gDeviceType, 1, &deviceId, NULL);
	bResult &= SilentCheck("Get device ID (gDeviceType)", CL_SUCCESS, iRet);


	SynchronizedThread* threads[numThreads];

	for (size_t i = 0; i < numThreads; ++i)
	{
		threads[i] = new BuildProgramThread((int)i,  context, deviceId);
	}

	SynchronizedThreadPool pool;
	pool.Init(threads, numThreads);
	pool.StartAll();
	pool.WaitAll();

	for (size_t i = 0; i < numThreads && bResult; ++i)
	{
		bResult &= static_cast<BuildProgramThread*>(threads[i])->GetResult();
	}

	clReleaseContext(context);
	return bResult;
}
