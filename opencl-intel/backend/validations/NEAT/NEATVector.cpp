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

#include "NEATVector.h"

namespace Validation {

NEATVector::NEATVector(VectorWidth width) : m_Width(width) {}

NEATVector::~NEATVector() {}

size_t NEATVector::GetSize() const { return m_Width.GetSize(); }

VectorWidth NEATVector::GetWidth() const { return m_Width.GetValue(); }

void NEATVector::SetWidth(VectorWidth in_width) { m_Width.SetValue(in_width); }

NEATValue &NEATVector::operator[](int i) {
  if (m_Width.GetValue() == INVALID_WIDTH)
    throw Exception::IllegalFunctionCall(
        "Trying to get value from NEATVector with unspecified type");
  if (m_Width.GetSize() <= (size_t)i)
    throw Exception::OutOfRange("Index is out of range");
  return m_Values[i];
}

const NEATValue &NEATVector::operator[](int i) const {
  if (m_Width.GetValue() == INVALID_WIDTH)
    throw Exception::IllegalFunctionCall(
        "Trying to get value from NEATVector with unspecified type");
  if (m_Width.GetSize() <= (size_t)i)
    throw Exception::OutOfRange("Index is out of range");
  return m_Values[i];
}
} // namespace Validation
