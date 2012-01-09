#include "CL/cl.h"
#include "cl_types.h"
#include "Logger.h"
#include "cl_objects_map.h"
#include <stdio.h>
#include "FrameworkTest.h"
#include <gtest/gtest.h>


// The following tests replace the old "main" function of framework_test_type.
//

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
    EXPECT_TRUE(clBuildProgramWithBinaryTest());
}


TEST(FrameworkTestType, Test_clBuildProgramWithSourceTest)
{
    EXPECT_TRUE(clBuildProgramWithSourceTest());
}


TEST(FrameworkTestType, Test_clCreateKernelTest)
{
    EXPECT_TRUE(clCreateKernelTest());
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


TEST(FrameworkTestType, DISABLED_Test_clFinishTest)
{
    EXPECT_TRUE(clFinishTest());
}


TEST(FrameworkTestType, DISABLED_Test_MT_order)
{
	  //Disabling. The test exposes real problems. See: CSSD100012184 and CSSD100012035
    EXPECT_TRUE(MultithreadedOrderViolation());
}


TEST(FrameworkTestType, Test_MisalignedHostPtr)
{
    EXPECT_TRUE(MisalignedUseHostPtrTest());
}


TEST(FrameworkTestType, Test_overloadingTest)
{
    EXPECT_TRUE(overloading_test());
}

TEST(FrameworkTestType, Test_userDefinedStruct)
{
    EXPECT_TRUE(clStructTest());
}

TEST(IocTests, Threads)
{
	EXPECT_TRUE(clIntelOfflineCompilerThreadsTest());
}

TEST(IocTests, BuildOptions)
{
	EXPECT_TRUE(clIntelOfflineCompilerBuildOptionsTest());
}

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

TEST(FrameworkTestType, DISABLED_Test_clfissionLogicTest)
{
    // Disabled because it's buggy, needs to be rewritten.
	EXPECT_TRUE(fission_logic_test());
}

TEST(FrameworkTestType, DISABLED_Test_clfissionThreadTest)
{
    // Disabled because it's buggy, needs to be rewritten.
    // As it is it "usually" works but can randomly fail when it in fact passed
    // CQ ticket CSSD100006157
	EXPECT_TRUE(fission_thread_test());
}

TEST(FrameworkTestType, Test_clfissionBuildTest)
{
	EXPECT_TRUE(fission_buildSubDevice_test());
}

TEST(FrameworkTestType, Test_clfissionDeviceInfoTest)
{
	EXPECT_TRUE(fission_deviceInfoSelectors_test());
}

TEST(FrameworkTestType, Test_clfissionCommandQTest)
{
	EXPECT_TRUE(fission_commandQ_test());
}

TEST(FrameworkTestType, Test_clfissionNumaTest)
{
	EXPECT_TRUE(fission_numa_test());
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
    ::testing::InitGoogleTest(&argc, argv);
    int rc = RUN_ALL_TESTS();

    if (rc == 0) {
        printf("\n==============\nTEST SUCCEDDED\n==============\n");
        return 1;
    }
    else {
        printf("\n==============\nTEST FAILED\n==============\n");
        return 0;
    }

    return 1;
}


