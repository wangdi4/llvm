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
    cl_int err = clReleaseProgram(m_program);
    EXPECT_OCL_SUCCESS(err, "clReleaseProgram");

    err = clReleaseContext(m_context);
    EXPECT_OCL_SUCCESS(err, "clReleaseContext");

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

  void createProgram() {
    m_kernelName = "some_kernel_test";
    std::string source = "kernel void " + m_kernelName +
                         "(global int* src, global int* dst) {"
                         "  size_t gid = get_global_id(0);"
                         "  dst[gid] = src[gid];"
                         "}";
    const char *csource = source.c_str();
    cl_int err;
    m_program =
        clCreateProgramWithSource(m_context, 1, &csource, nullptr, &err);
    ASSERT_OCL_SUCCESS(err, "clCreateProgramWithSource");
  }

  void testBody(const std::vector<std::string> &suffix,
                const std::vector<std::vector<std::string>> &patterns) {
    std::string prefix("framework_test_type");
    testBody(prefix, suffix, patterns);
  }

  void testBody(const std::string &prefix,
                const std::vector<std::string> &suffix,
                const std::vector<std::vector<std::string>> &patterns) {
    for (auto &e : m_env)
      ASSERT_TRUE(SETENV(e.first.c_str(), e.second.c_str()))
          << ("Failed to set env " + e.first);

    ASSERT_NO_FATAL_FAILURE(createContext());
    ASSERT_NO_FATAL_FAILURE(createProgram());

    cl_int err =
        clBuildProgram(m_program, 1, &m_device, nullptr, nullptr, nullptr);
    ASSERT_OCL_SUCCESS(err, "clBuildProgram");

    size_t numFiles = suffix.size();
    size_t numPatterns = patterns.size();
    ASSERT_TRUE(numPatterns == 0 || numPatterns == numFiles);
    for (size_t i = 0; i < numFiles; ++i) {
      Regex r(prefix + "_1_[0-9a-f]{16}" + suffix[i]);
      std::string dumpFilename = findFileInDir(".", r);
      ASSERT_TRUE(!dumpFilename.empty()) << (suffix[i] + " is not dumped");
      if (numPatterns != 0)
        ASSERT_TRUE(fileContains(dumpFilename, patterns[i]));
      m_dumpFilenames.push_back(dumpFilename);
    }
  }

protected:
  cl_platform_id m_platform;
  cl_device_id m_device;
  cl_context m_context;
  cl_program m_program;
  std::string m_kernelName;
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
  ASSERT_NO_FATAL_FAILURE(createProgram());

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
  ASSERT_TRUE(fileContains(irFile, patterns));
  m_dumpFilenames.push_back(irFile);

  patterns.push_back("Disassembly of section");
  ASSERT_TRUE(fileContains(asmFile, patterns));
  m_dumpFilenames.push_back(asmFile);
}

TEST_F(DumpEnvTest, OptIRAsmDebug) {
  ASSERT_NO_FATAL_FAILURE(createContext());
  ASSERT_NO_FATAL_FAILURE(createProgram());

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
  ASSERT_TRUE(fileContains(irFile, patterns));
  m_dumpFilenames.push_back(irFile);

  patterns.push_back("Disassembly of section .debug");
  ASSERT_TRUE(fileContains(asmFile, patterns));
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
      Regex r(prefix + "_2_[0-9a-f]{16}" + s);
      std::string filename = findFileInDir(".", r);
      ASSERT_TRUE(!filename.empty()) << ("AOT " + filename + " is not dumped");
      std::string asmHash = filename.substr(prefix.length() + 3, 16);
      ASSERT_EQ(hash, asmHash)
          << ("AOT dump " + filename + " has different hash than " + hash);
      m_dumpFilenames.push_back(filename);
    }
  };
  checkAOT();
}
