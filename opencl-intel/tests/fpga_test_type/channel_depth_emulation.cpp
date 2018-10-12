//===--- channel_depth_emulation.cpp -                          -*- C++ -*-===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
//
// Internal tests for Channel depth emulation feature
//
// ===--------------------------------------------------------------------=== //

#include "common_utils.h"
#include "simple_fixture.h"

#include <CL/cl.h>
#include <gtest/gtest.h>

#include <cstring>
#include <numeric>
#include <string>

class TestChannelDepthEmulation : public OCLFPGASimpleFixture {
protected:
  void doChannelsTest(int depth, std::vector<cl_int> &input,
                      cl_int should_be_ok, cl_int should_be_fail) {
    const std::string program_sources = "                                    \n\
        #pragma OPENCL EXTENSION cl_intel_channels: enable                   \n\
        #ifdef DEPTH                                                         \n\
          channel int c __attribute__((depth(DEPTH)));                       \n\
        #else                                                                \n\
          channel int c;                                                     \n\
        #endif                                                               \n\
                                                                             \n\
        __attribute__((max_global_work_dim(0)))                              \n\
        __kernel void writer(__global int* data, int should_be_ok,           \n\
            int should_be_fail, __global int* errors) {                      \n\
          *errors = 0;                                                       \n\
          bool res = false;                                                  \n\
          for (int i = 0; i < should_be_ok; ++i) {                           \n\
            res = write_channel_nb_intel(c, data[i]);                        \n\
            if (!res) {                                                      \n\
              ++(*errors);                                                   \n\
            }                                                                \n\
          }                                                                  \n\
          for (int i = 0; i < should_be_fail; ++i) {                         \n\
            int idx = should_be_ok + i;                                      \n\
            res = write_channel_nb_intel(c, data[idx]);                      \n\
            if (res) {                                                       \n\
              ++(*errors);                                                   \n\
            }                                                                \n\
          }                                                                  \n\
        }                                                                    \n\
                                                                             \n\
        __attribute__((max_global_work_dim(0)))                              \n\
        __kernel void reader(__global int* data, int should_be_ok,           \n\
            int should_be_fail, __global int* errors) {                      \n\
          *errors = 0;                                                       \n\
          bool res = false;                                                  \n\
          for (int i = 0; i < should_be_ok; ++i) {                           \n\
            data[i] = read_channel_nb_intel(c, &res);                        \n\
            if (!res) {                                                      \n\
              ++(*errors);                                                   \n\
            }                                                                \n\
          }                                                                  \n\
          for (int i = 0; i < should_be_fail; ++i) {                         \n\
            int temp = 0;                                                    \n\
            temp = read_channel_nb_intel(c, &res);                           \n\
            if (res) {                                                       \n\
              ++(*errors);                                                   \n\
            }                                                                \n\
          }                                                                  \n\
        }                                                                    \n\
      ";
    std::string build_options =
        (depth != 0) ? "-DDEPTH=" + std::to_string(depth) : "";
    ASSERT_TRUE(createAndBuildProgram(program_sources, build_options))
        << "createAndBuildProgram failed";

    cl_mem input_buffer = createBuffer<cl_int>(
        input.size(), CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, &input.front());
    ASSERT_NE(nullptr, input_buffer) << "createBuffer failed";

    cl_mem output_buffer =
        createBuffer<cl_int>(input.size(), CL_MEM_WRITE_ONLY);
    ASSERT_NE(nullptr, output_buffer) << "createBuffer failed";

    cl_mem reader_errors_buffer =
        createBuffer<cl_int>(1, CL_MEM_WRITE_ONLY);
    ASSERT_NE(nullptr, reader_errors_buffer) << "createBuffer failed";

    cl_mem writer_errors_buffer =
        createBuffer<cl_int>(1, CL_MEM_WRITE_ONLY);
    ASSERT_NE(nullptr, writer_errors_buffer) << "createBuffer failed";

    ASSERT_TRUE(enqueueTask("writer", input_buffer, should_be_ok,
                                  should_be_fail, reader_errors_buffer))
        << "enqueuTask failed";
    cl_int reader_errors = 0;
    ASSERT_TRUE(readBuffer<cl_int>(
        "writer", reader_errors_buffer, 1, &reader_errors))
        << "readBuffer failed";
    ASSERT_EQ(0, reader_errors) << "reader kernel was executed with errors";

    ASSERT_TRUE(enqueueTask("reader", output_buffer, should_be_ok,
                                  should_be_fail, writer_errors_buffer))
        << "enqueueTask failed";
    cl_int writer_errors = 0;
    ASSERT_TRUE(readBuffer<cl_int>(
        "reader", writer_errors_buffer, 1, &writer_errors))
        << "readBuffer failed";
    ASSERT_EQ(0, writer_errors) << "writer kernel was executed with errors";

    std::vector<cl_int> data(should_be_ok);
    ASSERT_TRUE(readBuffer<cl_int>("reader", output_buffer,
                                                  should_be_ok, &data.front()))
        << "readBuffer failed";

    for (cl_int i = 0; i < should_be_ok; ++i) {
      ASSERT_EQ(input[i], data[i])
          << "validation fails for " << i << "-th element of array";
    }
  }

