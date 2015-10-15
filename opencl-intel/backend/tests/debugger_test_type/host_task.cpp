#include "host_program_common.h"
#include "test_utils.h"
#include <cstdlib>
#include <algorithm>
#include <stdexcept>

using namespace std;


const char* EXPECTED_OUTPUT2 = ""
    "0   1.10,  2.20,  3.30,  7.40 - that's all\n";

bool compare_kernel_output2(const string& expected, const string& actual)
{
    vector<string> expected_vec = tokenize(expected, "\n\r");
    vector<string> actual_vec = tokenize(actual, "\n\r");

    sort(expected_vec.begin(), expected_vec.end());
    sort(actual_vec.begin(), actual_vec.end());

    return expected_vec == actual_vec;
}


// This host program is designed to test kernels that contain printf
// call. The kernel can do whatever it wants, as long as it prints
// the EXPECTED_OUTPUT.
// The kernel is invoked as task

// The extra arguments are:
//    <skip_verify>
//
// If skip_verify is provided and is 'true', skips expected output
// verification.
//
static void host_task_internal(
    cl::Context context, cl::Device device, cl::Kernel kernel,
    HostProgramExtraArgs extra_args)
{
    cl::CommandQueue queue(context, device, 0);

    bool skip_verify = false;
    if (extra_args.size() == 1) {
        skip_verify = extra_args[0] == "true";
    }

    int data_size = 4;

    cl::Buffer buf_in(context,
        CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR,
        sizeof(cl_uchar) * data_size, 0);
    kernel.setArg(0, buf_in);

    cl::Buffer buf_out(context,
        CL_MEM_READ_WRITE,
        sizeof(cl_uchar) * data_size, 0);
    kernel.setArg(1, buf_out);

    DTT_LOG("Executing kernel as task...");
    CaptureStdout();
    queue.enqueueTask(
        kernel);
    queue.finish();

    string out = GetCapturedStdout();
    if (!skip_verify && !compare_kernel_output2(EXPECTED_OUTPUT2, out)) {
        string errmsg = "Printf output comparison failed\n";
        errmsg += string("Expected:\n") + EXPECTED_OUTPUT2 + "~~~~~~\n";
        errmsg += string("Got:\n") + out + "~~~~~~\n";
        throw runtime_error(errmsg);
    }
}


// Export
//
HostProgramFunc host_task = host_task_internal;
