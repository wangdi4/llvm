/******************************************************************************
Copyright (c) Intel Corporation (2012,2013).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN AS IS BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name: OclBuiltinsHeaderGen.cpp

\*****************************************************************************/

#include "OclBuiltinsHeaderGen.h"
#include "OclBuiltinEmitter.h"
#include "CodeFormatter.h"
#include "cl_device_api.h"
#include "llvm/TableGen/Record.h"
#include "llvm/Module.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/LLVMContext.h"
#include "llvm/Support/IRReader.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Module.h"
#include <vector>
#include <sstream>
#include <memory>
#include <cstdio>

#define XSTR(A) STR(A)
#define STR(A) #A

namespace llvm{

OclBuiltinsHeaderGen::OclBuiltinsHeaderGen(RecordKeeper& rk)
:m_recordKeeper(rk){
}

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
static void build(const std::string& code, std::string fileName){
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

//
//MangledNameEmmiter: emits an entry in the mangled names array
//
struct MangledNameEmmiter{
  MangledNameEmmiter(CodeFormatter& s): m_formatter(s){
    m_formatter << "const char* mangledNames[] = {";
    m_formatter.endl();
    m_formatter.indent();
    //enable double extentions in clang
    m_code.append("#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n");
  }

  ~MangledNameEmmiter(){
    llvm::Module* pModule = NULL;
    const char* fileName = "builtins.ll";
    llvm::SMDiagnostic errDiagnostic;
    llvm::LLVMContext context;
    build(m_code, fileName);
    pModule = llvm::ParseIRFile(fileName, errDiagnostic, context);
    assert(pModule && "module parsing failed");
    //deleting the temporary output file
    remove(fileName);
    llvm::Module::const_iterator it = pModule->begin(), e = pModule->end();
    assert(pModule && "null llvm module");
    assert(it != e && "module contains no functions!" );
    int biCounter = 0;
    while (it != e){
      m_formatter << "\"" << it->getName() << "\"" << ",//" << biCounter++;
      m_formatter.endl();
      ++it;
    }
    m_formatter.unindent();
    m_formatter << "}; //end mangled names";
    m_formatter.endl();
    //generating the corresponding BI prototypes array
    m_formatter << "const char* prototypes[] = {";
    m_formatter.endl();
    m_formatter.indent();
    std::list<std::string>::const_iterator lit = m_protorypes.begin(),
    le = m_protorypes.end();
    biCounter = 0;
    while(lit != le){
      m_formatter << "\"" << *lit++ << "\",//" << biCounter++;
      m_formatter.endl();
    }
    m_formatter.unindent();
    m_formatter << "};//end prototypes;";
    m_formatter.endl();
  }

  //generates the prototype of a fucntion as string
  std::string getPrototype(const llvm::OclBuiltin* bi, const std::string& type){
    std::string ret;
    ret.append(bi->getCFunc(type));
    ret.append("(");
    size_t argumentNum = bi->getNumArguments();
    if (argumentNum >0){
      for(unsigned i=0 ; i < argumentNum-1 ; i++){
        ret.append(bi->getArgumentCType(i, type));
        ret.append(", ");
      }
      ret.append(bi->getArgumentCType(argumentNum-1, type));
    }
    ret.append(")");
    return ret;
  }

  virtual void operator () (const std::pair<std::string, llvm::OclBuiltin*>& it){
    const OclBuiltin* pBuiltin = it.second;
    if (pBuiltin->isSvml() || !pBuiltin->isOverlodable())
      return;
    OclBuiltin::const_type_iterator typeIter, typeEnd = pBuiltin->type_end();
    //creating a function descriptor array
    for (typeIter = pBuiltin->type_begin() ; typeIter != typeEnd; ++typeIter){
      //mangled name
      std::string biName = (*typeIter)->getName();
      std::string bi = pBuiltin->getCProto(biName);
      m_protorypes.push_back(getPrototype(pBuiltin, biName));
      bi += generateDummyBody(
        pBuiltin->getReturnBaseCType(biName),
        pBuiltin->getReturnVectorLength(biName)
      );
      m_code.append(bi);
      m_code.append("\n");
    }
  }

protected:

  //generates 'dummy code' (which does nothing but lets the module compile)
  std::string generateDummyBody(const std::string& type, size_t veclen)const{
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

  //holds the code with the builtins declaration, and dummy body
  std::string m_code;

  //contains the prototype of all the builtin functions
  std::list<std::string> m_protorypes;

  //stream to which code should be generated
  CodeFormatter& m_formatter;
};

void OclBuiltinsHeaderGen::run(raw_ostream& stream){
  OclBuiltinDB bidb(m_recordKeeper);
  CodeFormatter formatter(stream);
  formatter << "#ifndef __MANGLED_BI_NAMES_H__";
  formatter.endl();
  formatter << "#define __MANGLED_BI_NAMES_H__";
  formatter.endl();
  {
    MangledNameEmmiter mangleEmmiter(formatter);
    OclBuiltinDB::const_proto_iterator i = bidb.proto_begin(),
    e = bidb.proto_end();
    while(i != e){
      mangleEmmiter(*i);
      ++i;
    }
  }
  formatter.endl();
  formatter << "#endif//__MANGLED_BI_NAMES_H__";
  formatter.endl();
}

}
