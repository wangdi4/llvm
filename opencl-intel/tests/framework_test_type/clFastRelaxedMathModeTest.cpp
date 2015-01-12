#include "CL/cl.h"
#include "cl_types.h"
#include <stdio.h>
#include "FrameworkTest.h"
#include <gtest/gtest.h>
#include "cl_device_api.h"

#ifdef _WIN32
#include <windows.h>
#endif

#if defined(_WIN32)
#define SETENV(NAME,VALUE)      (SetEnvironmentVariableA(NAME,VALUE) != 0)
#define UNSETENV(NAME)          (SetEnvironmentVariableA(NAME,NULL) != 0)
#else
#define SETENV(NAME,VALUE)      (setenv(NAME,VALUE,1) == 0)
#define UNSETENV(NAME)          (unsetenv(NAME) == 0)
#endif

#define CL_CONFIG_USE_FAST_RELAXED_MATH "CL_CONFIG_USE_FAST_RELAXED_MATH"
#define CL_CONFIG_USE_FAST_RELAXED_MATH_VALUE "True"

extern cl_device_type gDeviceType;

//#define DEBUGGING_DEATH_TEST

static bool deathTestFailure()
{
#ifndef DEBUGGING_DEATH_TEST
    exit(1);
#endif
    return false;
}

static bool deathTestSuccess()
{
#ifndef DEBUGGING_DEATH_TEST
    exit(0);
#endif
    return true;
}

static bool setClFastRelaxedMathMode()
{
    bool bResult = true;

    bResult = SETENV(CL_CONFIG_USE_FAST_RELAXED_MATH, CL_CONFIG_USE_FAST_RELAXED_MATH_VALUE);

    //UNSETENV(CL_CONFIG_USE_FAST_RELAXED_MATH);

    if(!bResult)
        printf("ERROR: Cannot set CL_CONFIG_USE_FAST_RELAXED_MATH=%s!\n", CL_CONFIG_USE_FAST_RELAXED_MATH_VALUE);
    return bResult;
}

bool clFastRelaxedMathModeTest()
{
    bool bResult = setClFastRelaxedMathMode();
    if(!bResult) {
      return deathTestFailure();
    }

    const char *ocl_test_program = {\
    "__kernel void test_kernel(__global float* inout)\n"
    "{\n"
    "#ifndef __FAST_RELAXED_MATH__\n"
    "These erroneous lines indicate that -cl-fast-relaxed-math\n"
    "wasn't actually passed to the compiler build options.\n"
    "#endif\n"
    "  size_t gid = get_global_id(0);\n"
    "  inout[gid] = inout[gid] + inout[gid];\n"
    "}\n"
    };

    cl_uint uiNumDevices = 0;
    cl_device_id * pDevices;

    cl_context context;
    cl_program clProg;

    cl_platform_id platform = 0;

    cl_int iRet = clGetPlatformIDs(1, &platform, NULL);
    if (CL_SUCCESS != iRet)
    {
        printf("clGetPlatformIDs = %s\n", ClErrTxt(iRet));
        return deathTestFailure();
    }

     cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };

    // get device(s)
    iRet = clGetDeviceIDs(platform, gDeviceType, 0, NULL, &uiNumDevices);
    if (CL_SUCCESS != iRet)
    {
        printf("clGetDeviceIDs = %s\n", ClErrTxt(iRet));
        return deathTestFailure();
    }

    // initialize arrays
    pDevices = new cl_device_id[uiNumDevices];

    // initialize arrays pDevices = new cl_device_id[uiNumDevices];
    iRet = clGetDeviceIDs(platform, gDeviceType, uiNumDevices, pDevices, NULL);
    if (CL_SUCCESS != iRet)
    {
        printf("clGetDeviceIDs = %s\n", ClErrTxt(iRet));
        delete []pDevices;
        return deathTestFailure();
    }

    context = clCreateContext(prop, uiNumDevices, pDevices, NULL, NULL, &iRet);
    if (CL_SUCCESS != iRet)
    {
        printf("clCreateContext = %s\n", ClErrTxt(iRet));
        delete []pDevices;
        return deathTestFailure();
    }
    printf("context = %d\n", (std::size_t)context);

    clProg = clCreateProgramWithSource(context, 1,
        (const char**)&ocl_test_program, NULL, &iRet);
    if (CL_SUCCESS != iRet)
    {
        printf("clCreateProgramWithSource = %s\n", ClErrTxt(iRet));
        delete []pDevices;
        return deathTestFailure();
    }

    iRet = clBuildProgram(clProg, 1, &pDevices[0], NULL, NULL, NULL);
    if (CL_SUCCESS != iRet)
    {
        printf("clBuildProgram = %s\n", ClErrTxt(iRet));
        delete []pDevices;
        return deathTestFailure();
    }
    else
        printf("Built program successfully\n");

    // Release objects
    clReleaseProgram(clProg);
    clReleaseContext(context);
    delete []pDevices;

    return deathTestSuccess();
}

void clFastRelaxedMathModeDeathTest()
{
#ifdef DEBUGGING_DEATH_TEST
    EXPECT_TRUE( clFastRelaxedMathModeTest() );
#else
    ::testing::FLAGS_gtest_death_test_style = "threadsafe";
    EXPECT_EXIT( { clFastRelaxedMathModeTest(); exit(1); }, ::testing::ExitedWithCode(0), "" );
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////
//
// Do not remove DeathTest suffix from the test names - it is the Google Test requirement
//
////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __ANDROID__

TEST(FrameworkTestTypeDeathTest, Test_ClFastRelaxedMathModeTest_Basic)
{
    clFastRelaxedMathModeDeathTest();
}

#endif // __ANDROID__
