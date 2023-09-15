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

#ifndef __BINARY_DATA_READER_H__
#define __BINARY_DATA_READER_H__

#include "Buffer.h"
#include "DataVersion.h"
#include "Exception.h"
#include "IBufferContainerList.h"
#include "IContainerVisitor.h"
#include "IDataReader.h"
#include "IMemoryObject.h"
#include "Image.h"
#include "llvm/Support/DataTypes.h"
#include <fstream>
#include <string>
#include <vector>

namespace Validation {
/// @brief IBufferContainerList binary reader
class BinaryContainerListReader : public IDataReader {
public:
  /// @brief Ctor
  /// @param [in] - filename to read the from
  BinaryContainerListReader(const std::string &filename)
      : m_stream(filename.c_str(), std::ios_base::in | std::ios_base::binary) {
    if (m_stream.fail())
      throw Exception::IOError("BinaryContainerListReader: cannot open file " +
                               filename);
  }

  /// @brief reads the file data and populate the given container
  /// @param [in] - pointer to IBufferContainerList
  void Read(IContainer *pContainer) override {
    assert(NULL != pContainer);
    readBufferContainerList(static_cast<IBufferContainerList *>(pContainer));
  }

private:
  template <class T> void readValue(T &value) {
    m_stream.read((char *)&value, sizeof(T));
    if (m_stream.fail())
      throw Exception::IOError("BinaryContainerListReader: read failed");
  }

  void readBufferContainerList(IBufferContainerList *pContainerList) {
    assert(NULL != pContainerList);

    pContainerList->SetDataVersion(readDataVersion());

    uint32_t containerCount = 0;
    readValue(containerCount);
    for (uint32_t i = 0; i < containerCount; ++i) {
      IBufferContainer *pBufferContainer =
          pContainerList->CreateBufferContainer();
      readBufferContainer(pBufferContainer);
    }
  }

  void readMarker(std::string &marker) {
    uint32_t markerSize;
    readValue(markerSize);
    std::vector<char> markerVal(markerSize + 1);
    markerVal[markerSize] = '\0';
    m_stream.read(&markerVal[0], markerSize);
    if (m_stream.gcount() != markerSize) {
      throw Exception::IOError(
          "Can't read memory object marker from the data file.");
    }
    marker = std::string(&markerVal[0]);
  }

  void readElementDesc(TypeDesc &td) {
    uint32_t typeValue;
    readValue(typeValue);
    td.SetType(TypeVal(typeValue));
    uint32_t sizeInBytes;
    readValue(sizeInBytes);
    td.SetTypeAllocSize(sizeInBytes);

    if (td.IsAggregate() || td.IsPointer()) {
      uint64_t numOfSubElements;
      readValue(numOfSubElements);
      td.SetNumberOfElements(numOfSubElements);

      uint64_t numOfSubTypes;
      readValue(numOfSubTypes);
      td.SetNumOfSubTypes(numOfSubTypes);

      TypeDesc subTypeDesc;
      readElementDesc(subTypeDesc);
      td.SetSubTypeDesc(0, subTypeDesc);
      if (td.IsStruct()) {
        uint64_t offset;
        readValue(offset);
        subTypeDesc.SetOffsetInStruct(offset);
        td.SetSubTypeDesc(0, subTypeDesc);
        for (uint64_t i = 1; i < numOfSubTypes; ++i) {
          TypeDesc subTD;
          readElementDesc(subTD);
          readValue(offset);
          subTD.SetOffsetInStruct(offset);
          td.SetSubTypeDesc(i, subTD);
        }
      }
    }
  }

  uint32_t readDataVersion() {
    std::string signature = DataVersion::GetDataVersionSignature();
    uint32_t signatureSize = signature.size();

    uint32_t readVersion = 0;

    int pos = m_stream.tellg();

    std::string signatureVal(signatureSize, '\0');
    m_stream.read(&signatureVal[0], signatureSize);
    if (m_stream.fail())
      throw Exception::IOError(
          "readDataVersion: read data version signature failed");

    if (signatureVal == signature) {
      // read a constant number of decimal digits representing the data version
      // like 00001 or 00002
      std::string versionStr(DataVersion::GetNumOfDigits(), '\0');
      m_stream.read(&versionStr[0], versionStr.size());
      if (m_stream.fail())
        throw Exception::IOError("readDataVersion: read data version failed");

      std::stringstream ss(versionStr);
      ss >> readVersion;

    } else {
      m_stream.seekg(pos);
    }
    return readVersion;
  }

  void readBufferContainer(IBufferContainer *pContainer) {
    assert(NULL != pContainer);

    uint32_t bufferCount = 0;
    readValue(bufferCount);

    for (uint32_t i = 0; i < bufferCount; ++i) {
      std::string marker;
      readMarker(marker);

      if (marker == Buffer::GetBufferName()) {
        // TODO: Fix all size_t in writing/reading.
        // Potentially it is not portable.

        // first read the buffer descriptor
        uint32_t numOfElements;
        readValue(numOfElements);
        TypeDesc elemDesc;
        readElementDesc(elemDesc);
        uint32_t isNeat;
        readValue(isNeat);
        // then the buffer data
        BufferDesc bd;
        bd.SetNumOfElements((size_t)numOfElements);
        bd.SetElementDecs(elemDesc);
        bd.SetNeat(bool(isNeat));
        IMemoryObject *pBuffer = pContainer->CreateBuffer(bd);
        m_stream.read((char *)pBuffer->GetDataPtr(), bd.GetSizeInBytes());
      } else if (marker == Image::GetImageName()) {
        uint32_t signature; // it could be 2 or 3 for old version and it is
                            // ffffffff for other
        uint32_t imageType2Read;
        ImageTypeVal imageType;
        ImageSizeDesc sizes;
        ImageChannelOrderVal imageOrder;
        ImageChannelDataTypeVal imageDataType;
        uint32_t pixelSize;
        bool isNEAT;
        readValue(signature);
        if (signature == 0xffffffff) {
          // OpenCL 1.2 and higher
          uint32_t versionHigh;
          uint32_t versionLow;
          readValue(versionHigh);
          readValue(versionLow);
          readValue(imageType2Read);
          imageType = (ImageTypeVal)imageType2Read;
          readValue(sizes);
        } else {
          // OpenCL 1.1 read
          ImageSizeDesc_1_1 oldSizes;
          // here signature actually is num of dimensions

          bool isArray = false; // no image arrays supported by OpenCL 1.1
          imageType = GetImageTypeFromDimCount(signature, isArray);
          readValue(oldSizes);

          sizes.width = oldSizes.width;
          sizes.height = oldSizes.height;
          sizes.depth = oldSizes.depth;
          sizes.row = oldSizes.row;
          sizes.slice = oldSizes.slice;
        }
        readValue(imageOrder);
        readValue(imageDataType);
        readValue(pixelSize);
        readValue(isNEAT);

        ImageDesc imd(imageType, sizes, imageDataType, imageOrder, isNEAT);
        assert((size_t)pixelSize == imd.GetElementSize());

        IMemoryObject *pImage = pContainer->CreateImage(imd);
        m_stream.read((char *)pImage->GetDataPtr(), imd.GetSizeInBytes());
      } else {
        throw Exception::IOError(
            std::string("Buffer container do not support ") + marker);
      }
    }
  }

private:
  std::fstream m_stream;
};
} // namespace Validation

#endif // __BINARY_DATA_READER_H__
