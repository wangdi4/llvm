// INTEL CONFIDENTIAL
//
// Copyright 2011-2018 Intel Corporation.
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

#include "OCLBuilder.h"
#include "gtest_wrapper.h"
#include <sstream>
#include <string.h>
#include <string>

using namespace Intel::OpenCL::DeviceBackend::Utils;
using namespace Intel::OpenCL::FECompilerAPI;
using namespace Validation;

static std::string buildLibName(const char *s) {
  std::stringstream ret;
#ifdef _WIN32
  ret << s << ".dll";
#else
  ret << "lib" << s << ".so";
#endif
  return ret.str();
}

TEST(ClangStadalone, DISABLED_instance_creation) {
// this test crashes, so it is disabled until CSSD100013412 will be fixed
#if defined(_WIN32)
#if defined(_M_X64)
  std::string clangLib = buildLibName("clang_compiler64");
#else
  std::string clangLib = buildLibName("clang_compiler32");
#endif
#else
  std::string clangLib = buildLibName("clang_compiler");
#endif
  const char *source = "__kernel void add (__global const int *a, __global "
                       "const int *b, __global int *c){"
                       "int tid = get_global_id(0);"
                       "c[tid] = b[tid] + a[tid];"
                       "}";
  OCLBuilder &builder = OCLBuilder::Instance();
  IOCLFEBinaryResult *binaryResult =
      builder.withSource(source).withLibrary(clangLib.c_str()).build();
  ASSERT_TRUE(binaryResult);
  builder.close();
}
