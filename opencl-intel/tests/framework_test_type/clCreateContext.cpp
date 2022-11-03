#include "FrameworkTest.h"
#include "FrameworkTestThreads.h"
#include "TestsHelpClasses.h"
#include "cl_types.h"
#include <stdio.h>

extern cl_device_type gDeviceType;

class CreateContextTest : public ::testing::Test {
  virtual void SetUp() override {
    cl_int err = clGetPlatformIDs(1, &m_platform, NULL);
    ASSERT_OCL_SUCCESS(err, "clGetPlatformIDs");
  }

protected:
  cl_platform_id m_platform;
};

/*******************************************************************************
 * clCreateContextTest
 * -------------------
 * Get device ids (gDeviceType)
 * Create context
 * Create context from type (gDeviceType)
 * Retain context
 * Release context
 * Get context info (CL_CONTEXT_REFERENCE_COUNT)
 ******************************************************************************/
TEST_F(CreateContextTest, Basic) {
  printf("clCreateContextTest\n");
  cl_uint uiNumDevices = 0;
  cl_device_id *pDevices;
  cl_context context;

  cl_int iRet = CL_SUCCESS;

  cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                   (cl_context_properties)m_platform, 0},
                        badProps2[] = {CL_CONTEXT_PLATFORM,
                                       (cl_context_properties)m_platform,
                                       CL_CONTEXT_PLATFORM,
                                       (cl_context_properties)m_platform, 0},
                        badProps3[] = {1, 0, 0}; // unsupported property name

  cl_context context_default =
      clCreateContextFromType(prop, gDeviceType, NULL, NULL, &iRet);
  ASSERT_OCL_SUCCESS(iRet, "Create Context from type (gDeviceType)");

  clCreateContextFromType(badProps2, gDeviceType, NULL, NULL, &iRet);
  ASSERT_EQ(iRet, CL_INVALID_PROPERTY)
      << "Create Context from type (duplicate property name) should fail";

  clCreateContextFromType(badProps3, gDeviceType, NULL, NULL, &iRet);
  ASSERT_EQ(iRet, CL_INVALID_PROPERTY)
      << "Create Context from type (unsupported property name should fail";

  iRet = clGetDeviceIDs(m_platform,
                        CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_GPU |
                            CL_DEVICE_TYPE_ACCELERATOR,
                        0, NULL, &uiNumDevices);
  ASSERT_OCL_SUCCESS(iRet, "clGetDeviceIDs get num_devices");

  pDevices = new cl_device_id[uiNumDevices];
  iRet = clGetDeviceIDs(m_platform,
                        CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_GPU |
                            CL_DEVICE_TYPE_ACCELERATOR,
                        uiNumDevices, pDevices, NULL);
  ASSERT_OCL_SUCCESS(iRet, "clGetDeviceIDs");

  context = clCreateContext(prop, uiNumDevices, pDevices, NULL, NULL, &iRet);
  ASSERT_OCL_SUCCESS(iRet, "clCreateContext");
  printf("context = %p\n", (void *)context);

  cl_context context2 =
      clCreateContextFromType(prop, gDeviceType, NULL, NULL, &iRet);
  ASSERT_OCL_SUCCESS(iRet, "clCreateContextFromType from gDeviceType");
  printf("context2 = %p\n", (void *)context2);

  // Query for number of devices in a context, should be 1
  cl_uint uiCntxCnt = 0;
  iRet = clGetContextInfo(context, CL_CONTEXT_NUM_DEVICES, sizeof(cl_uint),
                          &uiCntxCnt, NULL);
  ASSERT_OCL_SUCCESS(iRet, "clGetContextInfo CL_CONTEXT_NUM_DEVICES");
  printf("Number of devices in a context is %d\n", (int)uiCntxCnt);

  cl_uint uiCntxRefCnt = 0;
  iRet = clGetContextInfo(context, CL_CONTEXT_REFERENCE_COUNT, sizeof(cl_uint),
                          &uiCntxRefCnt, NULL);
  ASSERT_OCL_SUCCESS(iRet, "clGetContextInfo CL_CONTEXT_REFERENCE_COUNT");
  printf("context ref count = %d\n", uiCntxRefCnt);
  iRet = clRetainContext(context);
  ASSERT_OCL_SUCCESS(iRet, "clRetainContext");
  iRet = clGetContextInfo(context, CL_CONTEXT_REFERENCE_COUNT, sizeof(cl_uint),
                          &uiCntxRefCnt, NULL);
  ASSERT_OCL_SUCCESS(iRet, "clGetContextInfo CL_CONTEXT_REFERENCE_COUNT");
  printf("context ref count (after retain) = %d\n", uiCntxRefCnt);

  // Release contexts
  for (cl_uint ui = 0; ui < uiCntxRefCnt; ui++) {
    clReleaseContext(context);
  }
  clReleaseContext(context2);
  clReleaseContext(context_default);
  delete[] pDevices;
}

class ContextThread : public SynchronizedThread {
public:
  ContextThread(cl_device_id device) : m_device(device), m_result(false) {}

  virtual ~ContextThread() {}

  bool GetResult() const { return m_result; }

protected:
  virtual void ThreadRoutine() {
    cl_int err;
    cl_context context =
        clCreateContext(nullptr, 1, &m_device, nullptr, nullptr, &err);
    if (CL_FAILED(err))
      return;

    err = clReleaseContext(context);
    if (CL_FAILED(err))
      return;

    m_result = true;
  }

  cl_device_id m_device;
  bool m_result;
};

TEST_F(CreateContextTest, MultiThreads) {
  cl_device_id device;
  cl_int err = clGetDeviceIDs(m_platform, gDeviceType, 1, &device, nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetDeviceIDs");

  size_t numThreads = 10;

  std::vector<SynchronizedThread *> threads(numThreads);

  for (size_t i = 0; i < numThreads; ++i)
    threads[i] = new ContextThread(device);

  SynchronizedThreadPool pool;
  pool.Init(&threads[0], numThreads);
  pool.StartAll();
  pool.WaitAll();

  bool res = true;
  for (size_t i = 0; i < numThreads && res; ++i) {
    res &= static_cast<ContextThread *>(threads[i])->GetResult();
  }
  ASSERT_TRUE(res) << "ContextThread failed";

  for (size_t i = 0; i < numThreads; ++i)
    delete threads[i];
}
