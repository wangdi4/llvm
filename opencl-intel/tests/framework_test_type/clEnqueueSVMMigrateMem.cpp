#include "CL21.h"

TEST_F(CL21, EnqueueSVMMigrateMem_Positive) {
  cl_int iRet = CL_SUCCESS;

  const size_t SVMBuffers_size = 2;
  const size_t svmBuffer_size = 10;

  std::vector<const void *> svmBuffers(SVMBuffers_size, NULL);
  std::vector<size_t> svmBuffers_sizes(svmBuffers.size(), svmBuffer_size);
  for (auto &svmBuf : svmBuffers) {
    svmBuf = clSVMAlloc(m_context, CL_MEM_READ_WRITE, svmBuffer_size, 0);
    assert(svmBuf);
  }

  iRet = clEnqueueSVMMigrateMem(m_queue, svmBuffers.size(), &svmBuffers[0],
                                &svmBuffers_sizes[0],
                                CL_MIGRATE_MEM_OBJECT_HOST, 0, NULL, NULL);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clEnqueueSVMMigrateMem failed. ";

  iRet = clEnqueueSVMMigrateMem(
      m_queue, svmBuffers.size(), &svmBuffers[0], &svmBuffers_sizes[0],
      CL_MIGRATE_MEM_OBJECT_CONTENT_UNDEFINED, 0, NULL, NULL);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clEnqueueSVMMigrateMem failed. ";

  iRet = clFinish(m_queue);
  ASSERT_EQ(CL_SUCCESS, iRet) << "clFinish failed.";
}

TEST_F(CL21, EnqueueSVMMigrateMem_Negative) {
  cl_int iRet = CL_SUCCESS;

  std::vector<const void *> svmBuffers(2, NULL);
  const size_t svmBuffer_size = 10;
  std::vector<size_t> svmBuffers_sizes(svmBuffers.size(), svmBuffer_size);
  for (auto &svmBuf : svmBuffers) {
    svmBuf = clSVMAlloc(m_context, CL_MEM_READ_WRITE, svmBuffer_size, 0);
    assert(svmBuf);
  }

  iRet = clEnqueueSVMMigrateMem(NULL, svmBuffers.size(), &svmBuffers[0],
                                &svmBuffers_sizes[0],
                                CL_MIGRATE_MEM_OBJECT_HOST, 0, NULL, NULL);
  ASSERT_EQ(CL_INVALID_COMMAND_QUEUE, iRet)
      << " clEnqueueSVMMigrateMem with invalid queue failed. ";

  iRet =
      clEnqueueSVMMigrateMem(m_queue, 0, &svmBuffers[0], &svmBuffers_sizes[0],
                             CL_MIGRATE_MEM_OBJECT_HOST, 0, NULL, NULL);
  ASSERT_EQ(CL_INVALID_VALUE, iRet)
      << " clEnqueueSVMMigrateMem with invalid num_svm_pointers failed. ";

  iRet = clEnqueueSVMMigrateMem(m_queue, svmBuffers.size(), NULL,
                                &svmBuffers_sizes[0],
                                CL_MIGRATE_MEM_OBJECT_HOST, 0, NULL, NULL);
  ASSERT_EQ(CL_INVALID_VALUE, iRet)
      << " clEnqueueSVMMigrateMem with invalid(NULL) svm_pointers failed. ";

  std::vector<size_t> svmBuffers_wrong_sizes(svmBuffers.size(),
                                             svmBuffer_size + 12);
  iRet = clEnqueueSVMMigrateMem(m_queue, svmBuffers.size(), &svmBuffers[0],
                                &svmBuffers_wrong_sizes[0],
                                CL_MIGRATE_MEM_OBJECT_HOST, 0, NULL, NULL);
  ASSERT_EQ(CL_INVALID_VALUE, iRet)
      << " clEnqueueSVMMigrateMem with invalid sizes failed. ";

  // Addresses is not contained within an existing clSVMAlloc allocation.
  std::vector<const void *> wrong_svmBuffers(2, &svmBuffers);
  iRet = clEnqueueSVMMigrateMem(m_queue, svmBuffers.size(),
                                &wrong_svmBuffers[0], &svmBuffers_sizes[0],
                                CL_MIGRATE_MEM_OBJECT_HOST, 0, NULL, NULL);
  ASSERT_EQ(CL_INVALID_VALUE, iRet)
      << " clEnqueueSVMMigrateMem with invalid pointers failed. ";

  iRet = clEnqueueSVMMigrateMem(m_queue, svmBuffers.size(), &svmBuffers[0],
                                &svmBuffers_sizes[0],
                                CL_MIGRATE_MEM_OBJECT_HOST, 2, NULL, NULL);
  ASSERT_EQ(CL_INVALID_EVENT_WAIT_LIST, iRet)
      << " clEnqueueSVMMigrateMem with invalid numbers of events failed. ";

  cl_event user_event = clCreateUserEvent(m_context, &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateUserEvent failed. ";
  iRet = clEnqueueSVMMigrateMem(
      m_queue, svmBuffers.size(), &svmBuffers[0], &svmBuffers_sizes[0],
      CL_MIGRATE_MEM_OBJECT_HOST, 0, &user_event, NULL);
  ASSERT_EQ(CL_INVALID_EVENT_WAIT_LIST, iRet)
      << " clEnqueueSVMMigrateMem with invalid numbers of events failed. ";

  iRet = clEnqueueSVMMigrateMem(
      m_queue, svmBuffers.size(), &svmBuffers[0], &svmBuffers_sizes[0],
      CL_MIGRATE_MEM_OBJECT_HOST, 1, (cl_event *)&m_queue, NULL);
  ASSERT_EQ(CL_INVALID_EVENT_WAIT_LIST, iRet)
      << " clEnqueueSVMMigrateMem with invalid event failed. ";

  // iRet = clEnqueueSVMMigrateMem(m_queue, svmBuffers.size(), &svmBuffers[0],
  // &svmBuffers_sizes[0], 0, 0, NULL, NULL); ASSERT_EQ(CL_INVALID_VALUE, iRet)
  //     << " clEnqueueSVMMigrateMem failed. ";

  iRet = clFinish(m_queue);
  ASSERT_EQ(CL_SUCCESS, iRet) << "clFinish failed.";
}
