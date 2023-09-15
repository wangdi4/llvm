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

#include "NEATValue.h"

namespace Validation {
/// assignment o-r
NEATValue &NEATValue::operator=(const NEATValue &p) {
  if (this != &p) { // make sure not same object
    m_Status = p.m_Status;
    // m_Type   = p.m_Type;
    memcpy(m_min, p.m_min, MaxBytes);
    memcpy(m_max, p.m_max, MaxBytes);
  }
  return *this;
}

void NEATValue::SetStatus(Status in_Status) { m_Status = in_Status; }

const NEATValue::Status &NEATValue::GetStatus() const { return m_Status; }

bool NEATValue::IsUnknown() const { return (m_Status == UNKNOWN); }

/// Checks whether variable can have any value.
/// both "unwritten" status and "any" status applicable
bool NEATValue::IsAny() const { return (m_Status == NEATValue::ANY); }

bool NEATValue::IsAcc() const { return (m_Status == ACCURATE); }

bool NEATValue::IsUnwritten() const { return (m_Status == UNWRITTEN); }

std::istream &operator>>(std::istream &is, NEATValue &k) {
  std::string StatusStr;
  is >> StatusStr;
  if ("ACCURATE" == StatusStr) {
    k.m_Status = NEATValue::ACCURATE;
    for (uint32_t i = 0; i < NEATValue::MaxBytes; ++i) {
      int32_t _t;
      is >> _t;
      k.m_min[i] = static_cast<uint8_t>(_t);
    }
  } else if ("UNKNOWN" == StatusStr) {
    k.m_Status = NEATValue::UNKNOWN;
  } else if ("UNWRITTEN" == StatusStr) {
    k.m_Status = NEATValue::UNWRITTEN;
  } else if ("ANY" == StatusStr) {
    k.m_Status = NEATValue::ANY;
  } else if ("INTERVAL" == StatusStr) {
    k.m_Status = NEATValue::INTERVAL;
    for (uint32_t i = 0; i < NEATValue::MaxBytes; ++i) {
      int32_t _t;
      is >> _t;
      k.m_min[i] = static_cast<uint8_t>(_t);
    }
    for (uint32_t i = 0; i < NEATValue::MaxBytes; ++i) {
      int32_t _t;
      is >> _t;
      k.m_max[i] = static_cast<uint8_t>(_t);
    }
  } else {
    throw Exception::IOError("Invalid input NEAT state variable");
  }
  return is;
}

std::ostream &operator<<(std::ostream &os, const NEATValue &k) {
  switch (k.m_Status) {
  case NEATValue::ACCURATE:
    os << "ACCURATE"
       << " ";
    for (uint32_t i = 0; i < NEATValue::MaxBytes; ++i)
      os << (int32_t)k.m_min[i] << " ";
    break;
  case NEATValue::UNKNOWN:
    os << "UNKNOWN";
    break;
  case NEATValue::UNWRITTEN:
    os << "UNWRITTEN";
    break;
  case NEATValue::ANY:
    os << "ANY";
    break;
  case NEATValue::INTERVAL:
    os << "INTERVAL"
       << " ";
    for (uint32_t i = 0; i < NEATValue::MaxBytes; ++i)
      os << (int32_t)k.m_min[i] << " ";
    for (uint32_t i = 0; i < NEATValue::MaxBytes; ++i)
      os << (int32_t)k.m_max[i] << " ";
    break;
  }
  return os;
}
} // namespace Validation
