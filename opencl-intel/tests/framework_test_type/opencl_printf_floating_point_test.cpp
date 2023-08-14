/// Differences between OpenCL C and C99 printf:
/// The conversion specifiers f, F, e, E, g, G, a, A convert a float argument
/// to a double only if the double data type is supported. Refer to the
/// description of CL_DEVICE_DOUBLE_FP_CONFIG. If the double data type is not
/// supported, the argument will be a float instead of a double.

/// Test that printf correctly displays floating point numbers in both cases:
/// double data type is supported and is not supported. Unsupported double data
/// type is emulated by disabling cl_khr_fp64 extension. We can disable the
/// extension in version of OpenCL 1.1 (that is why clBuildProgram is called
/// with -cl-std=CL1.1). Starting from OpenCL 1.2 cl_khr_fp64 is an optional
/// core feature and can not be disabled by the user.

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
#include <iostream>
#include <string>

using namespace std;

extern cl_device_type gDeviceType;

namespace {
const char *KERNEL_CODE_STR =
    "#define print(arg)"
    "  printf(\"%f %F %.6f \", NAN, NAN, arg);"
    "  printf(\"%.6e %.6E \", arg, arg);"
    "  printf(\"%.6g \", arg);"
    "  printf(\"%.6A %.6a\\n\", arg, arg );\n"

    "__kernel void hello()"
    "{\n"
    "  const float pi_f = 3.141593f;\n"
    "  print(pi_f);\n"
    "  #pragma OPENCL EXTENSION cl_khr_fp64 : disable\n"
    "  print(pi_f);\n"
    "}\n";

const char *EXPECTED_OUTPUT = "nan NAN 3.141593 3.141593e+00 3.141593E+00 "
                              "3.14159 0X1.921FB8P+1 0x1.921fb8p+1\n"
                              "nan NAN 3.141593 3.141593e+00 3.141593E+00 "
                              "3.14159 0X1.921FB8P+1 0x1.921fb8p+1\n";

} // namespace

bool opencl_printf_floating_point_test() {
  cl_int err = CL_SUCCESS;
  string kernel_code = KERNEL_CODE_STR;

  cout << "---------------------------------------\n";
  cout << "opencl_printf_floating_point_test\n";
  cout << "---------------------------------------\n";

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

    if (CL_SUCCESS != program_.build(devices, "-cl-std=CL1.2")) {
      string buildlog;
      program_.getBuildInfo(devices[0], CL_PROGRAM_BUILD_LOG, &buildlog);
      cout << "FAIL: Build log:\n" << buildlog << endl;
      throw err;
    }

    cl::Kernel kernel(program_, "hello", &err);

    if (!CaptureStdout()) {
      cout << "Can't create a temporary file for capturing stdout\n";
      return false;
    }
    queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(1),
                               cl::NullRange, NULL, NULL);
    queue.finish();

    string out = GetCapturedStdout();
    if (!compare_kernel_output(EXPECTED_OUTPUT, out)) {
      cout << "FAIL: kernel output verification failed" << endl
           << "Expected:\n"
           << EXPECTED_OUTPUT << "------------\n"
           << "Got:\n"
           << out << "------------\n";
      return false;
    }
    return true;
  } catch (cl::Error err) {
    cout << "FAIL: " << err.what() << "(" << err.err() << ")" << endl;
    cout << "ClErrTxt error: " << ClErrTxt(err.err()) << endl;
    return false;
  }
  return true;
}
