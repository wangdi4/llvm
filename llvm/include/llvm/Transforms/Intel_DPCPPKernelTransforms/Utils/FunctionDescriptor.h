//===- FunctionDescriptor.h - Function descriptor utilities -----*- C++ -*-===//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef INTEL_DPCPP_KERNEL_TRANSFORMS_UTILS_FUNCTION_DESCRIPTOR_H
#define INTEL_DPCPP_KERNEL_TRANSFORMS_UTILS_FUNCTION_DESCRIPTOR_H

#include "ParameterType.h"
#include "llvm/ADT/StringRef.h"

namespace llvm {
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
} // namespace width

typedef std::vector<RefParamType> TypeVector;

struct FunctionDescriptor {
  // returns: a human readable string representation of the function's
  // prototype.
  std::string toString() const;
  // The name of the function (stripped).
  std::string Name;
  // Parameter list of the function
  TypeVector Parameters;
  //'version width'; the width to which this function is suitable for
  width::V Width = width::NONE;

  bool operator==(const FunctionDescriptor &) const;

  // enables function descriptors to serve as keys in stl maps.
  bool operator<(const FunctionDescriptor &) const;
  void assignAutomaticWidth();
  bool isNull() const;

  // create a singular value, that represents a 'null' FunctionDescriptor
  static FunctionDescriptor null();

  static StringRef nullString();
};

} // namespace reflection
} // namespace llvm

#endif // INTEL_DPCPP_KERNEL_TRANSFORMS_UTILS_FUNCTION_DESCRIPTOR_H
