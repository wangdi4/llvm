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

#ifndef __IMAGE_CHANNEL_ORDER_H__
#define __IMAGE_CHANNEL_ORDER_H__

#include "Exception.h"
#include <cassert>
#include <cstddef> // for std::size_t
#include <map>
#include <string>

namespace Validation {
/// Structure describing type of elements in Buffer
/// ImageChannelOrderVal's first element must be 0 and last element must be
/// INVALID_CHANNEL_ORDER
enum ImageChannelOrderVal {
  OpenCL_R = 0,
  OpenCL_Rx,
  OpenCL_A,
  OpenCL_INTENSITY,
  OpenCL_LUMINANCE,
  OpenCL_RG,
  OpenCL_RGx,
  OpenCL_RA,
  OpenCL_RGB,
  OpenCL_RGBx,
  OpenCL_RGBA,
  OpenCL_ARGB,
  OpenCL_BGRA,
  OpenCL_DEPTH,
  OpenCL_sRGBA,
  OpenCL_sBGRA,

  // marker for invalid order
  UNSPECIFIED_CHANNEL_ORDER,
  INVALID_CHANNEL_ORDER
};

inline size_t GetChannelCount(const ImageChannelOrderVal &order) {
  switch (order) {
  case OpenCL_R:
  case OpenCL_A:
  case OpenCL_Rx:
  case OpenCL_INTENSITY:
  case OpenCL_LUMINANCE:
  case OpenCL_DEPTH:
    return 1;

  case OpenCL_RG:
  case OpenCL_RA:
  case OpenCL_RGx:
    return 2;

  case OpenCL_RGB:
  case OpenCL_RGBx:
    return 3;

  case OpenCL_RGBA:
  case OpenCL_ARGB:
  case OpenCL_BGRA:
  case OpenCL_sRGBA:
  case OpenCL_sBGRA:
    return 4;

  default:
    throw Exception::InvalidArgument(
        "GetChannelCount::Unknown ImageChannelOrderVal value");
  }
}
/// Data type container and its helper functions
class ImageChannelOrderValWrapper {
public:
  /// default ctor
  ImageChannelOrderValWrapper() : m_value(UNSPECIFIED_CHANNEL_ORDER) {}

  explicit ImageChannelOrderValWrapper(const ImageChannelOrderVal &value)
      : m_value(value) {
    CheckValueAndThrow(value);
  }

  ImageChannelOrderVal GetValue() const { return m_value; }

  void SetValue(ImageChannelOrderVal val) {
    // if there is no metadata for value
    CheckValueAndThrow(val);
    m_value = val;
  }

  /// @return number of channels
  std::size_t GetSize() const { return m_metaData[m_value].m_size; }
  /// @returns  Text representation of image channel order
  std::string ToString() const { return m_metaData[m_value].m_toString; }

  static ImageChannelOrderVal ValueOf(const std::string &str) {
    // ImageChannelOrderVal first element must be 0 and last element must be
    // INVALID_CHANNEL_ORDER
    for (int i = 0; i < INVALID_CHANNEL_ORDER; i++) {
      ImageChannelOrderVal dt = (ImageChannelOrderVal)i;
      // should consider  comparing the strings converted to lower case to allow
      // more flexibility
      if (m_metaData[dt].m_toString == str) {
        return dt;
      }
    }
    throw Exception::InvalidArgument("NonSupported Image channel order " + str);
  };

  inline bool operator==(const ImageChannelOrderValWrapper &a) const {
    return (a.m_value == m_value);
  }

private:
  ImageChannelOrderVal m_value;

  inline void CheckValueAndThrow(const ImageChannelOrderVal &in_value) {
    if (m_metaData.count(in_value) < 1) {
      throw Exception::InvalidArgument(
          "Invalid arg. No metadata for this ImageChannelOrder");
    }
  }

  class ImageChannelOrderMetadata {
  public:
    ImageChannelOrderMetadata() : m_size(0), m_toString(""){};

    ImageChannelOrderMetadata(std::size_t size, const std::string &toString)
        : m_size(size), m_toString(toString){};

    std::size_t m_size;
    std::string m_toString;
  };

  typedef std::map<ImageChannelOrderVal, ImageChannelOrderMetadata>
      ImageChannelOrderMetadataMap;
  static ImageChannelOrderMetadataMap m_metaData;
  static ImageChannelOrderMetadataMap initStaticMap();
};

} // namespace Validation

#endif // __IMAGE_CHANNEL_ORDER_H__
