#include "CL/cl.h"
#include "cl_cpu_detect.h"
#include "cl_types.h"

#include <fstream>
#include <gtest/gtest.h>
#include <stdio.h>

#include "FrameworkTest.h"
#include "common_utils.h"

extern cl_device_type gDeviceType;

class CheckBinaryVersionSuit : public ::testing::Test {
protected:
  cl_platform_id m_platform;
  cl_device_id m_device;
  cl_context m_context;

  std::string m_CachedBinaryWithoutSectionFileName =
      "cached_binary_without_section.bin";
  std::string m_CachedBinaryWithWrongVersionFileName =
      "cached_binary_with_wrong_version.bin";
  std::string m_CachedBinaryWithCurrentVersionFileName =
      "cached_binary_current_version.bin";
  std::string m_CachedBinaryUpToDateVersionFileName =
      "cached_binary_up_to_date.bin";

  void ReadBinary(std::string filename, std::vector<unsigned char>& file) {
    std::ifstream binaryFile(get_exe_dir() + filename,
           std::fstream::binary | std::fstream::in);
    ASSERT_TRUE(binaryFile.is_open()) << "Unable to open file";

    std::copy(std::istreambuf_iterator<char>(binaryFile),
        std::istreambuf_iterator<char>(),
        std::back_inserter(file));
    binaryFile.close();
  }

  void GenerateUpToDateBinaryFile() {
    bool bResult = true;
    cl_int rc = CL_OUT_OF_HOST_MEMORY;
    const char *testProgram[] = {"__kernel void test_kernel(__global int* A) {"
                                 "   int id = get_global_id(0);"
                                 "   printf(\"%d \", A[id]);"
                                 "}"};

    cl_program clProg;
    bResult &= BuildProgramSynch(m_context, 1, (const char **)&testProgram,
                                 NULL, "", &clProg);
    ASSERT_TRUE(bResult) << "Unable to build up-to-date binary";

    size_t binarySize = 0;
    char *pBinaries = NULL;
    if (bResult) {
      // get the binary
      rc = clGetProgramInfo(clProg, CL_PROGRAM_BINARY_SIZES, sizeof(size_t),
                            &binarySize, NULL);
      ASSERT_EQ(CL_SUCCESS, rc) << "clGetProgramInfo(CL_PROGRAM_BINARY_SIZES)";

      // cache the binary
      pBinaries = new char[binarySize];
      rc = clGetProgramInfo(clProg, CL_PROGRAM_BINARIES, binarySize, &pBinaries,
                            NULL);
      ASSERT_EQ(CL_SUCCESS, rc) << "clGetProgramInfo(CL_PROGRAM_BINARIES)";

      FILE *fout;
      std::string filename = get_exe_dir() +
          m_CachedBinaryUpToDateVersionFileName;
      fout = fopen(filename.c_str(), "wb");
      ASSERT_FALSE(fout == NULL) << "Failed to open file.\n";

      fwrite(pBinaries, 1, binarySize, fout);
      ASSERT_EQ(ferror(fout), 0) << "Error in writing to file " << filename;
      ASSERT_EQ(fclose(fout), 0) << "Failed to close file " << filename;
    }
  }

  void SetUp() override {
    m_platform = 0;
    cl_int rc = CL_OUT_OF_HOST_MEMORY;
    rc = clGetPlatformIDs(1, &m_platform, NULL);
    ASSERT_EQ(CL_SUCCESS, rc) << "Unable to get platform";

    rc = clGetDeviceIDs(m_platform, gDeviceType, 1, &m_device, NULL);
    ASSERT_EQ(CL_SUCCESS, rc) << "Unable to get device";

    cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                     (cl_context_properties)m_platform, 0};
    m_context = clCreateContext(prop, 1, &m_device, NULL, NULL, &rc);
    ASSERT_EQ(CL_SUCCESS, rc) << "Unable to create context";
  }

  void TearDown() override {
    cl_int rc = clReleaseContext(m_context);
    ASSERT_EQ(CL_SUCCESS, rc) << "Unable to release context";
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
  unsigned char* p_binary = binary.data();

  cl_int binaryStatus;
  cl_int rc;
  clCreateProgramWithBinary(
      m_context, 1, &m_device, &binarySize,
      const_cast<const unsigned char **>(&p_binary), &binaryStatus, &rc);
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
  unsigned char* p_binary = binary.data();

  cl_int binaryStatus;
  cl_int rc;
  clCreateProgramWithBinary(
      m_context, 1, &m_device, &binarySize,
      const_cast<const unsigned char **>(&p_binary), &binaryStatus, &rc);
  ASSERT_EQ(CL_INVALID_BINARY, rc) << "Failed to create program with binary";
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
      " according to following steps (only Linux is supported):\n"
      "1. export CL_CONFIG_CPU_TARGET_ARCH=corei7\n"
      "2. cd " + get_exe_dir() + "\n"
      "3. ./framework_test_type --gtest_filter=*BinaryWithUpToDateVersionTest\n"
      "4. copy " + m_CachedBinaryUpToDateVersionFileName + " to " +
      m_CachedBinaryWithCurrentVersionFileName + " in source dir";
  ASSERT_EQ(CL_SUCCESS, rc) << "Failed to create program. If "
                               "OCL_CACHED_BINARY_VERSION has changed, please "
                            << updateMsg;

  rc = clBuildProgram(program, 1, &m_device, "", nullptr, nullptr);
  ASSERT_EQ(CL_SUCCESS, rc)
      << "Failed to build program. If program serialization has changed, "
         "please increment OCL_CACHED_BINARY_VERSION and "
      << updateMsg;
#endif
}

// Trying to create program from cached binary file
// which has up-to-date version
TEST_F(CheckBinaryVersionSuit, BinaryWithUpToDateVersionTest) {
  GenerateUpToDateBinaryFile();
  std::vector<unsigned char> binary;
  ReadBinary(m_CachedBinaryUpToDateVersionFileName, binary);
  ASSERT_NE(binary.size(), 0) << "Unable to read binary";
  std::size_t binarySize;
  binarySize = binary.size();
  unsigned char* p_binary = binary.data();

  cl_int binaryStatus;
  cl_int rc;
  clCreateProgramWithBinary(
      m_context, 1, &m_device, &binarySize,
      const_cast<const unsigned char **>(&p_binary), &binaryStatus, &rc);
  ASSERT_EQ(CL_SUCCESS, rc) << "Failed to create program with binary";
}
