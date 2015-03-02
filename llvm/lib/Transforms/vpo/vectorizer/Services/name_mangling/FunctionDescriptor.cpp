/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "FunctionDescriptor.h"
#include "ParameterType.h"
#include <sstream>

namespace reflection{

///////////////////////////////////////////////////////////////////////////////
//Purpose: helps to determines the vector width of a given function descriptor
//Note: we could in fact use plain poly-morphism here, (no covariance problem
//here), but this will compel us to add the getWidth method throughout the Type
//inheritance tree, when it is only needed here.
///////////////////////////////////////////////////////////////////////////////
struct VWidthResolver: TypeVisitor {
  void visit(const PrimitiveType*) {
    m_width = FunctionDescriptor::SCALAR;
  }

  void visit(const VectorType* v) {
    m_width = v->getLength();
  }

  void visit(const PointerType* p) {
    p->getPointee()->accept(this);
  }

  void visit(const BlockType*) {
    m_width = FunctionDescriptor::SCALAR;
  }

  void visit(const UserDefinedType*) {
    m_width = FunctionDescriptor::SCALAR;
  }

  unsigned width() const { return m_width; }
private:
  unsigned m_width;
};

llvm::StringRef FunctionDescriptor::nullString(){
  return llvm::StringRef("<invalid>");
}

std::string FunctionDescriptor::toString()const{
  std::stringstream stream;
  if ( isNull() ){
    stream << "null descriptor";
    return stream.str();
  }
  stream << name << "(";
  size_t paramCount = parameters.size();
  if (paramCount > 0){
    for (size_t i=0 ; i<paramCount-1 ; ++i)
      stream << parameters[i]->toString() << ", ";
    stream << parameters[paramCount-1]->toString();
  }
  stream << ")";
  return stream.str();
}

static bool equal(const TypeVector& l, const TypeVector& r){
  if (&l == &r)
    return true;
  if (l.size() != r.size())
    return false;
  TypeVector::const_iterator itl = l.begin(), itr = r.begin(),
  endl = l.end();
  while(itl != endl){
    if (!(*itl)->equals(*itr))
      return false;
    ++itl;
    ++itr;
  }
  return true;
}

//
//FunctionDescriptor
//

bool FunctionDescriptor::operator == (const FunctionDescriptor& that)const{
  if (this == &that)
    return true;
  if (name != that.name)
    return false;
  return equal(parameters, that.parameters);
}

bool FunctionDescriptor::operator < (const FunctionDescriptor& that)const{
  int strCmp = name.compare(that.name);
  if( strCmp )
    return (strCmp < 0);
  size_t len = parameters.size(), thatLen = that.parameters.size();
  if (len != thatLen)
    return len < thatLen;
  TypeVector::const_iterator it = parameters.begin(),
  e = parameters.end(), thatit = that.parameters.begin();
  while (it != e){
    int cmp = (*it)->toString().compare((*thatit)->toString());
    if (cmp)
      return (cmp < 0);
    ++thatit;
    ++it;
  }
  return false;
}

void FunctionDescriptor::assignAutomaticWidth(){
  VWidthResolver widthResolver;
  unsigned w = FunctionDescriptor::SCALAR;
  size_t paramCount = parameters.size();
  for (size_t i=0 ; i<paramCount ; ++i){
    parameters[i]->accept(&widthResolver);
    if (w < widthResolver.width())
      w = widthResolver.width();
  }
  width = w;
}

bool FunctionDescriptor::isNull()const{
  return (name.empty() && parameters.empty());
}

FunctionDescriptor FunctionDescriptor::null(){
  FunctionDescriptor fd;
  fd.name = "";
  return fd;
}

}
