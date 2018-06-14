#include "CL/cl.h"

#include <numeric>
#include <cstring>
#include <string>

#include "FrameworkTest.h"
#include "common_utils.h"
#include "gtest/gtest.h"

extern cl_device_type gDeviceType;

static void doTestImpl(cl_context context, cl_kernel input, cl_kernel output,
                       std::vector<cl_int> &data) {
  cl_int error = CL_SUCCESS;
  cl_mem input_buffer = clCreateBuffer(
      context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
      data.size() * sizeof(cl_int), (void *)&data.front(), &error);
  ASSERT_EQ(CL_SUCCESS, error) << " clCreateBuffer failed.";

  error = clSetKernelArg(input, 0, sizeof(cl_mem), &input_buffer);
  ASSERT_EQ(CL_SUCCESS, error) << " clSetKernelArg failed.";

  std::vector<cl_int> output_data(data.size(), 0);
  cl_mem output_buffer =
      clCreateBuffer(context, CL_MEM_WRITE_ONLY,
                     sizeof(cl_int) * output_data.size(), nullptr, &error);
  ASSERT_EQ(CL_SUCCESS, error) << " clCreateBuffer failed.";

  error = clSetKernelArg(output, 0, sizeof(cl_mem), &output_buffer);
  ASSERT_EQ(CL_SUCCESS, error) << " clSetKernelArg failed.";

  cl_device_id device = nullptr;
  error = clGetContextInfo(context, CL_CONTEXT_DEVICES, sizeof(cl_device_id),
                           &device, nullptr);
  ASSERT_EQ(CL_SUCCESS, error) << " clGetContextInfo failed.";

  cl_command_queue input_queue =
      clCreateCommandQueueWithProperties(context, device, nullptr, &error);
  ASSERT_EQ(CL_SUCCESS, error) << " clCreateCommandQueueWithProperties failed.";
  cl_command_queue output_queue =
      clCreateCommandQueueWithProperties(context, device, nullptr, &error);
  ASSERT_EQ(CL_SUCCESS, error) << " clCreateCommandQueueWithProperties failed.";

  size_t global_size[] = {1, 1, 1};
  size_t local_size[] = {1, 1, 1};
  error = clEnqueueNDRangeKernel(input_queue, input, 1, nullptr, global_size,
                                 local_size, 0, nullptr, nullptr);
  ASSERT_EQ(CL_SUCCESS, error) << " clEnqueueNDRangeKernel failed.";

  error = clEnqueueNDRangeKernel(output_queue, output, 1, nullptr, global_size,
                                 local_size, 0, nullptr, nullptr);
  ASSERT_EQ(CL_SUCCESS, error) << " clEnqueueNDRangeKernel failed.";

  error = clEnqueueReadBuffer(output_queue, output_buffer, CL_TRUE, 0,
                              sizeof(cl_int) * output_data.size(),
                              &output_data.front(), 0, nullptr, nullptr);
  ASSERT_EQ(CL_SUCCESS, error) << " clEnqueueReadBuffer failed.";

  for (cl_int i = 0; i < (cl_int)data.size(); ++i) {
    ASSERT_EQ(data[i], output_data[i]) << " output data is wrong!";
  }

  clReleaseCommandQueue(input_queue);
  clReleaseCommandQueue(output_queue);
  clReleaseMemObject(input_buffer);
  clReleaseMemObject(output_buffer);
}

