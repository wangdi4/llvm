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

#ifndef NEAT_VECTOR_H
#define NEAT_VECTOR_H

#include "NEATValue.h"
#include "VectorWidth.h"
#define MAX_VECTOR_WIDTH 16

namespace Validation {
struct NEATVector {
public:
  // Default ctor. To enable inserting into structures with default ctor.
  NEATVector() {}

  NEATVector(VectorWidth width);

  ~NEATVector();
  void SetWidth(VectorWidth width);
  VectorWidth GetWidth() const;
  size_t GetSize() const;
  NEATValue &operator[](int i);
  const NEATValue &operator[](int i) const;

private:
  VectorWidthWrapper m_Width;
  NEATValue m_Values[MAX_VECTOR_WIDTH];
};
} // namespace Validation

#endif // NEAT_VECTOR_H