  void doPipesTest(int depth, std::vector<cl_int> &input, cl_int should_be_ok,
                   cl_int should_be_fail) {
    const std::string program_sources = "                                    \n\
        #ifdef DEPTH                                                         \n\
          #define PIPE_DEPTH __attribute__((depth(DEPTH)))                   \n\
        #else                                                                \n\
          #define PIPE_DEPTH                                                 \n\
        #endif                                                               \n\
                                                                             \n\
        __attribute__((max_global_work_dim(0)))                              \n\
        __kernel void writer(__global int* data,                             \n\
            write_only pipe int c PIPE_DEPTH, int should_be_ok,              \n\
            int should_be_fail, __global int* errors) {                      \n\
          *errors = 0;                                                       \n\
          int res = false;                                                   \n\
          for (int i = 0; i < should_be_ok; ++i) {                           \n\
            res = write_pipe(c, &data[i]);                                   \n\
            if (res != 0) {                                                  \n\
              ++(*errors);                                                   \n\
            }                                                                \n\
          }                                                                  \n\
          for (int i = 0; i < should_be_fail; ++i) {                         \n\
            int idx = should_be_ok + i;                                      \n\
            res = write_pipe(c, &data[idx]);                                 \n\
            if (res == 0) {                                                  \n\
              ++(*errors);                                                   \n\
            }                                                                \n\
          }                                                                  \n\
        }                                                                    \n\
                                                                             \n\
        __attribute__((max_global_work_dim(0)))                              \n\
        __kernel void reader(__global int* data,                             \n\
            read_only pipe int c PIPE_DEPTH, int should_be_ok,               \n\
            int should_be_fail, __global int* errors) {                      \n\
          *errors = 0;                                                       \n\
          int res = false;                                                   \n\
          for (int i = 0; i < should_be_ok; ++i) {                           \n\
            res = read_pipe(c, &data[i]);                                    \n\
            if (res != 0) {                                                  \n\
              ++(*errors);                                                   \n\
            }                                                                \n\
          }                                                                  \n\
          for (int i = 0; i < should_be_fail; ++i) {                         \n\
            int temp = 0;                                                    \n\
            res = read_pipe(c, &temp);                                       \n\
            if (res == 0) {                                                  \n\
              ++(*errors);                                                   \n\
            }                                                                \n\
          }                                                                  \n\
        }                                                                    \n\
      ";
    std::string build_options =
        (depth != 0) ? "-DDEPTH=" + std::to_string(depth) : "";
    ASSERT_TRUE(createAndBuildProgram(program_sources, build_options))
        << "createAndBuildProgram failed";

    cl_mem input_buffer = createBuffer<cl_int>(
        input.size(), CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, &input.front());
    ASSERT_NE(nullptr, input_buffer) << "createBuffer failed";

    cl_mem output_buffer =
        createBuffer<cl_int>(input.size(), CL_MEM_WRITE_ONLY);
    ASSERT_NE(nullptr, output_buffer) << "createBuffer failed";

    cl_mem reader_errors_buffer =
        createBuffer<cl_int>(1, CL_MEM_WRITE_ONLY);
    ASSERT_NE(nullptr, reader_errors_buffer) << "createBuffer failed";

    cl_mem writer_errors_buffer =
        createBuffer<cl_int>(1, CL_MEM_WRITE_ONLY);
    ASSERT_NE(nullptr, writer_errors_buffer) << "createBuffer failed";

    cl_mem pipe = createPipe<cl_int>(depth);
    ASSERT_NE(nullptr, pipe) << "createPipe failed";

    ASSERT_TRUE(enqueueTask("writer", input_buffer, pipe, should_be_ok,
                                  should_be_fail, reader_errors_buffer))
        << "enqueuTask failed";
    cl_int reader_errors = 0;
    ASSERT_TRUE(readBuffer<cl_int>(
        "writer", reader_errors_buffer, 1, &reader_errors))
        << "readBuffer failed";
    ASSERT_EQ(0, reader_errors) << "reader kernel was executed with errors";

    ASSERT_TRUE(enqueueTask("reader", output_buffer, pipe,
                                           should_be_ok, should_be_fail,
                                           writer_errors_buffer))
        << "enqueueTask failed";
    cl_int writer_errors = 0;
    ASSERT_TRUE(readBuffer<cl_int>(
        "reader", writer_errors_buffer, 1, &writer_errors))
        << "readBuffer failed";
    ASSERT_EQ(0, writer_errors) << "writer kernel was executed with errors";

    std::vector<cl_int> data(should_be_ok);
    ASSERT_TRUE(readBuffer<cl_int>("reader", output_buffer,
                                                  should_be_ok, &data.front()))
        << "readBuffer failed";

    for (cl_int i = 0; i < should_be_ok; ++i) {
      ASSERT_EQ(input[i], data[i])
          << "validation fails for " << i << "-th element of array";
    }
  }

