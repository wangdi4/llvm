#include "CL/cl.h"
#include "FrameworkTest.h"
#include "cl_types.h"
#include "gtest_wrapper.h"
#include <stdio.h>
#include <string>

extern cl_device_type gDeviceType;

static void
tryToBuildProgram(const char *ClSrc1, const char *ClSrc2,
                  std::string &LinkLogStr,
                  cl_int ExpectedLinkReturnCode = CL_LINK_PROGRAM_FAILURE) {
  LinkLogStr.clear();

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
  cl_program ClProg1 = clCreateProgramWithSource(
      context, 1, (const char **)&ClSrc1, NULL, &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet)
      << " clCreateProgramWithSource for 1st source file failed.";

  cl_program ClProg2 = clCreateProgramWithSource(
      context, 1, (const char **)&ClSrc2, NULL, &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet)
      << " clCreateProgramWithSource for 2nd source file failed.";

  // Compile programs
  iRet = clCompileProgram(ClProg1, 1, &device, "-cl-std=CL2.0", 0, NULL, NULL,
                          NULL, NULL);
  ASSERT_EQ(CL_SUCCESS, iRet)
      << " clCompileProgram for 1st source file failed.";

  iRet = clCompileProgram(ClProg2, 1, &device, "-cl-std=CL2.0", 0, NULL, NULL,
                          NULL, NULL);
  ASSERT_EQ(CL_SUCCESS, iRet)
      << " clCompileProgram for 2nd source file failed.";

  // Try to link programs to an executable
  const cl_program ToLink[] = {ClProg1, ClProg2};

  cl_program ClProgLinked =
      clLinkProgram(context, 1, &device, "", 2, ToLink, NULL, NULL, &iRet);
  ASSERT_EQ(ExpectedLinkReturnCode, iRet)
      << " clLinkProgram unexpected return code.";

  // Check that CL_PROGRAM_BUILD_LOG query returns linkage error message
  size_t LinkLogSize = 0;
  iRet = clGetProgramBuildInfo(ClProgLinked, device, CL_PROGRAM_BUILD_LOG, 0,
                               NULL, &LinkLogSize);
  ASSERT_EQ(CL_SUCCESS, iRet) << " Device failed to return linkage log size.";

  char *LinkLog = (char *)malloc(LinkLogSize);
  iRet = clGetProgramBuildInfo(ClProgLinked, device, CL_PROGRAM_BUILD_LOG,
                               LinkLogSize, LinkLog, NULL);
  ASSERT_EQ(CL_SUCCESS, iRet)
      << " clGetProgramBuildInfo CL_PROGRAM_BUILD_LOG failed";
  ASSERT_TRUE(LinkLog) << " No Link process log";

  LinkLogStr.assign(LinkLog);

  free(LinkLog);
  clReleaseProgram(ClProg1);
  clReleaseProgram(ClProg2);
  clReleaseProgram(ClProgLinked);
  clReleaseContext(context);
}

void clFuncIncompatParamASOnLinkageTest() {
  const char *ClSrc1 = "\
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

  const char *ClSrc2 = "\
    int testLibFunc(__global int *ptr)\n\
    {\n\
        return *ptr;\n\
    }\n\
    ";

  printf("---------------------------------------\n");
  printf("clFuncIncompatParamASOnLinkageTest\n");
  printf("---------------------------------------\n");

  std::string LinkLogStr;
  tryToBuildProgram(ClSrc1, ClSrc2, LinkLogStr);

  ASSERT_TRUE(
      std::string::npos !=
      LinkLogStr.find(
          "testLibFunc [passing parameter 1 with incompatible address space]"))
      << "Expected link error description not found";
}

void clFuncWrongNumParamsOnLinkageTest() {
  const char *ClSrc1 = "\
    #ifdef __cplusplus\n\
    extern \" C \" {\n\
    #endif\n\
    int testLibFunc(int a, int b);\n\
    #ifdef __cplusplus\n\
    }\n\
    #endif\n\
    \n\
    __kernel void test(__global int *a,\n\
                      __global int *res)\n\
    {\n\
        int gid = get_global_id(0);\n\
        int b = a[gid];\n\
        res[gid] = testLibFunc(b, gid);\n\
    }\n\
    ";

  const char *ClSrc2 = "\
    int testLibFunc(int a)\n\
    {\n\
        return a*a;\n\
    }\n\
    ";

  printf("---------------------------------------\n");
  printf("clFuncWrongNumParamsOnLinkageTest\n");
  printf("---------------------------------------\n");

  std::string LinkLogStr;
  tryToBuildProgram(ClSrc1, ClSrc2, LinkLogStr);

  ASSERT_TRUE(std::string::npos !=
              LinkLogStr.find("testLibFunc [wrong number of arguments to "
                              "function call, expected 1, have 2]"))
      << "Expected link error description not found";
}

void clFuncIdenticalLayoutStructOnLinkageTest() {
  const char *ClSrc1 = R"(
typedef struct A StructA;
typedef struct B {
  int x;
  int y;
} StructB;

#ifdef __cplusplus
extern "C" {
#endif
  int libfuncA(StructA *);
  int libfuncB(StructB *);
#ifdef __cplusplus
}
#endif

__kernel void test(__global int *x, __global StructA *y, __global StructB *z) {
  *x = libfuncA(y);
  *x = libfuncB(z);
}
    )";

  const char *ClSrc2 = R"(
typedef struct A {
  int x;
  int y;
} StructA;

int libfuncA(StructA *V) { return V->x; }
int libfuncB(StructA *V) { return V->y; }
    )";

  printf("---------------------------------------\n");
  printf("clFuncIdenticalLayoutStructOnLinkageTest\n");
  printf("---------------------------------------\n");

  std::string LinkLogStr;
  tryToBuildProgram(ClSrc1, ClSrc2, LinkLogStr,
                    /*ExpectedLinkReturnCode*/ CL_SUCCESS);
}
