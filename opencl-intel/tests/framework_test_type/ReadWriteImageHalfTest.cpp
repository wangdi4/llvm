// Copyright (C) 2023 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "CL/cl.h"
#include "TestsHelpClasses.h"
#include "common_utils.h"
#include "test_utils.h"
#include <cstddef>

extern cl_device_type gDeviceType;

static const char *Test1d[] = {R"(
__kernel void image_test(__global float4* outbuf,
     __global float4* inbuf,
    __read_write image1d_t img) {
  size_t x = get_global_id(0);

  half4 write_data_h = convert_half4(inbuf[x]);
  
  // write inbuf to half type image
  write_imageh(img, x, write_data_h);
  
  // read half type image
  half4 read_data_h = read_imageh(img, x);
  
  // convert to float to verify result
  outbuf[x] = convert_float4(read_data_h);
})"};
static const char *Test2d[] = {R"(
__kernel void image_test(__global float4* outbuf,
     __global float4* inbuf,
    __read_write image2d_t img) {
  size_t x = get_global_id(0);
  size_t y = get_global_id(1);
  size_t width = get_image_width(img);
  size_t idx = width * y + x;
  
  half4 write_data_h = convert_half4(inbuf[idx]);
  
  // write inbuf to half type image
  write_imageh(img, (int2){x, y}, write_data_h);
  
  // read half type image
  half4 read_data_h = read_imageh(img, (int2){x, y});
  
  // convert to float to verify result
  outbuf[idx] = convert_float4(read_data_h);
})"};
static const char *Test3d[] = {R"(
__kernel void image_test(__global float4* inbuf, 
    __global float4* outbuf,
    __read_write image3d_t img) {
  size_t x = get_global_id(0);
  size_t y = get_global_id(1);
  size_t z = get_global_id(2);
  size_t width = get_image_width(img);
  size_t height = get_image_height(img);
  size_t idx = height * width * z + width * y + x;

  half4 write_data_h = convert_half4(inbuf[idx]);
  
  // write inbuf to half type image
  write_imageh(img, (int4){x, y, z, 0}, write_data_h);
  
  // read half type image
  half4 read_data_h = read_imageh(img, (int4){x, y, z, 0});
  
  // convert to float to verify result
  outbuf[idx] = convert_float4(read_data_h);
})"};

static void testImage(cl_context Context, cl_command_queue Queue,
                      const cl_image_desc *ImgDesc) {
  cl_int Err;

  size_t ImgSize = ImgDesc->image_row_pitch;
  if (size_t Height = ImgDesc->image_height)
    ImgSize *= Height;
  if (size_t Depth = ImgDesc->image_depth)
    ImgSize *= Depth;
  size_t ByteSizeH = ImgSize * sizeof(half);
  size_t ByteSizeF = ImgSize * sizeof(float);

  half *Img = (half *)malloc(ByteSizeH);
  ASSERT_NE(Img, nullptr);
  float *Input = (float *)malloc(ByteSizeF);
  float *Output = (float *)malloc(ByteSizeF);
  ASSERT_NE(Output, nullptr);

  static const float Val[] = {1.1f, 2.2f, 3.3f, 4.4f, 5.5f};

  for (size_t I = 0; I < ImgSize; ++I) {
    Input[I] = Val[I % 5];
    Output[I] = Val[I % 5];
    Img[I] = 0;
  }

  cl_image_format HalfFormat = {CL_RGBA, CL_HALF_FLOAT};

  int Dim;
  const char **Src;
  switch (ImgDesc->image_type) {
  case CL_MEM_OBJECT_IMAGE1D:
    Dim = 1;
    Src = Test1d;
    break;
  case CL_MEM_OBJECT_IMAGE2D:
    Dim = 2;
    Src = Test2d;
    break;
  case CL_MEM_OBJECT_IMAGE3D:
    Dim = 3;
    Src = Test3d;
    break;
  }

  cl_program Program;
  Err = BuildProgramSynch(Context, 1, Src, nullptr, "-cl-std=CL3.0", &Program);
  ASSERT_TRUE(Err) << "BuildProgramSynch failed";

  cl_mem CLImg =
      clCreateImage(Context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                    &HalfFormat, ImgDesc, Img, &Err);
  ASSERT_OCL_SUCCESS(Err, "clCreateImage");

  cl_mem Inbuf =
      clCreateBuffer(Context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                     ByteSizeF, Input, &Err);
  ASSERT_OCL_SUCCESS(Err, "clCreateBuffer");

  cl_mem Outbuf =
      clCreateBuffer(Context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                     ByteSizeF, Output, &Err);
  ASSERT_OCL_SUCCESS(Err, "clCreateBuffer");

  cl_kernel Kernel = clCreateKernel(Program, "image_test", &Err);
  ASSERT_OCL_SUCCESS(Err, "clCreateKernel");

  Err = clSetKernelArg(Kernel, 0, sizeof(cl_mem), &Inbuf);
  ASSERT_OCL_SUCCESS(Err, "clSetKernelArg(0)");
  Err = clSetKernelArg(Kernel, 1, sizeof(cl_mem), &Outbuf);
  ASSERT_OCL_SUCCESS(Err, "clSetKernelArg(1)");
  Err = clSetKernelArg(Kernel, 2, sizeof(cl_mem), &CLImg);
  ASSERT_OCL_SUCCESS(Err, "clSetKernelArg(0)");

  size_t Gsize[] = {ImgDesc->image_width,
                    ImgDesc->image_height == 0 ? 1 : ImgDesc->image_height,
                    ImgDesc->image_depth == 0 ? 1 : ImgDesc->image_depth};

  cl_event Ev;
  Err = clEnqueueNDRangeKernel(Queue, Kernel, Dim, nullptr, Gsize, nullptr, 0,
                               nullptr, &Ev);
  ASSERT_OCL_SUCCESS(Err, "clEnqueueNDRangeKernel");

  Err = clEnqueueReadBuffer(Queue, Outbuf, true, 0, ByteSizeF, Output, 1, &Ev,
                            nullptr);
  ASSERT_OCL_SUCCESS(Err, "clEnqueueReadBuffer");

  Err = clFinish(Queue);
  ASSERT_OCL_SUCCESS(Err, "clFinish");

  for (size_t I = 0; I < ImgSize; I++) {
    EXPECT_LE(abs(Output[I] - Val[I % 5]), 0.003)
        << "Output mismatched at " << I << ".\n";
  }

  free(Img);
  free(Output);
  free(Input);
  Err = clReleaseEvent(Ev);
  ASSERT_OCL_SUCCESS(Err, "clReleaseEvent");
  Err = clReleaseKernel(Kernel);
  ASSERT_OCL_SUCCESS(Err, "clReleaseKernel");
  Err = clReleaseMemObject(Outbuf);
  ASSERT_OCL_SUCCESS(Err, "clReleaseMemObject");
  Err = clReleaseProgram(Program);
  ASSERT_OCL_SUCCESS(Err, "clReleaseProgram");
  Err = clReleaseMemObject(CLImg);
  ASSERT_OCL_SUCCESS(Err, "clReleaseMemObject");
}

