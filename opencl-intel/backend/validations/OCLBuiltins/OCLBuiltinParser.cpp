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
    m_Map["v"] = OCLBuiltinParser::VOID;
    m_Map["b"] = OCLBuiltinParser::BOOL;
    m_Map["c"] = OCLBuiltinParser::CHAR;
    m_Map["h"] = OCLBuiltinParser::UCHAR;
    m_Map["s"] = OCLBuiltinParser::SHORT;
    m_Map["t"] = OCLBuiltinParser::USHORT;
    m_Map["i"] = OCLBuiltinParser::INT;
    m_Map["j"] = OCLBuiltinParser::UINT;
    m_Map["l"] = OCLBuiltinParser::LONG;
    m_Map["m"] = OCLBuiltinParser::ULONG;
    m_Map["x"] = OCLBuiltinParser::LONGLONG;
    m_Map["y"] = OCLBuiltinParser::ULONGLONG;
    m_Map["n"] = OCLBuiltinParser::INT128;
    m_Map["o"] = OCLBuiltinParser::UINT128;
    m_Map["f"] = OCLBuiltinParser::FLOAT;
    m_Map["d"] = OCLBuiltinParser::DOUBLE;
    m_Map["e"] = OCLBuiltinParser::LONGDOUBLE;
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

    std::string operator[](
        const OCLBuiltinParser::BasicArgType& type) const
    {
        std::map<OCLBuiltinParser::BasicArgType, std::string>::const_iterator 
            it = m_Map.find(type);
        if (it == m_Map.end())
            throw Exception::InvalidArgument("");
        return it->second;
    }
private:
    void initStatic();
    static std::map<OCLBuiltinParser::BasicArgType, std::string> m_Map;
};

void BasicTypeToStr::initStatic()
{
    m_Map[OCLBuiltinParser::VOID]       = "v";
    m_Map[OCLBuiltinParser::BOOL]       = "b";
    m_Map[OCLBuiltinParser::CHAR]       = "c";
    m_Map[OCLBuiltinParser::UCHAR]      = "h";
    m_Map[OCLBuiltinParser::SHORT]      = "s";
    m_Map[OCLBuiltinParser::USHORT]     = "t";
    m_Map[OCLBuiltinParser::INT]        = "i";
    m_Map[OCLBuiltinParser::UINT]       = "j";
    m_Map[OCLBuiltinParser::LONG]       = "l";
    m_Map[OCLBuiltinParser::ULONG]      = "m";
    m_Map[OCLBuiltinParser::LONGLONG]   = "x";
    m_Map[OCLBuiltinParser::ULONGLONG]  = "y";
    m_Map[OCLBuiltinParser::INT128]     = "n";
    m_Map[OCLBuiltinParser::UINT128]    = "o";
    m_Map[OCLBuiltinParser::FLOAT]      = "f";
    m_Map[OCLBuiltinParser::DOUBLE]     = "d";
    m_Map[OCLBuiltinParser::LONGDOUBLE] = "e";
}

std::map<OCLBuiltinParser::BasicArgType, std::string> BasicTypeToStr::m_Map;

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

