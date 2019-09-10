// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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
#include "cl_utils.h"
#include <cstdlib>
#include <stdexcept>

using namespace std;

extern const char* getWorkitemFocus();

// Run two NDrange instances. One with the given kernel, another with an
// extra kernel read from a file provided as the first extra argument.
//
static void host_multi_ndrange_internal(
    cl::Context context, cl::Device device, cl::Program program,
    HostProgramExtraArgs extra_args)
{
    cl::Kernel kernel(program, "main_kernel");
    cl::CommandQueue queue(context, device, 0);

    int data_size = 1024;
    int ndrange_global_size = 32;
    int ndrange_local_size = 1;

    string cl_code;

    if (extra_args.size() == 1) {
        cl_code = ReadFileContents(extra_args[0]);
        if (cl_code.size() == 0)
            throw runtime_error("host_multi_ndrange can't read kernel from file: " + extra_args[0]);
        DTT_LOG("Additional CL file name: " + extra_args[0]);
    }
    else
        throw runtime_error("host_multi_ndrange expected a CL filename");

    // Create the second kernel
    //
    cl::Program::Sources sources(1, make_pair(cl_code.c_str(), cl_code.size()));
    cl::Program prog = cl::Program(context, sources);

    // Build the program with debug arguments enabled.
    // In case of a build error, we can provide more information on the
    // failure from the build log.
    //
    try {
        vector<cl::Device> devices(1, device);
        string build_flags = "-g ";
        const char* gworkitem = getWorkitemFocus();
        if (gworkitem) {
            build_flags += "-gworkitem=";
            build_flags += gworkitem;
            build_flags += " ";
        }
        build_flags += string("-s \"") + extra_args[0] + "\"";
        prog.build(devices, build_flags.c_str());
    }
    catch (cl::Error err) {
        string buildlog;
        prog.getBuildInfo(device, CL_PROGRAM_BUILD_LOG, &buildlog);
        cerr << "Build error, log:\n" << buildlog << endl;
        throw;
    }

    cl::Kernel kernel2(prog, "main_kernel");

    // Data flow:
    // buf_int --> |kernel| --> buf_mid --> |kernel2| --> buf_out
    //
    cl::Buffer buf_in(context,
        CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR,
        sizeof(cl_uchar) * data_size, 0);
    kernel.setArg(0, buf_in);

    cl::Buffer buf_mid(context,
        CL_MEM_READ_WRITE,
        sizeof(cl_uchar) * data_size, 0);
    kernel.setArg(1, buf_mid);
    kernel2.setArg(0, buf_mid);

    cl::Buffer buf_out(context,
        CL_MEM_READ_WRITE,
        sizeof(cl_uchar) * data_size, 0);
    kernel2.setArg(1, buf_mid);

    DTT_LOG("Executing kernel in NDRange...");
    queue.enqueueNDRangeKernel(
        kernel,
        cl::NullRange,
        cl::NDRange(ndrange_global_size),
        cl::NDRange(ndrange_local_size));

	queue.finish();

    DTT_LOG("Executing kernel2 in NDRange...");
    queue.enqueueNDRangeKernel(
        kernel2,
        cl::NullRange,
        cl::NDRange(ndrange_global_size),
        cl::NDRange(ndrange_local_size));

    queue.finish();
}


// Export
//
HostProgramFunc host_multi_ndrange = host_multi_ndrange_internal;
