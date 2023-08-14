#include "CL/cl.h"
#include "FrameworkTestThreads.h"
#include "TestsHelpClasses.h"
#include "tbb/global_control.h"
#include "tbb/parallel_for.h"
#include <cmath>

extern cl_device_type gDeviceType;

struct arg_struct {
  int *inp1;
  int *inp2;
  int val;
  int *output;
};

static void CL_CALLBACK NativeFunc(void *args) {
  if (!SilentCheck("args properly supplied", true, args != NULL)) {
    return;
  }

  arg_struct *arguments = (arg_struct *)args;
  *arguments->output = *arguments->inp1 + *arguments->inp2 + arguments->val;
}

class EnqueueNativeKernelTest : public ::testing::Test {
protected:
  virtual void SetUp() override {
    cl_int err = clGetPlatformIDs(1, &m_platform, NULL);
    ASSERT_OCL_SUCCESS(err, "clGetPlatformIDs");

    err = clGetDeviceIDs(m_platform, gDeviceType, 1, &m_device, NULL);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceIDs");

    m_context = clCreateContext(NULL, 1, &m_device, NULL, NULL, &err);
    ASSERT_OCL_SUCCESS(err, "clCreateContext");

    cl_command_queue_properties properties[] = {
        CL_QUEUE_PROPERTIES, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, 0};
    m_queue = clCreateCommandQueueWithProperties(m_context, m_device,
                                                 properties, &err);
    ASSERT_OCL_SUCCESS(err, "clCreateCommandQueueWithProperties");
  }

  virtual void TearDown() override {
    cl_int err = clReleaseCommandQueue(m_queue);
    EXPECT_OCL_SUCCESS(err, "clReleaseCommandQueue");
    err = clReleaseContext(m_context);
    EXPECT_OCL_SUCCESS(err, "clReleaseContext");
  }

protected:
  cl_platform_id m_platform;
  cl_device_id m_device;
  cl_context m_context;
  cl_command_queue m_queue;
};

