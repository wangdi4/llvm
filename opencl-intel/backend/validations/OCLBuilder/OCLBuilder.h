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

#ifndef __OCL_BUILDER_H__
#define __OCL_BUILDER_H__

#include "BE_DynamicLib.h"
#include "CommonOCLBuilder.h"
#include "Exception.h"
#include "frontend_api.h"

namespace Validation {

//
// Description: Simplifies the OCL source building process
//
class OCLBuilder {
public:
  OCLBuilder(OCLBuilder const &) = delete;
  OCLBuilder &operator=(OCLBuilder const &) = delete;

  // returns a singleton instance of this.
  static OCLBuilder &Instance();

  OCLBuilder &createCompiler();

  // Sets the build options
  OCLBuilder &withBuildOptions(const char *options);

  // sets the OCL source to be compiled
  OCLBuilder &withSource(const char *src);

  OCLBuilder &withExtensions(bool IsFPGA);
  OCLBuilder &withOpenCLCFeatures();

  OCLBuilder &withFP16Support(bool);
  OCLBuilder &withFP64Support(bool);
  OCLBuilder &withImageSupport(bool);
  OCLBuilder &withFpgaEmulator(bool);
  // cleanup function
  void close();

  // Compiles the (previously given) source file with the compiler loaded from
  //(the previously given) library.
  // Returns value: IOCLFEBinaryResult, which contains the binary result in its
  // bytecode form, with some metadeta on it (size in bytes etc.)
  Intel::OpenCL::ClangFE::IOCLFEBinaryResult *build();

private:
  OCLBuilder();

  Intel::OpenCL::Utils::CommonOCLBuilder &m_CommonBuilder;
};

} // namespace Validation

#endif //__OCL_BUILDER_H__
