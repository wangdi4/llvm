//===--- host_side_pipes.cpp -                                  -*- C++ -*-===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
//
// Internal tests for Host-side pipes feature
//
// ===--------------------------------------------------------------------=== //
#include "CL/cl_fpga_ext.h"
#include "base_fixture.h"
#include "gtest_wrapper.h"
#include "pretty_printers.h"

#include <algorithm>
#include <cstdlib>
#include <random>
#include <string>

class TestHostSidePipes : public OCLFPGABaseFixture {
protected:
  typedef OCLFPGABaseFixture parent_t;

  void SetUp() override;
  void TearDown() override;

  bool runLoopbackKernel(cl_int numPackets);
  cl_int readPipe(cl_mem pipe, void *mem);
  cl_int writePipe(cl_mem pipe, const void *mem);
  cl_int mapPipe(cl_mem pipe, size_t num_bytes, void **mem);
  cl_int unmapPipe(cl_mem pipe, size_t num_bytes, void *mem);

  cl_device_id m_device;
  cl_context m_context;
  cl_command_queue m_queue;

  cl_program m_program;
  cl_kernel m_loopback;
  cl_mem m_pipeRead;
  cl_mem m_pipeWrite;

  static const std::string m_program_source;
  const int m_maxBufferSize = 128;

  cl_int (*read_pipe_fn)(cl_mem, void *);
  cl_int (*write_pipe_fn)(cl_mem, const void *);
  void *(*map_pipe_fn)(cl_mem, cl_map_flags, size_t, size_t *, cl_int *);
  cl_int (*unmap_pipe_fn)(cl_mem, void *, size_t, size_t *);
};

const std::string TestHostSidePipes::m_program_source =
    "                               \n\
#pragma OPENCL EXTENSION cl_intel_fpga_host_pipe : enable                               \n\
__kernel void loopback(read_only pipe int pin __attribute__((intel_host_accessible)),   \n\
                       write_only pipe int pout __attribute__((intel_host_accessible)), \n\
                       int iters) {                                                     \n\
    for (int i = 0; i < iters; ++i) {                                                   \n\
      int val = 0;                                                                      \n\
      while (read_pipe(pin, &val)) {}                                                   \n\
      while (write_pipe(pout, &val)) {}                                                 \n\
    }                                                                                   \n\
  }                                                                                     \n\
";

void TestHostSidePipes::SetUp() {
  parent_t::SetUp();

  m_device = device();
  m_context = createContext(m_device);
  ASSERT_NE(nullptr, m_context) << "createContext failed";

  cl_int error = CL_SUCCESS;
  m_queue =
      clCreateCommandQueueWithProperties(m_context, m_device, nullptr, &error);
  ASSERT_EQ(CL_SUCCESS, error)
      << "clCreateCommandQueueWithProperties failed with error "
      << ErrToStr(error);

  m_program = createAndBuildProgram(m_context, m_program_source);
  ASSERT_NE(nullptr, m_program) << "createAndBuildProgram failed";

  m_loopback = createKernel(m_program, "loopback");
  ASSERT_NE(nullptr, m_loopback) << "createKernel failed";

  m_pipeRead = createPipe(m_context, sizeof(cl_int), m_maxBufferSize,
                          CL_MEM_HOST_READ_ONLY);
  ASSERT_NE(nullptr, m_pipeRead) << "createPipe failed";

  m_pipeWrite = createPipe(m_context, sizeof(cl_int), m_maxBufferSize,
                           CL_MEM_HOST_WRITE_ONLY);
  ASSERT_NE(nullptr, m_pipeWrite) << "createPipe failed";

  read_pipe_fn = (cl_int(*)(cl_mem, void *))clGetExtensionFunctionAddress(
      "clReadPipeIntelFPGA");
  ASSERT_NE(nullptr, read_pipe_fn)
      << "clGetExtensionFunctionAddress(clReadPipeIntelFPGA) failed";

  write_pipe_fn =
      (cl_int(*)(cl_mem, const void *))clGetExtensionFunctionAddress(
          "clWritePipeIntelFPGA");
  ASSERT_NE(nullptr, write_pipe_fn)
      << "clGetExtensionFunctionAddress(clWritePipeIntelFPGA) failed";

  map_pipe_fn = (void *(*)(cl_mem, cl_map_flags, size_t, size_t *, cl_int *))
      clGetExtensionFunctionAddress("clMapHostPipeIntelFPGA");
  ASSERT_NE(nullptr, map_pipe_fn)
      << "clGetExtensionFunctionAddress(clMapPipeIntelFPGA) failed";

  unmap_pipe_fn = (cl_int(*)(cl_mem, void *, size_t, size_t *))
      clGetExtensionFunctionAddress("clUnmapHostPipeIntelFPGA");
  ASSERT_NE(nullptr, unmap_pipe_fn)
      << "clGetExtensionFunctionAddress(clUnmapPipeIntelFPGA) failed";
}

