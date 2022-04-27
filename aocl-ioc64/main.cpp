//===-- main.cpp - Homegrown version of SSG's ioc64 program to compile and link
// OpenCL for the fast emulator
//
// INTEL CONFIDENTIAL
// Copyright 2020 Intel Corporation
//
// The source code contained or described herein and all documents related to
// the source code ("Material") are owned by Intel Corporation or its suppliers
// or licensors. Title to the Material remains with Intel Corporation or its
// suppliers and licensors. The Material contains trade secrets and proprietary
// and confidential information of Intel or its suppliers and licensors. The
// Material is protected by worldwide copyright and trade secret laws and
// treaty provisions. No part of the Material may be used, copied, reproduced,
// modified, published, uploaded, posted, transmitted, distributed, or
// disclosed in any way without Intel's prior express written permission.
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery
// of the Materials, either expressly, by implication, inducement, estoppel or
// otherwise. Any license under such intellectual property rights must be
// express and approved by Intel in writing.
//
//===----------------------------------------------------------------------===//
//
// This is the ACL ioc driver program.  It provides an interface to the fast
// emulator.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/Path.h"

#include <iostream>
#include <iterator>
#include <fstream>
#include <list>
#include <set>
#include <system_error>

// OpenCL headers
#include <CL/cl_ext.h> // From icd

using namespace llvm;
#define DEBUG_TYPE "ioc64"
#define DEBUG LLVM_DEBUG

inline bool CL_FAILED(cl_int ReturnCode) { return CL_SUCCESS != ReturnCode; }

inline bool CL_SUCCEEDED(cl_int ReturnCode) { return CL_SUCCESS == ReturnCode; }

#define CL_ERR_FAILURE -2800 // from cl_types.h

typedef std::unique_ptr<cl_program, std::function<void(cl_program *)>>
    cl_program_sptr;

static int GetDeviceParam(cl_device_id clDeviceId, cl_device_info clDeviceInfo,
                          std::string &strParamValueRet);
static int GetBuildLog(cl_program clProgram, cl_device_id clDeviceId,
                       std::string &strBuildLog);
static int getProgramBinaries();
static const char *myClErrTxt(cl_int error_code);
static cl_program_sptr
getProgramsFromBinaries(const std::list<std::string> &lst, cl_context &con,
                        cl_device_id &device);
static cl_program getProgramFromSPIRV(const std::string &spv, cl_context &con,
                                      cl_device_id &device);

static std::string formatClError(const std::string &Base, cl_int CLError) {
  return Base + ": " + std::to_string(CLError) + " (" + myClErrTxt(CLError) + ")";
}

// General options for ioc64.
static cl::opt<std::string> BuildOptions("bo", cl::desc("Build options"));

namespace {
// Avoid conflict with 'link' from unistd.h included from the OpenCL includes
enum Commands { build, compile, link };
static cl::opt<Commands> Command(
    "cmd", cl::desc("Command"),
    cl::values(
        clEnumVal(build,
                  "Build executable IR from OpenCL source or OpenCL .obj file"),
        clEnumVal(compile, "Compile from OpenCL source to .obj"),
        clEnumVal(link, "Link OpenCL .obj file(s)")));
} // namespace

enum SupportedDevices { fpga_fast_emu };
static cl::opt<SupportedDevices> Device(
    "xxdevice", cl::desc("Target device"),
    cl::values(clEnumVal(fpga_fast_emu, "Compile for the FPGA emulator")));

static cl::opt<bool> Version("xxversion", cl::desc("Print compiler version"));

static cl::opt<bool>
    ForceAVX("force-avx", cl::desc("Force code generation for Intel(R) AVX"));

static cl::opt<std::string> Input("input", cl::desc("Input OpenCl file"));

static cl::opt<std::string> IR("ir", cl::desc("Output file"));

static cl::opt<std::string> SPIRV("spv", cl::desc("Input SPIR-V file"));

static cl::opt<std::string>
    Binaries("binary", cl::desc("Input OpenCL .obj files to link"));

namespace {
cl_platform_id platform = 0;
cl_context context = 0;
cl_program program = nullptr;
cl_device_id device = 0;
} // namespace

#define STR_INTEL_PLATFORM_FAST_EMU                                            \
  "Intel(R) FPGA Emulation Platform"

