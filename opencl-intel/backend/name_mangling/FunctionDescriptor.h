/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
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
