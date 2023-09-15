// Copyright (c) 2006 Intel Corporation
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

#include "CL/cl.h"
#include "FrameworkTestThreads.h"
#include "gtest_wrapper.h"
#include "tbb/global_control.h"
#include "test_utils.h"
#include <cstdlib>
#include <cstring>
#include <iostream>

#define BUF_SIZE 1024

using namespace std;

extern cl_device_type gDeviceType;

static const char *sProg =
    "struct Node\n"
    "{\n"
    "  int m_num;\n"
    "  global struct Node* m_pNext;\n"
    "};\n"
    "\n"
    "kernel void SumIntsInLinkedList(global struct Node* pHead, global int* "
    "pResult)\n"
    "{\n"
    "  *pResult = 0;\n"
    "  while (pHead != (global void*)0)\n"
    "  {\n"
    "    *pResult += pHead->m_num;\n"
    "    pHead = pHead->m_pNext;\n"
    "  }\n"
    "}\n"
    "\n"
    "kernel void SumInts(global int* iArr, int szArrSize, global int* pSum, "
    "global int* pNull)\n"
    "{\n"
    "  *pSum = 0;\n"
    "  for (size_t i = 0; i < szArrSize; i++)\n"
    "  {\n"
    "    *pSum += iArr[i];\n"
    "  }\n"
    "}\n"
    "\n"
    "kernel void migrate_kernel(global int* a)\n"
    "{\n"
    "  a[get_global_id(0)] = 0;\n"
    "}\n";

struct Node {
  cl_int m_num;
  Node *m_pNext;
};

static void TestSetKernelExecInfo(cl_context context, cl_device_id device,
                                  cl_command_queue queue, cl_program prog) {
  const cl_int szListSize = 1000;
  std::vector<Node *> nodes;
  for (cl_int i = 0; i < szListSize; i++) {
    Node *const pNode = (Node *)clSVMAlloc(
        context, CL_MEM_SVM_FINE_GRAIN_BUFFER, sizeof(Node), 0);
    CheckException("clSVMAlloc", pNode != NULL, true);
    pNode->m_num = i;
    if (i == szListSize - 1) {
      pNode->m_pNext = NULL;
    }
    if (i > 0) {
      nodes[i - 1]->m_pNext = pNode;
    }
    nodes.push_back(pNode);
  }
  cl_int *const piResult = (cl_int *)clSVMAlloc(
      context, CL_MEM_SVM_FINE_GRAIN_BUFFER, sizeof(cl_int), 0);
  CheckException("clSVMAlloc", piResult != NULL, true);

  cl_int iRet;
  cl_kernel kernel = clCreateKernel(prog, "SumIntsInLinkedList", &iRet);
  CheckException("clCreateKernel", CL_SUCCESS, iRet);
  iRet = clSetKernelArgSVMPointer(kernel, 0, nodes[0]);
  CheckException("clSetKernelArgSVMPointer", CL_SUCCESS, iRet);
  iRet = clSetKernelArgSVMPointer(kernel, 1, piResult);
  CheckException("clSetKernelArgSVMPointer", CL_SUCCESS, iRet);
  const cl_bool bFineGrainSystem = CL_FALSE;
  iRet = clSetKernelExecInfo(kernel, CL_KERNEL_EXEC_INFO_SVM_FINE_GRAIN_SYSTEM,
                             sizeof(bFineGrainSystem), &bFineGrainSystem);
  CheckException("clSetKernelExecInfo", CL_SUCCESS, iRet);
  iRet = clSetKernelExecInfo(kernel, CL_KERNEL_EXEC_INFO_SVM_PTRS,
                             (nodes.size() - 1) * sizeof(void *), &nodes[1]);
  CheckException("clSetKernelExecInfo", CL_SUCCESS, iRet);

  const size_t szGlobalWorkOffset = 0, szWorkSize = 1;
  iRet = clEnqueueNDRangeKernel(queue, kernel, 1, &szGlobalWorkOffset,
                                &szWorkSize, NULL, 0, NULL, NULL);
  CheckException("clEnqueueNDRangeKernel", CL_SUCCESS, iRet);
  iRet = clFinish(queue);
  CheckException("clFinish", CL_SUCCESS, iRet);

  const cl_int iExpected = szListSize * (2 * nodes[0]->m_num + szListSize - 1) /
                           2; // sum of arithmetic progression
  if (iExpected != *piResult) {
    throw exception();
  }

  iRet = clReleaseKernel(kernel);
  CheckException("clReleaseKernel", CL_SUCCESS, iRet);
  for (std::vector<Node *>::iterator iter = nodes.begin(); iter != nodes.end();
       iter++) {
    clSVMFree(context, *iter);
  }
  clSVMFree(context, piResult);
}