static int getFPGAPlatform() {
  cl_uint numPlatforms = 0;
  const cl_uint maxPlatforms = 10;
  const size_t maxPlatformNameLength = 256;
  cl_platform_id platforms[maxPlatforms];
  cl_int err = clGetPlatformIDs(maxPlatforms, &platforms[0], &numPlatforms);

  if (err != CL_SUCCESS || numPlatforms == 0) {
    // We didn't find any platforms
    std::cout << "Unable to load OpenCL platforms\n";
    return 1;
  }
  DEBUG(dbgs() << "Found " << numPlatforms << " platforms:\n";
        char platformName[maxPlatformNameLength];
        for (size_t i = 0; i < numPlatforms; i++) {
          clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME,
                            sizeof(platformName), &platformName[0], nullptr);
          dbgs() << i << ": " << platformName << '\n';
        });
  for (cl_uint i = 0; i < numPlatforms; i++) {
    char platformName[maxPlatformNameLength];
    err = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME,
                            sizeof(platformName), &platformName[0], nullptr);
    if (err == CL_SUCCESS && strncmp(STR_INTEL_PLATFORM_FAST_EMU, platformName,
                                     sizeof(STR_INTEL_PLATFORM_FAST_EMU) - 1) == 0) {
      platform = platforms[i];
      std::cout << "Platform name: " << platformName << '\n';
      return 0;
    }
  }
  std::cerr << "Unable to find Intel(R) FPGA Emulation Platform\n";
  return 1;
}

static int loadOpenCLContext() {
  int clErr = 0;
  if (getFPGAPlatform())
    return 1;

  clErr =
      clGetDeviceIDs(platform, CL_DEVICE_TYPE_ACCELERATOR, 1, &device, nullptr);
  if (CL_FAILED(clErr) || 0 == device) {
    DEBUG(
        if (CL_FAILED(clErr)) {
          dbgs() << "clGetDeviceIDs failed: " << clErr << '\n';
        } else { dbgs() << "clGetDeviceIDs returned 0 devices\n"; });
    DEBUG(dbgs() << formatClError("Failed to get OpenCL device...", clErr)
                 << '\n');
    return 1;
  }

  DEBUG(dbgs() << "Found device: " << device << '\n');

  DEBUG(dbgs() << "Creating context...\n");
  context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &clErr);
  if (CL_FAILED(clErr) || nullptr == context) {
    DEBUG(
        if (CL_FAILED(clErr)) {
          dbgs() << "clCreateContext failed: " << clErr << '\n';
        } else { dbgs() << "clCreateContext returned a null context\n"; });
    DEBUG(dbgs() << formatClError("Failed to create OpenCL context...", clErr)
                 << '\n');
    return 1;
  }

  DEBUG(dbgs() << "Created context\n");
  clErr = clGetContextInfo(context, CL_CONTEXT_DEVICES, sizeof(cl_device_id),
                           &device, nullptr);
  if (CL_FAILED(clErr)) {
    DEBUG(dbgs() << formatClError("Failed to get context's devices...", clErr)
                 << '\n');
    return 1;
  }

  std::string strDeviceInfo;
  if (CL_SUCCEEDED(GetDeviceParam(device, CL_DEVICE_NAME, strDeviceInfo))) {
    std::cout << "Device name: " + strDeviceInfo << '\n';
  }
  if (CL_SUCCEEDED(GetDeviceParam(device, CL_DEVICE_VERSION, strDeviceInfo))) {
    std::cout << "Device version: " + strDeviceInfo << '\n';
  }
  if (CL_SUCCEEDED(GetDeviceParam(device, CL_DEVICE_VENDOR, strDeviceInfo))) {
    std::cout << "Device vendor: " + strDeviceInfo << '\n';
  }
  if (CL_SUCCEEDED(GetDeviceParam(device, CL_DEVICE_PROFILE, strDeviceInfo))) {
    std::cout << "Device profile: " + strDeviceInfo << '\n';
  }

  return 0;
}

static std::string trim(const std::string &str) {
  auto copy = str;
  // Remove trailing spaces.
  while (!copy.empty() && isspace(copy[copy.size() - 1])) {
    copy.erase(copy.size() - 1);
  }

  // Find first non-space:
  size_t left_pos = 0;
  while (copy.size() > left_pos && isspace(copy[left_pos])) {
    left_pos++;
  }
  if (left_pos == 0)
    return copy;

  return copy.substr(left_pos);
}

const std::string SKX_STR =
    "Intel(R) Advanced Vector Extensions 512 (Intel(R) AVX-512)";
const std::string AVX2_STR =
    "Intel(R) Advanced Vector Extensions 2 (Intel(R) AVX2)";
const std::string AVX_STR =
    "Intel(R) Advanced Vector Extensions (Intel(R) AVX)";
const std::string SSE42_STR =
    "Intel(R) Streaming SIMD Extensions 4.2 (Intel(R) SSE4.2)";
const std::string SSSE3_STR =
    "Supplemental Streaming SIMD Extensions 3 (SSSE3)";

