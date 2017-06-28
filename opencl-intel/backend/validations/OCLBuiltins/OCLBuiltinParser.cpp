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

File Name:  OCLBuiltinParser.cpp


\*****************************************************************************/

#define DEBUG_TYPE "OCLBuiltinParser"

#include "assert.h"
#include <map>
#include <sstream>
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Regex.h"
#include "llvm/ADT/SmallVector.h"

#include "Exception.h"
#include "OCLBuiltinParser.h"

#include "NameMangleAPI.h"
#include "ParameterType.h"
#include "FunctionDescriptor.h"

#include <iostream>

using namespace llvm;
using namespace Validation;

/// helper singleton class for storing map of string to basic type
class BasicTypeFromStr
{
public:
    BasicTypeFromStr()
    {
        if(m_Map.empty())
            initStatic();
    }
    
    OCLBuiltinParser::BasicArgType operator[](
        const std::string& str) const
    {
        std::map<std::string, OCLBuiltinParser::BasicArgType>::const_iterator 
                it = m_Map.find(str);
        if (it == m_Map.end())
            throw Exception::InvalidArgument("Basic type not detected");
        return it->second;
    }
private:
    void initStatic();
    static std::map<std::string, OCLBuiltinParser::BasicArgType> m_Map;
};

void BasicTypeFromStr::initStatic()
{
    m_Map["void"] = OCLBuiltinParser::VOID;
    m_Map["bool"] = OCLBuiltinParser::BOOL;
    m_Map["char"] = OCLBuiltinParser::CHAR;
    m_Map["uchar"] = OCLBuiltinParser::UCHAR;
    m_Map["short"] = OCLBuiltinParser::SHORT;
    m_Map["ushort"] = OCLBuiltinParser::USHORT;
    m_Map["int"] = OCLBuiltinParser::INT;
    m_Map["uint"] = OCLBuiltinParser::UINT;
    m_Map["long"] = OCLBuiltinParser::LONG;
    m_Map["ulong"] = OCLBuiltinParser::ULONG;
    m_Map["float"] = OCLBuiltinParser::FLOAT;
    m_Map["double"] = OCLBuiltinParser::DOUBLE;
    m_Map["image1d_t"] = OCLBuiltinParser::IMAGE_1D_T;
    m_Map["image2d_t"] = OCLBuiltinParser::IMAGE_2D_T;
    m_Map["image2d_depth_t"] = OCLBuiltinParser::IMAGE_2D_DEPH_T;
    m_Map["image3d_t"] = OCLBuiltinParser::IMAGE_3D_T;
    m_Map["image1d_buffer_t"] = OCLBuiltinParser::IMAGE_1D_BUFFER_T;
    m_Map["image1d_array_t"] = OCLBuiltinParser::IMAGE_1D_ARRAY_T;
    m_Map["image2d_array_t"] = OCLBuiltinParser::IMAGE_2D_ARRAY_T;
    m_Map["image2d_array_depth_t"] = OCLBuiltinParser::IMAGE_2D_ARRAY_DEPH_T;
    m_Map["event_t"] = OCLBuiltinParser::EVENT_T;
    m_Map["clk_event_t"] = OCLBuiltinParser::CLK_EVENT_T;
    m_Map["queue_t"] = OCLBuiltinParser::QUEUE_T;
    m_Map["pipe_t"] = OCLBuiltinParser::PIPE_T;
    m_Map["sampler_t"] = OCLBuiltinParser::SAMPLER_T;
}

std::map<std::string, OCLBuiltinParser::BasicArgType> BasicTypeFromStr::m_Map;

/// helper class for converting basic type to a string.
class BasicTypeToStr
{
public:
    BasicTypeToStr()
    {
        if(m_Map.empty())
            initStatic();
    }

    reflection::TypePrimitiveEnum operator[](
        const OCLBuiltinParser::BasicArgType& type) const
    {
        std::map<OCLBuiltinParser::BasicArgType, reflection::TypePrimitiveEnum >::const_iterator 
            it = m_Map.find(type);
        if (it == m_Map.end())
            throw Exception::InvalidArgument("Unknown basic type");
        return it->second;
    }

private:
    void initStatic();
    static std::map<OCLBuiltinParser::BasicArgType, reflection::TypePrimitiveEnum> m_Map;
};