static void TestSetKernelArgSVMPointer(cl_context context, cl_device_id device,
                                       cl_command_queue queue, cl_program prog,
                                       bool bSysPtrs) {
  cl_int *const piArr =
      bSysPtrs ? (cl_int *)malloc(BUF_SIZE)
               : (cl_int *)clSVMAlloc(context, CL_MEM_SVM_FINE_GRAIN_BUFFER,
                                      BUF_SIZE, 0);
  CheckException(bSysPtrs ? "malloc" : "clSVMAlloc", piArr != NULL, true);
  cl_int *const piResult =
      bSysPtrs ? (cl_int *)malloc(BUF_SIZE)
               : (cl_int *)clSVMAlloc(context, CL_MEM_SVM_FINE_GRAIN_BUFFER,
                                      sizeof(cl_int), 0);
  CheckException(bSysPtrs ? "malloc" : "clSVMAlloc", piArr != NULL, true);

  // initialize piArr
  for (size_t i = 0; i < BUF_SIZE / sizeof(cl_int); i++) {
    piArr[i] = i;
  }

  const cl_int iStartIndex = BUF_SIZE / sizeof(cl_int) / 2,
               iNumElems = BUF_SIZE / sizeof(cl_int) / 2;
  const cl_int iExpected = iNumElems *
                           (2 * piArr[iStartIndex] + iNumElems - 1) /
                           2; // sum of arithmetic progression
  cl_int iRet;
  cl_kernel kernel = clCreateKernel(prog, "SumInts", &iRet);
  CheckException("clCreateKernel", CL_SUCCESS, iRet);
  iRet = clSetKernelArgSVMPointer(kernel, 0, &piArr[iStartIndex]);
  CheckException("clSetKernelArgSVMPointer", CL_SUCCESS, iRet);
  iRet = clSetKernelArg(kernel, 1, sizeof(cl_int), &iNumElems);
  CheckException("clSetKernelArg", CL_SUCCESS, iRet);
  iRet = clSetKernelArgSVMPointer(kernel, 2, piResult);
  CheckException("clSetKernelArgSVMPointer", CL_SUCCESS, iRet);
  iRet = clSetKernelArgSVMPointer(kernel, 3, piResult);
  CheckException("clSetKernelArgSVMPointer", CL_SUCCESS, iRet);

  const size_t szGlobalWorkOffset = 0, szWorkSize = 1;
  cl_event e;
  iRet = clEnqueueNDRangeKernel(queue, kernel, 1, &szGlobalWorkOffset,
                                &szWorkSize, NULL, 0, NULL, &e);
  CheckException("clEnqueueNDRangeKernel", CL_SUCCESS, iRet);
  iRet = clWaitForEvents(1, &e);
  CheckException("clWaitForEvents", CL_SUCCESS, iRet);

  if (iExpected != *piResult) {
    throw exception();
  }
  // Check the handling of null svm pointer won't be affected
  // by the first enqueue
  iRet = clSetKernelArgSVMPointer(kernel, 3, nullptr);
  CheckException("clSetKernelArgSVMPointer", CL_SUCCESS, iRet);

  iRet = clEnqueueNDRangeKernel(queue, kernel, 1, &szGlobalWorkOffset,
                                &szWorkSize, NULL, 0, NULL, &e);
  CheckException("clEnqueueNDRangeKernel", CL_SUCCESS, iRet);
  iRet = clWaitForEvents(1, &e);
  CheckException("clWaitForEvents", CL_SUCCESS, iRet);

  iRet = clReleaseKernel(kernel);
  CheckException("clReleaseKernel", CL_SUCCESS, iRet);
  if (bSysPtrs) {
    free(piArr);
    free(piResult);
  } else {
    clSVMFree(context, piArr);
    clSVMFree(context, piResult);
  }
}

