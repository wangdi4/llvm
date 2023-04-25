// Copyright (c) 2020 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

#include "FrameworkTest.h"
#include "FrameworkTestThreads.h"
#include "TestsHelpClasses.h"
#include "common_utils.h"
#include "cpu_dev_limits.h"
#include "ocl_config.h"

extern cl_device_type gDeviceType;

enum TestKind { NoEnv, DisableParallel };

static bool checkResultBuffer(char *buffer, size_t size, char value) {
  for (size_t i = 0; i < size; i++) {
    if (buffer[i] != value)
      return false;
  }
  return true;
}

class ParallelCopySetTest : public ::testing::TestWithParam<TestKind> {
protected:
  virtual void SetUp() override {
    // Set env.
    switch (GetParam()) {
    case NoEnv:
      break;
    case DisableParallel:
      ASSERT_TRUE(SETENV(CL_CONFIG_ENABLE_PARALLEL_COPY, "False"));
      break;
    }

    cl_int err = clGetPlatformIDs(1, &m_platform, NULL);
    ASSERT_OCL_SUCCESS(err, "clGetPlatformIDs");

    err = clGetDeviceIDs(m_platform, gDeviceType, 1, &m_device, NULL);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceIDs");

    m_context = clCreateContext(NULL, 1, &m_device, NULL, NULL, &err);
    ASSERT_OCL_SUCCESS(err, "clCreateContext");

    cl_command_queue_properties properties[] = {CL_QUEUE_PROPERTIES,
                                                CL_QUEUE_PROFILING_ENABLE, 0};
    m_queue = clCreateCommandQueueWithProperties(m_context, m_device,
                                                 properties, &err);
    ASSERT_OCL_SUCCESS(err, "clCreateCommandQueueWithProperties");

    m_size = 4 * 1024 * 1024;
    m_checkSize = 16;
    m_value = 16;
  }

  virtual void TearDown() override {
    cl_int err = clReleaseCommandQueue(m_queue);
    EXPECT_OCL_SUCCESS(err, "clReleaseCommandQueue");
    err = clReleaseContext(m_context);
    EXPECT_OCL_SUCCESS(err, "clReleaseContext");

    // Unset env.
    switch (GetParam()) {
    case DisableParallel:
      ASSERT_TRUE(UNSETENV(CL_CONFIG_ENABLE_PARALLEL_COPY));
      break;
    default:
      break;
    }
  }

  void initBuffer(char *buffer) {
    memset(buffer, 0, m_size);
    for (size_t i = 0; i < m_checkSize; i++)
      buffer[i] = m_value;
  }

  bool checkBuffer(char *buffer) { return checkBuffer(buffer, m_value); }

  bool checkBuffer(char *buffer, char value) {
    return checkResultBuffer(buffer, m_checkSize, value);
  }

  void checkCmdType(cl_event event, cl_command_type expected) {
    // Check command type.
    cl_command_type commandType;
    cl_int err = clGetEventInfo(event, CL_EVENT_COMMAND_TYPE,
                                sizeof(cl_command_type), &commandType, nullptr);
    ASSERT_EQ(err, CL_SUCCESS) << "clGetEventInfo CL_EVENT_COMMAND_TYPE failed";
    ASSERT_EQ(commandType, expected);
  }

protected:
  cl_platform_id m_platform;
  cl_device_id m_device;
  cl_context m_context;
  cl_command_queue m_queue;
  size_t m_size;
  size_t m_checkSize;
  char m_value;
};

