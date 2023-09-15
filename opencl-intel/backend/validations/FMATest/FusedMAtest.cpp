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

//|
//| TEST: FMA test (fused mul add test)
//|
//| Purpose
//| -------
//|
//| Test the fused multiply ADD instruction
//|
//| Method
//| ------
//|
//| 1. Create an OpenCL program that uses fma instruction
//| 2. Calculate the result and the supposed output
//| 2. Compare the result from the direct calc and from the OpenCL calc
//|
//| -------------
//|
//| Return true in case of SUCCESS.

// Includes:
#include "CL/cl.h"
#include "gtest_wrapper.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>

// Define:
#define SCALAR 1
#define VEC2 2
#define VEC4 4
#define VEC8 8
#define NUM_OF_VALUES                                                          \
  8 // Quantity of values that we review and therefore the size of the array
#define D1 (1.234567)
#define D2 (1.234567e+8)
#define D3 (1.234567e+17)
#define D4 (3.14159265)
#define F1 (1.234567e+17f)
#define F2 (1.234f)
#define F3 (1.234567e+8f)
#define F4 (3.14159265f)
#define FLOAT_ULP 0.1f
#define DOUBLE_ULP 0.1

// Calc the result on OpenCL with the src we got:
template <class T>
bool opencl_mul_add_calc(T *a, T *b, T *c, T *results,
                         std::string *program_source, int vec_size) {
  cl_program program[1];
  cl_kernel kernel[1];
  cl_platform_id platform;
  cl_command_queue cmd_queue;
  cl_context context = NULL;
  cl_device_id device = NULL;
  cl_int err = 0;
  size_t buffer_size;
  int type_size = sizeof(T);

  // Device
  if (clGetPlatformIDs(1, &platform, NULL) != CL_SUCCESS) {
    return false;
  }
  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &device, NULL);
  if (err)
    return false;
  cl_mem a_mem, b_mem, c_mem, r_mem;

  // Context
  cl_context_properties properties[3] = {CL_CONTEXT_PLATFORM,
                                         (cl_context_properties)platform, 0};
  context =
      clCreateContextFromType(properties, CL_DEVICE_TYPE_CPU, NULL, NULL, &err);
  if (err)
    return false;

  // Command queue
  cmd_queue = clCreateCommandQueueWithProperties(context, device, 0, &err);
  if (err)
    return false;
  // Program
  const char *prog = program_source->c_str();
  size_t siz = program_source->size() + 1;
  program[0] = clCreateProgramWithSource(context, 1, &prog, &siz, &err);
  if (err)
    return false;
  if (clBuildProgram(program[0], 0, NULL, NULL, NULL, NULL) != CL_SUCCESS)
    return false;

  // Kernel
  kernel[0] = clCreateKernel(program[0], "mul_add", &err);
  if (err)
    return false;
  // Mem buffers
  buffer_size = type_size * NUM_OF_VALUES;
  // Array a
  a_mem = clCreateBuffer(context, CL_MEM_READ_ONLY, buffer_size, NULL, NULL);
  err = clEnqueueWriteBuffer(cmd_queue, a_mem, CL_TRUE, 0, buffer_size,
                             (void *)a, 0, NULL, NULL);
  // Array b
  b_mem = clCreateBuffer(context, CL_MEM_READ_ONLY, buffer_size, NULL, NULL);
  err |= clEnqueueWriteBuffer(cmd_queue, b_mem, CL_TRUE, 0, buffer_size,
                              (void *)b, 0, NULL, NULL);
  // Array c
  c_mem = clCreateBuffer(context, CL_MEM_READ_ONLY, buffer_size, NULL, NULL);
  err |= clEnqueueWriteBuffer(cmd_queue, c_mem, CL_TRUE, 0, buffer_size,
                              (void *)c, 0, NULL, NULL);
  // Results array
  r_mem = clCreateBuffer(context, CL_MEM_READ_WRITE, buffer_size, NULL, NULL);
  err |= clEnqueueWriteBuffer(cmd_queue, r_mem, CL_TRUE, 0, buffer_size,
                              (void *)results, 0, NULL, NULL);
  if (err)
    return false;
  clFinish(cmd_queue);

  // Arguments
  err = clSetKernelArg(kernel[0], 0, sizeof(cl_mem), &a_mem);
  err |= clSetKernelArg(kernel[0], 1, sizeof(cl_mem), &b_mem);
  err |= clSetKernelArg(kernel[0], 2, sizeof(cl_mem), &c_mem);
  err |= clSetKernelArg(kernel[0], 3, sizeof(cl_mem), &r_mem);
  if (err)
    return false;
  size_t global_work_size = NUM_OF_VALUES / vec_size;
  err = clEnqueueNDRangeKernel(cmd_queue, kernel[0], 1, NULL, &global_work_size,
                               NULL, 0, NULL, NULL);
  if (err)
    return false;
  clFinish(cmd_queue);
  err = clEnqueueReadBuffer(cmd_queue, r_mem, CL_TRUE, 0, buffer_size, results,
                            0, NULL, NULL);
  if (err)
    return false;
  clFinish(cmd_queue);

  // exe
  err = clEnqueueNDRangeKernel(cmd_queue, kernel[0], 1, NULL, &global_work_size,
                               NULL, 0, NULL, NULL);
  if (err)
    return false;
  clFinish(cmd_queue);

  // Release
  clReleaseMemObject(a_mem);
  clReleaseMemObject(b_mem);
  clReleaseMemObject(c_mem);
  clReleaseMemObject(r_mem);
  clReleaseCommandQueue(cmd_queue);
  clReleaseContext(context);
  return true;
}