static void discoverCPUType() {
  // TODO: Does the user need to be able to set this?
  if (ForceAVX) {
    std::cout << "Setting target instruction set architecture to: AVX ("
              << AVX_STR << ")\n";
    // Force to a known CPU target;
#ifdef _WIN32
    _putenv("CL_CONFIG_CPU_TARGET_ARCH=corei7-avx");
#else
    setenv("CL_CONFIG_CPU_TARGET_ARCH", "corei7-avx", 1);
#endif
    return;
  }
  StringMap<bool> cpu_features;
  sys::getHostCPUFeatures(cpu_features);
  std::string type;
  if (cpu_features["avx2"]) {
    type = AVX2_STR;
  } else if (cpu_features["avx"]) {
    type = AVX_STR;
  } else if (cpu_features["sse4.2"]) {
    type = SSE42_STR;
  } else if (cpu_features["sse3"]) {
    type = SSSE3_STR;
  } else {
    type = "Unknown SIMD";
  }
  std::cout << "Setting target instruction set architecture to: Default ("
            << type << ")\n";

  // Clear out the environment to use the default setting
#ifdef _WIN32
  _putenv("CL_CONFIG_CPU_TARGET_ARCH=");
#else
  unsetenv("CL_CONFIG_CPU_TARGET_ARCH");
#endif
}

static int compileProgram() {
  // Add the parent path to the options as an include path.
  std::string options = BuildOptions;
  auto parent_dir = sys::path::parent_path(Input);
  if (!parent_dir.empty()) {
    options += " -I \"";
    options += parent_dir;
    options += "\"";
  }

  std::cout << "Using build options: " << options << '\n';
  discoverCPUType();

  int retcode = loadOpenCLContext();
  if (retcode) {
    exit(1);
  }

  DEBUG(dbgs() << "Reading input file: " << Input << '\n');
  sys::fs::file_status file_stat;
  auto err = sys::fs::status(Input, file_stat);
  if (err) {
    std::cout << "Unable to open input file '" << Input
              << "', error: " << err.message() << '\n';
    exit(1);
  }
  std::ifstream inputFile(Input.c_str());
  std::string code((std::istreambuf_iterator<char>(inputFile)),
              (std::istreambuf_iterator<char>()));
  inputFile.close();

  if (trim(code).empty()) {
    std::cout << "File '" << Input << " contains no code\n";
    exit(1);
  }

  DEBUG(dbgs() << "Creating program...\n");
  const char *oclCode = code.c_str();
  cl_int clErr = 0;
  program = clCreateProgramWithSource(context, 1, (const char **)&oclCode,
                                      nullptr, &clErr);
  if (CL_FAILED(clErr) || 0 == program) {
    DEBUG(
        if (CL_FAILED(clErr)) {
          dbgs() << "clCreateProgramWithSource failed: " << clErr << '\n';
        } else { dbgs() << "clCreateProgramWithSource return 0 program\n"; });
    clReleaseContext(context);
    context = 0;
    DEBUG(dbgs() << formatClError("Failed to create program from source...",
                                  clErr)
                 << '\n');
    return 1;
  }

  DEBUG(dbgs() << "Building program...\n");
  clErr = clCompileProgram(program, 1, &device, options.c_str(), 0, nullptr,
                           nullptr, nullptr, nullptr);
  std::string build_log;
  GetBuildLog(program, device, build_log);
  std::cout << build_log;

  if (CL_FAILED(clErr)) {
    clReleaseProgram(program);
    program = 0;
    clReleaseContext(context);
    context = 0;
    std::cout << formatClError("Failed to build program...", clErr)
              << "\nCompilation failed!\n";
    return 1;
  }

  return getProgramBinaries();
}

int getProgramBinaries() {
  // build succeded, getting the IR binary
  // in case of failure, the function will return CL_SUCCESS. the front end will
  // check for the file later on to see if the file was created or not

  size_t pszProgramBinarySize[1] = {0};
  unsigned char *pucBinaries[1] = {nullptr};
  auto clErr = clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES,
                                sizeof(size_t), pszProgramBinarySize, nullptr);
  if (CL_FAILED(clErr) || pszProgramBinarySize[0] == 0) {
    clReleaseProgram(program);
    program = 0;
    clReleaseContext(context);
    context = 0;
    DEBUG(
        if (pszProgramBinarySize[0] == 0) {
          dbgs() << "Binary size is 0\n";
        } else {
          dbgs() << formatClError("Failed to get IR binary size...", clErr)
                 << '\n';
        });
    return CL_SUCCESS;
  }

  pucBinaries[0] = new unsigned char[pszProgramBinarySize[0]];
  clErr = clGetProgramInfo(program, CL_PROGRAM_BINARIES,
                           sizeof(unsigned char *), pucBinaries, nullptr);
  if (CL_FAILED(clErr)) {
    delete[] pucBinaries[0];
    clReleaseProgram(program);
    program = 0;
    clReleaseContext(context);
    context = 0;
    DEBUG(dbgs() << formatClError("Failed to get IR binary data...", clErr)
                 << '\n');
    return CL_SUCCESS;
  }

  FILE *pIRFile = fopen(IR.c_str(), "wb+");
  if (nullptr == pIRFile) {
    delete[] pucBinaries[0];
    clReleaseProgram(program);
    program = 0;
    clReleaseContext(context);
    context = 0;
    DEBUG(dbgs() << "Failed to save program binary...\n");
    return CL_SUCCESS;
  }

  const unsigned char *pucBinaryDataToWrite = pucBinaries[0];
  size_t BinaryDataToWriteSize = pszProgramBinarySize[0];

  if (!fwrite(pucBinaryDataToWrite,
              BinaryDataToWriteSize * sizeof(unsigned char), 1, pIRFile)) {
    // Successful write, this was added due to a bug in gcc
  }
  fclose(pIRFile);

  delete[] pucBinaries[0];
  return CL_SUCCESS;
}

