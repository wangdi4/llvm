#include "CL/cl.h"
#include "cl_types.h"
#include <stdio.h>
#include "FrameworkTest.h"
#include <gtest/gtest.h>
#include <string>
#include <assert.h>
#include <map>


// The following tests replace the old "main" function of framework_test_type.
//

//BC open functions:
void cpuBCOpen(FILE*& pIRFile){
  FOPEN(pIRFile, "test.bc", "rb");
}

void micBCOpen(FILE*& pIRFile){
  FOPEN(pIRFile, "mic_test.bc", "rb");
}

// CL_DEVICE_TYPE_CPU is the default device.
cl_device_type gDeviceType = CL_DEVICE_TYPE_CPU;

std::map<cl_device_type, openBcFunc> gBcfuncMap;

TEST(FrameworkTestType, Test_clfissionBasicTest)
{
	EXPECT_TRUE(fission_basic_test());
}

TEST(FrameworkTestType, Test_clFissionReadBufferTest)
{
    EXPECT_TRUE(fission_read_buffer_test());
}

TEST(FrameworkTestType, Test_clfissionOptionsTest)
{
	EXPECT_TRUE(fission_options_test());
}

TEST(FrameworkTestType, Test_clfissionLogicTest)
{
	EXPECT_TRUE(fission_logic_test());
}

TEST(FrameworkTestType, Test_clfissionBuildTest)
{
	EXPECT_TRUE(fission_buildSubDevice_test());
}

TEST(FrameworkTestType, Test_clfissionBuildMultipleTest)
{
	EXPECT_TRUE(fission_buildMultipleSubDevices_test());
}

TEST(FrameworkTestType, Test_clfissionDeviceInfoTest)
{
	EXPECT_TRUE(fission_deviceInfoSelectors_test());
}

TEST(FrameworkTestType, Test_clfissionCommandQTest)
{
	EXPECT_TRUE(fission_commandQ_test());
}

TEST(FrameworkTestType, Test_clfissionContextTest)
{
	EXPECT_TRUE(fission_context_test());
}

TEST(FrameworkTestType, Test_clfissionSubdivisionTest)
{
	EXPECT_TRUE(fission_subdivision_test());
}

TEST(FrameworkTestType, Test_clfissionTwoQueuesTest)
{
	EXPECT_TRUE(fission_two_queues_test());
}

TEST(FrameworkTestType, Test_clfissionByNamesTest)
{
	EXPECT_TRUE(fission_by_names_test());
}
TEST(FrameworkTestType, Test_clfissionReadBufferBetweenDevicesTest)
{
	EXPECT_TRUE(fission_read_buffer_between_device_test());
}

TEST(FrameworkTestType, Test_clGetDeviceIDsTest)
{
    EXPECT_TRUE(clGetDeviceIDsTest());
}


TEST(FrameworkTestType, Test_clGetPlatformInfoTest)
{
    EXPECT_TRUE(clGetPlatformInfoTest());
}


TEST(FrameworkTestType, Test_clGetDeviceInfoTest)
{
    EXPECT_TRUE(clGetDeviceInfoTest());
}


TEST(FrameworkTestType, Test_clOutOfOrderTest)
{
    EXPECT_TRUE(clOutOfOrderTest());
}


TEST(FrameworkTestType, Test_clCreateContextTest)
{
    EXPECT_TRUE(clCreateContextTest());
}


TEST(FrameworkTestType, Test_clBuildProgramWithBinaryTest)
{
    std::map<cl_device_type, openBcFunc>::iterator iter = gBcfuncMap.find(gDeviceType);
	if (gBcfuncMap.end() == iter)
	{
		FAIL();
	}
    EXPECT_TRUE(clBuildProgramWithBinaryTest(iter->second));
}

TEST(FrameworkTestType, Test_clBuildProgramWithSourceTest)
{
    EXPECT_TRUE(clBuildProgramWithSourceTest());
}

TEST(FrameworkTestType, Test_clBuildProgramTwiceTest)
{
    EXPECT_TRUE(clBuildProgramTwiceTest());
}

TEST(FrameworkTestType, Test_clBuildEmptyProgramTest)
{
    EXPECT_TRUE(clBuildEmptyProgramTest());
}

TEST(FrameworkTestType, Test_clLinkProgramTest)
{
    EXPECT_TRUE(clLinkProgramTest());
}

