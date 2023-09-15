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

#ifndef __EXPLICITBLOCKLITERAL_ARGUMENT_H__
#define __EXPLICITBLOCKLITERAL_ARGUMENT_H__

#include "cl_device_api.h"
#include "llvm/Transforms/SYCLTransforms/Utils/ImplicitArgsUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/TypeAlignment.h"

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

struct BlockLiteral;

/// OpenCL2.0.
/// BlockLiteral argument. Passed as 0th argument in enqueued kernel
class ExplicitBlockLiteralArgument : public ExplicitArgument {
  const BlockLiteral *m_pBL;

public:
  /// @brief Ctor saves BlockLiteral ptr to be used in setValue
  /// @param pValue           Implict argument's value destination pointer
  /// @param arg              OpenCL argument
  /// @param pBL              BlockLiteral pointers
  ExplicitBlockLiteralArgument(char *pValue, const llvm::KernelArgument &arg,
                               const BlockLiteral *pBL)
      : ExplicitArgument(pValue, arg), m_pBL(pBL) {}

  /// @brief Set argument as pointer to BlockLiteral structure.
  ///        Hack for using inside Binary::InitParams():
  ///        actual BlockLiteral pointer is obtained via ctor
  /// @param   is ignored
  virtual void setValue(const char *) override {
    ExplicitArgument::setValue((const char *)&m_pBL);
  }
};

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel

#endif // __EXPLICITBLOCKLITERAL_ARGUMENT_H__
