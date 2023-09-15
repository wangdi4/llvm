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

#ifndef __RefALUVER_H__
#define __RefALUVER_H__

#include "llvm/Support/DataTypes.h"

namespace Validation {
class RefALUVersion {
public:
  static uint64_t GetVersion() { return refALUVersion; }

private:
  /// NEAT version to be incremented after every change in NEAT
  static const uint64_t refALUVersion = 1;
};
} // namespace Validation
#endif // __RefALUVER_H__
