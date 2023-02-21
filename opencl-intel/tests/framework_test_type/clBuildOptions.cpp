// Copyright (c) 2015 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_TARGET_OPENCL_VERSION 200

#include "CL/opencl.hpp"
#include "FrameworkTest.h"
#include "cl_types.h"
#include "gtest_wrapper.h"
#include <stdio.h>

/**
 * In this test the kernel is built with a macro defined through 'options'
 * argument of clBuildProgram(). These options are parsed by common clang and
 * passed to the preprocessor. Purpose of the test to ensure that given options
 * are parsed correctly and match with expected value.
 */

extern cl_device_type gDeviceType;

void clBuildOptionsTest() {
  using std::make_pair;
  using std::vector;
  std::cout << "clBuildOptionsTest\n";
  std::string source("#define xstr(s) str(s)\n\
         #define str(s) #s\n\
         kernel void clBuildOptionsTest(global char *result,\
                                        unsigned int resultCapacity,\
                                        global unsigned int* resultSize)\
         {\
             constant char *s = xstr(msg);\
             unsigned int cnt = 0;\
             while( s[cnt] != 0 && cnt < resultCapacity ) \
             {\
                 result[cnt] = s[cnt];\
                 cnt++;\
             }\
             result[cnt] = '\\0';\
             *resultSize=++cnt;\
         }");

  vector<std::pair<const char *, const char *>> options;
  options.push_back(make_pair("", "msg"));
  options.push_back(make_pair("-Dmsg", "1"));
  options.push_back(make_pair("-Dmsg=", ""));
  options.push_back(make_pair("-Dmsg=x", "x"));
  options.push_back(make_pair("-Dmsg=\"x\"", "x"));
  options.push_back(make_pair("-Dmsg=\\\"x\\\"", "\"x\""));
  options.push_back(make_pair("-Dmsg=\"\\\"x\\\"\"", "\"x\""));
  options.push_back(make_pair("-Dmsg=\\\\\\\"x\\\\\\\"", "\\\"x\\\""));
  options.push_back(make_pair("-Dmsg=\"x y\"", "x y"));
  options.push_back(make_pair("-Dmsg=\"\\\"x y\\\"\"", "\"x y\""));
  options.push_back(make_pair("-Dmsg=\"\\\\\\\"x y\\\\\\\"\"", "\\\"x y\\\""));
  options.push_back(
      make_pair("-Dmsg=\"\\\"\\\\\\\"x y\\\\\\\"\\\"\"", "\"\\\"x y\\\"\""));

  cl_uint capacity = 128;
  vector<char> resultBuffer(capacity);
  cl_uint resultSize = 0;
  vector<cl::Platform> platforms;
  vector<cl::Device> devices;
  try {
    cl::Platform::get(&platforms);
    platforms[0].getDevices(gDeviceType, &devices);
    cl::Context context(devices);
    cl::CommandQueue queue(context, devices[0]);
    cl::Buffer resultBuf(context, CL_MEM_WRITE_ONLY, capacity);
    cl::Buffer resultSizeBuf(context, CL_MEM_WRITE_ONLY, sizeof(resultSize));
    cl::Program program(context, source, /*call clBuildProgram = */ false);

    for (size_t i = 0; i < options.size(); ++i) {
      try {
        program.build(devices, options[i].first);
      } catch (cl::Error &e) {
        if (e.err() == CL_BUILD_PROGRAM_FAILURE) {
          FAIL() << "clBuildProgram failed. Build log:\n"
                 << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0]);
        }
        throw;
      }
      cl::Kernel kernel(program, "clBuildOptionsTest");
      kernel.setArg(0, sizeof(resultBuf), &resultBuf);
      kernel.setArg(1, sizeof(capacity), &capacity);
      kernel.setArg(2, sizeof(resultSizeBuf), &resultSizeBuf);
      cl::NDRange global(1);
      queue.enqueueNDRangeKernel(kernel, cl::NullRange, global);
      queue.finish();
      queue.enqueueReadBuffer(resultSizeBuf, CL_TRUE, 0, sizeof(resultSize),
                              &resultSize);
      ASSERT_LE(resultSize, capacity);
      queue.enqueueReadBuffer(resultBuf, CL_TRUE, 0, resultSize,
                              &resultBuffer[0]);
      ASSERT_STREQ(&resultBuffer[0], options[i].second);
    }
  } catch (cl::Error &e) {
    FAIL() << "Call to " << e.what() << " failed: Error code = " << e.err()
           << '\n';
  }
  SUCCEED();
}
