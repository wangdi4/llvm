#include "CL/cl.h"

#include <numeric>
#include <cstring>
#include <string>

#include "FrameworkTest.h"
#include "common_utils.h"
#include "gtest/gtest.h"

extern cl_device_type gDeviceType;

static void doChannelsTestImpl(cl_context context, cl_kernel input,
                               cl_kernel output, std::vector<cl_int> &data,
                               cl_int should_be_ok, cl_int should_be_fail) {
  cl_int error = CL_SUCCESS;
  cl_mem input_buffer = clCreateBuffer(
      context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
      data.size() * sizeof(cl_int), &data.front(), &error);
  ASSERT_EQ(CL_SUCCESS, error) << " clCreateBuffer failed.";
  cl_mem errors_buffer1 = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
                                         sizeof(cl_int), nullptr, &error);
  cl_mem errors_buffer2 = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
                                         sizeof(cl_int), nullptr, &error);

  error = clSetKernelArg(input, 0, sizeof(cl_mem), &input_buffer);
  ASSERT_EQ(CL_SUCCESS, error) << " clSetKernelArg failed.";
  error = clSetKernelArg(input, 1, sizeof(cl_int), &should_be_ok);
  ASSERT_EQ(CL_SUCCESS, error) << " clSetKernelArg failed.";
  error = clSetKernelArg(input, 2, sizeof(cl_int), &should_be_fail);
  ASSERT_EQ(CL_SUCCESS, error) << " clSetKernelArg failed.";
  error = clSetKernelArg(input, 3, sizeof(cl_mem), &errors_buffer1);
  ASSERT_EQ(CL_SUCCESS, error) << " clSetKernelArg failed.";

  std::vector<cl_int> output_data(data.size(), 0);
  cl_mem output_buffer =
      clCreateBuffer(context, CL_MEM_WRITE_ONLY,
                     sizeof(cl_int) * output_data.size(), nullptr, &error);
  ASSERT_EQ(CL_SUCCESS, error) << " clCreateBuffer failed.";

  error = clSetKernelArg(output, 0, sizeof(cl_mem), &output_buffer);
  ASSERT_EQ(CL_SUCCESS, error) << " clSetKernelArg failed.";
  error = clSetKernelArg(output, 1, sizeof(cl_int), &should_be_ok);
  ASSERT_EQ(CL_SUCCESS, error) << " clSetKernelArg failed.";
  error = clSetKernelArg(output, 2, sizeof(cl_int), &should_be_fail);
  ASSERT_EQ(CL_SUCCESS, error) << " clSetKernelArg failed.";
  error = clSetKernelArg(output, 3, sizeof(cl_mem), &errors_buffer2);
  ASSERT_EQ(CL_SUCCESS, error) << " clSetKernelArg failed.";

  cl_device_id device = nullptr;
  error = clGetContextInfo(context, CL_CONTEXT_DEVICES, sizeof(cl_device_id),
                           &device, nullptr);
  ASSERT_EQ(CL_SUCCESS, error) << " clGetContextInfo failed.";

  cl_command_queue queue =
      clCreateCommandQueueWithProperties(context, device, nullptr, &error);
  ASSERT_EQ(CL_SUCCESS, error) << " clCreateCommandQueueWithProperties failed.";

  size_t global_size[] = {1, 1, 1};
  size_t local_size[] = {1, 1, 1};
  error = clEnqueueNDRangeKernel(queue, input, 1, nullptr, global_size,
                                 local_size, 0, nullptr, nullptr);
  ASSERT_EQ(CL_SUCCESS, error) << " clEnqueueNDRangeKernel failed.";

  cl_int errors = 0;
  error = clEnqueueReadBuffer(queue, errors_buffer1, CL_TRUE, 0, sizeof(cl_int),
                              &errors, 0, nullptr, nullptr);
  ASSERT_EQ(CL_SUCCESS, error) << " clEnqueueReadBuffer failed.";
  ASSERT_EQ(0, errors) << " errors occured during execution of writer!";

  error = clEnqueueNDRangeKernel(queue, output, 1, nullptr, global_size,
                                 local_size, 0, nullptr, nullptr);
  ASSERT_EQ(CL_SUCCESS, error) << " clEnqueueNDRangeKernel failed.";

  error = clEnqueueReadBuffer(queue, errors_buffer2, CL_TRUE, 0, sizeof(cl_int),
                              &errors, 0, nullptr, nullptr);
  ASSERT_EQ(CL_SUCCESS, error) << " clEnqueueReadBuffer failed.";
  ASSERT_EQ(0, errors) << " errors occured during execution of reader!";

  error = clEnqueueReadBuffer(queue, output_buffer, CL_TRUE, 0,
                              sizeof(cl_int) * output_data.size(),
                              &output_data.front(), 0, nullptr, nullptr);
  ASSERT_EQ(CL_SUCCESS, error) << " clEnqueueReadBuffer failed.";

  for (cl_int i = 0; i < should_be_ok; ++i) {
    ASSERT_EQ(data[i], output_data[i]) << " output data is wrong!";
  }

  clReleaseCommandQueue(queue);
  clReleaseMemObject(input_buffer);
  clReleaseMemObject(output_buffer);
  clReleaseMemObject(errors_buffer1);
  clReleaseMemObject(errors_buffer2);
}

