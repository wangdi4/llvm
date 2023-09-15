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

#ifndef SATest_EXCEPTION_H
#define SATest_EXCEPTION_H

#include "Exception.h"

namespace Validation {
namespace Exception {
/// Exception for reporting test comparison failure
DEFINE_VALIDATION_EXCEPTION(TestFailException)

/// Exception for reporting test run process failure
DEFINE_VALIDATION_EXCEPTION(TestRunnerException)

/// Exception for reporting reference run process failure
DEFINE_VALIDATION_EXCEPTION(TestReferenceRunnerException)

/// Exception for reporting general failures
DEFINE_VALIDATION_EXCEPTION(GeneralException)

/// Exception for reporting COI library failure
DEFINE_VALIDATION_EXCEPTION(COIUsageException)

/// Exception for reporting back-end failure
DEFINE_VALIDATION_EXCEPTION(BackendException)

/// Exception for invalid execution environment
DEFINE_VALIDATION_EXCEPTION(InvalidEnvironmentException)
} // namespace Exception
} // namespace Validation

#endif // SATest_EXCEPTION_H
