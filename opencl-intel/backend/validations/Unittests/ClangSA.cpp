/*****************************************************************************\

Copyright (c) Intel Corporation (2011).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name: ClangSA.cpp
\*****************************************************************************/

#include <gtest/gtest.h>
#include <string>
#include <sstream>
#include <string.h>
#include "OCLBuilder.h"

using namespace Intel::OpenCL::DeviceBackend::Utils;
using namespace Intel::OpenCL::FECompilerAPI;
using namespace Validation;

static std::string buildLibName (const char* s){
  std::stringstream ret;
#ifdef _WIN32
  ret << s << ".dll";
#else
  ret << "lib" << s << ".so";
#endif
  return ret.str();
}

TEST(ClangStadalone, instance_creation){
// this test crashes, so it is disabled until CSSD100013412 will be fixed
  std::string clangLib = buildLibName("clang_compiler");
  const char* source =   "__kernel void add (__global const int *a, __global const int *b, __global int *c){"
  "int tid = get_global_id(0);"
  "c[tid] = b[tid] + a[tid];"
  "}";
  OCLBuilder& builder = OCLBuilder::instance();
  IOCLFEBinaryResult* binaryResult =
    builder.withSource(source).withLibrary(clangLib.c_str()).build();
  ASSERT_TRUE(binaryResult);
  builder.close();
}