bool testDouble(std::string src, int vec_size, int i, int k) {
  double ropencl[NUM_OF_VALUES];
  double a[NUM_OF_VALUES] = {0, D1, D1, D1, D3, D3, D2, D2};
  double b[NUM_OF_VALUES] = {0, D4, D1, D1, D3, D3, D2, D2};
  double c[NUM_OF_VALUES] = {D1, D1, D2, D3, 0, D2, D3, D1};
  bool flag = true;
  double res[NUM_OF_VALUES];
  // calc the result
  for (int j = 0; j < NUM_OF_VALUES; ++j)
    res[j] = i * a[j] * b[j] + k * c[j];
  // calc the OpenCL result
  if (opencl_mul_add_calc(a, b, c, ropencl, &src, vec_size) != true) {
    std::cout << "OpenCL FAILED " << std::endl;
    return false;
  }
  // Compare calculation
  for (int j = 0; j < NUM_OF_VALUES; j++) {
    if ((ropencl[j] < (res[j] - DOUBLE_ULP)) ||
        (ropencl[j] > (res[j] + DOUBLE_ULP))) {
      std::cout << "FAILED: OpenCL calculation was not as expected ( " << j
                << " )" << std::endl;
      flag = false;
    }
  }
  return flag;
}
bool testFloat(std::string src, int vec_size, int i, int k) {
  float ropencl[NUM_OF_VALUES];
  float a[NUM_OF_VALUES] = {0, F1, F1, F1, F3, F3, F2, F2};
  float b[NUM_OF_VALUES] = {0, F4, F1, F1, F3, F3, F2, F2};
  float c[NUM_OF_VALUES] = {F1, F1, F2, F3, 0, F2, F3, F1};
  bool flag = true;
  float res[NUM_OF_VALUES];
  // calc the result
  for (int j = 0; j < NUM_OF_VALUES; ++j)
    res[j] = i * a[j] * b[j] + k * c[j];
  // calc the OpenCL result
  if (opencl_mul_add_calc(a, b, c, ropencl, &src, vec_size) != true) {
    std::cout << "OpenCL FAILED " << std::endl;
    return false;
  }
  // Compare calculation
  for (int j = 0; j < NUM_OF_VALUES; j++) {
    if ((ropencl[j] < (res[j] - DOUBLE_ULP)) ||
        (ropencl[j] > (res[j] + DOUBLE_ULP))) {
      std::cout << "FAILED: OpenCL calculation was not as expected ( " << j
                << " )" << std::endl;
      flag = false;
    }
  }
  return flag;
}

// the tests: A=a*b+c B=a*b-c C=-a*b+c D=-a*b-c

