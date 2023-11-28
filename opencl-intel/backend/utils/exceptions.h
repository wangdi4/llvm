// INTEL CONFIDENTIAL
//
// Copyright 2010 Intel Corporation.
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

#ifndef __EXCEPTIONS_H__
#define __EXCEPTIONS_H__

#include "cl_device_api.h"
#include <stdexcept>
#include <string>

#ifndef LLVM_BACKEND_UNUSED
#if defined(_WIN32)
#define LLVM_BACKEND_UNUSED
#else
#define LLVM_BACKEND_UNUSED __attribute__((unused))
#endif
#endif
#ifdef _MSC_VER
#pragma warning(disable : 4985) /* disable ceil warnings */
#endif

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {
namespace Exceptions {

/// base class for validation exceptions
class DeviceBackendExceptionBase : public std::runtime_error {
public:
  DeviceBackendExceptionBase(const std::string &str,
                             cl_dev_err_code errCode = CL_DEV_ERROR_FAIL)
      : std::runtime_error(str), m_errCode(errCode) {}

  virtual ~DeviceBackendExceptionBase() throw() {}

  cl_dev_err_code GetErrorCode() const { return m_errCode; }

private:
  cl_dev_err_code m_errCode;
};

/// macro for convenient definition of device backend exceptions derived from
/// the base class DeviceBackendExceptionBase
#define DEFINE_EXCEPTION(__name)                                               \
  namespace Exceptions {                                                       \
  class __name : public Exceptions::DeviceBackendExceptionBase {               \
  public:                                                                      \
    __name(const std::string &str,                                             \
           cl_dev_err_code errCode = CL_DEV_ERROR_FAIL)                        \
        : DeviceBackendExceptionBase(std::string(#__name) + ' ' + str,         \
                                     errCode) {}                               \
  };                                                                           \
  }

} // namespace Exceptions

DEFINE_EXCEPTION(CompilerException)
// exception to signal compiler to emit a "build error" diagnostic.
DEFINE_EXCEPTION(UserErrorCompilerException)

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel

#endif // __EXCEPTIONS_H__
