#include "CL/cl.h"

#include "FrameworkTest.h"
#include "gtest/gtest.h"

#include <numeric>
#include <vector>
#include <string>

extern cl_device_type gDeviceType;

static void FPGAChannelsMultipleCallSize() {
  cl_int error = CL_SUCCESS;
  cl_platform_id platform = nullptr;

  error = clGetPlatformIDs(1, &platform, nullptr);
  ASSERT_EQ(CL_SUCCESS, error) << " clGetPlatformIDs failed.";

  cl_device_id device = nullptr;
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

  const char *program_source = "\n\
    #pragma OPENCL EXTENSION cl_intel_channels : enable                      \n\
    channel int a;                                                           \n\
    channel int b[3];                                                        \n\
                                                                             \n\
    __attribute__((max_global_work_dim(0)))                                  \n\
    __kernel void host_reader(__global int *data, int size) {                \n\
      for (int i = 0; i < size; ++i) {                                       \n\
        write_channel_intel(a, data[i]);                                     \n\
        while (!write_channel_nb_intel(a, data[i]));                         \n\
        write_channel_intel(b[0], data[i]);                                  \n\
        write_channel_intel(b[1], data[i]);                                  \n\
        write_channel_intel(b[2], data[i]);                                  \n\
        while (!write_channel_nb_intel(b[0], data[i]));                      \n\
        while (!write_channel_nb_intel(b[1], data[i]));                      \n\
        while (!write_channel_nb_intel(b[2], data[i]));                      \n\
      }                                                                      \n\
    }                                                                        \n\
                                                                             \n\
    __attribute__((max_global_work_dim(0)))                                  \n\
    __kernel void host_writer(__global int *data, int size) {                \n\
      bool valid = false;                                                    \n\
      for (int i = 0; i < size; ++i) {                                       \n\
        data[8 * i + 0] = read_channel_intel(a);                             \n\
        do {                                                                 \n\
          data[8 * i + 1] = read_channel_nb_intel(a, &valid);                \n\
        } while (!valid);                                                    \n\
        data[8 * i + 2] = read_channel_intel(b[0]);                          \n\
        data[8 * i + 3] = read_channel_intel(b[1]);                          \n\
        data[8 * i + 4] = read_channel_intel(b[2]);                          \n\
        do {                                                                 \n\
          data[8 * i + 5] = read_channel_nb_intel(b[0], &valid);             \n\
        } while (!valid);                                                    \n\
        do {                                                                 \n\
          data[8 * i + 6] = read_channel_nb_intel(b[1], &valid);             \n\
        } while (!valid);                                                    \n\
        do {                                                                 \n\
          data[8 * i + 7] = read_channel_nb_intel(b[2], &valid);             \n\
        } while (!valid);                                                    \n\
      }                                                                      \n\
    }                                                                        \n\
    ";

  cl_program program =
      clCreateProgramWithSource(context, 1, &program_source, nullptr, &error);
  ASSERT_EQ(CL_SUCCESS, error) << " clCreateProgramWithSource failed.";

  error = clBuildProgram(program, 1, &device, "", nullptr, nullptr);
  EXPECT_EQ(CL_SUCCESS, error) << " clBuildProgram failed.";
  if (CL_SUCCESS != error) {
    size_t logSize = 0;
    error = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0,
                                  nullptr, &logSize);
    ASSERT_EQ(CL_SUCCESS, error) << " clGetProgramBuildInfo failed.";
    std::string log("", logSize);
    error = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
                                  logSize, &log[0], nullptr);
    ASSERT_EQ(CL_SUCCESS, error) << " clGetProgramBuildInfo failed.";
    FAIL() << "Build program failed. Logs: " << log << "\n";
  }

  cl_int size = 1000;
  std::vector<cl_int> data(size);
  std::iota(data.begin(), data.end(), 0);

  cl_mem input_buffer =
      clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                     sizeof(cl_int) * size, &data.front(), &error);
  ASSERT_EQ(CL_SUCCESS, error) << " clCreateBuffer failed.";

  cl_mem output_buffer = clCreateBuffer(
      context, CL_MEM_WRITE_ONLY, sizeof(cl_int) * size * 8, nullptr, &error);
  ASSERT_EQ(CL_SUCCESS, error) << " clCreateBuffer failed.";

  cl_kernel host_reader = clCreateKernel(program, "host_reader", &error);
  ASSERT_EQ(CL_SUCCESS, error) << " clCreateKernel failed.";

  error = clSetKernelArg(host_reader, 0, sizeof(cl_mem), &input_buffer);
  ASSERT_EQ(CL_SUCCESS, error) << " clSetKernelArg failed.";
  error = clSetKernelArg(host_reader, 1, sizeof(cl_int), &size);
  ASSERT_EQ(CL_SUCCESS, error) << " clSetKernelArg failed.";

  cl_kernel host_writer = clCreateKernel(program, "host_writer", &error);
  ASSERT_EQ(CL_SUCCESS, error) << " clCreateKernel failed.";

  error = clSetKernelArg(host_writer, 0, sizeof(cl_mem), &output_buffer);
  ASSERT_EQ(CL_SUCCESS, error) << " clSetKernelArg failed.";
  error = clSetKernelArg(host_writer, 1, sizeof(cl_int), &size);
  ASSERT_EQ(CL_SUCCESS, error) << " clSetKernelArg failed.";

  cl_command_queue reader_queue =
      clCreateCommandQueueWithProperties(context, device, nullptr, &error);
  ASSERT_EQ(CL_SUCCESS, error) << " clCreateCommandQueueWithProperties failed.";
  cl_command_queue writer_queue =
      clCreateCommandQueueWithProperties(context, device, nullptr, &error);
  ASSERT_EQ(CL_SUCCESS, error) << " clCreateCommandQueueWithProperties failed.";

  size_t globalSize[] = {1, 1, 1};
  size_t localSize[] = {1, 1, 1};
  error = clEnqueueNDRangeKernel(reader_queue, host_reader, 1, nullptr,
                                 globalSize, localSize, 0, nullptr, nullptr);
  ASSERT_EQ(CL_SUCCESS, error) << " clEnqueueNDRangeKernel failed.";
  error = clEnqueueNDRangeKernel(writer_queue, host_writer, 1, nullptr,
                                 globalSize, localSize, 0, nullptr, nullptr);
  ASSERT_EQ(CL_SUCCESS, error) << " clEnqueueNDRangeKernel failed.";

  std::vector<cl_int> result(size * 8);
  error = clEnqueueReadBuffer(writer_queue, output_buffer, CL_TRUE, 0,
                              sizeof(cl_int) * size * 8, &result.front(), 0,
                              nullptr, nullptr);
  ASSERT_EQ(CL_SUCCESS, error) << " clEnqueueReadBuffer failed.";

  for (cl_int i = 0; i < size; ++i) {
    ASSERT_EQ(data[i], result[8 * i + 0]) << " Data is differ at index " << i;
    ASSERT_EQ(data[i], result[8 * i + 1]) << " Data is differ at index " << i;
    ASSERT_EQ(data[i], result[8 * i + 2]) << " Data is differ at index " << i;
    ASSERT_EQ(data[i], result[8 * i + 3]) << " Data is differ at index " << i;
    ASSERT_EQ(data[i], result[8 * i + 4]) << " Data is differ at index " << i;
    ASSERT_EQ(data[i], result[8 * i + 5]) << " Data is differ at index " << i;
    ASSERT_EQ(data[i], result[8 * i + 6]) << " Data is differ at index " << i;
    ASSERT_EQ(data[i], result[8 * i + 7]) << " Data is differ at index " << i;
  }

  clReleaseCommandQueue(reader_queue);
  clReleaseCommandQueue(writer_queue);
  clReleaseMemObject(input_buffer);
  clReleaseMemObject(output_buffer);
  clReleaseKernel(host_reader);
  clReleaseKernel(host_writer);
  clReleaseProgram(program);
  clReleaseContext(context);
}

TEST(FPGA, Test_ChannelsMultipleCallSite) {
  ASSERT_NO_FATAL_FAILURE(FPGAChannelsMultipleCallSize());
}
