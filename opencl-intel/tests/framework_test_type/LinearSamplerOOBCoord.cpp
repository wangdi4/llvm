// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.

// The test checks that correct value is returned when we read 'pixel center'
// of a border pixel using linear filter.

#include "TestsHelpClasses.h"
#include "test_utils.h"

extern cl_device_type gDeviceType;

static const char *test_1d[] = {R"(
__kernel void image_test(
    __read_only image1d_t img , sampler_t samp, __global float4* buf) {
  size_t x = get_global_id(0);
  buf[x] = read_imagef(img, samp, (float)x + .5f);
})"};
static const char *test_2d[] = {R"(
__kernel void image_test(
    __read_only image2d_t img , sampler_t samp, __global float4* buf) {
  size_t x = get_global_id(0);
  size_t y = get_global_id(1);
  size_t width = get_image_width(img);
  size_t idx = width * y + x;
  buf[idx] = read_imagef(img, samp, (float2){x + .5f, y + .5f});
})"};
static const char *test_3d[] = {R"(
__kernel void image_test(
    __read_only image3d_t img , sampler_t samp, __global float4* buf) {
  size_t x = get_global_id(0);
  size_t y = get_global_id(1);
  size_t z = get_global_id(2);
  size_t width = get_image_width(img);
  size_t height = get_image_height(img);
  size_t idx = height * width * z + width * y + x;
  buf[idx] = read_imagef(img, samp, (float4){x + .5f, y + .5f, z + .5f, 0});
})"};

