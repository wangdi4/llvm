// INTEL CONFIDENTIAL
//
// Copyright 2017-2018 Intel Corporation.
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

#define CHECK_ERROR(error, message)                           \
    if (error != CL_SUCCESS) {                                \
      throw runtime_error(message);                           \
      return;                                                 \
    }                                                         \
    error = !CL_SUCCESS;                                      \

using namespace std;

cl_int readPipe(cl_mem pipe, void* mem)
{
    cl_int error = CL_SUCCESS;
    do
    {
        error = clReadPipeIntelFPGA(pipe, mem);
    }
    while (error == CL_PIPE_EMPTY);

    return error;
}

cl_int writePipe(cl_mem pipe, const void* mem)
{
    cl_int error = CL_SUCCESS;
    do
    {
        error = clWritePipeIntelFPGA(pipe, mem);
    }
    while (error == CL_PIPE_FULL);

    return error;
}

static void host_fpga_host_side_pipes_internal(
    cl::Context context, cl::Device device, cl::Program program,
    HostProgramExtraArgs extra_args)
{
    cl::Kernel kernel(program, "main_kernel");
    const int maxBufferSize = 128;
    cl::CommandQueue queue(context, device);

    cl_int iRet = !CL_SUCCESS;

    // TODO: Enable cl2.cpp to avoid using OpenCL C API for 2.0 features
    cl_mem pipeRead = clCreatePipe(context(), CL_MEM_HOST_READ_ONLY,
                              sizeof(cl_int), maxBufferSize,
                              nullptr, &iRet);
    CHECK_ERROR(iRet, "Failed to create a pipe (read)");

    cl_mem pipeWrite = clCreatePipe(context(), CL_MEM_HOST_WRITE_ONLY,
                               sizeof(cl_int), maxBufferSize,
                               nullptr, &iRet);
    CHECK_ERROR(iRet, "Failed to create a pipe (write)");

    kernel.setArg(0, sizeof(pipeWrite), &pipeWrite);
    kernel.setArg(1, sizeof(pipeRead), &pipeRead);
    kernel.setArg(2, sizeof(maxBufferSize), &maxBufferSize);

    DTT_LOG("[FPGA] Running application with host side pipes...");

    queue.enqueueTask(kernel);
    queue.flush();

    for (cl_int i = 0; i < (cl_int)maxBufferSize; ++i)
    {
        iRet = writePipe(pipeWrite, &i);
        CHECK_ERROR(iRet, "Failed to write to a pipe!");
    }

    for (cl_int i = 0; i < (cl_int)maxBufferSize; ++i)
    {
        cl_int got = -1;
        iRet = readPipe(pipeRead, &got);
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
