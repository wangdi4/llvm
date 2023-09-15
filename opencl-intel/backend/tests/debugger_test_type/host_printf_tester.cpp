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
#include "test_utils.h"
#include <algorithm>
#include <cstdlib>
#include <stdexcept>

using namespace std;

const char *EXPECTED_OUTPUT = ""
                              "3   1.10,  2.20,  3.30,  7.40 - that's all\n";

// Since kernel output may be in arbitrary order (we can't ensure which work
// item runs first), to compare it to an expected output we split the output to
// lines and sort them.
// Note: this assumes each work-item outputs one or more lines ending with '\n'
//
bool compare_kernel_output(const string &expected, const string &actual) {
  vector<string> expected_vec = tokenize(expected, "\n\r");
  vector<string> actual_vec = tokenize(actual, "\n\r");

  sort(expected_vec.begin(), expected_vec.end());
  sort(actual_vec.begin(), actual_vec.end());

  return expected_vec == actual_vec;
}

// This host program is designed to test kernels that contain printf
// calls. The kernel can do whatever it wants, as long as it prints
// the EXPECTED_OUTPUT.
// The kernel is invoked in 1D NDrange with global size 4.

// The extra arguments are:
//    <skip_verify>
//
// If skip_verify is provided and is 'true', skips expected output
// verification.
//
static void host_printf_tester_internal(cl::Context context, cl::Device device,
                                        cl::Program program,
                                        HostProgramExtraArgs extra_args) {
  cl::Kernel kernel(program, "main_kernel");
  cl::CommandQueue queue(context, device, 0);

  bool skip_verify = false;
  if (extra_args.size() == 1) {
    skip_verify = extra_args[0] == "true";
  }

  int data_size = 4;
  int ndrange_global_size = 4;
  int ndrange_local_size = 1;

  cl::Buffer buf_in(context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR,
                    sizeof(cl_uchar) * data_size, 0);
  kernel.setArg(0, buf_in);

  cl::Buffer buf_out(context, CL_MEM_READ_WRITE, sizeof(cl_uchar) * data_size,
                     0);
  kernel.setArg(1, buf_out);

  DTT_LOG("Executing kernel in NDRange...");
  CaptureStdout();
  queue.enqueueNDRangeKernel(kernel, cl::NullRange,
                             cl::NDRange(ndrange_global_size),
                             cl::NDRange(ndrange_local_size));
  queue.finish();

  string out = GetCapturedStdout();
  if (!skip_verify && !compare_kernel_output(EXPECTED_OUTPUT, out)) {
    string errmsg = "Printf output comparison failed\n";
    errmsg += string("Expected:\n") + EXPECTED_OUTPUT + "~~~~~~\n";
    errmsg += string("Got:\n") + out + "~~~~~~\n";
    throw runtime_error(errmsg);
  }
}

// Export
//
HostProgramFunc host_printf_tester = host_printf_tester_internal;
