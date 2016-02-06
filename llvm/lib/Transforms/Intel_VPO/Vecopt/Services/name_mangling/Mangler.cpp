/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "FunctionDescriptor.h"
#include "Utils.h"
#include "ParameterType.h"
#include <assert.h>
#include <string>
#include <sstream>
#include <map>

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

  MangleVisitor(std::stringstream& s): m_stream(s) {
  }

  void operator() (const reflection::ParamType* t){
    t->accept(this);
  }

  //visit methods
  void visit(const reflection::PrimitiveType* t){
    //NOTE! we don't use  DUPLICANT_STR here, since primitive strings are
    //shorter or less then the DUPLICANT_STR itself.
    m_stream << reflection::mangledPrimitiveString(t->getPrimitive());
  }

  void visit(const reflection::PointerType* p) {
    int typeIndex = getTypeIndex(p);
    if( -1 != typeIndex ) {
      m_stream << reflection::getDuplicateString(typeIndex);
      return;
    }
    m_stream << "P";
    for (unsigned int i=0; i < p->getAttributes().size(); ++i) {
      m_stream << getMangledAttribute(p->getAttributes()[i]);
    }
    p->getPointee()->accept(this);
    m_dupList.push_back((reflection::ParamType*)p);
  }

  void visit(const reflection::VectorType* v){
    int typeIndex = getTypeIndex(v);
    if( -1 != typeIndex ) {
      m_stream << reflection::getDuplicateString(typeIndex);
      return;
    }
    m_stream << "Dv" << v->getLength() << "_";
    v->getScalarType()->accept(this);
    m_dupList.push_back((reflection::ParamType*)v);
  }

  void visit(const reflection::BlockType* p) {
    int typeIndex = getTypeIndex(p);
    if( -1 != typeIndex ) {
      m_stream << reflection::getDuplicateString(typeIndex);
      return;
    }
    m_stream << "U" << "13block_pointerFv";
    for (unsigned int i=0; i < p->getNumOfParams(); ++i) {
      p->getParam(i)->accept(this);
    }
    m_dupList.push_back((reflection::ParamType*)p);
    m_stream << "E";
  }

  void visit(const reflection::UserDefinedType* pTy){
    int typeIndex = getTypeIndex(pTy);
    if( -1 != typeIndex ) {
      m_stream << reflection::getDuplicateString(typeIndex);
      return;
    }
    std::string name = pTy->toString();
    m_stream << name.size() << name;
    m_dupList.push_back((reflection::ParamType*)pTy);
  }

private:

  int getTypeIndex(const reflection::ParamType* t)const{
    for (unsigned int i=0; i < m_dupList.size(); ++i) {
      if (t->equals(m_dupList[i])) {
        return i;
      }
    }
    return -1;
  }

  //holds the mangled string representing the prototype of the function
  std::stringstream& m_stream;
  //list of types 'seen' so far
  reflection::DuplicatedTypeList m_dupList;
};

std::string mangle(const reflection::FunctionDescriptor& fd){
  if (fd.isNull())
    return reflection::FunctionDescriptor::nullString();
  std::stringstream ret;
  ret << "_Z" << fd.name.length() << fd.name;
  MangleVisitor visitor(ret);
  for (unsigned int i=0; i < fd.parameters.size(); ++i) {
    fd.parameters[i]->accept(&visitor);
  }
  return ret.str();
}
