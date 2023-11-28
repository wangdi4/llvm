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

#include "CommonOCLBuilder.h"
#include "clang_device_info.h"
#include "ocl_string_exception.h"
#include "opencl_clang.h"
#include <string>

using namespace Intel::OpenCL::FECompilerAPI;
using namespace Intel::OpenCL::ClangFE;
using namespace Intel::OpenCL::Utils;
using namespace std;

CommonOCLBuilder &CommonOCLBuilder::instance() { return _instance; }

CommonOCLBuilder &CommonOCLBuilder::withBuildOptions(const char *options) {
  m_options = options;
  return *this;
}

CommonOCLBuilder &CommonOCLBuilder::withSource(const char *src) {
  m_source = src;
  return *this;
}

CommonOCLBuilder &
CommonOCLBuilder::withExtensions(const std::string &extensions) {
  m_extensions = extensions;
  return *this;
}

CommonOCLBuilder &
CommonOCLBuilder::withOpenCLCFeatures(const std::string &features) {
  m_OpenCLCFeatures = features;
  return *this;
}

CommonOCLBuilder &CommonOCLBuilder::withFP16Support(bool isFP16Supported) {
  m_bSupportFP16 = isFP16Supported;
  return *this;
}

CommonOCLBuilder &CommonOCLBuilder::withFP64Support(bool isFP64Supported) {
  m_bSupportFP64 = isFP64Supported;
  return *this;
}

CommonOCLBuilder &CommonOCLBuilder::withImageSupport(bool areImagesSupported) {
  m_bSupportImages = areImagesSupported;
  return *this;
}

CommonOCLBuilder &CommonOCLBuilder::withFpgaEmulator(bool isFpgaEmulation) {
  m_bFpgaEmulator = isFpgaEmulation;
  return *this;
}

void CommonOCLBuilder::close() {
  if (m_pCompiler)
    m_pCompiler->Release();
  m_pCompiler = nullptr;
}

IOCLFEBinaryResult *CommonOCLBuilder::build() {
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
  IOCLFEBinaryResult *res;
  int rc = m_pCompiler->CompileProgram(&programDescriptor, &res);
  if (rc || nullptr == res) {
    std::string errorMessage = "compilation failed: ";
    if (res)
      errorMessage.append(res->GetErrorLog());
    else
      errorMessage.append("<unknown error>");
    throw ocl_string_exception(errorMessage);
  }
  //
  // Linking
  //
  IOCLFEBinaryResult *executableResult = nullptr;
  FELinkProgramsDescriptor linkDescriptor;
  IOCLFELinkKernelNames *linkKernelNames = nullptr;
  linkDescriptor.uiNumBinaries = 1;
  linkDescriptor.pszOptions = "";
  const void *irBuffer = res->GetIR();
  linkDescriptor.pBinaryContainers = &irBuffer;
  size_t execsize = res->GetIRSize();
  linkDescriptor.puiBinariesSizes = &execsize;
  linkDescriptor.pKernelNames = &linkKernelNames;
  rc = m_pCompiler->LinkPrograms(&linkDescriptor, &executableResult);
  if (rc || nullptr == executableResult)
    throw ocl_string_exception("linkage failed");
  return executableResult;
}

CommonOCLBuilder::CommonOCLBuilder()
    : m_pCompiler(nullptr), m_bSupportFP16(true), m_bSupportFP64(true),
      m_bSupportImages(true), m_bFpgaEmulator(false) {}

CommonOCLBuilder &CommonOCLBuilder::createCompiler() {
  Intel::OpenCL::ClangFE::CLANG_DEV_INFO sDeviceInfo = {
      m_extensions.c_str(), m_OpenCLCFeatures.c_str(),
      m_bSupportImages,     m_bSupportFP16,
      m_bSupportFP64,       0,
      m_bFpgaEmulator};

  int rc =
      CreateFrontEndInstance(&sDeviceInfo, sizeof(sDeviceInfo), &m_pCompiler);
  if (rc || nullptr == m_pCompiler)
    throw ocl_string_exception("create frontend instance failed");
  return *this;
}

CommonOCLBuilder CommonOCLBuilder::_instance;
