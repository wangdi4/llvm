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

File Name: FunctionDescriptor.cpp

\****************************************************************************/

#include "FunctionDescriptor.h"
#include "Type.h"
#include <sstream>

namespace reflection{

///////////////////////////////////////////////////////////////////////////////
//Purpose: helps to determines the vector width of a given function descriptor
//Note: we could in fact use plain poly-morphism here, (no covariance problem
//here), but this will compel us to add the getWidth method throughout the Type
//inheritance tree, when it is only needed here.
///////////////////////////////////////////////////////////////////////////////
struct VWidthResolver: TypeVisitor{
  void visit(const Type*){
    m_width = width::SCALAR;
  }

  void visit(const Vector* v){
    m_width = static_cast<width::V>(v->getLen());
  }

  void visit(const Pointer* p){
    p->getPoitee()->accept(this);
  }

  width::V width()const{return m_width;}
private:
  width::V m_width;
};

std::string FunctionDescriptor::toString()const{
  std::stringstream stream;
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

static bool equal(const std::vector<Type*>& l, const std::vector<Type*>& r){
  if (&l == &r)
    return true;
  if (l.size() != r.size())
    return false;
  std::vector<Type*>::const_iterator itl = l.begin(), itr = r.begin(),
  endl = l.end();
  while(itl != endl){
    if (!(*itl)->equals(*itr))
      return false;
    ++itl;
    ++itr;
  }
  return true;
}

bool FunctionDescriptor::operator == (const FunctionDescriptor& that)const{
  if (this == &that)
    return true;
  if (name != that.name)
    return false;
  return equal(parameters, that.parameters);
}

width::V FunctionDescriptor::getWidth()const{
  VWidthResolver widthResolver;
  width::V ret = width::SCALAR;
  size_t paramCount = parameters.size();
  for (size_t i=0 ; i<paramCount ; ++i){
    parameters[i]->accept(&widthResolver);
    if (ret < widthResolver.width())
      ret = widthResolver.width();
  }
  return ret;
}

}