class SetKernelArgSVMPointerThread : public SynchronizedThread {
public:
  SetKernelArgSVMPointerThread(cl_context context, cl_device_id device,
                               cl_command_queue queue, cl_program prog)
      : m_context(context), m_device(device), m_queue(queue), m_prog(prog) {}

  bool getResult() const { return m_result; }

protected:
  void ThreadRoutine() override {
    try {
      TestSetKernelArgSVMPointer(m_context, m_device, m_queue, m_prog, false);
      m_result = true;
    } catch (const std::exception(&)) {
      m_result = false;
    }
  }

  cl_context m_context;
  cl_device_id m_device;
  cl_command_queue m_queue;
  cl_program m_prog;
  bool m_result;
};

void TestSetKernelArgSVMPointerMultiThreads(cl_context context,
                                            cl_device_id device,
                                            cl_command_queue queue,
                                            cl_program prog) {
  unsigned numThreads = getMaxNumExternalThreads();

  std::vector<SynchronizedThread *> threads(numThreads);
  for (size_t i = 0; i < numThreads; ++i)
    threads[i] = new SetKernelArgSVMPointerThread(context, device, queue, prog);

  SynchronizedThreadPool pool;
  pool.Init(&threads[0], numThreads);
  pool.StartAll();
  pool.WaitAll();

  for (size_t i = 0; i < numThreads; ++i) {
    bool res =
        static_cast<SetKernelArgSVMPointerThread *>(threads[i])->getResult();
    CheckException("SetKernelArgSVMPointerThread", true, res);
  }

  for (size_t i = 0; i < numThreads; ++i)
    delete threads[i];
}

typedef void(CL_CALLBACK *pfnFreeFunc)(cl_command_queue queue,
                                       cl_uint uiNumSvmPtrs, void *pSvmPtrs[],
                                       void *pUserData);

static void CL_CALLBACK SVMFreeCallback(cl_command_queue queue,
                                        cl_uint uiNumSvmPtrs, void *pSvmPtrs[],
                                        void *pUserData) {
  cout << "Executing SVMFreeCallback" << endl;
  for (cl_uint i = 0; i < uiNumSvmPtrs; i++) {
    clSVMFree(*(cl_context *)pUserData, pSvmPtrs[i]);
  }
}

enum InitMethod { MEMCPY, MEM_FILL, MAP_UNMAP };

static const char pattern[] = "hello world SVM";

static void InitBuffers(InitMethod method, cl_command_queue queue,
                        void *pSrcBuf, void *pSvmPtr) {
  cl_int iRet = clEnqueueSVMMemFill(queue, pSrcBuf, pattern, sizeof(pattern),
                                    BUF_SIZE, 0, NULL, NULL);
  CheckException("clEnqueueSVMMemFill", CL_SUCCESS, iRet);

  switch (method) {
  case MEMCPY:
    iRet = clEnqueueSVMMemcpy(queue, CL_FALSE, pSvmPtr, pSrcBuf, BUF_SIZE, 0,
                              NULL, NULL);
    CheckException("clEnqueueSVMMemcpy", CL_SUCCESS, iRet);
    break;
  case MEM_FILL:
    iRet = clEnqueueSVMMemFill(queue, pSvmPtr, pattern, sizeof(pattern),
                               BUF_SIZE, 0, NULL, NULL);
    CheckException("clEnqueueSVMMemFill", CL_SUCCESS, iRet);
    break;
  case MAP_UNMAP:
    iRet = clEnqueueSVMMap(queue, CL_TRUE, CL_MAP_WRITE_INVALIDATE_REGION,
                           pSvmPtr, BUF_SIZE, 0, NULL, NULL);
    CheckException("clEnqueueSVMMap", CL_SUCCESS, iRet);
    MEMCPY_S(pSvmPtr, BUF_SIZE, pSrcBuf, BUF_SIZE);
    iRet = clEnqueueSVMUnmap(queue, pSvmPtr, 0, NULL, NULL);
    CheckException("clEnqueueSVMUnmap", CL_SUCCESS, iRet);
    break;
  }
}

