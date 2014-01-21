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

File Name: CommonOCLBuilder.cpp

\*****************************************************************************/

#include "CommonOCLBuilder.h"
#include <string>
#include <clang_device_info.h>
#include "ocl_string_exception.h"

using namespace Intel::OpenCL::FECompilerAPI;
using namespace Intel::OpenCL::Utils;
using namespace std;

CommonOCLBuilder& CommonOCLBuilder::instance(){
  return _instance;
}

CommonOCLBuilder& CommonOCLBuilder::withLibrary(const char* lib){
  m_pCompiler = createCompiler(lib);
  return *this;
}

CommonOCLBuilder& CommonOCLBuilder::withBuildOptions(const char* options){
  m_options = options;
  return *this;
}

CommonOCLBuilder& CommonOCLBuilder::withSource(const char* src){
	m_source = src;
	return *this;
}

CommonOCLBuilder& CommonOCLBuilder::withExtensions(const char* extensions){
	m_extensions = extensions;
	return *this;
}

CommonOCLBuilder& CommonOCLBuilder::withFP64Support(bool isFP64Supported) {
  m_bSupportFP64 = isFP64Supported;
  return *this;
}

CommonOCLBuilder& CommonOCLBuilder::withImageSupport(bool areImagesSupported) {
  m_bSupportImages = areImagesSupported;
  return *this;
}

void CommonOCLBuilder::close(){
  m_dynamicLoader.Close();
  if (m_pCompiler)
    m_pCompiler->Release();
  m_pCompiler = NULL;
}

IOCLFEBinaryResult* CommonOCLBuilder::build(){
  if (NULL == m_pCompiler)
    throw ocl_string_exception("loader wasn't assigned");
  if (m_source.empty())
    throw ocl_string_exception("OCL source wasn't assigned");
  FECompileProgramDescriptor programDescriptor;
  programDescriptor.pProgramSource = m_source.c_str();
  programDescriptor.uiNumInputHeaders = 0;
  programDescriptor.pInputHeaders = NULL;
  programDescriptor.pszInputHeadersNames = NULL;
  programDescriptor.pszOptions = m_options.c_str();
  IOCLFEBinaryResult* res;
  int rc = m_pCompiler->CompileProgram(&programDescriptor, &res);
  if (rc || NULL == res){
    std::string errorMessage = "compilation failed: ";
    if (res)
      errorMessage.append (res->GetErrorLog());
    else
      errorMessage.append("<unknown error>");
    throw ocl_string_exception(errorMessage);
  }
  //
  //Linking
  //
  IOCLFEBinaryResult* executableResult = NULL;
  FELinkProgramsDescriptor linkDescriptor;
  linkDescriptor.uiNumBinaries = 1;
  linkDescriptor.pszOptions = "";
  const void* irBuffer = res->GetIR();
  linkDescriptor.pBinaryContainers = &irBuffer;
  size_t execsize = res->GetIRSize();
  linkDescriptor.puiBinariesSizes = &execsize;
  rc = m_pCompiler->LinkPrograms(&linkDescriptor, &executableResult);
  if (rc || NULL == executableResult)
    throw ocl_string_exception("linkage failed");
  return executableResult;
}

CommonOCLBuilder::CommonOCLBuilder(): m_pCompiler(NULL), m_bSupportFP64(true),
m_bSupportImages(true) {
}

IOCLFECompiler* CommonOCLBuilder::createCompiler(const char* lib){
  IOCLFECompiler* ret;
  //constants
  const char* fnFactoryName = "CreateFrontEndInstance";
  const char* strDeviceOptions = m_extensions.c_str();

  Intel::OpenCL::ClangFE::CLANG_DEV_INFO sDeviceInfo = {
    strDeviceOptions,
    m_bSupportImages,
    m_bSupportFP64,
    0
    };

  //
  //loading clang dll
  //
  m_dynamicLoader.Load(lib);
  fnCreateFECompilerInstance* factoryMethod =
    (fnCreateFECompilerInstance*)(intptr_t)m_dynamicLoader.GetFunctionPtrByName(fnFactoryName);
  int rc = factoryMethod(&sDeviceInfo, sizeof(sDeviceInfo), &ret);
  if (rc || NULL == ret)
    throw ocl_string_exception("factory method failed");
  return ret;
}

CommonOCLBuilder CommonOCLBuilder::_instance;
