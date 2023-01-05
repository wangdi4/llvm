#include "FrameworkTest.h"
#include "TestsHelpClasses.h"
#include "cl_cpu_detect.h"
#include "cl_types.h"
#include "common_utils.h"
#include "llvm/Support/Path.h"
#include <fstream>
#include <stdio.h>

extern cl_device_type gDeviceType;

class CheckBinaryVersionSuit : public ::testing::Test {
protected:
  cl_platform_id m_platform;
  cl_device_id m_device;
  cl_context m_context;
  cl_command_queue m_queue;
  // These binaries use same kernel source code as GenerateUpToDateBinaryFile().
  // Steps to generate these binaries are also similar with ones in
  // GenerateUpToDateBinaryFile() but different openCL RT.
  std::vector<std::string> m_CompatibilityBinaryFileName = {
#if defined(__linux__)
    "cached_binary_gold_release_linux.bin",
    "cached_binary_gold_update1_release_linux.bin",
    "cached_binary_gold_update2_release_linux.bin",
#elif defined(_WIN32) && !defined(_WIN64)
    "cached_binary_gold_release_windows32.bin",
    "cached_binary_gold_update1_release_windows32.bin",
    "cached_binary_gold_update2_release_windows32.bin"
#elif defined(_WIN64)
    "cached_binary_gold_release_windows.bin",
    "cached_binary_gold_update1_release_windows.bin",
    "cached_binary_gold_update2_release_windows.bin"
#endif
  };

  std::string m_CachedBinaryWithoutSectionFileName =
      "cached_binary_without_section.bin";
  std::string m_CachedBinaryWithWrongVersionFileName =
      "cached_binary_with_wrong_version.bin";
  std::string m_CachedBinaryWithCurrentVersionFileName =
      "cached_binary_current_version.bin";
  std::string m_CachedBinaryUpToDateVersionFileName =
      "cached_binary_up_to_date.bin";

  void ReadBinary(std::string filename, std::vector<unsigned char> &file) {
    std::ifstream binaryFile(get_exe_dir() + filename,
                             std::fstream::binary | std::fstream::in);
    ASSERT_TRUE(binaryFile.is_open()) << "Unable to open file";

    std::copy(std::istreambuf_iterator<char>(binaryFile),
              std::istreambuf_iterator<char>(), std::back_inserter(file));
    binaryFile.close();
  }

  void GenerateUpToDateBinaryFile() {
    const char *testProgram[] = {"__kernel void test(__global int* A) {"
                                 "   int lid = get_local_id(0);"
                                 "   if (0 == lid)"
                                 "     A[get_group_id(0)] = get_local_size(0);"
                                 "}"};

    cl_program clProg;
    bool bResult = BuildProgramSynch(m_context, 1, (const char **)&testProgram,
                                     NULL, "", &clProg);
    ASSERT_TRUE(bResult) << "Unable to build up-to-date binary";

    size_t binarySize = 0;
    char *pBinaries = NULL;

    // get the binary
    cl_int rc = clGetProgramInfo(clProg, CL_PROGRAM_BINARY_SIZES,
                                 sizeof(size_t), &binarySize, NULL);
    ASSERT_OCL_SUCCESS(rc, "clGetProgramInfo(CL_PROGRAM_BINARY_SIZES)");

    // cache the binary
    pBinaries = new char[binarySize];
    rc = clGetProgramInfo(clProg, CL_PROGRAM_BINARIES, binarySize, &pBinaries,
                          NULL);
    ASSERT_OCL_SUCCESS(rc, "clGetProgramInfo(CL_PROGRAM_BINARIES)");

    FILE *fout;
    std::string filename =
        get_exe_dir() + m_CachedBinaryUpToDateVersionFileName;
    fout = fopen(filename.c_str(), "wb");
    ASSERT_FALSE(fout == NULL) << "Failed to open file.\n";

    fwrite(pBinaries, 1, binarySize, fout);
    ASSERT_EQ(ferror(fout), 0) << "Error in writing to file " << filename;
    ASSERT_EQ(fclose(fout), 0) << "Failed to close file " << filename;
  }