static void TestEnqueueSVMCommands(cl_context context, cl_command_queue queue,
                                   InitMethod initMethod,
                                   bool bEnqueueFree = false,
                                   pfnFreeFunc freeFunc = NULL,
                                   void *pUserData = NULL) {
  void *const pSvmPtr1 = clSVMAlloc(context, CL_MEM_SVM_FINE_GRAIN_BUFFER,
                                    BUF_SIZE, 0),
              *const pSvmPtr2 = clSVMAlloc(
                  context, CL_MEM_SVM_FINE_GRAIN_BUFFER, BUF_SIZE, 0),
              *const pSvmPtr3 = clSVMAlloc(
                  context, CL_MEM_SVM_FINE_GRAIN_BUFFER, BUF_SIZE, 0),
              *const pSvmPtr4 = clSVMAlloc(
                  context, CL_MEM_SVM_FINE_GRAIN_BUFFER, BUF_SIZE, 0);
  CheckException("clSVMAlloc", pSvmPtr1 != NULL, true);
  CheckException("clSVMAlloc", pSvmPtr2 != NULL, true);
  CheckException("clSVMAlloc", pSvmPtr3 != NULL, true);
  CheckException("clSVMAlloc", pSvmPtr4 != NULL, true);

  cl_int iRet;
  // create a buffer from the whole of pSvmPtr3
  cl_mem buf3 =
      clCreateBuffer(context, CL_MEM_USE_HOST_PTR, BUF_SIZE, pSvmPtr3, &iRet);
  CheckException("clCreateBuffer", CL_SUCCESS, iRet);
  // create 2 buffers from the 2 halves of pSvmPtr4
  cl_mem buf4a = clCreateBuffer(context, CL_MEM_USE_HOST_PTR, BUF_SIZE / 2,
                                pSvmPtr4, &iRet);
  CheckException("clCreateBuffer", CL_SUCCESS, iRet);
  cl_mem buf4b = clCreateBuffer(context, CL_MEM_USE_HOST_PTR, BUF_SIZE / 2,
                                (char *)pSvmPtr4 + BUF_SIZE / 2, &iRet);
  CheckException("clCreateBuffer", CL_SUCCESS, iRet);

  // create 2 sub-buffers from the 2 halves of buf4a
  cl_buffer_region regionA, regionB;
  regionA.origin = 0;
  regionB.origin = BUF_SIZE / 4;
  regionA.size = regionB.size = BUF_SIZE / 4;
  cl_mem subBuf4aa = clCreateSubBuffer(buf4a, 0, CL_BUFFER_CREATE_TYPE_REGION,
                                       &regionA, &iRet);
  CheckException("clCreateSubBuffer", CL_SUCCESS, iRet);
  cl_mem subBuf4ab = clCreateSubBuffer(buf4a, 0, CL_BUFFER_CREATE_TYPE_REGION,
                                       &regionB, &iRet);
  CheckException("clCreateSubBuffer", CL_SUCCESS, iRet);

  char *srcBuf
#ifdef _WIN32
      = (char *)_aligned_malloc(
          BUF_SIZE,
          sizeof(pattern)); // srcBuf is initialized using clEnqueueSVMMemFill
#else
      ;
  int res = posix_memalign((void **)&srcBuf, sizeof(pattern), BUF_SIZE);
  assert(0 == res);
  (void)res;
#endif
  char middleBuf[BUF_SIZE], dstBuf[BUF_SIZE];
  // pass the data from srcBuf to dstBuf through the SVM buffers, cl_mem buffers
  // and sub-buffers
  InitBuffers(initMethod, queue, srcBuf, pSvmPtr1);
  cl_event memcpyEvent;
  iRet = clEnqueueSVMMemcpy(
      queue, CL_FALSE, pSvmPtr2, pSvmPtr1, BUF_SIZE, 0, NULL,
      &memcpyEvent); // check a path in the code with event dependency
  CheckException("clEnqueueSVMMemcpy", CL_SUCCESS, iRet);
  iRet = clEnqueueSVMMemcpy(queue, CL_FALSE, middleBuf, pSvmPtr2, BUF_SIZE, 1,
                            &memcpyEvent, NULL);
  CheckException("clEnqueueSVMMemcpy", CL_SUCCESS, iRet);
  iRet = clEnqueueWriteBuffer(queue, buf3, CL_FALSE, 0, BUF_SIZE, middleBuf, 0,
                              NULL, NULL);
  CheckException("clEnqueueWriteBuffer", CL_SUCCESS, iRet);
  iRet = clEnqueueCopyBuffer(queue, buf3, buf4a, 0, 0, BUF_SIZE / 2, 0, NULL,
                             NULL);
  CheckException("clEnqueueCopyBuffer", CL_SUCCESS, iRet);
  iRet = clEnqueueCopyBuffer(queue, buf3, buf4b, BUF_SIZE / 2, 0, BUF_SIZE / 2,
                             0, NULL, NULL);
  CheckException("clEnqueueCopyBuffer", CL_SUCCESS, iRet);
  iRet = clEnqueueReadBuffer(queue, subBuf4aa, CL_FALSE, 0, BUF_SIZE / 4,
                             dstBuf, 0, NULL, NULL);
  CheckException("clEnqueueReadBuffer", CL_SUCCESS, iRet);
  iRet = clEnqueueReadBuffer(queue, subBuf4ab, CL_FALSE, 0, BUF_SIZE / 4,
                             &dstBuf[BUF_SIZE / 4], 0, NULL, NULL);
  CheckException("clEnqueueReadBuffer", CL_SUCCESS, iRet);
  iRet = clEnqueueReadBuffer(queue, buf4b, CL_TRUE, 0, BUF_SIZE / 2,
                             &dstBuf[BUF_SIZE / 2], 0, NULL, NULL);
  CheckException("clEnqueueReadBuffer", CL_SUCCESS, iRet);

  bool bDiff = false;
  for (size_t i = 0; i < BUF_SIZE; i++) {
    if (srcBuf[i] != dstBuf[i]) {
      cout << "srcBuf differs from dstBuf at index " << i << endl;
      bDiff = true;
      break;
    }
  }

  iRet = clReleaseMemObject(subBuf4aa);
  CheckException("clReleaseMemObject", CL_SUCCESS, iRet);
  iRet = clReleaseMemObject(subBuf4ab);
  CheckException("clReleaseMemObject", CL_SUCCESS, iRet);
  iRet = clReleaseMemObject(buf3);
  CheckException("clReleaseMemObject", CL_SUCCESS, iRet);
  iRet = clReleaseMemObject(buf4a);
  CheckException("clReleaseMemObject", CL_SUCCESS, iRet);
  iRet = clReleaseMemObject(buf4b);
  CheckException("clReleaseMemObject", CL_SUCCESS, iRet);

  if (!bEnqueueFree) {
    clSVMFree(
        context,
        NULL); // check that we handle NULL pointer correctly and don't crash
    clSVMFree(context, pSvmPtr1);
    clSVMFree(context, pSvmPtr2);
    clSVMFree(context, pSvmPtr3);
    clSVMFree(context, pSvmPtr4);
  } else {
    void *pSvmPtrs[] = {pSvmPtr1, pSvmPtr2, pSvmPtr3, pSvmPtr4};
    iRet = clEnqueueSVMFree(queue, sizeof(pSvmPtrs) / sizeof(pSvmPtrs[0]),
                            pSvmPtrs, freeFunc, pUserData, 0, NULL, NULL);
    CheckException("clEnqueueSVMFree", CL_SUCCESS, iRet);
    iRet = clFinish(queue);
    CheckException("clFinish", CL_SUCCESS, iRet);
  }
#ifdef _WIN32
  _aligned_free(srcBuf);
#else
  free(srcBuf);
#endif
  if (bDiff) {
    throw exception();
  }
}

