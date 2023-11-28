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

const unsigned ROTATION_SIZE = 16;

#pragma pack(push)
#pragma pack(1)
typedef struct {
  float f1, f2;
  cl_float4 position;

  cl_uint size;

  float rotation[ROTATION_SIZE];
} KernelArg;
#pragma pack(pop)

// Run a 1-dimensional NDrange on the given kernel, with kernel arguments
// uchar* buf_in, uchar* buf_out and KernelArg*.
// The data passed in the KernelArg is some constants defined here.
//
// buf_in is initialized to a running sequence of 0, 1, ..., data_size - 1
//
// The extra arguments are:
//    <data_size> <ndrange_global_size> <ndrange_local_size>
//
static void host_struct_kernel_arg_internal(cl::Context context,
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

  KernelArg karg;
  karg.f1 = 100.0f;
  karg.f2 = -9999.25f;
  karg.position.s[0] = 0.25f;
  karg.position.s[1] = 0.75f;
  karg.position.s[2] = 1.25f;
  karg.position.s[3] = 2.25f;
  karg.size = data_size;
  for (unsigned i = 0; i < ROTATION_SIZE; ++i)
    karg.rotation[i] = float(i);

  cl::Buffer buf_kernel_arg(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                            sizeof(KernelArg), &karg, 0);
  kernel.setArg(2, buf_kernel_arg);

  DTT_LOG("Executing kernel in NDRange...");
  queue.enqueueNDRangeKernel(kernel, cl::NullRange,
                             cl::NDRange(ndrange_global_size),
                             cl::NDRange(ndrange_local_size));
  queue.finish();
}

// Export
//
HostProgramFunc host_struct_kernel_arg = host_struct_kernel_arg_internal;