static int linkProgram() {
  discoverCPUType();
  int retcode = loadOpenCLContext();
  if (retcode) {
    exit(1);
  }

  std::list<std::string> binaries;
  cl_program_sptr progs;
  if (SPIRV.getNumOccurrences()) {
    // Handle one SPIRV input file
    DEBUG(dbgs() << "SPIR-V: " << SPIRV << '\n');
    binaries.push_back(SPIRV);
    program = getProgramFromSPIRV(SPIRV, context, device);
    if (!program)
      return CL_ERR_FAILURE;
    return getProgramBinaries();
  }

  // List of LLVM IR files
  // Split Binaries at ','s
  std::size_t cur, prev = 0;
  cur = Binaries.find_first_of(',');
  while (cur != std::string::npos) {
    binaries.push_back(Binaries.substr(prev, cur - prev));
    prev = cur + 1;
    cur = Binaries.find_first_of(',', prev);
  }
  binaries.push_back(Binaries.substr(prev, cur - prev));
  DEBUG(unsigned i = 0; for (auto it
                             : binaries) {
    dbgs() << "Binary[" << i++ << "]: " << it << '\n';
  });
  progs = getProgramsFromBinaries(binaries, context, device);
  if (!progs) {
    clReleaseContext(context);
    context = nullptr;
    DEBUG(dbgs() << "Failed to get program array from binary files\n");
    return CL_ERR_FAILURE;
  }
  cl_int clErr;
  program =
      clLinkProgram(context, 1, &device, BuildOptions.c_str(), binaries.size(),
                    progs.get(), nullptr, nullptr, &clErr);
  std::string build_log;
  GetBuildLog(program, device, build_log);
  std::cout << build_log;
  if (CL_FAILED(clErr)) {
    DEBUG(dbgs() << formatClError("Failed to link program...", clErr) << '\n');
    clReleaseContext(context);
    context = nullptr;
    return CL_ERR_FAILURE;
  }

  return getProgramBinaries();
}

static int buildProgram() {
  cl_int clErr = 0;
  discoverCPUType();
  int retcode = loadOpenCLContext();
  if (retcode) {
    exit(1);
  }

  if (Binaries.getNumOccurrences()) {
    // Use Binaries as input
    std::list<std::string> binaries;
    cl_program_sptr progs;
    std::cout << "Using build options: " << BuildOptions << '\n';
    binaries.push_back(Binaries);
    DEBUG(dbgs() << "Binary: " << Binaries << '\n');
    progs = getProgramsFromBinaries(binaries, context, device);
    if (!progs) {
      clReleaseContext(context);
      context = nullptr;
      DEBUG(dbgs() << "Failed to get program array from binary file\n");
      return CL_ERR_FAILURE;
    }
    clErr = clBuildProgram(*progs, 1, &device, BuildOptions.c_str(), nullptr,
                           nullptr);
  } else if ((Input.getNumOccurrences())) {
    // Use OpenCL source file as input
    // Add the parent path to the options as an include path.
    std::string options = BuildOptions;
    auto parent_dir = sys::path::parent_path(Input);
    if (!parent_dir.empty()) {
      options += " -I \"";
      options += parent_dir;
      options += "\"";
    }
    std::cout << "Using build options: " << options << '\n';
    DEBUG(dbgs() << "Reading input file: " << Input << '\n');
    sys::fs::file_status file_stat;
    auto err = sys::fs::status(Input, file_stat);
    if (err) {
      std::cout << "Unable to open input file '" << Input
                << "', error: " << err.message() << '\n';
      exit(1);
    }
    std::ifstream inputFile(Input.c_str());
    std::string code((std::istreambuf_iterator<char>(inputFile)),
                     (std::istreambuf_iterator<char>()));
    inputFile.close();

    if (trim(code).empty()) {
      std::cout << "File " << Input << " contains no code.\n";
      exit(1);
    }

    DEBUG(dbgs() << "Creating program...\n");
    const char *oclCode = code.c_str();

    program = clCreateProgramWithSource(context, 1, (const char **)&oclCode,
                                        nullptr, &clErr);
    if (CL_FAILED(clErr) || 0 == program) {
      DEBUG(if (CL_FAILED(clErr)) {
        dbgs() << "clCreateProgramWithSource failed: " << clErr << '\n';
      } else { dbgs() << "clCreateProgramWithSource return 0 program\n"; });
      clReleaseContext(context);
      context = 0;
      DEBUG(dbgs() << formatClError("Failed to create program from source...",
                                    clErr)
                   << '\n');
      return 1;
    }
    clErr = clBuildProgram(program, 1, &device, BuildOptions.c_str(), nullptr,
                           nullptr);
  }

  std::string build_log;
  GetBuildLog(program, device, build_log);
  std::cout << build_log;
  if (CL_FAILED(clErr)) {
    DEBUG(dbgs() << formatClError("Failed to build program...", clErr) << '\n');
    clReleaseContext(context);
    context = nullptr;
    return CL_ERR_FAILURE;
  }

  return getProgramBinaries();
}

