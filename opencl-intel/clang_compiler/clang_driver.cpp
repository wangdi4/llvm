//==---- clang_driver.cpp --- OpenCL front-end compiler -------*- C++ -*---=
//
// Copyright (C) 2009-2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------===
#include "stdafx.h"

#include "cache_binary_handler.h"
#include "clang_driver.h"
#include "common_clang.h"
#include "elf_binary.h"
#include "mic_dev_limits.h"

#include <Logger.h>
#include <cl_autoptr_ex.h>
#include <cl_cpu_detect.h>
#include <cl_sys_info.h>

#include <spirv/1.0/spirv.hpp>

#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/SPIRV.h>
#include <llvm/Support/SwapByteOrder.h>
#include <llvm/Support/raw_ostream.h>

#include <algorithm>
#include <cctype>
#include <istream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

using namespace Intel::OpenCL::ELFUtils;
using namespace Intel::OpenCL::ClangFE;
using namespace Intel::OpenCL::Utils;
using namespace std;

typedef auto_ptr_ex<IOCLFEBinaryResult, ReleaseDP<IOCLFEBinaryResult>>
    IOCLFEBinaryResultPtr;

#if defined(_WIN32)
#define GET_CURR_WORKING_DIR(len, buff) GetCurrentDirectoryA(len, buff)
#define PASS_PCH
#else
#define GET_CURR_WORKING_DIR(len, buff) getcwd(buff, len)
#endif

#define MAX_STR_BUFF 1024

// Declare logger client
DECLARE_LOGGER_CLIENT;

#ifdef OCLFRONTEND_PLUGINS
#include "compile_data.h"
#include "link_data.h"
#include "plugin_manager.h"
#include "source_file.h"
Intel::OpenCL::PluginManager g_pluginManager;

//
//Creates a source file object from a given contents string, and a serial identifier.
//
static Intel::OpenCL::Frontend::SourceFile
createSourceFile(const char *contents, const char *options, unsigned serial,
                 Intel::OpenCL::ClangFE::IOCLFEBinaryResult *pResult = nullptr) {
  // composing a file name based on the current time
  std::stringstream fileName;
  std::string strContents(contents);

  if (pResult) {
    fileName << pResult->GetIRName();
  }

  fileName << serial << ".cl";
  Intel::OpenCL::Frontend::SourceFile ret = Intel::OpenCL::Frontend::SourceFile(
      std::string(fileName.str()), std::string(strContents),
      std::string(options));

  if (pResult) {
    Intel::OpenCL::Frontend::BinaryBuffer buffer(pResult->GetIR(),
                                                 pResult->GetIRSize());
    ret.setBinaryBuffer(buffer);
  }
  return ret;
}
#endif //OCLFRONTEND_PLUGINS

const char *GetOpenCLVersionStr(OPENCL_VERSION ver) {
  switch (ver) {
  case OPENCL_VERSION_1_2:
    return "120";
  case OPENCL_VERSION_2_0:
    return "200";
  case OPENCL_VERSION_2_1:
    return "210";
  default:
    throw "Unknown OpenCL version";
  }
}

std::string GetCurrentDir() {
  char szCurrDirrPath[MAX_STR_BUFF];
  if (!GET_CURR_WORKING_DIR(MAX_STR_BUFF, szCurrDirrPath))
    return std::string();

  std::stringstream ss;

  ss << "\"" << szCurrDirrPath << "\"";

  return ss.str();
}