void OCLBuiltinParser::ParseArg(const std::string& ArgumentsStr, 
                                uint32_t& ArgumentsStrOffs, ArgVector& al,
                                const ArgVector& glArg)
{
    Regex pattern("([vbchstijlmxynofde])([A-Za-z0-9_]*)|"
        "(Dv)([0-9]+)_([A-Za-z0-9_]+)|"
        "(A)([0-9]+)_([A-Za-z0-9_]+)|"
        "(P)([V])?([K])?(U3AS[0-9])?([0-9]+)?([A-Za-z0-9_]+)|"
        "(S_)([A-Za-z0-9_]*)");

    // object that will contain the sequence of sub-matches
    SmallVector<StringRef, 32> res;
    // substring with current arguments
    std::string sstr(ArgumentsStr.substr(ArgumentsStrOffs));
    // match the string
    bool valid = pattern.match(sstr, &res);

    // must parse ok
    if(!valid)
    {
        throw Exception::InvalidArgument(
            "Unknown format of OCL built in argument");
    }

    // switches of different argument types
    bool bBasic =            !res[1].empty();
    bool bVector =           !res[3].empty();
    bool bArray =            !res[6].empty();
    bool bPointer =          !res[9].empty();
    bool bSubstitution =     !res[15].empty();

    ARG newArg;
    BasicTypeFromStr BasicTypeConvertor;

    if(bBasic)
    {   // if basic type
        
        // dirty hack to check Regex did not skip symbols in the beginning
        // of string till it finds known symbol from Basic type
        // wo hack this function name _Z3mixU78__vector4fS_S_ does not fail
        if(res[1].begin() != sstr.c_str())
        {
            throw Exception::InvalidArgument(
                "Unknown format of OCL built in argument");
        }
        
        std::string sBasic =            res[1];
        newArg.genType = BASIC;
        newArg.basicType = BasicTypeConvertor[sBasic];
        ArgumentsStrOffs += sBasic.length();
    }
    else if(bVector)
    {
        // vector type
        std::string sVector =           res[3];
        std::string sVectorElem =       res[4];
        std::string sVectorType =       res[5];
        ArgVector vecArg;
        // increase offset
        ArgumentsStrOffs += sVector.length() + sVectorElem.length() +
                                strlen("_");
        // extract basic type
        ParseArg(ArgumentsStr, ArgumentsStrOffs, vecArg, glArg);
        // check vector stores basic type
        assert(vecArg.size() == 1 && "vector size should be 1");
        assert(vecArg[0].genType == BASIC && 
            "vector element should have basic type");
        // fill vector params
        newArg.genType = VECTOR;
        // element type
        newArg.vecType.elType = vecArg[0].basicType;
        // num of elements in vector
        newArg.vecType.elNum = atoi(sVectorElem.c_str());
        // verify num of elements
        assert(newArg.vecType.elNum > 0 && newArg.vecType.elNum <= 16);
    }
    else if(bArray)
    {
        // array type
        std::string sArray =            res[6];
        std::string sArraySize =        res[7];
        std::string sArrayType =        res[8];
        ArgVector arrArg;
        ArgumentsStrOffs += sArray.length() + sArraySize.length() + 
            strlen("_");
        // extract type
        ParseArg(ArgumentsStr, ArgumentsStrOffs, arrArg, glArg);
        // check array type vector is size() == 1
        assert(arrArg.size() == 1 && "array type size() != 1 ");
        newArg.genType = ARRAY;
        newArg.arrType.elType.push_back(arrArg[0]);
        newArg.arrType.elNum = atoi(sArraySize.c_str());
    }
    else if (bPointer)
    {   // pointer with length after it i.e. P10_image2d_t
        const std::string sPointer = res[9];
        const std::string sPointerWithVolatileN = res[10];
        const std::string sPointerWithConstN = res[11];
        const std::string sPointerWithAdrSpaceN = res[12];
        const std::string sPointerWithLengthN = res[13];
        const std::string sPointerType = res[14];
        
        ArgumentsStrOffs += sPointer.length();

        bool bPointerWithVolatileN = (sPointerWithVolatileN != "");
        bool bPointerWithConstN = (sPointerWithConstN != "");
        bool bPointerWithAdrSpaceN = (sPointerWithAdrSpaceN != "");
        bool bPointerWithLenN = (sPointerWithLengthN != "");
        
        newArg.genType = POINTER;
        
        if( bPointerWithVolatileN )
        {
            ArgumentsStrOffs += sPointerWithVolatileN.length();
        }

        if( bPointerWithConstN )
        {
            ArgumentsStrOffs += sPointerWithConstN.length();
        }

        if(bPointerWithAdrSpaceN)
        {
            const std::string sAddrSpaceStr = sPointerWithAdrSpaceN;
            const std::string sAddrSpaceNum = 
                sAddrSpaceStr.substr(std::string("U3AS").size());
            newArg.ptrType.isAddrSpace = true;
            newArg.ptrType.AddrSpace = OCLBuiltinParser::AddressSpace(atoi(sAddrSpaceNum.c_str()));
            ArgumentsStrOffs += sPointerWithAdrSpaceN.length();
        }
        
        if(bPointerWithLenN)
        {
            const std::string sPointerStrLen = sPointerWithLengthN;
            const std::string sPointerStr =  sPointerType;
            uint32_t _len = atoi(sPointerStrLen.c_str());
            // check length fits into string
            if(!(_len <= sPointerStr.length()))
            {
                throw Exception::InvalidArgument("Invalid pointer type length");
            }

            newArg.ptrType.ptrToStr = sPointerStr.substr(0,_len);
            ArgumentsStrOffs += sPointerStrLen.length() + _len;
        }
        else
        {
            std::string sPointerArg = sPointerType;
            ArgVector PtrArg;

            // extract basic type
            ParseArg(ArgumentsStr, ArgumentsStrOffs, PtrArg, glArg);
            newArg.ptrType.ptrToStr = "";
            newArg.ptrType.ptrType.push_back(PtrArg[0]);
        }

    }
    else if (bSubstitution)
    {
        std::string sSubstitution =             res[15];
        if(glArg.size() == 0)
        {
            throw Exception::InvalidArgument(
                "Substitution cannot be first argument");
        }
        ARG prevArg =  glArg.back();
        newArg = prevArg;
        ArgumentsStrOffs += sSubstitution.length();
    }
    else
    {
        throw Exception::InvalidArgument(
            "Unknown argument format of OCL built in argument");
    }
    al.push_back(newArg);
}

