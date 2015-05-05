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
	#define RETURN_TYPE_ENTRY_POINT unsigned int
	#define FPRINTF fprintf_s
#else
	#if defined(__ANDROID__)
		#define SET_FPOS_T(var, val) (var) = (val)
		#define GET_FPOS_T(var) var
		#define STDCALL
	#else
		#define SET_FPOS_T(var, val) ((var).__pos = (val))
		#define GET_FPOS_T(var) ((var).__pos)
		#define STDCALL
	#endif // __ANDROID__
	#define SLEEP(mili) usleep(mili * 1000)
	#define FOPEN(file, name, mode) (file) = fopen((name), (mode))
	#define ABS64(x) llabs(x)
	#define RETURN_TYPE_ENTRY_POINT void *
	#define FPRINTF fprintf
#endif

typedef void (*openBcFunc)(FILE*&);

bool CheckHandle(const wchar_t * name, cl_platform_id expected, cl_platform_id result);
bool CheckHandle(const wchar_t * name, cl_device_id expected, cl_device_id result);
bool CheckHandle(const wchar_t * name, cl_context expected, cl_context result);
bool CheckHandle(const wchar_t * name, cl_command_queue expected, cl_command_queue result);
bool CheckHandle(const wchar_t * name, cl_mem expected, cl_mem result);
bool CheckHandle(const wchar_t * name, cl_program expected, cl_program result);
bool CheckHandle(const wchar_t * name, cl_kernel expected, cl_kernel result);
bool CheckHandle(const wchar_t * name, cl_event expected, cl_event result);
bool CheckHandle(const wchar_t * name, cl_sampler expected, cl_sampler result);
bool CheckHandleImpl(const wchar_t * name, void* expected, void* result, bool bRes);


bool clCreateContextTest();
bool clGetPlatformInfoTest();
bool clGetDeviceInfoTest();
bool clBuildProgramWithSourceTest();
bool clCheckJITSaveLoadTest();
bool clCheckJITSaveTest();
bool clCheckJITLoadTest();
bool clBuildProgramWithBinaryTest(openBcFunc pFunc);
bool clBuildProgramTwiceTest();
bool clBuildEmptyProgramTest();
bool clLinkProgramTest();
bool clCreateKernelTest(openBcFunc pFunc);
bool clExecutionTest();
bool clCreateBufferTest();
bool clCreateSubBufferTest();
bool clEnqueueRWBuffer();
bool clFastRelaxedMathModeTest();
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
bool VectorizerModeTest();
bool EventCallbackTest();
bool opencl_printf_test();
bool MisalignedUseHostPtrTest();
bool clIntelOfflineCompilerThreadsTest();
bool clStructTest();
bool clIntelOfflineCompilerBuildOptionsTest();
bool fission_basic_test();
bool fission_read_buffer_test();
bool fission_options_test();
bool fission_logic_test();
bool fission_buildSubDevice_test();
bool fission_buildMultipleSubDevices_test();
bool fission_deviceInfoSelectors_test();
bool fission_commandQ_test();
bool fission_context_test();
bool fission_subdivision_test();
bool fission_two_queues_test();
bool fission_by_names_test();
bool fission_read_buffer_between_device_test();
bool api_test();
bool immediateExecutionTest();
bool clCreateImageTest();
bool MultithreadedBuildTest();
bool EventDependenciesTest();
bool CreateReleaseOOOQueueTest();
bool clGetKernelArgInfoTest();
bool ShutdownFromChildThread();
bool predictable_partition_test();
bool clMultipleExecutionTest();
bool cl20ExecutionModel();
bool cl_CPU_MIC_IntegerExecuteTest();
bool cl_CPU_MIC_MigrateTest();
bool cl_CPU_MIC_MapUnmapTest_InOrder();
bool cl_CPU_MIC_MapUnmapTest_OutOfOrder();
bool cl_CPU_MIC_Common_RT_SubBuffers_Async();
bool cl_CPU_MIC_Common_RT_SubBuffers_Async_With_Buffer_Release();
bool cl_CPU_MIC_Parallel_NDRange_Execution_With_Read_Of_Same_Buffer();
bool cl_ALL_Devices_SubBuffer_Simple_Test();
bool cl_ALL_Devices_Common_RT_SubBuffers_Async();
bool cl_ALL_Devices_Common_RT_SubBuffers_Async_With_Buffer_Release();
bool cl_APFLevelForce();
bool cl_GenStats();
bool clDoNotVectorizeUnreachable();
bool clSvmTest();
bool clFlexibleNdrange();
bool clPipes();
bool clSampler();
bool clAoSFieldScatterGather();
bool clCheckVectorizingDim1And2AndUniteWG(int progIndex, bool hasLocalWGSize);
bool clCheckVectorizingOnAllDimAndCantUniteWG(int progIndex, bool oddDimention, bool hasLocalWGSize);
//#define CUDA_DEVICE
