// ===--- host_fpga_fp16.cpp - a debugger test for FP16               -----===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "host_program_common.h"
#include <cstdlib>
#include <sstream>
#include <stdexcept>

static void host_fpga_fp16_internal(
    cl::Context context, cl::Device device, cl::Program program,
    HostProgramExtraArgs extra_args)
{
    cl::Kernel kernel(program, "fp16");
    cl::CommandQueue queue(context, device, 0);

    short input = 0x3555; // ~0.333 in half
    short output = 0;
    short output_ref = 0x3800; // ~0.5 in half

    cl::Buffer buf_in(
        context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        sizeof(input), &input);

    cl::Buffer buf_out(
        context, CL_MEM_READ_WRITE, sizeof(output));

    kernel.setArg(0, buf_in);
    kernel.setArg(1, buf_out);

    DTT_LOG("Executing convert_fp16 kernel...");

    queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(1));

    queue.enqueueReadBuffer(buf_out, CL_TRUE, 0,
                            sizeof(output), &output);

    if (output != output_ref)
    {
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