  void doVPOTest(std::vector<cl_int> &input, int depth) {
    const std::string program_sources = "                                    \n\
        #pragma OPENCL EXTENSION cl_intel_channels: enable                   \n\
        #ifdef DEPTH                                                         \n\
          channel int c __attribute__((depth(DEPTH)));                       \n\
        #else                                                                \n\
          channel int c;                                                     \n\
        #endif                                                               \n\
                                                                             \n\
        __attribute__((max_global_work_dim(0)))                              \n\
        __kernel void writer(__global int* data) {                           \n\
          #pragma omp simd                                                   \n\
          for (int i = 0; i < DATA_SIZE; ++i) {                              \n\
            write_channel_intel(c, data[i]);                                 \n\
          }                                                                  \n\
        }                                                                    \n\
                                                                             \n\
        __attribute__((max_global_work_dim(0)))                              \n\
        __kernel void reader(__global int* data) {                           \n\
          #pragma omp simd                                                   \n\
          for (int i = 0; i < DATA_SIZE; ++i) {                              \n\
            data[i] = read_channel_intel(c);                                 \n\
          }                                                                  \n\
        }                                                                    \n\
      ";

    std::string build_options =
        "-DDATA_SIZE=" + std::to_string(input.size()) + " ";
    if (depth != 0) {
      build_options += "-DDEPTH=" + std::to_string(depth);
    }
    ASSERT_TRUE(createAndBuildProgram(program_sources, build_options))
        << "createAndBuildProgram failed";

    cl_mem input_buffer = createBuffer<cl_int>(
        input.size(), CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, &input.front());
    ASSERT_NE(nullptr, input_buffer) << "createBuffer failed";

    cl_mem output_buffer =
        createBuffer<cl_int>(input.size(), CL_MEM_WRITE_ONLY);
    ASSERT_NE(nullptr, output_buffer) << "createBuffer failed";

    ASSERT_TRUE(enqueueTask("writer", input_buffer))
        << "enqueueTask failed";
    ASSERT_TRUE(enqueueTask("reader", output_buffer))
        << "enqueueTask failed";

    std::vector<cl_int> data(input.size(), 0);
    ASSERT_TRUE(readBuffer<cl_int>("reader", output_buffer,
                                                  data.size(), &data.front()))
        << "readBuffer failed";

    for (cl_int i = 0; i < (cl_int)data.size(); ++i) {
      ASSERT_EQ(input[i], data[i])
          << " verification failed for " << i << "-th element of array";
    }
  }
};