void BasicTypeToStr::initStatic()
{
    m_Map[OCLBuiltinParser::BOOL]                   = reflection::PRIMITIVE_BOOL;
    m_Map[OCLBuiltinParser::CHAR]                   = reflection::PRIMITIVE_CHAR;
    m_Map[OCLBuiltinParser::UCHAR]                  = reflection::PRIMITIVE_UCHAR;
    m_Map[OCLBuiltinParser::SHORT]                  = reflection::PRIMITIVE_SHORT;
    m_Map[OCLBuiltinParser::USHORT]                 = reflection::PRIMITIVE_USHORT;
    m_Map[OCLBuiltinParser::INT]                    = reflection::PRIMITIVE_INT;
    m_Map[OCLBuiltinParser::UINT]                   = reflection::PRIMITIVE_UINT;
    m_Map[OCLBuiltinParser::LONG]                   = reflection::PRIMITIVE_LONG;
    m_Map[OCLBuiltinParser::ULONG]                  = reflection::PRIMITIVE_ULONG;
    m_Map[OCLBuiltinParser::FLOAT]                  = reflection::PRIMITIVE_FLOAT;
    m_Map[OCLBuiltinParser::DOUBLE]                 = reflection::PRIMITIVE_DOUBLE;
    m_Map[OCLBuiltinParser::IMAGE_1D_T]             = reflection::PRIMITIVE_IMAGE_1D_T;
    m_Map[OCLBuiltinParser::IMAGE_2D_T]             = reflection::PRIMITIVE_IMAGE_2D_T;
    m_Map[OCLBuiltinParser::IMAGE_2D_DEPH_T]        = reflection::PRIMITIVE_IMAGE_2D_DEPTH_T;
    m_Map[OCLBuiltinParser::IMAGE_3D_T]             = reflection::PRIMITIVE_IMAGE_3D_T;
    m_Map[OCLBuiltinParser::IMAGE_1D_BUFFER_T]      = reflection::PRIMITIVE_IMAGE_1D_BUFFER_T;
    m_Map[OCLBuiltinParser::IMAGE_1D_ARRAY_T]       = reflection::PRIMITIVE_IMAGE_1D_ARRAY_T;
    m_Map[OCLBuiltinParser::IMAGE_2D_ARRAY_T]       = reflection::PRIMITIVE_IMAGE_2D_ARRAY_T;
    m_Map[OCLBuiltinParser::IMAGE_2D_ARRAY_DEPH_T]  = reflection::PRIMITIVE_IMAGE_2D_ARRAY_DEPTH_T;
    m_Map[OCLBuiltinParser::EVENT_T]                = reflection::PRIMITIVE_EVENT_T;
    m_Map[OCLBuiltinParser::CLK_EVENT_T]            = reflection::PRIMITIVE_CLK_EVENT_T;
    m_Map[OCLBuiltinParser::QUEUE_T]                = reflection::PRIMITIVE_QUEUE_T;
    m_Map[OCLBuiltinParser::PIPE_T]                 = reflection::PRIMITIVE_PIPE_T;
    m_Map[OCLBuiltinParser::SAMPLER_T]              = reflection::PRIMITIVE_SAMPLER_T;
}

std::map<OCLBuiltinParser::BasicArgType, reflection::TypePrimitiveEnum> BasicTypeToStr::m_Map;

// template singleton class
// taken from source http://www.yolinux.com/TUTORIALS/C++Singleton.html
template <class T>
class Singleton
{
public:
        static T* Instance() {
                if(!m_pInstance) m_pInstance = new T;
                assert(m_pInstance !=NULL);
                return m_pInstance;
    }
protected:
        Singleton();
        ~Singleton();
private:
        Singleton(Singleton const&);
        Singleton& operator=(Singleton const&);
        static T* m_pInstance;
};
template <class T> T* Singleton<T>::m_pInstance=NULL;

// struct for storing name of builtin and its arguments
struct ArgPair
{
    std::string name;
    OCLBuiltinParser::ArgVector args;
};

/// map for caching results of OCLBuiltinParser
/// @param std::string - clang compiled LLVM string with builtin
///             like "_Z10native_sinDv4_f"
/// @param ArgPair - struct with already parsed builtin name and 
///             its arguments
typedef std::map<std::string, ArgPair> ArgVectorMap;

