// Copyright (c) 2006-2012 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

///////////////////////////////////////////////////////////
//  CPUDeviceTest.cpp
///////////////////////////////////////////////////////////

#include "cl_sys_info.h"
#include "cl_utils.h"
#include "common_utils.h"
#include "cpu_dev_test.h"
#include "image_test.h"
#include "kernel_execute_test.h"
#include "logger_test.h"
#include "program.h"
#include "program_service_test.h"
#include "task_executor.h"

#include <assert.h>
#include <cl_device_api.h>
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <sched.h>
#endif

#define STR_LEN 100

using namespace Intel::OpenCL::TaskExecutor;


extern bool memoryTest(bool profiling);
extern bool mapTest();
extern bool AffinityRootDeviceTest(affinityMask_t* pMask);
extern bool AffinitySubDeviceTest(affinityMask_t* pMask);

IOCLDeviceAgent*		dev_entry;
cl_ulong profile_run = 0;
cl_ulong profile_complete = 0;
RTMemObjService localRTMemService;
affinityMask_t* pMask = NULL;

// Static variables for testing
static TestKernel_param_t	gNativeKernelParam;
volatile bool	gExecDone = true;

unsigned int gDeviceIdInType = 0;

namespace Intel { namespace OpenCL { namespace Utils {

FrameworkUserLogger* g_pUserLogger = NULL;

}}}

void CPUTestCallbacks::clDevCmdStatusChanged(cl_dev_cmd_id  cmd_id, void* data, cl_int cmd_status, cl_int completion_result, cl_ulong timer )
{
	unsigned int cmdId = (unsigned int)(size_t)cmd_id;

	printf("The command status changed %u status is %u, result %X\n", cmdId, cmd_status, completion_result);
	if ( CL_COMPLETE == cmd_status )
	{
		switch(cmdId)
		{
		case CL_DEV_CMD_EXEC_NATIVE:
		case CL_DEV_CMD_COPY:
		case CL_DEV_CMD_EXEC_KERNEL: case CL_DEV_CMD_EXEC_TASK: case CL_DEV_CMD_READ: case CL_DEV_CMD_WRITE:
		case CL_DEV_CMD_MAP: case CL_DEV_CMD_UNMAP:
			gExecDone = true;
			break;
		}
		profile_complete = timer;
		printf("Elapsed time is %lu nano second\n", profile_complete - profile_run);
	}
	if(CL_RUNNING == cmd_status)
	{
		profile_run = timer;
	}

	Intel::OpenCL::Utils::OclAutoMutex mutex(&m_mutex);
	for (std::set<IOCLFrameworkCallbacks*>::iterator iter = m_userCallbacks.begin(); iter != m_userCallbacks.end(); iter++)
	{
		(*iter)->clDevCmdStatusChanged(cmd_id, data, cmd_status, completion_result, timer);
	}
}

Intel::OpenCL::TaskExecutor::ITaskExecutor* CPUTestCallbacks::clDevGetTaskExecutor()
{
    return GetTaskExecutor();
}

//GetDeviceInfo with CL_DEVICE_TYPE test
bool clGetDeviceInfo_TypeTest()
{
	cl_device_type device_type;
	size_t param_value_size;

	cl_int iRes = clDevGetDeviceInfo(gDeviceIdInType, CL_DEVICE_TYPE, sizeof(cl_device_type), NULL, &param_value_size);
	if (CL_DEV_FAILED(iRes))
	{
		printf("clDevGetDeviceInfo failed: %s\n",clDevErr2Txt((cl_dev_err_code)iRes));
		return false;
	}

	iRes = clDevGetDeviceInfo(gDeviceIdInType, CL_DEVICE_TYPE, sizeof(cl_device_type), &device_type, &param_value_size);
	if (CL_DEV_FAILED(iRes))
	{
		printf("clDevGetDeviceInfo failed: %s\n",clDevErr2Txt((cl_dev_err_code)iRes));
		return false;
	}
	else
	{
		if (param_value_size != sizeof(cl_device_type))
		{
			printf("clDevGetDeviceInfo failed param value size is in wrong size: %zu instead of %zu\n",param_value_size,sizeof(cl_device_type));
			return false;
		}
		if (device_type != CL_DEVICE_TYPE_ACCELERATOR &&
				device_type != CL_DEVICE_TYPE_CPU)
		{
			printf("clDevGetDeviceInfo failed wrong device type %lu instead of %d\n", device_type,CL_DEVICE_TYPE_CPU);
			return false;
		}
		printf("clDevGetDeviceInfo succeeded\n");
		return true;
	}
}

