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

#include "ClangUtils.h"
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <cctype>
#include "cl_device_api.h"

#include "llvm/Support/FileSystem.h"

static std::string getZeroLiteral(const std::string& type){
  if ("char" == type || "short" == type || "int" == type ||
      "uchar" == type || "ushort" == type || "uint" == type ||
      "bool" == type)
    return "0";
  else if ("long" == type || "ulong" == type)
    return "0L";
  else if ("float" == type)
    return "0.0f";
  else if ("double" == type)
    return "0.0";
  else if ("event_t" == type)
    //This is a work around:
    // 1. there is no zero value for event_t
    // 2. built-ins that need to return event_t has an argument of type event_t called "event".
    return "event";
  llvm::errs() << "unhandled type " << type << "\n";
  assert (0 && "unrecognized type");
  return "";
}

//builds the given code to a file with a given name
void build(const std::string& code, std::string fileName){
  const char* clangpath = XSTR(CLANG_BIN_PATH);
  const char* include_dir = XSTR(CCLANG_INCLUDE_PATH);

  std::stringstream options;
  options << "-cc1 -x cl -disable-intel-proprietary-opts -emit-llvm-bc -include opencl-c.h";
  options << " " << "-opencl-builtins -fblocks -cl-std=CL2.0 -D__OPENCL_C_VERSION__=200";
  options <<  " " << "-triple" << " "
      << ((sizeof(size_t)*8 == 64) ? "spir64-unknown-unknown" : "spir-unknown-unknown") << " ";

  const char* tmpfile = "tmp.cl";
  assert(fileName != tmpfile && "tmp.cl is reserved!");
  //writing the cl code to the input file
  std::error_code ec;
  llvm::raw_fd_ostream input(tmpfile, ec, llvm::sys::fs::FA_Write);
  if( ec )
  {
      llvm::errs() << "couldn't open a file " << tmpfile << ": " << ec.message();
      exit(1);
  }
  input << code;
  input.close();

  //building the command line
  std::stringstream cmdline;
  cmdline << clangpath << " " << options.str() << " -o " << fileName << " -I " << include_dir << " " << tmpfile;
  int res = system(cmdline.str().c_str());
  if( res ){
    llvm::errs() << "bi compilation failed!\n";
    exit(1);
  }

  //deleting the temporary file
  remove(tmpfile);
}

//generates 'dummy code' (which does nothing but lets the module compile)
std::string generateDummyBody(const std::string& type, size_t veclen){
  std::stringstream sstream;
  sstream << "{return ";
  if ("void" == type){
    sstream << ";}";
    return sstream.str();
  }
  std::string zeroLiteral = getZeroLiteral(type);
  if ("event_t" == type) {
    //Cannot cast to event_t type, just return the "ZeroLiteral" value.
    sstream << zeroLiteral << ";}";
  } else {
    sstream << "(" << type;
    if (veclen > 1)
      sstream << veclen;
     sstream << ")" << " (" << zeroLiteral;
    for (size_t i = 1 ; i<veclen ; i++)
      sstream << "," << zeroLiteral;
    sstream << ");}";
  }
  return sstream.str();
}