static void doTest(std::vector<cl_int> &data, int depth) {
  cl_int error = CL_SUCCESS;
  cl_platform_id platform = nullptr;
  cl_device_id device = nullptr;

  error = clGetPlatformIDs(1, &platform, nullptr);
  ASSERT_EQ(CL_SUCCESS, error) << " clGetPlatformIDs failed.";

  error = clGetDeviceIDs(platform, gDeviceType, 1, &device, nullptr);
  ASSERT_EQ(CL_SUCCESS, error) << " clGetDeviceIDs failed to obtain device "
                               << "of " << gDeviceType << " type.";

  const cl_context_properties props[5] = {
      CL_CONTEXT_PLATFORM, (cl_context_properties)platform,
      CL_CONTEXT_FPGA_EMULATOR_INTEL, CL_TRUE, 0
  };

  cl_context context =
      clCreateContext(props, 1, &device, nullptr, nullptr, &error);
  ASSERT_EQ(CL_SUCCESS, error) << " clCreateContext failed.";

  const char *program_sources = "                                            \n\
      #pragma OPENCL EXTENSION cl_intel_channels: enable                     \n\
      #ifdef DEPTH                                                           \n\
        channel int c __attribute__((depth(DEPTH)));                         \n\
      #else                                                                  \n\
        channel int c;                                                       \n\
      #endif                                                                 \n\
                                                                             \n\
      __attribute__((max_global_work_dim(0)))                                \n\
      __kernel void writer(__global int* data) {                             \n\
        #pragma omp simd                                                     \n\
        for (int i = 0; i < DATA_SIZE; ++i) {                                \n\
          write_channel_intel(c, data[i]);                                   \n\
        }                                                                    \n\
      }                                                                      \n\
                                                                             \n\
      __attribute__((max_global_work_dim(0)))                                \n\
      __kernel void reader(__global int* data) {                             \n\
        #pragma omp simd                                                     \n\
        for (int i = 0; i < DATA_SIZE; ++i) {                                \n\
          data[i] = read_channel_intel(c);                                   \n\
        }                                                                    \n\
      }                                                                      \n\
    ";

  cl_program program =
      clCreateProgramWithSource(context, 1, &program_sources, nullptr, &error);
  ASSERT_EQ(CL_SUCCESS, error) << " clCreateProgramWithSource failed.";

  std::string build_options =
      "-DDATA_SIZE=" + std::to_string(data.size()) + " ";
  if (depth != 0) {
    build_options += "-DDEPTH=" + std::to_string(depth);
  }
  error = clBuildProgram(program, 1, &device, build_options.c_str(), nullptr,
                         nullptr);
  EXPECT_EQ(CL_SUCCESS, error) << " clBuildProgram failed.";
  if (CL_SUCCESS != error) {
    size_t log_size = 0;
    error = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0,
                                  nullptr, &log_size);
    ASSERT_EQ(CL_SUCCESS, error) << " clGetProgramBuildInfo failed.";
    std::string log("", log_size);
    error = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
                                  log_size, &log[0], nullptr);
    ASSERT_EQ(CL_SUCCESS, error) << " clGetProgramBuildInfo failed.";
    std::cout << log << std::endl;
    return;
  }

  cl_kernel reader = clCreateKernel(program, "writer", &error);
  ASSERT_EQ(CL_SUCCESS, error) << " clCreateKernel failed.";
  cl_kernel writer = clCreateKernel(program, "reader", &error);
  ASSERT_EQ(CL_SUCCESS, error) << " clCreateKernel failed.";

  doTestImpl(context, reader, writer, data);

  clReleaseKernel(reader);
  clReleaseKernel(writer);
  clReleaseProgram(program);
  clReleaseContext(context);
}

static void FPGAChannelDepthEmulationStrictWithDepthVPO() {
  SETENV("CL_CONFIG_CHANNEL_DEPTH_EMULATION_MODE", "strict");
  SETENV("VOLCANO_CLANG_OPTIONS",
         "-fopenmp -fintel-openmp -fopenmp-tbb -fintel-compatibility");
  std::vector<cl_int> data(128);
  std::iota(data.begin(), data.end(), 0);
  doTest(data, 128);
  UNSETENV("VOLCANO_CLANG_OPTIONS");
  UNSETENV("CL_CONFIG_CHANNEL_DEPTH_EMULATION_MODE");
}

static void FPGAChannelDepthEmulationStrictWithoutDepthVPO() {
  SETENV("CL_CONFIG_CHANNEL_DEPTH_EMULATION_MODE", "strict");
  SETENV("VOLCANO_CLANG_OPTIONS",
         "-fopenmp -fintel-openmp -fopenmp-tbb -fintel-compatibility");
  std::vector<cl_int> data(128);
  std::iota(data.begin(), data.end(), 0);
  doTest(data, 0);
  UNSETENV("VOLCANO_CLANG_OPTIONS");
  UNSETENV("CL_CONFIG_CHANNEL_DEPTH_EMULATION_MODE");
}