TEST(FrameworkTestType, Test_clCreateKernelTest)
{
	std::map<cl_device_type, openBcFunc>::iterator iter = gBcfuncMap.find(gDeviceType);
	if (gBcfuncMap.end() == iter)
	{
		FAIL();
	}
    EXPECT_TRUE(clCreateKernelTest(iter->second));
}

TEST(FrameworkTestType, Test_clGetKernelArgInfoTest)
{
    EXPECT_TRUE(clGetKernelArgInfoTest());
}

TEST(FrameworkTestType, Test_clCreateBufferTest)
{
    EXPECT_TRUE(clCreateBufferTest());
}

TEST(FrameworkTestType, Test_clCreateSubBufferTest)
{
    EXPECT_TRUE(clCreateSubBufferTest());
}

TEST(FrameworkTestType, Test_clEnqueueRWBuffer)
{
    EXPECT_TRUE(clEnqueueRWBuffer());
}

TEST(FrameworkTestType, Test_clImagePermissions)
{
    EXPECT_TRUE(clImagePermissions());
}

TEST(FrameworkTestType, Test_clExecutionTest)
{
    EXPECT_TRUE(clExecutionTest());
}

TEST(FrameworkTestType, Test_clOODotProductTest)
{
    EXPECT_TRUE(clOODotProductTest(1));
}

TEST(FrameworkTestType, Test_clEnqueueCopyBufferTest)
{
    EXPECT_TRUE(clEnqueueCopyBufferTest());
}


TEST(FrameworkTestType, Test_clMapBufferTest)
{
    EXPECT_TRUE(clMapBufferTest());
}


TEST(FrameworkTestType, Test_clCopyImageTest)
{
    EXPECT_TRUE(clCopyImageTest());
}


TEST(FrameworkTestType, Test_clBuildProgramMaxArgsTest)
{
    EXPECT_TRUE(clBuildProgramMaxArgsTest());
}


TEST(FrameworkTestType, Test_clKernelAttributesTest)
{
    EXPECT_TRUE(clKernelAttributesTest());
}


TEST(FrameworkTestType, Test_clKernelBarrierTest)
{
    EXPECT_TRUE(clKernelBarrierTest());
}

TEST(FrameworkTestType, Test_clCreateImageTest)
{
    EXPECT_TRUE(clCreateImageTest());
}

TEST(FrameworkTestType, Test_clImageExecuteTest)
{
    EXPECT_TRUE(clImageExecuteTest());
}


TEST(FrameworkTestType, Test_clIntegerExecuteTest)
{
    EXPECT_TRUE(clIntegerExecuteTest());
}

TEST(FrameworkTestType, Test_clWorkItemFunctionsTest)
{
    EXPECT_TRUE(clWorkItemFunctionsTest());
}


TEST(FrameworkTestType, Test_clMathExecuteTest)
{
    EXPECT_TRUE(clMathExecuteTest());
}


TEST(FrameworkTestType, Test_memset_test)
{
    EXPECT_TRUE(memset_test());
}


TEST(FrameworkTestType, Test_MT_context_retain)
{
    EXPECT_TRUE(MultithreadedContextRefCount());
}


TEST(FrameworkTestType, Test_MT_release)
{
    EXPECT_TRUE(MultithreadedReleaseObjects());
}


TEST(FrameworkTestType, Test_MT_execution)
{
    EXPECT_TRUE(ConcurrentExecutionTest());
}


TEST(FrameworkTestType, Test_ConcurrentBuildProgramTest)
{
    EXPECT_TRUE(ConcurrentBuildProgramTest());
}


//#ifndef _M_X64
TEST(FrameworkTestType, Test_clNativeFunctionTest)
{
    EXPECT_TRUE(clNativeFunctionTest());
}
//#endif // _M_X64


TEST(FrameworkTestType, Test_clRelaxedFunctionTest)
{
    EXPECT_TRUE(clRelaxedFunctionTest());
}


TEST(FrameworkTestType, Test_EnqueueNativeKernelTest)
{
    EXPECT_TRUE(EnqueueNativeKernelTest());
}


TEST(FrameworkTestType, Test_TBB)
{
    EXPECT_TRUE(TBBTest());
}


TEST(FrameworkTestType, Test_clLocalStructTest)
{
    EXPECT_TRUE(clLocalStructTest());
}

TEST(FrameworkTestType, Test_opencl_printf_test)
{
    EXPECT_TRUE(opencl_printf_test());
}