void TestHostSidePipes::TearDown() { parent_t::TearDown(); }

cl_int TestHostSidePipes::readPipe(cl_mem pipe, void *mem) {
  cl_int error = CL_SUCCESS;
  do {
    error = (*read_pipe_fn)(pipe, mem);
  } while (error == CL_PIPE_EMPTY);

  return error;
}

cl_int TestHostSidePipes::writePipe(cl_mem pipe, const void *mem) {
  cl_int error = CL_SUCCESS;
  do {
    error = (*write_pipe_fn)(pipe, mem);
  } while (error == CL_PIPE_FULL);

  return error;
}

cl_int TestHostSidePipes::mapPipe(cl_mem pipe, size_t numBytes, void **mem) {
  cl_int error = CL_SUCCESS;

  size_t mappedSize = 0;
  do {
    *mem = (*map_pipe_fn)(pipe, 0, numBytes, &mappedSize, &error);
  } while (error == CL_OUT_OF_RESOURCES);

  if (mappedSize != numBytes) {
    return CL_OUT_OF_RESOURCES;
  }
  return error;
}

cl_int TestHostSidePipes::unmapPipe(cl_mem pipe, size_t numBytes, void *mem) {
  cl_int error = CL_SUCCESS;
  size_t unmappedSize = 0;
  do {
    error = (*unmap_pipe_fn)(pipe, mem, numBytes, &unmappedSize);
  } while (error == CL_OUT_OF_RESOURCES);

  if (unmappedSize != numBytes) {
    return CL_OUT_OF_RESOURCES;
  }
  return error;
}

bool TestHostSidePipes::runLoopbackKernel(cl_int numPackets) {
  cl_int error = CL_SUCCESS;
  error = clSetKernelArg(m_loopback, 0, sizeof(m_pipeWrite), &m_pipeWrite);
  EXPECT_EQ(error, CL_SUCCESS)
      << "clSetKernelArg failed with error " << ErrToStr(error);
  if (error != CL_SUCCESS) {
    return false;
  }

  error = clSetKernelArg(m_loopback, 1, sizeof(m_pipeRead), &m_pipeRead);
  EXPECT_EQ(error, CL_SUCCESS)
      << "clSetKernelArg failed with error " << ErrToStr(error);
  if (error != CL_SUCCESS) {
    return false;
  }

  error = clSetKernelArg(m_loopback, 2, sizeof(numPackets), &numPackets);
  EXPECT_EQ(error, CL_SUCCESS)
      << "clSetKernelArg failed with error " << ErrToStr(error);
  if (error != CL_SUCCESS) {
    return false;
  }

  size_t global_work_size[1] = {1};
  size_t local_work_size[1] = {1};
  error =
      clEnqueueNDRangeKernel(m_queue, m_loopback, 1, nullptr, global_work_size,
                             local_work_size, 0, nullptr, nullptr);
  EXPECT_EQ(error, CL_SUCCESS)
      << "clEnqueueNDRangeKernel failed with error " << ErrToStr(error);
  if (error != CL_SUCCESS) {
    return false;
  }

  clFlush(m_queue);
  return true;
}

TEST_F(TestHostSidePipes, ReadWrite) {
  size_t numPackets = m_maxBufferSize;
  ASSERT_TRUE(runLoopbackKernel(numPackets)) << "Couldn't run loopback kernel";

  for (cl_int i = 0; i < (cl_int)numPackets; ++i) {
    ASSERT_EQ(CL_SUCCESS, writePipe(m_pipeWrite, &i));
  }

  for (cl_int i = 0; i < (cl_int)numPackets; ++i) {
    cl_int got = -1;
    ASSERT_EQ(CL_SUCCESS, readPipe(m_pipeRead, &got));

    ASSERT_EQ(i, got) << "Verification failed";
  }
}

