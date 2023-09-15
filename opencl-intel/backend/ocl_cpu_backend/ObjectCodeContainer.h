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

#pragma once

#include "cl_dev_backend_api.h"
#include "cl_types.h"

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

/**
 * Represents the container for Binary Object which contains serialized data
 * for the whole ocl program
 */
class ObjectCodeContainer : public ICLDevBackendCodeContainer {
public:
  ObjectCodeContainer(const void *pBinary, size_t uiBinarySize);
  ~ObjectCodeContainer();

  const void *GetCode() const override;
  size_t GetCodeSize() const override;

private:
  char *m_pBinary;
  size_t m_uiBinarySize;

  // Klockwork Issue
  ObjectCodeContainer(const ObjectCodeContainer &x);

  // Klockwork Issue
  ObjectCodeContainer &operator=(const ObjectCodeContainer &x);
};

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