static void TestEnqueueSVMRuntimeCommand(cl_context context,
                                         cl_command_queue queue) {
  size_t len = 256;
  std::vector<int> src(len, 2);
  std::vector<int> dst(len);

  // clEnqueueSVMMemcpy
  cl_int err;
  cl_event event;
  err = clEnqueueSVMMemcpy(queue, CL_TRUE, &dst[0], &src[0], len * sizeof(int),
                           0, nullptr, &event);
  CheckException("clEnqueueSVMMemcpy", CL_SUCCESS, err);

  for (size_t i = 0; i < len; ++i)
    ASSERT_EQ(dst[i], src[i]);

  // Check elapsed time
  cl_ulong start, end;
  err = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START,
                                sizeof(cl_ulong), &start, nullptr);
  CheckException("clGetEventProfilingInfo CL_PROFILING_COMMAND_START",
                 CL_SUCCESS, err);
  err = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END,
                                sizeof(cl_ulong), &end, nullptr);
  CheckException("clGetEventProfilingInfo CL_PROFILING_COMMAND_END", CL_SUCCESS,
                 err);
  EXPECT_NE(start, end) << "Command elapsed time should not be zero";

  // clEnqueueSVMMemFill
  const char pattern[4] = {'0', '1', '2', '3'};
  size_t patternSize = sizeof(pattern) / sizeof(pattern[0]);
  size_t bufSize = patternSize * len;
  std::vector<char> buffer(bufSize);
  cl_event event2;
  cl_int iRet = clEnqueueSVMMemFill(queue, &buffer[0], pattern, sizeof(pattern),
                                    bufSize, 0, nullptr, &event2);
  CheckException("clEnqueueSVMMemFill", CL_SUCCESS, iRet);

  int countErrors = 0;
  for (size_t i = 0; i < bufSize; i += patternSize)
    if (0 != strncmp(&buffer[i], pattern, patternSize))
      countErrors++;
  ASSERT_EQ(countErrors, 0);

  // Check elapsed time
  err = clGetEventProfilingInfo(event2, CL_PROFILING_COMMAND_START,
                                sizeof(cl_ulong), &start, nullptr);
  CheckException("clGetEventProfilingInfo CL_PROFILING_COMMAND_START",
                 CL_SUCCESS, err);
  err = clGetEventProfilingInfo(event2, CL_PROFILING_COMMAND_END,
                                sizeof(cl_ulong), &end, nullptr);
  CheckException("clGetEventProfilingInfo CL_PROFILING_COMMAND_END", CL_SUCCESS,
                 err);
  EXPECT_NE(start, end) << "Command elapsed time should not be zero";
}