static void FPGAChannelDepthEmulationDefaultWithDepthVPO() {
  SETENV("CL_CONFIG_CHANNEL_DEPTH_EMULATION_MODE", "default");
  SETENV("VOLCANO_CLANG_OPTIONS",
         "-fopenmp -fintel-openmp -fopenmp-tbb -fintel-compatibility");
  std::vector<cl_int> data(128);
  std::iota(data.begin(), data.end(), 0);
  doTest(data, 128);
  UNSETENV("VOLCANO_CLANG_OPTIONS");
  UNSETENV("CL_CONFIG_CHANNEL_DEPTH_EMULATION_MODE");
}

static void FPGAChannelDepthEmulationDefaultWithoutDepthVPO() {
  SETENV("CL_CONFIG_CHANNEL_DEPTH_EMULATION_MODE", "default");
  SETENV("VOLCANO_CLANG_OPTIONS",
         "-fopenmp -fintel-openmp -fopenmp-tbb -fintel-compatibility");
  std::vector<cl_int> data(128);
  std::iota(data.begin(), data.end(), 0);
  doTest(data, 0);
  UNSETENV("VOLCANO_CLANG_OPTIONS");
  UNSETENV("CL_CONFIG_CHANNEL_DEPTH_EMULATION_MODE");
}

static void FPGAChannelDepthEmulationIgnoreDepthWithDepthVPO() {
  SETENV("CL_CONFIG_CHANNEL_DEPTH_EMULATION_MODE", "ignore-depth");
  SETENV("VOLCANO_CLANG_OPTIONS",
         "-fopenmp -fintel-openmp -fopenmp-tbb -fintel-compatibility");
  std::vector<cl_int> data(128);
  std::iota(data.begin(), data.end(), 0);
  doTest(data, 128);
  UNSETENV("VOLCANO_CLANG_OPTIONS");
  UNSETENV("CL_CONFIG_CHANNEL_DEPTH_EMULATION_MODE");
}

static void FPGAChannelDepthEmulationIgnoreDepthWithoutDepthVPO() {
  SETENV("CL_CONFIG_CHANNEL_DEPTH_EMULATION_MODE", "ignore-depth");
  SETENV("VOLCANO_CLANG_OPTIONS",
         "-fopenmp -fintel-openmp -fopenmp-tbb -fintel-compatibility");
  std::vector<cl_int> data(128);
  std::iota(data.begin(), data.end(), 0);
  doTest(data, 0);
  UNSETENV("VOLCANO_CLANG_OPTIONS");
  UNSETENV("CL_CONFIG_CHANNEL_DEPTH_EMULATION_MODE");
}

#ifdef BUILD_FPGA_EMULATOR
TEST(FPGA, Test_ChannelDepthEmulationStrictVPO)
{
    ASSERT_NO_FATAL_FAILURE(FPGAChannelDepthEmulationStrictWithDepthVPO());
    ASSERT_NO_FATAL_FAILURE(FPGAChannelDepthEmulationStrictWithoutDepthVPO());
}

TEST(FPGA, Test_ChannelDepthEmulationDefaultVPO)
{
    ASSERT_NO_FATAL_FAILURE(FPGAChannelDepthEmulationDefaultWithDepthVPO());
    ASSERT_NO_FATAL_FAILURE(FPGAChannelDepthEmulationDefaultWithoutDepthVPO());
}

TEST(FPGA, Test_ChannelDepthEmulationIgnoreDepthVPO)
{
    ASSERT_NO_FATAL_FAILURE(FPGAChannelDepthEmulationIgnoreDepthWithDepthVPO());
    ASSERT_NO_FATAL_FAILURE(
        FPGAChannelDepthEmulationIgnoreDepthWithoutDepthVPO());
}
#endif // BUILD_FPGA_EMULATOR
