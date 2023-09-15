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

#ifndef __DATATYPE_H__
#define __DATATYPE_H__

#include "Exception.h"
#include "llvm/Support/DataTypes.h" // LLVM data types
#include <assert.h>
#include <map>

namespace Validation {
/// Structure describing type of elements in Buffer
/// DataTypeVal's first element must be 0 and last element must be
/// INVALID_DATA_TYPE
enum DataTypeVal {
  // Floating point types
  F16 = 0,
  F32,
  F64,
  // Signed integer types
  I8,
  I16,
  I32,
  I64,
  // Unsigned integer types
  U8,
  U16,
  U32,
  U64,
  // marker for invalid datatype
  INVALID_DATA_TYPE,
  UNSPECIFIED_DATA_TYPE
};

/// Data type container and its helper functions
class DataTypeValWrapper {
public:
  /// default ctor
  DataTypeValWrapper() : m_value(INVALID_DATA_TYPE) {}

  explicit DataTypeValWrapper(const DataTypeVal &value) : m_value(value) {
    // if there is no metadata for value
    CheckValueAndThrow(value);
  }

  DataTypeVal GetValue() const { return m_value; }

  void SetValue(DataTypeVal val) {
    // if there is no metadata for value
    CheckValueAndThrow(val);
    m_value = val;
  }

  std::size_t GetSize() const { return m_metaData[m_value].m_size; }
  bool IsFloatingPoint() const { return m_metaData[m_value].m_isFloatingPoint; }
  std::string ToString() const { return m_metaData[m_value].m_toString; }

  static DataTypeVal ValueOf(const std::string &str) {
    // DataTypeVal first element must be 0 and last element must be
    // INVALID_DATA_TYPE
    for (int i = 0; i < INVALID_DATA_TYPE; i++) {
      DataTypeVal dt = (DataTypeVal)i;
      // should consider  comparing the strings converted to lower case to allow
      // more flexibility
      if (m_metaData[dt].m_toString == str) {
        return dt;
      }
    }
    throw Exception::InvalidArgument("NonSupported Data Type " + str);
  }

private:
  DataTypeVal m_value;

  inline void CheckValueAndThrow(const DataTypeVal &in_value) {
    if (m_metaData.count(in_value) < 1) {
      throw Exception::InvalidArgument(
          "Invalid arg. No metadata for this DataType");
    }
  }

  class DataTypeMetadata {
  public:
    DataTypeMetadata() : m_size(0), m_isFloatingPoint(false), m_toString("") {}

    DataTypeMetadata(std::size_t size, bool isFloatingPoint,
                     const std::string &toString)
        : m_size(size), m_isFloatingPoint(isFloatingPoint),
          m_toString(toString) {}

    std::size_t m_size;
    bool m_isFloatingPoint;
    std::string m_toString;
  };

  typedef std::map<DataTypeVal, DataTypeMetadata> DataTypeMetadataMap;
  static DataTypeMetadataMap m_metaData;
  static DataTypeMetadataMap initStaticMap();
};

} // namespace Validation

#endif // __DATATYPE_H__