static void doChannelsTest(int depth, std::vector<cl_int> &data,
                           cl_int should_be_ok, cl_int should_be_fail) {
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
      __kernel void writer(__global int* data, int should_be_ok,             \n\
          int should_be_fail, __global int* errors) {                        \n\
        *errors = 0;                                                         \n\
        bool res = false;                                                    \n\
        for (int i = 0; i < should_be_ok; ++i) {                             \n\
          res = write_channel_nb_intel(c, data[i]);                          \n\
          if (!res) {                                                        \n\
            ++(*errors);                                                     \n\
          }                                                                  \n\
        }                                                                    \n\
        for (int i = 0; i < should_be_fail; ++i) {                           \n\
          int idx = should_be_ok + i;                                        \n\
          res = write_channel_nb_intel(c, data[idx]);                        \n\
          if (res) {                                                         \n\
            ++(*errors);                                                     \n\
          }                                                                  \n\
        }                                                                    \n\
      }                                                                      \n\
                                                                             \n\
      __attribute__((max_global_work_dim(0)))                                \n\
      __kernel void reader(__global int* data, int should_be_ok,             \n\
          int should_be_fail, __global int* errors) {                        \n\
        *errors = 0;                                                         \n\
        bool res = false;                                                    \n\
        for (int i = 0; i < should_be_ok; ++i) {                             \n\
          data[i] = read_channel_nb_intel(c, &res);                          \n\
          if (!res) {                                                        \n\
            ++(*errors);                                                     \n\
          }                                                                  \n\
        }                                                                    \n\
        for (int i = 0; i < should_be_fail; ++i) {                           \n\
          int temp = 0;                                                      \n\
          temp = read_channel_nb_intel(c, &res);                             \n\
          if (res) {                                                         \n\
            ++(*errors);                                                     \n\
          }                                                                  \n\
        }                                                                    \n\
      }                                                                      \n\
    ";

  cl_program program =
      clCreateProgramWithSource(context, 1, &program_sources, nullptr, &error);
  ASSERT_EQ(CL_SUCCESS, error) << " clCreateProgramWithSource failed.";

  std::string build_options = "";
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

  ASSERT_NO_FATAL_FAILURE(doChannelsTestImpl(context, reader, writer, data,
                                             should_be_ok, should_be_fail));

  clReleaseKernel(reader);
  clReleaseKernel(writer);
  clReleaseProgram(program);
  clReleaseContext(context);
}

