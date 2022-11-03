//===--- autorun.cpp -                                          -*- C++ -*-===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
//
// Internal tests for autorun kernels feature
//
// ===--------------------------------------------------------------------=== //
#include "simple_fixture.h"

#include "gtest_wrapper.h"
#include <CL/cl.h>

#include <algorithm>
#include <numeric>
#include <string>
#include <vector>

class TestAutorun : public OCLFPGASimpleFixture {
protected:
  void launchTest(std::vector<cl_int> &input_data) {
    cl_mem input_buffer = createBuffer<cl_int>(
        input_data.size(), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        &input_data.front());
    ASSERT_NE(nullptr, input_buffer) << "createBuffer failed";
    m_output_buffer =
        createBuffer<cl_int>(input_data.size(), CL_MEM_WRITE_ONLY);
    ASSERT_NE(nullptr, m_output_buffer) << "createBuffer failed";

    cl_int num_elements = input_data.size();
    ASSERT_TRUE(enqueueTask("writer", num_elements, input_buffer))
        << "enqueueTask failed";
    ASSERT_TRUE(enqueueTask("reader", num_elements, m_output_buffer))
        << "enqueueTask failed";
  }

  void verifyResults(const std::vector<cl_int> &reference_data) {
    std::vector<cl_int> data(reference_data.size());
    ASSERT_TRUE(readBuffer<cl_int>("reader", m_output_buffer, data.size(),
                                   &data.front()))
        << "readBuffer failed";
    for (size_t i = 0; i < reference_data.size(); ++i) {
      ASSERT_EQ(reference_data[i], data[i])
          << " invalid value of " << i << "-th"
          << " element of resulting array";
    }
  }

private:
  cl_mem m_output_buffer = nullptr;
};

TEST_F(TestAutorun, SWIWithoutReplication) {
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

  ASSERT_TRUE(createAndBuildProgram(program_sources))
      << "createAndBuildProgram failed";

  std::vector<cl_int> input_data(1000);
  std::iota(input_data.begin(), input_data.end(), 0);

  ASSERT_NO_FATAL_FAILURE(launchTest(input_data));

  std::vector<cl_int> reference_data;
  std::transform(input_data.begin(), input_data.end(),
                 std::back_inserter(reference_data),
                 [](cl_int v) { return v + 1; });

  ASSERT_NO_FATAL_FAILURE(verifyResults(reference_data));
}

TEST_F(TestAutorun, SWIWithReplication) {
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
  ASSERT_TRUE(
      createAndBuildProgram(program_sources, "-DN=" + std::to_string(N)))
      << "createAndBuildProgram failed";

  std::vector<cl_int> input_data(1000);
  std::iota(input_data.begin(), input_data.end(), 0);

  ASSERT_NO_FATAL_FAILURE(launchTest(input_data));

  std::vector<cl_int> reference_data;
  std::transform(input_data.begin(), input_data.end(),
                 std::back_inserter(reference_data),
                 [](cl_int v) { return v + N; });

  ASSERT_NO_FATAL_FAILURE(verifyResults(reference_data));
}

TEST_F(TestAutorun, SWG111WithoutReplication) {
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

  ASSERT_TRUE(createAndBuildProgram(program_sources))
      << "createAndBuildProgram failed";

  std::vector<cl_int> input_data(1000);
  std::iota(input_data.begin(), input_data.end(), 0);

  ASSERT_NO_FATAL_FAILURE(launchTest(input_data));

  std::vector<cl_int> reference_data;
  std::transform(input_data.begin(), input_data.end(),
                 std::back_inserter(reference_data),
                 [](cl_int v) { return v + 1; });

  ASSERT_NO_FATAL_FAILURE(verifyResults(reference_data));
}

TEST_F(TestAutorun, SWG111WithReplication) {
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
  ASSERT_TRUE(
      createAndBuildProgram(program_sources, "-DN=" + std::to_string(N)))
      << "createAndBuildProgram failed";

  std::vector<cl_int> input_data(1000);
  std::iota(input_data.begin(), input_data.end(), 0);

  ASSERT_NO_FATAL_FAILURE(launchTest(input_data));

  std::vector<cl_int> reference_data;
  std::transform(input_data.begin(), input_data.end(),
                 std::back_inserter(reference_data),
                 [](cl_int v) { return v + N; });

  ASSERT_NO_FATAL_FAILURE(verifyResults(reference_data));
}

TEST_F(TestAutorun, SWG811WithoutReplication) {
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
  ASSERT_TRUE(
      createAndBuildProgram(program_sources, "-DW=" + std::to_string(W)))
      << "createAndBuildProgram failed";

  std::vector<cl_int> input_data(1000);
  std::iota(input_data.begin(), input_data.end(), 0);

  ASSERT_NO_FATAL_FAILURE(launchTest(input_data));

  std::vector<cl_int> reference_data;
  std::transform(input_data.begin(), input_data.end(),
                 std::back_inserter(reference_data), [](cl_int v) {
                   static cl_int local_id = 0;
                   return v + (local_id++ % W);
                 });

  ASSERT_NO_FATAL_FAILURE(verifyResults(reference_data));
}

TEST_F(TestAutorun, SWG811WithReplication) {
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
  ASSERT_TRUE(
      createAndBuildProgram(program_sources, "-DN=" + std::to_string(N) +
                                                 " -DW=" + std::to_string(W)))
      << "createAndBuildProgram failed";

  std::vector<cl_int> input_data(1000);
  std::iota(input_data.begin(), input_data.end(), 0);

  ASSERT_NO_FATAL_FAILURE(launchTest(input_data));

  std::vector<cl_int> reference_data;
  std::transform(input_data.begin(), input_data.end(),
                 std::back_inserter(reference_data), [](cl_int v) {
                   static cl_int local_id = 0;
                   return v + N * (local_id++ % W);
                 });

  ASSERT_NO_FATAL_FAILURE(verifyResults(reference_data));
}

