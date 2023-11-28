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
#include <stdexcept>

using namespace std;

// Run a 1-dimensional NDrange on the given kernel, with kernel arguments
// float4* buf_in, float4* buf_out, int width, int height
//
// buf_in is initialized to a running sequence of 0.0, 1.0, ..., width*height
// - 1.0 each replicated in float4.
//
// The extra arguments are:
//    <width> <height> <ndrange_global_size> <ndrange_local_size>
//
static void host_1d_float4_with_size_internal(cl::Context context,
                                              cl::Device device,
                                              cl::Program program,
                                              HostProgramExtraArgs extra_args) {
  cl::Kernel kernel(program, "main_kernel");
  cl::CommandQueue queue(context, device, 0);

  int width = 1024;
  int height = 256;
  int ndrange_global_size = 32;
  int ndrange_local_size = 1;

  if (extra_args.size() == 4) {
    width = atoi(extra_args[0].c_str());
    height = atoi(extra_args[1].c_str());
    ndrange_global_size = atoi(extra_args[2].c_str());
    ndrange_local_size = atoi(extra_args[3].c_str());
  }

  int data_size = width * height;

  // Data for the input buffer
  vector<cl_float4> databuf(data_size);
  for (int i = 0; i < data_size; ++i) {
    databuf[i].s[0] = cl_float(i);
    databuf[i].s[1] = cl_float(i);
    databuf[i].s[2] = cl_float(i);
    databuf[i].s[3] = cl_float(i);
  }

  cl::Buffer buf_in(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                    sizeof(cl_float4) * data_size, &databuf[0], 0);
  kernel.setArg(0, buf_in);

  cl::Buffer buf_out(context, CL_MEM_READ_WRITE, sizeof(cl_float4) * data_size,
                     0);
  kernel.setArg(1, buf_out);

  kernel.setArg(2, width);
  kernel.setArg(3, height);

  DTT_LOG("Executing kernel in NDRange...");
  queue.enqueueNDRangeKernel(kernel, cl::NullRange,
                             cl::NDRange(ndrange_global_size),
                             cl::NDRange(ndrange_local_size));
  queue.finish();
}

// Export
//
HostProgramFunc host_1d_float4_with_size = host_1d_float4_with_size_internal;
