//==---- clang_copmiler.cpp - OpenCL front-end compiler -------*- C++ -*---=
//
// Copyright (C) 2009-2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------===

#include "stdafx.h"
#include "clang_compiler.h"
#include "clang_driver.h"
#include "common_clang.h"

#include <Logger.h>
#include <cl_sys_defines.h>
#include <cl_device_api.h>
#include <cl_shutdown.h>

#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/Mutex.h"

#if defined (_WIN32)
#include<windows.h>
#endif
#include <string.h>
#include <memory>
#include <sstream>
#include <ctime>

using namespace Intel::OpenCL::ClangFE;
using namespace Intel::OpenCL::Utils;

#if defined (_WIN32)
#define DLL_EXPORT _declspec(dllexport)
#else
#define DLL_EXPORT
#endif

using namespace Intel::OpenCL::FECompilerAPI;

extern DECLARE_LOGGER_CLIENT;

void ClangCompilerTerminate() { llvm::llvm_shutdown(); }

static volatile bool lazyClangCompilerInit =
    true; // the flag must be 'volatile' to prevent caching in a CPU register
static llvm::sys::Mutex lazyClangCompilerInitMutex;

void ClangCompilerInitialize() {
  if (lazyClangCompilerInit) {
    llvm::sys::ScopedLock lock(lazyClangCompilerInitMutex);

    if (lazyClangCompilerInit) {
      atexit(ClangCompilerTerminate);
      lazyClangCompilerInit = false;
    }
  }
}

// ClangFECompiler class implementation
ClangFECompiler::ClangFECompiler(const void *pszDeviceInfo) {
  CLANG_DEV_INFO *pDevInfo = (CLANG_DEV_INFO *)pszDeviceInfo;

  memset(&m_sDeviceInfo, 0, sizeof(CLANG_DEV_INFO));

  m_sDeviceInfo.sExtensionStrings = STRDUP(pDevInfo->sExtensionStrings);
  m_sDeviceInfo.bImageSupport = pDevInfo->bImageSupport;
  m_sDeviceInfo.bDoubleSupport = pDevInfo->bDoubleSupport;
  m_sDeviceInfo.bEnableSourceLevelProfiling =
      pDevInfo->bEnableSourceLevelProfiling;
  m_config.Initialize(GetConfigFilePath());
}

ClangFECompiler::~ClangFECompiler() {
  if (NULL != m_sDeviceInfo.sExtensionStrings) {
    free((void *)m_sDeviceInfo.sExtensionStrings);
  }
}

int ClangFECompiler::CompileProgram(FECompileProgramDescriptor *pProgDesc,
                                    IOCLFEBinaryResult **pBinaryResult) {
  assert(NULL != pProgDesc);
  assert(NULL != pBinaryResult);

  return ClangFECompilerCompileTask(pProgDesc, m_sDeviceInfo, m_config)
      .Compile(pBinaryResult);
}

int ClangFECompiler::LinkPrograms(
    Intel::OpenCL::FECompilerAPI::FELinkProgramsDescriptor *pProgDesc,
    IOCLFEBinaryResult **pBinaryResult) {
  assert(NULL != pProgDesc);
  assert(NULL != pBinaryResult);

  return ClangFECompilerLinkTask(pProgDesc).Link(pBinaryResult);
}

int ClangFECompiler::ParseSPIRV(FESPIRVProgramDescriptor *pProgDesc,
                                IOCLFEBinaryResult **pBinaryResult) {
  assert(NULL != pProgDesc);
  assert(NULL != pBinaryResult);

  return ClangFECompilerParseSPIRVTask(pProgDesc, m_sDeviceInfo)
      .ParseSPIRV(pBinaryResult);
}

int ClangFECompiler::GetKernelArgInfo(const void *pBin, size_t uiBinarySize,
                                      const char *szKernelName,
                                      IOCLFEKernelArgInfo **pArgInfo) {
  assert(NULL != pBin);
  assert(NULL != szKernelName);
  assert(NULL != pArgInfo);

  return ClangFECompilerGetKernelArgInfoTask().GetKernelArgInfo(
      pBin, uiBinarySize, szKernelName, pArgInfo);
}

bool ClangFECompiler::CheckCompileOptions(const char *szOptions,
                                          char *szUnrecognizedOptions,
                                          size_t uiUnrecognizedOptionsSize) {
  return ClangFECompilerCheckCompileOptions(
      szOptions, szUnrecognizedOptions, uiUnrecognizedOptionsSize, m_config);
}

bool ClangFECompiler::CheckLinkOptions(const char *szOptions,
                                       char *szUnrecognizedOptions,
                                       size_t uiUnrecognizedOptionsSize) {
  return ClangFECompilerCheckLinkOptions(szOptions, szUnrecognizedOptions,
                                         uiUnrecognizedOptionsSize);
}

namespace Intel {
namespace OpenCL {
namespace Utils {
FrameworkUserLogger *g_pUserLogger = NULL;
}}}

extern "C" DLL_EXPORT int
CreateFrontEndInstance(const void *pDeviceInfo, size_t devInfoSize,
                       IOCLFECompiler **pFECompiler,
                       Intel::OpenCL::Utils::FrameworkUserLogger *pUserLogger) {
  // Lazy initialization
  ClangCompilerInitialize();

  assert(NULL != pFECompiler);
  assert(devInfoSize == sizeof(CLANG_DEV_INFO));

  g_pUserLogger = pUserLogger;

  try {
    *pFECompiler = new ClangFECompiler(pDeviceInfo);
    return CL_SUCCESS;
  } catch (std::bad_alloc &) {
    LOG_ERROR(TEXT("%S"), TEXT("Can't allocate compiler instance"));
    return CL_OUT_OF_HOST_MEMORY;
  }
}

