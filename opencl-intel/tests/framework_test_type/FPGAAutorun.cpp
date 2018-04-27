#include "CL/cl.h"

#include "FrameworkTest.h"
#include "gtest/gtest.h"

#include <algorithm>
#include <numeric>
#include <string>
#include <vector>

extern cl_device_type gDeviceType;

class FPGAAutorun : public ::testing::Test {
protected:
  void SetUp() {
    cl_int error = CL_SUCCESS;
    error = clGetPlatformIDs(1, &m_platform, nullptr);
    ASSERT_EQ(CL_SUCCESS, error) << " clGetPlatformIDs failed.";

    error = clGetDeviceIDs(m_platform, gDeviceType, 1, &m_device, nullptr);
    ASSERT_EQ(CL_SUCCESS, error) << " clGetDeviceIDs failed to obtain device "
                                 << "of " << gDeviceType << " type.";

    const cl_context_properties props[5] = {
        CL_CONTEXT_PLATFORM,
        (cl_context_properties)m_platform,
        CL_CONTEXT_FPGA_EMULATOR_INTEL,
        CL_TRUE,
        0
    };

    m_context = clCreateContext(props, 1, &m_device, nullptr, nullptr, &error);
    ASSERT_EQ(CL_SUCCESS, error) << " clCreateContext failed.";
    m_reader_queue = clCreateCommandQueueWithProperties(m_context, m_device,
                                                        nullptr, &error);
    ASSERT_EQ(CL_SUCCESS, error)
        << " clCreateCommandQueueWithProperties failed.";
    m_writer_queue = clCreateCommandQueueWithProperties(m_context, m_device,
                                                        nullptr, &error);
    ASSERT_EQ(CL_SUCCESS, error)
        << " clCreateCommandQueueWithProperties failed.";
  }

  void CreateAndBuildProgram(const std::string &program_sources,
                             const std::string &build_options) {
    cl_int error = CL_SUCCESS;
    const char *psources = program_sources.c_str();
    m_program =
        clCreateProgramWithSource(m_context, 1, &psources, nullptr, &error);
    ASSERT_EQ(CL_SUCCESS, error) << " clCreateProgramWithSource failed.";
    error = clBuildProgram(m_program, 1, &m_device, build_options.c_str(),
                           nullptr, nullptr);
    EXPECT_EQ(CL_SUCCESS, error) << " clBuildProgram failed.";
    if (CL_SUCCESS != error) {
      size_t logSize = 0;
      error = clGetProgramBuildInfo(m_program, m_device, CL_PROGRAM_BUILD_LOG,
                                    0, nullptr, &logSize);
      ASSERT_EQ(CL_SUCCESS, error) << " clGetProgramBuildInfo failed.";
      std::string log("", logSize);
      error = clGetProgramBuildInfo(m_program, m_device, CL_PROGRAM_BUILD_LOG,
                                    logSize, &log[0], nullptr);
      ASSERT_EQ(CL_SUCCESS, error) << " clGetProgramBuildInfo failed.";
      std::cout << log << std::endl;
      return;
    }
  }

  void LaunchKernels(std::vector<cl_int> &input_data) {
    cl_int error = CL_SUCCESS;
    m_reader = clCreateKernel(m_program, "reader", &error);
    ASSERT_EQ(CL_SUCCESS, error) << " clCreateKernel failed.";
    m_writer = clCreateKernel(m_program, "writer", &error);
    ASSERT_EQ(CL_SUCCESS, error) << " clCreateKernel failed.";

    cl_mem input_buffer = clCreateBuffer(
        m_context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        input_data.size() * sizeof(cl_int), &input_data.front(), &error);
    ASSERT_EQ(CL_SUCCESS, error) << " clCreateBuffer failed.";

    m_output_buffer =
        clCreateBuffer(m_context, CL_MEM_WRITE_ONLY,
                       input_data.size() * sizeof(cl_int), nullptr, &error);
    ASSERT_EQ(CL_SUCCESS, error) << " clCreateBuffer failed.";

    cl_int num_elements = (cl_int)input_data.size();
    error = clSetKernelArg(m_reader, 0, sizeof(cl_int), &num_elements);
    ASSERT_EQ(CL_SUCCESS, error) << " clSetKernelArg failed.";
    error = clSetKernelArg(m_reader, 1, sizeof(cl_mem), &m_output_buffer);
    ASSERT_EQ(CL_SUCCESS, error) << " clSetKernelArg failed.";

    error = clSetKernelArg(m_writer, 0, sizeof(cl_int), &num_elements);
    ASSERT_EQ(CL_SUCCESS, error) << " clSetKernelArg failed.";
    error = clSetKernelArg(m_writer, 1, sizeof(cl_mem), &input_buffer);
    ASSERT_EQ(CL_SUCCESS, error) << " clSetKernelArg failed.";

    size_t global_size[] = {1, 1, 1};
    size_t local_size[] = {1, 1, 1};
    error =
        clEnqueueNDRangeKernel(m_writer_queue, m_writer, 1, nullptr,
                               global_size, local_size, 0, nullptr, nullptr);
    ASSERT_EQ(CL_SUCCESS, error) << " clEnqueueNDRangeKernel failed.";
    error =
        clEnqueueNDRangeKernel(m_reader_queue, m_reader, 1, nullptr,
                               global_size, local_size, 0, nullptr, nullptr);
    ASSERT_EQ(CL_SUCCESS, error) << " clEnqueueNDRangeKernel failed.";

    clFinish(m_reader_queue);
    clReleaseMemObject(input_buffer);
  }

