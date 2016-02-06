/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef __VECTORIZER_FUNCTION_H__
#define __VECTORIZER_FUNCTION_H__

#include <string>

namespace reflection{
  struct FunctionDescriptor;
}

namespace intel{

struct hashEntry;

///////////////////////////////////////////////////////////////////////////////
//Purpose: Provides vectorizer-specific data on a (built-in) function
///////////////////////////////////////////////////////////////////////////////
struct VectorizerFunction{
  virtual ~VectorizerFunction(){}
  /////////////////////////////////////////////////////////////////////////////
  //Purpose: returns the width of the function represented by the receiver of
  //this object.
  /////////////////////////////////////////////////////////////////////////////
  virtual unsigned getWidth()const = 0;

  /////////////////////////////////////////////////////////////////////////////
  //Purpose: indicates whether the function represented by the receiver of
  //this object is packetizable (i.e., can it be widened to a semantically
  //equivalent function, with a different width).
  /////////////////////////////////////////////////////////////////////////////
  virtual bool isPacketizable()const = 0;

  /////////////////////////////////////////////////////////////////////////////
  //Purpose: indicates whether the function represented by the receiver of
  //this object is scalarizing (i.e., can it be replaced with a scalar
  //version, semantically equivalent to this function).
  /////////////////////////////////////////////////////////////////////////////
  virtual bool isScalarizable()const = 0;

  /////////////////////////////////////////////////////////////////////////////
  //Purpose: returns the mangled string of the built-in function which is
  //equivalent to this one, in the requested version.
  //Note: if the receiver represents a function that does not have the requested
  //version, FunctionDescriptor::nullString() is returned.
  /////////////////////////////////////////////////////////////////////////////
  virtual std::string getVersion(unsigned)const = 0;

  /////////////////////////////////////////////////////////////////////////////
  //Purpose: indicates whether the receiver represents a valid value
  /////////////////////////////////////////////////////////////////////////////
  virtual bool isNull()const = 0;
};

struct VFunctionFactory{
  virtual VectorizerFunction* create(const char*) = 0;
};

}
#endif//__VECTORIZER_FUNCTION_H__
