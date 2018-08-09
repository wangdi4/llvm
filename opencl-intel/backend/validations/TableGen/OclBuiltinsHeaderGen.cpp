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
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
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

static std::string deleteSuffix(const std::string& oldName)
{
    std::string ret = oldName;
    size_t pos;

    // we need to delete
    if((pos = oldName.find("_g2l"))!=std::string::npos)
        ret.erase(pos,4);
    else if((pos = oldName.find("_l2g"))!=std::string::npos)
        ret.erase(pos,4);

    return ret;
}



static std::string processFunctionName(const std::string& oldName)
{
    std::string ret = oldName;
    size_t pos;

    std::string readImage = "read_image";
    std::string writeImage = "write_image";
    
    if((pos= oldName.find(readImage, 0))==0) {
        return readImage;
    }
    if((pos= oldName.find(writeImage, 0))==0) {
        return writeImage;
    }

    if((pos= oldName.find("native_", 0, 7))==0) //found at the string begin
        ret.erase(0,7);
    else if((pos= oldName.find("half_", 0, 5))==0)//half
        ret.erase(0,5);
    else if((pos= oldName.find("fast_", 0, 5))==0)//fast
        ret.erase(0,5);
    else if((pos= oldName.find("atom_", 0, 5))==0)//fast 
        ret.replace(0,5,"atomic_");

    // half_ or native_ is already deleted, replace divide by div
    if((pos= ret.find("divide", 0, 6))==0)
        ret.replace(0,6,"div");

    //replace vloada and vstorea
    if((pos= oldName.find("vloada", 0, 6))==0)
        ret.replace(0,6,"vload");
    else if((pos= oldName.find("vstorea", 0, 7))==0)
        ret.replace(0,7,"vstore");

    //delete all possible numbers in vload/vstore functions
    if((oldName.find("vload", 0, 5)==0)||(oldName.find("vstore", 0, 6)==0))
    {
        char numbers[]="123468";
        for(uint32_t i =0; i<strlen(numbers);++i)
            ret.erase(std::remove(ret.begin(), ret.end(), numbers[i]), ret.end());
    }

    if((oldName.find("convert", 0, 7)==0))
        ret = "convert";

    return ret;
}




static std::string getRefFunction(const llvm::OclBuiltin* bi, const std::string& type){
    std::string ret;
    ret.append("lle_X_");

    std::string cFuncRef = bi->getCFuncRef(type);

    std::string funcName = bi->getName();
    std::string asyncFuncName = "async_work_group_strided_copy";
    std::string getImageDim2 = "get_image_dim_2d";
    std::string getImageDim3 = "get_image_dim_3d";
    std::string samplerless = "_samplerless";

    // we have two functions async_work_group_strided_copy, distinguishied by memory access qualifiers only
    // so, we add suffixes _g2l and _l2g to the basic function name, but only here, after .ll code generated,
    // because if we add suffixes before generation of .ll code, we will get wrong mangled names
    if(( funcName.find(asyncFuncName)) != std::string::npos ) {
        ret.append(processFunctionName(funcName));
    } else {
        // we need to get different names for reference functions for 2d and 3d images, but we need to have
        // the same name before generation of .ll code, so we add "2" or "3" here
        if(( funcName.find(getImageDim2)) != std::string::npos ) {        
            cFuncRef.append("2");
        } else if(( funcName.find(getImageDim3)) != std::string::npos ) {
            cFuncRef.append("3");
        }
        std::string tempFuncName = processFunctionName(cFuncRef);
        if(funcName.find(samplerless) != std::string::npos ) {
             tempFuncName.append(samplerless);
        }

        if((tempFuncName.find("vstore_half_",0,12)==0))
        {
            std::string typeSuffix=bi->getArgumentBaseCType(0,type);
            typeSuffix.erase(1, typeSuffix.size()-1);//delete all exept first letter
            if(typeSuffix=="f"||typeSuffix=="d")//float or double
                tempFuncName.replace(0,12, std::string("vstore")+typeSuffix+"_half_");//rebuild name
        }

        ret.append(tempFuncName);
    }

    size_t ruleCount = bi->getTemplateLength();
    if(ruleCount!=0)
    {
        //apply rule
        ret.append("<");
        for(unsigned i = 0; i<ruleCount ; ++i)
        {
            ret.append(bi->getTemplate(i, type));
            if(i < ruleCount-1)
                 ret.append(",");
        }
        ret.append(">");
    }
    return ret;
}

//
//MangledNameEmmiter: emits an entry in the mangled names array
//
struct MangledNameEmmiter{
  MangledNameEmmiter(CodeFormatter& s): m_formatter(s){
    m_formatter.indent();
    m_formatter.endl();
  }

typedef std::pair<const OclBuiltin*,std::string> TypedBi;
typedef std::list<TypedBi> TypedBiList;
typedef TypedBiList::const_iterator TypedBiIter;

