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
#include "llvm/Support/Path.h"


static std::string getZeroLiteral(const std::string& type){
  if ("char" == type || "short" == type || "int" == type ||
      "uchar" == type || "ushort" == type || "uint" == type)
    return "0";
  if ("long" == type || "ulong" == type)
    return "0L";
  if ("float" == type)
    return "0.0f";
  if ("double" == type)
    return "0.0";

  if ("event_t" == type)
    return "0";
  if ("size_t" == type)
    return "0";


  llvm::errs() << "unhandled type " << type << "\n";
  assert (0 && "unrecognized type");
  return "";
}

//builds the given code to a file with a given name
void build(const std::string& code, std::string fileName){
  const char* clangpath = XSTR(CLANG_BIN_PATH);
  const char* include_dir = XSTR(CCLANG_INCLUDE_PATH);

  std::stringstream options;
  options << "-cc1 -emit-llvm-bc -include opencl-c.h -disable-intel-proprietary-opts -cl-std=CL2.0";
  options <<  " " << "-triple" << " "
      << ((sizeof(size_t)*8 == 64) ? "spir64-unknown-unknown" : "spir-unknown-unknown") << " ";

  llvm::SmallString<128> tmpfile;
  std::error_code ec;
  ec = llvm::sys::fs::createUniqueFile("tmp-%%%%%%%.cl", tmpfile);
  if( ec )
  {
      llvm::errs() << "couldn't generate unique name: " << ec.message() << "\n";
      exit(1);
  }
  assert(fileName != tmpfile.str() && "fileName is reserved!");

  //writing the cl code to the input file
  llvm::raw_fd_ostream input(tmpfile.c_str(), ec, llvm::sys::fs::FA_Write);
  if( ec )
  {
      llvm::errs() << "couldn't open a file " << tmpfile.str() << ": " << ec.message();
      exit(1);
  }
  input << code;
  input.close();

  //building the command line
  std::stringstream cmdline;
  cmdline << clangpath << " " << options.str() << " -o " << fileName << " -I " << include_dir << " " << tmpfile.c_str();
  int res = system(cmdline.str().c_str());
  if( res ){
    llvm::errs() << "bi compilation failed!\n";
    exit(1);
  }

  //deleting the temporary file
  remove(tmpfile.c_str());
}

//generates 'dummy code' (which does nothing but lets the module compile)
std::string generateDummyBody(const std::string& type, size_t veclen){
  std::stringstream sstream;

  // hack for functions async_work_group_coopy and async_work_group_strided_copy
  // returning event_t. Istead of {return event_t(0);}, the string
  // {return __builtin_astype((void *)0, event_t);} must be used
  if(( type.find("event_t")) != std::string::npos ) {
    sstream << "{return __builtin_astype((void *)0, event_t);}";
  } else {
    sstream << "{return ";
    if ("void" == type){
      sstream << ";} ";
      return sstream.str();
    }
    std::string zeroLiteral = getZeroLiteral(type);
    sstream << "(" << type;
    if (veclen > 1)
      sstream << veclen;
     sstream << ")" << " (" << zeroLiteral;
    for (size_t i = 1 ; i<veclen ; i++)
      sstream << "," << zeroLiteral;
    sstream << ");} ";
  }
  return sstream.str();
}