TEST_P(ParallelCopySetTest, readBuffer) {
  cl_int err;

  // Create and initialize buffer.
  char *src = new char[m_size];
  initBuffer(src);
  cl_mem buffer =
      clCreateBuffer(m_context, CL_MEM_USE_HOST_PTR, m_size, src, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateBuffer");

  char *dst = new char[m_size];
  cl_event event;
  err = clEnqueueReadBuffer(m_queue, buffer, CL_TRUE, 0, m_size, dst, 0,
                            nullptr, &event);
  ASSERT_OCL_SUCCESS(err, "clEnqueueReadBuffer");

  // Check result.
  ASSERT_TRUE(checkBuffer(dst));

  // Check command type.
  ASSERT_NO_FATAL_FAILURE(checkCmdType(event, CL_COMMAND_READ_BUFFER));

  // Test event profiling.
  cl_ulong start, end;
  err = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START,
                                sizeof(cl_ulong), &start, nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetEventProfilingInfo");
  err = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END,
                                sizeof(cl_ulong), &end, nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetEventProfilingInfo");
  ASSERT_GT(end, start);

  err = clReleaseMemObject(buffer);
  ASSERT_OCL_SUCCESS(err, "clReleaseBuffer");
  delete[] src;
  delete[] dst;
}

TEST_P(ParallelCopySetTest, writeBuffer) {
  cl_int err;

  // Create and initialize buffer.
  char *src = new char[m_size];
  initBuffer(src);

  char *dst = new char[m_size];
  cl_mem buffer =
      clCreateBuffer(m_context, CL_MEM_USE_HOST_PTR, m_size, dst, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateBuffer");

  cl_event event;
  err = clEnqueueWriteBuffer(m_queue, buffer, CL_TRUE, 0, m_size, src, 0,
                             nullptr, &event);
  ASSERT_OCL_SUCCESS(err, "clEnqueueReadBuffer");

  // Check result.
  void *mappedPtr = clEnqueueMapBuffer(m_queue, buffer, CL_TRUE, CL_MAP_READ, 0,
                                       m_size, 0, nullptr, nullptr, &err);
  ASSERT_OCL_SUCCESS(err, "clEnqueueMapBuffer");
  ASSERT_TRUE(checkBuffer((char *)mappedPtr));
  err =
      clEnqueueUnmapMemObject(m_queue, buffer, mappedPtr, 0, nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueUnmapMemObj");
  err = clFinish(m_queue);
  ASSERT_OCL_SUCCESS(err, "clFinish");

  // Check command type.
  ASSERT_NO_FATAL_FAILURE(checkCmdType(event, CL_COMMAND_WRITE_BUFFER));

  err = clReleaseMemObject(buffer);
  ASSERT_OCL_SUCCESS(err, "clReleaseBuffer");
  delete[] src;
  delete[] dst;
}

TEST_P(ParallelCopySetTest, writeBufferRetainReleaseContext) {
  cl_int err;

  err = clRetainContext(m_context);
  ASSERT_OCL_SUCCESS(err, "clRetainContext");

  err = clReleaseContext(m_context);
  ASSERT_OCL_SUCCESS(err, "clReleaseContext");

  // Create and initialize buffer.
  char *src = new char[m_size];
  initBuffer(src);

  char *dst = new char[m_size];
  cl_mem buffer =
      clCreateBuffer(m_context, CL_MEM_USE_HOST_PTR, m_size, dst, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateBuffer");

  err = clEnqueueWriteBuffer(m_queue, buffer, CL_TRUE, 0, m_size, src, 0,
                             nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueReadBuffer");

  // Check result.
  void *mappedPtr = clEnqueueMapBuffer(m_queue, buffer, CL_TRUE, CL_MAP_READ, 0,
                                       m_size, 0, nullptr, nullptr, &err);
  ASSERT_OCL_SUCCESS(err, "clEnqueueMapBuffer");
  ASSERT_TRUE(checkBuffer((char *)mappedPtr));
  err =
      clEnqueueUnmapMemObject(m_queue, buffer, mappedPtr, 0, nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueUnmapMemObj");
  err = clFinish(m_queue);
  ASSERT_OCL_SUCCESS(err, "m_queue");

  err = clReleaseMemObject(buffer);
  ASSERT_OCL_SUCCESS(err, "clReleaseBuffer");
  delete[] src;
  delete[] dst;
}

TEST_P(ParallelCopySetTest, readWriteBuffer) {
  cl_int err;

  // Create and initialize buffer.
  char *src = new char[m_size];
  initBuffer(src);

  cl_mem buffer =
      clCreateBuffer(m_context, CL_MEM_READ_WRITE, m_size, nullptr, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateBuffer");
  err = clEnqueueWriteBuffer(m_queue, buffer, CL_FALSE, 0, m_size, src, 0,
                             nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueReadBuffer");

  char *dst = new char[m_size];
  err = clEnqueueReadBuffer(m_queue, buffer, CL_TRUE, 0, m_size, dst, 0,
                            nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueReadBuffer");

  // Check result.
  ASSERT_TRUE(checkBuffer(dst));

  err = clReleaseMemObject(buffer);
  ASSERT_OCL_SUCCESS(err, "clReleaseBuffer");
  delete[] src;
  delete[] dst;
}

TEST_P(ParallelCopySetTest, copyBuffer) {
  cl_int err;

  // Create and initialize buffer.
  char *src = new char[m_size];
  initBuffer(src);
  cl_mem bufferSrc =
      clCreateBuffer(m_context, CL_MEM_USE_HOST_PTR, m_size, src, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateBuffer");

  cl_mem bufferDst =
      clCreateBuffer(m_context, CL_MEM_WRITE_ONLY, m_size, nullptr, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateBuffer");

  // Copy from buffer to buffer.
  cl_event event;
  err = clEnqueueCopyBuffer(m_queue, bufferSrc, bufferDst, 0, 0, m_size, 0,
                            nullptr, &event);
  ASSERT_OCL_SUCCESS(err, "clEnqueueReadBuffer");

  // Copy from buffer to buffer
  char *dst = new char[m_size];
  cl_mem bufferDst2 =
      clCreateBuffer(m_context, CL_MEM_USE_HOST_PTR, m_size, dst, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateBuffer");
  err = clEnqueueCopyBuffer(m_queue, bufferDst, bufferDst2, 0, 0, m_size, 0,
                            nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueReadBuffer");

  err = clFinish(m_queue);
  ASSERT_OCL_SUCCESS(err, "clFinish");

  // Check result.
  void *mappedPtr =
      clEnqueueMapBuffer(m_queue, bufferDst2, CL_TRUE, CL_MAP_READ, 0, m_size,
                         0, nullptr, nullptr, &err);
  ASSERT_OCL_SUCCESS(err, "clEnqueueMapBuffer");
  ASSERT_TRUE(checkBuffer((char *)mappedPtr));
  err = clEnqueueUnmapMemObject(m_queue, bufferDst2, mappedPtr, 0, nullptr,
                                nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueUnmapMemObj");
  err = clFinish(m_queue);
  ASSERT_OCL_SUCCESS(err, "clFinish");

  // Check command type.
  ASSERT_NO_FATAL_FAILURE(checkCmdType(event, CL_COMMAND_COPY_BUFFER));

  err = clReleaseMemObject(bufferSrc);
  ASSERT_OCL_SUCCESS(err, "clReleaseBuffer");
  err = clReleaseMemObject(bufferDst);
  ASSERT_OCL_SUCCESS(err, "clReleaseBuffer");
  delete[] src;
  delete[] dst;
}

TEST_P(ParallelCopySetTest, svmMemcpy) {
  // Create and initialize buffer.
  char *src = new char[m_size];
  initBuffer(src);

  // Copy from host to SVM buffer.
  void *svmBuffer =
      clSVMAlloc(m_context, CL_MEM_SVM_FINE_GRAIN_BUFFER, m_size, 0);
  ASSERT_NE(svmBuffer, nullptr);
  cl_int err = clEnqueueSVMMemcpy(m_queue, CL_TRUE, svmBuffer, src, m_size, 0,
                                  nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueSVMMemcpy");

  // Check result.
  ASSERT_TRUE(checkBuffer((char *)svmBuffer));

  // Copy from SVM buffer to SVM buffer.
  void *svmBuffer2 = clSVMAlloc(m_context, CL_MEM_READ_ONLY, m_size, 0);
  ASSERT_NE(svmBuffer2, nullptr);
  err = clEnqueueSVMMemcpy(m_queue, CL_TRUE, svmBuffer2, svmBuffer, m_size, 0,
                           nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueSVMMemcpy");

  // Copy from SVM buffer to host.
  cl_event event;
  char *dst = new char[m_size];
  err = clEnqueueSVMMemcpy(m_queue, CL_FALSE, dst, svmBuffer2, m_size, 0,
                           nullptr, &event);
  ASSERT_OCL_SUCCESS(err, "clEnqueueSVMMemcpy");

  err = clWaitForEvents(1, &event);
  ASSERT_OCL_SUCCESS(err, "clWaitForEvents");

  // Check result.
  ASSERT_TRUE(checkBuffer(dst));

  // Check command type.
  ASSERT_NO_FATAL_FAILURE(checkCmdType(event, CL_COMMAND_SVM_MEMCPY));

  clSVMFree(m_context, svmBuffer);
  clSVMFree(m_context, svmBuffer2);
  delete[] src;
  delete[] dst;
}

TEST_P(ParallelCopySetTest, svmMemFill) {
  // Create and fill SVM buffer.
  void *svmBuffer = clSVMAlloc(m_context, CL_MEM_READ_ONLY, m_size, 0);
  ASSERT_NE(svmBuffer, nullptr);
  cl_int err =
      clEnqueueSVMMemFill(m_queue, svmBuffer, &m_value, sizeof(m_value), m_size,
                          0, nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueSVMMemFill");

  // Copy from SVM buffer to host.
  char *dst = new char[m_size];
  err = clEnqueueSVMMemcpy(m_queue, CL_TRUE, dst, svmBuffer, m_size, 0, nullptr,
                           nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueSVMMemcpy");

  // Check result.
  ASSERT_TRUE(checkBuffer(dst));

  // Fill with 0.
  char value = 0;
  cl_event event;
  err = clEnqueueSVMMemFill(m_queue, svmBuffer, (const void *)&value,
                            sizeof(value), m_size, 0, nullptr, &event);
  ASSERT_OCL_SUCCESS(err, "clEnqueueSVMMemFill");

  // Copy from SVM buffer to host.
  err = clEnqueueSVMMemcpy(m_queue, CL_TRUE, dst, svmBuffer, m_size, 0, nullptr,
                           nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueSVMMemcpy");

  // Check result.
  ASSERT_TRUE(checkBuffer(dst, value));

  // Check command type.
  ASSERT_NO_FATAL_FAILURE(checkCmdType(event, CL_COMMAND_SVM_MEMFILL));

  clSVMFree(m_context, svmBuffer);
  delete[] dst;
}

TEST_P(ParallelCopySetTest, usmMemcpy) {
  cl_int err;

  // Allocate USM buffers.
  cl_mem_properties_intel *properties = nullptr;
  cl_uint alignment = 0;
  char *hostUSM = (char *)clHostMemAllocINTEL(m_context, properties, m_size,
                                              alignment, &err);
  ASSERT_OCL_SUCCESS(err, "clHostMemAllocINTEL");
  alignment = 0;
  char *deviceUSM = (char *)clDeviceMemAllocINTEL(
      m_context, m_device, properties, m_size, alignment, &err);
  ASSERT_OCL_SUCCESS(err, "clDeviceMemAllocINTEL");

  // Initialize buffer.
  initBuffer(hostUSM);

  // Copy from host USM to device USM.
  err = clEnqueueMemcpyINTEL(m_queue, CL_FALSE, deviceUSM, hostUSM, m_size, 0,
                             nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueMemcpyINTEL");

  // Copy from device USM to host.
  char *dst = new char[m_size];
  err = clEnqueueMemcpyINTEL(m_queue, CL_TRUE, dst, deviceUSM, m_size, 0,
                             nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueMemcpyINTEL");

  // Check result.
  ASSERT_TRUE(checkBuffer(dst));

  // Copy from host to shared USM.
  char *sharedUSM = (char *)clSharedMemAllocINTEL(
      m_context, m_device, properties, m_size, alignment, &err);
  ASSERT_OCL_SUCCESS(err, "clSharedMemAllocINTEL");
  err = clEnqueueMemcpyINTEL(m_queue, CL_TRUE, sharedUSM, dst, m_size, 0,
                             nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueMemcpyINTEL");

  // Copy from shared USM to host.
  cl_event event;
  memset(dst, 0, m_size);
  err = clEnqueueMemcpyINTEL(m_queue, CL_TRUE, dst, sharedUSM, m_size, 0,
                             nullptr, &event);
  ASSERT_OCL_SUCCESS(err, "clEnqueueMemcpyINTEL");

  // Check result.
  ASSERT_TRUE(checkBuffer(dst));

  // Check command type.
  ASSERT_NO_FATAL_FAILURE(checkCmdType(event, CL_COMMAND_MEMCPY_INTEL));

  err = clMemFreeINTEL(m_context, hostUSM);
  ASSERT_OCL_SUCCESS(err, "clMemFreeINTEL");
  err = clMemFreeINTEL(m_context, deviceUSM);
  ASSERT_OCL_SUCCESS(err, "clMemFreeINTEL");
  err = clMemFreeINTEL(m_context, sharedUSM);
  ASSERT_OCL_SUCCESS(err, "clMemFreeINTEL");
  delete[] dst;
}

TEST_P(ParallelCopySetTest, usmMemFill) {
  cl_int err;

  // Allocate and fill USM buffer.
  cl_mem_properties_intel *properties = nullptr;
  cl_uint alignment = 0;
  alignment = 0;
  char *deviceUSM = (char *)clDeviceMemAllocINTEL(
      m_context, m_device, properties, m_size, alignment, &err);
  ASSERT_OCL_SUCCESS(err, "clDeviceMemAllocINTEL");
  err = clEnqueueMemFillINTEL(m_queue, deviceUSM, &m_value, sizeof(m_value),
                              m_size, 0, nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueMemFillINTEL");

  // Copy from device USM to host.
  char *dst = new char[m_size];
  err = clEnqueueMemcpyINTEL(m_queue, CL_TRUE, dst, deviceUSM, m_size, 0,
                             nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueMemcpyINTEL");

  // Check result.
  ASSERT_TRUE(checkBuffer(dst));

  // Fill with 0.
  cl_event event;
  char value = 0;
  err = clEnqueueMemFillINTEL(m_queue, deviceUSM, &value, sizeof(value), m_size,
                              0, nullptr, &event);
  ASSERT_OCL_SUCCESS(err, "clEnqueueMemFillINTEL");
  err = clEnqueueMemcpyINTEL(m_queue, CL_TRUE, dst, deviceUSM, m_size, 0,
                             nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueMemcpyINTEL");

  // Check result.
  ASSERT_TRUE(checkBuffer(dst, value));

  // Check command type.
  ASSERT_NO_FATAL_FAILURE(checkCmdType(event, CL_COMMAND_MEMFILL_INTEL));

  err = clMemFreeINTEL(m_context, deviceUSM);
  ASSERT_OCL_SUCCESS(err, "clMemFreeINTEL");
  delete[] dst;
}

TEST_P(ParallelCopySetTest, usmMemset) {
  cl_int err;

  // Allocate and memset USM buffer.
  cl_mem_properties_intel *properties = nullptr;
  cl_uint alignment = 0;
  alignment = 0;
  char *sharedUSM = (char *)clSharedMemAllocINTEL(
      m_context, m_device, properties, m_size, alignment, &err);
  ASSERT_OCL_SUCCESS(err, "clDeviceMemAllocINTEL");
  cl_int value = (cl_int)m_value;
  err = clEnqueueMemsetINTEL(m_queue, sharedUSM, value, m_size, 0, nullptr,
                             nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueMemsetINTEL");

  // Copy from device USM to host.
  char *dst = new char[m_size];
  err = clEnqueueMemcpyINTEL(m_queue, CL_TRUE, dst, sharedUSM, m_size, 0,
                             nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueMemcpyINTEL");

  // Check result.
  ASSERT_TRUE(checkBuffer(dst));

  // memset with 0.
  cl_event event;
  value = 0;
  err = clEnqueueMemsetINTEL(m_queue, sharedUSM, value, m_size, 0, nullptr,
                             &event);
  ;
  ASSERT_OCL_SUCCESS(err, "clEnqueueMemsetINTEL");
  err = clEnqueueMemcpyINTEL(m_queue, CL_TRUE, dst, sharedUSM, m_size, 0,
                             nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueMemcpyINTEL");

  // Check result.
  ASSERT_TRUE(checkBuffer(dst, (char)value));

  // Check command type.
  ASSERT_NO_FATAL_FAILURE(checkCmdType(event, CL_COMMAND_MEMSET_INTEL));

  err = clMemFreeINTEL(m_context, sharedUSM);
  ASSERT_OCL_SUCCESS(err, "clMemFreeINTEL");
  delete[] dst;
}

#ifndef _WIN32

class ReadBufferThread : public SynchronizedThread {
public:
  ReadBufferThread(cl_context context, cl_device_id device)
      : m_context(context), m_device(device), m_result(false) {}

  virtual ~ReadBufferThread() {}

  bool GetResult() const { return m_result; }

protected:
  virtual void ThreadRoutine() {
    cl_int err;
    cl_command_queue queue =
        clCreateCommandQueueWithProperties(m_context, m_device, nullptr, &err);
    if (!CL_SUCCEEDED(err))
      return;
    // Create and initialize buffer.
    size_t size = 1024 * 1024;
    char value = 16;
    char *src = new char[size];
    for (size_t i = 0; i < size; i++)
      src[i] = value;
    cl_mem buffer =
        clCreateBuffer(m_context, CL_MEM_USE_HOST_PTR, size, src, &err);
    if (!CL_SUCCEEDED(err))
      return;

    char *dst = new char[size];
    err = clEnqueueReadBuffer(queue, buffer, CL_TRUE, 0, size, dst, 0, nullptr,
                              nullptr);
    if (!CL_SUCCEEDED(err))
      return;

    err = clReleaseMemObject(buffer);
    if (!CL_SUCCEEDED(err))
      return;

    err = clReleaseCommandQueue(queue);
    if (!CL_SUCCEEDED(err))
      return;

    // Check result.
    m_result = checkResultBuffer(dst, size, value);

    delete[] src;
    delete[] dst;
  }

  cl_context m_context;
  cl_device_id m_device;
  bool m_result;
};

TEST_P(ParallelCopySetTest, readBufferMultiThreads) {
  size_t numThreads = 10;

  std::vector<SynchronizedThread *> threads(numThreads);

  for (size_t i = 0; i < numThreads; ++i)
    threads[i] = new ReadBufferThread(m_context, m_device);

  SynchronizedThreadPool pool;
  pool.Init(&threads[0], numThreads);
  pool.StartAll();
  pool.WaitAll();

  bool res = true;
  for (size_t i = 0; i < numThreads && res; ++i) {
    res &= static_cast<ReadBufferThread *>(threads[i])->GetResult();
  }
  ASSERT_TRUE(res) << "readBufferMultiThreads test fails";

  for (size_t i = 0; i < numThreads; ++i)
    delete threads[i];
}

#endif // #ifndef _WIN32

INSTANTIATE_TEST_SUITE_P(KernelLibrary, ParallelCopySetTest,
                         ::testing::Values(NoEnv, DisableParallel));