TEST_F(EnqueueNativeKernelTest, basic) {
  // Create memory objects for test
  cl_int err;
  int data1 = 0xABCD;
  int data2 = 0x1234;
  cl_mem mem1 = clCreateBuffer(m_context, CL_MEM_COPY_HOST_PTR, sizeof(int),
                               &data1, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateBuffer");
  cl_mem mem2 = clCreateBuffer(m_context, CL_MEM_COPY_HOST_PTR, sizeof(int),
                               &data2, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateBuffer");
  cl_mem res =
      clCreateBuffer(m_context, CL_MEM_READ_WRITE, sizeof(int), NULL, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateBuffer");

  arg_struct args;
  args.val = 1234;

  cl_mem memList[] = {mem1, mem2, res};
  const void *offsets[] = {&args.inp2, &args.inp1, &args.output};
  cl_event e;
  err = clEnqueueNativeKernel(m_queue, NativeFunc, &args, sizeof(arg_struct), 3,
                              memList, offsets, 0, NULL, &e);
  ASSERT_OCL_SUCCESS(err, "clEnqueueNativeKernel");
  int data3;
  err = clEnqueueReadBuffer(m_queue, res, CL_TRUE, 0, sizeof(int), &data3, 1,
                            &e, NULL);
  ASSERT_OCL_SUCCESS(err, "clEnqueueReadBuffer");

  ASSERT_EQ(data3, data1 + data2 + args.val) << " result mismatch";

  err = clReleaseMemObject(mem1);
  ASSERT_OCL_SUCCESS(err, "clReleaseMemObject(mem1)");
  err = clReleaseMemObject(mem2);
  ASSERT_OCL_SUCCESS(err, "clReleaseMemObject(mem2)");
  err = clReleaseMemObject(res);
  ASSERT_OCL_SUCCESS(err, "clReleaseMemObject(res)");
}

struct ArgIntensive {
  float *r;
  int n;
};

static void CL_CALLBACK computeIntensiveFunc(void *arg) {
  ArgIntensive *arguments = static_cast<ArgIntensive *>(arg);
  float *r = arguments->r;
  int n = arguments->n;
  for (int i = 0; i < n; ++i) {
    // computations to 'waste' some time
    float f_i = (float)i;
    float q = std::cos(f_i * 0.00342f) * 5.38f;
    float p = std::exp(f_i * 0.0891f) * 0.007f;
    if ((q * 84.22f - 3.045f * p) > (p * q - 2352.3)) {
      r[i] = std::cos(p * 0.021) * 7542.4f + 52.54f * std::exp(q * -0.3463f);
    } else {
      r[i] = std::sin(p * 0.021) * 7542.4f + 52.54f * std::exp2(q * -0.3463f);
    }
  }
}

class NativeKernelThread : public SynchronizedThread {
public:
  NativeKernelThread(cl_context context, cl_device_id device,
                     cl_command_queue q, float *r, int size, int i)
      : m_context(context), m_device(device), m_q(q), m_result(false), m_r(r),
        m_size(size) {}

  bool getResult() const { return m_result; }

protected:
  void ThreadRoutine() override {
    ArgIntensive args = {m_r, m_size};
    cl_event e;
    cl_int err = clEnqueueNativeKernel(m_q, computeIntensiveFunc, &args,
                                       sizeof(ArgIntensive), 0, nullptr,
                                       nullptr, 0, nullptr, &e);
    m_result = (CL_SUCCESS == err);
    if (!m_result)
      return;
    err = clWaitForEvents(1, &e);
    m_result = (CL_SUCCESS == err);
  }

  cl_context m_context;
  cl_device_id m_device;
  cl_command_queue m_q;
  bool m_result;
  float *m_r;
  int m_size;
};

/// This test checks that host threads will not hang.
TEST_F(EnqueueNativeKernelTest, multiThreadEnqueueWait) {
  unsigned numThreads = getMaxNumExternalThreads();
  int N = 1200000;
  int blockSize = N / numThreads;
  std::vector<float> r(N);

  std::vector<SynchronizedThread *> threads(numThreads);

  for (unsigned i = 0; i < numThreads; ++i)
    threads[i] = new NativeKernelThread(m_context, m_device, m_queue,
                                        &r[blockSize * i], blockSize, i);

  SynchronizedThreadPool pool;
  pool.Init(&threads[0], numThreads);
  pool.StartAll();
  pool.WaitAll();

  for (unsigned i = 0; i < numThreads; ++i) {
    bool res = static_cast<NativeKernelThread *>(threads[i])->getResult();
    ASSERT_TRUE(res) << "NativeKernelThread " << i << " failed";
  }

  for (unsigned i = 0; i < numThreads; ++i)
    delete threads[i];
}

/// This test checks that TBB worker threads will not hang.
TEST_F(EnqueueNativeKernelTest, multiTBBThreadEnqueueWait) {
  unsigned numThreads = getMaxNumExternalThreads();
  int N = 1200000;
  int blockSize = N / numThreads;
  std::vector<float> r(N);

  tbb::parallel_for(tbb::blocked_range<int>(0, numThreads),
                    [&](tbb::blocked_range<int>(range)) {
                      for (int i = range.begin(); i < range.end(); ++i) {
                        ArgIntensive args = {&r[blockSize * i], blockSize};
                        cl_event e;
                        cl_int err = clEnqueueNativeKernel(
                            m_queue, computeIntensiveFunc, &args,
                            sizeof(ArgIntensive), 0, nullptr, nullptr, 0,
                            nullptr, &e);
                        ASSERT_OCL_SUCCESS(err, "clEnqueueNativeKernel");
                        err = clWaitForEvents(1, &e);
                        ASSERT_OCL_SUCCESS(err, "clWaitForEvents");
                        err = clReleaseEvent(e);
                        ASSERT_OCL_SUCCESS(err, "clReleaseEvent");
                      }
                    });
}
