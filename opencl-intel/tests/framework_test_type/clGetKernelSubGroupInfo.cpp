#include "CL21.h"
#include <stdio.h>
#if defined _M_X64 || defined __x86_64__
static const char* BC_FILE = "reqd_num_sub_groups_64.bc";
#else
static const char* BC_FILE = "reqd_num_sub_groups_32.bc";
#endif

#if defined(_WIN32) || defined (__ANDROID__)
    #define SET_FPOS_T(var, val) (var) = (val)
    #define GET_FPOS_T(var) var
#else
    #define SET_FPOS_T(var, val) ((var).__pos = (val))
    #define GET_FPOS_T(var) ((var).__pos)
#endif

void CL21::GetKernelSubGroupInfo_MAX_SB_SIZE() const
{
    cl_int iRet = CL_SUCCESS;

    cl_kernel kern = nullptr;
    GetDummyKernel(kern);

    {// local work sizes is [20,20,20]
        size_t dummy_vec[] = {20, 20, 20};
        std::vector<size_t> local_work_sizes(dummy_vec, dummy_vec + sizeof(dummy_vec) / sizeof(size_t));
        size_t max_SG_size = 0;
        size_t returned_size = 0;
        iRet = clGetKernelSubGroupInfo(kern,
                                       m_device,
                                       CL_KERNEL_MAX_SUB_GROUP_SIZE_FOR_NDRANGE,
                                       local_work_sizes.size() * sizeof(local_work_sizes[0]),
                                       &local_work_sizes[0],
                                       sizeof(max_SG_size),
                                       &max_SG_size,
                                       &returned_size);
        ASSERT_EQ(CL_SUCCESS, iRet) << " clGetKernelSubGroupInfo failed. ";
        ASSERT_EQ(sizeof(size_t), returned_size)
            << " clGetKernelSubGroupInfo failed. Expected and returned sizes differ. ";
        ASSERT_LT((size_t)0, max_SG_size)
            << " clGetKernelSubGroupInfo failed. Max subgroup size can't be less than 1. ";
    }

    {// local work sizes is [20,20,20] and pass nullptr as param_value_ret_size
        size_t dummy_vec[] = {20, 20, 20};
        std::vector<size_t> local_work_sizes(dummy_vec, dummy_vec + sizeof(dummy_vec) / sizeof(size_t));
        size_t max_SG_size = 0;
        iRet = clGetKernelSubGroupInfo(kern,
                                       m_device,
                                       CL_KERNEL_MAX_SUB_GROUP_SIZE_FOR_NDRANGE,
                                       local_work_sizes.size() * sizeof(local_work_sizes[0]),
                                       &local_work_sizes[0],
                                       sizeof(max_SG_size),
                                       &max_SG_size,
                                       /*param_value_size_ret*/nullptr);
        ASSERT_EQ(CL_SUCCESS, iRet) << " clGetKernelSubGroupInfo failed. ";
        ASSERT_LT((size_t)0, max_SG_size)
            << " clGetKernelSubGroupInfo failed. Max subgroup size can't be less than 1. ";
    }

    {// local work sizes is [1,1,1]
        size_t dummy_vec[] = {1, 1, 1};
        std::vector<size_t> local_work_sizes(dummy_vec, dummy_vec + sizeof(dummy_vec) / sizeof(size_t));
        size_t max_SG_size = 0;
        size_t returned_size = 0;
        iRet = clGetKernelSubGroupInfo(kern,
                                       m_device,
                                       CL_KERNEL_MAX_SUB_GROUP_SIZE_FOR_NDRANGE,
                                       local_work_sizes.size() * sizeof(local_work_sizes[0]),
                                       &local_work_sizes[0],
                                       sizeof(max_SG_size),
                                       &max_SG_size,
                                       &returned_size);
        ASSERT_EQ(CL_SUCCESS, iRet) << " clGetKernelSubGroupInfo failed. ";
        ASSERT_EQ(sizeof(size_t), returned_size)
            << " clGetKernelSubGroupInfo failed. Expected and returned sizes differ. ";
        ASSERT_LT((size_t)0, max_SG_size)
            << " clGetKernelSubGroupInfo failed. Max subgroup size can't be less than 1. ";
    }

    {// Null input value
        size_t max_SG_size = 0;
        size_t returned_size = 0;
        iRet = clGetKernelSubGroupInfo(kern,
                                       m_device,
                                       CL_KERNEL_MAX_SUB_GROUP_SIZE_FOR_NDRANGE,
                                       /*input_value_size*/0,
                                       /*input_value*/nullptr,
                                       sizeof(max_SG_size),
                                       &max_SG_size,
                                       &returned_size);
        ASSERT_EQ(CL_SUCCESS, iRet) << " clGetKernelSubGroupInfo failed. ";
        ASSERT_EQ(sizeof(size_t), returned_size)
            << " clGetKernelSubGroupInfo failed. Expected and returned sizes differ. ";
        ASSERT_LT((size_t)0, max_SG_size)
            << " clGetKernelSubGroupInfo failed. Max subgroup size can't be less than 1. ";
    }

    {// Null param value
        size_t dummy_vec[] = {1, 1, 1};
        std::vector<size_t> local_work_sizes(dummy_vec, dummy_vec + sizeof(dummy_vec) / sizeof(size_t));
        size_t max_SG_size = 0;
        size_t returned_size = 1;
        iRet = clGetKernelSubGroupInfo(kern,
                                       m_device,
                                       CL_KERNEL_MAX_SUB_GROUP_SIZE_FOR_NDRANGE,
                                       local_work_sizes.size() * sizeof(local_work_sizes[0]),
                                       &local_work_sizes[0],
                                       sizeof(max_SG_size),
                                       /*param_value*/nullptr,
                                       &returned_size);
        ASSERT_EQ(CL_SUCCESS, iRet) << " clGetKernelSubGroupInfo failed. ";
        ASSERT_EQ((size_t)0, returned_size)
            << " clGetKernelSubGroupInfo failed. Expected and returned sizes differ. ";
    }

    {// local work sizes is [0,0,0]
        size_t dummy_vec[] = {0, 0, 0};
        std::vector<size_t> local_work_sizes(dummy_vec, dummy_vec + sizeof(dummy_vec) / sizeof(size_t));
        size_t max_SG_size = 0;
        size_t returned_size = 0;
        iRet = clGetKernelSubGroupInfo(kern,
                                       m_device,
                                       CL_KERNEL_MAX_SUB_GROUP_SIZE_FOR_NDRANGE,
                                       local_work_sizes.size() * sizeof(local_work_sizes[0]),
                                       &local_work_sizes[0],
                                       sizeof(max_SG_size),
                                       &max_SG_size,
                                       &returned_size);
        ASSERT_EQ(CL_SUCCESS, iRet) << " clGetKernelSubGroupInfo failed. ";
        ASSERT_EQ(sizeof(size_t), returned_size)
            << " clGetKernelSubGroupInfo failed. Expected and returned sizes differ. ";
        ASSERT_EQ((size_t)0, max_SG_size)
            << " clGetKernelSubGroupInfo failed. Expected: max subgroup size is 0 for local size {0, 0, 0}. ";
    }

    {
        size_t dummy_vec[] = {1, 0, 1};
        std::vector<size_t> local_work_sizes(dummy_vec, dummy_vec + sizeof(dummy_vec) / sizeof(size_t));
        size_t max_SG_size = 0;
        size_t returned_size = 0;
        iRet = clGetKernelSubGroupInfo(kern,
                                       m_device,
                                       CL_KERNEL_MAX_SUB_GROUP_SIZE_FOR_NDRANGE,
                                       local_work_sizes.size() * sizeof(local_work_sizes[0]),
                                       &local_work_sizes[0],
                                       sizeof(max_SG_size),
                                       &max_SG_size,
                                       &returned_size);
        ASSERT_EQ(CL_SUCCESS, iRet) << " clGetKernelSubGroupInfo failed. ";
        ASSERT_EQ(sizeof(size_t), returned_size)
            << " clGetKernelSubGroupInfo failed. Expected and returned sizes differ. ";
    }

}