bool FMAtest_scalar_floatA() {
  std::string test_program_scalar_float =
      "__kernel void mul_add (__global float *a,  __global float *b, __global "
      "float *c, __global float *result) { int gid = get_global_id(0); "
      "result[gid] = (a[gid] * b[gid])+c[gid];}";
  return (testFloat(test_program_scalar_float, SCALAR, 1, 1));
}
bool FMAtest_scalar_floatB() {
  std::string test_program_scalar_float =
      "__kernel void mul_add (__global float *a,  __global float *b, __global "
      "float *c, __global float *result) { int gid = get_global_id(0); "
      "result[gid] = (a[gid] * b[gid])-c[gid];}";
  return (testFloat(test_program_scalar_float, SCALAR, 1, -1));
}
bool FMAtest_scalar_floatC() {
  std::string test_program_scalar_float =
      "__kernel void mul_add (__global float *a,  __global float *b, __global "
      "float *c, __global float *result) { int gid = get_global_id(0); "
      "result[gid] = -(a[gid] * b[gid])+c[gid];}";
  return (testFloat(test_program_scalar_float, SCALAR, -1, 1));
}
bool FMAtest_scalar_floatD() {
  std::string test_program_scalar_float =
      "__kernel void mul_add (__global float *a,  __global float *b, __global "
      "float *c, __global float *result) { int gid = get_global_id(0); "
      "result[gid] = -(a[gid] * b[gid])-c[gid];}";
  return (testFloat(test_program_scalar_float, SCALAR, -1, -1));
}
bool FMAtest_scalar_doubleA() {
  std::string test_program_scalar_double =
      "__kernel void mul_add (__global double *a,  __global double *b, "
      "__global double *c, __global double *result) { int gid = "
      "get_global_id(0); result[gid] = (a[gid] * b[gid])+c[gid];}";
  return (testDouble(test_program_scalar_double, SCALAR, 1, 1));
}
bool FMAtest_scalar_doubleB() {
  std::string test_program_scalar_double =
      "__kernel void mul_add (__global double *a,  __global double *b, "
      "__global double *c, __global double *result) { int gid = "
      "get_global_id(0); result[gid] = (a[gid] * b[gid])-c[gid];}";
  return (testDouble(test_program_scalar_double, SCALAR, 1, -1));
}
bool FMAtest_scalar_doubleC() {
  std::string test_program_scalar_double =
      "__kernel void mul_add (__global double *a,  __global double *b, "
      "__global double *c, __global double *result) { int gid = "
      "get_global_id(0); result[gid] = -(a[gid] * b[gid])+c[gid];}";
  return (testDouble(test_program_scalar_double, SCALAR, -1, 1));
}
bool FMAtest_scalar_doubleD() {
  std::string test_program_scalar_double =
      "__kernel void mul_add (__global double *a,  __global double *b, "
      "__global double *c, __global double *result) { int gid = "
      "get_global_id(0); result[gid] = -(a[gid] * b[gid])-c[gid];}";
  return (testDouble(test_program_scalar_double, SCALAR, -1, -1));
}

