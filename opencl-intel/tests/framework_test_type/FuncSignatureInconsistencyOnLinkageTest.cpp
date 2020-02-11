#include "CL/cl.h"
#include "FrameworkTest.h"
#include "cl_types.h"
#include <gtest/gtest.h>
#include <stdio.h>

#include <string>

extern cl_device_type gDeviceType;

void clFuncSignatureInconsistencyOnLinkageTest() {
  const char *MainProg = "\
    #ifdef __cplusplus\n\
    extern \" C \" {\n\
    #endif\n\
    int testLibFunc(int *ptr);\n\
    #ifdef __cplusplus\n\
    }\n\
    #endif\n\
    \n\
    __kernel void test(__global int *a,\n\
                      __global int *res)\n\
    {\n\
        int gid = get_global_id(0);\n\
        int b = a[gid];\n\
        res[gid] = testLibFunc(&b);\n\
    }\n\
    ";

  const char *LibProg = "\
    int testLibFunc(__global int *ptr)\n\
    {\n\
        return *ptr;\n\
    }\n\
    ";

  printf("---------------------------------------\n");
  printf("clFuncSignatureInconsistencyOnLinkageTest\n");
  printf("---------------------------------------\n");

  cl_int iRet = CL_SUCCESS;
  cl_platform_id platform = nullptr;
  cl_device_id device = nullptr;

  // Get platform
  iRet = clGetPlatformIDs(/*num_entries=*/1, &platform,
                          /*num_platforms=*/nullptr);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clGetPlatformIDs failed.";

  // Get device
  iRet = clGetDeviceIDs(platform, gDeviceType, /*num_entries=*/1, &device,
                        /*num_devices=*/nullptr);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clGetDeviceIDs failed on trying to obtain "
                              << gDeviceType << " device type.";

  // Create context
  const cl_context_properties prop[5] = {CL_CONTEXT_PLATFORM,
                                         (cl_context_properties)platform, 0};
  cl_context context = clCreateContext(prop, /*num_devices=*/1, &device,
                                       /*pfn_notify=*/nullptr,
                                       /*user_data=*/nullptr, &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateContext failed.";

  // Create programs with source
  cl_program PrMain = clCreateProgramWithSource(
      context, 1, (const char **)&MainProg, NULL, &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateProgramWithSource failed.";

  cl_program PrLib = clCreateProgramWithSource(
      context, 1, (const char **)&LibProg, NULL, &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateProgramWithSource failed.";

  // Compile programs
  iRet = clCompileProgram(PrMain, 1, &device, "-cl-std=CL2.0", 0,
                          NULL, NULL, NULL, NULL);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCompileProgram for PrMain failed.";

  iRet = clCompileProgram(PrLib, 1, &device, "-cl-std=CL2.0", 0,
                          NULL, NULL, NULL, NULL);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCompileProgram for PrLib failed.";

  // Try to link program with library to an executable
  const cl_program ToLink[] = {PrMain, PrLib};

  cl_program PrMainLinked =
      clLinkProgram(context, 1, &device, "", 2, ToLink, NULL, NULL, &iRet);
  ASSERT_EQ(CL_LINK_PROGRAM_FAILURE, iRet)
      << " clLinkProgram unexpectedly passed.";

  // Check that CL_PROGRAM_BUILD_LOG query returns linkage error message
  size_t SizePrMainLinkLog = 0;
  iRet = clGetProgramBuildInfo(PrMainLinked, device, CL_PROGRAM_BUILD_LOG,
                               0, NULL, &SizePrMainLinkLog);
  ASSERT_EQ(CL_SUCCESS, iRet)
      << " Device failed to return linkage log size.";

  char *LogPrMainLink = (char *)malloc(SizePrMainLinkLog);
  iRet = clGetProgramBuildInfo(PrMainLinked, device, CL_PROGRAM_BUILD_LOG,
                               SizePrMainLinkLog, LogPrMainLink, NULL);
  ASSERT_EQ(CL_SUCCESS, iRet)
      << " clGetProgramBuildInfo CL_PROGRAM_BUILD_LOG failed";
  ASSERT_TRUE(LogPrMainLink) << " No Link process log";

  std::string LogPrMainLinkStr{LogPrMainLink};
  ASSERT_TRUE(std::string::npos != LogPrMainLinkStr.find(
      "testLibFunc [arguments in different address spaces]"))
      << " Program Linkage failure description not found";

  free(LogPrMainLink);
  clReleaseProgram(PrMain);
  clReleaseProgram(PrLib);
  clReleaseProgram(PrMainLinked);
  clReleaseContext(context);
}
