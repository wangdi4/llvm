/*****************************************************************************\

Copyright (c) Intel Corporation (2011,2012).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name: OCLBuilder.h

\*****************************************************************************/
#ifndef __OCL_BUILDER_H__
#define __OCL_BUILDER_H__

#include "frontend_api.h"
#include "DynamicLib.h"
#include "Exception.h"

namespace Validation{

//
//Name: OCLBinaryFactory
//Description: Simplifies the OCL source building process
//
class OCLBuilder{
public:
  //
  //returns a singleton instance of this.
  static OCLBuilder& instance();

  //Sets the name of the library, from which the compiler will be loaded
  OCLBuilder& withLibrary(const char* lib);

  //Sets the build options
  OCLBuilder& withBuildOptions(const char* options);

  //sets the OCL source to be compiled
  OCLBuilder& withSource(const char* src);

  OCLBuilder& withFP64Support(bool );
  OCLBuilder& withImageSupport(bool );
  //cleanup function
  void close();

  //Compiles the (previously given) source file with the compiler loaded from
  //(the previously given) library.
  //Returns value: IOCLFEBinaryResult, which contains the binary result in its
  //bytecode form, with some metadeta on it (size in bytes etc.)
  Intel::OpenCL::FECompilerAPI::IOCLFEBinaryResult* build();

private:
  OCLBuilder();

  Intel::OpenCL::FECompilerAPI::IOCLFECompiler* createCompiler(const char* lib);

  //Statically initialized instance of the builder
  static OCLBuilder _instance;
  //compiler pointer, extracted by 'withLibrary' method
  Intel::OpenCL::FECompilerAPI::IOCLFECompiler* m_pCompiler;
  //used to load the library
  Intel::OpenCL::DeviceBackend::Utils::DynamicLib m_dynamicLoader;
  //source to be compiled
  std::string m_source;
  //build options
  std::string m_options;
  //Indicates whether doubles are supported by the device
  bool m_bSupportFP64;
  //Indicates whether images are supported by the device
  bool m_bSupportImages;
};

}

#endif //__OCL_BUILDER_H__