TEST_F(TestHostSidePipes, MapUnmap) {
  size_t numPackets = m_maxBufferSize;
  ASSERT_TRUE(runLoopbackKernel(numPackets)) << "Couldn't run loopback kernel";

  for (cl_int i = 0; i < (cl_int)numPackets; ++i) {
    void *mem = nullptr;
    ASSERT_EQ(CL_SUCCESS, mapPipe(m_pipeWrite, sizeof(cl_int), &mem));
    memcpy(mem, &i, sizeof(cl_int));
    ASSERT_EQ(CL_SUCCESS, unmapPipe(m_pipeWrite, sizeof(cl_int), mem));
  }

  for (cl_int i = 0; i < (cl_int)numPackets; ++i) {
    cl_int got = -1;
    void *mem = nullptr;
    ASSERT_EQ(CL_SUCCESS, mapPipe(m_pipeRead, sizeof(cl_int), &mem));
    memcpy(&got, mem, sizeof(cl_int));
    ASSERT_EQ(CL_SUCCESS, unmapPipe(m_pipeRead, sizeof(cl_int), mem));

    ASSERT_EQ(i, got) << "Verification failed";
  }
}

TEST_F(TestHostSidePipes, Mix) {
  size_t numPackets = m_maxBufferSize;
  ASSERT_TRUE(runLoopbackKernel(numPackets)) << "Couldn't run loopback kernel";

  for (cl_int i = 0; i < (cl_int)numPackets; ++i) {
    if (i % 2) {
      void *mem = nullptr;
      ASSERT_EQ(CL_SUCCESS, mapPipe(m_pipeWrite, sizeof(cl_int), &mem));
      memcpy(mem, &i, sizeof(cl_int));
      ASSERT_EQ(CL_SUCCESS, unmapPipe(m_pipeWrite, sizeof(cl_int), mem));
    } else {
      ASSERT_EQ(CL_SUCCESS, writePipe(m_pipeWrite, &i));
    }
  }

  for (cl_int i = 0; i < (cl_int)numPackets; ++i) {
    cl_int got = -1;

    if (i % 2) {
      void *mem = nullptr;
      ASSERT_EQ(CL_SUCCESS, mapPipe(m_pipeRead, sizeof(cl_int), &mem));
      memcpy(&got, mem, sizeof(cl_int));
      ASSERT_EQ(CL_SUCCESS, unmapPipe(m_pipeRead, sizeof(cl_int), mem));
    } else {
      ASSERT_EQ(CL_SUCCESS, readPipe(m_pipeRead, &got));
    }

    ASSERT_EQ(i, got) << "Verification failed";
  }
}

TEST_F(TestHostSidePipes, MapSeq) {
  size_t numPackets = m_maxBufferSize;
  ASSERT_TRUE(runLoopbackKernel(numPackets)) << "Couldn't run loopback kernel";

  std::vector<void *> maps;
  std::vector<cl_int> indices;
  std::mt19937 r(666);

  for (cl_int i = 0; i < (cl_int)numPackets; ++i) {
    void *mem = nullptr;
    ASSERT_EQ(CL_SUCCESS, mapPipe(m_pipeWrite, sizeof(cl_int), &mem));
    maps.push_back(mem);
    indices.push_back(i);
  }

  std::shuffle(indices.begin(), indices.end(), r);

  for (cl_int i : indices) {
    void *mem = maps[i];
    memcpy(mem, &i, sizeof(cl_int));
    ASSERT_EQ(CL_SUCCESS, unmapPipe(m_pipeWrite, sizeof(cl_int), mem));
  }

  for (cl_int i = 0; i < (cl_int)numPackets; ++i) {
    cl_int got = -1;
    void *mem = nullptr;
    ASSERT_EQ(CL_SUCCESS, mapPipe(m_pipeRead, sizeof(cl_int), &mem));
    memcpy(&got, mem, sizeof(cl_int));
    ASSERT_EQ(CL_SUCCESS, unmapPipe(m_pipeRead, sizeof(cl_int), mem));

    ASSERT_EQ(i, got) << "Verification failed";
  }
}