class TestChannelDepthEmulationWithEmptyEnv : public TestChannelDepthEmulation {
};

enum ChannelDepthMode { STRICT, DEFAULT, IGNOREDEPTH };

template <ChannelDepthMode mode, bool vpo>
class TestChannelDepthEmulationEnvHelper : public TestChannelDepthEmulation {
protected:
  typedef TestChannelDepthEmulation parent_t;

  void SetUp() override {
    switch (mode) {
    case STRICT:
      SETENV("CL_CONFIG_CHANNEL_DEPTH_EMULATION_MODE", "strict");
      break;
    case DEFAULT:
      SETENV("CL_CONFIG_CHANNEL_DEPTH_EMULATION_MODE", "default");
      break;
    case IGNOREDEPTH:
      SETENV("CL_CONFIG_CHANNEL_DEPTH_EMULATION_MODE", "ignore-depth");
      break;
    }
    if (vpo) {
      SETENV("VOLCANO_CLANG_OPTIONS",
             "-fopenmp -fintel-openmp-region -fopenmp-tbb -fintel-compatibility");
    }
    parent_t::SetUp();
  }

  void TearDown() override {
    parent_t::TearDown();
    UNSETENV("CL_CONFIG_CHANNEL_DEPTH_EMULATION_MODE");
    if (vpo) {
      UNSETENV("VOLCANO_CLANG_OPTIONS");
    }
  }
};

class TestChannelDepthEmulationWithStrictEnv
    : public TestChannelDepthEmulationEnvHelper<STRICT, false> {};
class TestChannelDepthEmulationWithDefaultEnv
    : public TestChannelDepthEmulationEnvHelper<DEFAULT, false> {};
class TestChannelDepthEmulationWithIgnoreDepthEnv
    : public TestChannelDepthEmulationEnvHelper<IGNOREDEPTH, false> {};
class TestChannelDepthEmulationVPOWithStrictEnv
    : public TestChannelDepthEmulationEnvHelper<STRICT, true> {};
class TestChannelDepthEmulationVPOWithDefaultEnv
    : public TestChannelDepthEmulationEnvHelper<DEFAULT, true> {};
class TestChannelDepthEmulationVPOWithIgnoreDepthEnv
    : public TestChannelDepthEmulationEnvHelper<IGNOREDEPTH, true> {};

TEST_F(TestChannelDepthEmulationWithEmptyEnv, ChannelsWithDepth) {
  std::vector<cl_int> data(129);
  std::iota(data.begin(), data.end(), 0);
  ASSERT_NO_FATAL_FAILURE(doChannelsTest(128, data, 128, 1));
}

TEST_F(TestChannelDepthEmulationWithEmptyEnv, ChannelsWithoutDepth) {
  std::vector<cl_int> data(2);
  std::iota(data.begin(), data.end(), 0);
  ASSERT_NO_FATAL_FAILURE(doChannelsTest(0, data, 1, 1));
}

TEST_F(TestChannelDepthEmulationWithEmptyEnv, PipesWithDepth) {
  std::vector<cl_int> data(129);
  std::iota(data.begin(), data.end(), 0);
  ASSERT_NO_FATAL_FAILURE(doPipesTest(128, data, 128, 1));
}

TEST_F(TestChannelDepthEmulationWithEmptyEnv, PipesWithoutDepth) {
  std::vector<cl_int> data(2);
  std::iota(data.begin(), data.end(), 0);
  ASSERT_NO_FATAL_FAILURE(doPipesTest(1, data, 1, 1));
}

