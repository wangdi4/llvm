#include "bi_tests.h"

#include <fstream>
#include <sstream>

class PipeTest : public BITest {
protected:
  virtual void SetUp() {
    BITest::SetUp();

    std::ifstream file("pipe_tests.cl");
    std::stringstream buf;
    buf << file.rdbuf();

    std::string program_src = buf.str();
    const char* src = program_src.c_str();
    cl_int error;
    program =
      clCreateProgramWithSource(context, 1, &src, NULL, &error);
    ASSERT_EQ(error, CL_SUCCESS);

    error = clBuildProgram(program, 1, &device, "-cl-std=CL2.0", NULL, NULL);
    size_t log_size;
    clGetProgramBuildInfo(program, device,
                          CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
    std::vector<char> log(log_size);
    clGetProgramBuildInfo(program, device,
                          CL_PROGRAM_BUILD_LOG, log_size, &log[0], NULL);

    ASSERT_EQ(error, CL_SUCCESS) << &log[0];
  }

  virtual void TearDown() {
    BITest::TearDown();
    for (auto k : kernels) {
      clReleaseKernel(k);
    }
    clReleaseProgram(program);
  }

  cl_kernel createKernel(const char* name) {
    cl_int error;
    cl_kernel k = clCreateKernel(program, name, &error);
    if (error != CL_SUCCESS) {
      return nullptr;
    }
    kernels.push_back(k);
    return k;
  }

  cl_int enqueueSingleWI(cl_command_queue queue, cl_kernel kernel) {
    size_t globalSize[] = { 1 };
    size_t localSize[] = { 1 };
    return clEnqueueNDRangeKernel(queue, kernel, 1, NULL,
                                  globalSize, localSize,
                                  0, NULL, NULL);
  }

  cl_int enqueueSingleWI(cl_command_queue queue, const char* name) {
    cl_kernel kernel = createKernel(name);
    if (kernel == nullptr) {
      return CL_INVALID_KERNEL;
    }
    return enqueueSingleWI(queue, kernel);
  }

  cl_program program;
  std::vector<cl_kernel> kernels;
};

TEST_F(PipeTest, SinglePacket) {
  auto q1 = createCommandQueue();
  auto q2 = createCommandQueue();
  ASSERT_TRUE(q1 != nullptr && q2 != nullptr);

  cl_int error = enqueueSingleWI(q1, "single_packet_init");
  ASSERT_EQ(error, CL_SUCCESS);
  clFinish(q1);

  for (int i = 0; i < 1000; ++i) {
    error = enqueueSingleWI(q1, "single_packet_reader");
    ASSERT_EQ(error, CL_SUCCESS);
    error = enqueueSingleWI(q2, "single_packet_writer");
    ASSERT_EQ(error, CL_SUCCESS);

    clFinish(q1);
    clFinish(q2);
  }
}

TEST_F(PipeTest, MultiplePackets) {
  auto q1 = createCommandQueue();
  auto q2 = createCommandQueue();
  ASSERT_TRUE(q1 != nullptr && q2 != nullptr);

  cl_int error = enqueueSingleWI(q1, "multiple_packets_init");
  ASSERT_EQ(error, CL_SUCCESS);
  clFinish(q1);

  for (int i = 0; i < 100; ++i) {
    error = enqueueSingleWI(q1, "multiple_packets_reader");
    ASSERT_EQ(error, CL_SUCCESS);
    error = enqueueSingleWI(q2, "multiple_packets_writer");
    ASSERT_EQ(error, CL_SUCCESS);

    clFinish(q1);
    clFinish(q2);
  }
}

TEST_F(PipeTest, NonUniform) {
  auto q1 = createCommandQueue();
  auto q2 = createCommandQueue();
  ASSERT_TRUE(q1 != nullptr && q2 != nullptr);

  cl_int error = enqueueSingleWI(q1, "non_uniform_init");
  ASSERT_EQ(error, CL_SUCCESS);
  clFinish(q1);

  for (int i = 0; i < 100; ++i) {
    error = enqueueSingleWI(q1, "non_uniform_reader1");
    ASSERT_EQ(error, CL_SUCCESS);
    error = enqueueSingleWI(q2, "non_uniform_writer1");
    ASSERT_EQ(error, CL_SUCCESS);

    clFinish(q1);
    clFinish(q2);
  }

  for (int i = 0; i < 100; ++i) {
    error = enqueueSingleWI(q1, "non_uniform_reader2");
    ASSERT_EQ(error, CL_SUCCESS);
    error = enqueueSingleWI(q2, "non_uniform_writer2");
    ASSERT_EQ(error, CL_SUCCESS);

    clFinish(q1);
    clFinish(q2);
  }
}

TEST_F(PipeTest, OneSlow) {
  auto q1 = createCommandQueue();
  auto q2 = createCommandQueue();
  ASSERT_TRUE(q1 != nullptr && q2 != nullptr);

  cl_int error = enqueueSingleWI(q1, "one_slow_init");
  ASSERT_EQ(error, CL_SUCCESS);
  clFinish(q1);

  // 1 iteration takes ~12sec to complete, no need for more
  for (int i = 0; i < 1; ++i) {
    error = enqueueSingleWI(q1, "one_slow_reader1");
    ASSERT_EQ(error, CL_SUCCESS);
    error = enqueueSingleWI(q2, "one_slow_writer1");
    ASSERT_EQ(error, CL_SUCCESS);

    clFinish(q1);
    clFinish(q2);
  }

  // same here
  for (int i = 0; i < 1; ++i) {
    error = enqueueSingleWI(q1, "one_slow_reader2");
    ASSERT_EQ(error, CL_SUCCESS);
    error = enqueueSingleWI(q2, "one_slow_writer2");
    ASSERT_EQ(error, CL_SUCCESS);

    clFinish(q1);
    clFinish(q2);
  }
}

TEST_F(PipeTest, BillionPackets) {
  auto q1 = createCommandQueue();
  auto q2 = createCommandQueue();
  ASSERT_TRUE(q1 != nullptr && q2 != nullptr);

  cl_int error = enqueueSingleWI(q1, "billion_packets_init");
  ASSERT_EQ(error, CL_SUCCESS);
  clFinish(q1);

  // takes ~50sec for 1 iteration
  for (int i = 0; i < 1; ++i) {
    error = enqueueSingleWI(q1, "billion_packets_reader");
    ASSERT_EQ(error, CL_SUCCESS);
    error = enqueueSingleWI(q2, "billion_packets_writer");
    ASSERT_EQ(error, CL_SUCCESS);

    clFinish(q1);
    clFinish(q2);
  }
}

TEST_F(PipeTest, BigMaxPackets) {
  auto q1 = createCommandQueue();
  auto q2 = createCommandQueue();
  ASSERT_TRUE(q1 != nullptr && q2 != nullptr);

  cl_int max_packets[] = { 128,
                           200, 250, 256, 257, 1024, 2048,
                           1024 * 8, 1024 * 16,
                           1024 * 8 + 5, 1024 * 16 - 3,
                           1024 * 32, 1024 * 64 };

  cl_kernel init = createKernel("max_packets_init");
  ASSERT_TRUE(init != nullptr);

  for (cl_int max_p : max_packets) {
    clSetKernelArg(init, 0, sizeof(cl_int), &max_p);
    cl_int error = enqueueSingleWI(q1, init);
    ASSERT_EQ(error, CL_SUCCESS);
    clFinish(q1);

    error = enqueueSingleWI(q1, "max_packets_reader");
    ASSERT_EQ(error, CL_SUCCESS);
    error = enqueueSingleWI(q2, "max_packets_writer");
    ASSERT_EQ(error, CL_SUCCESS);

    clFinish(q1);
    clFinish(q2);
  }
}

TEST_F(PipeTest, SmallMaxPackets) {
  auto q1 = createCommandQueue();
  auto q2 = createCommandQueue();
  ASSERT_TRUE(q1 != nullptr && q2 != nullptr);

  std::vector<cl_int> max_packets;
  for (cl_int i = 1; i < 256; ++i) {
    max_packets.push_back(i);
  }

  cl_kernel init = createKernel("max_packets_init");
  ASSERT_TRUE(init != nullptr);

  for (cl_int max_p : max_packets) {
    clSetKernelArg(init, 0, sizeof(cl_int), &max_p);
    cl_int error = enqueueSingleWI(q1, init);
    ASSERT_EQ(error, CL_SUCCESS);
    clFinish(q1);

    error = enqueueSingleWI(q1, "max_packets_reader");
    ASSERT_EQ(error, CL_SUCCESS);
    error = enqueueSingleWI(q2, "max_packets_writer");
    ASSERT_EQ(error, CL_SUCCESS);

    clFinish(q1);
    clFinish(q2);
  }
}
