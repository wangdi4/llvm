/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "FunctionDescriptor.h"
#include "Utils.h"
#include "Type.h"
#include <assert.h>
#include <string>
#include <sstream>
#include <list>
#include <algorithm>

//
//Implementation of an API.
//Mangle the given function descriptor to a mangled name.
//The mangling algorithm is intendent to match the Itanium mangling altorithm.
//More concretly, it is designed to match clang 3.0 itanium mangling.
//

//Array of constants used by clang to duplicate parameters
//S_  is used to duplicate the 1st parameter
//S0_ is used to duplicate the 2nd parameter (within the same string)
//Note!: theoretically, we need "S1_, S2_,....), but those two are enough for
//all the builtin functions in openCL, so that will do until proven otherwise.
const char* DUPLICANT_STR[2] = {"S_", "S0_"};

class MangleVisitor: public reflection::TypeVisitor{
public:

  MangleVisitor(std::stringstream& s): m_stream(s){}

  void operator() (const reflection::Type* t){
    t->accept(this);
  }

  //visit methods
  void visit(const reflection::Type* t){
    //NOTE! we don't use  DUPLICANT_STR here, since primitive strings are
    //shorter or less then the DUPLICANT_STR itself.
    m_stream << mangledString(t);
  }

  void visit(const reflection::Pointer* p){
    int typeIndex = getTypeIndex(p);
    if( -1 != typeIndex ) {
      m_stream << getDuplicateString(typeIndex);
      return;
    }
    m_stream << "P";
    std::vector<std::string>::const_reverse_iterator e = p->rendAttributes(),
    it = p->rbeginAttributes();
    while (it != e ){
      if (*it == "__private")
        m_stream << "U3AS0";
      else if(*it == "__global")
        m_stream << "U3AS1";
      else if(*it == "__constant")
        m_stream << "U3AS2";
      else if(*it == "__local")
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
    p->getPointee()->accept(this);
    addIfNotExist(p);
  }

  void visit(const reflection::Vector* v){
    int typeIndex = getTypeIndex(v);
    if( -1 != typeIndex ) {
      m_stream << getDuplicateString(typeIndex);
      return;
    }
    addIfNotExist(v);
    m_stream << "Dv" << v->getLen() << "_" << mangledString(v);
  }

  void visit(const reflection::UserDefinedTy* pTy){
    int typeIndex = getTypeIndex(pTy);
    if( -1 != typeIndex ) {
      m_stream << getDuplicateString(typeIndex);
      return;
    }
    addIfNotExist(pTy);
    std::string name = pTy->toString();
    m_stream << name.size() << name;
  }

private:

  void addIfNotExist(const reflection::Type* t){
    std::list<const reflection::Type*>::const_iterator it = m_listTys.begin(),
      e = m_listTys.end();
    while (it != e){
      if ((*it)->equals(t))
        return;
      ++it;
    }
    m_listTys.push_back(t);
  }

  int getTypeIndex(const reflection::Type* t)const{
    int ret = 0;
    std::list<const reflection::Type*>::const_iterator it = m_listTys.begin(),
      e = m_listTys.end();
    while (it != e){
      if ((*it)->equals(t))
        return ret;
      ++ret;
      ++it;
    }
    return -1;
  }

  static std::string getDuplicateString(int index){
    assert (index >= 0 && "illegal index");
    if (0 == index)
      return "S_";
    std::stringstream ss;
    ss << "S" << index-1 << "_";
    return ss.str();
  }

  //holds the mangled string representing the prototype of the function
  std::stringstream& m_stream;
  //list of types 'seen' so far
  std::list<const reflection::Type*> m_listTys;
};

std::string mangle(const reflection::FunctionDescriptor& fd){
  if (fd.isNull())
    return reflection::FunctionDescriptor::nullString();
  std::stringstream ret;
  ret << "_Z" << fd.name.length() << fd.name;
  MangleVisitor visitor(ret);
  std::for_each(fd.parameters.begin(), fd.parameters.end(), visitor);
  return ret.str();
}
