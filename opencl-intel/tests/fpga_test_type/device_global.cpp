#include "CL/cl_gvp_ext.h"
#include "TestsHelpClasses.h"
#include "base_fixture.h"
#include "common_utils.h"

#include <vector>

static const char *BinaryFile1 = "device_global_image1.bc";
static const char *BinaryFile2 = "device_global_image2.bc";

class DeviceGlobalTest : public OCLFPGABaseFixture {
public:
  DeviceGlobalTest()
      : m_device(nullptr), m_context(nullptr), m_queue(nullptr),
        m_clEnqueueReadGlobalVariableINTEL(nullptr),
        m_clEnqueueWriteGlobalVariableINTEL(nullptr) {}

protected:
  using parent_t = OCLFPGABaseFixture;
  void SetUp() override {
    // Device global is a sycl feature, and sycl only support 64bit. So, there
    // is no need to run this test on win32 platform.
#if defined(_WIN32) && !defined(_WIN64)
    GTEST_SKIP();
#endif
    parent_t::SetUp();
    m_device = parent_t::device();
    m_context = parent_t::createContext(m_device);
    m_queue = parent_t::createCommandQueue(m_context, m_device);

    cl_platform_id platform = parent_t::platform();

    m_clEnqueueReadGlobalVariableINTEL = (clEnqueueReadGlobalVariableINTEL_fn)
        clGetExtensionFunctionAddressForPlatform(
            platform, "clEnqueueReadGlobalVariableINTEL");
    ASSERT_NE(nullptr, m_clEnqueueReadGlobalVariableINTEL)
        << "clGetExtensionFunctionAddressForPlatform("
           "\"clEnqueueReadGlobalVariableINTEL\") failed.";

    m_clEnqueueWriteGlobalVariableINTEL = (clEnqueueWriteGlobalVariableINTEL_fn)
        clGetExtensionFunctionAddressForPlatform(
            platform, "clEnqueueWriteGlobalVariableINTEL");
    ASSERT_NE(nullptr, m_clEnqueueWriteGlobalVariableINTEL)
        << "clGetExtensionFunctionAddressForPlatform("
           "\"clEnqueueWriteGlobalVariableINTEL\") failed.";
  }

  void TearDown() override { parent_t::TearDown(); }

  void BuildProgram(const char *BinaryFile, cl_program &program) {
    cl_int Err;
    std::vector<unsigned char> Binary;
    ASSERT_NO_FATAL_FAILURE(
        readBinary((get_exe_dir() + BinaryFile).c_str(), Binary));

    size_t Length = Binary.size();
    const unsigned char *ImageData =
        reinterpret_cast<const unsigned char *>(Binary.data());
    program = clCreateProgramWithBinary(m_context, 1, &m_device, &Length,
                                        &ImageData, nullptr, &Err);
    ASSERT_OCL_SUCCESS(Err, "clCreateProgramWithBinary");

    Err = clBuildProgram(program, 1, &m_device, "", nullptr, nullptr);
    ASSERT_OCL_SUCCESS(Err, "clBuildProgram");
  }

protected:
  cl_device_id m_device;
  cl_context m_context;
  cl_command_queue m_queue;
  clEnqueueReadGlobalVariableINTEL_fn m_clEnqueueReadGlobalVariableINTEL;
  clEnqueueWriteGlobalVariableINTEL_fn m_clEnqueueWriteGlobalVariableINTEL;
};

TEST_F(DeviceGlobalTest, resetGlobalWithinImageScope) {
  cl_int Err;
  cl_program Program1;
  ASSERT_NO_FATAL_FAILURE(BuildProgram(BinaryFile1, Program1));

  cl_program Program2;
  ASSERT_NO_FATAL_FAILURE(BuildProgram(BinaryFile2, Program2));

  cl_kernel Kernel1 = clCreateKernel(Program1, "add_counter1", &Err);
  ASSERT_OCL_SUCCESS(Err, "clCreateKernel");

  cl_kernel Kernel2 = clCreateKernel(Program2, "add_counter2", &Err);
  ASSERT_OCL_SUCCESS(Err, "clCreateKernel");

  size_t One = 1;
  // 1.launch kernel counter1 in image1, then program1 will be loaded.
  Err = clEnqueueNDRangeKernel(m_queue, Kernel1, 1, nullptr, &One, &One, 0,
                               nullptr, nullptr);
  ASSERT_OCL_SUCCESS(Err, "clEnqueueNDRangeKernel");

  Err = clFinish(m_queue);
  ASSERT_OCL_SUCCESS(Err, "clFinish");

  int output = 0;
  Err = m_clEnqueueReadGlobalVariableINTEL(m_queue, Program1, "counter1",
                                           CL_TRUE, sizeof(int), 0, &output, 0,
                                           nullptr, nullptr);
  ASSERT_OCL_SUCCESS(Err, "clEnqueueReadGlobalVariableINTEL");
  ASSERT_EQ(output, 1) << "The value of device global 'counter1' is incorrect.";

  Err = clFinish(m_queue);
  ASSERT_OCL_SUCCESS(Err, "clFinish");

  // 2.launch kernel counter2 in image2, then program2 will be loaded and
  // program2 will be marked as inactive.
  Err = clEnqueueNDRangeKernel(m_queue, Kernel2, 1, nullptr, &One, &One, 0,
                               nullptr, nullptr);
  ASSERT_OCL_SUCCESS(Err, "clEnqueueNDRangeKernel");

  Err = clFinish(m_queue);
  ASSERT_OCL_SUCCESS(Err, "clFinish");

  output = 0;
  Err = m_clEnqueueReadGlobalVariableINTEL(m_queue, Program2, "counter2",
                                           CL_TRUE, sizeof(int), 0, &output, 0,
                                           nullptr, nullptr);
  ASSERT_OCL_SUCCESS(Err, "clEnqueueReadGlobalVariableINTEL");
  ASSERT_EQ(output, 1) << "The value of device global 'counter2' is incorrect.";

  Err = clFinish(m_queue);
  ASSERT_OCL_SUCCESS(Err, "clFinish");

  // 3.launch kernel counter1 again, then program1 will be loaded and program2
  // will be marked as inactive. Since we re-loaded program1, the device global
  // with property will be reset.
  Err = clEnqueueNDRangeKernel(m_queue, Kernel1, 1, nullptr, &One, &One, 0,
                               nullptr, nullptr);
  ASSERT_OCL_SUCCESS(Err, "clEnqueueNDRangeKernel");

  Err = clFinish(m_queue);
  ASSERT_OCL_SUCCESS(Err, "clFinish");

  output = 0;
  Err = m_clEnqueueReadGlobalVariableINTEL(m_queue, Program1, "counter1",
                                           CL_TRUE, sizeof(int), 0, &output, 0,
                                           nullptr, nullptr);
  ASSERT_OCL_SUCCESS(Err, "clEnqueueReadGlobalVariableINTEL");
  ASSERT_EQ(output, 1) << "The value of device global 'counter1' is incorrect.";

  Err = clFinish(m_queue);
  ASSERT_OCL_SUCCESS(Err, "clFinish");

  Err = clReleaseKernel(Kernel1);
  ASSERT_OCL_SUCCESS(Err, "clReleaseKernel");
  Err = clReleaseKernel(Kernel2);
  ASSERT_OCL_SUCCESS(Err, "clReleaseKernel");
  Err = clReleaseProgram(Program1);
  ASSERT_OCL_SUCCESS(Err, "clReleaseProgram");
  Err = clReleaseProgram(Program2);
  ASSERT_OCL_SUCCESS(Err, "clReleaseProgram");
}
