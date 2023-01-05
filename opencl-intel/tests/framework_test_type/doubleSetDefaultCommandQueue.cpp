#include "CL20.h"

TEST_F(CL20, DoubleSetDefaultCommandQueue) {
  // Spec( 2.0 rev31, page 92) says:
  // There can only be one default device queue for each device within a
  // context. clCreateCommandQueueWithProperties with CL_QUEUE_PROPERTIES set to
  // CL_QUEUE_ON_DEVICE | CL_QUEUE_ON_DEVICE_DEFAULT will return the default
  // device queue that has already been created and increment its retain count
  // by 1.
  cl_int iRet = CL_SUCCESS;

  cl_queue_properties prop_for_default[] = {
      CL_QUEUE_PROPERTIES,
      CL_QUEUE_ON_DEVICE_DEFAULT | CL_QUEUE_ON_DEVICE |
          CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE,
      0};

  cl_command_queue default_queue1 = clCreateCommandQueueWithProperties(
      m_context, m_device, prop_for_default, &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateCommandQueueWithProperties(create "
                                 "default device queue) failed. ";

  cl_uint ref_count1 = 0;
  iRet = clGetCommandQueueInfo(default_queue1, CL_QUEUE_REFERENCE_COUNT,
                               sizeof(ref_count1), &ref_count1, NULL);
  ASSERT_EQ(CL_SUCCESS, iRet)
      << " clGetCommandQueueInfo( CL_QUEUE_REFERENCE_COUNT query ) failed. ";

  cl_command_queue default_queue2 = clCreateCommandQueueWithProperties(
      m_context, m_device, prop_for_default, &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateCommandQueueWithProperties(create "
                                 "default device queue) failed. ";

  cl_uint ref_count2 = 0;
  iRet = clGetCommandQueueInfo(default_queue2, CL_QUEUE_REFERENCE_COUNT,
                               sizeof(ref_count2), &ref_count2, NULL);
  ASSERT_EQ(CL_SUCCESS, iRet)
      << " clGetCommandQueueInfo( CL_QUEUE_REFERENCE_COUNT query ) failed. ";

  ASSERT_EQ(default_queue1, default_queue2)
      << "Error! RT retuned different default queue for one device.";
  ASSERT_EQ(ref_count1 + 1, ref_count2)
      << " Error! Unexpected reference counts. ";
}
