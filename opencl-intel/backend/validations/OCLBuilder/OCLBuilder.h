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
  Intel::OpenCL::FECompilerAPI::IOCLFEBinaryResult* build();

private:
  OCLBuilder();
  OCLBuilder(OCLBuilder const&);
  OCLBuilder& operator=(OCLBuilder const&);
  /*
  Intel::OpenCL::FECompilerAPI::IOCLFECompiler* createCompiler(const char* lib);
  */

  Intel::OpenCL::Utils::CommonOCLBuilder& m_CommonBuilder;
};

}

#endif //__OCL_BUILDER_H__
