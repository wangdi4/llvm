#include "ClangUtils.h"
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <cctype>
#include "cl_device_api.h"

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
  llvm::errs() << "unhandled type " << type << "\n";
  assert (0 && "unrecognized type");
  return "";
}

//builds the given code to a file with a given name
void build(const std::string& code, std::string fileName){
  const char* clangpath = XSTR(CLANG_BIN_PATH);
  const char* options = "-cc1 -emit-llvm -include opencl_.h -opencl-builtins";
  const char* include_dir = XSTR(CLANG_INCLUDE_PATH);
  const char* tmpfile = "tmp.cl";
  assert(fileName != tmpfile && "tmp.cl is reserved!");
  //writing the cl code to the input file
  std::string errInfo;
  llvm::raw_fd_ostream input(tmpfile, errInfo);
  input << code;
  input.close();
  //building the command line
  std::stringstream cmdline;
  cmdline << clangpath << " " << options << " -o " << fileName << " -I " << include_dir << " " << tmpfile;
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
  sstream << "(" << type;
  if (veclen > 1)
    sstream << veclen;
   sstream << ")" << " (" << zeroLiteral;
  for (size_t i = 1 ; i<veclen ; i++)
    sstream << "," << zeroLiteral;
  sstream << ");}";
  return sstream.str();
}
