// INTEL CONFIDENTIAL
//
// Copyright 2018-2021 Intel Corporation.
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

#include "Compile.h"
#include "cl_autoptr_ex.h"
#include "cl_config.h"
#include "cl_cpu_detect.h"
#include "cl_env.h"
#include "clang_device_info.h"
#include "common_clang.h"
#include "opencl_c_features.h"

#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>

#include <sstream>

using namespace Intel::OpenCL::ClangFE;
using namespace Intel::OpenCL::Utils;

typedef auto_ptr_ex<IOCLFEBinaryResult, ReleaseDP<IOCLFEBinaryResult>>
    IOCLFEBinaryResultPtr;

extern const char OPENCL_CTH_PRE_RELEASE_H[];
extern unsigned int OPENCL_CTH_PRE_RELEASE_H_size;
extern const char OPENCL_CTH_PRE_RELEASE_H_name[];

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
  case OPENCL_VERSION_3_0:
    return "300";
  default:
    throw "Unknown OpenCL version";
  }
}

int ClangFECompilerCompileTask::Compile(IOCLFEBinaryResult **pBinaryResult) {
  bool bProfiling = false, bRelaxedMath = false;

  llvm::SmallVector<llvm::StringRef, 8> splittedOptions;
  llvm::StringRef(m_pProgDesc->pszOptions).split(splittedOptions, " ");
  for (const auto opt : splittedOptions) {
    if (opt.str() == "-profiling")
      bProfiling = true;
    if (opt.str() == "-cl-fast-relaxed-math")
      bRelaxedMath = true;
  }

  std::stringstream options;
  options << m_pProgDesc->pszOptions;

  // Force the -profiling option if such was not supplied by user
  if (m_sDeviceInfo.bEnableSourceLevelProfiling && !bProfiling)
    options << " -profiling";

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

  optionsEx << " -Dcl_intel_device_attribute_query";
  optionsEx << " -Dcl_intel_required_subgroup_size";
  optionsEx << " -Dcl_intel_subgroups_char";
  optionsEx << " -Dcl_intel_subgroups_long";
  // FIXME: Re-claim cl_khr_subgroup_ballot support when we implement all
  // required builtins.
  // optionsEx << " -Dcl_khr_subgroup_ballot";
  optionsEx << " -Dcl_khr_subgroup_shuffle";
  optionsEx << " -Dcl_khr_subgroup_shuffle_relative";
  optionsEx << " -Dcl_khr_subgroup_extended_types";
  optionsEx << " -Dcl_khr_subgroup_non_uniform_arithmetic";

  // Define OpenCL C 3.0 feature macros.
  // FPGA emulator only support OpenCL C 1.2, so no macro will be defined.
  if (!m_pProgDesc->bFpgaEmulator) {
    optionsEx << " -cl-ext=+" OPENCL_C_3D_IMAGE_WRITES;
    optionsEx << " -cl-ext=+" OPENCL_C_ATOMIC_ORDER_ACQ_REL;
    optionsEx << " -cl-ext=+" OPENCL_C_ATOMIC_ORDER_SEQ_CST;
    optionsEx << " -D" OPENCL_C_ATOMIC_SCOPE_DEVICE "=1";
    optionsEx << " -D" OPENCL_C_ATOMIC_SCOPE_ALL_DEVICES "=1";
    optionsEx << " -cl-ext=+" OPENCL_C_DEVICE_ENQUEUE;
    optionsEx << " -cl-ext=+" OPENCL_C_GENERIC_ADDRESS_SPACE;
    optionsEx << " -cl-ext=+" OPENCL_C_FP64;
    optionsEx << " -cl-ext=+" OPENCL_C_IMAGES;
    optionsEx << " -cl-ext=+" OPENCL_C_INT64;
    optionsEx << " -cl-ext=+" OPENCL_C_PIPES;
    optionsEx << " -cl-ext=+" OPENCL_C_PROGRAM_SCOPE_GLOBAL_VARIABLES;
    optionsEx << " -cl-ext=+" OPENCL_C_READ_WRITE_IMAGES;
    optionsEx << " -cl-ext=+" OPENCL_C_SUBGROUPS;
    optionsEx << " -D" OPENCL_C_WORK_GROUP_COLLECTIVE_FUNCTIONS "=1";
  }

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
    optionsEx << " -cl-ext=+cl_khr_3d_image_writes ";
  }

  if (m_sDeviceInfo.bImageSupport)
    optionsEx << " -D__IMAGE_SUPPORT__=1";

#ifndef INTEL_PRODUCT_RELEASE
  std::string IntermediateType;
  Intel::OpenCL::Utils::getEnvVar(IntermediateType, "OCL_INTERMEDIATE");
  if (IntermediateType == "SPIRV")
    optionsEx << " -emit-spirv";
#endif // INTEL_PRODUCT_RELEASE

  // Reallocate headers to include another one
  std::vector<const char *> InputHeaders;
  std::vector<const char *> InputHeadersNames;
  InputHeaders.assign(m_pProgDesc->pInputHeaders,
                      m_pProgDesc->pInputHeaders +
                          m_pProgDesc->uiNumInputHeaders);
  InputHeadersNames.assign(m_pProgDesc->pszInputHeadersNames,
                           m_pProgDesc->pszInputHeadersNames +
                               m_pProgDesc->uiNumInputHeaders);

  // Input header with OpenCL pre-release extensions
  // Skip Emulator devices
  if (!m_pProgDesc->bFpgaEmulator) {
    // Append the header to the program source
    InputHeaders.push_back(OPENCL_CTH_PRE_RELEASE_H);
    InputHeadersNames.push_back(OPENCL_CTH_PRE_RELEASE_H_name);

    optionsEx << " -include " << OPENCL_CTH_PRE_RELEASE_H_name;
  }

  IOCLFEBinaryResultPtr spBinaryResult;

  int res = ::Compile(m_pProgDesc->pProgramSource, InputHeaders.data(),
                      InputHeaders.size(), InputHeadersNames.data(), 0, 0,
                      options.str().c_str(),   // pszOptions
                      optionsEx.str().c_str(), // pszOptionsEx
                      GetOpenCLVersionStr(m_config.GetOpenCLVersion()),
                      spBinaryResult.getOutPtr());

#ifdef OCLFRONTEND_PLUGINS
  std::string Env;
  if (Intel::OpenCL::Utils::getEnvVar(Env, "OCLBACKEND_PLUGINS") &&
      Intel::OpenCL::Utils::getEnvVar(Env, "OCL_DISABLE_SOURCE_RECORDER")) {
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
