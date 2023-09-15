// INTEL CONFIDENTIAL
//
// Copyright 2013 Intel Corporation.
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

#ifndef __IMAGE_TYPE_H__
#define __IMAGE_TYPE_H__

#include "Exception.h"
#include "TypeDesc.h"
#include "dxfloat.h"
#include <cassert>
#include <cstddef> // for std::size_t
#include <map>
#include <string>

namespace Validation {
enum ImageTypeVal {
  OpenCL_MEM_OBJECT_IMAGE1D = 0,
  OpenCL_MEM_OBJECT_IMAGE1D_BUFFER,
  OpenCL_MEM_OBJECT_IMAGE1D_ARRAY,
  OpenCL_MEM_OBJECT_IMAGE2D,
  OpenCL_MEM_OBJECT_IMAGE2D_ARRAY,
  OpenCL_MEM_OBJECT_IMAGE3D,

  UNSPECIFIED_MEM_OBJECT_IMAGE,
  INVALID_MEM_OBJECT_IMAGE
};

class ImageTypeValWrapper {
public:
  ImageTypeValWrapper() : m_value(INVALID_MEM_OBJECT_IMAGE) {}
  explicit ImageTypeValWrapper(const ImageTypeVal &val) : m_value(val) {
    // if there is no metadata for value
    CheckValueAndThrow(val);
  }
  ImageTypeVal GetValue() const { return m_value; }

  void SetValue(ImageTypeVal val) {
    // if there is no metadata for value
    CheckValueAndThrow(val);
    m_value = val;
  }

  std::size_t GetDimentionCount() const {
    return m_metaData[m_value].m_Dimentions;
  }
  bool IsArray() const { return m_metaData[m_value].m_isArray; }
  std::string ToString() const { return m_metaData[m_value].m_toString; }

  static ImageTypeVal ValueOf(const std::string &str) {
    // ImageChannelDataTypeVal first element must be 0 and last element must be
    // INVALID_CHANNEL_ORDER
    for (int i = 0; i < INVALID_MEM_OBJECT_IMAGE; i++) {
      ImageTypeVal dt = (ImageTypeVal)i;
      // should consider  comparing the strings converted to lower case to allow
      // more flexibility
      if (m_metaData[dt].m_toString == str) {
        return dt;
      }
    }
    throw Exception::InvalidArgument("NonSupported Image Channel Data Type " +
                                     str);
  }

  inline bool operator==(const ImageTypeVal &a) const { return (a == m_value); }

  inline bool operator==(const ImageTypeValWrapper &a) const {
    return (a.m_value == m_value);
  }

private:
  ImageTypeVal m_value;

  inline void CheckValueAndThrow(const ImageTypeVal &in_value) {
    if (m_metaData.count(in_value) < 1) {
      throw Exception::InvalidArgument(
          "Invalid arg. No metadata for this ImageChannelDataType");
    }
  }

  class ImageTypeValMetadata {
  public:
    ImageTypeValMetadata()
        : m_Dimentions(0), m_isArray(false), m_toString("") {}

    ImageTypeValMetadata(std::size_t Dimentions, bool isArray,
                         const std::string &toString)
        : m_Dimentions(Dimentions), m_isArray(isArray), m_toString(toString) {}

    std::size_t m_Dimentions;
    bool m_isArray;
    std::string m_toString;
  };
  typedef std::map<ImageTypeVal, ImageTypeValMetadata> ImageTypeValMetadataMap;
  static ImageTypeValMetadataMap m_metaData;
  static ImageTypeValMetadataMap initStaticMap();
};

} // namespace Validation

#endif // __IMAGE_TYPE_H__
