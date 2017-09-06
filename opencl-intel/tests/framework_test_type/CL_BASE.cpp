#include "CL_BASE.h"
#include "cl_config.h"

cl_platform_id CL_base::m_platform;
cl_device_id CL_base::m_device;
cl_context CL_base::m_context;
cl_command_queue CL_base::m_queue;
bool CL_base::m_hasFailure;
OPENCL_VERSION CL_base::m_version;

void CL_base::SetUpTestCase()
{
    m_hasFailure = true; //Switch off it at the end of function.
    cl_int iRet = CL_SUCCESS;
    m_platform = 0;
    m_device = NULL;
    m_context = NULL;
    m_version = OPENCL_VERSION::OPENCL_VERSION_UNKNOWN;

    //Get platform.
    iRet = clGetPlatformIDs(1, &m_platform, NULL);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clGetPlatformIDs failed.";

    //Check OpenCL 2.1 availability.
    size_t platVer_size = 0;
    iRet = clGetPlatformInfo(m_platform, CL_PLATFORM_VERSION, 0, NULL, &platVer_size);
    ASSERT_EQ(CL_SUCCESS, iRet)
        << " clGetPlatformInfo failed on trying to obtain CL_PLATFORM_VERSION string's size.";
    ASSERT_NE((size_t)0, platVer_size) << " CL_PLATFORM_VERSION string's size is 0 ";

    std::string platVer_str("", platVer_size);
    iRet = clGetPlatformInfo(m_platform, CL_PLATFORM_VERSION, platVer_str.size(), &platVer_str[0], NULL);
    ASSERT_EQ(CL_SUCCESS, iRet)
        << " clGetPlatformInfo failed on trying to obtain CL_PLATFORM_VERSION string.";
    if(!platVer_str.compare(0, 10, "OpenCL 2.1"))
        m_version = OPENCL_VERSION::OPENCL_VERSION_2_1;
    else if(!platVer_str.compare(0, 10, "OpenCL 2.0"))
        m_version = OPENCL_VERSION::OPENCL_VERSION_2_0;
    else if(!platVer_str.compare(0, 10, "OpenCL 1.2"))
        m_version = OPENCL_VERSION::OPENCL_VERSION_1_2;

    //Get device.
    iRet = clGetDeviceIDs(m_platform, gDeviceType, 1, &m_device, NULL);
    ASSERT_EQ(CL_SUCCESS, iRet)
        << " clGetDeviceIDs failed on trying to obtain " << gDeviceType << " device type.";

    //Create context.
    const cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)m_platform, 0 };
    m_context = clCreateContext(prop, 1, &m_device, NULL, NULL, &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateContext failed. ";

    m_queue = clCreateCommandQueueWithProperties(m_context, m_device, 0, &iRet);
    CheckException("clCreateCommandQueueWithProperties", CL_SUCCESS, iRet);

    m_hasFailure = false; //No errors occured if we here.
}

void CL_base::SetUp()
{
    const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
    std::cout << "=============================================================" << std::endl;
    std::cout << test_info->test_case_name() << "." << test_info->name() << std::endl;
    std::cout << "=============================================================" << std::endl;

    ASSERT_FALSE(m_hasFailure) << "Has errors in SetUpTestCase function. Skipping...";
    Init();
}

void CL_base::TearDown()
{
    clFinish(m_queue);
}