bool clGetDeviceInfo_Test()
{

	size_t param_value_size;
	size_t image2DMaxWidth, image3DMaxDepth, profilingTimerResolution;
	cl_uint maxWorkItemDimension, preferredVecortWidthShort, maxClockFreq, globalMemCacheLineSize;
	cl_ulong globalMemCacheSize;
	bool ret = true;

	cl_int iRes;

	//CL_DEVICE_MAX_WORK_ITEM_DIMENSION test
	iRes = clDevGetDeviceInfo(gDeviceIdInType, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(maxWorkItemDimension), &maxWorkItemDimension, &param_value_size);
	if (CL_DEV_FAILED(iRes))
	{
		printf("clDevGetDeviceInfo failed: %s\n",clDevErr2Txt((cl_dev_err_code)iRes));
		ret =  false;
	}
	else
	{
		if(param_value_size != sizeof(maxWorkItemDimension))
		{
			printf("clDevGetDeviceInfo failed param value size is in wrong size: %zu instead of %zu\n",param_value_size,sizeof(cl_uint));
			ret =  false;
		}
		printf("maxWorkItemDimension %d\n", maxWorkItemDimension);
	}
	//CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT test
	iRes = clDevGetDeviceInfo(gDeviceIdInType, CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT, sizeof(preferredVecortWidthShort), &preferredVecortWidthShort, &param_value_size);
	if (CL_DEV_FAILED(iRes))
	{
		printf("clDevGetDeviceInfo failed: %s\n",clDevErr2Txt((cl_dev_err_code)iRes));
		ret =  false;
	}
	else
	{
		if(param_value_size != sizeof(preferredVecortWidthShort))
		{
			printf("clDevGetDeviceInfo failed param value size is in wrong size: %zu instead of %zu\n",param_value_size,sizeof(cl_uint));
			ret =  false;
		}
		printf("preferredVecortWidthShort %d\n", preferredVecortWidthShort);
	}

	//CL_DEVICE_MAX_CLOCK_FREQUENCY test
	iRes = clDevGetDeviceInfo(gDeviceIdInType, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(maxClockFreq), &maxClockFreq, &param_value_size);
	if (CL_DEV_FAILED(iRes))
	{
		printf("clDevGetDeviceInfo failed: %s\n",clDevErr2Txt((cl_dev_err_code)iRes));
		ret =  false;
	}
	else
	{
		if(param_value_size != sizeof(maxClockFreq))
		{
			printf("clDevGetDeviceInfo failed param value size is in wrong size: %zu instead of %zu\n",param_value_size,sizeof(cl_uint));
			ret =  false;
		}
		printf("maxClockFreq %d\n", maxClockFreq);
	}

	//CL_DEVICE_IMAGE2D_MAX_WIDTH test
	iRes = clDevGetDeviceInfo(gDeviceIdInType, CL_DEVICE_IMAGE2D_MAX_WIDTH, sizeof(image2DMaxWidth), &image2DMaxWidth, &param_value_size);
	if (CL_DEV_FAILED(iRes))
	{
		printf("clDevGetDeviceInfo failed: %s\n",clDevErr2Txt((cl_dev_err_code)iRes));
		ret =  false;
	}
	else
	{
		if(param_value_size != sizeof(image2DMaxWidth))
		{
			printf("clDevGetDeviceInfo failed param value size is in wrong size: %zu instead of %zu\n",param_value_size,sizeof(cl_uint));
			ret =  false;
		}
		printf("image2DMaxWidth %zu\n", image2DMaxWidth);
	}
	//CL_DEVICE_IMAGE3D_MAX_DEPTH test
	iRes = clDevGetDeviceInfo(gDeviceIdInType, CL_DEVICE_IMAGE3D_MAX_DEPTH, sizeof(image3DMaxDepth), &image3DMaxDepth, &param_value_size);
	if (CL_DEV_FAILED(iRes))
	{
		printf("clDevGetDeviceInfo failed: %s\n",clDevErr2Txt((cl_dev_err_code)iRes));
		ret =  false;
	}
	else
	{
		if(param_value_size != sizeof(image3DMaxDepth))
		{
			printf("clDevGetDeviceInfo failed param value size is in wrong size: %zu instead of %zu\n",param_value_size,sizeof(cl_uint));
			ret =  false;
		}
		printf("image3DMaxDepth %zu\n", image3DMaxDepth);
	}
	//CL_DEVICE_GLOBAL_MEM_CACHE_SIZE test
	iRes = clDevGetDeviceInfo(gDeviceIdInType, CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, sizeof(globalMemCacheSize), &globalMemCacheSize, &param_value_size);
	if (CL_DEV_FAILED(iRes))
	{
		printf("clDevGetDeviceInfo failed: %s\n",clDevErr2Txt((cl_dev_err_code)iRes));
		ret =  false;
	}
	else
	{
		if(param_value_size != sizeof(globalMemCacheSize))
		{
			printf("clDevGetDeviceInfo failed param value size is in wrong size: %zu instead of %zu\n",param_value_size,sizeof(cl_ulong));
			ret =  false;
		}
		printf("globalMemCacheSize %lu\n", globalMemCacheSize);
	}
	//CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE test
	iRes = clDevGetDeviceInfo(gDeviceIdInType, CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, sizeof(globalMemCacheLineSize), &globalMemCacheLineSize, &param_value_size);
	if (CL_DEV_FAILED(iRes))
	{
		printf("clDevGetDeviceInfo failed: %s\n",clDevErr2Txt((cl_dev_err_code)iRes));
		ret =  false;
	}
	else
	{
		if(param_value_size != sizeof(globalMemCacheLineSize))
		{
			printf("clDevGetDeviceInfo failed param value size is in wrong size: %zu instead of %zu\n",param_value_size,sizeof(globalMemCacheLineSize));
			ret =  false;
		}
		printf("globalMemCacheLineSize %u\n", globalMemCacheLineSize);
	}

	//CL_DEVICE_PROFILING_TIMER_RESOLUTION test
	iRes = clDevGetDeviceInfo(gDeviceIdInType, CL_DEVICE_PROFILING_TIMER_RESOLUTION, sizeof(profilingTimerResolution), &profilingTimerResolution, &param_value_size);
	if (CL_DEV_FAILED(iRes))
	{
		printf("clDevGetDeviceInfo failed: %s\n",clDevErr2Txt((cl_dev_err_code)iRes));
		ret =  false;
	}
	else
	{
		if(param_value_size != sizeof(profilingTimerResolution))
		{
			printf("clDevGetDeviceInfo failed param value size is in wrong size: %zu instead of %zu\n",param_value_size,sizeof(profilingTimerResolution));
			ret =  false;
		}
		printf("profilingTimerResolution %zu\n", profilingTimerResolution);
	}

	return ret;
}