bool FMAtest_v4_floatA() {
  std::string test_program_v4_float =
      "__kernel void mul_add (__global float4 *a,  __global float4 *b, "
      "__global float4 *c, __global float4 *result) { int gid = "
      "get_global_id(0); result[gid] = (a[gid] * b[gid])+c[gid];}";
  return (testFloat(test_program_v4_float, VEC4, 1, 1));
}
bool FMAtest_v4_floatB() {
  std::string test_program_v4_float =
      "__kernel void mul_add (__global float4 *a,  __global float4 *b, "
      "__global float4 *c, __global float4 *result) { int gid = "
      "get_global_id(0); result[gid] = (a[gid] * b[gid])-c[gid];}";
  return (testFloat(test_program_v4_float, VEC4, 1, -1));
}
bool FMAtest_v4_floatC() {
  std::string test_program_v4_float =
      "__kernel void mul_add (__global float4 *a,  __global float4 *b, "
      "__global float4 *c, __global float4 *result) { int gid = "
      "get_global_id(0); result[gid] = -(a[gid] * b[gid])+c[gid];}";
  return (testFloat(test_program_v4_float, VEC4, -1, 1));
}
bool FMAtest_v4_floatD() {
  std::string test_program_v4_float =
      "__kernel void mul_add (__global float4 *a,  __global float4 *b, "
      "__global float4 *c, __global float4 *result) { int gid = "
      "get_global_id(0); result[gid] = -(a[gid] * b[gid])-c[gid];}";
  return (testFloat(test_program_v4_float, VEC4, -1, -1));
}
bool FMAtest_v2_doubleA() {
  std::string test_program_v2_double =
      "__kernel void mul_add (__global double2 *a,  __global double2 *b, "
      "__global double2 *c, __global double2 *result) { int gid = "
      "get_global_id(0); result[gid] = (a[gid] * b[gid])+c[gid];}";
  return (testDouble(test_program_v2_double, VEC2, 1, 1));
}
bool FMAtest_v2_doubleB() {
  std::string test_program_v2_double =
      "__kernel void mul_add (__global double2 *a,  __global double2 *b, "
      "__global double2 *c, __global double2 *result) { int gid = "
      "get_global_id(0); result[gid] = (a[gid] * b[gid])-c[gid];}";
  return (testDouble(test_program_v2_double, VEC2, 1, -1));
}
bool FMAtest_v2_doubleC() {
  std::string test_program_v2_double =
      "__kernel void mul_add (__global double2 *a,  __global double2 *b, "
      "__global double2 *c, __global double2 *result) { int gid = "
      "get_global_id(0); result[gid] = -(a[gid] * b[gid])+c[gid];}";
  return (testDouble(test_program_v2_double, VEC2, -1, 1));
}
bool FMAtest_v2_doubleD() {
  std::string test_program_v2_double =
      "__kernel void mul_add (__global double2 *a,  __global double2 *b, "
      "__global double2 *c, __global double2 *result) { int gid = "
      "get_global_id(0); result[gid] = -(a[gid] * b[gid])-c[gid];}";
  return (testDouble(test_program_v2_double, VEC2, -1, -1));
}
bool FMAtest_v8_floatA() {
  std::string test_program_v8_float =
      "__kernel void mul_add (__global float8 *a,  __global float8 *b, "
      "__global float8 *c, __global float8 *result) { int gid = "
      "get_global_id(0); result[gid] = (a[gid] * b[gid])+c[gid];}";
  return (testFloat(test_program_v8_float, VEC8, 1, 1));
}
bool FMAtest_v8_floatB() {
  std::string test_program_v8_float =
      "__kernel void mul_add (__global float8 *a,  __global float8 *b, "
      "__global float8 *c, __global float8 *result) { int gid = "
      "get_global_id(0); result[gid] = (a[gid] * b[gid])-c[gid];}";
  return (testFloat(test_program_v8_float, VEC8, 1, -1));
}
bool FMAtest_v8_floatC() {
  std::string test_program_v8_float =
      "__kernel void mul_add (__global float8 *a,  __global float8 *b, "
      "__global float8 *c, __global float8 *result) { int gid = "
      "get_global_id(0); result[gid] = -(a[gid] * b[gid])+c[gid];}";
  return (testFloat(test_program_v8_float, VEC8, -1, 1));
}
bool FMAtest_v8_floatD() {
  std::string test_program_v8_float =
      "__kernel void mul_add (__global float8 *a,  __global float8 *b, "
      "__global float8 *c, __global float8 *result) { int gid = "
      "get_global_id(0); result[gid] = -(a[gid] * b[gid])-c[gid];}";
  return (testFloat(test_program_v8_float, VEC8, -1, -1));
}
bool FMAtest_v4_doubleA() {
  std::string test_program_v4_double =
      "__kernel void mul_add (__global double4 *a,  __global double4 *b, "
      "__global double4 *c, __global double4 *result) { int gid = "
      "get_global_id(0); result[gid] = (a[gid] * b[gid])+c[gid];}";
  return (testDouble(test_program_v4_double, VEC4, 1, 1));
}
bool FMAtest_v4_doubleB() {
  std::string test_program_v4_double =
      "__kernel void mul_add (__global double4 *a,  __global double4 *b, "
      "__global double4 *c, __global double4 *result) { int gid = "
      "get_global_id(0); result[gid] = (a[gid] * b[gid])-c[gid];}";
  return (testDouble(test_program_v4_double, VEC4, 1, -1));
}
bool FMAtest_v4_doubleC() {
  std::string test_program_v4_double =
      "__kernel void mul_add (__global double4 *a,  __global double4 *b, "
      "__global double4 *c, __global double4 *result) { int gid = "
      "get_global_id(0); result[gid] = -(a[gid] * b[gid])+c[gid];}";
  return (testDouble(test_program_v4_double, VEC4, -1, 1));
}
bool FMAtest_v4_doubleD() {
  std::string test_program_v4_double =
      "__kernel void mul_add (__global double4 *a,  __global double4 *b, "
      "__global double4 *c, __global double4 *result) { int gid = "
      "get_global_id(0); result[gid] = -(a[gid] * b[gid])-c[gid];}";
  return (testDouble(test_program_v4_double, VEC4, -1, -1));
}