  ~MangledNameEmmiter(){
    std::string err;
    llvm::SmallString<128> fileName;
    llvm::sys::fs::createUniqueFile("builtins-%%%%%%%.ll", fileName);
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
    std::string code = "#pragma OPENCL EXTENSION cl_khr_depth_images : enable\n";
    typedbiList.sort(isLess);
    TypedBiIter typeit, typee = typedbiList.end();
    for(typeit = typedbiList.begin(); typeit != typee ; ++typeit)
      code += generateBuiltinOverload(typeit->first, typeit->second);
    build(code, fileName.str());
    std::unique_ptr<llvm::Module> pModule = llvm::parseIRFile(fileName.str(), errDiagnostic, context);
    assert(pModule && "module parsing failed");
    //deleting the temporary output file
    remove(fileName.c_str());
    llvm::Module::const_iterator it = pModule->begin(), e = pModule->end();
    assert(pModule && "null llvm module");
    //assert(it != e && "module contains no functions!" );

    typeit = typedbiList.begin();
    assert(pModule->size() == typedbiList.size() && "number of builded functions does not match builtin list");
    int biCounter = 0;
    while (it != e&& typeit != typee ){
        m_formatter << "BUILTINS_API llvm::GenericValue lle_X_" << deleteSuffix(it->getName()) 
            << "( llvm::FunctionType *FT, llvm::ArrayRef<llvm::GenericValue> Args) { return " 
            << getRefFunction(typeit->first, typeit->second) << "(FT,Args);}//" << biCounter++;
      m_formatter.endl();
      ++it;
      ++typeit;
    }
    m_formatter.unindent();
    m_formatter.endl();
  }

  virtual void operator () (const std::pair<std::string, llvm::OclBuiltin*>& it){
    const OclBuiltin* pBuiltin = it.second;
    if (pBuiltin->isSvml() || !pBuiltin->isOverlodable())
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

    // hack for functions async_work_group_coopy and async_work_group_strided_copy
    if(( ret.find("async_work_group_")) != std::string::npos ) {
        size_t pos1 = ret.find("__local");
        if(pos1 != std::string::npos) {
            size_t pos2 = ret.find("__local",pos1+1,7);
            if(pos2 != std::string::npos) {
                ret.replace(pos2, 7, "const __global");
            }
        } else {
            pos1 = ret.find("__global");
            if(pos1 != std::string::npos) {
               size_t pos2 = ret.find("__global",pos1+1,8);
               if(pos2 != std::string::npos) {
                   ret.replace(pos2, 8, "const __local");
               }
            }
        }
    }

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



static std::string
ConvertCRLF2LF(const std::string& text)
{
  std::string ret;

  size_t cpos = 0, npos;
  do {
    npos = text.find("\r\n", cpos);

    if (npos != std::string::npos) {
      ret += text.substr(cpos, npos - cpos);
      ret += "\n"; // replace with LF
      npos += 2; // skip CRLF
    } else
      ret += text.substr(cpos);

    if (std::string::npos == npos)
      npos = text.size();
    cpos = npos;
  } while (cpos < text.size());

  return ret;
}

static std::string RemoveCommonLeadingSpaces(const std::string& t)
{
  std::string text = ConvertCRLF2LF(t);

  // How the common leading space is calculated:
  // - if a line has only space and ends with a new line, it's not considered
  //   during leading space calculation.
  // - otherwise, its leading space is counted as the consecutive spaces (only
  //   ' ') from the beginning and the common leading space is the minimal one.

  size_t common_leading_spaces = 0;
  int lines_with_leading_spaces = 0;
  size_t cpos = 0, npos;
  do {
    npos = text.find("\n", cpos);

    size_t newline = 1;
    if (std::string::npos == npos) {
      npos = text.size();
      newline = 0;
    }

    size_t lws = 0;
    while (cpos < npos && ' ' == text[cpos]) {
      ++lws;
      ++cpos;
    }

    if (cpos < npos || !newline) {
      // Only consider line with non-space characters.
      if (lines_with_leading_spaces)
        common_leading_spaces = std::min(common_leading_spaces, lws);
      else
        common_leading_spaces = lws;
      lines_with_leading_spaces++;
    }

    cpos = npos + newline; // Try next line.
  } while (cpos < text.size());

  // Only eligible line is found or the common leading space is 0.
  if (0 == common_leading_spaces)
    return text;

  std::string ret;
  cpos = 0;
  do {
    npos = text.find("\n", cpos);

    size_t newline = 1;
    if (std::string::npos == npos) {
      npos = text.size();
      newline = 0;
    }

    if ((npos - cpos) > common_leading_spaces)
      ret += text.substr(cpos + common_leading_spaces, npos - cpos - common_leading_spaces);

    if (newline)
      ret += "\n";

    cpos = npos + newline;
  } while (cpos < text.size());

  return ret;
}

void OclBuiltinsHeaderGen::run(raw_ostream& stream){
  OclBuiltinDB bidb(m_recordKeeper);
  emitSourceFileHeader("Reference OpenCL Builtins", stream);
  CodeFormatter formatter(stream);

  formatter << RemoveCommonLeadingSpaces(bidb.getProlog());
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

  formatter << RemoveCommonLeadingSpaces(bidb.getEpilog());
  formatter.endl();

}

}