//GetDeviceInfo with CL_DEVICE_VENDOR_ID test
bool clGetDeviceInfo_VendorIdTest()
{
	cl_uint uVendorId;
	size_t param_value_size;
	cl_uint input_size = sizeof(cl_uint);


	cl_int iRes = clDevGetDeviceInfo(gDeviceIdInType, CL_DEVICE_VENDOR_ID, input_size, &uVendorId, &param_value_size);
	if (CL_DEV_FAILED(iRes))
	{
		printf("clDevGetDeviceInfo failed: %s\n",clDevErr2Txt((cl_dev_err_code)iRes));
		return false;
	}
	else
	{
		if(param_value_size != input_size)
		{
			printf("clDevGetDeviceInfo failed param value size is in wrong size: %zu instead of %d\n",param_value_size,input_size);
			return false;
		}
		printf("Vendor id is %u\n", uVendorId);
		return true;
	}

}

//GetDeviceInfo with CL_DEVICE_MAX_COMPUTE_UNITS test
bool clGetDeviceInfo_MaxComputeUnitTest()
{
	cl_uint uCoreNum;
	size_t param_value_size;
	cl_uint input_size = sizeof(cl_uint);


	cl_int iRes = clDevGetDeviceInfo(gDeviceIdInType, CL_DEVICE_MAX_COMPUTE_UNITS, input_size, &uCoreNum, &param_value_size);
	if (CL_DEV_FAILED(iRes))
	{
		printf("clDevGetDeviceInfo failed: %s\n",clDevErr2Txt((cl_dev_err_code)iRes));
		return false;
	}
	else
	{
		if(param_value_size != input_size)
		{
			printf("clDevGetDeviceInfo failed param value size is in wrong size: %zu instead of %d\n",param_value_size,input_size);
			return false;
		}
		printf("Max Compute Units is %u\n", uCoreNum);
		return true;
	}

}

