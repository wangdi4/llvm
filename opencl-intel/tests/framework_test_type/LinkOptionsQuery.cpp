#include "CL/cl.h"
#include "FrameworkTest.h"
#include "cl_types.h"
#include "gtest_wrapper.h"
#include <stdio.h>
#include <string>

extern cl_device_type gDeviceType;

void clGetProgramBuildInfoTest() {
  const char *Program0 = "\
    __kernel void test_kernel0(__global int* pBuff)\n\
    {\n\
        size_t id = get_global_id(0);\n\
        pBuff[id] = id * 1;\n\
    }\n\
    ";

  const char *Program1 = "\
    __kernel void test_kernel1(__global int* pBuff)\n\
    {\n\
        size_t id = get_global_id(0);\n\
        pBuff[id] = id * 2;\n\
    }\n\
    ";

  const char *Program2 = "\
    __kernel void test_kernel2(__global int* pBuff)\n\
    {\n\
        size_t id = get_global_id(0);\n\
        pBuff[id] = id * 42;\n\
    }\n\
    ";

  printf("---------------------------------------\n");
  printf("clGetProgramBuildInfoTest\n");
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
  cl_program Pr0 = clCreateProgramWithSource(
      context, 1, (const char **)&Program0, NULL, &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateProgramWithSource failed.";

  cl_program Pr1 = clCreateProgramWithSource(
      context, 1, (const char **)&Program1, NULL, &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateProgramWithSource failed.";

  cl_program Pr2 = clCreateProgramWithSource(
      context, 1, (const char **)&Program2, NULL, &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet)
      << " clCreateProgramWithSource for Program2 failed.";

  // Compile programs
  // All programs compiler with '-cl-fast-relaxed-math' option,
  // which will be keeped after link and passed to the backend.
  // But it shouldn't be exposed to an user through
  // clGetProgramBuildInfo() 'CL_PROGRAM_BUILD_OPTIONS' query
  iRet = clCompileProgram(Pr0, 1, &device, "-cl-fast-relaxed-math", 0, NULL,
                          NULL, NULL, NULL);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCompileProgram for Pr0 failed.";

  iRet = clCompileProgram(Pr1, 1, &device, "-cl-fast-relaxed-math", 0, NULL,
                          NULL, NULL, NULL);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCompileProgram for Pr1 failed.";

  iRet = clCompileProgram(Pr2, 1, &device, "-cl-denorms-are-zero", 0, NULL,
                          NULL, NULL, NULL);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCompileProgram for Pr2 failed.";

  // Link programs 1 and 2 to an executable
  const cl_program ToLink[] = {Pr0, Pr1};

  cl_program Pr01Linked =
      clLinkProgram(context, 1, &device, "-cl-denorms-are-zero", 2, ToLink,
                    NULL, NULL, &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateProgramWithSource failed.";

  // Check that CL_PROGRAM_BUILD_OPTIONS query returns the options have been
  // specified in clLinkProgram(),
  // '-cl-fast-relaxed-math' shouldn't be exposed
  size_t SizePr01Linked = 0;
  iRet = clGetProgramBuildInfo(Pr01Linked, device, CL_PROGRAM_BUILD_OPTIONS, 0,
                               NULL, &SizePr01Linked);
  ASSERT_EQ(CL_SUCCESS, iRet)
      << " Device failed to return compile options size.";

  char *OptsPr01Linked = (char *)malloc(SizePr01Linked);
  iRet = clGetProgramBuildInfo(Pr01Linked, device, CL_PROGRAM_BUILD_OPTIONS,
                               SizePr01Linked, OptsPr01Linked, NULL);
  ASSERT_EQ(CL_SUCCESS, iRet)
      << " clGetProgramBuildInfo CL_PROGRAM_BUILD_OPTIONS failed";
  ASSERT_TRUE(OptsPr01Linked) << " No LinkOptions";
  ASSERT_EQ("-cl-denorms-are-zero", std::string(OptsPr01Linked))
      << " Inconsist options after clLinkProgram()";

  // Link programs 1 and 2 to a library
  cl_program Pr01LinkedToLib =
      clLinkProgram(context, 1, &device, "-create-library -cl-denorms-are-zero",
                    2, ToLink, NULL, NULL, &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateProgramWithSource failed.";

  const cl_program ToLink2[] = {Pr01LinkedToLib, Pr2};

  // Link the library and program 3
  cl_program Pr012Linked =
      clLinkProgram(context, 1, &device, "-cl-fast-relaxed-math", 2, ToLink2,
                    NULL, NULL, &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateProgramWithSource failed.";

  // Check that CL_PROGRAM_BUILD_OPTIONS query returns the options have been
  // specified in last clLinkProgram(), '-cl-denorms-are-zero' shouldn't
  // be exposed
  size_t SizePr012Linked = 0;
  iRet = clGetProgramBuildInfo(Pr012Linked, device, CL_PROGRAM_BUILD_OPTIONS, 0,
                               NULL, &SizePr012Linked);
  ASSERT_EQ(CL_SUCCESS, iRet)
      << " Device failed to return compile options size.";

  char *OptsPr012Linked = (char *)malloc(SizePr012Linked);
  iRet = clGetProgramBuildInfo(Pr012Linked, device, CL_PROGRAM_BUILD_OPTIONS,
                               SizePr012Linked, OptsPr012Linked, NULL);
  ASSERT_EQ(CL_SUCCESS, iRet)
      << " clGetProgramBuildInfo CL_PROGRAM_BUILD_OPTIONS failed";
  ASSERT_TRUE(OptsPr012Linked) << " No LinkOptions";
  ASSERT_EQ("-cl-fast-relaxed-math", std::string(OptsPr012Linked))
      << " Inconsist options after clLinkProgram()";

  clReleaseProgram(Pr0);
  clReleaseProgram(Pr1);
  clReleaseProgram(Pr2);
  clReleaseProgram(Pr01Linked);
  clReleaseProgram(Pr01LinkedToLib);
  clReleaseProgram(Pr012Linked);
  clReleaseContext(context);

  free(OptsPr01Linked);
  free(OptsPr012Linked);
}