// singleton for storing ArgVectorMap
typedef Singleton<ArgVectorMap> ArgVectorMapSingleton;


  struct GetTypeVisitor: reflection::TypeVisitor {

    llvm::OCLBuiltinParser::ARG newArg;
    BasicTypeFromStr BasicTypeConvertor;

    llvm::OCLBuiltinParser::ARG getArg(){
        return newArg;
    }

    void visit(const reflection::PrimitiveType* p) {
      newArg.basicType = BasicTypeConvertor[p->toString()];
      newArg.genType = OCLBuiltinParser::BASIC;
    }

    void visit(const reflection::VectorType* p) {
      p->getScalarType()->accept(this);
      newArg.vecType.elType = newArg.basicType;
      newArg.vecType.elNum = p->getLength();
      newArg.genType = OCLBuiltinParser::VECTOR;
    }

    void visit(const reflection::PointerType* p) {

      llvm::OCLBuiltinParser::ARG pointerArg;

      p->getPointee()->accept(this);
      pointerArg = newArg;
      newArg.genType = OCLBuiltinParser::POINTER;
      newArg.ptrType.ptrType.push_back(pointerArg);
      newArg.ptrType.ptrToStr = p->toString();
    }

    void visit(const reflection::AtomicType* p) {
      assert(false && "need to support Atomic Parameter type");
    }

    void visit(const reflection::BlockType* p) {
      assert(false && "need to support Block Parameter type");
    }

    void visit(const reflection::UserDefinedType* p) {
      //user defined type, in OCL, its images
      std::string gotString = p->toString();
      size_t found_1d = gotString.find("image1d");
      size_t found_2d = gotString.find("image2d");
      size_t found_3d = gotString.find("image3d");
      size_t found_sampler = gotString.find("sampler");

      if(found_1d != std::string::npos || found_2d != std::string::npos || found_3d != std::string::npos) {
         newArg.genType = OCLBuiltinParser::IMAGE;
         newArg.imgType.imgStr = gotString;
      } else if (found_sampler != std::string::npos ) {
          newArg.genType = OCLBuiltinParser::SAMPLER;
      } else {
         newArg.genType = OCLBuiltinParser::NA;
      }
    }

  };



bool OCLBuiltinParser::ParseOCLBuiltin(const std::string& in_str, 
                                       std::string& out_FuncStr, 
                                       ArgVector& args)
{
  // check if we have parser results in cache ArgVectorMapSingleton
  ArgVectorMap::iterator it = ArgVectorMapSingleton::Instance()->find(in_str);
  if (it != ArgVectorMapSingleton::Instance()->end()) {
    out_FuncStr = (it->second).name;
    args = (it->second).args;
    return true;
  }

  if (isMangledName(in_str.c_str())) {
    // extract built in name and arguments
    reflection::FunctionDescriptor fd = demangle(in_str.c_str());
    std::string FuncNameStr = fd.name;

    // vector with arguments
    ArgVector al;

    if (fd.parameters.empty()) {
      throw Validation::Exception::InvalidArgument(
          "Unknown format of OCL built in argument");
    }

    for (int i = 0; i < (int)fd.parameters.size(); i++) {
      GetTypeVisitor typeVisitor;
      fd.parameters[i]->accept(&typeVisitor);
      ARG newArg = typeVisitor.getArg();

      if (newArg.genType == NA) {
        throw Validation::Exception::InvalidArgument(
            "Unknown format of OCL built in argument");
      }

      newArg.ptrType.isAddrSpace = false;
      newArg.ptrType.isPointsToConst = false;

      if (newArg.genType == POINTER) {

        if (newArg.ptrType.ptrType.back().genType == NA) {
          throw Validation::Exception::InvalidArgument(
              "Unknown format of OCL built in argument");
        }

        // find attributes : __global, __constant, etc
        reflection::PointerType *pPTy =
            reflection::dyn_cast<reflection::PointerType>(&*fd.parameters[i]);
        for (unsigned int j = 0; j < pPTy->getAttributes().size(); ++j) {
          switch (pPTy->getAttributes()[j]) {
          case reflection::ATTR_LOCAL:
            newArg.ptrType.isAddrSpace = true;
            newArg.ptrType.AddrSpace = OCLBuiltinParser::LOCAL;
            break;
          case reflection::ATTR_CONSTANT:
            newArg.ptrType.isAddrSpace = true;
            newArg.ptrType.AddrSpace = OCLBuiltinParser::CONSTANT;
            break;
          case reflection::ATTR_GLOBAL:
            newArg.ptrType.isAddrSpace = true;
            newArg.ptrType.AddrSpace = OCLBuiltinParser::GLOBAL;
            break;
          case reflection::ATTR_PRIVATE:
            newArg.ptrType.isAddrSpace = true;
            newArg.ptrType.AddrSpace = OCLBuiltinParser::PRIVATE;
            break;
          case reflection::ATTR_GENERIC:
            newArg.ptrType.isAddrSpace = true;
            newArg.ptrType.AddrSpace = OCLBuiltinParser::GENERIC;
            break;
          case reflection::ATTR_CONST:
            newArg.ptrType.isPointsToConst = true;
            break;
          default:
            break;
          }
        }
      }
      al.push_back(newArg);
    }

    // fill function output
    out_FuncStr = FuncNameStr;
    args = al;

    // add parse results to cache
    std::string parseStr(in_str);
    ArgPair ap = {FuncNameStr, args};
    ArgVectorMapSingleton::Instance()->insert(
        std::pair<std::string, ArgPair>(parseStr, ap));

    return true;
  } else {
    // todo: add here methods to extract BI names
    // other that "_Z{num_of_sym_BI_name}{BI_name}
    return false;
  }
}