  void TestProgram(cl_program program, const std::string &errMsg) {
    cl_int rc;
    cl_kernel kernel = clCreateKernel(program, "test", &rc);
    ASSERT_EQ(CL_SUCCESS, rc) << "clCreateKernel failed." << errMsg;

    size_t gdim = 8;
    size_t ldim = 2;
    std::vector<int> results(gdim / ldim);
    rc = clSetKernelArgMemPointerINTEL(kernel, 0, &results[0]);
    ASSERT_EQ(CL_SUCCESS, rc)
        << "clSetKernelArgMemPointerINTEL failed." << errMsg;

    rc = clEnqueueNDRangeKernel(m_queue, kernel, 1, nullptr, &gdim, &ldim, 0,
                                nullptr, nullptr);
    ASSERT_EQ(CL_SUCCESS, rc) << "clEnqueueNDRangeKernel failed." << errMsg;
    rc = clFinish(m_queue);
    for (size_t i = 0; i < results.size(); ++i)
      ASSERT_EQ(results[i], ldim)
          << "results[" << i << "] validation failed." << errMsg;

    rc = clReleaseKernel(kernel);
    ASSERT_OCL_SUCCESS(rc, "clReleaseKernel");
  }

  void LoadBinaryAndRun(const std::string &cachedBinaryFileName) {
    std::vector<unsigned char> binary;
    ReadBinary(cachedBinaryFileName, binary);
    ASSERT_NE(binary.size(), 0) << "Unable to read binary";
    std::size_t binarySize;
    binarySize = binary.size();
    unsigned char *p_binary = binary.data();

    cl_int binaryStatus;
    cl_int rc;
    cl_program program = clCreateProgramWithBinary(
        m_context, 1, &m_device, &binarySize,
        const_cast<const unsigned char **>(&p_binary), &binaryStatus, &rc);
    ASSERT_OCL_SUCCESS(binaryStatus, "clCreateProgramWithBinary failed");
    ASSERT_OCL_SUCCESS(rc, "clCreateProgramWithBinary");

    rc = clBuildProgram(program, 1, &m_device, "", nullptr, nullptr);
    ASSERT_OCL_SUCCESS(rc, "clBuildProgram");

    ASSERT_NO_FATAL_FAILURE(TestProgram(program, ""));

    rc = clReleaseProgram(program);
    ASSERT_OCL_SUCCESS(rc, "clReleaseProgram");
  }

  void SetUp() override {
    cl_int rc = clGetPlatformIDs(1, &m_platform, NULL);
    ASSERT_OCL_SUCCESS(rc, "Unable to get platform");

    rc = clGetDeviceIDs(m_platform, gDeviceType, 1, &m_device, NULL);
    ASSERT_OCL_SUCCESS(rc, "Unable to get device");

    cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                     (cl_context_properties)m_platform, 0};
    m_context = clCreateContext(prop, 1, &m_device, NULL, NULL, &rc);
    ASSERT_OCL_SUCCESS(rc, "Unable to create context");

    m_queue =
        clCreateCommandQueueWithProperties(m_context, m_device, nullptr, &rc);
    ASSERT_OCL_SUCCESS(rc, "clCreateCommandQueueWithProperties");
  }

  void TearDown() override {
    cl_int rc = clReleaseCommandQueue(m_queue);
    ASSERT_OCL_SUCCESS(rc, "clReleaseCommandQueue");
    rc = clReleaseContext(m_context);
    ASSERT_OCL_SUCCESS(rc, "Unable to release context");
  }
};

// Trying to create program from cached binary file
// which doesn't contain .ocl.ver section
TEST_F(CheckBinaryVersionSuit, BinaryWithoutSectionTest) {
  std::vector<unsigned char> binary;
  ReadBinary(m_CachedBinaryWithoutSectionFileName, binary);
  ASSERT_NE(binary.size(), 0) << "Unable to read binary";
  std::size_t binarySize;
  binarySize = binary.size();
  unsigned char *p_binary = binary.data();

  cl_int binaryStatus;
  cl_int rc;
  clCreateProgramWithBinary(m_context, 1, &m_device, &binarySize,
                            const_cast<const unsigned char **>(&p_binary),
                            &binaryStatus, &rc);
  ASSERT_EQ(CL_INVALID_BINARY, rc) << "Failed to create program with binary";
}

