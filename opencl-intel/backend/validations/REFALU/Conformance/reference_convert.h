// INTEL CONFIDENTIAL
//
// Copyright 2012 Intel Corporation.
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

#include "llvm/Support/DataTypes.h"

namespace Conformance {
uint16_t float2half_rte(float f);
uint16_t float2half_rtz(float f);
uint16_t float2half_rtp(float f);
uint16_t float2half_rtn(float f);
uint16_t double2half_rte(double f);
uint16_t double2half_rtz(double f);
uint16_t double2half_rtp(double f);
uint16_t double2half_rtn(double f);
} // namespace Conformance
