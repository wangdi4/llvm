// INTEL CONFIDENTIAL
//
// Copyright 2019-2020 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "FrameworkTest.h"
#include "TestsHelpClasses.h"
#include "common_utils.h"
#include <CL/cl.h>
#include <CL/cl_gvp_ext.h>
#include <time.h>

extern cl_device_type gDeviceType;

class GVPointerExtensionTest : public ::testing::TestWithParam<bool> {
public:
  GVPointerExtensionTest()
      : m_platform(nullptr), m_device(nullptr), m_context(nullptr),
        m_queue(nullptr), m_clGetDeviceGlobalVariablePointerINTEL(nullptr) {}

protected:
  virtual void SetUp() {
    m_options = "-cl-std=CL2.0";
    if (GetParam()) {
      m_options += " -g -cl-opt-disable";
#if defined _WIN32 && (defined _M_X64 || defined __x86_64__)
      ASSERT_TRUE(SETENV("CL_CONFIG_USE_NATIVE_DEBUGGER", "1"));
#endif
    }

    cl_int err = clGetPlatformIDs(1, &m_platform, nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetPlatformIDs");

    err = clGetDeviceIDs(m_platform, gDeviceType, 1, &m_device, nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceIDs");

    m_context = clCreateContext(nullptr, 1, &m_device, nullptr, nullptr, &err);
    ASSERT_OCL_SUCCESS(err, "clCreateContext");

    m_queue =
        clCreateCommandQueueWithProperties(m_context, m_device, nullptr, &err);
    ASSERT_OCL_SUCCESS(err, "clCreateCommandQueueWithProperties");

    // Get extension function address
    m_clGetDeviceGlobalVariablePointerINTEL =
        (clGetDeviceGlobalVariablePointerINTEL_fn)
            clGetExtensionFunctionAddressForPlatform(
                m_platform, "clGetDeviceGlobalVariablePointerINTEL");
    ASSERT_NE(nullptr, m_clGetDeviceGlobalVariablePointerINTEL)
        << "clGetExtensionFunctionAddressForPlatform("
           "\"clGetDeviceGlobalVariablePointerINTEL\") failed.";

    srand((unsigned)time(nullptr));
  }

  virtual void TearDown() {
    cl_int err;
    if (m_queue) {
      err = clReleaseCommandQueue(m_queue);
      EXPECT_OCL_SUCCESS(err, "clReleaseCommandQueue");
    }
    if (m_context) {
      err = clReleaseContext(m_context);
      EXPECT_OCL_SUCCESS(err, "clReleaseContext");
    }
  }

  void CheckBuildError(cl_program &program, cl_int err) {
    if (CL_SUCCESS != err) {
      std::string log;
      ASSERT_NO_FATAL_FAILURE(GetBuildLog(m_device, program, log));
      FAIL() << log;
    }
  }

  // Build program from source and check global variable names.
  // gvName is global variable and negativeName is not.
  void TestBuildProgram(const char *source, cl_program &program,
                        const char *gvName, const char *negativeName) {
    cl_int err;

    // Check before creating program
    size_t gvSize;
    void *gvPtr;
    err = m_clGetDeviceGlobalVariablePointerINTEL(m_device, program, gvName,
                                                  &gvSize, &gvPtr);
    ASSERT_EQ(CL_INVALID_PROGRAM, err)
        << "clGetDeviceGlobalVariablePointerINTEL "
           "must return CL_INVALID_PROGRAM if "
           "program is not a valid OpenCL program";

    // Create program
    program = clCreateProgramWithSource(m_context, 1, &source, nullptr, &err);
    ASSERT_OCL_SUCCESS(err, "clCreateProgramWithSource");

    // Check before building program
    err = m_clGetDeviceGlobalVariablePointerINTEL(m_device, program, gvName,
                                                  nullptr, &gvPtr);
    ASSERT_EQ(CL_INVALID_PROGRAM_EXECUTABLE, err)
        << "clGetDeviceGlobalVariablePointerINTEL must return "
           "CL_INVALID_PROGRAM_EXECUTABLE if program wasn't build";

    // Build program
    err = clBuildProgram(program, 1, &m_device, m_options.c_str(), nullptr,
                         nullptr);
    ASSERT_NO_FATAL_FAILURE(CheckBuildError(program, err));

    // Check after building program
    err = m_clGetDeviceGlobalVariablePointerINTEL(m_device, program, gvName,
                                                  &gvSize, nullptr);
    ASSERT_EQ(CL_INVALID_VALUE, err)
        << "clGetDeviceGlobalVariablePointerINTEL "
           "must return CL_INVALID_VALUE if global_variable_pointer_ret is "
           "nullptr";

    err = m_clGetDeviceGlobalVariablePointerINTEL(m_device, program, nullptr,
                                                  &gvSize, &gvPtr);
    ASSERT_EQ(CL_INVALID_VALUE, err)
        << "clGetDeviceGlobalVariablePointerINTEL "
           "must return CL_INVALID_VALUE if global_variable_name is nullptr";

    err = m_clGetDeviceGlobalVariablePointerINTEL(nullptr, program, gvName,
                                                  nullptr, &gvPtr);
    ASSERT_EQ(CL_INVALID_DEVICE, err);

    err = m_clGetDeviceGlobalVariablePointerINTEL(
        m_device, program, negativeName, &gvSize, &gvPtr);
    ASSERT_EQ(CL_INVALID_ARG_VALUE, err);
  }

  // Test clCreateProgramWithBinary
  void TestProgramWithBinary(cl_program program, const char *gvName,
                             size_t gvSize, const char *negativeName) {
    cl_program program1;
    ASSERT_NO_FATAL_FAILURE(CreateAndBuildProgramFromProgramBinaries(
        m_context, m_device, m_options, program, program1));

    size_t size;
    void *ptr;
    cl_int err = m_clGetDeviceGlobalVariablePointerINTEL(m_device, program1,
                                                         gvName, &size, &ptr);
    ASSERT_OCL_SUCCESS(err, "m_clGetDeviceGlobalVariablePointerINTEL");
    ASSERT_NE(nullptr, ptr);
    ASSERT_EQ(size, gvSize);

    err = m_clGetDeviceGlobalVariablePointerINTEL(m_device, program1,
                                                  negativeName, &size, &ptr);
    ASSERT_EQ(CL_INVALID_ARG_VALUE, err);

    err = clReleaseProgram(program1);
    EXPECT_OCL_SUCCESS(err, "clReleaseProgram");
  }

protected:
  cl_platform_id m_platform;
  cl_device_id m_device;
  cl_context m_context;
  cl_command_queue m_queue;
  clGetDeviceGlobalVariablePointerINTEL_fn
      m_clGetDeviceGlobalVariablePointerINTEL;
  std::string m_options;
};

// Test clGetDeviceGlobalVariablePointerINTEL within a program.
TEST_P(GVPointerExtensionTest, singleProgram) {
  cl_int err;

  cl_int x = rand() & 255;
  std::ostringstream ss;
  ss << "global int x = " << x
     << ";\n"
        "kernel void test(global int* dst) {\n"
        "  if (get_global_id(0) == 0)\n"
        "    dst[0] = x * 2;\n"
        "}";
  std::string source = ss.str();

  // Build program
  cl_program program;
  ASSERT_NO_FATAL_FAILURE(
      TestBuildProgram(source.c_str(), program, "x", "test"));

  // Get global variable pointer
  size_t gvSize;
  void *gvPtr;
  err = m_clGetDeviceGlobalVariablePointerINTEL(m_device, program, "x", &gvSize,
                                                &gvPtr);
  ASSERT_OCL_SUCCESS(err, "clGetDeviceGlobalVariablePointerINTEL");
  ASSERT_NE(nullptr, gvPtr)
      << "clGetDeviceGlobalVariablePointerINTEL must return a non-nullptr "
         "value via global_variable_pointer_ret on success";
  cl_int *gvPtrInt = static_cast<cl_int *>(gvPtr);
  ASSERT_EQ(sizeof(x), gvSize);
  ASSERT_EQ(x, *gvPtrInt);

  // Check gvPtr is USM pointer
  cl_unified_shared_memory_type_intel memType;
  err = clGetMemAllocInfoINTEL(m_context, gvPtr, CL_MEM_ALLOC_TYPE_INTEL,
                               sizeof(memType), &memType, nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetMemAllocInfoINTEL");
  ASSERT_EQ(CL_MEM_TYPE_DEVICE_INTEL, memType);

  size_t bufferSize;
  err = clGetMemAllocInfoINTEL(m_context, gvPtr, CL_MEM_ALLOC_SIZE_INTEL,
                               sizeof(bufferSize), &bufferSize, nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetMemAllocInfoINTEL");
  ASSERT_EQ(gvSize, bufferSize);

  cl_device_id device;
  err = clGetMemAllocInfoINTEL(m_context, gvPtr, CL_MEM_ALLOC_DEVICE_INTEL,
                               sizeof(device), &device, nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetMemAllocInfoINTEL");
  ASSERT_EQ(m_device, device);

  // Test JIT save/load
  ASSERT_NO_FATAL_FAILURE(TestProgramWithBinary(program, "x", gvSize, "test"));

  // Create and execute Kernel
  cl_kernel kernel = clCreateKernel(program, "test", &err);
  ASSERT_OCL_SUCCESS(err, "clCreateKernel");
  err = clSetKernelArgMemPointerINTEL(kernel, 0, gvPtrInt);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArgMemPointerINTEL");
  size_t gdim = 1;
  size_t ldim = 1;
  err = clEnqueueNDRangeKernel(m_queue, kernel, 1, nullptr, &gdim, &ldim, 0,
                               nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueNDRangeKernel");
  err = clFinish(m_queue);
  ASSERT_OCL_SUCCESS(err, "clFinish");
  EXPECT_EQ(x * 2, *gvPtrInt);

  // Release resources
  err = clReleaseKernel(kernel);
  EXPECT_OCL_SUCCESS(err, "clReleaseKernel");
  err = clReleaseProgram(program);
  EXPECT_OCL_SUCCESS(err, "clReleaseProgram");
}

// Unreferenced global variable should be queryable by
// clGetDeviceGlobalVariablePointerINTEL.
TEST_P(GVPointerExtensionTest, singleProgramUnreferencedGlobal) {
  // Build program
  const char *source = "global uchar x;";
  cl_int err;
  cl_program program =
      clCreateProgramWithSource(m_context, 1, &source, nullptr, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateProgramWithSource");
  err = clBuildProgram(program, 1, &m_device, m_options.c_str(), nullptr,
                       nullptr);
  ASSERT_NO_FATAL_FAILURE(CheckBuildError(program, err));

  // Get global variable pointer
  size_t gvSize;
  void *gvPtr;
  err = m_clGetDeviceGlobalVariablePointerINTEL(m_device, program, "x", &gvSize,
                                                &gvPtr);
  EXPECT_OCL_SUCCESS(err, "clGetDeviceGlobalVariablePointerINTEL");
  EXPECT_NE(nullptr, gvPtr)
      << "clGetDeviceGlobalVariablePointerINTEL must return a non-nullptr "
         "value via global_variable_pointer_ret on success";
  EXPECT_EQ(sizeof(cl_uchar), gvSize);

  // Test JIT save/load
  ASSERT_NO_FATAL_FAILURE(TestProgramWithBinary(program, "x", gvSize, "y"));
  err = clReleaseProgram(program);
  ASSERT_OCL_SUCCESS(err, "clReleaseProgram");
}

// Constant global variable should be queryable.
TEST_P(GVPointerExtensionTest, constantGlobal) {
  // Build program
  const char *source = "constant int x = 0;";
  cl_int err;
  cl_program program =
      clCreateProgramWithSource(m_context, 1, &source, nullptr, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateProgramWithSource");
  err = clBuildProgram(program, 1, &m_device, m_options.c_str(), nullptr,
                       nullptr);
  ASSERT_NO_FATAL_FAILURE(CheckBuildError(program, err));

  // Get global variable pointer
  size_t gvSize;
  void *gvPtr;
  err = m_clGetDeviceGlobalVariablePointerINTEL(m_device, program, "x", &gvSize,
                                                &gvPtr);
  ASSERT_OCL_SUCCESS(err, "clGetDeviceGlobalVariablePointerINTEL");
  ASSERT_NE(nullptr, gvPtr)
      << "clGetDeviceGlobalVariablePointerINTEL must return a non-nullptr "
         "value via global_variable_pointer_ret on success";
  ASSERT_EQ(sizeof(cl_int), gvSize);

  // Test JIT save/load
  ASSERT_NO_FATAL_FAILURE(TestProgramWithBinary(program, "x", gvSize, "y"));
  err = clReleaseProgram(program);
  ASSERT_OCL_SUCCESS(err, "clReleaseProgram");
}

// Static global variable has internal linkage and should not be queryable by
// clGetDeviceGlobalVariablePointerINTEL.
TEST_P(GVPointerExtensionTest, singleProgramStaticGlobal) {
  // Build program
  const char *source = "static global int x = 0;\n"
                       "kernel void test(global int* dst) {\n"
                       "  if (get_global_id(0) == 0)\n"
                       "    *dst = x;\n"
                       "}";
  cl_int err;
  cl_program program =
      clCreateProgramWithSource(m_context, 1, &source, nullptr, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateProgramWithSource");
  err = clBuildProgram(program, 1, &m_device, m_options.c_str(), nullptr,
                       nullptr);
  ASSERT_NO_FATAL_FAILURE(CheckBuildError(program, err));

  // Get global variable pointer
  void *gvPtr;
  err = m_clGetDeviceGlobalVariablePointerINTEL(m_device, program, "x", nullptr,
                                                &gvPtr);
  EXPECT_EQ(CL_INVALID_ARG_VALUE, err)
      << "clGetDeviceGlobalVariablePointerINTEL must return "
         "CL_INVALID_ARG_VALUE if global_variable_name is static global";
  err = clReleaseProgram(program);
  EXPECT_OCL_SUCCESS(err, "clReleaseProgram");
}

// Test indirect access to global variable address queried by
// clGetDeviceGlobalVariablePointerINTEL.
TEST_P(GVPointerExtensionTest, singleProgramIndirectAccess) {
  cl_int err;

  cl_short x = (cl_short)(rand() & 255);
  std::ostringstream ss;
  ss << "typedef struct { short *a; } Ptrs;\n"
        "global short x = "
     << x
     << ";\n"
        "kernel void test(global Ptrs* dst) {\n"
        "  if (get_global_id(0) == 0)\n"
        "    *(dst->a) = x + 1;\n"
        "}";
  const std::string source = ss.str();
  const char *strings = source.c_str();

  // Build program
  cl_program program;
  program = clCreateProgramWithSource(m_context, 1, &strings, nullptr, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateProgramWithSource");
  err = clBuildProgram(program, 1, &m_device, m_options.c_str(), nullptr,
                       nullptr);
  ASSERT_NO_FATAL_FAILURE(CheckBuildError(program, err));

  // Get global variable pointer
  size_t gvSize;
  void *gvPtr;
  err = m_clGetDeviceGlobalVariablePointerINTEL(m_device, program, "x", &gvSize,
                                                &gvPtr);
  ASSERT_OCL_SUCCESS(err, "clGetDeviceGlobalVariablePointerINTEL");
  ASSERT_NE(nullptr, gvPtr)
      << "clGetDeviceGlobalVariablePointerINTEL must return a non-nullptr "
         "value via global_variable_pointer_ret on success";
  cl_short *gvPtrShort = static_cast<cl_short *>(gvPtr);
  ASSERT_EQ(sizeof(x), gvSize);
  ASSERT_EQ(x, *gvPtrShort);

  // Create kernel
  cl_kernel kernel = clCreateKernel(program, "test", &err);
  ASSERT_OCL_SUCCESS(err, "kernel");

  // clSetKernelExecInfo
  struct Ptrs {
    cl_short *a;
  };
  Ptrs *ptrs = (Ptrs *)clSharedMemAllocINTEL(m_context, m_device, NULL,
                                             sizeof(Ptrs), 0, &err);
  ASSERT_OCL_SUCCESS(err, "clSharedMemAllocINTEL");
  ptrs->a = gvPtrShort;
  err = clSetKernelArgMemPointerINTEL(kernel, 0, ptrs);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArgMemPointerINTEL");

  // According to current proposal, it is unnecessary to setKernelExecInfo if
  // kernel access pointer returned by clGetDeviceGlobalVariablePointerINTEL
  // from the same program that the kernel comes from.

  // Execute kernel
  size_t gdim = 1;
  size_t ldim = 1;
  err = clEnqueueNDRangeKernel(m_queue, kernel, 1, nullptr, &gdim, &ldim, 0,
                               nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueNDRangeKernel");
  err = clFinish(m_queue);
  ASSERT_OCL_SUCCESS(err, "clFinish");
  EXPECT_EQ(x + 1, *(ptrs->a));

  // Release resources
  err = clMemFreeINTEL(m_context, ptrs);
  EXPECT_OCL_SUCCESS(err, "clMemFreeINTEL");
  err = clReleaseKernel(kernel);
  EXPECT_OCL_SUCCESS(err, "clReleaseKernel");
  err = clReleaseProgram(program);
  EXPECT_OCL_SUCCESS(err, "clReleaseProgram");
}

// Test indirect access to global variable address queried by
// clGetDeviceGlobalVariablePointerINTEL from another program.
TEST_P(GVPointerExtensionTest, crossProgramsIndirectAccess) {
  cl_int err;

  // First kernel
  cl_long x = (cl_long)(rand() & 255);
  std::ostringstream ss;
  ss << "global long x = " << x
     << ";\n"
        "kernel void test1(global long* dst) {\n"
        "  if (get_global_id(0) == 0)\n"
        "    *dst = x;\n"
        "}";
  std::string source = ss.str();
  const char *strings = source.c_str();

  // Build first program
  cl_program program1;
  program1 = clCreateProgramWithSource(m_context, 1, &strings, nullptr, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateProgramWithSource");
  err = clBuildProgram(program1, 1, &m_device, m_options.c_str(), nullptr,
                       nullptr);
  ASSERT_NO_FATAL_FAILURE(CheckBuildError(program1, err));

  // Get global variable pointer from the first program
  size_t gvSize;
  void *gvPtr;
  err = m_clGetDeviceGlobalVariablePointerINTEL(m_device, program1, "x",
                                                &gvSize, &gvPtr);
  ASSERT_OCL_SUCCESS(err, "clGetDeviceGlobalVariablePointerINTEL");
  ASSERT_NE(nullptr, gvPtr)
      << "clGetDeviceGlobalVariablePointerINTEL must return a non-nullptr "
         "value via global_variable_pointer_ret on success";
  cl_long *gvPtrShort = static_cast<cl_long *>(gvPtr);
  ASSERT_EQ(sizeof(x), gvSize);
  ASSERT_EQ(x, *gvPtrShort);

  // Second kernel
  ss.clear();
  ss << "typedef struct { long *a; } Ptrs;\n"
     << "kernel void test2(global Ptrs* dst) {\n"
        "  if (get_global_id(0) == 0)\n"
        "    *(dst->a) += 1;\n"
        "}";
  source = ss.str();
  strings = source.c_str();

  // Build second program
  cl_program program2;
  program2 = clCreateProgramWithSource(m_context, 1, &strings, nullptr, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateProgramWithSource");
  err = clBuildProgram(program2, 1, &m_device, m_options.c_str(), nullptr,
                       nullptr);
  ASSERT_NO_FATAL_FAILURE(CheckBuildError(program2, err));

  // Create kernel from the second program
  cl_kernel kernel = clCreateKernel(program2, "test2", &err);
  ASSERT_OCL_SUCCESS(err, "kernel");

  // clSetKernelExecInfo
  struct Ptrs {
    cl_long *a;
  };
  Ptrs *ptrs = (Ptrs *)clSharedMemAllocINTEL(m_context, m_device, NULL,
                                             sizeof(Ptrs), 0, &err);
  ASSERT_OCL_SUCCESS(err, "clSharedMemAllocINTEL");
  ptrs->a = gvPtrShort;
  err = clSetKernelArgMemPointerINTEL(kernel, 0, ptrs);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArgMemPointerINTEL");
  err = clSetKernelExecInfo(kernel, CL_KERNEL_EXEC_INFO_USM_PTRS_INTEL,
                            sizeof(Ptrs), ptrs);
  ASSERT_OCL_SUCCESS(err, "clSetKernelExecInfo");
  // ptrs->a is device USM
  cl_bool useIndirectDevice = CL_TRUE;
  err = clSetKernelExecInfo(kernel,
                            CL_KERNEL_EXEC_INFO_INDIRECT_DEVICE_ACCESS_INTEL,
                            sizeof(useIndirectDevice), &useIndirectDevice);
  ASSERT_OCL_SUCCESS(err, "clSetKernelExecInfo");

  // Execute kernel
  size_t gdim = 1;
  size_t ldim = 1;
  err = clEnqueueNDRangeKernel(m_queue, kernel, 1, nullptr, &gdim, &ldim, 0,
                               nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueNDRangeKernel");
  err = clFinish(m_queue);
  ASSERT_OCL_SUCCESS(err, "clFinish");
  EXPECT_EQ(x + 1, *(ptrs->a));

  // Release resources
  err = clMemFreeINTEL(m_context, ptrs);
  EXPECT_OCL_SUCCESS(err, "clMemFreeINTEL");
  err = clReleaseKernel(kernel);
  EXPECT_OCL_SUCCESS(err, "clReleaseKernel");
  err = clReleaseProgram(program1);
  EXPECT_OCL_SUCCESS(err, "clReleaseProgram");
  err = clReleaseProgram(program2);
  EXPECT_OCL_SUCCESS(err, "clReleaseProgram");
}

// Local arrays are global variables with private linkage and should not be
// queryable by clGetDeviceGlobalVariablePointerINTEL.
TEST_P(GVPointerExtensionTest, singleProgramLocalArray) {
  // Build program
  const char *source = "kernel void test(global int* p) {\n"
                       "  int dim2Arr[2] = {1, 2};\n"
                       "}";
  cl_int err;
  cl_program program =
      clCreateProgramWithSource(m_context, 1, &source, nullptr, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateProgramWithSource");
  err = clBuildProgram(program, 1, &m_device, m_options.c_str(), nullptr,
                       nullptr);
  ASSERT_NO_FATAL_FAILURE(CheckBuildError(program, err));

  // Get global variable pointer
  void *gvPtr;
  err = m_clGetDeviceGlobalVariablePointerINTEL(m_device, program, "dim2Arr",
                                                nullptr, &gvPtr);
  EXPECT_EQ(CL_INVALID_ARG_VALUE, err)
      << "clGetDeviceGlobalVariablePointerINTEL must return "
         "CL_INVALID_ARG_VALUE if global_variable_name is local array";
  err = clReleaseProgram(program);
  EXPECT_OCL_SUCCESS(err, "clReleaseProgram");
}

INSTANTIATE_TEST_SUITE_P(GV, GVPointerExtensionTest, ::testing::Bool());
