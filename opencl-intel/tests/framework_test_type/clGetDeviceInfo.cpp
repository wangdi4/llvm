#include "CL/cl.h"
#include "cl_cpu_detect.h"
#include "cl_types.h"
#include <gtest/gtest.h>
#include <stdio.h>
#include "FrameworkTest.h"

extern cl_device_type gDeviceType;
using namespace Intel::OpenCL::Utils;

bool clGetDeviceInfoTest()
{
	printf("---------------------------------------\n");
	printf("clGetDeviceInfo\n");
	printf("---------------------------------------\n");
	bool bResult = true;
	cl_int iRes = CL_SUCCESS;
	cl_device_id devices[100];
	cl_device_type clDevType;
	cl_uint uiVendorId;
	cl_uint uiMaxComputeUnits;
	size_t size_ret = 0;

	cl_platform_id platform = 0;

	iRes = clGetPlatformIDs(1, &platform, NULL);
	bResult &= Check("clGetPlatformIDs", CL_SUCCESS, iRes);

	if (!bResult)
	{
		return bResult;
	}

	///////////////CPU device///////////////

	// clGetDeviceIDs
	iRes = clGetDeviceIDs(platform, gDeviceType, 100, devices, NULL);
	bResult &= Check("clGetDeviceIDs", CL_SUCCESS, iRes);

	// invalid device
	//iRes = clGetDeviceInfo((cl_device_id)-99, CL_DEVICE_TYPE, sizeof(cl_device_type), (void*)(&clDevType), NULL);
	//bResult &= Check("CL_DEVICE_TYPE, invalid device", CL_INVALID_DEVICE, iRes);

	// useless call (both returns are NULL) but allowed.
	iRes = clGetDeviceInfo(devices[0], CL_DEVICE_TYPE, 0, NULL, NULL);
	bResult &= Check("CL_DEVICE_TYPE, invalid args", CL_SUCCESS, iRes);

	// CL_DEVICE_TYPE
	// return size only
	iRes = clGetDeviceInfo(devices[0], CL_DEVICE_TYPE, 0, NULL, &size_ret);
	bResult &= Check("CL_DEVICE_TYPE, return size only args", CL_SUCCESS, iRes);
	if (CL_SUCCEEDED(iRes))
	{
		bResult &= CheckSize("check value", sizeof(cl_device_type), size_ret);
	}

  // CL_DRIVER_VERSION
  // return driver version
  // it's set as hardcoded number, see more in doc/library_versioning.rst
  // shall be fixed when it's changed
  char buffer[1024];
  iRes = clGetDeviceInfo(devices[0], CL_DRIVER_VERSION, sizeof(buffer), buffer, nullptr);
  bResult &= Check("CL_DRIVER_VERSION, all OK", CL_SUCCESS, iRes);

	// CL_DEVICE_TYPE
	// all OK
	iRes = clGetDeviceInfo(devices[0], CL_DEVICE_TYPE, sizeof(cl_device_type), (void*)(&clDevType), NULL);
	bResult &= Check("CL_DEVICE_TYPE, all OK", CL_SUCCESS, iRes);

	// CL_DEVICE_VENDOR_ID
	// all OK
	iRes = clGetDeviceInfo(devices[0], CL_DEVICE_VENDOR_ID, sizeof(cl_uint), &uiVendorId, NULL);
	bResult &= Check("CL_DEVICE_VENDOR_ID, all OK", CL_SUCCESS, iRes);
	
	// max compute units
	// all OK
	iRes = clGetDeviceInfo(devices[0], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &uiMaxComputeUnits, NULL);
	bResult &= Check("CL_DEVICE_MAX_COMPUTE_UNITS, all OK", CL_SUCCESS, iRes);

	// device version string
	size_t szDeviceVersionStringSize = 0;
    std::string deviceVersionString;
	iRes = clGetDeviceInfo(devices[0], CL_DEVICE_VERSION, 0, NULL, &szDeviceVersionStringSize);
	bResult &= Check("CL_DEVICE_VERSION - query size", CL_SUCCESS, iRes);
	if (bResult && 0 < szDeviceVersionStringSize)
	{
        deviceVersionString.resize(szDeviceVersionStringSize);
		iRes = clGetDeviceInfo(devices[0], CL_DEVICE_VERSION, szDeviceVersionStringSize, &deviceVersionString[0], NULL);
		bResult &= Check("CL_DEVICE_VERSION - get string", CL_SUCCESS, iRes);
		if (bResult)
		{
			printf("CL_DEVICE_VERSION: %s\n", deviceVersionString.c_str());
		}
	}


	cl_uint uiNativeVecWidth = 0;
	iRes = clGetDeviceInfo(devices[0],CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT, sizeof(cl_uint), &uiNativeVecWidth, NULL);
	bResult &= Check("CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT - query size", CL_SUCCESS, iRes);
	const auto CPUID = CPUDetect::GetInstance();
	if (CPUID->IsFeatureSupported(CFS_AVX512F))
	{
		bResult &= CheckSize("check value", 16, uiNativeVecWidth); // AVX512
	}
	else if (CPUID->IsFeatureSupported(CFS_AVX20))
	{
		bResult &= CheckSize("check value", 8, uiNativeVecWidth); // AVX2
	}
	else if (CPUID->IsFeatureSupported(CFS_AVX10))
	{
		bResult &= CheckSize("check value", 8, uiNativeVecWidth); // AVX
	}
	else
	{
		bResult &= CheckSize("check value", 4, uiNativeVecWidth); // SSE42
	}

	// CL_DEVICE_PROFILING_TIMER_RESOLUTION
	// Compare with native code results.
	size_t szResolution;
	size_t szNativeCodeResolution;
	iRes = clGetDeviceInfo(devices[0], CL_DEVICE_PROFILING_TIMER_RESOLUTION, sizeof(size_t), &szResolution, NULL);
#if defined( __linux__)
	struct timespec tp;
	clock_getres(CLOCK_MONOTONIC, &tp);
	szNativeCodeResolution = (size_t)tp.tv_nsec;
#elif defined(_WIN32)
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	szNativeCodeResolution = (size_t)(1e9/freq.QuadPart);
#endif
	if (gDeviceType != CL_DEVICE_TYPE_ACCELERATOR)
	{
		bResult &= CheckSize("CL_DEVICE_PROFILING_TIMER_RESOLUTION", szNativeCodeResolution, szResolution);
	}

    if (!deviceVersionString.compare(0, 10, "OpenCL 3.0"))
    {
        cl_bool nonUniformWGSupport = CL_FALSE;
        iRes = clGetDeviceInfo(devices[0], CL_DEVICE_NON_UNIFORM_WORK_GROUP_SUPPORT, sizeof(cl_bool), &nonUniformWGSupport, &size_ret);
        bResult &= Check("clGetDeviceInfo CL_DEVICE_NON_UNIFORM_WORK_GROUP_SUPPORT", CL_SUCCESS, iRes);
        bResult &= CheckSize("clGetDeviceInfo CL_DEVICE_NON_UNIFORM_WORK_GROUP_SUPPORT - query size", sizeof(cl_bool), size_ret);
        bResult &= Check("clGetDeviceInfo CL_DEVICE_NON_UNIFORM_WORK_GROUP_SUPPORT", CL_TRUE, nonUniformWGSupport);

        cl_bool wgCollectiveFuncSupport = CL_FALSE;
        iRes = clGetDeviceInfo(devices[0], CL_DEVICE_WORK_GROUP_COLLECTIVE_FUNCTIONS_SUPPORT, sizeof(cl_bool), &wgCollectiveFuncSupport, &size_ret);
        bResult &= Check("clGetDeviceInfo CL_DEVICE_WORK_GROUP_COLLECTIVE_FUNCTIONS_SUPPORT", CL_SUCCESS, iRes);
        bResult &= CheckSize("clGetDeviceInfo CL_DEVICE_WORK_GROUP_COLLECTIVE_FUNCTIONS_SUPPORT - query size", sizeof(cl_bool), size_ret);
        bResult &= Check("clGetDeviceInfo CL_DEVICE_WORK_GROUP_COLLECTIVE_FUNCTIONS_SUPPORT", CL_TRUE, wgCollectiveFuncSupport);

        cl_bool genericAddressSpaceSupport = CL_FALSE;
        iRes = clGetDeviceInfo(devices[0], CL_DEVICE_GENERIC_ADDRESS_SPACE_SUPPORT, sizeof(cl_bool), &genericAddressSpaceSupport, &size_ret);
        bResult &= Check("clGetDeviceInfo CL_DEVICE_GENERIC_ADDRESS_SPACE_SUPPORT", CL_SUCCESS, iRes);
        bResult &= CheckSize("clGetDeviceInfo CL_DEVICE_GENERIC_ADDRESS_SPACE_SUPPORT - query size", sizeof(cl_bool), size_ret);
        bResult &= Check("clGetDeviceInfo CL_DEVICE_GENERIC_ADDRESS_SPACE_SUPPORT", CL_TRUE, genericAddressSpaceSupport);

        cl_device_device_enqueue_capabilities enqueueCapabilities;
        iRes = clGetDeviceInfo(devices[0], CL_DEVICE_DEVICE_ENQUEUE_CAPABILITIES, sizeof(cl_device_device_enqueue_capabilities), &enqueueCapabilities, &size_ret);
        bResult &= Check("clGetDeviceInfo CL_DEVICE_DEVICE_ENQUEUE_CAPABILITIES", CL_SUCCESS, iRes);
        bResult &= CheckSize("clGetDeviceInfo CL_DEVICE_DEVICE_ENQUEUE_CAPABILITIES - query size", sizeof(cl_device_device_enqueue_capabilities), size_ret);
        bResult &= Check("clGetDeviceInfo CL_DEVICE_DEVICE_ENQUEUE_CAPABILITIES", CL_DEVICE_QUEUE_SUPPORTED|CL_DEVICE_QUEUE_REPLACEABLE_DEFAULT, enqueueCapabilities);

        cl_bool pipeSupport = CL_FALSE;
        iRes = clGetDeviceInfo(devices[0], CL_DEVICE_PIPE_SUPPORT, sizeof(cl_bool), &pipeSupport, &size_ret);
        bResult &= Check("clGetDeviceInfo CL_DEVICE_PIPE_SUPPORT", CL_SUCCESS, iRes);
        bResult &= CheckSize("clGetDeviceInfo CL_DEVICE_PIPE_SUPPORT - query size", sizeof(cl_bool), size_ret);
        bResult &= Check("clGetDeviceInfo CL_DEVICE_PIPE_SUPPORT", CL_TRUE, pipeSupport);

        size_t preferredWGSize;
        iRes = clGetDeviceInfo(devices[0], CL_DEVICE_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, sizeof(size_t), &preferredWGSize, &size_ret);
        bResult &= Check("clGetDeviceInfo CL_DEVICE_PREFERRED_WORK_GROUP_SIZE_MULTIPLE", CL_SUCCESS, iRes);
        bResult &= CheckSize("clGetDeviceInfo CL_DEVICE_PREFERRED_WORK_GROUP_SIZE_MULTIPLE - query size", sizeof(size_t), size_ret);
        bResult &= CheckSize("clGetDeviceInfo CL_DEVICE_PREFERRED_WORK_GROUP_SIZE_MULTIPLE", 128, preferredWGSize);
    }
	// max work item dimentions
	// all OK
	//cl_uint uiMaxWorkItemDim;
	//iRes = clGetDeviceInfo(devices[0], CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(cl_uint), &uiMaxWorkItemDim, NULL);
	//bResult &= Check("CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, all OK", CL_SUCCESS, iRes);


#ifdef _CUDA_DEVICE

	///////////////CPU device///////////////

	// clGetDeviceIDs
	iRes = clGetDeviceIDs(CL_DEVICE_TYPE_GPU, 100, devices, NULL);
	bResult &= Check("clGetDeviceIDs", CL_SUCCESS, iRes);

	// invalid device
	iRes = clGetDeviceInfo((cl_device_id)-99, CL_DEVICE_TYPE, sizeof(cl_device_type), (void*)(&clDevType), NULL);
	bResult &= Check("CL_DEVICE_TYPE, invalid device", CL_INVALID_DEVICE, iRes);

	// useless call (both returns are NULL) but allowed.
	iRes = clGetDeviceInfo(devices[0], CL_DEVICE_TYPE, 0, NULL, NULL);
	bResult &= Check("CL_DEVICE_TYPE, invalid args", CL_SUCCESS, iRes);

	// CL_DEVICE_TYPE
	// return size only
	iRes = clGetDeviceInfo(devices[0], CL_DEVICE_TYPE, 0, NULL, &size_ret);
	bResult &= Check("CL_DEVICE_TYPE, return size only args", CL_SUCCESS, iRes);
	if (CL_SUCCEEDED(iRes))
	{
		bResult &= CheckSize("check value", sizeof(cl_device_type), size_ret);
	}

	// CL_DEVICE_TYPE
	// all OK
	iRes = clGetDeviceInfo(devices[0], CL_DEVICE_TYPE, sizeof(cl_device_type), (void*)(&clDevType), NULL);
	bResult &= Check("CL_DEVICE_TYPE, all OK", CL_SUCCESS, iRes);

	// CL_DEVICE_VENDOR_ID
	// all OK
	iRes = clGetDeviceInfo(devices[0], CL_DEVICE_VENDOR_ID, sizeof(cl_uint), &uiVendorId, NULL);
	bResult &= Check("CL_DEVICE_VENDOR_ID, all OK", CL_SUCCESS, iRes);
	
	// max compute units
	// all OK
	iRes = clGetDeviceInfo(devices[0], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &uiMaxComputeUnits, NULL);
	bResult &= Check("CL_DEVICE_MAX_COMPUTE_UNITS, all OK", CL_SUCCESS, iRes);


	// max work item dimentions
	// all OK
	iRes = clGetDeviceInfo(devices[0], CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(cl_uint), &uiMaxWorkItemDim, NULL);
	bResult &= Check("CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, all OK", CL_SUCCESS, iRes);


	// max work item sizes
	// return size only
	iRes = clGetDeviceInfo(devices[0], CL_DEVICE_MAX_WORK_ITEM_SIZES, 0, NULL, &size_ret);
	bResult &= Check("CL_DEVICE_MAX_WORK_ITEM_SIZES, return size only", CL_SUCCESS, iRes);
	if (CL_SUCCEEDED(iRes))
	{
		bResult &= CheckSize("check value", uiMaxWorkItemDim * sizeof(size_t), size_ret);
	}


	// max work item sizes
	// all OK
	MaxWorkItemSizes = new size_t[size_ret / sizeof(size_t)];
	iRes = clGetDeviceInfo(devices[0], CL_DEVICE_MAX_WORK_ITEM_SIZES, uiMaxWorkItemDim * sizeof(size_t), MaxWorkItemSizes, NULL);
	bResult &= Check("CL_DEVICE_MAX_WORK_ITEM_SIZES, all OK", CL_SUCCESS, iRes);
	delete[] MaxWorkItemSizes;


	// max work group size
	// all OK
	iRes = clGetDeviceInfo(devices[0], CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &MaxWorkGroupSize, NULL);
	bResult &= Check("CL_DEVICE_MAX_WORK_GROUP_SIZE, all OK", CL_SUCCESS, iRes);


	// max clock frequency
	// all OK
	iRes = clGetDeviceInfo(devices[0], CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(size_t), &uiMaxClockFrequency, NULL);
	bResult &= Check("CL_DEVICE_MAX_CLOCK_FREQUENCY, all OK", CL_SUCCESS, iRes);


	// max mem alloc size
	// all OK
	iRes = clGetDeviceInfo(devices[0], CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(size_t), &ulMaxMemAllocSize, NULL);
	bResult &= Check("CL_DEVICE_MAX_MEM_ALLOC_SIZE, all OK", CL_SUCCESS, iRes);


	// image support
	// all OK
	iRes = clGetDeviceInfo(devices[0], CL_DEVICE_IMAGE_SUPPORT, sizeof(cl_bool), &bImageSupport, NULL);
	bResult &= Check("CL_DEVICE_IMAGE_SUPPORT, all OK", CL_SUCCESS, iRes);

#endif

	return bResult;
}