  void VerifyResults(std::vector<cl_int> &reference_data) {
    cl_int error = CL_SUCCESS;
    std::vector<cl_int> output_data(reference_data.size(), 0);
    error = clEnqueueReadBuffer(m_reader_queue, m_output_buffer, CL_TRUE, 0,
                                output_data.size() * sizeof(cl_int),
                                &output_data.front(), 0, nullptr, nullptr);

    for (size_t i = 0; i < reference_data.size(); ++i) {
      ASSERT_EQ(reference_data[i], output_data[i])
          << " invalid value of " << i << "-th"
          << " element of resulting array";
    }
  }

  void TearDown() {
    clReleaseContext(m_context);
    clReleaseCommandQueue(m_reader_queue);
    clReleaseCommandQueue(m_writer_queue);
    clReleaseProgram(m_program);
    clReleaseKernel(m_reader);
    clReleaseKernel(m_writer);
    clReleaseMemObject(m_output_buffer);
  }

  cl_platform_id m_platform;
  cl_device_id m_device;
  cl_context m_context;
  cl_command_queue m_reader_queue;
  cl_command_queue m_writer_queue;
  cl_program m_program;
  cl_kernel m_reader;
  cl_kernel m_writer;
  cl_mem m_output_buffer;
};

#ifdef BUILD_FPGA_EMULATOR
TEST_F(FPGAAutorun, SWIWithoutReplication) {
  std::string program_sources = "                                            \n\
      #pragma OPENCL EXTENSION cl_intel_channels: enable                     \n\
      channel int in, out;                                                   \n\
                                                                             \n\
      __attribute__((max_global_work_dim(0)))                                \n\
      __attribute__((autorun))                                               \n\
      __kernel void single_plus() {                                          \n\
        int a = read_channel_intel(in);                                      \n\
        write_channel_intel(out, a + 1);                                     \n\
      }                                                                      \n\
                                                                             \n\
      __kernel void reader(int n, __global int* data) {                      \n\
        for (int i = 0; i < n; ++i) {                                        \n\
          data[i] = read_channel_intel(out);                                 \n\
        }                                                                    \n\
      }                                                                      \n\
                                                                             \n\
      __kernel void writer(int n, __global int* data) {                      \n\
        for (int i = 0; i < n; ++i) {                                        \n\
          write_channel_intel(in, data[i]);                                  \n\
        }                                                                    \n\
      }                                                                      \n\
  ";

  ASSERT_NO_FATAL_FAILURE(CreateAndBuildProgram(program_sources, ""));

  std::vector<cl_int> input_data(1000);
  std::iota(input_data.begin(), input_data.end(), 0);

  ASSERT_NO_FATAL_FAILURE(LaunchKernels(input_data));

  std::vector<cl_int> reference_data;
  std::transform(input_data.begin(), input_data.end(),
                 std::back_inserter(reference_data),
                 [](cl_int v) { return v + 1; });

  ASSERT_NO_FATAL_FAILURE(VerifyResults(reference_data));
}

