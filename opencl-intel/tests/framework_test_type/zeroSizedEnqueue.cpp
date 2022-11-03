#include "CL21.h"

static void UnlockEventAndFinishQueue(cl_event block_event,
                                      cl_event blocked_event,
                                      const std::string &opName) {
  cl_int iRet = CL_SUCCESS;

  cl_int status = -1;
  iRet = clGetEventInfo(blocked_event, CL_EVENT_COMMAND_EXECUTION_STATUS,
                        sizeof(status), &status, nullptr);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clGetEventInfo failed. ";
  ASSERT_NE(CL_COMPLETE, status) << opName << "(zero sized enqueue) failed. ";

  iRet = clSetUserEventStatus(block_event, CL_COMPLETE);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clSetUserEventStatus failed. ";

  iRet = clWaitForEvents(/*number of events*/ 1, &blocked_event);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clWaitForEvents failed. ";

  status = -1;
  iRet = clGetEventInfo(blocked_event, CL_EVENT_COMMAND_EXECUTION_STATUS,
                        sizeof(status), &status, nullptr);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clGetEventInfo failed. ";
  ASSERT_EQ(CL_COMPLETE, status) << opName << "(zero sized enqueue) failed. ";
}

TEST_F(CL21, ZeroSized_clEnqueueReadBuffer) {
  cl_int iRet = CL_SUCCESS;

  cl_event read_event = nullptr;
  const size_t buffer_size = 8;
  std::vector<unsigned char> buffer(buffer_size, 0);
  std::vector<unsigned char> buffer_ref(buffer_size, 5);

  cl_mem cl_buffer = clCreateBuffer(m_context, CL_MEM_COPY_HOST_PTR,
                                    buffer_ref.size(), &buffer_ref[0], &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateBuffer failed. ";

  buffer_ref = buffer;

  cl_event block_event = clCreateUserEvent(m_context, &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateUserEvent failed. ";

  iRet =
      clEnqueueReadBuffer(m_queue, cl_buffer, CL_FALSE,
                          /*offset*/ 0,
                          /*size*/ 0, &buffer[0], 1, &block_event, &read_event);
  ASSERT_EQ(CL_SUCCESS, iRet)
      << " clEnqueueReadBuffer(zero sized enqueue) failed. ";
  ASSERT_NO_FATAL_FAILURE(UnlockEventAndFinishQueue(block_event, read_event,
                                                    "clEnqueueReadBuffer"));

  ASSERT_TRUE(buffer == buffer_ref) << " clEnqueueReadBuffer(zero sized "
                                       "enqueue) ref and actual datas differ.";
}

TEST_F(CL21, ZeroSized_clEnqueueWriteBuffer) {
  cl_int iRet = CL_SUCCESS;

  cl_event write_event = nullptr;
  const size_t buffer_size = 8;
  std::vector<unsigned char> buffer(buffer_size, 0);
  std::vector<unsigned char> buffer_ref(buffer_size, 5);

  cl_mem cl_buffer = clCreateBuffer(m_context, CL_MEM_COPY_HOST_PTR,
                                    buffer_ref.size(), &buffer_ref[0], &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateBuffer failed. ";

  cl_event block_event = clCreateUserEvent(m_context, &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateUserEvent failed. ";

  iRet = clEnqueueWriteBuffer(m_queue, cl_buffer, CL_FALSE,
                              /*offset*/ 0,
                              /*size*/ 0, &buffer[0], 1, &block_event,
                              &write_event);
  ASSERT_EQ(CL_SUCCESS, iRet)
      << " clEnqueueWriteBuffer(zero sized enqueue) failed. ";

  ASSERT_NO_FATAL_FAILURE(UnlockEventAndFinishQueue(block_event, write_event,
                                                    "clEnqueueWriteBuffer"));

  iRet = clFinish(m_queue);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clFinish failed. ";

  iRet = clEnqueueReadBuffer(m_queue, cl_buffer, CL_TRUE,
                             /*offset*/ 0, buffer_size, &buffer[0],
                             /*num_events*/ 0, nullptr, nullptr);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clEnqueueReadBuffer failed. ";

  ASSERT_TRUE(buffer == buffer_ref) << " clEnqueueWriteBuffer(zero sized "
                                       "enqueue) ref and actual datas differ.";
}

TEST_F(CL21, ZeroSized_clEnqueueNDRangeKernel) {
  cl_int iRet = CL_SUCCESS;

  cl_event enqueue_event = nullptr;
  cl_kernel kern = nullptr;
  ASSERT_NO_FATAL_FAILURE(GetDummyKernel(kern));

  cl_event block_event = clCreateUserEvent(m_context, &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateUserEvent failed. ";

  iRet = clEnqueueNDRangeKernel(m_queue, kern,
                                /*work_dim*/ 1,
                                /*global_work_offset*/ nullptr,
                                /*global_work_size*/ nullptr,
                                /*local_work_size*/ nullptr,
                                /*num_events*/ 1, &block_event, &enqueue_event);
  ASSERT_EQ(CL_SUCCESS, iRet)
      << " clEnqueueNDRangeKernel(zero sized enqueue) failed. ";

  ASSERT_NO_FATAL_FAILURE(UnlockEventAndFinishQueue(block_event, enqueue_event,
                                                    "clEnqueueNDRangeKernel"));
}

TEST_F(CL21, ZeroSized_clEnqueueCopyBuffer) {
  cl_int iRet = CL_SUCCESS;

  cl_event copy_event = nullptr;
  const size_t buffer_size = 8;
  std::vector<unsigned char> buffer(buffer_size, 0);
  std::vector<unsigned char> buffer_ref(buffer_size, 5);

  cl_mem cl_buffer_src = clCreateBuffer(m_context, CL_MEM_COPY_HOST_PTR,
                                        buffer.size(), &buffer[0], &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateBuffer failed. ";

  cl_mem cl_buffer_dst =
      clCreateBuffer(m_context, CL_MEM_COPY_HOST_PTR, buffer_ref.size(),
                     &buffer_ref[0], &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateBuffer failed. ";

  cl_event block_event = clCreateUserEvent(m_context, &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateUserEvent failed. ";

  iRet = clEnqueueCopyBuffer(m_queue, cl_buffer_src, cl_buffer_dst,
                             /*src_offset*/ 0,
                             /*dst_offset*/ 0,
                             /*size*/ 0,
                             /*num_events*/ 1, &block_event, &copy_event);
  ASSERT_EQ(CL_SUCCESS, iRet)
      << " clEnqueueCopyBuffer(zero sized enqueue) failed. ";

  ASSERT_NO_FATAL_FAILURE(UnlockEventAndFinishQueue(block_event, copy_event,
                                                    "clEnqueueCopyBuffer"));

  iRet = clEnqueueReadBuffer(m_queue, cl_buffer_dst, CL_TRUE,
                             /*offset*/ 0, buffer_ref.size(), &buffer[0],
                             /*num_events*/ 0, nullptr, nullptr);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clEnqueueReadBuffer failed. ";

  ASSERT_TRUE(buffer == buffer_ref) << " clEnqueueCopyBuffer(zero sized "
                                       "enqueue) ref and actual datas differ.";
}

TEST_F(CL21, ZeroSized_clEnqueueSVMFree) {
  cl_int iRet = CL_SUCCESS;

  cl_event svm_free_event = nullptr;

  cl_event block_event = clCreateUserEvent(m_context, &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateUserEvent failed. ";

  iRet = clEnqueueSVMFree(m_queue,
                          /*num_svm_pointers*/ 0,
                          /*svm_pointers*/ nullptr,
                          /*callback*/ nullptr,
                          /*user_data*/ nullptr,
                          /*num_event_in_wait_list*/ 1, &block_event,
                          &svm_free_event);
  ASSERT_EQ(CL_SUCCESS, iRet)
      << " clEnqueueSVMFree(zero sized enqueue) failed. ";

  ASSERT_NO_FATAL_FAILURE(UnlockEventAndFinishQueue(block_event, svm_free_event,
                                                    "clEnqueueCopyBuffer"));
}

TEST_F(CL21, ZeroSized_clEnqueueSVMFree_Negative) {
  cl_int iRet = CL_SUCCESS;

  const size_t svmBufferSize = 8;
  void *pSVMBuffer = clSVMAlloc(m_context, CL_MEM_READ_WRITE, svmBufferSize,
                                /*alignment*/ 0);
  ASSERT_NE((void *)nullptr, pSVMBuffer) << " clSVMAlloc failed. ";

  iRet = clEnqueueSVMFree(m_queue,
                          /*num_svm_pointers*/ 0,
                          /*svm_pointers*/ &pSVMBuffer,
                          /*callback*/ nullptr,
                          /*user_data*/ nullptr,
                          /*num_event_in_wait_list*/ 0,
                          /*event_wait_list*/ nullptr,
                          /*event*/ nullptr);
  ASSERT_EQ(CL_INVALID_VALUE, iRet)
      << " clEnqueueSVMFree(zero sized enqueue)(negative) failed. ";

  iRet = clEnqueueSVMFree(m_queue,
                          /*num_svm_pointers*/ 0,
                          /*svm_pointers*/ &pSVMBuffer,
                          /*callback*/ nullptr,
                          /*user_data*/ nullptr,
                          /*num_event_in_wait_list*/ 0,
                          /*event_wait_list*/ nullptr,
                          /*event*/ nullptr);
  ASSERT_EQ(CL_INVALID_VALUE, iRet)
      << " clEnqueueSVMFree(zero sized enqueue)(negative) failed. ";
}

TEST_F(CL21, ZeroSized_clEnqueueSVMMemcpy) {
  cl_int iRet = CL_SUCCESS;

  const size_t svmBufferSize_src = 8;
  std::vector<unsigned char> ref(svmBufferSize_src, 0);
  char *pSVMBuffer_src =
      (char *)clSVMAlloc(m_context, CL_MEM_READ_WRITE, svmBufferSize_src,
                         /*alignment*/ 0);
  ASSERT_NE((void *)nullptr, pSVMBuffer_src) << " clSVMAlloc failed. ";

  iRet = clEnqueueSVMMap(m_queue, CL_TRUE, CL_MAP_WRITE, pSVMBuffer_src,
                         svmBufferSize_src, 0, nullptr, nullptr);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clEnqueueSVMMap failed. ";
  memset(pSVMBuffer_src, 1, svmBufferSize_src);
  iRet = clEnqueueSVMUnmap(m_queue, pSVMBuffer_src, 0, nullptr, nullptr);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clEnqueueSVMUnmap failed. ";

  const size_t svmBufferSize_dst = 8;
  char *pSVMBuffer_dst =
      (char *)clSVMAlloc(m_context, CL_MEM_READ_WRITE, svmBufferSize_dst,
                         /*alignment*/ 0);
  ASSERT_NE((void *)nullptr, pSVMBuffer_dst) << " clSVMAlloc failed. ";

  iRet = clEnqueueSVMMap(m_queue, CL_TRUE, CL_MAP_WRITE, pSVMBuffer_dst,
                         svmBufferSize_dst, 0, nullptr, nullptr);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clEnqueueSVMMap failed. ";
  memcpy(pSVMBuffer_dst, &ref[0], ref.size());
  iRet = clEnqueueSVMUnmap(m_queue, pSVMBuffer_dst, 0, nullptr, nullptr);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clEnqueueSVMUnmap failed. ";

  cl_event svm_memcpy_event = nullptr;

  cl_event block_event = clCreateUserEvent(m_context, &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateUserEvent failed. ";

  iRet = clEnqueueSVMMemcpy(m_queue,
                            /*blocking_copy*/ CL_FALSE,
                            /*dst_ptr*/ pSVMBuffer_dst,
                            /*src_ptr*/ pSVMBuffer_src,
                            /*size*/ 0,
                            /*num_event_in_wait_list*/ 1, &block_event,
                            &svm_memcpy_event);
  ASSERT_EQ(CL_SUCCESS, iRet)
      << " clEnqueueSVMMemcpy(zero sized enqueue) failed. ";

  ASSERT_NO_FATAL_FAILURE(UnlockEventAndFinishQueue(
      block_event, svm_memcpy_event, "clEnqueueSVMMemcpy"));

  // Check content in pSVMBuffer_dst is not the same as ref any more
  iRet = clEnqueueSVMMap(m_queue, CL_TRUE, CL_MAP_WRITE, pSVMBuffer_dst,
                         svmBufferSize_dst, 0, nullptr, nullptr);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clEnqueueSVMMap failed. ";
  ASSERT_FALSE(memcmp(pSVMBuffer_dst, &ref[0], ref.size()))
      << "clEnqueueSVMMemcpy(zero sized enqueue) failed.";
  iRet = clEnqueueSVMUnmap(m_queue, pSVMBuffer_dst, 0, nullptr, nullptr);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clEnqueueSVMUnmap failed. ";

  iRet = clFinish(m_queue);
  ASSERT_EQ(CL_SUCCESS, iRet) << "clFinish failed.";
}

TEST_F(CL21, ZeroSized_clEnqueueSVMMemFill) {
  cl_int iRet = CL_SUCCESS;

  const size_t svmBufferSize_src = 8;
  std::vector<unsigned char> ref(svmBufferSize_src, 0);
  char *pSVMBuffer_src =
      (char *)clSVMAlloc(m_context, CL_MEM_READ_WRITE, svmBufferSize_src,
                         /*alignment*/ 0);
  ASSERT_NE((void *)nullptr, pSVMBuffer_src) << " clSVMAlloc failed. ";

  iRet = clEnqueueSVMMap(m_queue, CL_TRUE, CL_MAP_WRITE, pSVMBuffer_src,
                         svmBufferSize_src, 0, nullptr, nullptr);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clEnqueueSVMMap failed. ";
  memcpy(pSVMBuffer_src, &ref[0], ref.size());
  iRet = clEnqueueSVMUnmap(m_queue, pSVMBuffer_src, 0, nullptr, nullptr);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clEnqueueSVMUnmap failed. ";

  cl_event svm_memfill_event = nullptr;

  cl_event block_event = clCreateUserEvent(m_context, &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateUserEvent failed. ";

  unsigned char pattern = 1;
  iRet = clEnqueueSVMMemFill(m_queue, pSVMBuffer_src, &pattern, sizeof(pattern),
                             /*size*/ 0,
                             /*num_event_in_wait_list*/ 1, &block_event,
                             &svm_memfill_event);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clEnqueueSVMMemFill failed. ";

  ASSERT_NO_FATAL_FAILURE(UnlockEventAndFinishQueue(
      block_event, svm_memfill_event, "clEnqueueSVMMemFill"));

  // Check content in pSVMBuffer_src is not the same as ref any more
  iRet = clEnqueueSVMMap(m_queue, CL_TRUE, CL_MAP_WRITE, pSVMBuffer_src,
                         svmBufferSize_src, 0, nullptr, nullptr);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clEnqueueSVMMap failed. ";

  ASSERT_FALSE(memcmp(pSVMBuffer_src, &ref[0], ref.size()))
      << "clEnqueueSVMMemFill(zero sized enqueue) failed.";

  iRet = clEnqueueSVMUnmap(m_queue, pSVMBuffer_src, 0, nullptr, nullptr);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clEnqueueSVMUnmap failed. ";

  iRet = clFinish(m_queue);
  ASSERT_EQ(CL_SUCCESS, iRet) << "clFinish failed.";
}
