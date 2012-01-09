#include "CL/cl.h"
#include "cl_types.h"
#include "Logger.h"
#include "cl_objects_map.h"
#include <stdio.h>
#include "FrameworkTest.h"
#include "string.h"
using namespace Intel::OpenCL::Framework;

/**************************************************************************************************
* clGetPlatformInfoTest
* -------------------
* Get platform info (CL_PLATFORM_PROFILE)
* Get platform info (CL_PLATFORM_VERSION)
**************************************************************************************************/
bool clGetPlatformInfoTest()
{
	printf("---------------------------------------\n");
	printf("clGetPlatformInfo\n");
	printf("---------------------------------------\n");
	bool bResult = true;
	char platformInfoStr[256];
	size_t size_ret;

	cl_platform_id platform = 0;

	cl_int iRes = clGetPlatformIDs(1, &platform, NULL);
	bResult &= Check(L"clGetPlatformIDs", CL_SUCCESS, iRes);

	if (!bResult)
	{
		return bResult;
	}

	// CL_PLATFORM_PROFILE
	// all NULL
	iRes = clGetPlatformInfo(platform, CL_PLATFORM_PROFILE, 0, NULL, NULL);
	bResult &= Check(L"CL_PLATFORM_PROFILE, all NULL", CL_INVALID_VALUE, iRes);

	// CL_PLATFORM_PROFILE
	// get size only
	iRes = clGetPlatformInfo(platform, CL_PLATFORM_PROFILE, 0, NULL, &size_ret);
	bResult &= Check(L"CL_PLATFORM_PROFILE, get size only", CL_SUCCESS, iRes);
	if (CL_SUCCEEDED(iRes))
	{
		bResult &= CheckSize(L"check value", 13, size_ret);
	}

	// CL_PLATFORM_PROFILE
	// size < actual size
	iRes = clGetPlatformInfo(platform, CL_PLATFORM_PROFILE, 5, platformInfoStr, &size_ret);
	bResult &= Check(L"CL_PLATFORM_PROFILE, size < actual size", CL_INVALID_VALUE, iRes);

	// CL_PLATFORM_PROFILE
	// size ok, no size return
	iRes = clGetPlatformInfo(platform, CL_PLATFORM_PROFILE, 256, platformInfoStr, NULL);
	bResult &= Check(L"CL_PLATFORM_PROFILE, size ok, no size return", CL_SUCCESS, iRes);
	if (CL_SUCCEEDED(iRes))
	{
		bResult &= CheckStr(L"check value", "FULL_PROFILE", platformInfoStr);
	}

	// CL_PLATFORM_VERSION
	// all NULL
	iRes = clGetPlatformInfo(platform, CL_PLATFORM_VERSION, 0, NULL, NULL);
	bResult &= Check(L"CL_PLATFORM_VERSION, all NULL", CL_INVALID_VALUE, iRes);

	// CL_PLATFORM_VERSION
	// get size only
	iRes = clGetPlatformInfo(platform, CL_PLATFORM_VERSION, 0, NULL, &size_ret);
	bResult &= Check(L"CL_PLATFORM_VERSION, get size only", CL_SUCCESS, iRes);
	if (CL_SUCCEEDED(iRes))
	{
#ifdef _WIN32
		bResult &= CheckSize(L"check value", 19, size_ret);
#else
		bResult &= CheckSize(L"check value", 17, size_ret);
#endif
	}

	// CL_PLATFORM_VERSION
	// size < actual size
	iRes = clGetPlatformInfo(platform, CL_PLATFORM_VERSION, 5, platformInfoStr, &size_ret);
	bResult &= Check(L"CL_PLATFORM_VERSION, size < actual size", CL_INVALID_VALUE, iRes);

	// CL_PLATFORM_VERSION
	// size ok, no size return
	iRes = clGetPlatformInfo(platform, CL_PLATFORM_VERSION, 256, platformInfoStr, NULL);
	bResult &= Check(L"CL_PLATFORM_VERSION, size ok, no size return", CL_SUCCESS, iRes);
	if (CL_SUCCEEDED(iRes))
	{
#ifdef _WIN32
		bResult &= CheckStr(L"check value", "OpenCL 1.1 WINDOWS", platformInfoStr);
#else
		bResult &= CheckStr(L"check value", "OpenCL 1.1 LINUX", platformInfoStr);
#endif
	}
	return bResult;
}