//GetDeviceInfo with CL_DEVICE_AVAILABLE test
bool clGetDeviceInfo_DeviceAvilable()
{
	cl_bool bDevice;
	cl_uint input_size = sizeof(cl_bool);


	cl_int iRes = clDevGetDeviceInfo(gDeviceIdInType, CL_DEVICE_AVAILABLE, input_size, &bDevice, NULL);
	if (CL_DEV_FAILED(iRes))
	{
		printf("clDevGetDeviceInfo failed: %s\n",clDevErr2Txt((cl_dev_err_code)iRes));
		return false;
	}
	else
	{
		if(bDevice)
		{
			printf("Device is avilable");
			return true;
		}
		else
		{
			printf("Device is not avilable");
			return false;

		}
		return true;
	}
}

//GetDeviceInfo with CL_DEVICE_EXECUTION_CAPABILITIES test
bool clGetDeviceInfo_DeviceExecutionProperties()
{
	cl_device_exec_capabilities capabilities;
	cl_uint input_size = sizeof(cl_device_exec_capabilities);


	cl_int iRes = clDevGetDeviceInfo(gDeviceIdInType, CL_DEVICE_EXECUTION_CAPABILITIES, input_size, &capabilities, NULL);
	if (CL_DEV_FAILED(iRes))
	{
		printf("clDevGetDeviceInfo failed: %s\n",clDevErr2Txt((cl_dev_err_code)iRes));
		return false;
	}
	else
	{

		if(capabilities & CL_EXEC_NATIVE_KERNEL)
		{
			printf("Device has CL_EXEC_NATIVE_FN_AS_KERNEL capabilities\n");
		}
		if(capabilities & CL_EXEC_KERNEL)
		{
			printf("Device has CL_EXEC_KERNEL capabilities\n");
			return true;
		}
		else
		{
			printf("Device doesnt have basic capabilities\n");
			//Return true for now
			return true;

		}
		return true;
	}

}

//GetDeviceInfo with CL_DEVICE_QUEUE_PROPERTIES test
bool clGetDeviceInfo_QueueProperties()
{
	cl_command_queue_properties queueProperties;
	cl_uint input_size = sizeof(cl_command_queue_properties);


	cl_int iRes = clDevGetDeviceInfo(gDeviceIdInType, CL_DEVICE_QUEUE_PROPERTIES, input_size, &queueProperties, NULL);
	if (CL_DEV_FAILED(iRes))
	{
		printf("clDevGetDeviceInfo failed: %s\n",clDevErr2Txt((cl_dev_err_code)iRes));
		return false;
	}
	else
	{

		if(queueProperties & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE)
		{
			printf("Device has CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE queue properties\n");
		}
		if(queueProperties & CL_QUEUE_PROFILING_ENABLE)
		{
			printf("Device has CL_QUEUE_PROFILING_ENABLE queue properties\n");
		}
		else
		{
			printf("Device doesnt have basic properties\n");
			return false;

		}
		return true;
	}

}
//GetDeviceInfo with CL_DEVICE_NAME, CL_DEVICE_VENDOR, CL_DEVICE_PROFILE, CL_DEVICE_VERSION test
bool clGetDeviceInfo_DeviceStrings()
{

	char name[STR_LEN];

	size_t str_size = 0;

	cl_int iRes = clDevGetDeviceInfo(gDeviceIdInType, CL_DEVICE_NAME, STR_LEN, name, &str_size);
	if (CL_DEV_FAILED(iRes))
	{
		printf("clDevGetDeviceInfo failed: %s\n",clDevErr2Txt((cl_dev_err_code)iRes));
		return false;
	}
	if ( '\0' != name[str_size-1] )
	{
		printf("clDevGetDeviceInfo invalid device name\n");
		return false;
	}
	printf("Device name is %s\n",name);

	iRes = clDevGetDeviceInfo(gDeviceIdInType, CL_DEVICE_VENDOR, STR_LEN, name, &str_size);
	if (CL_DEV_FAILED(iRes))
	{
		printf("clDevGetDeviceInfo failed: %s\n",clDevErr2Txt((cl_dev_err_code)iRes));
		return false;
	}
	if ( '\0' != name[str_size-1] )
	{
		printf("clDevGetDeviceInfo invalid vendor name\n");
		return false;
	}

	printf("Device vendor %s\n",name);

	iRes = clDevGetDeviceInfo(gDeviceIdInType, CL_DEVICE_PROFILE, STR_LEN, name, &str_size);
	if (CL_DEV_FAILED(iRes))
	{
		printf("clDevGetDeviceInfo failed: %s\n",clDevErr2Txt((cl_dev_err_code)iRes));
		return false;
	}
	if ( '\0' != name[str_size-1] )
	{
		printf("clDevGetDeviceInfo invalid device profile\n");
		return false;
	}

	printf("Device profile is %s\n",name);

	iRes = clDevGetDeviceInfo(gDeviceIdInType, CL_DEVICE_VERSION, STR_LEN, name, &str_size);
	if (CL_DEV_FAILED(iRes))
	{
		printf("clDevGetDeviceInfo failed: %s\n",clDevErr2Txt((cl_dev_err_code)iRes));
		return false;
	}
	if ( '\0' != name[str_size-1] )
	{
		printf("clDevGetDeviceInfo invalid device version\n");
		return false;
	}

	printf("Device version is %s\n",name);

	return true;
}

