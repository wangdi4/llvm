// INTEL CONFIDENTIAL
//
// Copyright 2011-2018 Intel Corporation.
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

#ifndef __IMAGE_CHANNEL_DATA_TYPE_H__
#define __IMAGE_CHANNEL_DATA_TYPE_H__

#include "Exception.h"
#include "TypeDesc.h"
#include "dxfloat.h"
#include <cassert>
#include <cstddef> // for std::size_t
#include <map>
#include <string>

namespace Validation {
/// Structure describing type of elements in Buffer
/// ImageChannelDataTypeVal's first element must be 0 and last element must be
/// INVALID_CHANNEL_ORDER
enum ImageChannelDataTypeVal {
  OpenCL_SNORM_INT8 = 0,
  OpenCL_SNORM_INT16,
  OpenCL_UNORM_INT8,
  OpenCL_UNORM_INT16,
  OpenCL_UNORM_SHORT_565,
  OpenCL_UNORM_SHORT_555,
  OpenCL_UNORM_INT_101010,
  OpenCL_SIGNED_INT8,
  OpenCL_SIGNED_INT16,
  OpenCL_SIGNED_INT32,
  OpenCL_UNSIGNED_INT8,
  OpenCL_UNSIGNED_INT16,
  OpenCL_UNSIGNED_INT32,
  OpenCL_HALF_FLOAT,
  OpenCL_FLOAT,

  UNSPECIFIED_IMAGE_DATA_TYPE,
  INVALID_IMAGE_DATA_TYPE
};

/// helper class to map OpenCL pixel formats to C types
template <ImageChannelDataTypeVal T> class ImageChannelDataTypeValToCType;

#define DEF_IMAGE_CHANNELTYPE_TO_CTYPE(_CTYPE, _PIXTYPE)                       \
  template <> class ImageChannelDataTypeValToCType<_PIXTYPE> {                 \
  public:                                                                      \
    typedef _CTYPE type;                                                       \
  };

DEF_IMAGE_CHANNELTYPE_TO_CTYPE(int8_t, OpenCL_SNORM_INT8)
DEF_IMAGE_CHANNELTYPE_TO_CTYPE(int16_t, OpenCL_SNORM_INT16)
DEF_IMAGE_CHANNELTYPE_TO_CTYPE(uint8_t, OpenCL_UNORM_INT8)
DEF_IMAGE_CHANNELTYPE_TO_CTYPE(uint16_t, OpenCL_UNORM_INT16)
DEF_IMAGE_CHANNELTYPE_TO_CTYPE(uint16_t, OpenCL_UNORM_SHORT_565)
DEF_IMAGE_CHANNELTYPE_TO_CTYPE(uint16_t, OpenCL_UNORM_SHORT_555)
DEF_IMAGE_CHANNELTYPE_TO_CTYPE(uint32_t, OpenCL_UNORM_INT_101010)
DEF_IMAGE_CHANNELTYPE_TO_CTYPE(int8_t, OpenCL_SIGNED_INT8)
DEF_IMAGE_CHANNELTYPE_TO_CTYPE(int16_t, OpenCL_SIGNED_INT16)
DEF_IMAGE_CHANNELTYPE_TO_CTYPE(int32_t, OpenCL_SIGNED_INT32)
DEF_IMAGE_CHANNELTYPE_TO_CTYPE(uint8_t, OpenCL_UNSIGNED_INT8)
DEF_IMAGE_CHANNELTYPE_TO_CTYPE(uint16_t, OpenCL_UNSIGNED_INT16)
DEF_IMAGE_CHANNELTYPE_TO_CTYPE(uint32_t, OpenCL_UNSIGNED_INT32)
DEF_IMAGE_CHANNELTYPE_TO_CTYPE(CFloat16, OpenCL_HALF_FLOAT)
DEF_IMAGE_CHANNELTYPE_TO_CTYPE(float, OpenCL_FLOAT)

/// helper function to convert OpenCL pixel format to  TypeDesc::TypeVal type
inline TypeVal
ImageChannelDataTypeToTypeVal(const ImageChannelDataTypeVal &val) {
  TypeVal ret;
  switch (val) {
  case OpenCL_SNORM_INT8:
    ret = TCHAR;
    break;
  case OpenCL_SNORM_INT16:
    ret = TSHORT;
    break;
  case OpenCL_UNORM_INT8:
    ret = TUCHAR;
    break;
  case OpenCL_UNORM_INT16:
    ret = TUSHORT;
    break;
  case OpenCL_UNORM_SHORT_565:
    ret = TUSHORT;
    break;
  case OpenCL_UNORM_SHORT_555:
    ret = TUSHORT;
    break;
  case OpenCL_UNORM_INT_101010:
    ret = TUINT;
    break;
  case OpenCL_SIGNED_INT8:
    ret = TCHAR;
    break;
  case OpenCL_SIGNED_INT16:
    ret = TSHORT;
    break;
  case OpenCL_SIGNED_INT32:
    ret = TINT;
    break;
  case OpenCL_UNSIGNED_INT8:
    ret = TUCHAR;
    break;
  case OpenCL_UNSIGNED_INT16:
    ret = TUSHORT;
    break;
  case OpenCL_UNSIGNED_INT32:
    ret = TUINT;
    break;
  case OpenCL_HALF_FLOAT:
    ret = THALF;
    break;
  case OpenCL_FLOAT:
    ret = TFLOAT;
    break;
  default:
    throw Exception::InvalidArgument("");
  }
  return ret;
}

/// Data type container and its helper functions
class ImageChannelDataTypeValWrapper {
public:
  /// default ctor
  ImageChannelDataTypeValWrapper() : m_value(UNSPECIFIED_IMAGE_DATA_TYPE) {}