// a*b+c
TEST(FMAtest_adds_add, FMAtest_double) {
  EXPECT_TRUE(FMAtest_scalar_doubleA());
}
TEST(FMAtest_adds_add, FMAtest_double2) { EXPECT_TRUE(FMAtest_v2_doubleA()); }
TEST(FMAtest_adds_add, FMAtest_double4) { EXPECT_TRUE(FMAtest_v4_doubleA()); }
TEST(FMAtest_adds_add, FMAtest_float) { EXPECT_TRUE(FMAtest_scalar_floatA()); }
TEST(FMAtest_adds_add, FMAtest_float4) { EXPECT_TRUE(FMAtest_v4_floatA()); }
TEST(FMAtest_adds_add, FMAtest_float8) { EXPECT_TRUE(FMAtest_v8_floatA()); }
// a*b-c
TEST(FMAtest_add_sub, FMAtest_double) { EXPECT_TRUE(FMAtest_scalar_doubleB()); }
TEST(FMAtest_add_sub, FMAtest_double2) { EXPECT_TRUE(FMAtest_v2_doubleB()); }
TEST(FMAtest_add_sub, FMAtest_double4) { EXPECT_TRUE(FMAtest_v4_doubleB()); }
TEST(FMAtest_add_sub, FMAtest_float) { EXPECT_TRUE(FMAtest_scalar_floatB()); }
TEST(FMAtest_add_sub, FMAtest_float4) { EXPECT_TRUE(FMAtest_v4_floatB()); }
TEST(FMAtest_add_sub, FMAtest_float8) { EXPECT_TRUE(FMAtest_v8_floatB()); }
//-a*b+c
TEST(FMAtest_sub_add, FMAtest_double) { EXPECT_TRUE(FMAtest_scalar_doubleC()); }
TEST(FMAtest_sub_add, FMAtest_double2) { EXPECT_TRUE(FMAtest_v2_doubleC()); }
TEST(FMAtest_sub_add, FMAtest_double4) { EXPECT_TRUE(FMAtest_v4_doubleC()); }
TEST(FMAtest_sub_add, FMAtest_float) { EXPECT_TRUE(FMAtest_scalar_floatC()); }
TEST(FMAtest_sub_add, FMAtest_float4) { EXPECT_TRUE(FMAtest_v4_floatC()); }
TEST(FMAtest_sub_add, FMAtest_float8) { EXPECT_TRUE(FMAtest_v8_floatC()); }
//-a*b-c
TEST(FMAtest_sub_sub, FMAtest_double) { EXPECT_TRUE(FMAtest_scalar_doubleD()); }
TEST(FMAtest_sub_sub, FMAtest_double2) { EXPECT_TRUE(FMAtest_v2_doubleD()); }
TEST(FMAtest_sub_sub, FMAtest_double4) { EXPECT_TRUE(FMAtest_v4_doubleD()); }
TEST(FMAtest_sub_sub, FMAtest_float) { EXPECT_TRUE(FMAtest_scalar_floatD()); }
TEST(FMAtest_sub_sub, FMAtest_float4) { EXPECT_TRUE(FMAtest_v4_floatD()); }
TEST(FMAtest_sub_sub, FMAtest_float8) { EXPECT_TRUE(FMAtest_v8_floatD()); }
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