// test CommandList functions
bool CommandList_Test()
{
	//Create command list
	cl_dev_cmd_list_props props = CL_DEV_LIST_ENABLE_OOO;
	cl_dev_cmd_list list;

	cl_int iRes = dev_entry->clDevCreateCommandList(props, 0, &list);
	if (CL_DEV_FAILED(iRes))
	{
		printf("pclDevCreateCommandList failed: %s\n",clDevErr2Txt((cl_dev_err_code)iRes));
		return false;
	}

	iRes = dev_entry->clDevReleaseCommandList(list);
	if (CL_DEV_FAILED(iRes))
	{
		printf("clDevReleaseCommandList failed: %s\n",clDevErr2Txt((cl_dev_err_code)iRes));
		return false;
	}

	return true;
}

//test native kernel execution
bool ExecuteNativeKernel_Test(bool profiling)
{
	cl_int iRes;

	//Create command list
	cl_dev_cmd_param_native nativeParam;

	// Set Native Kernel Parameters
	gNativeKernelParam.count = 10;
	gNativeKernelParam.buff = (int*)malloc(gNativeKernelParam.count*sizeof(int));
	memset(gNativeKernelParam.buff, 0, gNativeKernelParam.count*sizeof(int));

	// Set Execute parameters
	memset(&nativeParam, 0, sizeof(cl_dev_cmd_param_native));
	nativeParam.func_ptr = _TestKernel;
	nativeParam.args = sizeof(TestKernel_param_t);
	nativeParam.argv = &gNativeKernelParam;

	//Execute command
	cl_dev_cmd_desc  cmds;
	cl_uint count = 1;

	cmds.type = CL_DEV_CMD_EXEC_NATIVE;
	cmds.id = (cl_dev_cmd_id)CL_DEV_CMD_EXEC_NATIVE;
	cmds.params = &nativeParam;
	cmds.param_size = sizeof(cl_dev_cmd_param_native);
	cmds.profiling = profiling;

	gExecDone = false;

	cl_dev_cmd_desc* cmdsBuff = &cmds;
	iRes = dev_entry->clDevCommandListExecute(0, &cmdsBuff, count);
	if (CL_DEV_FAILED(iRes))
	{
		printf("clDevCommandListExecute failed: %s\n",clDevErr2Txt((cl_dev_err_code)iRes));
		return false;
	}

	while(!gExecDone )
	{
		SLEEP(10);
	}

	bool	match = true;

	// Test for kernel completion
	for(int i=0; i<gNativeKernelParam.count; ++i)
	{
		match &= (gNativeKernelParam.buff[i] == i);
	}

	free(gNativeKernelParam.buff);
	if ( match )
	{
		printf("Native Kernel Execution succeeded\n");
		return true;
	}
	else
	{
		printf("Native Kernel Execution failed\n");
		return false;
	}

	return true;
}

