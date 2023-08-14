// Tests for the __opencl_printf builtin (accessible as a 'printf' call inside
// kernels).
//
// Note: the __opencl_printf is comprehensively tested in the backend. Here the
// goal is only to verify that a printf call from the kernel is correctly
// linked to the __opencl_printf builtin, and allows to print vectors.
//

// Use the OpenCL C++ bindings, with exceptions enabled. For MSVC, disable
// warning 4290 (C++ exception specifications ignored) that's emitted from
// CL/opencl.hpp
#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_TARGET_OPENCL_VERSION 200

#include "CL/cl.h"
#include "cl_utils.h"
#include "test_utils.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4290)
#endif // _MSC_VER
#include "CL/opencl.hpp"
#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
using namespace std;

extern cl_device_type gDeviceType;

const bool DEBUG = false;

const char *KERNEL_CODE_STR =
    ""
    "__kernel void hello(__global uchar* buf_in, __global uchar* buf_out)"
    "{__constant char* value1 = \"Hello1\";"
    " __constant char* format = \"%s,%s\\n\";"
    " __constant char* value2 = \"Hello2\";"
    " printf(format,value1,value2);"
    " value1 = (__constant char*)0;"
    " printf(format,value1,value2);"
    " buf_out[get_global_id(0)] = buf_in[get_global_id(0)]; "
    " float4 fl4 = (float4)(1.1f, 2.2f, 3.3f, 4.4f);"
    " fl4.w += (float)get_global_id(0);"
    " int2 ii2 = (int2)(get_global_id(0), 9);"
    " printf(\"%d %6.2v4f - %v2d - a char %c and an int %d\\n\", ii2.x, fl4, "
    "ii2, 'k', 112233); "
    "}";

const char *EXPECTED_OUTPUT =
    ""
    "Hello1,Hello2\n(null),Hello2\n0   1.10,  2.20,  3.30,  4.40 - 0,9 - a "
    "char k and an int 112233\n"
    "Hello1,Hello2\n(null),Hello2\n1   1.10,  2.20,  3.30,  5.40 - 1,9 - a "
    "char k and an int 112233\n"
    "Hello1,Hello2\n(null),Hello2\n2   1.10,  2.20,  3.30,  6.40 - 2,9 - a "
    "char k and an int 112233\n"
    "Hello1,Hello2\n(null),Hello2\n3   1.10,  2.20,  3.30,  7.40 - 3,9 - a "
    "char k and an int 112233\n";

bool opencl_printf_test() {
  cl_int err = CL_SUCCESS;
  string kernel_code = KERNEL_CODE_STR;

  cout << "---------------------------------------\n";
  cout << "opencl_printf_test\n";
  cout << "---------------------------------------\n";

  if (DEBUG) {
    cout << "Running kernel:\n----------------------------------------------\n"
         << kernel_code << "\n----------------------------------------------\n";
  }

  try {
    vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    if (platforms.size() == 0) {
      cout << "FAIL: 0 platforms found\n";
      return false;
    }

    cl_context_properties properties[] = {
        CL_CONTEXT_PLATFORM, (cl_context_properties)(platforms[0])(), 0};
    cl::Context context(gDeviceType, properties);

    vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();

    cl::CommandQueue queue(context, devices[0], 0, &err);

    cl::Program::Sources source(1, kernel_code);
    cl::Program program_ = cl::Program(context, source);

    // In case of a build error, we can provide more information on the
    // failure from the build log.
    //
    try {
      program_.build(devices);
    } catch (cl::Error err) {
      string buildlog;
      program_.getBuildInfo(devices[0], CL_PROGRAM_BUILD_LOG, &buildlog);
      cout << "FAIL: Build log:\n" << buildlog << endl;
      throw;
    }

    cl::Kernel kernel(program_, "hello", &err);

    size_t datalen = 4;

    cl::Buffer buf_in(context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR,
                      sizeof(cl_uchar) * datalen, 0, &err);

    cl::Buffer buf_out(context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR,
                       sizeof(cl_uchar) * datalen, 0, &err);

    kernel.setArg(0, buf_in);
    kernel.setArg(1, buf_out);

    if (!CaptureStdout()) {
      fprintf(stderr, "Can't create a temporary file for capturing stdout\n");
      return false;
    }
    queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(datalen),
                               cl::NullRange, NULL, NULL);
    queue.finish();

    string out = GetCapturedStdout();
    if (compare_kernel_output(EXPECTED_OUTPUT, out))
      return true;
    else {
      cout << "FAIL: kernel output verification failed" << endl;
      cout << "Expected:\n" << EXPECTED_OUTPUT << "------------\n";
      cout << "Got:\n" << out << "------------\n";
      return false;
    }
    cout << "Captured stdout:\n" << out << endl;
  } catch (cl::Error err) {
    cout << "FAIL: " << err.what() << "(" << err.err() << ")" << endl;

    fprintf(stderr, "ClErrTxt error: %s\n", ClErrTxt(err.err()));
    return false;
  }

  if (DEBUG) {
    printf("-----------------------------------------\n");
    printf("And now back to the console once again\n");
  }

  return true;
}
