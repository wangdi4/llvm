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

#include "CL/cl_fpga_ext.h"
#include "host_program_common.h"
#include "test_utils.h"
#include <stdexcept>

#define CHECK(success, message)                                                \
  if (!(success)) {                                                            \
    throw runtime_error(message);                                              \
    return;                                                                    \
  }

#define CHECK_ERROR(error, message)                                            \
  CHECK(error == CL_SUCCESS, message);                                         \
  error = !CL_SUCCESS;

using namespace std;

typedef cl_int (*read_pipe_fn)(cl_mem, void *);
typedef cl_int (*write_pipe_fn)(cl_mem, const void *);

cl_int readPipe(read_pipe_fn readFun, cl_mem pipe, void *mem) {
  cl_int error = CL_SUCCESS;
  do {
    error = readFun(pipe, mem);
  } while (error == CL_PIPE_EMPTY);

  return error;
}

cl_int writePipe(write_pipe_fn writeFun, cl_mem pipe, const void *mem) {
  cl_int error = CL_SUCCESS;
  do {
    error = writeFun(pipe, mem);
  } while (error == CL_PIPE_FULL);

  return error;
}

static void
host_fpga_host_side_pipes_internal(cl::Context context, cl::Device device,
                                   cl::Program program,
                                   HostProgramExtraArgs extra_args) {
  cl::Kernel kernel(program, "main_kernel");
  const int maxBufferSize = 128;
  cl::CommandQueue queue(context, device);

  cl_int iRet = !CL_SUCCESS;

  read_pipe_fn readFun = (read_pipe_fn)(uintptr_t)clGetExtensionFunctionAddress(
      "clReadPipeIntelFPGA");
  CHECK(readFun, "clGetExtensionFunctionAddress(clReadPipeIntelFPGA) failed");

  write_pipe_fn writeFun =
      (write_pipe_fn)(uintptr_t)clGetExtensionFunctionAddress(
          "clWritePipeIntelFPGA");
  CHECK(writeFun, "clGetExtensionFunctionAddress(clWritePipeIntelFPGA) failed");

  // TODO: Enable cl2.cpp to avoid using OpenCL C API for 2.0 features
  cl_mem pipeRead = clCreatePipe(context(), CL_MEM_HOST_READ_ONLY,
                                 sizeof(cl_int), maxBufferSize, nullptr, &iRet);
  CHECK_ERROR(iRet, "Failed to create a pipe (read)");

  cl_mem pipeWrite =
      clCreatePipe(context(), CL_MEM_HOST_WRITE_ONLY, sizeof(cl_int),
                   maxBufferSize, nullptr, &iRet);
  CHECK_ERROR(iRet, "Failed to create a pipe (write)");

  kernel.setArg(0, sizeof(pipeWrite), &pipeWrite);
  kernel.setArg(1, sizeof(pipeRead), &pipeRead);
  kernel.setArg(2, sizeof(maxBufferSize), &maxBufferSize);

  DTT_LOG("[FPGA] Running application with host side pipes...");

  queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(1));
  queue.flush();

  for (cl_int i = 0; i < (cl_int)maxBufferSize; ++i) {
    iRet = writePipe(writeFun, pipeWrite, &i);
    CHECK_ERROR(iRet, "Failed to write to a pipe!");
  }

  for (cl_int i = 0; i < (cl_int)maxBufferSize; ++i) {
    cl_int got = -1;
    iRet = readPipe(readFun, pipeRead, &got);
    CHECK_ERROR(iRet, "Failed to read to a pipe!");
    if (i != got) {
      throw runtime_error("Verification failed");
      return;
    }
  }
  queue.finish();
}

// Export
//
HostProgramFunc host_fpga_host_side_pipes = host_fpga_host_side_pipes_internal;