static void doChannelDiagnosticTest() {
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
      channel int c;                                                         \n\
                                                                             \n\
      __attribute__((max_global_work_dim(0)))                                \n\
      __kernel void writer(__global int* data, int should_be_ok,             \n\
          int should_be_fail, __global int* errors) {                        \n\
        *errors = 0;                                                         \n\
        bool res = false;                                                    \n\
        for (int i = 0; i < should_be_ok; ++i) {                             \n\
          res = write_channel_nb_intel(c, data[i]);                          \n\
          if (!res) {                                                        \n\
            ++(*errors);                                                     \n\
          }                                                                  \n\
        }                                                                    \n\
        for (int i = 0; i < should_be_fail; ++i) {                           \n\
          int idx = should_be_ok + i;                                        \n\
          res = write_channel_nb_intel(c, data[idx]);                        \n\
          if (res) {                                                         \n\
            ++(*errors);                                                     \n\
          }                                                                  \n\
        }                                                                    \n\
      }                                                                      \n\
    ";

  cl_program program =
      clCreateProgramWithSource(context, 1, &program_sources, nullptr, &error);
  ASSERT_EQ(CL_SUCCESS, error) << " clCreateProgramWithSource failed.";

  cl_int build_error =
      clBuildProgram(program, 1, &device, "", nullptr, nullptr);
  EXPECT_EQ(CL_SUCCESS, build_error) << " clBuildProgram failed.";
  size_t log_size = 0;
  error = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0,
                                nullptr, &log_size);
  ASSERT_EQ(CL_SUCCESS, error) << " clGetProgramBuildInfo failed.";
  std::string log("", log_size);
  error = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
                                log_size, &log[0], nullptr);
  ASSERT_EQ(CL_SUCCESS, error) << " clGetProgramBuildInfo failed.";
  if (CL_SUCCESS != build_error) {
    std::cout << log << std::endl;
  }

  std::string expected_diag = "Warning: The default channel depths in the "
                              "emulation flow will be different from the "
                              "hardware flow depth (0) to speed up emulation. "
                              "The following channels are affected:\n - c";
  ASSERT_NE(std::string::npos, log.find(expected_diag))
      << " expected diagnostic message not found in build log!";

  clReleaseProgram(program);
  clReleaseContext(context);
}