// The following tests replace the old "main" function of framework_test_type.
//
TEST(CpuDeviceTestType, Test_InitLogger)
{
	EXPECT_TRUE(InitLoggerTest());
}

TEST(CpuDeviceTestType, Test_clGetDeviceInfo)
{
	EXPECT_TRUE(clGetDeviceInfo_Test());
}

TEST(CpuDeviceTestType, Test_clGetDeviceInfoType)
{
	EXPECT_TRUE(clGetDeviceInfo_TypeTest());
}

TEST(CpuDeviceTestType, Test_clGetDeviceInfo_VendorId)
{
	EXPECT_TRUE(clGetDeviceInfo_VendorIdTest());
}

TEST(CpuDeviceTestType, Test_clGetDeviceInfo_MaxComputeUnit)
{
	EXPECT_TRUE(clGetDeviceInfo_MaxComputeUnitTest());
}

TEST(CpuDeviceTestType, Test_clGetDeviceInfo_DeviceAvilable)
{
	EXPECT_TRUE(clGetDeviceInfo_DeviceAvilable());
}

TEST(CpuDeviceTestType, Test_clGetDeviceInfo_DeviceExecutionProperties)
{
	EXPECT_TRUE(clGetDeviceInfo_DeviceExecutionProperties());
}

TEST(CpuDeviceTestType, Test_clGetDeviceInfo_QueueProperties)
{
	EXPECT_TRUE(clGetDeviceInfo_QueueProperties());
}

TEST(CpuDeviceTestType, Test_clGetDeviceInfo_DeviceStrings)
{
	EXPECT_TRUE(clGetDeviceInfo_DeviceStrings());
}

TEST(CpuDeviceTestType, Test_imageTest)
{
	EXPECT_TRUE(imageTest(true));
}

TEST(CpuDeviceTestType, Test_CommandList)
{
	EXPECT_TRUE(CommandList_Test());
}

TEST(CpuDeviceTestType, Test_ExecuteNativeKernel)
{
	EXPECT_TRUE(ExecuteNativeKernel_Test(true));
}

TEST(CpuDeviceTestType, Test_BuildFromBinary)
{
	std::string filename = get_exe_dir() + "test.bc";
	EXPECT_TRUE(BuildFromBinary_test(filename.c_str(), 2, "dot_product", 3));
}

TEST(CpuDeviceTestType, Test_memoryTest)
{
	EXPECT_TRUE(memoryTest(true));
}

TEST(CpuDeviceTestType, Test_KernelExecute_Math)
{
	std::string filename = get_exe_dir() + "test.bc";
	EXPECT_TRUE(KernelExecute_Math_Test(filename.c_str()));
}

#ifndef _WIN32
TEST(CpuDeviceTestType, DISABLED_Test_AffinityRootDevice)   // ticket CSSD100020139
{
	EXPECT_TRUE(AffinityRootDeviceTest(pMask));
}

TEST(CpuDeviceTestType, DISABLED_Test_AffinitySubDevice)    // ticket CSSD100020139
{
	EXPECT_TRUE(AffinitySubDeviceTest(pMask));
}
#endif
// Manual test, don't enable
//TEST(CpuDeviceTestType, Test_KernelExecute_Lcl_Mem)
//{
//	EXPECT_TRUE(KernelExecute_Lcl_Mem_Test("test.bc"));
//}