TEST_F(FPGAAutorun, SWIWithReplication) {
  std::string program_sources = "                                            \n\
      #pragma OPENCL EXTENSION cl_intel_channels: enable                     \n\
      channel int arr[N + 1];                                                \n\
                                                                             \n\
      __attribute__((max_global_work_dim(0)))                                \n\
      __attribute__((num_compute_units(N, 1, 1)))                            \n\
      __attribute__((autorun))                                               \n\
      __kernel void chained_plus() {                                         \n\
        int a = read_channel_intel(arr[get_compute_id(0)]);                  \n\
        write_channel_intel(arr[get_compute_id(0) + 1], a + 1);              \n\
      }                                                                      \n\
                                                                             \n\
      __kernel void reader(int n, __global int* data) {                      \n\
        for (int i = 0; i < n; ++i) {                                        \n\
          data[i] = read_channel_intel(arr[N]);                              \n\
        }                                                                    \n\
      }                                                                      \n\
                                                                             \n\
      __kernel void writer(int n, __global int* data) {                      \n\
        for (int i = 0; i < n; ++i) {                                        \n\
          write_channel_intel(arr[0], data[i]);                              \n\
        }                                                                    \n\
      }                                                                      \n\
  ";

  const int N = 5;
  ASSERT_NO_FATAL_FAILURE(
      CreateAndBuildProgram(program_sources, "-DN=" + std::to_string(N)));

  std::vector<cl_int> input_data(1000);
  std::iota(input_data.begin(), input_data.end(), 0);

  ASSERT_NO_FATAL_FAILURE(LaunchKernels(input_data));

  std::vector<cl_int> reference_data;
  std::transform(input_data.begin(), input_data.end(),
                 std::back_inserter(reference_data),
                 [N](cl_int v) { return v + N; });

  ASSERT_NO_FATAL_FAILURE(VerifyResults(reference_data));
}

TEST_F(FPGAAutorun, SWG111WithoutReplication) {
  std::string program_sources = "                                            \n\
      #pragma OPENCL EXTENSION cl_intel_channels: enable                     \n\
      channel int in, out;                                                   \n\
                                                                             \n\
      __attribute__((reqd_work_group_size(1, 1, 1)))                         \n\
      __attribute__((autorun))                                               \n\
      __kernel void single_plus() {                                          \n\
        int a = read_channel_intel(in);                                      \n\
        write_channel_intel(out, a + 1);                                     \n\
      }                                                                      \n\
                                                                             \n\
      __kernel void reader(int n, __global int* data) {                      \n\
        for (int i = 0; i < n; ++i) {                                        \n\
          data[i] = read_channel_intel(out);                                 \n\
        }                                                                    \n\
      }                                                                      \n\
                                                                             \n\
      __kernel void writer(int n, __global int* data) {                      \n\
        for (int i = 0; i < n; ++i) {                                        \n\
          write_channel_intel(in, data[i]);                                  \n\
        }                                                                    \n\
      }                                                                      \n\
  ";

  ASSERT_NO_FATAL_FAILURE(CreateAndBuildProgram(program_sources, ""));

  std::vector<cl_int> input_data(1000);
  std::iota(input_data.begin(), input_data.end(), 0);

  ASSERT_NO_FATAL_FAILURE(LaunchKernels(input_data));

  std::vector<cl_int> reference_data;
  std::transform(input_data.begin(), input_data.end(),
                 std::back_inserter(reference_data),
                 [](cl_int v) { return v + 1; });

  ASSERT_NO_FATAL_FAILURE(VerifyResults(reference_data));
}

TEST_F(FPGAAutorun, SWG111WithReplication) {
  std::string program_sources = "                                            \n\
      #pragma OPENCL EXTENSION cl_intel_channels: enable                     \n\
      channel int arr[N + 1];                                                \n\
                                                                             \n\
      __attribute__((reqd_work_group_size(1, 1, 1)))                         \n\
      __attribute__((num_compute_units(N, 1, 1)))                            \n\
      __attribute__((autorun))                                               \n\
      __kernel void chained_plus() {                                         \n\
        int a = read_channel_intel(arr[get_compute_id(0)]);                  \n\
        write_channel_intel(arr[get_compute_id(0) + 1], a + 1);              \n\
      }                                                                      \n\
                                                                             \n\
      __kernel void reader(int n, __global int* data) {                      \n\
        for (int i = 0; i < n; ++i) {                                        \n\
          data[i] = read_channel_intel(arr[N]);                              \n\
        }                                                                    \n\
      }                                                                      \n\
                                                                             \n\
      __kernel void writer(int n, __global int* data) {                      \n\
        for (int i = 0; i < n; ++i) {                                        \n\
          write_channel_intel(arr[0], data[i]);                              \n\
        }                                                                    \n\
      }                                                                      \n\
  ";

  const int N = 5;
  ASSERT_NO_FATAL_FAILURE(
      CreateAndBuildProgram(program_sources, "-DN=" + std::to_string(N)));

  std::vector<cl_int> input_data(1000);
  std::iota(input_data.begin(), input_data.end(), 0);

  ASSERT_NO_FATAL_FAILURE(LaunchKernels(input_data));

  std::vector<cl_int> reference_data;
  std::transform(input_data.begin(), input_data.end(),
                 std::back_inserter(reference_data),
                 [N](cl_int v) { return v + N; });

  ASSERT_NO_FATAL_FAILURE(VerifyResults(reference_data));
}

