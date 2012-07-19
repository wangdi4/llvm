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

  File Name: Mangler.cpp

  \****************************************************************************/

#include "FunctionDescriptor.h"
#include "Type.h"
#include <assert.h>
#include <string>
#include <sstream>
#include <algorithm>

//
//Implementation of an API.
//Mangle the given function descriptor to a mangled name.
//The mangling algorithm is intendent to match the Itanium mangling altorithm.
//More concretly, it is designed to match clang 3.0 itanium mangling.
//
const char* mangledTypes[reflection::NUM_TYPES] = {
  "b", //BOOL
  "h", //UCHAR
  "c", //CHAR
  "t", //USHORT
  "s", //SHORT
  "j", //UINT
  "i", //INT
  "m", //ULONG
  "l", //LONG
  "f", //HALF
  "d", //FLOAT
  "Dh" //DOUBLE
};

//Array of constants used by clang to duplicate parameters
//S_  is used to duplicate the 1st parameter
//S0_ is used to duplicate the 2nd parameter (within the same string)
//Note!: theoretically, we need "S1_, S2_,....), but those two are enough for
//all the builtin functions in openCL, so that will do until proven otherwise.
const char* DUPLICANT_STR[2] = {"S_", "S0_"};

//BOOL is the first type enum, and its not neccesrily valed zero.
static const char* primitiveToString(const reflection::Type* t){
  assert(t && "null pointer");
  assert( (t->getPrimitive() >= reflection::primitives::BOOL) &&
  (t->getPrimitive() <= reflection::primitives::DOUBLE) && "invalid primitive type");
  return mangledTypes[t->getPrimitive()-reflection::primitives::BOOL];
}

class MangleVisitor: public reflection::TypeVisitor{
public:

  MangleVisitor(std::stringstream& s): m_stream(s), m_previous(NULL),
  m_recursiveCounter(0) , m_dupIndex(0){}

  void operator() (const reflection::Type* t){
    t->accept(this);
  }

  //visit methods
  void visit(const reflection::Type* t){
    //NOTE! we don't use  DUPLICANT_STR here, since primitive strings are
    //shorter or less then the DUPLICANT_STR itself.
    m_stream << primitiveToString(t);
    m_previous = t;
  }

  void visit(const reflection::Pointer* p){
    if( isDuplicant(p) ) {
      m_stream << getAndupdateDuplicant();
      return;
    }
    m_stream << "P";
    std::vector<std::string>::const_reverse_iterator it =
    p->rbeginAttributes(), e = p->rendAttributes();
    while (it != e ){
      if (*it == "__private")
        m_stream << "U3AS0";
      else if(*it == "__global")
        m_stream << "U3AS1";
      else if(*it == "__local")
        m_stream << "U3AS2";
      else if(*it == "__const")
        m_stream << "U3AS3";
      else if(*it == "restrict")
        m_stream << "r";
      else if(*it == "volatile")
        m_stream << "V";
      else if(*it == "const")
        m_stream << "K";
      #ifndef NDEBUG
      else
        assert(false && "dont know this attribute!");
      #endif
      ++it;
    }
    ++m_recursiveCounter;
    p->getPoitee()->accept(this);
    m_previous = p;
  }

  void visit(const reflection::Vector* v){
    if( isDuplicant(v) ){
      m_stream << getAndupdateDuplicant();
      return;
    }
    m_stream << "Dv" << v->getLen() << "_" << primitiveToString(v);
    m_previous = v;
  }

private:
  bool isDuplicant(const reflection::Type* t)const{
    if (NULL == m_previous)
      return false; //its the first one..
    return m_previous->equals(t);
  }

  const char* getAndupdateDuplicant(){
    const char* ret = DUPLICANT_STR[m_dupIndex];
    //if we have a match inside a nested visit, we are duplicating the next
    //parameter (S0_...)
    m_dupIndex = std::max(m_dupIndex, m_recursiveCounter);
    assert(m_dupIndex < (sizeof(DUPLICANT_STR)/sizeof(const char*)) );
    return ret;
  }
  //holds the mangled string representing the prototype of the function
  std::stringstream& m_stream;
  const reflection::Type* m_previous;
  //holds the 'recursion depth' of the visit call. (each time one visit call
  //calls another, it is incremented by one).
  unsigned m_recursiveCounter;
  //the index for the 'duplicate parameter' array
  //indicates whether this visit call in initiated by yet another visit call
  unsigned m_dupIndex;
};

std::string mangle(const reflection::FunctionDescriptor& fd){
  std::stringstream ret;
  ret << "_Z" << fd.name.length() << fd.name;
  MangleVisitor visitor(ret);
  std::for_each(fd.parameters.begin(), fd.parameters.end(), visitor);
  return ret.str();
}
