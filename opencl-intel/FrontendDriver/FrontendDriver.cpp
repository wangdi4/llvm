// INTEL CONFIDENTIAL
//
// Copyright 2009 Intel Corporation.
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

#include "FrontendDriver.h"
#include "Compile.h"
#include "GetKernelArgInfo.h"
#include "Link.h"
#include "Logger.h"
#include "ParseSPIRV.h"
#include "SPIRMaterializer.h"
#include "cl_device_api.h"
#include "cl_dynamic_lib.h"
#include "cl_logger.h"
#include "cl_sys_defines.h"
#include "cl_sys_info.h"
#include "opencl_clang.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/Mutex.h"
#include "llvm/Support/Path.h"

#include <ctime>
#include <memory>
#include <sstream>
#include <string.h>

using namespace Intel::OpenCL::ClangFE;
using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::FECompilerAPI;

#ifdef _WIN32
static llvm::ManagedStatic<OclDynamicLib> m_dlClangLib;

#pragma comment(lib, "cl_sys_utils.lib")
#endif

static DECLARE_LOGGER_CLIENT;

static bool LoadCommonClang() {
#ifdef _WIN32
  // CCLANG_LIB_NAME is defined in CMakeLists.txt
  std::string modulePath = std::string(MAX_PATH, '\0');

  Intel::OpenCL::Utils::GetModuleDirectory(&modulePath[0], MAX_PATH);
  modulePath.resize(modulePath.find_first_of('\0'));

#ifdef INTEL_CUSTOMIZATION
#ifndef INTEL_PRODUCT_RELEASE
  llvm::SmallString<128> TempPath(modulePath);
  llvm::sys::path::append(TempPath, CCLANG_LIB_NAME);
  if (!llvm::sys::fs::exists(TempPath))
    modulePath = DEFAULT_OCL_LIBRARY_DIR;
#endif // !INTEL_PRODUCT_RELEASE
#endif // INTEL_CUSTOMIZATION

  llvm::SmallString<128> clangPath(modulePath);
  llvm::sys::path::append(clangPath, CCLANG_LIB_NAME);

  if (m_dlClangLib->Load(clangPath.c_str()) != 0) {
    LogErrorA("Failed to load common clang from %s", clangPath.c_str());
    if (FrameworkUserLogger::GetInstance()->IsErrorLoggingEnabled())
      FrameworkUserLogger::GetInstance()->PrintError(
          "Failed to load " + std::string(CCLANG_LIB_NAME) +
          " with error info: " + m_dlClangLib->GetError());

    return false;
  }
#endif // _WIN32
  return true;
}

static bool ClangCompilerInitialize() {
  // 'volatile' prevents caching in a CPU register.
  static volatile bool LoadSuccessful = true;
  static llvm::once_flag OnceFlag;
  llvm::call_once(OnceFlag, [&]() { LoadSuccessful = LoadCommonClang(); });
  return LoadSuccessful;
}

// ClangFECompiler class implementation
ClangFECompiler::ClangFECompiler(const void *pszDeviceInfo) {
  const CLANG_DEV_INFO *pDevInfo = (const CLANG_DEV_INFO *)pszDeviceInfo;

  m_sDeviceInfo.sExtensionStrings = STRDUP(pDevInfo->sExtensionStrings);
  m_sDeviceInfo.sOpenCLCFeatureStrings =
      STRDUP(pDevInfo->sOpenCLCFeatureStrings);
  m_sDeviceInfo.bImageSupport = pDevInfo->bImageSupport;
  m_sDeviceInfo.bHalfSupport = pDevInfo->bHalfSupport;
  m_sDeviceInfo.bDoubleSupport = pDevInfo->bDoubleSupport;
  m_sDeviceInfo.bEnableSourceLevelProfiling =
      pDevInfo->bEnableSourceLevelProfiling;
  m_sDeviceInfo.bIsFPGAEmu = pDevInfo->bIsFPGAEmu;
  m_config.Initialize(GetConfigFilePath());
}

ClangFECompiler::~ClangFECompiler() {
  if (m_sDeviceInfo.sExtensionStrings)
    free((void *)m_sDeviceInfo.sExtensionStrings);
  if (m_sDeviceInfo.sOpenCLCFeatureStrings)
    free((void *)m_sDeviceInfo.sOpenCLCFeatureStrings);
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
                                         pProgDesc->pszOptions,
                                         0,
                                         nullptr,
                                         nullptr};

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

std::unique_ptr<IOCLFESpecConstInfo>
ClangFECompiler::GetSpecConstInfo(FESPIRVProgramDescriptor *pProgDesc) {
  return ClangFECompilerParseSPIRVTask(pProgDesc, m_sDeviceInfo)
      .getSpecConstInfo();
}

int CreateFrontEndInstance(const void *pDeviceInfo, size_t devInfoSize,
                           IOCLFECompiler **pFECompiler) {
  INIT_LOGGER_CLIENT("FrontendDriver", LL_DEBUG);

  // Lazy initialization
  if (!ClangCompilerInitialize()) {
    return CL_COMPILER_NOT_AVAILABLE;
  }

  assert(nullptr != pFECompiler && "Front-end compiler can't be null");
  assert(devInfoSize == sizeof(CLANG_DEV_INFO) && "Ivalid device information");

  try {
    *pFECompiler = new ClangFECompiler(pDeviceInfo);
    RELEASE_LOGGER_CLIENT;
    return CL_SUCCESS;
  } catch (std::bad_alloc &) {
    LogErrorA(TEXT("Can't allocate compiler instance"));
    RELEASE_LOGGER_CLIENT;
    return CL_OUT_OF_HOST_MEMORY;
  }
}
