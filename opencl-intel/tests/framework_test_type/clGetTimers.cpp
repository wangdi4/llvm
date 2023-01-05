#include "CL21.h"

TEST_F(CL21, GetPlatformInfo_DEVICE_TIMER_RESOLUTION) {
  cl_int iRet = CL_SUCCESS;

  cl_ulong timer_res = 0;
  size_t ret_size = 0;
  iRet = clGetPlatformInfo(m_platform, CL_PLATFORM_HOST_TIMER_RESOLUTION,
                           sizeof(timer_res), &timer_res, &ret_size);
  ASSERT_EQ(CL_SUCCESS, iRet)
      << " clGetPlatformInfo(CL_PLATFORM_HOST_TIMER_RESOLUTION query) failed. ";
  ASSERT_EQ(sizeof(cl_ulong), ret_size);
}

TEST_F(CL21, GetHostTimer_Negative) {
  cl_int iRet = CL_SUCCESS;

  cl_ulong host_timer = 0;

  iRet = clGetHostTimer(cl_device_id(-1), &host_timer);
  ASSERT_EQ(CL_INVALID_DEVICE, iRet)
      << " clGetHostTimer with invalid device failed. ";

  iRet = clGetHostTimer(m_device, NULL);
  ASSERT_EQ(CL_INVALID_VALUE, iRet)
      << " clGetHostTimer with invalid ptr to host_timer failed. ";
}

TEST_F(CL21, GetDeviceAndHostTimer_Negative) {
  cl_int iRet = CL_SUCCESS;

  cl_ulong device_timer = 0;
  cl_ulong host_timer = 0;

  // Wrong prameters for call.
  iRet = clGetDeviceAndHostTimer(cl_device_id(-1), &device_timer, &host_timer);
  ASSERT_EQ(CL_INVALID_DEVICE, iRet)
      << " clGetDeviceAndHostTimer with invalid device failed. ";

  iRet = clGetDeviceAndHostTimer(m_device, NULL, &host_timer);
  ASSERT_EQ(CL_INVALID_VALUE, iRet)
      << " clGetDeviceAndHostTimer with invalid ptr to device_timer failed. ";

  iRet = clGetDeviceAndHostTimer(m_device, &device_timer, NULL);
  ASSERT_EQ(CL_INVALID_VALUE, iRet)
      << " clGetDeviceAndHostTimer with invalid ptr to host_timer. ";
}

TEST_F(CL21, GetHostTimer) {
  cl_int iRet = CL_SUCCESS;

  cl_ulong host_timer = 0;
  iRet = clGetHostTimer(m_device, &host_timer);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clGetHostTimer failed. ";
  ASSERT_NE((cl_ulong)0, host_timer)
      << "Looks like clGetHostTimer has not set host time.";
}

TEST_F(CL21, GetDeviceAndHostTimer) {
  cl_int iRet = CL_SUCCESS;

  cl_ulong device_timer = 0;
  cl_ulong host_timer = 0;

  iRet = clGetDeviceAndHostTimer(m_device, &device_timer, &host_timer);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clGetDeviceAndHostTimer failed. ";
  ASSERT_NE((cl_ulong)0, host_timer)
      << "Looks like clGetHostTimer has not set host time.";
  ASSERT_NE((cl_ulong)0, device_timer)
      << "Looks like clGetHostTimer has not set host time.";
}