static void doPipesTestImpl(cl_context context, cl_mem pipe, cl_kernel input,
                            cl_kernel output, std::vector<cl_int> &data,
                            cl_int should_be_ok, cl_int should_be_fail) {
  cl_int error = CL_SUCCESS;
  cl_mem input_buffer = clCreateBuffer(
      context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
      data.size() * sizeof(cl_int), &data.front(), &error);
  ASSERT_EQ(CL_SUCCESS, error) << " clCreateBuffer failed.";
  cl_mem errors_buffer1 = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
                                         sizeof(cl_int), nullptr, &error);
  cl_mem errors_buffer2 = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
                                         sizeof(cl_int), nullptr, &error);

  error = clSetKernelArg(input, 0, sizeof(cl_mem), &input_buffer);
  ASSERT_EQ(CL_SUCCESS, error) << " clSetKernelArg failed.";
  error = clSetKernelArg(input, 1, sizeof(cl_mem), &pipe);
  ASSERT_EQ(CL_SUCCESS, error) << " clSetKernelArg failed.";
  error = clSetKernelArg(input, 2, sizeof(cl_int), &should_be_ok);
  ASSERT_EQ(CL_SUCCESS, error) << " clSetKernelArg failed.";
  error = clSetKernelArg(input, 3, sizeof(cl_int), &should_be_fail);
  ASSERT_EQ(CL_SUCCESS, error) << " clSetKernelArg failed.";
  error = clSetKernelArg(input, 4, sizeof(cl_mem), &errors_buffer1);
  ASSERT_EQ(CL_SUCCESS, error) << " clSetKernelArg failed.";

  std::vector<cl_int> output_data(data.size(), 0);
  cl_mem output_buffer =
      clCreateBuffer(context, CL_MEM_WRITE_ONLY,
                     sizeof(cl_int) * output_data.size(), nullptr, &error);
  ASSERT_EQ(CL_SUCCESS, error) << " clCreateBuffer failed.";

  error = clSetKernelArg(output, 0, sizeof(cl_mem), &output_buffer);
  ASSERT_EQ(CL_SUCCESS, error) << " clSetKernelArg failed.";
  error = clSetKernelArg(output, 1, sizeof(cl_mem), &pipe);
  ASSERT_EQ(CL_SUCCESS, error) << " clSetKernelArg failed.";
  error = clSetKernelArg(output, 2, sizeof(cl_int), &should_be_ok);
  ASSERT_EQ(CL_SUCCESS, error) << " clSetKernelArg failed.";
  error = clSetKernelArg(output, 3, sizeof(cl_int), &should_be_fail);
  ASSERT_EQ(CL_SUCCESS, error) << " clSetKernelArg failed.";
  error = clSetKernelArg(output, 4, sizeof(cl_mem), &errors_buffer2);
  ASSERT_EQ(CL_SUCCESS, error) << " clSetKernelArg failed.";

  cl_device_id device = nullptr;
  error = clGetContextInfo(context, CL_CONTEXT_DEVICES, sizeof(cl_device_id),
                           &device, nullptr);
  ASSERT_EQ(CL_SUCCESS, error) << " clGetContextInfo failed.";

  cl_command_queue queue =
      clCreateCommandQueueWithProperties(context, device, nullptr, &error);
  ASSERT_EQ(CL_SUCCESS, error) << " clCreateCommandQueueWithProperties failed.";

  size_t global_size[] = {1, 1, 1};
  size_t local_size[] = {1, 1, 1};
  error = clEnqueueNDRangeKernel(queue, input, 1, nullptr, global_size,
                                 local_size, 0, nullptr, nullptr);
  ASSERT_EQ(CL_SUCCESS, error) << " clEnqueueNDRangeKernel failed.";

  cl_int errors = 0;
  error = clEnqueueReadBuffer(queue, errors_buffer1, CL_TRUE, 0, sizeof(cl_int),
                              &errors, 0, nullptr, nullptr);
  ASSERT_EQ(CL_SUCCESS, error) << " clEnqueueReadBuffer failed.";
  ASSERT_EQ(0, errors) << " errors occured during execution of writer!";

  error = clEnqueueNDRangeKernel(queue, output, 1, nullptr, global_size,
                                 local_size, 0, nullptr, nullptr);
  ASSERT_EQ(CL_SUCCESS, error) << " clEnqueueNDRangeKernel failed.";

  error = clEnqueueReadBuffer(queue, errors_buffer2, CL_TRUE, 0, sizeof(cl_int),
                              &errors, 0, nullptr, nullptr);
  ASSERT_EQ(CL_SUCCESS, error) << " clEnqueueReadBuffer failed.";
  ASSERT_EQ(0, errors) << " errors occured during execution of reader!";

  error = clEnqueueReadBuffer(queue, output_buffer, CL_TRUE, 0,
                              sizeof(cl_int) * output_data.size(),
                              &output_data.front(), 0, nullptr, nullptr);
  ASSERT_EQ(CL_SUCCESS, error) << " clEnqueueReadBuffer failed.";

  for (cl_int i = 0; i < should_be_ok; ++i) {
    ASSERT_EQ(data[i], output_data[i]) << " output data is wrong!";
  }

  clReleaseCommandQueue(queue);
  clReleaseMemObject(input_buffer);
  clReleaseMemObject(output_buffer);
  clReleaseMemObject(errors_buffer1);
  clReleaseMemObject(errors_buffer2);
}

