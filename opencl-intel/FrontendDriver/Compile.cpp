// INTEL CONFIDENTIAL
//
// Copyright 2018 Intel Corporation.
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

#include "cl_autoptr_ex.h"
#include "cl_config.h"
#include "cl_cpu_detect.h"
#include "cl_env.h"
#include "clang_device_info.h"
#include "common_clang.h"
#include "Compile.h"

#include <llvm/ADT/StringRef.h>

#include <sstream>

using namespace Intel::OpenCL::ClangFE;
using namespace Intel::OpenCL::Utils;

typedef auto_ptr_ex<IOCLFEBinaryResult, ReleaseDP<IOCLFEBinaryResult>>
    IOCLFEBinaryResultPtr;

#if defined(_WIN32)
#define GET_CURR_WORKING_DIR(len, buff) GetCurrentDirectoryA(len, buff)
#define PASS_PCH
#else
#define GET_CURR_WORKING_DIR(len, buff) getcwd(buff, len)
#endif

#define MAX_STR_BUFF 1024

#ifdef OCLFRONTEND_PLUGINS
#include "compile_data.h"
#include "plugin_manager.h"
#include "source_file.h"
Intel::OpenCL::PluginManager g_pluginManager;

//
// Creates a source file object from a given contents string, and a serial
// identifier.
//
static Intel::OpenCL::Frontend::SourceFile createSourceFile(
    const char *contents, const char *options, unsigned serial,
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
#endif // OCLFRONTEND_PLUGINS


std::string GetCurrentDir() {
  char szCurrDirrPath[MAX_STR_BUFF];
  if (!GET_CURR_WORKING_DIR(MAX_STR_BUFF, szCurrDirrPath))
    return std::string();

  std::stringstream ss;

  ss << "\"" << szCurrDirrPath << "\"";

  return ss.str();
}

static std::string getCPUSignatureMacro() {
  const std::string ArchName = CPUDetect::GetInstance()->GetCPUArchShortName();
  assert(!ArchName.empty() && "Arch name is empty!!!");

  std::string CPUSignature(" -D__INTEL_OPENCL_CPU_");
  CPUSignature += ArchName + "__=1";
  return CPUSignature;
}

const char *GetOpenCLVersionStr(OPENCL_VERSION ver) {
  switch (ver) {
  case OPENCL_VERSION_1_0:
    return "100";
  case OPENCL_VERSION_1_2:
    return "120";
  case OPENCL_VERSION_2_0:
    return "200";
  case OPENCL_VERSION_2_1:
    return "210";
  case OPENCL_VERSION_2_2:
    return "220";
  default:
    throw "Unknown OpenCL version";
  }
}

static bool shouldIgnoreOptionForDebug(llvm::StringRef opt) {
  return opt.contains("openmp") || opt == "-fintel-compatibility";
}

int ClangFECompilerCompileTask::Compile(IOCLFEBinaryResult **pBinaryResult) {
  bool bProfiling   = false,
       bDebug       = false,
       bRelaxedMath = false,
       bNoOpts      = false;

  llvm::SmallVector<llvm::StringRef, 8> splittedOptions;
  llvm::StringRef(m_pProgDesc->pszOptions).split(splittedOptions, " ");
  for (const auto opt : splittedOptions) {
    if (opt.str() == "-profiling") bProfiling = true;
    if (opt.str() == "-g") bDebug = true;
    if (opt.str() == "-cl-fast-relaxed-math") bRelaxedMath = true;
    if (opt.str() == "-cl-opt-disable") bNoOpts = true;
  }

  std::stringstream options;
  options << m_pProgDesc->pszOptions;

  // Force the -profiling option if such was not supplied by user
  if (m_sDeviceInfo.bEnableSourceLevelProfiling && !bProfiling)
    options << " -profiling";

  // By default clang compiles OpenCL sources with '-O2' optimization level
  // Force it to -O0 in case of compilation with debug information
  if (bDebug && !bNoOpts) {
    options << " -cl-opt-disable";
    bNoOpts = true;
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
  optionsEx << getCPUSignatureMacro();

  // Triple spir assumes that all extensions should be supported.
  // To tell to compiler which extensions are actually supported by this
  // particular device we pass supported extensions via -cl-ext option.
  // Order of extensions matters! Later overwrites former.
  // First of all we disable *all* extension and then we enable extension
  // per the device info.
  llvm::StringRef ExtStr(m_sDeviceInfo.sExtensionStrings);
  llvm::SmallVector<llvm::StringRef, 16> ExtVec;
  ExtStr.split(ExtVec, ' ', -1, false);
  optionsEx << " -cl-ext=-all";
  for (auto Ext : ExtVec)
    optionsEx << ",+" << Ext.str();

  // If working as fpga emulator, pass special triple.
  if (m_pProgDesc->bFpgaEmulator) {
#if defined(_WIN64) || defined(__x86_64__) || defined(_M_AMD64) ||             \
    defined(_M_X64)
    options << " -triple spir64-unknown-unknown-intelfpga";
#elif defined(_WIN32) || defined(i386) || defined(__i386__) || defined(__x86__)
    options << " -triple spir-unknown-unknown-intelfpga";
#else
#error "Can't define target triple: unknown architecture."
#endif

    // For now we can enable FPGA emulation only for the whole OpenCL context.
    // It is deemed that a better approach would be to have FPGA emulator as
    // another device. Then cl_intel_channels extension should be in
    // m_sDeviceInfo.sExtensionStrings and can be handled uniformly with other
    // extensions supported by the device. But according to Andrew Savonichev,
    // the device based approach has several flaws:
    // 1. We have a single BE instance for all devices, it would be equally
    //    difficult to implement a check for FPGA in the BE.
    // 2. ATM we can link against libintelocl.so to avoid device selection.
    //    If libintelocl.so provides 2 devices, we would need to patch all
    //    benchmarks/samples with a device selection code.
    // 3. I think we only used 'experimental 2.x' as a separate platform
    //    (not a device). Having 2 devices seems to be an overkill for this
    //    purpose.
    // In FPGA HW: Since some of the extensions are either partially supported,
    // or are not yet conformant (have not been tested or do not pass
    // conformance tests), what we claim to support in the platform/device
    // queries should be different from what we allow and have implemented.
    // We're aligning with it's behavior in FPGA emulator.
    optionsEx << " -cl-ext=+cl_intel_channels";
    optionsEx << " -cl-ext=+cl_khr_local_int32_base_atomics ";
    optionsEx << " -cl-ext=+cl_khr_local_int32_extended_atomics ";
    optionsEx << " -cl-ext=+cl_khr_global_int32_base_atomics ";
    optionsEx << " -cl-ext=+cl_khr_global_int32_extended_atomics ";
    optionsEx << " -cl-ext=+cl_khr_fp64";
    optionsEx << " -cl-ext=+cl_khr_fp16";

    optionsEx << " -DINTELFPGA_CL";
  }

  if (m_sDeviceInfo.bImageSupport) {
    optionsEx << " -D__IMAGE_SUPPORT__=1";
  }

  // In case of compilation with '-cl-opt-disable' option clang generates
  // 'optnone' attribute for all functions including kernels.
  // It conflicts with some optimizations we need to keep functional correctness
  // Pass '-disable-0O-optnone' to disable the implicit 'optnone'
  if (bNoOpts) {
    optionsEx << " -disable-O0-optnone";
  }

  if (m_pProgDesc->bFpgaEmulator) {
    std::string optionsClang;

// INTEL VPO BEGIN
    // FIXME: OpenMP is not enabled by default, because it requires
    // -fintel-compatibility flag, which affect on clang behavior beyond OpenMP.
    // Should be enabled back when this issue gets resolved.
    //
    // optionsClang = "-fopenmp -fintel-openmp -fopenmp-tbb -fintel-compatibility";
// INTEL VPO END

    std::string envVolcanoClangOptions;
    cl_err_code err = GetEnvVar(envVolcanoClangOptions,
        "VOLCANO_CLANG_OPTIONS");
    if (!CL_FAILED(err)) {
#ifdef NDEBUG
      // Append user options to default options.
      optionsClang += envVolcanoClangOptions;
#else
      // Allow default OpenMP flags to be overridden for debug purposes.
      optionsClang = envVolcanoClangOptions;
#endif
    }

    if (!optionsClang.empty()) {
      std::stringstream optionsSS(optionsClang);
      std::string buf;
      while (getline(optionsSS, buf,' ')) {
        if (bDebug && shouldIgnoreOptionForDebug(buf)) {
          continue;
        }
        optionsEx << " " << buf;
      }
    }
  }

#ifndef INTEL_PRODUCT_RELEASE
  llvm::StringRef intermediateType(getenv("OCL_INTERMEDIATE"));
  if (intermediateType.equals("SPIRV")) {
    optionsEx << " -emit-spirv";
  }
#endif // INTEL_PRODUCT_RELEASE

  IOCLFEBinaryResultPtr spBinaryResult;

  int res = ::Compile(m_pProgDesc->pProgramSource, m_pProgDesc->pInputHeaders,
                      m_pProgDesc->uiNumInputHeaders,
                      m_pProgDesc->pszInputHeadersNames, 0, 0,
                      options.str().c_str(),   // pszOptions
                      optionsEx.str().c_str(), // pszOptionsEx
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

bool Intel::OpenCL::ClangFE::ClangFECompilerCheckCompileOptions(
    const char *szOptions, char *szUnrecognizedOptions,
    size_t uiUnrecognizedOptionsSize, const BasicCLConfigWrapper &) {
  return ::CheckCompileOptions(szOptions, szUnrecognizedOptions,
                               uiUnrecognizedOptionsSize);
}
