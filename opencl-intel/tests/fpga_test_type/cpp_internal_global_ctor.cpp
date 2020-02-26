// Copyright (c) 2020 Intel Corporation
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

// internal-ctor-kernel.ioc.obj is created from:
//  command:
//    ./aocl-ioc64 -cmd=compile -device=fpga_fast_emu -bo='-cl-std=CL1.2
//    -cl-opt-disable -s "kernel.cl"  -DINTELFPGA_CL=201' -input=kernel.cl
//    -ir=internal-ctor-kernel.ioc.obj
//
//  kernel.cl:
//    int myfunc(int x);
//    kernel void test(__global int *out) {
//      out[0] = myfunc(0);
//    }
//
// internal-ctor-lib.cpp.ioc.obj is created from
//  command:
//    ./aocl-clang -cc1 -emit-llvm-bc -O3 -disable-llvm-passes -x c++
//    --std=c++14 -fhls -D_HLS_EMBEDDED_PROFILE -x c++ --std=c++14 -fhls
//    -D_HLS_EMBEDDED_PROFILE -D__INTELFPGA_TYPE__=NONE -DHLS_X86 -triple
//    spir64-unknown-unknown-intelfpga lib.cpp  -o internal-ctor-lib.cpp.ioc.obj
//    -Wunknown-pragmas -Wuninitialized -D__INTELFPGA_COMPILER__=201
//
//  lib.cpp:
//    struct glob{
//      volatile int val;
//      glob(int x) {val = x;}
//    };
//    struct glob globobj(4);
//    extern "C" int myfunc(int x){
//      return globobj.val + x;
//    }

#include "base_fixture.h"
#include "common_utils.h"
#include <fstream>
#include <iterator>

class CPPTest : public OCLFPGABaseFixture {};

static void testProgram(cl_context context, cl_command_queue queue,
                        cl_program program) {
  cl_int err;
  std::string kernelName = "test";
  cl_kernel kernel = clCreateKernel(program, kernelName.c_str(), &err);
  ASSERT_EQ(CL_SUCCESS, err) << "clCreateKernel failed";

  size_t globalSize = 1;
  cl_mem buffer = clCreateBuffer(context, CL_MEM_READ_WRITE,
                                 sizeof(cl_int) * globalSize, nullptr, &err);
  ASSERT_EQ(CL_SUCCESS, err) << "clCreateBuffer failed";

  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&buffer);
  ASSERT_EQ(CL_SUCCESS, err) << "clSetKernelArg failed";

  err = clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &globalSize,
                               &globalSize, 0, nullptr, nullptr);
  ASSERT_EQ(CL_SUCCESS, err) << "clEnqueueNDRangeKernel failed";

  std::vector<int> bufferHost(globalSize);
  err = clEnqueueReadBuffer(queue, buffer, CL_TRUE, 0,
                            sizeof(cl_int) * globalSize, &(bufferHost[0]), 0,
                            nullptr, nullptr);
  ASSERT_EQ(CL_SUCCESS, err) << "clEnqueueReadBuffer failed";

  // Check result is correct
  ASSERT_EQ(bufferHost[0], 4) << "Output mismatch";

  // Release resources
  err = clReleaseMemObject(buffer);
  ASSERT_EQ(CL_SUCCESS, err) << "clReleaseMemObject failed";
  err = clReleaseKernel(kernel);
  ASSERT_EQ(CL_SUCCESS, err) << "clReleaseKernel failed";
}

TEST_F(CPPTest, internalGlobalCtor) {
  cl_device_id deviceId = device();
  cl_context context = createContext(deviceId);
  ASSERT_NE(context, nullptr);
  cl_command_queue queue = createCommandQueue(context, deviceId);
  ASSERT_NE(queue, nullptr);

  // Create program from object files
  cl_int err;
  std::vector<std::string> filenames = {"internal-ctor-kernel.ioc.obj",
                                        "internal-ctor-lib.cpp.ioc.obj"};
  int num_programs = (int)filenames.size();
  std::vector<cl_program> programs(num_programs);
  for (int i = 0; i < num_programs; ++i) {
    std::ifstream objFile(get_exe_dir() + filenames[i], std::fstream::binary);
    ASSERT_TRUE(objFile.is_open()) << ("Unable to open file " + filenames[i]);
    std::vector<char> obj(std::istreambuf_iterator<char>(objFile), {});
    ASSERT_FALSE(obj.empty()) << (filenames[i] + " is empty");
    size_t size = obj.size();
    const unsigned char *binary = (const unsigned char *)&obj[0];
    cl_int status;
    programs[i] = clCreateProgramWithBinary(context, 1, &deviceId, &size,
                                            &binary, &status, &err);
    ASSERT_EQ(CL_SUCCESS, err) << "clCreateProgramWithBinary failed";
    ASSERT_EQ(CL_SUCCESS, status) << "clCreateProgramWithBinary failed";
  }

  // Link programs
  cl_program program1 =
      clLinkProgram(context, 1, &deviceId, nullptr, num_programs, &programs[0],
                    nullptr, NULL, &err);
  ASSERT_EQ(CL_SUCCESS, err) << "clLinkProgram failed";

  // Test program
  ASSERT_NO_FATAL_FAILURE(testProgram(context, queue, program1));

  // JIT save/load
  size_t binarySize;
  err = clGetProgramInfo(program1, CL_PROGRAM_BINARY_SIZES, sizeof(binarySize),
                         &binarySize, nullptr);
  ASSERT_EQ(CL_SUCCESS, err) << "clGetProgramInfo CL_PROGRAM_BINARY_SIZES";
  std::vector<unsigned char> binary(binarySize);
  const unsigned char *binaries[1] = {&binary[0]};
  err = clGetProgramInfo(program1, CL_PROGRAM_BINARIES, sizeof(binaries),
                         &binaries, nullptr);
  ASSERT_EQ(CL_SUCCESS, err) << "clGetProgramInfo CL_PROGRAM_BINARIES";

  cl_int status;
  cl_program program2 = clCreateProgramWithBinary(
      context, 1, &deviceId, &binarySize, binaries, &status, &err);
  ASSERT_EQ(CL_SUCCESS, err) << "clCreateProgramWithBinary";
  ASSERT_EQ(CL_SUCCESS, status) << "clCreateProgramWithBinary";
  err = clBuildProgram(program2, 1, &deviceId, nullptr, nullptr, nullptr);
  ASSERT_EQ(CL_SUCCESS, err) << "clBuildProgram failed";

  // Test program from JIT-load
  ASSERT_NO_FATAL_FAILURE(testProgram(context, queue, program2));

  // Release programs
  for (int i = 0; i < num_programs; ++i) {
    err = clReleaseProgram(programs[i]);
    ASSERT_EQ(CL_SUCCESS, err) << "clReleaseProgram failed";
  }
  err = clReleaseProgram(program1);
  ASSERT_EQ(CL_SUCCESS, err) << "clReleaseProgram failed";
  err = clReleaseProgram(program2);
  ASSERT_EQ(CL_SUCCESS, err) << "clReleaseProgram failed";
}
