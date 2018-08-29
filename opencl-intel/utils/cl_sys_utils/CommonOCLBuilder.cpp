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


#include "CommonOCLBuilder.h"
#include "common_clang.h"
#include <string>
#include <clang_device_info.h>
#include "ocl_string_exception.h"

using namespace Intel::OpenCL::FECompilerAPI;
using namespace Intel::OpenCL::ClangFE;
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

CommonOCLBuilder& CommonOCLBuilder::withFpgaEmulator(bool isFpgaEmulation) {
  m_bFpgaEmulator = isFpgaEmulation;
  return *this;
}

void CommonOCLBuilder::close(){
  m_dynamicLoader.Close();
  if (m_pCompiler)
    m_pCompiler->Release();
  m_pCompiler = nullptr;
}

IOCLFEBinaryResult* CommonOCLBuilder::build(){
  if (nullptr == m_pCompiler)
    throw ocl_string_exception("loader wasn't assigned");
  if (m_source.empty())
    throw ocl_string_exception("OCL source wasn't assigned");
  FECompileProgramDescriptor programDescriptor;
  programDescriptor.pProgramSource = m_source.c_str();
  programDescriptor.uiNumInputHeaders = 0;
  programDescriptor.pInputHeaders = nullptr;
  programDescriptor.pszInputHeadersNames = nullptr;
  programDescriptor.pszOptions = m_options.c_str();
  programDescriptor.bFpgaEmulator = m_bFpgaEmulator;
  IOCLFEBinaryResult* res;
  int rc = m_pCompiler->CompileProgram(&programDescriptor, &res);
  if (rc || nullptr == res){
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
  IOCLFEBinaryResult* executableResult = nullptr;
  FELinkProgramsDescriptor linkDescriptor;
  linkDescriptor.uiNumBinaries = 1;
  linkDescriptor.pszOptions = "";
  const void* irBuffer = res->GetIR();
  linkDescriptor.pBinaryContainers = &irBuffer;
  size_t execsize = res->GetIRSize();
  linkDescriptor.puiBinariesSizes = &execsize;
  rc = m_pCompiler->LinkPrograms(&linkDescriptor, &executableResult);
  if (rc || nullptr == executableResult)
    throw ocl_string_exception("linkage failed");
  return executableResult;
}

CommonOCLBuilder::CommonOCLBuilder():
  m_pCompiler(nullptr),
  m_bSupportFP64(true),
  m_bSupportImages(true),
  m_bFpgaEmulator(false){
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
  assert(factoryMethod && "GetFunctionPtrByName failed");
  int rc = factoryMethod(&sDeviceInfo, sizeof(sDeviceInfo), &ret, nullptr);
  if (rc || nullptr == ret)
    throw ocl_string_exception("factory method failed");
  return ret;
}

CommonOCLBuilder CommonOCLBuilder::_instance;
