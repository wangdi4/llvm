//==--- openmp_test_type.cpp - Tests for OpenMP support -*- C++ -*----------==//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include <cl_env.h>
#include <common_utils.h>
#include <options.hpp>

#include <CL/cl.h>
#include <gtest/gtest.h>

#include <string>
#include <vector>

cl_device_type gDeviceType = CL_DEVICE_TYPE_CPU;

class OmpTestType : public ::testing::Test {
protected:
  OmpTestType() : m_platform(nullptr),
                  m_device(nullptr),
                  m_context(nullptr),
                  m_queue(nullptr),
                  m_kernel_error(0),
                  m_mem_kernel_error(nullptr)
  {
  }

  virtual void SetUp();
  virtual void TearDown();

  void runTest(const char* name);

  cl_platform_id   m_platform;
  cl_device_id     m_device;
  cl_context       m_context;
  cl_command_queue m_queue;
  cl_int           m_kernel_error;
  cl_mem           m_mem_kernel_error;
};

void OmpTestType::SetUp() {
  cl_int error = CL_SUCCESS;

  cl_uint numPlatforms;
  error = clGetPlatformIDs(0, NULL, &numPlatforms);
  ASSERT_EQ(CL_SUCCESS, error) << "clGetPlatformIDs failed";
  ASSERT_GE(1u, numPlatforms) << "clGetPlatformIDs failed";

  error = clGetPlatformIDs(numPlatforms, &m_platform, NULL);
  ASSERT_EQ(CL_SUCCESS, error) << "clGetPlatformIDs failed";

  cl_uint numDevices;
  error = clGetDeviceIDs(m_platform, gDeviceType, 0, NULL, &numDevices);
  ASSERT_EQ(CL_SUCCESS, error) << "clGetDeviceIDs failed";
  ASSERT_GE(1u, numPlatforms) << "clGetPlatformIDs failed";

  error = clGetDeviceIDs(m_platform, gDeviceType, numDevices, &m_device, NULL);
  ASSERT_EQ(CL_SUCCESS, error) << "clGetDeviceIDs failed";

  m_context = clCreateContext(NULL, 1, &m_device, NULL, NULL, &error);
  ASSERT_EQ(CL_SUCCESS, error) << "clCreateContext failed";

  m_queue = clCreateCommandQueueWithProperties(
      m_context, m_device, NULL, &error);
  ASSERT_EQ(CL_SUCCESS, error) << "clCreateCommandQueueWithProperties failed";

  m_mem_kernel_error = clCreateBuffer(
      m_context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
      sizeof(m_kernel_error), &m_kernel_error, &error);
  ASSERT_EQ(CL_SUCCESS, error) << "clCreateBuffer failed";
}

void OmpTestType::TearDown() {
  if (m_mem_kernel_error) clReleaseMemObject(m_mem_kernel_error);
  if (m_queue)            clReleaseCommandQueue(m_queue);
  if (m_context)          clReleaseContext(m_context);
  if (m_device)           clReleaseDevice(m_device);
}

void OmpTestType::runTest(const char* name) {
  cl_int error = CL_SUCCESS;

  std::string programSrc = "#include \"";
  programSrc += get_exe_dir();
  programSrc += "/device/";
  programSrc += name;
  programSrc += ".cl\"";

  const char* src = programSrc.c_str();
  cl_program program = clCreateProgramWithSource(
      m_context, 1, &src, NULL, &error);
  ASSERT_EQ(CL_SUCCESS, error) << "clCreateProgramWithSource failed";

  const char* options = "";
#ifdef DEBUG
  options = "-DDEBUG"
#endif

  error = clBuildProgram(program, 0, NULL, options, NULL, NULL);

  if (error != CL_SUCCESS) {
    size_t logSize;
    clGetProgramBuildInfo(program, m_device,
                          CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);
    std::vector<char> log(logSize);
    clGetProgramBuildInfo(program, m_device,
                          CL_PROGRAM_BUILD_LOG, logSize, &log[0], NULL);
    ASSERT_EQ(error, CL_SUCCESS) << "\nBuild log:\n" << &log[0];
  }

  cl_kernel kernel = clCreateKernel(program, name, &error);
  ASSERT_EQ(CL_SUCCESS, error) << "clCreateKernel failed";

  error = clSetKernelArg(kernel, 0, sizeof(m_kernel_error), &m_mem_kernel_error);
  ASSERT_EQ(CL_SUCCESS, error) << "clSetKernelArg failed";

  size_t globalSize = 1;
  size_t localSize  = 1;

  error = clEnqueueNDRangeKernel(
      m_queue, kernel, 1, NULL, &globalSize, &localSize, 0, NULL, NULL);
  ASSERT_EQ(CL_SUCCESS, error) << "clEnqueueNDRangeKernel failed";

  error = clFinish(m_queue);
  ASSERT_EQ(CL_SUCCESS, error) << "clFinish failed";

  ASSERT_EQ(0, m_kernel_error) <<
    "Kernel execution failed: non zero exit code";

  clReleaseKernel(kernel);
  clReleaseProgram(program);
}

CommandLineOption<std::string> deviceOption("--device_type");

