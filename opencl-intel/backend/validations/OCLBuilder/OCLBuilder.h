// INTEL CONFIDENTIAL
//
// Copyright 2011-2018 Intel Corporation.
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

#include "frontend_api.h"
#include <BE_DynamicLib.h>
#include <Exception.h>
#include <CommonOCLBuilder.h>

namespace Validation{

//
//Description: Simplifies the OCL source building process
//
class OCLBuilder {
public:
  //
  //returns a singleton instance of this.
  static OCLBuilder& Instance();

  //Sets the name of the library, from which the compiler will be loaded
  OCLBuilder& withLibrary(const char* lib);

  //Sets the build options
  OCLBuilder& withBuildOptions(const char* options);

  //sets the OCL source to be compiled
  OCLBuilder& withSource(const char* src);

  OCLBuilder& withExtensions(const char* extentions);

  OCLBuilder& withFP64Support(bool );
  OCLBuilder& withImageSupport(bool );
  //cleanup function
  void close();

  //Compiles the (previously given) source file with the compiler loaded from
  //(the previously given) library.
  //Returns value: IOCLFEBinaryResult, which contains the binary result in its
  //bytecode form, with some metadeta on it (size in bytes etc.)
  Intel::OpenCL::ClangFE::IOCLFEBinaryResult* build();

private:
  OCLBuilder();
  OCLBuilder(OCLBuilder const&);
  OCLBuilder& operator=(OCLBuilder const&);

  Intel::OpenCL::Utils::CommonOCLBuilder& m_CommonBuilder;
};

}

#endif //__OCL_BUILDER_H__
