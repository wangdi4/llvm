// INTEL CONFIDENTIAL
//
// Copyright 2006-2019 Intel Corporation.
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

#include "svm_buffer.h"

namespace Intel {
namespace OpenCL {
namespace Framework {

/**
 * This class represents a unified shared memory (USM) buffer.
 */
class USMBuffer : public SharedBuffer {
public:
  PREPARE_SHARED_PTR(USMBuffer)

  /**
   * @param pContext Context used to create the USM buffer.
   * @return a newly allocated USMBuffer.
   */
  static SharedPtr<USMBuffer> Allocate(const SharedPtr<Context> &pContext) {
    return new USMBuffer(pContext);
  }

  void SetType(cl_unified_shared_memory_type_intel type) { m_type = type; }
  cl_unified_shared_memory_type_intel GetType() const    { return m_type; }

  void SetFlags(cl_mem_alloc_flags_intel flags) { m_flags = flags; }
  cl_mem_alloc_flags_intel GetFlags() const     { return m_flags; }

  void SetDevice(cl_device_id device) { m_device = device; }
  cl_device_id GetDevice() const      { return m_device; }

private:
  USMBuffer(const SharedPtr<Context> &context) : SharedBuffer(context),
    m_device(nullptr), m_flags(0), m_type(CL_MEM_TYPE_UNKNOWN_INTEL) {}

  cl_device_id                        m_device;
  cl_mem_alloc_flags_intel            m_flags;
  cl_unified_shared_memory_type_intel m_type;
};

} // namespace Framework
} // namespace OpenCL
} // namespace Intel
