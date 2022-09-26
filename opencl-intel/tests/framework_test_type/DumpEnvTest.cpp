// Copyright 2021 Intel Corporation.
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

#include "TestsHelpClasses.h"
#include "common_utils.h"

extern cl_device_type gDeviceType;

class DumpEnvTest : public ::testing::Test {
protected:
  virtual void TearDown() override {
    cl_int Err = clReleaseProgram(m_program);
    EXPECT_OCL_SUCCESS(Err, "clReleaseProgram");

    if (!m_kernelName1.empty()) {
      Err = clReleaseProgram(m_program1);
      EXPECT_OCL_SUCCESS(Err, "clReleaseProgram");
    }

    Err = clReleaseContext(m_context);
    EXPECT_OCL_SUCCESS(Err, "clReleaseContext");

    for (auto &e : m_env)
      ASSERT_TRUE(UNSETENV(e.first.c_str()))
          << ("Failed to unset env " + e.first);

    // Delete dumped files
    for (auto &filename : m_dumpFilenames)
      (void)std::remove(filename.c_str());
  }

  void createContext() {
    cl_int err = clGetPlatformIDs(1, &m_platform, nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetPlatformIDs");

    err = clGetDeviceIDs(m_platform, gDeviceType, 1, &m_device, nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceIDs");

    m_context = clCreateContext(nullptr, 1, &m_device, nullptr, nullptr, &err);
    ASSERT_OCL_SUCCESS(err, "clCreateContext");
  }

  void createProgramHelper(const std::string &KernelName, cl_program &Program) {
    std::string Source = "kernel void " + KernelName +
                         "(global int* src, global int* dst) {"
                         "  size_t gid = get_global_id(0);"
                         "  dst[gid] = src[gid];"
                         "}";
    const char *Csource = Source.c_str();
    cl_int Err;
    Program = clCreateProgramWithSource(m_context, 1, &Csource, nullptr, &Err);
    ASSERT_OCL_SUCCESS(Err, "clCreateProgramWithSource");
  }

  void createFirstProgram() {
    m_kernelName = "first_kernel";
    createProgramHelper(m_kernelName, m_program);
  }

  void createSecondProgram() {
    m_kernelName1 = "second_kernel";
    createProgramHelper(m_kernelName1, m_program1);
  }

  void testBody(const std::vector<std::string> &suffix,
                const std::vector<std::vector<std::string>> &patterns) {
    std::string prefix("framework_test_type");
    testBody(prefix, suffix, patterns);
  }

  void
  validateDumpedFiles(const std::string &Prefix,
                      const std::vector<std::string> &Suffix,
                      const std::vector<std::vector<std::string>> &Patterns,
                      unsigned ProgramIndex = 1) {
    size_t NumFiles = Suffix.size();
    size_t NumPatterns = Patterns.size();
    ASSERT_TRUE(NumPatterns == 0 || NumPatterns == NumFiles);
    for (size_t I = 0; I < NumFiles; ++I) {
      std::string FilenamePattern = Prefix + "_" +
                                    std::to_string(ProgramIndex) +
                                    "_[0-9a-f]{16}" + Suffix[I];
      Regex R(FilenamePattern);
      std::vector<std::string> DumpFilenames = findFilesInDir(".", R);
      ASSERT_TRUE(!DumpFilenames.empty())
          << (FilenamePattern + " is not dumped");
      if (NumPatterns != 0) {
        bool contains = llvm::any_of(DumpFilenames, [&](auto &Filename) {
          return fileContains(Filename, Patterns[I]);
        });
        ASSERT_TRUE(contains) << "Pattern \"" << llvm::join(Patterns[I], ";")
                              << "\" is not found in files "
                              << llvm::join(DumpFilenames, ";") << "\n";
        ;
      }
      m_dumpFilenames.insert(m_dumpFilenames.end(), DumpFilenames.begin(),
                             DumpFilenames.end());
    }
  }

  void testBody(const std::string &prefix,
                const std::vector<std::string> &suffix,
                const std::vector<std::vector<std::string>> &patterns) {
    for (auto &e : m_env)
      ASSERT_TRUE(SETENV(e.first.c_str(), e.second.c_str()))
          << ("Failed to set env " + e.first);

    ASSERT_NO_FATAL_FAILURE(createContext());
    ASSERT_NO_FATAL_FAILURE(createFirstProgram());

    cl_int err =
        clBuildProgram(m_program, 1, &m_device, nullptr, nullptr, nullptr);
    ASSERT_OCL_SUCCESS(err, "clBuildProgram");

    ASSERT_NO_FATAL_FAILURE(validateDumpedFiles(prefix, suffix, patterns));
  }

protected:
  cl_platform_id m_platform;
  cl_device_id m_device;
  cl_context m_context;
  cl_program m_program;
  cl_program m_program1;
  std::string m_kernelName;
  std::string m_kernelName1;
  std::vector<std::string> m_dumpFilenames;
  std::vector<std::pair<std::string, std::string>> m_env;
};

TEST_F(DumpEnvTest, StatsEqualizerAll) {
  m_env = {{"VOLCANO_EQUALIZER_STATS", "All"}};
  std::vector<std::string> suffix = {"_eq.ll"};
  std::vector<std::vector<std::string>> patterns = {
      {m_kernelName, "spir_kernel"}};
  ASSERT_NO_FATAL_FAILURE(testBody(suffix, patterns));
}

TEST_F(DumpEnvTest, StatsAll) {
  m_env = {{"VOLCANO_STATS", "All"}};
  std::vector<std::string> suffix = {".ll"};
  std::vector<std::vector<std::string>> patterns = {
      {m_kernelName, "!sycl.kernels"}};
  ASSERT_NO_FATAL_FAILURE(testBody(suffix, patterns));
}

TEST_F(DumpEnvTest, OptIRAsm) {
  ASSERT_NO_FATAL_FAILURE(createContext());
  ASSERT_NO_FATAL_FAILURE(createFirstProgram());

  std::string dir = get_exe_dir();
  std::string asmFile = dir + "dump_opt.asm";
  std::string irFile = dir + "dump_opt.ll";
  // Build program
  std::string options =
      "-dump-opt-asm=\"" + asmFile + "\" -dump-opt-llvm=\"" + irFile + "\"";

  cl_int err = clBuildProgram(m_program, 1, &m_device, options.c_str(), nullptr,
                              nullptr);
  ASSERT_OCL_SUCCESS(err, "clBuildProgram");

  // Check dumped files contain kernel name and section
  std::vector<std::string> patterns = {m_kernelName};
  ASSERT_NO_FATAL_FAILURE(checkFileContains(irFile, patterns));
  m_dumpFilenames.push_back(irFile);

  patterns.push_back("Disassembly of section");
  ASSERT_NO_FATAL_FAILURE(checkFileContains(asmFile, patterns));
  m_dumpFilenames.push_back(asmFile);
}

TEST_F(DumpEnvTest, OptIRAsmDebug) {
  ASSERT_NO_FATAL_FAILURE(createContext());
  ASSERT_NO_FATAL_FAILURE(createFirstProgram());

  std::string dir = get_exe_dir();
  std::string asmFile = dir + "dump_opt_debug.asm";
  std::string irFile = dir + "dump_opt_debug.ll";

  // Build program
  std::string options =
      "-dump-opt-asm=\"" + asmFile + "\" -dump-opt-llvm=\"" + irFile + "\" -g";

  cl_int err = clBuildProgram(m_program, 1, &m_device, options.c_str(), nullptr,
                              nullptr);
  ASSERT_OCL_SUCCESS(err, "clBuildProgram");

  // Check dumped files contain kernel name and section
  std::vector<std::string> patterns = {m_kernelName};
  ASSERT_NO_FATAL_FAILURE(checkFileContains(irFile, patterns));
  m_dumpFilenames.push_back(irFile);

  patterns.push_back("Disassembly of section .debug");
  ASSERT_NO_FATAL_FAILURE(checkFileContains(asmFile, patterns));
  m_dumpFilenames.push_back(asmFile);
}

TEST_F(DumpEnvTest, Asm) {
  m_env = {{"CL_CONFIG_DUMP_ASM", "True"}};
  std::vector<std::string> suffix = {".asm"};
  std::vector<std::vector<std::string>> patterns = {
      {m_kernelName, "Disassembly of section"}};
  ASSERT_NO_FATAL_FAILURE(testBody(suffix, patterns));
}

TEST_F(DumpEnvTest, Bin) {
  m_env = {{"CL_CONFIG_DUMP_BIN", "True"}};
  std::vector<std::string> suffix = {".bin"};
  std::vector<std::vector<std::string>> patterns = {
      {m_kernelName, {0x7f, 'E', 'L', 'F'}}};
  ASSERT_NO_FATAL_FAILURE(testBody(suffix, patterns));
}

/// Check that all dumped filenames contains the same hash.
TEST_F(DumpEnvTest, CheckHashSame) {
  std::string prefix = "TMP";
  m_env = {{"CL_CONFIG_DUMP_FILE_NAME_PREFIX", prefix},
           {"VOLCANO_EQUALIZER_STATS", "All"},
           {"VOLCANO_STATS", "All"},
           {"CL_CONFIG_DUMP_ASM", "True"},
           {"CL_CONFIG_DUMP_BIN", "True"}};
  std::vector<std::string> suffix = {"_eq.ll", ".ll", ".asm", ".bin"};
  std::vector<std::vector<std::string>> patterns;
  ASSERT_NO_FATAL_FAILURE(testBody(prefix, suffix, patterns));

  // Check hashes are the same.
  ASSERT_EQ(suffix.size(), m_dumpFilenames.size());
  std::string hash;
  for (auto &filename : m_dumpFilenames) {
    std::string hashCurr = filename.substr(prefix.length() + 3, 16);
    if (hash.empty()) {
      hash = hashCurr;
      continue;
    }
    ASSERT_EQ(hash, hashCurr)
        << (filename + " has different hash than " + hash);
  }

  // Check that asm and bin files are dumped in AOT and hash is the same as JIT.
  auto checkAOT = [&]() {
    size_t binarySize;
    cl_int err = clGetProgramInfo(m_program, CL_PROGRAM_BINARY_SIZES,
                                  sizeof(binarySize), &binarySize, nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetProgramInfo CL_PROGRAM_BINARY_SIZES");
    std::vector<unsigned char> binary(binarySize);
    unsigned char *binaries[1] = {&binary[0]};
    err = clGetProgramInfo(m_program, CL_PROGRAM_BINARIES, sizeof(binaries),
                           &binaries, nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetProgramInfo CL_PROGRAM_BINARIES");

    cl_int binaryStatus;
    cl_program programAOT = clCreateProgramWithBinary(
        m_context, 1, &m_device, &binarySize,
        const_cast<const unsigned char **>(binaries), &binaryStatus, &err);
    ASSERT_OCL_SUCCESS(binaryStatus, "clCreateProgramWithBinary binaryStatus");
    ASSERT_OCL_SUCCESS(err, "clCreateProgramWithBinary");

    err = clBuildProgram(programAOT, 1, &m_device, nullptr, nullptr, nullptr);
    ASSERT_OCL_SUCCESS(err, "clBuildProgram");
    err = clReleaseProgram(programAOT);
    ASSERT_OCL_SUCCESS(err, "clReleaseProgram");

    suffix = {".asm", ".bin"};
    for (auto &s : suffix) {
      std::string filenamePattern = prefix + "_2_[0-9a-f]{16}" + s;
      Regex r(filenamePattern);
      std::vector<std::string> filenames = findFilesInDir(".", r);
      ASSERT_TRUE(!filenames.empty())
          << ("AOT " + filenamePattern + " is not dumped");
      bool found = llvm::any_of(filenames, [&](auto &filename) {
        return hash == filename.substr(prefix.length() + 3, 16);
      });
      ASSERT_TRUE(found) << ("AOT " + filenamePattern + " with hash " + hash +
                             " is not dumped");
      m_dumpFilenames.insert(m_dumpFilenames.end(), filenames.begin(),
                             filenames.end());
    }
  };
  checkAOT();
}

/// Check that IR can be dumped for an independent program which is built after
/// a previous clCreateKernel call.
TEST_F(DumpEnvTest, DumpAfterCreateKernel) {
  ASSERT_TRUE(SETENV("VOLCANO_EQUALIZER_STATS", "all"));
  // There is race condition between StatsEqualizerAll and DumpAfterCreateKernel
  // if CL_CONFIG_DUMP_FILE_NAME_PREFIX is not set, supposing the following
  // steps happen:
  //   1. Both tests dump it _eq.ll files. So there are two _eq.ll files.
  //   2. Both tests find the first _eq.ll file.
  //   3. Test DumpAfterCreateKernel finishes and the _eq.ll file is deleted.
  //   4. Test StatsEqualizerAll fails to read the _eq.ll file and test fails.
  std::string Prefix = "TMP2";
  ASSERT_TRUE(SETENV("CL_CONFIG_DUMP_FILE_NAME_PREFIX", Prefix.c_str()));
  ASSERT_NO_FATAL_FAILURE(createContext());
  ASSERT_NO_FATAL_FAILURE(createFirstProgram());

  // "-g" option guarantees that LLDJIT is used on windows.
  cl_int Err = clBuildProgram(m_program, 1, &m_device, "-g", nullptr, nullptr);
  ASSERT_OCL_SUCCESS(Err, "clBuildProgram");
  // clCreateKernel --> Program::Finalize() --> LLDJIT::finalizeObject() -->
  // lld::coff::link() --> cl::ResetAllOptionOccurrences()
  cl_kernel Ker = clCreateKernel(m_program, m_kernelName.c_str(), &Err);
  ASSERT_OCL_SUCCESS(Err, "clCreateKernel");
  Err = clReleaseKernel(Ker);
  ASSERT_OCL_SUCCESS(Err, "clReleaseKernel");

  std::vector<std::string> Suffix = {"_eq.ll"};
  std::vector<std::vector<std::string>> Patterns = {
      {m_kernelName, "spir_kernel"}};
  ASSERT_NO_FATAL_FAILURE(
      validateDumpedFiles(Prefix, Suffix, Patterns, /*ProgramIndex*/ 1));

  // Check whether IR is dumped for an independent program built after a
  // previous clCreateKernel call (where all llvm cl options are reset).
  ASSERT_NO_FATAL_FAILURE(createSecondProgram());
  Err = clBuildProgram(m_program1, 1, &m_device, "-g", nullptr, nullptr);
  ASSERT_OCL_SUCCESS(Err, "clBuildProgram");
  Patterns.clear();
  Patterns.push_back({m_kernelName1, "spir_kernel"});
  ASSERT_NO_FATAL_FAILURE(
      validateDumpedFiles(Prefix, Suffix, Patterns, /*ProgramIndex*/ 2));

  ASSERT_TRUE(UNSETENV("VOLCANO_EQUALIZER_STATS"));
  ASSERT_TRUE(UNSETENV("CL_CONFIG_DUMP_FILE_NAME_PREFIX"));
}
