// INTEL CONFIDENTIAL
//
// Copyright 2019 Intel Corporation.
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

using namespace std;

static void host_compile_link_internal(
    cl::Context context, cl::Device device,
    cl::Program unusedProgram, // This program is built with clBuildProgram,
                               // while we want to build it with
                               // clCompileProgram followed by clLinkProgram.
    HostProgramExtraArgs extra_args) {
  const char *src = "__kernel void foo() { size_t gid = get_global_id(0); }";
  cl::Program program = cl::Program(context, src, /*build=*/false,
                                    /*err=*/NULL);
  program.compile("-g -cl-opt-disable -s compile_link.cl");
  program = cl::linkProgram({program});

  cl::CommandQueue queue(context, device);
  cl::Kernel kernel(program, "foo");

  queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(1));

  DTT_LOG("[FPGA] Running application with device code built "
          "by compile-and-link flow...");

  queue.finish();
}

// Export
//
HostProgramFunc host_compile_link = host_compile_link_internal;
