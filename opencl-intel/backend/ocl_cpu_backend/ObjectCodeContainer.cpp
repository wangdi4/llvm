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

#include "ObjectCodeContainer.h"
#include "cl_device_api.h"

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

ObjectCodeContainer::ObjectCodeContainer(const void *pBinary,
                                         size_t uiBinarySize) {
  assert(pBinary && "Code container pointer must be valid");
  m_pBinary = new char[uiBinarySize];
  m_uiBinarySize = uiBinarySize;
  std::copy((const char *)pBinary, (const char *)pBinary + uiBinarySize,
            m_pBinary);
}

ObjectCodeContainer::~ObjectCodeContainer() { delete[] m_pBinary; }

const void *ObjectCodeContainer::GetCode() const {
  return (const void *)m_pBinary;
}

size_t ObjectCodeContainer::GetCodeSize() const { return m_uiBinarySize; }

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
