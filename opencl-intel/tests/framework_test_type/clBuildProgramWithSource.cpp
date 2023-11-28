// INTEL CONFIDENTIAL
//
// Copyright 2012 Intel Corporation.
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

#include "CL21.h"
#include "TestsHelpClasses.h"

class BuildProgramWithSourceTest : public CL21 {};

/*******************************************************************************
 * clBuildProgram
 * -------------------
 * (1) get device ids
 * (2) create context
 * (3) create binary
 * (4) create program with source
 * (5) build program
 ******************************************************************************/
TEST_F(BuildProgramWithSourceTest, basic) {
  const char *strings[] = {
      "__kernel void test_kernel(__global char16* pBuff0, __global char* "
      "pBuff1, __global char* pBuff2, image2d_t __read_only test_image)"
      "{"
      "    size_t id = get_global_id(0);"
      "    pBuff0[id] = pBuff1[id] ? pBuff0[id] : pBuff2[id];"
      "}"};

  // check if all devices support images
  cl_bool isImagesSupported;
  cl_int err = clGetDeviceInfo(m_device, CL_DEVICE_IMAGE_SUPPORT,
                               sizeof(cl_bool), &isImagesSupported, nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo(CL_DEVICE_IMAGE_SUPPORT)");
  ASSERT_TRUE(isImagesSupported) << "image is not supported on the device";

  cl_program program;
  bool ret = BuildProgramSynch(m_context, 1, (const char **)&strings, nullptr,
                               "-cl-denorms-are-zero", &program);
  ASSERT_TRUE(ret) << "BuildProgramSynch failed";

  // get the binary
  size_t binarySize;
  err = clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES, sizeof(size_t),
                         &binarySize, nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetProgramInfo(CL_PROGRAM_BINARY_SIZES)");
  char *binaries = new char[binarySize];
  err = clGetProgramInfo(program, CL_PROGRAM_BINARIES, sizeof(binaries),
                         &binaries, nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetProgramInfo(CL_PROGRAM_BINARIES)");
  delete[] binaries;

  cl_kernel kern = clCreateKernel(program, "test_kernel", &err);
  ASSERT_OCL_SUCCESS(err, "clCreateKernel");
  err = clSetKernelArg(kern, 2, sizeof(cl_mem), nullptr);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArg()");
  err = clSetKernelArg(kern, 3, sizeof(cl_mem), nullptr);
  ASSERT_OCL_EQ(err, CL_INVALID_ARG_VALUE, "clSetKernelArg(C)");

  // Release objects
  err = clReleaseKernel(kern);
  ASSERT_OCL_SUCCESS(err, "clReleaseKernel");
  err = clReleaseProgram(program);
  ASSERT_OCL_SUCCESS(err, "clReleaseProgram");
}

/// This test checks clGetProgramInfo returns valid error code.
TEST_F(BuildProgramWithSourceTest, buildFail) {
  const char *strings[] = {"kernel"};
  cl_int err;
  cl_program program =
      clCreateProgramWithSource(m_context, 1, strings, nullptr, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateProgramWithSource");
  err = clBuildProgram(program, 1, &m_device, nullptr, nullptr, nullptr);
  ASSERT_OCL_EQ(err, CL_BUILD_PROGRAM_FAILURE, "clBuildProgram");

  // get the binary
  size_t binarySize;
  err = clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES, sizeof(size_t),
                         &binarySize, nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetProgramInfo(CL_PROGRAM_BINARY_SIZES)");
  ASSERT_OCL_EQ(binarySize, 0, "clGetProgramInfo(CL_PROGRAM_BINARY_SIZES)");
  char *binaries = new char[binarySize + 1];
  err = clGetProgramInfo(program, CL_PROGRAM_BINARIES, sizeof(binaries),
                         &binaries, nullptr);
  ASSERT_OCL_EQ(err, CL_INVALID_PROGRAM,
                "clGetProgramInfo(CL_PROGRAM_BINARIES)");
  delete[] binaries;

  // Release objects
  err = clReleaseProgram(program);
  ASSERT_OCL_SUCCESS(err, "clReleaseProgram");
}
