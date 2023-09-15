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

#ifndef __BINARY_DATA_WRITER_H__
#define __BINARY_DATA_WRITER_H__

#include "Buffer.h"
#include "BufferDesc.h"
#include "DataVersion.h"
#include "Exception.h"
#include "IBufferContainerList.h"
#include "IContainerVisitor.h"
#include "IDataWriter.h"
#include "Image.h"
#include "ImageDesc.h"
#include "llvm/Support/DataTypes.h"
#include <fstream>
#include <string>

namespace Validation {
/// @brief IBufferContainerList object writer to data file in Binary format
/// Implements IDataWriter interface
/// Usage:
///     Could be used in two main ways:
///     1.  BufferContainerList list;
///         BinaryContainerListWriter writer("filename");
///         writer.Write(&list);
///
///     2.  BinaryContainerListWriter writer("filename");
///         BufferContainerList list;
///         list.Accept(&writer);
///         writer.Flush();
class BinaryContainerListWriter : public IDataWriter, public IContainerVisitor {
public:
  /// @brief Ctor.
  /// @param [in] - fileName name of file to write to
  BinaryContainerListWriter(const std::string &filename)
      : m_stream(filename.c_str(), std::ios_base::out | std::ios_base::binary) {
  }

  /// @brief write data from IBufferContainerList object to stream
  /// @param [in] - pContainer pointer to object with IBufferContainerList
  /// interface
  void Write(const IContainer *pContainer) override {
    assert(NULL != pContainer);
    pContainer->Accept(*this);
    Flush();
  }

  /// @brief flushes the stream to the file
  /// @param [in] - pContainer pointer to object with IBufferContainerList
  /// interface
  void Flush() { m_stream.flush(); }

private:
  static const uint32_t imageSignature =
      0xffffffff; // signature to be written as a starting point of image
  static const uint32_t imageVersionHigh = 1; // OpenCL 1.2
  static const uint32_t imageVersionLow = 2;

  template <class T> void writeValue(T value) {
    m_stream.write((const char *)&value, sizeof(T));
  }

  void writeMarker(const std::string &marker) {
    uint32_t markerSize = (uint32_t)marker.size();
    m_stream.write((const char *)&markerSize, sizeof(uint32_t));
    m_stream.write((const char *)marker.c_str(), markerSize);
  }

  void writeDataVersion() {
    std::string signature = DataVersion::GetDataVersionSignature();
    uint32_t signatureSize = signature.size();
    m_stream.write((const char *)signature.c_str(), signatureSize);

    std::string version = DataVersion::GetCurrentDataVersionString();
    m_stream.write((const char *)version.c_str(), version.size());
  }

  void writeElementDesc(TypeDesc td) {
    writeValue((uint32_t)td.GetType());
    writeValue((uint32_t)td.GetSizeInBytes());
    // if we are saving vector, array or pointer data type, no need
    // to store description of all elements of array or vector. They are all the
    // same.
    if (td.IsAggregate() || td.IsPointer()) {
      uint64_t numOfSubElements = td.GetNumberOfElements();
      writeValue(numOfSubElements);

      uint64_t numOfSubTypes = td.GetNumOfSubTypes();
      writeValue(numOfSubTypes);
      writeElementDesc(td.GetSubTypeDesc(0));
      if (td.IsStruct()) {
        writeValue(td.GetSubTypeDesc(0).GetOffsetInStruct());
        for (uint64_t i = 1; i < numOfSubTypes; ++i) {
          writeElementDesc(td.GetSubTypeDesc(i));
          writeValue(td.GetSubTypeDesc(i).GetOffsetInStruct());
        }
      }
    }
  }

  void visitBuffer(const IMemoryObject *pBuffer) override {
    assert(NULL != pBuffer);

    // Write marker.
    writeMarker(pBuffer->GetName());

    // first we write down the descriptor
    BufferDesc bd = GetBufferDescription(pBuffer->GetMemoryObjectDesc());
    writeValue((uint32_t)bd.NumOfElements());
    writeElementDesc(bd.GetElementDescription());
    writeValue((uint32_t)bd.IsNEAT());

    size_t size = bd.GetSizeInBytes();
    // then the buffer itself
    m_stream.write((const char *)pBuffer->GetDataPtr(), size);
  }

  void visitImage(const IMemoryObject *pImage) override {
    assert(NULL != pImage);

    writeMarker(pImage->GetName());

    // first we write down the descriptor
    ImageDesc imDesc = GetImageDescription(pImage->GetMemoryObjectDesc());
    writeValue(imageSignature);
    writeValue(imageVersionHigh);
    writeValue(imageVersionLow);
    writeValue((uint32_t)imDesc.GetImageType());
    writeValue(imDesc.GetSizesDesc());
    writeValue(imDesc.GetImageChannelOrder());
    writeValue(imDesc.GetImageChannelDataType());
    writeValue((uint32_t)imDesc.GetElementSize());
    writeValue(imDesc.IsNEAT());
    // then the image itself
    m_stream.write((const char *)pImage->GetDataPtr(), imDesc.GetSizeInBytes());
  }

  void visitBufferContainer(const IBufferContainer *pBufferContainer) override {
    assert(NULL != pBufferContainer);
    writeValue((uint32_t)pBufferContainer->GetMemoryObjectCount());
  }

  void visitBufferContainerList(
      const IBufferContainerList *pBufferContainerList) override {
    assert(NULL != pBufferContainerList);
    writeDataVersion();
    writeValue((uint32_t)pBufferContainerList->GetBufferContainerCount());
  }

private:
  std::fstream m_stream;
};

} // namespace Validation

#endif // __BINARY_DATA_WRITER_H__