// Clean up options from the rest of LLVM
static void fixOptions();

#if INTEL_PRODUCT_RELEASE
class HelpPrinter {
public:
  explicit HelpPrinter() = default;
  explicit HelpPrinter(StringMap<cl::Option *> OptMap_)
      : OptMap(std::move(OptMap_)) {}

  void print() {
    raw_ostream &OS = outs();
    OS << "OVERVIEW: This is the emulator compiler/linker\n";
    OS << "USAGE: aocl-ioc64 [options]"
       << "\n\n";
    OS << "OPTIONS:\n\n";

    size_t MaxArgLen = 0;
    for (const auto &Opt : OptMap) {
      MaxArgLen = std::max(MaxArgLen, Opt.getValue()->getOptionWidth());
    }

    for (const auto &Opt : OptMap) {
      if (!Opt.getValue()->getOptionHiddenFlag()) {
        Opt.getValue()->printOptionInfo(MaxArgLen);
      }
    }
  }

  void operator=(bool OptionWasSpecified) {
    if (!OptionWasSpecified)
      return;

    print();

    exit(0);
  }

private:
  StringMap<cl::Option *> OptMap;
};
#endif // INTEL_PRODUCT_RELEASE

// main - Entry point for the aocl-ioc64 command
int main(int argc, char **argv) {
#if INTEL_PRODUCT_RELEASE
  // --help option implementation is disabled in LLVM CommandLine
  // library for xmain release builds, adding custom --help implementation
  StringMap<cl::Option *> &OptionsMap = cl::getRegisteredOptions();
  HelpPrinter HelpPrinterInstance(OptionsMap);
  cl::opt<HelpPrinter, true, cl::parser<bool>> OptHelp(
      "help", cl::desc("Display available options"),
      cl::location(HelpPrinterInstance), cl::ValueDisallowed);
  cl::alias OptHelpA("h", cl::desc("Alias for --help"), cl::aliasopt(OptHelp));
#endif // INTEL_PRODUCT_RELEASE

  fixOptions();
  cl::ParseCommandLineOptions(argc, argv,
                              "This is the emulator compiler/linker");

  // Handle -version option
  if (Version) {
    std::cout << "Intel(R) SDK for OpenCL(TM) - offline compiler command line, "
                 "version 1.0\n";
    exit(0);
  }

  if (!Command.getNumOccurrences()) {
    std::cerr << "-cmd option must be specified\n";
    exit(1);
  }

  if (!Device.getNumOccurrences()) {
    std::cerr << "-device must be specified\n";
    exit(1);
  }

  int retcode = 0;
  if (Command == compile) {
    if (!Input.getNumOccurrences()) {
      std::cerr << "-cmd=compile needs -input option\n";
      exit(1);
    }
    if (!IR.getNumOccurrences()) {
      std::cerr << "-cmd=compile needs -ir option\n";
      exit(1);
    }
    retcode = compileProgram();
  } else if (Command == Commands::link) {
    if (!Binaries.getNumOccurrences() && !SPIRV.getNumOccurrences()) {
      std::cerr << "-cmd=link needs -binary or -spv option\n";
      exit(1);
    }
    // Ensure we don't have both
    if (Binaries.getNumOccurrences() && SPIRV.getNumOccurrences()) {
      std::cerr << "-cmd=link accepts only one of -binary or -spv option\n";
      exit(1);
    }
    if (!IR.getNumOccurrences()) {
      std::cerr << "-cmd=link needs -ir option\n";
      exit(1);
    }
    retcode = linkProgram();
  } else if (Command == build) {
    if (!Input.getNumOccurrences() && !Binaries.getNumOccurrences()) {
      std::cerr << "-cmd=build needs -input or -binary option\n";
      exit(1);
    }
    // Ensure we don't have both
    if (Input.getNumOccurrences() && Binaries.getNumOccurrences()) {
      std::cerr << "-cmd=build accepts only one of -binary or -input option\n";
      exit(1);
    }
    if (!IR.getNumOccurrences()) {
      std::cerr << "-cmd=build needs -ir option\n";
      exit(1);
    }
    retcode = buildProgram();
  }
  return retcode;
}

