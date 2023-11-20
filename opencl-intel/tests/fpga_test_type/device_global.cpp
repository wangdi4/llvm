#include "CL/cl_internal_ext.h"
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

  void BuildProgram(const char *BinaryFile, cl_context context,
                    cl_device_id device, cl_program *program) {
    cl_int Err;
    std::vector<unsigned char> Binary;
    ASSERT_NO_FATAL_FAILURE(
        readBinary((get_exe_dir() + BinaryFile).c_str(), Binary));

    size_t Length = Binary.size();
    const unsigned char *ImageData =
        reinterpret_cast<const unsigned char *>(Binary.data());
    *program = clCreateProgramWithBinary(context, 1, &device, &Length,
                                         &ImageData, nullptr, &Err);
    ASSERT_OCL_SUCCESS(Err, "clCreateProgramWithBinary");

    Err = clBuildProgram(*program, 1, &device, "", nullptr, nullptr);
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
  clProgramWrapper Program1;
  ASSERT_NO_FATAL_FAILURE(
      BuildProgram(BinaryFile1, m_context, m_device, &Program1));

  clProgramWrapper Program2;
  ASSERT_NO_FATAL_FAILURE(
      BuildProgram(BinaryFile2, m_context, m_device, &Program2));

  clKernelWrapper Kernel1 = clCreateKernel(Program1, "add_counter1", &Err);
  ASSERT_OCL_SUCCESS(Err, "clCreateKernel");

  clKernelWrapper Kernel2 = clCreateKernel(Program2, "add_counter2", &Err);
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
}

TEST_F(DeviceGlobalTest, resetGlobalInParentDeivceCreatedProgram) {
  cl_int Err;
  cl_uint MaxComputeUnits;
  const int NumSubdevices = 2;
  Err = clGetDeviceInfo(m_device, CL_DEVICE_MAX_COMPUTE_UNITS,
                        sizeof(MaxComputeUnits), &MaxComputeUnits, nullptr);
  ASSERT_OCL_SUCCESS(Err, "clGetDeviceInfo");

  // create room for 1 more device_id, so that we can put the parent device in
  // there.
  cl_device_id Devices[NumSubdevices + 1];
  cl_device_partition_property PartitionProps[] = {
      CL_DEVICE_PARTITION_EQUALLY, MaxComputeUnits / NumSubdevices, 0};
  Err = clCreateSubDevices(m_device, PartitionProps, NumSubdevices, Devices,
                           nullptr);
  ASSERT_OCL_SUCCESS(Err, "clCreateSubDevices");

  Devices[NumSubdevices] = m_device;
  clContextWrapper Context = clCreateContext(nullptr, NumSubdevices + 1,
                                             Devices, nullptr, nullptr, &Err);
  ASSERT_OCL_SUCCESS(Err, "clCreateContext");

  // create and build program by using parent device
  clProgramWrapper Program;
  ASSERT_NO_FATAL_FAILURE(
      BuildProgram(BinaryFile1, Context, m_device, &Program));

  clKernelWrapper Kernel = clCreateKernel(Program, "add_counter1", &Err);
  ASSERT_OCL_SUCCESS(Err, "clCreateKernel");

  clCommandQueueWrapper Queues[NumSubdevices];
  for (int i = 0; i < NumSubdevices; i++) {
    Queues[i] =
        clCreateCommandQueueWithProperties(Context, Devices[i], nullptr, &Err);
    ASSERT_OCL_SUCCESS(Err, "clCreateCommandQueueWithProperties");
  }

  size_t One = 1;
  for (int i = 0; i < NumSubdevices; i++) {
    Err = clEnqueueNDRangeKernel(Queues[i], Kernel, 1, nullptr, &One, &One, 0,
                                 nullptr, nullptr);
    ASSERT_OCL_SUCCESS(Err, "clEnqueueNDRangeKernel");

    Err = clFinish(Queues[i]);
    ASSERT_OCL_SUCCESS(Err, "clFinish");
  }

  int Val = 0;
  Err = m_clEnqueueReadGlobalVariableINTEL(m_queue, Program, "counter1",
                                           CL_TRUE, sizeof(Val), 0, &Val, 0,
                                           nullptr, nullptr);
  ASSERT_OCL_SUCCESS(Err, "clEnqueueReadGlobalVariableINTEL");
  ASSERT_EQ(Val, 2);
}