TEST_F(TestAutorun, SWG811WithWhileTrueWithoutReplication) {
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
  ASSERT_TRUE(
      createAndBuildProgram(program_sources, "-DW=" + std::to_string(W)))
      << "createAndBuildProgram failed";

  std::vector<cl_int> input_data(1000);
  std::iota(input_data.begin(), input_data.end(), 0);

  ASSERT_NO_FATAL_FAILURE(launchTest(input_data));

  std::vector<cl_int> reference_data;
  std::transform(input_data.begin(), input_data.end(),
                 std::back_inserter(reference_data), [](cl_int v) {
                   static cl_int local_id = 0;
                   return v + (local_id++ % W);
                 });

  ASSERT_NO_FATAL_FAILURE(verifyResults(reference_data));
}

TEST_F(TestAutorun, SWG811WithWhileTrueWithReplication) {
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
  ASSERT_TRUE(
      createAndBuildProgram(program_sources, "-DN=" + std::to_string(N) +
                                                 " -DW=" + std::to_string(W)))
      << "createAndBuildProgram failed";

  std::vector<cl_int> input_data(1000);
  std::iota(input_data.begin(), input_data.end(), 0);

  ASSERT_NO_FATAL_FAILURE(launchTest(input_data));

  std::vector<cl_int> reference_data;
  std::transform(input_data.begin(), input_data.end(),
                 std::back_inserter(reference_data), [](cl_int v) {
                   static cl_int local_id = 0;
                   return v + N * (local_id++ % W);
                 });

  ASSERT_NO_FATAL_FAILURE(verifyResults(reference_data));
}

TEST_F(TestAutorun, SWG811WithoutReplicationCheckGID) {
  std::string program_sources = "                                            \n\
      #pragma OPENCL EXTENSION cl_intel_channels: enable                     \n\
      channel int in, out;                                                   \n\
                                                                             \n\
      size_t get_global_linear_id() {                                        \n\
        return ((get_global_id(2) - get_global_offset(2)) *                  \n\
            get_global_size(1) * get_global_size(0)) +                       \n\
            ((get_global_id(1) - get_global_offset(1)) *                     \n\
            get_global_size(0)) + (get_global_id(0) -                        \n\
            get_global_offset(0));                                           \n\
      }                                                                      \n\
                                                                             \n\
      __attribute__((reqd_work_group_size(W, 1, 1)))                         \n\
      __attribute__((autorun))                                               \n\
      __kernel void single_plus() {                                          \n\
        int a = read_channel_intel(in);                                      \n\
        write_channel_intel(out, a + get_global_linear_id());                \n\
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
  ASSERT_TRUE(
      createAndBuildProgram(program_sources, "-DW=" + std::to_string(W)))
      << "createAndBuildProgram failed";

  std::vector<cl_int> input_data(1000);
  std::iota(input_data.begin(), input_data.end(), 0);

  ASSERT_NO_FATAL_FAILURE(launchTest(input_data));

  std::vector<cl_int> reference_data;
  std::transform(input_data.begin(), input_data.end(),
                 std::back_inserter(reference_data), [](cl_int v) {
                   static cl_int global_linear_id = 0;
                   return v + global_linear_id++;
                 });

  ASSERT_NO_FATAL_FAILURE(verifyResults(reference_data));
}

TEST_F(TestAutorun, SWG811WithReplicationCheckGID) {
  std::string program_sources = "                                            \n\
      #pragma OPENCL EXTENSION cl_intel_channels: enable                     \n\
      channel int arr[N + 1];                                                \n\
                                                                             \n\
      size_t get_global_linear_id() {                                        \n\
        return ((get_global_id(2) - get_global_offset(2)) *                  \n\
            get_global_size(1) * get_global_size(0)) +                       \n\
            ((get_global_id(1) - get_global_offset(1)) *                     \n\
            get_global_size(0)) + (get_global_id(0) -                        \n\
            get_global_offset(0));                                           \n\
      }                                                                      \n\
                                                                             \n\
      __attribute__((reqd_work_group_size(W, 1, 1)))                         \n\
      __attribute__((num_compute_units(N, 1, 1)))                            \n\
      __attribute__((autorun))                                               \n\
      __kernel void chained_plus() {                                         \n\
        int a = read_channel_intel(arr[get_compute_id(0)]);                  \n\
        write_channel_intel(arr[get_compute_id(0) + 1],                      \n\
            a + get_global_linear_id());                                     \n\
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

  ASSERT_TRUE(
      createAndBuildProgram(program_sources, "-DN=" + std::to_string(N) +
                                                 " -DW=" + std::to_string(W)))
      << "createAndBuildProgram failed";

  std::vector<cl_int> input_data(1000);
  std::iota(input_data.begin(), input_data.end(), 0);

  ASSERT_NO_FATAL_FAILURE(launchTest(input_data));

  std::vector<cl_int> reference_data;
  std::transform(input_data.begin(), input_data.end(),
                 std::back_inserter(reference_data), [](cl_int v) {
                   static cl_int global_linear_id = 0;
                   return v + N * (global_linear_id++);
                 });

  ASSERT_NO_FATAL_FAILURE(verifyResults(reference_data));
}