void CL21::GetKernelSubGroupInfo_SG_COUNT() const
{
    cl_int iRet = CL_SUCCESS;

    cl_kernel kern = nullptr;
    GetDummyKernel(kern);


    {// local work sizes is [20,20,20]
        size_t dummy_vec[] = {20, 20, 20};
        std::vector<size_t> local_work_sizes(dummy_vec, dummy_vec + sizeof(dummy_vec) / sizeof(size_t));
        size_t number_of_SG = 0;
        size_t returned_size = 0;
        iRet = clGetKernelSubGroupInfo(kern,
                                       m_device,
                                       CL_KERNEL_SUB_GROUP_COUNT_FOR_NDRANGE,
                                       local_work_sizes.size() * sizeof(local_work_sizes[0]),
                                       &local_work_sizes[0],
                                       sizeof(number_of_SG),
                                       &number_of_SG,
                                       &returned_size);
        ASSERT_EQ(CL_SUCCESS, iRet) << " clGetKernelSubGroupInfo failed. ";
        ASSERT_EQ(sizeof(size_t), returned_size)
            << " clGetKernelSubGroupInfo failed. Expected and returned size differ. ";
        ASSERT_LT((size_t)0, number_of_SG)
            << " clGetKernelSubGroupInfo failed. Max subgroup size can't be less than 1. ";
    }

    {// local work sizes is [1,1,1]
        size_t dummy_vec[] = {1, 1, 1};
        std::vector<size_t> local_work_sizes(dummy_vec, dummy_vec + sizeof(dummy_vec) / sizeof(size_t));
        size_t number_of_SG = 0;
        size_t returned_size = 0;
        iRet = clGetKernelSubGroupInfo(kern,
                                       m_device,
                                       CL_KERNEL_SUB_GROUP_COUNT_FOR_NDRANGE,
                                       local_work_sizes.size() * sizeof(local_work_sizes[0]),
                                       &local_work_sizes[0],
                                       sizeof(number_of_SG),
                                       &number_of_SG,
                                       &returned_size);
        ASSERT_EQ(CL_SUCCESS, iRet) << " clGetKernelSubGroupInfo failed. ";
        ASSERT_EQ(sizeof(size_t), returned_size)
            << " clGetKernelSubGroupInfo failed. Expected and returned sizes differ. ";
        ASSERT_LT((size_t)0, number_of_SG)
            << " clGetKernelSubGroupInfo failed. Max subgroup size can't be less than 1. ";
    }

    {// local work sizes is [0,0,0]
        size_t dummy_vec[] = {0, 0, 0};
        std::vector<size_t> local_work_sizes(dummy_vec, dummy_vec + sizeof(dummy_vec) / sizeof(size_t));
        size_t number_of_SG = 0;
        size_t returned_size = 0;
        iRet = clGetKernelSubGroupInfo(kern,
                                       m_device,
                                       CL_KERNEL_SUB_GROUP_COUNT_FOR_NDRANGE,
                                       local_work_sizes.size() * sizeof(local_work_sizes[0]),
                                       &local_work_sizes[0],
                                       sizeof(number_of_SG),
                                       &number_of_SG,
                                       &returned_size);
        ASSERT_EQ(CL_SUCCESS, iRet) << " clGetKernelSubGroupInfo failed. ";
        ASSERT_EQ(sizeof(size_t), returned_size)
            << " clGetKernelSubGroupInfo failed. Expected and returned sizes differ. ";
        ASSERT_LT((size_t)0, number_of_SG)
            << " clGetKernelSubGroupInfo failed. Max subgroup size can't be less than 1. ";
    }

    {// Null param value
        size_t dummy_vec[] = {20, 20, 20};
        std::vector<size_t> local_work_sizes(dummy_vec, dummy_vec + sizeof(dummy_vec) / sizeof(size_t));
        size_t number_of_SG = 0;
        size_t returned_size = 1;
        iRet = clGetKernelSubGroupInfo(kern,
                                       m_device,
                                       CL_KERNEL_SUB_GROUP_COUNT_FOR_NDRANGE,
                                       local_work_sizes.size() * sizeof(local_work_sizes[0]),
                                       &local_work_sizes[0],
                                       sizeof(number_of_SG),
                                       /*param_value*/nullptr,
                                       &returned_size);
        ASSERT_EQ(CL_SUCCESS, iRet) << " clGetKernelSubGroupInfo failed. ";
        ASSERT_EQ((size_t)0, returned_size)
            << " clGetKernelSubGroupInfo failed. Expected and returned sizes differ. ";
    }
}