TEST(FrameworkTestType, Test_VecTypeHintTest)
{
    EXPECT_TRUE(VecTypeHintTest());
}


TEST(FrameworkTestType, Test_EventCallbackTest)
{
    EXPECT_TRUE(EventCallbackTest());
}


TEST(FrameworkTestType, Test_MT_order)
{
    EXPECT_TRUE(MultithreadedOrderViolation());
}

// Because we don't copy misaligned buffers with CL_MEM_USE_HOST_PTR anymore, the kernel code crashes. This can be fixed by changes in LLVM to handle misaligned access, if it is decided to be done.
TEST(FrameworkTestType, DISABLED_Test_MisalignedHostPtr)
{
    EXPECT_TRUE(MisalignedUseHostPtrTest());
}

TEST(FrameworkTestType, Test_userDefinedStruct)
{
    EXPECT_TRUE(clStructTest());
}

TEST(FrameworkTestType, Test_clapiTest)
{
	EXPECT_TRUE(api_test());
}
TEST(FrameworkTestType, Test_ImmediateExecutionTest)
{
    EXPECT_TRUE(immediateExecutionTest());
}

TEST(FrameworkTestType, Test_MT_build)
{
    EXPECT_TRUE(MultithreadedBuildTest());
}
TEST(FrameworkTestType, Test_CreateReleaseOOOQueueTest)
{
    EXPECT_TRUE(CreateReleaseOOOQueueTest());
}

TEST(FrameworkTestType, Test_EventDependenciesTest)
{
    EXPECT_TRUE(EventDependenciesTest());
}

TEST(FrameworkTestType, Test_ShutdownFromChildThread)
{
	EXPECT_TRUE(ShutdownFromChildThread());
}

TEST(FrameworkTestType, Test_PredictablePartition)
{
	EXPECT_TRUE(predictable_partition_test());
}

TEST(FrameworkTestType, Test_clMultipleExecutionTest)
{
    EXPECT_TRUE(clMultipleExecutionTest());
}

TEST(FrameworkTestType, Test_clSvmTest)
{
	EXPECT_TRUE(clSvmTest());
}

TEST(FrameworkTestType, DISABLED_Test_clFlexibleNdrange)    // disabled until BE finish their implementation
{
	EXPECT_TRUE(clFlexibleNdrange());
}

TEST(FrameworkTestType, Test_clPipes)
{
	EXPECT_TRUE(clPipes());
}

TEST(FrameWorkTestType, Test_cl20ExecutionModel)
{
	EXPECT_TRUE(cl20ExecutionModel());
}

template <typename T>
class CommandLineOption{
  std::string m_name;

  T convertToT(const std::string&)const;
public:
  CommandLineOption(const char* cmdName);
  T getValue(const std::string&)const;
  bool isMatch(const std::string&)const;
};

template <typename T>
bool CommandLineOption<T>::isMatch(const std::string& s)const{
  size_t size = m_name.size();
  bool b = s.substr(0, size) == m_name;
  if (!b){
    printf("substr=%s\n", s.substr(0, size).c_str());
    printf("m_name=%s\n", m_name.c_str());
    printf("%s:%d\n", __FILE__, __LINE__);
    return false;
  }
  b = s.at(size) == '=';
  if (!b){
    printf("%s:%d\n", __FILE__, __LINE__);
    return false;
  }
  return true;
}

template <typename T>
CommandLineOption<T>::CommandLineOption(const char* cmdName):
  m_name(cmdName){
}

template <>
std::string CommandLineOption<std::string>::convertToT(const std::string& s)const{
  return s;
}

template<>
int CommandLineOption<int>::convertToT(const std::string& s)const{
  return atoi(s.c_str());
}

template<>
bool CommandLineOption<bool>::convertToT(const std::string& s)const{
  return s == "true" ? true : false;
}

template <typename T>
T CommandLineOption<T>::getValue(const std::string& cmdString)const{
  assert(isMatch(cmdString));
  size_t valueSeparator = cmdString.find('=');
  assert(std::string::npos != valueSeparator);
  return convertToT(cmdString.substr(valueSeparator+1, cmdString.size()));
}

CommandLineOption<std::string> deviceOption("--device_type");

#ifdef INCLUDE_MIC_DEVICE
TEST(FrameworkTestType, Test_CPU_MIC_IntegerExecute)
{
    EXPECT_TRUE(cl_CPU_MIC_IntegerExecuteTest());
}

