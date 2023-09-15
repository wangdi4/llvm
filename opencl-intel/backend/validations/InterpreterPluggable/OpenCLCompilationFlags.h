// INTEL CONFIDENTIAL
//
// Copyright 2013 Intel Corporation.
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

#ifndef OPEN_CL_REF_COMPILATION_FLAGS_H
#define OPEN_CL_REF_COMPILATION_FLAGS_H

#include "llvm/IR/Module.h"
#include <string>

namespace Validation {

class CompilationFlags {

public:
  static unsigned getCLVersionFromMetadata(llvm::Module *M);
  static unsigned getCLVersionFromFlags(const std::string &Flags);

  static bool hasFastRelaxedMathFlag(const std::string &Flags);
  static bool hasUniformWGSizeFlag(const std::string &Flags);
};

} // namespace Validation

#endif