bool OCLBuiltinParser::ParseOCLBuiltin(const std::string& in_str, 
                                       std::string& out_FuncStr, 
                                       ArgVector& args)
{
    // check if we have parser results in cache ArgVectorMapSingleton
    ArgVectorMap::iterator it = ArgVectorMapSingleton::Instance()->find(in_str);
    if (it != ArgVectorMapSingleton::Instance()->end())
    {
        out_FuncStr = (it->second).name;
        args = (it->second).args;
        return true;
    }

    // pattern for _Z builtins
    Regex pattern("_Z([0-9]+)([A-Za-z0-9_]+)");
    std::string err;
    
    // object that will contain the sequence of sub-matches
    SmallVector<StringRef, 4> result;

    // match the IP address with the regular expression
    bool valid = pattern.match(in_str, &result);

    // if _Z found
    if(valid)
    {
        // string with number of chars in builtin name
        std::string FuncNameLenStr = result[1];
        // number of chars in builtin name
        uint32_t FuncNameLen = atoi(FuncNameLenStr.c_str());
        // string with built in name and arguments
        std::string FuncNameStrPlus(result[2]);

        // check func name length fits into string length
        if(!(FuncNameLen < FuncNameStrPlus.length()))
        {
            throw Exception::InvalidArgument("Invalid built-in name length");
        }
        // extract built in name
        std::string FuncNameStr(FuncNameStrPlus.substr(0, FuncNameLen));
        // extract string with arguments
        std::string ArgumentsStr(FuncNameStrPlus.substr(FuncNameLen));
        // init offset in argument string
        uint32_t ArgumentsStrOffs = 0;
        // vector with arguments
        ArgVector al;
        // loop over arguments
        while (ArgumentsStrOffs < ArgumentsStr.length())
        {
            // extract one argument
            ParseArg(ArgumentsStr, ArgumentsStrOffs, al, al);
        }

        // fill function output 
        out_FuncStr = FuncNameStr;
        args = al;
        
        // add parse results to cache
        std::string parseStr(in_str);
        ArgPair ap = { FuncNameStr, args };
        ArgVectorMapSingleton::Instance()->insert(
            std::pair<std::string, ArgPair>(parseStr, ap));
        
        return true;
    }
    else
    {
        // todo: add here methods to extract BI names 
        // other that "_Z{num_of_sym_BI_name}{BI_name}
        return false;
    }
}

bool OCLBuiltinParser::getOCLMangledTypeName(std::ostringstream& typeStream,
                                        const OCLBuiltinParser::ARG& arg)
{
    BasicTypeToStr typeConvertor;
    switch (arg.genType)
    {
    case OCLBuiltinParser::BASIC:
        {
            // if argument type is basic, just add corresponding literal to the string.
            typeStream << typeConvertor[arg.basicType];
            break;
        }
    case OCLBuiltinParser::POINTER:
        {
            if (arg.ptrType.isAddrSpace && arg.ptrType.AddrSpace != OCLBuiltinParser::PRIVATE)
            {
                if (arg.ptrType.isPointsToConst)
                {
                    typeStream << "PKU3AS";
                }
                else
                {
                    typeStream << "PU3AS";
                }
                typeStream << (int)arg.ptrType.AddrSpace;
                getOCLMangledTypeName(typeStream, arg.ptrType.ptrType[0]);
            }
            else
            {
                if (arg.ptrType.isPointsToConst)
                {
                    typeStream << "PK";
                }
                else
                {
                    typeStream << "P";
                }
                getOCLMangledTypeName(typeStream, arg.ptrType.ptrType[0]);
            }
            break;
        }
    case OCLBuiltinParser::ARRAY:
        {
            typeStream << "A" << arg.arrType.elNum << "_";
            getOCLMangledTypeName(typeStream, arg.arrType.elType[0]);
            break;
        }
    case OCLBuiltinParser::VECTOR:
        {
            typeStream << "Dv" << arg.vecType.elNum << "_";
            ARG vecElemType;
            vecElemType.genType = BASIC;
            vecElemType.basicType = arg.vecType.elType;
            getOCLMangledTypeName(typeStream, vecElemType);
            break;
        }
    default:
        {
            throw Validation::Exception::InvalidArgument("Unknown type to mangle!");
        }
    }
    return true;
}

bool OCLBuiltinParser::GetOCLMangledName( const std::string& in_funcName,
                                         const ArgVector& in_args,
                                         std::string& out_str )
{
    const std::string clangPrefix("_Z");
    // string with argument manglings.
    std::ostringstream suffix;
    for (std::size_t i = 0; i < in_args.size(); ++i)
    {
        getOCLMangledTypeName(suffix, in_args[i]);
    }
    std::ostringstream funcNameSize;
    funcNameSize << in_funcName.length();
    out_str = clangPrefix + funcNameSize.str() + in_funcName + suffix.str();
    return true;
}
