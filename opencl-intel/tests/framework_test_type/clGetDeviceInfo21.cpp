#include "CL/cl_ext.h"
#include "CL21.h"
#include <algorithm>
#include <vector>

TEST_F(CL21, GetDeviceInfo_INDEPENDENT_PROGRESS) {
  cl_int iRet = CL_SUCCESS;

  cl_bool ifp_supported = CL_TRUE;
  size_t ret_size = 0;
  iRet = clGetDeviceInfo(m_device,
                         CL_DEVICE_SUB_GROUP_INDEPENDENT_FORWARD_PROGRESS,
                         sizeof(ifp_supported), &ifp_supported, &ret_size);

  ASSERT_EQ(CL_SUCCESS, iRet) << " clGetDeviceInfo(CL_DEVICE_SUBGROUP_"
                                 "INDEPENDENT_FORWARD_PROGRESS) failed. ";
  ASSERT_EQ(sizeof(cl_bool), ret_size)
      << " clGetDeviceInfo(CL_DEVICE_SUBGROUP_INDEPENDENT_FORWARD_PROGRESS) "
         "failed. "
      << " Expected and returned size differ. ";
  ASSERT_EQ((cl_bool)CL_FALSE, ifp_supported)
      << " clGetDeviceInfo(CL_DEVICE_SUBGROUP_INDEPENDENT_FORWARD_PROGRESS) "
         "failed. "
      << " Unexpected query result. ";
}

TEST_F(CL21, GetDeviceInfo_CL_DEVICE_MAX_NUM_SUB_GROUPS) {
  cl_int iRet = CL_SUCCESS;

  cl_uint max_num_SG_for_device = 0;
  size_t ret_size = 0;
  iRet = clGetDeviceInfo(m_device, CL_DEVICE_MAX_NUM_SUB_GROUPS,
                         sizeof(max_num_SG_for_device), &max_num_SG_for_device,
                         &ret_size);

  ASSERT_EQ(CL_SUCCESS, iRet)
      << " clGetDeviceInfo(CL_DEVICE_MAX_NUM_SUB_GROUPS query) failed. ";
  ASSERT_EQ(sizeof(cl_uint), ret_size)
      << " clGetDeviceInfo(CL_DEVICE_MAX_NUM_SUB_GROUPS) failed. "
      << " Expected and returned size differ. ";
  const cl_uint MinSGSize = 4;
  ASSERT_EQ((cl_uint)(CPU_MAX_WORK_GROUP_SIZE / MinSGSize),
            max_num_SG_for_device)
      << " clGetDeviceInfo(CL_DEVICE_MAX_NUM_SUB_GROUPS query) failed. "
      << " Unexpected query result. ";
}

TEST_F(CL21, GetDeviceInfo_CL_DEVICE_SUB_GROUP_SIZES_INTEL) {
  cl_int iRet = CL_SUCCESS;

  size_t ret_size = 0;
  iRet = clGetDeviceInfo(m_device, CL_DEVICE_SUB_GROUP_SIZES_INTEL, 0, nullptr,
                         &ret_size);
  ASSERT_EQ(CL_SUCCESS, iRet)
      << " clGetDeviceInfo(CL_DEVICE_SUB_GROUP_SIZES_INTEL query) failed. ";
  // Return type of CL_DEVICE_SUB_GROUP_SIZES_INTEL is size_t[].
  std::vector<size_t> supported_sg_sizes_for_device(ret_size / sizeof(size_t));
  iRet = clGetDeviceInfo(m_device, CL_DEVICE_SUB_GROUP_SIZES_INTEL, ret_size,
                         supported_sg_sizes_for_device.data(), nullptr);
  ASSERT_EQ(CL_SUCCESS, iRet)
      << " clGetDeviceInfo(CL_DEVICE_SUB_GROUP_SIZES_INTEL query) failed. ";

  std::vector<size_t> expected_sg_sizes = CPU_DEV_SUB_GROUP_SIZES;
  std::sort(supported_sg_sizes_for_device.begin(),
            supported_sg_sizes_for_device.end());
  ASSERT_EQ(supported_sg_sizes_for_device, expected_sg_sizes)
      << " clGetDeviceInfo(CL_DEVICE_SUB_GROUP_SIZES_INTEL query) failed. "
      << " Unexpected query result. ";
}