TEST_F(FPGAAutorun, SWG811WithoutReplication) {
  std::string program_sources = "                                            \n\
      #pragma OPENCL EXTENSION cl_intel_channels: enable                     \n\
      channel int in, out;                                                   \n\
                                                                             \n\
      __attribute__((reqd_work_group_size(W, 1, 1)))                         \n\
      __attribute__((autorun))                                               \n\
      __kernel void single_plus() {                                          \n\
        int a = read_channel_intel(in);                                      \n\
        write_channel_intel(out, a + get_local_id(0));                       \n\
      }                                                                      \n\
                                                                             \n\
      __kernel void reader(int n, __global int* data) {                      \n\
        for (int i = 0; i < n; ++i) {                                        \n\
          data[i] = read_channel_intel(out);                                 \n\
        }                                                                    \n\
      }                                                                      \n\
                                                                             \n\
      __kernel void writer(int n, __global int* data) {                      \n\
        for (int i = 0; i < n; ++i) {                                        \n\
          write_channel_intel(in, data[i]);                                  \n\
        }                                                                    \n\
      }                                                                      \n\
  ";

  const int W = 8;
  ASSERT_NO_FATAL_FAILURE(
      CreateAndBuildProgram(program_sources, "-DW=" + std::to_string(W)));

  std::vector<cl_int> input_data(1000);
  std::iota(input_data.begin(), input_data.end(), 0);

  ASSERT_NO_FATAL_FAILURE(LaunchKernels(input_data));

  std::vector<cl_int> reference_data;
  std::transform(input_data.begin(), input_data.end(),
                 std::back_inserter(reference_data), [W](cl_int v) {
                   static cl_int local_id = 0;
                   return v + (local_id++ % W);
                 });

  ASSERT_NO_FATAL_FAILURE(VerifyResults(reference_data));
}

TEST_F(FPGAAutorun, SWG811WithReplication) {
  std::string program_sources = "                                            \n\
      #pragma OPENCL EXTENSION cl_intel_channels: enable                     \n\
      channel int arr[N + 1];                                                \n\
                                                                             \n\
      __attribute__((reqd_work_group_size(W, 1, 1)))                         \n\
      __attribute__((num_compute_units(N, 1, 1)))                            \n\
      __attribute__((autorun))                                               \n\
      __kernel void chained_plus() {                                         \n\
        int a = read_channel_intel(arr[get_compute_id(0)]);                  \n\
        write_channel_intel(arr[get_compute_id(0) + 1], a + get_local_id(0));\n\
      }                                                                      \n\
                                                                             \n\
      __kernel void reader(int n, __global int* data) {                      \n\
        for (int i = 0; i < n; ++i) {                                        \n\
          data[i] = read_channel_intel(arr[N]);                              \n\
        }                                                                    \n\
      }                                                                      \n\
                                                                             \n\
      __kernel void writer(int n, __global int* data) {                      \n\
        for (int i = 0; i < n; ++i) {                                        \n\
          write_channel_intel(arr[0], data[i]);                              \n\
        }                                                                    \n\
      }                                                                      \n\
  ";

  const int N = 5;
  const int W = 8;
  ASSERT_NO_FATAL_FAILURE(
      CreateAndBuildProgram(program_sources, "-DN=" + std::to_string(N) +
                                                 " -DW=" + std::to_string(W)));

  std::vector<cl_int> input_data(1000);
  std::iota(input_data.begin(), input_data.end(), 0);

  ASSERT_NO_FATAL_FAILURE(LaunchKernels(input_data));

  std::vector<cl_int> reference_data;
  std::transform(input_data.begin(), input_data.end(),
                 std::back_inserter(reference_data), [N, W](cl_int v) {
                   static cl_int local_id = 0;
                   return v + N * (local_id++ % W);
                 });

  ASSERT_NO_FATAL_FAILURE(VerifyResults(reference_data));
}