TEST_F(TestHostSidePipes, MapMulti) {
  size_t numPackets = m_maxBufferSize;
  ASSERT_TRUE(runLoopbackKernel(numPackets)) << "Couldn't run loopback kernel";

  void *mem = nullptr;
  ASSERT_EQ(CL_SUCCESS,
            mapPipe(m_pipeWrite, sizeof(cl_int) * numPackets, &mem));

  for (cl_int i = 0; i < (cl_int)numPackets; ++i) {
    memcpy((cl_int *)mem + i, &i, sizeof(cl_int));
    ASSERT_EQ(CL_SUCCESS, unmapPipe(m_pipeWrite, sizeof(cl_int), mem));
  }

  for (cl_int i = 0; i < (cl_int)numPackets; ++i) {
    cl_int got = -1;
    void *mem = nullptr;
    ASSERT_EQ(CL_SUCCESS, mapPipe(m_pipeRead, sizeof(cl_int), &mem));
    memcpy(&got, mem, sizeof(cl_int));
    ASSERT_EQ(CL_SUCCESS, unmapPipe(m_pipeRead, sizeof(cl_int), mem));

    ASSERT_EQ(i, got) << "Verification failed";
  }
}

TEST_F(TestHostSidePipes, QueryMaxPipes) {
  size_t maxReadPipes = 0;
  size_t maxWritePipes = 0;

  size_t valueSizeRet = 0;

  cl_int err =
      clGetDeviceInfo(m_device, CL_DEVICE_MAX_HOST_READ_PIPES_INTEL,
                      sizeof(maxReadPipes), &maxReadPipes, &valueSizeRet);
  ASSERT_EQ(CL_SUCCESS, err) << "clGetDeviceInfo failed";
  ASSERT_EQ(sizeof(maxReadPipes), valueSizeRet);
  ASSERT_GT(maxReadPipes, 0U);

  err = clGetDeviceInfo(m_device, CL_DEVICE_MAX_HOST_WRITE_PIPES_INTEL,
                        sizeof(maxWritePipes), &maxWritePipes, &valueSizeRet);
  ASSERT_EQ(CL_SUCCESS, err) << "clGetDeviceInfo failed";
  ASSERT_EQ(sizeof(maxWritePipes), valueSizeRet);
  ASSERT_GT(maxWritePipes, 0U);
}

TEST_F(TestHostSidePipes, KernelArgMismatchNegative) {
  //  The direction indicated in the clCreatePipe flag must be
  //  opposite to the pipe access type specified on the kernel
  //  argument.
  cl_int error;
  error = clSetKernelArg(m_loopback, 0, sizeof(m_pipeRead), &m_pipeRead);
  EXPECT_EQ(CL_INVALID_ARG_VALUE, error);

  error = clSetKernelArg(m_loopback, 1, sizeof(m_pipeWrite), &m_pipeWrite);
  EXPECT_EQ(CL_INVALID_ARG_VALUE, error);
}

TEST_F(TestHostSidePipes, KernelArgNoAttrNegative) {
  const std::string programSrc = "                                           \n\
      __kernel void no_attr(read_only pipe int pin,                          \n\
                            write_only pipe int pout) {}                     \n\
      ";

  cl_program program = createAndBuildProgram(m_context, programSrc);
  ASSERT_NE(nullptr, program) << "createAndBuildProgram failed";

  // Host-side pipe can only be bound to a kernel argument with
  // __attribute__((intel_host_accessible))
  cl_kernel noAttrKernel = createKernel(program, "no_attr");
  ASSERT_NE(nullptr, noAttrKernel) << "createKernel failed";

  cl_int error = CL_SUCCESS;
  error = clSetKernelArg(noAttrKernel, 0, sizeof(m_pipeWrite), &m_pipeWrite);
  EXPECT_EQ(CL_INVALID_ARG_VALUE, error);

  error = clSetKernelArg(noAttrKernel, 1, sizeof(m_pipeRead), &m_pipeRead);
  EXPECT_EQ(CL_INVALID_ARG_VALUE, error);
}