int main(int argc, char **argv) {

  // FIXME:
  // We pass -fintel-compatibility here to bypass OpenMP pragma
  // resolution in the frontend. They will be passed down to the VPO
  // vectorizer and resolved there. This option should not be enabled by
  // default, because in addition to OpenMP bypass it also does several
  // icc compatibility changes in Sema. When enabled by default this can
  // break existing OpenCL code.
  //
  // Discussion to change the option or add a new one is ongoing. Until
  // then, -fintel-compatibility would be enabled here and it would only
  // affect openmp_test_type suite.
  if (!getenv("VOLCANO_CLANG_OPTIONS")) {
    SETENV("VOLCANO_CLANG_OPTIONS",
           "-fopenmp -fintel-openmp -fopenmp-tbb -fintel-compatibility");
  }

  std::map<std::string, cl_device_type> clDeviceTypeMap;
  clDeviceTypeMap["cpu"] = CL_DEVICE_TYPE_CPU;
  clDeviceTypeMap["fpga_fast_emu"] = CL_DEVICE_TYPE_ACCELERATOR;
  clDeviceTypeMap["default"] = CL_DEVICE_TYPE_DEFAULT;
  clDeviceTypeMap["all"] = CL_DEVICE_TYPE_ALL;
  ::testing::InitGoogleTest(&argc, argv);
  std::string deviceTypeStr;
  if (argc > 1) {
    for (int i = 1 ; i < argc ; i++)
      if (deviceOption.isMatch(argv[i])) {
        deviceTypeStr = deviceOption.getValue(argv[i]);
        auto iter = clDeviceTypeMap.find(deviceTypeStr);
        if (iter == clDeviceTypeMap.end()) {
            printf("error: unkown device option: %s\n", deviceTypeStr.c_str());
            return 1;
        }
        gDeviceType = iter->second;
      }
  }

  if (GetEnv(deviceTypeStr, "CL_DEVICE_TYPE")) {
    std::map<std::string, cl_device_type>::iterator iter =
        clDeviceTypeMap.find(deviceTypeStr);
    if (iter == clDeviceTypeMap.end()) {
      printf("error: unkown value of CL_DEVICE_TYPE env variable: %s\n",
          deviceTypeStr.c_str());
      return 1;
    }
    gDeviceType = iter->second;
  }

  return RUN_ALL_TESTS();
}

#define OMP_TEST(name) TEST_F(OmpTestType, name) {      \
  runTest(#name);                                       \
}

OMP_TEST(atomic_capture_post_1stmt_par_scalar_mul)
OMP_TEST(atomic_capture_post_1stmt_par_scalar_ptr_mul)
OMP_TEST(atomic_capture_post_par_scalar_mul)
OMP_TEST(atomic_capture_post_par_scalar_ptr_mul)
OMP_TEST(atomic_capture_pre_par_scalar_div)
OMP_TEST(atomic_capture_pre_par_scalar_ptr_div)
OMP_TEST(atomic_capture_swap_par_array)
OMP_TEST(atomic_capture_swap_par_array_ptr)
OMP_TEST(atomic_read_par_scalar)
OMP_TEST(atomic_read_par_scalar_ptr)
OMP_TEST(atomic_update_par_scalar_add)
OMP_TEST(atomic_update_par_scalar_add_noop)
OMP_TEST(atomic_update_par_scalar_mul)
OMP_TEST(atomic_update_par_scalar_ptr_add)
OMP_TEST(atomic_update_par_scalar_ptr_mul)
OMP_TEST(atomic_write_par_scalar)
OMP_TEST(atomic_write_par_scalar_ptr)
OMP_TEST(critical_par_for_array)
OMP_TEST(critical_par_scalar)
OMP_TEST(for_firstprivate)
OMP_TEST(for_lastprivate)
OMP_TEST(for_private)
OMP_TEST(for_reduction)
OMP_TEST(for_reduction_array)
OMP_TEST(loop_trip_test_br)
OMP_TEST(loop_trip_test_gt)
OMP_TEST(loop_trip_test_gt_stride2)
OMP_TEST(loop_trip_test_gte)
OMP_TEST(loop_trip_test_gte_stride2)
OMP_TEST(loop_trip_test_lt)
OMP_TEST(loop_trip_test_lt_stride2)
OMP_TEST(loop_trip_test_lte)
OMP_TEST(loop_trip_test_lte_stride2)
OMP_TEST(par_firstprivate)
OMP_TEST(par_for_firstprivate)
OMP_TEST(par_for_lastprivate)
OMP_TEST(par_for_private)
OMP_TEST(par_for_reduction)
OMP_TEST(par_for_reduction_array)
OMP_TEST(par_for_shared)
OMP_TEST(par_master)
OMP_TEST(par_nested)
OMP_TEST(par_region_atomic)
OMP_TEST(par_region_reduction)
OMP_TEST(par_simd_matmul)
OMP_TEST(par_single)
OMP_TEST(simd_condlastprivate)
OMP_TEST(simd_func_linear)
OMP_TEST(simd_func_simdlen)
OMP_TEST(simd_func_svml_sin)
OMP_TEST(simd_func_uniform)
OMP_TEST(simd_func_vector)
OMP_TEST(simd_lastprivate)
OMP_TEST(simd_max_reduction)
OMP_TEST(simd_min_reduction)
OMP_TEST(simd_private)
OMP_TEST(simd_simdlen)
OMP_TEST(simd_sum_reduction)
OMP_TEST(simd_sum_reduction2)
OMP_TEST(simd_test)
OMP_TEST(taskloop_firstprivate)
OMP_TEST(taskloop_nested_reduction_atomic)
OMP_TEST(taskloop_private)
OMP_TEST(taskloop_reduction1)
OMP_TEST(taskloop_shared)
