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

#include "OclBuiltinsHeaderGen.h"
#include "OclBuiltinEmitter.h"
#include "CodeFormatter.h"
#include "ConversionParser.h"
#include "ClangUtils.h"
#include "cl_device_api.h"
#include "llvm/TableGen/Record.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/IR/Module.h"
#include <vector>
#include <sstream>
#include <memory>
#include <cstdio>
#include <cctype>

namespace llvm{

OclBuiltinsHeaderGen::OclBuiltinsHeaderGen(RecordKeeper& rk)
:m_recordKeeper(rk){
}

//generates the prototype of a fucntion as string
static std::string getPrototype(const llvm::OclBuiltin* bi, const std::string& type){
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

//
//MangledNameEmmiter: emits an entry in the mangled names array
//
struct MangledNameEmmiter{
  MangledNameEmmiter(CodeFormatter& s): m_formatter(s){
    m_formatter << "const char* mangledNames[] = {";
    m_formatter.endl();
    m_formatter.indent();
    m_formatter.endl();
  }

typedef std::pair<const OclBuiltin*,std::string> TypedBi;
typedef std::list<TypedBi> TypedBiList;
typedef TypedBiList::const_iterator TypedBiIter;

  ~MangledNameEmmiter(){
    const char* fileName = "builtins.ll";
    llvm::SMDiagnostic errDiagnostic;
    llvm::LLVMContext context;
    std::list<const OclBuiltin*>::const_iterator biit = m_builtins.begin(),
    bie = m_builtins.end();
    TypedBiList typedbiList;
    for(; biit != bie ; ++biit){
      OclBuiltin::const_type_iterator typeIter, typeEnd = (*biit)->type_end();
      for (typeIter=(*biit)->type_begin() ; typeIter!=typeEnd; ++typeIter){
        TypedBi typedBi( *biit, (*typeIter)->getName() );
        typedbiList.push_back( typedBi );
      }
    }
    //enable double extentions in clang
    std::string code;
    typedbiList.sort(isLess);
    TypedBiIter typeit, typee = typedbiList.end();
    OclBuiltinAttr IA = OclBuiltinAttr::CreateInilineAttribute();
    for(typeit = typedbiList.begin(); typeit != typee ; ++typeit){
      OclBuiltin *B = const_cast<OclBuiltin*>(typeit->first);
      B->removeAttribute(IA);
      code += generateBuiltinOverload(B, typeit->second).append("\n");
    }
    build(code, fileName);
    std::unique_ptr<llvm::Module> pModule = llvm::parseIRFile(fileName, errDiagnostic, context);
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
    m_formatter << "const char* UNUSED(prototypes[]) = {";
    m_formatter.endl();
    m_formatter.indent();
    biCounter = 0;
    for(typeit = typedbiList.begin(); typeit != typee ; ++typeit){
      m_formatter << "\"" << getPrototype(typeit->first, typeit->second) <<
      "\", //" << biCounter++;
      m_formatter.endl();
    }
    m_formatter.unindent();
    m_formatter << "};//end prototypes;";
    m_formatter.endl();
  }

  virtual void operator () (const std::pair<std::string, llvm::OclBuiltin*>& it){
    const OclBuiltin* pBuiltin = it.second;
    // BUGBUG: isBrokenNameMangling() is a temporary w/around for name mangling in-compat with SPIR (CQ CSSD100017714)
    if (pBuiltin->isSvml() || !pBuiltin->isOverlodable() || !pBuiltin->shouldGenerate() || pBuiltin->isBrokenNameMangling())
      return;
    m_builtins.push_back(pBuiltin);
  }

protected:
  static bool isConversion(const std::string& s){
    const std::string prefix("convert_");
    return (s.substr(0, prefix.length()) == prefix);
  }
  static bool isLess(const TypedBi& leftBi, const TypedBi& rightBi){
    size_t lindex , rindex;
    std::string left = getPrototype(leftBi.first, leftBi.second);
    std::string right = getPrototype(rightBi.first, rightBi.second);
    lindex = left.find('(');
    rindex = right.find('(');
    assert(lindex != std::string::npos && "illegal function prototype");
    assert(rindex != std::string::npos && "illegal function prototype");
    //is it a conversion function (conversion of the same type needs to be
    //grouped together)
    std::string lname = left.substr(0, lindex);
    std::string rname = right.substr(0, rindex);
    if (isConversion(lname) && isConversion(rname)){
      reflection::ConversionDescriptor ldesc(lname), rdesc(rname);
      return ldesc.compare(rdesc) < 0;
    }
    return lname.compare(rname) < 0;
  }

  std::string generateBuiltinOverload(const OclBuiltin* pBuiltin,
    const std::string& typeName){
    std::string ret = pBuiltin->getCProto(typeName);
    ret += generateDummyBody(
      pBuiltin->getReturnBaseCType(typeName),
      pBuiltin->getReturnVectorLength(typeName)
    );
    return ret;
  }

  //a list of built in function to be generated
  std::list<const OclBuiltin*> m_builtins;

  //stream to which code should be generated
  CodeFormatter& m_formatter;
};

const char* DESC=
"Ocl BuiltIn funcs data: mangled name array + corresponding prototype array";

void OclBuiltinsHeaderGen::run(raw_ostream& stream){
  OclBuiltinDB bidb(m_recordKeeper);
  emitSourceFileHeader(DESC, stream);
  CodeFormatter formatter(stream);
  formatter << "#ifndef __MANGLED_BI_NAMES_H__";
  formatter.endl();
  formatter << "#define __MANGLED_BI_NAMES_H__";
  formatter.endl();
  //
  //emit the UNUSED macro
  //
  formatter << "#if defined(_WIN32) || defined(_WIN64)";
  formatter.endl();
  formatter << "#define UNUSED(X) __pragma(warning(suppress:4100))X";
  formatter.endl();
  formatter << "#else";
  formatter.endl();
  formatter << "#define UNUSED(X) X __attribute__((unused))";
  formatter.endl();
  formatter << "#endif//UNUSED";
  formatter.endl();
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
