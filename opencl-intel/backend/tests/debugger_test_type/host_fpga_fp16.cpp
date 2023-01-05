// INTEL CONFIDENTIAL
//
// Copyright 2018 Intel Corporation.
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
#include <sstream>
#include <stdexcept>

static void host_fpga_fp16_internal(cl::Context context, cl::Device device,
                                    cl::Program program,
                                    HostProgramExtraArgs extra_args) {
  cl::Kernel kernel(program, "fp16");
  cl::CommandQueue queue(context, device, 0);

  short input = 0x3555; // ~0.333 in half
  short output = 0;
  short output_ref = 0x3800; // ~0.5 in half

  cl::Buffer buf_in(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                    sizeof(input), &input);

  cl::Buffer buf_out(context, CL_MEM_READ_WRITE, sizeof(output));

  kernel.setArg(0, buf_in);
  kernel.setArg(1, buf_out);

  DTT_LOG("Executing convert_fp16 kernel...");

  queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(1));

  queue.enqueueReadBuffer(buf_out, CL_TRUE, 0, sizeof(output), &output);

  if (output != output_ref) {
    std::stringstream ss;
    ss << "Mismatch error:\n";
    ss << "Got:      " << output << "\n";
    ss << "Expected: " << output_ref << "\n";

    throw std::runtime_error(ss.str());
  }
}

// Export
//
HostProgramFunc host_fpga_fp16 = host_fpga_fp16_internal;
