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

#include "ImageCallbackManager.h"
#include "CPUCompiler.h"
#include "Compiler.h"
#include "ImageCallbackLibrary.h"
#include "exceptions.h"
#include <assert.h>
#include <memory>
#include <string>

// void RegisterBIFunctions(void);

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

ImageCallbackManager *ImageCallbackManager::s_pInstance = nullptr;

ImageCallbackManager::ImageCallbackManager() {}

ImageCallbackManager::~ImageCallbackManager() {
  for (ImageCallbackMap::iterator i = m_ImageCallbackLibs.begin(),
                                  e = m_ImageCallbackLibs.end();
       i != e; ++i) {
    delete i->second;
  }
}

void ImageCallbackManager::Init() {
  assert(!s_pInstance);
  s_pInstance = new ImageCallbackManager();
}

void ImageCallbackManager::Terminate() {
  if (nullptr != s_pInstance) {
    delete s_pInstance;
    s_pInstance = nullptr;
  }
}

ImageCallbackManager *ImageCallbackManager::GetInstance() {
  assert(s_pInstance);
  return s_pInstance;
}

bool ImageCallbackManager::InitLibrary(const ICompilerConfig &config,
                                       bool isCpu,
                                       Intel::OpenCL::Utils::CPUDetect *cpuId) {
  if (!isCpu) {
    // Nothing but CPU is supported
    return true;
  }

  // Initialize compiler first to get archId and CPUFeatures
  std::unique_ptr<CPUCompiler> spCompiler(new CPUCompiler(config));

  // Retrieve CPU ID
  cpuId = spCompiler->GetCpuId();

  // Find library for this platform if it has been built earlier
  ImageCallbackMap::iterator it = m_ImageCallbackLibs.find(cpuId->GetCPU());
  if (it != m_ImageCallbackLibs.end())
    return true;

  // ImageCallbackLibrary becomes the owner of compiler. So release compiler
  // here
  std::unique_ptr<ImageCallbackLibrary> spLibrary(
      new ImageCallbackLibrary(cpuId, spCompiler.release()));
  spLibrary->Build();

  if (!spLibrary->LoadExecutable()) {
    return false; // failed to load library to the device
  }
  m_ImageCallbackLibs[cpuId->GetCPU()] = spLibrary.release();
  return true;
}

ImageCallbackFunctions *ImageCallbackManager::getCallbackFunctions(
    const Intel::OpenCL::Utils::CPUDetect *cpuId) {
  ImageCallbackMap::iterator it = m_ImageCallbackLibs.find(cpuId->GetCPU());
  if (it == m_ImageCallbackLibs.end()) {
    throw Exceptions::CompilerException(
        "Requested image function for library that hasn't been loaded");
  }

  return m_ImageCallbackLibs[cpuId->GetCPU()]->getImageCallbackFunctions();
}
} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