static void TestImage(cl_context context, cl_command_queue queue,
                      cl_sampler sampler, const cl_image_desc *img_desc) {
  cl_int err;

  size_t ByteSize = img_desc->image_row_pitch;
  if (size_t height = img_desc->image_height)
    ByteSize *= height;
  if (size_t depth = img_desc->image_depth)
    ByteSize *= depth;
  size_t ByteSizeF = ByteSize * sizeof(float);

  uint8_t *input = (uint8_t *)malloc(ByteSize);
  ASSERT_NE(input, nullptr);
  float *output = (float *)malloc(ByteSizeF);
  ASSERT_NE(output, nullptr);

  static const float val[] = {.1f, .3f, .5f, .7f, .9f};

  for (size_t i = 0; i < ByteSize; ++i)
    input[i] = 255 * val[i % 5];

  cl_image_format format = {CL_RGBA, CL_UNORM_INT8};

  int dim;
  const char **src;
  switch (img_desc->image_type) {
  case CL_MEM_OBJECT_IMAGE1D:
    dim = 1;
    src = test_1d;
    break;
  case CL_MEM_OBJECT_IMAGE2D:
    dim = 2;
    src = test_2d;
    break;
  case CL_MEM_OBJECT_IMAGE3D:
    dim = 3;
    src = test_3d;
    break;
  }

  cl_program program;
  err = BuildProgramSynch(context, 1, src, nullptr, nullptr, &program);
  ASSERT_TRUE(err) << "BuildProgramSynch failed";

  cl_mem clImg = clCreateImage(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                               &format, img_desc, input, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateImage");

  cl_mem outbuf =
      clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
                     ByteSizeF, output, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateBuffer");

  cl_kernel kernel = clCreateKernel(program, "image_test", &err);
  ASSERT_OCL_SUCCESS(err, "clCreateKernel");

  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &clImg);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArg(0)");
  err = clSetKernelArg(kernel, 1, sizeof(cl_sampler), &sampler);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArg(1)");
  err = clSetKernelArg(kernel, 2, sizeof(cl_mem), &outbuf);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArg(2)");

  size_t gsize[] = {img_desc->image_width,
                    img_desc->image_height == 0 ? 1 : img_desc->image_height,
                    img_desc->image_depth == 0 ? 1 : img_desc->image_depth};
  cl_event ev;
  err = clEnqueueNDRangeKernel(queue, kernel, dim, nullptr, gsize, nullptr, 0,
                               nullptr, &ev);
  ASSERT_OCL_SUCCESS(err, "clEnqueueNDRangeKernel");

  err = clEnqueueReadBuffer(queue, outbuf, true, 0, ByteSizeF, output, 1, &ev,
                            nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueReadBuffer");

  err = clFinish(queue);
  ASSERT_OCL_SUCCESS(err, "clFinish");

  for (size_t i = 0; i < ByteSize; i++)
    EXPECT_LE(abs(output[i] - val[i % 5]), 0.003)
        << "Output mismatched at " << i << ".\n";

  free(input);
  free(output);
  err = clReleaseEvent(ev);
  ASSERT_OCL_SUCCESS(err, "clReleaseEvent");
  err = clReleaseKernel(kernel);
  ASSERT_OCL_SUCCESS(err, "clReleaseKernel");
  err = clReleaseMemObject(outbuf);
  ASSERT_OCL_SUCCESS(err, "clReleaseMemObject");
  err = clReleaseProgram(program);
  ASSERT_OCL_SUCCESS(err, "clReleaseProgram");
  err = clReleaseMemObject(clImg);
  ASSERT_OCL_SUCCESS(err, "clReleaseMemObject");
}

static void TestImage1D(cl_context context, cl_command_queue queue,
                        cl_sampler sampler) {
  constexpr size_t Width = 4;
  constexpr size_t RowPitch = Width * 4; // num channels in CL_RGBA

  cl_image_desc image_desc = {
      CL_MEM_OBJECT_IMAGE1D, Width, 0, 0, 0, RowPitch, 0, 0, 0, {nullptr}};

  TestImage(context, queue, sampler, &image_desc);
}

static void TestImage2D(cl_context context, cl_command_queue queue,
                        cl_sampler sampler) {
  constexpr size_t Width = 4;
  constexpr size_t Height = 4;
  constexpr size_t RowPitch = Width * 4; // num channels in CL_RGBA

  cl_image_desc image_desc = {
      CL_MEM_OBJECT_IMAGE2D, Width, Height, 0, 0, RowPitch, 0, 0, 0, {nullptr}};

  TestImage(context, queue, sampler, &image_desc);
}

static void TestImage3D(cl_context context, cl_command_queue queue,
                        cl_sampler sampler) {
  constexpr size_t Width = 4;
  constexpr size_t Height = 4;
  constexpr size_t Depth = 4;
  constexpr size_t RowPitch = Width * 4; // num channels in CL_RGBA

  cl_image_desc image_desc = {CL_MEM_OBJECT_IMAGE3D,
                              Width,
                              Height,
                              Depth,
                              0,
                              RowPitch,
                              0,
                              0,
                              0,
                              {nullptr}};

  TestImage(context, queue, sampler, &image_desc);
}

void LinearSampleOOBCoord() {
  cl_int err;
  cl_platform_id platform = nullptr;
  cl_device_id device = nullptr;
  cl_context context = nullptr;

  err = clGetPlatformIDs(1, &platform, nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetPlatformIDs");
  err = clGetDeviceIDs(platform, gDeviceType, 1, &device, nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetDeviceIDs");

  const cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                         (cl_context_properties)platform, 0};
  context = clCreateContextFromType(prop, gDeviceType, nullptr, nullptr, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateContextFromType");

  cl_sampler_properties sampProps[7] = {CL_SAMPLER_NORMALIZED_COORDS,
                                        CL_FALSE,
                                        CL_SAMPLER_ADDRESSING_MODE,
                                        CL_ADDRESS_NONE,
                                        CL_SAMPLER_FILTER_MODE,
                                        CL_FILTER_LINEAR,
                                        0};

  cl_sampler sampler = clCreateSamplerWithProperties(context, sampProps, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateSamplerWithProperties");

  cl_command_queue queue =
      clCreateCommandQueueWithProperties(context, device, NULL, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateCommandQueueWithProperties");

  TestImage1D(context, queue, sampler);
  TestImage2D(context, queue, sampler);
  TestImage3D(context, queue, sampler);

  err = clReleaseSampler(sampler);
  ASSERT_OCL_SUCCESS(err, "clReleaseSampler");
  err = clReleaseContext(context);
  ASSERT_OCL_SUCCESS(err, "clReleaseContext");
}