static void doPipesTest(int depth, std::vector<cl_int> &data,
                        cl_int should_be_ok, cl_int should_be_fail) {
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
      #ifdef DEPTH                                                           \n\
        #define PIPE_DEPTH __attribute__((depth(DEPTH)))                     \n\
      #else                                                                  \n\
        #define PIPE_DEPTH                                                   \n\
      #endif                                                                 \n\
                                                                             \n\
      __attribute__((max_global_work_dim(0)))                                \n\
      __kernel void writer(__global int* data,                               \n\
          write_only pipe int c PIPE_DEPTH, int should_be_ok,                \n\
          int should_be_fail, __global int* errors) {                        \n\
        *errors = 0;                                                         \n\
        int res = false;                                                     \n\
        for (int i = 0; i < should_be_ok; ++i) {                             \n\
          res = write_pipe(c, &data[i]);                                     \n\
          if (res != 0) {                                                    \n\
            ++(*errors);                                                     \n\
          }                                                                  \n\
        }                                                                    \n\
        for (int i = 0; i < should_be_fail; ++i) {                           \n\
          int idx = should_be_ok + i;                                        \n\
          res = write_pipe(c, &data[idx]);                                   \n\
          if (res == 0) {                                                    \n\
            ++(*errors);                                                     \n\
          }                                                                  \n\
        }                                                                    \n\
      }                                                                      \n\
                                                                             \n\
      __attribute__((max_global_work_dim(0)))                                \n\
      __kernel void reader(__global int* data,                               \n\
          read_only pipe int c PIPE_DEPTH, int should_be_ok,                 \n\
          int should_be_fail, __global int* errors) {                        \n\
        *errors = 0;                                                         \n\
        int res = false;                                                     \n\
        for (int i = 0; i < should_be_ok; ++i) {                             \n\
          res = read_pipe(c, &data[i]);                                      \n\
          if (res != 0) {                                                    \n\
            ++(*errors);                                                     \n\
          }                                                                  \n\
        }                                                                    \n\
        for (int i = 0; i < should_be_fail; ++i) {                           \n\
          int temp = 0;                                                      \n\
          res = read_pipe(c, &temp);                                         \n\
          if (res == 0) {                                                    \n\
            ++(*errors);                                                     \n\
          }                                                                  \n\
        }                                                                    \n\
      }                                                                      \n\
    ";

  cl_program program =
      clCreateProgramWithSource(context, 1, &program_sources, nullptr, &error);
  ASSERT_EQ(CL_SUCCESS, error) << " clCreateProgramWithSource failed.";

  std::string build_options = "";
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

  cl_mem pipe =
      clCreatePipe(context, 0, sizeof(cl_int), depth, nullptr, &error);
  ASSERT_EQ(CL_SUCCESS, error) << " clCreatePipe failed.";

  ASSERT_NO_FATAL_FAILURE(doPipesTestImpl(context, pipe, reader, writer, data,
                                          should_be_ok, should_be_fail));

  clReleaseKernel(reader);
  clReleaseKernel(writer);
  clReleaseProgram(program);
  clReleaseContext(context);
}

static void FPGAChannelDepthEmulationWithoutEnvWithDepth() {
  UNSETENV("CL_CONFIG_CHANNEL_DEPTH_EMULATION_MODE");
  std::vector<cl_int> data(129);
  std::iota(data.begin(), data.end(), 0);
  ASSERT_NO_FATAL_FAILURE(doChannelsTest(128, data, 128, 1));
}

static void FPGAChannelDepthEmulationWithoutEnvWithoutDepth() {
  UNSETENV("CL_CONFIG_CHANNEL_DEPTH_EMULATION_MODE");
  std::vector<cl_int> data(2);
  std::iota(data.begin(), data.end(), 0);
  ASSERT_NO_FATAL_FAILURE(doChannelsTest(0, data, 1, 1));
}

static void FPGAChannelDepthEmulationStrictWithDepth() {
  SETENV("CL_CONFIG_CHANNEL_DEPTH_EMULATION_MODE", "strict");
  std::vector<cl_int> data(129);
  std::iota(data.begin(), data.end(), 0);
  ASSERT_NO_FATAL_FAILURE(doChannelsTest(128, data, 128, 1));
  UNSETENV("CL_CONFIG_CHANNEL_DEPTH_EMULATION_MODE");
}