static void testImage1D(cl_context Context, cl_command_queue Queue) {
  constexpr size_t Width = 4;
  constexpr size_t RowPitch = Width * 4; // num channels in CL_RGBA

  cl_image_desc ImageDesc = {CL_MEM_OBJECT_IMAGE1D,   Width, 0, 0, 0,
                             RowPitch * sizeof(half), 0,     0, 0, {nullptr}};

  testImage(Context, Queue, &ImageDesc);
}

static void testImage2D(cl_context Context, cl_command_queue Queue) {
  constexpr size_t Width = 4;
  constexpr size_t Height = 4;
  constexpr size_t RowPitch = Width * 4; // num channels in CL_RGBA

  cl_image_desc ImageDesc = {
      CL_MEM_OBJECT_IMAGE2D,   Width, Height, 0, 0,
      RowPitch * sizeof(half), 0,     0,      0, {nullptr}};

  testImage(Context, Queue, &ImageDesc);
}

static void testImage3D(cl_context Context, cl_command_queue Queue) {
  constexpr size_t Width = 4;
  constexpr size_t Height = 4;
  constexpr size_t Depth = 4;
  constexpr size_t RowPitch = Width * 4; // num channels in CL_RGBA

  cl_image_desc ImageDesc = {
      CL_MEM_OBJECT_IMAGE3D,   Width, Height, Depth, 0,
      RowPitch * sizeof(half), 0,     0,      0,     {nullptr}};

  testImage(Context, Queue, &ImageDesc);
}

void ReadWriteImageHalfTest() {
  cl_int Err;
  cl_platform_id Platform = nullptr;
  cl_device_id Device = nullptr;
  cl_context Context = nullptr;

  Err = clGetPlatformIDs(1, &Platform, nullptr);
  ASSERT_OCL_SUCCESS(Err, "clGetPlatformIDs");
  Err = clGetDeviceIDs(Platform, gDeviceType, 1, &Device, nullptr);
  ASSERT_OCL_SUCCESS(Err, "clGetDeviceIDs");

  const cl_context_properties Prop[3] = {CL_CONTEXT_PLATFORM,
                                         (cl_context_properties)Platform, 0};
  Context = clCreateContextFromType(Prop, gDeviceType, nullptr, nullptr, &Err);
  ASSERT_OCL_SUCCESS(Err, "clCreateContextFromType");

  cl_command_queue Queue =
      clCreateCommandQueueWithProperties(Context, Device, NULL, &Err);
  ASSERT_OCL_SUCCESS(Err, "clCreateCommandQueueWithProperties");

  testImage1D(Context, Queue);
  testImage2D(Context, Queue);
  testImage3D(Context, Queue);

  Err = clReleaseContext(Context);
  ASSERT_OCL_SUCCESS(Err, "clReleaseContext");
}
