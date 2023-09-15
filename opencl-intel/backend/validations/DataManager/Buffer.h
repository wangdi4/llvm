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

#ifndef __BUFFER_H__
#define __BUFFER_H__

#include "Exception.h"
#include "IMemoryObject.h"          // IMemoryObject declaration
#include "llvm/Support/DataTypes.h" // LLVM data types
#include <assert.h>

namespace Validation {
class BufferContainer;
class IContainerVisitor;

/// Class for containing data.
class Buffer : public IMemoryObject {
public:
  /// @brief Initializing ctor. Allocates memory for buffer's data using
  /// values from buffer description.
  Buffer(const BufferDesc &desc);

  virtual ~Buffer();

  Buffer &operator=(Buffer &) = delete;

  virtual void *GetDataPtr() const override { return (void *)m_data; }

  virtual const IMemoryObjectDesc *GetMemoryObjectDesc() const override {
    return &m_desc;
  }

  void Accept(IContainerVisitor &visitor) const override;

  virtual std::string GetName() const override { return GetBufferName(); }

  static std::string GetBufferName() { return std::string("Buffer"); }

private:
  /// hide copy constructor
  Buffer(const Buffer &)
      : IMemoryObject(), m_desc(0, INVALID_WIDTH, INVALID_DATA_TYPE, false) {}

  /// @brief Allocates memory for buffer's data using existing buffer
  /// description values
  void AllocateMemoryForData();
  /// Buffer's data values
  uint8_t *m_data;
  /// Buffer description containing types of values, size of buffer, etc.
  BufferDesc m_desc;
  /// declare friend
  friend class BufferContainer;
};

BufferDesc GetBufferDescription(const IMemoryObjectDesc *iDesc);

template <typename T> class BufferAccessor {
public:
  /// @brief ctor. Specify buffer that you want to work with
  BufferAccessor(const IMemoryObject &in_buf) : m_buf(in_buf) {
    BufferDesc m_desc = GetBufferDescription(in_buf.GetMemoryObjectDesc());
    if (m_desc.GetElementDescription().IsStruct())
      throw Exception::InvalidArgument(
          "Stuctures are not supported by buffer accessor");
    if (m_desc.GetElementDescription().IsAggregate() &&
        !m_desc.GetElementDescription().IsStruct() &&
        m_desc.GetElementDescription().GetSubTypeDesc(0).GetSizeInBytes() !=
            sizeof(T))
      throw Exception::InvalidArgument(
          "Buffer accessor type parameter size is different from element size "
          "in buffer desc");
    if (!m_desc.GetElementDescription().IsComposite() &&
        m_desc.GetElementDescription().GetSizeInBytes() != sizeof(T))
      throw Exception::InvalidArgument(
          "Buffer accessor type parameter size is different from element size "
          "in buffer desc");
    m_data = m_buf.GetDataPtr();
    // Calculate address
    m_SizeOfVector = m_desc.GetElementDescription().GetSizeInBytes();
    m_SizeOfVectorElementDataType =
        m_desc.GetElementDescription().IsComposite()
            ? m_desc.GetElementDescription().GetSubTypeDesc(0).GetSizeInBytes()
            : m_desc.GetElementDescription().GetSizeInBytes();
  }

  /// @brief Gets
  /// @param in_vecIndex  Index of vector that contains element
  /// @param in_index     Index of element in the vector
  /// As this is template class we need to place declaration here
  inline T &GetElem(const std::size_t in_vecIndex,
                    const std::size_t in_offset) const {
    // Calculate address
    T *dataPointer = reinterpret_cast<T *>(
        reinterpret_cast<char *>(m_data) + m_SizeOfVector * in_vecIndex +
        in_offset * m_SizeOfVectorElementDataType);
    T &res = *dataPointer;
    return res;
  }

  inline void SetElem(const std::size_t in_vecIndex,
                      const std::size_t in_offset, const T &newVal) {
    // Calculate address
    T *dataPointer = reinterpret_cast<T *>(
        reinterpret_cast<char *>(m_data) + m_SizeOfVector * in_vecIndex +
        in_offset * m_SizeOfVectorElementDataType);
    *dataPointer = newVal;
  }

  BufferAccessor(const BufferAccessor &b) = delete;
  BufferAccessor &operator=(const BufferAccessor &) = delete;

private:
  const IMemoryObject &m_buf;
  // pointer to data
  void *m_data;
  /// number of vector elements
  size_t m_SizeOfVector;
  /// size of vector element in bytes
  size_t m_SizeOfVectorElementDataType;
};

} // namespace Validation

#endif // __BUFFER_H__
