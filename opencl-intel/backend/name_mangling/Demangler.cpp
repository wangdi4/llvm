/****************************************************************************
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

  File Name: Demangler.cpp

\****************************************************************************/

#include <sstream>
#include <cctype>
#include <cstdlib>
#include "antlr/AST.hpp"
#include "DemangleLexer.hpp"
#include "DemangleParser.hpp"
#include "FunctionDescriptor.h"

extern bool isApplePrefixFree(const std::string& s);
//
//A callback delegate (aka functor), to handle the discovery of parameters by
//the parser (accumulator)
//
struct TypeAccumulator: reflection::TypeDelegate<TypeAccumulator> {

TypeAccumulator(){
  m_cb = &TypeAccumulator::pushParameter;
  m_pReceiver = this;
}

void pushParameter(reflection::Type* t){
  m_parameters.push_back(t->clone());
}

std::vector<reflection::Type*> m_parameters;

};//End TypeAccumulator

const char* PREFIX = "_Z";

size_t PREFIX_LEN = 2;

#ifndef NDEBUG
static const char* APPLE_PREFIX = "__cl_";
#endif

size_t APPLE_PREFIX_LEN = 5;
//parsing methods for the raw mangled name
//
static bool peelPrefix(llvm::StringRef& s){
  if (PREFIX != s.substr(0,PREFIX_LEN) )
      return false;
  s = s.substr(PREFIX_LEN, s.size()-PREFIX_LEN);
  return true;
}

static llvm::StringRef peelNameLen(llvm::StringRef& s){
  int len = 0;
  llvm::StringRef::const_iterator it = s.begin();
  while(isdigit(*it++))
    ++len;
  llvm::StringRef ret = s.substr(0, len);
  s = s.substr(len, s.size()-len);
  return ret;
}

//
//Implementation of an API function.
//Indicating whether the given string is a mangled function name
//
bool isMangledName(const char* rawString){
  const std::string strName(rawString);
  if (strName.substr(0, PREFIX_LEN) != PREFIX)
    return false;
  return isdigit(strName[PREFIX_LEN]);
}

//
//Implementation of an API function.
//converts the given mangled name string to a FunctionDescriptor object.
//
static reflection::FunctionDescriptor demangleImpl(const char* rawstring){
  ANTLR_USING_NAMESPACE(antlr)
  ANTLR_USING_NAMESPACE(namemangling)
  if (!rawstring || reflection::FunctionDescriptor::nullString() == rawstring)
    return reflection::FunctionDescriptor::null();
  std::stringstream input;
  llvm::StringRef mangledName(rawstring);
  //making sure it starts with _Z
  if (false == peelPrefix(mangledName))
    return reflection::FunctionDescriptor::null();
  llvm::StringRef nameLen = peelNameLen(mangledName);
  //cutting the prefix
  int len = atoi(nameLen.data());
  std::string functionName = mangledName.substr(0, len);
  std::string parameters =
    mangledName.substr(len, mangledName.size() - len);
  input << parameters;
  DemangleLexer lexer(input);
  DemangleParser parser(lexer);
  parser.setLexer(&lexer);
  //
  //setting the callback to parameter discovery
  //
  TypeAccumulator parameterAccumulator;
  parser.setCallback(&parameterAccumulator);
  // Parse the input expression
  try {
    parser.demangle();
  }catch(ANTLRException ex){
    return reflection::FunctionDescriptor::null();
  }
  reflection::FunctionDescriptor ret;
  ret.name = functionName;
  ret.parameters.insert ( ret.parameters.begin(),
    parameterAccumulator.m_parameters.rbegin(),
    parameterAccumulator.m_parameters.rend() );
  return ret;
}

reflection::FunctionDescriptor demangle(const char* rawstring){
  reflection::FunctionDescriptor ret = demangleImpl(rawstring);
  if (ret.isNull())
    return ret;
  if (isApplePrefixFree(ret.name))
    return ret;
  assert ((ret.name.substr(0, APPLE_PREFIX_LEN) == APPLE_PREFIX) && "not an apple prefix!");
  ret.name = ret.name.substr(APPLE_PREFIX_LEN, ret.name.length() - APPLE_PREFIX_LEN);
  return ret;
}

class DemanglerException: public std::exception{
public:
  const char* what() const throw(){
    return "invalid prefix";
  }
};

//
//Implementation of an API function.
llvm::StringRef stripName(const char* rawstring){
  llvm::StringRef mangledName(rawstring);
  //making sure it starts with _Z
  if (false == peelPrefix(mangledName))
    throw DemanglerException();
  std::string nameLen = peelNameLen(mangledName);
  //cutting the prefix
  int len = atoi(nameLen.c_str());
  return llvm::StringRef(rawstring + PREFIX_LEN + nameLen.length(), len);
}
