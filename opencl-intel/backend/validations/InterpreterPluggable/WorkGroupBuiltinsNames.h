// INTEL CONFIDENTIAL
//
// Copyright 2011 Intel Corporation.
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

#ifndef WORK_GROUP_BUILTINS_NAMES
#define WORK_GROUP_BUILTINS_NAMES

#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Type.h"
#include "llvm/Transforms/SYCLTransforms/Utils/FunctionDescriptor.h"
#include <map>
#include <string>

namespace Validation {

/// helper class for finding workgroup builtins
class WorkGroupBultinsNames {
public:
  // ctor
  WorkGroupBultinsNames();
  ///@brief determinates whether given mangled function name belongs
  /// to work group builtins names or not.
  ///@param [in] MangledName - mangled name to be processed
  ///@return is MangledName belongs to work group builtins
  bool isWorkGroupBuiltin(std::string MangledName);
  ///@brief constructs mangled name of pre-exec method for given function
  ///@param [in] MangeldName - mangled name of function for which you want to
  /// construct mangled pre-exec method name
  //@return mangled name of pre-exec method
  std::string getMangledPreExecMethodName(std::string MangeldName);

private:
  typedef std::map<std::string, llvm::reflection::FunctionDescriptor>
      WGBuiltinsNamesMap;
  typedef std::pair<std::string, llvm::reflection::FunctionDescriptor>
      WGBuiltinsNamesDesc;
  WGBuiltinsNamesMap m_MangledNames;
};
} // namespace Validation

#endif