int ClangFECompilerCompileTask::Compile(IOCLFEBinaryResult **pBinaryResult) {
  LOG_INFO(TEXT("%s"), TEXT("enter"));

  bool bProfiling = std::string(m_pProgDesc->pszOptions).find("-profiling") !=
                    std::string::npos;
  bool bRelaxedMath =
      std::string(m_pProgDesc->pszOptions).find("-cl-fast-relaxed-math") !=
      std::string::npos;

  // Force the -profiling option if such was not supplied by user
  std::stringstream options;
  options << m_pProgDesc->pszOptions;

  if (m_sDeviceInfo.bEnableSourceLevelProfiling && !bProfiling) {
    options << " -profiling";
  }

  // Passing -cl-fast-relaxed-math option if specifed in the environment
  // variable or in the config
  const bool useRelaxedMath = m_config.UseRelaxedMath();

  if (useRelaxedMath && !bRelaxedMath) {
    options << " -cl-fast-relaxed-math";
  }

  std::stringstream optionsEx;
  // Add current directory
  optionsEx << " -I" << GetCurrentDir();
  optionsEx << " -mstackrealign";
  optionsEx << " -D__ENDIAN_LITTLE__=1";

  std::stringstream supportedExtensions;
  supportedExtensions << m_sDeviceInfo.sExtensionStrings;

  // If working as fpga emulator, pass special triple.
  if (m_pProgDesc->bFpgaEmulator) {
#if defined(_WIN64) || defined(__x86_64__) || defined(_M_AMD64) ||             \
    defined(_M_X64)
    options << " -triple spir64-unknown-unknown-intelfpga";
#elif defined(_WIN32) || defined(i386) || defined(__i386__) ||                 \
    defined(__x86__) || defined(__ANDROID__)
    options << " -triple spir-unknown-unknown-intelfpga";
#else
#error "Can't define target triple: unknown architecture."
#endif

    supportedExtensions << " cl_altera_channels";
  }

  if (m_sDeviceInfo.bImageSupport) {
    optionsEx << " -D__IMAGE_SUPPORT__=1";
  }

  IOCLFEBinaryResultPtr spBinaryResult;

  int res = ::Compile(m_pProgDesc->pProgramSource, m_pProgDesc->pInputHeaders,
                      m_pProgDesc->uiNumInputHeaders,
                      m_pProgDesc->pszInputHeadersNames, 0, 0,
                      options.str().c_str(),   // pszOptions
                      optionsEx.str().c_str(), // pszOptionsEx
                      supportedExtensions.str().c_str(),
                      GetOpenCLVersionStr(m_config.GetOpenCLVersion()),
                      spBinaryResult.getOutPtr());

#ifdef OCLFRONTEND_PLUGINS
  if (getenv("OCLBACKEND_PLUGINS") && getenv("OCL_DISABLE_SOURCE_RECORDER")) {
    Intel::OpenCL::Frontend::CompileData compileData;
    Intel::OpenCL::Frontend::SourceFile sourceFile =
        createSourceFile(m_pProgDesc->pProgramSource, m_pProgDesc->pszOptions,
                         0, spBinaryResult.get());
    compileData.sourceFile(sourceFile);
    for (unsigned headerCount = 0; headerCount < m_pProgDesc->uiNumInputHeaders;
         headerCount++) {
      compileData.addIncludeFile(
          createSourceFile(m_pProgDesc->pszInputHeadersNames[headerCount],
                           "", // include files comes without compliation flags
                           headerCount + 1));
    }
    g_pluginManager.OnCompile(&compileData);
  }
#endif // OCLFRONTEND_PLUGINS

  if (pBinaryResult) {
    *pBinaryResult = spBinaryResult.release();
  }
  return res;
}

