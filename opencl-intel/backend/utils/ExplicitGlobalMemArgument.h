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

#ifndef __EXPLICIT_GLOBAL_MEM_ARGUMENT_H__
#define __EXPLICIT_GLOBAL_MEM_ARGUMENT_H__

#include "ExplicitArgument.h"
#include "cl_types.h"

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

// TODO : rename this class? it is used both for global and constsant mem args
class ExplicitGlobalMemArgument : public ExplicitArgument {

public:
  /// @brief Constructor
  /// @param pValue           Implict argument's value destination pointer
  /// @param arg              OpenCL argument
  ExplicitGlobalMemArgument(char *pValue, const llvm::KernelArgument &arg)
      : ExplicitArgument(pValue, arg) {}

  /// @brief Overriding implementation
  /// @brief Sets the value of this argument
  /// @param pValueSrc       The src from which to copy the value
  virtual void setValue(const char *pValue) override {
    // The src is a mem descriptor and we need to extract the pointer to memory
    // if it is an image, we suppose the imageAuxData instead
    const void *pGlobalData = nullptr;
    cl_mem_obj_descriptor *pMemObj =
        !pValue ? nullptr
                : (*reinterpret_cast<cl_mem_obj_descriptor **>(
                      const_cast<char *>(pValue)));
    if (nullptr != pMemObj) {
      if (pMemObj->memObjType == CL_MEM_OBJECT_BUFFER)
        pGlobalData = pMemObj->pData;
      else
        pGlobalData = pMemObj->imageAuxData;
    }
    ExplicitArgument::setValue(reinterpret_cast<char *>(&pGlobalData));
  }
};

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel

#endif // __EXPLICIT_GLOBAL_MEM_ARGUMENT_H__
