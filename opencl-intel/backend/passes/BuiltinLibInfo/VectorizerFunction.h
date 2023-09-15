// INTEL CONFIDENTIAL
//
// Copyright 2012 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#ifndef __VECTORIZER_FUNCTION_H__
#define __VECTORIZER_FUNCTION_H__

#include <string>

namespace intel {

struct hashEntry;

///////////////////////////////////////////////////////////////////////////////
// Purpose: Provides vectorizer-specific data on a (built-in) function
///////////////////////////////////////////////////////////////////////////////
struct VectorizerFunction {
  virtual ~VectorizerFunction() {}
  /////////////////////////////////////////////////////////////////////////////
  // Purpose: returns the width of the function represented by the receiver of
  // this object.
  /////////////////////////////////////////////////////////////////////////////
  virtual unsigned getWidth() const = 0;

  /////////////////////////////////////////////////////////////////////////////
  // Purpose: indicates whether the function represented by the receiver of
  // this object is packetizable (i.e., can it be widened to a semantically
  // equivalent function, with a different width).
  /////////////////////////////////////////////////////////////////////////////
  virtual bool isPacketizable() const = 0;

  /////////////////////////////////////////////////////////////////////////////
  // Purpose: indicates whether the function represented by the receiver of
  // this object is scalarizing (i.e., can it be replaced with a scalar
  // version, semantically equivalent to this function).
  /////////////////////////////////////////////////////////////////////////////
  virtual bool isScalarizable() const = 0;

  /////////////////////////////////////////////////////////////////////////////
  // Purpose: returns the mangled string of the built-in function which is
  // equivalent to this one, in the requested version.
  // Note!: until we fix it (due to legacy reasons), the versions are orginized
  // as follows:
  // 0 -> SCALAR, 1 -> TWO, 2 -> FOUR, 3 -> EIGHT, 4 -> SIXTEEN, 5 -> THREE
  // Note: if the receiver represents a function that does not have the
  // requested version, FunctionDescriptor::nullString() is returned.
  /////////////////////////////////////////////////////////////////////////////
  virtual std::string getVersion(unsigned) const = 0;

  /////////////////////////////////////////////////////////////////////////////
  // Purpose: indicates whether the receiver represents a valid value
  /////////////////////////////////////////////////////////////////////////////
  virtual bool isNull() const = 0;
};

struct VFunctionFactory {
  virtual VectorizerFunction *create(const char *) = 0;
  virtual ~VFunctionFactory() {}
};

} // namespace intel
#endif //__VECTORIZER_FUNCTION_H__