void CL21::GetKernelSubGroupInfo_LOCAL_SIZE_FOR_SG_COUNT() const
{
    cl_int iRet = CL_SUCCESS;

    cl_kernel kern = nullptr;
    GetDummyKernel(kern);

    {// Desired SG count is 10
        size_t dummy_vec[] = {1, 1, 1};
        std::vector<size_t> local_work_sizes(dummy_vec, dummy_vec + sizeof(dummy_vec) / sizeof(size_t));
        size_t desired_SG_count = 10;
        size_t returned_size = 0;
        iRet = clGetKernelSubGroupInfo(kern,
                                       m_device,
                                       CL_KERNEL_LOCAL_SIZE_FOR_SUB_GROUP_COUNT,
                                       sizeof(desired_SG_count),
                                       &desired_SG_count,
                                       local_work_sizes.size() * sizeof(local_work_sizes[0]),
                                       &local_work_sizes[0],
                                       &returned_size);
        ASSERT_EQ(CL_SUCCESS, iRet) << " clGetKernelSubGroupInfo failed. ";
        ASSERT_EQ(sizeof(size_t) * local_work_sizes.size(), returned_size)
            << " clGetKernelSubGroupInfo failed. Expected and returned sizes differ. ";
        for(size_t i = 0; i < local_work_sizes.size(); ++i)
            ASSERT_EQ(size_t(0), local_work_sizes[i])
                << " clGetKernelSubGroupInfo failed. Expected and returned value differ. ";
    }

    {// Desired SG count is 0
        size_t dummy_vec[] = {1, 1, 1};
        std::vector<size_t> local_work_sizes(dummy_vec, dummy_vec + sizeof(dummy_vec) / sizeof(size_t));
        size_t desired_SG_count = 0;
        size_t returned_size = 0;
        iRet = clGetKernelSubGroupInfo(kern,
                                       m_device,
                                       CL_KERNEL_LOCAL_SIZE_FOR_SUB_GROUP_COUNT,
                                       sizeof(desired_SG_count),
                                       &desired_SG_count,
                                       local_work_sizes.size() * sizeof(local_work_sizes[0]),
                                       &local_work_sizes[0],
                                       &returned_size);
        ASSERT_EQ(CL_SUCCESS, iRet) << " clGetKernelSubGroupInfo failed. ";
        ASSERT_EQ(local_work_sizes.size() * sizeof(local_work_sizes[0]), returned_size)
            << " clGetKernelSubGroupInfo failed. Expected and returned sizes differ. ";
        for(size_t i = 0; i < local_work_sizes.size(); ++i)
            ASSERT_EQ(size_t(0), local_work_sizes[i])
                << " clGetKernelSubGroupInfo failed. Expected and returned value differ. ";
    }

    {// Desired SG count is 1
        size_t maxWGSize = 0;
        iRet = clGetDeviceInfo(m_device,
                               CL_DEVICE_MAX_WORK_GROUP_SIZE,
                               sizeof(maxWGSize),
                               &maxWGSize,
                               nullptr);
        ASSERT_EQ(CL_SUCCESS, iRet) << " clGetDeviceInfo failed. ";

        size_t dummy_vec[] = {0, 0, 0};
        std::vector<size_t> local_work_sizes(dummy_vec, dummy_vec + sizeof(dummy_vec) / sizeof(size_t));
        size_t desired_SG_count = 1;
        size_t returned_size = 0;
        iRet = clGetKernelSubGroupInfo(kern,
                                       m_device,
                                       CL_KERNEL_LOCAL_SIZE_FOR_SUB_GROUP_COUNT,
                                       sizeof(desired_SG_count),
                                       &desired_SG_count,
                                       local_work_sizes.size() * sizeof(local_work_sizes[0]),
                                       &local_work_sizes[0],
                                       &returned_size);
        ASSERT_EQ(CL_SUCCESS, iRet) << " clGetKernelSubGroupInfo failed. ";
        ASSERT_EQ(local_work_sizes.size() * sizeof(local_work_sizes[0]), returned_size)
            << " clGetKernelSubGroupInfo failed. Expected and returned sizes differ. ";
        ASSERT_EQ(size_t(maxWGSize), local_work_sizes[0])
                << " clGetKernelSubGroupInfo failed. Expected and returned value differ. ";
        ASSERT_EQ(size_t(1),    local_work_sizes[1])
                << " clGetKernelSubGroupInfo failed. Expected and returned value differ. ";
        ASSERT_EQ(size_t(1),    local_work_sizes[2])
                << " clGetKernelSubGroupInfo failed. Expected and returned value differ. ";
    }

    {// Null input value
        size_t dummy_vec[] = {0, 0, 0};
        std::vector<size_t> local_work_sizes(dummy_vec, dummy_vec + sizeof(dummy_vec) / sizeof(size_t));
        size_t returned_size = 0;
        iRet = clGetKernelSubGroupInfo(kern,
                                       m_device,
                                       CL_KERNEL_LOCAL_SIZE_FOR_SUB_GROUP_COUNT,
                                       /*input_value_size*/0,
                                       /*input_value*/nullptr,
                                       local_work_sizes.size() * sizeof(local_work_sizes[0]),
                                       &local_work_sizes[0],
                                       &returned_size);
        ASSERT_EQ(CL_SUCCESS, iRet) << " clGetKernelSubGroupInfo failed. ";
        ASSERT_EQ((size_t)0, returned_size)
            << " clGetKernelSubGroupInfo failed. Expected and returned sizes differ. ";
        for(size_t i = 0; i < local_work_sizes.size(); ++i)
            ASSERT_EQ(size_t(0), local_work_sizes[i])
                << " clGetKernelSubGroupInfo failed. Expected and returned value differ. ";
    }

    {// Null param value
        size_t desired_SG_count = 1;
        size_t returned_size = 0;
        iRet = clGetKernelSubGroupInfo(kern,
                                       m_device,
                                       CL_KERNEL_LOCAL_SIZE_FOR_SUB_GROUP_COUNT,
                                       sizeof(desired_SG_count),
                                       &desired_SG_count,
                                       /*param_value_size*/0,
                                       /*param_value*/nullptr,
                                       &returned_size);
        ASSERT_EQ(CL_SUCCESS, iRet) << " clGetKernelSubGroupInfo failed. ";
    }
}