TEST(FrameworkTestType, Test_CPU_MIC_Migrate)
{
    EXPECT_TRUE(cl_CPU_MIC_MigrateTest());
}

TEST(FrameworkTestType, Test_CPU_MIC_MapUnmap_InOrder)
{
    EXPECT_TRUE(cl_CPU_MIC_MapUnmapTest_InOrder());
}

TEST(FrameworkTestType, Test_CPU_MIC_MapUnmap_OutOfOrder)
{
    EXPECT_TRUE(cl_CPU_MIC_MapUnmapTest_OutOfOrder());
}

TEST(FrameworkTestType, Test_CPU_MIC_CommonRT_SubBuffers_Async)
{
    EXPECT_TRUE(cl_CPU_MIC_Common_RT_SubBuffers_Async());
}

TEST(FrameworkTestType, Test_CPU_MIC_Common_RT_SubBuffers_Async_With_Buffer_Release)
{
    EXPECT_TRUE(cl_CPU_MIC_Common_RT_SubBuffers_Async_With_Buffer_Release());
}

TEST(FrameworkTestType, cl_CPU_MIC_Parallel_NDRange_Execution_With_Read_Of_Same_Buffer)
{
	EXPECT_TRUE(cl_CPU_MIC_Parallel_NDRange_Execution_With_Read_Of_Same_Buffer());
}

TEST(FrameworkTestType, DISABLED_cl_ALL_Devices_SubBuffer_Simple_Test) //CSSD100015775
{
    EXPECT_TRUE(cl_ALL_Devices_SubBuffer_Simple_Test());
}

TEST(FrameworkTestType, DISABLED_cl_ALL_Devices_Common_RT_SubBuffers_Async) //CSSD100015775
{
    EXPECT_TRUE(cl_ALL_Devices_Common_RT_SubBuffers_Async());
}

TEST(FrameworkTestType, DISABLED_cl_ALL_Devices_Common_RT_SubBuffers_Async_With_Buffer_Release) //CSSD100015775
{
    EXPECT_TRUE(cl_ALL_Devices_Common_RT_SubBuffers_Async_With_Buffer_Release());
}

TEST(FrameworkTestType, TEST_APFLevel)
{
    EXPECT_TRUE(cl_APFLevelForce());
}

#endif

// To run individual tests, use the --gtest_filter=<pattern> command-line 
// option. For example, to only Test_EventCallbackTest, use:
// --gtest_filter=Test_EventCallbackTest
//
// To run all tests whose names end with ExecuteTest, use:
// --gtest_filter=*ExecuteTest
//
// Read the gtest documentation for more information.
//
int main(int argc, char** argv)
{
	std::map<std::string, cl_device_type> clDeviceTypeMap;
    gBcfuncMap[CL_DEVICE_TYPE_CPU] = cpuBCOpen;
    gBcfuncMap[CL_DEVICE_TYPE_ACCELERATOR] = micBCOpen;
	gBcfuncMap[CL_DEVICE_TYPE_GPU] = cpuBCOpen;
	gBcfuncMap[CL_DEVICE_TYPE_DEFAULT] = cpuBCOpen;
	gBcfuncMap[CL_DEVICE_TYPE_ALL] = cpuBCOpen;
	clDeviceTypeMap["cpu"] = CL_DEVICE_TYPE_CPU;
	clDeviceTypeMap["mic"] = CL_DEVICE_TYPE_ACCELERATOR;
	clDeviceTypeMap["gpu"] = CL_DEVICE_TYPE_GPU;
	clDeviceTypeMap["default"] = CL_DEVICE_TYPE_DEFAULT;
	clDeviceTypeMap["all"] = CL_DEVICE_TYPE_ALL;
    ::testing::InitGoogleTest(&argc, argv);
    if (argc > 1) {//are there still arguments left?
      for (int i=1 ; i<argc ; i++)
        if (deviceOption.isMatch(argv[i]))
		{
		  std::string deviceTypeStr = deviceOption.getValue(argv[i]);
		  std::map<std::string, cl_device_type>::iterator iter = clDeviceTypeMap.find(deviceTypeStr);
		  if (iter == clDeviceTypeMap.end())
		  {
              printf("error: unkown device option: %s\n", deviceTypeStr.c_str());
              return 1;
		  }
		  gDeviceType = iter->second;
		}
    }
   return RUN_ALL_TESTS();
}


