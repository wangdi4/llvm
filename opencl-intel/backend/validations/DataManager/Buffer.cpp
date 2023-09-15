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

#include "Buffer.h"
#include "IContainerVisitor.h"
#include "llvm/Support/DataTypes.h"

namespace Validation {

///////////////////////////////////
/// Buffer implementation
///////////////////////////////////////
void Buffer::AllocateMemoryForData() {
  // Compute size of memory to allocate
  std::size_t buffSize = m_desc.GetSizeInBytes();
  // Allocate memory
  m_data = new uint8_t[buffSize];
}

Buffer::Buffer(const BufferDesc &desc) : m_desc(desc) {
  if (!m_desc.IsFloatingPoint() && m_desc.IsNEAT()) {
    throw Exception::InvalidArgument(
        "NEAT Buffer without floating point couldn't be created.");
  }
  // Allocate memory
  AllocateMemoryForData();
}

Buffer::~Buffer() {
  if (0 != m_data) {
    delete[] m_data;
  }
}

void Buffer::Accept(IContainerVisitor &visitor) const {
  visitor.visitBuffer(this);
}

BufferDesc GetBufferDescription(const IMemoryObjectDesc *iDesc) {
  // assert(NULL != dynamic_cast<const BufferDesc *>(iDesc) && "BufferDesc is
  // expected");
  return *static_cast<const BufferDesc *>(iDesc);
}

} // namespace Validation