void CL21::GetKernelSubGroupInfo_Negative() const
{
    cl_int iRet = CL_SUCCESS;

    cl_kernel kern = nullptr;
    GetDummyKernel(kern);

    {
        size_t dummy_vec[] = {1, 1, 1};
        std::vector<size_t> local_work_sizes(dummy_vec, dummy_vec + sizeof(dummy_vec) / sizeof(size_t));
        size_t max_SG_size = 0;
        size_t returned_size = 0;
        iRet = clGetKernelSubGroupInfo((cl_kernel)m_device,
                                       m_device,
                                       CL_KERNEL_MAX_SUB_GROUP_SIZE_FOR_NDRANGE,
                                       local_work_sizes.size() * sizeof(local_work_sizes[0]),
                                       &local_work_sizes[0],
                                       sizeof(max_SG_size),
                                       &max_SG_size,
                                       &returned_size);
        ASSERT_EQ(CL_INVALID_KERNEL, iRet) << " clGetKernelSubGroupInfo failed. ";
    }

    {
        size_t dummy_vec[] = {1, 1, 1};
        std::vector<size_t> local_work_sizes(dummy_vec, dummy_vec + sizeof(dummy_vec) / sizeof(size_t));
        size_t max_SG_size = 0;
        size_t returned_size = 0;
        iRet = clGetKernelSubGroupInfo(kern,
                                       (cl_device_id)kern,
                                       CL_KERNEL_MAX_SUB_GROUP_SIZE_FOR_NDRANGE,
                                       local_work_sizes.size() * sizeof(local_work_sizes[0]),
                                       &local_work_sizes[0],
                                       sizeof(max_SG_size),
                                       &max_SG_size,
                                       &returned_size);
        ASSERT_EQ(CL_INVALID_DEVICE, iRet) << " clGetKernelSubGroupInfo failed. ";
    }

    {// Null input value for CL_KERNEL_SUB_GROUP_COUNT_FOR_NDRANGE
        size_t number_of_SG = 0;
        size_t returned_size = 0;
        iRet = clGetKernelSubGroupInfo(kern,
                                       m_device,
                                       CL_KERNEL_SUB_GROUP_COUNT_FOR_NDRANGE,
                                       /*input_value_size*/0,
                                       /*input_value*/nullptr,
                                       sizeof(number_of_SG),
                                       &number_of_SG,
                                       &returned_size);
        ASSERT_EQ(CL_INVALID_VALUE, iRet) << " clGetKernelSubGroupInfo failed. ";
    }

    {// Input value size if 0 for CL_KERNEL_SUB_GROUP_COUNT_FOR_NDRANGE
        size_t dummy_vec[] = {1, 1, 1};
        std::vector<size_t> local_work_sizes(dummy_vec, dummy_vec + sizeof(dummy_vec) / sizeof(size_t));
        size_t number_of_SG = 0;
        size_t returned_size = 0;
        iRet = clGetKernelSubGroupInfo(kern,
                                       m_device,
                                       CL_KERNEL_SUB_GROUP_COUNT_FOR_NDRANGE,
                                       /*input_value_size*/0,
                                       &local_work_sizes[0],
                                       sizeof(number_of_SG),
                                       &number_of_SG,
                                       &returned_size);
        ASSERT_EQ(CL_INVALID_VALUE, iRet) << " clGetKernelSubGroupInfo failed. ";
    }

}

