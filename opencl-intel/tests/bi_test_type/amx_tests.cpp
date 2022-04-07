#include "bi_tests.h"
#include "common_utils.h"

#include <fstream>
#include <vector>

// This test is used to check whether AMX tile builtins can be compiled without
// error.

// sycl_amx_builtin.spv is generated from the following sycl program
// "sycl_amx_builtin.cpp" via the command:
//   clang++ -fsycl-device-only -fno-sycl-use-bitcode -Xclang
//   -fsycl-unnamed-lambda -std=c++14 sycl_amx_builtin.cpp -o
//   sycl_amx_builtin.spv

// clang-format off
/*
#include <CL/sycl.hpp>
#include <immintrin.h>
SYCL_EXTERNAL extern "C" _tile1024i
_tileloadd64_internal(short row, short col, char *buf, size_t stride);
SYCL_EXTERNAL extern "C" void _tilestored64_internal(short row, short col,
                                                           char *buf,
                                                           size_t stride,
                                                           _tile1024i tile);
SYCL_EXTERNAL extern "C" _tile1024i
_tdpbssd_internal(unsigned short m, unsigned short n, unsigned short k,
                        _tile1024i dst, _tile1024i src1, _tile1024i src2);
SYCL_EXTERNAL extern "C" _tile1024i
_tdpbsud_internal(unsigned short m, unsigned short n, unsigned short k,
                        _tile1024i dst, _tile1024i src1, _tile1024i src2);
SYCL_EXTERNAL extern "C" _tile1024i
_tdpbusd_internal(unsigned short m, unsigned short n, unsigned short k,
                        _tile1024i dst, _tile1024i src1, _tile1024i src2);
SYCL_EXTERNAL extern "C" _tile1024i
_tdpbuud_internal(unsigned short m, unsigned short n, unsigned short k,
                        _tile1024i dst, _tile1024i src1, _tile1024i src2);
SYCL_EXTERNAL extern "C" _tile1024i
_tdpbuud_internal(unsigned short m, unsigned short n, unsigned short k,
                        _tile1024i dst, _tile1024i src1, _tile1024i src2);
SYCL_EXTERNAL extern "C" _tile1024i
_tdpbf16ps_internal(unsigned short m, unsigned short n, unsigned short k,
                          _tile1024i dst, _tile1024i src1, _tile1024i src2);
#ifndef __SYCL_DEVICE_ONLY__
SYCL_EXTERNAL extern "C" _tile1024i
_tileloadd64_internal(short row, short col, char *buf, size_t stride) {}
SYCL_EXTERNAL extern "C" void _tilestored64_internal(short row, short col,
                                                           char *buf,
                                                           size_t stride,
                                                           _tile1024i tile) {}
SYCL_EXTERNAL extern "C" _tile1024i
_tdpbssd_internal(unsigned short m, unsigned short n, unsigned short k,
                        _tile1024i dst, _tile1024i src1, _tile1024i src2) {}
SYCL_EXTERNAL extern "C" _tile1024i
_tdpbsud_internal(unsigned short m, unsigned short n, unsigned short k,
                        _tile1024i dst, _tile1024i src1, _tile1024i src2) {}
SYCL_EXTERNAL extern "C" _tile1024i
_tdpbusd_internal(unsigned short m, unsigned short n, unsigned short k,
                        _tile1024i dst, _tile1024i src1, _tile1024i src2) {}
SYCL_EXTERNAL extern "C" _tile1024i
_tdpbuud_internal(unsigned short m, unsigned short n, unsigned short k,
                        _tile1024i dst, _tile1024i src1, _tile1024i src2) {}
SYCL_EXTERNAL extern "C" _tile1024i
_tdpbf16ps_internal(unsigned short m, unsigned short n, unsigned short k,
                          _tile1024i dst, _tile1024i src1, _tile1024i src2) {}
#endif
#define N 1024
using namespace cl::sycl;
int main() {
  queue Q;
  int *shared_array = malloc_shared<int>(N, Q);
  for (int i = 0; i < N; i++) {
    shared_array[i] = i;
  }
  Q.submit([&](handler &h) {
     h.parallel_for(N, [=](id<1> i) {
       char buf[512];
       memset(buf, 0, sizeof(buf));
       _tile1024i tile =
           _tileloadd64_internal(8, 8, (char *)shared_array, 64);
       tile = _tdpbssd_internal(8, 8, 8, tile, tile, tile);
       tile = _tdpbsud_internal(8, 8, 8, tile, tile, tile);
       tile = _tdpbusd_internal(8, 8, 8, tile, tile, tile);
       tile = _tdpbusd_internal(8, 8, 8, tile, tile, tile);
       tile = _tdpbf16ps_internal(8, 8, 8, tile, tile, tile);
       _tilestored64_internal(8, 8, (char *)shared_array, 128, tile);
     });
   }).wait();
}
*/
// clang-format on

class AMXTest : public BITest {
protected:
  virtual void SetUp() {
    SETENV("CL_CONFIG_CPU_TARGET_ARCH", "sapphirerapids");
    BITest::SetUp();
    std::string filename = get_exe_dir() + "sycl_amx_builtin.spv";
    ASSERT_NO_FATAL_FAILURE(readBinary(filename, spvBinary));
  }
  std::vector<unsigned char> spvBinary;
};

// AMX builtins are not available on 32 bit OS
#if defined(_WIN32) && !defined(_WIN64)
TEST_F(AMXTest, DISABLED_AMXBuiltins) {
#else
TEST_F(AMXTest, AMXBuiltins) {
#endif
  cl_int error = CL_SUCCESS;

  cl_program program = clCreateProgramWithIL(context, spvBinary.data(),
                                             spvBinary.size(), &error);
  ASSERT_EQ(CL_SUCCESS, error) << "clCreateProgramWithIL failed";

  error = clBuildProgram(program, 0, nullptr, "", nullptr, nullptr);
  EXPECT_EQ(CL_SUCCESS, error) << "clBuildProgram failed";

  if (CL_SUCCESS != error) {
    size_t log_size = 0;
    error = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0,
                                  nullptr, &log_size);
    ASSERT_EQ(CL_SUCCESS, error) << "clGetProgramBuildInfo failed";

    std::string log("", log_size);
    error = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
                                  log_size, &log[0], nullptr);
    ASSERT_EQ(CL_SUCCESS, error) << "clGetProgramBuildInfo failed";
    FAIL() << log << "\n";
  }
}