static void TestMigrate(cl_context context, cl_device_id device,
                        cl_command_queue queue, cl_program prog) {
  cl_int err;

  // Create another command queue
  cl_command_queue queue2 =
      clCreateCommandQueueWithProperties(context, device, nullptr, &err);
  CheckException("clCreateCommandQueueWithProperties", CL_SUCCESS, err);

  cl_kernel kernel = clCreateKernel(prog, "migrate_kernel", &err);
  CheckException("clCreateKernel", CL_SUCCESS, err);

  // Create SVM buffer
  size_t global_size = 256;
  char *buf = (char *)clSVMAlloc(context, CL_MEM_READ_WRITE,
                                 global_size * sizeof(cl_uint), 16);
  if (!buf) {
    printf("clSVMAlloc failed\n");
    throw exception();
  }

  err = clSetKernelArgSVMPointer(kernel, 0, (void *)buf);
  CheckException("clSetKernelArgSVMPointer", CL_SUCCESS, err);

  // Enqueue Migrate command to the first queue
  const char *ptrs[] = {buf};
  const size_t szs[] = {13};
  cl_event evt;
  err = clEnqueueSVMMigrateMem(queue, 1, (const void **)ptrs, szs, 0, 0,
                               nullptr, &evt);
  CheckException("clEnqueueSVMMigrateMem", CL_SUCCESS, err);

  // OpenCL 3.0: Test event command type CL_COMMAND_SVM_MIGRATE_MEM.
  cl_command_type command_type;
  err = clGetEventInfo(evt, CL_EVENT_COMMAND_TYPE, sizeof(cl_command_type),
                       &command_type, nullptr);
  CheckException("clGetEventInfo", CL_SUCCESS, err);
  ASSERT_EQ(command_type, CL_COMMAND_SVM_MIGRATE_MEM);

  cl_event event;
  err = clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &global_size, nullptr,
                               0, nullptr, &event);
  CheckException("clEnqueueNDRangeKernel", CL_SUCCESS, err);

  // Enqueue Migrate command to the second queue
  const char *ptrs2[] = {buf};
  const size_t szs2[] = {0};
  err = clEnqueueSVMMigrateMem(queue2, 1, (const void **)ptrs2, szs2, 0, 1,
                               &event, nullptr);
  CheckException("clEnqueueSVMMigrateMem", CL_SUCCESS, err);
  err = clEnqueueNDRangeKernel(queue2, kernel, 1, nullptr, &global_size,
                               nullptr, 0, nullptr, nullptr);
  CheckException("clEnqueueNDRangeKernel", CL_SUCCESS, err);

  err = clFinish(queue);
  CheckException("clFinish", CL_SUCCESS, err);
  err = clFinish(queue2);
  CheckException("clFinish", CL_SUCCESS, err);

  // Release resources
  clSVMFree(context, buf);
  clReleaseEvent(evt);
  clReleaseEvent(event);
  clReleaseCommandQueue(queue2);
}