static void fixOptions() {
  // Clean out all options we don't care about
  StringMap<cl::Option *> &Map = cl::getRegisteredOptions();
  std::set<std::string> optionsToKeep, optionsToHide;
  optionsToKeep.insert("binary");
  optionsToKeep.insert("bo");
  optionsToKeep.insert("cmd");
  optionsToKeep.insert("debug-only");
  optionsToKeep.insert("help");
  optionsToKeep.insert("input");
  optionsToKeep.insert("ir");
  optionsToKeep.insert("spv");
  optionsToKeep.insert("xxdevice");
  optionsToKeep.insert("xxversion");

  for (auto &OptEntry : Map) {
    std::string ArgName = OptEntry.getKey().str();
    if (optionsToKeep.find(ArgName) == optionsToKeep.end()) {
      optionsToHide.insert(ArgName);
    }
  }

  // Now remove the ones we don't want.
  Map.erase("device");
  Map.erase("version");
  optionsToHide.erase("device");
  optionsToHide.erase("version");

  // And hide the rest
  for (auto it : optionsToHide) {
    Map[it]->setHiddenFlag(cl::ReallyHidden);
  }

  // Fix up descriptions.
  Map["help"]->setDescription("Print options");

  // Remap xxdevice to device
  Map["xxdevice"]->setArgStr("device");

  // Remap xxversion to version
  Map["xxversion"]->setArgStr("version");
}

int GetDeviceParam(cl_device_id clDeviceId, cl_device_info clDeviceInfo,
                   std::string &strParamValueRet) {
  size_t szStrSize = 0;
  char *pcValue = nullptr;
  strParamValueRet = "";

  cl_int clErr =
      clGetDeviceInfo(clDeviceId, clDeviceInfo, 0, nullptr, &szStrSize);
  if (CL_FAILED(clErr)) {
    return clErr;
  }
  pcValue = new char[szStrSize];
  if (nullptr == pcValue) {
    return CL_ERR_FAILURE;
  }
  clErr =
      clGetDeviceInfo(clDeviceId, clDeviceInfo, szStrSize, pcValue, nullptr);
  if (CL_FAILED(clErr)) {
    delete[] pcValue;
    return clErr;
  }
  strParamValueRet = std::string(pcValue);
  delete[] pcValue;
  return clErr;
}

int GetBuildLog(cl_program clProgram, cl_device_id clDeviceId,
                std::string &strBuildLog) {
  cl_int clErr = CL_SUCCESS;
  size_t szBuildLogLength = 0;
  clErr = clGetProgramBuildInfo(clProgram, clDeviceId, CL_PROGRAM_BUILD_LOG, 0,
                                nullptr, &szBuildLogLength);
  if (CL_FAILED(clErr)) {
    strBuildLog += "Failed to get program build info...";
    DEBUG(dbgs() << "Failed to get program build info...\n");
    return CL_ERR_FAILURE;
  }
  char *pBuildLog = new char[szBuildLogLength];
  if (nullptr == pBuildLog) {
    strBuildLog += "Memory allocation failure...";
    DEBUG(dbgs() << "Memory allocation failure...\n");
    return CL_ERR_FAILURE;
  }
  clErr = clGetProgramBuildInfo(clProgram, clDeviceId, CL_PROGRAM_BUILD_LOG,
                                szBuildLogLength, pBuildLog, nullptr);
  if (CL_FAILED(clErr)) {
    delete[] pBuildLog;
    strBuildLog += "Failed to get program build info...";
    DEBUG(dbgs() << "Failed to get program build info...\n");
    return CL_ERR_FAILURE;
  }
  // strBuildLog = ReplaceFileName(pBuildLog);
  strBuildLog = pBuildLog;
  return CL_SUCCESS;
}