TEST(CpuDeviceTestType, Test_mapTest)
{
	EXPECT_TRUE(mapTest());
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

CPUTestCallbacks g_dev_callbacks;


int CPUDeviceTest_Main()
{
	ITaskExecutor* pTaskExecutor = GetTaskExecutor();
	EXPECT_TRUE(pTaskExecutor!=NULL);

	// Initialize Task Executor
	int iThreads = pTaskExecutor->Init(NULL, TE_AUTO_THREADS);
	EXPECT_TRUE(iThreads>0);

	//Create and Init the device

	static CPUTestLogger		log_desc;

	size_t numDevicesInDeviceType = 0;
	cl_int iRes = clDevGetAvailableDeviceList(0, NULL, &numDevicesInDeviceType);
	EXPECT_TRUE(CL_DEV_SUCCEEDED(iRes));
	if (0 == numDevicesInDeviceType)
	{
		return -1;
	}

	size_t deviceIdsListSizeRet = 0;
	unsigned int deviceIdsList[1] = {0};
	iRes = clDevGetAvailableDeviceList(1, deviceIdsList, &deviceIdsListSizeRet);
	EXPECT_TRUE(CL_DEV_SUCCEEDED(iRes));
	if (0 == deviceIdsListSizeRet)
	{
		printf("\nTest failed (no available devices returned)\n");
		return -1;
	}

	gDeviceIdInType = deviceIdsList[0];

	iRes = clDevCreateDeviceInstance(gDeviceIdInType, &g_dev_callbacks, &log_desc, &dev_entry, NULL);
	EXPECT_TRUE(CL_DEV_SUCCEEDED(iRes));

	int rc = RUN_ALL_TESTS();
	dev_entry->clDevCloseDevice();
    return rc;
}
#ifndef _WIN32
void printAffinityMask(affinityMask_t* affinityMask)
{
    int max_cpus = CPU_SETSIZE;

    unsigned char aff;
    bool needPrint = false;
    for (int set = max_cpus; set > 7; set -= 8)
    {
        aff = 0;
        for (int cpu = set - 8; cpu < set; ++cpu)
        {
            if (CPU_ISSET(cpu, affinityMask))
            {
                aff |= (1 << (cpu - set + 8));
            }
        }
        needPrint |= (aff != 0);
        if (needPrint)
        {
            printf("%hhx", aff);
        }
    }
    printf("\n");
}
void initAndPrintRandomMask(affinityMask_t* affinityMask)
{
    CPU_ZERO(affinityMask);
    srand(time(NULL));
    unsigned long numProcessors = Intel::OpenCL::Utils::GetNumberOfProcessors();
    for (unsigned long i = 0; i < numProcessors; ++i)
    {
        if (rand() & 0x1)
        {
            CPU_SET(i, affinityMask);
        }
    }
    int count = CPU_COUNT(affinityMask);
    if (0 == count)
    {
        CPU_SET(rand() % numProcessors, affinityMask);
        ++count;
    }

    printf("Using mask 0x");
    printAffinityMask(affinityMask);
}

void translateAndPrintMask(affinityMask_t* affinityMask, unsigned long val)
{
    CPU_ZERO(affinityMask);
    int i = 0;
    while (val > 0)
    {
        if (val & 0x1)
        {
            CPU_SET(i, affinityMask);
        }
        val >>= 1;
        ++i;
    }
    printf("Using mask 0x");
    printAffinityMask(affinityMask);
}
#endif

int main(int argc, char* argv[])
{
	::testing::InitGoogleTest(&argc, argv);
#ifndef _WIN32
    affinityMask_t affinityMask;
    //Check for parameters not to gtest
    if (argc > 1)
    {
        if (argc != 2)
        {
            printf("Usage: %s <-mask=val> where val is a number in hex format or RANDOM\n", argv[0]);
            return -1;
        }
        const char* param = argv[1];
        if (strlen(param) < strlen("-mask="))
        {
            printf("Usage: %s <-mask=val> where val is a number in hex format or RANDOM\n", argv[0]);
            return -1;
        }

        if (!strcmp(param, "-mask=RANDOM"))
        {
            initAndPrintRandomMask(&affinityMask);
        }
        else
        {
            unsigned long aff = strtoul(param+6, NULL, 0);
            if (0 == aff)
            {
                printf("illegal value specified for mask (%s)\n", param+6);
                return -1;
            }
            translateAndPrintMask(&affinityMask, aff);
        }

        sched_setaffinity(0, sizeof(affinityMask), &affinityMask);
        pMask = &affinityMask;
    }
#endif
    int rc = CPUDeviceTest_Main();

	/* [QA]: move result checking to CPUDeviceTest_Main to avoid additional output in case if running with --gtest_list_tests option
	if (rc == 0) {
		printf("\n==============\nTEST SUCCEDDED\n==============\n");
	}
	else {
		printf("\n==============\nTEST FAILED\n==============\n");
	}
	*/

	return rc;
}