/// Call clCreateBuffer with a pointer returned by clSVMAlloc as its host_ptr
/// argument, and CL_MEM_USE_HOST_PTR is set in its flags argument.
/// clCreateBuffer should succeed if its size not out of bound.
static void TestCreateBufferWithOffset(cl_context context) {
  size_t size = 256;
  cl_uint alignment = 16;
  char *svmBuffer =
      (char *)clSVMAlloc(context, CL_MEM_READ_WRITE, size, alignment);
  if (!svmBuffer)
    throw exception();

  cl_int err;
  size_t offset1 = size / 2 - 1;
  size_t size1 = size / 2;
  cl_mem memBuffer1 = clCreateBuffer(context, CL_MEM_USE_HOST_PTR, size1,
                                     svmBuffer + offset1, &err);
  CheckException("clCreateBuffer", CL_SUCCESS, err);
  if (!memBuffer1)
    throw exception();

  size_t offset2 = size / 2 + 1;
  size_t size2 = size / 2;
  cl_mem memBuffer2 = clCreateBuffer(context, CL_MEM_USE_HOST_PTR, size2,
                                     svmBuffer + offset2, &err);
  if (CL_SUCCESS == err || memBuffer2)
    throw exception();
}

bool clSvmTest() {
  cl_int iRet = CL_SUCCESS;
  cl_platform_id platform = 0;
  bool bResult = true;
  cl_device_id device = NULL;
  cl_context context = NULL;
  cl_command_queue queue = NULL;
  cl_program prog = NULL;
  cl_device_svm_capabilities svm_caps;
  size_t temp;

  std::cout << "============================================================="
            << std::endl;
  std::cout << "clTestEnqueueSVMCommands" << std::endl;
  std::cout << "============================================================="
            << std::endl;

  try {
    iRet = clGetPlatformIDs(1, &platform, NULL);
    CheckException("clGetPlatformIDs", CL_SUCCESS, iRet);
    iRet = clGetDeviceIDs(platform, gDeviceType, 1, &device, NULL);
    CheckException("clGetDeviceIDs", CL_SUCCESS, iRet);
    iRet = clGetDeviceInfo(device, CL_DEVICE_SVM_CAPABILITIES, sizeof(svm_caps),
                           &svm_caps, &temp);
    CheckException("clGetDeviceInfo(CL_DEVICE_SVM_CAPABILITIES)", CL_SUCCESS,
                   iRet);

    const cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                           (cl_context_properties)platform, 0};
    context = clCreateContextFromType(prop, gDeviceType, NULL, NULL, &iRet);
    CheckException("clCreateContextFromType", CL_SUCCESS, iRet);

    cl_command_queue_properties properties[] = {CL_QUEUE_PROPERTIES,
                                                CL_QUEUE_PROFILING_ENABLE, 0};
#if _WIN32
#pragma warning(suppress : 4996)
#endif
    queue =
        clCreateCommandQueueWithProperties(context, device, properties, &iRet);
    CheckException("clCreateCommandQueueWithProperties", CL_SUCCESS, iRet);

    TestCreateBufferWithOffset(context);

    TestEnqueueSVMCommands(context, queue, MEMCPY);
    TestEnqueueSVMCommands(context, queue, MEM_FILL);
    TestEnqueueSVMCommands(context, queue, MAP_UNMAP);
    TestEnqueueSVMCommands(context, queue, MEMCPY, true);
    TestEnqueueSVMCommands(context, queue, MEMCPY, true, SVMFreeCallback,
                           &context);

    TestEnqueueSVMRuntimeCommand(context, queue);

    const size_t szLengths = {strlen(sProg)};
    cl_program prog =
        clCreateProgramWithSource(context, 1, &sProg, &szLengths, &iRet);
    CheckException("clCreateProgramWithSource", CL_SUCCESS, iRet);
    iRet = clBuildProgram(prog, 1, &device, NULL, NULL, NULL);
    CheckException("clBuildProgram", CL_SUCCESS, iRet);
    TestSetKernelArgSVMPointer(context, device, queue, prog, false);
    TestSetKernelArgSVMPointer(context, device, queue, prog, true);
    TestSetKernelArgSVMPointerMultiThreads(context, device, queue, prog);
    TestSetKernelExecInfo(context, device, queue, prog);
    TestMigrate(context, device, queue, prog);

    cl_device_svm_capabilities svmCaps;
    iRet = clGetDeviceInfo(device, CL_DEVICE_SVM_CAPABILITIES, sizeof(svmCaps),
                           &svmCaps, NULL);
    CheckException("clGetDeviceInfo", CL_SUCCESS, iRet);
    CheckException(
        "clGetDeviceInfo",
        (cl_device_svm_capabilities)(CL_DEVICE_SVM_COARSE_GRAIN_BUFFER |
                                     CL_DEVICE_SVM_FINE_GRAIN_BUFFER |
                                     CL_DEVICE_SVM_FINE_GRAIN_SYSTEM |
                                     CL_DEVICE_SVM_ATOMICS),
        svmCaps);
  } catch (const std::exception &) {
    bResult = false;
  }
  if (context) {
    clReleaseContext(context);
  }
  if (queue) {
    clReleaseCommandQueue(queue);
  }
  if (prog) {
    clReleaseProgram(prog);
  }
  return bResult;
}