  explicit ImageChannelDataTypeValWrapper(const ImageChannelDataTypeVal &value)
      : m_value(value) {
    // if there is no metadata for value
    CheckValueAndThrow(value);
  }

  ImageChannelDataTypeVal GetValue() const { return m_value; }

  void SetValue(ImageChannelDataTypeVal val) {
    // if there is no metadata for value
    CheckValueAndThrow(val);
    m_value = val;
  }

  std::size_t GetSize() const { return m_metaData[m_value].m_size; }
  bool IsFloatingPoint() const { return m_metaData[m_value].m_isFloatingPoint; }
  std::string ToString() const { return m_metaData[m_value].m_toString; }

  static ImageChannelDataTypeVal ValueOf(const std::string &str) {
    // ImageChannelDataTypeVal first element must be 0 and last element must be
    // INVALID_CHANNEL_ORDER
    for (int i = 0; i < INVALID_IMAGE_DATA_TYPE; i++) {
      ImageChannelDataTypeVal dt = (ImageChannelDataTypeVal)i;
      // should consider  comparing the strings converted to lower case to allow
      // more flexibility
      if (m_metaData[dt].m_toString == str) {
        return dt;
      }
    }
    throw Exception::InvalidArgument("NonSupported Image Channel Data Type " +
                                     str);
  }

  inline bool operator==(const ImageChannelDataTypeValWrapper &a) const {
    return (a.m_value == m_value);
  }

private:
  ImageChannelDataTypeVal m_value;

  inline void CheckValueAndThrow(const ImageChannelDataTypeVal &in_value) {
    if (m_metaData.count(in_value) < 1) {
      throw Exception::InvalidArgument(
          "Invalid arg. No metadata for this ImageChannelDataType");
    }
  }

  class ImageChannelDataTypeMetadata {
  public:
    ImageChannelDataTypeMetadata()
        : m_size(0), m_isFloatingPoint(false), m_toString("") {}

    ImageChannelDataTypeMetadata(std::size_t size, bool isFloatingPoint,
                                 const std::string &toString)
        : m_size(size), m_isFloatingPoint(isFloatingPoint),
          m_toString(toString) {}

    std::size_t m_size;
    bool m_isFloatingPoint;
    std::string m_toString;
  };
  typedef std::map<ImageChannelDataTypeVal, ImageChannelDataTypeMetadata>
      ImageChannelDataTypeMetadataMap;
  static ImageChannelDataTypeMetadataMap m_metaData;
  static ImageChannelDataTypeMetadataMap initStaticMap();
};

} // namespace Validation

#endif // __IMAGE_CHANNEL_DATA_TYPE_H__
