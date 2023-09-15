// INTEL CONFIDENTIAL
//
// Copyright 2010 Intel Corporation.
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

#ifndef __EXPLICIT_ARGUMENT_H__
#define __EXPLICIT_ARGUMENT_H__

#include "cl_device_api.h"
#include "llvm/Transforms/SYCLTransforms/Utils/ImplicitArgsUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/TypeAlignment.h"

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

class ExplicitArgument : public llvm::FunctionArgument {

public:
  /// @brief Constructor
  /// @param pValue           Explicit argument's value destination pointer
  /// @param arg              OpenCL argument
  ExplicitArgument(char *pValue, const llvm::KernelArgument &arg)
      : llvm::FunctionArgument(pValue, llvm::TypeAlignment::getSize(arg),
                               llvm::TypeAlignment::getAlignment(arg)) {}
};

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel

#endif // __EXPLICIT_ARGUMENT_H__
