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

#ifndef __VECTORWIDTH_H__
#define __VECTORWIDTH_H__

#include "Exception.h"
#include "llvm/Support/DataTypes.h" // LLVM data types
#include <map>

namespace Validation {
/// Number of components in vector.
/// Notation V#
/// # - number of components in vector.
/// VectorWidth's first element must be 0 and last element must be INVALID_WIDTH
enum VectorWidth { V1 = 0, V2, V3, V4, V8, V16, INVALID_WIDTH };

/// helper wrapper class for
class VectorWidthWrapper {
public:
  /// ctors
  VectorWidthWrapper() : m_value(INVALID_WIDTH) {}
  explicit VectorWidthWrapper(const VectorWidth &value) : m_value(value) {
    // there is metadata for value
    if (m_metaData.count(m_value) < 1) {
      throw Exception::InvalidArgument(
          "Invalid arg. No metadata for this VectorWidth");
    }
  }

  VectorWidth GetValue() const { return m_value; }
  void SetValue(const VectorWidth &value) { m_value = value; }
  std::size_t GetSize() const { return m_metaData[m_value].m_size; }
  std::string ToString() const { return m_metaData[m_value].m_toString; }

  static VectorWidth ValueOf(const std::string &str) {
    // need to notice that VectorWidth's first element must be 0 and last
    // element must be INVALID_WIDTH should put a comment in the VectorWidth
    // enum definition
    for (int i = 0; i < INVALID_WIDTH; i++) {
      VectorWidth vectorWidth = (VectorWidth)i;
      // should consider  comparing the strings converted to lower case to allow
      // more flexibility
      if (m_metaData[vectorWidth].m_toString == str) {
        return vectorWidth;
      }
    }
    throw Exception::InvalidArgument("NonSupported Vector Width");
  }

  static VectorWidth ValueOf(const std::size_t &val) {
    // need to notice that VectorWidth's first element must be 0 and last
    // element must be INVALID_WIDTH should put a comment in the VectorWidth
    // enum definition
    for (int i = 0; i < INVALID_WIDTH; i++) {
      VectorWidth vectorWidth = (VectorWidth)i;
      // should consider  comparing the strings converted to lower case to allow
      // more flexibility
      if (m_metaData[vectorWidth].m_size == (std::size_t)val) {
        return vectorWidth;
      }
    }
    throw Exception::InvalidArgument("NonSupported Vector Width");
  }

private:
  /// VectorWidth value
  VectorWidth m_value;

  /// helper class to store associated data with VectorWidth
  class VectorWidthMetadata {
  public:
    VectorWidthMetadata(std::size_t size, const std::string &toString)
        : m_size(size), m_toString(toString) {}

    VectorWidthMetadata() : m_size(0), m_toString("") {}

    /// Number of elements in vector.
    std::size_t m_size;
    /// VectorWidth value as string
    std::string m_toString;
  };

  /// static map from NEATValue to its Metadata
  typedef std::map<VectorWidth, VectorWidthMetadata> VectorWidthMetadataMap;
  static VectorWidthMetadataMap m_metaData;

  /// init static members
  static VectorWidthMetadataMap initStaticMap();
};

} // namespace Validation
#endif // __VECTORWIDTH_H__