const char *myClErrTxt(cl_int error_code) {
  switch (error_code) {
    // OpenCL error codes
  case (CL_SUCCESS):
    return "CL_SUCCESS";
  case (CL_DEVICE_NOT_FOUND):
    return "CL_DEVICE_NOT_FOUND";
  case (CL_DEVICE_NOT_AVAILABLE):
    return "CL_DEVICE_NOT_AVAILABLE";
  case (CL_COMPILER_NOT_AVAILABLE):
    return "CL_COMPILER_NOT_AVAILABLE";
  case (CL_MEM_OBJECT_ALLOCATION_FAILURE):
    return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
  case (CL_OUT_OF_RESOURCES):
    return "CL_OUT_OF_RESOURCES";
  case (CL_OUT_OF_HOST_MEMORY):
    return "CL_OUT_OF_HOST_MEMORY";
  case (CL_PROFILING_INFO_NOT_AVAILABLE):
    return "CL_PROFILING_INFO_NOT_AVAILABLE";
  case (CL_MEM_COPY_OVERLAP):
    return "CL_MEM_COPY_OVERLAP";
  case (CL_IMAGE_FORMAT_MISMATCH):
    return "CL_IMAGE_FORMAT_MISMATCH";
  case (CL_IMAGE_FORMAT_NOT_SUPPORTED):
    return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
  case (CL_BUILD_PROGRAM_FAILURE):
    return "CL_BUILD_PROGRAM_FAILURE";
  case (CL_MAP_FAILURE):
    return "CL_MAP_FAILURE";
  case (CL_MISALIGNED_SUB_BUFFER_OFFSET):
    return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
  case (CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST):
    return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
  case (CL_COMPILE_PROGRAM_FAILURE):
    return "CL_COMPILE_PROGRAM_FAILURE";
  case (CL_LINKER_NOT_AVAILABLE):
    return "CL_LINKER_NOT_AVAILABLE";
  case (CL_LINK_PROGRAM_FAILURE):
    return "CL_LINK_PROGRAM_FAILURE";
  case (CL_DEVICE_PARTITION_FAILED):
    return "CL_DEVICE_PARTITION_FAILED";
  case (CL_KERNEL_ARG_INFO_NOT_AVAILABLE):
    return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";
  case (CL_INVALID_VALUE):
    return "CL_INVALID_VALUE";
  case (CL_INVALID_DEVICE_TYPE):
    return "CL_INVALID_DEVICE_TYPE";
  case (CL_INVALID_PLATFORM):
    return "CL_INVALID_PLATFORM";
  case (CL_INVALID_DEVICE):
    return "CL_INVALID_DEVICE";
  case (CL_INVALID_CONTEXT):
    return "CL_INVALID_CONTEXT";
  case (CL_INVALID_QUEUE_PROPERTIES):
    return "CL_INVALID_QUEUE_PROPERTIES";
  case (CL_INVALID_COMMAND_QUEUE):
    return "CL_INVALID_COMMAND_QUEUE";
  case (CL_INVALID_HOST_PTR):
    return "CL_INVALID_HOST_PTR";
  case (CL_INVALID_MEM_OBJECT):
    return "CL_INVALID_MEM_OBJECT";
  case (CL_INVALID_IMAGE_FORMAT_DESCRIPTOR):
    return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
  case (CL_INVALID_IMAGE_SIZE):
    return "CL_INVALID_IMAGE_SIZE";
  case (CL_INVALID_SAMPLER):
    return "CL_INVALID_SAMPLER";
  case (CL_INVALID_BINARY):
    return "CL_INVALID_BINARY";
  case (CL_INVALID_BUILD_OPTIONS):
    return "CL_INVALID_BUILD_OPTIONS";
  case (CL_INVALID_PROGRAM):
    return "CL_INVALID_PROGRAM";
  case (CL_INVALID_PROGRAM_EXECUTABLE):
    return "CL_INVALID_PROGRAM_EXECUTABLE";
  case (CL_INVALID_KERNEL_NAME):
    return "CL_INVALID_KERNEL_NAME";
  case (CL_INVALID_KERNEL_DEFINITION):
    return "CL_INVALID_KERNEL_DEFINITION";
  case (CL_INVALID_KERNEL):
    return "CL_INVALID_KERNE";
  case (CL_INVALID_ARG_INDEX):
    return "CL_INVALID_ARG_INDEX";
  case (CL_INVALID_ARG_VALUE):
    return "CL_INVALID_ARG_VALUE";
  case (CL_INVALID_ARG_SIZE):
    return "CL_INVALID_ARG_SIZE";
  case (CL_INVALID_KERNEL_ARGS):
    return "CL_INVALID_KERNEL_ARGS";
  case (CL_INVALID_WORK_DIMENSION):
    return "CL_INVALID_WORK_DIMENSION";
  case (CL_INVALID_WORK_GROUP_SIZE):
    return "CL_INVALID_WORK_GROUP_SIZE";
  case (CL_INVALID_WORK_ITEM_SIZE):
    return "CL_INVALID_WORK_ITEM_SIZE";
  case (CL_INVALID_GLOBAL_OFFSET):
    return "CL_INVALID_GLOBAL_OFFSET";
  case (CL_INVALID_EVENT_WAIT_LIST):
    return "CL_INVALID_EVENT_WAIT_LIST";
  case (CL_INVALID_EVENT):
    return "CL_INVALID_EVENT";
  case (CL_INVALID_OPERATION):
    return "CL_INVALID_OPERATION";
  case (CL_INVALID_GL_OBJECT):
    return "CL_INVALID_GL_OBJECT";
  case (CL_INVALID_BUFFER_SIZE):
    return "CL_INVALID_BUFFER_SIZE";
  case (CL_INVALID_MIP_LEVEL):
    return "CL_INVALID_MIP_LEVE";
  case (CL_INVALID_PROPERTY):
    return "CL_INVALID_PROPERTY";
  case (CL_INVALID_IMAGE_DESCRIPTOR):
    return "CL_INVALID_IMAGE_DESCRIPTOR";
  case (CL_INVALID_COMPILER_OPTIONS):
    return "CL_INVALID_COMPILER_OPTIONS";
  case (CL_INVALID_LINKER_OPTIONS):
    return "CL_INVALID_LINKER_OPTIONS";
  case (CL_INVALID_DEVICE_PARTITION_COUNT):
    return "CL_INVALID_DEVICE_PARTITION_COUNT";
  default:
    return "Unknown Error Code";
  }
}