bool OCLBuiltinParser::GetOCLMangledName( const std::string& in_funcName,
                                         const ArgVector& in_args,
                                         std::string& out_str )
{   
    reflection::FunctionDescriptor fd;
    BasicTypeToStr typeConvertor;
    fd.name = in_funcName;
    // the parameters pointed in function descriptor should exist at the time of mangle operation,
    // because reflection::FunctionDescriptor::parameters 
    // so we create parameters and put pointers on them to the vector in order to delete them later,
    // after the mangle operation will be completed
    
    for (std::size_t i = 0; i < in_args.size(); ++i)
    {
        const OCLBuiltinParser::ARG arg = in_args[i];
        switch (arg.genType)
        {
            case OCLBuiltinParser::BASIC:
            {
                // create parameter
                reflection::PrimitiveType *primitiveType = new reflection::PrimitiveType(typeConvertor[arg.basicType]);
                // 
                fd.parameters.push_back(primitiveType);
                break;
            }
            case OCLBuiltinParser::VECTOR:
            {
                reflection::RefParamType primitiveType(new reflection::PrimitiveType(typeConvertor[arg.vecType.elType]));
                reflection::VectorType *vectorType = new reflection::VectorType(primitiveType,(int)arg.vecType.elNum);
                fd.parameters.push_back(vectorType);
                break;
            }
            case OCLBuiltinParser::POINTER:
            {
                reflection::PointerType *pPointerType;
                switch (arg.ptrType.ptrType[0].genType) {
                    case OCLBuiltinParser::BASIC:
                    {
                        reflection::RefParamType primitiveType( 
                            new reflection::PrimitiveType(typeConvertor[arg.ptrType.ptrType[0].basicType]));
                        pPointerType = new reflection::PointerType(primitiveType);
                        fd.parameters.push_back(pPointerType);
                        break;
                    }
                    case OCLBuiltinParser::VECTOR:
                    {
                        reflection::RefParamType primitiveType(
                            new reflection::PrimitiveType(typeConvertor[arg.ptrType.ptrType[0].vecType.elType]));
                        reflection::VectorType *vectorType = 
                            new reflection::VectorType(primitiveType,(int)arg.ptrType.ptrType[0].vecType.elNum);
                        pPointerType = new reflection::PointerType(vectorType);
                        fd.parameters.push_back(pPointerType);
                        break;
                    }
                    default:
                    {
                        throw Validation::Exception::InvalidArgument("Wrong pointer type!");
                    }
                }

                if (arg.ptrType.isAddrSpace) {
                    switch(arg.ptrType.AddrSpace) {
                    case PRIVATE:
                        pPointerType->addAttribute(reflection::ATTR_PRIVATE);
                        break;
                    case GLOBAL:
                        pPointerType->addAttribute(reflection::ATTR_GLOBAL);
                        break;
                    case CONSTANT:
                        pPointerType->addAttribute(reflection::ATTR_CONSTANT);
                        break;
                    case LOCAL:
                        pPointerType->addAttribute(reflection::ATTR_LOCAL);
                        break;
                    case GENERIC:
                        pPointerType->addAttribute(reflection::ATTR_GENERIC);
                        break;
                    default:
                        throw Validation::Exception::InvalidArgument("Unknown address space!");
                    }
                }

                if (arg.ptrType.isPointsToConst) {
                    pPointerType->addAttribute(reflection::ATTR_CONST);
                }

                break;
            }
            default:
            {
                throw Validation::Exception::InvalidArgument("Unknown type to mangle!");
            }
        }
    }

    out_str = mangle(fd);
    return true;
}


