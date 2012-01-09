#include "CL/cl.h"
#include "cl_types.h"
#include "Logger.h"
#include "cl_objects_map.h"
#include <stdio.h>
#include "FrameworkTest.h"
using namespace Intel::OpenCL::Framework;

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
	bResult &= Check(L"clGetPlatformIDs", CL_SUCCESS, iRes);

	if (!bResult)
	{
		return bResult;
	}

	///////////////CPU device///////////////

	// clGetDeviceIDs
	iRes = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 100, devices, NULL);
	bResult &= Check(L"clGetDeviceIDs", CL_SUCCESS, iRes);

	// invalid device
	//iRes = clGetDeviceInfo((cl_device_id)-99, CL_DEVICE_TYPE, sizeof(cl_device_type), (void*)(&clDevType), NULL);
	//bResult &= Check(L"CL_DEVICE_TYPE, invalid device", CL_INVALID_DEVICE, iRes);

	// invalid args
	iRes = clGetDeviceInfo(devices[0], CL_DEVICE_TYPE, 0, NULL, NULL);
	bResult &= Check(L"CL_DEVICE_TYPE, invalid args", CL_INVALID_VALUE, iRes);

	// CL_DEVICE_TYPE
	// return size only
	iRes = clGetDeviceInfo(devices[0], CL_DEVICE_TYPE, 0, NULL, &size_ret);
	bResult &= Check(L"CL_DEVICE_TYPE, return size only args", CL_SUCCESS, iRes);
	if (CL_SUCCEEDED(iRes))
	{
		bResult &= CheckSize(L"check value", sizeof(cl_device_type), size_ret);
	}

	// CL_DEVICE_TYPE
	// all OK
	iRes = clGetDeviceInfo(devices[0], CL_DEVICE_TYPE, sizeof(cl_device_type), (void*)(&clDevType), NULL);
	bResult &= Check(L"CL_DEVICE_TYPE, all OK", CL_SUCCESS, iRes);

	// CL_DEVICE_VENDOR_ID
	// all OK
	iRes = clGetDeviceInfo(devices[0], CL_DEVICE_VENDOR_ID, sizeof(cl_uint), &uiVendorId, NULL);
	bResult &= Check(L"CL_DEVICE_VENDOR_ID, all OK", CL_SUCCESS, iRes);
	
	// max compute units
	// all OK
	iRes = clGetDeviceInfo(devices[0], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &uiMaxComputeUnits, NULL);
	bResult &= Check(L"CL_DEVICE_MAX_COMPUTE_UNITS, all OK", CL_SUCCESS, iRes);

	// device version string
	size_t szDeviceVersionStringSize = 0;
	iRes = clGetDeviceInfo(devices[0], CL_DEVICE_VERSION, NULL, NULL, &szDeviceVersionStringSize);
	bResult &= Check(L"CL_DEVICE_VERSION - query size", CL_SUCCESS, iRes);
	if (bResult && 0 < szDeviceVersionStringSize)
	{
		char * pDeviceVersionString = new char[szDeviceVersionStringSize + 1];
		iRes = clGetDeviceInfo(devices[0], CL_DEVICE_VERSION, szDeviceVersionStringSize + 1, pDeviceVersionString, NULL);
		bResult &= Check(L"CL_DEVICE_VERSION - get string", CL_SUCCESS, iRes);
		if (bResult)
		{
			printf("CL_DEVICE_VERSION: %s\n", pDeviceVersionString);
		}
		delete[] pDeviceVersionString;
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
	bResult &= CheckSize(L"CL_DEVICE_PROFILING_TIMER_RESOLUTION", szNativeCodeResolution, szResolution);

	// max work item dimentions
	// all OK
	//cl_uint uiMaxWorkItemDim;
	//iRes = clGetDeviceInfo(devices[0], CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(cl_uint), &uiMaxWorkItemDim, NULL);
	//bResult &= Check(L"CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, all OK", CL_SUCCESS, iRes);


#ifdef _CUDA_DEVICE

	///////////////CPU device///////////////

	// clGetDeviceIDs
	iRes = clGetDeviceIDs(CL_DEVICE_TYPE_GPU, 100, devices, NULL);
	bResult &= Check(L"clGetDeviceIDs", CL_SUCCESS, iRes);

	// invalid device
	iRes = clGetDeviceInfo((cl_device_id)-99, CL_DEVICE_TYPE, sizeof(cl_device_type), (void*)(&clDevType), NULL);
	bResult &= Check(L"CL_DEVICE_TYPE, invalid device", CL_INVALID_DEVICE, iRes);

	// invalid args
	iRes = clGetDeviceInfo(devices[0], CL_DEVICE_TYPE, 0, NULL, NULL);
	bResult &= Check(L"CL_DEVICE_TYPE, invalid args", CL_INVALID_VALUE, iRes);

	// CL_DEVICE_TYPE
	// return size only
	iRes = clGetDeviceInfo(devices[0], CL_DEVICE_TYPE, 0, NULL, &size_ret);
	bResult &= Check(L"CL_DEVICE_TYPE, return size only args", CL_SUCCESS, iRes);
	if (CL_SUCCEEDED(iRes))
	{
		bResult &= CheckSize(L"check value", sizeof(cl_device_type), size_ret);
	}

	// CL_DEVICE_TYPE
	// all OK
	iRes = clGetDeviceInfo(devices[0], CL_DEVICE_TYPE, sizeof(cl_device_type), (void*)(&clDevType), NULL);
	bResult &= Check(L"CL_DEVICE_TYPE, all OK", CL_SUCCESS, iRes);

	// CL_DEVICE_VENDOR_ID
	// all OK
	iRes = clGetDeviceInfo(devices[0], CL_DEVICE_VENDOR_ID, sizeof(cl_uint), &uiVendorId, NULL);
	bResult &= Check(L"CL_DEVICE_VENDOR_ID, all OK", CL_SUCCESS, iRes);
	
	// max compute units
	// all OK
	iRes = clGetDeviceInfo(devices[0], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &uiMaxComputeUnits, NULL);
	bResult &= Check(L"CL_DEVICE_MAX_COMPUTE_UNITS, all OK", CL_SUCCESS, iRes);


	// max work item dimentions
	// all OK
	iRes = clGetDeviceInfo(devices[0], CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(cl_uint), &uiMaxWorkItemDim, NULL);
	bResult &= Check(L"CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, all OK", CL_SUCCESS, iRes);


	// max work item sizes
	// return size only
	iRes = clGetDeviceInfo(devices[0], CL_DEVICE_MAX_WORK_ITEM_SIZES, 0, NULL, &size_ret);
	bResult &= Check(L"CL_DEVICE_MAX_WORK_ITEM_SIZES, return size only", CL_SUCCESS, iRes);
	if (CL_SUCCEEDED(iRes))
	{
		bResult &= CheckSize(L"check value", uiMaxWorkItemDim * sizeof(size_t), size_ret);
	}


	// max work item sizes
	// all OK
	MaxWorkItemSizes = new size_t[size_ret / sizeof(size_t)];
	iRes = clGetDeviceInfo(devices[0], CL_DEVICE_MAX_WORK_ITEM_SIZES, uiMaxWorkItemDim * sizeof(size_t), MaxWorkItemSizes, NULL);
	bResult &= Check(L"CL_DEVICE_MAX_WORK_ITEM_SIZES, all OK", CL_SUCCESS, iRes);
	delete[] MaxWorkItemSizes;


	// max work group size
	// all OK
	iRes = clGetDeviceInfo(devices[0], CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &MaxWorkGroupSize, NULL);
	bResult &= Check(L"CL_DEVICE_MAX_WORK_GROUP_SIZE, all OK", CL_SUCCESS, iRes);


	// max clock frequency
	// all OK
	iRes = clGetDeviceInfo(devices[0], CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(size_t), &uiMaxClockFrequency, NULL);
	bResult &= Check(L"CL_DEVICE_MAX_CLOCK_FREQUENCY, all OK", CL_SUCCESS, iRes);


	// max mem alloc size
	// all OK
	iRes = clGetDeviceInfo(devices[0], CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(size_t), &ulMaxMemAllocSize, NULL);
	bResult &= Check(L"CL_DEVICE_MAX_MEM_ALLOC_SIZE, all OK", CL_SUCCESS, iRes);


	// image support
	// all OK
	iRes = clGetDeviceInfo(devices[0], CL_DEVICE_IMAGE_SUPPORT, sizeof(cl_bool), &bImageSupport, NULL);
	bResult &= Check(L"CL_DEVICE_IMAGE_SUPPORT, all OK", CL_SUCCESS, iRes);

#endif

	return bResult;
}
