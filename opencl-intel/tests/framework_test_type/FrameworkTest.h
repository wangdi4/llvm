#pragma once

#include <cl_utils.h>
#include <CL/cl_ext.h>

#ifndef _WIN32
#include <sys/types.h>
#endif
#include "test_utils.h"

#ifdef _WIN32
	#define STDCALL __stdcall
	#define SLEEP(mili) Sleep(mili)
	#define FOPEN(file, name, mode) fopen_s(&(file), (name), (mode))
	#define SET_FPOS_T(var, val) (var) = (val)
	#define GET_FPOS_T(var) var
	#define ABS64(x) _abs64(x)
	#define RETURN_TYPE_ENTRY_POINT int
	#define FPRINTF fprintf_s
#else
	#define STDCALL __attribute((stdcall))
	#define SLEEP(mili) usleep(mili * 1000)
	#define FOPEN(file, name, mode) (file) = fopen((name), (mode))
	#define SET_FPOS_T(var, val) ((var).__pos = (val))
	#define GET_FPOS_T(var) ((var).__pos)
	#define ABS64(x) llabs(x)
	#define RETURN_TYPE_ENTRY_POINT void *
	#define FPRINTF fprintf
#endif
bool CheckHandle(wchar_t * name, cl_platform_id expected, cl_platform_id result);
bool CheckHandle(wchar_t * name, cl_device_id expected, cl_device_id result);
bool CheckHandle(wchar_t * name, cl_context expected, cl_context result);
bool CheckHandle(wchar_t * name, cl_command_queue expected, cl_command_queue result);
bool CheckHandle(wchar_t * name, cl_mem expected, cl_mem result);
bool CheckHandle(wchar_t * name, cl_program expected, cl_program result);
bool CheckHandle(wchar_t * name, cl_kernel expected, cl_kernel result);
bool CheckHandle(wchar_t * name, cl_event expected, cl_event result);
bool CheckHandle(wchar_t * name, cl_sampler expected, cl_sampler result);
bool CheckHandleImpl(wchar_t * name, void* expected, void* result, bool bRes);


bool clCreateContextTest();
bool clGetPlatformInfoTest();
bool clGetDeviceInfoTest();
bool clGetDeviceIDsTest();
bool clBuildProgramWithSourceTest();
bool clBuildProgramWithBinaryTest();
bool clCreateKernelTest();
bool clExecutionTest();
bool clCreateBufferTest();
bool clCreateSubBufferTest();
bool clEnqueueRWBuffer();
bool clImagePermissions();
bool clOutOfOrderTest();
bool clOODotProductTest( int iNumLoops );
bool clQuickExecutionTest();
bool clEnqueueCopyBufferTest();
bool clMapBufferTest();
bool clCopyImageTest();
bool clBuildProgramMaxArgsTest();
bool clKernelAttributesTest();
bool clKernelBarrierTest();
bool clImageExecuteTest();
bool clIntegerExecuteTest();
bool clWorkItemFunctionsTest();
bool clMathExecuteTest();
bool reuse_mem_test();
bool memset_test();
bool MultithreadedContextRefCount();
bool clNativeFunctionTest();
bool clRelaxedFunctionTest();
bool ConcurrentBuildProgramTest();
bool MultithreadedReleaseObjects();
bool MultithreadedHelloWorld();
bool ConcurrentExecutionTest();
bool MultithreadedOrderViolation();
bool EnqueueNativeKernelTest();
bool TBBTest();
bool printf_test();
bool MultithreadedPrintf();
bool clLocalStructTest();
bool VecTypeHintTest();
bool EventCallbackTest();
bool clFinishTest();
bool opencl_printf_test();
bool MisalignedUseHostPtrTest();
bool overloading_test();
bool clIntelOfflineCompilerThreadsTest();
bool clStructTest();
bool clIntelOfflineCompilerBuildOptionsTest();
bool fission_basic_test();
bool fission_read_buffer_test();
bool fission_options_test();
bool fission_logic_test();
bool fission_thread_test();
bool fission_buildSubDevice_test();
bool fission_deviceInfoSelectors_test();
bool fission_commandQ_test();
bool fission_numa_test();
bool fission_context_test();
bool fission_subdivision_test();
bool fission_two_queues_test();
bool fission_by_names_test();
bool api_test();
bool immediateExecutionTest();
bool clCreateImageTest();
bool MultithreadedBuildTest();
bool EventDependenciesTest();
bool CreateReleaseOOOQueueTest();
//#define CUDA_DEVICE



