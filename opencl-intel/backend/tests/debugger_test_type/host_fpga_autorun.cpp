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
#include <numeric>
#include <stdexcept>

using namespace std;

static void host_fpga_autorun_internal(cl::Context context, cl::Device device,
                                       cl::Program program,
                                       HostProgramExtraArgs extra_args) {
  const int N = 5;
  const cl_int numElements = 1000;
  const size_t bufferSize = numElements * sizeof(cl_int);

  vector<cl_int> input_data(numElements);
  iota(input_data.begin(), input_data.end(), 0);

  cl::CommandQueue reader_queue(context, device);
  cl::CommandQueue writer_queue(context, device);

  cl::Buffer buf_in(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, bufferSize,
                    &input_data[0]);
  cl::Buffer buf_out(context, CL_MEM_WRITE_ONLY, bufferSize);

  cl::Kernel reader_for_single_plus(program, "reader_for_single_plus");
  cl::Kernel writer_for_single_plus(program, "writer_for_single_plus");

  reader_for_single_plus.setArg(0, sizeof(cl_int), &numElements);
  reader_for_single_plus.setArg(1, sizeof(buf_out), &buf_out);

  writer_for_single_plus.setArg(0, sizeof(cl_int), &numElements);
  writer_for_single_plus.setArg(1, sizeof(buf_in), &buf_in);

  DTT_LOG("[FPGA] Running application with autorun kernels...");

  writer_queue.enqueueNDRangeKernel(writer_for_single_plus, cl::NullRange,
                                    cl::NDRange(1, 1, 1), cl::NDRange(1, 1, 1));
  reader_queue.enqueueNDRangeKernel(reader_for_single_plus, cl::NullRange,
                                    cl::NDRange(1, 1, 1), cl::NDRange(1, 1, 1));
  reader_queue.finish();

  vector<cl_int> output_data(numElements);
  reader_queue.enqueueReadBuffer(buf_out, CL_TRUE, 0, bufferSize,
                                 &output_data[0]);
  for (cl_int i = 0; i < numElements; ++i) {
    if (i + 1 != output_data[i])
      throw runtime_error("Verification single kernels failed");
  }

  cl::Kernel reader_for_chained_plus(program, "reader_for_chained_plus");
  cl::Kernel writer_for_chained_plus(program, "writer_for_chained_plus");

  reader_queue.enqueueFillBuffer(buf_out, 0, 0, bufferSize);
  fill(output_data.begin(), output_data.end(), 0);
  reader_queue.finish();

  reader_for_chained_plus.setArg(0, sizeof(cl_int), &numElements);
  reader_for_chained_plus.setArg(1, sizeof(buf_out), &buf_out);

  writer_for_chained_plus.setArg(0, sizeof(cl_int), &numElements);
  writer_for_chained_plus.setArg(1, sizeof(buf_in), &buf_in);

  writer_queue.enqueueNDRangeKernel(writer_for_chained_plus, cl::NullRange,
                                    cl::NDRange(1, 1, 1), cl::NDRange(1, 1, 1));
  reader_queue.enqueueNDRangeKernel(reader_for_chained_plus, cl::NullRange,
                                    cl::NDRange(1, 1, 1), cl::NDRange(1, 1, 1));
  reader_queue.finish();
  reader_queue.enqueueReadBuffer(buf_out, CL_TRUE, 0, bufferSize,
                                 &output_data[0]);
  for (cl_int i = 0; i < numElements; ++i) {
    if (i + N != output_data[i])
      throw runtime_error("Verification chained failed");
  }
  reader_queue.finish();
  writer_queue.finish();
}

// Export
//
HostProgramFunc host_fpga_autorun = host_fpga_autorun_internal;
