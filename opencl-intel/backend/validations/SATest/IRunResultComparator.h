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

#ifndef I_RUN_RESULT_COMPARATOR_H
#define I_RUN_RESULT_COMPARATOR_H

namespace Validation {
class IRunResult;
class IRunResultComparison;

/// @brief Contains the result of comparison for entire run result
class IRunResultComparator {
public:
  virtual ~IRunResultComparator(void) {}

  /// @brief Compares the result of two runs
  virtual IRunResultComparison *Compare(IRunResult *lhs,
                                        IRunResult *rhs) const = 0;
};
} // namespace Validation

#endif // I_RUN_RESULT_H
