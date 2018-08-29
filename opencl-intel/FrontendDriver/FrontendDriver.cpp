// INTEL CONFIDENTIAL
//
// Copyright 2009-2018 Intel Corporation.
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

#include "common_clang.h"
#include "Compile.h"
#include "FrontendDriver.h"
#include "GetKernelArgInfo.h"
#include "Link.h"
#include "ParseSPIRV.h"
#include "SPIRMaterializer.h"

#include <Logger.h>
#include <cl_device_api.h>
#include <cl_shutdown.h>
#include <cl_sys_defines.h>

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/Mutex.h"

#include <ctime>
#include <memory>
#include <sstream>
#include <string.h>

using namespace Intel::OpenCL::ClangFE;
using namespace Intel::OpenCL::Utils;

#if defined(_WIN32)
#define DLL_EXPORT _declspec(dllexport)
#else
#define DLL_EXPORT
#endif

using namespace Intel::OpenCL::FECompilerAPI;

DECLARE_LOGGER_CLIENT;

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
  if (!m_sDeviceInfo.sExtensionStrings)
    free((void *)m_sDeviceInfo.sExtensionStrings);
}

int ClangFECompiler::CompileProgram(FECompileProgramDescriptor *pProgDesc,
                                    IOCLFEBinaryResult **pBinaryResult) {
  assert(nullptr != pProgDesc && "Program description can't be null");
  assert(nullptr != pBinaryResult && "Result parameter can't be null");

  int result = ClangFECompilerCompileTask(pProgDesc, m_sDeviceInfo, m_config)
      .Compile(pBinaryResult);

  if (result != CL_SUCCESS ||
      !ClangFECompilerParseSPIRVTask::isSPIRV((*pBinaryResult)->GetIR(),
                                              (*pBinaryResult)->GetIRSize()))
    return result;

  // If we get here, ClangFECompilerCompileTask::Compile() generated a SPIR-V
  // module. Now we need to translate it to LLVM IR.

  // Keep pointer to SPIR-V binary to clear it *after* parsing
  auto *pSPIRVResult = *pBinaryResult;

  FESPIRVProgramDescriptor SPIRVProgDesc{pSPIRVResult->GetIR(),
                                         pSPIRVResult->GetIRSize(),
                                         pProgDesc->pszOptions};

  result = ClangFECompilerParseSPIRVTask(&SPIRVProgDesc, m_sDeviceInfo)
      .ParseSPIRV(pBinaryResult);

  // SPIR-V binary is not needed anymore, clear it.
  // NOTE: After pSPIRVResult removing SPIRVProgDesc will be invalid.
  pSPIRVResult->Release();

  return result;
}

int ClangFECompiler::LinkPrograms(
    Intel::OpenCL::FECompilerAPI::FELinkProgramsDescriptor *pProgDesc,
    IOCLFEBinaryResult **pBinaryResult) {
  assert(nullptr != pProgDesc && "Program description can't be null");
  assert(nullptr != pBinaryResult && "Result parameter can't be null");

  return ClangFECompilerLinkTask(pProgDesc).Link(pBinaryResult);
}

int ClangFECompiler::ParseSPIRV(FESPIRVProgramDescriptor *pProgDesc,
                                IOCLFEBinaryResult **pBinaryResult) {
  assert(nullptr != pProgDesc && "Program description can't be null");
  assert(nullptr != pBinaryResult && "Result parameter can't be null");

  return ClangFECompilerParseSPIRVTask(pProgDesc, m_sDeviceInfo)
      .ParseSPIRV(pBinaryResult);
}

int ClangFECompiler::MaterializeSPIR(FESPIRProgramDescriptor *pProgDesc,
                                     IOCLFEBinaryResult **pBinaryResult) {
  assert(nullptr != pProgDesc && "Program description can't be null");
  assert(nullptr != pBinaryResult && "Result parameter can't be null");

  return ClangFECompilerMaterializeSPIRTask(pProgDesc).MaterializeSPIR(
      pBinaryResult);
}

int ClangFECompiler::GetKernelArgInfo(const void *pBin, size_t uiBinarySize,
                                      const char *szKernelName,
                                      IOCLFEKernelArgInfo **pArgInfo) {
  assert(nullptr != pBin && "Binary can't be null");
  assert(nullptr != szKernelName && "Kernel name is required");
  assert(nullptr != pArgInfo && "Result parameter can't be null");

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
  return ClangLinkOptions(szOptions).checkOptions(szUnrecognizedOptions,
                                                  uiUnrecognizedOptionsSize);
}

namespace Intel {
namespace OpenCL {
namespace Utils {
FrameworkUserLogger *g_pUserLogger = nullptr;
}
}
}

extern "C" DLL_EXPORT int
CreateFrontEndInstance(const void *pDeviceInfo, size_t devInfoSize,
                       IOCLFECompiler **pFECompiler,
                       Intel::OpenCL::Utils::FrameworkUserLogger *pUserLogger) {
  // Lazy initialization
  ClangCompilerInitialize();

  assert(nullptr != pFECompiler && "Front-end compiler can't be null");
  assert(devInfoSize == sizeof(CLANG_DEV_INFO) && "Ivalid device information");

  g_pUserLogger = pUserLogger;

  try {
    *pFECompiler = new ClangFECompiler(pDeviceInfo);
    return CL_SUCCESS;
  } catch (std::bad_alloc &) {
    LogErrorA("%S", "Can't allocate compiler instance");
    return CL_OUT_OF_HOST_MEMORY;
  }
}