TEST_F(FPGAAutorun, SWG811WithWhileTrueWithoutReplication) {
  std::string program_sources = "                                            \n\
      #pragma OPENCL EXTENSION cl_intel_channels: enable                     \n\
      channel int in, out;                                                   \n\
                                                                             \n\
      __attribute__((reqd_work_group_size(W, 1, 1)))                         \n\
      __attribute__((autorun))                                               \n\
      __kernel void single_plus() {                                          \n\
        while (true) {                                                       \n\
          int a = read_channel_intel(in);                                    \n\
          write_channel_intel(out, a + get_local_id(0));                     \n\
        }                                                                    \n\
      }                                                                      \n\
                                                                             \n\
      __kernel void reader(int n, __global int* data) {                      \n\
        for (int i = 0; i < n; ++i) {                                        \n\
          data[i] = read_channel_intel(out);                                 \n\
        }                                                                    \n\
      }                                                                      \n\
                                                                             \n\
      __kernel void writer(int n, __global int* data) {                      \n\
        for (int i = 0; i < n; ++i) {                                        \n\
          write_channel_intel(in, data[i]);                                  \n\
        }                                                                    \n\
      }                                                                      \n\
  ";

  const int W = 8;
  ASSERT_NO_FATAL_FAILURE(
      CreateAndBuildProgram(program_sources, "-DW=" + std::to_string(W)));

  std::vector<cl_int> input_data(1000);
  std::iota(input_data.begin(), input_data.end(), 0);

  ASSERT_NO_FATAL_FAILURE(LaunchKernels(input_data));

  std::vector<cl_int> reference_data;
  std::transform(input_data.begin(), input_data.end(),
                 std::back_inserter(reference_data), [W](cl_int v) {
                   static cl_int local_id = 0;
                   return v + (local_id++ % W);
                 });

  ASSERT_NO_FATAL_FAILURE(VerifyResults(reference_data));
}

TEST_F(FPGAAutorun, SWG811WithWhileTrueWithReplication) {
  std::string program_sources = "                                            \n\
      #pragma OPENCL EXTENSION cl_intel_channels: enable                     \n\
      channel int arr[N + 1];                                                \n\
                                                                             \n\
      __attribute__((reqd_work_group_size(W, 1, 1)))                         \n\
      __attribute__((num_compute_units(N, 1, 1)))                            \n\
      __attribute__((autorun))                                               \n\
      __kernel void chained_plus() {                                         \n\
        while (true) {                                                       \n\
          int a = read_channel_intel(arr[get_compute_id(0)]);                \n\
          write_channel_intel(                                               \n\
              arr[get_compute_id(0) + 1], a + get_local_id(0));              \n\
        }                                                                    \n\
      }                                                                      \n\
                                                                             \n\
      __kernel void reader(int n, __global int* data) {                      \n\
        for (int i = 0; i < n; ++i) {                                        \n\
          data[i] = read_channel_intel(arr[N]);                              \n\
        }                                                                    \n\
      }                                                                      \n\
                                                                             \n\
      __kernel void writer(int n, __global int* data) {                      \n\
        for (int i = 0; i < n; ++i) {                                        \n\
          write_channel_intel(arr[0], data[i]);                              \n\
        }                                                                    \n\
      }                                                                      \n\
  ";

  const int N = 5;
  const int W = 8;
  ASSERT_NO_FATAL_FAILURE(
      CreateAndBuildProgram(program_sources, "-DN=" + std::to_string(N) +
                                                 " -DW=" + std::to_string(W)));

  std::vector<cl_int> input_data(1000);
  std::iota(input_data.begin(), input_data.end(), 0);

  ASSERT_NO_FATAL_FAILURE(LaunchKernels(input_data));

  std::vector<cl_int> reference_data;
  std::transform(input_data.begin(), input_data.end(),
                 std::back_inserter(reference_data), [N, W](cl_int v) {
                   static cl_int local_id = 0;
                   return v + N * (local_id++ % W);
                 });

  ASSERT_NO_FATAL_FAILURE(VerifyResults(reference_data));
}
#endif // BUILD_FPGA_EMULATOR
