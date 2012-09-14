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

//parsing methods for the raw mangled name
//
static std::string peelPrefix(std::string& s){
  if ("_Z" != s.substr(0,2) )
      throw "invalid prefix";
  s = s.substr(2, s.length()-2);
  return "_Z";
}

static std::string peelNameLen(std::string& s){
  int len = 0;
  std::string::const_iterator it = s.begin();
  while(isdigit(*it++))
    ++len;
  std::string ret = s.substr(0, len);
  s = s.substr(len, s.length()-len);
  return ret;
}

//
//Implementation of an API function.
//converts the given mangled name string to a FunctionDescriptor object.
//
reflection::FunctionDescriptor demangle(const char* rawstring){
  ANTLR_USING_NAMESPACE(antlr)
  ANTLR_USING_NAMESPACE(namemangling)
  std::stringstream input;
  std::string mangledName(rawstring);
  //making sure it starts with _Z
  peelPrefix(mangledName);
  std::string nameLen = peelNameLen(mangledName);
  //cutting the prefix
  int len = atoi(nameLen.c_str());
  std::string functionName = mangledName.substr(0, len);
  std::string parameters =
    mangledName.substr(len, mangledName.length() - len);
  input << parameters;
  DemangleLexer lexer(input);
  DemangleParser parser(lexer);
  //
  //setting the callback to parameter discovery
  //
  TypeAccumulator parameterAccumulator;
  parser.setCallback(&parameterAccumulator);
  // Parse the input expression
  parser.demangle();
  reflection::FunctionDescriptor ret;
  ret.name = functionName;
  ret.parameters.insert ( ret.parameters.begin(),
    parameterAccumulator.m_parameters.rbegin(),
    parameterAccumulator.m_parameters.rend() );
  return ret;
}

//
//Implementation of an API function.
std::string stripName(const char* rawstring){
  std::string mangledName(rawstring);
  //making sure it starts with _Z
  peelPrefix(mangledName);
  std::string nameLen = peelNameLen(mangledName);
  //cutting the prefix
  int len = atoi(nameLen.c_str());
  return mangledName.substr(0, len);
}
