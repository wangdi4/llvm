
#include "CL/cl.h"
#include "cl_types.h"
#include "Logger.h"
#include "cl_objects_map.h"
#include <stdio.h>
#include "FrameworkTest.h"
using namespace Intel::OpenCL::Framework;

bool clGetDeviceIDsTest()
{
	printf("---------------------------------------\n");
	printf("clGetPlatformInfo\n");
	printf("---------------------------------------\n");
	bool bResult = true;
	cl_int iRes = CL_SUCCESS;
	cl_uint size_ret = 0;

	cl_platform_id platform = 0;

	iRes = clGetPlatformIDs(1, &platform, NULL);
	bResult &= Check(L"clGetPlatformIDs", CL_SUCCESS, iRes);

	if (!bResult)
	{
		return bResult;
	}

	cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };

	cl_device_id devices[100];
	// CL_DEVICE_TYPE_CPU
	// invalid args
	iRes = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 0, NULL, NULL);
	bResult &= Check(L"CL_DEVICE_TYPE_CPU, invalid args", CL_INVALID_VALUE, iRes);

	// CL_DEVICE_TYPE_CPU
	// return size only
	iRes = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 0, NULL, &size_ret);
	bResult &= Check(L"CL_DEVICE_TYPE_CPU, return size only", CL_SUCCESS, iRes);

	// CL_DEVICE_TYPE_CPU
	devices[0] = 0;
	iRes = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 100, devices, &size_ret);
	bResult &= Check(L"CL_DEVICE_TYPE_CPU", CL_SUCCESS, iRes);
	if (CL_SUCCEEDED(iRes))
	{
		bResult &= CheckSize(L"check size", 1, size_ret);
	}

	// CL_DEVICE_TYPE_GPU
	devices[0] = 0;
	iRes = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 100, devices, NULL);
	bResult &= Check(L"CL_DEVICE_TYPE_GPU", CL_DEVICE_NOT_FOUND, iRes);
	
	// CL_DEVICE_TYPE_ACCELERATOR
	devices[0] = 0;
	iRes = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ACCELERATOR, 100, devices, NULL);
	bResult &= Check(L"CL_DEVICE_TYPE_ACCELERATOR", CL_DEVICE_NOT_FOUND, iRes);
	
	// CL_DEVICE_TYPE_DEFAULT
	devices[0] = 0;
	iRes = clGetDeviceIDs(platform, CL_DEVICE_TYPE_DEFAULT, 100, devices, NULL);
	bResult &= Check(L"CL_DEVICE_TYPE_DEFAULT", CL_SUCCESS, iRes);
	
	// CL_DEVICE_TYPE_ALL
	devices[0] = 0;
	iRes = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 100, devices, NULL);
	bResult &= Check(L"CL_DEVICE_TYPE_ALL", CL_SUCCESS, iRes);
	
	// CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_GPU
	devices[0] = 0;
	iRes = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_GPU, 100, devices, NULL);
	bResult &= Check(L"CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_GPU", CL_SUCCESS, iRes);
	
	// CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_ACCELERATOR
	devices[0] = 0;
	iRes = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_ACCELERATOR, 100, devices, NULL);
	bResult &= Check(L"CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_ACCELERATOR", CL_SUCCESS, iRes);
	
	// CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_DEFAULT
	devices[0] = 0;
	iRes = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_DEFAULT, 100, devices, NULL);
	bResult &= Check(L"CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_DEFAULT", CL_SUCCESS, iRes);
	
	// CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_ACCELERATOR
	devices[0] = 0;
	iRes = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_ACCELERATOR, 100, devices, NULL);
	bResult &= Check(L"CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_ACCELERATOR", CL_DEVICE_NOT_FOUND, iRes);
	
	// CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_DEFAULT
	devices[0] = 0;
	iRes = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_DEFAULT, 100, devices, NULL);
	bResult &= Check(L"CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_DEFAULT", CL_SUCCESS, iRes);
	
	// CL_DEVICE_TYPE_ACCELERATOR | CL_DEVICE_TYPE_DEFAULT
	devices[0] = 0;
	iRes = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ACCELERATOR | CL_DEVICE_TYPE_DEFAULT, 100, devices, NULL);
	bResult &= Check(L"CL_DEVICE_TYPE_ACCELERATOR | CL_DEVICE_TYPE_DEFAULT", CL_SUCCESS, iRes);
	
	// CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_ACCELERATOR
	devices[0] = 0;
	iRes = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_ACCELERATOR, 100, devices, NULL);
	bResult &= Check(L"CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_ACCELERATOR", CL_SUCCESS, iRes);
	
	// CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_DEFAULT
	devices[0] = 0;
	iRes = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_DEFAULT, 100, devices, NULL);
	bResult &= Check(L"CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_DEFAULT", CL_SUCCESS, iRes);
	
	// CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_ACCELERATOR | CL_DEVICE_TYPE_DEFAULT
	devices[0] = 0;
	iRes = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_ACCELERATOR | CL_DEVICE_TYPE_DEFAULT, 100, devices, NULL);
	bResult &= Check(L"CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_ACCELERATOR | CL_DEVICE_TYPE_DEFAULT", CL_SUCCESS, iRes);
	
	// CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_ACCELERATOR | CL_DEVICE_TYPE_DEFAULT
	devices[0] = 0;
	iRes = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_ACCELERATOR | CL_DEVICE_TYPE_DEFAULT, 100, devices, NULL);
	bResult &= Check(L"CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_ACCELERATOR | CL_DEVICE_TYPE_DEFAULT", CL_SUCCESS, iRes);

	return bResult;

}