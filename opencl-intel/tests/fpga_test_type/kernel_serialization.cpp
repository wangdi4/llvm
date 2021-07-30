//===--- kernel_serialization.cpp -                             -*- C++ -*-===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
//
// Internal tests for kernel serialization
//
// ===--------------------------------------------------------------------=== //
#include "TestsHelpClasses.h"
#include "simple_fixture.h"
#include <numeric>

class TestKernelSerialization : public OCLFPGASimpleFixture {
protected:
  void SetUp() override {
    OCLFPGASimpleFixture::SetUp();

    const std::string programSource = "\
        #pragma OPENCL EXTENSION cl_intel_channels : enable                  \n\
        channel int b[3];                                                    \n\
                                                                             \n\
        __attribute__((max_global_work_dim(0)))                              \n\
        __kernel void host_reader(__global int *data, int size) {            \n\
           for (int i = 0; i < size; ++i) {                                  \n\
            write_channel_intel(b[0], data[i] + 1);                          \n\
          }                                                                  \n\
        }                                                                    \n\
                                                                             \n\
        __attribute__((max_global_work_dim(0)))                              \n\
        __kernel void host_writer(__global int *data, int size) {            \n\
          for (int i = 0; i < size; ++i) {                                   \n\
            data[i] = read_channel_intel(b[0]);                              \n\
          }                                                                  \n\
        }";
    ASSERT_TRUE(createAndBuildProgram(programSource))
        << "createAndBuildProgram failed";

    m_size = 10;
    m_data.resize(m_size);
    std::iota(m_data.begin(), m_data.end(), 0);
    m_inputBuffer = createBuffer<cl_int>(
        m_size, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, m_data.data());
    ASSERT_NE(nullptr, m_inputBuffer) << "createBuffer failed";

    m_tempBuffer = createBuffer<cl_int>(m_size, CL_MEM_READ_WRITE);
    ASSERT_NE(nullptr, m_tempBuffer) << "createBuffer failed";

    m_outputBuffer = createBuffer<cl_int>(m_size, CL_MEM_WRITE_ONLY);
    ASSERT_NE(nullptr, m_outputBuffer) << "createBuffer failed";
  }

  void verify() {
    cl_int err = clFinish(getOOOQueue());
    ASSERT_OCL_SUCCESS(err, "clFinish");

    std::vector<cl_int> result(m_size);
    ASSERT_TRUE(readBuffer<cl_int>("host_writer", m_outputBuffer, m_size,
                                   &result.front()))
        << "readBuffer failed";

    for (cl_int i = 0; i < m_size; ++i)
      ASSERT_EQ(m_data[i] + 2, result[i]) << " Data is differ at index " << i;
  }

  cl_int m_size;
  std::vector<cl_int> m_data;
  cl_mem m_inputBuffer;
  cl_mem m_tempBuffer;
  cl_mem m_outputBuffer;
};

// If test sporadically hangs - that means the serialization isn't working.
TEST_F(TestKernelSerialization, Basic) {
  for (int i = 0; i < 2; ++i) {
    ASSERT_TRUE(enqueueOOOTask("host_reader", /*event*/ nullptr,
                               (i) ? m_tempBuffer : m_inputBuffer, m_size))
        << "enqueueOOOTask failed";
    ASSERT_TRUE(enqueueOOOTask("host_writer", /*event*/ nullptr,
                               (i) ? m_outputBuffer : m_tempBuffer, m_size))
        << "enqueueOOOTask failed";
  }

  ASSERT_NO_FATAL_FAILURE(verify());
}

/// Check test won't hang when clReleaseEvent is called right after
/// enqueueOOOTask.
TEST_F(TestKernelSerialization, clReleaseEvent) {
  for (int i = 0; i < 2; ++i) {
    cl_event readerEvent;
    ASSERT_TRUE(enqueueOOOTask("host_reader", &readerEvent,
                               (i) ? m_tempBuffer : m_inputBuffer, m_size))
        << "enqueueOOOTask failed";
    cl_int err = clReleaseEvent(readerEvent);
    ASSERT_OCL_SUCCESS(err, "clReleaseEvent");

    cl_event writerEvent;
    ASSERT_TRUE(enqueueOOOTask("host_writer", &writerEvent,
                               (i) ? m_outputBuffer : m_tempBuffer, m_size))
        << "enqueueOOOTask failed";
    err = clReleaseEvent(writerEvent);
    ASSERT_OCL_SUCCESS(err, "clReleaseEvent");
  }

  ASSERT_NO_FATAL_FAILURE(verify());
}
