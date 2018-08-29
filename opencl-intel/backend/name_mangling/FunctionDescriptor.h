// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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

#ifndef __FDESCRIPTOR_H__
#define __FDESCRIPTOR_H__

#include "ParameterType.h"
#include "Refcount.h"
#include "llvm/ADT/StringRef.h"
#include <string>
#include <vector>

namespace reflection {

namespace width {

enum V {
  NONE = 0,
  SCALAR = 1,
  TWO = 2,
  THREE = 3,
  FOUR = 4,
  EIGHT = 8,
  SIXTEEN = 16
};
const size_t OCL_VERSIONS = 6;
}

#define INVALID_ENTRY "<invalid>"

typedef std::vector<intel::RefCount<ParamType>> TypeVector;

struct FunctionDescriptor {
  //
  // returns: a human readable string representation of the function's
  // prototype.
  std::string toString() const;
  // The name of the function (stripped).
  std::string name;
  // Parameter list of the function
  TypeVector parameters;
  //'version width'; the width to which this function is suitable for
  width::V width;

  bool operator==(const FunctionDescriptor &) const;

  // enables function descriptors to serve as keys in stl maps.
  bool operator<(const FunctionDescriptor &) const;
  void assignAutomaticWidth();
  bool isNull() const;

  // create a singular value, that represents a 'null' FunctionDescriptor
  static FunctionDescriptor null();

  static llvm::StringRef nullString();
};

template <typename T>
std::ostream &operator<<(T &o, const reflection::FunctionDescriptor &fd) {
  o << fd.toString();
  return o;
}
} // end reflection

#endif
