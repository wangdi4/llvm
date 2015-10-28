#include "CL21.h"

void CL21::GetDeviceInfo_INDEPENDENT_PROGRESS() const
{
    cl_int iRet = CL_SUCCESS;

    cl_bool ifp_supported = CL_TRUE;
    size_t ret_size = 0;
    iRet = clGetDeviceInfo(m_device,
                           CL_DEVICE_SUBGROUP_INDEPENDENT_FORWARD_PROGRESS,
                           sizeof(ifp_supported),
                           &ifp_supported,
                           &ret_size);

    ASSERT_EQ(CL_SUCCESS, iRet)
        << " clGetDeviceInfo(CL_DEVICE_SUBGROUP_INDEPENDENT_FORWARD_PROGRESS) failed. ";
    ASSERT_EQ(sizeof(cl_bool), ret_size)
        << " clGetDeviceInfo(CL_DEVICE_SUBGROUP_INDEPENDENT_FORWARD_PROGRESS) failed. "
        << " Expected and returned size differ. ";
    ASSERT_EQ((cl_bool)CL_FALSE, ifp_supported)
        << " clGetDeviceInfo(CL_DEVICE_SUBGROUP_INDEPENDENT_FORWARD_PROGRESS) failed. "
        << " Unexpected query result. ";

}

void CL21::GetDeviceInfo_CL_DEVICE_MAX_NUM_SUB_GROUPS() const
{
    cl_int iRet = CL_SUCCESS;

    cl_uint max_num_SG_for_device = 0;
    size_t ret_size = 0;
    iRet = clGetDeviceInfo(m_device,
                           CL_DEVICE_MAX_NUM_SUB_GROUPS,
                           sizeof(max_num_SG_for_device),
                           &max_num_SG_for_device,
                           &ret_size);

    ASSERT_EQ(CL_SUCCESS, iRet)
        << " clGetDeviceInfo(CL_DEVICE_MAX_NUM_SUB_GROUPS query) failed. ";
    ASSERT_EQ(sizeof(cl_uint), ret_size)
        << " clGetDeviceInfo(CL_DEVICE_MAX_NUM_SUB_GROUPS) failed. "
        << " Expected and returned size differ. ";
    ASSERT_EQ((cl_uint)1, max_num_SG_for_device)
        << " clGetDeviceInfo(CL_DEVICE_MAX_NUM_SUB_GROUPS query) failed. "
        << " Unexpected query result. ";

}
