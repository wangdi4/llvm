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

#ifndef __COMMON_OCL_BUILDER_H__
#define __COMMON_OCL_BUILDER_H__

#include <frontend_api.h>
#include "cl_dynamic_lib.h"

namespace Intel { namespace OpenCL { namespace Utils {

//
//Name: OCLBinaryFactory
//Description: Simplifies the OCL source building process
//
class CommonOCLBuilder{
public:
  //
  //returns a singleton instance of this.
  static CommonOCLBuilder& instance();

  //Sets the name of the library, from which the compiler will be loaded
  CommonOCLBuilder& withLibrary(const char* lib);

  //Sets the build options
  CommonOCLBuilder& withBuildOptions(const char* options);

  //sets the OCL source to be compiled
  CommonOCLBuilder& withSource(const char* src);

  CommonOCLBuilder& withExtensions(const char* extentions);

  CommonOCLBuilder& withFP64Support(bool );
  CommonOCLBuilder& withImageSupport(bool );
  CommonOCLBuilder& withFpgaEmulator(bool );
  //cleanup function
  void close();

  //Compiles the (previously given) source file with the compiler loaded from
  //(the previously given) library.
  //Returns value: IOCLFEBinaryResult, which contains the binary result in its
  //bytecode form, with some metadeta on it (size in bytes etc.)
  Intel::OpenCL::ClangFE::IOCLFEBinaryResult* build();

private:
  CommonOCLBuilder();
  // Prevent misuse, can't be stack allocated
  CommonOCLBuilder(const CommonOCLBuilder& builder);

  Intel::OpenCL::FECompilerAPI::IOCLFECompiler* createCompiler(const char* lib);

  //Statically initialized instance of the builder
  static CommonOCLBuilder _instance;
  //compiler pointer, extracted by 'withLibrary' method
  Intel::OpenCL::FECompilerAPI::IOCLFECompiler* m_pCompiler;
  //used to load the library
  Intel::OpenCL::Utils::OclDynamicLib m_dynamicLoader;
  //source to be compiled
  std::string m_source;
  //build options
  std::string m_options;
  //extensions
  std::string m_extensions;
  //Indicates whether doubles are supported by the device
  bool m_bSupportFP64;
  //Indicates whether images are supported by the device
  bool m_bSupportImages;
  // Indicates whether FPGA emulation is supported by the device
  bool m_bFpgaEmulator;
};

}}} // Namespaces

#endif //__OCL_BUILDER_H__
