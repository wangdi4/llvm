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

#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

#include "ValidationErrorCode.h"
#include <stdexcept>
#include <string>

namespace Validation {
namespace Exception {

/// base class for validation exceptions
class ValidationExceptionBase : public std::runtime_error {
public:
  ValidationExceptionBase(const std::string &str,
                          VALIDATION_ERROR_CODE errCode = VALIDATION_ERROR_FAIL)
      : std::runtime_error(str), m_errCode(errCode) {}

  virtual ~ValidationExceptionBase() throw() {}

  VALIDATION_ERROR_CODE GetErrorCode() const { return m_errCode; }

private:
  VALIDATION_ERROR_CODE m_errCode;
};

/// macro for convenient definition of validation exceptions derived from
/// the base class ValidationExceptionBase
#define DEFINE_VALIDATION_EXCEPTION(__name)                                    \
  class __name : public ValidationExceptionBase {                              \
  public:                                                                      \
    __name(const std::string &str,                                             \
           VALIDATION_ERROR_CODE errCode = VALIDATION_ERROR_FAIL)              \
        : ValidationExceptionBase(std::string(#__name) + ' ' + str, errCode) { \
    }                                                                          \
  };

/// Exception for reporting file input/output errors
DEFINE_VALIDATION_EXCEPTION(IOError)

/// Exception for reporting function call invalid arguments
DEFINE_VALIDATION_EXCEPTION(InvalidArgument)

/// Exception for reporting unsupported by NEAT situations.
DEFINE_VALIDATION_EXCEPTION(NEATTrackFailure)

/// Exception for illegal function call exceptions
DEFINE_VALIDATION_EXCEPTION(IllegalFunctionCall)

/// Exception for out of range access
DEFINE_VALIDATION_EXCEPTION(OutOfRange)

/// Exception for Not implemented functions
DEFINE_VALIDATION_EXCEPTION(NotImplemented)

/// Exception for illegal command line parameters
DEFINE_VALIDATION_EXCEPTION(CmdLineException)

/// Exception for failed operation
DEFINE_VALIDATION_EXCEPTION(OperationFailed)

/// Exception for bad type in parser
DEFINE_VALIDATION_EXCEPTION(ParserBadTypeException)

/// Exception for bad type in generator
DEFINE_VALIDATION_EXCEPTION(GeneratorBadTypeException)

} // namespace Exception
} // namespace Validation

#endif // __EXCEPTION_H__