void CL21::GetKernelSubGroupInfo_MAX_NUM_SUB_GROUPS() const
{
    cl_int iRet = CL_SUCCESS;

    cl_kernel kern = nullptr;
    GetDummyKernel(kern);

    size_t returned_size = 0;
    size_t max_num_SG = 0;

    iRet = clGetKernelSubGroupInfo(kern,
                                   m_device,
                                   CL_KERNEL_MAX_NUM_SUB_GROUPS,
                                   /*input_value_size*/0,
                                   /*input_value*/nullptr,
                                   sizeof(max_num_SG),
                                   &max_num_SG,
                                   &returned_size);
    ASSERT_EQ(CL_SUCCESS, iRet)
        << " clGetKernelSubGroupInfo(CL_KERNEL_MAX_NUM_SUB_GROUPS query) failed. ";
    ASSERT_EQ(sizeof(size_t), returned_size)
        << " clGetKernelSubGroupInfo(CL_KERNEL_MAX_NUM_SUB_GROUPS) failed. Expected and returned size differ. ";
    ASSERT_LT((size_t)0, max_num_SG)
        << " clGetKernelSubGroupInfo(CL_KERNEL_MAX_NUM_SUB_GROUPS query) failed. Unexpected query result. ";
}

void CL21::GetKernelSubGroupInfo_COMPILE_NUM_SUB_GROUPS() const
{
    cl_int iRet = CL_SUCCESS;
    bool bResult = 0;

    // source code
    // __kernel __attribute__((required_num_sub_groups(5))) void test_reqd_num_sg(__global unsigned long *result)
    // {
    //     result = get_global_id(0);
    // }

    cl_context context;
    cl_device_id device;
    cl_platform_id platform = 0;

    iRet = clGetPlatformIDs(1, &platform, NULL);
    bResult &= Check("clGetPlatformIDs", CL_SUCCESS, iRet);

    cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };
    iRet = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &device, NULL);
    bResult &= Check("clGetDeviceIDs", CL_SUCCESS, iRet);

    context = clCreateContext(prop, 1, &device, NULL, NULL, &iRet);
    bResult &= Check("clCreateContext", CL_SUCCESS, iRet);

    // open binary file
    unsigned int uiContSize = 0;
    FILE* fout = fopen(BC_FILE, "rb");
    fpos_t fileSize;
    SET_FPOS_T(fileSize, 0);

    assert(fout && "Failed open file.");
    fseek(fout, 0, SEEK_END);
    fgetpos(fout, &fileSize);
    uiContSize += (unsigned int)GET_FPOS_T(fileSize);
    fseek(fout, 0, SEEK_SET);

    assert(uiContSize > 0 && "the input file must not be empty");
    unsigned char* pCont = (unsigned char*)malloc(uiContSize);

    // construct program container
    fread(((unsigned char*)pCont), 1, (size_t)GET_FPOS_T(fileSize), fout);
    fclose(fout);

    size_t binarySize = uiContSize;

    // create program with binary
    cl_int binaryStatus;
    cl_program prog = clCreateProgramWithBinary(context, 1, &device, &binarySize, const_cast<const unsigned char**>(&pCont), &binaryStatus, &iRet);
    bResult &= Check("clCreateProgramWithSource", CL_SUCCESS, iRet);

    iRet = clBuildProgram(prog, 1, &device, NULL, NULL, NULL);
    bResult &= Check("clBuildProgram", CL_SUCCESS, iRet);

    cl_kernel kern = clCreateKernel(prog, "test_reqd_num_sg", &iRet);
    bResult &= Check("clCreateKernel", CL_SUCCESS, iRet);

    size_t returned_size = 0;
    size_t required_num_SG = 0;

    iRet = clGetKernelSubGroupInfo(kern,
                                   m_device,
                                   CL_KERNEL_COMPILE_NUM_SUB_GROUPS,
                                   /*input_value_size*/0,
                                   /*input_value*/nullptr,
                                   sizeof(required_num_SG),
                                   &required_num_SG,
                                   &returned_size);
    clReleaseKernel(kern);
    clReleaseProgram(prog);
    ASSERT_EQ(CL_SUCCESS, iRet)
        << " clGetKernelSubGroupInfo(CL_KERNEL_COMPILE_NUM_SUB_GROUPS query) failed. ";
    ASSERT_EQ(sizeof(size_t), returned_size)
        << " clGetKernelSubGroupInfo(CL_KERNEL_COMPILE_NUM_SUB_GROUPS) failed. Expected and returned size differ. ";
    ASSERT_EQ((cl_uint)5, required_num_SG)
        << " clGetKernelSubGroupInfo(CL_KERNEL_COMPILE_NUM_SUB_GROUPS query) failed. Unexpected query result. ";
}