//
// ClangFECompilerLinkTask calls implementation
//
int ClangFECompilerLinkTask::Link(IOCLFEBinaryResult **pBinaryResult) {
  std::vector<void *> m_Binaries;
  std::vector<size_t> m_BinariesSizes;

  for (unsigned int i = 0; i < m_pProgDesc->uiNumBinaries; ++i) {
    if (CacheBinaryReader::IsValidCacheObject(
            m_pProgDesc->pBinaryContainers[i],
            m_pProgDesc->puiBinariesSizes[i])) {
      CacheBinaryReader reader(m_pProgDesc->pBinaryContainers[i],
                               m_pProgDesc->puiBinariesSizes[i]);
      m_Binaries.push_back((void *)reader.GetSectionData(g_irSectionName));
      m_BinariesSizes.push_back(reader.GetSectionSize(g_irSectionName));
    } else if (OCLElfBinaryReader::IsValidOpenCLBinary(
                   (const char *)m_pProgDesc->pBinaryContainers[i],
                   m_pProgDesc->puiBinariesSizes[i])) {
      OCLElfBinaryReader reader((const char *)m_pProgDesc->pBinaryContainers[i],
                                m_pProgDesc->puiBinariesSizes[i]);
      char *pBinaryData = nullptr;
      size_t uiBinaryDataSize = 0;
      reader.GetIR(pBinaryData, uiBinaryDataSize);
      m_Binaries.push_back(pBinaryData);
      m_BinariesSizes.push_back(uiBinaryDataSize);
    } else {
      m_Binaries.push_back((void *)m_pProgDesc->pBinaryContainers[i]);
      m_BinariesSizes.push_back(m_pProgDesc->puiBinariesSizes[i]);
    }
  }

  IOCLFEBinaryResultPtr spBinaryResult;

  int res = ::Link((const void **)m_Binaries.data(), m_pProgDesc->uiNumBinaries,
                   m_BinariesSizes.data(), m_pProgDesc->pszOptions,
                   spBinaryResult.getOutPtr());

#ifdef OCLFRONTEND_PLUGINS
  if (getenv("OCLBACKEND_PLUGINS") && getenv("OCL_DISABLE_SOURCE_RECORDER")) {
    Intel::OpenCL::Frontend::LinkData linkData;

    for (unsigned int i = 0; i < m_Binaries.size(); ++i) {
      linkData.addInputBuffer(m_Binaries[i], m_BinariesSizes[i]);
    }
    linkData.setOptions(m_pProgDesc->pszOptions);
    linkData.setBinaryResult(spBinaryResult.get());
    g_pluginManager.OnLink(&linkData);
  }
#endif // OCLFRONTEND_PLUGINS

  if (pBinaryResult) {
    *pBinaryResult = spBinaryResult.release();
  }
  return res;
}

namespace spv {
// See Table 1. "First Words of Physical Layout" of SPIR-V specification
enum { MagicNumberIdx = 0, SPIRVVersionIdx = 1, FirstOpIdx = 5 };
}

ClangFECompilerParseSPIRVTask::ClangFECompilerParseSPIRVTask(
    Intel::OpenCL::FECompilerAPI::FESPIRVProgramDescriptor *pProgDesc,
    Intel::OpenCL::ClangFE::CLANG_DEV_INFO const &sDeviceInfo)
    : m_pProgDesc(pProgDesc), m_sDeviceInfo(sDeviceInfo) {

  uint32_t const *spirvBC =
      reinterpret_cast<std::uint32_t const *>(pProgDesc->pSPIRVContainer);
  m_littleEndian = spirvBC[spv::MagicNumberIdx] == spv::MagicNumber;
}

std::uint32_t ClangFECompilerParseSPIRVTask::getSPIRVWord(
    std::uint32_t const *wordPtr) const {
  return m_littleEndian ? *wordPtr : llvm::sys::SwapByteOrder_32(*wordPtr);
}

