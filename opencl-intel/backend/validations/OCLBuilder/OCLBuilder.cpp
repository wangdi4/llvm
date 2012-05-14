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

File Name: OCLBuilder.cpp

\*****************************************************************************/

#include "OCLBuilder.h"
#include <cstring>
#include "clang_device_info.h"

using namespace Intel::OpenCL::FECompilerAPI;

namespace Validation{

OCLBuilder& OCLBuilder::instance(){
  return _instance;
}

OCLBuilder& OCLBuilder::withLibrary(const char* lib){
  m_pCompiler = createCompiler(lib);
  return *this;
}

OCLBuilder& OCLBuilder::withBuildOptions(const char* options){
  m_options = options;
  return *this;
}

OCLBuilder& OCLBuilder::withSource(const char* src){
  m_source = src;
  return *this;
}

OCLBuilder& OCLBuilder::withFP64Support(bool isFP64Supported) {
  m_bSupportFP64 = isFP64Supported;
  return *this;
}

OCLBuilder& OCLBuilder::withImageSupport(bool areImagesSupported) {
  m_bSupportImages = areImagesSupported;
  return *this;
}

void OCLBuilder::close(){
  m_dynamicLoader.Close();
//causes a crash.. investigate!
  /*if (m_pCompiler)
    m_pCompiler->Release();*/
  m_pCompiler = NULL;
}

IOCLFEBinaryResult* OCLBuilder::build(){
  if (NULL == m_pCompiler)
    throw Validation::Exception::OperationFailed("loader wasn't assigned");
  if (m_source.empty())
    throw Validation::Exception::OperationFailed("OCL source wasn't assigned");
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
    throw Validation::Exception::OperationFailed(errorMessage);
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
    throw Validation::Exception::OperationFailed("linkage failed");
  return executableResult;
}

OCLBuilder::OCLBuilder(): m_pCompiler(NULL), m_bSupportFP64(true),
m_bSupportImages(true) {
}

IOCLFECompiler* OCLBuilder::createCompiler(const char* lib){
  IOCLFECompiler* ret;
  //constants
  const char* fnFactoryName = "CreateFrontEndInstance";
  const char* strDeviceOptions = "cl_khr_fp64 cl_khr_icd"
  "cl_khr_global_int32_base_atomics cl_khr_global_int32_extended_atomics"
  " cl_khr_local_int32_base_atomics cl_khr_local_int32_extended_atomics"
  " cl_khr_byte_addressable_store cl_intel_printf cl_ext_device_fission"
  " cl_intel_exec_by_local_thread";

  Intel::OpenCL::ClangFE::CLANG_DEV_INFO sDeviceInfo = {
    strDeviceOptions,
    m_bSupportImages,
    m_bSupportFP64
    };

  //
  //loading clang dll
  //
  m_dynamicLoader.Load(lib);
  fnCreateFECompilerInstance* factoryMethod =
    (fnCreateFECompilerInstance*)(intptr_t)m_dynamicLoader.GetFuncPtr(fnFactoryName);
  int rc = factoryMethod(&sDeviceInfo, sizeof(sDeviceInfo), &ret);
  if (rc || NULL == ret)
    throw Validation::Exception::OperationFailed("factory method failed");
  return ret;
}

OCLBuilder OCLBuilder::_instance;

}
