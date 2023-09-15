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

#include "Serializer.h"
#include <map>

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

SerializationStatus::SerializationStatus()
    : m_pJITAllocator(nullptr), m_pBackendFactory(nullptr), m_RuntimeVersion(1),
      m_LLVMVersion(32) {}

void SerializationStatus::SetPointerMark(const std::string &mark,
                                         void *pointer) {
  std::map<std::string, void *>::iterator it = m_marksMap.find(mark);
  if (m_marksMap.end() != it) {
    assert(false && "Mark already exist on the serialization status");
    return;
  }

  m_marksMap[mark] = pointer;
}

void *SerializationStatus::GetPointerMark(const std::string &mark) {
  std::map<std::string, void *>::iterator it = m_marksMap.find(mark);
  if (m_marksMap.end() == it) {
    assert(false && "Mark do not exist on the serialization status");
    return nullptr;
  }

  return m_marksMap[mark];
}

void SerializationStatus::SetJITAllocator(
    ICLDevBackendJITAllocator *pJITAllocator) {
  m_pJITAllocator = pJITAllocator;
}

ICLDevBackendJITAllocator *SerializationStatus::GetJITAllocator() {
  return m_pJITAllocator;
}

void SerializationStatus::SetBackendFactory(
    IAbstractBackendFactory *pBackendFactory) {
  m_pBackendFactory = pBackendFactory;
}

IAbstractBackendFactory *SerializationStatus::GetBackendFactory() {
  return m_pBackendFactory;
}

void SerializationStatus::SerializeVersion(IOutputStream &stream) {
  Serializer::SerialPrimitive<int>(&m_RuntimeVersion, stream);
  Serializer::SerialPrimitive<int>(&m_LLVMVersion, stream);
}

void SerializationStatus::DeserialVersion(IInputStream &stream) {
  Serializer::DeserialPrimitive<int>(&m_RuntimeVersion, stream);
  Serializer::DeserialPrimitive<int>(&m_LLVMVersion, stream);
}

int SerializationStatus::GetRuntimeVersion() const { return m_RuntimeVersion; }

int SerializationStatus::GetLLVMVersion() const { return m_LLVMVersion; }

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