static void FPGAChannelDepthEmulationStrictWithoutDepth() {
  SETENV("CL_CONFIG_CHANNEL_DEPTH_EMULATION_MODE", "strict");
  std::vector<cl_int> data(2);
  std::iota(data.begin(), data.end(), 0);
  ASSERT_NO_FATAL_FAILURE(doChannelsTest(0, data, 1, 1));
  UNSETENV("CL_CONFIG_CHANNEL_DEPTH_EMULATION_MODE");
}

static void FPGAChannelDepthEmulationDefaultWithDepth() {
  SETENV("CL_CONFIG_CHANNEL_DEPTH_EMULATION_MODE", "default");
  std::vector<cl_int> data(129);
  std::iota(data.begin(), data.end(), 0);
  ASSERT_NO_FATAL_FAILURE(doChannelsTest(128, data, 128, 1));
  UNSETENV("CL_CONFIG_CHANNEL_DEPTH_EMULATION_MODE");
}

static void FPGAChannelDepthEmulationDefaultWithoutDepth() {
  SETENV("CL_CONFIG_CHANNEL_DEPTH_EMULATION_MODE", "default");
  std::vector<cl_int> data(129);
  std::iota(data.begin(), data.end(), 0);
  ASSERT_NO_FATAL_FAILURE(doChannelsTest(0, data, 129, 0));
  UNSETENV("CL_CONFIG_CHANNEL_DEPTH_EMULATION_MODE");
}

static void FPGAChannelDepthEmulationIgnoreDepthWithDepth() {
  SETENV("CL_CONFIG_CHANNEL_DEPTH_EMULATION_MODE", "ignore-depth");
  std::vector<cl_int> data(129);
  std::iota(data.begin(), data.end(), 0);
  ASSERT_NO_FATAL_FAILURE(doChannelsTest(128, data, 129, 0));
  UNSETENV("CL_CONFIG_CHANNEL_DEPTH_EMULATION_MODE");
}

static void FPGAChannelDepthEmulationIgnoreDepthWithoutDepth() {
  SETENV("CL_CONFIG_CHANNEL_DEPTH_EMULATION_MODE", "ignore-depth");
  std::vector<cl_int> data(129);
  std::iota(data.begin(), data.end(), 0);
  ASSERT_NO_FATAL_FAILURE(doChannelsTest(0, data, 129, 0));
  UNSETENV("CL_CONFIG_CHANNEL_DEPTH_EMULATION_MODE");
}

static void FPGAChannelDepthEmulationPipesStrictWithDepth() {
  SETENV("CL_CONFIG_CHANNEL_DEPTH_EMULATION_MODE", "strict");
  std::vector<cl_int> data(129);
  std::iota(data.begin(), data.end(), 0);
  ASSERT_NO_FATAL_FAILURE(doPipesTest(128, data, 128, 1));
  UNSETENV("CL_CONFIG_CHANNEL_DEPTH_EMULATION_MODE");
}

static void FPGAChannelDepthEmulationPipesStrictWithoutDepth() {
  SETENV("CL_CONFIG_CHANNEL_DEPTH_EMULATION_MODE", "strict");
  std::vector<cl_int> data(2);
  std::iota(data.begin(), data.end(), 0);
  ASSERT_NO_FATAL_FAILURE(doPipesTest(1, data, 1, 1));
  UNSETENV("CL_CONFIG_CHANNEL_DEPTH_EMULATION_MODE");
}

static void FPGAChannelDepthEmulationPipesDefaultWithDepth() {
  SETENV("CL_CONFIG_CHANNEL_DEPTH_EMULATION_MODE", "default");
  std::vector<cl_int> data(129);
  std::iota(data.begin(), data.end(), 0);
  ASSERT_NO_FATAL_FAILURE(doPipesTest(128, data, 128, 1));
  UNSETENV("CL_CONFIG_CHANNEL_DEPTH_EMULATION_MODE");
}