// Trying to create program from cached binary file
// which has FFFFFFF version
TEST_F(CheckBinaryVersionSuit, BinaryWithWrongVersionTest) {
  std::vector<unsigned char> binary;
  ReadBinary(m_CachedBinaryWithWrongVersionFileName, binary);
  ASSERT_NE(binary.size(), 0) << "Unable to read binary";
  std::size_t binarySize;
  binarySize = binary.size();
  unsigned char *p_binary = binary.data();

  cl_int binaryStatus;
  cl_int rc;
  clCreateProgramWithBinary(m_context, 1, &m_device, &binarySize,
                            const_cast<const unsigned char **>(&p_binary),
                            &binaryStatus, &rc);
  ASSERT_EQ(CL_INVALID_BINARY, rc) << "Binary with wrong version should fail";
}

// Trying to create program from cached binary file
// which has current version
TEST_F(CheckBinaryVersionSuit, BinaryWithCurrentVersionTest) {
  // cached_binary_current_version.bin is generated on Linux. So we don't run
  // this test on Windows.
#ifndef _WIN32
  // Check if the current CPU is supported.
  if (Intel::OpenCL::Utils::IsCPUSupported() != CL_SUCCESS)
    return;
  std::vector<unsigned char> binary;
  ReadBinary(m_CachedBinaryWithCurrentVersionFileName, binary);
  ASSERT_NE(binary.size(), 0) << "Unable to read binary";
  std::size_t binarySize;
  binarySize = binary.size();
  unsigned char *p_binary = binary.data();

  cl_int binaryStatus;
  cl_int rc;
  cl_program program = clCreateProgramWithBinary(
      m_context, 1, &m_device, &binarySize,
      const_cast<const unsigned char **>(&p_binary), &binaryStatus, &rc);
  std::string updateMsg =
      "update " + m_CachedBinaryWithCurrentVersionFileName +
      " using following steps (only Linux is supported):\n"
      "1. export CL_CONFIG_CPU_TARGET_ARCH=corei7\n"
      "2. cd " +
      get_exe_dir() +
      "\n"
      "3. ./framework_test_type --gtest_filter=*BinaryWithUpToDateVersionTest\n"
      "4. cp " +
      m_CachedBinaryUpToDateVersionFileName + " " + CURRENT_SOURCE_DIR +
      llvm::sys::path::get_separator().str() +
      m_CachedBinaryWithCurrentVersionFileName + "\n";
  ASSERT_EQ(CL_SUCCESS, binaryStatus)
      << "Failed to create program. If "
         "OCL_CACHED_BINARY_VERSION has changed, please "
      << updateMsg;
  ASSERT_OCL_SUCCESS(rc, "clCreateProgramWithBinary");

  updateMsg = "\nIf program serialization has changed, "
              "please increment OCL_CACHED_BINARY_VERSION and " +
              updateMsg;

  rc = clBuildProgram(program, 1, &m_device, "", nullptr, nullptr);
  ASSERT_EQ(CL_SUCCESS, rc) << "clBuildProgram failed." << updateMsg;

  ASSERT_NO_FATAL_FAILURE(TestProgram(program, updateMsg));

  rc = clReleaseProgram(program);
  ASSERT_OCL_SUCCESS(rc, "clReleaseProgram");
#endif
}

// Trying to create program from cached binary file
// which has up-to-date version
TEST_F(CheckBinaryVersionSuit, BinaryWithUpToDateVersionTest) {
  GenerateUpToDateBinaryFile();
  LoadBinaryAndRun(m_CachedBinaryUpToDateVersionFileName);
}

// Trying to create program from cached binary files which were generated by
// gold release, gold update1 release and gold update2 relase
TEST_F(CheckBinaryVersionSuit, CompatiblityTest) {
  auto size = m_CompatibilityBinaryFileName.size();
  for (size_t i = 0; i < size; ++i)
    LoadBinaryAndRun(m_CompatibilityBinaryFileName[i]);
}
