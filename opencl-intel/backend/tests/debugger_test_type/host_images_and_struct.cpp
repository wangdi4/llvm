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

#include "host_program_common.h"
#include <cstdlib>

using namespace std;

// Run a 1-dimensional NDrange on the given kernel, with kernel arguments
// uchar* buf_in, uchar* buf_out,
// __read_only  image2d_t image2df_in, __write_only image2d_t image2df_out,
// __read_only  image3d_t image3df_in,
// __read_only  image2d_t image2di_in, __write_only image2d_t image2di_out,
// __read_only  image3d_t image3di_in,
// __read_only  image2d_t image2dui_in, __write_only image2d_t image2dui_out,
// __read_only  image3d_t image3dui_in,
//  KernelArg* struct_in, __global KernelArg* struct_out, int4* vector_in, int4*
//  vector_out
//
// buf_in is initialized to a running sequence of 0, 1, ..., data_size - 1
//
// The extra arguments are:
//    <data_size> <ndrange_global_size> <ndrange_local_size>
//

#pragma pack(push)
#pragma pack(1)
typedef struct {
  int a;
  int b;
  cl_uchar c;
} KernelArg;
#pragma pack(pop)

static void host_images_and_struct_internal(cl::Context context,
                                            cl::Device device,
                                            cl::Program program,
                                            HostProgramExtraArgs extra_args) {
  cl::Kernel kernel(program, "main_kernel");
  cl::CommandQueue queue(context, device, 0);

  int data_size = 1024;
  int ndrange_global_size = 32;
  int ndrange_local_size = 1;

  if (extra_args.size() == 3) {
    data_size = atoi(extra_args[0].c_str());
    ndrange_global_size = atoi(extra_args[1].c_str());
    ndrange_local_size = atoi(extra_args[2].c_str());
  }

  // Data for the input buffer
  vector<cl_uchar> databuf(data_size);
  for (int i = 0; i < data_size; ++i) {
    databuf[i] = i;
  }

  cl::Buffer buf_in(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                    sizeof(cl_uchar) * data_size, &databuf[0], 0);
  kernel.setArg(0, buf_in);

  cl::Buffer buf_out(context, CL_MEM_READ_WRITE, sizeof(cl_uchar) * data_size,
                     0);
  kernel.setArg(1, buf_out);
  cl::Buffer buf_image(context, CL_MEM_READ_WRITE, sizeof(cl_int4) * 16, 0);
  int dataArr[16 * 4];

  // declaration of float type images
  cl::ImageFormat format_float(CL_RGBA, CL_FLOAT);
  cl::Image2D imagef2D1(
      context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR,
      format_float, (size_t)4, (size_t)4, (size_t)0, dataArr, NULL);
  cl::Image2D imagef2D2(
      context, CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR,
      format_float, (size_t)4, (size_t)4, (size_t)0, dataArr, NULL);
  kernel.setArg(2, imagef2D1);
  kernel.setArg(3, imagef2D2);
  cl::Image3D imagef3D1(context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR,
                        format_float, (size_t)4, (size_t)4, (size_t)100,
                        (size_t)0, (size_t)0, NULL, NULL);
  kernel.setArg(4, imagef3D1);

  // declaration of int type images
  cl::ImageFormat format_int(CL_RGBA, CL_SIGNED_INT32);
  cl::Image2D imagei2D1(context, CL_MEM_READ_ONLY, format_int, (size_t)100,
                        (size_t)100, (size_t)0, NULL, NULL);
  kernel.setArg(5, imagei2D1);
  cl::Image2D imagei2D2(context, CL_MEM_READ_WRITE, format_int, (size_t)100,
                        (size_t)100, (size_t)0, NULL, NULL);
  kernel.setArg(6, imagei2D2);
  cl::Image3D imagei3D1(context, CL_MEM_READ_ONLY, format_int, (size_t)100,
                        (size_t)100, (size_t)100, (size_t)0, (size_t)0, NULL,
                        NULL);
  kernel.setArg(7, imagei3D1);

  // declaration of unsigned int type images
  cl::ImageFormat format_uint(CL_RGBA, CL_UNSIGNED_INT8);
  cl::Image2D imageui2D1(context, CL_MEM_READ_ONLY, format_uint, (size_t)100,
                         (size_t)100, (size_t)0, NULL, NULL);
  kernel.setArg(8, imageui2D1);
  cl::Image2D imageui2D2(context, CL_MEM_READ_WRITE, format_uint, (size_t)100,
                         (size_t)100, (size_t)0, NULL, NULL);
  kernel.setArg(9, imageui2D2);
  cl::Image3D imageui3D1(context, CL_MEM_READ_ONLY, format_uint, (size_t)100,
                         (size_t)100, (size_t)100, (size_t)0, (size_t)0, NULL,
                         NULL);
  kernel.setArg(10, imageui3D1);

  // declaration of variables of type sturct KernelArg
  KernelArg karg1;
  karg1.a = 1;
  karg1.b = 2;
  karg1.c = 3;

  KernelArg karg2;
  karg2.a = 10;
  karg2.b = 20;
  karg2.c = 30;

  cl::Buffer buf_kernel_arg1(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                             sizeof(KernelArg), &karg1, 0);
  kernel.setArg(11, buf_kernel_arg1);

  cl::Buffer buf_kernel_arg2(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                             sizeof(KernelArg), &karg2, 0);
  kernel.setArg(12, buf_kernel_arg2);

  // declaration of vector of type int4
  cl_int4 vi1;
  vi1.s[0] = 0;
  vi1.s[1] = 1;
  vi1.s[2] = 2;
  vi1.s[3] = 3;
  cl_int4 vi2;
  vi2.s[0] = 0;
  vi2.s[1] = 2;
  vi2.s[2] = 4;
  vi2.s[3] = 6;

  cl::Buffer buf_vector1(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                         sizeof(cl_int4), &vi1, 0);
  kernel.setArg(13, buf_vector1);

  cl::Buffer buf_vector2(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                         sizeof(cl_int4), &vi2, 0);
  kernel.setArg(14, buf_vector2);

  DTT_LOG("Executing kernel in NDRange...");
  queue.enqueueNDRangeKernel(kernel, cl::NullRange,
                             cl::NDRange(ndrange_global_size),
                             cl::NDRange(ndrange_local_size));
  queue.finish();
}

// Export
//
HostProgramFunc host_images_and_struct = host_images_and_struct_internal;
