#include "CL20.h"

TEST_F(CL20, clSVMMap_FINE_GRAIN) {
  // !! Assume that the device supports FINE_GRAIN_SYSTEM.
  // The test validate that the clEnqueueSVMMap initializes
  // event and handles dependency correctly.

  cl_int iRet = CL_SUCCESS;

  const cl_event user_event = clCreateUserEvent(m_context, &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateUserEvent failed. ";

  std::vector<char> host_allocated_mem(10);
  cl_event map_event = nullptr;
  iRet = clEnqueueSVMMap(
      m_queue,
      /*blocking_map*/ CL_FALSE,
      /*map_flags*/ 0, &host_allocated_mem[0], host_allocated_mem.size(),
      /*num_events_in_wait_list*/ 1, &user_event, &map_event);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clEnqueueSVMMap failed. ";
  ASSERT_NE((void *)nullptr, map_event) << " clEnqueueSVMMap failed. ";

  cl_int event_status = -1;
  iRet = clGetEventInfo(map_event, CL_EVENT_COMMAND_EXECUTION_STATUS,
                        sizeof(event_status), &event_status, nullptr);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clGetEventInfo  failed. ";
  // The command should have CL_SUBMITTED or CL_QUEUED execution status.
  ASSERT_TRUE((CL_SUBMITTED | CL_QUEUED) & event_status)
      << " clEnqueueSVMMap failed. ";

  iRet = clSetUserEventStatus(user_event, CL_COMPLETE);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clGetEventInfo  failed. ";

  iRet = clFinish(m_queue);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clFinish  failed. ";

  event_status = -1;
  iRet = clGetEventInfo(map_event, CL_EVENT_COMMAND_EXECUTION_STATUS,
                        sizeof(event_status), &event_status, nullptr);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clGetEventInfo  failed. ";
  ASSERT_EQ(CL_COMPLETE, event_status) << " clGetEventInfo  failed. ";
}

TEST_F(CL20, clSVMUnmap_FINE_GRAIN) {
  // !! Assume that the device supports FINE_GRAIN_SYSTEM.
  // The test validate that the clEnqueueSVMUnmap initializes
  // event and handles dependency correctly.

  cl_int iRet = CL_SUCCESS;

  const cl_event user_event = clCreateUserEvent(m_context, &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateUserEvent failed. ";

  std::vector<char> host_allocated_mem(10);
  cl_event unmap_event = nullptr;
  iRet = clEnqueueSVMUnmap(m_queue, &host_allocated_mem[0],
                           /*num_events_in_wait_list*/ 1, &user_event,
                           &unmap_event);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clEnqueueSVMUnmap failed. ";
  ASSERT_NE((void *)nullptr, unmap_event) << " clEnqueueSVMUnmap failed. ";

  cl_int event_status = -1;
  iRet = clGetEventInfo(unmap_event, CL_EVENT_COMMAND_EXECUTION_STATUS,
                        sizeof(event_status), &event_status, nullptr);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clGetEventInfo  failed. ";
  // The command should have CL_SUBMITTED or CL_QUEUED execution status.
  ASSERT_TRUE((CL_SUBMITTED | CL_QUEUED) & event_status)
      << " clEnqueueSVMUnmap failed. ";

  iRet = clSetUserEventStatus(user_event, CL_COMPLETE);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clGetEventInfo  failed. ";

  iRet = clFinish(m_queue);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clFinish  failed. ";

  event_status = -1;
  iRet = clGetEventInfo(unmap_event, CL_EVENT_COMMAND_EXECUTION_STATUS,
                        sizeof(event_status), &event_status, nullptr);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clGetEventInfo  failed. ";
  ASSERT_EQ(CL_COMPLETE, event_status) << " clGetEventInfo  failed. ";
}
