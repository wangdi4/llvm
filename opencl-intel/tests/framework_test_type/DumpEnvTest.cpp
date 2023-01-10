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
  virtual void SetUp() override {
    // Set different prefix for each test, in order to avoid race condition in
    // dumped filenames.
    const auto *testInfo =
        ::testing::UnitTest::GetInstance()->current_test_info();
    m_prefix = std::string(testInfo->test_suite_name()) + testInfo->name();
    ASSERT_TRUE(SETENV("CL_CONFIG_DUMP_FILE_NAME_PREFIX", m_prefix.c_str()));
  }

  virtual void TearDown() override {
    cl_int Err;
    if (m_program) {
      Err = clReleaseProgram(m_program);
      EXPECT_OCL_SUCCESS(Err, "clReleaseProgram");
    }
    if (m_program1) {
      Err = clReleaseProgram(m_program1);
      EXPECT_OCL_SUCCESS(Err, "clReleaseProgram");
    }

    if (m_context) {
      Err = clReleaseContext(m_context);
      EXPECT_OCL_SUCCESS(Err, "clReleaseContext");
    }

    for (auto &e : m_env)
      ASSERT_TRUE(UNSETENV(e.first.c_str()))
          << ("Failed to unset env " + e.first);

    // Delete dumped files
    for (auto &filename : m_dumpFilenames)
      (void)std::remove(filename.c_str());

    ASSERT_TRUE(UNSETENV("CL_CONFIG_DUMP_FILE_NAME_PREFIX"));
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
    ASSERT_NO_FATAL_FAILURE(createProgramHelper(m_kernelName, m_program));
  }

  void createSecondProgram() {
    ASSERT_NO_FATAL_FAILURE(createProgramHelper(m_kernelName1, m_program1));
  }

  void
  validateDumpedFiles(const std::vector<std::string> &Suffix,
                      const std::vector<std::vector<std::string>> &Patterns,
                      unsigned ProgramIndex = 1) {
    size_t NumFiles = Suffix.size();
    size_t NumPatterns = Patterns.size();
    ASSERT_TRUE(NumPatterns == 0 || NumPatterns == NumFiles);
    for (size_t I = 0; I < NumFiles; ++I) {
      std::string FilenamePattern = m_prefix + "_" +
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

  void testBody(const std::vector<std::string> &suffix,
                const std::vector<std::vector<std::string>> &patterns) {
    for (auto &e : m_env)
      ASSERT_TRUE(SETENV(e.first.c_str(), e.second.c_str()))
          << ("Failed to set env " + e.first);

    ASSERT_NO_FATAL_FAILURE(createContext());
    ASSERT_NO_FATAL_FAILURE(createFirstProgram());

    cl_int err =
        clBuildProgram(m_program, 1, &m_device, nullptr, nullptr, nullptr);
    ASSERT_OCL_SUCCESS(err, "clBuildProgram");

    ASSERT_NO_FATAL_FAILURE(validateDumpedFiles(suffix, patterns));
  }

protected:
  cl_platform_id m_platform = nullptr;
  cl_device_id m_device = nullptr;
  cl_context m_context = nullptr;
  cl_program m_program = nullptr;
  cl_program m_program1 = nullptr;
  std::string m_kernelName = "first_kernel";
  std::string m_kernelName1 = "second_kernel";
  std::string m_prefix = "";
  std::vector<std::string> m_dumpFilenames;
  std::vector<std::pair<std::string, std::string>> m_env;
};

TEST_F(DumpEnvTest, IRBeforeOptimizer) {
  m_env = {{"CL_CONFIG_DUMP_IR_BEFORE_OPTIMIZER", "True"}};
  std::vector<std::string> suffix = {"_eq.ll"};
  std::vector<std::vector<std::string>> patterns = {
      {m_kernelName, "spir_kernel"}};
  ASSERT_NO_FATAL_FAILURE(testBody(suffix, patterns));
}

TEST_F(DumpEnvTest, IRAfterOptimizer) {
  m_env = {{"CL_CONFIG_DUMP_IR_AFTER_OPTIMIZER", "True"}};
  std::vector<std::string> suffix = {".ll"};
  std::vector<std::vector<std::string>> patterns = {
      {m_kernelName, "!sycl.kernels"}};
  ASSERT_NO_FATAL_FAILURE(testBody(suffix, patterns));
}

TEST_F(DumpEnvTest, Asm) {
  // Don't put CL_CONFIG_DUMP_ASM into m_env, to avoid asm being dumped twice.
  ASSERT_TRUE(SETENV("CL_CONFIG_DUMP_ASM", "True"));
  std::vector<std::string> suffix = {".s"};
  std::vector<std::vector<std::string>> patterns = {{m_kernelName, "globl"}};
  ASSERT_NO_FATAL_FAILURE(testBody(suffix, patterns));
  ASSERT_TRUE(UNSETENV("CL_CONFIG_DUMP_ASM"));
  cl_int err = clReleaseProgram(m_program);
  EXPECT_OCL_SUCCESS(err, "clReleaseProgram");
  m_program = nullptr;

  // Test loading asm and compiling asm to object.
  ASSERT_EQ(m_dumpFilenames.size(), 1);
  std::string asmFilename = m_dumpFilenames[0];
  ASSERT_TRUE(SETENV("CL_CONFIG_REPLACE_ASM", asmFilename.c_str()));
  ASSERT_NO_FATAL_FAILURE(createFirstProgram());
  err = clBuildProgram(m_program, 1, &m_device, nullptr, nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clBuildProgram");
  std::string log;
  ASSERT_NO_FATAL_FAILURE(GetBuildLog(m_device, m_program, log));
  ASSERT_NE(
      log.find("WARNING: replace device kernel assembly with " + asmFilename),
      std::string::npos);
  ASSERT_TRUE(UNSETENV("CL_CONFIG_REPLACE_ASM"));
  err = clReleaseProgram(m_program);
  EXPECT_OCL_SUCCESS(err, "clReleaseProgram");
  m_program = nullptr;

#ifndef _WIN32
  // Testing loading object file.
  std::string objFilename = asmFilename + ".o";
  std::string compileAsmToObj =
      (Twine(CXX_COMPILER) + Twine(" -c ") + Twine(asmFilename) +
       Twine(" -o ") + Twine(objFilename))
          .str();
  ASSERT_EQ(system(compileAsmToObj.c_str()), 0);
  m_dumpFilenames.push_back(objFilename);
  ASSERT_TRUE(SETENV("CL_CONFIG_REPLACE_OBJ", objFilename.c_str()));
  ASSERT_NO_FATAL_FAILURE(createFirstProgram());
  err = clBuildProgram(m_program, 1, &m_device, nullptr, nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clBuildProgram");
  ASSERT_NO_FATAL_FAILURE(GetBuildLog(m_device, m_program, log));
  ASSERT_NE(
      log.find("WARNING: replace device kernel object with " + objFilename),
      std::string::npos);
  ASSERT_TRUE(UNSETENV("CL_CONFIG_REPLACE_OBJ"));
#endif
}

TEST_F(DumpEnvTest, Disassembly) {
  m_env = {{"CL_CONFIG_DUMP_DISASSEMBLY", "True"}};
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
  m_env = {{"CL_CONFIG_DUMP_IR_BEFORE_OPTIMIZER", "True"},
           {"CL_CONFIG_DUMP_IR_AFTER_OPTIMIZER", "True"},
           {"CL_CONFIG_DUMP_ASM", "True"},
           {"CL_CONFIG_DUMP_DISASSEMBLY", "True"},
           {"CL_CONFIG_DUMP_BIN", "True"}};
  std::vector<std::string> suffix = {"_eq.ll", ".ll", ".s", ".asm", ".bin"};
  std::vector<std::vector<std::string>> patterns;
  ASSERT_NO_FATAL_FAILURE(testBody(suffix, patterns));

  // Check hashes are the same.
  ASSERT_EQ(suffix.size(), m_dumpFilenames.size());
  std::string hash;
  for (auto &filename : m_dumpFilenames) {
    std::string hashCurr = filename.substr(m_prefix.length() + 3, 16);
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
      std::string filenamePattern = m_prefix + "_2_[0-9a-f]{16}" + s;
      Regex r(filenamePattern);
      std::vector<std::string> filenames = findFilesInDir(".", r);
      ASSERT_TRUE(!filenames.empty())
          << ("AOT " + filenamePattern + " is not dumped");
      bool found = llvm::any_of(filenames, [&](auto &filename) {
        return hash == filename.substr(m_prefix.length() + 3, 16);
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
  ASSERT_TRUE(SETENV("CL_CONFIG_DUMP_IR_BEFORE_OPTIMIZER", "True"));
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
      validateDumpedFiles(Suffix, Patterns, /*ProgramIndex*/ 1));

  // Check whether IR is dumped for an independent program built after a
  // previous clCreateKernel call (where all llvm cl options are reset).
  ASSERT_NO_FATAL_FAILURE(createSecondProgram());
  Err = clBuildProgram(m_program1, 1, &m_device, "-g", nullptr, nullptr);
  ASSERT_OCL_SUCCESS(Err, "clBuildProgram");
  Patterns.clear();
  Patterns.push_back({m_kernelName1, "spir_kernel"});
  ASSERT_NO_FATAL_FAILURE(
      validateDumpedFiles(Suffix, Patterns, /*ProgramIndex*/ 2));

  ASSERT_TRUE(UNSETENV("CL_CONFIG_DUMP_IR_BEFORE_OPTIMIZER"));
}