bool ClangFECompilerParseSPIRVTask::isSPIRVSupported() const {
  std::uint32_t const *spirvBC =
      reinterpret_cast<std::uint32_t const *>(m_pProgDesc->pSPIRVContainer);
  // Pointer behind the last work in SPIR-V BC
  std::uint32_t const *const spirvEnd =
      spirvBC + m_pProgDesc->uiSPIRVContainerSize / 4;

  if (spirvEnd <= spirvBC + spv::FirstOpIdx)
    return false;
  // Check SPIR-V magic number is sane
  std::uint32_t const magicNumber = spirvBC[spv::MagicNumberIdx];
  if (!m_littleEndian &&
      magicNumber != llvm::sys::SwapByteOrder_32(spv::MagicNumber))
    return false;

  // SPIR-V version is 1.0
  std::uint32_t const version = getSPIRVWord(spirvBC + spv::SPIRVVersionIdx);
  if (version != spv::Version)
    return false;

  // Look for OpCapability instructions and check the declared capabilites
  // are supported by CPU device.
  std::uint32_t const *currentWord = spirvBC + spv::FirstOpIdx;
  while (currentWord < spirvEnd) {
    std::uint32_t const word = getSPIRVWord(currentWord);
    std::uint16_t wordCount = word >> 16;
    std::uint16_t opCode = word;
    // According to logical layout defined by SPIR-V spec. OpMemoryModel is
    // requeied
    if (opCode == spv::OpCapability) {
      std::uint32_t const capability = getSPIRVWord(currentWord + 1);
      switch (capability) {
      default:
        return false;
      case spv::CapabilityImageBasic:
      case spv::CapabilitySampledBuffer:
      case spv::CapabilitySampled1D:
      case spv::CapabilityImageReadWrite:
        if (!m_sDeviceInfo.bImageSupport)
          return false;
        break;

      case spv::CapabilityFloat64:
        if (!m_sDeviceInfo.bDoubleSupport)
          return false;
        break;

      // The following capabilities are common for all OpenCL 2.1 capable
      // devices
      case spv::CapabilityAddresses:
      case spv::CapabilityLinkage:
      case spv::CapabilityKernel:
      case spv::CapabilityVector16:
      case spv::CapabilityFloat16Buffer:
      case spv::CapabilityInt64:
      case spv::CapabilityPipes:
      case spv::CapabilityGroups:
      case spv::CapabilityDeviceEnqueue:
      case spv::CapabilityLiteralSampler:
      case spv::CapabilityInt16:
      case spv::CapabilityGenericPointer:
      case spv::CapabilityInt8:
        break;
      }
      // OpMemoryModel is a mandatory entry in a valid SPIR-V module
    } else if (opCode == spv::OpMemoryModel) {
      if (getSPIRVWord(currentWord + 2) != spv::MemoryModelOpenCL)
        return false;
      return true;
    }
    // Go for the next SPIR-V op.
    currentWord = currentWord + wordCount;
  }

  return false;
}

//
// ClangFECompilerParseSPIRVTask call implementation.
// Description:
// Implements conversion from a SPIR-V 1.0 program (incapsulated in ClangFECompilerParseSPIRVTask)
// to a llvm::Module, converts build options to LLVM metadata according to SPIR specification.
int ClangFECompilerParseSPIRVTask::ParseSPIRV(
    IOCLFEBinaryResult **pBinaryResult) {
  std::unique_ptr<OCLFEBinaryResult> pResult(new OCLFEBinaryResult());

  // verify build options
  unsigned int uiUnrecognizedOptionsSize = strlen(m_pProgDesc->pszOptions) + 1;
  std::unique_ptr<char> szUnrecognizedOptions(
      new char[uiUnrecognizedOptionsSize]);
  szUnrecognizedOptions.get()[uiUnrecognizedOptionsSize - 1] = '\0';

  if (!::CheckCompileOptions(m_pProgDesc->pszOptions,
                             szUnrecognizedOptions.get(),
                             uiUnrecognizedOptionsSize)) {
    std::stringstream errorMessage;
    errorMessage << "Unrecognized build options: ";
    errorMessage << szUnrecognizedOptions.get();
    errorMessage << "\n";
    pResult->setLog(errorMessage.str());

    if (pBinaryResult) {
      *pBinaryResult = pResult.release();
    }

    return CL_INVALID_COMPILER_OPTIONS;
  }

  // verify SPIR-V module is supported
  if (!isSPIRVSupported()) {
    if (pBinaryResult) {
      pResult->setLog("Usupported SPIR-V module\n");
      *pBinaryResult = pResult.release();
    }
    return CL_INVALID_PROGRAM;
  }

  // parse SPIR-V
  std::unique_ptr<llvm::LLVMContext> context(new llvm::LLVMContext());
  llvm::Module *pModule;
  std::string errorMsg;
  std::stringstream inputStream(
      std::string((const char *)m_pProgDesc->pSPIRVContainer,
                  m_pProgDesc->uiSPIRVContainerSize),
      std::ios_base::in);

  bool isParsed = llvm::ReadSPIRV(*context, inputStream, pModule, errorMsg);

  // Respect build options.
  // Compiler options layout in llvm metadata is defined by SPIR spec.
  // For example:
  // !opencl.compiler.options = !{!11}
  // !11 = !{!"-cl-fast-relaxed-math", !""-cl-mad-enable"}
  if (isParsed) {
    llvm::NamedMDNode *OCLCompOptsMD =
        pModule->getOrInsertNamedMetadata("opencl.compiler.options");
    // we do not expect spir-v parser to handle build options
    assert(OCLCompOptsMD->getNumOperands() == 0 &&
           "SPIR-V parser is not expected to handle compile options");

    if (OCLCompOptsMD->getNumOperands() == 0) {
      llvm::SmallVector<llvm::Metadata *, 5> OCLBuildOptions;

      std::vector<std::string> buildOptionsSeparated;
      std::stringstream optionsStrstream(m_pProgDesc->pszOptions);
      std::copy(std::istream_iterator<std::string>(optionsStrstream),
                std::istream_iterator<std::string>(),
                std::back_inserter(buildOptionsSeparated));

      for (auto option : buildOptionsSeparated) {
        OCLBuildOptions.push_back(llvm::MDString::get(*context, option));
      }

      OCLCompOptsMD->addOperand(llvm::MDNode::get(*context, OCLBuildOptions));
    }
  }

  assert(!verifyModule(*pModule) &&
         "SPIR-V consumer returned a broken module!");

  // setting the result in both sucessful an uncussessful cases
  // to pass the error log.

  // serialize to LLVM bitcode
  llvm::raw_svector_ostream ir_ostream(pResult->getIRBufferRef());
  llvm::WriteBitcodeToFile(pModule, ir_ostream);

  pResult->setLog(errorMsg);
  pResult->setIRType(IR_TYPE_COMPILED_OBJECT);
  pResult->setIRName(pModule->getName());

  if (pBinaryResult) {
    *pBinaryResult = pResult.release();
  }

  return isParsed ? CL_SUCCESS : CL_INVALID_PROGRAM;
}

