// INTEL CONFIDENTIAL
//
// Copyright 2017 Intel Corporation.
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
#include "test_utils.h"
#include <stdexcept>

using namespace std;

static void host_fpga_channels_internal(cl::Context context, cl::Device device,
                                        cl::Program program,
                                        HostProgramExtraArgs extra_args) {
  const int size = 1024;
  cl::Kernel k_reader(program, "channel_reader");
  cl::Kernel k_writer(program, "channel_writer");
  cl::CommandQueue queue1(context, device);
  cl::CommandQueue queue2(context, device);

  k_writer.setArg(0, sizeof(size), &size);

  cl::Buffer buf_out(context, CL_MEM_READ_WRITE, sizeof(cl_uint) * size);
  k_reader.setArg(0, sizeof(size), &size);
  k_reader.setArg(1, sizeof(buf_out), &buf_out);

  DTT_LOG("[FPGA] Running application with channels...");

  queue1.enqueueNDRangeKernel(k_writer, cl::NullRange, cl::NDRange(1));
  queue2.enqueueNDRangeKernel(k_reader, cl::NullRange, cl::NDRange(1));
  queue1.finish();
  queue2.finish();

  vector<cl_int> buf_result(size);
  queue2.enqueueReadBuffer(buf_out, CL_TRUE, 0, sizeof(cl_uint) * size,
                           &buf_result[0]);

  for (cl_int i = 0; i < size; ++i) {
    if (i != buf_result[i])
      throw runtime_error("Verification failed");
  }
  queue2.finish();
}

// Export
//
HostProgramFunc host_fpga_channels = host_fpga_channels_internal;