cl_program getProgramFromSPIRV(const std::string &spv, cl_context &con,
                               cl_device_id &device) {
  std::string strDeviceInfo;
  cl_int errcode_ret =
      GetDeviceParam(device, CL_DEVICE_EXTENSIONS, strDeviceInfo);
  DEBUG(dbgs() << "Extensions: " << strDeviceInfo << '\n');
  if (!CL_SUCCEEDED(errcode_ret)) {
    DEBUG(
        dbgs() << formatClError("CL_DEVICE_EXTENSIONS call failed", errcode_ret)
               << '\n');
    return nullptr;
  }

  if (strDeviceInfo.find("cl_khr_il_program") == std::string::npos) {
    DEBUG(dbgs() << "cl_khr_il_program extension is not available\n");
    return nullptr;
  }
  clCreateProgramWithILKHR_fn createProgFromSPV =
      reinterpret_cast<clCreateProgramWithILKHR_fn>(
          clGetExtensionFunctionAddressForPlatform(platform,
                                                   "clCreateProgramWithILKHR"));
  if (!createProgFromSPV) {
    DEBUG(dbgs() << "clGetExtensionFunctionAddress return nullptr\n");
    return nullptr;
  }

  std::ifstream irFile(spv.c_str(), std::ios::in | std::ios::binary);
  if (!irFile.is_open()) {
    DEBUG(dbgs() << "Failed to open SPIR-V file " << spv << '\n');
    return nullptr;
  }

  // get binary file data size and read its content to the buffer
  irFile.seekg(0, std::ios::end);
  size_t irDataSize = irFile.tellg();
  std::vector<unsigned char> irData(irDataSize);
  irFile.seekg(0, std::ios::beg);
  irFile.read((char *)irData.data(), irDataSize);
  irFile.close();

  const unsigned char *dataPtr = irData.data();
  cl_program prog =
      (*createProgFromSPV)(con, dataPtr, irDataSize, &errcode_ret);
  if (CL_FAILED(errcode_ret)) {
    std::string error = "Failed to create program from SPIR-V file " + spv;
    DEBUG(dbgs() << formatClError(error, errcode_ret) << '\n');
    return nullptr;
  }
  DEBUG(dbgs() << "Generated program from SPIR-V file " << spv << '\n');

  // Build the program.
  errcode_ret =
      clBuildProgram(prog, 1, &device, BuildOptions.c_str(), nullptr, nullptr);
  std::string build_log;
  GetBuildLog(prog, device, build_log);
  std::cout << build_log;
  if (CL_FAILED(errcode_ret)) {
    std::string error = "Failed to build program from SPIR-V file " + spv;
    DEBUG(dbgs() << formatClError(error, errcode_ret) << '\n');
    return nullptr;
  }
  DEBUG(dbgs() << "Built program from SPIR-V file " << spv << '\n');

  return prog;
}

cl_program_sptr getProgramsFromBinaries(const std::list<std::string> &lst,
                                        cl_context &con, cl_device_id &device) {
  size_t progNum = lst.size();
  cl_program *progArr = new cl_program[progNum];
  auto deleter = [progNum](cl_program *cl_progs) {
    if (cl_progs == nullptr)
      return;

    for (size_t i = 0; i < progNum; ++i) {
      if (cl_progs[i] != nullptr)
        clReleaseProgram(cl_progs[i]);
    }
    delete[] cl_progs;
    cl_progs = nullptr;
  };

  for (size_t i = 0; i < progNum; i++)
    progArr[i] = nullptr;

  cl_program_sptr guard(progArr, deleter);

  int pos = 0;
  for (auto iter : lst) {
    std::ifstream irFile(iter.c_str(), std::ios::in | std::ios::binary);
    if (irFile.is_open()) {
      // get binary file data size and read its content to the buffer
      irFile.seekg(0, std::ios::end);
      const int irDataSize = irFile.tellg();
      std::vector<unsigned char> irData(irDataSize);
      irFile.seekg(0, std::ios::beg);
      irFile.read((char *)irData.data(), irDataSize);
      irFile.close();

      size_t lengths = irDataSize;
      cl_int binary_status, errcode_ret;
      const unsigned char *dataPtr = irData.data();
      progArr[pos++] =
          clCreateProgramWithBinary(con, 1, &device, (const size_t *)&lengths,
                                    &dataPtr, &binary_status, &errcode_ret);
      if (CL_FAILED(binary_status) || CL_FAILED(errcode_ret)) {
        std::string error = "Failed to create program from IR file " + iter;
        DEBUG(dbgs() << formatClError(error, errcode_ret) << '\n');
        return nullptr;
      }
    } else {
      DEBUG(dbgs() << "Failed to open IR file " << iter << '\n');
      return nullptr;
    }
  }
  return guard;
}