int ClangFECompilerGetKernelArgInfoTask::GetKernelArgInfo(
    const void *pBin, size_t uiBinarySize, const char *szKernelName,
    IOCLFEKernelArgInfo **ppResult) {
  char *pIRBuffer = nullptr;
  size_t uiIRBufferSize = 0;

  if (CacheBinaryReader::IsValidCacheObject(pBin, uiBinarySize)) {
    CacheBinaryReader reader(pBin, uiBinarySize);
    pIRBuffer = (char *)reader.GetSectionData(g_irSectionName);
    uiIRBufferSize = reader.GetSectionSize(g_irSectionName);
  } else if (OCLElfBinaryReader::IsValidOpenCLBinary((const char *)pBin,
                                                     uiBinarySize)) {
    OCLElfBinaryReader reader((const char *)pBin, uiBinarySize);
    reader.GetIR(pIRBuffer, uiIRBufferSize);
  } else {
    pIRBuffer = const_cast<char *>((const char *)pBin);
    uiIRBufferSize = uiBinarySize;
  }
  return ::GetKernelArgInfo((const void *)pIRBuffer, uiIRBufferSize,
                            szKernelName, ppResult);
}

bool Intel::OpenCL::ClangFE::ClangFECompilerCheckCompileOptions(
    const char *szOptions, char *szUnrecognizedOptions,
    size_t uiUnrecognizedOptionsSize,
    const Intel::OpenCL::Utils::BasicCLConfigWrapper &) {
  return ::CheckCompileOptions(szOptions, szUnrecognizedOptions,
                               uiUnrecognizedOptionsSize);
}

bool Intel::OpenCL::ClangFE::ClangFECompilerCheckLinkOptions(
    const char *szOptions, char *szUnrecognizedOptions,
    size_t uiUnrecognizedOptionsSize) {
  return ::CheckLinkOptions(szOptions, szUnrecognizedOptions,
                            uiUnrecognizedOptionsSize);
}