static void FPGAChannelDepthEmulationPipesDefaultWithoutDepth() {
  SETENV("CL_CONFIG_CHANNEL_DEPTH_EMULATION_MODE", "default");
  std::vector<cl_int> data(129);
  std::iota(data.begin(), data.end(), 0);
  ASSERT_NO_FATAL_FAILURE(doPipesTest(1, data, 1, 128));
  UNSETENV("CL_CONFIG_CHANNEL_DEPTH_EMULATION_MODE");
}

static void FPGAChannelDepthEmulationPipesIgnoreDepthWithDepth() {
  SETENV("CL_CONFIG_CHANNEL_DEPTH_EMULATION_MODE", "ignore-depth");
  std::vector<cl_int> data(129);
  std::iota(data.begin(), data.end(), 0);
  ASSERT_NO_FATAL_FAILURE(doPipesTest(128, data, 129, 0));
  UNSETENV("CL_CONFIG_CHANNEL_DEPTH_EMULATION_MODE");
}

static void FPGAChannelDepthEmulationPipesIgnoreDepthWithoutDepth() {
  SETENV("CL_CONFIG_CHANNEL_DEPTH_EMULATION_MODE", "ignore-depth");
  std::vector<cl_int> data(129);
  std::iota(data.begin(), data.end(), 0);
  ASSERT_NO_FATAL_FAILURE(doPipesTest(1, data, 129, 0));
  UNSETENV("CL_CONFIG_CHANNEL_DEPTH_EMULATION_MODE");
}

static void FPGAChannelDepthEmulationDiagnosticMessage() {
  SETENV("CL_CONFIG_CHANNEL_DEPTH_EMULATION_MODE", "default");
  ASSERT_NO_FATAL_FAILURE(doChannelDiagnosticTest());
  UNSETENV("CL_CONFIG_CHANNEL_DEPTH_EMULATION_MODE");
}

#ifdef BUILD_FPGA_EMULATOR
TEST(FPGA, Test_ChannelDepthEmulationWithoutEnv)
{
    ASSERT_NO_FATAL_FAILURE(FPGAChannelDepthEmulationWithoutEnvWithDepth());
    ASSERT_NO_FATAL_FAILURE(FPGAChannelDepthEmulationWithoutEnvWithoutDepth());
}

TEST(FPGA, Test_ChannelDepthEmulationStrict)
{
    ASSERT_NO_FATAL_FAILURE(FPGAChannelDepthEmulationStrictWithDepth());
    ASSERT_NO_FATAL_FAILURE(FPGAChannelDepthEmulationStrictWithoutDepth());
    ASSERT_NO_FATAL_FAILURE(FPGAChannelDepthEmulationPipesStrictWithDepth());
    ASSERT_NO_FATAL_FAILURE(FPGAChannelDepthEmulationPipesStrictWithoutDepth());
}

TEST(FPGA, Test_ChannelDepthEmulationDefault)
{
    ASSERT_NO_FATAL_FAILURE(FPGAChannelDepthEmulationDefaultWithDepth());
    ASSERT_NO_FATAL_FAILURE(FPGAChannelDepthEmulationDefaultWithoutDepth());
    ASSERT_NO_FATAL_FAILURE(FPGAChannelDepthEmulationPipesDefaultWithDepth());
    ASSERT_NO_FATAL_FAILURE(
        FPGAChannelDepthEmulationPipesDefaultWithoutDepth());
    ASSERT_NO_FATAL_FAILURE(FPGAChannelDepthEmulationDiagnosticMessage());
}

TEST(FPGA, Test_ChannelDepthEmulationIgnoreDepth)
{
    ASSERT_NO_FATAL_FAILURE(FPGAChannelDepthEmulationIgnoreDepthWithDepth());
    ASSERT_NO_FATAL_FAILURE(FPGAChannelDepthEmulationIgnoreDepthWithoutDepth());
    ASSERT_NO_FATAL_FAILURE(
        FPGAChannelDepthEmulationPipesIgnoreDepthWithDepth());
    ASSERT_NO_FATAL_FAILURE(
        FPGAChannelDepthEmulationPipesIgnoreDepthWithoutDepth());
}
#endif // BUILD_FPGA_EMULATOR