TEST_F(TestHostSidePipes, FlagsGood) {
  cl_int iRet = CL_SUCCESS;
  cl_mem pipe = nullptr;
  pipe = clCreatePipe(m_context, CL_MEM_HOST_READ_ONLY, sizeof(cl_int),
                      m_maxBufferSize, nullptr, &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet)
      << "clCreatePipe(CL_MEM_HOST_READ_ONLY) failed with error "
      << ErrToStr(iRet);
  clReleaseMemObject(pipe);

  pipe = clCreatePipe(m_context, CL_MEM_HOST_READ_ONLY | CL_MEM_WRITE_ONLY,
                      sizeof(cl_int), m_maxBufferSize, nullptr, &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << "clCreatePipe(CL_MEM_HOST_READ_ONLY | "
                                 "CL_MEM_WRITE_ONLY) failed with error "
                              << ErrToStr(iRet);
  clReleaseMemObject(pipe);

  pipe = clCreatePipe(m_context, CL_MEM_HOST_WRITE_ONLY, sizeof(cl_int),
                      m_maxBufferSize, nullptr, &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet)
      << "clCreatePipe(CL_MEM_HOST_WRITE_ONLY) failed with error "
      << ErrToStr(iRet);
  clReleaseMemObject(pipe);

  pipe = clCreatePipe(m_context, CL_MEM_HOST_WRITE_ONLY | CL_MEM_READ_ONLY,
                      sizeof(cl_int), m_maxBufferSize, nullptr, &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << "clCreatePipe(CL_MEM_HOST_WRITE_ONLY | "
                                 "CL_MEM_READ_ONLY) failed with error "
                              << ErrToStr(iRet);
  clReleaseMemObject(pipe);
}

TEST_F(TestHostSidePipes, FlagsBad) {
  cl_int iRet = CL_SUCCESS;
  cl_mem pipe = nullptr;
  pipe = clCreatePipe(m_context, CL_MEM_HOST_READ_ONLY | CL_MEM_READ_ONLY,
                      sizeof(cl_int), m_maxBufferSize, nullptr, &iRet);
  ASSERT_EQ(CL_INVALID_VALUE, iRet) << "clCreatePipe(CL_MEM_HOST_READ_ONLY | "
                                       "CL_MEM_READ_ONLY) failed with error "
                                    << ErrToStr(iRet);
  clReleaseMemObject(pipe);

  pipe = clCreatePipe(m_context, CL_MEM_HOST_WRITE_ONLY | CL_MEM_WRITE_ONLY,
                      sizeof(cl_int), m_maxBufferSize, nullptr, &iRet);
  ASSERT_EQ(CL_INVALID_VALUE, iRet) << "clCreatePipe(CL_MEM_HOST_WRITE_ONLY | "
                                       "CL_MEM_WRITE_ONLY) failed with error "
                                    << ErrToStr(iRet);
  clReleaseMemObject(pipe);
}

TEST_F(TestHostSidePipes, BadPtr) {
  cl_int iRet = CL_SUCCESS;

  iRet = read_pipe_fn(m_pipeRead, nullptr);
  ASSERT_EQ(CL_INVALID_VALUE, iRet)
      << "clReadPipeIntelFPGA(nullptr): " << ErrToStr(iRet);

  iRet = write_pipe_fn(m_pipeWrite, nullptr);
  ASSERT_EQ(CL_INVALID_VALUE, iRet)
      << "clReadPipeIntelFPGA(nullptr): " << ErrToStr(iRet);

  map_pipe_fn(m_pipeRead, CL_MEM_HOST_READ_ONLY, sizeof(cl_int), nullptr,
              &iRet);
  ASSERT_EQ(CL_INVALID_VALUE, iRet)
      << "clMapPipeIntelFPGA(nullptr): " << ErrToStr(iRet);

  iRet =
      unmap_pipe_fn(m_pipeRead, nullptr, sizeof(cl_int), (size_t *)0xdeadbeef);
  ASSERT_EQ(CL_INVALID_VALUE, iRet)
      << "clUnmapPipeIntelFPGA(nullptr, 0xdeadbeef): " << ErrToStr(iRet);

  iRet = unmap_pipe_fn(m_pipeRead, (void *)0xdeadbeef, sizeof(cl_int), nullptr);
  ASSERT_EQ(CL_INVALID_VALUE, iRet)
      << "clUnmapPipeIntelFPGA(0xdeadbeef, nullptr): " << ErrToStr(iRet);
}
