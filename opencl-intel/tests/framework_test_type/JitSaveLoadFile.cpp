// INTEL CONFIDENTIAL
//
// Copyright 2020 Intel Corporation.
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

#include "CL/cl.h"
#include "FrameworkTest.h"
#include "TestsHelpClasses.h"
#include "cl_cpu_detect.h"
#include "common_utils.h"

extern cl_device_type gDeviceType;

class JitSaveLoadFileTest : public ::testing::Test {
protected:
  virtual void SetUp() override {
    cl_int err = clGetPlatformIDs(1, &m_platform, nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetPlatformIDs");

    err = clGetDeviceIDs(m_platform, gDeviceType, 1, &m_device, nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceIDs");

    m_context = clCreateContext(nullptr, 1, &m_device, nullptr, nullptr, &err);
    ASSERT_OCL_SUCCESS(err, "clCreateContext");

    m_queue =
        clCreateCommandQueueWithProperties(m_context, m_device, nullptr, &err);
    ASSERT_OCL_SUCCESS(err, "clCreateCommandQueueWithProperties");
  }

  virtual void TearDown() override {
    cl_int err = clReleaseCommandQueue(m_queue);
    EXPECT_OCL_SUCCESS(err, "clReleaseCommandQueue");
    err = clReleaseContext(m_context);
    EXPECT_OCL_SUCCESS(err, "clReleaseContext");
  }

  /// Build program from source and save program binary to file.
  void jitSave() {
    size_t binarySize = 0;
    std::string source =
        " __attribute__((reqd_work_group_size(" + std::to_string(m_reqdWGSize) +
        ",1,1)))\n"
        "__kernel void test_kernel(__global int* dst, __global int* src)\n"
        "{\n"
        "    size_t gid = get_global_id(0);\n"
        "    dst[gid] = (int)(10.0*(native_sin((float)src[gid])));\n"
        "}";
    const char *csource = source.c_str();

    // Build program
    cl_program program;
    ASSERT_TRUE(BuildProgramSynch(m_context, 1, &csource, nullptr,
                                  "-cl-denorms-are-zero", &program));

    // Get program binary
    cl_int err = clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES,
                                  sizeof(binarySize), &binarySize, nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetProgramInfo CL_PROGRAM_BINARY_SIZES");
    std::vector<unsigned char> binary(binarySize);
    unsigned char *binaries[1] = {&binary[0]};
    err = clGetProgramInfo(program, CL_PROGRAM_BINARIES, sizeof(binaries),
                           &binaries, nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetProgramInfo CL_PROGRAM_BINARIES");

    // Save binary to file
    std::ofstream ofs(m_binaryFilename, std::ios::binary);
    ASSERT_TRUE(ofs) << "Failed to open file to write";
    ofs.write((const char *)&binary[0], binarySize);
    ofs.close();
    ASSERT_TRUE(ofs) << "Error in writing file";

    // Release program
    err = clReleaseProgram(program);
    ASSERT_OCL_SUCCESS(err, "clReleaseProgram");
  }

  /// Load program binary from file and create new program from it.
  void jitLoad(const std::string &binaryFilename) {
    size_t binarySize = 0;
    // Read binary from file
    std::ifstream ifs(binaryFilename, std::ios::binary);
    ASSERT_TRUE(ifs) << "Failed to open file to read";
    std::vector<unsigned char> binary(std::istreambuf_iterator<char>(ifs), {});
    ifs.close();
    ASSERT_TRUE(ifs) << "Error in reading file";
    binarySize = binary.size();

    // Create and build program
    cl_int err;
    cl_int binaryStatus[1];
    const unsigned char *binaries[1] = {&binary[0]};
    cl_program program = clCreateProgramWithBinary(
        m_context, 1, &m_device, &binarySize, binaries, binaryStatus, &err);
    ASSERT_OCL_SUCCESS(err, "clCreateProgramWithBinary");
    ASSERT_OCL_SUCCESS(binaryStatus[0], "clCreateProgramWithBinary");

    err = clBuildProgram(program, 1, &m_device, nullptr, nullptr, nullptr);
    ASSERT_OCL_SUCCESS(err, "clBuildProgram");

    // Test program
    ASSERT_NO_FATAL_FAILURE(testBinaryRun(program));

    // Release program
    err = clReleaseProgram(program);
    ASSERT_OCL_SUCCESS(err, "clReleaseProgram");
  }

  /// Create and execute kernel. Verify kernel output.
  void testBinaryRun(cl_program program) {
    // Prepare buffers
    size_t globalWorkSize = m_reqdWGSize;
    std::vector<int> dst(globalWorkSize);
    for (size_t i = 0; i < globalWorkSize; ++i)
      dst[i] = 0;
    std::vector<int> src(globalWorkSize);
    for (size_t i = 0; i < globalWorkSize; ++i)
      src[i] = 90 * (i % 2);

    cl_int err;
    cl_mem bufDst =
        clCreateBuffer(m_context, CL_MEM_READ_WRITE,
                       sizeof(cl_int) * globalWorkSize, nullptr, &err);
    ASSERT_OCL_SUCCESS(err, "clCreateBuffer");
    err = clEnqueueWriteBuffer(m_queue, bufDst, CL_TRUE, 0,
                               sizeof(cl_int) * globalWorkSize, &(dst[0]), 0,
                               nullptr, nullptr);
    ASSERT_OCL_SUCCESS(err, "clEnqueueWriteBuffer");
    cl_mem bufSrc =
        clCreateBuffer(m_context, CL_MEM_READ_ONLY,
                       sizeof(cl_int) * globalWorkSize, nullptr, &err);
    ASSERT_OCL_SUCCESS(err, "clCreateBuffer");
    err = clEnqueueWriteBuffer(m_queue, bufSrc, CL_TRUE, 0,
                               sizeof(cl_int) * globalWorkSize, &(src[0]), 0,
                               nullptr, nullptr);
    ASSERT_OCL_SUCCESS(err, "clEnqueueWriteBuffer");

    // Create kernel
    cl_kernel kernel = clCreateKernel(program, "test_kernel", &err);
    ASSERT_OCL_SUCCESS(err, "clCreateKernel");

    // Check kernel info
    ASSERT_NO_FATAL_FAILURE(checkKernelInfo(kernel));

    err = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&bufDst);
    ASSERT_OCL_SUCCESS(err, "clSetKernelArg");
    err = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&bufSrc);
    ASSERT_OCL_SUCCESS(err, "clSetKernelArg");

    err = clEnqueueNDRangeKernel(m_queue, kernel, 1, nullptr, &globalWorkSize,
                                 &m_reqdWGSize, 0, nullptr, nullptr);
    ASSERT_OCL_SUCCESS(err, "clEnqueueNDRangeKernel");

    std::vector<int> result(globalWorkSize);
    err = clEnqueueReadBuffer(m_queue, bufDst, CL_TRUE, 0,
                              sizeof(cl_int) * globalWorkSize, &(result[0]), 0,
                              nullptr, nullptr);
    ASSERT_OCL_SUCCESS(err, "clEnqueueReadBuffer");

    for (int i = 0; i < (int)globalWorkSize; ++i)
      ASSERT_EQ(result[i], 8 * (i % 2))
          << ("result[" + std::to_string(i) + "] is incorrect");

    err = clReleaseMemObject(bufSrc);
    ASSERT_OCL_SUCCESS(err, "clReleaseMemObject");
    err = clReleaseMemObject(bufDst);
    ASSERT_OCL_SUCCESS(err, "clReleaseMemObject");
    err = clReleaseKernel(kernel);
    ASSERT_OCL_SUCCESS(err, "clReleaseKernel");
  }

  /// Query kernel information
  void checkKernelInfo(cl_kernel kernel) {
    size_t attrSize;
    cl_int err =
        clGetKernelInfo(kernel, CL_KERNEL_ATTRIBUTES, 0, nullptr, &attrSize);
    ASSERT_OCL_SUCCESS(err, "clGetKernelInfo");
    std::vector<char> attributes(attrSize);
    err = clGetKernelInfo(kernel, CL_KERNEL_ATTRIBUTES, attrSize,
                          attributes.data(), nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetKernelInfo");
    std::string attributesStr(attributes.data());
    ASSERT_TRUE(attributesStr.empty()) << "kernel attribute should be empty";
  }

protected:
  cl_platform_id m_platform;
  cl_device_id m_device;
  cl_context m_context;
  cl_command_queue m_queue;
  const std::string m_binaryFilename = "jit.bin";
  const size_t m_reqdWGSize = 8;
};

TEST_F(JitSaveLoadFileTest, binaryFile) {
  ASSERT_NO_FATAL_FAILURE(jitSave());
  ASSERT_NO_FATAL_FAILURE(jitLoad(m_binaryFilename));

  // Delete binary file
  (void)std::remove(m_binaryFilename.c_str());
}

// Pre-build binary is generated by JitSaveLoadFileTest.binaryFile with
// CL_CONFIG_CPU_TARGET_ARCH=corei7 setting. Running the binary requires
// __ocl_svml_h8_sinf1_native. The test checks current cpu device with higher
// level cpu arch can run the pre-build binary well.
#if defined(__linux__)
TEST_F(JitSaveLoadFileTest, reloadBuiltinModule) {
  if (Intel::OpenCL::Utils::CPUDetect::GetInstance()->HasSSE42()) {
    ASSERT_NO_FATAL_FAILURE(
        jitLoad(get_exe_dir() + "jit-saveload-cached-binary.bin"));
  }
}
#endif