TEST_F(TestChannelDepthEmulationWithStrictEnv, ChannelsWithDepth) {
  std::vector<cl_int> data(129);
  std::iota(data.begin(), data.end(), 0);
  ASSERT_NO_FATAL_FAILURE(doChannelsTest(128, data, 128, 1));
}

TEST_F(TestChannelDepthEmulationWithStrictEnv, ChannelsWithoutDepth) {
  std::vector<cl_int> data(2);
  std::iota(data.begin(), data.end(), 0);
  ASSERT_NO_FATAL_FAILURE(doChannelsTest(0, data, 1, 1));
}

TEST_F(TestChannelDepthEmulationWithStrictEnv, PipesWithDepth) {
  std::vector<cl_int> data(129);
  std::iota(data.begin(), data.end(), 0);
  ASSERT_NO_FATAL_FAILURE(doPipesTest(128, data, 128, 1));
}

TEST_F(TestChannelDepthEmulationWithStrictEnv, PipesWithoutDepth) {
  std::vector<cl_int> data(2);
  std::iota(data.begin(), data.end(), 0);
  ASSERT_NO_FATAL_FAILURE(doPipesTest(1, data, 1, 1));
}

TEST_F(TestChannelDepthEmulationWithDefaultEnv, ChannelsWithDepth) {
  std::vector<cl_int> data(129);
  std::iota(data.begin(), data.end(), 0);
  ASSERT_NO_FATAL_FAILURE(doChannelsTest(128, data, 128, 1));
}

TEST_F(TestChannelDepthEmulationWithDefaultEnv, ChannelsWithoutDepth) {
  std::vector<cl_int> data(129);
  std::iota(data.begin(), data.end(), 0);
  ASSERT_NO_FATAL_FAILURE(doChannelsTest(0, data, 129, 0));
}

TEST_F(TestChannelDepthEmulationWithDefaultEnv, PipesWithDepth) {
  std::vector<cl_int> data(129);
  std::iota(data.begin(), data.end(), 0);
  ASSERT_NO_FATAL_FAILURE(doPipesTest(128, data, 128, 1));
}

TEST_F(TestChannelDepthEmulationWithDefaultEnv, PipesWithoutDepth) {
  std::vector<cl_int> data(129);
  std::iota(data.begin(), data.end(), 0);
  ASSERT_NO_FATAL_FAILURE(doPipesTest(1, data, 1, 128));
}

TEST_F(TestChannelDepthEmulationWithIgnoreDepthEnv, ChannelsWithDepth) {
  std::vector<cl_int> data(129);
  std::iota(data.begin(), data.end(), 0);
  ASSERT_NO_FATAL_FAILURE(doChannelsTest(128, data, 129, 0));
}

TEST_F(TestChannelDepthEmulationWithIgnoreDepthEnv, ChannelsWithoutDepth) {
  std::vector<cl_int> data(129);
  std::iota(data.begin(), data.end(), 0);
  ASSERT_NO_FATAL_FAILURE(doChannelsTest(0, data, 129, 0));
}

TEST_F(TestChannelDepthEmulationWithIgnoreDepthEnv, PipesWithDepth) {
  std::vector<cl_int> data(129);
  std::iota(data.begin(), data.end(), 0);
  ASSERT_NO_FATAL_FAILURE(doPipesTest(128, data, 129, 0));
}

TEST_F(TestChannelDepthEmulationWithIgnoreDepthEnv, PipesWithoutDepth) {
  std::vector<cl_int> data(129);
  std::iota(data.begin(), data.end(), 0);
  ASSERT_NO_FATAL_FAILURE(doPipesTest(1, data, 129, 0));
}

TEST_F(TestChannelDepthEmulationVPOWithStrictEnv, ChannelsWithDepth) {
  std::vector<cl_int> data(128);
  std::iota(data.begin(), data.end(), 0);
  ASSERT_NO_FATAL_FAILURE(doVPOTest(data, 128));
}

TEST_F(TestChannelDepthEmulationVPOWithStrictEnv, ChannelsWithoutDepth) {
  std::vector<cl_int> data(128);
  std::iota(data.begin(), data.end(), 0);
  ASSERT_NO_FATAL_FAILURE(doVPOTest(data, 0));
}

