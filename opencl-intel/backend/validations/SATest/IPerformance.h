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

#ifndef I_PERFORMANCE_H
#define I_PERFORMANCE_H

#include "CL/cl_platform.h"
#include <string>

namespace Validation {
// @brief This is a base interface for the performance class visitor
class IPerformanceVisitor {
public:
  virtual ~IPerformanceVisitor() {}

  virtual void
  OnKernelSample(const std::string &kernel, unsigned int vectorSize,
                 cl_long buildTicks, double buildSDMean, cl_long executionTicks,
                 double executionSDMean, cl_long serializationTicks,
                 double serializationSDMean, cl_long deserializationTicks,
                 double deserializationSDMean) = 0;
};

/// @brief This class enables test execution performance measurements.
/// The measurements consist of build time and execution time.
class IPerformance {
public:
  virtual ~IPerformance(void) {}

  /// @brief Visits the performance data
  virtual void Visit(IPerformanceVisitor *pVisitor) const = 0;
};
} // namespace Validation

#endif // I_PERFORMANCE_H
