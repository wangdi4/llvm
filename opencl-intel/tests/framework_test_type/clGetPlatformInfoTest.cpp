#include "CL/cl.h"
#include "cl_types.h"
#include <stdio.h>
#include "FrameworkTest.h"
#include "string.h"
#include "cl_config.h"

/**************************************************************************************************
* clGetPlatformInfoTest
* -------------------
* Get platform info (CL_PLATFORM_PROFILE)
* Get platform info (CL_PLATFORM_VERSION)
**************************************************************************************************/
using namespace Intel::OpenCL::Utils;

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
    bResult &= Check("clGetPlatformIDs", CL_SUCCESS, iRes);

    if (!bResult)
    {
        return bResult;
    }

    // CL_PLATFORM_PROFILE
    // all NULL is allowed, albeit useless.
    iRes = clGetPlatformInfo(platform, CL_PLATFORM_PROFILE, 0, NULL, NULL);
    bResult &= Check("CL_PLATFORM_PROFILE, all NU", CL_SUCCESS, iRes);

    // CL_PLATFORM_PROFILE
    // get size only
    iRes = clGetPlatformInfo(platform, CL_PLATFORM_PROFILE, 0, NULL, &size_ret);
    bResult &= Check("CL_PLATFORM_PROFILE, get size only", CL_SUCCESS, iRes);
    if (CL_SUCCEEDED(iRes))
    {
        bResult &= CheckSize("check value", 13, size_ret);
    }

    // CL_PLATFORM_PROFILE
    // size < actual size
    iRes = clGetPlatformInfo(platform, CL_PLATFORM_PROFILE, 5, platformInfoStr, &size_ret);
    bResult &= Check("CL_PLATFORM_PROFILE, size < actual size", CL_INVALID_VALUE, iRes);

    // CL_PLATFORM_PROFILE
    // size ok, no size return
    iRes = clGetPlatformInfo(platform, CL_PLATFORM_PROFILE, 256, platformInfoStr, NULL);
    bResult &= Check("CL_PLATFORM_PROFILE, size ok, no size return", CL_SUCCESS, iRes);
    if (CL_SUCCEEDED(iRes))
    {
        bResult &= CheckStr("check value", "FULL_PROFILE", platformInfoStr);
    }

    // CL_PLATFORM_VERSION
    // all NULL is allowed, albeit useless.
    iRes = clGetPlatformInfo(platform, CL_PLATFORM_VERSION, 0, NULL, NULL);
    bResult &= Check("CL_PLATFORM_VERSION, all NU", CL_SUCCESS, iRes);

    // CL_PLATFORM_VERSION
    // get size only
    iRes = clGetPlatformInfo(platform, CL_PLATFORM_VERSION, 0, NULL, &size_ret);
    bResult &= Check("CL_PLATFORM_VERSION, get size only", CL_SUCCESS, iRes);
    if (CL_SUCCEEDED(iRes))
    {
#ifdef _WIN32
        bResult &= CheckSize("check value", 19, size_ret);
#else
        bResult &= CheckSize("check value", 17, size_ret);
#endif
    }

    // CL_PLATFORM_VERSION
    // size < actual size
    iRes = clGetPlatformInfo(platform, CL_PLATFORM_VERSION, 5, platformInfoStr, &size_ret);
    bResult &= Check("CL_PLATFORM_VERSION, size < actual size", CL_INVALID_VALUE, iRes);

    // CL_PLATFORM_VERSION
    // size ok, no size return
    iRes = clGetPlatformInfo(platform, CL_PLATFORM_VERSION, 256, platformInfoStr, NULL);
    bResult &= Check("CL_PLATFORM_VERSION, size ok, no size return", CL_SUCCESS, iRes);
    if (CL_SUCCEEDED(iRes))
    {
        std::string expectedString;
        expectedString += "OpenCL ";

        #ifdef BUILD_EXPERIMENTAL_21
        {
            expectedString += "2.1 ";
        }
        #else //BUILD_EXPERIMENTAL_21
        {
            switch (GetOpenclVerByCpuModel())
            {
                case OPENCL_VERSION_2_2:
                    expectedString += "2.2 ";
                    break;

                case OPENCL_VERSION_2_1:
                    expectedString += "2.1 ";
                    break;

                case OPENCL_VERSION_2_0:
                    expectedString += "2.0 ";
                    break;

                case OPENCL_VERSION_1_2:
                default:
                    expectedString += "1.2 ";
                    break;
            }
        }
        #endif //BUILD_EXPERIMENTAL_21

        #ifdef _WIN32
        {
            expectedString += "WINDOWS";
        }
        #elif defined (__linux__) && !defined (__ANDROID__)
        {
            expectedString += "LINUX";
        }
        #elif defined (__ANDROID__)
        {
            expectedString += "LINUX";
        }
        #else
        {
            #error Unhandled platform!
        }
        #endif


        bResult &= CheckStr("check value",
                &expectedString[0], platformInfoStr);
    }

    return bResult;
}
