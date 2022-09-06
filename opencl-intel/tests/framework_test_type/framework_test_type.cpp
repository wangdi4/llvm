#include "CL/cl.h"

#include <map>
#include <string>

#include "CL.h"
#include "CL20.h"
#include "CL21.h"
#include "IntelSubgroups.h"
#include "FrameworkTest.h"
#include "cl_env.h"
#include "cl_types.h"
#include "cl_user_logger.h"
#include "common_utils.h"
#include "options.hpp"

namespace Intel { namespace OpenCL { namespace Utils {

FrameworkUserLogger* g_pUserLogger = NULL;

}}}

// The following tests replace the old "main" function of framework_test_type.
//

//BC open functions:
void cpuBCOpen(FILE*& pIRFile){
    std::string filename = get_exe_dir() + "test.bc";
    FOPEN(pIRFile, filename.c_str(), "rb");
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

TEST(FrameworkTestType, DISABLED_Test_clfissionOptionsTest)
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

#ifdef _WIN32
TEST(FrameworkTestType, Test_clfissionByNamesTest)
{
    EXPECT_TRUE(fission_by_names_test());
}
#endif

TEST(FrameworkTestType, Test_clfissionReadBufferBetweenDevicesTest)
{
    EXPECT_TRUE(fission_read_buffer_between_device_test());
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

TEST(FrameworkTestType, Test_clBuildProgramWithBinaryTest)
{
    std::map<cl_device_type, openBcFunc>::iterator iter = gBcfuncMap.find(gDeviceType);
    if (gBcfuncMap.end() == iter)
    {
        FAIL();
    }
    EXPECT_TRUE(clBuildProgramWithBinaryTest(iter->second));
}

TEST(FrameworkTestType, Test_UnloadPlatformCompiler)
{
    EXPECT_TRUE(UnloadPlatformCompiler());
}

TEST(FrameworkTestType, Test_clBuildInvalidSpirProgramWithBinaryTest)
{
    EXPECT_TRUE(clBuildInvalidSpirProgramWithBinaryTest());
}

TEST(FrameworkTestType, Test_clBuildSpirvFriendlyIRProgramWithBinaryTest)
{
    EXPECT_TRUE(clBuildSpirvFriendlyIRProgramWithBinaryTest());
}

TEST(FrameworkTestType, Test_clCheckJITSaveLoadTest)
{
    EXPECT_TRUE(clCheckJITSaveLoadTest());
}

TEST(FrameworkTestType, DISABLED_Test_clCheckCPUArchForJIT)
{
    // GenerateBinaryFile before clCheckCPUArchForJIT
    // clCheckCPUArchForJIT cannot function w\o calling GenerateBinaryFile
    EXPECT_TRUE(GenerateBinaryFile());
    EXPECT_TRUE(clCheckCPUArchForJIT());
}

TEST(FrameworkTestType, Test_clBuildEmptyProgramTest)
{
    EXPECT_TRUE(clBuildEmptyProgramTest());
}

TEST(FrameworkTestType, Test_clLinkProgramTest)
{
    EXPECT_TRUE(clLinkProgramTest());
}

TEST(FrameworkTestType, Test_clGetProgramBuildInfoTest)
{
    ASSERT_NO_FATAL_FAILURE(clGetProgramBuildInfoTest());
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

TEST(FrameworkTestType, Test_clGetKernelArgInfoNotAvailableTest) {
    clGetKernelArgInfoNotAvailableTest();
}

TEST(FrameworkTestType, Test_clGetKernelArgInfoAfterLinkTest) {
    clGetKernelArgInfoAfterLinkTest();
}

TEST(FrameworkTestType, Test_clSetKernelArgInvalidArgSizeTest)
{
    clSetKernelArgInvalidArgSizeTest();
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

TEST(FrameworkTestType, Test_clCreateImageWithPropertiesTest)
{
    EXPECT_TRUE(clCreateImageWithPropertiesTest());
}

TEST(FrameworkTestType, Test_clCreateImageTestWithCPUArchSet)
{
    EXPECT_TRUE(clCreateImageTestWithCPUArchSet());
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

TEST(FrameworkTestType, Test_TBB)
{
    EXPECT_TRUE(TBBTest());
}

TEST(FrameworkTestType, DISABLED_Test_saturated_conversion_NaN_test)
{
    EXPECT_TRUE(saturated_conversion_NaN_test());
}

TEST(FrameworkTestType, Test_clLocalStructTest) { clLocalStructTest(); }

TEST(FrameworkTestType, Test_opencl_printf_test)
{
    EXPECT_TRUE(opencl_printf_test());
}

TEST(FrameworkTestType, Test_opencl_printf_floating_point_test)
{
  EXPECT_TRUE(opencl_printf_floating_point_test());
}

TEST(FrameworkTestType, Test_VecTypeHintTest)
{
    EXPECT_TRUE(VecTypeHintTest());
}

TEST(FrameworkTestType, DISABLED_Test_intelVecTypeHintTest)
{
    intelVecTypeHintTest();
}

TEST(FrameworkTestType, Test_intelReqdSubGroupSizeTest)
{
    intelReqdSubGroupSizeTest();
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

TEST(FrameworkTestType, DISABLED_Test_PredictablePartition)
{
    EXPECT_TRUE(predictable_partition_test());
}

// [CORC-3234] Disable the test as it is very compute-intensive and should be moved to Nightly.
TEST(FrameworkTestType, DISABLED_Test_clMultipleExecutionTest)
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

TEST(FrameworkTestType, Test_clSampler)
{
    EXPECT_TRUE(clSampler());
}

TEST(FrameWorkTestType, Test_cl20ExecutionModel)
{
    EXPECT_TRUE(cl20ExecutionModel());
}

TEST(FrameworkTestType, Test_clDoNotVectorizeUnreachable)
{
    EXPECT_TRUE(clDoNotVectorizeUnreachable());
}

TEST(FrameworkTestType, Test_cl_uniteWG_VectorizeOnDifferentDim)
{
    EXPECT_TRUE(clCheckVectorizingDim1And2AndUniteWG(0,false));
    EXPECT_TRUE(clCheckVectorizingDim1And2AndUniteWG(1,false));
    EXPECT_TRUE(clCheckVectorizingDim1And2AndUniteWG(0,true));
    EXPECT_TRUE(clCheckVectorizingDim1And2AndUniteWG(1,true));
    EXPECT_TRUE(clCheckVectorizingOnAllDimAndCantUniteWG(0,true,false));
    EXPECT_TRUE(clCheckVectorizingOnAllDimAndCantUniteWG(1,true,false));
    EXPECT_TRUE(clCheckVectorizingOnAllDimAndCantUniteWG(2,true,false));
    EXPECT_TRUE(clCheckVectorizingOnAllDimAndCantUniteWG(3,true,false));
    EXPECT_TRUE(clCheckVectorizingOnAllDimAndCantUniteWG(0,false,false));
    EXPECT_TRUE(clCheckVectorizingOnAllDimAndCantUniteWG(1,false,false));
    EXPECT_TRUE(clCheckVectorizingOnAllDimAndCantUniteWG(2,false,false));
    EXPECT_TRUE(clCheckVectorizingOnAllDimAndCantUniteWG(4,false,false));
    EXPECT_TRUE(clCheckVectorizingOnAllDimAndCantUniteWG(4,false,true));
}

TEST(FrameworkTestType, Test_CL11_Option)
{
    EXPECT_TRUE(clBuildWithCL11option());
}

TEST(FrameworkTestType, Test_clBuildOptions)
{
    clBuildOptionsTest();
}

TEST(FrameworkTestType, Test_clShutdownSVMTest)
{
    clShutdownSVMTest();
}

TEST(FrameworkTestType, Test_clTracingCheckExtensionsTest)
{
    EXPECT_TRUE(clTracingCheckExtensionsTest());
}

TEST(FrameworkTestType, Test_clTracingCheckExtensionsForPlatformTest)
{
    EXPECT_TRUE(clTracingCheckExtensionsForPlatformTest());
}

TEST(FrameworkTestType, Test_clTracingCheckInvalidArgsTest)
{
    EXPECT_TRUE(clTracingCheckInvalidArgsTest());
}

TEST(FrameworkTestType, Test_clTracingCheckInvactiveHandleTest)
{
    EXPECT_TRUE(clTracingCheckInvactiveHandleTest());
}

TEST(FrameworkTestType, Test_clTracingCheckTooManyHandlesTest)
{
    EXPECT_TRUE(clTracingCheckTooManyHandlesTest());
}

TEST(FrameworkTestType, Test_clTracingFlowCheckTest)
{
    EXPECT_TRUE(clTracingFlowCheckTest());
}

TEST(FrameworkTestType, Test_clTracingFunctionsEnabledCheckTest)
{
    EXPECT_TRUE(clTracingFunctionsEnabledCheckTest());
}

TEST(FrameworkTestType, Test_clTracingArgumentsChangedCheckTest)
{
    EXPECT_TRUE(clTracingArgumentsChangedCheckTest());
}

TEST(FrameworkTestType, Test_clTracingFunctionsDisabledCheckTest)
{
    EXPECT_TRUE(clTracingFunctionsDisabledCheckTest());
}

TEST(FrameworkTestType, Test_ClkEventAsKernelArg)
{
    EXPECT_TRUE(ClkEventAsKernelArg());
}

#if (!defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)) && !defined(_WIN32)
TEST(FrameworkTestType, Test_cl_DumpIRBeforeAfterPasses)
{
    cl_DumpIRBeforeAndAfterPasses();
}
#endif

// cl_intel_subgroup test.
TEST(INTEL_SUBGR, Test_GetKernelSubGroupInfoKHR) { clGetKernelSubGroupInfo(); }

// CL21 tests.
TEST(CL21_depr, Test_CloneKernel)
{
    EXPECT_TRUE(CloneKernel());
}

TEST(FrameworkTestType, cl_device_local_mem_size_test)
{
    EXPECT_TRUE(cl_device_local_mem_size_test());
}

#ifndef _WIN32
TEST(FrameworkTestType, cl_device_local_mem_size_unlimited_stack_test)
{
    EXPECT_TRUE(cl_device_local_mem_size_unlimited_stack_test());
}

TEST(FrameworkTestType, Test_TbbSetMaxThreads1)
{
    EXPECT_TRUE(TbbSetMaxThreads(1));
}

TEST(FrameworkTestType, Test_TbbSetMaxThreads2)
{
    EXPECT_TRUE(TbbSetMaxThreads(2));
}
#endif

TEST(FrameworkTestType, cl_CheckBuildNumber)
{
    EXPECT_TRUE(cl_CheckBuildNumber());
}

TEST(FrameworkTestType, Test_clFuncIncompatParamASOnLinkageTest)
{
    clFuncIncompatParamASOnLinkageTest();
}

TEST(FrameworkTestType, Test_clFuncWrongNumParamsOnLinkageTest)
{
    clFuncWrongNumParamsOnLinkageTest();
}

TEST(FrameworkTestType, Test_clFuncIdenticalLayoutStructOnLinkageTest) {
  clFuncIdenticalLayoutStructOnLinkageTest();
}

TEST(FrameworkTestType, Test_globalVariableQueryTest)
{
    globalVariableSizeQueryTest();
}

TEST(FrameworkTestType, Test_enqueueBlockProfilingTest)
{
    enqueueBlockProfilingTest();
}

TEST(FrameworkTestType, Test_clGetCommandQueueInfo)
{
    clGetCommandQueueInfo();
}

TEST(FrameworkTestType, Test_passBuildOptionByEnvTest) {
    passBuildOptionByEnvTest();
}

TEST(FrameworkTestType, Test_UniformWorkGroupTest)
{
    UniformWorkGroupTest();
}

TEST(FrameworkTestType, Test_clKernelLocalMemSizeQueryTest)
{
    clKernelLocalMemSizeQueryTest();
}

TEST(FrameworkTestType, Test_LinearSampleOOBCoord) {
  LinearSampleOOBCoord();
}

CommandLineOption<std::string> deviceOption("--device_type");

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
    gBcfuncMap[CL_DEVICE_TYPE_CPU] = cpuBCOpen;
    gBcfuncMap[CL_DEVICE_TYPE_ACCELERATOR] = cpuBCOpen;
    gBcfuncMap[CL_DEVICE_TYPE_GPU] = cpuBCOpen;
    gBcfuncMap[CL_DEVICE_TYPE_DEFAULT] = cpuBCOpen;
    gBcfuncMap[CL_DEVICE_TYPE_ALL] = cpuBCOpen;

    std::map<std::string, cl_device_type> clDeviceTypeMap;
    clDeviceTypeMap["cpu"] = CL_DEVICE_TYPE_CPU;
    clDeviceTypeMap["fpga-emu"] = CL_DEVICE_TYPE_ACCELERATOR;
    clDeviceTypeMap["gpu"] = CL_DEVICE_TYPE_GPU;
    clDeviceTypeMap["default"] = CL_DEVICE_TYPE_DEFAULT;
    clDeviceTypeMap["all"] = CL_DEVICE_TYPE_ALL;
    ::testing::InitGoogleTest(&argc, argv);

    std::string deviceTypeStr;
    if (argc > 1)
    { //are there still arguments left?
        for (int i=1 ; i<argc ; i++)
        {
            if (deviceOption.isMatch(argv[i]))
            {
                deviceTypeStr = deviceOption.getValue(argv[i]);
                std::map<std::string, cl_device_type>::iterator iter =
                    clDeviceTypeMap.find(deviceTypeStr);
                if (iter == clDeviceTypeMap.end())
                {
                    printf("error: unkown device option: %s\n",
                        deviceTypeStr.c_str());
                    return 1;
                }
                gDeviceType = iter->second;
            }
        }
    }

    if (GetEnv(deviceTypeStr, "CL_CONFIG_DEVICES"))
    {
        std::map<std::string, cl_device_type>::iterator iter =
            clDeviceTypeMap.find(deviceTypeStr);
        if (iter == clDeviceTypeMap.end())
        {
            printf("error: unknown value of CL_CONFIG_DEVICES env variable: %s\n",
                deviceTypeStr.c_str());
            return 1;
        }
        gDeviceType = iter->second;
    }

    return RUN_ALL_TESTS();
}