TEST_F(TestChannelDepthEmulationVPOWithDefaultEnv, ChannelsWithDepth) {
  std::vector<cl_int> data(128);
  std::iota(data.begin(), data.end(), 0);
  ASSERT_NO_FATAL_FAILURE(doVPOTest(data, 128));
}

TEST_F(TestChannelDepthEmulationVPOWithDefaultEnv, ChannelsWithoutDepth) {
  std::vector<cl_int> data(128);
  std::iota(data.begin(), data.end(), 0);
  ASSERT_NO_FATAL_FAILURE(doVPOTest(data, 0));
}

TEST_F(TestChannelDepthEmulationVPOWithIgnoreDepthEnv, ChannelsWithDepth) {
  std::vector<cl_int> data(128);
  std::iota(data.begin(), data.end(), 0);
  ASSERT_NO_FATAL_FAILURE(doVPOTest(data, 128));
}

TEST_F(TestChannelDepthEmulationVPOWithIgnoreDepthEnv, ChannelsWithoutDepth) {
  std::vector<cl_int> data(128);
  std::iota(data.begin(), data.end(), 0);
  ASSERT_NO_FATAL_FAILURE(doVPOTest(data, 0));
}

TEST_F(TestChannelDepthEmulationWithDefaultEnv, CheckDiagnosticMessage) {
  const char *program_sources = "                                            \n\
        #pragma OPENCL EXTENSION cl_intel_channels: enable                   \n\
        channel int c;                                                       \n\
                                                                             \n\
        __attribute__((max_global_work_dim(0)))                              \n\
        __kernel void writer(__global int* data, int should_be_ok,           \n\
            int should_be_fail, __global int* errors) {                      \n\
          *errors = 0;                                                       \n\
          bool res = false;                                                  \n\
          for (int i = 0; i < should_be_ok; ++i) {                           \n\
            res = write_channel_nb_intel(c, data[i]);                        \n\
            if (!res) {                                                      \n\
              ++(*errors);                                                   \n\
            }                                                                \n\
          }                                                                  \n\
          for (int i = 0; i < should_be_fail; ++i) {                         \n\
            int idx = should_be_ok + i;                                      \n\
            res = write_channel_nb_intel(c, data[idx]);                      \n\
            if (res) {                                                       \n\
              ++(*errors);                                                   \n\
            }                                                                \n\
          }                                                                  \n\
        }                                                                    \n\
      ";

  cl_int error = CL_SUCCESS;
  cl_program program = clCreateProgramWithSource(
      getContext(), 1, &program_sources, nullptr, &error);
  ASSERT_EQ(CL_SUCCESS, error)
      << " clCreateProgramWithSource failed with error " << ErrToStr(error);

  cl_int build_error =
      clBuildProgram(program, 0, nullptr, "", nullptr, nullptr);
  EXPECT_EQ(CL_SUCCESS, build_error)
      << " clBuildProgram failed with error " << ErrToStr(error);
  size_t log_size = 0;
  error = clGetProgramBuildInfo(program, getDevice(),
                                CL_PROGRAM_BUILD_LOG, 0, nullptr, &log_size);
  ASSERT_EQ(CL_SUCCESS, error)
      << " clGetProgramBuildInfo failed with error " << ErrToStr(error);
  std::string log("", log_size);
  error =
      clGetProgramBuildInfo(program, getDevice(), CL_PROGRAM_BUILD_LOG,
                            log_size, &log[0], nullptr);
  ASSERT_EQ(CL_SUCCESS, error)
      << " clGetProgramBuildInfo failed with error " << ErrToStr(error);
  if (CL_SUCCESS != build_error) {
    FAIL() << log << "\n";
  }

  std::string expected_diag = "Warning: The default channel depths in the "
                              "emulation flow will be different from the "
                              "hardware flow depth (0) to speed up emulation. "
                              "The following channels are affected:\n - c";
  ASSERT_NE(std::string::npos, log.find(expected_diag))
      << " expected diagnostic message not found in build log!";
}
